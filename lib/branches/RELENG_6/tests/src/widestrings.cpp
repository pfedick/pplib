/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
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
#include "ppl6.h"
#include <gtest/gtest.h>
#include "ppl6-tests.h"
#include <map>
#include <stdexcept>

namespace {

// The fixture for testing class Foo.
class WideStringTest : public ::testing::Test {
	protected:
	WideStringTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl6::SetGlobalEncoding("UTF-8");
	}
	virtual ~WideStringTest() {

	}
};

TEST_F(WideStringTest, ConstructorSimple) {
	ASSERT_NO_THROW({
			ppl6::CWString s1;
			ASSERT_TRUE(s1.Len()==0) << "String does not have zero length";
			const wchar_t *buf=s1.GetWPtr();
			ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
			ASSERT_EQ(0,buf[0]) << "Class did not return an empty c-string";

	}
	);
}


TEST_F(WideStringTest, ConstructorFromWideCharPtr) {
	ASSERT_NO_THROW({
			ppl6::CWString s1(L"A test string with unicode characters: äöü");
			ASSERT_EQ((size_t)42,s1.Len()) << "String does not have length of 42";
			const unsigned char *buf=(const unsigned char *)s1.GetPtr();
			ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
			ASSERT_EQ(L'A',(wchar_t)s1[0]) << "Unexpected Character in string";
			ASSERT_EQ(L'ü',(wchar_t)s1[41]) << "Unexpected Character in string";

	}
	);
}

