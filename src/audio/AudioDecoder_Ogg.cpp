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
    // Frames der ersten Kette: bei verketteten Ogg-Dateien zaehlt ov_pcm_total(-1) alle
    // Ketten, gespielt wird aber nur die erste (siehe Klassen-Doku).
    long firstChainFrames;

} OggVorbisPrivateData;

/** @brief Frameanzahl der ersten Kette, 0 bei Fehler oder leerer Datei
 *
 * ov_pcm_total(&vf, -1) zaehlt alle Ketten einer verketteten Datei und liefert -1 bei
 * Fehlern; der Wert wuerde als Mega-Zahl in AudioInfo landen.
 */
static long FirstChainFrames(OggVorbis_File& vf)
{
    long frames = ov_pcm_total(&vf, 0);
    if (frames < 0) frames = ov_pcm_total(&vf, -1);
    return (frames < 0) ? 0 : frames;
}

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
    // try deckt den ganzen Koerper ab: tell() und size() koennen ebenso werfen wie seek().
    try {
        FileObject::SeekOrigin origin;
        if (whence == SEEK_SET)
            origin = FileObject::SeekOrigin::SEEKSET;
        else if (whence == SEEK_CUR)
            origin = FileObject::SeekOrigin::SEEKCUR;
        else if (whence == SEEK_END)
            origin = FileObject::SeekOrigin::SEEKEND;
        else
            return -1;

        const int64_t base = (origin == FileObject::SeekOrigin::SEEKCUR)   ? (int64_t)f->tell()
                             : (origin == FileObject::SeekOrigin::SEEKEND) ? (int64_t)f->size()
                                                                           : 0;
        const int64_t target = base + (int64_t)offset;
        if (target < 0) return -1; // vor Dateianfang
        return (f->seek(offset, origin) == (uint64_t)target) ? 0 : -1;
    } catch (...) {
        return -1;
    }
}

static long ppl_tell_func(void* datasource)
{
    // Rueckgabewert -1 gilt libvorbis als Fehler; eine Exception duerfte nicht durch die
    // C-Frames von libvorbis hindurch laufen.
    try {
        return (long)static_cast<FileObject*>(datasource)->tell();
    } catch (...) {
        return -1;
    }
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
    OggVorbis_File* get()
    {
        return &vf;
    }

private:
    OggVorbis_File vf;
};

/** \brief Fuellt `AudioInfo` aus den Vorbis-Headern.
 *
 * `AudioInfo::Samples` und `AudioInfo::Length` beziehen sich nur auf die erste Kette,
 * weil der Decoder ausschliesslich die erste Kette abspielt (siehe `FirstChainFrames()`).
 * `AudioInfo::AudioStart` und `AudioInfo::AudioEnd` sind Seitengrenzen des Ogg-Containers
 * und enthalten damit die Seitenkoepfe. `AudioInfo::AudioSize` und `AudioInfo::AudioEnd`
 * bleiben ungesetzt, wenn libvorbisfile das Datenende nicht bestimmen kann, zum Beispiel
 * bei nicht seekbarem Input (`vf.end <= vf.offset`).
 *
 * Die Bitrate stammt aus den von libvorbis gemeldeten nominalen, unteren und oberen
 * Bitraten und ist eine Naeherung: sind untere und obere Bitrate gleich, gilt die Datei
 * als konstante Bitrate, sonst zaehlt die nominale Bitrate. Eine mit `libvorbis -q`
 * kodierte VBR-Datei wird darueber in der Regel als CBR gemeldet. Negative (libvorbis
 * meldet -1 fuer "unbekannt") und uebergrosse Werte - manche Encoder belegen die Felder
 * gar nicht - werden auf 0 bis 65535 kbit/s geklemmt.
 */
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
    // Klemmen VOR dem Cast: einen Double, der ausserhalb des Wertebereichs von
    // uint64_t liegt (manche Encoder schreiben uninitialisierte Headerfelder), zu casten
    // ist undefiniertes Verhalten. NaN trifft denselben Zweig.
    if (!(bits >= 0.0)) bits = 0.0;
    if (bits > (double)UINT16_MAX * 1000.0) bits = (double)UINT16_MAX * 1000.0;
    info.Bitrate = (uint16_t)((uint64_t)bits / 1000);
    info.BitsPerSample = 16;
    info.SampleType = AudioInfo::SampleEncoding::INTEGER;
    info.Mode = (info.Channels == 1) ? AudioInfo::ChannelMode::MONO : AudioInfo::ChannelMode::STEREO;
    info.AudioStart = vf.offset;

    // AudioEnd ist - wie bei WAV, MP3 und AIFF - die Position des letzten Datenbytes.
    if (vf.end > vf.offset) {
        info.AudioSize = vf.end - vf.offset;
        info.AudioEnd = info.AudioStart + info.AudioSize - 1;
    }

    info.BytesPerSample = info.BitsPerSample * info.Channels / 8;
    info.Samples = (uint64_t)FirstChainFrames(vf);
    if (info.Frequency > 0) info.Length = (uint32_t)((uint64_t)info.Samples * 1000 / info.Frequency);
}

