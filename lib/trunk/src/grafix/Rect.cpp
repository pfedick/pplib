/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
 *
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
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
//#ifdef HAVE_MATH_H
#include <math.h>
//#endif
#include "ppl7-grafix.h"

namespace ppl7 {
namespace grafix {

/*!\class Rect
 * \ingroup PPLGroupGrafik
 * \brief Repräsentiert ein Rechteck in einem zweidimensionalen Koordinatensystem
 */

Rect::Rect()
{
	rx=0;
	ry=0;
	rw=0;
	rh=0;
}

Rect::Rect(const Point &topLeft, const Point &bottomRight)
{
	rx=topLeft.x();
	ry=topLeft.y();
	rw=bottomRight.x()-rx+1;
	rh=bottomRight.y()-ry+1;
}

Rect::Rect(int x, int y, int width, int height)
{
	rx=x;
	ry=y;
	rw=width;
	rh=height;
}

Rect::Rect(const RECT &r)
{
	rx=r.left;
	ry=r.top;
	rw=r.right-r.left+1;
	rh=r.bottom-r.top+1;
}


bool Rect::isNull() const
{
	if (rw==0 && rh==0) return true;
	return false;
}

int Rect::left() const
{
	return rx;
}

int Rect::right() const
{
	return rx+rw-1;
}

int Rect::top() const
{
	return ry;
}

int Rect::bottom() const
{
	return ry+rh-1;
}

int Rect::width() const
{
	return rw;
}

int Rect::height() const
{
	return rh;
}

int Rect::x() const
{
	return rx;
}

int Rect::y() const
{
	return ry;
}

int Rect::x1() const
{
	return rx;
}

int Rect::y1() const
{
	return ry;
}

int Rect::x2() const
{
	return rx+rw-1;
}

int Rect::y2() const
{
	return ry+rh-1;
}


Size Rect::size() const
{
	return Size(rw,rh);
}

Point Rect::topLeft() const
{
	return Point(rx,ry);
}

Point Rect::topRight() const
{
	return Point(rx+rw-1,ry);
}

Point Rect::bottomLeft() const
{
	return Point(rx,ry+rh-1);
}

Point Rect::bottomRight() const
{
	return Point(rx+rw-1,ry+rh-1);
}

Rect Rect::normalized () const
{
	Rect r;
	if (rw>=0) {
		r.rx=rx;
		r.rw=rw;
	} else {
		r.rw=abs(rw);
		r.rx=rx-r.rw;
	}
	if (rh>=0) {
		r.ry=ry;
		r.rh=rh;
	} else {
		r.rh=abs(rh);
		r.ry=ry-r.rh;
	}
	return r;
}

bool Rect::intersects(const Rect &other)
{
	return (other.left()< right()
			&& other.right() > left()
			&& other.top() < bottom()
			&& other.bottom() > top());

}

#ifndef max
inline static int max(int v1, int v2)
{
	if (v1>v2) return v1;
	return v2;
}
#endif

#ifndef min
inline static int min(int v1, int v2)
{
	if (v1<v2) return v1;
	return v2;
}
#endif

Rect Rect::intersected(const Rect &other)
{
	Rect r;
	if (isNull()) return r;
	if (other.isNull()) return r;
	if (!intersects(other)) return r;
	r.setLeft(max(left(),other.left()));
	r.setTop(max(top(),other.top()));
	r.setRight(min(right(),other.right()));
	r.setBottom(min(bottom(),other.bottom()));
	return r;
}

void Rect::setTopLeft(const Point &topLeft)
{
	rx=topLeft.x();
	ry=topLeft.y();
}

void Rect::setBottomRight(const Point &bottomRight)
{
	rw=bottomRight.x()-rx+1;
	rh=bottomRight.y()-ry+1;
}

void Rect::setRect(int x, int y, int width, int height)
{
	rx=x;
	ry=y;
	rw=width;
	rh=height;
}

void Rect::setRect(const RECT &r)
{
	rx=r.left;
	ry=r.top;
	rw=r.right-r.left+1;
	rh=r.bottom-r.top+1;
}

void Rect::setCoords(int x1, int y1, int x2, int y2)
{
	rx=x1;
	ry=y1;
	rw=x2-x1+1;
	rh=y2-y1+1;
}

void Rect::setLeft(int left)
{
	rx=left;
}

void Rect::setRight(int right)
{
	rw=right-rx+1;
}

void Rect::setTop(int top)
{
	ry=top;
}

void Rect::setBottom(int bottom)
{
	rh=bottom-ry+1;
}

void Rect::setX(int x)
{
	rx=x;
}

void Rect::setY(int y)
{
	ry=y;
}

void Rect::setSize(const Size &size)
{
	rw=size.width();
	rh=size.height();
}

void Rect::setWidth(int width)
{
	rw=width;
}

void Rect::setHeight(int height)
{
	rh=height;
}


bool operator!= (const Rect &r1, const Rect &r2)
{
	if (r1.rx!=r2.rx) return true;
	if (r1.ry!=r2.ry) return true;
	if (r1.rw!=r2.rw) return true;
	if (r1.rh!=r2.rh) return true;
	return false;
}

bool operator== (const Rect &r1, const Rect &r2)
{
	if (r1.rx!=r2.rx) return false;
	if (r1.ry!=r2.ry) return false;
	if (r1.rw!=r2.rw) return false;
	if (r1.rh!=r2.rh) return false;
	return true;
}


}	// EOF namespace grafix
}	// EOF namespace ppl7
