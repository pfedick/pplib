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

#ifndef PPL7_CORE_PFPFILE_H_
#define PPL7_CORE_PFPFILE_H_

#include <ppl7/types/string.h>
#include <ppl7/types/bytearray.h>
#include <ppl7/core/compression.h>
#include <ppl7/core/fileobject.h>
#include <ppl7/core/mutex.h>

namespace ppl7
{
class PFPChunk
{
    friend class PFPFile;

private:
    String chunkname;
    void* chunkdata;
    size_t chunksize;

public:
    PFPChunk();
    ~PFPChunk();
    void setName(const String& chunkname);
    void setData(const void* ptr, size_t size);
    void setData(const ByteArrayPtr& data);
    void setData(const String& s);
    void setData(const char* s);
    size_t size();
    const void* data();
    const String& name();
};

class PFPFile
{
private:
    // TODO: std containers verwenden!
    List<PFPChunk*> Chunks;
    String id;
    uint8_t mainversion, subversion;
    Compression::Algorithm comp;

    void setParam(const String& chunkname, const String& data);
    void saveChunk(char* buffer, size_t& pp, PFPChunk* chunk);

public:
    class Iterator : public List<PFPChunk*>::Iterator
    {
    public:
        String findchunk;
    };

    Mutex myMutex; // WTF? Wofür brauchen wir hier einen Mutex?

    PFPFile();
    virtual ~PFPFile();
    void clear();
    void setAuthor(const String& author);
    void setCopyright(const String& copy);
    void setDescription(const String& descr);
    void setName(const String& name);
    void setVersion(int main = 0, int sub = 0);
    void setId(const String& id);
    void save(const String& filename);
    void addChunk(PFPChunk* chunk);
    void deleteChunk(PFPChunk* chunk);
    void deleteChunk(const String& chunkname);
    PFPChunk* findFirstChunk(Iterator& it, const String& chunkname) const;
    PFPChunk* findNextChunk(Iterator& it, const String& chunkname) const;
    virtual void list() const;
    void setCompression(Compression::Algorithm type);

    void load(FileObject& ff);
    void load(const String& file);

    bool ident(FileObject& ff);
    bool ident(const String& file);

    virtual int loadRequest(const String& id, int mainversion, int subversion);

    String getName() const;
    String getDescription() const;
    String getAuthor() const;
    String getCopyright() const;
    void getVersion(int* main, int* sub) const;
    const String& getID() const;
    int getMainVersion() const;
    int getSubVersion() const;
    Compression::Algorithm getCompression() const;

    void reset(Iterator& it) const;
    PFPChunk* getFirst(Iterator& it) const;
    PFPChunk* getNext(Iterator& it) const;
};

} // namespace ppl7

#endif /* PPL7_CORE_DIR_H_ */