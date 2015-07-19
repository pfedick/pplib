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

/*!\class CFilter_TGA
 * \ingroup PPLGroupGrafik
 * \brief Import-Filter für TGA-Dateien
 */


typedef struct _TgaHeader {
	ppldb IDLength;		/* 00h Size of Image ID field */
	ppldb ColorMapType;	/* 01h Color map type */
	ppldb ImageType;		/* 02h Image type code */
	ppldw CMapStart;		/* 03h Color map origin */
	ppldw CMapLength;	/* 05h Color map length */
	ppldb CMapDepth;		/* 07h Depth of color map entries */
	ppldw XOffset;		/* 08h X origin of image */
	ppldw YOffset;		/* 0Ah Y origin of image */
	ppldw Width;		/* 0Ch Width of image */
	ppldw Height;		/* 0Eh Height of image */
	ppldb PixelDepth;	/* 10h Image pixel size */
	ppldb ImageDescriptor;	/* 11h Image descriptor byte */
} TGAHEAD;

static void PeekHeader(const char *address, TGAHEAD *tga)
{
	tga->IDLength=(ppldb)peekb(address+0);
	tga->ColorMapType=(ppldb)peekb(address+1);
	tga->ImageType=(ppldb)peekb(address+2);
	tga->CMapStart=(ppldw)peekw(address+3);
	tga->CMapLength=(ppldw)peekw(address+5);
	tga->CMapDepth=(ppldb)peekb(address+7);
	tga->XOffset=(ppldw)peekw(address+8);
	tga->YOffset=(ppldw)peekw(address+10);
	tga->Width=(ppldw)peekw(address+12);
	tga->Height=(ppldw)peekw(address+14);
	tga->PixelDepth=(ppldb)peekb(address+16);
	tga->ImageDescriptor=(ppldb)peekb(address+17);
}

/*
static void PrintTGAHeader(TGAHEAD *tga)
{
	printf ("tga->IDLength=%u (%u)\n",tga->IDLength, &tga->IDLength);
	printf ("tga->ColorMapType=%u (%u)\n",tga->ColorMapType, &tga->ColorMapType);
	printf ("tga->ImageType=%u (%u)\n",tga->ImageType, &tga->ImageType);
	printf ("tga->CMapStart=%u (%u)\n",tga->CMapStart, &tga->CMapStart);
	printf ("tga->CMapLength=%u (%u)\n",tga->CMapLength, &tga->CMapLength);
	printf ("tga->CMapDepth=%u (%u)\n",tga->CMapDepth, &tga->CMapDepth);
	printf ("tga->XOffset=%u (%u)\n",tga->XOffset, &tga->XOffset);
	printf ("tga->YOffset=%u (%u)\n",tga->YOffset, &tga->YOffset);
	printf ("tga->Width=%u (%u)\n",tga->Width, &tga->Width);
	printf ("tga->Height=%u (%u)\n",tga->Height, &tga->Height);
	printf ("tga->PixelDepth=%u (%u)\n",tga->PixelDepth, &tga->PixelDepth);
	printf ("tga->ImageDescriptor=%u (%u)\n",tga->ImageDescriptor, &tga->ImageDescriptor);
}
*/

CFilter_TGA::CFilter_TGA()
{
}

CFilter_TGA::~CFilter_TGA()
{
}

int CFilter_TGA::Ident(CFileObject &file, IMAGE &img)
{
	TGAHEAD tgafield, *tga=&tgafield;
    const char *address=file.Map(0,256);
    if (address==NULL) return false;
	PeekHeader(address,tga);
	//PrintTGAHeader(tga);

	//printf ("Pruefe auf TGA... ");

	if (tga->IDLength==0 &&
		tga->ColorMapType==0 &&
		(tga->ImageDescriptor==32 || tga->ImageDescriptor==0) &&
		(tga->PixelDepth==24 || tga->PixelDepth==32) &&
		(tga->ImageType==2	|| tga->ImageType==10) ) {
			img.width=(ppldd)tga->Width;
			img.height=(ppldd)tga->Height;
			img.bitdepth=(ppldd)tga->PixelDepth;
			img.pitch=img.width*img.bitdepth/8;
			img.colors=(ppldd)256*256*256;
			SetError(0);
			//printf ("ok\n");
			return true;

	}
	//printf ("failed\n");
	SetError(20);
	return false;
}

