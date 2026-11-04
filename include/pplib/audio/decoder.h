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
     * Ein Sample ist immer ein Frame, also ein Wert pro Kanal. Die Zahl der Werte im Puffer
     * ist deshalb unabhängig von der Kanalzahl der Datei und entspricht immer \p num.
     *
     * @param num Die Anzahl der zu lesenden Samples.
     * @param buffer Zeiger auf den Puffer, in den die Samples geschrieben werden sollen.
     * @return Die tatsächlich gelesene Anzahl an Samples. Das Ende des Audiodatenbereichs
     *   liefert 0, ohne eine Exception zu werfen.
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
 * falls kein passender Decoder gefunden wurde oder der Decoder die Datei nicht öffnen kann.
 * In beiden Fällen wird kein Objekt angelegt bzw. das halb fertiggestellte wieder freigegeben;
 * die Funktion wirft keine Ausnahme. Ein erfolgreicher Rückgabewert ist ein bereits geöffneter
 * Decoder; ein zusätzliches `open()` durch den Aufrufer ist nicht erforderlich.
 */
AudioDecoder* GetAudioDecoder(FileObject& file);

/** @class AudioDecoder_Wave
 * @ingroup PPLGroupSound
 * @brief Decoder für unkomprimierte WAV-Dateien (RIFF/`WAVE`).
 *
 * Unterstützt werden die Format-Tags `1` (PCM) und `3` (IEEE-Float, nur 32 Bit) mit
 * 16, 24 oder 32 Bit pro Kanal und einem oder zwei Kanälen. Monodateien werden
 * unterstützt, indem das eine Kanal-Sample auf links und rechts kopiert wird. Alle
 * anderen Format-Tags (`EXTENSIBLE` `0xFFFE`, `ADPCM`, `µ-law`, `a-law`, …) sowie
 * 8-Bit- und 20-Bit-Samples lehnt `open()` mit `UnsupportedAudioFormatException` ab;
 * `IdentAudioFile()` erkennt solche Dateien erst gar nicht als `WAVE`.
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
 * | 32 Bit Float (Format-Tag 3)   | Wert × 32768, auf ±32768 begrenzt | Wert × 32768 (additiv) | unverändert                                   |
 *
 * Der `STEREOSAMPLE32`-Puffer dient dem Mischen mehrerer Quellen: er nimmt durchgehend
 * Werte im 16-Bit-Wertebereich auf, egal welche Bitbreite eine Quelle hat. Mehrere
 * decodierte Streams können dadurch ohne weitere Skalierung addiert und anschliessend
 * auf den 16-Bit-Bereich begrenzt werden. Abgeschnitten wird jeweils ohne Rundung.
 *
 * @note WAV-Samples sind Little Endian abgelegt; der Decoder liest sie bytweise, ist also
 * unabhängig von der Byteordnung des Hosts.
 *
 * @note Float-Samples sind normiert (typisch -1,0 bis 1,0). Werte ausserhalb dieses
 * Bereichs werden beim Umrechnen auf 16 Bit hart begrenzt, NaN wird zu 0.
 *
 * @see AudioInfo::SampleEncoding
 */
class AudioDecoder_Wave : public AudioDecoder
{
private:
    /** @brief Kodierung eines Kanal-Samples, wie sie `open()` aus dem AudioInfo ableitet. */
    enum class FrameEncoding : uint8_t
    {
        NONE = 0,   //!< kein offener Decoder bzw. nicht unterstuetzte Kodierung
        INT16 = 1,  //!< 16-Bit-Zweierkomplement
        INT24 = 2,  //!< 24-Bit-Zweierkomplement, gepackt in 3 Bytes
        INT32 = 3,  //!< 32-Bit-Zweierkomplement
        FLOAT32 = 4 //!< 32-Bit-IEEE-Float (WAV-Format-Tag 3)
    };

    FileObject* ff;
    AudioInfo info;
    size_t position;
    size_t bytesPerChannel; //!< Bytes eines Kanal-Samples innerhalb eines Frames
    FrameEncoding encoding;

    /** @brief Begrenzt die angeforderte Samplezahl und liefert den Dateiversatz des
     *         ersten Frames. Rueckgabewert 0 bedeutet "nichts zu lesen".
     *
     * @exception DecoderException Kein offener Dateihandle oder \p buffer ist NULL.
     */
    size_t prepareRead(size_t num, const void* buffer, uint64_t& offset) const;

public:
    AudioDecoder_Wave();
    ~AudioDecoder_Wave() override;

