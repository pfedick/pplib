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

TEST_F(CID3TagTest, Mp3LoadFileWithoutTags) {
	ppl6::CID3Tag Tags;
	EXPECT_EQ(0,Tags.Load("testdata/test_192cbr.mp3")) << "Loading MP3 File failed";
	EXPECT_EQ(402,ppl6::GetErrorCode()) << "Unexpected errorcode";
}

TEST_F(CID3TagTest, Mp3LoadFileWithTags) {
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



TEST_F(CID3TagTest, Mp3InitialTaggingWithoutPicture) {
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
	EXPECT_EQ(ppl6::CString("f9a333ac0f6ee3c92fae02390b25248f"),ppl6::CFile::MD5("tmp/test_tagged1.mp3"));

	ppl6::CID3Tag NewTags;
	EXPECT_EQ(1,NewTags.Load("tmp/test_tagged1.mp3")) << "Loading MP3 File failed";
	EXPECT_EQ(ppl6::CString("Patrick Fedick"),NewTags.GetArtist());
	EXPECT_EQ(ppl6::CString("Powerplay Jingle"),NewTags.GetTitle());
	EXPECT_EQ(ppl6::CString("Trance"),NewTags.GetGenre());
	EXPECT_EQ(ppl6::CString("Single"),NewTags.GetRemixer());
	EXPECT_EQ(ppl6::CString("Patrick F.-Productions"),NewTags.GetLabel());
	EXPECT_EQ(ppl6::CString("PPL Testdata"),NewTags.GetComment());
	EXPECT_EQ(ppl6::CString("2013"),NewTags.GetYear());
	EXPECT_EQ(ppl6::CString("PPL Testsuite"),NewTags.GetAlbum());
	EXPECT_EQ(ppl6::CString("2"),NewTags.GetTrack());
	EXPECT_EQ(ppl6::CString("138"),NewTags.GetBPM());
	EXPECT_EQ(ppl6::CString("am"),NewTags.GetKey());
	EXPECT_EQ(ppl6::CString("9"),NewTags.GetEnergyLevel());
	ppl6::CBinary cover;
	EXPECT_EQ(0,NewTags.GetPicture(3,cover));


}

TEST_F(CID3TagTest, Mp3InitialTaggingWithPicture) {
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
	ASSERT_EQ((size_t)125073,d.Size) << "Tagged File has unexpected size";
	EXPECT_EQ(ppl6::CString("6dc503f9722059d5a0de7171e07c68e4"),ppl6::CFile::MD5("tmp/test_tagged2.mp3"));

	ppl6::CID3Tag NewTags;
	EXPECT_EQ(1,NewTags.Load("tmp/test_tagged2.mp3")) << "Loading MP3 File failed";
	EXPECT_EQ(ppl6::CString("Patrick Fedick"),NewTags.GetArtist());
	EXPECT_EQ(ppl6::CString("Powerplay Jingle"),NewTags.GetTitle());
	EXPECT_EQ(ppl6::CString("Trance"),NewTags.GetGenre());
	EXPECT_EQ(ppl6::CString("Single"),NewTags.GetRemixer());
	EXPECT_EQ(ppl6::CString("Patrick F.-Productions"),NewTags.GetLabel());
	EXPECT_EQ(ppl6::CString("PPL Testdata"),NewTags.GetComment());
	EXPECT_EQ(ppl6::CString("2013"),NewTags.GetYear());
	EXPECT_EQ(ppl6::CString("PPL Testsuite"),NewTags.GetAlbum());
	EXPECT_EQ(ppl6::CString("2"),NewTags.GetTrack());
	EXPECT_EQ(ppl6::CString("138"),NewTags.GetBPM());
	EXPECT_EQ(ppl6::CString("am"),NewTags.GetKey());
	EXPECT_EQ(ppl6::CString("9"),NewTags.GetEnergyLevel());
	EXPECT_EQ(1,NewTags.GetPicture(3,cover));
	EXPECT_EQ((size_t)28402,cover.Size()) << "Embedded Cover has unexpected size";
	EXPECT_EQ(ppl6::CString("d665f69f04f1413eef91b3596de8dfb6"),cover.GetMD5Sum()) << "Embedded Cover has unexpected MD5 hash";

}

TEST_F(CID3TagTest, Mp3RetagWithPicture) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_192cbr_tagged.mp3","tmp/test_tagged3.mp3"));
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged3.mp3"));

	ppl6::CBinary cover;
	cover.Load("testdata/cover.jpg");
	EXPECT_EQ(1,Tags.SetPicture(3,cover,"image/jpeg"));
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged3.mp3",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)125074,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("052fd75215f383718e1368b535520c60"),ppl6::CFile::MD5("tmp/test_tagged3.mp3"));
}

