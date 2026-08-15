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
    s1.set(L"123.451,12");
    ASSERT_EQ(s1.isNumeric(), false) << "String should not be numeric";
    s1.set(L"-123.451,12");
    ASSERT_EQ(s1.isNumeric(), false) << "String should not be numeric";
    s1.set(L"-123.451,12-9");
    ASSERT_EQ(s1.isNumeric(), false) << "String should not be numeric";
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
}

TEST(WideStringTest, setStringRefWithoutSize)
{
    pplib::WideString s2(L"äöü, a test string with unicode characters");
    pplib::WideString s1;
    s1.set(s2);
    ASSERT_EQ(s1, s2) << "String as unexpected value";
    ASSERT_EQ((size_t)42, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, setStringRefWithSize)
{
    pplib::WideString s2(L"äöü, a tes");
    pplib::WideString s3(L"äöü, a test string with unicode characters");
    pplib::WideString s1;
    s1.set(s3, 10);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)10, s1.size()) << "String has unexpected length";
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
}

TEST(WideStringTest, setf)
{
    pplib::WideString s2(L"Ein Test, 42, Wide, 10000");
    pplib::WideString s1;
    s1.setf("Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
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
    ASSERT_EQ((size_t)26, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, appendStringWithoutSize)
{
    pplib::WideString expected(L"First Part äöü, äöü Second Part");
    pplib::WideString s1(L"First Part äöü, ");
    pplib::WideString s2(L"äöü Second Part");
    s1.append(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, appendStringWithSize)
{
    pplib::WideString expected(L"First Part äöü, äöü Seco");
    pplib::WideString s1(L"First Part äöü, ");
    pplib::WideString s2(L"äöü Second Part");
    s1.append(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)24, s1.size()) << "String has unexpected length";
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

TEST(WideStringTest, append_same_string)
{
    pplib::WideString expected("It's me, It's me, It's me, It's me, ");
    pplib::WideString s1("It's me, ");
    s1.append(s1);
    s1.append(s1);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)36, s1.size()) << "String has unexpected length";
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

TEST(WideStringTest, prependStringWithoutSize)
{
    pplib::WideString expected(L"äöü Second PartFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    pplib::WideString s2(L"äöü Second Part");
    s1.prepend(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)31, s1.size()) << "String has unexpected length";
}

TEST(WideStringTest, prependStringWithSize)
{
    pplib::WideString expected(L"äöü SFirst Part äöü, ");
    pplib::WideString s1(L"First Part äöü, ");
    pplib::WideString s2(L"äöü Second Part");
    s1.prepend(s2, 5);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)21, s1.size()) << "String has unexpected length";
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

TEST(WideStringTest, prepend_same_string)
{
    pplib::WideString expected("It's me, It's me, It's me, It's me, ");
    pplib::WideString s1("It's me, ");
    s1.prepend(s1);
    s1.prepend(s1);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)36, s1.size()) << "String has unexpected length";
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

