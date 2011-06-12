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


#ifdef HAVE_FREETYPE2
#include <ft2build.h>
#include FT_FREETYPE_H
#endif


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

/*!\class FontEngineFreeType
 * \ingroup PPLGroupGrafik
 * \brief Font-Engine für von FreeType unterstützte Fonts
 *
 * Diese Engine unterstützt TrueType, OpenType, Type1 und weitere von der Freetype-Library
 * unterstützte Formate.
 *
 * \see
 * http://www.freetype.org/
 */


#ifdef HAVE_FREETYPE2
typedef struct tagFreeTypeEngineData {
	FT_Library	ftlib;
} FREETYPE_ENGINE_DATA;

typedef struct tagFreeTypeFaceData {
	FT_Byte *buffer;
	FT_Face	face;
	int		kerning;
} FREETYPE_FACE_DATA;
#endif

FontEngineFreeType::FontEngineFreeType()
{
	ft=NULL;
#ifdef HAVE_FREETYPE2
#endif
}

FontEngineFreeType::~FontEngineFreeType()
{
#ifdef HAVE_FREETYPE2
	FREETYPE_ENGINE_DATA *f=(FREETYPE_ENGINE_DATA*)ft;
	if (f) {
		FT_Done_FreeType(f->ftlib);
		free(f);
	}
#endif
}

void FontEngineFreeType::init()
{
#ifndef HAVE_FREETYPE2
	throw UnsupportedFeatureException("Freetype2");
#else
	if (ft) return;
	FREETYPE_ENGINE_DATA *f=(FREETYPE_ENGINE_DATA *)malloc(sizeof(FREETYPE_ENGINE_DATA));
	if (!f) throw OutOfMemoryException();
	int error=FT_Init_FreeType(&f->ftlib );
	if (error) {
		free(f);
		throw FontEngineInitializationException();
	}
	ft=f;
#endif
}

int FontEngineFreeType::ident(FileObject &file)
{
#ifndef HAVE_FREETYPE2
	throw UnsupportedFeatureException("Freetype2");
#else
	FREETYPE_ENGINE_DATA *f=(FREETYPE_ENGINE_DATA*)ft;
	if (!f) throw FontEngineUninitializedException();
	const FT_Byte *buffer=(const FT_Byte *)file.map();
	size_t size=file.size();
	FT_Face face;
	int error = FT_New_Memory_Face(f->ftlib, buffer, size, 0, &face );
	if (error!=0) throw InvalidFontException();
	FT_Done_Face(face);
	return 1;
#endif
}

FontFile *FontEngineFreeType::loadFont(FileObject &file, const String &fontname)
{
#ifndef HAVE_FREETYPE2
	throw UnsupportedFeatureException("Freetype2");
#else
	FREETYPE_ENGINE_DATA *f=(FREETYPE_ENGINE_DATA*)ft;
	if (!f) throw FontEngineUninitializedException();
	FREETYPE_FACE_DATA *face=(FREETYPE_FACE_DATA*)malloc(sizeof(FREETYPE_FACE_DATA));
	if (!face) throw OutOfMemoryException();
	face->buffer=(FT_Byte *)file.load();
	size_t size=file.size();
	int error = FT_New_Memory_Face(f->ftlib, face->buffer, size, 0, &face->face );
	if (error!=0) {
		free(face->buffer);
		free(face);
		if (error!=0) throw InvalidFontException();
	}
	String name=fontname;
	if (name.isEmpty()) name.set(face->face->family_name);
	face->kerning=(int)FT_HAS_KERNING(face->face);		// Kerning unterstützt?
	FontFile *ff=new FontFile;
	ff->Name=fontname;
	ff->engine=this;
	ff->priv=face;
	return ff;
#endif
}

void FontEngineFreeType::deleteFont(FontFile *file)
{
#ifndef HAVE_FREETYPE2
	throw UnsupportedFeatureException("Freetype2");
#else
	if (!file) throw NullPointerException();
	if (file->engine!=this) throw InvalidFontEngineException();
	FREETYPE_FACE_DATA *face=(FREETYPE_FACE_DATA*)file->priv;
	if (face) {
		FT_Done_Face(face->face);
		free(face->buffer);
		free(face);
		file->priv=NULL;
	}
	file->engine=NULL;
#endif
}

