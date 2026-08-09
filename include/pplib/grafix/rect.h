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

#ifndef PPLIB_INCLUDE_GRAFIX_RECT_H
#define PPLIB_INCLUDE_GRAFIX_RECT_H

#include <stdint.h>

#include <pplib/grafix/point.h>
#include <pplib/grafix/size.h>

namespace pplib::grafix
{

class Point;
class Point16;
class Size;
class Size16;
class Rect16;

/**@class Rect
 * @brief Repräsentiert ein Rechteck in einem zweidimensionalen Koordinatensystem
 *
 * Ein Rechteck wird durch die Koordinaten der oberen linken Ecke (x, y) und die Breite (w) und Höhe (h) definiert.
 *
 * Nach Konvention werden die rechten und unteren Kanten des Rechtecks normalerweise als exklusiv betrachtet.
 * Mit anderen Worten liegt das Pixel mit den Koordinaten ( right, bottom ) unmittelbar außerhalb des
 * Rechtecks. Wenn zum Beispiel RECT an die Funktion FillRect übergeben wird, wird das Rechteck bis zur
 * (aber nicht einschließlich) rechten Spalte und der unteren Zeile von Pixeln gefüllt.
 */
class Rect
{
    friend bool operator!=(const Rect& r1, const Rect& r2);
    friend bool operator==(const Rect& r1, const Rect& r2);

private:
    int x, y;
    int w, h;

public:
    /**@brief Standardkonstruktor, initialisiert alle Werte mit 0
     */
    Rect();

    /**@brief Konstruktor, der zwei Punkte als gegenüberliegende Ecken des Rechtecks verwendet
     * Die Reihenfolge der Punkte spielt keine Rolle. Es wird automatisch das obere linke und das untere rechte
     * Ende des Rechtecks bestimmt.
     * @param[in] p1 Erster Punkt
     * @param[in] p2 Zweiter Punkt
     */
    Rect(const Point& p1, const Point& p2);
    /**@brief Konstruktor, der einen Punkt als obere linke Ecke und eine Größe für Breite und Höhe verwendet
     * @param[in] p Punkt, der die obere linke Ecke des Rechtecks definiert
     * @param[in] s Größe des Rechtecks, die die Breite und Höhe angibt
     */
    Rect(const Point& p, const Size& s);

    /**@brief Konstruktor, der die Koordinaten und die Größe des Rechtecks direkt angibt
     * @param[in] x X-Koordinate der oberen linken Ecke des Rechtecks
     * @param[in] y Y-Koordinate der oberen linken Ecke des Rechtecks
     * @param[in] width Breite des Rechtecks
     * @param[in] height Höhe des Rechtecks
     */
    Rect(int x, int y, int width, int height);

    /**@brief Konstruktor, der ein Rect16-Objekt in ein Rect-Objekt konvertiert
     * @param[in] other Das Rect16-Objekt, das konvertiert werden soll
     */
    Rect(const Rect16& r);

    /**@brief Rechteck hat keine Breite oder Höhe
     * @return Gibt true zurück, wenn Höche oder Breite 0 sind, andernfalls false.
     */
    bool isNull() const;
    //@brief Gibt die X-Koordinate der linken Seite des Rechtecks zurück.
    constexpr inline int left() const
    {
        return x;
    };
    //@brief Gibt die X-Koordinate der rechten Seite des Rechtecks zurück. Liegt 1 Pixel außerhalb des Rechtecks
    constexpr inline int right() const
    {
        return x + w;
    };
    //@brief Gibt die Y-Koordinate der oberen Seite des Rechtecks zurück.
    constexpr inline int top() const
    {
        return y;
    };
    //@brief Gibt die Y-Koordinate der unteren Seite des Rechtecks zurück. Liegt 1 Pixel außerhalb des Rechtecks
    constexpr inline int bottom() const
    {
        return y + h;
    };
    //@brief Gibt die X-Koordinate des letzten Pixels auf der rechten Seite des Rechtecks zurück.
    constexpr inline int lastX() const
    {
        return x + w - 1;
    }
    //@brief Gibt die Y-Koordinate des letzten Pixels auf der unteren Seite des Rechtecks zurück.
    constexpr inline int lastY() const
    {
        return y + h - 1;
    }

    //@brief Gibt die Breite des Rechtecks zurück.
    constexpr inline int width() const
    {
        return w;
    };
    //@brief Gibt die Höhe des Rechtecks zurück.
    constexpr inline int height() const
    {
        return h;
    };

