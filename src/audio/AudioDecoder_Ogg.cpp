/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <string.h>
#include <config_pplib.h>
#include <pplib/core/fileobject.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>
#include <pplib/core/mutex.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/decoder.h>

#ifdef HAVE_LIBVORBIS
#include <vorbis/vorbisfile.h>
#endif

namespace pplib
{

#ifdef HAVE_LIBVORBIS

typedef struct
{
    OggVorbis_File vf;
    ov_callbacks callbacks;

} OggVorbisPrivateData;

/** @brief Lese-Callback fuer libvorbis
 *
 * libvorbis erwartet die Zahl gelesener ELEMENTE und behandelt ein Kurzlesen als
 * normales Ende; eine Exception durch die C-Frames von libvorbis hindurch waere
 * nicht portabel definiert (Windows/MSVC: kein garantiertes Unwinding). Deshalb
 * wird Dateiende/Lesefehler als Kurzlesen (0) gemeldet.
 */
static size_t ppl_read_func(void* ptr, size_t size, size_t nmemb, void* datasource)
{
    if (size == 0 || nmemb == 0) return 0;
    FileObject* f = static_cast<FileObject*>(datasource);
    try {
        return f->read(ptr, size * nmemb) / size;
    } catch (...) {
        return 0; // EOF oder Lesefehler: Signal an libvorbis, keine Exception
    }
}

/** @brief Seek-Callback fuer libvorbis
 *
 * libvorbis verlangt 0 bei Erfolg und ungleich 0 als Fehler; FileObject::seek()
 * liefert dagegen die neue Position, die bei jedem Sprung auf ein anderes Ziel
 * als 0 wie ein Fehlschlag wirken wuerde.
 */
static int ppl_seek_func(void* datasource, ogg_int64_t offset, int whence)
{
    FileObject* f = static_cast<FileObject*>(datasource);
    FileObject::SeekOrigin origin;
    if (whence == SEEK_SET) origin = FileObject::SeekOrigin::SEEKSET;
    else if (whence == SEEK_CUR) origin = FileObject::SeekOrigin::SEEKCUR;
    else if (whence == SEEK_END) origin = FileObject::SeekOrigin::SEEKEND;
    else return -1;

    const int64_t base = (origin == FileObject::SeekOrigin::SEEKCUR) ? (int64_t)f->tell()
                     : (origin == FileObject::SeekOrigin::SEEKEND) ? (int64_t)f->size() : 0;
    const int64_t target = base + (int64_t)offset;
    if (target < 0) return -1; // vor Dateianfang
    try {
        return (f->seek(offset, origin) == (uint64_t)target) ? 0 : -1;
    } catch (...) {
        return -1;
    }
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
    callbacks.close_func = NULL;
    callbacks.read_func = ppl_read_func;
    callbacks.seek_func = ppl_seek_func;
    callbacks.tell_func = ppl_tell_func;
}

/** @brief RAII-Huelle fuer OggVorbis_File
 *
 * ov_open_callbacks() laesst bei Misserfolg allokierte Strukturen zurueck, die
 * erst ov_clear() freigibt. Ein frueher Rueckgabe- oder Throw-Pfad wuerde sie
 * sonst leakieren (bei wiederholtem Scannen beschaedigter Dateien ein Dauerleck).
 */
class OggVorbisHandle
{
public:
    OggVorbisHandle()
    {
        memset(&vf, 0, sizeof(OggVorbis_File));
    }
    OggVorbisHandle(const OggVorbisHandle&) = delete;
    OggVorbisHandle& operator=(const OggVorbisHandle&) = delete;
    ~OggVorbisHandle()
    {
        ov_clear(&vf);
    }
    OggVorbis_File* get() { return &vf; }

private:
    OggVorbis_File vf;
};

static void GetVorbisAudioInfo(AudioInfo& info, OggVorbis_File& vf, FileObject& file)
{
    info.Format = AudioInfo::AudioFormat::OGG;
    info.FileSize = file.size();
    info.Channels = vf.vi->channels;
    info.Frequency = vf.vi->rate;
    info.IsVBR = true;
    // libvorbis liefert Bit/s; AudioInfo::Bitrate ist kbit/s. Nominale, untere und
    // obere Bitrate duerfen nicht in unterschiedlichen Einheiten verglichen werden.
    double bits = vf.vi->bitrate_nominal;
    if (vf.vi->bitrate_lower == vf.vi->bitrate_upper) {
        info.IsVBR = false;
        if (vf.vi->bitrate_lower > 0.0) bits = vf.vi->bitrate_lower;
    }
    if (bits < 0.0) bits = 0.0;
    const uint64_t kbit = (uint64_t)bits / 1000;
    info.Bitrate = (kbit > UINT16_MAX) ? UINT16_MAX : (uint16_t)kbit;
    // pplib::PrintDebug("min: %d, max: %d, nominal: %d\n", vf.vi->bitrate_lower, vf.vi->bitrate_upper, vf.vi->bitrate_nominal);
    // info.Bitrate=ov_bitrate(&vf, -1);
    info.BitsPerSample = 16;
    info.AudioStart = vf.offset;

    // AudioEnd ist - wie bei WAV, MP3 und AIFF - die Position des letzten Datenbytes.
    if (vf.end > vf.offset) {
        info.AudioSize = vf.end - vf.offset;
        info.AudioEnd = info.AudioStart + info.AudioSize - 1;
    }

    info.BytesPerSample = info.BitsPerSample * info.Channels / 8;
    info.Samples = ov_pcm_total(&vf, -1);
    if (info.Frequency > 0)
        info.Length = (uint32_t)((uint64_t)info.Samples * 1000 / info.Frequency);
}

#endif

bool IdentOggVorbisAudioFile(FileObject& file, AudioInfo& info)
{
#ifdef HAVE_LIBVORBIS
    OggVorbisHandle handle;
    OggVorbis_File* vf = handle.get();
    ov_callbacks callbacks;
    init_callback(callbacks);
    file.seek(0);
    int res = ov_open_callbacks(&file, vf, NULL, 0, callbacks);
    // pplib::PrintDebug("try read vobis file, result is: %d\n", res);
    if (res != 0) return false; // Handle-Destruktor uebernimmt ov_clear()
    GetVorbisAudioInfo(info, *vf, file);
    return true;

#else
    return false;
#endif
}

AudioDecoder_Ogg::AudioDecoder_Ogg()
{
#ifndef HAVE_LIBVORBIS
    throw UnsupportedFeatureException("AudioDecoder_Ogg");
#else

    private_data = calloc(sizeof(OggVorbisPrivateData), 1);
    if (!private_data) throw OutOfMemoryException();
    OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
    init_callback(oggp->callbacks);
    position = 0;
    readbuffer = NULL;
    buffersize = 0;
    decodebuffer_size = 0;
    decodebuffer = NULL;
#endif
}

AudioDecoder_Ogg::~AudioDecoder_Ogg()
{
    close();
}

void AudioDecoder_Ogg::allocateBuffer(size_t size)
{
    if (size != buffersize) {
        free(readbuffer);
        readbuffer = (char*)malloc(size);
        if (!readbuffer) throw OutOfMemoryException();
        buffersize = size;
    }
}

void AudioDecoder_Ogg::open(FileObject& file, const AudioInfo* info)
{
#ifdef HAVE_LIBVORBIS
    OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
    file.seek(0);
    int res = ov_open_callbacks(&file, &oggp->vf, NULL, 0, oggp->callbacks);
    if (res != 0) throw UnsupportedAudioFormatException();
    if (!info) {
        GetVorbisAudioInfo(this->info, oggp->vf, file);
    } else {
        this->info = *info;
    }

    if (this->info.Channels < 1 || this->info.Channels > 2) throw UnsupportedAudioFormatException("channels <1 or >2");
    if (this->info.Channels == 1 && decodebuffer == NULL) {
        decodebuffer_size = 4096;
        decodebuffer = (char*)malloc(decodebuffer_size * 4);
        if (!decodebuffer) throw OutOfMemoryException();
    }

    position = 0;
#endif
}

void AudioDecoder_Ogg::close()
{
#ifdef HAVE_LIBVORBIS
    if (decodebuffer) {
        free(decodebuffer);
        decodebuffer = NULL;
        decodebuffer_size = 0;
    }
    if (private_data) {
        OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
        ov_clear(&oggp->vf);
        free(private_data);
        private_data = NULL;
    }
    if (readbuffer) {
        free(readbuffer);
        readbuffer = NULL;
        buffersize = 0;
    }
#endif
    position = 0;
}

const AudioInfo& AudioDecoder_Ogg::getAudioInfo() const
{
    return info;
}

void AudioDecoder_Ogg::seekSample(size_t sample)
{
#ifdef HAVE_LIBVORBIS
    OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
    ov_pcm_seek(&oggp->vf, sample);
    position = sample;

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
    OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
    int rest = num * info.BytesPerSample;
    int bitstream = 0;
    char* b = (char*)buffer;
    if (info.Channels == 2) {
        while (rest > 0) {
            long bytes_read = ov_read(&oggp->vf, b, rest, 0, 2, 1, &bitstream);
            if (bytes_read > 0)
                rest -= bytes_read;
            else if (bytes_read == 0)
                break;
            else
                throw DecoderException("AudioDecoder_Ogg::getSamples: %d", (int)bytes_read);
            b += bytes_read;
        }
        return num - (rest / 4);
    } else if (info.Channels == 1) {
        int size = decodebuffer_size;
        pplib::SAMPLE16* sample = (pplib::SAMPLE16*)decodebuffer;
        while (rest > 0) {
            if (rest < size) size = rest;
            long bytes_read = ov_read(&oggp->vf, decodebuffer, size, 0, 2, 1, &bitstream);
            if (bytes_read > 0) {
                rest -= bytes_read;
                for (int i = 0; i < bytes_read / 2; i++) {
                    buffer[i].left = sample[i];
                    buffer[i].right = sample[i];
                }
            } else if (bytes_read == 0)
                break;
            else
                throw DecoderException("AudioDecoder_Ogg::getSamples: %d", (int)bytes_read);
            buffer += (bytes_read / 2);
        }
        return num - (rest / 2);
    } else
        throw UnsupportedAudioFormatException("channels <1 or >2");
#else
    return 0;
#endif
}

size_t AudioDecoder_Ogg::addSamples(size_t num, STEREOSAMPLE32* buffer)
{
#ifdef HAVE_LIBVORBIS
    // OggVorbisPrivateData* oggp=static_cast<OggVorbisPrivateData*>(private_data);
    allocateBuffer(4 * num);
    size_t real_num = getSamples(num, (STEREOSAMPLE16*)readbuffer);
    for (size_t i = 0; i < real_num; i++) {
        buffer[i].left += ((STEREOSAMPLE16*)readbuffer)[i].left;
        buffer[i].right += ((STEREOSAMPLE16*)readbuffer)[i].right;
    }

    return real_num;
#else
    return 0;
#endif
}

size_t AudioDecoder_Ogg::getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer)
{
#ifdef HAVE_LIBVORBIS
    OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
    long total = 0;
    int bitstream = 0;
    while (total < (long)num) {
        float** pcm = nullptr;
        long got = ov_read_float(&oggp->vf, &pcm, (int)(num - total), &bitstream);
        if (got > 0) {
            if (info.Channels == 2) {
                for (long i = 0; i < got; i++) {
                    buffer[total + i].left = pcm[0][i];
                    buffer[total + i].right = pcm[1][i];
                }
            } else if (info.Channels == 1) {
                for (long i = 0; i < got; i++) {
                    buffer[total + i].left = pcm[0][i];
                    buffer[total + i].right = pcm[0][i];
                }
            } else
                throw UnsupportedAudioFormatException("channels <1 or >2");
            total += got;
        } else if (got == 0) {
            break; // EOF
        } else {
            throw DecoderException("AudioDecoder_Ogg::getSamples: %d", (int)got);
        }
    }
    position += total;
    return (size_t)total;
#else
    return 0;
#endif
}

size_t AudioDecoder_Ogg::addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer)
{
#ifdef HAVE_LIBVORBIS
    OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
    long total = 0;
    int bitstream = 0;
    while (total < (long)num) {
        float** pcm = nullptr;
        long got = ov_read_float(&oggp->vf, &pcm, (int)(num - total), &bitstream);
        if (got > 0) {
            if (info.Channels == 2) {
                for (long i = 0; i < got; i++) {
                    buffer[total + i].left += pcm[0][i];
                    buffer[total + i].right += pcm[1][i];
                }
            } else if (info.Channels == 1) {
                for (long i = 0; i < got; i++) {
                    buffer[total + i].left += pcm[0][i];
                    buffer[total + i].right += pcm[0][i];
                }
            } else
                throw UnsupportedAudioFormatException("channels <1 or >2");
            total += got;
        } else if (got == 0) {
            break; // EOF
        } else {
            throw DecoderException("AudioDecoder_Ogg::getSamples: %d", (int)got);
        }
    }
    position += total;
    return (size_t)total;
#else
    return 0;
#endif
}

} // namespace pplib
