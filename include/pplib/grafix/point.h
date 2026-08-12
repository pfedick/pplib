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

#ifndef PPLIB_INCLUDE_GRAFIX_POINT_H
#define PPLIB_INCLUDE_GRAFIX_POINT_H

#include <stdint.h>

namespace pplib
{
class String;
}

namespace pplib::grafix
{

#ifndef real_t
#ifdef PICO_BUILD
typedef float real_t;
#else
typedef double real_t;
#endif
#endif

class Point;
class Point16;
class Size;
class Size16;
class Rect;
class Rect16;

class PointF;

/** @class Point
 * @ingroup PPLGroupGrafik
 * @brief Repräsentiert einen Punkt in einem zweidimensionalen Koordinatensystem
 *
 * Diese Klasse repräsentiert einen Punkt in einem zweidimensionalen Koordinatensystem.
 * Zum Speichern der Koordinaten werden Integer (Ganzzahlen) verwendet.
 *
 * Ein Punkt wird durch seine X- und Y-Koordinate definiert, die mit den Funktionen
 * setX() und setY() gesetzt und mit x und y ausgelesen werden können.
 *
 * Ein Punkt kann auch als Vektor benutzt werden. Bei Addition und Subtraktion wird jede Komponente separat berechnet.
 * Ein Point-Object kann ausserdem mit einem Integer oder Double multipliziert oder geteilt werden.
 *
 */
class Point
{
public:
    int x = 0; /// @brief X-Koordinate des Punktes
    int y = 0; /// @brief Y-Koordinate des Punktes

    /** @brief Konstruktor mit Initialisierung auf (0/0)
     */
    Point() = default;

    /** @brief Konstruktor mit Initialisierung auf die angegebenen Koordinaten
     *
     * @param[in] x Die gewünschte X-Koordinate
     * @param[in] y Die gewünschte Y-Koordinate
     */
    Point(int x, int y)
        : x(x),
          y(y)
    {
    }

    /** @brief Konstruktor mit Initialisierung aus einem Point16
     *
     * @param[in] other Ein anderer Point16
     */
    Point(const Point16& other);

    /** @brief Konstruktor mit Initialisierung aus einem PointF
     *
     * @param[in] other Ein anderer PointF
     */
    Point(const PointF& other);

    /** @brief Liefert \c true zurück, wenn sowohl x als auch y 0 sind.
     *
     * Diese Funktion liefert \c true zurück, wenn der Punkt die Koordinaten (0/0) enthält.
     * \return
     * Liefert \c true zurück, wenn x und y 0 sind. Ist dies nicht der Fall, gibt die Funktion false zurück.
     *
     */
    inline bool isNull() const
    {
        return (x == 0 && y == 0);
    }

    /** @brief X-Koordinate setzen
     *
     * Mit dieser Funktion kann die X-Koordinate des Punktes gesetzt werden.
     * \param[in] x X-Koordinate
     */
    inline void setX(int x)
    {
        this->x = x;
    }

    /** @brief Y-Koordinate setzen
     *
     * Mit dieser Funktion kann die Y-Koordinate des Punktes gesetzt werden.
     * \param[in] y Y-Koordinate
     */
    inline void setY(int y)
    {
        this->y = y;
    }

    /** @brief X und Y-Koordinate gleichzeitig setzen
     *
     * Mit dieser Funktion kann die X- und Y-Koordinate des Punktes gleichzeitig gesetzt werden.
     * \param[in] x X-Koordinate
     * \param[in] y Y-Koordinate
     */
    inline void setPoint(int x, int y)
    {
        this->x = x;
        this->y = y;
    }

    /** @brief X und Y-Koordinate von einem anderen Point übernehmen
     *
     * Mit dieser Funktion werden die Koordinaten des Punktes \p other
     * übernommen.
     * \param[in] other Ein anderer Punkt
     */
    inline void setPoint(const Point& other)
    {
        x = other.x;
        y = other.y;
    }

