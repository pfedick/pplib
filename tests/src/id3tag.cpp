/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (ppl7).
 * Web: http://www.pfp.de/ppl/
 *
 *******************************************************************************
 * Copyright (c) 2015, Patrick Fedick <patrick@pfp.de>
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
#include <pthread.h>
#include <locale.h>
#include "../include/ppl7.h"
#include "../include/ppl7-audio.h"
#include <gtest/gtest.h>
#include "ppl7-tests.h"


namespace ppl7 {

class ID3FrameTest : public ::testing::Test {
	protected:
		ID3FrameTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~ID3FrameTest() {

	}
};

class ID3TagTest : public ::testing::Test {
	protected:
	ID3TagTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::Dir::mkDir("tmp");
	}
	virtual ~ID3TagTest() {

	}
};

TEST_F(ID3FrameTest, ConstructorWithoutName) {
	ASSERT_NO_THROW({
		ppl7::ID3Frame Frame;
		EXPECT_EQ(ppl7::String(),Frame.name());
		EXPECT_EQ((int)0,Frame.flags());
		EXPECT_EQ((size_t)0,Frame.size());
		EXPECT_FALSE(Frame.hasData());
	});
}

TEST_F(ID3FrameTest, ConstructorWithName) {
	ASSERT_NO_THROW({
		ppl7::ID3Frame Frame("TITL");
		EXPECT_EQ(ppl7::String("TITL"),Frame.name());
		EXPECT_EQ((int)0,Frame.flags());
		EXPECT_EQ((size_t)0,Frame.size());
		EXPECT_FALSE(Frame.hasData());
	});
}

TEST_F(ID3FrameTest, setFlagsGetFlags) {
	ppl7::ID3Frame Frame;

	ASSERT_NO_THROW({
		Frame.setFlags(42);
	});
	EXPECT_EQ((int)42,Frame.flags());
}

TEST_F(ID3FrameTest, setDataGetData) {
	ppl7::ID3Frame Frame;
	ppl7::ByteArray cover;
	ppl7::File::load(cover,"testdata/cover.jpg");

	ASSERT_NO_THROW({
		Frame.setData(cover);
	});
	EXPECT_TRUE(Frame.hasData());
	EXPECT_EQ((size_t)28402,Frame.size());
	ppl7::ByteArray newCover;
	ASSERT_NO_THROW({
		Frame.getData(newCover);
	});
	EXPECT_TRUE(cover == newCover);
	ppl7::ByteArray cover2;
	ASSERT_NO_THROW({
		Frame.setData(cover2);
	});
	EXPECT_FALSE(Frame.hasData());

}


TEST_F(ID3TagTest, ConstructorWithoutFile) {
	ASSERT_NO_THROW({
		ppl7::ID3Tag Tags;
		EXPECT_EQ((size_t)0,Tags.frameCount());
	});
}

TEST_F(ID3TagTest, addFindDeleteOneFrame) {
	ppl7::ID3Tag Tags;
	ppl7::ID3Frame *frame=new ppl7::ID3Frame("TIT2");
	Tags.addFrame(frame);
	EXPECT_EQ((size_t)1,Tags.frameCount());
	ppl7::ID3Frame *frame_found=Tags.findFrame("TIT2");
	EXPECT_EQ(frame,frame_found);
	Tags.deleteFrame(frame_found);
	EXPECT_EQ((size_t)0,Tags.frameCount());
}

TEST_F(ID3TagTest, clear) {
	ppl7::ID3Tag Tags;
	Tags.addFrame(new ppl7::ID3Frame("TIT1"));
	Tags.addFrame(new ppl7::ID3Frame("TIT2"));
	Tags.addFrame(new ppl7::ID3Frame("TIT3"));
	EXPECT_EQ((size_t)3,Tags.frameCount());
	Tags.clear();
	EXPECT_EQ((size_t)0,Tags.frameCount());
}

TEST_F(ID3TagTest, Mp3LoadFileWithoutTags) {
	ppl7::ID3Tag Tags;
	ASSERT_NO_THROW({
		Tags.load("testdata/test_192cbr.mp3");
	});
	EXPECT_EQ((size_t)0,Tags.frameCount());
}

TEST_F(ID3TagTest, copyAndDecodeTextWithoutEncodingByte) {
	ppl7::ID3Frame frame("TIT1");
	ppl7::ID3Tag Tag;
	ppl7::String result;
	ppl7::ByteArray ba;
	ppl7::String expected("Hällo Wörld");
	ba=expected.toEncoding("ISO-8859-1");
	frame.setData(ba);
	try {
		ID3TagTranscode::copyAndDecodeText(result,&frame,0);
	} catch (const ppl7::Exception &e) {
		e.print();
	}
	EXPECT_EQ(expected,result);
}

