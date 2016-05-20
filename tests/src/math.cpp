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
#include <locale.h>
#include <ppl7.h>
#include <gtest/gtest.h>
#include "ppl7-tests.h"


namespace {

// The fixture for testing class Foo.
class CalcTest : public ::testing::Test {
	protected:
		CalcTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~CalcTest() {

	}
};

TEST_F(CalcTest, SinglePositiveValue) {
	ASSERT_DOUBLE_EQ((double)2.0,ppl7::Calc(" 2	 "));
	ASSERT_DOUBLE_EQ((double)2.0,ppl7::Calc("2"));
	ASSERT_DOUBLE_EQ((double)19.1,ppl7::Calc("19.1"));
	ASSERT_DOUBLE_EQ((double)23.1234,ppl7::Calc("23.1234"));
	ASSERT_DOUBLE_EQ((double)23.1234,ppl7::Calc("+23.1234"));
}

TEST_F(CalcTest, SingleNegativeValue) {
	ASSERT_DOUBLE_EQ((double)-2.0,ppl7::Calc("-2"));
	ASSERT_DOUBLE_EQ((double)-19.1,ppl7::Calc("-19.1"));
}

TEST_F(CalcTest, AdditionOfTwoPositiveValues) {
	ASSERT_DOUBLE_EQ((double)19,ppl7::Calc("6+13"));
	ASSERT_DOUBLE_EQ((double)19,ppl7::Calc(" 6 + 13 "));
	ASSERT_DOUBLE_EQ((double)7,ppl7::Calc("-6 + 13 "));
}

TEST_F(CalcTest, AdditionOfNegativeAndPositiveValues) {
	ASSERT_DOUBLE_EQ((double)7,ppl7::Calc("-6+13"));
}
TEST_F(CalcTest, AdditionOfPositiveAndNegativeValues) {
	ASSERT_DOUBLE_EQ((double)-7,ppl7::Calc("6+-13"));
	ASSERT_DOUBLE_EQ((double)5.87,ppl7::Calc("6+-.13"));
}

TEST_F(CalcTest, SubtractionOfTwoPositiveValues) {
	ASSERT_DOUBLE_EQ((double)-7,ppl7::Calc("6-13"));
}

TEST_F(CalcTest, SubtractionOfTwoNegativeValues) {
	ASSERT_DOUBLE_EQ((double)-20,ppl7::Calc("-7-13"));
}

TEST_F(CalcTest, SubtractionOfPositiveAndNegativeValues) {
	ASSERT_DOUBLE_EQ((double)20,ppl7::Calc("7--13"));
}

TEST_F(CalcTest, Multiply) {
	ASSERT_DOUBLE_EQ((double)9,ppl7::Calc("3*3"));
	ASSERT_DOUBLE_EQ((double)-10.2,ppl7::Calc("3.4*-3"));
	ASSERT_DOUBLE_EQ((double)-9.61,ppl7::Calc("3.1*-3.1"));
}


TEST_F(CalcTest, PunktVorStrich) {
	ASSERT_DOUBLE_EQ((double)7,ppl7::Calc("2*4+8-3*3"));
}


TEST_F(CalcTest, Bracket) {
	ASSERT_DOUBLE_EQ((double)6,ppl7::Calc("2*4+(8-3*3)*2"));
}

}


