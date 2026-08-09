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

#include <string.h>
#include <config_pplib.h>
#include <pplib/grafix/drawable.h>
#include <pplib/grafix/image.h>
#include <pplib/grafix/grafix.h>
#include <pplib/grafix/imagereference.h>
#include <pplib/types/string.h>
#include <pplib/exceptions.h>

/*!\brief Blitting-Funktionen initialisieren
 *
 * \desc
 * Mit dieser Funktion werden die Blitting-Funktionen in Abhängigkeit des Farbformates
 * der Oberfläche initialisiert.
 *
 * Blitting-Funktionen (oder kurz "Blt") sind Funktionen, mit denen Rechteckige Grafiken - oder
 * auch nur Teile davon - in eine andere Grafik kopiert werden. Dabei wird unterschieden, ob der
 * Inhalt ohne Prüfung 1:1 kopiert wird (CSurface::Blt), ein bestimmte Farbe transparent
 * sein soll (CSurface::BltColorKey), der Alphakanal der Quellgrafik verwendet werden
 * soll (CSurface::AlphaBlt oder CSurface::DrawSprite) oder die Intensität eines
 * Schwarz-Weiss-Bildes verwendet wird, um eine bestimmte Farbe zu zeichnen (CSurface::BltDiffuse).
 *
 * @param[in] s Pointer auf die SURFACE-Struktur der Oberfläche.
 * \exception UnsupportedColorFormatException Wird geworfen, wenn das Farbformat \p format
 * nicht unterstützt wird.
 *
 * \remarks
 * Gegenwärtig werden nur Farbformate mit einer Tiefe von 32 Bit unterstützt.
 *
 */
void Grafix::initBlits(const RGBFormat& format, DRAWABLE_FUNCTIONS* fn) noexcept
{
    switch (format) {
    case RGBFormat::A8R8G8B8: // 32 Bit True Color
    case RGBFormat::A8B8G8R8:
    case RGBFormat::X8B8G8R8:
    case RGBFormat::X8R8G8B8:
        fn->Blt = Blt_32;
        fn->BltAlpha = BltAlpha_32;
        fn->BltAlphaMod = BltAlphaMod_32;
        fn->BltColorKey = BltColorKey_32;
        fn->BltDiffuse = BltDiffuse_32;
        fn->BltBlend = BltBlend_32;
        fn->BltChromaKey = BltChromaKey_32;
        fn->BltBackgoundOnChromaKey = BltBackgroundOnChromaKey_32;
        return;
    case RGBFormat::GREY8:
    case RGBFormat::A8:
        return;
    }
}

/*!\brief Überprüft, ob eine Blit-Aktion in den Zeichenbereich passt.
 *
 * \desc
 * Diese Funktion prüft, ob das zu zeichnende Rechteck überhaupt in die aktuelle
 * Zeichenfläche passt. Dabei wird das Quellrechteck bei Bedarf angepasst.
 *
 * \param[in,out] x X-Koordinate der Zielposition
 * \param[in,out] y Y-Koordinate der Zielposition
 * \param[in,out] r Quell-Rechteck
 *
 * \return
 * Die Funktion liefert 0 zurück, wenn das Rechteck komplett ausserhalb der
 * Zeichenfläche liegt, oder 1, wenn es ganz oder zumindest teilweise innerhalb der
 * Zeichenfläche liegt. In letzterem Fall werden die Koordinaten \p x, \p y und die
 * Dimensionen des Rechtecks \p r so angepasst, dass durch die nachfolgende Blt-Funktion
 * nur der sichtbare Bereich an die korrekte Position gezeichnet wird.
 */
int Drawable::fitRect(int& x, int& y, Rect& r)
{
    Rect screen(0, 0, data.width, data.height);
    Rect object(x, y, r.width(), r.height());
    Rect i = screen.intersected(object);

    if (i.isNull()) return 0;
    int32_t shiftx = i.left() - object.left();
    int32_t shifty = i.top() - object.top();
    x += shiftx;
    y += shifty;

    r.setLeft(r.left() + shiftx);
    r.setTop(r.top() + shifty);
    r.setWidth(i.width());
    r.setHeight(i.height());
    return 1;
}

