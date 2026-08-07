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

#ifndef PPLIB_INCLUDE_GRAFIX_IMAGE_H
#define PPLIB_INCLUDE_GRAFIX_IMAGE_H

#include <stdint.h>

namespace pplib::grafix
{
class Image : public Drawable
{
private:
    ByteArray myMemory;

public:
    /** @name Konstruktoren
     */
    //@{
    Image();
    Image(const Image& other);
    Image(const Drawable& other);
    Image(int width, int height, const RGBFormat& format = RGBFormat::A8R8G8B8);
    Image(const String& Filename, const RGBFormat& format = RGBFormat::unknown);
    Image(FileObject& file, const RGBFormat& format = RGBFormat::unknown);
    Image(const ByteArrayPtr& mem, const RGBFormat& format = RGBFormat::unknown);
    ~Image();
    //@}

    /** @name Verschiedenes
     */
    //@{
    void clear();
    void create(int width, int height, const RGBFormat& format = RGBFormat::A8R8G8B8);
    void create(void* base, uint32_t pitch, int width, int height, const RGBFormat& format = RGBFormat::A8R8G8B8);
    void load(const String& Filename, const RGBFormat& format = RGBFormat::unknown);
    void load(FileObject& file, const RGBFormat& format = RGBFormat::unknown);
    void load(const ByteArrayPtr& Mem, const RGBFormat& format = RGBFormat::unknown);
    void copy(const Drawable& other);
    void copy(const Drawable& other, const Rect& rect);
    void copy(const Image& other);
    Image& operator=(const Drawable& other);
    Image& operator=(const Image& other);
    size_t numBytes() const;
    ByteArrayPtr memory() const;
    operator ByteArrayPtr() const;
    //@}
};
} // namespace pplib::grafix
#endif // PPLIB_INCLUDE_GRAFIX_IMAGE_H
