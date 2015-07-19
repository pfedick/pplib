/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.16 $
 * $Date: 2009/02/08 09:55:35 $
 * $Id: Blit.cpp,v 1.16 2009/02/08 09:55:35 patrick Exp $
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


static int Blt_32 (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y)
{
	if (!target->Lock(target)) return 0;
	if (!source->Lock(source)) {
		PushError();
		target->Unlock(target);
		PopError();
		return 0;
	}
#ifdef HAVE_X86_ASSEMBLER
	BLTDATA data;
	data.src=(ppluint32*)(source->base8+srect->top*source->pitch8+srect->left*source->bytes_per_pixel);;
	data.tgt=(ppluint32*)(target->base8+y*target->pitch8+x*target->bytes_per_pixel);
	data.width=srect->right-srect->left;
	data.height=srect->bottom-srect->top;
	data.pitchsrc=source->pitch8;
	data.pitchtgt=target->pitch8;
	if (ASM_Blt32(&data)) {
		source->Unlock(source);
		target->Unlock(target);
		return 1;
	}
#endif
	ppldb *q, *z;

	q=source->base8+srect->top*source->pitch8+srect->left*source->bitdepth/8;
	z=target->base8+y*target->pitch8+x*target->bitdepth/8;
	int mywidth=srect->right-srect->left;
	int yy;

	mywidth=mywidth*target->bitdepth/8;
	for (yy=srect->top;yy<srect->bottom;yy++) {
		memmove (z,q,mywidth);
		q+=source->pitch8;
		z+=target->pitch8;
	}
	source->Unlock(source);
	target->Unlock(target);
	return 1;
}

static int BltColorKey_32 (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y, COLOR c)
{
	if (!target->Lock(target)) return 0;
	if (!source->Lock(source)) {
		PushError();
		target->Unlock(target);
		PopError();
		return 0;
	}
#ifdef HAVE_X86_ASSEMBLER
	BLTDATA data;
	data.src=(ppluint32*)(source->base8+srect->top*source->pitch8+srect->left*source->bytes_per_pixel);;
	data.tgt=(ppluint32*)(target->base8+y*target->pitch8+x*target->bytes_per_pixel);
	data.width=srect->right-srect->left;
	data.height=srect->bottom-srect->top;
	data.pitchsrc=source->pitch8;
	data.pitchtgt=target->pitch8;
	data.color=c;
	if (ASM_BltColorKey32(&data)) {
		source->Unlock(source);
		target->Unlock(target);
		return 1;
	}
#endif
	ppluint32 *q, *z;
	COLOR qc;
	q=(ppluint32*)(source->base8+srect->top*source->pitch8+srect->left*source->bitdepth/8);
	z=(ppluint32*)(target->base8+y*target->pitch8+x*target->bitdepth/8);
	int yy, xx;
	int width=srect->right-srect->left;
	for (yy=srect->top;yy<srect->bottom;yy++) {
		for (xx=0;xx<width;xx++) {
			qc=q[xx];
			if (qc!=c) z[xx]=qc;
		}
		q+=source->pitch32;
		z+=target->pitch32;
	}
	source->Unlock(source);
	target->Unlock(target);
	return 1;
}

