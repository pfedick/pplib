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

#include <ppl7/types/bytearray.h>
#include <ppl7/exceptions.h>
#include <ppl7/types/string.h>
#include <ppl7/types/widestring.h>

namespace ppl7
{

ByteArray::ByteArray()
{
    ptradr = NULL;
    ptrsize = 0;
}

ByteArray::~ByteArray()
{
    ::free(ptradr);
    ptradr = NULL;
}

ByteArray::ByteArray(const ByteArrayPtr& other)
{
    ptradr = NULL;
    ptrsize = 0;
    copy(other.ptradr, other.ptrsize);
}

ByteArray::ByteArray(ByteArray&& other)
{
    ptradr = other.ptradr;
    ptrsize = other.ptrsize;
    other.ptradr = NULL;
    other.ptrsize = 0;
}

ByteArray::ByteArray(const void* adr, size_t size)
{
    ptradr = NULL;
    ptrsize = 0;
    copy(adr, size);
}

ByteArray::ByteArray(const String& str)
{
    ptradr = NULL;
    ptrsize = 0;
    if (str.notEmpty()) {
        copy(str.getPtr(), str.size());
    }
}

ByteArray::ByteArray(const WideString& str)
{
    ptradr = NULL;
    ptrsize = 0;
    copy(str.getPtr(), str.size() * sizeof(wchar_t));
}

ByteArray::ByteArray(size_t size)
{
    ptradr = NULL;
    ptrsize = 0;
    malloc(size);
}

void ByteArray::useadr(void* adr, size_t size)
{
    ::free(ptradr);
    ptradr = adr;
    ptrsize = size;
}

void* ByteArray::copy(const void* adr, size_t size)
{
    ::free(ptradr);
    ptrsize = 0;
    ptradr = NULL;
    if (adr != NULL && size > 0) {
        ptradr = ::malloc(size + 4);
        if (!ptradr) {
            throw OutOfMemoryException();
        }
        memcpy(ptradr, adr, size);
        ptrsize = size;
        ((char*)ptradr)[ptrsize] = 0;
        ((char*)ptradr)[ptrsize + 1] = 0;
        ((char*)ptradr)[ptrsize + 2] = 0;
        ((char*)ptradr)[ptrsize + 3] = 0;
    }
    return ptradr;
}

void* ByteArray::copy(const ByteArrayPtr& other)
{
    return copy(other.ptradr, other.ptrsize);
}

void* ByteArray::append(const void* adr, size_t size)
{
    if (adr == NULL || size == 0) {
        return ptradr;
    }

    if (!ptradr) return copy(adr, size);
    size_t newsize = ptrsize + size;
    void* p = ::realloc(ptradr, newsize + 4);
    if (!p) throw OutOfMemoryException();
    ptradr = p;
    void* target = (char*)ptradr + ptrsize;
    memcpy(target, adr, size);
    ptrsize = newsize;
    ((char*)ptradr)[ptrsize] = 0;
    ((char*)ptradr)[ptrsize + 1] = 0;
    ((char*)ptradr)[ptrsize + 2] = 0;
    ((char*)ptradr)[ptrsize + 3] = 0;
    return ptradr;
}

void* ByteArray::append(const ByteArrayPtr& other)
{
    return append(other.ptradr, other.ptrsize);
}

void* ByteArray::prepend(const void* adr, size_t size)
{
    if (adr == NULL || size == 0) {
        return ptradr;
    }

    if (!ptradr) return copy(adr, size);
    size_t newsize = ptrsize + size;
    void* p = ::malloc(newsize + 4);
    if (!p) throw OutOfMemoryException();
    memcpy(p, adr, size);
    void* target = (char*)p + size;
    memcpy(target, ptradr, ptrsize);
    ::free(ptradr);
    ptradr = p;
    ptrsize = newsize;
    ((char*)ptradr)[ptrsize] = 0;
    ((char*)ptradr)[ptrsize + 1] = 0;
    ((char*)ptradr)[ptrsize + 2] = 0;
    ((char*)ptradr)[ptrsize + 3] = 0;

    return ptradr;
}

void* ByteArray::prepend(const ByteArrayPtr& other)
{
    return prepend(other.ptradr, other.ptrsize);
}

ByteArray& ByteArray::operator=(const ByteArrayPtr& other)
{
    copy(other.ptradr, other.ptrsize);
    return *this;
}

ByteArray& ByteArray::operator=(ByteArray&& other)
{
    if (this != &other) {
        ::free(ptradr);
        ptradr = other.ptradr;
        ptrsize = other.ptrsize;
        other.ptradr = NULL;
        other.ptrsize = 0;
    }
    return *this;
}

ByteArray& ByteArray::operator=(const String& str)
{
    free();
    copy(str.getPtr(), str.size());
    return *this;
}

ByteArray& ByteArray::operator=(const WideString& str)
{
    free();
    copy(str.getPtr(), str.size() * sizeof(wchar_t));
    return *this;
}

ByteArray& ByteArray::operator+=(const ByteArrayPtr& other)
{
    append(other.ptradr, other.ptrsize);
    return *this;
}

ByteArray& ByteArray::operator+=(const String& other)
{
    append(other.getPtr(), other.size());
    return *this;
}

ByteArray& ByteArray::operator+=(const WideString& other)
{
    append(other.getPtr(), other.size() * sizeof(wchar_t));
    return *this;
}

void* ByteArray::malloc(size_t size)
{
    ::free(ptradr);
    ptradr = ::malloc(size + 4);
    if (ptradr) {
        ptrsize = size;
    } else {
        throw OutOfMemoryException();
    }
    ((char*)ptradr)[ptrsize] = 0;
    ((char*)ptradr)[ptrsize + 1] = 0;
    ((char*)ptradr)[ptrsize + 2] = 0;
    ((char*)ptradr)[ptrsize + 3] = 0;
    return ptradr;
}

void* ByteArray::calloc(size_t size)
{
    ::free(ptradr);
    ptradr = ::calloc(size + 4, 1);
    if (ptradr) {
        ptrsize = size;
    } else {
        throw OutOfMemoryException();
    }
    return ptradr;
}

void ByteArray::free()
{
    ::free(ptradr);
    ptradr = NULL;
    ptrsize = 0;
}

void ByteArray::clear()
{
    ::free(ptradr);
    ptradr = NULL;
    ptrsize = 0;
}

void* ByteArray::fromHex(const String& hex)
{
    free();
    if (hex.isEmpty()) {
        throw IllegalArgumentException("Empty string");
    }
    size_t chars = hex.size();
    if ((chars & 1) == 1) {
        throw IllegalArgumentException("uneven number of characters");
    }
    malloc(chars >> 1);
    unsigned char* t = (unsigned char*)ptradr;
    unsigned char value;
    for (size_t source = 0, target = 0; source < chars; source += 2, target++) {
        wchar_t first = hex[source];
        wchar_t second = hex[source + 1];
        if (first >= '0' && first <= '9')
            value = (first - '0');
        else if (first >= 'a' && first <= 'f')
            value = (first - 'a' + 10);
        else if (first >= 'A' && first <= 'F')
            value = (first - 'A' + 10);
        else {
            free();
            throw IllegalArgumentException("invalid chars in input string");
        }
        value = value << 4;
        if (second >= '0' && second <= '9')
            value |= (second - '0');
        else if (second >= 'a' && second <= 'f')
            value |= (second - 'a' + 10);
        else if (second >= 'A' && second <= 'F')
            value |= (second - 'A' + 10);
        else {
            free();
            throw IllegalArgumentException("invalid chars in input string");
        }
        t[target] = value;
    }
    return ptradr;
}

std::ostream& operator<<(std::ostream& s, const ByteArray& ba)
{
    ppl7::String hex = ba.toHex();
    return s.write((const char*)hex, hex.size());
}

} // namespace ppl7
