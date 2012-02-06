/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.2 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: Fonts.cpp,v 1.2 2010/02/12 19:43:48 pafe Exp $
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

namespace ppl6 {
namespace grafix {

/*!\class CFont
 * \ingroup PPLGroupGrafik
 * \brief Repräsentiert die verschiedenen Parameter eines Fonts
 *
 * @return
 */

CFont::CFont()
{
	Name="PPL Liberation Sans";
	fontSize=12;
	flags=0;
	ori=0;
	rotationDegrees=0.0;
}

CFont::CFont(const CFont &other)
{
	Name=other.Name;
	cForeground=other.cForeground;
	cBorder=other.cBorder;
	cShadow=other.cShadow;
	fontSize=other.fontSize;
	flags=other.flags;
	ori=other.ori;
	rotationDegrees=other.rotationDegrees;
}

CFont &CFont::operator=(const CFont &other)
{
	Name=other.Name;
	cForeground=other.cForeground;
	cBorder=other.cBorder;
	cShadow=other.cShadow;
	fontSize=other.fontSize;
	flags=other.flags;
	ori=other.ori;
	rotationDegrees=other.rotationDegrees;
	return *this;
}

CString CFont::name() const
{
	return Name;
}

Color CFont::color() const
{
	return cForeground;
}

Color CFont::borderColor() const
{
	return cBorder;
}

Color CFont::shadowColor() const
{
	return cShadow;
}

bool CFont::bold() const
{
	if (flags&fBold) return true;
	return false;
}

bool CFont::italic() const
{
	if (flags&fItalic) return true;
	return false;
}

bool CFont::antialias() const
{
	if (flags&fAntialias) return true;
	return false;
}

bool CFont::drawBorder() const
{
	if (flags&fDrawBorder) return true;
	return false;
}

bool CFont::drawShadow() const
{
	if (flags&fDrawShadow) return true;
	return false;
}

bool CFont::drawUnderline() const
{
	if (flags&fUnderline) return true;
	return false;
}

int CFont::size() const
{
	return fontSize;
}

CFont::Orientation CFont::orientation() const
{
	return (Orientation)ori;
}

double CFont::rotation() const
{
	return rotationDegrees;
}


int CFont::setName(const CString &name)
{
	// TODO: Pr�fen, ob der Font existiert
	Name=name;
	return 1;
}

void CFont::setColor(const Color &c)
{
	cForeground=c;
}

void CFont::setBorderColor(const Color &c)
{
	cBorder=c;
}

void CFont::setShadowColor(const Color &c)
{
	cShadow=c;
}

void CFont::setColors(const Color &Foreground, const Color &Border, const Color Shadow)
{
	cForeground=Foreground;
	cBorder=Border;
	cShadow=Shadow;
}

void CFont::setBold(bool enable)
{
	if (!enable) flags&=(0xff-fBold);
	else flags|=fBold;
}

void CFont::setItalic(bool enable)
{
	if (!enable) flags&=(0xff-fItalic);
	else flags|=fItalic;
}

void CFont::setAntialias(bool enable)
{
	if (!enable) flags&=(0xff-fAntialias);
	else flags|=fAntialias;
}

void CFont::setDrawBorder(bool enable)
{
	if (!enable) flags&=(0xff-fDrawBorder);
	else flags|=fDrawBorder;
}

void CFont::setDrawShadow(bool enable)
{
	if (!enable) flags&=(0xff-fDrawShadow);
	else flags|=fDrawShadow;
}

void CFont::setDrawUnderline(bool enable)
{
	if (!enable) flags&=(0xff-fUnderline);
	else flags|=fUnderline;
}

void CFont::setMonospace(bool enable)
{
	if (!enable) flags&=(0xff-fMonospace);
	else flags|=fMonospace;
}

void CFont::setSize(int size)
{
	fontSize=size;
}

void CFont::setOrientation(Orientation o)
{
	ori=o;
}

void CFont::setRotation(double degrees)
{
	rotationDegrees=degrees;
}

Size CFont::measure(const CWString &text) const
{
	Size s;
	CGrafix *gfx=GetGrafix();
	if (!gfx) return s;
	CFontFile *file=gfx->FindFont(Name);
	if (!file) return s;
	if (!file->engine) return s;
	return file->engine->Measure(file,*this,text);
}

Size CFont::measure(const char *fmt, ...) const
{
	CWString s;
	va_list args;
	va_start(args, fmt);
	s.Vasprintf(fmt,args);
	va_end(args);
	return measure(s);
}



bool operator!= (const CFont &f1, const CFont &f2)
{
	if (f1.Name==f2.Name) return false;
	if (f1.fontSize==f2.fontSize) return false;
	if (f1.flags==f2.flags) return false;
	if (f1.cForeground==f2.cForeground) return false;
	if (f1.cBorder==f2.cBorder) return false;
	if (f1.cShadow==f2.cShadow) return false;
	if (f1.ori==f2.ori) return false;
	return true;
}

bool operator== (const CFont &f1, const CFont &f2)
{
	if (f1.Name!=f2.Name) return false;
	if (f1.fontSize!=f2.fontSize) return false;
	if (f1.flags!=f2.flags) return false;
	if (f1.cForeground!=f2.cForeground) return false;
	if (f1.cBorder!=f2.cBorder) return false;
	if (f1.cShadow!=f2.cShadow) return false;
	if (f1.ori!=f2.ori) return false;
	return true;
}



/*!\class CFontFile
 * \ingroup PPLGroupGrafik
 * \brief Interne Klasse zur Verwaltung aller geladener Font-Dateien
 */
CFontFile::CFontFile()
{
	engine=NULL;
	priv=NULL;
}

CFontFile::~CFontFile()
{
	if (engine) engine->DeleteFont(this);
}

int CFontFile::CompareNode(CTreeItem *item)
{
	CFontFile *cf=(CFontFile*)item;
	if (cf->Name<Name) return -1;
	if (cf->Name>Name) return 1;
	return 0;
}

int CFontFile::CompareValue(void *value)
{
	CString *s=(CString *)value;
	if (*s<Name) return -1;
	if (*s>Name) return 1;
	return 0;
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

int CGrafix::LoadFont(const CMemoryReference &memory, const char *fontname)
{
	CMemFile ff(memory);
	return LoadFont(&ff,fontname);
}

int CGrafix::LoadFont(CFileObject *ff, const char *fontname)
{
	if (!ff) {
		SetError(194,"int LoadFont(==> CFileObject *ff <==, const char *fontname)");
		return 0;
	}

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
			// Falls ein Font mit gleichem Namen geladen ist, löschen wir
			// diesen zuerst
			CString search=font->Name;
			CFontFile *oldfile=(CFontFile*)FontList.Find(&search);
			if (oldfile) delete oldfile;

			if (!FontList.Add(font)) {
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

int CGrafix::UnloadFont(const CString &fontname)
{
	CFontFile *file=FindFont(fontname);
	if (!file) return 0;
	Mutex.Lock();
	delete file;
	Mutex.Unlock();
	return 1;
}

CFontFile *CGrafix::FindFont(const CString &fontname)
{
	CString search=fontname;
	Mutex.Lock();
	CFontFile *font=(CFontFile*)FontList.Find(&search);
	Mutex.Unlock();
	if (!font) {
		SetError(1032,"%s",(const char*)fontname);
		return NULL;
	}
	return font;
}

CFontFile *CGrafix::FindFont(const CFont &font)
{
	return FindFont(font.name());
}


/*!\brief Text ausgeben
 *
 * \desc
 * Mit dieser Funktion wird der Text \p text auf der Grafik an den
 * Koordinaten \p x und \p y unter Verwendeung des Fonts \p font ausgegeben.
 *
 * @param font Zu verwendende Font-Parameter
 * @param x X-Koordinate
 * @param y Y-Koordinate
 * @param text Der auszugebende Text
 */
void CDrawable::print(const CFont &font, int x, int y, const CWString &text)
{
	CGrafix *gfx=GetGrafix();
	if (!gfx) return;
	CFontFile *file=gfx->FindFont(font.name());
	if (!file) return;
	if (font.drawShadow()) {
		file->engine->Render(file,font,*this,x+2,y+2,text,font.shadowColor());
	}
	if (font.drawBorder()) {
		for (int a=-1;a<2;a++) {
			for (int b=-1;b<2;b++) {
				file->engine->Render(file,font,*this,x+a,y+b,text,font.borderColor());
			}
		}
	}
	file->engine->Render(file,font,*this,x,y,text,font.color());
}

/*!\brief Formatierten Text ausgeben
 *
 * \desc
 * Mit dieser Funktion wird zunächst ein Text anhand des Formatstrings
 * \p fmt erstellt und dann auf der Grafik an den
 * Koordinaten \p x und \p y unter Verwendeung des Fonts \p font ausgegeben.
 *
 * @param font Zu verwendende Font-Parameter
 * @param x X-Koordinate
 * @param y Y-Koordinate
 * @param fmt Formatstring
 * @param ... optionale Parameter für den Formatstring
 */
void CDrawable::printf(const CFont &font, int x, int y, const char *fmt, ...)
{
	CWString s;
	va_list args;
	va_start(args, fmt);
	s.Vasprintf(fmt,args);
	va_end(args);
	print(font,x,y,s);
}



/*!\class CFontEngine
 * \ingroup PPLGroupGrafik
 * \brief Basisklasse für Font-Engines
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

int CFontEngine::Render(CFontFile *file, const CFont &font, CDrawable &draw, int x, int y, const CWString &text, const Color &color)
{
	SetError(1024,"Render");
	return 0;
}

Size CFontEngine::Measure(CFontFile *file, const CFont &font, const CWString &text)
{
	SetError(1024,"Measure");
	return Size();
}




} // EOF namespace grafix
} // EOF namespace ppl6
