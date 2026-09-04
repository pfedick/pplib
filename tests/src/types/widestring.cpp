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
#include <pplib/core/functions.h>

#include "pplib-tests.h"

namespace
{

TEST(WideStringTest, ConstructorSimple)
{
    ASSERT_NO_THROW({
        pplib::WideString s1;
        ASSERT_TRUE(s1.len() == 0) << "String does not have zero length";
        const wchar_t* buf = s1.getPtr();
        ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
        ASSERT_EQ(0, buf[0]) << "Class did not return an empty c-string";
    });
}

TEST(WideStringTest, ConstructorFromCharPtr)
{
    ASSERT_NO_THROW({
        pplib::WideString s1("A test string with unicode characters: äöü");
        ASSERT_EQ((size_t)42, s1.len()) << "String does not have length of 42";
        const unsigned char* buf = (const unsigned char*)s1.getPtr();
        ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
        ASSERT_EQ('A', (unsigned char)s1[0]) << "Unexpected Character in string";
        ASSERT_EQ(L'ü', (unsigned char)s1[41]) << "Unexpected Character in string";
    });
}

TEST(WideStringTest, ConstructorFromWideCharPtr)
{
    ASSERT_NO_THROW({
        pplib::WideString s1(L"A test string with unicode characters: äöü");
        ASSERT_EQ((size_t)42, s1.len()) << "String does not have length of 42";
        const unsigned char* buf = (const unsigned char*)s1.getPtr();
        ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
        ASSERT_EQ(L'A', (wchar_t)s1[0]) << "Unexpected Character in string";
        ASSERT_EQ(L'ü', (wchar_t)s1[41]) << "Unexpected Character in string";
    });
}

TEST(WideStringTest, ConstructorFromWideCharPtrWithSize)
{
    ASSERT_NO_THROW({
        pplib::WideString s1(L"A test string with unicode characters: äöü", 10);
        ASSERT_EQ((size_t)10, s1.len()) << "String does not have length of 42";
        ASSERT_EQ(pplib::WideString(L"A test str"), s1) << "String does not have expected value";
    });
}

TEST(WideStringTest, ConstructorFromString)
{
    ASSERT_NO_THROW({
        pplib::String s1("A test string with unicode characters: äöü");
        pplib::WideString s2(s1);
        ASSERT_EQ((size_t)42, s2.len()) << "String does not have length of 45";
        const wchar_t* buf = s2.getPtr();
        ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
        ASSERT_EQ(L'A', (wchar_t)s2[0]) << "Unexpected Character in string";
        ASSERT_EQ(L'ü', (wchar_t)s2[41]) << "Unexpected Character in string";
    });
}

TEST(WideStringTest, ConstructorFromWideString)
{
    ASSERT_NO_THROW({
        pplib::WideString s1(L"A test string with unicode characters: äöü");
        pplib::WideString s2(s1);
        ASSERT_EQ((size_t)42, s2.len()) << "String does not have expected length";
        const wchar_t* buf = s2.getPtr();
        ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
        ASSERT_EQ(L'A', (wchar_t)s2[0]) << "Unexpected Character in string";
        ASSERT_EQ(L'ü', (wchar_t)s2[41]) << "Unexpected Character in string";
    });
}

TEST(WideStringTest, ConstructorFromWideStringWithMoveOperator)
{
    ASSERT_NO_THROW({
        pplib::WideString s1(L"A test string with unicode characters: äöü");
        pplib::WideString s2(std::move(s1));
        ASSERT_EQ((size_t)42, s2.len()) << "String does not have expected length";
        const wchar_t* buf = s2.getPtr();
        ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
        ASSERT_EQ(L'A', (wchar_t)s2[0]) << "Unexpected Character in string";
        ASSERT_EQ(L'ü', (wchar_t)s2[41]) << "Unexpected Character in string";
    });
}

TEST(WideStringTest, ConstructorFromStdString)
{
    ASSERT_NO_THROW({
        std::string s1("A test string with unicode characters: äöü");
        pplib::WideString s2(s1);
        ASSERT_EQ((size_t)42, s2.len()) << "String does not have expected length";
        const wchar_t* buf = s2.getPtr();
        ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
        ASSERT_EQ(L'A', (wchar_t)s2[0]) << "Unexpected Character in string";
        ASSERT_EQ(L'ü', (wchar_t)s2[41]) << "Unexpected Character in string";
    });
}

TEST(WideStringTest, ConstructorFromStdWString)
{
    ASSERT_NO_THROW({
        std::wstring s1(L"A test string with unicode characters: äöü");
        ASSERT_EQ((size_t)42, s1.size()) << "std:wstring has unexpected size";
        pplib::WideString s2(s1);
        ASSERT_EQ((size_t)42, s2.len()) << "String does not have expected length";
        const wchar_t* buf = s2.getPtr();
        ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
        ASSERT_EQ(L'A', (wchar_t)s2[0]) << "Unexpected Character in string";
        ASSERT_EQ((wchar_t)L'ü', (wchar_t)s2[41]) << "Unexpected Character in string";
    });
}

TEST(WideStringTest, ConstructorFromByteArrayPtr)
{
    const wchar_t* data = L"A test string with unicode characters: äöü";
    pplib::ByteArrayPtr b1(data, 42 * sizeof(wchar_t));
    pplib::WideString s2(b1);
    ASSERT_EQ((size_t)42, s2.len()) << "String does not have length of 45";
    ASSERT_EQ(pplib::WideString(L"A test string with unicode characters: äöü"), s2) << "String does not have expected value";
}

TEST(WideStringTest, clear)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    s1.clear();
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    const wchar_t* buf = s1.getPtr();
    ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
    ASSERT_THROW({ ASSERT_EQ(0, (unsigned char)s1[0]) << "Unexpected Character in string"; }, pplib::OutOfBoundsException);
}

TEST(WideStringTest, CapacityAndReserve)
{
    pplib::WideString s1;
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

TEST(WideStringTest, ReserveThrowsExceptions)
{
    pplib::WideString s1;
    ASSERT_THROW(s1.reserve(std::numeric_limits<size_t>::max()), pplib::IllegalArgumentException)
        << "reserve did not throw IllegalArgumentException";

    ASSERT_THROW(s1.reserve(std::numeric_limits<size_t>::max() / sizeof(wchar_t) - 2), pplib::OutOfMemoryException)
        << "reserve did not throw OutOfMemoryException";
}

TEST(WideStringTest, len)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    ASSERT_EQ(s1.len(), (size_t)42) << "String has unexpected size";
}

TEST(WideStringTest, length)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    ASSERT_EQ(s1.length(), (size_t)42) << "String has unexpected size";
}

TEST(WideStringTest, byteLength)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    ASSERT_EQ(s1.byteLength(), (size_t)42 * sizeof(wchar_t)) << "String has unexpected size";
}

TEST(WideStringTest, size)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    ASSERT_EQ(s1.size(), (size_t)42) << "String has unexpected size";
}

TEST(WideStringTest, isEmpty)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    pplib::WideString s2;
    ASSERT_EQ(s1.isEmpty(), false) << "String is empty";
    ASSERT_EQ(s2.isEmpty(), true) << "String not empty";
}

TEST(WideStringTest, notEmpty)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    pplib::WideString s2;
    ASSERT_EQ(s1.notEmpty(), true) << "String is not empty";
    ASSERT_EQ(s2.notEmpty(), false) << "String is empty";
}

TEST(WideStringTest, isNumeric)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    ASSERT_EQ(s1.isNumeric(), false) << "String should not be numeric";
    s1.set(L"");
    ASSERT_EQ(s1.isNumeric(), false) << "String should not be numeric";
    s1.set(L"12345abcd");
    ASSERT_EQ(s1.isNumeric(), false) << "String should not be numeric";
    s1.set(L"12345");
    ASSERT_EQ(s1.isNumeric(), true) << "String should be numeric";
    s1.set(L"-123451,12");
    ASSERT_EQ(s1.isNumeric(), true) << "String should be numeric";
    s1.set(L"-123451.12");
    ASSERT_EQ(s1.isNumeric(), true) << "String should be numeric";
    s1.set(L"123.451,12");
    ASSERT_EQ(s1.isNumeric(), false) << "String should not be numeric";
    s1.set(L"-123.451,12");
    ASSERT_EQ(s1.isNumeric(), false) << "String should not be numeric";
    s1.set(L"-123.451,12-9");
    ASSERT_EQ(s1.isNumeric(), false) << "String should not be numeric";

    ASSERT_FALSE(pplib::WideString("123.45.67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::WideString("123,45,67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::WideString("123.45,67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::WideString("123,45.67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::WideString("123-45-67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::WideString("123-45.67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::WideString("123-45,67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::WideString("123.45-67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::WideString("123,45-67").isNumeric()) << "String should not be numeric";
    ASSERT_FALSE(pplib::WideString("123.").isNumeric()) << "String should not be numeric";

    EXPECT_EQ(pplib::WideString(L"+123").isNumeric(), true) << "String should be numeric";

    ASSERT_FALSE(pplib::WideString(L"+").isInteger()) << "String should not be an integer";
    ASSERT_FALSE(pplib::WideString(L"-").isInteger()) << "String should not be an integer";
    ASSERT_FALSE(pplib::WideString(L".").isInteger()) << "String should not be an integer";
    ASSERT_FALSE(pplib::WideString(L",").isInteger()) << "String should not be an integer";
    ASSERT_FALSE(pplib::WideString(L"+-").isInteger()) << "String should not be an integer";
    ASSERT_FALSE(pplib::WideString(L"-+").isInteger()) << "String should not be an integer";
    ASSERT_FALSE(pplib::WideString(L"+-123").isInteger()) << "String should not be an integer";
    ASSERT_FALSE(pplib::WideString(L"-+123").isInteger()) << "String should not be an integer";
    ASSERT_FALSE(pplib::WideString(L"123+").isInteger()) << "String should not be an integer";
    ASSERT_FALSE(pplib::WideString(L"123-").isInteger()) << "String should not be an integer";
}

TEST(WideStringTest, isInteger)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    ASSERT_EQ(s1.isInteger(), false) << "String should not be an integer";
    s1.set(L"");
    ASSERT_EQ(s1.isInteger(), false) << "String should not be an integer";
    s1.set(L"12345abcd");
    ASSERT_EQ(s1.isInteger(), false) << "String should not be an integer";
    s1.set(L"12345");
    ASSERT_EQ(s1.isInteger(), true) << "String should be an integer";
    s1.set(L"-12345");
    ASSERT_EQ(s1.isInteger(), true) << "String should be an integer";
    s1.set(L"123-451");
    ASSERT_EQ(s1.isInteger(), false) << "String should be an integer";
    s1.set(L"123.451,12");
    ASSERT_EQ(s1.isInteger(), false) << "String should be an integer";
    s1.set(L"-123.451,12");
    ASSERT_EQ(s1.isInteger(), false) << "String should be an integer";
    s1.set(L"-123.451,12-9");
    ASSERT_EQ(s1.isInteger(), false) << "String should not be an integer";

    EXPECT_EQ(pplib::WideString(L"+123").isInteger(), true) << "String should be an integer";
    EXPECT_EQ(pplib::WideString().isInteger(), false) << "Empty string should not be an integer";
    EXPECT_EQ(pplib::WideString(L"-").isInteger(), false) << "String should not be an integer";
    EXPECT_EQ(pplib::WideString(L".").isInteger(), false) << "String should not be an integer";
    EXPECT_EQ(pplib::WideString(L"+").isInteger(), false) << "String should not be an integer";
    EXPECT_EQ(pplib::WideString(L"+-").isInteger(), false) << "String should not be an integer";
    EXPECT_EQ(pplib::WideString(L"-+").isInteger(), false) << "String should not be an integer";
    EXPECT_EQ(pplib::WideString(L"+-123").isInteger(), false) << "String should not be an integer";
    EXPECT_EQ(pplib::WideString(L"-+123").isInteger(), false) << "String should not be an integer";
    EXPECT_EQ(pplib::WideString(L"123+").isInteger(), false) << "String should not be an integer";
    EXPECT_EQ(pplib::WideString(L"123-").isInteger(), false) << "String should not be an integer";
}

TEST(WideStringTest, isTrue)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    ASSERT_EQ(s1.isTrue(), false) << "String should not be true";
    s1.set(L"");
    ASSERT_EQ(s1.isTrue(), false) << "String should not be true";
    s1.set(L"12345abcd");
    ASSERT_EQ(s1.isTrue(), true) << "String should be true";
    s1.set(L"1");
    ASSERT_EQ(s1.isTrue(), true) << "String should be true";
    s1.set(L"12345");
    ASSERT_EQ(s1.isTrue(), true) << "String should be true";
    s1.set(L"true");
    ASSERT_EQ(s1.isTrue(), true) << "String should be true";
    s1.set(L"wahr");
    ASSERT_EQ(s1.isTrue(), true) << "String should be true";
    s1.set(L"ja");
    ASSERT_EQ(s1.isTrue(), true) << "String should be true";
    s1.set(L"yes");
    ASSERT_EQ(s1.isTrue(), true) << "String should be true";
    s1.set(L"false");
    ASSERT_EQ(s1.isTrue(), false) << "String should not be true";
    s1.set(L"falsch");
    ASSERT_EQ(s1.isTrue(), false) << "String should not be true";
    s1.set(L"nein");
    ASSERT_EQ(s1.isTrue(), false) << "String should not be true";
    s1.set(L"no");
    ASSERT_EQ(s1.isTrue(), false) << "String should not be true";
    ASSERT_TRUE(pplib::WideString("t").isTrue()) << "String should be true";
}

TEST(WideStringTest, isFalse)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    ASSERT_EQ(s1.isFalse(), true) << "String should be false";
    s1.set(L"");
    ASSERT_EQ(s1.isFalse(), true) << "String should be false";
    s1.set(L"12345abcd");
    ASSERT_EQ(s1.isFalse(), false) << "String should not be false";
    s1.set(L"12345");
    ASSERT_EQ(s1.isFalse(), false) << "String should not be false";
    s1.set(L"0");
    ASSERT_EQ(s1.isFalse(), true) << "String should be false";
    s1.set(L"true");
    ASSERT_EQ(s1.isFalse(), false) << "String should not be false";
    s1.set(L"wahr");
    ASSERT_EQ(s1.isFalse(), false) << "String should not be false";
    s1.set(L"ja");
    ASSERT_EQ(s1.isFalse(), false) << "String should not be false";
    s1.set(L"yes");
    ASSERT_EQ(s1.isFalse(), false) << "String should not be false";
    s1.set(L"false");
    ASSERT_EQ(s1.isFalse(), true) << "String should be false";
    s1.set(L"falsch");
    ASSERT_EQ(s1.isFalse(), true) << "String should be false";
    s1.set(L"nein");
    ASSERT_EQ(s1.isFalse(), true) << "String should be false";
    s1.set(L"no");
    ASSERT_EQ(s1.isFalse(), true) << "String should be false";
}

