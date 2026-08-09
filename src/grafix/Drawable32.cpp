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

static void PutPixel_32(const DrawableData& data, int x, int y, SurfaceColor color)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return;
    data.base32[(data.pitch >> 2) * y + x] = (uint32_t)color;
}

static SurfaceColor GetPixel_32(const DrawableData& data, int x, int y)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return 0;
    return ((SurfaceColor)data.base32[(data.pitch >> 2) * y + x]);
}

/// A8R8G8B8
SurfaceColor RGBBlend255_A8R8G8B8(const DrawableData& data, SurfaceColor ground, SurfaceColor top, uint8_t intensity)
{
    uint8_t a_src = (((top >> 24) & 0xFF) * intensity) / 255;
    if (a_src == 0) return ground;

    uint8_t a_dst = (ground >> 24) & 0xFF;
    if (a_src == 255 && a_dst == 255) return top;

    uint32_t inv_a_src = 255 - a_src;
    uint32_t a_out_32 = a_src + (a_dst * inv_a_src) / 255;
    uint8_t a_out = (a_out_32 > 255) ? 255 : (uint8_t)a_out_32;
    if (a_out == 0) return ground;

    uint32_t r_src = (top >> 16) & 0xFF, g_src = (top >> 8) & 0xFF, b_src = top & 0xFF;
    uint32_t r_dst = (ground >> 16) & 0xFF, g_dst = (ground >> 8) & 0xFF, b_dst = ground & 0xFF;

    uint32_t r = (r_src * a_src + r_dst * a_dst * inv_a_src / 255) / a_out;
    uint32_t g = (g_src * a_src + g_dst * a_dst * inv_a_src / 255) / a_out;
    uint32_t b = (b_src * a_src + b_dst * a_dst * inv_a_src / 255) / a_out;

    return ((uint32_t)a_out << 24) | (r << 16) | (g << 8) | b;
}

// A8B8G8R8
SurfaceColor RGBBlend255_A8B8G8R8(const DrawableData& data, SurfaceColor ground, SurfaceColor top, uint8_t intensity)
{
    uint8_t a_src = (((top >> 24) & 0xFF) * intensity) / 255;
    if (a_src == 0) return ground;

    uint8_t a_dst = (ground >> 24) & 0xFF;
    if (a_src == 255 && a_dst == 255) return top;

    uint32_t inv_a_src = 255 - a_src;
    uint32_t a_out_32 = a_src + (a_dst * inv_a_src) / 255;
    uint8_t a_out = (a_out_32 > 255) ? 255 : (uint8_t)a_out_32;
    if (a_out == 0) return ground;

    uint32_t r_src = top & 0xFF, g_src = (top >> 8) & 0xFF, b_src = (top >> 16) & 0xFF;
    uint32_t r_dst = ground & 0xFF, g_dst = (ground >> 8) & 0xFF, b_dst = (ground >> 16) & 0xFF;

    uint32_t r = (r_src * a_src + r_dst * a_dst * inv_a_src / 255) / a_out;
    uint32_t g = (g_src * a_src + g_dst * a_dst * inv_a_src / 255) / a_out;
    uint32_t b = (b_src * a_src + b_dst * a_dst * inv_a_src / 255) / a_out;

    return ((uint32_t)a_out << 24) | (b << 16) | (g << 8) | r;
}

// R8G8B8A8
SurfaceColor RGBBlend255_R8G8B8A8(const DrawableData& data, SurfaceColor ground, SurfaceColor top, uint8_t intensity)
{
    uint8_t a_src = (top & 0xFF) * intensity / 255;
    if (a_src == 0) return ground;

    uint8_t a_dst = ground & 0xFF;
    if (a_src == 255 && a_dst == 255) return top;

    uint32_t inv_a_src = 255 - a_src;
    uint32_t a_out_32 = a_src + (a_dst * inv_a_src) / 255;
    uint8_t a_out = (a_out_32 > 255) ? 255 : (uint8_t)a_out_32;
    if (a_out == 0) return ground;

    uint32_t r_src = (top >> 24) & 0xFF, g_src = (top >> 16) & 0xFF, b_src = (top >> 8) & 0xFF;
    uint32_t r_dst = (ground >> 24) & 0xFF, g_dst = (ground >> 16) & 0xFF, b_dst = (ground >> 8) & 0xFF;

    uint32_t r = (r_src * a_src + r_dst * a_dst * inv_a_src / 255) / a_out;
    uint32_t g = (g_src * a_src + g_dst * a_dst * inv_a_src / 255) / a_out;
    uint32_t b = (b_src * a_src + b_dst * a_dst * inv_a_src / 255) / a_out;

    return (r << 24) | (g << 16) | (b << 8) | a_out;
}

