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

#ifndef PPLIB_INCLUDE_GRAFIX_RGBFORMAT_H
#define PPLIB_INCLUDE_GRAFIX_RGBFORMAT_H

#include <stdint.h>
#include <pplib/types/string.h>

namespace pplib::grafix
{
/** @class RGBFormat
 * @brief Datentyp, der das Farbformat einer Zeichenfläche repräsentiert
 *
 * Mit dieser Klasse wird das Farbformat einer Zeichenfläche repräsentiert. Sie enthält nur
 * einen einzigen Wert aus der Enumeration RGBFormat::Identifier, der das verwendete
 * Farbformat festlegt.
 */
class RGBFormat
{
public:
    enum Identifier
    {
        unknown = 0,

        /// @name Allgemeine Formate, die in allen PPLIB-Versionen unterstützt werden
        //@{

        Monochrome1BitVertical, ///< 1 Bit pro Pixel, vertikal gepackt (z.B. SSD1322)
        GREY8,                  ///< 8 Bit pro Pixel: 8 Bit Graustufe
        A8,                     ///< 8 Bit pro Pixel: 8 Bit Alpha (Transparenz)
        R5G6B5,                 ///< 16 Bit pro Pixel: 5 Bit Rot, 6 Bit Grün, 5 Bit Blau
        A8R8G8B8,               ///< 32 Bit pro Pixel: 8 Bit Alpha, 8 Bit Rot, 8 Bit Grün, 8 Bit Blau

        // @}

#ifndef PICO_BUILD
        /// @name Erweiterte Formate, die nicht in Embedded-Systemen unterstützt werden
        //@{

        // 8 Bit
        /** @brief 8-Bit Farbwerte aus einer Farbpalette
         *
         * Es handelt sich um eine Grafik mit einer Farbpalette von 256 Farben. Satt des eigentlichen Farbwertes
         * enthält jeder Pixel der Grafik eine Referenz auf die Farbpalette. Dieses Format war Anfang der 90er
         * sehr populär, da es wenig Speicher braucht, sehr schnell ist und von jeder Grafikkarte unterstützt
         * wurde.
         */
        Palette,

        R3G3B2, /// 8-Bit, 3 Bit für Rot, 3 Bit für Grün und 2 Bit für Blau, wobei Blau in den unteren Bits enthalten ist.

        // 16 Bit

        B5G6R5, /// 16-Bit pro Pixel, 5 Bit für Blau, 6 Bit für Grün und 5 Bit für Rot, wobei Rot in den unteren Bits enthalten ist.

        /// 16-Bit High-Color, 5 Bit für Rot, 5 Bit für Grün und 5 Bit für Blau, wobei Blau in den unteren Bits enthalten ist und
        /// das höchstwertige Bit unbenutzt ist.
        X1R5G5B5,

        /// 16-Bit High-Color, 5 Bit für Rot, 5 Bit für Grün und 5 Bit für Blau, wobei Blau in den unteren Bits enthalten ist und
        /// das höchstwertige Bit als Alphakanal (Transparenz) verwendet wird
        X1B5G5R5,

        /// 16-Bit High-Color, 4 Bit für Rot, 4 Bit für Grün und 4 Bit für Blau, wobei Blau in den unteren Bits enthalten ist und
        /// die höchstwertigen 4-Bit unbenutzt sind.
        A1R5G5B5,

        /// 16-Bit High-Color, 4 Bit für Rot, 4 Bit für Grün und 4 Bit für Blau, wobei Blau in den unteren Bits enthalten ist und
        /// die höchstwertigen 4-Bit als Alphakanal (Transparenz
        A1B5G5R5,

        /// 16-Bit High-Color, 4 Bit für Rot, 4 Bit für Grün und 4 Bit für Blau, wobei Blau in den unteren Bits enthalten ist und
        /// die höchstwertigen 4-Bit unbenutzt sind.
        X4R4G4B4,

        /// 16-Bit High-Color, 4 Bit für Rot, 4 Bit für Grün und 4 Bit für Blau, wobei Blau in den unteren Bits enthalten ist und
        /// die höchstwertigen 4-Bit unbenutzt sind.
        X4B4G4R4,

        /// 16-Bit High-Color, 4 Bit für Rot, 4 Bit für Grün und 4 Bit für Blau, wobei Blau in den unteren Bits enthalten ist und
        /// die höchstwertigen 4-Bit als Alphakanal (Transparenz) verwendet werden.
        A4R4G4B4,

