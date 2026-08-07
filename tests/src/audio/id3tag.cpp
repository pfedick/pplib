/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (pplib).
 * Web: http://www.pfp.de/ppl/
 *
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <pplib.h>
#include <pplib-audio.h>
#include <gtest/gtest.h>
#include "pplib-tests.h"

namespace pplib
{

class ID3FrameTest : public ::testing::Test
{
protected:
    ID3FrameTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~ID3FrameTest()
    {
    }
};

class ID3TagTest : public ::testing::Test
{
protected:
    ID3TagTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
        pplib::Dir::mkDir("tmp");
    }
    virtual ~ID3TagTest()
    {
    }
};

TEST_F(ID3FrameTest, ConstructorWithoutName)
{
    ASSERT_NO_THROW({
        pplib::ID3Frame Frame;
        EXPECT_EQ(pplib::String(), Frame.name());
        EXPECT_EQ((int)0, Frame.flags());
        EXPECT_EQ((size_t)0, Frame.size());
        EXPECT_FALSE(Frame.hasData());
    });
}

TEST_F(ID3FrameTest, ConstructorWithName)
{
    ASSERT_NO_THROW({
        pplib::ID3Frame Frame("TITL");
        EXPECT_EQ(pplib::String("TITL"), Frame.name());
        EXPECT_EQ((int)0, Frame.flags());
        EXPECT_EQ((size_t)0, Frame.size());
        EXPECT_FALSE(Frame.hasData());
    });
}

TEST_F(ID3FrameTest, setFlagsGetFlags)
{
    pplib::ID3Frame Frame;

    ASSERT_NO_THROW({ Frame.setFlags(42); });
    EXPECT_EQ((int)42, Frame.flags());
}

TEST_F(ID3FrameTest, setDataGetData)
{
    pplib::ID3Frame Frame;
    pplib::ByteArray cover;
    pplib::File::load(cover, "testdata/cover.jpg");

    ASSERT_NO_THROW({ Frame.setData(cover); });
    EXPECT_TRUE(Frame.hasData());
    EXPECT_EQ((size_t)28402, Frame.size());
    pplib::ByteArray newCover;
    ASSERT_NO_THROW({ Frame.getData(newCover); });
    EXPECT_TRUE(cover == newCover);
    pplib::ByteArray cover2;
    ASSERT_NO_THROW({ Frame.setData(cover2); });
    EXPECT_FALSE(Frame.hasData());
}

TEST_F(ID3TagTest, ConstructorWithoutFile)
{
    ASSERT_NO_THROW({
        pplib::ID3Tag Tags;
        EXPECT_EQ((size_t)0, Tags.frameCount());
    });
}

TEST_F(ID3TagTest, addFindDeleteOneFrame)
{
    pplib::ID3Tag Tags;
    pplib::ID3Frame frame("TIT2");
    Tags.addFrame(frame);
    EXPECT_EQ((size_t)1, Tags.frameCount());
    pplib::ID3Frame* frame_found = Tags.findFrame("TIT2");
    EXPECT_TRUE(frame_found != nullptr);
    Tags.removeFrame(frame_found);
    EXPECT_EQ((size_t)0, Tags.frameCount());
}

TEST_F(ID3TagTest, clear)
{
    pplib::ID3Tag Tags;
    Tags.addFrame(pplib::ID3Frame("TIT1"));
    Tags.addFrame(pplib::ID3Frame("TIT2"));
    Tags.addFrame(pplib::ID3Frame("TIT3"));
    EXPECT_EQ((size_t)3, Tags.frameCount());
    Tags.clear();
    EXPECT_EQ((size_t)0, Tags.frameCount());
}

TEST_F(ID3TagTest, Mp3LoadFileWithoutTags)
{
    pplib::ID3Tag Tags;
    ASSERT_NO_THROW({ Tags.load("testdata/test_192cbr.mp3"); });
    EXPECT_EQ((size_t)0, Tags.frameCount());
}

TEST_F(ID3TagTest, copyAndDecodeTextWithoutEncodingByte)
{
    pplib::ID3Frame frame("TIT1");
    pplib::ID3Tag Tag;
    pplib::String result;
    pplib::ByteArray ba;
    pplib::String expected("Hällo Wörld");
    ba = expected.toEncoding("ISO-8859-1");
    frame.setData(ba);
    try {
        ID3Tag::copyAndDecodeText(result, &frame, 0, "UTF-8");
    }
    catch (const pplib::Exception& e) {
        e.print();
    }
    EXPECT_EQ(expected, result);
}

