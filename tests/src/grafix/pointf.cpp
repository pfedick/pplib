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

TEST(GrafixPointFTest, DefaultConstructors)
{
    ASSERT_NO_THROW({
        pplib::grafix::PointF p1;
        EXPECT_EQ(0.0f, p1.x);
        EXPECT_EQ(0.0f, p1.y);
    });
    pplib::grafix::PointF p2(19.123, 42.34);
    EXPECT_EQ(19.123f, p2.x);
    EXPECT_EQ(42.34f, p2.y);

    // Copy
    pplib::grafix::PointF p3(p2);
    EXPECT_EQ(19.123f, p3.x);
    EXPECT_EQ(42.34f, p3.y);
    // Move

    pplib::grafix::PointF p4(std::move(p3));
    EXPECT_EQ(19.123f, p4.x);
    EXPECT_EQ(42.34f, p4.y);
}

TEST(GrafixPointFTest, ConstructorFromPoint)
{
    ASSERT_NO_THROW({
        pplib::grafix::Point p1(-7, 12);
        pplib::grafix::PointF pf1(p1);
        EXPECT_EQ(-7.0f, pf1.x);
        EXPECT_EQ(12.0f, pf1.y);
    });
}

TEST(GrafixPointFTest, Assignment)
{
    pplib::grafix::PointF pf1(5.5f, 10.25f);
    pplib::grafix::PointF pf2;
    pf2 = pf1;
    EXPECT_EQ(5.5f, pf2.x);
    EXPECT_EQ(10.25f, pf2.y);

    // Move assignment
    pplib::grafix::PointF pf3;
    pf3 = std::move(pf2);
    EXPECT_EQ(5.5f, pf3.x);
    EXPECT_EQ(10.25f, pf3.y);

    // Zuweisung von einem Point
    pplib::grafix::PointF pf4;
    pf4 = pplib::grafix::Point(-3, 8);
    EXPECT_EQ(-3.0f, pf4.x);
    EXPECT_EQ(8.0f, pf4.y);
}

TEST(GrafixPointFTest, isNull)
{
    pplib::grafix::PointF p1;
    EXPECT_TRUE(p1.isNull());

    pplib::grafix::PointF p2(5.5f, 10.5f);
    EXPECT_FALSE(p2.isNull());

    pplib::grafix::PointF p3(0.0f, -0.0f);
    EXPECT_TRUE(p3.isNull());
}

TEST(GrafixPointFTest, Setters)
{
    pplib::grafix::PointF pf1, pf2;
    pf1.setPoint(7.5f, -14.25f);
    EXPECT_EQ(7.5f, pf1.x);
    EXPECT_EQ(-14.25f, pf1.y);

    pf2.setPoint(pf1);
    EXPECT_EQ(7.5f, pf2.x);
    EXPECT_EQ(-14.25f, pf2.y);

    pplib::grafix::Point p1(334, -124);
    pf2.setPoint(p1);
    EXPECT_EQ(334.0f, pf2.x);
    EXPECT_EQ(-124.0f, pf2.y);
}

TEST(GrafixPointFTest, InsideRect)
{
    pplib::grafix::Rect r1(0, 0, 10, 10);
    EXPECT_TRUE(pplib::grafix::PointF(5.0f, 5.0f).inside(r1));
    EXPECT_TRUE(pplib::grafix::PointF(0.0f, 0.0f).inside(r1));
    EXPECT_TRUE(pplib::grafix::PointF(9.0f, 9.0f).inside(r1));
    EXPECT_FALSE(pplib::grafix::PointF(9.5f, 9.5f).inside(r1));
    EXPECT_FALSE(pplib::grafix::PointF(10.0f, 10.0f).inside(r1));
    EXPECT_FALSE(pplib::grafix::PointF(-0.5f, 0.0f).inside(r1));
    EXPECT_FALSE(pplib::grafix::PointF(0.0f, -0.5f).inside(r1));
}

TEST(GrafixPointFTest, VectorLength)
{
    pplib::grafix::PointF p1(3.0f, 4.0f);
    EXPECT_DOUBLE_EQ(static_cast<pplib::grafix::real_t>(5), p1.vectorLength());

    pplib::grafix::PointF p2;
    EXPECT_DOUBLE_EQ(static_cast<pplib::grafix::real_t>(0), p2.vectorLength());

    pplib::grafix::PointF p3(-3.0f, -4.0f);
    EXPECT_DOUBLE_EQ(static_cast<pplib::grafix::real_t>(5), p3.vectorLength());
}

