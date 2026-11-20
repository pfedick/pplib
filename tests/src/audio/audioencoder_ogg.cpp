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
#include <config_pplib.h>
#include <pplib/types/string.h>
#include <pplib/types/bytearray.h>
#include <pplib/core/file.h>
#include <pplib/core/memfile.h>
#include <pplib/exceptions.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/id3tag.h>
#include <pplib/audio/decoder.h>
#include <pplib/audio/encoder.h>
#include <pplib/audio/sample_formats.h>

#include "pplib-tests.h"

namespace pplib
{

class AudioEncoderOggTest : public ::testing::Test
{
protected:
    AudioEncoderOggTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            throw std::exception();
        }
    }
};

TEST_F(AudioEncoderOggTest, ParameterValidationThrowsIllegalArgument)
{
    AudioEncoder_Ogg enc;
    EXPECT_THROW(enc.setVBR(-0.2f), IllegalArgumentException);
    EXPECT_THROW(enc.setVBR(1.1f), IllegalArgumentException);
    EXPECT_NO_THROW(enc.setVBR(-0.1f));
    EXPECT_NO_THROW(enc.setVBR(1.0f));

    EXPECT_THROW(enc.setABR(0), IllegalArgumentException);
    EXPECT_THROW(enc.setABR(-64), IllegalArgumentException);
    EXPECT_THROW(enc.setABR(192, 0, -1), IllegalArgumentException);
    EXPECT_THROW(enc.setABR(192, -1, 0), IllegalArgumentException);
    EXPECT_THROW(enc.setABR(192, 256, 128), IllegalArgumentException);
    EXPECT_THROW(enc.setABR(100, 128, 256), IllegalArgumentException);
    EXPECT_THROW(enc.setABR(300, 128, 256), IllegalArgumentException);
    EXPECT_NO_THROW(enc.setABR(192, 128, 256));

    EXPECT_THROW(enc.setCBR(0), IllegalArgumentException);
    EXPECT_THROW(enc.setCBR(-128), IllegalArgumentException);
    EXPECT_NO_THROW(enc.setCBR(192));
}

TEST_F(AudioEncoderOggTest, StateErrorsThrowExpectedExceptions)
{
    AudioEncoder_Ogg enc;
    AudioDecoder_Wave dec;
    File unopen;

    EXPECT_THROW(enc.encode(dec), EncoderNotStartedException);
    EXPECT_THROW(enc.finish(), EncoderNotStartedException);
    EXPECT_THROW(enc.startEncode(unopen), FileNotOpenException);

    MemFile out;
    out.open(nullptr, 0, true);
    ASSERT_NO_THROW(enc.startEncode(out));
    EXPECT_THROW(enc.startEncode(out), EncoderAlreadyStartedException);
    EXPECT_THROW(enc.setVBR(0.5f), EncoderAlreadyStartedException);
    EXPECT_THROW(enc.setABR(192), EncoderAlreadyStartedException);
    EXPECT_THROW(enc.setCBR(192), EncoderAlreadyStartedException);
    EXPECT_NO_THROW(enc.finish());
}

