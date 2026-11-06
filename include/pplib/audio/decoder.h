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
 * @note Alle Funktionen, die Samples lesen oder setzen, arbeiten in Frames: Ein Frame
 * enthält je einen Sample-Wert pro Kanal. \p num, Rückgabewerte und `AudioInfo::Samples`
 * zählen in Frames; `AudioInfo::BitsPerSample` bezieht sich auf einen einzelnen Kanalwert.
 *
 * Welche Dateiformate, Kanalzahlen und Bitbreiten ein Decoder unterstützt, steht in der
 * Beschreibung der abgeleiteten Klassen.
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

    /** @brief Verwendet die gegebene Datei als Quelle und initialisiert den Decoder.
     *
     * Diese Methode verwendet das angegebene FileObject \p file als Quelle der Audiodaten
     * und initialisiert den Decoder. Solange der Decoder existiert, darf das Fileobjekt nicht
     * vorzeitig geschlossen oder zerstört werden! Auch darf nicht parallel auf das FileObject zugegriffen werden, da
     * der Lesezeiger während der Nutzung des Decoders verändert wird.
     *
     * @param file Die zu öffnende Datei.
     * @param info Optional: Falls zuvor bereits Informationen über das Audio vorliegen (zum Beispiel nach Aufruf von `IdentAudioFile()`),
     * können diese hier übergeben werden. Die Funktion muss diese Informationen dann nicht noch einmal auslesen.
     *
     * @note Solange der Decoder existiert, muss das übergebene `FileObject` \p file gültig bleiben und darf nicht vorzeitig zerstört werden.
     * Passiert dies doch, muss mit weiteren als den angegebenen Exceptions und undefiniertem Verhalten gerechnet werden!
     * @note Wenn die Datei, auf die das FileObject zeigt, zur Laufzeit wächst, kann dies zu unerwartetem Verhalten führen, da der Decoder die
     * Dateigröße nur beim Öffnen der Datei ermittelt.
     *
     * @exception DecoderInitializationException Wenn der Decoder nicht initialisiert werden kann.
     * @exception UnsupportedAudioFormatException Wenn das Audio-Format der Datei nicht unterstützt wird.
     * @exception OutOfMemoryException Der Speicher für den Decoder oder interne Puffer konnte nicht zugewiesen werden.
     * @exception UnsupportedFeatureException Eine für einen spezifischen Dekoder benötigte third party library ist nicht verfügbar/einkompiliert.
     * @note Andere Exceptions können je nach Implementierung des spezifischen Decoders auftreten.
     */
    virtual void open(FileObject& file, const AudioInfo* info = NULL) = 0;

    /** @brief Schließt den Decoder und gibt alle Ressourcen frei.
     *
     * Diese Methode gibt alle vom Decoder belegten Ressourcen frei und setzt den Decoder in einen nicht initialisierten Zustand zurück.
     * Nach dem Aufruf von `close()` darf der Decoder erneut mit `open()` geöffnet werden.
     * 
     * @note Das zuvor übergebene `FileObject` \p file wird nicht geschlossen und ist weiterhin gültig. Die Anwendung
     * ist dafür verantwortlich, das `FileObject` nach dem Aufruf von `close()` zu schließen oder zu zerstören.
     * 
     * @exception UnsupportedFeatureException Eine für einen spezifischen Dekoder benötigte third party library ist nicht verfügbar/einkompiliert.
     */
    virtual void close() = 0;

    /** @brief Gibt Informationen über das Audio zurück.
     *
     * Gibt Informationen über das geöffnete Audio-File zurück. Wird die Methode vor `open()` oder nach `close()` aufgerufen,
     * wird ein leeres `AudioInfo` zurückgegeben.
     *
     * @return Referenz auf die AudioInfo-Struktur.
     */
    virtual const AudioInfo& getAudioInfo() const = 0;

    /** @brief Setzt die aktuelle Position auf das gegebene Sample.
     * @param sample Die Sample-Position, zu der gesprungen werden soll.
     * @exception DecoderException wird geworfen, wenn ein Fehler beim Setzen der Position auftritt, oder
     * der Decoder keine Audiodatei geöffnet hat.
     * @exception UnsupportedFeatureException Eine für einen spezifischen Dekoder benötigte third party library ist nicht verfügbar/einkompiliert.
     */
    virtual void seekSample(size_t sample) = 0;

    /** @brief Gibt die aktuelle Sample-Position zurück.
     *
     * @return Die aktuelle Sample-Position im Audio-Stream. Ist 0, wenn keine Audiodatei geöffnet ist oder der Stream am Anfang steht.
     */
    virtual size_t getPosition() const noexcept = 0;

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
     * @return Die tatsächlich gelesene Anzahl an Samples. 0, wenn keine Samples mehr verfügbar waren, oder ein Fehler aufgetreten ist.
     */
    virtual size_t getSamples(size_t num, STEREOSAMPLE16* buffer) noexcept = 0;

    /** @brief Addiert Audio-Samples aus dem File zum gegebenen Puffer.
     *
     * Liest \p num Audio-Samples aus dem Audio-Stream und addiert diese zum gegebenen Puffer \p buffer.
     *
     * Der Puffer ist hier im Format `STEREOSAMPLE32`. Er dient dem Mischen mehrerer Audio-Quellen. Er
     * nimmt durchgehend Werte im 16-Bit-Wertebereich auf, egal welche Bitbreite eine Quelle hat. Dateien mit höherer
     * Bitbreite werden entsprechend skaliert.
     *
     * Die Funktion kann verwendet werden, wenn zum Beispiel mehrere Audio-Streams gemischt werden sollen.
     *
     * @param num Die Anzahl der zu addierenden Samples.
     * @param buffer Zeiger auf den Puffer, zu dem die Samples addiert werden sollen.
     * @return Die tatsächlich addierte Anzahl an Samples. 0, wenn keine Samples mehr verfügbar waren, oder ein Fehler aufgetreten ist.
     */
    virtual size_t addSamples(size_t num, STEREOSAMPLE32* buffer) noexcept = 0;

    /** @brief Liest Samples in einen Puffer im Format STEREOSAMPLE_FLOAT ein.
     *
     * Liest \p num Audio-Samples aus dem Audio-Stream und schreibt sie in den gegebenen Puffer \p buffer.
     * Der Puffer ist hier im Format STEREOSAMPLE_FLOAT. Dabei entspricht ein Sample jeweils einem 32-Bit-Float-Wert
     * für links und einen 32-Bit-Float-Wert für rechts. Der Puffer muss entsprechend groß sein, um alle Samples aufzunehmen.
     *
     * @param num Die Anzahl der zu lesenden Samples.
     * @param buffer Zeiger auf den Puffer, in den die Samples geschrieben werden sollen.
     * @return Die tatsächlich gelesene Anzahl an Samples. 0, wenn keine Samples mehr verfügbar waren, oder ein Fehler aufgetreten ist.
     */
    virtual size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept = 0;

    /** @brief Addiert Samples zum gegebenen Puffer im Format STEREOSAMPLE_FLOAT.
     *
     * Liest \p num Audio-Samples aus dem Audio-Stream und addiert diese zum gegebenen Puffer \p buffer.
     *
     * Der Puffer ist hier im Format `STEREOSAMPLE_FLOAT`. Er dient dem Mischen mehrerer Audio-Quellen.
     * Die normierten Werte der Quelle werden zu den bestehenden Werten addiert.
     *
     * Die Funktion kann verwendet werden, wenn zum Beispiel mehrere Audio-Streams gemischt werden sollen.
     *
     * @param num Die Anzahl der zu addierenden Samples.
     * @param buffer Zeiger auf den Puffer, zu dem die Samples addiert werden sollen.
     * @return Die tatsächlich addierte Anzahl an Samples. 0, wenn keine Samples mehr verfügbar waren, oder ein Fehler aufgetreten ist.
     */
    virtual size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept = 0;
};

