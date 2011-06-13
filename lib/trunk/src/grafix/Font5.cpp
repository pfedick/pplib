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


//#undef HAVE_X86_ASSEMBLER

// Font-Blitter
typedef struct tagGLYPH {
	const char *data;
	char *target;
	ppluint32 pitch;
	pplint32 color;
} GLYPH;

extern "C" {
	int BltGlyph_M8_32 (GLYPH *g);
	int BltGlyph_M1_32 (GLYPH *g);
	int BltGlyph_AA8_32 (GLYPH *g);
	int BltGlyph_AA2_32 (GLYPH *g);
	int BltGlyph_AA4_32 (GLYPH *g);
}


namespace ppl7 {
namespace grafix {

/*!\page PFPFont5Format Format PFP Font, Version 5
 *
Die Version 5 verwendet als Basisformat das \ref PFPFileVersion3 "PFP-File-Format Version 3" mit seinen Chunks.
Das File trägt als ID "FONT", Hauptversion 5, Unterversion 0. Eine Datei enthält in der Regel immer nur
einen Font (z.B. Arial), kann aber beliebig viele Faces enthalten, die sich durch Style (Fett, Kursiv, Antialiased)
und Pixelgröße unterscheiden. Jedes Face ist in einem eigenen Chunk untergebracht, der den Namen "FACE" trägt.

\par Header
Jedes FACE beginnt mit einem 12 Byte großen Header:
\code
Byte 0:  Flags                                        1 Byte
Byte 1:  Pixelformat                                  1 Byte
Byte 2:  Size (Fontgröße in Pixel)                    2 Byte
Byte 4:  MaxBearingY                                  2 Byte
Byte 6:  MaxHeight                                    2 Byte
Byte 8:  Position Underscore                          2 Byte
Byte 10:  Anzahl Sprungtabellen                       2 Byte
\endcode
Beschreibung:
<ul>
<li><b>Flags</b>\n
Folgende Flags sind definiert:
\code
Bit 0: Font benutzt Antialiasing
Bit 1: Font ist fett (bold)
Bit 2: Font ist kursiv (italic)
\endcode
</li>
<li><b>Pixelformat</b>\n
\code
0 = undefiniert
1 = Monochrom, 8 Bit Pro Pixel (1 Byte)
2 = Monochrom, 1 Bit pro Pixel
3 = Antialiased, 8 Bit pro Pixel (1 Byte) = 256 Graustufen
4 = Antialiased, 2 Bit pro Pixel = 4 Graustufen
5 = Antialiased, 4 Bit pro Pixel = 16 Graustufen

\endcode
</li>
<li><b>Size</b>\n
Größe der Fonts</li>
<li><b>MaxBearingY</b>\n
Dieser positive Wert gibt an, wieviele Pixel nach oben des größte Glyph abweicht.
Da Font5-Fonts immer von einer Grundlinie aus gezeichnet werden, kann man mit diesem
Wert den höchsten Punkt im Font-Face ermitteln, um z.B. statt von der Grundline aus
von oben aus die Fonts abzubilden (Align=Top)</li>
<li><b>MaxHeight</b>\n
Dieser Wert gibt die maximale Höhe der Glyphs an und gleichzeitig die Anzahl Pixel,
die bei einem Zeilenumbruch auf der Y-Achse dazuaddiert werden müssen. Aus der
Differenz "MaxHeight-MaxBearingY" kann man ausserdem den tiefsten Punkt im
Fontface errechnen, für den Fall dass man die Fonts von unten aus abbilden
möchte (Align=Bottom).</li>
<li><b>Anzahl Sprungtabellen</b>\n
Dieser Wert gibt an, wieviele Sprungtabellen im Face enthalten sind.
</li>
</ul>
\par Sprungtabellen
Das FONT5-Format sieht vor, dass es mehrere Sprungtabellen geben kann. Dadurch ist es möglich
selektiv nur die Fonts in das File einzubinden, die man tatsächlich benötigt.
\code
Byte 0:  Start Character in Tabelle (UNICODE)         2 Byte
Byte 2:  End Character in Tabelle (UNICODE)           2 Byte
Byte 4:  Pointer auf Beginn der Sprungtabelle         4 Byte
\endcode
Beschreibung:
<ul>
<li><b>Start und Ende in Character Tabelle</b>\n
Diese beiden Werte geben an, mit welchem Zeichen die nachfolgende Sprungtabelle beginnt und
Ended. Da UNICODE verwendet wird, sind die Werte 2 Byte groß und im LittleEndian Format
angegeben.
</li>
<li><b>Pointer auf Beginn der Sprungtabelle</b>\n
Ein 4 Byte Pointer im Little Endian Format, der den Beginn der Sprungtabelle innerhalb des FACE angibt.
</li>
</ul>
Die Sprungtabellen selbst besteht aus 4-Byte Werten im Little Endian Format, die den Beginn des jeweiligen
Glyphs innerhalb des Font-FACE angibt. Dabei wird der 12-Byte große Header und die Sprungtabelle mitgerechnet.\n\n
Die Sprungtabelle enthält nur Pointer für existierende Glyphs. Ist ein Glyph nicht vorhanden, enthält der
Wert in der Sprungtabelle 0.\n\n
Die Position des ersten Sprungtabelle  errechnet sich folgendermaßen:
\code
Position= 12 + Anzahl Sprungtabellen * 8
\endcode

\par Glyphs
Ein Glyph besteht aus einem Header und einer Bitmap. Der Header ist folgendermaßen aufgebaut:
\code
Byte 0: Breite                  (2 Byte)
Byte 2: Höhe                    (2 Byte)
Byte 4: BearingX (signed)       (2 Byte)
Byte 6: BearingY (signed)       (2 Byte)
Byte 8: Advance                 (2 Byte)
\endcode
Das Format der Bitmap ergibt sich aus dem Pixelformat im FACE-Header.
Die Größe errechnet sich aus <i>BitsProPixel * Breite * Höhe</i>, wobei auf volle Byte aufgerundet wird.\n\n
Die Position des ersten Glyphs errechnet sich folgendermaßen:
\code
Position= 12 + Anzahl Sprungtabellen * 8 + Anzahl Chars in jeder Sprungtabelle * 4
\endcode

 */


/*!\class FontEngineFont5
 * \ingroup PPLGroupGrafik
 * \brief Font-Engine für PFP Version 5 Fonts
 */

FontEngineFont5::FontEngineFont5()
{
}

FontEngineFont5::~FontEngineFont5()
{
}

String FontEngineFont5::name() const
{
	return L"FontEngineFont5";
}

String FontEngineFont5::description() const
{
	return "Rendering of PPLib Version 5 Fonts";
}


void FontEngineFont5::init()
{
	// Es gibt nichts zu tun
}

int FontEngineFont5::ident(FileObject &file) throw()
{
	PFPFile ff;
	if (!ff.ident(file)) return 0;
	if (ff.getID()!=L"FONT") return 0;
	if (ff.getMainVersion()==5 && ff.getSubVersion()==0) {
		return 1;
	}
	return 0;
}

FontFile *FontEngineFont5::loadFont(FileObject &file, const String &fontname)
{
	PFPFile *File = new PFPFile;
	if (!File) throw OutOfMemoryException();
	try {
		File->load(file);
	} catch (...) {
		delete File;
		throw;
	}
	if (File->getID()!=L"FONT") throw InvalidFontException();
	if (File->getMainVersion()!=5 || File->getSubVersion()!=0) throw InvalidFontException();
	FontFile *ff=new FontFile;
	if (!ff) {
		delete File;
		throw OutOfMemoryException();
	}
	if (fontname.notEmpty()) ff->Name=fontname;
	else ff->Name=File->getName();
	ff->engine=this;
	// Wir Speichern nicht den Speicherblock der Daten, sondern nur den
	// Pointer auf das PFPFile
	ff->priv=File;
	return ff;
}

void FontEngineFont5::deleteFont(FontFile *file)
{
	if (!file) throw NullPointerException();
	if (file->engine!=this) throw InvalidFontEngineException();
	PFPFile *f=(PFPFile *)file->priv;
	delete f;
	file->priv=NULL;
	file->engine=NULL;
}


#ifndef HAVE_X86_ASSEMBLER
extern "C" {
void BltGlyph_M8_32 (GLYPH *g)
{
	pplint16 width=Peek16(g->data);
	pplint16 height=Peek16(g->data+2);
	const char *bitmap=g->data+10;
	ppluint32 *t=(ppluint32 *)g->target;
	int pitch=g->pitch>>2;
	SurfaceColor c=g->color;
	int v;
	for (int yy=0;yy<height;yy++) {
		for (int xx=0;xx<width;xx++) {
			v=bitmap[0];
			if (v) t[xx]=c;
			bitmap++;
		}
		t+=pitch;
	}
}
}
#endif

static int DrawGlyphMono8(DRAWABLE_DATA &data, const char *glyph, int x, int y, SurfaceColor c)
{
	pplint16 width=Peek16(glyph);
	pplint16 height=Peek16(glyph+2);
	pplint16 bearingx=Peek16(glyph+4);
	pplint16 bearingy=Peek16(glyph+6);
	int y1=y;
	int x1=x;
	x=x1+bearingx;
	y=y1-bearingy;

	const char *bitmap=glyph+10;
	int v;
	for (int yy=0;yy<height;yy++) {
		for (int xx=0;xx<width;xx++) {
			v=bitmap[0];
			if (v) data.fn->PutPixel(data,xx+x,yy+y,c);
			bitmap++;
		}
	}
	return 1;
}

static int DrawGlyphMono1(DRAWABLE_DATA &data, const char *glyph, int x, int y, SurfaceColor c)
{
	pplint16 width=Peek16(glyph);
	pplint16 height=Peek16(glyph+2);
	pplint16 bearingx=Peek16(glyph+4);
	pplint16 bearingy=Peek16(glyph+6);
	int y1=y;
	int x1=x;
	x=x1+bearingx;
	y=y1-bearingy;

	const char *bitmap=glyph+10;
	int v=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<height;yy++) {
		for (int xx=0;xx<width;xx++) {
			if (!bitcount) {
				v=bitmap[0];
				bitcount=8;
				bitmap++;
			}
			if (v&128) {
				data.fn->PutPixel(data,xx+x,yy+y,c);
			}
			v=v<<1;
			bitcount--;
		}
	}
	return 1;
}

static int DrawGlyphAA2(DRAWABLE_DATA &data, const char *glyph, int x, int y, SurfaceColor c)
{
	pplint16 width=Peek16(glyph);
	pplint16 height=Peek16(glyph+2);
	pplint16 bearingx=Peek16(glyph+4);
	pplint16 bearingy=Peek16(glyph+6);
	int y1=y;
	int x1=x;
	x=x1+bearingx;
	y=y1-bearingy;

	const char *bitmap=glyph+10;
	int v=0, v2=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<height;yy++) {
		for (int xx=0;xx<width;xx++) {
			if (!bitcount) {
				v=bitmap[0];
				bitcount=8;
				bitmap++;
			}
			if ((v2=v&192)) {
				if (v2==192) data.fn->BlendPixel(data,xx+x,yy+y,c,255);
				else if (v2==128) data.fn->BlendPixel(data,xx+x,yy+y,c,200);		// 200
				else data.fn->BlendPixel(data,xx+x,yy+y,c,100);					// 100
			}
			v=v<<2;
			bitcount-=2;
		}
	}
	return 1;
}