    /** @brief Öffnet die Datei und legt die Kodierung der Samples fest.
     *
     * Wird \p info übergeben, verwendet der Decoder diese Werte unverändert. Er prüft
     * dann Format, Kanalzahl, und die Kombination aus `BitsPerSample`, `BytesPerSample`
     * sowie `SampleType`; jede nicht dekodierbare Kombination wird mit
     * `UnsupportedAudioFormatException` abgelehnt. Die Ortsangaben `AudioStart` und
     * `AudioSize` werden nicht gegen die Datei geprueft; der Decoder begrenzt beim Lesen
     * lediglich auf die tatsaechlich vorhandene Datenmenge.
     *
     * @exception UnsupportedAudioFormatException Format ist nicht WAVE, oder die
     *   Sample-Kodierung wird nicht unterstützt.
     */
    void open(FileObject& file, const AudioInfo* info = nullptr) override;

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

    /** @brief Aktuelle Position in Frames, bezogen auf den Anfang der Audiodaten
     *         (`AudioInfo::AudioStart`), nicht auf den Dateianfang.
     */
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

    /** @brief Addiert Frames zu einem Puffer im Format STEREOSAMPLE_FLOAT (Mischbetrieb).
     *
     * @exception DecoderException Vor `open()` oder nach `close()`.
     */
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
        NONE = 0,   //!< kein offener Decoder bzw. nicht unterstuetzte Kodierung
        INT16 = 1,  //!< 16-Bit-Zweierkomplement
        INT24 = 2,  //!< 24-Bit-Zweierkomplement, gepackt in 3 Bytes
        INT32 = 3,  //!< 32-Bit-Zweierkomplement
        FLOAT32 = 4 //!< 32-Bit-IEEE-Float (AIFF-C-Kompression `FL32`)
    };

    FileObject* ff;
    AudioInfo info;
    size_t position;
    size_t bytesPerChannel; //!< Bytes eines Kanal-Samples innerhalb eines Frames
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

struct Mp3Data;

/**
 * @brief MP3-Decoder auf Basis von libmpg123.
 *
 * Getragen werden Mono und Stereo; die gelieferten Samples entsprechen der in der Datei
 * kodierten Abtastrate, es wird weder resampelt noch auf zwei Kanäle hochgemischt. Eine
 * MP3-Datei enthält keinen Header mit Rahmen- oder Samplezahl: die von `IdentAudioFile()`
 * gemeldete Zahl stammt aus dem Xing-/VBRI-Feld oder aus dem Durchzählen der Rahmen und kann
 * von der tatsächlich dekodierbaren Menge abweichen. Dekodiert wird bis zum letzten Rahmen.
 *
 * Das Ausgabeformat des Decoders wird in `open()` aus den ermittelten Parametern gesetzt;
 * das Umschalten zwischen 16-Bit- und Float-Zielpuffer während des Abspielens ist ohne
 * Sampleverlust möglich.
 *
 * @exception DecoderException libmpg123 ist nicht verfügbar oder nicht initialisierbar.
 * @exception UnsupportedAudioFormatException kein MP3, keine 1-2 Kanaele, fehlender Audiodatenblock.
 *
 * Ohne libmpg123 (`HAVE_MPG123` undefiniert) sind alle Methoden vorhanden, werfen aber
 * `UnsupportedFeatureException`.
 */
class AudioDecoder_MP3 : public AudioDecoder
{
private:
    Mp3Data* mp3; ///< libmpg123-Handle, Datenbasis und dekodierte Samples (in DER Uebersetzung definiert)

    AudioInfo info;
    size_t position;

