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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <gtest/gtest.h>

#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/array.h>
#include <pplib/exceptions.h>

#include "pplib-tests.h"

namespace
{

TEST(StringTest, ConstructorSimple)
{
    ASSERT_NO_THROW({
        pplib::String s1;
        ASSERT_TRUE(s1.len() == 0) << "String does not have zero length";
        const char* buf = s1.getPtr();
        ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
        ASSERT_EQ(0, buf[0]) << "Class did not return an empty c-string";
    });
}

TEST(StringTest, ConstructorFromCharPtr)
{
    ASSERT_NO_THROW({
        pplib::String s1("A test string with unicode characters: äöü");
        ASSERT_EQ((size_t)45, s1.len()) << "String does not have length of 42";
        const unsigned char* buf = (const unsigned char*)s1.getPtr();
        ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
        ASSERT_EQ('A', (unsigned char)s1[0]) << "Unexpected Character in string";
        ASSERT_EQ(188, (unsigned char)s1[44]) << "Unexpected Character in string";
    });
}

TEST(StringTest, ConstructorFromString)
{
    ASSERT_NO_THROW({
        pplib::String s1("A test string with unicode characters: äöü");
        pplib::String s2(s1);
        ASSERT_EQ((size_t)45, s2.len()) << "String does not have length of 45";
        const char* buf = s2.getPtr();
        ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
        ASSERT_EQ('A', (unsigned char)s2[0]) << "Unexpected Character in string";
        ASSERT_EQ(188, (unsigned char)s2[44]) << "Unexpected Character in string";
    });
}

TEST(StringTest, ConstructorWithMoveOperator)
{
    ASSERT_NO_THROW({
        pplib::String s1("A test string with unicode characters: äöü");
        pplib::String s2(std::move(s1));
        ASSERT_EQ((size_t)45, s2.len()) << "String does not have length of 45";
        ASSERT_EQ(pplib::String("A test string with unicode characters: äöü"), s2) << "String does not have expected value";
    });
}

TEST(StringTest, ConstructorFromStdString)
{
    ASSERT_NO_THROW({
        std::string s1("A test string with unicode characters: äöü");
        pplib::String s2(s1);
        ASSERT_EQ((size_t)45, s2.len()) << "String does not have length of 45";
        const char* buf = s2.getPtr();
        ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
        ASSERT_EQ('A', (unsigned char)s2[0]) << "Unexpected Character in string";
        ASSERT_EQ(188, (unsigned char)s2[44]) << "Unexpected Character in string";
    });
}

TEST(StringTest, ConstructorFromStdWString)
{

    std::wstring s1(L"A test string with unicode characters: äöü");
    ASSERT_EQ((size_t)42, s1.size()) << "std:wstring has unexpected size";
    pplib::String s2(s1);
    const char* buf = s2.getPtr();
    ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
    ASSERT_EQ('A', (unsigned char)s2[0]) << "Unexpected Character in string";
    ASSERT_EQ((size_t)45, s2.len()) << "String does not have length of 45";
    ASSERT_EQ(188, (unsigned char)s2[44]) << "Unexpected Character in string";
}

TEST(StringTest, ConstructorFromWideString)
{

    pplib::WideString s1(L"A test string with unicode characters: äöü");
    ASSERT_EQ((size_t)42, s1.size()) << "pplib::WideString has unexpected size";
    pplib::String s2(s1);
    const char* buf = s2.getPtr();
    ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
    ASSERT_EQ('A', (unsigned char)s2[0]) << "Unexpected Character in string";
    ASSERT_EQ((size_t)45, s2.len()) << "String does not have length of 45";
    ASSERT_EQ(188, (unsigned char)s2[44]) << "Unexpected Character in string";
}

TEST(StringTest, ConstructorFromByteArrayPtr)
{
    const char* data = "A test string with unicode characters: äöü";
    pplib::ByteArrayPtr b1((const unsigned char*)data, 45);
    pplib::String s2(b1);
    ASSERT_EQ((size_t)45, s2.len()) << "String does not have length of 45";
    ASSERT_EQ(pplib::String("A test string with unicode characters: äöü"), s2) << "String does not have expected value";
}

TEST(StringTest, clear)
{
    pplib::String s1("A test string with unicode characters: äöü");
    s1.clear();
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    const char* buf = s1.getPtr();
    ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
    ASSERT_THROW({ ASSERT_EQ(0, (unsigned char)s1[0]) << "Unexpected Character in string"; }, pplib::OutOfBoundsException);
}

TEST(StringTest, CapacityAndReserve)
{
    pplib::String s1;
    ASSERT_EQ((size_t)0, s1.capacity()) << "capacity did not return 0";
    s1.reserve(0);
    ASSERT_EQ((size_t)0, s1.capacity()) << "capacity did not return 0";
    s1.reserve(32);
    ASSERT_EQ((size_t)32, s1.capacity()) << "capacity did not return expected value";
    s1.reserve(128);
    ASSERT_EQ((size_t)128, s1.capacity()) << "capacity did not return expected value";
    s1.reserve(32);
    ASSERT_EQ((size_t)128, s1.capacity()) << "capacity did not return expected value";
}

TEST(StringTest, ReserveThrowsExceptions)
{
    pplib::String s1;
    ASSERT_THROW(s1.reserve(std::numeric_limits<size_t>::max()), pplib::IllegalArgumentException)
        << "reserve did not throw IllegalArgumentException";

    ASSERT_THROW(s1.reserve(std::numeric_limits<size_t>::max() - 1), pplib::OutOfMemoryException)
        << "reserve did not throw OutOfMemoryException";
}

TEST(StringTest, len)
{
    pplib::String s1("A test string with unicode characters: äöü");
    ASSERT_EQ(s1.len(), (size_t)45) << "String has unexpected size";
}

TEST(StringTest, length)
{
    pplib::String s1("A test string with unicode characters: äöü");
    ASSERT_EQ(s1.length(), (size_t)45) << "String has unexpected size";
}

TEST(StringTest, size)
{
    pplib::String s1("A test string with unicode characters: äöü");
    ASSERT_EQ(s1.size(), (size_t)45) << "String has unexpected size";
}

TEST(StringTest, isEmpty)
{
    pplib::String s1("A test string with unicode characters: äöü");
    pplib::String s2;
    ASSERT_EQ(s1.isEmpty(), false) << "String is empty";
    ASSERT_EQ(s2.isEmpty(), true) << "String not empty";
}

TEST(StringTest, notEmpty)
{
    pplib::String s1("A test string with unicode characters: äöü");
    pplib::String s2;
    ASSERT_EQ(s1.notEmpty(), true) << "String is not empty";
    ASSERT_EQ(s2.notEmpty(), false) << "String is empty";
}

TEST(StringTest, isNumeric)
{
    pplib::String s1("A test string with unicode characters: äöü");
    ASSERT_EQ(s1.isNumeric(), false) << "String should not be numeric";
    s1.set("");
    ASSERT_EQ(s1.isNumeric(), false) << "String should not be numeric";
    s1.set("12345abcd");
    ASSERT_EQ(s1.isNumeric(), false) << "String should not be numeric";
    s1.set("12345");
    ASSERT_EQ(s1.isNumeric(), true) << "String should be numeric";
    s1.set("-123451,12");
    ASSERT_EQ(s1.isNumeric(), true) << "String should be numeric";
    s1.set("-123451.12");
    ASSERT_EQ(s1.isNumeric(), true) << "String should be numeric";
    s1.set("123.451,12");
    ASSERT_EQ(s1.isNumeric(), false) << "String should be numeric";
    s1.set("-123.451,12");
    ASSERT_EQ(s1.isNumeric(), false) << "String should be numeric";
    s1.set("-123.451,12-9");
    ASSERT_EQ(s1.isNumeric(), false) << "String should not be numeric";

    ASSERT_FALSE(pplib::String("123.45.67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::String("123,45,67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::String("123.45,67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::String("123,45.67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::String("123-45-67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::String("123-45.67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::String("123-45,67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::String("123.45-67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::String("123,45-67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::String("123.").isNumeric()) << "String should not be numeric";
}

TEST(StringTest, isInteger)
{
    pplib::String s1("A test string with unicode characters: äöü");
    ASSERT_EQ(s1.isInteger(), false) << "String should not be an integer";
    s1.set("");
    ASSERT_EQ(s1.isInteger(), false) << "String should not be an integer";
    s1.set("12345abcd");
    ASSERT_EQ(s1.isInteger(), false) << "String should not be an integer";
    s1.set("12345");
    ASSERT_EQ(s1.isInteger(), true) << "String should be an integer";
    s1.set("-12345");
    ASSERT_EQ(s1.isInteger(), true) << "String should be an integer";
    s1.set("123-451");
    ASSERT_EQ(s1.isInteger(), false) << "String should be an integer";
    s1.set("123.451,12");
    ASSERT_EQ(s1.isInteger(), false) << "String should be an integer";
    s1.set("-123.451,12");
    ASSERT_EQ(s1.isInteger(), false) << "String should be an integer";
    s1.set("-123.451,12-9");
    ASSERT_EQ(s1.isInteger(), false) << "String should not be an integer";
}

TEST(StringTest, isTrue)
{
    pplib::String s1("A test string with unicode characters: äöü");
    ASSERT_EQ(s1.isTrue(), false) << "String should not be true";
    s1.set("");
    ASSERT_EQ(s1.isTrue(), false) << "String should not be true";
    s1.set("12345abcd");
    ASSERT_EQ(s1.isTrue(), true) << "String should be true";
    s1.set("1");
    ASSERT_EQ(s1.isTrue(), true) << "String should be true";
    s1.set("12345");
    ASSERT_EQ(s1.isTrue(), true) << "String should be true";
    s1.set("true");
    ASSERT_EQ(s1.isTrue(), true) << "String should be true";
    s1.set("wahr");
    ASSERT_EQ(s1.isTrue(), true) << "String should be true";
    s1.set("ja");
    ASSERT_EQ(s1.isTrue(), true) << "String should be true";
    s1.set("yes");
    ASSERT_EQ(s1.isTrue(), true) << "String should be true";
    s1.set("false");
    ASSERT_EQ(s1.isTrue(), false) << "String should not be true";
    s1.set("falsch");
    ASSERT_EQ(s1.isTrue(), false) << "String should not be true";
    s1.set("nein");
    ASSERT_EQ(s1.isTrue(), false) << "String should not be true";
    s1.set("no");
    ASSERT_EQ(s1.isTrue(), false) << "String should not be true";

    ASSERT_TRUE(pplib::String("t").isTrue()) << "String should be true";
}

TEST(StringTest, isFalse)
{
    pplib::String s1("A test string with unicode characters: äöü");
    ASSERT_EQ(s1.isFalse(), true) << "String should be false";
    s1.set("");
    ASSERT_EQ(s1.isFalse(), true) << "String should be false";
    s1.set("12345abcd");
    ASSERT_EQ(s1.isFalse(), false) << "String should not be false";
    s1.set("12345");
    ASSERT_EQ(s1.isFalse(), false) << "String should not be false";
    s1.set("0");
    ASSERT_EQ(s1.isFalse(), true) << "String should be false";
    s1.set("true");
    ASSERT_EQ(s1.isFalse(), false) << "String should not be false";
    s1.set("wahr");
    ASSERT_EQ(s1.isFalse(), false) << "String should not be false";
    s1.set("ja");
    ASSERT_EQ(s1.isFalse(), false) << "String should not be false";
    s1.set("yes");
    ASSERT_EQ(s1.isFalse(), false) << "String should not be false";
    s1.set("false");
    ASSERT_EQ(s1.isFalse(), true) << "String should be false";
    s1.set("falsch");
    ASSERT_EQ(s1.isFalse(), true) << "String should be false";
    s1.set("nein");
    ASSERT_EQ(s1.isFalse(), true) << "String should be false";
    s1.set("no");
    ASSERT_EQ(s1.isFalse(), true) << "String should be false";
}

TEST(StringTest, setConstCharWithoutSize)
{
    pplib::String s2("äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set("äöü, a test string with unicode characters");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
    ASSERT_EQ((size_t)45, s1.size()) << "String has unexpected length";
}

TEST(StringTest, setConstCharWithSize)
{
    pplib::String s2("äöü, a tes");
    pplib::String s1;
    s1.set("äöü, a test string with unicode characters", 13);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)13, s1.size()) << "String has unexpected length";
}

TEST(StringTest, setConstWChartWithoutSize)
{
    pplib::String s2("äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set(L"äöü, a test string with unicode characters");
    ASSERT_EQ(s1, s2) << "String as unexpected value";
    ASSERT_EQ((size_t)45, s1.size()) << "String has unexpected length";
}

TEST(StringTest, setConstWChartWithSize)
{
    pplib::String s2("äöü, a test s");
    pplib::String s1;
    s1.set(L"äöü, a test string with unicode characters", 13);
    ASSERT_EQ(pplib::String("äöü, a test s"), s1) << "String has unexpected value";
    ASSERT_EQ((size_t)16, s1.size()) << "String has unexpected length";
}

TEST(StringTest, setConstWChartWithNULL)
{
    pplib::String s1;
    s1.set((wchar_t*)NULL, 13);
    ASSERT_EQ((size_t)0, s1.size()) << "String has unexpected length";
    s1.set((wchar_t*)L"Hello World", 0);
    ASSERT_EQ((size_t)0, s1.size()) << "String has unexpected length";
}

TEST(StringTest, SetWcharInvalidCharacterEncoding)
{
    // 0xD800 ist ein ungültiger UTF-32 Code-Point (Surrogate Half)
    wchar_t invalid_wstr[] = {static_cast<wchar_t>(0xD800), 0};

    pplib::String str;
    EXPECT_THROW(str.set(invalid_wstr), pplib::CharacterEncodingException);
}

TEST(StringTest, setStringRefWithoutSize)
{
    pplib::String s2("äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set(s2);
    ASSERT_EQ(s1, s2) << "String as unexpected value";
    ASSERT_EQ((size_t)45, s1.size()) << "String has unexpected length";
}

TEST(StringTest, setStringRefWithSize)
{
    pplib::String s2("äöü, a tes");
    pplib::String s3("äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set(s3, 13);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)13, s1.size()) << "String has unexpected length";

    s1.set(s3, 500);
    ASSERT_EQ(s3, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)45, s1.size()) << "String has unexpected length";
}

TEST(StringTest, setSTDStringRefWithoutSize)
{
    pplib::String s2("äöü, a test string with unicode characters");
    std::string s3("äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set(s3);
    ASSERT_EQ(s1, s2) << "String as unexpected value";
    ASSERT_EQ((size_t)45, s1.size()) << "String has unexpected length";
}

TEST(StringTest, setSTDStringRefWithSize)
{
    pplib::String s2("äöü, a tes");
    std::string s3("äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set(s3, 13);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)13, s1.size()) << "String has unexpected length";

    s1.set(s3, 500);
    ASSERT_EQ(pplib::String("äöü, a test string with unicode characters"), s1) << "String has unexpected value";
    ASSERT_EQ((size_t)45, s1.size()) << "String has unexpected length";
}

TEST(StringTest, setSTDWStringRefWithoutSize)
{
    pplib::String s2("äöü, a test string with unicode characters");
    std::wstring s3(L"äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set(s3);
    ASSERT_EQ(s1, s2) << "String has unexpected value";
    ASSERT_EQ(s2.size(), s1.size()) << "String has unexpected length";

    s1.set(s3, 500);
    ASSERT_EQ(s1, s2) << "String has unexpected value";
    ASSERT_EQ(s2.size(), s1.size()) << "String has unexpected length";
}

TEST(StringTest, setSTDWStringRefWithSize)
{
    pplib::String s2("äöü, a tes");
    std::wstring s3(L"äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set(s3, 10);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ(s2.size(), s1.size()) << "String has unexpected length";
}

TEST(StringTest, setWideString)
{
    pplib::WideString ws(L"Hello World");
    pplib::String s1;
    s1.set(ws);
    ASSERT_EQ(pplib::String("Hello World"), s1) << "String has unexpected value";
    s1.set(ws, 5);
    ASSERT_EQ(pplib::String("Hello"), s1) << "String has unexpected value";
    s1.set(ws, 500);
    ASSERT_EQ(pplib::String("Hello World"), s1) << "String has unexpected value";
}

TEST(StringTest, setWithSelfAssignment)
{
    pplib::String s1("äöü, a test string with unicode characters");
    s1.set(s1);
    ASSERT_EQ((size_t)45, s1.size()) << "String has unexpected length";
    ASSERT_EQ(pplib::String("äöü, a test string with unicode characters"), s1) << "String has unexpected value";
}

TEST(StringTest, setWithSecondSetAndBiggerContent)
{
    pplib::String s1("äöü, a test string with unicode characters");
    s1.set("äöü, a test string with unicode characters and more content");
    ASSERT_EQ((size_t)62, s1.size()) << "String has unexpected length";
    ASSERT_EQ(pplib::String("äöü, a test string with unicode characters and more content"), s1) << "String has unexpected value";
}

TEST(StringTest, setWithSecondSetAndSmallerContent)
{
    pplib::String s1("äöü, a test string with unicode characters");
    s1.set("äöü, a test");
    ASSERT_EQ((size_t)14, s1.size()) << "String has unexpected length";
    ASSERT_EQ(pplib::String("äöü, a test"), s1) << "String has unexpected value";
}

TEST(StringTest, setWithByteArrayPtrWithoutSize)
{
    pplib::ByteArrayPtr b1((const unsigned char*)"Hello World", 11);
    pplib::String s1;
    s1.set(b1);
    ASSERT_EQ(pplib::String("Hello World"), s1) << "String has unexpected value";
    s1.set(b1, 5);
    ASSERT_EQ(pplib::String("Hello"), s1) << "String has unexpected value";
    s1.set(b1, 500);
    ASSERT_EQ(pplib::String("Hello World"), s1) << "String has unexpected value";
}

TEST(StringTest, setCharWithPosition)
{
    pplib::String s1("Hello World");
    s1.set(5, '-');
    ASSERT_EQ(pplib::String("Hello-World"), s1) << "String has unexpected value";
    ASSERT_THROW(s1.set(11, '-'), pplib::OutOfBoundsException) << "set() did not throw OutOfBoundsException";
}

TEST(StringTest, setf)
{
    pplib::String s2("Ein Test, 42, Wide, 10000");
    pplib::String s1;
    s1.setf("Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)25, s1.size()) << "String has unexpected length";
}

TEST(StringTest, format)
{
    pplib::String s1 = pplib::String::format("Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
    pplib::String s2("Ein Test, 42, Wide, 10000");
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)25, s1.size()) << "String has unexpected length";
}

TEST(StringTest, setWchart)
{
    pplib::String s2("a");
    pplib::String s1;
    s1.set((wchar_t)'a');
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)1, s1.size()) << "String has unexpected length";
}

static void test_vasprintf(pplib::String& str, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    str.vasprintf(fmt, args);
    va_end(args);
}

TEST(StringTest, vasprintf)
{
    pplib::String s2("Ein Test, 42, Wide, 10000");
    pplib::String s1;
    test_vasprintf(s1, "Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)25, s1.size()) << "String has unexpected length";
}

#ifndef _WIN32
// Der Test wird unter Windows nicht ausgeführt, da dort die Funktion vsnprintf() keine
// Fehler bei der Umwandlung von wchar_t* in die lokale Multibyte-Kodierung liefert.
TEST(StringTest, VasprintfEncodingError)
{
    wchar_t invalid_wstr[] = {static_cast<wchar_t>(0xD800), 0};

    // %ls versucht den wchar_t* in die lokale Multibyte-Kodierung umzuwandeln.
    // Bei ungültigen Zeichen liefert vsnprintf -1 zurück.
    EXPECT_THROW(pplib::String::format("%ls", invalid_wstr), pplib::Exception);
}
#endif

TEST(StringTest, appendConstWchartWithoutSize)
{
    pplib::String expected("First Part äöü, äöü Second Part");
    pplib::String s1("First Part äöü, ");
    s1.append(L"äöü Second Part");

    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ(expected.size(), s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendConstWchartWithSize)
{
    pplib::String expected("First Part äöü, äöü S");
    pplib::String s1("First Part äöü, ");
    s1.append(L"äöü Second Part", 5);

    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ(expected.size(), s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendConstCharPtrWithoutSize)
{
    pplib::String expected("First Part äöü, äöü Second Part");
    pplib::String s1("First Part äöü, ");
    s1.append("äöü Second Part");

    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendConstCharPtrWithSize)
{
    pplib::String expected("First Part äöü, äöü S");
    pplib::String s1("First Part äöü, ");
    s1.append("äöü Second Part", 8);

    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)27, s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendStringWithoutSize)
{
    pplib::String expected("First Part äöü, äöü Second Part");
    pplib::String s1("First Part äöü, ");
    pplib::String s2("äöü Second Part");
    s1.append(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendStringWithSize)
{
    pplib::String expected("First Part äöü, äöü S");
    pplib::String s1("First Part äöü, ");
    pplib::String s2("äöü Second Part");
    s1.append(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)27, s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendWideStringWithoutSize)
{
    pplib::String expected("First Part äöü, äöü Second Part");
    pplib::String s1("First Part äöü, ");
    pplib::WideString s2(L"äöü Second Part");
    s1.append(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendWideStringWithSize)
{
    pplib::String expected("First Part äöü, äöü Seco");
    pplib::String s1("First Part äöü, ");
    pplib::WideString s2(L"äöü Second Part");
    s1.append(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ(expected.size(), s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendStdStringWithoutSize)
{
    pplib::String expected("First Part äöü, äöü Second Part");
    pplib::String s1("First Part äöü, ");
    std::string s2("äöü Second Part");
    s1.append(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendStdStringWithSize)
{
    pplib::String expected("First Part äöü, äöü S");
    pplib::String s1("First Part äöü, ");
    std::string s2("äöü Second Part");
    s1.append(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)27, s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendStdWStringWithoutSize)
{
    pplib::String expected("First Part äöü, äöü Second Part");
    pplib::String s1("First Part äöü, ");
    std::wstring s2(L"äöü Second Part");
    s1.append(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ(expected.size(), s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendStdWStringWithSize)
{
    pplib::String expected("First Part äöü, äöü Seco");
    pplib::String s1("First Part äöü, ");
    std::wstring s2(L"äöü Second Part");
    s1.append(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ(expected.size(), s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendf)
{
    pplib::String expected("First Part äöü, Ein Test, 42, Wide, 10000");
    pplib::String s1("First Part äöü, ");
    s1.appendf("Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)44, s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendWchart)
{
    pplib::String expected("First Part äöü, a");
    pplib::String s1("First Part äöü, ");
    s1.append((wchar_t)'a');
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)20, s1.size()) << "String has unexpected length";
}

TEST(StringTest, append_same_string)
{
    pplib::String expected("It's me, It's me, It's me, It's me, ");
    pplib::String s1("It's me, ");
    s1.append(s1);
    s1.append(s1);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)36, s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendOnEmptyString)
{
    pplib::String expected("äöü Second Part");
    pplib::String s1;
    s1.append(L"äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)18, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.append("äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)18, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.append(pplib::String("äöü Second Part"));
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)18, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.append(pplib::WideString(L"äöü Second Part"));
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)18, s1.size()) << "String has unexpected length";

    s1.clear();
    std::string s3("äöü Second Part");
    s1.append(s3);
    ASSERT_EQ((size_t)18, s1.size()) << "String has unexpected length";
}

TEST(StringTest, appendNull)
{
    pplib::String s1("First Part äöü, ");
    s1.append((const char*)nullptr, 1);
    ASSERT_EQ(pplib::String("First Part äöü, "), s1) << "String has unexpected value";

    s1.append((const char*)"Hello World", 0);
    ASSERT_EQ(pplib::String("First Part äöü, "), s1) << "String has unexpected value";
}

TEST(StringTest, prependConstWchartWithoutSize)
{
    pplib::String expected("äöü Second PartFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    s1.prepend(L"äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST(StringTest, prependConstWchartWithSize)
{
    pplib::String expected("äöü SFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    s1.prepend(L"äöü Second Part", 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)27, s1.size()) << "String has unexpected length";
}

TEST(StringTest, prependConstCharPtrWithoutSize)
{
    pplib::String expected("äöü Second PartFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    s1.prepend("äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST(StringTest, prependConstCharPtrWithSize)
{
    pplib::String expected("äöü SFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    s1.prepend("äöü Second Part", 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)27, s1.size()) << "String has unexpected length";
}

TEST(StringTest, prependStringWithoutSize)
{
    pplib::String expected("äöü Second PartFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    pplib::String s2("äöü Second Part");
    s1.prepend(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST(StringTest, prependStringWithSize)
{
    pplib::String expected("äöü SFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    pplib::String s2("äöü Second Part");
    s1.prepend(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)27, s1.size()) << "String has unexpected length";
}

TEST(StringTest, prependStdStringWithoutSize)
{
    pplib::String expected("äöü Second PartFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    std::string s2("äöü Second Part");
    s1.prepend(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST(StringTest, prependStdStringWithSize)
{
    pplib::String expected("äöü SFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    std::string s2("äöü Second Part");
    s1.prepend(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)27, s1.size()) << "String has unexpected length";
}

TEST(StringTest, prependStdWStringWithoutSize)
{
    pplib::String expected("äöü Second PartFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    std::wstring s2(L"äöü Second Part");
    s1.prepend(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ(expected.size(), s1.size()) << "String has unexpected length";
}

TEST(StringTest, prependStdWStringWithSize)
{
    pplib::String expected("äöü SFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    std::wstring s2(L"äöü Second Part");
    s1.prepend(s2, 5);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ(expected.size(), s1.size()) << "String has unexpected length";
}

TEST(StringTest, prependf)
{
    pplib::String expected("Ein Test, 42, Wide, 10000First Part äöü, ");
    pplib::String s1("First Part äöü, ");
    s1.prependf("Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)44, s1.size()) << "String has unexpected length";
}

TEST(StringTest, prependWchart)
{
    pplib::String expected("aFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    s1.prepend((wchar_t)'a');
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)20, s1.size()) << "String has unexpected length";
}

TEST(StringTest, prepend_same_string)
{
    pplib::String expected("It's me, It's me, It's me, It's me, ");
    pplib::String s1("It's me, ");
    s1.prepend(s1);
    s1.prepend(s1);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)36, s1.size()) << "String has unexpected length";
}

TEST(StringTest, prependOnEmptyString)
{
    pplib::String expected("äöü Second Part");
    pplib::String s1;
    s1.prepend(L"äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)18, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.prepend("äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)18, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.prepend(pplib::String("äöü Second Part"));
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)18, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.prepend(pplib::WideString(L"äöü Second Part"));
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)18, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.prepend(std::string("äöü Second Part"));
    ASSERT_EQ((size_t)18, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.prepend(std::wstring(L"äöü Second Part"));
    ASSERT_EQ((size_t)18, s1.size()) << "String has unexpected length";
}

TEST(StringTest, prependNull)
{
    pplib::String s1("First Part äöü, ");
    s1.prepend((const char*)nullptr, 1);
    ASSERT_EQ(pplib::String("First Part äöü, "), s1) << "String has unexpected value";

    s1.prepend((const char*)"Hello World", 0);
    ASSERT_EQ(pplib::String("First Part äöü, "), s1) << "String has unexpected value";
}

TEST(StringTest, chopRight)
{
    pplib::String s1("A test string with unicode characters: äöü");
    pplib::String s2("A test string with unicode characters: ä");
    pplib::String s3("A test string with unicode characters: ");
    s1.chopRight(4);
    ASSERT_EQ((size_t)41, s1.len()) << "String does not have expected length";
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    s1.chopRight();
    s1.chopRight();
    ASSERT_EQ((size_t)39, s1.len()) << "String does not have expected length";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
    s1.chopRight(39);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    s2.clear();
    ASSERT_EQ(s2, s1) << "String has unexpected value";

    s1.set("");
    s1.chopRight(10);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have expected length";

    s1.set("Hello World");
    s1.chopRight(20);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have expected length";
}

TEST(StringTest, chopLeft)
{
    pplib::String s1("A test string with unicode characters: äöü");
    pplib::String s2("est string with unicode characters: äöü");
    pplib::String s3("st string with unicode characters: äöü");
    s1.chopLeft(3);
    ASSERT_EQ((size_t)42, s1.len()) << "String does not have expected length";
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    s1.chopLeft();
    ASSERT_EQ((size_t)41, s1.len()) << "String does not have expected length";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
    s1.chopRight(41);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    s2.clear();
    ASSERT_EQ(s2, s1) << "String has unexpected value";

    s1.set("");
    s1.chopLeft(10);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have expected length";

    s1.set("Hello World");
    s1.chopLeft(20);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have expected length";
}

TEST(StringTest, chop)
{
    pplib::String s1("A test string with unicode characters: äöü");
    pplib::String s2("A test string with unicode characters: ä");
    pplib::String s3("A test string with unicode characters: ");
    s1.chop(4);
    ASSERT_EQ((size_t)41, s1.len()) << "String does not have length of 40";
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    s1.chop();
    s1.chop();
    ASSERT_EQ((size_t)39, s1.len()) << "String does not have length of 39";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
    s1.chop(39);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    s2.clear();
    ASSERT_EQ(s2, s1) << "String has unexpected value";
}

TEST(StringTest, chomp)
{
    pplib::String s1("\n\r\nA test string\näöü\n\r\n");
    pplib::String s2("A test string\näöü");
    s1.chomp();
    ASSERT_EQ((size_t)20, s1.len()) << "String does not have length of 17";
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    s1.chomp();
    ASSERT_EQ((size_t)20, s1.len()) << "String does not have length of 17";
    ASSERT_EQ(s2, s1) << "String has unexpected value";
}

TEST(StringTest, cut_WithPos)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String expected("The quick ");
    s1.cut(10);
    ASSERT_EQ((size_t)10, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST(StringTest, cut_WithPos0)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String expected("");
    s1.cut(0);
    ASSERT_EQ((size_t)0, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST(StringTest, cut_WithPosBeyondStringLength)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String expected("The quick brown fox jumps over the lazy dog");
    s1.cut(100);
    ASSERT_EQ((size_t)43, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST(StringTest, cut_WithLetter)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String expected("The quick ");
    s1.cut("b");
    ASSERT_EQ((size_t)10, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST(StringTest, cut_WithNonexistingLetter)
{
    pplib::String s1("The quick brown fox");
    pplib::String expected("The quick brown fox");
    s1.cut("j");
    ASSERT_EQ((size_t)19, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST(StringTest, cut_WithString)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String expected("The quick ");
    s1.cut("brown");
    ASSERT_EQ((size_t)10, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST(StringTest, strstr)
{
    pplib::String s1("A test haystack string");
    pplib::String s3("haystack string");
    pplib::String s2;
    s2 = s1.strstr("haystack");
    ASSERT_EQ((size_t)15, s2.len()) << "String does not have length of 15";
    ASSERT_EQ(s3, s2) << "String has unexpected value";
    s2 = s1.strstr("beer");
    s3 = "";
    ASSERT_EQ((size_t)0, s2.len()) << "String does not have length of 0";
    ASSERT_EQ(s3, s2) << "String has unexpected value";

    s2 = s1.strstr("");
    ASSERT_EQ(s2, s1) << "String has unexpected value";
}

TEST(StringTest, repeated)
{
    pplib::String s1("_repeat_");
    pplib::String s3("_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
    pplib::String s2;
    s2 = s1.repeated(10);
    ASSERT_EQ((size_t)80, s2.len()) << "String does not have length of 80";
    ASSERT_EQ(s3, s2) << "String has unexpected value";
}

TEST(StringTest, repeat_by_count)
{
    pplib::String s1("_repeat_");
    s1.repeat(10);
    pplib::String s3("_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
    ASSERT_EQ((size_t)80, s1.len()) << "String does not have length of 80";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
}

TEST(StringTest, repeat_empty)
{
    pplib::String s1("");
    s1.repeat(10);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    ASSERT_EQ(pplib::String(""), s1) << "String has unexpected value";

    s1.set("Hello World");
    s1.repeat(0);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    ASSERT_EQ(pplib::String(""), s1) << "String has unexpected value";
}

TEST(StringTest, repeat_with_string)
{
    pplib::String s1("blah");
    s1.repeat(pplib::String("_repeat_"), 10);
    pplib::String s3("_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
    ASSERT_EQ((size_t)80, s1.len()) << "String does not have length of 80";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
}

TEST(StringTest, StringTest_repeat_empty)
{
    pplib::String s1("blah");
    s1.repeat(pplib::String(""), 10);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    ASSERT_EQ(pplib::String(""), s1) << "String has unexpected value";

    s1.set("Hello World");
    s1.repeat(pplib::String("_repeat_"), 0);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    ASSERT_EQ(pplib::String(""), s1) << "String has unexpected value";
}

TEST(StringTest, repeat_with_string_same_String)
{
    pplib::String s1("_repeat_");
    s1.repeat(s1, 10);
    pplib::String s3("_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
    ASSERT_EQ((size_t)80, s1.len()) << "String does not have length of 80";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
}

TEST(StringTest, repeat_with_code)
{
    pplib::String s1("_repeat_");
    s1.repeat('_', 10);
    pplib::String s3("__________");
    ASSERT_EQ((size_t)10, s1.len()) << "String does not have length of 10";
    ASSERT_EQ(s3, s1) << "String has unexpected value";

    s1.repeat(0, 10);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have expected length";

    s1.repeat('_', 0);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have expected length";
}

TEST(StringTest, trimLeft)
{
    pplib::String s1("\n\n    abc  \n");
    s1.trimLeft();
    ASSERT_EQ((size_t)6, s1.size());
    ASSERT_EQ(pplib::String("abc  \n"), s1);

    s1.set("  \r\n\tHello World\r\n\t   ");
    s1.trimLeft();
    ASSERT_EQ((size_t)17, s1.size());
    ASSERT_EQ(pplib::String("Hello World\r\n\t   "), s1);

    s1.set("");
    s1.trimLeft();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);

    s1.set("Nichts zu trimmen");
    s1.trimLeft();
    ASSERT_EQ((size_t)17, s1.size());
    ASSERT_EQ(pplib::String("Nichts zu trimmen"), s1);
}

TEST(StringTest, trimRight)
{
    pplib::String s1(" \n  abc  \n");
    s1.trimRight();
    ASSERT_EQ((size_t)7, s1.size());
    ASSERT_EQ(pplib::String(" \n  abc"), s1);

    s1.set("  \r\n\tHello World\r\n\t   ");
    s1.trimRight();
    ASSERT_EQ((size_t)16, s1.size());
    ASSERT_EQ(pplib::String("  \r\n\tHello World"), s1);

    s1.set("");
    s1.trimRight();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);

    s1.set("Nichts zu trimmen");
    s1.trimRight();
    ASSERT_EQ((size_t)17, s1.size());
    ASSERT_EQ(pplib::String("Nichts zu trimmen"), s1);
}

TEST(StringTest, trim)
{
    pplib::String s1("\n\n    abc  \n");
    s1.trim();
    ASSERT_EQ((size_t)3, s1.size());
    ASSERT_EQ(pplib::String("abc"), s1);

    s1.set("  \r\n\tHello World\r\n\t   ");
    s1.trim();
    ASSERT_EQ((size_t)11, s1.size());
    ASSERT_EQ(pplib::String("Hello World"), s1);

    s1.set("Nichts zu trimmen");
    s1.trim();
    ASSERT_EQ((size_t)17, s1.size());
    ASSERT_EQ(pplib::String("Nichts zu trimmen"), s1);
}

TEST(StringTest, trim_empty)
{
    pplib::String s1;
    s1.trim();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(), s1);
}

TEST(StringTest, trimmed)
{
    pplib::String s1("\n\n    abc  \n");
    pplib::String s2 = s1.trimmed();
    ASSERT_EQ((size_t)3, s2.size());
    ASSERT_EQ(pplib::String("abc"), s2);

    ASSERT_EQ(pplib::String("Hello World"), pplib::String("Hello World").trimmed());
    ASSERT_EQ(pplib::String("Hello World"), pplib::String("Hello World   ").trimmed());
    ASSERT_EQ(pplib::String("Hello World"), pplib::String("   Hello World").trimmed());
    ASSERT_EQ(pplib::String(""), pplib::String("").trimmed());
    ASSERT_EQ(pplib::String(""), pplib::String("   \n\t   \n").trimmed());
}

TEST(StringTest, trimLeftEmptyResult)
{
    pplib::String s1("\n\n   \n   \n");
    s1.trimLeft();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);
}

TEST(StringTest, trimRightEmptyResult)
{
    pplib::String s1("\n\n   \n   \n");
    s1.trimRight();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);
}

TEST(StringTest, trimEmptyResult)
{
    pplib::String s1("\n\n   \n   \n");
    s1.trim();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);
}

TEST(StringTest, trimLeftChars)
{
    pplib::String s1("\n\n    abc  \n");
    s1.trimLeft(" \n");
    ASSERT_EQ((size_t)6, s1.size());
    ASSERT_EQ(pplib::String("abc  \n"), s1);
}

TEST(StringTest, trimRightChars)
{
    pplib::String s1(" \n  abc  \n");
    s1.trimRight(" \n");
    ASSERT_EQ((size_t)7, s1.size());
    ASSERT_EQ(pplib::String(" \n  abc"), s1);
}

TEST(StringTest, trimChars)
{
    pplib::String s1("\n\n    abc  \n");
    s1.trim(" \n");
    ASSERT_EQ((size_t)3, s1.size());
    ASSERT_EQ(pplib::String("abc"), s1);
}

TEST(StringTest, trimLeftCharsEmptyResult)
{
    pplib::String s1("\n\n   \n   \n");
    s1.trimLeft(" \n");
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);
}

TEST(StringTest, trimRightCharsEmptyResult)
{
    pplib::String s1("\n\n   \n   \n");
    s1.trimRight(" \n");
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);
}

TEST(StringTest, trimEmptyCharsResult)
{
    pplib::String s1("\n\n   \n   \n");
    s1.trim(" \n");
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);
}

TEST(StringTest, trimCharsEmptry)
{
    pplib::String s1;
    s1.trim(" \n");
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);

    s1.set("Hello World");
    s1.trim("");
    ASSERT_EQ((size_t)11, s1.size());
    ASSERT_EQ(pplib::String("Hello World"), s1);
}

TEST(StringTest, operatorEqualEmpty)
{
    pplib::String s1("");
    pplib::String s2("");
    ASSERT_EQ(s1, s2);
}

TEST(StringTest, strcmpEmpty)
{
    pplib::String s1("");
    pplib::String s2("");
    ASSERT_EQ(0, s1.strcmp(s2));
}

TEST(StringTest, strcmpLower)
{
    pplib::String s1("ABcdef");
    pplib::String s2("Defghi");
    ASSERT_LT(s1.strcmp(s2), 0);
    ASSERT_LT(s1.strcmp(s2, 3), 0);
}

TEST(StringTest, strcmpLowerWithCase)
{
    pplib::String s1("ABcdef");
    pplib::String s2("abcdef");
    ASSERT_LT(s1.strcmp(s2), 0);
    ASSERT_LT(s1.strcmp(s2, 6), 0);
}

TEST(StringTest, strcmpHigher)
{
    pplib::String s1("ABcdef");
    pplib::String s2("Defghi");
    ASSERT_GT(s2.strcmp(s1), 0);
}

TEST(StringTest, strcmpHigherWithCase)
{
    pplib::String s1("ABcdef");
    pplib::String s2("abcdef");
    ASSERT_GT(s2.strcmp(s1), 0);
}

TEST(StringTest, strcmpEqual)
{
    pplib::String s1("ABcdef");
    pplib::String s2("ABcdef");
    ASSERT_EQ(s2.strcmp(s1), 0);
}

TEST(StringTest, strcasecmpEmpty)
{
    pplib::String s1("");
    pplib::String s2("");
    ASSERT_EQ(0, s1.strCaseCmp(s2));
}

TEST(StringTest, strcasecmpLower)
{
    pplib::String s1("ABcdef");
    pplib::String s2("Defghi");
    ASSERT_LT(s1.strCaseCmp(s2), 0);
    ASSERT_LT(s1.strCaseCmp(s2, 6), 0);
}

TEST(StringTest, strcasecmpLowerWithCase)
{
    pplib::String s1("ABcdef");
    pplib::String s2("abcdef");
    ASSERT_EQ(s1.strCaseCmp(s2), 0);
}

TEST(StringTest, strcasecmpHigher)
{
    pplib::String s1("ABcdef");
    pplib::String s2("Defghi");
    ASSERT_GT(s2.strCaseCmp(s1), 0);
}

TEST(StringTest, strcasecmpHigherWithCase)
{
    pplib::String s1("ABcdef");
    pplib::String s2("abcdef");
    ASSERT_EQ(s2.strCaseCmp(s1), 0);
}

TEST(StringTest, strcasecmpEqual)
{
    pplib::String s1("ABcdef");
    pplib::String s2("ABcdef");
    ASSERT_EQ(s2.strCaseCmp(s1), 0);
}

TEST(StringTest, left)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String s2 = s1.left(10);
    ASSERT_EQ(pplib::String("The quick "), s2);
    s2 = s1.left(100);
    ASSERT_EQ(pplib::String("The quick brown fox jumps over the lazy dog"), s2);
}

TEST(StringTest, right)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String s2 = s1.right(9);
    ASSERT_EQ(pplib::String(" lazy dog"), s2);
    s2 = s1.right(100);
    ASSERT_EQ(pplib::String("The quick brown fox jumps over the lazy dog"), s2);
}

TEST(StringTest, midWithLength)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String s2 = s1.mid(10, 10);
    ASSERT_EQ(pplib::String("brown fox "), s2);

    s2 = s1.mid(10, 0);
    ASSERT_EQ(pplib::String(""), s2);
}

TEST(StringTest, midWithoutLength)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String s2 = s1.mid(10);
    ASSERT_EQ(pplib::String("brown fox jumps over the lazy dog"), s2);
}

TEST(StringTest, midOnEmptyString)
{
    pplib::String s1;
    pplib::String s2 = s1.mid(10, 10);
    ASSERT_EQ(pplib::String(""), s2);

    s2 = s1.mid(10);
    ASSERT_EQ(pplib::String(""), s2);
}

TEST(StringTest, substrWithLength)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String s2 = s1.substr(10, 10);
    ASSERT_EQ(pplib::String("brown fox "), s2);
}

TEST(StringTest, substrWithoutLength)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String s2 = s1.substr(10);
    ASSERT_EQ(pplib::String("brown fox jumps over the lazy dog"), s2);
}

TEST(StringTest, lowerCase)
{
    pplib::String s1("The Quick Brown Fox Jumps over ÄÖÜ");
    pplib::String expected("the quick brown fox jumps over äöü");
    ASSERT_NO_THROW(s1.lowerCase());
    ASSERT_EQ(expected, s1);
    ASSERT_EQ(pplib::String(""), pplib::String("").lowerCase());
}

TEST(StringTest, upperCase)
{
    pplib::String s1("The Quick Brown Fox Jumps over äöü");
    pplib::String expected("THE QUICK BROWN FOX JUMPS OVER ÄÖÜ");
    ASSERT_NO_THROW(s1.upperCase());
    ASSERT_EQ(expected, s1);
    ASSERT_EQ(pplib::String(""), pplib::String("").upperCase());
}

TEST(StringTest, toLowerCase)
{
    ASSERT_EQ(pplib::String(L"the quick brown fox jumps over äöü"), pplib::String(L"The Quick Brown Fox Jumps over ÄÖÜ").toLowerCase());
}

TEST(StringTest, toUpperCase)
{
    ASSERT_EQ(pplib::String(L"THE QUICK BROWN FOX JUMPS OVER ÄÖÜ"), pplib::String(L"The Quick Brown Fox Jumps over äöü").toUpperCase());
}

TEST(StringTest, strchr_ExistingChar)
{
    pplib::String s1("The Quick Brown Fox Jumps over äöü");
    pplib::String expected("Fox Jumps over äöü");
    ASSERT_EQ(expected, s1.strchr('F')) << "Unexpected Result";
}

TEST(StringTest, strchr_NonExistingChar)
{
    pplib::String s1("The Quick Brown Fox Jumps over äöü");
    pplib::String expected("");
    ASSERT_EQ(expected, s1.strchr('L')) << "Unexpected Result";
}

TEST(StringTest, strrchr_ExistingChar)
{
    pplib::String s1("The Quick Brown Fox Jumps over äöü");
    pplib::String expected("over äöü");
    ASSERT_EQ(expected, s1.strrchr('o')) << "Unexpected Result";
}

TEST(StringTest, strrchr_NonExistingChar)
{
    pplib::String s1("The Quick Brown Fox Jumps over äöü");
    pplib::String expected("");
    ASSERT_EQ(expected, s1.strrchr('L')) << "Unexpected Result";
}

TEST(StringTest, toInt)
{
    EXPECT_EQ((int)1234, pplib::String("1234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)-1234, pplib::String("-1234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)1234, pplib::String("01234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)-1234, pplib::String("-01234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)0, pplib::String("abc123").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)0, pplib::String("0x1234").toInt()) << "Unexpected Result";
}

TEST(StringTest, toUnsignedInt)
{
    EXPECT_EQ((unsigned int)1234, pplib::String("1234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)-1234, pplib::String("-1234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)1234, pplib::String("01234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)-1234, pplib::String("-01234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)0, pplib::String("abc123").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)0, pplib::String("0x1234").toUnsignedInt()) << "Unexpected Result";
}

TEST(StringTest, toInt64)
{
    EXPECT_EQ((int64_t)1234, pplib::String("1234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)-1234, pplib::String("-1234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)1234, pplib::String("01234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)-1234, pplib::String("-01234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)0, pplib::String("abc123").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)0, pplib::String("0x1234").toInt64()) << "Unexpected Result";
}

TEST(StringTest, toUnsignedInt64)
{
    EXPECT_EQ((uint64_t)1234, pplib::String("1234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)-1234, pplib::String("-1234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)1234, pplib::String("01234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)-1234, pplib::String("-01234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)0, pplib::String("abc123").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)0, pplib::String("0x1234").toUnsignedInt64()) << "Unexpected Result";
}

TEST(StringTest, toInt_withoutNumber)
{
    pplib::String s1("The Quick Brown Fox Jumps over the lazy dog");
    EXPECT_EQ((int)0, s1.toInt()) << "Unexpected Result";
}

TEST(StringTest, toInt_123456)
{
    pplib::String s1("123456");
    EXPECT_EQ((int)123456, s1.toInt()) << "Unexpected Result";
}

TEST(StringTest, ToInt_minus123456)
{
    pplib::String s1("-123456");
    EXPECT_EQ((int)-123456, s1.toInt()) << "Unexpected Result";
}

TEST(StringTest, toInt_123456_point_567)
{
    pplib::String s1("123456.567");
    EXPECT_EQ((int)123456, s1.toInt()) << "Unexpected Result";
}

TEST(StringTest, toInt64_1242346214893456)
{
    pplib::String s1("1242346214893456");
    EXPECT_EQ((int64_t)1242346214893456, s1.toInt64()) << "Unexpected Result";
}

TEST(StringTest, toInt64_minus1242346214893456)
{
    pplib::String s1("-1242346214893456");
    EXPECT_EQ((int64_t)-1242346214893456, s1.toInt64()) << "Unexpected Result";
}

TEST(StringTest, toBool)
{
    pplib::String s1("A test string with unicode characters: äöü");
    ASSERT_EQ(s1.toBool(), false) << "String should not be true";
    s1.set("");
    ASSERT_EQ(s1.toBool(), false) << "String should not be true";
    s1.set("12345abcd");
    ASSERT_EQ(s1.toBool(), true) << "String should be true";
    s1.set("1");
    ASSERT_EQ(s1.toBool(), true) << "String should be true";
    s1.set("12345");
    ASSERT_EQ(s1.toBool(), true) << "String should be true";
    s1.set("true");
    ASSERT_EQ(s1.toBool(), true) << "String should be true";
    s1.set("wahr");
    ASSERT_EQ(s1.toBool(), true) << "String should be true";
    s1.set("ja");
    ASSERT_EQ(s1.toBool(), true) << "String should be true";
    s1.set("yes");
    ASSERT_EQ(s1.toBool(), true) << "String should be true";
    s1.set("false");
    ASSERT_EQ(s1.toBool(), false) << "String should not be true";
    s1.set("falsch");
    ASSERT_EQ(s1.toBool(), false) << "String should not be true";
    s1.set("nein");
    ASSERT_EQ(s1.toBool(), false) << "String should not be true";
    s1.set("no");
    ASSERT_EQ(s1.toBool(), false) << "String should not be true";
}

TEST(StringTest, ToLong_1124234674)
{
    pplib::String s1("124234674");
    EXPECT_EQ((long)124234674, s1.toLong()) << "Unexpected Result";
}

TEST(StringTest, toLongLong_1242346214893456)
{
    pplib::String s1("1242346214893456");
    EXPECT_EQ((long long)1242346214893456, s1.toLongLong()) << "Unexpected Result";
}

TEST(StringTest, ToFloat_182566142_346214893456)
{
    pplib::String s1("182566142.346214893456");
    EXPECT_EQ((float)182566142.346214893456, s1.toFloat()) << "Unexpected Result";
}

TEST(StringTest, ToDouble_182566142_346214893456)
{
    pplib::String s1("182566142.346214893456");
    EXPECT_EQ(((float)182566142.346214893456), ((float)s1.toDouble())) << "Unexpected Result";
}

TEST(StringTest, OperatorCharPositionPositiv)
{
    pplib::String s1("Hello World!");
    EXPECT_EQ((char)'H', s1[0]) << "Unexpected Result";
    EXPECT_EQ((char)'e', s1[1]) << "Unexpected Result";
    EXPECT_EQ((char)'l', s1[2]) << "Unexpected Result";
    EXPECT_EQ((char)'l', s1[3]) << "Unexpected Result";
    EXPECT_EQ((char)'o', s1[4]) << "Unexpected Result";
    EXPECT_EQ((char)' ', s1[5]) << "Unexpected Result";
    EXPECT_EQ((char)'W', s1[6]) << "Unexpected Result";
    EXPECT_EQ((char)'o', s1[7]) << "Unexpected Result";
    EXPECT_EQ((char)'r', s1[8]) << "Unexpected Result";
    EXPECT_EQ((char)'l', s1[9]) << "Unexpected Result";
    EXPECT_EQ((char)'d', s1[10]) << "Unexpected Result";
    EXPECT_EQ((char)'!', s1[11]) << "Unexpected Result";

    ASSERT_THROW({ ASSERT_EQ((char)0, s1[12]); }, pplib::OutOfBoundsException);
}

TEST(StringTest, OperatorConstCharPositionPositiv)
{
    const pplib::String s1("Hello World!");
    EXPECT_EQ((char)'H', s1[0]) << "Unexpected Result";
    EXPECT_EQ((char)'e', s1[1]) << "Unexpected Result";
    EXPECT_EQ((char)'l', s1[2]) << "Unexpected Result";
    EXPECT_EQ((char)'l', s1[3]) << "Unexpected Result";
    EXPECT_EQ((char)'o', s1[4]) << "Unexpected Result";
    EXPECT_EQ((char)' ', s1[5]) << "Unexpected Result";
    EXPECT_EQ((char)'W', s1[6]) << "Unexpected Result";
    EXPECT_EQ((char)'o', s1[7]) << "Unexpected Result";
    EXPECT_EQ((char)'r', s1[8]) << "Unexpected Result";
    EXPECT_EQ((char)'l', s1[9]) << "Unexpected Result";
    EXPECT_EQ((char)'d', s1[10]) << "Unexpected Result";
    EXPECT_EQ((char)'!', s1[11]) << "Unexpected Result";

    ASSERT_THROW({ ASSERT_EQ((char)0, s1[12]); }, pplib::OutOfBoundsException);
}

TEST(StringTest, OperatorCharPositionNegativ)
{
    pplib::String s1("Hello World!");
    EXPECT_EQ((char)'H', s1[0]) << "Unexpected Result";
    EXPECT_EQ((char)'!', s1[-1]) << "Unexpected Result";
    EXPECT_EQ((char)'d', s1[-2]) << "Unexpected Result";
    EXPECT_EQ((char)'l', s1[-3]) << "Unexpected Result";
    EXPECT_EQ((char)'r', s1[-4]) << "Unexpected Result";
    EXPECT_EQ((char)'o', s1[-5]) << "Unexpected Result";
    EXPECT_EQ((char)'W', s1[-6]) << "Unexpected Result";
    EXPECT_EQ((char)' ', s1[-7]) << "Unexpected Result";
    EXPECT_EQ((char)'o', s1[-8]) << "Unexpected Result";
    EXPECT_EQ((char)'l', s1[-9]) << "Unexpected Result";
    EXPECT_EQ((char)'l', s1[-10]) << "Unexpected Result";
    EXPECT_EQ((char)'e', s1[-11]) << "Unexpected Result";
    EXPECT_EQ((char)'H', s1[-12]) << "Unexpected Result";
    ASSERT_THROW({ ASSERT_EQ((char)0, s1[-13]); }, pplib::OutOfBoundsException);
}

TEST(StringTest, OperatorConstCharPositionNegativ)
{
    const pplib::String s1("Hello World!");
    EXPECT_EQ((char)'H', s1[0]) << "Unexpected Result";
    EXPECT_EQ((char)'!', s1[-1]) << "Unexpected Result";
    EXPECT_EQ((char)'d', s1[-2]) << "Unexpected Result";
    EXPECT_EQ((char)'l', s1[-3]) << "Unexpected Result";
    EXPECT_EQ((char)'r', s1[-4]) << "Unexpected Result";
    EXPECT_EQ((char)'o', s1[-5]) << "Unexpected Result";
    EXPECT_EQ((char)'W', s1[-6]) << "Unexpected Result";
    EXPECT_EQ((char)' ', s1[-7]) << "Unexpected Result";
    EXPECT_EQ((char)'o', s1[-8]) << "Unexpected Result";
    EXPECT_EQ((char)'l', s1[-9]) << "Unexpected Result";
    EXPECT_EQ((char)'l', s1[-10]) << "Unexpected Result";
    EXPECT_EQ((char)'e', s1[-11]) << "Unexpected Result";
    EXPECT_EQ((char)'H', s1[-12]) << "Unexpected Result";
    ASSERT_THROW({ ASSERT_EQ((char)0, s1[-13]); }, pplib::OutOfBoundsException);
}

TEST(StringTest, getChat)
{
    pplib::String s1("Hello World!");
    EXPECT_EQ((char)'H', s1.get(0)) << "Unexpected Result";
    EXPECT_EQ((char)'e', s1.get(1)) << "Unexpected Result";
    EXPECT_EQ((char)'!', s1.get(11)) << "Unexpected Result";
    ASSERT_THROW({ ASSERT_EQ((char)0, s1.get(12)); }, pplib::OutOfBoundsException);

    EXPECT_EQ((char)'!', s1.get(-1)) << "Unexpected Result";
    EXPECT_EQ((char)'d', s1.get(-2)) << "Unexpected Result";
    EXPECT_EQ((char)'H', s1.get(-12)) << "Unexpected Result";
    ASSERT_THROW({ ASSERT_EQ((char)0, s1.get(-13)); }, pplib::OutOfBoundsException);
}

TEST(StringTest, join)
{
    pplib::Array a;
    a.add("One");
    a.add("Two");
    a.add("Three");
    pplib::String s1(",");
    EXPECT_EQ(pplib::String("One,Two,Three"), s1.join(a));
}

TEST(StringTest, print)
{
    pplib::String s1("Hello World!");
    testing::internal::CaptureStdout();
    s1.print();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ("Hello World!", output) << "Unexpected Result";
}

TEST(StringTest, printEmptyString)
{
    pplib::String s1;
    testing::internal::CaptureStdout();
    s1.print();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ("", output) << "Unexpected Result";

    testing::internal::CaptureStdout();
    s1.print(true);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ("\n", output) << "Unexpected Result";
}

TEST(StringTest, printnl)
{
    pplib::String s1("Hello World!");
    testing::internal::CaptureStdout();
    s1.printnl();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ("Hello World!\n", output) << "Unexpected Result";
}

TEST(StringTest, hexDump)
{
    pplib::String s1("Hello World!");
    testing::internal::CaptureStdout();
    s1.hexDump();
    pplib::String output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.has("48 65 6C 6C 6F 20 57 6F 72 6C 64 21")) << "Unexpected Result";
    EXPECT_TRUE(output.has("Hello World!")) << "Unexpected Result";
}
TEST(StringTest, hexDumpEmptyString)
{
    pplib::String s2;
    testing::internal::CaptureStdout();
    s2.hexDump();
    pplib::String output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.has("HEXDUMP of String")) << "Unexpected Result";
}

// Equal Operatoren
TEST(StringTest, operatorEqualConstChar)
{
    pplib::String s1("Hello World!");
    pplib::String s2;
    s2 = "Hello World!";
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(StringTest, operatorEqualConstWchar)
{
    pplib::String s1("Hello World!");
    pplib::String s2;
    s2 = L"Hello World!";
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(StringTest, operatorEqualString)
{
    pplib::String s1("Hello World!");
    pplib::String s2;
    s2 = pplib::String("Hello World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(StringTest, operatorEqualStringMove)
{
    pplib::String s1("Hello World!");
    pplib::String s2;
    s2 = std::move(pplib::String("Hello World!"));
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(StringTest, operatorEqualStringMoveWithSameString)
{
    pplib::String s1("Hello World!");
    pplib::String s2 = s1;
    s2 = std::move(s2);
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(StringTest, operatorEqualWideString)
{
    pplib::String s1("Hello World!");
    pplib::String s2;
    s2 = pplib::WideString(L"Hello World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(StringTest, operatorEqualStdString)
{
    pplib::String s1("Hello World!");
    pplib::String s2;
    s2 = std::string("Hello World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(StringTest, operatorEqualStdWString)
{
    pplib::String s1("Hello World!");
    pplib::String s2;
    s2 = std::wstring(L"Hello World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(StringTest, operatorEqualChar)
{
    pplib::String s1("H");
    pplib::String s2;
    s2 = 'H';
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

// Equal Plus Operatoren
TEST(StringTest, operatorEqualPlusConstChar)
{
    pplib::String s1("Hello World!");
    pplib::String s2("Hello ");
    s2 += "World!";
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(StringTest, operatorEqualPlusConstWchar)
{
    pplib::String s1("Hello World!");
    pplib::String s2("Hello ");
    s2 += L"World!";
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(StringTest, operatorEqualPlusString)
{
    pplib::String s1("Hello World!");
    pplib::String s2("Hello ");
    s2 += pplib::String("World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(StringTest, operatorEqualPlusWideString)
{
    pplib::String s1("Hello World!");
    pplib::String s2("Hello ");
    s2 += pplib::WideString(L"World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(StringTest, operatorEqualPlusStdString)
{
    pplib::String s1("Hello World!");
    pplib::String s2("Hello ");
    s2 += std::string("World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(StringTest, operatorEqualPlusStdWString)
{
    pplib::String s1("Hello World!");
    pplib::String s2("Hello ");
    s2 += std::wstring(L"World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(StringTest, operatorEqualPlusChar)
{
    pplib::String s1("He");
    pplib::String s2("H");
    s2 += 'e';
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

// Operator plus

TEST(StringTest, operatorPlus_StringPlusString)
{
    pplib::String s1("Hello ");
    pplib::String s2("World!");
    pplib::String s3 = s1 + s2;
    ASSERT_EQ(pplib::String("Hello World!"), s3) << "String has unexpected value";
}

TEST(StringTest, operatorPlus_StringPlusConstCharPtr)
{
    pplib::String s1("Hello ");
    pplib::String s3 = s1 + "World!";
    ASSERT_EQ(pplib::String("Hello World!"), s3) << "String has unexpected value";
}

TEST(StringTest, operatorPlus_StringPlusConstWcharPtr)
{
    pplib::String s1("Hello ");
    pplib::String s3 = s1 + L"World!";
    ASSERT_EQ(pplib::String("Hello World!"), s3) << "String has unexpected value";
}

TEST(StringTest, operatorPlus_StringPlusWideString)
{
    pplib::String s1("Hello ");
    pplib::String s3 = s1 + pplib::WideString(L"World!");
    ASSERT_EQ(pplib::String("Hello World!"), s3) << "String has unexpected value";
}

TEST(StringTest, operatorPlus_StringPlusStdString)
{
    pplib::String s1("Hello ");
    pplib::String s3 = s1 + std::string("World!");
    ASSERT_EQ(pplib::String("Hello World!"), s3) << "String has unexpected value";
}

TEST(StringTest, operatorPlus_StringPlusStdWString)
{
    pplib::String s1("Hello ");
    pplib::String s3 = s1 + std::wstring(L"World!");
    ASSERT_EQ(pplib::String("Hello World!"), s3) << "String has unexpected value";
}

TEST(StringTest, operatorPlus_StringPlusChar)
{
    pplib::String s1("He");
    pplib::String s3 = s1 + 'l';
    ASSERT_EQ(pplib::String("Hel"), s3) << "String has unexpected value";
}

TEST(StringTest, operatorPlus_ConstCharPtrPlusString)
{
    pplib::String s1("World!");
    pplib::String s3 = "Hello " + s1;
    ASSERT_EQ(pplib::String("Hello World!"), s3) << "String has unexpected value";
}

TEST(StringTest, operatorPlus_ConstWcharPtrPlusString)
{
    pplib::String s1("World!");
    pplib::String s3 = L"Hello " + s1;
    ASSERT_EQ(pplib::String("Hello World!"), s3) << "String has unexpected value";
}

TEST(StringTest, operatorPlus_WideStringPlusString)
{
    pplib::String s1("World!");
    pplib::String s3 = pplib::WideString(L"Hello ") + s1;
    ASSERT_EQ(pplib::String("Hello World!"), s3) << "String has unexpected value";
}

TEST(StringTest, operatorPlus_StdStringPlusString)
{
    pplib::String s1("World!");
    pplib::String s3 = std::string("Hello ") + s1;
    ASSERT_EQ(pplib::String("Hello World!"), s3) << "String has unexpected value";
}

TEST(StringTest, operatorPlus_StdWStringPlusString)
{
    pplib::String s1("World!");
    pplib::String s3 = std::wstring(L"Hello ") + s1;
    ASSERT_EQ(pplib::String("Hello World!"), s3) << "String has unexpected value";
}

TEST(StringTest, operatorPlus_CharPlusString)
{
    pplib::String s1("el");
    pplib::String s3 = 'H' + s1;
    ASSERT_EQ(pplib::String("Hel"), s3) << "String has unexpected value";
}

// find

TEST(StringTest, find_forward)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    ASSERT_EQ((size_t)16, s1.find("fox")) << "String has unexpected value";
    ASSERT_EQ((size_t)0, s1.find("The")) << "String has unexpected value";
    ASSERT_EQ((size_t)40, s1.find("dog", 15)) << "String has unexpected value";
    ASSERT_EQ(pplib::String::npos, s1.find("cow")) << "String has unexpected value";
    ASSERT_EQ(pplib::String::npos, s1.find("")) << "String has unexpected value";
    ASSERT_EQ(pplib::String::npos, s1.find("brown", 100)) << "String has unexpected value";

    pplib::String empty;
    ASSERT_EQ(pplib::String::npos, empty.find("fox")) << "String has unexpected value";
}

TEST(StringTest, find_backward)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    ASSERT_EQ((size_t)16, s1.find("fox", -1)) << "String has unexpected value";
    ASSERT_EQ((size_t)42, s1.find("g", -1)) << "String has unexpected value";
    ASSERT_EQ((size_t)0, s1.find("The", -20)) << "String has unexpected value";
    ASSERT_EQ((size_t)pplib::String::npos, s1.find("cat", -1)) << "String has unexpected value";
    ASSERT_EQ((size_t)pplib::String::npos, s1.find("dog", -100)) << "String has unexpected value";
}

TEST(StringTest, findCase)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    ASSERT_EQ((size_t)16, s1.findCase("FOX")) << "String has unexpected value";
    ASSERT_EQ((size_t)0, s1.findCase("the")) << "String has unexpected value";
    ASSERT_EQ((size_t)40, s1.findCase("DOG", 15)) << "String has unexpected value";
    ASSERT_EQ(pplib::String::npos, s1.findCase("cow")) << "String has unexpected value";
    ASSERT_EQ(pplib::String::npos, s1.findCase("")) << "String has unexpected value";
    ASSERT_EQ(pplib::String::npos, s1.findCase("brown", 100)) << "String has unexpected value";

    pplib::String empty;
    ASSERT_EQ(pplib::String::npos, empty.findCase("fox")) << "String has unexpected value";
}

TEST(StringTest, has)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    ASSERT_TRUE(s1.has("fox")) << "String has unexpected value";
    ASSERT_TRUE(s1.has("The")) << "String has unexpected value";
    ASSERT_TRUE(s1.contains("FOX", true)) << "String has unexpected value";
    ASSERT_FALSE(s1.contains("cat")) << "String has unexpected value";
    ASSERT_FALSE(s1.contains("")) << "String has unexpected value";

    ASSERT_FALSE(pplib::String().has("fox")) << "String has unexpected value";
}

TEST(StringTest, startsWith)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    ASSERT_TRUE(s1.startsWith("The quick")) << "String has unexpected value";
    ASSERT_TRUE(s1.startsWith("quick", 4)) << "String has unexpected value";
    ASSERT_TRUE(s1.startsWith("quick", 4, 20)) << "String has unexpected value";
    ASSERT_TRUE(s1.startsWith("The", 0, 20)) << "String has unexpected value";
    ASSERT_FALSE(s1.startsWith("the quick")) << "String has unexpected value";
    ASSERT_FALSE(s1.startsWith("quick")) << "String has unexpected value";
}

TEST(StringTest, endsWith)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    ASSERT_TRUE(s1.endsWith("lazy dog")) << "String has unexpected value";
    ASSERT_TRUE(s1.endsWith("dog", 40)) << "String has unexpected value";
    ASSERT_TRUE(s1.endsWith("dog", 40, 4)) << "String has unexpected value";
    ASSERT_TRUE(s1.endsWith("dog", 0, 100)) << "String has unexpected value";
    ASSERT_FALSE(s1.endsWith("Lazy dog")) << "String has unexpected value";
    ASSERT_FALSE(s1.endsWith("lazy")) << "String has unexpected value";
}

TEST(StringTest, replace)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String s2 = s1.replace("fox", "cat");
    ASSERT_EQ(pplib::String("The quick brown cat jumps over the lazy dog"), s2) << "String has unexpected value";
    pplib::String s3 = s1.replace("the", "a");
    ASSERT_EQ(pplib::String("The quick brown cat jumps over a lazy dog"), s3) << "String has unexpected value";

    s1.set("");
    pplib::String s4 = s1.replace("fox", "cat");
    ASSERT_EQ(pplib::String(""), s4) << "String has unexpected value";

    s1.set("The quick brown fox jumps over the lazy dog");
    s1.replace("", "Hello World");
    ASSERT_EQ(pplib::String("The quick brown fox jumps over the lazy dog"), s1) << "String has unexpected value";
}

TEST(StringTest, shl)
{
    pplib::String s1("Hello World!");
    s1.shl(0, 5);
    ASSERT_EQ(pplib::String(" World!"), s1) << "String has unexpected value";
    s1.shl(' ', 2);
    ASSERT_EQ(pplib::String("orld!  "), s1) << "String has unexpected value";

    s1.shl(' ', 0);
    ASSERT_EQ(pplib::String("orld!  "), s1) << "String has unexpected value";
    s1.shl(' ', 100);
    ASSERT_EQ(pplib::String("       "), s1) << "String has unexpected value";

    s1.set("");
    s1.shl(' ', 5);
    ASSERT_EQ(pplib::String(""), s1) << "String has unexpected value";
}

TEST(StringTest, shr)
{
    pplib::String s1("Hello World!");
    s1.shr(0, 5);
    ASSERT_EQ(pplib::String("Hello W"), s1) << "String has unexpected value";
    s1.set("Hello World!");
    s1.shr(' ', 2);
    ASSERT_EQ(pplib::String("  Hello Worl"), s1) << "String has unexpected value";
    s1.shr(' ', 0);
    ASSERT_EQ(pplib::String("  Hello Worl"), s1) << "String has unexpected value";

    s1.set("");
    s1.shr(' ', 5);
    ASSERT_EQ(pplib::String(""), s1) << "String has unexpected value";

    s1.set("Hello World!");
    s1.shr(' ', 100);
    ASSERT_EQ(pplib::String("            "), s1) << "String has unexpected value";
}

TEST(StringTest, compareOperatorsWithStringObject)
{
    pplib::String s1("aaa");
    pplib::String s2("bbb");
    ASSERT_TRUE(s1 < s2) << "String has unexpected value";
    ASSERT_TRUE(s1 <= s2) << "String has unexpected value";
    ASSERT_TRUE(s1 <= s1) << "String has unexpected value";
    ASSERT_FALSE(s2 <= s1) << "String has unexpected value";
    ASSERT_TRUE(s2 > s1) << "String has unexpected value";
    ASSERT_FALSE(s1 > s2) << "String has unexpected value";
    ASSERT_TRUE(s2 >= s1) << "String has unexpected value";
    ASSERT_TRUE(s2 >= s2) << "String has unexpected value";
    ASSERT_FALSE(s1 >= s2) << "String has unexpected value";
    ASSERT_FALSE(s2 < s1) << "String has unexpected value";
    ASSERT_TRUE(s1 == s1) << "String has unexpected value";
    ASSERT_TRUE(s1 != s2) << "String has unexpected value";
    ASSERT_FALSE(s1 != s1) << "String has unexpected value";
}

TEST(StringTest, compareOperatorsWithConstCharPtr)
{
    pplib::String s1("bbb");
    ASSERT_TRUE(s1 < "ccc") << "String has unexpected value";
    ASSERT_FALSE(s1 < "aaa") << "String has unexpected value";
    ASSERT_TRUE(s1 <= "ccc") << "String has unexpected value";
    ASSERT_FALSE(s1 <= "aaa") << "String has unexpected value";
    ASSERT_TRUE(s1 > "aaa") << "String has unexpected value";
    ASSERT_TRUE(s1 >= "aaa") << "String has unexpected value";
    ASSERT_FALSE(s1 > "ccc") << "String has unexpected value";
    ASSERT_FALSE(s1 >= "ccc") << "String has unexpected value";
    ASSERT_TRUE(s1 >= "aaa") << "String has unexpected value";
    ASSERT_TRUE(s1 == "bbb") << "String has unexpected value";
    ASSERT_TRUE(s1 != "aaa") << "String has unexpected value";
    ASSERT_FALSE(s1 != "bbb") << "String has unexpected value";
}

} // namespace
