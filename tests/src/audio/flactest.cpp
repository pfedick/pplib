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

#include <pplib/types/string.h>
#include <pplib/core/functions.h>
#include <pplib/core/stringfunctions.h>
#include <pplib/core/dir.h>
#include <pplib/core/file.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/id3tag.h>
#include <pplib/audio/decoder.h>
#include <pplib/audio/encoder.h>

#include "pplib-tests.h"

namespace
{

class FlacTest : public ::testing::Test
{
protected:
    FlacTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~FlacTest()
    {
    }
};

/*
TEST_F(FlacTest, AudioInfoOnOriginalBeatportFile)
{
    pplib::File file("testdata/hidden/Factor B - A Gift To The Earth (Sneijder Extended Remix)-original.flac");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    info.print();
}
*/

static void copyTestFile(const pplib::String& src, const pplib::String& dst)
{
    pplib::Dir::mkDir("tmp");
    pplib::File in(src, pplib::File::FileMode::READ);
    pplib::File out;
    out.open(dst, pplib::File::FileMode::WRITE);
    out.copyFrom(in, 0, in.size(), 0);
    out.close();
    in.close();
}

TEST_F(FlacTest, LoadBasicFlacAndId3v2PrefixedFlac)
{
    pplib::ID3Tag tag;
    ASSERT_TRUE(tag.tryLoad("testdata/audio/test_44kHz_16bit.flac"));

    pplib::ID3Tag tagId3;
    ASSERT_TRUE(tagId3.tryLoad("testdata/audio/test_44kHz_16bit_id3v2.flac"));

    // Invalid / truncated metadata header must throw UnsupportedAudioFormatException
    pplib::Dir::mkDir("tmp");
    pplib::String badPath = "tmp/bad_flac_meta.flac";
    {
        pplib::File f;
        f.open(badPath, pplib::File::FileMode::WRITE);
        const char badHeader[8] = {'f', 'L', 'a', 'C', 0x00, 0x00, 0x00, 0x22}; // claims 34 bytes STREAMINFO, EOF immediately
        f.write(badHeader, sizeof(badHeader), 0);
        f.close();
    }
    pplib::ID3Tag tagBad;
    EXPECT_THROW(tagBad.load(badPath), pplib::UnsupportedAudioFormatException);
    pplib::File::remove(badPath);
}

TEST_F(FlacTest, RoundtripStandardTagsPopularimeterAndCustomText)
{
    pplib::String tmpPath = "tmp/flac_roundtrip_tags.flac";
    copyTestFile("testdata/audio/test_44kHz_16bit.flac", tmpPath);

    pplib::ByteArray synthBin(32);
    unsigned char* p = (unsigned char*)synthBin.ptr();
    for (size_t i = 0; i < 32; ++i)
        p[i] = (unsigned char)(i * 7 + 3);
    pplib::String encodedCustom = pplib::ToBase91(synthBin);

    {
        pplib::ID3Tag tag;
        tag.load(tmpPath);
        tag.setArtist("Factor B & Friends");
        tag.setTitle("A Gift To The Earth (Ümlaut Mix)");
        tag.setAlbum("Trance Classics");
        tag.setGenre("Trance");
        tag.setYear("2026");
        tag.setTrack("4/9");
        tag.setComment("Test Comment");
        tag.setRemixer("Sneijder");
        tag.setLabel("Theatre of the Mind");
        tag.setBPM("138");
        tag.setKey("A");
        tag.setEnergyLevel("8");
        tag.setPopularimeter("traktor@native-instruments.de", 204);
        tag.setUserDefinedText("TRAKTOR4", encodedCustom);
        tag.setUserDefinedText("ISRC", "NLD682602352");
        tag.save();
    }

    {
        pplib::ID3Tag tag;
        tag.load(tmpPath);
        EXPECT_EQ(pplib::String("Factor B & Friends"), tag.getArtist());
        EXPECT_EQ(pplib::String("A Gift To The Earth (Ümlaut Mix)"), tag.getTitle());
        EXPECT_EQ(pplib::String("Trance Classics"), tag.getAlbum());
        EXPECT_EQ(pplib::String("Trance"), tag.getGenre());
        EXPECT_EQ(pplib::String("2026"), tag.getYear());
        EXPECT_EQ(pplib::String("4/9"), tag.getTrack());
        EXPECT_EQ(pplib::String("Test Comment"), tag.getComment());
        EXPECT_EQ(pplib::String("Sneijder"), tag.getRemixer());
        EXPECT_EQ(pplib::String("Theatre of the Mind"), tag.getLabel());
        EXPECT_EQ(pplib::String("138"), tag.getBPM());
        EXPECT_EQ(pplib::String("A"), tag.getKey());
        EXPECT_EQ(pplib::String("8"), tag.getEnergyLevel());
        EXPECT_EQ(204, tag.getPopularimeter());
        EXPECT_EQ(encodedCustom, tag.getUserDefinedText("TRAKTOR4"));
        EXPECT_EQ(pplib::String("NLD682602352"), tag.getUserDefinedText("ISRC"));

        pplib::ByteArray decoded = pplib::FromBase91(tag.getUserDefinedText("TRAKTOR4"));
        ASSERT_EQ(synthBin.size(), decoded.size());
        EXPECT_EQ(0, memcmp(synthBin.ptr(), decoded.ptr(), synthBin.size()));

        // Known ID3v1 genre must be written without "(13)" prefix in FLAC
        tag.setGenre("Pop");
        tag.save();
    }

    {
        pplib::ID3Tag tag;
        tag.load(tmpPath);
        const pplib::ID3Frame* tcon = tag.findFrame("TCON");
        ASSERT_NE(nullptr, tcon);
        pplib::String rawGenre;
        pplib::ID3Tag::decode(tcon, 1, pplib::Peek8(tcon->dataPtr()), rawGenre, "UTF-8");
        EXPECT_EQ(pplib::String("Pop"), rawGenre);
    }

    pplib::File::remove(tmpPath);
}

TEST_F(FlacTest, RoundtripPictures)
{
    pplib::String tmpPath = "tmp/flac_roundtrip_pic.flac";
    copyTestFile("testdata/audio/test_44kHz_16bit.flac", tmpPath);

    pplib::ByteArray frontImg(128);
    memset(frontImg.ptr(), 0xAA, 128);
    pplib::ByteArray backImg(64);
    memset(backImg.ptr(), 0x55, 64);

    {
        pplib::ID3Tag tag;
        tag.load(tmpPath);
        tag.setPicture(pplib::ID3Tag::PIC_COVER_FRONT, frontImg, "image/jpeg");
        tag.setPicture(pplib::ID3Tag::PIC_COVER_BACK, backImg, "image/png");
        tag.save();
    }

    {
        pplib::ID3Tag tag;
        tag.load(tmpPath);
        EXPECT_TRUE(tag.hasPicture(pplib::ID3Tag::PIC_COVER_FRONT));
        EXPECT_TRUE(tag.hasPicture(pplib::ID3Tag::PIC_COVER_BACK));

        pplib::ByteArray outFront, outBack;
        ASSERT_TRUE(tag.getPicture(pplib::ID3Tag::PIC_COVER_FRONT, outFront));
        ASSERT_TRUE(tag.getPicture(pplib::ID3Tag::PIC_COVER_BACK, outBack));
        ASSERT_EQ(frontImg.size(), outFront.size());
        ASSERT_EQ(backImg.size(), outBack.size());
        EXPECT_EQ(0, memcmp(frontImg.ptr(), outFront.ptr(), frontImg.size()));
        EXPECT_EQ(0, memcmp(backImg.ptr(), outBack.ptr(), backImg.size()));

        tag.removePicture(pplib::ID3Tag::PIC_COVER_BACK);
        tag.save();
    }

    {
        pplib::ID3Tag tag;
        tag.load(tmpPath);
        EXPECT_TRUE(tag.hasPicture(pplib::ID3Tag::PIC_COVER_FRONT));
        EXPECT_FALSE(tag.hasPicture(pplib::ID3Tag::PIC_COVER_BACK));
    }

    pplib::File::remove(tmpPath);
}

TEST_F(FlacTest, InPlaceSaveOverflowAndSeektablePreserved)
{
    pplib::String tmpPath = "tmp/flac_inplace_seektable.flac";
    copyTestFile("testdata/audio/test_44kHz_16bit_seektable.flac", tmpPath);

    pplib::AudioInfo infoBefore;
    {
        pplib::File f(tmpPath);
        ASSERT_TRUE(pplib::IdentAudioFile(f, infoBefore));
    }

    // 1. Initial save creates PADDING (1024 bytes)
    {
        pplib::ID3Tag tag;
        tag.load(tmpPath);
        tag.setPaddingSize(1024);
        tag.setArtist("Artist 1");
        tag.save();
    }
    uint64_t sizeAfterFirstSave = pplib::File(tmpPath).size();

    // 2. Small update must fit in-place without changing file size
    {
        pplib::ID3Tag tag;
        tag.load(tmpPath);
        tag.setArtist("Artist 2");
        tag.setTitle("Short Title");
        tag.save();
    }
    EXPECT_EQ(sizeAfterFirstSave, pplib::File(tmpPath).size());

    // 3. Large update (>1024 bytes) overflows PADDING and rewrites via temp file
    {
        pplib::ID3Tag tag;
        tag.load(tmpPath);
        pplib::ByteArray bigPic(4096);
        memset(bigPic.ptr(), 0x7E, 4096);
        tag.setPicture(pplib::ID3Tag::PIC_COVER_FRONT, bigPic, "image/jpeg");
        tag.save();
    }
    EXPECT_GT(pplib::File(tmpPath).size(), sizeAfterFirstSave);

    // Verify SEEKTABLE is still present and AudioInfo is valid
    bool hasSeektable = false;
    {
        pplib::File f(tmpPath);
        pplib::AudioInfo infoAfter;
        ASSERT_TRUE(pplib::IdentAudioFile(f, infoAfter));
        EXPECT_EQ(infoBefore.Frequency, infoAfter.Frequency);
        EXPECT_EQ(infoBefore.Channels, infoAfter.Channels);
        EXPECT_EQ(infoBefore.Samples, infoAfter.Samples);

        uint64_t p = 4;
        while (p + 4 <= f.size()) {
            const char* hdr = f.map(p, 4);
            bool last = (pplib::Peek8(hdr) & 0x80) != 0;
            uint8_t type = pplib::Peek8(hdr) & 0x7F;
            uint32_t sz =
                ((uint32_t)pplib::Peek8(hdr + 1) << 16) | ((uint32_t)pplib::Peek8(hdr + 2) << 8) | (uint32_t)pplib::Peek8(hdr + 3);
            if (type == 3) hasSeektable = true;
            p += 4 + sz;
            if (last) break;
        }
    }
    EXPECT_TRUE(hasSeektable);

    pplib::File::remove(tmpPath);
}

TEST_F(FlacTest, SaveStripsLeadingId3v2Tag)
{
    pplib::String tmpPath = "tmp/flac_strip_id3v2.flac";
    copyTestFile("testdata/audio/test_44kHz_16bit_id3v2.flac", tmpPath);

    {
        pplib::File f(tmpPath);
        pplib::AudioInfo infoBefore;
        ASSERT_TRUE(pplib::IdentAudioFile(f, infoBefore));
        ASSERT_TRUE(infoBefore.HaveID3v2Tag);
    }

    {
        pplib::ID3Tag tag;
        tag.load(tmpPath);
        tag.setArtist("Clean FLAC Artist");
        tag.save();
    }

    {
        pplib::File f(tmpPath);
        const char* magic = f.map(0, 4);
        EXPECT_EQ(0, memcmp(magic, "fLaC", 4));

        pplib::AudioInfo infoAfter;
        ASSERT_TRUE(pplib::IdentAudioFile(f, infoAfter));
        EXPECT_FALSE(infoAfter.HaveID3v2Tag);
    }

    {
        pplib::ID3Tag tag;
        tag.load(tmpPath);
        EXPECT_EQ(pplib::String("Clean FLAC Artist"), tag.getArtist());
    }

    pplib::File::remove(tmpPath);
}

TEST_F(FlacTest, AudioEncoderFlacWithVorbisCommentsAndPicture)
{
    pplib::Dir::mkDir("tmp");
    pplib::String tmpPath = "tmp/flac_encoded_with_tags.flac";

    pplib::ByteArray frontImg(24);
    for (size_t i = 0; i < frontImg.size(); ++i)
        ((unsigned char*)frontImg.ptr())[i] = (unsigned char)(i + 10);
    pplib::ByteArray backImg(16);
    for (size_t i = 0; i < backImg.size(); ++i)
        ((unsigned char*)backImg.ptr())[i] = (unsigned char)(200 - i);

    pplib::ID3Tag srcTag;
    srcTag.setArtist("Factor B & Friends");
    srcTag.setTitle("Encoded Title (Ümlaut)");
    srcTag.setAlbum("Trance Album");
    srcTag.setTrack("3/10");
    srcTag.setBPM("138");
    srcTag.setKey("8A");
    srcTag.setRemixer("Club Mix");
    srcTag.setUserDefinedText("EnergyLevel", "8");
    srcTag.setPopularimeter("dj@example.com", 255);
    srcTag.setPicture(pplib::ID3Tag::PIC_COVER_FRONT, frontImg, "image/jpeg");
    srcTag.setPicture(pplib::ID3Tag::PIC_COVER_BACK, backImg, "image/png");

    pplib::File srcFile("testdata/audio/test_44kHz_16bit.flac", pplib::File::FileMode::READ);
    pplib::AudioDecoder_Flac decoder;
    decoder.open(srcFile);
    uint64_t singleSamples = decoder.getAudioInfo().Samples;

    {
        pplib::File outFile;
        outFile.open(tmpPath, pplib::File::FileMode::WRITE);
        pplib::AudioEncoder_Flac encoder;
        encoder.startEncode(outFile);
        encoder.setVorbisComments(srcTag);
        encoder.encode(decoder);
        encoder.encode(decoder);
        encoder.finish();
        outFile.close();
    }

    {
        pplib::File checkFile(tmpPath, pplib::File::FileMode::READ);
        pplib::AudioInfo info;
        ASSERT_TRUE(pplib::IdentAudioFile(checkFile, info));
        EXPECT_EQ(pplib::AudioInfo::AudioFormat::FLAC, info.Format);
        EXPECT_EQ(44100u, info.Frequency);
        EXPECT_EQ(2, info.Channels);
        EXPECT_EQ(singleSamples * 2, info.Samples);
    }

    {
        pplib::ID3Tag readTag;
        readTag.load(tmpPath);
        EXPECT_EQ(pplib::String("Factor B & Friends"), readTag.getArtist());
        EXPECT_EQ(pplib::String("Encoded Title (Ümlaut)"), readTag.getTitle());
        EXPECT_EQ(pplib::String("Trance Album"), readTag.getAlbum());
        EXPECT_EQ(pplib::String("3/10"), readTag.getTrack());
        EXPECT_EQ(pplib::String("138"), readTag.getBPM());
        EXPECT_EQ(pplib::String("8A"), readTag.getKey());
        EXPECT_EQ(pplib::String("Club Mix"), readTag.getRemixer());
        EXPECT_EQ(pplib::String("8"), readTag.getEnergyLevel());
        EXPECT_EQ((unsigned char)255, readTag.getPopularimeter());

        pplib::ByteArray outFront, outBack;
        ASSERT_TRUE(readTag.getPicture(pplib::ID3Tag::PIC_COVER_FRONT, outFront));
        ASSERT_TRUE(readTag.getPicture(pplib::ID3Tag::PIC_COVER_BACK, outBack));
        ASSERT_EQ(frontImg.size(), outFront.size());
        ASSERT_EQ(backImg.size(), outBack.size());
        EXPECT_EQ(0, memcmp(frontImg.ptr(), outFront.ptr(), frontImg.size()));
        EXPECT_EQ(0, memcmp(backImg.ptr(), outBack.ptr(), backImg.size()));
    }

    pplib::File::remove(tmpPath);
}

TEST_F(FlacTest, AudioEncoderFlacSetVorbisCommentsLifecycleErrors)
{
    pplib::Dir::mkDir("tmp");
    pplib::String tmpPath = "tmp/flac_encoder_lifecycle.flac";

    pplib::ID3Tag tag;
    tag.setArtist("Test Artist");

    pplib::AudioEncoder_Flac encoder;
    EXPECT_THROW(encoder.setVorbisComments(tag), pplib::EncoderNotStartedException);

    pplib::File srcFile("testdata/audio/test_44kHz_16bit.flac", pplib::File::FileMode::READ);
    pplib::AudioDecoder_Flac decoder;
    decoder.open(srcFile);

    pplib::File outFile;
    outFile.open(tmpPath, pplib::File::FileMode::WRITE);
    encoder.startEncode(outFile);
    encoder.encode(decoder);
    EXPECT_THROW(encoder.setVorbisComments(tag), pplib::EncoderAlreadyStartedException);
    encoder.finish();
    outFile.close();

    pplib::File::remove(tmpPath);
}

} // namespace