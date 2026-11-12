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

#include <gtest/gtest.h>

#include <pplib/grafix/grafix.h>
#include <pplib/grafix/rgbformat.h>
#include <pplib/types/string.h>
#include "pplib-tests.h"

namespace
{

// Liefert true, wenn der gegebene Identifier im aktiven Build verfuegbar ist
static bool isAvailable(pplib::grafix::RGBFormat::Identifier id)
{
#ifdef PICO_BUILD
    return id <= pplib::grafix::RGBFormat::A8R8G8B8;
#else
    (void)id;
    return true;
#endif
}

TEST(GrafixRGBFormatTest, DefaultConstructor)
{
    pplib::grafix::RGBFormat f;
    EXPECT_EQ(pplib::grafix::RGBFormat::unknown, f.format());
    // String-Literale vergleicht GoogleTest sonst pointerweise mit String
    EXPECT_STREQ("unknown", f.name().c_str());
    EXPECT_EQ(0, f.bitdepth());
    EXPECT_EQ(0, f.bytesPerPixel());
    EXPECT_EQ(0, f.bitsPerPixel());
}

TEST(GrafixRGBFormatTest, ConstructorWithIdentifier)
{
    pplib::grafix::RGBFormat f(pplib::grafix::RGBFormat::A8R8G8B8);
    EXPECT_EQ(static_cast<int>(pplib::grafix::RGBFormat::A8R8G8B8), f.format());
    EXPECT_STREQ("A8R8G8B8", f.name().c_str());

    pplib::grafix::RGBFormat mono(pplib::grafix::RGBFormat::Monochrome1BitVertical);
    EXPECT_EQ(static_cast<int>(pplib::grafix::RGBFormat::Monochrome1BitVertical), mono.format());
}

TEST(GrafixRGBFormatTest, IdentifierOutOfRange)
{
    pplib::grafix::RGBFormat f(pplib::grafix::RGBFormat::Monochrome1BitVertical);

    EXPECT_THROW(f.setFormat(static_cast<pplib::grafix::RGBFormat::Identifier>(-1)), pplib::grafix::UnknownColorFormatException);
    EXPECT_THROW(f.setFormat(pplib::grafix::RGBFormat::MaxIdentifiers), pplib::grafix::UnknownColorFormatException);
    EXPECT_THROW(f.setFormat(static_cast<pplib::grafix::RGBFormat::Identifier>(9999)), pplib::grafix::UnknownColorFormatException);

    // Format bleibt unveraendert, wenn setFormat fehlschlaegt
    EXPECT_EQ(pplib::grafix::RGBFormat::Monochrome1BitVertical, f);
}

TEST(GrafixRGBFormatTest, ConstructorValidatesIdentifier)
{
    // Der Identifier-Konstruktor validiert wie setFormat(Identifier)
    EXPECT_THROW(pplib::grafix::RGBFormat(static_cast<pplib::grafix::RGBFormat::Identifier>(-1)),
                 pplib::grafix::UnknownColorFormatException);
    EXPECT_THROW(pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::MaxIdentifiers), pplib::grafix::UnknownColorFormatException);
    EXPECT_THROW(pplib::grafix::RGBFormat(static_cast<pplib::grafix::RGBFormat::Identifier>(9999)),
                 pplib::grafix::UnknownColorFormatException);
}

TEST(GrafixRGBFormatTest, ConstructorWithName)
{
    pplib::grafix::RGBFormat f("GREY8");
    EXPECT_EQ(pplib::grafix::RGBFormat::GREY8, f);
    EXPECT_EQ(8, f.bitdepth());

    pplib::grafix::RGBFormat mono("Monochrome1BitVertical");
    EXPECT_EQ(pplib::grafix::RGBFormat::Monochrome1BitVertical, mono);
}

