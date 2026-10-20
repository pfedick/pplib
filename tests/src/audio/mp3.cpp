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

#include <clocale>
#include <cstring>
#include <vector>

#include <pplib/core/dir.h>
#include <pplib/core/file.h>
#include <pplib/core/functions.h>
#include <pplib/core/memfile.h>
#include <pplib/exceptions.h>
#include <pplib/types/bytearray.h>
#include <pplib/audio/mp3.h>

#include "pplib-tests.h"

namespace
{

using pplib::PPL_MPEG_HEADER;

//!\brief Beschreibung eines MPEG-Frames, wie er fuer die Tests zusammengebaut wird
struct FrameParams
{
    int mpegVersion; //!< 1 = MPEG-1, 2 = MPEG-2, 3 = MPEG-2.5
    int layer;       //!< 2 oder 3
    int bitrate;     //!< kbit/s
    int frequency;   //!< Hz
    int padding;     //!< 0 oder 1
};

//!\brief Referenz-Groesse eines Frames nach ISO/IEC 11172-3 (Padding additiv)
size_t refFrameSize(const FrameParams& p)
{
    return 144 * (size_t)p.bitrate * 1000 / (size_t)p.frequency + (size_t)p.padding;
}

//!\brief Samples pro Frame (Referenz, unabhaengig von der Bibliothek)
size_t refSamplesPerFrame(int mpegVersion)
{
    return (mpegVersion == 1) ? 1152 : 576;
}

//!\brief Die 2 Bit der Versionskennung im Header
int versionBits(int mpegVersion)
{
    switch (mpegVersion) {
        case 3: return 0;
        case 2: return 2;
        case 1: return 3;
        default: return 1; // reserviert
    }
}

// Bitraten nach ISO/IEC 11172-3 bzw. 13818-3, Schicht III
const int MPEG1_L3_BITRATES[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320};
const int MPEG2_L3_BITRATES[] = {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160};

// Bitraten fuer Schicht II
const int MPEG1_L2_BITRATES[] = {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384};
const int MPEG2_L2_BITRATES[] = {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160};

int bitrateIndex(const FrameParams& p)
{
    const int* table = (p.mpegVersion == 1) ? MPEG1_L3_BITRATES : MPEG2_L3_BITRATES;
    if (p.layer == 2) table = (p.mpegVersion == 1) ? MPEG1_L2_BITRATES : MPEG2_L2_BITRATES;
    for (int i = 1; i < 15; i++) {
        if (table[i] == p.bitrate) return i;
    }
    return 0;
}

int frequencyBits(const FrameParams& p)
{
    switch (p.frequency) {
        case 44100:
        case 22050:
        case 11025: return 0;
        case 48000:
        case 24000:
        case 8000: return 1;
        case 32000:
        case 16000: return 2;
        default: return 3; // reserviert
    }
}

//!\brief Frame mit beliebigem Header und genullter Payload anhaengen
/*! Der Payload wird mit 0x40 gefüllt, damit im Frame selbst kein
 *  Frame-Sync-Wort (0xFF Ex) entstehen kann. */
void appendRawFrame(pplib::ByteArray& data, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, size_t totalSize)
{
    const uint8_t hdr[4] = {b0, b1, b2, b3};
    data.append(hdr, 4);
    if (totalSize > 4) {
        std::vector<uint8_t> payload(totalSize - 4, 0x40);
        data.append(payload.data(), payload.size());
    }
}

//!\brief Gueltigen MPEG-Frame gemäss FrameParams anhaengen
void appendFrame(pplib::ByteArray& data, const FrameParams& p)
{
    const uint8_t b0 = 0xFF;
    const uint8_t b1 = (uint8_t)(0xE0 | (versionBits(p.mpegVersion) << 3) | ((4 - p.layer) << 1) | 1);
    const uint8_t b2 = (uint8_t)((bitrateIndex(p) << 4) | (frequencyBits(p) << 2) | ((p.padding & 1) << 1));
    const uint8_t b3 = 0xC0; // Stereo, kein Copyright/Original/Emphasis
    appendRawFrame(data, b0, b1, b2, b3, refFrameSize(p));
}

//!\brief 10-Byte-ID3v2-Header plus Body anhaengen
void appendId3v2(pplib::ByteArray& data, uint8_t version, uint8_t flags, const uint8_t sizeBytes[4], size_t bodySize)
{
    const uint8_t hdr[6] = {'I', 'D', '3', version, flags, 0};
    data.append(hdr, 6);
    data.append(sizeBytes, 4);
    if (bodySize > 0) {
        std::vector<uint8_t> body(bodySize, 0x20);
        data.append(body.data(), bodySize);
    }
}

//!\brief 128-Byte-ID3v1-Tag anhaengen
void appendId3v1(pplib::ByteArray& data)
{
    uint8_t tag[128];
    memset(tag, 0, sizeof(tag));
    tag[0] = 'T';
    tag[1] = 'A';
    tag[2] = 'G';
    data.append(tag, sizeof(tag));
}

//!\brief MPEG-2.5/Layer-III-Frame mit 8000 Hz, Bitrate-Index 1 (8 kbit/s)
FrameParams mpeg25Frame()
{
    return FrameParams{3, 3, 8, 8000, 0};
}

//!\brief FileObject, dessen setMapReadAhead() eine Exception wirft
/*! IdentMPEG() muss damit umgehen koennen, weil nicht jede FileObject-
 *  Subklasse Read-Ahead unterstuetzt. */
class NoReadAheadMemFile : public pplib::MemFile
{
public:
    void setMapReadAhead(size_t) override
    {
        throw pplib::UnimplementedVirtualFunctionException("no read ahead");
    }
};

//!rief FileObject, dessen map() eine Exception wirft
class ThrowingMapMemFile : public pplib::MemFile
{
public:
    char* map(uint64_t, size_t, pplib::FileObject::MapProtection) override
    {
        throw pplib::OverflowException("mappen fehlgeschlagen");
    }
};

class Mp3FunctionsTest : public ::testing::Test
{
protected:
    Mp3FunctionsTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~Mp3FunctionsTest()
    {
    }