TEST_F(WideStringTest, ConstructorFromStringPtr) {
	ASSERT_NO_THROW({
		ppl6::CString s1("A test string with unicode characters: äöü");
		ppl6::CWString s2(&s1);
		ASSERT_EQ((size_t)42,s2.Len()) << "String does not have length of 45";
		const wchar_t *buf=s2.GetWPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ(L'A',(wchar_t)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(L'ü',(wchar_t)s2[41]) << "Unexpected Character in string";
	}
	);
}



TEST_F(WideStringTest, ConstructorFromString) {
	ASSERT_NO_THROW({
		ppl6::CString s1("A test string with unicode characters: äöü");
		ppl6::CWString s2(s1);
		ASSERT_EQ((size_t)42,s2.Len()) << "String does not have length of 45";
		const wchar_t *buf=s2.GetWPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ(L'A',(wchar_t)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(L'ü',(wchar_t)s2[41]) << "Unexpected Character in string";
	});
}

TEST_F(WideStringTest, ConstructorFromWideString) {
	ASSERT_NO_THROW({
		ppl6::CWString s1(L"A test string with unicode characters: äöü");
		ppl6::CWString s2(s1);
		ASSERT_EQ((size_t)42,s2.Len()) << "String does not have expected length";
		const wchar_t *buf=s2.GetWPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ(L'A',(wchar_t)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(L'ü',(wchar_t)s2[41]) << "Unexpected Character in string";
	});
}

TEST_F(WideStringTest, ConstructorFromWideStringPtr) {
	ASSERT_NO_THROW({
		ppl6::CWString s1(L"A test string with unicode characters: äöü");
		ppl6::CWString s2(&s1);
		ASSERT_EQ((size_t)42,s2.Len()) << "String does not have expected length";
		const wchar_t *buf=s2.GetWPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ(L'A',(wchar_t)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(L'ü',(wchar_t)s2[41]) << "Unexpected Character in string";
	});
}

TEST_F(WideStringTest, ConstructorFromStdString) {
	ASSERT_NO_THROW({
		std::string s1("A test string with unicode characters: äöü");
		ppl6::CWString s2(s1);
		ASSERT_EQ((size_t)42,s2.Len()) << "String does not have expected length";
		const wchar_t *buf=s2.GetWPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ(L'A',(wchar_t)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(L'ü',(wchar_t)s2[41]) << "Unexpected Character in string";
	});
}

TEST_F(WideStringTest, ConstructorFromStdWString) {
	ASSERT_NO_THROW({
		std::wstring s1(L"A test string with unicode characters: äöü");
		ASSERT_EQ((size_t)42,s1.size()) << "std:wstring has unexpected size";
		ppl6::CWString s2(s1);
		ASSERT_EQ((size_t)42,s2.Len()) << "String does not have expected length";
		const wchar_t *buf=s2.GetWPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ(L'A',(wchar_t)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ((wchar_t)L'ü',(wchar_t)s2[41]) << "Unexpected Character in string";
	});
}

TEST_F(WideStringTest, clear) {
	ppl6::CWString s1(L"A test string with unicode characters: äöü");
	s1.Clear();
	ASSERT_EQ((size_t)0,s1.Len()) << "String does not have length of 0";
	const wchar_t *buf=s1.GetWPtr();
	ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
	ASSERT_NO_THROW({
			ASSERT_EQ((wchar_t)0,(wchar_t)s1[0]) << "Unexpected Character in string";
	});
}



TEST_F(WideStringTest, CapacityAndReserve) {
	ppl6::CWString s1;
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

TEST_F(WideStringTest, len) {
	ppl6::CWString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.Len(),(size_t)42) << "String has unexpected size";
}

TEST_F(WideStringTest, length) {
	ppl6::CWString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.Length(),(size_t)42) << "String has unexpected size";
}

TEST_F(WideStringTest, size) {
	ppl6::CWString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.Size(),(size_t)42*sizeof(wchar_t)) << "String has unexpected size";
}

TEST_F(WideStringTest, isEmpty) {
	ppl6::CWString s1(L"A test string with unicode characters: äöü");
	ppl6::CWString s2;
	ASSERT_EQ(s1.IsEmpty(),false) << "String is empty";
	ASSERT_EQ(s2.IsEmpty(),true) << "String not empty";
}

TEST_F(WideStringTest, notEmpty) {
	ppl6::CWString s1(L"A test string with unicode characters: äöü");
	ppl6::CWString s2;
	ASSERT_EQ(s1.NotEmpty(),true) << "String is not empty";
	ASSERT_EQ(s2.NotEmpty(),false) << "String is empty";
}

TEST_F(WideStringTest, IsNumeric) {
	ppl6::CWString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.IsNumeric(),false) << "String should not be numeric";
	s1.Set(L"");
	ASSERT_EQ(s1.IsNumeric(),false) << "String should not be numeric";
	s1.Set(L"12345abcd");
	ASSERT_EQ(s1.IsNumeric(),false) << "String should not be numeric";
	s1.Set(L"12345");
	ASSERT_EQ(s1.IsNumeric(),true) << "String should be numeric";
	s1.Set(L"123.451,12");
	ASSERT_EQ(s1.IsNumeric(),true) << "String should be numeric";
	s1.Set(L"-123.451,12");
	ASSERT_EQ(s1.IsNumeric(),true) << "String should be numeric";
	s1.Set(L"-123.451,12-9");
	ASSERT_EQ(s1.IsNumeric(),false) << "String should not be numeric";
}

TEST_F(WideStringTest, IsInteger) {
	ppl6::CWString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.IsInteger(),false) << "String should not be an integer";
	s1.Set(L"");
	ASSERT_EQ(s1.IsInteger(),false) << "String should not be an integer";
	s1.Set(L"12345abcd");
	ASSERT_EQ(s1.IsInteger(),false) << "String should not be an integer";
	s1.Set(L"12345");
	ASSERT_EQ(s1.IsInteger(),true) << "String should be an integer";
	s1.Set(L"-12345");
	ASSERT_EQ(s1.IsInteger(),true) << "String should be an integer";
	s1.Set(L"123-451");
	ASSERT_EQ(s1.IsInteger(),false) << "String should be an integer";
	s1.Set(L"123.451,12");
	ASSERT_EQ(s1.IsInteger(),false) << "String should be an integer";
	s1.Set(L"-123.451,12");
	ASSERT_EQ(s1.IsInteger(),false) << "String should be an integer";
	s1.Set(L"-123.451,12-9");
	ASSERT_EQ(s1.IsInteger(),false) << "String should not be an integer";
}

TEST_F(WideStringTest, setConstCharWithoutSize) {
	ppl6::CWString s2(L"äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1.Set("äöü, a test string with unicode characters");
	ASSERT_EQ(s1,s2) << "String has unexpected value";
	ASSERT_EQ((size_t)42,s1.Length()) << "String has unexpected length";
}


TEST_F(WideStringTest, setConstCharWithSize) {
	ppl6::CWString s2(L"äöü, a tes");
	ppl6::CWString s1;
	s1.Set("äöü, a test string with unicode characters",10);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)10,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, setConstWChartWithoutSize) {
	ppl6::CWString s2(L"äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1.Set(L"äöü, a test string with unicode characters");
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, setConstWChartWithSize) {
	ppl6::CWString s2(L"äöü, a tes");
	ppl6::CWString s1;
	s1.Set(L"äöü, a test string with unicode characters",10);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)10,s1.Length()) << "String has unexpected length";
}


TEST_F(WideStringTest, setStringPtrWithoutSize) {
	ppl6::CWString s2(L"äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1.Set(&s2);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, setStringPtrWithSize) {
	ppl6::CWString s2(L"äöü, a tes");
	ppl6::CWString s3(L"äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1.Set(&s3,10);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)10,s1.Length()) << "String has unexpected length";
}


TEST_F(WideStringTest, setStringRefWithoutSize) {
	ppl6::CWString s2(L"äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1.Set(s2);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, setStringRefWithSize) {
	ppl6::CWString s2(L"äöü, a tes");
	ppl6::CWString s3(L"äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1.Set(s3,10);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)10,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, setSTDStringRefWithoutSize) {
	ppl6::CWString s2(L"äöü, a test string with unicode characters");
	std::string s3("äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1.Set(s3);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.Length()) << "String has unexpected length";
}


TEST_F(WideStringTest, setSTDStringRefWithSize) {
	ppl6::CWString s2(L"äöü, a tes");
	std::string s3("äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1.Set(s3,10);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)10,s1.Length()) << "String has unexpected length";
}


TEST_F(WideStringTest, setSTDWStringRefWithoutSize) {
	ppl6::CWString s2(L"äöü, a test string with unicode characters");
	std::wstring s3(L"äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1.Set(s3);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, setSTDWStringRefWithSize) {
	ppl6::CWString s2(L"äöü, a tes");
	std::wstring s3(L"äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1.Set(s3,10);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)10,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, setf) {
	ppl6::CWString s2(L"Ein Test, 42, Wide, 10000");
	ppl6::CWString s1;
	s1.Setf("Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)25,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, setChar) {
	ppl6::CWString s2(L"A");
	ppl6::CWString s1;
	s1.SetChar(L'A');
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)1,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, setCharPos) {
	ppl6::CWString expected(L"Ahe Quick Arown Fox Jumps over the lazy doA");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ASSERT_EQ(1,s1.SetChar(L'A',-1));
	ASSERT_EQ(0,s1.SetChar(L'A',73));
	ASSERT_EQ(1,s1.SetChar(L'A',0));
	ASSERT_EQ(1,s1.SetChar(L'A',10));

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)43,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, strcpy) {
	ppl6::CWString expected(L"who let the dogs out?");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ASSERT_EQ(1,s1.Strcpy("who let the dogs out?"));
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, strncpy) {
	ppl6::CWString expected(L"who ");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ASSERT_EQ(1,s1.Strncpy("who let the dogs out?",4));
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)4,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, copy_charptr_without_size) {
	ppl6::CWString expected(L"who let the dogs out?");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ASSERT_EQ(1,s1.Copy("who let the dogs out?"));
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, copy_charptr_with_size) {
	ppl6::CWString expected(L"who ");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ASSERT_EQ(1,s1.Copy("who let the dogs out?",4));
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)4,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, copy_CString_without_size) {
	ppl6::CWString expected(L"who let the dogs out?");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CString s2("who let the dogs out?");
	ASSERT_EQ(1,s1.Copy(s2));
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, copy_CString_with_size) {
	ppl6::CWString expected(L"who ");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CString s2("who let the dogs out?");
	ASSERT_EQ(1,s1.Copy(s2,4));
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)4,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, Add) {
	ppl6::CWString expected(L"The Quick Brown Fox let the dogs out!");
	ppl6::CWString s1(L"The Quick Brown Fox ");
	ASSERT_EQ(1,s1.Add("let the dogs out!"));
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, AddChar) {
	ppl6::CWString expected(L"The Quick Brown Fox a");
	ppl6::CWString s1(L"The Quick Brown Fox ");
	ASSERT_EQ(1,s1.AddChar(L'a'));
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, InsertChar) {
	ppl6::CWString expected(L"The Quick Browny Foxy");
	ppl6::CWString s1(L"The Quick Brown Fox");
	ASSERT_EQ(1,s1.InsertChar(L'y',15));
	ASSERT_EQ(0,s1.InsertChar(L'y',30));
	ASSERT_EQ(1,s1.InsertChar(L'y',20));
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, Delete) {
	ppl6::CWString expected(L" Quick Bro Fo");
	ppl6::CWString s1(L"The Quick Brown Fox");
	EXPECT_EQ(1,s1.Delete(0,3));
	EXPECT_EQ(1,s1.Delete(15,1));
	EXPECT_EQ(0,s1.Delete(20,10));
	EXPECT_EQ(1,s1.Delete(10,2));
	EXPECT_EQ(expected,s1) << "String has unexpected value";
	EXPECT_EQ((size_t)13,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, setWchart) {
	ppl6::CWString s2(L"a");
	ppl6::CWString s1;
	s1.Set((wchar_t)'a');
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)1,s1.Length()) << "String has unexpected length";
}

static void test_vasprintf(ppl6::CWString &str,const char *fmt,...)
{
	va_list args;
	va_start(args, fmt);
	str.Vasprintf(fmt,args);
	va_end(args);
}


TEST_F(WideStringTest, vasprintf) {
	ppl6::CWString s2(L"Ein Test, 42, Wide, 10000");
	ppl6::CWString s1;
	test_vasprintf(s1,"Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)25,s1.Length()) << "String has unexpected length";
}


TEST_F(WideStringTest, appendConstWchartWithoutSize) {
	ppl6::CWString expected(L"First Part äöü, äöü Second Part");
	ppl6::CWString s1(L"First Part äöü, ");
	s1.Concat(L"äöü Second Part");

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendConstWchartWithSize) {
	ppl6::CWString expected(L"First Part äöü, äöü S");
	ppl6::CWString s1(L"First Part äöü, ");
	s1.Concat(L"äöü Second Part",5);

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendConstCharPtrWithoutSize) {
	ppl6::CWString expected(L"First Part äöü, äöü Second Part");
	ppl6::CWString s1(L"First Part äöü, ");
	s1.Concat(L"äöü Second Part");

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendConstCharPtrWithSize) {
	ppl6::CWString expected(L"First Part äöü, äöü Seco");
	ppl6::CWString s1(L"First Part äöü, ");
	s1.Concat("äöü Second Part",8);

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)24,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStringPtrWithoutSize) {
	ppl6::CWString expected(L"First Part äöü, äöü Second Part");
	ppl6::CWString s1(L"First Part äöü, ");
	ppl6::CWString s2(L"äöü Second Part");
	s1.Concat(&s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStringPtrWithSize) {
	ppl6::CWString expected(L"First Part äöü, äöü Seco");
	ppl6::CWString s1(L"First Part äöü, ");
	ppl6::CWString s2(L"äöü Second Part");
	s1.Concat(&s2,8);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)24,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStringWithoutSize) {
	ppl6::CWString expected(L"First Part äöü, äöü Second Part");
	ppl6::CWString s1(L"First Part äöü, ");
	ppl6::CWString s2(L"äöü Second Part");
	s1.Concat(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStringWithSize) {
	ppl6::CWString expected(L"First Part äöü, äöü Seco");
	ppl6::CWString s1(L"First Part äöü, ");
	ppl6::CWString s2(L"äöü Second Part");
	s1.Concat(s2,8);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)24,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStdStringWithoutSize) {
	ppl6::CWString expected(L"First Part äöü, äöü Second Part");
	ppl6::CWString s1(L"First Part äöü, ");
	std::string s2("äöü Second Part");
	s1.Concat(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStdStringWithSize) {
	ppl6::CWString expected(L"First Part äöü, äöü S");
	ppl6::CWString s1(L"First Part äöü, ");
	std::string s2("äöü Second Part");
	s1.Concat(s2,5);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStdWStringWithoutSize) {
	ppl6::CWString expected(L"First Part äöü, äöü Second Part");
	ppl6::CWString s1(L"First Part äöü, ");
	std::wstring s2(L"äöü Second Part");
	s1.Concat(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStdWStringWithSize) {
	ppl6::CWString expected(L"First Part äöü, äöü S");
	ppl6::CWString s1(L"First Part äöü, ");
	std::wstring s2(L"äöü Second Part");
	s1.Concat(s2,5);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendf) {
	ppl6::CWString expected(L"First Part äöü, Ein Test, 42, Wide, 10000");
	ppl6::CWString s1(L"First Part äöü, ");
	s1.Concatf("Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)41,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendWchart) {
	ppl6::CWString expected(L"First Part äöü, a");
	ppl6::CWString s1(L"First Part äöü, ");
	s1.Concat((wchar_t)'a');
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)17,s1.Length()) << "String has unexpected length";
}

#ifdef TODO
TEST_F(WideStringTest, chopRight) {
	ppl6::CWString s1(L"A test string with unicode characters: äöü");
	ppl6::CWString s2(L"A test string with unicode characters: ä");
	ppl6::CWString s3(L"A test string with unicode characters:");
	s1.ChopRight(2);
	ASSERT_EQ((size_t)40,s1.Len()) << "String does not have expected length";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	s1.ChopRight();
	s1.ChopRight();
	ASSERT_EQ((size_t)38,s1.Len()) << "String does not have expected length";
	ASSERT_EQ(s3,s1) << "String has unexpected value";
	s1.ChopRight(37);
	ASSERT_EQ((size_t)1,s1.Len()) << "String does not have length of 1";
	s1.ChopRight(1);
	ASSERT_EQ((size_t)0,s1.Len()) << "String does not have length of 0";
	s2.clear();
	ASSERT_EQ(s2,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, chopLeft) {
	ppl6::CWString s1(L"A test string with unicode characters: äöü");
	ppl6::CWString s2(L"est string with unicode characters: äöü");
	ppl6::CWString s3(L"st string with unicode characters: äöü");
	s1.ChopLeft(3);
	ASSERT_EQ((size_t)39,s1.Len()) << "String does not have expected length";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	s1.ChopLeft();
	ASSERT_EQ((size_t)38,s1.Len()) << "String does not have expected length";
	ASSERT_EQ(s3,s1) << "String has unexpected value";
	s1.ChopRight(37);
	ASSERT_EQ((size_t)1,s1.Len()) << "String does not have length of 1";
	s1.ChopRight(1);
	ASSERT_EQ((size_t)0,s1.Len()) << "String does not have length of 0";
	s2.clear();
	ASSERT_EQ(s2,s1) << "String has unexpected value";
}
#endif

TEST_F(WideStringTest, chop) {
	ppl6::CWString s1(L"A test string with unicode characters: äöü");
	ppl6::CWString s2(L"A test string with unicode characters: ä");
	ppl6::CWString s3(L"A test string with unicode characters:");
	s1.Chop(2);
	ASSERT_EQ((size_t)40,s1.Len()) << "String does not have length of 40";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	s1.Chop();
	s1.Chop();
	ASSERT_EQ((size_t)38,s1.Len()) << "String does not have length of 38";
	ASSERT_EQ(s3,s1) << "String has unexpected value";
	s1.Chop(37);
	ASSERT_EQ((size_t)1,s1.Len()) << "String does not have length of 1";
	s1.Chop();
	ASSERT_EQ((size_t)0,s1.Len()) << "String does not have length of 0";
	s2.Clear();
	ASSERT_EQ(s2,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, chomp) {
	ppl6::CWString s1(L"\n\r\nA test string\näöü\n\r\n");
	ppl6::CWString s2(L"A test string\näöü");
	s1.Chomp();
	ASSERT_EQ((size_t)17,s1.Len()) << "String does not have length of 17";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	s1.Chomp();
	ASSERT_EQ((size_t)17,s1.Len()) << "String does not have length of 17";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, cut_WithPos) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString expected(L"The quick ");
	s1.Cut(10);
	ASSERT_EQ((size_t)10,s1.Len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, cut_WithPos0) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString expected(L"");
	s1.Cut(0);
	ASSERT_EQ((size_t)0,s1.Len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, cut_WithPosBeyondStringLength) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString expected(L"The quick brown fox jumps over the lazy dog");
	s1.Cut(100);
	ASSERT_EQ((size_t)43,s1.Len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}


TEST_F(WideStringTest, trimLeft) {
	ppl6::CWString s1(L"\n\n    abc  \n");
	s1.LTrim();
	ASSERT_EQ((size_t)6,s1.Len());
	ASSERT_EQ(ppl6::CWString(L"abc  \n"),s1);
}

TEST_F(WideStringTest, trimLeftGrenzbereich) {
	ppl6::CWString s1;
	s1.Reserve(8);
	ASSERT_EQ(8,s1.Capacity());
	s1=L"  abcde";
	ASSERT_EQ(8,s1.Capacity());
	ASSERT_EQ(7,s1.Len());
	s1.LTrim();
	ASSERT_EQ(8,s1.Capacity());
	ASSERT_EQ(5,s1.Len());
	ASSERT_EQ(ppl6::CWString(L"abcde"),s1);
}

TEST_F(WideStringTest, trimRight) {
	ppl6::CWString s1(L" \n  abc  \n");
	s1.RTrim();
	ASSERT_EQ((size_t)7,s1.Len());
	ASSERT_EQ(ppl6::CWString(L" \n  abc"),s1);
}

TEST_F(WideStringTest, trim) {
	ppl6::CWString s1(L"\n\n    abc  \n");
	s1.Trim();
	ASSERT_EQ((size_t)3,s1.Len());
	ASSERT_EQ(ppl6::CWString(L"abc"),s1);
}

TEST_F(WideStringTest, trimGrenzbereich) {
	ppl6::CWString s1;
	s1.Reserve(8);
	ASSERT_EQ(8,s1.Capacity());
	s1=L"  abc  ";
	ASSERT_EQ(8,s1.Capacity());
	ASSERT_EQ(7,s1.Len());
	s1.Trim();
	ASSERT_EQ(8,s1.Capacity());
	ASSERT_EQ(3,s1.Len());
	ASSERT_EQ(ppl6::CWString(L"abc"),s1);
}

TEST_F(WideStringTest, trimLeftEmptyResult) {
	ppl6::CWString s1(L"\n\n   \n   \n");
	s1.LTrim();
	ASSERT_EQ((size_t)0,s1.Len());
	ASSERT_EQ(ppl6::CWString(L""),s1);
}

TEST_F(WideStringTest, trimRightEmptyResult) {
	ppl6::CWString s1(L"\n\n   \n   \n");
	s1.RTrim();
	ASSERT_EQ((size_t)0,s1.Len());
	ASSERT_EQ(ppl6::CWString(L""),s1);
}

TEST_F(WideStringTest, trimEmptyResult) {
	ppl6::CWString s1(L"\n\n   \n   \n");
	s1.Trim();
	ASSERT_EQ((size_t)0,s1.Len());
	ASSERT_EQ(ppl6::CWString(L""),s1);
}

TEST_F(WideStringTest, trimLeftChars) {
	ppl6::CWString s1(L"\n\n    abc  \n");
	s1.LTrim(" \n");
	ASSERT_EQ((size_t)6,s1.Len());
	ASSERT_EQ(ppl6::CWString(L"abc  \n"),s1);
}

TEST_F(WideStringTest, trimLeftCharsGrenzbereich) {
	ppl6::CWString s1;
	s1.Reserve(8);
	ASSERT_EQ(8,s1.Capacity());
	s1=L"  abcde";
	ASSERT_EQ(8,s1.Capacity());
	ASSERT_EQ(7,s1.Len());
	s1.LTrim(" ");
	ASSERT_EQ(8,s1.Capacity());
	ASSERT_EQ(5,s1.Len());
	ASSERT_EQ(ppl6::CWString(L"abcde"),s1);
}


TEST_F(WideStringTest, trimRightChars) {
	ppl6::CWString s1(L" \n  abc  \n");
	s1.RTrim(" \n");
	ASSERT_EQ((size_t)7,s1.Len());
	ASSERT_EQ(ppl6::CWString(L" \n  abc"),s1);
}

TEST_F(WideStringTest, trimChars) {
	ppl6::CWString s1(L"\n\n    abc  \n");
	s1.Trim(" \n");
	ASSERT_EQ((size_t)3,s1.Len());
	ASSERT_EQ(ppl6::CWString(L"abc"),s1);
}

TEST_F(WideStringTest, trimLeftCharsEmptyResult) {
	ppl6::CWString s1(L"\n\n   \n   \n");
	s1.LTrim(" \n");
	ASSERT_EQ((size_t)0,s1.Len());
	ASSERT_EQ(ppl6::CWString(L""),s1);
}

TEST_F(WideStringTest, trimRightCharsEmptyResult) {
	ppl6::CWString s1(L"\n\n   \n   \n");
	s1.RTrim(" \n");
	ASSERT_EQ((size_t)0,s1.Len());
	ASSERT_EQ(ppl6::CWString(L""),s1);
}

TEST_F(WideStringTest, trimEmptyCharsResult) {
	ppl6::CWString s1(L"\n\n   \n   \n");
	s1.Trim(" \n");
	ASSERT_EQ((size_t)0,s1.Len());
	ASSERT_EQ(ppl6::CWString(L""),s1);
}

TEST_F(WideStringTest, operatorEqualEmpty) {
	ppl6::CWString s1(L"");
	ppl6::CWString s2(L"");
	ASSERT_EQ(s1,s2);
}

TEST_F(WideStringTest, strcmpEmpty) {
	ppl6::CWString s1(L"");
	ppl6::CWString s2(L"");
	ASSERT_EQ(0,s1.StrCmp(s2));
}

TEST_F(WideStringTest, strcmpLower) {
	ppl6::CWString s1(L"ABcdef");
	ppl6::CWString s2(L"Defghi");
	ASSERT_LT(s1.StrCmp(s2),0);
}

TEST_F(WideStringTest, strcmpLowerWithCase) {
	ppl6::CWString s1(L"ABcdef");
	ppl6::CWString s2(L"abcdef");
	ASSERT_LT(s1.StrCmp(s2),0);
}

TEST_F(WideStringTest, strcmpHigher) {
	ppl6::CWString s1(L"ABcdef");
	ppl6::CWString s2(L"Defghi");
	ASSERT_GT(s2.StrCmp(s1),0);
}

TEST_F(WideStringTest, strcmpHigherWithCase) {
	ppl6::CWString s1(L"ABcdef");
	ppl6::CWString s2(L"abcdef");
	ASSERT_GT(s2.StrCmp(s1),0);
}

TEST_F(WideStringTest, strcmpEqual) {
	ppl6::CWString s1(L"ABcdef");
	ppl6::CWString s2(L"ABcdef");
	ASSERT_EQ(s2.StrCmp(s1),0);
}

TEST_F(WideStringTest, strcasecmpEmpty) {
	ppl6::CWString s1(L"");
	ppl6::CWString s2(L"");
	ASSERT_EQ(0,s1.StrCaseCmp(s2));
}

TEST_F(WideStringTest, strcasecmpLower) {
	ppl6::CWString s1(L"ABcdef");
	ppl6::CWString s2(L"Defghi");
	ASSERT_LT(s1.StrCaseCmp(s2),0);
}

TEST_F(WideStringTest, strcasecmpLowerWithCase) {
	ppl6::CWString s1(L"ABcdef");
	ppl6::CWString s2(L"abcdef");
	ASSERT_EQ(s1.StrCaseCmp(s2),0);
}

TEST_F(WideStringTest, strcasecmpHigher) {
	ppl6::CWString s1(L"ABcdef");
	ppl6::CWString s2(L"Defghi");
	ASSERT_GT(s2.StrCaseCmp(s1),0);
}

TEST_F(WideStringTest, strcasecmpHigherWithCase) {
	ppl6::CWString s1(L"ABcdef");
	ppl6::CWString s2(L"abcdef");
	ASSERT_EQ(s2.StrCaseCmp(s1),0);
}

TEST_F(WideStringTest, strcasecmpEqual) {
	ppl6::CWString s1(L"ABcdef");
	ppl6::CWString s2(L"ABcdef");
	ASSERT_EQ(s2.StrCaseCmp(s1),0);
}


TEST_F(WideStringTest, left) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString s2=s1.Left(10);
	ASSERT_EQ(ppl6::CWString(L"The quick "),s2);
}

TEST_F(WideStringTest, right) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString s2=s1.Right(9);
	ASSERT_EQ(ppl6::CWString(L" lazy dog"),s2);
}

TEST_F(WideStringTest, midWithLength) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString s2=s1.Mid(10,10);
	ASSERT_EQ(ppl6::CWString(L"brown fox "),s2);
}

TEST_F(WideStringTest, midWithoutLength) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString s2=s1.Mid(10);
	ASSERT_EQ(ppl6::CWString(L"brown fox jumps over the lazy dog"),s2);
}


TEST_F(WideStringTest, substrWithLength) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString s2=s1.SubStr(10,10);
	ASSERT_EQ(ppl6::CWString(L"brown fox "),s2);
}

TEST_F(WideStringTest, substrWithoutLength) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString s2=s1.SubStr(10);
	ASSERT_EQ(ppl6::CWString(L"brown fox jumps over the lazy dog"),s2);
}

TEST_F(WideStringTest, lowerCase) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over ÄÖÜ");
	ppl6::CWString expected(L"the quick brown fox jumps over äöü");
	ASSERT_NO_THROW(s1.LCase());
	ASSERT_EQ(expected,s1);
}

