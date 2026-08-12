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

#ifndef PPLIB_INCLUDE_GRAFIX_SIZE_H
#define PPLIB_INCLUDE_GRAFIX_SIZE_H

#include <stdint.h>

namespace pplib::grafix
{
class Size16;

#ifndef real_t
#ifdef PICO_BUILD
typedef float real_t;
#else
typedef double real_t;
#endif
#endif

/** @class Size
 * \ingroup PPLGroupGrafik
 * \brief Repräsentiert Breite und Höhe eines beliebigen Objekts in einem zweidimensionalen Koordinatensystem
 */
class Size
{
public:
    int width;  /// Breite des Objekts
    int height; /// Höhe des Objekts

    /** @brief Default Konstruktor
     *
     * Erstellt ein Size-Objekt mit 0 als Breite und Höhe
     */
    Size()
    {
        width = 0;
        height = 0;
    }

    /** @brief Konstruktor mit Werten
     *
     * Erstellt ein Size-Objekt mit der angegebenen Breite und Höhe
     *
     * @param width Breite des Objekts
     * @param height Höhe des Objekts
     */
    Size(int width, int height)
    {
        this->width = width;
        this->height = height;
    }

    /** @brief Konstruktor mit Size16
     *
     * Erstellt ein Size-Objekt mit den Werten eines Size16-Objekts
     *
     * @param other Size16-Objekt, dessen Werte übernommen werden sollen
     */
    Size(const Size16& other);

    /** @brief Prüft, ob das Size-Objekt Null ist (Breite und Höhe sind 0)
     *
     * @return true, wenn Breite und Höhe 0 sind, sonst false
     */
    inline bool isNull() const
    {
        return (width == 0 && height == 0);
    }

    /** @brief Prüft, ob das Size-Objekt leer ist (Breite oder Höhe ist 0)
     *
     * @return true, wenn Breite oder Höhe 0 ist, sonst false
     */
    inline bool isEmpty() const
    {
        return (width == 0 || height == 0);
    }

    /** @brief Prüft, ob das Size-Objekt gültig ist (Breite und Höhe sind >= 0)
     *
     * @return true, wenn Breite und Höhe >= 0 sind, sonst false
     */
    inline bool isValid() const
    {
        return (width >= 0 && height >= 0);
    }

    /** @brief Setzt die Höhe des Size-Objekts
     *
     * @param height Neue Höhe
     */
    inline void setHeight(int height)
    {
        this->height = height;
    }

    /** @brief Setzt die Breite des Size-Objekts
     *
     * @param width Neue Breite
     */
    inline void setWidth(int width)
    {
        this->width = width;
    }

    /** @brief Setzt die Breite und Höhe des Size-Objekts
     *
     * @param width Neue Breite
     * @param height Neue Höhe
     */
    inline void setSize(int width, int height)
    {
        this->width = width;
        this->height = height;
    }

    /** @brief Setzt die Breite und Höhe des Size-Objekts anhand eines Size-Objekts
     *
     * @param s Size-Objekt, dessen Werte übernommen werden sollen
     */
    inline void setSize(const Size& s)
    {
        width = s.width;
        height = s.height;
    }

    /** @brief Setzt die Breite und Höhe des Size-Objekts anhand eines Size16-Objekts
     *
     * @param s Size16-Objekt, dessen Werte übernommen werden sollen
     */
    void setSize(const Size16& s);

    /** @brief Zuweisungsoperator, der ein Size16-Objekt in ein Size-Objekt konvertiert
     *
     * @param other Size16-Objekt, das konvertiert und zugewiesen werden soll
     * @return Referenz auf das aktuelle Size-Objekt nach der Zuweisung
     */
    Size& operator=(const Size16& other);

    /** @brief Multiplikationsoperator, der die Breite und Höhe des Size-Objekts mit einem Faktor multipliziert
     *
     * @param factor Faktor, mit dem die Breite und Höhe multipliziert werden sollen
     * @return Referenz auf das aktuelle Size-Objekt nach der Multiplikation
     */
    inline Size& operator*=(real_t factor)
    {
        width = (int)((real_t)width * factor);
        height = (int)((real_t)height * factor);
        return *this;
    }