TEST_F(AudioEncoderOggTest, EncodeVbrStereoRoundtripAndProgress)
{
    File inFile;
    inFile.open("testdata/audio/test_44kHz.wav");
    AudioDecoder_Wave dec;
    dec.open(inFile);
    const uint64_t expectedSamples = dec.getAudioInfo().Samples;

    File outFile;
    outFile.open("tmp/enc_vbr.ogg", File::FileMode::WRITE);

    int lastProgress = -1;
    int callbacks = 0;
    auto progressCb = [](int progress, void* priv) {
        auto* pair = static_cast<std::pair<int*, int*>*>(priv);
        *(pair->first) = progress;
        (*(pair->second))++;
    };
    std::pair<int*, int*> priv{&lastProgress, &callbacks};

    AudioEncoder_Ogg enc;
    enc.setVBR(0.5f);
    enc.setProgressFunction(progressCb, &priv);
    enc.startEncode(outFile);
    enc.encode(dec);
    enc.finish();
    outFile.close();

    File verifyFile;
    verifyFile.open("tmp/enc_vbr.ogg");
    AudioInfo info;
    ASSERT_TRUE(IdentAudioFile(verifyFile, info));
    EXPECT_EQ(AudioInfo::AudioFormat::OGG, info.Format);
    EXPECT_EQ((uint8_t)2, info.Channels);
    EXPECT_EQ((uint32_t)44100, info.Frequency);
    EXPECT_GT(info.Bitrate, (uint16_t)0);
    EXPECT_EQ(expectedSamples, info.Samples);
    EXPECT_FALSE(info.HaveVorbisComment);
    EXPECT_GT(callbacks, 0);
    EXPECT_EQ(100, lastProgress);

    AudioDecoder_Ogg oggDec;
    oggDec.open(verifyFile);
    EXPECT_EQ(expectedSamples, oggDec.getAudioInfo().Samples);
    std::vector<STEREOSAMPLE16> pcm(4096);
    uint64_t decoded = 0;
    while (true) {
        size_t got = oggDec.getSamples(4096, pcm.data());
        if (got == 0) break;
        decoded += got;
    }
    EXPECT_EQ(expectedSamples, decoded);
}

TEST_F(AudioEncoderOggTest, EncodeMonoAbrAndCbrModes)
{
    {
        File inFile;
        inFile.open("testdata/audio/test_44kHz_16bit_mono.aiff");
        AudioDecoder_Aiff dec;
        dec.open(inFile);

        File outFile;
        outFile.open("tmp/enc_mono_abr.ogg", File::FileMode::WRITE);

        AudioEncoder_Ogg enc;
        enc.setABR(160, 128, 192);
        enc.startEncode(outFile);
        enc.encode(dec);
        enc.finish();
        outFile.close();

        File verify;
        verify.open("tmp/enc_mono_abr.ogg");
        AudioInfo info;
        ASSERT_TRUE(IdentAudioFile(verify, info));
        EXPECT_EQ(AudioInfo::AudioFormat::OGG, info.Format);
        EXPECT_EQ((uint8_t)1, info.Channels);
        EXPECT_EQ(AudioInfo::ChannelMode::MONO, info.Mode);
        EXPECT_EQ(dec.getAudioInfo().Samples, info.Samples);
    }

    {
        File inFile;
        inFile.open("testdata/audio/test_44kHz.wav");
        AudioDecoder_Wave dec;
        dec.open(inFile);

        File outFile;
        outFile.open("tmp/enc_cbr.ogg", File::FileMode::WRITE);

        AudioEncoder_Ogg enc;
        enc.setCBR(192);
        enc.startEncode(outFile);
        enc.encode(dec);
        enc.finish();
        outFile.close();

        File verify;
        verify.open("tmp/enc_cbr.ogg");
        AudioInfo info;
        ASSERT_TRUE(IdentAudioFile(verify, info));
        EXPECT_EQ(AudioInfo::AudioFormat::OGG, info.Format);
        EXPECT_FALSE(info.IsVBR);
        EXPECT_EQ((uint16_t)192, info.Bitrate);
    }
}

TEST_F(AudioEncoderOggTest, FormatMismatchAndStopEncoderThrowExpectedExceptions)
{
    File wavFile;
    wavFile.open("testdata/audio/test_44kHz.wav");
    AudioDecoder_Wave wavDec;
    wavDec.open(wavFile);

    File monoFile;
    monoFile.open("testdata/audio/test_44kHz_16bit_mono.aiff");
    AudioDecoder_Aiff monoDec;
    monoDec.open(monoFile);

    MemFile out;
    out.open(nullptr, 0, true);
    AudioEncoder_Ogg enc;
    enc.startEncode(out);
    enc.encode(wavDec);
    EXPECT_THROW(enc.encode(monoDec), EncoderAudioFormatMismatchException);
    enc.finish();

    wavFile.seek(0);
    wavDec.open(wavFile);
    AudioEncoder_Ogg abortEnc;
    abortEnc.startEncode(out);
    abortEnc.stopEncoder();
    EXPECT_THROW(abortEnc.encode(wavDec), EncoderAbortedException);
}

