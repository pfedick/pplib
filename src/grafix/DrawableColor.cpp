/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <string.h>
#include <config_pplib.h>
#include <pplib/grafix/drawable.h>
#include <pplib/grafix/image.h>
#include <pplib/grafix/grafix.h>
#include <pplib/grafix/imagereference.h>
#include <pplib/types/string.h>
#include <pplib/exceptions.h>

#ifdef HAVE_X86_ASSEMBLER

extern "C"
{
    pplib::grafix::SurfaceColor RGB_A8R8G8B8(int red, int green, int blue, int alpha);
    pplib::grafix::SurfaceColor RGBBlend_32_255(pplib::grafix::SurfaceColor ground, pplib::grafix::SurfaceColor top, int intensity);
}
#endif

namespace pplib::grafix
{

// Grey 8-Bit ************************************************************************
static SurfaceColor RGB_GREY8(int red, int green, int blue, int alpha)
{
    return (red * 11 + green * 16 + blue * 5) / 32;
    // return ((red+(green<<8)+(blue<<24)*255)/0xffffff);
}
static Color Surface2RGB_GREY8(SurfaceColor color)
{
    return Color(color, color, color);
}

// 32 Bit ****************************************************************************
#ifndef HAVE_X86_ASSEMBLER
static SurfaceColor RGB_A8R8G8B8(int red, int green, int blue, int alpha)
{
    uint32_t r, g, b, a;
    r = red & 255;
    g = green & 255;
    b = blue & 255;
    a = alpha & 255;
    return (b + (g << 8) + (r << 16) + (a << 24));
}
#endif

static SurfaceColor RGB_A8B8G8R8(int red, int green, int blue, int alpha)
{
    uint32_t r, g, b, a;
    r = red & 255;
    g = green & 255;
    b = blue & 255;
    a = alpha & 255;
    return (r + (g << 8) + (b << 16) + (a << 24));
}

static Color Surface2RGB_A8R8G8B8(SurfaceColor color)
{
    return Color((color >> 16) & 255, (color >> 8) & 255, color & 255, (color >> 24) & 255);
    /*
    c.setBlue(color&255);
    c.setGreen((color>>8)&255);
    c.setRed((color>>16)&255);
    c.setAlpha((color>>24)&255);
    return c;
    */
}

static Color Surface2RGB_A8B8G8R8(SurfaceColor color)
{
    Color c;
    c.setRed(color & 255);
    c.setGreen((color >> 8) & 255);
    c.setBlue((color >> 16) & 255);
    c.setAlpha((color >> 24) & 255);
    return c;
}

#ifndef HAVE_X86_ASSEMBLER
// Die C-Version ist hier schneller als MMX, oder doch nicht?
static SurfaceColor RGBBlend_32_255(SurfaceColor ground, SurfaceColor top, int intensity)
{
    uint32_t a1, r1, g1, b1, a2, r2, g2, b2, i1, i2;
    r1 = ground & 255;
    g1 = (ground >> 8) & 255;
    b1 = (ground >> 16) & 255;
    a1 = (ground >> 24) & 255;
    r2 = top & 255;
    g2 = (top >> 8) & 255;
    b2 = (top >> 16) & 255;
    a2 = (top >> 24) & 255;
    i2 = intensity & 255;
    i1 = 255 - i2;
    r1 = ((r1 * i1) + (r2 * i2)) / 255;
    g1 = ((g1 * i1) + (g2 * i2)) / 255;
    b1 = ((b1 * i1) + (b2 * i2)) / 255;
    a1 = ((a1 * i1) + (a2 * i2)) / 255;
    return r1 | (g1 << 8) | (b1 << 16) | (a1 << 24);
}
#endif

static SurfaceColor RGBBlend_32(SurfaceColor ground, SurfaceColor top, float intensity)
{
    return RGBBlend_32_255(ground, top, (int)(intensity * 255));
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
SurfaceColor Drawable::toNativeColor(const Color& c) const
{
    return fn->GetRGB(c.red(), c.green(), c.blue(), c.alpha());
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
    return fn->GetRGB(r, g, b, alpha);
}

void Grafix::initColors(const RGBFormat& format, GRAFIX_FUNCTIONS* fn)
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
        fn->GetRGB = RGB_A8R8G8B8;
        fn->Surface2RGB = Surface2RGB_A8R8G8B8;
        fn->RGBBlend = RGBBlend_32;
        fn->RGBBlend255 = RGBBlend_32_255;
        return;
    case RGBFormat::B8G8R8:
    case RGBFormat::A8B8G8R8:
    case RGBFormat::X8B8G8R8:
        fn->GetRGB = RGB_A8B8G8R8;
        fn->Surface2RGB = Surface2RGB_A8B8G8R8;
        fn->RGBBlend = RGBBlend_32;
        fn->RGBBlend255 = RGBBlend_32_255;
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
        fn->GetRGB = RGB_GREY8;
        fn->Surface2RGB = Surface2RGB_GREY8;
        fn->RGBBlend = RGBBlend_NULL;
        fn->RGBBlend255 = RGBBlend_255_NULL;
        return;
    default:
        fn->GetRGB = RGB_NULL;
        fn->Surface2RGB = Surface2RGB_NULL;
        fn->RGBBlend = RGBBlend_NULL;
        fn->RGBBlend255 = RGBBlend_255_NULL;
        break;
    }
}

} // namespace pplib::grafix