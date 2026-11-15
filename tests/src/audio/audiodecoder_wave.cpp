/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLib).
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

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

#include <pplib/audio/audioinfo.h>
#include <pplib/audio/decoder.h>
#include <pplib/audio/sample_formats.h>
#include <pplib/core/file.h>
#include <pplib/core/functions.h>
#include <pplib/core/memfile.h>

namespace
{

//!\brief Kleiner Byte-Builder fuer WAV-Container mit bekanntem Sample-Inhalt
static void putTag(std::vector<char>& v, const char* tag)
{
    for (int i = 0; i < 4; i++)
        v.push_back(tag[i]);
}
static void put16le(std::vector<char>& v, uint16_t val)
{
    v.push_back((char)(val & 0xFF));
    v.push_back((char)((val >> 8) & 0xFF));
}
static void put32le(std::vector<char>& v, uint32_t val)
{
    v.push_back((char)(val & 0xFF));
    v.push_back((char)((val >> 8) & 0xFF));
    v.push_back((char)((val >> 16) & 0xFF));
    v.push_back((char)((val >> 24) & 0xFF));
}

/** @brief Samplewert als Little-Endian-Bytes anhaengen
 *
 *  Der Wert wird niedrigkantig ausgegeben, deshalb koennen vorzeichenbehaftete Werte
 *  als uint32_t uebergeben werden (z.B. 0xFFED18 fuer -5000 in 24 Bit).
 */
static void appendLE(std::vector<char>& payload, uint32_t value, int bytes)
{
    for (int i = 0; i < bytes; i++)
        payload.push_back((char)((value >> (i * 8)) & 0xFF));
}

//!\brief normierten Float-Wert (IEEE-754, 32 Bit) anhaengen
static void appendFloat(std::vector<char>& payload, float value)
{
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    appendLE(payload, bits, 4);
}

/** @brief Minimaler RIFF/WAVE-Container (fmt + data) mit vorgegebenem Sample-Inhalt
 *
 *  Die deklarierten Groessen koennen bewusst ueber der real folgenden Payload liegen,
 *  um abgeschnittene Dateien zu erzeugen.
 *
 * @param dataSize   im data-Chunk deklarierte Groesse
 * @param audioBytes Zahl der Bytes, die ohne payload-Parameter gefuelllt werden
 * @param channels   Zahl der Kanaele
 * @param bitdepth   Bits pro Kanal-Sample
 * @param formatTag  1 = PCM, 3 = IEEE-Float, alles andere lehnt IdentWave ab
 * @param payload    aufeinanderfolgende Sample-Bytes
 */
static std::vector<char> MakeWave(uint32_t dataSize,
                                  size_t audioBytes,
                                  uint16_t channels = 2,
                                  uint16_t bitdepth = 16,
                                  uint32_t rate = 44100,
                                  uint16_t formatTag = 1,
                                  const std::vector<char>* payload = nullptr)
{
    std::vector<char> body;
    putTag(body, "fmt ");
    put32le(body, 16);
    put16le(body, formatTag);
    put16le(body, channels);
    put32le(body, rate);
    put32le(body, rate * channels * (bitdepth / 8));
    put16le(body, (uint16_t)((bitdepth / 8) * channels));
    put16le(body, bitdepth);

    putTag(body, "data");
    put32le(body, dataSize);
    if (payload) {
        body.insert(body.end(), payload->begin(), payload->end());
    } else {
        for (size_t i = 0; i < audioBytes; i++)
            body.push_back((char)(i & 0xFF));
    }

    std::vector<char> v;
    putTag(v, "RIFF");
    put32le(v, (uint32_t)(body.size() + 4));
    putTag(v, "WAVE");
    v.insert(v.end(), body.begin(), body.end());
    return v;
}

} // namespace

//!\brief Basis-Fixture fuer AudioDecoder_Wave
class AudioDecoderWaveTest : public ::testing::Test
{
protected:
    /** @brief Oeffnet einen Speichercontainer mit dem Wave-Dekoder */
    static void OpenWave(pplib::MemFile& file, pplib::AudioDecoder_Wave& decoder)
    {
        ASSERT_NO_THROW(decoder.open(file));
    }

    /** @brief Oeffnet eine echte Testdatei unter tests/testdata/audio */
    static void OpenTestFile(pplib::File& file, const char* name)
    {
        const std::string path = std::string("testdata/audio/") + name;
        ASSERT_NO_THROW(file.open(path.c_str())) << "open() warf fuer " << path;
        ASSERT_TRUE(file.isOpen()) << "Datei ist nicht geoeffnet: " << path;
    }
};

