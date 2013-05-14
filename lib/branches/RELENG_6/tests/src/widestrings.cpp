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
#include <ppl7.h>
#include <gtest/gtest.h>
#include "ppl6-tests.h"

namespace {

// The fixture for testing class Foo.
class WideStringTest : public ::testing::Test {
	protected:
	WideStringTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::WideString::setGlobalEncoding("UTF-8");
	}
	virtual ~WideStringTest() {

	}
};

TEST_F(WideStringTest, ConstructorSimple) {
	ASSERT_NO_THROW({
			ppl7::WideString s1;
			ASSERT_TRUE(s1.len()==0) << "String does not have zero length";
			const wchar_t *buf=s1.getPtr();
			ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
			ASSERT_EQ(0,buf[0]) << "Class did not return an empty c-string";

	}
	);
}

TEST_F(WideStringTest, ConstructorFromWideCharPtr) {
	ASSERT_NO_THROW({
			ppl7::WideString s1(L"A test string with unicode characters: äöü");
			ASSERT_EQ((size_t)42,s1.len()) << "String does not have length of 42";
			const unsigned char *buf=(const unsigned char *)s1.getPtr();
			ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
			ASSERT_EQ(L'A',(wchar_t)s1[0]) << "Unexpected Character in string";
			ASSERT_EQ(L'ü',(wchar_t)s1[41]) << "Unexpected Character in string";

	}
	);
}