TEST(WideStringTest, setConstCharWithoutSize)
{
    pplib::WideString s2(L"äöü, a test string with unicode characters");
    pplib::WideString s1;
    s1.set((const char*)"äöü, a test string with unicode characters");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
    ASSERT_EQ((size_t)42, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, setConstCharWithSize)
{
    pplib::WideString s2(L"äöü, a ");
    pplib::WideString s1;
    s1.set((const char*)"äöü, a test string with unicode characters", 10);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)7, s1.size()) << "String has unexpected length";

    // Size is bigger than the string length
    s1.set((const char*)"äöü, a test string with unicode characters", 100);
    ASSERT_EQ((size_t)42, s1.size()) << "String has unexpected length";
    ASSERT_EQ(pplib::WideString(L"äöü, a test string with unicode characters"), s1) << "String has unexpected value";
}

TEST(WideStringTest, setConstCharWithNull)
{
    pplib::WideString s2(L"äöü, a test string with unicode characters");
    pplib::WideString s1;
    s1.set((const char*)nullptr);
    ASSERT_EQ((size_t)0, s1.size()) << "String has unexpected length";
    s1.set((const char*)"Hello World", 0);
    ASSERT_EQ((size_t)0, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, SetCharInvalidCharacterEncoding)
{
    const char* invalid_wstr = "abc\x80xyz";
    pplib::WideString str;
    EXPECT_THROW(str.set(invalid_wstr), pplib::CharacterEncodingException);
}

TEST(WideStringTest, setConstWChartWithoutSize)
{
    pplib::WideString s2(L"äöü, a test string with unicode characters");
    pplib::WideString s1;
    s1.set(L"äöü, a test string with unicode characters");
    ASSERT_EQ(s1, s2) << "String as unexpected value";
    ASSERT_EQ((size_t)42, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, setConstWChartWithSize)
{
    pplib::WideString s2(L"äöü, a tes");
    pplib::WideString s1;
    s1.set(L"äöü, a test string with unicode characters", 10);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)10, s1.size()) << "String has unexpected length";

    // size is honored exactly, even if larger than the source's own length would allow to guess -
    // the caller is responsible for passing an accurate size
    s1.set(L"äöü, a test string with unicode characters", 43);
    ASSERT_EQ((size_t)43, s1.size()) << "String has unexpected length";

    // an embedded 0-character is preserved, not treated as end of string
    pplib::WideString s3(L"Hello World");
    s3.set(5, (wchar_t)0);
    pplib::WideString s4;
    s4.set(s3.getPtr(), s3.size());
    ASSERT_EQ((size_t)11, s4.size()) << "String has unexpected length";
}

TEST(WideStringTest, setConstWChartWithNull)
{
    pplib::WideString s2(L"äöü, a test string with unicode characters");
    pplib::WideString s1;
    s1.set((const wchar_t*)nullptr);
    ASSERT_EQ((size_t)0, s1.size()) << "String has unexpected length";
    s1.set((const wchar_t*)L"Hello World", 0);
    ASSERT_EQ((size_t)0, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, setConstWChartFromSameMemory)
{
    pplib::WideString s1(L"äöü, a test string with unicode characters");
    s1.set((const wchar_t*)s1.getPtr(), 10);
    ASSERT_EQ(pplib::WideString(L"äöü, a tes"), s1) << "String as unexpected value";
    ASSERT_EQ((size_t)10, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, setWideStringRefWithoutSize)
{
    pplib::WideString s2(L"äöü, a test string with unicode characters");
    pplib::WideString s1;
    s1.set(s2);
    ASSERT_EQ(s1, s2) << "String as unexpected value";
    ASSERT_EQ((size_t)42, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, setWideStringRefWithSize)
{
    pplib::WideString s2(L"äöü, a tes");
    pplib::WideString s3(L"äöü, a test string with unicode characters");
    pplib::WideString s1;
    s1.set(s3, 10);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)10, s1.size()) << "String has unexpected length";

    // Size is bigger than the string length
    s1.set(s3, 100);
    ASSERT_EQ((size_t)42, s1.size()) << "String has unexpected length";
    ASSERT_EQ(pplib::WideString(L"äöü, a test string with unicode characters"), s1) << "String has unexpected value";
}

TEST(WideStringTest, setStringRefWithoutSize)
{
    pplib::String s2("äöü, a test string with unicode characters");
    pplib::WideString s1;
    s1.set(s2);
    ASSERT_EQ(pplib::WideString(L"äöü, a test string with unicode characters"), s1) << "String has unexpected value";
    ASSERT_EQ((size_t)42, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, setStringRefWithSize)
{
    pplib::WideString s2(L"äöü, a tes");
    pplib::String s3("äöü, a test string with unicode characters");
    pplib::WideString s1;
    s1.set(s3, 13);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)10, s1.size()) << "String has unexpected length";

    // Size is bigger than the string length
    s1.set(s3, 100);
    ASSERT_EQ((size_t)42, s1.size()) << "String has unexpected length";
    ASSERT_EQ(pplib::WideString(L"äöü, a test string with unicode characters"), s1) << "String has unexpected value";
}

TEST(WideStringTest, setSTDStringRefWithoutSize)
{
    pplib::WideString s2(L"äöü, a test string with unicode characters");
    std::string s3((const char*)"äöü, a test string with unicode characters");
    pplib::WideString s1;
    s1.set(s3);
    ASSERT_EQ(s1, s2) << "String as unexpected value";
    ASSERT_EQ((size_t)42, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, setSTDStringRefWithSize)
{
    pplib::WideString s2(L"äöü, a ");
    std::string s3((const char*)"äöü, a test string with unicode characters");
    pplib::WideString s1;
    s1.set(s3, 10);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)7, s1.size()) << "String has unexpected length";

    // Size is bigger than the string length
    s1.set(s3, 100);
    ASSERT_EQ((size_t)42, s1.size()) << "String has unexpected length";
    ASSERT_EQ(pplib::WideString(L"äöü, a test string with unicode characters"), s1) << "String has unexpected value";
}

TEST(WideStringTest, setSTDWStringRefWithoutSize)
{
    pplib::WideString s2(L"äöü, a test string with unicode characters");
    std::wstring s3(L"äöü, a test string with unicode characters");
    pplib::WideString s1;
    s1.set(s3);
    ASSERT_EQ(s1, s2) << "String as unexpected value";
    ASSERT_EQ((size_t)42, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, setSTDWStringRefWithSize)
{
    pplib::WideString s2(L"äöü, a tes");
    std::wstring s3(L"äöü, a test string with unicode characters");
    pplib::WideString s1;
    s1.set(s3, 10);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)10, s1.size()) << "String has unexpected length";

    // Size is bigger than the string length
    s1.set(s3, 100);
    ASSERT_EQ((size_t)42, s1.size()) << "String has unexpected length";
    ASSERT_EQ(pplib::WideString(L"äöü, a test string with unicode characters"), s1) << "String has unexpected value";
}

TEST(WideStringTest, setByteArrayPtr)
{
    pplib::ByteArray ba(L"äöü, a test string with unicode characters", 42 * sizeof(wchar_t));
    pplib::WideString s2;
    s2.set(ba);
    ASSERT_EQ(pplib::WideString(L"äöü, a test string with unicode characters"), s2) << "String has unexpected value";
    ASSERT_EQ((size_t)42, s2.size()) << "String has unexpected length";
}

TEST(WideStringTest, setByteArrayPtrWithSize)
{
    pplib::ByteArray ba(L"äöü, a test string with unicode characters", 42 * sizeof(wchar_t));
    pplib::WideString s2;
    s2.set(ba, 10);
    ASSERT_EQ(pplib::WideString(L"äöü, a tes"), s2) << "String has unexpected value";
    ASSERT_EQ((size_t)10, s2.size()) << "String has unexpected length";
}

TEST(WideStringTest, setToPosition)
{
    pplib::WideString s2(L"äöü, a test string with unicode characters");
    s2.set(1, L'h');
    ASSERT_EQ(pplib::WideString(L"ähü, a test string with unicode characters"), s2) << "String has unexpected value";

    ASSERT_THROW(s2.set(100, L'h'), pplib::OutOfBoundsException) << "set() did not throw OutOfBoundsException";
}

TEST(WideStringTest, setf)
{
    pplib::WideString s2(L"Ein Test, 42, Wide, 10000");
    pplib::WideString s1;
    s1.setf("Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)25, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, format)
{
    pplib::WideString s1 = pplib::WideString::format("Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
    pplib::WideString s2(L"Ein Test, 42, Wide, 10000");
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)25, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, setWchart)
{
    pplib::WideString s2(L"a");
    pplib::WideString s1;
    s1.set((wchar_t)'a');
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)1, s1.size()) << "String has unexpected length";
}

static void test_vasprintf(pplib::WideString& str, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    str.vasprintf(fmt, args);
    va_end(args);
}

TEST(WideStringTest, vasprintf)
{
    pplib::WideString s2(L"Ein Test, 42, Wide, 10000");
    pplib::WideString s1;
    test_vasprintf(s1, "Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)25, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, appendConstWchartWithoutSize)
{
    pplib::WideString expected(L"First Part äöü, äöü Second Part");
    pplib::WideString s1(L"First Part äöü, ");
    s1.append(L"äöü Second Part");

    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, appendConstWchartWithSize)
{
    pplib::WideString expected(L"First Part äöü, äöü S");
    pplib::WideString s1(L"First Part äöü, ");
    s1.append(L"äöü Second Part", 5);

    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)21, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, append_same_string)
{
    pplib::WideString expected(L"It's me, It's me, It's me, It's me, ");
    pplib::WideString s1(L"It's me, ");
    s1.append(s1);
    s1.append(s1);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)36, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, appendOnEmptyString)
{
    pplib::WideString expected(L"äöü Second Part");
    pplib::WideString s1;
    s1.append(L"äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)15, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.append("äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)15, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.append(pplib::WideString(L"äöü Second Part"));
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)15, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.append(pplib::String("äöü Second Part"));
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)15, s1.size()) << "String has unexpected length";

    s1.clear();
    std::wstring s3(L"äöü Second Part");
    s1.append(s3);
    ASSERT_EQ((size_t)15, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, appendNull)
{
    pplib::WideString s1("First Part äöü, ");
    s1.append((const wchar_t*)nullptr, 1);
    ASSERT_EQ(pplib::WideString("First Part äöü, "), s1) << "String has unexpected value";

    s1.append((const wchar_t*)L"Hello World", 0);
    ASSERT_EQ(pplib::WideString("First Part äöü, "), s1) << "String has unexpected value";
}

TEST(WideStringTest, append_gemoetric_increase)
{
    pplib::WideString s1(L"Hello World");
    ASSERT_EQ((size_t)11, s1.capacity()) << "String has unexpected capacity";
    s1.append(L" and a lot of other text to increase the capacity of the string");
    // Eine Verdopplung wird hier nicht reichen. Die neue Kapazität muss
    // daher mindestens 11 + 63 = 74 sein. Exakt sollte sie 74 + 16 = 90 sein.
    ASSERT_GE(s1.capacity(), (size_t)74) << "String has unexpected capacity";
    ASSERT_EQ(s1.capacity(), (size_t)90) << "String has unexpected capacity";

    // Jetzt Appenden wir etwas kleines, was in die vorhandene Capacity passt.
    s1.append(L"!");
    ASSERT_EQ(s1.capacity(), (size_t)90) << "String has unexpected capacity";
}

