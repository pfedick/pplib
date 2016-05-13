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


TEST_F(CalcTest, ResolveSimpleBracket) {
	ASSERT_EQ((double)2.0,ppl7::Calc("(2)"));
}

TEST_F(CalcTest, SingleValue) {
	ASSERT_EQ((double)2.0,ppl7::Calc("2"));
	ASSERT_EQ((double)10.1,ppl7::Calc("10.1"));
}

TEST_F(CalcTest, Addition) {
	ASSERT_EQ((double)4.0,ppl7::Calc("2+2"));
	ASSERT_EQ((double)19.5,ppl7::Calc("2.1 + 17.4"));
	ASSERT_EQ((double)16.0,ppl7::Calc("9+(3+4)"));
}

TEST_F(CalcTest, Subtraction) {
	ASSERT_EQ((double)7.0,ppl7::Calc("9-2"));
	ASSERT_EQ((double)6.3,ppl7::Calc("9.4 - 3.1"));
	ASSERT_EQ((double)8.0,ppl7::Calc("9+(3-4)"));
}


}


