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

#ifndef PPLIB_INCLUDE_GRAFIX_SIZE_H
#define PPLIB_INCLUDE_GRAFIX_SIZE_H

#include <stdint.h>

namespace pplib::grafix
{
class Size16;
class Size
{
public:
    int width, height;

    Size();
    Size(int width, int height);
    Size(const Size& other);
    Size(const Size16& other);
    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;
    void setHeight(int height);
    void setWidth(int width);
    void setSize(int width, int height);
    void setSize(const Size& s);
    Size& operator=(const Size16& other);
    Size& operator*=(double factor);
    Size& operator+=(const Size& size);
    Size& operator-=(const Size& size);
    Size& operator/=(double divisor);

    static Size invalid();
};

bool operator!=(const Size& s1, const Size& s2);
bool operator==(const Size& s1, const Size& s2);
const Size operator*(const Size& size, double factor);
const Size operator*(double factor, const Size& size);
const Size operator+(const Size& s1, const Size& s2);
const Size operator-(const Size& s1, const Size& s2);
const Size operator-(const Size& size);
const Size operator/(const Size& size, double divisor);

class Size16
{
public:
    int16_t width, height;

    Size16();
    Size16(int16_t width, int16_t height);
    Size16(const Size& other);
    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;
    void setHeight(int16_t height);
    void setWidth(int16_t width);
    void setSize(int16_t width, int16_t height);
    void setSize(const Size16& s);
    Size16& operator=(const Size& other);
    Size16& operator*=(float factor);
    Size16& operator+=(const Size16& size);
    Size16& operator-=(const Size16& size);
    Size16& operator/=(float divisor);
};

bool operator!=(const Size16& s1, const Size16& s2);
bool operator==(const Size16& s1, const Size16& s2);
const Size16 operator*(const Size16& size, float factor);
const Size16 operator*(float factor, const Size16& size);
const Size16 operator+(const Size16& s1, const Size16& s2);
const Size16 operator-(const Size16& s1, const Size16& s2);
const Size16 operator-(const Size16& size);
const Size16 operator/(const Size16& size, float divisor);

} // namespace pplib::grafix
#endif // PPLIB_INCLUDE_GRAFIX_SIZE_H