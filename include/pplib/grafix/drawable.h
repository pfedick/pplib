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
    ///@name Minimal-Implementierung, die für jedes Drawable implementiert werden muss
    //@{

    /** @brief Schreibt einen Pixel an die angegebene Position
     *
     * Diese Funktion schreibt einen Pixel an die angegebene Position (x, y) in das Drawable.
     * Die Farbe des Pixels wird durch den Parameter c angegeben. Die Implementierung dieser
     * Funktion hängt vom spezifischen Drawable-Typ ab und muss entsprechend angepasst werden.
     * Ferner prüft die Funktion, ob die angegebenen Koordinaten innerhalb der Grenzen des Drawables liegen.
     * @param[in] data Die Daten des Drawables
     * @param[in] x X-Koordinate des Pixels
     * @param[in] y Y-Koordinate des Pixels
     * @param[in] c Die Farbe des Pixels
     */
    void (*PutPixel)(const DrawableData& data, int x, int y, SurfaceColor c);
    SurfaceColor (*GetPixel)(const DrawableData& data, int x, int y);
    SurfaceColor (*ToNativeColor)(const Color& c);
    Color (*FromNativeColor)(const SurfaceColor c);

    //@}

    ///@name Erweiterte Funktionen, die optional implementiert werden können
    //@{
    void (*BlendPixel)(const DrawableData& data, int x, int y, SurfaceColor c, uint8_t intensity);
    void (*AlphaPixel)(const DrawableData& data, int x, int y, SurfaceColor c);

    void (*Clear)(const DrawableData& data, SurfaceColor c);
    void (*DrawRect)(const DrawableData& data, const Rect& r, SurfaceColor c);
    /// @brief Füllt ein Rechteck mit der angegebenen Farbe. Der Aufrufer muss sicherstellen,
    /// dass das Rechteck innerhalb der Grenzen des Drawables liegt.
    void (*FillRect)(const DrawableData& data, const Rect& r, SurfaceColor c);
    void (*Xchange)(const DrawableData& data, const Rect& r, SurfaceColor farbe, SurfaceColor ersatzfarbe);
    void (*Invert)(const DrawableData& data, const Rect& r, SurfaceColor farbe1, SurfaceColor farbe2);
    void (*Negativ)(const DrawableData& data, const Rect& r);

    SurfaceColor (*RGBBlend255)(const DrawableData& data, SurfaceColor ground, SurfaceColor top, uint8_t intensity);

    void (*Blt)(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y);
    void (*BltDiffuse)(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, SurfaceColor c);
    void (*BltColorKey)(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, SurfaceColor c);
    void (*BltAlpha)(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y);
    void (*BltAlphaMod)(const DrawableData& target, const DrawableData& source, const Rect& srect, SurfaceColor mod, int x, int y);
    void (*BltBlend)(const DrawableData& target, const DrawableData& source, const Rect& srect, int x, int y, float factor);
    void (*BltChromaKey)(
        const DrawableData& target, const DrawableData& source, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y);
    void (*BltBackgroundOnChromaKey)(
        const DrawableData& target, const DrawableData& background, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y);
    //@}

} DRAWABLE_FUNCTIONS;

class ImageList;
class Sprite;
class Image;

class DrawableData
{
public:
    DRAWABLE_FUNCTIONS* fn = nullptr;
    union {
        void* base;
        uint8_t* base8;
        uint16_t* base16;
        uint32_t* base32;
    };
    uint32_t pitch = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    RGBFormat rgbformat;

    DrawableData()
        : base(nullptr)
    {
    }
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
    // void copyDrawableData(const DrawableData& other);

public:
    /** @name Konstruktoren
     */
    //@{

    Drawable() noexcept = default;

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

    /** @brief Konvertiert eine Farbe in das native Farbformat des Drawables
     *
     * Diese Funktion konvertiert eine Farbe in das native Farbformat des Drawables.
     *
     * @param c Die zu konvertierende Farbe
     * @return Die Farbe im nativen Farbformat des Drawables
     */
    inline SurfaceColor toNativeColor(const Color& c) const
    {
        return data.fn->ToNativeColor(c);
    }

    /** @brief Konvertiert eine Farbe aus dem nativen Farbformat des Drawables
     *
     * Diese Funktion konvertiert eine Farbe aus dem nativen Farbformat des Drawables
     * in das Color-Objekt.
     *
     * @param c Die zu konvertierende Farbe im nativen Farbformat des Drawables
     * @return Die Farbe als Color-Objekt
     */
    inline Color fromNativeColor(SurfaceColor c) const
    {
        return data.fn->FromNativeColor(c);
    }

    /** @brief Löscht die Grafik und füllt sie mit der angegebenen Farbe
     *
     * Diese Funktion löscht die Grafik und füllt sie mit der angegebenen Farbe.
     *
     * @param c Die Farbe, mit der die Grafik gefüllt werden soll
     */
    inline void cls(const Color& c)
    {
        data.fn->Clear(data, data.fn->ToNativeColor(c));
    }

