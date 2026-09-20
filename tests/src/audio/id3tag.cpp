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

#include <pplib/types/string.h>
// #include <pplib/types/widestring.h>
//  #include <pplib/types/array.h>
// #include <pplib/exceptions.h>
#include <pplib/core/dir.h>
#include <pplib/core/iconv.h>
#include <pplib/core/functions.h>
#include <pplib/core/memfile.h>
// #include <pplib/core/regex.h>
#include <pplib/audio/id3tag.h>

#include "pplib-tests.h"

namespace pplib
{

class ID3TagFrameTest : public ::testing::Test
{
protected:
    ID3TagFrameTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~ID3TagFrameTest()
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

TEST_F(ID3TagTest, FrameConstructorWithoutName)
{
    ASSERT_NO_THROW({
        pplib::ID3Frame Frame;
        EXPECT_EQ(pplib::String(), Frame.name());
        EXPECT_EQ((int)0, Frame.flags());
        EXPECT_EQ((size_t)0, Frame.size());
        EXPECT_FALSE(Frame.hasData());
    });
}

TEST_F(ID3TagTest, FrameConstructorWithName)
{
    ASSERT_NO_THROW({
        pplib::ID3Frame Frame("TITL");
        EXPECT_EQ(pplib::String("TITL"), Frame.name());
        EXPECT_EQ((int)0, Frame.flags());
        EXPECT_EQ((size_t)0, Frame.size());
        EXPECT_FALSE(Frame.hasData());
    });
}

TEST_F(ID3TagTest, FramesetFlagsGetFlags)
{
    pplib::ID3Frame Frame;

    ASSERT_NO_THROW({ Frame.setFlags(42); });
    EXPECT_EQ((int)42, Frame.flags());
}

TEST_F(ID3TagTest, FramesetDataGetData)
{
    pplib::ID3Frame Frame;
    pplib::ByteArray cover;
    pplib::File::load(cover, "testdata/audio/cover.jpg");

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

TEST_F(ID3TagTest, FrameConstructorWithData)
{
    pplib::ByteArray data;
    data.copy("TestData", 8);
    pplib::ID3Frame frame("TIT2", 5, data);
    EXPECT_EQ(pplib::String("TIT2"), frame.name());
    EXPECT_EQ(5, frame.flags());
    EXPECT_EQ((size_t)8, frame.size());
    EXPECT_TRUE(frame.hasData());
    EXPECT_FALSE(frame.isEmpty());
    EXPECT_EQ(data, frame.getData());

    pplib::ByteArray out;
    frame.getData(out);
    EXPECT_EQ(data, out);

    // frame.hexDump();
    pplib::ID3Frame emptyFrame;
    // emptyFrame.hexDump();
    EXPECT_TRUE(emptyFrame.isEmpty());
    EXPECT_FALSE(emptyFrame.hasData());
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
    ASSERT_NO_THROW({ Tags.load("testdata/audio/test_192cbr.mp3"); });
    EXPECT_EQ((size_t)0, Tags.frameCount());
}

TEST_F(ID3TagTest, copyAndDecodeTextWithoutEncodingByte)
{
    pplib::ID3Frame frame("TIT1");
    pplib::ID3Tag Tag;
    pplib::String result;
    pplib::ByteArray ba;
    pplib::String expected("Hällo Wörld");
    ba = pplib::Iconv::transcode(expected, "UTF-8", "ISO-8859-1");
    // ba = expected.toEncoding("ISO-8859-1");
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
    pplib::String expected("Hällo Wörld");
    ba = pplib::Iconv::transcode(expected, "UTF-8", "ISO-8859-1");
    // ba = expected.toEncoding("ISO-8859-1");
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
    pplib::String expected("Hällo Wörld");
    ba = pplib::Iconv::transcode(expected, "UTF-8", "UTF-16");
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
    pplib::String expected("Hällo Wörld");
    ba = pplib::Iconv::transcode(expected, "UTF-8", "UTF-16BE");
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
    ba = pplib::Iconv::transcode(expected, "UTF-8", "US-ASCII");
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
    ASSERT_NO_THROW({ Tags.load("testdata/audio/test_192cbr_taggedWithCover.mp3"); });
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
    ASSERT_NO_THROW({ pplib::File::copy("testdata/audio/test_192cbr.mp3", "tmp/test_tagged1.mp3"); });
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
    EXPECT_EQ(pplib::String("1c0573448bda0912a70d860e438886ed"), pplib::File::md5Hash("tmp/test_tagged1.mp3"));
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
    pplib::File::load(cover, "testdata/audio/cover.jpg");
    pplib::File::copy("testdata/audio/test_192cbr.mp3", "tmp/test_tagged2.mp3");
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
    ASSERT_EQ((size_t)125063, d.Size) << "Tagged File has unexpected size";
    EXPECT_EQ(pplib::String("abbc6bb0026c6cece302561562ef25f5"), pplib::File::md5Hash("tmp/test_tagged2.mp3"));

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
    pplib::File::copy("testdata/audio/test_192cbr_tagged.mp3", "tmp/test_tagged3.mp3");
    Tags.load("tmp/test_tagged3.mp3");

    pplib::ByteArray cover;
    pplib::File::load(cover, "testdata/audio/cover.jpg");
    Tags.setPicture(3, cover, "image/jpeg");
    Tags.save();
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged3.mp3", d);
    ASSERT_EQ((size_t)125064, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("1e78e5bd961d9db33d5bc1924e58f082"), pplib::File::md5Hash("tmp/test_tagged3.mp3"));
}

TEST_F(ID3TagTest, FindUserDefinedText)
{
    pplib::ID3Tag Tags;
    Tags.load("testdata/audio/test_192cbr_tagged.mp3");
    ID3Frame* frame = Tags.findUserDefinedText("TraktorRemixer");
    ASSERT_TRUE(frame != NULL);
    frame = Tags.findUserDefinedText("Gibtsnicht");
    ASSERT_TRUE(frame == NULL);
}

TEST_F(ID3TagTest, Mp3RetagStrings)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/audio/test_192cbr_tagged.mp3", "tmp/test_tagged4.mp3");
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
    ASSERT_EQ(pplib::String("8c794b6ede6599942f9d94d0432c701e"), pplib::File::md5Hash("tmp/test_tagged4.mp3"));
}

TEST_F(ID3TagTest, Mp3RemovePicture)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/audio/test_192cbr_taggedWithCover.mp3", "tmp/test_tagged5.mp3");
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
    ASSERT_EQ(pplib::String("05d0f69f9a2c1b55c9d5a3bd517adda6"), pplib::File::md5Hash("tmp/test_tagged5.mp3"));
}

TEST_F(ID3TagTest, Mp3RemoveAllTags)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/audio/test_192cbr_taggedWithCover.mp3", "tmp/test_tagged6.mp3");
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
    pplib::File::copy("testdata/audio/test_192cbr.mp3", "tmp/test_tagged7.mp3");
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
    pplib::File::copy("testdata/audio/test_192cbr_tagged.mp3", "tmp/test_tagged8.mp3");
    Tags.load("tmp/test_tagged8.mp3");
    Tags.save();
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged8.mp3", d);
    ASSERT_EQ((size_t)97072, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("0148f9031e2e7c3b5aa3280d8c833719"), pplib::File::md5Hash("tmp/test_tagged8.mp3"));
}

