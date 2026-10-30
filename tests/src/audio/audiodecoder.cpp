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

#include <gtest/gtest.h>

#include <cstring>
#include <vector>

#include <pplib/core/memfile.h>
#include <pplib/core/functions.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/decoder.h>
#include <pplib/audio/sample_formats.h>

namespace
{

using AF = pplib::AudioInfo::AudioFormat;

//!\brief Kleiner Byte-Builder fuer Container mit bekanntem Sample-Inhalt
static void putByte(std::vector<char>& v, char c)
{
    v.push_back(c);
}
static void putTag(std::vector<char>& v, const char* tag)
{
    for (int i = 0; i < 4; i++) v.push_back(tag[i]);
}
static void put16be(std::vector<char>& v, uint16_t val)
{
    v.push_back((char)(val >> 8));
    v.push_back((char)(val & 0xFF));
}
static void put32be(std::vector<char>& v, uint32_t val)
{
    v.push_back((char)((val >> 24) & 0xFF));
    v.push_back((char)((val >> 16) & 0xFF));
    v.push_back((char)((val >> 8) & 0xFF));
    v.push_back((char)(val & 0xFF));
}
static void put16le(std::vector<char>& v, uint16_t val)
{
    v.push_back((char)(val & 0xFF));
    v.push_back((char)(val >> 8));
}
static void put32le(std::vector<char>& v, uint32_t val)
{
    v.push_back((char)(val & 0xFF));
    v.push_back((char)((val >> 8) & 0xFF));
    v.push_back((char)((val >> 16) & 0xFF));
    v.push_back((char)((val >> 24) & 0xFF));
}

/** @brief Acht Byte 80-Bit-Extended (AIFF) fuer eine ganzzahlige Frequenz ohne Nachkommaanteil */
static void putExtended80(std::vector<char>& v, uint32_t rate)
{
    uint64_t mantissa = (uint64_t)rate << 48;
    put16be(v, 16383 + 15);
    for (int i = 7; i >= 0; i--) putByte(v, (char)(mantissa >> (i * 8)));
}

/** @brief Minimaler AIFF-Container (FORM/AIFF + COMM + SSND) mit vorgegebenem Sample-Inhalt
 *
 * @param channels  Zahl der Kanaele
 * @param bitdepth  Bits pro Kanal-Sample
 * @param ssndSize  deklarierte SSND-Groesse (darf groesser als der real folgende Inhalt sein)
 * @param samples   in COMM deklarierte Samplezahl (darf ueber der real folgenden Payload liegen)
 * @param payload   audioBytes aufeinanderfolgende Sample-Bytes
 */
static std::vector<char> MakeAIFF16(uint16_t channels, uint16_t bitdepth, uint32_t ssndSize, uint32_t samples,
                                    const std::vector<char>& payload)
{
    std::vector<char> body;
    putTag(body, "COMM");
    put32be(body, 18);
    put16be(body, channels);
    put32be(body, samples);
    put16be(body, bitdepth);
    putExtended80(body, 44100);

    putTag(body, "SSND");
    put32be(body, ssndSize);
    put32be(body, 0);  // offset
    put32be(body, 0);  // blockSize
    for (char c : payload) putByte(body, c);

    std::vector<char> v;
    putTag(v, "FORM");
    put32be(v, (uint32_t)(4 + body.size()));
    putTag(v, "AIFF");
    v.insert(v.end(), body.begin(), body.end());
    return v;
}

/** @brief Minimaler RIFF/WAVE-Container (fmt + data) mit vorgegebenem Sample-Inhalt */
static std::vector<char> MakeWave(uint32_t dataSize, size_t audioBytes, uint16_t channels = 2, uint16_t bitdepth = 16,
                                  uint32_t rate = 44100, const std::vector<char>* payload = nullptr)
{
    std::vector<char> body;
    putTag(body, "fmt ");
    put32le(body, 16);
    put16le(body, 1);  // PCM
    put16le(body, channels);
    put32le(body, rate);
    put32le(body, rate * channels * (bitdepth / 8));
    put16le(body, (uint16_t)((bitdepth / 8) * channels));
    put16le(body, bitdepth);

    putTag(body, "data");
    put32le(body, dataSize);
    if (payload) {
        for (char c : *payload) putByte(body, c);
    } else {
        for (size_t i = 0; i < audioBytes; i++) putByte(body, (char)(i & 0xFF));
    }

    std::vector<char> v;
    putTag(v, "RIFF");
    put32le(v, (uint32_t)(body.size() + 4));
    putTag(v, "WAVE");
    v.insert(v.end(), body.begin(), body.end());
    return v;
}

//!\brief Sample-Werte als Big-Endian-Bytes (AIFF) anhaengen
static void appendBE(std::vector<char>& payload, uint32_t value, int bytes)
{
    for (int i = bytes - 1; i >= 0; i--) putByte(payload, (char)((value >> (i * 8)) & 0xFF));
}

//!\brief Sample-Werte als Little-Endian-Bytes (WAV) anhaengen
static void appendLE(std::vector<char>& payload, uint16_t value)
{
    putByte(payload, (char)(value & 0xFF));
    putByte(payload, (char)((value >> 8) & 0xFF));
}

}  // namespace

