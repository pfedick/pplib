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
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <ppl7.h>
#include "ppl7-grafix.h"


namespace ppl7 {
namespace grafix {

#ifdef TODO

typedef struct {
	ppldb	red;
	ppldb	green;
	ppldb	blue;
	ppldb	alpha;
	ppldd	value;
} PPLFONTCOLOR;

class pplFont4 {
	public:
		PFPFile File;
		void	*fonthandle;
		char	*fontadr;
		char	*fontbaseadr;
		PPLFONTCOLOR	colortab[128];
		PPLFONTCOLOR	bordercolor;
		int		x,y;
		int		fixedwidth;

		ppldd	char_nums;
		ppldd	color_nums;
		ppldd	offset_fonttab;
		ppldd	offset_colortab;
		ppldb	version;
		ppldb	fontheight;
		ppldb	fontbase;
		ppldb	fontspace;
		ppldb	space;
		ppldb	fontmaxwidth;
		ppldb	size;
		ppldb	bitsperpixel;
		ppldb	flags;
		bool	underlined;
		bool	border;
		bool	fixed;
		bool	antialias;
} PPL_FONT_INFO;

#endif

#ifdef TODO
void Font4_Init(PPL_SURFACE *surface,PPL_FONT_INFO *font,PFPSTRUCT *pfp)
{
	char *adr=font->fontadr;
	char *fontbase;
	font->fontbaseadr=adr+pfp->header_length;
	fontbase=font->fontbaseadr;
	font->version=4;
	font->char_nums=(ppldd)ppl_peekb(fontbase);
	font->color_nums=(ppldd)ppl_peekb(fontbase+1);
	font->offset_fonttab=(ppldd)ppl_peekw(fontbase+2);
	font->offset_colortab=(ppldd)ppl_peekw(fontbase+4);
	font->fontheight=(ppldb)ppl_peekb(fontbase+6);
	font->fontbase=(ppldb)ppl_peekb(fontbase+7);
	font->fontspace=(ppldb)ppl_peekb(fontbase+8);
	font->space=(ppldb)ppl_peekb(fontbase+12);
	font->fontmaxwidth=0;
	if (pfp->subversion>0) {
		font->fontmaxwidth=(ppldb)ppl_peekb(fontbase+13);
	}
	font->fixedwidth=font->fontmaxwidth;
	font->GetTextSize=Font4_TextSize;
	if (pfp->subversion>1) {
		font->bitsperpixel=(ppldb)ppl_peekb(fontbase+14);
		font->flags=(ppldb)ppl_peekb(fontbase+15);
		font->GetTextSize=Font42_TextSize;
	}
	switch(surface->bitdepth) {
		case 8:
			font->Print=NULL;
			break;
		case 16:
			font->Print=Font4_Print_16;
			if (pfp->subversion==2) font->Print=Font42_Print_16;
			break;
		case 24:
			font->Print=NULL;
			break;
		case 32:
			font->Print=Font4_Print_32;
			if (pfp->subversion==2) font->Print=Font42_Print_32;
			break;
		default:
			font->Print=NULL;
	};
}
#endif


FontEngineFont4::FontEngineFont4()
{

}

FontEngineFont4::~FontEngineFont4()
{

}

String FontEngineFont4::name() const
{
	return "FontEngineFont5";
}

String FontEngineFont4::description() const
{
	return "Rendering of PPLib Version 4 Fonts";
}

void FontEngineFont4::init()
{
	// Es gibt nichts zu tun
}

int FontEngineFont4::ident(FileObject &file) throw()
{
	PFPFile ff;
	if (!ff.ident(file)) return 0;
	if (ff.getID()!="FONT") return 0;
	if (ff.getMainVersion()==4 && ff.getSubVersion()==2) {
		return 1;
	}
	return 0;
}

#ifdef TODO

FontFile *FontEngineFont4::loadFont(FileObject &file, const String &fontname)
{
	PFPFile *Font4 = new pplFont4;
	if (!Font4) throw OutOfMemoryException();
	try {
		Font4->File.load(file);
	} catch (...) {
		delete Font4;
		throw;
	}
	if (Font4->File.getID()!="FONT") throw InvalidFontException();
	if (Font4->File.getMainVersion()!=4 || Font4->File.getSubVersion()!=2) throw InvalidFontException();
	FontFile *ff=new FontFile;
	if (!ff) {
		delete Font4;
		throw OutOfMemoryException();
	}
	if (fontname.notEmpty()) ff->Name=fontname;
	else ff->Name=Font4->File.getName();
	ff->engine=this;
	// Wir Speichern nicht den Speicherblock der Daten, sondern nur den
	// Pointer auf das PFPFile
	ff->priv=Font4;
	return ff;
}

void FontEngineFont4::deleteFont(FontFile *file)
{
	if (!file) throw NullPointerException();
	if (file->engine!=this) throw InvalidFontEngineException();
	pplFont4 *f=static_cast<pplFont4 *>(file->priv);
	delete f;
	file->priv=NULL;
	file->engine=NULL;
}

#endif //TODO


#ifdef OLD


using namespace ppl;

static int Font42_TextSize(PPL_FONT_INFO *fnt, PPL_FONT_MEASURE *measure)
{
	int		fx=0, fy=0;
	int		maxfx=0;
	int		i=0;
	char	*fonttab;
	char	*fontdata;
	ppldd	color=0;
	ppldd	border=0;
	ppldb	ascii;
	int		job=measure->job;
	measure->r.left=0;
	measure->r.top=0;
	if (fnt->border) {								// Rahmen mit einbeziehen
		border=2;
	}
	char *text=measure->text;

	int oldx=0, oldy=0;

	while ((ascii=text[i])!=0) {
		if (job==2 && i>=measure->pos) {
			measure->p.x=fx;
			measure->p.y=fy;
			return 1;
		}
		if (job==3 && i>0) {
			if (oldx<=measure->p.x && fx>=measure->p.x
				&& oldy<=measure->p.y && fy+fnt->fontheight>=measure->p.y) {
				measure->pos=i-1;
				return 1;
			}
		}
		oldx=fx;
		oldy=fy;
		if (ascii=='&') {
			/*
			 * Es koennte ein HTML-Tag verwendet worden sein
			 * erstmal ignorieren, da wir noch keine Funktion haben, die HTML in ASCII wandelt
			 */
			//i+=HTML2ASCII(text+i,&ascii,&laenge)-1;
		}

		if (ascii=='<' && text[i+1]=='u' && text[i+2]=='>') {
			//fnt->underlined=true;
			i+=2;
		} else if (ascii=='<' && text[i+1]=='/' && text[i+2]=='u' && text[i+3]=='>') {
			//fnt->underlined=false;
			i+=3;

		} else if (ascii=='~' || ascii==10 || ascii==13) {
			if (fx>maxfx) maxfx=fx;
			fx=0;
			fy+=fnt->fontheight;
		} else if (ascii==32) {
			if (fnt->fixed) {
				fx+=fnt->fixedwidth;
			} else {
				fx+=fnt->space;
			}
		} else {
			fonttab=fnt->fontbaseadr+fnt->offset_fonttab+((ppldd)ascii<<1);
			/*
			 * Gibt es den Buchstaben in der Font-Tabelle?
			 */
			if ((fontdata=(char*)ppl_peekw(fonttab))!=0) {
				fontdata=((ppldd)fontdata<<2)+fnt->fontbaseadr;
				ppldd		breite=ppl_peekb(fontdata+1)+2;
				ppldd		fxcorrect=breite-2;
				if (fnt->fixed) {
					fxcorrect=fnt->fixedwidth;
				}
				/*
				 * X-Koordinaten updaten
				 */
				if (!fnt->fixed) fxcorrect+=fnt->fontspace;

				fx+=fxcorrect;
			}
		}
		i++;
	}
	/*
	 * Fertig, nun Hoehe und Breite zurueckgeben
	 */
	if (fx>maxfx) maxfx=fx;
	measure->r.right=maxfx;
	measure->r.bottom=fy+fnt->fontheight;
	if (job<2)	return 1;
	if (job==2) {
		measure->p.x=fx;
		measure->p.y=fy;
		return 1;
	}

	return 0;
}








static int Font42_Print_32(PPL_SURFACE *surface, struct tagPPL_FONT_INFO *fnt,char *text)
{
	PPL_SRECT	*clipper=(PPL_SRECT*)&surface->clipper;
	ppldd	FontBorderColorValue=fnt->bordercolor.value;
	ppldd	i=0;
	ppldd	border=0;
	int		fx=fnt->x, fy=fnt->y;
	ppldd	*output=surface->base32+(fy-1)*surface->pitch32+(fx-1);
	ppldd	color=fnt->colortab[0].value;
	PPLFONTCOLOR	*colortab=fnt->colortab;
	char	*fonttab;
	char	*fontdata;
	ppldb	ascii,wert;

	if (fnt->border) {								// Rahmen mit einbeziehen
		border=2;
	}

	while ((ascii=text[i])!=0) {					// Hauptschleife
		if (ascii=='<' && text[i+1]=='u' && text[i+2]=='>') {
			fnt->underlined=true;
			i+=2;
		} else if (ascii=='<' && text[i+1]=='/' && text[i+2]=='u' && text[i+3]=='>') {
			fnt->underlined=false;
			i+=3;

		} else if (ascii=='~' || ascii==10 || ascii==13) {
			fx=fnt->x;
			fy+=fnt->fontheight;
			output=surface->base32+(fy-1)*surface->pitch32+(fx-1);
			//target+=(fnt->fontheight*data->pitch);
			//output=(dd*)target;											// **4 Byte
		} else if (ascii==32) {
			if (fnt->fixed) {
				if (fnt->underlined) {
					surface->Line(surface,fx,fy+fnt->fontheight-2,fx+fnt->fixedwidth,fy+fnt->fontheight-2,color);
				}
				fx+=fnt->fixedwidth;
				output+=(fnt->fixedwidth);
			} else {
				if (fnt->underlined) {
					surface->Line(surface,fx,fy+fnt->fontheight-2,fx+fnt->space-1,fy+fnt->fontheight-2,color);
				}
				fx+=fnt->space;
				output+=(fnt->space);
			}
		} else {												// Normaler Buchstabe
			fonttab=fnt->fontbaseadr+fnt->offset_fonttab+((ppldd)ascii<<1);
			/*
			 * Gibt es den Buchstaben in der Font-Tabelle?
			 */
			if ((fontdata=(char*)ppl_peekw(fonttab))!=0) {
				fontdata=((ppldd)fontdata<<2)+fnt->fontbaseadr;
				ppldd	ystart=ppl_peekb(fontdata);
				ppldd	breite=ppl_peekb(fontdata+1)+2;
				ppldd	hoehe=ppl_peekb(fontdata+2)+2;
				ppldd	*zz=output;
				ppldd	fxcorrect=breite-2;
				ppldd	o=0;
				ppldd	*qq=(ppldd*)(fontdata+3);
				if (fnt->fixed) {
					fxcorrect=fnt->fixedwidth;
					zz+=((fnt->fixedwidth-breite)>>1);					// Buchstabe mittig setzen
				}
				if (ystart>0) zz+=ystart*surface->pitch32;

				/*
				 * Liegt der Buchstabe komplett im Clipping-Bereich?
				 * Wenn ja, benutzen wir die schnelle Zeichenmethode
				 */

				if (fx>=clipper->x1 && (ppldds)(fx+fxcorrect+border)<clipper->x2 &&
					fy>=clipper->y1 && (ppldds)(fy+hoehe+border)<clipper->y2)	{		// Ja -> schnelle Routine
					/*
					 * Besteht der Font nur aus einer Farbe, wird die
					 * uebergebene Farbe benutzt, sofern diese nicht
					 * < 0x0 oder > 0x00ffffff ist
					 */
					ppldd loopsize=breite*hoehe;
					ppldd z;
					ppldd byte;
					ppldb bits=0;
					byte=qq[0];
					qq++;
					if (fnt->border) {
						for (z=0;z<loopsize;z++) {
							wert=(ppldb)(byte&3);
							if (wert) {
								if (wert>1) zz[o]=(ppldd)color;
								else zz[o]=(ppldd)FontBorderColorValue;
							}
							bits+=2;
							byte>>=2;
							o++;
							if (bits>=32) {
								bits=0;
								byte=qq[0];
								qq++;
							}
							if (o>=breite) {
								zz+=surface->pitch32;
								o=0;
							}
						}
					} else {
						for (z=0;z<loopsize;z++) {
							wert=(ppldb)(byte&3);
							if (wert) {
								if (wert>1) zz[o]=(ppldd)color;
							}
							bits+=2;
							byte>>=2;
							o++;
							if (bits>=32) {
								bits=0;
								byte=qq[0];
								qq++;
							}
							if (o>=breite) {
								zz+=surface->pitch32;
								o=0;
							}
						}
					}
				} else {
					/*
					 * Buchstabe liegt ausserhalb des Clipping-Bereichs. Es muss geprueft werden,
					 * ob der Buchstabe ueberhaupt gezeichnet werden muss und wenn ja, mussen wir jeden
					 * Pixel pruefen
					 */
				}

				/*
				 * X-Koordinaten updaten
				 */
				if (!fnt->fixed) fxcorrect+=fnt->fontspace;

				/*
				 * Unterstreichen ?
				 */
				if (fnt->underlined) {
					surface->Line(surface,fx,fy+fnt->fontheight-2,fx+fxcorrect-1,fy+fnt->fontheight-2,color);
				}
				fx+=fxcorrect;
				output+=fxcorrect;
			}

		}
		i++;
	}

	fnt->x=fx;
	fnt->y=fy;

	return 0;
}





static int Font42_Print_16(PPL_SURFACE *surface, struct tagPPL_FONT_INFO *fnt,char *text)
{
	PPL_SRECT	*clipper=(PPL_SRECT*)&surface->clipper;
	ppldw	FontBorderColorValue=(ppldw)fnt->bordercolor.value;
	ppldd	i=0;
	ppldd	border=0;
	int		fx=fnt->x, fy=fnt->y;
	ppldw	*output=surface->base16+(fy-1)*surface->pitch16+(fx-1);
	ppldw	color=(ppldw)fnt->colortab[0].value;
	PPLFONTCOLOR	*colortab=fnt->colortab;
	char	*fonttab;
	char	*fontdata;
	ppldb	ascii,wert;

	if (fnt->border) {								// Rahmen mit einbeziehen
		border=2;
	}

	while ((ascii=text[i])!=0) {					// Hauptschleife
		if (ascii=='<' && text[i+1]=='u' && text[i+2]=='>') {
			fnt->underlined=true;
			i+=2;
		} else if (ascii=='<' && text[i+1]=='/' && text[i+2]=='u' && text[i+3]=='>') {
			fnt->underlined=false;
			i+=3;

		} else if (ascii=='~' || ascii==10 || ascii==13) {
			fx=fnt->x;
			fy+=fnt->fontheight;
			output=surface->base16+(fy-1)*surface->pitch16+(fx-1);
			//target+=(fnt->fontheight*data->pitch);
			//output=(dd*)target;											// **4 Byte
		} else if (ascii==32) {
			if (fnt->fixed) {
				if (fnt->underlined) {
					surface->Line(surface,fx,fy+fnt->fontheight-2,fx+fnt->fixedwidth,fy+fnt->fontheight-2,color);
				}
				fx+=fnt->fixedwidth;
				output+=(fnt->fixedwidth);
			} else {
				if (fnt->underlined) {
					surface->Line(surface,fx,fy+fnt->fontheight-2,fx+fnt->space-1,fy+fnt->fontheight-2,color);
				}
				fx+=fnt->space;
				output+=(fnt->space);
			}
		} else {												// Normaler Buchstabe
			fonttab=fnt->fontbaseadr+fnt->offset_fonttab+((ppldd)ascii<<1);
			/*
			 * Gibt es den Buchstaben in der Font-Tabelle?
			 */
			if ((fontdata=(char*)ppl_peekw(fonttab))!=0) {
				fontdata=((ppldd)fontdata<<2)+fnt->fontbaseadr;
				ppldd	ystart=ppl_peekb(fontdata);
				ppldd	breite=ppl_peekb(fontdata+1)+2;
				ppldd	hoehe=ppl_peekb(fontdata+2)+2;
				ppldw	*zz=output;
				ppldd	fxcorrect=breite-2;
				ppldd	o=0;
				ppldd	*qq=(ppldd*)(fontdata+3);
				if (fnt->fixed) {
					fxcorrect=fnt->fixedwidth;
					zz+=((fnt->fixedwidth-breite)>>1);					// Buchstabe mittig setzen
				}
				if (ystart>0) zz+=ystart*surface->pitch16;

				/*
				 * Liegt der Buchstabe komplett im Clipping-Bereich?
				 * Wenn ja, benutzen wir die schnelle Zeichenmethode
				 */

				if (fx>=clipper->x1 && (ppldds)(fx+fxcorrect+border)<clipper->x2 &&
					fy>=clipper->y1 && (ppldds)(fy+hoehe+border)<clipper->y2)	{		// Ja -> schnelle Routine
					/*
					 * Besteht der Font nur aus einer Farbe, wird die
					 * uebergebene Farbe benutzt, sofern diese nicht
					 * < 0x0 oder > 0x00ffffff ist
					 */
					ppldd loopsize=breite*hoehe;
					ppldd z;
					ppldd byte;
					ppldb bits=0;
					byte=qq[0];
					qq++;

					for (z=0;z<loopsize;z++) {
						wert=(ppldb)(byte&3);
						if (wert) {
							if (wert>1) zz[o]=(ppldw)color;
							else if(fnt->border) zz[o]=(ppldw)FontBorderColorValue;
						}
						bits+=2;
						byte>>=2;
						if (bits>=32) {
							bits=0;
							byte=qq[0];
							qq++;
						}
						o++;
						if (o>=breite) {
							o=0;
							zz+=surface->pitch16;
						}
					}
				} else {
					/*
					 * Buchstabe liegt ausserhalb des Clipping-Bereichs. Es muss geprueft werden,
					 * ob der Buchstabe ueberhaupt gezeichnet werden muss und wenn ja, mussen wir jeden
					 * Pixel pruefen
					 */
				}

				/*
				 * X-Koordinaten updaten
				 */
				if (!fnt->fixed) fxcorrect+=fnt->fontspace;

				/*
				 * Unterstreichen ?
				 */
				if (fnt->underlined) {
					surface->Line(surface,fx,fy+fnt->fontheight-2,fx+fxcorrect-1,fy+fnt->fontheight-2,color);
				}
				fx+=fxcorrect;
				output+=fxcorrect;
			}

		}
		i++;
	}

	fnt->x=fx;
	fnt->y=fy;

	return 0;
}


#endif // OLD


#ifdef OLD


using namespace ppl;

static int Font42_TextSize(PPL_FONT_INFO *fnt, PPL_FONT_MEASURE *measure)
{
	int		fx=0, fy=0;
	int		maxfx=0;
	int		i=0;
	char	*fonttab;
	char	*fontdata;
	ppldd	color=0;
	ppldd	border=0;
	ppldb	ascii;
	int		job=measure->job;
	measure->r.left=0;
	measure->r.top=0;
	if (fnt->border) {								// Rahmen mit einbeziehen
		border=2;
	}
	char *text=measure->text;

	int oldx=0, oldy=0;

	while ((ascii=text[i])!=0) {
		if (job==2 && i>=measure->pos) {
			measure->p.x=fx;
			measure->p.y=fy;
			return 1;
		}
		if (job==3 && i>0) {
			if (oldx<=measure->p.x && fx>=measure->p.x
				&& oldy<=measure->p.y && fy+fnt->fontheight>=measure->p.y) {
				measure->pos=i-1;
				return 1;
			}
		}
		oldx=fx;
		oldy=fy;
		if (ascii=='&') {
			/*
			 * Es koennte ein HTML-Tag verwendet worden sein
			 * erstmal ignorieren, da wir noch keine Funktion haben, die HTML in ASCII wandelt
			 */
			//i+=HTML2ASCII(text+i,&ascii,&laenge)-1;
		}

		if (ascii=='<' && text[i+1]=='u' && text[i+2]=='>') {
			//fnt->underlined=true;
			i+=2;
		} else if (ascii=='<' && text[i+1]=='/' && text[i+2]=='u' && text[i+3]=='>') {
			//fnt->underlined=false;
			i+=3;

		} else if (ascii=='~' || ascii==10 || ascii==13) {
			if (fx>maxfx) maxfx=fx;
			fx=0;
			fy+=fnt->fontheight;
		} else if (ascii==32) {
			if (fnt->fixed) {
				fx+=fnt->fixedwidth;
			} else {
				fx+=fnt->space;
			}
		} else {
			fonttab=fnt->fontbaseadr+fnt->offset_fonttab+((ppldd)ascii<<1);
			/*
			 * Gibt es den Buchstaben in der Font-Tabelle?
			 */
			if ((fontdata=(char*)ppl_peekw(fonttab))!=0) {
				fontdata=((ppldd)fontdata<<2)+fnt->fontbaseadr;
				ppldd		breite=ppl_peekb(fontdata+1)+2;
				ppldd		fxcorrect=breite-2;
				if (fnt->fixed) {
					fxcorrect=fnt->fixedwidth;
				}
				/*
				 * X-Koordinaten updaten
				 */
				if (!fnt->fixed) fxcorrect+=fnt->fontspace;

				fx+=fxcorrect;
			}
		}
		i++;
	}
	/*
	 * Fertig, nun Hoehe und Breite zurueckgeben
	 */
	if (fx>maxfx) maxfx=fx;
	measure->r.right=maxfx;
	measure->r.bottom=fy+fnt->fontheight;
	if (job<2)	return 1;
	if (job==2) {
		measure->p.x=fx;
		measure->p.y=fy;
		return 1;
	}

	return 0;
}








static int Font42_Print_32(PPL_SURFACE *surface, struct tagPPL_FONT_INFO *fnt,char *text)
{
	PPL_SRECT	*clipper=(PPL_SRECT*)&surface->clipper;
	ppldd	FontBorderColorValue=fnt->bordercolor.value;
	ppldd	i=0;
	ppldd	border=0;
	int		fx=fnt->x, fy=fnt->y;
	ppldd	*output=surface->base32+(fy-1)*surface->pitch32+(fx-1);
	ppldd	color=fnt->colortab[0].value;
	PPLFONTCOLOR	*colortab=fnt->colortab;
	char	*fonttab;
	char	*fontdata;
	ppldb	ascii,wert;

	if (fnt->border) {								// Rahmen mit einbeziehen
		border=2;
	}

	while ((ascii=text[i])!=0) {					// Hauptschleife
		if (ascii=='<' && text[i+1]=='u' && text[i+2]=='>') {
			fnt->underlined=true;
			i+=2;
		} else if (ascii=='<' && text[i+1]=='/' && text[i+2]=='u' && text[i+3]=='>') {
			fnt->underlined=false;
			i+=3;

		} else if (ascii=='~' || ascii==10 || ascii==13) {
			fx=fnt->x;
			fy+=fnt->fontheight;
			output=surface->base32+(fy-1)*surface->pitch32+(fx-1);
			//target+=(fnt->fontheight*data->pitch);
			//output=(dd*)target;											// **4 Byte
		} else if (ascii==32) {
			if (fnt->fixed) {
				if (fnt->underlined) {
					surface->Line(surface,fx,fy+fnt->fontheight-2,fx+fnt->fixedwidth,fy+fnt->fontheight-2,color);
				}
				fx+=fnt->fixedwidth;
				output+=(fnt->fixedwidth);
			} else {
				if (fnt->underlined) {
					surface->Line(surface,fx,fy+fnt->fontheight-2,fx+fnt->space-1,fy+fnt->fontheight-2,color);
				}
				fx+=fnt->space;
				output+=(fnt->space);
			}
		} else {												// Normaler Buchstabe
			fonttab=fnt->fontbaseadr+fnt->offset_fonttab+((ppldd)ascii<<1);
			/*
			 * Gibt es den Buchstaben in der Font-Tabelle?
			 */
			if ((fontdata=(char*)ppl_peekw(fonttab))!=0) {
				fontdata=((ppldd)fontdata<<2)+fnt->fontbaseadr;
				ppldd	ystart=ppl_peekb(fontdata);
				ppldd	breite=ppl_peekb(fontdata+1)+2;
				ppldd	hoehe=ppl_peekb(fontdata+2)+2;
				ppldd	*zz=output;
				ppldd	fxcorrect=breite-2;
				ppldd	o=0;
				ppldd	*qq=(ppldd*)(fontdata+3);
				if (fnt->fixed) {
					fxcorrect=fnt->fixedwidth;
					zz+=((fnt->fixedwidth-breite)>>1);					// Buchstabe mittig setzen
				}
				if (ystart>0) zz+=ystart*surface->pitch32;

				/*
				 * Liegt der Buchstabe komplett im Clipping-Bereich?
				 * Wenn ja, benutzen wir die schnelle Zeichenmethode
				 */

				if (fx>=clipper->x1 && (ppldds)(fx+fxcorrect+border)<clipper->x2 &&
					fy>=clipper->y1 && (ppldds)(fy+hoehe+border)<clipper->y2)	{		// Ja -> schnelle Routine
					/*
					 * Besteht der Font nur aus einer Farbe, wird die
					 * uebergebene Farbe benutzt, sofern diese nicht
					 * < 0x0 oder > 0x00ffffff ist
					 */
					ppldd loopsize=breite*hoehe;
					ppldd z;
					ppldd byte;
					ppldb bits=0;
					byte=qq[0];
					qq++;
					if (fnt->border) {
						for (z=0;z<loopsize;z++) {
							wert=(ppldb)(byte&3);
							if (wert) {
								if (wert>1) zz[o]=(ppldd)color;
								else zz[o]=(ppldd)FontBorderColorValue;
							}
							bits+=2;
							byte>>=2;
							o++;
							if (bits>=32) {
								bits=0;
								byte=qq[0];
								qq++;
							}
							if (o>=breite) {
								zz+=surface->pitch32;
								o=0;
							}
						}
					} else {
						for (z=0;z<loopsize;z++) {
							wert=(ppldb)(byte&3);
							if (wert) {
								if (wert>1) zz[o]=(ppldd)color;
							}
							bits+=2;
							byte>>=2;
							o++;
							if (bits>=32) {
								bits=0;
								byte=qq[0];
								qq++;
							}
							if (o>=breite) {
								zz+=surface->pitch32;
								o=0;
							}
						}
					}
				} else {
					/*
					 * Buchstabe liegt ausserhalb des Clipping-Bereichs. Es muss geprueft werden,
					 * ob der Buchstabe ueberhaupt gezeichnet werden muss und wenn ja, mussen wir jeden
					 * Pixel pruefen
					 */
				}

				/*
				 * X-Koordinaten updaten
				 */
				if (!fnt->fixed) fxcorrect+=fnt->fontspace;

				/*
				 * Unterstreichen ?
				 */
				if (fnt->underlined) {
					surface->Line(surface,fx,fy+fnt->fontheight-2,fx+fxcorrect-1,fy+fnt->fontheight-2,color);
				}
				fx+=fxcorrect;
				output+=fxcorrect;
			}

		}
		i++;
	}

	fnt->x=fx;
	fnt->y=fy;

	return 0;
}





static int Font42_Print_16(PPL_SURFACE *surface, struct tagPPL_FONT_INFO *fnt,char *text)
{
	PPL_SRECT	*clipper=(PPL_SRECT*)&surface->clipper;
	ppldw	FontBorderColorValue=(ppldw)fnt->bordercolor.value;
	ppldd	i=0;
	ppldd	border=0;
	int		fx=fnt->x, fy=fnt->y;
	ppldw	*output=surface->base16+(fy-1)*surface->pitch16+(fx-1);
	ppldw	color=(ppldw)fnt->colortab[0].value;
	PPLFONTCOLOR	*colortab=fnt->colortab;
	char	*fonttab;
	char	*fontdata;
	ppldb	ascii,wert;

	if (fnt->border) {								// Rahmen mit einbeziehen
		border=2;
	}

	while ((ascii=text[i])!=0) {					// Hauptschleife
		if (ascii=='<' && text[i+1]=='u' && text[i+2]=='>') {
			fnt->underlined=true;
			i+=2;
		} else if (ascii=='<' && text[i+1]=='/' && text[i+2]=='u' && text[i+3]=='>') {
			fnt->underlined=false;
			i+=3;

		} else if (ascii=='~' || ascii==10 || ascii==13) {
			fx=fnt->x;
			fy+=fnt->fontheight;
			output=surface->base16+(fy-1)*surface->pitch16+(fx-1);
			//target+=(fnt->fontheight*data->pitch);
			//output=(dd*)target;											// **4 Byte
		} else if (ascii==32) {
			if (fnt->fixed) {
				if (fnt->underlined) {
					surface->Line(surface,fx,fy+fnt->fontheight-2,fx+fnt->fixedwidth,fy+fnt->fontheight-2,color);
				}
				fx+=fnt->fixedwidth;
				output+=(fnt->fixedwidth);
			} else {
				if (fnt->underlined) {
					surface->Line(surface,fx,fy+fnt->fontheight-2,fx+fnt->space-1,fy+fnt->fontheight-2,color);
				}
				fx+=fnt->space;
				output+=(fnt->space);
			}
		} else {												// Normaler Buchstabe
			fonttab=fnt->fontbaseadr+fnt->offset_fonttab+((ppldd)ascii<<1);
			/*
			 * Gibt es den Buchstaben in der Font-Tabelle?
			 */
			if ((fontdata=(char*)ppl_peekw(fonttab))!=0) {
				fontdata=((ppldd)fontdata<<2)+fnt->fontbaseadr;
				ppldd	ystart=ppl_peekb(fontdata);
				ppldd	breite=ppl_peekb(fontdata+1)+2;
				ppldd	hoehe=ppl_peekb(fontdata+2)+2;
				ppldw	*zz=output;
				ppldd	fxcorrect=breite-2;
				ppldd	o=0;
				ppldd	*qq=(ppldd*)(fontdata+3);
				if (fnt->fixed) {
					fxcorrect=fnt->fixedwidth;
					zz+=((fnt->fixedwidth-breite)>>1);					// Buchstabe mittig setzen
				}
				if (ystart>0) zz+=ystart*surface->pitch16;

				/*
				 * Liegt der Buchstabe komplett im Clipping-Bereich?
				 * Wenn ja, benutzen wir die schnelle Zeichenmethode
				 */

				if (fx>=clipper->x1 && (ppldds)(fx+fxcorrect+border)<clipper->x2 &&
					fy>=clipper->y1 && (ppldds)(fy+hoehe+border)<clipper->y2)	{		// Ja -> schnelle Routine
					/*
					 * Besteht der Font nur aus einer Farbe, wird die
					 * uebergebene Farbe benutzt, sofern diese nicht
					 * < 0x0 oder > 0x00ffffff ist
					 */
					ppldd loopsize=breite*hoehe;
					ppldd z;
					ppldd byte;
					ppldb bits=0;
					byte=qq[0];
					qq++;

					for (z=0;z<loopsize;z++) {
						wert=(ppldb)(byte&3);
						if (wert) {
							if (wert>1) zz[o]=(ppldw)color;
							else if(fnt->border) zz[o]=(ppldw)FontBorderColorValue;
						}
						bits+=2;
						byte>>=2;
						if (bits>=32) {
							bits=0;
							byte=qq[0];
							qq++;
						}
						o++;
						if (o>=breite) {
							o=0;
							zz+=surface->pitch16;
						}
					}
				} else {
					/*
					 * Buchstabe liegt ausserhalb des Clipping-Bereichs. Es muss geprueft werden,
					 * ob der Buchstabe ueberhaupt gezeichnet werden muss und wenn ja, mussen wir jeden
					 * Pixel pruefen
					 */
				}

				/*
				 * X-Koordinaten updaten
				 */
				if (!fnt->fixed) fxcorrect+=fnt->fontspace;

				/*
				 * Unterstreichen ?
				 */
				if (fnt->underlined) {
					surface->Line(surface,fx,fy+fnt->fontheight-2,fx+fxcorrect-1,fy+fnt->fontheight-2,color);
				}
				fx+=fxcorrect;
				output+=fxcorrect;
			}

		}
		i++;
	}

	fnt->x=fx;
	fnt->y=fy;

	return 0;
}


#endif // OLD



} // EOF namespace grafix
} // EOF namespace ppl7
