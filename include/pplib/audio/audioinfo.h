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

#ifndef PPLIB_AUDIO_AUDIOINFO_H
#define PPLIB_AUDIO_AUDIOINFO_H

#include <cstdint>
#include <pplib/core/fileobject.h>

namespace pplib
{

//!\brief Struktur zum Speichern eines WAVE-Headers
typedef struct tagWAVEHeader
{
    uint32_t datastart;
    uint32_t numSamples;
    uint32_t bytes;
    uint32_t frequency;
    uint32_t bytespersample;
    uint32_t bytespersecond;
    uint32_t seconds;
    uint8_t channels;
    uint8_t bitdepth;
} WAVEHEADER;

class AudioInfo
{
public:
    AudioInfo();

    enum AudioFormat
    {
        UNKNOWN,
        WAVE,
        AIFF,
        MP3,
        OGG
    };
    enum ChannelMode
    {
        STEREO,
        MONO,
        DUAL_CHANNEL,
        JOINT_STEREO
    };

    AudioFormat Format;
    bool HaveID3v2Tag;
    bool IsVBR;
    uint32_t ID3v2TagStart;
    uint32_t FileSize;
    uint32_t AudioStart;
    uint32_t AudioEnd;
    uint32_t AudioSize;
    uint32_t Samples;
    uint32_t Length; // Length in ms
    uint32_t Frequency;
    uint16_t Bitrate;
    uint8_t BitsPerSample;
    uint8_t BytesPerSample;
    uint8_t Channels;
    ChannelMode Mode;
};

bool IdentAudioFile(FileObject& file, AudioInfo& info);
AudioInfo::AudioFormat IdentAudioFile(FileObject& file);

} // namespace pplib

#endif // PPLIB_AUDIO_AUDIOINFO_H