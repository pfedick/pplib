/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.5 $
 * $Date: 2010/05/01 19:06:42 $
 * $Id: CMP3Decode.cpp,v 1.5 2010/05/01 19:06:42 pafe Exp $
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


namespace ppl6 {


CMP3Decode::CMP3Decode()
{
	decoder=NULL;
}

CMP3Decode::~CMP3Decode()
{
	delete decoder;
}

int CMP3Decode::Init(const char *engine)
{
	delete decoder;
	if (!engine) {
#ifdef HAVE_LIBMAD
		decoder=new CMP3DecodeLibMad;
		return 1;
#elif defined HAVE_MPG123
		decoder=new CMP3DecodeLibMpg123;
		return 1;
/*
#elif defined HAVE_LAME_HIP_DECODE
		decoder=new CMP3DecodeLibLame;
		return 1;
		*/
#else
		SetError(554);
		return 0;
#endif
	}
	ppl6::CString d=engine;
	d.LCase();
	d.Trim();
	/*
	if (d=="lame") {
#ifdef HAVE_LAME_HIP_DECODE
		decoder=new CMP3DecodeLibLame;
		return 1;
#else
		SetError(555,"%s",(const char*)d);
		return 0;
#endif
	}
	*/
	if (d=="mad") {
#ifdef HAVE_LIBMAD
		decoder=new CMP3DecodeLibMad;
		return 1;
#else
		SetError(555,"%s",(const char*)d);
		return 0;
#endif
	}
	if (d=="mpg123") {
#ifdef HAVE_MPG123
		decoder=new CMP3DecodeLibMpg123;
		return 1;
#else
		SetError(555,"%s",(const char*)d);
		return 0;
#endif
	}
	SetError(556,"%s",(const char*)d);
	return 0;
}

void CMP3Decode::Clear()
{
	if (decoder==NULL && Init()==0) return;
	decoder->Clear();
}

int CMP3Decode::LoadFile(const CString &filename)
{
	if (decoder==NULL && Init()==0) return 0;
	return decoder->LoadFile(filename);
}

int CMP3Decode::LoadFile(CFileObject &file)
{
	if (decoder==NULL && Init()==0) return 0;
	return decoder->LoadFile(file);
}

int CMP3Decode::GetMpegHeader(PPL_MPEG_HEADER *mpg)
{
	if (decoder==NULL && Init()==0) return 0;
	return decoder->GetMpegHeader(mpg);
}

int CMP3Decode::Start()
{
	if (decoder==NULL && Init()==0) return 0;
	return decoder->Start();
}

int CMP3Decode::Stop()
{
	if (decoder==NULL && Init()==0) return 0;
	return decoder->Stop();
}

int CMP3Decode::Decode(int num, SAMPLE *left, SAMPLE *right)
{
	if (decoder==NULL && Init()==0) return 0;
	return decoder->Decode(num,left,right);
}


}	// EOF namespace ppl6

