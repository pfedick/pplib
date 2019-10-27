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
class PeekAndPokeTest : public ::testing::Test {
	protected:
	PeekAndPokeTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~PeekAndPokeTest() {

	}
};

TEST_F(PeekAndPokeTest, PokeFloat) {
	unsigned char buffer[8]={0,0,0,0,0,0,0,0};
	ppl7::PokeFloat(buffer,54785.957031);
	ASSERT_EQ(0xf5,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0x01,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0x56,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0x47,buffer[3]) << "Unexpected Value";
}

TEST_F(PeekAndPokeTest, PeekFloat) {
	unsigned char buffer[8]={0xf5,0x01,0x56,0x47,0,0,0,0};
	float f=ppl7::PeekFloat(buffer);
	ASSERT_EQ((float)54785.957031,f) << "Unexpected Value";
}

TEST_F(PeekAndPokeTest, Poke8) {
	unsigned char buffer[32];
	memset(buffer,0,32);
	ppl7::Poke8(buffer,123);
	ASSERT_EQ(123,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

	ppl7::Poke8(buffer,1);
	ASSERT_EQ(1,buffer[0]) << "Unexpected Value";
	ppl7::Poke8(buffer,255);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ppl7::Poke8(buffer,129);
	ASSERT_EQ(129,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

}

TEST_F(PeekAndPokeTest, Poke16) {
	unsigned char buffer[32];
	memset(buffer,0,32);
	ppl7::Poke16(buffer,123);
	ASSERT_EQ(123,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ppl7::Poke16(buffer,65535);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[1]) << "Unexpected Value";
	ppl7::Poke16(buffer,256);
	ASSERT_EQ(0,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(1,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

}

TEST_F(PeekAndPokeTest, Poke24) {
	unsigned char buffer[32];
	memset(buffer,0,32);
	ppl7::Poke24(buffer,123);
	ASSERT_EQ(123,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	memset(buffer,0,32);
	ppl7::Poke24(buffer,65535);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	memset(buffer,0,32);
	ppl7::Poke24(buffer,0xffffff);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[2]) << "Unexpected Value";
	memset(buffer,0,32);
	ppl7::Poke24(buffer,0x2ae9a4);
	ASSERT_EQ(0xa4,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0xe9,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0x2a,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

}

TEST_F(PeekAndPokeTest, Poke32) {
	unsigned char buffer[32];
	memset(buffer,0,32);
	ppl7::Poke32(buffer,123);
	ASSERT_EQ(123,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";
	memset(buffer,0,32);
	ppl7::Poke32(buffer,65535);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";
	memset(buffer,0,32);
	ppl7::Poke32(buffer,0xffffffff);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";
	memset(buffer,0,32);
	ppl7::Poke32(buffer,0xf02ae9a4);
	ASSERT_EQ(0xa4,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0xe9,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0x2a,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0xf0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

}

TEST_F(PeekAndPokeTest, Poke64) {
	unsigned char buffer[32];
	memset(buffer,0,32);
	ppl7::Poke64(buffer,123);
	ASSERT_EQ(123,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";
	memset(buffer,0,32);
	ppl7::Poke64(buffer,65535);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

	memset(buffer,0,32);
	ppl7::Poke64(buffer,0xffffffff);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

	memset(buffer,0,32);
	ppl7::Poke64(buffer,0xf02ae9a4);
	ASSERT_EQ(0xa4,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0xe9,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0x2a,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0xf0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

	memset(buffer,0,32);
	ppl7::Poke64(buffer,0xffffffffffffffff);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[7]) << "Unexpected Value";

	memset(buffer,0,32);
	ppl7::Poke64(buffer,0xa1379deef02ae9a4);
	ASSERT_EQ(0xa4,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0xe9,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0x2a,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0xf0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0xee,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0x9d,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0x37,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0xa1,buffer[7]) << "Unexpected Value";
}


TEST_F(PeekAndPokeTest, PokeN8) {
	unsigned char buffer[32];
	memset(buffer,0,32);
	ppl7::PokeN8(buffer,123);
	ASSERT_EQ(123,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

	ppl7::PokeN8(buffer,1);
	ASSERT_EQ(1,buffer[0]) << "Unexpected Value";
	ppl7::PokeN8(buffer,255);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ppl7::PokeN8(buffer,129);
	ASSERT_EQ(129,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

}

TEST_F(PeekAndPokeTest, PokeN16) {
	unsigned char buffer[32];
	memset(buffer,0,32);
	ppl7::PokeN16(buffer,123);
	ASSERT_EQ(0,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(123,buffer[1]) << "Unexpected Value";
	ppl7::PokeN16(buffer,65535);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[1]) << "Unexpected Value";
	ppl7::PokeN16(buffer,256);
	ASSERT_EQ(1,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

}

TEST_F(PeekAndPokeTest, PokeN32) {
	unsigned char buffer[32];
	memset(buffer,0,32);
	ppl7::PokeN32(buffer,123);
	ASSERT_EQ(0,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(123,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

	memset(buffer,0,32);
	ppl7::PokeN32(buffer,65535);
	ASSERT_EQ(0,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

	memset(buffer,0,32);
	ppl7::PokeN32(buffer,0xffffffff);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

	memset(buffer,0,32);
	ppl7::PokeN32(buffer,0xf02ae9a4);
	ASSERT_EQ(0xa4,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0xe9,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0x2a,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0xf0,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[7]) << "Unexpected Value";

}

TEST_F(PeekAndPokeTest, PokeN64) {
	unsigned char buffer[32];
	memset(buffer,0,32);
	ppl7::PokeN64(buffer,123);
	ASSERT_EQ(0,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(123,buffer[7]) << "Unexpected Value";
	memset(buffer,0,32);
	ppl7::PokeN64(buffer,65535);
	ASSERT_EQ(0,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[7]) << "Unexpected Value";

	memset(buffer,0,32);
	ppl7::PokeN64(buffer,0xffffffff);
	ASSERT_EQ(0,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[7]) << "Unexpected Value";

	memset(buffer,0,32);
	ppl7::PokeN64(buffer,0xf02ae9a4);
	ASSERT_EQ(0xa4,buffer[7]) << "Unexpected Value";
	ASSERT_EQ(0xe9,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0x2a,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0xf0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[0]) << "Unexpected Value";

	memset(buffer,0,32);
	ppl7::PokeN64(buffer,0xffffffffffffffff);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[7]) << "Unexpected Value";

	memset(buffer,0,32);
	ppl7::PokeN64(buffer,0xa1379deef02ae9a4);
	ASSERT_EQ(0xa4,buffer[7]) << "Unexpected Value";
	ASSERT_EQ(0xe9,buffer[6]) << "Unexpected Value";
	ASSERT_EQ(0x2a,buffer[5]) << "Unexpected Value";
	ASSERT_EQ(0xf0,buffer[4]) << "Unexpected Value";
	ASSERT_EQ(0xee,buffer[3]) << "Unexpected Value";
	ASSERT_EQ(0x9d,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0x37,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0xa1,buffer[0]) << "Unexpected Value";
}


TEST_F(PeekAndPokeTest, Peek8) {
	unsigned char buffer1[8]={0xa4,0xe9,0x2a,0xf0,0xee,0x9d,0x47,0xa1};
	unsigned char buffer2[8]={0,0,0,0,0,0,0,0};
	unsigned char buffer3[8]={255,255,255,255,255,255,255,255,};
	ASSERT_EQ((uint32_t)0xa4,ppl7::Peek8(buffer1)) << "Unexpected Value";
	ASSERT_EQ((uint32_t)0,ppl7::Peek8(buffer2)) << "Unexpected Value";
	ASSERT_EQ((uint32_t)0xff,ppl7::Peek8(buffer3)) << "Unexpected Value";
}

TEST_F(PeekAndPokeTest, Peek16) {
	unsigned char buffer1[8]={0xa4,0xe9,0x2a,0xf0,0xee,0x9d,0x47,0xa1};
	unsigned char buffer2[8]={0,0,0,0,0,0,0,0};
	unsigned char buffer3[8]={255,255,255,255,255,255,255,255,};
	ASSERT_EQ((uint32_t)0xe9a4,ppl7::Peek16(buffer1)) << "Unexpected Value";
	ASSERT_EQ((uint32_t)0,ppl7::Peek16(buffer2)) << "Unexpected Value";
	ASSERT_EQ((uint32_t)0xffff,ppl7::Peek16(buffer3)) << "Unexpected Value";
}

TEST_F(PeekAndPokeTest, Peek24) {
	unsigned char buffer1[8]={0xa4,0xe9,0x2a,0xf0,0xee,0x9d,0x47,0xa1};
	unsigned char buffer2[8]={0,0,0,0,0,0,0,0};
	unsigned char buffer3[8]={255,255,255,255,255,255,255,255,};
	ASSERT_EQ((uint32_t)0x2ae9a4,ppl7::Peek24(buffer1)) << "Unexpected Value";
	ASSERT_EQ((uint32_t)0,ppl7::Peek24(buffer2)) << "Unexpected Value";
	ASSERT_EQ((uint32_t)0xffffff,ppl7::Peek24(buffer3)) << "Unexpected Value";
}

TEST_F(PeekAndPokeTest, Peek32) {
	unsigned char buffer1[8]={0xa4,0xe9,0x2a,0xf0,0xee,0x9d,0x47,0xa1};
	unsigned char buffer2[8]={0,0,0,0,0,0,0,0};
	unsigned char buffer3[8]={255,255,255,255,255,255,255,255,};
	ASSERT_EQ((uint32_t)0xf02ae9a4,ppl7::Peek32(buffer1)) << "Unexpected Value";
	ASSERT_EQ((uint32_t)0,ppl7::Peek32(buffer2)) << "Unexpected Value";
	ASSERT_EQ((uint32_t)0xffffffff,ppl7::Peek32(buffer3)) << "Unexpected Value";
}

TEST_F(PeekAndPokeTest, Peek64) {
	unsigned char buffer1[8]={0xa4,0xe9,0x2a,0xf0,0xee,0x9d,0x47,0xa1};
	unsigned char buffer2[8]={0,0,0,0,0,0,0,0};
	unsigned char buffer3[8]={255,255,255,255,255,255,255,255,};
	ASSERT_EQ((uint64_t)0xa1479deef02ae9a4,ppl7::Peek64(buffer1)) << "Unexpected Value";
	ASSERT_EQ((uint64_t)0,ppl7::Peek64(buffer2)) << "Unexpected Value";
	ASSERT_EQ((uint64_t)0xffffffffffffffff,ppl7::Peek64(buffer3)) << "Unexpected Value";
}

TEST_F(PeekAndPokeTest, PeekN8) {
	unsigned char buffer1[8]={0xa4,0xe9,0x2a,0xf0,0xee,0x9d,0x47,0xa1};
	unsigned char buffer2[8]={0,0,0,0,0,0,0,0};
	unsigned char buffer3[8]={255,255,255,255,255,255,255,255,};
	ASSERT_EQ((uint32_t)0xa4,ppl7::PeekN8(buffer1)) << "Unexpected Value";
	ASSERT_EQ((uint32_t)0,ppl7::PeekN8(buffer2)) << "Unexpected Value";
	ASSERT_EQ((uint32_t)0xff,ppl7::PeekN8(buffer3)) << "Unexpected Value";
}

TEST_F(PeekAndPokeTest, PeekN16) {
	unsigned char buffer1[8]={0xa4,0xe9,0x2a,0xf0,0xee,0x9d,0x47,0xa1};
	unsigned char buffer2[8]={0,0,0,0,0,0,0,0};
	unsigned char buffer3[8]={255,255,255,255,255,255,255,255,};
	ASSERT_EQ((uint32_t)0xa4e9,ppl7::PeekN16(buffer1)) << "Unexpected Value";
	ASSERT_EQ((uint32_t)0,ppl7::PeekN16(buffer2)) << "Unexpected Value";
	ASSERT_EQ((uint32_t)0xffff,ppl7::PeekN16(buffer3)) << "Unexpected Value";
}

TEST_F(PeekAndPokeTest, PeekN32) {
	unsigned char buffer1[8]={0xa4,0xe9,0x2a,0xf0,0xee,0x9d,0x47,0xa1};
	unsigned char buffer2[8]={0,0,0,0,0,0,0,0};
	unsigned char buffer3[8]={255,255,255,255,255,255,255,255,};
	ASSERT_EQ((uint32_t)0xa4e92af0,ppl7::PeekN32(buffer1)) << "Unexpected Value";
	ASSERT_EQ((uint32_t)0,ppl7::PeekN32(buffer2)) << "Unexpected Value";
	ASSERT_EQ((uint32_t)0xffffffff,ppl7::PeekN32(buffer3)) << "Unexpected Value";
}

TEST_F(PeekAndPokeTest, PeekN64) {
	unsigned char buffer1[8]={0xa4,0xe9,0x2a,0xf0,0xee,0x9d,0x47,0xa1};
	unsigned char buffer2[8]={0,0,0,0,0,0,0,0};
	unsigned char buffer3[8]={255,255,255,255,255,255,255,255,};
	ASSERT_EQ((uint64_t)0xa4e92af0ee9d47a1,ppl7::PeekN64(buffer1)) << "Unexpected Value";
	ASSERT_EQ((uint64_t)0,ppl7::PeekN64(buffer2)) << "Unexpected Value";
	ASSERT_EQ((uint64_t)0xffffffffffffffff,ppl7::PeekN64(buffer3)) << "Unexpected Value";
}

}


