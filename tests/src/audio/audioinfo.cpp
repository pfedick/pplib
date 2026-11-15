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

#include <cstdio>
#include <string>
#include <type_traits>

#include <pplib/types/string.h>
#include <pplib/core/dir.h>
#include <pplib/core/functions.h>
#include <pplib/core/memfile.h>
#include <pplib/audio/audioinfo.h>

#include "pplib-tests.h"

namespace
{

// Kurzalias der scoped Enums, damit die Erwartungen in den Tests lesbar bleiben
using AF = pplib::AudioInfo::AudioFormat;
using CM = pplib::AudioInfo::ChannelMode;
using SE = pplib::AudioInfo::SampleEncoding;

class AudioInfoTest : public ::testing::Test
{
protected:
    AudioInfoTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~AudioInfoTest()
    {
    }

    /** @brief Testdatei aus testdata/audio oeffnen; bricht den Test mit Pfadnamen ab.
     *
     * File::open() wirft bei fehlender Datei. Ohne diese Kontrolle ware die
     * Fehlermeldung eines anschliessenden EXPECT nur "value of: false".
     */
    void OpenTestFile(pplib::File& file, const char* name)
    {
        const std::string path = std::string("testdata/audio/") + name;
        ASSERT_NO_THROW(file.open(path.c_str())) << "open() warf fuer " << path;
        ASSERT_TRUE(file.isOpen()) << "Datei ist nicht geoeffnet: " << path;
    }
};

TEST_F(AudioInfoTest, IdentMp3CBR192WithoutID3)
{
    pplib::File file;
    OpenTestFile(file, "test_192cbr.mp3");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(AF::MP3, info.Format);
    EXPECT_FALSE(info.HaveID3v2Tag);
    EXPECT_EQ((uint64_t)0, info.ID3v2TagStart);
    EXPECT_FALSE(info.IsVBR);
    EXPECT_EQ((uint16_t)192, info.Bitrate);
    EXPECT_EQ((uint8_t)2, info.Channels);
    EXPECT_EQ((uint8_t)16, info.BitsPerSample);
    EXPECT_EQ((uint8_t)4, info.BytesPerSample);
    EXPECT_EQ((uint32_t)44100, info.Frequency);
    EXPECT_EQ((uint64_t)176223, info.Samples);
    EXPECT_EQ(CM::JOINT_STEREO, info.Mode);
    EXPECT_EQ((uint64_t)95920, info.FileSize) << "Unexpected filesize";
    EXPECT_EQ((uint64_t)0, info.AudioStart) << "Unexpected audio start";
    EXPECT_EQ((uint64_t)95919, info.AudioEnd) << "Unexpected audio end";
    EXPECT_EQ((uint64_t)95920, info.AudioSize) << "Unexpected audio size";
}

TEST_F(AudioInfoTest, IdentMp3CBR320WithoutID3)
{
    pplib::File file;
    OpenTestFile(file, "test_320cbr.mp3");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(AF::MP3, info.Format);
    EXPECT_FALSE(info.HaveID3v2Tag);
    EXPECT_EQ((uint64_t)0, info.ID3v2TagStart);
    EXPECT_FALSE(info.IsVBR);
    EXPECT_EQ((uint16_t)320, info.Bitrate);
    EXPECT_EQ((uint8_t)2, info.Channels);
    EXPECT_EQ((uint8_t)16, info.BitsPerSample);
    EXPECT_EQ((uint8_t)4, info.BytesPerSample);
    EXPECT_EQ((uint32_t)44100, info.Frequency);
    EXPECT_EQ((uint64_t)176223, info.Samples);
}

TEST_F(AudioInfoTest, IdentMp3VBR192WithoutID3)
{
    pplib::File file;
    OpenTestFile(file, "test_192vbr.mp3");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(AF::MP3, info.Format);
    EXPECT_FALSE(info.HaveID3v2Tag);
    EXPECT_EQ((uint64_t)0, info.ID3v2TagStart);
    EXPECT_TRUE(info.IsVBR);
    EXPECT_EQ((uint16_t)176, info.Bitrate);
    EXPECT_EQ((uint8_t)2, info.Channels);
    EXPECT_EQ((uint8_t)16, info.BitsPerSample);
    EXPECT_EQ((uint8_t)4, info.BytesPerSample);
    EXPECT_EQ((uint32_t)44100, info.Frequency);
    EXPECT_EQ((uint64_t)176256, info.Samples);
}

TEST_F(AudioInfoTest, IdentMp3CBR192WithID3)
{
    pplib::File file;
    OpenTestFile(file, "test_192cbr_tagged.mp3");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(AF::MP3, info.Format);
    EXPECT_TRUE(info.HaveID3v2Tag);
    EXPECT_EQ((uint64_t)0, info.ID3v2TagStart);
    EXPECT_FALSE(info.IsVBR);
    EXPECT_EQ((uint16_t)192, info.Bitrate);
    EXPECT_EQ((uint8_t)2, info.Channels);
    EXPECT_EQ((uint8_t)16, info.BitsPerSample);
    EXPECT_EQ((uint8_t)4, info.BytesPerSample);
    EXPECT_EQ((uint32_t)44100, info.Frequency);
    EXPECT_EQ((uint64_t)176223, info.Samples);
}

TEST_F(AudioInfoTest, IdentAiffWithoutID3)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz.aiff");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info)) << "Audio format was not recognized";
    EXPECT_EQ(AF::AIFF, info.Format) << "Wrong audio format detected";
    EXPECT_FALSE(info.HaveID3v2Tag) << "ID3-Tag detected, but there is none in the file";
    EXPECT_EQ((uint64_t)0, info.ID3v2TagStart);
    EXPECT_FALSE(info.IsVBR) << "VBR detected, but it shouldn't";
    EXPECT_EQ((uint16_t)1411, info.Bitrate) << "Unexpected Bitrate";
    EXPECT_EQ((uint8_t)2, info.Channels) << "Unexpected number of channels";
    EXPECT_EQ((uint8_t)16, info.BitsPerSample) << "Unexpected Bits per Sample";
    EXPECT_EQ((uint8_t)4, info.BytesPerSample) << "Unexpected bytes per sample";
    EXPECT_EQ((uint32_t)44100, info.Frequency) << "Unexpected frequency";
    EXPECT_EQ((uint64_t)173695, info.Samples) << "Unexpected number of samples";
    EXPECT_EQ((uint32_t)3938, info.Length) << "Unexpected length";
    EXPECT_EQ((uint64_t)694834, info.FileSize) << "Unexpected filesize";
    EXPECT_EQ((uint64_t)54, info.AudioStart) << "Unexpected audio start";
    EXPECT_EQ((uint64_t)694833, info.AudioEnd) << "Unexpected audio end";
    EXPECT_EQ((uint64_t)694780, info.AudioSize) << "Unexpected audio size";
}

TEST_F(AudioInfoTest, IdentAiffWithID3)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz_tagged.aiff");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info)) << "Audio format was not recognized";
    EXPECT_EQ(AF::AIFF, info.Format) << "Wrong audio format detected";
    EXPECT_TRUE(info.HaveID3v2Tag) << "ID3-Tag detected, but there is none in the file";
    EXPECT_EQ((uint64_t)694842, info.ID3v2TagStart);
    EXPECT_FALSE(info.IsVBR) << "VBR detected, but it shouldn't";
    EXPECT_EQ((uint16_t)1411, info.Bitrate) << "Unexpected Bitrate";
    EXPECT_EQ((uint8_t)2, info.Channels) << "Unexpected number of channels";
    EXPECT_EQ((uint8_t)16, info.BitsPerSample) << "Unexpected Bits per Sample";
    EXPECT_EQ((uint8_t)4, info.BytesPerSample) << "Unexpected bytes per sample";
    EXPECT_EQ((uint32_t)44100, info.Frequency) << "Unexpected frequency";
    EXPECT_EQ((uint64_t)173695, info.Samples) << "Unexpected number of samples";
    EXPECT_EQ((uint32_t)3938, info.Length) << "Unexpected length";
    EXPECT_EQ((uint64_t)695866, info.FileSize) << "Unexpected filesize";
    EXPECT_EQ((uint64_t)54, info.AudioStart) << "Unexpected audio start";
    EXPECT_EQ((uint64_t)694833, info.AudioEnd) << "Unexpected audio end";
    EXPECT_EQ((uint64_t)694780, info.AudioSize) << "Unexpected audio size";
}

TEST_F(AudioInfoTest, IdentWaveWithoutID3)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz.wav");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info)) << "Audio format was not recognized";
    EXPECT_EQ(AF::WAVE, info.Format) << "Wrong audio format detected";
    EXPECT_FALSE(info.HaveID3v2Tag) << "ID3-Tag detected, but there is none in the file";
    EXPECT_EQ((uint64_t)0, info.ID3v2TagStart);
    EXPECT_FALSE(info.IsVBR) << "VBR detected, but it shouldn't";
    EXPECT_EQ((uint16_t)1411, info.Bitrate) << "Unexpected Bitrate";
    EXPECT_EQ((uint8_t)2, info.Channels) << "Unexpected number of channels";
    EXPECT_EQ((uint8_t)16, info.BitsPerSample) << "Unexpected Bits per Sample";
    EXPECT_EQ((uint8_t)4, info.BytesPerSample) << "Unexpected bytes per sample";
    EXPECT_EQ((uint32_t)44100, info.Frequency) << "Unexpected frequency";
    EXPECT_EQ((uint64_t)173695, info.Samples) << "Unexpected number of samples";
    EXPECT_EQ((uint32_t)3938, info.Length) << "Unexpected length";

    EXPECT_EQ((uint64_t)694824, info.FileSize) << "Unexpected filesize";
    EXPECT_EQ((uint64_t)44, info.AudioStart) << "Unexpected audio start";
    EXPECT_EQ((uint64_t)694824 - 44, info.AudioSize) << "Unexpected audio size";
    EXPECT_EQ((uint64_t)694823, info.AudioEnd) << "Unexpected audio end";
}

