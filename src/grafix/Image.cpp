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

#include "ppl7.h"
#include "ppl7-grafix.h"
//#include "grafix6.h"


namespace ppl7 {
namespace grafix {

/*!\class Image
 * \ingroup PPLGroupGrafik
 * \brief Eine Bitmap-Grafik im Hauptspeicher
 *
 * \example
 * \code
 * #include <ppl7.h>
 * #include <ppl7-grafix.h>
 *
 * using namespace ppl7;
 * using namespace ppl7::grafix;
 *
 * int main(int argc, char **argv) {
 * 	Grafix gfx;									// Grafik-Engine initialisieren
 * 	Image Image(640,480,RGBFormat::A8R8G8B8);	// Image erzeugen
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
 * Mit diesem Konstruktor wird ein leeres Image erstellt. Bevor es verwendet werden kann, muss
 * zunächst mit Image::create eine neue Grafik erstellt, mit Image::copy eine Grafik von einem anderen
 * Image oder Drawable kopiert oder mit Image::load eine Grafik aus einer Datei oder einem
 * Speicherbereich geladen werden.
 *
 */
Image::Image() : Drawable()
{

}

/*!\brief Copy-Konstruktor
 *
 * \desc
 * Mit diesem Konstruktor wird eine Kopie eines anderen Image erstellt.
 *
 * @param other Anderes Image
 */
Image::Image(const Image &other) : Drawable()
{
	copy(other);
}

/*!\brief Copy-Konstruktor
 *
 * \desc
 * Mit diesem Konstruktor wird eine Kopie eines Drawable erstellt.
 *
 * @param other Ein Drawable, von dem kopiert werden soll
 */
Image::Image(const Drawable &other) : Drawable()
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
Image::Image(int width, int height, const RGBFormat &format) : Drawable()
{
	create(width,height,format);
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
Image::Image(const String &Filename, const RGBFormat &format) : Drawable()
{
	File ff;
	ff.open(Filename,File::READ);
	load(ff,format);
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
Image::Image(FileObject &file, const RGBFormat &format) : Drawable()
{
	load(file,format);
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
Image::Image(const ByteArrayPtr &mem, const RGBFormat &format) : Drawable()
{
	load(mem,format);
}

/*!\brief Destruktor der Klasse
 *
 * \desc
 * Der Destruktor sorgt dafür, dass der durch die Klasse allokierte Speicher wieder
 * freigegeben wird.
 *
 */
Image::~Image()
{
	// Passiert alles automatisch
}

/*!\brief Speicher freigeben
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der durch das Objekt belegte Speicher
 * freigegeben. Es darf anschließend nicht mehr für Zeichenoperationen verwendet
 * werden. Mit Image::create kann es reinitialisiert werden.
 */
void Image::clear()
{
	myMemory.clear();
	fn=NULL;
	memset(&data,0,sizeof(data));
}

/*!\brief Grafik von einem Drawable kopieren
 *
 * \desc
 * Grafik von einem anderen Drawable kopieren.
 *
 * @param other Anderes Drawable
 * @return Im Erfolgsfall gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
void Image::copy(const Drawable &other)
{
	memset(&data,0,sizeof(data));
	fn=NULL;
	if (other.isEmpty()) return;
	// Das andere Drawable kann auch einen Ausschnitt aus einem größeren Bild
	// repräsentieren, daher kopieren wir die Pixeldaten Zeilenweise
	size_t size=other.data.width*other.data.height*(other.data.rgbformat.bitdepth()/8);
	if (!size) throw EmptyDrawableException();
	data.base=myMemory.malloc(size);
	if (!data.base) throw OutOfMemoryException();
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
}

/*!\brief Grafikausschnitt von einem Drawable kopieren
 *
 * \desc
 * Grafikausschnitt von einem anderen Drawable kopieren.
 *
 * @param other Anderes Drawable
 * @param rect Gewünschter Ausschnitt
 * @return Im Erfolgsfall gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
void Image::copy(const Drawable &other, const Rect &rect)
{
	copy(Drawable(other,rect));
}

/*!\brief Grafik kopieren
 *
 * \desc
 * Grafik von einem anderen Image kopieren.
 *
 * @param other Anderes Image
 * @return Im Erfolgsfall gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
void Image::copy(const Image &other)
{
	memset(&data,0,sizeof(data));
	fn=NULL;
	if (other.isEmpty()) return;
	if (!myMemory.copy(other.myMemory)) {
		throw OutOfMemoryException();
	}
	memcpy(&data,&other.data,sizeof(data));
	fn=other.fn;
	data.base=(void*)myMemory.adr();
}

/*!\brief Neues Image erstellen
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
void Image::create(int width, int height, const RGBFormat &format)
{
	if (data.width==width && data.height==height && data.rgbformat==format) {
		myMemory.memset(0);
		return;
	}
	fn=NULL;
	memset(&data,0,sizeof(data));
	myMemory.free();

	if (format==RGBFormat::unknown || format>=RGBFormat::MaxIdentifiers) {
		throw UnknownColorFormatException();
	}
	if (width>65535 || height>65535 || width<1 || height<1) throw InvalidImageSizeException();
	size_t size=width*height*(format.bytesPerPixel());
	data.base=myMemory.calloc(size);
	if (!data.base) throw OutOfMemoryException();
	data.pitch=width*(format.bitdepth()/8);
	data.rgbformat=format;
	data.width=width;
	data.height=height;
	initFunctions(data.rgbformat);
}

/*!\brief Neues Image aus einem Speicherbereich erstellen
 *
 * \desc
 * Mit dieser Funktion wird eine neue Grafik anhand des angegebenen Speicherbereichs erstellt.
 * Dabei wird zunächst eine neues Image erstellt und die Bilddaten des angegebenen Speicherbereichs
 * hineinkopiert.
 *
 * @param base Speicheradresse
 * @param pitch Anzahl Bytes pro Bildzeile
 * @param width Breite der Grafik in Pixel
 * @param height Höhe der Grafik in Pixel
 * @param format Farbformat
 * @return Im Erfolgsfall liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
void Image::create(void *base, ppluint32 pitch, int width, int height, const RGBFormat &format)
{
	copy(Drawable(base,pitch,width,height,format));
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
void Image::load(const String &Filename, const RGBFormat &format)
{
	File ff;
	ff.open(Filename,File::READ);
	load(ff,format);
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
void Image::load(const ByteArrayPtr &Mem, const RGBFormat &format)
{
	MemFile ff(Mem);
	load(ff,format);
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
void Image::load(FileObject &file, const RGBFormat &format)
{
	Grafix *gfx=GetGrafix();
	IMAGE img;
	ImageFilter *filter=gfx->findImageFilter(file,img);
	if (format!=RGBFormat::unknown) img.format=format;

	create(img.width,img.height,img.format);
	filter->load(file,*this,img);
}

/*!\brief Anzahl Bytes, die durch diese Grafik belegt sind
 *
 * \desc
 * Diese Funktion liefert die Anzahl Bytes zurück, die durch diese Grafik belegt sind.
 *
 * @return Anzahl Bytes
 */
size_t Image::numBytes() const
{
	return myMemory.size();
}


/*!\brief Grafik kopieren
 *
 * \desc
 * Mit diesem Operator wird eine Kopie des angegebenen Drawable \p other erstellt.
 *
 * @param other Ein Drawable, von dem die Kopie angefertigt werden soll
 * @return Liefert eine Referenz auf dieses Image zurück.
 */
Image &Image::operator=(const Drawable &other)
{
	copy(other);
	return *this;
}

/*!\brief Grafik kopieren
 *
 * \desc
 * Mit diesem Operator wird eine Kopie des angegebenen Image \p other erstellt.
 *
 * @param other Ein Image, von dem die Kopie angefertigt werden soll
 * @return Liefert eine Referenz auf dieses Image zurück.
 */
Image &Image::operator=(const Image &other)
{
	copy(other);
	return *this;
}

/*!\brief Referenz auf den Speicherbereich der Grafik holen
 *
 * \desc
 * Mit dieser Funktion kann eine Referenz des Speicherbereich der Grafik
 * als ByteArrayPtr-Objekt geholt werden.
 *
 * @return ByteArrayPtr-Objekt mit referenz auf den Speicherbereich der Grafik
 */
ByteArrayPtr Image::memory() const
{
	return myMemory;
}

/*!\brief Referenz auf den Speicherbereich der Grafik holen
 *
 * \desc
 * Mit dieser Funktion kann eine Referenz des Speicherbereich der Grafik
 * als ByteArrayPtr-Objekt geholt werden.
 *
 * @return ByteArrayPtr-Objekt mit referenz auf den Speicherbereich der Grafik
 */
Image::operator ByteArrayPtr() const
{
	return myMemory;
}


} // EOF namespace grafix
} // EOF namespace ppl7