    /** @brief Dekodiert den naechsten Block in den Ausgangspuffer `Mp3Data::outbuffer`.
     *
     * Setzt `outframes`/`outpos` zurueck und fuellt neu. Liefert false, wenn die
     * Audiodaten erschöpft sind (`Mp3Data::eof`); der Aufrufer liefert dann 0 Frames.
     */
    bool fillFrames();
    /** @brief Gleicht `info` (Kanalzahl, Abtastrate) an die von libmpg123 gemeldeten
     *         Parameter an und setzt die Format-Tabelle erneut.
     */
    void syncFormat();
    /** @brief Stellt die Kodierung des Ausgabepuffers ein (`MPG123_ENC_SIGNED_16` oder
     *         `MPG123_ENC_FLOAT_32`). Samples, die schon im internen Puffer stehen, bleiben
     *         erhalten und werden bei der naechsten Uebertragung umgerechnet.
     */
    void setTargetEncoding(int encoding);
    /** @brief öffnet die Spur über libmpg123 erneut, damit sie in der eingestellten
     *         Kodierung dekodiert, und sucht auf \p resume zurück.
     *
     * @note Nur aufrufen, wenn der interne Puffer leer ist; der Rückwärtssuchlauf ist
     *       der einzige Preis des Wechsels.
     */
    void switchEncoding(size_t resume);
    /** @brief Holt bis zu \p num Frames (zu je zwei Werten) aus dem Ausgangspuffer in
     *         \p dest; die Elementgroesse ergibt sich aus der eingestellten Kodierung.
     *         Stehen im Puffer noch Samples der vorherigen Kodierung, werden sie
     *         umgerechnet; bei Mono werden beide Kanäle gefüllt.
     */
    size_t transferFrames(size_t num, void* dest);

public:
    AudioDecoder_MP3();
    ~AudioDecoder_MP3() override;

    /** @brief Oeffnet den Stream und richtet das Ausgabeformat von libmpg123 nach den
     *         ermittelten Parametern ein.
     *
     * @exception UnsupportedAudioFormatException \p file ist kein MP3, oder Kanalzahl,
     *            Abtastrate und Audiodatenblock passen nicht zueinander.
     * @exception DecoderException libmpg123 stellt kein Handle zur Verfuegung.
     */
    void open(FileObject& file, const AudioInfo* info = NULL) override;

    /** @brief Gibt dekodierte Samples und das libmpg123-Handle frei. Die Datei des
     *         Aufrufers bleibt offen. Mehrfaches `close()` ist erlaubt; anschliessend
     *         melden die Lesemethoden `DecoderException`, `open()` ist erneut moeglich.
     */
    void close() override;

    const AudioInfo& getAudioInfo() const override;

    /** @brief Springt framegenau zur Frameposition. libmpg123 sucht dafuer bytemaessig
     *         im Audiodatenblock und landet immer an einer Rahmenkante; Springe ueber
     *         das Ende der Audiodaten hinaus werden nicht begrenzt.
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
     * Die MP3-Samples liegen bereits auf der 16-Bit-Skala des Mischpuffers und werden
     * unveraendert addiert.
     *
     * @exception DecoderException Vor `open()` oder nach `close()`.
     */
    size_t addSamples(size_t num, STEREOSAMPLE32* buffer) override;