    /** @brief Löscht die Grafik und füllt sie mit der Farbe 0
     *
     * Diese Funktion löscht die Grafik und füllt sie mit der Farbe 0.
     */
    inline void cls()
    {
        data.fn->Clear(data, 0);
    }

    //@}

    /** @name Pixel
     */
    //@{

    /** @brief Setzt einen Pixel an die angegebene Position
     *
     * Diese Funktion setzt einen Pixel an die angegebene Position (x, y) in das Drawable.
     * Die Farbe des Pixels wird durch den Parameter c angegeben. Die Funktion überprüft, ob
     * die angegebenen Koordinaten innerhalb der Grenzen des Drawables liegen.
     *
     * @param x X-Koordinate des Pixels
     * @param y Y-Koordinate des Pixels
     * @param c Die Farbe des Pixels
     */
    inline void putPixel(int x, int y, const Color& c)
    {
        data.fn->PutPixel(data, x, y, data.fn->ToNativeColor(c));
    }

    /** @brief Setzt einen Pixel an die angegebene Position
     *
     * Diese Funktion setzt einen Pixel an die angegebene Position (p.x, p.y) in das Drawable.
     * Die Farbe des Pixels wird durch den Parameter c angegeben. Die Funktion überprüft, ob
     * die angegebenen Koordinaten innerhalb der Grenzen des Drawables liegen.
     *
     * @param p Punkt, der die Position des Pixels angibt
     * @param c Die Farbe des Pixels
     */
    inline void putPixel(const Point& p, const Color& c)
    {
        data.fn->PutPixel(data, p.x, p.y, data.fn->ToNativeColor(c));
    }

    /** @brief Setzt einen Pixel an die angegebene Position mit Transparenz
     *
     * Diese Funktion setzt einen Pixel an die angegebene Position (x, y) in das Drawable.
     * Dabei wird der Alpha-Kanal der Farbe berücksichtigt, um die Transparenz zu bestimmen.
     * Die Funktion überprüft, ob die angegebenen Koordinaten innerhalb der Grenzen des Drawables liegen.
     *
     * @param x X-Koordinate des Pixels
     * @param y Y-Koordinate des Pixels
     * @param c Die Farbe des Pixels
     */
    inline void alphaPixel(int x, int y, const Color& c)
    {
        data.fn->AlphaPixel(data, x, y, data.fn->ToNativeColor(c));
    }

    /** @brief Setzt einen Pixel an die angegebene Position mit Transparenz
     *
     * Diese Funktion setzt einen Pixel an die angegebene Position (p.x, p.y) in das Drawable.
     * Dabei wird der Alpha-Kanal der Farbe berücksichtigt, um die Transparenz zu bestimmen.
     * Die Funktion überprüft, ob die angegebenen Koordinaten innerhalb der Grenzen des Drawables liegen.
     *
     * @param p Punkt, der die Position des Pixels angibt
     * @param c Die Farbe des Pixels
     */
    inline void alphaPixel(const Point& p, const Color& c)
    {
        data.fn->AlphaPixel(data, p.x, p.y, data.fn->ToNativeColor(c));
    }

    /** @brief Setzt einen Pixel an die angegebene Position mit Intensität
     *
     * Diese Funktion setzt einen Pixel an die angegebene Position (x, y) in das Drawable.
     * Dabei wird die Helligkeit der Farbe berücksichtigt, um die Intensität des Pixels zu bestimmen.
     * Die Funktion überprüft, ob die angegebenen Koordinaten innerhalb der Grenzen des Drawables liegen.
     *
     * @param x X-Koordinate des Pixels
     * @param y Y-Koordinate des Pixels
     * @param c Die Farbe des Pixels
     * @param intensity Helligkeit des Pixels als float-Wert zwischen 0.0 und 1.0
     */
    inline void blendPixel(int x, int y, const Color& c, float intensity)
    {
        data.fn->BlendPixel(data, x, y, data.fn->ToNativeColor(c), (int)(intensity * 255.0f));
    }

    /** @brief Setzt einen Pixel an die angegebene Position mit Intensität
     *
     * Diese Funktion setzt einen Pixel an die angegebene Position (x, y) in das Drawable.
     * Dabei wird die Helligkeit der Farbe berücksichtigt, um die Intensität des Pixels zu bestimmen.
     * Die Funktion überprüft, ob die angegebenen Koordinaten innerhalb der Grenzen des Drawables liegen.
     *
     * @param x X-Koordinate des Pixels
     * @param y Y-Koordinate des Pixels
     * @param c Die Farbe des Pixels
     * @param intensity Helligkeit des Pixels als Integer-Wert zwischen 0 und 255
     */
    inline void blendPixel(int x, int y, const Color& c, uint8_t intensity)
    {
        data.fn->BlendPixel(data, x, y, data.fn->ToNativeColor(c), intensity);
    }

    /** @brief Liest die Farbe eines Pixels an der angegebenen Position aus
     *
     * Diese Funktion liest die Farbe eines Pixels an der angegebenen Position (x, y) im Drawable aus.
     * Die Funktion überprüft, ob die angegebenen Koordinaten innerhalb der Grenzen des Drawables liegen.
     *
     * @param x X-Koordinate des Pixels
     * @param y Y-Koordinate des Pixels
     * @return Die Farbe des Pixels als Color-Objekt
     */
    inline Color getPixel(int x, int y) const
    {
        return data.fn->FromNativeColor(data.fn->GetPixel(data, x, y));
    }

