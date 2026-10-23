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

#include <gtest/gtest.h>
#include <vector>
#include <pplib/core/compression.h>
#include <pplib/core/functions.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/string.h>
#include <pplib/exceptions.h>
#include "pplib-tests.h"

namespace
{

static pplib::ByteArray createSampleData(size_t size)
{
    pplib::ByteArray ba;
    char* p = (char*)ba.malloc(size);
    for (size_t i = 0; i < size; ++i) {
        p[i] = (char)('A' + (i % 26));
    }
    return ba;
}

static pplib::ByteArray createRandomData(size_t size)
{
    pplib::ByteArray ba;
    char* p = (char*)ba.malloc(size);
    uint32_t state = 123456789;
    for (size_t i = 0; i < size; ++i) {
        state = state * 1664525 + 1013904223;
        p[i] = (char)(state >> 24);
    }
    return ba;
}

TEST(CompressionTest, DefaultConstructorAndProperties)
{
    pplib::Compression comp;
    EXPECT_EQ(comp.algorithm(), pplib::Compression::Algorithm::ZLIB);
    EXPECT_EQ(comp.level(), pplib::Compression::Level::Default);
    EXPECT_EQ(comp.prefix(), pplib::Compression::Prefix::NONE);

    comp.init(pplib::Compression::Algorithm::BZIP2, pplib::Compression::Level::High);
    EXPECT_EQ(comp.algorithm(), pplib::Compression::Algorithm::BZIP2);
    EXPECT_EQ(comp.level(), pplib::Compression::Level::High);

    comp.setPrefix(pplib::Compression::Prefix::V2);
    EXPECT_EQ(comp.prefix(), pplib::Compression::Prefix::V2);

    comp.setPrefix(pplib::Compression::Prefix::V1);
    EXPECT_EQ(comp.prefix(), pplib::Compression::Prefix::V1);
}

TEST(CompressionTest, ParameterizedConstructor)
{
    pplib::Compression comp(pplib::Compression::Algorithm::NONE, pplib::Compression::Level::Fast);
    EXPECT_EQ(comp.algorithm(), pplib::Compression::Algorithm::NONE);
    EXPECT_EQ(comp.level(), pplib::Compression::Level::Fast);
    EXPECT_EQ(comp.prefix(), pplib::Compression::Prefix::NONE);
}

TEST(CompressionTest, CopyAndMoveSemantics)
{
    pplib::Compression comp1(pplib::Compression::Algorithm::BZIP2, pplib::Compression::Level::High);
    comp1.setPrefix(pplib::Compression::Prefix::V2);

    // Copy constructor
    pplib::Compression comp2(comp1);
    EXPECT_EQ(comp2.algorithm(), pplib::Compression::Algorithm::BZIP2);
    EXPECT_EQ(comp2.level(), pplib::Compression::Level::High);
    EXPECT_EQ(comp2.prefix(), pplib::Compression::Prefix::V2);

    // Copy assignment
    pplib::Compression comp3;
    comp3 = comp1;
    EXPECT_EQ(comp3.algorithm(), pplib::Compression::Algorithm::BZIP2);
    EXPECT_EQ(comp3.level(), pplib::Compression::Level::High);
    EXPECT_EQ(comp3.prefix(), pplib::Compression::Prefix::V2);

    // Move constructor
    pplib::Compression comp4(std::move(comp1));
    EXPECT_EQ(comp4.algorithm(), pplib::Compression::Algorithm::BZIP2);
    EXPECT_EQ(comp4.level(), pplib::Compression::Level::High);
    EXPECT_EQ(comp4.prefix(), pplib::Compression::Prefix::V2);

    // Move assignment
    pplib::Compression comp5;
    comp5 = std::move(comp2);
    EXPECT_EQ(comp5.algorithm(), pplib::Compression::Algorithm::BZIP2);
    EXPECT_EQ(comp5.level(), pplib::Compression::Level::High);
    EXPECT_EQ(comp5.prefix(), pplib::Compression::Prefix::V2);
}

TEST(CompressionTest, RoundtripNoneNoPrefix)
{
    pplib::Compression comp(pplib::Compression::Algorithm::NONE);
    comp.setPrefix(pplib::Compression::Prefix::NONE);

    pplib::ByteArray original = createSampleData(256);
    pplib::ByteArray compressed = comp.compress(original);
    EXPECT_EQ(compressed.size(), original.size());
    EXPECT_EQ(memcmp(compressed.ptr(), original.ptr(), original.size()), 0);

    pplib::ByteArray decompressed = comp.uncompress(compressed);
    EXPECT_EQ(decompressed.size(), original.size());
    EXPECT_EQ(memcmp(decompressed.ptr(), original.ptr(), original.size()), 0);
}

TEST(CompressionTest, RoundtripZlibNoPrefix)
{
    pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);
    comp.setPrefix(pplib::Compression::Prefix::NONE);

