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
#include <pplib/core/pfpfile.h>
#include <pplib/core/compression.h>
#include <pplib/core/functions.h>
#include <pplib/core/file.h>
#include <pplib/exceptions.h>

namespace pplib
{

PFPChunk::~PFPChunk()
{
    if (ownMemory && chunkdata.ptr()) {
        ::free(chunkdata.ptr());
    }
}

void PFPChunk::setName(const String& chunkname)
{
    setName(chunkname.c_str(), chunkname.size());
}

void PFPChunk::setName(const char* chunkname, size_t size)
{
    if (!chunkname || size != 4) throw IllegalArgumentException();
    for (size_t i = 0; i < 4; i++) {
        unsigned char c = (unsigned char)chunkname[i];
        if (c < 32 || c > 127) throw IllegalArgumentException();
    }
    memcpy(this->chunkname, chunkname, 4);
    this->chunkname[4] = '\0';
    // Chunkname muss uppercase sein
    for (size_t i = 0; i < 4; i++) {
        this->chunkname[i] = (char)toupper(this->chunkname[i]);
    }
}

void PFPChunk::setData(const void* ptr, size_t size)
{
    if (!ptr && size == 0) {
        if (ownMemory && chunkdata.ptr()) {
            ::free(chunkdata.ptr());
        }
        ownMemory = false;
        chunkdata.use(nullptr, 0);
        return;
    }
    if (!ptr || size > (0xffffffff - 8)) throw IllegalArgumentException();
    char* buffer = (char*)malloc(size);
    if (!buffer) throw OutOfMemoryException();
    memcpy(buffer, ptr, size);
    if (ownMemory && chunkdata.ptr()) {
        free(chunkdata.ptr());
    }
    chunkdata.use(buffer, size);
    ownMemory = true;
}

void PFPChunk::setData(const ByteArrayPtr& data)
{
    setData(data.ptr(), data.size());
}

void PFPChunk::useData(const void* ptr, size_t size)
{
    if (ownMemory && chunkdata.ptr() && chunkdata.ptr() != ptr) {
        ::free(chunkdata.ptr());
    }
    chunkdata.use((void*)ptr, size);
    ownMemory = false;
}

void PFPChunk::useData(const ByteArrayPtr& data)
{
    useData(data.ptr(), data.size());
}

PFPChunk::PFPChunk(const PFPChunk& other)
{
    memcpy(chunkname, other.chunkname, 5);
    if (other.ownMemory == false) {
        ownMemory = false;
        chunkdata = other.chunkdata;
    } else {
        ownMemory = false;
        if (other.size() > 0) {
            char* buffer = (char*)malloc(other.size());
            if (!buffer) throw OutOfMemoryException();
            memcpy(buffer, other.data(), other.size());
            chunkdata.use(buffer, other.size());
            ownMemory = true;
        }
    }
}

PFPChunk::PFPChunk(PFPChunk&& other) noexcept
{
    memcpy(chunkname, other.chunkname, 5);
    ownMemory = other.ownMemory;
    chunkdata = other.chunkdata;
    other.chunkdata.use(nullptr, 0);
    other.ownMemory = false;
    memcpy(other.chunkname, "UNKN", 5);
}

PFPChunk& PFPChunk::operator=(const PFPChunk& other)
{
    if (this != &other) {
        char* buffer = nullptr;
        if (other.ownMemory && other.size() > 0) {
            buffer = (char*)malloc(other.size());
            if (!buffer) throw OutOfMemoryException();
            memcpy(buffer, other.data(), other.size());
        }
        if (ownMemory && chunkdata.ptr()) {
            ::free(chunkdata.ptr());
        }
        memcpy(chunkname, other.chunkname, 5);
        if (buffer) {
            chunkdata.use(buffer, other.size());
            ownMemory = true;
        } else {
            chunkdata = other.chunkdata;
            ownMemory = false;
        }
    }
    return *this;
}

PFPChunk& PFPChunk::operator=(PFPChunk&& other) noexcept
{
    if (this != &other) {
        if (ownMemory && chunkdata.ptr()) {
            ::free(chunkdata.ptr());
        }
        memcpy(chunkname, other.chunkname, 4);
        chunkname[4] = '\0';
        ownMemory = other.ownMemory;
        chunkdata = other.chunkdata;
        other.chunkdata.use(nullptr, 0);
        other.ownMemory = false;
        memcpy(other.chunkname, "UNKN", 5);
    }
    return *this;
}

PFPFile::PFPFile()
{
    id = "UNKN";
    mainversion = subversion = 0;
    comp = Compression::Algo_NONE;
}

PFPFile::~PFPFile()
{
    clear();
}

void PFPFile::clear()
{
    Chunks.clear();
    payload.clear();
    dataPtr.use(nullptr, 0);
    id = "UNKN";
    mainversion = subversion = 0;
    comp = Compression::Algo_NONE;
}

void PFPFile::setVersion(int main, int sub)
{
    if (main < 0 || main > 255 || sub < 0 || sub > 255) throw IllegalArgumentException();
    mainversion = (uint8_t)(main & 0xff);
    subversion = (uint8_t)(sub & 0xff);
}

void PFPFile::setId(const String& id)
{
    if (id.len() != 4) throw IllegalArgumentException();
    for (size_t i = 0; i < 4; i++) {
        wchar_t c = id[i];
        if (c < 32 || c > 127) throw IllegalArgumentException();
    }
    this->id = id;
}

void PFPFile::setCompression(Compression::Algorithm type)
{
    if (type > 2 || type < 0) throw UnknownCompressionMethodException();
    comp = type;
}

void PFPFile::setStringParam(const String& chunkname, const String& data)
{
    deleteChunk(chunkname);
    PFPChunk chunk(chunkname, ByteArrayPtr(data));
    Chunks.push_back(std::move(chunk));
}

void PFPFile::setAuthor(const String& author)
{
    setStringParam("AUTH", author);
}

void PFPFile::setCopyright(const String& copy)
{
    setStringParam("COPY", copy);
}

void PFPFile::setDescription(const String& descr)
{
    setStringParam("DESC", descr);
}

void PFPFile::setName(const String& name)
{
    setStringParam("NAME", name);
}

String PFPFile::getName() const
{
    for (auto& chunk : Chunks) {
        if (chunk.chunkname == "NAME") {
            return String(chunk.chunkdata);
        }
    }
    return String();
}

String PFPFile::getDescription() const
{
    for (auto& chunk : Chunks) {
        if (chunk.chunkname == "DESC") {
            return String(chunk.chunkdata);
        }
    }
    return String();
}

String PFPFile::getAuthor() const
{
    for (auto& chunk : Chunks) {
        if (chunk.chunkname == "AUTH") {
            return String(chunk.chunkdata);
        }
    }
    return String();
}

String PFPFile::getCopyright() const
{
    for (auto& chunk : Chunks) {
        if (chunk.chunkname == "COPY") {
            return String(chunk.chunkdata);
        }
    }
    return String();
}

size_t PFPFile::saveChunk(char* buffer, size_t pp, const PFPChunk* chunk)
{
    memcpy(buffer + pp, chunk->chunkname, 4);
    Poke32(buffer + pp + 4, chunk->size() + 8);
    pp += 8;
    if (chunk->size() > 0) {
        memcpy(buffer + pp, chunk->data(), chunk->size());
        pp += chunk->size();
    }
    return 8 + chunk->size();
}

void PFPFile::save(const String& filename)
{
    File ff;
    // Wir benötigen zuerst die Gesamtgröße aller Chunks
    size_t size = 24; // Headergröße
    for (auto& chunk : Chunks) {
        size += 8;
        size += chunk.size();
    }
    // plus ENDF-Chunk
    size += 8;

    // Datei zusammenbauen
    ByteArray data;
    char* p = (char*)data.malloc(size);
    size_t hsize = 24;
    memcpy(p, "PFP-File", 8);
    Poke8(p + 8, 3);
    Poke8(p + 9, (int)hsize);
    for (int i = 0; i < 4; i++)
        Poke8((p + 10 + i), (unsigned int)id[i]);
    Poke8(p + 15, mainversion);
    Poke8(p + 14, subversion);
    Poke8(p + 16, comp);
    Poke8(p + 17, 0);
    Poke8(p + 18, 0);
    Poke8(p + 19, 0);
    Poke32(p + 20, (uint32_t)GetTime());

    size_t pp = hsize;
    // Chunks zusammenfassen
    // Zuerst die vordefinierten, die wir am Anfang des Files wollen
    Iterator it;
    reset(it);
    PFPChunk* chunk;
    chunk = findFirstChunk(it, "NAME");
    if (chunk) saveChunk(p, pp, chunk);
    chunk = findFirstChunk(it, "AUTH");
    if (chunk) saveChunk(p, pp, chunk);
    chunk = findFirstChunk(it, "DESC");
    if (chunk) saveChunk(p, pp, chunk);
    chunk = findFirstChunk(it, "COPY");
    if (chunk) saveChunk(p, pp, chunk);
    // Restliche Chunks
    for (const auto& chunk : Chunks) {
        const std::string_view cn(chunk.chunkname, 4);
        if (cn != "NAME" && cn != "AUTH" && cn != "DESC" && cn != "COPY") {
            pp += saveChunk(p, pp, &chunk);
        }
    }
    memcpy(p + pp, "ENDF", 4);
    Poke32(p + pp + 4, 0);
    pp += 8;

    size_t savesize = pp - hsize;
    // Komprimierung?
    Compression c;
    ByteArray compressedData;
    if (comp) {
        size_t dstlen = savesize + 64;
        compressedData.malloc(dstlen);
        c.init(comp, Compression::Level_High);
        c.compress((void*)compressedData.ptr(), &dstlen, p + hsize, savesize);
        savesize = dstlen;
    }

    ff.open(filename, File::FileMode::WRITE);
    ff.write(p, hsize);
    if (comp) {
        char t[8];
        Poke32(t, (int)(pp - hsize));
        Poke32(t + 4, (int)savesize);
        ff.write(t, 8);
        ff.write(compressedData.ptr(), savesize);
    } else {
        ff.write(p + hsize, pp - hsize);
    }
    ff.close();
}

PFPChunk& PFPFile::addChunk(const PFPChunk& chunk)
{
    if (strncmp(chunk.chunkname, "UNKN", 4) == 0) throw IllegalArgumentException();
    Chunks.push_back(chunk);
    return Chunks.back();
}

PFPChunk& PFPFile::addChunk(PFPChunk&& chunk)
{
    if (strncmp(chunk.chunkname, "UNKN", 4) == 0) throw IllegalArgumentException();
    Chunks.push_back(std::move(chunk));
    return Chunks.back();
}

void PFPFile::deleteChunk(PFPChunk* chunk)
{
    if (!chunk) return;
    for (auto it = Chunks.begin(); it != Chunks.end(); ++it) {
        if (&(*it) == chunk) {
            Chunks.erase(it);
            return;
        }
    }
}

void PFPFile::deleteChunk(const String& chunkname)
{
    if (chunkname.len() != 4) return;
    char cn[4];
    memcpy(cn, chunkname.c_str(), 4);
    for (int i = 0; i < 4; i++)
        cn[i] = toupper(cn[i]);

    // Chunks.remove_if([&s](const PFPChunk& c) { return c.name() == s; });
    auto it = Chunks.begin();
    while (it != Chunks.end()) {
        if (strncmp(it->chunkname, cn, 4) == 0) {
            it = Chunks.erase(it); // erase liefert den Iterator auf das nachfolgende Element
        } else {
            ++it;
        }
    }
}

PFPChunk* PFPFile::findFirstChunk(Iterator& it, const String& chunkname) const
{
    it.started = false;
    return findNextChunk(it, chunkname);
}

PFPChunk* PFPFile::findNextChunk(Iterator& it, const String& chunkname) const
{
    if (chunkname.notEmpty()) {
        it.findchunk = chunkname;
    }
    if (it.findchunk.len() != 4) throw IllegalArgumentException();

    if (!it.started) {
        it.it = Chunks.begin();
        it.started = true;
    } else if (it.it != Chunks.end()) {
        ++it.it;
    }

    while (it.it != Chunks.end()) {
        if (strncmp(it.it->chunkname, it.findchunk.c_str(), 4) == 0) {
            return const_cast<PFPChunk*>(&(*it.it));
        }
        ++it.it;
    }
    return nullptr;
}

void PFPFile::reset(Iterator& it) const
{
    it.it = Chunks.begin();
    it.started = false;
}

PFPChunk* PFPFile::getFirst(Iterator& it) const
{
    reset(it);
    if (Chunks.empty()) return nullptr;
    it.started = true;
    return const_cast<PFPChunk*>(&(*Chunks.begin()));
}

PFPChunk* PFPFile::getNext(Iterator& it) const
{
    if (!it.started) return getFirst(it);
    if (it.it != Chunks.end()) {
        ++it.it;
        if (it.it != Chunks.end()) {
            return const_cast<PFPChunk*>(&(*it.it));
        }
    }
    return nullptr;
}

void PFPFile::list() const
{
    printf("PFP-File Version 3 ============================================\n");
    printf("ID: %s, Version: %i.%i, Komprimierung: ", (const char*)id, mainversion, subversion);
    switch (comp) {
    case 0:
        printf("keine\n");
        break;
    case 1:
        printf("Zlib\n");
        break;
    case 2:
        printf("Bzip2\n");
        break;
    default:
        printf("unbekannt\n");
        break;
    }
    String Tmp;
    Tmp = getName();
    if (Tmp.notEmpty()) printf("Name:        %s\n", (const char*)Tmp);
    Tmp = getAuthor();
    if (Tmp.notEmpty()) printf("Author:      %s\n", (const char*)Tmp);
    Tmp = getDescription();
    if (Tmp.notEmpty()) printf("Description: %s\n", (const char*)Tmp);
    Tmp = getCopyright();
    if (Tmp.notEmpty()) printf("Copyright:   %s\n", (const char*)Tmp);
    if (Chunks.empty()) {
        printf("Keine Chunks vorhanden\n");
    } else {
        printf("\nChunks:\n");
        for (auto& chunk : Chunks) {
            printf("  %s: %zu Bytes\n", (const char*)chunk.chunkname, chunk.size());
        }
    }
    printf("===============================================================\n");
}

bool PFPFile::ident(const String& file)
{
    File ff;
    try {
        ff.open(file, File::FileMode::READ);
    }
    catch (...) {
        return false;
    }
    return ident(ff);
}

bool PFPFile::ident(FileObject& ff)
{
    try {
        const char* p;
        p = ff.map(0, 24);
        if (strncmp(p, "PFP-File", 8) != 0) return false;
        if (Peek8(p + 8) != 3) return false;
        id.set(p + 10, 4);
        mainversion = Peek8(p + 15);
        subversion = Peek8(p + 14);
        comp = (Compression::Algorithm)Peek8(p + 16);
        return true;
    }
    catch (...) {
        return false;
    }
    return false;
}

bool PFPFile::ident(const ByteArrayPtr& buffer) noexcept
{
    if (buffer.size() < 24) return false;
    const char* p = (const char*)buffer.ptr();
    if (strncmp(p, "PFP-File", 8) != 0) return false;
    if (Peek8(p + 8) != 3) return false;
    id.set(p + 10, 4);
    mainversion = Peek8(p + 15);
    subversion = Peek8(p + 14);
    comp = (Compression::Algorithm)Peek8(p + 16);
    return true;
}

void PFPFile::useMemory(const ByteArrayPtr& data)
{
    if (data.size() < 24) throw InvalidFormatException();
    const char* p = (const char*)data.ptr();
    if (strncmp(p, "PFP-File", 8) != 0) throw InvalidFormatException();
    if (Peek8(p + 8) != 3) throw InvalidFormatException();
    size_t hsize = Peek8(p + 9); // In der Regel 24 Byte
    if (hsize < 24 || hsize > data.size()) throw InvalidFormatException();

    Chunks.clear();
    if (data.ptr() != payload.ptr()) {
        payload.clear();
    }
    dataPtr.use(nullptr, 0);
    id.set(p + 10, 4);
    mainversion = Peek8(p + 15);
    subversion = Peek8(p + 14);
    comp = (Compression::Algorithm)Peek8(p + 16);

    if (comp) {
        // Mindestens 8 Bytes für den Komprimierungsheader erforderlich
        if (data.size() - hsize < 8) throw InvalidFormatException();

        // Wir müssen erst dekomprimieren, dazu müssen wir dann eigenen Speicher allokieren.
        // Wir rechnen damit, dass `data` eventuell auf unseren eigenen Speicher `payload` zeigt.
        // Wir allokieren daher eigenen Speicher für die dekomprimierten Daten.

        size_t sizeunk = Peek32(p + hsize);
        size_t sizecomp = Peek32(p + hsize + 4);
        // Kurzer Bounds-Check, ob die komprimierten Daten innerhalb des Puffers liegen
        if (data.size() - hsize - 8 < sizecomp) throw InvalidFormatException();

        ByteArray uncompressedData;
        uncompressedData.malloc(sizeunk + 1);
        size_t dstlen = sizeunk;
        Compression c;
        c.init(comp);
        c.uncompress((void*)uncompressedData.ptr(), &dstlen, (const char*)data.ptr() + hsize + 8, sizecomp);
        if (dstlen != sizeunk) {
            throw DecompressionFailedException();
        }
        payload = std::move(uncompressedData);
        dataPtr.use(payload);

    } else {
        dataPtr.use((void*)(p + hsize), data.size() - hsize);
    }
    p = (const char*)dataPtr.ptr();
    size_t z = 0;
    size_t fsize = dataPtr.size();
    while (fsize - z >= 8) { // Bounds-Check, ob noch genügend Platz für einen Chunk-Header ist
        if (strncmp(p + z, "ENDF", 4) == 0) break;
        size_t size = Peek32(p + z + 4);
        // Chunk muss mindestens 8 Bytes groß sein und darf nicht über das Pufferende ragen
        if (size < 8 || size > fsize - z) break;

        PFPChunk chunk;
        chunk.setName(p + z, 4);
        chunk.useData(p + z + 8, size - 8);
        addChunk(std::move(chunk));
        z += size;
    }
}

void PFPFile::load(const String& file)
{
    File ff;
    ff.open(file, File::FileMode::READ);
    load(ff);
}

void PFPFile::load(FileObject& ff)
{
    const char* p;
    try {
        p = ff.map(0, 24);
    }
    catch (OverflowException&) {
        throw InvalidFormatException();
    }
    if (memcmp(p, "PFP-File", 8) != 0) throw InvalidFormatException();
    if (Peek8(p + 8) != 3) throw InvalidFormatException();

    // Sieht nach einer gültigen Datei aus, wir können mit dem Laden fortfahren.
    clear();
    ff.load(payload); // Komplettes File in den Speicher laden
    useMemory(payload);
}

} // namespace pplib
