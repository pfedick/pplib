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
#include <pplib/grafix/drawable.h>
#include <pplib/grafix/image.h>
#include <pplib/grafix/grafix.h>
#include <pplib/types/string.h>
#include <pplib/exceptions.h>

namespace pplib::grafix
{

Drawable::Drawable(const Drawable& other)
{
    data.fn = other.data.fn;
    data.base = other.data.base;
    data.pitch = other.data.pitch;
    data.width = other.data.width;
    data.height = other.data.height;
    data.rgbformat = other.data.rgbformat;
}

Drawable::Drawable(Drawable&& other)
{
    data.fn = other.data.fn;
    data.base = other.data.base;
    data.pitch = other.data.pitch;
    data.width = other.data.width;
    data.height = other.data.height;
    data.rgbformat = other.data.rgbformat;

    other.data.fn = nullptr;
    other.data.base = nullptr;
    other.data.pitch = 0;
    other.data.width = 0;
    other.data.height = 0;
    other.data.rgbformat = RGBFormat::unknown;
}

Drawable::Drawable(void* base, uint32_t pitch, uint32_t width, uint32_t height, const RGBFormat& format)
{
    create(base, pitch, width, height, format);
}

DRAWABLE_FUNCTIONS* Drawable::getFunctions() const
{
    return data.fn;
}

const DrawableData& Drawable::getData() const
{
    return data;
}

void Drawable::copy(const Drawable& other)
{
    data.fn = other.data.fn;
    data.base = other.data.base;
    data.pitch = other.data.pitch;
    data.width = other.data.width;
    data.height = other.data.height;
    data.rgbformat = other.data.rgbformat;
}

void Drawable::create(void* base, uint32_t pitch, uint32_t width, uint32_t height, const RGBFormat& format)
{
    if (!base) throw IllegalArgumentException();
    if (!pitch) throw IllegalArgumentException();
    if (width == 0 || height == 0) throw IllegalArgumentException();
    data.base = base;
    data.pitch = pitch;
    data.width = width;
    data.height = height;
    data.rgbformat = format;
    data.fn = GetGrafix().getGrafixFunctions(format);
}

Drawable& Drawable::operator=(const Drawable& other) noexcept
{
    data.fn = other.data.fn;
    data.base = other.data.base;
    data.pitch = other.data.pitch;
    data.width = other.data.width;
    data.height = other.data.height;
    data.rgbformat = other.data.rgbformat;
    return *this;
}

Drawable& Drawable::operator=(Drawable&& other) noexcept
{
    data.fn = other.data.fn;
    data.base = other.data.base;
    data.pitch = other.data.pitch;
    data.width = other.data.width;
    data.height = other.data.height;
    data.rgbformat = other.data.rgbformat;

    other.data.fn = nullptr;
    other.data.base = nullptr;
    other.data.pitch = 0;
    other.data.width = 0;
    other.data.height = 0;
    other.data.rgbformat = RGBFormat::unknown;

    return *this;
}

Drawable Drawable::getDrawable(const Rect& rect) const
{
    // Das Monochrome1BitVertical-Format wird nicht unterstützt
    if (data.rgbformat.format() == RGBFormat::Monochrome1BitVertical) {
        throw Exception("getDrawable for Monochrome1BitVertical format not supported");
    }
    if (isEmpty()) return Drawable();
    Rect self_rect(0, 0, data.width, data.height);
    Rect intersect = rect.intersected(self_rect);
    if (intersect.isNull()) {
        // Leeres Drawable zurückgeben
        return Drawable();
    }
    return Drawable(data.base8 + intersect.top() * data.pitch + (intersect.left() * data.rgbformat.bitdepth()) / 8, data.pitch,
                    intersect.width(), intersect.height(), data.rgbformat);
}

Drawable Drawable::getDrawable(const Rect16& rect) const
{
    // Das Monochrome1BitVertical-Format wird nicht unterstützt
    if (data.rgbformat.format() == RGBFormat::Monochrome1BitVertical) {
        throw Exception("getDrawable for Monochrome1BitVertical format not supported");
    }
    if (isEmpty()) return Drawable();
    Rect16 self_rect(0, 0, data.width, data.height);
    Rect16 intersect = rect.intersected(self_rect);
    if (intersect.isNull()) {
        // Leeres Drawable zurückgeben
        return Drawable();
    }
    return Drawable(data.base8 + intersect.top() * data.pitch + (intersect.left() * data.rgbformat.bitdepth()) / 8, data.pitch,
                    intersect.width(), intersect.height(), data.rgbformat);
}

inline void* Drawable::adr(uint32_t x, uint32_t y) const
{
    if (x < data.width && y < data.height) return (void*)((uint8_t*)data.base + y * data.pitch + x * data.rgbformat.bytesPerPixel());
    throw OutOfBoundsException();
}

static void scale_down_bilinear_rgba(const uint8_t* src, int sw, int sh, int spitch, uint8_t* dst, int dw, int dh, int dpitch)
{
    // Ratio berechnen. Wenn dw=1, dann 0, sonst (sw-1)/(dw-1) für exakte Endpunkt-Abbildung
    float x_ratio = (dw > 1) ? (float)(sw - 1) / (dw - 1) : 0;
    float y_ratio = (dh > 1) ? (float)(sh - 1) / (dh - 1) : 0;

    for (int j = 0; j < dh; j++) {
        float gy = j * y_ratio;
        int gyi = (int)gy;
        float ty = gy - gyi;

        // Sicherstellen, dass gyi+1 nicht out-of-bounds geht
        if (gyi >= sh - 1) {
            gyi = sh - 2;
            ty = 1.0f;
        }
        if (gyi < 0) {
            gyi = 0;
            ty = 0.0f;
        }

        const uint8_t* row1 = src + gyi * spitch;
        const uint8_t* row2 = src + (gyi + 1) * spitch;

        for (int i = 0; i < dw; i++) {
            float gx = i * x_ratio;
            int gxi = (int)gx;
            float tx = gx - gxi;

            // Sicherstellen, dass gxi+1 nicht out-of-bounds geht
            if (gxi >= sw - 1) {
                gxi = sw - 2;
                tx = 1.0f;
            }
            if (gxi < 0) {
                gxi = 0;
                tx = 0.0f;
            }

            // Die 4 Quell-Pixel (RGBA)
            const uint8_t* p00 = row1 + gxi * 4;
            const uint8_t* p10 = row1 + (gxi + 1) * 4;
            const uint8_t* p01 = row2 + gxi * 4;
            const uint8_t* p11 = row2 + (gxi + 1) * 4;

            uint8_t* d = dst + j * dpitch + i * 4;

            for (int c = 0; c < 4; c++) { // RGBA Kanäle
                float top = p00[c] * (1.0f - tx) + p10[c] * tx;
                float bottom = p01[c] * (1.0f - tx) + p11[c] * tx;
                d[c] = (uint8_t)(top * (1.0f - ty) + bottom * ty);
            }
        }
    }
}

static void scale_down_area_averaged_rgba(const uint8_t* src, int sw, int sh, int spitch, uint8_t* dst, int dw, int dh, int dpitch)
{
    float x_ratio = (float)sw / dw;
    float y_ratio = (float)sh / dh;

    for (int j = 0; j < dh; j++) {
        for (int i = 0; i < dw; i++) {
            // Berechne den Bereich in der Quelle (Floating Point für Genauigkeit)
            float x_start = i * x_ratio;
            float y_start = j * y_ratio;
            float x_end = x_start + x_ratio;
            float y_end = y_start + y_ratio;

            // Integrations-Variablen
            float r = 0, g = 0, b = 0, a = 0;
            float total_weight = 0;

            // Laufe über alle Quellpixel, die diesen Zielpixel beeinflussen
            for (int sy = (int)y_start; sy < (int)ceil(y_end) && sy < sh; sy++) {
                float y_weight = 1.0f;
                if (sy < y_start) y_weight -= (y_start - sy);
                if (sy + 1 > y_end) y_weight -= (sy + 1 - y_end);

                for (int sx = (int)x_start; sx < (int)ceil(x_end) && sx < sw; sx++) {
                    float x_weight = 1.0f;
                    if (sx < x_start) x_weight -= (x_start - sx);
                    if (sx + 1 > x_end) x_weight -= (sx + 1 - x_end);

                    float weight = x_weight * y_weight;
                    const uint8_t* p = &src[sy * spitch + sx * 4];

                    r += p[2] * weight; // R
                    g += p[1] * weight; // G
                    b += p[0] * weight; // B
                    a += p[3] * weight; // A
                    total_weight += weight;
                }
            }

            // Ergebnis schreiben
            uint8_t* d = &dst[j * dpitch + i * 4];
            d[2] = (uint8_t)(r / total_weight);
            d[1] = (uint8_t)(g / total_weight);
            d[0] = (uint8_t)(b / total_weight);
            d[3] = (uint8_t)(a / total_weight);
        }
    }
}

Image Drawable::scaled(uint32_t width, uint32_t height, bool keepAspectRation, bool smoothTransform) const
{
    Image tgt;
    tgt.create(width, height, data.rgbformat);
    int ow = data.width;
    int oh = data.height;
    int ox, oy;

    if (keepAspectRation) {
        int x1, y1, nw, nh;
        float ratio = (float)data.width / (float)data.height;
        if (height * ratio > width) {
            nw = width;
            nh = (int)((float)nw / ratio);
        } else {
            nh = height;
            nw = (int)((float)nh * ratio);
        }
        //::printf ("old: %i x %i, new: %i x %i, new Image: %i x %i\n",ow,oh,width,height,nw,nh);
        x1 = (width - nw) / 2;
        y1 = (height - nh) / 2;
        Drawable corrected_target = tgt.getDrawable(x1, y1, x1 + nw, y1 + nh);
        if (smoothTransform) {
            // Use bilinear scaling
            uint8_t* src = (uint8_t*)data.base8;
            uint8_t* dst = (uint8_t*)corrected_target.data.base8;
            if (nw < ow || nh < oh) {
                // Verkleinerung -> Area Averaging verwenden
                scale_down_area_averaged_rgba(src, ow, oh, data.pitch, dst, nw, nh, corrected_target.data.pitch);
            } else {
                // Vergrößerung (oder gleich groß) -> Bilinear verwenden
                scale_down_bilinear_rgba(src, ow, oh, data.pitch, dst, nw, nh, corrected_target.data.pitch);
            }
        } else {
            // Use nearest neighbor scaling
            for (int y = 0; y < nh; y++) {
                for (int x = 0; x < nw; x++) {
                    ox = x * ow / nw;
                    oy = y * oh / nh;
                    corrected_target.putPixel(x, y, getPixel(ox, oy));
                }
            }
        }
    } else {
        if (smoothTransform) {
            // Use bilinear scaling
            uint8_t* src = (uint8_t*)data.base8;
            uint8_t* dst = (uint8_t*)tgt.data.base8;
            if (width < ow || height < oh) {
                // Verkleinerung -> Area Averaging verwenden
                scale_down_area_averaged_rgba(src, ow, oh, data.pitch, dst, width, height, tgt.data.pitch);
            } else {
                // Vergrößerung (oder gleich groß) -> Bilinear verwenden
                scale_down_bilinear_rgba(src, ow, oh, data.pitch, dst, width, height, tgt.data.pitch);
            }
        } else {
            // Use nearest neighbor scaling
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    ox = x * ow / width;
                    oy = y * oh / height;
                    tgt.putPixel(x, y, getPixel(ox, oy));
                }
            }
        }
    }
    return tgt;
}

} // namespace pplib::grafix