/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 75 $
 * $Date: 2011-05-04 19:35:00 +0000 (Mi, 04 Mai 2011) $
 * $Id: Variant.cpp 75 2011-05-04 19:35:00Z pafe $
 *
 *******************************************************************************
 * Copyright (c) 2010, Patrick Fedick <patrick@pfp.de>
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
#include <ppl7.h>
#include <gtest/gtest.h>

namespace {

// The fixture for testing class Foo.
class FunctionsTest : public ::testing::Test {
	protected:
	FunctionsTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~FunctionsTest() {

	}
};

TEST_F(FunctionsTest, Poke8) {
	unsigned char buffer[32];
	memset(buffer,0,32);
	ppl7::Poke8(buffer,123);
	ASSERT_EQ(123,buffer[0]) << "Unexpected Value";
	ppl7::Poke8(buffer,1);
	ASSERT_EQ(1,buffer[0]) << "Unexpected Value";
	ppl7::Poke8(buffer,255);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ppl7::Poke8(buffer,129);
	ASSERT_EQ(129,buffer[0]) << "Unexpected Value";
}

TEST_F(FunctionsTest, Poke16) {
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
}

TEST_F(FunctionsTest, Poke24) {
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
}

TEST_F(FunctionsTest, Poke32) {
	unsigned char buffer[32];
	memset(buffer,0,32);
	ppl7::Poke32(buffer,123);
	ASSERT_EQ(123,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	memset(buffer,0,32);
	ppl7::Poke32(buffer,65535);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0,buffer[3]) << "Unexpected Value";
	memset(buffer,0,32);
	ppl7::Poke32(buffer,0xffffffff);
	ASSERT_EQ(255,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(255,buffer[3]) << "Unexpected Value";
	memset(buffer,0,32);
	ppl7::Poke32(buffer,0xf02ae9a4);
	ASSERT_EQ(0xa4,buffer[0]) << "Unexpected Value";
	ASSERT_EQ(0xe9,buffer[1]) << "Unexpected Value";
	ASSERT_EQ(0x2a,buffer[2]) << "Unexpected Value";
	ASSERT_EQ(0xf0,buffer[3]) << "Unexpected Value";
}



}


int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}


