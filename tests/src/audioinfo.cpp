/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 600 $
 * $Date: 2013-04-26 21:37:49 +0200 (Fr, 26 Apr 2013) $
 * $Id: assocarray.cpp 600 2013-04-26 19:37:49Z pafe $
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <ppl7.h>
#include <ppl7-audio.h>
#include <gtest/gtest.h>
#include "ppl7-tests.h"


namespace {

class AudioInfoTest : public ::testing::Test {
	protected:
	AudioInfoTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~AudioInfoTest() {

	}
};

TEST_F(AudioInfoTest, ConstructorAudioInfo) {
	ASSERT_NO_THROW({
		ppl7::AudioInfo Info;
	});
}

TEST_F(AudioInfoTest, IdentMp3CBR192WithoutID3) {
	ppl7::File file;
	file.open("testdata/test_192cbr.mp3");
	ppl7::AudioInfo info;
	ASSERT_TRUE(ppl7::IdentAudioFile(file,info));
	EXPECT_EQ(ppl7::AudioInfo::MP3,info.Format);
	EXPECT_FALSE(info.HaveID3v2Tag);
	EXPECT_EQ((ppluint32)0,info.ID3v2TagStart);
	EXPECT_FALSE(info.IsVBR);
	EXPECT_EQ((ppluint16)192,info.Bitrate);
	EXPECT_EQ((ppluint8)2,info.Channels);
	EXPECT_EQ((ppluint8)4,info.BytesPerSample);
	EXPECT_EQ((ppluint32)44100,info.Frequency);
	EXPECT_EQ((ppluint32)176223,info.Samples);
	EXPECT_EQ(ppl7::AudioInfo::JOINT_STEREO,info.Mode);
}

TEST_F(AudioInfoTest, IdentMp3CBR320WithoutID3) {
	ppl7::File file;
	file.open("testdata/test_320cbr.mp3");
	ppl7::AudioInfo info;
	ASSERT_TRUE(ppl7::IdentAudioFile(file,info));
	EXPECT_EQ(ppl7::AudioInfo::MP3,info.Format);
	EXPECT_FALSE(info.HaveID3v2Tag);
	EXPECT_EQ((ppluint32)0,info.ID3v2TagStart);
	EXPECT_FALSE(info.IsVBR);
	EXPECT_EQ((ppluint16)320,info.Bitrate);
	EXPECT_EQ((ppluint8)2,info.Channels);
	EXPECT_EQ((ppluint8)4,info.BytesPerSample);
	EXPECT_EQ((ppluint32)44100,info.Frequency);
	EXPECT_EQ((ppluint32)176223,info.Samples);
}

TEST_F(AudioInfoTest, IdentMp3VBR192WithoutID3) {
	ppl7::File file;
	file.open("testdata/test_192vbr.mp3");
	ppl7::AudioInfo info;
	ASSERT_TRUE(ppl7::IdentAudioFile(file,info));
	EXPECT_EQ(ppl7::AudioInfo::MP3,info.Format);
	EXPECT_FALSE(info.HaveID3v2Tag);
	EXPECT_EQ((ppluint32)0,info.ID3v2TagStart);
	EXPECT_TRUE(info.IsVBR);
	EXPECT_EQ((ppluint16)176,info.Bitrate);
	EXPECT_EQ((ppluint8)2,info.Channels);
	EXPECT_EQ((ppluint8)4,info.BytesPerSample);
	EXPECT_EQ((ppluint32)44100,info.Frequency);
	EXPECT_EQ((ppluint32)176256,info.Samples);
}

TEST_F(AudioInfoTest, IdentMp3CBR192WithID3) {
	ppl7::File file;
	file.open("testdata/test_192cbr_tagged.mp3");
	ppl7::AudioInfo info;
	ASSERT_TRUE(ppl7::IdentAudioFile(file,info));
	EXPECT_EQ(ppl7::AudioInfo::MP3,info.Format);
	EXPECT_TRUE(info.HaveID3v2Tag);
	EXPECT_EQ((ppluint32)0,info.ID3v2TagStart);
	EXPECT_FALSE(info.IsVBR);
	EXPECT_EQ((ppluint16)192,info.Bitrate);
	EXPECT_EQ((ppluint8)2,info.Channels);
	EXPECT_EQ((ppluint8)4,info.BytesPerSample);
	EXPECT_EQ((ppluint32)44100,info.Frequency);
	EXPECT_EQ((ppluint32)176223,info.Samples);
}


TEST_F(AudioInfoTest, IdentAiffWithoutID3) {
	ppl7::File file;
	file.open("testdata/test_44kHz.aiff");
	ppl7::AudioInfo info;
	ASSERT_TRUE(ppl7::IdentAudioFile(file,info)) << "Audio format was not recognized";
	EXPECT_EQ(ppl7::AudioInfo::AIFF,info.Format) << "Wrong audio format detected";
	EXPECT_FALSE(info.HaveID3v2Tag) << "ID3-Tag detected, but there is none in the file";
	EXPECT_EQ((ppluint32)0,info.ID3v2TagStart);
	EXPECT_FALSE(info.IsVBR) << "VBR detected, but it shouldn't";
	EXPECT_EQ((ppluint16)1411,info.Bitrate) << "Unexpected Bitrate";
	EXPECT_EQ((ppluint8)2,info.Channels) << "Unexpected number of channels";
	EXPECT_EQ((ppluint8)4,info.BytesPerSample) << "Unexpected bytes per sample";
	EXPECT_EQ((ppluint32)44100,info.Frequency) << "Unexpected frequency";
	EXPECT_EQ((ppluint32)173695,info.Samples) << "Unexpected number of samples";
	EXPECT_EQ((ppluint32)3938,info.Length) << "Unexpected length";
}

TEST_F(AudioInfoTest, IdentAiffWithID3) {
	ppl7::File file;
	file.open("testdata/test_44kHz_tagged.aiff");
	ppl7::AudioInfo info;
	ASSERT_TRUE(ppl7::IdentAudioFile(file,info)) << "Audio format was not recognized";
	EXPECT_EQ(ppl7::AudioInfo::AIFF,info.Format) << "Wrong audio format detected";
	EXPECT_TRUE(info.HaveID3v2Tag) << "ID3-Tag detected, but there is none in the file";
	EXPECT_EQ((ppluint32)694842,info.ID3v2TagStart);
	EXPECT_FALSE(info.IsVBR) << "VBR detected, but it shouldn't";
	EXPECT_EQ((ppluint16)1411,info.Bitrate) << "Unexpected Bitrate";
	EXPECT_EQ((ppluint8)2,info.Channels) << "Unexpected number of channels";
	EXPECT_EQ((ppluint8)4,info.BytesPerSample) << "Unexpected bytes per sample";
	EXPECT_EQ((ppluint32)44100,info.Frequency) << "Unexpected frequency";
	EXPECT_EQ((ppluint32)173695,info.Samples) << "Unexpected number of samples";
	EXPECT_EQ((ppluint32)3938,info.Length) << "Unexpected length";
}


TEST_F(AudioInfoTest, IdentWaveWithoutID3) {
	ppl7::File file;
	file.open("testdata/test_44kHz.wav");
	ppl7::AudioInfo info;
	ASSERT_TRUE(ppl7::IdentAudioFile(file,info)) << "Audio format was not recognized";
	EXPECT_EQ(ppl7::AudioInfo::WAVE,info.Format) << "Wrong audio format detected";
	EXPECT_FALSE(info.HaveID3v2Tag) << "ID3-Tag detected, but there is none in the file";
	EXPECT_EQ((ppluint32)0,info.ID3v2TagStart);
	EXPECT_FALSE(info.IsVBR) << "VBR detected, but it shouldn't";
	EXPECT_EQ((ppluint16)1411,info.Bitrate) << "Unexpected Bitrate";
	EXPECT_EQ((ppluint8)2,info.Channels) << "Unexpected number of channels";
	EXPECT_EQ((ppluint8)4,info.BytesPerSample) << "Unexpected bytes per sample";
	EXPECT_EQ((ppluint32)44100,info.Frequency) << "Unexpected frequency";
	EXPECT_EQ((ppluint32)173695,info.Samples) << "Unexpected number of samples";
	EXPECT_EQ((ppluint32)3938,info.Length) << "Unexpected length";

	EXPECT_EQ((ppluint32)694824,info.FileSize) << "Unexpected filesize";
	EXPECT_EQ((ppluint32)44,info.AudioStart) << "Unexpected audio start";
	EXPECT_EQ((ppluint32)694824-44,info.AudioSize) << "Unexpected audio size";
	EXPECT_EQ((ppluint32)694823,info.AudioEnd) << "Unexpected audio end";
}

}	// EOF namespace

