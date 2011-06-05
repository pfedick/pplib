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
#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include "ppl7-grafix.h"

#ifdef HAVE_X86_ASSEMBLER

extern "C" {
	ppl7::grafix::SurfaceColor RGB_A8R8G8B8(int red, int green, int blue, int alpha);
	ppl7::grafix::SurfaceColor RGBBlend_32_255 (ppl7::grafix::SurfaceColor ground, ppl7::grafix::SurfaceColor top, int intensity);
}
#endif


namespace ppl7 {
namespace grafix {



static SurfaceColor RGB_NULL (int red, int green, int blue, int alpha)
{
	return 0;
}
static Color Surface2RGB_NULL (SurfaceColor color)
{
	return Color();
}

static SurfaceColor RGBBlend_NULL (SurfaceColor ground, SurfaceColor top, float intensity)
{
	return 0;
}

// Grey 8-Bit ************************************************************************
static SurfaceColor RGB_GREY8 (int red, int green, int blue, int alpha)
{
	return (red * 11 + green * 16 + blue * 5)/32;
	//return ((red+(green<<8)+(blue<<24)*255)/0xffffff);
}
static Color Surface2RGB_GREY8 (SurfaceColor color)
{
	return Color(color,color,color);
}


// 32 Bit ****************************************************************************
#ifndef HAVE_X86_ASSEMBLER
static SurfaceColor RGB_A8R8G8B8 (int red, int green, int blue, int alpha)
{
	ppluint32 r,g,b,a;
	r=red & 255;
	g=green & 255;
	b=blue & 255;
	a=alpha & 255;
	return (b+(g<<8)+(r<<16)+(a<<24));
}
#endif

static SurfaceColor RGB_A8B8G8R8 (int red, int green, int blue, int alpha)
{
	ppluint32 r,g,b,a;
	r=red & 255;
	g=green & 255;
	b=blue & 255;
	a=alpha & 255;
	return (r+(g<<8)+(b<<16)+(a<<24));
}

static Color Surface2RGB_A8R8G8B8 (SurfaceColor color)
{
	Color c;
	c.setBlue(color&255);
	c.setGreen((color>>8)&255);
	c.setRed((color>>16)&255);
	c.setAlpha((color>>24)&255);
	return c;
}

static Color Surface2RGB_A8B8G8R8 (SurfaceColor color)
{
	Color c;
	c.setRed(color&255);
	c.setGreen((color>>8)&255);
	c.setBlue((color>>16)&255);
	c.setAlpha((color>>24)&255);
	return c;
}

#ifndef HAVE_X86_ASSEMBLER
// Die C-Version ist hier schneller als MMX, oder doch nicht?
static SurfaceColor RGBBlend_32_255 (SurfaceColor ground, SurfaceColor top, int intensity)
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

static SurfaceColor RGBBlend_32 (SurfaceColor ground, SurfaceColor top, float intensity)
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



/*!\brief Farbe in einen Farbformatspezifischen Wert umrechnen
 *
 * \desc
 * Mit dieser Funktion wird eine farbformat unabhängige Farbe \p c in einen
 * format spezifischen Wert umgerechnet.
 *
 * @param c Farbe
 * @return Farbwert
 */
SurfaceColor Drawable::rgb(const Color &c) const
{
	if (fn->RGB) return fn->RGB(c.red(),c.green(),c.blue(),c.alpha());
	return 0;
}

/*!\brief Farbe in einen Farbformatspezifischen Wert umrechnen
 *
 * \desc
 * Mit dieser Funktion wird eine farbformat unabhängige Farbe mit den
 * Komponenten rot, grün, blau und Transparenz in einen
 * format spezifischen Wert umgerechnet.
 *
 * @param r rot-Wert
 * @param g grün-Wert
 * @param b blau-Wert
 * @param alpha Transparenz
 * @return Farbwert
 */
SurfaceColor Drawable::rgb(int r, int g, int b, int alpha) const
{
	if (fn->RGB) return fn->RGB(r,g,b,alpha);
	return 0;
}


void Grafix::initColors(const RGBFormat &format, GRAFIX_FUNCTIONS *fn)
{
	switch (format) {
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
			fn->RGB=RGB_A8R8G8B8;
			fn->Surface2RGB=Surface2RGB_A8R8G8B8;
			fn->RGBBlend=RGBBlend_32;
			fn->RGBBlend255=RGBBlend_32_255;
			return;
		case RGBFormat::B8G8R8:
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8B8G8R8:
			fn->RGB=RGB_A8B8G8R8;
			fn->Surface2RGB=Surface2RGB_A8B8G8R8;
			fn->RGBBlend=RGBBlend_32;
			fn->RGBBlend255=RGBBlend_32_255;
			return;
			/*
		case RGBFormat::R3G3B2:
			data.RGB=RGB_R3G3B2;
			data.Surface2RGB=Surface2RGB_R3G3B2;
			data.RGBBlend=RGBBlend_R3G3B2;
			break;
			*/
		case RGBFormat::A8:
		case RGBFormat::GREY8:
			fn->RGB=RGB_GREY8;
			fn->Surface2RGB=Surface2RGB_GREY8;
			fn->RGBBlend=RGBBlend_NULL;
			return;
		default:
			fn->RGB=RGB_NULL;
			fn->Surface2RGB=Surface2RGB_NULL;
			fn->RGBBlend=RGBBlend_NULL;
	}
	throw UnsupportedColorFormatException("RGBFormat=%ls (%i)",(const wchar_t*)format.name(),format.format());
}





} // EOF namespace grafix
} // EOF namespace ppl7
