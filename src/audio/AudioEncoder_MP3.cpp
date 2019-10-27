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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prolog_ppl7.h"
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
	lame_set_write_id3tag_automatic((lame_global_flags*)gfp, 0);
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
		int preset=V2;
		switch (quality) {
			case 0: preset=V0; break;
			case 1: preset=V1; break;
			case 2: preset=V2; break;
			case 3: preset=V3; break;
			case 4: preset=V4; break;
			case 5: preset=V5; break;
			case 6: preset=V6; break;
			case 7: preset=V7; break;
			case 8: preset=V8; break;
			case 9: preset=V9; break;
			default: preset=V2; break;
		}
		//lame_set_preset((lame_global_flags*)gfp, preset);
		lame_set_bWriteVbrTag((lame_global_flags*)gfp,1);
		lame_set_VBR((lame_global_flags*)gfp, vbr_mtrh);
		lame_set_quality((lame_global_flags*)gfp,quality);
		lame_set_VBR_q((lame_global_flags*)gfp, quality);
		lame_set_VBR_quality((lame_global_flags*)gfp, (float)quality);
		lame_set_brate((lame_global_flags*)gfp, min);
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
		lame_set_bWriteVbrTag((lame_global_flags*)gfp,1);
		lame_set_preset((lame_global_flags*)gfp, avg);
		//lame_set_VBR((lame_global_flags*)gfp, vbr_abr);
		//lame_set_VBR_q((lame_global_flags*)gfp, quality);
		lame_set_quality((lame_global_flags*)gfp,quality);
		//lame_set_VBR_mean_bitrate_kbps((lame_global_flags*)gfp, avg);
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
		if (samples_got==0) break;
		samples_left-=samples_got;
		ppluint32 encodedbytes=lame_encode_buffer_interleaved((lame_global_flags*)gfp,(short*)readcache,samples_got,mp3buf,mp3bufsize);
		if (encodedbytes) {
			writeEncodedBytes((char *)mp3buf,encodedbytes);
		} else if (encodedbytes<0) dispatchEncoderError(encodedbytes);
		if (bStopEncode) throw EncoderAbortedException();
		if (ProgressFunc) {
			int progress=(int)((pplint64)(info.Samples-samples_left)*100/info.Samples);
			if (progress!=last_progress) {
				ProgressFunc(progress,ProgressFuncPrivData);
			}
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



} // end of namespace ppl