TEST_F(WideStringTest, ConstructorFromStringPtr) {
	ASSERT_NO_THROW({
		ppl7::String s1("A test string with unicode characters: äöü");
		ppl7::WideString s2(&s1);
		ASSERT_EQ((size_t)42,s2.len()) << "String does not have length of 45";
		const wchar_t *buf=s2.getPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ(L'A',(wchar_t)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(L'ü',(wchar_t)s2[41]) << "Unexpected Character in string";
	}
	);
}




TEST_F(WideStringTest, ConstructorFromString) {
	ASSERT_NO_THROW({
		ppl7::String s1("A test string with unicode characters: äöü");
		ppl7::WideString s2(s1);
		ASSERT_EQ((size_t)42,s2.len()) << "String does not have length of 45";
		const wchar_t *buf=s2.getPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ(L'A',(wchar_t)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(L'ü',(wchar_t)s2[41]) << "Unexpected Character in string";
	});
}

TEST_F(WideStringTest, ConstructorFromWideString) {
	ASSERT_NO_THROW({
		ppl7::WideString s1(L"A test string with unicode characters: äöü");
		ppl7::WideString s2(s1);
		ASSERT_EQ((size_t)42,s2.len()) << "String does not have expected length";
		const wchar_t *buf=s2.getPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ(L'A',(wchar_t)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(L'ü',(wchar_t)s2[41]) << "Unexpected Character in string";
	});
}

TEST_F(WideStringTest, ConstructorFromWideStringPtr) {
	ASSERT_NO_THROW({
		ppl7::WideString s1(L"A test string with unicode characters: äöü");
		ppl7::WideString s2(&s1);
		ASSERT_EQ((size_t)42,s2.len()) << "String does not have expected length";
		const wchar_t *buf=s2.getPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ(L'A',(wchar_t)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(L'ü',(wchar_t)s2[41]) << "Unexpected Character in string";
	});
}

TEST_F(WideStringTest, ConstructorFromStdString) {
	ASSERT_NO_THROW({
		std::string s1("A test string with unicode characters: äöü");
		ppl7::WideString s2(s1);
		ASSERT_EQ((size_t)42,s2.len()) << "String does not have expected length";
		const wchar_t *buf=s2.getPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ(L'A',(wchar_t)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(L'ü',(wchar_t)s2[41]) << "Unexpected Character in string";
	});
}

TEST_F(WideStringTest, ConstructorFromStdWString) {
	ASSERT_NO_THROW({
		std::wstring s1(L"A test string with unicode characters: äöü");
		ASSERT_EQ((size_t)42,s1.size()) << "std:wstring has unexpected size";
		ppl7::WideString s2(s1);
		ASSERT_EQ((size_t)42,s2.len()) << "String does not have expected length";
		const wchar_t *buf=s2.getPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ(L'A',(wchar_t)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ((wchar_t)L'ü',(wchar_t)s2[41]) << "Unexpected Character in string";
	});
}

TEST_F(WideStringTest, clear) {
	ppl7::WideString s1(L"A test string with unicode characters: äöü");
	s1.clear();
	ASSERT_EQ((size_t)0,s1.len()) << "String does not have length of 0";
	const wchar_t *buf=s1.getPtr();
	ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
	ASSERT_THROW({
			ASSERT_EQ(0,(unsigned char)s1[0]) << "Unexpected Character in string";
	},ppl7::OutOfBoundsEception);
}

TEST_F(WideStringTest, CapacityAndReserve) {
	ppl7::WideString s1;
	ASSERT_EQ((size_t)0,s1.capacity()) << "capacity did not return 0";
	s1.reserve(0);
	ASSERT_EQ((size_t)0,s1.capacity()) << "capacity did not return 0";
	s1.reserve(32);
	ASSERT_EQ((size_t)32,s1.capacity()) << "capacity did not return expected value";
	s1.reserve(128);
	ASSERT_EQ((size_t)128,s1.capacity()) << "capacity did not return expected value";
	s1.reserve(32);
	ASSERT_EQ((size_t)128,s1.capacity()) << "capacity did not return expected value";
}



TEST_F(WideStringTest, len) {
	ppl7::WideString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.len(),(size_t)42) << "String has unexpected size";
}

TEST_F(WideStringTest, length) {
	ppl7::WideString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.length(),(size_t)42) << "String has unexpected size";
}

TEST_F(WideStringTest, size) {
	ppl7::WideString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.size(),(size_t)42) << "String has unexpected size";
}

TEST_F(WideStringTest, isEmpty) {
	ppl7::WideString s1(L"A test string with unicode characters: äöü");
	ppl7::WideString s2;
	ASSERT_EQ(s1.isEmpty(),false) << "String is empty";
	ASSERT_EQ(s2.isEmpty(),true) << "String not empty";
}

TEST_F(WideStringTest, notEmpty) {
	ppl7::WideString s1(L"A test string with unicode characters: äöü");
	ppl7::WideString s2;
	ASSERT_EQ(s1.notEmpty(),true) << "String is not empty";
	ASSERT_EQ(s2.notEmpty(),false) << "String is empty";
}

TEST_F(WideStringTest, isNumeric) {
	ppl7::WideString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.isNumeric(),false) << "String should not be numeric";
	s1.set(L"");
	ASSERT_EQ(s1.isNumeric(),false) << "String should not be numeric";
	s1.set(L"12345abcd");
	ASSERT_EQ(s1.isNumeric(),false) << "String should not be numeric";
	s1.set(L"12345");
	ASSERT_EQ(s1.isNumeric(),true) << "String should be numeric";
	s1.set(L"123.451,12");
	ASSERT_EQ(s1.isNumeric(),true) << "String should be numeric";
	s1.set(L"-123.451,12");
	ASSERT_EQ(s1.isNumeric(),true) << "String should be numeric";
	s1.set(L"-123.451,12-9");
	ASSERT_EQ(s1.isNumeric(),false) << "String should not be numeric";
}

TEST_F(WideStringTest, isInteger) {
	ppl7::WideString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.isInteger(),false) << "String should not be an integer";
	s1.set(L"");
	ASSERT_EQ(s1.isInteger(),false) << "String should not be an integer";
	s1.set(L"12345abcd");
	ASSERT_EQ(s1.isInteger(),false) << "String should not be an integer";
	s1.set(L"12345");
	ASSERT_EQ(s1.isInteger(),true) << "String should be an integer";
	s1.set(L"-12345");
	ASSERT_EQ(s1.isInteger(),true) << "String should be an integer";
	s1.set(L"123-451");
	ASSERT_EQ(s1.isInteger(),false) << "String should be an integer";
	s1.set(L"123.451,12");
	ASSERT_EQ(s1.isInteger(),false) << "String should be an integer";
	s1.set(L"-123.451,12");
	ASSERT_EQ(s1.isInteger(),false) << "String should be an integer";
	s1.set(L"-123.451,12-9");
	ASSERT_EQ(s1.isInteger(),false) << "String should not be an integer";
}

TEST_F(WideStringTest, isTrue) {
	ppl7::WideString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set(L"");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set(L"12345abcd");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set(L"1");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set(L"12345");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set(L"true");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set(L"wahr");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set(L"ja");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set(L"yes");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set(L"false");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set(L"falsch");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set(L"nein");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set(L"no");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
}

TEST_F(WideStringTest, isFalse) {
	ppl7::WideString s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"12345abcd");
	ASSERT_EQ(s1.isFalse(),false) << "String should not be false";
	s1.set(L"12345");
	ASSERT_EQ(s1.isFalse(),false) << "String should not be false";
	s1.set(L"0");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"true");
	ASSERT_EQ(s1.isFalse(),false) << "String should not be false";
	s1.set(L"wahr");
	ASSERT_EQ(s1.isFalse(),false) << "String should not be false";
	s1.set(L"ja");
	ASSERT_EQ(s1.isFalse(),false) << "String should not be false";
	s1.set(L"yes");
	ASSERT_EQ(s1.isFalse(),false) << "String should not be false";
	s1.set(L"false");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"falsch");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"nein");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"no");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
}


TEST_F(WideStringTest, setConstCharWithoutSize) {
	ppl7::WideString s2(L"äöü, a test string with unicode characters");
	ppl7::WideString s1;
	s1.set(L"äöü, a test string with unicode characters");
	ASSERT_EQ(s1,s2) << "String has unexpected value";
	ASSERT_EQ((size_t)42,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, setConstCharWithSize) {
	ppl7::WideString s2(L"äöü, a tes");
	ppl7::WideString s1;
	s1.set(L"äöü, a test string with unicode characters",10);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)10,s1.size()) << "String has unexpected length";
}


