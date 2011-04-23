/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.1 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: CDrawable.cpp,v 1.1 2010/02/12 19:43:48 pafe Exp $
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

#include "ppl6.h"
#include "ppl6-grafix.h"
#include "grafix6.h"

//#undef HAVE_X86_ASSEMBLER

namespace ppl6 {
namespace grafix {


/*!\class CDrawable
 * \ingroup PPLGroupGrafik
 * \brief Basisklasse für Zeichenflächen
 *
 * \desc
 * Die Klasse CDrawable ist die Basisklasse für alle Grafikoperationen. Hier finden
 * sich Funktionen zum Zeichnen von Pixeln, Linien und andere geometrische Formen,
 * Bilder und Text. Sie verwaltet selbst keinen Speicher, sondern enthält nur einen Pointer
 * auf einen Speicherbereich, Anzahl Bytes pro Zeile sowie Breite und Höhe in Pixel und
 * das Farbformat.
 * \par
 * Um die Klasse verwenden zu können, muss sie zunächst mit der Funktion CDrawable::create
 * oder über den passenden Konstruktor initialisiert werden. Oft erhält man die Klasse aber
 * auch durch Aufruf einer Funktion. Die Klasse CImage ist von CDrawable abgeleitet und verwaltet
 * den für die Grafik benötigten Speicher.
 * \par
 * Ein CDrawable repräsentiert immer eine 2-Dimensionale Grafik mit einer horizontalen
 * X-Achse und einer vertikalen Y-Achse. Die Koordinaten (0/0) befinden sich in der linken
 * oberen Ecke und werden nach rechts, bzw. unten grösser.
 * \par
 * \image html CDrawable.png
 * \image latex CDrawable.png
 * \par
 * Eine Grafik mit einer Größe von 640 x 480 Pixeln hat somit Werte von 0 bis 639 auf
 * der X-Achse und 0 bis 479 auf der Y-Achse.
 *
 * \attention
 * Ein CDrawable kann beliebig kopiert werden. Dabei wird jedoch nur ein Pointer auf
 * den ursprünglichen Speicherbereich kopiert, nicht aber die Grafik selbst. Man muss
 * daher darauf achten, dass die ursprüngliche Speicher-verwaltende Klasse nicht
 * gelöscht wird, solange davon kopierte CDrawable-Klassen noch verwendet werden.
 * Ist dies ungewiss, sollte man stattdessen in ein CImage kopieren, wobei eine
 * echte Kopie des Originals erzeugt wird.
 * \par
 * \code
 * CImage Img(640,480,RGBFormat::A8R8G8B8);		// Bild erstellen
 * Img.CLS(Color(0,0,0));						// Bild komplett schwarz
 * // Bildausschnitt in ein CDrawable holen
 * CDrawable Draw=Img.getDrawable(Rect(100,50,300,200));
 * // Einen roten Rahmen um den Bildausschnitt zeichnen
 * Draw.drawRect(Draw.rect(),Color(255,0,0));
 * // Neue Grafik erstellen, dadurch wird der bisherige Speicher freigegeben
 * Img.create(320,200,,RGBFormat::A8R8G8B8);
 * // Achtung: Draw zeigt nun auf ungültigen Speicher und darf nicht mehr
 * // verwendet werden!
 * \endcode
 *
 * \example
 * Das folgende Beispiel zeigt, wie die oben zu sehende Grafik erzeugt und als PNG
 * gespeichert wird.
 * \include CDrawable.cpp
 */

/*!\class Konstruktor
 *
 * \desc
 * Mit diesem Konstruktor wird ein leeres CDrawable erstellt. Bevor es verwendet werden
 * kann, muss zunächst mit CDrawable::copy eine Kopie eines anderen CDrawable oder davon
 * abgeleiteten Objekts erstellt werden oder mit CDrawable::create ein neues CDrawable
 * anhand eines Speicherbereichs erstellt werden.
 */
CDrawable::CDrawable()
{
	memset(&data,0,sizeof(data));
	fn=NULL;
}

/*!\brief Copy-Konstruktor
 *
 * \desc
 * Mit diesem Konstruktor wird eine Kopie eines anderen CDrawable oder davon abgeleiteten
 * Klasse erstellt.
 *
 * @param other Ein anderes CDrawable
 */
CDrawable::CDrawable(const CDrawable &other)
{
	memcpy(&data,&other.data,sizeof(data));
	fn=other.fn;
}

/*!\brief Copy-Konstruktor mit Bildausschnitt
 *
 * \desc
 * Mit diesem Konstruktor wird der Bildausschnitt \p rect aus dem CDrawable \p other
 * kopiert.
 *
 * @param other Ein anderes CDrawable
 * @param rect Bildausschnitt
 *
 * \exception ppl6::Exception::InitialisationFailed Diese Exception wird geworfen,
 * wenn das CDrawable nicht kopiert werden kann. Die genaue Ursache kann über den Fehlercode
 * ausgelesen werden.
 */
CDrawable::CDrawable(const CDrawable &other, const Rect &rect)
{
	if (!copy(other,rect)) throw ppl6::Exception::InitialisationFailed();
}

/*!\brief Konstruktor mit Erstellung aus einem Speicherbereich
 *
 * \desc
 * Mit diesem Konstruktor wird ein neues CDrawable anhand einer Speicheradresse und
 * weiterer Parameter erstellt.
 *
 * @param base Speicheradresse
 * @param pitch Anzahl Bytes pro Bildzeile
 * @param width Breite der Grafik in Pixel
 * @param height Höhe der Grafik in Pixel
 * @param format Farbformat
 *
 * \exception ppl6::Exception::InitialisationFailed Diese Exception wird geworfen,
 * wenn das CDrawable nicht mit den angegebenen Parametern erstellt werden kann.
 * Die genaue Ursache kann über den Fehlercode ausgelesen werden.
 */
CDrawable::CDrawable(void *base, ppluint32 pitch, int width, int height, const RGBFormat &format)
{
	memset(&data,0,sizeof(data));
	fn=NULL;
	if (!create(base,pitch,width,height,format)) throw ppl6::Exception::InitialisationFailed();
}

/*!\brief Destruktor der Klasse
 *
 * \desc
 * Da die Klasse selbst keinen Speicher verwaltet, hat der Destruktor gegenwärtig keine
 * Funktion.
 */
CDrawable::~CDrawable()
{

}

/*!\brief Pointer auf die Grafik-Funktionen für das Farbformat dieses CDrawable holen
 *
 * \desc
 * Diese Funktion liefert einen Pointer auf die Grafik-Funktionen für das Farbformat
 * dieses CDrawable zurück. Dieser kann NULL sein, wenn das CDrawable noch nicht
 * initialisiert wurde oder das angegebene Farbformat nicht unterstützt wird.
 *
 * @return Pointer auf eine GRAFIX_FUNCTIONS Struktur oder NULL. Es wird kein
 * Fehlercode gesetzt.
 */
GRAFIX_FUNCTIONS *CDrawable::getFunctions()
{
	return fn;
}

/*!\brief Pointer auf die Datenstruktur des CDrawable holen
 *
 * \desc
 * Diese Funktion liefert einen Pointer auf die Struktur DRAWABLE_DATA des
 * CDrawable zurück. Diese enthält alle Informationen des CDrawable, wie den
 * Speicherbereich der Grafik, Breite und Höhe, Bytes pro Zeile (pitch) und das
 * Farbformat.
 *
 * @return Pointer auf die DRAWABLE_DATA Struktur des CDrawable.
 */
DRAWABLE_DATA *CDrawable::getData()
{
	return &data;
}

/*!\brief Farbformatabhängige Grafik-Funktionen initialisieren
 *
 * \desc
 * Diese private Funktion wird intern bei der Erstellung des CDrawable aufgerufen,
 * um die farbformatabhängigen Grafik-Funktionen zu initialisieren.
 *
 * @param format Das gewünschte Farbformat
 * @return Im Erfolgsfall liefert die Funktion 1 zurück, im Fehlerfall 0. Es wird
 * ein entsprechender Fehlercode gesetzt.
 */
int CDrawable::initFunctions(const RGBFormat &format)
{
	CGrafix *gfx=GetGrafix();
	if (!gfx) return 0;
	fn=data.fn=gfx->GetGrafixFunctions(format);
	if (!data.fn) return 0;
	return 1;
}

/*!\brief Grafik von einem anderen CDrawable kopieren
 *
 * \desc
 * Mit dieser Funktion wird eine Kopie eines anderen CDrawable erstellt.
 *
 * @param other Ein anderes CDrawable
 * @return Im Erfolgsfall liefert die Funktion 1 zurück, im Fehlerfall 0. Es wird
 * ein entsprechender Fehlercode gesetzt.
 */
int CDrawable::copy(const CDrawable &other)
{
	memcpy(&data,&other.data,sizeof(data));
	fn=other.fn;
	return 1;
}

/*!\brief Ausschnitt von einem anderen CDrawable kopieren
 *
 * \desc
 * Mit dieser Funktion wird der Bildausschnitt \p rect aus dem CDrawable \p other
 * kopiert.
 *
 * @param other Ein anderes CDrawable
 * @param rect Bildausschnitt
 *
 * @return Im Erfolgsfall liefert die Funktion 1 zurück, im Fehlerfal 0.
 */
int CDrawable::copy(const CDrawable &other, const Rect &rect)
{
	Rect o(0,0,other.data.width,other.data.height);
	Rect r=o.intersected(rect);
	memset(&data,0,sizeof(data));
	data.base8=other.data.base8
		+r.top()*other.data.pitch
		+r.left()*other.data.rgbformat.bytesPerPixel();
	data.pitch=other.data.pitch;
	data.width=r.width();
	data.height=r.height();
	data.rgbformat=other.data.rgbformat;
	fn=other.fn;
	data.fn=fn;
	return 1;
}

/*!\brief CDrawable anhand einer Speicheradresse initialisieren
 *
 * \desc
 * Mit dieser Funktion wird das CDrawable anhand einer Speicheradresse und
 * weiterer Parameter initialisiert.
 *
 * @param base Speicheradresse
 * @param pitch Anzahl Bytes pro Bildzeile
 * @param width Breite der Grafik in Pixel
 * @param height Höhe der Grafik in Pixel
 * @param format Farbformat
 *
 * \return Im Erfolgsfall gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CDrawable::create(void *base, ppluint32 pitch, int width, int height, const RGBFormat &format)
{
	if (!base) {
		ppl6::SetError(194,"CDrawable::create: *base zeigt auf NULL");
		return 0;
	}
	if (!pitch) {
		ppl6::SetError(194,"CDrawable::create: pitch darf nicht 0 sein");
		return 0;
	}
	if (width==0 || height==0) {
		ppl6::SetError(194,"CDrawable::create: width und height dürfen nicht 0 sein");
		return 0;
	}
	data.base=base;
	data.pitch=pitch;
	data.width=width;
	data.height=height;
	data.rgbformat=format;
	return initFunctions(format);
}


/*!\brief CDrawable kopieren
 *
 * \desc
 * Mit diesem Operator wird eine Kopie eines anderen CDrawable erstellt.
 *
 * @param other Anderes CDrawable
 * @return Referenz auf das neue CDrawable
 */
CDrawable &CDrawable::operator=(const CDrawable &other)
{
	memcpy(&data,&other.data,sizeof(data));
	fn=other.fn;
	return *this;
}

/*!\brief Rechteck des CDrawable auslesen
 *
 * \desc
 * Dieser Funktion liefert das Rechteck des CDrawable zurück. Die Koordinaten des
 * Rechtecks sind immer 0/0, Breite und Höhe sind die des CDrawable.
 *
 * @return Ein Objekt von Typ Rect.
 */
Rect CDrawable::rect() const
{
	return Rect(0,0,data.width,data.height);
}

/*!\brief Größe des CDrawable auslesen
 *
 * \desc
 * Diese Funktion liefert Breite und Höhe des CDrawable in einem Size-Objekt zurück.
 *
 * @return Objekt von Typ Size.
 */
Size CDrawable::size() const
{
	return Size(data.width,data.height);
}

/*!\brief Enthält dieses CDrawable eine Grafik?
 *
 * \desc
 * Mit dieser Funktion kann geprüft werden, ob das CDrawable eine gültige und somit nutzbare
 * Grafik enthält. Dies ist der Fall, wenn eine Basisadresse vorhanden ist, die Grafik eine
 * Höhe und Breite größer 0 und ein gültiges Farbformat enthält.
 *
 * @return Liefert \c true zurück, wenn das CDrawable eine gültige Grafik enthält, andernfalls
 * \c false.
 */
bool CDrawable::isEmpty() const
{
	if (data.base==0 || data.width==0 || data.height==0) return true;
	return false;
}

/*!\brief Farbformat des CDrawable auslesen
 *
 * \desc
 * Diese Funktion liefert das Farbformat des CDrawable zurück.
 *
 * @return Farbformat
 */
RGBFormat CDrawable::rgbformat() const
{
	return data.rgbformat;
}

/*!\brief Breite der Grafik in Pixel
 *
 * \desc
 * Diese Funktion liefert die Breite der Grafik in Pixel zurück.
 *
 * @return Breite in Pixel
 */
int CDrawable::width() const
{
	return data.width;
}

/*!\brief Höhe der Grafik in Pixel
 *
 * \desc
 * Diese Funktion liefert die Höhe der Grafik in Pixel zurück.
 *
 * @return Höhe in Pixel
 */
int CDrawable::height() const
{
	return data.height;
}

/*!\brief Bytes pro Zeile
 *
 * \desc
 * Mit dieser Funktion kann abgefragt werden, wieviele Bytes eine Grafikzeile benötigt.
 * Dies muss nicht unbedingt das Ergebnis von Breite mal BytesProPixel sein, sondern kann
 * auch mehr sein, z.B. wenn das CDrawable nur einen Ausschnitt aus einer größeren Grafik
 * enthält.
 *
 * @return Bytes pro Grafikzeile
 */
int CDrawable::pitch() const
{
	return data.pitch;
}

/*!\brief Bytes pro Pixel
 *
 * \desc
 * Mit dieser Funktion kann ausgelesen werden, wieviele Bytes ein einzelner Pixel
 * benötigt.
 *
 * @return Bytes pro Pixel
 */
int CDrawable::bytesPerPixel() const
{
	return data.rgbformat.bitdepth()/8;
}

/*!\brief Bittiefe der Grafik (Bits pro Pixel)
 *
 * \desc
 * Diese Funktion liefert die Anzahl Bits pro Pixel für das verwendete Farbformat zurück.
 *
 * @return Bits pro Pixel
 */
int CDrawable::bitdepth() const
{
	return data.rgbformat.bitdepth();
}

/*!\brief Ein neues CDrawable erstellen
 *
 * \desc
 * Diese Funktion liefert eine Kopie dieses CDrawable zurück.
 *
 * @return Kopie des CDrawable
 */
CDrawable CDrawable::getDrawable() const
{
	return *this;
}

/*!\brief Ein neues CDrawable anhand eines Ausschnitts erstellen
 *
 * \desc
 * Diese Funktion erstellt ein neues CDrawable Objekt anhand des angegebenen Ausschnitts \p rect.
 *
 * @param rect Der gewünschte Bildausschnitt
 * @return Neues CDrawable, das den gewünschten Ausschnitt repräsentiert
 */
CDrawable CDrawable::getDrawable(const Rect &rect) const
{
	return CDrawable(*this,rect);
}

/*!\brief Ein neues CDrawable anhand eines Ausschnitts erstellen
 *
 * \desc
 * Diese Funktion erstellt ein neues CDrawable Objekt anhand des angegebenen Koordinaten. Dabei
 * ist zu beachten, dass der Ausschnitt inklusive der rechten unteren Ecke erstellt wird.
 *
 * @param x1 X-Koordinate der linken oberen Ecke
 * @param y1 Y-Koordinate der linken oberen Ecke
 * @param x2 X-Koordinate der rechten unteren Ecke
 * @param y2 Y-Koordinate der rechten unteren Ecke
 * @return Neues CDrawable, das den gewünschten Ausschnitt repräsentiert
 */
CDrawable CDrawable::getDrawable(int x1, int y1, int x2, int y2) const
{
	return CDrawable(*this,Rect(x1,y1,x2-x1+1,y2-y1+1));
}

/*!\brief Speicheradresse der Grafik
 *
 * \desc
 * Diese Funktion gibt die Speicheradresse zurück, an der die Daten dieser Grafik
 * beginnen.
 *
 * \return Speicheradresse
 */
void *CDrawable::adr() const
{
	return data.base;
}

/*!\brief Speicheradresse der angegebenen Koordinaten
 *
 * \desc
 * Diese Funktion gibt die Speicheradresse zurück, an der sich die angegebenen
 * Koordinaten \p x und \p y befinden.
 * @param x X-Koordinate
 * @param y Y-Koordinate
 * \return Speicheradresse
 */
void *CDrawable::adr(int x, int y) const
{
	if (x<data.width && y<data.height) return data.base8+(y*data.pitch)+(x*data.rgbformat.bitdepth()/8);
	SetError(1070);
	return NULL;
}


} // EOF namespace grafix
} // EOF namespace ppl6
