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

#ifndef PPLIB_INCLUDE_GRAFIX_SPRITE_H
#define PPLIB_INCLUDE_GRAFIX_SPRITE_H

#include <stdint.h>

namespace pplib::grafix
{
class Sprite
{
private:
    class SpriteTexture
    {
    public:
        int id;
        int width, height;
        int bitdepth;
        RGBFormat rgbformat;
        Image surface;

        SpriteTexture()
        {
            id = width = height = bitdepth = 0;
        }
        ~SpriteTexture()
        {
        }
    };

    class SpriteIndexItem
    {
    public:
        int id;
        const Drawable* surface;
        Rect r;
        Point Pivot;
        Point Offset;

        SpriteIndexItem()
        {
            id = 0;
            surface = NULL;
        }
        SpriteIndexItem(const SpriteIndexItem& other)
            : r(other.r),
              Pivot(other.Pivot),
              Offset(other.Offset)
        {
            id = other.id;
            surface = other.surface;
        }
    };
    std::map<int, SpriteTexture> TextureList;
    std::map<int, SpriteIndexItem> SpriteList;

    void loadTexture(PFPChunk* chunk);
    void loadIndex(PFPChunk* chunk);
    const Drawable* findTexture(int id) const;

public:
    Sprite();
    ~Sprite();
    void load(const String& filename);
    void load(FileObject& ff);
    void clear();
    void draw(Drawable& target, int x, int y, int id) const;
    int numTextures() const;
    int numSprites() const;
};
} // namespace pplib::grafix
#endif // PPLIB_INCLUDE_GRAFIX_SPRITE_H
