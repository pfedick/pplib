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

#include <config_pplib.h>
#include <pplib/core/fileobject.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/decoder.h>

namespace pplib
{

/** @brief Begrenzt die Samplezahl auf die real im Audiofenster vorhandene Datenmenge
 *
 * Ein Dekodierer kann ein vom Aufrufer reingereichtes AudioInfo verwenden, dessen Samples
 * nicht zu AudioSize passt. Ohne diese Grenze wird hinter dem Audioende gelesen (map()
 * zeigt dann ggf. ueber das Chunk- bzw. Dateiende hinaus).
 */
static size_t ClampToAudioSize(const AudioInfo& info, size_t position, size_t samples)
{
    const size_t frames = (info.BytesPerSample > 0) ? (size_t)(info.AudioSize / info.BytesPerSample) : 0;
    if (position >= frames) return 0;
    return (position + samples > frames) ? (frames - position) : samples;
}

AudioDecoder_Wave::AudioDecoder_Wave()
{
    position = 0;
    bytesPerChannel = 0;
    encoding = FrameEncoding::NONE;
    ff = nullptr;
}

AudioDecoder_Wave::~AudioDecoder_Wave()
{
    close();
}

void AudioDecoder_Wave::open(FileObject& file, const AudioInfo* audioinfo)
{
    if (!audioinfo) {
        if (!IdentAudioFile(file, this->info)) {
            throw UnsupportedAudioFormatException();
        }
    } else {
        this->info = *audioinfo;
    }
    if (this->info.Format != AudioInfo::AudioFormat::WAVE) throw UnsupportedAudioFormatException();
    // Dekodiert wird mono (Kanal wird verdoppelt) und stereo, nicht mehr Kanael
    if (this->info.Channels < 1 || this->info.Channels > 2) throw UnsupportedAudioFormatException("Channels not 1 or 2");

    // Die Groesse eines Frames muss sich aus Bitbreite und Kanaelzahl ergeben, sonst
    // wuerden die Lesemethoden die Samples an der falschen Stelle suchen
    const size_t chanBytes = (size_t)((this->info.BitsPerSample + 7) / 8);
    if (this->info.BytesPerSample != (uint8_t)(chanBytes * this->info.Channels))
        throw UnsupportedAudioFormatException("BytesPerSample does not match BitsPerSample and Channels");

    switch (this->info.BitsPerSample) {
    case 32:
        encoding = (this->info.SampleType == AudioInfo::SampleEncoding::FLOAT) ? FrameEncoding::FLOAT32 : FrameEncoding::INT32;
        break;
    case 24:
        encoding = FrameEncoding::INT24;
        break;
    case 16:
        encoding = FrameEncoding::INT16;
        break;
    default:
        // 8 Bit, 20 Bit in 3 Bytes und alles darueber hinaus wird nicht dekodiert
        throw UnsupportedAudioFormatException("BitsPerSample is not 16, 24 or 32");
    }
    this->bytesPerChannel = chanBytes;

    position = 0;
    this->ff = &file;

    // Ohne Vorlesebereich wuerde sequentielles Playen jede Leseoperation ein
    // munmap/mmap-Paar ausloesen. Der Wert ist bewusst klein gehalten, weil WAV-Samples
    // bis zu 8 Bytes pro Frame belegen. Ein groesserer Wert des Aufrufers bleibt erhalten.
    if (file.getMapWindowSize() < 32 * 1024) file.setMapWindowSize(32 * 1024);
}

void AudioDecoder_Wave::close()
{
    ff = nullptr;
    position = 0;
    bytesPerChannel = 0;
    encoding = FrameEncoding::NONE;
    // Nach dem Schliessen gehoeren die Felder der offenen Datei nicht mehr dazu
    info = AudioInfo();
}

const AudioInfo& AudioDecoder_Wave::getAudioInfo() const
{
    return info;
}

void AudioDecoder_Wave::seekSample(size_t sample)
{
    if (sample < info.Samples)
        position = sample;
    else
        position = info.Samples;
}

namespace
{

/// @brief extendiert eine 24 Bit Zweierkomplementzahl auf 32 Bit
inline int32_t SignExtend24(uint32_t v)
{
    return (int32_t)(v << 8) >> 8;
}

/// @brief Vollskala der unterstuetzten Ganzzahl-Formate (2^15, 2^23, 2^31). Jede Bitbreite
///        wird auf ihre eigene Vollskala normiert, damit alle Formate im Float-Puffer den
///        Bereich -1.0 bis 1.0 fuellen.
constexpr float FULL_SCALE_16 = 32768.0f;
constexpr float FULL_SCALE_24 = 8388608.0f;
constexpr float FULL_SCALE_32 = 2147483648.0f;

/// @brief bringt einen normierten Float Samplewert auf die Ebene von STEREOSAMPLE16.
///        WAV Float Dateien (Format-Tag 3) speichern normierte Werte (Bereich -1.0 bis 1.0),
///        deshalb zuerst die 16 Bit Vollskala, dann Begrenzung. NaN wird 0.
inline int32_t ScaleFloatToSample16(float v)
{
    if (v != v) return 0; //NaN vergleicht niemals gleich, deshalb ist das der NaN-Test ohne <cmath>
                          //Ohne diese Prüfung greifen die beiden Begrenzungen nicht und (int32_t)v
                          //wäre undefiniertes Verhalten; im Mixpuffer würde NaN sonst dauerhaft weitergereicht.
    v *= FULL_SCALE_16;
    if (v > 32767.0f) return 32767;
    if (v < -32768.0f) return -32768;
    return (int32_t)v;
}

} // namespace

/** @brief bereitet den nächsten Lesevorgang vor
 *
 * Begrenzt die angeforderte Samplezahl auf das Datei- bzw. Sampleende und liefert in
 * \p offset den Dateiversatz des ersten Frames. Ein Rückgabewert von 0 bedeutet
 * "nichts zu lesen". Die Prüfungen stehen hier und nicht in den vier Lesemethoden,
 * weil sie für alle Zielformate völlig gleich sind.
 */
size_t AudioDecoder_Wave::prepareRead(size_t num, const void* buffer, uint64_t& offset) const noexcept
{
    offset = 0;
    if (!ff || !buffer || num == 0 || position >= info.Samples) return 0;
    if (info.BytesPerSample == 0) return 0; // Schutz vor Division durch 0

    size_t samples = num;
    if (position + samples > info.Samples) samples = info.Samples - position;
    samples = ClampToAudioSize(info, position, samples);

    offset = (uint64_t)info.AudioStart + (uint64_t)position * info.BytesPerSample;
    try {
        const uint64_t fileSize = (uint64_t)ff->size();
        if (offset >= fileSize) return 0;
        const size_t fits = (size_t)((fileSize - offset) / info.BytesPerSample);
        if (samples > fits) samples = fits;
        return samples;
    } catch (...) {
        return 0; // falls FileObject von außen geschlossen wurde
    }
}

size_t AudioDecoder_Wave::getPosition() const noexcept
{
    return position;
}

size_t AudioDecoder_Wave::getSamples(size_t num, STEREOSAMPLE16* buffer) noexcept
{
    uint64_t offset;
    size_t samples = prepareRead(num, buffer, offset);
    if (samples == 0) return 0;

    const char* data = ff->tryMap(offset, samples * info.BytesPerSample);
    if (!data) return 0;

    const size_t frameBytes = info.BytesPerSample;
    const size_t chanBytes = bytesPerChannel;
    const bool stereo = info.Channels == 2;

    switch (encoding) {
    case FrameEncoding::INT16:
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)Peek16(data);
                int16_t r = (int16_t)Peek16(data + chanBytes);
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
        } else {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)Peek16(data);
                buffer[i].left = l;
                buffer[i].right = l;
                data += frameBytes;
            }
        }
        break;
    case FrameEncoding::INT24:
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)(SignExtend24(Peek24(data)) >> 8);
                int16_t r = (int16_t)(SignExtend24(Peek24(data + chanBytes)) >> 8);
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
        } else {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)(SignExtend24(Peek24(data)) >> 8);
                buffer[i].left = l;
                buffer[i].right = l;
                data += frameBytes;
            }
        }
        break;
    case FrameEncoding::INT32:
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)((int32_t)Peek32(data) >> 16);
                int16_t r = (int16_t)((int32_t)Peek32(data + chanBytes) >> 16);
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
            break;
        } else {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)((int32_t)Peek32(data) >> 16);
                buffer[i].left = l;
                buffer[i].right = l;
                data += frameBytes;
            }
            break;
        }
    case FrameEncoding::FLOAT32:
        // WAV Float-Samples (Format-Tag 3) sind normiert (-1.0 .. 1.0), deshalb Skala 32768
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)ScaleFloatToSample16(PeekFloat(data));
                int16_t r = (int16_t)ScaleFloatToSample16(PeekFloat(data + chanBytes));
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
            break;
        } else {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)ScaleFloatToSample16(PeekFloat(data));
                buffer[i].left = l;
                buffer[i].right = l;
                data += frameBytes;
            }
            break;
        }
    default:
        // FrameEncoding::NONE laesst open() gar nicht erst zu den Lesemethoden kommen
        return 0;
    }
    position += samples;
    return samples;
}

