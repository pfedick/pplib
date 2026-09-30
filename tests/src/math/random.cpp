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

#include <pplib/core/random.h>

#include "pplib-tests.h"

namespace
{

TEST(RandomTest, ReproducibilityWithSeed)
{
    pplib::Random::seed(42);
    size_t a1 = pplib::Random::get(0, 100000);
    size_t a2 = pplib::Random::get(0, 100000);

    pplib::Random::seed(42);
    size_t b1 = pplib::Random::get(0, 100000);
    size_t b2 = pplib::Random::get(0, 100000);

    EXPECT_EQ(a1, b1);
    EXPECT_EQ(a2, b2);
}

TEST(RandomTest, IntegerRangeAndSwap)
{
    EXPECT_EQ(42, pplib::Random::get(42, 42));

    for (int i = 0; i < 100; ++i) {
        size_t val = pplib::Random::get(10, 20);
        EXPECT_GE(val, 10u);
        EXPECT_LE(val, 20u);

        // Prüft, ob std::swap(min, max) greift:
        size_t swapped = pplib::Random::get(20, 10);
        EXPECT_GE(swapped, 10u);
        EXPECT_LE(swapped, 20u);
    }
}

TEST(RandomTest, FloatRange)
{
    EXPECT_FLOAT_EQ(3.14f, pplib::Random::getFloat(3.14f, 3.14f));

    // Prüft, ob std::swap(min, max) greift:
    float swapped = pplib::Random::getFloat(5.0f, 1.0f);
    EXPECT_GE(swapped, 1.0f);
    EXPECT_LE(swapped, 5.0f);

    for (int i = 0; i < 100; ++i) {
        float val = pplib::Random::getFloat(1.0f, 5.0f);
        EXPECT_GE(val, 1.0f);
        EXPECT_LE(val, 5.0f);
    }
}

TEST(RandomTest, DoubleRange)
{
    EXPECT_DOUBLE_EQ(3.14f, pplib::Random::getDouble(3.14f, 3.14f));

    // Prüft, ob std::swap(min, max) greift:
    double swapped = pplib::Random::getDouble(5.0f, 1.0f);
    EXPECT_GE(swapped, 1.0f);
    EXPECT_LE(swapped, 5.0f);

    for (int i = 0; i < 100; ++i) {
        double val = pplib::Random::getDouble(1.0f, 5.0f);
        EXPECT_GE(val, 1.0f);
        EXPECT_LE(val, 5.0f);
    }
}

TEST(RandomTest, BytesAndFill)
{
    // Edge Cases
    EXPECT_EQ(0u, pplib::Random::bytes(0).size());

    pplib::ByteArray buf;
    buf.append("12345", 5);
    pplib::Random::fill(buf, 0);
    EXPECT_EQ(0u, buf.size());

    // Füllen mit existierender Größe
    buf.realloc(64);
    pplib::Random::fill(buf);
    EXPECT_EQ(64u, buf.size());

    // Plausibilität: Puffer darf nicht nur aus Nullen bestehen
    bool has_nonzero = false;
    for (size_t i = 0; i < buf.size(); ++i) {
        if (buf[i] != 0) {
            has_nonzero = true;
            break;
        }
    }
    EXPECT_TRUE(has_nonzero);
}

TEST(RandomTest, GlobalFunctions)
{
    pplib::srand(123);
    size_t r1 = pplib::rand(1, 100);
    pplib::srand(123);
    size_t r2 = pplib::rand(1, 100);
    EXPECT_EQ(r1, r2);

    EXPECT_GE(pplib::randf(0.0f, 1.0f), 0.0f);
    EXPECT_GE(pplib::randd(0.0, 1.0), 0.0);
}

} // namespace