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
#include <config_pplib.h>
#include <pplib/core/fileobject.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>
#include <pplib/core/mutex.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/encoder.h>
#include <utility>

#ifdef HAVE_LAME
#include <lame.h>
#endif

namespace pplib
{

#define ENCODESAMPLES 100000

AudioEncoder_MP3::AudioEncoder_MP3()
{
    out = nullptr;
    ProgressFunc = nullptr;
    ProgressFuncPrivData = nullptr;
    readcache = nullptr;
    mp3bufsize = 0;
    mp3buf = nullptr;
    have_firstaudio = false;
    started = false;
    bStopEncode = false;
    ProgressFunc = nullptr;
    samples = ENCODESAMPLES;
    gfp = nullptr;
}

AudioEncoder_MP3::~AudioEncoder_MP3()
{
    free(mp3buf);
    mp3buf = nullptr;
    free(readcache);
    readcache = nullptr;
#ifdef HAVE_LAME
    if (gfp) lame_close((lame_global_flags*)gfp);
#endif
}

void AudioEncoder_MP3::setCBR(int kbps, int quality)
{
    if (started) throw EncoderAlreadyStartedException("AudioEncoder_MP3::setCBR");
    if (quality < 0 || quality > 9) throw InvalidArgumentsException("AudioEncoder_MP3::setCBR: quality must be between 0 and 9");
    if (kbps <= 0 || kbps > 320) throw InvalidArgumentsException("AudioEncoder_MP3::setCBR: kbps must be > 0 and <= 320");
    this->encoder_mode = EncoderMode::CBR;
    this->min_bitrate = kbps;
    this->max_bitrate = kbps;
    this->quality = quality;
}

void AudioEncoder_MP3::setVBR(int min, int max, int quality)
{
    if (started) throw EncoderAlreadyStartedException("AudioEncoder_MP3::setVBR");
    if (quality < 0 || quality > 9) throw InvalidArgumentsException("AudioEncoder_MP3::setVBR: quality must be between 0 and 9");
    if (min <= 0 || min > 320) throw InvalidArgumentsException("AudioEncoder_MP3::setVBR: min must be > 0 and <= 320");
    if (max <= 0 || max > 320) throw InvalidArgumentsException("AudioEncoder_MP3::setVBR: max must be > 0 and <= 320");
    if (min > max) std::swap(min, max);
    this->min_bitrate = min;
    this->max_bitrate = max;
    this->quality = quality;
    this->encoder_mode = EncoderMode::VBR;
}

void AudioEncoder_MP3::setABR(int avg, int quality)
{
    if (started) throw EncoderAlreadyStartedException("AudioEncoder_MP3::setABR");
    if (quality < 0 || quality > 9) throw InvalidArgumentsException("AudioEncoder_MP3::setABR: quality must be between 0 and 9");
    if (avg <= 0 || avg > 320) throw InvalidArgumentsException("AudioEncoder_MP3::setABR: avg must be > 0 and <= 320");
    this->encoder_mode = EncoderMode::ABR;
    this->avg_bitrate = avg;
    this->quality = quality;
}

void AudioEncoder_MP3::setStereoMode(const AudioInfo::ChannelMode mode)
{
    if (started) throw EncoderAlreadyStartedException("AudioEncoder_MP3::setStereoMode");
    if (mode == AudioInfo::ChannelMode::DUAL_CHANNEL)
        throw UnsupportedFeatureException("AudioEncoder_MP3::setStereoMode (AudioInfo::ChannelMode::DUAL_CHANNEL)");
    if (mode != AudioInfo::ChannelMode::MONO && mode != AudioInfo::ChannelMode::STEREO && mode != AudioInfo::ChannelMode::JOINT_STEREO)
        throw InvalidArgumentsException("AudioEncoder_MP3::setStereoMode: mode must be MONO, STEREO, or JOINT_STEREO");
    this->stereo_mode = mode;
}

/*!Lowpass Frequenz für Filter setzen
 * @param [in] freq Frequenz:
 * * -1 = keinen Filter verwenden
 * * 0 = Lame entscheidet
 * * >0 = Frequenz in Hz
 */
void AudioEncoder_MP3::setLowpassFreq(int freq)
{
    if (started) throw EncoderAlreadyStartedException("AudioEncoder_MP3::setLowpassFreq");
    if (freq < -1) throw InvalidArgumentsException("AudioEncoder_MP3::setLowpassFreq: freq must be -1, 0, or >0");
    this->lowpass_freq = freq;
}

/*!Highpass Frequenz für Filter setzen
 * @param [in] freq Frequenz:
 * * -1 = keinen Filter verwenden
 * * 0 = Lame entscheidet
 * * >0 = Frequenz in Hz
 */
void AudioEncoder_MP3::setHighpassFreq(int freq)
{
    if (started) throw EncoderAlreadyStartedException("AudioEncoder_MP3::setHighpassFreq");
    if (freq < -1) throw InvalidArgumentsException("AudioEncoder_MP3::setHighpassFreq: freq must be -1, 0, or >0");
    this->highpass_freq = freq;
}

void AudioEncoder_MP3::initCBR()
{
#ifdef HAVE_LAME
    lame_set_VBR((lame_global_flags*)gfp, vbr_off);
    lame_set_brate((lame_global_flags*)gfp, max_bitrate);
    lame_set_quality((lame_global_flags*)gfp, quality);
#endif
}

#ifdef HAVE_LAME
static int preset_table[10] = {V0, V1, V2, V3, V4, V5, V6, V7, V8, V9};
#endif

void AudioEncoder_MP3::initVBR()
{
#ifdef HAVE_LAME
    int preset = V2;
    if (quality >= 0 && quality <= 9) preset = preset_table[quality];
    // lame_set_preset((lame_global_flags*)gfp, preset);
    lame_set_bWriteVbrTag((lame_global_flags*)gfp, 1);
    lame_set_VBR((lame_global_flags*)gfp, vbr_mtrh);
    lame_set_quality((lame_global_flags*)gfp, quality);
    lame_set_VBR_q((lame_global_flags*)gfp, quality);
    lame_set_VBR_quality((lame_global_flags*)gfp, (float)quality);
    lame_set_brate((lame_global_flags*)gfp, min_bitrate);
    lame_set_VBR_min_bitrate_kbps((lame_global_flags*)gfp, min_bitrate);
    lame_set_VBR_max_bitrate_kbps((lame_global_flags*)gfp, max_bitrate);

#endif
}

void AudioEncoder_MP3::initABR()
{
#ifdef HAVE_LAME
    lame_set_bWriteVbrTag((lame_global_flags*)gfp, 1);
    lame_set_preset((lame_global_flags*)gfp, avg_bitrate);
    lame_set_quality((lame_global_flags*)gfp, quality);
#endif
}

void AudioEncoder_MP3::initStereoMode()
{
#ifdef HAVE_LAME
    MPEG_mode m = JOINT_STEREO;
    if (stereo_mode == AudioInfo::ChannelMode::STEREO)
        m = STEREO;
    else if (stereo_mode == AudioInfo::ChannelMode::MONO)
        m = MONO;
    lame_set_mode((lame_global_flags*)gfp, m);
#endif
}

/**
 * @brief Initialisiert den MP3-Encoder mit den aktuellen Einstellungen.
 *
 * Diese Funktion muss vor dem Start der Kodierung aufgerufen werden, um die Encoder-Parameter zu setzen.
 */
void AudioEncoder_MP3::initEncoder()
{
#ifdef HAVE_LAME
    if (gfp) lame_close((lame_global_flags*)gfp);
    gfp = lame_init();
    if (!gfp) throw EncoderInitializationException();
    // LAME mit Standardwerten initialisieren
    lame_set_bWriteVbrTag((lame_global_flags*)gfp, 0);
    lame_set_write_id3tag_automatic((lame_global_flags*)gfp, 0);
    if (encoder_mode == EncoderMode::CBR) {
        initCBR();
    } else if (encoder_mode == EncoderMode::VBR) {
        initVBR();
    } else if (encoder_mode == EncoderMode::ABR) {
        initABR();
    }
    initStereoMode();
    lame_set_lowpassfreq((lame_global_flags*)gfp, lowpass_freq);
    lame_set_highpassfreq((lame_global_flags*)gfp, highpass_freq);

#endif
}

void AudioEncoder_MP3::startEncode(FileObject& output)
{
#ifndef HAVE_LAME
    throw pplib::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
    if (started) throw EncoderAlreadyStartedException();
    initEncoder();
    out = &output;
    started = true;
    have_firstaudio = false;
    bStopEncode = false;
#endif
}

void AudioEncoder_MP3::startEncode(int frequency, int channels)
{
#ifndef HAVE_LAME
    throw pplib::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
    if (started) throw EncoderAlreadyStartedException();
    initEncoder();
    out = NULL;
    started = true;
    have_firstaudio = true;
    bStopEncode = false;
    firstAudio.frequency = frequency;
    firstAudio.channels = channels;
    lame_set_in_samplerate((lame_global_flags*)gfp, frequency);
    lame_set_num_channels((lame_global_flags*)gfp, channels);
    if (lame_init_params((lame_global_flags*)gfp) < 0) {
        throw EncoderInitializationException();
    }
#endif
}

static inline void dispatchEncoderError(int code)
{
    // LCOV_EXCL_START
    if (code == -1) throw EncoderException("Buffer was too small");
    if (code == -2) throw OutOfMemoryException();
    if (code == -3) throw EncoderNotStartedException("Encoder not started, lame_init_params was not called");
    if (code == -4) throw EncoderException("Psychoacoustic error");
    throw EncoderException("Unknown Error");
    // LCOV_EXCL_STOP
}

int AudioEncoder_MP3::encodeBuffer(STEREOSAMPLE16* buffer, int num_samples, void* mp3buf, size_t buffersize)
{
#ifndef HAVE_LAME
    throw pplib::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
    if (!started) throw EncoderNotStartedException();
    int ret =
        lame_encode_buffer_interleaved((lame_global_flags*)gfp, (short int*)buffer, num_samples, (unsigned char*)mp3buf, (int)buffersize);
    if (ret < 0) dispatchEncoderError(ret);
    return ret;
#endif
}

int AudioEncoder_MP3::encodeBuffer(SAMPLE16* left, SAMPLE16* right, int num_samples, void* mp3buf, size_t buffersize)
{
#ifndef HAVE_LAME
    throw pplib::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
    if (!started) throw EncoderNotStartedException();
    int ret = lame_encode_buffer((lame_global_flags*)gfp, (const short int*)left, (const short int*)right, num_samples,
                                 (unsigned char*)mp3buf, (int)buffersize);
    if (ret < 0) dispatchEncoderError(ret);
    return ret;
#endif
}

int AudioEncoder_MP3::flushBuffer(void* mp3buf, size_t buffersize)
{
#ifndef HAVE_LAME
    throw pplib::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
    if (!started) return 0;
    int ret = lame_encode_flush((lame_global_flags*)gfp, (unsigned char*)mp3buf, (int)buffersize);
    return ret;
#endif
}

void AudioEncoder_MP3::encode(AudioDecoder& decoder)
{
#ifndef HAVE_LAME
    throw pplib::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
    if (!started) throw EncoderNotStartedException();
    if (bStopEncode) throw EncoderAbortedException();
    const AudioInfo& info = decoder.getAudioInfo();
    if (!have_firstaudio) {
        firstAudio.frequency = info.Frequency;
        firstAudio.channels = info.Channels;
        lame_set_in_samplerate((lame_global_flags*)gfp, firstAudio.frequency);
        lame_set_num_channels((lame_global_flags*)gfp, firstAudio.channels);
        if (lame_init_params((lame_global_flags*)gfp) < 0) {
            throw EncoderInitializationException();
        }
        have_firstaudio = true;
    } else {
        if (info.Frequency != firstAudio.frequency || info.Channels != firstAudio.channels) throw EncoderAudioFormatMismatchException();
    }
    uint32_t samples_left = info.Samples;
    if (!samples_left) return;

    mp3bufsize = (int)(1.25 * samples + 7200 + 100000);
    if (mp3buf) free(mp3buf);
    mp3buf = (unsigned char*)malloc(mp3bufsize);
    if (!mp3buf) throw OutOfMemoryException();
    if (readcache) free(readcache);
    size_t readcache_size = samples * sizeof(STEREOSAMPLE16) + 100;
    readcache = (char*)malloc(readcache_size);
    if (!readcache) throw OutOfMemoryException();
    decoder.seekSample(0);
    int last_progress = 0;
    while (samples_left > 0) {
        uint32_t current_samples = samples;
        if (current_samples > samples_left) current_samples = samples_left;
        size_t samples_got = decoder.getSamples(current_samples, (STEREOSAMPLE16*)readcache);
        if (samples_got == 0) break;
        samples_left -= samples_got;
        int encodedbytes;
        if (firstAudio.channels == 1) {
            STEREOSAMPLE16* src = (STEREOSAMPLE16*)readcache;
            short* mono = (short*)readcache;
            for (size_t i = 0; i < samples_got; ++i) {
                mono[i] = src[i].left;
            }
            encodedbytes = lame_encode_buffer((lame_global_flags*)gfp, mono, NULL, samples_got, mp3buf, mp3bufsize);
        } else {
            encodedbytes = lame_encode_buffer_interleaved((lame_global_flags*)gfp, (short*)readcache, samples_got, mp3buf, mp3bufsize);
        }
        if (encodedbytes > 0) {
            writeEncodedBytes((char*)mp3buf, (size_t)encodedbytes);
        } else if (encodedbytes < 0)
            dispatchEncoderError(encodedbytes);
        if (bStopEncode) throw EncoderAbortedException();
        if (ProgressFunc) {
            int progress = (int)((int64_t)(info.Samples - samples_left) * 100 / info.Samples);
            if (progress != last_progress) {
                ProgressFunc(progress, ProgressFuncPrivData);
            }
            last_progress = progress;
        }
    }
#endif
}

void AudioEncoder_MP3::finish()
{
#ifndef HAVE_LAME
    throw pplib::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
    if (!started) throw EncoderNotStartedException();
    if (have_firstaudio) {
        if (mp3buf) {
            uint32_t encodedbytes = lame_encode_flush_nogap((lame_global_flags*)gfp, mp3buf, mp3bufsize);
            if (encodedbytes) writeEncodedBytes((const char*)mp3buf, encodedbytes);
        } else {
            mp3buf = (unsigned char*)malloc(16192);
            if (!mp3buf) throw OutOfMemoryException();
            uint32_t encodedbytes = lame_encode_flush_nogap((lame_global_flags*)gfp, mp3buf, 16192);
            if (encodedbytes) writeEncodedBytes((const char*)mp3buf, encodedbytes);
        }
    }
    free(mp3buf);
    mp3buf = NULL;
    free(readcache);
    readcache = NULL;
    have_firstaudio = false;
    started = false;
#endif
}

void AudioEncoder_MP3::stopEncoder()
{
    bStopEncode = true;
}

const char* AudioEncoder_MP3::getLameVersion()
{
#ifndef HAVE_LAME
    throw pplib::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
    return get_lame_version();
#endif
}

const char* AudioEncoder_MP3::getPSYVersion()
{
#ifndef HAVE_LAME
    throw pplib::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
    return get_psy_version();
#endif
}

void AudioEncoder_MP3::writeEncodedBytes(const char* buffer, size_t bytes)
{
    if (out) {
        out->write(buffer, bytes);
    }
}

void AudioEncoder_MP3::setProgressFunction(void (*ProgressFunc)(int progress, void* priv), void* priv)
{
    this->ProgressFunc = ProgressFunc;
    this->ProgressFuncPrivData = priv;
}

void AudioEncoder_MP3::writeID3v2Tag(const ID3Tag& tag)
{
    if (!out) return;
    ByteArray ba;
    tag.generateId3V2Tag(ba);
    out->write(ba.ptr(), ba.size());
}

void AudioEncoder_MP3::writeID3v1Tag(const ID3Tag& tag)
{
    if (!out) return;
    ByteArray ba;
    tag.generateId3V1Tag(ba);
    out->write(ba.ptr(), ba.size());
}

} // namespace pplib
