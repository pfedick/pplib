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

static inline int16_t clamp16(int value)
{
    if (value < -32768) return -32768;
    if (value > 32767) return 32767;
    return value;
}

Point16::Point16(const Point& other) noexcept
{
    x = clamp16(other.x);
    y = clamp16(other.y);
}

void Point16::setPoint(const Point& other)
{
    x = clamp16(other.x);
    y = clamp16(other.y);
}

/*!\brief Länge des Vektors
 *
 * \desc
 * Diese Funktion berechnet die Länge des Vektors, gerechnet vom Ursprung (0/0) bis zu den
 * aktuellen Koordinaten anhand des "Satz des Pythagoras", und liefert diese als \c float zurück.
 * \par
 * Die Formel lautet:
 * \f$length = \sqrt{x^2+y^2}\f$
 *
 * \see
 * Siehe auch die verwandte Funktion Distance, die den Abstand zwischen zwei Punkten
 * berechnet.
 *
 */
float Point16::vectorLength() const
{
    return std::sqrt(static_cast<float>(x) * x + static_cast<float>(y) * y);
}

/*!\brief Berechnet den Abstand zwischen zwei Punkten
 *
 * \desc
 * Diese Funktion berechnet die Länge des Vektors zwischen den beiden Punkten \p p1 und \p p2
 * anhand des "Satz des Pythagoras", und liefert diese als \c float zurück.
 * \par
 * Die Formel lautet:
 * \f$distance = \sqrt{(p2.x()-p1.x())^2+(p2.y()-p1.y())^2}\f$
 *
 * \param[in] p1 Die Anfangskoordinate
 * \param[in] p2 Die Endkoordinate
 * \return
 * Der Abstand zwischen den beiden Punkten als \c float.
 *
 * \relates Point
 */
float Distance(const Point16& p1, const Point16& p2)
{
    float dx = static_cast<float>(p2.x) - p1.x;
    float dy = static_cast<float>(p2.y) - p1.y;
    return std::sqrt(dx * dx + dy * dy);
}

/*!\brief Länge des Vektors in "Manhattan Distance"
 *
 * \desc
 * Als "Manhattan Länge" bezeichnet man den Abstand zwischen zwei Punkten, berechnet anhand
 * der Längen ihrer rechten Winkel. Auf einer Fläche mit \c p1=(x1/y1) und \c p2=(x2/y1)
 * ist die Länge:
 * \par
 * \f$length = abs(x1-x2) + abs(y1-y2)\f$
 * \par
 * Der Name wurde tatsächlich in Anlehnung an den New Yorker Stadtteil Manhattan
 * gewählt, aufgrund der schachbrettartig angeordneten Strassen.
 *
 * \returns Die Länge zwischen dem Ursprung des Koordinatensystems (0/0) und dem Punkt als Integer.
 *
 */
int Point16::manhattanLength() const
{
    return abs(x) + abs(y);
}

bool Point16::inside(const Rect16& r) const
{
    if (r.left() <= x && x <= r.lastX()) {
        if (r.top() <= y && y <= r.lastY()) {
            return true;
        }
    }
    return false;
}

Point16& Point16::operator=(const Point& other) noexcept
{
    x = clamp16(other.x);
    y = clamp16(other.y);
    return *this;
}

/*!\brief Multiplikation mit einem Faktor
 *
 * \desc
 * Mit diesem Operator wird der aktuelle X- und Y-Wert des Punktes einzeln mit
 * \p factor multipliziert.
 *
 * \param[in] factor Der Faktor, mit dem die Koordinate multipliziert werden soll
 * \return Referenz auf den Point
 */
Point16& Point16::operator*=(float factor)
{
    x = (int)((float)x * factor);
    y = (int)((float)y * factor);
    return *this;
}

/*!\brief Addition mit einem anderen Point
 *
 * \desc
 * Der aktuelle Wert des Punktes wird mit dem Parameter \p point addiert. Dabei werden
 * die einzelnen Werte X und Y jeweils miteinander addiert.
 *
 * \param[in] point Referenz auf einen anderen Point
 * \return Referenz auf den Point
 */
Point16& Point16::operator+=(const Point16& point)
{
    x += point.x;
    y += point.y;
    return *this;
}

/*!\brief Subtraktion mit einem anderen Point
 *
 * \desc
 * Der aktuelle Wert des Punktes wird mit dem Parameter \p point subtrahiert. Dabei werden
 * die einzelnen Werte X und Y jeweils voneinander abgezogen.
 *
 * \param[in] point Referenz auf einen anderen Point
 * \return Referenz auf den Point
 */
Point16& Point16::operator-=(const Point16& point)
{
    x -= point.x;
    y -= point.y;
    return *this;
}

/*!\brief Division mit einem Divisor
 *
 * \desc
 * Mit diesem Operator wird der aktuelle X- und Y-Wert des Punktes einzeln durch den
 * \p divisor geteilt.
 *
 * \param[in] divisor Der Divisor, durch den die aktuelle Koordinate geteilt werden soll
 * \return Referenz auf den Point
 */
Point16& Point16::operator/=(float divisor)
{
    assert(divisor != 0.0f && "Division by zero in Point16 operator /");
    x = static_cast<uint16_t>(x / divisor);
    y = static_cast<uint16_t>(y / divisor);
    return *this;
}

const Point16 operator*(const Point16& point, float factor)
{
    return Point((int)(point.x * factor), (int)(point.y * factor));
}

const Point16 operator*(float factor, const Point16& point)
{
    return Point16((int)(point.x * factor), (int)(point.y * factor));
}

const Point16 operator+(const Point16& p1, const Point16& p2)
{
    return Point16(p1.x + p2.x, p1.y + p2.y);
}

const Point16 operator-(const Point16& p1, const Point16& p2)
{
    return Point16(p1.x - p2.x, p1.y - p2.y);
}

const Point16 operator-(const Point16& point)
{
    return Point16(0 - point.x, 0 - point.y);
}

const Point16 operator/(const Point16& point, float divisor)
{
    return Point16((int)(point.x / divisor), (int)(point.y / divisor));
}

static int cmp(const Point16& p1, const Point16& p2)
{
    if (p1.y < p2.y) return -1;
    if (p1.y > p2.y) return 1;
    if (p1.x < p2.x) return -1;
    if (p1.x > p2.x) return 1;
    return 0;
}

bool Point16::operator<(const Point16& other) const
{
    int c = cmp(*this, other);
    if (c < 0) return true;
    return false;
}

bool Point16::operator<=(const Point16& other) const
{
    int c = cmp(*this, other);
    if (c <= 0) return true;
    return false;
}

bool Point16::operator==(const Point16& other) const
{
    int c = cmp(*this, other);
    if (c == 0) return true;
    return false;
}

bool Point16::operator!=(const Point16& other) const
{
    int c = cmp(*this, other);
    if (c != 0) return true;
    return false;
}

bool Point16::operator>=(const Point16& other) const
{
    int c = cmp(*this, other);
    if (c >= 0) return true;
    return false;
}

bool Point16::operator>(const Point16& other) const
{
    int c = cmp(*this, other);
    if (c > 0) return true;
    return false;
}
}; // namespace pplib::grafix