TEST_F(WideStringTest, upperCase) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over äöü");
	ppl6::CWString expected(L"THE QUICK BROWN FOX JUMPS OVER ÄÖÜ");
	ASSERT_NO_THROW(s1.UCase());
	ASSERT_EQ(expected,s1);
}

TEST_F(WideStringTest, upperCaseWords) {
	ppl6::CWString s1(L"the quick brown fox jumps over äöü");
	ppl6::CWString expected(L"The Quick Brown Fox Jumps Over Äöü");
	ASSERT_NO_THROW(s1.UCWords());
	ASSERT_EQ(expected,s1);
}


TEST_F(WideStringTest, pregMatchPositive) {
	ppl6::CWString s1(L"Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
	ppl6::CWString expr(L"/^Lorem.*$/s");
	ASSERT_TRUE(s1.PregMatch(expr));
	expr.Set(L"/^Lorem.*$/is");
	ASSERT_TRUE(s1.PregMatch(expr));
	expr.Set(L"/consectetuer/");
	ASSERT_TRUE(s1.PregMatch(expr));
	expr.Set(L"/^.*consectetuer.*$/s");
	ASSERT_TRUE(s1.PregMatch(expr));
	expr.Set(L"/^.*mus\\.$/m");
	ASSERT_TRUE(s1.PregMatch(expr));
}

TEST_F(WideStringTest, pregMatchNegativ) {
	ppl6::CWString s1(L"Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
	ppl6::CWString expr(L"/^Looorem.*$/s");
	ASSERT_FALSE(s1.PregMatch(expr));
	expr.Set(L"/^ipsum.*$/is");
	ASSERT_FALSE(s1.PregMatch(expr));
	expr.Set(L"/patrick/");
	ASSERT_FALSE(s1.PregMatch(expr));
	expr.Set(L"/^.*patrick.*$/s");
	ASSERT_FALSE(s1.PregMatch(expr));
	expr.Set(L"/^.*mus\\.$/");
	ASSERT_FALSE(s1.PregMatch(expr));
}

TEST_F(WideStringTest, pregCapture) {
	ppl6::CArray m;
	ppl6::CWString s1(L"2012-05-18");
	ASSERT_TRUE(s1.PregMatch("/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})$/",&m));
	ASSERT_EQ(2012,m.GetString(1).ToInt()) << "Unexpected value in capture";
	ASSERT_EQ(5,m.GetString(2).ToInt()) << "Unexpected value in capture";
	ASSERT_EQ(18,m.GetString(3).ToInt()) << "Unexpected value in capture";
	ASSERT_EQ((size_t)4,m.Num()) << "Unexpected number auf captures";

}

TEST_F(WideStringTest, stdmap_add) {
	std::map<ppl6::CWString,ppl6::CWString> myMap;
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"the",L"1"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"quick",L"2"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"brown",L"3"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"fox",L"4"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"jumps",L"5"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"over",L"6"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"the",L"7"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"lazy",L"8"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"dog",L"9"));

	ASSERT_EQ((size_t)8,myMap.size()) << "Unexpected size of map";
}

