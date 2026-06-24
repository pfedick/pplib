/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
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

#include "prolog_ppl7.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ppl7.h>
#include <ppl7-audio.h>

// #define ID3DEBUG

namespace ppl7
{

static const char* genres[] = {"Blues", // 0
                               "Classic Rock",
                               "Country",
                               "Dance",
                               "Disco",
                               "Funk",
                               "Grunge",
                               "Hip-Hop",
                               "Jazz",
                               "Metal",
                               "New Age",
                               "Oldies",
                               "Other",
                               "Pop",
                               "R&B",
                               "Rap",
                               "Reggae",
                               "Rock",
                               "Techno",
                               "Industrial",
                               "Alternative",
                               "Ska",
                               "Death Metal",
                               "Pranks",
                               "Soundtrack",
                               "Euro-Techno",
                               "Ambient",
                               "Trip-Hop",
                               "Vocal",
                               "Jazz+Funk",    // 29
                               "Fusion",       // 30
                               "Trance",       // 31
                               "Classical",    // 32
                               "Instrumental", // 33
                               "Acid",         // 34
                               "House",        // 35
                               "Game",
                               "Sound Clip",
                               "Gospel",
                               "Noise",
                               "AlternRock", // 40
                               "Bass",
                               "Soul",
                               "Punk",
                               "Space",
                               "Meditative",
                               "Instrumental Pop",
                               "Instrumental Rock",
                               "Ethnic",
                               "Gothic",
                               "Darkwave",
                               "Techno-Industrial",
                               "Electronic",
                               "Pop-Folk",
                               "Eurodance",
                               "Dream",
                               "Southern Rock",
                               "Comedy",
                               "Cult",
                               "Gangsta",
                               "Top 40",
                               "Christian Rap",
                               "Pop/Funk",
                               "Jungle",
                               "Native American",
                               "Cabaret",
                               "New Wave",
                               "Psychadelic",
                               "Rave",
                               "Showtunes",
                               "Trailer",
                               "Lo-Fi",
                               "Tribal",
                               "Acid Punk",
                               "Acid Jazz",
                               "Polka",
                               "Retro",
                               "Musical",
                               "Rock & Roll",
                               "Hard Rock",
                               "Folk",
                               "Folk-Rock",
                               "National Folk",
                               "Swing",
                               "Fast Fusion",
                               "Bebob",
                               "Latin",
                               "Revival",
                               "Celtic",
                               "Bluegrass",
                               "Avantgarde",
                               "Gothic Rock",
                               "Progressive Rock",
                               "Psychedelic Rock",
                               "Symphonic Rock",
                               "Slow Rock",
                               "Big Band",
                               "Chorus",
                               "Easy Listening",
                               "Acoustic",
                               "Humour",
                               "Speech",
                               "Chanson",
                               "Opera",
                               "Chamber Music",
                               "Sonata",
                               "Symphony",
                               "Booty Bass",
                               "Primus",
                               "Porn Groove",
                               "Satire",
                               "Slow Jam",
                               "Club",
                               "Tango",
                               "Samba",
                               "Folklore",
                               "Ballad",
                               "Power Ballad",
                               "Rhythmic Soul",
                               "Freestyle",
                               "Duet",
                               "Punk Rock",
                               "Drum Solo",
                               "A capella",
                               "Euro-House",
                               "Dance Hall", // 125
                               "Goa",
                               "Drum’n’Bass", // 127
                               "Club-House",  // 128
                               "Hardcore",
                               "Terror",
                               "Indie",
                               "BritPop",
                               "Negerpunk",
                               "Polsk Punk",
                               "Beat",
                               "Christian Gangsta",
                               "Heavy Metal",
                               "Black Metal",
                               "Crossover",
                               "Contemporary Christian",
                               "Christian Rock",
                               "Merengue",
                               "Salsa",
                               "Thrash Metal",
                               "Anime",
                               "JPop",
                               "SynthPop", // 147
                               NULL};

static size_t strlen16(const char* data)
{
    size_t l = 0;
    uint16_t* d = (uint16_t*)data;
    while (d[l] != 0)
        l++;
    return l;
}

String GetID3GenreName(int id)
{
    int max = sizeof(genres) / sizeof(char*) - 2;
    if (id < 0 || id > max) return String();
    return String(genres[id]);
}

ID3Frame::ID3Frame()
{
    Flags = 0;
}

ID3Frame::ID3Frame(const String& name)
{
    ID.set(name, 4);
    Flags = 0;
}

ID3Frame::ID3Frame(const String& name, int flags, const ByteArrayPtr& data)
{
    ID.set(name, 4);
    Flags = flags;
    this->data.copy(data);
}

void ID3Frame::getData(ByteArray& data) const
{
    data.copy(this->data);
}

void ID3Frame::hexDump() const
{
    if (data.isEmpty()) return;
    data.hexDump();
}

void ID3Tag::copyAndDecodeText(String& s, const ID3Frame* frame, int offset) const
{
    if (!frame || frame->isEmpty()) {
        s.clear();
        return;
    }
    int encoding = Peek8(frame->dataPtr() + offset);
    if (encoding < 32) {
        ID3Tag::decode(frame, offset + 1, encoding, s, localCharset);
    } else {
        String from(frame->dataPtr() + offset, frame->size() - offset);
        Iconv iconv("ISO-8859-1", localCharset);
        iconv.transcode(from, s);
    }
}

void ID3Tag::copyAndDecodeText(String& s, const ID3Frame* frame, int offset, const ppl7::String& charset)
{
    if (!frame || frame->isEmpty()) {
        s.clear();
        return;
    }
    int encoding = Peek8(frame->dataPtr() + offset);
    if (encoding < 32) {
        ID3Tag::decode(frame, offset + 1, encoding, s, charset);
    } else {
        String from(frame->dataPtr() + offset, frame->size() - offset);
        Iconv iconv("ISO-8859-1", charset);
        iconv.transcode(from, s);
    }
}
int ID3Tag::decode(const ID3Frame* frame, int offset, int encoding, String& target) const
{
    return ID3Tag::decode(frame, offset, encoding, target, localCharset);
}

int ID3Tag::decode(const ID3Frame* frame, int offset, int encoding, String& target, const ppl7::String& charset)
{
    size_t size = 0;
    const char* data = frame->dataPtr() + offset;
    // printf ("frame->data=%tx, offset=%d, encoding=%d\n", (std::ptrdiff_t)frame->data, offset, encoding);
    if (encoding == 0) {
        // frame->hexDump();
        size = strlen(data);
        if (size + offset > frame->size()) size = frame->size() - offset;
        // Iconv iconv("ISO-8859-1",localCharset);
        target.set(Transcode(data, size, "ISO-8859-1", charset));
        return offset + size + 1;
    } else if (encoding == 1) {
        size = strlen16(data) * 2;
        if (size + offset > frame->size()) size = frame->size() - offset;
        target.set(Transcode(data, size, "UTF-16", charset));
        return offset + size + 2;
    } else if (encoding == 2) {
        size = strlen16(data) * 2;
        if (size + offset > frame->size()) size = frame->size() - offset;
        target.set(Transcode(data, size, "UTF-16BE", charset));
        return offset + size + 2;
    } else if (encoding == 3) {
        size = strlen(data);
        if (size + offset > frame->size()) size = frame->size() - offset;
        target.set(data, size);
        return offset + size + 1;
    } else if (encoding > 31) {
        size = strlen(data);
        if (size + offset > frame->size()) size = frame->size() - offset;
        target.set(Transcode(data, size, "ISO-8859-1", charset));
        return offset + size + 1;
    }
    return offset + size + 1;
}

/*!\brief Konstruktor ohne Dateinamen
 *
 * \desc
 * Durch Verwendung dieses Konstruktors wird die Klasse initialisiert, ohne dass
 * eine Datei geladen wird. Dies kann mit CID3Tag::Load später nachgeholt werden.
 *
 */
ID3Tag::ID3Tag()
{
    PaddingSize = 1024;
    PaddingSpace = 128;
    MaxPaddingSpace = 1024;
    Size = 0;
    Flags = 0;
    myAudioFormat = AF_UNKNOWN;
    localCharset = Iconv::getLocalCharset();
}

/*!\brief Konstruktor mit Dateinamen
 *
 * \desc
 * Durch Verwendung dieses Konstruktors wird die Klasse initialisiert und die
 * angegebene Date \p filename geladen. Dazu wird die Funktion CID3Tag::Load
 * aufgerufen.
 *
 * \param File Name der Audio-Datei, deren Tags geladen werden soll.
 *
 */
ID3Tag::ID3Tag(const String& File)
{
    PaddingSize = 1024;
    PaddingSpace = 128;
    MaxPaddingSpace = 1024;
    Size = 0;
    Flags = 0;
    myAudioFormat = AF_UNKNOWN;
    localCharset = Iconv::getLocalCharset();
    load(File);
}

/*!\brief Destruktor der Klasse
 *
 * \desc
 * Der Destruktor ruft die Funktion CID3Tag::Clear auf, wodurch sämtlicher durch die
 * Klasse reservierter Speicher wieder freigegeben wird.
 *
 */
ID3Tag::~ID3Tag()
{
    clear();
}

void ID3Tag::setPaddingSize(int bytes)
{
    PaddingSize = bytes;
}

void ID3Tag::setPaddingSpace(int bytes)
{
    PaddingSpace = bytes;
}

void ID3Tag::setMaxPaddingSpace(int bytes)
{
    MaxPaddingSpace = bytes;
}

void ID3Tag::setLocalCharset(const String& charset)
{
    localCharset = charset;
}

void ID3Tag::clear()
{
    frames.clear();
    Filename.clear();
    myAudioFormat = AF_UNKNOWN;
    Size = 0;
    Flags = 0;
}

void ID3Tag::clearTags()
{
    frames.clear();
}

ID3Tag::AudioFormat ID3Tag::identAudioFormat(FileObject& File)
{
    if (File.size() < 12) return AF_UNKNOWN;
    const char* adr = File.map(0, 12);
    if (!adr) return AF_UNKNOWN;
    // HexDump((void*)adr,12);
    // printf ("PeekN32(adr+4)=%ud, File.Size=%d\n",ppl7::PeekN32(adr+4), (uint32_t)File.size() );
    if (PeekN32(adr + 4) < File.size() && PeekN32(adr + 0) == 0x464F524D && PeekN32(adr + 8) == 0x41494646) return AF_AIFF;
    if (Peek32(adr + 4) <= File.size() - 8 && strncmp(adr + 0, "RIFF", 4) == 0 && strncmp(adr + 8, "WAVE", 4) == 0) return AF_WAVE;

    if (IdentMPEG(File, NULL)) return AF_MP3;
    return AF_UNKNOWN;
}

uint64_t ID3Tag::findId3Tag(FileObject& File)
{
    myAudioFormat = identAudioFormat(File);
    if (myAudioFormat == AF_UNKNOWN)
        throw ppl7::UnsupportedAudioFormatException();
    else if (myAudioFormat == AF_MP3)
        return 0;
    else if (myAudioFormat == AF_AIFF) {
        uint64_t p = 12;
        while (p + 8 < File.size()) {
            const char* adr = File.map(p, 8);
            if (!adr) break;
            uint32_t size = PeekN32(adr + 4);
            if (PeekN32(adr) == 0x49443320) return p + 8;
            p += size + 8;
            if (size % 2) p++;
        }
    } else if (myAudioFormat == AF_WAVE) {
        uint64_t p = 12;
        while (p + 8 < File.size()) {
            const char* adr = File.map(p, 8);
            if (!adr) break;
            uint32_t size = Peek32(adr + 4);
            if (Peek32(adr) == 0x20336469) return p + 8; // "id3 " (Little Endian)
            p += size + 8;
            if (size % 2) p++;
        }
    }
    return (uint64_t)-1;
}

void ID3Tag::load(const String& filename)
{
    Filename = filename;
    File ff;
    ff.open(filename, File::READ);
    load(ff);
}

static size_t synchronize(unsigned char* adr, size_t size)
{
    size_t src = 0;
    size_t tgt = 0;
    for (src = 0; src < size - 3; src += 1) {
        unsigned char byte = adr[src];
        adr[tgt] = byte;
        tgt++;

        if (byte == 255) {
            // HexDump(adr+src,10);
            if (adr[src + 1] == 0 && (adr[src + 2] & (32 + 64 + 128)) == (32 + 64 + 128)) {
                adr[tgt] = adr[src + 2];
                src += 2;
                tgt++;
            } else if (adr[src + 1] == 0 && adr[src + 2] == 0) {
                adr[tgt] = 0;
                src += 2;
                tgt++;
            }
        }
    }
    return tgt;
}

void ID3Tag::load(FileObject& file)
{
    frames.clear();
    myAudioFormat = AF_UNKNOWN;
    Size = 0;
    Flags = 0;

    // ID3V2 Header einlesen (10 Byte)
    uint64_t p = findId3Tag(file);
    if (p == (uint64_t)-1) {
        return;
    }
    const char* adr = file.map(p, 10);
#ifdef ID3DEBUG
    ppl7::HexDump(adr, 10);
#endif
    if (strncmp(adr, "ID3", 3) != 0) {
        return;
    }

    int version = 0;
    if (Peek8(adr + 3) == 3 && Peek8(adr + 4) == 0)
        version = 3; // Version 2.3
    else if (Peek8(adr + 3) == 4 && Peek8(adr + 4) == 0)
        version = 4; // Version 2.4
    if (version == 0) {
        throw UnsupportedID3TagVersionException();
    }
    Flags = Peek8(adr + 5);
    bool extendedHeader = false;
    bool unsyncFlag = false;
    // bool experimentalFlag=false;
    // bool footerFlag=false;
    if (Flags & 128) { // Unsynchonisation-Flag gesetzt
        unsyncFlag = true;
    }
    if (Flags & 64) { // Extended Header Flag
        extendedHeader = true;
    }
    /*
    if (version>3) {
        if (Flags&32) experimentalFlag=true;	// Experimental Flag is set
        if (Flags&16) footerFlag=true;			// Footer is present
    }
    */
    Size = Peek8(adr + 9);
    int s = Peek8(adr + 8);
    Size |= s << 7;
    s = Peek8(adr + 7);
    Size |= s << 14;
    s = Peek8(adr + 6);
    Size |= s << 21;

    // Read Tag into Memory
    ppl7::ByteArray buffer;
    file.seek(p);
    file.read(buffer, Size + 10);
    p = 10;
    int exHdrSize = 0;
    int footerSize = 0;
    if (extendedHeader) {
        adr = buffer.map(p, 4);
#ifdef ID3DEBUG
        printf("Extended Header detected:\n");
        HexDump(adr, 4);
#endif
        exHdrSize = Peek8(adr + 3);
        exHdrSize |= (Peek8(adr + 2)) << 7;
        exHdrSize |= (Peek8(adr + 1)) << 14;
        exHdrSize |= (Peek8(adr + 0)) << 21;
        p += exHdrSize;
    }

#ifdef ID3DEBUG
    printf("ID3 V2-Tag gefunden, Flags: %i, Laenge: %i Bytes\n", Flags, Size);
#endif

    // Jetzt lesen wir alle Frames in den Speicher
    while (p < buffer.size() - 10 - exHdrSize - footerSize) {
        adr = buffer.map(p, 10);
        if (!adr) break;
#ifdef ID3DEBUG
        HexDump((void*)adr, 10);
#endif
        if (Peek32(adr) == 0) break;
        ID3Frame Frame(String(adr, 4));
        Frame.setFlags(PeekN16(adr + 8));
        size_t frameSize = 0;

        if (version == 4) {
            frameSize = Peek8(adr + 7) | (Peek8(adr + 6) << 7) | (Peek8(adr + 5) << 14) | (Peek8(adr + 4) << 21);
        } else {
            frameSize = Peek8(adr + 7) | (Peek8(adr + 6) << 8) | (Peek8(adr + 5) << 16) | (Peek8(adr + 4) << 24);
        }
        if (!frameSize) {
            break;
        }
        adr = buffer.map(p + 10, frameSize);
        if (adr) {
            // HexDump(adr,Frame->Size);
            if (unsyncFlag) {
                size_t newSize = synchronize((unsigned char*)adr, frameSize);
                Frame.setData(adr, newSize);
            } else {
                Frame.setData(adr, frameSize);
            }
            frames.push_back(std::move(Frame));
        }
        p = p + 10 + frameSize;
    }
}

bool ID3Tag::tryLoad(const String& filename)
{
    try {
        Filename = filename;
        File ff;
        ff.open(filename, File::READ);
        load(ff);
        return true;
    }
    catch (...) {
    }
    return false;
}

bool ID3Tag::tryLoad(FileObject& file)
{
    try {
        load(file);
        return true;
    }
    catch (...) {
    }
    return false;
}

void ID3Tag::addFrame(const ID3Frame& frame)
{
    frames.push_back(frame);
}

void ID3Tag::addFrame(ID3Frame&& frame)
{
    frames.push_back(std::move(frame));
}

void ID3Tag::removeFrame(ID3Frame* frame)
{
    if (!frame) return;
    for (auto it = frames.begin(); it != frames.end(); ++it) {
        if (&(*it) == frame) {
            frames.erase(it);
            return;
        }
    }
}

size_t ID3Tag::frameCount() const
{
    return frames.size();
}

void ID3Tag::listFrames(bool hexdump) const
{
    for (const auto& frame : frames) {
        printf("Frame: %s, Size: %zu, Flags: %u\n", (const char*)frame.name(), frame.size(), frame.flags());
        if (hexdump) frame.hexDump();
    }
}

ID3Frame* ID3Tag::findFrame(const String& name) const
{
    for (const auto& frame : frames) {
        if (frame.name() == name) return const_cast<ID3Frame*>(&frame);
    }

    return NULL;
}

/*!\brief Benutzerdefinierten Text in einem TXXX-Frame finden
 *
 * \desc
 * Mit dieser Funktion wird nach einem benutzerdefinierten Text
 * in einem TXXX-Frame mit dem Namen \p description gesucht
 * und ein Pointer darauf zurückgegeben.
 *
 * @param name String mit der Description des gesuchten Frames.
 * @return Wurde das gewünschte Frame gefunden, gibt die Funktion einen
 * Pointer auf die CID3Frame-Klasse zurück, im Fehlerfall \c NULL.
 */
ID3Frame* ID3Tag::findUserDefinedText(const String& description) const
{
    for (const auto& frame : frames) {
        if (frame.name() == "TXXX") {
            String c;
            decode(&frame, +1, Peek8(frame.dataPtr()), c);
            if (c.strcmp(description) == 0) return const_cast<ID3Frame*>(&frame);
        }
    }
    return NULL;
}

void ID3Tag::setTextFrame(const String& framename, const String& text, TextEncoding enc)
{
    if (enc == ENC_USASCII || enc == ENC_ISO88591)
        setTextFrameISO88591(framename, text);
    else if (enc == ENC_UTF16)
        setTextFrameUtf16(framename, text);
    else
        setTextFrameUtf8(framename, text);
}

void ID3Tag::setTextFrameUtf8(const String& framename, const String& text)
{
    ByteArray enc;
    Iconv iconv(localCharset, "UTF-8");
    iconv.transcode(text, enc);
    ID3Frame newFrame(framename);
    ID3Frame* frame = findFrame(framename);
    if (!frame) {
        frame = &newFrame;
    }
    frame->Flags = 0;
    unsigned char* buffer = (unsigned char*)frame->data.malloc(enc.size() + 2);
    buffer[0] = 3; // UTF-8 Encoding
    if (enc.size() > 0) memcpy(buffer + 1, enc.ptr(), enc.size());
    buffer[1 + enc.size()] = 0; // Terminierendes 0-Byte
    // Frame hinzufügen, falls es neu ist
    if (frame == &newFrame) {
        frames.push_back(std::move(*frame));
    }
}

static void toUtf16LE(const String& localCharset, const String& text, ByteArray& enc)
{
    ByteArray buffer;
    Iconv iconv(localCharset, "UTF-16LE");
    iconv.transcode(text, buffer);
    char* b = (char*)enc.malloc(2 + buffer.size());
    if (!b) throw ppl7::OutOfMemoryException();
    b[0] = 0xff;
    b[1] = 0xfe;
    memcpy(b + 2, buffer.ptr(), buffer.size());
}

void ID3Tag::setTextFrameUtf16(const String& framename, const String& text)
{
    ByteArray enc;
    toUtf16LE(localCharset, text, enc);
    ID3Frame newFrame(framename);
    ID3Frame* frame = findFrame(framename);
    if (!frame) {
        frame = &newFrame;
    }
    frame->Flags = 0;
    unsigned char* buffer = (unsigned char*)frame->data.malloc(enc.size() + 3);

    Poke8(buffer, 1);
    Poke16(buffer + 1 + enc.size(), 0); // 2 terminierende 0-Byte, da UTF-16LE
    if (enc.size() > 0) memcpy(buffer + 1, enc.ptr(), enc.size());
    // Frame hinzufügen, falls es neu ist
    if (frame == &newFrame) {
        frames.push_back(std::move(*frame));
    }
}

void ID3Tag::setTextFrameISO88591(const String& framename, const String& text)
{
    ByteArray enc;
    Iconv iconv(localCharset, "ISO8859-1");
    iconv.transcode(text, enc);
    ID3Frame* frame = findFrame(framename);
    ID3Frame newFrame(framename);
    if (!frame) {
        frame = &newFrame;
    }
    frame->Flags = 0;
    unsigned char* buffer = (unsigned char*)frame->data.malloc(enc.size() + 2);
    buffer[0] = 0; // ISO-8859-1 Encoding
    if (enc.size() > 0) memcpy(buffer + 1, enc.ptr(), enc.size());
    buffer[1 + enc.size()] = 0; // Terminierendes 0-Byte
    // Frame hinzufügen, falls es neu ist
    if (frame == &newFrame) {
        frames.push_back(std::move(*frame));
    }
}

void ID3Tag::setArtist(const String& artist)
{
    setTextFrame("TPE1", artist);
}

void ID3Tag::setTitle(const String& title)
{
    setTextFrame("TIT2", title);
}

void ID3Tag::setAlbum(const String& album)
{
    setTextFrame("TALB", album);
}

void ID3Tag::setRemixer(const String& remixer)
{
    setTextFrame("TPE4", remixer);
    ByteArray enc, udf;
    String udfstring = "TraktorRemixer";
    toUtf16LE(localCharset, remixer, enc);
    toUtf16LE(localCharset, udfstring, udf);
    ID3Frame* frame = findUserDefinedText("TraktorRemixer");
    ID3Frame newFrame("TXXX");
    if (!frame) {
        frame = &newFrame;
    }
    frame->Flags = 0;
    unsigned char* buffer = (unsigned char*)frame->data.malloc(enc.size() + 5 + udf.size());
    buffer[0] = 1; // UTF-16LE Encoding
    memcpy(buffer + 1, udf.ptr(), udf.size());
    Poke16(buffer + 1 + udf.size(), 0); // 2 terminierende 0-Byte, da UTF-16LE
    memcpy(buffer + 3 + udf.size(), enc.ptr(), enc.size());
    Poke16(buffer + 3 + udf.size() + enc.size(), 0); // 2 terminierende 0-Byte, da UTF-16LE
    // Frame hinzufügen, falls es neu ist
    if (frame == &newFrame) {
        frames.push_back(std::move(*frame));
    }
}

void ID3Tag::setLabel(const String& label)
{
    setTextFrame("TPUB", label);
}

void ID3Tag::setTrack(const String& track)
{
    setTextFrame("TRCK", track);
}

void ID3Tag::setBPM(const String& bpm)
{
    setTextFrame("TBPM", bpm, ENC_USASCII);
}

void ID3Tag::setKey(const String& key)
{
    String k = key;
    k.replace("♯", "#");
    k.replace("♭", "b");
    setTextFrame("TKEY", k, ENC_USASCII);
}

void ID3Tag::setYear(const String& year)
{
    setTextFrame("TYER", year);
}

void ID3Tag::setGenre(const String& genre)
{
    String gen = genre;
    // Gibt es in der Genre-Liste einen passenden Eintrag?
    int genreid = 0;
    while (genres[genreid] != NULL) {
        if (genre.strcmp((char*)genres[genreid]) == 0) {
            gen.setf("(%i)", genreid);
            gen += genre;
            break;
        }
        genreid++;
    }
    setTextFrame("TCON", gen);
}

void ID3Tag::setComment(const String& comment)
{
    setComment("", comment);
}

/**@brief Kommentar setzen
 *
 * Mit dieser Funktion wird ein Kommentar zum Titel festgelegt.
 * Der Wert wird im Frame "COMM" gespeichert.
 *
 * @param[in] description Eine kurze, eindeutige Beschreibung des Kommentars. Dieser Wert wird in einem Unterframe gespeichert, damit
 * mehrere Kommentare möglich sind. Kann auch leer bleiben.
 * @param[in] comment String mit dem Kommentar
 */
void ID3Tag::setComment(const String& description, const String& comment)
{
    const char* framename = "COMM";
    ByteArray enc, descEnc;
    toUtf16LE(localCharset, description, descEnc);
    toUtf16LE(localCharset, comment, enc);

    ID3Frame newFrame(framename);
    ID3Frame* frame = &newFrame;

    for (auto& f : frames) {
        if (f.name() == framename) {
            // Wir müssen prüfen, ob die Description matcht.
            // Die Description steht ab Offset 4 (nach Enc und Lang)
            // und ist bei uns UTF-16LE mit BOM.
            if (f.size() > 4 + descEnc.size()) {
                if (memcmp(f.dataPtr() + 4, descEnc.ptr(), descEnc.size()) == 0) {
                    frame = &f;
                    break;
                }
            }
        }
    }

    frame->Flags = 0;

    unsigned char* buffer = (unsigned char*)frame->data.malloc(1 + 3 + descEnc.size() + 2 + enc.size() + 2);
    buffer[0] = 1;                   // UTF-16LE Encoding
    memcpy(buffer + 1, "\0\0\0", 3); // Unbekannte Sprache (oder "eng", siehe ISO-639-2)

    // Description (Short Content)
    memcpy(buffer + 4, descEnc.ptr(), descEnc.size());
    Poke16(buffer + 4 + descEnc.size(), 0); // Terminierung Description

    // Comment (Actual Content)
    memcpy(buffer + 4 + descEnc.size() + 2, enc.ptr(), enc.size());
    Poke16(buffer + 4 + descEnc.size() + 2 + enc.size(), 0); // Terminierung Comment

    if (frame == &newFrame) {
        frames.push_back(std::move(newFrame));
    }
}

/*!\brief Tag speichern
 *
 * \desc
 * Bei Aufruf dieser Funktion wird der ID3-Tag in der zuvor mit CID3Tag::Load
 * geladenen Datei gespeichert. Es wird sowohl ein ID3v2- als auch ain ID3v1-Tag
 * geschrieben.
 */
void ID3Tag::save()
{
    if (Filename.isEmpty()) {
        throw FilenameNotSetException();
    }
    if (myAudioFormat == AF_MP3)
        saveMP3();
    else if (myAudioFormat == AF_AIFF)
        saveAiff();
    else if (myAudioFormat == AF_WAVE)
        saveWave();
    else
        throw UnsupportedAudioFormatException("FormatId=%d", myAudioFormat);
}

static String getID3v1String(const ID3Tag& tag, const String& frameId)
{
    const ID3Frame* frame = tag.findFrame(frameId);
    if (!frame || frame->size() < 2) return "";
    const unsigned char* d = (const unsigned char*)frame->dataPtr();
    int enc = d[0];
    if (enc == 0) return String((const char*)d + 1, frame->size() - 1);
    if (enc == 1) return Transcode((const char*)d + 1, frame->size() - 1, "UTF-16", "ISO-8859-1");
    if (enc == 3) return Transcode((const char*)d + 1, frame->size() - 1, "UTF-8", "ISO-8859-1");
    return "";
}

void ID3Tag::generateId3V1Tag(ByteArray& tag) const
{
    if (frames.size() == 0) { // Keine Tags vorhanden
        tag.clear();
        return;
    }
    char* buffer = (char*)tag.calloc(128);
    strcpy(buffer, "TAG");

    String text;
    if ((text = getID3v1String(*this, "TIT2")).size()) strncpy(buffer + 3, text, 30);
    if ((text = getID3v1String(*this, "TPE1")).size()) strncpy(buffer + 33, text, 30);
    if ((text = getID3v1String(*this, "TALB")).size()) strncpy(buffer + 63, text, 30);
    if ((text = getID3v1String(*this, "TYER")).size()) strncpy(buffer + 93, text, 4);

    // Kommentar: Wir nehmen TPE4 (Remixer), falls vorhanden, sonst den Standard-Kommentar
    if ((text = getID3v1String(*this, "TPE4")).size())
        strncpy(buffer + 97, text, 30);
    else if ((text = getComment()).size())
        strncpy(buffer + 97, text, 30);

    // Tracknummer (ID3v1.1)
    String trk = getID3v1String(*this, "TRCK");
    if (trk.size() > 0) {
        int t = trk.toInt();
        if (t > 0 && t < 256) {
            buffer[125] = 0; // Marker für v1.1
            buffer[126] = (unsigned char)t;
        }
    }
    // Genre
    text = getID3v1String(*this, "TCON");
    if (text.size() > 0) {
        std::vector<String> matches;
        if (RegEx::capture("/^\\(([0-9]+)\\).*$/", text, matches)) {
            buffer[127] = (unsigned char)matches[1].toInt();
        } else {
            buffer[127] = 255;
        }
    }
}

void ID3Tag::generateId3V2Tag(ByteArray& tag) const
{
    if (frames.size() == 0) { // Keine Tags vorhanden
        tag.clear();
        return;
    }
    // Calculate required Memory
    size_t totalFrameSize = 0;
    for (const auto& frame : frames) {
        totalFrameSize += 10 + frame.size();
    }
    size_t tagSize = 10 + totalFrameSize;
    // Allocate Buffer
    char* buffer = (char*)tag.malloc(tagSize);

    // Write Tags
    char* header = buffer;
    header[0] = 'I';
    header[1] = 'D';
    header[2] = '3';
    header[3] = 4;
    header[4] = 0;
    header[5] = 0;

    // Synchsafe Size in den Header (nur die Größe der Frames ohne die 10 Bytes Header)
    uint32_t s = (uint32_t)totalFrameSize;
    buffer[9] = (s & 127);
    buffer[8] = ((s >> 7) & 127);
    buffer[7] = ((s >> 14) & 127);
    buffer[6] = ((s >> 21) & 127);

    char* frame = buffer + 10;
    for (auto& Frame : frames) {
        memcpy(frame, Frame.name().c_str(), 4);
        // Descynchronisieren
        uint32_t fs = Frame.size();
        Poke8(frame + 7, (fs & 127));
        Poke8(frame + 6, ((fs >> 7) & 127));
        Poke8(frame + 5, ((fs >> 14) & 127));
        Poke8(frame + 4, ((fs >> 21) & 127));
        PokeN16(frame + 8, Frame.Flags);
        memcpy(frame + 10, Frame.dataPtr(), Frame.size());
        frame += 10 + Frame.size();
    }
}

void ID3Tag::saveMP3()
{
    String tmpfile = Filename;
    tmpfile += ".rename.tmp";
    File n;
    File o;
    n.open(tmpfile, File::WRITE);
    o.open(Filename, File::READWRITE);
    // Wir benötigen exklusiven Zugriff auf das File
    // if (!o.LockExclusive(false)) return 0;		// TODO: Hat unter Windows keine Wirkung

    PPL_MPEG_HEADER mpg;
    if (!IdentMPEG(o, &mpg)) {
        throw UnsupportedAudioFormatException();
    }
    // Tags generieren
    ByteArray tagV1, tagV2;
    generateId3V1Tag(tagV1);
    generateId3V2Tag(tagV2);

    size_t pn = tagV2.size();
    size_t rest = 0;
    bool useoldfile = false;
    // Falls der neue Tag noch vor den ersten Frame passt und MaxPaddingSpace nicht
    // überschritten wird, schreiben wir ihn in das Originalfile
    if (pn == 0) {
        // Sonderfall, keine Tags vorhanden

    } else if (pn > 0 && pn <= mpg.start && mpg.start - pn < MaxPaddingSpace) {
        o.seek(0);
        o.write(tagV2);
        // Mit Nullen auffüllen bis zum Start der Frames
        rest = mpg.start - pn;
        useoldfile = true;
        // printf ("Keine Verschiebung nötig, verwende altes File\n");
    } else {
        n.write(tagV2);
        if (pn < PaddingSize) {
            // Wir füllen mit Nullen auf
            rest = PaddingSize - pn;
        } else {
            // Wir lassen mindestens "PaddingSpace" frei
            rest = PaddingSpace;
        }
    }
    if (rest > 0) {
        ByteArray space_ba;
        char* space = (char*)space_ba.calloc(1024);
        while (rest) {
            uint32_t bytes = rest;
            if (bytes > 1024) bytes = 1024;
            if (useoldfile)
                o.write(space, bytes, pn);
            else
                n.write(space, bytes, pn);
            pn += bytes;
            rest -= bytes;
        }
    }
    if (useoldfile) {
        // Am Ende noch den v1-Tag reinschreiben
        if (tagV1.size() > 0) {
            o.write(tagV1.ptr(), tagV1.size(), mpg.end);
        } else if (mpg.end < o.size()) {
            // Wenn kein v1-Tag mehr gewünscht ist, aber einer da war, Datei kürzen
            o.truncate(mpg.end);
        }
        n.close();
        o.close();
        File::remove(tmpfile);
    } else {
        // Nun kopieren wir die Musikframes
        n.copyFrom(o, (uint64_t)mpg.start, (uint64_t)mpg.size, (uint64_t)pn);
        // Und am Ende noch den v1-Tag
        if (tagV1.size() > 0) n.write(tagV1);
        n.close();
        o.close();
        File::remove(Filename);
        File::rename(tmpfile, Filename);
    }
}

bool ID3Tag::trySaveAiffInExistingFile(FileObject& o, ByteArrayPtr& tagV2)
{
    uint32_t qp = 12;
    while (qp + 8 < o.size()) {
        const char* adr = o.map(qp, 8);
        if (!adr) break;
        uint32_t chunkType = PeekN32(adr);
        uint32_t chunkSize = PeekN32(adr + 4);
        if (chunkType == 0x49443320) { // ID3-Chunk gefunden
            // printf ("Found ID3-Chunk with size: %u, Tag is: %u\n",size,tagV2.Size());
            if (chunkSize > tagV2.size()) {
                // Wir haben genug Platz.
                // Wenn der Slot VIEL zu groß ist (> 10% + Padding),
                // machen wir lieber einen Rewrite, um die Datei zu verkleinern.
                uint32_t max_allowed = tagV2.size() + (tagV2.size() / 10) + PaddingSize;
                if (chunkSize > max_allowed) break;

                o.write(tagV2.ptr(), tagV2.size(), qp + 8);
                // Rest des Chunks mit Nullen nullen
                if (chunkSize > tagV2.size()) {
                    ByteArray padding;
                    padding.calloc(chunkSize - tagV2.size());
                    o.write(padding);
                }
                return true;
            }
            break;
        }
        qp += chunkSize + 8;
        if (chunkSize % 2) qp++; // AIFF Alignment-Byte überspringen!
    }
    return false;
}

void ID3Tag::copyAiffToNewFile(FileObject& o, FileObject& n, ByteArrayPtr& tagV2)
{
    uint32_t qp = 12;
    uint32_t tp = 12;
    uint32_t formsize = 4;
    n.copyFrom(o, 0, 12, 0); // Header kopieren
    while (qp + 8 < o.size()) {
        const char* adr = o.map(qp, 8);
        if (!adr) break;
        uint32_t chunkType = PeekN32(adr);
        uint32_t chunkSize = PeekN32(adr + 4);
        uint32_t physicalSize = chunkSize + 8;
        if (chunkSize % 2) physicalSize++; // Padding einberechnen

        if (chunkType != 0x49443320) {           // ID3-Chunk gefunden
            n.copyFrom(o, qp, physicalSize, tp); // append chunk to temporary file
            tp += physicalSize;
            formsize += physicalSize;
        }
        qp += physicalSize;
    }

    // append new ID3-chunk
    if (tagV2.size() > 0) {
        uint32_t id3Size = tagV2.size();
        // Padding berechnen
        if (id3Size + PaddingSpace < PaddingSize)
            id3Size = PaddingSize;
        else
            id3Size += PaddingSpace;
        // AIFF: Chunk-Daten sollten vorzugsweise eine gerade Größe haben
        if (id3Size % 2) id3Size++;
        char header[8];
        PokeN32(header, 0x49443320); // "ID3 "
        PokeN32(header + 4, id3Size);
        n.write(header, 8, tp);
        n.write(tagV2.ptr(), tagV2.size(), tp + 8);
        // Padding
        if (id3Size > tagV2.size()) {
            ByteArray padding;
            padding.calloc(id3Size - tagV2.size());
            n.write(padding);
        }
        formsize += id3Size + 8;
    }
    // FORM-Größe im Header aktualisieren
    char buffer[4];
    PokeN32(buffer, formsize);
    n.write(buffer, 4, 4);
}

void ID3Tag::saveAiff()
{
    String tmpfile = Filename;
    tmpfile += ".rename.tmp";
    File n;
    File o;

    // Tags generieren
    ByteArray tagV2;
    generateId3V2Tag(tagV2);

    o.open(Filename, File::READWRITE);
    if (tagV2.size() > 0) {
        try {
            if (trySaveAiffInExistingFile(o, tagV2)) {
                o.close();
                return;
            }
        }
        catch (...) {
        }
    }
    // create temporary file for new tag
    n.touch(tmpfile);
    n.open(tmpfile, File::READWRITE);
    try {
        copyAiffToNewFile(o, n, tagV2);
    }
    catch (...) {
        n.close();
        o.close();
        File::remove(tmpfile);
        throw;
    }
    o.close();
    n.close();
    File::remove(Filename);
    File::rename(tmpfile, Filename);
}

String ID3Tag::getArtist() const
{
    String r;
    ID3Frame* frame = findFrame("TPE1");
    if (frame) {
        // frame->hexDump();
        copyAndDecodeText(r, frame, 0);
    }
    return r;
}

String ID3Tag::getTitle() const
{
    String r;
    ID3Frame* frame = findFrame("TIT2");
    if (frame) copyAndDecodeText(r, frame, 0);
    return r;
}

String ID3Tag::getGenre() const
{
    String r;
    String Tmp;
    ID3Frame* frame = findFrame("TCON");
    if (frame) copyAndDecodeText(Tmp, frame, 0);

    // Manchmal beginnt das Genre mit einer in Klammern gesetzten Ziffer.
    // Diese entspricht der GenreId des ID3v1-Tags
    std::vector<String> matches;
    if (RegEx::capture("/^\\(([0-9]+)\\)(.*)$/", Tmp, matches)) {
        // Wir bevorzugen den Text nach der Klammer
        r = matches[2];
        r.trim();
        if (r.isEmpty()) {
            // Nur wenn er leer ist, verwenden wir die GenreId
            r = matches[1];
            r.trim();
            r = GetID3GenreName(r.toInt());
        }
    } else if (RegEx::capture("/^([0-9]+)$/", Tmp, matches)) {
        // Manchmal haben wir aber auch nur eine Genre-Ziffer
        r = matches[1];
        r.trim();
        r = GetID3GenreName(r.toInt());
    } else {
        r = Tmp;
    }
    return r;
}

String ID3Tag::getComment() const
{
    return getComment("");
}

String ID3Tag::getComment(const String& description) const
{
    String defaultComment;
    for (const auto& frame : frames) {
        if (frame.name() == "COMM") {
            String desc, text;
            int encoding = Peek8(frame.dataPtr());
            int offset = decode(&frame, 4, encoding, desc);
            decode(&frame, offset, encoding, text);

            if (description.size() > 0 && desc == description) return text; // Exakter Treffer
            if (desc.isEmpty()) defaultComment = text;                      // Merken für später
        }
    }
    return defaultComment;
}

String ID3Tag::getRemixer() const
{
    String r;
    ID3Frame* frame = findFrame("TPE4");
    if (frame) copyAndDecodeText(r, frame, 0);
    return r;
}

String ID3Tag::getYear() const
{
    String r;
    ID3Frame* frame = findFrame("TYER");
    if (frame) copyAndDecodeText(r, frame, 0);
    return r;
}

String ID3Tag::getLabel() const
{
    String r;
    ID3Frame* frame = findFrame("TPUB");
    if (frame) copyAndDecodeText(r, frame, 0);
    return r;
}

String ID3Tag::getAlbum() const
{
    String r;
    ID3Frame* frame = findFrame("TALB");
    if (frame) copyAndDecodeText(r, frame, 0);
    return r;
}

String ID3Tag::getTrack() const
{
    String r;
    ID3Frame* frame = findFrame("TRCK");
    if (frame) copyAndDecodeText(r, frame, 0);
    return r;
}

String ID3Tag::getBPM() const
{
    String r;
    ID3Frame* frame = findFrame("TBPM");
    if (frame) copyAndDecodeText(r, frame, 0);
    return r;
}

String ID3Tag::getKey() const
{
    String r;
    ID3Frame* frame = findFrame("TKEY");
    if (frame) copyAndDecodeText(r, frame, 0);
    return r;
}

bool ID3Tag::getPicture(int type, ByteArray& bin) const
{
    bin.clear();
    for (const auto& frame : frames) {
        if (frame.ID == "APIC") {
            // Wir haben ein Picture
            String MimeType;
            int encoding = Peek8(frame.dataPtr());
            int offset = decode(&frame, 1, 0, MimeType);
            if ((int)Peek8(frame.dataPtr() + offset) == type) {
                String Description;
                offset = decode(&frame, offset + 1, encoding, Description);
                bin.copy(frame.dataPtr() + offset, frame.size() - offset);
                return true;
            }
        }
    }
    return false;
}

ByteArray ID3Tag::getPicture(int type) const
{
    ByteArray bin;
    getPicture(type, bin);
    return bin;
}

bool ID3Tag::hasPicture(int type) const
{
    for (const auto& frame : frames) {
        if (frame.ID == "APIC") {
            // Wir haben ein Picture
            String MimeType;
            // int encoding = Peek8(frame.dataPtr());
            int offset = decode(&frame, 1, 0, MimeType);
            if ((int)Peek8(frame.dataPtr() + offset) == type) {
                return true;
            }
        }
    }
    return false;
}

bool ID3Tag::getPrivateData(ByteArray& bin, const String& identifier) const
{
    ByteArrayPtr ref = getPrivateData(identifier);
    if (ref.isNull()) return false;
    bin.copy(ref);
    return true;
}

ByteArrayPtr ID3Tag::getPrivateData(const String& identifier) const
{
    for (const auto& frame : frames) {
        if (frame.ID == "PRIV") {
            // Wir haben ein PRIV-Frame
            if (identifier.strcmp(frame.dataPtr()) == 0) {
                return ByteArrayPtr(frame.dataPtr() + identifier.size() + 1, frame.size() - identifier.size() - 1);
            }
        }
    }
    return ByteArrayPtr();
}

String ID3Tag::getEnergyLevel() const
{
    String energy;
    for (const auto& frame : frames) {
        if (frame.ID == "TXXX") {
            // Wir haben ein TXXX-Frame
            int encoding = Peek8(frame.dataPtr());
            String identifier;
            int offset = decode(&frame, 1, encoding, identifier);
            if (identifier == "EnergyLevel") {
                decode(&frame, offset, encoding, energy);
                return energy;
            }
        }
    }
    return energy;
}

void ID3Tag::setEnergyLevel(const String& energy)
{
    ID3Frame* exisitingFrame = nullptr;

    for (auto& frame : frames) {
        if (frame.ID == "TXXX") {
            // Wir haben ein TXXX-Frame
            int encoding = Peek8(frame.dataPtr());
            String identifier;
            decode(&frame, 1, encoding, identifier);
            if (identifier == "EnergyLevel") {
                exisitingFrame = &frame;
                break;
            }
        }
    }
    ID3Frame newFrame("TXXX");
    ID3Frame* frame = exisitingFrame ? exisitingFrame : &newFrame;

    frame->Flags = 0;
    char* buffer = (char*)frame->data.malloc(14 + energy.size());
    buffer[0] = 3; // UTF-8 Encoding
    strcpy(buffer + 1, "EnergyLevel");
    Poke8(buffer + 12, 0);
    strncpy(buffer + 13, energy.c_str(), energy.size());
    Poke8(buffer + 13 + energy.size(), 0);
    if (frame == &newFrame) {
        frames.push_back(std::move(*frame));
    }
}

void ID3Tag::setPicture(int type, const ByteArrayPtr& bin, const String& MimeType)
{
    String name = "APIC";
    ID3Frame* frame = nullptr;
    for (auto& f : frames) {
        if (f.ID == name) {
            String OldMimeType;
            int offset = decode(&f, 1, 0, OldMimeType);
            if ((int)Peek8(f.dataPtr() + offset) == type) {
                frame = &f;
                break;
            }
        }
    }
    ID3Frame newFrame(name);
    if (!frame) {
        frame = &newFrame;
    }
    frame->Flags = 0;
    char* buffer = (char*)frame->data.malloc(bin.size() + MimeType.size() + 4);
    buffer[0] = 0; // ISO-8859-1 Encoding
    strcpy(buffer + 1, MimeType.c_str());
    Poke8(buffer + 2 + MimeType.size(), type);
    Poke8(buffer + 3 + MimeType.size(), 0);
    memcpy(buffer + 4 + MimeType.size(), bin.ptr(), bin.size());
    if (frame == &newFrame) {
        frames.push_back(std::move(*frame));
    }
}

void ID3Tag::removePicture(int type)
{
    String name = "APIC";
    for (auto it = frames.begin(); it != frames.end(); ++it) {
        if ((*it).ID == name) {
            String MimeType;
            int offset = decode(&(*it), 1, 0, MimeType);
            if ((int)Peek8((*it).dataPtr() + offset) == type) {
                frames.erase(it);
                return;
            }
        }
    }
}

String ID3Tag::getNullPaddedString(const ID3Frame& frame, size_t offset) const
{
    if (offset >= frame.size()) return "";
    size_t p = offset;
    while (p < frame.size() && frame.dataPtr()[p] != 0) {
        p++;
    }
    return String(frame.dataPtr() + offset, p - offset);
}

void ID3Tag::getAllPopularimeters(std::map<String, unsigned char>& data) const
{
    data.clear();
    for (const auto& frame : frames) {
        if (frame.ID == "POPM") {
            String email = getNullPaddedString(frame, 0);
            if (email.notEmpty()) {
                unsigned char rating = ppl7::Peek8(frame.dataPtr() + email.size() + 1);
                data.insert(std::pair<String, unsigned char>(email, rating));
            }
        }
    }
}

bool ID3Tag::hasPopularimeter(const String& email) const
{
    if (email.isEmpty()) return false;
    for (const auto& frame : frames) {
        if (frame.ID == "POPM") {
            String existingemail = getNullPaddedString(frame, 0);
            if (existingemail == email) {
                return true;
            }
        }
    }
    return false;
}

unsigned char ID3Tag::getPopularimeter(const String& email) const
{
    if (email.isEmpty()) return 0;
    for (const auto& frame : frames) {
        if (frame.ID == "POPM") {
            String existingemail = getNullPaddedString(frame, 0);
            if (existingemail == email) {
                return ppl7::Peek8(frame.dataPtr() + existingemail.size() + 1);
            }
        }
    }
    return 0;
}

unsigned char ID3Tag::getPopularimeter() const
{
    for (const auto& frame : frames) {
        if (frame.ID == "POPM") {
            String existingemail = getNullPaddedString(frame, 0);
            return ppl7::Peek8(frame.dataPtr() + existingemail.size() + 1);
        }
    }
    return 0;
}

void ID3Tag::setPopularimeter(const String& email, unsigned char rating)
{
    if (email.isEmpty()) return;
    ID3Frame* frame = nullptr;
    for (auto& f : frames) {
        if (f.ID == "POPM") {
            String existingemail = getNullPaddedString(f, 0);
            if (existingemail == email) {
                frame = &f;
                break;
            }
        }
    }
    ID3Frame newFrame("POPM");
    if (!frame) {
        frame = &newFrame;
    }
    char* buffer = (char*)frame->data.calloc(email.size() + 6);
    memcpy(buffer, email.getPtr(), email.size());
    buffer[email.size() + 1] = rating;

    if (frame == &newFrame) {
        frames.push_back(std::move(*frame));
    }
}

void ID3Tag::removePopularimeter()
{
    for (auto it = frames.begin(); it != frames.end();) {
        if ((*it).ID == "POPM") {
            it = frames.erase(it);
        } else {
            ++it;
        }
    }
}

bool ID3Tag::hasPopularimeter() const
{
    for (const auto& frame : frames) {
        if (frame.ID == "POPM") {
            return true;
        }
    }
    return false;
}

bool ID3Tag::trySaveWaveInExistingFile(FileObject& o, ByteArrayPtr& tagV2)
{
    uint32_t qp = 12;
    while (qp + 8 < o.size()) {
        const char* adr = o.map(qp, 8);
        if (!adr) break;
        uint32_t chunkID = Peek32(adr);
        uint32_t chunkSize = Peek32(adr + 4);
        if (chunkID == 0x20336469) { // "id3 "
            if (chunkSize >= tagV2.size()) {
                uint32_t max_allowed = tagV2.size() + (tagV2.size() / 10) + PaddingSize;
                if (chunkSize > max_allowed) break;
                o.write(tagV2.ptr(), tagV2.size(), qp + 8);
                if (chunkSize > tagV2.size()) {
                    ByteArray padding;
                    padding.calloc(chunkSize - tagV2.size());
                    o.write(padding);
                }
                return true;
            }
            break;
        }
        qp += chunkSize + 8;
        if (chunkSize % 2) qp++;
    }
    return false;
}

void ID3Tag::copyWaveToNewFile(FileObject& o, FileObject& n, ByteArrayPtr& tagV2)
{
    uint32_t qp = 12, tp = 12, formsize = 4;
    n.copyFrom(o, 0, 12, 0);
    while (qp + 8 < o.size()) {
        const char* adr = o.map(qp, 8);
        if (!adr) break;
        uint32_t chunkID = Peek32(adr);
        uint32_t chunkSize = Peek32(adr + 4);
        uint32_t physicalSize = chunkSize + 8;
        if (chunkSize % 2) physicalSize++;
        if (chunkID != 0x20336469) {
            n.copyFrom(o, qp, physicalSize, tp);
            tp += physicalSize;
            formsize += physicalSize;
        }
        qp += physicalSize;
    }
    if (tagV2.size() > 0) {
        uint32_t id3Size = tagV2.size();
        if (id3Size + PaddingSpace < PaddingSize)
            id3Size = PaddingSize;
        else
            id3Size += PaddingSpace;
        if (id3Size % 2) id3Size++;
        char header[8];
        Poke32(header, 0x20336469); // "id3 "
        Poke32(header + 4, id3Size);
        n.write(header, 8, tp);
        n.write(tagV2.ptr(), tagV2.size(), tp + 8);
        if (id3Size > tagV2.size()) {
            ByteArray padding;
            padding.calloc(id3Size - tagV2.size());
            n.write(padding);
        }
        formsize += id3Size + 8;
    }
    char buffer[4];
    Poke32(buffer, formsize);
    n.write(buffer, 4, 4); // RIFF-Größen-Update
}

void ID3Tag::saveWave()
{
    String tmpfile = Filename + ".rename.tmp";
    File n, o;
    ByteArray tagV2;
    generateId3V2Tag(tagV2);
    o.open(Filename, File::READWRITE);
    if (tagV2.size() > 0) {
        try {
            if (trySaveWaveInExistingFile(o, tagV2)) {
                o.close();
                return;
            }
        }
        catch (...) {
        }
    }
    n.touch(tmpfile);
    n.open(tmpfile, File::READWRITE);
    try {
        copyWaveToNewFile(o, n, tagV2);
    }
    catch (...) {
        n.close();
        o.close();
        File::remove(tmpfile);
        throw;
    }
    o.close();
    n.close();
    File::remove(Filename);
    File::rename(tmpfile, Filename);
}

} // namespace ppl7
