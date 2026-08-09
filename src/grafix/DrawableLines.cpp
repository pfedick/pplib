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

#include <stdint.h>
#include <pplib/grafix/drawable.h>

namespace pplib::grafix
{

/* signum function */
static int sgn(int x)
{
    return (x > 0) ? 1 : (x < 0) ? -1 : 0;
}

/* Bresenham Algorithmus */
static void Line_32(const DrawableData& data, int xstart, int ystart, int xend, int yend, SurfaceColor color)
{
    int x, y, t, dx, dy, incx, incy, pdx, pdy, ddx, ddy, es, el, err;

    /* Entfernung in beiden Dimensionen berechnen */
    dx = xend - xstart;
    dy = yend - ystart;

    /* Vorzeichen des Inkrements bestimmen */
    incx = sgn(dx);
    incy = sgn(dy);
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    /* feststellen, welche Entfernung größer ist */
    if (dx > dy) {
        /* x ist schnelle Richtung */
        pdx = incx;
        pdy = 0; /* pd. ist Parallelschritt */
        ddx = incx;
        ddy = incy; /* dd. ist Diagonalschritt */
        es = dy;
        el = dx; /* Fehlerschritte schnell, langsam */
    } else {
        /* y ist schnelle Richtung */
        pdx = 0;
        pdy = incy; /* pd. ist Parallelschritt */
        ddx = incx;
        ddy = incy; /* dd. ist Diagonalschritt */
        es = dx;
        el = dy; /* Fehlerschritte schnell, langsam */
    }

    /* Initialisierungen vor Schleifenbeginn */
    x = xstart;
    y = ystart;
    err = el / 2;
    data.fn->PutPixel(data, x, y, color);

    /* Pixel berechnen */
    for (t = 0; t < el; ++t) /* t zaehlt die Pixel, el ist auch Anzahl */
    {
        /* Aktualisierung Fehlerterm */
        err -= es;
        if (err < 0) {
            /* Fehlerterm wieder positiv (>=0) machen */
            err += el;
            /* Schritt in langsame Richtung, Diagonalschritt */
            x += ddx;
            y += ddy;
        } else {
            /* Schritt in schnelle Richtung, Parallelschritt */
            x += pdx;
            y += pdy;
        }
        data.fn->PutPixel(data, x, y, color);
    }
} /* gbham() */

static void SwapFloat(float* w1, float* w2)
{
    float t;
    t = *w1;
    *w1 = *w2;
    *w2 = t;
}

static float WuTrunc(float value) // Ganzzahligen Wert von Value zur�ckgeben
{
    return (float)((int32_t)value);
}

static float WuFrac(float value) // Kommastellen zurueckgeben
{
    return value - WuTrunc(value);
}

static float WuInvFrac(float value)
{
    return 1 - WuFrac(value);
}

static void WuLine(const DrawableData& data, float x1, float y1, float x2, float y2, SurfaceColor color)
{
    float grad, xd, yd; //,length,xm,ym;
    float brightness1, brightness2;

    xd = (x2 - x1); // Breite und Hoehe der Linie
    yd = (y2 - y1);

    if (abs((int32_t)xd) > abs((int32_t)yd)) { // check line gradient							==> Horizontale Linie
        if (x1 > x2) {                         // Wenn Linie von rechts nach links gezeichnet wird, tauschen
            SwapFloat(&x1, &x2);               // wir einfach die Koordinaten
            SwapFloat(&y1, &y2);
            xd = (x2 - x1); // Breite und Hoehe der Linie neu berechnen
            yd = (y2 - y1);
        }
        grad = yd / xd; // Gradient der Linie
        float xgap, xend, yend, yf;

        // End Point 1
        xend = WuTrunc(x1 + 0.5f);
        yend = y1 + grad * (xend - x1);

        xgap = WuInvFrac(x1 + 0.5f);

        int32_t ix1 = (int32_t)xend;
        int32_t iy1 = (int32_t)yend;

        brightness1 = WuInvFrac(yend) * xgap;
        brightness2 = WuFrac(yend) * xgap;
        data.fn->BlendPixel(data, ix1, iy1, color, (int)(brightness1 * 255));
        data.fn->BlendPixel(data, ix1, iy1 + 1, color, (int)(brightness2 * 255));

        yf = yend + grad;

        // End Point 2
        xend = WuTrunc(x2 + 0.5f);
        yend = y2 + grad * (xend - x2);

        xgap = WuInvFrac(x2 - 0.5f);

        int32_t ix2 = (int32_t)xend;
        int32_t iy2 = (int32_t)yend;

        brightness1 = WuInvFrac(yend) * xgap;
        brightness2 = WuFrac(yend) * xgap;
        data.fn->BlendPixel(data, ix2, iy2, color, (int)(brightness1 * 255));
        data.fn->BlendPixel(data, ix2, iy2 + 1, color, (int)(brightness2 * 255));

        // Main Loop
        for (int32_t x = ix1 + 1; x < ix2; x++) {
            brightness1 = WuInvFrac(yf);
            brightness2 = WuFrac(yf);
            data.fn->BlendPixel(data, x, (int)yf, color, (int)(brightness1 * 255));
            data.fn->BlendPixel(data, x, (int)yf + 1, color, (int)(brightness2 * 255));
            yf = yf + grad;
        }

    } else {                     // check line gradient							==> Vertikale Linie
        if (y1 > y2) {           // Wenn Linie von rechts nach links gezeichnet wird, tauschen
            SwapFloat(&x1, &x2); // wir einfach die Koordinaten
            SwapFloat(&y1, &y2);
            xd = (x2 - x1); // Breite und Hoehe der Linie neu berechnen
            yd = (y2 - y1);
        }
        grad = xd / yd; // Gradient der Linie

        float xend, yend, xf, ygap;
        // End Point 1
        yend = WuTrunc(y1 + 0.5f);
        xend = x1 + grad * (yend - y1);

        ygap = WuInvFrac(y1 + 0.5f);

        int32_t ix1 = (int32_t)xend;
        int32_t iy1 = (int32_t)yend;

        brightness1 = WuInvFrac(xend) * ygap;
        brightness2 = WuFrac(xend) * ygap;
        data.fn->BlendPixel(data, ix1, iy1, color, (int)(brightness1 * 255));
        data.fn->BlendPixel(data, ix1 + 1, iy1, color, (int)(brightness2 * 255));

        xf = xend + grad;

        // End Point 2
        yend = WuTrunc(y2 + 0.5f);
        xend = x2 + grad * (yend - y2);

        ygap = WuInvFrac(y2 - 0.5f);

        int32_t ix2 = (int32_t)xend;
        int32_t iy2 = (int32_t)yend;

        brightness1 = WuInvFrac(xend) * ygap;
        brightness2 = WuFrac(xend) * ygap;
        data.fn->BlendPixel(data, ix2, iy2, color, (int)(brightness1 * 255));
        data.fn->BlendPixel(data, ix2 + 1, iy2, color, (int)(brightness2 * 255));

        // Main Loop
        for (int32_t y = iy1 + 1; y < iy2; y++) {
            brightness1 = WuInvFrac(xf);
            brightness2 = WuFrac(xf);
            data.fn->BlendPixel(data, (int)xf, y, color, (int)(brightness1 * 255));
            data.fn->BlendPixel(data, (int)xf + 1, y, color, (int)(brightness2 * 255));
            xf = xf + grad;
        }
    }
}

static void WuLineThick(const DrawableData& data, float x1, float y1, float x2, float y2, SurfaceColor color, int strength)
{
    float grad, xd, yd; //,length,xm,ym;
    float brightness1, brightness2;

    xd = (x2 - x1); // Breite und Hoehe der Linie
    yd = (y2 - y1);

    if (abs((int32_t)xd) > abs((int32_t)yd)) { // check line gradient							==> Horizontale Linie
        // Zuerst korrigieren wir die Start- und Zielkoordinaten, damit die Linie mittig ist
        y1 -= strength / 2;
        y2 -= strength / 2;

        if (x1 > x2) {           // Wenn Linie von rechts nach links gezeichnet wird, tauschen
            SwapFloat(&x1, &x2); // wir einfach die Koordinaten
            SwapFloat(&y1, &y2);
            xd = (x2 - x1); // Breite und Hoehe der Linie neu berechnen
            yd = (y2 - y1);
        }
        grad = yd / xd; // Gradient der Linie
        float xgap, xend, yend, yf;
        // End Point 1
        xend = WuTrunc(x1 + 0.5f);
        yend = y1 + grad * (xend - x1);

        xgap = WuInvFrac(x1 + 0.5f);

        int32_t ix1 = (int32_t)xend;
        int32_t iy1 = (int32_t)yend;

        brightness1 = WuInvFrac(yend) * xgap;
        brightness2 = WuFrac(yend) * xgap;
        data.fn->BlendPixel(data, ix1, iy1, color, (int)(brightness1 * 255));
        for (int i = 1; i < strength; i++)
            data.fn->BlendPixel(data, ix1, iy1 + i, color, 255);
        data.fn->BlendPixel(data, ix1, iy1 + strength, color, (int)(brightness2 * 255));

        yf = yend + grad;

        // End Point 2
        xend = WuTrunc(x2 + 0.5f);
        yend = y2 + grad * (xend - x2);

        xgap = WuInvFrac(x2 - 0.5f);

        int32_t ix2 = (int32_t)xend;
        int32_t iy2 = (int32_t)yend;

        brightness1 = WuInvFrac(yend) * xgap;
        brightness2 = WuFrac(yend) * xgap;
        data.fn->BlendPixel(data, ix2, iy2, color, (int)(brightness1 * 255));
        for (int i = 1; i < strength; i++)
            data.fn->BlendPixel(data, ix2, iy2 + i, color, 255);
        data.fn->BlendPixel(data, ix2, iy2 + strength, color, (int)(brightness2 * 255));

        // Main Loop
        for (int32_t x = ix1 + 1; x < ix2; x++) {
            brightness1 = WuInvFrac(yf);
            brightness2 = WuFrac(yf);
            data.fn->BlendPixel(data, x, (int32_t)yf, color, (int)(brightness1 * 255));
            for (int i = 1; i < strength; i++)
                data.fn->BlendPixel(data, x, (int32_t)yf + i, color, 255);
            data.fn->BlendPixel(data, x, (int32_t)yf + strength, color, (int)(brightness2 * 255));
            yf = yf + grad;
        }

    } else { // check line gradient							==> Vertikale Linie
        // Zuerst korrigieren wir die Start- und Zielkoordinaten, damit die Linie mittig ist
        x1 -= strength / 2;
        x2 -= strength / 2;

        if (y1 > y2) {           // Wenn Linie von rechts nach links gezeichnet wird, tauschen
            SwapFloat(&x1, &x2); // wir einfach die Koordinaten
            SwapFloat(&y1, &y2);
            xd = (x2 - x1); // Breite und Hoehe der Linie neu berechnen
            yd = (y2 - y1);
        }
        grad = xd / yd; // Gradient der Linie
        float xend, yend, xf, ygap;
        // End Point 1
        yend = WuTrunc(y1 + 0.5f);
        xend = x1 + grad * (yend - y1);

        ygap = WuInvFrac(y1 + 0.5f);

        int32_t ix1 = (int32_t)xend;
        int32_t iy1 = (int32_t)yend;

        brightness1 = WuInvFrac(xend) * ygap;
        brightness2 = WuFrac(xend) * ygap;
        data.fn->BlendPixel(data, ix1, iy1, color, (int)(brightness1 * 255));
        for (int i = 1; i < strength; i++)
            data.fn->BlendPixel(data, ix1 + i, iy1, color, 255);
        data.fn->BlendPixel(data, ix1 + strength, iy1, color, (int)(brightness2 * 255));

        xf = xend + grad;

        // End Point 2
        yend = WuTrunc(y2 + 0.5f);
        xend = x2 + grad * (yend - y2);

        ygap = WuInvFrac(y2 - 0.5f);

        int32_t ix2 = (int32_t)xend;
        int32_t iy2 = (int32_t)yend;

        brightness1 = WuInvFrac(xend) * ygap;
        brightness2 = WuFrac(xend) * ygap;
        data.fn->BlendPixel(data, ix2, iy2, color, (int)(brightness1 * 255));
        for (int i = 1; i < strength; i++)
            data.fn->BlendPixel(data, ix2 + i, iy2, color, 255);
        data.fn->BlendPixel(data, ix2 + strength, iy2, color, (int)(brightness2 * 255));

        // Main Loop
        for (int32_t y = iy1 + 1; y < iy2; y++) {
            brightness1 = WuInvFrac(xf);
            brightness2 = WuFrac(xf);
            data.fn->BlendPixel(data, (int32_t)xf, y, color, (int)(brightness1 * 255));
            for (int i = 1; i < strength; i++)
                data.fn->BlendPixel(data, (int32_t)xf + i, y, color, 255);
            data.fn->BlendPixel(data, (int32_t)xf + strength, y, color, (int)(brightness2 * 255));
            xf = xf + grad;
        }
    }
}

static void LineAA(DrawableData& data, int x1, int y1, int x2, int y2, SurfaceColor color, int strength)
{
    if (strength == 1) {
        WuLine(data, (float)x1, (float)y1, (float)x2, (float)y2, color);
    } else {
        WuLineThick(data, (float)x1, (float)y1, (float)x2, (float)y2, color, strength);
    }
    return;
}

void Drawable::line(int x1, int y1, int x2, int y2, const Color& c)
{
    SurfaceColor native_color = data.fn->ToNativeColor(c);
    Line_32(data, x1, y1, x2, y2, native_color);
}

void Drawable::line(const Point& start, const Point& end, const Color& c)
{
    line(start.x, start.y, end.x, end.y, c);
}

void Drawable::lineAA(int x1, int y1, int x2, int y2, const Color& c, int strength)
{
    SurfaceColor native_color = data.fn->ToNativeColor(c);
    LineAA(data, x1, y1, x2, y2, native_color, strength);
}

void Drawable::lineAA(const Point& start, const Point& end, const Color& c, int strength)
{
    lineAA(start.x, start.y, end.x, end.y, c, strength);
}

} // namespace pplib::grafix