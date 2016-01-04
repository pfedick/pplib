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
class ByteArrayPtrTest : public ::testing::Test {
	protected:
	ByteArrayPtrTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~ByteArrayPtrTest() {

	}
};

TEST_F(ByteArrayPtrTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl7::ByteArrayPtr b1;
		ASSERT_TRUE(b1.isNull()==true) << "Array is not empty";
	}
	);
}


class ByteArrayPtrCompare : public ::testing::Test {
	protected:
		ByteArrayPtrCompare() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~ByteArrayPtrCompare() {

	}
};


// memBlock1 > MemBlock2
//           > MemBlock3
//           < MemBlock4
// memBlock2 < MemBlock1
//           > MemBlock3
//           < MemBlock4
// memBlock3 < MemBlock1
//           < MemBlock2
//           < MemBlock4
// memBlock4 > MemBlock1
//           > MemBlock2
//           > MemBlock4
static unsigned char memBlock1[]={123,77,42,255,192,16,9};
static unsigned char memBlock2[]={123,77,42,200,192,16,9};
static unsigned char memBlock3[]={123,77,42};
static unsigned char memBlock4[]={123,77,99};


TEST_F(ByteArrayPtrCompare, b1ZeroSize_b2ZeroSize) {
	ppl7::ByteArrayPtr b1,b2;
	ASSERT_EQ(0,b1.memcmp(b2));
}

TEST_F(ByteArrayPtrCompare, b1ZeroSize_b2NotZero) {
	ppl7::ByteArrayPtr b1;
	ppl7::ByteArrayPtr b2(memBlock1,sizeof(memBlock1));
	ASSERT_EQ((size_t)7,b2.size());
	ASSERT_EQ(-1,b1.memcmp(b2));
}

TEST_F(ByteArrayPtrCompare, b1NotZero_b2ZeroSize) {
	ppl7::ByteArrayPtr b1(memBlock1,sizeof(memBlock1));
	ppl7::ByteArrayPtr b2;
	ASSERT_EQ((size_t)7,b1.size());
	ASSERT_EQ(1,b1.memcmp(b2));
}

TEST_F(ByteArrayPtrCompare, b1_eq_b2_samesize) {
	ppl7::ByteArrayPtr b1(memBlock1,sizeof(memBlock1));
	ppl7::ByteArrayPtr b2(memBlock1,sizeof(memBlock1));
	ASSERT_EQ(0,b1.memcmp(b2));
}

TEST_F(ByteArrayPtrCompare, b1_lt_b2_samesize) {
	ppl7::ByteArrayPtr b1(memBlock2,sizeof(memBlock2));
	ppl7::ByteArrayPtr b2(memBlock1,sizeof(memBlock1));
	ASSERT_EQ(-1,b1.memcmp(b2));
}

TEST_F(ByteArrayPtrCompare, b1_gt_b2_samesize) {
	ppl7::ByteArrayPtr b1(memBlock1,sizeof(memBlock1));
	ppl7::ByteArrayPtr b2(memBlock2,sizeof(memBlock2));
	ASSERT_EQ(1,b1.memcmp(b2));
}

TEST_F(ByteArrayPtrCompare, b1_lt_b2__b1_shorter_b2) {
	ppl7::ByteArrayPtr b1(memBlock3,sizeof(memBlock3));
	ppl7::ByteArrayPtr b2(memBlock1,sizeof(memBlock1));
	ASSERT_EQ(-1,b1.memcmp(b2));
}

TEST_F(ByteArrayPtrCompare, b1_gt_b2__b1_shorter_b2) {
	ppl7::ByteArrayPtr b1(memBlock4,sizeof(memBlock4));
	ppl7::ByteArrayPtr b2(memBlock2,sizeof(memBlock2));
	ASSERT_EQ(1,b1.memcmp(b2));
}

TEST_F(ByteArrayPtrCompare, b1_lt_b2__b1_longer_b2) {
	ppl7::ByteArrayPtr b1(memBlock1,sizeof(memBlock1));
	ppl7::ByteArrayPtr b2(memBlock4,sizeof(memBlock4));
	ASSERT_EQ(-1,b1.memcmp(b2));
}

TEST_F(ByteArrayPtrCompare, b1_gt_b2__b1_longer_b2) {
	ppl7::ByteArrayPtr b1(memBlock2,sizeof(memBlock2));
	ppl7::ByteArrayPtr b2(memBlock3,sizeof(memBlock3));
	ASSERT_EQ(1,b1.memcmp(b2));
}

TEST_F(ByteArrayPtrCompare, operators_equal) {
	ppl7::ByteArrayPtr b1(memBlock1,sizeof(memBlock1));
	ppl7::ByteArrayPtr b2(memBlock1,sizeof(memBlock1));
	ASSERT_FALSE(b1<b2);
	ASSERT_TRUE(b1<=b2);
	ASSERT_TRUE(b1==b2);
	ASSERT_FALSE(b1!=b2);
	ASSERT_TRUE(b1>=b2);
	ASSERT_FALSE(b1>b2);

	ASSERT_EQ(b1,b2);

}

TEST_F(ByteArrayPtrCompare, operators_lower) {
	ppl7::ByteArrayPtr b1(memBlock2,sizeof(memBlock2));
	ppl7::ByteArrayPtr b2(memBlock1,sizeof(memBlock1));
	ASSERT_TRUE(b1<b2);
	ASSERT_TRUE(b1<=b2);
	ASSERT_FALSE(b1==b2);
	ASSERT_TRUE(b1!=b2);
	ASSERT_FALSE(b1>=b2);
	ASSERT_FALSE(b1>b2);

	ASSERT_NE(b1,b2);
	ASSERT_LT(b1,b2);
}

TEST_F(ByteArrayPtrCompare, operators_greater) {
	ppl7::ByteArrayPtr b1(memBlock1,sizeof(memBlock1));
	ppl7::ByteArrayPtr b2(memBlock2,sizeof(memBlock2));
	ASSERT_FALSE(b1<b2);
	ASSERT_FALSE(b1<=b2);
	ASSERT_FALSE(b1==b2);
	ASSERT_TRUE(b1!=b2);
	ASSERT_TRUE(b1>=b2);
	ASSERT_TRUE(b1>b2);

	ASSERT_NE(b1,b2);
	ASSERT_GT(b1,b2);

}


}

