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

#ifndef PPLIB_INCLUDE_GRAFIX_IMAGEFILTER_H
#define PPLIB_INCLUDE_GRAFIX_IMAGEFILTER_H

#include <stdint.h>

#include <pplib/types/string.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/assocarray.h>
#include <pplib/core/fileobject.h>
#include <pplib/grafix/drawable.h>

#include <pplib/grafix/rgbformat.h>

namespace pplib::grafix
{

typedef struct
{
    int width;
    int height;
    int pitch;
    int bitdepth;
    int colors;
    RGBFormat format;
} IMAGE;

class ImageFilter
{
private:
public:
    PPLIBEXCEPTION(IllegalImageFormatException, Exception);
    PPLIBEXCEPTION(EmptyImageException, Exception);

    ImageFilter();
    virtual ~ImageFilter();
    virtual int ident(FileObject& file, IMAGE& img);
    virtual void load(FileObject& file, Drawable& surface, IMAGE& img);
    virtual void save(const Drawable& surface, FileObject& file, const Rect& area, const AssocArray& param = AssocArray());
    virtual void save(const Drawable& surface, FileObject& file, const AssocArray& param = AssocArray());
    virtual String name();
    virtual String description();

    void saveFile(const String& filename, const Drawable& surface, const Rect& area, const AssocArray& param = AssocArray());
    void saveFile(const String& filename, const Drawable& surface, const AssocArray& param = AssocArray());
};

class ImageFilter_PNG : public ImageFilter
{
public:
    ImageFilter_PNG();
    virtual ~ImageFilter_PNG();
    virtual int ident(FileObject& file, IMAGE& img);
    virtual void load(FileObject& file, Drawable& surface, IMAGE& img);
    virtual void save(const Drawable& surface, FileObject& file, const AssocArray& param = AssocArray());
    virtual String name();
    virtual String description();
};

class ImageFilter_JPEG : public ImageFilter
{
public:
    ImageFilter_JPEG();
    virtual ~ImageFilter_JPEG();
    virtual int ident(FileObject& file, IMAGE& img);
    virtual void load(FileObject& file, Drawable& surface, IMAGE& img);
    virtual void save(const Drawable& surface, FileObject& file, const AssocArray& param = AssocArray());
    virtual String name();
    virtual String description();
};

class ImageFilter_BMP : public ImageFilter
{
public:
    ImageFilter_BMP();
    virtual ~ImageFilter_BMP();
    virtual int ident(FileObject& file, IMAGE& img);
    virtual void load(FileObject& file, Drawable& surface, IMAGE& img);
    virtual void save(const Drawable& surface, FileObject& file, const AssocArray& param = AssocArray());
    virtual String name();
    virtual String description();
};

class ImageFilter_TIFF : public ImageFilter
{
public:
    ImageFilter_TIFF();
    virtual ~ImageFilter_TIFF();
    virtual int ident(FileObject& file, IMAGE& img);
    virtual void load(FileObject& file, Drawable& surface, IMAGE& img);
    virtual void save(const Drawable& surface, FileObject& file, const AssocArray& param = AssocArray());
    virtual String name();
    virtual String description();
};

class ImageFilter_GIF : public ImageFilter
{
public:
    ImageFilter_GIF();
    virtual ~ImageFilter_GIF();
    virtual int ident(FileObject& file, IMAGE& img);
    virtual void load(FileObject& file, Drawable& surface, IMAGE& img);
    virtual void save(const Drawable& surface, FileObject& file, const AssocArray& param = AssocArray());
    virtual String name();
    virtual String description();
};

class ImageFilter_PPM : public ImageFilter
{
public:
    ImageFilter_PPM();
    virtual ~ImageFilter_PPM();
    virtual int ident(FileObject& file, IMAGE& img);
    virtual void load(FileObject& file, Drawable& surface, IMAGE& img);
    virtual void save(const Drawable& surface, FileObject& file, const AssocArray& param = AssocArray());
    virtual String name();
    virtual String description();
};

class ImageFilter_TGA : public ImageFilter
{
public:
    ImageFilter_TGA();
    virtual ~ImageFilter_TGA();
    virtual int ident(FileObject& file, IMAGE& img);
    virtual void load(FileObject& file, Drawable& surface, IMAGE& img);
    virtual void save(const Drawable& surface, FileObject& file, const AssocArray& param = AssocArray());
    virtual String name();
    virtual String description();
};

} // namespace pplib::grafix
#endif // PPLIB_INCLUDE_GRAFIX_IMAGEFILTER_H
