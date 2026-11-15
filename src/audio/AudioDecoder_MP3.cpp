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

#ifdef HAVE_MPG123
#include <mpg123.h>
#endif

namespace pplib
{

#ifdef HAVE_MPG123
static Mutex MPG123_GlobalMutex;
static uint64_t mpg123_instances = 0;
static bool mpg123_initialized = false;

/// Puffergroessen: auf dem Desktop grosshalten, auf Mikrocontrollern deutlich kleiner.
/// Der Ausgabepuffer faesst einen MP3 Rahmen mit maximal 1152 Samples je Kanal.
#ifdef PICO_BUILD
constexpr size_t OUTBUFF = 4 * 1024;
#else
constexpr size_t OUTBUFF = 32 * 1024;
#endif

/// normierter Float-Wert ([-1,1]) entspricht der 16 Bit Vollskala.
/// Nur noetig, wenn im Puffer noch 16 Bit Samples liegen und ein Float-Ziel gefuellt wird.
constexpr float FULL_SCALE_16 = 32768.0f;

struct Mp3Data
{
    mpg123_handle* handle = nullptr;
    FileObject* file = nullptr;
    int64_t audiostreamstart = 0;           ///< erstes Byte des Audiodatenblocks in der Datei
    int64_t audiolength = 0;                ///< Groesse des Audiodatenblocks in Bytes
    int64_t pos = 0;                        ///<Position innerhalb des Audiodatenblocks
    unsigned char* outbuffer = nullptr;     ///< dekodierte Samples in der Kodierung von bufencoding
    size_t outcapacity = 0;                 ///< Kapazitaet von outbuffer in Bytes
    size_t outframes = 0;                   ///< gueltige Frames im outbuffer
    size_t outpos = 0;                      ///< naechstes abzweigendes Frame
    size_t channels = 0;                    ///< Kanaele des Decoderausgangs
    int encoding = MPG123_ENC_SIGNED_16;    ///< vom Zielpuffer geforderte Kodierung
    int bufencoding = MPG123_ENC_SIGNED_16; ///< Kodierung des Inhalts von outbuffer
    bool eof = false;                       ///< kein weiteres Frame mehr zu holen
};

static size_t SampleWidth(int encoding)
{
    return (encoding == MPG123_ENC_FLOAT_32) ? sizeof(float) : sizeof(int16_t);
}

static int16_t ClampToInt16(float value)
{
    if (value >= 1.0f) return 32767;
    if (value <= -1.0f) return -32768;
    return (int16_t)(value * FULL_SCALE_16);
}

static std::string DecoderError(mpg123_handle* handle)
{
    return std::string(mpg123_strerror(handle));
}

/** @brief Rückruffunktion von libmpg123: Datenblock über FileObject lesen
 *
 * libmpg123 sieht einen eigenen Datenstrom, dessen Position 0 der erste Byte des
 * Audiodatenblocks ist. Anhaengsel wie ein ID3v1-Tag oder Auffuellbytes nach dem letzten
 * Rahmen gehoeren nicht zu diesem Strom und gelangen nicht in den Decoder.
 */
static mpg123_ssize_t ReadStream(void* clientdata, void* buffer, size_t len)
{
    Mp3Data* mp3 = static_cast<Mp3Data*>(clientdata);
    if (mp3 == nullptr || mp3->file == nullptr) return -1;
    if (mp3->pos >= mp3->audiolength) return 0;
    if (len > (size_t)(mp3->audiolength - mp3->pos)) len = (size_t)(mp3->audiolength - mp3->pos);
    try {
        mp3->file->seek((uint64_t)(mp3->audiostreamstart + mp3->pos));
        size_t got = mp3->file->read(buffer, len);
        mp3->pos += (int64_t)got;
        return (mpg123_ssize_t)got;
    } catch (Exception&) {
        // Die Rueckruffunktion laeuft in fremdem C-Code, eine Exception darf hier nicht herauslaufen
        return -1;
    }
}

/** @brief Rückruffunktion von libmpg123: Datenbasis in FileObject verschieben
 *
 * libmpg123 nutzt SEEK_END um die Groesse des Datenstroms zu ermitteln. Als Ende gilt die
 * Groesse des Audiodatenblocks, nicht das Dateiende, damit ein indexierter Sprung nie in
 * einem Anhang landet.
 */
static off_t SeekStream(void* clientdata, off_t offset, int origin)
{
    Mp3Data* mp3 = static_cast<Mp3Data*>(clientdata);
    if (mp3 == nullptr || mp3->file == nullptr) return -1;
    int64_t base;
    switch (origin) {
    case SEEK_SET:
        base = 0;
        break;
    case SEEK_CUR:
        base = mp3->pos;
        break;
    case SEEK_END:
        base = mp3->audiolength;
        break;
    default:
        return -1;
    }
    int64_t target = base + offset;
    if (target < 0) target = 0;
    if (target > mp3->audiolength) target = mp3->audiolength;
    try {
        mp3->file->seek((uint64_t)(mp3->audiostreamstart + target));
        mp3->pos = target;
        return (off_t)target;
    } catch (Exception&) {
        return -1;
    }
}

/** @brief Rückruffunktion von libmpg123: Datenbasis schliessen
 *
 * Die Datei gehoert dem Aufrufer und wird hier nicht geschlossen.
 */
static void CloseStream(void* /*clientdata*/)
{
}
#endif

AudioDecoder_MP3::AudioDecoder_MP3()
{
    position = 0;
#ifdef HAVE_MPG123
    mp3 = new (std::nothrow) Mp3Data;
    if (mp3 == nullptr) throw OutOfMemoryException();
    MPG123_GlobalMutex.lock();
    if (!mpg123_initialized) {
        int ret = mpg123_init();
        if (ret != MPG123_OK) {
            MPG123_GlobalMutex.unlock();
            delete mp3;
            mp3 = nullptr;
            throw DecoderInitializationException("mpg123_init: %s", mpg123_plain_strerror(ret));
        }
        mpg123_initialized = true;
    }
    mpg123_instances++;
    MPG123_GlobalMutex.unlock();
#else
    mp3 = nullptr;
#endif
}

AudioDecoder_MP3::~AudioDecoder_MP3()
{
#ifdef HAVE_MPG123
    close();
    if (mp3 != nullptr) {
        MPG123_GlobalMutex.lock();
        if (mpg123_instances) {
            mpg123_instances--;
            if (mpg123_instances == 0) {
                mpg123_exit();
                mpg123_initialized = false;
            }
        }
        MPG123_GlobalMutex.unlock();
        delete mp3;
        mp3 = nullptr;
    }
#endif
}

void AudioDecoder_MP3::close()
{
#ifndef HAVE_MPG123
    throw UnsupportedFeatureException("AudioDecoder_MP3: mpg123");
#else
    if (mp3 == nullptr) return;
    if (mp3->handle != nullptr) {
        mpg123_close(mp3->handle);
        mpg123_delete(mp3->handle);
        mp3->handle = nullptr;
    }
    free(mp3->outbuffer);
    mp3->outbuffer = nullptr;
    mp3->file = nullptr;
    mp3->audiostreamstart = 0;
    mp3->audiolength = 0;
    mp3->pos = 0;
    mp3->outcapacity = 0;
    mp3->outframes = 0;
    mp3->outpos = 0;
    mp3->channels = 0;
    mp3->encoding = MPG123_ENC_SIGNED_16;
    mp3->bufencoding = MPG123_ENC_SIGNED_16;
    mp3->eof = false;
    position = 0;
    info = AudioInfo();
#endif
}

const AudioInfo& AudioDecoder_MP3::getAudioInfo() const
{
    return info;
}

size_t AudioDecoder_MP3::getPosition() const noexcept
{
    return position;
}

#ifdef HAVE_MPG123

/** @brief setzt die Formatparameter des Decoders aus den ermittelten Audiodaten.
 *
 * libmpg123 waehlt einen Zielparameter aus der Formattabelle, der nicht zum kodierten Format
 * passt, indem es die Samples hochreampelt oder auf zwei Kanaele aufmischt. Deshalb wird die
 * Tabelle auf die reale Abtastrate und die reale Kanaelzahl gesetzt.
 */
static void ApplyDecoderFormat(mpg123_handle* handle, const AudioInfo& info, int encoding)
{
    mpg123_format_none(handle);
    int mask = (info.Channels == 1) ? MPG123_MONO : MPG123_STEREO;
    mpg123_format(handle, (long)info.Frequency, mask, encoding);
}
#endif // HAVE_MPG123

void AudioDecoder_MP3::open(FileObject& file, const AudioInfo* audioinfo)
{
#ifdef HAVE_MPG123
    close();
    if (mp3 == nullptr) throw DecoderInitializationException("MP3-Decoder nicht angelegt");

    if (audioinfo) {
        info = *audioinfo;
    } else if (!IdentAudioFile(file, info)) {
        throw UnsupportedAudioFormatException("File is not a MP3 file");
    }
    if (info.Format != AudioInfo::AudioFormat::MP3) {
        info = AudioInfo();
        throw UnsupportedAudioFormatException("File is not a MP3 file");
    }
    if (info.Channels == 0 || info.Channels > 2) {
        info = AudioInfo();
        throw UnsupportedAudioFormatException("MP3 with %zu channels is not supported", info.Channels);
    }
    if (info.Frequency == 0 || info.AudioStart >= info.AudioEnd) {
        info = AudioInfo();
        throw UnsupportedAudioFormatException("File contains no MP3 frames");
    }

    int err = 0;
    mp3->handle = mpg123_new(nullptr, &err);
    if (mp3->handle == nullptr) {
        close();
        throw DecoderInitializationException("mpg123_new: %s", mpg123_plain_strerror(err));
    }
    // MPG123_QUIET ist ein Bit aus mpg123_param_flags, kein eigener Parameterschluessel,
    // laesst sich also nur ueber die Flag-Kommandos setzen. ADD_FLAGS laesst die
    // Standardflags (Gapless, Auto-Resample) unangetastet; ohne dieses Bit schreibt
    // libmpg123 Notices und Fehlermeldungen ungefragt auf stderr.
    mpg123_param(mp3->handle, MPG123_ADD_FLAGS, MPG123_QUIET, 0);
    ApplyDecoderFormat(mp3->handle, info, mp3->encoding);
    mp3->channels = info.Channels;

    mp3->outcapacity = OUTBUFF;
    mp3->outbuffer = (unsigned char*)malloc(mp3->outcapacity);
    if (mp3->outbuffer == nullptr) {
        close();
        throw OutOfMemoryException();
    }
    mp3->file = &file;
    mp3->audiostreamstart = (int64_t)info.AudioStart;
    mp3->audiolength = (int64_t)info.AudioEnd - (int64_t)info.AudioStart;
    mp3->pos = 0;
    mp3->file->seek(info.AudioStart);

    if (mpg123_replace_reader_handle(mp3->handle, ReadStream, SeekStream, CloseStream) != MPG123_OK) {
        std::string msg = DecoderError(mp3->handle);
        close();
        throw DecoderInitializationException("mpg123_replace_reader_handle: %s", msg.c_str());
    }
    if (mpg123_open_handle(mp3->handle, mp3) != MPG123_OK) {
        std::string msg = DecoderError(mp3->handle);
        close();
        throw DecoderInitializationException("mpg123_open_handle: %s", msg.c_str());
    }
    position = 0;
#else
    (void)file;
    (void)audioinfo;
    throw UnsupportedFeatureException("AudioDecoder_MP3: mpg123");
#endif
}

/** @brief gleicht AudioInfo an die vom Decoder gemeldeten Parameter an
 *
 * Eine MP3-Datei kann in den Rahmen unterschiedliche Abtastraten und Kanaelzahlen verwenden.
 * Der Decoder haelt sie auf den in open() festgelegten Zielwerten, die Information ueber das
 * tatsaechlich kodierte Format stammt deshalb von libmpg123.
 */
void AudioDecoder_MP3::syncFormat()
{
#ifdef HAVE_MPG123
    long rate = 0;
    int channels = 0;
    int encoding = 0;
    if (mpg123_getformat(mp3->handle, &rate, &channels, &encoding) != MPG123_OK) return;
    if (rate <= 0 || channels < 1 || channels > 2) return;
    if ((size_t)rate == info.Frequency && (size_t)channels == mp3->channels) return;
    info.Frequency = (size_t)rate;
    info.Channels = (size_t)channels;
    info.BitsPerSample = (uint8_t)(SampleWidth(mp3->encoding) * 8);
    info.BytesPerSample = (uint8_t)(info.Channels * SampleWidth(mp3->encoding));
    mp3->channels = (size_t)channels;
    ApplyDecoderFormat(mp3->handle, info, mp3->encoding);
#endif // HAVE_MPG123
}

void AudioDecoder_MP3::seekSample(size_t sample)
{
    if (mp3 == nullptr) throw DecoderException("No MP3 file opened");
#ifdef HAVE_MPG123
    if (mp3->handle == nullptr) throw DecoderException("No MP3 file opened");
    if (sample == position) return;
    mp3->outframes = 0;
    mp3->outpos = 0;
    mp3->eof = false;
    off_t ret = mpg123_seek(mp3->handle, (off_t)sample, SEEK_SET);
    if (ret < 0) {
        std::string msg = DecoderError(mp3->handle);
        throw DecoderException("mpg123_seek: %s", msg.c_str());
    }
    position = (size_t)ret;
    syncFormat();
#else
    throw UnsupportedFeatureException("AudioDecoder_MP3: mpg123");
#endif // HAVE_MPG123
}

/** @brief stellt die Kodierung des Ausgabepuffers ein
 *
 * Samples, die sich noch im internen Puffer befinden, bleiben erhalten und werden von
 * `transferFrames()` in die neue Kodierung umgerechnet. Alle weiteren Frames liefert
 * libmpg123 in der neuen Kodierung.
 */
void AudioDecoder_MP3::setTargetEncoding(int encoding)
{
#ifdef HAVE_MPG123
    if (mp3 == nullptr || mp3->encoding == encoding) return;
    mp3->encoding = encoding;
    ApplyDecoderFormat(mp3->handle, info, encoding);
    const size_t sampleWidth = (encoding == MPG123_ENC_FLOAT_32) ? sizeof(float) : sizeof(int16_t);
    info.BitsPerSample = (uint8_t)(sampleWidth * 8);
    info.BytesPerSample = (uint8_t)(mp3->channels * sampleWidth);
#endif
}

bool AudioDecoder_MP3::fillFrames()
{
#ifdef HAVE_MPG123
    if (mp3 == nullptr || mp3->handle == nullptr || mp3->eof) return false;
    bool syncneeded = (mp3->outframes == 0 && mp3->outpos == 0);
    const size_t sampleWidth = (mp3->encoding == MPG123_ENC_FLOAT_32) ? sizeof(float) : sizeof(int16_t);
    const size_t width = (size_t)mp3->channels * sampleWidth;

    for (;;) {
        size_t bytes = 0;
        int ret = mpg123_read(mp3->handle, mp3->outbuffer, mp3->outcapacity, &bytes);
        if (bytes > 0) {
            mp3->outframes = bytes / width;
            mp3->outpos = 0;
            if (mp3->outframes != 0) {
                if (syncneeded) {
                    syncFormat();
                    syncneeded = false;
                }
                return true;
            }
        }
        if (ret == MPG123_NEW_FORMAT) {
            if (syncneeded) {
                syncFormat();
                syncneeded = false;
            }
            continue;
        }
        if (ret == MPG123_DONE || ret == MPG123_NEED_MORE) {
            mp3->eof = true;
            return false;
        }
        // Lesefehler / defekter Frame: Stream sauber beenden
        mp3->eof = true;
        return false;
    }
#else
    return false;
#endif
}

size_t AudioDecoder_MP3::getSamples(size_t num, STEREOSAMPLE16* buffer) noexcept
{
#ifdef HAVE_MPG123
    if (mp3 == nullptr || buffer == nullptr || mp3->handle == nullptr || num == 0) return 0;
    setTargetEncoding(MPG123_ENC_SIGNED_16);
    const bool stereo = (mp3->channels == 2);
    size_t done = 0;
    while (done < num) {
        if (mp3->outpos >= mp3->outframes) {
            if (!fillFrames()) break;
        }
        size_t avail = mp3->outframes - mp3->outpos;
        size_t n = (num - done < avail) ? (num - done) : avail;
        const int16_t* src = reinterpret_cast<const int16_t*>(mp3->outbuffer) + mp3->outpos * mp3->channels;
        if (stereo) {
            memcpy(&buffer[done], src, n * sizeof(STEREOSAMPLE16));
        } else {
            for (size_t i = 0; i < n; i++) {
                buffer[done + i].left = src[i];
                buffer[done + i].right = src[i];
            }
        }
        mp3->outpos += n;
        done += n;
    }
    position += done;
    return done;
#else
    return 0;
#endif
}

size_t AudioDecoder_MP3::addSamples(size_t num, STEREOSAMPLE32* buffer) noexcept
{
#ifdef HAVE_MPG123
    if (mp3 == nullptr || buffer == nullptr || mp3->handle == nullptr || num == 0) return 0;
    setTargetEncoding(MPG123_ENC_SIGNED_16);
    const bool stereo = (mp3->channels == 2);
    size_t done = 0;
    while (done < num) {
        if (mp3->outpos >= mp3->outframes) {
            if (!fillFrames()) break;
        }
        size_t avail = mp3->outframes - mp3->outpos;
        size_t n = (num - done < avail) ? (num - done) : avail;
        const int16_t* src = reinterpret_cast<const int16_t*>(mp3->outbuffer) + mp3->outpos * mp3->channels;
        if (stereo) {
            for (size_t i = 0; i < n; i++) {
                buffer[done + i].left += src[i * 2];
                buffer[done + i].right += src[i * 2 + 1];
            }
        } else {
            for (size_t i = 0; i < n; i++) {
                buffer[done + i].left += src[i];
                buffer[done + i].right += src[i];
            }
        }
        mp3->outpos += n;
        done += n;
    }
    position += done;
    return done;
#else
    return 0;
#endif
}

size_t AudioDecoder_MP3::getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept
{
#ifdef HAVE_MPG123
    if (mp3 == nullptr || buffer == nullptr || mp3->handle == nullptr || num == 0) return 0;
    setTargetEncoding(MPG123_ENC_FLOAT_32);
    const bool stereo = (mp3->channels == 2);
    size_t done = 0;
    while (done < num) {
        if (mp3->outpos >= mp3->outframes) {
            if (!fillFrames()) break;
        }
        size_t avail = mp3->outframes - mp3->outpos;
        size_t n = (num - done < avail) ? (num - done) : avail;
        const float* src = reinterpret_cast<const float*>(mp3->outbuffer) + mp3->outpos * mp3->channels;
        if (stereo) {
            memcpy(&buffer[done], src, n * sizeof(STEREOSAMPLE_FLOAT));
        } else {
            for (size_t i = 0; i < n; i++) {
                buffer[done + i].left = src[i];
                buffer[done + i].right = src[i];
            }
        }
        mp3->outpos += n;
        done += n;
    }
    position += done;
    return done;
#else
    return 0;
#endif
}

size_t AudioDecoder_MP3::addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept
{
#ifdef HAVE_MPG123
    if (mp3 == nullptr || buffer == nullptr || mp3->handle == nullptr || num == 0) return 0;
    setTargetEncoding(MPG123_ENC_FLOAT_32);
    const bool stereo = (mp3->channels == 2);
    size_t done = 0;
    while (done < num) {
        if (mp3->outpos >= mp3->outframes) {
            if (!fillFrames()) break;
        }
        size_t avail = mp3->outframes - mp3->outpos;
        size_t n = (num - done < avail) ? (num - done) : avail;
        const float* src = reinterpret_cast<const float*>(mp3->outbuffer) + mp3->outpos * mp3->channels;
        if (stereo) {
            for (size_t i = 0; i < n; i++) {
                buffer[done + i].left += src[i * 2];
                buffer[done + i].right += src[i * 2 + 1];
            }
        } else {
            for (size_t i = 0; i < n; i++) {
                buffer[done + i].left += src[i];
                buffer[done + i].right += src[i];
            }
        }
        mp3->outpos += n;
        done += n;
    }
    position += done;
    return done;
#else
    return 0;
#endif
}

} // namespace pplib
