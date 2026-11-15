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

#include <sstream>

#include <pplib/grafix/color.h>
#include "pplib-tests.h"

namespace
{

TEST(GrafixColorTest, Constructors)
{
    // Default: kompletter 32-Bit-Wert 0, also schwarz mit Alpha 0 (transparent)
    pplib::grafix::Color black;
    EXPECT_EQ(0, black.red());
    EXPECT_EQ(0, black.green());
    EXPECT_EQ(0, black.blue());
    EXPECT_EQ(0, black.alpha());
    EXPECT_EQ(0u, black.rgba());

    // Alpha-Default ist 255 (sichtbar)
    pplib::grafix::Color c(10, 20, 30);
    EXPECT_EQ(10, c.red());
    EXPECT_EQ(20, c.green());
    EXPECT_EQ(30, c.blue());
    EXPECT_EQ(255, c.alpha());

    pplib::grafix::Color transparent(10, 20, 30, 0);
    EXPECT_EQ(0, transparent.alpha());

    // Copy
    pplib::grafix::Color copy(c);
    EXPECT_EQ(c, copy);
}

TEST(GrafixColorTest, ConstructorFromUint32)
{
    pplib::grafix::Color c(0x11223344u);
    EXPECT_EQ(0x11223344u, c.rgba());
    EXPECT_EQ(static_cast<uint32_t>(0x11223344u), static_cast<uint32_t>(c));

    // Die Bytes des 32-Bit-Werts liegen je nach Endianess in unterschiedlicher
    // Reihenfolge auf den Komponenten r/g/b/a
#ifdef __BIG_ENDIAN__
    EXPECT_EQ(0x11, c.red());
    EXPECT_EQ(0x22, c.green());
    EXPECT_EQ(0x33, c.blue());
    EXPECT_EQ(0x44, c.alpha());
#else
    EXPECT_EQ(0x44, c.red());
    EXPECT_EQ(0x33, c.green());
    EXPECT_EQ(0x22, c.blue());
    EXPECT_EQ(0x11, c.alpha());
#endif
}

TEST(GrafixColorTest, Setters)
{
    pplib::grafix::Color c;

    c.setRed(1);
    c.setGreen(2);
    c.setBlue(3);
    c.setAlpha(4);
    EXPECT_EQ(1, c.red());
    EXPECT_EQ(2, c.green());
    EXPECT_EQ(3, c.blue());
    EXPECT_EQ(4, c.alpha());

    c.setColor(0xAABBCCDDu);
    EXPECT_EQ(0xAABBCCDDu, c.rgba());

    c.setColor(5, 6, 7);
    EXPECT_EQ(5, c.red());
    EXPECT_EQ(6, c.green());
    EXPECT_EQ(7, c.blue());
    EXPECT_EQ(255, c.alpha());

    c.set(8, 9, 10, 11);
    EXPECT_EQ(8, c.red());
    EXPECT_EQ(9, c.green());
    EXPECT_EQ(10, c.blue());
    EXPECT_EQ(11, c.alpha());

    c.set(pplib::grafix::Color(100, 110, 120, 130));
    EXPECT_EQ(pplib::grafix::Color(100, 110, 120, 130), c);
}

TEST(GrafixColorTest, Brightness)
{
    // Helligkeit = (r*11 + g*16 + b*5)/32, Alpha bleibt ausser Betracht
    EXPECT_EQ(0, pplib::grafix::Color(0, 0, 0).brightness());
    EXPECT_EQ(255, pplib::grafix::Color(255, 255, 255).brightness());
    EXPECT_EQ(87, pplib::grafix::Color(255, 0, 0).brightness());
    EXPECT_EQ(127, pplib::grafix::Color(0, 255, 0).brightness());
    EXPECT_EQ(39, pplib::grafix::Color(0, 0, 255).brightness());
    // Alpha hat keinen Einfluss auf die Helligkeit
    EXPECT_EQ(87, pplib::grafix::Color(255, 0, 0, 0).brightness());
}

TEST(GrafixColorTest, Grey)
{
    // Alle Kanale erhalten den Helligkeitswert, Alpha wird uebernommen
    pplib::grafix::Color grey = pplib::grafix::Color(255, 0, 0, 128).grey();
    EXPECT_EQ(87, grey.red());
    EXPECT_EQ(87, grey.green());
    EXPECT_EQ(87, grey.blue());
    EXPECT_EQ(128, grey.alpha());

    // Weisse Farben bleiben weiss
    EXPECT_EQ(pplib::grafix::Color(255, 255, 255, 255), pplib::grafix::Color(255, 255, 255).grey());
}

TEST(GrafixColorTest, Negative)
{
    pplib::grafix::Color neg = pplib::grafix::Color(1, 2, 3, 128).negative();
    EXPECT_EQ(pplib::grafix::Color(254, 253, 252, 128), neg);
    // Doppelte Negation ergibt wieder den Ausgangswert
    EXPECT_EQ(pplib::grafix::Color(1, 2, 3, 128), neg.negative());
}

TEST(GrafixColorTest, MatchWithTolerance)
{
    const pplib::grafix::Color base(100, 100, 100);

    EXPECT_TRUE(base.match(base));
    EXPECT_TRUE(base.match(pplib::grafix::Color(100, 100, 100, 77))); // Alpha wird ignoriert
    EXPECT_FALSE(base.match(pplib::grafix::Color(101, 100, 100)));

    EXPECT_TRUE(base.match(pplib::grafix::Color(103, 97, 100), 3));
    EXPECT_FALSE(base.match(pplib::grafix::Color(103, 96, 100), 3)); // gruen weicht um 4 ab
    EXPECT_TRUE(pplib::grafix::Color(0, 0, 0).match(pplib::grafix::Color(255, 255, 255), 255));
}

TEST(GrafixColorTest, YCbCrConversion)
{
    // Exakte Fixed-Point-Werte der Implementierung
    pplib::grafix::Color white(255, 255, 255);
    EXPECT_EQ(255, white.getY());
    EXPECT_EQ(128, white.getYCb());
    EXPECT_EQ(128, white.getYCr());

    pplib::grafix::Color black(0, 0, 0);
    EXPECT_EQ(0, black.getY());
    EXPECT_EQ(128, black.getYCb());
    EXPECT_EQ(128, black.getYCr());

    // Reines Blau: Cb wird auf 255 begrenzt, reines Rot: Cr wird auf 255 begrenzt
    EXPECT_EQ(255, pplib::grafix::Color(0, 0, 255).getYCb());
    EXPECT_EQ(255, pplib::grafix::Color(255, 0, 0).getYCr());

    // Primärfarben mit bekannten Naeherungswerten
    EXPECT_EQ(76, pplib::grafix::Color(255, 0, 0).getY());
    EXPECT_EQ(85, pplib::grafix::Color(255, 0, 0).getYCb());
    EXPECT_EQ(21, pplib::grafix::Color(0, 255, 0).getYCr());
}

TEST(GrafixColorTest, Blend)
{
    pplib::grafix::Color bg(0, 0, 0), fg(255, 255, 255);

    // intensity 0 -> nur Hintergrund, 255 -> nur Vordergrund, Alpha immer 255
    pplib::grafix::Color c;
    c.blend(bg, fg, 0);
    EXPECT_EQ(pplib::grafix::Color(0, 0, 0, 255), c);
    c.blend(bg, fg, 255);
    EXPECT_EQ(pplib::grafix::Color(255, 255, 255, 255), c);
    c.blend(bg, fg, 128);
    EXPECT_EQ(pplib::grafix::Color(128, 128, 128, 255), c);

    // Die statischen Varianten liefern identische Werte
    EXPECT_EQ(pplib::grafix::Color(128, 128, 128, 255), pplib::grafix::Color::getBlended(bg, fg, 128));
    EXPECT_EQ(pplib::grafix::Color(0, 0, 0, 255), pplib::grafix::Color::getBlended(bg, fg, 0));
}

TEST(GrafixColorTest, BlendFloat)
{
    pplib::grafix::Color bg(0, 0, 0), fg(255, 255, 255);

    pplib::grafix::Color c;
    c.blendf(bg, fg, 0.0f);
    EXPECT_EQ(pplib::grafix::Color(0, 0, 0, 255), c);
    c.blendf(bg, fg, 1.0f);
    EXPECT_EQ(pplib::grafix::Color(255, 255, 255, 255), c);
    c.blendf(bg, fg, 0.5f);
    EXPECT_EQ(pplib::grafix::Color(127, 127, 127, 255), c); // Abschneiden nach Komma

    EXPECT_EQ(pplib::grafix::Color(127, 127, 127, 255), pplib::grafix::Color::getBlendedf(bg, fg, 0.5f));
}

TEST(GrafixColorTest, Lerp)
{
    const pplib::grafix::Color black(0, 0, 0, 0);
    const pplib::grafix::Color white(255, 255, 255, 255);

    // t=0 liefert c1, t=1 liefert c2 (Standard-Konvention, auch der Alpha-Kanal wird interpoliert)
    EXPECT_EQ(black, pplib::grafix::Color::lerp(black, white, 0.0f));
    EXPECT_EQ(white, pplib::grafix::Color::lerp(black, white, 1.0f));
    EXPECT_EQ(pplib::grafix::Color(127, 127, 127, 127), pplib::grafix::Color::lerp(black, white, 0.5f));
}

TEST(GrafixColorTest, MultiplyWithAlpha)
{
    const pplib::grafix::Color c(100, 150, 200, 128);

    // Alle vier Kanale inkl. Alpha werden skaliert
    pplib::grafix::Color half = pplib::grafix::multiplyWithAlpha(c, 0.5f);
    EXPECT_EQ(pplib::grafix::Color(50, 75, 100, 64), half);

    // Werte ueber 255 werden begrenzt
    pplib::grafix::Color bright = pplib::grafix::multiplyWithAlpha(c, 2.0f);
    EXPECT_EQ(pplib::grafix::Color(200, 255, 255, 255), bright);

    // Faktor <= 0 erzeugt vollstaendig transparentes Schwarz
    EXPECT_EQ(pplib::grafix::Color(0, 0, 0, 0), pplib::grafix::multiplyWithAlpha(c, 0.0f));
    EXPECT_EQ(pplib::grafix::Color(0, 0, 0, 0), pplib::grafix::multiplyWithAlpha(c, -1.0f));
}

TEST(GrafixColorTest, MultiplyAssignOperator)
{
    pplib::grafix::Color c(100, 150, 200, 128);
    c *= 0.5f;
    EXPECT_EQ(pplib::grafix::Color(50, 75, 100, 128), c); // Alpha bleibt unveraendert

    c *= 2.5f;
    EXPECT_EQ(pplib::grafix::Color(125, 187, 250, 128), c);

    // Negativer Faktor: RGB auf 0, Alpha bleibt erhalten
    c *= -1.0f;
    EXPECT_EQ(pplib::grafix::Color(0, 0, 0, 128), c);
}

TEST(GrafixColorTest, FreeMultiplyOperators)
{
    const pplib::grafix::Color c(100, 150, 200, 128);

    // Beide Richtungen, Alpha bleibt unveraendert
    EXPECT_EQ(pplib::grafix::Color(50, 75, 100, 128), c * 0.5f);
    EXPECT_EQ(pplib::grafix::Color(50, 75, 100, 128), 0.5f * c);
    EXPECT_EQ(pplib::grafix::Color(200, 255, 255, 128), c * 2.0f);

    // Faktor <= 0: RGB schwarz, Alpha bleibt erhalten
    EXPECT_EQ(pplib::grafix::Color(0, 0, 0, 128), c * 0.0f);
    EXPECT_EQ(pplib::grafix::Color(0, 0, 0, 128), 0.0f * c);
}

TEST(GrafixColorTest, AddOperators)
{
    pplib::grafix::Color c(100, 150, 200, 100);
    c += pplib::grafix::Color(50, 10, 30, 50);
    EXPECT_EQ(pplib::grafix::Color(150, 160, 230, 150), c);

    // Alle Komponenten inklusive Alpha werden bei Uberlauf auf 255 begrenzt
    c += pplib::grafix::Color(200, 200, 200, 200);
    EXPECT_EQ(pplib::grafix::Color(255, 255, 255, 255), c);

    EXPECT_EQ(pplib::grafix::Color(255, 255, 255, 255), pplib::grafix::Color(200, 100, 50, 200) + pplib::grafix::Color(100, 200, 250, 100));
}

TEST(GrafixColorTest, ComparisonOperators)
{
    const pplib::grafix::Color a(1, 2, 3, 4);
    const pplib::grafix::Color b(1, 2, 3, 4);
    const pplib::grafix::Color max(255, 255, 255, 255);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != pplib::grafix::Color(1, 2, 3, 5));

    // Vergleich ueber den kompletten 32-Bit-Wert (Alpha fliesst ein)
    EXPECT_TRUE(pplib::grafix::Color(1, 2, 3, 4) < max);
    EXPECT_TRUE(max > pplib::grafix::Color(1, 2, 3, 4));
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a >= b);
    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a < b);
}

TEST(GrafixColorTest, OutputStream)
{
    std::ostringstream stream;
    stream << pplib::grafix::Color(1, 2, 3, 4);
    EXPECT_EQ("RGB(red=1, green=2, blue=3, alpha=4)", stream.str());
}

} // namespace
