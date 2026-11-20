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
#include <random>
#include <vector>
#include <config_pplib.h>
#include <pplib/core/fileobject.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/encoder.h>
#include <pplib/audio/sample_formats.h>
#include "OggVorbisHelper.h"

#ifdef HAVE_LIBVORBIS
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#endif

namespace pplib
{

struct AudioEncoder_Ogg::OggState
{
#ifdef HAVE_LIBVORBIS
    ogg_stream_state os{};
    vorbis_info vi{};
    vorbis_comment vc{};
    vorbis_dsp_state vd{};
    vorbis_block vb{};
    bool infoInit = false;
    bool commentInit = false;
    bool dspInit = false;
    bool blockInit = false;
    bool streamInit = false;

    void clearComments()
    {
        if (commentInit) {
            vorbis_comment_clear(&vc);
            commentInit = false;
        }
    }

    void clearStreamAndDsp()
    {
        if (blockInit) {
            vorbis_block_clear(&vb);
            blockInit = false;
        }
        if (dspInit) {
            vorbis_dsp_clear(&vd);
            dspInit = false;
        }
        if (streamInit) {
            ogg_stream_clear(&os);
            streamInit = false;
        }
        if (infoInit) {
            vorbis_info_clear(&vi);
            infoInit = false;
        }
    }