TEST_F(AudioInfoTest, IdentOggVorbis)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz.wav");
    ASSERT_NE(AF::OGG, pplib::IdentAudioFile(file));
    OpenTestFile(file, "test_44kHz.ogg");
    ASSERT_EQ(AF::OGG, pplib::IdentAudioFile(file));
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(AF::OGG, info.Format);
    EXPECT_FALSE(info.HaveID3v2Tag);
    EXPECT_EQ((uint64_t)0, info.ID3v2TagStart);
    EXPECT_FALSE(info.IsVBR);
    EXPECT_EQ((uint16_t)192, info.Bitrate);
    EXPECT_EQ((uint8_t)2, info.Channels);
    EXPECT_EQ((uint8_t)16, info.BitsPerSample);
    EXPECT_EQ((uint8_t)4, info.BytesPerSample);
    EXPECT_EQ((uint32_t)44100, info.Frequency);
    EXPECT_EQ((uint64_t)173695, info.Samples) << "Unexpected number of samples";
    EXPECT_EQ((uint32_t)3938, info.Length) << "Unexpected length";
    EXPECT_EQ((uint64_t)90768, info.FileSize) << "Unexpected filesize";
    EXPECT_EQ((uint64_t)8060, info.AudioStart) << "Unexpected audio start";
    EXPECT_EQ((uint64_t)90767, info.AudioEnd) << "Unexpected audio end";
    EXPECT_EQ((uint64_t)82708, info.AudioSize) << "Unexpected audio size";
}

//--- Helfer fuer synthetische Header (Negativ- und Edge-Faelle) -----------------

static void put8(std::vector<char>& v, uint8_t val)
{
    v.push_back((char)val);
}
static void put16le(std::vector<char>& v, uint16_t val)
{
    v.push_back((char)(val & 0xFF));
    v.push_back((char)(val >> 8));
}
static void put16be(std::vector<char>& v, uint16_t val)
{
    v.push_back((char)(val >> 8));
    v.push_back((char)(val & 0xFF));
}
static void put32le(std::vector<char>& v, uint32_t val)
{
    v.push_back((char)(val & 0xFF));
    v.push_back((char)((val >> 8) & 0xFF));
    v.push_back((char)((val >> 16) & 0xFF));
    v.push_back((char)(val >> 24));
}
static void put32be(std::vector<char>& v, uint32_t val)
{
    v.push_back((char)(val >> 24));
    v.push_back((char)((val >> 16) & 0xFF));
    v.push_back((char)((val >> 8) & 0xFF));
    v.push_back((char)(val & 0xFF));
}
static void putTag(std::vector<char>& v, const char* tag)
{
    v.insert(v.end(), tag, tag + 4);
}
//!\brief Sample-Rate als 10 Byte 80-Bit-Extended (big endian), optional mit Nachkommaanteil
static void putExtended80(std::vector<char>& v, uint32_t rate, uint32_t fraction = 0)
{
    uint64_t mantissa = ((uint64_t)rate << 48) | ((uint64_t)fraction << 16);
    put16be(v, 16383 + 15);
    for (int i = 7; i >= 0; i--)
        put8(v, (uint8_t)(mantissa >> (i * 8)));
}

/** @brief Minimaler AIFF-Container (FORM/AIFF oder FORM/AIFC + COMM + SSND)
 *
 * Die deklarierten Chunk-Groessen (commSize, ssndSize) werden unveraendert in die
 * Datei geschrieben, unabhaengig von der Zahl der tatsaechlich folgenden Bytes.
 * Damit lassen sich defekte oder abgeschnittene Dateien nachbauen.
 * Mit \p compType wird daraus AIFF-C: dann steht vor COMM ein FVER-Chunk und hinter
 * der Sample-Rate der 4-Byte-Kompressionstyp.
 */
static std::vector<char> MakeAIFF(uint32_t commSize,
                                  uint32_t ssndSize,
                                  uint32_t sampleOffset,
                                  size_t audioBytes,
                                  uint16_t channels = 2,
                                  uint16_t bitdepth = 16,
                                  uint32_t rate = 44100,
                                  uint32_t fraction = 0,
                                  const char* formType = "AIFF",
                                  const char* compType = nullptr)
{
    std::vector<char> body;
    if (compType != nullptr) {
        putTag(body, "FVER");
        put32be(body, 4);
        put32be(body, 0xA2805140); // AIFF-C-Version 1.0
    }
    putTag(body, "COMM");
    put32be(body, commSize);
    put16be(body, channels);
    put32be(body, (uint32_t)(audioBytes / ((bitdepth * channels + 7) / 8)));
    put16be(body, bitdepth);
    putExtended80(body, rate, fraction);
    if (compType != nullptr) putTag(body, compType);

    putTag(body, "SSND");
    put32be(body, ssndSize);
    put32be(body, sampleOffset);
    put32be(body, 0); // blockSize
    for (size_t i = 0; i < audioBytes; i++)
        put8(body, (char)(i & 0xFF));

    std::vector<char> v;
    putTag(v, "FORM");
    put32be(v, (uint32_t)(4 + body.size()));
    putTag(v, formType);
    v.insert(v.end(), body.begin(), body.end());
    return v;
}

/** @brief Minimaler RIFF/WAVE-Container (fmt, optional LIST und id3, dann data)
 *
 * Auch hier werden Chunk-Groessen deklariert, die nicht zum real folgenden Inhalt
 * passen muessen (defekte Dateien).
 */
static std::vector<char> MakeWAVE(uint32_t fmtSize,
                                  uint32_t dataSize,
                                  size_t audioBytes,
                                  uint16_t channels = 2,
                                  uint16_t bitdepth = 16,
                                  uint32_t rate = 44100,
                                  bool listChunkBeforeData = false,
                                  bool id3ChunkBeforeData = false,
                                  uint16_t formatTag = 1)
{
    std::vector<char> body;
    putTag(body, "fmt ");
    put32le(body, fmtSize);
    put16le(body, formatTag); // 1 = PCM, 3 = IEEE-Float
    put16le(body, channels);
    put32le(body, rate);
    put32le(body, rate * channels * (bitdepth / 8));
    put16le(body, (uint16_t)((bitdepth / 8) * channels));
    put16le(body, bitdepth);
    for (uint32_t i = 16; i < fmtSize && i < 40; i++)
        put8(body, 0);

    if (listChunkBeforeData) {
        putTag(body, "LIST");
        put32le(body, 14);
        putTag(body, "INFO");
        putTag(body, "ICMT");
        put32le(body, 2);
        put8(body, 'x');
        put8(body, 0);
    }

    if (id3ChunkBeforeData) {
        putTag(body, "id3 ");
        put32le(body, 10);
        putTag(body, "ID3\x03"); // 'I','D','3', Major-Version
        put8(body, 0);           // Minor-Version
        put8(body, 0);           // Flags
        put32le(body, 0);        // synchsafe Groesse: leerer Tag
    }

    putTag(body, "data");
    put32le(body, dataSize);
    for (size_t i = 0; i < audioBytes; i++)
        put8(body, (char)(i & 0xFF));

    std::vector<char> v;
    putTag(v, "RIFF");
    put32le(v, (uint32_t)(body.size() + 4));
    putTag(v, "WAVE");
    v.insert(v.end(), body.begin(), body.end());
    return v;
}

//!\brief Alle Felder mit unterscheidbaren Werten fuellen, um Veraenderungen nachweisen zu koennen
static pplib::AudioInfo SentinelInfo()
{
    pplib::AudioInfo info;
    info.Format = AF::MP3;
    info.HaveID3v2Tag = true;
    info.IsVBR = true;
    info.ID3v2TagStart = 0xDEAD;
    info.FileSize = 0xBEEF;
    info.AudioStart = 1;
    info.AudioEnd = 2;
    info.AudioSize = 3;
    info.Samples = 4;
    info.Length = 5;
    info.Frequency = 6;
    info.Bitrate = 7;
    info.BitsPerSample = 8;
    info.BytesPerSample = 9;
    info.Channels = 10;
    info.Mode = CM::JOINT_STEREO;
    return info;
}

static bool SameInfo(const pplib::AudioInfo& a, const pplib::AudioInfo& b)
{
    return a.Format == b.Format && a.HaveID3v2Tag == b.HaveID3v2Tag && a.IsVBR == b.IsVBR && a.ID3v2TagStart == b.ID3v2TagStart &&
           a.FileSize == b.FileSize && a.AudioStart == b.AudioStart && a.AudioEnd == b.AudioEnd && a.AudioSize == b.AudioSize &&
           a.Samples == b.Samples && a.Length == b.Length && a.Frequency == b.Frequency && a.Bitrate == b.Bitrate &&
           a.BitsPerSample == b.BitsPerSample && a.BytesPerSample == b.BytesPerSample && a.Channels == b.Channels && a.Mode == b.Mode;
}