    /** @brief X und Y-Koordinate von einem anderen Point16 übernehmen
     *
     * Mit dieser Funktion werden die Koordinaten des Punktes \p other
     * übernommen.
     * \param[in] other Ein anderer Punkt
     */
    void setPoint(const Point16& other);

    /** @brief X und Y-Koordinate von einem anderen PointF übernehmen
     *
     * Mit dieser Funktion werden die Koordinaten des Punktes \p other
     * übernommen.
     * \param[in] other Ein anderer Punkt
     */
    void setPoint(const PointF& other);

    /** @brief Prüft, ob der Punkt innerhalb eines Rechtecks liegt
     *
     * Mit dieser Funktion kann überprüft werden, ob der Punkt innerhalb des Rechtecks \p r liegt.
     * \param[in] r Ein Rechteck
     * \return Liefert \c true zurück, wenn der Punkt innerhalb des Rechtecks liegt. Ist dies nicht der Fall, gibt die Funktion false
     * zurück.
     */
    bool inside(const Rect& r) const;

    /** @brief Länge des Vektors
     *
     * Diese Funktion berechnet die Länge des Vektors, gerechnet vom Ursprung (0/0) bis zu den
     * aktuellen Koordinaten anhand des "Satz des Pythagoras", und liefert diese als \c real_t zurück.
     *
     * Die Formel lautet:
     * `length = sqrt{x^2+y^2}`
     *
     * Siehe auch die verwandte Funktion Distance, die den Abstand zwischen zwei Punkten
     * berechnet.
     *
     */
    real_t vectorLength() const;

    /** @brief Länge des Vektors in "Manhattan Distance"
     *
     * Als "Manhattan Länge" bezeichnet man den Abstand zwischen zwei Punkten, berechnet anhand
     * der Längen ihrer rechten Winkel. Auf einer Fläche mit \c p1=(x1/y1) und \c p2=(x2/y1)
     * ist die Länge:
     *
     * `length = abs(x1-x2) + abs(y1-y2)`
     *
     * Der Name wurde tatsächlich in Anlehnung an den New Yorker Stadtteil Manhattan
     * gewählt, aufgrund der schachbrettartig angeordneten Strassen.
     *
     * \returns Die Länge zwischen dem Ursprung des Koordinatensystems (0/0) und dem Punkt als Integer.
     *
     */
    int manhattanLength() const;

    Point& operator=(const Point16& other);

    Point& operator=(const PointF& other);

    /** @brief Multiplikation mit einem Faktor
     *
     * Mit diesem Operator wird der aktuelle X- und Y-Wert des Punktes einzeln mit
     * \p factor multipliziert.
     *
     * \param[in] factor Der Faktor, mit dem die Koordinate multipliziert werden soll
     * \return Referenz auf den Point
     */
    inline Point& operator*=(real_t factor)
    {
        x = (int)((real_t)x * factor);
        y = (int)((real_t)y * factor);
        return *this;
    }

    /** @brief Addition mit einem anderen Point
     *
     * Der aktuelle Wert des Punktes wird mit dem Parameter \p point addiert. Dabei werden
     * die einzelnen Werte X und Y jeweils miteinander addiert.
     *
     * \param[in] point Referenz auf einen anderen Point
     * \return Referenz auf den Point
     */
    inline Point& operator+=(const Point& point)
    {
        x += point.x;
        y += point.y;
        return *this;
    }

    /** @brief Subtraktion mit einem anderen Point
     *
     * Der aktuelle Wert des Punktes wird mit dem Parameter \p point subtrahiert. Dabei werden
     * die einzelnen Werte X und Y jeweils voneinander abgezogen.
     *
     * \param[in] point Referenz auf einen anderen Point
     * \return Referenz auf den Point
     */
    inline Point& operator-=(const Point& point)
    {
        x -= point.x;
        y -= point.y;
        return *this;
    }

