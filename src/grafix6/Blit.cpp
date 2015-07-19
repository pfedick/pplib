/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.3 $
 * $Date: 2010/02/20 13:25:20 $
 * $Id: Blit.cpp,v 1.3 2010/02/20 13:25:20 pafe Exp $
 *
 *******************************************************************************
 * Copyright (c) 2010, Patrick Fedick <patrick@pfp.de>
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
#include "ppl6.h"
#include "ppl6-grafix.h"
#include "grafix6.h"

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

namespace ppl6 {
namespace grafix {


static void *adr(const DRAWABLE_DATA &data, int x, int y)
{
	if (x<data.width && y<data.height) return data.base8+(y*data.pitch)+(x*data.rgbformat.bitdepth()/8);
	return NULL;
}


static int Blt_32 (DRAWABLE_DATA &target, const DRAWABLE_DATA &source, const Rect &srect, int x, int y)
{
	/*
	if (srect.width()==target.width && source.pitch==target.pitch) {
		BLTDATA data;
		data.src=(ppluint32*)adr(source,srect.left(),srect.top());
		data.tgt=(ppluint32*)adr(target,x,y);
		memcpy(data.tgt,data.src,source.pitch*srect.height());
		return 1;

	}
	*/
#ifdef HAVE_X86_ASSEMBLER
	BLTDATA data;
	data.src=(ppluint32*)adr(source,srect.left(),srect.top());
	data.tgt=(ppluint32*)adr(target,x,y);
	data.width=srect.width();
	data.height=srect.height();
	data.pitchsrc=source.pitch;
	data.pitchtgt=target.pitch;
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
	ppldd alpha1,psrc;
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
 * @return Liefert 1 zurück, wenn die Funktionen erfolgreich initialisiert wurden, 0, wenn das Farbformat
 * nicht unterstützt wird.
 *
 * \remarks
 * Gegenwärtig werden nur Farbformate mit einer Tiefe von 32 Bit unterstützt, jedoch könnten andere
 * Farbformate von der verwendeten Grafik-Engine unterstüzt werden.
 *
 */
int CGrafix::InitBlits(const RGBFormat &format, GRAFIX_FUNCTIONS *fn)
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
			return 1;
		case RGBFormat::GREY8:
		case RGBFormat::A8:
			return 1;

	}
	SetError(1013,"RGBFormat=%s (%i)",(const char*)format.name(),format.format());
	return 0;
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
int CDrawable::fitRect(int &x, int &y, Rect &r)
{
	Rect screen(0,0,data.width,data.height);
	Rect object(x,y,r.width(),r.height());
	Rect i=screen.intersected(object);		// TODO: Das ist falsch
	if (i.isNull()) return 0;
	int shiftx=i.x()-object.x();
	int shifty=i.y()-object.y();
	x+=shiftx;
	y+=shifty;
	r.setWidth(i.width());
	r.setHeight(i.height());
	r.setX(r.x()+shiftx);
	r.setY(r.y()+shifty);
	return 1;
}

/*!\brief Rechteck 1:1 kopieren
 *
 * \desc
 * Mit dieser Funktion wird die Quellzeichenfläche \p source
 * an die Position \p x / \p y der Zielzeichenfläche kopiert, wobei alle Farbinformationen 1:1 übernommen werden.
 * Es wird weder Alphablending (siehe CDrawable::bltAlpha) noch Colorkeying (siehe
 * CDrawable::bltColorKey) verwendet.
 * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert
 * (siehe CDrawable::fitRect).
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] x Optionale X-Koordinate der linken oberen Ecke in der Zielzeichenfläche. Wird der Parameter nicht
 *            angegeben, wird 0 verwendet.
 * \param[in] y Optionale Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche. Wird der Parameter
 *            nicht angegebenm wird 0 verwendet.
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CDrawable::blt(const CDrawable &source, int x, int y)
{
	return blt(source,source.rect(),x,y);
}

/*!\brief Rechteck 1:1 kopieren
 *
 * \desc
 * Mit dieser Funktion wird der Ausschnitt \p srect aus der Quellzeichenfläche \p source
 * an die Position \p x / \p y kopiert, wobei alle Farbinformationen 1:1 übernommen werden.
 * Es wird weder Alphablending (siehe CDrawable::bltAlpha) noch Colorkeying (siehe
 * CDrawable::bltColorKey) verwendet. Falls \p srect 0 ist, wird die komplette Quellzeichenfläche kopiert,
 * andernfalls nur der angegebene Ausschnitt.
 * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert
 * (siehe CDrawable::fitRect).
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] srect Rechteckiger Ausschnitt aus der Quellzeichenfläche, der kopiert werden soll
 * \param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CDrawable::blt(const CDrawable &source, const Rect &srect, int x, int y)
{
	if (source.isEmpty()) {
		SetError(1067);
		return 0;
	}
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
	if (!fitRect(x,y,q)) {
		SetError(1016);
		return 0;
	}
	if (fn->Blt) return fn->Blt(data,source.data,q,x,y);
	SetError(1012,"Blt");
	return 0;
}

int CDrawable::bltDiffuse(const CDrawable &source, int x, int y,const Color &c)
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
 * (siehe CDrawable::fitRect).
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] c Die gewünschte Pixelfarbe
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
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
 * (siehe CDrawable::fitRect).
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] srect Rechteckiger Ausschnitt aus der Quellzeichenfläche, der kopiert werden soll
 * \param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] c Die gewünschte Pixelfarbe
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 *
 */
int CDrawable::bltDiffuse(const CDrawable &source, const Rect &srect, int x, int y,const Color &c)
{
	if (source.isEmpty()) {
		SetError(1067);
		return 0;
	}
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
	if (!fitRect(x,y,q)) {
		SetError(1016);
		return 0;
	}
	if (fn->BltDiffuse) return fn->BltDiffuse(data,source.data,q,x,y,rgb(c));
	SetError(1012,"Blt");
	return 0;
}

/*!\brief Rechteck unter Berücksichtigung einer transparenten Schlüsselfarbe kopieren
 *
 * \desc
 * Mit dieser Funktion wird die Quellzeichenfläche \p source
 * an die Position \p x / \p y unter Berücksichtigung der Schlüsselfarbe \p c kopiert.
 * Pixel, die der Farbe \c entsprechen, bleiben dabei vollständig transparent, alle anderen
 * Pixel werden wie bei CDrawable::blt 1:1 kopiert.
 * \par
  * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert
 * (siehe CDrawable::fitRect).
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] c Die gewünschte Schlüsselfarbe (ColorKey)
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CDrawable::bltColorKey(const CDrawable &source, int x, int y,const Color &c)
{
	return bltColorKey(source,source.rect(),x,y,c);
}

/*!\brief Rechteck unter Berücksichtigung einer transparenten Schlüsselfarbe kopieren
 *
 * \desc
 * Mit dieser Funktion wird der Ausschnitt \p srect aus der Quellzeichenfläche \p source
 * an die Position \p x / \p y unter Berücksichtigung der Schlüsselfarbe \p c kopiert.
 * Pixel, die der Farbe \c entsprechen, bleiben dabei vollständig transparent, alle anderen
 * Pixel werden wie bei CDrawable::blt 1:1 kopiert.
 * \par
 * Falls \p srect 0 ist, wird die komplette Quellzeichenfläche kopiert, andernfalls nur der angegebene Ausschnitt.
 * Falls die Quelle nicht in die Zielzeichenfläche passt, wird nur der passende Teil kopiert
 * (siehe CDrawable::fitRect).
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] srect Rechteckiger Ausschnitt aus der Quellzeichenfläche, der kopiert werden soll
 * \param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] c Die gewünschte Schlüsselfarbe (ColorKey)
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CDrawable::bltColorKey(const CDrawable &source, const Rect &srect, int x, int y,const Color &c)
{
	if (source.isEmpty()) {
		SetError(1067);
		return 0;
	}
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
	if (!fitRect(x,y,q)) {
		SetError(1016);
		return 0;
	}
	if (fn->BltColorKey) return fn->BltColorKey(data,source.data,q,x,y,rgb(c));
	SetError(1012,"Blt");
	return 0;
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
 * (siehe CDrawable::fitRect).
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] c Die gewünschte Schlüsselfarbe (ColorKey)
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CDrawable::bltAlpha(const CDrawable &source, int x, int y)
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
 * (siehe CDrawable::fitRect).
 *
 * \param[in] source Die Quellzeichenfläche
 * \param[in] srect Rechteckiger Ausschnitt aus der Quellzeichenfläche, der kopiert werden soll
 * \param[in] x X-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] y Y-Koordinate der linken oberen Ecke in der Zielzeichenfläche
 * \param[in] c Die gewünschte Schlüsselfarbe (ColorKey)
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CDrawable::bltAlpha(const CDrawable &source, const Rect &srect, int x, int y)
{
	if (source.isEmpty()) {
		SetError(1067);
		return 0;
	}
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
	if (!fitRect(x,y,q)) {
		SetError(1016);
		return 0;
	}
	if (fn->BltAlpha) return fn->BltAlpha(data,source.data,q,x,y);
	SetError(1012,"Blt");
	return 0;
}

/*!\brief Grafik aus einer Image-Liste kopieren
 *
 * \desc
 * Mit dieser Funktion wird eine Grafik aus einer Image-Liste (siehe CImageList) kopiert.
 * Jenachdem welche Zeichenmethode in der Image-Liste definiert ist, wird dazu entweder
 * CDrawable::blt, CDrawable::bltDiffuse, CDrawable::bltColorKey oder CDrawable::bltAlpha
 * verwendet.
 *
 * @param iml Image-Liste
 * @param nr Nummer der Grafik innerhalb der Image-Liste
 * @param x X-Koordinate der Zielposition
 * @param y Y-Koordinate der Zielposition
 *
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CDrawable::draw(const CImageList &iml, int nr, int x, int y)
{
	Rect r=iml.getRect(nr);
	if (r.isNull()) {
		return 0;
	}
	switch ((int)iml.method) {
		case CImageList::BLT:
			return blt(iml,r,x,y);
			break;
		case CImageList::ALPHABLT:
			return bltAlpha(iml,r,x,y);
			break;
		case CImageList::COLORKEY:
			return bltColorKey(iml,r,x,y,iml.colorkey);
			break;
		case CImageList::DIFFUSE:
			return bltDiffuse(iml,r,x,y,iml.diffuse);
			break;
	}
	SetError(1042,"%i",(int)iml.drawMethod());
	return 0;
}

/*!\brief Grafik aus einer Image-Liste kopieren
 *
 * \desc
 * Mit dieser Funktion wird eine Grafik aus einer Image-Liste (siehe CImageList) kopiert.
 * Jenachdem welche Zeichenmethode in der Image-Liste definiert ist, wird dazu entweder
 * CDrawable::blt, CDrawable::bltDiffuse, CDrawable::bltColorKey oder CDrawable::bltAlpha
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
int CDrawable::draw(const CImageList &iml, int nr, int x, int y, const Color &diffuse)
{
	Rect r=iml.getRect(nr);
	if (r.isNull()) {
		return 0;
	}
	switch ((int)iml.method) {
		case CImageList::BLT:
			return blt(iml,r,x,y);
			break;
		case CImageList::ALPHABLT:
			return bltAlpha(iml,r,x,y);
			break;
		case CImageList::COLORKEY:
			return bltColorKey(iml,r,x,y,iml.colorkey);
			break;
		case CImageList::DIFFUSE:
			return bltDiffuse(iml,r,x,y,diffuse);
			break;
	}
	SetError(1042,"%i",(int)iml.drawMethod());
	return 0;
}

int CDrawable::draw(const CSprite &sprite, int nr, int x, int y)
{
	return sprite.draw(*this,x,y,nr);
}



} // EOF namespace grafix
} // EOF namespace ppl6

