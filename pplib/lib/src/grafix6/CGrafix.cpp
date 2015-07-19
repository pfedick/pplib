/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.35 $
 * $Date: 2009/12/12 18:53:07 $
 * $Id: CGrafix.cpp,v 1.35 2009/12/12 18:53:07 patrick Exp $
 *
 *******************************************************************************
 * Copyright (c) 2008, Patrick Fedick <patrick@pfp.de>
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
#include "ppl6-tk.h"


namespace ppl6 {
namespace grafix {

/*!\class CGrafix
 * \ingroup PPLGroupGrafik
 * \brief Hauptklasse für alle Grafik-Operationen
 *
 * Diese Klasse wird für alle grafischen Funktionen benötigt und muß einmalig
 * von der Anwendung instanziiert und Initialisiert werden.
 */


static CGrafix *pplgfx=NULL;

CGrafix *GetGrafix()
{
	if (pplgfx) return pplgfx;
	SetError(1040);
	return NULL;
}

CGrafix::CGrafix()
{
	PrimaryRGBFormat=0;
	PrimarySurface=NULL;
	engine=NULL;
	InitErrors();
	filter_png=NULL;
	filter_jpeg=NULL;
	filter_bmp=NULL;
	filter_gif=NULL;
	filter_ppm=NULL;
	filter_tga=NULL;
	alphatab=NULL;
	Toolbar=NULL;
	ButtonSymbolsSmall=NULL;
	Icons32=NULL;
}

CGrafix::~CGrafix()
{
	if (Toolbar) delete Toolbar;
	if (ButtonSymbolsSmall) delete ButtonSymbolsSmall;
	if (Icons32) delete Icons32;
	// Vorhandene Surfaces löschen
	Surfaces.Clear(true);
	Filter.Clear();
	Fonts.Clear(true);
	FontEngines.Clear(true);
	if (filter_png) delete filter_png;
	if (filter_jpeg) delete filter_jpeg;
	if (filter_bmp) delete filter_bmp;
	if (filter_gif) delete filter_gif;
	if (filter_ppm) delete filter_ppm;
	if (filter_tga) delete filter_tga;

	if (alphatab) free(alphatab);


}

int CGrafix::Init(CEngine *engine)
{
	Mutex.Lock();
	if (pplgfx) {
		Mutex.Unlock();
		SetError(1039);
		return 0;
	}
	if (this->engine) {
		Mutex.Unlock();
		SetError(1001);
		return 0;
	}
	if (engine) {
		// Die Engine muss vorher initialisiert worden sein
		if (engine->IsInit==false) {
			Mutex.Unlock();
			SetError(1002);
			return 0;
		}
		this->engine=engine;
	}
	Mutex.Unlock();
	InitAlphatab();
	// Standardfilter anlegen
#ifdef HAVE_PNG
	filter_png=new CFilter_PNG;
	AddFilter(filter_png);
#endif
#ifdef HAVE_JPEG
	filter_jpeg=new CFilter_JPEG;
	AddFilter(filter_jpeg);
#endif
	filter_bmp=new CFilter_BMP;
	AddFilter(filter_bmp);
	filter_gif=new CFilter_GIF;
	AddFilter(filter_gif);
	filter_ppm=new CFilter_PPM;
	AddFilter(filter_ppm);
	filter_tga=new CFilter_TGA;
	AddFilter(filter_tga);

	CFontEngineFont5 *font5=new ppl6::grafix::CFontEngineFont5;
	AddFontEngine(font5);
#ifdef HAVE_FREETYPE2
	CFontEngineFreeType *freetype=new ppl6::grafix::CFontEngineFreeType;
	AddFontEngine(freetype);
#endif
	CResource *res=GetPPLResource();

	LoadFont(res,34,"PPL Liberation Sans");
	LoadFont(res,35,"PPL Liberation Mono");
	pplgfx=this;

	if (!Toolbar) Toolbar=new CImageList;
	Toolbar->Load(res,14,16,16);
	Toolbar->SetDrawMethod(DRAWMETHOD::ALPHABLT);

	if (!Icons32) Icons32=new CImageList;
	Icons32->Load(res,13,32,32);
	Icons32->SetDrawMethod(DRAWMETHOD::ALPHABLT);

	if (!ButtonSymbolsSmall) ButtonSymbolsSmall=new CImageList;
	ButtonSymbolsSmall->Load(res,12,9,9);
	ButtonSymbolsSmall->SetDrawMethod(DRAWMETHOD::DIFFUSE);
	return 1;
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


CSurface *CGrafix::CreateWindow(const char *title, int width, int height, int rgbformat, int style, int flags, ppl6::tk::CWindow *window)
{
	// Für diese Funktion benötigen wir in jedem Fall eine Engine
	Mutex.Lock();
	if ((!engine) || (!engine->IsInit)) {
		Mutex.Unlock();
		SetError(1002);
		return NULL;
	}
	// Wir prüfen, ob die Engine diese Funktion unterstützt. Dazu rufen wir engine->CreateWindow
	// mit NULL als Parameter auf. Wird die Funktion nicht unterstützt, wird der Fehlercode
	// 1007 zurückgegeben
	if (!engine->CreateWindow(NULL)) {
		if (GetErrorCode()==1007) {
			Mutex.Unlock();
			return NULL;
		}
	}
	Mutex.Unlock();
	// Nun können wir die Window-Klasse erstellen
	CSurface *w=new CSurface;
	if (!w) {
			SetError(2);
			return NULL;
	}
	w->s.width=width;
	w->s.height=height;
	w->s.flags=flags;
	w->s.style=style;
	w->s.rgbformat=rgbformat;
	w->Name=title;
	w->Caption=title;
	w->gfx=this;
	// Jetzt rufen wir die Engine auf
	Mutex.Lock();
	if (!engine->CreateWindow(w,window)) {
		// Im Fehlerfall retten wir den Fehlercode
		PushError();
		Mutex.Unlock();
		// und löschen die Window-Klasse wieder
		delete w;
		PopError();
		return NULL;
	}
	Mutex.Unlock();
	w->engine=engine;
	w->s.clipper.left=0;
	w->s.clipper.top=0;
	w->s.clipper.right=width;
	w->s.clipper.bottom=height;

	w->s.flags|=Surface::Flags::PrimarySurface;
	w->s.type=Surface::Types::Primary;

	// Surface-Funktionen initialisieren, die nicht von der Engine initialisiert wurden
	if (!InitSurface(&w->s)) {
		PushError();
		delete w;
		PopError();
		return NULL;
	}
	// Das Fenster wird in die Surface-Liste aufgenommen
	Mutex.Lock();
	Surfaces.Add(w);
	PrimaryRGBFormat=w->s.rgbformat;
	PrimarySurface=w;
	Mutex.Unlock();
	if (window) {
		window->surface=w;
		window->w.width=width;
		window->w.height=height;
		window->InitColors();
		window->Load();
	}
	return w;
}

CSurface *CGrafix::CreateFullscreen(const char *title, int width, int height, int rgbformat, int style, int flags, int frequency, ppl6::tk::CWindow *window)
{
	// Für diese Funktion benötigen wir in jedem Fall eine Engine
	Mutex.Lock();
	if ((!engine) || (!engine->IsInit)) {
		Mutex.Unlock();
		SetError(1002);
		return NULL;
	}
	// Wir prüfen, ob die Engine diese Funktion unterstützt. Dazu rufen wir engine->CreateWindow
	// mit NULL als Parameter auf. Wird die Funktion nicht unterstützt, wird der Fehlercode
	// 1007 zurückgegeben
	if (!engine->CreateFullscreen(NULL)) {
		if (GetErrorCode()==1007) {
			Mutex.Unlock();
			return NULL;
		}
	}
	Mutex.Unlock();
	// Nun können wir die Window-Klasse erstellen
	CSurface *w=new CSurface;
	if (!w) {
		SetError(2);
		return NULL;
	}
	w->s.width=width;
	w->s.height=height;
	w->s.flags=flags;
	w->s.style=style;
	w->s.rgbformat=rgbformat;
	w->s.frequency=frequency;
	w->Name=title;
	w->Caption=title;
	w->gfx=this;
	// Jetzt rufen wir die Engine auf
	Mutex.Lock();
	if (!engine->CreateFullscreen(w,window)) {
		// Im Fehlerfall retten wir den Fehlercode
		PushError();
		Mutex.Unlock();
		// und löschen die Window-Klasse wieder
		delete w;
		PopError();
		return NULL;
	}
	Mutex.Unlock();
	w->engine=engine;
	w->s.clipper.left=0;
	w->s.clipper.top=0;
	w->s.clipper.right=width;
	w->s.clipper.bottom=height;
	w->s.flags|=Surface::Flags::PrimarySurface;
	w->s.flags|=Surface::Flags::Fullscreen;
	w->s.type=Surface::Types::Primary;

	// Surface-Funktionen initialisieren, die nicht von der Engine initialisiert wurden
	if (!InitSurface(&w->s)) {
		PushError();
		delete w;
		PopError();
		return NULL;
	}
	// Das Fenster wird in die Surface-Liste aufgenommen
	Mutex.Lock();
	Surfaces.Add(w);
	PrimaryRGBFormat=w->s.rgbformat;
	PrimarySurface=w;
	Mutex.Unlock();
	if (window) {
		window->surface=w;
		window->w.width=width;
		window->w.height=height;
		window->InitColors();
		window->Load();
	}

	return w;
}

CSurface *CGrafix::CreateSurface(const char *name, int width, int height, int rgbformat, int flags)
{
	bool UseEngine=false;
	if (!rgbformat) rgbformat=PrimaryRGBFormat;
	// Für diese Funktion benötigen wir nur eine Engine, wenn Hardware oder VideoMemory gewünscht ist
	if ((flags&Surface::Flags::Hardware) || (flags&Surface::Flags::VideoMemory)) {
		if ((!engine) || (!engine->IsInit)) {
			SetError(1002);
			return NULL;
		}
		// Wir prüfen, ob die Engine diese Funktion unterstützt. Dazu rufen wir engine->CreateWindow
		// mit NULL als Parameter auf. Wird die Funktion nicht unterstützt, wird der Fehlercode
		// 1007 zurückgegeben
		Mutex.Lock();
		if (!engine->CreateSurface(NULL)) {
			if (GetErrorCode()==1007) {
				Mutex.Unlock();
				return NULL;
			}
		}
		Mutex.Unlock();
		UseEngine=true;

	}
	// Nun können wir die Window-Klasse erstellen
	CSurface *w=new CSurface;
	if (!w) {
		SetError(2);
		return NULL;
	}
	w->s.width=width;
	w->s.height=height;
	w->s.flags=flags;
	w->s.rgbformat=rgbformat;

	w->Name=name;
	w->gfx=this;
	if (UseEngine) {
		// Jetzt rufen wir die Engine auf
		Mutex.Lock();
		if (!engine->CreateSurface(w)) {
			// Im Fehlerfall retten wir den Fehlercode
			PushError();
			Mutex.Unlock();
			// und löschen die Window-Klasse wieder
			delete w;
			PopError();
			return NULL;
		}
		Mutex.Unlock();
		w->engine=engine;
	} else {			// Software-Surface
		w->s.type=Surface::Types::Surface;
		w->engine=NULL;
		w->s.bitdepth=GetBitdepth(w->s.rgbformat);
		w->s.bytes_per_pixel=(w->s.bitdepth)>>3;
		w->s.pitch=w->s.bytes_per_pixel*width;
		w->s.base=calloc(height,w->s.pitch);
		if (!w->s.base) {
			delete w;
			SetError(2,"CGrafix::CreateSurface");
			return NULL;
		}
		w->s.base8=(ppluint8*)w->s.base;
		w->s.base16=(ppluint16*)w->s.base;
		w->s.base32=(ppluint32*)w->s.base;
		w->s.pitch8=w->s.pitch;
		w->s.pitch16=(w->s.pitch)>>1;
		w->s.pitch32=(w->s.pitch)>>2;

	}
	w->s.clipper.left=0;
	w->s.clipper.top=0;
	w->s.clipper.right=width;
	w->s.clipper.bottom=height;

	// Surface-Funktionen initialisieren, die nicht von der Engine initialisiert wurden
	if (!InitSurface(&w->s)) {
		PushError();
		delete w;
		PopError();
		return NULL;
	}
	// Das Fenster wird in die Surface-Liste aufgenommen
	Mutex.Lock();
	Surfaces.Add(w);
	Mutex.Unlock();
	return w;
}

static int Dummy (struct tagSURFACE *surface)
{
	return 1;
}

int CGrafix::InitSurface(SURFACE *s)
{
	s->alphatab=alphatab;
	if (!InitColors(s)) return 0;
	if (!InitPixel(s)) return 0;
	if (!InitLines(s)) return 0;
	if (!InitShapes(s)) return 0;
	if (!InitBlits(s)) return 0;
	/* Lock und Unlock werden intern auch direkt über die Struktur statt über das Surface aufgerufen,
	 * daher definieren wir eine Dummy-Funktion, die true zurückgibt
	 */
	if (!s->Lock) s->Lock=Dummy;
	if (!s->Unlock) s->Unlock=Dummy;
	if (!s->Flip) s->Flip=Dummy;
	return 1;
}


int CGrafix::StartMessageLoop()
{
	// Für diese Funktion benötigen wir in jedem Fall eine Engine
	Mutex.Lock();
	if ((!engine) || (!engine->IsInit)) {
		Mutex.Unlock();
		SetError(1002);
		return 0;
	}
	Mutex.Unlock();
	return engine->StartMessageLoop();
}

int CGrafix::LockWidgets()
{
	return Widgets.Lock();
}

int CGrafix::UnlockWidgets()
{
	return Widgets.Unlock();
}


} // EOF namespace grafix
} // EOF namespace ppl6