    /** @brief Division mit einem Divisor
     *
     * Mit diesem Operator wird der aktuelle X- und Y-Wert des Punktes einzeln durch den
     * \p divisor geteilt.
     *
     * \param[in] divisor Der Divisor, durch den die aktuelle Koordinate geteilt werden soll
     * \return Referenz auf den Point
     */
    inline Point& operator/=(real_t divisor)
    {
        x = (int)((real_t)x / divisor);
        y = (int)((real_t)y / divisor);
        return *this;
    }

    bool operator<(const Point& other) const;
    bool operator<=(const Point& other) const;
    bool operator==(const Point& other) const;
    bool operator!=(const Point& other) const;
    bool operator>=(const Point& other) const;
    bool operator>(const Point& other) const;
};

const Point operator*(const Point& point, real_t factor);
const Point operator*(real_t factor, const Point& point);
const Point operator+(const Point& p1, const Point& p2);
const Point operator-(const Point& p1, const Point& p2);
const Point operator-(const Point& point);
const Point operator/(const Point& point, real_t divisor);

/** @brief Berechnet den Abstand zwischen zwei Punkten
 *
 * Diese Funktion berechnet die Länge des Vektors zwischen den beiden Punkten \p p1 und \p p2
 * anhand des "Satz des Pythagoras", und liefert diese als \c real_t zurück.
 *
 * Die Formel lautet:
 * `distance = sqrt{(p2.x()-p1.x())^2+(p2.y()-p1.y())^2}`
 *
 * \param[in] p1 Die Anfangskoordinate
 * \param[in] p2 Die Endkoordinate
 * \return
 * Der Abstand zwischen den beiden Punkten als \c real_t.
 */
real_t Distance(const Point& p1, const Point& p2);

class Point16
{
public:
    int16_t x = 0;
    int16_t y = 0;

    Point16() = default;

    /** @brief Konstruktor mit Initialisierung auf die angegebenen Koordinaten
     *
     * Durch Verwendung dieses Konstruktors wird der Punkt mit den Koordinaten (\p x /\p y ) initialisiert.
     * \param[in] x Die gewünschte X-Koordinate
     * \param[in] y Die gewünschte Y-Koordinate
     */
    Point16(int16_t x, int16_t y) noexcept
        : x(x),
          y(y)
    {
    }
    Point16(const Point& other) noexcept;

    /** @brief Liefert \c true zurück, wenn sowohl x als auch y 0 sind.
     *
     * Diese Funktion liefert \c true zurück, wenn der Punkt die Koordinaten (0/0) enthält.
     * \return
     * Liefert \c true zurück, wenn x und y 0 sind. Ist dies nicht der Fall, gibt die Funktion false zurück.
     *
     */
    inline bool isNull() const
    {
        return (x == 0 && y == 0);
    }

    bool inside(const Rect16& r) const;
    float vectorLength() const;
    int manhattanLength() const;

    /** @brief X-Koordinate setzen
     *
     * \param[in] x X-Koordinate
     */
    inline void setX(int16_t x)
    {
        this->x = x;
    }

    /** @brief Y-Koordinate setzen
     *
     * \param[in] y Y-Koordinate
     */
    inline void setY(int16_t y)
    {
        this->y = y;
    }

    /** @brief X und Y-Koordinate gleichzeitig setzen
     *
     * Mit dieser Funktion kann die X- und Y-Koordinate des Punktes gleichzeitig gesetzt werden.
     * \param[in] x X-Koordinate
     * \param[in] y Y-Koordinate
     */
    inline void setPoint(int16_t x, int16_t y)
    {
        this->x = x;
        this->y = y;
    }

    inline void setPoint(const Point16& other) noexcept
    {
        x = other.x;
        y = other.y;
    }

