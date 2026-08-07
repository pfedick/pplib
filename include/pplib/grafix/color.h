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

#ifndef _PPLIB_INCLUDE_GRAFIX_COLOR
#define _PPLIB_INCLUDE_GRAFIX_COLOR

#include <stdint.h>
#include <ostream>
#include <cmath>

namespace pplib::grafix
{

/**
 * @class Color
 * @ingroup PPLGroupGrafik
 * @brief Ein Datenobjekt zum Speichern eines Farbwertes
 *
 * Diese Klasse wird als Objekt zum Speichern einer Farbe verwendet, die sich aus den
 * Komponenten Rot, Grün, Blau und optional einem Transparanz-Wert (Alpha-Wert) zusammensetzt.
 * Der Farbwert ist 32-Bit breit, wobei jede der 4 Komponenten 8 Bit verwendet.
 *
 */
class Color
{
    friend Color operator*(const Color& size, float factor) noexcept;
    friend Color operator*(float factor, const Color& size) noexcept;
    friend Color operator+(const Color& color1, const Color& color2) noexcept;
    friend Color multiplyWithAlpha(const Color& c, float factor) noexcept;
    ;

private:
#ifdef __LITTLE_ENDIAN__
    union {
        struct
        {
            uint8_t r, g, b, a;
        };
        uint32_t c;
    };
#else
    union {
        struct
        {
            uint8_t a, b, g, r;
        };
        uint32_t c;
    };
#endif
public:
    /**
     * @brief Konstruktor ohne Angabe von Werten
     *
     * Mit diesem Konstruktor wird eine Instanz der Klasse erzeugt, und mit dem Farbwert
     * für schwarz initialisiert.
     */
    Color() noexcept
    {
        c = 0;
    };

    /**
     * @brief Konstruktor mit Angabe der Farbwerte
     *
     * Mit diesem Konstruktor wird eine Instanz der Klasse erzeugt, die mit den angegebenen
     * Farbkomponenten initialisiert wird.
     *
     * @param red Wert zwischen 0 und 255, der den rot-Anteil der Farbe festlegt
     * @param green Wert zwischen 0 und 255, der den grün-Anteil der Farbe festlegt
     * @param blue Wert zwischen 0 und 255, der den blau-Anteil der Farbe festlegt
     * @param alpha Optionaler Wert zwischen 0 und 255, der die Transparenz der Farbe
     * festlegt. 0 bedeutet komplett transparent, 255 komplett sichtbar.
     */
    Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) noexcept
    {
        r = red;
        g = green;
        b = blue;
        a = alpha;
    };

    /**
     * @brief Konstruktor mit Angabe eines Farbwertes als 32-Bit Wert
     *
     * Mit diesem Konstruktor wird eine Instanz der Klasse erzeugt, die mit
     * dem angegebenen 32-Bit-Farbwert initialisiert wird.
     *
     * @param rgba 32-Bit Farbwert
     */
    Color(uint32_t rgba) noexcept
    {
        c = rgba;
    }

    /** @brief rot-Anteil auslesen
     *
     * Mit dieser Funktion wird der rot-Anteil der Farbe ausgelesen.
     *
     * @return rot-Anteil zwischen 0 und 255
     */
    constexpr inline uint8_t red() const noexcept
    {
        return r;
    }

    /** @brief grün-Anteil auslesen
     *
     * Mit dieser Funktion wird der grün-Anteil der Farbe ausgelesen.
     *
     * @return grün-Anteil zwischen 0 und 255
     */
    constexpr inline uint8_t green() const noexcept
    {
        return g;
    }

    /** @brief blau-Anteil auslesen
     *
     * Mit dieser Funktion wird der blau-Anteil der Farbe ausgelesen.
     *
     * @return blau-Anteil zwischen 0 und 255
     */
    constexpr inline uint8_t blue() const noexcept
    {
        return b;
    }

