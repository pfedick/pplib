/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
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
#include <pthread.h>
#include <locale.h>
#include <ppl6.h>
#include <gtest/gtest.h>
#include "ppl6-tests.h"

namespace {

// The fixture for testing class Foo.
class StringTest : public ::testing::Test {
	protected:
	StringTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl6::SetGlobalEncoding("UTF-8");
	}
	virtual ~StringTest() {

	}
};

TEST_F(StringTest, ConstructorSimple) {
	ASSERT_NO_THROW({
			ppl6::CString s1;
			ASSERT_TRUE(s1.Len()==0) << "String does not have zero length";
			const char *buf=s1.GetPtr();
			ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
			ASSERT_EQ(0,buf[0]) << "Class did not return an empty c-string";

	}
	);
}

TEST_F(StringTest, ConstructorFromCharPtr) {
	ASSERT_NO_THROW({
			ppl6::CString s1("A test string with unicode characters: äöü");
			ASSERT_EQ((size_t)45,s1.Len()) << "String does not have length of 42";
			const unsigned char *buf=(const unsigned char *)s1.GetPtr();
			ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
			ASSERT_EQ('A',(unsigned char)s1[0]) << "Unexpected Character in string";
			ASSERT_EQ(188,(unsigned char)s1[44]) << "Unexpected Character in string";

	}
	);
}

TEST_F(StringTest, ConstructorFromWCharPtr) {
	ASSERT_NO_THROW({
			ppl6::CString s1(L"A test string with unicode characters: äöü");
			ASSERT_EQ((size_t)45,s1.Len()) << "String does not have length of 42";
			const unsigned char *buf=(const unsigned char *)s1.GetPtr();
			ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
			ASSERT_EQ('A',(unsigned char)s1[0]) << "Unexpected Character in string";
			ASSERT_EQ(188,(unsigned char)s1[44]) << "Unexpected Character in string";

	}
	);
}