    /** @brief Liest Frames in einen Puffer im Format STEREOSAMPLE_FLOAT.
     *
     * Die Samples werden wie in `getSamples()` mit `STEREOSAMPLE16` durch 32768 geteilt,
     * damit beide Zieltypen dieselbe Skala haben. Ein Formatwechsel des Decoders
     * (Mono/Stereo, Samplefrequenz) mitten im Stream wird beruecksichtigt.
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

/** @class AudioDecoder_Ogg
 * @ingroup PPLGroupSound
 * @brief Decoder für Ogg-Vorbis-Dateien (`.ogg`, `.oga`) auf Basis von libvorbisfile.
 *
 * Unterstützt werden Dateien mit einem oder zwei Kanälen. Monodateien werden
 * unterstützt, indem das eine Kanal-Sample auf links und rechts kopiert wird. Mehr als
 * zwei Kanäle lehnt `open()` mit `UnsupportedAudioFormatException` ab; `IdentAudioFile()`
 * erkennt solche Dateien dagegen als `OGG` und meldet die Kanalzahl des Bitstroms:
 * Erkennung und Unterstützung sind getrennt, abgelehnt wird erst im Decoder.
 *
 * Ogg-Dateien können zu mehreren Ketten verkettet sein (mehrere Bitströme direkt
 * hintereinander, zum Beispiel durch `cat` zweier Dateien). Abgespielt wird nur die
 * erste Kette, obwohl libvorbisfile die verketteten Ströme fortlaufend weiterspielen
 * würde. Dazu passend beschreiben `AudioInfo::Samples` und `AudioInfo::Length` nur die
 * erste Kette; libvorbisfile meldet sonst die Summe aller Ketten.
 *
 * Die Bitbreite ist immer 16 (`AudioInfo::BitsPerSample`), die Abtastung
 * `SampleEncoding::INTEGER`, obwohl der Vorbis-Decoder intern mit Fließkomma arbeitet:
 * Die Integer-Pfade lesen über `ov_read()`, die Float-Pfade über `ov_read_float()`.
 *
 * | Quelle     | `getSamples(STEREOSAMPLE16)` | `addSamples(STEREOSAMPLE32)`     | `getSamples`/`addSamples`(STEREOSAMPLE_FLOAT) |
 * | ---------- | ---------------------------- | -------------------------------- | --------------------------------------------- |
 * | Ogg Vorbis | unverändert, 16 Bit          | unverändert, 16-Bit-Wertebereich | normiert, Bereich etwa −1,0 bis 1,0     |
 *
 * Der `STEREOSAMPLE32`-Puffer dient dem Mischen mehrerer Quellen. Die Float-Werte sind
 * die Decoderwerte ohne Rundung; die 16-Bit-Werte sind deren Rundung durch libvorbisfile,
 * unterscheiden sich also höchstens um eine Stelle (1/32768). Eine Konvertierung der
 * Abtastrate findet nicht statt.
 *
 * @note Alle vier Lesemethoden rechnen in Frames (ein Frame = ein Sample aller Kanäle):
 * Der übergebene Puffer fasst pro Frame zwei Werte, `num` ist die Zahl der Frames und der
 * Rückgabewert die Zahl tatsächlich gelesener Frames. Ist das Ende der ersten Kette
 * erreicht, liefert der Aufruf weniger Frames als angefordert; angeforderte Frames über
 * dem Kettenende entfallen, die entsprechenden Pufferfelder bleiben unverändert.
 *
 * @note `position` zählt Frames (nicht Kanal-Samples) und wird in allen vier Lesemethoden
 * nachgezogen; `open()` setzt sie auf 0, `seekSample()` setzt sie auf das Ziel.
 *
 * @note `seekSample()` lehnt Ziele hinter dem Ende der ersten Kette mit einer
 * `DecoderException` ab, statt sie still auf eine gültige Position umzusetzen. Ein Ziel
 * genau auf dem Kettenende gilt als Dateiende und liefert 0 Frames.
 *
 * @note `AudioInfo::AudioStart` und `AudioInfo::AudioEnd` sind Seitengrenzen des Ogg-
 * Containers: `AudioStart` zeigt auf den Anfang der ersten Datenseite nach den
 * Header-Seiten, `AudioEnd` auf das letzte Datenbyte des letzten Datenpages. Beide
 * Grenzen enthalten also die Seitenköpfe. `AudioInfo::AudioSize` und `AudioInfo::AudioEnd`
 * bleiben ungesetzt, wenn libvorbisfile das Ende nicht bestimmen kann (nicht seekbarer
 * Input).
 *
 * @note `AudioInfo::IsVBR` und `AudioInfo::Bitrate` leitet der Decoder aus den von
 * libvorbis gemeldeten nominalen, unteren und oberen Bitraten ab: sind untere und obere
 * Bitrate gleich, gilt die Datei als konstante Bitrate und `Bitrate` übernimmt diesen
 * Wert, sonst die nominale Bitrate. Die Angabe ist nur eine Näherung – eine mit
 * `libvorbis -q` kodierte VBR-Datei wird darüber in der Regel als CBR gemeldet.
 * `Bitrate` steht in kbit/s und wird auf den Bereich von `uint16_t` begrenzt; meldet
 * libvorbis einen negativen (fremde Encoder) oder unplausiblen Wert, steht 0 bzw. 65535.
 *
 * @note I/O-Fehler der übergebenen Datei erreichen den Aufrufer nicht: Die Callbacks des
 * Decoders fangen Ausnahmen des `FileObject` ab und melden sie libvorbisfile als
 * Kurzlesen. Ein Lesefehler wirkt damit wie ein Dateiende – die bis dahin dekodierten
 * Frames werden geliefert, danach liefert die Lesemethode 0. Ein Seek-Fehler beim
 * `seekSample()` meldet eine `DecoderException`.
 *
 * @note Eine angeschnittene Ogg-Datei erkennt `IdentAudioFile()` möglicherweise als `OGG`,
 * ohne die Samplezahl zu melden (`AudioInfo::Samples` und `AudioInfo::Length` bleiben 0);
 * kurze Rumpfstücke ohne vollständigen Header werden dagegen erst gar nicht erkannt. Ein
 * `getSamples()` auf einer solchen Datei liefert 0 Frames.
 *
 * @note Die Callback-Funktionen von libvorbisfile liefern Positionen als `long`. Auf
 * Plattformen mit 32 Bit grossem `long` (Windows/LLP64) sind dadurch Ogg-Dateien über
 * 2 GiB nicht korrekt seekbar; die Signaturen sind durch die Bibliothek vorgegeben.
 *
 * @note Voraussetzung ist ein pplib-Build mit libvorbis (CMake-Option
 * `PPLIB_WITH_VORBIS`). Fehlt die Bibliothek, wirft der Konstruktor eine
 * `UnsupportedFeatureException`; `open()`, `close()` und `seekSample()` enthalten dann
 * keinen Code und die Lesemethoden geben 0 zurück.
 *
 * @note Das übergebene `FileObject` gehört dem Decoder exklusiv: Er liest direkt über das
 * Objekt und verändert dabei die Dateiposition. `GetAudioDecoder()` liefert einen bereits
 * geöffneten Decoder; ein zusätzliches `open()` des Aufrufers ist überflüssig, aber
 * ungefährlich, weil `open()` den bisherigen Handle zunächst freigibt.
 *
 * @see AudioInfo::AudioFormat::OGG
 */
class AudioDecoder_Ogg : public AudioDecoder
{
private:
    void* private_data; //!< libvorbisfile-Handle; NULL, solange nicht geöffnet ist
    size_t position;    //!< nächste zu lesende Position in Frames (nicht Kanal-Samples)

