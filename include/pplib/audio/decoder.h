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

#ifndef PPLIB_AUDIO_DECODER_H
#define PPLIB_AUDIO_DECODER_H

#include <cstdint>
#include <pplib/core/baseexception.h>
#include <pplib/core/fileobject.h>
#include <pplib/audio/sample_formats.h>
#include <pplib/audio/audioinfo.h>

namespace pplib
{

PPLIBEXCEPTION(DecoderException, Exception);
PPLIBEXCEPTION(DecoderInitializationException, DecoderException);

/** @class AudioDecoder
 * @ingroup PPLGroupSound
 * @brief Abstrakte Basisklasse für Audio-Decoder.
 *
 * Diese Klasse definiert die Schnittstelle für Audio-Decoder. Alle spezifischen Audio-Decoder
 * (z.B. für WAV, MP3, OGG) müssen von dieser Klasse abgeleitet werden und die reinen virtuellen
 * Funktionen implementieren.
 *
 * Mit dem AudioDecoder können verschiedene Audio-Formate einheitlich dekodiert werden. Es muss jedoch
 * berücksichtigt werden, dass kein Upsampling, Downsampling oder automatische Formatkonvertierung erfolgt.
 * Der Decoder liefert die Samples in der nativen Frequenz des Audio-Files.
 *
 * Auch kann es sein, dass nicht alle Formate unterstützt werden.
 */
class AudioDecoder
{
public:
    /** @brief Standard-Konstruktor. */
    AudioDecoder() = default;

    AudioDecoder(const AudioDecoder&) = delete;
    AudioDecoder(AudioDecoder&&) = delete;
    AudioDecoder& operator=(const AudioDecoder&) = delete;
    AudioDecoder& operator=(AudioDecoder&&) = delete;

    /** @brief Destruktor der AudioDecoder-Klasse.
     *
     * Gibt alle vom Decoder belegten Ressourcen frei.
     */
    virtual ~AudioDecoder() {};

    /** @brief Öffnet die gegebene Datei und initialisiert den Decoder.
     *
     * Diese Methode öffnet die gegebene Datei und initialisiert den Decoder.
     *
     * @param file Die zu öffnende Datei.
     * @param info Optional: Falls zuvor bereits Informationen über das Audio vorliegen (zum Beispiel nach Aufruf von `IdentAudioFile()`),
     * können diese hier übergeben werden. Die Funktion muss diese Informationen dann nicht noch einmal auslesen.
     *
     * @exception DecoderInitializationException Wenn der Decoder nicht initialisiert werden kann.
     * @exception UnsupportedAudioFormatException Wenn das Audio-Format der Datei nicht unterstützt wird.
     * @note Andere Exceptions können je nach Implementierung des spezifischen Decoders auftreten.
     */
    virtual void open(FileObject& file, const AudioInfo* info = NULL) = 0;

    /** @brief Schließt den Decoder und gibt alle Ressourcen frei.
     *
     */
    virtual void close() = 0;

    /** @brief Gibt Informationen über das Audio zurück.
     *
     * Gibt Informationen über das geöffnete Audio-File zurück. Wird die Methode vor `open()` oder nach `close()` aufgerufen,
     * ist das Verhalten undefiniert.
     *
     * @return Referenz auf die AudioInfo-Struktur.
     */
    virtual const AudioInfo& getAudioInfo() const = 0;

    /** @brief Setzt die aktuelle Position auf das gegebene Sample.
     * @param sample Die Sample-Position, zu der gesprungen werden soll.
     */
    virtual void seekSample(size_t sample) = 0;

    /** @brief Gibt die aktuelle Sample-Position zurück.
     *
     * @return Die aktuelle Sample-Position im Audio-Stream.
     */
    virtual size_t getPosition() const = 0;

    /** @brief Liest Samples in einen Puffer im Format STEREOSAMPLE16 ein.
     *
     * Liest \p num Audio-Samples aus dem Audio-Stream und schreibt sie in den gegebenen Puffer \p buffer.
     * Der Puffer ist hier im Format STEREOSAMPLE16. Dabei entspricht ein Sample jeweils einem 16-Bit-Wert
     * für links und einen 16-Bit-Wert für rechts. Der Puffer muss entsprechend groß sein, um alle Samples aufzunehmen.
     *
     * @param num Die Anzahl der zu lesenden Samples.
     * @param buffer Zeiger auf den Puffer, in den die Samples geschrieben werden sollen.
     * @return Die tatsächlich gelesene Anzahl an Samples.
     */
    virtual size_t getSamples(size_t num, STEREOSAMPLE16* buffer) = 0;

