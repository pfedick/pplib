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
    case 3:
        return 0;
    case 2:
        return 2;
    case 1:
        return 3;
    default:
        return 1; // reserviert
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
    case 11025:
        return 0;
    case 48000:
    case 24000:
    case 8000:
        return 1;
    case 32000:
    case 16000:
        return 2;
    default:
        return 3; // reserviert
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
void appendId3v2(
    pplib::ByteArray& data, uint8_t version, uint8_t flags, const uint8_t sizeBytes[4], size_t bodySize, uint8_t reservedByte = 0)
{
    const uint8_t hdr[6] = {'I', 'D', '3', version, flags, reservedByte};
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

struct Id3v2Case
{
    uint8_t version;
    uint8_t flags;
    uint8_t sizeBytes[4];
    size_t bodySize;
    uint64_t expectedStart;
    uint8_t reservedByte = 0; //!< Byte 6 des Tags (reserviert), laut Spec 0
};

struct FrameSizeCase
{
    FrameParams frame;
    int expectedFrameSize;
};

struct VbrCase
{
    int mpegVersion;
    int bitrateA;
    int bitrateB;
    int frequency;
    size_t frames;
    bool expectVbr;
};

//! @brief FileObject, dessen map() eine Exception wirft
class ThrowingMapMemFile : public pplib::MemFile
{
public:
    char* map(uint64_t, size_t, pplib::FileObject::MapProtection) override
    {
        throw pplib::OutOfBoundsException("mappen fehlgeschlagen");
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

protected:
    //!\brief IdentMPEG() mit einer echten Testdatei, inkl. aller Header-Felder
    void expectRealFile(const RealFileCase& c)
    {
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

    //!\brief Daten mit junkSize Nullbytes: weder MP3 noch Exception, Header bleibt markiert
    void expectNoMp3InJunk(size_t junkSize, const char* what)
    {
        pplib::ByteArray data;
        if (junkSize > 0) {
            std::vector<uint8_t> junk(junkSize, 0x00);
            data.append(junk.data(), junkSize);
        }
        pplib::MemFile mf;
        openMem(data, mf);

        PPL_MPEG_HEADER mpg;
        markHeader(mpg);
        EXPECT_NO_THROW({ EXPECT_FALSE(pplib::IdentMPEG(mf, &mpg)); }) << what;
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

    //!\brief ID3v2-Tag vor den Audiodaten: Groesse entscheidet, wo IdentMPEG() fuendig wird
    void expectId3v2Tag(const Id3v2Case& c, const char* what)
    {
        // 10-Byte-Header, im Body ein MPEG-Frame plus 10 Byte Fuellung
        pplib::ByteArray data;
        appendId3v2(data, c.version, c.flags, c.sizeBytes, 0, c.reservedByte);
        appendFrame(data, FrameParams{1, 3, 192, 44100, 0});
        std::vector<uint8_t> fill(c.bodySize - 626, 0x00);
        data.append(fill.data(), fill.size());
        // Erst nach dem Tag beginnen die eigentlichen Audiodaten
        appendFrame(data, FrameParams{1, 3, 192, 44100, 0});

        pplib::MemFile mf;
        openMem(data, mf);
        PPL_MPEG_HEADER mpg;
        ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg)) << what;
        mf.close();
        EXPECT_EQ(c.expectedStart, mpg.start) << what;
    }

    //!\brief ID3v1-Tags am Dateiende bleiben ausserhalb der Audiodaten
    void expectId3v1Tags(size_t tagCount, const char* what)
    {
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
        ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg)) << what;
        mf.close();
        // hochstens zwei Tags werden erkannt, end zeigt auf das letzte Byte der Audiodaten
        const size_t stripped = (tagCount <= 2) ? tagCount : 2;
        EXPECT_EQ(audioSize + (tagCount - stripped) * 128 - 1, mpg.end) << what;
        EXPECT_EQ(audioSize + (tagCount - stripped) * 128, mpg.size) << what;
        EXPECT_EQ(audioSize + tagCount * 128, mpg.filesize) << what;
        EXPECT_EQ(mpg.size, mpg.end - mpg.start + 1);
    }

    //!\brief Framegroesse in Abhaengigkeit von Bitrate, Frequenz und Padding-Bit
    void expectFrameSize(const FrameSizeCase& c, const char* what)
    {
        pplib::ByteArray data;
        for (int i = 0; i < 4; i++) {
            appendFrame(data, c.frame);
        }
        pplib::MemFile mf;
        openMem(data, mf);
        PPL_MPEG_HEADER mpg;
        ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg)) << what;
        mf.close();

        EXPECT_EQ(c.expectedFrameSize, mpg.framesize) << what << ": Padding-Bit darf nicht im Nenner landen";
        EXPECT_EQ(c.frame.bitrate, mpg.bitrate);
        EXPECT_EQ(c.frame.frequency, mpg.frequency);
        EXPECT_EQ(4, mpg.frames);
        EXPECT_EQ((size_t)4 * (size_t)c.expectedFrameSize, mpg.size);
    }

    //!\brief Frame mit ungueltigem Header-Feld: IdentMPEG() findet kein MP3
    void expectInvalidHeader(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, const char* what)
    {
        pplib::ByteArray data;
        appendRawFrame(data, b0, b1, b2, b3, 630);
        pplib::MemFile mf;
        openMem(data, mf);
        PPL_MPEG_HEADER mpg;
        EXPECT_FALSE(pplib::IdentMPEG(mf, &mpg)) << what;
        mf.close();
    }

    //!\brief Bitratenwechsel fuehrt zu VBR-Korrektur, konstante Bitrate nicht
    void expectVbrState(const VbrCase& c, const char* what)
    {
        pplib::ByteArray data;
        for (size_t i = 0; i < c.frames; i++) {
            const int bitrate = (c.expectVbr && (i % 2)) ? c.bitrateB : c.bitrateA;
            appendFrame(data, FrameParams{c.mpegVersion, 3, bitrate, c.frequency, 0});
        }
        pplib::MemFile mf;
        openMem(data, mf);
        PPL_MPEG_HEADER mpg;
        ASSERT_TRUE(pplib::IdentMPEG(mf, &mpg)) << what;
        mf.close();

        EXPECT_EQ(c.expectVbr, mpg.vbr) << what;
        if (!c.expectVbr) {
            return;
        }
        // Nach der Korrektur: MPEG-1 = 1152 Samples/Frame, MPEG-2/2.5 = 576
        const size_t samples = c.frames * refSamplesPerFrame(c.mpegVersion);
        EXPECT_EQ((int)c.frames, mpg.frames);
        EXPECT_EQ(samples, mpg.samples);
        EXPECT_EQ(samples * 1000 / (size_t)c.frequency, mpg.mslength);
        EXPECT_EQ((c.bitrateA + c.bitrateB) / 2, mpg.bitrate);
    }

    //!\brief GetMP3Frame() liest saemtliche Frames der Audiodaten
    void expectAllFramesOf(const char* file)
    {
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
            if (buffer.size() == 0) {
                break;
            }
            ASSERT_LT(count, 1000u) << "Endlosschleife in " << file;
            // Kein Frame darf in einen ID3v1-Tag hineinlesen
            EXPECT_NE(0, memcmp(buffer.ptr(), "TAG", 3)) << "Frame ueberlappt dem ID3v1-Tag";
            EXPECT_EQ((size_t)mpg.framesize, buffer.size());
            count++;
            total += buffer.size();
            ASSERT_EQ(mpg.start + (size_t)mpg.framesize, f.tell()) << "Dateizeiger steht nicht hinter dem Frame";
        }
        f.close();
        EXPECT_EQ(153u, count) << file;
        EXPECT_EQ(audioSize, total) << "Die Frames decken nicht exakt die Audiodaten ab: " << file;
    }
};

