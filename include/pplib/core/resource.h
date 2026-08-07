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

#ifndef PPLIB_CORE_RESOURCE_H_
#define PPLIB_CORE_RESOURCE_H_

#include <pplib/types/string.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/core/fileobject.h>

namespace pplib
{
//! \brief PPL-Resourcen
class Resource
{
private:
    int count;
    int maxid, minid;
    int major, minor;
    ByteArray memory;
    ByteArrayPtr memref;
    void* firstchunk;

    void checkResource(const ByteArrayPtr& memory);
    void parse();
    void uncompress(void* resource);

    void* find(int id);
    void* find(const String& name);

public:
    Resource();
    ~Resource();
    void clear();
    void list();
    void load(const String& filename);
    void load(FileObject& file);
    void load(const ByteArrayPtr& memory);
    /* Bei Load wird die gesamte Datei in den Speicher geladen und erst
     * beim Loeschen der Resource wieder freigegeben. Die angegebene Datei
     * kann vorher geschlossen werden
     */
    void useMemory(const ByteArrayPtr& memory);
    void useMemory(void* ptr, size_t size);
    /* Wird UseMemory benutzt, muss ein Pointer auf einen Speicherbereich
     * angegeben werden, der seine Gueltigkeit nicht verlieren darf,
     * solange die Instanz von Resource existiert.
     */

    FileObject* getFile(int id);
    FileObject* getFile(const String& name);
    ByteArrayPtr getMemory(int id);
    ByteArrayPtr getMemory(const String& name);

    static Resource* getPPLResource();
    static void generateResourceHeader(const String& basispfad, const String& ConfigParser, const String& targetfile, const String& label);
};

Resource* GetPPLResource();

} // namespace pplib

#endif /* PPLIB_CORE_DIR_H_ */