    /** @brief Addiert Audio-Samples aus dem File zum gegebenen Puffer.
     *
     * Liest \p num Audio-Samples aus dem Audio-Stream und addiert diese zum gegebenen Puffer \p buffer.
     *
     * Der Puffer ist hier im Format STEREOSAMPLE32. Dabei entspricht ein Sample jeweils einem 32-Bit-Wert
     * für links und einen 32-Bit-Wert für rechts. Der Puffer muss entsprechend groß sein, um alle Samples aufzunehmen.
     *
     * Die Funktion kann verwendet werden, wenn zum Beispiel mehrere Audio-Streams gemischt werden sollen.
     *
     * @param num Die Anzahl der zu addierenden Samples.
     * @param buffer Zeiger auf den Puffer, zu dem die Samples addiert werden sollen.
     * @return Die tatsächlich addierte Anzahl an Samples.
     */
    virtual size_t addSamples(size_t num, STEREOSAMPLE32* buffer) = 0;

    /** @brief Liest Samples in einen Puffer im Format STEREOSAMPLE_FLOAT ein.
     *
     * Liest \p num Audio-Samples aus dem Audio-Stream und schreibt sie in den gegebenen Puffer \p buffer.
     * Der Puffer ist hier im Format STEREOSAMPLE_FLOAT. Dabei entspricht ein Sample jeweils einem 32-Bit-Float-Wert
     * für links und einen 32-Bit-Float-Wert für rechts. Der Puffer muss entsprechend groß sein, um alle Samples aufzunehmen.
     *
     * @param num Die Anzahl der zu lesenden Samples.
     * @param buffer Zeiger auf den Puffer, in den die Samples geschrieben werden sollen.
     * @return Die tatsächlich gelesene Anzahl an Samples.
     */
    virtual size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) = 0;

    /** @brief Addiert Samples zum gegebenen Puffer im Format STEREOSAMPLE_FLOAT.
     *
     * Liest \p num Audio-Samples aus dem Audio-Stream und addiert diese zum gegebenen Puffer \p buffer.
     *
     * Der Puffer ist hier im Format STEREOSAMPLE_FLOAT. Dabei entspricht ein Sample jeweils einem 32-Bit-Float-Wert
     * für links und einen 32-Bit-Float-Wert für rechts. Der Puffer muss entsprechend groß sein, um alle Samples aufzunehmen.
     *
     * Die Funktion kann verwendet werden, wenn zum Beispiel mehrere Audio-Streams gemischt werden sollen.
     *
     * @param num Die Anzahl der zu addierenden Samples.
     * @param buffer Zeiger auf den Puffer, zu dem die Samples addiert werden sollen.
     * @return Die tatsächlich addierte Anzahl an Samples.
     */
    virtual size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) = 0;
};

/** @ingroup PPLGroupSound
 * @brief Erstellt einen passenden Audio-Decoder für die gegebene Datei.
 *
 * Diese Funktion analysiert die gegebene Datei und erstellt einen passenden Audio-Decoder basierend auf dem Dateiformat.
 * Dieser wird als Zeiger auf ein AudioDecoder-Objekt zurückgegeben. Der Aufrufer ist für die Freigabe des Speichers verantwortlich
 * (delete).
 *
 * @param file Die Datei, für die ein Audio-Decoder erstellt werden soll.
 * @return Zeiger auf ein AudioDecoder-Objekt, das für die gegebene Datei geeignet ist, oder NULL,
 * falls kein passender Decoder gefunden wurde.
 */
AudioDecoder* GetAudioDecoder(FileObject& file);

class AudioDecoder_Wave : public AudioDecoder
{
private:
    FileObject* ff;
    AudioInfo info;
    size_t position;
    size_t samplesize;

public:
    AudioDecoder_Wave();
    ~AudioDecoder_Wave() override;
    void open(FileObject& file, const AudioInfo* info = NULL) override;
    void close() override;
    const AudioInfo& getAudioInfo() const override;
    void seekSample(size_t sample) override;
    size_t getPosition() const override;
    size_t getSamples(size_t num, STEREOSAMPLE16* buffer) override;
    size_t addSamples(size_t num, STEREOSAMPLE32* buffer) override;
    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;
    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;
};

