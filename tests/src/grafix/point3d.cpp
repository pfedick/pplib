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

#include <pplib/grafix/point.h>
#include "pplib-tests.h"

namespace
{

TEST(GrafixPoint3DTest, Constructors)
{
    pplib::grafix::Point3D p1;
    EXPECT_EQ(0, p1.x);
    EXPECT_EQ(0, p1.y);
    EXPECT_EQ(0, p1.z);

    pplib::grafix::Point3D p2(-7, 12, 3);
    EXPECT_EQ(-7, p2.x);
    EXPECT_EQ(12, p2.y);
    EXPECT_EQ(3, p2.z);

    // Copy
    pplib::grafix::Point3D p3(p2);
    EXPECT_EQ(-7, p3.x);
    EXPECT_EQ(12, p3.y);
    EXPECT_EQ(3, p3.z);
}

TEST(GrafixPoint3DTest, isNull)
{
    pplib::grafix::Point3D p1;
    EXPECT_TRUE(p1.isNull());

    EXPECT_FALSE(pplib::grafix::Point3D(1, 0, 0).isNull());
    EXPECT_FALSE(pplib::grafix::Point3D(0, 1, 0).isNull());
    EXPECT_FALSE(pplib::grafix::Point3D(0, 0, 1).isNull());
}

TEST(GrafixPoint3DTest, Setters)
{
    pplib::grafix::Point3D p1, p2;
    p1.setX(7);
    p1.setY(14);
    p1.setZ(-21);
    EXPECT_EQ(7, p1.x);
    EXPECT_EQ(14, p1.y);
    EXPECT_EQ(-21, p1.z);

    p1.setPoint(334, -124, 5);
    EXPECT_EQ(334, p1.x);
    EXPECT_EQ(-124, p1.y);
    EXPECT_EQ(5, p1.z);

    p2.setPoint(p1);
    EXPECT_EQ(334, p2.x);
    EXPECT_EQ(-124, p2.y);
    EXPECT_EQ(5, p2.z);
}

TEST(GrafixPoint3DTest, CompoundAssignmentOperators)
{
    pplib::grafix::Point3D p1(10, 20, 30);

    p1 += pplib::grafix::Point3D(5, -8, 1);
    EXPECT_EQ(15, p1.x);
    EXPECT_EQ(12, p1.y);
    EXPECT_EQ(31, p1.z);

    p1 -= pplib::grafix::Point3D(5, 2, 1);
    EXPECT_EQ(10, p1.x);
    EXPECT_EQ(10, p1.y);
    EXPECT_EQ(30, p1.z);

    p1 *= 1.5;
    EXPECT_EQ(15, p1.x);
    EXPECT_EQ(15, p1.y);
    EXPECT_EQ(45, p1.z);

    p1 /= 3.0;
    EXPECT_EQ(5, p1.x);
    EXPECT_EQ(5, p1.y);
    EXPECT_EQ(15, p1.z);
}

TEST(GrafixPoint3DTest, ArithmeticOperators)
{
    pplib::grafix::Point3D p1(10, 20, 30);
    pplib::grafix::Point3D p2(-3, -8, 5);

    pplib::grafix::Point3D sum = p1 + p2;
    EXPECT_EQ(pplib::grafix::Point3D(7, 12, 35), sum);

    pplib::grafix::Point3D diff = p1 - p2;
    EXPECT_EQ(pplib::grafix::Point3D(13, 28, 25), diff);

    // Originale duerfen unveraendert geblieben sein
    EXPECT_EQ(10, p1.x);
    EXPECT_EQ(20, p1.y);
    EXPECT_EQ(30, p1.z);

    pplib::grafix::Point3D negated = -p1;
    EXPECT_EQ(pplib::grafix::Point3D(-10, -20, -30), negated);
    EXPECT_EQ(pplib::grafix::Point3D(0, 0, 0), -pplib::grafix::Point3D());

    pplib::grafix::Point3D scaled1 = p1 * 2.5;
    EXPECT_EQ(pplib::grafix::Point3D(25, 50, 75), scaled1);

    pplib::grafix::Point3D scaled2 = 2.5 * p1;
    EXPECT_EQ(pplib::grafix::Point3D(25, 50, 75), scaled2);

    pplib::grafix::Point3D divided = p1 / 4.0;
    EXPECT_EQ(pplib::grafix::Point3D(2, 5, 7), divided);

    // Ganzzahlige Koordinaten werden Richtung Null abgeschnitten
    pplib::grafix::Point3D truncated = pplib::grafix::Point3D(-7, 3, -9) / 2.0;
    EXPECT_EQ(pplib::grafix::Point3D(-3, 1, -4), truncated);
}

TEST(GrafixPoint3DTest, ComparisonOperators)
{
    const pplib::grafix::Point3D a(5, 6, 7);
    const pplib::grafix::Point3D b(5, 6, 7);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);

    EXPECT_TRUE(a != pplib::grafix::Point3D(4, 6, 7));
    EXPECT_TRUE(a != pplib::grafix::Point3D(5, 5, 7));
    EXPECT_TRUE(a != pplib::grafix::Point3D(5, 6, 6));
    EXPECT_FALSE(a == pplib::grafix::Point3D(5, 6, 8));
}

} // namespace