static int AlphaBlt_32 (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y)
{
	if (!target->Lock(target)) return 0;
	if (!source->Lock(source)) {
		PushError();
		target->Unlock(target);
		PopError();
		return 0;
	}
#ifdef HAVE_X86_ASSEMBLER
	BLTDATA data;
	data.src=(ppluint32*)(source->base8+srect->top*source->pitch8+srect->left*source->bytes_per_pixel);;
	data.tgt=(ppluint32*)(target->base8+y*target->pitch8+x*target->bytes_per_pixel);
	data.width=srect->right-srect->left;
	data.height=srect->bottom-srect->top;
	data.pitchsrc=source->pitch8;
	data.pitchtgt=target->pitch8;
	if (ASM_AlphaBlt32(&data)) {
		source->Unlock(source);
		target->Unlock(target);
		return 1;
	}
#endif
	ppldd r,g,b,alpha1,alpha2,ptgt,psrc;
	ppluint32 *src, *tgt;
	src=(ppluint32*)(source->base8+srect->top*source->pitch8+srect->left*source->bytes_per_pixel);
	tgt=(ppluint32*)(target->base8+y*target->pitch8+x*target->bytes_per_pixel);
	int width=srect->right-srect->left;
	int yy, xx;
	char *alphatab=target->alphatab;
	for (yy=srect->top;yy<=srect->bottom;yy++) {
		for (xx=0;xx<=width;xx++) {
			psrc=src[xx];
			if (psrc) {
				if ((psrc&0xff000000)==0xff000000) tgt[xx]=psrc;
				else {
					alpha1=psrc>>24;
					ptgt=tgt[xx];
					alpha2=(255-alpha1)<<8;
					alpha1=alpha1<<8;
					r=alphatab[alpha1+(psrc&255)]+alphatab[alpha2+(ptgt&255)];
					psrc=psrc>>8;
					ptgt=ptgt>>8;
					g=alphatab[alpha1+(psrc&255)]+alphatab[alpha2+(ptgt&255)];
					psrc=psrc>>8;
					ptgt=ptgt>>8;
					b=alphatab[alpha1+(psrc&255)]+alphatab[alpha2+(ptgt&255)];
					tgt[xx]=r+(g<<8)+(b<<16);
				}
			}
		}
		src+=source->pitch32;
		tgt+=target->pitch32;
	}
	source->Unlock(source);
	target->Unlock(target);
	return 1;
}

static int BltDiffuse_32 (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y, COLOR c)
{
	if (!target->Lock(target)) return 0;
	if (!source->Lock(source)) {
		PushError();
		target->Unlock(target);
		PopError();
		return 0;
	}

#ifdef HAVE_X86_ASSEMBLER
	BLTDATA data;
	data.src=(ppluint32*)(source->base8+srect->top*source->pitch8+srect->left*source->bytes_per_pixel);;
	data.tgt=(ppluint32*)(target->base8+y*target->pitch8+x*target->bytes_per_pixel);
	data.width=srect->right-srect->left;
	data.height=srect->bottom-srect->top;
	data.pitchsrc=source->pitch8;
	data.pitchtgt=target->pitch8;
	data.color=c;
	if (ASM_BltDiffuse32(&data)) {
		source->Unlock(source);
		target->Unlock(target);
		return 1;
	}
#endif


	ppluint32 *z;
	ppluint8 *q;
	COLOR qc;

	q=(ppluint8*)(source->base8+srect->top*source->pitch8+srect->left*source->bitdepth/8);
	z=(ppluint32*)(target->base8+y*target->pitch8+x*target->bitdepth/8);
	int yy, xx;
	int width=srect->right-srect->left;
	for (yy=srect->top;yy<srect->bottom;yy++) {
		for (xx=0;xx<width;xx++) {
			qc=q[xx];
			if (qc) {
				if (qc==0xff) z[xx]=c;
				else {
					z[xx]=target->RGBBlend255(z[xx],c,qc);
				}
			}
		}
		q+=source->pitch8;
		z+=target->pitch32;
	}
	source->Unlock(source);
	target->Unlock(target);
	return 1;
}


int CGrafix::InitBlits(SURFACE *s)
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
{
	switch (s->rgbformat) {
		case RGBFormat::A8R8G8B8:		// 32 Bit True Color
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8B8G8R8:
		case RGBFormat::X8R8G8B8:
			if (!s->Blt) s->Blt=Blt_32;
			s->BltSoftware=Blt_32;
			if (!s->AlphaBlt) s->AlphaBlt=AlphaBlt_32;
			s->AlphaBltSoftware=AlphaBlt_32;
			if (!s->BltColorKey) s->BltColorKey=BltColorKey_32;
			s->BltColorKeySoftware=BltColorKey_32;
			if (!s->BltDiffuse) s->BltDiffuse=BltDiffuse_32;
			s->BltDiffuseSoftware=BltDiffuse_32;

			return 1;
		case RGBFormat::GREY8:
		case RGBFormat::A8:
			return 1;

	}
	SetError(1013,"RGBFormat=%s (%i)",GetRGBFormatName(s->rgbformat),s->rgbformat);
	return 0;
}