    /** @brief Liest die Farbe eines Pixels an der angegebenen Position aus
     *
     * Diese Funktion liest die Farbe eines Pixels an der angegebenen Position (p.x, p.y) im Drawable aus.
     * Die Funktion überprüft, ob die angegebenen Koordinaten innerhalb der Grenzen des Drawables liegen.
     *
     * @param p Punkt, der die Position des Pixels angibt
     * @return Die Farbe des Pixels als Color-Objekt
     */
    inline Color getPixel(const Point& p) const
    {
        return data.fn->FromNativeColor(data.fn->GetPixel(data, p.x, p.y));
    }
    //@}

    /** @name Geometrische Formen
     */
    //@{

    /** @brief Zeichnet ein Rechteck auf das Drawable
     *
     * Diese Funktion zeichnet ein Rechteck auf das Drawable. Die Position und Größe des Rechtecks
     * wird durch das Rect-Objekt angegeben. Die Farbe des Rechtecks wird durch den Parameter c angegeben.
     *
     * @param rect Das Rechteck, das gezeichnet werden soll
     * @param c Die Farbe des Rechtecks
     */
    inline void drawRect(const Rect& rect, const Color& c)
    {
        data.fn->DrawRect(data, rect, data.fn->ToNativeColor(c));
    }

    /** @brief Zeichnet ein Rechteck auf das Drawable
     *
     * Diese Funktion zeichnet ein Rechteck auf das Drawable. Die Position und Größe des Rechtecks
     * wird durch die angegebenen Koordinaten (x1, y1) und (x2, y2) definiert. Die Farbe des Rechtecks
     * wird durch den Parameter c angegeben.
     *
     * @param x1 X-Koordinate der linken oberen Ecke des Rechtecks
     * @param y1 Y-Koordinate der linken oberen Ecke des Rechtecks
     * @param x2 X-Koordinate der rechten unteren Ecke des Rechtecks
     * @param y2 Y-Koordinate der rechten unteren Ecke des Rechtecks
     * @param c Die Farbe des Rechtecks
     *
     * @note die Koordinaten (x2, y2) sind inklusive, d.h. das Rechteck wird bis zu diesen Koordinaten gezeichnet.
     */
    inline void drawRect(int x1, int y1, int x2, int y2, const Color& c)
    {
        data.fn->DrawRect(data, Rect::fromCoordsInclusive(x1, y1, x2, y2), data.fn->ToNativeColor(c));
    }

    /** @brief Füllt ein Rechteck auf das Drawable
     *
     * Diese Funktion füllt ein Rechteck auf das Drawable. Die Position und Größe des Rechtecks
     * wird durch das Rect-Objekt angegeben. Die Farbe des Rechtecks wird durch den Parameter c angegeben.
     *
     * @param rect Das Rechteck, das gefüllt werden soll
     * @param c Die Farbe des Rechtecks
     */
    inline void fillRect(const Rect& rect, const Color& c)
    {
        data.fn->FillRect(data, rect, data.fn->ToNativeColor(c));
    }

    /** @brief Füllt ein Rechteck auf das Drawable
     *
     * Diese Funktion füllt ein Rechteck auf das Drawable. Die Position und Größe des Rechtecks
     * wird durch die angegebenen Koordinaten (x1, y1) und (x2, y2) definiert. Die Farbe des Rechtecks
     * wird durch den Parameter c angegeben.
     *
     * @param x1 X-Koordinate der linken oberen Ecke des Rechtecks
     * @param y1 Y-Koordinate der linken oberen Ecke des Rechtecks
     * @param x2 X-Koordinate der rechten unteren Ecke des Rechtecks
     * @param y2 Y-Koordinate der rechten unteren Ecke des Rechtecks
     * @param c Die Farbe des Rechtecks
     *
     * @note die Koordinaten (x2, y2) sind inklusive, d.h. das Rechteck wird bis zu diesen Koordinaten gefüllt.
     */
    inline void fillRect(int x1, int y1, int x2, int y2, const Color& c)
    {
        data.fn->FillRect(data, Rect::fromCoordsInclusive(x1, y1, x2, y2), data.fn->ToNativeColor(c));
    }

    /** @brief Füllt eine Fläche mit der angegebenen Farbe, bis eine Grenze erreicht wird
     *
     * Diese Funktion füllt eine Fläche mit der angegebenen Farbe, beginnend an den Koordinaten (x, y),
     * bis eine Grenze erreicht wird, die durch die Farbe \p border definiert ist. Die Funktion überprüft,
     * ob die angegebenen Koordinaten innerhalb der Grenzen des Drawables liegen.
     *
     * @param x X-Koordinate des Startpunkts
     * @param y Y-Koordinate des Startpunkts
     * @param color Die Farbe, mit der die Fläche gefüllt werden soll
     * @param border Die Farbe, die als Grenze für das Füllen dient
     */
    void floodFill(int x, int y, const Color& color, const Color& border);