/*!\brief Rechteck 1:1 kopieren
 *
 * \desc
 * Mit dieser Funktion wird die Quellzeichenfläche \p source
 * an die Position \p x / \p y der Zielzeichenfläche kopiert, wobei alle Farbinformationen 1:1 übernommen werden.
 * Es wird weder Alphablending (siehe Drawable::bltAlpha) noch Colorkeying (siehe
 * Drawable::bltColorKey) verwendet.
 * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert
 * (siehe Drawable::fitRect). Falls die Quelle komplett außerhalb der Zeichenfläche liegt,
 * passiert nichts.
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] x Optionale X-Koordinate der linken oberen Ecke in der Zielzeichenfläche. Wird der Parameter nicht
 *            angegeben, wird 0 verwendet.
 * \param[in] y Optionale Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche. Wird der Parameter
 *            nicht angegebenm wird 0 verwendet.
 *
 * \exception EmptyDrawableException Der Parameter \p source enthält keinen darstellbaren Inhalt
 * \exception FunctionUnavailableException Funktion wird für das eingestellte Farbformat nicht unterstützt
 */
void Drawable::blt(const Drawable& source, int x, int y)
{
    blt(source, source.rect(), x, y);
}

/*!\brief Rechteck 1:1 kopieren
 *
 * \desc
 * Mit dieser Funktion wird der Ausschnitt \p srect aus der Quellzeichenfläche \p source
 * an die Position \p x / \p y kopiert, wobei alle Farbinformationen 1:1 übernommen werden.
 * Es wird weder Alphablending (siehe Drawable::bltAlpha) noch Colorkeying (siehe
 * Drawable::bltColorKey) verwendet. Falls \p srect 0 ist, wird die komplette Quellzeichenfläche kopiert,
 * andernfalls nur der angegebene Ausschnitt.
 * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert
 * (siehe Drawable::fitRect). Falls die Quelle komplett außerhalb der Zeichenfläche liegt,
 * passiert nichts.
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] srect Rechteckiger Ausschnitt aus der Quellzeichenfläche, der kopiert werden soll
 * \param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 *
 * \exception EmptyDrawableException Der Parameter \p source enthält keinen darstellbaren Inhalt
 * \exception FunctionUnavailableException Funktion wird für das eingestellte Farbformat nicht unterstützt
 */
void Drawable::blt(const Drawable& source, const Rect& srect, int x, int y)
{
    if (source.isEmpty() || !data.base) return;
    // Quellrechteck
    Rect q;
    if (srect.isNull()) {
        q = source.rect();
    } else {
        q = srect;
        if (q.left() < 0) q.setLeft(0);
        if (q.width() > source.width()) q.setWidth(source.width());
        if (q.top() < 0) q.setTop(0);
        if (q.height() > source.height()) q.setHeight(source.height());
    }
    //::printf ("rect=(%i/%i)-(%i/%i)\n", q.x1, q.y1, q.x2, q.y2);
    if (!fitRect(x, y, q)) return;
    //::printf ("rect=(%i/%i)-(%i/%i)\n", q.x1, q.y1, q.x2, q.y2);
    data.fn->Blt(data, source.data, q, x, y);
}

void Drawable::bltDiffuse(const Drawable& source, int x, int y, const Color& c)
/*!\brief Rechteck anhand der Intensität der Quellfarbe kopieren
 *
 * \desc
 * Mit dieser Funktion wird die Quellzeichenfläche \p source
 * an die Position \p x / \p y kopiert, wobei die Intensität der Quellpixel geprüft wird und
 * diese in gleicher Intensität mit der angegebenen Farbe \c gezeichnet werden. Bei
 * halbtransparenten Pixeln wird die Farbe mit dem Hintergrund gemischt. Die Funktion ist daher
 * zum Zeichnen von einfarbigen Grafiken unterschiedlicher Intensität gedacht (z.B. grafische Elemente
 * einer GUI).
 * \par
 * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert
 * (siehe Drawable::fitRect). Falls die Quelle komplett außerhalb der Zeichenfläche liegt,
 * passiert nichts.
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] c Die gewünschte Pixelfarbe
 *
 * \exception EmptyDrawableException Der Parameter \p source enthält keinen darstellbaren Inhalt
 * \exception FunctionUnavailableException Funktion wird für das eingestellte Farbformat nicht unterstützt
 *
 */
{
    bltDiffuse(source, source.rect(), x, y, c);
}

