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
#include <cstring>
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

AudioDecoder_Aiff::AudioDecoder_Aiff()
{
    position = 0;
    bytesPerChannel = 0;
    encoding = FrameEncoding::NONE;
    ff = nullptr;
}

AudioDecoder_Aiff::~AudioDecoder_Aiff()
{
    close();
}

void AudioDecoder_Aiff::open(FileObject& file, const AudioInfo* audioinfo)
{
    if (!audioinfo) {
        if (!IdentAudioFile(file, this->info)) {
            throw DecoderInitializationException();
        }
    } else {
        this->info = *audioinfo;
    }
    if (this->info.Format != AudioInfo::AudioFormat::AIFF) throw UnsupportedAudioFormatException();
    // Dekodiert wird mono (Kanal wird verdoppelt) und stereo, nicht mehr Kanael
    if (this->info.Channels < 1 || this->info.Channels > 2) throw UnsupportedAudioFormatException("Channels not 1 or 2");

    // Die Groesse eines Frames muss sich aus Bitbreite und Kanaelzahl ergeben, sonst
    // wuerden die Lesemethoden die Samples an der falschen Stelle suchen
    const size_t chanBytes = (size_t)((this->info.BitsPerSample + 7) / 8);
    if (this->info.BytesPerSample != (uint8_t)(chanBytes * this->info.Channels))
        throw UnsupportedAudioFormatException("BytesPerSample does not match BitsPerSample and Channels");

    switch (this->info.BitsPerSample) {
        case 32:
            encoding = (this->info.SampleType == AudioInfo::SampleEncoding::FLOAT)
                           ? FrameEncoding::FLOAT32
                           : FrameEncoding::INT32;
            break;
        case 24:
            encoding = FrameEncoding::INT24;
            break;
        case 16:
            encoding = FrameEncoding::INT16;
            break;
        default:
            // 8 Bit (und alles darueber hinaus) wird nicht dekodiert
            throw UnsupportedAudioFormatException("BitsPerSample is not 16, 24 or 32");
    }
    this->bytesPerChannel = chanBytes;

    position = 0;
    this->ff = &file;

    // Ohne Vorlesebereich wuerde sequentielles Playen jede Leseoperation ein
    // munmap/mmap-Paar ausloesen. Bewusst kleiner als bei den anderen Decodern, weil
    // AIFF-Samples bis zu 8 Bytes pro Frame belegen. Ein groesserer Wert des Aufrufers
    // bleibt erhalten und wird nicht zurueckgesetzt.
    if (file.getMapWindowSize() < 32 * 1024) file.setMapWindowSize(32 * 1024);
}

void AudioDecoder_Aiff::close()
{
    ff = nullptr;
    position = 0;
    bytesPerChannel = 0;
    encoding = FrameEncoding::NONE;
    // Nach dem Schliessen gehoeren die Felder der offenen Datei nicht mehr dazu
    info = AudioInfo();
}

const AudioInfo& AudioDecoder_Aiff::getAudioInfo() const
{
    return info;
}

void AudioDecoder_Aiff::seekSample(size_t sample)
{
    if (sample < info.Samples)
        position = sample;
    else
        position = info.Samples;
}

size_t AudioDecoder_Aiff::getPosition() const
{
    return position;
}

namespace {

/// @brief extendiert eine 24 Bit Zweierkomplementzahl auf 32 Bit
inline int32_t SignExtend24(uint32_t v)
{
    return (int32_t)(v << 8) >> 8;
}

/// @brief holt einen big endian IEEE 754 float aus den vier Bytes an p
inline float PeekFloatBE(const char* p)
{
    static_assert(sizeof(float) == 4, "AIFF float samples need a 4 byte floating point type");
    uint32_t bits = PeekN32(p);
    float f;
    memcpy(&f, &bits, sizeof(f));
    return f;
}

/// @brief bringt einen normierten Float Samplewert auf die Ebene von STEREOSAMPLE16.
///        AIFF-C Float Dateien speichern normierte Werte (Bereich -1.0 bis 1.0), deshalb
///        zuerst die 16 Bit Vollskaala, dann Begrenzung. NaN wird 0.
inline int32_t ScaleFloatToSample16(float v)
{
    if (v != v) return 0; //NaN vergleicht niemals gleich, deshalb ist das der NaN-Test ohne <cmath>.
                          //Ohne diese Prüfung greifen die beiden Begrenzungen nicht und (int32_t)v
                          //wäre undefiniertes Verhalten; im Mixpuffer würde NaN sonst dauerhaft weitergereicht.
    v *= 32768.0f;
    if (v > 32767.0f) return 32767;
    if (v < -32768.0f) return -32768;
    return (int32_t)v;
}

/// @brief Vollskaala aller Sampleformate, entspricht 2^31
constexpr float FULL_SCALE_F = 2147483648.0f;

} // namespace

