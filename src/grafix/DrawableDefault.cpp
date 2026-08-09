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
#include <pplib/grafix/color.h>
#include <pplib/types/string.h>

namespace pplib::grafix
{

// Nop-Funktionen, die nichts tun. Wird verwendet, wenn ein Drawable nicht initialisiert ist.
static void nopPutPixel(const DrawableData& data, int x, int y, SurfaceColor c)
{
}

static SurfaceColor nopGetPixel(const DrawableData& data, int x, int y)
{
    return 0;
}

static SurfaceColor nopToNativeColor(const Color& c)
{
    return 0;
}

static Color nopFromNativeColor(const SurfaceColor c)
{
    return Color(0, 0, 0, 0);
}

// Defaukt-Implementierungen, die lediglich GetPixcel, PutPixel, ToNativeColor und FromNativeColor verwenden,
// um die Funktionalität zu implementieren.
static SurfaceColor defaultRGBBlend255(const DrawableData& data, SurfaceColor ground, SurfaceColor top, uint8_t intensity)
{
    Color src = data.fn->FromNativeColor(top);

    // Effektiver Alpha-Wert unter Berücksichtigung von intensity
    uint8_t a_src = (src.alpha() * intensity) / 255;
    if (a_src == 0) return ground; // Vollkommen transparent -> Hintergrund unverändert

    Color dst = data.fn->FromNativeColor(ground);
    uint8_t a_dst = dst.alpha();

    if (a_src == 255 && a_dst == 255) return top; // Vollkommen deckend -> einfach überschreiben

    // Berechnung des neuen Alpha-Wertes: Porter-Duff "Over" Alpha-Berechnung
    uint32_t inv_a_src = 255 - a_src;
    uint32_t a_out_32 = a_src + (a_dst * inv_a_src) / 255;
    uint8_t a_out = (a_out_32 > 255) ? 255 : (uint8_t)a_out_32;

    if (a_out == 0) return ground;

    // RGB-Kanäle gewichtet mischen
    uint32_t r = (src.red() * a_src + dst.red() * a_dst * inv_a_src / 255) / a_out;
    uint32_t g = (src.green() * a_src + dst.green() * a_dst * inv_a_src / 255) / a_out;
    uint32_t b = (src.blue() * a_src + dst.blue() * a_dst * inv_a_src / 255) / a_out;

    Color result((uint8_t)r, (uint8_t)g, (uint8_t)b, a_out);
    return data.fn->ToNativeColor(result);
}

static void defaultAlphaPixel(const DrawableData& data, int x, int y, SurfaceColor c)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return;
    SurfaceColor screen = data.fn->GetPixel(data, x, y);
    SurfaceColor result = data.fn->RGBBlend255(data, screen, c, 255);
    data.fn->PutPixel(data, x, y, result);
}

static void defaultBlendPixel(const DrawableData& data, int x, int y, SurfaceColor c, uint8_t intensity)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return;
    SurfaceColor screen = data.fn->GetPixel(data, x, y);
    SurfaceColor result = data.fn->RGBBlend255(data, screen, c, intensity);
    data.fn->PutPixel(data, x, y, result);
}

static void defaultDrawRect(const DrawableData& data, const Rect& r, SurfaceColor c)
{
    for (int x = r.left(); x < r.right(); x++) {
        data.fn->PutPixel(data, x, r.top(), c);
        data.fn->PutPixel(data, x, r.lastY(), c);
    }
    for (int y = r.top(); y < r.bottom(); y++) {
        data.fn->PutPixel(data, r.left(), y, c);
        data.fn->PutPixel(data, r.lastX(), y, c);
    }
}

static void defaultFillRect(const DrawableData& data, const Rect& r, SurfaceColor c)
{
    Rect s(0, 0, data.width, data.height);
    Rect in = s.intersected(r);
    if (in.isNull()) return;
    for (int y = in.top(); y < in.bottom(); y++) {
        for (int x = in.left(); x < in.right(); x++) {
            data.fn->PutPixel(data, x, y, c);
        }
    }
}

static void defaultClear(const DrawableData& data, SurfaceColor c)
{
    // statt clear einfach ein FillRect über die gesamte Fläche aufrufen.
    data.fn->FillRect(data, Rect(0, 0, data.width, data.height), c);
}

static void defaultXchange(const DrawableData& data, const Rect& r, SurfaceColor farbe, SurfaceColor ersatzfarbe)
{
    Rect s(0, 0, data.width, data.height);
    Rect in = s.intersected(r);
    if (in.isNull()) return;
    for (int y = in.top(); y < in.bottom(); y++) {
        for (int x = in.left(); x < in.right(); x++) {
            SurfaceColor pixel = data.fn->GetPixel(data, x, y);
            if (pixel == farbe) {
                data.fn->PutPixel(data, x, y, ersatzfarbe);
            }
        }
    }
}

