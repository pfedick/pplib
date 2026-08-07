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

PFPChunk::PFPChunk()
{
    chunkname = "UNKN";
}

void PFPChunk::setName(const String& chunkname)
{
    if (chunkname.len() != 4) throw IllegalArgumentException();
    String s = chunkname;
    s.upperCase();
    for (size_t i = 0; i < 4; i++) {
        wchar_t c = s[i];
        if (c < 32 || c > 127) throw IllegalArgumentException();
    }
    this->chunkname = s;
}

void PFPChunk::setData(const void* ptr, size_t size)
{
    if (!ptr && size == 0) {
        chunkdata.clear();
        return;
    }
    if (!ptr) throw IllegalArgumentException();
    if (size > (0xffffffff - 8)) throw IllegalArgumentException();
    chunkdata.copy(ptr, size);
}

void PFPChunk::setData(const ByteArrayPtr& data)
{
    setData(data.ptr(), data.size());
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

static size_t saveChunk(char* buffer, size_t pp, const PFPChunk* chunk)
{
    const String& name = chunk->name();
    for (int i = 0; i < 4; i++)
        Poke8(buffer + pp + i, name[i]);
    Poke32(buffer + pp + 4, chunk->size());
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
        const String& cn = chunk.name();
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

void PFPFile::addChunk(PFPChunk* chunk)
{
    if (!chunk) throw NullPointerException();
    if (chunk->name() == "UNKN") {
        delete chunk;
        throw IllegalArgumentException();
    }
    Chunks.push_back(std::move(*chunk));
    delete chunk;
}

PFPChunk& PFPFile::addChunk(const PFPChunk& chunk)
{
    if (chunk.name() == "UNKN") throw IllegalArgumentException();
    Chunks.push_back(chunk);
    return Chunks.back();
}

PFPChunk& PFPFile::addChunk(PFPChunk&& chunk)
{
    if (chunk.name() == "UNKN") throw IllegalArgumentException();
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
    String s = chunkname;
    s.upperCase();
    // Chunks.remove_if([&s](const PFPChunk& c) { return c.name() == s; });
    auto it = Chunks.begin();
    while (it != Chunks.end()) {
        if (it->name() == s) {
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
        if (it.it->name() == it.findchunk) {
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
    size_t z, fsize;

    clear();
    id.set(p + 10, 4);
    mainversion = Peek8(p + 15);
    subversion = Peek8(p + 14);
    comp = (Compression::Algorithm)Peek8(p + 16);
    size_t hsize = Peek8(p + 9);
    ByteArray uncompressedData;
    if (comp) {
        p = (char*)ff.map(hsize, 8);
        if (!p) throw ReadException();
        size_t sizeunk = Peek32(p);
        size_t sizecomp = Peek32(p + 4);
        p = ff.map(hsize + 8, sizecomp);
        if (!p) throw ReadException();
        uncompressedData.malloc(sizeunk + 1);
        size_t dstlen = sizeunk;
        Compression c;
        c.init(comp);
        c.uncompress((void*)uncompressedData.ptr(), &dstlen, p, sizecomp);
        if (dstlen != sizeunk) {
            throw DecompressionFailedException();
        }
        p = (char*)uncompressedData.ptr();
        fsize = dstlen;
    } else {
        p = ff.map();
        p += hsize;
        fsize = ff.size() - hsize;
    }
    // Wir haben nun den ersten Chunk ab Pointer p
    z = 0;
    String Chunkname;
    size_t size = 0;
    while ((z += size) < fsize) {
        size = Peek32(p + z + 4);
        if (strncmp(p + z, "ENDF", 4) == 0) break;
        if (!size) break;
        // Falls z+size über das Ende der Datei geht, stimmt mit diesem Chunk was nicht
        if (z + size > fsize) break;
        PFPChunk chunk;
        Chunkname.set(p + z, 4);
        chunk.setName(Chunkname);
        chunk.setData(p + z + 8, size - 8);
        addChunk(std::move(chunk));
    }
}

} // namespace pplib
