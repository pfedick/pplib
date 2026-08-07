/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include "../include/pplib.h"
#include "../include/pplib-grafix.h"
#include <gtest/gtest.h>
#include "pplib-tests.h"

namespace
{

// The fixture for testing class Foo.
class GrafixRectTest : public ::testing::Test
{
protected:
    GrafixRectTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~GrafixRectTest()
    {
    }
};

TEST_F(GrafixRectTest, constructor_withoutParams)
{

    ASSERT_NO_THROW({
        pplib::grafix::Rect r;
        ASSERT_EQ((int)0, r.x1);
        ASSERT_EQ((int)0, r.y1);
        ASSERT_EQ((int)0, r.x2);
        ASSERT_EQ((int)0, r.y2);
    });
}

TEST_F(GrafixRectTest, constructor_fromPoints)
{
    pplib::grafix::Point p1(10, 20);
    pplib::grafix::Point p2(60, 70);

    ASSERT_NO_THROW({
        pplib::grafix::Rect r(p1, p2);
        ASSERT_EQ((int)10, r.x1);
        ASSERT_EQ((int)20, r.y1);
        ASSERT_EQ((int)60, r.x2);
        ASSERT_EQ((int)70, r.y2);
    });
}

TEST_F(GrafixRectTest, constructor_fromPointAndSize)
{
    pplib::grafix::Point p1(10, 20);
    pplib::grafix::Size s1(50, 50);

    ASSERT_NO_THROW({
        pplib::grafix::Rect r(p1, s1);
        ASSERT_EQ((int)10, r.x1);
        ASSERT_EQ((int)20, r.y1);
        ASSERT_EQ((int)60, r.x2);
        ASSERT_EQ((int)70, r.y2);
    });
}

TEST_F(GrafixRectTest, constructor_fromIntegers)
{

    ASSERT_NO_THROW({
        pplib::grafix::Rect r(10, 20, 50, 50);
        ASSERT_EQ((int)10, r.x1);
        ASSERT_EQ((int)20, r.y1);
        ASSERT_EQ((int)60, r.x2);
        ASSERT_EQ((int)70, r.y2);
    });
}

TEST_F(GrafixRectTest, constructor_fromOtherRect)
{
    pplib::grafix::Rect r1(10, 20, 50, 50);
    ASSERT_NO_THROW({
        pplib::grafix::Rect r(r1);
        ASSERT_EQ((int)10, r.x1);
        ASSERT_EQ((int)20, r.y1);
        ASSERT_EQ((int)60, r.x2);
        ASSERT_EQ((int)70, r.y2);
    });
}

TEST_F(GrafixRectTest, isNull)
{
    pplib::grafix::Rect r1;
    ASSERT_TRUE(r1.isNull());
    pplib::grafix::Rect r2(10, 20, 0, 0);
    ASSERT_TRUE(r2.isNull());

    pplib::grafix::Rect r3(10, 20, 0, 30);
    ASSERT_FALSE(r3.isNull());

    pplib::grafix::Rect r4(10, 20, 20, 0);
    ASSERT_FALSE(r4.isNull());

    pplib::grafix::Rect r5(10, 20, 50, 50);
    ASSERT_FALSE(r5.isNull());
}

TEST_F(GrafixRectTest, left)
{
    pplib::grafix::Rect r(10, 20, 50, 50);
    ASSERT_EQ((int)10, r.left());
}

TEST_F(GrafixRectTest, right)
{
    pplib::grafix::Rect r(10, 20, 50, 50);
    ASSERT_EQ((int)59, r.right());
}

TEST_F(GrafixRectTest, top)
{
    pplib::grafix::Rect r(10, 20, 50, 50);
    ASSERT_EQ((int)20, r.top());
}

TEST_F(GrafixRectTest, bottom)
{
    pplib::grafix::Rect r(10, 20, 50, 50);
    ASSERT_EQ((int)69, r.bottom());
}

TEST_F(GrafixRectTest, width)
{
    pplib::grafix::Rect r(10, 20, 51, 52);
    ASSERT_EQ((int)51, r.width());
}

TEST_F(GrafixRectTest, height)
{
    pplib::grafix::Rect r(10, 20, 51, 52);
    ASSERT_EQ((int)52, r.height());
}

TEST_F(GrafixRectTest, size)
{
    pplib::grafix::Rect r(10, 20, 51, 52);
    ASSERT_EQ(pplib::grafix::Size(51, 52), r.size());
}

TEST_F(GrafixRectTest, topLeft)
{
    pplib::grafix::Rect r(10, 20, 51, 52);
    ASSERT_EQ(pplib::grafix::Point(10, 20), r.topLeft());
}

TEST_F(GrafixRectTest, topRight)
{
    pplib::grafix::Rect r(10, 20, 51, 52);
    ASSERT_EQ(pplib::grafix::Point(61, 20), r.topRight());
}

TEST_F(GrafixRectTest, bottomLeft)
{
    pplib::grafix::Rect r(10, 20, 51, 52);
    ASSERT_EQ(pplib::grafix::Point(10, 72), r.bottomLeft());
}

TEST_F(GrafixRectTest, bottomRight)
{
    pplib::grafix::Rect r(10, 20, 51, 52);
    ASSERT_EQ(pplib::grafix::Point(61, 72), r.bottomRight());
}

TEST_F(GrafixRectTest, normalized)
{
    pplib::grafix::Rect r1;
    r1.x1 = 50;
    r1.x2 = 20;
    r1.y1 = 60;
    r1.y2 = 30;
    pplib::grafix::Rect r2 = r1.normalized();
    ASSERT_EQ((int)20, r2.x1);
    ASSERT_EQ((int)50, r2.x2);
    ASSERT_EQ((int)30, r2.y1);
    ASSERT_EQ((int)60, r2.y2);

    pplib::grafix::Rect r3 = r1.normalized();
    ASSERT_EQ((int)20, r3.x1);
    ASSERT_EQ((int)50, r3.x2);
    ASSERT_EQ((int)30, r3.y1);
    ASSERT_EQ((int)60, r3.y2);
}

TEST_F(GrafixRectTest, intersects)
{
    pplib::grafix::Rect r1(10, 20, 60, 60);
    pplib::grafix::Rect r2(80, 30, 30, 30);
    ASSERT_FALSE(r2.intersects(r1));

    pplib::grafix::Rect r3(20, 30, 90, 90);
    ASSERT_TRUE(r3.intersects(r1));
}

TEST_F(GrafixRectTest, intersected)
{
    pplib::grafix::Rect r1(10, 20, 60, 60);
    pplib::grafix::Rect r2(80, 30, 30, 30);
    pplib::grafix::Rect r3(20, 30, 90, 90);
    pplib::grafix::Rect null;

    ASSERT_TRUE(r2.intersected(r1).isNull());
    ASSERT_TRUE(null.intersected(r1).isNull());
    ASSERT_TRUE(r2.intersected(null).isNull());

    pplib::grafix::Rect r = r3.intersected(r1);
    ASSERT_FALSE(r.isNull());
    ASSERT_EQ((int)20, r.x1);
    ASSERT_EQ((int)70, r.x2);
    ASSERT_EQ((int)30, r.y1);
    ASSERT_EQ((int)80, r.y2);
}

TEST_F(GrafixRectTest, setTopLeft)
{
    pplib::grafix::Rect r;
    r.setTopLeft(pplib::grafix::Point(31, 22));
    ASSERT_EQ((int)31, r.x1);
    ASSERT_EQ((int)22, r.y1);
    ASSERT_EQ((int)0, r.x2);
    ASSERT_EQ((int)0, r.y2);
}

TEST_F(GrafixRectTest, setBottomRight)
{
    pplib::grafix::Rect r;
    r.setBottomRight(pplib::grafix::Point(31, 22));
    ASSERT_EQ((int)0, r.x1);
    ASSERT_EQ((int)0, r.y1);
    ASSERT_EQ((int)31, r.x2);
    ASSERT_EQ((int)22, r.y2);
}

TEST_F(GrafixRectTest, setRect_withInts)
{
    pplib::grafix::Rect r;
    r.setRect(31, 22, 40, 50);
    ASSERT_EQ((int)31, r.x1);
    ASSERT_EQ((int)22, r.y1);
    ASSERT_EQ((int)71, r.x2);
    ASSERT_EQ((int)72, r.y2);
}

TEST_F(GrafixRectTest, setRect_withRect)
{
    pplib::grafix::Rect r1(45, 72, 44, 33);
    pplib::grafix::Rect r;
    r.setRect(r1);
    ASSERT_EQ((int)45, r.x1);
    ASSERT_EQ((int)72, r.y1);
    ASSERT_EQ((int)45 + 44, r.x2);
    ASSERT_EQ((int)72 + 33, r.y2);
}

TEST_F(GrafixRectTest, setRect_withStructRECT)
{
    pplib::grafix::RECT r1 = {45, 72, 45 + 44, 72 + 33};
    pplib::grafix::Rect r;
    r.setRect(r1);
    ASSERT_EQ((int)45, r.x1);
    ASSERT_EQ((int)72, r.y1);
    ASSERT_EQ((int)45 + 44, r.x2);
    ASSERT_EQ((int)72 + 33, r.y2);
}

TEST_F(GrafixRectTest, setCoords_withInt)
{
    pplib::grafix::Rect r;
    r.setCoords(31, 22, 40, 50);
    ASSERT_EQ((int)31, r.x1);
    ASSERT_EQ((int)22, r.y1);
    ASSERT_EQ((int)40, r.x2);
    ASSERT_EQ((int)50, r.y2);
}

TEST_F(GrafixRectTest, setCoords_withPoints)
{
    pplib::grafix::Rect r;
    r.setCoords(pplib::grafix::Point(31, 22), pplib::grafix::Point(40, 50));
    ASSERT_EQ((int)31, r.x1);
    ASSERT_EQ((int)22, r.y1);
    ASSERT_EQ((int)40, r.x2);
    ASSERT_EQ((int)50, r.y2);
}

} // namespace