    pplib::ByteArray original = createSampleData(4096);
    pplib::ByteArray compressed = comp.compress(original);
    EXPECT_LT(compressed.size(), original.size());

    pplib::ByteArray decompressed = comp.uncompress(compressed);
    ASSERT_EQ(decompressed.size(), original.size());
    EXPECT_EQ(memcmp(decompressed.ptr(), original.ptr(), original.size()), 0);
}

TEST(CompressionTest, RoundtripZlibAllLevels)
{
    pplib::ByteArray original = createSampleData(2048);
    pplib::Compression::Level levels[] = {pplib::Compression::Level::Fast, pplib::Compression::Level::Normal,
                                          pplib::Compression::Level::High, pplib::Compression::Level::Default};

    for (auto lvl : levels) {
        pplib::Compression comp(pplib::Compression::Algorithm::ZLIB, lvl);
        pplib::ByteArray compressed = comp.compress(original);
        pplib::ByteArray decompressed = comp.uncompress(compressed);
        ASSERT_EQ(decompressed.size(), original.size());
        EXPECT_EQ(memcmp(decompressed.ptr(), original.ptr(), original.size()), 0);
    }
}

TEST(CompressionTest, RoundtripBzip2NoPrefix)
{
    pplib::Compression comp(pplib::Compression::Algorithm::BZIP2);
    comp.setPrefix(pplib::Compression::Prefix::NONE);

    pplib::ByteArray original = createSampleData(4096);
    pplib::ByteArray compressed = comp.compress(original);
    EXPECT_LT(compressed.size(), original.size());

    pplib::ByteArray decompressed = comp.uncompress(compressed);
    ASSERT_EQ(decompressed.size(), original.size());
    EXPECT_EQ(memcmp(decompressed.ptr(), original.ptr(), original.size()), 0);
}

TEST(CompressionTest, RoundtripBzip2AllLevels)
{
    pplib::ByteArray original = createSampleData(2048);
    pplib::Compression::Level levels[] = {pplib::Compression::Level::Fast, pplib::Compression::Level::Normal,
                                          pplib::Compression::Level::High, pplib::Compression::Level::Default};

    for (auto lvl : levels) {
        pplib::Compression comp(pplib::Compression::Algorithm::BZIP2, lvl);
        pplib::ByteArray compressed = comp.compress(original);
        pplib::ByteArray decompressed = comp.uncompress(compressed);
        ASSERT_EQ(decompressed.size(), original.size());
        EXPECT_EQ(memcmp(decompressed.ptr(), original.ptr(), original.size()), 0);
    }
}

TEST(CompressionTest, EmptyDataHandling)
{
    pplib::ByteArray empty;

    // Algo_NONE
    {
        pplib::Compression comp(pplib::Compression::Algorithm::NONE);
        pplib::ByteArray c = comp.compress(empty);
        EXPECT_EQ(c.size(), 0);
        pplib::ByteArray u = comp.uncompress(c);
        EXPECT_EQ(u.size(), 0);
    }

    // Algo_ZLIB without prefix
    {
        pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);
        pplib::ByteArray c = comp.compress(empty);
        EXPECT_GT(c.size(), 0); // zlib header for empty stream
        pplib::ByteArray u = comp.uncompress(c);
        EXPECT_EQ(u.size(), 0);
    }

    // Algo_ZLIB with Prefix_V2
    {
        pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);
        comp.setPrefix(pplib::Compression::Prefix::V2);
        pplib::ByteArray c = comp.compress(empty);
        pplib::ByteArray u = comp.uncompress(c);
        EXPECT_EQ(u.size(), 0);
    }

    // Algo_BZIP2 without and with Prefix_V1
    {
        pplib::Compression comp(pplib::Compression::Algorithm::BZIP2);
        pplib::ByteArray c = comp.compress(empty);
        pplib::ByteArray u = comp.uncompress(c);
        EXPECT_EQ(u.size(), 0);

        comp.setPrefix(pplib::Compression::Prefix::V1);
        pplib::ByteArray c1 = comp.compress(empty);
        pplib::ByteArray u1 = comp.uncompress(c1);
        EXPECT_EQ(u1.size(), 0);
    }
}