int CFilter_TGA::Load(CFileObject &file, CDrawable &surface, IMAGE &img)
{
	TGAHEAD tgafield, *tga=&tgafield;
	ppluint8 	* b1;
	Color	farbwert;
    ppluint8 *address=(ppluint8*)file.Map();
	if (address==NULL) {SetError(2); return false; }

	PeekHeader((char*)address,tga);

	//S2i *data=(S2i *)surface->internaldata;
	//CSurface *PriSurf=((DD2i *) (data->dd_internaldata))->PrimarySurface;
	//gby=img.height*img.pitch;
    //by=gby;

	//printf ("width: %u, height: %u, bitdepth: %u\n",surface->width, surface->height, surface->bitdepth);
    //printf ("img->width: %u, img->height: %u, img->bitdepth: %u\n",img->width, img->height, img->bitdepth);

	b1=address+18+tga->IDLength;
	ppldd mpl=3;

	switch (tga->ImageType) {
		case 2:					// Unkomprimierte Daten
			//printf ("Lese unkomprimierte Bilddaten...\n");
			if (tga->PixelDepth==32) mpl=4;
			if (tga->ImageDescriptor==0) {
				for (int y=(img.height-1);y>=0;y++) {
                	for (int x=0;x<img.width;x++) {
                    	farbwert=Color(b1[x*mpl+2], b1[x*mpl+1], b1[x*mpl] );
                    	surface.putPixel (x,y,farbwert);
                	}
                	b1+=img.pitch;
            	}

			} else {
				for (int y=0;y<img.height;y++) {
                	for (int x=0;x<img.width;x++) {
                    	farbwert=Color(b1[x*mpl+2], b1[x*mpl+1], b1[x*mpl] );
                    	surface.putPixel (x,y,farbwert);
                	}
                	b1+=img.pitch;
            	}
			}
            return true;
			break;

		case 10:				// Komprimierte Daten
			int y=0,x=0,byte1,repeat;
			int zeilen=0;
			int ym=1;

			if (tga->ImageDescriptor==0) {ym=-1; y=img.height-1; }


			while (zeilen<img.height) {
				byte1=peekb((char*)b1++);
				repeat=(byte1&127)+1;
				if (byte1&128) {	// Bit 7 gesetzt, es folgt daher ein Farbwert zur Wiederholung
					if (tga->PixelDepth==24) {
						//printf ("Addiere 3 Byte dazu\n");
						farbwert=Color(b1[2], b1[1], b1[0] );
						b1+=3;
					} else {
						farbwert=Color(b1[2], b1[1], b1[0],b1[3]);
						b1+=4;
					}
					//printf ("%u mal gleiche Farbe: %u\n",repeat,farbwert);
					for (int i=0;i<repeat;i++) {
						surface.putPixel(x++,y,farbwert);
						if (x>=img.width) { x=0;y+=ym; zeilen++;}

					}
				} else {			// Bit 7 geloescht, es folgen unkomprimierte Daten
					//printf ("%u mal verschiedene Farben\n",repeat);
					for (int i=0;i<repeat;i++) {
						if (tga->PixelDepth==24) {
							farbwert=Color(b1[2], b1[1], b1[0] );
							b1+=3;
						} else {
							farbwert=Color(b1[2], b1[1], b1[0],b1[3]);
							b1+=4;
						}
						surface.putPixel(x++,y,farbwert);
						if (x>=img.width) { x=0;y+=ym; zeilen++;}
					}
				}
			}
			return true;
			break;

	} // end switch

	SetError(33);
	return false;
}

int CFilter_TGA::Save (const CDrawable &surface, CFileObject &file, CAssocArray *param)
{
	SetError(1020);
	return false;
}

CString CFilter_TGA::Name()
{
	return "TGA";
}

CString CFilter_TGA::Description()
{
	return "TGA (Loader only)";
}


} // EOF namespace grafix
} // EOF namespace ppl6


