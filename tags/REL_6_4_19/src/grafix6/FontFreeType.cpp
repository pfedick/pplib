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

#include "ppl6-grafix.h"
#include "grafix6.h"

#ifdef HAVE_FREETYPE2
#include <ft2build.h>
#include FT_FREETYPE_H
#endif


//#undef HAVE_X86_ASSEMBLER

namespace ppl6 {
namespace grafix {

/*!\class CFontEngineFreeType
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

CFontEngineFreeType::CFontEngineFreeType()
{
	ft=NULL;
#ifdef HAVE_FREETYPE2
#endif
}

CFontEngineFreeType::~CFontEngineFreeType()
{
#ifdef HAVE_FREETYPE2
	FREETYPE_ENGINE_DATA *f=(FREETYPE_ENGINE_DATA*)ft;
	if (f) {
		FT_Done_FreeType(f->ftlib);
		free(f);
	}
#endif
}

int CFontEngineFreeType::Init()
{
#ifndef HAVE_FREETYPE2
	SetError(1035);
	return 0;
#else
	if (ft) {
		SetError(1036,"FreeType");
		return 0;
	}
	FREETYPE_ENGINE_DATA *f=(FREETYPE_ENGINE_DATA *)malloc(sizeof(FREETYPE_ENGINE_DATA));
	if (!f) {
		SetError(2);
		return 0;
	}
	int error=FT_Init_FreeType(&f->ftlib );
	if (error) {
		free(f);
		SetError(1038,"FreeType");
		return 0;
	}
	ft=f;
	return 1;
#endif
}

int CFontEngineFreeType::Ident(CFileObject *file)
{
#ifndef HAVE_FREETYPE2
	SetError(1035);
	return 0;
#else
	if (!file) {
		SetError(1027);
		return 0;
	}
	FREETYPE_ENGINE_DATA *f=(FREETYPE_ENGINE_DATA*)ft;
	if (!f) {
		SetError(1037,"FreeType");
		return 0;
	}
	const FT_Byte *buffer=(const FT_Byte *)file->Map();
	int size=(int)file->Size();
	FT_Face face;
	int error = FT_New_Memory_Face(f->ftlib, buffer, size, 0, &face );
	if (error!=0) {
		SetError(1027);
		return 0;
	}
	FT_Done_Face(face);
	return 1;
#endif
}

CFontFile *CFontEngineFreeType::LoadFont(CFileObject *file, const char *fontname)
{
#ifndef HAVE_FREETYPE2
	SetError(1035);
	return 0;
#else
	FREETYPE_ENGINE_DATA *f=(FREETYPE_ENGINE_DATA*)ft;
	if (!f) {
		SetError(1037,"FreeType");
		return 0;
	}
	FREETYPE_FACE_DATA *face=(FREETYPE_FACE_DATA*)malloc(sizeof(FREETYPE_FACE_DATA));
	if (!face) {
		SetError(2);
		return NULL;
	}
	face->buffer=(FT_Byte *)file->Load();
	int size=(int)file->Size();
	int error = FT_New_Memory_Face(f->ftlib, face->buffer, size, 0, &face->face );
	if (error!=0) {
		free(face);
		SetError(1027);
		return NULL;
	}
	if (!fontname) fontname=face->face->family_name;
	face->kerning=(int)FT_HAS_KERNING(face->face);		// Kerning unterstützt?
	CFontFile *ff=new CFontFile;
	ff->Name=fontname;
	ff->engine=this;
	ff->priv=face;
	return ff;
#endif
}

int CFontEngineFreeType::DeleteFont(CFontFile *file)
{
#ifndef HAVE_FREETYPE2
	SetError(1035);
	return 0;
#else
	if (!file) {
		SetError(194,"int CFontEngineFreeType::DeleteFont(CFontFile *file)");
		return 0;
	}
	if (file->engine!=this) {
		SetError(1029);
		return 0;
	}
	FREETYPE_FACE_DATA *face=(FREETYPE_FACE_DATA*)file->priv;
	if (face) {
		FT_Done_Face(face->face);
		free(face->buffer);
		free(face);
		file->priv=NULL;
	}
	return 1;
#endif
}


#ifdef HAVE_FREETYPE2
static void renderGlyphAA(CDrawable &draw, FT_Bitmap *bitmap, int x, int y, const Color &color)
{
	ppluint8 v=0;
	ppluint8 *glyph=(ppluint8 *)bitmap->buffer;
	for (int gy=0;gy<bitmap->rows;gy++) {
		for (int gx=0;gx<bitmap->width;gx++) {
			v=glyph[gx];
			if (v>0) {
				draw.blendPixel(x+gx,y+gy,color,v);
			}
		}
		glyph+=bitmap->pitch;
	}
}

static void renderGlyphMono(CDrawable &draw, FT_Bitmap *bitmap, int x, int y, const Color &color)
{
	ppluint8 v=0;
	ppluint8 *glyph=(ppluint8 *)bitmap->buffer;
	ppluint8 bitcount;
	ppluint8 bytecount;
	for (int gy=0;gy<bitmap->rows;gy++) {
		bitcount=0;
		bytecount=0;
		for (int gx=0;gx<bitmap->width;gx++) {
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
		glyph+=bitmap->pitch;
	}
}

#endif

int CFontEngineFreeType::Render(CFontFile *file, const CFont &font, CDrawable &surface, int x, int y, const CWString &text, const Color &color)
{
#ifndef HAVE_FREETYPE2
	SetError(1035);
	return 0;
#else
	if (file==NULL || file->priv==NULL) {
		SetError(1033);
		return 0;
	}
	FREETYPE_FACE_DATA *face=(FREETYPE_FACE_DATA*)file->priv;

	int error=FT_Set_Pixel_Sizes(face->face,0,font.size()+2);
	if (error!=0) {
		return 0;
	}
	int orgx=x<<6;
	int orgy=y<<6;
	int lastx=orgx;
	int code;
	double angle;
	bool rotate=false;
	FT_Matrix matrix; /* transformation matrix */
	if (font.rotation()!=0.0) {
		rotate=true;
		angle=font.rotation()*M_PI/180.0;
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
	size_t textlen=text.Len();

	while (p<textlen) {
		code=text[p];
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
				renderGlyphAA(surface,&slot->bitmap,(x>>6)+slot->bitmap_left,
						(y>>6)-slot->bitmap_top,color);
			} else {
				renderGlyphMono(surface,&slot->bitmap,(x>>6)+slot->bitmap_left,
						(y>>6)-slot->bitmap_top,color);
			}
			if (font.drawUnderline()) {

			}
			lastx=x+slot->advance.x;
			orgy-=slot->advance.y;
			last_glyph=glyph_index;
		}
	}
	return 1;
#endif
}