TEST(WideStringTest, strstr)
{
    pplib::WideString s1(L"A test haystack string");
    pplib::WideString s3(L"haystack string");
    pplib::WideString s2;
    s2 = s1.strstr(L"haystack");
    ASSERT_EQ((size_t)15, s2.len()) << "String does not have length of 15";
    ASSERT_EQ(s3, s2) << "String has unexpected value";
    s2 = s1.strstr(L"beer");
    s3 = "";
    ASSERT_EQ((size_t)0, s2.len()) << "String does not have length of 0";
    ASSERT_EQ(s3, s2) << "String has unexpected value";

    s2 = s1.strstr(L"");
    ASSERT_EQ(s2, s1) << "String has unexpected value";
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

TEST(WideStringTest, repeat_with_string)
{
    pplib::WideString s1(L"blah");
    s1.repeat(pplib::WideString(L"_repeat_"), 10);
    pplib::WideString s3(L"_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
    ASSERT_EQ((size_t)80, s1.len()) << "String does not have length of 80";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
}

TEST(WideStringTest, trimLeft)
{
    pplib::WideString s1(L"\n\n    abc  \n");
    s1.trimLeft();
    ASSERT_EQ((size_t)6, s1.size());
    ASSERT_EQ(pplib::WideString(L"abc  \n"), s1);
}

TEST(WideStringTest, trimRight)
{
    pplib::WideString s1(L" \n  abc  \n");
    s1.trimRight();
    ASSERT_EQ((size_t)7, s1.size());
    ASSERT_EQ(pplib::WideString(L" \n  abc"), s1);
}

TEST(WideStringTest, trim)
{
    pplib::WideString s1(L"\n\n    abc  \n");
    s1.trim();
    ASSERT_EQ((size_t)3, s1.size());
    ASSERT_EQ(pplib::WideString(L"abc"), s1);
}

TEST(WideStringTest, trimEmptyString)
{
    pplib::WideString s1(L"");
    s1.trim();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::WideString(L""), s1);
}

TEST(WideStringTest, trimmed)
{
    pplib::WideString s1(L"\n\n    abc  \n");
    pplib::WideString s2 = s1.trimmed();
    ASSERT_EQ((size_t)3, s2.size());
    ASSERT_EQ(pplib::WideString("abc"), s2);

    ASSERT_EQ(pplib::WideString("Hello World"), pplib::WideString("Hello World").trimmed());
    ASSERT_EQ(pplib::WideString("Hello World"), pplib::WideString("Hello World   ").trimmed());
    ASSERT_EQ(pplib::WideString("Hello World"), pplib::WideString("   Hello World").trimmed());
    ASSERT_EQ(pplib::WideString(""), pplib::WideString("").trimmed());
    ASSERT_EQ(pplib::WideString(""), pplib::WideString("   \n\t   \n").trimmed());
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
    ASSERT_EQ(pplib::WideString(L"abc  \n"), s1);
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

TEST(WideStringTest, operatorEqualEmpty)
{
    pplib::WideString s1(L"");
    pplib::WideString s2(L"");
    ASSERT_EQ(s1, s2);
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
}

TEST(WideStringTest, strcmpLowerWithCase)
{
    pplib::WideString s1(L"ABcdef");
    pplib::WideString s2(L"abcdef");
    ASSERT_LT(s1.strcmp(s2), 0);
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
}

TEST(WideStringTest, right)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString s2 = s1.right(9);
    ASSERT_EQ(pplib::WideString(L" lazy dog"), s2);
}

TEST(WideStringTest, midWithLength)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString s2 = s1.mid(10, 10);
    ASSERT_EQ(pplib::WideString(L"brown fox "), s2);
}

TEST(WideStringTest, midWithoutLength)
{
    pplib::WideString s1(L"The quick brown fox jumps over the lazy dog");
    pplib::WideString s2 = s1.mid(10);
    ASSERT_EQ(pplib::WideString(L"brown fox jumps over the lazy dog"), s2);
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
}

TEST(WideStringTest, upperCase)
{
    pplib::WideString s1(L"The Quick Brown Fox Jumps over äöü");
    pplib::WideString expected(L"THE QUICK BROWN FOX JUMPS OVER ÄÖÜ");
    ASSERT_NO_THROW(s1.upperCase());
    ASSERT_EQ(expected, s1);
}

