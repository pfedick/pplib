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

#include <string.h>
#include <pplib/grafix/drawable.h>
#include <pplib/grafix/grafix.h>
#include <pplib/grafix/imagefilter.h>
#include <config_pplib.h>

namespace pplib::grafix
{

/// Grafix-Engine (Singleton)
static Grafix* pplgfx = nullptr;

/** @brief Array mit den Grafix-Funktionen für die einzelnen Farbformate.
 *
 * Das Array enthält für jedes RGBFormat::Identifier einen Zeiger auf die zugehörigen Grafix-Funktionen.
 * Wenn ein Format noch nicht initialisiert wurde, ist der Zeiger auf nullptr gesetzt.
 * Die Funktionen werden bei Bedarf (Lazy-Loading) initialisiert und im Array gespeichert.
 * @note
 * Das Array ist statisch und wird beim Start der Anwendung einmalig initialisiert.
 */
static DRAWABLE_FUNCTIONS* drawable_functions[RGBFormat::MaxIdentifiers] = {nullptr};

Grafix& GetGrafix()
{
    if (pplgfx) return *pplgfx;
    throw NoGrafixEngineException();
}

Grafix::Grafix()
{
    if (pplgfx) {
        throw DuplicateGrafixEngineException();
    }
    pplgfx = this;

    // Standardfilter anlegen

    ImageFilter* filter = new ImageFilter_BMP;
    addImageFilter(filter);
    filter = new ImageFilter_GIF;
    addImageFilter(filter);

#ifdef HAVE_PNG
    filter = new ImageFilter_PNG;
    addImageFilter(filter);
#endif
#ifdef HAVE_JPEG
    filter = new ImageFilter_JPEG;
    addImageFilter(filter);
#endif
#ifdef HAVE_TIFF
    filter = new ImageFilter_TIFF;
    addImageFilter(filter);
#endif
    filter = new ImageFilter_PPM;
    addImageFilter(filter);

    filter = new ImageFilter_TGA;
    addImageFilter(filter);

    FontEngineFont5* font5 = new FontEngineFont5;
    addFontEngine(font5);
    FontEngineFont6* font6 = new FontEngineFont6;
    addFontEngine(font6);

#ifdef HAVE_FREETYPE2
    FontEngineFreeType* freetype = new FontEngineFreeType;
    addFontEngine(freetype);
#endif
}

Grafix::~Grafix()
{
    // cleanup grafix functions
    for (int i = 0; i < RGBFormat::MaxIdentifiers; i++) {
        if (drawable_functions[i]) {
            delete drawable_functions[i];
            drawable_functions[i] = nullptr;
        }
    }

    // cleanup image filters
    for (auto it = ImageFilterList.begin(); it != ImageFilterList.end(); ++it) {
        delete *it;
    }
    ImageFilterList.clear();

    // cleanup fonts
    for (auto it = FontList.begin(); it != FontList.end(); ++it) {
        delete it->second;
    }
    FontList.clear();
    // cleanup font engines
    for (auto it = FontEngineList.begin(); it != FontEngineList.end(); ++it) {
        delete *it;
    }
    FontEngineList.clear();
    if (pplgfx == this) pplgfx = NULL;
}

DRAWABLE_FUNCTIONS* getDefaultGrafixFunctions(); // definiert in src/grafix/DrawableDefault.cpp

DRAWABLE_FUNCTIONS* Grafix::getGrafixFunctions(const RGBFormat& format)
{
    if (format >= RGBFormat::MaxIdentifiers) throw UnknownColorFormatException();
    if (drawable_functions[format]) return drawable_functions[format];
    DRAWABLE_FUNCTIONS* fn = new DRAWABLE_FUNCTIONS;
    DRAWABLE_FUNCTIONS* defaultfn = getDefaultGrafixFunctions();
    memcpy(fn, defaultfn, sizeof(DRAWABLE_FUNCTIONS));
    if (format.bitdepth() == 32) initDrawable32(fn, format);
    drawable_functions[format] = fn;
    return fn;
}

} // namespace pplib::grafix
