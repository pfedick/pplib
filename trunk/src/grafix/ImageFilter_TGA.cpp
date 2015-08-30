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
#include "ppl7.h"
#include "ppl7-grafix.h"

namespace ppl7 {
namespace grafix {

/*!\class ImageFilter_TGA
 * \ingroup PPLGroupGrafik
 * \brief Import-Filter für TGA-Dateien
 */

typedef struct _TgaHeader {
	ppluint8 IDLength;		/* 00h Size of Image ID field */
	ppluint8 ColorMapType;	/* 01h Color map type */
	ppluint8 ImageType;		/* 02h Image type code */
	ppluint16 CMapStart;		/* 03h Color map origin */
	ppluint16 CMapLength;	/* 05h Color map length */
	ppluint8 CMapDepth;		/* 07h Depth of color map entries */
	ppluint16 XOffset;		/* 08h X origin of image */
	ppluint16 YOffset;		/* 0Ah Y origin of image */
	ppluint16 Width;		/* 0Ch Width of image */
	ppluint16 Height;		/* 0Eh Height of image */
	ppluint8 PixelDepth;	/* 10h Image pixel size */
	ppluint8 ImageDescriptor;	/* 11h Image descriptor byte */
} TGAHEAD;

static void PeekHeader(const char *address, TGAHEAD *tga)
{
	tga->IDLength=(ppluint8)Peek8(address+0);
	tga->ColorMapType=(ppluint8)Peek8(address+1);
	tga->ImageType=(ppluint8)Peek8(address+2);
	tga->CMapStart=(ppluint16)Peek16(address+3);
	tga->CMapLength=(ppluint16)Peek16(address+5);
	tga->CMapDepth=(ppluint8)Peek8(address+7);
	tga->XOffset=(ppluint16)Peek16(address+8);
	tga->YOffset=(ppluint16)Peek16(address+10);
	tga->Width=(ppluint16)Peek16(address+12);
	tga->Height=(ppluint16)Peek16(address+14);
	tga->PixelDepth=(ppluint8)Peek8(address+16);
	tga->ImageDescriptor=(ppluint8)Peek8(address+17);
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

ImageFilter_TGA::ImageFilter_TGA()
{
}

ImageFilter_TGA::~ImageFilter_TGA()
{
}

int ImageFilter_TGA::ident(FileObject &file, IMAGE &img)
{
	try {
		TGAHEAD tgafield, *tga=&tgafield;
		const char *address=file.map(0,256);
		if (address==NULL) return 0;
		PeekHeader(address,tga);
		//PrintTGAHeader(tga);

		//printf ("Pruefe auf TGA... ");

		if (tga->IDLength==0 &&
				tga->ColorMapType==0 &&
				(tga->ImageDescriptor==32 || tga->ImageDescriptor==0) &&
				(tga->PixelDepth==24 || tga->PixelDepth==32) &&
				(tga->ImageType==2	|| tga->ImageType==10) ) {
			img.width=(ppluint32)tga->Width;
			img.height=(ppluint32)tga->Height;
			img.bitdepth=(ppluint32)tga->PixelDepth;
			img.pitch=img.width*img.bitdepth/8;
			img.colors=(ppluint32)256*256*256;
			return 1;

		}
	} catch (...) {
		return 0;
	}
	return 0;
}

void ImageFilter_TGA::load(FileObject &file, Drawable &surface, IMAGE &img)
{
	TGAHEAD tgafield, *tga=&tgafield;
	ppluint8 	* b1;
	Color	farbwert;
    ppluint8 *address=(ppluint8*)file.map();

	PeekHeader((char*)address,tga);

	//S2i *data=(S2i *)surface->internaldata;
	//CSurface *PriSurf=((DD2i *) (data->dd_internaldata))->PrimarySurface;
	//gby=img.height*img.pitch;
    //by=gby;

	//printf ("width: %u, height: %u, bitdepth: %u\n",surface->width, surface->height, surface->bitdepth);
    //printf ("img->width: %u, img->height: %u, img->bitdepth: %u\n",img->width, img->height, img->bitdepth);

	b1=address+18+tga->IDLength;
	ppluint32 mpl=3;

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
            return;

		case 10:				// Komprimierte Daten
			int y=0,x=0;
			int zeilen=0;
			int ym=1;

			if (tga->ImageDescriptor==0) {ym=-1; y=img.height-1; }


			while (zeilen<img.height) {
				int byte1=Peek8((char*)b1++);
				int repeat=(byte1&127)+1;
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
			return;

	} // end switch
	throw UnknownImageFormatException();
}

void ImageFilter_TGA::save (const Drawable &surface, FileObject &file, const AssocArray &param)
{
	throw UnsupportedFeatureException("ImageFilter_TGA::save");
}

String ImageFilter_TGA::name()
{
	return "TGA";
}

String ImageFilter_TGA::description()
{
	return "TGA (Loader only)";
}


} // EOF namespace grafix
} // EOF namespace ppl7