TEST_F(ID3TagTest, copyAndDecodeTextWithEncodingByte0) {	// ISO-88591
	ppl7::ID3Frame frame("TIT1");
	ppl7::ID3Tag Tag;
	ppl7::String result;
	ppl7::ByteArray ba;
	ppl7::String expected("Hällo Wörld");
	ba=expected.toEncoding("ISO-8859-1");
	char prefix[1]={0};
	ba.prepend(prefix,1);
	ba.append(prefix,1);
	//ba.hexDump();
	frame.setData(ba);
	try {
		ID3TagTranscode::copyAndDecodeText(result,&frame,0);
	} catch (const ppl7::Exception &e) {
		e.print();
	}
	EXPECT_EQ(expected,result);
}

TEST_F(ID3TagTest, copyAndDecodeTextWithEncodingByte1) {		// UTF-16 mit BOM
	ppl7::ID3Frame frame("TIT1");
	ppl7::ID3Tag Tag;
	ppl7::String result;
	ppl7::ByteArray ba;
	ppl7::String expected("Hällo Wörld");
	ba=expected.toEncoding("UTF-16");
	char prefix[1]={1};
	ba.prepend(prefix,1);
	char suffix[4]={0,0,0,0};
	ba.append(suffix,2);
	frame.setData(ba);
	try {
		ID3TagTranscode::copyAndDecodeText(result,&frame,0);
	} catch (const ppl7::Exception &e) {
		e.print();
	}
	EXPECT_EQ(expected,result);
}

TEST_F(ID3TagTest, copyAndDecodeTextWithEncodingByte2) {		// UTF-16BE ohne BOM
	ppl7::ID3Frame frame("TIT1");
	ppl7::ID3Tag Tag;
	ppl7::String result;
	ppl7::ByteArray ba;
	ppl7::String expected("Hällo Wörld");
	ba=expected.toEncoding("UTF-16BE");
	char prefix[1]={2};
	ba.prepend(prefix,1);
	char suffix[4]={0,0,0,0};
	ba.append(suffix,2);

	frame.setData(ba);
	try {
		ID3TagTranscode::copyAndDecodeText(result,&frame,0);
	} catch (const ppl7::Exception &e) {
		e.print();
	}
	EXPECT_EQ(expected,result);
}


TEST_F(ID3TagTest, copyAndDecodeTextWithEncodingByte3) {		// UTF-8
	ppl7::ID3Frame frame("TIT1");
	ppl7::ID3Tag Tag;
	ppl7::String result;
	ppl7::ByteArray ba;
	ppl7::String expected("Hello World");
	ba=expected.toEncoding("US-ASCII");
	char prefix[1]={3};
	ba.prepend(prefix,1);
	char suffix[4]={0,0,0,0};
	ba.append(suffix,1);

	frame.setData(ba);
	try {
		ID3TagTranscode::copyAndDecodeText(result,&frame,0);
	} catch (const ppl7::Exception &e) {
		e.print();
	}
	EXPECT_EQ(expected,result);
}

TEST_F(ID3TagTest, SetAndGetRemixer) {
	ppl7::ID3Tag Tags;
	ASSERT_NO_THROW({
		Tags.setRemixer("Single");
	});
	EXPECT_EQ((size_t)2,Tags.frameCount());
	EXPECT_EQ(ppl7::String("Single"),Tags.getRemixer());
}

TEST_F(ID3TagTest, Mp3LoadFileWithTags) {
	ppl7::ID3Tag Tags;
	ASSERT_NO_THROW({
		Tags.load("testdata/test_192cbr_taggedWithCover.mp3");
	});
	EXPECT_EQ(ppl7::String("Patrick Fedick"),Tags.getArtist());
	EXPECT_EQ(ppl7::String("Powerplay Jingle"),Tags.getTitle());
	EXPECT_EQ(ppl7::String("Trance"),Tags.getGenre());
	EXPECT_EQ(ppl7::String("Single"),Tags.getRemixer());
	EXPECT_EQ(ppl7::String("Patrick F.-Productions"),Tags.getLabel());
	EXPECT_EQ(ppl7::String("PPL Testdata"),Tags.getComment());
	EXPECT_EQ(ppl7::String("2013"),Tags.getYear());
	EXPECT_EQ(ppl7::String("PPL Testsuite"),Tags.getAlbum());
	EXPECT_EQ(ppl7::String("2"),Tags.getTrack());
	EXPECT_EQ(ppl7::String("138"),Tags.getBPM());
	EXPECT_EQ(ppl7::String("am"),Tags.getKey());
	EXPECT_EQ(ppl7::String("9"),Tags.getEnergyLevel());
	ppl7::ByteArray cover;
	EXPECT_EQ(true,Tags.getPicture(3,cover));
	EXPECT_EQ((size_t)28402,cover.size()) << "Embedded Cover has unexpected size";
	EXPECT_EQ(ppl7::String("d665f69f04f1413eef91b3596de8dfb6"),cover.md5()) << "Embedded Cover has unexpected MD5 hash";
}