        /// 16-Bit High-Color, 4 Bit für Rot, 4 Bit für Grün und 4 Bit für Blau, wobei Blau in den unteren Bits enthalten ist und
        /// die höchstwertigen 4-Bit als Alphakanal (Transparenz) verwendet werden.
        A4B4G4R4,

        /// 16-Bit, 3 Bit für Rot, 3 Bit für Grün, 2 Bit für Blau und 8 Bit für den Alphakanal, wobei Blau in den unteren Bits enthalten
        /// ist und in den höchstwertigen 8-Bit der Alphakanal (Transparenz).
        A8R3G3B2,

        /// 24-Bit Truecolor, jeweils 8 Bit für Rot, Grün und Blau, wobei Blau im untersten Byte gespeichert wird.
        R8G8B8,

        /// 24-Bit Truecolor, jeweils 8 Bit für Rot, Grün und Blau, wobei Rot im untersten Byte gespeichert wird.
        B8G8R8,

        /// ABGR 32-Bit Truecolor, jeweils 8 Bit für den Alpha-Wert, Rot, Grün und Blau, wobei Blau im untersten Byte gespeichert wird
        A8B8G8R8,

        /// RGBA 32-Bit Truecolor, jeweils 8 Bit für den Alpha-Wert, Rot, Grün und Blau, wobei Rot im untersten Byte gespeichert wird
        R8G8B8A8,

        /// RGBA 32-Bit Truecolor, jeweils 8 Bit für den Alpha-Wert, Rot, Grün und Blau, wobei Rot im untersten Byte gespeichert wird
        B8G8R8A8,

        /// XRGB 32-Bit Truecolor, jeweils 8 Bit für Rot, Grün und Blau, wobei Blau im untersten Byte gespeichert wird und das höchste Byte
        /// unbenutzt ist
        X8R8G8B8,

        /// XBGR 32-Bit Truecolor, jeweils 8 Bit für Rot, Grün und Blau, wobei Rot im untersten Byte gespeichert wird und das höchste Byte
        /// unbenutzt ist
        X8B8G8R8,

        /// RGBX 32-Bit Truecolor, jeweils 8 Bit für Rot, Grün und Blau, wobei Blau im untersten Byte gespeichert wird und das höchste Byte
        /// unbenutzt ist
        R8G8B8X8,

        /// BGRX 32-Bit Truecolor, jeweils 8 Bit für Rot, Grün und Blau, wobei Rot im untersten Byte gespeichert wird und das höchste Byte
        /// unbenutzt ist
        B8G8R8X8,

        //@}
#endif
        MaxIdentifiers /// Obergrenze der Identifikatoren
    };

private:
    /**
     * @brief Das aktuell gesetzte Farbformat.
     */
    Identifier format_id = unknown;

public:
    RGBFormat() noexcept = default;

    /** @brief Konstruktor mit Identifier
     *
     * Mit diesem Konstruktor wird die Klasse mit einem vorgegebenen Farbformat initialisiert
     *
     * @param[in] id Wert aus der Enumeration RGBFormat::Identifier
     */
    RGBFormat(Identifier id);

    /** @brief Konstruktor mit Namen
     *
     * Mit diesem Konstruktor wird die Klasse mit einem Farbformat initialisiert, dass anhand
     * seines Namens definiert wird. Die Namen sind identisch mit denen aus der Enumeration
     * RGBFormat::Identifier
     *
     * @param[in] Identifier String mit dem Namen des Farbformats
     * @exception UnknownColorFormatException Wird geworfen, wenn der \p Identifier nicht bekannt ist.
     *
     * \example
     * \code
     * RGBFormat rgb("A8R8B8G8");
     * \endcode
     */
    RGBFormat(const String& Identifier);

    /** @brief Farbformat Anhand einer ID festlegen
     *
     * Mit dieser Funktion wird das Farbformat anhand eines Wertes aus der Enumeration
     * RGBFormat::Identifier gesetzt.
     *
     * @param[in] id Wert aus der Enumeration RGBFormat::Identifier
     * @exception UnknownColorFormatException Wird geworfen, wenn der \p id nicht bekannt ist.
     */
    void setFormat(Identifier id);

    /** @brief Farbformat anhand seines Namens festlegen
     *
     * Mit dieser Funktion wird das Farbformat anhand seines Namens gesetzt.
     * Die Namen sind identisch mit denen aus der Enumeration RGBFormat::Identifier.
     *
     * @param[in] Identifier String mit dem Namen des Farbformats
     * @exception UnknownColorFormatException Wird geworfen, wenn der \p Identifier nicht bekannt ist
     *
     * @example
     * @code
     * RGBFormat rgb;
     * rgb.setFormat("A8R8B8G8");
     * @endcode
     * erkannt wurde.
     */
    void setFormat(const String& Identifier);