    void elipse(int x, int y, int radx, int rady, const Color& c, bool fill = false);
    void elipse(int x, int y, int radx, int rady, const Color& c, bool fill, const Color& fillcolor, int startangle, int endangle);
    inline void circle(int x, int y, int rad, const Color& c, bool fill = false)
    {
        elipse(x, y, rad, rad, c, fill);
    }
    inline void circle(const Point& p, int rad, const Color& c, bool fill = false)
    {
        elipse(p.x, p.y, rad, rad, c, fill);
    }
    //@}

    /** @name Effekte
     */
    //@{
    inline void xchange(const Rect& rect, const Color& color, const Color& replace)
    {
        data.fn->Xchange(data, rect, data.fn->ToNativeColor(color), data.fn->ToNativeColor(replace));
    }

    inline void invert(const Rect& rect, const Color& color1, const Color& color2)
    {
        data.fn->Invert(data, rect, data.fn->ToNativeColor(color1), data.fn->ToNativeColor(color2));
    }

    inline void negativ(const Rect& rect)
    {
        data.fn->Negativ(data, rect);
    }

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

    /** @brief Bit-Blit, Rechteck 1:1 kopieren
     *
     * Mit dieser Funktion wird die Quellzeichenfläche \p source
     * an die Position \p x / \p y der Zielzeichenfläche kopiert, wobei alle Farbinformationen 1:1 übernommen werden.
     * Es wird weder Alphablending (siehe Drawable::bltAlpha) noch Colorkeying (siehe
     * Drawable::bltColorKey) verwendet.
     * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert (Clipping).
     * Falls die Quelle komplett außerhalb der Zeichenfläche liegt, passiert nichts.
     *
     * @param[in] source Die Quellzeichenfläche
     * @param[in] x Optionale X-Koordinate der linken oberen Ecke in der Zielzeichenfläche. Wird der Parameter nicht
     *            angegeben, wird 0 verwendet.
     * @param[in] y Optionale Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche. Wird der Parameter
     *            nicht angegebenm wird 0 verwendet.
     *
     */
    inline void blt(const Drawable& source, int x = 0, int y = 0)
    {
        data.fn->Blt(data, source.data, source.rect(), x, y);
    }

    /** @brief Bit-Blit, Rechteck 1:1 kopieren
     *
     * Mit dieser Funktion wird die Quellzeichenfläche \p source
     * an die Position \p x / \p y der Zielzeichenfläche kopiert, wobei alle Farbinformationen 1:1 übernommen werden.
     * Es wird weder Alphablending (siehe Drawable::bltAlpha) noch Colorkeying (siehe
     * Drawable::bltColorKey) verwendet.
     * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert (Clipping).
     * Falls die Quelle komplett außerhalb der Zeichenfläche liegt, passiert nichts.
     *
     * @param[in] source Die Quellzeichenfläche
     * @param[in] srect Das Rechteck in der Quellzeichenfläche, das kopiert werden soll
     * @param[in] x Optionale X-Koordinate der linken oberen Ecke in der Zielzeichenfläche. Wird der Parameter nicht
     *            angegeben, wird 0 verwendet.
     * @param[in] y Optionale Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche. Wird der Parameter
     *            nicht angegebenm wird 0 verwendet.
     */
    inline void blt(const Drawable& source, const Rect& srect, int x = 0, int y = 0)
    {
        data.fn->Blt(data, source.data, srect, x, y);
    }

    /** @brief Rechteck anhand der Intensität der Quellfarbe kopieren
     *
     * Mit dieser Funktion wird die Quellzeichenfläche \p source
     * an die Position \p x / \p y kopiert, wobei die Intensität der Quellpixel geprüft wird und
     * diese in gleicher Intensität mit der angegebenen Farbe \c gezeichnet werden. Bei
     * halbtransparenten Pixeln wird die Farbe mit dem Hintergrund gemischt. Die Funktion ist daher
     * zum Zeichnen von einfarbigen Grafiken unterschiedlicher Intensität gedacht (z.B. grafische Elemente
     * einer GUI).
     *
     * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert (Clipping).
     * Falls die Quelle komplett außerhalb der Zeichenfläche liegt, passiert nichts.
     *
     * @param[in] source Die Quellzeichenfläche
     * @param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     * @param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     * @param[in] c Die gewünschte Pixelfarbe
     *
     */
    inline void bltDiffuse(const Drawable& source, int x = 0, int y = 0, const Color& c = Color())
    {
        data.fn->BltDiffuse(data, source.data, source.rect(), x, y, data.fn->ToNativeColor(c));
    }

