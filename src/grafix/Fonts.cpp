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

#include "ppl7.h"
#include "ppl7-grafix.h"

namespace ppl7 {
namespace grafix {

/*!\class Font
 * \ingroup PPLGroupGrafik
 * \brief Repräsentiert die verschiedenen Parameter eines Fonts
 *
 * @return
 */

Font::Font()
{
	Name=L"Default";
	fontSize=12;
	flags=0;
	ori=0;
	rotationDegrees=0.0;
}

Font::Font(const Font &other)
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

Font &Font::operator=(const Font &other)
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

const String &Font::name() const
{
	return Name;
}

Color Font::color() const
{
	return cForeground;
}

Color Font::borderColor() const
{
	return cBorder;
}

Color Font::shadowColor() const
{
	return cShadow;
}

bool Font::bold() const
{
	if (flags&fBold) return true;
	return false;
}

bool Font::italic() const
{
	if (flags&fItalic) return true;
	return false;
}

bool Font::antialias() const
{
	if (flags&fAntialias) return true;
	return false;
}

bool Font::drawBorder() const
{
	if (flags&fDrawBorder) return true;
	return false;
}

bool Font::drawShadow() const
{
	if (flags&fDrawShadow) return true;
	return false;
}

bool Font::drawUnderline() const
{
	if (flags&fUnderline) return true;
	return false;
}

int Font::size() const
{
	return fontSize;
}

Font::Orientation Font::orientation() const
{
	return (Orientation)ori;
}

double Font::rotation() const
{
	return rotationDegrees;
}

int Font::setName(const String &name)
{
	// TODO: Prüfen, ob der Font existiert
	Name=name;
	return 1;
}

void Font::setColor(const Color &c)
{
	cForeground=c;
}

void Font::setBorderColor(const Color &c)
{
	cBorder=c;
}

void Font::setShadowColor(const Color &c)
{
	cShadow=c;
}

void Font::setColors(const Color &Foreground, const Color &Border, const Color &Shadow)
{
	cForeground=Foreground;
	cBorder=Border;
	cShadow=Shadow;
}

void Font::setBold(bool enable)
{
	if (!enable) flags&=(0xff-fBold);
	else flags|=fBold;
}

void Font::setItalic(bool enable)
{
	if (!enable) flags&=(0xff-fItalic);
	else flags|=fItalic;
}

void Font::setAntialias(bool enable)
{
	if (!enable) flags&=(0xff-fAntialias);
	else flags|=fAntialias;
}

void Font::setDrawBorder(bool enable)
{
	if (!enable) flags&=(0xff-fDrawBorder);
	else flags|=fDrawBorder;
}

void Font::setDrawShadow(bool enable)
{
	if (!enable) flags&=(0xff-fDrawShadow);
	else flags|=fDrawShadow;
}

void Font::setDrawUnderline(bool enable)
{
	if (!enable) flags&=(0xff-fUnderline);
	else flags|=fUnderline;
}

void Font::setMonospace(bool enable)
{
	if (!enable) flags&=(0xff-fMonospace);
	else flags|=fMonospace;
}

void Font::setSize(int size)
{
	fontSize=size;
}

void Font::setOrientation(Orientation o)
{
	ori=o;
}

void Font::setRotation(double degrees)
{
	rotationDegrees=degrees;
}

Size Font::measure(const String &text) const
{
	Size s;
	Grafix *gfx=GetGrafix();
	FontFile *file=gfx->findFont(Name);
	if (!file) return s;
	if (!file->engine) return s;
	return file->engine->measure(*file,*this,text);
}

Rect Font::boundary(const String &text, int x, int y) const
{
	Rect r;
	Grafix *gfx=GetGrafix();
	FontFile *file=gfx->findFont(Name);
	if (!file) return r;
	if (!file->engine) return r;
	return file->engine->boundary(*file,*this,text,x,y);
}


bool operator!= (const Font &f1, const Font &f2)
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

bool operator== (const Font &f1, const Font &f2)
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



/*!\class FontFile
 * \ingroup PPLGroupGrafik
 * \brief Interne Klasse zur Verwaltung aller geladener Font-Dateien
 */
FontFile::FontFile()
{
	engine=NULL;
	priv=NULL;
}

FontFile::~FontFile()
{
	if (engine) engine->deleteFont(this);
}




/*!\brief FontEngine hinzufügen
 *
 * Mit dieser Funktion wird eine FontEngine der Grafikengine hinzugefügt. Darunter
 * versteht man eine von FontEngine abgeleitete Klasse, die in der Lage ist Fontdateien
 * in einem bestimmten Format zu verarbeiten und darzustellen.
 *
 * Die FontEngine muss von der Anwendung mit "new" erstellt werden und als Pointer
 * an die Grafix-Engine übergeben werden. Grafix verwaltet die Engine ab diesem
 * Moment und kümmert sich auch um deren Löschung bei Programmende. Die Anwendung darf
 * die FontEngine nicht selbst löschen!
 *
 * \param engine Pointer auf die Klasse mit der FontEngine
 * \returns Liefert true (1) zurück, wenn die Engine erfolgreich aufgenommen werden
 * konnte, sonst false (0). Ein entsprechender Fehlercode wird gesetzt.
 */
void Grafix::addFontEngine(FontEngine *engine)
{
	if (!engine) throw NullPointerException();
	engine->init();
	myMutex.lock();
	try {
		FontEngineList.add(engine);
	} catch (...) {
		myMutex.unlock();
		throw;
	}
	myMutex.unlock();
}

void Grafix::loadFont(const String &filename, const String &fontname)
{
	File ff;
	ff.open(filename,File::READ);
	loadFont(ff,fontname);
}

void Grafix::loadFont(const ByteArrayPtr &memory, const String &fontname)
{
	MemFile ff(memory);
	loadFont(ff,fontname);
}

void Grafix::loadFont(FileObject &ff, const String &fontname)
{
	myMutex.lock();
	List<FontEngine*>::Iterator it;
	// Passenden Filter finden
	FontEngineList.reset(it);
	FontEngine *engine;
	try {
		while (FontEngineList.getPrevious(it)) {
			engine=it.value();
			int id=engine->ident(ff);
			if (id==1) {

				FontFile *font=engine->loadFont(ff, fontname);
				if (!font) throw InvalidFontException();
				// Falls ein Font mit gleichem Namen geladen ist, löschen wir
				// diesen zuerst
				try {
					FontFile *old=FontList.find(font->Name);
					if (old) {
						FontList.erase(font->Name);
						old->engine->deleteFont(old);
					}
				} catch (ItemNotFoundException) {
					// Das macht nix
				} catch (...) {
					font->engine->deleteFont(font);
					myMutex.unlock();
					throw;
				}
				try {
					FontList.add(font->Name,font);
				} catch (...) {
					font->engine->deleteFont(font);
					myMutex.unlock();
					throw;
				}
				myMutex.unlock();
				return;
			}
		}
	} catch (EndOfListException &) {
		myMutex.unlock();
		fontname.print();
		throw NoSuitableFontEngineException();
	} catch (...) {
		myMutex.unlock();
		throw;
	}
	myMutex.unlock();
	throw NoSuitableFontEngineException();
}

void Grafix::unloadFont(const String &fontname)
{
	myMutex.lock();
	try {
		FontFile *file=FontList.find(fontname);
		if (file) {
			FontList.erase(fontname);
			file->engine->deleteFont(file);
		}
	} catch (...) {
		myMutex.unlock();
		throw;
	}
	myMutex.unlock();
	return;
}

FontFile *Grafix::findFont(const String &fontname)
{
	myMutex.lock();
	try {
		FontFile *file=FontList.find(fontname);
		myMutex.unlock();
		if (file) return file;
	} catch (ItemNotFoundException &) {
		myMutex.unlock();
	} catch (...) {
		myMutex.unlock();
		throw;
	}
	throw FontNotFoundException(fontname);
}

void Grafix::listFonts()
{
	myMutex.lock();
	printf ("Available Fonts:\n");
	AVLTree<String, FontFile*>::Iterator it;
	FontList.reset(it);
	while (FontList.getNext(it)) {
		FontFile *ff=it.value();
		printf ("    %s, Engine: %s\n",(const char*)ff->Name, (const char*)ff->engine->name());
	}
	myMutex.unlock();
}

FontFile *Grafix::findFont(const Font &font)
{
	return findFont(font.name());
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
void Drawable::print(const Font &font, int x, int y, const String &text)
{
	Grafix *gfx=GetGrafix();
	FontFile *file=gfx->findFont(font.name());
	if (font.drawShadow()) {
		file->engine->render(*file,font,*this,x+2,y+2,text,font.shadowColor());
	}
	if (font.drawBorder()) {
		for (int a=-1;a<2;a++) {
			for (int b=-1;b<2;b++) {
				file->engine->render(*file,font,*this,x+a,y+b,text,font.borderColor());
			}
		}
	}
	file->engine->render(*file,font,*this,x,y,text,font.color());
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
void Drawable::printf(const Font &font, int x, int y, const char *fmt, ...)
{
	String s;
	va_list args;
	va_start(args, fmt);
	s.vasprintf(fmt,args);
	va_end(args);
	print(font,x,y,s);
}



/*!\class FontEngine
 * \ingroup PPLGroupGrafik
 * \brief Basisklasse für Font-Engines
 *
 * \see
 * - FontEngineFont5
 * - FontEngineFreeType
 */


FontEngine::FontEngine()
{
}

FontEngine::~FontEngine()
{
}

void FontEngine::init()
{
	throw UnimplementedVirtualFunctionException();
}

int FontEngine::ident(FileObject &ff) throw()
{
	return 0;
}

FontFile *FontEngine::loadFont(FileObject &ff, const String &fontname)
{
	throw UnimplementedVirtualFunctionException();
}

void FontEngine::deleteFont(FontFile *file)
{
	throw UnimplementedVirtualFunctionException();
}

void FontEngine::render(const FontFile &file, const Font &font, Drawable &draw, int x, int y, const WideString &text, const Color &color)
{
	throw UnimplementedVirtualFunctionException();
}

Size FontEngine::measure(const FontFile &file, const Font &font, const WideString &text)
{
	throw UnimplementedVirtualFunctionException();
}

Rect FontEngine::boundary(const FontFile &file, const Font &font, const WideString &text, int x, int y)
{
	throw UnimplementedVirtualFunctionException();
}

String FontEngine::name() const
{
	throw UnimplementedVirtualFunctionException();
}

String FontEngine::description() const
{
	throw UnimplementedVirtualFunctionException();
}



} // EOF namespace grafix
} // EOF namespace ppl7