TEST_F(AudioInfoTest, ConstructorDefaults)
{
    pplib::AudioInfo info;
    EXPECT_EQ(AF::UNKNOWN, info.Format);
    EXPECT_FALSE(info.HaveID3v2Tag);
    EXPECT_FALSE(info.IsVBR);
    EXPECT_EQ((uint64_t)0, info.ID3v2TagStart);
    EXPECT_EQ((uint64_t)0, info.FileSize);
    EXPECT_EQ((uint64_t)0, info.AudioStart);
    EXPECT_EQ((uint64_t)0, info.AudioEnd);
    EXPECT_EQ((uint64_t)0, info.AudioSize);
    EXPECT_EQ((uint64_t)0, info.Samples);
    EXPECT_EQ((uint32_t)0, info.Length);
    EXPECT_EQ((uint32_t)0, info.Frequency);
    EXPECT_EQ((uint16_t)0, info.Bitrate);
    EXPECT_EQ((uint8_t)0, info.BitsPerSample);
    EXPECT_EQ((uint8_t)0, info.BytesPerSample);
    EXPECT_EQ((uint8_t)0, info.Channels);
    EXPECT_EQ(CM::UNKNOWN, info.Mode);
}

// Die Groessenfelder sind bewusst 64 Bit, damit Dateien über 4 GB und lange MP3s
// korrekt beschrieben werden koennen. Der Test fuehrt zu keiner Laufzeitpruefung.
TEST_F(AudioInfoTest, SizeFieldsAre64Bit)
{
    static_assert(std::is_same_v<decltype(pplib::AudioInfo::ID3v2TagStart), uint64_t>);
    static_assert(std::is_same_v<decltype(pplib::AudioInfo::FileSize), uint64_t>);
    static_assert(std::is_same_v<decltype(pplib::AudioInfo::AudioStart), uint64_t>);
    static_assert(std::is_same_v<decltype(pplib::AudioInfo::AudioEnd), uint64_t>);
    static_assert(std::is_same_v<decltype(pplib::AudioInfo::AudioSize), uint64_t>);
    static_assert(std::is_same_v<decltype(pplib::AudioInfo::Samples), uint64_t>);
    SUCCEED();
}

TEST_F(AudioInfoTest, FormatOverloadMatchesInfoOverload)
{
    struct Entry
    {
        const char* path;
        pplib::AudioInfo::AudioFormat format;
        bool identifiable = true;
    } matrix[] = {
        {"testdata/audio/test_192cbr.mp3", AF::MP3},
        {"testdata/audio/test_44kHz.wav", AF::WAVE},
        {"testdata/audio/test_44kHz_24bit.wav", AF::WAVE},
        {"testdata/audio/test_44kHz_32bit.wav", AF::WAVE},
        {"testdata/audio/test_44kHz_float.wav", AF::WAVE},
        {"testdata/audio/test_44kHz.aiff", AF::AIFF},
        {"testdata/audio/test_44kHz_16bit_mono.aiff", AF::AIFF},
        {"testdata/audio/test_44kHz_24bit.aiff", AF::AIFF},
        {"testdata/audio/test_44kHz_32bit.aiff", AF::AIFF},
        {"testdata/audio/test_44kHz_32bit_float.aiff", AF::AIFF},
        {"testdata/audio/test_44kHz.ogg", AF::OGG},
        {"testdata/audio/test_44kHz_tagged.wav", AF::WAVE},
        {"testdata/audio/test_44kHz_tagged.aiff", AF::AIFF},
        {"testdata/audio/test_192cbr_tagged.mp3", AF::MP3},
        {"testdata/audio/test_192cbr_taggedWithCover.mp3", AF::MP3},
        {"testdata/audio/tags_at_eof.aiff", AF::AIFF},
        {"testdata/audio/cover.jpg", AF::UNKNOWN, false},
        // Der Ogg-Container ist codec-neutral; unterstuetzt ist nur Vorbis.
        {"testdata/audio/test_48kHz.opus", AF::UNKNOWN, false},
    };
    for (Entry& entry : matrix) {
        pplib::File file;
        ASSERT_NO_THROW(file.open(entry.path)) << entry.path;
        ASSERT_TRUE(file.isOpen()) << entry.path;
        pplib::AudioInfo info;
        EXPECT_EQ(entry.format, pplib::IdentAudioFile(file)) << entry.path;
        if (!entry.identifiable) {
            ASSERT_FALSE(pplib::IdentAudioFile(file, info)) << entry.path;
            EXPECT_EQ(AF::UNKNOWN, info.Format) << entry.path;
        } else {
            ASSERT_TRUE(pplib::IdentAudioFile(file, info)) << entry.path;
            EXPECT_EQ(entry.format, info.Format) << entry.path;
        }
    }
}

TEST_F(AudioInfoTest, NonAudioContentLeavesInfoUntouched)
{
    std::vector<char> data(2048, 'x');
    memcpy(data.data(), "NOT AN AUDIO FILE AT ALL, JUST PLAIN TEXT", 37);
    pplib::MemFile file(data.data(), data.size());
    pplib::AudioInfo expected = SentinelInfo();
    pplib::AudioInfo info = expected;
    ASSERT_FALSE(pplib::IdentAudioFile(file, info));
    EXPECT_TRUE(SameInfo(info, expected)) << "info darf bei Misserfolg nicht veraendert werden";
    EXPECT_EQ(AF::UNKNOWN, pplib::IdentAudioFile(file));
}

TEST_F(AudioInfoTest, EmptyAndTruncatedFiles)
{
    std::vector<char> empty(8, 0);
    pplib::MemFile fileEmpty(empty.data(), 0);
    EXPECT_NO_THROW({
        pplib::AudioInfo info = SentinelInfo();
        ASSERT_FALSE(pplib::IdentAudioFile(fileEmpty, info));
    });
    EXPECT_EQ(AF::UNKNOWN, pplib::IdentAudioFile(fileEmpty));

    // RIFF-Kopf angeschnitten: nur 20 Bytes, keine Ausnahme erwartet
    std::vector<char> wave = MakeWAVE(16, 8, 8);
    wave.resize(20);
    pplib::MemFile fileShort(wave.data(), wave.size());
    EXPECT_NO_THROW({
        pplib::AudioInfo info = SentinelInfo();
        ASSERT_FALSE(pplib::IdentAudioFile(fileShort, info));
    });

    // fmt-Chunk kuendigt 2 GByte an, real folgen nur wenige Bytes
    std::vector<char> hugeFmt = MakeWAVE(0x7FFFFFFF, 8, 8);
    pplib::MemFile fileHuge(hugeFmt.data(), hugeFmt.size());
    EXPECT_NO_THROW({
        pplib::AudioInfo info = SentinelInfo();
        ASSERT_FALSE(pplib::IdentAudioFile(fileHuge, info));
    });
}

TEST_F(AudioInfoTest, WaveAbsurdBitDepthIsRejected)
{
    // Bitdepth weit jenseits der unterstuetzten Obergrenze von 32 Bit. Frueher lief die
    // Pruefung ueber (bitdepth/8)*channels, was als uint8_t-Puffer 0 ergeben haette ->
    // Division durch 0 bei audioSize % bytesPerSample.
    std::vector<char> wave = MakeWAVE(16, 16, 16, 1, 2048, 44100);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info = SentinelInfo();
    EXPECT_FALSE(pplib::IdentAudioFile(file, info));
    EXPECT_TRUE(SameInfo(info, SentinelInfo()));
    EXPECT_EQ(AF::WAVE, pplib::IdentAudioFile(file));
}

TEST_F(AudioInfoTest, WaveDataChunkBeyondEof)
{
    // data-Chunk kuendigt 1 MByte an, real folgen nur 8 Byte Audiodaten
    std::vector<char> wave = MakeWAVE(16, 1024 * 1024, 8);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint64_t)(wave.size() - 44), info.AudioSize);
    EXPECT_EQ((uint64_t)(wave.size() - 1), info.AudioEnd);
    EXPECT_EQ((uint64_t)2, info.Samples); // 8 Byte / 4 Byte je Sample
}

TEST_F(AudioInfoTest, WaveListChunkBeforeData)
{
    std::vector<char> wave = MakeWAVE(16, 8, 8, 2, 16, 22050, true, false);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info)) << "LIST-Chunk zwischen fmt und data muss uebersprungen werden";
    EXPECT_EQ((uint32_t)22050, info.Frequency);
    EXPECT_EQ((uint64_t)66, info.AudioStart) << "Standard-44 plus 22 Byte LIST-Chunk";
    EXPECT_EQ((uint64_t)8, info.AudioSize);
    EXPECT_EQ((uint64_t)2, info.Samples);
}

TEST_F(AudioInfoTest, WaveId3ChunkBeforeData)
{
    std::vector<char> wave = MakeWAVE(16, 8, 8, 2, 16, 44100, false, true);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_TRUE(info.HaveID3v2Tag);
    EXPECT_EQ((uint64_t)44, info.ID3v2TagStart) << "Anfang des ID3v2-Tags im id3-Chunk erwartet";
    EXPECT_EQ((uint64_t)62, info.AudioStart);
    EXPECT_EQ((uint64_t)8, info.AudioSize);
}

