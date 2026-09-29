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

static void PutPixel_16(const DrawableData& data, int x, int y, SurfaceColor color)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return;
    data.base16[(data.pitch >> 1) * y + x] = (uint16_t)color;
}

static SurfaceColor GetPixel_16(const DrawableData& data, int x, int y)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return 0;
    return ((SurfaceColor)data.base16[(data.pitch >> 1) * y + x]);
}

static void FillRect_16(const DrawableData& data, const Rect& r, SurfaceColor c)
{
    Rect s(0, 0, data.width, data.height);
    Rect in = s.intersected(r);
    if (in.isNull()) return;
    uint16_t target_pitch16 = data.pitch >> 1;
    for (int y = in.top(); y < in.bottom(); y++) {
        for (int x = in.left(); x < in.right(); x++) {
            data.base16[target_pitch16 * y + x] = c;
        }
    }
}

#ifndef PICO_BUILD
static SurfaceColor RGBBlend255_16_R5G6B5(const DrawableData& data, SurfaceColor ground, SurfaceColor top, uint8_t intensity)
{
    if (intensity == 0) return ground;
    if (intensity == 255) return top;

    uint32_t r_src = (top >> 11) & 0x1F;
    uint32_t g_src = (top >> 5) & 0x3F;
    uint32_t b_src = top & 0x1F;

    uint32_t r_dst = (ground >> 11) & 0x1F;
    uint32_t g_dst = (ground >> 5) & 0x3F;
    uint32_t b_dst = ground & 0x1F;

    uint32_t inv = 255 - intensity;
    uint32_t r = (r_src * intensity + r_dst * inv) / 255;
    uint32_t g = (g_src * intensity + g_dst * inv) / 255;
    uint32_t b = (b_src * intensity + b_dst * inv) / 255;

    return (r << 11) | (g << 5) | b;
}

static SurfaceColor RGBBlend255_16_X1555(const DrawableData& data, SurfaceColor ground, SurfaceColor top, uint8_t intensity)
{
    if (intensity == 0) return ground;
    if (intensity == 255) return top;

    uint32_t c1_src = (top >> 10) & 0x1F;
    uint32_t c2_src = (top >> 5) & 0x1F;
    uint32_t c3_src = top & 0x1F;

    uint32_t c1_dst = (ground >> 10) & 0x1F;
    uint32_t c2_dst = (ground >> 5) & 0x1F;
    uint32_t c3_dst = ground & 0x1F;

    uint32_t inv = 255 - intensity;
    uint32_t c1 = (c1_src * intensity + c1_dst * inv) / 255;
    uint32_t c2 = (c2_src * intensity + c2_dst * inv) / 255;
    uint32_t c3 = (c3_src * intensity + c3_dst * inv) / 255;

    return (c1 << 10) | (c2 << 5) | c3;
}

static SurfaceColor RGBBlend255_16_A1555(const DrawableData& data, SurfaceColor ground, SurfaceColor top, uint8_t intensity)
{
    if (!(top & 0x8000) || intensity == 0) return ground;

    if (!(ground & 0x8000)) {
        return (intensity >= 128) ? top : ground;
    }
    if (intensity == 255) return top;

    uint32_t c1_src = (top >> 10) & 0x1F;
    uint32_t c2_src = (top >> 5) & 0x1F;
    uint32_t c3_src = top & 0x1F;

    uint32_t c1_dst = (ground >> 10) & 0x1F;
    uint32_t c2_dst = (ground >> 5) & 0x1F;
    uint32_t c3_dst = ground & 0x1F;

    uint32_t inv = 255 - intensity;
    uint32_t c1 = (c1_src * intensity + c1_dst * inv) / 255;
    uint32_t c2 = (c2_src * intensity + c2_dst * inv) / 255;
    uint32_t c3 = (c3_src * intensity + c3_dst * inv) / 255;

    return 0x8000 | (c1 << 10) | (c2 << 5) | c3;
}

static SurfaceColor RGBBlend255_16_X444(const DrawableData& data, SurfaceColor ground, SurfaceColor top, uint8_t intensity)
{
    if (intensity == 0) return ground;
    if (intensity == 255) return top;

    uint32_t c1_src = (top >> 8) & 0x0F;
    uint32_t c2_src = (top >> 4) & 0x0F;
    uint32_t c3_src = top & 0x0F;

    uint32_t c1_dst = (ground >> 8) & 0x0F;
    uint32_t c2_dst = (ground >> 4) & 0x0F;
    uint32_t c3_dst = ground & 0x0F;

    uint32_t inv = 255 - intensity;
    uint32_t c1 = (c1_src * intensity + c1_dst * inv) / 255;
    uint32_t c2 = (c2_src * intensity + c2_dst * inv) / 255;
    uint32_t c3 = (c3_src * intensity + c3_dst * inv) / 255;

    return (c1 << 8) | (c2 << 4) | c3;
}