    char* readbuffer;      //!< Zwischenpuffer der 16-Bit-Pfade
    size_t buffersize;     //!< Größe von `readbuffer` in Byte
    char* decodebuffer;    //!< reservierter Vorrat für die Callback-Funktionen
    int decodebuffer_size; //!< Größe von `decodebuffer` in Byte
    AudioInfo info;

    void allocateBuffer(size_t size);

public:
    AudioDecoder_Ogg();
    ~AudioDecoder_Ogg() override;

    /** @brief Öffnet die Datei, liest die Vorbis-Header und prüft die Unterstützung.
     *
     * Wird \p info übergeben, gelten die Werte des Objekts; der Decoder prüft nur die
     * Kanalzahl dagegen. Andernfalls ermittelt er sie aus den Vorbis-Headern und
     * bestimmt `AudioInfo::Samples` und `AudioInfo::Length` aus der ersten Kette.
     * Ein bereits offener Handle wird vorher freigegeben, `open()` nach `close()` ist
     * also erlaubt.
     *
     * @exception UnsupportedAudioFormatException Datei ist kein Ogg-Vorbis-Stream, oder
     *   das Format wird nicht unterstützt (mehr als zwei Kanäle).
     * @exception OutOfMemoryException Der Handle oder der interne Puffer fehlt.
     */
    void open(FileObject& file, const AudioInfo* info = NULL) override;

    /** @brief Gibt den libvorbisfile-Handle und die internen Puffer frei, setzt
     *         `position` auf 0 und ist aufrufbar, auch wenn nichts geöffnet ist. */
    void close() override;

    const AudioInfo& getAudioInfo() const override;

    /** @brief Springt zu Frame \p sample innerhalb der ersten Kette.
     *
     * @exception DecoderException Kein offener Decoder, oder \p sample liegt hinter dem
     *   Ende der ersten Kette.
     */
    void seekSample(size_t sample) override;

    size_t getPosition() const override;

    /** @brief Liest Frames in einen Puffer im Format STEREOSAMPLE16.
     *
     * Monodateien werden auf beide Kanäle hochgemischt; \p num und der Rückgabewert sind
     * Frames. Angeforderte Frames über dem Ende der ersten Kette entfallen. Die Werte
     * entsprechen der Rundung, die libvorbisfile aus den Float-Werten des Dekoders bildet.
     *
     * @exception DecoderException Kein offener Decoder, oder \p buffer ist NULL.
     */
    size_t getSamples(size_t num, STEREOSAMPLE16* buffer) override;