size_t AudioDecoder_Wave::addSamples(size_t num, STEREOSAMPLE32* buffer) noexcept
{
    uint64_t offset;
    size_t samples = prepareRead(num, buffer, offset);
    if (samples == 0) return 0;

    const char* data = ff->tryMap(offset, samples * info.BytesPerSample);
    if (!data) return 0;

    const size_t frameBytes = info.BytesPerSample;
    const size_t chanBytes = bytesPerChannel;
    const bool stereo = info.Channels == 2;

    // Der Mixpuffer nimmt durchgehend Werte im 16 Bit Bereich auf, damit Quellen
    // unterschiedlicher Bitbreite ohne weitere Skalierung aufaddiert werden koennen.
    // Abgeschnitten wird ohne Rundung: Uebersteuerungsprodukte entstehen erst durch das
    // Aufaddieren und werden am Mixbus-Ausgang begrenzt
    switch (encoding) {
    case FrameEncoding::INT16:
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)Peek16(data);
                int16_t r = (int16_t)Peek16(data + chanBytes);
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        } else {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)Peek16(data);
                buffer[i].left += l;
                buffer[i].right += l;
                data += frameBytes;
            }
            break;
        }
    case FrameEncoding::INT24:
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)(SignExtend24(Peek24(data)) >> 8);
                int16_t r = (int16_t)(SignExtend24(Peek24(data + chanBytes)) >> 8);
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        } else {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)(SignExtend24(Peek24(data)) >> 8);
                buffer[i].left += l;
                buffer[i].right += l;
                data += frameBytes;
            }
            break;
        }
    case FrameEncoding::INT32:
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)((int32_t)Peek32(data) >> 16);
                int16_t r = (int16_t)((int32_t)Peek32(data + chanBytes) >> 16);
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        } else {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)((int32_t)Peek32(data) >> 16);
                buffer[i].left += l;
                buffer[i].right += l;
                data += frameBytes;
            }
            break;
        }
    case FrameEncoding::FLOAT32:
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)ScaleFloatToSample16(PeekFloat(data));
                int16_t r = (int16_t)ScaleFloatToSample16(PeekFloat(data + chanBytes));
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        } else {
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)ScaleFloatToSample16(PeekFloat(data));
                buffer[i].left += l;
                buffer[i].right += l;
                data += frameBytes;
            }
            break;
        }
    default:
        return 0;
    }
    position += samples;
    return samples;
}

