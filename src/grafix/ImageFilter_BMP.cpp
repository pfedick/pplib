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
#include "ppl7.h"
#include "ppl7-grafix.h"

namespace ppl7 {
namespace grafix {

/*!\class ImageFilter_BMP
 * \ingroup PPLGroupGrafik
 * \brief Import-/Export-Filter für BMP-Dateien
 */

#undef tagBITMAPFILEHEADER

#ifndef _WIN32
/*
typedef struct tagBITMAPFILEHEADER {
        uint16_t		bfType;
        uint32_t		bfSize;
        uint16_t		bfReserved1;
        uint16_t		bfReserved2;
        uint32_t		bfOffBits;
} BITMAPFILEHEADER;
*/

typedef struct tagRGBQUAD {
        uint8_t		rgbBlue;
        uint8_t		rgbGreen;
        uint8_t		rgbRed;
        uint8_t		rgbReserved;
} RGBQUAD;

/*
typedef struct tagBITMAPINFOHEADER {    //
    uint32_t   biSize;                // 0   Groesse des Bitmapinfoheaders
    uint32_t   biWidth;               // 4   Breite des Bildes
    uint32_t   biHeight;              // 8   Hoehe des Bildes
    uint16_t   biPlanes;              // 12  Planes
    uint16_t   biBitCount;            // 14  Bits/Pixel: 4,8 oder 24
    uint32_t   biCompression;         // 16  0=Keine Kompression
    uint32_t   biSizeImage;           // 20  0, wenn keine Kompression, sonst Groesse
    uint32_t   biXPelsPerMeter;		// 24
    uint32_t   biYPelsPerMeter;		// 28
    uint16_t   biClrUsed;				// 32
    uint16_t   biClrImportant;		// 34
} BITMAPINFOHEADER;
*/

#endif

#ifndef _WIN32
/*
typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO;
*/
#endif


ImageFilter_BMP::ImageFilter_BMP()
{
}

ImageFilter_BMP::~ImageFilter_BMP()
{
}

int ImageFilter_BMP::ident(FileObject &file, IMAGE &img)
{
	try {
		const char *address=file.map(0,256);
		if (address==NULL) {
			return false;
		}

		const char *bmh=address;
		if (strncmp ((char *)bmh,"BM",2)==0) {
			const char *bmia=address+14;
			uint32_t compression=Peek32((char *)(bmia+16));

			if (compression==0 || compression==3) {
				if (compression!=0 && compression!=3) {	// Nur unkomprimierte BMP's
					return false;
				}
				img.format=RGBFormat::unknown;
				img.width=Peek32(bmia+4);
				img.height=Peek32(bmia+8);
				img.bitdepth=Peek16(bmia+14);
				if (img.bitdepth!=8 && img.bitdepth!=24 && img.bitdepth!=32) {
					return false;
				}
				img.pitch=img.width*img.bitdepth/8;
				if (img.bitdepth==24) {
					if ((img.pitch & 3) != 0)
						img.pitch=((img.pitch+3)/4)*4;
				}
				if (img.bitdepth==8) {
					img.colors=256;
					img.format=RGBFormat::Palette;
				}
				if (img.bitdepth==16) {
					img.colors=65536;
					img.format=RGBFormat::R5G6B5;
				}
				if (img.bitdepth==24 || img.bitdepth==32) {
					img.colors=256*256*256;
					img.format=RGBFormat::X8R8G8B8;
				}
				return true;
			}
		}
	} catch (...) {

	}
	return 0;
}

void ImageFilter_BMP::load(FileObject &file, Drawable &surface, IMAGE &img)
{
	uint8_t * b1, *bmia;
	uint8_t *address=(uint8_t *)file.map();
	if (address==NULL) throw NullPointerException();

	//S2i *data=(S2i *)surface->internaldata;
	//CSurface *PriSurf=((DD2i *) (data->dd_internaldata))->PrimarySurface;

	bmia=address+14;
	//data->transparent=-1;


    b1= (address+Peek32((char *)(address+10)));

	int x,y;
	switch (img.bitdepth) {
		case 8:					// 8-Bit
			if (Peek32((char *)(bmia+16))==0) {	// nur unkomprimiert
				uint32_t sourcebytesperline=img.pitch;
				if ((sourcebytesperline & 3) != 0)
					sourcebytesperline=((sourcebytesperline+3)/4)*4;
				uint8_t *b2=(uint8_t*)surface.adr();
				// Ist eine Umwandlung der Bittiefe notwendig?
				if (surface.bitdepth()==img.bitdepth) { // nein
					int i;
					for (i=0;i<(int32_t)img.height;i++) {
						memcpy((char *)(b2+i*img.pitch),
							(char *)(b1+(img.height-1-i)*sourcebytesperline),
							img.pitch);
					}
					RGBQUAD * rgbq= (RGBQUAD *) (address+18+36);
					for (i=0;i<256;i++) {
						// TODO:
						//surface->SetColor(i,rgbq->rgbRed,rgbq->rgbGreen, rgbq->rgbBlue);
						rgbq++;
					}
					return;
				}
				// Surface hat eine hoehere Bittiefe, jeder Pixel muss
				// umgerechnet werden
				b1=b1+img.height*sourcebytesperline;
				RGBQUAD *basergbq= (RGBQUAD *) (address+18+36)-1;
				//Debug.Printf ("basergbq=%u\n",basergbq);

				for (int y=0;y<img.height;y++) {
					b1-=sourcebytesperline;
					for (int x=0;x<img.width;x++) {
						uint8_t pixel=b1[x];
						RGBQUAD * rgbq= basergbq+pixel;
						surface.putPixel(x,y,Color(rgbq->rgbRed, rgbq->rgbGreen, rgbq->rgbBlue,255));
					}

				}
				return;
			}
			break;
		case 24:			// 24-Bit
			b1=b1+img.height*img.pitch;
			//DLOGLEVEL(8) ("PriSurf=%u",PriSurf);

			for (y=0;y<img.height;y++) {
				b1-=img.pitch;
				for (x=0;x<img.width;x++) {
					surface.putPixel (x,y,Color(b1[x*3+2], b1[x*3+1], b1[x*3+0], 255));
				}
			}
			return;
		case 32:			// 32-Bit
            b1=b1+img.height*img.pitch;
            for (y=0;y<img.height;y++) {
                b1-=img.pitch;
                for (x=0;x<img.width;x++) {
                    surface.putPixel (x,y,Color(b1[x*4+2], b1[x*4+1], b1[x*4+0], b1[x*4+3]));
                }
            }
            return;


	} // end switch
	throw IllegalImageFormatException();
}

void ImageFilter_BMP::save (const Drawable &surface, FileObject &file, const AssocArray &param)
{
	Color pixel;
	uint32_t bpp, bfOffBits;
	if (surface.bitdepth()>8) {
		bpp=3;
		bfOffBits=54;
	} else {
		bpp=1;
		bfOffBits=54+768;
		throw UnsupportedFeatureException("ImageFilter_BMP::save with Palette");
	}

	uint32_t size=bfOffBits+(surface.width()*surface.height())*bpp;
	char *buffer=(char *)malloc(size);
	if (!buffer) throw OutOfMemoryException();
	try {
		char *bmh=buffer;
		char *bmia=bmh+14;
		Poke8(bmh,'B');
		Poke8(bmh+1,'M');
		Poke32(bmh+2,size);
		Poke16(bmh+6,0);
		Poke16(bmh+8,0);
		Poke32(bmh+10,54);				// bfOffBits=Beginn der Bitmap

		Poke32(bmia+0,40);				// biSize
		Poke32(bmia+4,surface.width());	// biWidth
		Poke32(bmia+8,surface.height());	// biHeight
		Poke16(bmia+12,1);				// biPlanes
		Poke16(bmia+14,bpp*8);			// biBitCount
		Poke32(bmia+16,0);				// biCompression
		Poke32(bmia+20,0);				// biSizeImage
		Poke32(bmia+24,0);				// biXPelsPerMeter
		Poke32(bmia+28,0);				// biYPelsPerMeter
		Poke32(bmia+32,0);				// biClrUsed
		Poke32(bmia+36,0);				// biClrImportant

		char *img=bmh+bfOffBits;
		if (surface.bitdepth()==8) {		// Palette speichern
			throw ppl7::UnsupportedFeatureException("Cannot save a ppl7::Drawable with bitdepth of 8");
			// Hier fehlt noch der Code, um die Palette zu speichern.
			// Ausserdem muss noch geprueft werden, an welcher Position die
			// Palette im BMP-File gespeichert werden muss, und was sich
			// am Header alles aendert.
		}
		for (int y=(surface.height()-1);y>=0;y--) {
			if (surface.bitdepth()>8) {
				for (int x=0;x<(surface.width());x++) {
					pixel=surface.getPixel(x,y);
					img[x*bpp]=(uint8_t)pixel.blue();
					img[x*bpp+1]=(uint8_t)pixel.green();
					img[x*bpp+2]=(uint8_t)pixel.red();
				}
			}
			img+=surface.width()*bpp;

		}

		file.write(buffer,size);
	} catch (...) {
		free (buffer);
		throw;
	}
	free (buffer);
}

String ImageFilter_BMP::name()
{
	return "BMP";
}

String ImageFilter_BMP::description()
{
	return "Windows Bitmap-Dateien";
}



} // EOF namespace grafix
} // EOF namespace ppl7


