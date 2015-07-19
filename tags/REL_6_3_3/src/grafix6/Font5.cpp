/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.31 $
 * $Date: 2009/06/22 13:10:31 $
 * $Id: Font5.cpp,v 1.31 2009/06/22 13:10:31 patrick Exp $
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
#include "grafix6.h"

//#undef HAVE_X86_ASSEMBLER

namespace ppl6 {
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


/*!\class CFontEngineFont5
 * \ingroup PPLGroupGrafik
 * \brief Font-Engine für PFP Version 5 Fonts
 */

CFontEngineFont5::CFontEngineFont5()
{
}

CFontEngineFont5::~CFontEngineFont5()
{
}

int CFontEngineFont5::Init()
{
	return 1;
}

int CFontEngineFont5::Ident(CFileObject *file)
{
	if (!file) {
		SetError(1027);
		return 0;
	}
	PFPSTRUCT pfp;
	if (!ident(*file, &pfp)) {
		SetError(1027);
		return 0;
	}
	if (strncasecmp((char*)pfp.file_id,"FONT",4)!=0) {
		SetError(1027);
		return 0;
	}
	if (pfp.mainversion==5 && pfp.subversion==0) {
		return 1;
	}
	SetError(1027);
	return 0;
}

CFontFile *CFontEngineFont5::LoadFont(CFileObject *file, const char *fontname)
{
	PFPFile *File = new PFPFile;
	if (!File->Load(file)) {
		PushError();
		delete File;
		PopError();
		return NULL;
	}
	if (strcmp(File->GetID(),"FONT")!=0 || File->GetMainVersion()!=5 || File->GetSubVersion()!=0) {
		delete File;
		SetError(1027);
		return NULL;
	}
	CFontFile *ff=new CFontFile;
	if (fontname) ff->Name=fontname;
	else {
		const char *tmp=File->GetName();
		if (!tmp) {
			delete ff;
			delete File;
			SetError(1028);
			return NULL;
		}
		ff->Name=tmp;
	}
	ff->engine=this;
	ff->ptr=File;
	return ff;
}

int CFontEngineFont5::DeleteFont(CFontFile *file)
{
	if (!file) {
		SetError(194,"int CFont5::DeleteFont(CFontFile *file)");
		return 0;
	}
	if (file->engine!=this) {
		SetError(1029);
		return 0;
	}
	PFPFile *f=(PFPFile *)file->ptr;
	if (f) delete f;

	return 1;
}

int CFontEngineFont5::Unselect(CFont *font)
{
	font->priv=NULL;
	return 1;
}

#ifndef HAVE_X86_ASSEMBLER
extern "C" {
void BltGlyph_M8_32 (GLYPH *g)
{
	pplint16 width=peek16(g->data);
	pplint16 height=peek16(g->data+2);
	pplint16 bearingx=peek16(g->data+4);
	pplint16 bearingy=peek16(g->data+6);
	pplint16 advance=peek16(g->data+8);
	char *bitmap=g->data+10;
	ppluint32 *t=(ppluint32 *)g->target;
	int pitch=g->pitch>>2;
	COLOR c=g->color;
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

int DoGlyph(CSurface *surface, SURFACE *s, char *glyph, COLOR c)
{
	pplint16 width=peek16(glyph);
	pplint16 height=peek16(glyph+2);
	pplint16 bearingx=peek16(glyph+4);
	pplint16 bearingy=peek16(glyph+6);
	pplint16 advance=peek16(glyph+8);
	int y1=s->lasty;
	int x1=s->lastx;
	int x=s->lastx+bearingx;
	int y=s->lasty-bearingy;

	char *bitmap=glyph+10;
	int v;
	for (int yy=0;yy<height;yy++) {
		for (int xx=0;xx<width;xx++) {
			v=bitmap[0];
			if (v) surface->PutPixel(xx+x,yy+y,c);
			bitmap++;
		}
	}
	s->lastx=x1+advance;
	s->lasty=y1;
	return 1;
}

int DoGlyphMono1(CSurface *surface, SURFACE *s, char *glyph, COLOR c)
{
	pplint16 width=peek16(glyph);
	pplint16 height=peek16(glyph+2);
	pplint16 bearingx=peek16(glyph+4);
	pplint16 bearingy=peek16(glyph+6);
	pplint16 advance=peek16(glyph+8);
	int y1=s->lasty;
	int x1=s->lastx;
	int x=s->lastx+bearingx;
	int y=s->lasty-bearingy;

	char *bitmap=glyph+10;
	int v=0;
	ppldb bitcount=0;
	for (int yy=0;yy<height;yy++) {
		for (int xx=0;xx<width;xx++) {
			if (!bitcount) {
				v=bitmap[0];
				bitcount=8;
				bitmap++;
			}
			if (v&128) {
				surface->PutPixel(xx+x,yy+y,c);
			}
			v=v<<1;
			bitcount--;
		}
	}
	s->lastx=x1+advance;
	s->lasty=y1;
	return 1;
}

int DoGlyphAA2(CSurface *surface, SURFACE *s, char *glyph, COLOR c)
{
	pplint16 width=peek16(glyph);
	pplint16 height=peek16(glyph+2);
	pplint16 bearingx=peek16(glyph+4);
	pplint16 bearingy=peek16(glyph+6);
	pplint16 advance=peek16(glyph+8);
	int y1=s->lasty;
	int x1=s->lastx;
	int x=s->lastx+bearingx;
	int y=s->lasty-bearingy;

	char *bitmap=glyph+10;
	int v=0, v2=0;
	ppldb bitcount=0;
	for (int yy=0;yy<height;yy++) {
		for (int xx=0;xx<width;xx++) {
			if (!bitcount) {
				v=bitmap[0];
				bitcount=8;
				bitmap++;
			}
			if ((v2=v&192)) {
				if (v2==192) surface->BlendPixel(xx+x,yy+y,c,255);
				else if (v2==128) surface->BlendPixel(xx+x,yy+y,c,200);		// 200
				else surface->BlendPixel(xx+x,yy+y,c,100);					// 100
			}

			v=v<<2;
			bitcount-=2;
		}
	}
	s->lastx=x1+advance;
	s->lasty=y1;
	return 1;
}


int DoGlyphAA(CSurface *surface, SURFACE *s, char *glyph, COLOR c)
{
	pplint16 width=peek16(glyph);
	pplint16 height=peek16(glyph+2);
	pplint16 bearingx=peek16(glyph+4);
	pplint16 bearingy=peek16(glyph+6);
	pplint16 advance=peek16(glyph+8);
	int y1=s->lasty;
	int x1=s->lastx;
	int x=s->lastx+bearingx;
	int y=s->lasty-bearingy;

	char *bitmap=glyph+10;
	int v;
	for (int yy=0;yy<height;yy++) {
		for (int xx=0;xx<width;xx++) {
			v=bitmap[0];
			if (v) surface->BlendPixel(xx+x,yy+y,c,v);
			bitmap++;
		}
	}
	s->lastx=x1+advance;
	s->lasty=y1;
	return 1;
}


PFPChunk *CFontEngineFont5::SelectFont(CFont *font)
{
	if (font==NULL || font->file==NULL || font->file->ptr==NULL) {
		SetError(1033);
		return NULL;
	}
	PFPFile *f=(PFPFile *)font->file->ptr;
	PFPChunk *c=(PFPChunk *)font->priv;
	char *header;
	if (!c) {
		// Wir mussen zuerst den passenden Chunk finden
		f->Mutex.Lock();
		f->Reset();
		int flags=0;
		if (font->Antialias) flags|=1;
		if (font->Bold) flags|=2;
		if (font->Italic) flags|=4;

		while ((c=f->FindNextChunk("FACE"))) {
			header=(char*)c->Data();
			if (!header) continue;
			if (((int)peek16(header+2))==(int)font->Size && (int)peek8(header)==flags) {
				break;
			}
		}
		f->Mutex.Unlock();
	}
	if (!c) {
		SetError(1034);
		return 0;
	}
	return c;
}

int CFontEngineFont5::Render(CFont *font, CSurface *surface, int x, int y, const char *text, COLOR color)
{
	if (!text) {
		SetError(194,"int CFontEngineFont5::Render(CFont *font, CSurface *surface, int x, int y, ==> char *text <==, COLOR color)");
		return 0;
	}
	CWString s=text;
	return Render(font,surface,x,y,&s,color);
}

static char *FindJumpTable(char *header, int *start, int *end, int code)
{
	int numJumpTables=peek16(header+10);
	char *jumpindex=header+12;
	for (int ii=0;ii<numJumpTables;ii++) {
		*start=peek16(jumpindex);
		*end=peek16(jumpindex+2);
		if (code>=*start && code<=*end) {
			return header+peek32(jumpindex+4);
		}
		jumpindex+=8;
	}
	return NULL;
}

int CFontEngineFont5::Render(CFont *font, CSurface *surface, int x, int y, CWString *text, COLOR color)
{
	if (font==NULL || font->file==NULL || font->file->ptr==NULL) {
		SetError(1033);
		return 0;
	}
	SURFACE *s=surface->GetSurfaceParams();
	//PFPFile *f=(PFPFile *)font->file->ptr;
	PFPChunk *c=SelectFont(font);
	if (!c) return 0;
	char *header=(char*)c->Data();
	char *jump=NULL;
	char *glyph;
	int pixelformat=peek8(header+1);
	int code;
	int start=0;
	int end=0;
	pplint16 bearingy, bearingx, width, height, advance;
	pplint16 MaxHeight, MaxBearingY;
	MaxHeight=peek16(header+6);
	MaxBearingY=peekw(header+4);

	s->lastx=x;
	s->lasty=y;
	int (*BltGlyph) (GLYPH *surface)=NULL;
	int (*ErsatzGlyph) (CSurface *surface, SURFACE *s, char *glyph, COLOR c)=NULL;
	GLYPH g;
	g.color=color;
	switch (pixelformat) {
		case 1:				// Monochrom, 8 Bit pro Pixel
			switch (s->rgbformat) {
				case RGBFormat::X8R8G8B8:
				case RGBFormat::X8B8G8R8:
				case RGBFormat::A8R8G8B8:
				case RGBFormat::A8B8G8R8:
					#ifdef HAVE_X86_ASSEMBLER
						BltGlyph=BltGlyph_M8_32;
					#endif
					break;

			};
			ErsatzGlyph=DoGlyph;
			break;
		case 2:				// Monochrom, 1 Bit pro Pixel
			switch (s->rgbformat) {
				case RGBFormat::X8R8G8B8:
				case RGBFormat::X8B8G8R8:
				case RGBFormat::A8R8G8B8:
				case RGBFormat::A8B8G8R8:
					#ifdef HAVE_X86_ASSEMBLER
						BltGlyph=BltGlyph_M1_32;
					#endif
					break;

			};
			ErsatzGlyph=DoGlyphMono1;
			break;
		case 3:				// Antialiased, 8 Bit pro Pixel
			switch (s->rgbformat) {
				case RGBFormat::X8R8G8B8:
				case RGBFormat::X8B8G8R8:
				case RGBFormat::A8R8G8B8:
				case RGBFormat::A8B8G8R8:
					#ifdef HAVE_X86_ASSEMBLER
						BltGlyph=BltGlyph_AA8_32;
					#endif
					break;
			};
			ErsatzGlyph=DoGlyphAA;
			break;
		case 4:				// Antialiased, 2 Bit pro Pixel
			switch (s->rgbformat) {
				case RGBFormat::X8R8G8B8:
				case RGBFormat::X8B8G8R8:
				case RGBFormat::A8R8G8B8:
				case RGBFormat::A8B8G8R8:
					#ifdef HAVE_X86_ASSEMBLER
						BltGlyph=BltGlyph_AA2_32;
					#endif
					break;
			};
			ErsatzGlyph=DoGlyphAA2;
			break;

	};
	int orgx=x;
	int p=0;
	bool drawn;
	if (font->Orientation==ORIENTATION::TOP) {
		s->lasty+=MaxBearingY;
	}

	while ((code=text->GetChar(p))&0xff) {
		p++;
		if (code==10) {											// Newline
			s->lastx=orgx;
			//s->lasty+=(int)((pplint16)peek16(header+6)+2);
			s->lasty+=MaxHeight;
		} else if (code=='\t') {					// Tab
			if (32<start || 32>end) jump=FindJumpTable(header,&start,&end,32);
			if (!jump) continue;
			// Glyph holen
			glyph=header+peek32(jump+((32-start)<<2));
			if (glyph) {
				advance=peek16(glyph+8);
				s->lastx+=advance*4;
			}
		} else {											// Rest
			if (code<start || code>end) jump=FindJumpTable(header,&start,&end,code);
			if (!jump) continue;
			// Glyph holen
			glyph=header+peek32(jump+((code-start)<<2));
			if (glyph) {
				drawn=false;
				if (BltGlyph) {
					width=peek16(glyph);
					height=peek16(glyph+2);
					bearingy=peek16(glyph+6);
					bearingx=peek16(glyph+4);
					advance=peek16(glyph+8);
					x=s->lastx+s->originx+bearingx;
					y=s->lasty+s->originy-bearingy;
					if (x>=s->clipper.left && x+width<s->clipper.right
							&& y>=s->clipper.top && y+height<s->clipper.bottom) {
						//if (y>590)
						//	printf("Char: %i, x: %i, y: %i, height: %i, clipper.bottom: %i, bearingy: %i\n",code,x,y,height,s->clipper.bottom,bearingy);
						g.data=glyph;
						g.target=(char*)s->base8+y*s->pitch8+x*s->bytes_per_pixel;
						g.pitch=s->pitch8;
						if (BltGlyph(&g)) {
							drawn=true;
							s->lastx+=advance;
						}
					}
				}
				if (!drawn) ErsatzGlyph(surface,s,glyph,color);
			}
		}
	}
	return 1;
}

int CFontEngineFont5::Measure(CFont *font, const char *text, DIMENSION *measure)
{
	if (!text) {
		SetError(194,"");
		return 0;
	}
	CWString s=text;
	return Measure(font,&s,measure);
}

int CFontEngineFont5::Measure(CFont *font, CWString *text, DIMENSION *measure)
{
	if (font==NULL || font->file==NULL || font->file->ptr==NULL) {
		SetError(1033);
		return 0;
	}
	PFPChunk *c=SelectFont(font);
	if (!c) return 0;
	char *header=(char*)c->Data();
	char *jump=NULL;
	char *glyph;
	int code;
	int start=0;
	int end=0;
	pplint16 bearingy, bearingx, width, height, advance;
	pplint16 MaxHeight, MaxBearingY;
	MaxHeight=peek16(header+6);
	MaxBearingY=peekw(header+4);

	int p=0;
	int miny=0, maxy=0;
	measure->width=0;
	measure->height=0;
	int x=0,y=0;
	while ((code=text->GetChar(p))) {
		p++;
		if (code==10) {											// Newline
			y+=MaxHeight;
			x=0;
		} else if (code=='\t') {								// Tab
			if (32<start || 32>end) jump=FindJumpTable(header,&start,&end,32);
			if (!jump) continue;
			// Glyph holen
			glyph=header+peek32(jump+((32-start)<<2));
			if (glyph) {
				advance=peek16(glyph+8);
				x+=advance*4;
			}
		} else { 												// Rest
			if (code<start || code>end) jump=FindJumpTable(header,&start,&end,code);
			if (!jump) continue;
			// Glyph holen
			glyph=header+peek32(jump+((code-start)<<2));
			if (glyph) {
				width=peek16(glyph);
				height=peek16(glyph);
				bearingy=peek16(glyph+6);
				bearingx=peek16(glyph+4);
				advance=peek16(glyph+8);
				x+=advance;
				if (y-bearingy < miny) miny=y-bearingy;
				if (y-bearingy+height > maxy) maxy=y-bearingy+height;
				if (x>measure->width) measure->width=x;
			}
		}
	}
	measure->height=maxy-miny+1;
	return 1;
}





} // EOF namespace grafix
} // EOF namespace ppl6