// --------------------------------------------------------------------------------------
// IdentMPEG() mit den echten Testdateien
// --------------------------------------------------------------------------------------

// Die Erwartungswerte je Datei: start, end, size, filesize, Version, Layer,
// Bitrate, Frequenz, Framegroesse, Frames, VBR-Kennung, Samples, Millisekunden
TEST_F(Mp3FunctionsTest, IdentMpegReadsUntaggedCbrFile)
{
    expectRealFile(RealFileCase{"testdata/audio/test_192cbr.mp3", 0, 95919, 95920, 95920, 1, 3, 192, 44100, 626, 153, false, 176223, 3996});
}

TEST_F(Mp3FunctionsTest, IdentMpegSkipsId3v2BeforeAudioData)
{
    expectRealFile(
        RealFileCase{"testdata/audio/test_192cbr_tagged.mp3", 1024, 96943, 95920, 97072, 1, 3, 192, 44100, 626, 153, false, 176223, 3996});
}

TEST_F(Mp3FunctionsTest, IdentMpegSkipsLargeId3v2WithCoverArt)
{
    expectRealFile(RealFileCase{"testdata/audio/test_192cbr_taggedWithCover.mp3", 29025, 124944, 95920, 125073, 1, 3, 192, 44100, 626, 153,
                                false, 176223, 3996});
}

