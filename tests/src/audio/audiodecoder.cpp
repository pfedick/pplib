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
#include <limits>
#include <vector>

#include <pplib/core/memfile.h>
#include <pplib/core/functions.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/decoder.h>
#include <pplib/core/file.h>
#include <string>
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

    /** @brief Oeffnet echte Testdateien unter tests/testdata/audio, ohne Dekoder */
    static void OpenFileOnly(pplib::File& file, const char* name)
    {
        const std::string path = std::string("testdata/audio/") + name;
        ASSERT_NO_THROW(file.open(path.c_str())) << "open() warf fuer " << path;
        ASSERT_TRUE(file.isOpen()) << "Datei ist nicht geoeffnet: " << path;
    }

    /** @brief Oeffnet echte Testdateien unter tests/testdata/audio mit dem AIFF-Dekoder */
    static void OpenAIFF(pplib::File& file, pplib::AudioDecoder_Aiff& decoder, const char* name)
    {
        OpenFileOnly(file, name);
        ASSERT_NO_THROW(decoder.open(file)) << name;
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
    decoder.seekSample(9);
    EXPECT_EQ((size_t)4, decoder.getPosition()) << "seekSample jenseits des letzten Frames klemmt an Samples";
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

TEST_F(AudioDecoderTest, AiffSixteenBitFloatPathDividesBySignedScale)
{
    std::vector<char> payload;
    appendBE(payload, 0x1234, 2);
    appendBE(payload, 0x8000, 2);  // INT16_MIN -> -1.0f
    appendBE(payload, 0x7FFF, 2);  // knapp unter der positiven Vollskaala
    appendBE(payload, 0x0001, 2);
    std::vector<char> aiff = MakeAIFF16(2, 16, 8 + (uint32_t)payload.size(), 2, payload);

    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file));

    pplib::STEREOSAMPLE_FLOAT buf[4];
    EXPECT_EQ((size_t)2, decoder.getSamples(4, buf));
    EXPECT_FLOAT_EQ(0x1234 / 32768.0f, buf[0].left);
    EXPECT_FLOAT_EQ(-1.0f, buf[0].right);
    EXPECT_FLOAT_EQ(32767 / 32768.0f, buf[1].left);
    EXPECT_FLOAT_EQ(1 / 32768.0f, buf[1].right);

    // Der Float-Mischpuffer wird aufaddiert, nicht ersetzt. Der 16 Bit Wert muss
    // dieselbe normierte Ebene wie oben erreichen
    pplib::STEREOSAMPLE_FLOAT mix[4];
    for (size_t i = 0; i < 4; i++) { mix[i].left = 1.0f; mix[i].right = -1.0f; }
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.addSamples(4, mix));
    EXPECT_FLOAT_EQ(1.0f + 0x1234 / 32768.0f, mix[0].left);
    EXPECT_FLOAT_EQ(-1.0f + 1 / 32768.0f, mix[1].right);
}

TEST_F(AudioDecoderTest, AiffTwentyFourBitMixBufferUsesSixteenBitLevel)
{
    std::vector<char> payload;
    appendBE(payload, 0x010203, 3);
    appendBE(payload, 0x040506, 3);
    appendBE(payload, 0x800000, 3);  // -8388608
    appendBE(payload, 0xFFFFFF, 3);  // -1
    std::vector<char> aiff = MakeAIFF16(2, 24, 8 + (uint32_t)payload.size(), 2, payload);

    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file));

    pplib::STEREOSAMPLE32 mix[4] = {};
    EXPECT_EQ((size_t)2, decoder.addSamples(4, mix));
    EXPECT_EQ(0x0102, mix[0].left);
    EXPECT_EQ(0x0405, mix[0].right);
    EXPECT_EQ((int32_t)-32768, mix[1].left);
    EXPECT_EQ((int32_t)-1, mix[1].right) << "24 Bit darf nicht mehr in der Vollskaala des Mischpuffers landen";

    pplib::STEREOSAMPLE_FLOAT fbuf[4];
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.getSamples(4, fbuf));
    EXPECT_FLOAT_EQ(0x010203 / 8388608.0f, fbuf[0].left);
    EXPECT_FLOAT_EQ(-1.0f, fbuf[1].left);
}

