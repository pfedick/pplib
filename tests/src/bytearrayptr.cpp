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

TEST_F(ByteArrayPtrTest, ConstructorFromPointerAndSize) {
	void *adr=this;
	const void *adr2=this;
	size_t size=1234567;

	ASSERT_NO_THROW({
		ppl7::ByteArrayPtr b1(adr,size);
		ASSERT_FALSE(b1.isNull()) << "Array is not empty";
		ASSERT_EQ(adr,b1.ptr());
		ASSERT_EQ(size,b1.size());
	}
	);

	ASSERT_NO_THROW({
		ppl7::ByteArrayPtr b1(adr2,size);
		ASSERT_FALSE(b1.isNull()) << "Array is not empty";
		ASSERT_EQ(adr2,b1.ptr());
		ASSERT_EQ(size,b1.size());
	}
	);
}


TEST_F(ByteArrayPtrTest, ConstructorFromByteArrayPtr) {
	ppl7::ByteArrayPtr b1(this,1234567);
	ASSERT_NO_THROW({
		ppl7::ByteArrayPtr b2(b1);
		ASSERT_FALSE(b2.isNull()) << "Array is not empty";
		ASSERT_EQ(this,b2.ptr());
		ASSERT_EQ(1234567,b2.size());
	}
	);
}

TEST_F(ByteArrayPtrTest, ConstructorFromString) {
	ppl7::String s1("Hello World");
	ASSERT_NO_THROW({
		ppl7::ByteArrayPtr b2(s1);
		ASSERT_FALSE(b2.isNull()) << "Array is not empty";
		ASSERT_EQ(s1.getPtr(),b2.ptr());
		ASSERT_EQ(s1.size(),b2.size());
	}
	);
}

TEST_F(ByteArrayPtrTest, ConstructorFromWideString) {
	ppl7::WideString s1(L"Hello World");
	ASSERT_NO_THROW({
		ppl7::ByteArrayPtr b2(s1);
		ASSERT_FALSE(b2.isNull()) << "Array is not empty";
		ASSERT_EQ(s1.getPtr(),b2.ptr());
		ASSERT_EQ(s1.size(),b2.size());
	}
	);
}

//0d,0e,37,42,81,ff,42,00,4c,17,12
static unsigned char binarydata[]={13,14,55,66,129,255,66,0,76,23,18};

TEST_F(ByteArrayPtrTest, isNull) {
	ppl7::ByteArrayPtr b2;
	ASSERT_TRUE(b2.isNull());
	b2.use(binarydata,sizeof(binarydata));
	ASSERT_FALSE(b2.isNull());
	b2.use(binarydata,0);
	ASSERT_FALSE(b2.isNull());
}
TEST_F(ByteArrayPtrTest, isEmpty) {
	ppl7::ByteArrayPtr b2;
	ASSERT_TRUE(b2.isEmpty());
	b2.use(binarydata,sizeof(binarydata));
	ASSERT_FALSE(b2.isEmpty());
	b2.use(binarydata,0);
	ASSERT_TRUE(b2.isEmpty());
}

TEST_F(ByteArrayPtrTest, size) {
	ppl7::ByteArrayPtr b2(binarydata,sizeof(binarydata));
	ASSERT_EQ(sizeof(binarydata),b2.size());
	ppl7::ByteArrayPtr b1;
	ASSERT_EQ((size_t)0,b1.size());
}

TEST_F(ByteArrayPtrTest, ptr) {
	ppl7::ByteArrayPtr b2(binarydata,sizeof(binarydata));
	ASSERT_EQ((const void*)&binarydata,b2.ptr());
	ppl7::ByteArrayPtr b1;
	ASSERT_EQ((const void*)NULL,b1.ptr());
}

TEST_F(ByteArrayPtrTest, adr) {
	ppl7::ByteArrayPtr b2(binarydata,sizeof(binarydata));
	ASSERT_EQ((const void*)&binarydata,b2.adr());
	ppl7::ByteArrayPtr b1;
	ASSERT_EQ((const void*)NULL,b1.adr());
}

TEST_F(ByteArrayPtrTest, toCharPtr) {
	ppl7::ByteArrayPtr b2(binarydata,sizeof(binarydata));
	ASSERT_EQ((const char*)&binarydata,b2.toCharPtr());
	ppl7::ByteArrayPtr b1;
	ASSERT_EQ((const char*)NULL,b1.toCharPtr());
}