TEST_F(AudioDecoderWaveTest, OpenRejectsUnplayableOrForeignContainer)
{
    // data-Chunk ohne abspielbare Samples
    std::vector<char> wave = MakeWave(0, 0);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    EXPECT_THROW(decoder.open(file), pplib::UnsupportedAudioFormatException);

    // komprimiertes Format: Tag 6 (a-Law) wird nicht erkannt, open() muss werfen
    std::vector<char> law = MakeWave(8, 8, 2, 16, 44100, 6);
    pplib::MemFile lawFile(law.data(), law.size());
    pplib::AudioInfo info;
    EXPECT_FALSE(pplib::IdentAudioFile(lawFile, info)) << "Tag 6 ist kein erkanntes WAV-Format";
    pplib::AudioDecoder_Wave decoder2;
    EXPECT_THROW(decoder2.open(lawFile), pplib::UnsupportedAudioFormatException);

    // EXTENSIBLE (Tag 0xFFFE) wird bewusst nicht unterstuetzt
    std::vector<char> ext = MakeWave(8, 8, 2, 16, 44100, 0xFFFE);
    pplib::MemFile extFile(ext.data(), ext.size());
    EXPECT_THROW(decoder2.open(extFile), pplib::UnsupportedAudioFormatException);
}

TEST_F(AudioDecoderWaveTest, OpenRejectsUnsupportedBitDepths)
{
    // 8 Bit erkannt, aber nicht dekodiert
    std::vector<char> wave8 = MakeWave(8, 8, 2, 8, 44100);
    pplib::MemFile file8(wave8.data(), wave8.size());
    pplib::AudioInfo info8;
    ASSERT_TRUE(pplib::IdentAudioFile(file8, info8));
    EXPECT_EQ((uint64_t)8, info8.BitsPerSample);
    pplib::AudioDecoder_Wave decoder8;
    EXPECT_THROW(decoder8.open(file8), pplib::UnsupportedAudioFormatException);

    // 20 Bit mono hat 3 Bytes pro Frame: erkennbarer Container, kein Dekoder-Format
    std::vector<char> wave20 = MakeWave(12, 12, 1, 20, 44100);
    pplib::MemFile file20(wave20.data(), wave20.size());
    pplib::AudioInfo info20;
    ASSERT_TRUE(pplib::IdentAudioFile(file20, info20));
    EXPECT_EQ((uint64_t)3, info20.BytesPerSample);
    EXPECT_EQ((uint64_t)4, info20.Samples);
    pplib::AudioDecoder_Wave decoder20;
    EXPECT_THROW(decoder20.open(file20), pplib::UnsupportedAudioFormatException);
}

TEST_F(AudioDecoderWaveTest, MonoReplicatesTheOnlyChannel)
{
    std::vector<char> payload;
    appendLE(payload, 0x1111, 2);
    appendLE(payload, 0xFFF7, 2); // -9
    std::vector<char> wave = MakeWave((uint32_t)payload.size(), payload.size(), 1, 16, 44100, 1, &payload);

    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    OpenWave(file, decoder);
    EXPECT_EQ((uint64_t)1, decoder.getAudioInfo().Channels);
    EXPECT_EQ((uint64_t)2, decoder.getAudioInfo().BytesPerSample);
    EXPECT_EQ((uint64_t)2, decoder.getAudioInfo().Samples);

    pplib::STEREOSAMPLE16 buf[4];
    EXPECT_EQ((size_t)2, decoder.getSamples(4, buf));
    EXPECT_EQ((int16_t)0x1111, buf[0].left);
    EXPECT_EQ((int16_t)0x1111, buf[0].right);
    EXPECT_EQ((int16_t)-9, buf[1].left);
    EXPECT_EQ((int16_t)-9, buf[1].right);

    pplib::STEREOSAMPLE_FLOAT fbuf[4];
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.getSamples(4, fbuf));
    EXPECT_FLOAT_EQ(-9 / 32768.0f, fbuf[1].left);
    EXPECT_FLOAT_EQ(fbuf[1].left, fbuf[1].right);
}