TEST_F(ID3TagTest, copyAndDecodeTextWithEncodingByte0)
{ // ISO-88591
    pplib::ID3Frame frame("TIT1");
    pplib::ID3Tag Tag;
    pplib::String result;
    pplib::ByteArray ba;
    pplib::String expected(pplib::Iconv::Utf8ToLocal("Hällo Wörld"));
    ba = expected.toEncoding("ISO-8859-1");
    char prefix[1] = {0};
    ba.prepend(prefix, 1);
    ba.append(prefix, 1);
    // ba.hexDump();
    frame.setData(ba);
    try {
        ID3Tag::copyAndDecodeText(result, &frame, 0, "UTF-8");
    }
    catch (const pplib::Exception& e) {
        e.print();
    }
    EXPECT_EQ(expected, result);
}

TEST_F(ID3TagTest, copyAndDecodeTextWithEncodingByte1)
{ // UTF-16 mit BOM
    pplib::ID3Frame frame("TIT1");
    pplib::ID3Tag Tag;
    pplib::String result;
    pplib::ByteArray ba;
    pplib::String expected(pplib::Iconv::Utf8ToLocal("Hällo Wörld"));
    ba = expected.toEncoding("UTF-16");
    char prefix[1] = {1};
    ba.prepend(prefix, 1);
    char suffix[4] = {0, 0, 0, 0};
    ba.append(suffix, 2);
    frame.setData(ba);
    try {
        ID3Tag::copyAndDecodeText(result, &frame, 0, "UTF-8");
    }
    catch (const pplib::Exception& e) {
        e.print();
    }
    EXPECT_EQ(expected, result);
}

TEST_F(ID3TagTest, copyAndDecodeTextWithEncodingByte2)
{ // UTF-16BE ohne BOM
    pplib::ID3Frame frame("TIT1");
    pplib::ID3Tag Tag;
    pplib::String result;
    pplib::ByteArray ba;
    pplib::String expected(pplib::Iconv::Utf8ToLocal("Hällo Wörld"));
    ba = expected.toEncoding("UTF-16BE");
    char prefix[1] = {2};
    ba.prepend(prefix, 1);
    char suffix[4] = {0, 0, 0, 0};
    ba.append(suffix, 2);

    frame.setData(ba);
    try {
        ID3Tag::copyAndDecodeText(result, &frame, 0, "UTF-8");
    }
    catch (const pplib::Exception& e) {
        e.print();
    }
    EXPECT_EQ(expected, result);
}

TEST_F(ID3TagTest, copyAndDecodeTextWithEncodingByte3)
{ // UTF-8
    pplib::ID3Frame frame("TIT1");
    pplib::ID3Tag Tag;
    pplib::String result;
    pplib::ByteArray ba;
    pplib::String expected("Hello World");
    ba = expected.toEncoding("US-ASCII");
    char prefix[1] = {3};
    ba.prepend(prefix, 1);
    char suffix[4] = {0, 0, 0, 0};
    ba.append(suffix, 1);

    frame.setData(ba);
    try {
        ID3Tag::copyAndDecodeText(result, &frame, 0, "UTF-8");
    }
    catch (const pplib::Exception& e) {
        e.print();
    }
    EXPECT_EQ(expected, result);
}

TEST_F(ID3TagTest, SetAndGetRemixer)
{
    pplib::ID3Tag Tags;
    ASSERT_NO_THROW({ Tags.setRemixer("Single"); });
    EXPECT_EQ((size_t)2, Tags.frameCount());
    EXPECT_EQ(pplib::String("Single"), Tags.getRemixer());
}

