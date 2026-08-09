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

#include <pplib/grafix/rgbformat.h>
#include <pplib/grafix/grafix.h>
#include <pplib/exceptions.h>
#include <pplib/types/string.h>
namespace pplib::grafix
{

RGBFormat::RGBFormat(Identifier id)
{
    format_id = id;
}

RGBFormat::RGBFormat(const String& Identifier)
{
    setFormat(Identifier);
}

void RGBFormat::setFormat(Identifier id)
{
    if (id < unknown || id >= MaxIdentifiers) throw UnknownColorFormatException();
    format_id = id;
}

void RGBFormat::setFormat(const String& Identifier)
{
    if (Identifier == "unknown")
        format_id = unknown;
    else if (Identifier == "Monochrome1BitVertical")
        format_id = Monochrome1BitVertical;
    else if (Identifier == "GREY8")
        format_id = GREY8;
    else if (Identifier == "A8")
        format_id = A8;
    else if (Identifier == "R5G6B5")
        format_id = R5G6B5;
    else if (Identifier == "A8R8G8B8")
        format_id = A8R8G8B8;
#ifndef PICO_BUILD
    else if (Identifier == "Palette")
        format_id = Palette;
    else if (Identifier == "R3G3B2")
        format_id = R3G3B2;
    else if (Identifier == "B5G6R5")
        format_id = B5G6R5;
    else if (Identifier == "X1R5G5B5")
        format_id = X1R5G5B5;
    else if (Identifier == "X1B5G5R5")
        format_id = X1B5G5R5;
    else if (Identifier == "A1R5G5B5")
        format_id = A1R5G5B5;
    else if (Identifier == "A1B5G5R5")
        format_id = A1B5G5R5;
    else if (Identifier == "X4R4G4B4")
        format_id = X4R4G4B4;
    else if (Identifier == "X4B4G4R4")
        format_id = X4B4G4R4;
    else if (Identifier == "A4R4G4B4")
        format_id = A4R4G4B4;
    else if (Identifier == "A4B4G4R4")
        format_id = A4B4G4R4;
    else if (Identifier == "A8R3G3B2")
        format_id = A8R3G3B2;
    else if (Identifier == "R8G8B8")
        format_id = R8G8B8;
    else if (Identifier == "B8G8R8")
        format_id = B8G8R8;
    else if (Identifier == "A8B8G8R8")
        format_id = A8B8G8R8;
    else if (Identifier == "R8G8B8A8")
        format_id = R8G8B8A8;
    else if (Identifier == "R8G8B8X8")
        format_id = R8G8B8X8;
    else if (Identifier == "B8G8R8A8")
        format_id = B8G8R8A8;
    else if (Identifier == "B8G8R8X8")
        format_id = B8G8R8X8;
    else if (Identifier == "X8R8G8B8")
        format_id = X8R8G8B8;
    else if (Identifier == "X8B8G8R8")
        format_id = X8B8G8R8;

#endif
    else {
        throw UnknownColorFormatException(Identifier);
    }
}

String RGBFormat::name() const
{
    switch (format_id) {
    case RGBFormat::unknown:
        return "unknown";
    case RGBFormat::Monochrome1BitVertical:
        return "Monochrome1BitVertical";
    case RGBFormat::GREY8:
        return "GREY8";
    case RGBFormat::A8:
        return "A8";
    case RGBFormat::R5G6B5:
        return "R5G6B5";
    case RGBFormat::A8R8G8B8:
        return "A8R8G8B8";
#ifndef PICO_BUILD
    case RGBFormat::Palette:
        return "Palette";
    case RGBFormat::R3G3B2:
        return "R3G3B2";
    case RGBFormat::B5G6R5:
        return "B5G6R5";
    case RGBFormat::X1R5G5B5:
        return "X1R5G5B5";
    case RGBFormat::X1B5G5R5:
        return "X1B5G5R5";
    case RGBFormat::A1R5G5B5:
        return "A1R5G5B5";
    case RGBFormat::A1B5G5R5:
        return "A1B5G5R5";
    case RGBFormat::X4R4G4B4:
        return "X4R4G4B4";
    case RGBFormat::X4B4G4R4:
        return "X4B4G4R4";
    case RGBFormat::A4R4G4B4:
        return "A4R4G4B4";
    case RGBFormat::A4B4G4R4:
        return "A4B4G4R4";
    case RGBFormat::A8R3G3B2:
        return "A8R3G3B2";
    case RGBFormat::R8G8B8:
        return "R8G8B8";
    case RGBFormat::B8G8R8:
        return "B8G8R8";
    case RGBFormat::A8B8G8R8:
        return "A8B8G8R8";
    case RGBFormat::R8G8B8A8:
        return "R8G8B8A8";
    case RGBFormat::R8G8B8X8:
        return "R8G8B8X8";
    case RGBFormat::B8G8R8A8:
        return "B8G8R8A8";
    case RGBFormat::B8G8R8X8:
        return "B8G8R8X8";
    case RGBFormat::X8R8G8B8:
        return "X8R8G8B8";
    case RGBFormat::X8B8G8R8:
        return "X8B8G8R8";
#endif
    }
    return "unknown";
}

int RGBFormat::bitdepth() const
{
    switch (format_id) {
    case RGBFormat::A8:
    case RGBFormat::GREY8:
        return 8;

    case RGBFormat::Monochrome1BitVertical:
        return 1;

    case RGBFormat::R5G6B5:
        return 16;
    case RGBFormat::A8R8G8B8:
        return 32;

#ifndef PICO_BUILD
    case RGBFormat::Palette:
    case RGBFormat::R3G3B2:
        return 8;
    case RGBFormat::B5G6R5:
    case RGBFormat::X1R5G5B5:
    case RGBFormat::X1B5G5R5:
    case RGBFormat::X4R4G4B4:
    case RGBFormat::A1R5G5B5:
    case RGBFormat::A1B5G5R5:
    case RGBFormat::A4R4G4B4:
    case RGBFormat::A8R3G3B2:
        return 16;
    case RGBFormat::R8G8B8:
    case RGBFormat::B8G8R8:
        return 24;
    case RGBFormat::A8B8G8R8:
    case RGBFormat::R8G8B8A8:
    case RGBFormat::R8G8B8X8:
    case RGBFormat::B8G8R8A8:
    case RGBFormat::B8G8R8X8:
    case RGBFormat::X8R8G8B8:
    case RGBFormat::X8B8G8R8:
        return 32;
#endif
    };
    return 0;
}

uint32_t RGBFormat::bytesForWidth(uint32_t width) const
{
    if (format_id == Monochrome1BitVertical) {
        return width; // 1 Bit pro Pixel, aber vertikal gepackt: 8 Pixel pro Byte, also width/8, aber da vertikal gepackt, ist es einfach
                      // width
    }
    return (width * bitdepth()) >> 3;
}

} // namespace pplib::grafix