/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
 *
 *******************************************************************************
 * Copyright (c) 2012, Patrick Fedick <patrick@pfp.de>
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
#include <map>

#include "ppl6-grafix.h"
#include "grafix6.h"


/*!\page PFPFont6Format Format PFP Font, Version 6
 *
Die Version 6 ist angelehnt an Version 5 und verwendet als Basisformat das
\ref PFPFileVersion3 "PFP-File-Format Version 3" mit seinen Chunks. Die Wesentlichen Unterschiede zu Version 5 sind:
- Es gibt keine Sprungtabellen mehr
- Die Glyphs für die vorhandenen Zeichen liegen hintereinander in Chunks, die Applikation muss sich selbst merken,
wo ein Glyph zu finden ist (z.B. innerhalb eines binären Baumes)
- Die Glyphs unterstützen Hints, also die Information, ob eine bestimmte Zeichenkombination näher zueinander dargestellt
werden können oder mehr Platz brauchen. Beispiel: "To" - das "o" kann in diesem Fall nach links unter den T-Strich
gerückt werden. Bei "Tö" geht das aber ja nach Font nicht.

Das File trägt als ID "FONT", Hauptversion 6, Unterversion 0. Eine Datei enthält in der Regel immer nur
einen Font (z.B. Liberation Sans), kann aber beliebig viele Faces enthalten, die sich durch Style (Fett, Kursiv, Antialiased)
und Pixelgröße unterscheiden. Jedes Face ist in einem eigenen Chunk untergebracht, der den Namen "FACE" trägt.

\par Header
Jedes FACE beginnt mit einem 12 Byte großen Header:
\code
Byte 0:  Flags                                        (1 Byte)
Byte 1:  Pixelformat                                  (1 Byte)
Byte 2:  Size (Fontgröße in Pixel)                    (2 Byte)
Byte 4:  MaxBearingY                                  (2 Byte)
Byte 6:  MaxHeight                                    (2 Byte)
Byte 8:  Position Underscore                          (2 Byte)
Byte 10: Anzahl Glyphs                                (2 Byte)
\endcode
Beschreibung:
<ul>
<li><b>Flags</b>\n
Folgende Flags sind definiert:
\code
Bit 0: Font benutzt Antialiasing
Bit 1: Font ist fett (bold)
Bit 2: Font ist kursiv (italic)
Bit 3: Font unterstützt Hints
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
Da Font6-Fonts immer von einer Grundlinie aus gezeichnet werden, kann man mit diesem
Wert den höchsten Punkt im Font-Face ermitteln, um z.B. statt von der Grundline aus
von oben aus die Fonts abzubilden (Align=Top)</li>
<li><b>MaxHeight</b>\n
Dieser Wert gibt die maximale Höhe der Glyphs an und gleichzeitig die Anzahl Pixel,
die bei einem Zeilenumbruch auf der Y-Achse dazuaddiert werden müssen. Aus der
Differenz "MaxHeight-MaxBearingY" kann man ausserdem den tiefsten Punkt im
Fontface errechnen, für den Fall dass man die Fonts von unten aus abbilden
möchte (Align=Bottom).</li>
<li><b>Position Underscore</b>\n
</li>
<li><b>Anzahl Glyphs</b>\n
Anzahl Glyphs/Zeichen in diesem Face.
</li>

</ul>
\par
Der Aufbau ist somit fast identisch zu Version 5, nur Byte 10 enthält statt der Anzahl Spruntabellen
die Anzahl Glyphs.

\par Glyphs
Ein wesentlicher Unterschied zu Version 5 (und allen vorhergehenden) besteht darin,
dass es keine Sprungtabellen mehr gibt. Stattdessen folgen nun direkt die Chunks mit den Glyphs.
Der Aufbau der Chunks ist folgendermassen:
\code
Byte 0:    Größe des Chunks in Bytes                     (4 Byte)
Byte 4:    Unicode-Wert des Zeichens                     (2 Byte)
Byte 6:    Breite                                        (2 Byte)
Byte 8:    Höhe                                          (2 Byte)
Byte 10:   BearingX (signed)                             (2 Byte)
Byte 12:   BearingY (signed)                             (2 Byte)
Byte 14:   Advance                                       (2 Byte)
Byte 16:   Hints-Tabelle                                 (n Byte)
Byte 16+n: Bitmap                                        (x Byte)

\endcode
Beschreibung:
<ul>
	<li><b>Größe des Chunks</b>\n
	Größe des Chunks in Byte einschließlich dieses Werts
	</li>
	<li><b>Unicode-Wert des Zeichens</b>\n
	Dieser Wert ist 2 Byte gross, das heisst es lassen such nur Zeichen abbilden,
	deren Unicode-Werte zwischen 0 und 65535 liegen.
	</li>
	<li><b>Breite</b>\n
	Breite des Zeichens in Pixel
	</li>
	<li><b>Höhe</b>\n
	Höhe des Zeichens in Pixel
	</li>
	<li><b>BearingX</b>\n
	Wert, der beim Zeichnen dazuaddiert werden muss, bevor die erste Spalte des Zeichens gezeichnet werden kann.
	</li>
	<li><b>BearingY</b>\n
	Wert, der beim Zeichnen dazuaddiert werden muss, bevor die Zeile des Zeichens gezeichnet werden kann.
	</li>
	<li><b>Advance</b>\n
	Wert, der dazuaddiert werden muss, um an die Position des nächsten darzustellenden Zeichens zu kommen.
	Falls es zu der Zeichenkombination Hints gibt, muss dessen Wert zu diesem noch dazuaddiert werden.
	</li>
	<li><b>Hints-Tabelle</b>\n
	Unterstützt der Font Hints (Bit 3 im Flags-Feld des FACE-Headers ist gesetzt),
	schließt sich nun die Hints-Tabele an. Ist dies nicht der Fall, folgt sofort die Bitmap.
	Die Tabelle ist folgendermassen aufgebaut:
	\code
Byte 0: Unicode                   (2 Byte)
Byte 2: Offset (signed)           (2 Byte)
	\endcode
	Unicode gibt den Unicode-Wert des nachfolgenden Zeichens an, Offset die Verschiebung nach
	links oder rechts. Das Ende der Tabelle wird dadurch gekennzeichnet, dass der Unicode-Wert 0 ist.
	Um Platz zu sparen werden in der Tabelle nur Zeichenkombinationen gespeichert, bei denen es auch
	tatsächlich eine Verschiebung gibt.
	</li>
	<li><b>Bitmap</b>\n
	Das Format der Bitmap ergibt sich aus dem Pixelformat im FACE-Header.
	Die Größe errechnet sich aus <i>BitsProPixel * Breite * Höhe</i>, wobei auf volle Byte aufgerundet wird.
	</li>

</ul>
Aufgrund dieses Aufbaus läßt es sich nicht errechnen, an welcher Stelle im File sich ein bestimmtes
Glyph befinden. Die Applikation muss daher alle Glyphs lesen und sich diese Information selbst
erstellen, zum Beispiel innerhalb eines binären Baumes.

 */