//!\brief Basisklasse mit Fixture, damit die Dekoder-Calls identisch bleiben
class AudioDecoderTest : public ::testing::Test
{
protected:
    /** @brief Oeffnet einen Speichercontainer mit dem passenden Dekoder.
     *
     *  Der Aufrufer besitzt den Dekoder nicht - er wird hier direkt getestet.
     */
    static void OpenWave(pplib::MemFile& file, pplib::AudioDecoder_Wave& decoder)
    {
        ASSERT_NO_THROW(decoder.open(file));
    }
};

TEST_F(AudioDecoderTest, WaveGetSamples16RoundTrip)
{
    // 4 Frames Stereo/16 bit, Werte vorzeichenbehaftet
    std::vector<char> payload;                        // 4 Frames a 2 Kanael a 16 Bit
    appendLE(payload, 0x1111);
    appendLE(payload, 0x2222);
    appendLE(payload, (uint16_t)0xFFFF);  // -1
    appendLE(payload, 0x8000);            // INT16_MIN
    appendLE(payload, 0x3333);
    appendLE(payload, 0x4444);
    appendLE(payload, 0x5555);
    appendLE(payload, 0x6666);
    std::vector<char> wave = MakeWave((uint32_t)payload.size(), payload.size(), 2, 16, 44100, &payload);

    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    ASSERT_NO_THROW(decoder.open(file));
    EXPECT_EQ((size_t)4, decoder.getAudioInfo().Samples);
    EXPECT_EQ((uint64_t)4, decoder.getAudioInfo().BytesPerSample);

    pplib::STEREOSAMPLE16 buf[8];
    EXPECT_EQ((size_t)4, decoder.getSamples(8, buf));
    EXPECT_EQ((int16_t)0x1111, buf[0].left);
    EXPECT_EQ((int16_t)0x2222, buf[0].right);
    EXPECT_EQ((int16_t)-1, buf[1].left);
    EXPECT_EQ((int16_t)-32768, buf[1].right);
    EXPECT_EQ((int16_t)0x3333, buf[2].left);
    EXPECT_EQ((int16_t)0x6666, buf[3].right);
    EXPECT_EQ((size_t)4, decoder.getPosition());
    EXPECT_EQ((size_t)0, decoder.getSamples(4, buf)) << "nach dem letzten Frame liefert der Dekoder 0";
}

