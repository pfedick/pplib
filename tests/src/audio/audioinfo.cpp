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

#include <pplib/types/string.h>
#include <pplib/core/dir.h>
#include <pplib/core/functions.h>
#include <pplib/core/memfile.h>
#include <pplib/audio/audioinfo.h>

#include "pplib-tests.h"

namespace
{

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
    EXPECT_EQ(pplib::AudioInfo::MP3, info.Format);
    EXPECT_FALSE(info.HaveID3v2Tag);
    EXPECT_EQ((uint32_t)0, info.ID3v2TagStart);
    EXPECT_FALSE(info.IsVBR);
    EXPECT_EQ((uint16_t)192, info.Bitrate);
    EXPECT_EQ((uint8_t)2, info.Channels);
    EXPECT_EQ((uint8_t)16, info.BitsPerSample);
    EXPECT_EQ((uint8_t)4, info.BytesPerSample);
    EXPECT_EQ((uint32_t)44100, info.Frequency);
    EXPECT_EQ((uint32_t)176223, info.Samples);
    EXPECT_EQ(pplib::AudioInfo::JOINT_STEREO, info.Mode);
    EXPECT_EQ((uint32_t)95920, info.FileSize) << "Unexpected filesize";
    EXPECT_EQ((uint32_t)0, info.AudioStart) << "Unexpected audio start";
    EXPECT_EQ((uint32_t)95919, info.AudioEnd) << "Unexpected audio end";
    EXPECT_EQ((uint32_t)95920, info.AudioSize) << "Unexpected audio size";

}

TEST_F(AudioInfoTest, IdentMp3CBR320WithoutID3)
{
    pplib::File file;
    OpenTestFile(file, "test_320cbr.mp3");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(pplib::AudioInfo::MP3, info.Format);
    EXPECT_FALSE(info.HaveID3v2Tag);
    EXPECT_EQ((uint32_t)0, info.ID3v2TagStart);
    EXPECT_FALSE(info.IsVBR);
    EXPECT_EQ((uint16_t)320, info.Bitrate);
    EXPECT_EQ((uint8_t)2, info.Channels);
    EXPECT_EQ((uint8_t)16, info.BitsPerSample);
    EXPECT_EQ((uint8_t)4, info.BytesPerSample);
    EXPECT_EQ((uint32_t)44100, info.Frequency);
    EXPECT_EQ((uint32_t)176223, info.Samples);
}

TEST_F(AudioInfoTest, IdentMp3VBR192WithoutID3)
{
    pplib::File file;
    OpenTestFile(file, "test_192vbr.mp3");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(pplib::AudioInfo::MP3, info.Format);
    EXPECT_FALSE(info.HaveID3v2Tag);
    EXPECT_EQ((uint32_t)0, info.ID3v2TagStart);
    EXPECT_TRUE(info.IsVBR);
    EXPECT_EQ((uint16_t)176, info.Bitrate);
    EXPECT_EQ((uint8_t)2, info.Channels);
    EXPECT_EQ((uint8_t)16, info.BitsPerSample);
    EXPECT_EQ((uint8_t)4, info.BytesPerSample);
    EXPECT_EQ((uint32_t)44100, info.Frequency);
    EXPECT_EQ((uint32_t)176256, info.Samples);
}

TEST_F(AudioInfoTest, IdentMp3CBR192WithID3)
{
    pplib::File file;
    OpenTestFile(file, "test_192cbr_tagged.mp3");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(pplib::AudioInfo::MP3, info.Format);
    EXPECT_TRUE(info.HaveID3v2Tag);
    EXPECT_EQ((uint32_t)0, info.ID3v2TagStart);
    EXPECT_FALSE(info.IsVBR);
    EXPECT_EQ((uint16_t)192, info.Bitrate);
    EXPECT_EQ((uint8_t)2, info.Channels);
    EXPECT_EQ((uint8_t)16, info.BitsPerSample);
    EXPECT_EQ((uint8_t)4, info.BytesPerSample);
    EXPECT_EQ((uint32_t)44100, info.Frequency);
    EXPECT_EQ((uint32_t)176223, info.Samples);
}