    /** @brief Rechteck anhand der Intensität der Quellfarbe kopieren
     *
     * Mit dieser Funktion wird die Quellzeichenfläche \p source
     * an die Position \p x / \p y kopiert, wobei die Intensität der Quellpixel geprüft wird und
     * diese in gleicher Intensität mit der angegebenen Farbe \c gezeichnet werden. Bei
     * halbtransparenten Pixeln wird die Farbe mit dem Hintergrund gemischt. Die Funktion ist daher
     * zum Zeichnen von einfarbigen Grafiken unterschiedlicher Intensität gedacht (z.B. grafische Elemente
     * einer GUI).
     *
     * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert (Clipping).
     * Falls die Quelle komplett außerhalb der Zeichenfläche liegt, passiert nichts.
     *
     * @param[in] source Die Quellzeichenfläche
     * @param[in] srect Das Rechteck in der Quellzeichenfläche, das kopiert werden soll
     * @param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     * @param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     * @param[in] c Die gewünschte Pixelfarbe
     *
     */
    inline void bltDiffuse(const Drawable& source, const Rect& srect, int x = 0, int y = 0, const Color& c = Color())
    {
        data.fn->BltDiffuse(data, source.data, srect, x, y, data.fn->ToNativeColor(c));
    }

    /** @brief Rechteck anhand der Quellfarbe kopieren, wobei eine bestimmte Farbe als transparent behandelt wird
     *
     * Mit dieser Funktion wird die Quellzeichenfläche \p source
     * an die Position \p x / \p y kopiert, wobei die angegebene Farbe \c c als transparent behandelt wird.
     * Alle Pixel in der Quellzeichenfläche, die die Farbe \c c haben, werden nicht gezeichnet und der Hintergrund bleibt sichtbar.
     *
     * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert (Clipping).
     * Falls die Quelle komplett außerhalb der Zeichenfläche liegt, passiert nichts.
     *
     * @param[in] source Die Quellzeichenfläche
     * @param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     * @param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     * @param[in] c Die Farbe, die als transparent behandelt werden soll
     *
     */
    inline void bltColorKey(const Drawable& source, int x = 0, int y = 0, const Color& c = Color())
    {
        data.fn->BltColorKey(data, source.data, source.rect(), x, y, data.fn->ToNativeColor(c));
    }

    /** @brief Rechteck anhand der Quellfarbe kopieren, wobei eine bestimmte Farbe als transparent behandelt wird
     *
     * Mit dieser Funktion wird die Quellzeichenfläche \p source
     * an die Position \p x / \p y kopiert, wobei die angegebene Farbe \c c als transparent behandelt wird.
     * Alle Pixel in der Quellzeichenfläche, die die Farbe \c c haben, werden nicht gezeichnet und der Hintergrund bleibt sichtbar.
     *
     * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert (Clipping).
     * Falls die Quelle komplett außerhalb der Zeichenfläche liegt, passiert nichts.
     *
     * @param[in] source Die Quellzeichenfläche
     * @param[in] srect Das Rechteck in der Quellzeichenfläche, das kopiert werden soll
     * @param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     * @param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     * @param[in] c Die Farbe, die als transparent behandelt werden soll
     *
     */
    inline void bltColorKey(const Drawable& source, const Rect& srect, int x = 0, int y = 0, const Color& c = Color())
    {
        data.fn->BltColorKey(data, source.data, srect, x, y, data.fn->ToNativeColor(c));
    }

    /** @brief Rechteck unter Berücksichtigung des Alphakanals kopieren
     *
     * Mit dieser Funktion wird die Quellzeichenfläche \p source
     * an die Position \p x / \p y kopiert, wobei die Transparenz der Quellpixel (Alphakanal) berücksichtigt wird.
     * Der Alphakanal bestimmt die Transparenz eines Pixels. Ist sie 0, wird der Pixel nicht
     * kopiert, bei einem Wert von 255 wird er 1:1 kopiert. Dazwischen wird die Farbe abhängig
     * vom Transparenz-Wert mit dem Hintergrund vermischt.
     *
     * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert (Clipping).
     * Falls die Quelle komplett außerhalb der Zeichenfläche liegt, passiert nichts.
     *
     * @param[in] source Die Quellzeichenfläche
     * @param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     * @param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     */
    inline void bltAlpha(const Drawable& source, int x = 0, int y = 0)
    {
        data.fn->BltAlpha(data, source.data, source.rect(), x, y);
    }

    /** @brief Rechteck unter Berücksichtigung des Alphakanals kopieren
     *
     * Mit dieser Funktion wird die Quellzeichenfläche \p source
     * an die Position \p x / \p y kopiert, wobei die Transparenz der Quellpixel (Alphakanal) berücksichtigt wird.
     * Der Alphakanal bestimmt die Transparenz eines Pixels. Ist sie 0, wird der Pixel nicht
     * kopiert, bei einem Wert von 255 wird er 1:1 kopiert. Dazwischen wird die Farbe abhängig
     * vom Transparenz-Wert mit dem Hintergrund vermischt.
     *
     * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert (Clipping).
     * Falls die Quelle komplett außerhalb der Zeichenfläche liegt, passiert nichts.
     *
     * @param[in] source Die Quellzeichenfläche
     * @param[in] srect Das Rechteck in der Quellzeichenfläche, das kopiert werden soll
     * @param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     * @param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     */
    inline void bltAlpha(const Drawable& source, const Rect& srect, int x = 0, int y = 0)
    {
        data.fn->BltAlpha(data, source.data, srect, x, y);
    }

