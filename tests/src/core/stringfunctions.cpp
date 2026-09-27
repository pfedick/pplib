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
#include <pplib/core/stringfunctions.h>

#include "pplib-tests.h"

namespace
{

// The fixture for testing class Foo.
class StringFunctionTest : public ::testing::Test
{
protected:
    StringFunctionTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~StringFunctionTest()
    {
    }
};

// 0d,0e,37,42,81,ff,42,00,4c,17,12
static unsigned char binarydata[] = {13, 14, 55, 66, 129, 255, 66, 0, 76, 23, 18};

TEST_F(StringFunctionTest, ToBase64)
{
    pplib::ByteArrayPtr b2;
    b2.use(binarydata, 7);
    EXPECT_EQ(pplib::String("DQ43QoH/Qo=="), ToBase64(b2));
    b2.use(binarydata, 8);
    EXPECT_EQ(pplib::String("DQ43QoH/QgD="), ToBase64(b2));
    b2.use(binarydata, 9);
    EXPECT_EQ(pplib::String("DQ43QoH/QgBM"), ToBase64(b2));
    b2.use(binarydata, 10);
    EXPECT_EQ(pplib::String("DQ43QoH/QgBMFw=="), ToBase64(b2));
    b2.use(binarydata, 11);
    EXPECT_EQ(pplib::String("DQ43QoH/QgBMFxJ="), ToBase64(b2));
}

TEST_F(StringFunctionTest, FromBase64)
{
    pplib::ByteArrayPtr b1;
    try {
        b1.use(binarydata, 7);
        EXPECT_EQ(b1, pplib::FromBase64(pplib::String("DQ43QoH/Qo==")));
        b1.use(binarydata, 8);
        EXPECT_EQ(b1, pplib::FromBase64(pplib::String("DQ43QoH/QgD=")));
        b1.use(binarydata, 9);
        EXPECT_EQ(b1, pplib::FromBase64(pplib::String("DQ43QoH/QgBM")));
        b1.use(binarydata, 10);
        EXPECT_EQ(b1, pplib::FromBase64(pplib::String("DQ43QoH/QgBMFw==")));
        b1.use(binarydata, 11);
        EXPECT_EQ(b1, pplib::FromBase64(pplib::String("DQ43QoH/QgBMFxJ=")));
    }
    catch (const pplib::Exception& e) {
        e.print();
    }
    catch (const std::exception& e) {
        printf("std::exception: %s\n", e.what());
    }
}

TEST_F(StringFunctionTest, UpperCaseWords)
{
    pplib::String s1("the quick brown fox jumps over äöü");
    pplib::String expected("The Quick Brown Fox Jumps Over Äöü");
    pplib::String result = UpperCaseWords(s1);
    ASSERT_EQ(expected, result);
}

TEST_F(StringFunctionTest, StripSlashes)
{
    EXPECT_EQ(pplib::String(""), pplib::StripSlashes(pplib::String("")));
    EXPECT_EQ(pplib::String("Hallo Welt"), pplib::StripSlashes(pplib::String("Hallo Welt")));
    EXPECT_EQ(pplib::String("Hallo Welt"), pplib::StripSlashes(pplib::String("Hallo\\ Welt")));
    EXPECT_EQ(pplib::String("Hallon Welt"), pplib::StripSlashes(pplib::String("Hallo\\n Welt")));
    EXPECT_EQ(pplib::String("Hallo\\Welt"), pplib::StripSlashes(pplib::String("Hallo\\\\Welt")));
    EXPECT_EQ(pplib::String("Hallo Welt"), pplib::StripSlashes(pplib::String("\\Hallo Welt")));
    EXPECT_EQ(pplib::String("Hallo Welt"), pplib::StripSlashes(pplib::String("Hallo Welt\\")));
}

TEST_F(StringFunctionTest, Repeat)
{
    EXPECT_EQ(pplib::String(""), pplib::Repeat(pplib::String("abc"), 0));
    EXPECT_EQ(pplib::String(""), pplib::Repeat(pplib::String(""), 5));
    EXPECT_EQ(pplib::String("abc"), pplib::Repeat(pplib::String("abc"), 1));
    EXPECT_EQ(pplib::String("abcabcabc"), pplib::Repeat(pplib::String("abc"), 3));
}

TEST_F(StringFunctionTest, Trim)
{
    EXPECT_EQ(pplib::String(""), pplib::Trim(pplib::String("")));
    EXPECT_EQ(pplib::String(""), pplib::Trim(pplib::String("   \t\r\n  ")));
    EXPECT_EQ(pplib::String("hello"), pplib::Trim(pplib::String("  hello  ")));
    EXPECT_EQ(pplib::String("hello world"), pplib::Trim(pplib::String("\t\nhello world\r\n")));
}

TEST_F(StringFunctionTest, UpperCaseAndLowerCase)
{
    EXPECT_EQ(pplib::String("HELLO WORLD"), pplib::UpperCase(pplib::String("hello world")));
    EXPECT_EQ(pplib::String("hello world"), pplib::LowerCase(pplib::String("HELLO WORLD")));
    EXPECT_EQ(pplib::String(""), pplib::UpperCase(pplib::String("")));
    EXPECT_EQ(pplib::String(""), pplib::LowerCase(pplib::String("")));
}

TEST_F(StringFunctionTest, StrCmpAndStrCaseCmp)
{
    pplib::String a("apple");
    pplib::String b("banana");
    pplib::String aUpper("APPLE");

    EXPECT_EQ(0, pplib::StrCmp(a, a));
    EXPECT_EQ(-1, pplib::StrCmp(a, b));
    EXPECT_EQ(1, pplib::StrCmp(b, a));

    EXPECT_NE(0, pplib::StrCmp(a, aUpper));
    EXPECT_EQ(0, pplib::StrCaseCmp(a, aUpper));
    EXPECT_EQ(-1, pplib::StrCaseCmp(a, b));
    EXPECT_EQ(1, pplib::StrCaseCmp(b, a));
}

TEST_F(StringFunctionTest, InstrAndInstrCaseString)
{
    pplib::String haystack("The quick brown fox jumps over the lazy dog");

    // Case sensitive
    EXPECT_EQ(4, pplib::Instr(haystack, pplib::String("quick")));
    EXPECT_EQ(0, pplib::Instr(haystack, pplib::String("The")));
    EXPECT_EQ(31, pplib::Instr(haystack, pplib::String("the"), 0)); // 'the' lowercase is at 31
    EXPECT_EQ(31, pplib::Instr(haystack, pplib::String("the"), 5));
    EXPECT_EQ(-1, pplib::Instr(haystack, pplib::String("Dog"))); // only "dog" (lowercase) exists
    EXPECT_EQ(-1, pplib::Instr(haystack, pplib::String("cat")));

    // Case insensitive
    EXPECT_EQ(0, pplib::InstrCase(haystack, pplib::String("the"), 0));
    EXPECT_EQ(31, pplib::InstrCase(haystack, pplib::String("the"), 5));
    EXPECT_EQ(16, pplib::InstrCase(haystack, pplib::String("FOX")));
    EXPECT_EQ(40, pplib::InstrCase(haystack, pplib::String("DOG")));
    EXPECT_EQ(-1, pplib::InstrCase(haystack, pplib::String("cat")));

    // Instrcase alias for String
    EXPECT_EQ(16, pplib::Instrcase(haystack, pplib::String("FOX")));
}

TEST_F(StringFunctionTest, InstrWithUtf8)
{
    const char* haystack = "Hällo Wörld";

    // Instr (case sensitive)
    EXPECT_EQ(0, pplib::Instr(haystack, "Häl", 0));
    EXPECT_EQ(7, pplib::Instr(haystack, "Wörld", 0));
    EXPECT_EQ(-1, pplib::Instr(haystack, "Hallo", 0));

    // Instrcase (case insensitive)
    EXPECT_EQ(0, pplib::Instrcase(haystack, "häl", 0));
    EXPECT_EQ(7, pplib::Instrcase(haystack, "wÖrld", 0));
    EXPECT_EQ(-1, pplib::Instrcase(haystack, "hallo", 0));
}

TEST_F(StringFunctionTest, InstrAndInstrcaseCharPtr)
{
    const char* haystack = "HelloWorldHello";

    // Instr (case sensitive)
    EXPECT_EQ(0, pplib::Instr(haystack, "Hello", 0));
    EXPECT_EQ(10, pplib::Instr(haystack, "Hello", 5));
    EXPECT_EQ(5, pplib::Instr(haystack, "World", 0));
    EXPECT_EQ(-1, pplib::Instr(haystack, "world", 0));
    EXPECT_EQ(-1, pplib::Instr(haystack, "xyz", 0));
    EXPECT_EQ(-1, pplib::Instr(haystack, "Hello", 20)); // start past length
    EXPECT_EQ(-1, pplib::Instr(nullptr, "Hello", 0));
    EXPECT_EQ(-1, pplib::Instr(haystack, nullptr, 0));

    // Instrcase (case insensitive)
    EXPECT_EQ(5, pplib::Instrcase(haystack, "world", 0));
    EXPECT_EQ(0, pplib::Instrcase(haystack, "hello", 0));
    EXPECT_EQ(10, pplib::Instrcase(haystack, "HELLO", 5));
    EXPECT_EQ(-1, pplib::Instrcase(haystack, "xyz", 0));
    EXPECT_EQ(-1, pplib::Instrcase(haystack, "Hello", 20));
    EXPECT_EQ(-1, pplib::Instrcase(nullptr, "hello", 0));
    EXPECT_EQ(-1, pplib::Instrcase(haystack, nullptr, 0));

    // Empty needle
    EXPECT_EQ(-1, pplib::Instr(haystack, "", 0));
    EXPECT_EQ(-1, pplib::Instr(haystack, "", 3));
    EXPECT_EQ(-1, pplib::Instrcase(haystack, "", 0));
    EXPECT_EQ(-1, pplib::Instrcase(haystack, "", 3));
}

TEST_F(StringFunctionTest, InstrAndInstrcaseWcharPtr)
{
    const wchar_t* haystack = L"WideHelloWorld";

    // Instr (case sensitive)
    EXPECT_EQ(4, pplib::Instr(haystack, L"Hello", 0));
    EXPECT_EQ(-1, pplib::Instr(haystack, L"hello", 0));
    EXPECT_EQ(-1, pplib::Instr(haystack, L"xyz", 0));
    EXPECT_EQ(-1, pplib::Instr(nullptr, L"Hello", 0));
    EXPECT_EQ(-1, pplib::Instr(haystack, nullptr, 0));
    EXPECT_EQ(-1, pplib::Instr(haystack, L"Hello", 20));

    // Instrcase (case insensitive)
    EXPECT_EQ(4, pplib::Instrcase(haystack, L"hello", 0));
    EXPECT_EQ(9, pplib::Instrcase(haystack, L"WORLD", 0));
    EXPECT_EQ(-1, pplib::Instrcase(haystack, L"XYZ", 0));
    EXPECT_EQ(-1, pplib::Instrcase(nullptr, L"hello", 0));
    EXPECT_EQ(-1, pplib::Instrcase(haystack, nullptr, 0));
    EXPECT_EQ(-1, pplib::Instrcase(haystack, L"hello", 20));

    // Empty needle
    EXPECT_EQ(-1, pplib::Instr(haystack, L"", 0));
    EXPECT_EQ(-1, pplib::Instr(haystack, L"", 2));
    EXPECT_EQ(-1, pplib::Instrcase(haystack, L"", 0));
    EXPECT_EQ(-1, pplib::Instrcase(haystack, L"", 2));
}

TEST_F(StringFunctionTest, SubstringHelpers)
{
    pplib::String s("0123456789");

    EXPECT_EQ(pplib::String("012"), pplib::Left(s, 3));
    EXPECT_EQ(pplib::String("0123456789"), pplib::Left(s, 20));
    EXPECT_EQ(pplib::String(""), pplib::Left(s, 0));

    EXPECT_EQ(pplib::String("789"), pplib::Right(s, 3));
    EXPECT_EQ(pplib::String("0123456789"), pplib::Right(s, 20));
    EXPECT_EQ(pplib::String(""), pplib::Right(s, 0));

    EXPECT_EQ(pplib::String("345"), pplib::Mid(s, 3, 3));
    EXPECT_EQ(pplib::String("3456789"), pplib::Mid(s, 3));

    EXPECT_EQ(pplib::String("345"), pplib::SubStr(s, 3, 3));
    EXPECT_EQ(pplib::String("3456789"), pplib::SubStr(s, 3));
}

TEST_F(StringFunctionTest, ToStringHelper)
{
    EXPECT_EQ(pplib::String("Number: 42, String: test"), pplib::ToString("Number: %d, String: %s", 42, "test"));
    EXPECT_EQ(pplib::String(""), pplib::ToString(nullptr));
}

TEST_F(StringFunctionTest, ReplaceHelper)
{
    pplib::String s("foo bar foo baz");
    EXPECT_EQ(pplib::String("qux bar qux baz"), pplib::Replace(s, "foo", "qux"));
    EXPECT_EQ(pplib::String("foo bar foo baz"), pplib::Replace(s, "nonexistent", "x"));
}

TEST_F(StringFunctionTest, TypeInspectionHelpers)
{
    // IsTrue
    EXPECT_TRUE(pplib::IsTrue(pplib::String("true")));
    EXPECT_TRUE(pplib::IsTrue(pplib::String("1")));
    EXPECT_TRUE(pplib::IsTrue(pplib::String("yes")));
    EXPECT_TRUE(pplib::IsTrue(pplib::String("t")));
    EXPECT_FALSE(pplib::IsTrue(pplib::String("false")));
    EXPECT_FALSE(pplib::IsTrue(pplib::String("0")));
    EXPECT_FALSE(pplib::IsTrue(pplib::String("no")));

    // IsDigits
    EXPECT_TRUE(pplib::IsDigits(pplib::String("1234567890")));
    EXPECT_FALSE(pplib::IsDigits(pplib::String("123a45")));
    EXPECT_FALSE(pplib::IsDigits(pplib::String("")));

    // IsInteger
    EXPECT_TRUE(pplib::IsInteger(pplib::String("123")));
    EXPECT_TRUE(pplib::IsInteger(pplib::String("+123")));
    EXPECT_TRUE(pplib::IsInteger(pplib::String("-456")));
    EXPECT_FALSE(pplib::IsInteger(pplib::String("12.34")));
    EXPECT_FALSE(pplib::IsInteger(pplib::String("abc")));

    // IsNumeric
    EXPECT_TRUE(pplib::IsNumeric(pplib::String("123.45")));
    EXPECT_TRUE(pplib::IsNumeric(pplib::String("-0.5")));
    EXPECT_FALSE(pplib::IsNumeric(pplib::String("abc")));
}

TEST_F(StringFunctionTest, StrTokHelper)
{
    // Return by value, default newline delimiter
    pplib::String multiline("line1\nline2\n\nline3\n");
    pplib::Array arr1 = pplib::StrTok(multiline);
    ASSERT_EQ(3u, arr1.size());
    EXPECT_EQ(pplib::String("line1"), arr1[0]);
    EXPECT_EQ(pplib::String("line2"), arr1[1]);
    EXPECT_EQ(pplib::String("line3"), arr1[2]);

    // Custom delimiter, multiple consecutive delimiters ignored
    pplib::String csv("one,,two;three,four");
    pplib::Array arr2;
    pplib::StrTok(arr2, csv, ",");
    ASSERT_EQ(3u, arr2.size());
    EXPECT_EQ(pplib::String("one"), arr2[0]);
    EXPECT_EQ(pplib::String("two;three"), arr2[1]);
    EXPECT_EQ(pplib::String("four"), arr2[2]);

    // Empty string
    pplib::Array arrEmpty = pplib::StrTok(pplib::String(""));
    EXPECT_EQ(0u, arrEmpty.size());
}

TEST_F(StringFunctionTest, EscapeAndUnescapeHTMLTags)
{
    pplib::String original("<div class=\"box\">&copy; 'A & B' > 5</div>");
    pplib::String escaped = pplib::EscapeHTMLTags(original);
    EXPECT_EQ(pplib::String("&lt;div class=\"box\"&gt;&amp;copy; 'A &amp; B' &gt; 5&lt;/div&gt;"), escaped);

    pplib::String unescaped = pplib::UnescapeHTMLTags(escaped);
    EXPECT_EQ(original, unescaped);

    // Test that &amp;lt; does NOT double-unescape to <
    pplib::String alreadyEscaped("&amp;lt;");
    EXPECT_EQ(pplib::String("&lt;"), pplib::UnescapeHTMLTags(alreadyEscaped));
}

TEST_F(StringFunctionTest, HexHelpers)
{
    const char rawBytes[] = {0x00, 0x12, (char)0xAB, (char)0xFF};
    pplib::ByteArray ba(rawBytes, sizeof(rawBytes));

    pplib::String hex = pplib::ToHex(ba);
    EXPECT_TRUE(hex.strCaseCmp("0012abff") == 0);

    pplib::ByteArray converted = pplib::Hex2ByteArray(hex);
    EXPECT_EQ(sizeof(rawBytes), converted.size());
    EXPECT_EQ(0, memcmp(rawBytes, converted.ptr(), sizeof(rawBytes)));
}

TEST_F(StringFunctionTest, UrlEncodeAndDecode)
{
    pplib::String text("Hallo Welt! 1+1=2");
    pplib::String encoded = pplib::UrlEncode(text);
    EXPECT_EQ(pplib::String("Hallo+Welt!+1%2B1%3D2"), encoded);

    pplib::String decoded = pplib::UrlDecode(encoded);
    EXPECT_EQ(text, decoded);

    // Test non-ASCII UTF-8 characters (the bug where HexPairValue failed on values >= 0x80)
    pplib::String utf8Text("Äpfel & Übergrößen");
    pplib::String utf8Encoded = pplib::UrlEncode(utf8Text);
    pplib::String utf8Decoded = pplib::UrlDecode(utf8Encoded);
    EXPECT_EQ(utf8Text, utf8Decoded);

    // Test decode edge cases (invalid hex pair, incomplete percent at end)
    EXPECT_EQ(pplib::String("test?"), pplib::UrlDecode(pplib::String("test%")));
    EXPECT_EQ(pplib::String("test?1"), pplib::UrlDecode(pplib::String("test%1")));
    EXPECT_EQ(pplib::String("test?ZZ"), pplib::UrlDecode(pplib::String("test%ZZ")));

    // Test lowercase hex characters
    EXPECT_EQ(pplib::String("a+b=c"), pplib::UrlDecode(pplib::String("a%2bb%3dc")));
}

TEST_F(StringFunctionTest, Transcode)
{
    // Transcode ASCII/Latin-1 between UTF-8 and ISO-8859-1
    pplib::String utf8Str("Hällö Wörld");
    pplib::String isoStr = pplib::Transcode(utf8Str, "UTF-8", "ISO-8859-1");
    EXPECT_NE(utf8Str, isoStr); // Byte representation differs

    pplib::String backToUtf8 = pplib::Transcode(isoStr, "ISO-8859-1", "UTF-8");
    EXPECT_EQ(utf8Str, backToUtf8);

    // Pointer overload
    pplib::String ptrTranscoded = pplib::Transcode(utf8Str.c_str(), utf8Str.size(), "UTF-8", "ISO-8859-1");
    EXPECT_EQ(isoStr, ptrTranscoded);
}

// ByteArray fromBase64(const String &base64);

} // namespace
