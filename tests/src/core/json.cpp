/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
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
class JsonTest : public ::testing::Test {
	protected:
	JsonTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~JsonTest() {

	}
};

TEST_F(JsonTest, ParseFromStringWithDictToAssocArray) {
	ppl7::String text;
	ppl7::File::load(text,"testdata/jsontest1.json");

	ppl7::AssocArray data;
	try {
		ppl7::Json::loads(data,text);
	} catch (const ppl7::Exception &exp) {
		exp.print();
		FAIL() << "unexpected exception occured";

	}
	/*
	ppl7::AssocArray::const_iterator it;
	for (it=data.begin();it!=data.end();++it) {
		printf ("key=>>%s<<\n",(const char*)(*it).first);
	}
	*/
	EXPECT_EQ((size_t)15,data.size());
	EXPECT_EQ(ppl7::String("Root Value"),data["RootKey"]);
	EXPECT_EQ(ppl7::String("newline\n, tab\ttab, backs\\ash"),data["EscapeSecences"]);
	EXPECT_EQ(ppl7::String("12345"),data["integer"]);
	EXPECT_EQ(ppl7::String("17.999"),data["float"]);
	EXPECT_TRUE(data.exists("dict"));
	EXPECT_EQ(ppl7::String("value1"),data["dict/innerdict_key1"]);
	EXPECT_EQ(ppl7::String("value2"),data["dict/key2"]);
	EXPECT_TRUE(data.exists("empty_array"));
	EXPECT_TRUE(data.exists("empty_dict"));
	EXPECT_TRUE(data.exists("empty_string"));
	EXPECT_EQ(ppl7::String(""),data["empty_string"]);
	EXPECT_EQ(ppl7::String("String1"),data["array_same_line/0"]);
	EXPECT_EQ(ppl7::String("String2"),data["array_same_line/1"]);
	EXPECT_EQ(ppl7::String("String3"),data["array_same_line/2"]);
	EXPECT_EQ(ppl7::String("String1"),data["array_multiline/0"]);
	EXPECT_EQ(ppl7::String("String1"),data["array_multiline/0"]);
	EXPECT_EQ(ppl7::String("String2"),data["array_multiline/1"]);
	EXPECT_EQ(ppl7::String("22.443"),data["array_multiline/2"]);
	EXPECT_EQ(ppl7::String("schachtel1"),data["array_multiline/3/0"]);
	EXPECT_EQ(ppl7::String("schachtel2"),data["array_multiline/3/1"]);
	EXPECT_EQ(ppl7::String("value1"),data["array_multiline/4/schachteldict1"]);
	EXPECT_EQ(ppl7::String("value2"),data["array_multiline/4/schachteldict2"]);
	EXPECT_EQ(ppl7::String("12345"),data["array_multiline/5"]);
	EXPECT_EQ(ppl7::String("Dieser String geht über\nmehrere Zeilen."),data["Zeilenumbruch"]);
	EXPECT_EQ(ppl7::String("value2"),data["key2"]);
	EXPECT_EQ(ppl7::String("true"),data["true"]);
	EXPECT_EQ(ppl7::String("false"),data["false"]);
	EXPECT_EQ(ppl7::String("null"),data["null"]);

	//data.list();
}

TEST_F(JsonTest, ParseFromStringWithArrayToAssocArray) {
	ppl7::String text;
	ppl7::File::load(text,"testdata/jsontest2.json");

	ppl7::AssocArray data;
	try {
		ppl7::Json::loads(data,text);
	} catch (const ppl7::Exception &exp) {
		exp.print();
		FAIL() << "unexpected exception occured";
	}
	EXPECT_EQ((size_t)3,data.size());
	EXPECT_EQ(ppl7::String("value1"),data["0"]);
	EXPECT_EQ(ppl7::String("inner_value1"),data["1/key1"]);
	EXPECT_EQ(ppl7::String("inner_value2"),data["1/key2"]);
	EXPECT_EQ(ppl7::String("value3"),data["2"]);
}

TEST_F(JsonTest, NegativTest_GarbageBeforeBegin) {
	ppl7::String text;
	ppl7::File::load(text,"testdata/jsontest3.json");

	ppl7::AssocArray data;
	ASSERT_THROW(ppl7::Json::loads(data,text),ppl7::UnexpectedCharacterException);
}
TEST_F(JsonTest, NegativTest_GarbageAfterEnd) {
	ppl7::String text;
	ppl7::File::load(text,"testdata/jsontest4.json");
	ppl7::AssocArray data;
	ASSERT_THROW(ppl7::Json::loads(data,text),ppl7::UnexpectedCharacterException);
}
TEST_F(JsonTest, NegativTest_InvalidKey) {
	ppl7::String text("{ \"key1\": \"value1\", must_fail: \"value2\", \"key3\": \"value3\"}");
	ppl7::AssocArray data;
	ASSERT_THROW(ppl7::Json::loads(data,text),ppl7::UnexpectedCharacterException);
}

TEST_F(JsonTest, NegativTest_MissingValue) {
	ppl7::String text("{ \"key1\": \"value1\", \"key2\":, \"key3\": \"value3\"}");
	ppl7::AssocArray data;
	ASSERT_THROW(ppl7::Json::loads(data,text),ppl7::UnexpectedCharacterException);
}




}

