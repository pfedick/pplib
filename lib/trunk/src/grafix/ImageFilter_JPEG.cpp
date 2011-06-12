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
#include "ppl7.h"
#include "ppl7-grafix.h"

#ifdef HAVE_JPEG
#undef INT32

#ifdef FAR
#undef FAR
#endif

extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}


namespace ppl7 {
namespace grafix {

/*!\class ppl6::grafix::CFilter_JPEG
 * \ingroup PPLGroupGrafik
 * \brief Import-/Export-Filter für JPEG-Dateien
 *
 * Blah
 */


#define PREREAD		512
#define READBUFFER	50000
#define OUTPUT_BUF_SIZE	8192

#ifndef JDCT_DEFAULT
typedef enum {
	JDCT_ISLOW,		/* slow but accurate integer algorithm */
	JDCT_IFAST,		/* faster, less accurate integer method */
	JDCT_FLOAT		/* floating-point: accurate, fast on fast HW */
} J_DCT_METHOD;

#define JDCT_DEFAULT  JDCT_ISLOW
#endif
#ifndef JDCT_FASTEST		/* may be overridden in jconfig.h */
#define JDCT_FASTEST  JDCT_IFAST
#endif

/*
 * JPEG Data Source Manager
 */

typedef struct {
  struct jpeg_source_mgr pub;	/* public fields */

  ImageFilter_JPEG		*img;			/* source stream */
  FileObject			*file;
  bool					start_of_file;
  size_t				pos;			// Position in der Datei
} SOURCE_MGR;

static char EOI_Marker[]={
	(ppluint8)0xFF,
	(ppluint8)JPEG_EOI
};

METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
	size_t nbytes;
	SOURCE_MGR *src=(SOURCE_MGR*)cinfo->src;
	FileObject *file=src->file;
	nbytes=PREREAD;
	src->pub.next_input_byte=(ppluint8*)file->map(0,PREREAD);
	if (!src->pub.next_input_byte) {
		ERREXIT(cinfo, JERR_INPUT_EMPTY);
		src->pub.next_input_byte=(ppluint8*)EOI_Marker;
		nbytes=2;
	}
	src->pub.bytes_in_buffer = nbytes;
	src->pos=nbytes;
	src->start_of_file = false;
}



METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
	size_t nbytes;
	SOURCE_MGR *src=(SOURCE_MGR*)cinfo->src;
	FileObject *file=src->file;
	if (file->size()<READBUFFER) {	// Datei ist kleiner als 100 KB, daher komplett in den
									// Speicher laden
		nbytes=file->size()-src->pos;
	} else {
		nbytes=file->size()-src->pos;	// Datei ist groesser, daher in 100000-Bytes-Haeppchen laden
		if (nbytes>READBUFFER) nbytes=READBUFFER;
	}