/** @class AudioDecoder_Aiff
 * @ingroup PPLGroupSound
 * @brief Decoder für unkomprimierte AIFF- und AIFF-C-Dateien.
 *
 * Unterstützt werden klassische AIFF-Dateien (`FORM`/`AIFF`) sowie AIFF-C
 * (`FORM`/`AIFC`) mit dem Kompressionstyp `NONE` oder `FL32`. Alle anderen
 * AIFF-C-Kompressionen (zum Beispiel `sowt`, `ulaw`, `alaw`, `adpc`, `fl64`) leitet
 * `open()` mit `UnsupportedAudioFormatException` ab, ebenso Dateien mit mehr als zwei
 * Kanälen. Monodateien werden unterstützt, indem das eine Kanal-Sample auf links und
 * rechts kopiert wird.
 *
 * Die Samples stehen in der nativen Abtastrate der Datei zur Verfügung; eine
 * Konvertierung der Abtastrate findet nicht statt. Beim Lesen werden die Werte auf den
 * Wertebereich des Zielpuffers skaliert:
 *
 * | Quelle                        | `getSamples(STEREOSAMPLE16)` | `addSamples(STEREOSAMPLE32)` | `getSamples`/`addSamples`(STEREOSAMPLE_FLOAT) |
 * | ----------------------------- | ---------------------------- | ---------------------------- | --------------------------------------------- |
 * | 16 Bit Integer                | unverändert                  | unverändert                  | Wert / 32768,0f                               |
 * | 24 Bit Integer                | Wert >> 8                    | Wert >> 8                    | Wert / 8388608,0f                             |
 * | 32 Bit Integer                | Wert >> 16                   | Wert >> 16                   | Wert / 2147483648,0f                          |
 * | 32 Bit Float (AIFF-C `FL32`)  | Wert × 32768, auf ±32768 begrenzt | Wert × 32768 (additiv) | unverändert                                   |
 *
 * Der `STEREOSAMPLE32`-Puffer dient dem Mischen mehrerer Quellen: er nimmt durchgehend
 * Werte im 16-Bit-Wertebereich auf, egal welche Bitbreite eine Quelle hat. Mehrere
 * decodierte Streams können dadurch ohne weitere Skalierung addiert und anschliessend
 * auf den 16-Bit-Bereich begrenzt werden. Abgeschnitten wird jeweils ohne Rundung.
 *
 * @note Die AIFF-Spezifikation definiert Samplewerte als Vorzeichen-Betrag-Werte
 * (Sign-Magnitude). Der Decoder interpretiert sie - wie die gängigen Encoder
 * (ffmpeg, Audacity) - als Zweierkomplement.
 *
 * @note Ein 24-Bit-Sample belegt genau 3 Bytes je Kanal, ein 16-Bit-Sample 2 Bytes.
 * Das in der Spezifikation erwähnte Auffüllen auf 16-Bit-Wortgrenzen (24 Bit in 4 Bytes)
 * wird nicht erkannt; eine so geschriebene Datei liefert falsche Samples.
 *
 * @see AudioInfo::SampleEncoding
 */
class AudioDecoder_Aiff : public AudioDecoder
{
private:
    /** @brief Kodierung eines Kanal-Samples, wie sie `open()` aus dem AudioInfo ableitet. */
    enum class FrameEncoding : uint8_t
    {
        NONE = 0,    //!< kein offener Decoder bzw. nicht unterstuetzte Kodierung
        INT16 = 1,   //!< 16-Bit-Zweierkomplement
        INT24 = 2,   //!< 24-Bit-Zweierkomplement, gepackt in 3 Bytes
        INT32 = 3,   //!< 32-Bit-Zweierkomplement
        FLOAT32 = 4  //!< 32-Bit-IEEE-Float (AIFF-C-Kompression `FL32`)
    };

    FileObject* ff;
    AudioInfo info;
    size_t position;
    size_t bytesPerChannel;  //!< Bytes eines Kanal-Samples innerhalb eines Frames
    FrameEncoding encoding;

    /** @brief Begrenzt die angeforderte Samplezahl und liefert den Dateiversatz des
     *         ersten Frames. Rueckgabewert 0 bedeutet "nichts zu lesen".
     *
     * @exception DecoderException Kein offener Dateihandle oder \p buffer ist NULL.
     */
    size_t prepareRead(size_t num, const void* buffer, uint64_t& offset) const;

public:
    AudioDecoder_Aiff();
    ~AudioDecoder_Aiff() override;

