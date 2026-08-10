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

#ifndef PPLIB_INCLUDE_GRAFIX_FONT_H
#define PPLIB_INCLUDE_GRAFIX_FONT_H

#include <stdint.h>

#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/bytearray.h>
#include <pplib/core/fileobject.h>
#include <pplib/core/baseexception.h>
#include <pplib/grafix/color.h>
#include <pplib/grafix/size.h>
#include <pplib/grafix/rect.h>

namespace pplib
{
class PFPChunk;
}
namespace pplib::grafix
{

PPLIBEXCEPTION(InvalidFontException, Exception);
PPLIBEXCEPTION(NoSuitableFontEngineException, Exception);

PPLIBEXCEPTION(FontEngineInitializationException, Exception);
PPLIBEXCEPTION(FontEngineUninitializedException, Exception);

PPLIBEXCEPTION(FontNotFoundException, Exception);
PPLIBEXCEPTION(InvalidFontEngineException, Exception);

// Font6 Exceptions
PPLIBEXCEPTION(InvalidFontFormatException, Exception);
PPLIBEXCEPTION(InvalidFontFaceException, Exception);
PPLIBEXCEPTION(UnknownFontFaceException, Exception);

class Drawable;
class FontEngine;

class Font
{
    friend bool operator!=(const Font& f1, const Font& f2);
    friend bool operator==(const Font& f1, const Font& f2);

private:
    enum FontFlags
    {
        fBold = 1,
        fItalic = 2,
        fAntialias = 4,
        fDrawBorder = 8,
        fDrawShadow = 16,
        fUnderline = 32,
        fMonospace = 64
    };

    String Name;
    Color cForeground;
    Color cBorder;
    Color cShadow;
    uint16_t fontSize;
    uint8_t flags;
    uint8_t ori;
    double rotationDegrees;

public:
    enum Orientation
    {
        LEFT = 1,
        MIDDLE,
        RIGHT,
        TOP,
        BOTTOM,
        BASE
    };

    Font();
    Font(const Font& other);

    const String& name() const;
    Color color() const;
    Color borderColor() const;
    Color shadowColor() const;
    bool bold() const;
    bool italic() const;
    bool antialias() const;
    bool drawBorder() const;
    bool drawShadow() const;
    bool drawUnderline() const;
    bool monospace() const;
    int size() const;
    Orientation orientation() const;
    Size measure(const WideString& text) const;
    Rect boundary(const WideString& text, int x = 0, int y = 0) const;
    double rotation() const;

    int setName(const String& name);
    void setColor(const Color& c);
    void setBorderColor(const Color& c);
    void setShadowColor(const Color& c);
    void setColors(const Color& Foreground, const Color& Border, const Color& Shadow);
    void setBold(bool enable);
    void setItalic(bool enable);
    void setAntialias(bool enable);
    void setDrawBorder(bool enable);
    void setDrawShadow(bool enable);
    void setDrawUnderline(bool enable);
    void setMonospace(bool enable);
    void setSize(int size);
    void setOrientation(Orientation o);
    void setRotation(double degrees);

    Font& operator=(const Font& other);
};
bool operator!=(const Font& f1, const Font& f2);
bool operator==(const Font& f1, const Font& f2);

class FontFile
{
    friend class Grafix;

private:
public:
    String Name;
    ByteArray Memory;
    FontEngine* engine;
    void* priv;

    FontFile();
    ~FontFile();
};

class FontEngine
{
    friend class Grafix;

private:
public:
    FontEngine();
    virtual ~FontEngine();

    virtual bool ident(FileObject& file) noexcept;
    virtual FontFile* loadFont(FileObject& file, const String& fontname);
    virtual void deleteFont(FontFile* file);
    virtual void render(const FontFile& file, const Font& font, Drawable& draw, int x, int y, const WideString& text, const Color& color);
    virtual Size measure(const FontFile& file, const Font& font, const WideString& text);
    virtual Rect boundary(const FontFile& file, const Font& font, const WideString& text, int x, int y);
    virtual String name() const;
    virtual String description() const;
};

class FontEngineFont5 : public FontEngine
{
private:
    PFPChunk* selectFont(const FontFile& file, const Font& font);
    void renderInternal(PFPChunk* c, const Font& font, Drawable& draw, int x, int y, const WideString& text, const Color& color);

public:
    FontEngineFont5();
    ~FontEngineFont5();
    bool ident(FileObject& file) noexcept override;
    FontFile* loadFont(FileObject& file, const String& fontname) override;
    void deleteFont(FontFile* file) override;
    void render(const FontFile& file, const Font& font, Drawable& draw, int x, int y, const WideString& text, const Color& color) override;
    Size measure(const FontFile& file, const Font& font, const WideString& text) override;
    String name() const override;
    String description() const override;
};

class FontEngineFont6 : public FontEngine
{
private:
public:
    FontEngineFont6();
    ~FontEngineFont6();
    bool ident(FileObject& file) noexcept override;
    FontFile* loadFont(FileObject& file, const String& fontname) override;
    void deleteFont(FontFile* file) override;
    void render(const FontFile& file, const Font& font, Drawable& draw, int x, int y, const WideString& text, const Color& color) override;
    Size measure(const FontFile& file, const Font& font, const WideString& text) override;
    Rect boundary(const FontFile& file, const Font& font, const WideString& text, int x, int y) override;
    String name() const override;
    String description() const override;
};

class FontEngineFreeType : public FontEngine
{
private:
    void* ft;

public:
    FontEngineFreeType();
    ~FontEngineFreeType();
    bool ident(FileObject& file) noexcept override;
    FontFile* loadFont(FileObject& file, const String& fontname) override;
    void deleteFont(FontFile* file) override;
    void render(const FontFile& file, const Font& font, Drawable& draw, int x, int y, const WideString& text, const Color& color) override;
    Size measure(const FontFile& file, const Font& font, const WideString& text) override;
    String name() const override;
    String description() const override;
};

} // namespace pplib::grafix

#endif // PPLIB_INCLUDE_GRAFIX_FONT_H