    //!\brief ByteArray als geoeffnete MemFile bereitstellen
    void openMem(pplib::ByteArray& data, pplib::MemFile& mf)
    {
        if (data.size() == 0) {
            // MemFile::open() lehnt einen Nullpointer mit Groesse 0 ab
            mf.open(nullptr, 0, true);
        } else {
            mf.open(data.ptr(), data.size(), false);
        }
    }

    //!\brief Alle Felder der Struktur auf einen markierbaren Wert setzen
    void markHeader(PPL_MPEG_HEADER& mpg)
    {
        mpg = PPL_MPEG_HEADER{};
        mpg.start = 0x1111;
        mpg.end = 0x2222;
        mpg.size = 0x3333;
        mpg.filesize = 0x4444;
        mpg.version = 0x55;
        mpg.layer = 0x66;
        mpg.bitrate = 0x77;
        mpg.frequency = 0x88;
        mpg.framesize = 0x99;
        mpg.frames = 0xAA;
        mpg.samples = 0xBB;
        mpg.vbr = true;
    }
};

// --------------------------------------------------------------------------------------
// IdentMPEG() mit den echten Testdateien
// --------------------------------------------------------------------------------------

struct RealFileCase
{
    const char* file;
    uint64_t start;
    uint64_t end;
    uint64_t size;
    uint64_t filesize;
    int version;
    int layer;
    int bitrate;
    int frequency;
    int framesize;
    int frames;
    bool vbr;
    uint64_t samples;
    uint64_t mslength;
};

class Mp3IdentMpegRealFile : public Mp3FunctionsTest, public ::testing::WithParamInterface<RealFileCase>
{
};

TEST_P(Mp3IdentMpegRealFile, IdentMpegFillsHeader)
{
    const RealFileCase& c = GetParam();
    pplib::File f;
    f.open(c.file, pplib::File::FileMode::READ);
    PPL_MPEG_HEADER mpg;
    ASSERT_TRUE(pplib::IdentMPEG(f, &mpg)) << "Kein MP3 in " << c.file;
    f.close();

    EXPECT_EQ(c.filesize, mpg.filesize);
    EXPECT_EQ(c.start, mpg.start);
    EXPECT_EQ(c.end, mpg.end) << "end muss das letzte Byte der Audiodaten sein (inklusiv)";
    EXPECT_EQ(c.size, mpg.size) << "size muss end - start + 1 sein";
    EXPECT_EQ(c.size, mpg.end - mpg.start + 1);
    EXPECT_EQ(c.version, mpg.version);
    EXPECT_EQ(c.layer, mpg.layer);
    EXPECT_EQ(c.bitrate, mpg.bitrate);
    EXPECT_EQ(c.frequency, mpg.frequency);
    EXPECT_EQ(c.framesize, mpg.framesize);
    EXPECT_EQ(c.frames, mpg.frames);
    EXPECT_EQ(c.vbr, mpg.vbr);
    EXPECT_EQ(c.samples, mpg.samples);
    EXPECT_EQ(c.mslength, mpg.mslength);
}

INSTANTIATE_TEST_SUITE_P(
    Mp3Testdata, Mp3IdentMpegRealFile,
    ::testing::Values(
        // datei, start, end, size, filesize, ver, layer, bitrate, freq, framesize, frames, vbr, samples, ms
        RealFileCase{"testdata/audio/test_192cbr.mp3", 0, 95919, 95920, 95920, 1, 3, 192, 44100, 626, 153, false,
                     176223, 3996},
        RealFileCase{"testdata/audio/test_192cbr_tagged.mp3", 1024, 96943, 95920, 97072, 1, 3, 192, 44100, 626, 153,
                     false, 176223, 3996},
        RealFileCase{"testdata/audio/test_192cbr_taggedWithCover.mp3", 29025, 124944, 95920, 125073, 1, 3, 192, 44100,
                     626, 153, false, 176223, 3996},
        RealFileCase{"testdata/audio/test_192vbr.mp3", 0, 87900, 87901, 87901, 1, 3, 176, 44100, 417, 153, true,
                     176256, 3996},
        RealFileCase{"testdata/audio/test_320cbr.mp3", 0, 159867, 159868, 159868, 1, 3, 320, 44100, 1044, 153, false,
                     176223, 3996}));

// --------------------------------------------------------------------------------------
// IdentMPEG() lehnt Nicht-MP3-Daten ab, ohne Exceptions zu werfen
// --------------------------------------------------------------------------------------

class Mp3IdentMpegReject : public Mp3FunctionsTest, public ::testing::WithParamInterface<size_t>
{
};

TEST_P(Mp3IdentMpegReject, NoMp3NoException)
{
    const size_t junkSize = GetParam();
    pplib::ByteArray data;
    if (junkSize > 0) {
        std::vector<uint8_t> junk(junkSize, 0x00);
        data.append(junk.data(), junkSize);
    }
    pplib::MemFile mf;
    openMem(data, mf);

    PPL_MPEG_HEADER mpg;
    markHeader(mpg);
    EXPECT_NO_THROW({ EXPECT_FALSE(pplib::IdentMPEG(mf, &mpg)); });
    mf.close();
    // Bei einem Fehlschlag muss die Struktur vollstaendig zurueckgesetzt sein
    EXPECT_EQ(0u, mpg.start);
    EXPECT_EQ(0u, mpg.end);
    EXPECT_EQ(0u, mpg.size);
    EXPECT_EQ(0u, mpg.filesize);
    EXPECT_EQ(0, mpg.version);
    EXPECT_EQ(0, mpg.bitrate);
    EXPECT_EQ(0u, mpg.samples);
    EXPECT_FALSE(mpg.vbr);
}

// 0 und 3 Byte: kuerzer als der 10-Byte-Header, den IdentMPEG mappen moechte.
// 500 Byte: frueher OverflowException aus map(0, 1024)
INSTANTIATE_TEST_SUITE_P(Mp3Kurzdateien, Mp3IdentMpegReject, ::testing::Values((size_t)0, (size_t)3, (size_t)100, (size_t)500));

TEST_F(Mp3FunctionsTest, IdentMpegMatchesBytePatternsInOtherFormats)
{
    // IdentMPEG() sucht ausschliesslich ein plausibles Frame-Sync-Wort und
    // validiert dessen Header. In WAV-/OGG-Daten kommen solche Byte-Folgen vor,
    // deshalb liefert die Funktion dort true. Die Formatwahl treffen die
    // Aufrufer (AudioInfo::IdentAudioFile, ID3Tag::identAudioFormat), die
    // AIFF/WAV/OGG ausdruecklich vor MPEG pruefen.
    const char* files[] = {"testdata/audio/test_44kHz.wav", "testdata/audio/test_44kHz.ogg"};
    for (const char* file : files) {
        pplib::File f;
        f.open(file, pplib::File::FileMode::READ);
        PPL_MPEG_HEADER mpg;
        markHeader(mpg);
        ASSERT_TRUE(pplib::IdentMPEG(f, &mpg)) << file;
        // Die Audiodaten des Fremdformats werden als MPEG-Frames interpretiert
        EXPECT_GT(mpg.size, 0u) << file;
        EXPECT_EQ(mpg.size, mpg.end - mpg.start + 1) << file;
        f.close();
    }
}

TEST_F(Mp3FunctionsTest, IdentMpegAcceptsNullPointer)
{
    pplib::File f;
    f.open("testdata/audio/test_192cbr.mp3", pplib::File::FileMode::READ);
    ASSERT_NO_THROW({ ASSERT_TRUE(pplib::IdentMPEG(f, nullptr)); });
    f.close();

    pplib::ByteArray junk;
    std::vector<uint8_t> zeros(200, 0x00);
    junk.append(zeros.data(), zeros.size());
    pplib::MemFile mf;
    openMem(junk, mf);
    EXPECT_NO_THROW({ EXPECT_FALSE(pplib::IdentMPEG(mf, nullptr)); });
    mf.close();
}

TEST_F(Mp3FunctionsTest, IdentMpegIsRepeatable)
{
    pplib::File f;
    f.open("testdata/audio/test_192cbr_tagged.mp3", pplib::File::FileMode::READ);
    PPL_MPEG_HEADER first;
    ASSERT_TRUE(pplib::IdentMPEG(f, &first));
    PPL_MPEG_HEADER second;
    ASSERT_TRUE(pplib::IdentMPEG(f, &second));
    f.close();
    EXPECT_EQ(first.start, second.start);
    EXPECT_EQ(first.end, second.end);
    EXPECT_EQ(first.size, second.size);
    EXPECT_EQ(first.frames, second.frames);
}

TEST_F(Mp3FunctionsTest, IdentMpegToleratesUnsupportedReadAhead)
{
    pplib::ByteArray data;
    for (int i = 0; i < 5; i++) {
        appendFrame(data, FrameParams{1, 3, 192, 44100, 0});
    }
    NoReadAheadMemFile mf;
    mf.open(data.ptr(), data.size(), false);
    PPL_MPEG_HEADER mpg;
    ASSERT_NO_THROW({ ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg)); });
    EXPECT_EQ(0u, mpg.start);
    EXPECT_EQ(5, mpg.frames);
    mf.close();
}