static void defaultInvert(const DrawableData& data, const Rect& r, SurfaceColor farbe1, SurfaceColor farbe2)
{
    Rect s(0, 0, data.width, data.height);
    Rect in = s.intersected(r);
    if (in.isNull()) return;
    for (int y = in.top(); y < in.bottom(); y++) {
        for (int x = in.left(); x < in.right(); x++) {
            SurfaceColor pixel = data.fn->GetPixel(data, x, y);
            if (pixel == farbe1) {
                data.fn->PutPixel(data, x, y, farbe2);
            } else if (pixel == farbe2) {
                data.fn->PutPixel(data, x, y, farbe1);
            }
        }
    }
}

static void defaultNegativ(const DrawableData& data, const Rect& r)
{
    Rect s(0, 0, data.width, data.height);
    Rect in = s.intersected(r);
    if (in.isNull()) return;
    for (int y = in.top(); y < in.bottom(); y++) {
        for (int x = in.left(); x < in.right(); x++) {
            SurfaceColor pixel = data.fn->GetPixel(data, x, y);
            Color c = data.fn->FromNativeColor(pixel);
            SurfaceColor neg_pixel = data.fn->ToNativeColor(c.negative());
            data.fn->PutPixel(data, x, y, neg_pixel);
        }
    }
}

bool clip(const DrawableData& target, const DrawableData& source, const Rect& srect, int& x, int& y, Rect& clipped_srect)
{
    // 1. Ziel- und Quell-Grenzrechtecke
    Rect target_rect(0, 0, target.width, target.height);
    Rect source_rect(0, 0, source.width, source.height);

    // 2. Quellrechteck an Quellgrafik anpassen
    Rect srect_clipped = source_rect.intersected(srect);
    if (srect_clipped.isNull()) return false;

    // 3. Korrektur der Zielkoordinaten, falls srect oben/links außerhalb der Quelle lag
    int offset_x = srect_clipped.left() - srect.left();
    int offset_y = srect_clipped.top() - srect.top();

    // 4. Verschobenes Rechteck im Zielraum
    Rect dest_rect(x + offset_x, y + offset_y, srect_clipped.width(), srect_clipped.height());

    // 5. Schnittmenge mit den Zielgrenzen
    Rect final_dest_rect = target_rect.intersected(dest_rect);
    if (final_dest_rect.isNull()) return false;

    // 6. Berechne die Anpassung an Quelle und Ziel durch das Ziel-Clipping
    int dx = final_dest_rect.left() - dest_rect.left();
    int dy = final_dest_rect.top() - dest_rect.top();

    clipped_srect = Rect(srect_clipped.left() + dx, srect_clipped.top() + dy, final_dest_rect.width(), final_dest_rect.height());

    // 7. Aktualisiere die Zielkoordinaten
    x = final_dest_rect.left();
    y = final_dest_rect.top();

    return true;
}

static void defaultBlt(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y)
{
    Rect q;
    if (!clip(target, source, srect, x, y, q)) return;

    for (int sy = 0; sy < q.height(); sy++) {
        for (int sx = 0; sx < q.width(); sx++) {
            SurfaceColor p = source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
            target.fn->PutPixel(target, x + sx, y + sy, p);
        }
    }
}

static void defaultBltDiffuse(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, SurfaceColor c)
{
    Rect q;
    if (!clip(target, source, srect, x, y, q)) return;
    if (source.rgbformat == RGBFormat::GREY8 || source.rgbformat == RGBFormat::A8) {
        // Fast Path für 8-Bit Graustufen / Alpha-Masken
        for (int sy = 0; sy < q.height(); sy++) {
            for (int sx = 0; sx < q.width(); sx++) {
                uint8_t intensity = (uint8_t)source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
                if (intensity > 0) {
                    SurfaceColor ground = target.fn->GetPixel(target, x + sx, y + sy);
                    SurfaceColor diffuse_color = target.fn->RGBBlend255(target, ground, c, intensity);
                    target.fn->PutPixel(target, x + sx, y + sy, diffuse_color);
                }
            }
        }
    } else {
        // Fallback für farbige Quellformate: Helligkeit (brightness) berechnen
        for (int sy = 0; sy < q.height(); sy++) {
            for (int sx = 0; sx < q.width(); sx++) {
                SurfaceColor top = source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
                uint8_t intensity = source.fn->FromNativeColor(top).brightness();
                if (intensity > 0) {
                    SurfaceColor ground = target.fn->GetPixel(target, x + sx, y + sy);
                    SurfaceColor diffuse_color = target.fn->RGBBlend255(target, ground, c, intensity);
                    target.fn->PutPixel(target, x + sx, y + sy, diffuse_color);
                }
            }
        }
    }
}

