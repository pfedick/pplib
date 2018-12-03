/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 *******************************************************************************
 * Copyright (c) 2018, Patrick Fedick <patrick@pfp.de>
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
#include <ppl6.h>
#include <ppl7.h>
#include "ppl7-ppl6compat.h"
#include <gtest/gtest.h>
#include "ppl7-tests.h"

namespace {

// The fixture for testing class Foo.
class PPL6CompatStringTest : public ::testing::Test {
	protected:
	PPL6CompatStringTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~PPL6CompatStringTest() {

	}
};

TEST_F(PPL6CompatStringTest, 6constructor) {
	ASSERT_NO_THROW({
		ppl6::CString p6("this is a ppl6 string äöü");
	});

}

TEST_F(PPL6CompatStringTest, 6to7Constructor) {
	ppl6::CString p6("this is a ppl6 string äöü");
	ppl7::String p7(p6);
	ASSERT_EQ(ppl7::String("this is a ppl6 string äöü"),p7);
}

TEST_F(PPL6CompatStringTest, 6to7Assign) {
	ppl6::CString p6("this is a ppl6 string äöü");
	ppl7::String p7;
	p7=p6;
	ASSERT_EQ(ppl7::String("this is a ppl6 string äöü"),p7);
}

TEST_F(PPL6CompatStringTest, 6to7OperatorPlus) {
	ppl6::CString p6("this is a ppl6 string äöü");
	ppl7::String p7a("Hello, "),p7;
	p7=p7a+p6;
	ASSERT_EQ(ppl7::String("Hello, this is a ppl6 string äöü"),p7);

	p7=p6+p7a;
	ASSERT_EQ(ppl7::String("this is a ppl6 string äöüHello, "),p7);

}

TEST_F(PPL6CompatStringTest, 6to7OperatorPlusAssign) {
	ppl6::CString p6("this is a ppl6 string äöü");
	ppl7::String p7("Hello, ");
	p7+=p6;
	ASSERT_EQ(ppl7::String("Hello, this is a ppl6 string äöü"),p7);
}

TEST_F(PPL6CompatStringTest, 7to6ConstructorTo6) {
	ppl7::String p7("this is a ppl7 string äöü");
	ppl6::CString p6(ppl7::to6(p7));
	ASSERT_EQ(ppl6::CString("this is a ppl7 string äöü"),p6);
}

TEST_F(PPL6CompatStringTest, 7to6Assign) {
	ppl7::String p7("this is a ppl7 string äöü");
	ppl6::CString p6=p7;
	ASSERT_EQ(ppl6::CString("this is a ppl7 string äöü"),p6);
}

}

