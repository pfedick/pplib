/*******************************************************************************
 * This file is part of "Patrick's Programming Library" for Raspberry Pico,
 * based on PPLib Version 7.
 * Web: https://github.com/pfedick/pico-pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include "picopplib-grafix.h"
#include <assert.h>

namespace picopplib
{

static inline int16_t clamp16(int value)
{
    if (value < -32768) return -32768;
    if (value > 32767) return 32767;
    return value;
}

Size16::Size16()
{
    width = 0;
    height = 0;
}

Size16::Size16(int16_t width, int16_t height)
{
    this->width = width;
    this->height = height;
}

Size16::Size16(const Size& other)
{
    width = clamp16(other.width);
    height = clamp16(other.height);
}

bool Size16::isNull() const
{
    if (height == 0 && width == 0) return true;
    return false;
}

bool Size16::isEmpty() const
{
    if (height == 0 || width == 0) return true;
    return false;
}

bool Size16::isValid() const
{
    if (width < 0) return false;
    if (height < 0) return false;
    return true;
}

void Size16::setHeight(int16_t height)
{
    this->height = height;
}

void Size16::setWidth(int16_t width)
{
    this->width = width;
}

void Size16::setSize(int16_t width, int16_t height)
{
    this->width = width;
    this->height = height;
}

void Size16::setSize(const Size16& other)
{
    width = other.width;
    height = other.height;
}

Size16& Size16::operator=(const Size& other)
{
    width = clamp16(other.width);
    height = clamp16(other.height);
    return *this;
}

Size16& Size16::operator*=(float factor)
{
    width = (int)(width * factor);
    height = (int)(height * factor);
    return *this;
}

Size16& Size16::operator+=(const Size16& size)
{
    width += size.width;
    height += size.height;
    return *this;
}

Size16& Size16::operator-=(const Size16& size)
{
    width -= size.width;
    height -= size.height;
    return *this;
}

Size16& Size16::operator/=(float divisor)
{
    assert(divisor != 0.0 && "Division by zero in Size16 operator /");
    width = (int)(width / divisor);
    height = (int)(height / divisor);
    return *this;
}

bool operator!=(const Size16& s1, const Size16& s2)
{
    if (s1.width != s2.width) return true;
    if (s1.height != s2.height) return true;
    return false;
}

bool operator==(const Size16& s1, const Size16& s2)
{
    if (s1.width != s2.width) return false;
    if (s1.height != s2.height) return false;
    return true;
}

const Size16 operator*(const Size16& size, float factor)
{
    return Size16((int)(size.width * factor), (int)(size.height * factor));
}

const Size16 operator*(float factor, const Size16& size)
{
    return Size16((int)(size.width * factor), (int)(size.height * factor));
}

const Size16 operator+(const Size16& s1, const Size16& s2)
{
    return Size16(s1.width + s2.width, s1.height + s2.height);
}

const Size16 operator-(const Size16& s1, const Size16& s2)
{
    return Size16(s1.width - s2.width, s1.height - s2.height);
}

const Size16 operator-(const Size16& size)
{
    return Size16(0 - size.width, 0 - size.height);
}

const Size16 operator/(const Size16& size, float divisor)
{
    return Size16((int)(size.width / divisor), (int)(size.height / divisor));
}

} // namespace picopplib
