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
 * Copyright (c) 2010, Patrick Fedick <patrick@pfp.de>
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
#include "ppl7-tests.h"

namespace {

// The fixture for testing class Foo.
class StringTest : public ::testing::Test {
	protected:
	StringTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~StringTest() {

	}
};

TEST_F(StringTest, ConstructorSimple) {
	ASSERT_NO_THROW({
			ppl7::String s1;
			ASSERT_TRUE(s1.len()==0) << "String does not have zero length";
			const char *buf=s1.getPtr();
			ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
			ASSERT_EQ(0,buf[0]) << "Class did not return an empty c-string";

	}
	);
}

TEST_F(StringTest, ConstructorFromCharPtr) {
	ASSERT_NO_THROW({
			ppl7::String s1("A test string with unicode characters: äöü");
			ASSERT_EQ((size_t)45,s1.len()) << "String does not have length of 42";
			const unsigned char *buf=(const unsigned char *)s1.getPtr();
			ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
			ASSERT_EQ('A',(unsigned char)s1[0]) << "Unexpected Character in string";
			ASSERT_EQ(188,(unsigned char)s1[44]) << "Unexpected Character in string";

	}
	);
}

#ifdef DEPR
TEST_F(StringTest, ConstructorFromWideCharPtr) {
	ASSERT_NO_THROW({
			ppl7::String s1(L"A test string with unicode characters: äöü");
			ASSERT_TRUE(s1.len()==42) << "String does not length of 42";
			const char *buf=s1.getPtr();
			ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a widecharacter string";
			ASSERT_TRUE(L'A'==buf[0]) << "Class did not return an empty widecharacter string";
			ASSERT_TRUE(L'ü'==buf[41]) << "unexpected unicode character";

	}
	);
}
#endif

TEST_F(StringTest, ConstructorFromStringPtr) {
	ASSERT_NO_THROW({
		ppl7::String s1("A test string with unicode characters: äöü");
		ppl7::String s2(&s1);
		ASSERT_EQ((size_t)45,s2.len()) << "String does not have length of 45";
		const char *buf=s2.getPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ('A',(unsigned char)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(188,(unsigned char)s2[44]) << "Unexpected Character in string";
	}
	);
}




TEST_F(StringTest, ConstructorFromString) {
	ASSERT_NO_THROW({
		ppl7::String s1("A test string with unicode characters: äöü");
		ppl7::String s2(s1);
		ASSERT_EQ((size_t)45,s2.len()) << "String does not have length of 45";
		const char *buf=s2.getPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ('A',(unsigned char)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(188,(unsigned char)s2[44]) << "Unexpected Character in string";
	});
}

TEST_F(StringTest, ConstructorFromStdString) {
	ASSERT_NO_THROW({
		std::string s1("A test string with unicode characters: äöü");
		ppl7::String s2(s1);
		ASSERT_EQ((size_t)45,s2.len()) << "String does not have length of 45";
		const char *buf=s2.getPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ('A',(unsigned char)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(188,(unsigned char)s2[44]) << "Unexpected Character in string";
	});
}

TEST_F(StringTest, ConstructorFromStdWString) {
	ASSERT_NO_THROW({
		std::wstring s1(L"A test string with unicode characters: äöü");
		ASSERT_EQ((size_t)42,s1.size()) << "std:wstring has unexpected size";
		ppl7::String s2(s1);
		ASSERT_EQ((size_t)45,s2.len()) << "String does not have length of 45";
		const char *buf=s2.getPtr();
		ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
		ASSERT_EQ('A',(unsigned char)s2[0]) << "Unexpected Character in string";
		ASSERT_EQ(188,(unsigned char)s2[44]) << "Unexpected Character in string";
	});
}

TEST_F(StringTest, clear) {
	ppl7::String s1("A test string with unicode characters: äöü");
	s1.clear();
	ASSERT_EQ((size_t)0,s1.len()) << "String does not have length of 0";
	const char *buf=s1.getPtr();
	ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a c-string";
	ASSERT_THROW({
			ASSERT_EQ(0,(unsigned char)s1[0]) << "Unexpected Character in string";
	},ppl7::OutOfBoundsEception);
}

TEST_F(StringTest, CapacityAndReserve) {
	ppl7::String s1;
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



TEST_F(StringTest, len) {
	ppl7::String s1("A test string with unicode characters: äöü");
	ASSERT_EQ(s1.len(),(size_t)45) << "String has unexpected size";
}

TEST_F(StringTest, length) {
	ppl7::String s1("A test string with unicode characters: äöü");
	ASSERT_EQ(s1.length(),(size_t)45) << "String has unexpected size";
}

TEST_F(StringTest, size) {
	ppl7::String s1("A test string with unicode characters: äöü");
	ASSERT_EQ(s1.size(),(size_t)45) << "String has unexpected size";
}

TEST_F(StringTest, isEmpty) {
	ppl7::String s1("A test string with unicode characters: äöü");
	ppl7::String s2;
	ASSERT_EQ(s1.isEmpty(),false) << "String is empty";
	ASSERT_EQ(s2.isEmpty(),true) << "String not empty";
}

TEST_F(StringTest, notEmpty) {
	ppl7::String s1("A test string with unicode characters: äöü");
	ppl7::String s2;
	ASSERT_EQ(s1.notEmpty(),true) << "String is not empty";
	ASSERT_EQ(s2.notEmpty(),false) << "String is empty";
}

TEST_F(StringTest, isNumeric) {
	ppl7::String s1("A test string with unicode characters: äöü");
	ASSERT_EQ(s1.isNumeric(),false) << "String should not be numeric";
	s1.set("");
	ASSERT_EQ(s1.isNumeric(),false) << "String should not be numeric";
	s1.set("12345abcd");
	ASSERT_EQ(s1.isNumeric(),false) << "String should not be numeric";
	s1.set("12345");
	ASSERT_EQ(s1.isNumeric(),true) << "String should be numeric";
	s1.set("123.451,12");
	ASSERT_EQ(s1.isNumeric(),true) << "String should be numeric";
	s1.set("-123.451,12");
	ASSERT_EQ(s1.isNumeric(),true) << "String should be numeric";
	s1.set("-123.451,12-9");
	ASSERT_EQ(s1.isNumeric(),false) << "String should not be numeric";
}

TEST_F(StringTest, isInteger) {
	ppl7::String s1("A test string with unicode characters: äöü");
	ASSERT_EQ(s1.isInteger(),false) << "String should not be an integer";
	s1.set("");
	ASSERT_EQ(s1.isInteger(),false) << "String should not be an integer";
	s1.set("12345abcd");
	ASSERT_EQ(s1.isInteger(),false) << "String should not be an integer";
	s1.set("12345");
	ASSERT_EQ(s1.isInteger(),true) << "String should be an integer";
	s1.set("-12345");
	ASSERT_EQ(s1.isInteger(),true) << "String should be an integer";
	s1.set("123-451");
	ASSERT_EQ(s1.isInteger(),false) << "String should be an integer";
	s1.set("123.451,12");
	ASSERT_EQ(s1.isInteger(),false) << "String should be an integer";
	s1.set("-123.451,12");
	ASSERT_EQ(s1.isInteger(),false) << "String should be an integer";
	s1.set("-123.451,12-9");
	ASSERT_EQ(s1.isInteger(),false) << "String should not be an integer";
}

TEST_F(StringTest, isTrue) {
	ppl7::String s1("A test string with unicode characters: äöü");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set("");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set("12345abcd");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set("1");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set("12345");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set("true");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set("wahr");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set("ja");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set("yes");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set("false");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set("falsch");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set("nein");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set("no");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
}

TEST_F(StringTest, isFalse) {
	ppl7::String s1("A test string with unicode characters: äöü");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set("");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set("12345abcd");
	ASSERT_EQ(s1.isFalse(),false) << "String should not be false";
	s1.set("12345");
	ASSERT_EQ(s1.isFalse(),false) << "String should not be false";
	s1.set("0");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set("true");
	ASSERT_EQ(s1.isFalse(),false) << "String should not be false";
	s1.set("wahr");
	ASSERT_EQ(s1.isFalse(),false) << "String should not be false";
	s1.set("ja");
	ASSERT_EQ(s1.isFalse(),false) << "String should not be false";
	s1.set("yes");
	ASSERT_EQ(s1.isFalse(),false) << "String should not be false";
	s1.set("false");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set("falsch");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set("nein");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set("no");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
}


TEST_F(StringTest, setConstCharWithoutSize) {
	ppl7::String s2("äöü, a test string with unicode characters");
	ppl7::String s1;
	s1.set("äöü, a test string with unicode characters");
	ASSERT_EQ(s1,s2) << "String has unexpected value";
	ASSERT_EQ((size_t)45,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setConstCharWithSize) {
	ppl7::String s2("äöü, a tes");
	ppl7::String s1;
	s1.set("äöü, a test string with unicode characters",13);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)13,s1.size()) << "String has unexpected length";
}


TEST_F(StringTest, setConstWChartWithoutSize) {
	ppl7::String s2("äöü, a test string with unicode characters");
	ppl7::String s1;
	s1.set("äöü, a test string with unicode characters");
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)45,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setConstWChartWithSize) {
	ppl7::String s2("äöü, a tes");
	ppl7::String s1;
	s1.set("äöü, a test string with unicode characters",13);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)13,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setStringPtrWithoutSize) {
	ppl7::String s2("äöü, a test string with unicode characters");
	ppl7::String s1;
	s1.set(&s2);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)45,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setStringPtrWithSize) {
	ppl7::String s2("äöü, a tes");
	ppl7::String s3("äöü, a test string with unicode characters");
	ppl7::String s1;
	s1.set(&s3,13);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)13,s1.size()) << "String has unexpected length";
}