int CSurface::CheckBltClipping (int *x, int *y, RECT * r)
/*!\brief Überprüft, ob eine Blit-Aktion in den Clipping-Bereich passt.
 *
 * \desc
 * Diese Funktion prüft, ob das zu zeichnende Rechteck überhaupt in den aktuellen
 * Clipping-Bereich der Oberfläche passt. Dabei wird das Quellrechteck bei Bedarf
 * angepasst.
 *
 * \param[in,out] x Pointer auf die X-Koordinate der Zielposition
 * \param[in,out] y Pointer auf die Y-Koordinate der Zielposition
 * \param[in,out] r Pointer auf das Quell-Rechteck
 *
 * \return
 * Die Funktion liefert 0 zurück, wenn das Rechteck komplett ausserhalb des aktuellen
 * Clipping-Bereichs liegt, oder 1, wenn es ganz oder zumindest teilweise innerhalb des
 * Clipping-Bereichs liegt. In letzterem Fall werden die Koordinaten \p x, \p y und die
 * Dimensionen des Rechtecks \p r so angepasst, dass durch die nachfolgende Blt-Funktion
 * nur der Sichtbare Bereich an die korrekte Position gezeichnet wird.
 */
{
	int width,height;
	width=r->right-r->left;
	height=r->bottom-r->top;

	// Falls x oder y komplett ausserhalb des Clipping-Bereichs liegt,
	// braucht nichts ausgegeben werden
	if (*x+width<s.clipper.left || *x>=s.clipper.right) return 0;
	if (*y+height<s.clipper.top || *y>=s.clipper.bottom) return 0;


	// Koordinaten bei Bedarf korrigieren
	if (*x<s.clipper.left || *x+r->right >= s.clipper.right) {
		if (*x<s.clipper.left) {
			r->left=r->left-*x+s.clipper.left;
			*x=s.clipper.left;
		}
		if (*x+width>=s.clipper.right) {
			r->right=r->left-*x+s.clipper.right;
		}
	}
	if (*y<s.clipper.top || *y+r->bottom >= s.clipper.bottom) {
		if (*y<s.clipper.top) {
			r->top=r->top-*y+s.clipper.top;
			*y=s.clipper.top;
		}
		if (*y+height>=s.clipper.bottom) {
			r->bottom=r->top-*y+s.clipper.bottom;
		}
	}
	return 1;
}

int CSurface::Blt(CSurface *source, RECT *srect, int x, int y)
/*!\brief Rechteck 1:1 kopieren
 *
 */
{
	if (!source) {
		SetError(1015,"Blt");
		return 0;
	}
	// Quellrechteck
	RECT q;
	if (srect) {
		q.left=srect->left;
		q.top=srect->top;
		q.right=srect->right;
		q.bottom=srect->bottom;
		if (q.left<0) q.left=0;
		if (q.right>source->s.width) q.right=source->s.width;
		if (q.top<0) q.top=0;
		if (q.bottom>source->s.height) q.bottom=source->s.height;
	} else {
		q.left=0;
		q.top=0;
		q.right=source->s.width;
		q.bottom=source->s.height;
	}

	// Clipping pruefen
	int xx=x+s.originx, yy=y+s.originy;
	if (!CheckBltClipping(&xx, &yy, &q)) {
		SetError(1016);
		return 0;
	}
	if (q.right-q.left==0 || q.bottom-q.top==0) {
		SetError(1016);
		return 0;
	}
	// Wenn Quell- und Ziel-Surface die gleiche Engine benutzen, verwenden wir Blt
	if (engine==source->engine) {
		if (s.Blt) return s.Blt(&s,&source->s,&q,xx,yy);
	}
	if (s.BltSoftware) return s.BltSoftware(&s,&source->s,&q,xx,yy);
	SetError(1012,"Blt");
	return 0;
}

