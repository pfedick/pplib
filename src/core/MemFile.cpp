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

MemFile::MemFile()
{
    buffer = NULL;
    mysize = 0;
    pos = 0;
    MemBase = NULL;
    readonly = false;
    maxsize = 0;
    buffersize = 0;
}

MemFile::MemFile(void* adresse, size_t size, bool writeable)
    : MemFile()
{
    open(adresse, size, writeable);
}

MemFile::MemFile(const ByteArrayPtr& memory)
    : MemFile()
{
    if (memory.isEmpty()) {
        throw IllegalArgumentException();
    }
    MemBase = (char*)memory.adr();
    mysize = memory.size();
    readonly = true;
}

MemFile::MemFile(MemFile&& other) noexcept
{
    mysize = other.mysize;
    pos = other.pos;
    maxsize = other.maxsize;
    buffersize = other.buffersize;
    MemBase = other.MemBase;
    buffer = other.buffer;
    readonly = other.readonly;

    other.mysize = 0;
    other.pos = 0;
    other.maxsize = 0;
    other.buffersize = 0;
    other.MemBase = nullptr;
    other.buffer = nullptr;
    other.readonly = true;
}

MemFile& MemFile::operator=(MemFile&& other) noexcept
{
    if (this != &other) {
        close();
        mysize = other.mysize;
        pos = other.pos;
        maxsize = other.maxsize;
        buffersize = other.buffersize;
        MemBase = other.MemBase;
        buffer = other.buffer;
        readonly = other.readonly;

        other.mysize = 0;
        other.pos = 0;
        other.maxsize = 0;
        other.buffersize = 0;
        other.MemBase = nullptr;
        other.buffer = nullptr;
        other.readonly = true;
    }
    return *this;
}

MemFile::~MemFile()
{
    close();
}

void MemFile::open(void* adresse, size_t size, bool writeable)
{
    // if (adresse==NULL || size==0) throw IllegalArgumentException();
    close();
    MemBase = (char*)adresse;
    mysize = size;
    pos = 0;
    if (writeable == true) {
        buffer = MemBase;
        readonly = false;
        buffersize = size;
    } else {
        buffersize = 0;
        readonly = true;
    }
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
    if (newsize > buffersize) {
        // pplib::PrintDebug("MemFile::resizeBuffer, old size: %d, requested size: %d, new size: %d\n", (int)buffersize, size, newsize);
        char* buf = (char*)realloc(buffer, newsize);
        if (!buf) throw OutOfMemoryException();
        buffer = buf;
        MemBase = buf;
        buffersize = newsize;
    }
    mysize = size;
    if (pos > mysize) pos = mysize;
}

bool MemFile::isOpen() const
{
    if (MemBase != NULL) return true;
    return false;
}

void MemFile::close()
{
    MemBase = NULL;
    mysize = 0;
    pos = 0;
    buffersize = 0;
    readonly = true;
    if (buffer != 0) {
        free(buffer);
        buffer = 0;
    }
}

uint64_t MemFile::size() const
{
    return (int64_t)mysize;
}

void MemFile::rewind()
{
    pos = 0;
}

void MemFile::seek(uint64_t position)
{
    if (MemBase != NULL || readonly == false) {
        if (position <= mysize) {
            pos = position;
        } else {
            throw OverflowException();
        }
        return;
    }
    throw FileNotOpenException();
}