// Font-Blitter
typedef struct tagRenderContext {
	const char *bitmap;
	char *target;
	ppluint32 pitch;
	pplint32 color;
	pplint16 width;
	pplint16 height;
} RENDER_CONTEXT;


namespace ppl6 {
namespace grafix {


class Font6Glyph
{
	private:
	public:
		int width;
		int height;
		int bearingX;
		int bearingY;
		int advance;
		const char *bitmap;
		std::map<wchar_t,int> Hints;
		int getHint(wchar_t nextGlyph) const;
};

class Font6Face
{
	private:

	public:
		int	Flags;
		int Pixelformat;
		int Size;
		int MaxBearingY;
		int MaxHeight;
		int Underscore;
		std::map<wchar_t,Font6Glyph> Glyphs;

		const Font6Glyph* getGlyph(wchar_t code) const;

};

class Font6Renderer
{
	private:
		PFPFile ff;
		std::map<ppluint32,Font6Face> Faces;
		CString Name, Author, Copyright, Description;

		void loadFace(const char *data, size_t size);
		void loadGlyph(Font6Face &Face, const char *data, size_t size);

		const Font6Face* getFace(int size, int flags);

		void renderInternal(const Font6Face &face, grafix::CDrawable &draw, const CFont &font, int x, int y, const CWString &text, const Color &color);



	public:
		Font6Renderer();
		~Font6Renderer();

		void loadFont(const CString &filename);
		void loadFont(CFileObject &file);

