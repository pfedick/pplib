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
	memset(&wave,0,sizeof(WAVEHEADER));
}

AudioDecoder_Wave::~AudioDecoder_Wave()
{
	close();
}

void AudioDecoder_Wave::close()
{
	ff=NULL;
	position=0;
	samplesize=0;
	memset(&wave,0,sizeof(WAVEHEADER));
}

bool AudioDecoder_Wave::ident(const String &filename)
{
	File ff;
	ff.open(filename);
	return ident(ff);
}

bool AudioDecoder_Wave::ident(FileObject &file)
{
	try {
		if (file.size()<64) return false;
		const char *buffer=file.map(0,64);					// Die ersten 64 Bytes des Files laden
		if (!buffer) return false;
		if (::strncmp(buffer,"RIFF",4)!=0) return false;
		if (::strncmp(buffer+8,"WAVE",4)!=0) return false;
		if (::strncmp(buffer+12,"fmt ",4)!=0) return false;
		const char *fmt=buffer+12;
		if (Peek16(fmt+8)!=1) return false;					// Keine PCM Daten
		int bitdepth=Peek16(fmt+0x16);
		if (bitdepth!=8 && bitdepth!=16) return false;
		int channels=Peek16(fmt+0x0a);
		if (channels!=1 && channels!=2) return false;
		ppluint32 fmtchunklen=Peek32(fmt+4);
		buffer=file.map(fmtchunklen+20,8);					// Jetzt zum Datenchunk
		if (!buffer) return false;							// File zu klein
		if (::strncmp(buffer,"data",4)!=0) return false;	// Kein Datenchunk
	} catch (...) {
		return false;
	}
	return true;
}

void AudioDecoder_Wave::readWaveHeader(FileObject &file, WAVEHEADER &header)
{
	const char *buffer=NULL;
	const char *fmt=NULL;
	buffer=file.map(0,64);			// Die ersten 64 Bytes des Files laden
	if (!buffer) throw InvalidFormatException();
	fmt=buffer+12;
	ppluint32 fmtchunklen=Peek32(fmt+4);
	header.channels=(ppluint8)Peek16(fmt+0x0a);
	header.frequency=Peek32(fmt+0x0c);
	header.bitdepth=(ppluint8)Peek16(fmt+0x16);

	buffer=file.map(fmtchunklen+20,8);	// Jetzt zum Datenchunk
	if (!buffer) throw InvalidFormatException();
	if (strncmp(buffer,"data",4)!=0) throw InvalidFormatException();
	header.bytes=Peek32(buffer+4);
	header.datastart=fmtchunklen+28;
	samplesize=header.bitdepth/8;
	header.bytespersample=header.channels*samplesize;
	header.numSamples=header.bytes/header.bytespersample;
	header.bytespersecond=header.frequency*header.bytespersample;
	header.seconds=header.numSamples/header.frequency;

}

void AudioDecoder_Wave::open(const String &filename)
{
	if (ff) close();
	myFile.open(filename);
	open(myFile);
}

void AudioDecoder_Wave::open(FileObject &file)
{
	if (ff) close();
	if (!ident(file)) throw InvalidFormatException();
	ff=&file;
	readWaveHeader(file,wave);
	file.seek(wave.datastart);
	const char *buffer=file.map(wave.datastart,64);
	ppl7::HexDump(buffer,64);
}

void AudioDecoder_Wave::getWaveHeader(WAVEHEADER &header) const
{
	memcpy(&header,&wave,sizeof(WAVEHEADER));
}

size_t AudioDecoder_Wave::bitdepth() const
{
	return wave.bitdepth;
}

size_t AudioDecoder_Wave::bytesPerSample() const
{
	return samplesize;
}

void AudioDecoder_Wave::seekSample(size_t sample)
{
	if (sample>=wave.numSamples) throw OutOfBoundsEception();
	position=sample;
	ff->seek(position*wave.bytespersample+wave.datastart);
}

size_t AudioDecoder_Wave::getSamples(size_t num, void *interleafed)
{
	if (position+num>=wave.numSamples) num=wave.numSamples-position;
	size_t read=ff->read(interleafed,num*wave.bytespersample)/wave.bytespersample;
	position+=read;
	return read;
}

size_t AudioDecoder_Wave::getSamples(size_t num, void *left, void *right)
{
	if (position+num>=wave.numSamples) num=wave.numSamples-position;
	void *buffer=malloc(num*wave.bytespersample);
	if (!buffer) throw OutOfMemoryException();
	size_t read=ff->read(buffer,num*wave.bytespersample)/wave.bytespersample;
	for (size_t i=0;i<read;i++) {
		memcpy((char*)left+i*samplesize,(char*)buffer+i*wave.bytespersample,samplesize);
		memcpy((char*)right+i*samplesize,(char*)buffer+i*wave.bytespersample+samplesize,samplesize);
	}

	free(buffer);
	return read;
}

size_t AudioDecoder_Wave::getSamples(size_t num, float *left, float *right)
{
	if (position+num>=wave.numSamples) num=wave.numSamples-position;
	void *buffer=malloc(num*wave.bytespersample);
	if (!buffer) throw OutOfMemoryException();
	size_t read=ff->read(buffer,num*wave.bytespersample)/wave.bytespersample;
	if (wave.bitdepth==16) {
		pplint16 *buffer16=(pplint16*)buffer;
		for (size_t i=0;i<read;i++) {
			left[i]=(float)buffer16[i*2]/32768.0;
			right[i]=(float)buffer16[i*2+1]/32768.0;
		}
	} else {
		free(buffer);
		throw UnsupportedDataTypeException();
	}
	free(buffer);
	return read;
}

}	// EOF namespace ppl7

