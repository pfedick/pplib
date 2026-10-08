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
#include <pplib/core/memfile.h>
#include <pplib/core/fileobject.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/exceptions.h>

namespace pplib
{

MemFile::MemFile() noexcept
{
    mysize = 0;
    pos = 0;
    MemBase = nullptr;
    maxsize = 0;
    readonly = false;
    is_open = true;
}

MemFile::MemFile(void* adresse, size_t size, bool writeable)
    : MemFile()
{
    open(adresse, size, writeable);
}

MemFile::MemFile(const ByteArrayPtr& memory)
    : MemFile()
{
    open(memory);
}

MemFile::MemFile(MemFile&& other) noexcept
{
    mysize = other.mysize;
    pos = other.pos;
    maxsize = other.maxsize;
    MemBase = other.MemBase;
    writebuffer = std::move(other.writebuffer);
    readonly = other.readonly;
    is_open = other.is_open;
    setFilename(other.filename());
    other.setFilename("");

    other.mysize = 0;
    other.pos = 0;
    other.maxsize = 0;
    other.writebuffer.clear();
    other.MemBase = nullptr;
    other.readonly = true;
    other.is_open = false;
}

MemFile& MemFile::operator=(MemFile&& other) noexcept
{
    if (this != &other) {
        close();
        mysize = other.mysize;
        pos = other.pos;
        maxsize = other.maxsize;
        MemBase = other.MemBase;
        writebuffer = std::move(other.writebuffer);
        readonly = other.readonly;
        is_open = other.is_open;
        setFilename(other.filename());
        other.setFilename("");

        other.mysize = 0;
        other.pos = 0;
        other.maxsize = 0;
        other.MemBase = nullptr;
        other.writebuffer.clear();
        other.readonly = true;
        other.is_open = false;
    }
    return *this;
}

MemFile::~MemFile()
{
    close();
}

void MemFile::open(void* adresse, size_t size, bool writeable)
{
    close();
    MemBase = (char*)adresse;
    mysize = size;
    pos = 0;
    if (writeable == true) {
        if (adresse != nullptr && size > 0) writebuffer.useadr(adresse, size);
        readonly = false;
    } else {
        if (adresse == nullptr && size == 0) throw IllegalArgumentException();
        readonly = true;
    }
    is_open = true;
}

void MemFile::open(const ByteArrayPtr& memory)
{
    if (memory.isEmpty()) throw IllegalArgumentException();
    open((void*)memory.adr(), memory.size(), false);
}

void MemFile::openReadWrite(void* adresse, size_t size)
{
    open(adresse, size, true);
}

void MemFile::setMaxSize(size_t size)
{
    maxsize = size;
}

size_t MemFile::maxSize() const
{
    return maxsize;
}

void MemFile::resizeBuffer(size_t size)
{
    if (readonly) throw ReadOnlyException();
    if (maxsize > 0 && size > maxsize) throw BufferExceedsLimitException();
    size_t newsize = (((size + 8191) >> 13) << 13);
    if (size > SIZE_MAX - 8191) throw OverflowException();
    if (newsize > writebuffer.size()) {
        // pplib::PrintDebug("MemFile::resizeBuffer, old size: %d, requested size: %d, new size: %d\n", (int)buffersize, size, newsize);
        MemBase = (char*)writebuffer.realloc(newsize);
    }
    mysize = size;
    if (pos > mysize) pos = mysize;
}

void MemFile::close()
{
    MemBase = nullptr;
    mysize = 0;
    pos = 0;
    writebuffer.clear();
    readonly = true;
    is_open = false;
}

uint64_t MemFile::size() const
{
    if (!isOpen()) throw FileNotOpenException();
    return (uint64_t)mysize;
}

void MemFile::rewind()
{
    seek(0);
}

void MemFile::seek(uint64_t position)
{
    if (!isOpen()) throw FileNotOpenException();
    if (position <= mysize) {
        pos = position;
    } else {
        throw FileSeekException();
    }
}

uint64_t MemFile::seek(int64_t offset, SeekOrigin origin)
{
    if (!isOpen()) throw FileNotOpenException();
    int64_t newpos = 0;
    switch (origin) {
    case SEEKCUR:
        newpos = (int64_t)pos + offset;
        break;
    case SEEKEND:
        newpos = (int64_t)mysize + offset;
        break;
    case SEEKSET:
        newpos = offset;
        break;
    default:
        throw IllegalArgumentException();
    }
    if (newpos < 0) throw InvalidArgumentsException();
    if ((uint64_t)newpos > mysize) {
        throw FileSeekException("pos=%lld, offset=%lld, origin=%d", (uint64_t)pos, offset, origin);
    }
    pos = (size_t)newpos;
    return pos;
}

uint64_t MemFile::tell()
{
    if (!isOpen()) throw FileNotOpenException();
    return pos;
}

size_t MemFile::fread(void* ptr, size_t size, size_t nmemb)
{
    if (!isOpen()) throw FileNotOpenException();
    if (ptr == nullptr) throw IllegalArgumentException();
    if (size == 0 || nmemb == 0) return 0;
    if (pos >= mysize) throw EndOfFileException();
    size_t by = nmemb;
    if (pos + (by * size) > mysize) by = (size_t)(mysize - pos) / size;
    if (by == 0) throw EndOfFileException();
    memmove(ptr, MemBase + pos, by * size);
    pos += (by * size);
    return by;
}

size_t MemFile::fwrite(const void* ptr, size_t size, size_t nmemb)
{
    if (!isOpen()) throw FileNotOpenException();
    if (size == 0 || nmemb == 0) return 0;
    if (ptr == nullptr) throw IllegalArgumentException();
    if (readonly) throw ReadOnlyException();
    if (size > 0 && nmemb > SIZE_MAX / size) throw OverflowException();
    size_t bytes = nmemb * size;
    if (bytes > SIZE_MAX - pos) throw OverflowException();
    if (pos + bytes > mysize) resizeBuffer(pos + bytes);
    memmove(MemBase + pos, ptr, bytes);
    pos += bytes;
    return nmemb;
}

char* MemFile::fgets(char* buffer1, size_t num)
{
    if (buffer1 == nullptr || num == 0) throw IllegalArgumentException();
    if (!isOpen()) throw FileNotOpenException();
    if (pos >= mysize) return nullptr;

    size_t by = num - 1;
    if (pos + by > mysize) by = mysize - pos;
    const char* ptr = MemBase + pos;
    size_t i;
    for (i = 0; i < by; i++) {
        if ((buffer1[i] = ptr[i]) == '\n') {
            i++;
            break;
        }
    }
    buffer1[i] = '\0';
    pos += i;
    return buffer1;
}

wchar_t* MemFile::fgetws(wchar_t* buffer1, size_t num)
{
    if (buffer1 == nullptr || num == 0) throw IllegalArgumentException();
    if (!isOpen()) throw FileNotOpenException();
    if (pos >= mysize) return nullptr;

    size_t available_wchars = (mysize - pos) / sizeof(wchar_t);
    if (available_wchars == 0) return nullptr;

    size_t max_read = (num - 1 < available_wchars) ? (num - 1) : available_wchars;
    size_t i = 0;
    for (i = 0; i < max_read; i++) {
        wchar_t ch;
        memcpy(&ch, MemBase + pos + (i * sizeof(wchar_t)), sizeof(wchar_t));
        buffer1[i] = ch;
        if (ch == L'\n') {
            i++;
            break;
        }
    }
    buffer1[i] = L'\0';
    pos += i * sizeof(wchar_t);
    return buffer1;
}

void MemFile::fputs(const char* str)
{
    if (!str) throw IllegalArgumentException();
    if (!isOpen()) throw FileNotOpenException();
    fwrite(str, 1, strlen(str));
}

void MemFile::fputws(const wchar_t* str)
{
    if (!str) throw IllegalArgumentException();
    if (!isOpen()) throw FileNotOpenException();
    fwrite(str, sizeof(wchar_t), wcslen(str));
}

void MemFile::fputc(int c)
{
    char buf[1];
    buf[0] = c;
    fwrite(buf, 1, 1);
}

void MemFile::fputwc(wchar_t c)
{
    wchar_t buf[1];
    buf[0] = c;
    fwrite(buf, sizeof(wchar_t), 1);
}

int MemFile::fgetc()
{
    if (!isOpen()) throw FileNotOpenException();
    if (pos >= mysize) return EOF;
    return static_cast<unsigned char>(MemBase[pos++]);
}

wchar_t MemFile::fgetwc()
{
    if (!isOpen()) throw FileNotOpenException();
    if (pos + sizeof(wchar_t) > mysize) return static_cast<wchar_t>(WEOF);
    wchar_t ch;
    memcpy(&ch, MemBase + pos, sizeof(wchar_t));
    pos += sizeof(wchar_t);
    return ch;
}

bool MemFile::eof() const
{
    if (!isOpen()) throw FileNotOpenException();
    if (pos >= mysize) return true;
    return false;
}

char* MemFile::adr(size_t adresse)
{
    if (!isOpen()) throw FileNotOpenException();
    if (adresse > mysize) throw OutOfBoundsException();
    return (MemBase + adresse);
}

void MemFile::setMapReadAhead(size_t bytes)
{
}

char* MemFile::map(uint64_t position, size_t size, MapProtection prot)
{
    if (!isOpen()) throw FileNotOpenException();
    if (prot == MapProtection::READWRITE && readonly) throw ReadOnlyException();
    if (position > mysize || size > mysize - position) {
        throw OverflowException();
    }
    return (MemBase + position);
}

void MemFile::unmap()
{
    return;
}

void MemFile::flush()
{
    return;
}

void MemFile::sync()
{
    return;
}

int MemFile::getFileNo() const
{
    throw OperationUnavailableException();
}

void MemFile::truncate(uint64_t length)
{
    if (!isOpen()) throw FileNotOpenException();
    if (readonly) throw ReadOnlyException();
    if (length > SIZE_MAX) throw OverflowException();
    if (length < mysize) {
        resizeBuffer(length);
        return;
    } else if (length == mysize)
        return;
    size_t oldsize = mysize;
    size_t increase = length - mysize;
    resizeBuffer(length);
    memset(MemBase + oldsize, 0, increase);
}

void MemFile::lockShared(bool block)
{
    throw OperationUnavailableException();
}

void MemFile::lockExclusive(bool block)
{
    throw OperationUnavailableException();
}

void MemFile::unlock()
{
    throw OperationUnavailableException();
}

} // end of namespace pplib
