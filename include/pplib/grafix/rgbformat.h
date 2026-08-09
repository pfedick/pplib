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

#ifndef PPLIB_INCLUDE_GRAFIX_RGBFORMAT_H
#define PPLIB_INCLUDE_GRAFIX_RGBFORMAT_H

#include <stdint.h>
#include <pplib/types/string.h>

namespace pplib::grafix
{
/** @class RGBFormat
 * @brief Datentyp, der das Farbformat einer Zeichenfläche repräsentiert
 *
 * Mit dieser Klasse wird das Farbformat einer Zeichenfläche repräsentiert. Sie enthält nur
 * einen einzigen Wert aus der Enumeration RGBFormat::Identifier, der das verwendete
 * Farbformat festlegt.
 */
class RGBFormat
{
public:
    enum Identifier
    {
        unknown = 0,

        // Allgemeine Formate, die in allen PPLIB-Versionen unterstützt werden

        Monochrome1BitVertical, ///< 1 Bit pro Pixel, vertikal gepackt (z.B. SSD1322)
        GREY8,                  ///< 8 Bit pro Pixel: 8 Bit Graustufe
        A8,                     ///< 8 Bit pro Pixel: 8 Bit Alpha (Transparenz)
        R5G6B5,                 ///< 16 Bit pro Pixel: 5 Bit Rot, 6 Bit Grün, 5 Bit Blau
        A8R8G8B8,               ///< 32 Bit pro Pixel: 8 Bit Alpha, 8 Bit Rot, 8 Bit Grün, 8 Bit Blau

#ifndef PICO_BUILD
        // Erweiterte Formate, die nicht in Embedded-Systemen unterstützt werden

        // 8 Bit
        Palette,
        R3G3B2,

        // 16 Bit

        B5G6R5,
        X1R5G5B5,
        X1B5G5R5,
        A1R5G5B5,
        A1B5G5R5,
        X4R4G4B4,
        X4B4G4R4,
        A4R4G4B4,
        A4B4G4R4,
        A8R3G3B2,

        // 24 Bit
        R8G8B8,
        B8G8R8,

        // 32 Bit
        A8B8G8R8,
        R8G8B8A8, // RGBA
        B8G8R8A8, // BGRA
        X8R8G8B8,
        X8B8G8R8,
        R8G8B8X8, // RGBX
        B8G8R8X8, // BGRX

#endif

        MaxIdentifiers ///< Obergrenze der Identifikatoren
    };

private:
    /**
     * @brief Das aktuell gesetzte Farbformat.
     */
    Identifier format_id;

public:
    RGBFormat();
    RGBFormat(Identifier id);
    RGBFormat(const String& Identifier);

    void setFormat(Identifier id);
    void setFormat(const String& Identifier);

    int format() const;
    String name() const;
    int bitdepth() const;
    int bytesPerPixel() const;
    int bitsPerPixel() const;

    RGBFormat& operator=(Identifier id);
    operator int() const;

    friend bool operator!=(const RGBFormat& r1, const RGBFormat& r2);
    friend bool operator==(const RGBFormat& r1, const RGBFormat& r2);
    friend bool operator==(const RGBFormat& r1, RGBFormat::Identifier r2);
    friend bool operator!=(const RGBFormat& r1, RGBFormat::Identifier r2);
    friend bool operator==(RGBFormat::Identifier r1, const RGBFormat& r2);
    friend bool operator!=(RGBFormat::Identifier r1, const RGBFormat& r2);
};

bool operator!=(const RGBFormat& r1, const RGBFormat& r2);
bool operator==(const RGBFormat& r1, const RGBFormat& r2);
bool operator==(const RGBFormat& r1, RGBFormat::Identifier r2);
bool operator!=(const RGBFormat& r1, RGBFormat::Identifier r2);
bool operator==(RGBFormat::Identifier r1, const RGBFormat& r2);
bool operator!=(RGBFormat::Identifier r1, const RGBFormat& r2);

} // namespace pplib::grafix

#endif // PPLIB_INCLUDE_GRAFIX_RGBFORMAT_H