		const CString & name() const;
		const CString & author() const;
		const CString & copyright() const;
		const CString & description() const;
		size_t numFaces() const;


		void	render(grafix::CDrawable &draw, const CFont &font, int x, int y, const CWString &text, const Color &color);
		Size	measure(const CFont &font, const CWString &text);


	// Exceptions

};


static void DrawGlyphMono8_0(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			v=bitmap[0];
			if (v) data.fn->PutPixel(data,xx+x,yy+y,c);
			bitmap++;
		}
	}
}

static void DrawGlyphMono8_90(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			v=bitmap[0];
			if (v) data.fn->PutPixel(data,x-yy,y+xx,c);
			bitmap++;
		}
	}
}

static void DrawGlyphMono8_180(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			v=bitmap[0];
			if (v) data.fn->PutPixel(data,x-xx,y-yy,c);
			bitmap++;
		}
	}
}

static void DrawGlyphMono8_270(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			v=bitmap[0];
			if (v) data.fn->PutPixel(data,x+yy,y-xx,c);
			bitmap++;
		}
	}
}


static void DrawGlyphMono1_0(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
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
}

static void DrawGlyphMono1_90(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			if (!bitcount) {
				v=bitmap[0];
				bitcount=8;
				bitmap++;
			}
			if (v&128) {
				data.fn->PutPixel(data,x-yy,y+xx,c);
			}
			v=v<<1;
			bitcount--;
		}
	}
}

static void DrawGlyphMono1_180(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			if (!bitcount) {
				v=bitmap[0];
				bitcount=8;
				bitmap++;
			}
			if (v&128) {
				data.fn->PutPixel(data,x-xx,y-yy,c);
			}
			v=v<<1;
			bitcount--;
		}
	}
}

static void DrawGlyphMono1_270(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			if (!bitcount) {
				v=bitmap[0];
				bitcount=8;
				bitmap++;
			}
			if (v&128) {
				data.fn->PutPixel(data,x+yy,y-xx,c);
			}
			v=v<<1;
			bitcount--;
		}
	}
}

static void DrawGlyphAA2_0(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v=0, v2=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
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
}

static void DrawGlyphAA2_90(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v=0, v2=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			if (!bitcount) {
				v=bitmap[0];
				bitcount=8;
				bitmap++;
			}
			if ((v2=v&192)) {
				if (v2==192) data.fn->BlendPixel(data,x-yy,y+xx,c,255);
				else if (v2==128) data.fn->BlendPixel(data,x-yy,y+xx,c,200);		// 200
				else data.fn->BlendPixel(data,x-yy,y+xx,c,100);					// 100
			}
			v=v<<2;
			bitcount-=2;
		}
	}
}

static void DrawGlyphAA2_180(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v=0, v2=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			if (!bitcount) {
				v=bitmap[0];
				bitcount=8;
				bitmap++;
			}
			if ((v2=v&192)) {
				if (v2==192) data.fn->BlendPixel(data,x-xx,y-yy,c,255);
				else if (v2==128) data.fn->BlendPixel(data,x-xx,y-yy,c,200);		// 200
				else data.fn->BlendPixel(data,x-xx,y-yy,c,100);					// 100
			}
			v=v<<2;
			bitcount-=2;
		}
	}
}

static void DrawGlyphAA2_270(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v=0, v2=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			if (!bitcount) {
				v=bitmap[0];
				bitcount=8;
				bitmap++;
			}
			if ((v2=v&192)) {
				if (v2==192) data.fn->BlendPixel(data,x+yy,y-xx,c,255);
				else if (v2==128) data.fn->BlendPixel(data,x+yy,y-xx,c,200);		// 200
				else data.fn->BlendPixel(data,x+yy,y-xx,c,100);					// 100
			}
			v=v<<2;
			bitcount-=2;
		}
	}
}


static void DrawGlyphAA4_0(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v=0, v2=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
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
}

static void DrawGlyphAA4_90(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v=0, v2=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			if (!bitcount) {
				v=bitmap[0];
				bitcount=8;
				bitmap++;
			}
			if ((v2=v&240)) {
				v2=v2>>4;
				data.fn->BlendPixel(data,x-yy,y+xx,c,v2*255/15);
			}
			v=v<<4;
			bitcount-=4;
		}
	}
}