TEST_F(AudioInfoTest, IdentAiffWithoutID3)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz.aiff");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info)) << "Audio format was not recognized";
    EXPECT_EQ(pplib::AudioInfo::AIFF, info.Format) << "Wrong audio format detected";
    EXPECT_FALSE(info.HaveID3v2Tag) << "ID3-Tag detected, but there is none in the file";
    EXPECT_EQ((uint32_t)0, info.ID3v2TagStart);
    EXPECT_FALSE(info.IsVBR) << "VBR detected, but it shouldn't";
    EXPECT_EQ((uint16_t)1411, info.Bitrate) << "Unexpected Bitrate";
    EXPECT_EQ((uint8_t)2, info.Channels) << "Unexpected number of channels";
    EXPECT_EQ((uint8_t)16, info.BitsPerSample) << "Unexpected Bits per Sample";
    EXPECT_EQ((uint8_t)4, info.BytesPerSample) << "Unexpected bytes per sample";
    EXPECT_EQ((uint32_t)44100, info.Frequency) << "Unexpected frequency";
    EXPECT_EQ((uint32_t)173695, info.Samples) << "Unexpected number of samples";
    EXPECT_EQ((uint32_t)3938, info.Length) << "Unexpected length";
    EXPECT_EQ((uint32_t)694834, info.FileSize) << "Unexpected filesize";
    EXPECT_EQ((uint32_t)54, info.AudioStart) << "Unexpected audio start";
    EXPECT_EQ((uint32_t)694833, info.AudioEnd) << "Unexpected audio end";
    EXPECT_EQ((uint32_t)694780, info.AudioSize) << "Unexpected audio size";

}

TEST_F(AudioInfoTest, IdentAiffWithID3)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz_tagged.aiff");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info)) << "Audio format was not recognized";
    EXPECT_EQ(pplib::AudioInfo::AIFF, info.Format) << "Wrong audio format detected";
    EXPECT_TRUE(info.HaveID3v2Tag) << "ID3-Tag detected, but there is none in the file";
    EXPECT_EQ((uint32_t)694842, info.ID3v2TagStart);
    EXPECT_FALSE(info.IsVBR) << "VBR detected, but it shouldn't";
    EXPECT_EQ((uint16_t)1411, info.Bitrate) << "Unexpected Bitrate";
    EXPECT_EQ((uint8_t)2, info.Channels) << "Unexpected number of channels";
    EXPECT_EQ((uint8_t)16, info.BitsPerSample) << "Unexpected Bits per Sample";
    EXPECT_EQ((uint8_t)4, info.BytesPerSample) << "Unexpected bytes per sample";
    EXPECT_EQ((uint32_t)44100, info.Frequency) << "Unexpected frequency";
    EXPECT_EQ((uint32_t)173695, info.Samples) << "Unexpected number of samples";
    EXPECT_EQ((uint32_t)3938, info.Length) << "Unexpected length";
    EXPECT_EQ((uint32_t)695866, info.FileSize) << "Unexpected filesize";
    EXPECT_EQ((uint32_t)54, info.AudioStart) << "Unexpected audio start";
    EXPECT_EQ((uint32_t)694833, info.AudioEnd) << "Unexpected audio end";
    EXPECT_EQ((uint32_t)694780, info.AudioSize) << "Unexpected audio size";

}

TEST_F(AudioInfoTest, IdentWaveWithoutID3)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz.wav");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info)) << "Audio format was not recognized";
    EXPECT_EQ(pplib::AudioInfo::WAVE, info.Format) << "Wrong audio format detected";
    EXPECT_FALSE(info.HaveID3v2Tag) << "ID3-Tag detected, but there is none in the file";
    EXPECT_EQ((uint32_t)0, info.ID3v2TagStart);
    EXPECT_FALSE(info.IsVBR) << "VBR detected, but it shouldn't";
    EXPECT_EQ((uint16_t)1411, info.Bitrate) << "Unexpected Bitrate";
    EXPECT_EQ((uint8_t)2, info.Channels) << "Unexpected number of channels";
    EXPECT_EQ((uint8_t)16, info.BitsPerSample) << "Unexpected Bits per Sample";
    EXPECT_EQ((uint8_t)4, info.BytesPerSample) << "Unexpected bytes per sample";
    EXPECT_EQ((uint32_t)44100, info.Frequency) << "Unexpected frequency";
    EXPECT_EQ((uint32_t)173695, info.Samples) << "Unexpected number of samples";
    EXPECT_EQ((uint32_t)3938, info.Length) << "Unexpected length";

    EXPECT_EQ((uint32_t)694824, info.FileSize) << "Unexpected filesize";
    EXPECT_EQ((uint32_t)44, info.AudioStart) << "Unexpected audio start";
    EXPECT_EQ((uint32_t)694824 - 44, info.AudioSize) << "Unexpected audio size";
    EXPECT_EQ((uint32_t)694823, info.AudioEnd) << "Unexpected audio end";
}

