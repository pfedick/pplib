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
#include <math.h>
#include <cstdint>
#include <config_pplib.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/mp3.h>
#include <pplib/core/functions.h>

namespace pplib
{

bool IdentOggVorbisAudioFile(FileObject& file, AudioInfo& info);

namespace
{

//!\brief Vergleich eines 4-Byte-Chunknamens (FourCC) ohne Heap-Allokation
bool fourcc(const char* adr, const char tag[4])
{
    return memcmp(adr, tag, 4) == 0;
}

//!\brief MPEG-Kanalmodus (2 Bit des Frame-Headers) auf ChannelMode abbilden
constexpr AudioInfo::ChannelMode mpegModeTBL[4] = {
    AudioInfo::ChannelMode::STEREO,        // 0
    AudioInfo::ChannelMode::JOINT_STEREO,  // 1
    AudioInfo::ChannelMode::DUAL_CHANNEL,  // 2
    AudioInfo::ChannelMode::MONO           // 3
};

/** @brief Liegt ein Ogg-Stream mit Vorbis-Codec vor?
 *
 * Der Ogg-Container ist codec-neutral: "OggS" allein bedeutet nur, dass irgendein
 * Stream in einem Ogg-Container vorliegt. Unterstuetzt ist hier ausschliesslich
 * Vorbis, dessen ID-Packet mit der Signatur "vorbis" in der ersten Kopfseite
 * hinter dem Seitenkopf (27 Bytes plus Segmentliste) steht. Opus, FLAC oder
 * unbekannte Codecs liefern damit UNKNOWN.
 */
bool isOggVorbis(FileObject& file, uint64_t fsize)
{
    //Reicht wegen moeglicher Vorab-Seiten (Continuation Flag) etwas groesszugigig
    //gepuffert; tryMap liefert nullptr, wenn die Datei kuerzer ist.
    const size_t probe = (fsize < 96) ? (size_t)fsize : 96;
    const char* adr = file.tryMap(0, probe);
    if (adr == nullptr) return false;
    for (uint64_t p = 27; p + 6 <= probe; p++) {
        if (memcmp(adr + p, "vorbis", 6) == 0) return true;
    }
    return false;
}

/** @brief 80-Bit-IEEE-754-Extended (big endian, 10 Bytes) plattformneutral dekodieren.
 *
 * AIFF kodiert die Sample-Rate in 80-Bit-Extended. Ein direktes Umcopieren der
 * Bytes in eine C++-long-double ist nicht portabel, da long double auf Windows,
 * ARM und FreeBSD i.d.R. nur 8 Byte belegt. Hier wird rein mit Ganzzahlen gerechnet.
 */
double readExtended80(const char* adr)
{
    const uint8_t* e = reinterpret_cast<const uint8_t*>(adr);
    uint64_t mantissa = 0;
    for (int i = 2; i < 10; i++)
        mantissa = (mantissa << 8) | e[i];
    int exponent = ((int)(e[0] & 0x7F) << 8) | e[1];
    double value = (double)mantissa * ldexp(1.0, exponent - 16383 - 63);
    return (e[0] & 0x80) ? -value : value;
}

bool IdentAIFF(FileObject& file, AudioInfo& info)
{
    const uint64_t fsize = file.size();

    info.Format = AudioInfo::AudioFormat::AIFF;
    info.IsVBR = false;
    info.FileSize = fsize;

    //Die FORM-Huelle (Bytes 0..11) ist keine eigenstaendige Chunkstufe der inneren
    //Schleife: die Innerchunks beginnen fest bei 12, ihr Ende ergibt sich aus der
    //Dateigroesse (allfaelliges Padding/Trailing am Dateiende wird toleriert)
    uint64_t p = 12;

    bool foundCOMM = false;
    bool foundSSND = false;
    uint64_t samples = 0;
    uint64_t frequency = 0;
    uint16_t channels = 0;
    uint16_t bitdepth = 0;
    uint64_t audioStart = 0;
    uint64_t audioSize = 0;

    while (p + 8 <= fsize) {
        const char* adr = file.tryMap(p, 8);
        if (adr == nullptr) break;
        uint32_t size = PeekN32(adr + 4);
        uint64_t payload = p + 8;
        uint64_t next = payload + size;

        if (fourcc(adr, "COMM") && size >= 18) {
            //18 Byte: Kanael (2) + Samples (4) + Bitdepth (2) + Sample-Rate als 80-Bit-Extended (10)
            if (payload + 18 > fsize) break;
            const char* comm = file.map(payload, 18);
            channels = PeekN16(comm);
            samples = PeekN32(comm + 2);
            bitdepth = PeekN16(comm + 6);
            double f = readExtended80(comm + 8);
            if (f > 0.0 && f <= (double)UINT32_MAX) frequency = (uint64_t)(f + 0.5);
            foundCOMM = true;
        } else if (fourcc(adr, "SSND") && size >= 8) {
            if (payload + 8 > fsize) break;
            const char* ssnd = file.map(payload, 8);
            uint32_t offset = PeekN32(ssnd);
            uint64_t dataStart = payload + 8 + offset;
            if (dataStart > fsize) break;
            audioStart = dataStart;
            //Deklarierte Chunkgroesse abzgl. 8 Byte SSND-Kopf, aber nie ueber das Dateiende
            uint64_t declared = size - 8;
            uint64_t rest = fsize - dataStart;
            audioSize = (declared > rest) ? rest : declared;
            foundSSND = true;
        } else if (fourcc(adr, "ID3 ") || fourcc(adr, "id3 ")) {
            info.HaveID3v2Tag = true;
            info.ID3v2TagStart = p + 8; // Anfang des ID3v2-Tags hinter dem Chunknamen
        }

        if (size & 1) next++; // Chunks werden an Word-Grenzen gepaddet
        if (p >= next) break; // Stillstand/Endlosschleife verhindern
        p = next;
    }

    if (!foundCOMM || !foundSSND || frequency == 0) return false;
    if (channels == 0 || channels > 2) return false; // nicht unterstuetzt

    if (bitdepth < 8 || (bitdepth / 8) * channels == 0) return false; // 20-Bit u.a. tolerieren
    uint8_t bytesPerSample = (uint8_t)((bitdepth / 8) * channels);
    if (bytesPerSample == 0) return false; // bitdepth so gross, dass die Kuerzung auf 0 fiele

    info.Samples = samples;
    info.BitsPerSample = (uint8_t)bitdepth;
    info.Channels = (uint8_t)channels;
    info.Mode = (channels == 1) ? AudioInfo::ChannelMode::MONO : AudioInfo::ChannelMode::STEREO;
    info.Frequency = (uint32_t)frequency;
    info.BytesPerSample = bytesPerSample;
    info.AudioStart = audioStart;
    info.AudioSize = audioSize;
    if (audioSize > 0) info.AudioEnd = info.AudioStart + audioSize - 1;
    if (samples > 0) info.Length = (uint32_t)((uint64_t)samples * 1000 / frequency);
    info.Bitrate = (uint16_t)((uint64_t)frequency * bytesPerSample * 8 / 1000);
    return true;
}

bool IdentWave(FileObject& file, AudioInfo& info)
{
    const uint64_t fsize = file.size();

    info.Format = AudioInfo::AudioFormat::WAVE;
    info.IsVBR = false;
    info.FileSize = fsize;

    bool foundFMT = false;
    bool foundDATA = false;
    uint32_t frequency = 0;
    uint16_t channels = 0;
    uint16_t bitdepth = 0;
    uint64_t declaredDataSize = 0;
    uint64_t audioStart = 0;

    //Chunk-Schleife: LIST-, fact- oder id3-Chunks zwischen fmt und data sind erlaubt
    uint64_t p = 12;
    while (p + 8 <= fsize) {
        const char* adr = file.tryMap(p, 8);
        if (adr == nullptr) break;
        size_t size = Peek32(adr + 4); // WAV ist Little Endian, hier kein PeekN32
        uint64_t payload = p + 8;
        uint64_t next = payload + size;

        if (fourcc(adr, "fmt ")) {
            if (size < 16 || payload + 16 > fsize) break;
            const char* fmt = file.map(payload, 16);
            channels = Peek16(fmt + 2);
            frequency = Peek32(fmt + 4);
            bitdepth = Peek16(fmt + 14);
            foundFMT = true;
        } else if (fourcc(adr, "data")) {
            declaredDataSize = size;
            audioStart = payload;
            foundDATA = true;
        } else if (fourcc(adr, "id3 ") || fourcc(adr, "ID3 ")) {
            info.HaveID3v2Tag = true;
            info.ID3v2TagStart = p + 8; // Anfang des ID3v2-Tags hinter dem Chunknamen
        }

        if (size & 1) next++; // Wort-Padding
        if (p >= next) break;
        p = next;
    }

    if (!foundFMT || !foundDATA) return false;
    if (channels == 0 || channels > 2) return false;
    if (bitdepth < 8 || (bitdepth / 8) * channels == 0) return false; // 20-Bit u.a. tolerieren
    if (frequency == 0) return false;

    uint8_t bytesPerSample = (uint8_t)((bitdepth / 8) * channels);
    if (bytesPerSample == 0) return false; // bitdepth so gross, dass die Kuerzung auf 0 fiele
    uint64_t maxAudioSize = fsize - audioStart;
    uint64_t audioSize = (declaredDataSize > maxAudioSize) ? maxAudioSize : declaredDataSize;
    audioSize -= audioSize % bytesPerSample;

    info.Channels = (uint8_t)channels;
    info.Mode = (channels == 1) ? AudioInfo::ChannelMode::MONO : AudioInfo::ChannelMode::STEREO;
    info.Frequency = frequency;
    info.BitsPerSample = (uint8_t)bitdepth;
    info.BytesPerSample = bytesPerSample;
    info.AudioStart = audioStart;
    info.AudioSize = audioSize;
    if (audioSize > 0) info.AudioEnd = info.AudioStart + audioSize - 1;
    info.Samples = audioSize / bytesPerSample;
    if (info.Samples > 0) info.Length = (uint32_t)(info.Samples * 1000 / frequency);
    info.Bitrate = (uint16_t)((uint64_t)frequency * bytesPerSample * 8 / 1000);
    return true;
}

bool IdentMP3(FileObject& file, AudioInfo& info, PPL_MPEG_HEADER& mp3)
{
    info.Format = AudioInfo::AudioFormat::MP3;
    info.IsVBR = mp3.vbr;
    info.FileSize = file.size();
    info.AudioStart = mp3.start;
    info.AudioEnd = mp3.end;
    info.AudioSize = (mp3.end >= mp3.start) ? (mp3.end - mp3.start + 1) : 0;
    info.Samples = mp3.samples;
    info.Channels = (uint8_t)mp3.stereo;
    info.Bitrate = (uint16_t)mp3.bitrate;
    info.Frequency = (uint32_t)mp3.frequency;
    info.BytesPerSample = (uint8_t)(mp3.stereo * 2);
    info.BitsPerSample = 16;
    info.Length = (uint32_t)mp3.mslength;
    //mp3.mode kommt aus 2 Bits des Frame-Headers, liegt also immer in 0..3
    info.Mode = mpegModeTBL[mp3.mode & 0x3];

    //MP3-Tags stehen immer am Dateianfang; ob einer vorliegt, steht in HaveID3v2Tag
    info.HaveID3v2Tag = false;
    info.ID3v2TagStart = 0;
    if (const char* adr = file.tryMap(0, 10)) {
        if (strncmp(adr, "ID3", 3) == 0) {
            //ID3v2 ab Version 2.2 akzeptieren, jede Revision (Major >= 2)
            if (Peek8(adr + 3) >= 2) info.HaveID3v2Tag = true;
        }
    }
    return true;
}

/** @brief Gemeinsames Format-Sniffing beider IdentAudioFile-Overloads.
 *
 * Ermittelt anhand der Headerbytes das Audioformat. Bei MP3 wird zusatzlich
 * (nur wenn \p mp3 non-null) der MPEG-Header geparst, da dessen Identifikation
 * ins Dateiinnere reicht und nicht aus den ersten 24 Bytes erkennbar ist.
 */
AudioInfo::AudioFormat DetectFormat(FileObject& file, PPL_MPEG_HEADER* mp3)
{
    const uint64_t fsize = file.size();
    //Mehrere harte 24-Byte-Zugriffe (RIFF/fmt, FORM/AIFF) folgen ungeprüft: bei
    //Dateien unter 24 Bytes ist das Muster schon per Definition ungueltig.
    if (fsize < 24) return AudioInfo::AudioFormat::UNKNOWN;

    const char* adr = file.tryMap(0, 24);
    if (adr == nullptr) return AudioInfo::AudioFormat::UNKNOWN;

    // AIFF: FORM-Groesse (+8 Header) darf das Dateiende nicht ueberschreiten;
    // Padding/Trailing-Bytes am Dateiende sind damit toleriert. AIFF-C ist bewusst
    // nicht eingeschlossen, es wird (noch) nicht dekodiert.
    if (fourcc(adr, "FORM") && (uint64_t)PeekN32(adr + 4) + 8 <= fsize && fourcc(adr + 8, "AIFF"))
        return AudioInfo::AudioFormat::AIFF;

    // RIFF / WAV (nur PCM, Format-Tag 1)
    if (fourcc(adr, "RIFF") && fourcc(adr + 8, "WAVE") && fourcc(adr + 12, "fmt ") && Peek16(adr + 20) == 1)
        return AudioInfo::AudioFormat::WAVE;

    // Ogg ist ein codec-neutraer Container: unterstuetzt ist hier ausschliesslich
    // Vorbis, Opus/FLAC bleiben UNKNOWN. Die MPEG-Suche entfaellt in dem Fall,
    // da sie in Ogg-Nutzdaten regelmaessig ein vorgetaeuschtes Frame-Sync-Wort findet.
    if (fourcc(adr, "OggS"))
        return isOggVorbis(file, fsize) ? AudioInfo::AudioFormat::OGG : AudioInfo::AudioFormat::UNKNOWN;

    PPL_MPEG_HEADER local;
    PPL_MPEG_HEADER* target = mp3 ? mp3 : &local;
    if (IdentMPEG(file, target)) return AudioInfo::AudioFormat::MP3;

    return AudioInfo::AudioFormat::UNKNOWN;
}

} // namespace

bool IdentAudioFile(FileObject& file, AudioInfo& info)
{
    PPL_MPEG_HEADER mp3;
    AudioInfo::AudioFormat format = DetectFormat(file, &mp3);
    if (format == AudioInfo::AudioFormat::UNKNOWN) return false;

    // info wird erst im Erfolgsfall veroeffentlicht, damit der Aufrufer bei false
    // kein teilbefuelltes Objekt vorfindet
    AudioInfo result;
    bool ok = false;
    switch (format) {
    case AudioInfo::AudioFormat::AIFF:
        ok = IdentAIFF(file, result);
        break;
    case AudioInfo::AudioFormat::WAVE:
        ok = IdentWave(file, result);
        break;
    case AudioInfo::AudioFormat::MP3:
        ok = IdentMP3(file, result, mp3);
        break;
    case AudioInfo::AudioFormat::OGG:
        ok = IdentOggVorbisAudioFile(file, result);
        break;
    default:
        break;
    }
    if (!ok) return false;
    info = result;
    return true;
}

AudioInfo::AudioFormat IdentAudioFile(FileObject& file)
{
    return DetectFormat(file, nullptr);
}

} // namespace pplib