static void DrawGlyphAA4_180(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v=0, v2=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			if (!bitcount) {
				v=bitmap[0];
				bitcount=8;
				bitmap++;
			}
			if ((v2=v&240)) {
				v2=v2>>4;
				data.fn->BlendPixel(data,x-xx,y-yy,c,v2*255/15);
			}
			v=v<<4;
			bitcount-=4;
		}
	}
}

static void DrawGlyphAA4_270(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v=0, v2=0;
	ppluint8 bitcount=0;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			if (!bitcount) {
				v=bitmap[0];
				bitcount=8;
				bitmap++;
			}
			if ((v2=v&240)) {
				v2=v2>>4;
				data.fn->BlendPixel(data,x+yy,y-xx,c,v2*255/15);
			}
			v=v<<4;
			bitcount-=4;
		}
	}
}


static void DrawGlyphAA8_0(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			v=bitmap[0];
			if (v) data.fn->BlendPixel(data,xx+x,yy+y,c,v);
			bitmap++;
		}
	}
}

static void DrawGlyphAA8_90(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			v=bitmap[0];
			if (v) data.fn->BlendPixel(data,xx+x,yy+y,c,v);
			bitmap++;
		}
	}
}

static void DrawGlyphAA8_180(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			v=bitmap[0];
			if (v) data.fn->BlendPixel(data,xx+x,yy+y,c,v);
			bitmap++;
		}
	}
}
static void DrawGlyphAA8_270(DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)
{
	const char *bitmap=glyph.bitmap;
	int v;
	for (int yy=0;yy<glyph.height;yy++) {
		for (int xx=0;xx<glyph.width;xx++) {
			v=bitmap[0];
			if (v) data.fn->BlendPixel(data,xx+x,yy+y,c,v);
			bitmap++;
		}
	}
}

Font6Renderer::Font6Renderer()
{

}

Font6Renderer::~Font6Renderer()
{

}

const CString & Font6Renderer::name() const
{
	return Name;
}

const CString & Font6Renderer::author() const
{
	return Author;
}

const CString & Font6Renderer::copyright() const
{
	return Copyright;
}

const CString & Font6Renderer::description() const
{
	return Description;
}

size_t Font6Renderer::numFaces() const
{
	return Faces.size();
}

void Font6Renderer::loadFont(const CString &filename)
{
	CFile file;
	file.Open(filename,"rb");
	loadFont(file);
}

void Font6Renderer::loadFont(CFileObject &file)
{
	if (!ff.Ident(file)) throw InvalidFontFormatException(file.GetFilename());
	if (ToString("%s",ff.GetID())!="FONT") throw InvalidFontFormatException(file.GetFilename());
	if (ff.GetMainVersion()!=6 || ff.GetSubVersion()!=0) throw InvalidFontFormatException(file.GetFilename());
	ff.Load(&file);
	Name=ff.GetName();
	Author=ff.GetAuthor();
	Copyright=ff.GetCopyright();
	Description=ff.GetDescription();
	// Die Faces laden
	ff.Reset();
	PFPChunk *c;
	while ((c=ff.FindNextChunk("FACE"))) {
		loadFace((const char*)c->Data(),c->Size());
	}

}

void Font6Renderer::loadFace(const char *data, size_t size)
{
	if (size<12) throw InvalidFontFaceException(Name);
	Font6Face Face;
	Face.Flags=Peek8(data+0);
	Face.Pixelformat=Peek8(data+1);
	Face.Size=Peek16(data+2);
	Face.MaxBearingY=Peek16(data+4);
	Face.MaxHeight=Peek16(data+6);
	Face.Underscore=Peek16(data+8);
	size_t numGlyphs=Peek16(data+10);
	ppluint32 id;
	id=(Face.Flags&7)<<16;
	id|=Face.Size;
	size_t p=12;
	//printf ("Lade Face mit id %i, Size: %i, Flags: %i, Bytes: %zi\n",id,Face.Size,Face.Flags,size);

	// Wir fügen zuerst das Face in die Map ein
	Faces.insert(std::pair<ppluint32,Font6Face>(id,Face));

	// Damit wir im nächsten Schritt die Glyphen direkt innerhalb des Eintrags in der Map
	// ergänzen können, was uns eine ganze Menge Kopiererei erspart
	Font6Face &mapFace=Faces[id];
	//printf ("Num Glyphs: %zi\n",numGlyphs);
	for (size_t i=0;i<numGlyphs;i++) {
		size_t s=Peek32(data+p);
		loadGlyph(mapFace,data+p,s);
		p+=s;
	}
	//printf ("%zi Bytes gelesen\n",p);
}

