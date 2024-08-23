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
    FileObject* ff=(FileObject*)datasource;
    if (whence == SEEK_SET) return static_cast<FileObject*>(datasource)->seek(offset, FileObject::SeekOrigin::SEEKSET);
    if (whence == SEEK_CUR) return static_cast<FileObject*>(datasource)->seek(offset, FileObject::SeekOrigin::SEEKCUR);
    if (whence == SEEK_END) return static_cast<FileObject*>(datasource)->seek(offset, FileObject::SeekOrigin::SEEKEND);
    return -1;
}

static int ppl_close_func(void* datasource)
{
    static_cast<FileObject*>(datasource)->close();
    return 0;
}

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
    ff=NULL;
    position=0;
    samplesize=0;
#endif
}


AudioDecoder_Ogg::~AudioDecoder_Ogg()
{
#ifdef HAVE_LIBVORBIS
    if (private_data) {
        OggVorbisPrivateData* oggp=static_cast<OggVorbisPrivateData*>(private_data);


        free(private_data);
    }
#endif
}

void AudioDecoder_Ogg::open(FileObject& file, const AudioInfo* info)
{
#ifdef HAVE_LIBVORBIS
    OggVorbisPrivateData* oggp=static_cast<OggVorbisPrivateData*>(private_data);
    file.seek(0);
    int res=ov_open_callbacks(&file, &oggp->vf, NULL, 0, oggp->callbacks);
    if (res != 0) throw UnsupportedAudioFormatException();


#endif

}

const AudioInfo& AudioDecoder_Ogg::getAudioInfo() const
{
    AudioInfo info;
#ifdef HAVE_LIBVORBIS
    getAudioInfo(info);
#endif
    return info;
}

void AudioDecoder_Ogg::getAudioInfo(AudioInfo& info) const
{
#ifdef HAVE_LIBVORBIS
#endif
}

void AudioDecoder_Ogg::seekSample(size_t sample)
{
#ifdef HAVE_LIBVORBIS
#endif

}

size_t AudioDecoder_Ogg::getPosition() const
{
#ifdef HAVE_LIBVORBIS
#else
    return 0;
#endif

}

size_t AudioDecoder_Ogg::getSamples(size_t num, STEREOSAMPLE16* buffer)
{
#ifdef HAVE_LIBVORBIS
#else
    return 0;
#endif
}

size_t AudioDecoder_Ogg::addSamples(size_t num, STEREOSAMPLE32* buffer)
{
#ifdef HAVE_LIBVORBIS
#else
    return 0;
#endif
}

size_t AudioDecoder_Ogg::getSamples(size_t num, float* left, float* right)
{
#ifdef HAVE_LIBVORBIS
#else
    return 0;
#endif
}

size_t AudioDecoder_Ogg::getSamples(size_t num, SAMPLE16* left, SAMPLE16* right)
{
#ifdef HAVE_LIBVORBIS
#else
    return 0;
#endif
}


}	// EOF namespace ppl7
