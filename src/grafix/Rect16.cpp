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

inline static int16_t max(int16_t v1, int16_t v2)
{
    if (v1 > v2) return v1;
    return v2;
}

inline static int16_t min(int16_t v1, int16_t v2)
{
    if (v1 < v2) return v1;
    return v2;
}

inline static int16_t abs(int16_t v)
{
    if (v < 0) return -v;
    return v;
}

static inline int16_t clamp16(int value)
{
    if (value < -32768) return -32768;
    if (value > 32767) return 32767;
    return value;
}

Rect16 Rect16::fromPoints(const Point16& p1, const Point16& p2)
{
    /// Hilfsfunktion, um ein Rechteck aus zwei Punkten zu erstellen, ohne sich Gedanken über die Reihenfolge der Punkte machen zu müssen
    Rect16 r;
    r.x = min(p1.x, p2.x);
    r.y = min(p1.y, p2.y);
    r.w = abs(p1.x - p2.x);
    r.h = abs(p1.y - p2.y);
    return r;
}

Rect16 Rect16::fromCoordsInclusive(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    /// Hilfsfunktion, um ein Rechteck aus den Koordinaten der oberen linken und unteren rechten Ecke zu erstellen,
    /// wobei die unteren rechten Koordinaten "inklusiv" sind, also innerhalb des Rechtecks liegen.
    /// Falls die Koordinaten nicht in der richtigen Reihenfolge angegeben werden, wird automatisch das obere linke und das
    /// untere rechte Ende des Rechtecks bestimmt.
    Rect16 r;
    r.x = min(x1, x2);
    r.y = min(y1, y2);
    r.w = abs(x2 - x1) + 1;
    r.h = abs(y2 - y1) + 1;
    return r;
}

Rect16::Rect16()
{
    x = 0;
    y = 0;
    w = 0;
    h = 0;
}

Rect16::Rect16(const Point16& p1, const Point16& p2)
{
    *this = fromPoints(p1, p2);
}

Rect16::Rect16(const Point16& p, const Size16& s)
{
    x = p.x;
    y = p.y;
    w = s.width;
    h = s.height;
}

Rect16::Rect16(const Rect& other)
{
    x = clamp16(other.left());
    y = clamp16(other.top());
    w = clamp16(other.width());
    h = clamp16(other.height());
}

Rect16::Rect16(int16_t x, int16_t y, int16_t width, int16_t height)
{
    this->x = x;
    this->y = y;
    this->w = width;
    this->h = height;
}

bool Rect16::isNull() const
{
    if (w == 0 || h == 0) return true;
    return false;
}

bool Rect16::intersects(const Rect16& other) const
{
    if (isNull() || other.isNull()) return false;
    return left() < other.right() && right() > other.left() && top() < other.bottom() && bottom() > other.top();
}

Rect16 Rect16::intersected(const Rect16& other) const
{
    if (isNull() || other.isNull() || !intersects(other)) {
        return Rect16();
    }

    int16_t resX = max(x, other.x);
    int16_t resY = max(y, other.y);
    int16_t resW = min(right(), other.right()) - resX;
    int16_t resH = min(bottom(), other.bottom()) - resY;
    return Rect16(resX, resY, resW, resH);
}

void Rect16::setTopLeft(const Point16& topLeft)
{
    x = topLeft.x;
    y = topLeft.y;
}

void Rect16::setBottomRight(const Point16& bottomRight)
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

void Rect16::setRect(int16_t x, int16_t y, int16_t width, int16_t height)
{
    this->x = x;
    this->y = y;
    this->w = width;
    this->h = height;
}

void Rect16::setRect(const Rect& other)
{
    x = clamp16(other.left());
    y = clamp16(other.top());
    w = clamp16(other.width());
    h = clamp16(other.height());
}

void Rect16::setRect(const Rect16& other)
{
    x = other.x;
    y = other.y;
    w = other.w;
    h = other.h;
}

void Rect16::setCoords(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    x = min(x1, x2);
    y = min(y1, y2);
    w = abs(x2 - x1);
    h = abs(y2 - y1);
}

void Rect16::setCoords(const Point16& p1, const Point16& p2)
{
    *this = fromPoints(p1, p2);
}

void Rect16::setRight(int16_t right)
{
    w = abs(right - x);
    if (right < x) {
        x = right;
    }
}

void Rect16::setBottom(int16_t bottom)
{
    h = abs(bottom - y);
    if (bottom < y) {
        y = bottom;
    }
}

Rect16& Rect16::operator=(const Rect& other)
{
    x = clamp16(other.left());
    y = clamp16(other.top());
    w = clamp16(other.width());
    h = clamp16(other.height());
    return *this;
}

bool operator!=(const Rect16& r1, const Rect16& r2)
{
    if (r1.x != r2.x) return true;
    if (r1.y != r2.y) return true;
    if (r1.w != r2.w) return true;
    if (r1.h != r2.h) return true;
    return false;
}

bool operator==(const Rect16& r1, const Rect16& r2)
{
    if (r1.x != r2.x) return false;
    if (r1.y != r2.y) return false;
    if (r1.w != r2.w) return false;
    if (r1.h != r2.h) return false;
    return true;
}

} // namespace pplib::grafix