size_t AudioDecoder_Wave::getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept
{
    uint64_t offset;
    size_t samples = prepareRead(num, buffer, offset);
    if (samples == 0) return 0;

    const char* data = ff->tryMap(offset, samples * info.BytesPerSample);
    if (!data) return 0;

    const size_t frameBytes = info.BytesPerSample;
    const size_t chanBytes = bytesPerChannel;
    const bool stereo = info.Channels == 2;

    switch (encoding) {
    case FrameEncoding::INT16:
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                float l = (int16_t)Peek16(data) / FULL_SCALE_16;
                float r = (int16_t)Peek16(data + chanBytes) / FULL_SCALE_16;
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
            break;
        } else {
            for (size_t i = 0; i < samples; i++) {
                float l = (int16_t)Peek16(data) / FULL_SCALE_16;
                buffer[i].left = l;
                buffer[i].right = l;
                data += frameBytes;
            }
            break;
        }
    case FrameEncoding::INT24:
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                float l = (float)SignExtend24(Peek24(data)) / FULL_SCALE_24;
                float r = (float)SignExtend24(Peek24(data + chanBytes)) / FULL_SCALE_24;
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
            break;
        } else {
            for (size_t i = 0; i < samples; i++) {
                float l = (float)SignExtend24(Peek24(data)) / FULL_SCALE_24;
                buffer[i].left = l;
                buffer[i].right = l;
                data += frameBytes;
            }
            break;
        }
    case FrameEncoding::INT32:
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                float l = (float)(int32_t)Peek32(data) / FULL_SCALE_32;
                float r = (float)(int32_t)Peek32(data + chanBytes) / FULL_SCALE_32;
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
            break;
        } else {
            for (size_t i = 0; i < samples; i++) {
                float l = (float)(int32_t)Peek32(data) / FULL_SCALE_32;
                buffer[i].left = l;
                buffer[i].right = l;
                data += frameBytes;
            }
            break;
        }
    case FrameEncoding::FLOAT32:
        // Die Samples stehen bereits als normierter Float in der Datei, die Skala entfaellt
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                float l = PeekFloat(data);
                float r = PeekFloat(data + chanBytes);
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
            break;
        } else {
            for (size_t i = 0; i < samples; i++) {
                float l = PeekFloat(data);
                buffer[i].left = l;
                buffer[i].right = l;
                data += frameBytes;
            }
            break;
        }
    default:
        return 0;
    }
    position += samples;
    return samples;
}