    /** @brief alpha-Anteil auslesen
     *
     * Mit dieser Funktion wird der alpha-Anteil der Farbe ausgelesen.
     *
     * @return alpha-Anteil zwischen 0 und 255
     */
    constexpr inline uint8_t alpha() const noexcept
    {
        return a;
    }

    /** @brief Farbwert als 32-Bit-Wert auslesen
     *
     * Mit dieser Funktion kann der Farbwert als 32-Bit-Wert ausgelesen werden.
     *
     * @return 32-Bit Farbwert
     */
    constexpr inline uint32_t rgba() const noexcept
    {
        return c;
    }

    /** @brief rot-Anteil setzen
     *
     * Mit dieser Funktion wird der rot-Anteil der Farbe gesetzt.
     *
     * @param red rot-Anteil zwischen 0 und 255.
     */
    inline void setRed(uint8_t red) noexcept
    {
        r = red;
    }

    /** @brief grün-Anteil setzen
     *
     * Mit dieser Funktion wird der grün-Anteil der Farbe gesetzt.
     *
     * @param green grün-Anteil zwischen 0 und 255.
     */
    inline void setGreen(uint8_t green) noexcept
    {
        g = green;
    }

    /** @brief blau-Anteil setzen
     *
     * Mit dieser Funktion wird der blau-Anteil der Farbe gesetzt.
     *
     * @param blue blau-Anteil zwischen 0 und 255.
     */
    inline void setBlue(uint8_t blue) noexcept
    {
        b = blue;
    }

    /** @brief alpha-Anteil setzen
     *
     * Mit dieser Funktion wird der alpha-Anteil der Farbe gesetzt.
     *
     * @param alpha alpha-Anteil zwischen 0 und 255.
     */
    inline void setAlpha(uint8_t alpha) noexcept
    {
        a = alpha;
    }

    /** @brief Farbwert anhand eines 32-Bit-Wertes setzen
     *
     * Mit dieser Funktgion wird die Farbe anhand des angegebenen
     * 32-Bit-Farbwertes gesetzt.
     *
     * @param rgba 32-Bit Farbwert
     */
    inline void setColor(uint32_t rgba) noexcept
    {
        c = rgba;
    }