    /** @brief Integer-Wert des Farbformats auslesen
     *
     * Mit dieser Funktion wird der Integer-Wert des Farbformates ausgelesen.
     * Der Wert entspricht der Position des Farbformates innerhalb der Enumeration RGBFormat::Identifier.
     *
     * @return Integer-Wert, der das Farbformat repräsentiert.
     */
    inline int format() const
    {
        return format_id;
    }

    /** @brief Name des Farbformates
     *
     * Diese Funktion gibt den Namen des aktuell gewählten Farbformates als String zurück.
     *
     * @return String mit dem Namen des Farbformates, oder "unknown", wenn kein Farbformat
     * gesetzt wurde.
     */
    String name() const;

    /** @brief Bittiefe des Farbformates
     *
     * Diese Funktion liefert die Anzahl Bits zurück, die zur Darstellung des gewählten
     * Farbformats erforderlich sind. Ist in der Regel 8, 16, 24, 32 oder 64. Die Funktion ist
     * identisch mit RGBFormat::bitsPerPixel.
     *
     * @return Anzahl erforderlicher Bits oder 0, wenn kein Farbformat gesetzt ist.
     */
    int bitdepth() const;

    /** @brief Anzahl Bytes pro Pixel
     *
     * Diese Funktion liefert die Anzahl Bytes zurück, die zur Darstellung des gewählten
     * Farbformats erforderlich sind. Ist in der Regel 1, 2, 3, 4 oder 8. Die Funktion ruft
     * intern RGBFormat::bitdepth auf und teilt das Ergebnis durch 8.
     *
     * @note Formate mit einer Bittiefe < 8 liefern hier 0 zurück.
     *
     * @return Anzahl erforderlicher Bytes pro Pixel oder 0, wenn kein Farbformat gesetzt ist.
     */
    inline uint8_t bytesPerPixel() const
    {
        return bitdepth() >> 3;
    }

    /** @brief Anzahl erforderlicher Bits zur Darstellung eines Pixels.
     *
     * Diese Funktion liefert die Anzahl Bits zurück, die zur Darstellung des gewählten
     * Farbformats erforderlich sind. Ist in der Regel 8, 16, 24, 32 oder 64. Die Funktion ist
     * identisch mit RGBFormat::bitdepth.
     *
     * @return Anzahl erforderlicher Bits oder 0, wenn kein Farbformat gesetzt ist.
     */
    inline uint8_t bitsPerPixel() const
    {
        return bitdepth();
    }

    /** @brief Anzahl Bytes pro Pixel
     *
     * Diese Funktion liefert die Anzahl Bytes für eine bestimmte Anzahl von Pixeln zurück,
     * abhängig vom gewählten Farbformat.
     *
     * @param[in] width Anzahl Pixel, für die die erforderlichen Bytes berechnet werden sollen.
     * @return Anzahl erforderlicher Bytes pro Pixel oder 0, wenn kein Farbformat gesetzt ist.
     * @note Sonderfall ist das Farbformat Monochrome1BitVertical, bei dem die Pixel
     * vertikal gepackt sind. In diesem Fall entspricht die Anzahl Bytes der Anzahl Pixel,
     * da 8 Pixel pro Byte gespeichert werden, aber vertikal gepackt, so dass es einfach
     * width ist.
     */
    uint32_t bytesForWidth(uint32_t width) const;

    /** @brief Farbformat Anhand eines Identifiers setzten
     *
     * Mit diesem Operator wird das Farbformat anhand eines Wertes aus der Enumeration
     * RGBFormat::Identifier gesetzt.
     *
     * @param[in] id Wert aus der Enumeration RGBFormat::Identifier
     * @return Liefert eine Referenz auf die Klasse zurück.
     */
    inline RGBFormat& operator=(Identifier id)
    {
        format_id = id;
        return *this;
    }

    /** @brief Integer-Wert des Farbformats auslesen
     *
     * Mit diesem Operator wird der Integer-Wert des Farbformates ausgelesen.
     * Der Wert entspricht der Position des Farbformates innerhalb der Enumeration RGBFormat::Identifier.
     *
     * @return Integer-Wert, der das Farbformat repräsentiert.
     */
    inline operator int() const
    {
        return format_id;
    }

