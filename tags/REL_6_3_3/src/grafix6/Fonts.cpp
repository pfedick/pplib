/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.24 $
 * $Date: 2009/12/12 18:53:08 $
 * $Id: Fonts.cpp,v 1.24 2009/12/12 18:53:08 patrick Exp $
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


namespace ppl6 {
namespace grafix {

/*!\class CFontFile
 * \brief Interne Klasse zur Verwaltung aller geladener Font-Dateien
 * \ingroup PPLGroupGrafik
 */
CFontFile::CFontFile()
{
	ptr=NULL;
	size=0;
	engine=NULL;
	priv=NULL;
}

CFontFile::~CFontFile()
{
	if (engine) engine->DeleteFont(this);
}


int CGrafix::AddFontEngine(CFontEngine *engine)
/*!\brief FontEngine hinzufügen
 *
 * Mit dieser Funktion wird eine FontEngine der Grafikengine hinzugefügt. Darunter
 * versteht man eine von CFontEngine abgeleitete Klasse, die in der Lage ist Fontdateien
 * in einem bestimmten Format zu verarbeiten und darzustellen.
 *
 * Die FontEngine muss von der Anwendung mit "new" erstellt werden und als Pointer
 * an die CGrafix-Engine übergeben werden. CGrafix verwaltet die Engine ab diesem
 * Moment und kümmert sich auch um deren Löschung bei Programmende. Die Anwendung darf
 * die FontEngine nicht selbst löschen!
 *
 * \param engine Pointer auf die Klasse mit der FontEngine
 * \returns Liefert true (1) zurück, wenn die Engine erfolgreich aufgenommen werden
 * konnte, sonst false (0). Ein entsprechender Fehlercode wird gesetzt.
 */
{
	if (!engine) {
		SetError(194,"int CGrafix::AddFontEngine(FontEngine *engine)");
		return 0;
	}
	if (!engine->gfx) engine->gfx=this;
	else if (engine->gfx!=this) {
		SetError(1026);
		return 0;
	}
	if (!engine->Init()) return 0;
	Mutex.Lock();
	if (!FontEngines.Add(engine)) {
		Mutex.Unlock();
		ExtendError(1022);
		return 0;
	}
	Mutex.Unlock();
	return 1;
}

int CGrafix::LoadFont(const char *filename, const char *fontname)
{
	CFile ff;
	if (!ff.Open(filename,"rb")) return 0;
	return LoadFont(&ff,fontname);
}

int CGrafix::LoadFont(CFileObject *ff, const char *fontname)
{
	if (!ff) {
		SetError(194,"int LoadFont(==> CFileObject *ff <==, const char *fontname)");
		return 0;
	}
	// Falls ein Font mit gleichem Namen geladen ist, löschen wir
	// diesen zuerst
	//TODO: DeleteFont(fontname);

	Mutex.Lock();
	// Passenden Filter finden
	FontEngines.Reset();
	CFontEngine *engine;
	while ((engine=(CFontEngine*)FontEngines.GetPrevious())) {
		if (engine->Ident(ff)) {
			CFontFile *font=engine->LoadFont(ff, fontname);
			if (!font) {
				PushError();
				delete font;
				PopError();
				Mutex.Unlock();
				return 0;
			}
			if (!Fonts.Add(font)) {
				PushError();
				delete (font);
				PopError();
				Mutex.Unlock();
				ExtendError(1023);
				return 0;
			}
			Mutex.Unlock();
			return 1;
		}
	}
	Mutex.Unlock();
	if (fontname) SetError(1025,"%s",fontname);
	else SetError(1025);
	return 0;
}

int CGrafix::LoadFont(CResource *res, int i, const char *fontname)
{
	if (!res) {
		SetError(194,"int LoadFont(==> CResource *res <==, int i, const char *fontname)");
		return 0;
	}
	CFileObject *ff=res->GetFile(i);
	if (!ff) return 0;
	int ret=LoadFont(ff,fontname);
	PushError();
	delete ff;
	PopError();
	return ret;
}

int CGrafix::LoadFont(CResource *res, const char *name, const char *fontname)
{
	if (!res) {
		SetError(194,"int LoadFont(==> CResource *res <==, const char *name, const char *fontname)");
		return 0;
	}
	if (!name) {
		SetError(194,"int LoadFont(CResource *res, ==> const char *name <==, const char *fontname)");
		return 0;
	}
	CFileObject *ff=res->GetFile((char*)name);
	if (!ff) return 0;
	int ret=LoadFont(ff,fontname);
	PushError();
	delete ff;
	PopError();
	return ret;
}

int CGrafix::DeleteFont(const char *name)
{
	SetError(1031);
	return 0;
}

CFontFile *CGrafix::FindFont(const char *name)
{
	Mutex.Lock();
	Fonts.Reset();
	CFontFile *font;
	while ((font=(CFontFile*)Fonts.GetNext())) {
		if (font->Name==name) {
			Mutex.Unlock();
			return font;
		}
	}
	Mutex.Unlock();
	SetError(1032,"%s",name);
	return NULL;
}

/*!\class CFontEngine
 * \brief Basisklasse für Font-Engines
 * \ingroup PPLGroupGrafik
 *
 * \see
 * - CFontEngineFont5
 * - CFontEngineFreeType
 */


CFontEngine::CFontEngine()
{
	gfx=NULL;
}

CFontEngine::~CFontEngine()
{
}

int CFontEngine::Init()
{
	SetError(1024,"Init");
	return 0;
}

int CFontEngine::Ident(CFileObject *ff)
{
	SetError(1024,"Ident");
	return 0;
}

CFontFile *CFontEngine::LoadFont(CFileObject *ff, const char *fontname)
{
	SetError(1024,"LoadFont");
	return 0;
}

int CFontEngine::DeleteFont(CFontFile *file)
{
	SetError(1024,"DeleteFont");
	return 0;
}

int CFontEngine::Render(CFont *font, CSurface *surface, int x, int y, const char *text, COLOR color)
{
	SetError(1024,"Render");
	return 0;
}

int CFontEngine::Measure(CFont *font, const char *text, DIMENSION *measure)
{
	SetError(1024,"Measure");
	return 0;
}

int CFontEngine::Render(CFont *font, CSurface *surface, int x, int y, CWString *text, COLOR color)
{
	SetError(1024,"Render");
	return 0;
}

int CFontEngine::Measure(CFont *font, CWString *text, DIMENSION *measure)
{
	SetError(1024,"Measure");
	return 0;
}


int CFontEngine::Unselect(CFont *font)
{
	SetError(1024,"Unselect");
	return 0;
}

ppluint16 *CFontEngine::ToUTF16(char *text)
{
	CIconv Iconv;
	if (!Iconv.Init("UTF-8","UTF-16")) return 0;
	ppluint16 *t=(ppluint16 *)Iconv.Transcode((const char*)text,0);
	//HexDump(t,strlen(text)*2+4);
	return t;
}


int CSurface::Printf(CFont *font, int x, int y, const char *text, ...)
{
	CString String;
	va_list args;
	va_start(args, text);
	String.VaSprintf(text,args);
	va_end(args);
	return Print(font,x,y,(const char*)String);
}

int CSurface::Print(int x, int y, const char *text)
{
	return Print(&Font,x,y,text);
}

int CSurface::Print(int x, int y, CWString *text)
{
	return Print(&Font,x,y,text);
}

int CSurface::Printf(int x, int y, const char *text, ...)
{
	CString String;
	va_list args;
	va_start(args, text);
	String.VaSprintf(text,args);
	va_end(args);
	return Print(&Font,x,y,(const char*)String);
}

int CSurface::Print(const char *text)
{
	return Print(&Font,s.lastx,s.lasty,text);
}

int CSurface::Print(CWString *text)
{
	return Print(&Font,s.lastx,s.lasty,text);
}

int CSurface::Printf(const char *text, ...)
{
	CString String;
	va_list args;
	va_start(args, text);
	String.VaSprintf(text,args);
	va_end(args);
	return Print(&Font,s.lastx,s.lasty,(const char*)String);
}

int CSurface::Print(CFont *font, int x, int y, const char *text)
{
	if (!font) {
		SetError(1030);
		return 0;
	}
	if (!font->engine) {		// Dürfte nie vorkommen
		SetError(1025);
		return 0;
	}
	font->gfx=gfx;
	if (font->Shadow) font->engine->Render(font,this,x+1,y+1,text,font->shadow_color);

	else if (font->Border) {
		for (int yy=-1;yy<2;yy++) {
			for (int xx=-1;xx<2;xx++) {
				font->engine->Render(font,this,x+xx,y+yy,text,font->border_color);
			}
		}
	}

	return font->engine->Render(font,this,x,y,text,font->color);
}

int CSurface::Print(CFont *font, int x, int y, CWString *text)
{
	if (!font) {
		SetError(1030);
		return 0;
	}
	if (!font->engine) {		// Dürfte nie vorkommen
		SetError(1025);
		return 0;
	}
	font->gfx=gfx;
	if (font->Shadow) font->engine->Render(font,this,x+1,y+1,text,font->shadow_color);

	else if (font->Border) {
		for (int yy=-1;yy<2;yy++) {
			for (int xx=-1;xx<2;xx++) {
				font->engine->Render(font,this,x+xx,y+yy,text,font->border_color);
			}
		}
	}

	return font->engine->Render(font,this,x,y,text,font->color);
}

int CSurface::CharWidth(CFont *font, wchar_t c)
{
	CWString s;
	s.SetChar(c);
	DIMENSION d;
	if (!TextSize(font,&d,&s)) return 0;
	return d.width;
}

int CSurface::CharHeight(CFont *font, wchar_t c)
{
	CWString s;
	s.SetChar(c);
	DIMENSION d;
	if (!TextSize(font,&d,&s)) return 0;
	return d.height;
}

int CSurface::TextSize(CFont *font, DIMENSION *d, const char *text)
{
	if (!font) {
		SetError(1030);
		return 0;
	}
	if (!font->engine) {		// Dürfte nie vorkommen
		SetError(1025);
		return 0;
	}
	font->gfx=gfx;
	return font->engine->Measure(font,text,d);
}

int CSurface::TextSize(CFont *font, DIMENSION *d, CWString *text)
{
	if (!font) {
		SetError(1030);
		return 0;
	}
	if (!font->engine) {		// Dürfte nie vorkommen
		SetError(1025);
		return 0;
	}
	font->gfx=gfx;
	return font->engine->Measure(font,text,d);
}


int CSurface::TextSizef(CFont *font, DIMENSION *d, const char *text, ...)
{
	CString String;
	va_list args;
	va_start(args, text);
	String.VaSprintf(text,args);
	va_end(args);
	return TextSize(font,d,(const char*)String);
}

int CSurface::TextSizef(DIMENSION *d, const char *text, ...)
{
	CString String;
	va_list args;
	va_start(args, text);
	String.VaSprintf(text,args);
	va_end(args);
	return TextSize(&Font,d,(const char*)String);
}

int CSurface::TextSize(DIMENSION *d, const char *text)
{
	return TextSize(&Font,d,text);
}


/*!\class CFont
 * \brief Klasse zur Einstellung verschiedener Font-Parameter
 * \ingroup PPLGroupGrafik
 */

CFont::CFont()
{
	priv=NULL;
	engine=NULL;
	gfx=GetGrafix();
	file=NULL;
	Reset();
	Orientation.SetCallback(this,&Orientation);
	Size.SetCallback(this,&Size);
	Bold.SetCallback(this,&Bold);
	Italic.SetCallback(this,&Italic);
	Antialias.SetCallback(this,&Antialias);
	Underline.SetCallback(this,&Underline);
	Shadow.SetCallback(this,&Shadow);
	Border.SetCallback(this,&Border);
	Name.SetCallback(this,&Name);
}

void CFont::Callback(void *data)
{
	if (data==&Name) {
		CString Tmp=Name;
		LoadFont(Name);
	} else {
		if (engine) engine->Unselect(this);
	}
	Change();
}

int CFont::LoadFont(const char *name)
{
	if (!gfx) {
		gfx=GetGrafix();
		if (!gfx) {
			PushError();
			Name=OldName;
			PopError();
			return 0;
		}
	}
	CFontFile *f=gfx->FindFont(name);
	if (!f) {
		PushError();
		Name=OldName;
		PopError();
		return 0;
	}
	OldName=name;
	Name.DeleteCallback();
	Name=name;
	Name.SetCallback(this,&Name);
	if (engine) engine->Unselect(this);
	file=f;
	engine=f->engine;
	return 1;
}

int CFont::Reset()
{
	Orientation=ORIENTATION::BASE;
	Size=12;
	Bold=false;
	Italic=false;
	Antialias=true;
	Underline=false;
	color=0;
	shadow_color=0;
	border_color=0;
	Shadow=false;
	Border=false;
	LoadFont("PPL Liberation Sans");
	return 1;
}



} // EOF namespace grafix
} // EOF namespace ppl6