/*!\brief Rechteck anhand der Intensität der Quellfarbe kopieren
 *
 * \desc
 * Mit dieser Funktion wird der Ausschnitt \p srect aus der Quellzeichenfläche \p source
 * an die Position \p x / \p y kopiert, wobei die Intensität der Quellpixel geprüft wird und
 * diese in gleicher Intensität mit der angegebenen Farbe \c gezeichnet werden. Bei
 * halbtransparenten Pixeln wird die Farbe mit dem Hintergrund gemischt. Die Funktion ist daher
 * zum Zeichnen von einfarbigen Grafiken unterschiedlicher Intensität gedacht (z.B. grafische Elemente
 * einer GUI).
 * \par
 * Falls \p srect 0 ist, wird die komplette Quellzeichenfläche kopiert, andernfalls nur der angegebene Ausschnitt.
 * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert
 * (siehe Drawable::fitRect). Falls die Quelle komplett außerhalb der Zeichenfläche liegt,
 * passiert nichts.
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] srect Rechteckiger Ausschnitt aus der Quellzeichenfläche, der kopiert werden soll
 * \param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] c Die gewünschte Pixelfarbe
 *
 * \exception EmptyDrawableException Der Parameter \p source enthält keinen darstellbaren Inhalt
 * \exception FunctionUnavailableException Funktion wird für das eingestellte Farbformat nicht unterstützt
 *
 */
void Drawable::bltDiffuse(const Drawable& source, const Rect& srect, int x, int y, const Color& c)
{
    if (source.isEmpty() || !data.base) return;
    // Quellrechteck
    Rect q;
    if (srect.isNull()) {
        q = source.rect();
    } else {
        q = srect;
        if (q.left() < 0) q.setLeft(0);
        if (q.width() > source.width()) q.setWidth(source.width());
        if (q.top() < 0) q.setTop(0);
        if (q.height() > source.height()) q.setHeight(source.height());
    }
    if (!fitRect(x, y, q)) return;
    data.fn->BltDiffuse(data, source.data, q, x, y, toNativeColor(c));
}

/*!\brief Rechteck unter Berücksichtigung einer transparenten Schlüsselfarbe kopieren
 *
 * \desc
 * Mit dieser Funktion wird die Quellzeichenfläche \p source
 * an die Position \p x / \p y unter Berücksichtigung der Schlüsselfarbe \p c kopiert.
 * Pixel, die der Farbe \c entsprechen, bleiben dabei vollständig transparent, alle anderen
 * Pixel werden wie bei Drawable::blt 1:1 kopiert.
 * \par
 * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert
 * (siehe Drawable::fitRect). Falls die Quelle komplett außerhalb der Zeichenfläche liegt,
 * passiert nichts.
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] c Die gewünschte Schlüsselfarbe (ColorKey)
 *
 * \exception EmptyDrawableException Der Parameter \p source enthält keinen darstellbaren Inhalt
 * \exception FunctionUnavailableException Funktion wird für das eingestellte Farbformat nicht unterstützt
 */
void Drawable::bltColorKey(const Drawable& source, int x, int y, const Color& c)
{
    bltColorKey(source, source.rect(), x, y, c);
}

/*!\brief Rechteck unter Berücksichtigung einer transparenten Schlüsselfarbe kopieren
 *
 * \desc
 * Mit dieser Funktion wird der Ausschnitt \p srect aus der Quellzeichenfläche \p source
 * an die Position \p x / \p y unter Berücksichtigung der Schlüsselfarbe \p c kopiert.
 * Pixel, die der Farbe \c entsprechen, bleiben dabei vollständig transparent, alle anderen
 * Pixel werden wie bei Drawable::blt 1:1 kopiert.
 * \par
 * Falls \p srect 0 ist, wird die komplette Quellzeichenfläche kopiert, andernfalls nur der angegebene Ausschnitt.
 * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert
 * (siehe Drawable::fitRect). Falls die Quelle komplett außerhalb der Zeichenfläche liegt,
 * passiert nichts.
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] srect Rechteckiger Ausschnitt aus der Quellzeichenfläche, der kopiert werden soll
 * \param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] c Die gewünschte Schlüsselfarbe (ColorKey)
 *
 * \exception EmptyDrawableException Der Parameter \p source enthält keinen darstellbaren Inhalt
 * \exception FunctionUnavailableException Funktion wird für das eingestellte Farbformat nicht unterstützt
 */
