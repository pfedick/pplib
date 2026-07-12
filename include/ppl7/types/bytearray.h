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

#ifndef PPL7_TYPES_BYTEARRAY_H_
#define PPL7_TYPES_BYTEARRAY_H_

#include <iostream>
#include <ppl7/types/bytearrayptr.h>

namespace ppl7
{
class String;
class WideString;

class ByteArray : public ByteArrayPtr
{
private:
public:
    ByteArray();
    ByteArray(const ByteArrayPtr& other);
    ByteArray(const ByteArray& other);
    ByteArray(ByteArray&& other);
    ByteArray(const String& str);
    ByteArray(const WideString& str);
    ByteArray(const void* adr, size_t size);
    ByteArray(size_t size);
    ~ByteArray();
    void useadr(void* adr, size_t size);
    void* copy(const void* adr, size_t size);
    void* copy(const ByteArrayPtr& other);
    void* append(void* adr, size_t size);
    void* append(const ByteArrayPtr& other);
    void* prepend(void* adr, size_t size);
    void* prepend(const ByteArrayPtr& other);
    void* fromHex(const String& hex);
    void* malloc(size_t size);
    void* calloc(size_t size);
    void free();
    void clear();
    ByteArray& operator=(const ByteArrayPtr& other);
    ByteArray& operator=(const ByteArray& other);
    ByteArray& operator=(ByteArray&& other);
    ByteArray& operator=(const String& str);
    ByteArray& operator=(const WideString& str);
    operator const void*() const;
    operator const unsigned char*() const;
    operator const char*() const;
};
std::ostream& operator<<(std::ostream& s, const ByteArray& ba);

} // namespace ppl7

#endif // PPL7_TYPES_BYTEARRAY_H_
