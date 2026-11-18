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

#include <pplib/audio/audioinfo.h>
#include <pplib/audio/decoder.h>
#include <pplib/audio/encoder.h>
#include <pplib/audio/id3tag.h>
#include <pplib/audio/sample_formats.h>
#include <pplib/core/dir.h>
#include <pplib/core/file.h>

namespace
{

class AudioEncoderAiffTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        pplib::Dir::mkDir("tmp");
    }
};

TEST_F(AudioEncoderAiffTest, StereoRoundtripWithId3Tag)
{
    pplib::String tmpPath = "tmp/aiff_encoded_stereo_tagged.aiff";

    pplib::ByteArray coverImg(21); // ungerade Laenge prueft zugleich das Chunk-Padding
    for (size_t i = 0; i < coverImg.size(); ++i) ((unsigned char*)coverImg.ptr())[i] = (unsigned char)(i + 5);

    pplib::ID3Tag srcTag;
    srcTag.setArtist("Factor B");
    srcTag.setTitle("AIFF Encoded Track");
    srcTag.setAlbum("AIFF Album");
    srcTag.setPicture(pplib::ID3Tag::PIC_COVER_FRONT, coverImg, "image/jpeg");

    pplib::File srcFile("testdata/audio/test_44kHz.aiff", pplib::File::FileMode::READ);
    pplib::AudioDecoder_Aiff srcDecoder;
    srcDecoder.open(srcFile);
    uint64_t singleSamples = srcDecoder.getAudioInfo().Samples;

    pplib::STEREOSAMPLE16 expected[16] = {};
    ASSERT_EQ((size_t)16, srcDecoder.getSamples(16, expected));

    {
        pplib::File outFile;
        outFile.open(tmpPath, pplib::File::FileMode::WRITE);
        pplib::AudioEncoder_Aiff encoder;
        encoder.startEncode(outFile);
        encoder.writeID3v2Tag(srcTag);
        encoder.encode(srcDecoder);
        encoder.encode(srcDecoder);
        encoder.finish();
        outFile.close();
    }

    {
        pplib::File checkFile(tmpPath, pplib::File::FileMode::READ);
        pplib::AudioInfo info;
        ASSERT_TRUE(pplib::IdentAudioFile(checkFile, info));
        EXPECT_EQ(pplib::AudioInfo::AudioFormat::AIFF, info.Format);
        EXPECT_EQ(44100u, info.Frequency);
        EXPECT_EQ(2, info.Channels);
        EXPECT_EQ(16, info.BitsPerSample);
        EXPECT_EQ(singleSamples * 2, info.Samples);

        pplib::AudioDecoder_Aiff outDecoder;
        outDecoder.open(checkFile);
        pplib::STEREOSAMPLE16 actual[16] = {};
        ASSERT_EQ((size_t)16, outDecoder.getSamples(16, actual));
        for (size_t i = 0; i < 16; ++i) {
            EXPECT_EQ(expected[i].left, actual[i].left);
            EXPECT_EQ(expected[i].right, actual[i].right);
        }
    }

    {
        pplib::ID3Tag readTag;
        readTag.load(tmpPath);
        EXPECT_EQ(pplib::String("Factor B"), readTag.getArtist());
        EXPECT_EQ(pplib::String("AIFF Encoded Track"), readTag.getTitle());
        EXPECT_EQ(pplib::String("AIFF Album"), readTag.getAlbum());

        pplib::ByteArray outCover;
        ASSERT_TRUE(readTag.getPicture(pplib::ID3Tag::PIC_COVER_FRONT, outCover));
        ASSERT_EQ(coverImg.size(), outCover.size());
        EXPECT_EQ(0, memcmp(coverImg.ptr(), outCover.ptr(), coverImg.size()));
    }

    pplib::File::remove(tmpPath);
}

TEST_F(AudioEncoderAiffTest, MonoRoundtrip)
{
    pplib::String tmpPath = "tmp/aiff_encoded_mono.aiff";

    pplib::File srcFile("testdata/audio/test_44kHz_16bit_mono.aiff", pplib::File::FileMode::READ);
    pplib::AudioDecoder_Aiff srcDecoder;
    srcDecoder.open(srcFile);
    uint64_t expectedSamples = srcDecoder.getAudioInfo().Samples;

    pplib::STEREOSAMPLE16 expected[16] = {};
    ASSERT_EQ((size_t)16, srcDecoder.getSamples(16, expected));

    {
        pplib::File outFile;
        outFile.open(tmpPath, pplib::File::FileMode::WRITE);
        pplib::AudioEncoder_Aiff encoder;
        encoder.startEncode(outFile);
        encoder.encode(srcDecoder);
        encoder.finish();
        outFile.close();
    }

    {
        pplib::File checkFile(tmpPath, pplib::File::FileMode::READ);
        pplib::AudioInfo info;
        ASSERT_TRUE(pplib::IdentAudioFile(checkFile, info));
        EXPECT_EQ(pplib::AudioInfo::AudioFormat::AIFF, info.Format);
        EXPECT_EQ(44100u, info.Frequency);
        EXPECT_EQ(1, info.Channels);
        EXPECT_EQ(16, info.BitsPerSample);
        EXPECT_EQ(expectedSamples, info.Samples);

        pplib::AudioDecoder_Aiff outDecoder;
        outDecoder.open(checkFile);
        pplib::STEREOSAMPLE16 actual[16] = {};
        ASSERT_EQ((size_t)16, outDecoder.getSamples(16, actual));
        for (size_t i = 0; i < 16; ++i) {
            EXPECT_EQ(expected[i].left, actual[i].left);
            EXPECT_EQ(expected[i].right, actual[i].right);
        }
    }

    pplib::File::remove(tmpPath);
}

TEST_F(AudioEncoderAiffTest, LifecycleAndFormatMismatchErrors)
{
    pplib::String tmpPath = "tmp/aiff_encoder_errors.aiff";

    pplib::ID3Tag tag;
    tag.setArtist("Test Artist");

    pplib::File stereoFile("testdata/audio/test_44kHz.aiff", pplib::File::FileMode::READ);
    pplib::AudioDecoder_Aiff stereoDecoder;
    stereoDecoder.open(stereoFile);

    pplib::File monoFile("testdata/audio/test_44kHz_16bit_mono.aiff", pplib::File::FileMode::READ);
    pplib::AudioDecoder_Aiff monoDecoder;
    monoDecoder.open(monoFile);

    pplib::AudioEncoder_Aiff encoder;
    EXPECT_THROW(encoder.writeID3v2Tag(tag), pplib::EncoderNotStartedException);
    EXPECT_THROW(encoder.encode(stereoDecoder), pplib::EncoderNotStartedException);
    EXPECT_THROW(encoder.finish(), pplib::EncoderNotStartedException);

    pplib::File outFile;
    outFile.open(tmpPath, pplib::File::FileMode::WRITE);
    encoder.startEncode(outFile);
    EXPECT_THROW(encoder.startEncode(outFile), pplib::EncoderAlreadyStartedException);

    encoder.encode(stereoDecoder);
    EXPECT_THROW(encoder.writeID3v2Tag(tag), pplib::EncoderAlreadyStartedException);
    EXPECT_THROW(encoder.encode(monoDecoder), pplib::EncoderAudioFormatMismatchException);
    encoder.finish();
    outFile.close();

    pplib::File::remove(tmpPath);
}

} // namespace
