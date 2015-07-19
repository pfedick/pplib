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


namespace ppl6 {
namespace grafix {

/*
static void Line_32 (DRAWABLE_DATA &data, int x1, int y1, int x2, int y2, SurfaceColor color)
{
	GRAFIX_FUNCTIONS *fn=data.fn;
	if (!fn->PutPixel) return;
	ppldds xx1,xx2,yy1,yy2,StepX,StepY;
	//ppldd * pp;

	// Sonderfall 1: Anfangs- und Endkoordinaten sind identisch
	if (x1==x2 && y1==y2) {
		fn->PutPixel(data,x1,y1,color);
		return;
	}

	// Zuerst sorgen wir dafuer, dass die Linie immer von links nach rechts gezeichnet
	// werden kann

	if (x1>x2) {
		xx1=x2;
		yy1=y2;
		xx2=x1;
		yy2=y1;
	} else {
		xx1=x1;
		yy1=y1;
		xx2=x2;
		yy2=y2;
	}

	// Danach wird der Steigungswert in X- und Y-Richtung berechnet

	StepX=xx2-xx1;
	StepY=abs(yy2-yy1);

	if (StepX==0) {
		// Sonderfall 2: Gerade Vertikale Linie
		int yylow=lowest(yy1,yy2);
		for (int i=0;i<=StepY;i++) {
			fn->PutPixel(data,xx1,i+yylow,color);
		}
		return;
	}
	if (StepY==0) {
		// Sonderfall 3: Gerade Horizontale Linie
		int xxlow=lowest(xx1,xx2);
		for (int i=0;i<=StepX;i++) fn->PutPixel(data,xxlow+i,yy1,color);
		return;
	}

	// Startposition berechnen
	if (StepX<StepY) {
		if (yy1<yy2) {	// Fall 1
			int x=0;
			for (int i=0;i<=StepY;i++) {
				x=xx1+(i*StepX/StepY);
				fn->PutPixel(data,x,yy1,color);
				yy1++;
			}
			return;
		} else {		// Fall 4
			int x=0;
			for (int i=0;i<=StepY;i++) {
				x=xx1+(i*StepX/StepY);
				fn->PutPixel(data,x,yy1,color);
				yy1--;
			}
			return;
		}
	} else {
		if (yy1<yy2) {	// Fall 2
			int y=0,yy=0;
			for (int i=0;i<=StepX;i++) {
				y=yy1+(i*StepY/StepX);
				if (y>yy) {yy1++; yy=y;}
				fn->PutPixel(data,xx1+i,y,color);
			}
			return;
		} else {		// Fall 3
			int y=0,yy=0;
			for (int i=0;i<=StepX;i++) {
				y=yy1+(i*StepY/StepX);
				if (y>yy) {yy1--; yy=y;}
				fn->PutPixel(data,xx1+i,y,color);
			}
			return;
		}
	}
	return;
}
*/


/* signum function */
static int sgn(int x){
  return (x > 0) ? 1 : (x < 0) ? -1 : 0;
}

/* Bresenham Algorithmus */
static void Line_32 (DRAWABLE_DATA &data, int xstart, int ystart, int xend, int yend, SurfaceColor color)
{
	int x, y, t, dx, dy, incx, incy, pdx, pdy, ddx, ddy, es, el, err;

	/* Entfernung in beiden Dimensionen berechnen */
	dx = xend - xstart;
	dy = yend - ystart;

	/* Vorzeichen des Inkrements bestimmen */
	incx = sgn(dx);
	incy = sgn(dy);
	if(dx<0) dx = -dx;
	if(dy<0) dy = -dy;

	/* feststellen, welche Entfernung größer ist */
	if (dx>dy)
	{
		/* x ist schnelle Richtung */
		pdx=incx; pdy=0;    /* pd. ist Parallelschritt */
		ddx=incx; ddy=incy; /* dd. ist Diagonalschritt */
		es =dy;   el =dx;   /* Fehlerschritte schnell, langsam */
	} else
	{
		/* y ist schnelle Richtung */
		pdx=0;    pdy=incy; /* pd. ist Parallelschritt */
		ddx=incx; ddy=incy; /* dd. ist Diagonalschritt */
		es =dx;   el =dy;   /* Fehlerschritte schnell, langsam */
	}

	/* Initialisierungen vor Schleifenbeginn */
	x = xstart;
	y = ystart;
	err = el/2;
	data.fn->PutPixel(data,x,y,color);

	/* Pixel berechnen */
	for(t=0; t<el; ++t) /* t zaehlt die Pixel, el ist auch Anzahl */
	{
		/* Aktualisierung Fehlerterm */
		err -= es;
		if(err<0)
		{
			/* Fehlerterm wieder positiv (>=0) machen */
			err += el;
			/* Schritt in langsame Richtung, Diagonalschritt */
			x += ddx;
			y += ddy;
		} else
		{
			/* Schritt in schnelle Richtung, Parallelschritt */
			x += pdx;
			y += pdy;
		}
		data.fn->PutPixel(data,x,y,color);
	}
} /* gbham() */


/*
static int Line_32 (SURFACE *data, int x1, int y1, int x2, int y2, COLOR color)
{
	//DLOGLEVEL(9) ("%u->Surface::Line_32(%u,%u,%u,%u,%08X)",data->sclass,x1,y1,x2,y2,color);
	int xx1,xx2,yy1,yy2,Farbe,StepX,StepY;
	ppldd * pp;
	if (!data) return 0;

	ppldb	*base=data->base8;
	ppldd		pitch=data->pitch8;
	ppldd		pitch32=data->pitch32;
	if (!base) return 0;
	if (!pitch) return 0;


	// Nur Zeichnen, wenn innerhalb des Surface
	if (x1<data->clipper.left || y1<data->clipper.top ||
		x2>=data->clipper.right || y2>=data->clipper.bottom) {
		// Secure-Methode verwenden
		return Line_32_Secure(data,x1,y1,x2,y2,color);
	}
	// Sonderfall 1: Anfangs- und Endkoordinaten sind identisch
	if (x1==x2 && y1==y2) {
		data->PutPixel (data,x1,y1,color);
		return 1;
	}

	// Zuerst sorgen wir dafuer, dass die Linie immer von links nach rechts gezeichnet
	// werden kann

	if (x1>x2) {
		xx1=x2;
		yy1=y2;
		xx2=x1;
		yy2=y1;
	} else {
		xx1=x1;
		yy1=y1;
		xx2=x2;
		yy2=y2;
	}
	Farbe=color;

	// Danach wird der Steigungswert in X- und Y-Richtung berechnet

	StepX=xx2-xx1;
	StepY=abs(yy2-yy1);

	// Manchmal wird der Endpunkt nicht gezeichnet
	//PutPixel_32 (data,x2,y2,color);

	if (StepX==0) {
		// Sonderfall 2: Gerade Vertikale Linie
		pp=(ppldd *) (base+lowest(yy1,yy2)*pitch+(xx1<<2));
		for (int i=0;i<=StepY;i++) {
			pp[0]=(ppldd)Farbe;
			pp+=pitch32;
		}
		return 1;
	}
	if (StepY==0) {
		// Sonderfall 3: Gerade Horizontale Linie
		pp=(ppldd *) (base+yy1*pitch+(xx1<<2));
		for (int i=0;i<=StepX;i++) pp[i]=(ppldd)Farbe;
		return 1;
	}

	// Startposition berechnen
	pp=(ppldd *) (base+yy1*pitch+(xx1<<2));
	if (StepX<StepY) {
		if (yy1<yy2) {	// Fall 1
			int x=0;
			for (int i=0;i<=StepY;i++) {
				x=i*StepX/StepY;
				pp[x]=(ppldd)Farbe;
				pp+=pitch32;
			}
			return 1;
		} else {		// Fall 4
			int x=0;
			for (int i=0;i<=StepY;i++) {
				x=i*StepX/StepY;
				pp[x]=(ppldd)Farbe;
				pp-=pitch32;
			}
			return 1;
		}
	} else {
		if (yy1<yy2) {	// Fall 2
			int y=0,yy=0;
			for (int i=0;i<=StepX;i++) {
				y=i*StepY/StepX;
				if (y>yy) {pp+=pitch32; yy=y;}
				pp[i]=(ppldd)Farbe;
			}
			return 1;
		} else {		// Fall 3
			int y=0,yy=0;
			for (int i=0;i<=StepX;i++) {
				y=i*StepY/StepX;
				if (y>yy) {pp-=pitch32; yy=y;}
				pp[i]=(ppldd)Farbe;
			}
			return 1;
		}
	}
	return 1;
}
*/

static void SwapFloat(float *w1, float *w2)
{
	float t;
	t=*w1;
	*w1=*w2;
	*w2=t;
}

static float WuTrunc(float value)			// Ganzzahligen Wert von Value zur�ckgeben
{
	return (float)((ppldds)value);
}

static float WuFrac(float value)			// Kommastellen zurueckgeben
{
	return value-WuTrunc(value);
}

static float WuInvFrac(float value)
{
	return 1-WuFrac(value);
}


static void WuLine (DRAWABLE_DATA &data, float x1, float y1, float x2, float y2, SurfaceColor color)
{
	float grad, xd,yd;//,length,xm,ym;
	float xgap,xend,yend,yf,xf,ygap;
	float brightness1, brightness2;

	GRAFIX_FUNCTIONS *fn=data.fn;
	if (!fn->BlendPixel) return;

	ppldds x,ix1,ix2,iy1,iy2,y;

	xd=(x2-x1);						// Breite und Hoehe der Linie
	yd=(y2-y1);

	if (abs((ppldds)xd)>abs((ppldds)yd)) {			// check line gradient							==> Horizontale Linie
		if (x1>x2) {				// Wenn Linie von rechts nach links gezeichnet wird, tauschen
			SwapFloat(&x1,&x2);		// wir einfach die Koordinaten
			SwapFloat(&y1,&y2);
			xd=(x2-x1);				// Breite und Hoehe der Linie neu berechnen
			yd=(y2-y1);
		}
		grad=yd/xd;					// Gradient der Linie

		// End Point 1
		xend=WuTrunc(x1+0.5f);
		yend=y1+grad*(xend-x1);

		xgap=WuInvFrac(x1+0.5f);

		ix1=(ppldds)xend;
		iy1=(ppldds)yend;

		brightness1=WuInvFrac(yend)*xgap;
		brightness2=WuFrac(yend)*xgap;
		fn->BlendPixel(data,ix1,iy1,color,(int)(brightness1*255));
		fn->BlendPixel(data,ix1,iy1+1,color,(int)(brightness2*255));

		yf=yend+grad;

		// End Point 2
		xend=WuTrunc(x2+0.5f);
		yend=y2+grad*(xend-x2);

		xgap=WuInvFrac(x2-0.5f);

		ix2=(ppldds)xend;
		iy2=(ppldds)yend;

		brightness1=WuInvFrac(yend)*xgap;
		brightness2=WuFrac(yend)*xgap;
		fn->BlendPixel(data,ix2,iy2,color,(int)(brightness1*255));
		fn->BlendPixel(data,ix2,iy2+1,color,(int)(brightness2*255));

		// Main Loop
		for (x=ix1+1; x<ix2; x++) {
			brightness1=WuInvFrac(yf);
			brightness2=WuFrac(yf);
			fn->BlendPixel(data,x,(int)yf,color,(int)(brightness1*255));
			fn->BlendPixel(data,x,(int)yf+1,color,(int)(brightness2*255));
			yf=yf+grad;
		}

	} else {						// check line gradient							==> Vertikale Linie
		if (y1>y2) {				// Wenn Linie von rechts nach links gezeichnet wird, tauschen
			SwapFloat(&x1,&x2);		// wir einfach die Koordinaten
			SwapFloat(&y1,&y2);
			xd=(x2-x1);				// Breite und Hoehe der Linie neu berechnen
			yd=(y2-y1);
		}
		grad=xd/yd;					// Gradient der Linie

		// End Point 1
		yend=WuTrunc(y1+0.5f);
		xend=x1+grad*(yend-y1);

		ygap=WuInvFrac(y1+0.5f);

		ix1=(ppldds)xend;
		iy1=(ppldds)yend;

		brightness1=WuInvFrac(xend)*ygap;
		brightness2=WuFrac(xend)*ygap;
		fn->BlendPixel(data,ix1,iy1,color,(int)(brightness1*255));
		fn->BlendPixel(data,ix1+1,iy1,color,(int)(brightness2*255));

		xf=xend+grad;

		// End Point 2
		yend=WuTrunc(y2+0.5f);
		xend=x2+grad*(yend-y2);

		ygap=WuInvFrac(y2-0.5f);

		ix2=(ppldds)xend;
		iy2=(ppldds)yend;

		brightness1=WuInvFrac(xend)*ygap;
		brightness2=WuFrac(xend)*ygap;
		fn->BlendPixel(data,ix2,iy2,color,(int)(brightness1*255));
		fn->BlendPixel(data,ix2+1,iy2,color,(int)(brightness2*255));

		// Main Loop
		for (y=iy1+1; y<iy2; y++) {
			brightness1=WuInvFrac(xf);
			brightness2=WuFrac(xf);
			fn->BlendPixel(data,(int)xf,y,color,(int)(brightness1*255));
			fn->BlendPixel(data,(int)xf+1,y,color,(int)(brightness2*255));
			xf=xf+grad;
		}

	}
}

static void WuLineThick (DRAWABLE_DATA &data, float x1, float y1, float x2, float y2, SurfaceColor color, int strength)
{
	float grad, xd,yd;//,length,xm,ym;
	float xgap,xend,yend,yf,xf,ygap;
	float brightness1, brightness2;

	ppldds x,ix1,ix2,iy1,iy2,y;

	GRAFIX_FUNCTIONS *fn=data.fn;
	if (!fn->BlendPixel) return;

	xd=(x2-x1);						// Breite und Hoehe der Linie
	yd=(y2-y1);

	if (abs((ppldds)xd)>abs((ppldds)yd)) {			// check line gradient							==> Horizontale Linie
		// Zuerst korrigieren wir die Start- und Zielkoordinaten, damit die Linie mittig ist
		y1-=strength/2;
		y2-=strength/2;

		if (x1>x2) {				// Wenn Linie von rechts nach links gezeichnet wird, tauschen
			SwapFloat(&x1,&x2);		// wir einfach die Koordinaten
			SwapFloat(&y1,&y2);
			xd=(x2-x1);				// Breite und Hoehe der Linie neu berechnen
			yd=(y2-y1);
		}
		grad=yd/xd;					// Gradient der Linie

		// End Point 1
		xend=WuTrunc(x1+0.5f);
		yend=y1+grad*(xend-x1);

		xgap=WuInvFrac(x1+0.5f);

		ix1=(ppldds)xend;
		iy1=(ppldds)yend;

		brightness1=WuInvFrac(yend)*xgap;
		brightness2=WuFrac(yend)*xgap;
		fn->BlendPixel(data,ix1,iy1,color,(int)(brightness1*255));
		for (int i=1;i<strength;i++) fn->BlendPixel(data,ix1,iy1+i,color,255);
		fn->BlendPixel(data,ix1,iy1+strength,color,(int)(brightness2*255));

		yf=yend+grad;

		// End Point 2
		xend=WuTrunc(x2+0.5f);
		yend=y2+grad*(xend-x2);

		xgap=WuInvFrac(x2-0.5f);

		ix2=(ppldds)xend;
		iy2=(ppldds)yend;

		brightness1=WuInvFrac(yend)*xgap;
		brightness2=WuFrac(yend)*xgap;
		fn->BlendPixel(data,ix2,iy2,color,(int)(brightness1*255));
		for (int i=1;i<strength;i++) fn->BlendPixel(data,ix2,iy2+i,color,255);
		fn->BlendPixel(data,ix2,iy2+strength,color,(int)(brightness2*255));

		// Main Loop
		for (x=ix1+1; x<ix2; x++) {
			brightness1=WuInvFrac(yf);
			brightness2=WuFrac(yf);
			fn->BlendPixel(data,x,(ppldds)yf,color,(int)(brightness1*255));
			for (int i=1;i<strength;i++) fn->BlendPixel(data,x,(ppldds)yf+i,color,255);
			fn->BlendPixel(data,x,(ppldds)yf+strength,color,(int)(brightness2*255));
			yf=yf+grad;
		}

	} else {										// check line gradient							==> Vertikale Linie
		// Zuerst korrigieren wir die Start- und Zielkoordinaten, damit die Linie mittig ist
		x1-=strength/2;
		x2-=strength/2;

		if (y1>y2) {				// Wenn Linie von rechts nach links gezeichnet wird, tauschen
			SwapFloat(&x1,&x2);		// wir einfach die Koordinaten
			SwapFloat(&y1,&y2);
			xd=(x2-x1);				// Breite und Hoehe der Linie neu berechnen
			yd=(y2-y1);
		}
		grad=xd/yd;					// Gradient der Linie

		// End Point 1
		yend=WuTrunc(y1+0.5f);
		xend=x1+grad*(yend-y1);

		ygap=WuInvFrac(y1+0.5f);

		ix1=(ppldds)xend;
		iy1=(ppldds)yend;

		brightness1=WuInvFrac(xend)*ygap;
		brightness2=WuFrac(xend)*ygap;
		fn->BlendPixel(data,ix1,iy1,color,(int)(brightness1*255));
		for (int i=1;i<strength;i++) fn->BlendPixel(data,ix1+i,iy1,color,255);
		fn->BlendPixel(data,ix1+strength,iy1,color,(int)(brightness2*255));

		xf=xend+grad;

		// End Point 2
		yend=WuTrunc(y2+0.5f);
		xend=x2+grad*(yend-y2);

		ygap=WuInvFrac(y2-0.5f);

		ix2=(ppldds)xend;
		iy2=(ppldds)yend;

		brightness1=WuInvFrac(xend)*ygap;
		brightness2=WuFrac(xend)*ygap;
		fn->BlendPixel(data,ix2,iy2,color,(int)(brightness1*255));
		for (int i=1;i<strength;i++) fn->BlendPixel(data,ix2+i,iy2,color,255);
		fn->BlendPixel(data,ix2+strength,iy2,color,(int)(brightness2*255));

		// Main Loop
		for (y=iy1+1; y<iy2; y++) {
			brightness1=WuInvFrac(xf);
			brightness2=WuFrac(xf);
			fn->BlendPixel(data,(ppldds)xf,y,color,(int)(brightness1*255));
			for (int i=1;i<strength;i++) fn->BlendPixel(data,(ppldds)xf+i,y,color,255);
			fn->BlendPixel(data,(ppldds)xf+strength,y,color,(int)(brightness2*255));
			xf=xf+grad;
		}

	}
}

static void LineAA (DRAWABLE_DATA &data, int x1, int y1, int x2, int y2, SurfaceColor color, int strength)
{
	if (strength==1) {
		WuLine(data,(float)x1,(float)y1,(float)x2,(float)y2,color);
	} else {
		WuLineThick(data,(float)x1,(float)y1,(float)x2,(float)y2,color,strength);
	}
	return;
}


int CGrafix::InitLines(const RGBFormat &format, GRAFIX_FUNCTIONS *fn)
{
	switch (format) {
		case RGBFormat::A8R8G8B8:		// 32 Bit True Color
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8B8G8R8:
		case RGBFormat::X8R8G8B8:
			fn->LineAA=LineAA;
			fn->Line=Line_32;
			return 1;

		case RGBFormat::A8:
		case RGBFormat::GREY8:
			return 1;

	}
	SetError(1013,"RGBFormat=%s (%i)",(const char*)format.name(),format.format());
	return 0;
}

void CDrawable::line (int x1, int y1, int x2, int y2, const Color &c)
{
	if (fn->Line) fn->Line(data,x1,y1,x2,y2,rgb(c));
}

void CDrawable::line (const Point &start, const Point &end, const Color &c)
{
	if (fn->Line) fn->Line(data,start.x(), start.y(), end.x(), end.y(), rgb(c));
}

void CDrawable::lineAA (int x1, int y1, int x2, int y2, const Color &c, int strength)
{
	if (fn->LineAA) fn->LineAA(data,x1,y1,x2,y2,rgb(c),strength);
}

void CDrawable::lineAA (const Point &start, const Point &end, const Color &c, int strength)
{
	if (fn->LineAA) fn->LineAA(data,start.x(), start.y(), end.x(), end.y(), rgb(c), strength);
}



} // EOF namespace grafix
} // EOF namespace ppl6