// --------------------------------------------------------------------------------------
// IdentMPEG(): ID3v2-Tag vor den Audiodaten
// --------------------------------------------------------------------------------------
// Der Tag-Body enthaelt einen gueltigen MPEG-Frame. Wird der Tag akzeptiert,
// liegen die Audiodaten hinter dem Tag, anderenfalls mitten im Tag.

struct Id3v2Case
{
    uint8_t version;
    uint8_t flags;
    uint8_t sizeBytes[4];
    size_t bodySize;
    uint64_t expectedStart;
};

class Mp3IdentMpegId3v2 : public Mp3FunctionsTest, public ::testing::WithParamInterface<Id3v2Case>
{
};

TEST_P(Mp3IdentMpegId3v2, TagSizeAcceptedOrRejected)
{
    const Id3v2Case& c = GetParam();
    // 10-Byte-Header, im Body ein MPEG-Frame plus 10 Byte Fuellung
    pplib::ByteArray data;
    appendId3v2(data, c.version, c.flags, c.sizeBytes, 0);
    appendFrame(data, FrameParams{1, 3, 192, 44100, 0});
    std::vector<uint8_t> fill(c.bodySize - 626, 0x00);
    data.append(fill.data(), fill.size());
    // Erst nach dem Tag beginnen die eigentlichen Audiodaten
    appendFrame(data, FrameParams{1, 3, 192, 44100, 0});

    pplib::MemFile mf;
    openMem(data, mf);
    PPL_MPEG_HEADER mpg;
    ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg));
    mf.close();
    EXPECT_EQ(c.expectedStart, mpg.start);
}

