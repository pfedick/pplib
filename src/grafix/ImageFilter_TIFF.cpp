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

#include "../../include/prolog_ppl7.h"
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_TIFF
#include <tiffio.h>
#endif

#include "ppl7.h"
#include "ppl7-grafix.h"

namespace ppl7 {
namespace grafix {

/*!\class ImageFilter_TIFF
 * \ingroup PPLGroupGrafik
 * \brief Import-Filter für TIFF-Dateien
 */

#ifdef HAVE_TIFF
static tsize_t readproc(thandle_t handle, tdata_t data, tsize_t size)
{
	FileObject *ff=static_cast<FileObject*>(handle);
	return ff->read(data,size);
}

static tsize_t writeproc(thandle_t handle, tdata_t data, tsize_t size)
{
	FileObject *ff=static_cast<FileObject*>(handle);
	return ff->write(data,size);
}

static toff_t seekproc(thandle_t handle, toff_t offset, int whence)
{
	FileObject *ff=static_cast<FileObject*>(handle);
	/*
        If whence is SEEK_SET, the offset is set to offset bytes.

           If whence is SEEK_CUR, the offset is set to its current location
           plus offset bytes.

           If whence is SEEK_END, the offset is set to the size of the file
           plus offset bytes.

           If whence is SEEK_HOLE, the offset of the start of the next hole
           greater than or equal to the supplied offset is returned.  The def‐
           inition of a hole is provided below.

           If whence is SEEK_DATA, the offset is set to the start of the next
           non-hole file region greater than or equal to the supplied offset.

	 */
	switch (whence) {
		case SEEK_SET: return ff->seek(offset,File::SEEKSET);
		case SEEK_CUR: return ff->seek(offset,File::SEEKCUR);
		case SEEK_END: return ff->seek(offset,File::SEEKEND);

	}
	return 0;
}

static int closeproc(thandle_t handle)
{
	//FileObject *ff=(FileObject*) handle;
	//ff->close();
	return 0;
}

static toff_t sizeproc(thandle_t handle)
{
	FileObject *ff=static_cast<FileObject*>(handle);
	return ff->size();
}

#endif

ImageFilter_TIFF::ImageFilter_TIFF()
{
#ifdef HAVE_TIFF
	TIFFSetWarningHandler(NULL);
	TIFFSetErrorHandler(NULL);
#endif
}

ImageFilter_TIFF::~ImageFilter_TIFF()
{
}

int ImageFilter_TIFF::ident(FileObject &file, IMAGE &img)
{
#ifdef HAVE_TIFF
	try {
		file.seek(0);
		TIFF* tif=TIFFClientOpen("no name", "r", &file,
		    readproc, writeproc, seekproc,
		    closeproc, sizeproc, NULL, NULL);
		if (tif==NULL) return 0;
		uint32 w, h, depth;
		uint16 bits;
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits);
		TIFFGetField(tif, TIFFTAG_IMAGEDEPTH, &depth);

		//printf ("w=%i, h=%i, bits=%i, depth=%i\n",w,h,(int)bits,depth);

		img.width=w;
		img.height=h;
		img.bitdepth=32;
		img.pitch=img.width*img.bitdepth/8;
		img.format=RGBFormat::X8R8G8B8;


		TIFFClose(tif);
		return 1;
	} catch (...) {
		return 0;
	}
	return 0;
#else
	return 0;
#endif
}

void ImageFilter_TIFF::load(FileObject &file, Drawable &surface, IMAGE &img)
{
#ifdef HAVE_TIFF
	file.seek(0);
	TIFF* tif=TIFFClientOpen("no name", "r", &file,
	    readproc, writeproc, seekproc,
	    closeproc, sizeproc, NULL, NULL);
	if (tif==NULL) throw UnknownImageFormatException();
	uint32 w, h;
	uint16 o;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	TIFFGetField(tif, TIFFTAG_ORIENTATION, &o);

	//printf ("Orientation: %i\n",o);

	size_t npixels = w * h;
	uint32* raster = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
	if (raster == NULL) {
		TIFFClose(tif);
		throw OutOfMemoryException();
	}
	if (TIFFReadRGBAImage(tif, w, h, raster, 0)) {
		ppluint32 p=0, abgr;
		Color c;
		for (uint32 y=0;y<h;y++) {
			for (uint32 x=0;x<w;x++) {
				abgr=raster[p++];
				c.setColor(abgr&255,(abgr>>8)&255,(abgr>>16)&255,(abgr>>24)&255);
				surface.putPixel(x,h-y-1,c);
			}
		}
		_TIFFfree(raster);
		TIFFClose(tif);
		return;
	}
	_TIFFfree(raster);
	TIFFClose(tif);
	throw UnknownImageFormatException();
#else
	throw UnsupportedFeatureException("ImageFilter_TIFF");
#endif
}

void ImageFilter_TIFF::save (const Drawable &surface, FileObject &file, const AssocArray &param)
{
#ifdef HAVE_TIFF
	throw UnsupportedFeatureException("ImageFilter_TIFF::save");
#else
	throw UnsupportedFeatureException("ImageFilter_TIFF");
#endif
}

String ImageFilter_TIFF::name()
{
	return "TIFF";
}

String ImageFilter_TIFF::description()
{
	return "TIFF (Loader only)";
}


} // EOF namespace grafix
} // EOF namespace ppl7


