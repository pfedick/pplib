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
#include <pplib/grafix/fonts.h>
// #include <pplib/grafix/imagereference.h>

namespace pplib::grafix
{
typedef uint32_t SurfaceColor;

// ACHTUNG: Bei Änderungen in der Reihenfolge muss auch die Assembler-Struktur
// in src/asm/common.asminc angepasst werden!
struct DRAWABLE_FUNCTIONS;

class DrawableData;
class ImageReference;

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

    void (*Blt)(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y);
    void (*BltDiffuse)(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, SurfaceColor c);
    void (*BltColorKey)(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, SurfaceColor c);
    void (*BltAlpha)(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y);
    void (*BltAlphaMod)(const DrawableData& target, const DrawableData& source, const Rect& srect, SurfaceColor mod, int x, int y);
    void (*BltBlend)(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, float factor);
    void (*BltChromaKey)(
        const DrawableData& target, const DrawableData& source, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y);
    void (*BltBackgoundOnChromaKey)(
        const DrawableData& target, const DrawableData& background, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y);

} DRAWABLE_FUNCTIONS;

class ImageList;
class Sprite;
class Image;

class DrawableData
{
public:
    DRAWABLE_FUNCTIONS* fn = nullptr;
    union {
        void* base = nullptr;
        uint8_t* base8;
        uint16_t* base16;
        uint32_t* base32;
    };
    uint32_t pitch = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    RGBFormat rgbformat;
};

/** @class Drawable
 * @brief Eine Klasse, die eine Zeichenfläche im Speicher repräsentiert
 *
 * Die Klasse Drawable stellt eine zweidimensonale Zeichenfläche dar, auf der verschiedene grafische
 * Operationen durchgeführt werden können. Sie bietet Funktionen zum Zeichnen von Pixeln,
 * Linien, Rechtecken, Kreisen und anderen geometrischen Formen sowie zum Kopieren und Skalieren
 * von Bildinhalten. Die Klasse unterstützt verschiedene Farbformate und ermöglicht die Arbeit
 * mit Transparenz und Farbverläufen.
 *
 * Die Klasse verwaltet selbst keinen Speicher für die Zeichenfläche, sondern
 * muss bei Initialisierung mit einem Speicherbereich, der Breite, Höhe, Pitch und
 * Farbformat angegeben werden. Verwende die Klasse Image, um eine Drawable-Instanz
 * zu erstellen, die den Speicher selbst verwaltet.
 */
class Drawable
{
    friend class Image;

private:
    // GRAFIX_FUNCTIONS* fn; => obsolete, da in DrawableData vorhanden
    DrawableData data;

    // void clearDrawableData();
    // void copyDrawableData(const DRAWABLE_DATA& other);

public:
    /** @name Konstruktoren
     */
    //@{

    /** @class Konstruktor
     *
     * Mit diesem Konstruktor wird ein leeres Drawable erstellt. Bevor es verwendet werden
     * kann, muss zunächst mit Drawable::copy eine Kopie eines anderen Drawable oder davon
     * abgeleiteten Objekts erstellt werden oder mit Drawable::create ein neues Drawable
     * anhand eines Speicherbereichs erstellt werden.
     */
    Drawable() {};

    /** @class Copy-Konstruktor
     *
     * Mit diesem Konstruktor wird eine Kopie eines anderen Drawable oder davon abgeleiteten
     * Klasse erstellt.
     *
     * @param other Ein anderes Drawable
     */
    Drawable(const Drawable& other);

    /** @class Move-Konstruktor
     *
     * Mit diesem Konstruktor wird ein Drawable von einem anderen Drawable oder davon abgeleiteten
     * Klasse übernommen.
     *
     * @param other Ein anderes Drawable
     */
    Drawable(Drawable&& other);

    /** @class Konstruktor mit Speicherbereich
     *
     * Mit diesem Konstruktor wird ein Drawable anhand eines Speicherbereichs erstellt.
     *
     * @param base Zeiger auf den Speicherbereich, der die Pixeldaten enthält
     * @param pitch Anzahl der Bytes pro Zeile (Breite des Bildes in Bytes)
     * @param width Breite des Bildes in Pixeln
     * @param height Höhe des Bildes in Pixeln
     * @param format Farbformat der Pixel (z.B. RGBFormat::R5G6B5)
     * @exception IllegalArgumentException Wird ausgelöst, wenn base null ist, pitch 0 ist oder width/height 0 sind.
     * @exception NoGrafixEngineException Wird ausgelöst, wenn die Grafikengine nicht initialisiert ist.
     */
    Drawable(void* base, uint32_t pitch, uint32_t width, uint32_t height, const RGBFormat& format);