TEST_F(AudioInfoTest, IdentOggVorbis)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz.wav");
    ASSERT_NE(pplib::AudioInfo::OGG, pplib::IdentAudioFile(file));
    OpenTestFile(file, "test_44kHz.ogg");
    ASSERT_EQ(pplib::AudioInfo::OGG, pplib::IdentAudioFile(file));
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(pplib::AudioInfo::OGG, info.Format);
    EXPECT_FALSE(info.HaveID3v2Tag);
    EXPECT_EQ((uint32_t)0, info.ID3v2TagStart);
    EXPECT_FALSE(info.IsVBR);
    EXPECT_EQ((uint16_t)192, info.Bitrate);
    EXPECT_EQ((uint8_t)2, info.Channels);
    EXPECT_EQ((uint8_t)16, info.BitsPerSample);
    EXPECT_EQ((uint8_t)4, info.BytesPerSample);
    EXPECT_EQ((uint32_t)44100, info.Frequency);
    EXPECT_EQ((uint32_t)173695, info.Samples) << "Unexpected number of samples";
    EXPECT_EQ((uint32_t)3938, info.Length) << "Unexpected length";
    EXPECT_EQ((uint32_t)90768, info.FileSize) << "Unexpected filesize";
    EXPECT_EQ((uint32_t)8060, info.AudioStart) << "Unexpected audio start";
    EXPECT_EQ((uint32_t)90767, info.AudioEnd) << "Unexpected audio end";
    EXPECT_EQ((uint32_t)82708, info.AudioSize) << "Unexpected audio size";

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

/** @brief Minimaler AIFF-Container (FORM/AIFF + COMM + SSND)
 *
 * Die deklarierten Chunk-Groessen (commSize, ssndSize) werden unveraendert in die
 * Datei geschrieben, unabhaengig von der Zahl der tatsaechlich folgenden Bytes.
 * Damit lassen sich defekte oder abgeschnittene Dateien nachbauen.
 */
static std::vector<char> MakeAIFF(uint32_t commSize, uint32_t ssndSize, uint32_t sampleOffset,
                                  size_t audioBytes, uint16_t channels = 2, uint16_t bitdepth = 16,
                                  uint32_t rate = 44100, uint32_t fraction = 0)
{
    std::vector<char> body;
    putTag(body, "COMM");
    put32be(body, commSize);
    put16be(body, channels);
    put32be(body, (uint32_t)(audioBytes / ((bitdepth / 8) * channels)));
    put16be(body, bitdepth);
    putExtended80(body, rate, fraction);

    putTag(body, "SSND");
    put32be(body, ssndSize);
    put32be(body, sampleOffset);
    put32be(body, 0); // blockSize
    for (size_t i = 0; i < audioBytes; i++)
        put8(body, (char)(i & 0xFF));

    std::vector<char> v;
    putTag(v, "FORM");
    put32be(v, (uint32_t)(4 + body.size()));
    putTag(v, "AIFF");
    v.insert(v.end(), body.begin(), body.end());
    return v;
}

/** @brief Minimaler RIFF/WAVE-Container (fmt, optional LIST und id3, dann data)
 *
 * Auch hier werden Chunk-Groessen deklariert, die nicht zum real folgenden Inhalt
 * passen muessen (defekte Dateien).
 */