TEST_F(StringTest, setStringRefWithoutSize) {
	ppl7::String s2("äöü, a test string with unicode characters");
	ppl7::String s1;
	s1.set(s2);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)45,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setStringRefWithSize) {
	ppl7::String s2("äöü, a tes");
	ppl7::String s3("äöü, a test string with unicode characters");
	ppl7::String s1;
	s1.set(s3,13);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)13,s1.size()) << "String has unexpected length";
}


TEST_F(StringTest, setSTDStringRefWithoutSize) {
	ppl7::String s2("äöü, a test string with unicode characters");
	std::string s3("äöü, a test string with unicode characters");
	ppl7::String s1;
	s1.set(s3);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)45,s1.size()) << "String has unexpected length";
}



TEST_F(StringTest, setSTDStringRefWithSize) {
	ppl7::String s2("äöü, a tes");
	std::string s3("äöü, a test string with unicode characters");
	ppl7::String s1;
	s1.set(s3,13);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)13,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setSTDWStringRefWithoutSize) {
	ppl7::String s2("äöü, a test string with unicode characters");
	std::wstring s3(L"äöü, a test string with unicode characters");
	ppl7::String s1;
	s1.set(s3);
	ASSERT_EQ(s1,s2) << "String as unexpected value";
	ASSERT_EQ((size_t)45,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setSTDWStringRefWithSize) {
	ppl7::String s2("äöü, a tes");
	std::wstring s3(L"äöü, a test string with unicode characters");
	ppl7::String s1;
	s1.set(s3,10);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)13,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setf) {
	ppl7::String s2("Ein Test, 42, Wide, 10000");
	ppl7::String s1;
	s1.setf("Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)25,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, setWchart) {
	ppl7::String s2("a");
	ppl7::String s1;
	s1.set((wchar_t)'a');
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)1,s1.size()) << "String has unexpected length";
}