TEST_F(StringTest, ConstructorFromStringPtr) {
	ASSERT_NO_THROW({
		ppl6::CString s1("A test string with unicode characters: äöü");
		ppl6::CString s2(&s1);
		ASSERT_EQ((size_t)45,s2.Len()) << "String does not have length of 45";
		const char *buf=s2.GetPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ('A',(unsigned char)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(188,(unsigned char)s2[44]) << "Unexpected Character in string";
	}
	);
}


TEST_F(StringTest, ConstructorFromString) {
	ASSERT_NO_THROW({
		ppl6::CString s1("A test string with unicode characters: äöü");
		ppl6::CString s2(s1);
		ASSERT_EQ((size_t)45,s2.Len()) << "String does not have length of 45";
		const char *buf=s2.GetPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ('A',(unsigned char)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(188,(unsigned char)s2[44]) << "Unexpected Character in string";
	});
}

TEST_F(StringTest, ConstructorFromWString) {
	ASSERT_NO_THROW({
		ppl6::CWString s1(L"A test string with unicode characters: äöü");
		ppl6::CString s2(s1);
		ASSERT_EQ((size_t)45,s2.Len()) << "String does not have length of 45";
		const char *buf=s2.GetPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ('A',(unsigned char)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(188,(unsigned char)s2[44]) << "Unexpected Character in string";
	});
}


TEST_F(StringTest, ConstructorFromStdString) {
	ASSERT_NO_THROW({
		std::string s1("A test string with unicode characters: äöü");
		ppl6::CString s2(s1);
		ASSERT_EQ((size_t)45,s2.Len()) << "String does not have length of 45";
		const char *buf=s2.GetPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ('A',(unsigned char)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(188,(unsigned char)s2[44]) << "Unexpected Character in string";
	});
}



TEST_F(StringTest, clear) {
	ppl6::CString s1("A test string with unicode characters: äöü");
	s1.Clear();
	ASSERT_EQ((size_t)0,s1.Len()) << "String does not have length of 0";
	const char *buf=s1.GetPtr();
	ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
	ASSERT_NO_THROW({
			ASSERT_EQ(0,(unsigned char)s1[0]) << "Unexpected Character in string";
	});
}

TEST_F(StringTest, CapacityAndReserve) {
	ppl6::CString s1;
	ASSERT_EQ((size_t)0,s1.Capacity()) << "capacity did not return 0";
	s1.Reserve(0);
	ASSERT_EQ((size_t)0,s1.Capacity()) << "capacity did not return 0";
	s1.Reserve(32);
	ASSERT_EQ((size_t)32,s1.Capacity()) << "capacity did not return expected value";
	s1.Reserve(128);
	ASSERT_EQ((size_t)128,s1.Capacity()) << "capacity did not return expected value";
	s1.Reserve(32);
	ASSERT_EQ((size_t)128,s1.Capacity()) << "capacity did not return expected value";
}


TEST_F(StringTest, len) {
	ppl6::CString s1("A test string with unicode characters: äöü");
	ASSERT_EQ(s1.Len(),(size_t)45) << "String has unexpected size";
}

TEST_F(StringTest, length) {
	ppl6::CString s1("A test string with unicode characters: äöü");
	ASSERT_EQ(s1.Length(),(size_t)45) << "String has unexpected size";
}

TEST_F(StringTest, size) {
	ppl6::CString s1("A test string with unicode characters: äöü");
	ASSERT_EQ(s1.Size(),(size_t)45) << "String has unexpected size";
}

TEST_F(StringTest, isEmpty) {
	ppl6::CString s1("A test string with unicode characters: äöü");
	ppl6::CString s2;
	ASSERT_EQ(s1.IsEmpty(),false) << "String is empty";
	ASSERT_EQ(s2.IsEmpty(),true) << "String not empty";
}

TEST_F(StringTest, notEmpty) {
	ppl6::CString s1("A test string with unicode characters: äöü");
	ppl6::CString s2;
	ASSERT_EQ(s1.NotEmpty(),true) << "String is not empty";
	ASSERT_EQ(s2.NotEmpty(),false) << "String is empty";
}

TEST_F(StringTest, isNumeric) {
	ppl6::CString s1("A test string with unicode characters: äöü");
	ASSERT_EQ(s1.IsNumeric(),false) << "String should not be numeric";
	s1.Set("");
	ASSERT_EQ(s1.IsNumeric(),false) << "String should not be numeric";
	s1.Set("12345abcd");
	ASSERT_EQ(s1.IsNumeric(),false) << "String should not be numeric";
	s1.Set("12345");
	ASSERT_EQ(s1.IsNumeric(),true) << "String should be numeric";
	s1.Set("123.451,12");
	ASSERT_EQ(s1.IsNumeric(),true) << "String should be numeric";
	s1.Set("-123.451,12");
	ASSERT_EQ(s1.IsNumeric(),true) << "String should be numeric";
	s1.Set("-123.451,12-9");
	ASSERT_EQ(s1.IsNumeric(),false) << "String should not be numeric";
}

TEST_F(StringTest, isInteger) {
	ppl6::CString s1("A test string with unicode characters: äöü");
	ASSERT_EQ(s1.IsInteger(),false) << "String should not be an integer";
	s1.Set("");
	ASSERT_EQ(s1.IsInteger(),false) << "String should not be an integer";
	s1.Set("12345abcd");
	ASSERT_EQ(s1.IsInteger(),false) << "String should not be an integer";
	s1.Set("12345");
	ASSERT_EQ(s1.IsInteger(),true) << "String should be an integer";
	s1.Set("-12345");
	ASSERT_EQ(s1.IsInteger(),true) << "String should be an integer";
	s1.Set("123-451");
	ASSERT_EQ(s1.IsInteger(),false) << "String should be an integer";
	s1.Set("123.451,12");
	ASSERT_EQ(s1.IsInteger(),false) << "String should be an integer";
	s1.Set("-123.451,12");
	ASSERT_EQ(s1.IsInteger(),false) << "String should be an integer";
	s1.Set("-123.451,12-9");
	ASSERT_EQ(s1.IsInteger(),false) << "String should not be an integer";
}


TEST_F(StringTest, setConstCharWithoutSize) {
	ppl6::CString s2("äöü, a test string with unicode characters");
	ppl6::CString s1;
	s1.Set("äöü, a test string with unicode characters");
	ASSERT_EQ(s1,s2) << "String has unexpected value";
	ASSERT_EQ((size_t)45,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, setConstCharWithSize) {
	ppl6::CString s2("äöü, a tes");
	ppl6::CString s1;
	s1.Set("äöü, a test string with unicode characters",13);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)13,s1.Size()) << "String has unexpected length";
}


TEST_F(StringTest, setConstWChartWithoutSize) {
	ppl6::CString s2("äöü, a test string with unicode characters");
	ppl6::CString s1;
	s1.Set("äöü, a test string with unicode characters");
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)45,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, setConstWChartWithSize) {
	ppl6::CString s2("äöü, a test s");
	ppl6::CString s1;
	s1.Set(L"äöü, a test string with unicode characters",13);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)16,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, setStringPtrWithoutSize) {
	ppl6::CString s2("äöü, a test string with unicode characters");
	ppl6::CString s1;
	s1.Set(&s2);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)45,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, setStringPtrWithSize) {
	ppl6::CString s2("äöü, a tes");
	ppl6::CString s3("äöü, a test string with unicode characters");
	ppl6::CString s1;
	s1.Set(&s3,13);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)13,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, setStringRefWithoutSize) {
	ppl6::CString s2("äöü, a test string with unicode characters");
	ppl6::CString s1;
	s1.Set(s2);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)45,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, setStringRefWithSize) {
	ppl6::CString s2("äöü, a tes");
	ppl6::CString s3("äöü, a test string with unicode characters");
	ppl6::CString s1;
	s1.Set(s3,13);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)13,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, setSTDStringRefWithoutSize) {
	ppl6::CString s2("äöü, a test string with unicode characters");
	std::string s3("äöü, a test string with unicode characters");
	ppl6::CString s1;
	s1.Set(s3);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)45,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, setSTDStringRefWithSize) {
	ppl6::CString s2("äöü, a tes");
	std::string s3("äöü, a test string with unicode characters");
	ppl6::CString s1;
	s1.Set(s3,13);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)13,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, setSTDWStringRefWithoutSize) {
	ppl6::CString s2("äöü, a test string with unicode characters");
	std::wstring s3(L"äöü, a test string with unicode characters");
	ppl6::CString s1;
	s1.Set(s3);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)45,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, setSTDWStringRefWithSize) {
	ppl6::CString s2("äöü, a test s");
	std::wstring s3(L"äöü, a test string with unicode characters");
	ppl6::CString s1;
	s1.Set(s3,13);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)16,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, setf) {
	ppl6::CString s2("Ein Test, 42, Wide, 10000");
	ppl6::CString s1;
	s1.Setf("Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)25,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, sprintf) {
	ppl6::CString s2("Ein Test, 42, Wide, 10000");
	ppl6::CString s1;
	s1.Sprintf("Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)25,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, setchart) {
	ppl6::CString s2("a");
	ppl6::CString s1;
	s1.SetChar('a');
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)1,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, setCharPos) {
	ppl6::CString expected(L"Ahe Quick Arown Fox Jumps over the lazy doAB");
	ppl6::CString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	s1.SetChar('A',-1);
	s1.SetChar('A',73);
	s1.SetChar('A',0);
	s1.SetChar('A',10);
	s1.SetChar('B',43);

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)44,s1.Length()) << "String has unexpected length";
}