INSTANTIATE_TEST_SUITE_P(
    Mp3Id3v2, Mp3IdentMpegId3v2,
    ::testing::Values(
        // gueltiger ID3v2.3-Tag, Groesse synchsafe (636 = 0x27C -> 0x04<<7|0x7C)
        Id3v2Case{3, 0x00, {0x00, 0x00, 0x04, 0x7C}, 636, 646},
        // ID3v2.4 mit Extended-Header-Flag (Bit 6)
        Id3v2Case{4, 0x40, {0x00, 0x00, 0x04, 0x7C}, 636, 646},
        // ID3v2.3 mit gesetztem Reserve-Bit in Byte 5 (vom Reader der Bibliothek akzeptiert)
        Id3v2Case{3, 0x00, {0x00, 0x00, 0x04, 0x7C}, 636, 646},
        // ID3v2.2: Groesse in 3 Byte, Byte 9 wird ignoriert
        Id3v2Case{2, 0x00, {0x00, 0x02, 0x7C, 0xFF}, 636, 646},
        // Version 5 ist ungueltig -> Frame im Tag-Body wird gefunden
        Id3v2Case{5, 0x00, {0x00, 0x00, 0x04, 0x7C}, 636, 10},
        // unsaubere synchsafe-Codierung (Byte >= 0x80)
        Id3v2Case{3, 0x00, {0x00, 0x00, 0x84, 0x7C}, 636, 10},
        // Flag-Bit 0x10 ist in ID3v2.4 nicht definiert
        Id3v2Case{4, 0x10, {0x00, 0x00, 0x04, 0x7C}, 636, 10},
        // ID3v2.2 kennt keine Flag-Bits
        Id3v2Case{2, 0x80, {0x00, 0x02, 0x7C, 0xFF}, 636, 10},
        // deklarierte Tag-Groesse liegt hinter dem Dateiende
        Id3v2Case{3, 0x00, {0x7F, 0x7F, 0x7F, 0x7F}, 636, 10}));

// --------------------------------------------------------------------------------------
// IdentMPEG(): ID3v1-Tag am Dateiende
// --------------------------------------------------------------------------------------