    //@brief Gibt die Größe des Rechtecks als Size-Objekt zurück.
    inline Size size() const
    {
        return Size(w, h);
    };

    //@brief Gibt die Koordinaten der oberen linken Ecke des Rechtecks als Point-Objekt zurück.
    inline Point topLeft() const
    {
        return Point(x, y);
    };
    //@brief Gibt die Koordinaten der oberen rechten Ecke des Rechtecks als Point-Objekt zurück.
    inline Point topRight() const
    {
        return Point(x + w, y);
    };
    //@brief Gibt die Koordinaten der unteren linken Ecke des Rechtecks als Point-Objekt zurück.
    inline Point bottomLeft() const
    {
        return Point(x, y + h);
    };
    //@brief Gibt die Koordinaten der unteren rechten Ecke des Rechtecks als Point-Objekt zurück.
    inline Point bottomRight() const
    {
        return Point(x + w, y + h);
    };

    //@brief Berechnet das Schnittrechteck zwischen diesem Rechteck und einem anderen Rechteck. Wenn die Rechtecke sich nicht schneiden,
    // wird ein Null-Rechteck zurückgegeben.
    Rect intersected(const Rect& other) const;

    //@brief Überprüft, ob dieses Rechteck mit einem anderen Rechteck schneidet. Null-Rechtecke schneiden nicht.
    bool intersects(const Rect& other) const;

    /**@brief Setzt die Koordinaten der oberen linken Ecke des Rechtecks.
     * @param[in] topLeft Punkt, der die neue obere linke Ecke des Rechtecks definiert
     */
    void setTopLeft(const Point& topLeft);

    /**@brief Setzt die Koordinaten der unteren rechten Ecke des Rechtecks.
     * @param[in] bottomRight Punkt, der die neue untere rechte Ecke des Rechtecks definiert. Dieser liegt 1 Pixel außerhalb des Rechtecks
     */
    void setBottomRight(const Point& bottomRight);

    /**@brief Setzt die Koordinaten und die Größe des Rechtecks anhand eines anderen Rechtecks.
     * @param[in] r Rechteck, dessen Koordinaten und Größe übernommen werden sollen
     */
    void setRect(const Rect& r);

    /**@brief Setzt die Koordinaten und die Größe des Rechtecks anhand eines anderen Rechtecks vom Typ Rect16.
     * @param[in] r Rechteck, dessen Koordinaten und Größe übernommen werden sollen
     */
    void setRect(const Rect16& r);

    /**@brief Setzt die Koordinaten und die Größe des Rechtecks anhand der Koordinaten der oberen linken Ecke und der Breite und Höhe.
     * @param[in] x X-Koordinate der oberen linken Ecke des Rechtecks
     * @param[in] y Y-Koordinate der oberen linken Ecke des Rechtecks
     * @param[in] width Breite des Rechtecks
     * @param[in] height Höhe des Rechtecks
     */

    void setRect(int x, int y, int width, int height);
    /**@brief Setzt die Koordinaten des Rechtecks anhand von zwei Punkten, die gegenüberliegende Ecken des Rechtecks bilden. Die Reihenfolge
     * der Punkte spielt keine Rolle.
     * @param[in] x1 X-Koordinate des ersten Punkts
     * @param[in] y1 Y-Koordinate des ersten Punkts
     * @param[in] x2 X-Koordinate des zweiten Punkts. Dieser ist "exklusiv" und liegt 1 Pixel außerhalb des Rechtecks
     * @param[in] y2 Y-Koordinate des zweiten Punkts. Dieser ist "exklusiv" und liegt 1 Pixel außerhalb des Rechtecks
     */
    void setCoords(int x1, int y1, int x2, int y2);

    /**@brief Setzt die Koordinaten des Rechtecks anhand von zwei Punkten, die gegenüberliegende Ecken des Rechtecks bilden. Die Reihenfolge
     * der Punkte spielt keine Rolle.
     * @param[in] p1 Erster Punkt
     * @param[in] p2 Zweiter Punkt. Dieser ist "exklusiv" und liegt 1 Pixel außerhalb des Rechtecks
     */
    void setCoords(const Point& p1, const Point& p2);

    /**@brief Setzt die X-Koordinate der linken Seite des Rechtecks.
     * @param[in] left Neue X-Koordinate der linken Seite des Rechtecks
     */
    constexpr inline void setLeft(int left)
    {
        x = left;
    }