TEST_F(StringTest, AddChar) {
	ppl6::CString expected(L"The Quick Brown Foxx");
	ppl6::CString s1(L"The Quick Brown Fox");
	s1.AddChar('x');
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)20,s1.Length()) << "String has unexpected length";
}


static void test_VaSprintf(ppl6::CString &str,const char *fmt,...)
{
	va_list args;
	va_start(args, fmt);
	str.VaSprintf(fmt,args);
	va_end(args);
}


TEST_F(StringTest, VaSprintf) {
	ppl6::CString s2("Ein Test, 42, Wide, 10000");
	ppl6::CString s1;
	test_VaSprintf(s1,"Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)25,s1.Size()) << "String has unexpected length";
}

static void test_Vasprintf(ppl6::CString &str,const char *fmt,...)
{
	va_list args;
	va_start(args, fmt);
	str.VaSprintf(fmt,args);
	va_end(args);
}


TEST_F(StringTest, Vasprintf) {
	ppl6::CString s2("Ein Test, 42, Wide, 10000");
	ppl6::CString s1;
	test_Vasprintf(s1,"Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)25,s1.Size()) << "String has unexpected length";
}


TEST_F(StringTest, ConcatConstCharPtrWithoutSize) {
	ppl6::CString expected("First Part äöü, äöü Second Part");
	ppl6::CString s1("First Part äöü, ");
	s1.Concat("äöü Second Part");

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, ConcatConstCharPtrWithSize) {
	ppl6::CString expected("First Part äöü, äöü S");
	ppl6::CString s1("First Part äöü, ");
	s1.Concat("äöü Second Part",8);

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)27,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, ConcatStringWithoutSize) {
	ppl6::CString expected("First Part äöü, äöü Second Part");
	ppl6::CString s1("First Part äöü, ");
	ppl6::CString s2("äöü Second Part");
	s1.Concat(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, ConcatStringWithSize) {
	ppl6::CString expected("First Part äöü, äöü S");
	ppl6::CString s1("First Part äöü, ");
	ppl6::CString s2("äöü Second Part");
	s1.Concat(s2,8);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)27,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, Concatf) {
	ppl6::CString expected("First Part äöü, Ein Test, 42, Wide, 10000");
	ppl6::CString s1("First Part äöü, ");
	s1.Concatf("Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)44,s1.Size()) << "String has unexpected length";
}

TEST_F(StringTest, copy_charptr_without_size) {
	ppl6::CString expected("who let the dogs out?");
	ppl6::CString s1("The Quick Brown Fox Jumps over the lazy dog");
	s1.Copy("who let the dogs out?");
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.Length()) << "String has unexpected length";
}

TEST_F(StringTest, copy_charptr_with_size) {
	ppl6::CString expected("who ");
	ppl6::CString s1("The Quick Brown Fox Jumps over the lazy dog");
	s1.Copy("who let the dogs out?",4);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)4,s1.Length()) << "String has unexpected length";
}