TEST_F(AudioInfoTest, Wave20BitPadsEachChannelSampleToThreeBytes)
{
    // Aufgerundet wird je Kanal-Sample, nicht global: mono 3 Bytes pro Frame, stereo 2 x 3 = 6
    std::vector<char> wave = MakeWAVE(16, 12, 12, 1, 20, 8000, false, false);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint8_t)1, info.Channels);
    EXPECT_EQ(CM::MONO, info.Mode);
    EXPECT_EQ((uint8_t)20, info.BitsPerSample);
    EXPECT_EQ((uint8_t)3, info.BytesPerSample) << "20 Bit auf volle Bytes aufgerundet";
    EXPECT_EQ((uint64_t)4, info.Samples) << "12 Byte / 3 Byte je Frame";

    std::vector<char> stereo = MakeWAVE(16, 12, 12, 2, 20, 8000, false, false);
    pplib::MemFile stereoFile(stereo.data(), stereo.size());
    pplib::AudioInfo st;
    ASSERT_TRUE(pplib::IdentAudioFile(stereoFile, st));
    EXPECT_EQ((uint8_t)6, st.BytesPerSample) << "je Kanal auf 3 Bytes aufgefuellt";
    EXPECT_EQ((uint64_t)2, st.Samples) << "12 Byte / 6 Byte je Frame";
    EXPECT_EQ((uint16_t)384, st.Bitrate) << "8000 Frames/s * 6 Byte";
}

TEST_F(AudioInfoTest, Wave8BitMono48kHzExtensibleFmt)
{
    // 8 Bit Mono bei 48 kHz, fmt-Chunk mit 40 Byte (EXTENSIBLE-artig)
    std::vector<char> wave = MakeWAVE(40, 12, 12, 1, 8, 48000);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint8_t)1, info.Channels);
    EXPECT_EQ(CM::MONO, info.Mode);
    EXPECT_EQ((uint8_t)8, info.BitsPerSample);
    EXPECT_EQ((uint8_t)1, info.BytesPerSample);
    EXPECT_EQ((uint32_t)48000, info.Frequency);
    EXPECT_EQ((uint64_t)12, info.Samples);
    EXPECT_EQ((uint16_t)384, info.Bitrate);
    EXPECT_EQ((uint64_t)68, info.AudioStart) << "40 Byte fmt-Payload statt 16";
}

TEST_F(AudioInfoTest, WaveFloatFormatTagUsesFloatSampleEncoding)
{
    // 32 Bit sind sowohl Ganzzahl als auch float: erst der Format-Tag 3 aus dem fmt-Chunk
    // entscheidet, und ohne ihn waere eine Float-Datei keinem Dekoder zugeordnet
    std::vector<char> wave = MakeWAVE(16, 32, 32, 2, 32, 44100, false, false, 3);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(SE::FLOAT, info.SampleType);
    EXPECT_EQ((uint8_t)32, info.BitsPerSample);
    EXPECT_EQ((uint8_t)8, info.BytesPerSample);
    EXPECT_EQ((uint64_t)4, info.Samples) << "32 Byte / 8 Byte je Frame";
    EXPECT_EQ((uint16_t)2822, info.Bitrate) << "44100 Frames/s * 8 Byte";
}

TEST_F(AudioInfoTest, WavePcmWidthsUseIntegerSampleEncoding)
{
    // Die unterstuetzten PCM-Breiten: Samples stets pro Frame, bei mono nur ein Kanal
    const uint16_t widths[] = {16, 24, 32};
    for (uint16_t bits : widths) {
        SCOPED_TRACE(bits);
        const uint8_t frameBytes = (uint8_t)((bits + 7) / 8 * 2);
        std::vector<char> wave = MakeWAVE(16, 48, 48, 2, bits, 44100, false, false);
        pplib::MemFile file(wave.data(), wave.size());
        pplib::AudioInfo info;
        ASSERT_TRUE(pplib::IdentAudioFile(file, info));
        EXPECT_EQ(SE::INTEGER, info.SampleType);
        EXPECT_EQ((uint8_t)bits, info.BitsPerSample);
        EXPECT_EQ(frameBytes, info.BytesPerSample);
        EXPECT_EQ((uint64_t)(48 / frameBytes), info.Samples);

        std::vector<char> mono = MakeWAVE(16, 48, 48, 1, bits, 44100, false, false);
        pplib::MemFile monoFile(mono.data(), mono.size());
        pplib::AudioInfo minfo;
        ASSERT_TRUE(pplib::IdentAudioFile(monoFile, minfo));
        EXPECT_EQ((uint8_t)1, minfo.Channels);
        EXPECT_EQ(CM::MONO, minfo.Mode);
        EXPECT_EQ((uint8_t)(bits / 8), minfo.BytesPerSample);
        EXPECT_EQ((uint64_t)(48 / (bits / 8)), minfo.Samples);
    }
}

TEST_F(AudioInfoTest, WaveFormatTagOutsidePcmAndFloatIsNotSupported)
{
    // EXTENSIBLE (0xFFFE) und aLaw (6) haben keinen Dekoder: beide Overloads weisen die
    // Datei ab, damit spaeter nicht ein Dekoder mit falschen Annahmen gewaehlt wird
    const uint16_t tags[] = {0xFFFE, 6, 0};
    for (uint16_t tag : tags) {
        SCOPED_TRACE(tag);
        std::vector<char> wave = MakeWAVE(40, 16, 16, 2, 16, 44100, false, false, tag);
        pplib::MemFile file(wave.data(), wave.size());
        pplib::AudioInfo info = SentinelInfo();
        EXPECT_FALSE(pplib::IdentAudioFile(file, info));
        EXPECT_EQ(AF::UNKNOWN, pplib::IdentAudioFile(file)) << "Container wird nicht mehr zugesagt";

        // Der Tag 3 verlangt 32 Bit; 16-Bit-Float gibt es nicht
        std::vector<char> mismatch = MakeWAVE(16, 16, 16, 2, 16, 44100, false, false, 3);
        pplib::MemFile badFile(mismatch.data(), mismatch.size());
        pplib::AudioInfo badInfo;
        EXPECT_FALSE(pplib::IdentAudioFile(badFile, badInfo));
    }
}

TEST_F(AudioInfoTest, BothBitDepth33IsRejected)
{
    // Obergrenze 32 Bit ist inklusiv, 33 Bit muss fuer beide Container abgewiesen werden
    std::vector<char> wave = MakeWAVE(16, 16, 16, 1, 33, 44100);
    pplib::MemFile fileWave(wave.data(), wave.size());
    pplib::AudioInfo infoWave;
    EXPECT_FALSE(pplib::IdentAudioFile(fileWave, infoWave));

    std::vector<char> aiff = MakeAIFF(18, 16, 0, 16, 1, 33, 44100, 0);
    pplib::MemFile fileAiff(aiff.data(), aiff.size());
    pplib::AudioInfo infoAiff;
    EXPECT_FALSE(pplib::IdentAudioFile(fileAiff, infoAiff));
    EXPECT_EQ(AF::WAVE, pplib::IdentAudioFile(fileWave)) << "Container bleibt erkennbar";
    EXPECT_EQ(AF::AIFF, pplib::IdentAudioFile(fileAiff));
}

TEST_F(AudioInfoTest, EmptyAudioPayloadIsRejected)
{
    // data- bzw. SSND-Payload enthaelt keine abspielbaren Samples -> kein gueltiges Audio.
    // Der Contract: Bei false bleibt das uebergebene info vollstaendig unveraendert.
    std::vector<char> wave = MakeWAVE(16, 0, 0);
    pplib::MemFile fileWave(wave.data(), wave.size());
    pplib::AudioInfo infoWave = SentinelInfo();
    EXPECT_FALSE(pplib::IdentAudioFile(fileWave, infoWave));
    EXPECT_TRUE(SameInfo(infoWave, SentinelInfo()));
    EXPECT_EQ(AF::WAVE, pplib::IdentAudioFile(fileWave)) << "Container bleibt erkennbar";

    // SSND deklariert 8 Byte (nur Offset + Blockgroesse), es folgen keine Samples
    std::vector<char> aiff = MakeAIFF(18, 8, 0, 8);
    pplib::MemFile fileAiff(aiff.data(), aiff.size());
    pplib::AudioInfo infoAiff = SentinelInfo();
    EXPECT_FALSE(pplib::IdentAudioFile(fileAiff, infoAiff));
    EXPECT_TRUE(SameInfo(infoAiff, SentinelInfo()));
    EXPECT_EQ(AF::AIFF, pplib::IdentAudioFile(fileAiff));
}

TEST_F(AudioInfoTest, ExtremeFrequencyClampsBitrate)
{
    // 32 Bit Stereo bei 4 GHz: die Bitrate liegt weit ueber uint16_t und muss
    // begrenzt werden, der Typ von AudioInfo::Bitrate bleibt aber uint16_t
    std::vector<char> wave = MakeWAVE(16, 16, 16, 2, 32, 4000000000u);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint8_t)32, info.BitsPerSample) << "32 Bit sind noch innerhalb der Obergrenze";
    EXPECT_EQ((uint8_t)8, info.BytesPerSample);
    EXPECT_EQ((uint16_t)UINT16_MAX, info.Bitrate);
}