static void test_vasprintf(ppl7::String &str,const char *fmt,...)
{
	va_list args;
	va_start(args, fmt);
	str.vasprintf(fmt,args);
	va_end(args);
}


TEST_F(StringTest, vasprintf) {
	ppl7::String s2("Ein Test, 42, Wide, 10000");
	ppl7::String s1;
	test_vasprintf(s1,"Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)25,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendConstWchartWithoutSize) {
	ppl7::String expected("First Part äöü, äöü Second Part");
	ppl7::String s1("First Part äöü, ");
	s1.append("äöü Second Part");

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendConstWchartWithSize) {
	ppl7::String expected("First Part äöü, äöü S");
	ppl7::String s1("First Part äöü, ");
	s1.append(L"äöü Second Part",5);

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)27,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendConstCharPtrWithoutSize) {
	ppl7::String expected("First Part äöü, äöü Second Part");
	ppl7::String s1("First Part äöü, ");
	s1.append("äöü Second Part");

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendConstCharPtrWithSize) {
	ppl7::String expected("First Part äöü, äöü S");
	ppl7::String s1("First Part äöü, ");
	s1.append("äöü Second Part",8);

	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)27,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendStringPtrWithoutSize) {
	ppl7::String expected("First Part äöü, äöü Second Part");
	ppl7::String s1("First Part äöü, ");
	ppl7::String s2("äöü Second Part");
	s1.append(&s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendStringPtrWithSize) {
	ppl7::String expected("First Part äöü, äöü S");
	ppl7::String s1("First Part äöü, ");
	ppl7::String s2("äöü Second Part");
	s1.append(&s2,8);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)27,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendStringWithoutSize) {
	ppl7::String expected("First Part äöü, äöü Second Part");
	ppl7::String s1("First Part äöü, ");
	ppl7::String s2("äöü Second Part");
	s1.append(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendStringWithSize) {
	ppl7::String expected("First Part äöü, äöü S");
	ppl7::String s1("First Part äöü, ");
	ppl7::String s2("äöü Second Part");
	s1.append(s2,8);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)27,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendStdStringWithoutSize) {
	ppl7::String expected("First Part äöü, äöü Second Part");
	ppl7::String s1("First Part äöü, ");
	std::string s2("äöü Second Part");
	s1.append(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendStdStringWithSize) {
	ppl7::String expected("First Part äöü, äöü S");
	ppl7::String s1("First Part äöü, ");
	std::string s2("äöü Second Part");
	s1.append(s2,8);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)27,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendStdWStringWithoutSize) {
	ppl7::String expected("First Part äöü, äöü Second Part");
	ppl7::String s1("First Part äöü, ");
	std::wstring s2(L"äöü Second Part");
	s1.append(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendStdWStringWithSize) {
	ppl7::String expected("First Part äöü, äöü Seco");
	ppl7::String s1("First Part äöü, ");
	std::wstring s2(L"äöü Second Part");
	s1.append(s2,8);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)30,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendf) {
	ppl7::String expected("First Part äöü, Ein Test, 42, Wide, 10000");
	ppl7::String s1("First Part äöü, ");
	s1.appendf("Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)44,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, appendWchart) {
	ppl7::String expected("First Part äöü, a");
	ppl7::String s1("First Part äöü, ");
	s1.append((wchar_t)'a');
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)20,s1.size()) << "String has unexpected length";
}