static void defaultBltColorKey(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, SurfaceColor c)
{
    Rect q;
    if (!clip(target, source, srect, x, y, q)) return;
    for (int sy = 0; sy < q.height(); sy++) {
        for (int sx = 0; sx < q.width(); sx++) {
            SurfaceColor p = source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
            if (p != c) {
                target.fn->PutPixel(target, x + sx, y + sy, p);
            }
        }
    }
}

static void defaultBltAlpha(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y)
{
    Rect q;
    if (!clip(target, source, srect, x, y, q)) return;

    if (target.rgbformat == source.rgbformat) {
        // Fast Path: Identisches Farbformat
        for (int sy = 0; sy < q.height(); sy++) {
            for (int sx = 0; sx < q.width(); sx++) {
                SurfaceColor top = source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
                target.fn->AlphaPixel(target, x + sx, y + sy, top);
            }
        }
    } else {
        // Formatkonvertierung: Source-Native -> Color -> Target-Native
        for (int sy = 0; sy < q.height(); sy++) {
            for (int sx = 0; sx < q.width(); sx++) {
                SurfaceColor source_pixel = source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
                Color c = source.fn->FromNativeColor(source_pixel);
                SurfaceColor target_pixel = target.fn->ToNativeColor(c);
                target.fn->AlphaPixel(target, x + sx, y + sy, target_pixel);
            }
        }
    }
}

static inline Color GetColorModulated(const Color& top, const Color& mod)
{
    Color result;
    result.setAlpha(top.alpha() * mod.alpha() / 255);
    result.setRed(top.red() * mod.red() / 255);
    result.setGreen(top.green() * mod.green() / 255);
    result.setBlue(top.blue() * mod.blue() / 255);
    return result;
}

static void defaultBltAlphaMod(const DrawableData& target, const DrawableData& source, const Rect& srect, SurfaceColor mod, int x, int y)
{
    Rect q;
    if (!clip(target, source, srect, x, y, q)) return;
    Color mod_color = target.fn->FromNativeColor(mod);

    for (int sy = 0; sy < q.height(); sy++) {
        for (int sx = 0; sx < q.width(); sx++) {
            SurfaceColor source_pixel = source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
            Color top_color = source.fn->FromNativeColor(source_pixel);
            Color modulated_color = GetColorModulated(top_color, mod_color);
            SurfaceColor target_pixel = target.fn->ToNativeColor(modulated_color);
            target.fn->AlphaPixel(target, x + sx, y + sy, target_pixel);
        }
    }
}

static inline uint8_t clampToByte(int value)
{
    if (value < 0) return 0;
    if (value > 255) return 255;
    return static_cast<uint8_t>(value);
}

static void defaultBltBlend(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, float factor)
{
    Rect q;
    if (!clip(target, source, srect, x, y, q)) return;
    for (int sy = 0; sy < q.height(); sy++) {
        for (int sx = 0; sx < q.width(); sx++) {
            SurfaceColor source_pixel = source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
            Color top_color = source.fn->FromNativeColor(source_pixel);
            top_color.setAlpha(clampToByte(top_color.alpha() * factor));
            SurfaceColor target_pixel = target.fn->ToNativeColor(top_color);
            target.fn->AlphaPixel(target, x + sx, y + sy, target_pixel);
        }
    }
}

static void defaultBltChromaKey(
    const DrawableData& target, const DrawableData& source, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y)
{
}

static void defaultBltBackgroundOnChromaKey(
    const DrawableData& target, const DrawableData& background, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y)
{
}

static DRAWABLE_FUNCTIONS nop = {
    nopPutPixel,                    // PutPixel
    nopGetPixel,                    // GetPixel
    nopToNativeColor,               // ToNativeColor
    nopFromNativeColor,             // FromNativeColor
    defaultBlendPixel,              // BlendPixel
    defaultAlphaPixel,              // AlphaPixel
    defaultClear,                   // Clear
    defaultDrawRect,                // DrawRect
    defaultFillRect,                // FillRect
    defaultXchange,                 // Xchange
    defaultInvert,                  // Invert
    defaultNegativ,                 // Negativ
    defaultRGBBlend255,             // RGBBlend255
    defaultBlt,                     // Blt
    defaultBltDiffuse,              // BltDiffuse
    defaultBltColorKey,             // BltColorKey
    defaultBltAlpha,                // BltAlpha
    defaultBltAlphaMod,             // BltAlphaMod
    defaultBltBlend,                // BltBlend
    defaultBltChromaKey,            // BltChromaKey
    defaultBltBackgroundOnChromaKey // BltBackgroundOnChromaKey
};

DRAWABLE_FUNCTIONS* getDefaultGrafixFunctions()
{
    return &nop;
}

} // namespace pplib::grafix