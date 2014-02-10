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

class CID3TagTest : public ::testing::Test {
	protected:
	CID3TagTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl6::SetGlobalEncoding("UTF-8");
	}
	virtual ~CID3TagTest() {

	}
};

TEST_F(CID3TagTest, ConstructorWithoutFile) {
	ASSERT_NO_THROW({
		ppl6::CID3Tag Tags;
	});
}

TEST_F(CID3TagTest, LoadFileWithoutTags) {
	ppl6::CID3Tag Tags;
	EXPECT_EQ(0,Tags.Load("testdata/test_192cbr.mp3")) << "Loading MP3 File failed";
	EXPECT_EQ(402,ppl6::GetErrorCode()) << "Unexpected errorcode";
}

TEST_F(CID3TagTest, LoadFileWithTags) {
	ppl6::CID3Tag Tags;
	EXPECT_EQ(1,Tags.Load("testdata/test_192cbr_taggedWithCover.mp3")) << "Loading MP3 File failed";
	EXPECT_EQ(ppl6::CString("Patrick Fedick"),Tags.GetArtist());
	EXPECT_EQ(ppl6::CString("Powerplay Jingle"),Tags.GetTitle());
	EXPECT_EQ(ppl6::CString("Trance"),Tags.GetGenre());
	EXPECT_EQ(ppl6::CString("Single"),Tags.GetRemixer());
	EXPECT_EQ(ppl6::CString("Patrick F.-Productions"),Tags.GetLabel());
	EXPECT_EQ(ppl6::CString("PPL Testdata"),Tags.GetComment());
	EXPECT_EQ(ppl6::CString("2013"),Tags.GetYear());
	EXPECT_EQ(ppl6::CString("PPL Testsuite"),Tags.GetAlbum());
	EXPECT_EQ(ppl6::CString("2"),Tags.GetTrack());
	EXPECT_EQ(ppl6::CString("138"),Tags.GetBPM());
	EXPECT_EQ(ppl6::CString("am"),Tags.GetKey());
	EXPECT_EQ(ppl6::CString("9"),Tags.GetEnergyLevel());
	ppl6::CBinary cover;
	EXPECT_EQ(1,Tags.GetPicture(3,cover));
	EXPECT_EQ((size_t)28402,cover.Size()) << "Embedded Cover has unexpected size";
	EXPECT_EQ(ppl6::CString("d665f69f04f1413eef91b3596de8dfb6"),cover.GetMD5Sum()) << "Embedded Cover has unexpected MD5 hash";
}



TEST_F(CID3TagTest, InitialTaggingWithoutPicture) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_192cbr.mp3","tmp/test_tagged1.mp3"));

	ASSERT_EQ(0,Tags.Load("tmp/test_tagged1.mp3"));
	EXPECT_EQ(402,ppl6::GetErrorCode()) << "Unexpected errorcode";
	EXPECT_EQ(1,Tags.SetArtist("Patrick Fedick"));
	EXPECT_EQ(1,Tags.SetTitle("Powerplay Jingle"));
	EXPECT_EQ(1,Tags.SetGenre("Trance"));
	EXPECT_EQ(1,Tags.SetRemixer("Single"));
	EXPECT_EQ(1,Tags.SetLabel("Patrick F.-Productions"));
	EXPECT_EQ(1,Tags.SetComment("PPL Testdata"));
	EXPECT_EQ(1,Tags.SetYear("2013"));
	EXPECT_EQ(1,Tags.SetAlbum("PPL Testsuite"));
	EXPECT_EQ(1,Tags.SetTrack("2"));
	EXPECT_EQ(1,Tags.SetBPM("138"));
	EXPECT_EQ(1,Tags.SetKey("am"));
	EXPECT_EQ(1,Tags.SetEnergyLevel("9"));
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged1.mp3",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)97072,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("861daa2b4dbdbab8189961622b2d36dc"),ppl6::CFile::MD5("tmp/test_tagged1.mp3"));


}