class Mp3IdentMpegId3v1 : public Mp3FunctionsTest, public ::testing::WithParamInterface<size_t>
{
};

TEST_P(Mp3IdentMpegId3v1, TagsAreExcludedFromAudioData)
{
    const size_t tagCount = GetParam();
    pplib::ByteArray data;
    for (int i = 0; i < 5; i++) {
        appendFrame(data, FrameParams{1, 3, 192, 44100, 0});
    }
    const size_t audioSize = 5 * 626;
    for (size_t i = 0; i < tagCount; i++) {
        appendId3v1(data);
    }

    pplib::MemFile mf;
    openMem(data, mf);
    PPL_MPEG_HEADER mpg;
    ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg));
    mf.close();

    // hochstens zwei Tags werden erkannt, end zeigt auf das letzte Byte der Audiodaten
    const size_t stripped = (tagCount <= 2) ? tagCount : 2;
    EXPECT_EQ(audioSize + (tagCount - stripped) * 128 - 1, mpg.end);
    EXPECT_EQ(audioSize + (tagCount - stripped) * 128, mpg.size);
    EXPECT_EQ(audioSize + tagCount * 128, mpg.filesize);
    EXPECT_EQ(mpg.size, mpg.end - mpg.start + 1);
}

// 0, 1, 2 und 3 Tags am Dateiende
INSTANTIATE_TEST_SUITE_P(Mp3Id3v1, Mp3IdentMpegId3v1, ::testing::Values((size_t)0, (size_t)1, (size_t)2, (size_t)3));

// --------------------------------------------------------------------------------------
// IdentMPEG(): Framesize in Abhaengigkeit von Bitrate, Frequenz und Padding
// --------------------------------------------------------------------------------------

struct FrameSizeCase
{
    FrameParams frame;
    int expectedFrameSize;
};

class Mp3IdentMpegFrameSize : public Mp3FunctionsTest, public ::testing::WithParamInterface<FrameSizeCase>
{
};

TEST_P(Mp3IdentMpegFrameSize, FrameSizeMatchesStandard)
{
    const FrameSizeCase& c = GetParam();
    pplib::ByteArray data;
    for (int i = 0; i < 4; i++) {
        appendFrame(data, c.frame);
    }
    pplib::MemFile mf;
    openMem(data, mf);
    PPL_MPEG_HEADER mpg;
    ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg));
    mf.close();

    EXPECT_EQ(c.expectedFrameSize, mpg.framesize) << "Padding-Bit darf nicht im Nenner landen";
    EXPECT_EQ(c.frame.bitrate, mpg.bitrate);
    EXPECT_EQ(c.frame.frequency, mpg.frequency);
    EXPECT_EQ(4, mpg.frames);
    EXPECT_EQ((size_t)4 * (size_t)c.expectedFrameSize, mpg.size);
}

INSTANTIATE_TEST_SUITE_P(
    Mp3FrameSize, Mp3IdentMpegFrameSize,
    ::testing::Values(FrameSizeCase{{1, 3, 192, 44100, 0}, 626}, FrameSizeCase{{1, 3, 192, 44100, 1}, 627},
                      FrameSizeCase{{1, 3, 128, 44100, 0}, 417}, FrameSizeCase{{1, 3, 128, 44100, 1}, 418},
                      FrameSizeCase{{2, 3, 32, 22050, 0}, 208}, FrameSizeCase{{2, 3, 40, 22050, 1}, 262},
                      FrameSizeCase{{1, 2, 192, 44100, 0}, 626},
                      FrameSizeCase{{2, 2, 128, 22050, 1}, 836}));

// --------------------------------------------------------------------------------------
// IdentMPEG(): ungueltige Header-Felder werden verworfen
// --------------------------------------------------------------------------------------

class Mp3IdentMpegInvalidHeader : public Mp3FunctionsTest, public ::testing::WithParamInterface<std::array<uint8_t, 4>>
{
};

TEST_P(Mp3IdentMpegInvalidHeader, InvalidHeaderIsNotMp3)
{
    const std::array<uint8_t, 4> hdr = GetParam();
    pplib::ByteArray data;
    appendRawFrame(data, hdr[0], hdr[1], hdr[2], hdr[3], 630);
    pplib::MemFile mf;
    openMem(data, mf);
    PPL_MPEG_HEADER mpg;
    EXPECT_FALSE(pplib::IdentMPEG(mf, &mpg));
    mf.close();
}