TEST_F(AudioInfoTest, AiffSampleOffsetAndChunkBeyondEof)
{
    // SSND kuendigt 1 MByte an, real folgen 16 Byte; Sample-Offset 8 verschiebt den Beginn
    std::vector<char> aiff = MakeAIFF(18, 1024 * 1024, 8, 16);
    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint64_t)62, info.AudioStart) << "SSND-Kopf (8) plus deklariertes Sample-Offset (8)";
    EXPECT_EQ((uint64_t)8, info.AudioSize) << "nur die bis zum Dateiende folgenden Bytes";
    EXPECT_EQ((uint64_t)(aiff.size() - 1), info.AudioEnd);
    EXPECT_EQ((uint64_t)aiff.size(), info.FileSize);
    EXPECT_EQ((uint64_t)2, info.Samples) << "COMM deklariert 4, real folgen nur 2 Sample-Frames";
}

TEST_F(AudioInfoTest, AiffTrailingHalfFrameIsTrimmed)
{
    // Nach Sample-Offset 8 folgen 117 Byte; bei Stereo/16 Bit (4 Byte je Frame) ist das
    // letzte Byte ein halber Frame und darf nicht als abspielbar published werden
    std::vector<char> aiff = MakeAIFF(18, 8 + 117, 3, 120);
    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint64_t)116, info.AudioSize);
    EXPECT_EQ(info.AudioStart + info.AudioSize - 1, info.AudioEnd) << "AudioEnd innerhalb des Audiodatenbereichs";
    EXPECT_LT(info.AudioEnd, info.FileSize) << "angehaengtes Einzelbyte aussen vor";
    EXPECT_EQ((uint64_t)29, info.Samples) << "auf die real folgenden Frames begrenzt";
}

TEST_F(AudioInfoTest, AiffFractionalSampleRate)
{
    // 80-Bit-Extended mit Nachkommaanteil: 44100.5 Hz wird auf 44101 gerundet
    std::vector<char> aiff = MakeAIFF(18, 16, 0, 8, 2, 16, 44100, 0x80000000);
    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint32_t)44101, info.Frequency);
    EXPECT_EQ((uint16_t)1411, info.Bitrate);
}

TEST_F(AudioInfoTest, AiffMonoSetsMonoChannelMode)
{
    // Ein Kanal -> Mode MONO (bei MP3 wird die Zuordnung ueber die MPEG-Modi geprueft,
    // für AIFF/WAV war der MONO-Zweig bisher ungetestet)
    std::vector<char> aiff = MakeAIFF(18, 512, 0, 512, 1, 16, 44100, 0);
    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint8_t)1, info.Channels);
    EXPECT_EQ(CM::MONO, info.Mode);
    EXPECT_EQ((uint8_t)2, info.BytesPerSample);
    EXPECT_EQ((uint16_t)705, info.Bitrate);
}

TEST_F(AudioInfoTest, AiffAbsurdBitDepthIsRejected)
{
    // Bitdepth weit jenseits der Obergrenze von 32 Bit. Frueher lief die Pruefung ueber
    // (bitdepth/8)*channels, was als uint8_t-Puffer auf 0 fiele; statt mit BytesPerSample 0
    // weiterzuarbeiten (audioSize % bytesPerSample), wird der Kandidat jetzt verworfen.
    std::vector<char> aiff = MakeAIFF(18, 16, 0, 16, 1, 2048, 44100, 0);
    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioInfo info = SentinelInfo();
    EXPECT_FALSE(pplib::IdentAudioFile(file, info));
    EXPECT_TRUE(SameInfo(info, SentinelInfo()));
    EXPECT_EQ(AF::AIFF, pplib::IdentAudioFile(file));
}

TEST_F(AudioInfoTest, AiffCommChunkBeyondEof)
{
    std::vector<char> aiff = MakeAIFF(0x7FFFFFFF, 8, 0, 8);
    pplib::MemFile file(aiff.data(), aiff.size());
    EXPECT_NO_THROW({
        pplib::AudioInfo info = SentinelInfo();
        ASSERT_FALSE(pplib::IdentAudioFile(file, info));
    });
    // Container ist erkennbar, die Auswertung der Kommponenten scheitert
    EXPECT_EQ(AF::AIFF, pplib::IdentAudioFile(file));
}

TEST_F(AudioInfoTest, AiffFormSizeBeyondEof)
{
    // FORM-Groesse weist ueber das Dateiende -> nicht mehr als AIFF akzeptiert
    std::vector<char> aiff = MakeAIFF(18, 8, 0, 8);
    aiff[4] = (char)0x7F;
    aiff[5] = (char)0xFF;
    aiff[6] = (char)0xFF;
    aiff[7] = (char)0xFF;
    pplib::MemFile file(aiff.data(), aiff.size());
    EXPECT_EQ(AF::UNKNOWN, pplib::IdentAudioFile(file));
    pplib::AudioInfo info;
    ASSERT_FALSE(pplib::IdentAudioFile(file, info));
}

TEST_F(AudioInfoTest, AiffOtherBitDepthsUsePaddedFrames)
{
    // Die von Audacity erzeugten Dateien decken die bei AIFF real vorkommenden Bitweiten ab.
    // 24 Bit werden pro Kanal in 3 Byte gepackt (kein 4-Byte-Padding, wie es die Spec nennt),
    // 32 Bit in 4 Byte. Die Samplezahl richtet sich nach den real folgenden Daten.
    struct Entry
    {
        const char* name;
        uint8_t channels;
        uint8_t bits;
        uint8_t bytesPerSample;
        uint16_t bitrate;
        CM mode;
    } matrix[] = {
        {"test_44kHz_16bit_mono.aiff", (uint8_t)1, (uint8_t)16, (uint8_t)2, (uint16_t)705, CM::MONO},
        {"test_44kHz_24bit.aiff", (uint8_t)2, (uint8_t)24, (uint8_t)6, (uint16_t)2116, CM::STEREO},
        {"test_44kHz_32bit.aiff", (uint8_t)2, (uint8_t)32, (uint8_t)8, (uint16_t)2822, CM::STEREO},
    };
    for (Entry& entry : matrix) {
        pplib::File file;
        OpenTestFile(file, entry.name);
        pplib::AudioInfo info;
        ASSERT_TRUE(pplib::IdentAudioFile(file, info)) << entry.name;
        EXPECT_EQ(entry.channels, info.Channels) << entry.name;
        EXPECT_EQ(entry.bits, info.BitsPerSample) << entry.name;
        EXPECT_EQ(entry.bytesPerSample, info.BytesPerSample) << entry.name;
        EXPECT_EQ(entry.bitrate, info.Bitrate) << entry.name;
        EXPECT_EQ(entry.mode, info.Mode) << entry.name;
        EXPECT_EQ((uint64_t)173695, info.Samples) << entry.name;
        EXPECT_EQ((uint32_t)3938, info.Length) << entry.name;
        EXPECT_EQ(SE::INTEGER, info.SampleType) << entry.name;
    }
}

TEST_F(AudioInfoTest, AiffcFloatSetsSampleType)
{
    // AIFF-C mit Kompressionstyp FL32: 32 Bit, aber float-codiert. Der vorgeschaltete
    // FVER- und der PEAK-Chunk muessen beim Durchlaufen der Innerchunks uebersprungen werden.
    pplib::File file;
    OpenTestFile(file, "test_44kHz_32bit_float.aiff");

    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(AF::AIFF, info.Format);
    EXPECT_EQ(SE::FLOAT, info.SampleType);
    EXPECT_EQ((uint8_t)32, info.BitsPerSample);
    EXPECT_EQ((uint8_t)8, info.BytesPerSample);
    EXPECT_EQ((uint64_t)173695, info.Samples);
    EXPECT_EQ((uint64_t)104, info.AudioStart);
    EXPECT_EQ((uint64_t)1389560, info.AudioSize);
    EXPECT_EQ((uint16_t)2822, info.Bitrate);
}

TEST_F(AudioInfoTest, AiffcUncompressedIsAccepted)
{
    // AIFF-C mit NONE entspricht dem klassischen AIFF, traegt aber FVER und Kompressionstyp
    std::vector<char> aiff = MakeAIFF(22, 8 + 64, 0, 64, 2, 16, 44100, 0, "AIFC", "NONE");
    pplib::MemFile file(aiff.data(), aiff.size());

    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(SE::INTEGER, info.SampleType);
    EXPECT_EQ((uint8_t)16, info.BitsPerSample);
    EXPECT_EQ((uint16_t)1411, info.Bitrate);
}

TEST_F(AudioInfoTest, AiffcCompressedIsRejected)
{
    // Dekodiert wird nur NONE und FL32. Typen wie sowt/ulaw/alaw/adpc bleiben ununterstuetzt;
    // am Container selbst ist daran nichts zu aendern (Format-Uberladung erkennt AIFF).
    std::vector<char> aiff = MakeAIFF(22, 8 + 64, 0, 64, 2, 16, 44100, 0, "AIFC", "sowt");
    pplib::MemFile file(aiff.data(), aiff.size());

    pplib::AudioInfo info = SentinelInfo();
    EXPECT_FALSE(pplib::IdentAudioFile(file, info));
    EXPECT_TRUE(SameInfo(info, SentinelInfo()));
    EXPECT_EQ(AF::AIFF, pplib::IdentAudioFile(file));
}