int CSurface::BltColorKey(CSurface *source, COLOR c, RECT *srect, int x, int y)
/*!\brief Rechteck unter Berücksichtigung einer transparenten Farbe kopieren
 *
 */
{
	if (!source) {
		SetError(1015,"Blt");
		return 0;
	}
	// Quellrechteck
	RECT q;
	if (srect) {
		q.left=srect->left;
		q.top=srect->top;
		q.right=srect->right;
		q.bottom=srect->bottom;
		if (q.left<0) q.left=0;
		if (q.right>source->s.width) q.right=source->s.width;
		if (q.top<0) q.top=0;
		if (q.bottom>source->s.height) q.bottom=source->s.height;
	} else {
		q.left=0;
		q.top=0;
		q.right=source->s.width;
		q.bottom=source->s.height;
	}

	// Clipping pruefen
	int xx=x+s.originx, yy=y+s.originy;
	if (!CheckBltClipping(&xx, &yy, &q)) {
		SetError(1016);
		return 0;
	}
	if (q.right-q.left==0 || q.bottom-q.top==0) {
		SetError(1016);
		return 0;
	}
	// Wenn Quell- und Ziel-Surface die gleiche Engine benutzen, verwenden wir Blt
	if (engine==source->engine) {
		if (s.BltColorKey) return s.BltColorKey(&s,&source->s,&q,xx,yy,c);
	}
	if (s.BltColorKeySoftware) return s.BltColorKeySoftware(&s,&source->s,&q,xx,yy,c);
	SetError(1012,"Blt");
	return 0;
}

int CSurface::AlphaBlt(CSurface *source, RECT *srect, int x, int y)
/*!\brief Rechteck unter Berücksichtigung des Alpha-Kanals kopieren
 *
 */
{
	if (!source) {
		SetError(1015,"Blt");
		return 0;
	}
	// Quellrechteck
	RECT q;
	if (srect) {
		q.left=srect->left;
		q.top=srect->top;
		q.right=srect->right;
		q.bottom=srect->bottom;
		if (q.left<0) q.left=0;
		if (q.right>source->s.width) q.right=source->s.width;
		if (q.top<0) q.top=0;
		if (q.bottom>source->s.height) q.bottom=source->s.height;
	} else {
		q.left=0;
		q.top=0;
		q.right=source->s.width;
		q.bottom=source->s.height;
	}

	// Clipping pruefen
	int xx=x+s.originx, yy=y+s.originy;
	if (!CheckBltClipping(&xx, &yy, &q)) {
		SetError(1016);
		return 0;
	}
	if (q.right-q.left==0 || q.bottom-q.top==0) {
		SetError(1016);
		return 0;
	}
	// Wenn Quell- und Ziel-Surface die gleiche Engine benutzen, verwenden wir Blt
	if (engine==source->engine) {
		if (s.AlphaBlt) return s.AlphaBlt(&s,&source->s,&q,xx,yy);
	}
	if (s.AlphaBltSoftware) return s.AlphaBltSoftware(&s,&source->s,&q,xx,yy);
	SetError(1012,"AlphaBlt");
	return 0;
}