TEST_F(AudioEncoderOggTest, SetVorbisCommentsStateErrorsAndRoundtripWithCover)
{
    ID3Tag srcTag;
    srcTag.setArtist("Ogg Encoder Artist");
    srcTag.setTitle("Ogg Encoder Title");
    srcTag.setAlbum("Ogg Encoder Album");
    srcTag.setYear("2026");
    srcTag.setTrack("2/8");
    srcTag.setGenre("Trance");
    srcTag.setComment("Encoded by pplib");
    srcTag.setUserDefinedText("CUSTOM_TAG", "CustomValue");

    ByteArray cover;
    uint8_t* p = static_cast<uint8_t*>(cover.malloc(2048));
    for (size_t i = 0; i < 2048; ++i)
        p[i] = (uint8_t)((i * 13) & 0xFF);
    srcTag.setPicture(3, cover, "image/jpeg");

    {
        AudioEncoder_Ogg enc;
        EXPECT_THROW(enc.setVorbisComments(srcTag), EncoderNotStartedException);
    }

    File wavFile;
    wavFile.open("testdata/audio/test_44kHz.wav");
    AudioDecoder_Wave dec;
    dec.open(wavFile);

    File outFile;
    outFile.open("tmp/enc_tagged.ogg", File::FileMode::WRITE);

    AudioEncoder_Ogg enc;
    enc.setVBR(0.4f);
    enc.startEncode(outFile);
    ASSERT_NO_THROW(enc.setVorbisComments(srcTag));
    enc.encode(dec);
    EXPECT_THROW(enc.setVorbisComments(srcTag), EncoderAlreadyStartedException);
    enc.finish();
    outFile.close();

    File verifyFile;
    verifyFile.open("tmp/enc_tagged.ogg");
    AudioInfo info;
    ASSERT_TRUE(IdentAudioFile(verifyFile, info));
    EXPECT_EQ(AudioInfo::AudioFormat::OGG, info.Format);
    EXPECT_TRUE(info.HaveVorbisComment);
    EXPECT_EQ(dec.getAudioInfo().Samples, info.Samples);
    verifyFile.close();

    ID3Tag loaded;
    ASSERT_NO_THROW(loaded.load("tmp/enc_tagged.ogg"));
    EXPECT_EQ(String("Ogg Encoder Artist"), loaded.getArtist());
    EXPECT_EQ(String("Ogg Encoder Title"), loaded.getTitle());
    EXPECT_EQ(String("Ogg Encoder Album"), loaded.getAlbum());
    EXPECT_EQ(String("2026"), loaded.getYear());
    EXPECT_EQ(String("2/8"), loaded.getTrack());
    EXPECT_EQ(String("Trance"), loaded.getGenre());
    EXPECT_EQ(String("Encoded by pplib"), loaded.getComment());
    EXPECT_EQ(String("CustomValue"), loaded.getUserDefinedText("CUSTOM_TAG"));

    std::vector<ID3Tag::FlacPicture> pics;
    loaded.exportVorbisPictures(pics);
    ASSERT_EQ((size_t)1, pics.size());
    EXPECT_EQ((uint32_t)3, pics[0].type);
    EXPECT_EQ(String("image/jpeg"), pics[0].mime);
    ASSERT_EQ(cover.size(), pics[0].data.size());
    EXPECT_EQ(0, memcmp(cover.ptr(), pics[0].data.ptr(), cover.size()));
}

} // namespace pplib