TEST_F(ID3TagTest, Mp3LoadFileWithTags)
{
    pplib::ID3Tag Tags;
    ASSERT_NO_THROW({ Tags.load("testdata/test_192cbr_taggedWithCover.mp3"); });
    EXPECT_EQ(pplib::String("Patrick Fedick"), Tags.getArtist());
    EXPECT_EQ(pplib::String("Powerplay Jingle"), Tags.getTitle());
    EXPECT_EQ(pplib::String("Trance"), Tags.getGenre());
    EXPECT_EQ(pplib::String("Single"), Tags.getRemixer());
    EXPECT_EQ(pplib::String("Patrick F.-Productions"), Tags.getLabel());
    EXPECT_EQ(pplib::String("PPL Testdata"), Tags.getComment());
    EXPECT_EQ(pplib::String("2013"), Tags.getYear());
    EXPECT_EQ(pplib::String("PPL Testsuite"), Tags.getAlbum());
    EXPECT_EQ(pplib::String("2"), Tags.getTrack());
    EXPECT_EQ(pplib::String("138"), Tags.getBPM());
    EXPECT_EQ(pplib::String("am"), Tags.getKey());
    EXPECT_EQ(pplib::String("9"), Tags.getEnergyLevel());
    pplib::ByteArray cover;
    EXPECT_EQ(true, Tags.getPicture(3, cover));
    EXPECT_EQ((size_t)28402, cover.size()) << "Embedded Cover has unexpected size";
    EXPECT_EQ(pplib::String("d665f69f04f1413eef91b3596de8dfb6"), pplib::Md5(cover)) << "Embedded Cover has unexpected MD5 hash";
}

TEST_F(ID3TagTest, Mp3InitialTaggingWithoutPicture)
{
    pplib::ID3Tag Tags;
    ASSERT_NO_THROW({ pplib::File::copy("testdata/test_192cbr.mp3", "tmp/test_tagged1.mp3"); });
    ASSERT_NO_THROW({ Tags.load("tmp/test_tagged1.mp3"); });
    EXPECT_EQ((size_t)0, Tags.frameCount());
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
    EXPECT_EQ((size_t)13, Tags.frameCount());
    ASSERT_NO_THROW({ Tags.save(); });
    // Save again
    ASSERT_NO_THROW({ Tags.save(); });

    pplib::DirEntry d;
    ASSERT_NO_THROW({ pplib::File::statFile("tmp/test_tagged1.mp3", d); });
    ASSERT_EQ((size_t)97072, d.Size) << "Tagged File has unexpected size";
    EXPECT_EQ(pplib::String("f9a333ac0f6ee3c92fae02390b25248f"), pplib::File::md5Hash("tmp/test_tagged1.mp3"));
    pplib::ID3Tag NewTags;
    ASSERT_NO_THROW({ NewTags.load("tmp/test_tagged1.mp3"); });
    EXPECT_EQ(pplib::String("Patrick Fedick"), NewTags.getArtist());
    EXPECT_EQ(pplib::String("Powerplay Jingle"), NewTags.getTitle());
    EXPECT_EQ(pplib::String("Trance"), NewTags.getGenre());
    EXPECT_EQ(pplib::String("Single"), NewTags.getRemixer());
    EXPECT_EQ(pplib::String("Patrick F.-Productions"), NewTags.getLabel());
    EXPECT_EQ(pplib::String("PPL Testdata"), NewTags.getComment());
    EXPECT_EQ(pplib::String("2013"), NewTags.getYear());
    EXPECT_EQ(pplib::String("PPL Testsuite"), NewTags.getAlbum());
    EXPECT_EQ(pplib::String("2"), NewTags.getTrack());
    EXPECT_EQ(pplib::String("138"), NewTags.getBPM());
    EXPECT_EQ(pplib::String("am"), NewTags.getKey());
    EXPECT_EQ(pplib::String("9"), NewTags.getEnergyLevel());
    pplib::ByteArray cover;
    EXPECT_EQ(false, Tags.getPicture(3, cover));
}

