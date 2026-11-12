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
#include <pplib/grafix/rect.h>
#include <pplib/grafix/size.h>
#include "pplib-tests.h"

namespace
{

TEST(GrafixRectTest, Constructors)
{
    pplib::grafix::Rect r1;
    EXPECT_TRUE(r1.isNull());
    EXPECT_EQ(0, r1.width());
    EXPECT_EQ(0, r1.height());

    pplib::grafix::Rect r2(10, 20, 30, 40);
    EXPECT_EQ(10, r2.left());
    EXPECT_EQ(20, r2.top());
    EXPECT_EQ(30, r2.width());
    EXPECT_EQ(40, r2.height());

    // Gegenueberliegende Ecken, Reihenfolge egal
    pplib::grafix::Rect r3(pplib::grafix::Point(50, 60), pplib::grafix::Point(10, 20));
    EXPECT_EQ(10, r3.left());
    EXPECT_EQ(20, r3.top());
    EXPECT_EQ(40, r3.width());
    EXPECT_EQ(40, r3.height());

    // Punkt + Groesse
    pplib::grafix::Rect r4(pplib::grafix::Point(3, 4), pplib::grafix::Size(5, 6));
    EXPECT_EQ(3, r4.left());
    EXPECT_EQ(4, r4.top());
    EXPECT_EQ(5, r4.width());
    EXPECT_EQ(6, r4.height());
}

TEST(GrafixRectTest, isNull)
{
    EXPECT_TRUE(pplib::grafix::Rect(0, 0, 0, 0).isNull());
    EXPECT_TRUE(pplib::grafix::Rect(5, 5, 0, 10).isNull());
    EXPECT_TRUE(pplib::grafix::Rect(5, 5, 10, 0).isNull());
    EXPECT_FALSE(pplib::grafix::Rect(5, 5, 1, 1).isNull());
}

TEST(GrafixRectTest, EdgeAccessors)
{
    const pplib::grafix::Rect r(10, 20, 30, 40);
    EXPECT_EQ(10, r.left());
    EXPECT_EQ(40, r.right()); // exklusiv
    EXPECT_EQ(20, r.top());
    EXPECT_EQ(60, r.bottom()); // exklusiv
    EXPECT_EQ(39, r.lastX());  // letztes Pixel innerhalb
    EXPECT_EQ(59, r.lastY());

    EXPECT_EQ(pplib::grafix::Size(30, 40), r.size());
    EXPECT_EQ(pplib::grafix::Point(10, 20), r.topLeft());
    EXPECT_EQ(pplib::grafix::Point(40, 20), r.topRight());
    EXPECT_EQ(pplib::grafix::Point(10, 60), r.bottomLeft());
    EXPECT_EQ(pplib::grafix::Point(40, 60), r.bottomRight());
}

TEST(GrafixRectTest, SimpleSetters)
{
    pplib::grafix::Rect r(10, 20, 30, 40);

    r.setX(1);
    r.setY(2);
    EXPECT_EQ(1, r.left());
    EXPECT_EQ(2, r.top());
    EXPECT_EQ(30, r.width());

    r.setLeft(3);
    r.setTop(4);
    EXPECT_EQ(3, r.left());
    EXPECT_EQ(4, r.top());

    r.setWidth(7);
    r.setHeight(8);
    EXPECT_EQ(7, r.width());
    EXPECT_EQ(8, r.height());

    r.setSize(pplib::grafix::Size(11, 12));
    EXPECT_EQ(11, r.width());
    EXPECT_EQ(12, r.height());

    r.setTopLeft(pplib::grafix::Point(50, 60));
    EXPECT_EQ(50, r.left());
    EXPECT_EQ(60, r.top());
    EXPECT_EQ(11, r.width());
}

TEST(GrafixRectTest, SetRightAndBottom)
{
    pplib::grafix::Rect r(10, 10, 5, 5);

    r.setRight(20);
    EXPECT_EQ(10, r.left());
    EXPECT_EQ(10, r.width());

    r.setBottom(5);
    // Neuer underer Rand liegt ueber der oberen Kante, y wird angepasst
    EXPECT_EQ(5, r.top());
    EXPECT_EQ(5, r.height());
    EXPECT_EQ(10, r.left());
    EXPECT_EQ(20, r.right());
}

TEST(GrafixRectTest, SetBottomRight)
{
    pplib::grafix::Rect r(10, 10, 5, 5);

    r.setBottomRight(pplib::grafix::Point(20, 30));
    EXPECT_EQ(10, r.left());
    EXPECT_EQ(10, r.top());
    EXPECT_EQ(10, r.width());
    EXPECT_EQ(20, r.height());

    // Punkt liegt links/ueber der oberen linken Ecke -> Ecke wird verschoben
    r.setBottomRight(pplib::grafix::Point(5, 8));
    EXPECT_EQ(5, r.left());
    EXPECT_EQ(5, r.width());
    EXPECT_EQ(8, r.top());
    EXPECT_EQ(2, r.height());
}

TEST(GrafixRectTest, SetRectAndSetCoords)
{
    pplib::grafix::Rect r;

    r.setRect(1, 2, 3, 4);
    EXPECT_EQ(pplib::grafix::Rect(1, 2, 3, 4), r);

    r.setRect(pplib::grafix::Rect(30, 40, 50, 60));
    EXPECT_EQ(pplib::grafix::Rect(30, 40, 50, 60), r);

    r.setCoords(0, 0, 10, 5);
    EXPECT_EQ(pplib::grafix::Rect(0, 0, 10, 5), r);

    // Reihenfolge der Punkte ist egal
    r.setCoords(10, 5, 0, 0);
    EXPECT_EQ(pplib::grafix::Rect(0, 0, 10, 5), r);

    r.setCoords(pplib::grafix::Point(10, 5), pplib::grafix::Point(0, 0));
    EXPECT_EQ(pplib::grafix::Rect(0, 0, 10, 5), r);
}

TEST(GrafixRectTest, FromPoints)
{
    // Rechteckkanten sind exklusiv: (10/10) liegt ausserhalb des resultierenden Rects
    EXPECT_EQ(pplib::grafix::Rect(0, 5, 10, 15), pplib::grafix::Rect::fromPoints(pplib::grafix::Point(10, 20), pplib::grafix::Point(0, 5)));
    EXPECT_TRUE(
        pplib::grafix::Point(9, 19).inside(pplib::grafix::Rect::fromPoints(pplib::grafix::Point(0, 5), pplib::grafix::Point(10, 20))));
    EXPECT_FALSE(
        pplib::grafix::Point(10, 20).inside(pplib::grafix::Rect::fromPoints(pplib::grafix::Point(0, 5), pplib::grafix::Point(10, 20))));
}

TEST(GrafixRectTest, FromCoordsInclusive)
{
    // Unterer rechter Rand ist inklusiv, ein Punkt ergibt ein 1x1-Rechteck
    EXPECT_EQ(pplib::grafix::Rect(0, 5, 11, 16), pplib::grafix::Rect::fromCoordsInclusive(10, 20, 0, 5));
    EXPECT_EQ(pplib::grafix::Rect(5, 5, 1, 1), pplib::grafix::Rect::fromCoordsInclusive(5, 5, 5, 5));
    EXPECT_TRUE(pplib::grafix::Point(5, 5).inside(pplib::grafix::Rect::fromCoordsInclusive(5, 5, 5, 5)));
}

TEST(GrafixRectTest, Intersects)
{
    const pplib::grafix::Rect a(0, 0, 10, 10);

    EXPECT_TRUE(a.intersects(pplib::grafix::Rect(5, 5, 10, 10)));
    EXPECT_TRUE(a.intersects(pplib::grafix::Rect(-5, -5, 10, 10)));
    EXPECT_TRUE(a.intersects(pplib::grafix::Rect(2, 2, 4, 4)));         // vollstaendig enthalten
    EXPECT_TRUE(a.intersects(pplib::grafix::Rect(-10, -10, 100, 100))); // enthaelt other

    // Kanten beruehren sich, ueberlappen aber nicht
    EXPECT_FALSE(a.intersects(pplib::grafix::Rect(10, 0, 5, 5)));
    EXPECT_FALSE(a.intersects(pplib::grafix::Rect(0, 10, 5, 5)));
    EXPECT_FALSE(a.intersects(pplib::grafix::Rect(11, 11, 5, 5)));

    // Null-Rechtecke schneiden nie
    EXPECT_FALSE(a.intersects(pplib::grafix::Rect(5, 5, 0, 5)));
    EXPECT_FALSE(pplib::grafix::Rect(5, 5, 0, 5).intersects(a));
    EXPECT_FALSE(a.intersects(a.intersected(pplib::grafix::Rect(100, 100, 5, 5))));
}

TEST(GrafixRectTest, Intersected)
{
    const pplib::grafix::Rect a(0, 0, 10, 10);

    EXPECT_EQ(pplib::grafix::Rect(5, 5, 5, 5), a.intersected(pplib::grafix::Rect(5, 5, 10, 10)));
    EXPECT_EQ(pplib::grafix::Rect(5, 5, 5, 5), pplib::grafix::Rect(5, 5, 10, 10).intersected(a));
    EXPECT_EQ(pplib::grafix::Rect(0, 0, 5, 5), a.intersected(pplib::grafix::Rect(-5, -5, 10, 10)));
    EXPECT_EQ(pplib::grafix::Rect(2, 2, 4, 4), a.intersected(pplib::grafix::Rect(2, 2, 4, 4)));
    EXPECT_EQ(pplib::grafix::Rect(0, 0, 10, 10), a.intersected(pplib::grafix::Rect(-10, -10, 100, 100)));

    // Keine Ueberlappung oder Null-Rechteck -> Null-Rechteck
    EXPECT_TRUE(a.intersected(pplib::grafix::Rect(20, 20, 5, 5)).isNull());
    EXPECT_TRUE(a.intersected(pplib::grafix::Rect(5, 5, 0, 5)).isNull());
}

TEST(GrafixRectTest, ComparisonOperators)
{
    const pplib::grafix::Rect a(1, 2, 3, 4);
    const pplib::grafix::Rect b(1, 2, 3, 4);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);

    EXPECT_TRUE(a != pplib::grafix::Rect(0, 2, 3, 4));
    EXPECT_TRUE(a != pplib::grafix::Rect(1, 0, 3, 4));
    EXPECT_TRUE(a != pplib::grafix::Rect(1, 2, 0, 4));
    EXPECT_TRUE(a != pplib::grafix::Rect(1, 2, 3, 0));
    EXPECT_FALSE(a == pplib::grafix::Rect(0, 0, 0, 0));
}

} // namespace