    void clearAll()
    {
        clearStreamAndDsp();
        clearComments();
    }
#endif
};

AudioEncoder_Ogg::AudioEncoder_Ogg()
{
#ifdef HAVE_LIBVORBIS
    state = new OggState();
#endif
}

AudioEncoder_Ogg::~AudioEncoder_Ogg()
{
#ifdef HAVE_LIBVORBIS
    if (state) {
        state->clearAll();
        delete state;
        state = nullptr;
    }
#endif
}

void AudioEncoder_Ogg::setVBR(float q)
{
    if (started) throw EncoderAlreadyStartedException();
    if (q < -0.1f || q > 1.0f) {
        throw IllegalArgumentException("AudioEncoder_Ogg: VBR quality must be between -0.1 and 1.0");
    }
    encoder_mode = EncoderMode::VBR;
    quality = q;
}

void AudioEncoder_Ogg::setABR(int nominalKbps, int minKbps, int maxKbps)
{
    if (started) throw EncoderAlreadyStartedException();
    if (nominalKbps <= 0 || minKbps == 0 || maxKbps == 0 || minKbps < -1 || maxKbps < -1) {
        throw IllegalArgumentException("AudioEncoder_Ogg: invalid ABR bitrate");
    }
    if (minKbps > 0 && maxKbps > 0 && minKbps > maxKbps) {
        throw IllegalArgumentException("AudioEncoder_Ogg: minKbps > maxKbps");
    }
    if ((minKbps > 0 && nominalKbps < minKbps) || (maxKbps > 0 && nominalKbps > maxKbps)) {
        throw IllegalArgumentException("AudioEncoder_Ogg: nominalKbps outside [minKbps, maxKbps]");
    }
    encoder_mode = EncoderMode::ABR;
    avg_bitrate = nominalKbps;
    min_bitrate = minKbps;
    max_bitrate = maxKbps;
}

void AudioEncoder_Ogg::setCBR(int kbps)
{
    if (started) throw EncoderAlreadyStartedException();
    if (kbps <= 0) {
        throw IllegalArgumentException("AudioEncoder_Ogg: CBR bitrate must be > 0");
    }
    encoder_mode = EncoderMode::CBR;
    avg_bitrate = kbps;
    min_bitrate = kbps;
    max_bitrate = kbps;
}

void AudioEncoder_Ogg::setProgressFunction(void (*ProgressFunc)(int progress, void* priv), void* priv)
{
    progressFunc = ProgressFunc;
    progressPriv = priv;
}

void AudioEncoder_Ogg::drainPages(bool flush)
{
#ifdef HAVE_LIBVORBIS
    ogg_page og{};
    while (flush ? ogg_stream_flush(&state->os, &og) : ogg_stream_pageout(&state->os, &og)) {
        if (og.header_len > 0) {
            if (out->write(og.header, (size_t)og.header_len) != (size_t)og.header_len) {
                throw EncoderException("AudioEncoder_Ogg: failed to write Ogg page header"); // LCOV_EXCL_LINE
            }
        }
        if (og.body_len > 0) {
            if (out->write(og.body, (size_t)og.body_len) != (size_t)og.body_len) {
                throw EncoderException("AudioEncoder_Ogg: failed to write Ogg page body"); // LCOV_EXCL_LINE
            }
        }
    }
#else
    (void)flush;
#endif
}

void AudioEncoder_Ogg::initEncoder(uint32_t freq, uint8_t ch)
{
#ifndef HAVE_LIBVORBIS
    (void)freq;
    (void)ch;
    throw UnsupportedFeatureException("AudioEncoder_Ogg");
#else
    if (freq == 0 || ch < 1 || ch > 2) {
        throw EncoderInitializationException("AudioEncoder_Ogg: unsupported frequency or channels");
    }

    state->clearStreamAndDsp();
    vorbis_info_init(&state->vi);
    state->infoInit = true;

    int ret = -1;
    if (encoder_mode == EncoderMode::VBR) {
        ret = vorbis_encode_init_vbr(&state->vi, (long)ch, (long)freq, quality);
    } else if (encoder_mode == EncoderMode::ABR) {
        long minBps = (min_bitrate > 0) ? (long)min_bitrate * 1000L : -1L;
        long nomBps = (long)avg_bitrate * 1000L;
        long maxBps = (max_bitrate > 0) ? (long)max_bitrate * 1000L : -1L;
        ret = vorbis_encode_init(&state->vi, (long)ch, (long)freq, maxBps, nomBps, minBps);
    } else {
        long bps = (long)avg_bitrate * 1000L;
        ret = vorbis_encode_init(&state->vi, (long)ch, (long)freq, bps, bps, bps);
    }
    if (ret != 0) {
        state->clearStreamAndDsp();
        throw EncoderInitializationException("AudioEncoder_Ogg: vorbis_encode_init failed (%d)", ret);
    }

    if (vorbis_analysis_init(&state->vd, &state->vi) != 0) {
        state->clearStreamAndDsp();                                                       // LCOV_EXCL_LINE
        throw EncoderInitializationException("AudioEncoder_Ogg: vorbis_analysis_init"); // LCOV_EXCL_LINE
    }
    state->dspInit = true;

    if (vorbis_block_init(&state->vd, &state->vb) != 0) {
        state->clearStreamAndDsp();                                                    // LCOV_EXCL_LINE
        throw EncoderInitializationException("AudioEncoder_Ogg: vorbis_block_init"); // LCOV_EXCL_LINE
    }
    state->blockInit = true;

    std::random_device rd;
    ogg_stream_init(&state->os, (int)rd());
    state->streamInit = true;

    if (!state->commentInit) {
        vorbis_comment_init(&state->vc);
        state->commentInit = true;
    }

    ogg_packet header{};
    ogg_packet headerComm{};
    ogg_packet headerCode{};
    if (vorbis_analysis_headerout(&state->vd, &state->vc, &header, &headerComm, &headerCode) != 0) {
        state->clearStreamAndDsp();                                                          // LCOV_EXCL_LINE
        throw EncoderInitializationException("AudioEncoder_Ogg: vorbis_analysis_headerout"); // LCOV_EXCL_LINE
    }
    ogg_stream_packetin(&state->os, &header);
    ogg_stream_packetin(&state->os, &headerComm);
    ogg_stream_packetin(&state->os, &headerCode);
    drainPages(true);
#endif
}

void AudioEncoder_Ogg::startEncode(FileObject& output)
{
#ifndef HAVE_LIBVORBIS
    (void)output;
    throw UnsupportedFeatureException("AudioEncoder_Ogg");
#else
    if (started) throw EncoderAlreadyStartedException();
    if (!output.isOpen()) throw FileNotOpenException();
    state->clearAll();
    out = &output;
    frequency = 0;
    channels = 0;
    have_firstaudio = false;
    bStopEncode.store(false);
    started = true;
#endif
}

void AudioEncoder_Ogg::setVorbisComments(const ID3Tag& tag)
{
#ifndef HAVE_LIBVORBIS
    (void)tag;
    throw UnsupportedFeatureException("AudioEncoder_Ogg");
#else
    if (!started) throw EncoderNotStartedException();
    if (have_firstaudio) throw EncoderAlreadyStartedException();

    state->clearComments();
    vorbis_comment_init(&state->vc);
    state->commentInit = true;

    std::vector<String> entries;
    tag.exportVorbisComments(entries);
    for (const String& s : entries) {
        vorbis_comment_add(&state->vc, s.c_str());
    }

    std::vector<ID3Tag::FlacPicture> pics;
    tag.exportVorbisPictures(pics);
    for (const ID3Tag::FlacPicture& p : pics) {
        ByteArray picData;
        BuildFlacPictureBlock(p, picData);
        String line = String("METADATA_BLOCK_PICTURE=") + picData.toBase64();
        vorbis_comment_add(&state->vc, line.c_str());
    }
#endif
}

void AudioEncoder_Ogg::encode(AudioDecoder& decoder)
{
#ifndef HAVE_LIBVORBIS
    (void)decoder;
    throw UnsupportedFeatureException("AudioEncoder_Ogg");
#else
    if (!started || !out) throw EncoderNotStartedException();

    const AudioInfo& info = decoder.getAudioInfo();
    if (!have_firstaudio) {
        initEncoder(info.Frequency, info.Channels);
        frequency = info.Frequency;
        channels = info.Channels;
        have_firstaudio = true;
    } else if (info.Frequency != frequency || info.Channels != channels) {
        throw EncoderAudioFormatMismatchException();
    }

    const size_t blockFrames = 4096;
    STEREOSAMPLE_FLOAT* floatBuf = static_cast<STEREOSAMPLE_FLOAT*>(readcache.malloc(blockFrames * sizeof(STEREOSAMPLE_FLOAT)));
    uint64_t encodedSamples = 0;
    const uint64_t totalSamples = info.Samples;

    auto drainBlocks = [&]() {
        while (vorbis_analysis_blockout(&state->vd, &state->vb) == 1) {
            vorbis_analysis(&state->vb, nullptr);
            vorbis_bitrate_addblock(&state->vb);
            ogg_packet op{};
            while (vorbis_bitrate_flushpacket(&state->vd, &op)) {
                ogg_stream_packetin(&state->os, &op);
                drainPages(false);
            }
        }
    };

    while (true) {
        if (bStopEncode.load()) {
            throw EncoderAbortedException();
        }
        const size_t got = decoder.getSamples(blockFrames, floatBuf);
        if (got == 0) break;

        float** buffer = vorbis_analysis_buffer(&state->vd, (int)got);
        if (channels == 1) {
            for (size_t i = 0; i < got; ++i) {
                buffer[0][i] = floatBuf[i].left;
            }
        } else {
            for (size_t i = 0; i < got; ++i) {
                buffer[0][i] = floatBuf[i].left;
                buffer[1][i] = floatBuf[i].right;
            }
        }
        vorbis_analysis_wrote(&state->vd, (int)got);
        drainBlocks();

        encodedSamples += got;
        if (progressFunc && totalSamples > 0) {
            int progress = (int)(encodedSamples * 100 / totalSamples);
            if (progress > 100) progress = 100;
            progressFunc(progress, progressPriv);
        }
    }
#endif
}

void AudioEncoder_Ogg::finish()
{
#ifndef HAVE_LIBVORBIS
    throw UnsupportedFeatureException("AudioEncoder_Ogg");
#else
    if (!started || !out) throw EncoderNotStartedException();
    if (have_firstaudio) {
        vorbis_analysis_wrote(&state->vd, 0);
        while (vorbis_analysis_blockout(&state->vd, &state->vb) == 1) {
            vorbis_analysis(&state->vb, nullptr);
            vorbis_bitrate_addblock(&state->vb);
            ogg_packet op{};
            while (vorbis_bitrate_flushpacket(&state->vd, &op)) {
                ogg_stream_packetin(&state->os, &op);
                drainPages(false);
            }
        }
        drainPages(true);
    }
    state->clearAll();
    started = false;
    have_firstaudio = false;
    out = nullptr;
#endif
}

void AudioEncoder_Ogg::stopEncoder()
{
    bStopEncode.store(true);
}

} // namespace pplib