INSTANTIATE_TEST_SUITE_P(
    Mp3Invalid, Mp3IdentMpegInvalidHeader,
    ::testing::Values(
        std::array<uint8_t, 4>{0xFF, 0xFB, 0x00, 0xC0}, // Bitrate-Index 0
        std::array<uint8_t, 4>{0xFF, 0xFB, 0xF0, 0xC0}, // Bitrate-Index 15
        std::array<uint8_t, 4>{0xFF, 0xFB, 0xBC, 0xC0}, // Frequenz-Index 3 (reserviert)
        std::array<uint8_t, 4>{0xFF, 0xF9, 0xB0, 0xC0}, // reservierte Versionskennung
        std::array<uint8_t, 4>{0xFF, 0xF8, 0xB0, 0xC0}, // Layer-Kennung 0 (reserviert)
        std::array<uint8_t, 4>{0xFF, 0x1F, 0xB0, 0xC0}  // unvollstaendiges Sync-Wort
        ));

TEST_F(Mp3FunctionsTest, IdentMpegFindsHeaderAtEndOfFile)
{
    pplib::ByteArray data;
    std::vector<uint8_t> junk(100, 0x40);
    data.append(junk.data(), junk.size());
    // Nur der 4 Byte lange Header, ohne Payload - direkt am Dateiende
    // 0xB0 = Bitrate-Index 11 (192 kbit/s), 44,1 kHz, ohne Padding
    const uint8_t hdr[4] = {0xFF, 0xFB, 0xB0, 0xC0};
    data.append(hdr, 4);

    pplib::MemFile mf;
    openMem(data, mf);
    PPL_MPEG_HEADER mpg;
    ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg)) << "Header an der letzten moeglichen Position wird uebersehen";
    mf.close();
    EXPECT_EQ(100u, mpg.start);
    EXPECT_EQ(103u, mpg.end);
    EXPECT_EQ(4u, mpg.size);
    EXPECT_EQ(0, mpg.frames);
}

TEST_F(Mp3FunctionsTest, IdentMpegReadsModeAndFlagBits)
{
    struct ModeCase
    {
        uint8_t b3;
        int mode;
        int stereo;
    };
    const ModeCase cases[] = {{0x00, 0, 2}, {0x40, 1, 2}, {0x80, 2, 2}, {0xC0, 3, 1}};
    for (const ModeCase& c : cases) {
        pplib::ByteArray data;
        // Bit 0 in Byte 1 = 0 -> CRC-geschuetzt; Copyright/Original/Emphasis gesetzt
        appendRawFrame(data, 0xFF, 0xFA, 0xB0, (uint8_t)(c.b3 | 0x0E), 626);
        pplib::MemFile mf;
        openMem(data, mf);
        PPL_MPEG_HEADER mpg;
        ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg));
        mf.close();
        EXPECT_EQ(c.mode, mpg.mode);
        EXPECT_EQ(c.stereo, mpg.stereo);
        EXPECT_EQ(1, mpg.error_protection);
        EXPECT_EQ(1, mpg.copyright);
        EXPECT_EQ(1, mpg.original);
        EXPECT_EQ(2, mpg.emphasis);
    }
}

// --------------------------------------------------------------------------------------
// CheckVBR(): Bitratenwechsel und Samples pro Frame
// --------------------------------------------------------------------------------------

struct VbrCase
{
    int mpegVersion;
    int bitrateA;
    int bitrateB;
    int frequency;
    size_t frames;
    bool expectVbr;
};

class Mp3CheckVbr : public Mp3FunctionsTest, public ::testing::WithParamInterface<VbrCase>
{
};

TEST_P(Mp3CheckVbr, VbrDetectionAndSampleCount)
{
    const VbrCase& c = GetParam();
    pplib::ByteArray data;
    for (size_t i = 0; i < c.frames; i++) {
        const int bitrate = (c.expectVbr && (i % 2)) ? c.bitrateB : c.bitrateA;
        appendFrame(data, FrameParams{c.mpegVersion, 3, bitrate, c.frequency, 0});
    }
    pplib::MemFile mf;
    openMem(data, mf);
    PPL_MPEG_HEADER mpg;
    ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg));
    mf.close();

    EXPECT_EQ(c.expectVbr, mpg.vbr);
    if (!c.expectVbr) return;

    // Nach der Korrektur: MPEG-1 = 1152 Samples/Frame, MPEG-2/2.5 = 576
    const size_t samples = c.frames * refSamplesPerFrame(c.mpegVersion);
    EXPECT_EQ((int)c.frames, mpg.frames);
    EXPECT_EQ(samples, mpg.samples);
    EXPECT_EQ(samples * 1000 / (size_t)c.frequency, mpg.mslength);
    EXPECT_EQ((c.bitrateA + c.bitrateB) / 2, mpg.bitrate);
}

INSTANTIATE_TEST_SUITE_P(
    Mp3Vbr, Mp3CheckVbr,
    ::testing::Values(VbrCase{1, 128, 192, 44100, 6, true},   // MPEG-1, 1152 Samples/Frame
                      VbrCase{2, 32, 40, 22050, 6, true},     // MPEG-2, 576 Samples/Frame
                      VbrCase{3, 8, 16, 8000, 6, true},       // MPEG-2.5, 576 Samples/Frame
                      VbrCase{1, 192, 192, 44100, 6, false},  // konstante Bitrate
                      VbrCase{2, 40, 40, 22050, 60, false})); // lange CBR-Datei (Suchabbruch)