TEST_F(AudioDecoderTest, WaveFloatAndAddSamplesMatchIntegerPath)
{
    std::vector<char> payload;
    appendLE(payload, 0x1111);
    appendLE(payload, 0x2222);
    appendLE(payload, (uint16_t)0x8000);  // INT16_MIN
    appendLE(payload, 0);
    std::vector<char> wave = MakeWave((uint32_t)payload.size(), payload.size(), 2, 16, 44100, &payload);

    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    ASSERT_NO_THROW(decoder.open(file));

    pplib::STEREOSAMPLE_FLOAT fbuf[4];
    EXPECT_EQ((size_t)2, decoder.getSamples(4, fbuf));
    EXPECT_FLOAT_EQ(0x1111 / 32768.0f, fbuf[0].left);
    EXPECT_FLOAT_EQ(-1.0f, fbuf[1].left);

    decoder.seekSample(0);
    pplib::STEREOSAMPLE32 ibuf[4] = {};
    EXPECT_EQ((size_t)2, decoder.addSamples(4, ibuf));
    EXPECT_EQ(0x1111, ibuf[0].left);
    EXPECT_EQ((int32_t)0x8000, ibuf[1].left);
}

TEST_F(AudioDecoderTest, WaveEmptyPayloadOpenThrows)
{
    // data-Payload ohne abspielbare Samples: IdentAudioFile lehnt ab, open() muss werfen
    std::vector<char> wave = MakeWave(0, 0);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    EXPECT_THROW(decoder.open(file), pplib::UnsupportedAudioFormatException);
}

TEST_F(AudioDecoderTest, WaveHostileAudioInfoIsClampedToPayload)
{
    std::vector<char> payload;
    for (int i = 0; i < 4; i++) {
        appendLE(payload, (uint16_t)(0x1000 + i));
        appendLE(payload, (uint16_t)(0x2000 + i));
    }
    std::vector<char> wave = MakeWave((uint32_t)payload.size(), payload.size(), 2, 16, 44100, &payload);
    pplib::MemFile file(wave.data(), wave.size());

    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    info.Samples = 100000;  // deklariert weit mehr Frames, als die Payload hergibt

    pplib::AudioDecoder_Wave decoder;
    ASSERT_NO_THROW(decoder.open(file, &info));
    pplib::STEREOSAMPLE16 buf[16];
    EXPECT_EQ((size_t)4, decoder.getSamples(16, buf)) << "nur die real folgenden Frames lesen";
    EXPECT_EQ((size_t)4, decoder.getPosition());
    EXPECT_EQ((size_t)0, decoder.getSamples(16, buf));
}

TEST_F(AudioDecoderTest, Wave20BitMonoIsPublishedWithThreeBytesPerFrame)
{
    // 20 bit mono hat 3 Bytes pro Frame; der Wave-Dekoder lehnt es trotzdem ab (nur 16 bit stereo)
    std::vector<char> wave = MakeWave(12, 12, 1, 20, 44100);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint64_t)3, info.BytesPerSample);
    EXPECT_EQ((uint64_t)4, info.Samples);

    pplib::AudioDecoder_Wave decoder;
    EXPECT_THROW(decoder.open(file), pplib::UnsupportedAudioFormatException);
}

TEST_F(AudioDecoderTest, Aiff16BitStereoRoundTrip)
{
    std::vector<char> payload;
    appendBE(payload, 0x1234, 2);
    appendBE(payload, 0x4321, 2);
    appendBE(payload, 0x8000, 2);  // INT16_MIN
    appendBE(payload, 0x0001, 2);
    std::vector<char> aiff = MakeAIFF16(2, 16, 8 + (uint32_t)payload.size(), 2, payload);

    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file));
    EXPECT_EQ((size_t)2, decoder.getAudioInfo().Samples);

    pplib::STEREOSAMPLE16 buf[8];
    EXPECT_EQ((size_t)2, decoder.getSamples(8, buf));
    EXPECT_EQ((int16_t)0x1234, buf[0].left);
    EXPECT_EQ((int16_t)0x4321, buf[0].right);
    EXPECT_EQ((int16_t)-32768, buf[1].left);
    EXPECT_EQ((int16_t)1, buf[1].right);
    EXPECT_EQ((size_t)0, decoder.getSamples(8, buf)) << "COMM deklariert 2 Frames, mehr liefert die Payload nicht";
}