static int DrawGlyphAA4(DRAWABLE_DATA &data, const char *glyph, int x, int y, SurfaceColor c)
{
	pplint16 width=Peek16(glyph);
	pplint16 height=Peek16(glyph+2);
	pplint16 bearingx=Peek16(glyph+4);
	pplint16 bearingy=Peek16(glyph+6);
	int y1=y;
	int x1=x;
	x=x1+bearingx;
	y=y1-bearingy;

	const char *bitmap=glyph+10;
	int v=0, v2=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<height;yy++) {
		for (int xx=0;xx<width;xx++) {
			if (!bitcount) {
				v=bitmap[0];
				bitcount=8;
				bitmap++;
			}
			if ((v2=v&240)) {
				v2=v2>>4;
				data.fn->BlendPixel(data,xx+x,yy+y,c,v2*255/15);
			}
			v=v<<4;
			bitcount-=4;
		}
	}
	return 1;
}

static int DrawGlyphAA8(DRAWABLE_DATA &data, const char *glyph, int x, int y, SurfaceColor c)
{
	pplint16 width=Peek16(glyph);
	pplint16 height=Peek16(glyph+2);
	pplint16 bearingx=Peek16(glyph+4);
	pplint16 bearingy=Peek16(glyph+6);
	int y1=y;
	int x1=x;
	x=x1+bearingx;
	y=y1-bearingy;

	const char *bitmap=glyph+10;
	int v;
	for (int yy=0;yy<height;yy++) {
		for (int xx=0;xx<width;xx++) {
			v=bitmap[0];
			if (v) data.fn->BlendPixel(data,xx+x,yy+y,c,v);
			bitmap++;
		}
	}
	return 1;
}

