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

#include <string.h>
#include <ppl7/types/bytearrayptr.h>
#include <ppl7/types/string.h>
#include <ppl7/types/widestring.h>
#include <ppl7/functions.h>
#include <ppl7/exceptions.h>

namespace ppl7
{

ByteArrayPtr::ByteArrayPtr()
{
    ptradr = NULL;
    ptrsize = 0;
}

ByteArrayPtr::ByteArrayPtr(const ByteArrayPtr& other)
{
    ptradr = other.ptradr;
    ptrsize = other.ptrsize;
}

ByteArrayPtr::ByteArrayPtr(const String& data)
{
    ptradr = (void*)data.getPtr();
    ptrsize = data.size();
}

ByteArrayPtr::ByteArrayPtr(const WideString& data)
{
    ptradr = (void*)data.getPtr();
    ptrsize = data.size();
}

ByteArrayPtr::ByteArrayPtr(const void* adr, size_t size)
{
    ptradr = (void*)adr;
    ptrsize = size;
}

bool ByteArrayPtr::isNull() const
{
    if (!ptradr || !ptrsize) return true;
    return false;
}

bool ByteArrayPtr::isEmpty() const
{
    if (!ptradr || !ptrsize) return true;
    return false;
}

bool ByteArrayPtr::notEmpty() const
{
    if (ptradr && ptrsize) return true;
    return false;
}

size_t ByteArrayPtr::size() const
{
    if (!ptradr) return 0;
    return ptrsize;
}

const void* ByteArrayPtr::adr() const
{
    return ptradr;
}

const char* ByteArrayPtr::map(size_t position, size_t size) const
{
    if (!ptradr) throw OutOfBoundsException("ByteArrayPtr::map has no memory assigned");
    if (position > ptrsize || size > ptrsize - position)
        throw OutOfBoundsException("ByteArrayPtr::map position (%zu) + size (%zu) exceeds size of ByteArray (%zu > %zu)", position, size,
                                   position + size, ptrsize);
    return (const char*)ptradr + position;
}

void ByteArrayPtr::truncate(size_t position)
{
    if (position > ptrsize)
        throw OutOfBoundsException("ByteArrayPtr::truncate position exceeds size of ByteArray (%zu > %zu)", position, ptrsize);
    ptrsize = position;
}

const void* ByteArrayPtr::ptr() const
{
    return ptradr;
}

void ByteArrayPtr::use(void* adr, size_t size)
{
    ptradr = adr;
    ptrsize = size;
}

void ByteArrayPtr::use(const ByteArrayPtr& other)
{
    ptradr = other.ptradr;
    ptrsize = other.ptrsize;
}

ByteArrayPtr& ByteArrayPtr::operator=(const ByteArrayPtr& other)
{
    ptradr = other.ptradr;
    ptrsize = other.ptrsize;
    return *this;
}

ByteArrayPtr::operator const void*() const
{
    return ptradr;
}

ByteArrayPtr::operator const unsigned char*() const
{
    return (const unsigned char*)ptradr;
}

ByteArrayPtr::operator const char*() const
{
    return (const char*)ptradr;
}

unsigned char ByteArrayPtr::operator[](size_t pos) const
{
    if (ptradr != NULL && pos < ptrsize) return ((unsigned char*)ptradr)[pos];
    throw OutOfBoundsException();
}

unsigned char& ByteArrayPtr::operator[](size_t pos)
{
    if (ptradr != NULL && pos < ptrsize) return static_cast<unsigned char*>(ptradr)[pos];
    throw OutOfBoundsException();
}

unsigned char ByteArrayPtr::operator[](int pos) const
{
    if (ptradr != NULL && pos < ptrsize) return ((unsigned char*)ptradr)[pos];
    throw OutOfBoundsException();
}

unsigned char& ByteArrayPtr::operator[](int pos)
{
    if (ptradr != NULL && pos < ptrsize) return static_cast<unsigned char*>(ptradr)[pos];
    throw OutOfBoundsException();
}

void ByteArrayPtr::set(size_t pos, unsigned char value)
{
    if (ptradr == NULL || pos >= ptrsize) throw OutOfBoundsException();
    ((unsigned char*)ptradr)[pos] = value;
}

unsigned char ByteArrayPtr::get(size_t pos) const
{
    if (ptradr != NULL && pos < ptrsize) return ((unsigned char*)ptradr)[pos];
    throw OutOfBoundsException();
}

String ByteArrayPtr::toString() const
{
    if (!ptradr) return String();
    return String((const char*)ptradr, ptrsize);
}

WideString ByteArrayPtr::toWideString() const
{
    if (!ptradr) return WideString();
    return WideString((const wchar_t*)ptradr, ptrsize / sizeof(wchar_t));
}

String ByteArrayPtr::toHex() const
{
    String str;
    if (!ptradr || ptrsize == 0) return str;
    str.reserve(ptrsize * 2);
    for (size_t i = 0; i < ptrsize; i++)
        str.appendf("%02x", ((unsigned char*)ptradr)[i]);
    return str;
}

String ByteArrayPtr::toBase64() const
{
    return ToBase64(*this);
}

const char* ByteArrayPtr::toCharPtr() const
{
    return (const char*)ptradr;
}

uint32_t ByteArrayPtr::crc32() const
{
    if (ptrsize == 0) throw EmptyDataException();
    return Crc32(ptradr, ptrsize);
}

void ByteArrayPtr::hexDump() const
{
    hexDump(0, ptrsize);
}

void ByteArrayPtr::hexDump(size_t bytes) const
{
    hexDump(0, bytes);
}

void ByteArrayPtr::hexDump(size_t offset, size_t bytes) const
{
    if (!ptradr || ptrsize == 0) {
        PrintDebug("HEXDUMP of ByteArray: No Data, address or size is 0\n");
        return;
    }
    if (offset >= ptrsize) {
        PrintDebug("HEXDUMP of ByteArray: Offset %zu exceeds size of ByteArray (%zu)\n", offset, ptrsize);
        return;
    }
    if (bytes > ptrsize - offset) bytes = ptrsize - offset;
    char* start = (char*)ptradr + offset;
    PrintDebug("HEXDUMP of ByteArray: %zu Bytes starting at Address %p:\n", bytes, start);
    HexDump(start, bytes, true);
}

void ByteArrayPtr::memset(int value)
{
    if (ptradr != NULL && ptrsize > 0) ::memset(ptradr, value, ptrsize);
}

int ByteArrayPtr::memcmp(const ByteArrayPtr& other) const
{
    size_t min = ptrsize;
    if (other.ptrsize < min) min = other.ptrsize;
    if (min > 0 && ptradr != NULL && other.ptradr != NULL) {
        int res = ::memcmp(ptradr, other.ptradr, min);
        if (res != 0) return res;
    }
    if (ptrsize < other.ptrsize) return -1;
    if (ptrsize > other.ptrsize) return 1;
    return 0;
}

std::ostream& operator<<(std::ostream& s, const ByteArrayPtr& ba)
{
    ppl7::String hex = ba.toHex();
    return s.write((const char*)hex, hex.size());
}

} // namespace ppl7