TEST_F(ID3TagTest, Mp3InitialTaggingWithPicture)
{
    pplib::ID3Tag Tags;
    pplib::ByteArray cover;
    pplib::File::load(cover, "testdata/cover.jpg");
    pplib::File::copy("testdata/test_192cbr.mp3", "tmp/test_tagged2.mp3");
    Tags.load("tmp/test_tagged2.mp3");
    EXPECT_EQ((size_t)0, Tags.frameCount());
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
        Tags.setPicture(3, cover, "image/jpeg");
    });
    ASSERT_NO_THROW({ Tags.save(); });
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged2.mp3", d);
    ASSERT_EQ((size_t)125073, d.Size) << "Tagged File has unexpected size";
    EXPECT_EQ(pplib::String("6dc503f9722059d5a0de7171e07c68e4"), pplib::File::md5Hash("tmp/test_tagged2.mp3"));

    pplib::ID3Tag NewTags;
    ASSERT_NO_THROW({ NewTags.load("tmp/test_tagged2.mp3"); });
    EXPECT_EQ(pplib::String("Patrick Fedick"), NewTags.getArtist());
    EXPECT_EQ(pplib::String("Powerplay Jingle"), NewTags.getTitle());
    EXPECT_EQ(pplib::String("Trance"), NewTags.getGenre());
    EXPECT_EQ(pplib::String("Single"), NewTags.getRemixer());
    EXPECT_EQ(pplib::String("Patrick F.-Productions"), NewTags.getLabel());
    EXPECT_EQ(pplib::String("PPL Testdata"), NewTags.getComment());
    EXPECT_EQ(pplib::String("2013"), NewTags.getYear());
    EXPECT_EQ(pplib::String("PPL Testsuite"), NewTags.getAlbum());
    EXPECT_EQ(pplib::String("2"), NewTags.getTrack());
    EXPECT_EQ(pplib::String("138"), NewTags.getBPM());
    EXPECT_EQ(pplib::String("am"), NewTags.getKey());
    EXPECT_EQ(pplib::String("9"), NewTags.getEnergyLevel());
    EXPECT_EQ(true, NewTags.getPicture(3, cover));
    EXPECT_EQ((size_t)28402, cover.size()) << "Embedded Cover has unexpected size";
    EXPECT_EQ(pplib::String("d665f69f04f1413eef91b3596de8dfb6"), pplib::Md5(cover)) << "Embedded Cover has unexpected MD5 hash";
}

TEST_F(ID3TagTest, Mp3RetagWithPicture)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/test_192cbr_tagged.mp3", "tmp/test_tagged3.mp3");
    Tags.load("tmp/test_tagged3.mp3");

    pplib::ByteArray cover;
    pplib::File::load(cover, "testdata/cover.jpg");
    Tags.setPicture(3, cover, "image/jpeg");
    Tags.save();
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged3.mp3", d);
    ASSERT_EQ((size_t)125074, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("052fd75215f383718e1368b535520c60"), pplib::File::md5Hash("tmp/test_tagged3.mp3"));
}

TEST_F(ID3TagTest, FindUserDefinedText)
{
    pplib::ID3Tag Tags;
    Tags.load("testdata/test_192cbr_tagged.mp3");
    ID3Frame* frame = Tags.findUserDefinedText("TraktorRemixer");
    ASSERT_TRUE(frame != NULL);
    frame = Tags.findUserDefinedText("Gibtsnicht");
    ASSERT_TRUE(frame == NULL);
}

TEST_F(ID3TagTest, Mp3RetagStrings)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/test_192cbr_tagged.mp3", "tmp/test_tagged4.mp3");
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

    pplib::ID3Tag NewTags;
    NewTags.load("tmp/test_tagged4.mp3");
    EXPECT_EQ(pplib::String("New Artist"), NewTags.getArtist());
    EXPECT_EQ(pplib::String("New Jingle"), NewTags.getTitle());
    EXPECT_EQ(pplib::String("Techno"), NewTags.getGenre());
    EXPECT_EQ(pplib::String("Maxi"), NewTags.getRemixer());
    EXPECT_EQ(pplib::String("New Label"), NewTags.getLabel());
    EXPECT_EQ(pplib::String("New Testdata"), NewTags.getComment());
    EXPECT_EQ(pplib::String("2014"), NewTags.getYear());
    EXPECT_EQ(pplib::String("New Testsuite"), NewTags.getAlbum());
    EXPECT_EQ(pplib::String("3"), NewTags.getTrack());
    EXPECT_EQ(pplib::String("140"), NewTags.getBPM());
    EXPECT_EQ(pplib::String("em"), NewTags.getKey());
    EXPECT_EQ(pplib::String("7"), NewTags.getEnergyLevel());

    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged4.mp3", d);
    ASSERT_EQ((size_t)97072, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("ffdf4dc172e7c5cc07b543557c33b6d1"), pplib::File::md5Hash("tmp/test_tagged4.mp3"));
}