static SurfaceColor RGBBlend255_16_A444(const DrawableData& data, SurfaceColor ground, SurfaceColor top, uint8_t intensity)
{
    uint32_t a_src_4 = (top >> 12) & 0x0F;
    uint32_t a_src = (a_src_4 * intensity) / 255;
    if (a_src == 0) return ground;

    uint32_t a_dst = (ground >> 12) & 0x0F;
    if (a_src == 15 && a_dst == 15) return top;

    uint32_t inv_a_src = 15 - a_src;
    uint32_t a_out = a_src + (a_dst * inv_a_src) / 15;
    if (a_out == 0) return ground;

    uint32_t c1_src = (top >> 8) & 0x0F;
    uint32_t c2_src = (top >> 4) & 0x0F;
    uint32_t c3_src = top & 0x0F;

    uint32_t c1_dst = (ground >> 8) & 0x0F;
    uint32_t c2_dst = (ground >> 4) & 0x0F;
    uint32_t c3_dst = ground & 0x0F;

    uint32_t c1 = (c1_src * a_src + c1_dst * a_dst * inv_a_src / 15) / a_out;
    uint32_t c2 = (c2_src * a_src + c2_dst * a_dst * inv_a_src / 15) / a_out;
    uint32_t c3 = (c3_src * a_src + c3_dst * a_dst * inv_a_src / 15) / a_out;

    return (a_out << 12) | (c1 << 8) | (c2 << 4) | c3;
}

static SurfaceColor RGBBlend255_16_A8R3G3B2(const DrawableData& data, SurfaceColor ground, SurfaceColor top, uint8_t intensity)
{
    uint8_t a_src = (((top >> 8) & 0xFF) * intensity) / 255;
    if (a_src == 0) return ground;

    uint8_t a_dst = (ground >> 8) & 0xFF;
    if (a_src == 255 && a_dst == 255) return top;

    uint32_t inv_a_src = 255 - a_src;
    uint32_t a_out_32 = a_src + (a_dst * inv_a_src) / 255;
    uint8_t a_out = (a_out_32 > 255) ? 255 : (uint8_t)a_out_32;
    if (a_out == 0) return ground;

    uint32_t r_src = (top >> 5) & 0x07;
    uint32_t g_src = (top >> 2) & 0x07;
    uint32_t b_src = top & 0x03;

    uint32_t r_dst = (ground >> 5) & 0x07;
    uint32_t g_dst = (ground >> 2) & 0x07;
    uint32_t b_dst = ground & 0x03;

    uint32_t r = (r_src * a_src + r_dst * a_dst * inv_a_src / 255) / a_out;
    uint32_t g = (g_src * a_src + g_dst * a_dst * inv_a_src / 255) / a_out;
    uint32_t b = (b_src * a_src + b_dst * a_dst * inv_a_src / 255) / a_out;

    return ((uint32_t)a_out << 8) | (r << 5) | (g << 2) | b;
}
#endif

// Für RP2040 gäb es noch diese Alternative ohne Division durch 255, was ungenauer,
// aber schneller ist, da der Chip keine Hardwareunterstützung für Division hat.

#ifdef PICO_BUILD
static SurfaceColor RGBBlend255_16_R5G6B5_Fast(const DrawableData& data, SurfaceColor ground, SurfaceColor top, uint8_t intensity)
{
    if (intensity == 0) return ground;
    if (intensity == 255) return top;

    // R und B parallel maskieren (durch 6 Bits Grün getrennt)
    uint32_t rb_src = top & 0xF81F;
    uint32_t g_src = top & 0x07E0;

    uint32_t rb_dst = ground & 0xF81F;
    uint32_t g_dst = ground & 0x07E0;

    // RB parallel und G separat per Shift blenden (intensity 0..256 skaliert)
    uint32_t alpha = intensity + 1; // 1..256
    uint32_t rb = rb_dst + (((rb_src - rb_dst) * alpha) >> 8);
    uint32_t g = g_dst + (((g_src - g_dst) * alpha) >> 8);

    return (rb & 0xF81F) | (g & 0x07E0);
}
#endif

static void AlphaPixel_16(const DrawableData& data, int x, int y, SurfaceColor color)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return;
    uint16_t* p = &data.base16[(data.pitch >> 1) * y + x];
    *p = (uint16_t)data.fn->RGBBlend255(data, *p, color, 255);
}