TEST_F(ID3TagTest, AiffLoadFileWithoutTags)
{
    pplib::ID3Tag Tags;
    ASSERT_NO_THROW({ Tags.load("testdata/audio/test_44kHz.aiff"); });
    EXPECT_EQ((size_t)0, Tags.frameCount()) << "Unexpected number of ID3-Frames";
}

TEST_F(ID3TagTest, AiffLoadFileWithTags)
{
    pplib::ID3Tag Tags;
    ASSERT_NO_THROW({ Tags.load("testdata/audio/test_44kHz_taggedWithCover.aiff"); });
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
    pplib::File::copy("testdata/audio/test_44kHz.aiff", "tmp/test_tagged1.aiff");
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
    EXPECT_EQ(pplib::String("926d5dad6321cb9f435e3341e7cac50d"), pplib::File::md5Hash("tmp/test_tagged1.aiff"));

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
    pplib::File::load(cover, "testdata/audio/cover.jpg");
    pplib::File::copy("testdata/audio/test_44kHz.aiff", "tmp/test_tagged2.aiff");
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
    ASSERT_EQ((size_t)723858, d.Size) << "Tagged File has unexpected size";
    EXPECT_EQ(pplib::String("712091397916792c4bdf3340f7b5a4fa"), pplib::File::md5Hash("tmp/test_tagged2.aiff"));

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
    pplib::File::copy("testdata/audio/test_44kHz_tagged.aiff", "tmp/test_tagged3.aiff");
    Tags.load("tmp/test_tagged3.aiff");
    pplib::ByteArray cover;
    pplib::File::load(cover, "testdata/audio/cover.jpg");
    Tags.setPicture(3, cover, "image/jpeg");
    Tags.save();
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged3.aiff", d);
    ASSERT_EQ((size_t)723858, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("39e2fd3fa1eb134c63092d3c0770a025"), pplib::File::md5Hash("tmp/test_tagged3.aiff"));
}

TEST_F(ID3TagTest, AiffRetagStrings)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/audio/test_44kHz_tagged.aiff", "tmp/test_tagged4.aiff");
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
    ASSERT_EQ(pplib::String("a52134f4203b0b7db4520124b5f6678f"), pplib::File::md5Hash("tmp/test_tagged4.aiff"));
}

TEST_F(ID3TagTest, AiffRetagStringsAndRetagWithCoverAgain)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/audio/test_44kHz_tagged.aiff", "tmp/test_tagged10.aiff");
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
    pplib::File::load(cover, "testdata/audio/cover.jpg");
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
    ASSERT_EQ((size_t)723804, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("aa2d422cffba6fc99d4d6108d8f86ff2"), pplib::File::md5Hash("tmp/test_tagged10.aiff"));
}

TEST_F(ID3TagTest, AiffRemovePicture)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/audio/test_44kHz_taggedWithCover.aiff", "tmp/test_tagged5.aiff");
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
    ASSERT_EQ(pplib::String("bde30dc6e7364f7cc88417a999c32bbc"), pplib::File::md5Hash("tmp/test_tagged5.aiff"));
}

TEST_F(ID3TagTest, AiffRemoveAllTags)
{
    pplib::ID3Tag Tags;
    pplib::File::copy("testdata/audio/test_44kHz_taggedWithCover.aiff", "tmp/test_tagged6.aiff");
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
    pplib::File::copy("testdata/audio/test_44kHz.aiff", "tmp/test_tagged7.aiff");
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
    pplib::File::copy("testdata/audio/test_44kHz_tagged.aiff", "tmp/test_tagged8.aiff");
    Tags.load("tmp/test_tagged8.aiff");
    Tags.save();
    pplib::DirEntry d;
    pplib::File::statFile("tmp/test_tagged8.aiff", d);
    ASSERT_EQ((size_t)695866, d.Size) << "Tagged File has unexpected size";
    ASSERT_EQ(pplib::String("a4566b3944fed0229a95d78a1eff9a84"), pplib::File::md5Hash("tmp/test_tagged8.aiff"));
}

TEST_F(ID3TagTest, GetID3GenreName)
{
    ASSERT_EQ(pplib::String("Blues"), pplib::GetID3GenreName(0));
    ASSERT_EQ(pplib::String("SynthPop"), pplib::GetID3GenreName(147));
    ASSERT_EQ(pplib::String("Trance"), pplib::GetID3GenreName(31));
    ASSERT_EQ(pplib::String(), pplib::GetID3GenreName(-1));
    ASSERT_EQ(pplib::String(), pplib::GetID3GenreName(148));
}

