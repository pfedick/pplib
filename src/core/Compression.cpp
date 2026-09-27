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
#include <pplib/core/compression.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#ifdef HAVE_BZIP2
#include <bzlib.h>
#endif

namespace pplib
{

Compression::Compression()
{
    buffer = NULL;
    uncbuffer = NULL;
    aaa = Algo_ZLIB;
    lll = Level_Default;
    prefix = Prefix_None;
}

Compression::Compression(Algorithm method, Level level)
{
    buffer = NULL;
    uncbuffer = NULL;
    aaa = method;
    lll = level;
    prefix = Prefix_None;
}

Compression::~Compression()
{
    if (buffer) free(buffer);
    if (uncbuffer) free(uncbuffer);
}

void Compression::usePrefix(Prefix prefix)
{
    this->prefix = prefix;
}

void Compression::init(Algorithm method, Level level)
{
#ifndef HAVE_LIBZ
    if (method == Algo_ZLIB) {
        throw UnsupportedFeatureException("Zlib");
    }
#endif
#ifndef HAVE_BZIP2
    if (method == Algo_BZIP2) {
        throw UnsupportedFeatureException("Bzip2");
    }
#endif

    aaa = method;
    lll = level;
}

void Compression::doNone(void* dst, size_t* dstlen, const void* src, size_t size)
{
    if (*dstlen < size) {
        *dstlen = size;
        throw BufferTooSmallException();
    }
    memcpy(dst, src, size);
    *dstlen = size;
}

void Compression::doZlib(void* dst, size_t* dstlen, const void* src, size_t size)
{
#ifndef HAVE_LIBZ
    throw UnsupportedFeatureException("Zlib");
#else
    uLongf dstlen_zlib;
    int zcomplevel;
    switch (lll) { // Kompressionslevel festlegen
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
    dstlen_zlib = (uLongf)*dstlen;
    int res = ::compress2((Bytef*)dst, (uLongf*)&dstlen_zlib, (const Bytef*)src, (uLong)size, zcomplevel);
    if (res == Z_OK) {
        *dstlen = (uint32_t)dstlen_zlib;
        return;
    } else if (res == Z_MEM_ERROR) {
        throw OutOfMemoryException();
    } else if (res == Z_BUF_ERROR) {
        *dstlen = (uint32_t)dstlen_zlib;
        throw BufferTooSmallException();
    } else if (res == Z_STREAM_ERROR)
        throw CompressionFailedException();
    throw CompressionFailedException();
#endif
}

void Compression::doBzip2(void* dst, size_t* dstlen, const void* src, size_t size)
{
#ifndef HAVE_BZIP2
    throw UnsupportedFeatureException("Bzip2");
#else
    int zcomplevel;
    switch (lll) {
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
    int ret = BZ2_bzBuffToBuffCompress((char*)dst, (unsigned int*)dstlen, (char*)src, (int)size, zcomplevel, 0, 30);
    if (ret == BZ_OK) {
        return;
    } else if (ret == BZ_MEM_ERROR) {
        throw OutOfMemoryException();
    } else if (ret == BZ_OUTBUFF_FULL) {
        throw BufferTooSmallException();
    }
    throw CompressionFailedException();
#endif
}

void Compression::unNone(void* dst, size_t* dstlen, const void* src, size_t srclen)
{
    if (*dstlen < srclen) {
        *dstlen = srclen;
        throw BufferTooSmallException();
    }
    memcpy(dst, src, srclen);
    *dstlen = srclen;
}

void Compression::unZlib(void* dst, size_t* dstlen, const void* src, size_t srclen)
{
#ifndef HAVE_LIBZ
    throw UnsupportedFeatureException("Zlib");
#else
    size_t d;
    uLongf dstlen_zlib;
    d = *dstlen;
    dstlen_zlib = (uLongf)d;
    int ret = ::uncompress((Bytef*)dst, &dstlen_zlib, (const Bytef*)src, (uLong)srclen);
    if (ret == Z_OK) {
        *dstlen = (uint32_t)dstlen_zlib;
        return;
    } else if (ret == Z_MEM_ERROR) {
        throw OutOfMemoryException();
    } else if (ret == Z_BUF_ERROR) {
        *dstlen = (uint32_t)dstlen_zlib;
        throw BufferTooSmallException();
    } else if (ret == Z_DATA_ERROR) {
        throw CorruptedDataException("Z_DATA_ERROR");
    }
    throw DecompressionFailedException();
#endif
}

void Compression::unBzip2(void* dst, size_t* dstlen, const void* src, size_t srclen)
{
#ifndef HAVE_BZIP2
    throw UnsupportedFeatureException("Bzip2");
#else
    int ret = BZ2_bzBuffToBuffDecompress((char*)dst, (unsigned int*)dstlen, (char*)src, (int)srclen, 0, 0);
    if (ret == BZ_OK) {
        return;
    } else if (ret == BZ_MEM_ERROR) {
        throw OutOfMemoryException();
    } else if (ret == BZ_OUTBUFF_FULL) {
        throw BufferTooSmallException();
    } else if (ret == BZ_DATA_ERROR || ret == BZ_DATA_ERROR_MAGIC || ret == BZ_UNEXPECTED_EOF) {
        throw CorruptedDataException();
    }
    throw DecompressionFailedException();
#endif
}

void Compression::compress(void* dst, size_t* dstlen, const void* src, size_t srclen, Algorithm a)
{
    if ((!src) || (!dst)) throw NullPointerException();
    if (dstlen == NULL) throw NullPointerException();
    if (a == Unknown) a = aaa;
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

ByteArrayPtr Compression::compress(const void* ptr, size_t size)
{
    if (buffer) free(buffer);
    size_t dstlen = size + 64;
    buffer = malloc(dstlen + 9);
    if (!buffer) throw OutOfMemoryException();
    char* tgt = (char*)buffer + 9;
    compress(tgt, &dstlen, ptr, size);
    if (prefix == Prefix_None) {
        return ByteArrayPtr(tgt, dstlen);
    } else if (prefix == Prefix_V1) {
        char* prefix = (char*)buffer;
        Poke8(prefix, (aaa & 7));        // Nur die unteren 3 Bits sind gültig, Rest 0
        Poke32(prefix + 1, (int)size);   // Größe Unkomprimiert
        Poke32(prefix + 5, (int)dstlen); // Größe Komprimiert
        return ByteArrayPtr(prefix, dstlen + 9);
    } else if (prefix == Prefix_V2) {
        // Zuerst prüfen wir, wieviel Bytes wir für die jeweiligen Blöcke brauchen
        int b_unc = 4, b_comp = 4;
        int flag = aaa & 7; // Nur die unteren 3 Bits sind gültig, Rest 0
        flag |= 8;          // Version 2-Bit setzen

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
        char* prefix = tgt - bytes;
        char* p2 = prefix + 1;

        // Daten unkomprimiert
        if (b_unc == 1) {
            Poke8(prefix + 1, (int)size);
            p2 = prefix + 2;
        } else if (b_unc == 2) {
            Poke16(prefix + 1, (int)size);
            p2 = prefix + 3;
            flag |= 16;
        } else if (b_unc == 3) {
            Poke24(prefix + 1, (int)size);
            p2 = prefix + 4;
            flag |= 32;
        } else {
            Poke32(prefix + 1, (int)size);
            p2 = prefix + 5;
            flag |= (16 + 32);
        }

        // Daten komprimiert
        if (b_comp == 1) {
            Poke8(p2, (int)dstlen);
        } else if (b_comp == 2) {
            Poke16(p2, (int)dstlen);
            flag |= 64;
        } else if (b_comp == 3) {
            Poke24(p2, (int)dstlen);
            flag |= 128;
        } else {
            Poke32(p2, (int)dstlen);
            flag |= (128 + 64);
        }
        Poke8(prefix, flag);
        /*
        printf ("DEBUG\n");
        printf ("b_unc=%d, b_comp=%d, bytes=%d, flag=%d\n", b_unc, b_comp, bytes, flag);
        printf ("size unc=%zd, size_comp=%zd\n", size, dstlen);
        */
        return ByteArrayPtr(prefix, dstlen + bytes);
    }
    // Bis hierhin sollte es nicht kommen
    throw UnknownException();
}

ByteArrayPtr Compression::compress(const ByteArrayPtr& in)
{
    return compress(in.ptr(), in.size());
}

void Compression::compress(ByteArray& out, const void* ptr, size_t size)
{
    ByteArrayPtr r = compress(ptr, size);
    out.copy(r);
}

void Compression::compress(ByteArray& out, const ByteArrayPtr& in)
{
    compress(out, in.adr(), in.size());
}

void Compression::uncompress(void* dst, size_t* dstlen, const void* src, size_t srclen, Algorithm a)
{
    if ((!src) || (!dst)) throw NullPointerException();
    if (dstlen == NULL) throw NullPointerException();
    if (a == Unknown) a = aaa;
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

ByteArrayPtr Compression::uncompress(const void* ptr, size_t size)
{
    if (uncbuffer) free(uncbuffer);
    uncbuffer = NULL;
    if (prefix == Prefix_None) {
        size_t bsize = size * 3;
        while (1) {
            if (uncbuffer) free(uncbuffer);
            uncbuffer = malloc(bsize);
            if (!uncbuffer) throw OutOfMemoryException();
            // Wir prüfen, ob das Ergebnis in den Buffer passt
            size_t dstlen = bsize;
            try {
                uncompress(uncbuffer, &dstlen, ptr, size);
                return ByteArrayPtr(uncbuffer, dstlen);
            }
            catch (BufferTooSmallException&) {
                // Der Buffer war nicht gross genug, wir vergrößern ihn
                bsize += size;
            }
            catch (...) {
                free(uncbuffer);
                uncbuffer = NULL;
                throw;
            }
        }
    } else if (prefix == Prefix_V1) {
        char* buffer = (char*)ptr;
        int flag = Peek8(buffer);
        size_t size_unc = Peek32(buffer + 1);
        size_t size_comp = Peek32(buffer + 5);
        // printf ("Flag: %i, unc: %u, comp: %u\n",flag,size_unc, size_comp);
        if (uncbuffer) free(uncbuffer);
        uncbuffer = malloc(size_unc);
        if (!uncbuffer) throw OutOfMemoryException();
        size_t dstlen = size_unc;
        try {
            uncompress(uncbuffer, &dstlen, buffer + 9, size_comp, (Algorithm)(flag & 7));
            return ByteArrayPtr(uncbuffer, dstlen);
        }
        catch (...) {
            free(uncbuffer);
            uncbuffer = NULL;
            throw;
        }
    } else if (prefix == Prefix_V2) {
        char* buffer = (char*)ptr;
        int flag = Peek8(buffer);
        Algorithm a = (Algorithm)(flag & 7);
        if ((flag & 8) == 0) { // Bit 3 muss aber gesetzt sein
            throw CorruptedDataException("wrong flag");
        }
        int b_unc = 4, b_comp = 4;
        if ((flag & 48) == 0)
            b_unc = 1;
        else if ((flag & 48) == 16)
            b_unc = 2;
        else if ((flag & 48) == 32)
            b_unc = 3;
        else
            b_unc = 4;
        if ((flag & 192) == 0)
            b_comp = 1;
        else if ((flag & 192) == 64)
            b_comp = 2;
        else if ((flag & 192) == 128)
            b_comp = 3;
        else
            b_comp = 4;

        size_t size_unc = 0;
        if (b_unc == 1)
            size_unc = Peek8(buffer + 1);
        else if (b_unc == 2)
            size_unc = Peek16(buffer + 1);
        else if (b_unc == 3)
            size_unc = Peek24(buffer + 1);
        else
            size_unc = Peek32(buffer + 1);

        if (uncbuffer) free(uncbuffer);
        uncbuffer = malloc(size_unc);
        if (!uncbuffer) throw OutOfMemoryException();
        size_t dstlen = size_unc;
        size_t bytes = 1 + b_unc + b_comp;
        /*
        printf ("b_unc=%d, b_comp=%d, bytes=%d, dstlen=%zd, size=%zd\n",
                b_unc, b_comp, bytes, dstlen,size);
        if (size==804) {
            HexDump(buffer,size);
        }
        */
        try {
            uncompress(uncbuffer, &dstlen, buffer + bytes, size - bytes, a);
            return ByteArrayPtr(uncbuffer, dstlen);
        }
        catch (...) {
            free(uncbuffer);
            uncbuffer = NULL;
            throw;
        }
    }
    throw DecompressionFailedException();
}

ByteArrayPtr Compression::uncompress(const ByteArrayPtr& in)
{
    return uncompress(in.ptr(), in.size());
}

void Compression::uncompress(ByteArray& out, const void* ptr, size_t size)
{
    ByteArrayPtr b = uncompress(ptr, size);
    out.copy(b);
}

void Compression::uncompress(ByteArray& out, const ByteArrayPtr& object)
{
    uncompress(out, object.ptr(), object.size());
}

void Compress(ByteArray& out, const ByteArrayPtr& in, Compression::Algorithm method, Compression::Level level)
{
    Compression comp;
    comp.init(method, level);
    comp.usePrefix(Compression::Prefix_V2);
    comp.compress(out, in);
}

/*!\ingroup PPLIB_COMPRESSION
 * \relatesalso Compression
 * \brief Daten dekomprimieren
 *
 * \descr
 * Mit dieser Funktion werden die in \p in enthaltenen komprimierten Daten
 * entpackt und das Ergebnis im CBinary-Objekt \p out gespeichert.
 * \par
 * Die Funktion geht davon aus, dass die komprimierten Daten mit einem
 * Version 2 Prefix beginnen (siehe \ref Compression_Prefix). Ist dies nicht der
 * Fall, sollte statt dieser Funktion die Klasse Compression verwendet werden,
 * deren Compression::Uncompress-Funktionen auch Dekomprimierung ohne Prefix
 * unterstützen.
 *
 * @param[out] out CBinary-Objekt, in dem die entpackten Daten gespeichert werden sollen
 * @param[in] in Das CBinary-Objekt, das die komprimierten Daten enthält
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0
 *
 * \see Compression
 */
void Uncompress(ByteArray& out, const ByteArrayPtr& in)
{
    Compression comp;
    comp.usePrefix(Compression::Prefix_V2);
    comp.uncompress(out, in);
}

void CompressZlib(ByteArray& out, const ByteArrayPtr& in, Compression::Level level)
/*!\ingroup PPLIB_COMPRESSION
 * \relatesalso Compression
 * \brief Daten mit ZLib komprimieren
 *
 * \descr
 * Mit dieser Funktion wird der durch \p in referenzierte Speicherbereich
 * mit der Komprimierungsmethode ZLib und dem Komprimierungslevel \p level komprimiert
 * und das Ergebnis im CMemory-Objekt \p out gespeichert.
 * \par
 * Die Funktion stellt den komprimierten Daten automatisch einen Version 2 Prefix voran (siehe
 * \ref Compression_Prefix), so dass die komprimierten Daten durch Aufruf der Funktion
 * Uncompress ohne Angabe der Kompressionsmethod wieder entpackt werden kann.
 *
 * @param[out] out ByteArray-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] in Ein ByteArrayPtr-Objekt mit den zu komprimierenden Daten.
 * @param[in] level Der gewünschte Komprimierungslevel (siehe Compression::Level). Der Default ist
 * Compression::Level_High
 *
 * \see Compression
 */
{
    Compress(out, in, Compression::Algo_ZLIB, level);
}

void CompressBZip2(ByteArray& out, const ByteArrayPtr& in, Compression::Level level)
/*!\ingroup PPLIB_COMPRESSION
 * \relatesalso Compression
 * \brief Daten mit BZip2 komprimieren
 *
 * \descr
 * Mit dieser Funktion wird der durch \p in referenzierte Speicherbereich
 * mit der Komprimierungsmethode BZip2 und dem Komprimierungslevel \p level komprimiert
 * und das Ergebnis im CMemory-Objekt \p out gespeichert.
 * \par
 * Die Funktion stellt den komprimierten Daten automatisch einen Version 2 Prefix voran (siehe
 * \ref Compression_Prefix), so dass die komprimierten Daten durch Aufruf der Funktion
 * Uncompress ohne Angabe der Kompressionsmethod wieder entpackt werden kann.
 *
 * @param[out] out CMemory-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] in Ein CMemoryReference-Objekt mit den zu komprimierenden Daten.
 * @param[in] level Der gewünschte Komprimierungslevel (siehe Compression::Level). Der Default ist
 * Compression::Level_High
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * komprimierten Daten kann \p out entnommen werden.
 *
 * \see Compression
 */
{
    Compress(out, in, Compression::Algo_BZIP2, level);
}

} // namespace pplib