TEST(GrafixRGBFormatTest, UnknownNameThrows)
{
    EXPECT_THROW(pplib::grafix::RGBFormat("R5G5B5"), pplib::grafix::UnknownColorFormatException);
    EXPECT_THROW(pplib::grafix::RGBFormat(""), pplib::grafix::UnknownColorFormatException);
    // Gross-/Kleinschreibung wird strikt verglichen
    EXPECT_THROW(pplib::grafix::RGBFormat("grey8"), pplib::grafix::UnknownColorFormatException);

    pplib::grafix::RGBFormat f(pplib::grafix::RGBFormat::GREY8);
    EXPECT_THROW(f.setFormat("A8R8B8G8"), pplib::grafix::UnknownColorFormatException);
    EXPECT_EQ(pplib::grafix::RGBFormat::GREY8, f);
}

TEST(GrafixRGBFormatTest, NameRoundTrip)
{
    // Fuer alle im aktiven Build verfuegbaren Identifier gilt: name() -> setFormat(name()) -> identisch
    for (int id = pplib::grafix::RGBFormat::unknown; id < pplib::grafix::RGBFormat::MaxIdentifiers; id++) {
        pplib::grafix::RGBFormat::Identifier ident = static_cast<pplib::grafix::RGBFormat::Identifier>(id);
        if (!isAvailable(ident)) continue;

        pplib::grafix::RGBFormat f(ident);
        const pplib::String formatname = f.name();
        EXPECT_EQ(id, f.format()) << "format() liefert unexpected Wert fuer ID " << id;

        pplib::grafix::RGBFormat f2(formatname);
        EXPECT_EQ(id, f2.format()) << "Name \"" << formatname << "\" resolved unexpected";
        EXPECT_EQ(f, f2);
    }
}

TEST(GrafixRGBFormatTest, Bitdepth)
{
    EXPECT_EQ(0, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::unknown).bitdepth());
    EXPECT_EQ(1, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::Monochrome1BitVertical).bitdepth());
    EXPECT_EQ(8, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::GREY8).bitdepth());
    EXPECT_EQ(16, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::R5G6B5).bitdepth());
    EXPECT_EQ(32, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::A8R8G8B8).bitdepth());

#ifndef PICO_BUILD
    EXPECT_EQ(8, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::A8).bitdepth());
    EXPECT_EQ(8, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::Palette).bitdepth());
    EXPECT_EQ(8, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::R3G3B2).bitdepth());

    EXPECT_EQ(16, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::B5G6R5).bitdepth());
    EXPECT_EQ(16, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::X1R5G5B5).bitdepth());
    EXPECT_EQ(16, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::X1B5G5R5).bitdepth());
    EXPECT_EQ(16, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::A1R5G5B5).bitdepth());
    EXPECT_EQ(16, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::A1B5G5R5).bitdepth());
    EXPECT_EQ(16, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::X4R4G4B4).bitdepth());
    EXPECT_EQ(16, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::X4B4G4R4).bitdepth());
    EXPECT_EQ(16, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::A4R4G4B4).bitdepth());
    EXPECT_EQ(16, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::A4B4G4R4).bitdepth());
    EXPECT_EQ(16, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::A8R3G3B2).bitdepth());

    EXPECT_EQ(24, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::R8G8B8).bitdepth());
    EXPECT_EQ(24, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::B8G8R8).bitdepth());

    EXPECT_EQ(32, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::A8B8G8R8).bitdepth());
    EXPECT_EQ(32, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::R8G8B8A8).bitdepth());
    EXPECT_EQ(32, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::R8G8B8X8).bitdepth());
    EXPECT_EQ(32, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::B8G8R8A8).bitdepth());
    EXPECT_EQ(32, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::B8G8R8X8).bitdepth());
    EXPECT_EQ(32, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::X8R8G8B8).bitdepth());
    EXPECT_EQ(32, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::X8B8G8R8).bitdepth());
#endif
}