TEST(GrafixPointFTest, ManhattanLength)
{
    pplib::grafix::PointF p1(3.5f, 4.0f);
    EXPECT_FLOAT_EQ(7.5f, p1.manhattanLength());

    pplib::grafix::PointF p2(-3.5f, -4.0f);
    EXPECT_FLOAT_EQ(7.5f, p2.manhattanLength());

    pplib::grafix::PointF p3;
    EXPECT_FLOAT_EQ(0.0f, p3.manhattanLength());
}

TEST(GrafixPointFTest, Distance)
{
    EXPECT_DOUBLE_EQ(static_cast<pplib::grafix::real_t>(5),
                     pplib::grafix::Distance(pplib::grafix::PointF(1.0f, 2.0f), pplib::grafix::PointF(4.0f, 6.0f)));
    EXPECT_DOUBLE_EQ(static_cast<pplib::grafix::real_t>(5),
                     pplib::grafix::Distance(pplib::grafix::PointF(4.0f, 6.0f), pplib::grafix::PointF(1.0f, 2.0f)));
    EXPECT_DOUBLE_EQ(static_cast<pplib::grafix::real_t>(0),
                     pplib::grafix::Distance(pplib::grafix::PointF(7.5f, -3.0f), pplib::grafix::PointF(7.5f, -3.0f)));
}

TEST(GrafixPointFTest, CompoundAssignmentOperators)
{
    pplib::grafix::PointF pf1(10.0f, 20.0f);

    pf1 += pplib::grafix::PointF(5.5f, -8.0f);
    EXPECT_FLOAT_EQ(15.5f, pf1.x);
    EXPECT_FLOAT_EQ(12.0f, pf1.y);

    pf1 -= pplib::grafix::PointF(5.5f, 2.0f);
    EXPECT_FLOAT_EQ(10.0f, pf1.x);
    EXPECT_FLOAT_EQ(10.0f, pf1.y);

    pf1 *= 1.5;
    EXPECT_FLOAT_EQ(15.0f, pf1.x);
    EXPECT_FLOAT_EQ(15.0f, pf1.y);

    pf1 /= 3.0;
    EXPECT_FLOAT_EQ(5.0f, pf1.x);
    EXPECT_FLOAT_EQ(5.0f, pf1.y);
}

TEST(GrafixPointFTest, ArithmeticOperators)
{
    pplib::grafix::PointF pf1(10.0f, 20.0f);
    pplib::grafix::PointF pf2(-3.5f, -8.0f);

    pplib::grafix::PointF sum = pf1 + pf2;
    EXPECT_FLOAT_EQ(6.5f, sum.x);
    EXPECT_FLOAT_EQ(12.0f, sum.y);

    pplib::grafix::PointF diff = pf1 - pf2;
    EXPECT_FLOAT_EQ(13.5f, diff.x);
    EXPECT_FLOAT_EQ(28.0f, diff.y);

    // Originale duerfen unveraendert geblieben sein
    EXPECT_FLOAT_EQ(10.0f, pf1.x);
    EXPECT_FLOAT_EQ(20.0f, pf1.y);

    pplib::grafix::PointF negated = -pf1;
    EXPECT_FLOAT_EQ(-10.0f, negated.x);
    EXPECT_FLOAT_EQ(-20.0f, negated.y);
    EXPECT_TRUE(pplib::grafix::PointF(0.0f, 0.0f) == -pplib::grafix::PointF());

    pplib::grafix::PointF scaled1 = pf1 * 2.5;
    EXPECT_FLOAT_EQ(25.0f, scaled1.x);
    EXPECT_FLOAT_EQ(50.0f, scaled1.y);

    pplib::grafix::PointF scaled2 = 2.5 * pf1;
    EXPECT_FLOAT_EQ(25.0f, scaled2.x);
    EXPECT_FLOAT_EQ(50.0f, scaled2.y);

    pplib::grafix::PointF divided = pf1 / 4.0;
    EXPECT_FLOAT_EQ(2.5f, divided.x);
    EXPECT_FLOAT_EQ(5.0f, divided.y);
}

TEST(GrafixPointFTest, ComparisonOperators)
{
    const pplib::grafix::PointF a(5.0f, 5.0f);
    const pplib::grafix::PointF b(5.0f, 5.0f);
    const pplib::grafix::PointF smallerY(9.0f, 4.0f);
    const pplib::grafix::PointF greaterY(-9.0f, 6.0f);
    const pplib::grafix::PointF smallerX(4.0f, 5.0f);
    const pplib::grafix::PointF greaterX(6.0f, 5.0f);

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
