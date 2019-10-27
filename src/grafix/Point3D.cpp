/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
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

#include "prolog_ppl7.h"
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
//#ifdef HAVE_MATH_H
#include <math.h>
//#endif
#include "ppl7-grafix.h"

namespace ppl7 {
namespace grafix {

/*!\class Point3D
 * \ingroup PPLGroupGrafik
 * \brief Repräsentiert einen Punkt in einem dreidimensionalen Koordinatensystem
 *
 * \desc
 * Diese Klasse repräsentiert einen Punkt in einem dreidimensionalen Koordinatensystem.
 * Zum Speichern der Koordinaten werden Integer (Ganzzahlen) verwendet.
 * \par
 * Ein Punkt wird durch seine X-, Y- und Z-Koordinate definiert, die mit den Funktionen
 * setX(), setY(), setZ() oder setPoint(). Die Koordinaten können über Point3D.x, Point3D.y
 * und Point3D.z aber auch direkt angesprochen werden.
 *
 * \par
 * Beispiele:
 * \code
 Point3D p1(4,12,6);
 Point3D p2;
 Point3D p3;
 p2.setX(4);
 p2.setY(12);
 p2.setZ(6);
 p3.x=4;
 p3.y=12;
 p3.z=6;
 * \endcode
 * \par
 * Ein Punkt kann auch als Vektor benutzt werden. Bei Addition und Subtraktion wird jede Komponente separat berechnet.
 * Ein Point-Object kann ausserdem mit einem Integer oder Double multipliziert oder geteilt werden.
 *
 * \see
 * Die Klasse Point bietet die gleiche Funktionalität, repräsentiert jedoch einen Punkt
 * in einem zweidimensionalen Koordinatensystem.
 *
 */


/*!\var Point3D::x
 * \brief Speichert die X-Koordinate des Punktes
 */

/*!\var Point3D::y
 * \brief Speichert die Y-Koordinate des Punktes
 */

/*!\var Point3D::z
 * \brief Speichert die Z-Koordinate des Punktes
 */

/*!\brief Konstruktor mit Initialisierung auf (0/0/0)
 *
 * \desc
 * Durch Verwendung dieses Konstruktors wird der Punkt mit den Koordinaten (0/0/0) initialisiert.
 */
Point3D::Point3D()
{
	x=0;
	y=0;
	z=0;
}

/*!\brief Konstruktor mit Initialisierung auf die angegebenen Koordinaten
 *
 * \desc
 * Durch Verwendung dieses Konstruktors wird der Punkt mit den Koordinaten (\p x /\p y /\p z ) initialisiert.
 * \param[in] x Die gewünschte X-Koordinate
 * \param[in] y Die gewünschte Y-Koordinate
 * \param[in] z Die gewünschte Z-Koordinate
 */
Point3D::Point3D(int x, int y, int z)
{
	this->x=x;
	this->y=y;
	this->z=z;
}

/*!\brief Konstruktor mit Kopierfunktion
 *
 * \desc
 * Mit diesem Konstruktor werden die Daten eines anderen Point3D übernommen.
 *
 * @param other Referenz auf einen anderen Point3D
 */
Point3D::Point3D(const Point3D &other)
{
	x=other.x;
	y=other.y;
	z=other.z;
}

/*!\brief Liefert \c true zurück, wenn x, y und z 0 sind.
 *
 * \desc
 * Diese Funktion liefert \c true zurück, wenn der Punkt die Koordinaten (0/0/0) enthält.
 * \return
 * Liefert \c true zurück, wenn x, y und z 0 sind. Ist dies nicht der Fall, gibt die Funktion false zurück.
 *
 */
bool Point3D::isNull() const
{
	if (x==0 && y==0 && z==0) return true;
	return false;
}


/*!\brief X-Koordinate setzen
 *
 * \desc
 * Mit dieser Funktion kann die X-Koordinate des Punktes gesetzt werden.
 * \param[in] x X-Koordinate
 */
void Point3D::setX(int x)
{
	this->x=x;
}

/*!\brief Y-Koordinate setzen
 *
 * \desc
 * Mit dieser Funktion kann die Y-Koordinate des Punktes gesetzt werden.
 * \param[in] y Y-Koordinate
 */
void Point3D::setY(int y)
{
	this->y=y;
}

/*!\brief Z-Koordinate setzen
 *
 * \desc
 * Mit dieser Funktion kann die Z-Koordinate des Punktes gesetzt werden.
 * \param[in] z Z-Koordinate
 */
void Point3D::setZ(int z)
{
	this->z=z;
}


/*!\brief X-, Y- und Z-Koordinate gleichzeitig setzen
 *
 * \desc
 * Mit dieser Funktion kann die X-, Y- und Z-Koordinate des Punktes gleichzeitig gesetzt werden.
 * \param[in] x X-Koordinate
 * \param[in] y Y-Koordinate
 * \param[in] z Z-Koordinate
 */
void Point3D::setPoint(int x, int y, int z)
{
	this->x=x;
	this->y=y;
	this->z=z;
}

/*!\brief Anderen Point3D kopieren
 *
 * \desc
 * Mit dieser Funktion werden die Daten des Point3D \p other übernommen.
 *
 * @param other Referenz auf einen anderen Point3D
 */
void Point3D::setPoint(const Point3D &other)
{
	x=other.x;
	y=other.y;
	z=other.z;
}

/*!\brief Multiplikation mit einem Faktor
 *
 * \desc
 * Mit diesem Operator wird der aktuelle X-, Y- und Z-Wert des Punktes einzeln mit
 * \p factor multipliziert.
 *
 * \param[in] factor Der Faktor, mit dem die Koordinate multipliziert werden soll
 * \return Referenz auf den Point3D
 */
Point3D &Point3D::operator*= (double factor)
{
	x=(int)(x*factor);
	y=(int)(y*factor);
	z=(int)(z*factor);
	return *this;
}

/*!\brief Addition mit einem anderen Point3D
 *
 * \desc
 * Der aktuelle Wert des Punktes wird mit dem Parameter \p point addiert. Dabei werden
 * die einzelnen Werte X, Y und Z jeweils miteinander addiert.
 *
 * \param[in] point Referenz auf einen anderen Point3D
 * \return Referenz auf den Point3D
 */
Point3D &Point3D::operator+= (const Point3D &point)
{
	x+=point.x;
	y+=point.y;
	z+=point.z;
	return *this;
}

/*!\brief Subtraktion mit einem anderen Point3D
 *
 * \desc
 * Der aktuelle Wert des Punktes wird mit dem Parameter \p point subtrahiert. Dabei werden
 * die einzelnen Werte X, Y und Z jeweils voneinander abgezogen.
 *
 * \param[in] point Referenz auf einen anderen Point3D
 * \return Referenz auf den Point3D
 */
Point3D &Point3D::operator-= (const Point3D &point)
{
	x-=point.x;
	y-=point.y;
	z-=point.z;
	return *this;
}

/*!\brief Division mit einem Divisor
 *
 * \desc
 * Mit diesem Operator wird der aktuelle X-, Y- und Z-Wert des Punktes einzeln durch den
 * \p divisor geteilt.
 *
 * \param[in] divisor Der Divisor, durch den die aktuelle Koordinate geteilt werden soll
 * \return Referenz auf den Point3D
 */
Point3D &Point3D::operator/= (double divisor)
{
	x=(int)(x/divisor);
	y=(int)(y/divisor);
	z=(int)(z/divisor);
	return *this;
}

bool operator!= (const Point3D &p1, const Point3D &p2)
{
	if (p1.x!=p2.x) return true;
	if (p1.y!=p2.y) return true;
	if (p1.z!=p2.z) return true;
	return false;
}

bool operator== (const Point3D &p1, const Point3D &p2)
{
	if (p1.x!=p2.x) return false;
	if (p1.y!=p2.y) return false;
	if (p1.z!=p2.z) return false;
	return true;
}

const Point3D operator* (const Point3D &point, double factor)
{
	return Point3D ((int)(point.x*factor),(int)(point.y*factor), (int)(point.z*factor));
}

const Point3D operator* (double factor, const Point3D &point)
{
	return Point3D ((int)(point.x*factor),(int)(point.y*factor), (int)(point.z*factor));
}

const Point3D operator+ (const Point3D &p1, const Point3D &p2)
{
	return Point3D (p1.x+p2.x,p1.y+p2.y,p1.z+p2.z);
}

const Point3D operator- (const Point3D &p1, const Point3D &p2)
{
	return Point3D (p1.x-p2.x,p1.y-p2.y,p1.z-p2.z);
}

const Point3D operator- (const Point3D &point)
{
	return Point3D (0-point.x,0-point.y,0-point.z);
}

const Point3D operator/ (const Point3D &point, double divisor)
{
	return Point3D ((int)(point.x/divisor),(int)(point.y/divisor),(int)(point.z/divisor));
}

} // EOF namespace grafix
} // EOF namespace ppl7