TEST_F(AudioDecoderWaveTest, GetSamples16RoundTrip)
{
    // 4 Frames Stereo/16 bit, Werte vorzeichenbehaftet
    std::vector<char> payload;
    appendLE(payload, 0x1111, 2);
    appendLE(payload, 0x2222, 2);
    appendLE(payload, 0xFFFF, 2); // -1
    appendLE(payload, 0x8000, 2); // INT16_MIN
    appendLE(payload, 0x3333, 2);
    appendLE(payload, 0x4444, 2);
    appendLE(payload, 0x5555, 2);
    appendLE(payload, 0x6666, 2);
    std::vector<char> wave = MakeWave((uint32_t)payload.size(), payload.size(), 2, 16, 44100, 1, &payload);

    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    OpenWave(file, decoder);
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

TEST_F(AudioDecoderWaveTest, AddSamples16BitKeepsSignedValues)
{
    // Regressionstest: die Samples wurden vorzeichenlos aufaddiert, jedes negative Sample
    // erschien im Mixpuffer um rund 65536 zu hoch
    std::vector<char> payload;
    appendLE(payload, 0xEC78, 2); // -5000
    appendLE(payload, 0xD8F0, 2); // -10000
    appendLE(payload, 0x7FFF, 2);
    appendLE(payload, 0x8000, 2); // INT16_MIN
    std::vector<char> wave = MakeWave((uint32_t)payload.size(), payload.size(), 2, 16, 44100, 1, &payload);

    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    OpenWave(file, decoder);

    pplib::STEREOSAMPLE16 reference[4] = {};
    EXPECT_EQ((size_t)2, decoder.getSamples(2, reference));

    pplib::STEREOSAMPLE32 mix[4] = {};
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.addSamples(4, mix));
    EXPECT_EQ((int32_t)-5000, mix[0].left);
    EXPECT_EQ((int32_t)-10000, mix[0].right);
    EXPECT_EQ((int32_t)32767, mix[1].left);
    EXPECT_EQ((int32_t)-32768, mix[1].right);
    EXPECT_EQ((int32_t)reference[0].left, mix[0].left);
}

TEST_F(AudioDecoderWaveTest, FloatPathsMatchIntegerPathFor16BitSource)
{
    std::vector<char> payload;
    appendLE(payload, 0x1111, 2);
    appendLE(payload, 0x2222, 2);
    appendLE(payload, 0x8000, 2); // INT16_MIN
    appendLE(payload, 0, 2);
    std::vector<char> wave = MakeWave((uint32_t)payload.size(), payload.size(), 2, 16, 44100, 1, &payload);

    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    OpenWave(file, decoder);

    pplib::STEREOSAMPLE_FLOAT buf[4];
    EXPECT_EQ((size_t)2, decoder.getSamples(4, buf));
    EXPECT_FLOAT_EQ(0x1111 / 32768.0f, buf[0].left);
    EXPECT_FLOAT_EQ(0x2222 / 32768.0f, buf[0].right);
    EXPECT_FLOAT_EQ(-1.0f, buf[1].left);
    EXPECT_FLOAT_EQ(0.0f, buf[1].right);

    pplib::STEREOSAMPLE_FLOAT mix[4] = {};
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.addSamples(4, mix));
    EXPECT_FLOAT_EQ(buf[0].left, mix[0].left);
    EXPECT_FLOAT_EQ(buf[1].left, mix[1].left);

    // zweimal aufaddiert ergibt die doppelte Aussteuerung
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.addSamples(4, mix));
    EXPECT_FLOAT_EQ(2.0f * buf[0].left, mix[0].left);
}

TEST_F(AudioDecoderWaveTest, Int24IsScaledTo16BitMixLevel)
{
    std::vector<char> payload;
    appendLE(payload, 0x008000, 3); // +32768
    appendLE(payload, 0x000100, 3); // +256
    appendLE(payload, 0xFFEC78, 3); // -5000
    appendLE(payload, 0xFFFF00, 3); // -256
    appendLE(payload, 0x800000, 3); // INT24_MIN
    appendLE(payload, 0, 3);
    std::vector<char> wave = MakeWave((uint32_t)payload.size(), payload.size(), 2, 24, 44100, 1, &payload);

    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    OpenWave(file, decoder);
    EXPECT_EQ((uint64_t)24, decoder.getAudioInfo().BitsPerSample);
    EXPECT_EQ((uint64_t)6, decoder.getAudioInfo().BytesPerSample);
    EXPECT_EQ((uint64_t)3, decoder.getAudioInfo().Samples);

    pplib::STEREOSAMPLE16 buf[4];
    EXPECT_EQ((size_t)2, decoder.getSamples(2, buf));
    EXPECT_EQ((int16_t)128, buf[0].left);
    EXPECT_EQ((int16_t)1, buf[0].right);

    pplib::STEREOSAMPLE32 mix[4] = {};
    decoder.seekSample(0);
    EXPECT_EQ((size_t)3, decoder.addSamples(4, mix));
    EXPECT_EQ((int32_t)-20, mix[1].left);    // -5000 >> 8
    EXPECT_EQ((int32_t)-1, mix[1].right);    // -256 >> 8
    EXPECT_EQ((int32_t)-32768, mix[2].left); // INT24_MIN entspricht der 16 Bit Vollskala
    EXPECT_EQ((int32_t)0, mix[2].right);

    // die normierte Float-Ebene nutzt die volle 24-Bit-Breite
    pplib::STEREOSAMPLE_FLOAT fbuf[4];
    decoder.seekSample(0);
    EXPECT_EQ((size_t)3, decoder.getSamples(4, fbuf));
    EXPECT_NEAR(-5000 / 8388608.0, fbuf[1].left, 1e-6);
    EXPECT_FLOAT_EQ(-1.0f, fbuf[2].left);
}