	src->pub.next_input_byte=(ppluint8*)file->map(src->pos,nbytes);
	if (!src->pub.next_input_byte) {
		ERREXIT(cinfo, JERR_INPUT_EMPTY);
		src->pub.next_input_byte=(ppluint8*)EOI_Marker;
		nbytes=2;
	}
	src->pub.bytes_in_buffer = nbytes;
	src->pos+=nbytes;
	src->start_of_file = false;
	return TRUE;
}

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
/*
 * TODO: Hier kann eine Endlosschleife auftreten!!!
 * Wenn num_bytes größer ist, als das was noch im Buffer oder der Datei übrig ist
 */
{
	SOURCE_MGR *src=(SOURCE_MGR*)cinfo->src;
	if (num_bytes > 0) {
		while (num_bytes > (long) src->pub.bytes_in_buffer) {
			num_bytes -= (long) src->pub.bytes_in_buffer;
			(void) fill_input_buffer(cinfo);
			if ((long) src->pub.bytes_in_buffer==0) return;
			/* note we assume that fill_input_buffer will never return FALSE,
			 * so suspension need not be handled.
			 */
		}
		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}

/*
static resync_to_restart (j_decompress_ptr cinfo, int desired)
{
}
*/

METHODDEF(void)
term_source (j_decompress_ptr cinfo)
{
}


static void init_sourcemgr (SOURCE_MGR* src, FileObject *file, ImageFilter_JPEG *klasse)
{

	src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data = skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->pub.term_source = term_source;
	src->file = file;
	src->img=klasse;
	src->pub.bytes_in_buffer = 0;	   /* forces fill_input_buffer on first read */
    src->pub.next_input_byte = NULL;   /* until buffer loaded */
}



/****************************************************************************************
 * JPEG Data Destination Manager
 */

typedef struct {
	struct jpeg_destination_mgr pub; /* public fields */

	FileObject	*file;
	char		*buffer;
} DEST_MGR;


METHODDEF(void)
init_destination (j_compress_ptr cinfo)
{
	DEST_MGR *dest = (DEST_MGR*) cinfo->dest;
	// Allocate the output buffer --- it will be released when done with image
	dest->buffer = (char *) (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,  OUTPUT_BUF_SIZE * sizeof(JOCTET));
	dest->file->seek(0);
	dest->pub.next_output_byte = (ppluint8*)dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}


METHODDEF(boolean)
empty_output_buffer (j_compress_ptr cinfo)
{
	DEST_MGR *dest = (DEST_MGR *) cinfo->dest;

	/*
	if (JFWRITE(dest->outfile, dest->buffer, OUTPUT_BUF_SIZE) !=
      (size_t) OUTPUT_BUF_SIZE)
	ERREXIT(cinfo, JERR_FILE_WRITE);
	*/
	if (dest->file->write(dest->buffer,OUTPUT_BUF_SIZE)!=OUTPUT_BUF_SIZE)
		ERREXIT(cinfo, JERR_FILE_WRITE);
	dest->pub.next_output_byte = (ppluint8*)dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
	return TRUE;
}



METHODDEF(void)
term_destination (j_compress_ptr cinfo)
{
	DEST_MGR *dest = (DEST_MGR *) cinfo->dest;
	size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

	/* Write any data remaining in the buffer */
	if (datacount > 0) {
		if (dest->file->write(dest->buffer,datacount)!=datacount)
			ERREXIT(cinfo, JERR_FILE_WRITE);
	}
}


ImageFilter_JPEG::ImageFilter_JPEG()
/*!\brief Konstruktor der Klasse
 */
{
}

ImageFilter_JPEG::~ImageFilter_JPEG()
{
}

int ImageFilter_JPEG::ident(FileObject &file, IMAGE &img)
{
	try {
		bool ret=false;
		const char *address=file.map(0,256);
		if (address==NULL) {
			return false;
		}

		if (Peek8(address)!=0xff  ||
				Peek8(address+1)!=0xd8 ||
				Peek8(address+2)!=0xff) {
			return false;
		}
		//if (ppl_peekd(address)!=0xe0ffd8ff || ppl_peekw(address+4)!=0x1000 ) return false;
		//if (strncmp ((char *)address+6,"JFIF",4)!=0) return false;

		struct jpeg_decompress_struct cinfo;
		struct jpeg_error_mgr jerr;
		//SOURCE_MGR* src;

		cinfo.err=jpeg_std_error(&jerr);
		jpeg_create_decompress(&cinfo);

		cinfo.src = (struct jpeg_source_mgr *) cinfo.mem->alloc_small ((j_common_ptr) &cinfo, JPOOL_PERMANENT,sizeof(SOURCE_MGR));


		init_sourcemgr((SOURCE_MGR*) cinfo.src,&file,this);


		if(jpeg_read_header(&cinfo,false)==JPEG_HEADER_OK) {

			if (cinfo.jpeg_color_space==JCS_UNKNOWN) {
				img.bitdepth=0;
				img.colors=0;
			}else if (cinfo.jpeg_color_space==JCS_GRAYSCALE) {
				img.bitdepth=8;
				img.colors=256;
				img.format=RGBFormat::GREY8;
			} else {
				img.bitdepth=24;
				img.colors=0x1000000;
				img.format=RGBFormat::X8R8G8B8;
			}

			if (img.bitdepth>0) {
				img.width=cinfo.image_width;
				img.height=cinfo.image_height;
				img.pitch=0;
				ret=true;
			}
		}
		jpeg_destroy_decompress(&cinfo);
		return ret;
	} catch (...) {
		return false;
	}
}


void ImageFilter_JPEG::load(FileObject &file, Drawable &surface, IMAGE &img)
{
	size_t buffersize;
	ppluint32 x,y,rot,gruen,blau;
	char *buffer;
	const char *address=file.map(0,256);
	if (address==NULL) throw NullPointerException();

	if (Peek8(address)!=0xff  ||
		Peek8(address+1)!=0xd8 ||
		Peek8(address+2)!=0xff) {
			throw IllegalImageFormatException();
	}

	/*
	if (ppl_peekd(address)!=0xe0ffd8ff || ppl_peekw(address+4)!=0x1000 ) {
		err=88;
		return;
	}
	if (strncmp ((char *)address+6,"JFIF",4)!=0) {
		err=88;
		return;
	}
	*/
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err=jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	cinfo.src = (struct jpeg_source_mgr *) cinfo.mem->alloc_small ((j_common_ptr) &cinfo, JPOOL_PERMANENT,sizeof(SOURCE_MGR));

	init_sourcemgr((SOURCE_MGR*) cinfo.src,&file,this);


	if(jpeg_read_header(&cinfo,false)==JPEG_HEADER_OK) {
		cinfo.out_color_components=JCS_RGB;
		jpeg_start_decompress(&cinfo);
		buffersize=cinfo.output_width * cinfo.output_components;
		buffer=(char *)malloc(buffersize);
		if (!buffer) throw OutOfMemoryException();
		y=0;
		while (cinfo.output_scanline < cinfo.output_height) {
			jpeg_read_scanlines (&cinfo,(unsigned char **)&buffer,1);
			rot=gruen=blau=0;

			if (img.bitdepth==8) {
				for (x=0;x<cinfo.output_width;x++) {
					rot=Peek8(buffer+x);
					surface.putPixel(x,y,Color(rot,rot,rot));
				}
			} else {
				for (x=0;x<cinfo.output_width;x++) {
					rot=Peek8(buffer+x*3+0);
					gruen=Peek8(buffer+x*3+1);
					blau=Peek8(buffer+x*3+2);
					surface.putPixel(x,y,Color(rot,gruen,blau));
				}
			}
			y++;
		}
		free (buffer);
		//jpeg_finish_decompress(&cinfo);
	}
	jpeg_destroy_decompress(&cinfo);
}

void ImageFilter_JPEG::save (const Drawable &surface, FileObject &file, const AssocArray &param)
{
	int x,y;
	char *buffer;

	int	quality=85;		// 0-100									default=85
	int	smooth=0;		// 0-100, 0=off, 1=minimal, 100=maximal		default=0
	bool force_baseline=true;	//									default=true
	bool optimized=false;		//									default=false
	int	dct_method=JDCT_ISLOW;	//									default=JDCT_ISLOW

	if (param.exists(L"quality")) quality=param.getString(L"quality").toInt();
	if (param.exists(L"smooth")) smooth=param.getString(L"smooth").toInt();
	if (param.exists(L"force_baseline")) force_baseline=param.getString(L"force_baseline").toBool();
	if (param.exists(L"optimized")) optimized=param.getString(L"optimized").toBool();
	if (param.exists(L"dct_method")) dct_method=param.getString(L"dct_method").toInt();

	if (quality<0) quality=0;
	if (quality>100) quality=100;
	if (smooth<0) smooth=0;
	if (smooth>100) smooth=100;

	buffer=(char *)malloc(surface.width()*3);	// Buffer fuer Scanline
	if (buffer==0) throw OutOfMemoryException();

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];

	row_pointer[0]=(JSAMPROW)buffer;

	cinfo.err=jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	DEST_MGR *dest = (DEST_MGR *) cinfo.dest;
    cinfo.dest = (struct jpeg_destination_mgr *)(*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT, sizeof(DEST_MGR));

	dest = (DEST_MGR *) cinfo.dest;
	dest->pub.init_destination = init_destination;
	dest->pub.empty_output_buffer = empty_output_buffer;
	dest->pub.term_destination = term_destination;
	dest->file=&file;

	cinfo.image_width=surface.width();
	cinfo.image_height=surface.height();
	cinfo.input_components=3;
	cinfo.in_color_space=JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo,quality,force_baseline);
	cinfo.dct_method=(J_DCT_METHOD)dct_method;
	cinfo.optimize_coding=optimized;
	cinfo.smoothing_factor=smooth;

	jpeg_start_compress(&cinfo,true);
	Color farbe;
	for (y=0;y<surface.height();y++) {
		for (x=0;x<surface.width();x++) {
			farbe=surface.getPixel(x,y);
			buffer[x*3]=(ppluint8)farbe.blue();
			buffer[x*3+1]=(ppluint8)farbe.green();
			buffer[x*3+2]=(ppluint8)farbe.red();
		}
		jpeg_write_scanlines(&cinfo,row_pointer,1);
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	free (buffer);
}

String ImageFilter_JPEG::name()
{
	return L"JPG";
}

String ImageFilter_JPEG::description()
{
	return L"JPEG";
}


} // EOF namespace grafix
} // EOF namespace ppl7
#endif // _HAVE_JPEG