TEST_F(StringTest, copy_CString_without_size) {
	ppl6::CString expected("who let the dogs out?");
	ppl6::CString s1("The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CString s2("who let the dogs out?");
	s1.Copy(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.Length()) << "String has unexpected length";
}

TEST_F(StringTest, copy_CString_with_size) {
	ppl6::CString expected("who ");
	ppl6::CString s1("The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CString s2("who let the dogs out?");
	s1.Copy(s2,4);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)4,s1.Length()) << "String has unexpected length";
}

TEST_F(StringTest, strcpy_CString) {
	ppl6::CString expected("who let the dogs out?");
	ppl6::CString s1("The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CString s2("who let the dogs out?");
	s1.Strcpy(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.Length()) << "String has unexpected length";
}

TEST_F(StringTest, strncpy_CString) {
	ppl6::CString expected("who ");
	ppl6::CString s1("The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CString s2("who let the dogs out?");
	s1.Strncpy(s2,4);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)4,s1.Length()) << "String has unexpected length";
}

TEST_F(StringTest, strcpy_CharPtr) {
	ppl6::CString expected("who let the dogs out?");
	ppl6::CString s1("The Quick Brown Fox Jumps over the lazy dog");
	s1.Strcpy("who let the dogs out?");
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.Length()) << "String has unexpected length";
}

TEST_F(StringTest, strncpy_CharPtr) {
	ppl6::CString expected("who ");
	ppl6::CString s1("The Quick Brown Fox Jumps over the lazy dog");
	s1.Strncpy("who let the dogs out?",4);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)4,s1.Length()) << "String has unexpected length";
}

TEST_F(StringTest, chop) {
	ppl6::CString s1("A test string with unicode characters: äöü");
	ppl6::CString s2("A test string with unicode characters: ä");
	ppl6::CString s3("A test string with unicode characters: ");
	s1.Chop(4);
	ASSERT_EQ((size_t)41,s1.Len()) << "String does not have length of 40";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	s1.Chop();
	s1.Chop();
	ASSERT_EQ((size_t)39,s1.Len()) << "String does not have length of 39";
	ASSERT_EQ(s3,s1) << "String has unexpected value";
	s1.Chop(39);
	ASSERT_EQ((size_t)0,s1.Len()) << "String does not have length of 0";
	s2.Clear();
	ASSERT_EQ(s2,s1) << "String has unexpected value";
}