static void BlendPixel_16(const DrawableData& data, int x, int y, SurfaceColor color, uint8_t intensity)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return;
    uint16_t* p = &data.base16[(data.pitch >> 1) * y + x];
    *p = (uint16_t)data.fn->RGBBlend255(data, *p, color, intensity);
}

static void Blt_16(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y)
{
    Rect q;
    if (!clip(target, source, srect, x, y, q)) return;
    uint32_t target_pitch16 = target.pitch >> 1;
    if (target.rgbformat == source.rgbformat) {
        uint32_t source_pitch16 = source.pitch >> 1;
        size_t width = q.width() * 2;
        for (int sy = 0; sy < q.height(); sy++) {
            memmove(&target.base16[target_pitch16 * (y + sy) + x], &source.base16[source_pitch16 * (q.top() + sy) + q.left()], width);
        }
    } else {
        for (int sy = 0; sy < q.height(); sy++) {
            for (int sx = 0; sx < q.width(); sx++) {
                SurfaceColor p = source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
                Color c = source.fn->FromNativeColor(p);
                target.base16[target_pitch16 * (y + sy) + x + sx] = (uint16_t)target.fn->ToNativeColor(c);
            }
        }
    }
}

// Helper functions to scale lower bit-depth color components to 8-bit
static inline constexpr uint8_t scale1to8(uint8_t v)
{
    return v ? 255 : 0;
}
static inline constexpr uint8_t scale2to8(uint8_t v)
{
    return (v << 6) | (v << 4) | (v << 2) | v;
}
static inline constexpr uint8_t scale3to8(uint8_t v)
{
    return (v << 5) | (v << 2) | (v >> 1);
}
static inline constexpr uint8_t scale4to8(uint8_t v)
{
    return (v << 4) | v;
}
static inline constexpr uint8_t scale5to8(uint8_t v)
{
    return (v << 3) | (v >> 2);
}
static inline constexpr uint8_t scale6to8(uint8_t v)
{
    return (v << 2) | (v >> 4);
}

/** @brief Initialisiert die Funktionszeiger für ein 16-Bit-DRAWABLE
 *
 * Diese Funktion initialisiert die Funktionszeiger in der DRAWABLE_FUNCTIONS-Struktur für ein 16-Bit-DRAWABLE,
 * in Abhängigkeit des angegebenen RGBFormats. Es wird davon ausgegangen, dass die DRAWABLE_FUNCTIONS-Struktur
 * bereits korrekt mit den generischen Methoden initialisiert wurde. Wir müssen hier also nur die 4 Mindest-Methoden
 * definieren (PutPixel, GetPixel, ToNativeColor, FromNativeColor), sowie die Methoden, für die wir optimierte
 * Implementierungen haben (BlendPixel, AlphaPixel, FillRect).
 *
 * @param[in] fn Zeiger auf die DRAWABLE_FUNCTIONS-Struktur, die initialisiert werden soll
 * @param[in] format Das RGBFormat, das das Farbformat des DRAWABLEs angibt
 */
