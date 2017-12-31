/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
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

#include "ppl6.h"
#include "ppl6-grafix.h"

namespace ppl6 {
namespace grafix {

/*!\class CGrafix
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
 * gewünschte Grafik-Engine initialisieren und mit CGrafix::UseEngine anmelden.
 * Siehe dazu auch die Basisklasse für Grafikengines GFXEngine, sowie die
 * davon abgeleiteten Klassen GFXEngineDX9 und GFXEngineSDL.
 *
 * \par Grafik-Filter
 * Ein Grafik-Filter ist eine von CFilter abgeleitete Klasse, mit der man ein bestimmtes
 * Grafikformat lesen oder schreiben kann. Die PPL-Library bringt bereits Filter für
 * die Format PNG, JPEG, GIF, BMP, PPM und TGA mit, die bei Initialisierung von CGrafix
 * automatisch eingebunden werden. Bringt die Anwendung weitere Filter mit, müssen
 * diese mit CGrafix::AddFilter registriert werden.
 *
 * \par Fonts
 * Genau wie bei den Grafikfiltern gibt es auch Klassen für verschiedene Font-Formate.
 * Die Basisklasse hierfür ist CFontEngine. PPL bringt bereits eine Engine für das eigene
 * Font5-Format mit, sowie für die FreeType-Library, mit der unter anderem Fonts im TrueType
 * oder OpenType Format dargestellt werden können. Um weitere Engines hinzuzufügen,
 * muss die Funktion CGrafix::AddFontEngine aufgerufen werden.
 * \par
 * Standardmäßig sind ist der freie Font "Liberation Sans" und "Liberation Mono" im
 * Font5-Format in einigen Auflösungen enthalten. Um weitere Fonts zu laden muss
 * die Funktion CGrafix::LoadFont verwendet werden.
 *
 * \exception Exception::ExistingCGrafixInstance Wird geworfen, wenn versucht wird eine zweite
 * Instanz der Klasse zu erstellen
 */

static CGrafix *pplgfx=NULL;
char *alphatab=NULL;

static GRAFIX_FUNCTIONS functions[RGBFormat::MaxIdentifiers];


CGrafix *GetGrafix()
{
	if (pplgfx) return pplgfx;
	SetError(1040);
	return NULL;
}


CGrafix::CGrafix()
{
	GlobalMutex.Lock();
	if (pplgfx) {
		GlobalMutex.Unlock();
		throw DuplicateInstanceException("CGrafix");
	}
	alphatab=NULL;
	engine=NULL;
	filter_png=NULL;
	filter_jpeg=NULL;
	filter_bmp=NULL;
	filter_gif=NULL;
	filter_ppm=NULL;
	filter_tga=NULL;
	InitErrors();
	InitAlphatab();

	// Farbformat-abhängige Funktionen initialisieren
	for (int i=0;i<RGBFormat::MaxIdentifiers;i++) {
		InitFunctions(RGBFormat((RGBFormat::Identifier)i),&functions[i]);
	}

	pplgfx=this;
	GlobalMutex.Unlock();


	// Standardfilter anlegen
	filter_bmp=new CFilter_BMP;
	AddFilter(filter_bmp);
	filter_gif=new CFilter_GIF;
	AddFilter(filter_gif);

#ifdef HAVE_PNG
	filter_png=new CFilter_PNG;
	AddFilter(filter_png);
#endif
#ifdef HAVE_JPEG
	filter_jpeg=new CFilter_JPEG;
	AddFilter(filter_jpeg);
#endif
	filter_ppm=new CFilter_PPM;
	AddFilter(filter_ppm);
	filter_tga=new CFilter_TGA;
	AddFilter(filter_tga);

	CFontEngineFont5 *font5=new CFontEngineFont5;
	AddFontEngine(font5);
	FontEngineFont6 *font6=new FontEngineFont6;
	AddFontEngine(font6);

	#ifdef HAVE_FREETYPE2
	CFontEngineFreeType *freetype=new CFontEngineFreeType;
	AddFontEngine(freetype);
#endif
	CResource *res=GetPPLResource();

	LoadFont(res->GetMemory(34),"PPL Liberation Sans");
	LoadFont(res->GetMemory(35),"PPL Liberation Mono");


	Toolbar.load(res->GetMemory(14),16,16,CImageList::ALPHABLT);
	Icons32.load(res->GetMemory(13),32,32,CImageList::ALPHABLT);
	ButtonSymbolsSmall.load(res->GetMemory(12),9,9,CImageList::DIFFUSE);
	pplgfx=this;
}


CGrafix::~CGrafix()
{
	FontList.Clear(true);
	FontEngines.Clear(true);
	ImageFilter.Clear(true);
	if (alphatab) free(alphatab);
	if (pplgfx==this) pplgfx=NULL;
}

void CGrafix::InitAlphatab()
{
	ppldd alpha,i,a;
	alphatab=(char*)malloc(65536);
	if (!alphatab) {
		SetError(2);
		return;
	}
	for (alpha=0;alpha<256;alpha++) {
		for (i=0;i<256;i++) {
			a=alpha<<8;
			alphatab[a+i]=(ppldb)((i*alpha)>>8);
		}
	}
}


int CGrafix::InitFunctions(const RGBFormat &format,GRAFIX_FUNCTIONS *fn)
{
	memset(fn,0,sizeof(GRAFIX_FUNCTIONS));
	InitColors(format,fn);
	InitPixel(format,fn);
	InitShapes(format,fn);
	InitLines(format,fn);
	InitBlits(format,fn);
	//fn->supported=1;
	return 1;
}

GRAFIX_FUNCTIONS *CGrafix::GetGrafixFunctions(const RGBFormat &format)
{
	GRAFIX_FUNCTIONS *fn=&functions[format];
	/*
	if (!fn->supported) {
		SetError(1013);
		return NULL;
	}
	*/
	return fn;
}


int CGrafix::UseEngine(GFXEngine *engine)
{
	if (!engine) {
		SetError(1068);
		return 0;
	}

	Mutex.Lock();
	if (this->engine) {
		Mutex.Unlock();
		SetError(1001);
		return 0;
	}
	// Die Engine muss vorher initialisiert worden sein
	if (engine->isInit()==false) {
		Mutex.Unlock();
		SetError(1002);
		return 0;
	}
	this->engine=engine;
	Mutex.Unlock();
	return 1;
}

bool CGrafix::CreateWindow(tk::Widget &widget)
{
	if (!engine) {
		SetError(1071);
		return false;
	}
	return engine->createWindow(widget);
}

bool CGrafix::CreateSurface(CSurface &surface)
{
	if (!(surface.flags&CSurface::Hardware)) {
		return software.createSurface(surface);
	}
	if (!engine) {
		SetError(1071);
		return false;
	}
	return engine->createSurface(surface);
}

} // EOF namespace grafix
} // EOF namespace ppl6
