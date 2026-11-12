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

#include <cstdio>
#include <string>
#include <type_traits>

#include <pplib/types/string.h>
#include <pplib/grafix/point.h>
#include <pplib/grafix/rect.h>
#include "pplib-tests.h"

namespace
{

TEST(GrafixPointTest, DefaultConstructors)
{
    ASSERT_NO_THROW({
        pplib::grafix::Point p1;
        EXPECT_EQ(0, p1.x);
        EXPECT_EQ(0, p1.y);
    });
    pplib::grafix::Point p2(19, 42);
    EXPECT_EQ(19, p2.x);
    EXPECT_EQ(42, p2.y);

    // Copy
    pplib::grafix::Point p3(p2);
    EXPECT_EQ(19, p3.x);
    EXPECT_EQ(42, p3.y);
    // Move

    pplib::grafix::Point p4(std::move(p3));
    EXPECT_EQ(19, p4.x);
    EXPECT_EQ(42, p4.y);
}

TEST(GrafixPointTest, ConstructorWithPointF)
{
    ASSERT_NO_THROW({
        pplib::grafix::PointF pf1(1.5f, 2.5f);
        pplib::grafix::Point p1(pf1);
        EXPECT_EQ(1, p1.x);
        EXPECT_EQ(2, p1.y);
    });
}

TEST(GrafixPointTest, Assignment)
{
    pplib::grafix::Point p1(5, 10);
    pplib::grafix::Point p2;
    p2 = p1;
    EXPECT_EQ(5, p2.x);
    EXPECT_EQ(10, p2.y);

    // Move assignment
    pplib::grafix::Point p3;
    p3 = std::move(p2);
    EXPECT_EQ(5, p3.x);
    EXPECT_EQ(10, p3.y);
}

TEST(GrafixPointTest, isNull)
{
    pplib::grafix::Point p1;
    EXPECT_TRUE(p1.isNull());

    pplib::grafix::Point p2(5, 10);
    EXPECT_FALSE(p2.isNull());
}

TEST(GrafixPointTest, Setters)
{
    pplib::grafix::Point p1, p2;
    p1.setX(7);
    p1.setY(14);
    EXPECT_EQ(7, p1.x);
    EXPECT_EQ(14, p1.y);

    p1.setPoint(334, -124);
    EXPECT_EQ(334, p1.x);
    EXPECT_EQ(-124, p1.y);

    p2.setPoint(p1);
    EXPECT_EQ(334, p2.x);
    EXPECT_EQ(-124, p2.y);

    pplib::grafix::PointF pf1(245.123f, -45.5f);
    p1.setPoint(pf1);
    EXPECT_EQ(245, p1.x);
    EXPECT_EQ(-45, p1.y);
}

TEST(GrafixPointTest, InsideRect)
{
    pplib::grafix::Point p1(5, 5);
    pplib::grafix::Rect r1(0, 0, 10, 10);
    EXPECT_TRUE(p1.inside(r1));

    EXPECT_TRUE(pplib::grafix::Point(5, 5).inside(r1));
    EXPECT_TRUE(pplib::grafix::Point(0, 0).inside(r1));
    EXPECT_TRUE(pplib::grafix::Point(9, 9).inside(r1));
    EXPECT_FALSE(pplib::grafix::Point(10, 10).inside(r1));
    EXPECT_FALSE(pplib::grafix::Point(-1, 0).inside(r1));
    EXPECT_FALSE(pplib::grafix::Point(0, -1).inside(r1));
    EXPECT_FALSE(pplib::grafix::Point(10, 0).inside(r1));
    EXPECT_FALSE(pplib::grafix::Point(0, 10).inside(r1));
}

TEST(GrafixPointTest, VectorLength)
{
    pplib::grafix::Point p1(3, 4);
    EXPECT_EQ(static_cast<pplib::grafix::real_t>(5), p1.vectorLength());
}

} // namespace