TEST_F(AudioDecoderTest, AiffThirtyTwoBitIntegerDownscaled)
{
    std::vector<char> payload;
    appendBE(payload, 0x12340000, 4);
    appendBE(payload, 0x7FFFFFFF, 4);
    appendBE(payload, 0x80000000, 4);  // INT32_MIN -> INT16_MIN
    appendBE(payload, 0xFFFFFFFF, 4);  // -1 -> -1
    std::vector<char> aiff = MakeAIFF16(2, 32, 8 + (uint32_t)payload.size(), 2, payload);

    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file));
    EXPECT_EQ((uint64_t)8, decoder.getAudioInfo().BytesPerSample);

    pplib::STEREOSAMPLE16 buf[4];
    EXPECT_EQ((size_t)2, decoder.getSamples(4, buf));
    EXPECT_EQ((int16_t)0x1234, buf[0].left);
    EXPECT_EQ((int16_t)0x7FFF, buf[0].right);
    EXPECT_EQ((int16_t)-32768, buf[1].left);
    EXPECT_EQ((int16_t)-1, buf[1].right);

    pplib::STEREOSAMPLE_FLOAT fbuf[4];
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.getSamples(4, fbuf));
    EXPECT_FLOAT_EQ(0x1234 / 32768.0f, fbuf[0].left);
    EXPECT_FLOAT_EQ(-1.0f, fbuf[1].left) << "INT32_MIN/2^31 ist exakt -1.0f";

    pplib::STEREOSAMPLE32 mix32[4];
    for (pplib::STEREOSAMPLE32& m : mix32) { m.left = 1; m.right = -1; }
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.addSamples(4, mix32));
    EXPECT_EQ(0x1235, mix32[0].left) << "32 Bit muessen im Mischpuffer das 16 Bit Niveau halten";
    EXPECT_EQ(0x7FFE, mix32[0].right);
    EXPECT_EQ((int32_t)-32767, mix32[1].left);
    EXPECT_EQ((int32_t)-2, mix32[1].right);
}

TEST_F(AudioDecoderTest, AiffTwentyFourBitSumsIntoFloatMixBuffer)
{
    std::vector<char> payload;
    appendBE(payload, 0x010203, 3);
    appendBE(payload, 0x800000, 3);  // -8388608
    appendBE(payload, 0xFFFFFF, 3);  // -1
    appendBE(payload, 0x008000, 3);
    std::vector<char> aiff = MakeAIFF16(2, 24, 8 + (uint32_t)payload.size(), 2, payload);

    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file));

    pplib::STEREOSAMPLE_FLOAT mixf[4];
    for (pplib::STEREOSAMPLE_FLOAT& m : mixf) { m.left = 0.25f; m.right = -0.25f; }
    EXPECT_EQ((size_t)2, decoder.addSamples(4, mixf));
    EXPECT_FLOAT_EQ(0.25f + 0x010203 / 8388608.0f, mixf[0].left) << "24 Bit wird auf Normalskala aufaddiert";
    EXPECT_FLOAT_EQ(-1.25f, mixf[0].right);
    EXPECT_FLOAT_EQ(0.25f - 1.0f / 8388608.0f, mixf[1].left) << "-1 in 24 Bit ist fast nichts";
    EXPECT_FLOAT_EQ(-0.25f + 0x008000 / 8388608.0f, mixf[1].right);
}

