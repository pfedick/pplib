/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (ppl7).
 * Web: http://www.pfp.de/ppl/
 *
 *******************************************************************************
 * Copyright (c) 2015, Patrick Fedick <patrick@pfp.de>
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

#include "../include/prolog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include "../include/ppl7.h"
#include <gtest/gtest.h>
#include "ppl7-tests.h"


namespace ppl7 {

class IconvTest : public ::testing::Test {
	protected:
		IconvTest() {
			if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
				printf ("setlocale fehlgeschlagen\n");
				throw std::exception();
			}
		}
		virtual ~IconvTest() {

		}
};

TEST_F(IconvTest, ConstructorWithoutFile) {
	ASSERT_NO_THROW({
		ppl7::Iconv iconv;
	});
}

TEST_F(IconvTest, enumerateCharsets) {
	ppl7::Array list;
	ppl7::Iconv::enumerateCharsets(list);
	ASSERT_GT(list.size(),(size_t)10);
	//list.list();
	ASSERT_TRUE(list.has("US-ASCII"));
	ASSERT_TRUE(list.has("UTF-8"));
	ASSERT_TRUE(list.has("LATIN1"));
	ASSERT_TRUE(list.has("UTF-16"));
	ASSERT_TRUE(list.has("UTF-32"));
}

TEST_F(IconvTest, getLocalCharset) {
#ifdef WIN32
	ASSERT_EQ("UTF-8",ppl7::Iconv::getLocalCharset());
#else
	ASSERT_EQ(ppl7::String("UTF-8"),ppl7::Iconv::getLocalCharset());
#endif
}

TEST_F(IconvTest, initWithKnownCharsets) {
	ppl7::Iconv iconv;
	ASSERT_NO_THROW({
		iconv.init("ISO-8859-1","UTF-8");
	});
}

TEST_F(IconvTest, initWithUnknownCharsetsThrowsException) {
	ppl7::Iconv iconv;
	ASSERT_THROW({
		iconv.init("ISO-PATRICKF","UTF-8");
	},ppl7::UnsupportedCharacterEncodingException);
}

static const char test_string_iso88591[] =
		{(char)0x48,(char)0xe4,(char)0x6c,(char)0x6c,(char)0x6f,(char)0x20,
		 (char)0x57,(char)0xf6,(char)0x72,(char)0x6c,(char)0x64};

static const char test_string_utf16be[] =
{
		0x00,0x48,0x00,0xe4,0x00,0x6c,0x00,0x6c,0x00,0x6f,0x00,0x20,0x00,0x57,0x00,0xf6,
		0x00,0x72,0x00,0x6c,0x00,0x64,0x00,0x21
};

TEST_F(IconvTest, transcodeWithString_ISO88591_UTF8) {
	ppl7::Iconv iconv;
	iconv.init("ISO-8859-1","UTF-8");
	ppl7::String source(test_string_iso88591);
	ppl7::String target;
	iconv.transcode(source,target);
	ASSERT_EQ(ppl7::String("Hällo Wörld"),target);
}

TEST_F(IconvTest, transcodeByteArray_UTF16BE_UTF8) {
	ppl7::String expected("Hällo Wörld!");
	ppl7::Iconv iconv;
	iconv.init("UTF-16BE","UTF-8");
	ppl7::ByteArray source(test_string_utf16be,sizeof(test_string_utf16be));
	ppl7::ByteArray target;
	iconv.transcode(source,target);
	ASSERT_EQ(ppl7::ByteArray(expected),target);
}

TEST_F(IconvTest, transcodeWithByteArray) {
	ppl7::Iconv iconv;
	ppl7::String expected("Hällo Wörld");
	iconv.init("ISO-8859-1","UTF-8");
	ppl7::ByteArray source(test_string_iso88591,sizeof(test_string_iso88591));
	ppl7::ByteArray target;
	iconv.transcode(source,target);
	ASSERT_EQ(ppl7::ByteArray(expected),target);
}


}	// EOF namespace

