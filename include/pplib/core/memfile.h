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

#ifndef PPLIB_CORE_MEMFILE_H_
#define PPLIB_CORE_MEMFILE_H_

#include <pplib/core/fileobject.h>

namespace pplib
{
class MemFile : public FileObject
{
private:
    size_t mysize;
    size_t pos;
    size_t maxsize;
    size_t buffersize;
    char* MemBase;
    char* buffer;
    bool readonly;

    void resizeBuffer(size_t size);

public:
    MemFile();
    MemFile(void* adresse, size_t size, bool writeable = false);
    MemFile(const ByteArrayPtr& memory);
    ~MemFile();

    MemFile(const MemFile&) = delete;
    MemFile& operator=(const MemFile&) = delete;
    MemFile(MemFile&& other) noexcept;
    MemFile& operator=(MemFile&& other) noexcept;

    void open(void* adresse, size_t size, bool writeable = false);
    void open(const ByteArrayPtr& memory);
    void openReadWrite(void* adresse, size_t size);
    char* adr(size_t adresse);
    void setMaxSize(size_t size);
    size_t maxSize() const;

    // Virtuelle Funktionen
    void close() override;
    void rewind() override;
    void seek(uint64_t position) override;
    uint64_t seek(int64_t offset, SeekOrigin origin) override;
    uint64_t tell() override;
    size_t fread(void* ptr, size_t size, size_t nmemb) override;
    size_t fwrite(const void* ptr, size_t size, size_t nmemb) override;
    char* fgets(char* buffer, size_t num) override;
    wchar_t* fgetws(wchar_t* buffer, size_t num = 1024) override;
    void fputc(int c) override;
    int fgetc() override;
    void fputwc(wchar_t c) override;
    wchar_t fgetwc() override;
    void fputs(const char* str) override;
    void fputws(const wchar_t* str) override;
    bool eof() const override;
    uint64_t size() const override;
    char* map(uint64_t position, size_t size, MapProtection prot = MapProtection::READ) override;
    void unmap() override;
    void setMapReadAhead(size_t bytes) override;
    int getFileNo() const override;
    void flush() override;
    void sync() override;
    void truncate(uint64_t length) override;
    bool isOpen() const override;
    void lockShared(bool block = true) override;
    void lockExclusive(bool block = true) override;
    void unlock() override;
};

} // namespace pplib

#endif /* PPLIB_CORE_MEMFILE_H_ */