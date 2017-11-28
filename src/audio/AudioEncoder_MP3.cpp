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
	out=NULL;
	ProgressFunc=NULL;
	ProgressFuncPrivData=NULL;
	readcache=NULL;
	mp3bufsize=0;
	mp3buf=NULL;
	have_firstaudio=false;
	started=false;
	bStopEncode=false;
	ProgressFunc=NULL;
	samples=ENCODESAMPLES;
#ifdef HAVE_LAME
	gfp=lame_init();			// LAME initialisieren
	if (!gfp) throw ppl7::InitializationFailedException("AudioEncoder_MP3: Lame");

	// LAME mit Standardwerten initialisieren
	lame_set_bWriteVbrTag((lame_global_flags*)gfp,0);
	/*
	lame_set_quality((lame_global_flags*)gfp,2);	// Beste Qualitaet und gute Performance
	lame_set_mode((lame_global_flags*)gfp,STEREO);
	lame_set_brate((lame_global_flags*)gfp,192);
	*/
	/*
	lame_set_lowpassfreq((lame_global_flags*)gfp,-1);	// Filter disablen
	lame_set_highpassfreq((lame_global_flags*)gfp,-1);	// Filter disablen
	*/
#endif
}

AudioEncoder_MP3::~AudioEncoder_MP3()
{
#ifdef HAVE_LAME
	if (gfp) lame_close ((lame_global_flags*)gfp);
#endif
	/*
	PushError();
	if (started) FinishEncode();
	if (mp3buf) free(mp3buf);
	if (readcache) free(readcache);
	if (gfp) lame_close ((lame_global_flags*)gfp);
	PopError();
	*/
}

void AudioEncoder_MP3::setCBR(int kbps, int quality)
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	if (!started) {
		lame_set_VBR((lame_global_flags*)gfp, vbr_off);
		lame_set_brate((lame_global_flags*)gfp,kbps);
		lame_set_quality((lame_global_flags*)gfp,quality);
	}
#endif
}

void AudioEncoder_MP3::setVBR(int min, int max, int quality)
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	if (!started) {
		int tmp;
		if(min>max) {
			tmp=min;
			min=max;
			max=tmp;
		}
		lame_set_VBR((lame_global_flags*)gfp, vbr_mtrh);
		lame_set_quality((lame_global_flags*)gfp,quality);
		lame_set_VBR_q((lame_global_flags*)gfp, quality);
		lame_set_VBR_quality((lame_global_flags*)gfp, (float)quality);
		lame_set_VBR_min_bitrate_kbps((lame_global_flags*)gfp, min);
		lame_set_VBR_max_bitrate_kbps((lame_global_flags*)gfp, max);
	}
#endif
}


void AudioEncoder_MP3::setABR(int avg, int quality)
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	if (!started) {
		lame_set_VBR((lame_global_flags*)gfp, vbr_abr);
		lame_set_VBR_q((lame_global_flags*)gfp, quality);
		lame_set_quality((lame_global_flags*)gfp,quality);
		lame_set_VBR_mean_bitrate_kbps((lame_global_flags*)gfp, avg);
	}
#endif
}


void AudioEncoder_MP3::setQuality(int quality)
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	if (!started) {
		lame_set_quality((lame_global_flags*)gfp,quality);
		lame_set_VBR_q((lame_global_flags*)gfp, quality);
	}
#endif
}


void AudioEncoder_MP3::setStereoMode(const AudioInfo::ChannelMode mode)
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	if (!started) {
		MPEG_mode m;
		switch (mode) {
			case AudioInfo::STEREO:
				m=STEREO;
				break;
			case AudioInfo::JOINT_STEREO:
				m=JOINT_STEREO;
				break;
			case AudioInfo::DUAL_CHANNEL:
				throw UnsupportedFeatureException("AudioEncoder_MP3::setStereoMode (AudioInfo::ChannelMode::DUAL_CHANNEL)");
			case AudioInfo::MONO:
				m=MONO;
				break;
			default:
				m=STEREO;
				break;
		}
		lame_set_mode((lame_global_flags*)gfp,m);
	}