TEST_F(Mp3FunctionsTest, IdentMpegMarksVariableBitrateFile)
{
    expectRealFile(RealFileCase{"testdata/audio/test_192vbr.mp3", 0, 87900, 87901, 87901, 1, 3, 176, 44100, 417, 153, true, 176256, 3996});
}

TEST_F(Mp3FunctionsTest, IdentMpegReadsHighBitrateCbrFile)
{
    expectRealFile(
        RealFileCase{"testdata/audio/test_320cbr.mp3", 0, 159867, 159868, 159868, 1, 3, 320, 44100, 1044, 153, false, 176223, 3996});
}

// --------------------------------------------------------------------------------------
// IdentMPEG() lehnt Nicht-MP3-Daten ab, ohne Exceptions zu werfen
// --------------------------------------------------------------------------------------

// 0 und 3 Byte: kuerzer als der 10-Byte-Header, den IdentMPEG() mappen moechte
TEST_F(Mp3FunctionsTest, IdentMpegRejectsEmptyData)
{
    expectNoMp3InJunk(0, "leere Datei");
}

TEST_F(Mp3FunctionsTest, IdentMpegRejectsThreeBytesOfJunk)
{
    expectNoMp3InJunk(3, "drei Byte");
}

TEST_F(Mp3FunctionsTest, IdentMpegRejectsHundredBytesOfJunk)
{
    expectNoMp3InJunk(100, "hundert Byte ohne Sync-Wort");
}

// Regression: frueher OverflowException aus map(0, 1024) bei Dateien < 1024 Byte
TEST_F(Mp3FunctionsTest, IdentMpegRejectsShortFileWithoutOverflow)
{
    expectNoMp3InJunk(500, "500 Byte");
}

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

// --------------------------------------------------------------------------------------
// IdentMPEG(): ID3v2-Tag vor den Audiodaten
// --------------------------------------------------------------------------------------
// Der Tag-Body enthaelt einen gueltigen MPEG-Frame. Wird der Tag akzeptiert,
// liegen die Audiodaten hinter dem Tag, anderenfalls mitten im Tag.

//!\brief Tag mit Version, Flags, reserviertem Byte und Groessenangabe; Body mit Frame plus Fuellung
TEST_F(Mp3FunctionsTest, IdentMpegAcceptsId3v23Tag)
{
    // Groesse synchsafe codiert: 636 = 0x27C -> 0x04 << 7 | 0x7C
    expectId3v2Tag(Id3v2Case{3, 0x00, {0x00, 0x00, 0x04, 0x7C}, 636, 646}, "gueltiger ID3v2.3-Tag");
}

