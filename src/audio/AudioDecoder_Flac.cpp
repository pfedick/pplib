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
 * CONSEQUENTIAL DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <string.h>
#include <vector>
#include <config_pplib.h>
#include <pplib/core/fileobject.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/decoder.h>

#ifdef HAVE_LIBFLAC
#include <FLAC/stream_decoder.h>
#endif

namespace pplib
{

#ifdef HAVE_LIBFLAC

//! Sinnbild fuer "kein laufender Sprung" in FlacState::seekTarget
static const uint64_t kNoSeek = 0xFFFFFFFFFFFFFFFFULL;

/** @brief libFLAC-Zustand eines Decoders
 *
 * Die Rohsamples stehen verskraenkt (links, rechts, links, ...) und als vorzeichenbehaftete
 * `int32_t`-Werte im libFLAC-Urwertebereich (rechtsbuendig) bereit. Die Zahl der Kanäle ist
 * immer zwei; bei Mono-Quellen wird das einzelne Sample beim Decodieren dupliziert.
 */
struct FlacState
{
    FLAC__StreamDecoder* dec;
    FileObject* file;
    uint64_t base;  //!< Dateiversatz des "fLaC"-Magic: libFLAC zaehlt ab hier
    std::vector<int32_t> raw;
    size_t rawWrite;  //!< Schreibzeiger in Kanal-Samples
    size_t rawRead;   //!< Leszeiger in Kanal-Samples
    uint32_t channels;
    bool atEnd;       //!< libFLAC meldete Ende des Samples-Datensatzes
    bool error;       //!< Decodier- oder Callbackfehler
    bool haveMeta;
    uint32_t metaChannels;
    uint32_t metaBits;
    uint32_t metaMaxBlocksize;  //!< STREAMINFO max blocksize: bemisst die Reservierung des Rohpuffers
    uint64_t metaSamples;  //!< STREAMINFO Gesamtzahl Samples, 0 = unbekannt
    uint64_t totalFrames;  //!< bisher gelieferte Frames, auch ueber den Lesepuffer zurueck
    uint64_t seekTarget;   //!< Samplezahl, bis zu der gelieferte Frames verworfen werden (kNoSeek = keiner)

