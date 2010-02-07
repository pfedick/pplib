/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.16 $
 * $Date: 2009/02/07 00:12:06 $
 * $Id: Colors.cpp,v 1.16 2009/02/07 00:12:06 patrick Exp $
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

//#undef HAVE_X86_ASSEMBLER

#ifdef HAVE_X86_ASSEMBLER

extern "C" {
	ppl6::grafix::COLOR RGB_A8R8G8B8(int red, int green, int blue, int alpha);
	ppl6::grafix::COLOR RGBBlend_32_255 (ppl6::grafix::COLOR ground, ppl6::grafix::COLOR top, int intensity);
}
#endif




namespace ppl6 {
namespace grafix {

int GetBitdepth(int rgbFormat)
{
	switch(rgbFormat) {
		case RGBFormat::Palette:
		case RGBFormat::A8:
		case RGBFormat::R3G3B2:
		case RGBFormat::GREY8:
			return 8;
		case RGBFormat::R5G6B5:
		case RGBFormat::B5G6R5:
		case RGBFormat::R5G5B5:
		case RGBFormat::B5G5R5:
		case RGBFormat::R4G4B4:
		case RGBFormat::A4R4G4B4:
		case RGBFormat::A1R5G5B5:
		case RGBFormat::A8R3G3B2:
			return 16;
		case RGBFormat::R8G8B8:
		case RGBFormat::B8G8R8:
			return 24;
		case RGBFormat::A8R8G8B8:
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8R8G8B8:
		case RGBFormat::X8B8G8R8:
		case RGBFormat::GREYALPHA32:
			return 32;
		case RGBFormat::A16R16G16B16:
		case RGBFormat::X16R16G16B16:
			return 64;
	};
	return 0;
}

const char *GetRGBFormatName(int rgbformat)
{
	switch (rgbformat) {
		case RGBFormat::Palette: return "PPL_RGBFormat_Palette";
		case RGBFormat::R5G6B5: return "PPL_RGBFormat_R5G6B5";
		case RGBFormat::B5G6R5: return "PPL_RGBFormat_B5G6R5";
		case RGBFormat::R5G5B5: return "PPL_RGBFormat_R5G5B5";
		case RGBFormat::B5G5R5: return "PPL_RGBFormat_B5G5R5";
		case RGBFormat::R4G4B4: return "PPL_RGBFormat_R4G4B4";
		case RGBFormat::R8G8B8: return "PPL_RGBFormat_R8G8B8";
		case RGBFormat::B8G8R8: return "PPL_RGBFormat_B8G8R8";
		case RGBFormat::A8R8G8B8: return "PPL_RGBFormat_A8R8G8B8";
		case RGBFormat::A8B8G8R8: return "PPL_RGBFormat_A8B8G8R8";
		case RGBFormat::R3G3B2: return "PPL_RGBFormat_R3G3B2";
		case RGBFormat::A4R4G4B4: return "PPL_RGBFormat_A4R4G4B4";
		case RGBFormat::A1R5G5B5: return "PPL_RGBFormat_A1R5G5B5";
		case RGBFormat::A8: return "PPL_RGBFormat_A8";
		case RGBFormat::A8R3G3B2: return "PPL_RGBFormat_A8R3G3B2";
		case RGBFormat::X8R8G8B8: return "PPL_RGBFormat_X8R8G8B8";
		case RGBFormat::X8B8G8R8: return "PPL_RGBFormat_X8B8G8R8";
		case RGBFormat::GREY8: return "PPL_RGBFormat_GREY8";
		case RGBFormat::GREYALPHA32: return "PPL_RGBFormat_GREYALPHA32";
	}
	return "PPL_RGBFormat_unknown";
}

static COLOR RGB_NULL (int red, int green, int blue, int alpha)
{
	return 0;
}
static COLOR Surface2RGB_NULL (COLOR color)
{
	return 0;
}

static COLOR RGBBlend_NULL (COLOR ground, COLOR top, float intensity)
{
	return 0;
}

// 32 Bit ****************************************************************************
#ifndef HAVE_X86_ASSEMBLER
static COLOR RGB_A8R8G8B8 (int red, int green, int blue, int alpha)
{
	ppluint32 r,g,b,a;
	r=red & 255;
	g=green & 255;
	b=blue & 255;
	a=alpha & 255;
	return (b+(g<<8)+(r<<16)+(a<<24));
}
#endif

static COLOR RGB_A8B8G8R8 (int red, int green, int blue, int alpha)
{
	ppluint32 r,g,b,a;
	r=red & 255;
	g=green & 255;
	b=blue & 255;
	a=alpha & 255;
	return (r+(g<<8)+(b<<16)+(a<<24));
}

static COLOR Surface2RGB_A8R8G8B8 (COLOR color)
{
	ppldd r,g,b,a;
	b=color&255;
	g=(color>>8)&255;
	r=(color>>16)&255;
	a=(color>>24)&255;
	return (r+(g<<8)+(b<<16)+(a<<24));
}

static COLOR Surface2RGB_A8B8G8R8 (COLOR color)
{
	ppldd r,g,b,a;
	r=color&255;
	g=(color>>8)&255;
	b=(color>>16)&255;
	a=(color>>24)&255;
	return (r+(g<<8)+(b<<16)+(a<<24));
}

#ifndef HAVE_X86_ASSEMBLER
// Die C-Version ist hier schneller als MMX, oder doch nicht?
static COLOR RGBBlend_32_255 (COLOR ground, COLOR top, int intensity)
{
	ppldd r1,g1,b1,r2,g2,b2,i1,i2;
	r1=ground&255;
	g1=(ground>>8)&255;
	b1=(ground>>16)&255;
	r2=top&255;
	g2=(top>>8)&255;
	b2=(top>>16)&255;
	i2=intensity&255;
	i1=255-i2;
	r1=((r1*i1) + (r2*i2))/255;
	g1=((g1*i1) + (g2*i2))/255;
	b1=((b1*i1) + (b2*i2))/255;

	return r1+(g1<<8)+(b1<<16)+0xff000000;
}
#endif

static COLOR RGBBlend_32 (COLOR ground, COLOR top, float intensity)
{
	return RGBBlend_32_255(ground, top,(int)(intensity*255));
	/*
	ppldd r1,g1,b1,r2,g2,b2,i1,i2;
	r1=ground&255;
	g1=(ground>>8)&255;
	b1=(ground>>16)&255;
	r2=top&255;
	g2=(top>>8)&255;
	b2=(top>>16)&255;
	i2=(ppldd)(intensity*255);
	i1=255-i2;

	r1=(r1*i1/255) + (r2*i2/255);
	g1=(g1*i1/255) + (g2*i2/255);
	b1=(b1*i1/255) + (b2*i2/255);

	return r1+(g1<<8)+(b1<<16)+0xff000000;
	*/
}


int CGrafix::InitColors(SURFACE *s)
{
	switch (s->rgbformat) {
		/*
		case RGBFormat::Palette:
			data.RGB=RGB_Palette;
			data.Surface2RGB=Surface2RGB_Palette;
			data.RGBBlend=RGBBlend_8;
			break;
		case RGBFormat::R5G6B5:
			data.RGB=RGB_R5G6B5;
			data.Surface2RGB=Surface2RGB_R5G6B5;
			data.RGBBlend=RGBBlend_R5G6B5;
			break;
		case RGBFormat::B5G6R5:
			data.RGB=RGB_B5G6R5;
			data.Surface2RGB=Surface2RGB_B5G6R5;
			data.RGBBlend=RGBBlend_B5G6R5;
			break;
		case RGBFormat::R5G5B5:
			data.RGB=RGB_R5G5B5;
			data.Surface2RGB=Surface2RGB_R5G5B5;
			data.RGBBlend=RGBBlend_R5G5B5;
			break;
		case RGBFormat::B5G5R5:
			data.RGB=RGB_B5G5R5;
			data.Surface2RGB=Surface2RGB_B5G5R5;
			data.RGBBlend=RGBBlend_B5G5R5;
			break;
		case RGBFormat::R4G4B4:
			data.RGB=RGB_R4G4B4;
			data.Surface2RGB=Surface2RGB_R4G4B4;
			data.RGBBlend=RGBBlend_R4G4B4;
			break;
		case RGBFormat::A4R4G4B4:
			data.RGB=RGB_A4R4G4B4;
			data.Surface2RGB=Surface2RGB_A4R4G4B4;
			data.RGBBlend=RGBBlend_A4R4G4B4;
			break;
			*/
		case RGBFormat::R8G8B8:
		case RGBFormat::A8R8G8B8:
		case RGBFormat::X8R8G8B8:
			s->RGB=RGB_A8R8G8B8;
			s->Surface2RGB=Surface2RGB_A8R8G8B8;
			s->RGBBlend=RGBBlend_32;
			s->RGBBlend255=RGBBlend_32_255;
			return 1;
		case RGBFormat::B8G8R8:
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8B8G8R8:
			s->RGB=RGB_A8B8G8R8;
			s->Surface2RGB=Surface2RGB_A8B8G8R8;
			s->RGBBlend=RGBBlend_32;
			s->RGBBlend255=RGBBlend_32_255;
			return 1;
			/*
		case RGBFormat::R3G3B2:
			data.RGB=RGB_R3G3B2;
			data.Surface2RGB=Surface2RGB_R3G3B2;
			data.RGBBlend=RGBBlend_R3G3B2;
			break;
			*/
		case RGBFormat::A8:
		case RGBFormat::GREY8:
			s->RGB=RGB_NULL;
			s->Surface2RGB=Surface2RGB_NULL;
			s->RGBBlend=RGBBlend_NULL;
			return 1;
			/*
		default:
			data.RGB=RGB_NULL;
			data.Surface2RGB=Surface2RGB_NULL;
			data.RGBBlend=RGBBlend_NULL;
			SetError(199);
			return 0;
			*/
	}
	SetError(1013,"RGBFormat=%s (%i)",GetRGBFormatName(s->rgbformat),s->rgbformat);
	return 0;
}

COLOR CSurface::RGB(int r, int g, int b, int a)
{
	if (s.RGB) return s.RGB(r,g,b,a);
	SetError(1012,"RGB");
	return 0;
}

COLOR CSurface::RGB(RGBA *c)
{
	if (s.RGB) return s.RGB(c->red,c->green,c->blue,c->alpha);
	SetError(1012,"RGB");
	return 0;
}

COLOR CSurface::RGB(RGBA c)
{
	if (s.RGB) return s.RGB(c.red,c.green,c.blue,c.alpha);
	SetError(1012,"RGB");
	return 0;

}

COLOR CSurface::Surface2RGB (COLOR color)
{
	if (s.Surface2RGB) return s.Surface2RGB(color);
	SetError(1012,"Surface2RGB");
	return 0;
}

COLOR CSurface::Surface2RGB (COLOR color, RGBA *c)
{
	if (!s.Surface2RGB) {
		SetError(1012,"Surface2RGB");
		return 0;
	}
	COLOR rcolor=s.Surface2RGB(color);
	if (!c) return rcolor;
	c->red=(ppldb)(rcolor&255);
	c->green=(ppldb)((rcolor>>8)&255);
	c->blue=(ppldb)((rcolor>>16)&255);
	c->alpha=(ppldb)((rcolor>>24)&255);
	return rcolor;
}

COLOR CSurface::RGBBlend(COLOR background, COLOR foreground, int alpha)
{
	if (s.RGBBlend255) return s.RGBBlend255(background, foreground,alpha);
	SetError(1012,"RGBBlend");
	return 0;
}


void CSurface::SetColorKey(COLOR c)
{
	s.colorkey=c;
	s.hascolorkey=1;
}

void CSurface::DeleteColorKey()
{
	s.hascolorkey=0;
}

COLOR CSurface::GetColorKey()
{
	return s.colorkey;
}

int CSurface::HasColorKey(COLOR *c)
{
	if (s.hascolorkey) {
		if (c) *c=s.colorkey;
		return 1;
	}
	return 0;
}

void CSurface::SetColor(COLOR c)
{
	s.lastcolor=c;
}

COLOR CSurface::Brightness(COLOR color, float factor)
{
	if (factor==1.0) return color;
	RGBA c;
	Surface2RGB(color,&c);
	int r=(int)(c.red*factor);
	int g=(int)(c.green*factor);
	int b=(int)(c.blue*factor);
	if (r>255) r=255;
	if (g>255) g=255;
	if (b>255) b=255;
	return RGB(r,g,b);
}

} // EOF namespace grafix
} // EOF namespace ppl6