PFPChunk *FontEngineFont5::selectFont(const FontFile &file, const Font &font)
{
	if (file.priv==NULL) throw NullPointerException();
	PFPFile *f=(PFPFile *)file.priv;
	PFPChunk *c;
	const char *header;

	// Wir mussen zuerst den passenden Chunk finden
	f->myMutex.lock();
	PFPFile::Iterator it;
	f->reset(it);
	int flags=0;
	if (font.antialias()) flags|=1;
	if (font.bold()) flags|=2;
	if (font.italic()) flags|=4;

	while ((c=f->findNextChunk(it,L"FACE"))) {
		header=(const char*)c->data();
		if (!header) continue;
		if (((int)Peek16(header+2))==(int)font.size() && (int)Peek8(header)==flags) {
			break;
		}
	}
	f->myMutex.unlock();
	return c;
}

static const char *FindJumpTable(const char *header, int *start, int *end, int code)
{
	int numJumpTables=Peek16(header+10);
	const char *jumpindex=header+12;
	for (int ii=0;ii<numJumpTables;ii++) {
		*start=Peek16(jumpindex);
		*end=Peek16(jumpindex+2);
		if (code>=*start && code<=*end) {
			return header+Peek32(jumpindex+4);
		}
		jumpindex+=8;
	}
	return NULL;
}