#endif
}


/*!Lowpass Frequenz für Filter setzen
 * @param [in] freq Frequenz:
 * * -1 = keinen Filter verwenden
 * * 0 = Lame entscheidet
 * * >0 = Frequenz in Hz
 */
void AudioEncoder_MP3::setLowpassFreq(int freq)
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	if (!started) {
		lame_set_lowpassfreq((lame_global_flags*)gfp,freq);
	}
#endif
}

/*!Highpass Frequenz für Filter setzen
 * @param [in] freq Frequenz:
 * * -1 = keinen Filter verwenden
 * * 0 = Lame entscheidet
 * * >0 = Frequenz in Hz
 */
void AudioEncoder_MP3::setHighpassFreq(int freq)
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	if (!started) {
		lame_set_highpassfreq((lame_global_flags*)gfp,freq);
	}
#endif
}

void AudioEncoder_MP3::startEncode(FileObject &output)
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	if (started) throw EncoderAlreadyStartedException();
	out=&output;
	started=true;
	have_firstaudio=false;
	bStopEncode=false;
#endif
}

void AudioEncoder_MP3::startEncode(int frequency, int channels)
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	if (started) throw EncoderAlreadyStartedException();
	out=NULL;
	started=true;
	have_firstaudio=true;
	bStopEncode=false;
	firstAudio.frequency=frequency;
	firstAudio.channels=channels;
	lame_set_in_samplerate((lame_global_flags*)gfp,frequency);
	lame_set_num_channels((lame_global_flags*)gfp,channels);
	if (lame_init_params((lame_global_flags*)gfp)<0) {
		throw EncoderInitializationException();
	}
#endif
}

static inline void dispatchEncoderError(int code)
{
	if (code==-1) throw EncoderBufferTooSmallException();
	if (code==-2) throw OutOfMemoryException();
	if (code==-3) throw EncoderNotStartedException();
	if (code==-4) throw EncoderPsychoAcousticException();
	throw EncoderException("Unknown Error");
}

int AudioEncoder_MP3::encodeBuffer(STEREOSAMPLE16 *buffer, int num, void *mp3buf, size_t buffersize)
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	if (!started) throw EncoderNotStartedException();
	int ret=lame_encode_buffer_interleaved((lame_global_flags*)gfp,
		(short int *) buffer,
		num,
		(unsigned char*) mp3buf,
		(int)buffersize);
	if (ret<0) dispatchEncoderError(ret);
	return ret;
#endif
}

int AudioEncoder_MP3::encodeBuffer(SAMPLE16 *left, SAMPLE16 *right, int num, void *mp3buf, size_t buffersize)
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	if (!started) throw EncoderNotStartedException();
	int ret=lame_encode_buffer((lame_global_flags*)gfp,
		(const short int *) left,
		(const short int *) right,
		num,
		(unsigned char*) mp3buf,
		(int)buffersize);
	if (ret<0) dispatchEncoderError(ret);
	return ret;
#endif
}


int AudioEncoder_MP3::flushBuffer(void *mp3buf, size_t buffersize)
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	if (!started) return 0;
	int ret=lame_encode_flush((lame_global_flags*)gfp,
		(unsigned char*) mp3buf,
		(int)buffersize);
	return ret;
#endif
}