static std::vector<char> MakeWAVE(uint32_t fmtSize, uint32_t dataSize, size_t audioBytes,
                                  uint16_t channels = 2, uint16_t bitdepth = 16, uint32_t rate = 44100,
                                  bool listChunkBeforeData = false, bool id3ChunkBeforeData = false)
{
    std::vector<char> body;
    putTag(body, "fmt ");
    put32le(body, fmtSize);
    put16le(body, 1); // PCM
    put16le(body, channels);
    put32le(body, rate);
    put32le(body, rate * channels * (bitdepth / 8));
    put16le(body, (uint16_t)((bitdepth / 8) * channels));
    put16le(body, bitdepth);
    for (uint32_t i = 16; i < fmtSize && i < 40; i++) put8(body, 0);

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
        put8(body, 0);          // Minor-Version
        put8(body, 0);          // Flags
        put32le(body, 0);       // synchsafe Groesse: leerer Tag
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
    info.Format = pplib::AudioInfo::MP3;
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
    info.Mode = pplib::AudioInfo::JOINT_STEREO;
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
    EXPECT_EQ(pplib::AudioInfo::UNKNOWN, info.Format);
    EXPECT_FALSE(info.HaveID3v2Tag);
    EXPECT_FALSE(info.IsVBR);
    EXPECT_EQ((uint32_t)0, info.ID3v2TagStart);
    EXPECT_EQ((uint32_t)0, info.FileSize);
    EXPECT_EQ((uint32_t)0, info.AudioStart);
    EXPECT_EQ((uint32_t)0, info.AudioEnd);
    EXPECT_EQ((uint32_t)0, info.AudioSize);
    EXPECT_EQ((uint32_t)0, info.Samples);
    EXPECT_EQ((uint32_t)0, info.Length);
    EXPECT_EQ((uint32_t)0, info.Frequency);
    EXPECT_EQ((uint16_t)0, info.Bitrate);
    EXPECT_EQ((uint8_t)0, info.BitsPerSample);
    EXPECT_EQ((uint8_t)0, info.BytesPerSample);
    EXPECT_EQ((uint8_t)0, info.Channels);
    EXPECT_EQ(pplib::AudioInfo::STEREO, info.Mode);
}

TEST_F(AudioInfoTest, FormatOverloadMatchesInfoOverload)
{
    struct Entry {
        const char* path;
        pplib::AudioInfo::AudioFormat format;
        bool identifiable = true;
    } matrix[] = {
        {"testdata/audio/test_192cbr.mp3", pplib::AudioInfo::MP3},
        {"testdata/audio/test_44kHz.wav", pplib::AudioInfo::WAVE},
        {"testdata/audio/test_44kHz.aiff", pplib::AudioInfo::AIFF},
        {"testdata/audio/test_44kHz.ogg", pplib::AudioInfo::OGG},
        {"testdata/audio/test_44kHz_tagged.wav", pplib::AudioInfo::WAVE},
        {"testdata/audio/test_44kHz_tagged.aiff", pplib::AudioInfo::AIFF},
        {"testdata/audio/test_192cbr_tagged.mp3", pplib::AudioInfo::MP3},
        {"testdata/audio/test_192cbr_taggedWithCover.mp3", pplib::AudioInfo::MP3},
        {"testdata/audio/tags_at_eof.aiff", pplib::AudioInfo::AIFF},
        {"testdata/audio/cover.jpg", pplib::AudioInfo::UNKNOWN, false},
        // Ogg-Container wird erkannt, aber der Vorbis-Decoder scheitert an Opus.
        {"testdata/audio/test_48kHz.opus", pplib::AudioInfo::OGG, false},
    };
    for (Entry& entry : matrix) {
        pplib::File file;
        ASSERT_NO_THROW(file.open(entry.path)) << entry.path;
        ASSERT_TRUE(file.isOpen()) << entry.path;
        pplib::AudioInfo info;
        EXPECT_EQ(entry.format, pplib::IdentAudioFile(file)) << entry.path;
        if (!entry.identifiable) {
            ASSERT_FALSE(pplib::IdentAudioFile(file, info)) << entry.path;
            EXPECT_EQ(pplib::AudioInfo::UNKNOWN, info.Format) << entry.path;
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
    EXPECT_EQ(pplib::AudioInfo::UNKNOWN, pplib::IdentAudioFile(file));
}

TEST_F(AudioInfoTest, EmptyAndTruncatedFiles)
{
    std::vector<char> empty(8, 0);
    pplib::MemFile fileEmpty(empty.data(), 0);
    EXPECT_NO_THROW({
        pplib::AudioInfo info = SentinelInfo();
        ASSERT_FALSE(pplib::IdentAudioFile(fileEmpty, info));
    });
    EXPECT_EQ(pplib::AudioInfo::UNKNOWN, pplib::IdentAudioFile(fileEmpty));

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

TEST_F(AudioInfoTest, WaveDataChunkBeyondEof)
{
    // data-Chunk kuendigt 1 MByte an, real folgen nur 8 Byte Audiodaten
    std::vector<char> wave = MakeWAVE(16, 1024 * 1024, 8);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint32_t)(wave.size() - 44), info.AudioSize);
    EXPECT_EQ((uint32_t)(wave.size() - 1), info.AudioEnd);
    EXPECT_EQ((uint32_t)2, info.Samples); // 8 Byte / 4 Byte je Sample
}

TEST_F(AudioInfoTest, WaveListChunkBeforeData)
{
    std::vector<char> wave = MakeWAVE(16, 8, 8, 2, 16, 22050, true, false);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info)) << "LIST-Chunk zwischen fmt und data muss uebersprungen werden";
    EXPECT_EQ((uint32_t)22050, info.Frequency);
    EXPECT_EQ((uint32_t)66, info.AudioStart) << "Standard-44 plus 22 Byte LIST-Chunk";
    EXPECT_EQ((uint32_t)8, info.AudioSize);
    EXPECT_EQ((uint32_t)2, info.Samples);
}