TEST_F(AudioDecoderTest, AiffThirtyTwoBitSumsIntoFloatMixBuffer)
{
    std::vector<char> payload;
    appendBE(payload, 0x40000000, 4);   // +0.5
    appendBE(payload, 0xC0000000, 4);   // -0.5
    appendBE(payload, 0xFFFFFFFF, 4);   // -1
    appendBE(payload, 0x7FFFFFFF, 4);   // fast +1
    std::vector<char> aiff = MakeAIFF16(2, 32, 8 + (uint32_t)payload.size(), 2, payload);

    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file));

    pplib::STEREOSAMPLE_FLOAT mixf[4];
    for (pplib::STEREOSAMPLE_FLOAT& m : mixf) { m.left = 0.25f; m.right = -0.25f; }
    EXPECT_EQ((size_t)2, decoder.addSamples(4, mixf));
    EXPECT_FLOAT_EQ(0.75f, mixf[0].left) << "32 Bit wird ueber 2^31 auf Normalskala aufaddiert";
    EXPECT_FLOAT_EQ(-0.75f, mixf[0].right);
    EXPECT_FLOAT_EQ(0.25f - 1.0f / 2147483648.0f, mixf[1].left);
    EXPECT_FLOAT_EQ(-0.25f + 2147483647.0f / 2147483648.0f, mixf[1].right);
}

TEST_F(AudioDecoderTest, AiffFloatSamplesScaleToAllTargets)
{
    // Offene Float-Dateien enthalten normierte Werte, kein ganzzahliges Sample ist exakt darstellbar
    auto putFloat = [](std::vector<char>& v, float f) {
        uint32_t bits;
        memcpy(&bits, &f, sizeof(bits));
        appendBE(v, bits, 4);
    };
    std::vector<char> payload;
    putFloat(payload, 0.5f);
    putFloat(payload, -0.25f);
    putFloat(payload, 2.0f);    // uebSteuerung muss begrenzt werden
    putFloat(payload, -1.0f);

    // AIFF-C mit FVER und Kompression 'FL32' statt CLASS-COMM
    std::vector<char> body;
    putTag(body, "FVER");
    put32be(body, 4);
    put32be(body, 0xA2805140);
    putTag(body, "COMM");
    put32be(body, 24);  // 18 Bytes plus Kompressionstyp plus leerer PString
    put16be(body, 2);
    put32be(body, 2);
    put16be(body, 32);
    putExtended80(body, 44100);
    putTag(body, "FL32");
    putByte(body, 0);  // PString-Laenge 0
    putByte(body, 0);  // Fill auf even
    putTag(body, "SSND");
    put32be(body, 8 + (uint32_t)payload.size());
    put32be(body, 0);
    put32be(body, 0);
    for (char c : payload) putByte(body, c);
    std::vector<char> aifc;
    putTag(aifc, "FORM");
    put32be(aifc, (uint32_t)(4 + body.size()));
    putTag(aifc, "AIFC");
    aifc.insert(aifc.end(), body.begin(), body.end());

    pplib::MemFile file(aifc.data(), aifc.size());
    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file));
    ASSERT_EQ(pplib::AudioInfo::SampleEncoding::FLOAT, decoder.getAudioInfo().SampleType);

    pplib::STEREOSAMPLE_FLOAT fbuf[4];
    EXPECT_EQ((size_t)2, decoder.getSamples(4, fbuf));
    EXPECT_FLOAT_EQ(0.5f, fbuf[0].left);
    EXPECT_FLOAT_EQ(-0.25f, fbuf[0].right);
    EXPECT_FLOAT_EQ(2.0f, fbuf[1].left) << "Float decodiert offen, ohne Begrenzung";

    pplib::STEREOSAMPLE32 mix[4] = {};
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.addSamples(4, mix));
    EXPECT_EQ(16384, mix[0].left);
    EXPECT_EQ(-8192, mix[0].right);
    EXPECT_EQ(32767, mix[1].left) << "uebSteuerung muss auf den 16 Bit Bereich begrenzt werden";

    pplib::STEREOSAMPLE16 s16[4];
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.getSamples(4, s16));
    EXPECT_EQ((int16_t)16384, s16[0].left);
    EXPECT_EQ((int16_t)-32768, s16[1].right);

    // Additiver Pfad in den Float-Mischpuffer: offene Floatwerte bleiben unveraendert,
    // auch uebersteuerte 2.0f wird nicht begrenzt
    pplib::STEREOSAMPLE_FLOAT mixf[4];
    for (size_t i = 0; i < 4; i++) { mixf[i].left = 0.25f; mixf[i].right = -0.25f; }
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.addSamples(4, mixf));
    EXPECT_FLOAT_EQ(0.75f, mixf[0].left);
    EXPECT_FLOAT_EQ(-0.5f, mixf[0].right);
    EXPECT_FLOAT_EQ(2.25f, mixf[1].left) << "mixen ohne Begrenzung, begrenzt wird erst am Mixbusausgang";
}

