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
#include "ppl7-grafix.h"

namespace ppl7 {
namespace grafix {

/*!\class Point
 * \ingroup PPLGroupGrafik
 * \brief Repräsentiert einen Punkt in einem zweidimensionalen Koordinatensystem
 *
 * \desc
 * Diese Klasse repräsentiert einen Punkt in einem zweidimensionalen Koordinatensystem.
 * Zum Speichern der Koordinaten werden Integer (Ganzzahlen) verwendet.
 * \image html Class_Point.png
 * \image latex Class_Point.png
 * \par
 * Ein Punkt wird durch seine X- und Y-Koordinate definiert, die mit den Funktionen
 * setX() und setY() gesetzt und mit x() und y() ausgelesen werden können.
 * Dir Funktionen rx() und ry() liefern Referenzen auf die Koordinaten und erlauben es so die Werte direkt
 * zu manipulieren.
 * \par
 * Beispiele:
 * \code
 Point p1(4,12);
 Point p2;
 Point p3;
 p2.setX(4);
 p2.setY(12);
 p3.rx()=4;
 p3.ry()=12;
 * \endcode
 * \par
 * Ein Punkt kann auch als Vektor benutzt werden. Bei Addition und Subtraktion wird jede Komponente separat berechnet.
 * Ein Point-Object kann ausserdem mit einem Integer oder Double multipliziert oder geteilt werden.
 *
 * \see
 * Die Klasse Point3D bietet die gleiche Funktionalität, repräsentiert jedoch einen Punkt
 * in einem dreidimensionalen Koordinatensystem.
 */


/*!\var Point::x
 * \brief Speichert die X-Koordinate des Punktes
 */

/*!\var Point::y
 * \brief Speichert die Y-Koordinate des Punktes
 */

/*!\brief Konstruktor mit Initialisierung auf (0/0)
 *
 * \desc
 * Durch Verwendung dieses Konstruktors wird der Punkt mit den Koordinaten (0/0) initialisiert.
 */
Point::Point()
{
	x=0;
	y=0;
}

/*!\brief Konstruktor mit Initialisierung auf die angegebenen Koordinaten
 *
 * \desc
 * Durch Verwendung dieses Konstruktors wird der Punkt mit den Koordinaten (\p x /\p y ) initialisiert.
 * \param[in] x Die gewünschte X-Koordinate
 * \param[in] y Die gewünschte Y-Koordinate
 */
Point::Point(int x, int y)
{
	this->x=x;
	this->y=y;
}

/*!\brief Konstruktor mit Initialisierung auf die im String angegebenen Koordinaten
 *
 * \desc
 * Durch Verwendung dieses Konstruktors wird der Punkt mit den im String
 * \p s vorhandenen Koordinaten initialisiert.
 * \param[in] s String mit den Komma-getrennten Koordinaten
 */
Point::Point(const String &s)
{
	String c=s;
	c.replace(";",",");
	Array a;
	a.explode(c,",",0,true);
	x=a[0].toInt();
	y=a[1].toInt();
}

/*!\brief Liefert \c true zurück, wenn sowohl x als auch y 0 sind.
 *
 * \desc
 * Diese Funktion liefert \c true zurück, wenn der Punkt die Koordinaten (0/0) enthält.
 * \return
 * Liefert \c true zurück, wenn x und y 0 sind. Ist dies nicht der Fall, gibt die Funktion false zurück.
 *
 */
bool Point::isNull() const
{
	if (x==0 && y==0) return true;
	return false;
}

/*!\brief X-Koordinate setzen
 *
 * \desc
 * Mit dieser Funktion kann die X-Koordinate des Punktes gesetzt werden.
 * \param[in] x X-Koordinate
 */
void Point::setX(int x)
{
	this->x=x;
}

/*!\brief Y-Koordinate setzen
 *
 * \desc
 * Mit dieser Funktion kann die Y-Koordinate des Punktes gesetzt werden.
 * \param[in] y Y-Koordinate
 */
void Point::setY(int y)
{
	this->y=y;
}

/*!\brief X und Y-Koordinate gleichzeitig setzen
 *
 * \desc
 * Mit dieser Funktion kann die X- und Y-Koordinate des Punktes gleichzeitig gesetzt werden.
 * \param[in] x X-Koordinate
 * \param[in] y Y-Koordinate
 */
void Point::setPoint(int x, int y)
{
	this->x=x;
	this->y=y;
}

/*!\brief X und Y-Koordinate von einem anderen Point übernehmen
 *
 * \desc
 * Mit dieser Funktion werden die Koordinaten des Punktes \p other
 * übernommen.
 * \param[in] other Ein anderer Punkt
 */
void Point::setPoint(const Point &other)
{
	x=other.x;
	y=other.y;
}


/*!\brief Länge des Vektors
 *
 * \desc
 * Diese Funktion berechnet die Länge des Vektors, gerechnet vom Ursprung (0/0) bis zu den
 * aktuellen Koordinaten anhand des "Satz des Pythagoras", und liefert diese als \c double zurück.
 * \par
 * Die Formel lautet:
 * \f$length = \sqrt{x^2+y^2}\f$
 *
 * \see
 * Siehe auch die verwandte Funktion Distance, die den Abstand zwischen zwei Punkten
 * berechnet.
 *
 */
double Point::vectorLength() const
{
	return sqrt((double)((x*x)+(y*y)));
}

/*!\brief Berechnet den Abstand zwischen zwei Punkten
 *
 * \desc
 * Diese Funktion berechnet die Länge des Vektors zwischen den beiden Punkten \p p1 und \p p2
 * anhand des "Satz des Pythagoras", und liefert diese als \c double zurück.
 * \par
 * Die Formel lautet:
 * \f$distance = \sqrt{(p2.x()-p1.x())^2+(p2.y()-p1.y())^2}\f$
 *
 * \param[in] p1 Die Anfangskoordinate
 * \param[in] p2 Die Endkoordinate
 * \return
 * Der Abstand zwischen den beiden Punkten als \c double.
 *
 * \relates Point
 */
double Distance(const Point &p1, const Point &p2)
{
	double a=abs(p2.x-p1.x);
	double b=abs(p2.y-p1.y);
	return sqrt((a*a)+(b*b));
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
int Point::manhattanLength() const
{
	return abs(x)+abs(y);
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
Point &Point::operator*= (double factor)
{
	x=(int)((double)x*factor);
	y=(int)((double)y*factor);
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
Point &Point::operator+= (const Point &point)
{
	x+=point.x;
	y+=point.y;
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
Point &Point::operator-= (const Point &point)
{
	x-=point.x;
	y-=point.y;
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
Point &Point::operator/= (double divisor)
{
	x=(int)((double)x/divisor);
	y=(int)((double)y/divisor);
	return *this;
}

bool operator!= (const Point &p1, const Point &p2)
{
	if (p1.x!=p2.x) return true;
	if (p1.y!=p2.y) return true;
	return false;
}

bool operator== (const Point &p1, const Point &p2)
{
	if (p1.x!=p2.x) return false;
	if (p1.y!=p2.y) return false;
	return true;
}

const Point operator* (const Point &point, double factor)
{
	return Point ((int)(point.x*factor),(int)(point.y*factor));
}

const Point operator* (double factor, const Point &point)
{
	return Point ((int)(point.x*factor),(int)(point.y*factor));
}

const Point operator+ (const Point &p1, const Point &p2)
{
	return Point (p1.x+p2.x,p1.y+p2.y);
}

const Point operator- (const Point &p1, const Point &p2)
{
	return Point (p1.x-p2.x,p1.y-p2.y);
}

const Point operator- (const Point &point)
{
	return Point (0-point.x,0-point.y);
}

const Point operator/ (const Point &point, double divisor)
{
	return Point((int)(point.x/divisor),(int)(point.y/divisor));
}

} // EOF namespace grafix
} // EOF namespace ppl7