    /** @brief Öffnet die Datei und legt die Kodierung der Samples fest.
     *
     * Wird \p info übergeben, verwendet der Decoder diese Werte unverändert. Er prüft
     * dann Format, Kanalzahl, und die Kombination aus `BitsPerSample`, `BytesPerSample`
     * sowie `SampleType`; jede nicht dekodierbare Kombination wird mit
     * `UnsupportedAudioFormatException` abgelehnt. Alle weiteren Felder, insbesondere
     * `AudioStart` und `AudioSize`, werden vorausgesetzt und nicht gegen die Datei
     * validiert - übergebene Werte müssen also von `IdentAudioFile()` stammen.
     *
     * @exception UnsupportedAudioFormatException Format ist nicht AIFF/AIFF-C, oder die
     *   Sample-Kodierung wird nicht unterstützt.
     * @exception DecoderInitializationException Das Format wurde nicht erkannt.
     */
    void open(FileObject& file, const AudioInfo* info = NULL) override;

    /** @brief Schließt den Decoder und erklärt das AudioInfo für ungültig.
     *
     * Nach `close()` liefert `getAudioInfo()` ein leeres `AudioInfo` (alle Felder 0),
     * `seekSample()` hat keine Wirkung und die Lesemethoden werfen eine Exception.
     */
    void close() override;

    /** @brief Gibt Informationen über das geöffnete Audio zurück.
     *
     * Sinnvolle Werte liefert die Methode nur zwischen `open()` und `close()`.
     */
    const AudioInfo& getAudioInfo() const override;

    /** @brief Setzt die Position auf das gegebene Sample (Frame).
     *
     * Werte jenseits des Dateiendes werden an `AudioInfo::Samples` begrenzt; eine
     * Exception wird nie ausgelöst. Nach `close()` bleibt die Position auf 0 stehen.
     */
    void seekSample(size_t sample) override;

    size_t getPosition() const override;

    /** @brief Liest Frames in einen Puffer im Format STEREOSAMPLE16.
     *
     * Rückgabe ist die tatsächlich gelesene Anzahl an Frames (ein Frame = ein Sample je
     * Kanal), höchstens \p num. Am Dateiende wird 0 ohne Exception geliefert. Der
     * Puffer muss groß genug für die returnierte Anzahl sein; er wird nicht aufgefüllt.
     *
     * @exception DecoderException Vor `open()` oder nach `close()`.
     */
    size_t getSamples(size_t num, STEREOSAMPLE16* buffer) override;

    /** @brief Addiert Frames zu einem Puffer im Format STEREOSAMPLE32 (Mischbetrieb).
     *
     * Siehe Klassenbeschreibung zur Skala der addierten Werte.
     *
     * @exception DecoderException Vor `open()` oder nach `close()`.
     */
    size_t addSamples(size_t num, STEREOSAMPLE32* buffer) override;

    /** @brief Liest Frames in einen Puffer im Format STEREOSAMPLE_FLOAT.
     *
     * @exception DecoderException Vor `open()` oder nach `close()`.
     */
    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;

    /** @brief Addiert Frames zu einem Puffer im Format STEREOSAMPLE_FLOAT.
     *
     * @exception DecoderException Vor `open()` oder nach `close()`.
     */
    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;
};

class AudioDecoder_MP3 : public AudioDecoder
{
private:
    void* decoder;
    FileObject* ff;
    uint8_t* readbuffer;
    uint8_t* outbuffer;

    AudioInfo info;
    size_t position;
    size_t samplesize;
    size_t out_offset, out_size;
    bool isRunning;
    bool needInput;
    int lastDecodeFormat;

    size_t fillDecodeBuffer();

public:
    AudioDecoder_MP3();
    ~AudioDecoder_MP3() override;
    void open(FileObject& file, const AudioInfo* info = NULL) override;
    void close() override;
    const AudioInfo& getAudioInfo() const override;
    void seekSample(size_t sample) override;
    size_t getPosition() const override;
    size_t getSamples(size_t num, STEREOSAMPLE16* buffer) override;
    size_t addSamples(size_t num, STEREOSAMPLE32* buffer) override;
    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;
    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;
};

class AudioDecoder_Ogg : public AudioDecoder
{
private:
    void* private_data;
    size_t position;
    char* readbuffer;
    size_t buffersize;
    char* decodebuffer;
    int decodebuffer_size;
    AudioInfo info;

    void allocateBuffer(size_t size);

public:
    AudioDecoder_Ogg();
    ~AudioDecoder_Ogg() override;
    void open(FileObject& file, const AudioInfo* info = NULL) override;
    void close() override;
    const AudioInfo& getAudioInfo() const override;
    void seekSample(size_t sample) override;
    size_t getPosition() const override;
    size_t getSamples(size_t num, STEREOSAMPLE16* buffer) override;
    size_t addSamples(size_t num, STEREOSAMPLE32* buffer) override;
    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;
    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;
};

} // namespace pplib

#endif // PPLIB_AUDIO_DECODER_H