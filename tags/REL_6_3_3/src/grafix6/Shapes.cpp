/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.14 $
 * $Date: 2009/02/13 11:27:09 $
 * $Id: Shapes.cpp,v 1.14 2009/02/13 11:27:09 patrick Exp $
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

static int ClearScreen_8 (SURFACE *surface, COLOR color)
{
	if (!surface->Lock(surface)) return 0;
	int x,y;
	ppluint8 *adr;
	adr=surface->base8;
	for (y=0;y<surface->height;y++) {
		for (x=0;x<surface->width;x++) adr[x]=(ppluint8)color;
		adr+=surface->pitch8;
	}
	surface->Unlock(surface);
	return 1;
}

static int ClearScreen_32 (SURFACE *surface, COLOR color)
{
	if (!surface->Lock(surface)) return 0;
	int x,y;
	ppldd *adr32;
	adr32=surface->base32;
	for (y=0;y<surface->height;y++) {
		for (x=0;x<surface->width;x++) adr32[x]=(ppldd)color;
		adr32+=surface->pitch32;
	}
	surface->Unlock(surface);
	return 1;
}


static int Rect_32 (SURFACE *surface, RECT *r, COLOR Farbe)
{
	//DLOGLEVEL(8) ("%u->CSurface::Box_32(%u,%u,%u,%u,%u,%u)",data->sclass,x1,y1,x2,y2,Farbe,Flags);
	int y,x;
	if (r->left<surface->clipper.left || r->top<surface->clipper.top || r->right>=surface->clipper.right || r->bottom>=surface->clipper.bottom) {
		y=r->bottom-1;
		for (x=r->left;x<r->right;x++) {
			surface->PutPixel(surface,x,r->top,Farbe);
			surface->PutPixel(surface,x,y,Farbe);
		}
		x=r->right-1;
		for (y=r->top;y<r->bottom;y++) {
			surface->PutPixel(surface,r->left,y,Farbe);
			surface->PutPixel(surface,x,y,Farbe);
		}

		return 1;
	}

	ppldd * pp;
	ppldd pitch32=surface->pitch32;
	pp=surface->base32+r->top*pitch32;
	for (x=r->left;x<r->right;x++) pp[x]=(ppldd)Farbe;
	x=r->right-1;
	for (y=r->top+1;y<r->bottom;y++) {
		pp+=pitch32;
		pp[r->left]=(ppldd)Farbe;
		pp[x]=(ppldd)Farbe;
	}
	for (x=r->left;x<r->right;x++) pp[x]=(ppldd)Farbe;
	return 1;
}

static int FillRect_32 (SURFACE *data, RECT *r, COLOR Farbe)
{
	//DLOGLEVEL(8) ("%u->CSurface::Box_32(%u,%u,%u,%u,%u,%u)",data->sclass,x1,y1,x2,y2,Farbe,Flags);

	if (!data->Surface->FitRect (r)) return 0;
	#ifdef HAVE_X86_ASSEMBLER
		RECTDATA d;
		d.tgt=(ppluint32*)(data->base8+r->top*data->pitch8+r->left*data->bytes_per_pixel);
		d.width=r->right-r->left;
		d.height=r->bottom-r->top;
		d.pitch=data->pitch8;
		d.color=Farbe;
		ASM_FillRect32(&d);
	#else
		ppldd * pp;
		ppldds y,x;
		ppldd pitch32=data->pitch32;
		pp=data->base32+r->top*pitch32;
		for (y=r->top;y<r->bottom;y++) {
			for (x=r->left;x<r->right;x++) pp[x]=(ppldd)Farbe;
			pp+=pitch32;
		}
	#endif
	return 1;
}


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

int CGrafix::InitShapes(SURFACE *s)
/*!\brief Initialisiert Funktionen zur Behandlung von Rechtecken
 *
 * \desc
 * Diese Funktion initialisiert eine Reihe von Funktionen zur bearbeitung von Rechtecken.
 * Dabei handelt es sich um die Funktionen:
 * - CLS
 * - Rect
 * - FillRect
 * - Xchange
 * - Invert
 * - Negativ
 *
 * @param[in] s Pointer auf die SURFACE-Struktur der Oberfläche
 * @return Liefert 1 zurück, wenn das Farbformat unterstützt wird, sonst 0
 */
{
	switch (s->rgbformat) {
		case RGBFormat::A8R8G8B8:		// 32 Bit True Color
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8B8G8R8:
		case RGBFormat::X8R8G8B8:
			if (!s->CLS) s->CLS=ClearScreen_32;
			if (!s->Rect) s->Rect=Rect_32;
			if (!s->FillRect) s->FillRect=FillRect_32;
			if (!s->Xchange) s->Xchange=Xchange_32;
			if (!s->Invert) s->Invert=Invert_32;
			if (!s->Negativ) s->Negativ=Negativ_32;
			return 1;
		case RGBFormat::A8:
		case RGBFormat::GREY8:
			if (!s->CLS) s->CLS=ClearScreen_8;
			return 1;
	}
	SetError(1013,"RGBFormat=%s (%i)",GetRGBFormatName(s->rgbformat),s->rgbformat);
	return 0;
}