TEST_F(ID3TagTest, Mp3InitialTaggingWithoutPicture) {
	ppl7::ID3Tag Tags;
	ASSERT_NO_THROW({
		ppl7::File::copy("testdata/test_192cbr.mp3","tmp/test_tagged1.mp3");
	});
	ASSERT_NO_THROW({
		Tags.load("tmp/test_tagged1.mp3");
	});
	EXPECT_EQ((size_t)0,Tags.frameCount());
	ASSERT_NO_THROW({
		Tags.setArtist("Patrick Fedick");
		Tags.setTitle("Powerplay Jingle");
		Tags.setGenre("Trance");
		Tags.setRemixer("Single");
		Tags.setLabel("Patrick F.-Productions");
		Tags.setComment("PPL Testdata");
		Tags.setYear("2013");
		Tags.setAlbum("PPL Testsuite");
		Tags.setTrack("2");
		Tags.setBPM("138");
		Tags.setKey("am");
		Tags.setEnergyLevel("9");
	});
	EXPECT_EQ((size_t)13,Tags.frameCount());
	ASSERT_NO_THROW({
		Tags.save();
	});
	// Save again
	ASSERT_NO_THROW({
		Tags.save();
	});

	ppl7::DirEntry d;
	ASSERT_NO_THROW({
		ppl7::File::stat("tmp/test_tagged1.mp3",d);
	});
	ASSERT_EQ((size_t)97072,d.Size) << "Tagged File has unexpected size";
	EXPECT_EQ(ppl7::String("f9a333ac0f6ee3c92fae02390b25248f"),ppl7::File::md5Hash("tmp/test_tagged1.mp3"));
	ppl7::ID3Tag NewTags;
	ASSERT_NO_THROW({
		NewTags.load("tmp/test_tagged1.mp3");
	});
	EXPECT_EQ(ppl7::String("Patrick Fedick"),NewTags.getArtist());
	EXPECT_EQ(ppl7::String("Powerplay Jingle"),NewTags.getTitle());
	EXPECT_EQ(ppl7::String("Trance"),NewTags.getGenre());
	EXPECT_EQ(ppl7::String("Single"),NewTags.getRemixer());
	EXPECT_EQ(ppl7::String("Patrick F.-Productions"),NewTags.getLabel());
	EXPECT_EQ(ppl7::String("PPL Testdata"),NewTags.getComment());
	EXPECT_EQ(ppl7::String("2013"),NewTags.getYear());
	EXPECT_EQ(ppl7::String("PPL Testsuite"),NewTags.getAlbum());
	EXPECT_EQ(ppl7::String("2"),NewTags.getTrack());
	EXPECT_EQ(ppl7::String("138"),NewTags.getBPM());
	EXPECT_EQ(ppl7::String("am"),NewTags.getKey());
	EXPECT_EQ(ppl7::String("9"),NewTags.getEnergyLevel());
	ppl7::ByteArray cover;
	EXPECT_EQ(false,Tags.getPicture(3,cover));
}


TEST_F(ID3TagTest, Mp3InitialTaggingWithPicture) {
	ppl7::ID3Tag Tags;
	ppl7::ByteArray cover;
	ppl7::File::load(cover,"testdata/cover.jpg");
	ppl7::File::copy("testdata/test_192cbr.mp3","tmp/test_tagged2.mp3");
	Tags.load("tmp/test_tagged2.mp3");
	EXPECT_EQ((size_t)0,Tags.frameCount());
	ASSERT_NO_THROW({
		Tags.setArtist("Patrick Fedick");
		Tags.setTitle("Powerplay Jingle");
		Tags.setGenre("Trance");
		Tags.setRemixer("Single");
		Tags.setLabel("Patrick F.-Productions");
		Tags.setComment("PPL Testdata");
		Tags.setYear("2013");
		Tags.setAlbum("PPL Testsuite");
		Tags.setTrack("2");
		Tags.setBPM("138");
		Tags.setKey("am");
		Tags.setEnergyLevel("9");
		Tags.setPicture(3,cover,"image/jpeg");
	});
	ASSERT_NO_THROW({
		Tags.save();
	});
	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged2.mp3",d);
	ASSERT_EQ((size_t)125073,d.Size) << "Tagged File has unexpected size";
	EXPECT_EQ(ppl7::String("6dc503f9722059d5a0de7171e07c68e4"),ppl7::File::md5Hash("tmp/test_tagged2.mp3"));

	ppl7::ID3Tag NewTags;
	ASSERT_NO_THROW({
		NewTags.load("tmp/test_tagged2.mp3");
	});
	EXPECT_EQ(ppl7::String("Patrick Fedick"),NewTags.getArtist());
	EXPECT_EQ(ppl7::String("Powerplay Jingle"),NewTags.getTitle());
	EXPECT_EQ(ppl7::String("Trance"),NewTags.getGenre());
	EXPECT_EQ(ppl7::String("Single"),NewTags.getRemixer());
	EXPECT_EQ(ppl7::String("Patrick F.-Productions"),NewTags.getLabel());
	EXPECT_EQ(ppl7::String("PPL Testdata"),NewTags.getComment());
	EXPECT_EQ(ppl7::String("2013"),NewTags.getYear());
	EXPECT_EQ(ppl7::String("PPL Testsuite"),NewTags.getAlbum());
	EXPECT_EQ(ppl7::String("2"),NewTags.getTrack());
	EXPECT_EQ(ppl7::String("138"),NewTags.getBPM());
	EXPECT_EQ(ppl7::String("am"),NewTags.getKey());
	EXPECT_EQ(ppl7::String("9"),NewTags.getEnergyLevel());
	EXPECT_EQ(true,NewTags.getPicture(3,cover));
	EXPECT_EQ((size_t)28402,cover.size()) << "Embedded Cover has unexpected size";
	EXPECT_EQ(ppl7::String("d665f69f04f1413eef91b3596de8dfb6"),cover.md5()) << "Embedded Cover has unexpected MD5 hash";
}