TEST_F(AudioDecoderWaveTest, Int32IsScaledTo16BitMixLevel)
{
    std::vector<char> payload;
    appendLE(payload, 0x01000000, 4); // +16777216
    appendLE(payload, 0x00010000, 4); // +65536
    appendLE(payload, 0xFFFFEC78, 4); // -5000
    appendLE(payload, 0x80000000, 4); // INT32_MIN
    std::vector<char> wave = MakeWave((uint32_t)payload.size(), payload.size(), 2, 32, 44100, 1, &payload);

    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    OpenWave(file, decoder);
    EXPECT_EQ((uint64_t)32, decoder.getAudioInfo().BitsPerSample);
    EXPECT_EQ((uint64_t)8, decoder.getAudioInfo().BytesPerSample);
    EXPECT_EQ((int)(int)pplib::AudioInfo::SampleEncoding::INTEGER, (int)decoder.getAudioInfo().SampleType);

    pplib::STEREOSAMPLE16 buf[4];
    EXPECT_EQ((size_t)1, decoder.getSamples(1, buf));
    EXPECT_EQ((int16_t)256, buf[0].left);
    EXPECT_EQ((int16_t)1, buf[0].right);

    pplib::STEREOSAMPLE32 mix[4] = {};
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.addSamples(4, mix));
    EXPECT_EQ((int32_t)-1, mix[1].left); // -5000 >> 16
    EXPECT_EQ((int32_t)-32768, mix[1].right);

    pplib::STEREOSAMPLE_FLOAT fbuf[4];
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.getSamples(4, fbuf));
    EXPECT_NEAR(-5000 / 2147483648.0, fbuf[1].left, 1e-9);
    EXPECT_FLOAT_EQ(-1.0f, fbuf[1].right);
}

TEST_F(AudioDecoderWaveTest, FloatSamplesUseTheWholeNormalizedRange)
{
    // Format-Tag 3 = IEEE-Float, normiert: Vollskala ist 1.0 statt 32768
    std::vector<char> payload;
    appendFloat(payload, 0.5f);
    appendFloat(payload, -0.25f);
    appendFloat(payload, 1.5f); // uebersteuert
    appendFloat(payload, -1.0f);
    std::vector<char> wave = MakeWave((uint32_t)payload.size(), payload.size(), 2, 32, 44100, 3, &payload);

    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo probe;
    ASSERT_TRUE(pplib::IdentAudioFile(file, probe)) << "Tag 3 muss als WAV erkannt werden";
    ASSERT_EQ((int)pplib::AudioInfo::SampleEncoding::FLOAT, (int)probe.SampleType);

    pplib::AudioDecoder_Wave decoder;
    OpenWave(file, decoder);
    EXPECT_EQ((uint64_t)32, decoder.getAudioInfo().BitsPerSample);
    EXPECT_EQ((int)pplib::AudioInfo::SampleEncoding::FLOAT, (int)decoder.getAudioInfo().SampleType);
    EXPECT_EQ((uint64_t)2, decoder.getAudioInfo().Samples);

    pplib::STEREOSAMPLE16 buf[4];
    EXPECT_EQ((size_t)2, decoder.getSamples(4, buf));
    EXPECT_EQ((int16_t)16384, buf[0].left);
    EXPECT_EQ((int16_t)-8192, buf[0].right);
    EXPECT_EQ((int16_t)32767, buf[1].left) << "Uebersteuerung wird auf PositiveFullScale begrenzt";
    EXPECT_EQ((int16_t)-32768, buf[1].right);

    pplib::STEREOSAMPLE32 mix[4] = {};
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.addSamples(4, mix));
    EXPECT_EQ((int32_t)16384, mix[0].left);
    EXPECT_EQ((int32_t)-32768, mix[1].right);

    // auf der Float-Ebene stehen die Werte unveraendert, auch ueber der Vollskala
    pplib::STEREOSAMPLE_FLOAT fbuf[4];
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.getSamples(4, fbuf));
    EXPECT_FLOAT_EQ(0.5f, fbuf[0].left);
    EXPECT_FLOAT_EQ(-0.25f, fbuf[0].right);
    EXPECT_FLOAT_EQ(1.5f, fbuf[1].left);
    EXPECT_FLOAT_EQ(-1.0f, fbuf[1].right);

    pplib::STEREOSAMPLE_FLOAT fmix[4] = {};
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.addSamples(4, fmix));
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.addSamples(4, fmix));
    EXPECT_FLOAT_EQ(1.0f, fmix[0].left);
    EXPECT_FLOAT_EQ(3.0f, fmix[1].left);
}