Size CFontEngineFreeType::Measure(CFontFile *file, const CFont &font, const CWString &text)
{
	Size s;
#ifndef HAVE_FREETYPE2
	SetError(1035);
	return s;
#else
	if (file==NULL || file->priv==NULL) {
		SetError(1033);
		return s;
	}
	FREETYPE_FACE_DATA *face=(FREETYPE_FACE_DATA*)file->priv;

	int error=FT_Set_Pixel_Sizes(face->face,0,font.size()+2);
	if (error!=0) {
		return s;
	}
	FT_Set_Transform( face->face, NULL, NULL );

	int width=0,height=0;

	int code;
	FT_GlyphSlot slot=face->face->glyph;
	FT_UInt			glyph_index, last_glyph=0;
	FT_Vector		kerning;
	kerning.x=0;
	kerning.y=0;
	size_t p=0;
	size_t textlen=text.Len();
	while (p<textlen) {
		code=text[p];
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
			if (face->kerning>0 && last_glyph>0) {
				error=FT_Get_Kerning(face->face,last_glyph,glyph_index,FT_KERNING_DEFAULT,&kerning);
				width+=kerning.x;
			}
			width+=(slot->advance.x);
			if (width>s.width()) s.setWidth(width);
			last_glyph=glyph_index;
		}
	}
	s.setHeight(height+font.size()+2);
	s.setWidth(s.width()>>6);
	return s;
#endif
}



} // EOF namespace grafix
} // EOF namespace ppl6

