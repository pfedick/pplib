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
#include <pplib/types/array.h>
#include <pplib/exceptions.h>

#include "pplib-tests.h"

namespace
{

// The fixture for testing class Foo.
class StringTest : public ::testing::Test
{
protected:
    StringTest()
    {
        if (setlocale(LC_ALL, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~StringTest()
    {
    }
};

TEST_F(StringTest, ConstructorSimple)
{
    ASSERT_NO_THROW({
        pplib::String s1;
        ASSERT_TRUE(s1.len() == 0) << "String does not have zero length";
        const char* buf = s1.getPtr();
        ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
        ASSERT_EQ(0, buf[0]) << "Class did not return an empty c-string";
    });
}

TEST_F(StringTest, ConstructorFromCharPtr)
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

TEST_F(StringTest, ConstructorFromString)
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

TEST_F(StringTest, ConstructorFromStdString)
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

TEST_F(StringTest, ConstructorFromStdWString)
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

TEST_F(StringTest, ConstructorFromWideString)
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

TEST_F(StringTest, clear)
{
    pplib::String s1("A test string with unicode characters: äöü");
    s1.clear();
    ASSERT_EQ((size_t)0, s1.len()) << "String does not have length of 0";
    const char* buf = s1.getPtr();
    ASSERT_TRUE(NULL != buf) << "Class did not return a pointer to a c-string";
    ASSERT_THROW({ ASSERT_EQ(0, (unsigned char)s1[0]) << "Unexpected Character in string"; }, pplib::OutOfBoundsException);
}

TEST_F(StringTest, CapacityAndReserve)
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

TEST_F(StringTest, len)
{
    pplib::String s1("A test string with unicode characters: äöü");
    ASSERT_EQ(s1.len(), (size_t)45) << "String has unexpected size";
}

TEST_F(StringTest, length)
{
    pplib::String s1("A test string with unicode characters: äöü");
    ASSERT_EQ(s1.length(), (size_t)45) << "String has unexpected size";
}

TEST_F(StringTest, size)
{
    pplib::String s1("A test string with unicode characters: äöü");
    ASSERT_EQ(s1.size(), (size_t)45) << "String has unexpected size";
}

TEST_F(StringTest, isEmpty)
{
    pplib::String s1("A test string with unicode characters: äöü");
    pplib::String s2;
    ASSERT_EQ(s1.isEmpty(), false) << "String is empty";
    ASSERT_EQ(s2.isEmpty(), true) << "String not empty";
}

TEST_F(StringTest, notEmpty)
{
    pplib::String s1("A test string with unicode characters: äöü");
    pplib::String s2;
    ASSERT_EQ(s1.notEmpty(), true) << "String is not empty";
    ASSERT_EQ(s2.notEmpty(), false) << "String is empty";
}

TEST_F(StringTest, isNumeric)
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
}

TEST_F(StringTest, isInteger)
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

TEST_F(StringTest, isTrue)
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
}

TEST_F(StringTest, isFalse)
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

TEST_F(StringTest, setConstCharWithoutSize)
{
    pplib::String s2("äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set("äöü, a test string with unicode characters");
    ASSERT_EQ(s1, s2) << "String has unexpected value";
    ASSERT_EQ((size_t)45, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setConstCharWithSize)
{
    pplib::String s2("äöü, a tes");
    pplib::String s1;
    s1.set("äöü, a test string with unicode characters", 13);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)13, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setConstWChartWithoutSize)
{
    pplib::String s2("äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set("äöü, a test string with unicode characters");
    ASSERT_EQ(s1, s2) << "String as unexpected value";
    ASSERT_EQ((size_t)45, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setConstWChartWithSize)
{
    pplib::String s2("äöü, a tes");
    pplib::String s1;
    s1.set("äöü, a test string with unicode characters", 13);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)13, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setStringRefWithoutSize)
{
    pplib::String s2("äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set(s2);
    ASSERT_EQ(s1, s2) << "String as unexpected value";
    ASSERT_EQ((size_t)45, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setStringRefWithSize)
{
    pplib::String s2("äöü, a tes");
    pplib::String s3("äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set(s3, 13);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)13, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setSTDStringRefWithoutSize)
{
    pplib::String s2("äöü, a test string with unicode characters");
    std::string s3("äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set(s3);
    ASSERT_EQ(s1, s2) << "String as unexpected value";
    ASSERT_EQ((size_t)45, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setSTDStringRefWithSize)
{
    pplib::String s2("äöü, a tes");
    std::string s3("äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set(s3, 13);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)13, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setSTDWStringRefWithoutSize)
{
    pplib::String s2("äöü, a test string with unicode characters");
    std::wstring s3(L"äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set(s3);
    ASSERT_EQ(s1, s2) << "String has unexpected value";
    ASSERT_EQ(s2.size(), s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setSTDWStringRefWithSize)
{
    pplib::String s2("äöü, a tes");
    std::wstring s3(L"äöü, a test string with unicode characters");
    pplib::String s1;
    s1.set(s3, 10);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ(s2.size(), s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setf)
{
    pplib::String s2("Ein Test, 42, Wide, 10000");
    pplib::String s1;
    s1.setf("Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)25, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setWchart)
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

TEST_F(StringTest, vasprintf)
{
    pplib::String s2("Ein Test, 42, Wide, 10000");
    pplib::String s1;
    test_vasprintf(s1, "Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
    ASSERT_EQ(s2, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)25, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendConstWchartWithoutSize)
{
    pplib::String expected("First Part äöü, äöü Second Part");
    pplib::String s1("First Part äöü, ");
    s1.append(L"äöü Second Part");

    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ(expected.size(), s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendConstWchartWithSize)
{
    pplib::String expected("First Part äöü, äöü S");
    pplib::String s1("First Part äöü, ");
    s1.append(L"äöü Second Part", 5);

    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ(expected.size(), s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendConstCharPtrWithoutSize)
{
    pplib::String expected("First Part äöü, äöü Second Part");
    pplib::String s1("First Part äöü, ");
    s1.append("äöü Second Part");

    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendConstCharPtrWithSize)
{
    pplib::String expected("First Part äöü, äöü S");
    pplib::String s1("First Part äöü, ");
    s1.append("äöü Second Part", 8);

    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)27, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendStringWithoutSize)
{
    pplib::String expected("First Part äöü, äöü Second Part");
    pplib::String s1("First Part äöü, ");
    pplib::String s2("äöü Second Part");
    s1.append(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendStringWithSize)
{
    pplib::String expected("First Part äöü, äöü S");
    pplib::String s1("First Part äöü, ");
    pplib::String s2("äöü Second Part");
    s1.append(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)27, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendStdStringWithoutSize)
{
    pplib::String expected("First Part äöü, äöü Second Part");
    pplib::String s1("First Part äöü, ");
    std::string s2("äöü Second Part");
    s1.append(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendStdStringWithSize)
{
    pplib::String expected("First Part äöü, äöü S");
    pplib::String s1("First Part äöü, ");
    std::string s2("äöü Second Part");
    s1.append(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)27, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendStdWStringWithoutSize)
{
    pplib::String expected("First Part äöü, äöü Second Part");
    pplib::String s1("First Part äöü, ");
    std::wstring s2(L"äöü Second Part");
    s1.append(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ(expected.size(), s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendStdWStringWithSize)
{
    pplib::String expected("First Part äöü, äöü Seco");
    pplib::String s1("First Part äöü, ");
    std::wstring s2(L"äöü Second Part");
    s1.append(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ(expected.size(), s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendf)
{
    pplib::String expected("First Part äöü, Ein Test, 42, Wide, 10000");
    pplib::String s1("First Part äöü, ");
    s1.appendf("Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)44, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendWchart)
{
    pplib::String expected("First Part äöü, a");
    pplib::String s1("First Part äöü, ");
    s1.append((wchar_t)'a');
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)20, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, append_same_string)
{
    pplib::String expected("It's me, It's me, It's me, It's me, ");
    pplib::String s1("It's me, ");
    s1.append(s1);
    s1.append(s1);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)36, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependConstWchartWithoutSize)
{
    pplib::String expected("äöü Second PartFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    s1.prepend("äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependConstWchartWithSize)
{
    pplib::String expected("äöü SFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    s1.prepend("äöü Second Part", 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)27, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependConstCharPtrWithoutSize)
{
    pplib::String expected("äöü Second PartFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    s1.prepend("äöü Second Part");
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependConstCharPtrWithSize)
{
    pplib::String expected("äöü SFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    s1.prepend("äöü Second Part", 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)27, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependStringWithoutSize)
{
    pplib::String expected("äöü Second PartFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    pplib::String s2("äöü Second Part");
    s1.prepend(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependStringWithSize)
{
    pplib::String expected("äöü SFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    pplib::String s2("äöü Second Part");
    s1.prepend(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)27, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependStdStringWithoutSize)
{
    pplib::String expected("äöü Second PartFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    std::string s2("äöü Second Part");
    s1.prepend(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)37, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependStdStringWithSize)
{
    pplib::String expected("äöü SFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    std::string s2("äöü Second Part");
    s1.prepend(s2, 8);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)27, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependStdWStringWithoutSize)
{
    pplib::String expected("äöü Second PartFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    std::wstring s2(L"äöü Second Part");
    s1.prepend(s2);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ(expected.size(), s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependStdWStringWithSize)
{
    pplib::String expected("äöü SFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    std::wstring s2(L"äöü Second Part");
    s1.prepend(s2, 5);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ(expected.size(), s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependf)
{
    pplib::String expected("Ein Test, 42, Wide, 10000First Part äöü, ");
    pplib::String s1("First Part äöü, ");
    s1.prependf("Ein %s, %i, %ls, %u", "Test", 42, L"Wide", 10000);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)44, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependWchart)
{
    pplib::String expected("aFirst Part äöü, ");
    pplib::String s1("First Part äöü, ");
    s1.prepend((wchar_t)'a');
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)20, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prepend_same_string)
{
    pplib::String expected("It's me, It's me, It's me, It's me, ");
    pplib::String s1("It's me, ");
    s1.prepend(s1);
    s1.prepend(s1);
    ASSERT_EQ(expected, s1) << "String has unexpected value";
    ASSERT_EQ((size_t)36, s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, chopRight)
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
}

TEST_F(StringTest, chopLeft)
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
}

TEST_F(StringTest, chop)
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

TEST_F(StringTest, chomp)
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

TEST_F(StringTest, cut_WithPos)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String expected("The quick ");
    s1.cut(10);
    ASSERT_EQ((size_t)10, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithPos0)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String expected("");
    s1.cut(0);
    ASSERT_EQ((size_t)0, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithPosBeyondStringLength)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String expected("The quick brown fox jumps over the lazy dog");
    s1.cut(100);
    ASSERT_EQ((size_t)43, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithLetter)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String expected("The quick ");
    s1.cut("b");
    ASSERT_EQ((size_t)10, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithNonexistingLetter)
{
    pplib::String s1("The quick brown fox");
    pplib::String expected("The quick brown fox");
    s1.cut("j");
    ASSERT_EQ((size_t)19, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithString)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String expected("The quick ");
    s1.cut("brown");
    ASSERT_EQ((size_t)10, s1.len()) << "String has unexpected length";
    ASSERT_EQ(expected, s1) << "String has unexpected value";
}

TEST_F(StringTest, strstr)
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

TEST_F(StringTest, repeated)
{
    pplib::String s1("_repeat_");
    pplib::String s3("_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
    pplib::String s2;
    s2 = s1.repeated(10);
    ASSERT_EQ((size_t)80, s2.len()) << "String does not have length of 80";
    ASSERT_EQ(s3, s2) << "String has unexpected value";
}

TEST_F(StringTest, repeat_by_count)
{
    pplib::String s1("_repeat_");
    s1.repeat(10);
    pplib::String s3("_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
    ASSERT_EQ((size_t)80, s1.len()) << "String does not have length of 80";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
}

TEST_F(StringTest, repeat_with_string)
{
    pplib::String s1("blah");
    s1.repeat(pplib::String("_repeat_"), 10);
    pplib::String s3("_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
    ASSERT_EQ((size_t)80, s1.len()) << "String does not have length of 80";
    ASSERT_EQ(s3, s1) << "String has unexpected value";
}

TEST_F(StringTest, trimLeft)
{
    pplib::String s1("\n\n    abc  \n");
    s1.trimLeft();
    ASSERT_EQ((size_t)6, s1.size());
    ASSERT_EQ(pplib::String("abc  \n"), s1);
}

TEST_F(StringTest, trimRight)
{
    pplib::String s1(" \n  abc  \n");
    s1.trimRight();
    ASSERT_EQ((size_t)7, s1.size());
    ASSERT_EQ(pplib::String(" \n  abc"), s1);
}

TEST_F(StringTest, trim)
{
    pplib::String s1("\n\n    abc  \n");
    s1.trim();
    ASSERT_EQ((size_t)3, s1.size());
    ASSERT_EQ(pplib::String("abc"), s1);
}

TEST_F(StringTest, trim_empty)
{
    pplib::String s1;
    s1.trim();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(), s1);
}

TEST_F(StringTest, trimmed)
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

TEST_F(StringTest, trimLeftEmptyResult)
{
    pplib::String s1("\n\n   \n   \n");
    s1.trimLeft();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);
}

TEST_F(StringTest, trimRightEmptyResult)
{
    pplib::String s1("\n\n   \n   \n");
    s1.trimRight();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);
}

TEST_F(StringTest, trimEmptyResult)
{
    pplib::String s1("\n\n   \n   \n");
    s1.trim();
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);
}

TEST_F(StringTest, trimLeftChars)
{
    pplib::String s1("\n\n    abc  \n");
    s1.trimLeft(" \n");
    ASSERT_EQ((size_t)6, s1.size());
    ASSERT_EQ(pplib::String("abc  \n"), s1);
}

TEST_F(StringTest, trimRightChars)
{
    pplib::String s1(" \n  abc  \n");
    s1.trimRight(" \n");
    ASSERT_EQ((size_t)7, s1.size());
    ASSERT_EQ(pplib::String(" \n  abc"), s1);
}

TEST_F(StringTest, trimChars)
{
    pplib::String s1("\n\n    abc  \n");
    s1.trim(" \n");
    ASSERT_EQ((size_t)3, s1.size());
    ASSERT_EQ(pplib::String("abc"), s1);
}

TEST_F(StringTest, trimLeftCharsEmptyResult)
{
    pplib::String s1("\n\n   \n   \n");
    s1.trimLeft(" \n");
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);
}

TEST_F(StringTest, trimRightCharsEmptyResult)
{
    pplib::String s1("\n\n   \n   \n");
    s1.trimRight(" \n");
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);
}

TEST_F(StringTest, trimEmptyCharsResult)
{
    pplib::String s1("\n\n   \n   \n");
    s1.trim(" \n");
    ASSERT_EQ((size_t)0, s1.size());
    ASSERT_EQ(pplib::String(""), s1);
}

TEST_F(StringTest, operatorEqualEmpty)
{
    pplib::String s1("");
    pplib::String s2("");
    ASSERT_EQ(s1, s2);
}

TEST_F(StringTest, strcmpEmpty)
{
    pplib::String s1("");
    pplib::String s2("");
    ASSERT_EQ(0, s1.strcmp(s2));
}

TEST_F(StringTest, strcmpLower)
{
    pplib::String s1("ABcdef");
    pplib::String s2("Defghi");
    ASSERT_LT(s1.strcmp(s2), 0);
}

TEST_F(StringTest, strcmpLowerWithCase)
{
    pplib::String s1("ABcdef");
    pplib::String s2("abcdef");
    ASSERT_LT(s1.strcmp(s2), 0);
}

TEST_F(StringTest, strcmpHigher)
{
    pplib::String s1("ABcdef");
    pplib::String s2("Defghi");
    ASSERT_GT(s2.strcmp(s1), 0);
}

TEST_F(StringTest, strcmpHigherWithCase)
{
    pplib::String s1("ABcdef");
    pplib::String s2("abcdef");
    ASSERT_GT(s2.strcmp(s1), 0);
}

TEST_F(StringTest, strcmpEqual)
{
    pplib::String s1("ABcdef");
    pplib::String s2("ABcdef");
    ASSERT_EQ(s2.strcmp(s1), 0);
}

TEST_F(StringTest, strcasecmpEmpty)
{
    pplib::String s1("");
    pplib::String s2("");
    ASSERT_EQ(0, s1.strCaseCmp(s2));
}

TEST_F(StringTest, strcasecmpLower)
{
    pplib::String s1("ABcdef");
    pplib::String s2("Defghi");
    ASSERT_LT(s1.strCaseCmp(s2), 0);
}

TEST_F(StringTest, strcasecmpLowerWithCase)
{
    pplib::String s1("ABcdef");
    pplib::String s2("abcdef");
    ASSERT_EQ(s1.strCaseCmp(s2), 0);
}

TEST_F(StringTest, strcasecmpHigher)
{
    pplib::String s1("ABcdef");
    pplib::String s2("Defghi");
    ASSERT_GT(s2.strCaseCmp(s1), 0);
}

TEST_F(StringTest, strcasecmpHigherWithCase)
{
    pplib::String s1("ABcdef");
    pplib::String s2("abcdef");
    ASSERT_EQ(s2.strCaseCmp(s1), 0);
}

TEST_F(StringTest, strcasecmpEqual)
{
    pplib::String s1("ABcdef");
    pplib::String s2("ABcdef");
    ASSERT_EQ(s2.strCaseCmp(s1), 0);
}

TEST_F(StringTest, left)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String s2 = s1.left(10);
    ASSERT_EQ(pplib::String("The quick "), s2);
}

TEST_F(StringTest, right)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String s2 = s1.right(9);
    ASSERT_EQ(pplib::String(" lazy dog"), s2);
}

TEST_F(StringTest, midWithLength)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String s2 = s1.mid(10, 10);
    ASSERT_EQ(pplib::String("brown fox "), s2);
}

TEST_F(StringTest, midWithoutLength)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String s2 = s1.mid(10);
    ASSERT_EQ(pplib::String("brown fox jumps over the lazy dog"), s2);
}

TEST_F(StringTest, substrWithLength)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String s2 = s1.substr(10, 10);
    ASSERT_EQ(pplib::String("brown fox "), s2);
}

TEST_F(StringTest, substrWithoutLength)
{
    pplib::String s1("The quick brown fox jumps over the lazy dog");
    pplib::String s2 = s1.substr(10);
    ASSERT_EQ(pplib::String("brown fox jumps over the lazy dog"), s2);
}

TEST_F(StringTest, lowerCase)
{
    pplib::String s1("The Quick Brown Fox Jumps over ÄÖÜ");
    pplib::String expected("the quick brown fox jumps over äöü");
    ASSERT_NO_THROW(s1.lowerCase());
    ASSERT_EQ(expected, s1);
}

TEST_F(StringTest, upperCase)
{
    pplib::String s1("The Quick Brown Fox Jumps over äöü");
    pplib::String expected("THE QUICK BROWN FOX JUMPS OVER ÄÖÜ");
    ASSERT_NO_THROW(s1.upperCase());
    ASSERT_EQ(expected, s1);
}

TEST_F(StringTest, toLowerCase)
{
    ASSERT_EQ(pplib::String(L"the quick brown fox jumps over äöü"), pplib::String(L"The Quick Brown Fox Jumps over ÄÖÜ").toLowerCase());
}

TEST_F(StringTest, toUpperCase)
{
    ASSERT_EQ(pplib::String(L"THE QUICK BROWN FOX JUMPS OVER ÄÖÜ"), pplib::String(L"The Quick Brown Fox Jumps over äöü").toUpperCase());
}

TEST_F(StringTest, strchr_ExistingChar)
{
    pplib::String s1("The Quick Brown Fox Jumps over äöü");
    pplib::String expected("Fox Jumps over äöü");
    ASSERT_EQ(expected, s1.strchr('F')) << "Unexpected Result";
}

TEST_F(StringTest, strchr_NonExistingChar)
{
    pplib::String s1("The Quick Brown Fox Jumps over äöü");
    pplib::String expected("");
    ASSERT_EQ(expected, s1.strchr('L')) << "Unexpected Result";
}

TEST_F(StringTest, strrchr_ExistingChar)
{
    pplib::String s1("The Quick Brown Fox Jumps over äöü");
    pplib::String expected("over äöü");
    ASSERT_EQ(expected, s1.strrchr('o')) << "Unexpected Result";
}

TEST_F(StringTest, strrchr_NonExistingChar)
{
    pplib::String s1("The Quick Brown Fox Jumps over äöü");
    pplib::String expected("");
    ASSERT_EQ(expected, s1.strrchr('L')) << "Unexpected Result";
}

TEST_F(StringTest, toInt)
{
    EXPECT_EQ((int)1234, pplib::String("1234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)-1234, pplib::String("-1234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)1234, pplib::String("01234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)-1234, pplib::String("-01234").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)0, pplib::String("abc123").toInt()) << "Unexpected Result";
    EXPECT_EQ((int)0, pplib::String("0x1234").toInt()) << "Unexpected Result";
}

TEST_F(StringTest, toUnsignedInt)
{
    EXPECT_EQ((unsigned int)1234, pplib::String("1234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)-1234, pplib::String("-1234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)1234, pplib::String("01234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)-1234, pplib::String("-01234").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)0, pplib::String("abc123").toUnsignedInt()) << "Unexpected Result";
    EXPECT_EQ((unsigned int)0, pplib::String("0x1234").toUnsignedInt()) << "Unexpected Result";
}

TEST_F(StringTest, toInt64)
{
    EXPECT_EQ((int64_t)1234, pplib::String("1234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)-1234, pplib::String("-1234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)1234, pplib::String("01234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)-1234, pplib::String("-01234").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)0, pplib::String("abc123").toInt64()) << "Unexpected Result";
    EXPECT_EQ((int64_t)0, pplib::String("0x1234").toInt64()) << "Unexpected Result";
}

TEST_F(StringTest, toUnsignedInt64)
{
    EXPECT_EQ((uint64_t)1234, pplib::String("1234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)-1234, pplib::String("-1234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)1234, pplib::String("01234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)-1234, pplib::String("-01234").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)0, pplib::String("abc123").toUnsignedInt64()) << "Unexpected Result";
    EXPECT_EQ((uint64_t)0, pplib::String("0x1234").toUnsignedInt64()) << "Unexpected Result";
}

TEST_F(StringTest, toInt_withoutNumber)
{
    pplib::String s1("The Quick Brown Fox Jumps over the lazy dog");
    EXPECT_EQ((int)0, s1.toInt()) << "Unexpected Result";
}

TEST_F(StringTest, toInt_123456)
{
    pplib::String s1("123456");
    EXPECT_EQ((int)123456, s1.toInt()) << "Unexpected Result";
}

TEST_F(StringTest, ToInt_minus123456)
{
    pplib::String s1("-123456");
    EXPECT_EQ((int)-123456, s1.toInt()) << "Unexpected Result";
}

TEST_F(StringTest, toInt_123456_point_567)
{
    pplib::String s1("123456.567");
    EXPECT_EQ((int)123456, s1.toInt()) << "Unexpected Result";
}

TEST_F(StringTest, toInt64_1242346214893456)
{
    pplib::String s1("1242346214893456");
    EXPECT_EQ((int64_t)1242346214893456, s1.toInt64()) << "Unexpected Result";
}

TEST_F(StringTest, toInt64_minus1242346214893456)
{
    pplib::String s1("-1242346214893456");
    EXPECT_EQ((int64_t)-1242346214893456, s1.toInt64()) << "Unexpected Result";
}

TEST_F(StringTest, toBool)
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

TEST_F(StringTest, ToLong_1124234674)
{
    pplib::String s1("124234674");
    EXPECT_EQ((long)124234674, s1.toLong()) << "Unexpected Result";
}

TEST_F(StringTest, toLongLong_1242346214893456)
{
    pplib::String s1("1242346214893456");
    EXPECT_EQ((long long)1242346214893456, s1.toLongLong()) << "Unexpected Result";
}

TEST_F(StringTest, ToFloat_182566142_346214893456)
{
    pplib::String s1("182566142.346214893456");
    EXPECT_EQ((float)182566142.346214893456, s1.toFloat()) << "Unexpected Result";
}

TEST_F(StringTest, ToDouble_182566142_346214893456)
{
    pplib::String s1("182566142.346214893456");
    EXPECT_EQ(((float)182566142.346214893456), ((float)s1.toDouble())) << "Unexpected Result";
}

TEST_F(StringTest, OperatorCharPositionPositiv)
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

TEST_F(StringTest, OperatorCharPositionNegativ)
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

TEST_F(StringTest, join)
{
    pplib::Array a;
    a.add("One");
    a.add("Two");
    a.add("Three");
    pplib::String s1(",");
    EXPECT_EQ(pplib::String("One,Two,Three"), s1.join(a));
}

} // namespace

#ifdef old
printf("sizeof String: %zd\n", sizeof(pplib::String));
if (setlocale(LC_CTYPE, "de_DE.UTF-8") == NULL) {
    printf("setlocale fehlgeschlagen\n");
    return 1;
}

try {
    // pplib::String::setGlobalEncoding("iso-8859-1");
    pplib::String::setGlobalEncoding("UTF-8");
}
catch (pplib::NullPointerException) {
    printf("NPE\n");
    return 1;
}
catch (pplib::UnsupportedFeatureException) {
    printf("Transkodierung wird nicht unterstützt\n");
    return 1;
}
catch (pplib::UnsupportedCharacterEncodingException) {
    printf("UnsupportedFeatureException: Transkodierung wird nicht unterstützt\n");
    return 1;
}
catch (...) {
    printf("Unbekannte Exception\n");
    throw;
}

pplib::String s1;
s1.reserve(128);

s1.setf("Test mit UTF-8 Umlaütän ß!");
printf("capacity: %zd, length: %zd\n", s1.capacity(), s1.length());

// s1.set(L"Test mit ISO-8859-1 Umla�t�n �!");
s1.printnl();

s1.append("Noch mehr UTF-8");
printf("capacity: %zd, length: %zd\n", s1.capacity(), s1.length());
s1.printnl();

s1.appendf("capacity: %zd\n", s1.capacity());

printf("capacity: %zd, length: %zd\n", s1.capacity(), s1.length());
s1.printnl();

// printf ("UTF-8 String als const char*: >>>%s<<<\n",(const char*)s1.toUtf8());

printf("prepend\n");
s1.prependf("Davor :-)  =>");
s1.printnl();

printf("ok\n");

s1 = ".String1.";
pplib::String s2;
// s2=s1+"Hallo Welt"+s1+"Noch mehr"+s1;
s2 = s1 + "Hallo Welt" + s1 + "Noch mehr";
s2.printnl();

pplib::String s3 = "Eiñ Täśt zµm Kôdiéren";
pplib::ByteArray b = s3.toEncoding("UTF-8");
b.hexDump();

printf("left: %ls\n", (const wchar_t*)s3.left(3));
printf("right: %ls\n", (const wchar_t*)s3.right(4));
printf("mid (2): %ls\n", (const wchar_t*)s3.mid(2));
printf("mid (2,5): %ls\n", (const wchar_t*)s3.mid(2, 5));
printf("mid (40,5): %ls\n", (const wchar_t*)s3.mid(40, 5));

// s1.hexDump();
return 0;
}
#endif