TEST(CompressionTest, IncompressibleDataBoundSafety)
{
    // Random bytes cannot be compressed and expand slightly.
    pplib::ByteArray randomData = createRandomData(2048);

    pplib::Compression compZlib(pplib::Compression::Algorithm::ZLIB);
    pplib::ByteArray cZ = compZlib.compress(randomData);
    pplib::ByteArray uZ = compZlib.uncompress(cZ);
    ASSERT_EQ(uZ.size(), randomData.size());
    EXPECT_EQ(memcmp(uZ.ptr(), randomData.ptr(), randomData.size()), 0);

    pplib::Compression compBz(pplib::Compression::Algorithm::BZIP2);
    pplib::ByteArray cB = compBz.compress(randomData);
    pplib::ByteArray uB = compBz.uncompress(cB);
    ASSERT_EQ(uB.size(), randomData.size());
    EXPECT_EQ(memcmp(uB.ptr(), randomData.ptr(), randomData.size()), 0);
}

TEST(CompressionTest, LargeBufferExponentialGrowth)
{
    // 64 KB of highly compressible data tested without prefix to exercise buffer growth
    pplib::ByteArray largeData = createSampleData(65536);

    pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);
    comp.setPrefix(pplib::Compression::Prefix::NONE);
    pplib::ByteArray compressed = comp.compress(largeData);

    pplib::ByteArray decompressed = comp.uncompress(compressed);
    ASSERT_EQ(decompressed.size(), largeData.size());
    EXPECT_EQ(memcmp(decompressed.ptr(), largeData.ptr(), largeData.size()), 0);
}

TEST(CompressionTest, PrefixV1Roundtrip)
{
    pplib::Compression::Algorithm algos[] = {pplib::Compression::Algorithm::NONE, pplib::Compression::Algorithm::ZLIB,
                                             pplib::Compression::Algorithm::BZIP2};

    pplib::ByteArray original = createSampleData(512);

    for (auto algo : algos) {
        pplib::Compression comp(algo);
        comp.setPrefix(pplib::Compression::Prefix::V1);

        pplib::ByteArray compressed = comp.compress(original);
        ASSERT_GE(compressed.size(), 9);

        // Check V1 header
        uint8_t flag = ((const uint8_t*)compressed.ptr())[0];
        EXPECT_EQ(flag & 7, static_cast<unsigned>(algo));
        EXPECT_EQ(pplib::Peek32((const char*)compressed.ptr() + 1), original.size());
        EXPECT_EQ(pplib::Peek32((const char*)compressed.ptr() + 5), compressed.size() - 9);

        pplib::ByteArray decompressed = comp.uncompress(compressed);
        ASSERT_EQ(decompressed.size(), original.size());
        EXPECT_EQ(memcmp(decompressed.ptr(), original.ptr(), original.size()), 0);
    }
}

TEST(CompressionTest, PrefixV2VariableSizes)
{
    pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);
    comp.setPrefix(pplib::Compression::Prefix::V2);

    // 1-byte size (<= 255)
    {
        pplib::ByteArray small = createSampleData(100);
        pplib::ByteArray c = comp.compress(small);
        uint8_t flag = ((const uint8_t*)c.ptr())[0];
        EXPECT_TRUE(flag & 8);         // Version 2 bit set
        EXPECT_EQ((flag >> 4) & 3, 0); // 1 byte uncompressed
        pplib::ByteArray u = comp.uncompress(c);
        ASSERT_EQ(u.size(), small.size());
        EXPECT_EQ(memcmp(u.ptr(), small.ptr(), small.size()), 0);
    }

    // 2-byte size (> 255 && <= 65535)
    {
        pplib::ByteArray medium = createSampleData(1000);
        pplib::ByteArray c = comp.compress(medium);
        uint8_t flag = ((const uint8_t*)c.ptr())[0];
        EXPECT_TRUE(flag & 8);
        EXPECT_EQ((flag >> 4) & 3, 1); // 2 bytes uncompressed
        pplib::ByteArray u = comp.uncompress(c);
        ASSERT_EQ(u.size(), medium.size());
        EXPECT_EQ(memcmp(u.ptr(), medium.ptr(), medium.size()), 0);
    }

    // 3-byte size (> 65535)
    {
        pplib::ByteArray large = createSampleData(70000);
        pplib::ByteArray c = comp.compress(large);
        uint8_t flag = ((const uint8_t*)c.ptr())[0];
        EXPECT_TRUE(flag & 8);
        EXPECT_EQ((flag >> 4) & 3, 2); // 3 bytes uncompressed
        pplib::ByteArray u = comp.uncompress(c);
        ASSERT_EQ(u.size(), large.size());
        EXPECT_EQ(memcmp(u.ptr(), large.ptr(), large.size()), 0);
    }
}

