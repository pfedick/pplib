/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 890 $
 * $Date: 2014-02-04 14:27:53 +0100 (Di, 04 Feb 2014) $
 * $Id: PythonHelper.cpp 890 2014-02-04 13:27:53Z pafe $
 * $URL: https://svn.code.sf.net/p/pplib/code/lib/branches/RELENG_6/tests/src/PythonHelper.cpp $
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

#include "../include/prolog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include "../include/ppl6.h"
#include <gtest/gtest.h>
#include "ppl6-tests.h"

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
		ppl6::SetGlobalEncoding("UTF-8");
	}
	virtual ~PythonHelperTest() {

	}
};


TEST_F(PythonHelperTest, escapeString) {
	ppl6::CString res;

	ASSERT_NO_THROW ({
		res=ppl6::PythonHelper::escapeString("Emailadresse: \"test@test.de\"");
	});
	ASSERT_EQ(ppl6::CString("Emailadresse: \\\"test@test.de\\\""),res);
}

TEST_F(PythonHelperTest, escapeStringWithBackslashes) {
	ppl6::CString res;

	ASSERT_NO_THROW ({
		res=ppl6::PythonHelper::escapeString("test\\,test");
	});
	ASSERT_EQ(ppl6::CString("test\\\\,test"),res);
}


TEST_F(PythonHelperTest, toHashWithEmptyArray) {
	ppl6::CAssocArray a;
	ppl6::CString res;

	ASSERT_NO_THROW ({
		res=ppl6::PythonHelper::toHash(a,"key");
	});
	ASSERT_EQ(ppl6::CString("key = {}\n"),res);
}

TEST_F(PythonHelperTest, toHashWithEmptyArrayEmptyKey) {
	ppl6::CAssocArray a;
	ppl6::CString res;
	ASSERT_NO_THROW ({
		res=ppl6::PythonHelper::toHash(a,"");
	});
	ASSERT_EQ(ppl6::CString(""),res);
}

TEST_F(PythonHelperTest, toHashNonRecursive) {
	ppl6::CAssocArray a;
	ppl6::CString res;
	ppl6::CString expected="key = {\n"
			"    \"key1\": \"value1\",\n"
			"    \"key2\": 20,\n"
			"    \"key3\": \"Geht\\nueber\\nmehrere\\nZeilen\\n\"\n"
			"}\n";

	a.Set("key1","value1");
	a.Set("key2","20");
	a.Set("key3","Geht\nueber\nmehrere\nZeilen\n");
	ASSERT_NO_THROW ({
		res=ppl6::PythonHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}


TEST_F(PythonHelperTest, toHashNonRecursiveWithQuotationMark) {
	ppl6::CAssocArray a;
	ppl6::CString res;
	ppl6::CString expected="key = {\n"
			"    \"key1\": \"Ein \\\"besonderes\\\" Zeichen\",\n"
			"    \"key2\": 20,\n"
			"    \"key3\": \"Geht\\nueber\\nmehrere\\nZeilen\\n\"\n"
			"}\n";

	a.Set("key1","Ein \"besonderes\" Zeichen");
	a.Set("key2","20");
	a.Set("key3","Geht\nueber\nmehrere\nZeilen\n");
	ASSERT_NO_THROW ({
		res=ppl6::PythonHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

TEST_F(PythonHelperTest, toHashNonRecursiveWithAtChar) {
	ppl6::CAssocArray a;
	ppl6::CString res;
	ppl6::CString expected="key = {\n"
			"    \"email\": \"ppl@pfp.de\",\n"
			"    \"key1\": \"Ein \\\"besonderes\\\" Zeichen: Backslash \\\\\",\n"
			"    \"key2\": 20,\n"
			"    \"key3\": \"Geht\\nueber\\nmehrere\\nZeilen\\n\"\n"
			"}\n";

	a.Set("key1","Ein \"besonderes\" Zeichen: Backslash \\");
	a.Set("key2","20");
	a.Set("key3","Geht\nueber\nmehrere\nZeilen\n");
	a.Set("email","ppl@pfp.de");
	ASSERT_NO_THROW ({
		res=ppl6::PythonHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

TEST_F(PythonHelperTest, toHashRecursive) {
	ppl6::CAssocArray a;
	ppl6::CString res;
	ppl6::CString expected="key = {\n"
			"    \"key1\": \"value1\",\n"
			"    \"key2\": 20,\n"
			"    \"key3\": \"Geht\\nueber\\nmehrere\\nZeilen\\n\",\n"
			"    \"tree1\": {\n"
			"        \"key1\": \"subvalue1\",\n"
			"        \"key2\": \"subvalue2\"\n"
			"    }\n"
			"}\n";

	a.Set("key1","value1");
	a.Set("key2","20");
	a.Set("key3","Geht\nueber\nmehrere\nZeilen\n");
	a.Set("tree1/key1","subvalue1");
	a.Set("tree1/key2","subvalue2");
	ASSERT_NO_THROW ({
		res=ppl6::PythonHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

TEST_F(PythonHelperTest, toHashRecursiveWithQuotationMark) {
	ppl6::CAssocArray a;
	ppl6::CString res;
	ppl6::CString expected="key = {\n"
			"    \"key1\": \"Ein \\\"besonderes\\\" \\\\Zeichen\",\n"
			"    \"key2\": 20,\n"
			"    \"key3\": \"Geht\\nueber\\nmehrere\\nZeilen\\n\",\n"
			"    \"tree1\": {\n"
			"        \"key1\": \"subvalue1\",\n"
			"        \"key2\": \"subvalue2\",\n"
			"        \"key3\": \"Ein \\\"besonderes\\\" \\\\Zeichen\"\n"
			"    }\n"
			"}\n";


	a.Set("key1","Ein \"besonderes\" \\Zeichen");
	a.Set("key2","20");
	a.Set("key3","Geht\nueber\nmehrere\nZeilen\n");
	a.Set("tree1/key1","subvalue1");
	a.Set("tree1/key2","subvalue2");
	a.Set("tree1/key3","Ein \"besonderes\" \\Zeichen");

	ASSERT_NO_THROW ({
		res=ppl6::PythonHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

TEST_F(PythonHelperTest, toHashRecursiveWithAtChar) {
	ppl6::CAssocArray a;
	ppl6::CString res;
	ppl6::CString expected="key = {\n"
			"    \"email\": \"ppl@pfp.de\",\n"
			"    \"key1\": \"Ein \\\"besonderes\\\" \\\\Zeichen\",\n"
			"    \"key2\": 20,\n"
			"    \"key3\": \"Geht\\nueber\\nmehrere\\nZeilen\\n\",\n"
			"    \"tree1\": {\n"
			"        \"email\": \"test@test.de\",\n"
			"        \"key1\": \"subvalue1\",\n"
			"        \"key2\": \"subvalue2\"\n"
			"    }\n"
			"}\n";

	a.Set("key1","Ein \"besonderes\" \\Zeichen");
	a.Set("key2","20");
	a.Set("key3","Geht\nueber\nmehrere\nZeilen\n");
	a.Set("email","ppl@pfp.de");
	a.Set("tree1/key1","subvalue1");
	a.Set("tree1/key2","subvalue2");
	a.Set("tree1/email","test@test.de");

	ASSERT_NO_THROW ({
		res=ppl6::PythonHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

}


