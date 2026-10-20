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

#ifndef PPLIB_AUDIO_MP3_H
#define PPLIB_AUDIO_MP3_H

#include <cstdint>

namespace pplib
{
// Vollstaendige Definitionen sind fuer die Nutzung der Funktionen erforderlich:
// #include <pplib/core/fileobject.h> und #include <pplib/core/bytearray.h>
class FileObject;
class ByteArray;

/** @brief Informationen zu den Audiodaten einer MPEG-Audiodatei (MP3)
 *
 * Alle Felder werden value-initialisiert (0, bzw. @c false), sodass ein
 * nicht gefuellter Header erkennbar bleibt.
 */
struct PPL_MPEG_HEADER
{
    uint64_t start = 0;      //!< Offset des ersten MPEG-Frames (nach ID3v2-Tag)
    uint64_t end = 0;        //!< Offset des letzten Bytes der Audiodaten (inklusive, vor ID3v1-Tag)
    uint64_t size = 0;       //!< Groesse der Audiodaten in Bytes (end - start + 1)
    uint64_t filesize = 0;   //!< Groesse der Datei in Bytes
    int32_t version = 0;     //!< MPEG-Version: 1 = MPEG-1, 2 = MPEG-2, 3 = MPEG-2.5, 0 = unbekannt
    int32_t layer = 0;       //!< Layer der Kodierung: 2 oder 3 (Layer I wird nicht erkannt)
    int32_t error_protection = 0; //!< gesetzt, wenn der Frame CRC-geschuetzt ist
    int32_t bitrate_index = 0;    //!< Bitrate-Index aus dem Frame-Header (1..14)
    int32_t bitrate = 0;          //!< Bitrate in kbit/s (bei VBR: Durchschnitt der geprüften Frames)
    int32_t frequency_index = 0;  //!< Frequenz-Index aus dem Frame-Header (0..2)
    int32_t frequency = 0;        //!< Abtastrate in Hz
    int32_t padding = 0;          //!< gesetztes Padding-Bit des Frames
    int32_t extension = 0;        //!< gesetztes Private-Bit des Frames
    int32_t mode = 0;             //!< Kanalmodus: 0 Stereo, 1 Joint-Stereo, 2 Dual-Channel, 3 Mono
    int32_t mode_ext = 0;         //!< Erweiterung des Kanalmodus
    int32_t copyright = 0;        //!< gesetztes Copyright-Bit
    int32_t original = 0;         //!< gesetztes Original-Bit
    int32_t emphasis = 0;         //!< Pre-Emphasis-Modus
    int32_t stereo = 0;           //!< Anzahl Kanäle (1 oder 2)
    int32_t framesize = 0;        //!< Groesse eines Frames in Bytes (inkl. Padding)
    int32_t frames = 0;           //!< Anzahl Frames der Datei (bei VBR: Anzahl geprüfter Frames)
    uint64_t mslength = 0;        //!< Abspielzeit in Millisekunden
    uint64_t length = 0;          //!< Abspielzeit in Sekunden
    uint64_t samples = 0;         //!< Anzahl Samples
    bool vbr = false;             //!< Datei nutzt variable Bitrate
};

/** @brief MPEG-Audiodaten (MP3) identifizieren und Header auswerten
 *
 * Die Datei wird am Anfang auf ein ID3v2-Tag untersucht, danach wird der erste
 * gueltige Frame-Header gesucht. Ein oder zwei ID3v1-Tags am Dateiende werden
 * von der Laenge der Audiodaten abgezogen. Anschliessend prueft CheckVBR(), ob
 * die Datei eine variable Bitrate nutzt.
 *
 * @param[in] file Die zu untersuchende Datei
 * @param[out] mpg Die Fuellungs-Informationen; @c nullptr ist erlaubt, dann
 *                 werden die Daten verworfen (nur Pruefung, ob MP3 vorliegt)
 * @retval true Die Datei enthaelt MPEG-Audiodaten
 * @retval false Kein MPEG-Frame gefunden; alle Felder sind zurueckgesetzt
 *
 * @note Es werden keine Exceptions geworfen.
 */
bool IdentMPEG(FileObject& file, PPL_MPEG_HEADER* mpg);

/** @brief Naechsten MPEG-Frame lesen
 *
 * Der Frame-Header wird ab der aktuellen Dateiposition gesucht. Nach successfullem
 * Lesen steht der Dateizeiger hinter dem gelesenen Frame, andernfalls an seiner
 * alten Position.
 *
 * @param[in,out] file Die Datei, aus der gelesen wird
 * @param[in,out] mpg Die von IdentMPEG gefuellten Frame-Informationen; wird um
 *                    die Werte des gefundenen Frames aktualisiert
 * @param[out] buffer Der Inhalt des Frames; bei Fehlschlag geleert
 *
 * @note Bei einer abgeschnittenen Datei wird kein Frame geliefert und keine
 *       Exception geworfen.
 */
void GetMP3Frame(FileObject& file, PPL_MPEG_HEADER& mpg, ByteArray& buffer);

} // namespace pplib

#endif // PPLIB_AUDIO_MP3_H