void Font6Renderer::loadGlyph(Font6Face &Face, const char *data, size_t size)
{
	Font6Glyph NewGlyph;
	wchar_t unicode=Peek16(data+4);
	// Um spätere Kopiererei bei einfügen des Glyphs in die Map zu vermeiden,
	// fügen wir den Glyph zuerst in die Map ein
	Face.Glyphs.insert(std::pair<wchar_t,Font6Glyph>(unicode,NewGlyph));
	// Und arbeiten dann auf dem Objekt innerhalb der Map
	Font6Glyph &Glyph=Face.Glyphs[unicode];
	Glyph.width=Peek16(data+6);
	Glyph.height=Peek16(data+8);
	Glyph.bearingX=(short)Peek16(data+10);
	Glyph.bearingY=(short)Peek16(data+12);
	Glyph.advance=Peek16(data+14);
	size_t p=16;
	if (Face.Flags&8) {		// Wir haben Hints
		wchar_t c;
		while ((c=Peek16(data+p))) {
			Glyph.Hints.insert(std::pair<wchar_t,int>(c,(short)Peek16(data+p+2)));
			p+=4;
		}
		p+=4;
	}
	Glyph.bitmap=data+p;
	/*
	printf ("Reading Glyph: Size: %zi, Unicode: %i = %lc, width: %i, height: %i, advance: %i, Hints: %zi\n",
			size,unicode,unicode,Glyph.width,Glyph.height, Glyph.advance, Glyph.Hints.size());
			*/
}

const Font6Face* Font6Renderer::getFace(int size, int flags)
{
	ppluint32 id;
	id=(flags&7)<<16;
	id|=(size&0xffff);
	//printf("Suche id %i aus %zi Faces\n",id,Faces.size());
	std::map<ppluint32,Font6Face>::const_iterator it;
	it=Faces.find(id);
	if (it==Faces.end()) return NULL;
	return &it->second;
}

const Font6Glyph* Font6Face::getGlyph(wchar_t code) const
{
	std::map<wchar_t,Font6Glyph>::const_iterator it;
	it=Glyphs.find(code);
	if (it==Glyphs.end()) return NULL;
	return &it->second;
}

int Font6Glyph::getHint(wchar_t nextGlyph) const
{
	if (Hints.size()==0) return 0;
	std::map<wchar_t,int>::const_iterator it;
	it=Hints.find(nextGlyph);
	if (it==Hints.end()) return 0;
	return it->second;
}

void Font6Renderer::render(grafix::CDrawable &draw, const CFont &font, int x, int y, const CWString &text, const Color &color)
{
	int flags=0;
	if (font.antialias()) flags|=1;
	if (font.bold()) flags|=2;
	if (font.italic()) flags|=4;
	const Font6Face *face=getFace(font.size(),flags);
	if (face) {
		renderInternal(*face,draw,font,x,y,text, color);
		return;
	}
	// Fallback für fette Schrift, wenn kein Face für Bold enthalten ist
	if (flags&2) {
		flags-=2;
		face=getFace(font.size(),flags);
		if (face) {
			renderInternal(*face,draw,font,x,y,text,color);
			renderInternal(*face,draw,font,x+1,y,text,color);
			return;
		}
	}
	throw UnknownFontFaceException();
}

