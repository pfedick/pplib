/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2014, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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

#include "../include/prolog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include "../include/ppl6.h"
#include <gtest/gtest.h>
#include "ppl6-tests.h"
#include "../include/config.h"
#include "../include/ppl6-sound.h"


namespace {

class AudioInfoTest : public ::testing::Test {
	protected:
	AudioInfoTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl6::SetGlobalEncoding("UTF-8");
	}
	virtual ~AudioInfoTest() {

	}
};

TEST_F(AudioInfoTest, ConstructorAudioInfo) {
	ASSERT_NO_THROW({
		ppl6::AudioInfo Info;
	});
}

TEST_F(AudioInfoTest, IdentMp3CBR192WithoutID3) {
	ppl6::CFile file;
	ASSERT_EQ(1,file.Open("testdata/test_192cbr.mp3"));
	ppl6::AudioInfo info;
	ASSERT_TRUE(ppl6::IdentAudioFile(file,info));
	EXPECT_EQ(ppl6::AudioInfo::MP3,info.Format);
	EXPECT_FALSE(info.HaveID3v2Tag);
	EXPECT_FALSE(info.IsVBR);
	EXPECT_EQ((ppluint16)192,info.Bitrate);
	EXPECT_EQ((ppluint8)2,info.Channels);
	EXPECT_EQ((ppluint8)4,info.BytesPerSample);
	EXPECT_EQ((ppluint32)44100,info.Frequency);


}


TEST_F(AudioInfoTest, IdentAiffWithoutID3) {
	ppl6::CFile file;
	ASSERT_EQ(1,file.Open("testdata/test_44kHz.aiff"));
	ppl6::AudioInfo info;
	ASSERT_TRUE(ppl6::IdentAudioFile(file,info)) << "Audio format was not recognized";
	EXPECT_EQ(ppl6::AudioInfo::AIFF,info.Format) << "Wrong audio format detected";
	EXPECT_FALSE(info.HaveID3v2Tag) << "ID3-Tag detected, but there is none in the file";
	EXPECT_FALSE(info.IsVBR) << "VBR detected, but it shouldn't";
	EXPECT_EQ((ppluint16)1411,info.Bitrate) << "Unexpected Bitrate";
	EXPECT_EQ((ppluint8)2,info.Channels) << "Unexpected number of channels";
	EXPECT_EQ((ppluint8)4,info.BytesPerSample) << "Unexpected bytes per sample";
	EXPECT_EQ((ppluint32)44100,info.Frequency) << "Unexpected frequency";


}

}	// EOF namespace

