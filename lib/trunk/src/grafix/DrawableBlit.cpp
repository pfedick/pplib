/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
 *
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include "prolog.h"
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include "ppl7.h"
#include "ppl7-grafix.h"


//#undef HAVE_NASM
#ifdef HAVE_X86_ASSEMBLER
typedef struct {
	void		*src;
	void		*tgt;
	ppluint32	width;
	ppluint32	height;
	ppluint32	pitchsrc;
	ppluint32	pitchtgt;
	ppluint32	color;
} BLTDATA;

extern "C" {
	int ASM_AlphaBlt32(BLTDATA *d);
	int ASM_Blt32(BLTDATA *d);
	int ASM_BltColorKey32(BLTDATA *d);
	int ASM_BltDiffuse32(BLTDATA *d);
}
#endif

namespace ppl7 {
namespace grafix {

extern char *alphatab;


static void *adr(const DRAWABLE_DATA &data, int x, int y)
{
	if (x<data.width && y<data.height) return data.base8+(y*data.pitch)+(x*data.rgbformat.bitdepth()/8);
	return NULL;
}


static int Blt_32 (DRAWABLE_DATA &target, const DRAWABLE_DATA &source, const Rect &srect, int x, int y)
{
#ifdef HAVE_X86_ASSEMBLER
	BLTDATA data;
	data.src=(ppluint32*)adr(source,srect.left(),srect.top());
	data.tgt=(ppluint32*)adr(target,x,y);
	data.width=srect.width();
	data.height=srect.height();
	data.pitchsrc=source.pitch;
	data.pitchtgt=target.pitch;
	/*
	printf ("Blt src=%lx\n",data.src);
	printf ("Blt tgt=%lx\n",data.tgt);
	printf ("width=%i\n",data.width);
	printf ("height=%i\n",data.height);
	printf ("pitchsrc=%i\n",data.pitchsrc);
	printf ("pitchtgt=%i\n",data.pitchtgt);
	*/

	if (ASM_Blt32(&data)) {
		return 1;
	}
	return 0;
#endif
	ppluint8 *q, *z;
	q=(ppluint8 *)adr(source,srect.left(),srect.top());
	z=(ppluint8 *)adr(target,x,y);
	int mywidth=srect.width()*target.rgbformat.bytesPerPixel();
	int yy;
	for (yy=0;yy<srect.height();yy++) {
		memmove (z,q,mywidth);
		q+=source.pitch;
		z+=target.pitch;
	}
	return 1;
}

static int BltAlpha_32 (DRAWABLE_DATA &target, const DRAWABLE_DATA &source, const Rect &srect, int x, int y)
{
#ifdef HAVE_X86_ASSEMBLER
	BLTDATA data;
	data.src=(ppluint32*)adr(source,srect.left(),srect.top());
	data.tgt=(ppluint32*)adr(target,x,y);
	data.width=srect.width();
	data.height=srect.height();
	data.pitchsrc=source.pitch;
	data.pitchtgt=target.pitch;
	if (ASM_AlphaBlt32(&data)) {
		return 1;
	}
	return 0;
#endif
	ppluint32 alpha1,psrc;
	ppluint32 *src, *tgt;
	src=(ppluint32*)adr(source,srect.left(),srect.top());
	tgt=(ppluint32*)adr(target,x,y);
	int width=srect.width();
	int yy, xx;
	if (!alphatab) return 0;

	for (yy=0;yy<srect.height();yy++) {
		for (xx=0;xx<width;xx++) {
			psrc=src[xx];
			if (psrc) {
				if ((psrc&0xff000000)==0xff000000) tgt[xx]=psrc;
				else {
					alpha1=psrc>>24;
					tgt[xx]=target.fn->RGBBlend255(tgt[xx],psrc,alpha1);
				}
			}
		}
		src+=(source.pitch>>2);
		tgt+=(target.pitch>>2);
	}
	return 1;
}



static int BltColorKey_32 (DRAWABLE_DATA &target, const DRAWABLE_DATA &source, const Rect &srect, int x, int y, SurfaceColor c)
{
#ifdef HAVE_X86_ASSEMBLER
	BLTDATA data;
	data.src=(ppluint32*)adr(source,srect.left(),srect.top());
	data.tgt=(ppluint32*)adr(target,x,y);
	data.width=srect.width();
	data.height=srect.height();
	data.pitchsrc=source.pitch;
	data.pitchtgt=target.pitch;
	data.color=c;
	if (ASM_BltColorKey32(&data)) {
		return 1;
	}
#endif
	ppluint32 *q, *z;
	SurfaceColor qc;
	q=(ppluint32*)adr(source,srect.left(),srect.top());
	z=(ppluint32*)adr(target,x,y);
	int pitch_tgt=target.pitch>>2;
	int pitch_src=source.pitch>>2;
	int yy, xx;
	int width=srect.width();
	for (yy=0;yy<srect.height();yy++) {
		for (xx=0;xx<width;xx++) {
			qc=q[xx];
			if (qc!=c) z[xx]=qc;
		}
		q+=pitch_src;
		z+=pitch_tgt;
	}
	return 1;
}

static int BltDiffuse_32 (DRAWABLE_DATA &target, const DRAWABLE_DATA &source, const Rect &srect, int x, int y, SurfaceColor c)
{
#ifdef HAVE_X86_ASSEMBLER
	BLTDATA data;
	data.src=(ppluint32*)adr(source,srect.left(),srect.top());
	data.tgt=(ppluint32*)adr(target,x,y);
	data.width=srect.width();
	data.height=srect.height();
	data.pitchsrc=source.pitch;
	data.pitchtgt=target.pitch;
	data.color=c;
	if (ASM_BltDiffuse32(&data)) {
		return 1;
	}
	return 0;
#endif
	ppluint32 *z;
	ppluint8 *q;
	SurfaceColor qc;
	q=(ppluint8*)adr(source,srect.left(),srect.top());
	z=(ppluint32*)adr(target,x,y);
	int pitch32=target.pitch>>2;
	int yy, xx;
	int width=srect.width();
	for (yy=0;yy<srect.height();yy++) {
		for (xx=0;xx<width;xx++) {
			qc=q[xx];
			if (qc) {
				if (qc==0xff) z[xx]=c;
				else {
					z[xx]=target.fn->RGBBlend255(z[xx],c,qc);
				}
			}
		}
		q+=source.pitch;
		z+=pitch32;
	}
	return 1;
}


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
void Grafix::initBlits(const RGBFormat &format, GRAFIX_FUNCTIONS *fn)
{
	switch (format) {
		case RGBFormat::A8R8G8B8:		// 32 Bit True Color
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8B8G8R8:
		case RGBFormat::X8R8G8B8:
			fn->Blt=Blt_32;
			fn->BltAlpha=BltAlpha_32;
			fn->BltColorKey=BltColorKey_32;
			fn->BltDiffuse=BltDiffuse_32;
			return;
		case RGBFormat::GREY8:
		case RGBFormat::A8:
			return;

	}
	throw UnsupportedColorFormatException("RGBFormat=%ls (%i)",(const wchar_t*)format.name(),format.format());
}


/*!\brief Überprüft, ob eine Blit-Aktion in den Zeichenbereich passt.
 *
 * \desc
 * Diese Funktion prüft, ob das zu zeichnende Rechteck überhaupt in die aktuelle
 * Zeichenfläche. Dabei wird das Quellrechteck bei Bedarf angepasst.
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
int Drawable::fitRect(int &x, int &y, Rect &r)
{
	Rect screen(0,0,data.width,data.height);
	Rect object(x,y,r.width(),r.height());
	Rect i=screen.intersected(object);		// TODO: Das ist falsch

	//::printf ("Drawable::fitRect screen (%i/%i)-(%i/%i)\n", screen.x1, screen.y1, screen.x2, screen.y2);
	//::printf ("Drawable::fitRect object (%i/%i)-(%i/%i)\n", object.x1, object.y1, object.x2, object.y2);
	//::printf ("Drawable::fitRect i (%i/%i)-(%i/%i)\n", i.x1, i.y1, i.x2, i.y2);

	if (i.isNull()) return 0;
	int shiftx=i.x1-object.x1;
	int shifty=i.y1-object.y1;
	x+=shiftx;
	y+=shifty;

	r.x1+=shiftx;
	r.y1+=shifty;
	r.x2=r.x1+i.width();
	r.y2=r.y1+i.height();
	//::printf ("Drawable::fitRect r (%i/%i)-(%i/%i)\n", r.x1, r.y1, r.x2, r.y2);
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
void Drawable::blt(const Drawable &source, int x, int y)
{
	return blt(source,source.rect(),x,y);
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
void Drawable::blt(const Drawable &source, const Rect &srect, int x, int y)
{
	if (source.isEmpty()) throw EmptyDrawableException();
	// Quellrechteck
	Rect q;
	if (srect.isNull()) {
		q=source.rect();
	} else {
		q=srect;
		if (q.left()<0) q.setLeft(0);
		if (q.width()>source.width()) q.setWidth(source.width());
		if (q.top()<0) q.setTop(0);
		if (q.height()>source.height()) q.setHeight(source.height());
	}
	//::printf ("rect=(%i/%i)-(%i/%i)\n", q.x1, q.y1, q.x2, q.y2);
	if (!fitRect(x,y,q)) return;
	//::printf ("rect=(%i/%i)-(%i/%i)\n", q.x1, q.y1, q.x2, q.y2);
	if (!fn->Blt) throw FunctionUnavailableException("Drawable::blt");
	fn->Blt(data,source.data,q,x,y);
}

void Drawable::bltDiffuse(const Drawable &source, int x, int y,const Color &c)
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
	return bltDiffuse(source,source.rect(),x,y,c);
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
void Drawable::bltDiffuse(const Drawable &source, const Rect &srect, int x, int y,const Color &c)
{
	if (source.isEmpty()) throw EmptyDrawableException();
	// Quellrechteck
	Rect q;
	if (srect.isNull()) {
		q=source.rect();
	} else {
		q=srect;
		if (q.left()<0) q.setLeft(0);
		if (q.width()>source.width()) q.setWidth(source.width());
		if (q.top()<0) q.setTop(0);
		if (q.height()>source.height()) q.setHeight(source.height());
	}
	if (!fitRect(x,y,q)) return;
	if (!fn->BltDiffuse) throw FunctionUnavailableException("Drawable::bltDiffuse");
	fn->BltDiffuse(data,source.data,q,x,y,rgb(c));
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
void Drawable::bltColorKey(const Drawable &source, int x, int y,const Color &c)
{
	return bltColorKey(source,source.rect(),x,y,c);
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
void Drawable::bltColorKey(const Drawable &source, const Rect &srect, int x, int y,const Color &c)
{
	if (source.isEmpty()) throw EmptyDrawableException();
	// Quellrechteck
	Rect q;
	if (srect.isNull()) {
		q=source.rect();
	} else {
		q=srect;
		if (q.left()<0) q.setLeft(0);
		if (q.width()>source.width()) q.setWidth(source.width());
		if (q.top()<0) q.setTop(0);
		if (q.height()>source.height()) q.setHeight(source.height());
	}
	if (!fitRect(x,y,q)) return;
	if (!fn->BltColorKey) throw FunctionUnavailableException("Drawable::bltColorKey");
	fn->BltColorKey(data,source.data,q,x,y,rgb(c));
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
void Drawable::bltAlpha(const Drawable &source, int x, int y)
{
	return bltAlpha(source,source.rect(),x,y);
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
void Drawable::bltAlpha(const Drawable &source, const Rect &srect, int x, int y)
{
	if (source.isEmpty()) throw EmptyDrawableException();
	// Quellrechteck
	Rect q;
	if (srect.isNull()) {
		q=source.rect();
	} else {
		q=srect;
		if (q.left()<0) q.setLeft(0);
		if (q.width()>source.width()) q.setWidth(source.width());
		if (q.top()<0) q.setTop(0);
		if (q.height()>source.height()) q.setHeight(source.height());
	}
	if (!fitRect(x,y,q)) return;
	if (!fn->BltAlpha) throw FunctionUnavailableException("Drawable::bltAlpha");
	fn->BltAlpha(data,source.data,q,x,y);
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
void Drawable::draw(const ImageList &iml, int nr, int x, int y)
{
	Rect r=iml.getRect(nr);
	switch ((int)iml.method) {
		case ImageList::BLT:
			blt(iml,r,x,y);
			return;
		case ImageList::ALPHABLT:
			bltAlpha(iml,r,x,y);
			return;
		case ImageList::COLORKEY:
			bltColorKey(iml,r,x,y,iml.colorkey);
			return;
		case ImageList::DIFFUSE:
			bltDiffuse(iml,r,x,y,iml.diffuse);
			return;
	}
	throw UnknownBltMethodException();
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
void Drawable::draw(const ImageList &iml, int nr, int x, int y, const Color &diffuse)
{
	Rect r=iml.getRect(nr);
	switch ((int)iml.method) {
		case ImageList::BLT:
			blt(iml,r,x,y);
			return;
		case ImageList::ALPHABLT:
			bltAlpha(iml,r,x,y);
			return;
		case ImageList::COLORKEY:
			bltColorKey(iml,r,x,y,iml.colorkey);
			return;
		case ImageList::DIFFUSE:
			bltDiffuse(iml,r,x,y,diffuse);
			return;
	}
	throw UnknownBltMethodException();
}

#ifdef DONE
void Drawable::draw(const Sprite &sprite, int nr, int x, int y)
{
	sprite.draw(*this,x,y,nr);
}
#endif


} // EOF namespace grafix
} // EOF namespace ppl7

