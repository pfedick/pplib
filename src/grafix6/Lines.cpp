/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.8 $
 * $Date: 2009/01/25 01:03:38 $
 * $Id: Lines.cpp,v 1.8 2009/01/25 01:03:38 patrick Exp $
 *
 *******************************************************************************
 * Copyright (c) 2008, Patrick Fedick <patrick@pfp.de>
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

static int Line_32_Secure (SURFACE *data, int x1, int y1, int x2, int y2, COLOR color)
{
	//DLOGLEVEL(9) ("%u->Surface::Line_32_Secure(%u,%u,%u,%u,%08X)",data->sclass,x1,y1,x2,y2,color);
	ppldds xx1,xx2,yy1,yy2,Farbe,StepX,StepY;
	ppldd * pp;

	ppldb	*base=data->base8;
	ppldd		pitch=data->pitch8;
	ppldd		pitch32=data->pitch32;

	// Sonderfall 1: Anfangs- und Endkoordinaten sind identisch
	if (x1==x2 && y1==y2) {
		data->PutPixel(data,x1,y1,color);
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
		int yylow=lowest(yy1,yy2);
		for (int i=0;i<=StepY;i++) {
			data->PutPixel(data,xx1,i+yylow,Farbe);
		}
		return 1;
	}
	if (StepY==0) {
		// Sonderfall 3: Gerade Horizontale Linie
		int xxlow=lowest(xx1,xx2);
		for (int i=0;i<=StepX;i++) data->PutPixel(data,xxlow+i,yy1,Farbe);
		return 1;
	}

	// Startposition berechnen
	// TODO: PutPixel_32_Secure muss noch eingebaut werden!
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


static void WuLine (SURFACE *surface, float x1, float y1, float x2, float y2,COLOR color)
{
	float grad, xd,yd;//,length,xm,ym;
	float xgap,xend,yend,yf,xf,ygap;
	float brightness1, brightness2;

	if (!surface->BlendPixel) return;

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
		surface->BlendPixel(surface,ix1,iy1,color,brightness1);
		surface->BlendPixel(surface,ix1,iy1+1,color,brightness2);

		yf=yend+grad;

		// End Point 2
		xend=WuTrunc(x2+0.5f);
		yend=y2+grad*(xend-x2);

		xgap=WuInvFrac(x2-0.5f);

		ix2=(ppldds)xend;
		iy2=(ppldds)yend;

		brightness1=WuInvFrac(yend)*xgap;
		brightness2=WuFrac(yend)*xgap;
		surface->BlendPixel(surface,ix2,iy2,color,brightness1);
		surface->BlendPixel(surface,ix2,iy2+1,color,brightness2);

		// Main Loop
		for (x=ix1+1; x<ix2; x++) {
			brightness1=WuInvFrac(yf);
			brightness2=WuFrac(yf);
			surface->BlendPixel(surface,x,(ppldds)yf,color,brightness1);
			surface->BlendPixel(surface,x,(ppldds)yf+1,color,brightness2);
			yf=yf+grad;
		}

	} else {										// check line gradient							==> Vertikale Linie
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
		surface->BlendPixel(surface,ix1,iy1,color,brightness1);
		surface->BlendPixel(surface,ix1+1,iy1,color,brightness2);

		xf=xend+grad;

		// End Point 2
		yend=WuTrunc(y2+0.5f);
		xend=x2+grad*(yend-y2);

		ygap=WuInvFrac(y2-0.5f);

		ix2=(ppldds)xend;
		iy2=(ppldds)yend;

		brightness1=WuInvFrac(xend)*ygap;
		brightness2=WuFrac(xend)*ygap;
		surface->BlendPixel(surface,ix2,iy2,color,brightness1);
		surface->BlendPixel(surface,ix2+1,iy2,color,brightness2);

		// Main Loop
		for (y=iy1+1; y<iy2; y++) {
			brightness1=WuInvFrac(xf);
			brightness2=WuFrac(xf);
			surface->BlendPixel(surface,(ppldds)xf,y,color,brightness1);
			surface->BlendPixel(surface,(ppldds)xf+1,y,color,brightness2);
			xf=xf+grad;
		}

	}
}