TEST_F(AudioDecoderWaveTest, FloatNaNBecomesSilence)
{
    // Byte-Muster fuer NaN, damit es nicht auf die fliest Kommastelle ankommt
    std::vector<char> payload;
    appendLE(payload, 0x7FC00000, 4);
    appendLE(payload, 0, 4);
    appendLE(payload, 0, 4);
    appendLE(payload, 0, 4);
    std::vector<char> wave = MakeWave((uint32_t)payload.size(), payload.size(), 2, 32, 44100, 3, &payload);

    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    OpenWave(file, decoder);

    pplib::STEREOSAMPLE16 buf[2];
    EXPECT_EQ((size_t)2, decoder.getSamples(2, buf));
    EXPECT_EQ((int16_t)0, buf[0].left) << "NaN wird als Stille interpretiert";
    EXPECT_EQ((int16_t)0, buf[0].right);
    EXPECT_EQ((int16_t)0, buf[1].left);
}

TEST_F(AudioDecoderWaveTest, SeekSampleClampsToAudioData)
{
    std::vector<char> wave = MakeWave(1024, 1024, 2, 16, 44100);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    OpenWave(file, decoder);
    const size_t total = (size_t)decoder.getAudioInfo().Samples;
    ASSERT_GT(total, (size_t)4);

    decoder.seekSample(total + 500);
    EXPECT_EQ(total, decoder.getPosition()) << "seekSample() darf nicht ueber das Datenende stellen";
    pplib::STEREOSAMPLE16 buf[8];
    EXPECT_EQ((size_t)0, decoder.getSamples(8, buf));

    // nach seekSample(0) beginnt die Datenmenge wieder von vorn
    pplib::STEREOSAMPLE16 first[4];
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.getSamples(2, first));
    pplib::STEREOSAMPLE16 again[2];
    decoder.seekSample(0);
    EXPECT_EQ((size_t)1, decoder.getSamples(1, again));
    EXPECT_EQ(first[0].left, again[0].left);
    EXPECT_EQ(first[0].right, again[0].right);

    // ohne seekSample() lesen die Aufrufe hintereinander aufeinanderfolgende Frames
    decoder.seekSample(total / 2);
    pplib::STEREOSAMPLE16 middle[8];
    ASSERT_EQ((size_t)8, decoder.getSamples(8, middle));
    pplib::STEREOSAMPLE16 next[8];
    ASSERT_EQ((size_t)8, decoder.getSamples(8, next));
    EXPECT_NE(middle[0].left, next[0].left) << "der zweite Aufruf liegt hinter dem ersten";

    decoder.seekSample(total / 2);
    pplib::STEREOSAMPLE16 repeat[8];
    ASSERT_EQ((size_t)8, decoder.getSamples(8, repeat));
    for (size_t i = 0; i < 8; i++) {
        EXPECT_EQ(middle[i].left, repeat[i].left) << "Frame " << i;
        EXPECT_EQ(middle[i].right, repeat[i].right) << "Frame " << i;
    }
}