void FontEngineFreeType::render(const FontFile &file, const Font &font, Drawable &draw, int x, int y, const String &text, const Color &color)
{
#ifndef HAVE_FREETYPE2
	throw UnsupportedFeatureException("Freetype2");
#else
	if (file.priv==NULL) throw InvalidFontException();
	FREETYPE_FACE_DATA *face=(FREETYPE_FACE_DATA*)file.priv;
	int error=FT_Set_Pixel_Sizes(face->face,0,font.size()+2);
	if (error!=0) throw InvalidFontException();

	void (*BltGlyph) (GLYPH *surface)=NULL;
	GLYPH g;
	//Color color=font.color();
	g.color=draw.rgb(color);
	switch (draw.rgbformat()) {
		case RGBFormat::X8R8G8B8:
		case RGBFormat::X8B8G8R8:
		case RGBFormat::A8R8G8B8:
		case RGBFormat::A8B8G8R8:
			//BltGlyph=BltGlyph_M8_32;
			break;
	};
	int lastx=x;
	//int lasty=y;
	int orgx=x;
	int orgy=y;
	int code;
	FT_GlyphSlot slot=face->face->glyph;
	FT_UInt			glyph_index, last_glyph=0;
	FT_Vector		kerning;
	ppluint8 v=0;
	int p=0;
	while ((code=text[p])) {
		p++;
		y=orgy;
		if (code==10) {											// Newline
			lastx=x=orgx;
			orgy+=font.size()+2;
			y=orgy;
			last_glyph=0;
		} else {
			glyph_index=FT_Get_Char_Index(face->face,code);
			if (!glyph_index) continue;
			// Antialiasing
			if (font.antialias()) {
				error=FT_Load_Glyph(face->face,glyph_index,FT_LOAD_DEFAULT|FT_LOAD_RENDER);
			} else {
				error=FT_Load_Glyph(face->face,glyph_index,FT_LOAD_DEFAULT|FT_LOAD_TARGET_MONO|FT_LOAD_RENDER);
			}
			if (error!=0) continue;
			x=x+slot->bitmap_left;
			y=y-slot->bitmap_top;
			if (face->kerning>0 && last_glyph>0) {
				error=FT_Get_Kerning(face->face,last_glyph,glyph_index,FT_KERNING_DEFAULT,&kerning);
				if (error==0) {
					x+=(kerning.x>>6);
				}
			}

			if (BltGlyph) {

			} else {
				char *glyph=(char *)slot->bitmap.buffer;
				if (font.antialias()) {
					for (int gy=0;gy<slot->bitmap.rows;gy++) {
						for (int gx=0;gx<slot->bitmap.width;gx++) {
							v=glyph[gx];
							if (v>0) {
								draw.blendPixel(x+gx,y+gy,color,v);
							}
						}
						glyph+=slot->bitmap.pitch;
					}
				} else {
					ppluint8 bitcount=0;
					ppluint8 bytecount=0;
					for (int gy=0;gy<slot->bitmap.rows;gy++) {
						for (int gx=0;gx<slot->bitmap.width;gx++) {
							if (!bitcount) {
								v=glyph[bytecount];
								bitcount=8;
								bytecount++;
							}
							if(v&128) {
								draw.putPixel(x+gx,y+gy,color);
							}
							v=v<<1;
							bitcount--;
						}
						glyph+=slot->bitmap.pitch;
						bitcount=0;
						bytecount=0;
					}

				}
			}
			x+=(slot->advance.x>>6);
			last_glyph=glyph_index;
		}
	}
#endif
}

Size FontEngineFreeType::measure(const FontFile &file, const Font &font, const String &text)
{
	Size s;
#ifndef HAVE_FREETYPE2
	throw UnsupportedFeatureException("Freetype2");
#else
	if (file.priv==NULL) throw InvalidFontException();
	FREETYPE_FACE_DATA *face=(FREETYPE_FACE_DATA*)file.priv;
	int error=FT_Set_Pixel_Sizes(face->face,0,font.size()+2);
	if (error!=0) throw InvalidFontException();

	int lastx=0;
	int orgx=0;
	int orgy=0;
	int code;
	int x=0,y=0;
	FT_GlyphSlot slot=face->face->glyph;
	FT_UInt			glyph_index, last_glyph=0;
	FT_Vector		kerning;
	int p=0;
	while ((code=text[p])) {
		p++;
		y=orgy;
		if (code==10) {											// Newline
			lastx=x=orgx;
			orgy+=font.size()+2;
			y=orgy;
			last_glyph=0;
		} else {
			glyph_index=FT_Get_Char_Index(face->face,code);
			if (!glyph_index) continue;
			// Antialiasing
			if (font.antialias()) {
				error=FT_Load_Glyph(face->face,glyph_index,FT_LOAD_DEFAULT|FT_LOAD_RENDER);
			} else {
				error=FT_Load_Glyph(face->face,glyph_index,FT_LOAD_DEFAULT|FT_LOAD_TARGET_MONO|FT_LOAD_RENDER);
			}
			if (error!=0) continue;
			x=x+slot->bitmap_left;
			y=y-slot->bitmap_top;
			if (face->kerning>0 && last_glyph>0) {
				error=FT_Get_Kerning(face->face,last_glyph,glyph_index,FT_KERNING_DEFAULT,&kerning);
				if (error==0) {
					x+=(kerning.x>>6);
				}
			}
			x+=(slot->advance.x>>6);
			if (x>s.width) s.width=x;
			last_glyph=glyph_index;
		}
	}
	s.setHeight(orgy+font.size()+2);
	return s;
#endif
}


} // EOF namespace grafix
} // EOF namespace ppl7