TEST_F(ID3TagTest, Mp3RetagWithPicture) {
	ppl7::ID3Tag Tags;
	ppl7::File::copy("testdata/test_192cbr_tagged.mp3","tmp/test_tagged3.mp3");
	Tags.load("tmp/test_tagged3.mp3");

	ppl7::ByteArray cover;
	ppl7::File::load(cover,"testdata/cover.jpg");
	Tags.setPicture(3,cover,"image/jpeg");
	Tags.save();
	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged3.mp3",d);
	ASSERT_EQ((size_t)125074,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl7::String("052fd75215f383718e1368b535520c60"),ppl7::File::md5Hash("tmp/test_tagged3.mp3"));
}

TEST_F(ID3TagTest, FindUserDefinedText) {
	ppl7::ID3Tag Tags;
	Tags.load("testdata/test_192cbr_tagged.mp3");
	ID3Frame *frame=Tags.findUserDefinedText("TraktorRemixer");
	ASSERT_TRUE(frame!=NULL);
	frame=Tags.findUserDefinedText("Gibtsnicht");
	ASSERT_TRUE(frame==NULL);
}

TEST_F(ID3TagTest, Mp3RetagStrings) {
	ppl7::ID3Tag Tags;
	ppl7::File::copy("testdata/test_192cbr_tagged.mp3","tmp/test_tagged4.mp3");
	Tags.load("tmp/test_tagged4.mp3");

	Tags.setArtist("New Artist");
	Tags.setTitle("New Jingle");
	Tags.setGenre("Techno");
	Tags.setRemixer("Maxi");
	Tags.setLabel("New Label");
	Tags.setComment("New Testdata");
	Tags.setYear("2014");
	Tags.setAlbum("New Testsuite");
	Tags.setTrack("3");
	Tags.setBPM("140");
	Tags.setKey("em");
	Tags.setEnergyLevel("7");
	Tags.save();

	ppl7::ID3Tag NewTags;
	NewTags.load("tmp/test_tagged4.mp3");
	EXPECT_EQ(ppl7::String("New Artist"),NewTags.getArtist());
	EXPECT_EQ(ppl7::String("New Jingle"),NewTags.getTitle());
	EXPECT_EQ(ppl7::String("Techno"),NewTags.getGenre());
	EXPECT_EQ(ppl7::String("Maxi"),NewTags.getRemixer());
	EXPECT_EQ(ppl7::String("New Label"),NewTags.getLabel());
	EXPECT_EQ(ppl7::String("New Testdata"),NewTags.getComment());
	EXPECT_EQ(ppl7::String("2014"),NewTags.getYear());
	EXPECT_EQ(ppl7::String("New Testsuite"),NewTags.getAlbum());
	EXPECT_EQ(ppl7::String("3"),NewTags.getTrack());
	EXPECT_EQ(ppl7::String("140"),NewTags.getBPM());
	EXPECT_EQ(ppl7::String("em"),NewTags.getKey());
	EXPECT_EQ(ppl7::String("7"),NewTags.getEnergyLevel());

	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged4.mp3",d);
	ASSERT_EQ((size_t)97072,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl7::String("ffdf4dc172e7c5cc07b543557c33b6d1"),ppl7::File::md5Hash("tmp/test_tagged4.mp3"));
}


TEST_F(ID3TagTest, Mp3RemovePicture) {
	ppl7::ID3Tag Tags;
	ppl7::File::copy("testdata/test_192cbr_taggedWithCover.mp3","tmp/test_tagged5.mp3");
	Tags.load("tmp/test_tagged5.mp3");
	ASSERT_TRUE(Tags.hasPicture(3));
	ASSERT_NO_THROW(Tags.removePicture(3));
	ASSERT_NO_THROW(Tags.save());

	ppl7::ID3Tag NewTags;
	NewTags.load("tmp/test_tagged5.mp3");
	ASSERT_FALSE(Tags.hasPicture(3));

	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged5.mp3",d);
	ASSERT_EQ((size_t)97073,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl7::String("e2dacaeff7f3dbc0d54ed63e88ba519d"),ppl7::File::md5Hash("tmp/test_tagged5.mp3"));
}