//!\brief Extended-Header-Flag (Bit 6) ist in ID3v2.4 definiert
TEST_F(Mp3FunctionsTest, IdentMpegAcceptsId3v24ExtendedHeaderFlag)
{
    expectId3v2Tag(Id3v2Case{4, 0x40, {0x00, 0x00, 0x04, 0x7C}, 636, 646}, "ID3v2.4 mit Extended-Header-Flag");
}

//!\brief Bit 0x10 ist in ID3v2.3 nicht definiert, das Tag wird verworfen
TEST_F(Mp3FunctionsTest, IdentMpegIgnoresId3v23ReservedFlagBit)
{
    expectId3v2Tag(Id3v2Case{3, 0x10, {0x00, 0x00, 0x04, 0x7C}, 636, 10}, "ID3v2.3 mit reserviertem Flag-Bit 0x10");
}

//!\brief das reservierte Byte 6 des ID3v2-Headers wird nicht geprueft (abweichend von der Spec)
TEST_F(Mp3FunctionsTest, IdentMpegIgnoresReservedHeaderByte)
{
    expectId3v2Tag(Id3v2Case{3, 0x00, {0x00, 0x00, 0x04, 0x7C}, 636, 646, 0x40}, "ID3v2.3 mit reserviertem Byte 0x40");
}

//!\brief ID3v2.2 codiert die Groesse in 3 Byte, Byte 9 wird ignoriert
TEST_F(Mp3FunctionsTest, IdentMpegAcceptsId3v22ThreeByteSize)
{
    expectId3v2Tag(Id3v2Case{2, 0x00, {0x00, 0x02, 0x7C, 0xFF}, 636, 646}, "ID3v2.2 mit 3-Byte-Groesse");
}

//!\brief Version 5 ist ungueltig, das Frame im Tag-Body wird gefunden
TEST_F(Mp3FunctionsTest, IdentMpegIgnoresId3v25Version)
{
    expectId3v2Tag(Id3v2Case{5, 0x00, {0x00, 0x00, 0x04, 0x7C}, 636, 10}, "ID3v2.5 ist ungueltig");
}

//!\brief unsaubere synchsafe-Codierung (Byte >= 0x80) fuehrt zurueck in den Dateianfang
TEST_F(Mp3FunctionsTest, IdentMpegIgnoresUnsynchsafeTagSize)
{
    expectId3v2Tag(Id3v2Case{3, 0x00, {0x00, 0x00, 0x84, 0x7C}, 636, 10}, "unsaubere synchsafe-Codierung");
}

//!\brief Flag-Bit 0x10 ist in ID3v2.4 nicht definiert
TEST_F(Mp3FunctionsTest, IdentMpegIgnoresId3v24UndefinedFlagBit)
{
    expectId3v2Tag(Id3v2Case{4, 0x10, {0x00, 0x00, 0x04, 0x7C}, 636, 10}, "ID3v2.4 mit undefined Flag-Bit 0x10");
}

//!\brief ID3v2.2 kennt keine Flag-Bits
TEST_F(Mp3FunctionsTest, IdentMpegIgnoresId3v22FlagByte)
{
    expectId3v2Tag(Id3v2Case{2, 0x80, {0x00, 0x02, 0x7C, 0xFF}, 636, 10}, "ID3v2.2 mit Flag-Bit 0x80");
}

//!\brief deklarierte Tag-Groesse liegt hinter dem Dateiende
TEST_F(Mp3FunctionsTest, IdentMpegIgnoresTagSizeBeyondEof)
{
    expectId3v2Tag(Id3v2Case{3, 0x00, {0x7F, 0x7F, 0x7F, 0x7F}, 636, 10}, "Tag-Groesse hinter dem Dateiende");
}

// --------------------------------------------------------------------------------------
// IdentMPEG(): ID3v1-Tag am Dateiende
// --------------------------------------------------------------------------------------

//!\brief ID3v1-Tags am Dateiende bleiben ausserhalb der Audiodaten
TEST_F(Mp3FunctionsTest, IdentMpegWithoutId3v1Tag)
{
    expectId3v1Tags(0, "kein ID3v1-Tag");
}

