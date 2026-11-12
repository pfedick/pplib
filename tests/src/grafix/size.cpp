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

#include <pplib/grafix/size.h>
#include "pplib-tests.h"

namespace
{

TEST(GrafixSizeTest, Constructors)
{
    pplib::grafix::Size s1;
    EXPECT_EQ(0, s1.width);
    EXPECT_EQ(0, s1.height);

    pplib::grafix::Size s2(640, -480);
    EXPECT_EQ(640, s2.width);
    EXPECT_EQ(-480, s2.height);

    // Copy
    pplib::grafix::Size s3(s2);
    EXPECT_EQ(640, s3.width);
    EXPECT_EQ(-480, s3.height);
}

TEST(GrafixSizeTest, isNull)
{
    EXPECT_TRUE(pplib::grafix::Size().isNull());
    EXPECT_TRUE(pplib::grafix::Size(0, 0).isNull());
    EXPECT_FALSE(pplib::grafix::Size(0, 5).isNull());
    EXPECT_FALSE(pplib::grafix::Size(5, 0).isNull());
    EXPECT_FALSE(pplib::grafix::Size(-1, -1).isNull());
}

TEST(GrafixSizeTest, isEmpty)
{
    EXPECT_TRUE(pplib::grafix::Size().isEmpty());
    EXPECT_TRUE(pplib::grafix::Size(0, 5).isEmpty());
    EXPECT_TRUE(pplib::grafix::Size(5, 0).isEmpty());
    EXPECT_FALSE(pplib::grafix::Size(5, 5).isEmpty());
    EXPECT_FALSE(pplib::grafix::Size(-1, -1).isEmpty());
}

TEST(GrafixSizeTest, isValid)
{
    EXPECT_TRUE(pplib::grafix::Size().isValid());
    EXPECT_TRUE(pplib::grafix::Size(0, 5).isValid());
    EXPECT_TRUE(pplib::grafix::Size(5, 0).isValid());
    EXPECT_TRUE(pplib::grafix::Size(5, 5).isValid());
    EXPECT_FALSE(pplib::grafix::Size(-1, 5).isValid());
    EXPECT_FALSE(pplib::grafix::Size(5, -1).isValid());
    EXPECT_FALSE(pplib::grafix::Size::invalid().isValid());
}

TEST(GrafixSizeTest, Setters)
{
    pplib::grafix::Size s1, s2;
    s1.setWidth(7);
    s1.setHeight(14);
    EXPECT_EQ(7, s1.width);
    EXPECT_EQ(14, s1.height);

    s1.setSize(334, -124);
    EXPECT_EQ(334, s1.width);
    EXPECT_EQ(-124, s1.height);

    s2.setSize(s1);
    EXPECT_EQ(334, s2.width);
    EXPECT_EQ(-124, s2.height);
}

TEST(GrafixSizeTest, Invalid)
{
    pplib::grafix::Size s = pplib::grafix::Size::invalid();
    EXPECT_EQ(-1, s.width);
    EXPECT_EQ(-1, s.height);
    EXPECT_FALSE(s.isValid());
}

TEST(GrafixSizeTest, CompoundAssignmentOperators)
{
    pplib::grafix::Size s1(10, 20);

    s1 += pplib::grafix::Size(5, -8);
    EXPECT_EQ(15, s1.width);
    EXPECT_EQ(12, s1.height);

    s1 -= pplib::grafix::Size(5, 2);
    EXPECT_EQ(10, s1.width);
    EXPECT_EQ(10, s1.height);

    s1 *= 1.5;
    EXPECT_EQ(15, s1.width);
    EXPECT_EQ(15, s1.height);

    s1 /= 3.0;
    EXPECT_EQ(5, s1.width);
    EXPECT_EQ(5, s1.height);
}

TEST(GrafixSizeTest, ArithmeticOperators)
{
    pplib::grafix::Size s1(10, 20);
    pplib::grafix::Size s2(-3, -8);

    pplib::grafix::Size sum = s1 + s2;
    EXPECT_EQ(pplib::grafix::Size(7, 12), sum);

    pplib::grafix::Size diff = s1 - s2;
    EXPECT_EQ(pplib::grafix::Size(13, 28), diff);

    // Originale duerfen unveraendert geblieben sein
    EXPECT_EQ(10, s1.width);
    EXPECT_EQ(20, s1.height);

    pplib::grafix::Size negated = -s1;
    EXPECT_EQ(pplib::grafix::Size(-10, -20), negated);

    pplib::grafix::Size scaled1 = s1 * 2.5;
    EXPECT_EQ(pplib::grafix::Size(25, 50), scaled1);

    pplib::grafix::Size scaled2 = 2.5 * s1;
    EXPECT_EQ(pplib::grafix::Size(25, 50), scaled2);

    pplib::grafix::Size divided = s1 / 4.0;
    EXPECT_EQ(pplib::grafix::Size(2, 5), divided);

    // Ganzzahlige Werte werden Richtung Null abgeschnitten
    pplib::grafix::Size truncated = pplib::grafix::Size(-7, 3) / 2.0;
    EXPECT_EQ(pplib::grafix::Size(-3, 1), truncated);
}

TEST(GrafixSizeTest, ComparisonOperators)
{
    const pplib::grafix::Size a(5, 6);
    const pplib::grafix::Size b(5, 6);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);

    EXPECT_TRUE(a != pplib::grafix::Size(4, 6));
    EXPECT_TRUE(a != pplib::grafix::Size(5, 5));
    EXPECT_FALSE(a == pplib::grafix::Size(-5, -6));
    EXPECT_FALSE(a == pplib::grafix::Size());
}

} // namespace