TEST(WideStringTest, appendConstCharPtrWithoutSize)
{
    pplib::WideString expected(L"First Part äöü, äöü Second Part");
    pplib::WideString s1(L"First Part äöü, ");
    s1.append((const char*)"äöü Second Part");

    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, appendConstCharPtrWithSize)
{
    pplib::WideString expected(L"First Part äöü, äöü Sec");
    pplib::WideString s1(L"First Part äöü, ");
    s1.append((const char*)"äöü Second Part", 10);

    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)23, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, appendWideStringWithoutSize)
{
    pplib::WideString expected(L"First Part äöü, äöü Second Part");
    pplib::WideString s1(L"First Part äöü, ");
    pplib::WideString s2(L"äöü Second Part");
    s1.append(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, appendWideStringWithSize)
{
    pplib::WideString expected(L"First Part äöü, äöü Seco");
    pplib::WideString s1(L"First Part äöü, ");
    pplib::WideString s2(L"äöü Second Part");
    s1.append(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)24, s1.size()) << "String has unexpected length";

    // Size is bigger than String
    s1.set(L"First Part äöü, ");
    s1.append(s2, 100);
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
    ASSERT_EQ(pplib::WideString(L"First Part äöü, äöü Second Part"), s1) << "String has unexpected value";
}

TEST(WideStringTest, appendStringWithoutSize)
{
    pplib::WideString expected(L"First Part äöü, äöü Second Part");
    pplib::WideString s1(L"First Part äöü, ");
    pplib::String s2("äöü Second Part");
    s1.append(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    // pplib::HexDump(s1.getPtr(), s1.size() * sizeof(wchar_t));
}

TEST(WideStringTest, appendStringWithSize)
{
    pplib::WideString expected(L"First Part äöü, äöü Seco");
    pplib::WideString s1(L"First Part äöü, ");
    pplib::String s2("äöü Second Part");
    s1.append(s2, 11);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)24, s1.size()) << "String has unexpected length";

    // Size is bigger than String
    s1.set(L"First Part äöü, ");
    s1.append(s2, 100);
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
    ASSERT_EQ(pplib::WideString(L"First Part äöü, äöü Second Part"), s1) << "String has unexpected value";
}

