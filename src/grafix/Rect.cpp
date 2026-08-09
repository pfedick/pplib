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
#include <pplib/grafix/rect.h>
#include <pplib/grafix/point.h>

namespace pplib::grafix
{

inline static int max(int v1, int v2)
{
    if (v1 > v2) return v1;
    return v2;
}

inline static int min(int v1, int v2)
{
    if (v1 < v2) return v1;
    return v2;
}

inline static int abs(int v)
{
    if (v < 0) return -v;
    return v;
}

Rect Rect::fromPoints(const Point& p1, const Point& p2)
{
    /// Hilfsfunktion, um ein Rechteck aus zwei Punkten zu erstellen, ohne sich Gedanken über die Reihenfolge der Punkte machen zu müssen
    Rect r;
    r.x = min(p1.x, p2.x);
    r.y = min(p1.y, p2.y);
    r.w = abs(p1.x - p2.x);
    r.h = abs(p1.y - p2.y);
    return r;
}

Rect Rect::fromCoordsInclusive(int x1, int y1, int x2, int y2)
{
    /// Hilfsfunktion, um ein Rechteck aus den Koordinaten der oberen linken und unteren rechten Ecke zu erstellen,
    /// wobei die unteren rechten Koordinaten "inklusiv" sind, also innerhalb des Rechtecks liegen.
    /// Falls die Koordinaten nicht in der richtigen Reihenfolge angegeben werden, wird automatisch das obere linke und das
    /// untere rechte Ende des Rechtecks bestimmt.
    Rect r;
    r.x = min(x1, x2);
    r.y = min(y1, y2);
    r.w = abs(x2 - x1) + 1;
    r.h = abs(y2 - y1) + 1;
    return r;
}

Rect::Rect()
{
    x = 0;
    y = 0;
    w = 0;
    h = 0;
}

Rect::Rect(const Point& p1, const Point& p2)
{
    *this = fromPoints(p1, p2);
}

Rect::Rect(const Point& p, const Size& s)
{
    x = p.x;
    y = p.y;
    w = s.width;
    h = s.height;
}

Rect::Rect(const Rect16& other)
{
    x = other.left();
    y = other.top();
    w = other.width();
    h = other.height();
}

Rect::Rect(int x, int y, int width, int height)
{
    this->x = x;
    this->y = y;
    this->w = width;
    this->h = height;
}

bool Rect::isNull() const
{
    if (w == 0 || h == 0) return true;
    return false;
}

bool Rect::intersects(const Rect& other) const
{
    if (isNull() || other.isNull()) return false;
    return left() < other.right() && right() > other.left() && top() < other.bottom() && bottom() > other.top();
}

Rect Rect::intersected(const Rect& other) const
{
    if (isNull() || other.isNull() || !intersects(other)) {
        return Rect();
    }

    int resX = max(x, other.x);
    int resY = max(y, other.y);
    int resW = min(right(), other.right()) - resX;
    int resH = min(bottom(), other.bottom()) - resY;

    return Rect(resX, resY, resW, resH);
}

void Rect::setTopLeft(const Point& topLeft)
{
    x = topLeft.x;
    y = topLeft.y;
}

void Rect::setBottomRight(const Point& bottomRight)
{
    w = abs(bottomRight.x - x);
    if (bottomRight.x < x) {
        x = bottomRight.x;
    }
    h = abs(bottomRight.y - y);
    if (bottomRight.y < y) {
        y = bottomRight.y;
    }
}

void Rect::setRect(int x, int y, int width, int height)
{
    this->x = x;
    this->y = y;
    this->w = width;
    this->h = height;
}

void Rect::setRect(const Rect& other)
{
    x = other.x;
    y = other.y;
    w = other.w;
    h = other.h;
}

void Rect::setRect(const Rect16& other)
{
    x = other.left();
    y = other.top();
    w = other.width();
    h = other.height();
}

void Rect::setCoords(int x1, int y1, int x2, int y2)
{
    x = min(x1, x2);
    y = min(y1, y2);
    w = abs(x2 - x1);
    h = abs(y2 - y1);
}

void Rect::setCoords(const Point& p1, const Point& p2)
{
    *this = fromPoints(p1, p2);
}

void Rect::setRight(int right)
{
    w = abs(right - x);
    if (right < x) {
        x = right;
    }
}

void Rect::setBottom(int bottom)
{
    h = abs(bottom - y);
    if (bottom < y) {
        y = bottom;
    }
}

Rect& Rect::operator=(const Rect16& other)
{
    x = other.left();
    y = other.top();
    w = other.width();
    h = other.height();
    return *this;
}

bool operator!=(const Rect& r1, const Rect& r2)
{
    if (r1.x != r2.x) return true;
    if (r1.y != r2.y) return true;
    if (r1.w != r2.w) return true;
    if (r1.h != r2.h) return true;
    return false;
}

bool operator==(const Rect& r1, const Rect& r2)
{
    if (r1.x != r2.x) return false;
    if (r1.y != r2.y) return false;
    if (r1.w != r2.w) return false;
    if (r1.h != r2.h) return false;
    return true;
}

} // namespace pplib::grafix