void Grafix::initDrawable16(DRAWABLE_FUNCTIONS* fn, const RGBFormat& format) noexcept
{
    if (format.bitdepth() != 16) return;

    fn->PutPixel = PutPixel_16;
    fn->GetPixel = GetPixel_16;
    fn->AlphaPixel = AlphaPixel_16;
    fn->BlendPixel = BlendPixel_16;
    /*
    // fn->DrawRect = DrawRect_32; // Optimierung lohnt sich nicht
    */
    fn->FillRect = FillRect_16;
    fn->Blt = Blt_16;
    /*
    fn->BltDiffuse = BltDiffuse_32;
    fn->BltColorKey = BltColorKey_32;
    fn->BltAlpha = BltAlpha_32;
    fn->BltAlphaMod = BltAlphaMod_32;
    fn->BltBlend = BltBlend_32;
    fn->BltChromaKey = BltChromaKey_32;
    fn->BltBackgroundOnChromaKey = BltBackgroundOnChromaKey_32;
    */

    switch (format.format()) {
        // 5-6-5 Formate
    case RGBFormat::R5G6B5:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return ((c.red() >> 3) << 11) | ((c.green() >> 2) << 5) | (c.blue() >> 3);
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color(scale5to8((c >> 11) & 0x1F), scale6to8((c >> 5) & 0x3F), scale5to8(c & 0x1F));
        };

#ifdef PICO_BUILD
        fn->RGBBlend255 = RGBBlend255_16_R5G6B5_Fast;
#else
        fn->RGBBlend255 = RGBBlend255_16_R5G6B5;
#endif
        break;
#ifndef PICO_BUILD
    case RGBFormat::B5G6R5:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return ((c.blue() >> 3) << 11) | ((c.green() >> 2) << 5) | (c.red() >> 3);
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color(scale5to8(c & 0x1F), scale6to8((c >> 5) & 0x3F), scale5to8((c >> 11) & 0x1F));
        };
        fn->RGBBlend255 = RGBBlend255_16_R5G6B5;
        break;
        // 5-5-5 Formate (X1 / A1)
    case RGBFormat::X1R5G5B5:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return ((c.red() >> 3) << 10) | ((c.green() >> 3) << 5) | (c.blue() >> 3);
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color(scale5to8((c >> 10) & 0x1F), scale5to8((c >> 5) & 0x1F), scale5to8(c & 0x1F), 255);
        };
        fn->RGBBlend255 = RGBBlend255_16_X1555;
        break;

    case RGBFormat::X1B5G5R5:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return ((c.blue() >> 3) << 10) | ((c.green() >> 3) << 5) | (c.red() >> 3);
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color(scale5to8(c & 0x1F), scale5to8((c >> 5) & 0x1F), scale5to8((c >> 10) & 0x1F), 255);
        };
        fn->RGBBlend255 = RGBBlend255_16_X1555;
        break;

    case RGBFormat::A1R5G5B5:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return ((c.alpha() >> 7) << 15) | ((c.red() >> 3) << 10) | ((c.green() >> 3) << 5) | (c.blue() >> 3);
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color(scale5to8((c >> 10) & 0x1F), scale5to8((c >> 5) & 0x1F), scale5to8(c & 0x1F), scale1to8((c >> 15) & 0x01));
        };
        fn->RGBBlend255 = RGBBlend255_16_A1555;
        break;

    case RGBFormat::A1B5G5R5:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return ((c.alpha() >> 7) << 15) | ((c.blue() >> 3) << 10) | ((c.green() >> 3) << 5) | (c.red() >> 3);
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color(scale5to8(c & 0x1F), scale5to8((c >> 5) & 0x1F), scale5to8((c >> 10) & 0x1F), scale1to8((c >> 15) & 0x01));
        };
        fn->RGBBlend255 = RGBBlend255_16_A1555;
        break;
    // 4-4-4 Formate (X4 / A4)
    case RGBFormat::X4R4G4B4:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return ((c.red() >> 4) << 8) | ((c.green() >> 4) << 4) | (c.blue() >> 4);
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color(scale4to8((c >> 8) & 0x0F), scale4to8((c >> 4) & 0x0F), scale4to8(c & 0x0F), 255);
        };
        fn->RGBBlend255 = RGBBlend255_16_X444;
        break;

    case RGBFormat::X4B4G4R4:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return ((c.blue() >> 4) << 8) | ((c.green() >> 4) << 4) | (c.red() >> 4);
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color(scale4to8(c & 0x0F), scale4to8((c >> 4) & 0x0F), scale4to8((c >> 8) & 0x0F), 255);
        };
        fn->RGBBlend255 = RGBBlend255_16_X444;
        break;

    case RGBFormat::A4R4G4B4:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return ((c.alpha() >> 4) << 12) | ((c.red() >> 4) << 8) | ((c.green() >> 4) << 4) | (c.blue() >> 4);
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color(scale4to8((c >> 8) & 0x0F), scale4to8((c >> 4) & 0x0F), scale4to8(c & 0x0F), scale4to8((c >> 12) & 0x0F));
        };
        fn->RGBBlend255 = RGBBlend255_16_A444;
        break;

    case RGBFormat::A4B4G4R4:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return ((c.alpha() >> 4) << 12) | ((c.blue() >> 4) << 8) | ((c.green() >> 4) << 4) | (c.red() >> 4);
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color(scale4to8(c & 0x0F), scale4to8((c >> 4) & 0x0F), scale4to8((c >> 8) & 0x0F), scale4to8((c >> 12) & 0x0F));
        };
        fn->RGBBlend255 = RGBBlend255_16_A444;
        break;
        // 8-3-3-2 Format
    case RGBFormat::A8R3G3B2:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return (c.alpha() << 8) | ((c.red() >> 5) << 5) | ((c.green() >> 5) << 2) | (c.blue() >> 6);
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color(scale3to8((c >> 5) & 0x07), scale3to8((c >> 2) & 0x07), scale2to8(c & 0x03), (c >> 8) & 0xFF);
        };
        fn->RGBBlend255 = RGBBlend255_16_A8R3G3B2;
        break;

#endif
    }
}

} // namespace pplib::grafix