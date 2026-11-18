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

#include <pplib/crypto/securerandom.h>
#include <pplib/exceptions.h>

#include <cstring>
#include <limits>

#include "pplib-tests.h"

namespace
{

class SecureRandomTest : public ::testing::Test
{
protected:
    SecureRandomTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~SecureRandomTest()
    {
    }
};

TEST_F(SecureRandomTest, BytesZeroReturnsEmpty)
{
    pplib::ByteArray buf = pplib::SecureRandom::bytes(0);
    EXPECT_EQ(0u, buf.size());
}

TEST_F(SecureRandomTest, BytesGeneratesRequestedSizeAndDistinctValues)
{
    pplib::ByteArray a = pplib::SecureRandom::bytes(32);
    pplib::ByteArray b = pplib::SecureRandom::bytes(32);

    ASSERT_EQ(32u, a.size());
    ASSERT_EQ(32u, b.size());
    EXPECT_NE(0, std::memcmp(a.ptr(), b.ptr(), 32));
}

TEST_F(SecureRandomTest, BytesHandlesMultiChunkSizes)
{
    constexpr size_t exactChunk = 65536;
    pplib::ByteArray buf1 = pplib::SecureRandom::bytes(exactChunk);
    ASSERT_EQ(exactChunk, buf1.size());

    constexpr size_t multiChunk = 65536 * 2 + 123;
    pplib::ByteArray buf2 = pplib::SecureRandom::bytes(multiChunk);
    ASSERT_EQ(multiChunk, buf2.size());

    // Stichprobe über Chunk-Grenzen hinweg: nicht alle Bytes am Ende dürfen 0 sein
    const unsigned char* tail = static_cast<const unsigned char*>(buf2.ptr()) + 65536;
    bool hasNonZero = false;
    for (size_t i = 0; i < multiChunk - 65536; ++i) {
        if (tail[i] != 0) {
            hasNonZero = true;
            break;
        }
    }
    EXPECT_TRUE(hasNonZero);
}

TEST_F(SecureRandomTest, FillWithSizeZeroClearsBuffer)
{
    pplib::ByteArray buf(16);
    std::memset(const_cast<void*>(buf.ptr()), 0xAA, 16);

    pplib::ByteArray& ref = pplib::SecureRandom::fill(buf, 0);
    EXPECT_EQ(&buf, &ref);
    EXPECT_EQ(0u, buf.size());
}

TEST_F(SecureRandomTest, FillWithSizeResizesAndOverwrites)
{
    pplib::ByteArray buf(8);
    std::memset(const_cast<void*>(buf.ptr()), 0xAA, 8);

    // Wachsen lassen
    pplib::ByteArray& refGrow = pplib::SecureRandom::fill(buf, 64);
    EXPECT_EQ(&buf, &refGrow);
    ASSERT_EQ(64u, buf.size());

    // Gleiche Größe überschreiben
    pplib::ByteArray previous(buf);
    pplib::ByteArray& refSame = pplib::SecureRandom::fill(buf, 64);
    EXPECT_EQ(&buf, &refSame);
    ASSERT_EQ(64u, buf.size());
    EXPECT_NE(0, std::memcmp(previous.ptr(), buf.ptr(), 64));

    // Schrumpfen lassen
    pplib::ByteArray& refShrink = pplib::SecureRandom::fill(buf, 16);
    EXPECT_EQ(&buf, &refShrink);
    EXPECT_EQ(16u, buf.size());
}

TEST_F(SecureRandomTest, FillWithSizeMultiChunk)
{
    constexpr size_t size = 65536 + 512;
    pplib::ByteArray buf;
    pplib::ByteArray& ref = pplib::SecureRandom::fill(buf, size);
    EXPECT_EQ(&buf, &ref);
    ASSERT_EQ(size, buf.size());
}

TEST_F(SecureRandomTest, FillExistingBuffer)
{
    pplib::ByteArray empty;
    pplib::ByteArray& refEmpty = pplib::SecureRandom::fill(empty);
    EXPECT_EQ(&empty, &refEmpty);
    EXPECT_EQ(0u, empty.size());

    pplib::ByteArray buf(32);
    std::memset(const_cast<void*>(buf.ptr()), 0x55, 32);
    pplib::ByteArray original(buf);

    pplib::ByteArray& ref = pplib::SecureRandom::fill(buf);
    EXPECT_EQ(&buf, &ref);
    ASSERT_EQ(32u, buf.size());
    EXPECT_NE(0, std::memcmp(original.ptr(), buf.ptr(), 32));
}

TEST_F(SecureRandomTest, IntegerRangeAndSwap)
{
    EXPECT_EQ(0u, pplib::SecureRandom::get(0, 0));
    EXPECT_EQ(42u, pplib::SecureRandom::get(42, 42));

    for (int i = 0; i < 100; ++i) {
        size_t val = pplib::SecureRandom::get(10, 20);
        EXPECT_GE(val, 10u);
        EXPECT_LE(val, 20u);

        // Prüft, ob std::swap(min, max) greift:
        size_t swapped = pplib::SecureRandom::get(20, 10);
        EXPECT_GE(swapped, 10u);
        EXPECT_LE(swapped, 20u);
    }
}

TEST_F(SecureRandomTest, IntegerFullRangeAndNearMax)
{
    const size_t maxVal = std::numeric_limits<size_t>::max();
    EXPECT_NO_THROW({
        size_t full = pplib::SecureRandom::get(0, maxVal);
        EXPECT_LE(full, maxVal);
        size_t fullSwapped = pplib::SecureRandom::get(maxVal, 0);
        EXPECT_LE(fullSwapped, maxVal);
    });

    const size_t maxMinusOne = maxVal - 1;
    size_t val = pplib::SecureRandom::get(0, maxMinusOne);
    EXPECT_LE(val, maxMinusOne);
}

TEST_F(SecureRandomTest, IntegerBinaryRangeProducesBothValues)
{
    bool seenZero = false;
    bool seenOne = false;
    for (int i = 0; i < 100; ++i) {
        size_t val = pplib::SecureRandom::get(0, 1);
        EXPECT_LE(val, 1u);
        if (val == 0) seenZero = true;
        if (val == 1) seenOne = true;
    }
    EXPECT_TRUE(seenZero);
    EXPECT_TRUE(seenOne);
}

} // namespace