TEST(CompressionTest, OutParameterOverloads)
{
    pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);
    pplib::ByteArray original = createSampleData(500);

    pplib::ByteArray compressed;
    comp.compress(compressed, original);
    EXPECT_GT(compressed.size(), 0);

    pplib::ByteArray decompressed;
    comp.uncompress(decompressed, compressed);
    ASSERT_EQ(decompressed.size(), original.size());
    EXPECT_EQ(memcmp(decompressed.ptr(), original.ptr(), original.size()), 0);

    // Overload taking (const void*, size_t)
    pplib::ByteArray compressed2;
    comp.compress(compressed2, original.ptr(), original.size());
    pplib::ByteArray decompressed2;
    comp.uncompress(decompressed2, compressed2.ptr(), compressed2.size());
    ASSERT_EQ(decompressed2.size(), original.size());
    EXPECT_EQ(memcmp(decompressed2.ptr(), original.ptr(), original.size()), 0);

    // Overload returning ByteArray taking (const void*, size_t)
    pplib::ByteArray compressed3 = comp.compress(original.ptr(), original.size());
    pplib::ByteArray decompressed3 = comp.uncompress(compressed3.ptr(), compressed3.size());
    ASSERT_EQ(decompressed3.size(), original.size());
    EXPECT_EQ(memcmp(decompressed3.ptr(), original.ptr(), original.size()), 0);
}

TEST(CompressionTest, LowLevelApi)
{
    pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);
    pplib::ByteArray original = createSampleData(500);

    std::vector<char> dst(2048);
    size_t dstlen = dst.size();
    comp.compress(dst.data(), &dstlen, original.ptr(), original.size(), pplib::Compression::Algorithm::ZLIB);
    EXPECT_GT(dstlen, 0);

    std::vector<char> uncompressed(2048);
    size_t uncompressed_len = uncompressed.size();
    comp.uncompress(uncompressed.data(), &uncompressed_len, dst.data(), dstlen, pplib::Compression::Algorithm::ZLIB);
    ASSERT_EQ(uncompressed_len, original.size());
    EXPECT_EQ(memcmp(uncompressed.data(), original.ptr(), original.size()), 0);
}

TEST(CompressionTest, LowLevelBufferTooSmallException)
{
    pplib::Compression comp;
    pplib::ByteArray original = createSampleData(500);

    // Compress buffer too small
    std::vector<char> dstBuffer(1024);
    size_t tinyLen = 2;
    EXPECT_THROW(comp.compress(dstBuffer.data(), &tinyLen, original.ptr(), original.size(), pplib::Compression::Algorithm::NONE),
                 pplib::BufferTooSmallException);

    tinyLen = 2;
    EXPECT_THROW(comp.compress(dstBuffer.data(), &tinyLen, original.ptr(), original.size(), pplib::Compression::Algorithm::ZLIB),
                 pplib::BufferTooSmallException);

    tinyLen = 2;
    EXPECT_THROW(comp.compress(dstBuffer.data(), &tinyLen, original.ptr(), original.size(), pplib::Compression::Algorithm::BZIP2),
                 pplib::BufferTooSmallException);

    // Uncompress buffer too small
    pplib::ByteArray compressed = comp.compress(original);
    tinyLen = 2;
    EXPECT_THROW(comp.uncompress(dstBuffer.data(), &tinyLen, compressed.ptr(), compressed.size(), pplib::Compression::Algorithm::ZLIB),
                 pplib::BufferTooSmallException);

    tinyLen = 2;
    EXPECT_THROW(comp.uncompress(dstBuffer.data(), &tinyLen, compressed.ptr(), compressed.size(), pplib::Compression::Algorithm::NONE),
                 pplib::BufferTooSmallException);

    pplib::Compression compBz(pplib::Compression::Algorithm::BZIP2);
    pplib::ByteArray compressedBz = compBz.compress(original);
    tinyLen = 2;
    EXPECT_THROW(
        compBz.uncompress(dstBuffer.data(), &tinyLen, compressedBz.ptr(), compressedBz.size(), pplib::Compression::Algorithm::BZIP2),
        pplib::BufferTooSmallException);
}

TEST(CompressionTest, IllegalArgumentExceptions)
{
    pplib::Compression comp;
    char dst[64];
    size_t dstlen = sizeof(dst);
    const char* src = "hello";

    EXPECT_THROW(comp.compress(nullptr, &dstlen, src, 5), pplib::IllegalArgumentException);
    EXPECT_THROW(comp.compress(dst, nullptr, src, 5), pplib::IllegalArgumentException);
    EXPECT_THROW(comp.compress(dst, &dstlen, nullptr, 5), pplib::IllegalArgumentException);

    EXPECT_THROW(comp.uncompress(nullptr, &dstlen, src, 5), pplib::IllegalArgumentException);
    EXPECT_THROW(comp.uncompress(dst, nullptr, src, 5), pplib::IllegalArgumentException);
    EXPECT_THROW(comp.uncompress(dst, &dstlen, nullptr, 5), pplib::IllegalArgumentException);
}

