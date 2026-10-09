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
    void readWaveHeader(FileObject& file, WAVEHEADER& header);

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

class AudioDecoder_Aiff : public AudioDecoder
{
private:
    FileObject* ff;
    AudioInfo info;
    size_t position;
    size_t samplesize;

public:
    AudioDecoder_Aiff();
    ~AudioDecoder_Aiff() override;
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