    //@}

    /** @name Verschiedenes
     */
    //@{

    /** @brief Pointer auf die Grafik-Funktionen für das Farbformat dieses Drawable holen
     *
     * Diese Funktion liefert einen Pointer auf die Grafik-Funktionen für das Farbformat
     * dieses Drawable zurück. Dieser kann NULL sein, wenn das Drawable noch nicht
     * initialisiert wurde oder das angegebene Farbformat nicht unterstützt wird.
     *
     * @return Pointer auf eine DRAWABLE_FUNCTIONS Struktur oder NULL, wenn das Drawable
     * nicht initialisiert war.
     */
    DRAWABLE_FUNCTIONS* getFunctions() const;

    /** @brief Referenz auf die Datenstruktur des Drawable holen
     *
     * Diese Funktion liefert eine Referenz auf die Struktur DrawableData des
     * Drawable zurück. Diese enthält alle Informationen des Drawable, wie den
     * Speicherbereich der Grafik, Breite und Höhe, Bytes pro Zeile (pitch) und das
     * Farbformat.
     *
     * @return Pointer auf die DrawableData Struktur des Drawable.
     */
    const DrawableData& getData() const;

    /** @brief Grafik von einem anderen Drawable kopieren
     *
     * Mit dieser Funktion wird eine Kopie eines anderen Drawable erstellt.
     *
     * @param other Ein anderes Drawable
     */
    void copy(const Drawable& other);

    /** @brief Drawable anhand eines Speicherbereichs erstellen
     *
     * Mit dieser Funktion wird ein Drawable anhand eines Speicherbereichs erstellt.
     *
     * @param base Zeiger auf den Speicherbereich, der die Pixeldaten enthält
     * @param pitch Anzahl der Bytes pro Zeile (Breite des Bildes in Bytes)
     * @param width Breite des Bildes in Pixeln
     * @param height Höhe des Bildes in Pixeln
     * @param format Farbformat der Pixel (z.B. RGBFormat::R5G6B5)
     * @exception IllegalArgumentException Wird ausgelöst, wenn base null ist, pitch 0 ist oder width/height 0 sind.
     * @exception NoGrafixEngineException Wird ausgelöst, wenn die Grafikengine nicht initialisiert ist.
     */
    void create(void* base, uint32_t pitch, uint32_t width, uint32_t height, const RGBFormat& format);

    /**
     * @brief  Zuweisungsoperator für Drawable
     *
     * @param other Ein anderes Drawable, das kopiert werden soll
     * @return Referenz auf das aktuelle Drawable-Objekt nach der Zuweisung
     */
    Drawable& operator=(const Drawable& other) noexcept;

    /**
     * @brief Move-Zuweisungsoperator für Drawable
     *
     * @param other Ein anderes Drawable, dessen Ressourcen übernommen werden sollen
     * @return Referenz auf das aktuelle Drawable-Objekt nach der Zuweisung
     */
    Drawable& operator=(Drawable&& other) noexcept;

    /** @brief Rechteck des Drawable auslesen
     *
     * Dieser Funktion liefert das Rechteck des Drawable zurück. Die Koordinaten des
     * Rechtecks sind immer 0/0, Breite und Höhe sind die des Drawable.
     *
     * @return Ein Objekt von Typ Rect.
     */
    inline Rect rect() const
    {
        return Rect(0, 0, data.width, data.height);
    }

    /** @brief Rechteck des Drawable auslesen
     *
     * Dieser Funktion liefert das Rechteck des Drawable zurück. Die Koordinaten des
     * Rechtecks sind immer 0/0, Breite und Höhe sind die des Drawable.
     *
     * @return Ein Objekt von Typ Rect16, das die Koordinaten und Größe des Rechtecks in 16-Bit Ganzzahlen enthält.
     */
    inline Rect16 rect16() const
    {
        return Rect16(0, 0, data.width, data.height);
    }