TEST_F(CID3TagTest, Mp3RetagStrings) {
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
	ASSERT_EQ(ppl6::CString("ffdf4dc172e7c5cc07b543557c33b6d1"),ppl6::CFile::MD5("tmp/test_tagged4.mp3"));
}

TEST_F(CID3TagTest, Mp3RemovePicture) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_192cbr_taggedWithCover.mp3","tmp/test_tagged5.mp3"));
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged5.mp3"));
	ASSERT_NO_THROW(Tags.RemovePicture(3));
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged5.mp3",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)97073,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("e2dacaeff7f3dbc0d54ed63e88ba519d"),ppl6::CFile::MD5("tmp/test_tagged5.mp3"));
}

TEST_F(CID3TagTest, Mp3RemoveAllTags) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_192cbr_taggedWithCover.mp3","tmp/test_tagged6.mp3"));
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged6.mp3"));
	ASSERT_NO_THROW(Tags.ClearTags());
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged6.mp3",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)95921,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("692bf339243cee92f1c639b10ffde45e"),ppl6::CFile::MD5("tmp/test_tagged6.mp3"));

}

TEST_F(CID3TagTest, Mp3NoTagsAndNoChange) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_192cbr.mp3","tmp/test_tagged7.mp3"));
	ASSERT_EQ(0,Tags.Load("tmp/test_tagged7.mp3"));
	ASSERT_NO_THROW(Tags.ClearTags());
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged7.mp3",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)95920,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("0abbdd3ce267358a0b3bf3f0a015e74e"),ppl6::CFile::MD5("tmp/test_tagged7.mp3"));

}

TEST_F(CID3TagTest, Mp3RetagWithoutChanges) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_192cbr_tagged.mp3","tmp/test_tagged8.mp3"));
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged8.mp3"));
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged8.mp3",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)97072,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("c5ff756219cba391c99423ddd6cca625"),ppl6::CFile::MD5("tmp/test_tagged8.mp3"));
}

TEST_F(CID3TagTest, AiffLoadFileWithoutTags) {
	ppl6::CID3Tag Tags;
	EXPECT_EQ(0,Tags.Load("testdata/test_44kHz.aiff")) << "Loading AIFF File failed";
	EXPECT_EQ(402,ppl6::GetErrorCode()) << "Unexpected errorcode";
}