uint64_t MemFile::seek(int64_t offset, SeekOrigin origin)
{
    if (MemBase != NULL || readonly == false) {
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
    throw FileNotOpenException();
}

uint64_t MemFile::tell()
{
    if (MemBase != NULL || readonly == false) {
        return pos;
    }
    throw FileNotOpenException();
}

size_t MemFile::fread(void* ptr, size_t size, size_t nmemb)
{
    if (MemBase == NULL) throw FileNotOpenException();
    if (ptr == NULL) throw IllegalArgumentException();
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
    if (size == 0 || nmemb == 0) return 0;
    if (ptr == NULL) throw IllegalArgumentException();
    if (MemBase == NULL && readonly == true) throw FileNotOpenException();
    if (readonly) throw ReadOnlyException();
    if (size > 0 && nmemb > SIZE_MAX / size) throw OverflowException();
    size_t bytes = nmemb * size;
    if (pos + bytes > mysize) resizeBuffer(pos + bytes);
    memmove(MemBase + pos, ptr, bytes);
    pos += bytes;
    return bytes;
}

char* MemFile::fgets(char* buffer1, size_t num)
{
    if (buffer1 == nullptr || num == 0) throw IllegalArgumentException();
    if (MemBase != NULL) {
        if (pos >= mysize) throw EndOfFileException();
        uint64_t by;
        by = num - 1;
        if (pos + by > mysize) by = (uint64_t)(mysize - pos);
        char* ptr = MemBase + pos;
        uint64_t i;
        for (i = 0; i < by; i++) {
            if ((buffer1[i] = ptr[i]) == '\n') {
                i++;
                break;
            }
        }
        buffer1[i] = 0;
        pos += i;
        if (pos > mysize) pos = mysize; // EndOfFileException erfolgt beim nächsten Leseversuch
        return buffer1;
    }
    throw FileNotOpenException();
}

wchar_t* MemFile::fgetws(wchar_t* buffer1, size_t num)
{
    if (buffer1 == nullptr || num == 0) throw IllegalArgumentException();
    if (MemBase == NULL) throw FileNotOpenException();
    if (pos >= mysize) throw EndOfFileException();

    size_t available_wchars = (mysize - pos) / sizeof(wchar_t);
    size_t max_read = (num - 1 < available_wchars) ? (num - 1) : available_wchars;

    size_t i = 0;
    for (i = 0; i < max_read; i++) {
        wchar_t ch; // temporärer Speicher für das gelesene wchar_t, korrekt aligned für wchar_t
        memcpy(&ch, MemBase + pos + (i * sizeof(wchar_t)), sizeof(wchar_t));
        buffer1[i] = ch;
        if (ch == L'\n') {
            i++;
            break;
        }
    }
    buffer1[i] = 0;
    pos += i * sizeof(wchar_t);
    return buffer1;
}

void MemFile::fputs(const char* str)
{
    if (!str) throw IllegalArgumentException();
    if (MemBase != NULL || readonly == false) {
        fwrite((void*)str, 1, (uint32_t)strlen(str));
        return;
    }
    throw FileNotOpenException();
}

void MemFile::fputws(const wchar_t* str)
{
    if (!str) throw IllegalArgumentException();
    if (MemBase != NULL || readonly == false) {
        fwrite(str, 1, (uint32_t)wcslen(str) * sizeof(wchar_t));
        return;
    }
    throw FileNotOpenException();
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
    if (MemBase == NULL) throw FileNotOpenException();
    if (pos >= mysize) throw EndOfFileException();
    return static_cast<unsigned char>(MemBase[pos++]);
}

wchar_t MemFile::fgetwc()
{
    wchar_t buf[1];
    size_t n = fread(buf, sizeof(wchar_t), 1);
    if (n == 0) throw EndOfFileException();
    return buf[0];
}

bool MemFile::eof() const
{
    if (MemBase != NULL || readonly == false) {
        if (pos >= mysize) return true;
        return false;
    }
    throw FileNotOpenException();
}

char* MemFile::adr(size_t adresse)
{
    if (MemBase != NULL) {
        return (MemBase + adresse);
    }
    throw FileNotOpenException();
}

void MemFile::setMapReadAhead(size_t bytes)
{
}

char* MemFile::map(uint64_t position, size_t size, MapProtection prot)
{
    if (MemBase == NULL) throw FileNotOpenException();
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
    if (readonly) throw ReadOnlyException();
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