    FlacState()
        : dec(nullptr), file(nullptr), base(0), rawWrite(0), rawRead(0), channels(2),
          atEnd(false), error(false), haveMeta(false), metaChannels(0), metaBits(0),
          metaMaxBlocksize(0), metaSamples(0), totalFrames(0), seekTarget(kNoSeek)
    {
    }
};

/** @brief RAII-Huelle fuer FLAC__stream_decoder
 *
 * Nach fehlgeschlagener Initialisierung muss die Instanz trotzdem mit
 * FLAC__stream_decoder_delete() freigegeben werden; ein frueher Rueckgabe- oder
 * Throw-Pfad wuerde sie sonst leakieren.
 */
class FlacDecoderHandle
{
public:
    FlacDecoderHandle() { dec = FLAC__stream_decoder_new(); }
    FlacDecoderHandle(const FlacDecoderHandle&) = delete;
    FlacDecoderHandle& operator=(const FlacDecoderHandle&) = delete;
    ~FlacDecoderHandle()
    {
        if (dec != nullptr) FLAC__stream_decoder_delete(dec);
    }
    FLAC__StreamDecoder* get() { return dec; }
    void release() { dec = nullptr; }  //!< Uebergibt die Instanz an den Aufrufer

private:
    FLAC__StreamDecoder* dec;
};

/** @brief Lese-Callback: libFLAC liest ab Dateiversatz `base`.
 *
 * Eine Exception durch die C-Frame-Ebenen von libFLAC waere nicht portabel definiert,
 * deshalb werden Dateiende und Lesefehler als Status gemeldet.
 */
static FLAC__StreamDecoderReadStatus flacReadCb(const FLAC__StreamDecoder*, FLAC__byte buffer[], size_t* bytes, void* client)
{
    FlacState* st = static_cast<FlacState*>(client);
    if (*bytes == 0) return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    // pplib meldet das Dateiende uber eine Exception, libFLAC erwartet einen Status.
    // Deshalb wird die Anfrage vorher auf das verbleibende Reststueck beschraenkt.
    const uint64_t pos = st->file->tell();
    const uint64_t total = st->file->size();
    if (pos >= total) {
        *bytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }
    if (*bytes > (size_t)(total - pos)) *bytes = (size_t)(total - pos);
    try {
        const size_t got = st->file->read(buffer, *bytes);
        if (got == 0) {
            *bytes = 0;
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        }
        *bytes = got;
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    } catch (...) {
        *bytes = 0;
        st->error = true;
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
}

static FLAC__StreamDecoderSeekStatus flacSeekCb(const FLAC__StreamDecoder*, FLAC__uint64 absolute_byte_offset, void* client)
{
    FlacState* st = static_cast<FlacState*>(client);
    const uint64_t target = st->base + absolute_byte_offset;
    if (target < st->base || target > st->file->size()) return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    try {
        if (st->file->seek(target, FileObject::SeekOrigin::SEEKSET) != target) return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    } catch (...) {
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }
}

static FLAC__StreamDecoderTellStatus flacTellCb(const FLAC__StreamDecoder*, FLAC__uint64* absolute_byte_offset, void* client)
{
    FlacState* st = static_cast<FlacState*>(client);
    const uint64_t now = st->file->tell();
    if (now < st->base) return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
    *absolute_byte_offset = now - st->base;
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

static FLAC__StreamDecoderLengthStatus flacLengthCb(const FLAC__StreamDecoder*, FLAC__uint64* stream_length, void* client)
{
    FlacState* st = static_cast<FlacState*>(client);
    const uint64_t total = st->file->size();
    if (total < st->base) return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
    *stream_length = total - st->base;
    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

static FLAC__bool flacEofCb(const FLAC__StreamDecoder*, void* client)
{
    FlacState* st = static_cast<FlacState*>(client);
    return st->file->eof() ? true : false;
}

/** @brief Write-Callback: Rahmen werden verskraenkt und im Rohpuffer gesammelt.
 *
 * Mono-Rahmen werden auf beide Kanale kopiert. Uber den reservierten Platz hinaus
 * wächst der Puffer bei Bedarf; schlägt das fehl, wird der Abbruch gemeldet und der
 * Fehlerzustand gesetzt - eine Exception darf die C-Ebenen nicht durchlaufen.
 */
static FLAC__StreamDecoderWriteStatus flacWriteCb(const FLAC__StreamDecoder*, const FLAC__Frame* frame, const int32_t* const buffer[], void* client)
{
    FlacState* st = static_cast<FlacState*>(client);
    size_t frames = frame->header.blocksize;
    if (frame->header.channels != st->channels) {
        st->error = true;  // Kanalzahl wechselt mitten im Stream: nicht unterstuetzt
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    if (frames == 0 || buffer[0] == nullptr) return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    // Nach seekSample() liefert libFLAC auch Raeume, die vor dem Ziel liegen (einschliesslich
    /// der beim Suchen intern gelesenen). Alles vor dem Ziel wird hier verworfen.
    size_t start = 0;
    if (st->seekTarget != kNoSeek) {
        if (frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER) {
            const uint64_t first = frame->header.number.sample_number;
            if (first + frames <= st->seekTarget) return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
            if (first < st->seekTarget) start = (size_t)(st->seekTarget - first);
        }
        st->seekTarget = kNoSeek;
    }
    frames -= start;
    try {
        st->raw.resize(st->rawWrite + frames * 2);
    } catch (...) {
        st->error = true;
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    int32_t* out = st->raw.data() + st->rawWrite;
    if (st->channels == 1) {
        for (size_t i = start; i < start + frames; i++) {
            out[0] = buffer[0][i];
            out[1] = buffer[0][i];
            out += 2;
        }
    } else {
        for (size_t i = start; i < start + frames; i++) {
            out[0] = buffer[0][i];
            out[1] = buffer[1][i];
            out += 2;
        }
    }
    st->rawWrite += frames * 2;
    st->totalFrames += frames;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void flacMetadataCb(const FLAC__StreamDecoder*, const FLAC__StreamMetadata* metadata, void* client)
{
    FlacState* st = static_cast<FlacState*>(client);
    if (metadata->type != FLAC__METADATA_TYPE_STREAMINFO) return;
    st->haveMeta = true;
    st->metaChannels = metadata->data.stream_info.channels;
    st->metaBits = metadata->data.stream_info.bits_per_sample;
    st->metaMaxBlocksize = metadata->data.stream_info.max_blocksize;
    st->metaSamples = metadata->data.stream_info.total_samples;
}

static void flacErrorCb(const FLAC__StreamDecoder*, FLAC__StreamDecoderErrorStatus status, void* client)
{
    FlacState* st = static_cast<FlacState*>(client);
    // Nach dem letzten Frame koennen abhangende Daten (z.B. ein ID3v1-Tag) als SYNC-Verlust
    // oder unlesbarer Frame gemeldet werden. Steht die Samplezahl fest und ist das Sample-
    // Ende schon ueberschritten, ist das unbeachtlich.
    if (status == FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC ||
        status == FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM)
    {
        if (st->metaSamples > 0 && st->totalFrames >= st->metaSamples) return;
    }
    st->error = true;
}

/** @brief Sucht den Dateiversatz des "fLaC"-Magic, auch hinter einem ID3v2-Tag.
 *
 *  @return Position der Magie, oder 0xFFFFFFFFFFFFFFFF, falls sie nicht gefunden wird.
 */
static uint64_t FlacStreamStart(FileObject& file)
{
    const uint64_t fsize = file.size();
    if (fsize < 44) return 0xFFFFFFFFFFFFFFFFULL;  // 10 Byte Tag + 4 Magie + Kopf + STREAMINFO
    char head[10];
    if (file.seek(0, FileObject::SeekOrigin::SEEKSET) != 0) return 0xFFFFFFFFFFFFFFFFULL;
    if (file.read(head, 4) != 4) return 0xFFFFFFFFFFFFFFFFULL;
    uint64_t pos = 0;
    if (memcmp(head, "ID3", 3) == 0) {
        char tag[10];
        memcpy(tag, head, 4);
        if (file.read(tag + 4, 6) != 6) return 0xFFFFFFFFFFFFFFFFULL;
        if ((uint8_t)tag[3] < 2) return 0xFFFFFFFFFFFFFFFFULL;
        const uint32_t size = ((uint32_t)(uint8_t)tag[6] << 21) | ((uint32_t)(uint8_t)tag[7] << 14)
                            | ((uint32_t)(uint8_t)tag[8] << 7) | (uint8_t)tag[9];
        pos = 10 + size;
        if (pos + 4 > fsize) return 0xFFFFFFFFFFFFFFFFULL;
        if (file.seek(pos, FileObject::SeekOrigin::SEEKSET) != pos) return 0xFFFFFFFFFFFFFFFFULL;
        if (file.read(head, 4) != 4) return 0xFFFFFFFFFFFFFFFFULL;
    }
    if (memcmp(head, "fLaC", 4) != 0) return 0xFFFFFFFFFFFFFFFFULL;
    return pos;
}

#endif  // HAVE_LIBFLAC

AudioDecoder_Flac::AudioDecoder_Flac()
{
#ifndef HAVE_LIBFLAC
    throw UnsupportedFeatureException("AudioDecoder_Flac");
#else
    state = nullptr;
    ff = nullptr;
    position = 0;
    srcBits = 0;
    memset(&info, 0, sizeof(AudioInfo));
#endif
}

AudioDecoder_Flac::~AudioDecoder_Flac()
{
#ifdef HAVE_LIBFLAC
    if (state != nullptr) {
        if (state->dec != nullptr) FLAC__stream_decoder_finish(state->dec);
        delete state;
    }
#endif
}

const AudioInfo& AudioDecoder_Flac::getAudioInfo() const
{
    return info;
}

size_t AudioDecoder_Flac::getPosition() const
{
    return position;
}

#ifdef HAVE_LIBFLAC

void AudioDecoder_Flac::open(FileObject& file, const AudioInfo* given)
{
    if (ff != nullptr) close();
    AudioInfo local;
    const AudioInfo* src = given;
    if (src == nullptr) {
        if (!IdentAudioFile(file, local)) throw UnsupportedAudioFormatException();
        src = &local;
    }

    if (src->Format != AudioInfo::AudioFormat::FLAC) throw UnsupportedAudioFormatException();
    if (src->Channels == 0 || src->Channels > 2) throw UnsupportedAudioFormatException();
    if (src->BitsPerSample != 16 && src->BitsPerSample != 24) {
        throw UnsupportedAudioFormatException();
    }
    // FLAC kennt nur vorzeichenbehaftete Ganzzahlen; ein AudioInfo mit FLOAT ist abgelehnt
    if (src->SampleType != AudioInfo::SampleEncoding::INTEGER) throw UnsupportedAudioFormatException();

    const uint64_t base = FlacStreamStart(file);
    if (base == 0xFFFFFFFFFFFFFFFFULL) throw DecoderException("AudioDecoder_Flac: FLAC-Datenanfang nicht gefunden");
    // libFLAC liest auch das "fLaC"-Magic selbst: Der Dateizeiger muss genau dort stehen
    if (file.seek(base, FileObject::SeekOrigin::SEEKSET) != base) {
        throw DecoderException("AudioDecoder_Flac: FLAC-Datenanfang nicht gefunden");
    }

    FlacState* st = new FlacState();
    st->file = &file;
    st->base = base;
    st->channels = src->Channels;

    FlacDecoderHandle handle;
    if (handle.get() == nullptr) {
        delete st;
        throw DecoderInitializationException("AudioDecoder_Flac");
    }
    FLAC__StreamDecoder* dec = handle.get();
    FLAC__stream_decoder_set_md5_checking(dec, false);
    FLAC__stream_decoder_set_metadata_ignore_all(dec);
    FLAC__stream_decoder_set_metadata_respond(dec, FLAC__METADATA_TYPE_STREAMINFO);

    const FLAC__StreamDecoderInitStatus init = FLAC__stream_decoder_init_stream(
        dec, flacReadCb, flacSeekCb, flacTellCb, flacLengthCb, flacEofCb,
        flacWriteCb, flacMetadataCb, flacErrorCb, st);
    if (init != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        delete st;
        throw DecoderException("AudioDecoder_Flac: libFLAC-Initialisierung fehlgeschlagen");
    }
    // Der libFLAC-Handle liegt jetzt im Objekt: erst nach erfolgreichem init zuweisen
    state = st;
    state->dec = dec;
    handle.release();

    if (!FLAC__stream_decoder_process_until_end_of_metadata(dec)) {
        FLAC__stream_decoder_finish(dec);
        delete state;
        state = nullptr;
        throw DecoderException("AudioDecoder_Flac: Metadaten unvollstaendig");
    }
    // Stimmen die vom Sniffer gemeldeten Werte nicht mit dem STREAMINFO-Block ueberein,
    // wurde eine fremde Datei mit korrektem AudioInfo geoeffnet.
    if (!state->haveMeta) {
        FLAC__stream_decoder_finish(dec);
        delete state;
        state = nullptr;
        throw DecoderException("AudioDecoder_Flac: STREAMINFO fehlt");
    }
    if (state->metaChannels != src->Channels || state->metaBits != src->BitsPerSample) {
        FLAC__stream_decoder_finish(dec);
        delete state;
        state = nullptr;
        throw UnsupportedAudioFormatException();
    }

    ff = &file;
    info = *src;
    srcBits = (uint8_t)src->BitsPerSample;
    position = 0;
    // Reservierung passend zum groessten Rahmen, damit der Write-Callback nicht wachsen muss
    state->raw.reserve((state->metaMaxBlocksize > 0 ? state->metaMaxBlocksize : 4096u) * 2);
}

void AudioDecoder_Flac::close()
{
    if (ff == nullptr) return;
    if (state != nullptr) {
        if (state->dec != nullptr) FLAC__stream_decoder_finish(state->dec);
        delete state;
        state = nullptr;
    }
    ff = nullptr;
    position = 0;
    srcBits = 0;
    memset(&info, 0, sizeof(AudioInfo));
}

void AudioDecoder_Flac::seekSample(size_t sample)
{
    if (ff == nullptr || state == nullptr) return;
    // libFLAC kann nur springen, wenn die Samplezahl feststeht; sonst wird neu
    //initialisiert und von vorn bis zum Ziel gelesen.
    if (info.Samples > 0 && sample >= (size_t)info.Samples) sample = (size_t)info.Samples - 1;
    if (sample == position) return;

    state->raw.clear();
    state->rawRead = 0;
    state->rawWrite = 0;
    state->atEnd = false;
    state->error = false;
    state->seekTarget = kNoSeek;

    if (info.Samples > 0) {
        state->seekTarget = sample;
        if (FLAC__stream_decoder_seek_absolute(state->dec, (FLAC__uint64)sample)) {
            position = sample;
            return;
        }
        // Sprung fehlgeschlagen: Decoder in einen definierten Zustand bringen
        state->seekTarget = kNoSeek;
        FLAC__stream_decoder_reset(state->dec);
        state->raw.clear();
        state->rawRead = 0;
        state->rawWrite = 0;
        state->atEnd = false;
        state->error = false;
    }

    // Neustart am Datenanfang und sequenziell bis zum Ziel vorlesen
    FLAC__stream_decoder_reset(state->dec);
    if (!FLAC__stream_decoder_process_until_end_of_metadata(state->dec)) {
        state->error = true;
        return;
    }
    position = 0;
    while (position < sample) {
        const size_t avail = fillFrames(sample - position);
        if (avail == 0) return;  // Dateiende: Position bleibt stehen
        const size_t need = sample - position;
        const size_t skip = avail < need ? avail : need;
        state->rawRead += skip * 2;
        position += skip;
    }
}

/** @brief Decodiert so lange weiter, bis \p num Frames im Rohpuffer bereitstehen.
 *
 *  @return Anzahl bereitstehender Frames; 0, wenn das Ende des Samples erreicht ist.
 */
size_t AudioDecoder_Flac::fillFrames(size_t num)
{
    size_t avail = (state->rawWrite - state->rawRead) / 2;
    while (avail < num) {
        if (state->atEnd || state->error) break;
        FLAC__stream_decoder_process_single(state->dec);
        // Am Streamende liefert process_single() wahr zurueck und laesst den Zustand
        // END_OF_STREAM stehen. Der Rueckgabewert kann die Schleife also nicht beenden,
        // sonst laeuft sie ohne Fortschritt weiter.
        switch (FLAC__stream_decoder_get_state(state->dec)) {
            case FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC:
            case FLAC__STREAM_DECODER_READ_FRAME:
                break;   // normaler Betrieb, es kann noch Rahmen bringen
            case FLAC__STREAM_DECODER_END_OF_STREAM:
            case FLAC__STREAM_DECODER_END_OF_LINK:
                state->atEnd = true;
                break;
            default:      // SEEK_ERROR, ABORTED, Speicherfehler, uninitialisiert
                state->error = true;
                break;
        }
        avail = (state->rawWrite - state->rawRead) / 2;
    }
    // Rohpuffer bei vollstaendigem Verbrauch zuruecksetzen, damit er nicht waechst
    if (state->rawRead == state->rawWrite) {
        state->rawRead = 0;
        state->rawWrite = 0;
        state->raw.clear();
    }
    return avail;
}

/** @brief Gemeinsamer Kern aller vier Lesemethoden.
 *
 *  Aus den Rohsamples im libFLAC-Wertebereich werden die Zielformate erzeugt:
 *  16 Bit unveraendert, 24 Bit rechtsbuendig auf 16 Bit gebracht (>>8); Float
 *  normiert der Bitbreite entsprechend.
 */
size_t AudioDecoder_Flac::transferFrames(size_t num, STEREOSAMPLE16* buf16, STEREOSAMPLE32* buf32,
                                         STEREOSAMPLE_FLOAT* bufFloat, bool add)
{
    if (ff == nullptr || state == nullptr) throw DecoderException("AudioDecoder_Flac: Decoder ist nicht geoeffnet");
    if (buf16 == nullptr && buf32 == nullptr && bufFloat == nullptr) throw DecoderException("AudioDecoder_Flac: buffer ist NULL");
    if (num == 0) return 0;

    // Ein FLAC-Rahmen liegt oft vollstaendig im Puffer; die Zahl bereiter Frames kann
    // ueber dem Wunsch liegen. Uebernommen wird nur, was der Aufrufer Platz hat.
    const size_t avail = fillFrames(num);
    if (avail == 0) {
        // Ein Bibliotheksfehler darf nicht wie Dateiende aussehen: ohne Fehlermeldung
        // bekame der Aufrufer stillschweigend zu wenige Samples zurueck.
        // Ein Sync-Verlust am physischen Dateiende ist eine abgeschnittene Datei,
        // kein Fehler: es wird geliefert, was da ist.
        // Ein Fehler erst mit dem Ende des Samples-Datensatzes ist eine abgeschnittene
        // Datei: es wird geliefert, was vollstaendig dekodierbar war.
        if (state->error && !state->atEnd) throw DecoderException("AudioDecoder_Flac: Decode-Fehler");
        return 0;
    }
    const size_t got = avail < num ? avail : num;

    const int32_t* in = state->raw.data() + state->rawRead;
    const bool shift24 = (srcBits == 24);
    const float scale = shift24 ? (1.0f / 8388608.0f) : (1.0f / 32768.0f);
    int32_t left, right;
    for (size_t i = 0; i < got; i++) {
        const int32_t l = in[0];
        const int32_t r = in[1];
        in += 2;
        left = l;
        right = r;
        if (shift24) {
            // Arithmetisches Schieben: vorzeichenbehaftete Werte bleiben vorzeichenerhaltend
            left >>= 8;
            right >>= 8;
        }
        if (buf16 != nullptr) {
            buf16[i].left = (SAMPLE16)left;
            buf16[i].right = (SAMPLE16)right;
        } else if (buf32 != nullptr) {
            if (add) {
                buf32[i].left += left;
                buf32[i].right += right;
            } else {
                buf32[i].left = left;
                buf32[i].right = right;
            }
        } else {
            // Float nutzt den vollen Rohwert, nicht den auf 16 Bit geschoebenen
            const float fleft = (float)l * scale;
            const float fright = (float)r * scale;
            if (add) {
                bufFloat[i].left += fleft;
                bufFloat[i].right += fright;
            } else {
                bufFloat[i].left = fleft;
                bufFloat[i].right = fright;
            }
        }
    }
    state->rawRead += got * 2;
    position += got;
    if (state->rawRead == state->rawWrite) {
        state->rawRead = 0;
        state->rawWrite = 0;
        state->raw.clear();
    }
    return got;
}

size_t AudioDecoder_Flac::getSamples(size_t num, STEREOSAMPLE16* buffer)
{
    return transferFrames(num, buffer, nullptr, nullptr, false);
}

size_t AudioDecoder_Flac::addSamples(size_t num, STEREOSAMPLE32* buffer)
{
    return transferFrames(num, nullptr, buffer, nullptr, true);
}

size_t AudioDecoder_Flac::getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer)
{
    return transferFrames(num, nullptr, nullptr, buffer, false);
}

size_t AudioDecoder_Flac::addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer)
{
    return transferFrames(num, nullptr, nullptr, buffer, true);
}

#else  // ohne libFLAC: alle Methoden werfen bzw. liefern nichts

void AudioDecoder_Flac::open(FileObject&, const AudioInfo*)
{
    throw UnsupportedFeatureException("AudioDecoder_Flac");
}

void AudioDecoder_Flac::close() {}

size_t AudioDecoder_Flac::fillFrames(size_t) { return 0; }

size_t AudioDecoder_Flac::transferFrames(size_t num, STEREOSAMPLE16*, STEREOSAMPLE32*, STEREOSAMPLE_FLOAT*, bool)
{
    throw DecoderException("AudioDecoder_Flac");
}

void AudioDecoder_Flac::seekSample(size_t) {}

size_t AudioDecoder_Flac::getSamples(size_t num, STEREOSAMPLE16*) { return 0; }
size_t AudioDecoder_Flac::addSamples(size_t num, STEREOSAMPLE32*) { return 0; }
size_t AudioDecoder_Flac::getSamples(size_t num, STEREOSAMPLE_FLOAT*) { return 0; }
size_t AudioDecoder_Flac::addSamples(size_t num, STEREOSAMPLE_FLOAT*) { return 0; }

#endif  // HAVE_LIBFLAC

}  // namespace pplib