TEST_F(WideStringTest, stdmap_search) {
	std::map<ppl6::CWString,ppl6::CWString> myMap;
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"the",L"1"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"quick",L"2"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"brown",L"3"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"fox",L"4"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"jumps",L"5"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"over",L"6"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"the",L"7"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"lazy",L"8"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"dog",L"9"));

	ppl6::CWString s1;

	ASSERT_EQ(ppl6::CWString(L"3"),myMap.at("brown")) << "Unexpected Result";
	ASSERT_EQ(ppl6::CWString(L"8"),myMap.at("lazy")) << "Unexpected Result";
	ASSERT_EQ(ppl6::CWString(L"1"),myMap.at("the")) << "Unexpected Result";
	ASSERT_EQ(ppl6::CWString(L"6"),myMap.at("over")) << "Unexpected Result";

	ASSERT_THROW({
		ASSERT_EQ(ppl6::CWString(L""),myMap.at("blue")) << "Unexpected Result";
	}, std::out_of_range);

}

TEST_F(WideStringTest, stdmap_walk) {
	std::map<ppl6::CWString,ppl6::CWString> myMap;
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"the",L"1"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"quick",L"2"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"brown",L"3"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"fox",L"4"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"jumps",L"5"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"over",L"6"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"the",L"7"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"lazy",L"8"));
	myMap.insert(std::pair<ppl6::CWString,ppl6::CWString>(L"dog",L"9"));

	std::map<ppl6::CWString,ppl6::CWString>::const_iterator it;
	it=myMap.begin();
	ASSERT_EQ(ppl6::CWString(L"brown"),(*it).first) << "Unexpected Result";
	it++;
	ASSERT_EQ(ppl6::CWString(L"dog"),(*it).first) << "Unexpected Result";
	it++;
	ASSERT_EQ(ppl6::CWString(L"fox"),(*it).first) << "Unexpected Result";
	it++;
	ASSERT_EQ(ppl6::CWString(L"jumps"),(*it).first) << "Unexpected Result";
	it++;
	ASSERT_EQ(ppl6::CWString(L"lazy"),(*it).first) << "Unexpected Result";
	it++;
	ASSERT_EQ(ppl6::CWString(L"over"),(*it).first) << "Unexpected Result";
	it++;
	ASSERT_EQ(ppl6::CWString(L"quick"),(*it).first) << "Unexpected Result";
	it++;
	ASSERT_EQ(ppl6::CWString(L"the"),(*it).first) << "Unexpected Result";
	it++;

	ASSERT_EQ(myMap.end(),it) << "Not at end of map";

}