TEST_F(ID3TagTest, Mp3RemovePicture)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/test_192cbr_taggedWithCover.mp3", "tmp/test_tagged5.mp3");
    Tags.load("tmp/test_tagged5.mp3");
    ASSERT_TRUE(Tags.hasPicture(3));
    ASSERT_NO_THROW(Tags.removePicture(3));
    ASSERT_NO_THROW(Tags.save());

    pplib::ID3Tag NewTags;
    NewTags.load("tmp/test_tagged5.mp3");
    ASSERT_FALSE(Tags.hasPicture(3));

    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged5.mp3", d);
    ASSERT_EQ((size_t)97073, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("e2dacaeff7f3dbc0d54ed63e88ba519d"), pplib::File::md5Hash("tmp/test_tagged5.mp3"));
}

TEST_F(ID3TagTest, Mp3RemoveAllTags)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/test_192cbr_taggedWithCover.mp3", "tmp/test_tagged6.mp3");
    Tags.load("tmp/test_tagged6.mp3");
    ASSERT_NO_THROW(Tags.clearTags());
    Tags.save();
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged6.mp3", d);
    ASSERT_EQ((size_t)95921, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("692bf339243cee92f1c639b10ffde45e"), pplib::File::md5Hash("tmp/test_tagged6.mp3"));
}

TEST_F(ID3TagTest, Mp3NoTagsAndNoChange)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/test_192cbr.mp3", "tmp/test_tagged7.mp3");
    Tags.load("tmp/test_tagged7.mp3");
    ASSERT_NO_THROW(Tags.clearTags());
    ASSERT_NO_THROW(Tags.save());
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged7.mp3", d);
    ASSERT_EQ((size_t)95920, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("0abbdd3ce267358a0b3bf3f0a015e74e"), pplib::File::md5Hash("tmp/test_tagged7.mp3"));
}

TEST_F(ID3TagTest, Mp3RetagWithoutChanges)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/test_192cbr_tagged.mp3", "tmp/test_tagged8.mp3");
    Tags.load("tmp/test_tagged8.mp3");
    Tags.save();
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged8.mp3", d);
    ASSERT_EQ((size_t)97072, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("c5ff756219cba391c99423ddd6cca625"), pplib::File::md5Hash("tmp/test_tagged8.mp3"));
}

TEST_F(ID3TagTest, AiffLoadFileWithoutTags)
{
    pplib::ID3Tag Tags;
    ASSERT_NO_THROW({ Tags.load("testdata/test_44kHz.aiff"); });
    EXPECT_EQ((size_t)0, Tags.frameCount()) << "Unexpected number of ID3-Frames";
}

TEST_F(ID3TagTest, AiffLoadFileWithTags)
{
    pplib::ID3Tag Tags;
    ASSERT_NO_THROW({ Tags.load("testdata/test_44kHz_taggedWithCover.aiff"); });
    EXPECT_EQ(pplib::String("Patrick Fedick"), Tags.getArtist());
    EXPECT_EQ(pplib::String("Powerplay Jingle"), Tags.getTitle());
    EXPECT_EQ(pplib::String("Trance"), Tags.getGenre());
    EXPECT_EQ(pplib::String("Single"), Tags.getRemixer());
    EXPECT_EQ(pplib::String("Patrick F.-Productions"), Tags.getLabel());
    EXPECT_EQ(pplib::String("PPL Testdata"), Tags.getComment());
    EXPECT_EQ(pplib::String("2013"), Tags.getYear());
    EXPECT_EQ(pplib::String("PPL Testsuite"), Tags.getAlbum());
    EXPECT_EQ(pplib::String("2"), Tags.getTrack());
    EXPECT_EQ(pplib::String("138"), Tags.getBPM());
    EXPECT_EQ(pplib::String("am"), Tags.getKey());
    EXPECT_EQ(pplib::String("9"), Tags.getEnergyLevel());
    pplib::ByteArray cover;
    EXPECT_EQ(true, Tags.getPicture(3, cover));
    EXPECT_EQ((size_t)28402, cover.size()) << "Embedded Cover has unexpected size";
    EXPECT_EQ(pplib::String("d665f69f04f1413eef91b3596de8dfb6"), pplib::Md5(cover)) << "Embedded Cover has unexpected MD5 hash";
}