TEST_F(ByteArrayPtrTest, operatorVoidPtr) {
	ppl7::ByteArrayPtr b2(binarydata,sizeof(binarydata));
	ASSERT_EQ((const void*)&binarydata,(const void*)b2);
}

TEST_F(ByteArrayPtrTest, operatorCharPtr) {
	ppl7::ByteArrayPtr b2(binarydata,sizeof(binarydata));
	ASSERT_EQ((const char*)&binarydata,(const char*)b2);
}

TEST_F(ByteArrayPtrTest, operatorUnsignedCharPtr) {
	ppl7::ByteArrayPtr b2(binarydata,sizeof(binarydata));
	ASSERT_EQ((const unsigned char*)&binarydata,(const unsigned char*)b2);
}

TEST_F(ByteArrayPtrTest, operatorCopy) {
	ppl7::ByteArrayPtr b1(binarydata,sizeof(binarydata));
	ppl7::ByteArrayPtr b2;
	b2=b1;
	ASSERT_EQ((const void*)&binarydata,b2.adr());
	ASSERT_EQ(sizeof(binarydata),b2.size());
}

TEST_F(ByteArrayPtrTest, usePtrAndSize) {
	ppl7::ByteArrayPtr b2;
	b2.use(binarydata,sizeof(binarydata));
	ASSERT_EQ((const void*)&binarydata,b2.adr());
	ASSERT_EQ(sizeof(binarydata),b2.size());
}

TEST_F(ByteArrayPtrTest, useOtherByteArrayPtr) {
	ppl7::ByteArrayPtr b1(binarydata,sizeof(binarydata));
	ppl7::ByteArrayPtr b2;
	b2.use(b1);
	ASSERT_EQ((const void*)&binarydata,b2.adr());
	ASSERT_EQ(sizeof(binarydata),b2.size());
}


TEST_F(ByteArrayPtrTest, Get) {
	ppl7::ByteArrayPtr b2(binarydata,sizeof(binarydata));
	EXPECT_EQ(13,b2.get(0));
	EXPECT_EQ(14,b2.get(1));
	EXPECT_EQ(55,b2.get(2));
	EXPECT_EQ(0,b2.get(7));
	EXPECT_EQ(23,b2.get(9));
	EXPECT_EQ(18,b2.get(10));

	ASSERT_THROW(b2.get(11),ppl7::OutOfBoundsEception);
}

TEST_F(ByteArrayPtrTest, OperatorGet) {
	ppl7::ByteArrayPtr b2(binarydata,sizeof(binarydata));
	EXPECT_EQ(13,b2[0]);
	EXPECT_EQ(14,b2[1]);
	EXPECT_EQ(55,b2[2]);
	EXPECT_EQ(0,b2[7]);
	EXPECT_EQ(23,b2[9]);
	EXPECT_EQ(18,b2[10]);
	ASSERT_THROW(b2[11],ppl7::OutOfBoundsEception);
}

TEST_F(ByteArrayPtrTest, Set) {
	unsigned char localdata[]={13,14,55,66,129,255,66,0,76,23,18};
	ppl7::ByteArrayPtr b2(localdata,sizeof(localdata));
	EXPECT_EQ(13,b2[0]);
	b2.set(0,69);
	EXPECT_EQ(69,b2[0]);
	b2.set(10,123);
	EXPECT_EQ(123,b2[10]);
	ASSERT_THROW(b2.set(11,42),ppl7::OutOfBoundsEception);
}

TEST_F(ByteArrayPtrTest, toHex) {
	ppl7::ByteArrayPtr b2(binarydata,sizeof(binarydata));
	ASSERT_EQ(ppl7::String("0d0e374281ff42004c1712"),b2.toHex());
}

TEST_F(ByteArrayPtrTest, toBase64) {
	////0d,0e,37,42,81,ff,42,00,4c,17,12
	ppl7::ByteArrayPtr b2;
	b2.use(binarydata,7);
	EXPECT_EQ(ppl7::String("DQ43QoH/Qo=="),b2.toBase64());
	b2.use(binarydata,8);
	EXPECT_EQ(ppl7::String("DQ43QoH/QgD="),b2.toBase64());
	b2.use(binarydata,9);
	EXPECT_EQ(ppl7::String("DQ43QoH/QgBM"),b2.toBase64());
	b2.use(binarydata,10);
	EXPECT_EQ(ppl7::String("DQ43QoH/QgBMFw=="),b2.toBase64());
	b2.use(binarydata,11);
	EXPECT_EQ(ppl7::String("DQ43QoH/QgBMFxJ="),b2.toBase64());

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