TEST(CompressionTest, CorruptedDataExceptions)
{
    // Garbage data to Zlib
    {
        pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);
        const char garbage[] = "this is definitely not a zlib stream";
        EXPECT_THROW(comp.uncompress(garbage, sizeof(garbage)), pplib::CorruptedDataException);
    }

    // Garbage data to Bzip2
    {
        pplib::Compression comp(pplib::Compression::Algorithm::BZIP2);
        const char garbage[] = "this is definitely not a bzip2 stream";
        EXPECT_THROW(comp.uncompress(garbage, sizeof(garbage)), pplib::CorruptedDataException);
    }

    // Prefix V1 corrupted
    {
        pplib::Compression comp;
        comp.setPrefix(pplib::Compression::Prefix::V1);
        char shortData[4] = {0, 0, 0, 0};
        EXPECT_THROW(comp.uncompress(shortData, 4), pplib::CorruptedDataException);

        char truncated[10] = {1, 100, 0, 0, 0, 50, 0, 0, 0, 0};
        EXPECT_THROW(comp.uncompress(truncated, 10), pplib::CorruptedDataException);
    }

    // Prefix V2 corrupted
    {
        pplib::Compression comp;
        comp.setPrefix(pplib::Compression::Prefix::V2);
        EXPECT_THROW(comp.uncompress(nullptr, 0), pplib::CorruptedDataException);

        // Bit 3 not set
        char wrongFlag[8] = {1, 0, 0, 0, 0, 0, 0, 0};
        EXPECT_THROW(comp.uncompress(wrongFlag, 8), pplib::CorruptedDataException);

        // Header too short for declared byte counts
        char tooShort[2] = {(char)(8 | 48 | 192), 0}; // says 4 bytes unc + 4 bytes comp, but total size is 2
        EXPECT_THROW(comp.uncompress(tooShort, 2), pplib::CorruptedDataException);

        // Deklarierte komprimierte Groesse ueberschreitet den vorhandenen Puffer
        char truncated[4] = {static_cast<char>(0x08 | 0x01), 10, static_cast<char>(200), 0};
        EXPECT_THROW(comp.uncompress(truncated, 4), pplib::CorruptedDataException);
    }
}

TEST(CompressionTest, IllegalArgumentException)
{
    pplib::Compression comp;
    char dst[64];
    size_t dstlen = sizeof(dst);
    const char src[] = "test";

    EXPECT_THROW(comp.compress(dst, &dstlen, src, 4, (pplib::Compression::Algorithm)999), pplib::IllegalArgumentException);
    EXPECT_THROW(comp.uncompress(dst, &dstlen, src, 4, (pplib::Compression::Algorithm)999), pplib::IllegalArgumentException);
}

TEST(CompressionTest, StandaloneFunctions)
{
    pplib::ByteArray original = createSampleData(1024);

    // Compress / Uncompress generic
    {
        pplib::ByteArray c1 = pplib::Compress(original, pplib::Compression::Algorithm::ZLIB);
        pplib::ByteArray u1 = pplib::Uncompress(c1);
        ASSERT_EQ(u1.size(), original.size());
        EXPECT_EQ(memcmp(u1.ptr(), original.ptr(), original.size()), 0);

        pplib::ByteArray c2;
        pplib::Compress(c2, original, pplib::Compression::Algorithm::ZLIB);
        pplib::ByteArray u2;
        pplib::Uncompress(u2, c2);
        ASSERT_EQ(u2.size(), original.size());
        EXPECT_EQ(memcmp(u2.ptr(), original.ptr(), original.size()), 0);
    }

    // CompressZlib
    {
        pplib::ByteArray c1 = pplib::CompressZlib(original);
        pplib::ByteArray u1 = pplib::Uncompress(c1);
        ASSERT_EQ(u1.size(), original.size());
        EXPECT_EQ(memcmp(u1.ptr(), original.ptr(), original.size()), 0);

        pplib::ByteArray c2;
        pplib::CompressZlib(c2, original);
        pplib::ByteArray u2 = pplib::Uncompress(c2);
        ASSERT_EQ(u2.size(), original.size());
        EXPECT_EQ(memcmp(u2.ptr(), original.ptr(), original.size()), 0);
    }

    // CompressBZip2
    {
        pplib::ByteArray c1 = pplib::CompressBZip2(original);
        pplib::ByteArray u1 = pplib::Uncompress(c1);
        ASSERT_EQ(u1.size(), original.size());
        EXPECT_EQ(memcmp(u1.ptr(), original.ptr(), original.size()), 0);

        pplib::ByteArray c2;
        pplib::CompressBZip2(c2, original);
        pplib::ByteArray u2 = pplib::Uncompress(c2);
        ASSERT_EQ(u2.size(), original.size());
        EXPECT_EQ(memcmp(u2.ptr(), original.ptr(), original.size()), 0);
    }
}

