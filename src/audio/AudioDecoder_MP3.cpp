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

#ifdef HAVE_MPG123_H
#include <mpg123.h>
#endif

namespace ppl7 {

#ifdef HAVE_MPG123
static Mutex MPG123_GlobalMutex;
static uint64_t mpg123_instances=0;
#define READBUFFER	1024*1024
#define OUTBUFF 32768
#endif

AudioDecoder_MP3::AudioDecoder_MP3()
{
	position=0;
	samplesize=0;
	ff=NULL;
	decoder=NULL;
	readbuffer=NULL;
	outbuffer=NULL;
	out_offset=0;
	out_size=0;
	isRunning=false;
	needInput=false;
#ifdef HAVE_MPG123
	MPG123_GlobalMutex.lock();
	if (!mpg123_instances) {
		int ret=mpg123_init();
		if (ret!=MPG123_OK) {
			MPG123_GlobalMutex.unlock();
			throw DecoderInitializationException("mpg123_init: %s", mpg123_plain_strerror(ret));
		}
	}
	mpg123_instances++;
	MPG123_GlobalMutex.unlock();
#endif
}

AudioDecoder_MP3::~AudioDecoder_MP3()
{
	free(readbuffer);
	free(outbuffer);
#ifdef HAVE_MPG123
	if (decoder) mpg123_delete((mpg123_handle*)decoder);
	MPG123_GlobalMutex.lock();
	if (mpg123_instances) {
		mpg123_instances--;
		if (mpg123_instances==0) {
			mpg123_exit();
		}
	}
	MPG123_GlobalMutex.unlock();

#endif
}

void AudioDecoder_MP3::open(FileObject &file, const AudioInfo *info)
{
#ifndef HAVE_MPG123
	throw ppl7::UnsupportedFeatureException("AudioDecoder_MP3: mpg123");
#else

	if (!info) {
		if (!IdentAudioFile(file, this->info)) {
			throw UnsupportedAudioFormatException();
		}
	} else {
		this->info=*info;
	}
	if (this->info.Format!=AudioInfo::MP3) throw UnsupportedAudioFormatException();
	position=0;
	this->ff=&file;
	if (decoder) mpg123_delete((mpg123_handle*)decoder);
	int err;
	decoder=mpg123_new(NULL,&err);
	if (!decoder) throw DecoderInitializationException("mpg123_new: %s", mpg123_plain_strerror(err));
	err=mpg123_open_feed((mpg123_handle*)decoder);
	if (err!=MPG123_OK) throw DecoderException("mpg123_open_feed: %s", mpg123_plain_strerror(err));
	mpg123_param((mpg123_handle*)decoder, MPG123_VERBOSE,2,0);
	mpg123_format_none((mpg123_handle*)decoder);
	mpg123_format((mpg123_handle*)decoder,44100,2, MPG123_ENC_SIGNED_16);
	out_offset=0;
	out_size=0;
	isRunning=true;
	needInput=true;
	// Buffer Reservieren, falls noch nicht vorhanden
	if (!readbuffer) {
		readbuffer=(uint8_t*)malloc(READBUFFER);
		if (!readbuffer) throw OutOfMemoryException();
	}
	if (!outbuffer) {
		outbuffer=(uint8_t*)malloc(OUTBUFF);
		if (!outbuffer) throw OutOfMemoryException();
	}
	this->ff->seek(this->info.AudioStart);
#endif
}

const AudioInfo & AudioDecoder_MP3::getAudioInfo() const
{
	return info;
}

void AudioDecoder_MP3::getAudioInfo(AudioInfo &info) const
{
	info=this->info;
}

void AudioDecoder_MP3::seekSample(size_t sample)
{
	if (sample==0) {
		ff->seek(info.AudioStart);
		return;
	}
	throw ppl7::UnsupportedFeatureException("AudioDecoder_MP3::seekSample");
	//if (sample<info.Samples) position=sample;
	//else position=info.Samples;
}

size_t AudioDecoder_MP3::getPosition() const
{
	return position;
}

size_t AudioDecoder_MP3::fillDecodeBuffer()
{
#ifdef HAVE_MPG123
	size_t size=0;
	int ret;
	while (size==0) {
		ret=mpg123_read((mpg123_handle*)decoder,outbuffer,OUTBUFF,&size);
		if (ret==MPG123_NEED_MORE) {
			size_t restbytes=ff->size()-ff->tell();
			size_t maxbytes=READBUFFER;
			if (maxbytes>restbytes) maxbytes=restbytes;
			if (maxbytes==0) return 0;
			size_t len = ff->read(readbuffer,maxbytes);
			if (len==0) return 0;
			ret=mpg123_feed((mpg123_handle*)decoder,readbuffer,len);
			if (ret!=MPG123_OK) {
				throw DecoderException("mpg123_feed: %s",mpg123_strerror((mpg123_handle*)decoder));
			}
		} else if (ret==MPG123_NEW_FORMAT) {
			long rate;
			int channels, enc;
			mpg123_getformat((mpg123_handle*)decoder, &rate, &channels, &enc);
		} else if (ret!=MPG123_OK) {
			throw DecoderException("mpg123_read: %s",mpg123_strerror((mpg123_handle*)decoder));
		}
	}
	out_size=size;
	out_offset=0;
	return size;
#else
	return 0;
#endif
}



size_t AudioDecoder_MP3::getSamples(size_t num, STEREOSAMPLE16 *interleafed)
{
#ifndef HAVE_MPG123
	throw ppl7::UnsupportedFeatureException("AudioDecoder_MP3: mpg123");
#else
	size_t rest=num;
	while (rest) {
		size_t av=out_size-out_offset;
		if (av>0) {
			size_t s=(int)(av/4);
			if (s>rest) s=rest;
			STEREOSAMPLE16 *p=(STEREOSAMPLE16*)(outbuffer+out_offset);
			memcpy(interleafed, p,s*4);
			interleafed+=s;
			rest-=s;
			out_offset+=(s*4);
		} else {
			if (!fillDecodeBuffer()) break;
		}
	}
	position+=num-rest;
	return num-rest;

#endif
}

size_t AudioDecoder_MP3::addSamples(size_t num, STEREOSAMPLE32 *buffer)
{
#ifndef HAVE_MPG123
	throw ppl7::UnsupportedFeatureException("AudioDecoder_MP3: mpg123");
#else
	size_t rest=num;
	while (rest) {
		size_t av=out_size-out_offset;
		if (av>0) {
			size_t s=(int)(av/4);
			if (s>rest) s=rest;
			STEREOSAMPLE16 *p=(STEREOSAMPLE16*)(outbuffer+out_offset);
			for (size_t i=0;i<s;i++) {
				buffer[i].left+=p[i].left;
				buffer[i].right+=p[i].right;
			}
			//memcpy(interleafed, p,s*4);
			buffer+=s;
			rest-=s;
			out_offset+=(s*4);
		} else {
			if (!fillDecodeBuffer()) break;
		}
	}
	position+=num-rest;
	return num-rest;

#endif
}


size_t AudioDecoder_MP3::getSamples(size_t num, SAMPLE16 *left, SAMPLE16 *right)
{
#ifndef HAVE_MPG123
	throw ppl7::UnsupportedFeatureException("AudioDecoder_MP3: mpg123");
#else
	size_t rest=num;
	while (rest) {
		size_t av=out_size-out_offset;
		if (av>0) {
			size_t s=(int)(av/4);
			if (s>rest) s=rest;
			STEREOSAMPLE16 *p=(STEREOSAMPLE16*)(outbuffer+out_offset);
			for (size_t i=0;i<s;i++) {
				left[i]=p[i].left;
				right[i]=p[i].right;
			}
			left+=s;
			right+=s;
			rest-=s;
			out_offset+=(s*4);
		} else {
			if (!fillDecodeBuffer()) break;
		}
	}
	position+=num-rest;
	return num-rest;
#endif
}

size_t AudioDecoder_MP3::getSamples(size_t num, float *left, float *right)
{
	throw ppl7::UnsupportedFeatureException("AudioDecoder_MP3::getSamples(size_t num, float *left, float *right)");
}

}	// EOF namespace ppl7