static void WuLineThick (SURFACE *surface, float x1, float y1, float x2, float y2,COLOR color, int strength)
{
	float grad, xd,yd;//,length,xm,ym;
	float xgap,xend,yend,yf,xf,ygap;
	float brightness1, brightness2;

	ppldds x,ix1,ix2,iy1,iy2,y;

	if (!surface->BlendPixel) return;

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
		surface->BlendPixel(surface,ix1,iy1,color,brightness1);
		for (int i=1;i<strength;i++) surface->BlendPixel(surface,ix1,iy1+i,color,1.0f);
		surface->BlendPixel(surface,ix1,iy1+strength,color,brightness2);

		yf=yend+grad;

		// End Point 2
		xend=WuTrunc(x2+0.5f);
		yend=y2+grad*(xend-x2);

		xgap=WuInvFrac(x2-0.5f);

		ix2=(ppldds)xend;
		iy2=(ppldds)yend;

		brightness1=WuInvFrac(yend)*xgap;
		brightness2=WuFrac(yend)*xgap;
		surface->BlendPixel(surface,ix2,iy2,color,brightness1);
		for (int i=1;i<strength;i++) surface->BlendPixel(surface,ix2,iy2+i,color,1.0f);
		surface->BlendPixel(surface,ix2,iy2+strength,color,brightness2);

		// Main Loop
		for (x=ix1+1; x<ix2; x++) {
			brightness1=WuInvFrac(yf);
			brightness2=WuFrac(yf);
			surface->BlendPixel(surface,x,(ppldds)yf,color,brightness1);
			for (int i=1;i<strength;i++) surface->BlendPixel(surface,x,(ppldds)yf+i,color,1.0f);
			surface->BlendPixel(surface,x,(ppldds)yf+strength,color,brightness2);
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
		surface->BlendPixel(surface,ix1,iy1,color,brightness1);
		for (int i=1;i<strength;i++) surface->BlendPixel(surface,ix1+i,iy1,color,1.0f);
		surface->BlendPixel(surface,ix1+strength,iy1,color,brightness2);

		xf=xend+grad;

		// End Point 2
		yend=WuTrunc(y2+0.5f);
		xend=x2+grad*(yend-y2);

		ygap=WuInvFrac(y2-0.5f);

		ix2=(ppldds)xend;
		iy2=(ppldds)yend;

		brightness1=WuInvFrac(xend)*ygap;
		brightness2=WuFrac(xend)*ygap;
		surface->BlendPixel(surface,ix2,iy2,color,brightness1);
		for (int i=1;i<strength;i++) surface->BlendPixel(surface,ix2+i,iy2,color,1.0f);
		surface->BlendPixel(surface,ix2+strength,iy2,color,brightness2);

		// Main Loop
		for (y=iy1+1; y<iy2; y++) {
			brightness1=WuInvFrac(xf);
			brightness2=WuFrac(xf);
			surface->BlendPixel(surface,(ppldds)xf,y,color,brightness1);
			for (int i=1;i<strength;i++) surface->BlendPixel(surface,(ppldds)xf+i,y,color,1.0f);
			surface->BlendPixel(surface,(ppldds)xf+strength,y,color,brightness2);
			xf=xf+grad;
		}

	}
}

static int LineAA (SURFACE *surface, int x1, int y1, int x2, int y2,COLOR color, int strength)
{
	if (strength==1) {
		WuLine(surface,(float)x1,(float)y1,(float)x2,(float)y2,color);
	} else {
		WuLineThick(surface,(float)x1,(float)y1,(float)x2,(float)y2,color,strength);
	}
	return 1;
}


int CGrafix::InitLines(SURFACE *s)
{
	switch (s->rgbformat) {
		case RGBFormat::A8R8G8B8:		// 32 Bit True Color
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8B8G8R8:
		case RGBFormat::X8R8G8B8:
			if (!s->LineAA) s->LineAA=LineAA;
			if (!s->Line) s->Line=Line_32;
			return 1;

		case RGBFormat::A8:
		case RGBFormat::GREY8:
			return 1;

	}
	SetError(1013,"RGBFormat=%s (%i)",GetRGBFormatName(s->rgbformat),s->rgbformat);
	return 0;

}

int CSurface::Line(int x1, int y1, int x2, int y2, COLOR c)
{
	s.lastcolor=c;
	s.lastx=x2;
	s.lasty=y2;
	if (s.Line) return s.Line(&s,x1+s.originx,y1+s.originy,x2+s.originx,y2+s.originy,c);
	SetError(1012,"Line");
	return 0;
}

int CSurface::Line(int x2, int y2, COLOR c)
{
	s.lastcolor=c;
	if (s.Line) {
		int ret=s.Line(&s,s.lastx+s.originx,s.lasty+s.originy,x2+s.originx,y2+s.originy,c);
		s.lastx=x2;
		s.lasty=y2;
		return ret;
	}
	SetError(1012,"Line");
	return 0;
}

int CSurface::LineAA(int x1, int y1, int x2, int y2, COLOR c, int strength)
{
	s.lastcolor=c;
	s.lastx=x2;
	s.lasty=y2;
	if (s.LineAA) return s.LineAA(&s,x1+s.originx,y1+s.originy,x2+s.originx,y2+s.originy,c,strength);
	SetError(1012,"Line");
	return 0;
}

int CSurface::LineAA(int x2, int y2, COLOR c, int strength)
{
	s.lastcolor=c;
	if (s.LineAA) {
		int ret=s.LineAA(&s,s.lastx+s.originx,s.lasty+s.originy,x2+s.originx,y2+s.originy,c,strength);
		s.lastx=x2;
		s.lasty=y2;
		return ret;
	}
	SetError(1012,"Line");
	return 0;
}


} // EOF namespace grafix
} // EOF namespace ppl6