void Drawable::bltColorKey(const Drawable& source, const Rect& srect, int x, int y, const Color& c)
{
    if (source.isEmpty() || !data.base) return;
    // Quellrechteck
    Rect q;
    if (srect.isNull()) {
        q = source.rect();
    } else {
        q = srect;
        if (q.left() < 0) q.setLeft(0);
        if (q.width() > source.width()) q.setWidth(source.width());
        if (q.top() < 0) q.setTop(0);
        if (q.height() > source.height()) q.setHeight(source.height());
    }
    if (!fitRect(x, y, q)) return;
    data.fn->BltColorKey(data, source.data, q, x, y, toNativeColor(c));
}

/*!\brief Rechteck unter Berücksichtigung des Alpha-Kanals kopieren
 *
 * \desc
 * Mit dieser Funktion wird die Quellzeichenfläche \p source
 * an die Position \p x / \p y unter Berücksichtigung des Alphakanals der Quelle kopiert.
 * Der Alphakanal bestimmt die Transparenz eines Pixels. Ist sie 0, wird der Pixel nicht
 * kopiert, bei einem Wert von 255 wird er 1:1 kopiert. Dazwischen wird die Farbe abhängig
 * vom Transparenz-Wert mit dem Hintergrund vermischt.
 * \par
 * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert
 * (siehe Drawable::fitRect). Falls die Quelle komplett außerhalb der Zeichenfläche liegt,
 * passiert nichts.
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] c Die gewünschte Schlüsselfarbe (ColorKey)
 *
 * \exception EmptyDrawableException Der Parameter \p source enthält keinen darstellbaren Inhalt
 * \exception FunctionUnavailableException Funktion wird für das eingestellte Farbformat nicht unterstützt
 */
void Drawable::bltAlpha(const Drawable& source, int x, int y)
{
    bltAlpha(source, source.rect(), x, y);
}

/*!\brief Rechteck unter Berücksichtigung des Alpha-Kanals kopieren
 *
 * \desc
 * Mit dieser Funktion wird der Ausschnitt \p srect aus der Quellzeichenfläche \p source
 * an die Position \p x / \p y unter Berücksichtigung des Alphakanals der Quelle kopiert.
 * Der Alphakanal bestimmt die Transparenz eines Pixels. Ist sie 0, wird der Pixel nicht
 * kopiert, bei einem Wert von 255 wird er 1:1 kopiert. Dazwischen wird die Farbe abhängig
 * vom Transparenz-Wert mit dem Hintergrund vermischt.
 * \par
 * Falls \p srect 0 ist, wird die komplette Quellzeichenfläche kopiert, andernfalls nur der angegebene Ausschnitt.
 * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert
 * (siehe Drawable::fitRect). Falls die Quelle komplett außerhalb der Zeichenfläche liegt,
 * passiert nichts.
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] srect Rechteckiger Ausschnitt aus der Quellzeichenfläche, der kopiert werden soll
 * \param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] c Die gewünschte Schlüsselfarbe (ColorKey)
 *
 * \exception EmptyDrawableException Der Parameter \p source enthält keinen darstellbaren Inhalt
 * \exception FunctionUnavailableException Funktion wird für das eingestellte Farbformat nicht unterstützt
 */
void Drawable::bltAlpha(const Drawable& source, const Rect& srect, int x, int y)
{
    if (source.isEmpty() || !data.base) return;
    // Quellrechteck
    Rect q;
    if (srect.isNull()) {
        q = source.rect();
    } else {
        q = srect;
        if (q.left() < 0) q.setLeft(0);
        if (q.width() > source.width()) q.setWidth(source.width());
        if (q.top() < 0) q.setTop(0);
        if (q.height() > source.height()) q.setHeight(source.height());
    }
    if (!fitRect(x, y, q)) return;
    data.fn->BltAlpha(data, source.data, q, x, y);
}

