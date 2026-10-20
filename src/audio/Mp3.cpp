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
#include <algorithm>
#include <array>
#include <cstdint>
#include <config_pplib.h>
#include <pplib/core/file.h>
#include <pplib/exceptions.h>
#include <pplib/audio/mp3.h>

// Einkommentieren, um Debug-Ausgaben der MP3-Auswertung auf stdout zu aktivieren
// #define ID3DEBUG

#ifdef ID3DEBUG
#include <cstdio>
#include <cinttypes>
#define MP3DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define MP3DEBUG_PRINT(...) \
    do {                    \
    } while (0)
#endif

namespace pplib
{

static void CheckVBR(FileObject& file, PPL_MPEG_HEADER* mpg);
static int64_t FindNextHeader(FileObject& file, int64_t pos, PPL_MPEG_HEADER* mpg);

namespace
{

// Modus-Werte des Header-Bytes 4 (Bits 7-6)
constexpr int MPG_MD_MONO = 3;

// Scan-Fenster beim Absuchen nach Frame-Headern
constexpr uint64_t SCAN_WINDOW = 1024;

// CheckVBR bricht die Suche nach Bitratenwechseln nach dieser Anzahl Frames ab
constexpr uint64_t VBR_SCAN_LIMIT = 50;

// Index = Versionsbits des Headers (Bits 4-3 von Byte 1), Wert = MPEG-Version:
// 00=MPEG-2.5 (3), 01=reserviert (0), 10=MPEG-2 (2), 11=MPEG-1 (1)
constexpr std::array<int, 4> MPEGVersion{3, 0, 2, 1};

// Bitraten in kbit/s, indexiert als [version - 1][layer - 1][bitrate_index].
// Die Schicht Layer I (Index 0) wird von FindNextHeader nicht akzeptiert.
constexpr std::array<std::array<std::array<int, 15>, 3>, 3> MP3Bitrates{
    {{{{0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448},
       {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384},
       {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320}}},
     {{{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256},
       {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160},
       {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}}},
     {{{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256},
       {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160},
       {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}}}}};

// Abtastraten in Hz, indexiert als [version - 1][frequency_index]
constexpr std::array<std::array<int, 4>, 3> MPEGFrequencies{
    {{44100, 48000, 32000, 0}, {22050, 24000, 16000, 0}, {11025, 8000, 8000, 0}}};

//!\brief Samples pro Frame. Layer I (384) wird von FindNextHeader nicht erkannt.
constexpr int samplesPerFrame(int version)
{
    return (version == 1) ? 1152 : 576; // MPEG-1 : MPEG-2/2.5
}

//!\brief Groesse eines MPEG-Frames in Bytes, das Padding-Bit wird addiert
constexpr int computeFrameSize(int bitrate, int frequency, int padding)
{
    if (bitrate <= 0 || frequency <= 0) return 0;
    return 144 * bitrate * 1000 / frequency + padding;
}

/** @brief Groesse eines ID3v2-Tags am Dateianfang ermitteln
 *
 * @param[in] hdr Die ersten 10 Bytes der Datei
 * @return Gesamte Laenge des Tags inkl. 10-Byte-Header, 0 wenn kein gueltiges
 *         ID3v2-Tag erkannt wurde.
 */
uint64_t getId3v2TagSize(const unsigned char* hdr)
{
    // 0x49 0x44 0x33 vv ff rr sz sz sz sz
    if (hdr[0] != 'I' || hdr[1] != 'D' || hdr[2] != '3') return 0;
    const int version = hdr[3];
    // Unterstuetzt werden ID3v2.2 bis ID3v2.4; die Reservebits (0xE0) muessen 0 sein
    if (version < 2 || version > 4) return 0;
    // Nur die fuer die Version definierten Flag-Bits duerfen gesetzt sein:
    // v2.2 keine, v2.3 0x80/0x40/0x20, v2.4 0x80/0x40/0x20/0x01
    const int undefinedFlags = (version == 2) ? 0xFF : (version == 3 ? 0x1F : 0x1E);
    if (hdr[4] & undefinedFlags) return 0;
    if (version == 2) {
        // ID3v2.2: Groesse als 3 Byte unsynchsafe
        return ((uint64_t)hdr[6] << 16) + ((uint64_t)hdr[7] << 8) + hdr[8] + 10;
    }
    // ID3v2.3/2.4: Groesse synchsafe, jedes Byte < 0x80
    if (hdr[6] >= 0x80 || hdr[7] >= 0x80 || hdr[8] >= 0x80 || hdr[9] >= 0x80) return 0;
    return ((uint64_t)hdr[6] << 21) + ((uint64_t)hdr[7] << 14) + ((uint64_t)hdr[8] << 7) + hdr[9] + 10;
}

/** @brief Datei in den Speicher mappen, ohne Exceptions zuzulassen
 *
 * @return Pointer auf das Mapping oder @c nullptr, wenn der Bereich nicht
 *         gemappt werden kann.
 */
const char* mapNoThrow(FileObject& file, uint64_t position, size_t bytes)
{
    try {
        return file.map(position, bytes);
    }
    catch (...) {
        return nullptr;
    }
}

} // namespace

bool IdentMPEG(FileObject& file, PPL_MPEG_HEADER* mpg)
{
    PPL_MPEG_HEADER local;
    if (!mpg) mpg = &local;
    *mpg = PPL_MPEG_HEADER{};

    mpg->filesize = file.size();
    if (mpg->filesize == 0) return false;

    // Beginnt die Datei mit einem ID3v2-Tag, beginnen die Audiodaten dahinter
    mpg->start = 0;
    if (mpg->filesize >= 10) {
        const char* buffer = mapNoThrow(file, 0, 10);
        if (buffer) {
            const uint64_t tagsize = getId3v2TagSize(reinterpret_cast<const unsigned char*>(buffer));
            if (tagsize > 10 && tagsize < mpg->filesize) mpg->start = tagsize;
        }
    }

    // Jetzt den ersten MP3-Header suchen
    const int64_t p = FindNextHeader(file, static_cast<int64_t>(mpg->start), mpg);
    if (p < 0) {
        *mpg = PPL_MPEG_HEADER{};
        return false;
    }
    mpg->start = static_cast<uint64_t>(p);

    // Ende der Audiodaten: ein oder zwei ID3v1-Tags am Dateiende abziehen.
    // Ein Tag belegt 128 Bytes und beginnt jeweils 127 Bytes vor dem aktuellen Ende.
    mpg->end = mpg->filesize - 1;
    for (int i = 0; i < 2; i++) {
        if (mpg->end < 128) break;
        const char* tail = mapNoThrow(file, mpg->end - 127, 127);
        if (!tail) break;
        if (tail[0] != 'T' || tail[1] != 'A' || tail[2] != 'G') break;
        mpg->end -= 128;
    }

    mpg->size = (mpg->end >= mpg->start) ? (mpg->end - mpg->start + 1) : 0;
    if (mpg->size > mpg->filesize) mpg->size = mpg->filesize;

    mpg->framesize = computeFrameSize(mpg->bitrate, mpg->frequency, mpg->padding);
    if (mpg->framesize) mpg->frames = static_cast<int32_t>(mpg->size / (uint64_t)mpg->framesize);
    if (mpg->bitrate > 0) {
        mpg->mslength = mpg->size * 8 / (uint64_t)mpg->bitrate;
        mpg->length = mpg->mslength / 1000;
    }
    mpg->samples = (uint64_t)((double)mpg->frequency * (double)mpg->mslength / 1000.0);

    CheckVBR(file, mpg);

    MP3DEBUG_PRINT("   MPEG-Kennung bei %" PRIu64 ": MPEG %d, Layer %d, %" PRId32 " kbit/s, %d Hz\n", mpg->start, mpg->version,
                   mpg->layer, mpg->bitrate, mpg->frequency);
    MP3DEBUG_PRINT("   Framesize %d, Frames %" PRId32 ", %" PRIu64 " ms, %" PRIu64 " Samples, vbr=%d\n", mpg->framesize,
                   mpg->frames, mpg->mslength, mpg->samples, mpg->vbr ? 1 : 0);
    return true;
}

void GetMP3Frame(FileObject& file, PPL_MPEG_HEADER& mpg, ByteArray& buffer)
{
    const uint64_t oldpos = file.tell();
    auto noFrame = [&]() {
        buffer.clear();
        file.seek(oldpos);
    };

    // Vorbedingung: end und filesize wurden von IdentMPEG gesetzt. Ist die
    // Struktur leer, werden die Werte von der Datei uebernommen.
    const uint64_t actual = file.size();
    if (mpg.filesize == 0) {
        mpg.filesize = actual;
        mpg.end = actual ? actual - 1 : 0;
    }
    // Ein veraltetes Ende der Struktur darf nicht ueber die Datei hinauszeigen
    const uint64_t dataEnd = std::min<uint64_t>(mpg.end, actual ? actual - 1 : 0);
    if (oldpos > dataEnd || oldpos > (uint64_t)INT64_MAX) {
        noFrame();
        return;
    }

    const int64_t pos = FindNextHeader(file, static_cast<int64_t>(oldpos), &mpg);
    // Ein Header jenseits der Audiodaten (z. B. Byte-Muster im ID3v1-Tag) ist kein Frame
    if (pos < 0 || (uint64_t)pos > dataEnd) {
        noFrame();
        return;
    }
    mpg.start = (uint64_t)pos;

    // Die Framesize darf das Datenende (und damit ID3v1-Tags) nicht ueberschreiten
    const uint64_t available = dataEnd - mpg.start + 1;
    const size_t readsize = (size_t)std::min<uint64_t>((uint64_t)mpg.framesize, available);
    size_t got = 0;
    try {
        got = file.read(buffer, readsize);
    }
    catch (const EndOfFileException&) {
        // Abgeschnittene Datei: kein Frame liefern
        got = 0;
    }
    if (got != readsize) {
        // Nie ein unvollstaendiges Frame ausliefern
        noFrame();
        return;
    }

    MP3DEBUG_PRINT("   Frame bei %" PRIu64 ": %zu Bytes\n", mpg.start, readsize);
#ifdef ID3DEBUG
    buffer.hexDump();
#endif
    file.seek(std::min<uint64_t>(mpg.start + (uint64_t)mpg.framesize, actual));
}

/** @brief Naechsten gueltigen MPEG-Frame-Header finden
 *
 * Die Struktur \p mpg wird nur aktualisiert, wenn ein gueltiger Header gefunden
 * wurde. Bei Fehlschlag bleibt sie unveraendert und die Funktion liefert -1.
 */
static int64_t FindNextHeader(FileObject& file, int64_t pos, PPL_MPEG_HEADER* mpg)
{
    if (pos < 0) return -1;
    const uint64_t filesize = file.size();
    uint64_t searchpos = (uint64_t)pos;

    while (searchpos + 4 <= filesize) {
        uint64_t mapsize = filesize - searchpos;
        if (mapsize > SCAN_WINDOW) mapsize = SCAN_WINDOW;
        const char* window = mapNoThrow(file, searchpos, (size_t)mapsize);
        if (!window) return -1;

        for (uint64_t p = 0; p + 4 <= mapsize; p++) {
            const unsigned char* header = reinterpret_cast<const unsigned char*>(window) + p;
            // 11 Bit Sync: aaaaaaaaaaabbccdeeeeff
            //  a=Sync, b=Version, c=Layer, d=Protection, e=Bitrate-Index, f=Frequenz-Index
            if (header[0] != 0xFF || (header[1] & 0xE0) != 0xE0) continue;

            PPL_MPEG_HEADER h = *mpg;
            h.version = MPEGVersion[(header[1] >> 3) & 0x3];
            h.layer = 4 - ((header[1] >> 1) & 0x3);
            h.bitrate_index = (header[2] >> 4) & 0x0F;
            h.frequency_index = (header[2] >> 2) & 0x3;
            // 0 und 15 sind als Bitrate-Index nicht erlaubt, 3 als Frequenz-Index nicht erlaubt
            if (h.version == 0 || h.layer < 2 || h.layer > 3 || h.bitrate_index == 0 || h.bitrate_index == 15 ||
                h.frequency_index == 3)
                continue;

            h.start = searchpos + p;
            h.bitrate = MP3Bitrates[h.version - 1][h.layer - 1][h.bitrate_index];
            h.frequency = MPEGFrequencies[h.version - 1][h.frequency_index];
            h.error_protection = !(header[1] & 0x1);
            h.padding = (header[2] >> 1) & 0x01;
            h.extension = header[2] & 0x01;
            h.mode = (header[3] >> 6) & 0x3;
            h.mode_ext = (header[3] >> 4) & 0x3;
            h.copyright = (header[3] >> 3) & 0x1;
            h.original = (header[3] >> 2) & 0x1;
            h.emphasis = header[3] & 0x3;
            h.stereo = (h.mode == MPG_MD_MONO) ? 1 : 2;
            h.framesize = computeFrameSize(h.bitrate, h.frequency, h.padding);
            h.vbr = false;
            h.size = (h.end >= h.start) ? (h.end - h.start + 1) : 0;
            if (h.size > h.filesize) h.size = h.filesize;
            h.frames = h.framesize ? (int32_t)(h.size / (uint64_t)h.framesize) : 0;
            h.mslength = h.bitrate ? (h.size * 8 / (uint64_t)h.bitrate) : 0;
            h.length = h.mslength / 1000;
            h.samples = (uint64_t)((double)h.frequency * (double)h.mslength / 1000.0);
            // vbr bleibt unveraendert: CheckVBR setzt das Flag, nicht der Header

            *mpg = h;
            return (int64_t)h.start;
        }
        // Die letzten 3 Bytes des Fensters koennen den Anfang eines Headers bilden,
        // der ueber das Fenster hinausragt
        searchpos += mapsize - 3;
    }
    return -1;
}

/** @brief Prüfen, ob die Datei eine variable Bitrate besitzt
 *
 * Dazu werden die Bitraten der folgenden Frames verglichen. Bei langen Dateien
 * wird die Suche nach VBR_SCAN_LIMIT Frames ohne Bitratenwechsel abgebrochen.
 */
static void CheckVBR(FileObject& file, PPL_MPEG_HEADER* mpg)
{
    PPL_MPEG_HEADER m;
    m.start = mpg->start;
    m.end = mpg->end;
    m.size = mpg->size;
    m.filesize = mpg->filesize;

    uint64_t frames = 0;
    uint64_t avgbitrate = 0;
    int lastbitrate = mpg->bitrate;
    mpg->vbr = false;

    try {
        file.setMapReadAhead(1024 * 1024); // 1 MB Read-Cache
    }
    catch (const UnimplementedVirtualFunctionException&) {
        // Nicht alle FileObject-Subklassen unterstuetzen Read-Ahead
    }

    int64_t pos = FindNextHeader(file, (int64_t)mpg->start, &m);
    if (pos < 0) return;
    while ((uint64_t)pos < mpg->end) {
        frames++;
        avgbitrate += m.bitrate;
        if (lastbitrate != m.bitrate) {
            mpg->vbr = true;
            lastbitrate = m.bitrate;
        }
        if (frames > VBR_SCAN_LIMIT && !mpg->vbr) break;
        while (true) {
            pos += m.framesize;
            pos = FindNextHeader(file, pos, &m);
            if ((pos < 0) || ((uint64_t)pos > mpg->end)) break;
            if (m.layer == mpg->layer && m.version == mpg->version) break;
            // Frame mit anderer Version oder Schicht: nur die Headerfelder
            // uebernehmen, start/end/size und das vbr-Flag bleiben erhalten
            mpg->version = m.version;
            mpg->layer = m.layer;
            mpg->bitrate = m.bitrate;
            mpg->bitrate_index = m.bitrate_index;
            mpg->frequency = m.frequency;
            mpg->frequency_index = m.frequency_index;
            mpg->mode = m.mode;
            mpg->stereo = m.stereo;
            mpg->framesize = m.framesize;
            lastbitrate = m.bitrate;
            avgbitrate = m.bitrate;
            frames = 0;
        }
        if ((pos < 0) || ((uint64_t)pos > mpg->end)) break;
    }

    if (mpg->vbr && frames > 0) {
        const uint64_t samples = frames * (uint64_t)samplesPerFrame(mpg->version);
        mpg->frames = (int32_t)frames;
        mpg->samples = samples;
        if (mpg->frequency > 0) {
            mpg->mslength = samples * 1000 / (uint64_t)mpg->frequency;
            mpg->length = samples / (uint64_t)mpg->frequency;
        }
        mpg->bitrate = (int32_t)(avgbitrate / frames);
    }
}

} // namespace pplib
