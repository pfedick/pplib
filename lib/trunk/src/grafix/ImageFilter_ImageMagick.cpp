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

#ifdef HAVE_IMAGEMAGICK
#include <wand/MagickWand.h>
#endif
#include "ppl7.h"
#include "ppl7-grafix.h"

namespace ppl7 {
namespace grafix {

/*!\class ImageFilter_ImageMagick
 * \ingroup PPLGroupGrafik
 * \brief Import-/Export-Filter für diverse Formate
 */


ImageFilter_ImageMagick::ImageFilter_ImageMagick()
{
#ifdef HAVE_IMAGEMAGICK
	MagickWandGenesis();
#endif
}

ImageFilter_ImageMagick::~ImageFilter_ImageMagick()
{
#ifdef HAVE_IMAGEMAGICK
	MagickWandTerminus();
#endif
}

int ImageFilter_ImageMagick::ident(FileObject &file, IMAGE &img)
{
#ifdef HAVE_IMAGEMAGICK
	try {
		MagickWand *wand=NewMagickWand();
		if (!wand) return 0;
		const void *blob=file.map();

		MagickBooleanType ok=MagickPingImageBlob(wand, blob, file.size());
		if (ok) {
			img.bitdepth=MagickGetImageDepth(wand);
			img.width=MagickGetImageWidth(wand);
			img.height=MagickGetImageHeight(wand);
			img.pitch=0;
			img.format=RGBFormat::unknown;

			ImageType t=MagickGetImageType(wand);
			switch (t) {
				case TrueColorType: img.format=RGBFormat::X8R8G8B8; break;
				case TrueColorMatteType: img.format=RGBFormat::A8R8G8B8; break;
				case GrayscaleType: img.format=RGBFormat::GREY8; break;
				case GrayscaleMatteType: img.format=RGBFormat::GREYALPHA32; break;
				case PaletteType: img.format=RGBFormat::Palette; break;
				case PaletteMatteType: img.format=RGBFormat::Palette; break;
				default: img.format=RGBFormat::unknown;
			}
			img.format=RGBFormat::A8R8G8B8;
			printf ("MagickImage erkannt. bitdepth=%i, width=%i, height=%i\n",img.bitdepth,
					img.width,img.height);
			printf ("Pitch: %i, format: %s\n",img.pitch,(const char*) img.format.name());
		}
		DestroyMagickWand(wand);
		if (ok) return 1;
		return 0;

	} catch (...) {
		return 0;
	}
#endif
	return 0;
}

void ImageFilter_ImageMagick::load(FileObject &file, Drawable &surface, IMAGE &img)
{
#ifdef HAVE_IMAGEMAGICK

	MagickWand *wand=NewMagickWand();
	if (!wand) throw NullPointerException();
	const void *blob=file.map();
	MagickBooleanType ok=MagickReadImageBlob(wand,blob,file.size());
	if (!ok) {
		DestroyMagickWand(wand);
		throw UnknownImageFormatException();
	}

#endif

}

void ImageFilter_ImageMagick::save (const Drawable &surface, FileObject &file, const AssocArray &param)
{

}

String ImageFilter_ImageMagick::name()
{
	return "ImageMagick";
}

String ImageFilter_ImageMagick::description()
{
	return "Import Filter with ImageMagick";
}



} // EOF namespace grafix
} // EOF namespace ppl7