    /** @brief Größe des Drawable auslesen
     *
     * Diese Funktion liefert Breite und Höhe des Drawable in einem Size-Objekt zurück.
     *
     * @return Objekt von Typ Size.
     */
    inline Size size() const
    {
        return Size(data.width, data.height);
    }

    /** @brief Größe des Drawable in 16-Bit Ganzzahlen auslesen
     *
     * Diese Funktion liefert Breite und Höhe des Drawable in einem Size16-Objekt zurück.
     *
     * @return Objekt von Typ Size16, das die Breite und Höhe in 16-Bit Ganzzahlen enthält.
     */
    inline Size16 size16() const
    {
        return Size16(data.width, data.height);
    }

    /** @brief Breite der Grafik in Pixel
     *
     * Diese Funktion liefert die Breite der Grafik in Pixel zurück.
     *
     * @return Breite in Pixel
     */
    inline uint32_t width() const
    {
        return data.width;
    }

    /** @brief Höhe der Grafik in Pixel
     *
     * Diese Funktion liefert die Höhe der Grafik in Pixel zurück.
     *
     * @return Höhe in Pixel
     */
    inline uint32_t height() const
    {
        return data.height;
    }

    /** @brief Anzahl der Bytes pro Zeile (Pitch)
     *
     * Mit dieser Funktion kann abgefragt werden, wieviele Bytes eine Grafikzeile benötigt.
     * Dies muss nicht unbedingt das Ergebnis von Breite mal BytesProPixel sein, sondern kann
     * auch mehr sein, z.B. wenn das Drawable nur einen Ausschnitt aus einer größeren Grafik
     * enthält.
     *
     * @return Pitch in Bytes
     */
    inline uint32_t pitch() const
    {
        return data.pitch;
    }

    /** @brief Farbformat des Drawable
     *
     * Diese Funktion liefert das Farbformat des Drawable zurück.
     *
     * @return Farbformat als RGBFormat-Objekt
     */
    inline RGBFormat rgbformat() const
    {
        return data.rgbformat;
    }

    /** @brief Enthält dieses Drawable eine Grafik?
     *
     * Mit dieser Funktion kann geprüft werden, ob das Drawable eine gültige und somit nutzbare
     * Grafik enthält. Dies ist der Fall, wenn eine Basisadresse vorhanden ist, die Grafik eine
     * Höhe und Breite größer 0 und ein gültiges Farbformat enthält.
     *
     * @return Liefert \c true zurück, wenn das Drawable eine gültige Grafik enthält, andernfalls
     * \c false.
     */
    inline bool isEmpty() const
    {
        return (data.base == nullptr || data.width == 0 || data.height == 0 || data.rgbformat == RGBFormat::unknown);
    }

    /**
     * @brief Speicheradresse der Grafik
     * Diese Funktion gibt die Speicheradresse zurück, an der die Daten dieser Grafik
     * beginnen.
     * @return Pointer auf den Speicherbereich der Grafikdaten
     */
    inline void* adr() const
    {
        return data.base;
    }

    /**
     * @brief Speicheradresse eines Pixels in der Grafik
     * Diese Funktion gibt die Speicheradresse zurück, an der die Daten des Pixels
     * an den Koordinaten (x,y) beginnen.
     * @param x X-Koordinate des Pixels
     * @param y Y-Koordinate des Pixels
     * @return Pointer auf den Speicherbereich des Pixels
     * @exception OutOfBoundsException Wird ausgelöst, wenn die Koordinaten außerhalb der Grafik liegen.
     */
    void* adr(uint32_t x, uint32_t y) const;

    /** @brief Ein neues Drawable anhand eines Ausschnitts erstellen
     *
     * Diese Funktion erstellt ein neues Drawable Objekt anhand des angegebenen Ausschnitts \p rect.
     *
     * @param rect Der gewünschte Bildausschnitt
     * @return Neues Drawable, das den gewünschten Ausschnitt repräsentiert. Kann leer
     * sein, wenn der Ausschnitt außerhalb des Drawable liegt.
     */
    Drawable getDrawable(const Rect& rect) const;

    /** @brief Ein neues Drawable anhand eines Ausschnitts erstellen
     *
     * Diese Funktion erstellt ein neues Drawable Objekt anhand des angegebenen Ausschnitts \p rect.
     *
     * @param rect Der gewünschte Bildausschnitt
     * @return Neues Drawable, das den gewünschten Ausschnitt repräsentiert. Kann leer
     * sein, wenn der Ausschnitt außerhalb des Drawable liegt.
     */
    Drawable getDrawable(const Rect16& rect) const;