TEST_F(AudioDecoderTest, AiffMonoChannelIsReplicated)
{
    std::vector<char> payload;
    appendBE(payload, 0x1234, 2);
    appendBE(payload, 0x8000, 2);
    std::vector<char> aiff = MakeAIFF16(1, 16, 8 + (uint32_t)payload.size(), 2, payload);

    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file));
    ASSERT_EQ((uint64_t)1, decoder.getAudioInfo().Channels);
    ASSERT_EQ((uint64_t)2, decoder.getAudioInfo().BytesPerSample);

    pplib::STEREOSAMPLE16 buf[4];
    EXPECT_EQ((size_t)2, decoder.getSamples(4, buf));
    EXPECT_EQ((int16_t)0x1234, buf[0].left);
    EXPECT_EQ((int16_t)0x1234, buf[0].right);
    EXPECT_EQ((int16_t)-32768, buf[1].left);
    EXPECT_EQ((int16_t)-32768, buf[1].right);

    pplib::STEREOSAMPLE32 mix[4] = {};
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.addSamples(4, mix));
    EXPECT_EQ(0x1234, mix[0].left);
    EXPECT_EQ(0x1234, mix[0].right) << "mono muss im Mischpuffer ebenfalls doppelt stehen";
    EXPECT_EQ((int32_t)-32768, mix[1].left);
    EXPECT_EQ((int32_t)-32768, mix[1].right);

    pplib::STEREOSAMPLE_FLOAT fbuf[4];
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.getSamples(4, fbuf));
    EXPECT_FLOAT_EQ(0x1234 / 32768.0f, fbuf[0].left);
    EXPECT_FLOAT_EQ(0x1234 / 32768.0f, fbuf[0].right) << "mono muss auch im Float-Ziel doppelt stehen";
    EXPECT_FLOAT_EQ(-1.0f, fbuf[1].left);
}

TEST_F(AudioDecoderTest, AiffOpenRejectsSampleFormatesOutsideSupport)
{
    // 8, 12 und 20 Bit werden vom Parser erkannt, aber nicht dekodiert
    const uint16_t depths[] = {8, 12, 20};
    for (uint16_t bitdepth : depths) {
        const size_t channelBytes = (size_t)((bitdepth + 7) / 8);
        std::vector<char> payload;
        for (size_t i = 0; i < 2 * channelBytes * 2; i++) putByte(payload, (char)0x5A);
        std::vector<char> aiff = MakeAIFF16(2, bitdepth, 8 + (uint32_t)payload.size(), 2, payload);

        pplib::MemFile file(aiff.data(), aiff.size());
        pplib::AudioDecoder_Aiff decoder;
        EXPECT_THROW(decoder.open(file), pplib::UnsupportedAudioFormatException) << "BitsPerSample " << bitdepth;
    }

    // Kann der Parser die Datei nicht zuordnen, hilft auch kein AudioInfo: open()
    // muss mit DecoderInitializationException abbrechen statt ein leeres Info zu dekodieren
    std::vector<char> junk;
    for (int i = 0; i < 200; i++) putByte(junk, (char)(i * 7));
    pplib::MemFile foreign(junk.data(), junk.size());
    pplib::AudioDecoder_Aiff blinder;
    EXPECT_THROW(blinder.open(foreign), pplib::DecoderInitializationException);
}