TEST_F(AudioDecoderTest, Aiff20BitStereoIsRejectedByOpenGuard)
{
    std::vector<char> payload;
    for (int i = 0; i < 4; i++) {
        appendBE(payload, 0x010203, 3);     // linkes Kanal-Sample auf 3 Bytes aufgefuellt
        appendBE(payload, 0x040506, 3);     // rechtes Kanal-Sample
    }
    // 20 bit stereo: pro Kanal auf volle Bytes aufgerundet, also 2 x 3 = 6 Bytes pro Frame
    std::vector<char> aiff = MakeAIFF16(2, 20, 8 + (uint32_t)payload.size(), 6, payload);

    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioInfo info;
    ASSERT_NO_THROW(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint64_t)6, info.BytesPerSample) << "BytesPerSample pro Frame, nicht pro Kanal";
    EXPECT_EQ((uint64_t)4, info.Samples) << "COMM deklariert 6 Frames, Payload liefert 4";

    pplib::AudioDecoder_Aiff decoder;
    EXPECT_THROW(decoder.open(file), pplib::UnsupportedAudioFormatException) << "20 bit ist kein Dekoderformat";
}

TEST_F(AudioDecoderTest, Aiff24BitDownmixSignExtended)
{
    std::vector<char> payload;
    appendBE(payload, 0x010203, 3);
    appendBE(payload, 0x040506, 3);
    appendBE(payload, 0xFFFFFF, 3);  // -1
    appendBE(payload, 0x800000, 3);  // groesster negativer 24-bit-Wert
    uint32_t frames = 2;
    std::vector<char> aiff = MakeAIFF16(2, 24, 8 + (uint32_t)payload.size(), frames, payload);

    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file));

    pplib::STEREOSAMPLE16 buf[4];
    EXPECT_EQ((size_t)2, decoder.getSamples(4, buf));
    EXPECT_EQ((int16_t)0x0102, buf[0].left);
    EXPECT_EQ((int16_t)0x0405, buf[0].right);
    EXPECT_EQ((int16_t)-1, buf[1].left);
    EXPECT_EQ((int16_t)0x8000, buf[1].right) << "24-bit-Vorzeichen muss auf 16 bit erhalten bleiben";
}

TEST_F(AudioDecoderTest, AiffOverstatedSampleCountDecodesOnlyRealFrames)
{
    // COMM deklariert 100 Frames, SSND deklariert 1 MB, real folgen nur 4 Frames
    std::vector<char> payload;
    for (uint32_t frame = 0; frame < 4; frame++) {
        appendBE(payload, 0x1000 + frame, 2);
        appendBE(payload, 0x2000 + frame, 2);
    }
    std::vector<char> aiff = MakeAIFF16(2, 16, 1024 * 1024, 100, payload);

    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((size_t)4, info.Samples) << "Parser klemmt COMM auf die reale Payload-Groesse ein";

    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file, &info));
    EXPECT_EQ((size_t)4, decoder.getAudioInfo().Samples);

    pplib::STEREOSAMPLE16 buf[16];
    EXPECT_EQ((size_t)4, decoder.getSamples(16, buf));
    EXPECT_EQ((size_t)4, decoder.getPosition());
    EXPECT_EQ((size_t)0, decoder.getSamples(16, buf)) << "Position hinter dem letzten Frame liefert 0";
}

TEST_F(AudioDecoderTest, AiffHostileAudioInfoIsClampedToPayload)
{
    std::vector<char> payload;
    for (uint32_t frame = 0; frame < 4; frame++) {
        appendBE(payload, 0x1000 + frame, 2);
        appendBE(payload, 0x2000 + frame, 2);
    }
    std::vector<char> aiff = MakeAIFF16(2, 16, 8 + (uint32_t)payload.size(), 4, payload);
    pplib::MemFile file(aiff.data(), aiff.size());

    // Absichtlich widerspruechliches AudioInfo: Samples viel groesser als AudioSize/BytesPerSample
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    info.Samples = 1000000;

    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file, &info));
    pplib::STEREOSAMPLE16 buf[16];
    EXPECT_EQ((size_t)4, decoder.getSamples(16, buf)) << "Dekoder liest nie ueber AudioEnd hinaus";
    EXPECT_EQ((size_t)4, decoder.getPosition());
    EXPECT_EQ((size_t)0, decoder.getSamples(16, buf));
}