    /** @brief Addiert Frames zu einem Puffer im Format STEREOSAMPLE32.
     *
     * \p num und der Rückgabewert sind Frames; der Zielwert bleibt auf den
     * 16-Bit-Wertebereich begrenzt.
     *
     * @exception DecoderException Kein offener Decoder, oder \p buffer ist NULL.
     * @exception OutOfMemoryException Der Read-Puffer des Decoders reicht nicht.
     */
    size_t addSamples(size_t num, STEREOSAMPLE32* buffer) override;

    /** @brief Liest Frames in einen Puffer im Format STEREOSAMPLE_FLOAT.
     *
     * Die Float-Pfade dekodieren direkt mit `ov_read_float()`, ohne die Zwischenrundung
     * über 16 Bit. Monodateien werden auf beide Kanäle hochgemischt; \p num und der
     * Rückgabewert sind Frames. Das Kettenende der ersten Kette begrenzt die Lieferung.
     *
     * @exception DecoderException Kein offener Decoder, oder \p buffer ist NULL.
     */
    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;

    /** @brief Addiert Frames zu einem Puffer im Format STEREOSAMPLE_FLOAT.
     *
     * Der Zielwert wird auf den Wertebereich von `STEREOSAMPLE_FLOAT` begrenzt.
     *
     * @exception DecoderException Kein offener Decoder, oder \p buffer ist NULL.
     */
    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;
};

//! libFLAC-Handle, Datenbasis und Rohsamples; vollstaendig in AudioDecoder_Flac.cpp definiert
struct FlacState;

/** @class AudioDecoder_Flac
 * @ingroup PPLGroupSound
 * @brief Decoder für FLAC-Dateien (`fLaC`) mit 16 oder 24 Bit pro Kanal.
 *
 * Unterstützt werden Dateien mit einem oder zwei Kanälen und 16 oder 24 Bit pro
 * Kanal. Monodateien werden unterstützt, indem das eine Kanal-Sample auf links und
 * rechts kopiert wird. Alles andere (mehr Kanäle, 8/32 Bit, Kompression in Ogg)
 * lehnt `open()` mit `UnsupportedAudioFormatException` ab. `IdentAudioFile()` erkennt
 * solche Dateien dagegen als `FLAC` und meldet Kanalzahl und Bitbreite korrekt:
 * Erkennung und Unterstützung sind getrennt, abgelehnt wird erst hier.
 *
 * Metadaten-Blöcke (Padding, SEEKTABLE, VORBIS_COMMENT, PICTURE) werden übersprungen.
 * Ein vorangestelltes ID3v2-Tag wird beim Auffinden des Datenanfangs übersprungen; ein
 * ID3v1-Tag am Dateiende läuft über die Fehlerbehandlung der Decodier-Callbackfunktion
 * zusammen mit sonstigem Anhängsel als toleriertes Dateiende. Eine Konvertierung
 * der Abtastrate findet nicht statt. Beim Lesen
 * werden die Werte auf den Wertebereich des Zielpuffers skaliert:
 *
 * | Quelle        | `getSamples(STEREOSAMPLE16)` | `addSamples(STEREOSAMPLE32)` | `getSamples`/`addSamples`(STEREOSAMPLE_FLOAT) |
 * | ------------- | ---------------------------- | ---------------------------- | --------------------------------------------- |
 * | 16 Bit        | unverändert                  | unverändert                  | Wert / 32768,0f                               |
 * | 24 Bit        | Wert >> 8                    | Wert >> 8                    | Wert / 8388608,0f                             |
 *
 * Der `STEREOSAMPLE32`-Puffer dient dem Mischen mehrerer Quellen: er nimmt durchgehend
 * Werte im 16-Bit-Wertebereich auf, egal welche Bitbreite eine Quelle hat.
 * Abgeschnitten wird jeweils ohne Rundung.
 *
 * @note libFLAC liefert die Samples pro Kanal als separate, rechtsbündige `int32_t`-Ebenen;
 * der Decoder verschränkt (interleave) sie und wandelt sie beim Lesen um. Jede der vier
 * Lesemethoden schaltet vor ihrer Schleife über die Bitbreite der Quelle und lässt in der
 * innersten Schleife keine Formatentscheidung mehr.
 *
 * @note Voraussetzung ist ein pplib-Build mit libFLAC (CMake-Option `PPLIB_WITH_FLAC`).
 * Fehlt die Bibliothek, ist das Objekt konstruierbar, aber leer: bis auf `getAudioInfo()`
 * und `getPosition()` wirft jede Methode eine `UnsupportedFeatureException`.
 *
 * @note Die MD5-Prüfsumme einer FLAC-Datei wird bewusst nicht ausgewertet: Bei stückweisem
 * Lesen mit toleriertem Dateiende kann libFLAC sie nicht gültig abschließen. Defekte
 * Audiodaten melden eine `DecoderException`, eine abgeschnittene Datei liefert die
 * vollständig dekodierbaren Frames und endet mit einem Rückgabewert unter der
 * angeforderten Zahl. Ein Sync-Verlust, der erst nach dem letzten Rahmen gemeldet wird
 * (zum Beispiel ein Anhängsel wie ein ID3v1-Tag), ist von einem Defekt nicht zu
 * unterscheiden und zählt als Dateiende.
 *
 * @note Das übergebene `FileObject` gehört dem Decoder exklusiv: Er liest direkt über das
 * Objekt und verändert dabei die Dateiposition. Nutzen zwei Decoder dasselbe `FileObject`,
 * liest der zweite ab der Position, die der erste hinterlassen hat.
 *
 * @see AudioInfo::AudioFormat::FLAC
 */
class AudioDecoder_Flac : public AudioDecoder
{
private:
    /** @brief Kodierung eines Kanal-Samples, wie sie der STREAMINFO-Block vorgibt. */
    enum class FrameEncoding : uint8_t
    {
        NONE = 0,  //!< kein offener Decoder
        INT16 = 1, //!< 16-Bit-Zweierkomplement
        INT24 = 2  //!< 24-Bit-Zweierkomplement, von libFLAC in `int32_t` geliefert
    };

