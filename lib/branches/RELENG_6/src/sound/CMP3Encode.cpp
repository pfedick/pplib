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

#include "ppl6.h"
#include "ppl6-sound.h"
#ifdef HAVE_LAME
#ifdef HAVE_LAME_LAME_H
#include <lame/lame.h>
#else
#include <lame.h>
#endif

#define ENCODESAMPLES	100000

namespace ppl6 {


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



/*!\class CMP3Encode
 * \ingroup PPLGroupSound
 */


CMP3Encode::CMP3Encode()
{
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

	// LAME mit Standardwerten initialisieren
	/*
	lame_set_quality((lame_global_flags*)gfp,2);	// Beste Qualitaet und gute Performance
	lame_set_mode((lame_global_flags*)gfp,STEREO);
	lame_set_brate((lame_global_flags*)gfp,192);
	*/
	lame_set_lowpassfreq((lame_global_flags*)gfp,-1);	// Filter disablen
	lame_set_highpassfreq((lame_global_flags*)gfp,-1);	// Filter disablen
}

CMP3Encode::~CMP3Encode()
{
	PushError();
	if (started) FinishEncode();
	if (mp3buf) free(mp3buf);
	if (readcache) free(readcache);
	if (gfp) lame_close ((lame_global_flags*)gfp);
	PopError();
}

int CMP3Encode::ReadWaveHeader(CFileObject *ff, PPL_WAVEHEADER *wav)
{
	const char *buffer=NULL;
	const char *fmt=NULL;
	bzero(wav,sizeof(PPL_WAVEHEADER));
	buffer=ff->Map(0,64);			// Die ersten 64 Bytes des Files laden
	if (!buffer) return 0;			// File zu klein


	if (strncmp(buffer,"RIFF",4)!=0) return 0;

	//ppldd riffchunklen=peekd(buffer+4);
	if (strncmp(buffer+8,"WAVE",4)!=0) return 0;
	if (strncmp(buffer+12,"fmt ",4)!=0) return 0;
	fmt=buffer+12;
	if (peekw(fmt+8)!=1) return 0;	// Keine PCM Daten

	ppldd fmtchunklen=peekd(fmt+4);
	wav->channels=(ppldb)peekw(fmt+0x0a);
	wav->frequency=peekd(fmt+0x0c);
	wav->bitdepth=(ppldb)peekw(fmt+0x16);

	buffer=ff->Map(fmtchunklen+20,8);	// Jetzt zum Datenchunk
	if (!buffer) return 0;				// File zu klein
	//ppl_dumphex(buffer,4);
	if (strncmp(buffer,"data",4)!=0) return 0;	// Kein Datenchunk
	wav->bytes=peekd(buffer+4);
	wav->datastart=fmtchunklen+28;

	if (wav->bitdepth==8) wav->bytespersample=1;
	if (wav->bitdepth==16) wav->bytespersample=2;
	wav->bytespersample=wav->bytespersample*wav->channels;
	if (!wav->bytespersample) return 0;						// Hier stimmt was nicht, weder 8 noch 16 Bit
	wav->numSamples=wav->bytes/wav->bytespersample;
	wav->bytespersecond=wav->frequency*wav->bytespersample;
	wav->seconds=wav->numSamples/wav->frequency;
	return 1;
}

void CMP3Encode::SetWatchThread(CThread *t)
{
	WatchThread=t;
}

void CMP3Encode::SetBitrate(int kbps)
{
	if ((!started) && gfp!=NULL) {
		lame_set_VBR((lame_global_flags*)gfp, vbr_off);
		lame_set_brate((lame_global_flags*)gfp,kbps);
	}
}

void CMP3Encode::SetCBR(int kbps, int quality)
{
	if ((!started) && gfp!=NULL) {
		lame_set_VBR((lame_global_flags*)gfp, vbr_off);
		lame_set_brate((lame_global_flags*)gfp,kbps);
		lame_set_quality((lame_global_flags*)gfp,quality);
	}
}


void CMP3Encode::SetVBR(int min, int max, int quality)
{
	if ((!started) && gfp!=NULL) {
		int tmp;
		if(min>max) {
			tmp=min;
			min=max;
			max=tmp;
		}
		lame_set_quality((lame_global_flags*)gfp,quality);
		lame_set_VBR((lame_global_flags*)gfp, vbr_mtrh);
		lame_set_VBR_q((lame_global_flags*)gfp, quality);
		lame_set_VBR_quality((lame_global_flags*)gfp, (float)quality);
		lame_set_VBR_min_bitrate_kbps((lame_global_flags*)gfp, min);
		lame_set_VBR_max_bitrate_kbps((lame_global_flags*)gfp, max);
	}
}

void CMP3Encode::SetABR(int avg, int quality)
{
	if ((!started) && gfp!=NULL) {
		lame_set_VBR((lame_global_flags*)gfp, vbr_abr);
		lame_set_VBR_q((lame_global_flags*)gfp, quality);
		lame_set_quality((lame_global_flags*)gfp,quality);
		lame_set_VBR_mean_bitrate_kbps((lame_global_flags*)gfp, avg);
	}
}

void CMP3Encode::SetQuality(int quality)
{
	if ((!started) && gfp!=NULL) {
		lame_set_quality((lame_global_flags*)gfp,quality);
		lame_set_VBR_q((lame_global_flags*)gfp, quality);
	}
}

void CMP3Encode::SetMode(int mode)
{
	if ((!started) && gfp!=NULL) {
		MPEG_mode m;
		switch (mode) {
			case MP3::MODE::STEREO:
				m=STEREO;
				break;
			case MP3::MODE::JOINT_STEREO:
				m=JOINT_STEREO;
				break;
			case MP3::MODE::DUAL_CHANNEL:
				m=DUAL_CHANNEL;
				break;
			case MP3::MODE::MONO:
				m=MONO;
				break;
			default:
				m=STEREO;
				break;
		}
		lame_set_mode((lame_global_flags*)gfp,m);
	}
}

void CMP3Encode::SetLowpassFreq(int freq)
{
	if ((!started) && gfp!=NULL) lame_set_lowpassfreq((lame_global_flags*)gfp,freq);
}

void CMP3Encode::SetHighpassFreq(int freq)
{
	if ((!started) && gfp!=NULL) lame_set_highpassfreq((lame_global_flags*)gfp,freq);
}

void CMP3Encode::SetEncoderDelay(int milliseconds)
{
	encoderdelay=milliseconds;
}

int CMP3Encode::HandleWatchThread()
{
	if (!WatchThread) return 1;
	WatchThread->ThreadWaitSuspended(100);
	if (WatchThread->ThreadShouldStop()) return 0;
	return 1;
}

int CMP3Encode::StartEncode(CFileObject *output)
{
	if (started) {
		SetError(217,"Encoder laeuft bereits");
		return 0;
	}
	if (!gfp) {
		SetError(217,"lame_init() fehlgeschlagen");
		return 0;
	}
	out=output;
	started=true;
	have_firstwave=false;
	progress.position=0;
	progress.percent=0.0f;
	progress.eta=0;
	progress.timestarted=getmicrotime();
	progress.wav=NULL;
	bStopEncode=false;
	return 1;
}

int CMP3Encode::StartEncode(int frequency, int channels)
{
	if (!StartEncode(NULL)) return 0;

	lame_set_in_samplerate((lame_global_flags*)gfp,frequency);
	lame_set_num_channels((lame_global_flags*)gfp,channels);
	if (lame_init_params((lame_global_flags*)gfp)<0) {
		SetError(217,"lame_init_params() fehlgeschlagen => falsche Parameter");
		return 0;
	}
	return 1;
}

int CMP3Encode::EncodeBuffer(SAMPLE *left, SAMPLE *right, int num, void *mp3buf, int buffersize)
{
	if (!started) {
		SetError(218);
		return 0;
	}
	int ret=lame_encode_buffer((lame_global_flags*)gfp,
		(const short int *) left,
		(const short int *) right,
		num,
		(unsigned char*) mp3buf,
		buffersize);
	return ret;
}

int CMP3Encode::EncodeBuffer(STEREOSAMPLE *buffer, int num, void *mp3buf, int buffersize)
{
	if (!started) {
		SetError(218);
		return 0;
	}
	int ret=lame_encode_buffer_interleaved((lame_global_flags*)gfp,
		(short int *) buffer,
		num,
		(unsigned char*) mp3buf,
		buffersize);
	return ret;
}

int CMP3Encode::FlushBuffer(void *mp3buf, int buffersize)
{
	if (!started) {
		SetError(218);
		return 0;
	}
	int ret=lame_encode_flush((lame_global_flags*)gfp,
		(unsigned char*) mp3buf,
		buffersize);
	return ret;
}

int CMP3Encode::EncodeFile(CFileObject *file)
{
	PPL_WAVEHEADER wav;
	if (!started) {
		SetError(218);
		return 0;
	}
	if (bStopEncode) return 0;
	if (!have_firstwave) {			// Erstes File
		if (!ReadWaveHeader(file, &firstwave)) {			// Unbekanntes Format
			SetError(20);
			return 0;
		}
		// Lame-Parameter initialisieren
		//lame_set_num_samples((lame_global_flags*)gfp,firstwave.numSamples);
		lame_set_in_samplerate((lame_global_flags*)gfp,firstwave.frequency);
		lame_set_num_channels((lame_global_flags*)gfp,firstwave.channels);
		if (lame_init_params((lame_global_flags*)gfp)<0) {
			SetError(217,"lame_init_params() fehlgeschlagen => falsche Parameter");
			return 0;
		}
		have_firstwave=true;
	}
	if (!ReadWaveHeader(file, &wav)) {			// Unbekanntes Format
		SetError(20);
		return 0;
	}
	if (wav.frequency!=firstwave.frequency || wav.channels!=firstwave.channels) {
		SetError(217,"WAVE-Format entspricht nicht dem Format des ersten Files");
		return 0;
	}

	//lame_set_num_samples((lame_global_flags*)gfp,wav.numSamples);

	lame_version_t lameversion;
	get_lame_version_numerical(&lameversion);

	ppldd encodedbytes=0;
	ppldd pp=0;
	ppldd bytes=0;
	ppldd rest=wav.bytes;
	//char *pcmbuffer=NULL;

	mp3bufsize=(int)(1.25*samples+7200+100000);
	if (mp3buf) free(mp3buf);
	mp3buf=(unsigned char*)malloc(mp3bufsize);
	if (!mp3buf) {
		SetError(2);
		return 0;
	}

	if (readcache) free(readcache);

	bytes=samples*wav.bytespersample;

	readcache=(char *)malloc(bytes+100);
	if (!readcache) {
		SetError(2);
		return 0;
	}


	progress.position_thisfile=0;
	progress.bytes_thisfile=wav.numSamples*wav.bytespersample;
	progress.wav=&wav;
	progress.mpg=NULL;


	file->Seek(wav.datastart);



	while (rest>0) {
		if(!HandleWatchThread()) return 0;
		bytes=samples*wav.bytespersample;
		if (bytes>rest) bytes=rest;

		file->Read(readcache,bytes);



		pp+=bytes;
		progress.position_thisfile+=bytes;
		progress.percent=(float)progress.position_thisfile*100.0f/(float)progress.bytes_thisfile;
		progress.eta=(float)progress.bytes_thisfile*(progress.now-progress.timestarted)/(float)progress.position_thisfile;
		progress.position+=bytes;
		progress.now=getmicrotime();

		if (ProgressFunc) ProgressFunc(&progress);
		rest-=bytes;
		encodedbytes=lame_encode_buffer_interleaved((lame_global_flags*)gfp,(short*)readcache,(bytes/wav.bytespersample),mp3buf,mp3bufsize);

		if (encodedbytes) {
			WriteEncodedBytes((char *)mp3buf,encodedbytes);
		}
		if (encoderdelay) MSleep(encoderdelay);
		if (bStopEncode) break;

	}
	progress.now=getmicrotime();
	progress.timeend=progress.now;
	progress.percent=100;
	progress.eta=0;

	if (ProgressFunc) ProgressFunc(&progress);

	return 1;
}

int CMP3Encode::EncodeFile(CMP3Decode *decode)
{
	PPL_MPEG_HEADER mpg;
	if (!started) {
		SetError(218);
		return 0;
	}
	if (bStopEncode) return 0;
	if (!have_firstwave) {			// Erstes File
		if (!decode->GetMpegHeader(&mpg)) {			// Unbekanntes Format
			SetError(20);
			return 0;
		}
		memset(&firstwave,0,sizeof(PPL_WAVEHEADER));
		firstwave.bitdepth=16;
		if (mpg.stereo==2) firstwave.channels=2;
		else firstwave.channels=1;
		firstwave.frequency=mpg.frequency;

		// Lame-Parameter initialisieren
		//lame_set_num_samples((lame_global_flags*)gfp,firstwave.numSamples);
		lame_set_in_samplerate((lame_global_flags*)gfp,firstwave.frequency);
		lame_set_num_channels((lame_global_flags*)gfp,firstwave.channels);
		if (lame_init_params((lame_global_flags*)gfp)<0) {
			SetError(217,"lame_init_params() fehlgeschlagen => falsche Parameter");
			return 0;
		}
		have_firstwave=true;
	}
	if (!decode->GetMpegHeader(&mpg)) {			// Unbekanntes Format
		SetError(20);
		return 0;
	}
	int channels=1;
	if (mpg.stereo==2) channels=2;
	PPL_WAVEHEADER wav;
	memset(&wav,0,sizeof(PPL_WAVEHEADER));
	wav.bitdepth=16;
	wav.channels=channels;
	wav.frequency=mpg.frequency;
	wav.seconds=mpg.length;
	wav.numSamples=mpg.samples;
	if (mpg.frequency!=(int)firstwave.frequency || channels!=firstwave.channels) {
		SetError(217,"MP3-Format entspricht nicht dem Format des ersten Files");
		return 0;
	}

	//lame_set_num_samples((lame_global_flags*)gfp,wav.numSamples);

	lame_version_t lameversion;
	get_lame_version_numerical(&lameversion);

	ppldd decoded_samples=0;
	ppldd encoded_bytes=0;
	//ppldd pp=0;
	//ppldd bytes=0;
	//char *pcmbuffer=NULL;

	mp3bufsize=(int)(1.25*samples+7200+100000);
	mp3bufsize=(int)(samples*sizeof(SAMPLE)*2);
	if (mp3buf) free(mp3buf);
	mp3buf=(unsigned char*)malloc(mp3bufsize);
	if (!mp3buf) {
		SetError(2);
		return 0;
	}

	SAMPLE *left=(SAMPLE*)malloc(sizeof(SAMPLE)*samples);
	if (!left) {
		SetError(2);
		return 0;
	}
	SAMPLE *right=(SAMPLE*)malloc(sizeof(SAMPLE)*samples);
	if (!right) {
		free(left);
		SetError(2);
		return 0;
	}


	progress.position_thisfile=0;
	progress.bytes_thisfile=mpg.samples;
	progress.wav=&wav;
	progress.mpg=&mpg;

	if (!decode->Start()) {
		free(left);
		free(right);
		return 0;
	}
	while (1) {
		if(!HandleWatchThread()) return 0;
		decoded_samples=decode->Decode(samples,left,right);
		if (!decoded_samples) return 0;
		progress.position_thisfile+=decoded_samples;
		progress.position+=decoded_samples;
		progress.now=getmicrotime();
		progress.percent=(float)progress.position_thisfile*100.0f/(float)progress.bytes_thisfile;
		progress.past=progress.now-progress.timestarted;
		progress.eta=((float)progress.bytes_thisfile*progress.past/(float)progress.position_thisfile)-progress.past;
		progress.faktor=(float)progress.wav->seconds/((float)progress.past+(float)progress.eta);


		if (ProgressFunc) ProgressFunc(&progress);
		encoded_bytes=lame_encode_buffer((lame_global_flags*)gfp,
			(const short int *) left,
			(const short int *) right,
			decoded_samples,
			(unsigned char*) mp3buf,
			mp3bufsize);

		if (encoded_bytes) {
			WriteEncodedBytes((char *)mp3buf,encoded_bytes);
		}
		if (encoderdelay) MSleep(encoderdelay);
		if (bStopEncode) break;
		if (decoded_samples<samples) break;
	}
	progress.now=getmicrotime();
	progress.timeend=progress.now;

	if (ProgressFunc) ProgressFunc(&progress);
	free(left);
	free(right);
	decode->Stop();
	return 1;
}

int CMP3Encode::FinishEncode()
{
	if (!started) {
		SetError(218);
		return 0;
	}

	ppldd encodedbytes=0;
	if (mp3buf) {
		if (!have_firstwave) {
			SetError(217,"Nothing to encode");
			return 0;
		}

		encodedbytes=lame_encode_flush((lame_global_flags*)gfp,mp3buf,mp3bufsize);
		if (encodedbytes) WriteEncodedBytes((char *)mp3buf,encodedbytes);
	} else {
		mp3buf=(unsigned char*)malloc(16192);
		encodedbytes=lame_encode_flush((lame_global_flags*)gfp,mp3buf,16192);
	}
	free (mp3buf);
	mp3buf=NULL;
	free(readcache);
	readcache=NULL;

	have_firstwave=false;
	started=false;
	return 1;
}

void CMP3Encode::WriteEncodedBytes(char *buffer, ppldd bytes)
{
	if (out) {
		out->Write(buffer,bytes);
	}
}

void CMP3Encode::SetProgressFunction(void (*Progress) (PPL_SOUNDPROGRESS *prog), void *data)
{
	ProgressFunc=Progress;
	progress.data=data;
}


void CMP3Encode::StopEncode()
{
	bStopEncode=true;
}

const char *CMP3Encode::GetLameVersion()
{
	return get_lame_version();
}

const char *CMP3Encode::GetPSYVersion()
{
	return get_psy_version();
}

} // end of namespace ppl

#endif // _HAVE_LAME