TEST_F(AudioInfoTest, AiffcWithoutCompressionFieldIsRejected)
{
    // COMM misst nur 18 Byte, enthaelt also keinen Kompressionstyp -> nicht beschreibbar
    std::vector<char> aiff = MakeAIFF(18, 8 + 64, 0, 64, 2, 16, 44100, 0, "AIFC", nullptr);
    pplib::MemFile file(aiff.data(), aiff.size());

    pplib::AudioInfo info = SentinelInfo();
    EXPECT_FALSE(pplib::IdentAudioFile(file, info));
    EXPECT_TRUE(SameInfo(info, SentinelInfo()));
}

TEST_F(AudioInfoTest, AiffcFl32WithWrongBitdepthIsRejected)
{
    // FL32 verspricht 32-Bit-Float. Weicht die Bitweite ab, ist die Datei defekt.
    std::vector<char> aiff = MakeAIFF(22, 8 + 64, 0, 64, 2, 16, 44100, 0, "AIFC", "FL32");
    pplib::MemFile file(aiff.data(), aiff.size());

    pplib::AudioInfo info = SentinelInfo();
    EXPECT_FALSE(pplib::IdentAudioFile(file, info));
    EXPECT_TRUE(SameInfo(info, SentinelInfo()));
}

/** @brief Minimaler MP3-Stream mit vorgegebenem Mode-Feld des Frame- Headers.
 *
 * 3 Frames a 104 Byte (MPEG-1 Layer III, 32 kbit/s, 44,1 kHz, ohne Padding).
 */
static std::vector<char> MakeMP3(uint8_t mode, size_t frames = 3)
{
    const size_t frameBytes = 104;
    std::vector<char> data(frames * frameBytes, 0);
    for (size_t i = 0; i < frames; i++) {
        char* hdr = data.data() + i * frameBytes;
        hdr[0] = (char)0xFF;
        hdr[1] = (char)0xFB;        // Sync, MPEG-1, Layer III, ohne CRC
        hdr[2] = (char)0x20;        // Bitrate-Index 2 (32 kbit/s), 44,1 kHz
        hdr[3] = (char)(mode << 6); // Mode-Feld, keine Mode-Erweiterung
    }
    return data;
}

TEST_F(AudioInfoTest, Mp3ModeFieldSelectsChannelMode)
{
    struct
    {
        uint8_t mode;
        pplib::AudioInfo::ChannelMode expect;
        uint8_t channels;
    } modes[] = {
        {(uint8_t)0, CM::STEREO, 2},
        {(uint8_t)2, CM::DUAL_CHANNEL, 2},
        {(uint8_t)3, CM::MONO, 1},
    };
    for (auto& m : modes) {
        std::vector<char> mp3 = MakeMP3(m.mode);
        pplib::MemFile file(mp3.data(), mp3.size());
        pplib::AudioInfo info = SentinelInfo();
        ASSERT_TRUE(pplib::IdentAudioFile(file, info)) << "Mode-Feld " << (unsigned)m.mode;
        EXPECT_EQ(AF::MP3, info.Format);
        EXPECT_EQ(m.expect, info.Mode) << "Mode-Feld " << (unsigned)m.mode;
        EXPECT_EQ(m.channels, info.Channels);
        EXPECT_EQ((uint64_t)mp3.size(), info.FileSize);
        EXPECT_EQ((uint64_t)0, info.AudioStart);
        EXPECT_EQ((uint64_t)mp3.size() - 1, info.AudioEnd);
        EXPECT_EQ((uint64_t)mp3.size(), info.AudioSize);
        EXPECT_FALSE(info.IsVBR);
        file.close();
    }
}

TEST_F(AudioInfoTest, Mp3WithID3v2TagAtStart)
{
    pplib::File file;
    ASSERT_NO_THROW(file.open("testdata/audio/test_192cbr_tagged.mp3"));
    ASSERT_TRUE(file.isOpen());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(AF::MP3, info.Format);
    EXPECT_TRUE(info.HaveID3v2Tag);
    // Das ID3v2-Tag steht am Dateianfang, die Audiodaten beginnen dahinter.
    EXPECT_EQ((uint64_t)0, info.ID3v2TagStart);
    EXPECT_EQ((uint64_t)97072, info.FileSize);
    EXPECT_EQ((uint64_t)1024, info.AudioStart);
    EXPECT_EQ((uint64_t)96943, info.AudioEnd);
    EXPECT_EQ((uint64_t)95920, info.AudioSize);
    EXPECT_EQ((uint64_t)176223, info.Samples);
    EXPECT_EQ(CM::JOINT_STEREO, info.Mode);
    file.close();
}

TEST_F(AudioInfoTest, AiffWithTagsAtEndOfFile)
{
    pplib::File file;
    ASSERT_NO_THROW(file.open("testdata/audio/tags_at_eof.aiff"));
    ASSERT_TRUE(file.isOpen());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(AF::AIFF, info.Format);
    EXPECT_TRUE(info.HaveID3v2Tag);
    EXPECT_EQ((uint64_t)102204, info.FileSize);
    EXPECT_EQ((uint64_t)90, info.AudioStart);
    EXPECT_EQ((uint64_t)101933, info.AudioEnd);
    EXPECT_EQ((uint64_t)101844, info.AudioSize);
    EXPECT_EQ((uint64_t)101942, info.ID3v2TagStart);
    EXPECT_EQ((uint8_t)2, info.Channels);
    EXPECT_EQ((uint32_t)44100, info.Frequency);
    file.close();
}

TEST_F(AudioInfoTest, OggShorterThanCodecSignatureWindowIsUnknown)
{
    // "OggS" allein ist kein Codec-Nachweis. Die Datei sei lang genug fuer die
    // Headerpruefung (24 Bytes), aber kuerzer als das Signaturfenster: Die Schleife
    // darf nicht ueber das Mapping hinaus lesen.
    char buf[30] = {};
    memcpy(buf, "OggS", 4);
    pplib::MemFile mf;
    ASSERT_NO_THROW(mf.open(buf, sizeof(buf), false));
    EXPECT_EQ(AF::UNKNOWN, pplib::IdentAudioFile(mf));
    pplib::AudioInfo info = SentinelInfo();
    EXPECT_FALSE(pplib::IdentAudioFile(mf, info));
    EXPECT_TRUE(SameInfo(info, SentinelInfo()));
    mf.close();
}

TEST_F(AudioInfoTest, TruncatedOggVorbisReturnsFalse)
{
    // abgeschnittener Ogg-Vorbis-Stream: libvorbis bekommt Dateiende als Kurzlesen
    // gemeldet, die Identifikation liefert false statt einer Exception und gibt ihre
    // Allokationen wieder frei (ov_clear auf jedem Rueckgabepfad)
    pplib::File src;
    OpenTestFile(src, "test_44kHz.ogg");
    const size_t total = (size_t)src.size();
    std::vector<char> all(total);
    ASSERT_EQ(total, src.read(all.data(), total));
    src.close();

    const size_t cuts[] = {3000, 300};
    for (size_t cut : cuts) {
        ASSERT_LT(cut, total) << "Testdatei ist zu kurz fuer den Schnitt bei " << cut;
        pplib::MemFile file(all.data(), cut);
        pplib::AudioInfo expected = SentinelInfo();
        pplib::AudioInfo info = SentinelInfo();
        EXPECT_NO_THROW(EXPECT_FALSE(pplib::IdentAudioFile(file, info))) << "Schnitt bei " << cut << " Bytes";
        EXPECT_TRUE(SameInfo(info, expected)) << "info darf bei Misserfolg nicht veraendert werden";
    }
}

