/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
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

#include "prolog_ppl7.h"
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include "ppl7.h"
#include "ppl7-grafix.h"



#ifdef HAVE_X86_ASSEMBLER
typedef struct {
	void* src;
	void* tgt;
	uint32_t	width;
	uint32_t	height;
	uint32_t	pitchsrc;
	uint32_t	pitchtgt;
	uint32_t	color;
} BLTDATA;

typedef struct {
	char* sadr;
	char* bgadr;
	char* tgadr;
	uint32_t spitch;
	uint32_t bgpitch;
	uint32_t tgpitch;
	int width;
	int height;
	int cb_key;
	int cr_key;
	int tola;
	int tolb;
} BLTCHROMADATA;

extern "C" {
	int ASM_AlphaBlt32(BLTDATA* d);
	int ASM_Blt32(BLTDATA* d);
	int ASM_BltColorKey32(BLTDATA* d);
	int ASM_BltDiffuse32(BLTDATA* d);
	int ASM_BltBlend32_MMX(BLTDATA* d, int factor);
	int ASM_BltBlend32_SSE_Align1(BLTDATA* d, int factor);
	int ASM_BltBlend32_SSE_Align2(BLTDATA* d, int factor);
	int ASM_BltChromaKey32(BLTCHROMADATA* d);
}
#endif