TEST_F(ID3TagTest, AiffInitialTaggingWithoutPicture)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/test_44kHz.aiff", "tmp/test_tagged1.aiff");
    ASSERT_NO_THROW({ Tags.load("tmp/test_tagged1.aiff"); });
    EXPECT_EQ((size_t)0, Tags.frameCount()) << "Unexpected number of ID3-Frames";

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
        }
        catch (const pplib::Exception& e) {
            e.print();
            throw;
        }
    });
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged1.aiff", d);
    EXPECT_EQ((size_t)695866, d.Size) << "Tagged File has unexpected size";
    EXPECT_EQ(pplib::String("ee7fa3d57fd26f6a45e1f9448fd8c09c"), pplib::File::md5Hash("tmp/test_tagged1.aiff"));

    pplib::ID3Tag NewTags;
    pplib::ByteArray cover;
    ASSERT_NO_THROW({ NewTags.load("tmp/test_tagged1.aiff"); });

    EXPECT_EQ(pplib::String("Patrick Fedick"), NewTags.getArtist());
    EXPECT_EQ(pplib::String("Powerplay Jingle"), NewTags.getTitle());
    EXPECT_EQ(pplib::String("Trance"), NewTags.getGenre());
    EXPECT_EQ(pplib::String("Single"), NewTags.getRemixer());
    EXPECT_EQ(pplib::String("Patrick F.-Productions"), NewTags.getLabel());
    EXPECT_EQ(pplib::String("PPL Testdata"), NewTags.getComment());
    EXPECT_EQ(pplib::String("2013"), NewTags.getYear());
    EXPECT_EQ(pplib::String("PPL Testsuite"), NewTags.getAlbum());
    EXPECT_EQ(pplib::String("2"), NewTags.getTrack());
    EXPECT_EQ(pplib::String("138"), NewTags.getBPM());
    EXPECT_EQ(pplib::String("am"), NewTags.getKey());
    EXPECT_EQ(pplib::String("9"), NewTags.getEnergyLevel());
    EXPECT_EQ(false, NewTags.getPicture(3, cover));
}

TEST_F(ID3TagTest, AiffInitialTaggingWithPicture)
{
    pplib::ID3Tag Tags;
    pplib::ByteArray cover;
    pplib::File::load(cover, "testdata/cover.jpg");
    pplib::File::copy("testdata/test_44kHz.aiff", "tmp/test_tagged2.aiff");
    Tags.load("tmp/test_tagged2.aiff");
    EXPECT_EQ((size_t)0, Tags.frameCount());
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
        Tags.setPicture(3, cover, "image/jpeg");
    });
    ASSERT_NO_THROW({ Tags.save(); });
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged2.aiff", d);
    ASSERT_EQ((size_t)723867, d.Size) << "Tagged File has unexpected size";
    EXPECT_EQ(pplib::String("8b48f5ce239455880fa35f0a83ae30e5"), pplib::File::md5Hash("tmp/test_tagged2.aiff"));

    pplib::ID3Tag NewTags;
    ASSERT_NO_THROW({ NewTags.load("tmp/test_tagged2.aiff"); });
    EXPECT_EQ(pplib::String("Patrick Fedick"), NewTags.getArtist());
    EXPECT_EQ(pplib::String("Powerplay Jingle"), NewTags.getTitle());
    EXPECT_EQ(pplib::String("Trance"), NewTags.getGenre());
    EXPECT_EQ(pplib::String("Single"), NewTags.getRemixer());
    EXPECT_EQ(pplib::String("Patrick F.-Productions"), NewTags.getLabel());
    EXPECT_EQ(pplib::String("PPL Testdata"), NewTags.getComment());
    EXPECT_EQ(pplib::String("2013"), NewTags.getYear());
    EXPECT_EQ(pplib::String("PPL Testsuite"), NewTags.getAlbum());
    EXPECT_EQ(pplib::String("2"), NewTags.getTrack());
    EXPECT_EQ(pplib::String("138"), NewTags.getBPM());
    EXPECT_EQ(pplib::String("am"), NewTags.getKey());
    EXPECT_EQ(pplib::String("9"), NewTags.getEnergyLevel());
    EXPECT_EQ(true, NewTags.getPicture(3, cover));
    EXPECT_EQ((size_t)28402, cover.size()) << "Embedded Cover has unexpected size";
    EXPECT_EQ(pplib::String("d665f69f04f1413eef91b3596de8dfb6"), pplib::Md5(cover)) << "Embedded Cover has unexpected MD5 hash";
}

