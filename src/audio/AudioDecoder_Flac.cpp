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

/** @brief Rueckgabewert von FlacStreamStart(), wenn kein FLAC-Datenanfang gefunden wurde */
static const uint64_t kNoStreamStart = 0xFFFFFFFFFFFFFFFFULL;

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
    uint64_t base; //!< Dateiversatz des "fLaC"-Magic: libFLAC zaehlt ab hier
    std::vector<int32_t> raw;
    size_t rawWrite; //!< Schreibzeiger in Kanal-Samples
    size_t rawRead;  //!< Leszeiger in Kanal-Samples
    uint32_t channels;
    bool atEnd;   //!< libFLAC meldete Ende des Samples-Datensatzes
    bool error;   //!< Decodier- oder Callbackfehler
    bool ioError; //!< Fehler im Dateizugriff eines Betriebs-Callbacks
    bool haveMeta;
    uint32_t metaChannels;
    uint32_t metaBits;
    uint32_t metaMaxBlocksize; //!< STREAMINFO max blocksize: bemisst die Reservierung des Rohpuffers
    uint64_t metaSamples;      //!< STREAMINFO Gesamtzahl Samples, 0 = unbekannt
    uint64_t totalFrames;      //!< bisher gelieferte Frames, auch ueber den Lesepuffer zurueck
    uint64_t seekTarget;       //!< Samplezahl, bis zu der gelieferte Frames verworfen werden (kNoSeek = keiner)

    FlacState()
        : dec(nullptr),
          file(nullptr),
          base(0),
          rawWrite(0),
          rawRead(0),
          channels(2),
          atEnd(false),
          error(false),
          ioError(false),
          haveMeta(false),
          metaChannels(0),
          metaBits(0),
          metaMaxBlocksize(0),
          metaSamples(0),
          totalFrames(0),
          seekTarget(kNoSeek)
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
    FlacDecoderHandle()
    {
        dec = FLAC__stream_decoder_new();
    }
    FlacDecoderHandle(const FlacDecoderHandle&) = delete;
    FlacDecoderHandle& operator=(const FlacDecoderHandle&) = delete;
    ~FlacDecoderHandle()
    {
        if (dec != nullptr) FLAC__stream_decoder_delete(dec);
    }
    FLAC__StreamDecoder* get()
    {
        return dec;
    }
    void release()
    {
        dec = nullptr;
    } //!< Uebergibt die Instanz an den Aufrufer

private:
    FLAC__StreamDecoder* dec;
};

/** @brief Raeumt einen libFLAC-Decoder vollstaendig aus dem Speicher.
 *
 * finish() zerstoert nur die Decodier-Interna nicht: ohne anschiessendes delete()
 * bleiben die internen Puffer der Instanz verloren.
 */
static void ReleaseFlacDecoder(FLAC__StreamDecoder* dec)
{
    if (dec == nullptr) return;
    FLAC__stream_decoder_finish(dec);
    FLAC__stream_decoder_delete(dec);
}

/** @brief Lese-Callback: libFLAC liest ab Dateiversatz `base`.
 *
 * Eine Exception durch die C-Frame-Ebenen von libFLAC waere nicht portabel definiert,
 * deshalb werden Dateiende und Lesefehler als Status gemeldet. Die Dateizugriffe
 * liegen aus demselben Grund vollstaendig im `try`-Block.
 */
