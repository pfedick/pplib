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
#include <vector>
#include <config_pplib.h>
#include <pplib/core/fileobject.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>
#include <pplib/core/mutex.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/encoder.h>
#include <pplib/audio/sample_formats.h>

#ifdef HAVE_LIBFLAC
#include <FLAC/stream_encoder.h>
#include <FLAC/metadata.h>
#endif

#define ENCODESAMPLES 100000

namespace pplib
{

// Callbacks
#ifdef HAVE_LIBFLAC
static FLAC__StreamEncoderWriteStatus flacWriteCb(
    const FLAC__StreamEncoder*, const FLAC__byte buffer[], size_t bytes, uint32_t, uint32_t, void* client_data)
{
    FileObject* out = static_cast<FileObject*>(client_data);
    try {
        if (out->write(buffer, bytes) != bytes) {
            // LCOV_EXCL_START
            return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
        }
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    } catch (...) {
        return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
    }
    // LCOV_EXCL_STOP
}

static FLAC__StreamEncoderSeekStatus flacSeekCb(const FLAC__StreamEncoder*, FLAC__uint64 absolute_byte_offset, void* client_data)
{
    FileObject* out = static_cast<FileObject*>(client_data);
    try {
        out->seek(absolute_byte_offset);
        return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;
        // LCOV_EXCL_START
    } catch (...) {

        return FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR;
    }
    // LCOV_EXCL_STOP
}

static FLAC__StreamEncoderTellStatus flacTellCb(const FLAC__StreamEncoder*, FLAC__uint64* absolute_byte_offset, void* client_data)
{
    FileObject* out = static_cast<FileObject*>(client_data);
    try {
        *absolute_byte_offset = out->tell();
        return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
        // LCOV_EXCL_START
    } catch (...) {
        return FLAC__STREAM_ENCODER_TELL_STATUS_ERROR;
    }
    // LCOV_EXCL_STOP
}

#endif

struct AudioEncoder_Flac::FlacState
{
    FLAC__StreamEncoder* encoder = nullptr;
    FLAC__StreamMetadata* padding = nullptr;
    FLAC__StreamMetadata* vorbisComment = nullptr;
    std::vector<FLAC__StreamMetadata*> pictures;

#ifdef HAVE_LIBFLAC
    void clearTagMetadata()
    {
        if (vorbisComment) {
            FLAC__metadata_object_delete(vorbisComment);
            vorbisComment = nullptr;
        }
        for (FLAC__StreamMetadata* pic : pictures) {
            if (pic) FLAC__metadata_object_delete(pic);
        }
        pictures.clear();
    }