TEST_F(AudioDecoderWaveTest, DecodingRequiresAnOpenedFile)
{
    std::vector<char> wave = MakeWave(1024, 1024, 2, 16, 44100);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    pplib::STEREOSAMPLE16 buf[8];

    EXPECT_EQ((size_t)0, decoder.getSamples(8, buf)) << "ohne open() ist Dekodieren 0";
    pplib::STEREOSAMPLE_FLOAT fbuf[8];
    EXPECT_EQ((size_t)0, decoder.getSamples(8, fbuf));
    pplib::STEREOSAMPLE_FLOAT fmix[8] = {};
    EXPECT_EQ((size_t)0, decoder.addSamples(8, fmix));
    pplib::STEREOSAMPLE32 mix[8] = {};
    EXPECT_EQ((size_t)0, decoder.addSamples(8, mix));
    EXPECT_EQ((size_t)0, decoder.getSamples(8, (pplib::STEREOSAMPLE16*)nullptr));

    // open() und schliessendes close() im selben Scope: der Zerleger darf nicht abstuerzen
    {
        pplib::AudioDecoder_Wave scoped;
        scoped.open(file);
        EXPECT_NO_THROW(scoped.getSamples(8, buf));
    }

    OpenWave(file, decoder);
    decoder.seekSample(10);
    EXPECT_EQ((size_t)10, decoder.getPosition());
    decoder.close();
    EXPECT_EQ((uint64_t)0, decoder.getAudioInfo().Samples) << "close() gibt den AudioInfo-Datensatz zurueck";
    EXPECT_EQ((size_t)0, decoder.getSamples(8, buf)) << "nach close() liefert der Dekoder 0";
}

TEST_F(AudioDecoderWaveTest, TruncatedFileIsClampedToRealData)
{
    // data-Chunk kuendigt 8192 Bytes an, es folgen aber nur 100 Bytes
    std::vector<char> wave = MakeWave(8192, 100, 2, 16, 44100, 1);
    pplib::MemFile file(wave.data(), wave.size());

    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint64_t)25, info.Samples) << "IdentWave meldet nur real vorhandene Frames";

    pplib::AudioDecoder_Wave decoder;
    OpenWave(file, decoder);
    EXPECT_EQ((uint64_t)25, decoder.getAudioInfo().Samples);

    pplib::STEREOSAMPLE16 buf[64];
    EXPECT_EQ((size_t)25, decoder.getSamples(64, buf)) << "die Ueberzahl liefert keine Geisterdaten";
    EXPECT_EQ((size_t)0, decoder.getSamples(64, buf));
}

TEST_F(AudioDecoderWaveTest, HostileAudioInfoIsClampedToRealData)
{
    // ein fremder AudioInfo-Datensatz kann Groessen melden, die nicht zur Datei passen.
    // Der Dekoder darf dann weder Geisterdaten liefern noch mit OverflowException abstuerzen
    std::vector<char> payload;
    for (int i = 0; i < 8; i++)
        appendLE(payload, 0x1111, 2);
    std::vector<char> wave = MakeWave((uint32_t)payload.size(), payload.size(), 2, 16, 44100, 1, &payload);

    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    info.Samples = 50000;
    info.AudioSize = 900000000;

    pplib::AudioDecoder_Wave decoder;
    ASSERT_NO_THROW(decoder.open(file, &info));
    pplib::STEREOSAMPLE16 buf[64];
    EXPECT_EQ((size_t)4, decoder.getSamples(64, buf)) << "nur die real vorhandenen Frames sind lesbar";
    EXPECT_EQ((int16_t)0x1111, buf[0].left);
    EXPECT_EQ((size_t)0, decoder.getSamples(64, buf));
}

TEST_F(AudioDecoderWaveTest, MappingWindowCoversAWholeBlockAtOnce)
{
    // 5000 Frames = 20000 Bytes: mehr als das 16 KiB Standardfenster, das vor dem Fix
    // jeden grossen Lesezugriff hart abschnitt
    std::vector<char> wave = MakeWave(20000, 20000, 2, 16, 44100);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    OpenWave(file, decoder);
    ASSERT_EQ((uint64_t)5000, decoder.getAudioInfo().Samples);

    std::vector<pplib::STEREOSAMPLE16> buf(5000);
    EXPECT_EQ((size_t)5000, decoder.getSamples(5000, buf.data()));
}

