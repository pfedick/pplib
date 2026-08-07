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

#ifndef PPLIB_INCLUDE_GRAFIX_IMAGELIST_H
#define PPLIB_INCLUDE_GRAFIX_IMAGELIST_H

#include <stdint.h>

namespace pplib::grafix
{
class ImageList : public Image
{
    friend class Drawable;

public:
    enum DRAWMETHOD
    {
        BLT = 1,
        COLORKEY,
        ALPHABLT,
        DIFFUSE
    };

private:
    size_t numIcons;
    int width, height;
    int numX, numY;
    Color colorkey;
    Color diffuse;
    DRAWMETHOD method;

public:
    ImageList();
    ImageList(const ImageList& other);
    ImageList(const Drawable& draw, int icon_width, int icon_height, DRAWMETHOD method);
    ImageList(const String& Filename, int icon_width, int icon_height, DRAWMETHOD method);
    ImageList(FileObject& file, int icon_width, int icon_height, DRAWMETHOD method);
    ImageList(const ByteArrayPtr& mem, int icon_width, int icon_height, DRAWMETHOD method);
    ~ImageList();

    void clear();
    void setDrawMethod(DRAWMETHOD method);
    void setColorKey(const Color& key);
    void setDiffuseColor(const Color& c);
    void setIconSize(int width, int height);

    void copy(const ImageList& other);
    void load(const Drawable& draw, int icon_width, int icon_height, DRAWMETHOD method);
    void load(const String& Filename, int icon_width, int icon_height, DRAWMETHOD method);
    void load(FileObject& file, int icon_width, int icon_height, DRAWMETHOD method);
    void load(const ByteArrayPtr& mem, int icon_width, int icon_height, DRAWMETHOD method);

    size_t num() const;
    Size iconSize() const;
    Rect getRect(size_t nr) const;
    DRAWMETHOD drawMethod() const;
    Drawable getDrawable(size_t nr) const;
    Image getDrawable(size_t nr, const Color& diffuse_color) const;
    Color colorKey() const;
    Color diffuseColor() const;

    ImageList& operator=(const ImageList& other);
};

} // namespace pplib::grafix
#endif // PPLIB_INCLUDE_GRAFIX_IMAGELIST_H