TEST_F(AudioInfoTest, OpusInOggContainerIsNotSupported)
{
    // Der Ogg-Container ist codec-neutral. Da nur Vorbis dekodiert wird, gilt
    // ein Opus-Stream als unbekannt: beide Overloads liefere UNKNOWN bzw. false.
    pplib::File file;
    ASSERT_NO_THROW(file.open("testdata/audio/test_48kHz.opus"));
    ASSERT_TRUE(file.isOpen());
    EXPECT_EQ(AF::UNKNOWN, pplib::IdentAudioFile(file));
    pplib::AudioInfo expected = SentinelInfo();
    pplib::AudioInfo info = SentinelInfo();
    EXPECT_FALSE(pplib::IdentAudioFile(file, info));
    EXPECT_TRUE(SameInfo(info, expected)) << "info darf bei Misserfolg nicht veraendert werden";
    file.close();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// FLAC: Container, STREAMINFO und ID3-Grenzen
////////////////////////////////////////////////////////////////////////////////////////////////

/** @brief Schreibt Bits in einen Byte-Puffer, MSB zuerst (Reihenfolge der STREAMINFO-Felder) */
class FlacBitWriter
{
public:
    FlacBitWriter(std::vector<char>& out)
        : out(out)
    {
    }
    void Put(uint64_t value, uint32_t count)
    {
        for (uint32_t i = count; i > 0; i--) {
            buf = (char)((buf << 1) | ((value >> (i - 1)) & 1));
            if (++bits == 8) {
                out.push_back(buf);
                buf = 0;
                bits = 0;
            }
        }
    }
    /// schliesst ein unvollstaendiges Byte mit Nullen auf (STREAMINFO ist exakt 34 Byte,
    /// noetig nur fuer nachgebaut defekte Bloeke)
    void Flush()
    {
        if (bits != 0) {
            out.push_back((char)(buf << (8 - bits)));
            buf = 0;
            bits = 0;
        }
    }

private:
    std::vector<char>& out;
    char buf = 0;
    uint8_t bits = 0;
};

/// Metadata-Block-Kopf: 1 Bit last-flag, 7 Bit Typ, 3 Byte Groesse (Big Endian)
static void PutFLACBlockHeader(std::vector<char>& v, uint8_t type, uint32_t size, bool last)
{
    v.push_back((char)((last ? 0x80 : 0x00) | (type & 0x7F)));
    v.push_back((char)((size >> 16) & 0xFF));
    v.push_back((char)((size >> 8) & 0xFF));
    v.push_back((char)(size & 0xFF));
}

/** @brief STREAMINFO-Block (Typ 0) mit vorgegebenen Werten
 *
 * \p blockBits setzt die Bitbreite der Block-/Framegroessen, damit sich mit kleinem
 * maxBlocksize die Obergrenze der Samplezahl gezielt unterholen laesst.
 */
static void PutFLACStreamInfo(std::vector<char>& v,
                              uint16_t channels,
                              uint32_t bitdepth,
                              uint32_t rate,
                              uint64_t totalSamples,
                              uint16_t maxBlocksize = 4096,
                              bool last = true,
                              uint32_t blockBits = 16)
{
    PutFLACBlockHeader(v, 0, 34, last);
    FlacBitWriter bits(v);
    bits.Put(maxBlocksize, blockBits);
    bits.Put(maxBlocksize, blockBits);
    bits.Put(0, 24); // minFrameSize
    bits.Put(0, 24); // maxFrameSize
    bits.Put(rate, 20);
    bits.Put(channels - 1, 3);
    bits.Put(bitdepth - 1, 5);
    bits.Put(totalSamples, 36);
    for (int i = 0; i < 16; i++)
        bits.Put(0, 8); // MD5
    bits.Flush();
}

/** @brief Minimaler FLAC-Container: Magic, optional Padding vor STREAMINFO, dann \p audioBytes Daten */
static std::vector<char> MakeFLAC(uint16_t channels = 2,
                                  uint32_t bitdepth = 16,
                                  uint32_t rate = 44100,
                                  uint64_t totalSamples = 100,
                                  size_t audioBytes = 512,
                                  uint16_t maxBlocksize = 4096,
                                  size_t paddingBefore = 0)
{
    std::vector<char> v;
    putTag(v, "fLaC");
    if (paddingBefore != 0) {
        PutFLACBlockHeader(v, 1, (uint32_t)paddingBefore, false);
        for (size_t i = 0; i < paddingBefore; i++)
            put8(v, 0);
    }
    PutFLACStreamInfo(v, channels, bitdepth, rate, totalSamples, maxBlocksize);
    for (size_t i = 0; i < audioBytes; i++)
        put8(v, (char)(i & 0xFF));
    return v;
}

/// schreibt 128 Bytes ID3v1-Tag (Magic "TAG") an das Dateiende
static void AppendID3v1(std::vector<char>& v)
{
    put8(v, 'T');
    put8(v, 'A');
    put8(v, 'G');
    for (int i = 0; i < 125; i++)
        put8(v, 0);
}

/** @brief Alle Felder einer erkannten FLAC-Datei gegen die Erwartungen pruefen
 *
 * Die Samples-Angabe ist bei allen Testdateien 173695, daher sind Length und Bitrate
 * feste Rechenwerte (Length = Samples*1000/Frequency, Bitrate = AudioSize*8/FileSize*kHz).
 */
void ExpectFLAC(const pplib::AudioInfo& info,
                uint64_t fileSize,
                uint64_t audioStart,
                uint64_t audioSize,
                uint64_t samples,
                uint32_t length,
                uint16_t bitrate,
                uint8_t channels,
                uint8_t bitdepth,
                uint32_t frequency = 44100)
{
    EXPECT_EQ(AF::FLAC, info.Format);
    EXPECT_EQ(fileSize, info.FileSize);
    EXPECT_EQ(audioStart, info.AudioStart) << "erster Frame-Byte";
    EXPECT_EQ(audioStart + audioSize - 1, info.AudioEnd);
    EXPECT_EQ(audioSize, info.AudioSize) << " Frames bis Dateiende, ohne ID3-Tags";
    EXPECT_EQ(samples, info.Samples);
    EXPECT_EQ(length, info.Length);
    EXPECT_EQ(bitrate, info.Bitrate);
    EXPECT_EQ(frequency, info.Frequency);
    EXPECT_EQ(channels, info.Channels);
    EXPECT_EQ(bitdepth, info.BitsPerSample);
    EXPECT_EQ((uint8_t)((bitdepth / 8) * channels), info.BytesPerSample);
    EXPECT_EQ(channels == 1 ? CM::MONO : CM::STEREO, info.Mode);
    EXPECT_EQ(SE::INTEGER, info.SampleType) << "FLAC kennt keine Fliesskomma-Samples";
    EXPECT_FALSE(info.IsVBR);
}

TEST_F(AudioInfoTest, Flac16BitStereo)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz_16bit.flac");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    ExpectFLAC(info, 278361, 86, 278275, 173695, 3938, 565, 2, 16);
    EXPECT_FALSE(info.HaveID3v2Tag);
    EXPECT_EQ((uint64_t)0, info.ID3v2TagStart);
}

TEST_F(AudioInfoTest, Flac24BitStereo)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz_24bit.flac");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    ExpectFLAC(info, 280674, 86, 280588, 173695, 3938, 569, 2, 24);
}

TEST_F(AudioInfoTest, FlacMono)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz_mono.flac");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    // VORBIS_COMMENT und 8192 Byte Padding vor den Frames verschieben AudioStart
    ExpectFLAC(info, 139616, 8286, 131330, 173695, 3938, 266, 1, 16);
}

TEST_F(AudioInfoTest, FlacSkipsSeektableAndVorbisComment)
{
    // STREAMINFO + SEEKTABLE (72 Byte) + VORBIS_COMMENT (40 Byte, letzter Block)
    pplib::File file;
    OpenTestFile(file, "test_44kHz_16bit_seektable.flac");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    ExpectFLAC(info, 278437, 162, 278275, 173695, 3938, 565, 2, 16);
}

TEST_F(AudioInfoTest, FlacWithID3v2TagAtStartShiftsAudioStart)
{
    // 8192 Byte grosses ID3v2.3-Tag (Syncsafe-Value 8192, Kopf 10 Byte) vor "fLaC"
    pplib::File file;
    OpenTestFile(file, "test_44kHz_16bit_id3v2.flac");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_TRUE(info.HaveID3v2Tag);
    EXPECT_EQ((uint64_t)0, info.ID3v2TagStart);
    ExpectFLAC(info, 286563, 8288, 278275, 173695, 3938, 565, 2, 16);
    file.close();
}

TEST_F(AudioInfoTest, FlacWithID3v1TagAtEndExcludesTag)
{
    // 128 Byte ID3v1 am Ende: AudioEnd liegt davor, die Audiodaten selbst sind identisch
    pplib::File file;
    OpenTestFile(file, "test_44kHz_16bit_id3v1.flac");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    ExpectFLAC(info, 278489, 86, 278275, 173695, 3938, 565, 2, 16);
    EXPECT_FALSE(info.HaveID3v2Tag);
}

TEST_F(AudioInfoTest, FlacReportsHeaderSampleCountForTruncatedFile)
{
    // Nur der Kopf ist erhalten, die Frames fehlen. Die Erkennung liefert die deklarierte
    // Samplezahl der STREAMINFO; dass der Decoder spaeter am Dateiende endet, prueft die Decoder-Suite.
    pplib::File file;
    OpenTestFile(file, "test_44kHz_16bit_truncated.flac");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint64_t)20000, info.FileSize);
    EXPECT_EQ((uint64_t)86, info.AudioStart);
    EXPECT_EQ((uint64_t)19914, info.AudioSize);
    EXPECT_EQ((uint64_t)173695, info.Samples);
    EXPECT_EQ((uint32_t)3938, info.Length);
    EXPECT_EQ((uint16_t)40, info.Bitrate) << "Bitrate errechnet sich aus den vorhandenen Bytes";
}

TEST_F(AudioInfoTest, FlacStreamWithoutSampleCountKeepsLengthZero)
{
    // Eine uber eine Pipe kodierte FLAC-Datei nennt im STREAMINFO 0 als Gesamtsamplezahl.
    // Ein ratengeschaezter Wert waere eine erfundene Obergrenze fur seekSample(), deshalb
    // bleiben Samples, Length und Bitrate 0; die vorhandenen Bytes werden korrekt gemeldet.
    pplib::File file;
    OpenTestFile(file, "test_44kHz_16bit_stream.flac");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    // Nach dem Kopf steht ein grosses PADDING-Block, die Frames starten spaeter
    ExpectFLAC(info, 288051, 8286, 279765, 0, 0, 0, 2, 16);
    EXPECT_FALSE(info.HaveID3v2Tag);
}