    void setPoint(const Point& other) noexcept;
    Point16& operator=(const Point& other) noexcept;
    Point16& operator*=(float factor);
    Point16& operator+=(const Point16& point);
    Point16& operator-=(const Point16& point);
    Point16& operator/=(float divisor);

    bool operator<(const Point16& other) const;
    bool operator<=(const Point16& other) const;
    bool operator==(const Point16& other) const;
    bool operator!=(const Point16& other) const;
    bool operator>=(const Point16& other) const;
    bool operator>(const Point16& other) const;
};

const Point16 operator*(const Point16& point, float factor);
const Point16 operator*(float factor, const Point16& point);
const Point16 operator+(const Point16& p1, const Point16& p2);
const Point16 operator-(const Point16& p1, const Point16& p2);
const Point16 operator-(const Point16& point);
const Point16 operator/(const Point16& point, float divisor);
float Distance(const Point16& p1, const Point16& p2);

class PointF
{
public:
    float x, y;

    PointF();
    PointF(float x, float y);
    PointF(const Point& other);
    PointF(const PointF& other);
    bool isNull() const;
    bool inside(const Rect& r) const;
    real_t vectorLength() const;
    float manhattanLength() const;
    void setPoint(float x, float y);
    void setPoint(const Point& other);
    void setPoint(const PointF& other);
    PointF& operator=(const Point& other);
    PointF& operator=(const PointF& other);
    PointF& operator*=(real_t factor);
    PointF& operator+=(const PointF& point);
    PointF& operator-=(const PointF& point);
    PointF& operator/=(real_t divisor);

    bool operator<(const PointF& other) const;
    bool operator<=(const PointF& other) const;
    bool operator==(const PointF& other) const;
    bool operator!=(const PointF& other) const;
    bool operator>=(const PointF& other) const;
    bool operator>(const PointF& other) const;
};
const PointF operator*(const PointF& point, real_t factor);
const PointF operator*(real_t factor, const PointF& point);
const PointF operator+(const PointF& p1, const PointF& p2);
const PointF operator-(const PointF& p1, const PointF& p2);
const PointF operator-(const PointF& point);
const PointF operator/(const PointF& point, real_t divisor);
real_t Distance(const PointF& p1, const PointF& p2);

class Point3D
{
    // Freunde der Klasse
    friend bool operator==(const Point3D& p1, const Point3D& p2);
    friend bool operator!=(const Point3D& p1, const Point3D& p2);
    friend const Point3D operator*(const Point3D& point, real_t factor);
    friend const Point3D operator*(real_t factor, const Point3D& point);
    friend const Point3D operator+(const Point3D& p1, const Point3D& p2);
    friend const Point3D operator-(const Point3D& p1, const Point3D& p2);
    friend const Point3D operator-(const Point3D& point);
    friend const Point3D operator/(const Point3D& point, real_t divisor);

private:
public:
    int x, y, z;
    Point3D();
    Point3D(int x, int y, int z);
    Point3D(const Point3D& other);
    bool isNull() const;
    void setX(int x);
    void setY(int y);
    void setZ(int z);
    void setPoint(int x, int y, int z);
    void setPoint(const Point3D& other);
    Point3D& operator*=(real_t factor);
    Point3D& operator+=(const Point3D& point);
    Point3D& operator-=(const Point3D& point);
    Point3D& operator/=(real_t divisor);
};

bool operator!=(const Point3D& p1, const Point3D& p2);
bool operator==(const Point3D& p1, const Point3D& p2);
const Point3D operator*(const Point3D& point, real_t factor);
const Point3D operator*(real_t factor, const Point3D& point);
const Point3D operator+(const Point3D& p1, const Point3D& p2);
const Point3D operator-(const Point3D& p1, const Point3D& p2);
const Point3D operator-(const Point3D& point);
const Point3D operator/(const Point3D& point, real_t divisor);

} // namespace pplib::grafix
#endif // PPLIB_INCLUDE_GRAFIX_POINT_H