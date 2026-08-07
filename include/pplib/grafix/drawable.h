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

#ifndef PPLIB_INCLUDE_GRAFIX_DRAWABLE_H
#define PPLIB_INCLUDE_GRAFIX_DRAWABLE_H

#include <stdint.h>

namespace pplib::grafix
{
typedef uint32_t SurfaceColor;

// ACHTUNG: Bei Änderungen in der Reihenfolge muss auch die Assembler-Struktur
// in src/asm/common.asminc angepasst werden!
struct GRAFIX_FUNCTIONS;

typedef struct
{
    GRAFIX_FUNCTIONS* fn;
    union {
        void* base;
        uint8_t* base8;
        uint16_t* base16;
        uint32_t* base32;
    };
    uint32_t pitch;
    int width;
    int height;
    RGBFormat rgbformat;
} DRAWABLE_DATA;

typedef struct GRAFIX_FUNCTIONS
{
    void (*CLS)(DRAWABLE_DATA& data, SurfaceColor c);
    void (*PutPixel)(const DRAWABLE_DATA& data, int x, int y, SurfaceColor c);
    void (*BlendPixel)(const DRAWABLE_DATA& data, int x, int y, SurfaceColor c, int brightness);
    void (*AlphaPixel)(const DRAWABLE_DATA& data, int x, int y, SurfaceColor c);
    SurfaceColor (*GetPixel)(const DRAWABLE_DATA& data, int x, int y);

    void (*DrawRect)(DRAWABLE_DATA& data, const Rect& r, SurfaceColor c);
    void (*FillRect)(DRAWABLE_DATA& data, const Rect& r, SurfaceColor c);
    void (*Xchange)(DRAWABLE_DATA& data, const Rect& r, SurfaceColor farbe, SurfaceColor ersatzfarbe);
    void (*Invert)(DRAWABLE_DATA& data, const Rect& r, SurfaceColor farbe1, SurfaceColor farbe2);
    void (*Negativ)(DRAWABLE_DATA& data, const Rect& r);

    SurfaceColor (*GetRGB)(int r, int g, int b, int a);
    SurfaceColor (*RGBBlend)(SurfaceColor ground, SurfaceColor top, float intensity);
    SurfaceColor (*RGBBlend255)(SurfaceColor ground, SurfaceColor top, int intensity);
    Color (*Surface2RGB)(SurfaceColor color);
    void (*LineAA)(DRAWABLE_DATA& data, int x1, int y1, int x2, int y2, SurfaceColor color, int strength);
    void (*Line)(DRAWABLE_DATA& data, int x1, int y1, int x2, int y2, SurfaceColor color);

    int (*Blt)(DRAWABLE_DATA& target, const DRAWABLE_DATA& source, const Rect& srect, int x, int y);
    int (*BltDiffuse)(DRAWABLE_DATA& target, const DRAWABLE_DATA& source, const Rect& srect, int x, int y, SurfaceColor c);
    int (*BltColorKey)(DRAWABLE_DATA& target, const DRAWABLE_DATA& source, const Rect& srect, int x, int y, SurfaceColor c);
    int (*BltAlpha)(DRAWABLE_DATA& target, const DRAWABLE_DATA& source, const Rect& srect, int x, int y);
    int (*BltAlphaMod)(DRAWABLE_DATA& target, const DRAWABLE_DATA& source, const Rect& srect, SurfaceColor mod, int x, int y);
    int (*BltBlend)(DRAWABLE_DATA& target, const DRAWABLE_DATA& source, const Rect& srect, int x, int y, float factor);
    void (*BltChromaKey)(
        DRAWABLE_DATA& target, const DRAWABLE_DATA& source, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y);
    void (*BltBackgoundOnChromaKey)(
        DRAWABLE_DATA& target, const DRAWABLE_DATA& background, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y);

} GRAFIX_FUNCTIONS;

class ImageList;
class Sprite;
class Image;

class Drawable
{
    friend class Image;

private:
    GRAFIX_FUNCTIONS* fn;
    DRAWABLE_DATA data;
    void initFunctions(const RGBFormat& format);
    void clearDrawableData();
    void copyDrawableData(const DRAWABLE_DATA& other);

public:
    /** @name Konstruktoren
     */
    //@{
    Drawable();
    Drawable(const Drawable& other);
    Drawable(const Drawable& other, const Rect& rect);
    Drawable(const Drawable& other, const Point& p, const Size& s);
    Drawable(void* base, uint32_t pitch, int width, int height, const RGBFormat& format);
    ~Drawable();
    //@}

    /** @name Verschiedenes
     */
    //@{
    GRAFIX_FUNCTIONS* getFunctions();
    DRAWABLE_DATA* getData();

    void copy(const Drawable& other);
    void copy(const Drawable& other, const Rect& rect);
    void copy(const Drawable& other, const Point& p, const Size& s);
    void create(void* base, uint32_t pitch, int width, int height, const RGBFormat& format);
    Drawable& operator=(const Drawable& other);