TEST_F(WideStringTest, setConstWChartWithoutSize) {
	ppl7::WideString s2(L"äöü, a test string with unicode characters");
	ppl7::WideString s1;
	s1.set(L"äöü, a test string with unicode characters");
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, setConstWChartWithSize) {
	ppl7::WideString s2(L"äöü, a tes");
	ppl7::WideString s1;
	s1.set(L"äöü, a test string with unicode characters",10);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)10,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, setStringPtrWithoutSize) {
	ppl7::WideString s2(L"äöü, a test string with unicode characters");
	ppl7::WideString s1;
	s1.set(&s2);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, setStringPtrWithSize) {
	ppl7::WideString s2(L"äöü, a tes");
	ppl7::WideString s3(L"äöü, a test string with unicode characters");
	ppl7::WideString s1;
	s1.set(&s3,10);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)10,s1.size()) << "String has unexpected length";
}


TEST_F(WideStringTest, setStringRefWithoutSize) {
	ppl7::WideString s2(L"äöü, a test string with unicode characters");
	ppl7::WideString s1;
	s1.set(s2);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, setStringRefWithSize) {
	ppl7::WideString s2(L"äöü, a tes");
	ppl7::WideString s3(L"äöü, a test string with unicode characters");
	ppl7::WideString s1;
	s1.set(s3,10);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)10,s1.size()) << "String has unexpected length";
}


TEST_F(WideStringTest, setSTDStringRefWithoutSize) {
	ppl7::WideString s2(L"äöü, a test string with unicode characters");
	std::string s3("äöü, a test string with unicode characters");
	ppl7::WideString s1;
	s1.set(s3);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.size()) << "String has unexpected length";
}



TEST_F(WideStringTest, setSTDStringRefWithSize) {
	ppl7::WideString s2(L"äöü, a tes");
	std::string s3("äöü, a test string with unicode characters");
	ppl7::WideString s1;
	s1.set(s3,10);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)10,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, setSTDWStringRefWithoutSize) {
	ppl7::WideString s2(L"äöü, a test string with unicode characters");
	std::wstring s3(L"äöü, a test string with unicode characters");
	ppl7::WideString s1;
	s1.set(s3);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)42,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, setSTDWStringRefWithSize) {
	ppl7::WideString s2(L"äöü, a tes");
	std::wstring s3(L"äöü, a test string with unicode characters");
	ppl7::WideString s1;
	s1.set(s3,10);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)10,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, setf) {
	ppl7::WideString s2(L"Ein Test, 42, Wide, 10000");
	ppl7::WideString s1;
	s1.setf("Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)25,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, setWchart) {
	ppl7::WideString s2(L"a");
	ppl7::WideString s1;
	s1.set((wchar_t)'a');
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)1,s1.size()) << "String has unexpected length";
}

static void test_vasprintf(ppl7::WideString &str,const char *fmt,...)
{
	va_list args;
	va_start(args, fmt);
	str.vasprintf(fmt,args);
	va_end(args);
}