TEST_F(StringTest, prependConstWchartWithoutSize) {
	ppl7::String expected("äöü Second PartFirst Part äöü, ");
	ppl7::String s1("First Part äöü, ");
	s1.prepend("äöü Second Part");
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependConstWchartWithSize) {
	ppl7::String expected("äöü SFirst Part äöü, ");
	ppl7::String s1("First Part äöü, ");
	s1.prepend("äöü Second Part",8);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)27,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependConstCharPtrWithoutSize) {
	ppl7::String expected("äöü Second PartFirst Part äöü, ");
	ppl7::String s1("First Part äöü, ");
	s1.prepend("äöü Second Part");
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependConstCharPtrWithSize) {
	ppl7::String expected("äöü SFirst Part äöü, ");
	ppl7::String s1("First Part äöü, ");
	s1.prepend("äöü Second Part",8);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)27,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependStringPtrWithoutSize) {
	ppl7::String expected("äöü Second PartFirst Part äöü, ");
	ppl7::String s1("First Part äöü, ");
	ppl7::String s2("äöü Second Part");
	s1.prepend(&s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependStringPtrWithSize) {
	ppl7::String expected("äöü SFirst Part äöü, ");
	ppl7::String s1("First Part äöü, ");
	ppl7::String s2("äöü Second Part");
	s1.prepend(&s2,8);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)27,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependStringWithoutSize) {
	ppl7::String expected("äöü Second PartFirst Part äöü, ");
	ppl7::String s1("First Part äöü, ");
	ppl7::String s2("äöü Second Part");
	s1.prepend(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependStringWithSize) {
	ppl7::String expected("äöü SFirst Part äöü, ");
	ppl7::String s1("First Part äöü, ");
	ppl7::String s2("äöü Second Part");
	s1.prepend(s2,8);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)27,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependStdStringWithoutSize) {
	ppl7::String expected("äöü Second PartFirst Part äöü, ");
	ppl7::String s1("First Part äöü, ");
	std::string s2("äöü Second Part");
	s1.prepend(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependStdStringWithSize) {
	ppl7::String expected("äöü SFirst Part äöü, ");
	ppl7::String s1("First Part äöü, ");
	std::string s2("äöü Second Part");
	s1.prepend(s2,8);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)27,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependStdWStringWithoutSize) {
	ppl7::String expected("äöü Second PartFirst Part äöü, ");
	ppl7::String s1("First Part äöü, ");
	std::wstring s2(L"äöü Second Part");
	s1.prepend(s2);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)37,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependStdWStringWithSize) {
	ppl7::String expected("äöü SFirst Part äöü, ");
	ppl7::String s1("First Part äöü, ");
	std::wstring s2(L"äöü Second Part");
	s1.prepend(s2,5);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)27,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependf) {
	ppl7::String expected("Ein Test, 42, Wide, 10000First Part äöü, ");
	ppl7::String s1("First Part äöü, ");
	s1.prependf("Ein %s, %i, %ls, %u","Test",42,L"Wide",10000);
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)44,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, prependWchart) {
	ppl7::String expected("aFirst Part äöü, ");
	ppl7::String s1("First Part äöü, ");
	s1.prepend((wchar_t)'a');
	ASSERT_EQ(expected,s1) << "String has unexpected value";
	ASSERT_EQ((size_t)20,s1.size()) << "String has unexpected length";
}

TEST_F(StringTest, chopRight) {
	ppl7::String s1("A test string with unicode characters: äöü");
	ppl7::String s2("A test string with unicode characters: ä");
	ppl7::String s3("A test string with unicode characters: ");
	s1.chopRight(4);
	ASSERT_EQ((size_t)41,s1.len()) << "String does not have expected length";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	s1.chopRight();
	s1.chopRight();
	ASSERT_EQ((size_t)39,s1.len()) << "String does not have expected length";
	ASSERT_EQ(s3,s1) << "String has unexpected value";
	s1.chopRight(39);
	ASSERT_EQ((size_t)0,s1.len()) << "String does not have length of 0";
	s2.clear();
	ASSERT_EQ(s2,s1) << "String has unexpected value";
}