#endif

/** \brief Prueft, ob die Datei ein Ogg-Vorbis-Stream ist, und fuellt `AudioInfo` mit den
 *         Headern des Bitstroms.
 *
 * Die Funktion lehnt nichts ab, was der Decoder spaeter nicht unterstuetzt: sie meldet
 * Kanalzahl, Abtastrate und Bitrate des ersten Bitstroms, auch bei Dateien mit mehr als
 * zwei Kanaelen. Abgeschlossen wird das Format erst in `AudioDecoder_Ogg::open()`.
 *
 * Die Funktion oeffnet und schliesst die Datei, veraendert also die Dateiposition.
 *
 * Aufgerufen wird sie von `IdentAudioFile()` in `AudioInfo.cpp`, die dort eine eigene
 * Deklaration traegt; eine Header-Deklaration ist nicht gewollt (modulintern).
 */
bool IdentOggVorbisAudioFile(FileObject& file, AudioInfo& info)
{
#ifdef HAVE_LIBVORBIS
    OggVorbisHandle handle;
    OggVorbis_File* vf = handle.get();
    ov_callbacks callbacks;
    init_callback(callbacks);
    file.seek(0);
    int res = ov_open_callbacks(&file, vf, NULL, 0, callbacks);
    if (res != 0) return false; // Handle-Destruktor uebernimmt ov_clear()
    GetVorbisAudioInfo(info, *vf, file);
    return true;

#else
    return false;
#endif
}

AudioDecoder_Ogg::AudioDecoder_Ogg()
{
    // Der Handle entsteht erst in open(); NULL steht fuer "nicht geoeffnet" und die
    // Lese- und Seek-Funktionen werfen in diesem Zustand eine DecoderException.
    private_data = NULL;
    position = 0;
    decodebuffer_size = 0;
    decodebuffer = NULL;
}

AudioDecoder_Ogg::~AudioDecoder_Ogg()
{
#ifdef HAVE_LIBVORBIS
    close();
#endif
}

void AudioDecoder_Ogg::open(FileObject& file, const AudioInfo* info)
{
#ifndef HAVE_LIBVORBIS
    throw UnsupportedFeatureException("AudioDecoder_Ogg");
#else
    // open() uebernimmt die Freigabe eines bereits vorhandenen Handles: ein zweites
    // open() ohne close() liess sonst den kompletten libvorbisfile-Zustand stehen,
    // und ein open() nach close() arbeitete auf dem NULL-Zeiger.
    close();
    private_data = calloc(sizeof(OggVorbisPrivateData), 1);
    if (!private_data) throw OutOfMemoryException();
    OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
    init_callback(oggp->callbacks);
    file.seek(0);
    int res = ov_open_callbacks(&file, &oggp->vf, NULL, 0, oggp->callbacks);
    if (res != 0) {
        close(); // ov_clear() gibt auch den halb aufgebauten Handle frei
        throw UnsupportedAudioFormatException();
    }
    if (!info) {
        GetVorbisAudioInfo(this->info, oggp->vf, file);
    } else {
        this->info = *info;
    }

    // Die Kanalzahl wird aussliesslich hier geprueft, die Lesepfade verlasssen sich darauf.
    if (this->info.Channels < 1 || this->info.Channels > 2) {
        close();
        throw UnsupportedAudioFormatException("channels <1 or >2");
    }
    // Samples und Length beschreiben das, was abgespielt wird: nur die erste Kette
    oggp->firstChainFrames = FirstChainFrames(oggp->vf);
    this->info.Samples = (uint64_t)oggp->firstChainFrames;
    if (this->info.Frequency > 0) this->info.Length = (uint32_t)((uint64_t)this->info.Samples * 1000 / this->info.Frequency);

    // Interner Lese-Puffer, von allen Lesepfaden benutzt; damit bleibt die bei
    // ov_read() angeforderte Bytezahl konstant und passt in ein int.
    decodebuffer_size = 2048 * 2 * 2; // Frames * Kanaele * Bytes je Kanal-Sample
    decodebuffer = (char*)malloc(decodebuffer_size);
    if (!decodebuffer) {
        close();
        throw OutOfMemoryException();
    }

    position = 0;
#endif
}

