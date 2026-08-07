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

#ifndef PPLIB_INCLUDE_GRAFIX_GRAFIX_H
#define PPLIB_INCLUDE_GRAFIX_GRAFIX_H

#include <stdint.h>

namespace pplib::grafix
{
class ImageFilter;
class FontEngine;
class FontFile;

class Grafix
{
private:
    Mutex myMutex;
    std::list<ImageFilter*> ImageFilterList;
    std::list<FontEngine*> FontEngineList;
    std::map<String, FontFile*> FontList;

    ImageFilter* filter_png;
    ImageFilter* filter_jpeg;
    ImageFilter* filter_bmp;
    ImageFilter* filter_gif;
    ImageFilter* filter_ppm;
    ImageFilter* filter_tga;
    ImageFilter* filter_tiff;
    ImageFilter* filter_magick;
    RGBFormat PrimaryRGBFormat;

    void initAlphatab();

    void initFunctions(const RGBFormat& format, GRAFIX_FUNCTIONS* fn);
    void initColors(const RGBFormat& format, GRAFIX_FUNCTIONS* fn);
    void initPixel(const RGBFormat& format, GRAFIX_FUNCTIONS* fn);
    void initShapes(const RGBFormat& format, GRAFIX_FUNCTIONS* fn);
    void initLines(const RGBFormat& format, GRAFIX_FUNCTIONS* fn);
    void initBlits(const RGBFormat& format, GRAFIX_FUNCTIONS* fn);

public:
    Grafix();
    ~Grafix();

    ImageList Toolbar;
    ImageList ButtonSymbolsSmall;
    ImageList Icons32;

    GRAFIX_FUNCTIONS* getGrafixFunctions(const RGBFormat& format);

    // Image-Filter und Loader
    void addImageFilter(ImageFilter* filter);
    void unloadImageFilter(ImageFilter* filter);
    ImageFilter* findImageFilter(const String& name);
    ImageFilter* findImageFilter(FileObject& ff, IMAGE& img);

    // Fonts
    void addFontEngine(FontEngine* engine);
    void loadFont(const String& filename, const String& fontname = String());
    void loadFont(FileObject& ff, const String& fontname = String());
    void loadFont(const ByteArrayPtr& memory, const String& fontname = String());
    void unloadFont(const String& fontname);
    FontFile* findFont(const String& fontname);
    FontFile* findFont(const Font& font);
    void listFonts();
};

Grafix* GetGrafix();
} // namespace pplib::grafix
#endif // PPLIB_INCLUDE_GRAFIX_GRAFIX_H
