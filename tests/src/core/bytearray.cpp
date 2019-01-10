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
class ByteArrayTest : public ::testing::Test {
	protected:
	ByteArrayTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~ByteArrayTest() {

	}
};

TEST_F(ByteArrayTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl7::ByteArray b1;
		ASSERT_TRUE(b1.isNull()==true) << "Array is not empty";
	}
	);
}

TEST_F(ByteArrayTest, CopyConstructorWithEmptyObject) {
	ppl7::ByteArray b1;
	ASSERT_NO_THROW({
		ppl7::ByteArray b2(b1);
	});
}

TEST_F(ByteArrayTest, AssignmentWithEmptyObject) {
	ppl7::ByteArray b1;
	ASSERT_NO_THROW({
		ppl7::ByteArray b2=b1;
	});
	ASSERT_NO_THROW({
		ppl7::ByteArray b3;
		b3=b1;
	});

}

TEST_F(ByteArrayTest, load_and_base64) {
	ppl7::ByteArray ba;
	ppl7::File::load(ba,"testdata/screenshot1.png");
	ASSERT_EQ((size_t)59627,ba.size());
	ppl7::String ba64=ba.toBase64();
	ppl7::ByteArray ba2=ppl7::FromBase64(ba64);
	ASSERT_EQ((size_t)59627,ba2.size());
}




// memBlock1 > MemBlock2
// memBlock2 < MemBlock1
static unsigned char memBlock1[]={123,77,42,255,192,16,9};
static unsigned char memBlock2[]={123,77,42,200,192,16,9};

TEST_F(ByteArrayTest, operators_equal) {
	ppl7::ByteArrayPtr b1(memBlock1,sizeof(memBlock1));
	ppl7::ByteArrayPtr b2(memBlock1,sizeof(memBlock1));
	ASSERT_FALSE(b1<b2);
	ASSERT_TRUE(b1<=b2);
	ASSERT_TRUE(b1==b2);
	ASSERT_FALSE(b1!=b2);
	ASSERT_TRUE(b1>=b2);
	ASSERT_FALSE(b1>b2);
}

TEST_F(ByteArrayTest, operators_lower) {
	ppl7::ByteArrayPtr b1(memBlock2,sizeof(memBlock2));
	ppl7::ByteArrayPtr b2(memBlock1,sizeof(memBlock1));
	ASSERT_TRUE(b1<b2);
	ASSERT_TRUE(b1<=b2);
	ASSERT_FALSE(b1==b2);
	ASSERT_TRUE(b1!=b2);
	ASSERT_FALSE(b1>=b2);
	ASSERT_FALSE(b1>b2);
}

TEST_F(ByteArrayTest, operators_greater) {
	ppl7::ByteArrayPtr b1(memBlock1,sizeof(memBlock1));
	ppl7::ByteArrayPtr b2(memBlock2,sizeof(memBlock2));
	ASSERT_FALSE(b1<b2);
	ASSERT_FALSE(b1<=b2);
	ASSERT_FALSE(b1==b2);
	ASSERT_TRUE(b1!=b2);
	ASSERT_TRUE(b1>=b2);
	ASSERT_TRUE(b1>b2);
}



}