TEST_F(StringTest, chomp) {
	ppl6::CString s1("\n\r\nA test string\näöü\n\r\n");
	ppl6::CString s2("A test string\näöü");
	s1.Chomp();
	ASSERT_EQ((size_t)20,s1.Len()) << "String does not have length of 17";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	s1.Chomp();
	ASSERT_EQ((size_t)20,s1.Len()) << "String does not have length of 17";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithPos) {
	ppl6::CString s1("The quick brown fox jumps over the lazy dog");
	ppl6::CString expected("The quick ");
	s1.Cut(10);
	ASSERT_EQ((size_t)10,s1.Len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithPos0) {
	ppl6::CString s1("The quick brown fox jumps over the lazy dog");
	ppl6::CString expected("");
	s1.Cut(0);
	ASSERT_EQ((size_t)0,s1.Len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithPosBeyondStringLength) {
	ppl6::CString s1("The quick brown fox jumps over the lazy dog");
	ppl6::CString expected("The quick brown fox jumps over the lazy dog");
	s1.Cut(100);
	ASSERT_EQ((size_t)43,s1.Len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithLetter) {
	ppl6::CString s1("The quick brown fox jumps over the lazy dog");
	ppl6::CString expected("The quick ");
	s1.Cut("b");
	ASSERT_EQ((size_t)10,s1.Len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithNonexistingLetter) {
	ppl6::CString s1("The quick brown fox");
	ppl6::CString expected("The quick brown fox");
	s1.Cut("j");
	ASSERT_EQ((size_t)19,s1.Len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithString) {
	ppl6::CString s1("The quick brown fox jumps over the lazy dog");
	ppl6::CString expected("The quick ");
	s1.Cut("brown");
	ASSERT_EQ((size_t)10,s1.Len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}


TEST_F(StringTest, repeate) {
	ppl6::CString s1("_repeat_");
	ppl6::CString s3("_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
	ppl6::CString s2;
	s2=s1.Repeat(10);
	ASSERT_EQ((size_t)80,s2.Len()) << "String does not have length of 80";
	ASSERT_EQ(s3,s2) << "String has unexpected value";
}

TEST_F(StringTest, trimLeft) {
	ppl6::CString s1("\n\n    abc  \n");
	s1.LTrim();
	ASSERT_EQ((size_t)6,s1.Size());
	ASSERT_EQ(ppl6::CString("abc  \n"),s1);
}

TEST_F(StringTest, trimRight) {
	ppl6::CString s1(" \n  abc  \n");
	s1.RTrim();
	ASSERT_EQ((size_t)7,s1.Size());
	ASSERT_EQ(ppl6::CString(" \n  abc"),s1);
}

TEST_F(StringTest, trim) {
	ppl6::CString s1("\n\n    abc  \n");
	s1.Trim();
	ASSERT_EQ((size_t)3,s1.Size());
	ASSERT_EQ(ppl6::CString("abc"),s1);
}


TEST_F(StringTest, trimLeftEmptyResult) {
	ppl6::CString s1("\n\n   \n   \n");
	s1.LTrim();
	ASSERT_EQ((size_t)0,s1.Size());
	ASSERT_EQ(ppl6::CString(""),s1);
}

TEST_F(StringTest, trimRightEmptyResult) {
	ppl6::CString s1("\n\n   \n   \n");
	s1.RTrim();
	ASSERT_EQ((size_t)0,s1.Size());
	ASSERT_EQ(ppl6::CString(""),s1);
}

TEST_F(StringTest, trimEmptyResult) {
	ppl6::CString s1("\n\n   \n   \n");
	s1.Trim();
	ASSERT_EQ((size_t)0,s1.Size());
	ASSERT_EQ(ppl6::CString(""),s1);
}

TEST_F(StringTest, trimLeftChars) {
	ppl6::CString s1("\n\n    abc  \n");
	s1.LTrim(" \n");
	ASSERT_EQ((size_t)6,s1.Size());
	ASSERT_EQ(ppl6::CString("abc  \n"),s1);
}

TEST_F(StringTest, trimRightChars) {
	ppl6::CString s1(" \n  abc  \n");
	s1.RTrim(" \n");
	ASSERT_EQ((size_t)7,s1.Size());
	ASSERT_EQ(ppl6::CString(" \n  abc"),s1);
}

TEST_F(StringTest, trimChars) {
	ppl6::CString s1("\n\n    abc  \n");
	s1.Trim(" \n");
	ASSERT_EQ((size_t)3,s1.Size());
	ASSERT_EQ(ppl6::CString("abc"),s1);
}

TEST_F(StringTest, trimLeftCharsEmptyResult) {
	ppl6::CString s1("\n\n   \n   \n");
	s1.LTrim(" \n");
	ASSERT_EQ((size_t)0,s1.Size());
	ASSERT_EQ(ppl6::CString(""),s1);
}

TEST_F(StringTest, trimRightCharsEmptyResult) {
	ppl6::CString s1("\n\n   \n   \n");
	s1.RTrim(" \n");
	ASSERT_EQ((size_t)0,s1.Size());
	ASSERT_EQ(ppl6::CString(""),s1);
}

TEST_F(StringTest, trimEmptyCharsResult) {
	ppl6::CString s1("\n\n   \n   \n");
	s1.Trim(" \n");
	ASSERT_EQ((size_t)0,s1.Size());
	ASSERT_EQ(ppl6::CString(""),s1);
}

TEST_F(StringTest, operatorEqualEmpty) {
	ppl6::CString s1("");
	ppl6::CString s2("");
	ASSERT_EQ(s1,s2);
}

TEST_F(StringTest, strcmpEmpty) {
	ppl6::CString s1("");
	ppl6::CString s2("");
	ASSERT_EQ(0,s1.StrCmp(s2));
}

TEST_F(StringTest, strcmpLower) {
	ppl6::CString s1("ABcdef");
	ppl6::CString s2("Defghi");
	ASSERT_LT(s1.StrCmp(s2),0);
}

TEST_F(StringTest, strcmpLowerWithCase) {
	ppl6::CString s1("ABcdef");
	ppl6::CString s2("abcdef");
	ASSERT_LT(s1.StrCmp(s2),0);
}

TEST_F(StringTest, strcmpHigher) {
	ppl6::CString s1("ABcdef");
	ppl6::CString s2("Defghi");
	ASSERT_GT(s2.StrCmp(s1),0);
}

TEST_F(StringTest, strcmpHigherWithCase) {
	ppl6::CString s1("ABcdef");
	ppl6::CString s2("abcdef");
	ASSERT_GT(s2.StrCmp(s1),0);
}

TEST_F(StringTest, strcmpEqual) {
	ppl6::CString s1("ABcdef");
	ppl6::CString s2("ABcdef");
	ASSERT_EQ(s2.StrCmp(s1),0);
}

TEST_F(StringTest, strcasecmpEmpty) {
	ppl6::CString s1("");
	ppl6::CString s2("");
	ASSERT_EQ(0,s1.StrCaseCmp(s2));
}

TEST_F(StringTest, strcasecmpLower) {
	ppl6::CString s1("ABcdef");
	ppl6::CString s2("Defghi");
	ASSERT_LT(s1.StrCaseCmp(s2),0);
}

TEST_F(StringTest, strcasecmpLowerWithCase) {
	ppl6::CString s1("ABcdef");
	ppl6::CString s2("abcdef");
	ASSERT_EQ(s1.StrCaseCmp(s2),0);
}

TEST_F(StringTest, strcasecmpHigher) {
	ppl6::CString s1("ABcdef");
	ppl6::CString s2("Defghi");
	ASSERT_GT(s2.StrCaseCmp(s1),0);
}

TEST_F(StringTest, strcasecmpHigherWithCase) {
	ppl6::CString s1("ABcdef");
	ppl6::CString s2("abcdef");
	ASSERT_EQ(s2.StrCaseCmp(s1),0);
}

TEST_F(StringTest, strcasecmpEqual) {
	ppl6::CString s1("ABcdef");
	ppl6::CString s2("ABcdef");
	ASSERT_EQ(s2.StrCaseCmp(s1),0);
}

TEST_F(StringTest, left) {
	ppl6::CString s1("The quick brown fox jumps over the lazy dog");
	ppl6::CString s2=s1.Left(10);
	ASSERT_EQ(ppl6::CString("The quick "),s2);
}

TEST_F(StringTest, right) {
	ppl6::CString s1("The quick brown fox jumps over the lazy dog");
	ppl6::CString s2=s1.Right(9);
	ASSERT_EQ(ppl6::CString(" lazy dog"),s2);
}

TEST_F(StringTest, midWithLength) {
	ppl6::CString s1("The quick brown fox jumps over the lazy dog");
	ppl6::CString s2=s1.Mid(10,10);
	ASSERT_EQ(ppl6::CString("brown fox "),s2);
}

TEST_F(StringTest, midWithoutLength) {
	ppl6::CString s1("The quick brown fox jumps over the lazy dog");
	ppl6::CString s2=s1.Mid(10);
	ASSERT_EQ(ppl6::CString("brown fox jumps over the lazy dog"),s2);
}


TEST_F(StringTest, substrWithLength) {
	ppl6::CString s1("The quick brown fox jumps over the lazy dog");
	ppl6::CString s2=s1.SubStr(10,10);
	ASSERT_EQ(ppl6::CString("brown fox "),s2);
}

TEST_F(StringTest, substrWithoutLength) {
	ppl6::CString s1("The quick brown fox jumps over the lazy dog");
	ppl6::CString s2=s1.SubStr(10);
	ASSERT_EQ(ppl6::CString("brown fox jumps over the lazy dog"),s2);
}

TEST_F(StringTest, lowerCase) {
	ppl6::CString s1("The Quick Brown Fox Jumps over ÄÖÜ");
	ppl6::CString expected("the quick brown fox jumps over äöü");
	ASSERT_NO_THROW(s1.LCase());
	ASSERT_EQ(expected,s1);
}

TEST_F(StringTest, upperCase) {
	ppl6::CString s1("The Quick Brown Fox Jumps over äöü");
	ppl6::CString expected("THE QUICK BROWN FOX JUMPS OVER ÄÖÜ");
	ASSERT_NO_THROW(s1.UCase());
	ASSERT_EQ(expected,s1);
}

TEST_F(StringTest, upperCaseWords) {
	ppl6::CString s1("the quick brown fox jumps over äöü");
	ppl6::CString expected("The Quick Brown Fox Jumps Over Äöü");
	ASSERT_NO_THROW(s1.UCWords());
	ASSERT_EQ(expected,s1);
}

TEST_F(StringTest, pregMatchPositive) {
	ppl6::CString s1("Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
	ppl6::CString expr("/^Lorem.*$/s");
	ASSERT_TRUE(s1.PregMatch(expr));
	expr.Set("/^Lorem.*$/is");
	ASSERT_TRUE(s1.PregMatch(expr));
	expr.Set("/consectetuer/");
	ASSERT_TRUE(s1.PregMatch(expr));
	expr.Set("/^.*consectetuer.*$/s");
	ASSERT_TRUE(s1.PregMatch(expr));
	expr.Set("/^.*mus\\.$/m");
	ASSERT_TRUE(s1.PregMatch(expr));
}


TEST_F(StringTest, pregMatchNegativ) {
	ppl6::CString s1("Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
	ppl6::CString expr("/^Looorem.*$/s");
	ASSERT_FALSE(s1.PregMatch(expr));
	expr.Set("/^ipsum.*$/is");
	ASSERT_FALSE(s1.PregMatch(expr));
	expr.Set("/patrick/");
	ASSERT_FALSE(s1.PregMatch(expr));
	expr.Set("/^.*patrick.*$/s");
	ASSERT_FALSE(s1.PregMatch(expr));
	expr.Set("/^.*mus\\.$/");
	ASSERT_FALSE(s1.PregMatch(expr));
}

TEST_F(StringTest, pregCapture) {
	ppl6::CArray m;
	ppl6::CString s1("2012-05-18");
	ASSERT_TRUE(s1.PregMatch("/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})$/",m));
	ASSERT_EQ(2012,m.GetString(1).ToInt()) << "Unexpected value in capture";
	ASSERT_EQ(5,m.GetString(2).ToInt()) << "Unexpected value in capture";
	ASSERT_EQ(18,m.GetString(3).ToInt()) << "Unexpected value in capture";
	ASSERT_EQ((size_t)4,m.Num()) << "Unexpected number auf captures";

}

TEST_F(StringTest, pregReplace) {
	ppl6::CString s1("Lorem ipsum dolor sit amet.");
	ppl6::CString s2("Lor3m ipsum dolor sit am3t.");
	s1.PregReplace("/e/","3");
	ASSERT_EQ(s2,s1) << "Unexpected result from pregReplace";
}

TEST_F(StringTest, toBool) {
	ppl6::CString s1("A test string with unicode characters: äöü");
	ASSERT_EQ(s1.ToBool(),false) << "String should not be true";
	s1.Set("");
	ASSERT_EQ(s1.ToBool(),false) << "String should not be true";
	s1.Set("12345abcd");
	ASSERT_EQ(s1.ToBool(),true) << "String should be true";
	s1.Set("1");
	ASSERT_EQ(s1.ToBool(),true) << "String should be true";
	s1.Set("12345");
	ASSERT_EQ(s1.ToBool(),true) << "String should be true";
	s1.Set("true");
	ASSERT_EQ(s1.ToBool(),true) << "String should be true";
	s1.Set("wahr");
	ASSERT_EQ(s1.ToBool(),true) << "String should be true";
	s1.Set("ja");
	ASSERT_EQ(s1.ToBool(),true) << "String should be true";
	s1.Set("yes");
	ASSERT_EQ(s1.ToBool(),true) << "String should be true";
	s1.Set("false");
	ASSERT_EQ(s1.ToBool(),false) << "String should not be true";
	s1.Set("falsch");
	ASSERT_EQ(s1.ToBool(),false) << "String should not be true";
	s1.Set("nein");
	ASSERT_EQ(s1.ToBool(),false) << "String should not be true";
	s1.Set("no");
	ASSERT_EQ(s1.ToBool(),false) << "String should not be true";
}


/* TODO
		CString(const CBinary &bin);

		~CString();

		void ImportBuffer(char *buffer, size_t bytes);
		static void setInitialBuffersize(ppluint32 size);
		void Print(bool attach_newline=false) const;
		void HexDump() const;
		void HexDump(void *buffer, ppldd bytes, bool skipheader=false);


		void Add(const char *str, int bytes=-1);
		const char *GetPtr() const;
		char Get(int pos) const;

		void StripSlashes();
		void Trim();
		void LTrim();
		void RTrim();
		void Trim(const char *str);
		void LTrim(const char *str);
		void RTrim(const char *str);
		void Chomp();
		void Chop(int chars=1);
		void LCase();
		void UCase();
		void UCWords();
		void TrimL(ppluint32 chars);
		void TrimR(ppluint32 chars);
		void Cut(int position);
		void Cut(const char *letter);
		void Shr(char c, int size);
		void Shl(char c, int size);
		int StrCmp(const char *str, int size=0) const;
		int StrCmp(const CString &str, int size=0) const;
		int StrCaseCmp(const char *str, int size=0) const;
		int StrCaseCmp(const CString &str, int size=0) const;
		int Instr(const char *str, int pos=0) const;
		int Instr(const CString &str, int pos=0) const;
		int InstrCase(const CString &str, int pos=0) const;

		CString& Repeat(int num);
		CString& Repeat(const char *str, int num);
		CString& Repeat(char ascii, int num);
		CString& Repeat(const CString& str, int num);

		int ToInt() const;
		pplint64 ToInt64() const;
		bool ToBool() const;
		long ToLong() const;
		long long ToLongLong() const;
		float ToFloat() const;
		double ToDouble() const;
		CString& Replace(const char* str, const char* byStr);
		int Find(const char* str, int pos) const;

		int PregMatch(const char *expression, CArray *res=NULL);
		int PregMatch(const CString &expression, CArray &res) const;
		const char *GetMatch(int index) const;
		int PregReplace(const char *expression, const char *replace, int maxreplace=0);
		int PregReplace(const char *expression, CString &replace, int maxreplace=0);
		void PregEscape();

		CString Left(ppluint32 len) const;
		CString Right(ppluint32 len) const;
		CString Mid(ppluint32 start, ppluint32 len = 0xffffffff) const;
		CString SubStr(ppluint32 start, ppluint32 len = 0xffffffff) const;
		CString GetMD5() const;
		int	MD5(const CString &str);
		int	MD5(const char *str);
		int	MD5(const char *buffer, int size);
		int	MD5();
		int	Transcode(const char *fromcharset, const char *tocharset="UTF-8");
		int Transcode(const char *fromcharset, const char *tocharset, CBinary &Target) const;
		int	TranscodeText(const CString &text, const char *fromcharset, const char *tocharset="UTF-8");
		int	TranscodeText(const char *text, const char *fromcharset, const char *tocharset="UTF-8");
		int	TranscodeText(const char *text, int size, const char *fromcharset, const char *tocharset="UTF-8");

		operator const char *() const;
		operator int() const;
		operator bool() const;
		operator unsigned int() const;

		char operator[](int pos) const;
		CString& operator=(char* str);
		CString& operator=(const char* str);
		CString& operator=(const wchar_t* str);
		CString& operator=(const CString *str);
		CString& operator=(const std::string &str);
		CString& operator=(const CString& str);
		CString& operator=(const CWString& str);
		CString& operator+=(const char* str);
		CString& operator+=(int c);
		CString& operator+=(const CString& str);
		CString& operator=(const CBinary& str);

		bool operator<(const char* str) const;
		bool operator<=(const char* str) const;
		bool operator==(const char* str) const;
		bool operator!=(const char* str) const;
		bool operator>=(const char* str) const;
		bool operator>(const char* str) const;

		bool operator<(const CString &str) const;
		bool operator<=(const CString &str) const;
		bool operator==(const CString &str) const;
		bool operator!=(const CString &str) const;
		bool operator>=(const CString &str) const;
		bool operator>(const CString &str) const;


#ifdef WITH_QT
		CString(const QString &q) {
			Init();
			QByteArray a=q.toUtf8();
			Set((const char*)a);
		}
		CString(QString *q) {
			Init();
			QByteArray a=q->toUtf8();
			Set((const char*)a);
		}
		operator const QString() const {
			QString q=GetPtr();
			return q;
		}

		operator const QVariant() const {
			QString q=GetPtr();
			QVariant v=q;
			return v;
		}

		CString& operator=(const QString& q) {
			QByteArray a=q.toUtf8();
			Set((const char*)a);
			return *this;
		}
		CString& operator=(const QString *q) {
			QByteArray a=q->toUtf8();
			Set((const char*)a);
			return *this;
		}
		CString& operator=(QString& q) {
			QByteArray a=q.toUtf8();
			Set((const char*)a);
			return *this;
		}
		CString& operator=(QString *q) {
			QByteArray a=q->toUtf8();
			Set((const char*)a);
			return *this;
		}

#endif


//		int operator==(const char *str);

};
CString operator+(const CString &str1, const CString& str2);
CString operator+(const char *str1, const CString& str2);
CString operator+(const CString &str1, const char *str2);
 *
 */

}


