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

#ifndef PPLIB_AUDIO_ENCODER_H
#define PPLIB_AUDIO_ENCODER_H

#include <cstdint>
#include <pplib/core/baseexception.h>
#include <pplib/core/fileobject.h>
#include <pplib/audio/sample_formats.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/id3tag.h>
#include <pplib/audio/decoder.h>

namespace pplib
{
PPLIBEXCEPTION(EncoderException, Exception);
PPLIBEXCEPTION(EncoderAlreadyStartedException, EncoderException);
PPLIBEXCEPTION(EncoderNotStartedException, EncoderException);
PPLIBEXCEPTION(EncoderBufferTooSmallException, EncoderException);
PPLIBEXCEPTION(EncoderPsychoAcousticException, EncoderException);
PPLIBEXCEPTION(EncoderInitializationException, EncoderException);
PPLIBEXCEPTION(EncoderAbortedException, EncoderException);
PPLIBEXCEPTION(EncoderAudioFormatMismatchException, EncoderException);

/** @class AudioEncoder_MP3
 * @brief MP3 Audio Encoder
 * @ingroup PPLGroupSound
 *
 * Mit diesem Encoder können Audio-Daten im MP3-Format kodiert werden. Er basiert auf der LAME-Bibliothek.
 * (siehe https://lame.sourceforge.io/).
 *
 * Um den Encoder zu verwenden, sind mehrere Schritte erforderlich:
 * 1. Konfiguration des Encoders über die entsprechenden set-Funktionen. Hierbei wird zum Beispiel die Qualität,
 *    Bitrate oder der Stereo-Modus festgelegt. Grundsätzlich muss man sich zwischen den 3 Modi CBR, VBR und ABR entscheiden:
 *    - CBR: Constant Bitrate, konstante Bitrate über die gesamte Datei.
 *    - VBR: Variable Bitrate, die Bitrate kann je nach Komplexität des Audios variieren.
 *    - ABR: Average Bitrate, die Bitrate wird im Durchschnitt auf den angegebenen Wert eingestellt.
 * 2. Audio-Daten encodieren, entweder über die Low-Level-Funktionen oder die High-Level-Funktion.
 * 3. Kodierprozess beenden mit finish() oder flushBuffer()
 *
 * Im Schritt zwei können beliebig viele Audio-Datenblöcke encodiert werden. Auch mehrere Audio-Dateien können
 * nacheinander verarbeitet werden, solange der Encoder nicht beendet wurde.
 *
 * Der Encoder kennt zwei Modi:
 * 1. Low-Level Encoding mit Buffern:
 *    Hierbei werden die Funktionen startEncode(int frequency, int channels), encodeBuffer() und flushBuffer() verwendet.
 * 2. High-Level Encoding mit AudioDecoder:
 *    Hierbei werden die Funktionen startEncode(), encode() und finish() verwendet. Die encode()-Methode übernimmt dabei das
 *    komplette Einlesen der Quelldatei und das Kodieren der Audio-Daten. Die Quelldatei muss zuvor mit AudioDecoder geöffnet werden.
 *    Falls der Fortschritt der encode()-Methode verfolgt werden soll, kann zuvor eine Progress-Funktion über setProgressFunction()
 *    gesetzt werden.
 *
 */
class AudioEncoder_MP3
{
private:
    class FirstAudio
    {
    public:
        int frequency;
        int channels;
    };
    void* gfp;
    FirstAudio firstAudio;
    FileObject* out;
    char* readcache;
    int mp3bufsize;
    unsigned char* mp3buf;

    uint32_t samples;

    void (*ProgressFunc)(int progress, void* priv);
    void* ProgressFuncPrivData;

    bool have_firstaudio;
    bool started;
    bool bStopEncode;

    void writeEncodedBytes(const char* buffer, size_t bytes);

public:
    AudioEncoder_MP3();
    ~AudioEncoder_MP3();

    /** @brief Konfiguriert den VBR-Modus (Variable Bitrate) des MP3-Encoders.
     *
     * @param min Minimale Bitrate in kbps.
     * @param max Maximale Bitrate in kbps.
     * @param quality Qualitätsstufe (0-9, 0=beste Qualität, 9=geringste Qualität).
     */
    void setVBR(int min = 32, int max = 320, int quality = 2);

    /** @brief Konfiguriert den CBR-Modus (Constant Bitrate) des MP3-Encoders.
     *
     * @param kbps Bitrate in kbps.
     * @param quality Qualitätsstufe (0-9, 0=beste Qualität, 9=geringste Qualität).
     */
    void setCBR(int kbps = 320, int quality = 2);

    /** @brief Konfiguriert den ABR-Modus (Average Bitrate) des MP3-Encoders.
     *
     * @param kbps Durchschnittliche Bitrate in kbps.
     * @param quality Qualitätsstufe (0-9, 0=beste Qualität, 9=geringste Qualität).
     */
    void setABR(int kbps = 192, int quality = 2);

    /** @brief Stereo-Modus des MP3-Encoders konfigurieren.
     *
     * @note Mögliche Werte für den Stereo-Modus sind:
     * * AudioInfo::ChannelMode::STEREO
     * * AudioInfo::ChannelMode::JOINT_STEREO
     * * AudioInfo::ChannelMode::DUAL_CHANNEL (wird von LAME nicht unterstützt)
     * * AudioInfo::ChannelMode::MONO
     *
     * @param mode Stereo-Modus (Standard: AudioInfo::ChannelMode::JOINT_STEREO).
     */
    void setStereoMode(const AudioInfo::ChannelMode mode = AudioInfo::ChannelMode::JOINT_STEREO);