// B8G8R8A8
SurfaceColor RGBBlend255_B8G8R8A8(const DrawableData& data, SurfaceColor ground, SurfaceColor top, uint8_t intensity)
{
    uint8_t a_src = (top & 0xFF) * intensity / 255;
    if (a_src == 0) return ground;

    uint8_t a_dst = ground & 0xFF;
    if (a_src == 255 && a_dst == 255) return top;

    uint32_t inv_a_src = 255 - a_src;
    uint32_t a_out_32 = a_src + (a_dst * inv_a_src) / 255;
    uint8_t a_out = (a_out_32 > 255) ? 255 : (uint8_t)a_out_32;
    if (a_out == 0) return ground;

    uint32_t r_src = (top >> 8) & 0xFF, g_src = (top >> 16) & 0xFF, b_src = (top >> 24) & 0xFF;
    uint32_t r_dst = (ground >> 8) & 0xFF, g_dst = (ground >> 16) & 0xFF, b_dst = (ground >> 24) & 0xFF;

    uint32_t r = (r_src * a_src + r_dst * a_dst * inv_a_src / 255) / a_out;
    uint32_t g = (g_src * a_src + g_dst * a_dst * inv_a_src / 255) / a_out;
    uint32_t b = (b_src * a_src + b_dst * a_dst * inv_a_src / 255) / a_out;

    return (b << 24) | (g << 16) | (r << 8) | a_out;
}

static void AlphaPixel_32(const DrawableData& data, int x, int y, SurfaceColor color)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return;
    SurfaceColor screen = data.base32[(data.pitch >> 2) * y + x];
    SurfaceColor result = data.fn->RGBBlend255(data, screen, color, 255);
    data.base32[(data.pitch >> 2) * y + x] = result;
}

static void BlendPixel_32(const DrawableData& data, int x, int y, SurfaceColor color, uint8_t intensity)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return;
    SurfaceColor screen = data.base32[(data.pitch >> 2) * y + x];
    SurfaceColor result = data.fn->RGBBlend255(data, screen, color, intensity);
    data.base32[(data.pitch >> 2) * y + x] = result;
}

// static void DrawRect_32(const DrawableData& data, const Rect& r, SurfaceColor c)

static void FillRect_32(const DrawableData& data, const Rect& r, SurfaceColor c)
{
    Rect s(0, 0, data.width, data.height);
    Rect in = s.intersected(r);
    if (in.isNull()) return;
    uint32_t target_pitch32 = data.pitch >> 2;
    for (int y = in.top(); y < in.bottom(); y++) {
        for (int x = in.left(); x < in.right(); x++) {
            data.base32[target_pitch32 * y + x] = c;
        }
    }
}

static void Blt_32(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y)
{
    Rect q;
    if (!clip(target, source, srect, x, y, q)) return;
    uint32_t target_pitch32 = target.pitch >> 2;
    if (target.rgbformat == source.rgbformat) {
        for (int sy = 0; sy < q.height(); sy++) {
            memmove(&target.base32[target_pitch32 * (y + sy) + x], &source.base32[(source.pitch >> 2) * (q.top() + sy) + q.left()],
                    q.width() * sizeof(SurfaceColor));
        }
    } else {
        for (int sy = 0; sy < q.height(); sy++) {
            for (int sx = 0; sx < q.width(); sx++) {
                SurfaceColor p = source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
                Color c = source.fn->FromNativeColor(p);
                target.base32[target_pitch32 * (y + sy) + x + sx] = target.fn->ToNativeColor(c);
            }
        }
    }
}

