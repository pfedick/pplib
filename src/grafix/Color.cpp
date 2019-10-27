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
#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include "ppl7-grafix.h"

namespace ppl7 {
namespace grafix {


/*!\union RGBA
 * \relates Color
 * \ingroup PPLGroupGrafik
 * \brief 32-Bit Struktur zum Speichern eines RGB-Farbwertes
 *
 * \desc
 * Diese Union dient zum Speichern eines 32-Bit Farbwertes. Der Zugriff kann entweder direkt auf den 32-Bit-Wert mittels \p RGBA.color erfolgen
 * oder auf jede der 3 Farbkomponenten direkt mittels \p RGBA.red, \p RGBA.green und \p RGBA.blue. Als vierte Komponente ist noch der
 * Transparanz-Wert \p RGBA.alpha vorhanden.
 * \par
 * Die Union ist so definiert, dass der rot-Wert immer im niedrigsten Byte und der Transparenz-Wert im höchsten Byte gespeichert wird,
 * unabhängig von der Prozessorarchitektur.
 */


/*!\class Color
 * \ingroup PPLGroupGrafik
 * \brief Ein Datenobjekt zum Speichern eines Farbwertes
 *
 * \desc
 * Diese Klasse wird als Objekt zum Speichern einer Farbe verwendet, die sich aus den
 * Komponenten Rot, Grün, Blau und optional einem Transparanz-Wert (Alpha-Wert) zusammensetzt.
 * Der Farbwert ist 32-Bit breit, wobei jede der 4 Komponenten 8 Bit verwendet.
 *
 */


/*!\var Color::c
 * \brief 32-Bit-Farbwert
 */

static inline int clamp(int value)
{
	if (value<0) return 0;
	if (value>255) return 255;
	return value;
}


/*!\brief Konstruktor ohne Angabe von Werten
 *
 * \desc
 * Mit diesem Konstruktor wird eine Instanz der Klasse erzeugt, und mit dem Farbwert
 * für schwarz initialisiert.
 */
Color::Color()
{
	c=0;
	a=255;
}

/*!\brief Konstruktor mit Angabe der Farbwerte
 *
 * \desc
 * Mit diesem Konstruktor wird eine Instanz der Klasse erzeugt, die mit den angegebenen
 * Farbkomponenten initialisiert wird.
 *
 * @param red Wert zwischen 0 und 255, der den rot-Anteil der Farbe festlegt
 * @param green Wert zwischen 0 und 255, der den grün-Anteil der Farbe festlegt
 * @param blue Wert zwischen 0 und 255, der den blau-Anteil der Farbe festlegt
 * @param alpha Optionaler Wert zwischen 0 und 255, der die Transparenz der Farbe
 * festlegt. 0 bedeutet komplett transparent, 255 komplett sichtbar.
 */
Color::Color(int red, int green, int blue, int alpha)
{
	r=clamp(red);
	g=clamp(green);
	b=clamp(blue);
	a=clamp(alpha);
}

/*!\brief Konstruktor mit Angabe eines Farbwertes als 32-Bit Wert
 *
 * \desc
 * Mit diesem Konstruktor wird eine Instanz der Klasse erzeugt, die mit
 * dem angegebenen 32-Bit-Farbwert initialisiert wird.
 *
 * @param rgba 32-Bit Farbwert mit folgendem Aufbau:
 * - das unterste Byte enthält den rot-Wert zwischen 0 und 255
 * - das zweite Byte enthält den grün-Wert zwischen 0 und 255
 * - Byte 3 enthält den Blau-Wert zwischen 0 und 255
 * - Byte 4 den Alpha-Wert zwischen 0 und 255
 */
Color::Color(uint32_t rgba)
{
	setColor(rgba);
}


/*!\brief Konstruktor mit Angabe eines 32-Bit-Farbwertes und der Transparenz
 *
 * \desc
 * Mit diesem Konstruktor wird eine Instanz der Klasse erzeugt, die mit
 * dem angegebenen 32-Bit-Farbwert \p rgb und der Transparenz \p alpha initialisiert wird.
 *
 * @param rgb 32-Bit Farbwert mit folgendem Aufbau:
 * - das unterste Byte enthält den rot-Wert zwischen 0 und 255
 * - das zweite Byte enthält den grün-Wert zwischen 0 und 255
 * - Byte 3 enthält den Blau-Wert zwischen 0 und 255
 * @param alpha Der Transparenz-Wert der Farbe zwischen 0 (völlig transparent)
 * und 255 (vollständig sichtbar).
 */
Color::Color(uint32_t rgb, int alpha)
{
	setColor(rgb, alpha);
}


/*!\brief Farbwert als 32-Bit-Wert auslesen
 *
 * \desc
 * Mit dieser Funktion kann der Farbwert als 32-Bit-Wert ausgelesen werden.
 *
 * @return 32-Bit Farbwert mit folgendem Aufbau:
 * - das unterste Byte enthält den rot-Wert
 * - das zweite Byte enthält den grün-Wert
 * - Byte 3 enthält den Blau-Wert
 * - Byte 4 den Alpha-Wert
 */
uint32_t Color::color() const
{
	return c;
}

/*!\brief Farbwert als 32-Bit-Wert auslesen
 *
 * \desc
 * Mit dieser Funktion kann der Farbwert als 32-Bit-Wert ausgelesen werden.
 *
 * @return 32-Bit Farbwert mit folgendem Aufbau:
 * - das unterste Byte enthält den rot-Wert
 * - das zweite Byte enthält den grün-Wert
 * - Byte 3 enthält den Blau-Wert
 * - Byte 4 den Alpha-Wert
 */
uint32_t Color::rgba() const
{
	return c;
}

/*!\brief Farbwert als 32-Bit-Wert ohne Alpha-Kanal auslesen
 *
 * \desc
 * Mit dieser Funktion kann der Farbwert als 32-Bit-Wert ohne den Transparenz-Wert
 * ausgelesen werden.
 *
 * @return 32-Bit Farbwert mit folgendem Aufbau:
 * - das unterste Byte enthält den rot-Wert
 * - das zweite Byte enthält den grün-Wert
 * - Byte 3 enthält den Blau-Wert
 * - Byte 4 ist immer 0
 */
uint32_t Color::rgb() const
{
	return c&0xffffff;
}

/*!\brief rot-Anteil auslesen
 *
 * \desc
 * Mit dieser Funktion wird der rot-Anteil der Farbe ausgelesen.
 *
 * @return rot-Anteil zwischen 0 und 255
 */
int Color::red() const
{
	return r;
}

/*!\brief grün-Anteil auslesen
 *
 * \desc
 * Mit dieser Funktion wird der grün-Anteil der Farbe ausgelesen.
 *
 * @return grün-Anteil zwischen 0 und 255
 */
int Color::green() const
{
	return g;
}

/*!\brief blau-Anteil auslesen
 *
 * \desc
 * Mit dieser Funktion wird der blau-Anteil der Farbe ausgelesen.
 *
 * @return blau-Anteil zwischen 0 und 255
 */
int Color::blue() const
{
	return b;
}

/*!\brief Transparenz-Wert auslesen
 *
 * \desc
 * Mit dieser Funktion wird der Transparenz-Wert (Alpha) der Farbe
 * ausgelesen.
 *
 * @return alpha-Wert zwischen 0 (völlig transparent) und 255 (vollständig
 * sichtbar)
 */
int Color::alpha() const
{
	return a;
}

/*!\brief rot-Anteil setzen
 *
 * \desc
 * Mit dieser Funktion wird der rot-Anteil der Farbe gesetzt.
 *
 * @param red rot-Anteil zwischen 0 und 255. Ein Wert kleiner 0 wird zu 0,
 * ein Wert größer 255 wird zu 255.
 */
void Color::setRed(int red)
{
	r=clamp(red);
}

/*!\brief grün-Anteil setzen
 *
 * \desc
 * Mit dieser Funktion wird der grün-Anteil der Farbe gesetzt.
 *
 * @param red grün-Anteil zwischen 0 und 255. Ein Wert kleiner 0 wird zu 0,
 * ein Wert größer 255 wird zu 255.
 */
void Color::setGreen(int green)
{
	g=clamp(green);
}

/*!\brief blau-Anteil setzen
 *
 * \desc
 * Mit dieser Funktion wird der blau-Anteil der Farbe gesetzt.
 *
 * @param red blau-Anteil zwischen 0 und 255. Ein Wert kleiner 0 wird zu 0,
 * ein Wert größer 255 wird zu 255.
 */
void Color::setBlue(int blue)
{
	b=clamp(blue);
}

/*!\brief Transparenz-Wert setzen
 *
 * \desc
 * Mit dieser Funktion wird der Transparenz-Wert (Alpha) der Farbe gesetzt.
 *
 * @param red Transparenz-Wert zwischen 0 (völlig transparent) und 255
 * (vollständig sichtbar). Ein Wert kleiner 0 wird zu 0,
 * ein Wert größer 255 wird zu 255.
 */
void Color::setAlpha(int alpha)
{
	a=clamp(alpha);
}

/*!\brief Farbwert anhand der einzelnen Farbkomponenten setzen
 *
 * \desc
 * Mit dieser Funktion wird die Farbe anhand ihrer einzelnen
 * Komponenten gesetzt.

 * @param red Wert zwischen 0 und 255, der den rot-Anteil der Farbe festlegt
 * @param green Wert zwischen 0 und 255, der den grün-Anteil der Farbe festlegt
 * @param blue Wert zwischen 0 und 255, der den blau-Anteil der Farbe festlegt
 * @param alpha Optionaler Wert zwischen 0 und 255, der die Transparenz der Farbe
 * festlegt. 0 bedeutet komplett transparent, 255 komplett sichtbar. Wird der
 * Parameter nicht angegeben, wird automatisch 255 gesetzt.
 */
void Color::setColor(int red, int green, int blue, int alpha)
{
	r=clamp(red);
	g=clamp(green);
	b=clamp(blue);
	a=clamp(alpha);
}

/*!\brief Farbwert anhand der einzelnen Farbkomponenten setzen
 *
 * \desc
 * Mit dieser Funktion wird die Farbe anhand ihrer einzelnen
 * Komponenten gesetzt.

 * @param red Wert zwischen 0 und 255, der den rot-Anteil der Farbe festlegt
 * @param green Wert zwischen 0 und 255, der den grün-Anteil der Farbe festlegt
 * @param blue Wert zwischen 0 und 255, der den blau-Anteil der Farbe festlegt
 * @param alpha Optionaler Wert zwischen 0 und 255, der die Transparenz der Farbe
 * festlegt. 0 bedeutet komplett transparent, 255 komplett sichtbar. Wird der
 * Parameter nicht angegeben, wird automatisch 255 gesetzt.
 */
void Color::set(int red, int green, int blue, int alpha)
{
	r=clamp(red);
	g=clamp(green);
	b=clamp(blue);
	a=clamp(alpha);
}

void Color::set(const Color &other)
{
	r=other.r;
	g=other.g;
	b=other.b;
	a=other.a;
}

/*!\brief Farbwert anhand eines 32-Bit-Wertes setzen
 *
 * \desc
 * Mit dieser Funktgion wird die Farbe anhand des angegebenen
 * 32-Bit-Farbwertes gesetzt.
 *
 * @param rgba 32-Bit Farbwert mit folgendem Aufbau:
 * - das unterste Byte enthält den rot-Wert
 * - das zweite Byte enthält den grün-Wert
 * - Byte 3 enthält den Blau-Wert
 * - Byte 4 den Alpha-Wert
 */
void Color::setColor(uint32_t rgba)
{
	r=rgba&255;
	g=(rgba>>8)&255;
	b=(rgba>>16)&255;
	a=(rgba>>24)&255;
}

/*!\brief Farbwert anhand eines 32-Bit-Farbwertes und der Transparenz setzen
 *
 * \desc
 * Mit dieser Funktion wird die Farbe anhand des
 * angegebenen 32-Bit-Farbwerts \p rgb und der Transparenz \p alpha gesetzt.
 *
 * @param rgb 32-Bit Farbwert mit folgendem Aufbau:
 * - das unterste Byte enthält den rot-Wert zwischen 0 und 255
 * - das zweite Byte enthält den grün-Wert zwischen 0 und 255
 * - Byte 3 enthält den Blau-Wert zwischen 0 und 255
 * @param alpha Der Transparenz-Wert der Farbe zwischen 0 (völlig transparent)
 * und 255 (vollständig sichtbar).
 */
void Color::setColor(uint32_t rgb, int alpha)
{
	r=rgb&255;
	g=(rgb>>8)&255;
	b=(rgb>>16)&255;
	a=clamp(alpha);
}

/*!\brief Helligkeit der Farbe
 *
 * \desc
 * Mit dieser Funktion kann die Helligkeit des Farbwerts ausgelesen werden. Um die Helligkeit
 * zu berechnen, werden die 3 Komponenten der Farbe rot, grün und blau unterschiedlich gewichtet.
 * Da grün am hellsten ist, wird dieser Komponente am höchsten bewertet, gefolgt von rot und
 * zuletzt blau. Die genaue Formel lautet:
 * \f$helligkeit = (rot * 11 + grün * 16 + blau * 5)/32\f$.
 * Der Alpha-Kanal (Transparenz) spielt bei der Berechnung keine Rolle.
 *
 * @return Helligkeit der Farbe auf einer Skala von 0-255
 */
int Color::brightness() const
{
	return (r * 11 + g * 16 + b * 5)/32;
}

/*!\brief Grauwert der Farbe
 *
 * \desc
 * Diese Funktion ist identisch zu Color::brightness, nur wird hier nicht der
 * Helligkeitswert zurückgegeben, sondern eine neue Color-Klasse, deren Farbkomponenten
 * jeweils mit dem gleichen Helligkeitswert initialisiert werden. Der Transparenz-Wert wird
 * übernommen.
 *
 * @return Grau-Farbe
 */
Color Color::grey() const
{
	int v=brightness();
	return Color(v,v,v,a);
}

Color Color::negativ() const
{
	return Color(255-r,255-g,255-b,a);
}

/*!\brief Farbübereinstimmung prüfen
 *
 * \desc
 * Diese Funktion prüft, ob die Farbe \p other mit dieser übereinstimmt, wobei jeder der
 * drei Farbwerte (rot, grün, blau) eine maximale Abweichung von \p tolerance haben darf.
 * Der Alpha-Kanal wird dabei nicht berücksichtigt.
 *
 * @param other Der zu vergleichende Farbwert
 * @param tolerance Ein optionaler Parameter, der die maximale Toleranz auf einer Skala
 * von 0-255 angibt. Der Default ist 0
 * @return Stimmen die Farbwerte überein, liefert die Funktion \c true zurück, andernfalls \c false.
 */
bool Color::match(const Color &other, int tolerance) const
{
	if (c==other.c) return true;
	if (abs(r-other.r)>tolerance) return false;
	if (abs(g-other.g)>tolerance) return false;
	if (abs(b-other.b)>tolerance) return false;
	return true;
}

/*!\brief Farben überblenden
 *
 * \desc
 * Mit dieser Funktion werden die zwei Farbwerte \p background und \p foreground
 * anhand des dritten Parameters \p intensity vermischt. Dabei gilt, je höher der Wert
 * \p intensity, desto stärker kommt der Farbwert \p foreground zur Geltung.
 *
 * @param background Hintergrundfarbe
 * @param foreground Vordergrundfarbe
 * @param intensity Intensität der Vordergrundfarbe auf einer Skala von 0-255
 * @return Neuer Farbwert
 */
Color &Color::blend(const Color &background, const Color &foreground, int intensity)
{
	int i2=intensity&255;
	int i1=255-i2;
	r=((background.r*i1) + (foreground.r*i2))/255;
	g=((background.g*i1) + (foreground.g*i2))/255;
	b=((background.b*i1) + (foreground.b*i2))/255;
	a=255;
	return *this;
}

/*!\brief Farben überblenden
 *
 * \desc
 * Mit dieser Funktion werden die zwei Farbwerte \p background und \p foreground
 * anhand des dritten Parameters \p intensity vermischt. Dabei gilt, je höher der Wert
 * \p intensity, desto stärker kommt der Farbwert \p foreground zur Geltung.
 *
 * @param background Hintergrundfarbe
 * @param foreground Vordergrundfarbe
 * @param intensity Intensität der Vordergrundfarbe auf einer Skala von 0.0 bis 1.0
 * @return Neuer Farbwert
 */
Color &Color::blendf(const Color &background, const Color &foreground, float intensity)
{
	float i2=intensity;
	float i1=1.0f-i2;
	r=(int)((background.r*i1) + (foreground.r*i2));
	g=(int)((background.g*i1) + (foreground.g*i2));
	b=(int)((background.b*i1) + (foreground.b*i2));
	a=255;
	return *this;
}

/*!\brief Multiplikation der einzelnen Farbkomponenten mit einem Faktor
 *
 * \desc
 * Mit diesem Operator werden die einzelnen Farbkomponenten rot, grün und blau
 * mit dem angegebenen Faktor multipliziert. Falls das Ergebnis dabei kleiner 0 wird, wird
 * der Wert auf 0 korrigiert, Wert größer 255 werden zu 255.
 *
 * @param factor Der Multiplikator als float-Wert
 * @return Neuer Farbwert
 */
Color &Color::operator*= (float factor)
{
	r=clamp((int)(r*factor));
	g=clamp((int)(r*factor));
	b=clamp((int)(r*factor));
	return *this;
}

Color &Color::operator+= (const Color &other)
{
	r=clamp(r+other.r);
	g=clamp(g+other.g);
	b=clamp(b+other.b);
	a=clamp(a+other.a);
	return *this;
}


/*!\brief Farbe in das YCbCr-Farbmodell umrechnen
 *
 * \desc
 * Das YCbCr-Modell teilt die Farbinformation auf in die Grundhelligkeit Y
 * und die zwei Farbkomponenten Cb (Blue-Yellow Chrominance) und Cr (Red-Green Chrominance).
 * Mit Y wird hier die Helligkeitsachse aus dem CIE-Normvalenzsystem verwendet. Sie entspricht
 * der Hellempfindlichkeit des Auges, die im grünen Spektralbereich am größten ist (V-Lambda-Kurve).
 * Chrominance oder kurz Chroma bedeutet Buntheit im Allgemeinen und Farbigkeit in Bezug auf
 * Helligkeit-Farbigkeits-Modelle.
 * \par
 * Die Funktion Color::getY gibt die Grundhelligkeit zurück,
 * Color::getYCb den Cb-Wert und Color::getYCr den Cr-Wert
 *
 * @return Grundhelligkeit
 */
int Color::getY() const
{
	return (int) round(0.299*r + 0.587*g + 0.114*b);
}

//!\copydoc Color::getY
int Color::getYCb() const
{
	return (int) round(128 + -0.168736*r - 0.331264*g + 0.5*b);
}

//!\copydoc Color::getY
int Color::getYCr() const
{
	return (int) round(128 + 0.5*r - 0.418688*g - 0.081312*b);
}



/*!\brief Farbwert als 32-Bit-Wert auslesen
 *
 * \desc
 * Mit diesem Operator kann der Farbwert der Klasse als 32-Bit-Wert ausgelesen werden.
 *
 * @return 32-Bit Farbwert mit folgendem Aufbau:
 * - das unterste Byte enthält den rot-Wert
 * - das zweite Byte enthält den grün-Wert
 * - Byte 3 enthält den Blau-Wert
 * - Byte 4 den Alpha-Wert
 */
Color::operator uint32_t() const
{
	return c;
}

/*!\brief Farbwert als 32-Bit-Wert auslesen
 *
 * \desc
 * Mit diesem Operator kann der Farbwert der Klasse als 32-Bit-Wert ausgelesen werden.
 *
 * @return 32-Bit Farbwert mit folgendem Aufbau:
 * - das unterste Byte enthält den rot-Wert
 * - das zweite Byte enthält den grün-Wert
 * - Byte 3 enthält den Blau-Wert
 * - Byte 4 den Alpha-Wert
 */
Color::operator int32_t() const
{
	return c;
}

bool Color::operator<(const Color &other) const
{
	if (c<other.c) return true;
	return false;
}

bool Color::operator<=(const Color &other) const
{
	if (c<=other.c) return true;
	return false;
}

/*!\brief Vergleich zweier Farben
 *
 * \desc
 * Mit den Operatoren \p == und \p != können zwei Farbwerte miteinander verglichen werden.
 *
 * @param other Anderer Farbwert
 * @return Bei Verwendung des Operators \p == wird \c true zurückgegeben, wenn beide Farbwerte
 * identisch sind und \c false, wenn dies nicht der Fall ist. Bei Verwendung des Operators
 * \p != wird \c true zurückgegeben, wenn die Farbwerte unterschiedlich sind und \c false, wenn
 * sie identisch sind.
 */
bool Color::operator==(const Color &other) const
		{
	if (c==other.c) return true;
	return false;
		}

/*!\brief Vergleich zweier Farben
 *
 * \desc
 * Mit den Operatoren \p == und \p != können zwei Farbwerte miteinander verglichen werden.
 *
 * @param other Anderer Farbwert
 * @return Bei Verwendung des Operators \p == wird \c true zurückgegeben, wenn beide Farbwerte
 * identisch sind und \c false, wenn dies nicht der Fall ist. Bei Verwendung des Operators
 * \p != wird \c true zurückgegeben, wenn die Farbwerte unterschiedlich sind und \c false, wenn
 * sie identisch sind.
 */
bool Color::operator!=(const Color &other) const
		{
	if (c!=other.c) return true;
	return false;
		}

bool Color::operator>=(const Color &other) const
		{
	if (c>=other.c) return true;
	return false;
		}

bool Color::operator>(const Color &other) const
{
	if (c>other.c) return true;
	return false;
}


/*!\brief Multiplikation einer Farbe mit einem Faktor
 *
 * \desc
 * Mit diesem Operator wird die angegebene Farbe \p color mit dem Faktor \p factor
 * multipliziert und das Ergebnis als neuer Farbwert zurückgegeben. Ein Wert größer 1.0
 * bewirkt, dass die Farbe heller wird, bei Werten kleiner 1,0 wird die Farbe dunkler.
 *
 * Ist das Ergebnis der Multiplikation eines Farbwertes größer 255, wird er auf 255 gesetzt.
 * Ein negativer Faktor wird zu 0.
 *
 * @param color Farbwert
 * @param factor Multiplikations-Faktor
 * @return Neuer Farbwert
 */
const Color operator* (const Color &color, float factor)
{
	if (factor<0) factor=0.0f;
	return Color(clamp((int)((float)color.r*factor)),
		clamp((int)((float)color.g*factor)),
		clamp((int)((float)color.b*factor)),
		color.a);
}

/*!\brief Multiplikation einer Farbe mit einem Faktor
 *
 * \desc
 * Mit diesem Operator wird die angegebene Farbe \p color mit dem Faktor \p factor
 * multipliziert und das Ergebnis als neuer Farbwert zurückgegeben. Ein Wert größer 1.0
 * bewirkt, dass die Farbe heller wird, bei Werten kleiner 1,0 wird die Farbe dunkler.
 *
 * Ist das Ergebnis der Multiplikation eines Farbwertes größer 255, wird er auf 255 gesetzt.
 * Ein negativer Faktor wird zu 0.
 *
 * @param factor Multiplikations-Faktor
 * @param color Farbwert
 * @return Neuer Farbwert
 */
const Color operator* (float factor, const Color &color)
{
	if (factor<0) factor=0.0;
	return Color(clamp((int)((float)color.r*factor)),
		clamp((int)((float)color.g*factor)),
		clamp((int)((float)color.b*factor)),
		color.a);
}

std::ostream& operator<<(std::ostream& s, const Color &c)
{
	String t;
	t.setf("RGB(red=%i, green=%i, blue=%i, alpha=%i)",c.red(), c.green(), c.blue(), c.alpha());
	return s.write((const char*)t.getPtr(),t.size());
}


Color Color::getBlended(const Color &background, const Color &foreground, int intensity)
{
	Color c;
	int i2=intensity&255;
	int i1=255-i2;
	c.r=((background.r*i1) + (foreground.r*i2))/255;
	c.g=((background.g*i1) + (foreground.g*i2))/255;
	c.b=((background.b*i1) + (foreground.b*i2))/255;
	c.a=255;
	return c;
}

Color Color::getBlendedf(const Color &background, const Color &foreground, float intensity)
{
	Color c;
	float i2=intensity;
	float i1=1.0f-i2;
	c.r=(int)((background.r*i1) + (foreground.r*i2));
	c.g=(int)((background.g*i1) + (foreground.g*i2));
	c.b=(int)((background.b*i1) + (foreground.b*i2));
	c.a=255;
	return c;
}


const Color operator+ (const Color &color1, const Color &color2)
{
	return Color(clamp(color1.r+color2.r),
			clamp(color1.g+color2.g),
			clamp(color1.r+color2.b),
			clamp(color1.a+color2.a));
}


} // EOF namespace grafix
} // EOF namespace ppl7
