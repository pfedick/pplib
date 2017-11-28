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


class AudioEncoder_MP3_Test : public ::testing::Test {
	protected:
	AudioEncoder_MP3_Test() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::Dir::mkDir("tmp");
	}
	virtual ~AudioEncoder_MP3_Test() {

	}
};

TEST_F(AudioEncoder_MP3_Test, Constructor) {
	ASSERT_NO_THROW({
		ppl7::AudioEncoder_MP3 encoder;
	});
}

static void ProgressFunc(int progress, void *priv)
{
	printf ("%d %%\n",progress);
}

TEST_F(AudioEncoder_MP3_Test, EncodeCBRFromAiff) {
	ppl7::AudioEncoder_MP3 encoder;
	ppl7::AudioDecoder_Aiff decoder;
	ppl7::File file("testdata/test_44kHz_tagged.aiff");
	ppl7::ID3Tag tag;
	tag.load(file);
	decoder.open(file);
	try {
		encoder.setCBR(320,0);
		encoder.setStereoMode(AudioInfo::JOINT_STEREO);
		encoder.setProgressFunction(ProgressFunc, this);
		ppl7::File out("tmp/encoder_cbr320.mp3", ppl7::File::WRITE);
		encoder.startEncode(out);
		encoder.writeID3v2Tag(tag);
		encoder.encode(decoder);
		encoder.finish();
		encoder.writeID3v1Tag(tag);
	} catch (const ppl7::Exception &exp) {
		exp.print();
	}


}


}	// EOF namespace