void FontEngineFont5::render(const FontFile &file, const Font &font, Drawable &draw, int x, int y, const String &text, const Color &color)
{
	PFPChunk *c=selectFont(file,font);
	if (c) renderInternal(c,font,draw,x,y,text,color);
	if (font.bold()) {
		Font f=font;
		f.setBold(false);
		c=selectFont(file,f);
		if (c) {
			renderInternal(c,font,draw,x,y,text,color);
			return renderInternal(c,font,draw,x+1,y,text,color);
		}
	}
	throw InvalidFontException();
}

void FontEngineFont5::renderInternal(PFPChunk *c, const Font &font, Drawable &draw, int x, int y, const String &text, const Color &color)
{
	DRAWABLE_DATA *data=draw.getData();
	const char *header=(char*)c->data();
	const char *jump=NULL;
	const char *glyph;
	int pixelformat=Peek8(header+1);
	int code;
	int start=0;
	int end=0;
	pplint16 bearingy, bearingx, width, height, advance;
	pplint16 MaxHeight, MaxBearingY;
	MaxHeight=Peek16(header+6);
	MaxBearingY=Peek16(header+4);

	int lastx=x;
	int lasty=y;

	int (*BltGlyph) (GLYPH *surface)=NULL;
	int (*ErsatzGlyph) (DRAWABLE_DATA &data, const char *glyph, int x, int y, SurfaceColor c)=NULL;
	GLYPH g;
	g.color=draw.rgb(color);
	switch (pixelformat) {
		case 1:				// Monochrom, 8 Bit pro Pixel
			switch (draw.rgbformat()) {
				case RGBFormat::X8R8G8B8:
				case RGBFormat::X8B8G8R8:
				case RGBFormat::A8R8G8B8:
				case RGBFormat::A8B8G8R8:
					#ifdef HAVE_X86_ASSEMBLER
						BltGlyph=BltGlyph_M8_32;
					#endif
					break;

			};
			ErsatzGlyph=DrawGlyphMono8;
			break;
		case 2:				// Monochrom, 1 Bit pro Pixel
			switch (draw.rgbformat()) {
				case RGBFormat::X8R8G8B8:
				case RGBFormat::X8B8G8R8:
				case RGBFormat::A8R8G8B8:
				case RGBFormat::A8B8G8R8:
					#ifdef HAVE_X86_ASSEMBLER
						BltGlyph=BltGlyph_M1_32;
					#endif
					break;

			};
			ErsatzGlyph=DrawGlyphMono1;
			break;
		case 3:				// Antialiased, 8 Bit pro Pixel
			switch (draw.rgbformat()) {
				case RGBFormat::X8R8G8B8:
				case RGBFormat::X8B8G8R8:
				case RGBFormat::A8R8G8B8:
				case RGBFormat::A8B8G8R8:
					#ifdef HAVE_X86_ASSEMBLER
						//BltGlyph=BltGlyph_AA8_32;
					#endif
					break;
			};
			ErsatzGlyph=DrawGlyphAA8;
			break;
		case 4:				// Antialiased, 2 Bit pro Pixel
			switch (draw.rgbformat()) {
				case RGBFormat::X8R8G8B8:
				case RGBFormat::X8B8G8R8:
				case RGBFormat::A8R8G8B8:
				case RGBFormat::A8B8G8R8:
					#ifdef HAVE_X86_ASSEMBLER
						BltGlyph=BltGlyph_AA2_32;
					#endif
					break;
			};
			ErsatzGlyph=DrawGlyphAA2;
			break;
		case 5:				// Antialiased, 4 Bit pro Pixel
			switch (draw.rgbformat()) {
				case RGBFormat::X8R8G8B8:
				case RGBFormat::X8B8G8R8:
				case RGBFormat::A8R8G8B8:
				case RGBFormat::A8B8G8R8:
					#ifdef HAVE_X86_ASSEMBLER
						BltGlyph=BltGlyph_AA4_32;
					#endif
					break;
			};
			ErsatzGlyph=DrawGlyphAA4;
			break;
		default:
			throw InvalidFontException();
	};
	int orgx=x;
	int p=0;
	bool drawn;
	if (font.orientation()==Font::TOP) {
		lasty+=MaxBearingY;
	}

	while ((code=text[p])) {
		//printf ("code[%i]= %i\n",p,code);
		p++;
		if (code==10) {											// Newline
			lastx=orgx;
			lasty+=MaxHeight;
		} else if (code=='\t') {					// Tab
			if (32<start || 32>end) jump=FindJumpTable(header,&start,&end,32);
			if (!jump) continue;
			// Glyph holen
			glyph=header+Peek32(jump+((32-start)<<2));
			if (glyph) {
				advance=Peek16(glyph+8);
				lastx+=advance*4;
			}
		} else {											// Rest
			if (code<start || code>end) jump=FindJumpTable(header,&start,&end,code);
			if (!jump) continue;
			// Glyph holen
			glyph=header+Peek32(jump+((code-start)<<2));
			if (glyph) {
				drawn=false;
				advance=Peek16(glyph+8);
				if (BltGlyph) {
					width=Peek16(glyph);
					height=Peek16(glyph+2);
					bearingy=Peek16(glyph+6);
					bearingx=Peek16(glyph+4);
					x=lastx+bearingx;
					y=lasty-bearingy;
					if (x>=0 && x+width<draw.width()
							&& y>=0 && y+height<draw.height()) {
						//if (y>590)
						//	printf("Char: %i, x: %i, y: %i, height: %i, clipper.bottom: %i, bearingy: %i\n",code,x,y,height,s->clipper.bottom,bearingy);
						g.data=glyph;
						g.pitch=draw.pitch();
						g.target=(char*)draw.adr(x,y);
						//HexDump(&g,sizeof(g));
						if (BltGlyph(&g)) {
							drawn=true;
						}
					}
				}
				if (!drawn) ErsatzGlyph(*data,glyph,lastx,lasty,g.color);
				lastx+=advance;
			}
		}
	}
}