static void getBlitter(const Font6Face &face, const grafix::CDrawable &draw, int rotate, void (**BltGlyph) (RENDER_CONTEXT *context), void (**ErsatzGlyph) (DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c))
{
	*BltGlyph=NULL;
	*ErsatzGlyph=NULL;
	switch (face.Pixelformat) {
		case 1:				// Monochrom, 8 Bit pro Pixel
			switch (draw.rgbformat()) {
				case RGBFormat::X8R8G8B8:
				case RGBFormat::X8B8G8R8:
				case RGBFormat::A8R8G8B8:
				case RGBFormat::A8B8G8R8:
					#ifdef HAVE_X86_ASSEMBLER
						//BltGlyph=BltGlyph_M8_32;
					#endif
					break;

			};
			switch (rotate) {
				case 0: *ErsatzGlyph=DrawGlyphMono8_0; break;
				case 90: *ErsatzGlyph=DrawGlyphMono8_90; break;
				case 180: *ErsatzGlyph=DrawGlyphMono8_180; break;
				case 270: *ErsatzGlyph=DrawGlyphMono8_270; break;
			}

			break;
		case 2:				// Monochrom, 1 Bit pro Pixel
			switch (draw.rgbformat()) {
				case RGBFormat::X8R8G8B8:
				case RGBFormat::X8B8G8R8:
				case RGBFormat::A8R8G8B8:
				case RGBFormat::A8B8G8R8:
					#ifdef HAVE_X86_ASSEMBLER
						//BltGlyph=BltGlyph_M1_32;
					#endif
					break;

			};
			switch (rotate) {
				case 0: *ErsatzGlyph=DrawGlyphMono1_0; break;
				case 90: *ErsatzGlyph=DrawGlyphMono1_90; break;
				case 180: *ErsatzGlyph=DrawGlyphMono1_180; break;
				case 270: *ErsatzGlyph=DrawGlyphMono1_270; break;
			}
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
			switch (rotate) {
				case 0: *ErsatzGlyph=DrawGlyphAA8_0; break;
				case 90: *ErsatzGlyph=DrawGlyphAA8_90; break;
				case 180: *ErsatzGlyph=DrawGlyphAA8_180; break;
				case 270: *ErsatzGlyph=DrawGlyphAA8_270; break;
			}

			break;
		case 4:				// Antialiased, 2 Bit pro Pixel
			switch (draw.rgbformat()) {
				case RGBFormat::X8R8G8B8:
				case RGBFormat::X8B8G8R8:
				case RGBFormat::A8R8G8B8:
				case RGBFormat::A8B8G8R8:
					#ifdef HAVE_X86_ASSEMBLER
						//BltGlyph=BltGlyph_AA2_32;
					#endif
					break;
			};
			switch (rotate) {
				case 0: *ErsatzGlyph=DrawGlyphAA2_0; break;
				case 90: *ErsatzGlyph=DrawGlyphAA2_90; break;
				case 180: *ErsatzGlyph=DrawGlyphAA2_180; break;
				case 270: *ErsatzGlyph=DrawGlyphAA2_270; break;
			}
			break;
		case 5:				// Antialiased, 4 Bit pro Pixel
			switch (draw.rgbformat()) {
				case RGBFormat::X8R8G8B8:
				case RGBFormat::X8B8G8R8:
				case RGBFormat::A8R8G8B8:
				case RGBFormat::A8B8G8R8:
					#ifdef HAVE_X86_ASSEMBLER
						//BltGlyph=BltGlyph_AA4_32;
					#endif
					break;
			};
			switch (rotate) {
				case 0: *ErsatzGlyph=DrawGlyphAA4_0; break;
				case 90: *ErsatzGlyph=DrawGlyphAA4_90; break;
				case 180: *ErsatzGlyph=DrawGlyphAA4_180; break;
				case 270: *ErsatzGlyph=DrawGlyphAA4_270; break;
			}
			break;
		default:
			throw InvalidFontException();
	};
}