TEST_F(WideStringTest, vasprintf) {
	ppl7::WideString s2(L"Ein Test, 42, Wide, 10000");
	ppl7::WideString s1;
	test_vasprintf(s1,"Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)25,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendConstWchartWithoutSize) {
	ppl7::WideString expected(L"First Part äöü, äöü Second Part");
	ppl7::WideString s1(L"First Part äöü, ");
	s1.append(L"äöü Second Part");

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendConstWchartWithSize) {
	ppl7::WideString expected(L"First Part äöü, äöü S");
	ppl7::WideString s1(L"First Part äöü, ");
	s1.append(L"äöü Second Part",5);

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendConstCharPtrWithoutSize) {
	ppl7::WideString expected(L"First Part äöü, äöü Second Part");
	ppl7::WideString s1(L"First Part äöü, ");
	s1.append(L"äöü Second Part");

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendConstCharPtrWithSize) {
	ppl7::WideString expected(L"First Part äöü, äöü Seco");
	ppl7::WideString s1(L"First Part äöü, ");
	s1.append(L"äöü Second Part",8);

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)24,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStringPtrWithoutSize) {
	ppl7::WideString expected(L"First Part äöü, äöü Second Part");
	ppl7::WideString s1(L"First Part äöü, ");
	ppl7::WideString s2(L"äöü Second Part");
	s1.append(&s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStringPtrWithSize) {
	ppl7::WideString expected(L"First Part äöü, äöü Seco");
	ppl7::WideString s1(L"First Part äöü, ");
	ppl7::WideString s2(L"äöü Second Part");
	s1.append(&s2,8);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)24,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStringWithoutSize) {
	ppl7::WideString expected(L"First Part äöü, äöü Second Part");
	ppl7::WideString s1(L"First Part äöü, ");
	ppl7::WideString s2(L"äöü Second Part");
	s1.append(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStringWithSize) {
	ppl7::WideString expected(L"First Part äöü, äöü Seco");
	ppl7::WideString s1(L"First Part äöü, ");
	ppl7::WideString s2(L"äöü Second Part");
	s1.append(s2,8);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)24,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStdStringWithoutSize) {
	ppl7::WideString expected(L"First Part äöü, äöü Second Part");
	ppl7::WideString s1(L"First Part äöü, ");
	std::string s2("äöü Second Part");
	s1.append(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStdStringWithSize) {
	ppl7::WideString expected(L"First Part äöü, äöü S");
	ppl7::WideString s1(L"First Part äöü, ");
	std::string s2("äöü Second Part");
	s1.append(s2,5);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStdWStringWithoutSize) {
	ppl7::WideString expected(L"First Part äöü, äöü Second Part");
	ppl7::WideString s1(L"First Part äöü, ");
	std::wstring s2(L"äöü Second Part");
	s1.append(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendStdWStringWithSize) {
	ppl7::WideString expected(L"First Part äöü, äöü S");
	ppl7::WideString s1(L"First Part äöü, ");
	std::wstring s2(L"äöü Second Part");
	s1.append(s2,5);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendf) {
	ppl7::WideString expected(L"First Part äöü, Ein Test, 42, Wide, 10000");
	ppl7::WideString s1(L"First Part äöü, ");
	s1.appendf("Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)41,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, appendWchart) {
	ppl7::WideString expected(L"First Part äöü, a");
	ppl7::WideString s1(L"First Part äöü, ");
	s1.append((wchar_t)'a');
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)17,s1.size()) << "String has unexpected length";
}


TEST_F(WideStringTest, prependConstWchartWithoutSize) {
	ppl7::WideString expected(L"äöü Second PartFirst Part äöü, ");
	ppl7::WideString s1(L"First Part äöü, ");
	s1.prepend(L"äöü Second Part");
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, prependConstWchartWithSize) {
	ppl7::WideString expected(L"äöü SFirst Part äöü, ");
	ppl7::WideString s1(L"First Part äöü, ");
	s1.prepend(L"äöü Second Part",5);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, prependConstCharPtrWithoutSize) {
	ppl7::WideString expected(L"äöü Second PartFirst Part äöü, ");
	ppl7::WideString s1(L"First Part äöü, ");
	s1.prepend(L"äöü Second Part");
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, prependConstCharPtrWithSize) {
	ppl7::WideString expected(L"äöü SFirst Part äöü, ");
	ppl7::WideString s1(L"First Part äöü, ");
	s1.prepend(L"äöü Second Part",5);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, prependStringPtrWithoutSize) {
	ppl7::WideString expected(L"äöü Second PartFirst Part äöü, ");
	ppl7::WideString s1(L"First Part äöü, ");
	ppl7::WideString s2(L"äöü Second Part");
	s1.prepend(&s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, prependStringPtrWithSize) {
	ppl7::WideString expected(L"äöü SFirst Part äöü, ");
	ppl7::WideString s1(L"First Part äöü, ");
	ppl7::WideString s2(L"äöü Second Part");
	s1.prepend(&s2,5);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, prependStringWithoutSize) {
	ppl7::WideString expected(L"äöü Second PartFirst Part äöü, ");
	ppl7::WideString s1(L"First Part äöü, ");
	ppl7::WideString s2(L"äöü Second Part");
	s1.prepend(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, prependStringWithSize) {
	ppl7::WideString expected(L"äöü SFirst Part äöü, ");
	ppl7::WideString s1(L"First Part äöü, ");
	ppl7::WideString s2(L"äöü Second Part");
	s1.prepend(s2,5);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, prependStdStringWithoutSize) {
	ppl7::WideString expected(L"äöü Second PartFirst Part äöü, ");
	ppl7::WideString s1(L"First Part äöü, ");
	std::string s2("äöü Second Part");
	s1.prepend(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, prependStdStringWithSize) {
	ppl7::WideString expected(L"äöü SFirst Part äöü, ");
	ppl7::WideString s1(L"First Part äöü, ");
	std::string s2("äöü Second Part");
	s1.prepend(s2,5);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, prependStdWStringWithoutSize) {
	ppl7::WideString expected(L"äöü Second PartFirst Part äöü, ");
	ppl7::WideString s1(L"First Part äöü, ");
	std::wstring s2(L"äöü Second Part");
	s1.prepend(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)31,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, prependStdWStringWithSize) {
	ppl7::WideString expected(L"äöü SFirst Part äöü, ");
	ppl7::WideString s1(L"First Part äöü, ");
	std::wstring s2(L"äöü Second Part");
	s1.prepend(s2,5);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)21,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, prependf) {
	ppl7::WideString expected(L"Ein Test, 42, Wide, 10000First Part äöü, ");
	ppl7::WideString s1(L"First Part äöü, ");
	s1.prependf("Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)41,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, prependWchart) {
	ppl7::WideString expected(L"aFirst Part äöü, ");
	ppl7::WideString s1(L"First Part äöü, ");
	s1.prepend((wchar_t)'a');
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)17,s1.size()) << "String has unexpected length";
}

TEST_F(WideStringTest, chopRight) {
	ppl7::WideString s1(L"A test string with unicode characters: äöü");
	ppl7::WideString s2(L"A test string with unicode characters: ä");
	ppl7::WideString s3(L"A test string with unicode characters:");
	s1.chopRight(2);
	ASSERT_EQ((size_t)40,s1.len()) << "String does not have expected length";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	s1.chopRight();
	s1.chopRight();
	ASSERT_EQ((size_t)38,s1.len()) << "String does not have expected length";
	ASSERT_EQ(s3,s1) << "String has unexpected value";
	s1.chopRight(37);
	ASSERT_EQ((size_t)1,s1.len()) << "String does not have length of 1";
	s1.chopRight(1);
	ASSERT_EQ((size_t)0,s1.len()) << "String does not have length of 0";
	s2.clear();
	ASSERT_EQ(s2,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, chopLeft) {
	ppl7::WideString s1(L"A test string with unicode characters: äöü");
	ppl7::WideString s2(L"est string with unicode characters: äöü");
	ppl7::WideString s3(L"st string with unicode characters: äöü");
	s1.chopLeft(3);
	ASSERT_EQ((size_t)39,s1.len()) << "String does not have expected length";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	s1.chopLeft();
	ASSERT_EQ((size_t)38,s1.len()) << "String does not have expected length";
	ASSERT_EQ(s3,s1) << "String has unexpected value";
	s1.chopRight(37);
	ASSERT_EQ((size_t)1,s1.len()) << "String does not have length of 1";
	s1.chopRight(1);
	ASSERT_EQ((size_t)0,s1.len()) << "String does not have length of 0";
	s2.clear();
	ASSERT_EQ(s2,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, chop) {
	ppl7::WideString s1(L"A test string with unicode characters: äöü");
	ppl7::WideString s2(L"A test string with unicode characters: ä");
	ppl7::WideString s3(L"A test string with unicode characters:");
	s1.chop(2);
	ASSERT_EQ((size_t)40,s1.len()) << "String does not have length of 40";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	s1.chop();
	s1.chop();
	ASSERT_EQ((size_t)38,s1.len()) << "String does not have length of 38";
	ASSERT_EQ(s3,s1) << "String has unexpected value";
	s1.chop(37);
	ASSERT_EQ((size_t)1,s1.len()) << "String does not have length of 1";
	s1.chop();
	ASSERT_EQ((size_t)0,s1.len()) << "String does not have length of 0";
	s2.clear();
	ASSERT_EQ(s2,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, chomp) {
	ppl7::WideString s1(L"\n\r\nA test string\näöü\n\r\n");
	ppl7::WideString s2(L"A test string\näöü");
	s1.chomp();
	ASSERT_EQ((size_t)17,s1.len()) << "String does not have length of 17";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	s1.chomp();
	ASSERT_EQ((size_t)17,s1.len()) << "String does not have length of 17";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, cut_WithPos) {
	ppl7::WideString s1(L"The quick brown fox jumps over the lazy dog");
	ppl7::WideString expected(L"The quick ");
	s1.cut(10);
	ASSERT_EQ((size_t)10,s1.len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, cut_WithPos0) {
	ppl7::WideString s1(L"The quick brown fox jumps over the lazy dog");
	ppl7::WideString expected(L"");
	s1.cut(0);
	ASSERT_EQ((size_t)0,s1.len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, cut_WithPosBeyondStringLength) {
	ppl7::WideString s1(L"The quick brown fox jumps over the lazy dog");
	ppl7::WideString expected(L"The quick brown fox jumps over the lazy dog");
	s1.cut(100);
	ASSERT_EQ((size_t)43,s1.len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, cut_WithLetter) {
	ppl7::WideString s1(L"The quick brown fox jumps over the lazy dog");
	ppl7::WideString expected(L"The quick ");
	s1.cut(L"b");
	ASSERT_EQ((size_t)10,s1.len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, cut_WithNonexistingLetter) {
	ppl7::WideString s1(L"The quick brown fox");
	ppl7::WideString expected(L"The quick brown fox");
	s1.cut(L"j");
	ASSERT_EQ((size_t)19,s1.len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(WideStringTest, cut_WithString) {
	ppl7::WideString s1(L"The quick brown fox jumps over the lazy dog");
	ppl7::WideString expected(L"The quick ");
	s1.cut(L"brown");
	ASSERT_EQ((size_t)10,s1.len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}


TEST_F(WideStringTest, strstr) {
	ppl7::WideString s1(L"A test haystack string");
	ppl7::WideString s3(L"haystack string");
	ppl7::WideString s2;
	s2=s1.strstr(L"haystack");
	ASSERT_EQ((size_t)15,s2.len()) << "String does not have length of 15";
	ASSERT_EQ(s3,s2) << "String has unexpected value";
	s2=s1.strstr(L"beer");
	s3="";
	ASSERT_EQ((size_t)0,s2.len()) << "String does not have length of 0";
	ASSERT_EQ(s3,s2) << "String has unexpected value";

	s2=s1.strstr(L"");
	ASSERT_EQ(s2,s1) << "String has unexpected value";

}

TEST_F(WideStringTest, repeated) {
	ppl7::WideString s1(L"_repeat_");
	ppl7::WideString s3(L"_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
	ppl7::WideString s2;
	s2=s1.repeated(10);
	ASSERT_EQ((size_t)80,s2.len()) << "String does not have length of 80";
	ASSERT_EQ(s3,s2) << "String has unexpected value";
}

TEST_F(WideStringTest, trimLeft) {
	ppl7::WideString s1(L"\n\n    abc  \n");
	s1.trimLeft();
	ASSERT_EQ((size_t)6,s1.size());
	ASSERT_EQ(ppl7::WideString(L"abc  \n"),s1);
}

TEST_F(WideStringTest, trimRight) {
	ppl7::WideString s1(L" \n  abc  \n");
	s1.trimRight();
	ASSERT_EQ((size_t)7,s1.size());
	ASSERT_EQ(ppl7::WideString(L" \n  abc"),s1);
}

TEST_F(WideStringTest, trim) {
	ppl7::WideString s1(L"\n\n    abc  \n");
	s1.trim();
	ASSERT_EQ((size_t)3,s1.size());
	ASSERT_EQ(ppl7::WideString(L"abc"),s1);
}

TEST_F(WideStringTest, trimLeftEmptyResult) {
	ppl7::WideString s1(L"\n\n   \n   \n");
	s1.trimLeft();
	ASSERT_EQ((size_t)0,s1.size());
	ASSERT_EQ(ppl7::WideString(L""),s1);
}

TEST_F(WideStringTest, trimRightEmptyResult) {
	ppl7::WideString s1(L"\n\n   \n   \n");
	s1.trimRight();
	ASSERT_EQ((size_t)0,s1.size());
	ASSERT_EQ(ppl7::WideString(L""),s1);
}

TEST_F(WideStringTest, trimEmptyResult) {
	ppl7::WideString s1(L"\n\n   \n   \n");
	s1.trim();
	ASSERT_EQ((size_t)0,s1.size());
	ASSERT_EQ(ppl7::WideString(L""),s1);
}

TEST_F(WideStringTest, trimLeftChars) {
	ppl7::WideString s1(L"\n\n    abc  \n");
	s1.trimLeft(L" \n");
	ASSERT_EQ((size_t)6,s1.size());
	ASSERT_EQ(ppl7::WideString(L"abc  \n"),s1);
}

TEST_F(WideStringTest, trimRightChars) {
	ppl7::WideString s1(L" \n  abc  \n");
	s1.trimRight(L" \n");
	ASSERT_EQ((size_t)7,s1.size());
	ASSERT_EQ(ppl7::WideString(L" \n  abc"),s1);
}

TEST_F(WideStringTest, trimChars) {
	ppl7::WideString s1(L"\n\n    abc  \n");
	s1.trim(L" \n");
	ASSERT_EQ((size_t)3,s1.size());
	ASSERT_EQ(ppl7::WideString(L"abc"),s1);
}

TEST_F(WideStringTest, trimLeftCharsEmptyResult) {
	ppl7::WideString s1(L"\n\n   \n   \n");
	s1.trimLeft(L" \n");
	ASSERT_EQ((size_t)0,s1.size());
	ASSERT_EQ(ppl7::WideString(L""),s1);
}

TEST_F(WideStringTest, trimRightCharsEmptyResult) {
	ppl7::WideString s1(L"\n\n   \n   \n");
	s1.trimRight(L" \n");
	ASSERT_EQ((size_t)0,s1.size());
	ASSERT_EQ(ppl7::WideString(L""),s1);
}

TEST_F(WideStringTest, trimEmptyCharsResult) {
	ppl7::WideString s1(L"\n\n   \n   \n");
	s1.trim(L" \n");
	ASSERT_EQ((size_t)0,s1.size());
	ASSERT_EQ(ppl7::WideString(L""),s1);
}

TEST_F(WideStringTest, operatorEqualEmpty) {
	ppl7::WideString s1(L"");
	ppl7::WideString s2(L"");
	ASSERT_EQ(s1,s2);
}

TEST_F(WideStringTest, strcmpEmpty) {
	ppl7::WideString s1(L"");
	ppl7::WideString s2(L"");
	ASSERT_EQ(0,s1.strcmp(s2));
}

TEST_F(WideStringTest, strcmpLower) {
	ppl7::WideString s1(L"ABcdef");
	ppl7::WideString s2(L"Defghi");
	ASSERT_LT(s1.strcmp(s2),0);
}

TEST_F(WideStringTest, strcmpLowerWithCase) {
	ppl7::WideString s1(L"ABcdef");
	ppl7::WideString s2(L"abcdef");
	ASSERT_LT(s1.strcmp(s2),0);
}

TEST_F(WideStringTest, strcmpHigher) {
	ppl7::WideString s1(L"ABcdef");
	ppl7::WideString s2(L"Defghi");
	ASSERT_GT(s2.strcmp(s1),0);
}

TEST_F(WideStringTest, strcmpHigherWithCase) {
	ppl7::WideString s1(L"ABcdef");
	ppl7::WideString s2(L"abcdef");
	ASSERT_GT(s2.strcmp(s1),0);
}

TEST_F(WideStringTest, strcmpEqual) {
	ppl7::WideString s1(L"ABcdef");
	ppl7::WideString s2(L"ABcdef");
	ASSERT_EQ(s2.strcmp(s1),0);
}

TEST_F(WideStringTest, strcasecmpEmpty) {
	ppl7::WideString s1(L"");
	ppl7::WideString s2(L"");
	ASSERT_EQ(0,s1.strcasecmp(s2));
}

TEST_F(WideStringTest, strcasecmpLower) {
	ppl7::WideString s1(L"ABcdef");
	ppl7::WideString s2(L"Defghi");
	ASSERT_LT(s1.strcasecmp(s2),0);
}

TEST_F(WideStringTest, strcasecmpLowerWithCase) {
	ppl7::WideString s1(L"ABcdef");
	ppl7::WideString s2(L"abcdef");
	ASSERT_EQ(s1.strcasecmp(s2),0);
}

TEST_F(WideStringTest, strcasecmpHigher) {
	ppl7::WideString s1(L"ABcdef");
	ppl7::WideString s2(L"Defghi");
	ASSERT_GT(s2.strcasecmp(s1),0);
}

TEST_F(WideStringTest, strcasecmpHigherWithCase) {
	ppl7::WideString s1(L"ABcdef");
	ppl7::WideString s2(L"abcdef");
	ASSERT_EQ(s2.strcasecmp(s1),0);
}

TEST_F(WideStringTest, strcasecmpEqual) {
	ppl7::WideString s1(L"ABcdef");
	ppl7::WideString s2(L"ABcdef");
	ASSERT_EQ(s2.strcasecmp(s1),0);
}


TEST_F(WideStringTest, left) {
	ppl7::WideString s1(L"The quick brown fox jumps over the lazy dog");
	ppl7::WideString s2=s1.left(10);
	ASSERT_EQ(ppl7::WideString(L"The quick "),s2);
}

TEST_F(WideStringTest, right) {
	ppl7::WideString s1(L"The quick brown fox jumps over the lazy dog");
	ppl7::WideString s2=s1.right(9);
	ASSERT_EQ(ppl7::WideString(L" lazy dog"),s2);
}

TEST_F(WideStringTest, midWithLength) {
	ppl7::WideString s1(L"The quick brown fox jumps over the lazy dog");
	ppl7::WideString s2=s1.mid(10,10);
	ASSERT_EQ(ppl7::WideString(L"brown fox "),s2);
}

TEST_F(WideStringTest, midWithoutLength) {
	ppl7::WideString s1(L"The quick brown fox jumps over the lazy dog");
	ppl7::WideString s2=s1.mid(10);
	ASSERT_EQ(ppl7::WideString(L"brown fox jumps over the lazy dog"),s2);
}


TEST_F(WideStringTest, substrWithLength) {
	ppl7::WideString s1(L"The quick brown fox jumps over the lazy dog");
	ppl7::WideString s2=s1.substr(10,10);
	ASSERT_EQ(ppl7::WideString(L"brown fox "),s2);
}

TEST_F(WideStringTest, substrWithoutLength) {
	ppl7::WideString s1(L"The quick brown fox jumps over the lazy dog");
	ppl7::WideString s2=s1.substr(10);
	ASSERT_EQ(ppl7::WideString(L"brown fox jumps over the lazy dog"),s2);
}

TEST_F(WideStringTest, lowerCase) {
	ppl7::WideString s1(L"The Quick Brown Fox Jumps over ÄÖÜ");
	ppl7::WideString expected(L"the quick brown fox jumps over äöü");
	ASSERT_NO_THROW(s1.lowerCase());
	ASSERT_EQ(expected,s1);
}

TEST_F(WideStringTest, upperCase) {
	ppl7::WideString s1(L"The Quick Brown Fox Jumps over äöü");
	ppl7::WideString expected(L"THE QUICK BROWN FOX JUMPS OVER ÄÖÜ");
	ASSERT_NO_THROW(s1.upperCase());
	ASSERT_EQ(expected,s1);
}

TEST_F(WideStringTest, upperCaseWords) {
	ppl7::WideString s1(L"the quick brown fox jumps over äöü");
	ppl7::WideString expected(L"The Quick Brown Fox Jumps Over Äöü");
	ASSERT_NO_THROW(s1.upperCaseWords());
	ASSERT_EQ(expected,s1);
}


TEST_F(WideStringTest, pregMatchPositive) {
	ppl7::WideString s1(L"Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
	ppl7::WideString expr(L"/^Lorem.*$/s");
	ASSERT_TRUE(s1.pregMatch(expr));
	expr.set(L"/^Lorem.*$/is");
	ASSERT_TRUE(s1.pregMatch(expr));
	expr.set(L"/consectetuer/");
	ASSERT_TRUE(s1.pregMatch(expr));
	expr.set(L"/^.*consectetuer.*$/s");
	ASSERT_TRUE(s1.pregMatch(expr));
	expr.set(L"/^.*mus\\.$/m");
	ASSERT_TRUE(s1.pregMatch(expr));
}

TEST_F(WideStringTest, pregMatchNegativ) {
	ppl7::WideString s1(L"Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
	ppl7::WideString expr(L"/^Looorem.*$/s");
	ASSERT_FALSE(s1.pregMatch(expr));
	expr.set(L"/^ipsum.*$/is");
	ASSERT_FALSE(s1.pregMatch(expr));
	expr.set(L"/patrick/");
	ASSERT_FALSE(s1.pregMatch(expr));
	expr.set(L"/^.*patrick.*$/s");
	ASSERT_FALSE(s1.pregMatch(expr));
	expr.set(L"/^.*mus\\.$/");
	ASSERT_FALSE(s1.pregMatch(expr));
}

TEST_F(WideStringTest, pregCapture) {
	ppl7::Array m;
	ppl7::WideString s1(L"2012-05-18");
	ASSERT_TRUE(s1.pregMatch(L"/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})$/",m));
	ASSERT_EQ(2012,m[1].toInt()) << "Unexpected value in capture";
	ASSERT_EQ(5,m[2].toInt()) << "Unexpected value in capture";
	ASSERT_EQ(18,m[3].toInt()) << "Unexpected value in capture";
	ASSERT_EQ((size_t)4,m.size()) << "Unexpected number auf captures";

}



TEST_F(WideStringTest, Utf8toUtf8) {
	ASSERT_NO_THROW({
		ppl7::WideString s1(L"A test string with unicode characters: äöü");
		ppl7::ByteArray a=s1.toUtf8();
		ASSERT_EQ((size_t)45,a.size()) << "String does not have expected length";
		ASSERT_EQ((unsigned char)'A',(unsigned char)a.get(0)) << "Unexpected Character in string";
		ASSERT_EQ((unsigned char)188,(unsigned char)a.get(44)) << "Unexpected Character in string";
	});
}

TEST_F(WideStringTest, ISO88591toUtf8) {
	if (setlocale(LC_ALL,"de_DE.ISO8859-1")==NULL) {
		FAIL() << "setlocale fehlgeschlagen\n";
	}
	ppl7::WideString s1;
	ASSERT_NO_THROW({
		s1.set(L"A test string with unicode characters: ");
		s1.append(0xe4);
		s1.append(0xf6);
		s1.append(0xfc);
	});
	ASSERT_EQ((size_t)42,s1.size()) << "String does not have expected length";
	ASSERT_EQ('A',(unsigned char)s1[0]) << "Unexpected Character in string";
	ASSERT_EQ(228,(unsigned char)s1[39]) << "Unexpected Character in string";
	ppl7::ByteArray a;
	ASSERT_NO_THROW({
		a=s1.toUtf8();
	});
	//a.hexDump();
	ASSERT_EQ((size_t)45,a.size()) << "String does not have expected length";
	ASSERT_EQ((unsigned char)'A',(unsigned char)a.get(0)) << "Unexpected Character in string";
	ASSERT_EQ(188,(unsigned char)a.get(44)) << "Unexpected Character in string";
}

TEST_F(WideStringTest, strchr_ExistingChar) {
	ppl7::WideString s1(L"The Quick Brown Fox Jumps over äöü");
	ppl7::WideString expected(L"Fox Jumps over äöü");
	ASSERT_EQ(expected,s1.strchr('F')) << "Unexpected Result";
}

TEST_F(WideStringTest, strchr_NonExistingChar) {
	ppl7::WideString s1(L"The Quick Brown Fox Jumps over äöü");
	ppl7::WideString expected(L"");
	ASSERT_EQ(expected,s1.strchr('L')) << "Unexpected Result";
}

TEST_F(WideStringTest, strrchr_ExistingChar) {
	ppl7::WideString s1(L"The Quick Brown Fox Jumps over äöü");
	ppl7::WideString expected(L"over äöü");
	ASSERT_EQ(expected,s1.strrchr('o')) << "Unexpected Result";
}

TEST_F(WideStringTest, strrchr_NonExistingChar) {
	ppl7::WideString s1(L"The Quick Brown Fox Jumps over äöü");
	ppl7::WideString expected(L"");
	ASSERT_EQ(expected,s1.strrchr('L')) << "Unexpected Result";
}


}

