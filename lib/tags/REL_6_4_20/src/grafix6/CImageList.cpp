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

#include "ppl6-grafix.h"
#include "grafix6.h"


namespace ppl6 {
namespace grafix {

/*!\class CImageList
 * \ingroup PPLGroupGrafik
 * \brief Ein Container für eine Grafik mit mehreren gleichgroßen Icons
 * @return
 */

CImageList::CImageList()
{
	method=BLT;
	width=height=numIcons=0;
	numX=numY=0;
}

CImageList::CImageList(const CImageList &other)
{
	if (!copy(other)) throw Exception();
}

CImageList::CImageList(const CDrawable &draw, int icon_width, int icon_height, DRAWMETHOD method)
{
	if (!load(draw,icon_width,icon_height,method)) throw Exception();
}

CImageList::CImageList(const CString &Filename, int icon_width, int icon_height, DRAWMETHOD method)
{
	if (!load(Filename,icon_width,icon_height,method)) throw Exception();
}

CImageList::CImageList(CFileObject &file, int icon_width, int icon_height, DRAWMETHOD method)
{
	if (!load(file,icon_width,icon_height,method)) throw Exception();
}

CImageList::CImageList(const CMemoryReference &mem, int icon_width, int icon_height, DRAWMETHOD method)
{
	if (!load(mem,icon_width,icon_height,method)) throw Exception();
}

CImageList::~CImageList()
{

}

void CImageList::clear()
{
	method=BLT;
	width=height=numIcons=0;
	numX=numY=0;
}

int CImageList::copy(const CImageList &other)
{
	if (!CImage::copy(other)) return 0;
	width=other.width;
	height=other.height;
	numIcons=other.numIcons;
	colorkey=other.colorkey;
	diffuse=other.diffuse;
	method=other.method;
	numX=other.numX;
	numY=other.numY;
	return 1;
}

int CImageList::load(const CDrawable &draw,int icon_width, int icon_height, DRAWMETHOD method)
{
	if (!CImage::copy(draw)) return 0;
	width=icon_width;
	height=icon_height;
	this->method=method;
	numX=CImage::width()/width;
	numY=CImage::height()/height;
	numIcons=numX*numY;
	return 1;
}

int CImageList::load(const CString &Filename, int icon_width, int icon_height, DRAWMETHOD method)
{
	if (!CImage::load(Filename)) return 0;
	width=icon_width;
	height=icon_height;
	this->method=method;
	numX=CImage::width()/width;
	numY=CImage::height()/height;
	numIcons=numX*numY;
	return 1;
}

int CImageList::load(CFileObject &file, int icon_width, int icon_height, DRAWMETHOD method)
{
	if (!CImage::load(file)) return 0;
	width=icon_width;
	height=icon_height;
	this->method=method;
	numX=CImage::width()/width;
	numY=CImage::height()/height;
	numIcons=numX*numY;
	return 1;
}

int CImageList::load(const CMemoryReference &mem, int icon_width, int icon_height, DRAWMETHOD method)
{
	if (!CImage::load(mem)) return 0;
	width=icon_width;
	height=icon_height;
	this->method=method;
	numX=CImage::width()/width;
	numY=CImage::height()/height;
	numIcons=numX*numY;
	return 1;
}


void CImageList::setDrawMethod(DRAWMETHOD method)
{
	this->method=method;
}

void CImageList::setColorKey(const Color &key)
{
	colorkey=key;
}

void CImageList::setDiffuseColor(const Color &c)
{
	diffuse=c;
}

void CImageList::setIconSize(int width, int height)
{
	this->width=width;
	this->height=height;
}


int CImageList::num() const
{
	return numIcons;
}

Size CImageList::iconSize() const
{
	return Size(width,height);
}

Rect CImageList::getRect(int nr) const
{
	Rect r;
	if (!numIcons) {
		SetError(1043);
		return r;
	}
	if (nr>=numIcons) {
		SetError(1044,"Nummer: %i, Liste: 0-%i",nr,numIcons-1);
		return r;
	}
	int h,w;
	h=nr/numX;
	w=nr%numX;
	r.setRect(w*width,h*height,width,height);
	return r;
}

CImageList::DRAWMETHOD CImageList::drawMethod() const
{
	return method;
}

CDrawable CImageList::getDrawable(int nr) const
{
	Rect r=getRect(nr);
	if (r.isNull()) return CDrawable();
	return CDrawable(*this,r);
}

Color CImageList::colorKey() const
{
	return colorkey;
}

Color CImageList::diffuseColor() const
{
	return diffuse;
}

CImageList &CImageList::operator=(const CImageList &other)
{
	if (!copy(other)) throw Exception();
	return *this;
}




} // EOF namespace grafix
} // EOF namespace ppl6