// ---------------------------------------------------------------------------
// MAX_DATA_SIZE: 32-Bit-Laengenfelder der Low-Level-Bibliotheken
// ---------------------------------------------------------------------------

TEST(CompressionTest, MaxDataSizeLimitLowLevel)
{
    pplib::Compression comp;
    char src[] = "hello";
    char dst[64];
    const size_t tooBig = pplib::Compression::MAX_DATA_SIZE + 1;

    // zu grosse Quelldaten
    size_t dstlen = sizeof(dst);
    EXPECT_THROW(comp.compress(dst, &dstlen, src, tooBig), pplib::IllegalArgumentException);
    dstlen = sizeof(dst);
    EXPECT_THROW(comp.uncompress(dst, &dstlen, src, tooBig), pplib::IllegalArgumentException);

    // zu grosse Zielpuffer
    dstlen = tooBig;
    EXPECT_THROW(comp.compress(dst, &dstlen, src, sizeof(src)), pplib::IllegalArgumentException);
    dstlen = tooBig;
    EXPECT_THROW(comp.uncompress(dst, &dstlen, src, sizeof(src)), pplib::IllegalArgumentException);

    // Der Grenzwert selbst ist erlaubt
    dstlen = pplib::Compression::MAX_DATA_SIZE;
    EXPECT_NO_THROW(comp.compress(dst, &dstlen, src, sizeof(src)));
}

TEST(CompressionTest, MaxDataSizeLimitByteArray)
{
    char buf[] = "test";
    pplib::ByteArrayPtr huge(buf, pplib::Compression::MAX_DATA_SIZE + 1);
    pplib::Compression comp;

    EXPECT_THROW(comp.compress(huge), pplib::IllegalArgumentException);
    EXPECT_THROW(comp.uncompress(huge), pplib::IllegalArgumentException);

    pplib::ByteArray out;
    EXPECT_THROW(comp.compress(out, huge), pplib::IllegalArgumentException);
    EXPECT_THROW(comp.uncompress(out, huge), pplib::IllegalArgumentException);
}

// ---------------------------------------------------------------------------
// BufferTooSmallException: dstlen = bereits geschriebene Bytes
// ---------------------------------------------------------------------------

TEST(CompressionTest, BufferTooSmallReportsWrittenBytes)
{
    // Zufallsdaten lassen sich praktisch nicht komprimieren
    pplib::ByteArray original = createRandomData(5000);
    std::vector<char> dst(100);

    pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);
    size_t dstlen = dst.size();
    try {
        comp.compress(dst.data(), &dstlen, original.ptr(), original.size());
        FAIL() << "Expected BufferTooSmallException";
    }
    catch (const pplib::BufferTooSmallException&) {
        EXPECT_EQ(dstlen, 100);
    }

    // Algorithm::NONE meldet die exakt benoetigte Groesse
    pplib::Compression copy(pplib::Compression::Algorithm::NONE);
    dstlen = 10;
    try {
        copy.compress(dst.data(), &dstlen, original.ptr(), original.size());
        FAIL() << "Expected BufferTooSmallException";
    }
    catch (const pplib::BufferTooSmallException&) {
        EXPECT_EQ(dstlen, 5000);
    }
}

// ---------------------------------------------------------------------------
// Prefix-Auswertung: Flag-Bits und Längenfelder
// ---------------------------------------------------------------------------

TEST(CompressionTest, PrefixV1FlagValidation)
{
    pplib::Compression comp;
    comp.setPrefix(pplib::Compression::Prefix::V1);

    // reservierte Bits 3..7 sind nicht leer
    char reserved[9] = {static_cast<char>(0x08), 5, 0, 0, 0, 0, 0, 0, 0};
    EXPECT_THROW(comp.uncompress(reserved, 9), pplib::CorruptedDataException);

    // im Flag kodierter, nicht unterstuetzter Algorithmus
    char unknown[9] = {3, 0, 0, 0, 0, 0, 0, 0, 0};
    EXPECT_THROW(comp.uncompress(unknown, 9), pplib::CorruptedDataException);
}