    /**@brief Setzt die X-Koordinate der rechten Seite des Rechtecks.
     * @param[in] right Neue X-Koordinate der rechten Seite des Rechtecks. Dieser liegt 1 Pixel außerhalb des Rechtecks
     */
    void setRight(int right);

    /**@brief Setzt die Y-Koordinate der oberen Seite des Rechtecks.
     * @param[in] top Neue Y-Koordinate der oberen Seite des Rechtecks
     */
    constexpr inline void setTop(int top)
    {
        y = top;
    }

    /**@brief Setzt die Y-Koordinate der unteren Seite des Rechtecks.
     * @param[in] bottom Neue Y-Koordinate der unteren Seite des Rechtecks. Dieser liegt 1 Pixel außerhalb des Rechtecks
     */
    void setBottom(int bottom);

    /**@brief Setzt die X-Koordinate der oberen linken Ecke des Rechtecks.
     * @param[in] x Neue X-Koordinate der oberen linken Ecke des Rechtecks
     */
    constexpr inline void setX(int x)
    {
        this->x = x;
    }

    /**@brief Setzt die Y-Koordinate der oberen linken Ecke des Rechtecks.
     * @param[in] y Neue Y-Koordinate der oberen linken Ecke des Rechtecks
     */
    constexpr inline void setY(int y)
    {
        this->y = y;
    }

    /**@brief Setzt die Größe des Rechtecks.
     * @param[in] size Neue Größe des Rechtecks, die die Breite und Höhe angibt
     */
    constexpr inline void setSize(const Size& size)
    {
        w = size.width;
        h = size.height;
    }

    /**@brief Setzt die Breite des Rechtecks.
     * @param[in] width Neue Breite des Rechtecks
     */
    constexpr inline void setWidth(int width)
    {
        w = width;
    }

    /**@brief Setzt die Höhe des Rechtecks.
     * @param[in] height Neue Höhe des Rechtecks
     */
    constexpr inline void setHeight(int height)
    {
        h = height;
    }

    /**@brief Zuweisungsoperator, der ein Rect16-Objekt in ein Rect-Objekt konvertiert
     * @param[in] other Das Rect16-Objekt, das konvertiert und zugewiesen werden soll
     * @return Referenz auf das aktualisierte Rect-Objekt
     */
    Rect& operator=(const Rect16& other);

    /**@brief Erstellt ein Rechteck aus zwei Punkten, deren Reihenfolge egal ist
     * Diese Funktion erstellt ein Rechteck aus zwei Punkten, ohne dass die Reihenfolge der Punkte eine Rolle spielt.
     * Es wird automatisch das obere linke und das untere rechte Ende des Rechtecks bestimmt.
     * @param[in] p1 Erster Punkt
     * @param[in] p2 Zweiter Punkt
     * @return Rechteck, das die beiden Punkte als gegenüberliegende Ecken hat
     */
    static Rect fromPoints(const Point& p1, const Point& p2);

    /** @brief Erstellt ein Rechteck aus den Koordinaten der oberen linken und unteren rechten Ecke
     *
     * Diese Funktion erstellt ein Rechteck aus den Koordinaten der oberen linken und unteren rechten Ecke,
     * wobei die unteren rechten Koordinaten "inklusiv" sind, also innerhalb des Rechtecks liegen.
     * Falls die Koordinaten nicht in der richtigen Reihenfolge angegeben werden, wird automatisch das obere linke
     * und das untere rechte Ende des Rechtecks bestimmt.
     *
     * @param[in] x1 X-Koordinate der oberen linken Ecke des Rechtecks
     * @param[in] y1 Y-Koordinate der oberen linken Ecke des Rechtecks
     * @param[in] x2 X-Koordinate der unteren rechten Ecke des Rechtecks. Dieser ist "inklusiv", liegt also innerhalb des Rechtecks
     * @param[in] y2 Y-Koordinate der unteren rechten Ecke des Rechtecks. Dieser ist "inklusiv", liegt also innerhalb des Rechtecks
     * @return Rechteck, das die angegebenen Koordinaten als obere linke und untere rechte Ecke hat
     */
    static Rect fromCoordsInclusive(int x1, int y1, int x2, int y2);
};

bool operator!=(const Rect& r1, const Rect& r2);
bool operator==(const Rect& r1, const Rect& r2);

/**@class Rect16
 * @brief Repräsentiert ein Rechteck in einem zweidimensionalen Koordinatensystem mittels 16-Bit-Ganzzahlen
 *
 * Ein Rechteck wird durch die Koordinaten der oberen linken Ecke (x, y) und die Breite (w) und Höhe (h) definiert.
 *
 * Nach Konvention werden die rechten und unteren Kanten des Rechtecks normalerweise als exklusiv betrachtet.
 * Mit anderen Worten liegt das Pixel mit den Koordinaten ( right, bottom ) unmittelbar außerhalb des
 * Rechtecks. Wenn zum Beispiel RECT an die Funktion FillRect übergeben wird, wird das Rechteck bis zur
 * (aber nicht einschließlich) rechten Spalte und der unteren Zeile von Pixeln gefüllt.
 */
class Rect16
{
    friend bool operator!=(const Rect16& r1, const Rect16& r2);
    friend bool operator==(const Rect16& r1, const Rect16& r2);

private:
    int16_t x, y;
    int16_t w, h;

public:
    /**@brief Standardkonstruktor, initialisiert alle Werte mit 0
     */
    Rect16();