    /** @brief Additionsoperator, der die Breite und Höhe des Size-Objekts mit einem anderen Size-Objekt addiert
     *
     * @param size Size-Objekt, dessen Breite und Höhe addiert werden sollen
     * @return Referenz auf das aktuelle Size-Objekt nach der Addition
     */
    inline Size& operator+=(const Size& size)
    {
        width += size.width;
        height += size.height;
        return *this;
    }

    /** @brief Subtraktionsoperator, der die Breite und Höhe des Size-Objekts mit einem anderen Size-Objekt subtrahiert
     *
     * @param size Size-Objekt, dessen Breite und Höhe subtrahiert werden sollen
     * @return Referenz auf das aktuelle Size-Objekt nach der Subtraktion
     */
    inline Size& operator-=(const Size& size)
    {
        width -= size.width;
        height -= size.height;
        return *this;
    }

    /** @brief Divisionsoperator, der die Breite und Höhe des Size-Objekts durch einen Divisor teilt
     *
     * @param divisor Divisor, durch den die Breite und Höhe geteilt werden sollen
     * @return Referenz auf das aktuelle Size-Objekt nach der Division
     */
    inline Size& operator/=(real_t divisor)
    {
        width = (int)((real_t)width / divisor);
        height = (int)((real_t)height / divisor);
        return *this;
    }

    /** @brief Gibt ein ungültiges Size-Objekt zurück
     *
     * @return Size-Objekt mit Breite und Höhe -1
     */
    inline static Size invalid()
    {
        return Size(-1, -1);
    };
};

/** @brief Ungleichheitsoperator für Size-Objekte
 *
 * Vergleicht zwei Size-Objekte auf Ungleichheit.
 *
 * @param s1 Erstes Size-Objekt
 * @param s2 Zweites Size-Objekt
 * @return true, wenn die Breite oder Höhe unterschiedlich ist, sonst false
 */
inline bool operator!=(const Size& s1, const Size& s2)
{
    return (s1.width != s2.width || s1.height != s2.height);
}

/** @brief Gleichheitsoperator für Size-Objekte
 *
 * Vergleicht zwei Size-Objekte auf Gleichheit.
 *
 * @param s1 Erstes Size-Objekt
 * @param s2 Zweites Size-Objekt
 * @return true, wenn die Breite und Höhe gleich sind, sonst false
 */
inline bool operator==(const Size& s1, const Size& s2)
{
    return (s1.width == s2.width && s1.height == s2.height);
}

/** @brief Multiplikationsoperator für Size-Objekte
 *
 * Multipliziert die Breite und Höhe eines Size-Objekts mit einem Faktor.
 *
 * @param size Size-Objekt, dessen Breite und Höhe multipliziert werden sollen
 * @param factor Faktor, mit dem die Breite und Höhe multipliziert werden sollen
 * @return Neues Size-Objekt mit den multiplizierten Werten
 */
inline const Size operator*(const Size& size, real_t factor)
{
    return Size((int)((real_t)size.width * factor), (int)((real_t)size.height * factor));
}

/** @brief Multiplikationsoperator für Size-Objekte
 *
 * Multipliziert die Breite und Höhe eines Size-Objekts mit einem Faktor.
 *
 * @param factor Faktor, mit dem die Breite und Höhe multipliziert werden sollen
 * @param size Size-Objekt, dessen Breite und Höhe multipliziert werden sollen
 * @return Neues Size-Objekt mit den multiplizierten Werten
 */
inline const Size operator*(real_t factor, const Size& size)
{
    return Size((int)((real_t)size.width * factor), (int)((real_t)size.height * factor));
}

/** @brief Additionsoperator für Size-Objekte
 *
 * Addiert die Breite und Höhe zweier Size-Objekte.
 *
 * @param s1 Erstes Size-Objekt
 * @param s2 Zweites Size-Objekt
 * @return Neues Size-Objekt mit der addierten Breite und Höhe
 */
inline const Size operator+(const Size& s1, const Size& s2)
{
    return Size(s1.width + s2.width, s1.height + s2.height);
}

/** @brief Subtraktionsoperator für Size-Objekte
 *
 * Subtrahiert die Breite und Höhe zweier Size-Objekte.
 *
 * @param s1 Erstes Size-Objekt
 * @param s2 Zweites Size-Objekt
 * @return Neues Size-Objekt mit der subtrahierten Breite und Höhe
 */
inline const Size operator-(const Size& s1, const Size& s2)
{
    return Size(s1.width - s2.width, s1.height - s2.height);
}

/** @brief Negationsoperator für Size-Objekte
 *
 * Negiert die Breite und Höhe eines Size-Objekts.
 *
 * @param size Size-Objekt, dessen Breite und Höhe negiert werden sollen
 * @return Neues Size-Objekt mit negierter Breite und Höhe
 */
inline const Size operator-(const Size& size)
{
    return Size(0 - size.width, 0 - size.height);
}

/** @brief Divisionsoperator für Size-Objekte
 *
 * Teilt die Breite und Höhe eines Size-Objekts durch einen Divisor.
 *
 * @param size Size-Objekt, dessen Breite und Höhe geteilt werden sollen
 * @param divisor Divisor, durch den die Breite und Höhe geteilt werden sollen
 * @return Neues Size-Objekt mit der geteilten Breite und Höhe
 */
inline const Size operator/(const Size& size, real_t divisor)
{
    return Size((int)((real_t)size.width / divisor), (int)((real_t)size.height / divisor));
}

/** @class Size16
 * \ingroup PPLGroupGrafik
 * \brief Repräsentiert Breite und Höhe eines beliebigen Objekts in einem zweidimensionalen Koordinatensystem mit 16-Bit-Ganzzahlen
 */
class Size16
{
public:
    int16_t width;  /// Breite des Objekts
    int16_t height; /// Höhe des Objekts