void AudioEncoder_MP3::encode(AudioDecoder &decoder)
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	if (!started) throw EncoderNotStartedException();
	if (bStopEncode) throw EncoderAbortedException();
	const AudioInfo &info=decoder.getAudioInfo();
	if (!have_firstaudio) {
		firstAudio.frequency=info.Frequency;
		firstAudio.channels=info.Channels;
		lame_set_in_samplerate((lame_global_flags*)gfp,firstAudio.frequency);
		lame_set_num_channels((lame_global_flags*)gfp,firstAudio.channels);
		if (lame_init_params((lame_global_flags*)gfp)<0) {
			throw EncoderInitializationException();
		}
		have_firstaudio=true;
	} else {
		if (info.Frequency!=firstAudio.frequency || info.Channels!=firstAudio.channels)
			throw EncoderAudioFormatMismatchException();
	}
	ppluint32 samples_left=info.Samples;
	if (!samples_left) return;

	mp3bufsize=(int)(1.25*samples+7200+100000);
	if (mp3buf) free(mp3buf);
	mp3buf=(unsigned char*)malloc(mp3bufsize);
	if (!mp3buf) throw OutOfMemoryException();
	if (readcache) free(readcache);
	size_t readcache_size=samples*info.BytesPerSample+100;
	readcache=(char *)malloc(readcache_size);
	if (!readcache) throw OutOfMemoryException();
	decoder.seekSample(0);
	int last_progress=0;
	while (samples_left>0) {
		ppluint32 current_samples=samples;
		if (current_samples>samples_left) current_samples=samples_left;
		size_t samples_got=decoder.getSamples(current_samples,(STEREOSAMPLE16*)readcache);
		samples_left-=samples_got;
		ppluint32 encodedbytes=lame_encode_buffer_interleaved((lame_global_flags*)gfp,(short*)readcache,samples_got,mp3buf,mp3bufsize);
		if (encodedbytes) {
			writeEncodedBytes((char *)mp3buf,encodedbytes);
		} else if (encodedbytes<0) dispatchEncoderError(encodedbytes);
		if (bStopEncode) throw EncoderAbortedException();
		if (ProgressFunc) {
			int progress=(info.Samples-samples_left)*100/info.Samples;
			if (progress!=last_progress) ProgressFunc(progress,ProgressFuncPrivData);
			last_progress=progress;
		}
	}
#endif
}

void AudioEncoder_MP3::finish()
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	if (!started) throw EncoderNotStartedException();
	if (have_firstaudio) {
		if (mp3buf) {
			ppluint32 encodedbytes=lame_encode_flush_nogap((lame_global_flags*)gfp,mp3buf,mp3bufsize);
			if (encodedbytes) writeEncodedBytes((const char *)mp3buf,encodedbytes);
		} else {
			mp3buf=(unsigned char*)malloc(16192);
			if (!mp3buf) throw OutOfMemoryException();
			ppluint32 encodedbytes=lame_encode_flush_nogap((lame_global_flags*)gfp,mp3buf,16192);
			if (encodedbytes) writeEncodedBytes((const char *)mp3buf,encodedbytes);
		}
	}
	free (mp3buf);
	mp3buf=NULL;
	free(readcache);
	readcache=NULL;
	have_firstaudio=false;
	started=false;
#endif
}

void AudioEncoder_MP3::stop()
{
	bStopEncode=true;
}

const char *AudioEncoder_MP3::getLameVersion()
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	return get_lame_version();
#endif
}

const char *AudioEncoder_MP3::getPSYVersion()
{
#ifndef HAVE_LAME
	throw ppl7::UnsupportedFeatureException("AudioEncoder_MP3: Lame");
#else
	return get_psy_version();
#endif
}

void AudioEncoder_MP3::writeEncodedBytes(const char *buffer, size_t bytes)
{
	if (out) {
		out->write(buffer,bytes);
	}
}

void AudioEncoder_MP3::setProgressFunction(void (*ProgressFunc) (int progress, void *priv), void *priv)
{
	this->ProgressFunc=ProgressFunc;
	this->ProgressFuncPrivData=priv;
}

void AudioEncoder_MP3::writeID3v2Tag(const ID3Tag &tag)
{
	if (!out) return;
	ByteArray ba;
	tag.generateId3V2Tag(ba);
	out->write(ba.ptr(),ba.size());
}

void AudioEncoder_MP3::writeID3v1Tag(const ID3Tag &tag)
{
	if (!out) return;
	ByteArray ba;
	tag.generateId3V1Tag(ba);
	out->write(ba.ptr(),ba.size());
}



#ifdef TODO

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






#endif



} // end of namespace ppl



