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
class PythonHelperTest : public ::testing::Test {
	protected:
	PythonHelperTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen: LC_CTYPE\n");
			throw std::exception();
		}
		if (setlocale(LC_TIME,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen: LC_TIME\n");
			throw std::exception();
		}
	}
	virtual ~PythonHelperTest() {

	}
};

TEST_F(PythonHelperTest, escapeString) {
	ppl7::String res;

	ASSERT_NO_THROW ({
		res=ppl7::PythonHelper::escapeString("Emailadresse: \"test@test.de\"");
	});
	ASSERT_EQ(ppl7::String("Emailadresse: \\\"test@test.de\\\""),res);
}

TEST_F(PythonHelperTest, escapeStringWithBackslashes) {
	ppl7::String res;

	ASSERT_NO_THROW ({
		res=ppl7::PythonHelper::escapeString("test\\,test");
	});
	ASSERT_EQ(ppl7::String("test\\\\,test"),res);
}

TEST_F(PythonHelperTest, toHashWithEmptyArray) {
	ppl7::AssocArray a;
	ppl7::String res;

	ASSERT_NO_THROW ({
		res=ppl7::PythonHelper::toHash(a,"key");
	});
	ASSERT_EQ(ppl7::String("key = {}\n"),res);
}

TEST_F(PythonHelperTest, toHashWithEmptyArrayEmptyKey) {
	ppl7::AssocArray a;
	ppl7::String res;
	ASSERT_NO_THROW ({
		res=ppl7::PythonHelper::toHash(a,"");
	});
	ASSERT_EQ(ppl7::String(""),res);
}

TEST_F(PythonHelperTest, toHashNonRecursive) {
	ppl7::AssocArray a;
	ppl7::String res;
	ppl7::String expected="key = {\n"
			"    \"key1\": \"value1\",\n"
			"    \"key2\": 20,\n"
			"    \"key3\": \"Geht\\nueber\\nmehrere\\nZeilen\\n\",\n"
			"    \"key4\": True,\n"
			"    \"key5\": False,\n"
			"    \"key6\": None,\n"
			"    \"key7\": -17.22334\n"
			"}\n";

	a.set("key1","value1");
	a.set("key2","20");
	a.set("key3","Geht\nueber\nmehrere\nZeilen\n");
	a.set("key4","true");
	a.set("key5","false");
	a.set("key6","none");
	a.set("key7","-17.22334");
	ASSERT_NO_THROW ({
		res=ppl7::PythonHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

TEST_F(PythonHelperTest, toHashNonRecursiveWithQuotationMark) {
	ppl7::AssocArray a;
	ppl7::String res;
	ppl7::String expected="key = {\n"
			"    \"key1\": \"Ein \\\"besonderes\\\" Zeichen\",\n"
			"    \"key2\": 20,\n"
			"    \"key3\": \"Geht\\nueber\\nmehrere\\nZeilen\\n\"\n"
			"}\n";

	a.set("key1","Ein \"besonderes\" Zeichen");
	a.set("key2","20");
	a.set("key3","Geht\nueber\nmehrere\nZeilen\n");
	ASSERT_NO_THROW ({
		res=ppl7::PythonHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

TEST_F(PythonHelperTest, toHashNonRecursiveWithAtChar) {
	ppl7::AssocArray a;
	ppl7::String res;
	ppl7::String expected="key = {\n"
			"    \"email\": \"ppl@pfp.de\",\n"
			"    \"key1\": \"Ein \\\"besonderes\\\" Zeichen\",\n"
			"    \"key2\": 20,\n"
			"    \"key3\": \"Geht\\nueber\\nmehrere\\nZeilen\\n\"\n"
			"}\n";

	a.set("key1","Ein \"besonderes\" Zeichen");
	a.set("key2","20");
	a.set("key3","Geht\nueber\nmehrere\nZeilen\n");
	a.set("email","ppl@pfp.de");
	ASSERT_NO_THROW ({
		res=ppl7::PythonHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

TEST_F(PythonHelperTest, toHashRecursive) {
	ppl7::AssocArray a;
	ppl7::String res;
	ppl7::String expected="key = {\n"
			"    \"key1\": \"value1\",\n"
			"    \"key2\": 20,\n"
			"    \"key3\": \"Geht\\nueber\\nmehrere\\nZeilen\\n\",\n"
			"    \"tree1\": {\n"
			"        \"key1\": \"subvalue1\",\n"
			"        \"key2\": \"subvalue2\"\n"
			"    }\n"
			"}\n";

	a.set("key1","value1");
	a.set("key2","20");
	a.set("key3","Geht\nueber\nmehrere\nZeilen\n");
	a.set("tree1/key1","subvalue1");
	a.set("tree1/key2","subvalue2");
	ASSERT_NO_THROW ({
		res=ppl7::PythonHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

TEST_F(PythonHelperTest, toHashRecursiveWithQuotationMark) {
	ppl7::AssocArray a;
	ppl7::String res;
	ppl7::String expected="key = {\n"
			"    \"key1\": \"Ein \\\"besonderes\\\" Zeichen\",\n"
			"    \"key2\": 20,\n"
			"    \"key3\": \"Geht\\nueber\\nmehrere\\nZeilen\\n\",\n"
			"    \"tree1\": {\n"
			"        \"key1\": \"subvalue1\",\n"
			"        \"key2\": \"subvalue2\",\n"
			"        \"key3\": \"Ein \\\"besonderes\\\" Zeichen\"\n"
			"    }\n"
			"}\n";


	a.set("key1","Ein \"besonderes\" Zeichen");
	a.set("key2","20");
	a.set("key3","Geht\nueber\nmehrere\nZeilen\n");
	a.set("tree1/key1","subvalue1");
	a.set("tree1/key2","subvalue2");
	a.set("tree1/key3","Ein \"besonderes\" Zeichen");

	ASSERT_NO_THROW ({
		res=ppl7::PythonHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

TEST_F(PythonHelperTest, toHashRecursiveWithAtChar) {
	ppl7::AssocArray a;
	ppl7::String res;
	ppl7::String expected="key = {\n"
			"    \"email\": \"ppl@pfp.de\",\n"
			"    \"key1\": \"Ein \\\"besonderes\\\" Zeichen\",\n"
			"    \"key2\": 20,\n"
			"    \"key3\": \"Geht\\nueber\\nmehrere\\nZeilen\\n\",\n"
			"    \"tree1\": {\n"
			"        \"email\": \"test@test.de\",\n"
			"        \"key1\": \"subvalue1\",\n"
			"        \"key2\": \"subvalue2\"\n"
			"    }\n"
			"}\n";

	a.set("key1","Ein \"besonderes\" Zeichen");
	a.set("key2","20");
	a.set("key3","Geht\nueber\nmehrere\nZeilen\n");
	a.set("email","ppl@pfp.de");
	a.set("tree1/key1","subvalue1");
	a.set("tree1/key2","subvalue2");
	a.set("tree1/email","test@test.de");

	ASSERT_NO_THROW ({
		res=ppl7::PythonHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}


}


