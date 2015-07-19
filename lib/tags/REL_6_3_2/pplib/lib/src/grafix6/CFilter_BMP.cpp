/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.12 $
 * $Date: 2009/06/22 13:10:31 $
 * $Id: CFilter_BMP.cpp,v 1.12 2009/06/22 13:10:31 patrick Exp $
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

/*!\class CFilter_BMP
 * \ingroup PPLGroupGrafik
 * \brief Import-/Export-Filter für BMP-Dateien
 */

#undef tagBITMAPFILEHEADER

#ifndef _WIN32
typedef struct tagBITMAPFILEHEADER {
        ppldw		bfType;
        ppldd		bfSize;
        ppldw		bfReserved1;
        ppldw		bfReserved2;
        ppldd		bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagRGBQUAD {
        ppldb		rgbBlue;
        ppldb		rgbGreen;
        ppldb		rgbRed;
        ppldb		rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFOHEADER {    /* bmih */
    ppldd   biSize;                // 0   Groesse des Bitmapinfoheaders
    ppldd   biWidth;               // 4   Breite des Bildes
    ppldd   biHeight;              // 8   Hoehe des Bildes
    ppldw   biPlanes;              // 12  Planes
    ppldw   biBitCount;            // 14  Bits/Pixel: 4,8 oder 24
    ppldd   biCompression;         // 16  0=Keine Kompression
    ppldd   biSizeImage;           // 20  0, wenn keine Kompression, sonst Groesse
    ppldd   biXPelsPerMeter;		// 24
    ppldd   biYPelsPerMeter;		// 28
    ppldw   biClrUsed;				// 32
    ppldw   biClrImportant;		// 34
} BITMAPINFOHEADER;

#endif

#ifndef _WIN32
typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO;
#endif


CFilter_BMP::CFilter_BMP()
{
}

CFilter_BMP::~CFilter_BMP()
{
}

int CFilter_BMP::Ident(CFileObject *file, IMAGE *img)
{
	const char *address=file->Map(0,256);
	if (address==NULL) {
		SetError(1018);
		return false;
	}

	const char *bmh, *bmia;
	bmh=address;
	if (strncmp ((char *)bmh,"BM",2)==0) {
		bmia=address+14;
		ppldd compression=peekd((char *)(bmia+16));

		if (compression==0 || compression==3) {
			if (compression!=0 && compression!=3) {	// Nur unkomprimierte BMP's
				SetError(1018,"Komprimiertes BMP wird nicht unterstuetzt");
				return false;
			}
			img->format=RGBFormat::unknown;
			img->width=peekd(bmia+4);
			img->height=peekd(bmia+8);
			img->bitdepth=peekw(bmia+14);
			if (img->bitdepth!=8 && img->bitdepth!=24 && img->bitdepth!=32) {
				SetError(1018,"bitdepth wird nicht unterstützt");
				return false;
			}
			img->pitch=img->width*img->bitdepth/8;
			if (img->bitdepth==24) {
				if ((img->pitch & 3) != 0)
					img->pitch=((img->pitch+3)/4)*4;
			}
			if (img->bitdepth==8) {
				img->colors=256;
				img->format=RGBFormat::Palette;
			}
			if (img->bitdepth==16) {
				img->colors=65536;
				img->format=RGBFormat::R5G6B5;
			}
			if (img->bitdepth==24 || img->bitdepth==32) {
				img->colors=256*256*256;
				img->format=RGBFormat::X8R8G8B8;
			}
			SetError(0);
			return true;
		}
	}
	SetError(1018);
	return false;
}

int CFilter_BMP::Load(CFileObject * file, CSurface *surface, IMAGE *img)
{
	RGBQUAD * rgbq,*basergbq;
	ppluint8 * b1, * b2, *bmia;
	ppldd gby,by,sourcebytesperline;
	ppldd farbwert;
	ppldb pixel;

	if (file==NULL || surface==NULL ||img==NULL) {
		SetError(136);
		return false;
	}


	ppluint8 *address=(ppluint8 *)file->Map();
	if (address==NULL) {
		SetError(2);
		return false;
	}

	//S2i *data=(S2i *)surface->internaldata;
	//CSurface *PriSurf=((DD2i *) (data->dd_internaldata))->PrimarySurface;

	bmia=address+14;
	//data->transparent=-1;

    gby=img->height*img->pitch;
    by=gby;
    if (img->bitdepth==8) by+=256*4;

    b2=(ppluint8*)surface->Lock();
	if (!b2) {
		SetError(145);
		return false;
	}


    b1= (address+peekd((char *)(address+10)));

	int x,y;
	switch (img->bitdepth) {
		case 8:					// 8-Bit
			if (peekd((char *)(bmia+16))==0) {	// nur unkomprimiert
				sourcebytesperline=img->pitch;
				if ((sourcebytesperline & 3) != 0)
					sourcebytesperline=((sourcebytesperline+3)/4)*4;

				// Ist eine Umwandlung der Bittiefe notwendig?
				if (surface->GetBitdepth()==img->bitdepth) { // nein
					int i;
					for (i=0;i<(ppldds)img->height;i++) {
						memcpy((char *)(b2+i*img->pitch),
							(char *)(b1+(img->height-1-i)*sourcebytesperline),
							img->pitch);
					}
					rgbq= (RGBQUAD *) (address+18+sizeof(BITMAPINFOHEADER));
					for (i=0;i<256;i++) {
						// TODO:
						//surface->SetColor(i,rgbq->rgbRed,rgbq->rgbGreen, rgbq->rgbBlue);
						rgbq++;
					}
					surface->Unlock();
					SetError(0);
					return true;
				}
				// Surface hat eine hoehere Bittiefe, jeder Pixel muss
				// umgerechnet werden
				b1=b1+img->height*sourcebytesperline;
				basergbq= (RGBQUAD *) (address+18+sizeof(BITMAPINFOHEADER))-1;
				//Debug.Printf ("basergbq=%u\n",basergbq);

				for (int y=0;y<img->height;y++) {
					b1-=sourcebytesperline;
					for (int x=0;x<img->width;x++) {
						pixel=b1[x];
						rgbq= basergbq+pixel;
						farbwert=surface->RGB(rgbq->rgbRed,
														   rgbq->rgbGreen,
														   rgbq->rgbBlue);
						surface->PutPixel(x,y,farbwert);
					}

				}
				surface->Unlock();
				SetError(0);
				return true;
			}
			break;
		case 24:			// 24-Bit
			b1=b1+img->height*img->pitch;
			//DLOGLEVEL(8) ("PriSurf=%u",PriSurf);

			for (y=0;y<img->height;y++) {
				b1-=img->pitch;
				for (x=0;x<img->width;x++) {
					farbwert=surface->RGB(b1[x*3+2], b1[x*3+1], b1[x*3] );
					surface->PutPixel (x,y,farbwert);
				}
			}
			SetError(0);
			surface->Unlock();
			return true;
		case 32:			// 32-Bit
            b1=b1+img->height*img->pitch;
            for (y=0;y<img->height;y++) {
                b1-=img->pitch;
                for (x=0;x<img->width;x++) {
                    farbwert=surface->RGB(b1[x*4+2], b1[x*4+1], b1[x*4+0], b1[x*4+3]);
                    //farbwert=surface->RGB(b1[x*4+0], 0, 0 );
                    surface->PutPixel (x,y,farbwert);
                }
            }
            SetError(0);
            surface->Unlock();
            return true;


	} // end switch

	surface->Unlock();
	SetError(33);
	return false;
}

int CFilter_BMP::Save (CSurface * surface, CFileObject * file, RECT *area, CAssocArray *param)
{
	char *bmia, *bmh, *img;
	ppldd pixel;
	ppldd bpp, bfOffBits;
	if (surface->GetBitdepth()>8) {
		bpp=3;
		bfOffBits=54;
	} else {
		bpp=1;
		bfOffBits=54+768;
		SetError(88);				// Paletten werden noch nicht unterstuetzt
		return false;
	}


	ppldd size=bfOffBits+(surface->GetWidth()*surface->GetHeight())*bpp;
	char *buffer=(char *)malloc(size);
	if (buffer!=NULL) {
		bmh=buffer;
		bmia=bmh+14;
		pokes(bmh,(char*)"BM");
		poked(bmh+2,size);
		pokew(bmh+6,0);
		pokew(bmh+8,0);
		poked(bmh+10,54);				// bfOffBits=Beginn der Bitmap

		poked(bmia+0,40);				// biSize
		poked(bmia+4,surface->GetWidth());	// biWidth
		poked(bmia+8,surface->GetHeight());	// biHeight
		pokew(bmia+12,1);				// biPlanes
		pokew(bmia+14,bpp*8);			// biBitCount
		poked(bmia+16,0);				// biCompression
		poked(bmia+20,0);				// biSizeImage
		poked(bmia+24,0);				// biXPelsPerMeter
		poked(bmia+28,0);				// biYPelsPerMeter
		poked(bmia+32,0);				// biClrUsed
		poked(bmia+36,0);				// biClrImportant

		img=bmh+bfOffBits;
		if (surface->GetBitdepth()==8) {		// Palette speichern
			// Hier fehlt noch der Code, um die Palette zu speichern.
			// Ausserdem muss noch geprueft werden, an welcher Position die
			// Palette im BMP-File gespeichert werden muss, und was sich
			// am Header alles aendert.
		}

		surface->Lock();

		for (int y=(surface->GetHeight()-1);y>=0;y--) {
			if (surface->GetBitdepth()>8) {
				for (int x=0;x<(surface->GetWidth());x++) {
					pixel=surface->Surface2RGB(surface->GetPixel(x,y));
					img[x*bpp]=(ppldb)((pixel>>16)&255);		// blau
					img[x*bpp+1]=(ppldb)((pixel>>8)&255);	// gruen
					img[x*bpp+2]=(ppldb)(pixel&255);			// rot
				}
			}else {
				for (int x=0;x<(surface->GetWidth());x++) {
					img[x]=(ppldb)surface->GetPixel(x,y);
				}
			}
			img+=surface->GetWidth()*bpp;

		}
		surface->Unlock();
		file->Write(buffer,size);
	}
	free (buffer);
	SetError(0);
	return true;
}

const char *CFilter_BMP::GetName()
{
	return "BMP";
}

const char *CFilter_BMP::GetDescription()
{
	return "Windows Bitmap-Dateien";
}



} // EOF namespace grafix
} // EOF namespace ppl6