TEST(GrafixRGBFormatTest, BytesPerPixel)
{
    // bytesPerPixel() ist bitdepth()/8; fuer Formate < 8 Bit laut Doku 0
    pplib::grafix::RGBFormat mono(pplib::grafix::RGBFormat::Monochrome1BitVertical);
    EXPECT_EQ(1, mono.bitsPerPixel());
    EXPECT_EQ(0, mono.bytesPerPixel());

    EXPECT_EQ(0, pplib::grafix::RGBFormat().bytesPerPixel());
    EXPECT_EQ(1, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::GREY8).bytesPerPixel());
    EXPECT_EQ(2, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::R5G6B5).bytesPerPixel());
    EXPECT_EQ(4, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::A8R8G8B8).bytesPerPixel());
#ifndef PICO_BUILD
    EXPECT_EQ(3, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::R8G8B8).bytesPerPixel());
    EXPECT_EQ(2, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::X4B4G4R4).bytesPerPixel());
    EXPECT_EQ(2, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::A4B4G4R4).bytesPerPixel());
#endif
}

TEST(GrafixRGBFormatTest, BytesForWidth)
{
    EXPECT_EQ(0, pplib::grafix::RGBFormat().bytesForWidth(100));
    EXPECT_EQ(0, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::GREY8).bytesForWidth(0));

    // Sonderfall: vertikal gepackt, ein Byte pro Spalte unabhaengig von der Teilbarkeit durch 8
    pplib::grafix::RGBFormat mono(pplib::grafix::RGBFormat::Monochrome1BitVertical);
    EXPECT_EQ(0, mono.bytesForWidth(0));
    EXPECT_EQ(5, mono.bytesForWidth(5));
    EXPECT_EQ(8, mono.bytesForWidth(8));

    EXPECT_EQ(5, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::GREY8).bytesForWidth(5));
    EXPECT_EQ(10, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::R5G6B5).bytesForWidth(5));
    EXPECT_EQ(20, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::A8R8G8B8).bytesForWidth(5));

    // 24 Bit: ganzzahlige Division, hier ohne Rest
    EXPECT_EQ(15, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::R8G8B8).bytesForWidth(5));
    EXPECT_EQ(6, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::R8G8B8).bytesForWidth(2));

    // Grenzen des uint32_t: 4 Byte pro Pixel, ohne Overflow bis 2^30-1
    EXPECT_EQ(4u * 1024, pplib::grafix::RGBFormat(pplib::grafix::RGBFormat::A8R8G8B8).bytesForWidth(1024));
}

TEST(GrafixRGBFormatTest, AssignmentFromIdentifier)
{
    pplib::grafix::RGBFormat f;
    f = pplib::grafix::RGBFormat::GREY8;
    EXPECT_EQ(pplib::grafix::RGBFormat::GREY8, f);
    EXPECT_EQ(8, f.bitdepth());
}

TEST(GrafixRGBFormatTest, ComparisonOperators)
{
    const pplib::grafix::RGBFormat a(pplib::grafix::RGBFormat::GREY8);
    const pplib::grafix::RGBFormat b("GREY8");
    const pplib::grafix::RGBFormat c(pplib::grafix::RGBFormat::A8R8G8B8);

    // RGBFormat gegen RGBFormat
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
    EXPECT_FALSE(a == c);

    // RGBFormat gegen Identifier, in beiden Richtungen
    EXPECT_TRUE(a == pplib::grafix::RGBFormat::GREY8);
    EXPECT_TRUE(pplib::grafix::RGBFormat::GREY8 == a);
    EXPECT_TRUE(a != pplib::grafix::RGBFormat::A8R8G8B8);
    EXPECT_TRUE(pplib::grafix::RGBFormat::A8R8G8B8 != a);
    EXPECT_FALSE(a != pplib::grafix::RGBFormat::GREY8);
    EXPECT_FALSE(pplib::grafix::RGBFormat::GREY8 != a);
    EXPECT_FALSE(a == pplib::grafix::RGBFormat::A8R8G8B8);
    EXPECT_FALSE(pplib::grafix::RGBFormat::A8R8G8B8 == a);

    // impliziter Integer-Cast
    EXPECT_EQ(static_cast<int>(pplib::grafix::RGBFormat::GREY8), static_cast<int>(a));
}

} // namespace
