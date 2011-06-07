/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
 *
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
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

#include "ppl7.h"
#include "ppl7-grafix.h"
//#include "grafix6.h"


namespace ppl6 {
namespace grafix {

/*!\class CImage
 * \ingroup PPLGroupGrafik
 * \brief Eine Bitmap-Grafik im Hauptspeicher
 *
 * \example
 * \code
 * #include <ppl6.h>
 * #include <ppl6-grafix.h>
 *
 * using namespace ppl6;
 * using namespace ppl6::grafix;
 *
 * int main(int argc, char **argv) {
 * 	CGrafix gfx;									// Grafik-Engine initialisieren
 * 	CImage Image(640,480,RGBFormat::A8R8G8B8);	// Image erzeugen
 * 	Image.CLS(Color(255,255,255));				// Weisser Hintergrund
 * 	Image.DrawRect(Rect(10,10,100,100),Color(0));	// Schwarzes Rechteck mit einer Breite und Höhe von 100 Pixel
 * 	CFont Font;
 * 	Font.setAntialias(true);
 * 	Image.Print(Font,15,20,"Hello World");
 * 	CFilter_PNG PNG;
 * 	PNG.SaveFile(Image,"testbild.png");
 * 	return 0;
 * }
 * \endcode
 */

/*!\brief Konstruktor der Klasse
 *
 * \desc
 * Mit diesem Konstruktor wird ein leeres CImage erstellt. Bevor es verwendet werden kann, muss
 * zunächst mit CImage::create eine neue Grafik erstellt, mit CImage::copy eine Grafik von einem anderen
 * CImage oder CDrawable kopiert oder mit CImage::load eine Grafik aus einer Datei oder einem
 * Speicherbereich geladen werden.
 *
 */
CImage::CImage()
{
	memset(&data,0,sizeof(data));
	fn=NULL;
}

/*!\brief Copy-Konstruktor
 *
 * \desc
 * Mit diesem Konstruktor wird eine Kopie eines anderen CImage erstellt.
 *
 * @param other Anderes CImage
 */
CImage::CImage(const CImage &other)
{
	copy(other);
}

/*!\brief Copy-Konstruktor
 *
 * \desc
 * Mit diesem Konstruktor wird eine Kopie eines CDrawable erstellt.
 *
 * @param other Ein CDrawable, von dem kopiert werden soll
 */
CImage::CImage(const CDrawable &other)
{
	copy(other);
}

/*!\brief Erstellen einer neuen Grafik
 *
 * \desc
 * Mit dieser Funktion wird eine neue Grafik mit der Breite \p width, der Höhe \p height und dem
 * Farbformat \p format erstellt.
 *
 * @param width Breite in Pixel
 * @param height Höhe in Pixel
 * @param format gewünschtes Farbformat
 * \exception Exception::InitialisationFailed Diese Exception wird geworfen, wenn die Grafik
 * nicht erstellt werden konnte. Die genaue Ursache kann über die Fehlercodes ausgelesen werden.
 */
CImage::CImage(int width, int height, const RGBFormat &format)
{
	memset(&data,0,sizeof(data));
	fn=NULL;
	if (!create(width,height,format)) throw Exception();
}

/*!\brief Grafik aus einer Datei laden
 *
 * \desc
 * Mit diesem Konstruktor wird eine Grafik aus einer Datei geladen.
 *
 * @param Filename Der Dateiname
 * @param format Optionales Farbformat. Falls nicht angegeben, wird das Farbformat der Grafikdatei
 * verwendet. Andernfalls werden die Originalfarben der Grafikdatei in das angegebene Format konvertiert.
 * @exception Exception::FileOpenFailed Diese Exception wird geworfen, wenn die
 * angegebene Datei nicht geöffnet werden konnte.
 * @exception Exception::ImageLoadFailed Diese Exception wird geworfen, wenn die Grafik
 * nicht geladen werden konnte. Die genaue Ursache kann über die Fehlercodes ausgelesen werden.
 */
CImage::CImage(const CString &Filename, const RGBFormat &format)
{
	CFile ff;
	if (!ff.Open(Filename,"rb")) throw Exception();
	if (!load(ff,format)) throw Exception();
}

/*!\brief Grafik aus einer geöffneten Datei laden
 *
 * \desc
 * Mit diesem Konstruktor wird eine Grafik aus einer geöffneten Datei geladen.
 *
 * @param file Geöffnete Datei
 * @param format Optionales Farbformat. Falls nicht angegeben, wird das Farbformat der Grafikdatei
 * verwendet. Andernfalls werden die Originalfarben der Grafikdatei in das angegebene Format konvertiert.
 * @exception Exception::ImageLoadFailed Diese Exception wird geworfen, wenn die Grafik
 * nicht geladen werden konnte. Die genaue Ursache kann über die Fehlercodes ausgelesen werden.
 */
CImage::CImage(CFileObject &file, const RGBFormat &format)
{
	if (!load(file,format)) throw Exception();
}

/*!\brief Grafik aus einem Speicherbereich laden
 *
 * \desc
 * Mit diesem Konstruktor wird eine Grafik aus einem Speicherbereich geladen.
 *
 * @param mem Speicherbereich
 * @param format Optionales Farbformat. Falls nicht angegeben, wird das Farbformat der Grafikdatei
 * verwendet. Andernfalls werden die Originalfarben der Grafikdatei in das angegebene Format konvertiert.
 * @exception Exception::ImageLoadFailed Diese Exception wird geworfen, wenn die Grafik
 * nicht geladen werden konnte. Die genaue Ursache kann über die Fehlercodes ausgelesen werden.
 */
CImage::CImage(const CMemoryReference &mem, const RGBFormat &format)
{
	if (!load(mem,format)) throw Exception();
}

/*!\brief Destruktor der Klasse
 *
 * \desc
 * Der Destruktor sorgt dafür, dass der durch die Klasse allokierte Speicher wieder
 * freigegeben wird.
 *
 */
CImage::~CImage()
{
	// Passiert alles automatisch
}

/*!\brief Grafik von einem CDrawable kopieren
 *
 * \desc
 * Grafik von einem anderen CDrawable kopieren.
 *
 * @param other Anderes CDrawable
 * @return Im Erfolgsfall gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CImage::copy(const CDrawable &other)
{
	memset(&data,0,sizeof(data));
	fn=NULL;
	// Das andere CDrawable kann auch einen Ausschnitt aus einem größeren Bild
	// repräsentieren, daher kopieren wir die Pixeldaten Zeilenweise
	size_t size=other.data.width*other.data.height*(other.data.rgbformat.bitdepth()/8);
	if (!size) {
		SetError(1067);
		return 0;
	}
	data.base=myMemory.malloc(size);
	if (!data.base) {
		SetError(2);
		return 0;
	}
	data.fn=other.data.fn;
	fn=other.fn;
	data.width=other.data.width;
	data.height=other.data.height;
	data.rgbformat=other.data.rgbformat;
	data.pitch=data.width*(data.rgbformat.bitdepth()/8);
	// Jetzt die Pixel kopieren
	ppluint8 *qq=other.data.base8;
	ppluint8 *tt=data.base8;
	for (int y=0;y<data.height;y++) {
		memcpy(tt,qq,data.pitch);
		qq+=other.data.pitch;
		tt+=data.pitch;
	}
	return 1;
}

/*!\brief Grafikausschnitt von einem CDrawable kopieren
 *
 * \desc
 * Grafikausschnitt von einem anderen CDrawable kopieren.
 *
 * @param other Anderes CDrawable
 * @param rect Gewünschter Ausschnitt
 * @return Im Erfolgsfall gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CImage::copy(const CDrawable &other, const Rect &rect)
{
	return copy(CDrawable(other,rect));
}

/*!\brief Grafik kopieren
 *
 * \desc
 * Grafik von einem anderen CImage kopieren.
 *
 * @param other Anderes CImage
 * @return Im Erfolgsfall gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CImage::copy(const CImage &other)
{
	if (!myMemory.copy(other.myMemory)) {
		memset(&data,0,sizeof(data));
		fn=NULL;
		return 0;
	}
	memcpy(&data,&other.data,sizeof(data));
	fn=other.fn;
	data.base=myMemory.adr();
	return 1;
}

/*!\brief Neues CImage erstellen
 *
 * \desc
 * Mit dieser Funktion wird eine neue Grafik mit der Größe \p width * \p height und dem
 * Farbformat \p format erstellt.
 *
 * @param width Breite der Grafik
 * @param height Höhe der Grafik
 * @param format Farbformat der Grafik
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
int CImage::create(int width, int height, const RGBFormat &format)
{
	memset(&data,0,sizeof(data));
	myMemory.free();

	if (format==RGBFormat::unknown) {
		SetError(1065);
		return 0;
	}
	if (width>65535 || height>65535) {
		SetError(1076);
	}
	size_t size=width*height*(format.bytesPerPixel());
	if (!size) {
		SetError(1076);
		return 0;
	}
	data.base=myMemory.calloc(size);
	if (!data.base) {
		SetError(2);
		return 0;
	}
	data.pitch=width*(format.bitdepth()/8);
	data.rgbformat=format;
	data.width=width;
	data.height=height;
	if (!initFunctions(data.rgbformat)) {
		PushError();
		memset(&data,0,sizeof(data));
		myMemory.free();
		PopError();
		return 0;
	}
	return 1;
}

/*!\brief Neues CImage aus einem Speicherbereich erstellen
 *
 * \desc
 * Mit dieser Funktion wird eine neue Grafik anhand des angegebenen Speicherbereichs erstellt.
 * Dabei wird zunächst eine neues CImage erstellt und die Bilddaten des angegebenen Speicherbereichs
 * hineinkopiert.
 *
 * @param base Speicheradresse
 * @param pitch Anzahl Bytes pro Bildzeile
 * @param width Breite der Grafik in Pixel
 * @param height Höhe der Grafik in Pixel
 * @param format Farbformat
 * @return Im Erfolgsfall liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
int CImage::create(void *base, ppluint32 pitch, int width, int height, const RGBFormat &format)
{
	return copy(CDrawable(base,pitch,width,height,format));
}

/*!\brief Grafik aus einer Datei laden
 *
 * \desc
 * Mit dieser Funktion wird eine Grafik aus einer Datei geladen.
 *
 * @param Filename Der Dateiname
 * @param format Optionales Farbformat. Falls nicht angegeben, wird das Farbformat der Grafikdatei
 * verwendet. Andernfalls werden die Originalfarben der Grafikdatei in das angegebene Format konvertiert.
 * @return Im Erfolgsfall liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
int CImage::load(const CString &Filename, const RGBFormat &format)
{
	CFile ff;
	if (!ff.Open(Filename,"rb")) return 0;
	return load(ff,format);
}

/*!\brief Grafik aus einem Speicherbereich laden
 *
 * \desc
 * Mit dieser Funktion wird eine Grafik aus einem Speicherbereich geladen.
 *
 * @param Mem Referenz auf einen Speicherbereich
 * @param format Optionales Farbformat. Falls nicht angegeben, wird das Farbformat der Grafikdatei
 * verwendet. Andernfalls werden die Originalfarben der Grafikdatei in das angegebene Format konvertiert.
 * @return Im Erfolgsfall liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
int CImage::load(const CMemoryReference &Mem, const RGBFormat &format)
{
	CMemFile ff(Mem);
	return load(ff,format);
}

/*!\brief Grafik aus einer geöffneten Datei laden
 *
 * \desc
 * Mit dieser Funktion wird eine Grafik aus einer geöffneten Datei geladen.
 *
 * @param file Referenz auf eine bereits geöffnete Datei.
 * @param format Optionales Farbformat. Falls nicht angegeben, wird das Farbformat der Grafikdatei
 * verwendet. Andernfalls werden die Originalfarben der Grafikdatei in das angegebene Format konvertiert.
 * @return Im Erfolgsfall liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
int CImage::load(CFileObject &file, const RGBFormat &format)
{
	CGrafix *gfx=GetGrafix();
	if (!gfx) return 0;
	IMAGE img;
	CFilter *filter=gfx->FindFilter(file,img);
	if (!filter) return 0;
	if (format!=RGBFormat::unknown) img.format=format;
	if (!create(img.width,img.height,img.format)) return 0;
	if (!filter->Load(file,*this,img)) return 0;
	return 1;
}

/*!\brief Anzahl Bytes, die durch diese Grafik belegt sind
 *
 * \desc
 * Diese Funktion liefert die Anzahl Bytes zurück, die durch diese Grafik belegt sind.
 *
 * @return Anzahl Bytes
 */
size_t CImage::numBytes() const
{
	return myMemory.size();
}


/*!\brief Grafik kopieren
 *
 * \desc
 * Mit diesem Operator wird eine Kopie des angegebenen CDrawable \p other erstellt.
 *
 * @param other Ein CDrawable, von dem die Kopie angefertigt werden soll
 * @return Liefert eine Referenz auf dieses CImage zurück.
 */
CImage &CImage::operator=(const CDrawable &other)
{
	copy(other);
	return *this;
}

/*!\brief Grafik kopieren
 *
 * \desc
 * Mit diesem Operator wird eine Kopie des angegebenen CImage \p other erstellt.
 *
 * @param other Ein CImage, von dem die Kopie angefertigt werden soll
 * @return Liefert eine Referenz auf dieses CImage zurück.
 */
CImage &CImage::operator=(const CImage &other)
{
	copy(other);
	return *this;
}

/*!\brief Referenz auf den Speicherbereich der Grafik holen
 *
 * \desc
 * Mit dieser Funktion kann eine Referenz des Speicherbereich der Grafik
 * als CMemoryReference-Objekt geholt werden.
 *
 * @return CMemoryReference-Objekt mit referenz auf den Speicherbereich der Grafik
 */
CMemoryReference CImage::memory() const
{
	return myMemory;
}

/*!\brief Referenz auf den Speicherbereich der Grafik holen
 *
 * \desc
 * Mit dieser Funktion kann eine Referenz des Speicherbereich der Grafik
 * als CMemoryReference-Objekt geholt werden.
 *
 * @return CMemoryReference-Objekt mit referenz auf den Speicherbereich der Grafik
 */
CImage::operator CMemoryReference() const
{
	return myMemory;
}


} // EOF namespace grafix
} // EOF namespace ppl6
