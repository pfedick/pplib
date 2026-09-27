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
#include <algorithm>
#include <vector>
#include <pplib/core/compression.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

#if defined(HAVE_ZLIB) || defined(HAVE_LIBZ)
#include <zlib.h>
#endif

#ifdef HAVE_BZIP2
#include <bzlib.h>
#endif

namespace pplib
{

static size_t maxCompressedBound(size_t size)
{
    size_t bound = size + (size / 100) + 600;
#if defined(HAVE_ZLIB) || defined(HAVE_LIBZ)
    size_t zbound = (size_t)::compressBound((uLong)size);
    if (zbound > bound) bound = zbound;
#endif
    return bound;
}

Compression::Compression(Algorithm method, Level level)
    : algorithm_(method),
      level_(level),
      prefix_(Prefix_None)
{
}

void Compression::usePrefix(Prefix prefix)
{
    prefix_ = prefix;
}

void Compression::setPrefix(Prefix prefix)
{
    prefix_ = prefix;
}

Compression::Prefix Compression::prefix() const noexcept
{
    return prefix_;
}

Compression::Algorithm Compression::algorithm() const noexcept
{
    return algorithm_;
}

Compression::Level Compression::level() const noexcept
{
    return level_;
}

void Compression::init(Algorithm method, Level level)
{
#if !defined(HAVE_ZLIB) && !defined(HAVE_LIBZ)
    if (method == Algo_ZLIB) {
        throw UnsupportedFeatureException("Zlib");
    }
#endif
#ifndef HAVE_BZIP2
    if (method == Algo_BZIP2) {
        throw UnsupportedFeatureException("Bzip2");
    }
#endif

    algorithm_ = method;
    level_ = level;
}

void Compression::doNone(void* dst, size_t* dstlen, const void* src, size_t size)
{
    if (*dstlen < size) {
        *dstlen = size;
        throw BufferTooSmallException();
    }
    if (size > 0 && src) {
        memcpy(dst, src, size);
    }
    *dstlen = size;
}

void Compression::doZlib(void* dst, size_t* dstlen, const void* src, size_t size)
{
#if !defined(HAVE_ZLIB) && !defined(HAVE_LIBZ)
    throw UnsupportedFeatureException("Zlib");
#else
    const void* safe_src = (src != nullptr) ? src : "";
    uLongf dstlen_zlib = (uLongf)*dstlen;
    int zcomplevel;
    switch (level_) {
    case Level_Fast:
        zcomplevel = Z_BEST_SPEED;
        break;
    case Level_Normal:
        zcomplevel = 5;
        break;
    case Level_High:
        zcomplevel = Z_BEST_COMPRESSION;
        break;
    default:
        zcomplevel = Z_DEFAULT_COMPRESSION;
        break;
    }
    int res = ::compress2((Bytef*)dst, &dstlen_zlib, (const Bytef*)safe_src, (uLong)size, zcomplevel);
    *dstlen = (size_t)dstlen_zlib;
    if (res == Z_OK) {
        return;
    } else if (res == Z_BUF_ERROR) {
        throw BufferTooSmallException();
        // LCOV_EXCL_START
    } else if (res == Z_MEM_ERROR) {
        throw OutOfMemoryException();
    } else if (res == Z_STREAM_ERROR) {
        throw CompressionFailedException();
    }
    throw CompressionFailedException();
    // LCOV_EXCL_STOP
#endif
}

void Compression::doBzip2(void* dst, size_t* dstlen, const void* src, size_t size)
{
#ifndef HAVE_BZIP2
    throw UnsupportedFeatureException("Bzip2");
#else
    const void* safe_src = (src != nullptr) ? src : "";
    int zcomplevel;
    switch (level_) {
    case Level_Fast:
        zcomplevel = 1;
        break;
    case Level_Normal:
        zcomplevel = 5;
        break;
    case Level_High:
        zcomplevel = 9;
        break;
    default:
        zcomplevel = 5;
        break;
    }
    unsigned int bz_dstlen = (unsigned int)*dstlen;
    int ret = BZ2_bzBuffToBuffCompress((char*)dst, &bz_dstlen, (char*)safe_src, (int)size, zcomplevel, 0, 30);
    *dstlen = (size_t)bz_dstlen;
    if (ret == BZ_OK) {
        return;
    } else if (ret == BZ_OUTBUFF_FULL) {
        throw BufferTooSmallException();
        // LCOV_EXCL_START
    } else if (ret == BZ_MEM_ERROR) {
        throw OutOfMemoryException();
    }
    throw CompressionFailedException();
    // LCOV_EXCL_STOP
#endif
}

void Compression::unNone(void* dst, size_t* dstlen, const void* src, size_t srclen)
{
    if (*dstlen < srclen) {
        *dstlen = srclen;
        throw BufferTooSmallException();
    }
    if (srclen > 0 && src) {
        memcpy(dst, src, srclen);
    }
    *dstlen = srclen;
}

void Compression::unZlib(void* dst, size_t* dstlen, const void* src, size_t srclen)
{
#if !defined(HAVE_ZLIB) && !defined(HAVE_LIBZ)
    throw UnsupportedFeatureException("Zlib");
#else
    const void* safe_src = (src != nullptr) ? src : "";
    uLongf dstlen_zlib = (uLongf)*dstlen;
    int ret = ::uncompress((Bytef*)dst, &dstlen_zlib, (const Bytef*)safe_src, (uLong)srclen);
    *dstlen = (size_t)dstlen_zlib;
    if (ret == Z_OK) {
        return;
    } else if (ret == Z_BUF_ERROR) {
        throw BufferTooSmallException();
    } else if (ret == Z_DATA_ERROR) {
        throw CorruptedDataException("Z_DATA_ERROR");
        // LCOV_EXCL_START
    } else if (ret == Z_MEM_ERROR) {
        throw OutOfMemoryException();
    }
    throw DecompressionFailedException();
    // LCOV_EXCL_STOP
#endif
}

void Compression::unBzip2(void* dst, size_t* dstlen, const void* src, size_t srclen)
{
#ifndef HAVE_BZIP2
    throw UnsupportedFeatureException("Bzip2");
#else
    const void* safe_src = (src != nullptr) ? src : "";
    unsigned int bz_dstlen = (unsigned int)*dstlen;
    int ret = BZ2_bzBuffToBuffDecompress((char*)dst, &bz_dstlen, (char*)safe_src, (int)srclen, 0, 0);
    *dstlen = (size_t)bz_dstlen;
    if (ret == BZ_OK) {
        return;
    } else if (ret == BZ_OUTBUFF_FULL) {
        throw BufferTooSmallException();
    } else if (ret == BZ_DATA_ERROR || ret == BZ_DATA_ERROR_MAGIC || ret == BZ_UNEXPECTED_EOF) {
        throw CorruptedDataException();
        // LCOV_EXCL_START
    } else if (ret == BZ_MEM_ERROR) {
        throw OutOfMemoryException();
    }
    throw DecompressionFailedException();
    // LCOV_EXCL_STOP
#endif
}

void Compression::compress(void* dst, size_t* dstlen, const void* src, size_t srclen, Algorithm a)
{
    if (!dst || !dstlen) throw IllegalArgumentException();
    if (!src && srclen > 0) throw IllegalArgumentException();
    if (a == Unknown) a = algorithm_;
    switch (a) {
    case Algo_NONE:
        doNone(dst, dstlen, src, srclen);
        return;
    case Algo_ZLIB:
        doZlib(dst, dstlen, src, srclen);
        return;
    case Algo_BZIP2:
        doBzip2(dst, dstlen, src, srclen);
        return;
    default:
        throw UnsupportedFeatureException();
    }
}

ByteArray Compression::compress(const void* ptr, size_t size)
{
    return compress(ByteArrayPtr(ptr, size));
}

ByteArray Compression::compress(const ByteArrayPtr& in)
{
    const void* ptr = in.ptr();
    size_t size = in.size();

    if (prefix_ == Prefix_None) {
        size_t maxbound = maxCompressedBound(size);
        ByteArray out;
        out.malloc(maxbound);
        size_t dstlen = maxbound;
        compress(out.ptr(), &dstlen, ptr, size);
        out.truncate(dstlen);
        return out;
    }

    size_t maxbound = maxCompressedBound(size);
    std::vector<uint8_t> workbuf(maxbound);
    size_t dstlen = maxbound;
    compress(workbuf.data(), &dstlen, ptr, size);

    if (prefix_ == Prefix_V1) {
        ByteArray out;
        char* prefix = (char*)out.malloc(dstlen + 9);
        Poke8(prefix, (algorithm_ & 7));
        Poke32(prefix + 1, (uint32_t)size);
        Poke32(prefix + 5, (uint32_t)dstlen);
        if (dstlen > 0) {
            memcpy(prefix + 9, workbuf.data(), dstlen);
        }
        return out;
    } else if (prefix_ == Prefix_V2) {
        int b_unc = 4, b_comp = 4;
        int flag = (algorithm_ & 7) | 8; // Version 2 bit

        if (size <= 0xff)
            b_unc = 1;
        else if (size <= 0xffff)
            b_unc = 2;
        else if (size <= 0xffffff)
            b_unc = 3;

        if (dstlen <= 0xff)
            b_comp = 1;
        else if (dstlen <= 0xffff)
            b_comp = 2;
        else if (dstlen <= 0xffffff)
            b_comp = 3;

        int bytes = 1 + b_unc + b_comp;
        flag |= ((b_unc - 1) << 4);
        flag |= ((b_comp - 1) << 6);

        ByteArray out;
        char* prefix = (char*)out.malloc(dstlen + bytes);
        Poke8(prefix, flag);

        char* p_unc = prefix + 1;
        if (b_unc == 1)
            Poke8(p_unc, (uint8_t)size);
        else if (b_unc == 2)
            Poke16(p_unc, (uint16_t)size);
        else if (b_unc == 3)
            Poke24(p_unc, (uint32_t)size);
        else
            Poke32(p_unc, (uint32_t)size);

        char* p_comp = prefix + 1 + b_unc;
        if (b_comp == 1)
            Poke8(p_comp, (uint8_t)dstlen);
        else if (b_comp == 2)
            Poke16(p_comp, (uint16_t)dstlen);
        else if (b_comp == 3)
            Poke24(p_comp, (uint32_t)dstlen);
        else
            Poke32(p_comp, (uint32_t)dstlen);

        if (dstlen > 0) {
            memcpy(prefix + bytes, workbuf.data(), dstlen);
        }
        return out;
    }
    // LCOV_EXCL_START
    throw UnknownException();
    // LCOV_EXCL_STOP
}

void Compression::compress(ByteArray& out, const ByteArrayPtr& in)
{
    out = compress(in);
}

void Compression::compress(ByteArray& out, const void* ptr, size_t size)
{
    out = compress(ByteArrayPtr(ptr, size));
}

void Compression::uncompress(void* dst, size_t* dstlen, const void* src, size_t srclen, Algorithm a)
{
    if (!dst || !dstlen) throw IllegalArgumentException();
    if (!src && srclen > 0) throw IllegalArgumentException();
    if (a == Unknown) a = algorithm_;
    switch (a) {
    case Algo_NONE:
        unNone(dst, dstlen, src, srclen);
        return;
    case Algo_ZLIB:
        unZlib(dst, dstlen, src, srclen);
        return;
    case Algo_BZIP2:
        unBzip2(dst, dstlen, src, srclen);
        return;
    default:
        throw UnsupportedFeatureException();
    }
}

ByteArray Compression::uncompress(const void* ptr, size_t size)
{
    return uncompress(ByteArrayPtr(ptr, size));
}

ByteArray Compression::uncompress(const ByteArrayPtr& in)
{
    const void* ptr = in.ptr();
    size_t size = in.size();

    if (prefix_ == Prefix_None) {
        size_t bsize = (size > 0 ? size * 4 : 64);
        while (true) {
            ByteArray out;
            out.malloc(bsize);
            size_t dstlen = bsize;
            try {
                uncompress(out.ptr(), &dstlen, ptr, size);
                out.truncate(dstlen);
                return out;
            }
            catch (const BufferTooSmallException&) {
                bsize = std::max(bsize * 2, dstlen + 1024);
            }
        }
    } else if (prefix_ == Prefix_V1) {
        if (size < 9) throw CorruptedDataException("data too small for V1 prefix");
        const char* buffer = (const char*)ptr;
        int flag = Peek8(buffer);
        size_t size_unc = Peek32(buffer + 1);
        size_t size_comp = Peek32(buffer + 5);
        if (size < 9 + size_comp) throw CorruptedDataException("truncated V1 data");
        ByteArray out;
        out.malloc(size_unc);
        size_t dstlen = size_unc;
        uncompress(out.ptr(), &dstlen, buffer + 9, size_comp, (Algorithm)(flag & 7));
        out.truncate(dstlen);
        return out;
    } else if (prefix_ == Prefix_V2) {
        if (size < 1) throw CorruptedDataException("empty V2 data");
        const char* buffer = (const char*)ptr;
        int flag = Peek8(buffer);
        Algorithm a = (Algorithm)(flag & 7);
        if ((flag & 8) == 0) {
            throw CorruptedDataException("wrong flag: bit 3 not set");
        }
        int b_unc = ((flag >> 4) & 3) + 1;
        int b_comp = ((flag >> 6) & 3) + 1;
        size_t bytes = 1 + b_unc + b_comp;
        if (size < bytes) throw CorruptedDataException("data too small for V2 header");

        size_t size_unc = 0;
        const char* p_unc = buffer + 1;
        if (b_unc == 1)
            size_unc = Peek8(p_unc);
        else if (b_unc == 2)
            size_unc = Peek16(p_unc);
        else if (b_unc == 3)
            size_unc = Peek24(p_unc);
        else
            size_unc = Peek32(p_unc);

        size_t size_comp = 0;
        const char* p_comp = buffer + 1 + b_unc;
        if (b_comp == 1)
            size_comp = Peek8(p_comp);
        else if (b_comp == 2)
            size_comp = Peek16(p_comp);
        else if (b_comp == 3)
            size_comp = Peek24(p_comp);
        else
            size_comp = Peek32(p_comp);

        if (size < bytes + size_comp) {
            throw CorruptedDataException("truncated V2 data");
        }

        ByteArray out;
        out.malloc(size_unc);
        size_t dstlen = size_unc;
        uncompress(out.ptr(), &dstlen, buffer + bytes, size_comp, a);
        out.truncate(dstlen);
        return out;
    }
    // LCOV_EXCL_START
    throw DecompressionFailedException();
    // LCOV_EXCL_STOP
}

void Compression::uncompress(ByteArray& out, const ByteArrayPtr& in)
{
    out = uncompress(in);
}

void Compression::uncompress(ByteArray& out, const void* ptr, size_t size)
{
    out = uncompress(ByteArrayPtr(ptr, size));
}

ByteArray Compress(const ByteArrayPtr& in, Compression::Algorithm method, Compression::Level level)
{
    Compression comp(method, level);
    comp.setPrefix(Compression::Prefix_V2);
    return comp.compress(in);
}

void Compress(ByteArray& out, const ByteArrayPtr& in, Compression::Algorithm method, Compression::Level level)
{
    out = Compress(in, method, level);
}

ByteArray Uncompress(const ByteArrayPtr& in)
{
    Compression comp;
    comp.setPrefix(Compression::Prefix_V2);
    return comp.uncompress(in);
}

void Uncompress(ByteArray& out, const ByteArrayPtr& in)
{
    out = Uncompress(in);
}

ByteArray CompressZlib(const ByteArrayPtr& in, Compression::Level level)
{
    return Compress(in, Compression::Algo_ZLIB, level);
}

void CompressZlib(ByteArray& out, const ByteArrayPtr& in, Compression::Level level)
{
    out = CompressZlib(in, level);
}

ByteArray CompressBZip2(const ByteArrayPtr& in, Compression::Level level)
{
    return Compress(in, Compression::Algo_BZIP2, level);
}

void CompressBZip2(ByteArray& out, const ByteArrayPtr& in, Compression::Level level)
{
    out = CompressBZip2(in, level);
}

} // namespace pplib