TEST_F(AudioDecoderWaveTest, TestdataFilesPublishTheirRealFormat)
{
    struct Expectation
    {
        const char* filename;
        uint64_t bits;
        uint64_t bytesPerSample;
        int sampleType; // verglichen gegen (int) info.SampleType
    };
    const Expectation cases[] = {{"test_44kHz.wav", 16, 4, (int)pplib::AudioInfo::SampleEncoding::INTEGER},
                                 {"test_44kHz_24bit.wav", 24, 6, (int)pplib::AudioInfo::SampleEncoding::INTEGER},
                                 {"test_44kHz_32bit.wav", 32, 8, (int)pplib::AudioInfo::SampleEncoding::INTEGER},
                                 {"test_44kHz_float.wav", 32, 8, (int)pplib::AudioInfo::SampleEncoding::FLOAT}};

    for (const Expectation& c : cases) {
        SCOPED_TRACE(c.filename);
        pplib::File f;
        OpenTestFile(f, c.filename);
        pplib::AudioInfo info;
        ASSERT_TRUE(pplib::IdentAudioFile(f, info));
        EXPECT_EQ((int)pplib::AudioInfo::AudioFormat::WAVE, (int)info.Format);
        EXPECT_EQ(c.bits, info.BitsPerSample);
        EXPECT_EQ(c.bytesPerSample, info.BytesPerSample);
        EXPECT_EQ((uint64_t)2, info.Channels);
        EXPECT_EQ(c.sampleType, (int)info.SampleType);
        EXPECT_GT(info.Samples, (uint64_t)0);

        pplib::AudioDecoder_Wave dec;
        ASSERT_NO_THROW(dec.open(f));
        pplib::STEREOSAMPLE16 samples[64];
        EXPECT_EQ((size_t)64, dec.getSamples(64, samples));
        EXPECT_EQ((size_t)64, dec.getPosition());
        dec.seekSample(0);
        EXPECT_EQ((size_t)0, dec.getPosition());

        int32_t peak = 0;
        for (const pplib::STEREOSAMPLE16& s : samples) {
            peak = std::max(peak, (int32_t)std::abs((int)s.left));
            peak = std::max(peak, (int32_t)std::abs((int)s.right));
        }
        EXPECT_GT(peak, 300) << "die Testdateien enthalten pegelrelevantes Material";
    }
}

TEST_F(AudioDecoderWaveTest, AllSampleWidthsDescribeTheSameSignal)
{
    // die vier Testdateien enthalten dasselbe Material in 16, 24, 32 und Float-Format
    struct Source
    {
        const char* filename;
        pplib::File file;
    };
    Source sources[] = {{"test_44kHz.wav", {}}, {"test_44kHz_24bit.wav", {}}, {"test_44kHz_32bit.wav", {}}, {"test_44kHz_float.wav", {}}};

    pplib::STEREOSAMPLE_FLOAT reference[32];
    size_t frames = 0;
    for (Source& src : sources) {
        SCOPED_TRACE(src.filename);
        OpenTestFile(src.file, src.filename);
        pplib::AudioDecoder_Wave dec;
        ASSERT_NO_THROW(dec.open(src.file));
        pplib::STEREOSAMPLE_FLOAT buf[32];
        ASSERT_EQ((size_t)32, dec.getSamples(32, buf));
        if (frames == 0) {
            frames = 32;
            memcpy(reference, buf, sizeof(reference));
            continue;
        }
        for (size_t i = 0; i < frames; i++) {
            EXPECT_NEAR(reference[i].left, buf[i].left, 2e-3) << "Frame " << i;
            EXPECT_NEAR(reference[i].right, buf[i].right, 2e-3) << "Frame " << i;
        }
    }
    EXPECT_EQ((size_t)32, frames);
}

TEST_F(AudioDecoderWaveTest, OpenRejectsAudioInfoContradictingTheFile)
{
    // Ein mitgebrachter AudioInfo-Datensatz wird nicht ungeprueft uebernommen: er kann
    // Werte nennen, an denen die Lesemethoden die Samples verfehlen wuerden
    std::vector<char> wave = MakeWave(1024, 1024, 2, 16, 44100);
    pplib::MemFile file(wave.data(), wave.size());

    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    pplib::AudioDecoder_Wave decoder;

    info.BytesPerSample = 6; // sechs Bytes gehoehren zu 24 Bit oder zu drei Kanaelen
    EXPECT_THROW(decoder.open(file, &info), pplib::UnsupportedAudioFormatException);

    info.BytesPerSample = 4;
    info.Channels = 3;
    EXPECT_THROW(decoder.open(file, &info), pplib::UnsupportedAudioFormatException);
}