TEST_F(CID3TagTest, AiffLoadFileWithTags) {
	ppl6::CID3Tag Tags;
	EXPECT_EQ(1,Tags.Load("testdata/test_44kHz_taggedWithCover.aiff")) << "Loading MP3 File failed";
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

TEST_F(CID3TagTest, AiffInitialTaggingWithoutPicture) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_44kHz.aiff","tmp/test_tagged1.aiff"));

	ASSERT_EQ(0,Tags.Load("tmp/test_tagged1.aiff"));
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
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged1.aiff",d)) << "Tagged File does not exist!";
	EXPECT_EQ((size_t)695866,d.Size) << "Tagged File has unexpected size";
	EXPECT_EQ(ppl6::CString("14b05fc1b22f9a16e575daa539d5ac21"),ppl6::CFile::MD5("tmp/test_tagged1.aiff"));

	ppl6::CID3Tag NewTags;
	ppl6::CBinary cover;
	EXPECT_EQ(1,NewTags.Load("tmp/test_tagged1.aiff")) << "Loading MP3 File failed";
	EXPECT_EQ(ppl6::CString("Patrick Fedick"),NewTags.GetArtist());
	EXPECT_EQ(ppl6::CString("Powerplay Jingle"),NewTags.GetTitle());
	EXPECT_EQ(ppl6::CString("Trance"),NewTags.GetGenre());
	EXPECT_EQ(ppl6::CString("Single"),NewTags.GetRemixer());
	EXPECT_EQ(ppl6::CString("Patrick F.-Productions"),NewTags.GetLabel());
	EXPECT_EQ(ppl6::CString("PPL Testdata"),NewTags.GetComment());
	EXPECT_EQ(ppl6::CString("2013"),NewTags.GetYear());
	EXPECT_EQ(ppl6::CString("PPL Testsuite"),NewTags.GetAlbum());
	EXPECT_EQ(ppl6::CString("2"),NewTags.GetTrack());
	EXPECT_EQ(ppl6::CString("138"),NewTags.GetBPM());
	EXPECT_EQ(ppl6::CString("am"),NewTags.GetKey());
	EXPECT_EQ(ppl6::CString("9"),NewTags.GetEnergyLevel());
	EXPECT_EQ(0,NewTags.GetPicture(3,cover));
}

TEST_F(CID3TagTest, AiffInitialTaggingWithPicture) {
	ppl6::CID3Tag Tags;
	ppl6::CBinary cover;
	cover.Load("testdata/cover.jpg");
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_44kHz.aiff","tmp/test_tagged2.aiff"));
	ASSERT_EQ(0,Tags.Load("tmp/test_tagged2.aiff"));
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
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged2.aiff",d)) << "Tagged File does not exist!";
	EXPECT_EQ((size_t)723867,d.Size) << "Tagged File has unexpected size";
	EXPECT_EQ(ppl6::CString("5afec7657c4cfb2f527ecb1fb9dd8f7e"),ppl6::CFile::MD5("tmp/test_tagged2.aiff"));

	ppl6::CID3Tag NewTags;
	EXPECT_EQ(1,NewTags.Load("tmp/test_tagged2.aiff")) << "Loading MP3 File failed";
	EXPECT_EQ(ppl6::CString("Patrick Fedick"),NewTags.GetArtist());
	EXPECT_EQ(ppl6::CString("Powerplay Jingle"),NewTags.GetTitle());
	EXPECT_EQ(ppl6::CString("Trance"),NewTags.GetGenre());
	EXPECT_EQ(ppl6::CString("Single"),NewTags.GetRemixer());
	EXPECT_EQ(ppl6::CString("Patrick F.-Productions"),NewTags.GetLabel());
	EXPECT_EQ(ppl6::CString("PPL Testdata"),NewTags.GetComment());
	EXPECT_EQ(ppl6::CString("2013"),NewTags.GetYear());
	EXPECT_EQ(ppl6::CString("PPL Testsuite"),NewTags.GetAlbum());
	EXPECT_EQ(ppl6::CString("2"),NewTags.GetTrack());
	EXPECT_EQ(ppl6::CString("138"),NewTags.GetBPM());
	EXPECT_EQ(ppl6::CString("am"),NewTags.GetKey());
	EXPECT_EQ(ppl6::CString("9"),NewTags.GetEnergyLevel());
	EXPECT_EQ(1,NewTags.GetPicture(3,cover));
	EXPECT_EQ((size_t)28402,cover.Size()) << "Embedded Cover has unexpected size";
	EXPECT_EQ(ppl6::CString("d665f69f04f1413eef91b3596de8dfb6"),cover.GetMD5Sum()) << "Embedded Cover has unexpected MD5 hash";


}

