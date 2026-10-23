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
#include <pplib/core/compression.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#ifdef HAVE_BZIP2
#include <bzlib.h>
#endif

#ifdef HAVE_ZSTD
#include <zstd.h>
#endif

#ifdef HAVE_BROTLI
#include <brotli/encode.h>
#include <brotli/decode.h>
#endif

namespace pplib
{

static size_t maxCompressedBound(size_t size)
{
    size_t bound = size + (size / 100) + 600;
    // LCOV_EXCL_START
    if (bound < size) throw OutOfMemoryException(); // Überlauf: Puffer passt niemals
    // LCOV_EXCL_STOP
#ifdef HAVE_ZLIB
    size_t zbound = (size_t)::compressBound((uLong)size);
    if (zbound > bound) bound = zbound;
#endif
#ifdef HAVE_ZSTD
    size_t zdbound = (size_t)::ZSTD_compressBound((size_t)size);
    if (zdbound > bound) bound = zdbound;
#endif
#ifdef HAVE_BROTLI
    // Brotli kann im Worst Case deutlich expandieren; liefert die Funktion 0, ist die
    // Größe nicht darstellbar und der generische Fallback bleibt maßgeblich.
    size_t bbound = (size_t)::BrotliEncoderMaxCompressedSize(size);
    if (bbound > bound) bound = bbound;
#endif
    return bound;
}

Compression::Compression(Algorithm method, Level level)
{
    init(method, level);
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
#ifndef HAVE_ZLIB
    if (method == Algorithm::ZLIB) {
        throw UnsupportedFeatureException("Zlib");
    }
#endif
#ifndef HAVE_BZIP2
    if (method == Algorithm::BZIP2) {
        throw UnsupportedFeatureException("Bzip2");
    }
#endif
#ifndef HAVE_ZSTD
    if (method == Algorithm::ZSTD) {
        throw UnsupportedFeatureException("Zstd");
    }
#endif
#ifndef HAVE_BROTLI
    if (method == Algorithm::BROTLI) {
        throw UnsupportedFeatureException("Brotli");
    }
#endif
    switch (method) {
    case Algorithm::NONE:
    case Algorithm::ZLIB:
    case Algorithm::BZIP2:
    case Algorithm::ZSTD:
    case Algorithm::BROTLI:
        break;
    default:
        throw IllegalArgumentException("Unsupported compression algorithm");
    }
    algorithm_ = method;

    switch (level) {
    case Level::Fast:
    case Level::Normal:
    case Level::High:
    case Level::Default:
        break;
    default:
        throw IllegalArgumentException("Unsupported compression level");
    }
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
#ifndef HAVE_ZLIB
    throw UnsupportedFeatureException("Zlib");
#else
    const void* safe_src = (src != nullptr) ? src : "";
    uLongf dstlen_zlib = (uLongf)*dstlen;
    int zcomplevel;
    switch (level_) {
    case Level::Fast:
        zcomplevel = Z_BEST_SPEED;
        break;
    case Level::Normal:
        zcomplevel = 5;
        break;
    case Level::High:
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
    case Level::Fast:
        zcomplevel = 1;
        break;
    case Level::Normal:
        zcomplevel = 5;
        break;
    case Level::High:
        zcomplevel = 9;
        break;
    default:
        zcomplevel = 5;
        break;
    }
    unsigned int bz_dstlen = (unsigned int)*dstlen;
    int ret = BZ2_bzBuffToBuffCompress((char*)dst, &bz_dstlen, (char*)safe_src, (unsigned int)size, zcomplevel, 0, 30);
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

void Compression::doZstd(void* dst, size_t* dstlen, const void* src, size_t size)
{
#ifndef HAVE_ZSTD
    throw UnsupportedFeatureException("Zstd");
#else
    const void* safe_src = (src != nullptr) ? src : "";
    int zcomplevel;
    switch (level_) {
    case Level::Fast:
        zcomplevel = 1;
        break;
    case Level::Normal:
        zcomplevel = 5;
        break;
    case Level::High:
        zcomplevel = 19;
        break;
    default:
        zcomplevel = ZSTD_CLEVEL_DEFAULT;
        break;
    }
    size_t res = ::ZSTD_compress(dst, *dstlen, safe_src, size, zcomplevel);
    if (!ZSTD_isError(res)) {
        *dstlen = res;
        return;
    }
    int code = ZSTD_getErrorCode(res);
    if (code == ZSTD_error_dstSize_tooSmall) {
        throw BufferTooSmallException();
        // LCOV_EXCL_START
    } else if (code == ZSTD_error_memory_allocation) {
        throw OutOfMemoryException();
    }
    throw CompressionFailedException();
    // LCOV_EXCL_STOP
#endif
}

void Compression::doBrotli(void* dst, size_t* dstlen, const void* src, size_t size)
{
#ifndef HAVE_BROTLI
    throw UnsupportedFeatureException("Brotli");
#else
    const void* safe_src = (src != nullptr) ? src : "";
    int quality;
    switch (level_) {
    case Level::Fast:
        quality = 1;
        break;
    case Level::Normal:
        quality = 5;
        break;
    case Level::High:
        quality = BROTLI_MAX_QUALITY;
        break;
    default:
        quality = 6;
        break;
    }
    // Brotli unterscheidet beim One-Shot-Compress nicht zwischen "Puffer zu klein" und
    // echten Fehlern. Ist der Zielbuffer kleiner als von Brotli selbst kalkuliert, ist
    // die Ursache eindeutig.
    size_t brotli_bound = ::BrotliEncoderMaxCompressedSize(size);
    if (brotli_bound > 0 && *dstlen < brotli_bound) throw BufferTooSmallException();

    size_t encoded = *dstlen;
    if (::BrotliEncoderCompress(quality, BROTLI_DEFAULT_WINDOW, BROTLI_MODE_GENERIC, size, (const uint8_t*)safe_src, &encoded,
                                (uint8_t*)dst)) {
        *dstlen = encoded;
        return;
    }
    // LCOV_EXCL_START
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
#ifndef HAVE_ZLIB
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
    int ret = BZ2_bzBuffToBuffDecompress((char*)dst, &bz_dstlen, (char*)safe_src, (unsigned int)srclen, 0, 0);
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

void Compression::unZstd(void* dst, size_t* dstlen, const void* src, size_t srclen)
{
#ifndef HAVE_ZSTD
    throw UnsupportedFeatureException("Zstd");
#else
    const void* safe_src = (src != nullptr) ? src : "";
    size_t res = ::ZSTD_decompress(dst, *dstlen, safe_src, srclen);
    if (!ZSTD_isError(res)) {
        *dstlen = res;
        return;
    }
    switch (ZSTD_getErrorCode(res)) {
    case ZSTD_error_dstSize_tooSmall:
        throw BufferTooSmallException();
    case ZSTD_error_prefix_unknown:
    case ZSTD_error_corruption_detected:
    case ZSTD_error_checksum_wrong:
    case ZSTD_error_srcSize_wrong:
    case ZSTD_error_literals_headerWrong:
        throw CorruptedDataException("Zstd-Datenstrom beschaedigt");
        // LCOV_EXCL_START
    case ZSTD_error_memory_allocation:
        throw OutOfMemoryException();
    default:
        throw DecompressionFailedException();
        // LCOV_EXCL_STOP
    }
#endif
}

void Compression::unBrotli(void* dst, size_t* dstlen, const void* src, size_t srclen)
{
#ifndef HAVE_BROTLI
    throw UnsupportedFeatureException("Brotli");
#else
    const uint8_t* safe_src = (src != nullptr) ? (const uint8_t*)src : (const uint8_t*)"";
    size_t decoded = *dstlen;
    BrotliDecoderResult res = ::BrotliDecoderDecompress(srclen, safe_src, &decoded, (uint8_t*)dst);
    if (res == BROTLI_DECODER_RESULT_SUCCESS) {
        *dstlen = decoded;
        return;
    }
    // Der One-Shot-Decoder meldet "Puffer zu klein" und Datenfehler gleich. Er schreibt
    // vorher so viel wie moeglich in den Zielbuffer: ist er komplett gefuellt, war er zu klein.
    if (decoded == *dstlen) throw BufferTooSmallException();
    throw CorruptedDataException("Brotli-Datenstrom beschaedigt");
#endif
}

void Compression::compress(void* dst, size_t* dstlen, const void* src, size_t size, Algorithm a)
{
    if (!dst || !dstlen) throw IllegalArgumentException();
    if (!src && size > 0) throw IllegalArgumentException();
    if (size > MAX_DATA_SIZE || *dstlen > MAX_DATA_SIZE) {
        throw IllegalArgumentException("Datenmenge überschreitet Compression::MAX_DATA_SIZE");
    }
    if (a == Algorithm::Unknown) a = algorithm_;
    switch (a) {
    case Algorithm::NONE:
        doNone(dst, dstlen, src, size);
        return;
    case Algorithm::ZLIB:
        doZlib(dst, dstlen, src, size);
        return;
    case Algorithm::BZIP2:
        doBzip2(dst, dstlen, src, size);
        return;
    case Algorithm::ZSTD:
        doZstd(dst, dstlen, src, size);
        return;
    case Algorithm::BROTLI:
        doBrotli(dst, dstlen, src, size);
        return;
    default:
        throw IllegalArgumentException("Unsupported compression algorithm");
    }
}

ByteArray Compression::compress(const void* ptr, size_t size)
{
    return compress(ByteArrayPtr(ptr, size));
}

void Compression::compressTo(ByteArray& out, const void* ptr, size_t size)
{
    if (size > MAX_DATA_SIZE) throw IllegalArgumentException("Datenmenge überschreitet Compression::MAX_DATA_SIZE");

    if (prefix_ == Prefix::NONE) {
        size_t maxbound = maxCompressedBound(size);
        out.malloc(maxbound);
        size_t dstlen = maxbound;
        compress(out.ptr(), &dstlen, ptr, size);
        out.truncate(dstlen);
        return;
    }

    size_t maxbound = maxCompressedBound(size);

    if (prefix_ == Prefix::V1) {
        // Komprimierung erfolgt direkt in den Ausgabepuffer: kein Zwischenpuffer, kein memcpy
        char* prefix = (char*)out.malloc(maxbound + 9);
        Poke8(prefix, static_cast<uint8_t>(static_cast<unsigned>(algorithm_) & 7));
        size_t dstlen = maxbound;
        compress(prefix + 9, &dstlen, ptr, size);
        if (dstlen > 0xFFFFFFFFULL) throw CompressionFailedException("komprimierte Datenmenge überschreitet V1-Längenfeld");
        Poke32(prefix + 1, (uint32_t)size);
        Poke32(prefix + 5, (uint32_t)dstlen);
        out.truncate(dstlen + 9);
        return;
    }

    if (prefix_ == Prefix::V2) {
        // Der V2-Header ist maximal 9 Byte lang; die Daten stehen zunächst hinter dem Maximal-Header
        char* prefix = (char*)out.malloc(maxbound + 9);
        size_t dstlen = maxbound;
        compress(prefix + 9, &dstlen, ptr, size);
        if (dstlen > 0xFFFFFFFFULL) throw CompressionFailedException("komprimierte Datenmenge überschreitet V1-Längenfeld");

        int b_unc = 4;
        int b_comp = 4;
        int flag = (static_cast<unsigned>(algorithm_) & 7) | 8; // Version 2 bit

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

        if (bytes < 9) memmove(prefix + bytes, prefix + 9, dstlen);
        out.truncate(dstlen + bytes);
        return;
    }
    // LCOV_EXCL_START
    throw UnknownException();
    // LCOV_EXCL_STOP
}

ByteArray Compression::compress(const ByteArrayPtr& in)
{
    ByteArray out;
    compressTo(out, in.ptr(), in.size());
    return out;
}

void Compression::compress(ByteArray& out, const ByteArrayPtr& in)
{
    compressTo(out, in.ptr(), in.size());
}

void Compression::compress(ByteArray& out, const void* ptr, size_t size)
{
    compressTo(out, ptr, size);
}

void Compression::uncompress(void* dst, size_t* dstlen, const void* src, size_t srclen, Algorithm a)
{
    if (!dst || !dstlen) throw IllegalArgumentException();
    if (!src && srclen > 0) throw IllegalArgumentException();
    if (srclen > MAX_DATA_SIZE || *dstlen > MAX_DATA_SIZE) {
        throw IllegalArgumentException("Datenmenge überschreitet Compression::MAX_DATA_SIZE");
    }
    if (a == Algorithm::Unknown) a = algorithm_;
    switch (a) {
    case Algorithm::NONE:
        unNone(dst, dstlen, src, srclen);
        return;
    case Algorithm::ZLIB:
        unZlib(dst, dstlen, src, srclen);
        return;
    case Algorithm::BZIP2:
        unBzip2(dst, dstlen, src, srclen);
        return;
    case Algorithm::ZSTD:
        unZstd(dst, dstlen, src, srclen);
        return;
    case Algorithm::BROTLI:
        unBrotli(dst, dstlen, src, srclen);
        return;
    default:
        throw IllegalArgumentException("Unsupported compression algorithm");
    }
}

ByteArray Compression::uncompress(const void* ptr, size_t size)
{
    return uncompress(ByteArrayPtr(ptr, size));
}

void Compression::uncompressTo(ByteArray& out, const void* ptr, size_t size)
{
    if (size > MAX_DATA_SIZE) throw IllegalArgumentException("Datenmenge überschreitet Compression::MAX_DATA_SIZE");

    if (prefix_ == Prefix::NONE) {
        size_t bsize = (size > 0 ? size * 4 : 64);
        size_t buffersize = bsize;
        while (true) {
            out.malloc(buffersize);
            size_t dstlen = buffersize;
            try {
                uncompress(out.ptr(), &dstlen, ptr, size);
                out.truncate(dstlen);
                return;
            }
            catch (const BufferTooSmallException&) {
                buffersize += bsize;
            }
        }
    } else if (prefix_ == Prefix::V1) {
        if (size < 9) throw CorruptedDataException("data too small for V1 prefix");
        const char* buffer = (const char*)ptr;
        int flag = Peek8(buffer);
        if (flag & 0xF8) throw CorruptedDataException("reserved bits in V1 prefix set");
        unsigned algo = static_cast<unsigned>(flag & 7);
        if (algo > static_cast<unsigned>(Algorithm::BROTLI)) throw CorruptedDataException("unknown algorithm in V1 prefix");
        size_t size_unc = Peek32(buffer + 1);
        size_t size_comp = Peek32(buffer + 5);
        if (size_comp > size - 9) throw CorruptedDataException("truncated V1 data");
        out.malloc(size_unc);
        size_t dstlen = size_unc;
        uncompress(out.ptr(), &dstlen, buffer + 9, size_comp, static_cast<Algorithm>(algo));
        out.truncate(dstlen);
        return;
    } else if (prefix_ == Prefix::V2) {
        if (size < 1) throw CorruptedDataException("empty V2 data");
        const char* buffer = (const char*)ptr;
        int flag = Peek8(buffer);
        if ((flag & 8) == 0) {
            throw CorruptedDataException("wrong flag: bit 3 not set");
        }
        unsigned algo = static_cast<unsigned>(flag & 7);
        if (algo > static_cast<unsigned>(Algorithm::BROTLI)) throw CorruptedDataException("unknown algorithm in V2 prefix");
        Algorithm a = static_cast<Algorithm>(algo);
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

        if (size_comp > size - bytes) {
            throw CorruptedDataException("truncated V2 data");
        }

        out.malloc(size_unc);
        size_t dstlen = size_unc;
        uncompress(out.ptr(), &dstlen, buffer + bytes, size_comp, a);
        out.truncate(dstlen);
        return;
    }
    // LCOV_EXCL_START
    throw DecompressionFailedException();
    // LCOV_EXCL_STOP
}

ByteArray Compression::uncompress(const ByteArrayPtr& in)
{
    ByteArray out;
    uncompressTo(out, in.ptr(), in.size());
    return out;
}

void Compression::uncompress(ByteArray& out, const ByteArrayPtr& in)
{
    uncompressTo(out, in.ptr(), in.size());
}

void Compression::uncompress(ByteArray& out, const void* ptr, size_t size)
{
    uncompressTo(out, ptr, size);
}

ByteArray Compress(const ByteArrayPtr& in, Compression::Algorithm method, Compression::Level level)
{
    Compression comp(method, level);
    comp.setPrefix(Compression::Prefix::V2);
    return comp.compress(in);
}

void Compress(ByteArray& out, const ByteArrayPtr& in, Compression::Algorithm method, Compression::Level level)
{
    out = Compress(in, method, level);
}

ByteArray Uncompress(const ByteArrayPtr& in)
{
    Compression comp;
    comp.setPrefix(Compression::Prefix::V2);
    return comp.uncompress(in);
}

void Uncompress(ByteArray& out, const ByteArrayPtr& in)
{
    out = Uncompress(in);
}

ByteArray CompressZlib(const ByteArrayPtr& in, Compression::Level level)
{
    return Compress(in, Compression::Algorithm::ZLIB, level);
}

void CompressZlib(ByteArray& out, const ByteArrayPtr& in, Compression::Level level)
{
    out = CompressZlib(in, level);
}

ByteArray CompressBZip2(const ByteArrayPtr& in, Compression::Level level)
{
    return Compress(in, Compression::Algorithm::BZIP2, level);
}

void CompressBZip2(ByteArray& out, const ByteArrayPtr& in, Compression::Level level)
{
    out = CompressBZip2(in, level);
}

} // namespace pplib