    Size16()
    {
        width = 0;
        height = 0;
    }

    Size16(int16_t width, int16_t height)
    {
        this->width = width;
        this->height = height;
    }

    Size16(const Size& other);

    inline bool isNull() const
    {
        return (width == 0 && height == 0);
    }

    inline bool isEmpty() const
    {
        return (width == 0 || height == 0);
    }

    inline bool isValid() const
    {
        return (width >= 0 && height >= 0);
    }

    inline void setHeight(int16_t height)
    {
        this->height = height;
    }

    inline void setWidth(int16_t width)
    {
        this->width = width;
    }

    inline void setSize(int16_t width, int16_t height)
    {
        this->width = width;
        this->height = height;
    }

    inline void setSize(const Size16& s)
    {
        width = s.width;
        height = s.height;
    }

    void setSize(const Size& s);

    Size16& operator=(const Size& other);

    inline Size16& operator*=(float factor)
    {
        width = (int)((float)width * factor);
        height = (int)((float)height * factor);
        return *this;
    }

    inline Size16& operator+=(const Size16& size)
    {
        width += size.width;
        height += size.height;
        return *this;
    }

    inline Size16& operator-=(const Size16& size)
    {
        width -= size.width;
        height -= size.height;
        return *this;
    }

    inline Size16& operator/=(float divisor)
    {
        width = (int)((float)width / divisor);
        height = (int)((float)height / divisor);
        return *this;
    }
};

inline bool operator!=(const Size16& s1, const Size16& s2)
{
    return (s1.width != s2.width || s1.height != s2.height);
}

inline bool operator==(const Size16& s1, const Size16& s2)
{
    return (s1.width == s2.width && s1.height == s2.height);
}

inline const Size16 operator*(const Size16& size, float factor)
{
    return Size16((int)(size.width * factor), (int)(size.height * factor));
}

inline const Size16 operator*(float factor, const Size16& size)
{
    return Size16((int)(size.width * factor), (int)(size.height * factor));
}

inline const Size16 operator+(const Size16& s1, const Size16& s2)
{
    return Size16(s1.width + s2.width, s1.height + s2.height);
}

inline const Size16 operator-(const Size16& s1, const Size16& s2)
{
    return Size16(s1.width - s2.width, s1.height - s2.height);
}

inline const Size16 operator-(const Size16& size)
{
    return Size16(0 - size.width, 0 - size.height);
}

inline const Size16 operator/(const Size16& size, float divisor)
{
    return Size16((int)((float)size.width / divisor), (int)((float)size.height / divisor));
}

} // namespace pplib::grafix
#endif // PPLIB_INCLUDE_GRAFIX_SIZE_H