TEST_F(StringTest, chopLeft) {
	ppl7::String s1("A test string with unicode characters: äöü");
	ppl7::String s2("est string with unicode characters: äöü");
	ppl7::String s3("st string with unicode characters: äöü");
	s1.chopLeft(3);
	ASSERT_EQ((size_t)42,s1.len()) << "String does not have expected length";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	s1.chopLeft();
	ASSERT_EQ((size_t)41,s1.len()) << "String does not have expected length";
	ASSERT_EQ(s3,s1) << "String has unexpected value";
	s1.chopRight(41);
	ASSERT_EQ((size_t)0,s1.len()) << "String does not have length of 0";
	s2.clear();
	ASSERT_EQ(s2,s1) << "String has unexpected value";
}

TEST_F(StringTest, chop) {
	ppl7::String s1("A test string with unicode characters: äöü");
	ppl7::String s2("A test string with unicode characters: ä");
	ppl7::String s3("A test string with unicode characters: ");
	s1.chop(4);
	ASSERT_EQ((size_t)41,s1.len()) << "String does not have length of 40";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	s1.chop();
	s1.chop();
	ASSERT_EQ((size_t)39,s1.len()) << "String does not have length of 39";
	ASSERT_EQ(s3,s1) << "String has unexpected value";
	s1.chop(39);
	ASSERT_EQ((size_t)0,s1.len()) << "String does not have length of 0";
	s2.clear();
	ASSERT_EQ(s2,s1) << "String has unexpected value";
}

