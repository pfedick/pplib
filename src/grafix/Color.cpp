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

#include "pplib/grafix/color.h"
#include <pplib/types/string.h>

namespace pplib::grafix
{

static inline uint8_t clamp(int value) noexcept
{
    if (value < 0) return 0;
    if (value > 255) return 255;
    return static_cast<uint8_t>(value);
}

static inline uint8_t clamp_max(int value) noexcept
{
    if (value > 255) return 255;
    return static_cast<uint8_t>(value);
}

bool Color::match(const Color& other, uint8_t tolerance) const noexcept
{
    if (c == other.c) return true;
    if (std::abs(static_cast<int>(r) - static_cast<int>(other.r)) > tolerance) return false;
    if (std::abs(static_cast<int>(g) - static_cast<int>(other.g)) > tolerance) return false;
    if (std::abs(static_cast<int>(b) - static_cast<int>(other.b)) > tolerance) return false;
    return true;
}

Color& Color::blend(const Color& background, const Color& foreground, int intensity) noexcept
{
    int i2 = intensity & 255;
    int i1 = 255 - i2;
    r = ((background.r * i1) + (foreground.r * i2)) / 255;
    g = ((background.g * i1) + (foreground.g * i2)) / 255;
    b = ((background.b * i1) + (foreground.b * i2)) / 255;
    a = 255;
    return *this;
}

Color& Color::blendf(const Color& background, const Color& foreground, float intensity) noexcept
{
    float i2 = intensity;
    float i1 = 1.0f - i2;
    r = (int)((background.r * i1) + (foreground.r * i2));
    g = (int)((background.g * i1) + (foreground.g * i2));
    b = (int)((background.b * i1) + (foreground.b * i2));
    a = 255;
    return *this;
}

Color Color::lerp(const Color& c1, const Color& c2, float factor) noexcept
{
    float i2 = factor;
    float i1 = 1.0f - i2;
    return Color(
        clamp((int)(((float)c1.red() * i1) + ((float)c2.red() * i2))), clamp((int)(((float)c1.green() * i1) + ((float)c2.green() * i2))),
        clamp((int)(((float)c1.blue() * i1) + ((float)c2.blue() * i2))), clamp((int)(((float)c1.alpha() * i1) + ((float)c2.alpha() * i2))));
}

Color Color::getBlended(const Color& background, const Color& foreground, int intensity) noexcept
{
    Color c;
    int i2 = intensity & 255;
    int i1 = 255 - i2;
    c.r = ((background.r * i1) + (foreground.r * i2)) / 255;
    c.g = ((background.g * i1) + (foreground.g * i2)) / 255;
    c.b = ((background.b * i1) + (foreground.b * i2)) / 255;
    c.a = 255;
    return c;
}

Color Color::getBlendedf(const Color& background, const Color& foreground, float intensity) noexcept
{
    Color c;
    float i2 = intensity;
    float i1 = 1.0f - i2;
    c.r = (int)((background.r * i1) + (foreground.r * i2));
    c.g = (int)((background.g * i1) + (foreground.g * i2));
    c.b = (int)((background.b * i1) + (foreground.b * i2));
    c.a = 255;
    return c;
}

Color multiplyWithAlpha(const Color& c, float factor) noexcept
{
    if (factor <= 0.0f) {
        return Color(0, 0, 0, 0);
    }
    return Color(clamp_max((int)((float)c.r * factor)), clamp_max((int)((float)c.g * factor)), clamp_max((int)((float)c.b * factor)),
                 clamp_max((int)((float)c.a * factor)));
}

Color& Color::operator*=(float factor) noexcept
{
    if (factor <= 0.0f) {
        r = g = b = 0;
        return *this;
    }
    r = clamp_max((int)((float)r * factor));
    g = clamp_max((int)((float)g * factor));
    b = clamp_max((int)((float)b * factor));
    return *this;
}

Color& Color::operator+=(const Color& other) noexcept
{
    r = clamp_max(r + other.r);
    g = clamp_max(g + other.g);
    b = clamp_max(b + other.b);
    a = clamp_max(a + other.a);
    return *this;
}

Color operator*(const Color& color, float factor) noexcept
{
    if (factor <= 0.0f) return Color(0, 0, 0, color.a);
    return Color(clamp_max((int)((float)color.r * factor)), clamp_max((int)((float)color.g * factor)),
                 clamp_max((int)((float)color.b * factor)), color.a);
}

Color operator*(float factor, const Color& color) noexcept
{
    if (factor <= 0.0f) return Color(0, 0, 0, color.a);
    return Color(clamp_max((int)((float)color.r * factor)), clamp_max((int)((float)color.g * factor)),
                 clamp_max((int)((float)color.b * factor)), color.a);
}

Color operator+(const Color& color1, const Color& color2) noexcept
{
    return Color(clamp_max(color1.r + color2.r), clamp_max(color1.g + color2.g), clamp_max(color1.b + color2.b),
                 clamp_max(color1.a + color2.a));
}

std::ostream& operator<<(std::ostream& s, const Color& c)
{
    String t;
    t.setf("RGB(red=%i, green=%i, blue=%i, alpha=%i)", c.red(), c.green(), c.blue(), c.alpha());
    return s.write((const char*)t.getPtr(), t.size());
}

} // namespace pplib::grafix