    /**@brief Konstruktor, der zwei Punkte als gegenüberliegende Ecken des Rechtecks verwendet
     * Die Reihenfolge der Punkte spielt keine Rolle. Es wird automatisch das obere linke und das untere rechte
     * Ende des Rechtecks bestimmt.
     * @param[in] p1 Erster Punkt
     * @param[in] p2 Zweiter Punkt
     */
    Rect16(const Point16& p1, const Point16& p2);
    /**@brief Konstruktor, der einen Punkt als obere linke Ecke und eine Größe für Breite und Höhe verwendet
     * @param[in] p Punkt, der die obere linke Ecke des Rechtecks definiert
     * @param[in] s Größe des Rechtecks, die die Breite und Höhe angibt
     */
    Rect16(const Point16& p, const Size16& s);

    /**@brief Konstruktor, der die Koordinaten und die Größe des Rechtecks direkt angibt
     * @param[in] x X-Koordinate der oberen linken Ecke des Rechtecks
     * @param[in] y Y-Koordinate der oberen linken Ecke des Rechtecks
     * @param[in] width Breite des Rechtecks
     * @param[in] height Höhe des Rechtecks
     */
    Rect16(int16_t x, int16_t y, int16_t width, int16_t height);

    /**@brief Konstruktor, der ein Rect16-Objekt in ein Rect-Objekt konvertiert
     * @param[in] other Das Rect-Objekt, das konvertiert werden soll
     */
    Rect16(const Rect& r);

    /**@brief Rechteck hat keine Breite oder Höhe
     * @return Gibt true zurück, wenn Höche oder Breite 0 sind, andernfalls false.
     */
    bool isNull() const;
    //@brief Gibt die X-Koordinate der linken Seite des Rechtecks zurück.
    constexpr inline int16_t left() const
    {
        return x;
    };
    //@brief Gibt die X-Koordinate der rechten Seite des Rechtecks zurück. Liegt 1 Pixel außerhalb des Rechtecks
    constexpr inline int16_t right() const
    {
        return x + w;
    };
    //@brief Gibt die Y-Koordinate der oberen Seite des Rechtecks zurück.
    constexpr inline int16_t top() const
    {
        return y;
    };
    //@brief Gibt die Y-Koordinate der unteren Seite des Rechtecks zurück. Liegt 1 Pixel außerhalb des Rechtecks
    constexpr inline int16_t bottom() const
    {
        return y + h;
    };
    //@brief Gibt die X-Koordinate des letzten Pixels auf der rechten Seite des Rechtecks zurück.
    constexpr inline int16_t lastX() const
    {
        return x + w - 1;
    }
    //@brief Gibt die Y-Koordinate des letzten Pixels auf der unteren Seite des Rechtecks zurück.
    constexpr inline int16_t lastY() const
    {
        return y + h - 1;
    }

    //@brief Gibt die Breite des Rechtecks zurück.
    constexpr inline int16_t width() const
    {
        return w;
    };
    //@brief Gibt die Höhe des Rechtecks zurück.
    constexpr inline int16_t height() const
    {
        return h;
    };

    //@brief Gibt die Größe des Rechtecks als Size-Objekt zurück.
    inline Size16 size() const
    {
        return Size16(w, h);
    };

