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

#ifndef PPLIB_CORE_FILES_H_
#define PPLIB_CORE_FILES_H_

#include <pplib/types/string.h>

namespace pplib
{

class FileObject
{
private:
    String MyFilename;

protected:
    char* buffer;

public:
    enum SeekOrigin
    {
        SEEKCUR = 1,
        SEEKEND,
        SEEKSET
    };

    FileObject();
    virtual ~FileObject();

    void setFilename(const char* filename);
    void setFilename(const String& filename);
    const String& filename() const;

    size_t write(const void* source, size_t bytes, uint64_t fileposition);
    size_t write(const void* source, size_t bytes);
    size_t write(const ByteArrayPtr& object, size_t bytes = 0);
    size_t read(void* target, size_t bytes, uint64_t fileposition);
    size_t read(void* target, size_t bytes);
    size_t read(ByteArray& target, size_t bytes);
    uint64_t copyFrom(FileObject& quellfile, uint64_t quelloffset, uint64_t bytes, uint64_t zieloffset);
    uint64_t copyFrom(FileObject& quellfile, uint64_t bytes);
    int gets(String& buffer, size_t num = 1024);
    int getws(String& buffer, size_t num = 1024);
    String gets(size_t num = 1024);
    String getws(size_t num = 1024);
    void putsf(const char* fmt, ...);
    void puts(const String& str);
    void putws(const WideString& str);
    const char* map();
    char* load();
    int load(ByteArray& target);
    String md5();

    // Virtuelle Funktionen
    virtual void close();
    virtual void seek(uint64_t position);
    virtual uint64_t seek(int64_t offset, SeekOrigin origin);
    virtual uint64_t tell();
    virtual void rewind();
    virtual size_t fread(void* ptr, size_t size, size_t nmemb);
    virtual size_t fwrite(const void* ptr, size_t size, size_t nmemb);
    virtual char* fgets(char* buffer, size_t num);
    virtual wchar_t* fgetws(wchar_t* buffer, size_t num = 1024);
    virtual void fputs(const char* str);
    virtual void fputws(const wchar_t* str);
    virtual void fputc(int c);
    virtual int fgetc();
    virtual void fputwc(wchar_t c);
    virtual wchar_t fgetwc();
    virtual bool eof() const;
    virtual uint64_t size() const;
    virtual const char* map(uint64_t position, size_t size);
    virtual char* mapRW(uint64_t position, size_t size);
    virtual void unmap();
    virtual void setMapReadAhead(size_t bytes);
    virtual int getFileNo() const;
    virtual void flush();
    virtual void sync();
    virtual void truncate(uint64_t length);
    virtual bool isOpen() const;
    virtual void lockShared(bool block = true);
    virtual void lockExclusive(bool block = true);
    virtual void unlock();
};

} // namespace pplib

#endif /* PPLIB_CORE_THREADS_H_ */