void Font6Renderer::renderInternal(const Font6Face &face, grafix::CDrawable &draw, const CFont &font, int x, int y, const CWString &text,const Color &color)
{
	DRAWABLE_DATA *data=draw.getData();
	const Font6Glyph *glyph=NULL, *previous=NULL;
	RENDER_CONTEXT rc;
	void (*BltGlyph) (RENDER_CONTEXT *context)=NULL;
	void (*ErsatzGlyph) (DRAWABLE_DATA &data, const Font6Glyph &glyph, int x, int y, SurfaceColor c)=NULL;
	int startx=x;
	int lastx=x;
	int lasty=y;
	int kerningx=0;
	int rotate=(int)font.rotation();
	wchar_t code;
	size_t textlen=text.Len();
	size_t p=0;
	getBlitter(face,draw,rotate, &BltGlyph,&ErsatzGlyph);
	rc.color=draw.rgb(color);
	if (font.orientation()==CFont::TOP) {
		lasty+=face.MaxBearingY;
	}
	while (p<textlen) {
		code=text[p++];
		if (code==10) {								// Newline
			lastx=startx;
			lasty+=face.MaxHeight;
			glyph=NULL;
		} else if (code=='\t') {					// Tab
			glyph=face.getGlyph(32);
			if (glyph) lastx+=4*glyph->advance;
		} else {
			glyph=face.getGlyph(code);
			if (glyph==NULL || (BltGlyph==NULL && ErsatzGlyph==NULL)) {
				x=lastx;
				y=lasty;
				draw.drawRect(x,y-face.MaxBearingY,x+4,y,font.color());
				lastx+=5;
			} else {
				kerningx=0;
				if (previous) kerningx+=previous->getHint(code);
				switch (rotate) {
					case 0:
						x=lastx+glyph->bearingX+kerningx;
						y=lasty-glyph->bearingY;
						ErsatzGlyph(*data,*glyph,x,y,rc.color);
						lastx+=glyph->advance+kerningx;
						break;
					case 90:
						x=lastx+glyph->bearingY;
						y=lasty+glyph->bearingX+kerningx;
						ErsatzGlyph(*data,*glyph,x,y,rc.color);
						lasty+=glyph->advance+kerningx;
						break;
					case 180:
						x=lastx-glyph->bearingX-kerningx;
						y=lasty+glyph->bearingY;
						ErsatzGlyph(*data,*glyph,x,y,rc.color);
						lastx-=glyph->advance+kerningx;
						break;
					case 270:
						x=lastx-glyph->bearingY;
						y=lasty-glyph->bearingX-kerningx;
						ErsatzGlyph(*data,*glyph,x,y,rc.color);
						lasty-=glyph->advance+kerningx;
						break;
				}
			}
		}
		previous=glyph;
	}
}

Size Font6Renderer::measure(const CFont &font, const CWString &text)
{
	Size s;
	return s;
}

// ##################################################################################################################################################################
// ##################################################################################################################################################################
// ##################################################################################################################################################################


/*!\class FontEngineFont6
 * \ingroup PPLGroupGrafik
 * \brief Font-Engine für PFP Version 6 Fonts
 */

FontEngineFont6::FontEngineFont6()
{
}

FontEngineFont6::~FontEngineFont6()
{
}

int FontEngineFont6::Init()
{
	// Es gibt nichts zu tun
	return 1;
}

int FontEngineFont6::Ident(CFileObject *file)
{
	PFPFile ff;
	if (!ff.Ident(*file)) return 0;
	if (ToString("%s",ff.GetID())!="FONT") return 0;
	if (ff.GetMainVersion()==6 && ff.GetSubVersion()==0) {
		return 1;
	}
	return 0;
}

CFontFile *FontEngineFont6::LoadFont(CFileObject *file, const char *fontname)
{
	Font6Renderer *render = new Font6Renderer;
	if (!render) throw OutOfMemoryException();
	try {
		render->loadFont(*file);
	} catch (...) {
		delete render;
		throw;
	}
	CFontFile *ff=new CFontFile;
	if (!ff) {
		delete render;
		throw OutOfMemoryException();
	}
	if (fontname) ff->Name=fontname;
	else ff->Name=render->name();
	ff->engine=this;
	ff->priv=render;
	return ff;
}

int FontEngineFont6::DeleteFont(CFontFile *file)
{
	if (!file) {
		SetError(194,"int CFont5::DeleteFont(CFontFile *file)");
		return 0;
	}
	if (file->engine!=this) {
		SetError(1029);
		return 0;
	}
	Font6Renderer *render=(Font6Renderer *)file->priv;
	delete render;
	file->priv=NULL;
	file->engine=NULL;
	return 1;
}

int FontEngineFont6::Render(CFontFile *file, const CFont &font, CDrawable &draw, int x, int y, const CWString &text, const Color &color)
{
	Font6Renderer *render=(Font6Renderer *)file->priv;
	render->render(draw,font,x,y,text,color);
	return 1;
}

Size FontEngineFont6::Measure(CFontFile *file, const CFont &font, const CWString &text)
{
	Font6Renderer *render=(Font6Renderer *)file->priv;
	return render->measure(font,text);
}





} // EOF namespace grafix
} // EOF namespace ppl7