TEST_F(ID3TagTest, Mp3RemoveAllTags) {
	ppl7::ID3Tag Tags;
	ppl7::File::copy("testdata/test_192cbr_taggedWithCover.mp3","tmp/test_tagged6.mp3");
	Tags.load("tmp/test_tagged6.mp3");
	ASSERT_NO_THROW(Tags.clearTags());
	Tags.save();
	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged6.mp3",d);
	ASSERT_EQ((size_t)95921,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl7::String("692bf339243cee92f1c639b10ffde45e"),ppl7::File::md5Hash("tmp/test_tagged6.mp3"));

}

TEST_F(ID3TagTest, Mp3NoTagsAndNoChange) {
	ppl7::ID3Tag Tags;
	ppl7::File::copy("testdata/test_192cbr.mp3","tmp/test_tagged7.mp3");
	Tags.load("tmp/test_tagged7.mp3");
	ASSERT_NO_THROW(Tags.clearTags());
	ASSERT_NO_THROW(Tags.save());
	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged7.mp3",d);
	ASSERT_EQ((size_t)95920,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl7::String("0abbdd3ce267358a0b3bf3f0a015e74e"),ppl7::File::md5Hash("tmp/test_tagged7.mp3"));
}

TEST_F(ID3TagTest, Mp3RetagWithoutChanges) {
	ppl7::ID3Tag Tags;
	ppl7::File::copy("testdata/test_192cbr_tagged.mp3","tmp/test_tagged8.mp3");
	Tags.load("tmp/test_tagged8.mp3");
	Tags.save();
	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged8.mp3",d);
	ASSERT_EQ((size_t)97072,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl7::String("c5ff756219cba391c99423ddd6cca625"),ppl7::File::md5Hash("tmp/test_tagged8.mp3"));
}

TEST_F(ID3TagTest, AiffLoadFileWithoutTags) {
	ppl7::ID3Tag Tags;
	ASSERT_NO_THROW({
		Tags.load("testdata/test_44kHz.aiff");
	});
	EXPECT_EQ((size_t)0,Tags.frameCount()) << "Unexpected number of ID3-Frames";
}

TEST_F(ID3TagTest, AiffLoadFileWithTags) {
	ppl7::ID3Tag Tags;
	ASSERT_NO_THROW({
		Tags.load("testdata/test_44kHz_taggedWithCover.aiff");
	});
	EXPECT_EQ(ppl7::String("Patrick Fedick"),Tags.getArtist());
	EXPECT_EQ(ppl7::String("Powerplay Jingle"),Tags.getTitle());
	EXPECT_EQ(ppl7::String("Trance"),Tags.getGenre());
	EXPECT_EQ(ppl7::String("Single"),Tags.getRemixer());
	EXPECT_EQ(ppl7::String("Patrick F.-Productions"),Tags.getLabel());
	EXPECT_EQ(ppl7::String("PPL Testdata"),Tags.getComment());
	EXPECT_EQ(ppl7::String("2013"),Tags.getYear());
	EXPECT_EQ(ppl7::String("PPL Testsuite"),Tags.getAlbum());
	EXPECT_EQ(ppl7::String("2"),Tags.getTrack());
	EXPECT_EQ(ppl7::String("138"),Tags.getBPM());
	EXPECT_EQ(ppl7::String("am"),Tags.getKey());
	EXPECT_EQ(ppl7::String("9"),Tags.getEnergyLevel());
	ppl7::ByteArray cover;
	EXPECT_EQ(true,Tags.getPicture(3,cover));
	EXPECT_EQ((size_t)28402,cover.size()) << "Embedded Cover has unexpected size";
	EXPECT_EQ(ppl7::String("d665f69f04f1413eef91b3596de8dfb6"),cover.md5()) << "Embedded Cover has unexpected MD5 hash";
}

TEST_F(ID3TagTest, AiffInitialTaggingWithoutPicture) {
	ppl7::ID3Tag Tags;
	ppl7::File::copy("testdata/test_44kHz.aiff","tmp/test_tagged1.aiff");
	ASSERT_NO_THROW({
		Tags.load("tmp/test_tagged1.aiff");
	});
	EXPECT_EQ((size_t)0,Tags.frameCount()) << "Unexpected number of ID3-Frames";

	Tags.setArtist("Patrick Fedick");
	Tags.setTitle("Powerplay Jingle");
	Tags.setGenre("Trance");
	Tags.setRemixer("Single");
	Tags.setLabel("Patrick F.-Productions");
	Tags.setComment("PPL Testdata");
	Tags.setYear("2013");
	Tags.setAlbum("PPL Testsuite");
	Tags.setTrack("2");
	Tags.setBPM("138");
	Tags.setKey("am");
	Tags.setEnergyLevel("9");
	ASSERT_NO_THROW({
		try {
			Tags.save();
		} catch (const ppl7::Exception &e) {
			e.print();
			throw;
		}
	});

	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged1.aiff",d);
	EXPECT_EQ((size_t)695866,d.Size) << "Tagged File has unexpected size";
	EXPECT_EQ(ppl7::String("ee7fa3d57fd26f6a45e1f9448fd8c09c"),ppl7::File::md5Hash("tmp/test_tagged1.aiff"));

	ppl7::ID3Tag NewTags;
	ppl7::ByteArray cover;
	ASSERT_NO_THROW({
		NewTags.load("tmp/test_tagged1.aiff");
	});

	EXPECT_EQ(ppl7::String("Patrick Fedick"),NewTags.getArtist());
	EXPECT_EQ(ppl7::String("Powerplay Jingle"),NewTags.getTitle());
	EXPECT_EQ(ppl7::String("Trance"),NewTags.getGenre());
	EXPECT_EQ(ppl7::String("Single"),NewTags.getRemixer());
	EXPECT_EQ(ppl7::String("Patrick F.-Productions"),NewTags.getLabel());
	EXPECT_EQ(ppl7::String("PPL Testdata"),NewTags.getComment());
	EXPECT_EQ(ppl7::String("2013"),NewTags.getYear());
	EXPECT_EQ(ppl7::String("PPL Testsuite"),NewTags.getAlbum());
	EXPECT_EQ(ppl7::String("2"),NewTags.getTrack());
	EXPECT_EQ(ppl7::String("138"),NewTags.getBPM());
	EXPECT_EQ(ppl7::String("am"),NewTags.getKey());
	EXPECT_EQ(ppl7::String("9"),NewTags.getEnergyLevel());
	EXPECT_EQ(false,NewTags.getPicture(3,cover));
}