TEST_F(AudioDecoderTest, AiffOpenRejectsFrameSizeNotMatchingDepthAndChannels)
{
    // 24 Bit mit SPEC-Padding (4 Bytes je Kanal) und ein Info mit drei Kanaelen:
    // die Frame-Laenge muss immer Bitbreite x Kanaele sein, sonst werden die Samples
    // an der falschen Stelle gesucht
    std::vector<char> payload;
    for (int i = 0; i < 32; i++) putByte(payload, (char)0x5A);
    std::vector<char> aiff = MakeAIFF16(2, 24, 8 + (uint32_t)payload.size(), 4, payload);
    pplib::MemFile file(aiff.data(), aiff.size());

    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    pplib::AudioDecoder_Aiff padded;
    info.BytesPerSample = 8;
    EXPECT_THROW(padded.open(file, &info), pplib::UnsupportedAudioFormatException) << "24 Bit auf 4 Bytes gepolstert";

    pplib::AudioDecoder_Aiff widened;
    info = pplib::AudioInfo();
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    info.BitsPerSample = 32;
    info.BytesPerSample = 6;
    EXPECT_THROW(widened.open(file, &info), pplib::UnsupportedAudioFormatException) << "32 Bit mit nur 3 Bytes je Kanal";

    pplib::AudioDecoder_Aiff three;
    info = pplib::AudioInfo();
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    info.Channels = 3;
    info.BytesPerSample = 9;
    EXPECT_THROW(three.open(file, &info), pplib::UnsupportedAudioFormatException) << "mehr als zwei Kanaele";
}

TEST_F(AudioDecoderTest, AiffReadWithoutOpenOrAfterCloseThrows)
{
    std::vector<char> payload;
    appendBE(payload, 0x1234, 2);
    appendBE(payload, 0x4321, 2);
    std::vector<char> aiff = MakeAIFF16(2, 16, 8 + (uint32_t)payload.size(), 1, payload);
    pplib::MemFile file(aiff.data(), aiff.size());

    pplib::STEREOSAMPLE16 buf[4];
    pplib::AudioDecoder_Aiff unused;
    EXPECT_THROW(unused.getSamples(1, buf), pplib::DecoderException) << "readDecoded vor open()";

    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file));
    EXPECT_EQ((size_t)1, decoder.getSamples(1, buf));
    decoder.close();
    EXPECT_THROW(decoder.getSamples(1, buf), pplib::DecoderException) << "readDecoded nach close()";
    pplib::STEREOSAMPLE32 mix[4] = {};
    EXPECT_THROW(decoder.addSamples(1, mix), pplib::DecoderException);
    pplib::STEREOSAMPLE_FLOAT fbuf[4];
    EXPECT_THROW(decoder.getSamples(1, fbuf), pplib::DecoderException);
    pplib::AudioDecoder_Aiff offener;
    ASSERT_NO_THROW(offener.open(file));
    EXPECT_THROW(offener.getSamples(4, (pplib::STEREOSAMPLE16*)nullptr), pplib::DecoderException)
        << "ein fehlender Puffer ist kein ignurierbarer Aufruf";
    EXPECT_EQ((uint64_t)0, decoder.getAudioInfo().Samples) << "close() muss das AudioInfo invalidieren";
    EXPECT_EQ((uint64_t)0, decoder.getAudioInfo().AudioStart);
    EXPECT_EQ((uint64_t)0, decoder.getPosition());
}

