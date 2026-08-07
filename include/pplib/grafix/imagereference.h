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

#ifndef PPLIB_INCLUDE_GRAFIX_IMAGEREFERENCE_H
#define PPLIB_INCLUDE_GRAFIX_IMAGEREFERENCE_H

#include <stdint.h>

namespace pplib::grafix
{
enum class DrawMethod : uint8_t
{
    BLT = 1,
    ALPHABLT,
    DIFFUSE
};

class ImageReference
{
    friend class Drawable;

private:
    Color diffuse_color;
    Drawable pixel;
    DrawMethod draw_method;

public:
    ImageReference();
    ImageReference(const Drawable& draw, DrawMethod method = DrawMethod::ALPHABLT, const Color& diffuse = Color());
    Size16 size() const;
    DrawMethod drawMethod() const;
    const Drawable& getDrawable() const;
    Color diffuseColor() const;
    void setDrawMethod(DrawMethod method);
    void setDiffuseColor(const Color& c);
    void setDrawable(const Drawable& draw);
    void useDrawable(const Drawable& draw, DrawMethod method, const Color& diffuse = Color());
    inline uint16_t width() const
    {
        return pixel.width();
    };
    inline uint16_t height() const
    {
        return pixel.height();
    };
    inline RGBFormat format() const
    {
        return pixel.format();
    };
    inline bool isEmpty() const
    {
        return pixel.isEmpty();
    };
};

} // namespace pplib::grafix
#endif // PPLIB_INCLUDE_GRAFIX_IMAGEREFERENCE_H