    /** @brief Farbwert anhand der einzelnen Farbkomponenten setzen
     *
     * Mit dieser Funktion wird die Farbe anhand ihrer einzelnen Komponenten gesetzt.
     *
     * @param red Wert zwischen 0 und 255, der den rot-Anteil der Farbe festlegt
     * @param green Wert zwischen 0 und 255, der den grün-Anteil der Farbe festlegt
     * @param blue Wert zwischen 0 und 255, der den blau-Anteil der Farbe festlegt
     * @param alpha Optionaler Wert zwischen 0 und 255, der die Transparenz der Farbe
     * festlegt. 0 bedeutet komplett transparent, 255 komplett sichtbar. Wird der
     * Parameter nicht angegeben, wird automatisch 255 gesetzt.
     */
    inline void setColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) noexcept
    {
        r = red;
        g = green;
        b = blue;
        a = alpha;
    }

    /** @brief Farbwert anhand der einzelnen Farbkomponenten setzen
     *
     * Mit dieser Funktion wird die Farbe anhand ihrer einzelnen
     * Komponenten gesetzt.
     *
     * @param red Wert zwischen 0 und 255, der den rot-Anteil der Farbe festlegt
     * @param green Wert zwischen 0 und 255, der den grün-Anteil der Farbe festlegt
     * @param blue Wert zwischen 0 und 255, der den blau-Anteil der Farbe festlegt
     * @param alpha Optionaler Wert zwischen 0 und 255, der die Transparenz der Farbe
     * festlegt. 0 bedeutet komplett transparent, 255 komplett sichtbar. Wird der
     * Parameter nicht angegeben, wird automatisch 255 gesetzt.
     */
    inline void set(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) noexcept
    {
        r = red;
        g = green;
        b = blue;
        a = alpha;
    }

    /** @brief Farbwert anhand der einzelnen Farbkomponenten setzen
     *
     * Mit dieser Funktion wird die Farbe anhand ihrer einzelnen Komponenten gesetzt.
     *
     * @param red Wert zwischen 0 und 255, der den rot-Anteil der Farbe festlegt
     * @param green Wert zwischen 0 und 255, der den grün-Anteil der Farbe festlegt
     * @param blue Wert zwischen 0 und 255, der den blau-Anteil der Farbe festlegt
     * @param alpha Optionaler Wert zwischen 0 und 255, der die Transparenz der Farbe
     * festlegt. 0 bedeutet komplett transparent, 255 komplett sichtbar. Wird der
     * Parameter nicht angegeben, wird automatisch 255 gesetzt.
     */
    inline void set(const Color& other) noexcept
    {
        c = other.c;
    }

    /** @brief Helligkeit der Farbe
     *
     * Mit dieser Funktion kann die Helligkeit des Farbwerts ausgelesen werden. Um die Helligkeit
     * zu berechnen, werden die 3 Komponenten der Farbe rot, grün und blau unterschiedlich gewichtet.
     * Da grün am hellsten ist, wird dieser Komponente am höchsten bewertet, gefolgt von rot und
     * zuletzt blau. Die genaue Formel lautet:
     * `helligkeit = (rot * 11 + grün * 16 + blau * 5)/32\f$.`
     * Der Alpha-Kanal (Transparenz) spielt bei der Berechnung keine Rolle.
     *
     * @return Helligkeit der Farbe auf einer Skala von 0-255
     */
    inline uint8_t brightness() const noexcept
    {
        return (r * 11 + g * 16 + b * 5) / 32;
    }

    /**
     * @brief Grauwert der Farbe
     *
     * Diese Funktion ist identisch zu Color::brightness, nur wird hier nicht der
     * Helligkeitswert zurückgegeben, sondern eine neue Color-Klasse, deren Farbkomponenten
     * jeweils mit dem gleichen Helligkeitswert initialisiert werden. Der Alpha-Wert wird
     * übernommen.
     *
     * @return Grau-Farbe
     */
    Color grey() const noexcept
    {
        uint8_t b = brightness();
        return Color(b, b, b, a);
    }

    /**
     * @brief Negativ der Farbe
     *
     * Mit dieser Funktion wird das Negativ der Farbe zurückgegeben. Dabei werden die Farbwerte
     * rot, grün und blau invertiert, der Alpha-Kanal bleibt unverändert.
     *
     * @return Negativ-Farbe
     */
    inline Color negativ() const noexcept
    {
        return Color(255 - r, 255 - g, 255 - b, a);
    }

    /**
     * @brief Farbübereinstimmung prüfen
     *
     * Diese Funktion prüft, ob die Farbe \p other mit dieser übereinstimmt, wobei jeder der
     * drei Farbwerte (rot, grün, blau) eine maximale Abweichung von \p tolerance haben darf.
     * Der Alpha-Kanal wird dabei nicht berücksichtigt.
     *
     * @param other Der zu vergleichende Farbwert
     * @param tolerance Ein optionaler Parameter, der die maximale Toleranz auf einer Skala
     * von 0-255 angibt. Der Default ist 0
     * @return Stimmen die Farbwerte überein, liefert die Funktion \c true zurück, andernfalls \c false.
     */
    bool match(const Color& other, uint8_t tolerance = 0) const noexcept;

    /** @brief Farbe in das YCbCr-Farbmodell umrechnen: Grundhelligkeit Y
     *
     * Das YCbCr-Modell teilt die Farbinformation auf in die Grundhelligkeit Y
     * und die zwei Farbkomponenten Cb (Blue-Yellow Chrominance) und Cr (Red-Green Chrominance).
     * Mit Y wird hier die Helligkeitsachse aus dem CIE-Normvalenzsystem verwendet. Sie entspricht
     * der Hellempfindlichkeit des Auges, die im grünen Spektralbereich am größten ist (V-Lambda-Kurve).
     * Chrominance oder kurz Chroma bedeutet Buntheit im Allgemeinen und Farbigkeit in Bezug auf
     * Helligkeit-Farbigkeits-Modelle.
     *
     * @return Grundhelligkeit (Y) auf einer Skala von 0-255
     */
    inline uint8_t getY() const noexcept
    {
        // return (uint8_t)round(0.299 * r + 0.587 * g + 0.114 * b);
        return static_cast<uint8_t>((19595 * r + 38470 * g + 7471 * b + 32768) >> 16);
    }

    /** @brief Farbe in das YCbCr-Farbmodell umrechnen: Cb-Wert (Blue-Yellow Chrominance)
     *
     * Das YCbCr-Modell teilt die Farbinformation auf in die Grundhelligkeit Y
     * und die zwei Farbkomponenten Cb (Blue-Yellow Chrominance) und Cr (Red-Green Chrominance).
     * Mit Y wird hier die Helligkeitsachse aus dem CIE-Normvalenzsystem verwendet. Sie entspricht
     * der Hellempfindlichkeit des Auges, die im grünen Spektralbereich am größten ist (V-Lambda-Kurve).
     * Chrominance oder kurz Chroma bedeutet Buntheit im Allgemeinen und Farbigkeit in Bezug auf
     * Helligkeit-Farbigkeits-Modelle.
     *
     * @return Cb-Wert (Blue-Yellow Chrominance)
     */
    inline uint8_t getYCb() const noexcept
    {
        // return (uint8_t)round(128 + -0.168736 * r - 0.331264 * g + 0.5 * b);
        int cb = 128 + ((-11059 * r - 21709 * g + 32768 * b + 32768) >> 16);
        if (cb < 0) return 0;
        if (cb > 255) return 255;
        return static_cast<uint8_t>(cb);
    }

    /** @brief Farbe in das YCbCr-Farbmodell umrechnen: Cr-Wert (Red-Green Chrominance)
     *
     * Das YCbCr-Modell teilt die Farbinformation auf in die Grundhelligkeit Y
     * und die zwei Farbkomponenten Cb (Blue-Yellow Chrominance) und Cr (Red-Green Chrominance).
     * Mit Y wird hier die Helligkeitsachse aus dem CIE-Normvalenzsystem verwendet. Sie entspricht
     * der Hellempfindlichkeit des Auges, die im grünen Spektralbereich am größten ist (V-Lambda-Kurve).
     * Chrominance oder kurz Chroma bedeutet Buntheit im Allgemeinen und Farbigkeit in Bezug auf
     * Helligkeit-Farbigkeits-Modelle.
     *
     * @return Cr-Wert (Red-Green Chrominance)
     */
    inline uint8_t getYCr() const noexcept
    {
        // return (uint8_t)round(128 + 0.5 * r - 0.418688 * g - 0.081312 * b);
        int cr = 128 + ((32768 * r - 27439 * g - 5329 * b + 32768) >> 16);
        if (cr < 0) return 0;
        if (cr > 255) return 255;
        return static_cast<uint8_t>(cr);
    }

    /** @brief Farben überblenden
     *
     * Mit dieser Funktion werden die zwei Farbwerte \p background und \p foreground
     * anhand des dritten Parameters \p intensity vermischt. Dabei gilt, je höher der Wert
     * \p intensity, desto stärker kommt der Farbwert \p foreground zur Geltung.
     *
     * @param background Hintergrundfarbe
     * @param foreground Vordergrundfarbe
     * @param intensity Intensität der Vordergrundfarbe auf einer Skala von 0-255
     * @return Neuer Farbwert
     */
    Color& blend(const Color& background, const Color& foreground, int intensity) noexcept;

    /** @brief Farben überblenden
     *
     * Mit dieser Funktion werden die zwei Farbwerte \p background und \p foreground
     * anhand des dritten Parameters \p intensity vermischt. Dabei gilt, je höher der Wert
     * \p intensity, desto stärker kommt der Farbwert \p foreground zur Geltung.
     *
     * @param background Hintergrundfarbe
     * @param foreground Vordergrundfarbe
     * @param intensity Intensität der Vordergrundfarbe auf einer Skala von 0.0 bis 1.0
     * @return Neuer Farbwert
     */
    Color& blendf(const Color& background, const Color& foreground, float intensity) noexcept;

    /** @brief Lineare Interpolation zwischen zwei Farben
     *
     * Mit dieser Funktion kann eine lineare Interpolation zwischen zwei Farben durchgeführt werden.
     * Dabei wird der Farbwert \p c1 mit dem Faktor \p factor gewichtet, während der Farbwert \p c2
     * mit dem Faktor (1.0 - \p factor) gewichtet wird. Der Faktor \p factor muss dabei im Bereich
     * von 0.0 bis 1.0 liegen.
     *
     * @param c1 Erste Farbe
     * @param c2 Zweite Farbe
     * @param factor Gewichtungsfaktor für die erste Farbe auf einer Skala von 0.0 bis 1.0
     * @return Interpolierte Farbe
     */
    static Color lerp(const Color& c1, const Color& c2, float t) noexcept;

    /** @brief Multiplikation der einzelnen Farbkomponenten mit einem Faktor
     *
     * Mit diesem Operator werden die einzelnen Farbkomponenten rot, grün und blau
     * mit dem angegebenen Faktor multipliziert. Die einzelnen Farbwerte können dabei maximal
     * den Wert 255 annehmen. Bei einem negativen Faktor kommt schwarz raus.
     * Der Alpha-Kanal wird nicht verändert.
     *
     * @param factor Der Multiplikator als float-Wert
     * @return Neuer Farbwert
     */
    Color& operator*=(float factor) noexcept;

    /** @brief Addition zweier Farbwerte
     *
     * Mit diesem Operator werden die einzelnen Farbkomponenten rot, grün, blau und alpha
     * der beiden Farbwerte addiert. Die einzelnen Farbwerte können dabei maximal
     * den Wert 255 annehmen.
     *
     * @param other Der zweite Farbwert
     * @return Neuer Farbwert
     */
    Color& operator+=(const Color& other) noexcept;

    /** @brief Farbwert als 32-Bit-Wert auslesen
     *
     * Mit diesem Operator kann der Farbwert der Klasse als 32-Bit-Wert ausgelesen werden.
     *
     * @return 32-Bit Farbwert
     */
    constexpr inline operator uint32_t() const noexcept
    {
        return c;
    }

    inline bool operator<(const Color& other) const noexcept
    {
        return c < other.c;
    }

    inline bool operator<=(const Color& other) const noexcept
    {
        return c <= other.c;
    }

    inline bool operator==(const Color& other) const noexcept
    {
        return c == other.c;
    }

    inline bool operator!=(const Color& other) const noexcept
    {
        return c != other.c;
    }

    inline bool operator>=(const Color& other) const noexcept
    {
        return c >= other.c;
    }

    inline bool operator>(const Color& other) const noexcept
    {
        return c > other.c;
    }

    /** @brief Farben überblenden
     *
     * Mit dieser Funktion werden die zwei Farbwerte \p background und \p foreground
     * anhand des dritten Parameters \p intensity vermischt. Dabei gilt, je höher der Wert
     * \p intensity, desto stärker kommt der Farbwert \p foreground zur Geltung.
     *
     * @note Der Alpha-Kanal wird hier immer auf 255 gesetzt.
     *
     * @param background Hintergrundfarbe
     * @param foreground Vordergrundfarbe
     * @param intensity Intensität der Vordergrundfarbe auf einer Skala von 0-255
     * @return Neuer Farbwert
     */
    static Color getBlended(const Color& background, const Color& foreground, int intensity) noexcept;

    /** @brief Farben überblenden
     *
     * Mit dieser Funktion werden die zwei Farbwerte \p background und \p foreground
     * anhand des dritten Parameters \p intensity vermischt. Dabei gilt, je höher der Wert
     * \p intensity, desto stärker kommt der Farbwert \p foreground zur Geltung.
     *
     * @note Der Alpha-Kanal wird hier immer auf 255 gesetzt.
     *
     * @param background Hintergrundfarbe
     * @param foreground Vordergrundfarbe
     * @param intensity Intensität der Vordergrundfarbe auf einer Skala von 0.0 bis 1.0
     * @return Neuer Farbwert
     */
    static Color getBlendedf(const Color& background, const Color& foreground, float intensity) noexcept;
};