TEST_F(CID3TagTest, AiffRetagWithPicture) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_44kHz_tagged.aiff","tmp/test_tagged3.aiff"));
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged3.aiff"));

	ppl6::CBinary cover;
	cover.Load("testdata/cover.jpg");
	EXPECT_EQ(1,Tags.SetPicture(3,cover,"image/jpeg"));
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged3.aiff",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)723867,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("642691d6d125f70c222c1e8515cecdf5"),ppl6::CFile::MD5("tmp/test_tagged3.aiff"));
}

TEST_F(CID3TagTest, AiffRetagStrings) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_44kHz_tagged.aiff","tmp/test_tagged4.aiff"));
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged4.aiff"));

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
	EXPECT_EQ(1,NewTags.Load("tmp/test_tagged4.aiff")) << "Loading MP3 File failed";
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
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged4.aiff",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)695866,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("a254ec79ef0169d2fd6ad70f98a4c193"),ppl6::CFile::MD5("tmp/test_tagged4.aiff"));
}

TEST_F(CID3TagTest, AiffRetagStringsAndRetagWithCoverAgain) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_44kHz_tagged.aiff","tmp/test_tagged10.aiff"));
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged10.aiff"));

	EXPECT_EQ(1,Tags.SetArtist("Old Artist"));
	EXPECT_EQ(1,Tags.SetTitle("Old Jingle"));
	EXPECT_EQ(1,Tags.SetGenre("OldTechno"));
	EXPECT_EQ(1,Tags.SetRemixer("Old Maxi"));
	EXPECT_EQ(1,Tags.SetLabel("Old Label"));
	EXPECT_EQ(1,Tags.SetComment("Old Testdata"));
	EXPECT_EQ(1,Tags.SetYear("2013"));
	EXPECT_EQ(1,Tags.SetAlbum("Old Testsuite"));
	EXPECT_EQ(1,Tags.SetTrack("2"));
	EXPECT_EQ(1,Tags.SetBPM("120"));
	EXPECT_EQ(1,Tags.SetKey("am"));
	EXPECT_EQ(1,Tags.SetEnergyLevel("6"));
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs first time failed";

	ppl6::CID3Tag Tags2;
	ASSERT_EQ(1,Tags2.Load("tmp/test_tagged10.aiff")) << "Loading Tags after first save failed";
	EXPECT_EQ(1,Tags2.SetArtist("New Artist"));
	EXPECT_EQ(1,Tags2.SetTitle("New Jingle"));
	EXPECT_EQ(1,Tags2.SetGenre("Techno"));
	EXPECT_EQ(1,Tags2.SetRemixer("Maxi"));
	EXPECT_EQ(1,Tags2.SetLabel("New Label"));
	EXPECT_EQ(1,Tags2.SetComment("New Testdata"));
	EXPECT_EQ(1,Tags2.SetYear("2014"));
	EXPECT_EQ(1,Tags2.SetAlbum("New Testsuite"));
	EXPECT_EQ(1,Tags2.SetTrack("3"));
	EXPECT_EQ(1,Tags2.SetBPM("140"));
	EXPECT_EQ(1,Tags2.SetKey("em"));
	EXPECT_EQ(1,Tags2.SetEnergyLevel("7"));
	ppl6::CBinary cover;
	cover.Load("testdata/cover.jpg");
	EXPECT_EQ(1,Tags2.SetPicture(3,cover,"image/jpeg"));
	ASSERT_EQ(1,Tags2.Save()) << "Saving taggs the second time failed";

	ppl6::CID3Tag NewTags;
	EXPECT_EQ(1,NewTags.Load("tmp/test_tagged10.aiff")) << "Loading Tags after second save failed";
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
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged10.aiff",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)723813,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("da81ab68c0be03a0ff2be8e798232569"),ppl6::CFile::MD5("tmp/test_tagged10.aiff"));
}



TEST_F(CID3TagTest, AiffRemovePicture) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_44kHz_taggedWithCover.aiff","tmp/test_tagged5.aiff"));
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged5.aiff"));
	ASSERT_NO_THROW(Tags.RemovePicture(3));
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged5.aiff",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)695866,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("ddc103beb0e1687dd6631e31a4a06a62"),ppl6::CFile::MD5("tmp/test_tagged5.aiff"));
}