TEST_F(AudioInfoTest, WaveId3ChunkBeforeData)
{
    std::vector<char> wave = MakeWAVE(16, 8, 8, 2, 16, 44100, false, true);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_TRUE(info.HaveID3v2Tag);
    EXPECT_EQ((uint32_t)44, info.ID3v2TagStart) << "Anfang des ID3v2-Tags im id3-Chunk erwartet";
    EXPECT_EQ((uint32_t)62, info.AudioStart);
    EXPECT_EQ((uint32_t)8, info.AudioSize);
}

TEST_F(AudioInfoTest, Wave20BitMono)
{
    std::vector<char> wave = MakeWAVE(16, 12, 12, 1, 20, 8000, false, false);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint8_t)1, info.Channels);
    EXPECT_EQ(pplib::AudioInfo::MONO, info.Mode);
    EXPECT_EQ((uint8_t)20, info.BitsPerSample);
    EXPECT_EQ((uint8_t)2, info.BytesPerSample);
    EXPECT_EQ((uint32_t)6, info.Samples);
}

TEST_F(AudioInfoTest, Wave8BitMono48kHzExtensibleFmt)
{
    // 8 Bit Mono bei 48 kHz, fmt-Chunk mit 40 Byte (EXTENSIBLE-artig)
    std::vector<char> wave = MakeWAVE(40, 12, 12, 1, 8, 48000);
    pplib::MemFile file(wave.data(), wave.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint8_t)1, info.Channels);
    EXPECT_EQ(pplib::AudioInfo::MONO, info.Mode);
    EXPECT_EQ((uint8_t)8, info.BitsPerSample);
    EXPECT_EQ((uint8_t)1, info.BytesPerSample);
    EXPECT_EQ((uint32_t)48000, info.Frequency);
    EXPECT_EQ((uint32_t)12, info.Samples);
    EXPECT_EQ((uint16_t)384, info.Bitrate);
    EXPECT_EQ((uint32_t)68, info.AudioStart) << "40 Byte fmt-Payload statt 16";
}

TEST_F(AudioInfoTest, AiffSampleOffsetAndChunkBeyondEof)
{
    // SSND kuendigt 1 MByte an, real folgen 16 Byte; Sample-Offset 8 verschiebt den Beginn
    std::vector<char> aiff = MakeAIFF(18, 1024 * 1024, 8, 16);
    pplib::MemFile file(aiff.data(), aiff.size());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint32_t)62, info.AudioStart) << "SSND-Kopf (8) plus deklariertes Sample-Offset (8)";
    EXPECT_EQ((uint32_t)8, info.AudioSize) << "nur die bis zum Dateiende folgenden Bytes";
    EXPECT_EQ((uint32_t)(aiff.size() - 1), info.AudioEnd);
    EXPECT_EQ((uint32_t)aiff.size(), info.FileSize);
    EXPECT_EQ((uint32_t)4, info.Samples) << "COMM deklariert 4 Samples";
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

TEST_F(AudioInfoTest, AiffCommChunkBeyondEof)
{
    std::vector<char> aiff = MakeAIFF(0x7FFFFFFF, 8, 0, 8);
    pplib::MemFile file(aiff.data(), aiff.size());
    EXPECT_NO_THROW({
        pplib::AudioInfo info = SentinelInfo();
        ASSERT_FALSE(pplib::IdentAudioFile(file, info));
    });
    // Container ist erkennbar, die Auswertung der Kommponenten scheitert
    EXPECT_EQ(pplib::AudioInfo::AIFF, pplib::IdentAudioFile(file));
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
    EXPECT_EQ(pplib::AudioInfo::UNKNOWN, pplib::IdentAudioFile(file));
    pplib::AudioInfo info;
    ASSERT_FALSE(pplib::IdentAudioFile(file, info));
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
        hdr[1] = (char)0xFB;          // Sync, MPEG-1, Layer III, ohne CRC
        hdr[2] = (char)0x20;          // Bitrate-Index 2 (32 kbit/s), 44,1 kHz
        hdr[3] = (char)(mode << 6);   // Mode-Feld, keine Mode-Erweiterung
    }
    return data;
}