    FlacState* state; //!< libFLAC-Handle, Datenbasis und Rohsamples (in DER Uebersetzung definiert)
    FileObject* ff;
    AudioInfo info;
    size_t position;
    FrameEncoding encoding; //!< Bitbreite der Quelle, von `open()` aus STREAMINFO abgeleitet

    /** @brief Laedt Rohdaten decodiert nach, bis mindestens \p num Frames bereitstehen.
     *
     *  Decodier- oder Dateifehler werden nur im Zustand vermerkt, nicht geworfen.
     *  @return Anzahl bereitstehender Frames (kann wegen Dateiende kleiner sein). */
    size_t fillFrames(size_t num);

    /** @brief bereitet den naechsten Lesevorgang vor: prueft Voraussetzungen und stellt
     *         hochstens \p num Frames im Rohpuffer bereit. Ein Rueckgabewert von 0 bedeutet
     *         "nichts zu lesen". Die Pruefungen stehen hier und nicht in den vier
     *         Lesemethoden, weil sie fuer alle Zielformate voellig gleich sind.
     *
     *  @exception DecoderException Kein offener Decoder, \p buffer ist NULL oder die
     *          Audiodaten sind vor dem Ziel defekt. */
    size_t prepareRead(size_t num, const void* buffer);

    /** @brief Zaehlt \p frames uebernommene Frames weiter und raeumt den verbrauchten
     *         Teil des Rohpuffers zurueck. */
    void claimFrames(size_t frames);

public:
    AudioDecoder_Flac();
    ~AudioDecoder_Flac() override;