TEST_F(ID3TagTest, Overflow)
{
    pplib::ID3Tag Tags;
    try {
        Tags.load("testdata/audio/tags_at_eof.aiff");
    }
    catch (const pplib::Exception& ex) {
        ex.print();
    }
}

TEST_F(ID3TagTest, DecodeOffsetExceedsSize)
{
    pplib::ID3Frame frame("COMM");
    char data[1] = {0}; // Only encoding byte
    frame.setData(data, 1);

    pplib::String target;
    int nextOffset = pplib::ID3Tag::decode(&frame, 4, 0, target, "UTF-8");
    EXPECT_EQ(4, nextOffset);
    EXPECT_TRUE(target.isEmpty());
}

TEST_F(ID3TagTest, GetPrivateDataShortFrame)
{
    pplib::ID3Tag tag;
    pplib::ID3Frame frame("PRIV");
    frame.setData("MyIdent", 7);
    tag.addFrame(frame);

    pplib::ByteArray bin;
    EXPECT_FALSE(tag.getPrivateData(bin, "MyIdent"));
    pplib::ByteArrayPtr ptr = tag.getPrivateData("MyIdent");
    EXPECT_TRUE(ptr.isNull());
}

TEST_F(ID3TagTest, UnsynchronisationShortFrame)
{
    unsigned char tagData[] = {
        'I', 'D', '3', 4,   0, 0x80, 0, 0, 0, 11, // Tag Header, synchsafe size 11
        'T', 'E', 'S', 'T', 0, 0,    0, 1, 0, 0,  // Frame Header, size 1
        0xFF                                      // 1 data byte
    };
    pplib::ByteArray mp3;
    pplib::File::load(mp3, "testdata/audio/test_192cbr.mp3");
    mp3.prepend(tagData, sizeof(tagData));
    pplib::File::save(mp3.ptr(), mp3.size(), "tmp/test_unsync.mp3");

    pplib::ID3Tag tag;
    ASSERT_NO_THROW({ tag.load("tmp/test_unsync.mp3"); });
    pplib::ID3Frame* f = tag.findFrame("TEST");
    ASSERT_NE(nullptr, f);
    EXPECT_EQ((size_t)1, f->size());
}

TEST_F(ID3TagTest, FooterPresentIgnored)
{
    unsigned char tagData[] = {
        'I', 'D', '3', 4,   0,   0x10, 0, 0, 0, 15, // Tag Header, size 15
        'T', 'I', 'T', '2', 0,   0,    0, 5, 0, 0,  // Frame Header
        0,   'T', 'e', 's', 't',                    // 5 bytes data (ISO-8859-1)
        '3', 'D', 'I', 4,   0,   0x10, 0, 0, 0, 15  // 10 bytes Footer
    };
    pplib::ByteArray mp3;
    pplib::File::load(mp3, "testdata/audio/test_192cbr.mp3");
    mp3.prepend(tagData, sizeof(tagData));
    pplib::File::save(mp3.ptr(), mp3.size(), "tmp/test_footer.mp3");

    pplib::ID3Tag tag;
    ASSERT_NO_THROW({ tag.load("tmp/test_footer.mp3"); });
    EXPECT_EQ(pplib::String("Test"), tag.getTitle());
    EXPECT_EQ((size_t)1, tag.frameCount());
}

TEST_F(ID3TagTest, AiffCorruptedChunkSize)
{
    unsigned char aiffData[] = {'F', 'O', 'R', 'M', 0, 0, 0, 12, 'A', 'I', 'F', 'F', 'N', 'O', 'P', 'E', 0xFF, 0xFF, 0xFF, 0xF8};
    pplib::File::save(aiffData, sizeof(aiffData), "tmp/test_corrupt.aiff");
    pplib::ID3Tag tag;
    ASSERT_NO_THROW({ tag.load("tmp/test_corrupt.aiff"); });
    EXPECT_EQ((size_t)0, tag.frameCount());
}

TEST_F(ID3TagTest, CopyAndDecodeTextOffsetOutOfBounds)
{
    pplib::ID3Frame frame("TIT2");
    frame.setData("Test", 4);
    pplib::String target;
    ASSERT_NO_THROW({ pplib::ID3Tag::copyAndDecodeText(target, &frame, 10, "UTF-8"); });
    EXPECT_TRUE(target.isEmpty());
}

TEST_F(ID3TagTest, DecodeOddByteCountUtf16)
{
    pplib::ID3Frame frame("TIT2");
    char data[3] = {'A', 0, 'B'};
    frame.setData(data, 3);
    pplib::String target;
    ASSERT_NO_THROW({ pplib::ID3Tag::decode(&frame, 0, 1, target, "UTF-8"); });
    EXPECT_FALSE(target.isEmpty());
}

TEST_F(ID3TagTest, DecodeUnknownEncoding)
{
    pplib::ID3Frame frame("TIT2");
    frame.setData("Data", 4);
    pplib::String target;
    int nextOffset = pplib::ID3Tag::decode(&frame, 0, 5, target, "UTF-8");
    EXPECT_EQ(1, nextOffset);
    EXPECT_TRUE(target.isEmpty());
}

TEST_F(ID3TagTest, DecodeUtf8RespectsCharset)
{
    pplib::ID3Frame frame("TIT2");
    pplib::String utf8Str("Übergrößenträger");
    frame.setData(utf8Str.c_str(), utf8Str.size());
    pplib::String target;
    pplib::ID3Tag::decode(&frame, 0, 3, target, "ISO-8859-1");
    pplib::String expected = pplib::Transcode(utf8Str.c_str(), utf8Str.size(), "UTF-8", "ISO-8859-1");
    EXPECT_EQ(expected, target);
}