    void clearAllMetadata()
    {
        clearTagMetadata();
        if (padding) {
            FLAC__metadata_object_delete(padding);
            padding = nullptr;
        }
    }
#endif
};

AudioEncoder_Flac::AudioEncoder_Flac()
{
#ifdef HAVE_LIBFLAC
    state = new FlacState();
    state->encoder = FLAC__stream_encoder_new();
    if (!state->encoder) {
        // LCOV_EXCL_START
        delete state;
        state = nullptr;
        throw pplib::InitializationFailedException("AudioEncoder_Flac: libFLAC");
    }
    // LCOV_EXCL_STOP
#endif
}

AudioEncoder_Flac::~AudioEncoder_Flac()
{
#ifdef HAVE_LIBFLAC
    if (state) {
        if (state->encoder) {
            FLAC__stream_encoder_delete(state->encoder);
        }
        state->clearAllMetadata();
        delete state;
        state = nullptr;
    }
#endif
}

void AudioEncoder_Flac::setCompressionLevel(int level)
{
    if (level < 0 || level > 8) {
        throw pplib::IllegalArgumentException("AudioEncoder_Flac: compression level must be between 0 and 8");
    }
    compressionLevel = level;
}

void AudioEncoder_Flac::setProgressFunction(void (*ProgressFunc)(int progress, void* priv), void* priv)
{
    progressFunc = ProgressFunc;
    progressPriv = priv;
}

void AudioEncoder_Flac::startEncode(FileObject& output)
{
    if (started) throw EncoderAlreadyStartedException();
#ifdef HAVE_LIBFLAC
    if (state) state->clearAllMetadata();
#endif
    out = &output;
    bStopEncode = false;
    started = true;
    have_firstaudio = false;
}

void AudioEncoder_Flac::encode(AudioDecoder& decoder)
{
#ifndef HAVE_LIBFLAC
    throw pplib::UnsupportedFeatureException("AudioEncoder_Flac: libFLAC");
#else
    if (!started) throw EncoderNotStartedException();
    const AudioInfo& info = decoder.getAudioInfo();
    if (info.Channels != 1 && info.Channels != 2) {
        throw UnsupportedAudioFormatException("AudioEncoder_Flac: only mono and stereo are supported"); // LCOV_EXCL_LINE
    }
    if (!have_firstaudio) {
        frequency = info.Frequency;
        channels = (info.Channels == 1) ? 1 : 2;
        bitsPerSample = 16; // bei Nutzung von STEREOSAMPLE16

        FLAC__stream_encoder_set_channels(state->encoder, channels);
        FLAC__stream_encoder_set_bits_per_sample(state->encoder, bitsPerSample);
        FLAC__stream_encoder_set_sample_rate(state->encoder, frequency);
        FLAC__stream_encoder_set_compression_level(state->encoder, compressionLevel);

        if (!state->padding) {
            state->padding = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING);
            if (state->padding) {
                state->padding->length = 4096;
            }
        }

        std::vector<FLAC__StreamMetadata*> metadata;
        if (state->vorbisComment) metadata.push_back(state->vorbisComment);
        for (FLAC__StreamMetadata* pic : state->pictures) {
            if (pic) metadata.push_back(pic);
        }
        if (state->padding) metadata.push_back(state->padding);
        if (!metadata.empty()) {
            FLAC__stream_encoder_set_metadata(state->encoder, metadata.data(), static_cast<uint32_t>(metadata.size()));
        }

        if (FLAC__stream_encoder_init_stream(state->encoder, flacWriteCb, flacSeekCb, flacTellCb, nullptr, out) !=
            FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
            throw EncoderInitializationException(); // LCOV_EXCL_LINE
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

    if (readcache.size() < ENCODESAMPLES * sizeof(pplib::STEREOSAMPLE16)) {
        readcache.realloc(ENCODESAMPLES * sizeof(pplib::STEREOSAMPLE16));
    }
    if (writecache.size() < ENCODESAMPLES * sizeof(pplib::STEREOSAMPLE32)) {
        writecache.realloc(ENCODESAMPLES * sizeof(pplib::STEREOSAMPLE32));
    }

    pplib::STEREOSAMPLE16* readcache_ptr = (pplib::STEREOSAMPLE16*)readcache.ptr();
    pplib::STEREOSAMPLE32* writecache_ptr = (pplib::STEREOSAMPLE32*)writecache.ptr();
    FLAC__int32* mono_ptr = (FLAC__int32*)writecache.ptr();

    decoder.seekSample(0);

    int last_progress = 0;
    while (samples_left > 0) {
        if (bStopEncode) throw EncoderAbortedException();
        uint32_t current_samples = ENCODESAMPLES;
        if (current_samples > samples_left) current_samples = samples_left;
        size_t samples_got = decoder.getSamples(current_samples, readcache_ptr);
        if (samples_got == 0) break;
        // Auf 32-Bit konvertieren
        if (channels == 2) {
            for (size_t i = 0; i < samples_got; ++i) {
                writecache_ptr[i].left = readcache_ptr[i].left;
                writecache_ptr[i].right = readcache_ptr[i].right;
            }
        } else {
            for (size_t i = 0; i < samples_got; ++i) {
                mono_ptr[i] = readcache_ptr[i].left;
            }
        }
        if (!FLAC__stream_encoder_process_interleaved(state->encoder, (FLAC__int32*)writecache_ptr, static_cast<uint32_t>(samples_got)))
            throw EncoderException("AudioEncoder_Flac: FLAC__stream_encoder_process_interleaved failed"); // LCOV_EXCL_LINE

        samples_left -= samples_got;
        if (progressFunc) {
            int progress = (int)((int64_t)(info.Samples - samples_left) * 100 / info.Samples);
            if (progress != last_progress) {
                progressFunc(progress, progressPriv);
            }
            last_progress = progress;
        }
    }
#endif
}

void AudioEncoder_Flac::finish()
{
#ifndef HAVE_LIBFLAC
    throw pplib::UnsupportedFeatureException("AudioEncoder_Flac: libFLAC");
#else
    if (!started) throw EncoderNotStartedException();
    if (have_firstaudio && state && state->encoder) {
        if (!FLAC__stream_encoder_finish(state->encoder)) {
            throw EncoderException("AudioEncoder_Flac: FLAC__stream_encoder_finish failed"); // LCOV_EXCL_LINE
        }
    }
    if (state) {
        state->clearAllMetadata();
    }
    out = nullptr;
    have_firstaudio = false;
    started = false;
    bStopEncode = false;
#endif
}

void AudioEncoder_Flac::stopEncoder()
{
    bStopEncode = true;
}

void AudioEncoder_Flac::setVorbisComments(const ID3Tag& tag)
{
#ifndef HAVE_LIBFLAC
    throw pplib::UnsupportedFeatureException("AudioEncoder_Flac: libFLAC");
#else
    if (!started) throw EncoderNotStartedException();
    if (have_firstaudio) throw EncoderAlreadyStartedException();

    state->clearTagMetadata();

    std::vector<String> entries;
    tag.exportVorbisComments(entries);
    if (!entries.empty()) {
        state->vorbisComment = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
        if (!state->vorbisComment) {
            throw EncoderInitializationException("AudioEncoder_Flac: failed to allocate VORBIS_COMMENT"); // LCOV_EXCL_LINE
        }
        for (const String& s : entries) {
            FLAC__StreamMetadata_VorbisComment_Entry entry;
            entry.length = static_cast<FLAC__uint32>(s.size());
            entry.entry = reinterpret_cast<FLAC__byte*>(const_cast<char*>(s.c_str()));
            FLAC__metadata_object_vorbiscomment_append_comment(state->vorbisComment, entry, true);
        }
    }

    std::vector<ID3Tag::FlacPicture> pics;
    tag.exportVorbisPictures(pics);
    for (const ID3Tag::FlacPicture& p : pics) {
        FLAC__StreamMetadata* pic = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PICTURE);
        if (!pic) {
            // LCOV_EXCL_START
            state->clearTagMetadata();
            throw EncoderInitializationException("AudioEncoder_Flac: failed to allocate PICTURE");
            // LCOV_EXCL_STOP
        }
        pic->data.picture.type = static_cast<FLAC__StreamMetadata_Picture_Type>(p.type);
        FLAC__metadata_object_picture_set_mime_type(pic, const_cast<char*>(p.mime.c_str()), true);
        FLAC__metadata_object_picture_set_description(pic, reinterpret_cast<FLAC__byte*>(const_cast<char*>(p.description.c_str())), true);
        if (!p.data.isEmpty()) {
            FLAC__metadata_object_picture_set_data(pic, reinterpret_cast<FLAC__byte*>(const_cast<void*>(p.data.ptr())),
                                                   static_cast<FLAC__uint32>(p.data.size()), true);
        }
        state->pictures.push_back(pic);
    }
#endif
}

} // namespace pplib
