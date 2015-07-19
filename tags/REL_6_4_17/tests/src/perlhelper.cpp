/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 668 $
 * $Date: 2013-05-24 21:56:04 +0200 (Fr, 24 Mai 2013) $
 * $Id: datetime.cpp 668 2013-05-24 19:56:04Z pafe $
 * $URL: https://svn.code.sf.net/p/pplib/code/lib/branches/RELENG_6/tests/src/datetime.cpp $
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

namespace {

// The fixture for testing class Foo.
class PerlHelperTest : public ::testing::Test {
	protected:
	PerlHelperTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen: LC_CTYPE\n");
			throw std::exception();
		}
		if (setlocale(LC_TIME,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen: LC_TIME\n");
			throw std::exception();
		}
		ppl6::SetGlobalEncoding("UTF-8");
	}
	virtual ~PerlHelperTest() {

	}
};


TEST_F(PerlHelperTest, escapeString) {
	ppl6::CString res;

	ASSERT_NO_THROW ({
		res=ppl6::PerlHelper::escapeString("Emailadresse: \"test@test.de\"");
	});
	ASSERT_EQ(ppl6::CString("Emailadresse: \\\"test\\@test.de\\\""),res);
}


TEST_F(PerlHelperTest, toHashWithEmptyArray) {
	ppl6::CAssocArray a;
	ppl6::CString res;

	ASSERT_NO_THROW ({
		res=ppl6::PerlHelper::toHash(a,"key");
	});
	ASSERT_EQ(ppl6::CString("my %key;\n"),res);
}

TEST_F(PerlHelperTest, toHashWithEmptyArrayEmptyKey) {
	ppl6::CAssocArray a;
	ppl6::CString res;
	ASSERT_NO_THROW ({
		res=ppl6::PerlHelper::toHash(a,"");
	});
	ASSERT_EQ(ppl6::CString(""),res);
}

TEST_F(PerlHelperTest, toHashNonRecursive) {
	ppl6::CAssocArray a;
	ppl6::CString res;
	ppl6::CString expected="my %key;\n"
			"$key{key1}=\"value1\";\n"
			"$key{key2}=\"20\";\n"
			"$key{key3}=\"Geht\n"
			"ueber\n"
			"mehrere\n"
			"Zeilen\n"
			"\";\n";

	a.Set("key1","value1");
	a.Set("key2","20");
	a.Set("key3","Geht\nueber\nmehrere\nZeilen\n");
	ASSERT_NO_THROW ({
		res=ppl6::PerlHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

TEST_F(PerlHelperTest, toHashNonRecursiveWithQuotationMark) {
	ppl6::CAssocArray a;
	ppl6::CString res;
	ppl6::CString expected="my %key;\n"
			"$key{key1}=\"Ein \\\"besonderes\\\" Zeichen\";\n"
			"$key{key2}=\"20\";\n"
			"$key{key3}=\"Geht\n"
			"ueber\n"
			"mehrere\n"
			"Zeilen\n"
			"\";\n";

	a.Set("key1","Ein \"besonderes\" Zeichen");
	a.Set("key2","20");
	a.Set("key3","Geht\nueber\nmehrere\nZeilen\n");
	ASSERT_NO_THROW ({
		res=ppl6::PerlHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

TEST_F(PerlHelperTest, toHashNonRecursiveWithAtChar) {
	ppl6::CAssocArray a;
	ppl6::CString res;
	ppl6::CString expected="my %key;\n"
			"$key{email}=\"ppl\\@pfp.de\";\n"
			"$key{key1}=\"Ein \\\"besonderes\\\" Zeichen\";\n"
			"$key{key2}=\"20\";\n"
			"$key{key3}=\"Geht\n"
			"ueber\n"
			"mehrere\n"
			"Zeilen\n"
			"\";\n"
			;

	a.Set("key1","Ein \"besonderes\" Zeichen");
	a.Set("key2","20");
	a.Set("key3","Geht\nueber\nmehrere\nZeilen\n");
	a.Set("email","ppl@pfp.de");
	ASSERT_NO_THROW ({
		res=ppl6::PerlHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

TEST_F(PerlHelperTest, toHashRecursive) {
	ppl6::CAssocArray a;
	ppl6::CString res;
	ppl6::CString expected="my %key;\n"
			"$key{key1}=\"value1\";\n"
			"$key{key2}=\"20\";\n"
			"$key{key3}=\"Geht\n"
			"ueber\n"
			"mehrere\n"
			"Zeilen\n"
			"\";\n"
			"$key{tree1}{key1}=\"subvalue1\";\n"
			"$key{tree1}{key2}=\"subvalue2\";\n"
			;


	a.Set("key1","value1");
	a.Set("key2","20");
	a.Set("key3","Geht\nueber\nmehrere\nZeilen\n");
	a.Set("tree1/key1","subvalue1");
	a.Set("tree1/key2","subvalue2");
	ASSERT_NO_THROW ({
		res=ppl6::PerlHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

TEST_F(PerlHelperTest, toHashRecursiveWithQuotationMark) {
	ppl6::CAssocArray a;
	ppl6::CString res;
	ppl6::CString expected="my %key;\n"
			"$key{key1}=\"Ein \\\"besonderes\\\" Zeichen\";\n"
			"$key{key2}=\"20\";\n"
			"$key{key3}=\"Geht\n"
			"ueber\n"
			"mehrere\n"
			"Zeilen\n"
			"\";\n"
			"$key{tree1}{key1}=\"subvalue1\";\n"
			"$key{tree1}{key2}=\"subvalue2\";\n"
			"$key{tree1}{key3}=\"Ein \\\"besonderes\\\" Zeichen\";\n"
			;

	a.Set("key1","Ein \"besonderes\" Zeichen");
	a.Set("key2","20");
	a.Set("key3","Geht\nueber\nmehrere\nZeilen\n");
	a.Set("tree1/key1","subvalue1");
	a.Set("tree1/key2","subvalue2");
	a.Set("tree1/key3","Ein \"besonderes\" Zeichen");

	ASSERT_NO_THROW ({
		res=ppl6::PerlHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}

TEST_F(PerlHelperTest, toHashRecursiveWithAtChar) {
	ppl6::CAssocArray a;
	ppl6::CString res;
	ppl6::CString expected="my %key;\n"
			"$key{email}=\"ppl\\@pfp.de\";\n"
			"$key{key1}=\"Ein \\\"besonderes\\\" Zeichen\";\n"
			"$key{key2}=\"20\";\n"
			"$key{key3}=\"Geht\n"
			"ueber\n"
			"mehrere\n"
			"Zeilen\n"
			"\";\n"
			"$key{tree1}{email}=\"test\\@test.de\";\n"
			"$key{tree1}{key1}=\"subvalue1\";\n"
			"$key{tree1}{key2}=\"subvalue2\";\n"
			;

	a.Set("key1","Ein \"besonderes\" Zeichen");
	a.Set("key2","20");
	a.Set("key3","Geht\nueber\nmehrere\nZeilen\n");
	a.Set("email","ppl@pfp.de");
	a.Set("tree1/key1","subvalue1");
	a.Set("tree1/key2","subvalue2");
	a.Set("tree1/email","test@test.de");

	ASSERT_NO_THROW ({
		res=ppl6::PerlHelper::toHash(a,"key");
	});
	ASSERT_EQ(expected,res);
}


}