TEST_F(CID3TagTest, AiffRemoveAllTags) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_44kHz_taggedWithCover.aiff","tmp/test_tagged6.aiff"));
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged6.aiff"));
	ASSERT_NO_THROW(Tags.ClearTags());
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged6.aiff",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)694834,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("608bd0c668528fc0a143a4d3b8dac640"),ppl6::CFile::MD5("tmp/test_tagged6.aiff"));

}

TEST_F(CID3TagTest, AiffNoTagsAndNoChange) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_44kHz.aiff","tmp/test_tagged7.aiff"));
	ASSERT_EQ(0,Tags.Load("tmp/test_tagged7.aiff"));
	ASSERT_NO_THROW(Tags.ClearTags());
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged7.aiff",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)694834,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("608bd0c668528fc0a143a4d3b8dac640"),ppl6::CFile::MD5("tmp/test_tagged7.aiff"));

}

TEST_F(CID3TagTest, AiffRetagWithoutChanges) {
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/test_44kHz_tagged.aiff","tmp/test_tagged8.aiff"));
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged8.aiff"));
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged8.aiff",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)695866,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl6::CString("ddc103beb0e1687dd6631e31a4a06a62"),ppl6::CFile::MD5("tmp/test_tagged8.aiff"));
}

TEST_F(CID3TagTest, AiffRetagRealFile) {

	ppl6::CString TestFile="/home/patrick/svn/ppl6-tryout/Traktor4TagReader/181-Armin van Buuren - EIFORYA (Patrick F. Intro Edit).aiff";
	if (!ppl6::CFile::Exists(TestFile)) return;
	ASSERT_EQ(1,ppl6::CFile::CopyFile(TestFile,"tmp/test_tagged9.aiff"));
	ppl6::CID3Tag Tags;
	ASSERT_EQ(1,Tags.Load("tmp/test_tagged9.aiff"));

	EXPECT_EQ(1,Tags.SetArtist("Armin van Buuren"));
	EXPECT_EQ(1,Tags.SetTitle("EIFORYA (Patrick F. Intro Edit"));
	EXPECT_EQ(1,Tags.SetGenre("Trance"));
	EXPECT_EQ(1,Tags.SetRemixer("Patrick F."));
	EXPECT_EQ(1,Tags.SetLabel("Mainstage"));
	EXPECT_EQ(1,Tags.SetComment("created with MixedInKey Mashup"));
	EXPECT_EQ(1,Tags.SetYear("2014"));
	EXPECT_EQ(1,Tags.SetAlbum(""));
	EXPECT_EQ(1,Tags.SetTrack(""));
	EXPECT_EQ(1,Tags.SetBPM("132"));
	EXPECT_EQ(1,Tags.SetKey("em"));
	EXPECT_EQ(1,Tags.SetEnergyLevel("7"));
	ASSERT_EQ(1,Tags.Save()) << "Saving taggs failed";

	//Tags.ListFrames(1);
	//return;

	printf ("\n\n******************************************************************\n");
	ppl6::CID3Tag Tags2;
	ASSERT_EQ(1,Tags2.Load("tmp/test_tagged9.aiff")) << ppl6::Error2String();
	ppl6::CBinary cover;
	cover.Load("/home/patrick/svn/ppl6-tryout/Traktor4TagReader/eiforya.jpg");
	EXPECT_EQ(1,Tags2.SetPicture(3,cover,"image/jpeg"));
	ASSERT_EQ(1,Tags2.Save()) << "Saving taggs failed";

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/test_tagged9.aiff",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)44318155,d.Size) << "Tagged File has unexpected size";
	//ASSERT_EQ(ppl6::CString("ddc103beb0e1687dd6631e31a4a06a62"),ppl6::CFile::MD5("tmp/test_tagged9.aiff"));
}



}	// EOF namespace