TEST_F(ID3TagTest, AiffInitialTaggingWithPicture) {
	ppl7::ID3Tag Tags;
	ppl7::ByteArray cover;
	ppl7::File::load(cover,"testdata/cover.jpg");
	ppl7::File::copy("testdata/test_44kHz.aiff","tmp/test_tagged2.aiff");
	Tags.load("tmp/test_tagged2.aiff");
	EXPECT_EQ((size_t)0,Tags.frameCount());
	ASSERT_NO_THROW({
		Tags.setArtist("Patrick Fedick");
		Tags.setTitle("Powerplay Jingle");
		Tags.setGenre("Trance");
		Tags.setRemixer("Single");
		Tags.setLabel("Patrick F.-Productions");
		Tags.setComment("PPL Testdata");
		Tags.setYear("2013");
		Tags.setAlbum("PPL Testsuite");
		Tags.setTrack("2");
		Tags.setBPM("138");
		Tags.setKey("am");
		Tags.setEnergyLevel("9");
		Tags.setPicture(3,cover,"image/jpeg");
	});
	ASSERT_NO_THROW({
		Tags.save();
	});
	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged2.aiff",d);
	ASSERT_EQ((size_t)723867,d.Size) << "Tagged File has unexpected size";
	EXPECT_EQ(ppl7::String("8b48f5ce239455880fa35f0a83ae30e5"),ppl7::File::md5Hash("tmp/test_tagged2.aiff"));

	ppl7::ID3Tag NewTags;
	ASSERT_NO_THROW({
		NewTags.load("tmp/test_tagged2.aiff");
	});
	EXPECT_EQ(ppl7::String("Patrick Fedick"),NewTags.getArtist());
	EXPECT_EQ(ppl7::String("Powerplay Jingle"),NewTags.getTitle());
	EXPECT_EQ(ppl7::String("Trance"),NewTags.getGenre());
	EXPECT_EQ(ppl7::String("Single"),NewTags.getRemixer());
	EXPECT_EQ(ppl7::String("Patrick F.-Productions"),NewTags.getLabel());
	EXPECT_EQ(ppl7::String("PPL Testdata"),NewTags.getComment());
	EXPECT_EQ(ppl7::String("2013"),NewTags.getYear());
	EXPECT_EQ(ppl7::String("PPL Testsuite"),NewTags.getAlbum());
	EXPECT_EQ(ppl7::String("2"),NewTags.getTrack());
	EXPECT_EQ(ppl7::String("138"),NewTags.getBPM());
	EXPECT_EQ(ppl7::String("am"),NewTags.getKey());
	EXPECT_EQ(ppl7::String("9"),NewTags.getEnergyLevel());
	EXPECT_EQ(true,NewTags.getPicture(3,cover));
	EXPECT_EQ((size_t)28402,cover.size()) << "Embedded Cover has unexpected size";
	EXPECT_EQ(ppl7::String("d665f69f04f1413eef91b3596de8dfb6"),cover.md5()) << "Embedded Cover has unexpected MD5 hash";
}

TEST_F(ID3TagTest, AiffRetagWithPicture) {
	ppl7::ID3Tag Tags;
	ppl7::File::copy("testdata/test_44kHz_tagged.aiff","tmp/test_tagged3.aiff");
	Tags.load("tmp/test_tagged3.aiff");
	ppl7::ByteArray cover;
	ppl7::File::load(cover,"testdata/cover.jpg");
	Tags.setPicture(3,cover,"image/jpeg");
	Tags.save();
	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged3.aiff",d);
	ASSERT_EQ((size_t)723867,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl7::String("bc9d24e8dffffda4cd7208122e83e65f"),ppl7::File::md5Hash("tmp/test_tagged3.aiff"));
}