    //@brief Gibt die Koordinaten der oberen linken Ecke des Rechtecks als Point-Objekt zurück.
    inline Point16 topLeft() const
    {
        return Point16(x, y);
    };
    //@brief Gibt die Koordinaten der oberen rechten Ecke des Rechtecks als Point-Objekt zurück.
    inline Point16 topRight() const
    {
        return Point16(x + w, y);
    };
    //@brief Gibt die Koordinaten der unteren linken Ecke des Rechtecks als Point-Objekt zurück.
    inline Point16 bottomLeft() const
    {
        return Point16(x, y + h);
    };
    //@brief Gibt die Koordinaten der unteren rechten Ecke des Rechtecks als Point-Objekt zurück.
    inline Point16 bottomRight() const
    {
        return Point16(x + w, y + h);
    };

    //@brief Berechnet das Schnittrechteck zwischen diesem Rechteck und einem anderen Rechteck. Wenn die Rechtecke sich nicht schneiden,
    // wird ein Null-Rechteck zurückgegeben.
    Rect16 intersected(const Rect16& other) const;

    //@brief Überprüft, ob dieses Rechteck mit einem anderen Rechteck schneidet. Null-Rechtecke schneiden nicht.
    bool intersects(const Rect16& other) const;

    /**@brief Setzt die Koordinaten der oberen linken Ecke des Rechtecks.
     * @param[in] topLeft Punkt, der die neue obere linke Ecke des Rechtecks definiert
     */
    void setTopLeft(const Point16& topLeft);

    /**@brief Setzt die Koordinaten der unteren rechten Ecke des Rechtecks.
     * @param[in] bottomRight Punkt, der die neue untere rechte Ecke des Rechtecks definiert. Dieser liegt 1 Pixel außerhalb des Rechtecks
     */
    void setBottomRight(const Point16& bottomRight);

    /**@brief Setzt die Koordinaten und die Größe des Rechtecks anhand eines anderen Rechtecks.
     * @param[in] r Rechteck, dessen Koordinaten und Größe übernommen werden sollen
     */
    void setRect(const Rect& r);

    /**@brief Setzt die Koordinaten und die Größe des Rechtecks anhand eines anderen Rechtecks vom Typ Rect16.
     * @param[in] r Rechteck, dessen Koordinaten und Größe übernommen werden sollen
     */
    void setRect(const Rect16& r);

    /**@brief Setzt die Koordinaten und die Größe des Rechtecks anhand der Koordinaten der oberen linken Ecke und der Breite und Höhe.
     * @param[in] x X-Koordinate der oberen linken Ecke des Rechtecks
     * @param[in] y Y-Koordinate der oberen linken Ecke des Rechtecks
     * @param[in] width Breite des Rechtecks
     * @param[in] height Höhe des Rechtecks
     */

    void setRect(int16_t x, int16_t y, int16_t width, int16_t height);