    Rect rect() const;
    Size size() const;
    int width() const;
    int height() const;
    int pitch() const;
    int bytesPerPixel() const;
    int bitdepth() const;
    RGBFormat rgbformat() const;
    bool isEmpty() const;
    bool isNull() const;
    void* adr() const;
    void* adr(int x, int y) const;
    void cls(const Color& c);
    void cls();
    Drawable getDrawable() const;
    Drawable getDrawable(const Rect& rect) const;
    Drawable getDrawable(const Point& p, const Size& s) const;
    Drawable getDrawable(int x1, int y1, int x2, int y2) const;
    Image scaled(int width, int height, bool keepAspectRation = true, bool smoothTransform = false) const;
    void scale(Image& tgt, int width, int height, bool keepAspectRation = true, bool smoothTransform = false) const;
    //@}

    /** @name Farben
     */
    //@{
    SurfaceColor rgb(const Color& c) const;
    SurfaceColor rgb(int r, int g, int b, int alpha) const;
    //@}

    /** @name Pixel
     */
    //@{
    void putPixel(int x, int y, const Color& c);
    void putPixel(const Point& p, const Color& c);
    void alphaPixel(int x, int y, const Color& c);
    void alphaPixel(const Point& p, const Color& c);
    void blendPixel(int x, int y, const Color& c, float brightness);
    void blendPixel(int x, int y, const Color& c, int brightness);
    Color getPixel(int x, int y) const;
    Color getPixel(const Point& p) const;
    //@}

    /** @name Geometrische Formen
     */
    //@{
    void drawRect(const Rect& rect, const Color& c);
    void drawRect(int x1, int y1, int x2, int y2, const Color& c);
    void fillRect(const Rect& rect, const Color& c);
    void fillRect(int x1, int y1, int x2, int y2, const Color& c);
    void floodFill(int x, int y, const Color& color, const Color& border);
    void elipse(int x, int y, int radx, int rady, const Color& c, bool fill = false);
    void elipse(int x, int y, int radx, int rady, const Color& c, bool fill, const Color& fillcolor, int startangle, int endangle);
    void circle(int x, int y, int rad, const Color& c, bool fill = false);
    void circle(const Point& p, int rad, const Color& c, bool fill = false);
    //@}

    /** @name Effekte
     */
    //@{
    void xchange(const Rect& rect, const Color& color, const Color& replace);
    void invert(const Rect& rect, const Color& color1, const Color& color2);
    void negativ(const Rect& rect);
    void colorGradient(const Rect& rect, const Color& c1, const Color& c2, int direction);
    void colorGradient(int x1, int y1, int x2, int y2, const Color& c1, const Color& c2, int direction);
    //@}

    /** @name Linien zeichnen
     */
    //@{
    void line(int x1, int y1, int x2, int y2, const Color& c);
    void line(const Point& start, const Point& end, const Color& c);
    void lineAA(int x1, int y1, int x2, int y2, const Color& c, int strength = 1);
    void lineAA(const Point& start, const Point& end, const Color& c, int strength = 1);
    //@}

    /** @name Textausgabe
     */
    //@{
    void print(const Font& font, int x, int y, const String& text);
    void print(const Font& font, int x, int y, const WideString& text);
    void printf(const Font& font, int x, int y, const char* fmt, ...);
    //@}

    /** @name Blit-Funktionen
     * Kopieren von Grafiken mit verschiedenen Methoden
     */
    //@{
    int fitRect(int& x, int& y, Rect& r);
    void blt(const Drawable& source, int x = 0, int y = 0);
    void blt(const Drawable& source, const Rect& srect, int x = 0, int y = 0);
    void bltDiffuse(const Drawable& source, int x = 0, int y = 0, const Color& c = Color());
    void bltDiffuse(const Drawable& source, const Rect& srect, int x = 0, int y = 0, const Color& c = Color());
    void bltColorKey(const Drawable& source, int x = 0, int y = 0, const Color& c = Color());
    void bltColorKey(const Drawable& source, const Rect& srect, int x = 0, int y = 0, const Color& c = Color());
    void bltAlpha(const Drawable& source, int x = 0, int y = 0);
    void bltAlpha(const Drawable& source, const Rect& srect, int x = 0, int y = 0);
    void bltAlphaMod(const Drawable& source, const Color& mod, int x = 0, int y = 0);
    void bltAlphaMod(const Drawable& source, const Rect& srect, const Color& mod, int x = 0, int y = 0);
    void bltBlend(const Drawable& source, float factor, int x = 0, int y = 0);
    void bltBlend(const Drawable& source, float factor, const Rect& srect, int x = 0, int y = 0);
    void bltChromaKey(const Drawable& source, const Color& key, int tol1, int tol2, int x = 0, int y = 0);
    void bltChromaKey(const Drawable& source, const Rect& srect, const Color& key, int tol1, int tol2, int x = 0, int y = 0);
    void bltBackgroundOnChromaKey(const Drawable& background, const Color& key, int tol1, int tol2, int x = 0, int y = 0);
    void bltBackgroundOnChromaKey(
        const Drawable& background, const Rect& srect, const Color& key, int tol1, int tol2, int x = 0, int y = 0);

    void draw(const ImageList& iml, int nr, int x, int y);
    void draw(const ImageList& iml, int nr, int x, int y, const Color& diffuse);
    void draw(const Sprite& sprite, int nr, int x, int y);
    //@}
};

} // namespace pplib::grafix
#endif // PPLIB_INCLUDE_GRAFIX_DRAWABLE_H
