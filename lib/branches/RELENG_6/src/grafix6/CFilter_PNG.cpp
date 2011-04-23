/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.2 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: CFilter_PNG.cpp,v 1.2 2010/02/12 19:43:48 pafe Exp $
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

#ifdef HAVE_PNG
#include <png.h>


namespace ppl6 {
namespace grafix {

/*!\class CFilter_PNG
 * \ingroup PPLGroupGrafik
 * \brief Import-/Export-Filter für PNG-Dateien
 */


// PNG-Read
static void user_read_data(png_structp png_ptr,png_bytep data, png_uint_32 length)
{
	CFileObject *file = (CFileObject*)png_get_io_ptr(png_ptr);
	file->Read((char *)data,(ppldd)length);
}


// PNG-Write
static void user_write_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
	CFileObject *file = (CFileObject*)png_get_io_ptr(png_ptr);
	file->Write((char *)data,(ppldd)length);
}


static void user_flush_data(png_structp png_ptr)
{
	CFileObject *file = (CFileObject*)png_get_io_ptr(png_ptr);
	file->Flush();
}

/*
static void write_row_callback(png_structp png_ptr, png_uint_32 row, int pass)
{

}
*/


// PNG-Klasse
CFilter_PNG::CFilter_PNG()
{
	//compression_level=Z_BEST_COMPRESSION;
	//color_type=ppl6::COLORTYPE::RGB;
}

CFilter_PNG::~CFilter_PNG()
{
}

CString CFilter_PNG::Name()
{
	return "png";
}

CString CFilter_PNG::Description()
{
	return "Filter für Portable Network Graphics (PNG)";
}


int CFilter_PNG::Ident(CFileObject &file, IMAGE &img)
{
	const char *address=file.Map(0,256);
	file.Seek((ppldd)0);
	if (address==NULL) {
		SetError(1018);
		return 0;
	}

	if (png_sig_cmp((ppldb*)address, 0, 8)!=0) { // Ist es ein PNG-File?
		SetError(1018);
		return 0;
	}
	png_structp png_ptr = png_create_read_struct
       (PNG_LIBPNG_VER_STRING, NULL ,NULL, NULL);
    if (!png_ptr) {
    	SetError(1018);
    	return 0;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr,(png_infopp)NULL, (png_infopp)NULL);
        SetError(1018);
        return 0;
    }
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        SetError(1018);
        return 0;
    }
	png_set_read_fn(png_ptr,(voidp) &file, (png_rw_ptr) user_read_data);
    //png_set_write_fn(png_structp write_ptr, voidp write_io_ptr, png_rw_ptr write_data_fn,
    //    png_flush_ptr output_flush_fn);
	png_read_info(png_ptr, info_ptr);
	img.width=info_ptr->width;
    img.height=info_ptr->height;
    img.bitdepth=info_ptr->pixel_depth;			// Bits per Pixel
	img.colors=0;
	img.pitch=(info_ptr->pixel_depth/8)*info_ptr->width;	// Bits per Pixel;
	//img->pfp.header_version=0;
	bool supported=true;
	img.format=RGBFormat::unknown;
	if (info_ptr->bit_depth!=8) supported=false;		// Nur 8-Bit/Farbwert wird unterstützt

	switch (info_ptr->color_type) {
		case PNG_COLOR_TYPE_GRAY:
			img.bitdepth=8;
			img.colors=256;
			img.format=RGBFormat::GREY8;
			break;
		case PNG_COLOR_TYPE_PALETTE:
			img.bitdepth=8;
			img.colors=256;
			img.format=RGBFormat::Palette;
			//supported=false;
			break;
		case PNG_COLOR_TYPE_RGB:
			img.colors=0xffffff;
			img.bitdepth=24;
			img.format=RGBFormat::X8R8G8B8;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			img.colors=0xffffff;
			img.bitdepth=32;
			img.format=RGBFormat::A8R8G8B8;
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			img.colors=256;
			img.bitdepth=32;
			img.format=RGBFormat::GREYALPHA32;
	};

	if (info_ptr->interlace_type!=PNG_INTERLACE_NONE) {	// Interlaced wird nicht unterstützt
		supported=false;
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	if (!supported) {
		SetError(1018);
		return 0;
	}
	return 1;
}