TEST_F(AudioDecoderWaveTest, AddSamplesFloatBufferAccumulatesNormalizedValues)
{
    // Die Ueberladung, die in einen FLOAT-Mischpuffer addiert, wurde bisher nie getestet.
    // Sie normiert jede unterstuetzte Breite auf -1.0 .. 1.0 und haeft an, statt zu ersetzen
    struct Width
    {
        uint16_t bits;
        uint16_t tag;
        uint32_t half; // halbe Vollskala im Quellformat, vorzeichenbehaftet
    };
    const Width widths[] = {
        {16, 1, 0x00004000},
        {24, 1, 0x00400000},
        {32, 1, 0x40000000},
        {32, 3, 0}, // Float-Samples stehen bereits normiert in der Datei
    };
    for (const Width& w : widths) {
        SCOPED_TRACE(std::to_string(w.bits) + (w.tag == 3 ? " Bit float" : " Bit integer"));
        std::vector<char> payload;
        const uint32_t neg = (w.tag == 3) ? 0 : (uint32_t)(0x100000000ull - (uint64_t)w.half);
        if (w.tag == 3) {
            appendFloat(payload, 0.5f);
            appendFloat(payload, -0.5f);
            appendFloat(payload, -0.5f);
            appendFloat(payload, 0.5f);
        } else {
            appendLE(payload, w.half, w.bits / 8);
            appendLE(payload, neg, w.bits / 8);
            appendLE(payload, neg, w.bits / 8);
            appendLE(payload, w.half, w.bits / 8);
        }
        std::vector<char> wave = MakeWave((uint32_t)payload.size(), payload.size(), 2, w.bits, 44100, w.tag, &payload);
        pplib::MemFile file(wave.data(), wave.size());
        pplib::AudioDecoder_Wave decoder;
        OpenWave(file, decoder);

        pplib::STEREOSAMPLE_FLOAT mix[2] = {{0.25f, 0.25f}, {-0.25f, -0.25f}};
        EXPECT_EQ((size_t)2, decoder.addSamples(2, mix));
        EXPECT_NEAR(0.75f, mix[0].left, 1e-4f);
        EXPECT_NEAR(-0.25f, mix[0].right, 1e-4f);
        EXPECT_NEAR(-0.75f, mix[1].left, 1e-4f);
        EXPECT_NEAR(0.25f, mix[1].right, 1e-4f);
    }
}

TEST_F(AudioDecoderWaveTest, MonoIsReplicatedInEveryReadOverload)
{
    // -5000 in 24 Bit, mono: alle vier Lesemethoden muessen den einen Kanal verdoppeln
    const int32_t value = -5000;
    const uint32_t bits24 = 0xFFEC78;
    std::vector<char> payload;
    for (int i = 0; i < 4; i++)
        appendLE(payload, bits24, 3);
    std::vector<char> wave = MakeWave((uint32_t)payload.size(), payload.size(), 1, 24, 44100, 1, &payload);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioDecoder_Wave decoder;
    OpenWave(file, decoder);

    const int16_t at16 = (int16_t)(value >> 8); // -20: die Uebergabe an 16-bit-Stellen erfolgt skaliert
    pplib::STEREOSAMPLE16 i16[4];
    EXPECT_EQ((size_t)4, decoder.getSamples(4, i16));
    EXPECT_EQ(at16, i16[1].left);
    EXPECT_EQ(i16[1].left, i16[1].right);

    pplib::STEREOSAMPLE32 summed[4] = {{1, 1}, {1, 1}, {1, 1}, {1, 1}};
    decoder.seekSample(0);
    EXPECT_EQ((size_t)4, decoder.addSamples(4, summed));
    EXPECT_EQ((int32_t)at16 + 1, summed[2].left);
    EXPECT_EQ(summed[2].left, summed[2].right);

    const float norm = (float)value / 8388608.0f;
    pplib::STEREOSAMPLE_FLOAT fp[4];
    decoder.seekSample(0);
    EXPECT_EQ((size_t)4, decoder.getSamples(4, fp));
    EXPECT_NEAR(norm, fp[3].left, 1e-6f);
    EXPECT_FLOAT_EQ(fp[3].left, fp[3].right);

    pplib::STEREOSAMPLE_FLOAT mix[2] = {{0.5f, 0.5f}, {0.0f, 0.0f}};
    decoder.seekSample(0);
    EXPECT_EQ((size_t)2, decoder.addSamples(2, mix));
    EXPECT_NEAR(0.5f + norm, mix[0].left, 1e-6f);
    EXPECT_NEAR(0.5f + norm, mix[0].right, 1e-6f);
}