/** @brief bereitet den nächsten Lesevorgang vor
 *
 * Begrenzt die angeforderte Samplezahl auf das Datei- bzw. Sampleende und liefert in
 * \p offset den Dateiversatz des ersten Frames. Ein Rückgabewert von 0 bedeutet
 * "nichts zu lesen". Die Prüfungen stehen hier und nicht in den vier Lesemethoden,
 * weil sie für alle Zielformate völlig gleich sind.
 */
size_t AudioDecoder_Aiff::prepareRead(size_t num, const void* buffer, uint64_t& offset) const
{
    offset = 0;
    if (!ff) throw DecoderException("No file opened");
    if (!buffer) throw DecoderException("Buffer is null");
    if (num == 0) return 0;
    if (position >= info.Samples) return 0;

    size_t samples = num;
    if (position + samples > info.Samples) samples = info.Samples - position;
    samples = ClampToAudioSize(info, position, samples);

    // Die von map() gelesene Laenge darf nicht ueber das Dateiende hinauslaufen, sonst
    // wirft map() eine OverflowException. Noetig wird das nur bei einem vom Aufrufer
    // gelieferten AudioInfo, das eine zu grosse AudioSize nennt
    offset = (uint64_t)info.AudioStart + (uint64_t)position * info.BytesPerSample;
    if (offset >= (uint64_t)ff->size()) return 0;
    if (samples > (size_t)(((uint64_t)ff->size() - offset) / info.BytesPerSample))
        samples = (size_t)(((uint64_t)ff->size() - offset) / info.BytesPerSample);
    return samples;
}

size_t AudioDecoder_Aiff::getSamples(size_t num, STEREOSAMPLE16* buffer)
{
    uint64_t offset;
    size_t samples = prepareRead(num, buffer, offset);
    if (samples == 0) return 0;

    const char* data = ff->map(offset, samples * info.BytesPerSample);
    if (!data) throw DecoderException("Mapping audio data failed");

    const size_t frameBytes = info.BytesPerSample;
    const size_t chanBytes = bytesPerChannel;
    const bool stereo = info.Channels == 2;

    switch (encoding) {
        case FrameEncoding::INT16:
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)PeekN16(data);
                int16_t r = stereo ? (int16_t)PeekN16(data + chanBytes) : l;
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
            break;
        case FrameEncoding::INT24:
            // 24 Bit Zweierkomplement in 3 Bytes, ohne Rundung auf 16 Bit gekuerzt
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)(SignExtend24(PeekN24(data)) >> 8);
                int16_t r = stereo ? (int16_t)(SignExtend24(PeekN24(data + chanBytes)) >> 8) : l;
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
            break;
        case FrameEncoding::INT32:
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)((int32_t)PeekN32(data) >> 16);
                int16_t r = stereo ? (int16_t)((int32_t)PeekN32(data + chanBytes) >> 16) : l;
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
            break;
        case FrameEncoding::FLOAT32:
            // AIFF-C Float-Samples sind normiert (-1.0 .. 1.0), deshalb Skala 32768
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)ScaleFloatToSample16(PeekFloatBE(data));
                int16_t r = stereo ? (int16_t)ScaleFloatToSample16(PeekFloatBE(data + chanBytes)) : l;
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
            break;
        default:
            // FrameEncoding::NONE laesst open() gar nicht erst zu den Lesemethoden kommen
            break;
    }
    position += samples;
    return samples;
}

