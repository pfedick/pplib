/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/
#include <string.h>
#include <cstdint>
#include <limits>
#include <config_pplib.h>
#include <pplib/core/fileobject.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/encoder.h>
#include <pplib/audio/sample_formats.h>

#define ENCODESAMPLES 100000

namespace pplib
{

static void writeExtended80(char* dst, uint32_t rate)
{
    if (rate == 0) {
        memset(dst, 0, 10);
        return;
    }
    uint64_t mantissa = static_cast<uint64_t>(rate) << 32;
    int exp = 16383 + 31;
    while ((mantissa & 0x8000000000000000ULL) == 0) {
        mantissa <<= 1;
        --exp;
    }
    PokeN16(dst, static_cast<uint16_t>(exp & 0x7FFF));
    PokeN64(dst + 2, mantissa);
}

static void buildAiffHeader(char* hdr, uint32_t formSize, uint16_t channels, uint32_t numSamples, uint16_t bitsPerSample, uint32_t frequency, uint32_t ssndPayloadSize)
{
    // FORM-Header (12 Bytes)
    memcpy(hdr, "FORM", 4);
    PokeN32(hdr + 4, formSize);
    memcpy(hdr + 8, "AIFF", 4);

    // COMM-Chunk (26 Bytes)
    memcpy(hdr + 12, "COMM", 4);
    PokeN32(hdr + 16, 18);
    PokeN16(hdr + 20, channels);
    PokeN32(hdr + 22, numSamples);
    PokeN16(hdr + 26, bitsPerSample);
    writeExtended80(hdr + 28, frequency);

    // SSND-Chunk-Header (16 Bytes)
    memcpy(hdr + 38, "SSND", 4);
    PokeN32(hdr + 42, ssndPayloadSize);
    PokeN32(hdr + 46, 0); // offset
    PokeN32(hdr + 50, 0); // blockSize
}

AudioEncoder_Aiff::AudioEncoder_Aiff()
{
}

AudioEncoder_Aiff::~AudioEncoder_Aiff()
{
}

void AudioEncoder_Aiff::setProgressFunction(void (*ProgressFunc)(int progress, void* priv), void* priv)
{
    progressFunc = ProgressFunc;
    progressPriv = priv;
}

void AudioEncoder_Aiff::startEncode(FileObject& output)
{
    if (started) throw EncoderAlreadyStartedException();
    out = &output;
    formStartOffset = out->tell();
    id3TagData.clear();
    totalSamples = 0;
    frequency = 0;
    channels = 0;
    bStopEncode = false;
    have_firstaudio = false;
    started = true;
}

void AudioEncoder_Aiff::writeID3v2Tag(const ID3Tag& tag)
{
    if (!started) throw EncoderNotStartedException();
    if (have_firstaudio) throw EncoderAlreadyStartedException();
    id3TagData.clear();
    tag.generateId3V2Tag(id3TagData);
}

void AudioEncoder_Aiff::encode(AudioDecoder& decoder)
{
    if (!started) throw EncoderNotStartedException();
    const AudioInfo& info = decoder.getAudioInfo();
    if (info.Channels != 1 && info.Channels != 2) {
        throw UnsupportedAudioFormatException("AudioEncoder_Aiff: only mono and stereo are supported");
    }
    if (!have_firstaudio) {
        frequency = info.Frequency;
        channels = (info.Channels == 1) ? 1 : 2;
        bitsPerSample = 16;

        char hdr[54];
        buildAiffHeader(hdr, 46, channels, 0, bitsPerSample, frequency, 8);
        if (out->write(hdr, sizeof(hdr)) != sizeof(hdr)) {
            throw EncoderException("AudioEncoder_Aiff: failed to write AIFF header");
        }
        have_firstaudio = true;
    } else {
        uint8_t ch = (info.Channels == 1) ? 1 : 2;
        if (info.Frequency != frequency || ch != channels) {
            throw EncoderAudioFormatMismatchException();
        }
    }

    uint64_t samples_left = info.Samples;
    if (!samples_left) return;

    const size_t bytesPerFrame = static_cast<size_t>(channels) * sizeof(int16_t);
    if (readcache.size() < ENCODESAMPLES * sizeof(pplib::STEREOSAMPLE16)) {
        readcache.realloc(ENCODESAMPLES * sizeof(pplib::STEREOSAMPLE16));
    }
    if (writecache.size() < ENCODESAMPLES * bytesPerFrame) {
        writecache.realloc(ENCODESAMPLES * bytesPerFrame);
    }

    pplib::STEREOSAMPLE16* readcache_ptr = (pplib::STEREOSAMPLE16*)readcache.ptr();
    char* writecache_ptr = (char*)writecache.ptr();

    decoder.seekSample(0);

    int last_progress = 0;
    while (samples_left > 0) {
        if (bStopEncode) throw EncoderAbortedException();
        uint32_t current_samples = ENCODESAMPLES;
        if (current_samples > samples_left) current_samples = static_cast<uint32_t>(samples_left);
        size_t samples_got = decoder.getSamples(current_samples, readcache_ptr);
        if (samples_got == 0) break;

        if (channels == 2) {
            for (size_t i = 0; i < samples_got; ++i) {
                PokeN16(writecache_ptr + i * 4, static_cast<uint16_t>(readcache_ptr[i].left));
                PokeN16(writecache_ptr + i * 4 + 2, static_cast<uint16_t>(readcache_ptr[i].right));
            }
        } else {
            for (size_t i = 0; i < samples_got; ++i) {
                PokeN16(writecache_ptr + i * 2, static_cast<uint16_t>(readcache_ptr[i].left));
            }
        }

        const size_t bytesToWrite = samples_got * bytesPerFrame;
        if (out->write(writecache_ptr, bytesToWrite) != bytesToWrite) {
            throw EncoderException("AudioEncoder_Aiff: failed to write audio samples");
        }

        totalSamples += samples_got;
        samples_left -= samples_got;
        if (progressFunc && info.Samples > 0) {
            int progress = (int)((int64_t)(info.Samples - samples_left) * 100 / info.Samples);
            if (progress != last_progress) {
                progressFunc(progress, progressPriv);
            }
            last_progress = progress;
        }
    }
}

void AudioEncoder_Aiff::finish()
{
    if (!started) throw EncoderNotStartedException();
    if (have_firstaudio && out) {
        const uint64_t audioDataBytes = totalSamples * static_cast<uint64_t>(channels) * 2ULL;
        const uint64_t id3Size = id3TagData.size();
        const uint64_t id3PhysicalSize = (id3Size > 0) ? (8ULL + id3Size + (id3Size % 2ULL)) : 0ULL;
        const uint64_t formSize = 46ULL + audioDataBytes + id3PhysicalSize;

        if (totalSamples > std::numeric_limits<uint32_t>::max() || formSize > std::numeric_limits<uint32_t>::max()) {
            throw EncoderException("AudioEncoder_Aiff: file size exceeds 4 GiB AIFF limit");
        }

        if (id3Size > 0) {
            char id3Hdr[8];
            PokeN32(id3Hdr, 0x49443320); // "ID3 "
            PokeN32(id3Hdr + 4, static_cast<uint32_t>(id3Size));
            if (out->write(id3Hdr, sizeof(id3Hdr)) != sizeof(id3Hdr)) {
                throw EncoderException("AudioEncoder_Aiff: failed to write ID3 chunk header");
            }
            if (out->write(id3TagData.ptr(), id3Size) != id3Size) {
                throw EncoderException("AudioEncoder_Aiff: failed to write ID3 chunk data");
            }
            if (id3Size % 2 != 0) {
                char pad = 0;
                if (out->write(&pad, 1) != 1) {
                    throw EncoderException("AudioEncoder_Aiff: failed to write ID3 padding byte");
                }
            }
        }

        const uint64_t endPos = out->tell();
        char hdr[54];
        buildAiffHeader(hdr, static_cast<uint32_t>(formSize), channels, static_cast<uint32_t>(totalSamples), bitsPerSample, frequency,
                        static_cast<uint32_t>(8ULL + audioDataBytes));
        out->seek(formStartOffset);
        if (out->write(hdr, sizeof(hdr)) != sizeof(hdr)) {
            throw EncoderException("AudioEncoder_Aiff: failed to update AIFF header");
        }
        out->seek(endPos);
    }

    id3TagData.clear();
    out = nullptr;
    formStartOffset = 0;
    totalSamples = 0;
    frequency = 0;
    channels = 0;
    have_firstaudio = false;
    started = false;
    bStopEncode = false;
}

void AudioEncoder_Aiff::stopEncoder()
{
    bStopEncode = true;
}

} // namespace pplib