static void BltDiffuse_32(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, SurfaceColor c)
{
    Rect q;
    if (!clip(target, source, srect, x, y, q)) return;

    uint32_t target_pitch32 = target.pitch >> 2;

    if (source.rgbformat == RGBFormat::GREY8 || source.rgbformat == RGBFormat::A8) {
        // Fast Path für 8-Bit Quelle -> 32-Bit Ziel mit direktem Speicherzugriff
        for (int sy = 0; sy < q.height(); sy++) {
            const uint8_t* src = source.base8 + (q.top() + sy) * source.pitch + q.left();
            uint32_t* tgt = target.base32 + (y + sy) * target_pitch32 + x;
            for (int sx = 0; sx < q.width(); sx++) {
                uint8_t intensity = src[sx];
                if (intensity == 255) {
                    tgt[sx] = c;
                } else if (intensity > 0) {
                    tgt[sx] = target.fn->RGBBlend255(target, tgt[sx], c, intensity);
                }
            }
        }
    } else {
        // Fallback für sonstige Quellformate
        for (int sy = 0; sy < q.height(); sy++) {
            uint32_t* tgt = target.base32 + (y + sy) * target_pitch32 + x;
            for (int sx = 0; sx < q.width(); sx++) {
                SurfaceColor top = source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
                uint8_t intensity = source.fn->FromNativeColor(top).brightness();
                if (intensity == 255) {
                    tgt[sx] = c;
                } else if (intensity > 0) {
                    tgt[sx] = target.fn->RGBBlend255(target, tgt[sx], c, intensity);
                }
            }
        }
    }
}

static void BltColorKey_32(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, SurfaceColor c)
{
    Rect q;
    if (!clip(target, source, srect, x, y, q)) return;
    uint32_t target_pitch32 = target.pitch >> 2;

    if (target.rgbformat == source.rgbformat) {
        uint32_t source_pitch32 = source.pitch >> 2;
        for (int sy = 0; sy < q.height(); sy++) {
            const uint32_t* src = source.base32 + (q.top() + sy) * source_pitch32 + q.left();
            uint32_t* tgt = target.base32 + (y + sy) * target_pitch32 + x;
            for (int sx = 0; sx < q.width(); sx++) {
                uint32_t p = src[sx];
                if (p != c) {
                    tgt[sx] = p;
                }
            }
        }
    } else {
        SurfaceColor source_key = source.fn->ToNativeColor(target.fn->FromNativeColor(c));
        for (int sy = 0; sy < q.height(); sy++) {
            uint32_t* tgt = target.base32 + (y + sy) * target_pitch32 + x;
            for (int sx = 0; sx < q.width(); sx++) {
                SurfaceColor p = source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
                if (p != source_key) {
                    Color color = source.fn->FromNativeColor(p);
                    tgt[sx] = target.fn->ToNativeColor(color);
                }
            }
        }
    }
}

static void BltAlpha_32(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y)
{
    Rect q;
    if (!clip(target, source, srect, x, y, q)) return;

    uint32_t target_pitch32 = target.pitch >> 2;

    if (target.rgbformat == source.rgbformat) {
        uint32_t source_pitch32 = source.pitch >> 2;
        for (int sy = 0; sy < q.height(); sy++) {
            const uint32_t* src = source.base32 + (q.top() + sy) * source_pitch32 + q.left();
            uint32_t* tgt = target.base32 + (y + sy) * target_pitch32 + x;
            for (int sx = 0; sx < q.width(); sx++) {
                tgt[sx] = target.fn->RGBBlend255(target, tgt[sx], src[sx], 255);
            }
        }
    } else {
        for (int sy = 0; sy < q.height(); sy++) {
            uint32_t* tgt = target.base32 + (y + sy) * target_pitch32 + x;
            for (int sx = 0; sx < q.width(); sx++) {
                SurfaceColor source_pixel = source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
                Color c = source.fn->FromNativeColor(source_pixel);
                SurfaceColor target_pixel = target.fn->ToNativeColor(c);
                tgt[sx] = target.fn->RGBBlend255(target, tgt[sx], target_pixel, 255);
            }
        }
    }
}

static inline SurfaceColor GetColorModulated(SurfaceColor top, SurfaceColor mod)
{
    union ColorBytes {
        uint32_t c;
        uint8_t b[4];
    } src{top}, m{mod}, res;

    res.b[0] = static_cast<uint8_t>((src.b[0] * m.b[0]) / 255);
    res.b[1] = static_cast<uint8_t>((src.b[1] * m.b[1]) / 255);
    res.b[2] = static_cast<uint8_t>((src.b[2] * m.b[2]) / 255);
    res.b[3] = static_cast<uint8_t>((src.b[3] * m.b[3]) / 255);
    return res.c;
}

