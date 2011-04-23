/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.1 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: Point3D.cpp,v 1.1 2010/02/12 19:43:48 pafe Exp $
 *
 *******************************************************************************
 * Copyright (c) 2010, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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

#include "prolog.h"
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
#include "ppl6-grafix.h"

namespace ppl6 {
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
 * setX(), setY() und setZ() gesetzt und mit x(), y() und z() ausgelesen werden können.
 * Dir Funktionen rx(), ry() und rz() liefern Referenzen auf die Koordinaten und erlauben es so die Werte direkt
 * zu manipulieren.
 * \par
 * Beispiele:
 * \code
 Point3D p1(4,12,6);
 Point3D p2;
 Point3D p3;
 p2.setX(4);
 p2.setY(12);
 p2.setZ(6);
 p3.rx()=4;
 p3.ry()=12;
 p3.rz()=6;
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


/*!\var Point3D::px
 * \brief Speichert die X-Koordinate des Punktes
 */

/*!\var Point3D::py
 * \brief Speichert die Y-Koordinate des Punktes
 */

/*!\var Point3D::pz
 * \brief Speichert die Z-Koordinate des Punktes
 */

/*!\brief Konstruktor mit Initialisierung auf (0/0/0)
 *
 * \desc
 * Durch Verwendung dieses Konstruktors wird der Punkt mit den Koordinaten (0/0/0) initialisiert.
 */
Point3D::Point3D()
{
	px=0;
	py=0;
	pz=0;
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
	px=x;
	py=y;
	pz=z;
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
	if (px==0 && py==0 && pz==0) return true;
	return false;
}

/*!\brief Gibt die X-Koordinate zurück
 *
 * \desc
 * Diese Funktion liefert die aktuelle X-Koordinate des Punktes zurück.
 * \return
 * Wert der X-Koordinaten.
 */
int Point3D::x() const
{
	return px;
}

/*!\brief Gibt die Y-Koordinate zurück
 *
 * \desc
 * Diese Funktion liefert die aktuelle Y-Koordinate des Punktes zurück.
 * \return
 * Wert der Y-Koordinaten.
 */
int Point3D::y() const
{
	return py;
}

/*!\brief Gibt die Z-Koordinate zurück
 *
 * \desc
 * Diese Funktion liefert die aktuelle Z-Koordinate des Punktes zurück.
 * \return
 * Wert der Y-Koordinaten.
 */
int Point3D::z() const
{
	return pz;
}

/*!\brief Gibt eine Referenz auf die X-Koordinate zurück
 *
 * \desc
 * Diese Funktion liefert eine Referenz auf die X-Koordinate des Punktes, so dass diese auch ausserhalb der Klasse verändert
 * werden kann.
 * \return
 * Referenz auf die X-Koordinate.
 * \example
 * \code
 * ppl6::Point3D p;
 * p.rx()++;
 * \endcode
 */
int &Point3D::rx()
{
	return px;
}

/*!\brief Gibt eine Referenz auf die Y-Koordinate zurück
 *
 * \desc
 * Diese Funktion liefert eine Referenz auf die Y-Koordinate des Punktes, so dass diese auch ausserhalb der Klasse verändert
 * werden kann.
 * \return
 * Referenz auf die Y-Koordinate.
 * \example
 * \code
 * ppl6::Point3D p;
 * p.ry()++;
 * \endcode
 */
int &Point3D::ry()
{
	return py;
}

/*!\brief Gibt eine Referenz auf die Z-Koordinate zurück
 *
 * \desc
 * Diese Funktion liefert eine Referenz auf die Z-Koordinate des Punktes, so dass diese auch ausserhalb der Klasse verändert
 * werden kann.
 * \return
 * Referenz auf die Z-Koordinate.
 * \example
 * \code
 * ppl6::Point3D p;
 * p.rz()++;
 * \endcode
 */
int &Point3D::rz()
{
	return pz;
}

/*!\brief X-Koordinate setzen
 *
 * \desc
 * Mit dieser Funktion kann die X-Koordinate des Punktes gesetzt werden.
 * \param[in] x X-Koordinate
 */
void Point3D::setX(int x)
{
	px=x;
}

/*!\brief Y-Koordinate setzen
 *
 * \desc
 * Mit dieser Funktion kann die Y-Koordinate des Punktes gesetzt werden.
 * \param[in] y Y-Koordinate
 */
void Point3D::setY(int y)
{
	py=y;
}

/*!\brief Z-Koordinate setzen
 *
 * \desc
 * Mit dieser Funktion kann die Z-Koordinate des Punktes gesetzt werden.
 * \param[in] z Z-Koordinate
 */
void Point3D::setZ(int z)
{
	pz=z;
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
	px=x;
	py=y;
	pz=z;
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
	px=(int)(px*factor);
	py=(int)(py*factor);
	pz=(int)(pz*factor);
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
	px+=point.px;
	py+=point.py;
	pz+=point.pz;
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
	px-=point.px;
	py-=point.py;
	pz-=point.pz;
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
	px=(int)(px/divisor);
	py=(int)(py/divisor);
	pz=(int)(pz/divisor);
	return *this;
}

bool operator!= (const Point3D &p1, const Point3D &p2)
{
	if (p1.px!=p2.px) return true;
	if (p1.py!=p2.py) return true;
	if (p1.pz!=p2.pz) return true;
	return false;
}

bool operator== (const Point3D &p1, const Point3D &p2)
{
	if (p1.px!=p2.px) return false;
	if (p1.py!=p2.py) return false;
	if (p1.pz!=p2.pz) return false;
	return true;
}

const Point3D operator* (const Point3D &point, double factor)
{
	return Point3D ((int)(point.px*factor),(int)(point.py*factor), (int)(point.pz*factor));
}

const Point3D operator* (double factor, const Point3D &point)
{
	return Point3D ((int)(point.px*factor),(int)(point.py*factor), (int)(point.pz*factor));
}

const Point3D operator+ (const Point3D &p1, const Point3D &p2)
{
	return Point3D (p1.px+p2.px,p1.py+p2.py,p1.pz+p2.pz);
}

const Point3D operator- (const Point3D &p1, const Point3D &p2)
{
	return Point3D (p1.px-p2.px,p1.py-p2.py,p1.pz-p2.pz);
}

const Point3D operator- (const Point3D &point)
{
	return Point3D (0-point.px,0-point.py,0-point.pz);
}

const Point3D operator/ (const Point3D &point, double divisor)
{
	return Point3D ((int)(point.px/divisor),(int)(point.py/divisor),(int)(point.pz/divisor));
}

} // EOF namespace grafix
} // EOF namespace ppl6
