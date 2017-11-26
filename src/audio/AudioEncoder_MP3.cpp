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

#include "prolog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <strings.h>
#endif
#include <time.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN		// Keine MFCs
#include <windows.h>
#endif

#include "ppl7.h"
#include "ppl7-audio.h"
#ifdef HAVE_LAME
	#ifdef HAVE_LAME_LAME_H
		#include <lame/lame.h>
	#else
		#include <lame.h>
	#endif
#endif

namespace ppl7 {

#define ENCODESAMPLES	100000

/*!\class AudioEncoder_MP3
 * \ingroup PPLGroupSound
 */


AudioEncoder_MP3::AudioEncoder_MP3()
{
	/*
	encoderdelay=0;
	readcache=NULL;
	mp3buf=NULL;
	have_firstwave=false;
	started=false;
	bStopEncode=false;
	ProgressFunc=NULL;
	WatchThread=NULL;
	samples=ENCODESAMPLES;
	gfp=lame_init();			// LAME initialisieren
	if (!gfp) {
		SetError(217,"lame_init() fehlgeschlagen");
		return;
	}
	*/

	// LAME mit Standardwerten initialisieren
	/*
	lame_set_quality((lame_global_flags*)gfp,2);	// Beste Qualitaet und gute Performance
	lame_set_mode((lame_global_flags*)gfp,STEREO);
	lame_set_brate((lame_global_flags*)gfp,192);
	*/
	/*
	lame_set_lowpassfreq((lame_global_flags*)gfp,-1);	// Filter disablen
	lame_set_highpassfreq((lame_global_flags*)gfp,-1);	// Filter disablen
	*/
}

AudioEncoder_MP3::~AudioEncoder_MP3()
{
	/*
	PushError();
	if (started) FinishEncode();
	if (mp3buf) free(mp3buf);
	if (readcache) free(readcache);
	if (gfp) lame_close ((lame_global_flags*)gfp);
	PopError();
	*/
}


} // end of namespace ppl