static void BltAlphaMod_32(const DrawableData& target, const DrawableData& source, const Rect& srect, SurfaceColor mod, int x, int y)
{
    Rect q;
    if (!clip(target, source, srect, x, y, q)) return;
    uint32_t target_pitch32 = target.pitch >> 2;

    if (target.rgbformat == source.rgbformat) {
        uint32_t source_pitch32 = source.pitch >> 2;
        for (int sy = 0; sy < q.height(); sy++) {
            const uint32_t* src = source.base32 + (q.top() + sy) * source_pitch32 + q.left();
            uint32_t* tgt = target.base32 + (y + sy) * target_pitch32 + x;
            for (int sx = 0; sx < q.width(); sx++) {
                SurfaceColor target_pixel = GetColorModulated(src[sx], mod);
                tgt[sx] = target.fn->RGBBlend255(target, tgt[sx], target_pixel, 255);
            }
        }
    } else {
        for (int sy = 0; sy < q.height(); sy++) {
            uint32_t* tgt = target.base32 + (y + sy) * target_pitch32 + x;
            for (int sx = 0; sx < q.width(); sx++) {
                SurfaceColor source_pixel = source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
                Color c = source.fn->FromNativeColor(source_pixel);
                SurfaceColor converted_pixel = target.fn->ToNativeColor(c);
                SurfaceColor target_pixel = GetColorModulated(converted_pixel, mod);
                tgt[sx] = target.fn->RGBBlend255(target, tgt[sx], target_pixel, 255);
            }
        }
    }
}

static inline uint8_t clampToByte(int value)
{
    if (value < 0) return 0;
    if (value > 255) return 255;
    return static_cast<uint8_t>(value);
}

static void BltBlend_32(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, float factor)
{
    Rect q;
    if (!clip(target, source, srect, x, y, q)) return;

    uint8_t blend_intensity = clampToByte(static_cast<int>(factor * 255.0f));
    if (blend_intensity == 0) return;

    uint32_t target_pitch32 = target.pitch >> 2;

    if (target.rgbformat == source.rgbformat) {
        uint32_t source_pitch32 = source.pitch >> 2;
        for (int sy = 0; sy < q.height(); sy++) {
            const uint32_t* src = source.base32 + (q.top() + sy) * source_pitch32 + q.left();
            uint32_t* tgt = target.base32 + (y + sy) * target_pitch32 + x;
            for (int sx = 0; sx < q.width(); sx++) {
                tgt[sx] = target.fn->RGBBlend255(target, tgt[sx], src[sx], blend_intensity);
            }
        }
    } else {
        for (int sy = 0; sy < q.height(); sy++) {
            uint32_t* tgt = target.base32 + (y + sy) * target_pitch32 + x;
            for (int sx = 0; sx < q.width(); sx++) {
                SurfaceColor source_pixel = source.fn->GetPixel(source, q.left() + sx, q.top() + sy);
                Color c = source.fn->FromNativeColor(source_pixel);
                SurfaceColor target_pixel = target.fn->ToNativeColor(c);
                tgt[sx] = target.fn->RGBBlend255(target, tgt[sx], target_pixel, blend_intensity);
            }
        }
    }
}

#ifdef __LITTLE_ENDIAN__
union Pixel32_t {
    struct
    {
        uint8_t red, green, blue, alpha;
    };
    uint32_t c;
};
#else
union Pixel32_t {
    struct
    {
        uint8_t alpha, blue, green, red;
    };
    uint32_t c;
};
#endif

#ifndef max
static inline int max(int a, int b)
{
    if (a > b) {
        return (a);
    }
    return (b);
}
#endif

static inline double colorclose(int Cb_p, int Cr_p, int Cb_key, int Cr_key, int tola, int tolb)
{
    /*decides if a color is close to the specified hue*/
    double temp = sqrt((Cb_key - Cb_p) * (Cb_key - Cb_p) + (Cr_key - Cr_p) * (Cr_key - Cr_p));
    // SSE: sqrtss für float, SSE2: sqrtsd für double
    // Man könnte Cb und Cr in ein MME-Register packen, die Subtraktionen und Multiplikationen
    // parallel berechnen, das Ergebnis addieren und dann die Wurzel ziehen
    if (temp < tola) {
        return (0.0);
    }
    if (temp < tolb) {
        return ((temp - tola) / (tolb - tola));
    }
    return (1.0);
}