namespace ppl7 {
namespace grafix {

extern char* alphatab;


static void* adr(const DRAWABLE_DATA& data, int x, int y)
{
	if (x < data.width && y < data.height) return data.base8 + (y * data.pitch) + (x * data.rgbformat.bitdepth() / 8);
	return NULL;
}


static int Blt_32(DRAWABLE_DATA& target, const DRAWABLE_DATA& source, const Rect& srect, int x, int y)
{
#ifdef HAVE_X86_ASSEMBLER
	BLTDATA data;
	data.src=(uint32_t*)adr(source, srect.left(), srect.top());
	data.tgt=(uint32_t*)adr(target, x, y);
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
	uint8_t* q, * z;
	q=(uint8_t*)adr(source, srect.left(), srect.top());
	z=(uint8_t*)adr(target, x, y);
	int mywidth=srect.width() * target.rgbformat.bytesPerPixel();
	int yy;
	for (yy=0;yy < srect.height();yy++) {
		memmove(z, q, mywidth);
		q+=source.pitch;
		z+=target.pitch;
	}
	return 1;
}

#ifdef __LITTLE_ENDIAN__
union Pixel32_t {
	struct { uint8_t red, green, blue, alpha; };
	uint32_t c;
};
#else
union Pixel32_t {
	struct { uint8_t alpha, red, green, blue; };
	uint32_t c;
};
#endif

static inline Pixel32_t GetColorBltAlphablend_32(Pixel32_t ground, Pixel32_t top)
{
	Pixel32_t result;
	result.alpha=255 - ((255 - ground.alpha) * (255 - top.alpha) / 255);
	uint8_t areverse=255 - top.alpha;
	// red   = (colorRGBA1[0] * (255 - colorRGBA2[3]) + colorRGBA2[0] * colorRGBA2[3]) / 255
	result.red=(ground.red * areverse + top.red * top.alpha) / 255;
	result.green=(ground.green * areverse + top.green * top.alpha) / 255;
	result.blue=(ground.blue * areverse + top.blue * top.alpha) / 255;
	return result;

}


static int BltAlpha_32(DRAWABLE_DATA& target, const DRAWABLE_DATA& source, const Rect& srect, int x, int y)
{
#ifdef HAVE_X86_ASSEMBLER
	BLTDATA data;
	data.src=(uint32_t*)adr(source, srect.left(), srect.top());
	data.tgt=(uint32_t*)adr(target, x, y);
	data.width=srect.width();
	data.height=srect.height();
	data.pitchsrc=source.pitch;
	data.pitchtgt=target.pitch;
	if (ASM_AlphaBlt32(&data)) {
		return 1;
	}
	return 0;
#endif
	Pixel32_t* src, * tgt;
	src=(Pixel32_t*)adr(source, srect.left(), srect.top());
	tgt=(Pixel32_t*)adr(target, x, y);
	int width=srect.width();
	int yy, xx;
	for (yy=0;yy < srect.height();yy++) {
		for (xx=0;xx < width;xx++) {
			if (src[xx].c) {
				if (src[xx].alpha == 0xff) tgt[xx]=src[xx];
				else {
					tgt[xx]=GetColorBltAlphablend_32(tgt[xx], src[xx]);
				}
			}
		}
		src+=(source.pitch >> 2);
		tgt+=(target.pitch >> 2);
	}
	return 1;
}

static inline Pixel32_t GetColorBltAlphablendMod_32(Pixel32_t ground, Pixel32_t top, Pixel32_t mod)
{
	Pixel32_t result;
	result.alpha=255 - ((255 - ground.alpha) * (255 - top.alpha) / 255);
	uint8_t areverse=255 - top.alpha;
	// red   = (colorRGBA1[0] * (255 - colorRGBA2[3]) + colorRGBA2[0] * colorRGBA2[3]) / 255
	result.red=(ground.red * areverse + top.red * top.alpha * mod.red / 255) / 255;
	result.green=(ground.green * areverse + top.green * top.alpha * mod.green / 255) / 255;
	result.blue=(ground.blue * areverse + top.blue * top.alpha * mod.blue / 255) / 255;
	return result;

}

static inline Pixel32_t GetColorModulated(Pixel32_t top, Pixel32_t mod)
{
	Pixel32_t result;
	result.alpha=top.alpha;
	result.red=top.red * mod.red / 255;
	result.green=top.green * mod.green / 255;
	result.blue=top.blue * mod.blue / 255;
	return result;
}

static int BltAlphaMod_32(DRAWABLE_DATA& target, const DRAWABLE_DATA& source, const Rect& srect, SurfaceColor mod, int x, int y)
{
#ifdef HAVE_X86_ASSEMBLER
/*
	BLTDATA data;
	data.src=(uint32_t*)adr(source, srect.left(), srect.top());
	data.tgt=(uint32_t*)adr(target, x, y);
	data.width=srect.width();
	data.height=srect.height();
	data.pitchsrc=source.pitch;
	data.pitchtgt=target.pitch;
	data.color=mod;
	if (ASM_AlphaBltMod32(&data)) {
		return 1;
	}
	return 0;
	*/
#endif
	Pixel32_t* src, * tgt;
	src=(Pixel32_t*)adr(source, srect.left(), srect.top());
	tgt=(Pixel32_t*)adr(target, x, y);
	int width=srect.width();
	int yy, xx;
	Pixel32_t modulation;
	modulation.c=mod;
	for (yy=0;yy < srect.height();yy++) {
		for (xx=0;xx < width;xx++) {
			if (src[xx].c) {
				Pixel32_t pixel=GetColorModulated(src[xx], modulation);
				if (src[xx].alpha == 0xff) tgt[xx]=pixel;
				else {
					tgt[xx]=GetColorBltAlphablendMod_32(tgt[xx], src[xx], modulation);
				}
			}
		}
		src+=(source.pitch >> 2);
		tgt+=(target.pitch >> 2);
	}
	return 1;
}

static int BltBlend_32(DRAWABLE_DATA& target, const DRAWABLE_DATA& source, const Rect& srect, int x, int y, float factor)
{
#ifdef HAVE_X86_ASSEMBLER
	BLTDATA data;
	data.src=(uint32_t*)adr(source, srect.left(), srect.top());
	data.tgt=(uint32_t*)adr(target, x, y);
	data.width=srect.width();
	data.height=srect.height();
	data.pitchsrc=source.pitch;
	data.pitchtgt=target.pitch;
	int f=(int)(factor * 255.0);
	if (f < 0) f=0;
	if (f > 255) f=255;
	//::printf ("factor=%0.2f, f=%i\n",factor,f);

	if ((GetCPUCaps() & CPUCAPS::CPU_HAVE_SSE2)) {
		if ((data.width & 1) == 0 && (((ptrdiff_t)data.src) & 7) == 0 && (((ptrdiff_t)data.tgt) & 7) == 0) {
			if (ASM_BltBlend32_SSE_Align2(&data, f)) return 1;
		} else {
			if (ASM_BltBlend32_SSE_Align1(&data, f)) return 1;
		}
	} else {
		if (ASM_BltBlend32_MMX(&data, f)) return 1;
	}
#endif
	uint32_t* src, * tgt;
	src=(uint32_t*)adr(source, srect.left(), srect.top());
	tgt=(uint32_t*)adr(target, x, y);
	int width=srect.width();
	int yy, xx;
	if (!alphatab) return 0;
	Color psrc, ptgt, c;
	for (yy=0;yy < srect.height();yy++) {
		for (xx=0;xx < width;xx++) {
			ptgt.setColor(tgt[xx]);
			psrc.setColor(src[xx]);
			c.blendf(ptgt, psrc, factor);
			tgt[xx]=c.color();
		}
		src+=(source.pitch >> 2);
		tgt+=(target.pitch >> 2);
	}
	return 1;
}





static int BltColorKey_32(DRAWABLE_DATA& target, const DRAWABLE_DATA& source, const Rect& srect, int x, int y, SurfaceColor c)
{
#ifdef HAVE_X86_ASSEMBLER
	BLTDATA data;
	data.src=(uint32_t*)adr(source, srect.left(), srect.top());
	data.tgt=(uint32_t*)adr(target, x, y);
	data.width=srect.width();
	data.height=srect.height();
	data.pitchsrc=source.pitch;
	data.pitchtgt=target.pitch;
	data.color=c;
	if (ASM_BltColorKey32(&data)) {
		return 1;
	}
#endif
	uint32_t* q, * z;
	SurfaceColor qc;
	q=(uint32_t*)adr(source, srect.left(), srect.top());
	z=(uint32_t*)adr(target, x, y);
	int pitch_tgt=target.pitch >> 2;
	int pitch_src=source.pitch >> 2;
	int yy, xx;
	int width=srect.width();
	for (yy=0;yy < srect.height();yy++) {
		for (xx=0;xx < width;xx++) {
			qc=q[xx];
			if (qc != c) z[xx]=qc;
		}
		q+=pitch_src;
		z+=pitch_tgt;
	}
	return 1;
}

static int BltDiffuse_32(DRAWABLE_DATA& target, const DRAWABLE_DATA& source, const Rect& srect, int x, int y, SurfaceColor c)
{
#ifdef HAVE_X86_ASSEMBLER
	BLTDATA data;
	data.src=(uint32_t*)adr(source, srect.left(), srect.top());
	data.tgt=(uint32_t*)adr(target, x, y);
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
	uint32_t* z;
	uint8_t* q;
	SurfaceColor qc;
	q=(uint8_t*)adr(source, srect.left(), srect.top());
	z=(uint32_t*)adr(target, x, y);
	int pitch32=target.pitch >> 2;
	int yy, xx;
	int width=srect.width();
	for (yy=0;yy < srect.height();yy++) {
		for (xx=0;xx < width;xx++) {
			qc=q[xx];
			if (qc) {
				if (qc == 0xff) z[xx]=c;
				else {
					z[xx]=target.fn->RGBBlend255(z[xx], c, qc);
				}
			}
		}
		q+=source.pitch;
		z+=pitch32;
	}
	return 1;
}


typedef struct {
	union {
		struct { uint8_t b, g, r, a; };
		uint32_t c;
	};
} PIXEL;

#ifndef max
static inline int max(int a, int b)
{
	if (a > b) { return (a); }
	return (b);
}
#endif

static inline double colorclose(int Cb_p, int Cr_p, int Cb_key, int Cr_key, int tola, int tolb)
{
   /*decides if a color is close to the specified hue*/
	double temp = sqrt((Cb_key - Cb_p) * (Cb_key - Cb_p) + (Cr_key - Cr_p) * (Cr_key - Cr_p));
	// SSE: sqrtss für float, SSE2: sqrtsd für double
	// Man könnte Cb und Cr in ein MME-Register packen, die Subtraktionen und Multiplikationen
	// parallel berechnen, das Ergebnis addieren und dann die Wurzel ziehen
	if (temp < tola) { return (0.0); }
	if (temp < tolb) { return ((temp - tola) / (tolb - tola)); }
	return (1.0);
}


static inline int getYCb(int r, int g, int b)
{
	return (int)(128 + -0.168736 * r - 0.331264 * g + 0.5 * b);
}

static inline int getYCr(int r, int g, int b)
{
	return (int)(128 + 0.5 * r - 0.418688 * g - 0.081312 * b);
}

static void BltChromaKey_32(DRAWABLE_DATA& target, const DRAWABLE_DATA& source, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y)
{
	if (tol2 < tol1) tol2=tol1;
#ifdef HAVE_X86_ASSEMBLER
	if ((GetCPUCaps() & CPUCAPS::CPU_HAVE_SSE2)) {
		BLTCHROMADATA data;
		data.sadr=(char*)adr(source, srect.left(), srect.top());
		data.bgadr=(char*)adr(target, x, y);
		data.tgadr=data.bgadr;
		data.width=srect.width();
		data.height=srect.height();
		data.spitch=source.pitch;
		data.bgpitch=target.pitch;
		data.tgpitch=data.bgpitch;
		data.cb_key=key.getYCb();
		data.cr_key=key.getYCr();
		data.tola=tol1;
		data.tolb=tol2;
		//if ((((int)(data.width&255))&3)==0 && (((int)(data.sadr&255))&15)==0 && (((int)(data.bgadr&255))&15)==0) {
		if (ASM_BltChromaKey32(&data)) return;
	}
#endif
	double mask;
	int cb, cr;
	int cb_key=key.getYCb();
	int cr_key=key.getYCr();

	PIXEL c, bg, t;

	uint32_t* sadr=(uint32_t*)adr(source, srect.left(), srect.top());
	uint32_t spitch=source.pitch / 4;

	uint32_t* bgadr=(uint32_t*)adr(target, x, y);
	uint32_t bgpitch=target.pitch / 4;

	uint32_t* tgadr=(uint32_t*)adr(target, x, y);
	uint32_t tgpitch=target.pitch / 4;

	for (int y=0;y < srect.height();y++) {
		for (int x=0;x < srect.width();x++) {
			c.c=sadr[x];
			cb=getYCb(c.r, c.g, c.b);
			cr=getYCr(c.r, c.g, c.b);
			bg.c=bgadr[x];

			mask = 1 - colorclose(cb, cr, cb_key, cr_key, tol1, tol2);
			if (mask == 0.0) {
				tgadr[x]=c.c;
				continue;
			} else if (mask == 1.0) {
				tgadr[x]=bg.c;
			} else {
				t.r=(uint8_t)(max(c.r - mask * c.r, 0) + mask * bg.r);
				t.g=(uint8_t)(max(c.g - mask * c.g, 0) + mask * bg.g);
				t.b=(uint8_t)(max(c.b - mask * c.b, 0) + mask * bg.b);
				t.a=(uint8_t)(max(c.a - mask * c.a, 0) + mask * bg.a);
				tgadr[x]=t.c;
			}
		}
		sadr+=spitch;
		bgadr+=bgpitch;
		tgadr+=tgpitch;
	}
}


static void BltBackgroundOnChromaKey_32(DRAWABLE_DATA& target, const DRAWABLE_DATA& background, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y)
{
	if (tol2 < tol1) tol2=tol1;
#ifdef HAVE_X86_ASSEMBLER
	if ((GetCPUCaps() & CPUCAPS::CPU_HAVE_SSE2)) {
		BLTCHROMADATA data;
		data.sadr=(char*)adr(target, srect.left(), srect.top());
		data.bgadr=(char*)adr(background, x, y);
		data.tgadr=data.sadr;
		data.width=srect.width();
		data.height=srect.height();
		data.spitch=target.pitch;
		data.bgpitch=background.pitch;
		data.tgpitch=data.spitch;
		data.cb_key=key.getYCb();
		data.cr_key=key.getYCr();
		data.tola=tol1;
		data.tolb=tol2;
		if (ASM_BltChromaKey32(&data)) return;
	}
#endif
	double mask;
	int cb, cr;
	int cb_key=key.getYCb();
	int cr_key=key.getYCr();

	PIXEL c, bg, t;

	uint32_t* sadr=(uint32_t*)adr(target, srect.left(), srect.top());
	uint32_t spitch=target.pitch / 4;

	uint32_t* bgadr=(uint32_t*)adr(background, x, y);
	uint32_t bgpitch=background.pitch / 4;

	uint32_t* tgadr=(uint32_t*)adr(target, x, y);
	uint32_t tgpitch=target.pitch / 4;

	for (int y=0;y < srect.height();y++) {
		for (int x=0;x < srect.width();x++) {
			c.c=sadr[x];
			cb=getYCb(c.r, c.g, c.b);
			cr=getYCr(c.r, c.g, c.b);
			bg.c=bgadr[x];

			mask = 1 - colorclose(cb, cr, cb_key, cr_key, tol1, tol2);
			if (mask == 0.0) {
				tgadr[x]=c.c;
				continue;
			} else if (mask == 1.0) {
				tgadr[x]=bg.c;
			} else {
				t.r=(uint8_t)(max(c.r - mask * c.r, 0) + mask * bg.r);
				t.g= (uint8_t)(max(c.g - mask * c.g, 0) + mask * bg.g);
				t.b= (uint8_t)(max(c.b - mask * c.b, 0) + mask * bg.b);
				t.a= (uint8_t)(max(c.a - mask * c.a, 0) + mask * bg.a);
				tgadr[x]=t.c;
			}
		}
		sadr+=spitch;
		bgadr+=bgpitch;
		tgadr+=tgpitch;
	}
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
void Grafix::initBlits(const RGBFormat& format, GRAFIX_FUNCTIONS* fn)
{
	switch (format) {
		case RGBFormat::A8R8G8B8:		// 32 Bit True Color
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8B8G8R8:
		case RGBFormat::X8R8G8B8:
			fn->Blt=Blt_32;
			fn->BltAlpha=BltAlpha_32;
			fn->BltAlphaMod=BltAlphaMod_32;
			fn->BltColorKey=BltColorKey_32;
			fn->BltDiffuse=BltDiffuse_32;
			fn->BltBlend=BltBlend_32;
			fn->BltChromaKey=BltChromaKey_32;
			fn->BltBackgoundOnChromaKey=BltBackgroundOnChromaKey_32;
			return;
		case RGBFormat::GREY8:
		case RGBFormat::A8:
			return;

	}
	throw UnsupportedColorFormatException("RGBFormat=%s (%i)", (const char*)format.name(), format.format());
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
int Drawable::fitRect(int& x, int& y, Rect& r)
{
	Rect screen(0, 0, data.width, data.height);
	Rect object(x, y, r.width(), r.height());
	Rect i=screen.intersected(object);		// TODO: Das ist falsch

	//::printf ("Drawable::fitRect screen (%i/%i)-(%i/%i)\n", screen.x1, screen.y1, screen.x2, screen.y2);
	//::printf ("Drawable::fitRect object (%i/%i)-(%i/%i)\n", object.x1, object.y1, object.x2, object.y2);
	//::printf ("Drawable::fitRect i (%i/%i)-(%i/%i)\n", i.x1, i.y1, i.x2, i.y2);

	if (i.isNull()) return 0;
	int shiftx=i.x1 - object.x1;
	int shifty=i.y1 - object.y1;
	x+=shiftx;
	y+=shifty;

	r.x1+=shiftx;
	r.y1+=shifty;
	r.x2=r.x1 + i.width();
	r.y2=r.y1 + i.height();
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
	if (source.isEmpty()) throw EmptyDrawableException();
	// Quellrechteck
	Rect q;
	if (srect.isNull()) {
		q=source.rect();
	} else {
		q=srect;
		if (q.left() < 0) q.setLeft(0);
		if (q.width() > source.width()) q.setWidth(source.width());
		if (q.top() < 0) q.setTop(0);
		if (q.height() > source.height()) q.setHeight(source.height());
	}
	//::printf ("rect=(%i/%i)-(%i/%i)\n", q.x1, q.y1, q.x2, q.y2);
	if (!fitRect(x, y, q)) return;
	//::printf ("rect=(%i/%i)-(%i/%i)\n", q.x1, q.y1, q.x2, q.y2);
	if (!fn->Blt) throw FunctionUnavailableException("Drawable::blt");
	fn->Blt(data, source.data, q, x, y);
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
	if (source.isEmpty()) throw EmptyDrawableException();
	// Quellrechteck
	Rect q;
	if (srect.isNull()) {
		q=source.rect();
	} else {
		q=srect;
		if (q.left() < 0) q.setLeft(0);
		if (q.width() > source.width()) q.setWidth(source.width());
		if (q.top() < 0) q.setTop(0);
		if (q.height() > source.height()) q.setHeight(source.height());
	}
	if (!fitRect(x, y, q)) return;
	if (!fn->BltDiffuse) throw FunctionUnavailableException("Drawable::bltDiffuse");
	fn->BltDiffuse(data, source.data, q, x, y, rgb(c));
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
	if (source.isEmpty()) throw EmptyDrawableException();
	// Quellrechteck
	Rect q;
	if (srect.isNull()) {
		q=source.rect();
	} else {
		q=srect;
		if (q.left() < 0) q.setLeft(0);
		if (q.width() > source.width()) q.setWidth(source.width());
		if (q.top() < 0) q.setTop(0);
		if (q.height() > source.height()) q.setHeight(source.height());
	}
	if (!fitRect(x, y, q)) return;
	if (!fn->BltColorKey) throw FunctionUnavailableException("Drawable::bltColorKey");
	fn->BltColorKey(data, source.data, q, x, y, rgb(c));
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
	if (source.isEmpty()) throw EmptyDrawableException();
	// Quellrechteck
	Rect q;
	if (srect.isNull()) {
		q=source.rect();
	} else {
		q=srect;
		if (q.left() < 0) q.setLeft(0);
		if (q.width() > source.width()) q.setWidth(source.width());
		if (q.top() < 0) q.setTop(0);
		if (q.height() > source.height()) q.setHeight(source.height());
	}
	if (!fitRect(x, y, q)) return;
	if (!fn->BltAlpha) throw FunctionUnavailableException("Drawable::bltAlpha");
	fn->BltAlpha(data, source.data, q, x, y);
}

void Drawable::bltAlphaMod(const Drawable& source, const Color& mod, int x, int y)
{
	bltAlphaMod(source, source.rect(), x, y);
}

void Drawable::bltAlphaMod(const Drawable& source, const Rect& srect, const Color& mod, int x, int y)
{
	if (source.isEmpty()) throw EmptyDrawableException();
	// Quellrechteck
	Rect q;
	if (srect.isNull()) {
		q=source.rect();
	} else {
		q=srect;
		if (q.left() < 0) q.setLeft(0);
		if (q.width() > source.width()) q.setWidth(source.width());
		if (q.top() < 0) q.setTop(0);
		if (q.height() > source.height()) q.setHeight(source.height());
	}
	if (!fitRect(x, y, q)) return;
	if (!fn->BltAlphaMod) throw FunctionUnavailableException("Drawable::bltAlpha");
	fn->BltAlphaMod(data, source.data, q, rgb(mod), x, y);
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
	Rect r=iml.getRect(nr);
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
void Drawable::draw(const ImageList& iml, int nr, int x, int y, const Color& diffuse)
{
	Rect r=iml.getRect(nr);
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
	throw UnknownBltMethodException();
}

void Drawable::bltBlend(const Drawable& source, float factor, int x, int y)
{
	bltBlend(source, factor, source.rect(), x, y);
}

void Drawable::bltBlend(const Drawable& source, float factor, const Rect& srect, int x, int y)
{
	if (source.isEmpty()) throw EmptyDrawableException();
	if (factor <= 0.0f) return;
	if (factor >= 1.0f) {
		blt(source, srect, x, y);
		return;
	}
	// Quellrechteck
	Rect q;
	if (srect.isNull()) {
		q=source.rect();
	} else {
		q=srect;
		if (q.left() < 0) q.setLeft(0);
		if (q.width() > source.width()) q.setWidth(source.width());
		if (q.top() < 0) q.setTop(0);
		if (q.height() > source.height()) q.setHeight(source.height());
	}
	//::printf ("rect=(%i/%i)-(%i/%i)\n", q.x1, q.y1, q.x2, q.y2);
	if (!fitRect(x, y, q)) return;
	if (!fn->BltBlend) throw FunctionUnavailableException("Drawable::Blend");
	fn->BltBlend(data, source.data, q, x, y, factor);
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
void Drawable::bltChromaKey(const Drawable& source, const Color& key, int tol1, int tol2, int x, int y)
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
void Drawable::bltChromaKey(const Drawable& source, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y)
{
	if (source.isEmpty()) throw EmptyDrawableException();
	if (tol1 < 0 || tol1>255) throw IllegalArgumentException("0<=tol1<=255");
	if (tol2 < 0 || tol2>255) throw IllegalArgumentException("0<=tol2<=255");
	// Quellrechteck
	Rect q;
	if (srect.isNull()) {
		q=source.rect();
	} else {
		q=srect;
		if (q.left() < 0) q.setLeft(0);
		if (q.width() > source.width()) q.setWidth(source.width());
		if (q.top() < 0) q.setTop(0);
		if (q.height() > source.height()) q.setHeight(source.height());
	}
	if (!fitRect(x, y, q)) return;
	if (!fn->BltChromaKey) throw FunctionUnavailableException("Drawable::bltChromaKey");
	fn->BltChromaKey(data, source.data, q, key, tol1, tol2, x, y);
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
void Drawable::bltBackgroundOnChromaKey(const Drawable& background, const Color& key, int tol1, int tol2, int x, int y)
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
void Drawable::bltBackgroundOnChromaKey(const Drawable& background, const Rect& srect, const Color& key, int tol1, int tol2, int x, int y)
{
	if (background.isEmpty()) throw EmptyDrawableException();
	if (tol1 < 0 || tol1>255) throw IllegalArgumentException("0<=tol1<=255");
	if (tol2 < 0 || tol2>255) throw IllegalArgumentException("0<=tol2<=255");
	// Quellrechteck
	Rect q;
	if (srect.isNull()) {
		q=rect();
	} else {
		q=srect;
		if (q.left() < 0) q.setLeft(0);
		if (q.width() > width()) q.setWidth(width());
		if (q.top() < 0) q.setTop(0);
		if (q.height() > height()) q.setHeight(height());
	}
	if (!fitRect(x, y, q)) return;
	if (!fn->BltBackgoundOnChromaKey) throw FunctionUnavailableException("Drawable::bltBackgroundOnChromaKey");
	fn->BltBackgoundOnChromaKey(data, background.data, q, key, tol1, tol2, x, y);
}



#ifdef DONE
void Drawable::draw(const Sprite& sprite, int nr, int x, int y)
{
	sprite.draw(*this, x, y, nr);
}
#endif


} // EOF namespace grafix
} // EOF namespace ppl7