    /**@brief Setzt die Koordinaten des Rechtecks anhand von zwei Punkten, die gegenüberliegende Ecken des Rechtecks bilden. Die Reihenfolge
     * der Punkte spielt keine Rolle.
     * @param[in] x1 X-Koordinate des ersten Punkts
     * @param[in] y1 Y-Koordinate des ersten Punkts
     * @param[in] x2 X-Koordinate des zweiten Punkts. Dieser ist "exklusiv" und liegt 1 Pixel außerhalb des Rechtecks
     * @param[in] y2 Y-Koordinate des zweiten Punkts. Dieser ist "exklusiv" und liegt 1 Pixel außerhalb des Rechtecks
     */
    void setCoords(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

    /**@brief Setzt die Koordinaten des Rechtecks anhand von zwei Punkten, die gegenüberliegende Ecken des Rechtecks bilden. Die Reihenfolge
     * der Punkte spielt keine Rolle.
     * @param[in] p1 Erster Punkt
     * @param[in] p2 Zweiter Punkt. Dieser ist "exklusiv" und liegt 1 Pixel außerhalb des Rechtecks
     */
    void setCoords(const Point16& p1, const Point16& p2);

    /**@brief Setzt die X-Koordinate der linken Seite des Rechtecks.
     * @param[in] left Neue X-Koordinate der linken Seite des Rechtecks
     */
    constexpr inline void setLeft(int16_t left)
    {
        x = left;
    }

    /**@brief Setzt die X-Koordinate der rechten Seite des Rechtecks.
     * @param[in] right Neue X-Koordinate der rechten Seite des Rechtecks. Dieser liegt 1 Pixel außerhalb des Rechtecks
     */
    void setRight(int16_t right);

    /**@brief Setzt die Y-Koordinate der oberen Seite des Rechtecks.
     * @param[in] top Neue Y-Koordinate der oberen Seite des Rechtecks
     */
    constexpr inline void setTop(int16_t top)
    {
        y = top;
    }

    /**@brief Setzt die Y-Koordinate der unteren Seite des Rechtecks.
     * @param[in] bottom Neue Y-Koordinate der unteren Seite des Rechtecks. Dieser liegt 1 Pixel außerhalb des Rechtecks
     */
    void setBottom(int16_t bottom);

    /**@brief Setzt die X-Koordinate der oberen linken Ecke des Rechtecks.
     * @param[in] x Neue X-Koordinate der oberen linken Ecke des Rechtecks
     */
    constexpr inline void setX(int16_t x)
    {
        this->x = x;
    }

    /**@brief Setzt die Y-Koordinate der oberen linken Ecke des Rechtecks.
     * @param[in] y Neue Y-Koordinate der oberen linken Ecke des Rechtecks
     */
    constexpr inline void setY(int16_t y)
    {
        this->y = y;
    }

    /**@brief Setzt die Größe des Rechtecks.
     * @param[in] size Neue Größe des Rechtecks, die die Breite und Höhe angibt
     */
    constexpr inline void setSize(const Size16& size)
    {
        w = size.width;
        h = size.height;
    }

    /**@brief Setzt die Breite des Rechtecks.
     * @param[in] width Neue Breite des Rechtecks
     */
    constexpr inline void setWidth(int16_t width)
    {
        w = width;
    }

    /**@brief Setzt die Höhe des Rechtecks.
     * @param[in] height Neue Höhe des Rechtecks
     */
    constexpr inline void setHeight(int16_t height)
    {
        h = height;
    }

    /**@brief Zuweisungsoperator, der ein Rect-Objekt in ein Rect16-Objekt konvertiert
     * @param[in] other Das Rect-Objekt, das konvertiert und zugewiesen werden soll
     * @return Referenz auf das aktualisierte Rect-Objekt
     */
    Rect16& operator=(const Rect& other);

    /**@brief Erstellt ein Rechteck aus zwei Punkten, deren Reihenfolge egal ist
     * Diese Funktion erstellt ein Rechteck aus zwei Punkten, ohne dass die Reihenfolge der Punkte eine Rolle spielt.
     * Es wird automatisch das obere linke und das untere rechte Ende des Rechtecks bestimmt.
     * @param[in] p1 Erster Punkt
     * @param[in] p2 Zweiter Punkt, die Koordinaten sind "exklusiv" und liegen 1 Pixel außerhalb des Rechtecks
     * @return Rechteck, das die beiden Punkte als gegenüberliegende Ecken hat
     */
    static Rect16 fromPoints(const Point16& p1, const Point16& p2);

    /** @brief Erstellt ein Rechteck aus den Koordinaten der oberen linken und unteren rechten Ecke
     *
     * Diese Funktion erstellt ein Rechteck aus den Koordinaten der oberen linken und unteren rechten Ecke,
     * wobei die unteren rechten Koordinaten "inklusiv" sind, also innerhalb des Rechtecks liegen.
     * Falls die Koordinaten nicht in der richtigen Reihenfolge angegeben werden, wird automatisch das obere linke
     * und das untere rechte Ende des Rechtecks bestimmt.
     *
     * @param[in] x1 X-Koordinate der oberen linken Ecke des Rechtecks
     * @param[in] y1 Y-Koordinate der oberen linken Ecke des Rechtecks
     * @param[in] x2 X-Koordinate der unteren rechten Ecke des Rechtecks. Dieser ist "inklusiv", liegt also innerhalb des Rechtecks
     * @param[in] y2 Y-Koordinate der unteren rechten Ecke des Rechtecks. Dieser ist "inklusiv", liegt also innerhalb des Rechtecks
     * @return Rechteck, das die angegebenen Koordinaten als obere linke und untere rechte Ecke hat
     */
    static Rect16 fromCoordsInclusive(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
};

bool operator!=(const Rect16& r1, const Rect16& r2);
bool operator==(const Rect16& r1, const Rect16& r2);

} // namespace pplib::grafix

#endif // PPLIB_INCLUDE_GRAFIX_RECT_H
