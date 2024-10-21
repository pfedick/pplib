/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2024, Patrick Fedick <patrick@pfp.de>
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
#include <locale.h>
#include <ppl7.h>
#include <gtest/gtest.h>
#include "ppl7-tests.h"

namespace {

// The fixture for testing class Foo.
class PcreTest : public ::testing::Test {
	protected:
	PcreTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~PcreTest() {

	}
};


TEST_F(PcreTest, bool_compile_match_8) {

	ASSERT_NO_THROW({
		ppl7::RegEx::Pattern p=ppl7::RegEx::compile("^Hello.*$");
		ASSERT_TRUE(ppl7::RegEx::match(p,"Hello World"));
		ASSERT_FALSE(ppl7::RegEx::match(p,"Helleo World"));
	}
	);
}

TEST_F(PcreTest, bool_match_8) {

	ASSERT_NO_THROW({
		ASSERT_TRUE(ppl7::RegEx::match("^Hello.*$","Hello World"));
		ASSERT_FALSE(ppl7::RegEx::match("^Hello.*$","Helleo World"));
	}
	);
}

TEST_F(PcreTest, MatchPositive) {
	ppl7::String s1("Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
	ppl7::String expr("^Lorem.*$");
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1,ppl7::RegEx::Flags::DOTALL));
	expr.set("^Lorem.*$");
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1,ppl7::RegEx::Flags::DOTALL|ppl7::RegEx::Flags::CASELESS));
	expr.set("consectetuer");
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1));
	expr.set("^.*consectetuer.*$");
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1,ppl7::RegEx::Flags::DOTALL));
	expr.set("^.*mus\\.$");
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1,ppl7::RegEx::Flags::MULTILINE));
}

TEST_F(PcreTest, MatchNegativ) {
	ppl7::String s1("Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
	ppl7::String expr("^Looorem.*$");
	ASSERT_FALSE(ppl7::RegEx::match(expr,s1,ppl7::RegEx::Flags::DOTALL));
	expr.set("^ipsum.*$");
	ASSERT_FALSE(ppl7::RegEx::match(expr,s1,ppl7::RegEx::Flags::DOTALL|ppl7::RegEx::Flags::CASELESS));
	expr.set("patrick");
	ASSERT_FALSE(ppl7::RegEx::match(expr,s1));
	expr.set("^.*patrick.*$");
	ASSERT_FALSE(ppl7::RegEx::match(expr,s1,ppl7::RegEx::Flags::DOTALL));
	expr.set("^.*mus\\.$");
	ASSERT_FALSE(ppl7::RegEx::match(expr,s1));
}

TEST_F(PcreTest, MatchPerlRegExPositive) {
	ppl7::String s1("Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
	ppl7::String expr("/^Lorem.*$/s");
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1));
	expr.set("/^Lorem.*$/is");
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1));
	expr.set("/consectetuer/");
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1));
	expr.set("/^.*consectetuer.*$/s");
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1));
	expr.set("/^.*mus\\.$/m");
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1));
}


TEST_F(PcreTest, bool_capture_8) {
	std::vector<ppl7::String> m;
	ppl7::String s1("2012-05-18");
	ASSERT_TRUE(ppl7::RegEx::capture("/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})$/",s1,m));
	ASSERT_EQ((size_t)4,m.size()) << "Unexpected number auf captures";
	ASSERT_EQ(2012,m[1].toInt()) << "Unexpected value in capture";
	ASSERT_EQ(5,m[2].toInt()) << "Unexpected value in capture";
	ASSERT_EQ(18,m[3].toInt()) << "Unexpected value in capture";
	
}

/*

TEST_F(StringTest, pregReplace) {
	ppl7::String s1("Lorem ipsum dolor sit amet.");
	ppl7::String s2("Lor3m ipsum dolor sit am3t.");
	s1.pregReplace("/e/","3");
	ASSERT_EQ(s2,s1) << "Unexpected result from pregReplace";
}
*/



}


