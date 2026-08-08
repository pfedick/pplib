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

#include <pplib/grafix/drawable.h>
#include <pplib/types/string.h>

namespace pplib::grafix
{

// Nop-Funktionen, die nichts tun. Wird verwendet, wenn ein Drawable nicht initialisiert ist.
void nopClear(DrawableData& data, SurfaceColor c)
{
}

void nopPutPixel(const DrawableData& data, int x, int y, SurfaceColor c)
{
}

void nopBlendPixel(const DrawableData& data, int x, int y, SurfaceColor c, int brightness)
{
}

void nopAlphaPixel(const DrawableData& data, int x, int y, SurfaceColor c)
{
}

SurfaceColor nopGetPixel(const DrawableData& data, int x, int y)
{
    return 0;
}

void nopDrawRect(const DrawableData& data, const Rect& r, SurfaceColor c)
{
}
void nopFillRect(const DrawableData& data, const Rect& r, SurfaceColor c)
{
}
void nopXchange(const DrawableData& data, const Rect& r, SurfaceColor farbe, SurfaceColor ersatzfarbe)
{
}
void nopInvert(const DrawableData& data, const Rect& r, SurfaceColor farbe1, SurfaceColor farbe2)
{
}

void nopNegativ(const DrawableData& data, const Rect& r)
{
}

SurfaceColor nopToNativeColor(const Color& c)
{
    return 0;
}

Color nopFromNativeColor(const SurfaceColor c)
{
    return Color(0, 0, 0, 0);
}

SurfaceColor nopRGBBlend(SurfaceColor ground, SurfaceColor top, float intensity)
{
    return 0;
}

SurfaceColor nopRGBBlend255(SurfaceColor ground, SurfaceColor top, int intensity)
{
    return 0;
}

void nopLine(const DrawableData& data, int x1, int y1, int x2, int y2, SurfaceColor color)
{
}

void nopLineAA(const DrawableData& data, int x1, int y1, int x2, int y2, SurfaceColor color, int strength)
{
}

void nopBlt(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y)
{
}

void nopBltDiffuse(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, SurfaceColor c)
{
}

void nopBltColorKey(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, SurfaceColor c)
{
}

void nopBltAlpha(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y)
{
}

void nopBltAlphaMod(const DrawableData& target, const DrawableData& source, const Rect& srect, SurfaceColor mod, int x, int y)
{
}

void nopBltBlend(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, float factor)
{
}

void nopBltChromaKey(
    const DrawableData& target, const DrawableData& source, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y)
{
}

void nopBltBackgoundOnChromaKey(
    const DrawableData& target, const DrawableData& background, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y)
{
}

static DRAWABLE_FUNCTIONS nop = {
    nopClear,                  // Clear
    nopPutPixel,               // PutPixel
    nopBlendPixel,             // BlendPixel
    nopAlphaPixel,             // AlphaPixel
    nopGetPixel,               // GetPixel
    nopDrawRect,               // DrawRect
    nopFillRect,               // FillRect
    nopXchange,                // Xchange
    nopInvert,                 // Invert
    nopNegativ,                // Negativ
    nopToNativeColor,          // ToNativeColor
    nopFromNativeColor,        // FromNativeColor
    nopRGBBlend,               // RGBBlend
    nopRGBBlend255,            // RGBBlend255
    nopLineAA,                 // LineAA
    nopLine,                   // Line
    nopBlt,                    // Blt
    nopBltDiffuse,             // BltDiffuse
    nopBltColorKey,            // BltColorKey
    nopBltAlpha,               // BltAlpha
    nopBltAlphaMod,            // BltAlphaMod
    nopBltBlend,               // BltBlend
    nopBltChromaKey,           // BltChromaKey
    nopBltBackgoundOnChromaKey // BltBackgoundOnChromaKey
};

DRAWABLE_FUNCTIONS* getDefaultGrafixFunctions()
{
    return &nop;
}

} // namespace pplib::grafix