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

// ========== 1-Bit Monochrome (VERTIKAL) for OLEDs based on SSD1306 ==========
static void PutPixelMonochrome1BitVertical(const DrawableData& data, int x, int y, SurfaceColor color)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return;
    // Adresse: x + (y/8) * pitch
    // pitch = Bytes pro "Zeilen-Block" (8 Pixel hoch)
    uint8_t* ptr = data.base8 + x + (y >> 3) * data.pitch;
    int bit_offset = y & 7; // Bit innerhalb des Bytes (vertikal!)
    if (color)
        *ptr |= (1 << bit_offset);
    else
        *ptr &= ~(1 << bit_offset);
}

static SurfaceColor GetPixelMonochrome1BitVertical(const DrawableData& data, int x, int y)
{
    if (x < 0 || y < 0 || x >= data.width || y >= data.height) return 0;

    uint8_t* ptr = data.base8 + x + (y >> 3) * data.pitch;
    int bit_offset = y & 7; // Bit innerhalb des Bytes (vertikal!)

    return (*ptr & (1 << bit_offset)) ? 1 : 0;
}

static void BlendPixelMonochrome1BitVertical(const DrawableData& data, int x, int y, SurfaceColor color, uint8_t intensity)
{
    if (intensity >= 100) {
        PutPixelMonochrome1BitVertical(data, x, y, color);
        return;
    }
}

static void FillRectMonochrome1BitVertical(const DrawableData& data, const Rect& r, SurfaceColor c)
{
    Rect s(0, 0, data.width, data.height);
    Rect clipped = s.intersected(r);
    if (clipped.isNull()) return;

    if (clipped.isNull()) return;
    for (int y = clipped.top(); y < clipped.bottom(); y++) {
        for (int x = clipped.left(); x < clipped.right(); x++) {
            uint8_t* ptr = data.base8 + x + (y >> 3) * data.pitch;
            int bit_offset = y & 7; // Bit innerhalb des Bytes (vertikal!)
            if (c)
                *ptr |= (1 << bit_offset);
            else
                *ptr &= ~(1 << bit_offset);
        }
    }
}

/** @brief Initialisiert die Funktionszeiger für ein 1-Bit-DRAWABLE
 *
 * Diese Funktion initialisiert die Funktionszeiger in der DRAWABLE_FUNCTIONS-Struktur für ein 1-Bit-DRAWABLE,
 * in Abhängigkeit des angegebenen RGBFormats. Es wird davon ausgegangen, dass die DRAWABLE_FUNCTIONS-Struktur
 * bereits korrekt mit den generischen Methoden initialisiert wurde. Wir müssen hier also nur die 4 Mindest-Methoden
 * definieren (PutPixel, GetPixel, ToNativeColor, FromNativeColor), sowie die Methoden, für die wir optimierte
 * Implementierungen haben (BlendPixel, AlphaPixel, FillRect).
 *
 * @param[in] fn Zeiger auf die DRAWABLE_FUNCTIONS-Struktur, die initialisiert werden soll
 * @param[in] format Das RGBFormat, das das Farbformat des DRAWABLEs angibt
 */
void Grafix::initDrawable1(DRAWABLE_FUNCTIONS* fn, const RGBFormat& format) noexcept
{
    if (format.bitdepth() != 1) return;

    switch (format.format()) {
    case RGBFormat::Monochrome1BitVertical:
        fn->ToNativeColor = [](const Color& c) -> SurfaceColor { return (c.brightness() > 128 ? 1 : 0); };
        fn->FromNativeColor = [](const SurfaceColor c) -> Color { return (c == 1 ? Color(255, 255, 255) : Color(0, 0, 0)); };

        fn->PutPixel = PutPixelMonochrome1BitVertical;
        fn->GetPixel = GetPixelMonochrome1BitVertical;
        fn->BlendPixel = BlendPixelMonochrome1BitVertical;
        fn->FillRect = FillRectMonochrome1BitVertical;
        break;
    }
}

} // namespace pplib::grafix