int CSurface::Rect(int x1, int y1, int x2, int y2, COLOR c)
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
 * @return Die Funktion liefert 1 zurück, wenn das Rechteck gezeichnet werden konnte, sonst 0.
 *
 * \remarks
 * Falls zuvor die Funktion CSurface::SetOrigin aufgerufen wurde, werden die Dort festgelegten Koordinaten
 * zum x- und y-Wert hinzuaddiert.
 *
 */
{
	RECT r;
	r.left=x1+s.originx;
	r.top=y1+s.originy;
	r.right=x2+1+s.originx;
	r.bottom=y2+1+s.originy;
	if (s.Rect) return s.Rect(&s,&r,c);
	SetError(1012,"Rect");
	return 0;
}

int CSurface::FillRect(int x1, int y1, int x2, int y2, COLOR c)
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
 * @return Die Funktion liefert 1 zurück, wenn das Rechteck gezeichnet werden konnte, sonst 0.
 *
 * \remarks
 * Falls zuvor die Funktion CSurface::SetOrigin aufgerufen wurde, werden die Dort festgelegten Koordinaten
 * zum x- und y-Wert hinzuaddiert.
 *
 */
{
	RECT r;
	r.left=x1+s.originx;
	r.top=y1+s.originy;
	r.right=x2+1+s.originx;
	r.bottom=y2+1+s.originy;
	if (s.FillRect) return s.FillRect(&s,&r,c);
	SetError(1012,"Rect");
	return 0;
}

int CSurface::Rect(int x1, int y1, int x2, int y2)
/*!\brief Rechteck zeichnen
 *
 * \desc
 * Diese Funktion zeichnet ein Rechteck an die gewünschte Position. Als Farbe wird die
 * mit CSurface::SetColor gesetzt Farbe verwendet.
 *
 * @param[in] x1 X-Koordinate der oberen linken Ecke
 * @param[in] y1 Y-Koordinate der oberen linken Ecke
 * @param[in] x2 X-Koordinate der unteren rechten Ecke
 * @param[in] y2 Y-Koordinate der unteren rechten Ecke
 * @return Die Funktion liefert 1 zurück, wenn das Rechteck gezeichnet werden konnte, sonst 0.
 *
 * \remarks
 * Falls zuvor die Funktion CSurface::SetOrigin aufgerufen wurde, werden die Dort festgelegten Koordinaten
 * zum x- und y-Wert hinzuaddiert.
 */
{
	RECT r;
	r.left=x1+s.originx;
	r.top=y1+s.originy;
	r.right=x2+1+s.originx;
	r.bottom=y2+1+s.originy;
	if (s.Rect) return s.Rect(&s,&r,s.lastcolor);
	SetError(1012,"Rect");
	return 0;
}

int CSurface::FillRect(int x1, int y1, int x2, int y2)
/*!\brief Ausgefülltes Rechteck zeichnen
 *
 * \desc
 * Diese Funktion zeichnet ein ausgefülltes Rechteck an die gewünschte Position. Als Farbe wird die
 * mit CSurface::SetColor gesetzt Farbe verwendet.
 *
 * @param[in] x1 X-Koordinate der oberen linken Ecke
 * @param[in] y1 Y-Koordinate der oberen linken Ecke
 * @param[in] x2 X-Koordinate der unteren rechten Ecke
 * @param[in] y2 Y-Koordinate der unteren rechten Ecke
 * @return Die Funktion liefert 1 zurück, wenn das Rechteck gezeichnet werden konnte, sonst 0.
 *
 * \remarks
 * Falls zuvor die Funktion CSurface::SetOrigin aufgerufen wurde, werden die Dort festgelegten Koordinaten
 * zum x- und y-Wert hinzuaddiert.
 */
{
	RECT r;
	r.left=x1+s.originx;
	r.top=y1+s.originy;
	r.right=x2+1+s.originx;
	r.bottom=y2+1+s.originy;
	if (s.FillRect) return s.FillRect(&s,&r,s.lastcolor);
	SetError(1012,"Rect");
	return 0;
}

