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
#include <pplib/grafix/rgbformat.h>
#include <pplib/grafix/color.h>
#include <pplib/grafix/rect.h>
#include <pplib/grafix/font.h>
#include <pplib/grafix/imagereference.h>

namespace pplib::grafix
{
typedef uint32_t SurfaceColor;

// ACHTUNG: Bei Änderungen in der Reihenfolge muss auch die Assembler-Struktur
// in src/asm/common.asminc angepasst werden!
struct DRAWABLE_FUNCTIONS;

class DrawableData;

typedef struct DRAWABLE_FUNCTIONS
{
    void (*Clear)(DrawableData& data, SurfaceColor c);
    void (*PutPixel)(const DrawableData& data, int x, int y, SurfaceColor c);
    void (*BlendPixel)(const DrawableData& data, int x, int y, SurfaceColor c, int brightness);
    void (*AlphaPixel)(const DrawableData& data, int x, int y, SurfaceColor c);
    SurfaceColor (*GetPixel)(const DrawableData& data, int x, int y);

    void (*DrawRect)(const DrawableData& data, const Rect& r, SurfaceColor c);
    void (*FillRect)(const DrawableData& data, const Rect& r, SurfaceColor c);
    void (*Xchange)(const DrawableData& data, const Rect& r, SurfaceColor farbe, SurfaceColor ersatzfarbe);
    void (*Invert)(const DrawableData& data, const Rect& r, SurfaceColor farbe1, SurfaceColor farbe2);
    void (*Negativ)(const DrawableData& data, const Rect& r);

    SurfaceColor (*ToNativeColor)(const Color& c);
    Color (*FromNativeColor)(const SurfaceColor c);

    SurfaceColor (*RGBBlend)(SurfaceColor ground, SurfaceColor top, float intensity);
    SurfaceColor (*RGBBlend255)(SurfaceColor ground, SurfaceColor top, int intensity);

    void (*LineAA)(const DrawableData& data, int x1, int y1, int x2, int y2, SurfaceColor color, int strength);
    void (*Line)(const DrawableData& data, int x1, int y1, int x2, int y2, SurfaceColor color);

    void (*Blt)(const DrawableData& target, const const DrawableData& source, const Rect& srect, int x, int y);
    void (*BltDiffuse)(const DrawableData& target, const const DrawableData& source, const Rect& srect, int x, int y, SurfaceColor c);
    void (*BltColorKey)(const DrawableData& target, const const DrawableData& source, const Rect& srect, int x, int y, SurfaceColor c);
    void (*BltAlpha)(const DrawableData& target, const const DrawableData& source, const Rect& srect, int x, int y);
    void (*BltAlphaMod)(const DrawableData& target, const const DrawableData& source, const Rect& srect, SurfaceColor mod, int x, int y);
    void (*BltBlend)(const DrawableData& target, const const DrawableData& source, const Rect& srect, int x, int y, float factor);
    void (*BltChromaKey)(const DrawableData& target,
                         const const DrawableData& source,
                         const Rect& srect,
                         const Color& key,
                         int tol1,
                         int tol2,
                         int x,
                         int y);
    void (*BltBackgoundOnChromaKey)(const DrawableData& target,
                                    const const DrawableData& background,
                                    const Rect& srect,
                                    const Color& key,
                                    int tol1,
                                    int tol2,
                                    int x,
                                    int y);

} DRAWABLE_FUNCTIONS;

class ImageList;
class Sprite;
class Image;

class DrawableData
{
public:
    DRAWABLE_FUNCTIONS* fn;
    union {
        void* base;
        uint8_t* base8;
        uint16_t* base16;
        uint32_t* base32;
    };
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    RGBFormat rgbformat;
};

class Drawable
{
    friend class Image;

private:
    // GRAFIX_FUNCTIONS* fn; => obsolete, da in DrawableData vorhanden
    DrawableData data;
    void initFunctions(const RGBFormat& format);
    // void clearDrawableData();
    // void copyDrawableData(const DRAWABLE_DATA& other);

public:
    /** @name Konstruktoren
     */
    //@{
    Drawable();
    Drawable(const Drawable& other);
    Drawable(Drawable&& other);
    Drawable(void* base, uint32_t pitch, uint32_t width, uint32_t height, const RGBFormat& format);
    ~Drawable();
    //@}

    /** @name Verschiedenes
     */
    //@{
    DRAWABLE_FUNCTIONS* getFunctions() const;
    const DrawableData* getData() const;

    void copy(const Drawable& other);
    void create(void* base, uint32_t pitch, uint32_t width, uint32_t height, const RGBFormat& format);

    Drawable& operator=(const Drawable& other);
    Drawable& operator=(Drawable&& other) noexcept;

    Rect rect() const;
    Rect16 rect16() const;
    Size size() const;
    Size16 size16() const;
    uint32_t width() const;
    uint32_t height() const;
    uint32_t pitch() const;
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
    inline void putPixel(int x, int y, const Color& c)
    {
        data.fn->PutPixel(data, x, y, data.fn->ToNativeColor(c));
    }
    inline void putPixel(const Point& p, const Color& c)
    {
        data.fn->PutPixel(data, p.x, p.y, data.fn->ToNativeColor(c));
    }

    void alphaPixel(int x, int y, const Color& c);
    void alphaPixel(const Point& p, const Color& c);
    void blendPixel(int x, int y, const Color& c, float brightness);
    void blendPixel(int x, int y, const Color& c, int brightness);
    inline Color getPixel(int x, int y) const
    {
        return data.fn->FromNativeColor(data.fn->GetPixel(data, x, y));
    }

    inline Color getPixel(const Point& p) const
    {
        return data.fn->FromNativeColor(data.fn->GetPixel(data, p.x, p.y));
    }
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
    void draw(const ImageReference& imgref, int x, int y);
    void drawBlend(const ImageReference& imgref, int x, int y, float factor);

    void draw(const Sprite& sprite, int nr, int x, int y);
    //@}
};

} // namespace pplib::grafix
#endif // PPLIB_INCLUDE_GRAFIX_DRAWABLE_H