TEST_F(ID3TagTest, AiffRetagStrings) {
	ppl7::ID3Tag Tags;
	ppl7::File::copy("testdata/test_44kHz_tagged.aiff","tmp/test_tagged4.aiff");
	Tags.load("tmp/test_tagged4.aiff");
	Tags.setArtist("New Artist");
	Tags.setTitle("New Jingle");
	Tags.setGenre("Techno");
	Tags.setRemixer("Maxi");
	Tags.setLabel("New Label");
	Tags.setComment("New Testdata");
	Tags.setYear("2014");
	Tags.setAlbum("New Testsuite");
	Tags.setTrack("3");
	Tags.setBPM("140");
	Tags.setKey("em");
	Tags.setEnergyLevel("7");
	Tags.save();

	ppl7::ID3Tag NewTags;
	NewTags.load("tmp/test_tagged4.aiff");
	EXPECT_EQ(ppl7::String("New Artist"),NewTags.getArtist());
	EXPECT_EQ(ppl7::String("New Jingle"),NewTags.getTitle());
	EXPECT_EQ(ppl7::String("Techno"),NewTags.getGenre());
	EXPECT_EQ(ppl7::String("Maxi"),NewTags.getRemixer());
	EXPECT_EQ(ppl7::String("New Label"),NewTags.getLabel());
	EXPECT_EQ(ppl7::String("New Testdata"),NewTags.getComment());
	EXPECT_EQ(ppl7::String("2014"),NewTags.getYear());
	EXPECT_EQ(ppl7::String("New Testsuite"),NewTags.getAlbum());
	EXPECT_EQ(ppl7::String("3"),NewTags.getTrack());
	EXPECT_EQ(ppl7::String("140"),NewTags.getBPM());
	EXPECT_EQ(ppl7::String("em"),NewTags.getKey());
	EXPECT_EQ(ppl7::String("7"),NewTags.getEnergyLevel());

	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged4.aiff",d);
	ASSERT_EQ((size_t)695866,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl7::String("a254ec79ef0169d2fd6ad70f98a4c193"),ppl7::File::md5Hash("tmp/test_tagged4.aiff"));
}

TEST_F(ID3TagTest, AiffRetagStringsAndRetagWithCoverAgain) {
	ppl7::ID3Tag Tags;
	ppl7::File::copy("testdata/test_44kHz_tagged.aiff","tmp/test_tagged10.aiff");
	Tags.load("tmp/test_tagged10.aiff");


	Tags.setArtist("Old Artist");
	Tags.setTitle("Old Jingle");
	Tags.setGenre("OldTechno");
	Tags.setRemixer("Old Maxi");
	Tags.setLabel("Old Label");
	Tags.setComment("Old Testdata");
	Tags.setYear("2013");
	Tags.setAlbum("Old Testsuite");
	Tags.setTrack("2");
	Tags.setBPM("120");
	Tags.setKey("am");
	Tags.setEnergyLevel("6");
	Tags.save();

	ppl7::ID3Tag Tags2;
	Tags2.load("tmp/test_tagged10.aiff");
	Tags2.setArtist("New Artist");
	Tags2.setTitle("New Jingle");
	Tags2.setGenre("Techno");
	Tags2.setRemixer("Maxi");
	Tags2.setLabel("New Label");
	Tags2.setComment("New Testdata");
	Tags2.setYear("2014");
	Tags2.setAlbum("New Testsuite");
	Tags2.setTrack("3");
	Tags2.setBPM("140");
	Tags2.setKey("em");
	Tags2.setEnergyLevel("7");
	ppl7::ByteArray cover;
	ppl7::File::load(cover,"testdata/cover.jpg");
	Tags2.setPicture(3,cover,"image/jpeg");
	Tags2.save();

	ppl7::ID3Tag NewTags;
	NewTags.load("tmp/test_tagged10.aiff");
	EXPECT_EQ(ppl7::String("New Artist"),NewTags.getArtist());
	EXPECT_EQ(ppl7::String("New Jingle"),NewTags.getTitle());
	EXPECT_EQ(ppl7::String("Techno"),NewTags.getGenre());
	EXPECT_EQ(ppl7::String("Maxi"),NewTags.getRemixer());
	EXPECT_EQ(ppl7::String("New Label"),NewTags.getLabel());
	EXPECT_EQ(ppl7::String("New Testdata"),NewTags.getComment());
	EXPECT_EQ(ppl7::String("2014"),NewTags.getYear());
	EXPECT_EQ(ppl7::String("New Testsuite"),NewTags.getAlbum());
	EXPECT_EQ(ppl7::String("3"),NewTags.getTrack());
	EXPECT_EQ(ppl7::String("140"),NewTags.getBPM());
	EXPECT_EQ(ppl7::String("em"),NewTags.getKey());
	EXPECT_EQ(ppl7::String("7"),NewTags.getEnergyLevel());

	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged10.aiff",d);
	ASSERT_EQ((size_t)723813,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl7::String("01a1ae462ad55207e4b719d1eb83801c"),ppl7::File::md5Hash("tmp/test_tagged10.aiff"));
}