int CFilter_PNG::Load(CFileObject &file, CDrawable &surface, IMAGE &img)
{
	int x,y,bpp;
	int r,g,b,a;
	file.Seek((ppldd)0);
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL ,NULL, NULL);
    if (!png_ptr) return false;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)  {
        png_destroy_read_struct(&png_ptr,(png_infopp)NULL, (png_infopp)NULL);
        return false;
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        return false;
    }

	png_set_read_fn(png_ptr,(voidp) &file, (png_rw_ptr) user_read_data);

    //png_set_write_fn(png_structp write_ptr, voidp write_io_ptr, png_rw_ptr write_data_fn,
    //    png_flush_ptr output_flush_fn);

	png_read_info(png_ptr, info_ptr);

	//png_read_png(png_ptr, info_ptr,0,NULL);



	png_bytep row_pointer=(png_bytep) png_malloc(png_ptr,img.pitch);

	RGBFormat zformat=surface.rgbformat();		// Zielformat holen

	if (row_pointer!=NULL) {
		switch (info_ptr->color_type) {

		case PNG_COLOR_TYPE_RGB_ALPHA:
			bpp=info_ptr->pixel_depth/8;
			for (y=0;y<img.height;y++) {
				png_read_row(png_ptr, row_pointer, NULL);
				for (x=0;x<img.width;x++) {
					r=row_pointer[x*bpp+0];
					g=row_pointer[x*bpp+1];
					b=row_pointer[x*bpp+2];
					a=row_pointer[x*bpp+3];
					surface.putPixel(x,y,Color(r,g,b,a));
				}
			}
			break;
		case PNG_COLOR_TYPE_RGB:
			bpp=info_ptr->pixel_depth/8;
			for (y=0;y<img.height;y++) {
				png_read_row(png_ptr, row_pointer, NULL);
				for (x=0;x<img.width;x++) {
					r=row_pointer[x*bpp+0];
					g=row_pointer[x*bpp+1];
					b=row_pointer[x*bpp+2];
					surface.putPixel(x,y,Color(r,g,b,255));
				}
			}
			break;
		case PNG_COLOR_TYPE_GRAY:
			bpp=info_ptr->pixel_depth/8;
			// Ist das Zielformat auch Greyscale?
			if (zformat==RGBFormat::A8 || zformat==RGBFormat::GREY8) {
				for (y=0;y<img.height;y++) {
					png_read_row(png_ptr, row_pointer, NULL);
					for (x=0;x<img.width;x++) {
						r=row_pointer[x*bpp+0];
						surface.putPixel(x,y,Color(r,r,r));
					}
				}
			} else {
				for (y=0;y<img.height;y++) {
					png_read_row(png_ptr, row_pointer, NULL);
					for (x=0;x<img.width;x++) {
						r=row_pointer[x*bpp+0];
						surface.putPixel(x,y,Color(r,r,r));
					}
				}
			}
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			bpp=info_ptr->pixel_depth/8;
			for (y=0;y<img.height;y++) {
				png_read_row(png_ptr, row_pointer, NULL);
				for (x=0;x<img.width;x++) {
					r=row_pointer[x*bpp+0];
					a=row_pointer[x*bpp+1];
					surface.putPixel(x,y,Color(r,r,r,a));
				}
			}
			break;
		case PNG_COLOR_TYPE_PALETTE:
			int trans=info_ptr->num_trans-1;
			bpp=info_ptr->pixel_depth/8;
			png_colorp pal=info_ptr->palette;
			if (info_ptr->num_trans>0) {
				r=pal[trans].red;
				g=pal[trans].green;
				b=pal[trans].blue;
				//surface->SetColorKey(surface->RGB(r,g,b,0));
			}
			for (y=0;y<img.height;y++) {
				png_read_row(png_ptr, row_pointer, NULL);
				for (x=0;x<img.width;x++) {
					a=row_pointer[x*bpp+0];
					r=pal[a].red;
					g=pal[a].green;
					b=pal[a].blue;
					if (info_ptr->num_trans>0) {
						if (a==trans) a=0; else a=255;
					} else a=255;
					surface.putPixel(x,y,Color(r,g,b,a));
				}
			}

			break;

		}
		SetError(0);
	} else {
		SetError(145);
	}

	png_free(png_ptr,row_pointer);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	if (GetErrorCode()==0) return true;
	return false;
}