    /** @brief Öffnet die Datei, liest die Metadaten und prueft die Unterstuetzung.
     *
     * Wird \p info übergeben, prüft der Decoder Kanalzahl und Bitbreite gegen den
     * STREAMINFO-Block und lehnt Abweichungen ab. Andernfalls ermittelt er die Werte mit
     * `IdentAudioFile()`. Die Abtastrate steht im STREAMINFO-Block und gilt fest für den
     * gesamten Stream; `AudioInfo::Frequency` gibt sie unverändert weiter.
     * `AudioInfo::Bitrate` uebernimmt der Decoder aus dem `AudioInfo`-Objekt, das sie selbst
     * aus dem STREAMINFO-Block liest. Abgelehnt werden mehr als zwei Kanäle und Bitbreiten
     * ausser 16 und 24.
     *
     * Die Ortsangaben `AudioInfo::AudioStart` und `AudioInfo::AudioSize` aus \p info werden
     * nicht ausgewertet. Der Decoder sucht das `fLaC`-Magic selbst (inklusive ID3v2-Tag am
     * Dateianfang) und liest bis zum physischen Dateiende; ein ID3v1-Tag oder sonstiges
     * Anhängsel nach dem letzten Rahmen wird dabei toleriert, eine `AudioSize` aus einem
     * Container begrenzt den Datenstrom nicht.
     *
     * Das `FileObject` wird exklusiv an den Decoder uebergeben: Er liest direkt darin und
     * verandert die Dateiposition. Ein Objekt, das zugleich einem anderen Decoder oder
     * externen Lesern gehoert, sollte vorher eine kopierte eigene Datei besitzen.
     *
     * @exception UnsupportedAudioFormatException Datei ist kein FLAC, oder das Format
     *   wird nicht unterstützt.
     * @exception UnsupportedFeatureException pplib wurde ohne libFLAC gebaut.
     * @exception DecoderInitializationException libFLAC stellte keine Decoderinstanz zur
     *   Verfügung.
     * @exception DecoderException Der Datenanfang der FLAC-Kette ist nicht auffindbar,
     *   oder libFLAC konnte die Instanz nicht initialisieren.
     */
    void open(FileObject& file, const AudioInfo* info = nullptr) override;

    /** @brief Schließt den Decoder und erklärt das AudioInfo für ungültig.
     *
     * Nach `close()` liefert `getAudioInfo()` ein leeres `AudioInfo` (alle Felder 0),
     * `seekSample()` hat keine Wirkung und die Lesemethoden werfen eine Exception.
     *
     * @exception UnsupportedFeatureException pplib wurde ohne libFLAC gebaut.
     */
    void close() override;

    const AudioInfo& getAudioInfo() const override;

    /** @brief Setzt die Position auf das Sample mit der gegebenen Nummer.
     *
     * libFLAC springt zum Rahmen, der das Ziel-Sample enthaelt; die Position liegt
     * danach genau an dem angeforderten Sample. Werte ueber dem Ende werden auf das
     * letzte Sample begrenzt, wenn die Samplezahl bekannt ist.
     *
     * @note Schlaegt der Sprung fehl (Stream ohne Samplezahl oder unlesbarer Zielrahmen),
     * arbeitet der Decoder die Daten von der aktuellen Position linear ab. Die Position
     * bleibt dann nicht unveraendert, sondern endet am letzten erreichbar lesbaren Sample;
     * bei einer abgeschnittenen Datei liegt das deutlich vor dem angeforderten Ziel.
     *
     * @exception UnsupportedFeatureException pplib wurde ohne libFLAC gebaut.
     */
    void seekSample(size_t sample) override;

    size_t getPosition() const override;

    /** @brief Liest Frames in einen Puffer im Format STEREOSAMPLE16.
     *
     * @exception DecoderException Vor `open()` oder nach `close()`.
     * @exception UnsupportedFeatureException pplib wurde ohne libFLAC gebaut.
     */
    size_t getSamples(size_t num, STEREOSAMPLE16* buffer) override;

    /** @brief Addiert Frames zu einem Puffer im Format STEREOSAMPLE32.
     *
     * @exception DecoderException Vor `open()` oder nach `close()`.
     * @exception UnsupportedFeatureException pplib wurde ohne libFLAC gebaut.
     */
    size_t addSamples(size_t num, STEREOSAMPLE32* buffer) override;

    /** @brief Liest Frames in einen Puffer im Format STEREOSAMPLE_FLOAT.
     *
     * @exception DecoderException Vor `open()` oder nach `close()`.
     * @exception UnsupportedFeatureException pplib wurde ohne libFLAC gebaut.
     */
    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;

    /** @brief Addiert Frames zu einem Puffer im Format STEREOSAMPLE_FLOAT.
     *
     * @exception DecoderException Vor `open()` oder nach `close()`.
     * @exception UnsupportedFeatureException pplib wurde ohne libFLAC gebaut.
     */
    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;
};

} // namespace pplib

#endif // PPLIB_AUDIO_DECODER_H