    /** @brief Rechteck unter Berücksichtigung des Alphakanals und Modulation der Farbe kopieren
     *
     * Mit dieser Funktion wird die Quellzeichenfläche \p source
     * an die Position \p x / \p y kopiert, wobei die Transparenz der Quellpixel (Alphakanal) berücksichtigt wird.
     * Zusätzlich wird die Farbe der Quellpixel mit der angegebenen Modulationsfarbe \c mod multipliziert.
     *
     * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert (Clipping).
     * Falls die Quelle komplett außerhalb der Zeichenfläche liegt, passiert nichts.
     *
     * @param[in] source Die Quellzeichenfläche
     * @param[in] mod Die Modulationsfarbe, mit der die Quellpixel multipliziert werden
     * @param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     * @param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     */
    inline void bltAlphaMod(const Drawable& source, const Color& mod, int x = 0, int y = 0)
    {
        data.fn->BltAlphaMod(data, source.data, source.rect(), data.fn->ToNativeColor(mod), x, y);
    }

    /** @brief Rechteck unter Berücksichtigung des Alphakanals und Modulation der Farbe kopieren
     *
     * Mit dieser Funktion wird die Quellzeichenfläche \p source
     * an die Position \p x / \p y kopiert, wobei die Transparenz der Quellpixel (Alphakanal) berücksichtigt wird.
     * Zusätzlich wird die Farbe der Quellpixel mit der angegebenen Modulationsfarbe \c mod multipliziert.
     *
     * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert (Clipping).
     * Falls die Quelle komplett außerhalb der Zeichenfläche liegt, passiert nichts.
     *
     * @param[in] source Die Quellzeichenfläche
     * @param[in] srect Das Rechteck in der Quellzeichenfläche, das kopiert werden soll
     * @param[in] mod Die Modulationsfarbe, mit der die Quellpixel multipliziert werden
     * @param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     * @param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
     */
    inline void bltAlphaMod(const Drawable& source, const Rect& srect, const Color& mod, int x = 0, int y = 0)
    {
        data.fn->BltAlphaMod(data, source.data, srect, data.fn->ToNativeColor(mod), x, y);
    }

    inline void bltBlend(const Drawable& source, float factor, int x = 0, int y = 0)
    {
        data.fn->BltBlend(data, source.data, source.rect(), x, y, factor);
    }

    inline void bltBlend(const Drawable& source, float factor, const Rect& srect, int x = 0, int y = 0)
    {
        data.fn->BltBlend(data, source.data, srect, x, y, factor);
    }

    /** @brief Rechteck unter Berücksichtigung eines Farbschlüssels kopieren (Bluescreen-Effekt)
     *
     * Mit dieser Funktion kann ein "Bluescreen-Effekt" erzielt werden (siehe https://de.wikipedia.org/wiki/Bluescreen-Technik#Greenscreen).
     * Dabei wird die Quellgrafik \p source mittels eines Farbschlüssels \p key (Chroma Key), sowie zwei Toleranz-Werten
     * über den Hintergrund gelegt.
     *
     * @param source Quellgrafik
     * @param key Farbschlüssel (z.B. Color(0,0,255) für einen Bluescreen oder Color(0,255,0) für
     * einen Greenscreen)
     * @param tol1 Untere Toleranz: Farbabweichungen bis zu diesem Toleranzwert, werden komplett Transparent,
     * das heisst der Hintergrund wird übernommen
     * @param tol2 Obere Toleranz: Farbabweichungen, die zwischen \p tol1 und \p tol2 liegen, werden je nach
     * Stärke der Abweichung überblendet. Je stärker die Abweichung, desto mehr Hintergrund ist zu sehen
     * @param x Zielkoordinate für das Rechteck (optional, Default ist 0)
     * @param y Zielkoordinate für das Rechteck (optional, Default ist 0)
     *
     * @remarks Auf 64-Bit-Systemen mit SSE2-Unterstützung werden optimierte Assembler-Routinen verwendet.
     * Sofern Bildbreite durch 4 und die Speicheradressen durch 16 teilbar sind, werden jeweils 4 Pixel
     * gleichzeitig berechnet.
     *
     * @see Die Funktion bltChromaKey wendet den Farbschlüssel auf das Quellbild \p source an.
     * @see Die Funktion bltBackgroundOnChromaKey wendet den Farbschlüssel nicht auf das Quellbild \p source
     * sondern den Hintergrund an.
     */
    inline void bltChromaKey(const Drawable& source, const Color& key, uint8_t tol1, uint8_t tol2, int x = 0, int y = 0)
    {
        data.fn->BltChromaKey(data, source.data, source.rect(), key, tol1, tol2, x, y);
    }