TEST_F(ID3TagTest, AiffRetagWithPicture)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/test_44kHz_tagged.aiff", "tmp/test_tagged3.aiff");
    Tags.load("tmp/test_tagged3.aiff");
    pplib::ByteArray cover;
    pplib::File::load(cover, "testdata/cover.jpg");
    Tags.setPicture(3, cover, "image/jpeg");
    Tags.save();
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged3.aiff", d);
    ASSERT_EQ((size_t)723867, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("bc9d24e8dffffda4cd7208122e83e65f"), pplib::File::md5Hash("tmp/test_tagged3.aiff"));
}

TEST_F(ID3TagTest, AiffRetagStrings)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/test_44kHz_tagged.aiff", "tmp/test_tagged4.aiff");
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

    pplib::ID3Tag NewTags;
    NewTags.load("tmp/test_tagged4.aiff");
    EXPECT_EQ(pplib::String("New Artist"), NewTags.getArtist());
    EXPECT_EQ(pplib::String("New Jingle"), NewTags.getTitle());
    EXPECT_EQ(pplib::String("Techno"), NewTags.getGenre());
    EXPECT_EQ(pplib::String("Maxi"), NewTags.getRemixer());
    EXPECT_EQ(pplib::String("New Label"), NewTags.getLabel());
    EXPECT_EQ(pplib::String("New Testdata"), NewTags.getComment());
    EXPECT_EQ(pplib::String("2014"), NewTags.getYear());
    EXPECT_EQ(pplib::String("New Testsuite"), NewTags.getAlbum());
    EXPECT_EQ(pplib::String("3"), NewTags.getTrack());
    EXPECT_EQ(pplib::String("140"), NewTags.getBPM());
    EXPECT_EQ(pplib::String("em"), NewTags.getKey());
    EXPECT_EQ(pplib::String("7"), NewTags.getEnergyLevel());

    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged4.aiff", d);
    ASSERT_EQ((size_t)695866, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("a254ec79ef0169d2fd6ad70f98a4c193"), pplib::File::md5Hash("tmp/test_tagged4.aiff"));
}

TEST_F(ID3TagTest, AiffRetagStringsAndRetagWithCoverAgain)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/test_44kHz_tagged.aiff", "tmp/test_tagged10.aiff");
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

    pplib::ID3Tag Tags2;
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
    pplib::ByteArray cover;
    pplib::File::load(cover, "testdata/cover.jpg");
    Tags2.setPicture(3, cover, "image/jpeg");
    Tags2.save();

    pplib::ID3Tag NewTags;
    NewTags.load("tmp/test_tagged10.aiff");
    EXPECT_EQ(pplib::String("New Artist"), NewTags.getArtist());
    EXPECT_EQ(pplib::String("New Jingle"), NewTags.getTitle());
    EXPECT_EQ(pplib::String("Techno"), NewTags.getGenre());
    EXPECT_EQ(pplib::String("Maxi"), NewTags.getRemixer());
    EXPECT_EQ(pplib::String("New Label"), NewTags.getLabel());
    EXPECT_EQ(pplib::String("New Testdata"), NewTags.getComment());
    EXPECT_EQ(pplib::String("2014"), NewTags.getYear());
    EXPECT_EQ(pplib::String("New Testsuite"), NewTags.getAlbum());
    EXPECT_EQ(pplib::String("3"), NewTags.getTrack());
    EXPECT_EQ(pplib::String("140"), NewTags.getBPM());
    EXPECT_EQ(pplib::String("em"), NewTags.getKey());
    EXPECT_EQ(pplib::String("7"), NewTags.getEnergyLevel());

    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged10.aiff", d);
    ASSERT_EQ((size_t)723813, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("01a1ae462ad55207e4b719d1eb83801c"), pplib::File::md5Hash("tmp/test_tagged10.aiff"));
}

