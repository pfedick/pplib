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

// Modulinterne Deklaration, bewusst ohne Header: die Definition liegt in
// src/audio/AudioDecoder_Ogg.cpp und ist nicht Teil der offenen API.
// Die Signatur muss mit der dortigen Definition uebereinstimmen.
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

/** @brief Unkomprimierte Bitrate in kbit/s, begrenzt auf den Wertebereich von AudioInfo::Bitrate
 *
 * Frequenzen bis UINT32_MAX * 8 Byte * 8 Bit ueberschreiten uint16_t deutlich, deshalb wird
 * in uint64_t gerechnet und nach oben begrenzt (Typ ANDEN AudioInfo::Bitrate bleibt uint16_t).
 */
static uint16_t ToKbitRate(uint64_t frequency, uint64_t bytesPerFrame)
{
    const uint64_t kbit = frequency * bytesPerFrame * 8 / 1000;
    return (uint16_t)((kbit > UINT16_MAX) ? UINT16_MAX : kbit);
}

bool IdentAIFF(FileObject& file, AudioInfo& info)
{
    const uint64_t fsize = file.size();

    info.Format = AudioInfo::AudioFormat::AIFF;
    info.IsVBR = false;
    info.SampleType = AudioInfo::SampleEncoding::INTEGER;
    info.FileSize = fsize;

    //Der FORM-Subtyp entscheidet, ob klassisches AIFF oder AIFF-C vorliegt. Beide
    //werden unterstuetzt, AIFF-C allerdings nur unkomprimiert bzw. als 32-Bit-Float.
    if (fsize < 12) return false;
    const char* subtype = file.tryMap(8, 4);
    if (subtype == nullptr) return false;
    const bool isAIFC = fourcc(subtype, "AIFC");
    if (!isAIFC && !fourcc(subtype, "AIFF")) return false;

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
    uint32_t commSize = 0;
    char compType[4] = {'N', 'O', 'N', 'E'};  // klassisches AIFF gilt als unkomprimiert
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
            //AIFF-C haengt zusaetzlich einen 4-Byte-Kompressionstyp an (Comm-Groesse >= 22)
            const size_t headSize = (size >= 22) ? 22 : 18;
            if (payload + headSize > fsize) break;
            const char* comm = file.map(payload, headSize);
            channels = PeekN16(comm);
            samples = PeekN32(comm + 2);
            bitdepth = PeekN16(comm + 6);
            double f = readExtended80(comm + 8);
            if (f > 0.0 && f <= (double)UINT32_MAX) frequency = (uint64_t)(f + 0.5);
            if (headSize >= 22)
                for (int i = 0; i < 4; i++) compType[i] = comm[18 + i];
            commSize = size;
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
    if (bitdepth < 8 || bitdepth > 32) return false; // Obergrenze: >32 Bit wird nicht unterstuetzt

    //AIFF-C traegt einen Kompressionstyp. Dekodiert wird bewusst nur das Unkomprimierte
    //(`NONE`) sowie 32-Bit-Fliesskomma (`FL32`/`fl32`, Big Endian). Kompressionen wie
    //`sowt`, `ulaw`, `alaw`, `adpc` und 64-Bit-Float bleiben ununterstuetzt.
    if (isAIFC) {
        if (commSize < 22) return false; // AIFF-C ohne Kompressionstyp nicht beschreibbar
        if (fourcc(compType, "FL32") || fourcc(compType, "fl32")) {
            if (bitdepth != 32) return false;
            info.SampleType = AudioInfo::SampleEncoding::FLOAT;
        } else if (!fourcc(compType, "NONE")) {
            return false;
        }
    }

    //Bytes pro Sample-Frame (= ein Sample je Kanal). AIFF packt jedes Kanal-Sample in einen
    //auf volle Bytes aufgerundeten Behaelter, also aufgerundete Bytes je Kanal mal Kanaele
    //(20 bit stereo -> 2 x 3 = 6). Durch bitdepth <= 32 und channels <= 2 bleibt uint8_t (max. 8).
    uint8_t bytesPerSample = (uint8_t)(((bitdepth + 7) / 8) * channels);

    //Der Sample-Frame ist die kleinste abspielbare Einheit: angehaengtes Halb-sample am
    //Chunk-Ende abschneiden und die deklarierte Samplezahl an die real folgende Datenmenge anpassen
    audioSize -= audioSize % bytesPerSample;
    if (audioSize == 0) return false; // ohne abspielbare Samples kein gueltiges Audio
    const uint64_t frames = audioSize / bytesPerSample;
    if (samples > frames) samples = frames;

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
    info.Bitrate = ToKbitRate(frequency, bytesPerSample);
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
    uint16_t formatTag = 0;
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
            formatTag = Peek16(fmt);  // 1 = PCM, 3 = IEEE-Float, 0xFFFE = EXTENSIBLE
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
    if (formatTag != 1 && formatTag != 3) return false;  // nur PCM und 32-Bit-IEEE-Float
    if (formatTag == 3 && bitdepth != 32) return false;  // Float gibt es nur als 32 Bit
    if (channels == 0 || channels > 2) return false;
    if (bitdepth < 8 || bitdepth > 32) return false; // Obergrenze: >32 Bit wird nicht unterstuetzt
    if (frequency == 0) return false;

    //Bytes pro Sample-Frame, analog zu IdentAIFF und wie nBlockAlign des fmt-Chunks: je Kanal
    //auf volle Bytes aufgerundet, mal Kanaele; durch die Grenzen passt der Wert stets in uint8_t
    uint8_t bytesPerSample = (uint8_t)(((bitdepth + 7) / 8) * channels);
    uint64_t maxAudioSize = fsize - audioStart;
    uint64_t audioSize = (declaredDataSize > maxAudioSize) ? maxAudioSize : declaredDataSize;
    audioSize -= audioSize % bytesPerSample;
    if (audioSize == 0) return false; // leerer data-Chunk ist nicht abspielbar

    info.Channels = (uint8_t)channels;
    info.Mode = (channels == 1) ? AudioInfo::ChannelMode::MONO : AudioInfo::ChannelMode::STEREO;
    info.Frequency = frequency;
    info.BitsPerSample = (uint8_t)bitdepth;
    info.SampleType = (formatTag == 3) ? AudioInfo::SampleEncoding::FLOAT : AudioInfo::SampleEncoding::INTEGER;
    info.BytesPerSample = bytesPerSample;
    info.AudioStart = audioStart;
    info.AudioSize = audioSize;
    if (audioSize > 0) info.AudioEnd = info.AudioStart + audioSize - 1;
    info.Samples = audioSize / bytesPerSample;
    if (info.Samples > 0) info.Length = (uint32_t)(info.Samples * 1000 / frequency);
    info.Bitrate = ToKbitRate(frequency, bytesPerSample);
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

/** @brief Groesse eines ID3v2-Tags am Dateianfang ermitteln
 *
 * Ein ID3v2-Tag vor den Audiodaten ist bei MP3 üblich und wird von manchen
 * Exportwerkzeugen auch vor eine FLAC-Datei gesetzt, obwohl die FLAC-Spec das nicht
 * vorsieht. Für die Erkennung ist nur die Größe relevant, weil sie den Anfang der
 * Audiodaten verschiebt.
 *
 * @return Byteanzahl des Tags inklusive 10 Byte Kopf, 0 wenn kein ID3v2-Tag am Anfang steht
 */
uint64_t ID3v2TagSize(FileObject& file, uint64_t fsize)
{
    const char* adr = file.tryMap(0, 10);
    if (adr == nullptr) return 0;
    if (memcmp(adr, "ID3", 3) != 0) return 0;
    if (Peek8(adr + 3) < 2) return 0; // vor ID3v2.2 gibt es kein Größenfeld in diesem Format
    //Die Tag-Größe ist ein Synchsafe Integer: 4 x 7 Bit, der Kopf ist nicht mitgezählt
    const uint64_t size = ((uint64_t)Peek8(adr + 6) << 21) | ((uint64_t)Peek8(adr + 7) << 14)
                        | ((uint64_t)Peek8(adr + 8) << 7) | (uint64_t)Peek8(adr + 9);
    if (size + 10 + 4 > fsize) return 0; // Tag muss vollständig in der Datei liegen
    return size + 10;
}

/** @brief Bitfeld eines Big-Endian-Bitstroms lesen
 *
 * FLAC packt seine Metadata-Felder bitgenau und nicht byte-aligniert. @p start und
 * @p count zählen vom MSB von @p adr aus; @p count bleibt immer <= 36, das Ergebnis
 * passt also stets in uint64_t.
 */
uint64_t PeekBits(const char* adr, uint32_t start, uint32_t count)
{
    uint64_t value = 0;
    for (uint32_t i = 0; i < count; i++) {
        const uint32_t pos = start + i;
        const uint8_t byte = (uint8_t)adr[pos / 8];
        value = (value << 1) | (uint64_t)((byte >> (7 - (pos % 8))) & 1);
    }
    return value;
}

/** @brief FLAC-Datei identifizieren und aus dem STREAMINFO-Block beschreiben
 *
 * Ein FLAC-Stream besteht aus dem Magic `fLaC`, einer Folge von Metadata-Blöcken und
 * danach den komprimierten Frames. Der STREAMINFO-Block muss der erste Block sein; er
 * enthält Sample-Rate, Kanalzahl, Bitbreite und die Gesamtzahl Samples. Alle übrigen
 * Blocktypen (PADDING, SEEKTABLE, VORBIS_COMMENT, PICTURE ...) werden übersprungen,
 * weil die AudioInfo keine Tag-Inhalte abbildet.
 *
 * @return true = FLAC erkannt und unterstuetzt
 */
bool IdentFLAC(FileObject& file, AudioInfo& info)
{
    const uint64_t fsize = file.size();

    info.Format = AudioInfo::AudioFormat::FLAC;
    info.IsVBR = false;
    info.SampleType = AudioInfo::SampleEncoding::INTEGER; // Flieskomm-Samples gibt es in FLAC nicht
    info.FileSize = fsize;

    const uint64_t tagsize = ID3v2TagSize(file, fsize);
    if (tagsize != 0) {
        info.HaveID3v2Tag = true;
        info.ID3v2TagStart = 0; // wie bei MP3: das Tag beginnt immer am Dateianfang
    }

    uint32_t frequency = 0;
    uint32_t channels = 0;
    uint32_t bitdepth = 0;
    uint64_t declaredSamples = 0;
    uint64_t maxBlocksize = 4096;
    uint64_t p = tagsize + 4; // hinter dem "fLaC"-Magic
    bool haveSTREAMINFO = false;
    bool lastBlock = false;

    //Block-Schleife: der Block mit gesetztem last-Bit (Bit 7) schliesst die Metadata ab
    while (p + 4 <= fsize) {
        const char* hdr = file.tryMap(p, 4);
        if (hdr == nullptr) return false;
        lastBlock = (Peek8(hdr) & 0x80) != 0;
        const uint8_t type = (uint8_t)Peek8(hdr) & 0x7F;
        //Blocklänge: 3 Byte big endian, also bis 16 MByte je Block
        const uint64_t size = ((uint64_t)Peek8(hdr + 1) << 16) | ((uint64_t)Peek8(hdr + 2) << 8)
                            | (uint64_t)Peek8(hdr + 3);
        const uint64_t payload = p + 4;
        if (payload + size > fsize) return false; // Block reicht über das Dateiende

        if (type == 0) {
            //STREAMINFO ist Pflicht, muss der erste Block sein und ist immer 34 Bytes groß
            if (haveSTREAMINFO || payload + 34 > fsize || p != tagsize + 4 || size != 34) return false;
            const char* si = file.map(payload, 34);
            maxBlocksize    = PeekBits(si, 16, 16);  // maximale Blockgröße in Samples
            frequency       = (uint32_t)PeekBits(si, 80, 20);
            channels        = (uint32_t)PeekBits(si, 100, 3) + 1;
            bitdepth        = (uint32_t)PeekBits(si, 103, 5) + 1;
            declaredSamples = PeekBits(si, 108, 36);
            haveSTREAMINFO  = true;
        }

        p = payload + size;
        if (lastBlock) break;
    }
    if (!haveSTREAMINFO || !lastBlock) return false;

    if (frequency == 0) return false;
    //Kanaldichte und Bitbreite sind reine STREAMINFO-Fakten und werden gemeldet, auch wenn
    //pplib dafuer keinen Decoder hat (8/32 Bit, 5.1). Erkennung und Unterstuetzung sind
    //getrennt; abgelehnt wird erst in AudioDecoder_Flac::open().

    //Die Frames beginnen direkt hinter dem letzten Metadata-Block. Ein ID3v1-Tag am
    //Dateiende (128 Bytes, Magic "TAG") zählt nicht zu den Audiodaten.
    const uint64_t audioStart = p;
    uint64_t audioStop = fsize;
    if (fsize >= audioStart + 128) {
        const char* id3v1 = file.tryMap(fsize - 128, 3);
        if (id3v1 != nullptr && memcmp(id3v1, "TAG", 3) == 0) audioStop = fsize - 128;
    }
    const uint64_t audioSize = audioStop - audioStart;
    if (audioSize == 0) return false;

    //Die Samplezahl steht in 36 Bit und ist bei Stream-Aufnahmen 0. Nach oben wird sie an
    //die größtmögliche Zahl übertragener Samples geklemmt: jeder Frame überträgt höchstens
    //maxBlocksize Samples und kostet mindestens 1 Byte. Das ist eine Plausibilitätsgrenze,
    //keine exakte Grenze, weil FLAC komprimiert.
    uint64_t samples = declaredSamples;
    const uint64_t maxSamples = ((maxBlocksize == 0) ? 4096 : maxBlocksize) * audioSize;
    if (samples > maxSamples) samples = maxSamples;

    uint8_t bytesPerSample = (uint8_t)(((bitdepth + 7) / 8) * channels);

    info.Frequency = frequency;
    info.Channels = (uint8_t)channels;
    info.Mode = (channels == 1) ? AudioInfo::ChannelMode::MONO : AudioInfo::ChannelMode::STEREO;
    info.BitsPerSample = (uint8_t)bitdepth;
    info.BytesPerSample = bytesPerSample;
    info.AudioStart = audioStart;
    info.AudioSize = audioSize;
    info.AudioEnd = audioStart + audioSize - 1;
    info.Samples = samples;

    //Länge und Bitrate folgen aus der Samplezahl; bei unbekannter Samplezahl (STREAMINFO 0)
    //bleiben beide 0, der Decoder nutzt dann die von libFLAC gemeldete Laufzeitposition.
    if (samples > 0) {
        uint64_t ms = (uint64_t)samples * 1000 / frequency;
        info.Length = (uint32_t)((ms > UINT32_MAX) ? UINT32_MAX : ms);
        const uint64_t kbit = (audioSize * 8 / 1000) * frequency / samples;
        info.Bitrate = (uint16_t)((kbit > UINT16_MAX) ? UINT16_MAX : kbit);
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
    // Padding/Trailing-Bytes am Dateiende sind damit toleriert. AIFF-C (`AIFC`) ist
    // eingeschlossen, IdentAIFF() weist darin aber jede Kompression ausser `NONE`/`FL32` ab.
    if (fourcc(adr, "FORM") && (uint64_t)PeekN32(adr + 4) + 8 <= fsize
        && (fourcc(adr + 8, "AIFF") || fourcc(adr + 8, "AIFC")))
        return AudioInfo::AudioFormat::AIFF;

    // RIFF / WAV (PCM Tag 1 und IEEE-Float Tag 3). IdentWave() prueft die Tag-Bitbreiten-Kombination
    if (fourcc(adr, "RIFF") && fourcc(adr + 8, "WAVE") && fourcc(adr + 12, "fmt ")
        && (Peek16(adr + 20) == 1 || Peek16(adr + 20) == 3))
        return AudioInfo::AudioFormat::WAVE;

    // Ogg ist ein codec-neutraer Container: unterstuetzt ist hier ausschliesslich
    // Vorbis, Opus/FLAC bleiben UNKNOWN. Die MPEG-Suche entfaellt in dem Fall,
    // da sie in Ogg-Nutzdaten regelmaessig ein vorgetaeuschtes Frame-Sync-Wort findet.
    if (fourcc(adr, "OggS"))
        return isOggVorbis(file, fsize) ? AudioInfo::AudioFormat::OGG : AudioInfo::AudioFormat::UNKNOWN;

    // FLAC: Das "fLaC"-Magic sitzt am Dateianfang oder direkt hinter einem ID3v2-Tag, das
    // manche Exportwerkzeuge vor eine FLAC-Datei setzen. Vor der MPEG-Suche pruefen: die
    // findet in komprimierten FLAC-Nutzdaten regelmaessig eine vorgetaeuschte Frame-Sync.
    if (const char* flacadr = file.tryMap(ID3v2TagSize(file, fsize), 4)) {
        if (fourcc(flacadr, "fLaC")) return AudioInfo::AudioFormat::FLAC;
    }

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
    case AudioInfo::AudioFormat::FLAC:
        ok = IdentFLAC(file, result);
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