int CSurface::Rect(RECT *r)
/*!\brief Rechteck zeichnen
 *
 * \desc
 * Diese Funktion zeichnet ein Rechteck an die gewünschte Position. Als Farbe wird die
 * mit CSurface::SetColor gesetzt Farbe verwendet.
 *
 * @param[in] r Pointer auf eine RECT-Struktur mit den gewünschten Koordinaten.
 * Dabei ist darauf zu achten, dass die Koordinaten für die rechte untere Ecke und einen Pixel
 * erhöht sein müssen.
 * @return Die Funktion liefert 1 zurück, wenn das Rechteck gezeichnet werden konnte, sonst 0.
 *
 * \remarks
 * Falls zuvor die Funktion CSurface::SetOrigin aufgerufen wurde, werden die Dort festgelegten Koordinaten
 * zum x- und y-Wert hinzuaddiert.
 */
{
	if (!r) return 0;
	RECT rr;
	rr.left=r->left+s.originx;
	rr.top=r->top+s.originy;
	rr.right=r->right+s.originx;
	rr.bottom=r->bottom+s.originy;
	if (s.Rect) return s.Rect(&s,&rr,s.lastcolor);
	SetError(1012,"Rect");
	return 0;
}

int CSurface::FillRect(RECT *r)
/*!\brief Ausgefülltes Rechteck zeichnen
 *
 * \desc
 * Diese Funktion zeichnet ein ausgefülltes Rechteck an die gewünschte Position. Als Farbe wird die
 * mit CSurface::SetColor gesetzt Farbe verwendet.
 *
 * @param[in] r Pointer auf eine RECT-Struktur mit den gewünschten Koordinaten.
 * Dabei ist darauf zu achten, dass die Koordinaten für die rechte untere Ecke und einen Pixel
 * erhöht sein müssen.
 * @return Die Funktion liefert 1 zurück, wenn das Rechteck gezeichnet werden konnte, sonst 0.
 *
 * \remarks
 * Falls zuvor die Funktion CSurface::SetOrigin aufgerufen wurde, werden die Dort festgelegten Koordinaten
 * zum x- und y-Wert hinzuaddiert.
 */
{
	if (!r) return 0;
	RECT rr;
	rr.left=r->left+s.originx;
	rr.top=r->top+s.originy;
	rr.right=r->right+s.originx;
	rr.bottom=r->bottom+s.originy;
	if (s.FillRect) return s.FillRect(&s,&rr,s.lastcolor);
	SetError(1012,"Rect");
	return 0;
}

int CSurface::Rect(RECT *r, COLOR c)
/*!\brief Rechteck zeichnen
 *
 * \desc
 * Diese Funktion zeichnet ein Rechteck an die gewünschte Position.
 *
 * @param[in] r Pointer auf eine RECT-Struktur mit den gewünschten Koordinaten.
 * Dabei ist darauf zu achten, dass die Koordinaten für die rechte untere Ecke und einen Pixel
 * erhöht sein müssen.
 * @param[in] c Farbe des Rechtecks
 * @return Die Funktion liefert 1 zurück, wenn das Rechteck gezeichnet werden konnte, sonst 0.
 *
 * \remarks
 * Falls zuvor die Funktion CSurface::SetOrigin aufgerufen wurde, werden die Dort festgelegten Koordinaten
 * zum x- und y-Wert hinzuaddiert.
 */
{
	if (!r) return 0;
	RECT rr;
	rr.left=r->left+s.originx;
	rr.top=r->top+s.originy;
	rr.right=r->right+s.originx;
	rr.bottom=r->bottom+s.originy;
	if (s.Rect) return s.Rect(&s,&rr,c);
	SetError(1012,"Rect");
	return 0;
}