static FLAC__StreamDecoderReadStatus flacReadCb(const FLAC__StreamDecoder*, FLAC__byte buffer[], size_t* bytes, void* client)
{
    FlacState* st = static_cast<FlacState*>(client);
    if (*bytes == 0) return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    try {
        // pplib meldet das Dateiende uber eine Exception, libFLAC erwartet einen Status.
        // Deshalb wird die Anfrage vorher auf das verbleibende Reststueck beschraenkt.
        const uint64_t pos = st->file->tell();
        const uint64_t total = st->file->size();
        if (pos >= total) {
            *bytes = 0;
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        }
        if (*bytes > (size_t)(total - pos)) *bytes = (size_t)(total - pos);
        const size_t got = st->file->read(buffer, *bytes);
        if (got == 0) {
            *bytes = 0;
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        }
        *bytes = got;
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }
    catch (...) {
        *bytes = 0;
        st->error = true;
        st->ioError = true;
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
}

static FLAC__StreamDecoderSeekStatus flacSeekCb(const FLAC__StreamDecoder*, FLAC__uint64 absolute_byte_offset, void* client)
{
    FlacState* st = static_cast<FlacState*>(client);
    try {
        const uint64_t target = st->base + absolute_byte_offset;
        if (target < st->base || target > st->file->size()) return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
        if (st->file->seek(target, FileObject::SeekOrigin::SEEKSET) != target) return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    }
    catch (...) {
        st->error = true;
        st->ioError = true;
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }
}

static FLAC__StreamDecoderTellStatus flacTellCb(const FLAC__StreamDecoder*, FLAC__uint64* absolute_byte_offset, void* client)
{
    FlacState* st = static_cast<FlacState*>(client);
    try {
        const uint64_t now = st->file->tell();
        if (now < st->base) return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
        *absolute_byte_offset = now - st->base;
        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
    }
    catch (...) {
        // libFLAC fragt hier keinen Fehlerwert ab: 0 zusammen mit gesetztem Fehlermerkmal
        // ist die einzige Moglichkeit, den Abbruch sichtbar zu machen
        *absolute_byte_offset = 0;
        st->error = true;
        st->ioError = true;
        return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
    }
}

static FLAC__StreamDecoderLengthStatus flacLengthCb(const FLAC__StreamDecoder*, FLAC__uint64* stream_length, void* client)
{
    FlacState* st = static_cast<FlacState*>(client);
    try {
        const uint64_t total = st->file->size();
        if (total < st->base) return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
        *stream_length = total - st->base;
        return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
    }
    catch (...) {
        *stream_length = 0;
        st->error = true;
        st->ioError = true;
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
    }
}

static FLAC__bool flacEofCb(const FLAC__StreamDecoder*, void* client)
{
    FlacState* st = static_cast<FlacState*>(client);
    try {
        return st->file->eof() ? true : false;
    }
    catch (...) {
        // "kein Dateiende" laesst libFLAC weiterlesen: der Lese-Callback meldet dann
        // den Abbruch und setzt dieselben Fehlermerkmale
        st->error = true;
        st->ioError = true;
        return false;
    }
}

/** @brief Write-Callback: Rahmen werden verskraenkt und im Rohpuffer gesammelt.
 *
 * Mono-Rahmen werden auf beide Kanale kopiert. Uber den reservierten Platz hinaus
 * wächst der Puffer bei Bedarf; schlägt das fehl, wird der Abbruch gemeldet und der
 * Fehlerzustand gesetzt - eine Exception darf die C-Ebenen nicht durchlaufen.
 */
static FLAC__StreamDecoderWriteStatus flacWriteCb(const FLAC__StreamDecoder*,
                                                  const FLAC__Frame* frame,
                                                  const int32_t* const buffer[],
                                                  void* client)
{
    FlacState* st = static_cast<FlacState*>(client);
    size_t frames = frame->header.blocksize;
    if (frame->header.channels != st->channels) {
        st->error = true; // Kanalzahl wechselt mitten im Stream: nicht unterstuetzt
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    if (frames == 0 || buffer[0] == nullptr || (st->channels == 2 && buffer[1] == nullptr)) {
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    }
    // Nach seekSample() liefert libFLAC auch Rahmen, die vor dem Ziel liegen (einschliesslich
    /// der beim Suchen intern gelesenen). Alles vor dem Ziel wird hier verworfen.
    size_t start = 0;
    if (st->seekTarget != kNoSeek) {
        if (frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER) {
            const uint64_t first = frame->header.number.sample_number;
            if (first + frames <= st->seekTarget) return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
            if (first < st->seekTarget) start = (size_t)(st->seekTarget - first);
        }
        // Rahmen-nummerierte Stroeme tragen keine Samplezahl im Kopf, ihre Lage laesst sich
        // hier nicht bestimmen. Noetig ist das nicht: ein Sprung per seek_absolute() setzt
        // eine bekannte Samplezahl voraus, sonst uebernimmt der lineare Pfad in seekSample()
        // das Weglassen der uebersprungenen Frames.
        st->seekTarget = kNoSeek;
    }
    frames -= start;
    try {
        st->raw.resize(st->rawWrite + frames * 2);
    }
    catch (...) {
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
    if (status == FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC || status == FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM) {
        if (st->metaSamples > 0 && st->totalFrames >= st->metaSamples) return;
    }
    st->error = true;
}

/** @brief Sucht den Dateiversatz des "fLaC"-Magic, auch hinter einem ID3v2-Tag.
 *
 *  @return Position der Magie, oder kNoStreamStart, falls sie nicht gefunden wird.
 */
static uint64_t FlacStreamStart(FileObject& file)
{
    const uint64_t fsize = file.size();
    // 4 Byte Magie + 4 Byte Blockkopf + 34 Byte STREAMINFO + mindestens 1 Byte Rahmen;
    // genau diese Untergrenze setzt auch die Dateierkennung in AudioInfo.cpp
    if (fsize < 43) return kNoStreamStart;
    char head[10];
    if (file.seek(0, FileObject::SeekOrigin::SEEKSET) != 0) return kNoStreamStart;
    if (file.read(head, 4) != 4) return kNoStreamStart;
    uint64_t pos = 0;
    if (memcmp(head, "ID3", 3) == 0) {
        // Kappengroesse als "synchsafe number": je 7 Bit aus vier Bytes. Dieselbe Rechnung
        // steht fuer die Dateierkennung in ID3v2TagSize() (AudioInfo.cpp); dort liegt sie als
        // interne Funktion einer Uebersetzungseinheit und ist von hier nicht aufrufbar. Beide
        // Fassungen muessen bei Aenderungen zusammen angepasst werden.
        char tag[10];
        memcpy(tag, head, 4);
        if (file.read(tag + 4, 6) != 6) return kNoStreamStart;
        if ((uint8_t)tag[3] < 2) return kNoStreamStart;
        const uint32_t size =
            ((uint32_t)(uint8_t)tag[6] << 21) | ((uint32_t)(uint8_t)tag[7] << 14) | ((uint32_t)(uint8_t)tag[8] << 7) | (uint8_t)tag[9];
        pos = 10 + size;
        if (pos + 4 > fsize) return kNoStreamStart;
        if (file.seek(pos, FileObject::SeekOrigin::SEEKSET) != pos) return kNoStreamStart;
        if (file.read(head, 4) != 4) return kNoStreamStart;
    }
    if (memcmp(head, "fLaC", 4) != 0) return kNoStreamStart;
    return pos;
}

#endif // HAVE_LIBFLAC

// Der Konstruktor wirft nie, auch nicht ohne libFLAC: das Objekt ist dann guiltig, aber leer.
// Erst die Methoden selbst melden eine fehlende Bibliothek mit UnsupportedFeatureException.
AudioDecoder_Flac::AudioDecoder_Flac()
{
    state = nullptr;
    ff = nullptr;
    position = 0;
    encoding = FrameEncoding::NONE;
}

// Der Destruktor wirft niemals: ohne libFLAC ist state immer NULL, der Rumpf ist wegdefiniert
AudioDecoder_Flac::~AudioDecoder_Flac()
{
#ifdef HAVE_LIBFLAC
    if (state != nullptr) {
        ReleaseFlacDecoder(state->dec);
        state->dec = nullptr;
        delete state;
    }
#endif
}

const AudioInfo& AudioDecoder_Flac::getAudioInfo() const
{
    return info;
}

size_t AudioDecoder_Flac::getPosition() const noexcept
{
    return position;
}

void AudioDecoder_Flac::open(FileObject& file, const AudioInfo* given)
{
#ifndef HAVE_LIBFLAC
    throw UnsupportedFeatureException("AudioDecoder_Flac");
#else
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
    if (base == kNoStreamStart) throw DecoderException("AudioDecoder_Flac: FLAC-Datenanfang nicht gefunden");
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
        // Reine Verteidigung: FLAC__stream_decoder_new() scheitert praktisch nur bei
        // Speichernot. Ueber die oeffentliche Schnittstelle ist dieser Zweig nicht erreichbar.
        delete st;
        throw DecoderInitializationException("AudioDecoder_Flac");
    }
    FLAC__StreamDecoder* dec = handle.get();
    // Pruefsumme am Dateiende bewusst abgeschaltet: Bei stueckweisem Lesen mit toleriertem
    // Dateiende (abgeschnittene Datei, Anhaengsel wie ID3v1) kann libFLAC die MD5-Berechnung
    // nie gueltig abschliessen; ihre Pruefung wuerde intakte Dateien als defekt melden.
    FLAC__stream_decoder_set_md5_checking(dec, false);
    FLAC__stream_decoder_set_metadata_ignore_all(dec);
    FLAC__stream_decoder_set_metadata_respond(dec, FLAC__METADATA_TYPE_STREAMINFO);

    const FLAC__StreamDecoderInitStatus init = FLAC__stream_decoder_init_stream(dec, flacReadCb, flacSeekCb, flacTellCb, flacLengthCb,
                                                                                flacEofCb, flacWriteCb, flacMetadataCb, flacErrorCb, st);
    if (init != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        delete st;
        throw DecoderException("AudioDecoder_Flac: libFLAC-Initialisierung fehlgeschlagen");
    }
    // Der libFLAC-Handle liegt jetzt im Objekt: erst nach erfolgreichem init zuweisen
    state = st;
    state->dec = dec;
    handle.release();

    if (!FLAC__stream_decoder_process_until_end_of_metadata(dec)) {
        ReleaseFlacDecoder(dec);
        delete state;
        state = nullptr;
        throw DecoderException("AudioDecoder_Flac: Metadaten unvollstaendig");
    }
    // Reine Verteidigung: eine Datei ohne STREAMINFO lehnt schon die Dateierkennung in
    // AudioInfo.cpp ab. Erreichbar ist der Zweig nur fuer einen Aufrufer, der ein eigenes
    // AudioInfo mitgebracht hat.
    if (!state->haveMeta) {
        ReleaseFlacDecoder(dec);
        delete state;
        state = nullptr;
        throw DecoderException("AudioDecoder_Flac: STREAMINFO fehlt");
    }
    // Stimmen die gemeldeten Werte nicht mit dem STREAMINFO-Block ueberein, wurde eine
    // fremde Datei mit passendem AudioInfo geoeffnet.
    if (state->metaChannels != src->Channels || state->metaBits != src->BitsPerSample) {
        ReleaseFlacDecoder(dec);
        delete state;
        state = nullptr;
        throw UnsupportedAudioFormatException();
    }

    ff = &file;
    info = *src;
    // open() hat Bitbreiten ausser 16 und 24 schon abgelehnt, die Entscheidung faellt also
    // einmal und nicht in jeder innersten Schleife
    encoding = (src->BitsPerSample == 24) ? FrameEncoding::INT24 : FrameEncoding::INT16;
    position = 0;
    // Reservierung passend zum groessten Rahmen, damit der Write-Callback nicht wachsen muss
    state->raw.reserve((state->metaMaxBlocksize > 0 ? state->metaMaxBlocksize : 4096u) * 2);
#endif
}

void AudioDecoder_Flac::close()
{
#ifndef HAVE_LIBFLAC
    throw UnsupportedFeatureException("AudioDecoder_Flac");
#else
    if (ff == nullptr) return;
    if (state != nullptr) {
        ReleaseFlacDecoder(state->dec);
        state->dec = nullptr;
        delete state;
        state = nullptr;
    }
    ff = nullptr;
    position = 0;
    encoding = FrameEncoding::NONE;
    info = AudioInfo();
#endif
}

void AudioDecoder_Flac::seekSample(size_t sample)
{
#ifndef HAVE_LIBFLAC
    throw UnsupportedFeatureException("AudioDecoder_Flac");
#else
    if (ff == nullptr || state == nullptr) return;
    // libFLAC kann nur springen, wenn die Samplezahl feststeht: dann bleibt der Strom am
    // Zielrahmen stehen. Sonst wird der Decoder neu initialisiert und bis zum Ziel
    // durchgearbeitet; dieser sequenzielle Pfad setzt samtlichen Zustand zurueck.
    if (info.Samples > 0 && sample >= (size_t)info.Samples) sample = (size_t)info.Samples - 1;
    if (sample == position) return;

    state->raw.clear();
    state->rawRead = 0;
    state->rawWrite = 0;
    state->atEnd = false;
    state->error = false;
    // Ein neuer Versuch beginnt wieder sauber: bliebe ioError stehen, koennte
    // fillFrames() sein Ende-Flag nie mehr setzen und die Nachleseschleife faende
    // nie an, weil weder atEnd noch error gesetzt wird
    state->ioError = false;
    state->seekTarget = kNoSeek;
    state->totalFrames = 0; // Unterdückungsregel in flacErrorCb() wieder scharf stellen

    if (info.Samples > 0) {
        state->seekTarget = sample;
        if (FLAC__stream_decoder_seek_absolute(state->dec, (FLAC__uint64)sample)) {
            position = sample;
            return;
        }
        // Sprung fehlgeschlagen: der Decoder wird erst vor dem sequenziellen Pfad
        // resett; hier nur den Zustand des Decoders entwerten
        state->seekTarget = kNoSeek;
        state->raw.clear();
        state->rawRead = 0;
        state->rawWrite = 0;
        state->atEnd = false;
        state->error = false;
        state->ioError = false;
        state->totalFrames = 0;
    }

    // Neustart am Datenanfang und sequenziell bis zum Ziel vorlesen
    FLAC__stream_decoder_reset(state->dec);
    if (!FLAC__stream_decoder_process_until_end_of_metadata(state->dec)) {
        state->error = true;
        return;
    }
    position = 0;
    while (position < sample) {
        // In Portionen vorlesen: eine Anfrage ueber die ganze Sprungdistanz liesse den
        // Rohpuffer linear mitwachsen (ca. 1,27 GB pro Materialstunde 16 Bit/Stereo).
        // Nach jeder Portion ist der Puffer verbraucht und wird in fillFrames() zurueckgesetzt.
        const size_t chunk = (state->metaMaxBlocksize > 0) ? (size_t)state->metaMaxBlocksize : 4096u;
        const size_t distance = sample - position;
        const size_t avail = fillFrames(distance < chunk ? distance : chunk);
        if (avail == 0) return; // Dateiende: Position bleibt stehen
        const size_t need = sample - position;
        const size_t skip = avail < need ? avail : need;
        state->rawRead += skip * 2;
        position += skip;
    }
#endif
}

/** @brief Decodiert so lange weiter, bis \p num Frames im Rohpuffer bereitstehen.
 *
 *  Die Dateiposition des Dekoderstroms wird dabei nicht nachgefuehrt: was der Aufrufer
 *  schon uebernommen hat, ist nur im Rohpuffer verbraucht, libFLAC steht weiter am Ende
 *  des zuletzt gelieferten Rahmens.
 *  @return Anzahl bereitstehender Frames; 0, wenn das Ende des Samples erreicht ist.
 */
size_t AudioDecoder_Flac::fillFrames(size_t num)
{
#ifndef HAVE_LIBFLAC
    throw UnsupportedFeatureException("AudioDecoder_Flac");
#else
    // Vollstaendig verbraichten Puffer vor dem Nachlesen zuruecksetzen, damit die
    // Schreibposition nicht mit der Zahl der bisher gelesenen Frames waechst.
    // Ein Aufrufer, der alles uebernommen hat, hinterliess rawRead == rawWrite.
    if (state->rawRead == state->rawWrite) {
        state->rawRead = 0;
        state->rawWrite = 0;
        state->raw.clear(); // Kapazitaet bleibt erhalten, der Speicher wird wiedergenutzt
    }
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
            break; // normaler Betrieb, es kann noch Rahmen bringen
        case FLAC__STREAM_DECODER_END_OF_STREAM:
        case FLAC__STREAM_DECODER_END_OF_LINK:
            // Ein Abbruch im Dateizugriff laesst libFLAC ebenso "Streamende" melden
            // wie eine abgeschnittene Datei. Nur der Dateifehler unterscheidet beides.
            if (!state->ioError) state->atEnd = true;
            break;
        default: // SEEK_ERROR, ABORTED, Speicherfehler, uninitialisiert
            state->error = true;
            break;
        }
        avail = (state->rawWrite - state->rawRead) / 2;
    }
    return avail;
#endif
}