/** @ingroup PPLGroupSound
 * @brief Erstellt einen passenden Audio-Decoder für die gegebene Datei.
 *
 * Diese Funktion analysiert die gegebene Datei und erstellt einen passenden Audio-Decoder basierend auf dem Dateiformat.
 * Dieser wird als Zeiger auf ein AudioDecoder-Objekt zurückgegeben. Der Aufrufer ist für das Löschen des Decoders
 * verantwortlich (delete).
 *
 * @param file Die Datei, für die ein Audio-Decoder erstellt werden soll.
 * @return Zeiger auf ein AudioDecoder-Objekt, das für die gegebene Datei geeignet ist, oder NULL,
 * falls kein passender Decoder gefunden wurde oder der Decoder die Datei nicht öffnen kann.
 * In beiden Fällen wird kein Objekt angelegt bzw. das halb fertiggestellte wieder freigegeben;
 * die Funktion wirft keine Ausnahme. Ein erfolgreicher Rückgabewert ist ein bereits geöffneter
 * Decoder; ein zusätzliches `open()` durch den Aufrufer ist nicht erforderlich.
 *
 * @note Solange der Decoder existiert, muss das übergebene `FileObject` \p file gültig bleiben und darf nicht
 * vorzeitig zerstört werden!
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
     *         ersten Frames. Rueckgabewert 0 bedeutet "nichts zu lesen" oder Fehler,
     *         zum Beispiel wenn die Datei nicht geöffnet ist.
     */
    size_t prepareRead(size_t num, const void* buffer, uint64_t& offset) const noexcept;

