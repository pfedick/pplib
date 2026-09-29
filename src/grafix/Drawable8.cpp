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
#include <pplib/grafix/rect.h>
#include <pplib/grafix/drawable.h>
#include <pplib/grafix/image.h>
#include <pplib/grafix/grafix.h>
#include <pplib/grafix/imagereference.h>
#include <pplib/types/string.h>
#include <pplib/exceptions.h>

namespace pplib::grafix
{

static void PutPixel_8(const DrawableData& data, int x, int y, SurfaceColor color)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return;
    data.base8[data.pitch * y + x] = (uint8_t)color;
}

static SurfaceColor GetPixel_8(const DrawableData& data, int x, int y)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return 0;
    return ((SurfaceColor)data.base8[data.pitch * y + x]);
}

static void FillRect_8(const DrawableData& data, const Rect& r, SurfaceColor c)
{
    Rect s(0, 0, data.width, data.height);
    Rect in = s.intersected(r);
    if (in.isNull()) return;
    for (int y = in.top(); y < in.bottom(); y++) {
        memset(&data.base8[data.pitch * y + in.left()], c, in.width());
    }
}

static SurfaceColor RGBBlend255_8_GREY8(const DrawableData& data, SurfaceColor ground, SurfaceColor top, uint8_t intensity)
{
    if (intensity == 0) return ground;
    if (intensity == 255) return top;

    uint32_t inv = 255 - intensity;
    return (top * intensity + ground * inv) / 255;
}
static void BlendPixel_8_GREY8(const DrawableData& data, int x, int y, SurfaceColor color, uint8_t intensity)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return;
    uint8_t* p = &data.base8[data.pitch * y + x];
    *p = (uint8_t)data.fn->RGBBlend255(data, *p, color, intensity);
}

/** @brief Initialisiert die Funktionszeiger für ein 8-Bit-DRAWABLE
 *
 * Diese Funktion initialisiert die Funktionszeiger in der DRAWABLE_FUNCTIONS-Struktur für ein 8-Bit-DRAWABLE,
 * in Abhängigkeit des angegebenen RGBFormats. Es wird davon ausgegangen, dass die DRAWABLE_FUNCTIONS-Struktur
 * bereits korrekt mit den generischen Methoden initialisiert wurde. Wir müssen hier also nur die 4 Mindest-Methoden
 * definieren (PutPixel, GetPixel, ToNativeColor, FromNativeColor), sowie die Methoden, für die wir optimierte
 * Implementierungen haben (BlendPixel, AlphaPixel, FillRect).
 *
 * @param[in] fn Zeiger auf die DRAWABLE_FUNCTIONS-Struktur, die initialisiert werden soll
 * @param[in] format Das RGBFormat, das das Farbformat des DRAWABLEs angibt
 */
void Grafix::initDrawable8(DRAWABLE_FUNCTIONS* fn, const RGBFormat& format) noexcept
{
    if (format.bitdepth() != 8) return;

    switch (format.format()) {
    case RGBFormat::GREY8:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor { return (c.brightness()); };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color { return Color(c, c, c); };

        fn->PutPixel = PutPixel_8;
        fn->GetPixel = GetPixel_8;
        fn->RGBBlend255 = RGBBlend255_8_GREY8;
        fn->BlendPixel = BlendPixel_8_GREY8;
        fn->FillRect = FillRect_8;
        break;
    }
}

} // namespace pplib::grafix