TEST_F(ID3TagTest, AiffRemovePicture) {
	ppl7::ID3Tag Tags;
	ppl7::File::copy("testdata/test_44kHz_taggedWithCover.aiff","tmp/test_tagged5.aiff");
	Tags.load("tmp/test_tagged5.aiff");
	ASSERT_TRUE(Tags.hasPicture(3));
	ASSERT_NO_THROW(Tags.removePicture(3));
	ASSERT_NO_THROW(Tags.save());

	ppl7::ID3Tag NewTags;
	NewTags.load("tmp/test_tagged5.aiff");
	ASSERT_FALSE(Tags.hasPicture(3));

	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged5.aiff",d);
	ASSERT_EQ((size_t)695866,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl7::String("6d357b3f28e47d0cdc3cff6c4133b339"),ppl7::File::md5Hash("tmp/test_tagged5.aiff"));
}

TEST_F(ID3TagTest, AiffRemoveAllTags) {
	ppl7::ID3Tag Tags;
	ppl7::File::copy("testdata/test_44kHz_taggedWithCover.aiff","tmp/test_tagged6.aiff");
	Tags.load("tmp/test_tagged6.aiff");
	ASSERT_NO_THROW(Tags.clearTags());
	Tags.save();
	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged6.aiff",d);
	ASSERT_EQ((size_t)694834,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl7::String("608bd0c668528fc0a143a4d3b8dac640"),ppl7::File::md5Hash("tmp/test_tagged6.aiff"));
}


TEST_F(ID3TagTest, AiffNoTagsAndNoChange) {
	ppl7::ID3Tag Tags;
	ppl7::File::copy("testdata/test_44kHz.aiff","tmp/test_tagged7.aiff");
	Tags.load("tmp/test_tagged7.aiff");
	ASSERT_NO_THROW(Tags.clearTags());
	ASSERT_NO_THROW(Tags.save());
	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged7.aiff",d);
	ASSERT_EQ((size_t)694834,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl7::String("608bd0c668528fc0a143a4d3b8dac640"),ppl7::File::md5Hash("tmp/test_tagged7.aiff"));
}

TEST_F(ID3TagTest, AiffRetagWithoutChanges) {
	ppl7::ID3Tag Tags;
	ppl7::File::copy("testdata/test_44kHz_tagged.aiff","tmp/test_tagged8.aiff");
	Tags.load("tmp/test_tagged8.aiff");
	Tags.save();
	ppl7::DirEntry d;
	ppl7::File::stat("tmp/test_tagged8.aiff",d);
	ASSERT_EQ((size_t)695866,d.Size) << "Tagged File has unexpected size";
	ASSERT_EQ(ppl7::String("ddc103beb0e1687dd6631e31a4a06a62"),ppl7::File::md5Hash("tmp/test_tagged8.aiff"));
}


TEST_F(ID3TagTest, GetID3GenreName) {
	ASSERT_EQ(ppl7::String("Blues"),ppl7::GetID3GenreName(0));
	ASSERT_EQ(ppl7::String("SynthPop"),ppl7::GetID3GenreName(147));
	ASSERT_EQ(ppl7::String("Trance"),ppl7::GetID3GenreName(31));
	ASSERT_THROW(ppl7::GetID3GenreName(-1),ppl7::InvalidGenreException);
	ASSERT_THROW(ppl7::GetID3GenreName(148),ppl7::InvalidGenreException);
}




/*
TEST_F(ID3TagTest, AiffRetagRealFile) {

	ppl7::String TestFile="/home/patrick/svn/ppl7-tryout/Traktor4TagReader/181-Armin van Buuren - EIFORYA (Patrick F. Intro Edit).aiff";
	if (!ppl7::File::Exists(TestFile)) return;
	ASSERT_EQ(1,ppl7::File::CopyFile(TestFile,"tmp/test_tagged9.aiff"));
	ppl7::ID3Tag Tags;
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
	ppl7::ID3Tag Tags2;
	ASSERT_EQ(1,Tags2.Load("tmp/test_tagged9.aiff")) << ppl7::Error2String();
	ppl7::CBinary cover;
	cover.Load("/home/patrick/svn/ppl7-tryout/Traktor4TagReader/eiforya.jpg");
	EXPECT_EQ(1,Tags2.SetPicture(3,cover,"image/jpeg"));
	ASSERT_EQ(1,Tags2.Save()) << "Saving taggs failed";

	ppl7::CDirEntry d;
	ASSERT_EQ(1,ppl7::File::Stat("tmp/test_tagged9.aiff",d)) << "Tagged File does not exist!";
	ASSERT_EQ((size_t)44318523,d.Size) << "Tagged File has unexpected size";
	//ASSERT_EQ(ppl7::String("ddc103beb0e1687dd6631e31a4a06a62"),ppl7::File::MD5("tmp/test_tagged9.aiff"));
}
*/



}	// EOF namespace