namespace
{

/** @brief Vollskala der unterstuetzten Quellformate (2^15, 2^23). Jede Bitbreite wird auf
 *         ihre eigene Vollskala normiert, damit beide im Float-Puffer den Bereich
 *         -1,0 bis 1,0 fuellen.
 */
constexpr float FULL_SCALE_16 = 32768.0f;
constexpr float FULL_SCALE_24 = 8388608.0f;

} // namespace

/** @brief bereitet den naechsten Lesevorgang vor
 *
 *  Prueft die Voraussetzungen und stellt bis zu \p num Frames im Rohpuffer bereit. Ein
 *  Rueckgabewert von 0 bedeutet "nichts zu lesen". Die Pruefungen stehen hier und nicht in
 *  den vier Lesemethoden, weil sie fuer alle Zielformate voellig gleich sind.
 */
size_t AudioDecoder_Flac::prepareRead(size_t num, const void* buffer) noexcept
{
#ifndef HAVE_LIBFLAC
    return 0;
#else
    if (ff == nullptr || state == nullptr || buffer == nullptr || num == 0) return 0;

    // Ein FLAC-Rahmen liegt oft vollstaendig im Puffer; die Zahl bereiter Frames kann
    // ueber dem Wunsch liegen. Uebernommen wird nur, was der Aufrufer Platz hat.
    const size_t avail = fillFrames(num);
    if (avail == 0) return 0;
    return std::min(avail, num);
#endif
}