TEST_F(Mp3FunctionsTest, IdentMpegStripsOneId3v1Tag)
{
    expectId3v1Tags(1, "ein ID3v1-Tag");
}

TEST_F(Mp3FunctionsTest, IdentMpegStripsTwoId3v1Tags)
{
    // zwei Tags am Dateiende werden beide erkannt
    expectId3v1Tags(2, "zwei ID3v1-Tags");
}

TEST_F(Mp3FunctionsTest, IdentMpegIgnoresThirdId3v1Tag)
{
    // hochstens zwei Tags am Dateiende werden ignoriert, das dritte zaehlt zu den Audiodaten
    expectId3v1Tags(3, "drei ID3v1-Tags");
}

// --------------------------------------------------------------------------------------
// IdentMPEG(): Framesize in Abhaengigkeit von Bitrate, Frequenz und Padding
// --------------------------------------------------------------------------------------

//!\brief Framesize in Abhaengigkeit von Bitrate, Frequenz und Padding-Bit
TEST_F(Mp3FunctionsTest, IdentMpegFrameSizeOfMpeg1Layer3At192kbps)
{
    expectFrameSize(FrameSizeCase{FrameParams{1, 3, 192, 44100, 0}, 626}, "MPEG-1 Layer III, 192 kbit/s, ohne Padding");
}

TEST_F(Mp3FunctionsTest, IdentMpegFrameSizeOfMpeg1Layer3At192kbpsPadded)
{
    expectFrameSize(FrameSizeCase{FrameParams{1, 3, 192, 44100, 1}, 627}, "Padding-Bit erhoeht die Framegroesse um 1");
}

TEST_F(Mp3FunctionsTest, IdentMpegFrameSizeOfMpeg1Layer3At128kbps)
{
    expectFrameSize(FrameSizeCase{FrameParams{1, 3, 128, 44100, 0}, 417}, "MPEG-1 Layer III, 128 kbit/s, ohne Padding");
}

TEST_F(Mp3FunctionsTest, IdentMpegFrameSizeOfMpeg1Layer3At128kbpsPadded)
{
    expectFrameSize(FrameSizeCase{FrameParams{1, 3, 128, 44100, 1}, 418}, "MPEG-1 Layer III, 128 kbit/s, mit Padding");
}

TEST_F(Mp3FunctionsTest, IdentMpegFrameSizeOfMpeg2Layer3At32kbps)
{
    // 62'500 Bytes/s / (24000 Samples/s? nein: 8'000 Samples/s) -> Standardwert 208 Byte
    expectFrameSize(FrameSizeCase{FrameParams{2, 3, 32, 22050, 0}, 208}, "MPEG-2 Layer III, 32 kbit/s, 22,05 kHz");
}

TEST_F(Mp3FunctionsTest, IdentMpegFrameSizeOfMpeg2Layer3At40kbpsPadded)
{
    expectFrameSize(FrameSizeCase{FrameParams{2, 3, 40, 22050, 1}, 262}, "MPEG-2 Layer III, 40 kbit/s, mit Padding");
}

TEST_F(Mp3FunctionsTest, IdentMpegFrameSizeOfMpeg1Layer2At192kbps)
{
    expectFrameSize(FrameSizeCase{FrameParams{1, 2, 192, 44100, 0}, 626},
                    "Layer II nutzt bei 192 kbit/s dieselbe Framegroesse wie Layer III");
}

TEST_F(Mp3FunctionsTest, IdentMpegFrameSizeOfMpeg2Layer2At128kbpsPadded)
{
    expectFrameSize(FrameSizeCase{FrameParams{2, 2, 128, 22050, 1}, 836}, "MPEG-2 Layer II, 128 kbit/s, mit Padding");
}

// --------------------------------------------------------------------------------------
// IdentMPEG(): ungueltige Header-Felder werden verworfen
// --------------------------------------------------------------------------------------

