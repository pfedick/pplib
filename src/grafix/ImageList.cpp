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
//#include "grafix6.h"


namespace ppl7 {
namespace grafix {

/*!\class ImageList
 * \ingroup PPLGroupGrafik
 * \brief Ein Container für eine Grafik mit mehreren gleichgroßen Icons
 * @return
 */

ImageList::ImageList()
{
	method=BLT;
	width=height=numIcons=0;
	numX=numY=0;
}

ImageList::ImageList(const ImageList &other)
{
	copy(other);
}

ImageList::ImageList(const Drawable &draw, int icon_width, int icon_height, DRAWMETHOD method)
{
	load(draw,icon_width,icon_height,method);
}

ImageList::ImageList(const String &Filename, int icon_width, int icon_height, DRAWMETHOD method)
{
	load(Filename,icon_width,icon_height,method);
}

ImageList::ImageList(FileObject &file, int icon_width, int icon_height, DRAWMETHOD method)
{
	load(file,icon_width,icon_height,method);
}

ImageList::ImageList(const ByteArrayPtr &mem, int icon_width, int icon_height, DRAWMETHOD method)
{
	load(mem,icon_width,icon_height,method);
}

ImageList::~ImageList()
{

}

void ImageList::clear()
{
	method=BLT;
	width=height=numIcons=0;
	numX=numY=0;
}

void ImageList::copy(const ImageList &other)
{
	Image::copy(other);
	width=other.width;
	height=other.height;
	numIcons=other.numIcons;
	colorkey=other.colorkey;
	diffuse=other.diffuse;
	method=other.method;
	numX=other.numX;
	numY=other.numY;
}

void ImageList::load(const Drawable &draw,int icon_width, int icon_height, DRAWMETHOD method)
{
	Image::copy(draw);
	width=icon_width;
	height=icon_height;
	this->method=method;
	numX=Image::width()/width;
	numY=Image::height()/height;
	numIcons=numX*numY;
}

void ImageList::load(const String &Filename, int icon_width, int icon_height, DRAWMETHOD method)
{
	Image::load(Filename);
	width=icon_width;
	height=icon_height;
	this->method=method;
	numX=Image::width()/width;
	numY=Image::height()/height;
	numIcons=numX*numY;
}

void ImageList::load(FileObject &file, int icon_width, int icon_height, DRAWMETHOD method)
{
	Image::load(file);
	width=icon_width;
	height=icon_height;
	this->method=method;
	numX=Image::width()/width;
	numY=Image::height()/height;
	numIcons=numX*numY;
}

void ImageList::load(const ByteArrayPtr &mem, int icon_width, int icon_height, DRAWMETHOD method)
{
	Image::load(mem);
	width=icon_width;
	height=icon_height;
	this->method=method;
	numX=Image::width()/width;
	numY=Image::height()/height;
	numIcons=numX*numY;
}


void ImageList::setDrawMethod(DRAWMETHOD method)
{
	this->method=method;
}

void ImageList::setColorKey(const Color &key)
{
	colorkey=key;
}

void ImageList::setDiffuseColor(const Color &c)
{
	diffuse=c;
}

void ImageList::setIconSize(int width, int height)
{
	this->width=width;
	this->height=height;
}


size_t ImageList::num() const
{
	return numIcons;
}

Size ImageList::iconSize() const
{
	return Size(width,height);
}

Rect ImageList::getRect(size_t nr) const
{
	Rect r;
	if (numIcons==0 || nr>=numIcons) throw OutOfBoundsEception();
	int h,w;
	h=nr/numX;
	w=nr%numX;
	r.setRect(w*width,h*height,width,height);
	return r;
}

ImageList::DRAWMETHOD ImageList::drawMethod() const
{
	return method;
}

Drawable ImageList::getDrawable(size_t nr) const
{
	Rect r=getRect(nr);
	return Drawable(*this,r);
}

Color ImageList::colorKey() const
{
	return colorkey;
}

Color ImageList::diffuseColor() const
{
	return diffuse;
}

ImageList &ImageList::operator=(const ImageList &other)
{
	copy(other);
	return *this;
}




} // EOF namespace grafix
} // EOF namespace ppl7