TEST_F(AudioDecoderTest, AiffHostileAudioSizeBeyondFileIsClampedToFileEnd)
{
    std::vector<char> payload;
    for (uint32_t frame = 0; frame < 4; frame++) {
        appendBE(payload, 0x1000 + frame, 2);
        appendBE(payload, 0x2000 + frame, 2);
    }
    std::vector<char> aiff = MakeAIFF16(2, 16, 8 + (uint32_t)payload.size(), 4, payload);
    pplib::MemFile file(aiff.data(), aiff.size());

    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    info.Samples = 20;                            // luegt mehr Frames als die Datei traegt
    info.AudioSize = (uint64_t)file.size() * 2;  // AudioEnd liegt hinter dem Dateiende

    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file, &info));
    pplib::STEREOSAMPLE16 buf[16];
    EXPECT_NO_THROW(EXPECT_EQ((size_t)4, decoder.getSamples(16, buf))) << "map() darf nicht iber das Dateiende lesen";
    EXPECT_EQ((size_t)0, decoder.getSamples(16, buf));
    EXPECT_EQ((size_t)4, decoder.getPosition());
}

TEST_F(AudioDecoderTest, AiffMonoFloatSourceIsReplicatedAndClipped)
{
    auto putFloat = [](std::vector<char>& v, float f) {
        uint32_t bits;
        memcpy(&bits, &f, sizeof(bits));
        appendBE(v, bits, 4);
    };
    std::vector<char> payload;
    putFloat(payload, 0.5f);
    putFloat(payload, -2.0f);   // unterhalb des 16-Bit-Bereichs
    putFloat(payload, 2.0f);    // oberhalb des 16-Bit-Bereichs
    putFloat(payload, std::numeric_limits<float>::quiet_NaN());

    std::vector<char> body;
    putTag(body, "FVER");
    put32be(body, 4);
    put32be(body, 0xA2805140);
    putTag(body, "COMM");
    put32be(body, 24);
    put16be(body, 1);  // mono
    put32be(body, 4);
    put16be(body, 32);
    putExtended80(body, 44100);
    putTag(body, "FL32");
    putByte(body, 0);
    putByte(body, 0);
    putTag(body, "SSND");
    put32be(body, 8 + (uint32_t)payload.size());
    put32be(body, 0);
    put32be(body, 0);
    for (char c : payload) putByte(body, c);
    std::vector<char> aifc;
    putTag(aifc, "FORM");
    put32be(aifc, (uint32_t)(4 + body.size()));
    putTag(aifc, "AIFC");
    aifc.insert(aifc.end(), body.begin(), body.end());

    pplib::MemFile file(aifc.data(), aifc.size());
    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file));
    ASSERT_EQ((uint64_t)1, decoder.getAudioInfo().Channels);

    pplib::STEREOSAMPLE16 buf[4] = {};
    EXPECT_EQ((size_t)4, decoder.getSamples(4, buf));
    EXPECT_EQ((int16_t)16384, buf[0].left);
    EXPECT_EQ((int16_t)16384, buf[0].right) << "Float-mono muss beide Ausgaenge fuellen";
    EXPECT_EQ((int16_t)-32768, buf[1].left) << "nach unten auf den 16 Bit Bereich begrenzt";
    EXPECT_EQ((int16_t)32767, buf[2].left) << "nach oben auf den 16 Bit Bereich begrenzt";
    EXPECT_EQ((int16_t)0, buf[3].left) << "NaN wird zu 0, damit kein Mischpuffer vergiftet wird";

    pplib::STEREOSAMPLE_FLOAT fbuf[4];
    decoder.seekSample(0);
    EXPECT_EQ((size_t)4, decoder.getSamples(4, fbuf));
    EXPECT_FLOAT_EQ(0.5f, fbuf[0].left);
    EXPECT_FLOAT_EQ(0.5f, fbuf[0].right) << "auch im Float-Ziel wird der mono-Kanal verdoppelt";
    EXPECT_FLOAT_EQ(-2.0f, fbuf[1].left) << "ins Float-Ziel bleibt der Originalwert un begrenzt";
    EXPECT_FLOAT_EQ(2.0f, fbuf[2].left);
    EXPECT_NE(fbuf[3].left, fbuf[3].left) << "NaN wandert ins Float-Ziel unveraendert";

    pplib::STEREOSAMPLE32 mix[4] = {};
    decoder.seekSample(0);
    EXPECT_EQ((size_t)4, decoder.addSamples(4, mix));
    EXPECT_EQ(16384, mix[0].left);
    EXPECT_EQ(16384, mix[0].right);
    EXPECT_EQ((int32_t)-32768, mix[1].right) << "additiver Pfad muss ebenso begrenzen";
    EXPECT_EQ(0, mix[3].left) << "NaN faengt im Mixpuffer mit 0 an";
}

