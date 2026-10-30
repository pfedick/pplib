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

#ifndef PPLIB_AUDIO_AUDIOINFO_H
#define PPLIB_AUDIO_AUDIOINFO_H

#include <cstdint>
#include <pplib/core/baseexception.h>
#include <pplib/core/fileobject.h>

namespace pplib
{

/** @brief Werfen, wenn Audiodaten einem unterstützten Decoder zugeordnet werden können, das Format
 *         aber nicht dekodiert werden kann.
 *
 * Die Exception wird von den Decodern (z. B. pplib::AudioDecoder_Wave) und vom Tag-Zugriff
 * (src/audio/ID3Tag.cpp) geworfen, nicht von pplib::IdentAudioFile().
 */
PPLIBEXCEPTION(UnsupportedAudioFormatException, Exception);

/** @class AudioInfo
 * @ingroup PPLGroupAudio
 * @brief Metadaten einer Audiodatei
 *
 * \desc
 * AudioInfo wird von pplib::IdentAudioFile() gefüllt und beschreibt Format,
 * Aufloesung und die Lage der Audiodaten innerhalb der Datei. Die reinen
 * Datenfelder koennen auch direkt verglichen werden, ein eigenstaendiges
 * Parsen ist nicht erforderlich.
 *
 * Alle Positionsangaben sind 0-basierte Offsets innerhalb der Datei.
 */
class AudioInfo
{
public:
    AudioInfo() = default;

    /** @brief Unterstuetzte Audioformate
     *
     * Wird von pplib::IdentAudioFile() zurueckgegeben. UNKNOWN bedeutet,
     * dass keine bekannte Audio-Struktur erkannt wurde.
     *
     * \desc
     * Nicht jedes Format, das erkannt wird, laesst sich auch dekodieren. Der
     * Parser akzeptiert bewusst nur eine Teilmenge:
     * - WAVE: `fmt ` als erster Chunk, Format-Tag 1 (PCM), maximal 2 Kanaele und
     *   8..32 Bit. EXTENSIBLE, Float-PCM und Mehrkanal werden nicht erkannt.
     * - AIFF: Classic-AIFF nur als `FORM`/`AIFF`; AIFF-C (`AIFC`) gilt als UNKNOWN.
     * - OGG: ausschliesslich Ogg-Vorbis; Opus oder FLAC im Ogg-Container sind UNKNOWN.
     * - MP3: MPEG Layer 1-3.
     */
    enum class AudioFormat : uint8_t
    {
        UNKNOWN = 0,   //!< Kein bekanntes Audioformat
        WAVE = 1,      //!< RIFF/WAVE (PCM, 8..32 Bit, max. 2 Kanaele)
        AIFF = 2,      //!< AIFF (Apple Audio Interchange File Format, nicht AIFF-C)
        MP3 = 3,       //!< MPEG Layer 1-3
        OGG = 4        //!< Ogg Vorbis (Opus/FLAC im Ogg-Container gelten als UNKNOWN)
    };
    /** @brief Kanalmodus
     *
     * \desc
     * Deckt die vier MPEG-Modi ab, nicht einfach nur Mono/Stereo. Bei WAV und
     * AIFF wird MONO bzw. STEREO in Abhaengigkeit der Kanaelanzahl gesetzt.
     */
    enum class ChannelMode : uint8_t
    {
        UNKNOWN = 0,       //!< Kanalmodus unbekannt (Default, z. B. bei unbekanntem Format)
        STEREO = 1,        //!< Stereo
        MONO = 2,          //!< Mono
        DUAL_CHANNEL = 3,  //!< Zwei unabhaengige Kanaele (MPEG-Modus)
        JOINT_STEREO = 4   //!< Joint Stereo (MPEG-Modus)
    };