/** @brief zaehlt \p frames uebernommene Frames weiter und raeumt den Rohpuffer. */
void AudioDecoder_Flac::claimFrames(size_t frames)
{
#ifndef HAVE_LIBFLAC
    throw UnsupportedFeatureException("AudioDecoder_Flac");
#else
    state->rawRead += frames * 2; // zwei Kanal-Samples je Frame
    position += frames;
    if (state->rawRead == state->rawWrite) {
        // Nach dem letzten uebernommenen Frame zuruecksetzen, damit die Zeiger nicht mit
        // der Zahl der bisher gelesenen Frames wachsen; die Kapazitaet bleibt erhalten
        state->rawRead = 0;
        state->rawWrite = 0;
        state->raw.clear();
    }
#endif
}

/** @brief liefert 16-Bit-Samples: Quelle 16 Bit unveraendert, Quelle 24 Bit auf die
 *         16-Bit-Ebene geschoben.
 */
size_t AudioDecoder_Flac::getSamples(size_t num, STEREOSAMPLE16* buffer) noexcept
{
#ifndef HAVE_LIBFLAC
    return 0;
#else
    const size_t got = prepareRead(num, buffer);
    if (got == 0) return 0;

    const int32_t* in = state->raw.data() + state->rawRead;
    switch (encoding) {
    case FrameEncoding::INT16:
        for (size_t i = 0; i < got; i++) {
            buffer[i].left = (SAMPLE16)in[0];
            buffer[i].right = (SAMPLE16)in[1];
            in += 2;
        }
        break;
    case FrameEncoding::INT24:
        // Arithmetisches Schieben: vorzeichenbehaftete Werte bleiben vorzeichenerhaltend
        for (size_t i = 0; i < got; i++) {
            buffer[i].left = (SAMPLE16)(in[0] >> 8);
            buffer[i].right = (SAMPLE16)(in[1] >> 8);
            in += 2;
        }
        break;
    default:
        // FrameEncoding::NONE laesst open() gar nicht erst zu den Lesemethoden kommen
        break;
    }
    claimFrames(got);
    return got;
#endif
}