TEST_F(ID3TagTest, AiffRemovePicture)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/test_44kHz_taggedWithCover.aiff", "tmp/test_tagged5.aiff");
    Tags.load("tmp/test_tagged5.aiff");
    ASSERT_TRUE(Tags.hasPicture(3));
    ASSERT_NO_THROW(Tags.removePicture(3));
    ASSERT_NO_THROW(Tags.save());

    pplib::ID3Tag NewTags;
    NewTags.load("tmp/test_tagged5.aiff");
    ASSERT_FALSE(Tags.hasPicture(3));

    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged5.aiff", d);
    ASSERT_EQ((size_t)695866, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("6d357b3f28e47d0cdc3cff6c4133b339"), pplib::File::md5Hash("tmp/test_tagged5.aiff"));
}

TEST_F(ID3TagTest, AiffRemoveAllTags)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/test_44kHz_taggedWithCover.aiff", "tmp/test_tagged6.aiff");
    Tags.load("tmp/test_tagged6.aiff");
    ASSERT_NO_THROW(Tags.clearTags());
    Tags.save();
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged6.aiff", d);
    ASSERT_EQ((size_t)694834, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("608bd0c668528fc0a143a4d3b8dac640"), pplib::File::md5Hash("tmp/test_tagged6.aiff"));
}

TEST_F(ID3TagTest, AiffNoTagsAndNoChange)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/test_44kHz.aiff", "tmp/test_tagged7.aiff");
    Tags.load("tmp/test_tagged7.aiff");
    ASSERT_NO_THROW(Tags.clearTags());
    ASSERT_NO_THROW(Tags.save());
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged7.aiff", d);
    ASSERT_EQ((size_t)694834, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("608bd0c668528fc0a143a4d3b8dac640"), pplib::File::md5Hash("tmp/test_tagged7.aiff"));
}

TEST_F(ID3TagTest, AiffRetagWithoutChanges)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/test_44kHz_tagged.aiff", "tmp/test_tagged8.aiff");
    Tags.load("tmp/test_tagged8.aiff");
    Tags.save();
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged8.aiff", d);
    ASSERT_EQ((size_t)695866, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("ddc103beb0e1687dd6631e31a4a06a62"), pplib::File::md5Hash("tmp/test_tagged8.aiff"));
}

TEST_F(ID3TagTest, GetID3GenreName)
{
    ASSERT_EQ(pplib::String("Blues"), pplib::GetID3GenreName(0));
    ASSERT_EQ(pplib::String("SynthPop"), pplib::GetID3GenreName(147));
    ASSERT_EQ(pplib::String("Trance"), pplib::GetID3GenreName(31));
    ASSERT_THROW(pplib::GetID3GenreName(-1), pplib::InvalidGenreException);
    ASSERT_THROW(pplib::GetID3GenreName(148), pplib::InvalidGenreException);
}

TEST_F(ID3TagTest, Overflow)
{
    pplib::ID3Tag Tags;
    try {
        Tags.load("testdata/tags_at_eof.aiff");
    }
    catch (const pplib::Exception& ex) {
        ex.print();
    }
}

/*
TEST_F(ID3TagTest, AiffRetagRealFile) {

    pplib::String TestFile="/home/patrick/svn/pplib-tryout/Traktor4TagReader/181-Armin van Buuren - EIFORYA (Patrick F. Intro Edit).aiff";
    if (!pplib::File::Exists(TestFile)) return;
    ASSERT_EQ(1,pplib::File::CopyFile(TestFile,"tmp/test_tagged9.aiff"));
    pplib::ID3Tag Tags;
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
    pplib::ID3Tag Tags2;
    ASSERT_EQ(1,Tags2.Load("tmp/test_tagged9.aiff")) << pplib::Error2String();
    pplib::CBinary cover;
    cover.Load("/home/patrick/svn/pplib-tryout/Traktor4TagReader/eiforya.jpg");
    EXPECT_EQ(1,Tags2.SetPicture(3,cover,"image/jpeg"));
    ASSERT_EQ(1,Tags2.Save()) << "Saving taggs failed";

    pplib::CDirEntry d;
    ASSERT_EQ(1,pplib::File::statFile("tmp/test_tagged9.aiff",d)) << "Tagged File does not exist!";
    ASSERT_EQ((size_t)44318523,d.Size) << "Tagged File has unexpected size";
    //ASSERT_EQ(pplib::String("ddc103beb0e1687dd6631e31a4a06a62"),pplib::File::MD5("tmp/test_tagged9.aiff"));
}
*/

} // namespace pplib