    AudioFormat Format = AudioFormat::UNKNOWN;  //!< erkanntes Audioformat
    bool HaveID3v2Tag = false;               //!< Datei enthaelt einen ID3v2-Tag
    bool IsVBR = false;                       //!< Variable Bitrate (nur MP3)
    uint64_t ID3v2TagStart = 0;              //!< Offset des ID3v2-Tags (MP3: Tag beginnt bei 0)
    uint64_t FileSize = 0;                   //!< Groesse der Datei in Bytes
    uint64_t AudioStart = 0;                 //!< erster Byte der Audiodaten
    uint64_t AudioEnd = 0;                   //!< letzter Byte der Audiodaten (inklusive); es gilt immer AudioEnd - AudioStart + 1 == AudioSize
    uint64_t AudioSize = 0;                  //!< Groesse der Audiodaten in Bytes (frame-aligniert, bei WAV/AIFF immer > 0)
    uint64_t Samples = 0;                    //!< Anzahl Samples (nicht Kanal-Samples); bei WAV/AIFF Samples * BytesPerSample == AudioSize
    uint32_t Length = 0;                     //!< Spieldauer in Millisekunden
    uint32_t Frequency = 0;                  //!< Sample-Rate in Hz
    uint16_t Bitrate = 0;                    //!< Datenrate in kbit/s (bei PCM aus Frequency und BytesPerSample, bei Ueberlauf auf uint16_t-Max begrenzt)
    uint8_t BitsPerSample = 0;               //!< Aufloesung eines Kanal-Samples in Bits
    uint8_t BytesPerSample = 0;              //!< Bytes pro Sample-Frame, also alle Kanaele eines Sample-Zeitpunkts (AIFF/WAV: je Kanal auf volle Byte aufgerundet, dann mal Kanaele; 20 bit stereo => 6)
    uint8_t Channels = 0;                    //!< Anzahl Kanaele
    ChannelMode Mode = ChannelMode::UNKNOWN;  //!< Kanalmodus; bei unbekanntem Format ohne Aussage
};

/** @brief Audiodaten-Typ und -Aufbau einer Datei ermitteln
 *
 * \desc
 * Untersucht die Kopfbytes der Datei und fuellt @p info, sofern das Format
 * erkannt und unterstuetzt wird. Bei Misserfolg wird @p info nicht veraendert,
 * es bleiben also keine teilbefuellten Werte zurueck.
 *
 * Abgewiesen werden Konstellationen ohne abspielbare Samples (leere data- bzw.
 * SSND-Payload) sowie Bitauflösungen ausserhalb 8..32 Bit, weil andernfalls
 * BitsPerSample/BytesPerSample unsinnige Werte enthalten. Bei true gilt fuer
 * WAV und AIFF immer: `AudioSize > 0`, `AudioSize` ist ein Vielfaches von
 * `BytesPerSample` und `Samples * BytesPerSample == AudioSize`.
 *
 * Die Bedeutung von @p info ist formatabhaengig: WAV leitet Samples aus der
 * Payload ab, AIFF uebernimmt die Zahl aus COMM, klemmt sie aber an die real
 * folgende Datenmenge ein; bei Ogg stammt sie von ov_pcm_total() und bei MP3
 * aus einer Schaetzung des Parsers (für Ogg und MP3 gilt die obige Gleichung
 * daher nicht).
 *
 * @param[in]     file Geoeffnete Datei mit Audiodaten
 * @param[out]    info Bei true mit den Metadaten gefüllt
 * @return true = Format erkannt und unterstuetzt
 *
 * @exception pplib::FileNotOpenException @p file ist nicht geoeffnet.
 */
bool IdentAudioFile(FileObject& file, AudioInfo& info);

/** @brief Nur das Audioformat einer Datei ermitteln
 *
 * Wie IdentAudioFile(FileObject&, AudioInfo&), liefert aber ausschliesslich
 * den Formattyp. Unbekannt oder nicht unterstuetzt => AudioFormat::UNKNOWN.
 *
 * \desc
 * Das ist ein best-effort-Sniff auf die Kopfbytes, keine vollstaendige Prüfung:
 * ein erkanntes Format bedeutet nicht, dass der Parse-Pfad dieselbe Datei
 * akzeptiert (oder dass sie dekodierbar ist). Wer die Metadaten braucht, muss
 * die Variante mit AudioInfo-Parameter verwenden.
 *
 * @param[in] file Geoeffnete Datei mit Audiodaten
 * @return erkanntes Format, oder AudioInfo::AudioFormat::UNKNOWN
 * @exception pplib::FileNotOpenException @p file ist nicht geoeffnet.
 */
AudioInfo::AudioFormat IdentAudioFile(FileObject& file);

} // namespace pplib

#endif // PPLIB_AUDIO_AUDIOINFO_H