int CSurface::FillRect(RECT *r, COLOR c)
/*!\brief Ausgefülltes Rechteck zeichnen
 *
 * \desc
 * Diese Funktion zeichnet ein ausgefülltes Rechteck an die gewünschte Position.
 *
 * @param[in] r Pointer auf eine RECT-Struktur mit den gewünschten Koordinaten.
 * Dabei ist darauf zu achten, dass die Koordinaten für die rechte untere Ecke und einen Pixel
 * erhöht sein müssen.
 * @param[in] c Farbe des Rechtecks
 * @return Die Funktion liefert 1 zurück, wenn das Rechteck gezeichnet werden konnte, sonst 0.
 *
 * \remarks
 * Falls zuvor die Funktion CSurface::SetOrigin aufgerufen wurde, werden die Dort festgelegten Koordinaten
 * zum x- und y-Wert hinzuaddiert.
 */
{
	if (!r) return 0;
	RECT rr;
	rr.left=r->left+s.originx;
	rr.top=r->top+s.originy;
	rr.right=r->right+s.originx;
	rr.bottom=r->bottom+s.originy;
	if (s.FillRect) return s.FillRect(&s,&rr,c);
	SetError(1012,"Rect");
	return 0;
}


int CSurface::Xchange(int x1, int y1, int x2, int y2, COLOR color, COLOR ersatzfarbe)
/*!\brief Farben ersetzen
 *
 * \desc
 * Mit dieser Funktion werden sämtliche Pixel innerhalb eines Rechtecks, die der Farbe \p color
 * entsprechen, durch die \p ersatzfarbe ersetzt
 * @param[in] x1 X-Koordinate der oberen linken Ecke
 * @param[in] y1 Y-Koordinate der oberen linken Ecke
 * @param[in] x2 X-Koordinate der unteren rechten Ecke
 * @param[in] y2 Y-Koordinate der unteren rechten Ecke
 * @param color Alte Farbe
 * @param ersatzfarbe Neu Farbe
 * @return Die Funktion gibt ber Erfolg 1 zurück, sonst 0.
 */
{
	if (s.Xchange) return s.Xchange(&s,x1+s.originx,y1+s.originy,x2+s.originx,y2+s.originy,color,ersatzfarbe);
	SetError(1012,"Xchange");
	return 0;
}

int CSurface::Invert(int x1, int y1, int x2, int y2, COLOR color1, COLOR color2)
/*!\brief Farben vertauschen
 *
 * \desc
 * Mit dieser Funktion werden sämtliche Pixel innerhalb eines Rechtecks, die der Farbe \p color1
 * entsprechen, durch \p color2 ersetzt, und alle Farben mit dem Farbwert \p color2 werden durch
 * \p color1 ersetzt.
 * @param[in] x1 X-Koordinate der oberen linken Ecke
 * @param[in] y1 Y-Koordinate der oberen linken Ecke
 * @param[in] x2 X-Koordinate der unteren rechten Ecke
 * @param[in] y2 Y-Koordinate der unteren rechten Ecke
 * @param[in] color1 Erste Farbe
 * @param[in] color2 Zweite Farbe
 * @return Die Funktion gibt ber Erfolg 1 zurück, sonst 0.
 */
{
	if (s.Invert) return s.Invert(&s,x1+s.originx,y1+s.originy,x2+s.originx,y2+s.originy,color1,color2);
	SetError(1012,"Invert");
	return 0;
}

int CSurface::Negativ(int x1, int y1, int x2, int y2)
/*!\brief Negativ-Farben erstellen
 *
 * \desc
 * Mit dieser Funktion werden sämtliche Pixel innerhalb eines Rechtecks durch ihren negativen Wert
 * ersetzt.
 *
 * @param[in] x1 X-Koordinate der oberen linken Ecke
 * @param[in] y1 Y-Koordinate der oberen linken Ecke
 * @param[in] x2 X-Koordinate der unteren rechten Ecke
 * @param[in] y2 Y-Koordinate der unteren rechten Ecke
 * @return Die Funktion gibt ber Erfolg 1 zurück, sonst 0.
 */
{
	if (s.Negativ) return s.Negativ(&s,x1+s.originx,y1+s.originy,x2+s.originx,y2+s.originy);
	SetError(1012,"Negativ");
	return 0;
}