TEST_F(AudioDecoderTest, AiffZeroSampleReadsLeavePositionUnchanged)
{
    std::vector<char> payload;
    appendBE(payload, 0x1234, 2);
    appendBE(payload, 0x4321, 2);
    appendBE(payload, 0x1111, 2);
    appendBE(payload, 0x2222, 2);
    std::vector<char> aiff = MakeAIFF16(2, 16, 8 + (uint32_t)payload.size(), 2, payload);
    pplib::MemFile file(aiff.data(), aiff.size());

    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file));
    pplib::STEREOSAMPLE16 buf[4];
    EXPECT_EQ((size_t)0, decoder.getSamples(0, buf)) << "eine Abfrage von null Samples ist kein Fehler";
    pplib::STEREOSAMPLE_FLOAT fbuf[4];
    EXPECT_EQ((size_t)0, decoder.getSamples(0, fbuf));
    pplib::STEREOSAMPLE32 mix[4] = {};
    EXPECT_EQ((size_t)0, decoder.addSamples(0, mix));
    EXPECT_EQ((size_t)0, decoder.getPosition()) << "ein Leerraumlesen darf die Position nicht veraendern";
}

TEST_F(AudioDecoderTest, AiffAudioInfoOfAnotherFormatIsRejected)
{
    std::vector<char> payload;
    appendBE(payload, 0x1234, 2);
    appendBE(payload, 0x4321, 2);
    std::vector<char> aiff = MakeAIFF16(2, 16, 8 + (uint32_t)payload.size(), 1, payload);
    pplib::MemFile file(aiff.data(), aiff.size());

    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    info.Format = pplib::AudioInfo::AudioFormat::WAVE;

    pplib::AudioDecoder_Aiff decoder;
    EXPECT_THROW(decoder.open(file, &info), pplib::UnsupportedAudioFormatException)
        << "das Format muss auch bei mitgeliefertem AudioInfo geprueft werden";

    info.Format = pplib::AudioInfo::AudioFormat::AIFF;
    info.Channels = 5;
    EXPECT_THROW(decoder.open(file, &info), pplib::UnsupportedAudioFormatException)
        << "mehr als zwei Kanaael koennen nicht auf Stereodaten abgebildet werden";
    info.Channels = 0;
    EXPECT_THROW(decoder.open(file, &info), pplib::UnsupportedAudioFormatException) << "ein Kanal ohne Daten ist kein Ton";
}

TEST_F(AudioDecoderTest, AiffRaisesSmallMapWindowAndKeepsALargerOne)
{
    // Bei einem Vorlesebereich von null wuerde jeder getSamples-Aufruf ein
    // mmap/munmap-Paar ausloesen. Der Dekoder hebt ihn an,verschlechtert aber
    // einen bereits groesseren Wert des Aufrufers nicht
    pplib::File file;
    ASSERT_NO_THROW(OpenFileOnly(file, "test_44kHz.aiff"));
    file.setMapWindowSize(4 * 1024);
    pplib::AudioDecoder_Aiff decoder;
    ASSERT_NO_THROW(decoder.open(file));
    EXPECT_EQ((size_t)(32 * 1024), file.getMapWindowSize()) << "der Mindestbereich muss gesetzt werden";
    pplib::STEREOSAMPLE16 buf[8];
    EXPECT_EQ((size_t)8, decoder.getSamples(8, buf));

    pplib::File gross;
    ASSERT_NO_THROW(OpenFileOnly(gross, "test_44kHz.aiff"));
    gross.setMapWindowSize(64 * 1024);
    pplib::AudioDecoder_Aiff zweiter;
    ASSERT_NO_THROW(zweiter.open(gross));
    EXPECT_EQ((size_t)(64 * 1024), gross.getMapWindowSize()) << "ein groesserer Wert bleibt erhalten";
    EXPECT_EQ((size_t)8, zweiter.getSamples(8, buf));
    gross.close();
    file.close();
}

