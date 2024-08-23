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

#ifdef HAVE_LIBVORBIS
#include <vorbis/vorbisfile.h>
#endif

namespace ppl7 {

#ifdef HAVE_LIBVORBIS

typedef struct
{
    OggVorbis_File vf;
    ov_callbacks callbacks;

} OggVorbisPrivateData;


static size_t ppl_read_func(void* ptr, size_t size, size_t nmemb, void* datasource)
{
    return static_cast<FileObject*>(datasource)->read(ptr, size * nmemb);
}

static int ppl_seek_func(void* datasource, ogg_int64_t offset, int whence)
{
    if (whence == SEEK_SET) return static_cast<FileObject*>(datasource)->seek(offset, FileObject::SeekOrigin::SEEKSET);
    if (whence == SEEK_CUR) return static_cast<FileObject*>(datasource)->seek(offset, FileObject::SeekOrigin::SEEKCUR);
    if (whence == SEEK_END) return static_cast<FileObject*>(datasource)->seek(offset, FileObject::SeekOrigin::SEEKEND);
    return -1;
}

/*
static int ppl_close_func(void* datasource)
{
    static_cast<FileObject*>(datasource)->close();
    return 0;
}
*/

static long ppl_tell_func(void* datasource)
{
    return (long)static_cast<FileObject*>(datasource)->tell();
}

static void init_callback(ov_callbacks& callbacks)
{
    callbacks.close_func=NULL;
    callbacks.read_func=ppl_read_func;
    callbacks.seek_func=ppl_seek_func;
    callbacks.tell_func=ppl_tell_func;
}

static void GetVorbisAudioInfo(AudioInfo& info, OggVorbis_File& vf)
{
    info.Format=AudioInfo::AudioFormat::OGG;
    info.Channels=vf.vi->channels;
    info.Frequency=vf.vi->rate;
    info.IsVBR=true;
    info.Bitrate=vf.vi->bitrate_nominal / 1000;
    if (vf.vi->bitrate_lower == vf.vi->bitrate_upper) {
        info.IsVBR=false;
        if (vf.vi->bitrate_lower != 0) info.Bitrate=vf.vi->bitrate_lower;
    }
    //ppl7::PrintDebug("min: %d, max: %d, nominal: %d\n", vf.vi->bitrate_lower, vf.vi->bitrate_upper, vf.vi->bitrate_nominal);
    //info.Bitrate=ov_bitrate(&vf, -1);
    info.BitsPerSample=16;
    info.AudioStart=vf.offset;
    info.AudioEnd=vf.end;
    info.AudioSize=vf.end - vf.offset;
    info.BytesPerSample=4;
    info.Samples=ov_pcm_total(&vf, -1);
    info.Length=info.Samples * 1000 / info.Frequency;
}

#endif

bool IdentOggVorbisAudioFile(FileObject& file, AudioInfo& info)
{
#ifdef HAVE_LIBVORBIS
    ov_callbacks callbacks;
    OggVorbis_File vf;
    memset(&vf, 0, sizeof(OggVorbis_File));
    init_callback(callbacks);
    file.seek(0);
    int res=ov_open_callbacks(&file, &vf, NULL, 0, callbacks);
    //ppl7::PrintDebug("try read vobis file, result is: %d\n", res);
    if (res != 0) return false;
    GetVorbisAudioInfo(info, vf);
    ov_clear(&vf);
    return true;


#endif
    return false;
}



AudioDecoder_Ogg::AudioDecoder_Ogg()
{
#ifndef HAVE_LIBVORBIS
    throw UnsupportedFeatureException("AudioDecoder_Ogg");
#else

    private_data=calloc(sizeof(OggVorbisPrivateData), 1);
    if (!private_data) throw OutOfMemoryException();
    OggVorbisPrivateData* oggp=static_cast<OggVorbisPrivateData*>(private_data);
    init_callback(oggp->callbacks);
    position=0;
    readbuffer=NULL;
    buffersize=0;
#endif
}


AudioDecoder_Ogg::~AudioDecoder_Ogg()
{
#ifdef HAVE_LIBVORBIS
    if (private_data) {
        OggVorbisPrivateData* oggp=static_cast<OggVorbisPrivateData*>(private_data);
        ov_clear(&oggp->vf);
        free(private_data);
    }
    if (readbuffer) free(readbuffer);
#endif
}

void AudioDecoder_Ogg::allocateBuffer(size_t size)
{
    if (size != buffersize) {
        free(readbuffer);
        readbuffer=(char*)malloc(size);
        if (!readbuffer) throw OutOfMemoryException();
        buffersize=size;
    }
}

void AudioDecoder_Ogg::open(FileObject& file, const AudioInfo* info)
{
#ifdef HAVE_LIBVORBIS
    OggVorbisPrivateData* oggp=static_cast<OggVorbisPrivateData*>(private_data);
    file.seek(0);
    int res=ov_open_callbacks(&file, &oggp->vf, NULL, 0, oggp->callbacks);
    if (res != 0) throw UnsupportedAudioFormatException();
    getAudioInfo(this->info);

    position=0;


#endif

}

const AudioInfo& AudioDecoder_Ogg::getAudioInfo() const
{
    return info;
}

void AudioDecoder_Ogg::getAudioInfo(AudioInfo& info) const
{
#ifdef HAVE_LIBVORBIS
    OggVorbisPrivateData* oggp=static_cast<OggVorbisPrivateData*>(private_data);
    GetVorbisAudioInfo(info, oggp->vf);
#endif
}

void AudioDecoder_Ogg::seekSample(size_t sample)
{
#ifdef HAVE_LIBVORBIS
    OggVorbisPrivateData* oggp=static_cast<OggVorbisPrivateData*>(private_data);
    ov_pcm_seek(&oggp->vf, sample);
    position=sample;


#endif

}

size_t AudioDecoder_Ogg::getPosition() const
{
#ifdef HAVE_LIBVORBIS
    return position;
#else
    return 0;
#endif

}

size_t AudioDecoder_Ogg::getSamples(size_t num, STEREOSAMPLE16* buffer)
{
#ifdef HAVE_LIBVORBIS
    OggVorbisPrivateData* oggp=static_cast<OggVorbisPrivateData*>(private_data);
    //allocateBuffer(4 * num);
    size_t rest=num * 4;
    int bitstream=0;
    //ppl7::PrintDebug("reading %d samples, which is %d bytes\n", (int)num, (int)rest);
    char* b=(char*)buffer;
    while (rest > 0) {
        long bytes_read=ov_read(&oggp->vf, b, rest, 0, 2, 1, &bitstream);
        //ppl7::PrintDebug("bytes_read: %d, bitstream: %d, rest: %d\n", (int)bytes_read, bitstream, rest);
        if (bytes_read > 0) rest-=bytes_read;
        else if (bytes_read == 0) break;
        else throw DecoderException("AudioDecoder_Ogg::getSamples: %d", (int)bytes_read);
        b+=bytes_read;
    }

    return num - (rest / 4);
#else
    return 0;
#endif
}

size_t AudioDecoder_Ogg::addSamples(size_t num, STEREOSAMPLE32* buffer)
{
#ifdef HAVE_LIBVORBIS
    //OggVorbisPrivateData* oggp=static_cast<OggVorbisPrivateData*>(private_data);
    allocateBuffer(4 * num);
    size_t real_num=getSamples(num, (STEREOSAMPLE16*)readbuffer);
    for (size_t i=0;i < real_num;i++) {
        buffer[i].left+=((STEREOSAMPLE16*)readbuffer)[i].left;
        buffer[i].right+=((STEREOSAMPLE16*)readbuffer)[i].right;
    }


    return real_num;
#else
    return 0;
#endif
}

size_t AudioDecoder_Ogg::getSamples(size_t num, float* left, float* right)
{
#ifdef HAVE_LIBVORBIS
    throw UnsupportedFeatureException("AudioDecoder_Ogg::getSamples(size_t num, float* left, float* right)");
    return 0;
#else
    return 0;
#endif
}

size_t AudioDecoder_Ogg::getSamples(size_t num, SAMPLE16* left, SAMPLE16* right)
{
#ifdef HAVE_LIBVORBIS
    throw UnsupportedFeatureException("AudioDecoder_Ogg::getSamples(size_t num, SAMPLE16* left, SAMPLE16* right)");
    return 0;
#else
    return 0;
#endif
}


}	// EOF namespace ppl7
