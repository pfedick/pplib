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

#ifndef PPL7_TYPES_BYTEARRAYPTR_H_
#define PPL7_TYPES_BYTEARRAYPTR_H_

#include <stdint.h>
#include <iostream>
namespace ppl7
{

class String;
class WideString;

class ByteArrayPtr
{
    friend class ByteArray;

private:
    void* ptradr;
    size_t ptrsize;

public:
    ByteArrayPtr();
    ByteArrayPtr(const ByteArrayPtr& other);
    ByteArrayPtr(const String& data);
    ByteArrayPtr(const WideString& data);
    ByteArrayPtr(void* adr, size_t size);
    ByteArrayPtr(const void* adr, size_t size);
    bool isNull() const;
    bool isEmpty() const;
    size_t size() const;
    const void* adr() const;
    const void* ptr() const;
    void use(void* adr, size_t size);
    void use(const ByteArrayPtr& other);
    ByteArrayPtr& operator=(const ByteArrayPtr& other);
    operator const void*() const;
    operator const char*() const;
    operator const unsigned char*() const;
    String toHex() const;
    String toBase64() const;
    String toString() const;
    WideString toWideString() const;
    const char* toCharPtr() const;
    const char* map(size_t position, size_t size) const;
    void truncate(size_t position);
    String md5() const;
    uint32_t crc32() const;
    unsigned char operator[](size_t pos) const;
    void set(size_t pos, unsigned char value);
    unsigned char get(size_t pos) const;
    void hexDump() const;
    void hexDump(size_t bytes) const;
    void hexDump(size_t offset, size_t bytes) const;
    void memset(int value);
    int memcmp(const ByteArrayPtr& other) const;

    inline bool operator<(const ByteArrayPtr& other) const
    {
        return memcmp(other) < 0;
    }
    inline bool operator<=(const ByteArrayPtr& other) const
    {
        return memcmp(other) <= 0;
    }
    inline bool operator>(const ByteArrayPtr& other) const
    {
        return memcmp(other) > 0;
    }
    inline bool operator>=(const ByteArrayPtr& other) const
    {
        return memcmp(other) >= 0;
    }
    inline bool operator==(const ByteArrayPtr& other) const
    {
        return memcmp(other) == 0;
    }
    inline bool operator!=(const ByteArrayPtr& other) const
    {
        return memcmp(other) != 0;
    }
};
std::ostream& operator<<(std::ostream& s, const ByteArrayPtr& bap);

} // namespace ppl7

#endif // PPL7_TYPES_BYTEARRAYPTR_H_