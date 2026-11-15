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

    pplib::grafix::Point p2;
    EXPECT_EQ(static_cast<pplib::grafix::real_t>(0), p2.vectorLength());

    pplib::grafix::Point p3(-3, -4);
    EXPECT_EQ(static_cast<pplib::grafix::real_t>(5), p3.vectorLength());
}

TEST(GrafixPointTest, ManhattanLength)
{
    pplib::grafix::Point p1(3, 4);
    EXPECT_EQ(7, p1.manhattanLength());

    pplib::grafix::Point p2(-3, -4);
    EXPECT_EQ(7, p2.manhattanLength());

    pplib::grafix::Point p3;
    EXPECT_EQ(0, p3.manhattanLength());
}

TEST(GrafixPointTest, Distance)
{
    EXPECT_EQ(static_cast<pplib::grafix::real_t>(5), pplib::grafix::Distance(pplib::grafix::Point(1, 2), pplib::grafix::Point(4, 6)));
    EXPECT_EQ(static_cast<pplib::grafix::real_t>(5), pplib::grafix::Distance(pplib::grafix::Point(4, 6), pplib::grafix::Point(1, 2)));
    EXPECT_EQ(static_cast<pplib::grafix::real_t>(0), pplib::grafix::Distance(pplib::grafix::Point(7, -3), pplib::grafix::Point(7, -3)));
}

TEST(GrafixPointTest, AssignmentFromPointF)
{
    pplib::grafix::Point p1(5, 10);
    pplib::grafix::PointF pf1(-1.5f, -2.5f);
    p1 = pf1;
    EXPECT_EQ(-1, p1.x);
    EXPECT_EQ(-2, p1.y);
}

TEST(GrafixPointTest, CompoundAssignmentOperators)
{
    pplib::grafix::Point p1(10, 20);

    p1 += pplib::grafix::Point(5, -8);
    EXPECT_EQ(15, p1.x);
    EXPECT_EQ(12, p1.y);

    p1 -= pplib::grafix::Point(5, 2);
    EXPECT_EQ(10, p1.x);
    EXPECT_EQ(10, p1.y);

    p1 *= 1.5;
    EXPECT_EQ(15, p1.x);
    EXPECT_EQ(15, p1.y);

    p1 /= 3.0;
    EXPECT_EQ(5, p1.x);
    EXPECT_EQ(5, p1.y);
}

TEST(GrafixPointTest, ArithmeticOperators)
{
    pplib::grafix::Point p1(10, 20);
    pplib::grafix::Point p2(-3, -8);

    pplib::grafix::Point sum = p1 + p2;
    EXPECT_EQ(pplib::grafix::Point(7, 12), sum);

    pplib::grafix::Point diff = p1 - p2;
    EXPECT_EQ(pplib::grafix::Point(13, 28), diff);

    // Originale duerfen unveraendert geblieben sein
    EXPECT_EQ(10, p1.x);
    EXPECT_EQ(20, p1.y);

    pplib::grafix::Point negated = -p1;
    EXPECT_EQ(pplib::grafix::Point(-10, -20), negated);
    EXPECT_EQ(pplib::grafix::Point(0, 0), -pplib::grafix::Point());

    pplib::grafix::Point scaled1 = p1 * 2.5;
    EXPECT_EQ(pplib::grafix::Point(25, 50), scaled1);

    pplib::grafix::Point scaled2 = 2.5 * p1;
    EXPECT_EQ(pplib::grafix::Point(25, 50), scaled2);

    pplib::grafix::Point divided = p1 / 4.0;
    EXPECT_EQ(pplib::grafix::Point(2, 5), divided);

    // Ganzzahlige Koordinaten werden Richtung Null abgeschnitten
    pplib::grafix::Point truncated = pplib::grafix::Point(-7, 3) / 2.0;
    EXPECT_EQ(pplib::grafix::Point(-3, 1), truncated);
}

TEST(GrafixPointTest, ComparisonOperators)
{
    const pplib::grafix::Point a(5, 5);
    const pplib::grafix::Point b(5, 5);
    const pplib::grafix::Point smallerY(9, 4);
    const pplib::grafix::Point greaterY(-9, 6);
    const pplib::grafix::Point smallerX(4, 5);
    const pplib::grafix::Point greaterX(6, 5);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != greaterX);
    EXPECT_FALSE(a == smallerX);

    // Vergleich zuerst nach Y, dann nach X
    EXPECT_TRUE(a < greaterY);
    EXPECT_TRUE(smallerY < a);
    EXPECT_FALSE(a < smallerY);
    EXPECT_FALSE(a < a);

    EXPECT_TRUE(a > smallerY);
    EXPECT_FALSE(smallerY > a);
    EXPECT_FALSE(a > greaterY);
    EXPECT_FALSE(a > a);

    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a <= greaterX);
    EXPECT_FALSE(a <= smallerX);

    EXPECT_TRUE(a >= b);
    EXPECT_TRUE(a >= smallerX);
    EXPECT_FALSE(a >= greaterX);
}

} // namespace