TEST_F(ID3TagTest, Id3v24ExtendedHeader)
{
    // ID3v2.4 Header with Extended Header Flag (0x40)
    // Extended Header size (synchsafe) = 6 bytes (flags length 1, flag 1 byte, 4 bytes padding or data)
    // Total extended header = 4 + 6 = 10 bytes
    // Followed by TIT2 frame
    unsigned char tagData[] = {
        'I', 'D', '3', 4,   0,  0x40, 0, 0, 0, 25, // Header, size 25 synchsafe
        0,   0,   0,   6,   1,  0,    0, 0, 0, 0,  // 10 bytes Extended Header (size 6 excludes itself)
        'T', 'I', 'T', '2', 0,  0,    0, 5, 0, 0,  // TIT2 header, size 5
        0,   'T', 'e', 's', 't'                    // 5 bytes data (ISO-8859-1)
    };
    pplib::ByteArray mp3;
    pplib::File::load(mp3, "testdata/audio/test_192cbr.mp3");
    mp3.prepend(tagData, sizeof(tagData));
    pplib::File::save(mp3.ptr(), mp3.size(), "tmp/test_ext_header.mp3");

    pplib::ID3Tag tag;
    ASSERT_NO_THROW({ tag.load("tmp/test_ext_header.mp3"); });
    EXPECT_EQ(pplib::String("Test"), tag.getTitle());
}

TEST_F(ID3TagTest, GenerateId3v2TagIgnoresInvalidFrameName)
{
    pplib::ID3Tag tag;
    pplib::ID3Frame emptyFrame;
    pplib::ID3Frame shortFrame("A");
    tag.addFrame(emptyFrame);
    tag.addFrame(shortFrame);
    tag.setTitle("ValidTitle");

    pplib::ByteArray serialized;
    ASSERT_NO_THROW({ tag.generateId3V2Tag(serialized); });
    EXPECT_GT(serialized.size(), (size_t)0);

    // Verify loading the serialized tag
    pplib::ByteArray mp3;
    pplib::File::load(mp3, "testdata/audio/test_192cbr.mp3");
    mp3.prepend(serialized);
    pplib::File::save(mp3.ptr(), mp3.size(), "tmp/test_invalid_frames.mp3");

    pplib::ID3Tag tag2;
    ASSERT_NO_THROW({ tag2.load("tmp/test_invalid_frames.mp3"); });
    EXPECT_EQ(pplib::String("ValidTitle"), tag2.getTitle());
    EXPECT_EQ((size_t)1, tag2.frameCount());
}

TEST_F(ID3TagTest, UnsynchronisationReversing)
{
    // Tag with unsync flag (0x80) containing FF 00 41 ('A')
    // Must be de-unsynchronized to FF 41
    unsigned char tagData[] = {
        'I',  'D',  '3', 4,   0, 0x80, 0, 0, 0, 13, // Tag Header, size 13
        'T',  'E',  'S', 'T', 0, 0,    0, 3, 0, 0,  // Frame Header, size 3
        0xFF, 0x00, 'A'                             // 3 data bytes unsynchronized
    };
    pplib::ByteArray mp3;
    pplib::File::load(mp3, "testdata/audio/test_192cbr.mp3");
    mp3.prepend(tagData, sizeof(tagData));
    pplib::File::save(mp3.ptr(), mp3.size(), "tmp/test_unsync_ff00.mp3");

    pplib::ID3Tag tag;
    ASSERT_NO_THROW({ tag.load("tmp/test_unsync_ff00.mp3"); });
    pplib::ID3Frame* f = tag.findFrame("TEST");
    ASSERT_NE(nullptr, f);
    ASSERT_EQ((size_t)2, f->size());
    EXPECT_EQ((unsigned char)0xFF, (unsigned char)f->dataPtr()[0]);
    EXPECT_EQ('A', f->dataPtr()[1]);
}

TEST_F(ID3TagTest, GetPictureNoPictureData)
{
    pplib::ID3Tag tag;
    pplib::ID3Frame frame("APIC");
    // APIC: Encoding(1), MIME(image/jpeg\0), PicType(3), Description(\0), but NO picture bytes!
    const char data[] = {0, 'i', 'm', 'a', 'g', 'e', '/', 'j', 'p', 'e', 'g', 0, 3, 0};
    frame.setData(data, sizeof(data));
    tag.addFrame(frame);

    EXPECT_FALSE(tag.hasPicture(3));
    pplib::ByteArray bin;
    EXPECT_FALSE(tag.getPicture(3, bin));
    EXPECT_EQ((size_t)0, bin.size());
}

TEST_F(ID3TagTest, GetYearFallbackToTdrc)
{
    pplib::ID3Tag tag;
    tag.setTextFrame("TDRC", "2024-05-12");
    EXPECT_EQ(pplib::String("2024"), tag.getYear());

    pplib::ByteArray v1;
    tag.generateId3V1Tag(v1);
    EXPECT_EQ(0, memcmp((const char*)v1.ptr() + 93, "2024", 4));
}

TEST_F(ID3TagTest, PopularimeterEmptyEmail)
{
    pplib::ID3Tag tag;
    tag.setPopularimeter("", 196);
    EXPECT_TRUE(tag.hasPopularimeter(""));
    EXPECT_TRUE(tag.hasPopularimeter());
    EXPECT_EQ(196, tag.getPopularimeter(""));
    EXPECT_EQ(196, tag.getPopularimeter());

    std::map<pplib::String, unsigned char> ratings;
    tag.getAllPopularimeters(ratings);
    EXPECT_EQ((size_t)1, ratings.size());
    EXPECT_EQ(196, ratings[pplib::String("")]);
}

