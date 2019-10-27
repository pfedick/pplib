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

#include "prolog_ppl7.h"
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

String FontEngineFreeType::name() const
{
	return "FontEngineFreeType";
}

String FontEngineFreeType::description() const
{
	return "Rendering of TrueType and OpenType fonts";
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

int FontEngineFreeType::ident(FileObject &file) throw()
{
#ifndef HAVE_FREETYPE2
	return 0;
#else
	FREETYPE_ENGINE_DATA *f=(FREETYPE_ENGINE_DATA*)ft;
	if (!f) return 0;
	const FT_Byte *buffer=(const FT_Byte *)file.map();
	size_t size=file.size();
	FT_Face face;
	int error = FT_New_Memory_Face(f->ftlib, buffer, (FT_Long)size, 0, &face );
	if (error!=0) return 0;
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
	int error = FT_New_Memory_Face(f->ftlib, face->buffer, (FT_Long)size, 0, &face->face );
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


static void putPixel(Drawable &draw, int x, int y, const Color &color, int intensity)
{
	Color vg=color;
	Color bg=draw.getPixel(x,y);
	int a=vg.alpha()*intensity/255;
	if (a==0) return;
	if (a==255) {
		vg.setAlpha(bg.alpha()+a);
		draw.putPixel(x,y,vg);
		return;
	}

	int reva=255-a;
	int red=(bg.red()*reva+vg.red()*a)/255;
	int green=(bg.green()*reva+vg.green()*a)/255;
	int blue=(bg.blue()*reva+vg.blue()*a)/255;
	int alpha=bg.alpha()+a;
	draw.putPixel(x,y,Color(red,green,blue,alpha));
}

#ifdef HAVE_FREETYPE2
static void renderGlyphAA(Drawable &draw, FT_Bitmap *bitmap, int x, int y, const Color &color)
{
	ppluint8 v=0;
	ppluint8 *glyph=(ppluint8 *)bitmap->buffer;
	for (unsigned int gy=0;gy<bitmap->rows;gy++) {
		for (unsigned int gx=0;gx<bitmap->width;gx++) {
			v=glyph[gx];
			if (v>0) {
				putPixel(draw,x+gx,y+gy,color,v);
				//draw.blendPixel(x+gx,y+gy,color,v);
			}
		}
		glyph+=bitmap->pitch;
	}
}

static void renderGlyphMono(Drawable &draw, FT_Bitmap *bitmap, int x, int y, const Color &color)
{
	ppluint8 v=0;
	ppluint8 *glyph=(ppluint8 *)bitmap->buffer;
	for (unsigned int gy=0;gy<bitmap->rows;gy++) {
		ppluint8 bitcount=0;
		ppluint8 bytecount=0;
		for (unsigned int gx=0;gx<bitmap->width;gx++) {
			if (!bitcount) {
				v=glyph[bytecount];
				bitcount=8;
				bytecount++;
			}
			if(v&128) {
				putPixel(draw,x+gx,y+gy,color,255);
				//draw.alphaPixel(x+gx,y+gy,color);
			}
			v=v<<1;
			bitcount--;
		}
		glyph+=bitmap->pitch;
	}
}

#endif

void FontEngineFreeType::render(const FontFile &file, const Font &font, Drawable &draw, int x, int y, const WideString &text, const Color &color)
{
#ifndef HAVE_FREETYPE2
	throw UnsupportedFeatureException("Freetype2");
#else
	if (file.priv==NULL) throw InvalidFontException();
	FREETYPE_FACE_DATA *face=(FREETYPE_FACE_DATA*)file.priv;
	int error=FT_Set_Pixel_Sizes(face->face,0,font.size()+2);
	if (error!=0) throw InvalidFontException();

	int orgx=x<<6;
	int orgy=y<<6;
	int lastx=orgx;
	bool rotate=false;
	FT_Matrix matrix; /* transformation matrix */
	if (font.rotation()!=0.0) {
		rotate=true;
		double angle=font.rotation()*3.14159265359/180.0;
		/* set up matrix */
		matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
		matrix.xy = (FT_Fixed)( sin( angle ) * 0x10000L );
		matrix.yx = (FT_Fixed)( -sin( angle ) * 0x10000L );
		matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );
		FT_Set_Transform( face->face, &matrix, NULL );
	} else {
		FT_Set_Transform( face->face, NULL, NULL );
	}

	FT_GlyphSlot slot=face->face->glyph;
	FT_UInt			glyph_index, last_glyph=0;
	FT_Vector		kerning;
	kerning.x=0;
	kerning.y=0;

	size_t p=0;
	size_t textlen=text.len();

	while (p<textlen) {
		int code=text[p];
		p++;
		if (code==10) {											// Newline
			lastx=orgx;
			orgy+=(font.size()+2)<<6;
			last_glyph=0;
		} else {
			y=orgy;
			x=lastx;
			glyph_index=FT_Get_Char_Index(face->face,code);
			if (!glyph_index) continue;
			// Antialiasing
			if (font.antialias()) {
				error=FT_Load_Glyph(face->face,glyph_index,FT_LOAD_DEFAULT|FT_LOAD_RENDER|FT_LOAD_TARGET_NORMAL);
			} else {
				error=FT_Load_Glyph(face->face,glyph_index,FT_LOAD_DEFAULT|FT_LOAD_TARGET_MONO|FT_LOAD_RENDER);
			}
			if (error!=0) continue;
			//x=x+(slot->bitmap_left<<6);
			//y=y-(slot->bitmap_top<<6);
			if (face->kerning>0 && last_glyph>0 && rotate==false) {
				FT_Get_Kerning(face->face,last_glyph,glyph_index,FT_KERNING_DEFAULT,&kerning);
				x+=kerning.x;
				y+=kerning.y;
			}

			if (font.antialias()) {
				renderGlyphAA(draw,&slot->bitmap,(x>>6)+slot->bitmap_left,
						(y>>6)-slot->bitmap_top,color);
			} else {
				renderGlyphMono(draw,&slot->bitmap,(x>>6)+slot->bitmap_left,
						(y>>6)-slot->bitmap_top,color);
			}
			if (font.drawUnderline()) {

			}
			lastx=x+slot->advance.x;
			orgy-=slot->advance.y;
			last_glyph=glyph_index;
		}
	}
#endif
}

Size FontEngineFreeType::measure(const FontFile &file, const Font &font, const WideString &text)
{
	Size s;
#ifndef HAVE_FREETYPE2
	throw UnsupportedFeatureException("Freetype2");
#else
	if (file.priv==NULL) throw InvalidFontException();
	FREETYPE_FACE_DATA *face=(FREETYPE_FACE_DATA*)file.priv;
	int error=FT_Set_Pixel_Sizes(face->face,0,font.size()+2);
	if (error!=0) throw InvalidFontException();

	FT_Set_Transform( face->face, NULL, NULL );

	int width=0,height=0;

	FT_GlyphSlot slot=face->face->glyph;
	FT_UInt			glyph_index, last_glyph=0;
	FT_Vector		kerning;
	kerning.x=0;
	kerning.y=0;
	size_t p=0;
	size_t textlen=text.len();
	while (p<textlen) {
		int code=text[p];
		p++;
		if (code==10) {											// Newline
			width=0;
			height+=(font.size()+2);
			last_glyph=0;
		} else {
			glyph_index=FT_Get_Char_Index(face->face,code);
			if (!glyph_index) continue;
			// Antialiasing
			if (font.antialias()) {
				error=FT_Load_Glyph(face->face,glyph_index,FT_LOAD_DEFAULT|FT_LOAD_RENDER|FT_LOAD_TARGET_NORMAL);
			} else {
				error=FT_Load_Glyph(face->face,glyph_index,FT_LOAD_DEFAULT|FT_LOAD_TARGET_MONO|FT_LOAD_RENDER);
			}
			if (error!=0) continue;
			//x=x+slot->bitmap_left;
			//y=y-slot->bitmap_top;
			if (face->kerning>0 && last_glyph>0) {
				error=FT_Get_Kerning(face->face,last_glyph,glyph_index,FT_KERNING_DEFAULT,&kerning);
				width+=kerning.x;
			}
			width+=(slot->advance.x);
			if (width>s.width) s.width=width;
			last_glyph=glyph_index;
		}
	}
	s.setHeight(height+font.size()+2);
	s.width=s.width>>6;
	return s;
#endif
}


} // EOF namespace grafix
} // EOF namespace ppl7