void AudioDecoder_Ogg::close()
{
#ifndef HAVE_LIBVORBIS
    throw UnsupportedFeatureException("AudioDecoder_Ogg");
#else
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
    position = 0;
#endif
}

const AudioInfo& AudioDecoder_Ogg::getAudioInfo() const
{
    return info;
}

void AudioDecoder_Ogg::seekSample(size_t sample)
{
#ifndef HAVE_LIBVORBIS
    throw UnsupportedFeatureException("AudioDecoder_Ogg");
#else
    if (private_data == NULL) throw DecoderException("AudioDecoder_Ogg: Decoder ist nicht geoeffnet");
    OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
    const size_t limit = (size_t)oggp->firstChainFrames;
    // ov_pcm_seek() nimmt auch Ziele hinter dem Ende der ersten Kette an und setzt sie
    // still auf eine gueltige Position um; solche Ziele waren in Review ein stiller Fehl-Seek.
    if (sample > limit) throw DecoderException("AudioDecoder_Ogg: Ziel hinter dem Ende der ersten Kette");
    if (sample == limit) {
        position = sample;
        return;
    }
    if (ov_pcm_seek(&oggp->vf, (ogg_int64_t)sample) != 0) throw DecoderException("AudioDecoder_Ogg: ov_pcm_seek fehlgeschlagen");
    position = sample;
#endif
}

size_t AudioDecoder_Ogg::getPosition() const noexcept
{
    return position;
}

size_t AudioDecoder_Ogg::getSamples(size_t num, STEREOSAMPLE16* buffer) noexcept
{
#ifndef HAVE_LIBVORBIS
    return 0;
#else
    if (private_data == nullptr || buffer == nullptr) return 0;
    OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
    const size_t limit = (size_t)oggp->firstChainFrames;
    if (position >= limit) return 0;
    size_t want = num;
    if (want > limit - position) want = limit - position;

    const size_t channels = info.Channels; // open() garantiert 1 oder 2 Kanaele
    const size_t framebytes = channels * 2;
    const size_t maxframes = (size_t)decodebuffer_size / framebytes;
    size_t total = 0;
    int bitstream = 0;
    while (total < want) {
        size_t frames = want - total;
        if (frames > maxframes) frames = maxframes;
        // libvorbisfile liefert pro Aufruf hoechstens einen Decode-Block und puffert den
        // Rest, die gelieferte Bytezahl bleibt also oft unter der Anfrage. Abbruch nur am
        // Dateiende (Rueckgabewert 0); ein Rest unter einem Frame kann nicht uebernommen werden.
        long bytes_read = ov_read(&oggp->vf, decodebuffer, (int)(frames * framebytes), 0, 2, 1, &bitstream);
        if (bytes_read < 0) return 0;
        if (bytes_read == 0) break; // Dateiende
        const pplib::SAMPLE16* src = (const pplib::SAMPLE16*)decodebuffer;
        size_t got = (size_t)bytes_read / framebytes;
        if (got == 0) break;
        if (channels == 2) {
            for (size_t i = 0; i < got; i++) {
                buffer[total + i].left = src[i * 2];
                buffer[total + i].right = src[i * 2 + 1];
            }
        } else {
            for (size_t i = 0; i < got; i++) {
                buffer[total + i].left = src[i]; // Mono: gleicher Wert auf beide Kanaele
                buffer[total + i].right = src[i];
            }
        }
        total += got;
    }
    position += total;
    return total;
#endif
}

size_t AudioDecoder_Ogg::addSamples(size_t num, STEREOSAMPLE32* buffer) noexcept
{
#ifndef HAVE_LIBVORBIS
    return 0;
#else
    if (private_data == nullptr || buffer == nullptr) return 0;
    OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
    const size_t limit = (size_t)oggp->firstChainFrames;
    if (position >= limit) return 0;
    size_t want = num;
    if (want > limit - position) want = limit - position;

    const size_t channels = info.Channels; // open() garantiert 1 oder 2 Kanaele
    const size_t framebytes = channels * 2;
    const size_t maxframes = (size_t)decodebuffer_size / framebytes;
    size_t total = 0;
    int bitstream = 0;
    while (total < want) {
        size_t frames = want - total;
        if (frames > maxframes) frames = maxframes;
        long bytes_read = ov_read(&oggp->vf, decodebuffer, (int)(frames * framebytes), 0, 2, 1, &bitstream);
        if (bytes_read < 0) return 0;
        if (bytes_read == 0) break; // Dateiende
        const pplib::SAMPLE16* src = (const pplib::SAMPLE16*)decodebuffer;
        size_t got = (size_t)bytes_read / framebytes;
        if (got == 0) break;
        if (channels == 2) {
            for (size_t i = 0; i < got; i++) {
                buffer[total + i].left += src[i * 2];
                buffer[total + i].right += src[i * 2 + 1];
            }
        } else {
            for (size_t i = 0; i < got; i++) {
                buffer[total + i].left += src[i];
                buffer[total + i].right += src[i];
            }
        }
        total += got;
    }
    position += total;
    return total;
#endif
}