TEST(WideStringTest, upperCaseWords)
{
    pplib::WideString s1(L"the quick brown fox jumps over äöü");
    pplib::WideString expected(L"The Quick Brown Fox Jumps Over Äöü");
    ASSERT_NO_THROW(s1.upperCaseWords());
    ASSERT_EQ(expected, s1);
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

TEST(WideStringTest, Utf8toUtf8)
{
    ASSERT_NO_THROW({
        pplib::WideString s1(L"A test string with unicode characters: äöü");
        pplib::ByteArray a = s1.toUtf8();
        ASSERT_EQ((size_t)45, a.size()) << "String does not have expected length";
        ASSERT_EQ((unsigned char)'A', (unsigned char)a.get(0)) << "Unexpected Character in string";
        ASSERT_EQ((unsigned char)188, (unsigned char)a.get(44)) << "Unexpected Character in string";
    });
}

TEST(WideStringTest, ISO88591toUtf8)
{
    if (setlocale(LC_CTYPE, LATIN1_LOCALE) == NULL) {
        FAIL() << "setlocale fehlgeschlagen\n";
    }
    pplib::WideString s1;
    EXPECT_NO_THROW({
        s1.set(L"A test string with unicode characters: ");
        s1.append(0xe4);
        s1.append(0xf6);
        s1.append(0xfc);
    });
    EXPECT_EQ((size_t)42, s1.size()) << "String does not have expected length";
    EXPECT_EQ('A', (unsigned char)s1[0]) << "Unexpected Character in string";
    EXPECT_EQ(228, (unsigned char)s1[39]) << "Unexpected Character in string";
    pplib::ByteArray a;
    EXPECT_NO_THROW({ a = s1.toUtf8(); });
    // a.hexDump();
    EXPECT_EQ((size_t)45, a.size()) << "String does not have expected length";
    EXPECT_EQ((unsigned char)'A', (unsigned char)a.get(0)) << "Unexpected Character in string";
    EXPECT_EQ(188, (unsigned char)a.get(44)) << "Unexpected Character in string";
    if (setlocale(LC_ALL, DEFAULT_LOCALE) == NULL) {
        printf("setlocale fehlgeschlagen\n");
        throw std::exception();
    }
}

TEST(WideStringTest, strchr_ExistingChar)
{
    pplib::WideString s1(L"The Quick Brown Fox Jumps over äöü");
    pplib::WideString expected(L"Fox Jumps over äöü");
    ASSERT_EQ(expected, s1.strchr('F')) << "Unexpected Result";
}

TEST(WideStringTest, strchr_NonExistingChar)
{
    pplib::WideString s1(L"The Quick Brown Fox Jumps over äöü");
    pplib::WideString expected(L"");
    ASSERT_EQ(expected, s1.strchr('L')) << "Unexpected Result";
}

TEST(WideStringTest, strrchr_ExistingChar)
{
    pplib::WideString s1(L"The Quick Brown Fox Jumps over äöü");
    pplib::WideString expected(L"over äöü");
    ASSERT_EQ(expected, s1.strrchr('o')) << "Unexpected Result";
}

TEST(WideStringTest, strrchr_NonExistingChar)
{
    pplib::WideString s1(L"The Quick Brown Fox Jumps over äöü");
    pplib::WideString expected(L"");
    ASSERT_EQ(expected, s1.strrchr('L')) << "Unexpected Result";
}

TEST(WideStringTest, toInt)
{
    EXPECT_EQ((int)1234, pplib::WideString(L"1234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)-1234, pplib::WideString(L"-1234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)1234, pplib::WideString(L"01234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)-1234, pplib::WideString(L"-01234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)0, pplib::WideString(L"abc123").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)0, pplib::WideString(L"0x1234").toInt()) << "Unexpected Result";
}

TEST(WideStringTest, toUnsignedInt)
{
    EXPECT_EQ((unsigned int)1234, pplib::WideString(L"1234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)-1234, pplib::WideString(L"-1234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)1234, pplib::WideString(L"01234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)-1234, pplib::WideString(L"-01234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)0, pplib::WideString(L"abc123").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)0, pplib::WideString(L"0x1234").toUnsignedInt()) << "Unexpected Result";
}

TEST(WideStringTest, toInt64)
{
    EXPECT_EQ((int64_t)1234, pplib::WideString(L"1234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)-1234, pplib::WideString(L"-1234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)1234, pplib::WideString(L"01234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)-1234, pplib::WideString(L"-01234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)0, pplib::WideString(L"abc123").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)0, pplib::WideString(L"0x1234").toInt64()) << "Unexpected Result";
}

TEST(WideStringTest, toUnsignedInt64)
{
    EXPECT_EQ((uint64_t)1234, pplib::WideString(L"1234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)-1234, pplib::WideString(L"-1234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)1234, pplib::WideString(L"01234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)-1234, pplib::WideString(L"-01234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)0, pplib::WideString(L"abc123").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)0, pplib::WideString(L"0x1234").toUnsignedInt64()) << "Unexpected Result";
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

} // namespace