size_t AudioDecoder_Wave::addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept
{
    uint64_t offset;
    size_t samples = prepareRead(num, buffer, offset);
    if (samples == 0) return 0;

    const char* data = ff->tryMap(offset, samples * info.BytesPerSample);
    if (!data) return 0;

    const size_t frameBytes = info.BytesPerSample;
    const size_t chanBytes = bytesPerChannel;
    const bool stereo = info.Channels == 2;

    switch (encoding) {
    case FrameEncoding::INT16:
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                float l = (int16_t)Peek16(data) / FULL_SCALE_16;
                float r = (int16_t)Peek16(data + chanBytes) / FULL_SCALE_16;
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        } else {
            for (size_t i = 0; i < samples; i++) {
                float l = (int16_t)Peek16(data) / FULL_SCALE_16;
                buffer[i].left += l;
                buffer[i].right += l;
                data += frameBytes;
            }
            break;
        }
    case FrameEncoding::INT24:
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                float l = (float)SignExtend24(Peek24(data)) / FULL_SCALE_24;
                float r = (float)SignExtend24(Peek24(data + chanBytes)) / FULL_SCALE_24;
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        } else {
            for (size_t i = 0; i < samples; i++) {
                float l = (float)SignExtend24(Peek24(data)) / FULL_SCALE_24;
                buffer[i].left += l;
                buffer[i].right += l;
                data += frameBytes;
            }
            break;
        }
    case FrameEncoding::INT32:
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                float l = (float)(int32_t)Peek32(data) / FULL_SCALE_32;
                float r = (float)(int32_t)Peek32(data + chanBytes) / FULL_SCALE_32;
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        } else {
            for (size_t i = 0; i < samples; i++) {
                float l = (float)(int32_t)Peek32(data) / FULL_SCALE_32;
                buffer[i].left += l;
                buffer[i].right += l;
                data += frameBytes;
            }
            break;
        }
    case FrameEncoding::FLOAT32:
        if (stereo) {
            for (size_t i = 0; i < samples; i++) {
                float l = PeekFloat(data);
                float r = PeekFloat(data + chanBytes);
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        } else {
            for (size_t i = 0; i < samples; i++) {
                float l = PeekFloat(data);
                buffer[i].left += l;
                buffer[i].right += l;
                data += frameBytes;
            }
            break;
        }
    default:
        return 0;
    }
    position += samples;
    return samples;
}

} // namespace pplib