size_t AudioDecoder_Ogg::getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept
{
#ifndef HAVE_LIBVORBIS
    return 0;
#else
    if (private_data == nullptr || buffer == nullptr) return 0;
    OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
    const size_t limit = (size_t)oggp->firstChainFrames;
    if (position >= limit) return 0;
    size_t want = num;
    if (want > limit - position) want = limit - position;

    const size_t channels = info.Channels; // open() garantiert 1 oder 2 Kanaele
    size_t total = 0;
    int bitstream = 0;
    while (total < want) {
        size_t frames = want - total;
        if (frames > 8192) frames = 8192; // eine ov_read_float-Anfrage passt immer in ein int
        float** pcm = nullptr;
        // ov_read_float() zaehlt Limit und Rueckgabewert in Frames und liefert hoechstens
        // einen Decode-Block. Nicht abgenommene Frames eines Blocks verwirft libvorbisfile,
        // darum wird nie mehr angefordert als in diesem Aufruf auch ubernommen wird.
        long got = ov_read_float(&oggp->vf, &pcm, (int)frames, &bitstream);
        if (got < 0) return 0;
        // Rueckgabe 0 ist Dateiende; ohne Sample-Zeiger kann nichts ubernommen werden.
        if (got == 0 || pcm == NULL) break;
        const long n = (got > (long)frames) ? (long)frames : got;
        // Die Kanalzahl steht ausserhalb der inneren Schleife, analog zu den anderen Pfaden.
        if (channels == 2) {
            for (long i = 0; i < n; i++) {
                buffer[total + i].left = pcm[0][i];
                buffer[total + i].right = pcm[1][i];
            }
        } else {
            for (long i = 0; i < n; i++) {
                buffer[total + i].left = pcm[0][i];
                buffer[total + i].right = pcm[0][i]; // Mono: gleicher Wert auf beide Kanaele
            }
        }
        total += (size_t)n;
    }
    position += total;
    return total;
#endif
}

size_t AudioDecoder_Ogg::addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept
{
#ifndef HAVE_LIBVORBIS
    return 0;
#else
    if (private_data == nullptr || buffer == nullptr) return 0;
    OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
    const size_t limit = (size_t)oggp->firstChainFrames;
    if (position >= limit) return 0;
    size_t want = num;
    if (want > limit - position) want = limit - position;

    const size_t channels = info.Channels; // open() garantiert 1 oder 2 Kanaele
    size_t total = 0;
    int bitstream = 0;
    while (total < want) {
        size_t frames = want - total;
        if (frames > 8192) frames = 8192; // eine ov_read_float-Anfrage passt immer in ein int
        float** pcm = nullptr;
        // Limit und Rueckgabewert von ov_read_float zaehlen in Frames; ungenutzte Frames
        // eines Blocks gehen verloren, darum wird nie mehr angefordert als uebernommen wird.
        long got = ov_read_float(&oggp->vf, &pcm, (int)frames, &bitstream);
        if (got < 0) return 0;
        // Rueckgabe 0 ist Dateiende; ohne Sample-Zeiger kann nichts ubernommen werden.
        if (got == 0 || pcm == NULL) break;
        const long n = (got > (long)frames) ? (long)frames : got;
        // Die Kanalzahl steht ausserhalb der inneren Schleife, analog zu den anderen Pfaden.
        if (channels == 2) {
            for (long i = 0; i < n; i++) {
                buffer[total + i].left += pcm[0][i];
                buffer[total + i].right += pcm[1][i];
            }
        } else {
            for (long i = 0; i < n; i++) {
                buffer[total + i].left += pcm[0][i];
                buffer[total + i].right += pcm[0][i]; // Mono: gleicher Wert auf beide Kanaele
            }
        }
        total += (size_t)n;
    }
    position += total;
    return total;
#endif
}

} // namespace pplib