public:
    AudioDecoder_Wave();
    ~AudioDecoder_Wave() override;

    void open(FileObject& file, const AudioInfo* info = nullptr) override;

    void close() override;

    const AudioInfo& getAudioInfo() const override;

    void seekSample(size_t sample) override;

    size_t getPosition() const noexcept override;

    size_t getSamples(size_t num, STEREOSAMPLE16* buffer) noexcept override;

    size_t addSamples(size_t num, STEREOSAMPLE32* buffer) noexcept override;

    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept override;

    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept override;
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
     *         ersten Frames. Rueckgabewert 0 bedeutet "nichts zu lesen" oder Fehler,
     *         zum Beispiel wenn die Datei nicht geöffnet ist.
     */
    size_t prepareRead(size_t num, const void* buffer, uint64_t& offset) const noexcept;

public:
    AudioDecoder_Aiff();
    ~AudioDecoder_Aiff() override;

    void open(FileObject& file, const AudioInfo* info = NULL) override;

    void close() override;

    const AudioInfo& getAudioInfo() const override;

    void seekSample(size_t sample) override;

    size_t getPosition() const noexcept override;

    size_t getSamples(size_t num, STEREOSAMPLE16* buffer) noexcept override;

    size_t addSamples(size_t num, STEREOSAMPLE32* buffer) noexcept override;

    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept override;

    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept override;
};

struct Mp3Data;

/** @class AudioDecoder_MP3
 * @ingroup PPLGroupSound
 * @brief Decoder für MP3-Dateien auf Basis von libmpg123.
 *
 * Mit diesem Dekoder können mp3-Dateien gelesen und dekodiert werden.
 * Unterstützt werden Dateien mit 1 oder 2 Kanälen (Mono/Stereo). Die gelieferten Samples entsprechen
 * der in der Datei kodierten Abtastrate, es findet kein Resampling statt.
 *
 * @note MP3-Dateien enthalten keine Samplezahl; `AudioInfo::Samples` und `AudioInfo::Length`
 * stammen aus dem Xing-/VBRI-Feld oder aus dem Zählen der Rahmen und können von der
 * tatsächlich dekodierbaren Menge abweichen.
 * @note Das Zielformat kann während des Abspielens gewechselt werden (z. B. erst
 * `getSamples(STEREOSAMPLE16)`, dann `getSamples(STEREOSAMPLE_FLOAT)`), ohne dass Samples
 * verloren gehen.
 * @note Voraussetzung ist ein Build mit libmpg123 (`PPLIB_WITH_MPG123`). Fehlt sie, ist das
 * Objekt konstruierbar, aber alle Funktionen außer `getAudioInfo()` und `getPosition()` Werfen
 * eine `UnsupportedFeatureException`; `GetAudioDecoder()` erzeugt dann keinen MP3-Decoder.
 *
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

public:
    /** @brief Konstruktor für den MP3-Decoder.
     *
     * @exception OutOfMemoryException Wird geworfen, wenn nicht genügend Speicher für den Decoder verfügbar ist.
     * @exception DecoderInitializationException Wird geworfen, wenn die Initialisierung des Decoders fehlschlägt.
     */
    AudioDecoder_MP3();
    ~AudioDecoder_MP3() override;

    void open(FileObject& file, const AudioInfo* info = NULL) override;

    void close() override;

    const AudioInfo& getAudioInfo() const override;

    void seekSample(size_t sample) override;

    size_t getPosition() const noexcept override;

    size_t getSamples(size_t num, STEREOSAMPLE16* buffer) noexcept override;

    size_t addSamples(size_t num, STEREOSAMPLE32* buffer) noexcept override;

    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept override;

    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept override;
};

