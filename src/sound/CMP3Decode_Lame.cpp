/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
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

#ifdef HAVE_LAME
#ifdef HAVE_LAME_LAME_H
#include <lame/lame.h>
#else
#include <lame.h>
#endif
#endif


#ifdef UNDER_CONSTRUCTION
#define READBUFFER	131072
#define OUTSAMPLES	131072


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


CMP3DecodeLibLame::CMP3DecodeLibLame()
{
#ifdef HAVE_LAME_HIP_DECODE
	lame=NULL;
	ff=NULL;
	file=NULL;
	readbuffer=NULL;
	isRunning=false;
	out_size=OUTSAMPLES;
	out_offset=0;
	out_samples=0;
	out_left=NULL;
	out_right=NULL;
#endif

}

CMP3DecodeLibLame::~CMP3DecodeLibLame()
{
	PushError();
	Clear();
	PopError();
}

void CMP3DecodeLibLame::Clear()
{
	Stop();
	delete file;
	file=NULL;
	ff=NULL;
}

int CMP3DecodeLibLame::LoadFile(const CString &filename)
{
#ifdef HAVE_LAME_HIP_DECODE
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
	ppl6::SetError(555,"lame");
	return 0;
#endif

}

int CMP3DecodeLibLame::LoadFile(CFileObject &file)
{
#ifdef HAVE_LAME_HIP_DECODE
	if (!IdentMPEG(&file,&mpg)) return 0;
	PrintMpegHeader(&mpg);
	ff=&file;
	return 1;
#else
	ppl6::SetError(555,"lame");
	return 0;
#endif

}

int CMP3DecodeLibLame::GetMpegHeader(PPL_MPEG_HEADER *mpg)
{
#ifdef HAVE_LAME_HIP_DECODE
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
	ppl6::SetError(555,"lame");
	return 0;
#endif
}

int CMP3DecodeLibLame::Start()
{
#ifdef HAVE_LAME_HIP_DECODE
	Stop();
	if (!ff) {
		SetError(239);
		return 0;
	}
	hip_t hip;
	hip=hip_decode_init();
	if (!hip) {
		SetError(557,"lame");
		return 0;
	}
	lame=hip;
	// Buffer Reservieren, falls noch nicht vorhanden
	if (!readbuffer) {
		readbuffer=(ppluint8*)malloc(READBUFFER);
		if (!readbuffer) {
			SetError(2,"readbuffer");
			return 0;
		}
	}
	if (!out_left) {
		out_left=(SAMPLE*)malloc(sizeof(SAMPLE)*out_size);
		if (!out_left) {
			SetError(2,"out_left");
			return 0;
		}
	}
	if (!out_right) {
		out_right=(SAMPLE*)malloc(sizeof(SAMPLE)*out_size);
		if (!out_right) {
			SetError(2,"out_right");
			return 0;
		}
	}

	out_offset=0;
	out_samples=0;
	ff->Seek(mpg.start);
	isRunning=true;
	return 1;
#else
	ppl6::SetError(555,"lame");
	return 0;
#endif

}

int CMP3DecodeLibLame::Stop()
{
#ifdef HAVE_LAME_HIP_DECODE
	free(readbuffer);
	readbuffer=NULL;
	free(out_left);
	free(out_right);
	out_left=NULL;
	out_right=NULL;
	if (lame) hip_decode_exit((hip_t)lame);
	lame=NULL;
	isRunning=false;
	return 1;
#else
	ppl6::SetError(555,"lame");
	return 0;
#endif
}

int CMP3DecodeLibLame::Decode(int num, SAMPLE *left, SAMPLE *right)
{
#ifdef HAVE_LAME_HIP_DECODE
	debugprintf("CMP3DecodeLibLame::Decode angefordert=%i, noch im buffer=%i\n",num,out_samples);

	if (!num) {
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
	//int ret=0;
	if (!isRunning) {
		SetError(257);
		return 0;
	}
	int rest=num;
	int filled=0;
	//int copy=0;
	// Falls noch Samples im DecodeBuffer sind, kopieren wir diese
	// zuerst
	if (out_samples) {
		if (rest>out_samples) {
			debugprintf("rest>out_samples, kopiere %i samples zum Anforderer\n",out_samples);
			memcpy(left,out_left+out_offset,out_samples*sizeof(SAMPLE));
			memcpy(right,out_right+out_offset,out_samples*sizeof(SAMPLE));
			left+=out_samples;
			right+=out_samples;
			rest-=out_samples;
			filled=out_samples;
			out_samples=0;
			debugprintf("rest=%i, out_samples=%i, filled=%i\n",rest,out_samples,filled);
		} else {
			debugprintf("rest<out_samples, kopiere %i samples zum Anforderer\n",out_samples);
			memcpy(left,out_left+out_offset,rest*sizeof(SAMPLE));
			memcpy(right,out_right+out_offset,rest*sizeof(SAMPLE));
			out_samples-=rest;
			out_offset+=rest;
			debugprintf("rest=%i, out_samples=%i, filled=%i, beende CMP3DecodeLibLame::Decode\n",0,out_samples,rest);
			return rest;
		}
	}

	// Dann lesen wir solange neue Frames, bis alle Samples gef�llt
	// sind
	while (rest) {
		debugprintf("Decode-Schleife: rest=%i, out_samples=%i, filled=%i\n",rest,out_samples,filled);

		int ret=0;
		while (ret==0) {
			// Decode-Buffer füllen
			debugprintf("Lese %i Bytes ab File-Position %llu\n",READBUFFER,ff->Ftell());
			size_t len = ff->Read(readbuffer,READBUFFER);
			debugprintf("%i Bytes gelesen\n",len);
			if (len==0) break;
			debugprintf("Rufe Dekoder auf\n");
			ret=hip_decode((hip_t)lame,readbuffer,len,out_left,out_right);
			debugprintf("%i Samples dekodiert\n",ret);
		}
		debugprintf("OK\n");


		break;
	}
	/*

		int res=hip_decode(hip,FillDecodeBuffer();
		debugprintf("FillDecodeBuffer hat %i samples geliefert\n",num);
		if (num<=0) break;
		if (num<=rest) {
			debugprintf("num<=rest, Kopiere %i Samples\n",num);
			memcpy(left,out_left,num*sizeof(SAMPLE));
			memcpy(right,out_right,num*sizeof(SAMPLE));
			out_offset=0;
			out_samples=0;
			left+=num;
			right+=num;
			filled+=num;
			rest-=num;
			debugprintf("rest=%i, out_samples=%i, filled=%i\n",rest,out_samples,filled);
		} else {
			debugprintf("num>rest, Kopiere %i Samples\n",rest);
			memcpy(left,out_left,rest*sizeof(SAMPLE));
			memcpy(right,out_right,rest*sizeof(SAMPLE));
			out_offset=rest;
			out_samples=num-rest;
			left+=rest;
			right+=rest;
			filled+=rest;
			rest=0;
			debugprintf("rest=%i, out_samples=%i, filled=%i\n",rest,out_samples,filled);
		}
	}
	debugprintf("Dekodier-Schleife beendet\n");
	if (filled<samples) {
		rest=samples-filled;
		debugprintf("Fülle Rest mit 0 auf: %i Samples\n",rest);
		bzero(left,rest*sizeof(SAMPLE));
		bzero(right,rest*sizeof(SAMPLE));
	}
	*/

	debugprintf("Beende CMP3DecodeLibLame::Decode, %i Samples angefordert, %i Samples geliefert\n",num,filled);
	return filled;
#else
	ppl6::SetError(555,"lame");
	return 0;
#endif
}


}	// EOF namespace ppl6

#endif