TEST(CompressionTest, PrefixV1HugeLengthField)
{
    // Regressionstest: frueher lief die Groessenpruefung bei 32 Bit ueber,
    // weil (size_comp + 9) > size nicht mehr erkannt wurde.
    pplib::Compression comp;
    comp.setPrefix(pplib::Compression::Prefix::V1);

    char data[10] = {1, 5, 0, 0, 0, static_cast<char>(0xFF), static_cast<char>(0xFF), static_cast<char>(0xFF), static_cast<char>(0xFF), 0};
    EXPECT_THROW(comp.uncompress(data, 10), pplib::CorruptedDataException);
}

TEST(CompressionTest, PrefixV2UnknownAlgorithm)
{
    pplib::Compression comp;
    comp.setPrefix(pplib::Compression::Prefix::V2);

    // Bit 3 gesetzt (V2), aber Algorithmus-Bits = 5
    char data[4] = {static_cast<char>(0x08 | 5), 0, 0, 0};
    EXPECT_THROW(comp.uncompress(data, 4), pplib::CorruptedDataException);
}

TEST(CompressionTest, PrefixV2RoundtripNoneAndBzip2)
{
    pplib::ByteArray original = createSampleData(2000);

    pplib::Compression copy(pplib::Compression::Algorithm::NONE);
    copy.setPrefix(pplib::Compression::Prefix::V2);
    pplib::ByteArray cNone = copy.compress(original);
    pplib::ByteArray uNone = copy.uncompress(cNone);
    ASSERT_EQ(uNone.size(), original.size());
    EXPECT_EQ(memcmp(uNone.ptr(), original.ptr(), original.size()), 0);

    pplib::Compression bz(pplib::Compression::Algorithm::BZIP2);
    bz.setPrefix(pplib::Compression::Prefix::V2);
    pplib::ByteArray cBz = bz.compress(original);

    pplib::ByteArray uBz = bz.uncompress(cBz);
    ASSERT_EQ(uBz.size(), original.size());
    EXPECT_EQ(memcmp(uBz.ptr(), original.ptr(), original.size()), 0);

    // Der Algorithmus kommt aus dem Prefix, nicht aus der Einstellung des Objekts
    pplib::Compression reader;
    reader.setPrefix(pplib::Compression::Prefix::V2);
    pplib::ByteArray uRead = reader.uncompress(cBz);
    ASSERT_EQ(uRead.size(), original.size());
    EXPECT_EQ(memcmp(uRead.ptr(), original.ptr(), original.size()), 0);
}

TEST(CompressionTest, PrefixV1EmptyRoundtrip)
{
    pplib::ByteArray empty;
    pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);
    comp.setPrefix(pplib::Compression::Prefix::V1);

    pplib::ByteArray c = comp.compress(empty);
    pplib::ByteArray u = comp.uncompress(c);
    EXPECT_EQ(u.size(), 0);
}

// ---------------------------------------------------------------------------
// Out-Parameter: Ergebnis ersetzt den alten Pufferinhalt
// ---------------------------------------------------------------------------

TEST(CompressionTest, OutParameterReplacesOldContent)
{
    pplib::ByteArray original = createSampleData(3000);
    pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);

    pplib::ByteArray out = comp.compress(original);
    ASSERT_NE(out.size(), 0);

    // zweiter Aufruf in denselben Puffer darf nichts anhaengen
    size_t oldSize = out.size();
    comp.compress(out, original);
    EXPECT_EQ(out.size(), oldSize);

    pplib::ByteArray back = comp.uncompress(out);
    ASSERT_EQ(back.size(), original.size());
    EXPECT_EQ(memcmp(back.ptr(), original.ptr(), original.size()), 0);
}

TEST(CompressionTest, StandaloneUncompressCorruptHeader)
{
    pplib::ByteArray garbage;
    char* p = static_cast<char*>(garbage.malloc(20));
    memset(p, 0x41, 20);
    EXPECT_THROW(pplib::Uncompress(garbage), pplib::CorruptedDataException);
}

// ---------------------------------------------------------------------------
// Defekte / abgeschnittene Daten ohne Prefix
// ---------------------------------------------------------------------------

TEST(CompressionTest, TruncatedStreamNoPrefix)
{
    pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);
    pplib::ByteArray c = comp.compress(createRandomData(5000));
    ASSERT_GT(c.size(), 10u);
    c.truncate(c.size() / 2);
    EXPECT_THROW(comp.uncompress(c), pplib::CorruptedDataException);
}