TEST(WideStringTest, appendStdStringWithoutSize)
{
    pplib::WideString expected(L"First Part äöü, äöü Second Part");
    pplib::WideString s1(L"First Part äöü, ");
    std::string s2((const char*)"äöü Second Part");
    s1.append(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, appendStdStringWithSize)
{
    pplib::WideString expected(L"First Part äöü, äöü S");
    pplib::WideString s1(L"First Part äöü, ");
    std::string s2((const char*)"äöü Second Part");
    s1.append(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)21, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, appendStdWStringWithoutSize)
{
    pplib::WideString expected(L"First Part äöü, äöü Second Part");
    pplib::WideString s1(L"First Part äöü, ");
    std::wstring s2(L"äöü Second Part");
    s1.append(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, appendStdWStringWithSize)
{
    pplib::WideString expected(L"First Part äöü, äöü S");
    pplib::WideString s1(L"First Part äöü, ");
    std::wstring s2(L"äöü Second Part");
    s1.append(s2, 5);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)21, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, appendf)
{
    pplib::WideString expected(L"First Part äöü, Ein Test, 42, Wide, 10000");
    pplib::WideString s1(L"First Part äöü, ");
    s1.appendf("Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)41, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, appendWchart)
{
    pplib::WideString expected(L"First Part äöü, a");
    pplib::WideString s1(L"First Part äöü, ");
    s1.append((wchar_t)'a');
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)17, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prependConstWchartWithoutSize)
{
    pplib::WideString expected(L"äöü Second PartFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    s1.prepend(L"äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prependConstWchartWithSize)
{
    pplib::WideString expected(L"äöü SFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    s1.prepend(L"äöü Second Part", 5);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)21, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prependConstWchartOnEmptyString)
{
    pplib::WideString expected(L"äöü Second Part");
    pplib::WideString s1;
    s1.prepend(L"äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)15, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prependConstWchartWhichIsEmpty)
{
    pplib::WideString expected(L"äöü Second Part");
    pplib::WideString s1(L"äöü Second Part");
    s1.prepend((const wchar_t*)nullptr, 10);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)15, s1.size()) << "String has unexpected length";
    s1.prepend((const wchar_t*)L"Hello World", 0);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)15, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prependConstCharPtrWithoutSize)
{
    pplib::WideString expected(L"äöü Second PartFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    s1.prepend((const char*)"äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prependConstCharPtrWithSize)
{
    pplib::WideString expected(L"äöü SFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    s1.prepend((const char*)"äöü Second Part", 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)21, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prepend_same_string)
{
    pplib::WideString expected("It's me, It's me, It's me, It's me, ");
    pplib::WideString s1("It's me, ");
    s1.prepend(s1);
    s1.prepend(s1);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)36, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prepend_gemoetric_increase)
{
    pplib::WideString s1(L"Hello World");
    ASSERT_EQ((size_t)11, s1.capacity()) << "String has unexpected capacity";
    s1.prepend(L" and a lot of other text to increase the capacity of the string");
    // Eine Verdopplung wird hier nicht reichen. Die neue Kapazität muss
    // daher mindestens 11 + 63 = 74 sein. Exakt sollte sie 74 + 16 = 90 sein.
    ASSERT_GE(s1.capacity(), (size_t)74) << "String has unexpected capacity";
    ASSERT_EQ(s1.capacity(), (size_t)90) << "String has unexpected capacity";

    // Jetzt Appenden wir etwas kleines, was in die vorhandene Capacity passt.
    s1.prepend(L"!");
    ASSERT_EQ(s1.capacity(), (size_t)90) << "String has unexpected capacity";
}

TEST(WideStringTest, prependWideStringWithoutSize)
{
    pplib::WideString expected(L"äöü Second PartFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    pplib::WideString s2(L"äöü Second Part");
    s1.prepend(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prependWideStringWithSize)
{
    pplib::WideString expected(L"äöü SFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    pplib::WideString s2(L"äöü Second Part");
    s1.prepend(s2, 5);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)21, s1.size()) << "String has unexpected length";

    // Size is bigger than String
    s1.set(L"First Part äöü, ");
    s1.prepend(s2, 100);
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
    ASSERT_EQ(pplib::WideString(L"äöü Second PartFirst Part äöü, "), s1) << "String has unexpected value";
}

TEST(WideStringTest, prependStringWithoutSize)
{
    pplib::WideString expected(L"äöü Second PartFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    pplib::String s2("äöü Second Part");
    s1.prepend(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prependStringWithSize)
{
    pplib::WideString expected(L"äöü SFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    pplib::String s2("äöü Second Part");
    s1.prepend(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)21, s1.size()) << "String has unexpected length";
    // Size is bigger than String
    s1.set(L"First Part äöü, ");
    s1.prepend(s2, 100);
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
    ASSERT_EQ(pplib::WideString(L"äöü Second PartFirst Part äöü, "), s1) << "String has unexpected value";
}

TEST(WideStringTest, prependStdStringWithoutSize)
{
    pplib::WideString expected(L"äöü Second PartFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    std::string s2((const char*)"äöü Second Part");
    s1.prepend(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prependStdStringWithSize)
{
    pplib::WideString expected(L"äöü SFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    std::string s2((const char*)"äöü Second Part");
    s1.prepend(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)21, s1.size()) << "String has unexpected length";

    // Size is bigger than String
    s1.set(L"First Part äöü, ");
    s1.prepend(s2, 100);
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
    ASSERT_EQ(pplib::WideString(L"äöü Second PartFirst Part äöü, "), s1) << "String has unexpected value";
}

TEST(WideStringTest, prependStdWStringWithoutSize)
{
    pplib::WideString expected(L"äöü Second PartFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    std::wstring s2(L"äöü Second Part");
    s1.prepend(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prependStdWStringWithSize)
{
    pplib::WideString expected(L"äöü SFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    std::wstring s2(L"äöü Second Part");
    s1.prepend(s2, 5);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)21, s1.size()) << "String has unexpected length";

    // Size is bigger than String
    s1.set(L"First Part äöü, ");
    s1.prepend(s2, 100);
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
    ASSERT_EQ(pplib::WideString(L"äöü Second PartFirst Part äöü, "), s1) << "String has unexpected value";
}

TEST(WideStringTest, prependf)
{
    pplib::WideString expected(L"Ein Test, 42, Wide, 10000First Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    s1.prependf("Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)41, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prependWchart)
{
    pplib::WideString expected(L"aFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    s1.prepend((wchar_t)'a');
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)17, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prependOnEmptyString)
{
    pplib::WideString expected(L"äöü Second Part");
    pplib::WideString s1;
    s1.prepend(L"äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)15, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.prepend("äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)15, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.prepend(pplib::String("äöü Second Part"));
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)15, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.prepend(pplib::WideString(L"äöü Second Part"));
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)15, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.prepend(std::string("äöü Second Part"));
    ASSERT_EQ((size_t)15, s1.size()) << "String has unexpected length";

    s1.clear();
    s1.prepend(std::wstring(L"äöü Second Part"));
    ASSERT_EQ((size_t)15, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prependNull)
{
    pplib::WideString s1("First Part äöü, ");
    s1.prepend((const wchar_t*)nullptr, 1);
    ASSERT_EQ(pplib::WideString("First Part äöü, "), s1) << "String has unexpected value";

    s1.prepend((const wchar_t*)L"Hello World", 0);
    ASSERT_EQ(pplib::WideString("First Part äöü, "), s1) << "String has unexpected value";
}

TEST(WideStringTest, toUtf8)
{
    // Leerer String
    pplib::WideString s1;
    pplib::ByteArray res1 = s1.toUtf8();
    ASSERT_EQ((size_t)0, res1.size()) << "Empty string toUtf8 should be empty";

    // Einfacher ASCII
    pplib::WideString s2(L"ABC");
    pplib::ByteArray res2 = s2.toUtf8();
    ASSERT_EQ((size_t)3, res2.size()) << "ASCII string toUtf8 size mismatch";

    // Unicode BMP Zeichen (äöü)
    pplib::WideString s3(L"äöü");
    pplib::ByteArray res3 = s3.toUtf8();
    ASSERT_EQ((size_t)6, res3.size()) << "BMP Unicode string toUtf8 size mismatch";

    // Größere Codepoints (< 0x10000) -> 3 Bytes in UTF-8
    pplib::WideString s4(L"\u20AC"); // Euro-Zeichen €
    pplib::ByteArray res4 = s4.toUtf8();
    ASSERT_EQ((size_t)3, res4.size()) << "3-byte Codepoint toUtf8 size mismatch";

    // Plattformspezifische Tests
#if defined(_WIN32) || (defined(__SIZEOF_WCHAR_T__) && __SIZEOF_WCHAR_T__ == 2)
    // Surrogate-Paar für 🚀 (U+1F680) - Windows/16-Bit-Systeme
    pplib::WideString s5(L"\xD83D\xDE80");
    pplib::ByteArray res5 = s5.toUtf8();
    ASSERT_EQ((size_t)4, res5.size()) << "4-byte Codepoint (Surrogate Pair) toUtf8 size mismatch";
    const unsigned char* buf5 = (const unsigned char*)res5.ptr();
    ASSERT_EQ(0xF0, buf5[0]);
    ASSERT_EQ(0x9F, buf5[1]);
    ASSERT_EQ(0x9A, buf5[2]);
    ASSERT_EQ(0x80, buf5[3]);
#else
    // Direkter 32-Bit-Codepoint für Unix/macOS (U+1F680)
    pplib::WideString s5(L"\U0001F680");
    pplib::ByteArray res5 = s5.toUtf8();
    ASSERT_EQ((size_t)4, res5.size()) << "4-byte Codepoint (Direct 32-bit) toUtf8 size mismatch";
    const unsigned char* buf5 = (const unsigned char*)res5.ptr();
    ASSERT_EQ(0xF0, buf5[0]);
    ASSERT_EQ(0x9F, buf5[1]);
    ASSERT_EQ(0x9A, buf5[2]);
    ASSERT_EQ(0x80, buf5[3]);
#endif
}

TEST(WideStringTest, toUtf8InvalidSurrogate)
{
    // Ein isolierter Surrogate-Wert ist unabhaengig von der wchar_t-Breite kein gueltiger
    // Unicode-Codepoint (RFC 3629 schliesst Surrogates in UTF-8 explizit aus) und darf nicht
    // klaglos kodiert werden.

    // alleinstehendes High-Surrogate mitten im String, gefolgt von einem normalen Zeichen
    pplib::WideString s1(L"AAA");
    s1.set(1, (wchar_t)0xD800);
    ASSERT_THROW(s1.toUtf8(), pplib::CharacterEncodingException);

    // alleinstehendes Low-Surrogate ohne vorangehendes High-Surrogate
    pplib::WideString s2(L"AAA");
    s2.set(1, (wchar_t)0xDC00);
    ASSERT_THROW(s2.toUtf8(), pplib::CharacterEncodingException);

#if defined(_WIN32) || (defined(__SIZEOF_WCHAR_T__) && __SIZEOF_WCHAR_T__ == 2)
    // High-Surrogate ganz am Ende des Strings, kein nachfolgendes Low-Surrogate mehr vorhanden
    pplib::WideString s3(L"A");
    s3.append((wchar_t)0xD800);
    ASSERT_THROW(s3.toUtf8(), pplib::CharacterEncodingException);
#endif
}

TEST(WideStringTest, fromUtf8)
{
    // Leerer String
    pplib::WideString s1;
    s1.fromUtf8("");
    ASSERT_EQ((size_t)0, s1.len()) << "Empty UTF-8 string should result in empty WideString";

    // Einfacher ASCII
    pplib::WideString s2;
    s2.fromUtf8("ABC");
    ASSERT_EQ(pplib::WideString(L"ABC"), s2) << "ASCII UTF-8 string conversion mismatch";

    // Unicode BMP Zeichen (äöü)
    pplib::WideString s3;
    s3.fromUtf8("äöüTest");
    ASSERT_EQ(pplib::WideString(L"äöüTest"), s3) << "BMP UTF-8 string conversion mismatch";

    // Supplementary Plane Zeichen (🚀)
    pplib::WideString s4;
    s4.fromUtf8("🚀");
    ASSERT_EQ(pplib::WideString(L"🚀"), s4) << "Supplementary Plane UTF-8 string conversion mismatch";

    // 0xF0 bis 0xE0
    pplib::WideString s5;
    s5.fromUtf8("\xF0\x9F\x9A\x80"); // 🚀
    ASSERT_EQ(pplib::WideString(L"🚀"), s5) << "4-byte UTF-8 string conversion mismatch";

    // 0xE0 bis 0xC0 (3-byte UTF-8 sequence)
    pplib::WideString s6;
    s6.fromUtf8("\xE2\x9C\x93"); // ✓
    ASSERT_EQ(pplib::WideString(L"✓"), s6) << "3-byte UTF-8 string conversion mismatch";

    // 0xC0 bis 0x80 (2-byte UTF-8 sequence)
    pplib::WideString s7;
    s7.fromUtf8("\xC3\xA4"); // ä
    ASSERT_EQ(pplib::WideString(L"ä"), s7) << "2-byte UTF-8 string conversion mismatch";
}

TEST(WideStringTest, fromUtf8Invalid)
{
    const wchar_t REPLACEMENT_CHARACTER = (wchar_t)0xFFFD;

    // ungueltiges Continuation-Byte mitten in einer 3-Byte-Sequenz: nur das Lead-Byte wird
    // ersetzt, das restliche kaputte Byte danach ebenfalls, das dazwischenliegende 'A' bleibt erhalten
    {
        pplib::WideString expected;
        expected.append(REPLACEMENT_CHARACTER);
        expected.append(L'A');
        expected.append(REPLACEMENT_CHARACTER);
        pplib::WideString s;
        s.fromUtf8(pplib::ByteArrayPtr("\xE2\x41\x93", 3));
        ASSERT_EQ(expected, s) << "Invalid continuation byte was not replaced correctly";
    }

    // Overlong Encoding von NUL (\xC0\x80) muss abgelehnt werden, nicht zu einem eingebetteten
    // 0-Zeichen fuehren
    {
        pplib::WideString expected;
        expected.append(REPLACEMENT_CHARACTER);
        expected.append(REPLACEMENT_CHARACTER);
        pplib::WideString s;
        s.fromUtf8(pplib::ByteArrayPtr("\xC0\x80", 2));
        ASSERT_EQ(expected, s) << "Overlong encoding should be rejected, not decoded as NUL";
    }

    // Direkt in UTF-8 kodiertes UTF-16-Surrogate (U+D800) ist laut RFC 3629 ungueltig
    {
        pplib::WideString expected;
        expected.append(REPLACEMENT_CHARACTER);
        expected.append(REPLACEMENT_CHARACTER);
        expected.append(REPLACEMENT_CHARACTER);
        pplib::WideString s;
        s.fromUtf8(pplib::ByteArrayPtr("\xED\xA0\x80", 3));
        ASSERT_EQ(expected, s) << "Encoded surrogate should be rejected";
    }

    // Codepoint jenseits von U+10FFFF (rechnerisch durch das 4-Byte-Bitmuster darstellbar, aber
    // kein gueltiger Unicode-Codepoint)
    {
        pplib::WideString expected;
        for (int i = 0; i < 4; i++)
            expected.append(REPLACEMENT_CHARACTER);
        pplib::WideString s;
        s.fromUtf8(pplib::ByteArrayPtr("\xF7\xBF\xBF\xBF", 4));
        ASSERT_EQ(expected, s) << "Codepoint beyond U+10FFFF should be rejected";
    }

    // Am Ende der Eingabe abgeschnittene Mehrbyte-Sequenz
    {
        pplib::WideString expected;
        expected.append(L'A');
        expected.append(REPLACEMENT_CHARACTER);
        expected.append(REPLACEMENT_CHARACTER);
        pplib::WideString s;
        s.fromUtf8(pplib::ByteArrayPtr("A\xE2\x9C", 3));
        ASSERT_EQ(expected, s) << "Truncated multi-byte sequence at end of input should be replaced";
    }

    // einzelnes, verirrtes Continuation-Byte ohne vorangehendes Lead-Byte
    {
        pplib::WideString expected;
        expected.append(REPLACEMENT_CHARACTER);
        pplib::WideString s;
        s.fromUtf8(pplib::ByteArrayPtr("\x80", 1));
        ASSERT_EQ(expected, s) << "Lone continuation byte should be replaced";
    }
}

TEST(WideStringTest, toUCS4)
{
    // Leerer String
    pplib::WideString s1;
    pplib::ByteArray res1 = s1.toUCS4();
    ASSERT_EQ((size_t)0, res1.size()) << "Empty string toUCS4 should be empty";

    // Einfacher ASCII (4 Bytes pro Zeichen in UCS4)
    pplib::WideString s2(L"ABC");
    pplib::ByteArray res2 = s2.toUCS4();
    ASSERT_EQ((size_t)12, res2.size()) << "ASCII string toUCS4 size mismatch";

    // Unicode BMP Zeichen (4 Bytes pro Zeichen in UCS4)
    pplib::WideString s3(L"äöü");
    pplib::ByteArray res4 = s3.toUCS4();
    ASSERT_EQ((size_t)12, res4.size()) << "BMP Unicode string toUCS4 size mismatch";

    // Plattformspezifische Tests für Supplementary Planes
#if defined(_WIN32) || (defined(__SIZEOF_WCHAR_T__) && __SIZEOF_WCHAR_T__ == 2)
    // Windows: Surrogate-Paar für 🚀 (U+1F680)
    // Zwei 16-Bit-Elemente werden zu einem 32-Bit-Codepoint zusammengefasst
    pplib::WideString s5(L"\xD83D\xDE80");
    pplib::ByteArray res5 = s5.toUCS4();
    ASSERT_EQ((size_t)4, res5.size()) << "4-byte Codepoint (Surrogate Pair) toUCS4 size mismatch";
#else
    // Unix: Direkter 32-Bit-Codepoint für 🚀 (U+1F680)
    pplib::WideString s5(L"\U0001F680");
    pplib::ByteArray res5 = s5.toUCS4();
    ASSERT_EQ((size_t)4, res5.size()) << "4-byte Codepoint (Direct 32-bit) toUCS4 size mismatch";
#endif
}

TEST(WideStringTest, toUCS4InvalidSurrogate)
{
    // siehe WideStringTest.toUtf8InvalidSurrogate - dieselben ungueltigen Zustaende muessen
    // auch bei der Konvertierung nach UCS4 abgelehnt werden.
    pplib::WideString s1(L"AAA");
    s1.set(1, (wchar_t)0xD800);
    ASSERT_THROW(s1.toUCS4(), pplib::CharacterEncodingException);

    pplib::WideString s2(L"AAA");
    s2.set(1, (wchar_t)0xDC00);
    ASSERT_THROW(s2.toUCS4(), pplib::CharacterEncodingException);

#if defined(_WIN32) || (defined(__SIZEOF_WCHAR_T__) && __SIZEOF_WCHAR_T__ == 2)
    pplib::WideString s3(L"A");
    s3.append((wchar_t)0xD800);
    ASSERT_THROW(s3.toUCS4(), pplib::CharacterEncodingException);
#endif
}

TEST(WideStringTest, fromUCS4)
{
    // Leerer String
    uint32_t empty_ucs4[] = {0};
    pplib::WideString s1;
    s1.fromUCS4(empty_ucs4, 0);
    ASSERT_EQ((size_t)0, s1.len()) << "Empty UCS4 array should result in empty WideString";

    // Einfacher ASCII
    uint32_t ascii_ucs4[] = {65, 66, 67}; // "ABC"
    pplib::WideString s2;
    s2.fromUCS4(ascii_ucs4, 3);
    ASSERT_EQ(pplib::WideString(L"ABC"), s2) << "ASCII UCS4 array conversion mismatch";

    // Unicode BMP Zeichen (äöü)
    uint32_t bmp_ucs4[] = {0x00E4, 0x00F6, 0x00FC};
    pplib::WideString s3;
    s3.fromUCS4(bmp_ucs4, 3);
    ASSERT_EQ(pplib::WideString(L"äöü"), s3) << "BMP UCS4 array conversion mismatch";

    // Größere Codepoints (< 0x10000) -> z.B. Euro-Zeichen € (U+20AC)
    uint32_t large_bmp_ucs4[] = {0x20AC};
    pplib::WideString s4;
    s4.fromUCS4(large_bmp_ucs4, 1);
    ASSERT_EQ(pplib::WideString(L"\u20AC"), s4) << "Large BMP UCS4 array conversion mismatch";

    // Plattformspezifische Tests für Supplementary Planes (>= 0x10000)
#if defined(_WIN32) || (defined(__SIZEOF_WCHAR_T__) && __SIZEOF_WCHAR_T__ == 2)
    // Windows: Surrogate-Paar für 🚀 (U+1F680)
    // Das Ergebnis muss zwei 16-Bit-Elemente im WideString haben
    uint32_t rocket_ucs4[] = {0x1F680};
    pplib::WideString s5;
    s5.fromUCS4(rocket_ucs4, 1);
    ASSERT_EQ((size_t)2, s5.len()) << "Windows: Rocket UCS4 should result in 2 wide characters (surrogate pair)";
    ASSERT_EQ(pplib::WideString(L"\xD83D\xDE80"), s5) << "Windows: Rocket UCS4 conversion mismatch";
#else
    // Unix: Direkter 32-Bit-Codepoint für 🚀 (U+1F680)
    // Das Ergebnis muss ein einzelnes 32-Bit-Element im WideString haben
    uint32_t rocket_ucs4[] = {0x1F680};
    pplib::WideString s5;
    s5.fromUCS4(rocket_ucs4, 1);
    ASSERT_EQ((size_t)1, s5.len()) << "Unix: Rocket UCS4 should result in 1 wide character";
    ASSERT_EQ(pplib::WideString(L"\U0001F680"), s5) << "Unix: Rocket UCS4 conversion mismatch";
#endif
}

TEST(WideStringTest, fromUCS4_withByteArrayPtr)
{
    uint32_t bmp_ucs4[] = {0x00E4, 0x00F6, 0x00FC};
    pplib::ByteArray ucs4_bytes((const char*)bmp_ucs4, sizeof(bmp_ucs4));
    pplib::WideString s;
    s.fromUCS4(ucs4_bytes);
    ASSERT_EQ(pplib::WideString(L"äöü"), s) << "UCS4 ByteArray conversion mismatch";
}

TEST(WideStringTest, toString)
{
    pplib::WideString s1(L"äöü");
    pplib::String str = s1.toString();
    ASSERT_EQ(pplib::String("äöü"), str) << "WideString to String conversion mismatch";
}

TEST(WideStringTest, getChar)
{
    pplib::WideString s1("Hello World!");
    EXPECT_EQ((wchar_t)L'H', s1.get(0)) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'e', s1.get(1)) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'!', s1.get(11)) << "Unexpected Result";
    ASSERT_THROW({ ASSERT_EQ((wchar_t)0, s1.get(12)); }, pplib::OutOfBoundsException);

    EXPECT_EQ((wchar_t)L'!', s1.get(-1)) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'd', s1.get(-2)) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'H', s1.get(-12)) << "Unexpected Result";
    ASSERT_THROW({ ASSERT_EQ((wchar_t)0, s1.get(-13)); }, pplib::OutOfBoundsException);
}

TEST(WideStringTest, OperatorCharPositionPositiv)
{
    pplib::WideString s1(L"Hello World!");
    EXPECT_EQ((wchar_t)L'H', s1[0]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'e', s1[1]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'l', s1[2]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'l', s1[3]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'o', s1[4]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L' ', s1[5]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'W', s1[6]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'o', s1[7]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'r', s1[8]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'l', s1[9]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'd', s1[10]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'!', s1[11]) << "Unexpected Result";

    ASSERT_THROW({ ASSERT_EQ((wchar_t)0, s1[12]); }, pplib::OutOfBoundsException);
}

TEST(WideStringTest, OperatorConstCharPositionPositiv)
{
    const pplib::WideString s1(L"Hello World!");
    EXPECT_EQ((wchar_t)L'H', s1[0]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'e', s1[1]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'l', s1[2]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'l', s1[3]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'o', s1[4]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L' ', s1[5]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'W', s1[6]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'o', s1[7]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'r', s1[8]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'l', s1[9]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'd', s1[10]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'!', s1[11]) << "Unexpected Result";

    ASSERT_THROW({ ASSERT_EQ((wchar_t)0, s1[12]); }, pplib::OutOfBoundsException);
}

TEST(WideStringTest, OperatorCharPositionNegativ)
{
    pplib::WideString s1(L"Hello World!");
    EXPECT_EQ((wchar_t)L'H', s1[0]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'!', s1[-1]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'd', s1[-2]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'l', s1[-3]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'r', s1[-4]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'o', s1[-5]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'W', s1[-6]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L' ', s1[-7]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'o', s1[-8]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'l', s1[-9]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'l', s1[-10]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'e', s1[-11]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'H', s1[-12]) << "Unexpected Result";
    ASSERT_THROW({ ASSERT_EQ((wchar_t)0, s1[-13]); }, pplib::OutOfBoundsException);
}

TEST(WideStringTest, OperatorConstCharPositionNegativ)
{
    const pplib::WideString s1(L"Hello World!");
    EXPECT_EQ((wchar_t)L'H', s1[0]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'!', s1[-1]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'd', s1[-2]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'l', s1[-3]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'r', s1[-4]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'o', s1[-5]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'W', s1[-6]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L' ', s1[-7]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'o', s1[-8]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'l', s1[-9]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'l', s1[-10]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'e', s1[-11]) << "Unexpected Result";
    EXPECT_EQ((wchar_t)L'H', s1[-12]) << "Unexpected Result";
    ASSERT_THROW({ ASSERT_EQ((wchar_t)0, s1[-13]); }, pplib::OutOfBoundsException);
}

TEST(WideStringTest, print)
{
    pplib::WideString s1("Hello World!");
    testing::internal::CaptureStdout();
    s1.print();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ("Hello World!", output) << "Unexpected Result";
}

TEST(WideStringTest, printEmptyString)
{
    pplib::WideString s1;
    testing::internal::CaptureStdout();
    s1.print();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ("", output) << "Unexpected Result";

    testing::internal::CaptureStdout();
    s1.print(true);
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ("\n", output) << "Unexpected Result";
}

TEST(WideStringTest, printnl)
{
    pplib::WideString s1("Hello World!");
    testing::internal::CaptureStdout();
    s1.printnl();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ("Hello World!\n", output) << "Unexpected Result";
}

TEST(WideStringTest, hexDump)
{
    pplib::WideString s1("Hello World!");
    testing::internal::CaptureStdout();
    s1.hexDump();
    pplib::String output = testing::internal::GetCapturedStdout();
    // output.printnl();
#ifdef _WIN32
    EXPECT_TRUE(output.has("48 00 65 00 6C 00 6C 00 6F 00 20 00 57 00 6F 00")) << "Unexpected Result";
    EXPECT_TRUE(output.has("H.e.l.l.o. .W.o.")) << "Unexpected Result";
#else
    // 4byte
    EXPECT_TRUE(output.has("48 00 00 00 65 00 00 00 6C 00 00 00 6C 00 00 00")) << "Unexpected Result";
    EXPECT_TRUE(output.has("H...e...l...l...")) << "Unexpected Result";
#endif
}

TEST(WideStringTest, hexDumpEmptyString)
{
    pplib::WideString s2;
    testing::internal::CaptureStdout();
    s2.hexDump();
    pplib::String output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.has("HEXDUMP of String")) << "Unexpected Result";
}

// Equal Operatoren
TEST(WideStringTest, operatorEqualConstChar)
{
    pplib::WideString s1(L"Hello World!");
    pplib::WideString s2;
    s2 = "Hello World!";
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(WideStringTest, operatorEqualConstWchar)
{
    pplib::WideString s1(L"Hello World!");
    pplib::WideString s2;
    s2 = L"Hello World!";
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(WideStringTest, operatorEqualString)
{
    pplib::WideString s1(L"Hello World!");
    pplib::WideString s2;
    s2 = pplib::String("Hello World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(WideStringTest, operatorEqualWideStringMove)
{
    pplib::WideString s1(L"Hello World!");
    pplib::WideString s2;
    s2 = std::move(pplib::WideString(L"Hello World!"));
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(WideStringTest, operatorEqualStringMoveWithSameString)
{
    pplib::WideString s1(L"Hello World!");
    pplib::WideString s2 = s1;
    s2 = std::move(s2);
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(WideStringTest, operatorEqualWideString)
{
    pplib::WideString s1(L"Hello World!");
    pplib::WideString s2;
    const pplib::WideString s3(L"Hello World!");
    s2 = s3;
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(WideStringTest, operatorEqualStdString)
{
    pplib::WideString s1(L"Hello World!");
    pplib::WideString s2;
    s2 = std::string("Hello World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(WideStringTest, operatorEqualStdWString)
{
    pplib::WideString s1(L"Hello World!");
    pplib::WideString s2;
    s2 = std::wstring(L"Hello World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(WideStringTest, operatorEqualChar)
{
    pplib::WideString s1(L"H");
    pplib::WideString s2;
    s2 = 'H';
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

// Equal Plus Operatoren
TEST(WideStringTest, operatorEqualPlusConstChar)
{
    pplib::WideString s1(L"Hello World!");
    pplib::WideString s2(L"Hello ");
    s2 += "World!";
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(WideStringTest, operatorEqualPlusConstWchar)
{
    pplib::WideString s1(L"Hello World!");
    pplib::WideString s2(L"Hello ");
    s2 += L"World!";
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(WideStringTest, operatorEqualPlusString)
{
    pplib::WideString s1(L"Hello World!");
    pplib::WideString s2(L"Hello ");
    s2 += pplib::String("World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(WideStringTest, operatorEqualPlusWideString)
{
    pplib::WideString s1(L"Hello World!");
    pplib::WideString s2(L"Hello ");
    s2 += pplib::WideString(L"World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}
TEST(WideStringTest, operatorEqualPlusStdString)
{
    pplib::WideString s1(L"Hello World!");
    pplib::WideString s2(L"Hello ");
    s2 += std::string("World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(WideStringTest, operatorEqualPlusStdWString)
{
    pplib::WideString s1(L"Hello World!");
    pplib::WideString s2(L"Hello ");
    s2 += std::wstring(L"World!");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

TEST(WideStringTest, operatorEqualPlusChar)
{
    pplib::WideString s1(L"He");
    pplib::WideString s2(L"H");
    s2 += 'e';
    ASSERT_EQ(s1, s2) << "String has unexpected value";
}

// Operator plus

TEST(WideStringTest, operatorPlus_WideStringPlusWideString)
{
    pplib::WideString s1(L"Hello ");
    pplib::WideString s2(L"World!");
    pplib::WideString s3 = s1 + s2;
    ASSERT_EQ(pplib::WideString(L"Hello World!"), s3) << "String has unexpected value";
}

TEST(WideStringTest, operatorPlus_StringPlusConstCharPtr)
{
    pplib::WideString s1(L"Hello ");
    pplib::WideString s3 = s1 + "World!";
    ASSERT_EQ(pplib::WideString(L"Hello World!"), s3) << "String has unexpected value";
}

TEST(WideStringTest, operatorPlus_StringPlusConstWcharPtr)
{
    pplib::WideString s1(L"Hello ");
    pplib::WideString s3 = s1 + L"World!";
    ASSERT_EQ(pplib::WideString(L"Hello World!"), s3) << "String has unexpected value";
}

TEST(WideStringTest, operatorPlus_StringPlusStdString)
{
    pplib::WideString s1(L"Hello ");
    pplib::WideString s3 = s1 + std::string("World!");
    ASSERT_EQ(pplib::WideString(L"Hello World!"), s3) << "String has unexpected value";
}

TEST(WideStringTest, operatorPlus_StringPlusStdWString)
{
    pplib::WideString s1(L"Hello ");
    pplib::WideString s3 = s1 + std::wstring(L"World!");
    ASSERT_EQ(pplib::WideString(L"Hello World!"), s3) << "String has unexpected value";
}

TEST(WideStringTest, operatorPlus_StringPlusWChart)
{
    pplib::WideString s1(L"He");
    pplib::WideString s3 = s1 + L'l';
    ASSERT_EQ(pplib::WideString(L"Hel"), s3) << "String has unexpected value";
}

TEST(WideStringTest, operatorPlus_ConstCharPtrPlusWideString)
{
    pplib::WideString s1("World!");
    pplib::WideString s3 = "Hello " + s1;
    ASSERT_EQ(pplib::WideString(L"Hello World!"), s3) << "String has unexpected value";
}

TEST(WideStringTest, operatorPlus_ConstWcharPtrPlusWideString)
{
    pplib::WideString s1(L"World!");
    pplib::WideString s3 = L"Hello " + s1;
    ASSERT_EQ(pplib::WideString(L"Hello World!"), s3) << "String has unexpected value";
}

TEST(WideStringTest, operatorPlus_WideStringPlusString)
{
    pplib::WideString s1(L"World!");
    pplib::WideString s3 = pplib::WideString(L"Hello ") + s1;
    ASSERT_EQ(pplib::WideString(L"Hello World!"), s3) << "String has unexpected value";
}

TEST(WideStringTest, operatorPlus_StdStringPlusString)
{
    pplib::WideString s1(L"World!");
    pplib::WideString s3 = std::string("Hello ") + s1;
    ASSERT_EQ(pplib::WideString(L"Hello World!"), s3) << "String has unexpected value";
}

TEST(WideStringTest, operatorPlus_StdWStringPlusString)
{
    pplib::WideString s1(L"World!");
    pplib::WideString s3 = std::wstring(L"Hello ") + s1;
    ASSERT_EQ(pplib::WideString(L"Hello World!"), s3) << "String has unexpected value";
}

TEST(WideStringTest, operatorPlus_WChartPlusString)
{
    pplib::WideString s1(L"el");
    pplib::WideString s3 = L'H' + s1;
    ASSERT_EQ(pplib::WideString(L"Hel"), s3) << "String has unexpected value";
}

// find

TEST(WideStringTest, find_forward)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    ASSERT_EQ((size_t)16, s1.find(L"fox")) << "String has unexpected value";
    ASSERT_EQ((size_t)0, s1.find(L"The")) << "String has unexpected value";
    ASSERT_EQ((size_t)40, s1.find(L"dog", 15)) << "String has unexpected value";
    ASSERT_EQ(pplib::String::npos, s1.find(L"cow")) << "String has unexpected value";
    ASSERT_EQ(pplib::String::npos, s1.find(L"")) << "String has unexpected value";
    ASSERT_EQ(pplib::String::npos, s1.find(L"brown", 100)) << "String has unexpected value";

    pplib::WideString empty;
    ASSERT_EQ(pplib::WideString::npos, empty.find(L"fox")) << "String has unexpected value";
}

TEST(WideStringTest, find_backward)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    ASSERT_EQ((size_t)16, s1.find(L"fox", -1)) << "String has unexpected value";
    ASSERT_EQ((size_t)42, s1.find(L"g", -1)) << "String has unexpected value";
    ASSERT_EQ((size_t)0, s1.find(L"The", -20)) << "String has unexpected value";
    ASSERT_EQ((size_t)pplib::String::npos, s1.find(L"cat", -1)) << "String has unexpected value";
    ASSERT_EQ((size_t)pplib::String::npos, s1.find(L"dog", -100)) << "String has unexpected value";
}

TEST(WideStringTest, findCase)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    ASSERT_EQ((size_t)16, s1.findCase(L"FOX")) << "String has unexpected value";
    ASSERT_EQ((size_t)0, s1.findCase(L"the")) << "String has unexpected value";
    ASSERT_EQ((size_t)40, s1.findCase(L"DOG", 15)) << "String has unexpected value";
    ASSERT_EQ(pplib::WideString::npos, s1.findCase(L"cow")) << "String has unexpected value";
    ASSERT_EQ(pplib::WideString::npos, s1.findCase(L"")) << "String has unexpected value";
    ASSERT_EQ(pplib::WideString::npos, s1.findCase(L"brown", 100)) << "String has unexpected value";

    pplib::WideString empty;
    ASSERT_EQ(pplib::WideString::npos, empty.findCase(L"fox")) << "String has unexpected value";
}

TEST(WideStringTest, strcmpEmpty)
{
    pplib::WideString s1(L"");
    pplib::WideString s2(L"");
    ASSERT_EQ(0, s1.strcmp(s2));
}

TEST(WideStringTest, strcmpLower)
{
    pplib::WideString s1(L"ABcdef");
    pplib::WideString s2(L"Defghi");
    ASSERT_LT(s1.strcmp(s2), 0);
    ASSERT_LT(s1.strcmp(s2, 3), 0);
}

TEST(WideStringTest, strcmpLowerWithCase)
{
    pplib::WideString s1(L"ABcdef");
    pplib::WideString s2(L"abcdef");
    ASSERT_LT(s1.strcmp(s2), 0);
    ASSERT_LT(s1.strcmp(s2, 6), 0);
}

TEST(WideStringTest, strcmpHigher)
{
    pplib::WideString s1(L"ABcdef");
    pplib::WideString s2(L"Defghi");
    ASSERT_GT(s2.strcmp(s1), 0);
}

TEST(WideStringTest, strcmpHigherWithCase)
{
    pplib::WideString s1(L"ABcdef");
    pplib::WideString s2(L"abcdef");
    ASSERT_GT(s2.strcmp(s1), 0);
}

TEST(WideStringTest, strcmpEqual)
{
    pplib::WideString s1(L"ABcdef");
    pplib::WideString s2(L"ABcdef");
    ASSERT_EQ(s2.strcmp(s1), 0);
}

TEST(WideStringTest, strcasecmpEmpty)
{
    pplib::WideString s1(L"");
    pplib::WideString s2(L"");
    ASSERT_EQ(0, s1.strCaseCmp(s2));
}

TEST(WideStringTest, strcasecmpLower)
{
    pplib::WideString s1(L"ABcdef");
    pplib::WideString s2(L"Defghi");
    ASSERT_LT(s1.strCaseCmp(s2), 0);
    ASSERT_LT(s1.strCaseCmp(s2, 6), 0);
}

TEST(WideStringTest, strcasecmpLowerWithCase)
{
    pplib::WideString s1(L"ABcdef");
    pplib::WideString s2(L"abcdef");
    ASSERT_EQ(s1.strCaseCmp(s2), 0);
}

TEST(WideStringTest, strcasecmpHigher)
{
    pplib::WideString s1(L"ABcdef");
    pplib::WideString s2(L"Defghi");
    ASSERT_GT(s2.strCaseCmp(s1), 0);
}

TEST(WideStringTest, strcasecmpHigherWithCase)
{
    pplib::WideString s1(L"ABcdef");
    pplib::WideString s2(L"abcdef");
    ASSERT_EQ(s2.strCaseCmp(s1), 0);
}

TEST(WideStringTest, strcasecmpEqual)
{
    pplib::WideString s1(L"ABcdef");
    pplib::WideString s2(L"ABcdef");
    ASSERT_EQ(s2.strCaseCmp(s1), 0);
}

TEST(WideStringTest, left)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString s2 = s1.left(10);
    ASSERT_EQ(pplib::WideString(L"The quick "), s2);
    s2 = s1.left(100);
    ASSERT_EQ(pplib::WideString(L"The quick brown fox jumps over the lazy dog"), s2);
}

TEST(WideStringTest, right)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString s2 = s1.right(9);
    ASSERT_EQ(pplib::WideString(L" lazy dog"), s2);
    s2 = s1.right(100);
    ASSERT_EQ(pplib::WideString(L"The quick brown fox jumps over the lazy dog"), s2);
}

TEST(WideStringTest, midWithLength)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString s2 = s1.mid(10, 10);
    ASSERT_EQ(pplib::WideString(L"brown fox "), s2);

    s2 = s1.mid(10, 0);
    ASSERT_EQ(pplib::WideString(L""), s2);
}

TEST(WideStringTest, midWithoutLength)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString s2 = s1.mid(10);
    ASSERT_EQ(pplib::WideString(L"brown fox jumps over the lazy dog"), s2);
}

TEST(WideStringTest, midOnEmptyString)
{
    pplib::WideString s1;
    pplib::WideString s2 = s1.mid(10, 10);
    ASSERT_EQ(pplib::WideString(L""), s2);

    s2 = s1.mid(10);
    ASSERT_EQ(pplib::WideString(L""), s2);
}

TEST(WideStringTest, substrWithLength)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString s2 = s1.substr(10, 10);
    ASSERT_EQ(pplib::WideString(L"brown fox "), s2);
}

TEST(WideStringTest, substrWithoutLength)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString s2 = s1.substr(10);
    ASSERT_EQ(pplib::WideString(L"brown fox jumps over the lazy dog"), s2);
}

TEST(WideStringTest, lowerCase)
{
    pplib::WideString s1(L"The Quick Brown Fox Jumps over ÄÖÜ");
    pplib::WideString expected(L"the quick brown fox jumps over äöü");
    ASSERT_NO_THROW(s1.lowerCase());
    ASSERT_EQ(expected, s1);
    ASSERT_EQ(pplib::WideString(L""), pplib::WideString(L"").lowerCase());
}

TEST(WideStringTest, upperCase)
{
    pplib::WideString s1(L"The Quick Brown Fox Jumps over äöü");
    pplib::WideString expected(L"THE QUICK BROWN FOX JUMPS OVER ÄÖÜ");
    ASSERT_NO_THROW(s1.upperCase());
    ASSERT_EQ(expected, s1);
    ASSERT_EQ(pplib::WideString(L""), pplib::WideString(L"").upperCase());
}

TEST(WideStringTest, toLowerCase)
{
    ASSERT_EQ(pplib::WideString(L"the quick brown fox jumps over äöü"),
              pplib::WideString(L"The Quick Brown Fox Jumps over ÄÖÜ").toLowerCase());
}

TEST(WideStringTest, toUpperCase)
{
    ASSERT_EQ(pplib::WideString(L"THE QUICK BROWN FOX JUMPS OVER ÄÖÜ"),
              pplib::WideString(L"The Quick Brown Fox Jumps over äöü").toUpperCase());
}

TEST(WideStringTest, upperCaseWords)
{
    pplib::WideString expected(L"The Quick Brown Fox Jumps Over Äöü");
    pplib::WideString s1(L"the quick brown fox jumps over äöü");
    ASSERT_NO_THROW(s1.upperCaseWords());
    ASSERT_EQ(expected, s1);
    ASSERT_EQ(pplib::WideString(L""), pplib::WideString(L"").upperCaseWords());
}

TEST(WideStringTest, trimLeft)
{
    pplib::WideString s1(L"\n\n    abc  \n");
    s1.trimLeft();
    ASSERT_EQ((size_t)6, s1.size());
    ASSERT_EQ(pplib::WideString(L"abc  \n"), s1);

    s1.set(L"  \r\n\tHello World\r\n\t   ");
    s1.trimLeft();
    ASSERT_EQ((size_t)17, s1.size());
    ASSERT_EQ(pplib::WideString(L"Hello World\r\n\t   "), s1);

    s1.set(L"");
    s1.trimLeft();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::WideString(L""), s1);

    s1.set(L"Nichts zu trimmen");
    s1.trimLeft();
    ASSERT_EQ((size_t)17, s1.size());
    ASSERT_EQ(pplib::WideString(L"Nichts zu trimmen"), s1);
}

TEST(WideStringTest, trimRight)
{
    pplib::WideString s1(L" \n  abc  \n");
    s1.trimRight();
    ASSERT_EQ((size_t)7, s1.size());
    ASSERT_EQ(pplib::WideString(L" \n  abc"), s1);

    s1.set(L"  \r\n\tHello World\r\n\t   ");
    s1.trimRight();
    ASSERT_EQ((size_t)16, s1.size());
    ASSERT_EQ(pplib::WideString(L"  \r\n\tHello World"), s1);

    s1.set(L"");
    s1.trimRight();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::WideString(L""), s1);

    s1.set(L"Nichts zu trimmen");
    s1.trimRight();
    ASSERT_EQ((size_t)17, s1.size());
    ASSERT_EQ(pplib::WideString(L"Nichts zu trimmen"), s1);
}

TEST(WideStringTest, trim)
{
    pplib::WideString s1(L"\n\n    abc  \n");
    s1.trim();
    ASSERT_EQ((size_t)3, s1.size());
    ASSERT_EQ(pplib::WideString(L"abc"), s1);

    s1.set(L"  \r\n\tHello World\r\n\t   ");
    s1.trim();
    ASSERT_EQ((size_t)11, s1.size());
    ASSERT_EQ(pplib::WideString(L"Hello World"), s1);

    s1.set(L"Nichts zu trimmen");
    s1.trim();
    ASSERT_EQ((size_t)17, s1.size());
    ASSERT_EQ(pplib::WideString(L"Nichts zu trimmen"), s1);
}

TEST(WideStringTest, trim_empty)
{
    pplib::WideString s1;
    s1.trim();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::WideString(), s1);
}

TEST(WideStringTest, trimmed)
{
    pplib::WideString s1(L"\n\n    abc  \n");
    pplib::WideString s2 = s1.trimmed();
    ASSERT_EQ((size_t)3, s2.size());
    ASSERT_EQ(pplib::WideString(L"abc"), s2);

    ASSERT_EQ(pplib::WideString(L"Hello World"), pplib::WideString(L"Hello World").trimmed());
    ASSERT_EQ(pplib::WideString(L"Hello World"), pplib::WideString(L"Hello World   ").trimmed());
    ASSERT_EQ(pplib::WideString(L"Hello World"), pplib::WideString(L"   Hello World").trimmed());
    ASSERT_EQ(pplib::WideString(L""), pplib::WideString(L"").trimmed());
    ASSERT_EQ(pplib::WideString(L""), pplib::WideString(L"   \n\t   \n").trimmed());
}

TEST(WideStringTest, trimLeftEmptyResult)
{
    pplib::WideString s1(L"\n\n   \n   \n");
    s1.trimLeft();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::WideString(L""), s1);
}

TEST(WideStringTest, trimRightEmptyResult)
{
    pplib::WideString s1(L"\n\n   \n   \n");
    s1.trimRight();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::WideString(L""), s1);
}

TEST(WideStringTest, trimEmptyResult)
{
    pplib::WideString s1(L"\n\n   \n   \n");
    s1.trim();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::WideString(L""), s1);
}

TEST(WideStringTest, trimLeftChars)
{
    pplib::WideString s1(L"\n\n    abc  \n");
    s1.trimLeft(L" \n");
    ASSERT_EQ((size_t)6, s1.size());
    ASSERT_EQ(pplib::WideString("abc  \n"), s1);
}

TEST(WideStringTest, trimRightChars)
{
    pplib::WideString s1(L" \n  abc  \n");
    s1.trimRight(L" \n");
    ASSERT_EQ((size_t)7, s1.size());
    ASSERT_EQ(pplib::WideString(L" \n  abc"), s1);
}

TEST(WideStringTest, trimChars)
{
    pplib::WideString s1(L"\n\n    abc  \n");
    s1.trim(L" \n");
    ASSERT_EQ((size_t)3, s1.size());
    ASSERT_EQ(pplib::WideString(L"abc"), s1);
}

TEST(WideStringTest, trimLeftCharsEmptyResult)
{
    pplib::WideString s1(L"\n\n   \n   \n");
    s1.trimLeft(L" \n");
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::WideString(L""), s1);
}

TEST(WideStringTest, trimRightCharsEmptyResult)
{
    pplib::WideString s1(L"\n\n   \n   \n");
    s1.trimRight(L" \n");
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::WideString(L""), s1);
}

TEST(WideStringTest, trimEmptyCharsResult)
{
    pplib::WideString s1(L"\n\n   \n   \n");
    s1.trim(L" \n");
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::WideString(L""), s1);
}

TEST(WideStringTest, trimCharsEmptry)
{
    pplib::WideString s1;
    s1.trim(L" \n");
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::WideString(L""), s1);

    s1.set(L"Hello World");
    s1.trim(L"");
    ASSERT_EQ((size_t)11, s1.size());
    ASSERT_EQ(pplib::WideString(L"Hello World"), s1);
}

TEST(WideStringTest, chopRight)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    pplib::WideString s2(L"A test string with unicode characters: ä");
    pplib::WideString s3(L"A test string with unicode characters:");
    s1.chopRight(2);
    ASSERT_EQ((size_t)40, s1.len()) << "String does not have expected length";
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    s1.chopRight();
    s1.chopRight();
    ASSERT_EQ((size_t)38, s1.len()) << "String does not have expected length";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
    s1.chopRight(37);
    ASSERT_EQ((size_t)1, s1.len()) << "String does not have length of 1";
    s1.chopRight(1);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    s2.clear();
    ASSERT_EQ(s2, s1) << "String has unexpected value";

    s1.set("");
    s1.chopRight(10);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have expected length";

    s1.set(L"Hello World");
    s1.chopRight(20);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have expected length";
}

TEST(WideStringTest, chopLeft)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    pplib::WideString s2(L"est string with unicode characters: äöü");
    pplib::WideString s3(L"st string with unicode characters: äöü");
    s1.chopLeft(3);
    ASSERT_EQ((size_t)39, s1.len()) << "String does not have expected length";
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    s1.chopLeft();
    ASSERT_EQ((size_t)38, s1.len()) << "String does not have expected length";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
    s1.chopRight(37);
    ASSERT_EQ((size_t)1, s1.len()) << "String does not have length of 1";
    s1.chopRight(1);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    s2.clear();
    ASSERT_EQ(s2, s1) << "String has unexpected value";

    s1.set(L"");
    s1.chopLeft(10);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have expected length";

    s1.set(L"Hello World");
    s1.chopLeft(20);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have expected length";
}

TEST(WideStringTest, chop)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    pplib::WideString s2(L"A test string with unicode characters: ä");
    pplib::WideString s3(L"A test string with unicode characters:");
    s1.chop(2);
    ASSERT_EQ((size_t)40, s1.len()) << "String does not have length of 40";
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    s1.chop();
    s1.chop();
    ASSERT_EQ((size_t)38, s1.len()) << "String does not have length of 38";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
    s1.chop(37);
    ASSERT_EQ((size_t)1, s1.len()) << "String does not have length of 1";
    s1.chop();
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    s2.clear();
    ASSERT_EQ(s2, s1) << "String has unexpected value";

    s1.set(L"");
    s1.chop(10);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have expected length";
    s1.set(L"Hello World");
    s1.chop(20);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have expected length";
}

TEST(WideStringTest, chomp)
{
    pplib::WideString s1(L"\n\r\nA test string\näöü\n\r\n");
    pplib::WideString s2(L"A test string\näöü");
    s1.chomp();
    ASSERT_EQ((size_t)17, s1.len()) << "String does not have length of 17";
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    s1.chomp();
    ASSERT_EQ((size_t)17, s1.len()) << "String does not have length of 17";
    ASSERT_EQ(s2, s1) << "String has unexpected value";
}

TEST(WideStringTest, cut_WithPos)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString expected(L"The quick ");
    s1.cut(10);
    ASSERT_EQ((size_t)10, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST(WideStringTest, cut_WithPos0)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString expected(L"");
    s1.cut(0);
    ASSERT_EQ((size_t)0, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST(WideStringTest, cut_WithPosBeyondStringLength)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString expected(L"The quick brown fox jumps over the lazy dog");
    s1.cut(100);
    ASSERT_EQ((size_t)43, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST(WideStringTest, cut_WithLetter)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString expected(L"The quick ");
    s1.cut(L"b");
    ASSERT_EQ((size_t)10, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST(WideStringTest, cut_WithNonexistingLetter)
{
    pplib::WideString s1(L"The quick brown fox");
    pplib::WideString expected(L"The quick brown fox");
    s1.cut(L"j");
    ASSERT_EQ((size_t)19, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST(WideStringTest, cut_WithString)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString expected(L"The quick ");
    s1.cut(L"brown");
    ASSERT_EQ((size_t)10, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST(WideStringTest, cut_onEmptyString)
{
    pplib::WideString s1(L"");
    pplib::WideString expected(L"");
    s1.cut(10);
    ASSERT_EQ((size_t)0, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";

    s1.cut(L"a");
    ASSERT_EQ((size_t)0, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";

    s1.set(L"Hello World");
    s1.cut(L"");
    ASSERT_EQ((size_t)11, s1.len()) << "String has unexpected length";
    ASSERT_EQ(pplib::String("Hello World"), s1) << "String has unexpected value";
}

TEST(WideStringTest, strstr)
{
    pplib::WideString s1(L"A test haystack string");
    pplib::WideString s3(L"haystack string");
    pplib::WideString s2;
    s2 = s1.strstr(L"haystack");
    ASSERT_EQ((size_t)15, s2.len()) << "String does not have length of 15";
    ASSERT_EQ(s3, s2) << "String has unexpected value";
    s2 = s1.strstr(L"beer");
    s3 = L"";
    ASSERT_EQ((size_t)0, s2.len()) << "String does not have length of 0";
    ASSERT_EQ(s3, s2) << "String has unexpected value";

    s2 = s1.strstr(L"");
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    s1.set(L"");
    s2 = s1.strstr(L"haystack");
    ASSERT_EQ((size_t)0, s2.len()) << "String does not have length of 0";
}

TEST(WideStringTest, strchr_ExistingChar)
{
    pplib::WideString s1(L"The Quick Brown Fox Jumps over äöü");
    pplib::WideString expected(L"Fox Jumps over äöü");
    ASSERT_EQ(expected, s1.strchr(L'F')) << "Unexpected Result";

    s1.set(L"");
    ASSERT_EQ(pplib::WideString(L""), s1.strchr(L'F')) << "Unexpected Result";
}

TEST(WideStringTest, strchr_NonExistingChar)
{
    pplib::WideString s1(L"The Quick Brown Fox Jumps over äöü");
    pplib::WideString expected(L"");
    ASSERT_EQ(expected, s1.strchr(L'L')) << "Unexpected Result";
}

TEST(WideStringTest, strrchr_ExistingChar)
{
    pplib::WideString s1(L"The Quick Brown Fox Jumps over äöü");
    pplib::WideString expected(L"over äöü");
    ASSERT_EQ(expected, s1.strrchr(L'o')) << "Unexpected Result";

    s1.set(L"");
    ASSERT_EQ(pplib::WideString(L""), s1.strrchr(L'F')) << "Unexpected Result";
}

TEST(WideStringTest, strrchr_NonExistingChar)
{
    pplib::WideString s1(L"The Quick Brown Fox Jumps over äöü");
    pplib::WideString expected(L"");
    ASSERT_EQ(expected, s1.strrchr(L'L')) << "Unexpected Result";
}

TEST(WideStringTest, repeated)
{
    pplib::WideString s1(L"_repeat_");
    pplib::WideString s3(L"_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
    pplib::WideString s2;
    s2 = s1.repeated(10);
    ASSERT_EQ((size_t)80, s2.len()) << "String does not have length of 80";
    ASSERT_EQ(s3, s2) << "String has unexpected value";
}

TEST(WideStringTest, repeat_by_count)
{
    pplib::WideString s1(L"_repeat_");
    s1.repeat(10);
    pplib::WideString s3(L"_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
    ASSERT_EQ((size_t)80, s1.len()) << "String does not have length of 80";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
}

TEST(WideStringTest, repeat_empty)
{
    pplib::WideString s1(L"");
    s1.repeat(10);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    ASSERT_EQ(pplib::WideString(L""), s1) << "String has unexpected value";

    s1.set(L"Hello World");
    s1.repeat(0);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    ASSERT_EQ(pplib::WideString(L""), s1) << "String has unexpected value";
}

TEST(WideStringTest, repeat_with_string)
{
    pplib::WideString s1(L"blah");
    s1.repeat(pplib::WideString(L"_repeat_"), 10);
    pplib::WideString s3(L"_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
    ASSERT_EQ((size_t)80, s1.len()) << "String does not have length of 80";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
}

TEST(WideStringTest, StringTest_repeat_empty)
{
    pplib::WideString s1(L"blah");
    s1.repeat(pplib::WideString(L""), 10);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    ASSERT_EQ(pplib::WideString(L""), s1) << "String has unexpected value";

    s1.set(L"Hello World");
    s1.repeat(pplib::WideString(L"_repeat_"), 0);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    ASSERT_EQ(pplib::WideString(L""), s1) << "String has unexpected value";
}

TEST(WideStringTest, repeat_with_string_same_String)
{
    pplib::WideString s1(L"_repeat_");
    s1.repeat(s1, 10);
    pplib::WideString s3(L"_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
    ASSERT_EQ((size_t)80, s1.len()) << "String does not have length of 80";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
}

TEST(WideStringTest, repeat_with_code)
{
    pplib::WideString s1(L"_repeat_");
    s1.repeat(L'_', 10);
    pplib::WideString s3(L"__________");
    ASSERT_EQ((size_t)10, s1.len()) << "String does not have length of 10";
    ASSERT_EQ(s3, s1) << "String has unexpected value";

    s1.repeat(0, 10);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have expected length";

    s1.repeat(L'_', 0);
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have expected length";
}

TEST(WideStringTest, replace)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString s2 = s1.replace(L"fox", L"cat");
    ASSERT_EQ(pplib::WideString(L"The quick brown cat jumps over the lazy dog"), s2) << "String has unexpected value";
    pplib::WideString s3 = s1.replace(L"the", L"a");
    ASSERT_EQ(pplib::WideString(L"The quick brown cat jumps over a lazy dog"), s3) << "String has unexpected value";

    s1.set(L"");
    pplib::WideString s4 = s1.replace(L"fox", L"cat");
    ASSERT_EQ(pplib::WideString(L""), s4) << "String has unexpected value";

    s1.set(L"The quick brown fox jumps over the lazy dog");
    s1.replace(L"", L"Hello World");
    ASSERT_EQ(pplib::WideString(L"The quick brown fox jumps over the lazy dog"), s1) << "String has unexpected value";
}

TEST(WideStringTest, shl)
{
    pplib::WideString s1(L"Hello World!");
    s1.shl(0, 5);
    ASSERT_EQ(pplib::WideString(L" World!"), s1) << "String has unexpected value";
    s1.shl(L' ', 2);
    ASSERT_EQ(pplib::WideString(L"orld!  "), s1) << "String has unexpected value";

    s1.shl(L' ', 0);
    ASSERT_EQ(pplib::WideString(L"orld!  "), s1) << "String has unexpected value";
    s1.shl(L' ', 100);
    ASSERT_EQ(pplib::WideString(L"       "), s1) << "String has unexpected value";

    s1.set(L"");
    s1.shl(L' ', 5);
    ASSERT_EQ(pplib::WideString(L""), s1) << "String has unexpected value";
}

TEST(WideStringTest, shr)
{
    pplib::WideString s1(L"Hello World!");
    s1.shr(0, 5);
    ASSERT_EQ(pplib::WideString(L"Hello W"), s1) << "String has unexpected value";
    s1.set(L"Hello World!");
    s1.shr(L' ', 2);
    ASSERT_EQ(pplib::WideString(L"  Hello Worl"), s1) << "String has unexpected value";
    s1.shr(L' ', 0);
    ASSERT_EQ(pplib::WideString(L"  Hello Worl"), s1) << "String has unexpected value";

    s1.set(L"");
    s1.shr(L' ', 5);
    ASSERT_EQ(pplib::WideString(L""), s1) << "String has unexpected value";

    s1.set(L"Hello World!");
    s1.shr(L' ', 100);
    ASSERT_EQ(pplib::WideString(L"            "), s1) << "String has unexpected value";
}

TEST(WideStringTest, compareOperatorsWithStringObject)
{
    pplib::WideString s1(L"aaa");
    pplib::WideString s2(L"bbb");
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
    ASSERT_FALSE(s1 == s2) << "String has unexpected value";
    ASSERT_TRUE(s1 != s2) << "String has unexpected value";
    ASSERT_FALSE(s1 != s1) << "String has unexpected value";
}

TEST(WideStringTest, compareOperatorsWithConstCharPtr)
{
    pplib::WideString s1(L"bbb");
    ASSERT_TRUE(s1 < L"ccc") << "String has unexpected value";
    ASSERT_FALSE(s1 < L"aaa") << "String has unexpected value";
    ASSERT_TRUE(s1 <= L"ccc") << "String has unexpected value";
    ASSERT_FALSE(s1 <= L"aaa") << "String has unexpected value";
    ASSERT_TRUE(s1 > L"aaa") << "String has unexpected value";
    ASSERT_TRUE(s1 >= L"aaa") << "String has unexpected value";
    ASSERT_FALSE(s1 > L"ccc") << "String has unexpected value";
    ASSERT_FALSE(s1 >= L"ccc") << "String has unexpected value";
    ASSERT_TRUE(s1 >= L"aaa") << "String has unexpected value";
    ASSERT_TRUE(s1 == L"bbb") << "String has unexpected value";
    ASSERT_FALSE(s1 == L"aaa") << "String has unexpected value";

    ASSERT_TRUE(s1 != L"aaa") << "String has unexpected value";
    ASSERT_FALSE(s1 != L"bbb") << "String has unexpected value";
}

TEST(WideStringTest, c_str)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    const wchar_t* cstr = s1.c_str();
    ASSERT_EQ(pplib::WideString(L"The quick brown fox jumps over the lazy dog"), pplib::WideString(cstr)) << "String has unexpected value";

    s1.set(L"");
    cstr = s1.c_str();
    ASSERT_EQ(pplib::WideString(L""), pplib::WideString(cstr)) << "String has unexpected value";
}

TEST(WideStringTest, toWchart)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    const wchar_t* wstr = s1.toWchart();
    ASSERT_EQ(pplib::WideString(L"The quick brown fox jumps over the lazy dog"), pplib::WideString(wstr)) << "String has unexpected value";

    s1.set(L"");
    wstr = s1.toWchart();
    ASSERT_EQ(pplib::WideString(L""), pplib::WideString(wstr)) << "String has unexpected value";
}

TEST(WideStringTest, operator_wchart)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    const wchar_t* wstr = s1;
    ASSERT_EQ(pplib::WideString(L"The quick brown fox jumps over the lazy dog"), pplib::WideString(wstr)) << "String has unexpected value";

    s1.set(L"");
    wstr = s1;
    ASSERT_EQ(pplib::WideString(L""), pplib::WideString(wstr)) << "String has unexpected value";
}

TEST(WideStringTest, operator_bool)
{
    ASSERT_TRUE((bool)pplib::WideString(L"true"));
    ASSERT_FALSE((bool)pplib::WideString(L"false"));
}

TEST(WideStringTest, operator_int)
{
    ASSERT_EQ((int)pplib::WideString(L"1234"), 1234);
    ASSERT_EQ((int)pplib::WideString(L"-1234"), -1234);
    ASSERT_EQ((int)pplib::WideString(L"abc123"), 0);
    ASSERT_EQ((int)pplib::WideString(L""), 0);
}

TEST(WideStringTest, operator_unsigned_int)
{
    ASSERT_EQ((unsigned int)pplib::WideString(L"1234"), 1234u);
    ASSERT_EQ((unsigned int)pplib::WideString(L"-1234"), (unsigned int)-1234);
    ASSERT_EQ((unsigned int)pplib::WideString(L"abc123"), 0u);
    ASSERT_EQ((unsigned int)pplib::WideString(L""), 0u);
}

TEST(WideStringTest, operator_int64)
{
    ASSERT_EQ((int64_t)pplib::WideString(L"1234567890123456789"), (int64_t)1234567890123456789LL);
    ASSERT_EQ((int64_t)pplib::WideString(L"-1234567890123456789"), (int64_t)-1234567890123456789LL);
    ASSERT_EQ((int64_t)pplib::WideString(L"abc123"), (int64_t)0LL);
    ASSERT_EQ((int64_t)pplib::WideString(L""), (int64_t)0LL);
}

TEST(WideStringTest, operator_float)
{
    ASSERT_FLOAT_EQ((float)pplib::WideString(L"123.456"), 123.456f);
    ASSERT_FLOAT_EQ((float)pplib::WideString(L"-123.456"), -123.456f);
    ASSERT_FLOAT_EQ((float)pplib::WideString(L"abc123"), 0.0f);
    ASSERT_FLOAT_EQ((float)pplib::WideString(L""), 0.0f);
}

TEST(WideStringTest, operator_double)
{
    ASSERT_DOUBLE_EQ((double)pplib::WideString(L"123.456"), 123.456);
    ASSERT_DOUBLE_EQ((double)pplib::WideString(L"-123.456"), -123.456);
    ASSERT_DOUBLE_EQ((double)pplib::WideString(L"abc123"), 0.0);
    ASSERT_DOUBLE_EQ((double)pplib::WideString(L""), 0.0);
}

TEST(WideStringTest, operator_std_string)
{
    ASSERT_EQ((std::string)pplib::WideString(L"Hello World!"), std::string("Hello World!"));
    ASSERT_EQ((std::string)pplib::WideString(L""), std::string(""));
}

TEST(WideStringTest, operator_std_wstring)
{
    ASSERT_EQ((std::wstring)pplib::WideString(L"Hello World!"), std::wstring(L"Hello World!"));
    ASSERT_EQ((std::wstring)pplib::WideString(L""), std::wstring(L""));
}

TEST(WideStringTest, toInt)
{
    EXPECT_EQ((int)1234, pplib::WideString(L"1234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)-1234, pplib::WideString(L"-1234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)1234, pplib::WideString(L"01234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)-1234, pplib::WideString(L"-01234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)0, pplib::WideString(L"abc123").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)0, pplib::WideString(L"0x1234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)0, pplib::WideString(L"").toInt()) << "Unexpected Result";
}

TEST(WideStringTest, toUnsignedInt)
{
    EXPECT_EQ((unsigned int)1234, pplib::WideString(L"1234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)-1234, pplib::WideString(L"-1234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)1234, pplib::WideString(L"01234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)-1234, pplib::WideString(L"-01234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)0, pplib::WideString(L"abc123").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)0, pplib::WideString(L"0x1234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)0, pplib::WideString(L"").toUnsignedInt()) << "Unexpected Result";
}

TEST(WideStringTest, toInt64)
{
    EXPECT_EQ((int64_t)1234, pplib::WideString(L"1234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)-1234, pplib::WideString(L"-1234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)1234, pplib::WideString(L"01234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)-1234, pplib::WideString(L"-01234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)0, pplib::WideString(L"abc123").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)0, pplib::WideString(L"0x1234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)0, pplib::WideString(L"").toInt64()) << "Unexpected Result";
}

TEST(WideStringTest, toUnsignedInt64)
{
    EXPECT_EQ((uint64_t)1234, pplib::WideString(L"1234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)-1234, pplib::WideString(L"-1234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)1234, pplib::WideString(L"01234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)-1234, pplib::WideString(L"-01234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)0, pplib::WideString(L"abc123").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)0, pplib::WideString(L"0x1234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)0, pplib::WideString(L"").toUnsignedInt64()) << "Unexpected Result";
}

TEST(WideStringTest, toInt_withoutNumber)
{
    pplib::WideString s1(L"The Quick Brown Fox Jumps over the lazy dog");
    EXPECT_EQ((int)0, s1.toInt()) << "Unexpected Result";
}

TEST(WideStringTest, toInt_123456)
{
    pplib::WideString s1(L"123456");
    EXPECT_EQ((int)123456, s1.toInt()) << "Unexpected Result";
}

TEST(WideStringTest, ToInt_minus123456)
{
    pplib::WideString s1(L"-123456");
    EXPECT_EQ((int)-123456, s1.toInt()) << "Unexpected Result";
}

TEST(WideStringTest, toInt_123456_point_567)
{
    pplib::WideString s1(L"123456.567");
    EXPECT_EQ((int)123456, s1.toInt()) << "Unexpected Result";
}

TEST(WideStringTest, toInt64_1242346214893456)
{
    pplib::WideString s1(L"1242346214893456");
    EXPECT_EQ((int64_t)1242346214893456, s1.toInt64()) << "Unexpected Result";
}

TEST(WideStringTest, toInt64_minus1242346214893456)
{
    pplib::WideString s1(L"-1242346214893456");
    EXPECT_EQ((int64_t)-1242346214893456, s1.toInt64()) << "Unexpected Result";
}

TEST(WideStringTest, toBool)
{
    pplib::WideString s1(L"A test string with unicode characters: äöü");
    ASSERT_EQ(s1.toBool(), false) << "String should not be true";
    s1.set(L"");
    ASSERT_EQ(s1.toBool(), false) << "String should not be true";
    s1.set(L"12345abcd");
    ASSERT_EQ(s1.toBool(), true) << "String should be true";
    s1.set(L"1");
    ASSERT_EQ(s1.toBool(), true) << "String should be true";
    s1.set(L"12345");
    ASSERT_EQ(s1.toBool(), true) << "String should be true";
    s1.set(L"true");
    ASSERT_EQ(s1.toBool(), true) << "String should be true";
    s1.set(L"wahr");
    ASSERT_EQ(s1.toBool(), true) << "String should be true";
    s1.set(L"ja");
    ASSERT_EQ(s1.toBool(), true) << "String should be true";
    s1.set(L"yes");
    ASSERT_EQ(s1.toBool(), true) << "String should be true";
    s1.set(L"false");
    ASSERT_EQ(s1.toBool(), false) << "String should not be true";
    s1.set(L"falsch");
    ASSERT_EQ(s1.toBool(), false) << "String should not be true";
    s1.set(L"nein");
    ASSERT_EQ(s1.toBool(), false) << "String should not be true";
    s1.set(L"no");
    ASSERT_EQ(s1.toBool(), false) << "String should not be true";
}

TEST(WideStringTest, ToLong_1124234674)
{
    pplib::WideString s1(L"124234674");
    EXPECT_EQ((long)124234674, s1.toLong()) << "Unexpected Result";
}

TEST(WideStringTest, toLongLong_1242346214893456)
{
    pplib::WideString s1(L"1242346214893456");
    EXPECT_EQ((long long)1242346214893456, s1.toLongLong()) << "Unexpected Result";
}

TEST(WideStringTest, ToFloat_182566142_346214893456)
{
    setlocale(LC_NUMERIC, "C");
    pplib::WideString s1(L"182566142.346214893456");
    EXPECT_EQ((float)182566142.346214893456, s1.toFloat()) << "Unexpected Result";
}

TEST(WideStringTest, ToDouble_182566142_346214893456)
{
    setlocale(LC_NUMERIC, "C");
    pplib::WideString s1(L"182566142.346214893456");
    EXPECT_EQ((double)182566142.346214893456, s1.toDouble()) << "Unexpected Result";
}

TEST(WideStringTest, join)
{
    pplib::Array a;
    a.add("One");
    a.add("Two");
    a.add("Three");
    pplib::WideString s1(L",");
    EXPECT_EQ(pplib::WideString(L"One,Two,Three"), s1.join(a));
}

TEST(WideStringTest, has)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    ASSERT_TRUE(s1.has(L"fox")) << "String has unexpected value";
    ASSERT_TRUE(s1.has(L"The")) << "String has unexpected value";
    ASSERT_TRUE(s1.contains(L"FOX", true)) << "String has unexpected value";
    ASSERT_FALSE(s1.contains(L"cat")) << "String has unexpected value";
    ASSERT_FALSE(s1.contains(L"")) << "String has unexpected value";

    ASSERT_FALSE(pplib::String().has("fox")) << "String has unexpected value";
}

TEST(WideStringTest, startsWith)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    ASSERT_TRUE(s1.startsWith(L"The quick")) << "String has unexpected value";
    ASSERT_TRUE(s1.startsWith(L"quick", 4)) << "String has unexpected value";
    ASSERT_TRUE(s1.startsWith(L"quick", 4, 20)) << "String has unexpected value";
    ASSERT_TRUE(s1.startsWith(L"The", 0, 20)) << "String has unexpected value";
    ASSERT_FALSE(s1.startsWith(L"the quick")) << "String has unexpected value";
    ASSERT_FALSE(s1.startsWith(L"quick")) << "String has unexpected value";
}

TEST(WideStringTest, endsWith)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    ASSERT_TRUE(s1.endsWith(L"lazy dog")) << "String has unexpected value";
    ASSERT_TRUE(s1.endsWith(L"dog", 40)) << "String has unexpected value";
    ASSERT_TRUE(s1.endsWith(L"dog", 40, 4)) << "String has unexpected value";
    ASSERT_TRUE(s1.endsWith(L"dog", 0, 100)) << "String has unexpected value";
    ASSERT_FALSE(s1.endsWith(L"Lazy dog")) << "String has unexpected value";
    ASSERT_FALSE(s1.endsWith(L"lazy")) << "String has unexpected value";
}

TEST(WideStringTest, ostreamtest)
{
    pplib::WideString s1("Hello World!");
    testing::internal::CaptureStdout();
    std::cout << s1;
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ("Hello World!", output) << "Unexpected Result";
}

// Testing forward iterators const and non-const
TEST(WideStringTest, forwardIterator)
{
    pplib::WideString s1("Hello World!");
    std::wstring result;
    for (auto it = s1.begin(); it != s1.end(); ++it) {
        result += *it;
    }
    EXPECT_EQ(L"Hello World!", result) << "Unexpected Result";
}

TEST(WideStringTest, forwardIteratorConst)
{
    const pplib::WideString s1("Hello World!");
    std::wstring result;
    for (auto it = s1.cbegin(); it != s1.cend(); ++it) {
        result += *it;
    }
    EXPECT_EQ(L"Hello World!", result) << "Unexpected Result";
}

TEST(WideStringTest, forwardIteratorRangeBasedForLoop)
{
    pplib::WideString s1("Hello World!");
    std::wstring result;
    for (const auto& c : s1) {
        result += c;
    }
    EXPECT_EQ(L"Hello World!", result) << "Unexpected Result";
}

TEST(WideStringTest, backwardIterator)
{
    pplib::WideString s1("Hello World!");
    std::wstring result;
    for (auto it = s1.rbegin(); it != s1.rend(); ++it) {
        result += *it;
    }
    EXPECT_EQ(L"!dlroW olleH", result) << "Unexpected Result";
}

TEST(WideStringTest, backwardIteratorConst)
{
    const pplib::WideString s1("Hello World!");
    std::wstring result;
    for (auto it = s1.crbegin(); it != s1.crend(); ++it) {
        result += *it;
    }
    EXPECT_EQ(L"!dlroW olleH", result) << "Unexpected Result";
}

TEST(WideStringTest, backwardIteratorConstWith_rbegin_rend)
{
    const pplib::WideString s1("Hello World!");
    std::wstring result;
    for (auto it = s1.rbegin(); it != s1.rend(); ++it) {
        result += *it;
    }
    EXPECT_EQ(L"!dlroW olleH", result) << "Unexpected Result";
}

} // namespace