TEST_F(AudioDecoderTest, AiffRealFixturesDecodeToTheSameSixteenBitValues)
{
    // Die fuenf realen Dateien von Audacity enthalten dieselbe Wellenform in
    // 16 bit stereo, 16 bit mono, 24 bit stereo, 32 bit Integer und 32 bit Float.
    // Alle Dekoderpfade muessen dieselben 16 bit Werte liefern, sofern die
    // Audacity-Export durch das Runden nicht um 1 LSB abweichen
    pplib::File f16, f24, f32, fFl, fMono;
    pplib::AudioDecoder_Aiff d16, d24, d32, dFl, dMono;
    ASSERT_NO_THROW(OpenAIFF(f16, d16, "test_44kHz.aiff"));
    ASSERT_NO_THROW(OpenAIFF(f24, d24, "test_44kHz_24bit.aiff"));
    ASSERT_NO_THROW(OpenAIFF(f32, d32, "test_44kHz_32bit.aiff"));
    ASSERT_NO_THROW(OpenAIFF(fFl, dFl, "test_44kHz_32bit_float.aiff"));
    ASSERT_NO_THROW(OpenAIFF(fMono, dMono, "test_44kHz_16bit_mono.aiff"));
    ASSERT_EQ((uint64_t)2, d16.getAudioInfo().Channels);
    ASSERT_EQ((uint64_t)1, dMono.getAudioInfo().Channels) << "die Mono-Datei muss als mono erkannt werden";

    pplib::STEREOSAMPLE16 a[8], b[8], c[8], m[8];
    pplib::STEREOSAMPLE_FLOAT fl[8];
    ASSERT_EQ((size_t)8, d16.getSamples(8, a));
    ASSERT_EQ((size_t)8, d24.getSamples(8, b));
    ASSERT_EQ((size_t)8, d32.getSamples(8, c));
    ASSERT_EQ((size_t)8, dFl.getSamples(8, fl));
    ASSERT_EQ((size_t)8, dMono.getSamples(8, m));

    for (size_t i = 0; i < 8; i++) {
        EXPECT_NEAR((int)a[i].left, (int)b[i].left, 1) << "24 bit Linkskanal Frame " << i;
        EXPECT_NEAR((int)a[i].right, (int)b[i].right, 1) << "24 bit Rechtskanal Frame " << i;
        EXPECT_NEAR((int)a[i].left, (int)c[i].left, 1) << "32 bit Linkskanal Frame " << i;
        EXPECT_NEAR((int)a[i].right, (int)c[i].right, 1) << "32 bit Rechtskanal Frame " << i;
        EXPECT_NEAR((float)a[i].left / 32768.0f, fl[i].left, 1.0f / 32768.0f) << "Float Linkskanal Frame " << i;
        EXPECT_NEAR((float)a[i].right / 32768.0f, fl[i].right, 1.0f / 32768.0f) << "Float Rechtskanal Frame " << i;
        EXPECT_EQ(m[i].left, m[i].right) << "mono muss beide Ausgaenge fuellen Frame " << i;
        // Die Mono-Datei ist ein Downmix der Stereo-Vorlage, also Mitte aus beiden
        // Kanaelen. Der Vergleich mit den realen Dateien haelt nur bis 4 LSB, weil
        // Audacity beim Export mit Ditherung arbeitet
        EXPECT_NEAR((float)m[i].left, ((float)a[i].left + (float)a[i].right) / 2.0f, 4.0f) << "mono Frame " << i;
    }
}