TEST_F(WideStringTest, operator_plus_cwstr_cwstr) {
	ppl6::CWString expected(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString s1(L"The Quick Brown Fox ");
	ppl6::CWString s2(L"Jumps over the lazy dog");
	ppl6::CWString result=s1+s2;
	ASSERT_EQ(expected,result) << "Unexpected Result";
}

TEST_F(WideStringTest, operator_plus_cwstr_wchar) {
	ppl6::CWString expected(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString s1(L"The Quick Brown Fox ");
	ppl6::CWString result=s1+L"Jumps over the lazy dog";
	ASSERT_EQ(expected,result) << "Unexpected Result";
}

TEST_F(WideStringTest, operator_plus_wchar_cwstr) {
	ppl6::CWString expected(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString s2(L"Jumps over the lazy dog");
	ppl6::CWString result=L"The Quick Brown Fox "+s2;
	ASSERT_EQ(expected,result) << "Unexpected Result";
}

TEST_F(WideStringTest, ImportBuffer) {
	wchar_t *buffer=(wchar_t *)malloc(128*sizeof(wchar_t));
	wcscpy(buffer, L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString expected(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString s2(L"Jumps over the lazy dog");
	s2.ImportBuffer(buffer,128*sizeof(wchar_t));
	ASSERT_EQ(expected,s2) << "Unexpected Result";
}

TEST_F(WideStringTest, StripSlahes) {
	ppl6::CWString expected(L"The Quick Brown Fox");
	ppl6::CWString s1(L"The \\Quick\\ Brown \\Fox");
	s1.StripSlashes();
	EXPECT_EQ(expected,s1) << "String has unexpected value";
	EXPECT_EQ((size_t)19,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, TrimL) {
	ppl6::CWString expected(L"Quick Brown Fox");
	ppl6::CWString s1(L"The Quick Brown Fox");
	s1.TrimL(4);
	EXPECT_EQ(expected,s1) << "String has unexpected value";
	EXPECT_EQ((size_t)15,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, TrimR) {
	ppl6::CWString expected(L"The Quick Brown");
	ppl6::CWString s1(L"The Quick Brown Fox");
	s1.TrimR(4);
	EXPECT_EQ(expected,s1) << "String has unexpected value";
	EXPECT_EQ((size_t)15,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, SubStr) {
	ppl6::CWString expected(L"Brown Fox");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString result=s1.SubStr(10,9);
	ASSERT_EQ(expected,result) << "Unexpected Result";
	EXPECT_EQ((size_t)9,result.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, Mid) {
	ppl6::CWString expected(L"Brown Fox");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString result=s1.Mid(10,9);
	ASSERT_EQ(expected,result) << "Unexpected Result";
	EXPECT_EQ((size_t)9,result.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, Mid_LongerThanString) {
	ppl6::CWString expected(L"Jumps over the lazy dog");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString result=s1.Mid(20,100);
	ASSERT_EQ(expected,result) << "Unexpected Result";
	EXPECT_EQ((size_t)23,result.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, Mid_WithoutSize) {
	ppl6::CWString expected(L"Jumps over the lazy dog");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString result=s1.Mid(20);
	ASSERT_EQ(expected,result) << "Unexpected Result";
	EXPECT_EQ((size_t)23,result.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, Left) {
	ppl6::CWString expected(L"The Quick Brown");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString result=s1.Left(15);
	ASSERT_EQ(expected,result) << "Unexpected Result";
	EXPECT_EQ((size_t)15,result.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, Left_LongerThanString) {
	ppl6::CWString expected(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString result=s1.Left(100);
	ASSERT_EQ(expected,result) << "Unexpected Result";
	EXPECT_EQ((size_t)43,result.Length()) << "String has unexpected length";
}


TEST_F(WideStringTest, Right) {
	ppl6::CWString expected(L"lazy dog");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString result=s1.Right(8);
	ASSERT_EQ(expected,result) << "Unexpected Result";
	EXPECT_EQ((size_t)8,result.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, Right_LongerThanString) {
	ppl6::CWString expected(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString result=s1.Right(100);
	ASSERT_EQ(expected,result) << "Unexpected Result";
	EXPECT_EQ((size_t)43,result.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, Instr_wchar_NotFound) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	EXPECT_EQ((int)-1,s1.Instr(L"fish")) << "Unexpected Result";
}

TEST_F(WideStringTest, Instr_wchar_FirstPos) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	EXPECT_EQ((int)0,s1.Instr(L"The")) << "Unexpected Result";
}

TEST_F(WideStringTest, Instr_wchar_Middle) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	EXPECT_EQ((int)20,s1.Instr(L"Jumps")) << "Unexpected Result";
}

TEST_F(WideStringTest, Instr_wchar_WithStartPos) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	EXPECT_EQ((int)32,s1.Instr(L"he",10)) << "Unexpected Result";
}

TEST_F(WideStringTest, Instr_char_NotFound) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	EXPECT_EQ((int)-1,s1.Instr("fish")) << "Unexpected Result";
}

TEST_F(WideStringTest, Instr_char_FirstPos) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	EXPECT_EQ((int)0,s1.Instr("The")) << "Unexpected Result";
}

TEST_F(WideStringTest, Instr_char_Middle) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	EXPECT_EQ((int)20,s1.Instr("Jumps")) << "Unexpected Result";
}

TEST_F(WideStringTest, Instr_char_WithStartPos) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	EXPECT_EQ((int)32,s1.Instr("he",10)) << "Unexpected Result";
}

TEST_F(WideStringTest, Shr) {
	ppl6::CWString expected(L"AAAThe Quick Brown Fox Jumps over the lazy ");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	s1.Shr(L'A',3);
	EXPECT_EQ(expected,s1) << "Unexpected Result";
	EXPECT_EQ((size_t)43,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, Shl) {
	ppl6::CWString expected(L"Quick Brown Fox Jumps over the lazy dogAAAA");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	s1.Shl(L'A',4);
	EXPECT_EQ(expected,s1) << "Unexpected Result";
	EXPECT_EQ((size_t)43,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, Shr_longerThanString) {
	ppl6::CWString expected(L"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	s1.Shr(L'A',50);
	EXPECT_EQ(expected,s1) << "Unexpected Result";
	EXPECT_EQ((size_t)43,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, Shl_longerThanString) {
	ppl6::CWString expected(L"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	s1.Shl(L'A',50);
	EXPECT_EQ(expected,s1) << "Unexpected Result";
	EXPECT_EQ((size_t)43,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, repeat) {
	ppl6::CWString s1(L"_repeat_");
	ppl6::CWString s3(L"_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
	ppl6::CWString s2;
	s2=s1.Repeat(10);
	ASSERT_EQ((size_t)80,s2.Len()) << "String does not have length of 80";
	ASSERT_EQ(s3,s2) << "String has unexpected value";
}

TEST_F(WideStringTest, repeat_charptr) {
	ppl6::CWString s3(L"_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
	ppl6::CWString s2;
	s2.Repeat("_repeat_",10);
	ASSERT_EQ((size_t)80,s2.Len()) << "String does not have length of 80";
	ASSERT_EQ(s3,s2) << "String has unexpected value";
}

TEST_F(WideStringTest, repeat_wcharptr) {
	ppl6::CWString s3(L"_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
	ppl6::CWString s2;
	s2.Repeat(L"_repeat_",10);
	ASSERT_EQ((size_t)80,s2.Len()) << "String does not have length of 80";
	ASSERT_EQ(s3,s2) << "String has unexpected value";
}

TEST_F(WideStringTest, repeat_wchar) {
	ppl6::CWString s3(L"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	ppl6::CWString s2;
	s2.Repeat(L'A',80);
	ASSERT_EQ((size_t)80,s2.Len()) << "String does not have length of 80";
	ASSERT_EQ(s3,s2) << "String has unexpected value";
}

TEST_F(WideStringTest, repeat_CString) {
	ppl6::CString s1("_repeat_");
	ppl6::CWString s3(L"_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
	ppl6::CWString s2;
	s2.Repeat(s1,10);
	ASSERT_EQ((size_t)80,s2.Len()) << "String does not have length of 80";
	ASSERT_EQ(s3,s2) << "String has unexpected value";
}

TEST_F(WideStringTest, repeat_CWString) {
	ppl6::CWString s1(L"_repeat_");
	ppl6::CWString s3(L"_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
	ppl6::CWString s2;
	s2.Repeat(s1,10);
	ASSERT_EQ((size_t)80,s2.Len()) << "String does not have length of 80";
	ASSERT_EQ(s3,s2) << "String has unexpected value";
}

TEST_F(WideStringTest, Find_charptr_notfound) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");

	EXPECT_EQ((int)-1,s1.Find("fish",0)) << "Unexpected Result";
}

TEST_F(WideStringTest, Find_charptr) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	EXPECT_EQ((int)10,s1.Find("Brown",0)) << "Unexpected Result";
}

TEST_F(WideStringTest, Find_CWString) {
	ppl6::CWString needle(L"Brown");
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	EXPECT_EQ((int)10,s1.Find(needle,0)) << "Unexpected Result";
}

TEST_F(WideStringTest, Replace_charstr) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString expected(L"The Quick Green Fox Jumps over the lazy dog");
	ppl6::CWString s2;
	s2=s1.Replace("Brown","Green");
	EXPECT_EQ(s2,s1) << "Unexpected Result";
	EXPECT_EQ(expected,s1) << "Unexpected Result";
}

TEST_F(WideStringTest, Replace_CWString) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString expected(L"The Quick Green Fox Jumps over the lazy dog");
	ppl6::CWString s2;
	ppl6::CWString search(L"Brown");
	ppl6::CWString replace(L"Green");
	s2=s1.Replace(search,replace);
	EXPECT_EQ(s2,s1) << "Unexpected Result";
	EXPECT_EQ(expected,s1) << "Unexpected Result";
}

TEST_F(WideStringTest, Replace_CWString_Multi) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString expected(L"The Quick Brewn Fex Jumps ever the lazy deg");
	ppl6::CWString s2;
	ppl6::CWString search(L"o");
	ppl6::CWString replace(L"e");
	s2=s1.Replace(search,replace);
	EXPECT_EQ(s2,s1) << "Unexpected Result";
	EXPECT_EQ(expected,s1) << "Unexpected Result";
}

TEST_F(WideStringTest, ReplaceLetterList) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	ppl6::CWString expected(L"Thi Qiick Briwn Fix Jimps ivir thi lizy dig");
	ppl6::CWString s2;
	ppl6::CWString letters(L"aeou");
	s2=s1.ReplaceLetterList(letters,L'i');
	EXPECT_EQ(s2,s1) << "Unexpected Result";
	EXPECT_EQ(expected,s1) << "Unexpected Result";
}

TEST_F(WideStringTest, ToInt_withoutNumber) {
	ppl6::CWString s1(L"The Quick Brown Fox Jumps over the lazy dog");
	EXPECT_EQ((int)0,s1.ToInt()) << "Unexpected Result";
}

TEST_F(WideStringTest, ToInt_123456) {
	ppl6::CWString s1(L"123456");
	EXPECT_EQ((int)123456,s1.ToInt()) << "Unexpected Result";
}

TEST_F(WideStringTest, ToInt_minus123456) {
	ppl6::CWString s1(L"-123456");
	EXPECT_EQ((int)-123456,s1.ToInt()) << "Unexpected Result";
}

TEST_F(WideStringTest, ToInt_123456_point_567) {
	ppl6::CWString s1(L"123456.567");
	EXPECT_EQ((int)123456,s1.ToInt()) << "Unexpected Result";
}

TEST_F(WideStringTest, ToUInt_123456) {
	ppl6::CWString s1(L"123456");
	EXPECT_EQ((int)123456,s1.ToUInt()) << "Unexpected Result";
}

TEST_F(WideStringTest, ToUInt_minus123456) {
	ppl6::CWString s1(L"-123456");
	EXPECT_EQ((unsigned int)4294843840,s1.ToUInt()) << "Unexpected Result";
}

TEST_F(WideStringTest, ToInt64_1242346214893456) {
	ppl6::CWString s1(L"1242346214893456");
	EXPECT_EQ((pplint64)1242346214893456,s1.ToInt64()) << "Unexpected Result";
}

TEST_F(WideStringTest, ToInt64_minus1242346214893456) {
	ppl6::CWString s1(L"-1242346214893456");
	EXPECT_EQ((pplint64)-1242346214893456,s1.ToInt64()) << "Unexpected Result";
}

TEST_F(WideStringTest, ToUInt64_1242346214893456) {
	ppl6::CWString s1(L"1242346214893456");
	EXPECT_EQ((ppluint64)1242346214893456,s1.ToUInt64()) << "Unexpected Result";
}

TEST_F(WideStringTest, ToUInt64_minus1242346214893456) {
	ppl6::CWString s1(L"-1242346214893456");
	EXPECT_EQ((ppluint64)-1242346214893456,s1.ToUInt64()) << "Unexpected Result";
}

TEST_F(WideStringTest, ToBool) {
	ppl6::CWString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.ToBool(),false) << "String should not be true";
	s1.Set(L"");
	ASSERT_EQ(s1.ToBool(),false) << "String should not be true";
	s1.Set(L"12345abcd");
	ASSERT_EQ(s1.ToBool(),true) << "String should be true";
	s1.Set(L"1");
	ASSERT_EQ(s1.ToBool(),true) << "String should be true";
	s1.Set(L"12345");
	ASSERT_EQ(s1.ToBool(),true) << "String should be true";
	s1.Set(L"true");
	ASSERT_EQ(s1.ToBool(),true) << "String should be true";
	s1.Set(L"wahr");
	ASSERT_EQ(s1.ToBool(),true) << "String should be true";
	s1.Set(L"ja");
	ASSERT_EQ(s1.ToBool(),true) << "String should be true";
	s1.Set(L"yes");
	ASSERT_EQ(s1.ToBool(),true) << "String should be true";
	s1.Set(L"false");
	ASSERT_EQ(s1.ToBool(),false) << "String should not be true";
	s1.Set(L"falsch");
	ASSERT_EQ(s1.ToBool(),false) << "String should not be true";
	s1.Set(L"nein");
	ASSERT_EQ(s1.ToBool(),false) << "String should not be true";
	s1.Set(L"no");
	ASSERT_EQ(s1.ToBool(),false) << "String should not be true";
}

TEST_F(WideStringTest, ToLong_1124234674) {
	ppl6::CWString s1(L"124234674");
	EXPECT_EQ((long)124234674,s1.ToLong()) << "Unexpected Result";
}

TEST_F(WideStringTest, ToLongLong_1242346214893456) {
	ppl6::CWString s1(L"1242346214893456");
	EXPECT_EQ((long long)1242346214893456,s1.ToLongLong()) << "Unexpected Result";
}

TEST_F(WideStringTest, ToFloat_182566142_346214893456) {
	ppl6::CWString s1(L"182566142.346214893456");
	EXPECT_EQ((float)182566142.346214893456,s1.ToFloat()) << "Unexpected Result";
}

TEST_F(WideStringTest, ToDouble_182566142_346214893456) {
	ppl6::CWString s1(L"182566142.346214893456");
	EXPECT_EQ((double)182566142.346214893456,s1.ToDouble()) << "Unexpected Result";
}

TEST_F(WideStringTest, ToCString) {
	ppl6::CWString s1(L"Thé Bíg Brown Föx jüps ovär the silly dog");
	ppl6::CString expected("Thé Bíg Brown Föx jüps ovär the silly dog");
	EXPECT_EQ(expected,s1.ToCString()) << "Unexpected Result";
}

TEST_F(WideStringTest, isTrue) {
	ppl6::CWString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.IsTrue(),false) << "String should not be true";
	s1.Set(L"");
	ASSERT_EQ(s1.IsTrue(),false) << "String should not be true";
	s1.Set(L"12345abcd");
	ASSERT_EQ(s1.IsTrue(),true) << "String should be true";
	s1.Set(L"1");
	ASSERT_EQ(s1.IsTrue(),true) << "String should be true";
	s1.Set(L"12345");
	ASSERT_EQ(s1.IsTrue(),true) << "String should be true";
	s1.Set(L"true");
	ASSERT_EQ(s1.IsTrue(),true) << "String should be true";
	s1.Set(L"wahr");
	ASSERT_EQ(s1.IsTrue(),true) << "String should be true";
	s1.Set(L"ja");
	ASSERT_EQ(s1.IsTrue(),true) << "String should be true";
	s1.Set(L"yes");
	ASSERT_EQ(s1.IsTrue(),true) << "String should be true";
	s1.Set(L"false");
	ASSERT_EQ(s1.IsTrue(),false) << "String should not be true";
	s1.Set(L"falsch");
	ASSERT_EQ(s1.IsTrue(),false) << "String should not be true";
	s1.Set(L"nein");
	ASSERT_EQ(s1.IsTrue(),false) << "String should not be true";
	s1.Set(L"no");
	ASSERT_EQ(s1.IsTrue(),false) << "String should not be true";
}

TEST_F(WideStringTest, isFalse) {
	ppl6::CWString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.IsFalse(),true) << "String should be false";
	s1.Set(L"");
	ASSERT_EQ(s1.IsFalse(),true) << "String should be false";
	s1.Set(L"12345abcd");
	ASSERT_EQ(s1.IsFalse(),false) << "String should not be false";
	s1.Set(L"12345");
	ASSERT_EQ(s1.IsFalse(),false) << "String should not be false";
	s1.Set(L"0");
	ASSERT_EQ(s1.IsFalse(),true) << "String should be false";
	s1.Set(L"true");
	ASSERT_EQ(s1.IsFalse(),false) << "String should not be false";
	s1.Set(L"wahr");
	ASSERT_EQ(s1.IsFalse(),false) << "String should not be false";
	s1.Set(L"ja");
	ASSERT_EQ(s1.IsFalse(),false) << "String should not be false";
	s1.Set(L"yes");
	ASSERT_EQ(s1.IsFalse(),false) << "String should not be false";
	s1.Set(L"false");
	ASSERT_EQ(s1.IsFalse(),true) << "String should be false";
	s1.Set(L"falsch");
	ASSERT_EQ(s1.IsFalse(),true) << "String should be false";
	s1.Set(L"nein");
	ASSERT_EQ(s1.IsFalse(),true) << "String should be false";
	s1.Set(L"no");
	ASSERT_EQ(s1.IsFalse(),true) << "String should be false";
}

TEST_F(WideStringTest, GetChar) {
	ppl6::CWString s1(L"Vögel üben Gezwitscher oft ähnlich packend wie Jupp die Maus auf dem Xylophon einer Qualle.");
	EXPECT_EQ((wchar_t)L'ö',s1.GetChar(1));
	EXPECT_EQ((wchar_t)L'z',s1.GetChar(13));
	EXPECT_EQ((wchar_t)L'Q',s1.GetChar(-7));
	EXPECT_EQ((wchar_t)0,s1.GetChar(900));
}

TEST_F(WideStringTest, GetWPtr) {
	ppl6::CWString s1(L"Vögel üben Gezwitscher oft ähnlich packend wie Jupp die Maus auf dem Xylophon einer Qualle.");
	const wchar_t *result=s1.GetWPtr();
	EXPECT_EQ((int)0,wcscmp(L"Vögel üben Gezwitscher oft ähnlich packend wie Jupp die Maus auf dem Xylophon einer Qualle.",result));
}

TEST_F(WideStringTest, GetPtr) {
	ppl6::CWString s1(L"Vögel üben Gezwitscher oft ähnlich packend wie Jupp die Maus auf dem Xylophon einer Qualle.");
	const char *result=s1.GetPtr();
	EXPECT_EQ((int)0,strcmp("Vögel üben Gezwitscher oft ähnlich packend wie Jupp die Maus auf dem Xylophon einer Qualle.",result));
}

TEST_F(WideStringTest, operator_ConstWCharPtr) {
	ppl6::CWString s1(L"Vögel üben Gezwitscher oft ähnlich packend wie Jupp die Maus auf dem Xylophon einer Qualle.");
	const wchar_t *result=s1;
	EXPECT_EQ((int)0,wcscmp(L"Vögel üben Gezwitscher oft ähnlich packend wie Jupp die Maus auf dem Xylophon einer Qualle.",result));
}

TEST_F(WideStringTest, operator_ConstCharPtr) {
	ppl6::CWString s1(L"Vögel üben Gezwitscher oft ähnlich packend wie Jupp die Maus auf dem Xylophon einer Qualle.");
	const char *result=s1;
	EXPECT_EQ((int)0,strcmp("Vögel üben Gezwitscher oft ähnlich packend wie Jupp die Maus auf dem Xylophon einer Qualle.",result));
}


TEST_F(WideStringTest, operator_SquareBracket) {
	ppl6::CWString s1(L"Vögel üben Gezwitscher oft ähnlich packend wie Jupp die Maus auf dem Xylophon einer Qualle.");
	EXPECT_EQ((wchar_t)L'ö',s1[1]);
	EXPECT_EQ((wchar_t)L'z',s1[13]);
	EXPECT_EQ((wchar_t)L'Q',s1[-7]);
	EXPECT_EQ((wchar_t)0,s1[900]);
}

TEST_F(WideStringTest, operator_Int_123456) {
	ppl6::CWString s1(L"123456");
	EXPECT_EQ((int)123456,(int)s1) << "Unexpected Result";
}

TEST_F(WideStringTest, operator_Int_minus123456) {
	ppl6::CWString s1(L"-123456");
	EXPECT_EQ((int)-123456,(int)s1) << "Unexpected Result";
}

TEST_F(WideStringTest, operator_UnsignedInt_123456) {
	ppl6::CWString s1(L"123456");
	EXPECT_EQ((int)123456,(unsigned int)s1) << "Unexpected Result";
}

TEST_F(WideStringTest, operator_UnsignedInt_minus123456) {
	ppl6::CWString s1(L"-123456");
	EXPECT_EQ((unsigned int)4294843840,(unsigned int)s1) << "Unexpected Result";
}

TEST_F(WideStringTest, operator_Equal_CharPtr) {
	ppl6::CWString s2(L"äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1="äöü, a test string with unicode characters";
	ASSERT_EQ(s1,s2) << "String has unexpected value";
	ASSERT_EQ((size_t)42,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, operator_Equal_WChartPtr) {
	ppl6::CWString s2(L"äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1=L"äöü, a test string with unicode characters";
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, operator_Equal_WStringPtr) {
	ppl6::CWString s2(L"äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1=&s2;
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, operator_Equal_WStringRef) {
	ppl6::CWString s2(L"äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1=s2;
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, operator_Equal_StringPtr) {
	ppl6::CWString s2(L"äöü, a test string with unicode characters");
	ppl6::CString s3("äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1=&s3;
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, operator_Equal_StringRef) {
	ppl6::CWString s2(L"äöü, a test string with unicode characters");
	ppl6::CString s3("äöü, a test string with unicode characters");
	ppl6::CWString s1;
	s1=s3;
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, setWChart) {
	ppl6::CWString s2(L"A");
	ppl6::CWString s1;
	s1=L'A';
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)1,s1.Length()) << "String has unexpected length";
}


TEST_F(WideStringTest, operator_PlusEqual_ChartPtr) {
	ppl6::CWString expected(L"First Part äöü, äöü Second Part");
	ppl6::CWString s1(L"First Part äöü, ");
	s1+="äöü Second Part";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, operator_PlusEqual_WChartPtr) {
	ppl6::CWString expected(L"First Part äöü, äöü Second Part");
	ppl6::CWString s1(L"First Part äöü, ");
	s1+=L"äöü Second Part";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.Length()) << "String has unexpected length";
}


TEST_F(WideStringTest, operator_PlusEqual_CWString) {
	ppl6::CWString expected(L"First Part äöü, äöü Second Part");
	ppl6::CWString s1(L"First Part äöü, ");
	ppl6::CWString s2(L"äöü Second Part");
	s1+=s2;
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, operator_PlusEqual_CString) {
	ppl6::CWString expected(L"First Part äöü, äöü Second Part");
	ppl6::CWString s1(L"First Part äöü, ");
	ppl6::CString s2("äöü Second Part");
	s1+=s2;
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, operator_PlusEqual_Wchart) {
	ppl6::CWString expected(L"First Part äöü, a");
	ppl6::CWString s1(L"First Part äöü, ");
	s1+=L'a';
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)17,s1.Length()) << "String has unexpected length";
}

TEST_F(WideStringTest, operator_EqualEqual) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString s2(L"The quick brown fox jumps over the lazy dog");
	ASSERT_EQ(s1,s2) << "String has unexpected value";
}

TEST_F(WideStringTest, operator_NotEqual) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString s2(L"The five boxing wizards jump quickly");
	ASSERT_NE(s1,s2) << "String has unexpected value";
}

TEST_F(WideStringTest, operator_Lower) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString s2(L"The five boxing wizards jump quickly");
	ASSERT_LT(s2,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, operator_LowerEqual) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString s2(L"The five boxing wizards jump quickly");
	ppl6::CWString s3(L"The quick brown fox jumps over the lazy dog");
	EXPECT_LE(s2,s1) << "String has unexpected value";
	EXPECT_LE(s3,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, operator_Greater) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString s2(L"The five boxing wizards jump quickly");
	ASSERT_GT(s1,s2) << "String has unexpected value";
}

TEST_F(WideStringTest, operator_GreaterEqual) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString s2(L"The five boxing wizards jump quickly");
	ppl6::CWString s3(L"The quick brown fox jumps over the lazy dog");
	EXPECT_GE(s1,s2) << "String has unexpected value";
	EXPECT_GE(s1,s3) << "String has unexpected value";
}

TEST_F(WideStringTest, GetMD5) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString expected(L"fb6cb2ddc37312ccec5b4d013ba724b1");
	ppl6::CWString s2=s1.GetMD5();
	EXPECT_EQ(expected,s2);
	/*
	ppl6::CFile ff;
	ff.Open("wide.txt","wb");
	ff.Write((const wchar_t*)s1,s1.Length()*sizeof(wchar_t));
	ff.Close();
	*/
}

TEST_F(WideStringTest, MD5) {
	ppl6::CWString s1(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString expected(L"fb6cb2ddc37312ccec5b4d013ba724b1");
	EXPECT_EQ(1,s1.MD5());
	EXPECT_EQ(expected,s1);
}

TEST_F(WideStringTest, MD5_CWString) {
	ppl6::CWString s1;
	ppl6::CWString s2(L"The quick brown fox jumps over the lazy dog");
	ppl6::CWString expected(L"fb6cb2ddc37312ccec5b4d013ba724b1");
	EXPECT_EQ(1,s1.MD5(s2));
	EXPECT_EQ(expected,s1);
}

TEST_F(WideStringTest, MD5_charPtr) {
	ppl6::CWString s1;
	ppl6::CWString expected(L"9e107d9d372bb6826bd81d3542a419d6");
	EXPECT_EQ(1,s1.MD5("The quick brown fox jumps over the lazy dog"));
	EXPECT_EQ(expected,s1);
}


/* TODO:

		CBinary ToCBinary() const;
		const void *GetBuffer() const;
		CWString& operator=(const CBinary& str);
 */

}