TEST_F(AudioInfoTest, FlacReportsSixChannelsAsFLAC)
{
    // STREAMINFO weist 6 Kanaele aus: die Erkennung meldet die Fakten des Containers,
    // abgelehnt wird die Datei erst im Decoder (AudioDecoderFlacTest.OpenRejects... ).
    pplib::File file;
    OpenTestFile(file, "test_44kHz_51.flac");
    EXPECT_EQ(AF::FLAC, pplib::IdentAudioFile(file));
    pplib::AudioInfo info = SentinelInfo();
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    ExpectFLAC(info, 288844, 8328, 280516, 173695, 3938, 569, 6, 16);
}

TEST_F(AudioInfoTest, FlacReportsUnsupportedBitDepthsAsFLAC)
{
    // reale Datei mit 8 Bit pro Sample. Das 32-Bit-Beispiel ist nachgebaut, weil reale
    // Encoder 32 Bit nicht erzeugen; beide sind guiltiges FLAC, nur ohne Decoder in pplib.
    pplib::File file;
    OpenTestFile(file, "test_44kHz_8bit.flac");
    EXPECT_EQ(AF::FLAC, pplib::IdentAudioFile(file));
    pplib::AudioInfo info = SentinelInfo();
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    ExpectFLAC(info, 24670, 8304, 16366, 8820, 200, 650, 2, 8);

    std::vector<char> flac = MakeFLAC(2, 32, 44100, 100, 512);
    pplib::MemFile wide(flac.data(), flac.size());
    pplib::AudioInfo wideInfo = SentinelInfo();
    ASSERT_TRUE(pplib::IdentAudioFile(wide, wideInfo));
    ExpectFLAC(wideInfo, flac.size(), 42, 512, 100, 2, 1764, 2, 32);
}

TEST_F(AudioInfoTest, FlacStreamInfoMustBeFirstBlock)
{
    // Padding vor STREAMINFO: der Decoder verlangt den ersten Block
    std::vector<char> flac = MakeFLAC(2, 16, 44100, 100, 512, 4096, 8);
    pplib::MemFile file(flac.data(), flac.size());
    pplib::AudioInfo expected = SentinelInfo();
    pplib::AudioInfo info = SentinelInfo();
    EXPECT_FALSE(pplib::IdentAudioFile(file, info));
    EXPECT_TRUE(SameInfo(info, expected)) << "info darf bei Misserfolg nicht veraendert werden";
}

TEST_F(AudioInfoTest, FlacStreamInfoLengthBeyondToleranceIsRejected)
{
    std::vector<char> flac;
    putTag(flac, "fLaC");
    PutFLACBlockHeader(flac, 0, 33, true);
    FlacBitWriter bits(flac);
    bits.Put(4096, 16);
    bits.Put(4096, 16);
    bits.Put(0, 48);
    bits.Put(44100, 20);
    bits.Put(1, 3);
    bits.Put(15, 5);
    bits.Put(100, 36);
    for (int i = 0; i < 15; i++)
        bits.Put(0, 8); // ein Byte MD5 zu wenig
    bits.Flush();
    for (int i = 0; i < 512; i++)
        put8(flac, (char)(i & 0xFF));

    pplib::MemFile file(flac.data(), flac.size());
    pplib::AudioInfo info;
    EXPECT_FALSE(pplib::IdentAudioFile(file, info));
}

TEST_F(AudioInfoTest, FlacWithoutLastMetadataBlockIsRejected)
{
    // STREAMINFO und PADDING, aber kein Block mit last-metadata-block: die Position der
    // Frames steht nicht fest, also UNKNOWN. Nach dem letzten Block ist Dateiende.
    std::vector<char> flac;
    putTag(flac, "fLaC");
    PutFLACStreamInfo(flac, 2, 16, 44100, 100, 4096, false);
    PutFLACBlockHeader(flac, 1, 4, false);
    for (int i = 0; i < 4; i++)
        put8(flac, 0);

    pplib::MemFile file(flac.data(), flac.size());
    pplib::AudioInfo info;
    EXPECT_FALSE(pplib::IdentAudioFile(file, info));
}

TEST_F(AudioInfoTest, FlacMetadataBlockSizeBeyondEofIsRejected)
{
    // SEEKTABLE mit deklarierter Groesse hinter dem Dateiende
    std::vector<char> flac;
    putTag(flac, "fLaC");
    PutFLACStreamInfo(flac, 2, 16, 44100, 100, 4096, false);
    PutFLACBlockHeader(flac, 3, 1024 * 1024, true);

    pplib::MemFile file(flac.data(), flac.size());
    pplib::AudioInfo info;
    EXPECT_FALSE(pplib::IdentAudioFile(file, info));
}

TEST_F(AudioInfoTest, FlacSampleCountIsCappedByAvailableBytes)
{
    // STREAMINFO behauptet 1 Mio. Samples. Jeder Frame uebertraegt hoechstens maxBlocksize
    // Samples (hier 2) und kostet mindestens ein Byte, also wird auf 2 * 20000 begrenzt.
    std::vector<char> flac = MakeFLAC(2, 16, 44100, 1u << 20, 20000, 2);
    pplib::MemFile file(flac.data(), flac.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint64_t)(2 * 20000), info.Samples);
    EXPECT_EQ((uint32_t)(40000 * 1000 / 44100), info.Length);
    EXPECT_GT(info.Bitrate, (uint16_t)0);
}

TEST_F(AudioInfoTest, FlacStreamEnclosingZeroSampleCountGivesZeroDuration)
{
    // total samples = 0 bedeutet "unbekannt" (Stream-Aufnahme): Laenge und Bitrate sind 0,
    // die Vorwaertswerte der Streaminfo bleiben aber erhalten
    std::vector<char> flac = MakeFLAC(2, 24, 48000, 0, 20000);
    pplib::MemFile file(flac.data(), flac.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint64_t)0, info.Samples);
    EXPECT_EQ((uint32_t)0, info.Length);
    EXPECT_EQ((uint16_t)0, info.Bitrate);
    EXPECT_EQ((uint32_t)48000, info.Frequency);
    EXPECT_EQ((uint8_t)24, info.BitsPerSample);
    EXPECT_EQ((uint8_t)6, info.BytesPerSample);
    EXPECT_EQ((uint64_t)20000, info.AudioSize);
}

TEST_F(AudioInfoTest, FlacWithEmptyID3v2TagAtStart)
{
    // ID3v2-Header ohne Frames (Taggroesse 0): die Revision faehrt die Suche danach fort
    std::vector<char> flac = MakeFLAC(2, 16, 44100, 100, 512);
    std::vector<char> all;
    put8(all, 'I');
    put8(all, 'D');
    put8(all, '3'); // putTag wuerfe ein 4. Byte anhaengen
    put8(all, 3);   // Major Version 3
    put8(all, 0);   // Minor Version
    put8(all, 0);   // keine Flags
    for (int i = 0; i < 4; i++)
        put8(all, 0); // Taggroesse 0: nur der 10-Byte-Header
    all.insert(all.end(), flac.begin(), flac.end());

    pplib::MemFile file(all.data(), all.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_TRUE(info.HaveID3v2Tag);
    EXPECT_EQ(0u, info.ID3v2TagStart);
    // 10 Byte ID3v2-Header + 4 Byte Magie + Block-Kopf + 34 Byte STREAMINFO
    ExpectFLAC(info, all.size(), 10u + 4u + 4u + 34u, 512, 100, 2, 1764, 2, 16);
}

TEST_F(AudioInfoTest, FlacID3v2TagClaimingSizeBeyondEofIsUnknown)
{
    std::vector<char> all;
    put8(all, 'I');
    put8(all, 'D');
    put8(all, '3');
    put8(all, 3); // Major Version 3
    put8(all, 0); // Minor Version
    put8(all, 0); // keine Flags
    for (int i = 0; i < 4; i++)
        put8(all, 0x7F); // syncsichere 268435455
    std::vector<char> flac = MakeFLAC(2, 16, 44100, 100, 512);
    all.insert(all.end(), flac.begin(), flac.end());

    pplib::MemFile file(all.data(), all.size());
    EXPECT_EQ(AF::UNKNOWN, pplib::IdentAudioFile(file));
    pplib::AudioInfo info;
    EXPECT_FALSE(pplib::IdentAudioFile(file, info));
}

TEST_F(AudioInfoTest, FlacWithID3v1TagAtEndOfSyntheticFile)
{
    std::vector<char> flac = MakeFLAC(2, 16, 44100, 100, 512);
    AppendID3v1(flac);

    pplib::MemFile file(flac.data(), flac.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    // 4 Byte Magie + 1 Block-Kopf + 34 Byte STREAMINFO + 128 Byte ID3v1
    ExpectFLAC(info, 4u + 38u + 512u + 128u, 4u + 38u, 512, 100, 2, 1764, 2, 16);
    EXPECT_EQ(4u + 38u + 512u - 1u, info.AudioEnd);
}

TEST_F(AudioInfoTest, FlacEmptyStreamIsRejected)
{
    // Kopf vorhanden, aber keine Frames: wie bei WAVE/AIFF ist eine Datei ohne Audiodaten UNKNOWN
    std::vector<char> flac = MakeFLAC(2, 16, 44100, 0, 0);
    pplib::MemFile file(flac.data(), flac.size());
    pplib::AudioInfo info;
    EXPECT_FALSE(pplib::IdentAudioFile(file, info));
}

} // namespace