// --------------------------------------------------------------------------------------
// GetMP3Frame()
// --------------------------------------------------------------------------------------

class Mp3GetFrameRealFile : public Mp3FunctionsTest, public ::testing::WithParamInterface<const char*>
{
};

TEST_P(Mp3GetFrameRealFile, IteratesAllFramesOfAudioData)
{
    const char* file = GetParam();
    pplib::File f;
    f.open(file, pplib::File::FileMode::READ);
    PPL_MPEG_HEADER mpg;
    ASSERT_TRUE(pplib::IdentMPEG(f, &mpg));
    ASSERT_NO_THROW(f.seek(mpg.start));

    const uint64_t audioSize = mpg.size;
    pplib::ByteArray buffer;
    size_t count = 0;
    size_t total = 0;
    while (true) {
        pplib::GetMP3Frame(f, mpg, buffer);
        if (buffer.size() == 0) break;
        ASSERT_LT(count, 1000u) << "Endlosschleife in " << file;
        // Kein Frame darf in einen ID3v1-Tag hineinlesen
        EXPECT_NE(0, memcmp(buffer.ptr(), "TAG", 3)) << "Frame ueberlappt dem ID3v1-Tag";
        EXPECT_EQ((size_t)mpg.framesize, buffer.size());
        count++;
        total += buffer.size();
        ASSERT_EQ(mpg.start + (size_t)mpg.framesize, f.tell()) << "Dateizeiger steht nicht hinter dem Frame";
    }
    f.close();
    EXPECT_EQ(153u, count);
    EXPECT_EQ(audioSize, total) << "Die Frames decken nicht exakt die Audiodaten ab";
}

INSTANTIATE_TEST_SUITE_P(
    Mp3RealFrames, Mp3GetFrameRealFile,
    ::testing::Values("testdata/audio/test_192cbr.mp3", "testdata/audio/test_192cbr_tagged.mp3",
                      "testdata/audio/test_192cbr_taggedWithCover.mp3", "testdata/audio/test_192vbr.mp3",
                      "testdata/audio/test_320cbr.mp3"));

TEST_F(Mp3FunctionsTest, GetFrameSkipsGarbageBeforeFrame)
{
    pplib::ByteArray data;
    std::vector<uint8_t> junk(300, 0x40);
    data.append(junk.data(), junk.size());
    appendFrame(data, FrameParams{1, 3, 192, 44100, 0});
    data.append(junk.data(), junk.size());

    pplib::MemFile mf;
    openMem(data, mf);
    PPL_MPEG_HEADER mpg;
    ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg));
    ASSERT_NO_THROW(mf.seek(0));

    pplib::ByteArray buffer;
    pplib::GetMP3Frame(mf, mpg, buffer);
    EXPECT_EQ(626u, buffer.size());
    EXPECT_EQ(300u, mpg.start);
    EXPECT_EQ(926u, mf.tell());
    mf.close();
}

TEST_F(Mp3FunctionsTest, GetFrameClampsToAudioEnd)
{
    pplib::ByteArray data;
    // Header einer 626-Byte-Frames, aber nur 616 Byte vorhanden
    appendRawFrame(data, 0xFF, 0xFB, 0xB0, 0xC0, 616);
    pplib::MemFile mf;
    openMem(data, mf);
    PPL_MPEG_HEADER mpg;
    ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg));
    ASSERT_NO_THROW(mf.seek(0));

    pplib::ByteArray buffer;
    ASSERT_NO_THROW(pplib::GetMP3Frame(mf, mpg, buffer));
    EXPECT_EQ(616u, buffer.size()) << "Frame darf nicht ueber das Datenende hinauslesen";
    ASSERT_NO_THROW(pplib::GetMP3Frame(mf, mpg, buffer));
    EXPECT_EQ(0u, buffer.size());
    mf.close();
}

TEST_F(Mp3FunctionsTest, GetFrameClampsStaleEndToFileSize)
{
    pplib::ByteArray data;
    appendFrame(data, FrameParams{1, 3, 192, 44100, 0});
    pplib::MemFile mf;
    openMem(data, mf);
    PPL_MPEG_HEADER mpg;
    ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg));
    // Struktur enthaelt ein veraltetes Ende jenseits der Datei
    mpg.end = data.size() + 5000;
    ASSERT_NO_THROW(mf.seek(0));

    pplib::ByteArray buffer;
    ASSERT_NO_THROW(pplib::GetMP3Frame(mf, mpg, buffer));
    EXPECT_EQ(626u, buffer.size());
    // Der Dateizeiger darf nicht hinter das Dateiende gestellt werden
    ASSERT_NO_THROW(mf.seek(mpg.start + (size_t)mpg.framesize));
    mf.close();
}