TEST_F(CID3TagTest, InitialTaggingWithPicture) {
	ppl6::CID3Tag Tags;
	ppl6::CBinary cover;
	cover.Load("testdata/cover.jpg");


	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_192cbr.mp3","tmp/test_tagged2.mp3"));

	ASSERT_EQ(0,Tags.Load("tmp/test_tagged2.mp3"));
	EXPECT_EQ(402,ppl6::GetErrorCode()) << "Unexpected errorcode";
	EXPECT_EQ(1,Tags.SetArtist("Patrick Fedick"));
	EXPECT_EQ(1,Tags.SetTitle("Powerplay Jingle"));
	EXPECT_EQ(1,Tags.SetGenre("Trance"));
	EXPECT_EQ(1,Tags.SetRemixer("Single"));
	EXPECT_EQ(1,Tags.SetLabel("Patrick F.-Productions"));
	EXPECT_EQ(1,Tags.SetComment("PPL Testdata"));
	EXPECT_EQ(1,Tags.SetYear("2013"));
	EXPECT_EQ(1,Tags.SetAlbum("PPL Testsuite"));
	EXPECT_EQ(1,Tags.SetTrack("2"));
	EXPECT_EQ(1,Tags.SetBPM("138"));
	EXPECT_EQ(1,Tags.SetKey("am"));
	EXPECT_EQ(1,Tags.SetEnergyLevel("9"));
	EXPECT_EQ(1,Tags.SetPicture(3,cover,"image/jpeg"));

	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged2.mp3",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)125071,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("da9fdb368b613511ae881a452ebb4e91"),ppl6::CFile::MD5("tmp/test_tagged2.mp3"));
}

TEST_F(CID3TagTest, RetagWithPicture) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_192cbr_tagged.mp3","tmp/test_tagged3.mp3"));
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged3.mp3"));

	ppl6::CBinary cover;
	cover.Load("testdata/cover.jpg");
	EXPECT_EQ(1,Tags.SetPicture(3,cover,"image/jpeg"));
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged3.mp3",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)125072,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("1b11537ba011455c3ef04e5b3134faa8"),ppl6::CFile::MD5("tmp/test_tagged3.mp3"));
}

TEST_F(CID3TagTest, RetagStrings) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_192cbr_tagged.mp3","tmp/test_tagged4.mp3"));
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged4.mp3"));

	EXPECT_EQ(1,Tags.SetArtist("New Artist"));
	EXPECT_EQ(1,Tags.SetTitle("New Jingle"));
	EXPECT_EQ(1,Tags.SetGenre("Techno"));
	EXPECT_EQ(1,Tags.SetRemixer("Maxi"));
	EXPECT_EQ(1,Tags.SetLabel("New Label"));
	EXPECT_EQ(1,Tags.SetComment("New Testdata"));
	EXPECT_EQ(1,Tags.SetYear("2014"));
	EXPECT_EQ(1,Tags.SetAlbum("New Testsuite"));
	EXPECT_EQ(1,Tags.SetTrack("3"));
	EXPECT_EQ(1,Tags.SetBPM("140"));
	EXPECT_EQ(1,Tags.SetKey("em"));
	EXPECT_EQ(1,Tags.SetEnergyLevel("7"));
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";

	ppl6::CID3Tag NewTags;
	EXPECT_EQ(1,NewTags.Load("tmp/test_tagged4.mp3")) << "Loading MP3 File failed";
	EXPECT_EQ(ppl6::CString("New Artist"),NewTags.GetArtist());
	EXPECT_EQ(ppl6::CString("New Jingle"),NewTags.GetTitle());
	EXPECT_EQ(ppl6::CString("Techno"),NewTags.GetGenre());
	EXPECT_EQ(ppl6::CString("Maxi"),NewTags.GetRemixer());
	EXPECT_EQ(ppl6::CString("New Label"),NewTags.GetLabel());
	EXPECT_EQ(ppl6::CString("New Testdata"),NewTags.GetComment());
	EXPECT_EQ(ppl6::CString("2014"),NewTags.GetYear());
	EXPECT_EQ(ppl6::CString("New Testsuite"),NewTags.GetAlbum());
	EXPECT_EQ(ppl6::CString("3"),NewTags.GetTrack());
	EXPECT_EQ(ppl6::CString("140"),NewTags.GetBPM());
	EXPECT_EQ(ppl6::CString("em"),NewTags.GetKey());
	EXPECT_EQ(ppl6::CString("7"),NewTags.GetEnergyLevel());

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged4.mp3",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)97072,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("28c267be5390f2a43161331ad114ec9e"),ppl6::CFile::MD5("tmp/test_tagged4.mp3"));
}


TEST_F(CID3TagTest, RemovePicture) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_192cbr_taggedWithCover.mp3","tmp/test_tagged5.mp3"));
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged5.mp3"));
	ASSERT_NO_THROW(Tags.RemovePicture(3));
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged5.mp3",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)97073,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("ba390b6a0998b6e6a57cf982fd83d794"),ppl6::CFile::MD5("tmp/test_tagged5.mp3"));
}

TEST_F(CID3TagTest, RemoveAllTags) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_192cbr_taggedWithCover.mp3","tmp/test_tagged6.mp3"));
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged6.mp3"));
	ASSERT_NO_THROW(Tags.ClearTags());
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged6.mp3",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)97073,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("fa7113a23a13aebbbdfec39a4cfa3b2f"),ppl6::CFile::MD5("tmp/test_tagged6.mp3"));
}


}	// EOF namespace