TEST_F(AudioInfoTest, Mp3ModeFieldSelectsChannelMode)
{
    struct {
        uint8_t mode;
        pplib::AudioInfo::ChannelMode expect;
        uint8_t channels;
    } modes[] = {
        {(uint8_t)0, pplib::AudioInfo::STEREO, 2},
        {(uint8_t)2, pplib::AudioInfo::DUAL_CHANNEL, 2},
        {(uint8_t)3, pplib::AudioInfo::MONO, 1},
    };
    for (auto& m : modes) {
        std::vector<char> mp3 = MakeMP3(m.mode);
        pplib::MemFile file(mp3.data(), mp3.size());
        pplib::AudioInfo info = SentinelInfo();
        ASSERT_TRUE(pplib::IdentAudioFile(file, info)) << "Mode-Feld " << (unsigned)m.mode;
        EXPECT_EQ(pplib::AudioInfo::MP3, info.Format);
        EXPECT_EQ(m.expect, info.Mode) << "Mode-Feld " << (unsigned)m.mode;
        EXPECT_EQ(m.channels, info.Channels);
        EXPECT_EQ((uint32_t)mp3.size(), info.FileSize);
        EXPECT_EQ((uint32_t)0, info.AudioStart);
        EXPECT_EQ((uint32_t)mp3.size() - 1, info.AudioEnd);
        EXPECT_EQ((uint32_t)mp3.size(), info.AudioSize);
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
    EXPECT_EQ(pplib::AudioInfo::MP3, info.Format);
    EXPECT_TRUE(info.HaveID3v2Tag);
    // Das ID3v2-Tag steht am Dateianfang, die Audiodaten beginnen dahinter.
    EXPECT_EQ((uint32_t)0, info.ID3v2TagStart);
    EXPECT_EQ((uint32_t)97072, info.FileSize);
    EXPECT_EQ((uint32_t)1024, info.AudioStart);
    EXPECT_EQ((uint32_t)96943, info.AudioEnd);
    EXPECT_EQ((uint32_t)95920, info.AudioSize);
    EXPECT_EQ((uint32_t)176223, info.Samples);
    EXPECT_EQ(pplib::AudioInfo::JOINT_STEREO, info.Mode);
    file.close();
}

TEST_F(AudioInfoTest, AiffWithTagsAtEndOfFile)
{
    pplib::File file;
    ASSERT_NO_THROW(file.open("testdata/audio/tags_at_eof.aiff"));
    ASSERT_TRUE(file.isOpen());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(pplib::AudioInfo::AIFF, info.Format);
    EXPECT_TRUE(info.HaveID3v2Tag);
    EXPECT_EQ((uint32_t)102204, info.FileSize);
    EXPECT_EQ((uint32_t)90, info.AudioStart);
    EXPECT_EQ((uint32_t)101933, info.AudioEnd);
    EXPECT_EQ((uint32_t)101844, info.AudioSize);
    EXPECT_EQ((uint32_t)101942, info.ID3v2TagStart);
    EXPECT_EQ((uint8_t)2, info.Channels);
    EXPECT_EQ((uint32_t)44100, info.Frequency);
    file.close();
}

TEST_F(AudioInfoTest, OpusInOggContainerIsNotSupported)
{
    // Der Ogg-Container wird erkannt, eine zugehoerige AudioInfo liefert pplib
    // aber nicht: IdentOggVorbisAudioFile() kann keinen Opus-Stream oeffnen.
    // Beide Overloads liefen deshalb unterschiedlich -> Entscheidung offen.
    pplib::File file;
    ASSERT_NO_THROW(file.open("testdata/audio/test_48kHz.opus"));
    ASSERT_TRUE(file.isOpen());
    EXPECT_EQ(pplib::AudioInfo::OGG, pplib::IdentAudioFile(file));
    pplib::AudioInfo expected = SentinelInfo();
    pplib::AudioInfo info = SentinelInfo();
    EXPECT_FALSE(pplib::IdentAudioFile(file, info));
    EXPECT_TRUE(SameInfo(info, expected)) << "info darf bei Misserfolg nicht veraendert werden";
    file.close();
}

} // namespace

