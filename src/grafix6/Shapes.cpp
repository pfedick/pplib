/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
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

#ifdef HAVE_X86_ASSEMBLER
typedef struct {
	void		*tgt;
	ppluint32	width;
	ppluint32	height;
	ppluint32	pitch;
	ppluint32	color;
} RECTDATA;

extern "C" {
	int ASM_FillRect32(RECTDATA *d);
}
#endif

namespace ppl6 {
namespace grafix {

static void FillRect_32 (DRAWABLE_DATA &data, const Rect &r, SurfaceColor color);

/*
static int ClearScreen_16 (SURFACE *surface, COLOR color)
{
	if (!surface->Lock(surface)) return 0;
	int x,y;
	ppldw *adr16;
	adr16=surface->base16;
	for (y=0;y<surface->height;y++) {
		for (x=0;x<surface->width;x++) adr16[x]=(ppldw)color;
		adr16+=surface->pitch16;
	}
	surface->Unlock(surface);
	return 1;
}
*/

static void ClearScreen_8 (DRAWABLE_DATA &data, SurfaceColor c)
{
	int x,y;
	ppluint8 *adr;
	adr=data.base8;
	for (y=0;y<data.height;y++) {
		for (x=0;x<data.width;x++) adr[x]=(ppluint8)c;
		adr+=data.pitch;
	}
}

static void ClearScreen_32 (DRAWABLE_DATA &data, SurfaceColor c)
{
	FillRect_32(data,Rect(0,0,data.width,data.height),c);
	/*
	int x,y;
	ppluint32 *adr32;
	adr32=data.base32;
	ppluint32 pitch32=data.pitch>>2;
	for (y=0;y<data.height;y++) {
		for (x=0;x<data.width;x++) adr32[x]=(ppluint32)c;
		adr32+=pitch32;
	}
	*/
}


static void DrawRect_32 (DRAWABLE_DATA &data, const Rect &r, SurfaceColor color)
{
	int x,y;
	ppluint32 * pp;
	ppluint32 pitch32=data.pitch>>2;
	pp=data.base32+r.top()*pitch32+r.left();
	for (x=0;x<r.width();x++) pp[x]=(ppldd)color;
	x=r.width()-1;
	for (y=1;y<r.height();y++) {
		pp+=pitch32;
		pp[0]=(ppldd)color;
		pp[x]=(ppldd)color;
	}
	for (x=0;x<r.width();x++) pp[x]=(ppldd)color;
}

static void FillRect_32 (DRAWABLE_DATA &data, const Rect &r, SurfaceColor color)
{
	Rect s(0,0,data.width,data.height);
	Rect in=s.intersected(r);
	if (in.isNull()) return;
	#ifdef HAVE_X86_ASSEMBLER
		RECTDATA d;
		d.tgt=data.base8+in.top()*data.pitch+in.left()*data.rgbformat.bytesPerPixel();
		d.width=in.width();
		d.height=in.height();
		d.pitch=data.pitch;
		d.color=color;
		ASM_FillRect32(&d);
	#else
		ppluint32 * pp;
		int y,x;
		ppluint32 pitch32=data.pitch>>2;
		pp=data.base32+in.top()*pitch32;
		for (y=in.top();y<=in.bottom();y++) {
			for (x=in.left();x<=in.right();x++) pp[x]=(ppldd)color;
			pp+=pitch32;
		}
	#endif
}

/*
static int Xchange_32 (SURFACE* data, int x1, int y1, int x2, int y2, COLOR farbe, COLOR ersatzfarbe)
{
	ppldds y,x;
	RECT r;
	ppldds breite;
	ppldb * pp;
	ppldd * ppw;

	r.left=x1; r.top=y1; r.right=x2; r.bottom=y2;
	if (!data->Surface->FitRect (&r)) return 0;
	pp=(ppldb *) (data->base8+r.top*data->pitch8+x1*data->bytes_per_pixel);
	ppw=(ppldd *)pp;
	breite=r.right-r.left+1;
	for (y=r.top;y<r.bottom+1;y++) {
		for (x=0;x<breite;x++) {
			if (ppw[x]==(ppldd)farbe) ppw[x]=(ppldd)ersatzfarbe;
		}
		ppw+=data->pitch32;
	}
	return 1;
}

static int Invert_32 (SURFACE* data, int x1, int y1, int x2, int y2, COLOR color1, COLOR color2)
{
	COLOR pixel;
	ppldds y,x;
	RECT r;
	ppldds breite;
	ppldb * pp;
	ppldd * ppw;

	r.left=x1; r.top=y1; r.right=x2; r.bottom=y2;
	if (!data->Surface->FitRect (&r)) return 0;
	pp=(ppldb *) (data->base8+r.top*data->pitch8+x1*data->bytes_per_pixel);
	ppw=(ppldd *)pp;
	breite=r.right-r.left+1;
	for (y=r.top;y<r.bottom+1;y++) {
		for (x=0;x<breite;x++) {
			pixel=ppw[x];
			if (pixel==color1) ppw[x]=color2;
			if (pixel==color2) ppw[x]=color1;
		}
		ppw+=data->pitch32;
	}
	return 1;
}

static int Negativ_32 (SURFACE* data, int x1, int y1, int x2, int y2)
{
	RGBA pixel;
	ppldds y,x;
	RECT r;
	ppldds breite;
	ppldb * pp;
	ppldd * ppw;

	r.left=x1; r.top=y1; r.right=x2; r.bottom=y2;
	if (!data->Surface->FitRect (&r)) return 0;
	pp=(ppldb *) (data->base8+r.top*data->pitch8+x1*data->bytes_per_pixel);
	ppw=(ppldd *)pp;
	breite=r.right-r.left+1;
	for (y=r.top;y<r.bottom+1;y++) {
		for (x=0;x<breite;x++) {
			pixel.color=ppw[x];
			pixel.red=255-pixel.red;
			pixel.green=255-pixel.green;
			pixel.blue=255-pixel.blue;
			ppw[x]=pixel.color;
		}
		ppw+=data->pitch32;
	}
	return 1;
}
*/

int CGrafix::InitShapes(const RGBFormat &format, GRAFIX_FUNCTIONS *fn)
/*!\brief Initialisiert Funktionen zur Behandlung von Rechtecken
 *
 * \desc
 * Diese Funktion initialisiert eine Reihe von Funktionen zur Bearbeitung von Rechtecken.
 * Dabei handelt es sich um die Funktionen:
 * - CLS
 * - Rect
 * - FillRect
 * - Xchange
 * - Invert
 * - Negativ
 *
 * @param[in] format Das gewünschte Farbformat
 * @param[in] fn Pointer auf die Struktur mit den Funktionen
 * @return Liefert 1 zurück, wenn das Farbformat unterstützt wird, sonst 0
 */
{
	switch (format) {
		case RGBFormat::A8R8G8B8:		// 32 Bit True Color
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8B8G8R8:
		case RGBFormat::X8R8G8B8:
			fn->CLS=ClearScreen_32;
			fn->DrawRect=DrawRect_32;
			fn->FillRect=FillRect_32;
			//fn->Xchange=Xchange_32;
			//fn->Invert=Invert_32;
			//fn->Negativ=Negativ_32;
			return 1;
		case RGBFormat::A8:
		case RGBFormat::GREY8:
			fn->CLS=ClearScreen_8;
			return 1;
	}
	SetError(1013,"RGBFormat=%s (%i)",(const char*)format.name(),format.format());
	return 0;
}

/*!\brief Grafik löschen
 *
 * \desc
 * Durch Aufruf dieser Funktion wird die komplette Grafik gelöscht und mit
 * der Angegebenen Farbe \c gefüllt.
 *
 * \param[in] c Farbwert
 */
void CDrawable::cls(const Color &c)
{
	if (fn->FillRect) {
		fn->FillRect(data,Rect(0,0,data.width,data.height),rgb(c));
		return;
	}
	if (fn->CLS) fn->CLS(data,rgb(c));
}

/*!\brief Grafik löschen
 *
 * \desc
 * Durch Aufruf dieser Funktion wird die komplette Grafik gelöscht, indem
 * der Speicherbereich mit 0 beschrieben wird. Optional kann die Funktion
 * auch mit einem Farbwert aufgerufen werden (siehe CDrawable::cls(const Color &c)).
 */
void CDrawable::cls()
{
	if (fn->FillRect) {
		fn->FillRect(data,Rect(0,0,data.width,data.height),0);
		return;
	}
	if (fn->CLS) fn->CLS(data,0);
}


/*!\brief Rechteck zeichnen
 *
 * \desc
 * Diese Funktion zeichnet ein Rechteck an die gewünschte Position
 *
 * @param[in] rect Koordinaten des Rechtecks
 * @param[in] c Farbe des Rechtecks
 */
void CDrawable::drawRect(const Rect &rect, const Color &c)
{
	int y1,x1,y2,x2;
	if (rect.left()<0 || rect.top()<0 || rect.right()>=data.width || rect.bottom()>=data.height) {
		y1=rect.y1();
		y2=rect.y2();
		for (x1=rect.x1();x1<rect.right();x1++) {
			putPixel(x1,y1,c);
			putPixel(x1,y2,c);
		}
		x1=rect.x1();
		x2=rect.x2();
		for (y1=rect.top();y1<rect.bottom();y1++) {
			putPixel(x1,y1,c);
			putPixel(x2,y1,c);
		}
		return;
	}
	if (fn->DrawRect) fn->DrawRect(data,rect,rgb(c));
}


/*!\brief Rechteck zeichnen
 *
 * \desc
 * Diese Funktion zeichnet ein Rechteck an die gewünschte Position
 *
 * @param[in] x1 X-Koordinate der oberen linken Ecke
 * @param[in] y1 Y-Koordinate der oberen linken Ecke
 * @param[in] x2 X-Koordinate der unteren rechten Ecke
 * @param[in] y2 Y-Koordinate der unteren rechten Ecke
 * @param[in] c Farbe des Rechtecks
 */
void CDrawable::drawRect(int x1, int y1, int x2, int y2, const Color &c)
{
	Rect r;
	r.setCoords(x1,y1,x2,y2);
	drawRect(r,c);
	//if (fn->DrawRect) fn->DrawRect(data,r,RGB(c));
}



/*!\brief Ausgefülltes Rechteck zeichnen
 *
 * \desc
 * Diese Funktion zeichnet ein ausgefülltes Rechteck in der Farbe \p c an die gewünschte Position
 *
 * @param[in] rect Koordinaten des Rechtecks
 * @param[in] c Farbe des Rechtecks
 *
 */
void CDrawable::fillRect(const Rect &rect, const Color &c)
{
	if (fn->FillRect) fn->FillRect(data,rect,rgb(c));
}

/*!\brief Ausgefülltes Rechteck zeichnen
 *
 * \desc
 * Diese Funktion zeichnet ein ausgefülltes Rechteck in der Farbe \p c an die gewünschte Position
 *
 * @param[in] x1 X-Koordinate der oberen linken Ecke
 * @param[in] y1 Y-Koordinate der oberen linken Ecke
 * @param[in] x2 X-Koordinate der unteren rechten Ecke
 * @param[in] y2 Y-Koordinate der unteren rechten Ecke
 * @param[in] c Farbe des Rechtecks
 *
 */
void CDrawable::fillRect(int x1, int y1, int x2, int y2, const Color &c)
{
	Rect r;
	r.setCoords(x1,y1,x2,y2);
	if (fn->FillRect) fn->FillRect(data,r,rgb(c));
}

void CDrawable::xchange(const Rect &rect, const Color &color, const Color &replace)
/*!\brief Farben ersetzen
 *
 * \desc
 * Mit dieser Funktion werden sämtliche Pixel innerhalb eines Rechtecks, die der Farbe \p color
 * entsprechen, durch die Ersatzfarbe \p replace ersetzt
 * @param[in] rect Koordinaten des Rechtecks
 * @param color Alte Farbe
 * @param replace Neu Farbe
 */
{
	if (fn->Xchange) fn->Xchange(data,rect,rgb(color),rgb(replace));
}

void CDrawable::invert(const Rect &rect, const Color &color1, const Color &color2)
/*!\brief Farben vertauschen
 *
 * \desc
 * Mit dieser Funktion werden sämtliche Pixel innerhalb eines Rechtecks, die der Farbe \p color1
 * entsprechen, durch \p color2 ersetzt, und alle Farben mit dem Farbwert \p color2 werden durch
 * \p color1 ersetzt.
 * @param[in] rect Koordinaten des Rechtecks
 * @param[in] color1 Erste Farbe
 * @param[in] color2 Zweite Farbe
 */
{
	if (fn->Invert) fn->Invert(data,rect,rgb(color1),rgb(color2));
}

void CDrawable::negativ(const Rect &rect)
/*!\brief Negativ-Farben erstellen
 *
 * \desc
 * Mit dieser Funktion werden sämtliche Pixel innerhalb eines Rechtecks durch ihren negativen Wert
 * ersetzt.
 *
 * @param[in] rect Koordinaten des Rechtecks
 */
{
	if (fn->Negativ) fn->Negativ(data,rect);
}


void CDrawable::floodFill (int x, int y, const Color &color, const Color &border)
/*!\brief Fläche mit Farbe füllen
 *
 * \desc
 * Mit dieser Funktion kann eine beliebig geformte Fläche mit einer Farbe ausgefüllt werden.
 *
 * \param[in] x X-Koordinate eines beliebigen Punktes innerhalb der zu füllenden Fläche
 * \param[in] y Y-Koordinate eines beliebigen Punktes innerhalb der zu füllenden Fläche
 * \param[in] color Farbwert, mit der die Fläche gefüllt werden soll
 * \param[in] border Farbwert, der die Grenze der zu füllenden Fläche darstellt
 *
 * \remarks
 * Die Funktion ruft sich selbst mehrfach rekursiv auf. Der Code wurde im wesentlichen aus der freien
 * GD-Library, Version 1.2 entnommen
 *
 * \code
 * Portions copyright 1994, 1995, 1996, 1997, 1998, by Cold Spring
 * Harbor Laboratory. Funded under Grant P41-RR02188 by the National
 * Institutes of Health.
 *
 * Portions copyright 1996, 1997, 1998, by Boutell.Com, Inc.
 * \endcode
 */
{
	/*
	 * Quelle des Codes: GD-Library Version 1.2
	 */

	int lastBorder;
	/* Seek left */
	int leftLimit, rightLimit;
	int i;
	leftLimit = (-1);
	for (i = x; (i >= 0); i--) {
		if (getPixel(i, y) == border) {
			break;
		}
		putPixel(i, y, color);
		leftLimit = i;
	}
	if (leftLimit == (-1)) {
		return;
	}
	/* Seek right */
	rightLimit = x;
	for (i = (x+1); (i < data.width); i++) {
		if (getPixel(i, y) == border) {
			break;
		}
		putPixel(i, y, color);
		rightLimit = i;
	}
	/* Look at lines above and below and start paints */
	/* Above */
	if (y > 0) {
		lastBorder = 1;
		for (i = leftLimit; (i <= rightLimit); i++) {
			Color c;
			c = getPixel(i, y-1);
			if (lastBorder) {
				if ((c != border) && (c != color)) {
					floodFill(i, y-1,color,border);
					lastBorder = 0;
				}
			} else if ((c == border) || (c == color)) {
				lastBorder = 1;
			}
		}
	}
	/* Below */
	if (y < ((data.height) - 1)) {
		lastBorder = 1;
		for (i = leftLimit; (i <= rightLimit); i++) {
			Color c;
			c = getPixel(i, y+1);
			if (lastBorder) {
				if ((c != border) && (c != color)) {
					floodFill(i, y+1,color,border);
					lastBorder = 0;
				}
			} else if ((c == border) || (c == color)) {
				lastBorder = 1;
			}
		}
	}
	return;
}

/**************************************************************************
 * Kreise: Elipse, Circle                                                 *
 **************************************************************************/
void CDrawable::elipse (int x, int y, int radx, int rady, const Color &c, bool fill)
{
	int d;
	int i,x1,y1,x2=0,y2=0;
	for (i=0;i<1025;i++) {
		x1 = x + (sinus1024(i) * radx/1024);
		y1 = y + (cosinus1024(i) * rady/1024);
		if (i > 0) {
			d=abs(x2-x1)+abs(y2-y1);
			if (d > 1)
				line (x1, y1, x2, y2, c);
			else if (d==1)
				putPixel (x1,y1,c);
		}
		x2 = x1;
		y2 = y1;
	}
	if (fill) floodFill (x, y, c, c);
}

//void CSurface::Elipse (int x, int y, int radiusx, int radiusy, COLOR Farbe, int Flags, COLOR Fuellfarbe, int startwinkel, int endwinkel)
void CDrawable::elipse(int x, int y, int radx, int rady, const Color &c, bool fill, const Color &fillcolor, int start, int end)
{
	int d;
	int i,x1,y1,x2,y2;
	if (start!=end) {
		x2 = x + (sinus1024(start) * radx/1024);
		y2 = y + (cosinus1024(start) * rady/1024);
		putPixel (x2,y2,c);

		for (i=start;i<end+1;i++) {
			x1 = x + (sinus1024(i) * radx/1024);
			y1 = y + (cosinus1024(i) * rady/1024);
			if (i > 0) {
				d=abs(x2-x1)+abs(y2-y1);
				if (d > 1)
					line (x1, y1, x2, y2, c);
				else if (d==1)
					putPixel (x1,y1,c);
			}
			x2 = x1;
			y2 = y1;
		}
		if (fill) {
			x1 = x + (sinus1024((start+end)/2) * (radx-2)/1024);
			y1 = y + (cosinus1024((start+end)/2) * (rady-2)/1024);
			floodFill (x1, y1, fillcolor, c);
		}
	}
}

void CDrawable::circle (int x, int y, int rad, const Color &c, bool fill)
{
	elipse(x,y,rad,rad,c,fill);
}

/*
void CSurface::Polygon (int count, POINT *points, COLOR color, int Flags)
{
	POINT *p=points;
	if (points!=NULL) {
		PutPixel(p->x,p->y,color);
		count--;
		p++;
		while (count>0) {
			Line(p->x,p->y,color);
			p++;
			count--;
		}
		Line (points->x,points->y,color);
	}
}
*/

void CDrawable::colorGradient(const Rect &rect, const Color &c1, const Color &c2, int direction)
{
	colorGradient(rect.x1(), rect.y1(), rect.x2(), rect.y2(),c1,c2,direction);
}

void CDrawable::colorGradient(int x1, int y1, int x2, int y2, const Color &c1, const Color &c2, int direction)
{
	Color c;
	ppldd w1,w2;
	int range;
	c.setAlpha(255);
	if (direction==0) {
		range=x2-x1+1;
		for (ppldds x=0; x<range; x++) {
			w1=range-x;
			w2=x;
			c.setRed((c1.red()*w1/range)+(c2.red()*w2/range));
			c.setGreen((c1.green()*w1/range)+(c2.green()*w2/range));
			c.setBlue((c1.blue()*w1/range)+(c2.blue()*w2/range));
			line(x1+x,y1,x1+x,y2,c);
		}
	} else {
		range=y2-y1+1;
		for (ppldds y=0; y<range; y++) {
			w1=range-y;
			w2=y;
			c.setRed((c1.red()*w1/range)+(c2.red()*w2/range));
			c.setGreen((c1.green()*w1/range)+(c2.green()*w2/range));
			c.setBlue((c1.blue()*w1/range)+(c2.blue()*w2/range));
			line(x1,y1+y,x2,y1+y,c);
		}
	}
}

} // EOF namespace grafix
} // EOF namespace ppl6

