/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *******************************************************************************
 * Copyright (c) 2017, Patrick Fedick <patrick@pfp.de>
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

#include "prolog_ppl7.h"
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "ppl7.h"
#include "ppl7-audio.h"


namespace ppl7 {

AudioDecoder_Wave::AudioDecoder_Wave()
{
	position=0;
	samplesize=0;
	ff=NULL;
}

AudioDecoder_Wave::~AudioDecoder_Wave()
{

}

void AudioDecoder_Wave::open(FileObject &file, const AudioInfo *info)
{
	if (!info) {
		if (!IdentAudioFile(file, this->info)) {
			throw UnsupportedAudioFormatException();
		}
	} else {
		this->info=*info;
	}
	if (this->info.Format!=AudioInfo::WAVE) throw UnsupportedAudioFormatException();
	//if (this->info.Frequency!=44100) throw UnsupportedAudioFormatException("Frequency != 44100");
	//if (this->info.Bitrate!=1411) throw UnsupportedAudioFormatException("Bitrate != 1411");
	if (this->info.Channels!=2) throw UnsupportedAudioFormatException("Channels != 2");
	if (this->info.BitsPerSample!=16) throw UnsupportedAudioFormatException("BitsPerSample != 16");
	if (this->info.BytesPerSample!=4) throw UnsupportedAudioFormatException("BytesPerSample != 4");
	position=0;
	this->ff=&file;
}

const AudioInfo & AudioDecoder_Wave::getAudioInfo() const
{
	return info;
}

void AudioDecoder_Wave::getAudioInfo(AudioInfo &info) const
{
	info=this->info;
}

void AudioDecoder_Wave::seekSample(size_t sample)
{
	if (sample<info.Samples) position=sample;
	else position=info.Samples;
}

size_t AudioDecoder_Wave::getPosition() const
{
	return position;
}


size_t AudioDecoder_Wave::getSamples(size_t num, STEREOSAMPLE16 *interleafed)
{
	size_t samples=num;
	if (position+samples>info.Samples) samples=info.Samples-position;
	ff->seek(info.AudioStart+position*info.BytesPerSample);
	size_t read=ff->read(interleafed,samples*info.BytesPerSample)/info.BytesPerSample;
	position+=read;
	return read;
}

size_t AudioDecoder_Wave::addSamples(size_t num, STEREOSAMPLE32 *buffer)
{
	size_t samples=num;
	if (position+samples>info.Samples) samples=info.Samples-position;
	const char *data=ff->map(info.AudioStart+position*info.BytesPerSample, samples*info.BytesPerSample);
	if (info.BitsPerSample==16) {
		for (size_t i=0;i<samples;i++) {
			buffer[i].left+=Peek16(data);
			buffer[i].right+=Peek16(data+2);
			data+=4;
		}
	}
	position+=samples;
	return samples;
}


size_t AudioDecoder_Wave::getSamples(size_t num, SAMPLE16 *left, SAMPLE16 *right)
{
	size_t samples=num;
	if (position+samples>info.Samples) samples=info.Samples-position;
	const char *data=ff->map(info.AudioStart+position*info.BytesPerSample, samples*info.BytesPerSample);
	if (info.BitsPerSample==16) {
		for (size_t i=0;i<samples;i++) {
			left[i]=Peek16(data);
			right[i]=Peek16(data+2);
			data+=4;
		}
	}
	position+=samples;
	return samples;
}

size_t AudioDecoder_Wave::getSamples(size_t num, float *left, float *right)
{
	size_t samples=num;
	if (position+samples>info.Samples) samples=info.Samples-position;
	const char *data=ff->map(info.AudioStart+position*info.BytesPerSample, samples*info.BytesPerSample);
	int16_t *buffer16=(int16_t*)data;
	for (size_t i=0;i<samples;i++) {
		left[i]=(float)buffer16[i*2]/32768.0;
		right[i]=(float)buffer16[i*2+1]/32768.0;
	}
	position+=samples;
	return samples;
}

}	// EOF namespace ppl7

