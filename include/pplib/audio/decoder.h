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

#ifndef PPLIB_AUDIO_DECODER_H
#define PPLIB_AUDIO_DECODER_H

#include <cstdint>
#include <pplib/core/baseexception.h>
#include <pplib/core/fileobject.h>
#include <pplib/audio/sample_formats.h>
#include <pplib/audio/audioinfo.h>

namespace pplib
{

PPLIBEXCEPTION(DecoderException, Exception);
PPLIBEXCEPTION(DecoderInitializationException, DecoderException);

class AudioDecoder
{
public:
    virtual ~AudioDecoder() {};
    virtual void open(FileObject& file, const AudioInfo* info = NULL) = 0;
    virtual const AudioInfo& getAudioInfo() const = 0;
    virtual void getAudioInfo(AudioInfo& info) const = 0;
    virtual void seekSample(size_t sample) = 0;
    virtual size_t getPosition() const = 0;
    virtual size_t getSamples(size_t num, STEREOSAMPLE16* buffer) = 0;
    virtual size_t addSamples(size_t num, STEREOSAMPLE32* buffer) = 0;
    virtual size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) = 0;
    virtual size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) = 0;
};

AudioDecoder* GetAudioDecoder(FileObject& file);

class AudioDecoder_Wave : public AudioDecoder
{
private:
    FileObject* ff;
    AudioInfo info;
    size_t position;
    size_t samplesize;
    void readWaveHeader(FileObject& file, WAVEHEADER& header);

public:
    AudioDecoder_Wave();
    ~AudioDecoder_Wave();
    void open(FileObject& file, const AudioInfo* info = NULL);
    const AudioInfo& getAudioInfo() const;
    void getAudioInfo(AudioInfo& info) const;
    void seekSample(size_t sample);
    size_t getPosition() const;
    size_t getSamples(size_t num, STEREOSAMPLE16* buffer);
    size_t addSamples(size_t num, STEREOSAMPLE32* buffer);
    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer);
    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer);
};

class AudioDecoder_Aiff : public AudioDecoder
{
private:
    FileObject* ff;
    AudioInfo info;
    size_t position;
    size_t samplesize;

public:
    AudioDecoder_Aiff();
    ~AudioDecoder_Aiff();
    void open(FileObject& file, const AudioInfo* info = NULL);
    const AudioInfo& getAudioInfo() const;
    void getAudioInfo(AudioInfo& info) const;
    void seekSample(size_t sample);
    size_t getPosition() const;
    size_t getSamples(size_t num, STEREOSAMPLE16* buffer);
    size_t addSamples(size_t num, STEREOSAMPLE32* buffer);
    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer);
    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer);
};

class AudioDecoder_MP3 : public AudioDecoder
{
private:
    void* decoder;
    FileObject* ff;
    uint8_t* readbuffer;
    uint8_t* outbuffer;

    AudioInfo info;
    size_t position;
    size_t samplesize;
    size_t out_offset, out_size;
    bool isRunning;
    bool needInput;
    int lastDecodeFormat;

    size_t fillDecodeBuffer();

public:
    AudioDecoder_MP3();
    ~AudioDecoder_MP3();
    void open(FileObject& file, const AudioInfo* info = NULL);
    const AudioInfo& getAudioInfo() const;
    void getAudioInfo(AudioInfo& info) const;
    void seekSample(size_t sample);
    size_t getPosition() const;
    size_t getSamples(size_t num, STEREOSAMPLE16* buffer);
    size_t addSamples(size_t num, STEREOSAMPLE32* buffer);
    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer);
    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer);
};

class AudioDecoder_Ogg : public AudioDecoder
{
private:
    void* private_data;
    size_t position;
    char* readbuffer;
    size_t buffersize;
    char* decodebuffer;
    int decodebuffer_size;
    AudioInfo info;

    void allocateBuffer(size_t size);

public:
    AudioDecoder_Ogg();
    ~AudioDecoder_Ogg();
    void open(FileObject& file, const AudioInfo* info = NULL);
    const AudioInfo& getAudioInfo() const;
    void getAudioInfo(AudioInfo& info) const;
    void seekSample(size_t sample);
    size_t getPosition() const;
    size_t getSamples(size_t num, STEREOSAMPLE16* buffer);
    size_t addSamples(size_t num, STEREOSAMPLE32* buffer);
    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer);
    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer);
};

} // namespace pplib

#endif // PPLIB_AUDIO_DECODER_H