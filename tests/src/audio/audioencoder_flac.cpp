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
#include <pplib/audio/id3tag.h>
#include <pplib/audio/decoder.h>
#include <pplib/audio/encoder.h>
#include <pplib/audio/sample_formats.h>

#include "pplib-tests.h"

#ifdef HAVE_LIBFLAC

namespace
{

class AudioEncoderFlacTest : public ::testing::Test
{
protected:
    AudioEncoderFlacTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~AudioEncoderFlacTest()
    {
    }
};

TEST_F(AudioEncoderFlacTest, Roundtrip)
{
    // We use an aiff file with ID3Tags as Source, compress it to flac and then
    // decompress it back and compare the audio samples with the original ones from the AIFF file.

    pplib::File aiff;
    aiff.open("testdata/audio/test_44kHz_taggedWithCover.aiff");
    pplib::ID3Tag original_tag;
    original_tag.load(aiff);

    pplib::AudioDecoder_Aiff aiff_decoder;
    aiff_decoder.open(aiff);
    pplib::AudioInfo info = aiff_decoder.getAudioInfo();
    //info.print();
    EXPECT_EQ(44100, info.Frequency);
    EXPECT_EQ(2, info.Channels);
    EXPECT_EQ(16, info.BitsPerSample);
    EXPECT_EQ(173695, info.Samples);
    // we read the audio samples into a buffer
    pplib::ByteArray original_samples(info.Samples * sizeof(pplib::STEREOSAMPLE16));
    size_t samples_read = aiff_decoder.getSamples(info.Samples, (pplib::STEREOSAMPLE16*)original_samples.ptr());
    EXPECT_EQ(info.Samples, samples_read);
    pplib::Dir::mkDir("tmp");

    pplib::File flac;
    flac.open("tmp/test.flac", pplib::File::FileMode::WRITE);
    pplib::AudioEncoder_Flac flac_encoder;
    flac_encoder.setCompressionLevel(8);
    flac_encoder.startEncode(flac);
    flac_encoder.setVorbisComments(original_tag);
    flac_encoder.encode(aiff_decoder);
    flac_encoder.finish();
    flac.close();

    pplib::File flac_in;
    flac_in.open("tmp/test.flac");
    pplib::ID3Tag decoded_tag;
    decoded_tag.load(flac_in);
    //decoded_tag.listFrames();
    pplib::AudioDecoder_Flac flac_decoder;
    flac_decoder.open(flac_in);
    pplib::ByteArray decoded_samples(info.Samples * sizeof(pplib::STEREOSAMPLE16));
    size_t decoded_samples_read = flac_decoder.getSamples(info.Samples, (pplib::STEREOSAMPLE16*)decoded_samples.ptr());
    EXPECT_EQ(info.Samples, decoded_samples_read);
    EXPECT_EQ(original_samples, decoded_samples);
    pplib::AudioInfo flac_info = flac_decoder.getAudioInfo();
    EXPECT_EQ(44100, flac_info.Frequency);
    EXPECT_EQ(2, flac_info.Channels);
    EXPECT_EQ(16, flac_info.BitsPerSample);
    EXPECT_EQ(173695, flac_info.Samples);
    flac_in.close();
    pplib::File::remove("tmp/test.flac");

    // ID3Tag comparison
    EXPECT_EQ(original_tag.getArtist(), decoded_tag.getArtist());
    EXPECT_EQ(original_tag.getTitle(), decoded_tag.getTitle());
    EXPECT_EQ(original_tag.getAlbum(), decoded_tag.getAlbum());
    EXPECT_EQ(original_tag.getYear(), decoded_tag.getYear());
    EXPECT_EQ(original_tag.getComment(), decoded_tag.getComment());
    EXPECT_EQ(original_tag.getRemixer(), decoded_tag.getRemixer());
    EXPECT_EQ(original_tag.getLabel(), decoded_tag.getLabel());
    EXPECT_EQ(original_tag.getTrack(), decoded_tag.getTrack());
    EXPECT_EQ(original_tag.getGenre(), decoded_tag.getGenre());
    EXPECT_EQ(original_tag.getBPM(), decoded_tag.getBPM());
    EXPECT_EQ(original_tag.getKey(), decoded_tag.getKey());
    EXPECT_EQ(original_tag.getEnergyLevel(), decoded_tag.getEnergyLevel());
    EXPECT_EQ(original_tag.getPicture(0), decoded_tag.getPicture(0));
}

} // namespace

#endif // HAVE_LIBFLAC