    /** @brief Setzt die Lowpass-Frequenz für den MP3-Encoder.
     *
     * @param freq Frequenz in Hz (-1 = deaktiviert, 0 = LAME entscheidet).
     */
    void setLowpassFreq(int freq = 0);
    /** @brief Setzt die Highpass-Frequenz für den MP3-Encoder.
     *
     * @param freq Frequenz in Hz (-1 = deaktiviert, 0 = LAME entscheidet).
     */
    void setHighpassFreq(int freq = 0);

    /** @brief Setzt die Fortschrittsfunktion für den MP3-Encoder.
     *
     * @param ProgressFunc Zeiger auf die Fortschrittsfunktion.
     * @param priv Privatedaten, die an die Fortschrittsfunktion übergeben werden.
     */
    void setProgressFunction(void (*ProgressFunc)(int progress, void* priv), void* priv);

    /** @brief Startet die Kodierung in eine Ausgabedatei.
     *
     * @param output Referenz auf das Ausgabedateiobjekt.
     */
    void startEncode(FileObject& output);

    /** @brief Schreibt den ID3v2-Tag in die Ausgabedatei.
     *
     * Die Ausgabedatei muss zuvor durch startEncode() übergeben worden sein. Muss aufgerufen
     * werden, bevor die Kodierung beginnt, da der ID3v2-Tag An die aktuelle Position der
     * Ausgabedatei geschrieben wird.
     *
     * @param tag Referenz auf das ID3Tag-Objekt.
     */
    void writeID3v2Tag(const ID3Tag& tag);

    /** @brief Schreibt den ID3v1-Tag in die Ausgabedatei.
     *
     * Die Ausgabedatei muss zuvor durch startEncode() übergeben worden sein. Muss aufgerufen
     * werden, nachdem die Kodierung mit finish() abgeschlossen ist, da der ID3v1-Tag an das Ende der Ausgabedatei
     * geschrieben wird.
     *
     * @param tag Referenz auf das ID3Tag-Objekt.
     */
    void writeID3v1Tag(const ID3Tag& tag);

    /** @brief Kodiert Audio-Daten aus einem Decoder.
     *
     * Übernimmt die Audio-Daten aus dem angegebenen Decoder kodiert sie und schreibt sie in die Ausgabedatei.
     * Die Funktion kann beliebig oft aufgerufen werden, um zum Beispiel mehrere Audio-Dateien nacheinander zu kodieren.
     * @note Die Ausgabedatei muss vor dem Aufruf dieser Funktion durch startEncode() übergeben worden sein.
     *
     * @param decoder Referenz auf das AudioDecoder-Objekt.
     */
    void encode(AudioDecoder& decoder);

    /** @brief Beendet die Kodierung und schreibt alle verbleibenden Daten in die Ausgabedatei.
     *
     * Muss nach allen encode()-Aufrufen aufgerufen werden, um sicherzustellen, dass alle Daten korrekt kodiert
     * und geschrieben werden.
     */
    void finish();

    /** @brief Stoppt einen laufenden Encoder.
     *
     * Durch Aufruf dieser Funktion wird ein laufender Encoder gestoppt, der durch die Funktion
     * AudioEncoder_MP3::encode() gestartet wurde.
     */
    void stopEncoder();

    /** @brief Startet eine Low-Level Kodierung mit den angegebenen Audio-Parametern.
     *
     * @param frequency Abtastrate in Hz.
     * @param channels Anzahl der Audiokanäle.
     */
    void startEncode(int frequency, int channels);

    /** @brief Kodiert einen Puffer mit Audio-Daten.
     *
     * @param left Zeiger auf den linken Audiokanal.
     * @param right Zeiger auf den rechten Audiokanal.
     * @param num_samples Anzahl der Samples pro Kanal.
     * @param mp3buf Zeiger auf den Ausgabepuffer für die kodierten Daten.
     * @param buffersize Größe des Ausgabepuffers.
     * @return Anzahl der geschriebenen Bytes im Ausgabepuffer.
     */
    int encodeBuffer(SAMPLE16* left, SAMPLE16* right, int num_samples, void* mp3buf, size_t buffersize);

    /** @brief Kodiert einen Puffer mit Stereo-Audio-Daten.
     *
     * @param buffer Zeiger auf den Stereo-Audiopuffer.
     * @param num_samples Anzahl der Samples pro Kanal.
     * @param mp3buf Zeiger auf den Ausgabepuffer für die kodierten Daten.
     * @param buffersize Größe des Ausgabepuffers.
     * @return Anzahl der geschriebenen Bytes im Ausgabepuffer.
     */
    int encodeBuffer(STEREOSAMPLE16* buffer, int num_samples, void* mp3buf, size_t buffersize);

    /** @brief Schreibt alle verbleibenden kodierten Daten in den Ausgabepuffer.
     *
     * @param mp3buf Zeiger auf den Ausgabepuffer für die kodierten Daten.
     * @param buffersize Größe des Ausgabepuffers.
     * @return Anzahl der geschriebenen Bytes im Ausgabepuffer.
     */
    int flushBuffer(void* mp3buf, size_t buffersize);

    /** @brief Gibt die Version der LAME-Bibliothek zurück.
     *
     * @return C-String Zeichenkette mit der LAME-Version.
     */
    static const char* getLameVersion();

    /** @brief Gibt die Version der PSY-Bibliothek zurück.
     *
     * @return C-String Zeichenkette mit der PSY-Version.
     */
    static const char* getPSYVersion();
};
} // namespace pplib

#endif // PPLIB_AUDIO_ENCODER_H