int CSurface::BltDiffuse(CSurface *source, COLOR c, RECT *srect, int x, int y)
/*!\brief Rechteck anhand der Intensität der Quellfarbe kopieren
 *
 */
{
	if (!source) {
		SetError(1015,"Blt");
		return 0;
	}
	// Source muss bestimmtes Farbformat haben
	if (source->s.rgbformat!=RGBFormat::GREY8 && source->s.rgbformat!=RGBFormat::A8) {
		SetError(1019,"BltDiffuse benötigt GREY8 oder A8");
		return 0;
	}
	// Quellrechteck
	RECT q;
	if (srect) {
		q.left=srect->left;
		q.top=srect->top;
		q.right=srect->right;
		q.bottom=srect->bottom;
		if (q.left<0) q.left=0;
		if (q.right>source->s.width) q.right=source->s.width;
		if (q.top<0) q.top=0;
		if (q.bottom>source->s.height) q.bottom=source->s.height;
	} else {
		q.left=0;
		q.top=0;
		q.right=source->s.width;
		q.bottom=source->s.height;
	}

	// Clipping pruefen
	int xx=x+s.originx, yy=y+s.originy;
	if (!CheckBltClipping(&xx, &yy, &q)) {
		SetError(1016);
		return 0;
	}
	if (q.right-q.left==0 || q.bottom-q.top==0) {
		SetError(1016);
		return 0;
	}
	// Wenn Quell- und Ziel-Surface die gleiche Engine benutzen, verwenden wir BltDiffuse
	if (engine==source->engine) {
		if (s.BltDiffuse) return s.BltDiffuse(&s,&source->s,&q,xx,yy,c);
	}
	if (s.BltDiffuseSoftware) return s.BltDiffuseSoftware(&s,&source->s,&q,xx,yy,c);
	SetError(1012,"BltDiffuse");
	return 0;
}


int CSurface::BeginSprites()
/*!\brief Zeichnen von Sprites initialisieren (nur DirectX)
 *
 * @return
 */
{
	if (s.BeginSprites) return s.BeginSprites(&s);
	return 1;
}

int CSurface::EndSprites()
/*!\brief Zeichnen von Sprites beenden (nur DirectX)
 *
 * @return
 */
{
	if (s.EndSprites) return s.EndSprites(&s);
	return 1;
}

int CSurface::DrawSprite(CSurface *source, RECT *srect, int x, int y, COLOR c)
/*!\brief Sprite-Engine zum Kopieren eines Rechtecks mit Alpha-Kanal verwenden
 *
 * \desc
 * Diese Funktion verwendet die Sprite-Engine der verwendeten Grafik-Engine, um ein Rechteck
 * zu kopieren. Dies wird zur Zeit von der DirectX9-Engine unterstützt und hat den Vorteil, dass
 * der komplette Kopiervorgang durch die Grafik-Hardware durchgeführt wird und somit sehr schnell ist.
 *
 * Vor Nutzung dieser Funktion muss zunächst CSurface::BeginSprites aufegrufen werden und danach,
 * wenn alle Sprites gezeichnet wurden, CSurface::EndSprites. Dazwischen sollten keine weiteren
 * Grafik-Funktionen aufgerufen werden. Insbesondere sollte die Oberfläche nicht gelockt werden
 * (CSurface::Lock).
 *
 * @param[in] source Pointer auf die Quell-Oberfläche
 * @param[in] srect Pointer auf eine RECT-Struktur, die den zu kopierenden Ausschnitt aus der
 * Quell-Oberfläche definiert. Ist dieser Parameter NULL, wird die komplette Quell-Oberfläche
 * kopiert.
 * @param[in] x Ziel-Koordinate der linken oberen Ecke
 * @param[in] y Ziel-Koordinate der linken oberen Ecke
 * @param[in] c Modulator für Farbe und Alpha-Kanal der Quellgrafik. Ein Wert von 0xffffffff (=Default)
 * erhält alle Parameter.
 * @return Die Funktion gibt 1 zurück, wenn das Sprite gezeichnet wurde, sonst 0.
 *
 * \remarks
 * Sollte die Grafik-Engine diese Funktion nicht unterstützen, wird automatisch auf CSurface::AlphaBlt
 * zurückgegriffen.
 * \attention
 * Anders als bei allen anderen Blt-Funktionen wird hier der Clipping-Bereich nicht geprüft!
 */
{
	// Wenn Quell- und Ziel-Surface die gleiche Engine benutzen, verwenden wir DrawSprite
	if (engine==source->engine && s.DrawSprite!=NULL) return s.DrawSprite(&s,x,y,&source->s,srect,c);
	return AlphaBlt(source,srect,x,y);
}


} // EOF namespace grafix
} // EOF namespace ppl6