static inline int getYCb(int r, int g, int b)
{
    return (int)(128 + -0.168736 * r - 0.331264 * g + 0.5 * b);
}

static inline int getYCr(int r, int g, int b)
{
    return (int)(128 + 0.5 * r - 0.418688 * g - 0.081312 * b);
}

static void* adr(const DrawableData& data, int x, int y)
{
    if (x < data.width && y < data.height) return data.base8 + (y * data.pitch) + (x * data.rgbformat.bitdepth() / 8);
    return NULL;
}

static void BltChromaKey_32(
    const DrawableData& target, const DrawableData& source, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y)
{
    if (tol2 < tol1) tol2 = tol1;
    double mask;
    int cb, cr;
    int cb_key = key.getYCb();
    int cr_key = key.getYCr();

    Pixel32_t c, bg, t;

    uint32_t* sadr = (uint32_t*)adr(source, srect.left(), srect.top());
    uint32_t spitch = source.pitch / 4;

    uint32_t* bgadr = (uint32_t*)adr(target, x, y);
    uint32_t bgpitch = target.pitch / 4;

    uint32_t* tgadr = (uint32_t*)adr(target, x, y);
    uint32_t tgpitch = target.pitch / 4;

    for (int y = 0; y < srect.height(); y++) {
        for (int x = 0; x < srect.width(); x++) {
            c.c = sadr[x];
            cb = getYCb(c.red, c.green, c.blue);
            cr = getYCr(c.red, c.green, c.blue);
            bg.c = bgadr[x];

            mask = 1 - colorclose(cb, cr, cb_key, cr_key, tol1, tol2);
            if (mask == 0.0) {
                tgadr[x] = c.c;
                continue;
            } else if (mask == 1.0) {
                tgadr[x] = bg.c;
            } else {
                t.red = (uint8_t)(max(c.red - mask * c.red, 0) + mask * bg.red);
                t.green = (uint8_t)(max(c.green - mask * c.green, 0) + mask * bg.green);
                t.blue = (uint8_t)(max(c.blue - mask * c.blue, 0) + mask * bg.blue);
                t.alpha = (uint8_t)(max(c.alpha - mask * c.alpha, 0) + mask * bg.alpha);
                tgadr[x] = t.c;
            }
        }
        sadr += spitch;
        bgadr += bgpitch;
        tgadr += tgpitch;
    }
}

static void BltBackgroundOnChromaKey_32(
    const DrawableData& target, const DrawableData& background, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y)
{
    if (tol2 < tol1) tol2 = tol1;
    double mask;
    int cb, cr;
    int cb_key = key.getYCb();
    int cr_key = key.getYCr();

    Pixel32_t c, bg, t;

    uint32_t* sadr = (uint32_t*)adr(target, srect.left(), srect.top());
    uint32_t spitch = target.pitch / 4;

    uint32_t* bgadr = (uint32_t*)adr(background, x, y);
    uint32_t bgpitch = background.pitch / 4;

    uint32_t* tgadr = (uint32_t*)adr(target, x, y);
    uint32_t tgpitch = target.pitch / 4;

    for (int y = 0; y < srect.height(); y++) {
        for (int x = 0; x < srect.width(); x++) {
            c.c = sadr[x];
            cb = getYCb(c.red, c.green, c.blue);
            cr = getYCr(c.red, c.green, c.blue);
            bg.c = bgadr[x];

            mask = 1 - colorclose(cb, cr, cb_key, cr_key, tol1, tol2);
            if (mask == 0.0) {
                tgadr[x] = c.c;
                continue;
            } else if (mask == 1.0) {
                tgadr[x] = bg.c;
            } else {
                t.red = (uint8_t)(max(c.red - mask * c.red, 0) + mask * bg.red);
                t.green = (uint8_t)(max(c.green - mask * c.green, 0) + mask * bg.green);
                t.blue = (uint8_t)(max(c.blue - mask * c.blue, 0) + mask * bg.blue);
                t.alpha = (uint8_t)(max(c.alpha - mask * c.alpha, 0) + mask * bg.alpha);
                tgadr[x] = t.c;
            }
        }
        sadr += spitch;
        bgadr += bgpitch;
        tgadr += tgpitch;
    }
}