TEST_F(ID3TagTest, UserDefinedTextGeneric)
{
    pplib::ID3Tag tag;
    tag.setUserDefinedText("CustomField", "MyValue");
    EXPECT_EQ(pplib::String("MyValue"), tag.getUserDefinedText("CustomField"));

    tag.setUserDefinedText("CustomField", "UpdatedValue");
    EXPECT_EQ(pplib::String("UpdatedValue"), tag.getUserDefinedText("CustomField"));

    const pplib::ID3Tag& ctag = tag;
    const pplib::ID3Frame* cf = ctag.findUserDefinedText("CustomField");
    EXPECT_NE(nullptr, cf);
    EXPECT_EQ(nullptr, ctag.findUserDefinedText("DoesNotExist"));
    EXPECT_EQ(pplib::String(), ctag.getUserDefinedText("DoesNotExist"));
}

TEST_F(ID3TagTest, WaveLoadFileWithoutTags)
{
    pplib::ID3Tag Tags;
    ASSERT_NO_THROW({ Tags.load("testdata/audio/test_44kHz.wav"); });
    EXPECT_EQ((size_t)0, Tags.frameCount());
}

TEST_F(ID3TagTest, WaveInitialTaggingAndRetagging)
{
    pplib::ID3Tag Tags;
    pplib::ByteArray cover;
    pplib::File::load(cover, "testdata/audio/cover.jpg");
    pplib::File::copy("testdata/audio/test_44kHz.wav", "tmp/test_tagged1.wav");

    ASSERT_NO_THROW({ Tags.load("tmp/test_tagged1.wav"); });
    EXPECT_EQ((size_t)0, Tags.frameCount());

    Tags.setArtist("Patrick Fedick");
    Tags.setTitle("Wave Test");
    Tags.setGenre("Rock");
    Tags.setAlbum("PPL Audio");
    Tags.setYear("2026");
    Tags.setTrack("1/5");
    Tags.setComment("WAV Comment");
    Tags.setPicture(3, cover, "image/jpeg");
    Tags.setUserDefinedText("MyCustomWavTag", "WavValue");

    ASSERT_NO_THROW({ Tags.save(); });

    // Reload and verify
    pplib::ID3Tag Loaded;
    ASSERT_NO_THROW({ Loaded.load("tmp/test_tagged1.wav"); });
    EXPECT_EQ(pplib::String("Patrick Fedick"), Loaded.getArtist());
    EXPECT_EQ(pplib::String("Wave Test"), Loaded.getTitle());
    EXPECT_EQ(pplib::String("Rock"), Loaded.getGenre());
    EXPECT_EQ(pplib::String("PPL Audio"), Loaded.getAlbum());
    EXPECT_EQ(pplib::String("2026"), Loaded.getYear());
    EXPECT_EQ(pplib::String("1/5"), Loaded.getTrack());
    EXPECT_EQ(pplib::String("WAV Comment"), Loaded.getComment());
    EXPECT_EQ(pplib::String("WavValue"), Loaded.getUserDefinedText("MyCustomWavTag"));
    EXPECT_TRUE(Loaded.hasPicture(3));
    pplib::ByteArray loadedCover;
    EXPECT_TRUE(Loaded.getPicture(3, loadedCover));
    EXPECT_EQ(cover.size(), loadedCover.size());
    EXPECT_TRUE(cover == loadedCover);

    // Remove picture and re-save
    Loaded.removePicture(3);
    ASSERT_NO_THROW({ Loaded.save(); });

    pplib::ID3Tag LoadedNoPic;
    ASSERT_NO_THROW({ LoadedNoPic.load("tmp/test_tagged1.wav"); });
    EXPECT_FALSE(LoadedNoPic.hasPicture(3));
    EXPECT_EQ(pplib::String("Patrick Fedick"), LoadedNoPic.getArtist());

    // Remove all tags and re-save
    LoadedNoPic.clearTags();
    ASSERT_NO_THROW({ LoadedNoPic.save(); });

    pplib::ID3Tag EmptyTags;
    ASSERT_NO_THROW({ EmptyTags.load("tmp/test_tagged1.wav"); });
    EXPECT_EQ((size_t)0, EmptyTags.frameCount());
}

TEST_F(ID3TagTest, WaveUpperId3Chunk)
{
    // Tag a wave file, then rename chunk 'id3 ' to 'ID3 ' in the raw file
    pplib::File::copy("testdata/audio/test_44kHz.wav", "tmp/test_tagged_upper.wav");
    pplib::ID3Tag tag;
    tag.load("tmp/test_tagged_upper.wav");
    tag.setTitle("UpperID3Test");
    tag.save();

    // Find "id3 " and change to "ID3 " in the file
    pplib::File f;
    f.open("tmp/test_tagged_upper.wav", pplib::File::FileMode::READWRITE);
    pplib::ByteArray content;
    f.read(content, f.size());
    bool found = false;
    for (size_t i = 12; i + 4 <= content.size(); ++i) {
        if (memcmp((const char*)content.ptr() + i, "id3 ", 4) == 0) {
            f.seek(i);
            f.write("ID3 ", 4);
            found = true;
            break;
        }
    }
    f.close();
    ASSERT_TRUE(found);

    // Verify loading recognises uppercase "ID3 "
    pplib::ID3Tag tagUpper;
    ASSERT_NO_THROW({ tagUpper.load("tmp/test_tagged_upper.wav"); });
    EXPECT_EQ(pplib::String("UpperID3Test"), tagUpper.getTitle());

    // Re-save and verify it doesn't duplicate the chunk
    tagUpper.setTitle("UpperID3TestUpdated");
    ASSERT_NO_THROW({ tagUpper.save(); });

    pplib::ID3Tag tagVerify;
    ASSERT_NO_THROW({ tagVerify.load("tmp/test_tagged_upper.wav"); });
    EXPECT_EQ(pplib::String("UpperID3TestUpdated"), tagVerify.getTitle());
    EXPECT_EQ((size_t)1, tagVerify.frameCount());
}

