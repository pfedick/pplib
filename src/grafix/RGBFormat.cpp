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
#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include "ppl7-grafix.h"

namespace ppl7 {
namespace grafix {


/*!\class RGBFormat
 * \ingroup PPLGroupGrafik
 * \brief Datentyp, der das Farbformat einer Zeichenfläche repräsentiert
 *
 * \desc
 * Mit dieser Klasse wird das Farbformat einer Zeichenfläche repräsentiert. Sie enthält nur
 * einen einzigen Wert aus der Enumeration RGBFormat::Identifier, der das verwendete
 * Farbformat festlegt.
 */

/*!\enum RGBFormat::Identifier
 * \brief ID des Farbformats
 *
 * \desc
 * Diese Enumeration enthält die Namen der möglichen Farbformate. Gegenwärtig werden
 * von der PPL-Library aber nur folgende 32-Bit-Formate unterstützt:
 * - RGBFormat::X8R8G8B8
 * - RGBFormat::A8R8G8B8
 * - RGBFormat::X8B8G8R8
 * - RGBFormat::A8B8G8R8
 *
 */

/*!\var RGBFormat::Identifier RGBFormat::unknown
 * \brief Unbekanntes Farbformat
 */

/*!\var RGBFormat::Identifier RGBFormat::Palette
 * \brief 8-Bit Farbwerte aus einer Farbpalette.
 * Es handelt sich um eine Grafik mit einer Farbpalette von 256 Farben. Satt des eigentlichen Farbwertes
 * enthält jeder Pixel der Grafik eine Referenz auf die Farbpalette. Dieses Format war Anfang der 90er
 * sehr populär, da es wenig Speicher braucht, sehr schnell ist und von jeder Grafikkarte unterstützt
 * wurde.
 */

/*!\var RGBFormat::Identifier RGBFormat::R3G3B2
 * \brief 8-Bit, 3 Bit für Rot, 3 Bit für Grün und 2 Bit für Blau,
 * wobei Blau in den unteren Bits enthalten ist.
 */

/*!\var RGBFormat::Identifier RGBFormat::R5G6B5
 * \brief 16-Bit High-Color, 5 Bit für Rot, 6 Bit für Grün und 5 Bit für Blau,
 * wobei Blau in den unteren Bits enthalten ist
 */

/*!\var RGBFormat::Identifier RGBFormat::A8R3G3B2
 * \brief 16-Bit, 3 Bit für Rot, 3 Bit für Grün, 2 Bit für Blau und 8 Bit für
 * den Alphakanal, wobei Blau in den unteren Bits enthalten ist und in den höchstwertigen
 * 8-Bit der Alphakanal (Transparenz).
 */


/*!\var RGBFormat::Identifier RGBFormat::X1R5G5B5
 * \brief 16-Bit High-Color, 5 Bit für Rot, 5 Bit für Grün und 5 Bit für Blau,
 * wobei Blau in den unteren Bits enthalten ist und das höchstwertige Bit
 * unbenutzt ist.
 */

/*!\var RGBFormat::Identifier RGBFormat::A1R5G5B5
 * \brief 16-Bit High-Color, 5 Bit für Rot, 5 Bit für Grün und 5 Bit für Blau,
 * wobei Blau in den unteren Bits enthalten ist und das höchstwertige Bit
 * als Alphakanal (Transparenz) verwendet wird.
 */

/*!\var RGBFormat::Identifier RGBFormat::B5G6R5
 * \brief 16-Bit High-Color, 5 Bit für Blau, 6 Bit für Grün und 5 Bit für Rot,
 * wobei Rot in den unteren Bits enthalten ist
 */

/*!\var RGBFormat::Identifier RGBFormat::X1B5G5R5
 * \brief 16-Bit High-Color, jeder Farbwert wird mit 5 Bit dargestellt,
 * wobei Rot in den unteren Bits enthalten
 * ist und das höchstwertige Bit unbenutzt ist.
 */

/*!\var RGBFormat::Identifier RGBFormat::A1B5G5R5
 * \brief 16-Bit High-Color, jeder Farbwert wird mit 5 Bit dargestellt,
 * wobei Rot in den unteren Bits enthalten
 * ist und das höchstwertige Bit als Alphakanal (Transparenz) verwendet wird.
 */

/*!\var RGBFormat::Identifier RGBFormat::X4R4G4B4
 * \brief 16-Bit High-Color, jeweils 4 Bit für Rot, Grün, Blau,
 * wobei Blau in den unteren Bits enthalten ist und in den höchstwertigen
 * 4-Bit unbenutzt sind.
 */

/*!\var RGBFormat::Identifier RGBFormat::A4R4G4B4
 * \brief 16-Bit High-Color, jeweils 4 Bit für Rot, Grün, Blau und Alpha,
 * wobei Blau in den unteren Bits enthalten ist und in den höchstwertigen
 * 4-Bit der Alphakanal (Transparenz).
 */


/*!\var RGBFormat::Identifier RGBFormat::R8G8B8
 * \brief 24-Bit Truecolor, jeweils 8 Bit für Rot, Grün und Blau,
 * wobei Blau im untersten Byte gespeichert wird.
 */

/*!\var RGBFormat::Identifier RGBFormat::B8G8R8
 * \brief 24-Bit Truecolor, jeweils 8 Bit für Rot, Grün und Blau,
 * wobei Rot im untersten Byte gespeichert wird.
 */

/*!\var RGBFormat::Identifier RGBFormat::A8R8G8B8
 * \brief 32-Bit Truecolor, jeweils 8 Bit für den Alpha-Wert, Rot, Grün und Blau,
 * wobei Blau im untersten Byte gespeichert wird
 */

/*!\var RGBFormat::Identifier RGBFormat::A8B8G8R8
 * \brief 32-Bit Truecolor, jeweils 8 Bit für den Alpha-Wert, Rot, Grün und Blau,
 * wobei Rot im untersten Byte gespeichert wird
 */

/*!\var RGBFormat::Identifier RGBFormat::X8R8G8B8
 * \brief 32-Bit Truecolor, jeweils 8 Bit für Rot, Grün und Blau,
 * wobei Blau im untersten Byte gespeichert wird und das höchste Byte unbenutzt ist
 */

/*!\var RGBFormat::Identifier RGBFormat::X8B8G8R8
 * \brief 32-Bit Truecolor, jeweils 8 Bit für Rot, Grün und Blau,
 * wobei Rot im untersten Byte gespeichert wird und das höchste Byte unbenutzt ist
 */


/*!\var RGBFormat::Identifier RGBFormat::A16R16G16B16
 * \brief 64-Bit Truecolor, jeweils 16 Bit für den Alpha-Wert, Rot, Grün und Blau,
 * wobei Blau im untersten Doppelbyte gespeichert wird der Alpha-Kanal im obersten.
 */

/*!\var RGBFormat::Identifier RGBFormat::X16R16G16B16
 * \brief 64-Bit Truecolor, jeweils 16 Bit für den Rot, Grün und Blau-Wert,
 * wobei Rot im untersten Doppelbyte gespeichert wird und das oberste Doppelbyte
 * unbenutzt ist.
 */


/*!\var RGBFormat::Identifier RGBFormat::GREY8
 * \brief 8-Bit Graustufen
 */

/*!\var RGBFormat::Identifier RGBFormat::A8
 * \brief 8-Bit Alphakanal
 */

/*!\var RGBFormat::Identifier RGBFormat::GREYALPHA32
 * \brief 32-Bit Graustufen mit Alpha-Wert
 */

/*!\var RGBFormat::Identifier RGBFormat::MaxIdentifiers
 * \brief Marker für den höchsten Identifier. Wird intern von der Klasse CGrafix verwendet.
 */


/*!\var RGBFormat::f
 * \brief Interne Variable zum Speichern des Farbformats
 *
 * \desc
 * Diese Variable wird intern von der RGBFormat-Klasse zum Speichern des Farbformats verwendet.
 */

/*!\brief Konstruktor der Klasse
 *
 * \desc
 * Mit diesem Konstruktor wird die Klasse mit dem Farbformat "unknown" initialisiert.
 *
 */
RGBFormat::RGBFormat()
{
	f=unknown;
}

/*!\brief Konstruktor mit Identifier
 *
 * \desc
 * Mit diesem Konstruktor wird die Klasse mit einem vorgegebenen Farbformat initialisiert
 *
 * @param[in] id Wert aus der Enumeration RGBFormat::Identifier
 */
RGBFormat::RGBFormat(Identifier id)
{
	f=id;
}

/*!\brief Konstruktor mit Namen
 *
 * \desc
 * Mit diesem Konstruktor wird die Klasse mit einem Farbformat initialisiert, dass anhand
 * seines Namens definiert wird. Die Namen sind identisch mit denen aus der Enumeration
 * RGBFormat::Identifier
 *
 * @param[in] Identifier String mit dem Namen des Farbformats
 *
 * \example
 * \code
 * RGBFormat rgb("A8R8B8G8");
 * \endcode
 */
RGBFormat::RGBFormat(const String &Identifier)
{
	setFormat(Identifier);
}

/*!\brief Farbformat Anhand einer ID festlegen
 *
 * \desc
 * Mit dieser Funktion wird das Farbformat anhand eines Wertes aus der Enumeration
 * RGBFormat::Identifier gesetzt.
 *
 * @param[in] id Wert aus der Enumeration RGBFormat::Identifier
 */
void RGBFormat::setFormat(Identifier id)
{
	if (id<unknown || id>=MaxIdentifiers) throw UnknownColorFormatException();
	f=id;
}

/*!\brief Farbformat anhand seines Namens festlegen
 *
 * \desc
 * Mit dieser Funktion wird das Farbformat anhand seines Namens gesetzt.
 * Die Namen sind identisch mit denen aus der Enumeration RGBFormat::Identifier.
 *
 * @param[in] Identifier String mit dem Namen des Farbformats
 *
 * \example
 * \code
 * RGBFormat rgb;
 * rgb.setFormat("A8R8B8G8");
 * \endcode
 * @return Kein Rückgabewert, im Fehlerfall wirft die Funktion eine Exception.
 * \exception UnknownColorFormatException Wird geworfen, wenn der \p Identifier nicht
 * erkannt wurde.
 */
void RGBFormat::setFormat(const String &Identifier)
{
	if (Identifier=="Palette") f=Palette;
	else if (Identifier=="R5G6B5") f=R5G6B5;
	else if (Identifier=="B5G6R5") f=B5G6R5;
	else if (Identifier=="X1R5G5B5") f=X1R5G5B5;
	else if (Identifier=="X1B5G5R5") f=X1B5G5R5;
	else if (Identifier=="X4R4G4B4") f=X4R4G4B4;
	else if (Identifier=="A1R5G5B5") f=A1R5G5B5;
	else if (Identifier=="A1B5G5R5") f=A1B5G5R5;
	else if (Identifier=="A4R4G4B4") f=A4R4G4B4;
	else if (Identifier=="R8G8B8") f=R8G8B8;
	else if (Identifier=="B8G8R8") f=B8G8R8;
	else if (Identifier=="A8R8G8B8") f=A8R8G8B8;
	else if (Identifier=="A8B8G8R8") f=A8B8G8R8;
	else if (Identifier=="R3G3B2") f=R3G3B2;
	else if (Identifier=="A8") f=A8;
	else if (Identifier=="A8R3G3B2") f=A8R3G3B2;
	else if (Identifier=="X8R8G8B8") f=X8R8G8B8;
	else if (Identifier=="X8B8G8R8") f=X8B8G8R8;
	else if (Identifier=="GREY8") f=GREY8;
	else if (Identifier=="GREYALPHA32") f=GREYALPHA32;
	else {
		throw UnknownColorFormatException(Identifier);
	}
}

/*!\brief Integer-Wert des Farbformats auslesen
 *
 * \desc
 * Mit dieser Funktion wird der Integer-Wert des Farbformates ausgelesen.
 * Der Wert entspricht der Position des Farbformates innerhalb der Enumeration RGBFormat::Identifier.
 *
 * @return Integer-Wert, der das Farbformat repräsentiert.
 */
int RGBFormat::format() const
{
	return f;
}

/*!\brief Name des Farbformates
 *
 * \desc
 * Diese Funktion gibt den Namen des aktuell gewählten Farbformates als String zurück.
 *
 * @return String mit dem Namen des Farbformates, oder "unknown", wenn kein Farbformat
 * gesetzt wurde.
 */
String RGBFormat::name() const
{
	switch (f) {
		case RGBFormat::Palette: return "Palette";
		case RGBFormat::R5G6B5: return "R5G6B5";
		case RGBFormat::B5G6R5: return "B5G6R5";
		case RGBFormat::X1R5G5B5: return "X1R5G5B5";
		case RGBFormat::X1B5G5R5: return "X1B5G5R5";
		case RGBFormat::X4R4G4B4: return "X4R4G4B4";
		case RGBFormat::A1R5G5B5: return "A1R5G5B5";
		case RGBFormat::A1B5G5R5: return "A1B5G5R5";
		case RGBFormat::A4R4G4B4: return "A4R4G4B4";
		case RGBFormat::R8G8B8: return "R8G8B8";
		case RGBFormat::B8G8R8: return "B8G8R8";
		case RGBFormat::A8R8G8B8: return "A8R8G8B8";
		case RGBFormat::A8B8G8R8: return "A8B8G8R8";
		case RGBFormat::R3G3B2: return "R3G3B2";
		case RGBFormat::A8: return "A8";
		case RGBFormat::A8R3G3B2: return "A8R3G3B2";
		case RGBFormat::X8R8G8B8: return "X8R8G8B8";
		case RGBFormat::X8B8G8R8: return "X8B8G8R8";
		case RGBFormat::GREY8: return "GREY8";
		case RGBFormat::GREYALPHA32: return "GREYALPHA32";
	}
	return "unknown";
}

/*!\brief Bittiefe des Farbformates
 *
 * \desc
 * Diese Funktion liefert die Anzahl Bits zurück, die zur Darstellung des gewählten
 * Farbformats erforderlich sind. Ist in der Regel 8, 16, 24, 32 oder 64. Die Funktion ist
 * identisch mit RGBFormat::bitsPerPixel.
 *
 * @return Anzahl erforderlicher Bits oder 0, wenn kein Farbformat gesetzt ist.
 */
int RGBFormat::bitdepth() const
{
	switch(f) {
		case RGBFormat::Palette:
		case RGBFormat::A8:
		case RGBFormat::R3G3B2:
		case RGBFormat::GREY8:
			return 8;
		case RGBFormat::R5G6B5:
		case RGBFormat::B5G6R5:
		case RGBFormat::X1R5G5B5:
		case RGBFormat::X1B5G5R5:
		case RGBFormat::X4R4G4B4:
		case RGBFormat::A1R5G5B5:
		case RGBFormat::A1B5G5R5:
		case RGBFormat::A4R4G4B4:
		case RGBFormat::A8R3G3B2:
			return 16;
		case RGBFormat::R8G8B8:
		case RGBFormat::B8G8R8:
			return 24;
		case RGBFormat::A8R8G8B8:
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8R8G8B8:
		case RGBFormat::X8B8G8R8:
		case RGBFormat::GREYALPHA32:
			return 32;
	};
	return 0;
}

/*!\brief Anzahl erforderlicher Bits zur Darstellung eines Pixels.
 *
 * \desc
 * Diese Funktion liefert die Anzahl Bits zurück, die zur Darstellung des gewählten
 * Farbformats erforderlich sind. Ist in der Regel 8, 16, 24, 32 oder 64. Die Funktion ist
 * identisch mit RGBFormat::bitdepth.
 *
 * @return Anzahl erforderlicher Bits oder 0, wenn kein Farbformat gesetzt ist.
 */
int RGBFormat::bitsPerPixel() const
{
	return bitdepth();
}

/*!\brief Anzahl Bytes pro Pixel
 *
 * \desc
 * Diese Funktion liefert die Anzahl Bytes zurück, die zur Darstellung des gewählten
 * Farbformats erforderlich sind. Ist in der Regel 1, 2, 3, 4 oder 8. Die Funktion ruft
 * intern RGBFormat::bitdepth auf und teilt das Ergebnis durch 8.
 *
 * @return Anzahl erforderlicher Bytes pro Pixel oder 0, wenn kein Farbformat gesetzt ist.
 */
int RGBFormat::bytesPerPixel() const
{
	return bitdepth()>>3;
}

/*!\brief Farbformat Anhand eines Identifiers setzten
 *
 * \desc
 * Mit diesem Operator wird das Farbformat anhand eines Wertes aus der Enumeration
 * RGBFormat::Identifier gesetzt.
 *
 * @param[in] id Wert aus der Enumeration RGBFormat::Identifier
 * @return Liefert eine Referenz auf die Klasse zurück.
 */
RGBFormat &RGBFormat::operator=(Identifier id)
{
	f=id;
	return *this;
}

/*!\brief Integer-Wert des Farbformats auslesen
 *
 * \desc
 * Mit diesem Operator wird der Integer-Wert des Farbformates ausgelesen.
 * Der Wert entspricht der Position des Farbformates innerhalb der Enumeration RGBFormat::Identifier.
 *
 * @return Integer-Wert, der das Farbformat repräsentiert.
 */
RGBFormat::operator int() const
{
	return f;
}

/*!\brief Zwei Farbformate vergleichen
 *
 * \desc
 * Mit dem Operatoren != und == können zwei Farbwerte miteinander verglichen werden.
 *
 * @param r1 Erstes Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @param r2 Zweites Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @return Wird == verwendet,
 * liefert die Funktion \c true zurück, wenn beide Farbformate identisch sind, sonst \c false. Wird !=
 * verwendet, liefert die Funktion \c true zurück, wenn die beiden Farbformate unterschiedlich sind, sonst
 * \c true.
 */
bool operator!= (const RGBFormat &r1, const RGBFormat &r2)
{
	if (r1.f!=r2.f) return true;
	return false;
}

/*!\brief Zwei Farbformate vergleichen
 *
 * \desc
 * Mit dem Operatoren != und == können zwei Farbwerte miteinander verglichen werden.
 *
 * @param r1 Erstes Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @param r2 Zweites Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @return Wird == verwendet,
 * liefert die Funktion \c true zurück, wenn beide Farbformate identisch sind, sonst \c false. Wird !=
 * verwendet, liefert die Funktion \c true zurück, wenn die beiden Farbformate unterschiedlich sind, sonst
 * \c true.
 */
bool operator== (const RGBFormat &r1, const RGBFormat &r2)
{
	if (r1.f==r2.f) return true;
	return false;
}

/*!\brief Zwei Farbformate vergleichen
 *
 * \desc
 * Mit dem Operatoren != und == können zwei Farbwerte miteinander verglichen werden.
 *
 * @param r1 Erstes Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @param r2 Zweites Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @return Wird == verwendet,
 * liefert die Funktion \c true zurück, wenn beide Farbformate identisch sind, sonst \c false. Wird !=
 * verwendet, liefert die Funktion \c true zurück, wenn die beiden Farbformate unterschiedlich sind, sonst
 * \c true.
 */
bool operator!= (const RGBFormat &r1, RGBFormat::Identifier r2)
{
	if (r1.f!=r2) return true;
	return false;
}

/*!\brief Zwei Farbformate vergleichen
 *
 * \desc
 * Mit dem Operatoren != und == können zwei Farbwerte miteinander verglichen werden.
 *
 * @param r1 Erstes Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @param r2 Zweites Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @return Wird == verwendet,
 * liefert die Funktion \c true zurück, wenn beide Farbformate identisch sind, sonst \c false. Wird !=
 * verwendet, liefert die Funktion \c true zurück, wenn die beiden Farbformate unterschiedlich sind, sonst
 * \c true.
 */
bool operator== (const RGBFormat &r1, RGBFormat::Identifier r2)
{
	if (r1.f==r2) return true;
	return false;
}

/*!\brief Zwei Farbformate vergleichen
 *
 * \desc
 * Mit dem Operatoren != und == können zwei Farbwerte miteinander verglichen werden.
 *
 * @param r1 Erstes Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @param r2 Zweites Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @return Wird == verwendet,
 * liefert die Funktion \c true zurück, wenn beide Farbformate identisch sind, sonst \c false. Wird !=
 * verwendet, liefert die Funktion \c true zurück, wenn die beiden Farbformate unterschiedlich sind, sonst
 * \c true.
 */
bool operator!= (RGBFormat::Identifier r1, const RGBFormat &r2)
{
	if (r1!=r2.f) return true;
	return false;
}

/*!\brief Zwei Farbformate vergleichen
 *
 * \desc
 * Mit dem Operatoren != und == können zwei Farbwerte miteinander verglichen werden.
 *
 * @param r1 Erstes Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @param r2 Zweites Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @return Wird == verwendet,
 * liefert die Funktion \c true zurück, wenn beide Farbformate identisch sind, sonst \c false. Wird !=
 * verwendet, liefert die Funktion \c true zurück, wenn die beiden Farbformate unterschiedlich sind, sonst
 * \c true.
 */
bool operator== (RGBFormat::Identifier r1, const RGBFormat &r2)
{
	if (r1==r2.f) return true;
	return false;
}


} // EOF namespace grafix
} // EOF namespace ppl7
