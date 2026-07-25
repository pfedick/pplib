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

#ifndef PPL7_CORE_COMPRESSION_H_
#define PPL7_CORE_COMPRESSION_H_

#include <ppl7/types/bytearray.h>
#include <ppl7/types/bytearrayptr.h>

namespace ppl7
{
class Compression
{
public:
    enum Algorithm
    {
        Algo_NONE = 0,
        Algo_ZLIB,
        Algo_BZIP2,
        Unknown = 256
    };

    enum Level
    {
        Level_Fast = 0,
        Level_Normal,
        Level_Default,
        Level_High
    };

    enum Prefix
    {
        Prefix_None = 0,
        Prefix_V1,
        Prefix_V2,
    };

private:
    void* buffer;
    void* uncbuffer;
    Algorithm aaa;
    Level lll;
    Prefix prefix;

    void doNone(void* dst, size_t* dstlen, const void* src, size_t size);
    void doZlib(void* dst, size_t* dstlen, const void* src, size_t size);
    void doBzip2(void* dst, size_t* dstlen, const void* src, size_t size);

    void unNone(void* dst, size_t* dstlen, const void* src, size_t srclen);
    void unZlib(void* dst, size_t* dstlen, const void* src, size_t srclen);
    void unBzip2(void* dst, size_t* dstlen, const void* src, size_t srclen);

public:
    Compression();
    Compression(Algorithm method, Level level = Level_Default);
    ~Compression();
    void init(Algorithm method, Level level = Level_Default);
    void usePrefix(Prefix prefix);

    void compress(void* dst, size_t* dstlen, const void* src, size_t size, Algorithm a = Unknown);
    void compress(ByteArray& out, const void* ptr, size_t size);
    void compress(ByteArray& out, const ByteArrayPtr& in);
    ByteArrayPtr compress(const void* ptr, size_t size);
    ByteArrayPtr compress(const ByteArrayPtr& in);

    void uncompress(void* dst, size_t* dstlen, const void* src, size_t srclen, Algorithm a = Unknown);
    void uncompress(ByteArray& out, const ByteArrayPtr& data);
    void uncompress(ByteArray& out, const void* data, size_t size = 0);
    ByteArrayPtr uncompress(const void* ptr, size_t size);
    ByteArrayPtr uncompress(const ByteArrayPtr& in);
};

void Compress(ByteArray& out, const ByteArrayPtr& in, Compression::Algorithm method, Compression::Level level = Compression::Level_Default);
void CompressZlib(ByteArray& out, const ByteArrayPtr& in, Compression::Level level = Compression::Level_Default);
void CompressBZip2(ByteArray& out, const ByteArrayPtr& in, Compression::Level level = Compression::Level_Default);
void Uncompress(ByteArray& out, const ByteArrayPtr& in);

} // namespace ppl7

#endif // PPL7_CORE_COMPRESSION_H_