TEST_F(ID3TagTest, ConstructorWithFilenameAndPadding)
{
    pplib::ID3Tag tag("testdata/audio/test_44kHz.aiff");
    EXPECT_EQ((size_t)0, tag.frameCount());
    tag.setPaddingSize(2048);
    tag.setPaddingSpace(256);
    tag.setMaxPaddingSpace(4096);
    tag.setLocalCharset("UTF-8");
}

TEST_F(ID3TagTest, TryLoadFiles)
{
    pplib::ID3Tag tag;
    EXPECT_TRUE(tag.tryLoad("testdata/audio/test_44kHz.aiff"));
    EXPECT_FALSE(tag.tryLoad("testdata/audio/non_existing_file.mp3"));

    pplib::File f;
    f.open("testdata/audio/test_44kHz.aiff", pplib::File::FileMode::READ);
    EXPECT_TRUE(tag.tryLoad(f));
    f.close();

    pplib::MemFile mf;
    EXPECT_FALSE(tag.tryLoad(mf));
}

TEST_F(ID3TagTest, ExceptionsAndErrorHandling)
{
    pplib::ID3Tag tag;
    EXPECT_THROW(tag.save(), pplib::FilenameNotSetException);

    // Unsupported audio format (size >= 1024 so IdentMPEG doesn't overflow map)
    pplib::ByteArray dummy;
    dummy.calloc(2048);
    pplib::File::save(dummy.ptr(), dummy.size(), "tmp/invalid_format.bin");
    EXPECT_THROW(tag.load("tmp/invalid_format.bin"), pplib::UnsupportedAudioFormatException);

    unsigned char badVersionTag[] = {'I', 'D', '3', 5, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    pplib::ByteArray mp3;
    pplib::File::load(mp3, "testdata/audio/test_192cbr.mp3");
    mp3.prepend(badVersionTag, sizeof(badVersionTag));
    pplib::File::save(mp3.ptr(), mp3.size(), "tmp/test_bad_version.mp3");
    EXPECT_THROW(tag.load("tmp/test_bad_version.mp3"), pplib::UnsupportedID3TagVersionException);
}

TEST_F(ID3TagTest, Id3v23LoadingAndExtendedHeader)
{
    unsigned char tagData[] = {'I', 'D', '3', 3,   0,   0x40, 0, 0, 0, 25, 0, 0, 0, 6,   0,   0,   0,  0,
                               0,   0,   'T', 'I', 'T', '2',  0, 0, 0, 5,  0, 0, 0, 'T', 'e', 's', 't'};
    pplib::ByteArray mp3;
    pplib::File::load(mp3, "testdata/audio/test_192cbr.mp3");
    mp3.prepend(tagData, sizeof(tagData));
    pplib::File::save(mp3.ptr(), mp3.size(), "tmp/test_v23_ext.mp3");

    pplib::ID3Tag tag;
    ASSERT_NO_THROW({ tag.load("tmp/test_v23_ext.mp3"); });
    EXPECT_EQ(pplib::String("Test"), tag.getTitle());
}

TEST_F(ID3TagTest, FrameUnsynchronisationFlagV24)
{
    unsigned char tagData[] = {'I',  'D',  '3', 4, 0, 0, 0, 0, 0, 13, 'T', 'E', 'S', 'T', 0, 0, 0, 3, 0, 2, // Flags = 0x02
                               0xFF, 0x00, 'Z'};
    pplib::ByteArray mp3;
    pplib::File::load(mp3, "testdata/audio/test_192cbr.mp3");
    mp3.prepend(tagData, sizeof(tagData));
    pplib::File::save(mp3.ptr(), mp3.size(), "tmp/test_frame_unsync.mp3");

    pplib::ID3Tag tag;
    ASSERT_NO_THROW({ tag.load("tmp/test_frame_unsync.mp3"); });
    pplib::ID3Frame* f = tag.findFrame("TEST");
    ASSERT_NE(nullptr, f);
    EXPECT_EQ(0, f->flags() & 2);
    EXPECT_EQ((size_t)2, f->size());
    EXPECT_EQ((unsigned char)0xFF, (unsigned char)f->dataPtr()[0]);
    EXPECT_EQ('Z', f->dataPtr()[1]);
}

TEST_F(ID3TagTest, ListFramesAndRemoveFrame)
{
    pplib::ID3Tag tag;
    tag.setTitle("Sample");
    tag.setArtist("Band");
    testing::internal::CaptureStdout();
    tag.listFrames(false);
    pplib::String output = testing::internal::GetCapturedStdout();

    EXPECT_FALSE(output.isEmpty());
    EXPECT_TRUE(output.has("TIT2"));
    EXPECT_TRUE(output.has("TPE1"));
    EXPECT_FALSE(output.has("HEXDUMP"));

    testing::internal::CaptureStdout();
    tag.listFrames(true);
    output = testing::internal::GetCapturedStdout();
    EXPECT_FALSE(output.isEmpty());
    EXPECT_TRUE(output.has("TIT2"));
    EXPECT_TRUE(output.has("TPE1"));
    EXPECT_TRUE(output.has("HEXDUMP"));

    tag.removeFrame(nullptr);
    pplib::ID3Frame externalFrame("COMM");
    tag.removeFrame(&externalFrame);
    EXPECT_EQ((size_t)2, tag.frameCount());
}

TEST_F(ID3TagTest, TextEncodings)
{
    pplib::ID3Tag tag;
    tag.setTextFrame("TIT2", "UTF8-Title", pplib::ID3Tag::ENC_UTF8);
    EXPECT_EQ(pplib::String("UTF8-Title"), tag.getTitle());

    tag.setTextFrame("TIT2", "ISO-Title", pplib::ID3Tag::ENC_ISO88591);
    EXPECT_EQ(pplib::String("ISO-Title"), tag.getTitle());

    tag.setTextFrame("TIT2", "ASCII-Title", pplib::ID3Tag::ENC_USASCII);
    EXPECT_EQ(pplib::String("ASCII-Title"), tag.getTitle());

    tag.setTextFrame("TIT2", "UTF16-Title", pplib::ID3Tag::ENC_UTF16);
    EXPECT_EQ(pplib::String("UTF16-Title"), tag.getTitle());

    pplib::ID3Frame rawFrame("TIT2");
    rawFrame.setData("RawFallbackText", 15);
    pplib::String decoded;
    pplib::ID3Tag::decode(&rawFrame, 0, 35, decoded, "UTF-8");
    EXPECT_EQ(pplib::String("RawFallbackText"), decoded);
}

TEST_F(ID3TagTest, GetGenreVariations)
{
    pplib::ID3Tag tag;
    tag.setTextFrame("TCON", "(17)");
    EXPECT_EQ(pplib::GetID3GenreName(17), tag.getGenre());

    tag.setTextFrame("TCON", "17");
    EXPECT_EQ(pplib::GetID3GenreName(17), tag.getGenre());

    tag.setTextFrame("TCON", "SynthPop");
    EXPECT_EQ(pplib::String("SynthPop"), tag.getGenre());
}

TEST_F(ID3TagTest, GetYearDateVariations)
{
    pplib::ID3Tag tag;
    tag.setTextFrame("TDRC", "2026/08/15");
    EXPECT_EQ(pplib::String("2026"), tag.getYear());

    tag.setTextFrame("TDRC", "2026T14:30:00");
    EXPECT_EQ(pplib::String("2026"), tag.getYear());
}

TEST_F(ID3TagTest, PrivateDataAndPictures)
{
    pplib::ID3Tag tag;
    pplib::ByteArray privData;
    privData.copy("priv_data", 9);
    pplib::ByteArray frameContent;
    frameContent.copy("MyIdentifier", 12);
    char nullByte = 0;
    frameContent.append(&nullByte, 1);
    frameContent.append(privData);

    pplib::ID3Frame privFrame("PRIV", 0, frameContent);
    tag.addFrame(privFrame);

    pplib::ByteArray loadedPriv;
    EXPECT_TRUE(tag.getPrivateData(loadedPriv, "MyIdentifier"));
    EXPECT_EQ(privData, loadedPriv);
    EXPECT_FALSE(tag.getPrivateData(loadedPriv, "WrongId"));

    pplib::ByteArray cover1, cover2;
    cover1.copy("Cover1Data", 10);
    cover2.copy("Cover2DataNew", 13);
    tag.setPicture(3, cover1, "image/jpeg");
    EXPECT_EQ((size_t)10, tag.getPicture(3).size());

    tag.setPicture(3, cover2, "image/png");
    EXPECT_EQ((size_t)13, tag.getPicture(3).size());
    EXPECT_EQ(cover2, tag.getPicture(3));
}

TEST_F(ID3TagTest, PopularimeterEdgeCases)
{
    pplib::ID3Tag tag;
    EXPECT_FALSE(tag.hasPopularimeter());
    EXPECT_FALSE(tag.hasPopularimeter("test@example.com"));
    EXPECT_EQ(0, tag.getPopularimeter());
    EXPECT_EQ(0, tag.getPopularimeter("test@example.com"));

    tag.setPopularimeter("user1@example.com", 100);
    tag.setPopularimeter("user2@example.com", 200);
    EXPECT_TRUE(tag.hasPopularimeter("user1@example.com"));
    EXPECT_EQ(100, tag.getPopularimeter("user1@example.com"));

    // Update existing email
    tag.setPopularimeter("user1@example.com", 150);
    EXPECT_EQ(150, tag.getPopularimeter("user1@example.com"));

    // Add a non-POPM frame to test non-POPM skip in removePopularimeter
    tag.setTitle("PopularimeterTest");
    tag.removePopularimeter();
    EXPECT_FALSE(tag.hasPopularimeter());
    EXPECT_EQ((size_t)1, tag.frameCount());
    EXPECT_EQ(pplib::String("PopularimeterTest"), tag.getTitle());
}

TEST_F(ID3TagTest, GenerateId3v1EdgeCases)
{
    pplib::ID3Tag tag;
    pplib::ByteArray v1, v2;
    tag.generateId3V1Tag(v1);
    tag.generateId3V2Tag(v2);
    EXPECT_TRUE(v1.isEmpty());
    EXPECT_TRUE(v2.isEmpty());

    tag.setTextFrame("TIT2", "UTF8Title", pplib::ID3Tag::ENC_UTF8);
    tag.setComment("StandardComment");
    tag.setTextFrame("TCON", "Rock");

    tag.generateId3V1Tag(v1);
    EXPECT_FALSE(v1.isEmpty());
    EXPECT_EQ(0, memcmp((const char*)v1.ptr() + 3, "UTF8Title", 9));
    EXPECT_EQ(0, memcmp((const char*)v1.ptr() + 97, "StandardComment", 15));
    EXPECT_EQ((unsigned char)255, ((const unsigned char*)v1.ptr())[127]);

    // Frame with encoding 2 (UTF-16BE) returns empty string for v1
    pplib::ID3Tag tagEnc2;
    pplib::ID3Frame frameEnc2("TIT2");
    char enc2Data[] = {2, 0, 'X'};
    frameEnc2.setData(enc2Data, sizeof(enc2Data));
    tagEnc2.addFrame(frameEnc2);
    pplib::ByteArray v1Enc2;
    tagEnc2.generateId3V1Tag(v1Enc2);
    EXPECT_EQ(0, ((const char*)v1Enc2.ptr())[3]);
}

TEST_F(ID3TagTest, WaveSaveTagTooSmallForExistingChunk)
{
    pplib::File::copy("testdata/audio/test_44kHz.wav", "tmp/test_wave_large_chunk.wav");
    pplib::ID3Tag tagLarge;
    tagLarge.load("tmp/test_wave_large_chunk.wav");
    tagLarge.setPaddingSize(10000);
    tagLarge.setTitle("LargePaddingTitle");
    tagLarge.save();

    pplib::ID3Tag tagShrink;
    tagShrink.load("tmp/test_wave_large_chunk.wav");
    tagShrink.setPaddingSize(512);
    tagShrink.setTitle("Shrunk");
    tagShrink.save();

    pplib::ID3Tag tagVerify;
    tagVerify.load("tmp/test_wave_large_chunk.wav");
    EXPECT_EQ(pplib::String("Shrunk"), tagVerify.getTitle());

    // Test growing the tag larger than existing chunk (chunkSize < tagV2.size() -> break)
    tagShrink.setPaddingSize(5000);
    tagShrink.setTitle("EnlargedTag");
    tagShrink.save();

    pplib::ID3Tag tagVerify2;
    tagVerify2.load("tmp/test_wave_large_chunk.wav");
    EXPECT_EQ(pplib::String("EnlargedTag"), tagVerify2.getTitle());
}

TEST_F(ID3TagTest, Mp3TruncateV1TagOnRemoval)
{
    pplib::File::copy("testdata/audio/test_192cbr_tagged.mp3", "tmp/test_trunc_v1.mp3");
    pplib::ID3Tag tag;
    tag.load("tmp/test_trunc_v1.mp3");
    tag.clearTags();
    tag.save();

    pplib::ID3Tag reloaded;
    reloaded.load("tmp/test_trunc_v1.mp3");
    EXPECT_EQ((size_t)0, reloaded.frameCount());
}

TEST_F(ID3TagTest, OddChunkPaddingAiffAndWave)
{
    unsigned char aiffHeader[] = {'F', 'O', 'R', 'M', 0, 0, 0, 17, 'A', 'I', 'F', 'F'};
    unsigned char aiffChunk[] = {'N', 'O', 'P', 'E', 0, 0, 0, 5, 'H', 'e', 'l', 'l', 'o'};
    pplib::ByteArray aiffData;
    aiffData.copy(aiffHeader, sizeof(aiffHeader));
    aiffData.append(aiffChunk, sizeof(aiffChunk));
    pplib::File::save(aiffData.ptr(), aiffData.size(), "tmp/odd_chunk.aiff");

    pplib::ID3Tag tagAiff;
    ASSERT_NO_THROW({ tagAiff.load("tmp/odd_chunk.aiff"); });
    tagAiff.setTitle("OddAiff");
    ASSERT_NO_THROW({ tagAiff.save(); });

    pplib::ID3Tag reloadedAiff;
    ASSERT_NO_THROW({ reloadedAiff.load("tmp/odd_chunk.aiff"); });
    EXPECT_EQ(pplib::String("OddAiff"), reloadedAiff.getTitle());

    unsigned char waveHeader[] = {'R', 'I', 'F', 'F', 17, 0, 0, 0, 'W', 'A', 'V', 'E'};
    unsigned char waveChunk[] = {'n', 'o', 'p', 'e', 5, 0, 0, 0, 'H', 'e', 'l', 'l', 'o'};
    pplib::ByteArray waveData;
    waveData.copy(waveHeader, sizeof(waveHeader));
    waveData.append(waveChunk, sizeof(waveChunk));
    pplib::File::save(waveData.ptr(), waveData.size(), "tmp/odd_chunk.wav");

    pplib::ID3Tag tagWave;
    ASSERT_NO_THROW({ tagWave.load("tmp/odd_chunk.wav"); });
    tagWave.setTitle("OddWave");
    ASSERT_NO_THROW({ tagWave.save(); });

    pplib::ID3Tag reloadedWave;
    ASSERT_NO_THROW({ reloadedWave.load("tmp/odd_chunk.wav"); });
    EXPECT_EQ(pplib::String("OddWave"), reloadedWave.getTitle());
}

TEST_F(ID3TagTest, ZeroFrameSizeAndEmptyFrames)
{
    unsigned char tagData[] = {'I', 'D', '3', 4, 0, 0, 0, 0, 0, 10, 'T', 'I', 'T', '2', 0, 0, 0, 0, 0, 0};
    pplib::ByteArray mp3;
    pplib::File::load(mp3, "testdata/audio/test_192cbr.mp3");
    mp3.prepend(tagData, sizeof(tagData));
    pplib::File::save(mp3.ptr(), mp3.size(), "tmp/test_zero_framesize.mp3");

    pplib::ID3Tag tag;
    ASSERT_NO_THROW({ tag.load("tmp/test_zero_framesize.mp3"); });
    EXPECT_EQ((size_t)0, tag.frameCount());

    pplib::ID3Tag tagInvalid;
    tagInvalid.addFrame(pplib::ID3Frame(""));
    pplib::ByteArray v2;
    tagInvalid.generateId3V2Tag(v2);
    EXPECT_TRUE(v2.isEmpty());

    EXPECT_EQ(nullptr, tagInvalid.findUserDefinedText("NonExistent"));
}

TEST_F(ID3TagTest, SaveMP3ErrorHandling)
{
    pplib::File::copy("testdata/audio/test_192cbr.mp3", "tmp/corrupt_on_save.mp3");
    pplib::ID3Tag tag;
    tag.load("tmp/corrupt_on_save.mp3");

    pplib::ByteArray dummy;
    dummy.calloc(2048);
    pplib::File::save(dummy.ptr(), dummy.size(), "tmp/corrupt_on_save.mp3");

    EXPECT_THROW(tag.save(), pplib::UnsupportedAudioFormatException);

    pplib::ID3Tag tagUnknown;
    tagUnknown.tryLoad("tmp/corrupt_on_save.mp3");
    EXPECT_THROW(tagUnknown.save(), pplib::UnsupportedAudioFormatException);
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