TEST_F(Mp3FunctionsTest, GetFrameKeepsHeaderOnFailure)
{
    pplib::ByteArray data;
    std::vector<uint8_t> junk(400, 0x40);
    data.append(junk.data(), junk.size());
    pplib::MemFile mf;
    openMem(data, mf);

    PPL_MPEG_HEADER mpg;
    markHeader(mpg);
    pplib::ByteArray buffer;
    ASSERT_NO_THROW(pplib::GetMP3Frame(mf, mpg, buffer));
    EXPECT_EQ(0u, buffer.size());
    // Die Struktur darf nicht loeschend zurueckgesetzt werden
    EXPECT_EQ(0x1111u, mpg.start);
    EXPECT_EQ(0x2222u, mpg.end);
    EXPECT_EQ(0x4444u, mpg.filesize);
    EXPECT_EQ(0xAA, mpg.frames);
    mf.close();
}

TEST_F(Mp3FunctionsTest, GetFrameInitializesEmptyHeaderStruct)
{
    pplib::File f;
    f.open("testdata/audio/test_192cbr.mp3", pplib::File::FileMode::READ);
    PPL_MPEG_HEADER mpg;
    pplib::ByteArray buffer;
    ASSERT_NO_THROW(pplib::GetMP3Frame(f, mpg, buffer));
    EXPECT_EQ(626u, buffer.size());
    EXPECT_EQ(0u, mpg.start);
    EXPECT_EQ(95919u, mpg.end);
    EXPECT_EQ(95920u, mpg.filesize);
    EXPECT_EQ(626, mpg.framesize);
    f.close();
}

TEST_F(Mp3FunctionsTest, GetFrameAtEndOfAudioDataYieldsNothing)
{
    pplib::ByteArray data;
    appendFrame(data, FrameParams{1, 3, 192, 44100, 0});
    appendFrame(data, FrameParams{1, 3, 192, 44100, 0});
    appendId3v1(data);
    pplib::MemFile mf;
    openMem(data, mf);
    PPL_MPEG_HEADER mpg;
    ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg));
    EXPECT_EQ(2 * 626 - 1, mpg.end);

    // Zwei Bytes vor dem Datenende steht kein gueltiger Header mehr
    ASSERT_NO_THROW(mf.seek(mpg.end - 1));
    pplib::ByteArray buffer;
    ASSERT_NO_THROW(pplib::GetMP3Frame(mf, mpg, buffer));
    EXPECT_EQ(0u, buffer.size());
    EXPECT_EQ(mpg.end - 1, mf.tell());
    mf.close();
}


TEST_F(Mp3FunctionsTest, IdentMpegSurvivesFailingMap)
{
    pplib::ByteArray data;
    appendFrame(data, FrameParams{1, 3, 192, 44100, 0});
    ThrowingMapMemFile mf;
    mf.open(data.ptr(), data.size(), false);

    PPL_MPEG_HEADER mpg;
    ASSERT_NO_THROW({ EXPECT_FALSE(pplib::IdentMPEG(mf, &mpg)); });
    EXPECT_EQ(0u, mpg.filesize);
    pplib::ByteArray buffer;
    ASSERT_NO_THROW({ pplib::GetMP3Frame(mf, mpg, buffer); });
    EXPECT_EQ(0u, buffer.size());
    mf.close();
}

TEST_F(Mp3FunctionsTest, GetFrameDoesNotReadIntoId3v1Tag)
{
    // Ein Frame, unbrauchbare Daten und ein ID3v1-Tag, dessen Inhalt
    // zufaellig wie ein Frame-Header aussieht
    pplib::ByteArray data;
    appendFrame(data, FrameParams{1, 3, 192, 44100, 0});
    std::vector<uint8_t> junk(300, 0x40);
    data.append(junk.data(), junk.size());
    uint8_t tag[128];
    memset(tag, 0, sizeof(tag));
    memcpy(tag, "TAG", 3);
    const uint8_t fake[4] = {0xFF, 0xFB, 0xB0, 0xC0};
    memcpy(tag + 10, fake, 4);
    data.append(tag, sizeof(tag));

    pplib::MemFile mf;
    openMem(data, mf);
    PPL_MPEG_HEADER mpg;
    ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg));
    EXPECT_EQ(626u + 300u - 1, mpg.end);

    pplib::ByteArray buffer;
    pplib::GetMP3Frame(mf, mpg, buffer);
    EXPECT_EQ(626u, buffer.size());
    pplib::GetMP3Frame(mf, mpg, buffer);
    EXPECT_EQ(0u, buffer.size()) << "Byte-Muster im ID3v1-Tag wurde als Frame gelesen";
    mf.close();
}

} // namespace
