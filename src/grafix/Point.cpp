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
#include <cmath>
#include <assert.h>
#include <pplib/grafix/point.h>
#include <pplib/grafix/rect.h>

namespace pplib::grafix
{

void Point::setPoint(const Point16& other)
{
    x = other.x;
    y = other.y;
}

void Point::setPoint(const PointF& other)
{
    x = (int)other.x;
    y = (int)other.y;
}

Point::Point(const Point16& other)
    : x(other.x),
      y(other.y)
{
}
Point::Point(const PointF& other)
    : x((int)other.x),
      y((int)other.y)
{
}

Point& Point::operator=(const Point16& other)
{
    x = other.x;
    y = other.y;
    return *this;
}

Point& Point::operator=(const PointF& other)
{
    x = (int)other.x;
    y = (int)other.y;
    return *this;
}

bool Point::inside(const Rect& r) const
{
    if (r.left() <= x && x <= r.lastX()) {
        if (r.top() <= y && y <= r.lastY()) {
            return true;
        }
    }
    return false;
}

real_t Point::vectorLength() const
{

    return std::sqrt((real_t)(((real_t)x * (real_t)x) + ((real_t)y * (real_t)y)));
}

double Distance(const Point& p1, const Point& p2)
{
    double a = abs(p2.x - p1.x);
    double b = abs(p2.y - p1.y);
    return sqrt((a * a) + (b * b));
}

int Point::manhattanLength() const
{
    return abs(x) + abs(y);
}

const Point operator*(const Point& point, real_t factor)
{
    return Point((int)(point.x * factor), (int)(point.y * factor));
}

const Point operator*(real_t factor, const Point& point)
{
    return Point((int)(point.x * factor), (int)(point.y * factor));
}

const Point operator+(const Point& p1, const Point& p2)
{
    return Point(p1.x + p2.x, p1.y + p2.y);
}

const Point operator-(const Point& p1, const Point& p2)
{
    return Point(p1.x - p2.x, p1.y - p2.y);
}

const Point operator-(const Point& point)
{
    return Point(0 - point.x, 0 - point.y);
}

const Point operator/(const Point& point, real_t divisor)
{
    return Point((int)(point.x / divisor), (int)(point.y / divisor));
}

static int cmp(const Point& p1, const Point& p2)
{
    if (p1.y < p2.y) return -1;
    if (p1.y > p2.y) return 1;
    if (p1.x < p2.x) return -1;
    if (p1.x > p2.x) return 1;
    return 0;
}

bool Point::operator<(const Point& other) const
{
    int c = cmp(*this, other);
    if (c < 0) return true;
    return false;
}

bool Point::operator<=(const Point& other) const
{
    int c = cmp(*this, other);
    if (c <= 0) return true;
    return false;
}

bool Point::operator==(const Point& other) const
{
    int c = cmp(*this, other);
    if (c == 0) return true;
    return false;
}

bool Point::operator!=(const Point& other) const
{
    int c = cmp(*this, other);
    if (c != 0) return true;
    return false;
}

bool Point::operator>=(const Point& other) const
{
    int c = cmp(*this, other);
    if (c >= 0) return true;
    return false;
}

bool Point::operator>(const Point& other) const
{
    int c = cmp(*this, other);
    if (c > 0) return true;
    return false;
}

} // namespace pplib::grafix