void Drawable::bltAlphaMod(const Drawable& source, const Color& mod, int x, int y)
{
    bltAlphaMod(source, source.rect(), x, y);
}

void Drawable::bltAlphaMod(const Drawable& source, const Rect& srect, const Color& mod, int x, int y)
{
    if (source.isEmpty() || !data.base) return;
    // Quellrechteck
    Rect q;
    if (srect.isNull()) {
        q = source.rect();
    } else {
        q = srect;
        if (q.left() < 0) q.setLeft(0);
        if (q.width() > source.width()) q.setWidth(source.width());
        if (q.top() < 0) q.setTop(0);
        if (q.height() > source.height()) q.setHeight(source.height());
    }
    if (!fitRect(x, y, q)) return;
    data.fn->BltAlphaMod(data, source.data, q, toNativeColor(mod), x, y);
}

/*!\brief Grafik aus einer Image-Liste kopieren
 *
 * \desc
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
void Drawable::draw(const ImageList& iml, int nr, int x, int y)
{
    if (!data.base) return;
    Rect r = iml.getRect(nr);
    switch ((int)iml.method) {
    case ImageList::BLT:
        blt(iml, r, x, y);
        return;
    case ImageList::ALPHABLT:
        bltAlpha(iml, r, x, y);
        return;
    case ImageList::COLORKEY:
        bltColorKey(iml, r, x, y, iml.colorkey);
        return;
    case ImageList::DIFFUSE:
        bltDiffuse(iml, r, x, y, iml.diffuse);
        return;
    }
}

/*!\brief Grafik aus einer Image-Liste kopieren
 *
 * \desc
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
void Drawable::draw(const ImageList& iml, int nr, int x, int y, const Color& diffuse)
{
    if (!data.base) return;
    Rect r = iml.getRect(nr);
    switch ((int)iml.method) {
    case ImageList::BLT:
        blt(iml, r, x, y);
        return;
    case ImageList::ALPHABLT:
        bltAlpha(iml, r, x, y);
        return;
    case ImageList::COLORKEY:
        bltColorKey(iml, r, x, y, iml.colorkey);
        return;
    case ImageList::DIFFUSE:
        bltDiffuse(iml, r, x, y, diffuse);
        return;
    }
}

void Drawable::bltBlend(const Drawable& source, float factor, int x, int y)
{
    bltBlend(source, factor, source.rect(), x, y);
}

void Drawable::bltBlend(const Drawable& source, float factor, const Rect& srect, int x, int y)
{
    if (source.isEmpty() || !data.base) return;
    if (factor <= 0.0f) return;
    if (factor >= 1.0f) {
        blt(source, srect, x, y);
        return;
    }
    // Quellrechteck
    Rect q;
    if (srect.isNull()) {
        q = source.rect();
    } else {
        q = srect;
        if (q.left() < 0) q.setLeft(0);
        if (q.width() > source.width()) q.setWidth(source.width());
        if (q.top() < 0) q.setTop(0);
        if (q.height() > source.height()) q.setHeight(source.height());
    }
    //::printf ("rect=(%i/%i)-(%i/%i)\n", q.x1, q.y1, q.x2, q.y2);
    if (!fitRect(x, y, q)) return;
    data.fn->BltBlend(data, source.data, q, x, y, factor);
}

void Drawable::draw(const ImageReference& imgref, int x, int y)
{
    if (!data.base) return;
    switch (imgref.draw_method) {
    case DrawMethod::BLT:
        blt(imgref.pixel, imgref.pixel.rect(), x, y);
        return;
    case DrawMethod::ALPHABLT:
        bltAlpha(imgref.pixel, x, y);
        return;
    case DrawMethod::DIFFUSE:
        bltDiffuse(imgref.pixel, x, y, imgref.diffuse_color);
        return;
    }
}

void Drawable::drawBlend(const ImageReference& imgref, int x, int y, float factor)
{
    if (!data.base) return;
    switch (imgref.draw_method) {
    case DrawMethod::BLT:
        blt(imgref.pixel, imgref.pixel.rect(), x, y);
        return;
    case DrawMethod::ALPHABLT:
        bltBlend(imgref.pixel, factor, x, y);
        return;
    case DrawMethod::DIFFUSE:
        bltDiffuse(imgref.pixel, x, y, imgref.diffuse_color);
        return;
    }
}

/*!\brief Rechteck unter Berücksichtigung eines Farbschlüssels kopieren (Bluescreen-Effekt)
 *
 * \desc
 * Mit dieser Funktion kann ein "Bluescreen-Effekt" erzielt werden (siehe http://de.wikipedia.org/wiki/Bluescreen-Technik#Greenscreen).
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
void Drawable::bltChromaKey(const Drawable& source, const Color& key, uint8_t tol1, uint8_t tol2, int x, int y)
{
    bltChromaKey(source, source.rect(), key, tol1, tol2, x, y);
}

/*!\brief Rechteck unter Berücksichtigung eines Farbschlüssels kopieren (Bluescreen-Effekt)
 *
 * \desc
 * Mit dieser Funktion kann ein "Bluescreen-Effekt" erzielt werden (siehe http://de.wikipedia.org/wiki/Bluescreen-Technik#Greenscreen).
 * Dabei wird die Quellgrafik \p source mittels eines Farbschlüssels \p key (Chroma Key), sowie zwei Toleranz-Werten
 * über den Hintergrund gelegt.
 *
 * @param source Quellgrafik
 * @param srect Rechteckiger Ausschnitt aus der Quellgrafik \p source, der kopiert werden soll
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
void Drawable::bltChromaKey(const Drawable& source, const Rect& srect, const Color& key, uint8_t tol1, uint8_t tol2, int x, int y)
{
    if (source.isEmpty() || !data.base) return;
    // Quellrechteck
    Rect q;
    if (srect.isNull()) {
        q = source.rect();
    } else {
        q = srect;
        if (q.left() < 0) q.setLeft(0);
        if (q.width() > source.width()) q.setWidth(source.width());
        if (q.top() < 0) q.setTop(0);
        if (q.height() > source.height()) q.setHeight(source.height());
    }
    if (!fitRect(x, y, q)) return;
    data.fn->BltChromaKey(data, source.data, q, key, tol1, tol2, x, y);
}

/*!\brief Rechteck unter Berücksichtigung eines Farbschlüssels kopieren (Bluescreen-Effekt)
 *
 * \desc
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
void Drawable::bltBackgroundOnChromaKey(const Drawable& background, const Color& key, uint8_t tol1, uint8_t tol2, int x, int y)
{
    bltBackgroundOnChromaKey(background, rect(), key, tol1, tol2, x, y);
}

/*!\brief Rechteck unter Berücksichtigung eines Farbschlüssels kopieren (Bluescreen-Effekt)
 *
 * \desc
 * Mit dieser Funktion kann ein "Bluescreen-Effekt" erzielt werden (siehe http://de.wikipedia.org/wiki/Bluescreen-Technik#Greenscreen).
 * Dabei wird die Hintergundgrafik \p background mittels eines Farbschlüssels \p key (Chroma Key), sowie zwei Toleranz-Werten
 * über die Grafik gelegt.
 *
 * @param background Hintergundgrafik
 * @param srect Rechteckiger Ausschnitt aus der Hintergundgrafik \p background, der kopiert werden soll
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
void Drawable::bltBackgroundOnChromaKey(
    const Drawable& background, const Rect& srect, const Color& key, uint8_t tol1, uint8_t tol2, int x, int y)
{
    if (background.isEmpty() || !data.base) return;
    // Quellrechteck
    Rect q;
    if (srect.isNull()) {
        q = rect();
    } else {
        q = srect;
        if (q.left() < 0) q.setLeft(0);
        if (q.width() > width()) q.setWidth(width());
        if (q.top() < 0) q.setTop(0);
        if (q.height() > height()) q.setHeight(height());
    }
    if (!fitRect(x, y, q)) return;
    data.fn->BltBackgoundOnChromaKey(data, background.data, q, key, tol1, tol2, x, y);
}

} // namespace pplib::grafix