    /** @brief Rechteck unter Berücksichtigung eines Farbschlüssels kopieren (Bluescreen-Effekt)
     *
     * Mit dieser Funktion kann ein "Bluescreen-Effekt" erzielt werden (siehe https://de.wikipedia.org/wiki/Bluescreen-Technik#Greenscreen).
     * Dabei wird die Quellgrafik \p source mittels eines Farbschlüssels \p key (Chroma Key), sowie zwei Toleranz-Werten
     * über den Hintergrund gelegt.
     *
     * @param source Quellgrafik
     * @param srect Rechteck in der Quellgrafik, das kopiert werden soll
     * @param key Farbschlüssel (z.B. Color(0,0,255) für einen Bluescreen oder Color(0,255,0) für
     * einen Greenscreen)
     * @param tol1 Untere Toleranz: Farbabweichungen bis zu diesem Toleranzwert, werden komplett Transparent,
     * das heisst der Hintergrund wird übernommen
     * @param tol2 Obere Toleranz: Farbabweichungen, die zwischen \p tol1 und \p tol2 liegen, werden je nach
     * Stärke der Abweichung überblendet. Je stärker die Abweichung, desto mehr Hintergrund ist zu sehen
     * @param x Zielkoordinate für das Rechteck (optional, Default ist 0)
     * @param y Zielkoordinate für das Rechteck (optional, Default ist 0)
     *
     * @remarks Auf 64-Bit-Systemen mit SSE2-Unterstützung werden optimierte Assembler-Routinen verwendet.
     * Sofern Bildbreite durch 4 und die Speicheradressen durch 16 teilbar sind, werden jeweils 4 Pixel
     * gleichzeitig berechnet.
     *
     * @see Die Funktion bltChromaKey wendet den Farbschlüssel auf das Quellbild \p source an.
     * @see Die Funktion bltBackgroundOnChromaKey wendet den Farbschlüssel nicht auf das Quellbild \p source
     * sondern den Hintergrund an.
     */
    inline void bltChromaKey(const Drawable& source, const Rect& srect, const Color& key, uint8_t tol1, uint8_t tol2, int x = 0, int y = 0)
    {
        data.fn->BltChromaKey(data, source.data, srect, key, tol1, tol2, x, y);
    }

    /** @brief Rechteck unter Berücksichtigung eines Farbschlüssels kopieren (Bluescreen-Effekt)
     *
     * Mit dieser Funktion kann ein "Bluescreen-Effekt" erzielt werden (siehe http://de.wikipedia.org/wiki/Bluescreen-Technik#Greenscreen).
     * Dabei wird die Hintergundgrafik \p background mittels eines Farbschlüssels \p key (Chroma Key), sowie zwei Toleranz-Werten
     * über die Grafik gelegt.
     *
     * @param source Quellgrafik
     * @param key Farbschlüssel (z.B. Color(0,0,255) für einen Bluescreen oder Color(0,255,0) für
     * einen Greenscreen)
     * @param tol1 Untere Toleranz: Farbabweichungen bis zu diesem Toleranzwert, werden komplett Transparent,
     * das heisst der Hintergrund wird übernommen
     * @param tol2 Obere Toleranz: Farbabweichungen, die zwischen \p tol1 und \p tol2 liegen, werden je nach
     * Stärke der Abweichung überblendet. Je stärker die Abweichung, desto mehr Hintergrund ist zu sehen
     * @param x Zielkoordinate für das Rechteck (optional, Default ist 0)
     * @param y Zielkoordinate für das Rechteck (optional, Default ist 0)
     *
     * @remarks Auf 64-Bit-Systemen mit SSE2-Unterstützung werden optimierte Assembler-Routinen verwendet.
     * Sofern Bildbreite durch 4 und die Speicheradressen durch 16 teilbar sind, werden jeweils 4 Pixel
     * gleichzeitig berechnet.
     *
     * @see Die Funktion bltChromaKey wendet den Farbschlüssel auf das Quellbild \p source an.
     * @see Die Funktion bltBackgroundOnChromaKey wendet den Farbschlüssel nicht auf das Quellbild \p source
     * sondern den Hintergrund an.
     */
    inline void bltBackgroundOnChromaKey(const Drawable& background, const Color& key, uint8_t tol1, uint8_t tol2, int x = 0, int y = 0)
    {
        data.fn->BltBackgroundOnChromaKey(data, background.data, background.rect(), key, tol1, tol2, x, y);
    }