/** @brief mixt Samples in den 32-Bit-Mixpuffer: Werte bleiben durchgehend auf der
 *         16-Bit-Ebene, unabhaengig von der Bitbreite der Quelle.
 */
size_t AudioDecoder_Flac::addSamples(size_t num, STEREOSAMPLE32* buffer) noexcept
{
#ifndef HAVE_LIBFLAC
    throw UnsupportedFeatureException("AudioDecoder_Flac");
#else
    const size_t got = prepareRead(num, buffer);
    if (got == 0) return 0;

    const int32_t* in = state->raw.data() + state->rawRead;
    switch (encoding) {
    case FrameEncoding::INT16:
        for (size_t i = 0; i < got; i++) {
            buffer[i].left += in[0];
            buffer[i].right += in[1];
            in += 2;
        }
        break;
    case FrameEncoding::INT24:
        for (size_t i = 0; i < got; i++) {
            buffer[i].left += in[0] >> 8;
            buffer[i].right += in[1] >> 8;
            in += 2;
        }
        break;
    default:
        break;
    }
    claimFrames(got);
    return got;
#endif
}

/** @brief liefert normierte Float-Samples: jede Quellbitbreite wird auf ihre eigene
 *         Vollskala geteilt.
 */
size_t AudioDecoder_Flac::getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept
{
#ifndef HAVE_LIBFLAC
    throw UnsupportedFeatureException("AudioDecoder_Flac");
#else
    const size_t got = prepareRead(num, buffer);
    if (got == 0) return 0;

    const int32_t* in = state->raw.data() + state->rawRead;
    switch (encoding) {
    case FrameEncoding::INT16:
        for (size_t i = 0; i < got; i++) {
            buffer[i].left = (float)in[0] / FULL_SCALE_16;
            buffer[i].right = (float)in[1] / FULL_SCALE_16;
            in += 2;
        }
        break;
    case FrameEncoding::INT24:
        for (size_t i = 0; i < got; i++) {
            buffer[i].left = (float)in[0] / FULL_SCALE_24;
            buffer[i].right = (float)in[1] / FULL_SCALE_24;
            in += 2;
        }
        break;
    default:
        break;
    }
    claimFrames(got);
    return got;
#endif
}

/** @brief mixt normierte Float-Samples in den Float-Puffer. */
size_t AudioDecoder_Flac::addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept
{
#ifndef HAVE_LIBFLAC
    throw UnsupportedFeatureException("AudioDecoder_Flac");
#else
    const size_t got = prepareRead(num, buffer);
    if (got == 0) return 0;

    const int32_t* in = state->raw.data() + state->rawRead;
    switch (encoding) {
    case FrameEncoding::INT16:
        for (size_t i = 0; i < got; i++) {
            buffer[i].left += (float)in[0] / FULL_SCALE_16;
            buffer[i].right += (float)in[1] / FULL_SCALE_16;
            in += 2;
        }
        break;
    case FrameEncoding::INT24:
        for (size_t i = 0; i < got; i++) {
            buffer[i].left += (float)in[0] / FULL_SCALE_24;
            buffer[i].right += (float)in[1] / FULL_SCALE_24;
            in += 2;
        }
        break;
    default:
        break;
    }
    claimFrames(got);
    return got;
#endif
}

} // namespace pplib