/** @class AudioDecoder_Ogg
 * @ingroup PPLGroupSound
 * @brief Decoder für Ogg-Vorbis-Dateien (`.ogg`, `.oga`).
 *
 * Unterstützt werden Ogg-Dateien mit Vorbis-Audio und einem oder zwei Kanälen; Monodaten
 * werden auf links und rechts kopiert. Mehr als zwei Kanäle lehnt `open()` mit
 * `UnsupportedAudioFormatException` ab, während `IdentAudioFile()` solche Dateien als `OGG`
 * erkennt. Die Schnittstelle und das Verhalten der Funktionen beschreibt `AudioDecoder`.
 *
 * @see IdentAudioFile()
 *
 * Die gelieferten Werte hängen vom Zielpuffer ab:
 *
 * | Zielpuffer              | Werte                                        |
 * | ----------------------- | -------------------------------------------- |
 * | `STEREOSAMPLE16`        | 16-Bit-Werte der Datei                       |
 * | `STEREOSAMPLE32`        | dieselben Werte, für das Mischen mehrerer Quellen |
 * | `STEREOSAMPLE_FLOAT`    | normiert, Bereich etwa −1,0 bis 1,0          |
 *
 *
 * @note Eine verkettete Ogg-Datei (mehrere Bitströme hintereinander) wird nur bis zum Ende des
 * ersten Bitstroms gespielt; `AudioInfo::Samples` und `AudioInfo::Length` beschreiben ebenfalls
 * nur diesen Strom.
 * @note `seekSample()` lehnt ein Ziel jenseits des ersten Bitstroms mit `DecoderException` ab.
 * @note `AudioInfo::AudioStart`/`AudioEnd` bezeichnen Seiten des Ogg-Containers einschließlich
 * ihrer Köpfe; beide bleiben ungesetzt, wenn das Datenende nicht bestimmbar ist (nicht seekbarer Input).
 * @note `AudioInfo::IsVBR`/`Bitrate` sind close Angaben des Encoders, keine gemessenen Werte;
 * `Bitrate` steht in kbit/s.
 * @note Bei einer unvollständigen Datei (fehlendes Dateiende) liefern die Lesefunktionen die
 * dekodierbaren Frames und danach 0; `IdentAudioFile()` erkennt solche Dateien möglicherweise
 * ohne Samplezahl.
 * @note Voraussetzung ist ein Build mit libvorbis (`PPLIB_WITH_VORBIS`). Fehlt sie, wirft jede
 * Funktion außer `getAudioInfo()` und `getPosition()` eine `UnsupportedFeatureException`;
 * `GetAudioDecoder()` erzeugt dann keinen Ogg-Decoder.
 *
 * @see AudioInfo::AudioFormat::OGG
 */
class AudioDecoder_Ogg : public AudioDecoder
{
private:
    void* private_data; //!< libvorbisfile-Handle; NULL, solange nicht geöffnet ist
    size_t position;    //!< nächste zu lesende Position in Frames (nicht Kanal-Samples)

    char* decodebuffer;    //!< reservierter Vorrat für die Callback-Funktionen
    int decodebuffer_size; //!< Größe von `decodebuffer` in Byte
    AudioInfo info;

public:
    AudioDecoder_Ogg();
    ~AudioDecoder_Ogg() override;

    void open(FileObject& file, const AudioInfo* info = NULL) override;

    void close() override;

    const AudioInfo& getAudioInfo() const override;

    void seekSample(size_t sample) override;

    size_t getPosition() const noexcept override;

    size_t getSamples(size_t num, STEREOSAMPLE16* buffer) noexcept override;

    size_t addSamples(size_t num, STEREOSAMPLE32* buffer) noexcept override;

    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept override;

    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept override;
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
     *         "nichts zu lesen" oder ein Fehler ist aufgetreten.
     */
    size_t prepareRead(size_t num, const void* buffer) noexcept;

    /** @brief Zaehlt \p frames uebernommene Frames weiter und raeumt den verbrauchten
     *         Teil des Rohpuffers zurueck. */
    void claimFrames(size_t frames);

public:
    AudioDecoder_Flac();
    ~AudioDecoder_Flac() override;

    void open(FileObject& file, const AudioInfo* info = nullptr) override;

    void close() override;

    const AudioInfo& getAudioInfo() const override;

    void seekSample(size_t sample) override;

    size_t getPosition() const noexcept override;

    size_t getSamples(size_t num, STEREOSAMPLE16* buffer) noexcept override;

    size_t addSamples(size_t num, STEREOSAMPLE32* buffer) noexcept override;

    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept override;

    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) noexcept override;
};

} // namespace pplib

#endif // PPLIB_AUDIO_DECODER_H