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
#include <config_pplib.h>
#include <pplib/types/string.h>
#include <pplib/types/bytearray.h>
#include <pplib/core/dir.h>
#include <pplib/core/iconv.h>
#include <pplib/core/functions.h>
#include <pplib/core/memfile.h>
#include <pplib/exceptions.h>
#include <pplib/audio/id3tag.h>
#include <pplib/audio/decoder.h>
#include <pplib/audio/encoder.h>
#include <pplib/audio/sample_formats.h>

#include "pplib-tests.h"

#include <cmath>
#include <vector>

#ifdef HAVE_LAME

namespace
{

class AudioEncoderMp3Test : public ::testing::Test
{
protected:
    AudioEncoderMp3Test()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~AudioEncoderMp3Test()
    {
    }
};

TEST_F(AudioEncoderMp3Test, VersionStrings)
{
    const char* lameVer = pplib::AudioEncoder_MP3::getLameVersion();
    const char* psyVer = pplib::AudioEncoder_MP3::getPSYVersion();
    ASSERT_NE(nullptr, lameVer);
    ASSERT_NE(nullptr, psyVer);
    EXPECT_GT(strlen(lameVer), 0u);
    EXPECT_GT(strlen(psyVer), 0u);
}

TEST_F(AudioEncoderMp3Test, HighLevelCBRWithID3Tags)
{
    pplib::File aiff;
    aiff.open("testdata/audio/test_44kHz_taggedWithCover.aiff");
    pplib::ID3Tag original_tag;
    original_tag.load(aiff);

    pplib::AudioDecoder_Aiff aiff_decoder;
    aiff_decoder.open(aiff);
    const pplib::AudioInfo info = aiff_decoder.getAudioInfo();
    ASSERT_EQ(44100u, info.Frequency);
    ASSERT_EQ(2, info.Channels);
    ASSERT_GT(info.Samples, 0u);

    pplib::MemFile mp3_mem;
    pplib::AudioEncoder_MP3 mp3_encoder;
    mp3_encoder.setCBR(192, 2);
    mp3_encoder.setStereoMode(pplib::AudioInfo::ChannelMode::JOINT_STEREO);
    mp3_encoder.startEncode(mp3_mem);
    mp3_encoder.writeID3v2Tag(original_tag);
    mp3_encoder.encode(aiff_decoder);
    mp3_encoder.finish();
    mp3_encoder.writeID3v1Tag(original_tag);

    mp3_mem.seek(0);
    pplib::AudioInfo mp3_info;
    ASSERT_TRUE(pplib::IdentAudioFile(mp3_mem, mp3_info));
    EXPECT_EQ(pplib::AudioInfo::AudioFormat::MP3, mp3_info.Format);
    EXPECT_EQ(44100u, mp3_info.Frequency);
    EXPECT_EQ(2, mp3_info.Channels);
    EXPECT_EQ(192, mp3_info.Bitrate);
    EXPECT_GT(mp3_info.Samples, 0u);

    mp3_mem.seek(0);
    pplib::ID3Tag decoded_tag;
    decoded_tag.load(mp3_mem);
    EXPECT_EQ(original_tag.getArtist(), decoded_tag.getArtist());
    EXPECT_EQ(original_tag.getTitle(), decoded_tag.getTitle());
    EXPECT_EQ(original_tag.getAlbum(), decoded_tag.getAlbum());
    EXPECT_EQ(original_tag.getYear(), decoded_tag.getYear());
    EXPECT_EQ(original_tag.getTrack(), decoded_tag.getTrack());
    EXPECT_EQ(original_tag.getGenre(), decoded_tag.getGenre());
    EXPECT_EQ(original_tag.getPicture(0), decoded_tag.getPicture(0));

    mp3_mem.seek(0);
    pplib::AudioDecoder_MP3 mp3_decoder;
    mp3_decoder.open(mp3_mem);
    const pplib::AudioInfo dec_info = mp3_decoder.getAudioInfo();
    std::vector<pplib::STEREOSAMPLE16> samples(dec_info.Samples);
    size_t read_count = mp3_decoder.getSamples(dec_info.Samples, samples.data());
    EXPECT_GT(read_count, 0u);

    double energy = 0.0;
    for (size_t i = 0; i < read_count; ++i) {
        energy += static_cast<double>(samples[i].left) * samples[i].left;
        energy += static_cast<double>(samples[i].right) * samples[i].right;
    }
    EXPECT_GT(energy, 0.0);
}

TEST_F(AudioEncoderMp3Test, ModesVbrAbrMonoAndFilters)
{
    pplib::File aiff;
    aiff.open("testdata/audio/test_44kHz.aiff");
    pplib::AudioDecoder_Aiff aiff_decoder;
    aiff_decoder.open(aiff);

    auto encodeWithConfig = [&](auto configure) -> size_t {
        pplib::MemFile mem;
        pplib::AudioEncoder_MP3 enc;
        configure(enc);
        enc.startEncode(mem);
        enc.encode(aiff_decoder);
        enc.finish();

        mem.seek(0);
        pplib::AudioInfo info;
        EXPECT_TRUE(pplib::IdentAudioFile(mem, info));
        EXPECT_EQ(pplib::AudioInfo::AudioFormat::MP3, info.Format);
        EXPECT_EQ(44100u, info.Frequency);

        mem.seek(0);
        pplib::AudioDecoder_MP3 dec;
        dec.open(mem);
        pplib::STEREOSAMPLE16 buf[1024];
        EXPECT_GT(dec.getSamples(1024, buf), 0u);
        return mem.size();
    };

    size_t size128 = encodeWithConfig([](pplib::AudioEncoder_MP3& e) { e.setCBR(128, 5); });
    size_t size320 = encodeWithConfig([](pplib::AudioEncoder_MP3& e) { e.setCBR(320, 2); });
    EXPECT_GT(size320, size128);

    EXPECT_GT(encodeWithConfig([](pplib::AudioEncoder_MP3& e) {
                  e.setVBR(64, 192, 4);
                  e.setStereoMode(pplib::AudioInfo::ChannelMode::STEREO);
                  e.setLowpassFreq(16000);
                  e.setHighpassFreq(100);
              }),
              0u);

    EXPECT_GT(encodeWithConfig([](pplib::AudioEncoder_MP3& e) {
                  e.setABR(128, 2);
                  e.setLowpassFreq(-1);
                  e.setHighpassFreq(-1);
              }),
              0u);
}

TEST_F(AudioEncoderMp3Test, MonoSource)
{
    pplib::File aiff_mono;
    aiff_mono.open("testdata/audio/test_44kHz_16bit_mono.aiff");
    pplib::AudioDecoder_Aiff mono_decoder;
    mono_decoder.open(aiff_mono);

    pplib::MemFile mono_mem;
    pplib::AudioEncoder_MP3 mono_enc;
    mono_enc.setCBR(128, 2);
    mono_enc.setStereoMode(pplib::AudioInfo::ChannelMode::MONO);
    mono_enc.startEncode(mono_mem);
    mono_enc.encode(mono_decoder);
    mono_enc.finish();

    mono_mem.seek(0);
    pplib::AudioInfo mono_info;
    ASSERT_TRUE(pplib::IdentAudioFile(mono_mem, mono_info));
    EXPECT_EQ(1, mono_info.Channels);
}

TEST_F(AudioEncoderMp3Test, MultipleEncodeConcatenation)
{
    pplib::File aiff;
    aiff.open("testdata/audio/test_44kHz.aiff");
    pplib::AudioDecoder_Aiff aiff_decoder;
    aiff_decoder.open(aiff);
    const pplib::AudioInfo src_info = aiff_decoder.getAudioInfo();

    pplib::MemFile mp3_mem;
    pplib::AudioEncoder_MP3 mp3_encoder;
    mp3_encoder.setCBR(192, 2);
    mp3_encoder.startEncode(mp3_mem);
    mp3_encoder.encode(aiff_decoder);
    mp3_encoder.encode(aiff_decoder);
    mp3_encoder.finish();

    mp3_mem.seek(0);
    pplib::AudioInfo mp3_info;
    ASSERT_TRUE(pplib::IdentAudioFile(mp3_mem, mp3_info));
    // Durch MP3-Frame-Padding kann die Samplezahl leicht von 2 * src_info.Samples abweichen
    EXPECT_GT(mp3_info.Samples, src_info.Samples + src_info.Samples / 2);
}

TEST_F(AudioEncoderMp3Test, ProgressCallbackAndStopEncoder)
{
    struct ProgressState
    {
        std::vector<int> values;
        pplib::AudioEncoder_MP3* stopTarget = nullptr;
    };

    auto progress_cb = [](int progress, void* priv) {
        auto* st = static_cast<ProgressState*>(priv);
        st->values.push_back(progress);
        if (st->stopTarget) {
            st->stopTarget->stopEncoder();
        }
    };

    pplib::File aiff;
    aiff.open("testdata/audio/test_44kHz.aiff");
    pplib::AudioDecoder_Aiff aiff_decoder;
    aiff_decoder.open(aiff);

    ProgressState state;
    pplib::MemFile mp3_mem;
    pplib::AudioEncoder_MP3 mp3_encoder;
    mp3_encoder.setProgressFunction(progress_cb, &state);
    mp3_encoder.startEncode(mp3_mem);
    mp3_encoder.encode(aiff_decoder);
    mp3_encoder.finish();

    ASSERT_FALSE(state.values.empty());
    EXPECT_EQ(100, state.values.back());
    for (size_t i = 1; i < state.values.size(); ++i) {
        EXPECT_GT(state.values[i], state.values[i - 1]);
    }

    ProgressState abort_state;
    pplib::AudioEncoder_MP3 abort_encoder;
    abort_state.stopTarget = &abort_encoder;
    pplib::MemFile aborted_mem;
    abort_encoder.setProgressFunction(progress_cb, &abort_state);
    abort_encoder.startEncode(aborted_mem);
    EXPECT_THROW(abort_encoder.encode(aiff_decoder), pplib::EncoderAbortedException);
    abort_encoder.finish();
}

TEST_F(AudioEncoderMp3Test, LowLevelBufferEncodingAndChannelSeparation)
{
    const int freq = 44100;
    const int totalSamples = 22050; // 0,5 s
    std::vector<pplib::STEREOSAMPLE16> pcm(totalSamples);
    for (int i = 0; i < totalSamples; ++i) {
        double t = static_cast<double>(i) / freq;
        pcm[i].left = static_cast<pplib::SAMPLE16>(std::sin(2.0 * 3.14159265358979323846 * 440.0 * t) * 20000.0);
        pcm[i].right = 0;
    }

    pplib::AudioEncoder_MP3 enc;
    enc.setCBR(192, 2);
    enc.setStereoMode(pplib::AudioInfo::ChannelMode::STEREO);
    enc.startEncode(freq, 2);

    std::vector<unsigned char> outbuf(65536);
    pplib::MemFile mp3_mem;
    int written = enc.encodeBuffer(pcm.data(), totalSamples, outbuf.data(), outbuf.size());
    ASSERT_GE(written, 0);
    if (written > 0) mp3_mem.write(outbuf.data(), written);

    int flushed = enc.flushBuffer(outbuf.data(), outbuf.size());
    ASSERT_GE(flushed, 0);
    if (flushed > 0) mp3_mem.write(outbuf.data(), flushed);
    enc.finish();

    mp3_mem.seek(0);
    pplib::AudioDecoder_MP3 dec;
    dec.open(mp3_mem);
    const pplib::AudioInfo info = dec.getAudioInfo();
    std::vector<pplib::STEREOSAMPLE16> decoded(info.Samples);
    size_t got = dec.getSamples(info.Samples, decoded.data());
    ASSERT_GT(got, 0u);

    double energyLeft = 0.0;
    double energyRight = 0.0;
    for (size_t i = 0; i < got; ++i) {
        energyLeft += static_cast<double>(decoded[i].left) * decoded[i].left;
        energyRight += static_cast<double>(decoded[i].right) * decoded[i].right;
    }
    EXPECT_GT(energyLeft, 1e9);
    EXPECT_GT(energyLeft, energyRight * 100.0);
}

TEST_F(AudioEncoderMp3Test, LowLevelSplitChannelsAndBufferTooSmall)
{
    const int numSamples = 4096;
    std::vector<pplib::SAMPLE16> left(numSamples, 1000);
    std::vector<pplib::SAMPLE16> right(numSamples, -1000);

    pplib::AudioEncoder_MP3 enc;
    enc.startEncode(44100, 2);

    unsigned char tinyBuf[8];
    EXPECT_THROW(enc.encodeBuffer(left.data(), right.data(), numSamples, tinyBuf, sizeof(tinyBuf)), pplib::EncoderBufferTooSmallException);

    std::vector<unsigned char> okBuf(32768);
    int bytes = enc.encodeBuffer(left.data(), right.data(), numSamples, okBuf.data(), okBuf.size());
    EXPECT_GE(bytes, 0);
    int flushed = enc.flushBuffer(okBuf.data(), okBuf.size());
    EXPECT_GT(flushed, 0);
    enc.finish();
}

TEST_F(AudioEncoderMp3Test, ErrorPaths)
{
    pplib::AudioEncoder_MP3 enc;
    EXPECT_THROW(enc.setStereoMode(pplib::AudioInfo::ChannelMode::DUAL_CHANNEL), pplib::UnsupportedFeatureException);

    pplib::File aiff_stereo;
    aiff_stereo.open("testdata/audio/test_44kHz.aiff");
    pplib::AudioDecoder_Aiff dec_stereo;
    dec_stereo.open(aiff_stereo);

    pplib::File aiff_mono;
    aiff_mono.open("testdata/audio/test_44kHz_16bit_mono.aiff");
    pplib::AudioDecoder_Aiff dec_mono;
    dec_mono.open(aiff_mono);

    EXPECT_THROW(enc.encode(dec_stereo), pplib::EncoderNotStartedException);
    EXPECT_THROW(enc.finish(), pplib::EncoderNotStartedException);

    pplib::STEREOSAMPLE16 dummy[64] = {};
    unsigned char outbuf[8192];
    EXPECT_THROW(enc.encodeBuffer(dummy, 64, outbuf, sizeof(outbuf)), pplib::EncoderNotStartedException);
    EXPECT_THROW(enc.encodeBuffer((pplib::SAMPLE16*)dummy, (pplib::SAMPLE16*)dummy, 64, outbuf, sizeof(outbuf)),
                 pplib::EncoderNotStartedException);

    pplib::MemFile mp3_mem;
    enc.startEncode(mp3_mem);
    EXPECT_THROW(enc.startEncode(mp3_mem), pplib::EncoderAlreadyStartedException);

    enc.encode(dec_stereo);
    EXPECT_THROW(enc.encode(dec_mono), pplib::EncoderAudioFormatMismatchException);
    enc.finish();
}

} // namespace

#endif // HAVE_LAME