    /** @brief Ein neues Drawable anhand eines Ausschnitts erstellen
     *
     * Diese Funktion erstellt ein neues Drawable Objekt anhand des angegebenen Ausschnitts \p rect.
     *
     * @param p Punkt, der die obere linke Ecke des Ausschnitts definiert
     * @param s Größe des Ausschnitts, die die Breite und Höhe angibt
     * @return Neues Drawable, das den gewünschten Ausschnitt repräsentiert. Kann leer
     * sein, wenn der Ausschnitt außerhalb des Drawable liegt.
     */
    inline Drawable getDrawable(const Point& p, const Size& s) const
    {
        return getDrawable(Rect(p, s));
    }

    /** @brief Ein neues Drawable anhand eines Ausschnitts erstellen
     *
     * Diese Funktion erstellt ein neues Drawable Objekt anhand des angegebenen Koordinaten. Dabei
     * ist zu beachten, dass der Ausschnitt inklusive der rechten unteren Ecke erstellt wird.
     *
     * @param x1 X-Koordinate der linken oberen Ecke
     * @param y1 Y-Koordinate der linken oberen Ecke
     * @param x2 X-Koordinate der rechten unteren Ecke
     * @param y2 Y-Koordinate der rechten unteren Ecke
     * @return Neues Drawable, das den gewünschten Ausschnitt repräsentiert. Kann leer
     * sein, wenn der Ausschnitt außerhalb des Drawable liegt.
     */
    inline Drawable getDrawable(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2) const
    {
        return getDrawable(Rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1));
    }

    /** @brief Inhalt des Drawable in ein neues Image skalieren
     *
     * Mit dieser Funktion wird die Grafik in ein neues Image skaliert. Dabei kann
     * die Größe der Grafik angegeben werden. Wenn \p keepAspectRation auf \c true gesetzt ist,
     * wird das Seitenverhältnis der Grafik beibehalten, andernfalls wird die Grafik auf die
     * angegebene Größe skaliert.
     *
     * @param width Neue Breite der Grafik in Pixel
     * @param height Neue Höhe der Grafik in Pixel
     * @param keepAspectRation Beibehalten des Seitenverhältnisses?
     * @param smoothTransform Verwenden von bilinearer Skalierung?
     *
     * @note Wenn \p smoothTransform auf \c true gesetzt ist, wird die Grafik mit
     * bilinearer Skalierung skaliert, andernfalls wird die Grafik mit der
     * nächstgelegenen Pixelmethode skaliert. Bei bilinearer Skalierung kann
     * es zu leichten Unschärfen kommen, da die Pixel interpoliert werden.
     * Bei der nächstgelegenen Pixelmethode wird jeder Pixel auf den nächstgelegenen
     * Pixel der Originalgrafik gesetzt, was zu einem schärferen, aber auch
     * kantigeren Ergebnis führt.
     */
    Image scaled(uint32_t width, uint32_t height, bool keepAspectRation = true, bool smoothTransform = false) const;

    //@}

    /** @name Farben
     */
    //@{
    inline SurfaceColor toNativeColor(const Color& c) const
    {
        return data.fn->ToNativeColor(c);
    }

    inline Color fromNativeColor(SurfaceColor c) const
    {
        return data.fn->FromNativeColor(c);
    }

    inline void cls(const Color& c)
    {
        data.fn->Clear(data, data.fn->ToNativeColor(c));
    }

    inline void cls()
    {
        data.fn->Clear(data, 0);
    }

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

    inline void alphaPixel(int x, int y, const Color& c)
    {
        data.fn->AlphaPixel(data, x, y, data.fn->ToNativeColor(c));
    }

    inline void alphaPixel(const Point& p, const Color& c)
    {
        data.fn->AlphaPixel(data, p.x, p.y, data.fn->ToNativeColor(c));
    }

    inline void blendPixel(int x, int y, const Color& c, float brightness)
    {
        data.fn->BlendPixel(data, x, y, data.fn->ToNativeColor(c), (int)(brightness * 255.0f));
    }

    inline void blendPixel(int x, int y, const Color& c, int brightness)
    {
        data.fn->BlendPixel(data, x, y, data.fn->ToNativeColor(c), brightness);
    }

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
