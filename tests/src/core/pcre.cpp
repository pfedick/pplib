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


TEST_F(PcreTest, bool_compile_match) {

	ASSERT_NO_THROW({
		ppl7::RegEx::Pattern p=ppl7::RegEx::compile("^Hello.*$");
		ASSERT_TRUE(ppl7::RegEx::match(p,"Hello World"));
		ASSERT_FALSE(ppl7::RegEx::match(p,"Helleo World"));
	}
	ppl7::RegEx::compile("^.*\\.json$");
	
	);
}

TEST_F(PcreTest, bool_match) {

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
	ASSERT_TRUE(ppl7::RegEx::match("^.*\\.json$","blah.json"));
	
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
	ASSERT_FALSE(ppl7::RegEx::match("^.*\\.json$","."));
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


TEST_F(PcreTest, capture) {
	std::vector<ppl7::String> m;
	ppl7::String s1("2012-05-18");
	ASSERT_TRUE(ppl7::RegEx::capture("/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})$/",s1,m));
	ASSERT_EQ((size_t)4,m.size()) << "Unexpected number auf captures";
	ASSERT_EQ(2012,m[1].toInt()) << "Unexpected value in capture";
	ASSERT_EQ(5,m[2].toInt()) << "Unexpected value in capture";
	ASSERT_EQ(18,m[3].toInt()) << "Unexpected value in capture";
	
}


TEST_F(PcreTest, replace) {
	ppl7::String s1("Lorem ipsum dolor sit amet.");
	ppl7::String expected("Lor3m ipsum dolor sit am3t.");
	ppl7::String result=ppl7::RegEx::replace("/e/",s1,"3");
	ASSERT_EQ(expected,result) << "Unexpected result from pregReplace";

	ASSERT_EQ(ppl7::String("Lorem --- amet."),ppl7::RegEx::replace("ip.*sit",s1,"---"));
	ASSERT_EQ(ppl7::String("Lorem  amet."),ppl7::RegEx::replace("ip.*sit",s1,""));
	ASSERT_EQ(ppl7::String("Lor4m ipsum dolor sit amet."),ppl7::RegEx::replace("e",s1,"4",0,1));
	
}

TEST_F(PcreTest, escape) {
	ppl7::String s1("Lorem ipsum dolor sit amet.");
	ASSERT_EQ(ppl7::String("Hello \\+Wor\\/ld"),ppl7::RegEx::escape("Hello +Wor/ld"));
}


class PcreTestWideChar : public ::testing::Test {
	protected:
	PcreTestWideChar() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~PcreTestWideChar() {

	}
};

TEST_F(PcreTestWideChar, bool_compile) {

	ASSERT_NO_THROW({
		ppl7::RegEx::Pattern p=ppl7::RegEx::compile(L"^Hello.*$");
	}
	);
	ASSERT_NO_THROW({
		ppl7::RegEx::compile(L"^.*\\.json$");
	}
	);
}

TEST_F(PcreTestWideChar, bool_match) {

	ASSERT_NO_THROW({
		ppl7::RegEx::Pattern p=ppl7::RegEx::compile(L"^Hello.*$");
		ASSERT_TRUE(ppl7::RegEx::match(p,L"Hello World"));
		ASSERT_FALSE(ppl7::RegEx::match(p,L"Helleo World"));
	});
}

TEST_F(PcreTestWideChar, MatchPositive) {
	ppl7::WideString s1(L"Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
	ppl7::WideString expr(L"^Lorem.*$");
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1,ppl7::RegEx::Flags::DOTALL));
	expr.set(L"^Lorem.*$");
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1,ppl7::RegEx::Flags::DOTALL|ppl7::RegEx::Flags::CASELESS));
	expr.set(L"consectetuer");
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1));
	expr.set(L"^.*consectetuer.*$");
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1,ppl7::RegEx::Flags::DOTALL));
	
}

TEST_F(PcreTestWideChar, MatchMultiline) {
	ppl7::WideString s1(L"Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
	ppl7::WideString expr(L"^.*mus\\.$");
	ASSERT_TRUE(ppl7::RegEx::match(L"^.*\\.json$",L"blah.json"));
	ASSERT_TRUE(ppl7::RegEx::match(expr,s1,ppl7::RegEx::Flags::MULTILINE));

}

TEST_F(PcreTestWideChar, MatchNegativ) {
	ppl7::WideString s1(L"Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
	ppl7::WideString expr(L"^Looorem.*$");
	ASSERT_FALSE(ppl7::RegEx::match(expr,s1,ppl7::RegEx::Flags::DOTALL));
	expr.set(L"^ipsum.*$");
	ASSERT_FALSE(ppl7::RegEx::match(expr,s1,ppl7::RegEx::Flags::DOTALL|ppl7::RegEx::Flags::CASELESS));
	expr.set(L"patrick");
	ASSERT_FALSE(ppl7::RegEx::match(expr,s1));
	expr.set(L"^.*patrick.*$");
	ASSERT_FALSE(ppl7::RegEx::match(expr,s1,ppl7::RegEx::Flags::DOTALL));
	expr.set(L"^.*mus\\.$");
	ASSERT_FALSE(ppl7::RegEx::match(expr,s1));
	ASSERT_FALSE(ppl7::RegEx::match(L"^.*\\.json$",L"."));
}

TEST_F(PcreTestWideChar, capture) {
	std::vector<ppl7::WideString> m;
	ppl7::WideString s1(L"2012-05-18");
	ppl7::RegEx::Pattern p=ppl7::RegEx::compile(L"/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})$/i");
	ASSERT_TRUE(ppl7::RegEx::capture(L"/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})$/",s1,m));
	ASSERT_EQ((size_t)4,m.size()) << "Unexpected number auf captures";
	ASSERT_EQ(2012,m[1].toInt()) << "Unexpected value in capture";
	ASSERT_EQ(5,m[2].toInt()) << "Unexpected value in capture";
	ASSERT_EQ(18,m[3].toInt()) << "Unexpected value in capture";
}


TEST_F(PcreTestWideChar, replace) {
	ppl7::WideString s1(L"Lorem ipsum dolor sit amet.");
	ppl7::WideString expected(L"Lor3m ipsum dolor sit am3t.");
	ppl7::WideString result=ppl7::RegEx::replace(L"/e/",s1,L"3");
	ASSERT_EQ(expected,result) << "Unexpected result from pregReplace";

	ASSERT_EQ(ppl7::WideString(L"Lorem --- amet."),ppl7::RegEx::replace(L"ip.*sit",s1,L"---"));
	ASSERT_EQ(ppl7::WideString(L"Lorem  amet."),ppl7::RegEx::replace(L"ip.*sit",s1,L""));
	ASSERT_EQ(ppl7::WideString(L"Lor4m ipsum dolor sit amet."),ppl7::RegEx::replace(L"e",s1,L"4",0,1));
	
}

TEST_F(PcreTestWideChar, escape) {
	ppl7::WideString s1(L"Lorem ipsum dolor sit amet.");
	ASSERT_EQ(ppl7::WideString(L"Hello \\+Wor\\/ld"),ppl7::RegEx::escape(L"Hello +Wor/ld"));
}

}