int CFilter_PNG::Save (const CDrawable &surface, CFileObject &file, CAssocArray *param)
{
	Color farbe;
	int pitch,bpp,colortype;
	//int r,g,b,a;
	int x,y,width,height;
	ppldb *buffer;
	png_color	pc[256];
	//RGBA	rgb;

	if (surface.isEmpty()) {
		SetError(1069);
		return 0;
	}

	file.Seek((ppldd)0);
	width=surface.width();
	height=surface.height();

	pitch=colortype=0;
	int png_color_type=COLORTYPE::RGB;
	int compression_level=Z_BEST_COMPRESSION;
	RGBFormat srgb=surface.rgbformat();
	if (srgb==RGBFormat::A8R8G8B8) png_color_type=COLORTYPE::RGB_ALPHA;

	if (param) {
		const char *tmp;
		if ((tmp=param->Get("colortype"))) png_color_type=ppl6::atoi(tmp);
		if (!png_color_type) png_color_type=COLORTYPE::RGB;
	}

	switch (png_color_type) {
		case COLORTYPE::GRAY:
			colortype=PNG_COLOR_TYPE_GRAY;
			pitch=width*1;
			break;
		case COLORTYPE::PALETTE:
			colortype=PNG_COLOR_TYPE_PALETTE;
			pitch=width*1;
			break;
		case COLORTYPE::RGB:
			colortype=PNG_COLOR_TYPE_RGB;
			pitch=width*3;
			break;
		case COLORTYPE::RGB_ALPHA:
			colortype=PNG_COLOR_TYPE_RGB_ALPHA;
			pitch=width*4;
			break;
		case COLORTYPE::GRAY_ALPHA:
			colortype=PNG_COLOR_TYPE_GRAY_ALPHA;
			pitch=width*2;
		default:
			colortype=PNG_COLOR_TYPE_RGB;
			png_color_type=COLORTYPE::RGB;
			pitch=width*3;
	};


	png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL,NULL,NULL);
	if (!png_ptr) {
		SetError(184);
		return false;
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
		SetError(184);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr,&info_ptr);
		SetError(185);
		return false;
	}

	//png_set_read_fn(png_ptr,(voidp) file, (png_rw_ptr) user_read_data);
	png_set_write_fn(png_ptr,(voidp) &file, (png_rw_ptr) user_write_data, (png_flush_ptr) user_flush_data);

	// Compression-Level setzen

	png_set_compression_level (png_ptr, compression_level);




	png_set_IHDR(png_ptr, info_ptr, width,height,8,colortype,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);




	buffer=(ppldb *)png_malloc(png_ptr,pitch);
	if (buffer!=NULL) {
		// png_write_row(png_ptr, row_pointer);

		switch (colortype) {
			case PNG_COLOR_TYPE_PALETTE:
				bpp=1;
				if (surface.rgbformat()==RGBFormat::Palette) {	// Surface verwendet Palette
					for (int i=0;i<256;i++) {
						// TODO:
						//surface->GetColor(i,&rgb);
						//pc[i].red=rgb.red;
						//pc[i].green=rgb.green;
						//pc[i].blue=rgb.blue;
					}
					png_set_PLTE(png_ptr,info_ptr, &pc[0], 256);
					png_write_info(png_ptr, info_ptr);
					//png_write_PLTE (png_ptr, &pc[0],256);

					for (y=0;y<height;y++) {
						for (x=0;x<width;x++) {
							farbe=surface.getPixel(x,y);
							buffer[x]=(ppldb)(farbe.color()&0xff);
						}
						png_write_row(png_ptr, buffer);
					}
				} else {								// Surface verwendet keine Palette -> Konvertierung
					/* TODO:
					RGBA *pal=Get8BitTrueColorPalette ();
					for (int i=0;i<256;i++) {
						pc[i].red=pal[i].c.red;
						pc[i].green=pal[i].c.green;
						pc[i].blue=pal[i].c.blue;
						DLOG ("Farbe %i: RGB=%u, %u, %u",i,pc[i].red,pc[i].green,pc[i].blue);
					}
					png_set_PLTE(png_ptr,info_ptr, &pc[0], 256);
					png_write_info(png_ptr, info_ptr);
					//png_write_PLTE (png_ptr, &pc[0],256);

					for (y=area->top;y<area->bottom;y++) {
						DLOGLEVEL(2) ("png_write_row Zeile %u",y);
						for (x=0;x<width;x++) {
							farbe=surface->Surface2RGB(surface->GetPixel(area->left+x,y));
							r=(farbe&255)*7/255;
							g=((farbe>>8)&255)*7/255;
							b=((farbe>>16)&255)*3/255;
							buffer[x]=(ppldb)(r+(g<<3)+(b<<6));
						}
						png_write_row(png_ptr, buffer);
					}
					*/

				}
				break;
			case PNG_COLOR_TYPE_RGB:
				bpp=3;
				png_write_info(png_ptr, info_ptr);
				for (y=0;y<height;y++) {
					for (x=0;x<width;x++) {
						//farbe=surface->GetPixel(area->left+x,y);
						farbe=surface.getPixel(x,y);
						buffer[x*bpp+0]=(ppldb)farbe.red();
						buffer[x*bpp+1]=(ppldb)farbe.green();
						buffer[x*bpp+2]=(ppldb)farbe.blue();
					}
					png_write_row(png_ptr, buffer);
				}
				break;
			case PNG_COLOR_TYPE_RGB_ALPHA:
				bpp=4;
				png_write_info(png_ptr, info_ptr);
				for (y=0;y<height;y++) {
					for (x=0;x<width;x++) {
						farbe=surface.getPixel(x,y);
						buffer[x*bpp+0]=(ppldb)farbe.red();
						buffer[x*bpp+1]=(ppldb)farbe.green();
						buffer[x*bpp+2]=(ppldb)farbe.blue();
						buffer[x*bpp+3]=(ppldb)farbe.alpha();
					}
					png_write_row(png_ptr, buffer);
				}
				break;
			case PNG_COLOR_TYPE_GRAY:
				bpp=1;
				png_write_info(png_ptr, info_ptr);
				for (y=0;y<height;y++) {
					for (x=0;x<width;x++) {
						farbe=surface.getPixel(x,y);
						buffer[x*bpp]=(ppldb)farbe.brightness();
					}
					png_write_row(png_ptr, buffer);
				}

				break;
			case PNG_COLOR_TYPE_GRAY_ALPHA:
				bpp=2;
				png_write_info(png_ptr, info_ptr);
				for (y=0;y<height;y++) {
					for (x=0;x<width;x++) {
						farbe=surface.getPixel(x,y);
						buffer[x*bpp]=(ppldb)farbe.brightness();
						buffer[x*bpp+1]=(ppldb)farbe.alpha();
					}
					png_write_row(png_ptr, buffer);
				}
				break;
		}
		png_free(png_ptr,buffer);
		SetError(0);
	} else {
		SetError(145);
	}

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr,&info_ptr);
	if (GetErrorCode()==0) return true;
	return false;
}

} // EOF namespace grafix
} // EOF namespace ppl6


#endif // HAVE_PNG

