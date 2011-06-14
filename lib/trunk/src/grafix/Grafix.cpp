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
#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include "ppl7.h"
#include "ppl7-grafix.h"

namespace ppl7 {
namespace grafix {

/*!\class Grafix
 * \ingroup PPLGroupGrafik
 * \brief Hauptklasse für alle Grafik-Operationen
 *
 * Diese Klasse wird für alle grafischen Funktionen benötigt und muß einmalig
 * von der Anwendung instanziiert und Initialisiert werden. Beim Versuch eine zweite
 * Instanz anzulegen, wird eine Exception geworfen.
 *
 * \par Grafikengine
 * Ohne weiteres Zutun verwendet die PPL-Library nur Software-Routinen für die
 * Grafik-Funktionen. Die Darstellung von Fenstern oder Vollbildanwendungen sind
 * nicht möglich. Falls dies gewünscht ist, muss die Anwendung zunächst die
 * gewünschte Grafik-Engine initialisieren und mit Grafix::UseEngine anmelden.
 * Siehe dazu auch die Basisklasse für Grafikengines GFXEngine, sowie die
 * davon abgeleiteten Klassen GFXEngineDX9 und GFXEngineSDL.
 *
 * \par Grafik-Filter
 * Ein Grafik-Filter ist eine von CFilter abgeleitete Klasse, mit der man ein bestimmtes
 * Grafikformat lesen oder schreiben kann. Die PPL-Library bringt bereits Filter für
 * die Format PNG, JPEG, GIF, BMP, PPM und TGA mit, die bei Initialisierung von Grafix
 * automatisch eingebunden werden. Bringt die Anwendung weitere Filter mit, müssen
 * diese mit Grafix::AddFilter registriert werden.
 *
 * \par Fonts
 * Genau wie bei den Grafikfiltern gibt es auch Klassen für verschiedene Font-Formate.
 * Die Basisklasse hierfür ist CFontEngine. PPL bringt bereits eine Engine für das eigene
 * Font5-Format mit, sowie für die FreeType-Library, mit der unter anderem Fonts im TrueType
 * oder OpenType Format dargestellt werden können. Um weitere Engines hinzuzufügen,
 * muss die Funktion Grafix::AddFontEngine aufgerufen werden.
 * \par
 * Standardmäßig sind ist der freie Font "Liberation Sans" und "Liberation Mono" im
 * Font5-Format in einigen Auflösungen enthalten. Um weitere Fonts zu laden muss
 * die Funktion Grafix::LoadFont verwendet werden.
 *
 * \exception Exception::ExistingGrafixInstance Wird geworfen, wenn versucht wird eine zweite
 * Instanz der Klasse zu erstellen
 */

static Grafix *pplgfx=NULL;
char *alphatab=NULL;

static GRAFIX_FUNCTIONS functions[RGBFormat::MaxIdentifiers];


Grafix *GetGrafix()
{
	if (pplgfx) return pplgfx;
	throw NoGrafixEngineException();
}


Grafix::Grafix()
{
	if (pplgfx) {
		throw DuplicateGrafixEngineException();
	}
	alphatab=NULL;
	filter_png=NULL;
	filter_jpeg=NULL;
	filter_bmp=NULL;
	filter_gif=NULL;
	filter_ppm=NULL;
	filter_tga=NULL;
	filter_magick=NULL;
	filter_tiff=NULL;
	initAlphatab();

	// Farbformat-abhängige Funktionen initialisieren
	for (int i=0;i<RGBFormat::MaxIdentifiers;i++) {
		try {
			initFunctions(RGBFormat((RGBFormat::Identifier)i),&functions[i]);
		} catch (UnsupportedColorFormatException) {

		}
	}

	pplgfx=this;


	// Standardfilter anlegen
#ifdef HAVE_IMAGEMAGICK
	/*
	filter_magick=new ImageFilter_ImageMagick;
	addImageFilter(filter_magick);
	*/
#endif


	filter_bmp=new ImageFilter_BMP;
	addImageFilter(filter_bmp);
	filter_gif=new ImageFilter_GIF;
	addImageFilter(filter_gif);

#ifdef HAVE_PNG
	filter_png=new ImageFilter_PNG;
	addImageFilter(filter_png);
#endif
#ifdef HAVE_JPEG
	filter_jpeg=new ImageFilter_JPEG;
	addImageFilter(filter_jpeg);
#endif
#ifdef HAVE_TIFF
	filter_tiff=new ImageFilter_TIFF;
	addImageFilter(filter_tiff);
#endif
	filter_ppm=new ImageFilter_PPM;
	addImageFilter(filter_ppm);

	filter_tga=new ImageFilter_TGA;
	addImageFilter(filter_tga);


	FontEngineFont5 *font5=new FontEngineFont5;
	addFontEngine(font5);
#ifdef HAVE_FREETYPE2
	FontEngineFreeType *freetype=new FontEngineFreeType;
	addFontEngine(freetype);
#endif

	Resource *res=GetPPLResource();

	loadFont(res->getMemory(34),"Default");
	loadFont(res->getMemory(35),"Default Mono");
	Toolbar.load(res->getMemory(14),16,16,ImageList::ALPHABLT);
	Icons32.load(res->getMemory(13),32,32,ImageList::ALPHABLT);
	ButtonSymbolsSmall.load(res->getMemory(12),9,9,ImageList::DIFFUSE);
}


Grafix::~Grafix()
{
	/*
	FontList.Clear(true);
	*/
	if (alphatab) free(alphatab);

	List<FontEngine*>::Iterator it;
	FontEngine *engine;
	while (FontEngineList.getFirst(it)) {
		engine=it.value();
		FontEngineList.erase(engine);
		delete engine;
	}
	if (filter_magick) {
		ImageFilterList.erase(filter_magick);
		delete filter_magick;
	}

	if (filter_png) {
		ImageFilterList.erase(filter_png);
		delete filter_png;
	}
	if (filter_jpeg) {
		ImageFilterList.erase(filter_jpeg);
		delete filter_jpeg;
	}
	if (filter_bmp) {
		ImageFilterList.erase(filter_bmp);
		delete filter_bmp;
	}
	if (filter_gif) {
		ImageFilterList.erase(filter_gif);
		delete filter_gif;
	}
	if (filter_ppm) {
		ImageFilterList.erase(filter_ppm);
		delete filter_ppm;
	}
	if (filter_tga) {
		ImageFilterList.erase(filter_tga);
		delete filter_tga;
	}
	if (filter_tiff) {
		ImageFilterList.erase(filter_tiff);
		delete filter_tiff;
	}





}

void Grafix::initAlphatab()
{
	ppluint32 alpha,i,a;
	alphatab=(char*)malloc(65536);
	if (!alphatab) throw OutOfMemoryException();
	for (alpha=0;alpha<256;alpha++) {
		for (i=0;i<256;i++) {
			a=alpha<<8;
			alphatab[a+i]=(ppluint8)((i*alpha)>>8);
		}
	}
}


void Grafix::initFunctions(const RGBFormat &format,GRAFIX_FUNCTIONS *fn)
{
	memset(fn,0,sizeof(GRAFIX_FUNCTIONS));
	try {
		initColors(format,fn);
	} catch (UnsupportedColorFormatException) {

	}
	try {
		initPixel(format,fn);
	} catch (UnsupportedColorFormatException) {

	}
	try {
		initShapes(format,fn);
	} catch (UnsupportedColorFormatException) {

	}
	try {
		initLines(format,fn);
	} catch (UnsupportedColorFormatException) {

	}
	try {
		initBlits(format,fn);
	} catch (UnsupportedColorFormatException) {

	}
}

GRAFIX_FUNCTIONS *Grafix::getGrafixFunctions(const RGBFormat &format)
{
	if (format >= RGBFormat::MaxIdentifiers) throw UnknownColorFormatException();
	return &functions[format];
}





} // EOF namespace grafix
} // EOF namespace ppl7