Size FontEngineFont5::measure(const FontFile &file, const Font &font, const String &text)
{
	Size s;
	if (file.priv==NULL) throw NullPointerException();
	//PFPFile *f=(PFPFile *)file->priv;
	PFPChunk *c=selectFont(file,font);
	if (!c) return s;
	const char *header=(char*)c->data();
	const char *jump=NULL;
	const char *glyph;
	int code;
	int start=0;
	int end=0;
	pplint16 bearingy, bearingx, width, height, advance;
	pplint16 MaxHeight, MaxBearingY;
	MaxHeight=Peek16(header+6);
	MaxBearingY=Peek16(header+4);

	int p=0;
	int miny=0, maxy=0;
	int x=0,y=0;
	while ((code=text[p])) {
		p++;
		if (code==10) {											// Newline
			y+=MaxHeight;
			x=0;
		} else if (code=='\t') {								// Tab
			if (32<start || 32>end) jump=FindJumpTable(header,&start,&end,32);
			if (!jump) continue;
			// Glyph holen
			glyph=header+Peek32(jump+((32-start)<<2));
			if (glyph) {
				advance=Peek16(glyph+8);
				x+=advance*4;
			}
		} else { 												// Rest
			if (code<start || code>end) jump=FindJumpTable(header,&start,&end,code);
			if (!jump) continue;
			// Glyph holen
			glyph=header+Peek32(jump+((code-start)<<2));
			if (glyph) {
				width=Peek16(glyph);
				height=Peek16(glyph);
				bearingy=Peek16(glyph+6);
				bearingx=Peek16(glyph+4);
				advance=Peek16(glyph+8);
				x+=advance;
				if (y-bearingy < miny) miny=y-bearingy;
				if (y-bearingy+height > maxy) maxy=y-bearingy+height;
				if (x>s.width) s.width=x;
			}
		}
	}
	s.setHeight(maxy-miny+1);
	return s;
}





} // EOF namespace grafix
} // EOF namespace ppl7


