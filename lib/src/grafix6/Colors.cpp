/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.2 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: Colors.cpp,v 1.2 2010/02/12 19:43:48 pafe Exp $
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
#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include "ppl6-grafix.h"

#ifdef HAVE_X86_ASSEMBLER

extern "C" {
	ppl6::grafix::SurfaceColor RGB_A8R8G8B8(int red, int green, int blue, int alpha);
	ppl6::grafix::SurfaceColor RGBBlend_32_255 (ppl6::grafix::SurfaceColor ground, ppl6::grafix::SurfaceColor top, int intensity);
}
#endif


namespace ppl6 {
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
	r=Clamp(red);
	g=Clamp(green);
	b=Clamp(blue);
	a=Clamp(alpha);
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
Color::Color(ppluint32 rgba)
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
Color::Color(ppluint32 rgb, int alpha)
{
	setColor(rgb, alpha);
}

/*!\brief Zurechtschneiden von Farbwerten
 *
 * \desc
 * Diese interne Funktion wird verwendet, um übergebene Farbparameter
 * auf Werte zwischen 0 und 255 zu begrenzen. Ein Wert kleiner 0
 * wird durch 0 ersetzt, Werte größer 255 werden zu 255.
 *
 * @param value Der zu normalisierende Wert
 * @return Ein normalisierter Wert zwischen 0 und 255
 */
int Color::Clamp(int value)
{
	if (value<0) return 0;
	if (value>255) return 255;
	return value;
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
ppluint32 Color::color() const
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
ppluint32 Color::rgba() const
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
ppluint32 Color::rgb() const
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
	r=Clamp(red);
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
	g=Clamp(green);
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
	b=Clamp(blue);
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
	a=Clamp(alpha);
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
	r=Clamp(red);
	g=Clamp(green);
	b=Clamp(blue);
	a=Clamp(alpha);
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
void Color::setColor(ppluint32 rgba)
{
	RGBA c;
	c.color=rgba;
	r=c.red;
	g=c.green;
	b=c.blue;
	a=c.alpha;
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
void Color::setColor(ppluint32 rgb, int alpha)
{
	RGBA c;
	c.color=rgb;
	r=c.red;
	g=c.green;
	b=c.blue;
	a=alpha;
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
	r=Clamp((int)(r*factor));
	g=Clamp((int)(r*factor));
	b=Clamp((int)(r*factor));
	return *this;
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
Color::operator ppluint32() const
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
Color::operator pplint32() const
{
	return c;
}



/*!\brief Vergleich zweier Farben
 *
 * \desc
 * Mit den Operatoren \p == und \p != können zwei Farbwerte miteinander verglichen werden.
 *
 * @param c1 Erster Farbwert
 * @param c2 Zweiter Farbwert
 * @return Bei Verwendung des Operators \p == wird \c true zurückgegeben, wenn beide Farbwerte
 * identisch sind und \c false, wenn dies nicht der Fall ist. Bei Verwendung des Operators
 * \p != wird \c true zurückgegeben, wenn die Farbwerte unterschiedlich sind und \c false, wenn
 * sie identisch sind.
 */
bool operator!= (const Color &c1, const Color &c2)
{
	if (c1.c!=c2.c) return true;
	return false;
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
bool operator== (const Color &c1, const Color &c2)
{
	if (c1.c==c2.c) return true;
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
	return Color(clamp(color.r*factor),clamp(color.g*factor),clamp(color.b*factor),color.a);
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
	return Color(clamp(color.r*factor),clamp(color.g*factor),clamp(color.b*factor),color.a);
}



static SurfaceColor RGB_NULL (int red, int green, int blue, int alpha)
{
	return 0;
}
static Color Surface2RGB_NULL (SurfaceColor color)
{
	return Color();
}

static SurfaceColor RGBBlend_NULL (SurfaceColor ground, SurfaceColor top, float intensity)
{
	return 0;
}

// Grey 8-Bit ************************************************************************
static SurfaceColor RGB_GREY8 (int red, int green, int blue, int alpha)
{
	return (red * 11 + green * 16 + blue * 5)/32;
	//return ((red+(green<<8)+(blue<<24)*255)/0xffffff);
}
static Color Surface2RGB_GREY8 (SurfaceColor color)
{
	return Color(color,color,color);
}


// 32 Bit ****************************************************************************
#ifndef HAVE_X86_ASSEMBLER
static SurfaceColor RGB_A8R8G8B8 (int red, int green, int blue, int alpha)
{
	ppluint32 r,g,b,a;
	r=red & 255;
	g=green & 255;
	b=blue & 255;
	a=alpha & 255;
	return (b+(g<<8)+(r<<16)+(a<<24));
}
#endif

static SurfaceColor RGB_A8B8G8R8 (int red, int green, int blue, int alpha)
{
	ppluint32 r,g,b,a;
	r=red & 255;
	g=green & 255;
	b=blue & 255;
	a=alpha & 255;
	return (r+(g<<8)+(b<<16)+(a<<24));
}

static Color Surface2RGB_A8R8G8B8 (SurfaceColor color)
{
	Color c;
	c.setBlue(color&255);
	c.setGreen((color>>8)&255);
	c.setRed((color>>16)&255);
	c.setAlpha((color>>24)&255);
	return c;
}

static Color Surface2RGB_A8B8G8R8 (SurfaceColor color)
{
	Color c;
	c.setRed(color&255);
	c.setGreen((color>>8)&255);
	c.setBlue((color>>16)&255);
	c.setAlpha((color>>24)&255);
	return c;
}

#ifndef HAVE_X86_ASSEMBLER
// Die C-Version ist hier schneller als MMX, oder doch nicht?
static SurfaceColor RGBBlend_32_255 (SurfaceColor ground, SurfaceColor top, int intensity)
{
	ppldd r1,g1,b1,r2,g2,b2,i1,i2;
	r1=ground&255;
	g1=(ground>>8)&255;
	b1=(ground>>16)&255;
	r2=top&255;
	g2=(top>>8)&255;
	b2=(top>>16)&255;
	i2=intensity&255;
	i1=255-i2;
	r1=((r1*i1) + (r2*i2))/255;
	g1=((g1*i1) + (g2*i2))/255;
	b1=((b1*i1) + (b2*i2))/255;

	return r1+(g1<<8)+(b1<<16)+0xff000000;
}
#endif

static SurfaceColor RGBBlend_32 (SurfaceColor ground, SurfaceColor top, float intensity)
{
	return RGBBlend_32_255(ground, top,(int)(intensity*255));
	/*
	ppldd r1,g1,b1,r2,g2,b2,i1,i2;
	r1=ground&255;
	g1=(ground>>8)&255;
	b1=(ground>>16)&255;
	r2=top&255;
	g2=(top>>8)&255;
	b2=(top>>16)&255;
	i2=(ppldd)(intensity*255);
	i1=255-i2;

	r1=(r1*i1/255) + (r2*i2/255);
	g1=(g1*i1/255) + (g2*i2/255);
	b1=(b1*i1/255) + (b2*i2/255);

	return r1+(g1<<8)+(b1<<16)+0xff000000;
	*/
}

/*!\brief Farbe in einen Farbformatspezifischen Wert umrechnen
 *
 * \desc
 * Mit dieser Funktion wird eine farbformat unabhängige Farbe \p c in einen
 * format spezifischen Wert umgerechnet.
 *
 * @param c Farbe
 * @return Farbwert
 */
SurfaceColor CDrawable::rgb(const Color &c) const
{
	if (fn->RGB) return fn->RGB(c.red(),c.green(),c.blue(),c.alpha());
	return 0;
}

/*!\brief Farbe in einen Farbformatspezifischen Wert umrechnen
 *
 * \desc
 * Mit dieser Funktion wird eine farbformat unabhängige Farbe mit den
 * Komponenten rot, grün, blau und Transparenz in einen
 * format spezifischen Wert umgerechnet.
 *
 * @param r rot-Wert
 * @param g grün-Wert
 * @param b blau-Wert
 * @param alpha Transparenz
 * @return Farbwert
 */
SurfaceColor CDrawable::rgb(int r, int g, int b, int alpha) const
{
	if (fn->RGB) return fn->RGB(r,g,b,alpha);
	return 0;
}


int CGrafix::InitColors(const RGBFormat &format, GRAFIX_FUNCTIONS *fn)
{
	switch (format) {
		/*
		case RGBFormat::Palette:
			data.RGB=RGB_Palette;
			data.Surface2RGB=Surface2RGB_Palette;
			data.RGBBlend=RGBBlend_8;
			break;
		case RGBFormat::R5G6B5:
			data.RGB=RGB_R5G6B5;
			data.Surface2RGB=Surface2RGB_R5G6B5;
			data.RGBBlend=RGBBlend_R5G6B5;
			break;
		case RGBFormat::B5G6R5:
			data.RGB=RGB_B5G6R5;
			data.Surface2RGB=Surface2RGB_B5G6R5;
			data.RGBBlend=RGBBlend_B5G6R5;
			break;
		case RGBFormat::R5G5B5:
			data.RGB=RGB_R5G5B5;
			data.Surface2RGB=Surface2RGB_R5G5B5;
			data.RGBBlend=RGBBlend_R5G5B5;
			break;
		case RGBFormat::B5G5R5:
			data.RGB=RGB_B5G5R5;
			data.Surface2RGB=Surface2RGB_B5G5R5;
			data.RGBBlend=RGBBlend_B5G5R5;
			break;
		case RGBFormat::R4G4B4:
			data.RGB=RGB_R4G4B4;
			data.Surface2RGB=Surface2RGB_R4G4B4;
			data.RGBBlend=RGBBlend_R4G4B4;
			break;
		case RGBFormat::A4R4G4B4:
			data.RGB=RGB_A4R4G4B4;
			data.Surface2RGB=Surface2RGB_A4R4G4B4;
			data.RGBBlend=RGBBlend_A4R4G4B4;
			break;
			*/
		case RGBFormat::R8G8B8:
		case RGBFormat::A8R8G8B8:
		case RGBFormat::X8R8G8B8:
			fn->RGB=RGB_A8R8G8B8;
			fn->Surface2RGB=Surface2RGB_A8R8G8B8;
			fn->RGBBlend=RGBBlend_32;
			fn->RGBBlend255=RGBBlend_32_255;
			return 1;
		case RGBFormat::B8G8R8:
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8B8G8R8:
			fn->RGB=RGB_A8B8G8R8;
			fn->Surface2RGB=Surface2RGB_A8B8G8R8;
			fn->RGBBlend=RGBBlend_32;
			fn->RGBBlend255=RGBBlend_32_255;
			return 1;
			/*
		case RGBFormat::R3G3B2:
			data.RGB=RGB_R3G3B2;
			data.Surface2RGB=Surface2RGB_R3G3B2;
			data.RGBBlend=RGBBlend_R3G3B2;
			break;
			*/
		case RGBFormat::A8:
		case RGBFormat::GREY8:
			fn->RGB=RGB_GREY8;
			fn->Surface2RGB=Surface2RGB_GREY8;
			fn->RGBBlend=RGBBlend_NULL;
			return 1;
		default:
			fn->RGB=RGB_NULL;
			fn->Surface2RGB=Surface2RGB_NULL;
			fn->RGBBlend=RGBBlend_NULL;
	}
	SetError(1013,"RGBFormat=%s (%i)",(const char*)format.name(),format.format());
	return 0;
}





} // EOF namespace grafix
} // EOF namespace ppl6
