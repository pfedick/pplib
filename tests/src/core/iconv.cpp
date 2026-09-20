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
#include <pplib/types/widestring.h>
#include <pplib/types/array.h>
#include <pplib/types/bytearray.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>
#include <pplib/core/iconv.h>
#include <algorithm>
#include "pplib-tests.h"

namespace
{

class IconvTest : public ::testing::Test
{
protected:
    IconvTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~IconvTest()
    {
    }
    virtual void TearDown() override
    {
        setlocale(LC_CTYPE, DEFAULT_LOCALE);
    }
};

TEST_F(IconvTest, ConstructorWithoutFile)
{
    ASSERT_NO_THROW({ pplib::Iconv iconv; });
}

TEST_F(IconvTest, enumerateCharsets)
{
    pplib::Array list;
    pplib::Iconv::enumerateCharsets(list);
    ASSERT_GT(list.size(), (size_t)10);
    // list.list();
    ASSERT_TRUE(list.has("US-ASCII"));
    ASSERT_TRUE(list.has("UTF-8"));
    ASSERT_TRUE(list.has("LATIN1"));
    ASSERT_TRUE(list.has("UTF-16"));
    ASSERT_TRUE(list.has("UTF-32"));
}

static bool list_has(const std::list<pplib::String>& list, const pplib::String& value)
{
    return std::find(list.begin(), list.end(), value) != list.end();
}

TEST_F(IconvTest, enumerateCharsetsStdList)
{
    std::list<pplib::String> list;
    pplib::Iconv::enumerateCharsets(list);
    ASSERT_GT(list.size(), (size_t)10);
    // list.list();

    ASSERT_TRUE(list_has(list, "US-ASCII"));
    ASSERT_TRUE(list_has(list, "UTF-8"));
    ASSERT_TRUE(list_has(list, "LATIN1"));
    ASSERT_TRUE(list_has(list, "UTF-16"));
    ASSERT_TRUE(list_has(list, "UTF-32"));
}

TEST_F(IconvTest, getLocalCharset)
{
    ASSERT_EQ(pplib::String("UTF-8"), pplib::Iconv::getLocalCharset());
}

TEST_F(IconvTest, initWithKnownCharsets)
{
    pplib::Iconv iconv;
    ASSERT_NO_THROW({ iconv.init("ISO-8859-1", "UTF-8"); });
}

TEST_F(IconvTest, reInit)
{
    pplib::Iconv iconv;
    ASSERT_NO_THROW({ iconv.init("ISO-8859-1", "UTF-8"); });
    ASSERT_NO_THROW({ iconv.init("UTF-8", "ISO-8859-1"); });
}

TEST_F(IconvTest, initWithUnknownCharsetsThrowsException)
{
    pplib::Iconv iconv;
    ASSERT_THROW({ iconv.init("ISO-PATRICKF", "UTF-8"); }, pplib::UnsupportedCharacterEncodingException);
}

static const char test_string_iso88591[] = {(char)0x48, (char)0xe4, (char)0x6c, (char)0x6c, (char)0x6f, (char)0x20,
                                            (char)0x57, (char)0xf6, (char)0x72, (char)0x6c, (char)0x64};

static const char test_string_utf16be[] = {(char)0x00, (char)0x48, (char)0x00, (char)0xe4, (char)0x00, (char)0x6c, (char)0x00, (char)0x6c,
                                           (char)0x00, (char)0x6f, (char)0x00, (char)0x20, (char)0x00, (char)0x57, (char)0x00, (char)0xf6,
                                           (char)0x00, (char)0x72, (char)0x00, (char)0x6c, (char)0x00, (char)0x64, (char)0x00, (char)0x21};

TEST_F(IconvTest, transcodeWithString_ISO88591_UTF8)
{
    pplib::Iconv iconv;
    iconv.init("ISO-8859-1", "UTF-8");
    pplib::String source(test_string_iso88591, sizeof(test_string_iso88591));
    pplib::String target;
    iconv.transcode(source, target);
    ASSERT_EQ(pplib::String("Hällo Wörld"), target);
}

TEST_F(IconvTest, transcodeByteArray_UTF16BE_UTF8)
{
    pplib::String expected("Hällo Wörld!");
    pplib::Iconv iconv;
    iconv.init("UTF-16BE", "UTF-8");
    pplib::ByteArray source(test_string_utf16be, sizeof(test_string_utf16be));
    pplib::ByteArray target;
    iconv.transcode(source, target);
    ASSERT_EQ(pplib::ByteArray(expected), target);
}

TEST_F(IconvTest, transcodeWithByteArray)
{
    pplib::Iconv iconv;
    pplib::String expected("Hällo Wörld");
    iconv.init("ISO-8859-1", "UTF-8");
    pplib::ByteArray source(test_string_iso88591, sizeof(test_string_iso88591));
    pplib::ByteArray target;
    iconv.transcode(source, target);
    ASSERT_EQ(pplib::ByteArray(expected), target);
}

TEST_F(IconvTest, transcodeThrowsCharacterEncodingNotInitializedException)
{
    pplib::Iconv iconv;
    pplib::ByteArray source(test_string_iso88591, sizeof(test_string_iso88591));
    pplib::ByteArray target;
    ASSERT_THROW(iconv.transcode(source, target), pplib::CharacterEncodingNotInitializedException);
}

TEST_F(IconvTest, transcodeThrowsCharacterEncodingException)
{
    pplib::Iconv iconv;
    iconv.init("UTF-8", "ISO-8859-1");
    pplib::String source("Hällo Wörld");
    pplib::ByteArray target;
    // Intentionally corrupt the source to trigger a CharacterEncodingException
    source[0] = '\xFF'; // Invalid UTF-8 byte

    ASSERT_THROW(iconv.transcode(source, target), pplib::CharacterEncodingException);
}

TEST_F(IconvTest, transcodeStringToString)
{
    pplib::Iconv iconv;
    iconv.init("ISO-8859-1", "UTF-8");
    pplib::String source(test_string_iso88591, sizeof(test_string_iso88591));
    pplib::String target = iconv.transcode(source);
    ASSERT_EQ(pplib::String("Hällo Wörld"), target);
}

TEST_F(IconvTest, transcodeByteArrayPtrToByteArray)
{
    pplib::Iconv iconv;
    iconv.init("ISO-8859-1", "UTF-8");
    pplib::ByteArrayPtr source(test_string_iso88591, sizeof(test_string_iso88591));
    pplib::ByteArray target;
    target = iconv.transcode(source);
    ASSERT_EQ(pplib::ByteArray("Hällo Wörld"), target);
}

TEST_F(IconvTest, ISO88591toUtf8)
{
    // Wir nehmen ein ByteArray mit einem echten ISO-8859-1 Umlaut ("äöü")
    // In iso-8859-1: ä = 0xe4, ö = 0xf6, ü = 0xfc
    const unsigned char latin1_data[] = {0xe4, 0xf6, 0xfc, 0};
    pplib::ByteArray latin1_bytes(latin1_data, 3);

    // Nutze Deine neue, geniale Iconv::transcode Methode!
    pplib::ByteArray utf8_bytes = pplib::Iconv::transcode(latin1_bytes, "ISO-8859-1", "UTF-8");

    // "äöü" in UTF-8 belegt genau 6 Bytes
    ASSERT_EQ((size_t)6, utf8_bytes.size());

    // Die UTF-8 Byte-Prüfung:
    // ä = \xC3\xA4 (195, 164)
    // ö = \xC3\xB6 (195, 182)
    // ü = \xC3\xBC (195, 188)
    EXPECT_EQ(195, (unsigned char)utf8_bytes.get(0));
    EXPECT_EQ(164, (unsigned char)utf8_bytes.get(1));
    EXPECT_EQ(195, (unsigned char)utf8_bytes.get(2));
    EXPECT_EQ(182, (unsigned char)utf8_bytes.get(3));
    EXPECT_EQ(195, (unsigned char)utf8_bytes.get(4));
    EXPECT_EQ(188, (unsigned char)utf8_bytes.get(5));
}

TEST_F(IconvTest, transcodeUtf8_to_ISO88591)
{

    pplib::String source("Hällo Wörld");
    pplib::ByteArray target = pplib::Iconv::transcode("Hällo Wörld", "UTF-8", "ISO-8859-1");
    ASSERT_EQ((size_t)11, target.size()); // "Hällo Wörld" in ISO-8859-1 should be 11 bytes
    ASSERT_EQ('H', (unsigned char)target.get(0));
    ASSERT_EQ(228, (unsigned char)target.get(1)); // ä
    ASSERT_EQ('l', (unsigned char)target.get(2));
    ASSERT_EQ('l', (unsigned char)target.get(3));
    ASSERT_EQ('o', (unsigned char)target.get(4));
    ASSERT_EQ(' ', (unsigned char)target.get(5));
    ASSERT_EQ('W', (unsigned char)target.get(6));
    ASSERT_EQ(246, (unsigned char)target.get(7)); // ö
    ASSERT_EQ('r', (unsigned char)target.get(8));
    ASSERT_EQ('l', (unsigned char)target.get(9));
    ASSERT_EQ('d', (unsigned char)target.get(10));
}

TEST_F(IconvTest, transcodeUtf8_to_UTF16BE)
{
    pplib::String source("Hällo Wörld");
    pplib::ByteArray target = pplib::Iconv::transcode(source, "UTF-8", "UTF-16BE");
    ASSERT_EQ((size_t)22, target.size()); // "Hällo Wörld" in UTF-16BE should be 22 bytes
    // Check the first few bytes for correctness
    ASSERT_EQ(0, (unsigned char)target.get(0));   // 'H' high byte
    ASSERT_EQ('H', (unsigned char)target.get(1)); // 'H' low byte
    ASSERT_EQ(0, (unsigned char)target.get(2));   // 'ä' high byte
    ASSERT_EQ(228, (unsigned char)target.get(3)); // 'ä' low byte
    ASSERT_EQ(0, (unsigned char)target.get(4));   // 'l' high byte
    ASSERT_EQ('l', (unsigned char)target.get(5)); // 'l' low byte
    ASSERT_EQ(0, (unsigned char)target.get(6));   // 'l' high byte
    ASSERT_EQ('l', (unsigned char)target.get(7)); // 'l' low byte
    ASSERT_EQ(0, (unsigned char)target.get(8));   // 'o' high byte
    ASSERT_EQ('o', (unsigned char)target.get(9)); // 'o' low byte
}

TEST_F(IconvTest, utf8ToLocal)
{
    setlocale(LC_CTYPE, LATIN1_LOCALE);
    pplib::String source("Hällo Wörld");
    pplib::String target = pplib::Iconv::utf8ToLocal(source);
    pplib::String expected(test_string_iso88591, sizeof(test_string_iso88591));
    ASSERT_EQ(expected, target);
}

TEST_F(IconvTest, localToUtf8)
{
    setlocale(LC_CTYPE, LATIN1_LOCALE);

    pplib::String source(test_string_iso88591, sizeof(test_string_iso88591));
    pplib::String target = pplib::Iconv::localToUtf8(source);
    ASSERT_EQ(pplib::String("Hällo Wörld"), target);
}

TEST_F(IconvTest, fromWideString)
{
    setlocale(LC_CTYPE, DEFAULT_LOCALE);
    pplib::WideString source(L"Hällo Wörld");
    pplib::String target = pplib::Iconv::fromWideString(source, "UTF-8");
    ASSERT_EQ(pplib::String("Hällo Wörld"), target);
}

TEST_F(IconvTest, toWideString)
{
    setlocale(LC_CTYPE, DEFAULT_LOCALE);
    pplib::String source("Hällo Wörld");
    pplib::WideString target = pplib::Iconv::toWideString(source, "UTF-8");
    ASSERT_EQ(pplib::WideString(L"Hällo Wörld"), target);
}

} // namespace
