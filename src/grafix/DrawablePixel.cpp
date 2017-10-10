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

#include "ppl7.h"
#include "ppl7-grafix.h"
//#include "grafix6.h"


namespace ppl7 {
namespace grafix {

extern "C" {
#ifdef HAVE_X86_ASSEMBLER
	void AlphaPixel_32 (const DRAWABLE_DATA &data, int x, int y, SurfaceColor color);
	void PutPixel_32 (const DRAWABLE_DATA &data, int x, int y, SurfaceColor color);
	SurfaceColor GetPixel_32 (const DRAWABLE_DATA &data, int x, int y);
#endif
}

/*
 * 8 Bit
 */
static void PutPixel_8 (const DRAWABLE_DATA &data, int x, int y, SurfaceColor color)
{
	if (x<0 || y<0 || x>=data.width || y>=data.height) return;
	data.base8[data.pitch*y+x]=(ppluint8)(color&0xff);
}

static SurfaceColor GetPixel_8 (const DRAWABLE_DATA &data, int x, int y)
{
	if (x<0 || y<0 || x>=data.width || y>=data.height) return 0;
	return ((SurfaceColor)data.base8[data.pitch*y+x]);
}

/*
 * 16 Bit
 */
static void PutPixel_16 (const DRAWABLE_DATA &data, int x, int y, SurfaceColor color)
{
	if (x<0 || y<0 || x>=data.width || y>=data.height) return;
	data.base16[(data.pitch>>1)*y+x]=(ppluint16)(color&0xffff);
}

static SurfaceColor GetPixel_16 (const DRAWABLE_DATA &data, int x, int y)
{
	if (x<0 || y<0 || x>=data.width || y>=data.height) return 0;
	return ((SurfaceColor)data.base16[(data.pitch>>1)*y+x]);
}

/*
 * 32 Bit
 */
#ifndef HAVE_X86_ASSEMBLER
static void PutPixel_32 (const DRAWABLE_DATA &data, int x, int y, SurfaceColor color)
{
	if (x<0 || y<0 || x>=data.width || y>=data.height) return;
	data.base32[(data.pitch>>2)*y+x]=(ppluint32)color;
}


static SurfaceColor GetPixel_32 (const DRAWABLE_DATA &data, int x, int y)
{
	if (x<0 || y<0 || x>=data.width || y>=data.height) return 0;
	return ((SurfaceColor)data.base32[(data.pitch>>2)*y+x]);
}

static void AlphaPixel_32 (const DRAWABLE_DATA &data, int x, int y, SurfaceColor color)
{
	if (x<0 || y<0 || x>=data.width || y>=data.height) return;
	if (!data.fn->RGBBlend255) return;
	SurfaceColor screen=(SurfaceColor)data.base32[(data.pitch>>2)*y+x];
	SurfaceColor c=data.fn->RGBBlend255(screen,color,(color>>24));
	data.base32[(data.pitch>>2)*y+x]=(ppluint32)c;
}
#endif


static void BlendPixel_32 (const DRAWABLE_DATA &data, int x, int y, SurfaceColor color, int brightness)
{
	if (x<0 || y<0 || x>=data.width || y>=data.height) return;
	SurfaceColor screen=(SurfaceColor)data.base32[(data.pitch>>2)*y+x];
	SurfaceColor c=data.fn->RGBBlend255(screen,color,brightness);
	data.base32[(data.pitch>>2)*y+x]=(ppluint32)c;
}


static void PutPixel_NULL (const DRAWABLE_DATA &data, int x, int y, SurfaceColor color)
{
}

static SurfaceColor GetPixel_NULL (const DRAWABLE_DATA &data, int x, int y)
{
	return 0;
}

static void BlendPixel_NULL (const DRAWABLE_DATA &data, int x, int y, SurfaceColor c, int brightness)
{
}

static void AlphaPixel_NULL (const DRAWABLE_DATA &data, int x, int y, SurfaceColor c)
{
}

/*******************************************************************
 * Drawable
 *******************************************************************/


void Drawable::putPixel(int x, int y, const Color &c)
{
	fn->PutPixel(data,x,y,rgb(c));

}

void Drawable::putPixel(const Point &p, const Color &c)
{
	fn->PutPixel(data,p.x,p.y,rgb(c));
}

void Drawable::alphaPixel(int x, int y, const Color &c)
{
	fn->AlphaPixel(data,x,y,rgb(c));
}

void Drawable::alphaPixel(const Point &p, const Color &c)
{
	fn->AlphaPixel(data,p.x,p.y,rgb(c));
}

void Drawable::blendPixel(int x, int y, const Color &c, float brightness)
{
	fn->BlendPixel(data,x,y,rgb(c),(int)(brightness*255));
	/*
	if (x<0 || y<0 || x>=data.width || y>=data.height) return;
	if (!fn->GetPixel) return;
	if (!fn->RGBBlend) return;
	SurfaceColor color,screen=fn->GetPixel(data,x,y);
	color=rgb(c);
	color=fn->RGBBlend(screen,color,brightness);
	fn->PutPixel(data,x,y,color);
	*/
}

void Drawable::blendPixel(int x, int y, const Color &c, int brightness)
{
	fn->BlendPixel(data,x,y,rgb(c),brightness*255);
	/*
	if (x<0 || y<0 || x>=data.width || y>=data.height) return;
	SurfaceColor color,screen=fn->GetPixel(data,x,y);
	color=rgb(c);
	color=fn->RGBBlend255(screen,color,brightness);
	fn->PutPixel(data,x,y,color);
	*/
}

Color Drawable::getPixel(int x, int y) const
{
	SurfaceColor color=fn->GetPixel(data,x,y);
	return fn->Surface2RGB(color);
}

Color Drawable::getPixel(const Point &p) const
{
	SurfaceColor color=fn->GetPixel(data,p.x,p.y);
	return fn->Surface2RGB(color);
}

void Grafix::initPixel(const RGBFormat &format, GRAFIX_FUNCTIONS *fn)
{
	fn->PutPixel=PutPixel_NULL;
	fn->GetPixel=GetPixel_NULL;
	fn->BlendPixel=BlendPixel_NULL;
	fn->AlphaPixel=AlphaPixel_NULL;


	switch (format) {
		case RGBFormat::A8R8G8B8:		// 32 Bit True Color
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8B8G8R8:
		case RGBFormat::X8R8G8B8:
			fn->BlendPixel=BlendPixel_32;
			fn->AlphaPixel=AlphaPixel_32;
			/* no break */
		case RGBFormat::GREYALPHA32:
			fn->PutPixel=PutPixel_32;
			fn->GetPixel=GetPixel_32;
			return;
		case RGBFormat::R5G6B5:			// 16-Bit
		case RGBFormat::B5G6R5:
		case RGBFormat::X1R5G5B5:
		case RGBFormat::X1B5G5R5:
		case RGBFormat::X4R4G4B4:
		case RGBFormat::A1R5G5B5:
		case RGBFormat::A1B5G5R5:
		case RGBFormat::A4R4G4B4:
		case RGBFormat::A8R3G3B2:
			fn->PutPixel=PutPixel_16;
			fn->GetPixel=GetPixel_16;
			return;
		case RGBFormat::A8:				// 8-Bit
		case RGBFormat::GREY8:
			fn->PutPixel=PutPixel_8;
			fn->GetPixel=GetPixel_8;
			return;
	}
	throw UnsupportedColorFormatException("RGBFormat=%s (%i)",(const char*)format.name(),format.format());
}

} // EOF namespace grafix
} // EOF namespace ppl6