void CSurface::FloodFill (int x, int y, COLOR color, COLOR border)
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
		if (this->GetPixel(i, y) == border) {
			break;
		}
		this->PutPixel(i, y, color);
		leftLimit = i;
	}
	if (leftLimit == (-1)) {
		return;
	}
	/* Seek right */
	rightLimit = x;
	for (i = (x+1); (i < s.width); i++) {
		if (this->GetPixel(i, y) == border) {
			break;
		}
		this->PutPixel(i, y, color);
		rightLimit = i;
	}
	/* Look at lines above and below and start paints */
	/* Above */
	if (y > 0) {
		lastBorder = 1;
		for (i = leftLimit; (i <= rightLimit); i++) {
			ppldd c;
			c = this->GetPixel(i, y-1);
			if (lastBorder) {
				if ((c != border) && (c != color)) {
					FloodFill(i, y-1,color,border);
					lastBorder = 0;
				}
			} else if ((c == border) || (c == color)) {
				lastBorder = 1;
			}
		}
	}
	/* Below */
	if (y < ((s.height) - 1)) {
		lastBorder = 1;
		for (i = leftLimit; (i <= rightLimit); i++) {
			ppldd c;
			c = this->GetPixel(i, y+1);
			if (lastBorder) {
				if ((c != border) && (c != color)) {
					FloodFill(i, y+1,color,border);
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
void CSurface::Elipse (int x, int y, int radiusx, int radiusy, COLOR Farbe, int Flags)
{
	int d;
	int i,x1,y1,x2=0,y2=0;
	for (i=0;i<1025;i++) {
		x1 = x + (sinus1024(i) * radiusx/1024);
		y1 = y + (cosinus1024(i) * radiusy/1024);
		if (i > 0) {
			d=abs(x2-x1)+abs(y2-y1);
			if (d > 1)
				Line (x1, y1, x2, y2, Farbe);
			else if (d==1)
				PutPixel (x1,y1,Farbe);
		}
		x2 = x1;
		y2 = y1;
	}
	if (Flags &1) FloodFill (x, y, Farbe, Farbe);
}

void CSurface::Elipse (int x, int y, int radiusx, int radiusy, COLOR Farbe, int Flags, COLOR Fuellfarbe, int startwinkel, int endwinkel)
{
	int d;
	int i,x1,y1,x2,y2;
	if (startwinkel!=endwinkel) {
		x2 = x + (sinus1024(startwinkel) * radiusx/1024);
		y2 = y + (cosinus1024(startwinkel) * radiusy/1024);
		PutPixel (x2,y2,Farbe);

		for (i=startwinkel;i<endwinkel+1;i++) {
			x1 = x + (sinus1024(i) * radiusx/1024);
			y1 = y + (cosinus1024(i) * radiusy/1024);
			if (i > 0) {
				d=abs(x2-x1)+abs(y2-y1);
				if (d > 1)
					Line (x1, y1, x2, y2, Farbe);
				else if (d==1)
					PutPixel (x1,y1,Farbe);
			}
			x2 = x1;
			y2 = y1;
		}
		if (Flags &1) {
			x1 = x + (sinus1024((startwinkel+endwinkel)/2) * (radiusx-2)/1024);
			y1 = y + (cosinus1024((startwinkel+endwinkel)/2) * (radiusy-2)/1024);
			FloodFill (x1, y1, Fuellfarbe, Farbe);
		}
	}
}

void CSurface::Circle (int x, int y, int radius, COLOR Farbe, int Flags)
{
	Elipse(x,y,radius,radius,Farbe,Flags);
}

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

void CSurface::ColorGradient(int x1, int y1, int x2, int y2, COLOR color1, COLOR color2, int direction)
{
	RGBA col1, col2;
	Surface2RGB(color1,&col1);
	Surface2RGB(color2,&col2);
	ColorGradient(x1,y1,x2,y2,&col1,&col2,direction);
}

void CSurface::ColorGradient(int x1, int y1, int x2, int y2, RGBA *color1, RGBA *color2, int direction)
{
	RGBA c;
	ppldd w1,w2;
	int range;
	c.alpha=255;
	if (direction==0) {
		range=x2-x1+1;
		for (ppldds x=0; x<range; x++) {
			w1=range-x;
			w2=x;
			c.red=(ppldb)((color1->red*w1/range)+(color2->red*w2/range));
			c.green=(ppldb)((color1->green*w1/range)+(color2->green*w2/range));
			c.blue=(ppldb)((color1->blue*w1/range)+(color2->blue*w2/range));
			Line(x1+x,y1,x1+x,y2,RGB(&c));
		}
	} else {
		range=y2-y1+1;
		for (ppldds y=0; y<range; y++) {
			w1=range-y;
			w2=y;
			c.red=(ppldb)((color1->red*w1/range)+(color2->red*w2/range));
			c.green=(ppldb)((color1->green*w1/range)+(color2->green*w2/range));
			c.blue=(ppldb)((color1->blue*w1/range)+(color2->blue*w2/range));
			Line(x1,y1+y,x2,y1+y,RGB(&c));
		}
	}
}


} // EOF namespace grafix
} // EOF namespace ppl6