size_t AudioDecoder_Aiff::addSamples(size_t num, STEREOSAMPLE32* buffer)
{
    uint64_t offset;
    size_t samples = prepareRead(num, buffer, offset);
    if (samples == 0) return 0;

    const char* data = ff->map(offset, samples * info.BytesPerSample);
    if (!data) throw DecoderException("Mapping audio data failed");

    const size_t frameBytes = info.BytesPerSample;
    const size_t chanBytes = bytesPerChannel;
    const bool stereo = info.Channels == 2;

    // Der Mixpuffer nimmt durchgehend Werte im 16 Bit Bereich auf, damit Quellen
    // unterschiedlicher Bitbreite ohne weitere Skalierung aufaddiert werden koennen.
    // Abgeschnitten wird ohne Rundung: Uebersteuerungsprodukte entstehen erst durch das
    // Aufaddieren und werden am Mixbus-Ausgang begrenzt
    switch (encoding) {
        case FrameEncoding::INT16:
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)PeekN16(data);
                int16_t r = stereo ? (int16_t)PeekN16(data + chanBytes) : l;
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        case FrameEncoding::INT24:
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)(SignExtend24(PeekN24(data)) >> 8);
                int16_t r = stereo ? (int16_t)(SignExtend24(PeekN24(data + chanBytes)) >> 8) : l;
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        case FrameEncoding::INT32:
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)((int32_t)PeekN32(data) >> 16);
                int16_t r = stereo ? (int16_t)((int32_t)PeekN32(data + chanBytes) >> 16) : l;
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        case FrameEncoding::FLOAT32:
            for (size_t i = 0; i < samples; i++) {
                int16_t l = (int16_t)ScaleFloatToSample16(PeekFloatBE(data));
                int16_t r = stereo ? (int16_t)ScaleFloatToSample16(PeekFloatBE(data + chanBytes)) : l;
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        default:
            break;
    }
    position += samples;
    return samples;
}

size_t AudioDecoder_Aiff::getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer)
{
    uint64_t offset;
    size_t samples = prepareRead(num, buffer, offset);
    if (samples == 0) return 0;

    const char* data = ff->map(offset, samples * info.BytesPerSample);
    if (!data) throw DecoderException("Mapping audio data failed");

    const size_t frameBytes = info.BytesPerSample;
    const size_t chanBytes = bytesPerChannel;
    const bool stereo = info.Channels == 2;

    switch (encoding) {
        case FrameEncoding::INT16:
            for (size_t i = 0; i < samples; i++) {
                float l = (int16_t)PeekN16(data) / 32768.0f;
                float r = stereo ? (int16_t)PeekN16(data + chanBytes) / 32768.0f : l;
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
            break;
        case FrameEncoding::INT24:
            for (size_t i = 0; i < samples; i++) {
                float l = (float)SignExtend24(PeekN24(data)) / 8388608.0f;
                float r = stereo ? (float)SignExtend24(PeekN24(data + chanBytes)) / 8388608.0f : l;
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
            break;
        case FrameEncoding::INT32:
            for (size_t i = 0; i < samples; i++) {
                float l = (float)(int32_t)PeekN32(data) / FULL_SCALE_F;
                float r = stereo ? (float)(int32_t)PeekN32(data + chanBytes) / FULL_SCALE_F : l;
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
            break;
        case FrameEncoding::FLOAT32:
            // Die Werte liegen bereits als normierte Floats in der Datei
            for (size_t i = 0; i < samples; i++) {
                float l = PeekFloatBE(data);
                float r = stereo ? PeekFloatBE(data + chanBytes) : l;
                buffer[i].left = l;
                buffer[i].right = r;
                data += frameBytes;
            }
            break;
        default:
            break;
    }
    position += samples;
    return samples;
}

size_t AudioDecoder_Aiff::addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer)
{
    uint64_t offset;
    size_t samples = prepareRead(num, buffer, offset);
    if (samples == 0) return 0;

    const char* data = ff->map(offset, samples * info.BytesPerSample);
    if (!data) throw DecoderException("Mapping audio data failed");

    const size_t frameBytes = info.BytesPerSample;
    const size_t chanBytes = bytesPerChannel;
    const bool stereo = info.Channels == 2;

    switch (encoding) {
        case FrameEncoding::INT16:
            for (size_t i = 0; i < samples; i++) {
                float l = (int16_t)PeekN16(data) / 32768.0f;
                float r = stereo ? (int16_t)PeekN16(data + chanBytes) / 32768.0f : l;
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        case FrameEncoding::INT24:
            for (size_t i = 0; i < samples; i++) {
                float l = (float)SignExtend24(PeekN24(data)) / 8388608.0f;
                float r = stereo ? (float)SignExtend24(PeekN24(data + chanBytes)) / 8388608.0f : l;
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        case FrameEncoding::INT32:
            for (size_t i = 0; i < samples; i++) {
                float l = (float)(int32_t)PeekN32(data) / FULL_SCALE_F;
                float r = stereo ? (float)(int32_t)PeekN32(data + chanBytes) / FULL_SCALE_F : l;
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        case FrameEncoding::FLOAT32:
            for (size_t i = 0; i < samples; i++) {
                float l = PeekFloatBE(data);
                float r = stereo ? PeekFloatBE(data + chanBytes) : l;
                buffer[i].left += l;
                buffer[i].right += r;
                data += frameBytes;
            }
            break;
        default:
            break;
    }
    position += samples;
    return samples;
}

} // namespace pplib
