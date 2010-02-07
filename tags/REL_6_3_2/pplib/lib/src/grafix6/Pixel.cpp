/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.11 $
 * $Date: 2009/02/07 16:58:17 $
 * $Id: Pixel.cpp,v 1.11 2009/02/07 16:58:17 patrick Exp $
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
#ifdef __x86_64__
#ifdef __ELF__
#define ASM_ALPHAPIXELFAST32
#endif
#endif
#endif

extern "C" {
#ifdef ASM_ALPHAPIXELFAST32
	int AlphaPixelFast_32 (ppl6::grafix::SURFACE *surface, int x, int y, ppl6::grafix::COLOR color);
#endif
}



namespace ppl6 {
namespace grafix {

/*
 * Dummies
 */

static int AlphaPixel_Dummy (SURFACE *surface, int x, int y, COLOR color)
{
	return 0;
}


/*
 * 8 Bit
 */
static void PutPixel_8 (SURFACE *surface, int x, int y, COLOR color)
{
	if (x<surface->clipper.left || y<surface->clipper.top || x>surface->clipper.right || y>surface->clipper.bottom) return;
	surface->base8[surface->pitch8*y+x]=(ppluint8)(color&0xff);
}

static void PutPixelFast_8 (SURFACE *surface, int x, int y, COLOR color)
{
	surface->base8[surface->pitch8*y+x]=(ppluint8)(color&0xff);
}

static COLOR GetPixel_8 (SURFACE *surface, int x, int y)
{
	if (x<surface->clipper.left || y<surface->clipper.top || x>surface->clipper.right || y>surface->clipper.bottom) return 0;
	return ((COLOR)surface->base8[surface->pitch8*y+x]);
}

static COLOR GetPixelFast_8 (SURFACE *surface, int x, int y)
{
	return ((COLOR)surface->base8[surface->pitch8*y+x]);
}

/*
static COLOR BlendPixel_8 (SURFACE *surface, int x, int y, COLOR color, float brightness)
{
	return 0;
}
*/

static COLOR BlendPixel_A8 (SURFACE *surface, int x, int y, COLOR color, float brightness)
{
	if (x<surface->clipper.left || y<surface->clipper.top || x>surface->clipper.right || y>surface->clipper.bottom) return 0;
	COLOR screen,c;
	screen=surface->GetPixel(surface,x,y);
	c=surface->RGBBlend(screen,color,brightness);
	surface->PutPixel(surface,x,y,c);
	return c;
}

static COLOR BlendPixel_A8_255 (SURFACE *surface, int x, int y, COLOR color, int brightness)
{
	if (x<surface->clipper.left || y<surface->clipper.top || x>surface->clipper.right || y>surface->clipper.bottom) return 0;
	COLOR screen,c;
	screen=surface->GetPixel(surface,x,y);
	c=surface->RGBBlend255(screen,color,brightness);
	surface->PutPixel(surface,x,y,c);
	return c;
}

/*
 * 32 Bit
 */
static void PutPixel_32 (SURFACE *surface, int x, int y, COLOR color)
{
	if (x<surface->clipper.left || y<surface->clipper.top || x>surface->clipper.right || y>surface->clipper.bottom) return;
	surface->base32[surface->pitch32*y+x]=(ppluint32)color;
}

static void PutPixelFast_32 (SURFACE *surface, int x, int y, COLOR color)
{
	surface->base32[surface->pitch32*y+x]=(ppluint32)color;
}


static COLOR GetPixel_32 (SURFACE *surface, int x, int y)
{
	if (x<surface->clipper.left || y<surface->clipper.top || x>surface->clipper.right || y>surface->clipper.bottom) return 0;
	return ((COLOR)surface->base32[surface->pitch32*y+x]);
}

static COLOR GetPixelFast_32 (SURFACE *surface, int x, int y)
{
	return ((COLOR)surface->base32[surface->pitch32*y+x]);
}

static COLOR BlendPixel_32 (SURFACE *surface, int x, int y, COLOR color, float brightness)
{
	if (x<surface->clipper.left || y<surface->clipper.top || x>surface->clipper.right || y>surface->clipper.bottom) return 0;
	COLOR screen,c;
	screen=((COLOR)surface->base32[surface->pitch32*y+x]);
	c=surface->RGBBlend(screen,color,brightness);
	surface->base32[surface->pitch32*y+x]=(ppluint32)c;
	return c;
}


static COLOR BlendPixel_32_255 (SURFACE *surface, int x, int y, COLOR color, int brightness)
{
	if (x<surface->clipper.left || y<surface->clipper.top || x>surface->clipper.right || y>surface->clipper.bottom) return 0;
	COLOR screen,c;
	screen=((COLOR)surface->base32[surface->pitch32*y+x]);
	c=surface->RGBBlend255(screen,color,brightness);
	surface->base32[surface->pitch32*y+x]=(ppluint32)c;
	return c;
}

static int AlphaPixel_32 (SURFACE *surface, int x, int y, COLOR color)
{
	if (x<surface->clipper.left || y<surface->clipper.top || x>surface->clipper.right || y>surface->clipper.bottom) return 0;
	COLOR screen,c;
	screen=((COLOR)surface->base32[surface->pitch32*y+x]);
	c=surface->RGBBlend255(screen,color,(color>>24));
	surface->base32[surface->pitch32*y+x]=(ppluint32)c;
	return c;
}


#ifndef ASM_ALPHAPIXELFAST32
static int AlphaPixelFast_32 (SURFACE *surface, int x, int y, COLOR color)
{
	COLOR screen,c;
	screen=((COLOR)surface->base32[surface->pitch32*y+x]);
	c=surface->RGBBlend255(screen,color,(color>>24));
	surface->base32[surface->pitch32*y+x]=(ppluint32)c;
	return c;
}
#endif

int CGrafix::InitPixel(SURFACE *s)
/*!\brief Initialisierung der Pixel-Funktionen des Surface
 *
 * Mit diese internen Funktion werden die Pixel-Funktionen in Abhängigkeit des Farbformats der Oberfläche
 * initialisiert.
 *
 * @param s Pointer auf die SURFACE-Struktur
 * @return Liefert 1 zurück, wenn die Funktionen erfolgreich initialisiert wurden, 0, wenn das Farbformat
 * nicht unterstützt wird.
 */
{

	switch (s->rgbformat) {
		case RGBFormat::A8R8G8B8:		// 32 Bit True Color
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8B8G8R8:
		case RGBFormat::X8R8G8B8:
			if (!s->PutPixelSecure) s->PutPixelSecure=PutPixel_32;
			if (!s->PutPixelFast) s->PutPixelFast=PutPixelFast_32;
			if (!s->PutPixel) s->PutPixel=PutPixel_32;
			if (!s->GetPixelSecure) s->GetPixel=GetPixel_32;
			if (!s->GetPixelFast) s->GetPixel=GetPixelFast_32;
			if (!s->GetPixel) s->GetPixel=GetPixel_32;
			if (!s->BlendPixel) s->BlendPixel=BlendPixel_32;
			if (!s->BlendPixel255) s->BlendPixel255=BlendPixel_32_255;
			if (!s->AlphaPixel) s->AlphaPixel=AlphaPixel_32;
			if (!s->AlphaPixelSecure) s->AlphaPixelSecure=AlphaPixel_32;
			if (!s->AlphaPixelFast) s->AlphaPixelFast=AlphaPixelFast_32;

			//if (!data.ClearScreen) data.ClearScreen=ClearScreen_32;
			return 1;
		case RGBFormat::A8:
		case RGBFormat::GREY8:
			if (!s->PutPixelSecure) s->PutPixel=PutPixel_8;
			if (!s->PutPixelFast) s->PutPixel=PutPixelFast_8;
			if (!s->PutPixel) s->PutPixel=PutPixel_8;
			if (!s->GetPixelSecure) s->GetPixel=GetPixel_8;
			if (!s->GetPixelFast) s->GetPixel=GetPixelFast_8;
			if (!s->GetPixel) s->GetPixel=GetPixel_8;
			if (!s->BlendPixel) s->BlendPixel=BlendPixel_A8;
			if (!s->BlendPixel255) s->BlendPixel255=BlendPixel_A8_255;
			if (!s->AlphaPixel) s->AlphaPixel=AlphaPixel_Dummy;
			if (!s->AlphaPixelSecure) s->AlphaPixelSecure=AlphaPixel_Dummy;
			if (!s->AlphaPixelFast) s->AlphaPixelFast=AlphaPixel_Dummy;
			return 1;

	}
	SetError(1013,"RGBFormat=%s (%i)",GetRGBFormatName(s->rgbformat),s->rgbformat);
	return 0;
}

void CSurface::PutPixel(int x, int y, COLOR c)
/*!\brief Pixel zeichnen
 *
 * \desc
 * Mit dieser Funktion wird ein einzelner Pixel auf die Oberfläche gezeichnet.
 *
 * @param[in] x Die X-Koordinate des Pixels
 * @param[in] y Die Y-Koordinate des Pixels
 * @param[in] c Die Der Farbwert des Pixels
 *
 * Falls zuvor die Funktion CSurface::SetOrigin aufgerufen wurde, werden die Dort festgelegten Koordinaten
 * zum x- und y-Wert hinzuaddiert.
 *
 * \remarks
 * Es muss sichergestellt sein, dass die Oberfläche beschreibbar ist. Das heisst je nach Surface-Typ
 * und verwendeter Grafik-Engine muss zuvor CSurface::Lock aufgerufen worden sein.
 */
{
	s.PutPixel(&s,x+s.originx,y+s.originy,c);
}

int CSurface::AlphaPixel(int x, int y, COLOR c)
/*!\brief Pixel unter Berücksichtigung seines Alpha-Wertes zeichnen
 *
 * \desc
 * Mit dieser Funktion wird ein einzelner Pixel unter Berücksichtigung seines Alpha-Kanal-Werts
 * auf die Oberfläche gezeichnet. Ein höherer Alpha-Wert bewirkt, dass der Pixel intensiver erscheint.
 * Bei einem Wert von 0 bleibt der Hintergrund vollständig erhalten, bei einem Wert von 255 wird
 * er komplett mit dem Farbwert \p c überschrieben.
 *
 * @param[in] x Die X-Koordinate des Pixels
 * @param[in] y Die Y-Koordinate des Pixels
 * @param[in] c Die Der Farbwert des Pixels inklusive Alpha-Kanal
 * \return Die Funktion liefert den tatsächlich geschriebenen Farbwert zurück.
 *
 * Falls zuvor die Funktion CSurface::SetOrigin aufgerufen wurde, werden die Dort festgelegten Koordinaten
 * zum x- und y-Wert hinzuaddiert.
 *
 * \remarks
 * Es muss sichergestellt sein, dass die Oberfläche beschreibbar ist. Das heisst je nach Surface-Typ
 * und verwendeter Grafik-Engine muss zuvor CSurface::Lock aufgerufen worden sein.
 */
{
	return s.AlphaPixel(&s,x+s.originx,y+s.originy,c);
}

int CSurface::FastPixel(int flag)
/*!\brief Schneller Pixelroutine verwendet
 *
 * \desc
 * Ein Aufruf dieser Funktion mit \p true bewirkt, dass die Prüfung ob ein Pixel im erlaubten
 * Bereich liegt, abgeschaltet wird. Dadurch sind die Funktionen CSurface::PutPixel und
 * CSurface::GetPixel deutlich schneller. Die Anwendung muss jedoch sicherstellen, dass
 * kein Zugriff ausserhalb des erlaubten Bereiches stattfindet, da es sonst zu Schutzverletzungen
 * und Datenverlust kommen kann!
  *
 * \param[in] flag 1=schnellere Routinen, 0=sichere Routinen
 * \return Die Funktion liefert 1 zurück, wenn zuvor die Schnellen Routinen verwendet wurden, sonst
 * 0. Der Wert kann somit verwendet werden, um nach Ende einer Operation den alten Zustand
 * wiederherzustellen.
 *
 * \since
 * Die Funktion wurde mit Version 6.2.4 eingeführt.
 */
{
	int ret=0;
	if (s.PutPixel==s.PutPixelFast) ret=1;
	if (flag) {
		s.PutPixel=s.PutPixelFast;
		s.GetPixel=s.GetPixelFast;
		s.AlphaPixel=s.AlphaPixelFast;
		return ret;
	}
	s.PutPixel=s.PutPixelSecure;
	s.GetPixel=s.GetPixelSecure;
	s.AlphaPixel=s.AlphaPixelSecure;
	return ret;
}

COLOR CSurface::GetPixel(int x, int y)
/*!\brief Pixel auslesen
 *
 * \desc
 * Mit dieser Funktion wird der Farbwert eines einzelnen Pixels aus der Oberfläche ausgelesen.
 *
 * @param[in] x Die X-Koordinate des Pixels
 * @param[in] y Die Y-Koordinate des Pixels
 * @return Die Funktion gibt den Farbwert des Pixels an der gewünschten Position zurück.
 * Falls versucht wird Koordinaten ausserhalb der Oberfläche zu lesen, wird die Funktion 0
 * zurückgeben, was dem Farbwert schwarz entspricht. Die Anwendung muss selbst entscheiden, ob
 * es sich dabei um einen gültigen Farbwert oder einen Fehler handelt.
 *
 * Falls zuvor die Funktion CSurface::SetOrigin aufgerufen wurde, werden die Dort festgelegten Koordinaten
 * zum x- und y-Wert hinzuaddiert.
 *
 * \remarks
 * Es muss sichergestellt sein, dass die Oberfläche lesbar ist. Das heisst je nach Surface-Typ
 * und verwendeter Grafik-Engine muss zuvor CSurface::Lock aufgerufen worden sein.
 *
 *
 */

{
	return s.GetPixel(&s,x+s.originx,y+s.originy);
}

COLOR CSurface::BlendPixel(int x, int y, COLOR c, float brightness)
/*!\brief Pixel blenden anhand Helligkeitsparameter (float)
 *
 * \desc
 * Pixel blenden anhand Helligkeitsparameter (float)
 *
 * \param[in] x X-Koordinate
 * \param[in] y Y-Koordinate
 * \param[in] c Farbe
 * \param[in] brightness Fließkommawert zwischen 0 und 1, der angibt, wie intensiv der Farbwert dargestellt
 * werden soll. 0 Bedeutet vollständig transparent (nur Hintergrund ist zu sehen), 1 bedeutet
 * vollständig zu sehen (kein Hintergrund).
 *
 * \see CSurface::BlendPixel(int x, int y, COLOR c, int brightness)
 */
{
	if (s.BlendPixel) return (s.lastcolor=s.BlendPixel(&s,x+s.originx,y+s.originy,c,brightness));
	SetError(1012,"BlendPixel");
	return 0;
}

COLOR CSurface::BlendPixel(int x, int y, COLOR c, int brightness)
/*!\brief Pixel blenden anhand Helligkeitsparameter (int)
 *
 * \desc
 * Pixel blenden anhand Helligkeitsparameter (int)
 *
 * \param[in] x X-Koordinate
 * \param[in] y Y-Koordinate
 * \param[in] c Farbe
 * \param[in] brightness Integerwert zwischen 0 und 255, der angibt, wie intensiv der Farbwert dargestellt
 * werden soll. 0 Bedeutet vollständig transparent (nur Hintergrund ist zu sehen), 255 bedeutet
 * vollständig zu sehen (kein Hintergrund).
 *
 * \see CSurface::BlendPixel(int x, int y, COLOR c, float brightness)
 */
{
	if (s.BlendPixel255) return (s.lastcolor=s.BlendPixel255(&s,x+s.originx,y+s.originy,c,brightness));
	SetError(1012,"BlendPixel");
	return 0;
}



} // EOF namespace grafix
} // EOF namespace ppl6