TEST(CompressionTest, PrefixV1DataReadAsV2)
{
    pplib::Compression writer(pplib::Compression::Algorithm::NONE);
    writer.setPrefix(pplib::Compression::Prefix::V1);
    pplib::ByteArray c = writer.compress(createSampleData(300));

    pplib::Compression reader;
    reader.setPrefix(pplib::Compression::Prefix::V2);
    EXPECT_THROW(reader.uncompress(c), pplib::CorruptedDataException);
}

// ---------------------------------------------------------------------------
// Laengenfelder mit variabler Breite (V2)
// ---------------------------------------------------------------------------

TEST(CompressionTest, PrefixV2ThreeByteLengthFields)
{
    // > 64 KiB inkompressible Daten -> 3-Bit-Kennung, also je 3 Byte Feldlaenge
    pplib::ByteArray original = createRandomData(100000);
    pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);
    comp.setPrefix(pplib::Compression::Prefix::V2);

    pplib::ByteArray c = comp.compress(original);
    uint8_t flag = ((const uint8_t*)c.ptr())[0];
    EXPECT_EQ((flag >> 4) & 3, 2);
    EXPECT_EQ((flag >> 6) & 3, 2);

    pplib::ByteArray u = comp.uncompress(c);
    ASSERT_EQ(u.size(), original.size());
    EXPECT_EQ(memcmp(u.ptr(), original.ptr(), original.size()), 0);
}

TEST(CompressionTest, PrefixV2FourByteLengthFields)
{
    // Von Hand gebauter V2-Header mit 4 Byte breiten Laengenfeldern
    pplib::Compression zlib(pplib::Compression::Algorithm::ZLIB);
    pplib::ByteArray original = createSampleData(1000);
    pplib::ByteArray body = zlib.compress(original);

    pplib::ByteArray c;
    char* p = static_cast<char*>(c.malloc(body.size() + 9));
    p[0] = static_cast<char>(0x01 | 0x08 | (3 << 4) | (3 << 6));
    pplib::Poke32(p + 1, static_cast<uint32_t>(original.size()));
    pplib::Poke32(p + 5, static_cast<uint32_t>(body.size()));
    memcpy(p + 9, body.ptr(), body.size());
    c.truncate(body.size() + 9);

    pplib::Compression reader;
    reader.setPrefix(pplib::Compression::Prefix::V2);
    pplib::ByteArray u = reader.uncompress(c);
    ASSERT_EQ(u.size(), original.size());
    EXPECT_EQ(memcmp(u.ptr(), original.ptr(), original.size()), 0);
}

// Langsamer Test: 4 Byte breite Laengenfelder werden erst ab > 16 MiB geschrieben
TEST(CompressionTest, PrefixV2FourByteEncodeFields)
{
    pplib::ByteArray original = createRandomData(0x01000001);
    pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);
    comp.setPrefix(pplib::Compression::Prefix::V2);

    pplib::ByteArray c = comp.compress(original);
    uint8_t flag = ((const uint8_t*)c.ptr())[0];
    EXPECT_EQ((flag >> 4) & 3, 3);
    EXPECT_EQ((flag >> 6) & 3, 3);
    EXPECT_EQ(pplib::Peek32((const char*)c.ptr() + 1), original.size());

    pplib::ByteArray u = comp.uncompress(c);
    ASSERT_EQ(u.size(), original.size());
    EXPECT_EQ(memcmp(u.ptr(), original.ptr(), original.size()), 0);
}

// ---------------------------------------------------------------------------
// Algorithmus-Parameter der Low-Level-API
// ---------------------------------------------------------------------------
TEST(CompressionTest, LowLevelAlgorithmOverride)
{
    pplib::Compression comp(pplib::Compression::Algorithm::ZLIB);
    pplib::ByteArray original = createSampleData(500);
    std::vector<char> dst(4096);
    std::vector<char> out(4096);

    // Das Argument ueberstimmt die Instanzeinstellung
    size_t dstlen = dst.size();
    comp.compress(dst.data(), &dstlen, original.ptr(), original.size(), pplib::Compression::Algorithm::BZIP2);

    size_t outlen = out.size();
    comp.uncompress(out.data(), &outlen, dst.data(), dstlen, pplib::Compression::Algorithm::BZIP2);
    ASSERT_EQ(outlen, original.size());
    EXPECT_EQ(memcmp(out.data(), original.ptr(), original.size()), 0);

    // Unknown (Default) -> Algorithmus der Instanz
    dstlen = dst.size();
    comp.compress(dst.data(), &dstlen, original.ptr(), original.size());
    outlen = out.size();
    comp.uncompress(out.data(), &outlen, dst.data(), dstlen);
    EXPECT_EQ(outlen, original.size());
}

} // namespace
