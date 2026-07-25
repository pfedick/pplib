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

#ifndef PPL7_CORE_GZFILE_H_
#define PPL7_CORE_GZFILE_H_

#include <ppl7/types/string.h>
#include <ppl7/types/bytearray.h>
#include <ppl7/types/bytearrayptr.h>
#include <ppl7/core/fileobject.h>
#include <ppl7/core/file.h>

namespace ppl7
{

class GzFile : public FileObject
{
private:
    File* fh;
    void* ff;

public:
private:
    void throwErrno(int e);
    void throwErrno(int e, const String& filename);

public:
    GzFile();
    GzFile(const String& filename, File::FileMode mode = File::FileMode::READ);
    GzFile(int fd);
    virtual ~GzFile();

    void open(const String& filename, File::FileMode mode = File::FileMode::READ);
    void open(const char* filename, File::FileMode mode = File::FileMode::READ);
    void open(int fd, File::FileMode mode = File::FileMode::READ);

    // Virtuelle Funktionen
    virtual void close();
    virtual void rewind();
    virtual void seek(uint64_t position);
    virtual uint64_t seek(int64_t offset, SeekOrigin origin);
    virtual uint64_t tell();
    virtual bool eof() const;
    virtual bool isOpen() const;
    virtual size_t fread(void* ptr, size_t size, size_t nmemb);
    virtual char* fgets(char* buffer, size_t num);
    virtual int fgetc();
    virtual size_t fwrite(const void* ptr, size_t size, size_t nmemb);
};

} // namespace ppl7

#endif /* PPL7_CORE_GZFILE_H_ */