    friend bool operator!=(const RGBFormat& r1, const RGBFormat& r2);
    friend bool operator==(const RGBFormat& r1, const RGBFormat& r2);
    friend bool operator==(const RGBFormat& r1, RGBFormat::Identifier r2);
    friend bool operator!=(const RGBFormat& r1, RGBFormat::Identifier r2);
    friend bool operator==(RGBFormat::Identifier r1, const RGBFormat& r2);
    friend bool operator!=(RGBFormat::Identifier r1, const RGBFormat& r2);
};

/** @brief Zwei Farbformate vergleichen
 *
 * Mit dem Operatoren != und == können zwei Farbwerte miteinander verglichen werden.
 *
 * @param r1 Erstes Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @param r2 Zweites Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @return Wird == verwendet,
 * liefert die Funktion \c true zurück, wenn beide Farbformate identisch sind, sonst \c false. Wird !=
 * verwendet, liefert die Funktion \c true zurück, wenn die beiden Farbformate unterschiedlich sind, sonst
 * \c true.
 */
inline bool operator!=(const RGBFormat& r1, const RGBFormat& r2)
{
    return r1.format_id != r2.format_id;
}

/** @brief Zwei Farbformate vergleichen
 *
 * Mit dem Operatoren != und == können zwei Farbwerte miteinander verglichen werden.
 *
 * @param r1 Erstes Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @param r2 Zweites Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @return Wird == verwendet,
 * liefert die Funktion \c true zurück, wenn beide Farbformate identisch sind, sonst \c false. Wird !=
 * verwendet, liefert die Funktion \c true zurück, wenn die beiden Farbformate unterschiedlich sind, sonst
 * \c true.
 */
inline bool operator==(const RGBFormat& r1, const RGBFormat& r2)
{
    return r1.format_id == r2.format_id;
}

/** @brief Zwei Farbformate vergleichen
 *
 * Mit dem Operatoren != und == können zwei Farbwerte miteinander verglichen werden.
 *
 * @param r1 Erstes Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @param r2 Zweites Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @return Wird == verwendet,
 * liefert die Funktion \c true zurück, wenn beide Farbformate identisch sind, sonst \c false. Wird !=
 * verwendet, liefert die Funktion \c true zurück, wenn die beiden Farbformate unterschiedlich sind, sonst
 * \c true.
 */
inline bool operator==(const RGBFormat& r1, RGBFormat::Identifier r2)
{
    return r1.format_id == r2;
}

/** @brief Zwei Farbformate vergleichen
 *
 * Mit dem Operatoren != und == können zwei Farbwerte miteinander verglichen werden.
 *
 * @param r1 Erstes Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @param r2 Zweites Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @return Wird == verwendet,
 * liefert die Funktion \c true zurück, wenn beide Farbformate identisch sind, sonst \c false. Wird !=
 * verwendet, liefert die Funktion \c true zurück, wenn die beiden Farbformate unterschiedlich sind, sonst
 * \c true.
 */
inline bool operator!=(const RGBFormat& r1, RGBFormat::Identifier r2)
{
    return r1.format_id != r2;
}

/** @brief Zwei Farbformate vergleichen
 *
 * Mit dem Operatoren != und == können zwei Farbwerte miteinander verglichen werden.
 *
 * @param r1 Erstes Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @param r2 Zweites Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @return Wird == verwendet,
 * liefert die Funktion \c true zurück, wenn beide Farbformate identisch sind, sonst \c false. Wird !=
 * verwendet, liefert die Funktion \c true zurück, wenn die beiden Farbformate unterschiedlich sind, sonst
 * \c true.
 */
inline bool operator==(RGBFormat::Identifier r1, const RGBFormat& r2)
{
    return r1 == r2.format_id;
}

/** @brief Zwei Farbformate vergleichen
 *
 * Mit dem Operatoren != und == können zwei Farbwerte miteinander verglichen werden.
 *
 * @param r1 Erstes Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @param r2 Zweites Farbformat. Kenn entweder eine andere RGBFormat-Klasse sein, oder ein Wert
 * aus der Enumeration RGBFormat::Identifier
 * @return Wird == verwendet,
 * liefert die Funktion \c true zurück, wenn beide Farbformate identisch sind, sonst \c false. Wird !=
 * verwendet, liefert die Funktion \c true zurück, wenn die beiden Farbformate unterschiedlich sind, sonst
 * \c true.
 */
inline bool operator!=(RGBFormat::Identifier r1, const RGBFormat& r2)
{
    return r1 != r2.format_id;
}

} // namespace pplib::grafix

#endif // PPLIB_INCLUDE_GRAFIX_RGBFORMAT_H