TEST_F(StringTest, chomp) {
	ppl7::String s1("\n\r\nA test string\näöü\n\r\n");
	ppl7::String s2("A test string\näöü");
	s1.chomp();
	ASSERT_EQ((size_t)20,s1.len()) << "String does not have length of 17";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
	s1.chomp();
	ASSERT_EQ((size_t)20,s1.len()) << "String does not have length of 17";
	ASSERT_EQ(s2,s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithPos) {
	ppl7::String s1("The quick brown fox jumps over the lazy dog");
	ppl7::String expected("The quick ");
	s1.cut(10);
	ASSERT_EQ((size_t)10,s1.len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithPos0) {
	ppl7::String s1("The quick brown fox jumps over the lazy dog");
	ppl7::String expected("");
	s1.cut(0);
	ASSERT_EQ((size_t)0,s1.len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithPosBeyondStringLength) {
	ppl7::String s1("The quick brown fox jumps over the lazy dog");
	ppl7::String expected("The quick brown fox jumps over the lazy dog");
	s1.cut(100);
	ASSERT_EQ((size_t)43,s1.len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithLetter) {
	ppl7::String s1("The quick brown fox jumps over the lazy dog");
	ppl7::String expected("The quick ");
	s1.cut("b");
	ASSERT_EQ((size_t)10,s1.len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithNonexistingLetter) {
	ppl7::String s1("The quick brown fox");
	ppl7::String expected("The quick brown fox");
	s1.cut("j");
	ASSERT_EQ((size_t)19,s1.len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}

TEST_F(StringTest, cut_WithString) {
	ppl7::String s1("The quick brown fox jumps over the lazy dog");
	ppl7::String expected("The quick ");
	s1.cut("brown");
	ASSERT_EQ((size_t)10,s1.len()) << "String has unexpected length";
	ASSERT_EQ(expected,s1) << "String has unexpected value";
}


TEST_F(StringTest, strstr) {
	ppl7::String s1("A test haystack string");
	ppl7::String s3("haystack string");
	ppl7::String s2;
	s2=s1.strstr("haystack");
	ASSERT_EQ((size_t)15,s2.len()) << "String does not have length of 15";
	ASSERT_EQ(s3,s2) << "String has unexpected value";
	s2=s1.strstr("beer");
	s3="";
	ASSERT_EQ((size_t)0,s2.len()) << "String does not have length of 0";
	ASSERT_EQ(s3,s2) << "String has unexpected value";

	s2=s1.strstr("");
	ASSERT_EQ(s2,s1) << "String has unexpected value";

}

TEST_F(StringTest, repeated) {
	ppl7::String s1("_repeat_");
	ppl7::String s3("_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
	ppl7::String s2;
	s2=s1.repeated(10);
	ASSERT_EQ((size_t)80,s2.len()) << "String does not have length of 80";
	ASSERT_EQ(s3,s2) << "String has unexpected value";
}

TEST_F(StringTest, trimLeft) {
	ppl7::String s1("\n\n    abc  \n");
	s1.trimLeft();
	ASSERT_EQ((size_t)6,s1.size());
	ASSERT_EQ(ppl7::String("abc  \n"),s1);
}

TEST_F(StringTest, trimRight) {
	ppl7::String s1(" \n  abc  \n");
	s1.trimRight();
	ASSERT_EQ((size_t)7,s1.size());
	ASSERT_EQ(ppl7::String(" \n  abc"),s1);
}

TEST_F(StringTest, trim) {
	ppl7::String s1("\n\n    abc  \n");
	s1.trim();
	ASSERT_EQ((size_t)3,s1.size());
	ASSERT_EQ(ppl7::String("abc"),s1);
}

TEST_F(StringTest, trimmed) {
	ppl7::String s1("\n\n    abc  \n");
	ppl7::String s2=s1.trimmed();
	ASSERT_EQ((size_t)3,s2.size());
	ASSERT_EQ(ppl7::String("abc"),s2);
}


TEST_F(StringTest, trimLeftEmptyResult) {
	ppl7::String s1("\n\n   \n   \n");
	s1.trimLeft();
	ASSERT_EQ((size_t)0,s1.size());
	ASSERT_EQ(ppl7::String(""),s1);
}

TEST_F(StringTest, trimRightEmptyResult) {
	ppl7::String s1("\n\n   \n   \n");
	s1.trimRight();
	ASSERT_EQ((size_t)0,s1.size());
	ASSERT_EQ(ppl7::String(""),s1);
}

TEST_F(StringTest, trimEmptyResult) {
	ppl7::String s1("\n\n   \n   \n");
	s1.trim();
	ASSERT_EQ((size_t)0,s1.size());
	ASSERT_EQ(ppl7::String(""),s1);
}

TEST_F(StringTest, trimLeftChars) {
	ppl7::String s1("\n\n    abc  \n");
	s1.trimLeft(" \n");
	ASSERT_EQ((size_t)6,s1.size());
	ASSERT_EQ(ppl7::String("abc  \n"),s1);
}

TEST_F(StringTest, trimRightChars) {
	ppl7::String s1(" \n  abc  \n");
	s1.trimRight(" \n");
	ASSERT_EQ((size_t)7,s1.size());
	ASSERT_EQ(ppl7::String(" \n  abc"),s1);
}

TEST_F(StringTest, trimChars) {
	ppl7::String s1("\n\n    abc  \n");
	s1.trim(" \n");
	ASSERT_EQ((size_t)3,s1.size());
	ASSERT_EQ(ppl7::String("abc"),s1);
}

TEST_F(StringTest, trimLeftCharsEmptyResult) {
	ppl7::String s1("\n\n   \n   \n");
	s1.trimLeft(" \n");
	ASSERT_EQ((size_t)0,s1.size());
	ASSERT_EQ(ppl7::String(""),s1);
}

TEST_F(StringTest, trimRightCharsEmptyResult) {
	ppl7::String s1("\n\n   \n   \n");
	s1.trimRight(" \n");
	ASSERT_EQ((size_t)0,s1.size());
	ASSERT_EQ(ppl7::String(""),s1);
}

TEST_F(StringTest, trimEmptyCharsResult) {
	ppl7::String s1("\n\n   \n   \n");
	s1.trim(" \n");
	ASSERT_EQ((size_t)0,s1.size());
	ASSERT_EQ(ppl7::String(""),s1);
}

TEST_F(StringTest, operatorEqualEmpty) {
	ppl7::String s1("");
	ppl7::String s2("");
	ASSERT_EQ(s1,s2);
}

TEST_F(StringTest, strcmpEmpty) {
	ppl7::String s1("");
	ppl7::String s2("");
	ASSERT_EQ(0,s1.strcmp(s2));
}

TEST_F(StringTest, strcmpLower) {
	ppl7::String s1("ABcdef");
	ppl7::String s2("Defghi");
	ASSERT_LT(s1.strcmp(s2),0);
}

TEST_F(StringTest, strcmpLowerWithCase) {
	ppl7::String s1("ABcdef");
	ppl7::String s2("abcdef");
	ASSERT_LT(s1.strcmp(s2),0);
}

TEST_F(StringTest, strcmpHigher) {
	ppl7::String s1("ABcdef");
	ppl7::String s2("Defghi");
	ASSERT_GT(s2.strcmp(s1),0);
}

TEST_F(StringTest, strcmpHigherWithCase) {
	ppl7::String s1("ABcdef");
	ppl7::String s2("abcdef");
	ASSERT_GT(s2.strcmp(s1),0);
}

TEST_F(StringTest, strcmpEqual) {
	ppl7::String s1("ABcdef");
	ppl7::String s2("ABcdef");
	ASSERT_EQ(s2.strcmp(s1),0);
}

TEST_F(StringTest, strcasecmpEmpty) {
	ppl7::String s1("");
	ppl7::String s2("");
	ASSERT_EQ(0,s1.strcasecmp(s2));
}

TEST_F(StringTest, strcasecmpLower) {
	ppl7::String s1("ABcdef");
	ppl7::String s2("Defghi");
	ASSERT_LT(s1.strcasecmp(s2),0);
}

TEST_F(StringTest, strcasecmpLowerWithCase) {
	ppl7::String s1("ABcdef");
	ppl7::String s2("abcdef");
	ASSERT_EQ(s1.strcasecmp(s2),0);
}

TEST_F(StringTest, strcasecmpHigher) {
	ppl7::String s1("ABcdef");
	ppl7::String s2("Defghi");
	ASSERT_GT(s2.strcasecmp(s1),0);
}

TEST_F(StringTest, strcasecmpHigherWithCase) {
	ppl7::String s1("ABcdef");
	ppl7::String s2("abcdef");
	ASSERT_EQ(s2.strcasecmp(s1),0);
}

TEST_F(StringTest, strcasecmpEqual) {
	ppl7::String s1("ABcdef");
	ppl7::String s2("ABcdef");
	ASSERT_EQ(s2.strcasecmp(s1),0);
}


TEST_F(StringTest, left) {
	ppl7::String s1("The quick brown fox jumps over the lazy dog");
	ppl7::String s2=s1.left(10);
	ASSERT_EQ(ppl7::String("The quick "),s2);
}

TEST_F(StringTest, right) {
	ppl7::String s1("The quick brown fox jumps over the lazy dog");
	ppl7::String s2=s1.right(9);
	ASSERT_EQ(ppl7::String(" lazy dog"),s2);
}

TEST_F(StringTest, midWithLength) {
	ppl7::String s1("The quick brown fox jumps over the lazy dog");
	ppl7::String s2=s1.mid(10,10);
	ASSERT_EQ(ppl7::String("brown fox "),s2);
}

TEST_F(StringTest, midWithoutLength) {
	ppl7::String s1("The quick brown fox jumps over the lazy dog");
	ppl7::String s2=s1.mid(10);
	ASSERT_EQ(ppl7::String("brown fox jumps over the lazy dog"),s2);
}


TEST_F(StringTest, substrWithLength) {
	ppl7::String s1("The quick brown fox jumps over the lazy dog");
	ppl7::String s2=s1.substr(10,10);
	ASSERT_EQ(ppl7::String("brown fox "),s2);
}

TEST_F(StringTest, substrWithoutLength) {
	ppl7::String s1("The quick brown fox jumps over the lazy dog");
	ppl7::String s2=s1.substr(10);
	ASSERT_EQ(ppl7::String("brown fox jumps over the lazy dog"),s2);
}

TEST_F(StringTest, lowerCase) {
	ppl7::String s1("The Quick Brown Fox Jumps over ÄÖÜ");
	ppl7::String expected("the quick brown fox jumps over äöü");
	ASSERT_NO_THROW(s1.lowerCase());
	ASSERT_EQ(expected,s1);
}

TEST_F(StringTest, upperCase) {
	ppl7::String s1("The Quick Brown Fox Jumps over äöü");
	ppl7::String expected("THE QUICK BROWN FOX JUMPS OVER ÄÖÜ");
	ASSERT_NO_THROW(s1.upperCase());
	ASSERT_EQ(expected,s1);
}

TEST_F(StringTest, upperCaseWords) {
	ppl7::String s1("the quick brown fox jumps over äöü");
	ppl7::String expected("The Quick Brown Fox Jumps Over Äöü");
	ASSERT_NO_THROW(s1.upperCaseWords());
	ASSERT_EQ(expected,s1);
}


TEST_F(StringTest, pregMatchPositive) {
	ppl7::String s1("Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
	ppl7::String expr("/^Lorem.*$/s");
	ASSERT_TRUE(s1.pregMatch(expr));
	expr.set("/^Lorem.*$/is");
	ASSERT_TRUE(s1.pregMatch(expr));
	expr.set("/consectetuer/");
	ASSERT_TRUE(s1.pregMatch(expr));
	expr.set("/^.*consectetuer.*$/s");
	ASSERT_TRUE(s1.pregMatch(expr));
	expr.set("/^.*mus\\.$/m");
	ASSERT_TRUE(s1.pregMatch(expr));
}

TEST_F(StringTest, pregMatchNegativ) {
	ppl7::String s1("Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
	ppl7::String expr("/^Looorem.*$/s");
	ASSERT_FALSE(s1.pregMatch(expr));
	expr.set("/^ipsum.*$/is");
	ASSERT_FALSE(s1.pregMatch(expr));
	expr.set("/patrick/");
	ASSERT_FALSE(s1.pregMatch(expr));
	expr.set("/^.*patrick.*$/s");
	ASSERT_FALSE(s1.pregMatch(expr));
	expr.set("/^.*mus\\.$/");
	ASSERT_FALSE(s1.pregMatch(expr));
}

TEST_F(StringTest, pregCapture) {
	ppl7::Array m;
	ppl7::String s1("2012-05-18");
	ASSERT_TRUE(s1.pregMatch("/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})$/",m));
	ASSERT_EQ(2012,m[1].toInt()) << "Unexpected value in capture";
	ASSERT_EQ(5,m[2].toInt()) << "Unexpected value in capture";
	ASSERT_EQ(18,m[3].toInt()) << "Unexpected value in capture";
	ASSERT_EQ((size_t)4,m.size()) << "Unexpected number auf captures";

}



TEST_F(StringTest, Utf8toUtf8) {
	ASSERT_NO_THROW({
		ppl7::String s1("A test string with unicode characters: äöü");
		ppl7::ByteArray a=s1.toUtf8();
		ASSERT_EQ((size_t)45,a.size()) << "String does not have expected length";
		ASSERT_EQ((unsigned char)'A',(unsigned char)a.get(0)) << "Unexpected Character in string";
		ASSERT_EQ((unsigned char)188,(unsigned char)a.get(44)) << "Unexpected Character in string";
	});
}

TEST_F(StringTest, ISO88591toUtf8) {
	if (setlocale(LC_ALL,"de_DE.ISO8859-1")==NULL) {
		FAIL() << "setlocale fehlgeschlagen\n";
	}
	ppl7::String s1;
	ASSERT_NO_THROW({
		s1.set("A test string with unicode characters: ");
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

TEST_F(StringTest, strchr_ExistingChar) {
	ppl7::String s1("The Quick Brown Fox Jumps over äöü");
	ppl7::String expected("Fox Jumps over äöü");
	ASSERT_EQ(expected,s1.strchr('F')) << "Unexpected Result";
}

TEST_F(StringTest, strchr_NonExistingChar) {
	ppl7::String s1("The Quick Brown Fox Jumps over äöü");
	ppl7::String expected("");
	ASSERT_EQ(expected,s1.strchr('L')) << "Unexpected Result";
}

TEST_F(StringTest, strrchr_ExistingChar) {
	ppl7::String s1("The Quick Brown Fox Jumps over äöü");
	ppl7::String expected("over äöü");
	ASSERT_EQ(expected,s1.strrchr('o')) << "Unexpected Result";
}

TEST_F(StringTest, strrchr_NonExistingChar) {
	ppl7::String s1("The Quick Brown Fox Jumps over äöü");
	ppl7::String expected("");
	ASSERT_EQ(expected,s1.strrchr('L')) << "Unexpected Result";
}


}


#ifdef old
	printf ("sizeof String: %zd\n",sizeof(ppl7::String));
	if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
		printf ("setlocale fehlgeschlagen\n");
		return 1;
	}

	try {
		//ppl7::String::setGlobalEncoding("iso-8859-1");
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	catch (ppl7::NullPointerException) {
		printf ("NPE\n");
		return 1;
	}
	catch (ppl7::UnsupportedFeatureException) {
		printf ("Transkodierung wird nicht unterstützt\n");
		return 1;
	}
	catch (ppl7::UnsupportedCharacterEncodingException) {
		printf ("UnsupportedFeatureException: Transkodierung wird nicht unterstützt\n");
		return 1;
	}
	catch(...) {
		printf ("Unbekannte Exception\n");
		throw;
	}

	ppl7::String s1;
	s1.reserve(128);

	s1.setf("Test mit UTF-8 Umlaütän ß!");
	printf("capacity: %zd, length: %zd\n",s1.capacity(),s1.length());

	//s1.set(L"Test mit ISO-8859-1 Umla�t�n �!");
	s1.printnl();

	s1.append("Noch mehr UTF-8");
	printf("capacity: %zd, length: %zd\n",s1.capacity(),s1.length());
	s1.printnl();

	s1.appendf("capacity: %zd\n",s1.capacity());

	printf("capacity: %zd, length: %zd\n",s1.capacity(),s1.length());
	s1.printnl();

	//printf ("UTF-8 String als const char*: >>>%s<<<\n",(const char*)s1.toUtf8());

	printf ("prepend\n");
	s1.prependf("Davor :-)  =>");
	s1.printnl();

	printf ("ok\n");

	s1=".String1.";
	ppl7::String s2;
	//s2=s1+"Hallo Welt"+s1+"Noch mehr"+s1;
	s2=s1+"Hallo Welt"+s1+"Noch mehr";
	s2.printnl();

	ppl7::String s3="Eiñ Täśt zµm Kôdiéren";
	ppl7::ByteArray b=s3.toEncoding("UTF-8");
	b.hexDump();

	printf ("left: %ls\n",(const wchar_t*)s3.left(3));
	printf ("right: %ls\n",(const wchar_t*)s3.right(4));
	printf ("mid (2): %ls\n",(const wchar_t*)s3.mid(2));
	printf ("mid (2,5): %ls\n",(const wchar_t*)s3.mid(2,5));
	printf ("mid (40,5): %ls\n",(const wchar_t*)s3.mid(40,5));

	//s1.hexDump();
	return 0;
}
#endif