//!\brief Frame mit ungueltigem Header-Feld: IdentMPEG() findet kein MP3
TEST_F(Mp3FunctionsTest, IdentMpegRejectsBitrateIndexZero)
{
    expectInvalidHeader(0xFF, 0xFB, 0x00, 0xC0, "Bitrate-Index 0");
}

TEST_F(Mp3FunctionsTest, IdentMpegRejectsBitrateIndexFifteen)
{
    expectInvalidHeader(0xFF, 0xFB, 0xF0, 0xC0, "Bitrate-Index 15");
}

TEST_F(Mp3FunctionsTest, IdentMpegRejectsReservedFrequencyIndex)
{
    expectInvalidHeader(0xFF, 0xFB, 0xBC, 0xC0, "Frequenz-Index 3 (reserviert)");
}

TEST_F(Mp3FunctionsTest, IdentMpegRejectsReservedVersionBits)
{
    expectInvalidHeader(0xFF, 0xF9, 0xB0, 0xC0, "reservierte Versionskennung");
}

TEST_F(Mp3FunctionsTest, IdentMpegRejectsLayerBitsZero)
{
    expectInvalidHeader(0xFF, 0xF8, 0xB0, 0xC0, "Layer-Kennung 0 (reserviert)");
}

TEST_F(Mp3FunctionsTest, IdentMpegRejectsIncompleteSyncWord)
{
    expectInvalidHeader(0xFF, 0x1F, 0xB0, 0xC0, "unvollstaendiges Sync-Wort");
}

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

//!\brief Bitratenwechsel fuehrt zu VBR-Korrektur, konstante Bitrate nicht
TEST_F(Mp3FunctionsTest, CheckVbrCorrectsMpegAlternating)
{
    expectVbrState(VbrCase{1, 128, 192, 44100, 6, true}, "MPEG-1, 1152 Samples/Frame");
}

TEST_F(Mp3FunctionsTest, CheckVbrCorrectsSampleCount)
{
    expectVbrState(VbrCase{2, 32, 40, 22050, 6, true}, "MPEG-2, 576 Samples/Frame");
}

TEST_F(Mp3FunctionsTest, CheckVbrCorrectsMpegTwoFive)
{
    expectVbrState(VbrCase{3, 8, 16, 8000, 6, true}, "MPEG-2.5, 576 Samples/Frame");
}

TEST_F(Mp3FunctionsTest, CheckVbrKeepsConstantBitrate)
{
    expectVbrState(VbrCase{1, 192, 192, 44100, 6, false}, "konstante Bitrate");
}

TEST_F(Mp3FunctionsTest, CheckVbrStopsSearchAfterFrameCount)
{
    expectVbrState(VbrCase{2, 40, 40, 22050, 60, false}, "lange CBR-Datei (Suchabbruch)");
}

// --------------------------------------------------------------------------------------
// GetMP3Frame()
// --------------------------------------------------------------------------------------

//!\brief GetMP3Frame() liest saemtliche Frames der Audiodaten
TEST_F(Mp3FunctionsTest, GetFrameIteratesAllFramesOfCbrFile)
{
    expectAllFramesOf("testdata/audio/test_192cbr.mp3");
}

TEST_F(Mp3FunctionsTest, GetFrameIteratesAllFramesOfTaggedCbrFile)
{
    expectAllFramesOf("testdata/audio/test_192cbr_tagged.mp3");
}

TEST_F(Mp3FunctionsTest, GetFrameIteratesAllFramesOfTaggedFileWithCover)
{
    expectAllFramesOf("testdata/audio/test_192cbr_taggedWithCover.mp3");
}

TEST_F(Mp3FunctionsTest, GetFrameIteratesAllFramesOfVbrFile)
{
    expectAllFramesOf("testdata/audio/test_192vbr.mp3");
}

TEST_F(Mp3FunctionsTest, GetFrameIteratesAllFramesOfHighBitrateCbrFile)
{
    expectAllFramesOf("testdata/audio/test_320cbr.mp3");
}

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
