/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.8 $
 * $Date: 2010/07/26 10:28:51 $
 * $Id: CMP3Decode_Mpg123.cpp,v 1.8 2010/07/26 10:28:51 pafe Exp $
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

#include "prolog.h"
#include <string.h>
#include <limits.h>
#include "ppl6.h"
#include "ppl6-sound.h"

#ifdef HAVE_MPG123_H
#include <mpg123.h>
#endif

#undef DEBUG

#ifdef DEBUG
	#define debugprinterror() ppl6::PushError(); { \
		ppl6::CString debugerror, debugdate; \
		ppl6::Error2String(debugerror); \
		ppl6::MkDate(debugdate,"%d.%m.%Y-%H:%M:%S",ppl6::GetTime()); \
		printf ("%s.%i [%s:%i] ",(const char*)debugdate,(int)(ppl6::GetMilliSeconds()%1000),__FILE__,__LINE__); \
		printf ("%s",(const char*)debugerror); } ppl6::PopError();

	#define debugprintf(format, ...) ppl6::PushError(); { \
		ppl6::CString debugdate; \
		ppl6::MkDate(debugdate,"%d.%m.%Y-%H:%M:%S",ppl6::GetTime()); \
		printf ("%s.%i [%s:%i] ",(const char*)debugdate,(int)(ppl6::GetMilliSeconds()%1000),__FILE__,__LINE__); \
		printf (format, ##__VA_ARGS__); } ppl6::PopError();
#else
	#define debugprintf(format, ...) {}
	#define debugprinterror() {}
#endif

namespace ppl6 {


//#define READBUFFER	131072
#define READBUFFER	1024*1024
#define OUTBUFF 32768


static CMutex MPG123_GlobalMutex;
static ppluint32 instances=0;

CMP3DecodeLibMpg123::CMP3DecodeLibMpg123()
{
	isInitialized=false;
	file=NULL;
	needInput=false;
	hh=NULL;
	ff=NULL;
	readbuffer=NULL;
	outbuffer=NULL;
	out_offset=0;
	out_size=0;
#ifdef HAVE_MPG123
	MPG123_GlobalMutex.Lock();
	if (instances==0) {
		int ret=mpg123_init();
		if (ret!=MPG123_OK) {
			MPG123_GlobalMutex.Unlock();
			ppl6::SetError(200,"mpg123 library: %s",mpg123_plain_strerror(ret));
			debugprintf("CMP3DecodeLibMpg123: mpg123_init fehlgeschlagen\n");
			debugprinterror();
			throw ppl6::Exception();
		}
	}
	instances++;
	isInitialized=true;
	MPG123_GlobalMutex.Unlock();
#endif
}

CMP3DecodeLibMpg123::~CMP3DecodeLibMpg123()
{
	PushError();
	Clear();
#ifdef HAVE_MPG123
	MPG123_GlobalMutex.Lock();
	if (instances>0 && isInitialized==true) {
		instances--;
		if (instances==0) {
			mpg123_exit();
		}
	}
	MPG123_GlobalMutex.Unlock();
#endif
	PopError();
}

void CMP3DecodeLibMpg123::Clear()
{
	Stop();
	delete file;
	file=NULL;
	ff=NULL;
}

int CMP3DecodeLibMpg123::LoadFile(const CString &filename)
{
#ifdef HAVE_MPG123
	Clear();
	file=new CFile();
	if (!file->Open(filename,"rb")) {
		delete file;
		return 0;
	}
	int ret=LoadFile(*file);
	if (!ret) {
		PushError();
		delete file;
		PopError();
	}
	return ret;
#else
	ppl6::SetError(552);
	return 0;
#endif

}

int CMP3DecodeLibMpg123::LoadFile(CFileObject &file)
{
#ifdef HAVE_MPG123
	if (!IdentMPEG(&file,&mpg)) return 0;
	//PrintMpegHeader(&mpg);
	ff=&file;
	return 1;
#else
	ppl6::SetError(552);
	return 0;
#endif

}

int CMP3DecodeLibMpg123::GetMpegHeader(PPL_MPEG_HEADER *mpg)
{
#ifdef HAVE_MPG123
	if (!mpg) {
		SetError(194,"PPL_MPEG_HEADER *mpg");
		return 0;
	}
	if (!ff) {
		SetError(239);
		return 0;
	}
	memcpy(mpg,&this->mpg,sizeof(PPL_MPEG_HEADER));
	return 1;
#else
	ppl6::SetError(552);
	return 0;
#endif

}

int CMP3DecodeLibMpg123::Start()
{
#ifdef HAVE_MPG123
	Stop();
	if (!ff) {
		SetError(239);
		return 0;
	}
	// Buffer Reservieren, falls noch nicht vorhanden
	if (!readbuffer) {
		readbuffer=(ppluint8*)malloc(READBUFFER);
		if (!readbuffer) {
			SetError(2,"readbuffer");
			return 0;
		}
	}
	if (!outbuffer) {
		outbuffer=(ppluint8*)malloc(OUTBUFF);
		if (!outbuffer) {
			SetError(2,"outbuffer");
			return 0;
		}
	}
	out_offset=0;
	out_size=0;
	int e=0;
	hh=mpg123_new(NULL,&e);
	if (!hh) {
		SetError(553,"mpg123_new: %s",mpg123_plain_strerror(e));
		return 0;
	}

	e=mpg123_open_feed((mpg123_handle*)hh);
	if (e!=MPG123_OK) {
		SetError(553,"mpg123_new: %s",mpg123_strerror((mpg123_handle*)hh));
		return 0;
	}
	//mpg123_param((mpg123_handle*)hh, MPG123_RESYNC_LIMIT,8192,0);
	mpg123_param((mpg123_handle*)hh, MPG123_VERBOSE,2,0);

	mpg123_format_none((mpg123_handle*)hh);
	mpg123_format((mpg123_handle*)hh,44100,2, MPG123_ENC_SIGNED_16);



	ff->Seek(0);

	isRunning=true;
	needInput=true;
	return 1;
#else
	ppl6::SetError(552);
	return 0;
#endif

}

int CMP3DecodeLibMpg123::Stop()
{
#ifdef HAVE_MPG123
	free(readbuffer);
	readbuffer=NULL;
	free(outbuffer);
	outbuffer=NULL;
	if (hh) mpg123_delete((mpg123_handle*)hh);
	hh=NULL;
	isRunning=false;
	return 1;
#else
	ppl6::SetError(552);
	return 0;
#endif
}


int CMP3DecodeLibMpg123::FillDecodeBuffer()
{
#ifdef HAVE_MPG123
	debugprintf ("CMP3DecodeLibMpg123::FillDecodeBuffer: Versuche %i Bytes zu dekodieren\n",OUTBUFF);
	size_t size=0;
	int ret;
	while (size==0) {
		ret=mpg123_read((mpg123_handle*)hh,outbuffer,OUTBUFF,&size);
		if (ret==MPG123_NEED_MORE) {
			debugprintf("Lese %i Bytes ab File-Position %llu\n",READBUFFER,ff->Ftell());
			size_t len = ff->Read(readbuffer,READBUFFER);
			debugprintf("%i Bytes gelesen, übergebe an mpg123...\n",len);
			if (len==0) return 0;
			ret=mpg123_feed((mpg123_handle*)hh,readbuffer,len);
			debugprintf("Returncode von mpg123_feed: %i\n",ret);
			if (ret!=MPG123_OK) {
				SetError(553,"mpg123_new: %s",mpg123_strerror((mpg123_handle*)hh));
				debugprinterror();
				return 0;
			}
		} else if (ret==MPG123_NEW_FORMAT) {
			long rate;
			int channels, enc;
			mpg123_getformat((mpg123_handle*)hh, &rate, &channels, &enc);
			debugprintf("New format: %li Hz, %i channels, encoding value %i\n", rate, channels, enc);
		} else if (ret!=MPG123_OK) {
			SetError(553,"mpg123_new: %s",mpg123_strerror((mpg123_handle*)hh));
			debugprinterror();
			return 0;
		}
	}
	out_size=size;
	out_offset=0;
	return size;
#else
	return 0;
#endif
}

int CMP3DecodeLibMpg123::Decode(int samples, SAMPLE *left, SAMPLE *right)
{
#ifdef HAVE_MPG123
	debugprintf ("CMP3DecodeLibMpg123::Decode Aufruf, %i Samples gewünscht\n",samples);
	if (!samples) {
		SetError(258);
		return 0;
	}
	if (!left) {
		SetError(194,"SAMPLE *left");
		return 0;
	}
	if (!right) {
		SetError(194,"SAMPLE *right");
		return 0;
	}
	if (!isRunning) {
		SetError(257);
		return 0;
	}

	//ppl6::CFile out;
	//out.Open("debug.raw","ab");

	int rest=samples;
	while (rest) {
		debugprintf("Noch %i Samples Rest\n",rest);
		size_t av=out_size-out_offset;
		if (av>0) {
			int s=(int)(av/4);
			debugprintf ("Wir haben %u Bytes im Buffer = %i Samples\n",(unsigned int)av,s);
			if (s>rest) s=rest;
			debugprintf("Kopiere %i Samples\n",s);
			STEREOSAMPLE *p=(STEREOSAMPLE*)(outbuffer+out_offset);
			for (int i=0;i<s;i++) {
				left[i]=p[i].left;
				right[i]=p[i].right;
			}
			left+=s;
			right+=s;
			//out.Write(outbuffer+out_offset,s*4);
			rest-=s;
			out_offset+=(s*4);
			debugprintf ("out_offset ist jetzt: %u, noch %i Samples rest\n",(unsigned int)out_offset,(int)((out_size-out_offset)/4));
		} else {
			if (!FillDecodeBuffer()) break;
		}
	}
	debugprintf ("Gebe %i Samples zurück\n",(samples-rest));
	return samples-rest;
#else
	ppl6::SetError(552);
	return 0;
#endif

}

} // EOF namespace ppl6