/** @brief Multiplikation einer Farbe mit einem Faktor
 *
 * Mit dieser Funktion wird die angegebene Farbe \p c mit dem Faktor \p factor
 * multipliziert und das Ergebnis als neuer Farbwert zurückgegeben. Ein Wert größer 1.0
 * bewirkt, dass die Farbe heller wird, bei Werten kleiner 1,0 wird die Farbe dunkler.
 *
 * Ist das Ergebnis der Multiplikation eines Farbwertes größer 255, wird er auf 255 gesetzt.
 * Ein negativer Faktor wird zu 0. Der Alpha-Kanal wird ebenfalls mit dem Faktor multipliziert.
 *
 * @param c Farbwert
 * @param factor Multiplikations-Faktor
 * @return Neuer Farbwert
 */
Color multiplyWithAlpha(const Color& c, float factor) noexcept;

/** @brief Multiplikation einer Farbe mit einem Faktor
 *
 * Mit diesem Operator wird die angegebene Farbe \p color mit dem Faktor \p factor
 * multipliziert und das Ergebnis als neuer Farbwert zurückgegeben. Ein Wert größer 1.0
 * bewirkt, dass die Farbe heller wird, bei Werten kleiner 1,0 wird die Farbe dunkler.
 *
 * Ist das Ergebnis der Multiplikation eines Farbwertes größer 255, wird er auf 255 gesetzt.
 * Ein negativer Faktor wird zu 0. Der Alpha-Kanal wird nicht verändert.
 *
 * @param color Farbwert
 * @param factor Multiplikations-Faktor
 * @return Neuer Farbwert
 */
