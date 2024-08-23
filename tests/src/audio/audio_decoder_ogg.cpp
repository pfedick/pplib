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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <ppl7.h>
#include <ppl7-audio.h>
#include <gtest/gtest.h>
#include "ppl7-tests.h"


namespace ppl7 {


class AudioDecoder_Ogg_Test : public ::testing::Test {
protected:
	AudioDecoder_Ogg_Test() {
		if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
			printf("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::Dir::mkDir("tmp");
	}
	virtual ~AudioDecoder_Ogg_Test() {

	}
};

TEST_F(AudioDecoder_Ogg_Test, DecodeOggVorbis) {
	ppl7::File file;
	file.open("testdata/test_44kHz.ogg");
	ppl7::AudioDecoder_Ogg ogg;
	ogg.open(file);


	STEREOSAMPLE16* samples=(STEREOSAMPLE16*)malloc(sizeof(STEREOSAMPLE16) * 1024);
	ASSERT_TRUE(samples != NULL);
	size_t num_read=ogg.getSamples(1024, samples);
	ASSERT_EQ(num_read, 1024);



}





}	// EOF namespace