    /** @brief Rechteck unter Berücksichtigung eines Farbschlüssels kopieren (Bluescreen-Effekt)
     *
     * Mit dieser Funktion kann ein "Bluescreen-Effekt" erzielt werden (siehe http://de.wikipedia.org/wiki/Bluescreen-Technik#Greenscreen).
     * Dabei wird die Hintergundgrafik \p background mittels eines Farbschlüssels \p key (Chroma Key), sowie zwei Toleranz-Werten
     * über die Grafik gelegt.
     *
     * @param source Quellgrafik
     * @param srect Rechteck in der Quellgrafik, das kopiert werden soll
     * @param key Farbschlüssel (z.B. Color(0,0,255) für einen Bluescreen oder Color(0,255,0) für
     * einen Greenscreen)
     * @param tol1 Untere Toleranz: Farbabweichungen bis zu diesem Toleranzwert, werden komplett Transparent,
     * das heisst der Hintergrund wird übernommen
     * @param tol2 Obere Toleranz: Farbabweichungen, die zwischen \p tol1 und \p tol2 liegen, werden je nach
     * Stärke der Abweichung überblendet. Je stärker die Abweichung, desto mehr Hintergrund ist zu sehen
     * @param x Zielkoordinate für das Rechteck (optional, Default ist 0)
     * @param y Zielkoordinate für das Rechteck (optional, Default ist 0)
     *
     * @remarks Auf 64-Bit-Systemen mit SSE2-Unterstützung werden optimierte Assembler-Routinen verwendet.
     * Sofern Bildbreite durch 4 und die Speicheradressen durch 16 teilbar sind, werden jeweils 4 Pixel
     * gleichzeitig berechnet.
     *
     * @see Die Funktion bltChromaKey wendet den Farbschlüssel auf das Quellbild \p source an.
     * @see Die Funktion bltBackgroundOnChromaKey wendet den Farbschlüssel nicht auf das Quellbild \p source
     * sondern den Hintergrund an.
     */
    inline void bltBackgroundOnChromaKey(
        const Drawable& background, const Rect& srect, const Color& key, uint8_t tol1, uint8_t tol2, int x = 0, int y = 0)
    {
        data.fn->BltBackgroundOnChromaKey(data, background.data, srect, key, tol1, tol2, x, y);
    }

    /** @brief Grafik aus einer Image-Liste kopieren
     *
     * Mit dieser Funktion wird eine Grafik aus einer Image-Liste (siehe CImageList) kopiert.
     * Jenachdem welche Zeichenmethode in der Image-Liste definiert ist, wird dazu entweder
     * Drawable::blt, Drawable::bltDiffuse, Drawable::bltColorKey oder Drawable::bltAlpha
     * verwendet.
     *
     * @param iml Image-Liste
     * @param nr Nummer der Grafik innerhalb der Image-Liste
     * @param x X-Koordinate der Zielposition
     * @param y Y-Koordinate der Zielposition
     *
     * \exception EmptyDrawableException Der Parameter \p source enthält keinen darstellbaren Inhalt
     * \exception FunctionUnavailableException Funktion wird für das eingestellte Farbformat nicht unterstützt
     * \exception UnknownBltMethodException Die Zeichenmethode der ImageList ist unbekannt
     */
    void draw(const ImageList& iml, int nr, int x, int y);

    /** @brief Grafik aus einer Image-Liste kopieren
     *
     * Mit dieser Funktion wird eine Grafik aus einer Image-Liste (siehe CImageList) kopiert.
     * Jenachdem welche Zeichenmethode in der Image-Liste definiert ist, wird dazu entweder
     * Drawable::blt, Drawable::bltDiffuse, Drawable::bltColorKey oder Drawable::bltAlpha
     * verwendet. Ist die Methode CImageList::DIFFUSE, wird die Farbe \p diffuse statt der
     * in der Image-Liste definierten Farbe verwendet.
     *
     * @param iml Image-Liste
     * @param nr Nummer der Grafik innerhalb der Image-Liste
     * @param x X-Koordinate der Zielposition
     * @param y Y-Koordinate der Zielposition
     * @param diffuse Farbwert, sofern die Diffuse Zeichenmethode verwendet wird. Bei allen
     * anderen Zeichenmethoden wird der Parameter ignoriert.
     *
     * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
     */
    void draw(const ImageList& iml, int nr, int x, int y, const Color& diffuse);

    void draw(const ImageReference& imgref, int x, int y);
    void drawBlend(const ImageReference& imgref, int x, int y, float factor);

    void draw(const Sprite& sprite, int nr, int x, int y);
    //@}
};

/** @brief Berechnet den Ausschnitt des Quell-Drawables, der auf das Ziel-Drawable gezeichnet werden kann
 *
 * Diese Funktion berechnet den Ausschnitt des Quell-Drawables, der auf das Ziel-Drawable gezeichnet werden kann.
 * Sie überprüft, ob der angegebene Ausschnitt innerhalb der Grenzen des Quell-Drawables liegt und passt die Koordinaten
 * entsprechend an. Die Funktion gibt true zurück, wenn der Ausschnitt gültig ist und gezeichnet werden kann, andernfalls false.
 *
 * @param[in] target Das Ziel-Drawable, auf das gezeichnet werden soll
 * @param[in] source Das Quell-Drawable, von dem gezeichnet werden soll
 * @param[in] srect Der gewünschte Ausschnitt des Quell-Drawables
 * @param[in,out] x Die X-Koordinate im Ziel-Drawable, an der der Ausschnitt gezeichnet werden soll (wird angepasst)
 * @param[in,out] y Die Y-Koordinate im Ziel-Drawable, an der der Ausschnitt gezeichnet werden soll (wird angepasst)
 * @param[out] clipped_srect Der berechnete und angepasste Ausschnitt des Quell-Drawables (Ausgabeparameter)
 * @return true, wenn der Ausschnitt gültig ist und gezeichnet werden kann, andernfalls false
 */
bool clip(const DrawableData& target, const DrawableData& source, const Rect& srect, int& x, int& y, Rect& clipped_srect);

} // namespace pplib::grafix
#endif // PPLIB_INCLUDE_GRAFIX_DRAWABLE_H