Color operator*(const Color& color, float factor) noexcept;

/** @brief Multiplikation einer Farbe mit einem Faktor
 *
 * Mit diesem Operator wird die angegebene Farbe \p color mit dem Faktor \p factor
 * multipliziert und das Ergebnis als neuer Farbwert zurückgegeben. Ein Wert größer 1.0
 * bewirkt, dass die Farbe heller wird, bei Werten kleiner 1,0 wird die Farbe dunkler.
 *
 * Ist das Ergebnis der Multiplikation eines Farbwertes größer 255, wird er auf 255 gesetzt.
 * Ein negativer Faktor wird zu 0.
 *
 * @param factor Multiplikations-Faktor
 * @param color Farbwert
 * @return Neuer Farbwert
 */
Color operator*(float factor, const Color& color) noexcept;

/** @brief Addition zweier Farbwerte
 *
 * Mit diesem Operator werden die einzelnen Farbkomponenten rot, grün, blau und alpha
 * der beiden Farbwerte addiert. Die einzelnen Farbwerte können dabei maximal
 * den Wert 255 annehmen.
 *
 * @param color1 Erster Farbwert
 * @param color2 Zweiter Farbwert
 * @return Neuer Farbwert
 */
Color operator+(const Color& color1, const Color& color2) noexcept;

/** @brief Ausgabe eines Farbwertes auf einen Stream
 *
 * Mit diesem Operator kann ein Farbwert auf einen Stream ausgegeben werden. Dabei wird
 * der Farbwert in der Form RGB(red=..., green=..., blue=..., alpha=...) ausgegeben.
 *
 * @param s Stream, auf den der Farbwert ausgegeben werden soll
 * @param c Farbwert, der ausgegeben werden soll
 * @return Referenz auf den Stream
 */
std::ostream& operator<<(std::ostream& s, const Color& c);

} // namespace pplib::grafix

#endif // _PPLIB_INCLUDE_GRAFIX_COLOR