/** @brief Initialisiert die Funktionszeiger für ein 32-Bit-DRAWABLE
 *
 * Diese Funktion initialisiert die Funktionszeiger in der DRAWABLE_FUNCTIONS-Struktur für ein 32-Bit-DRAWABLE,
 * in Abhängigkeit des angegebenen RGBFormats. Es wird davon ausgegangen, dass die DRAWABLE_FUNCTIONS-Struktur
 * bereits korrekt mit den generischen Methoden initialisiert wurde. Wir müssen hier also nur die 4 Mindest-Methoden
 * definieren (PutPixel, GetPixel, ToNativeColor, FromNativeColor), sowie die Methoden, für die wir optimierte
 * Implementierungen haben (BlendPixel, AlphaPixel, FillRect).
 *
 * @param[in] fn Zeiger auf die DRAWABLE_FUNCTIONS-Struktur, die initialisiert werden soll
 * @param[in] format Das RGBFormat, das das Farbformat des DRAWABLEs angibt
 */
void initDrawable32(DRAWABLE_FUNCTIONS* fn, const RGBFormat& format)
{
    fn->PutPixel = PutPixel_32;
    fn->GetPixel = GetPixel_32;
    fn->AlphaPixel = AlphaPixel_32;
    fn->BlendPixel = BlendPixel_32;
    // fn->DrawRect = DrawRect_32; // Optimierung lohnt sich nicht
    fn->FillRect = FillRect_32;
    fn->Blt = Blt_32;
    fn->BltDiffuse = BltDiffuse_32;
    fn->BltColorKey = BltColorKey_32;
    fn->BltAlpha = BltAlpha_32;
    fn->BltAlphaMod = BltAlphaMod_32;
    fn->BltBlend = BltBlend_32;
    fn->BltChromaKey = BltChromaKey_32;
    fn->BltBackgroundOnChromaKey = BltBackgroundOnChromaKey_32;

    switch (format.format()) {
    case RGBFormat::A8R8G8B8:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return (c.alpha() << 24) | (c.red() << 16) | (c.green() << 8) | c.blue();
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF);
        };
        fn->RGBBlend255 = RGBBlend255_A8R8G8B8;
        break;
#ifndef PICO_BUILD
    case RGBFormat::A8B8G8R8:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return (c.alpha() << 24) | (c.blue() << 16) | (c.green() << 8) | c.red();
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color(c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF, (c >> 24) & 0xFF);
        };
        fn->RGBBlend255 = RGBBlend255_A8B8G8R8;
        break;
    case RGBFormat::R8G8B8A8:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return (c.red() << 24) | (c.green() << 16) | (c.blue() << 8) | c.alpha();
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color((c >> 24) & 0xFF, (c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
        };
        fn->RGBBlend255 = RGBBlend255_R8G8B8A8;
        break;
    case RGBFormat::B8G8R8A8:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor {
            return (c.blue() << 24) | (c.green() << 16) | (c.red() << 8) | c.alpha();
        };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color {
            return Color((c >> 8) & 0xFF, (c >> 16) & 0xFF, (c >> 24) & 0xFF, c & 0xFF);
        };
        fn->RGBBlend255 = RGBBlend255_B8G8R8A8;
        break;
    case RGBFormat::X8R8G8B8:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor { return (255 << 24 | c.red() << 16) | (c.green() << 8) | c.blue(); };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color { return Color((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, 255); };
        break;
    case RGBFormat::X8B8G8R8:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor { return (255 << 24 | c.blue() << 16) | (c.green() << 8) | c.red(); };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color { return Color(c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF, 255); };
        break;
    case RGBFormat::R8G8B8X8:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor { return (c.red() << 24) | (c.green() << 16) | (c.blue() << 8) | 255; };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color { return Color((c >> 24) & 0xFF, (c >> 16) & 0xFF, (c >> 8) & 0xFF, 255); };
        break;
    case RGBFormat::B8G8R8X8:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor { return (c.blue() << 24) | (c.green() << 16) | (c.red() << 8) | 255; };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color { return Color((c >> 8) & 0xFF, (c >> 16) & 0xFF, (c >> 24) & 0xFF, 255); };
        break;
#endif
    }
}

} // namespace pplib::grafix