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
 *
 * \desc
 * By convention, the right and bottom edges of the rectangle are normally considered exclusive.
 * In other words, the pixel whose coordinates are ( right, bottom ) lies immediately outside of
 * the rectangle. For example, when RECT is passed to the FillRect function, the rectangle is filled
 * up to, but not including, the right column and bottom row of pixels.
 *
 */

Rect::Rect()
{
	x1=0;
	y1=0;
	x2=0;
	y2=0;
}

Rect::Rect(const Point &p1, const Point &p2)
{
	x1=p1.x;
	y1=p1.y;
	x2=p2.x;
	y2=p2.y;
}

Rect::Rect(const Rect &other)
{
	x1=other.x1;
	y1=other.y1;
	x2=other.x2;
	y2=other.y2;
}

Rect::Rect(int x, int y, int width, int height)
{
	x1=x;
	y1=y;
	x2=x+width;
	y2=y+height;
}


bool Rect::isNull() const
{
	if (x1==x2 && y1==y2) return true;
	return false;
}

int Rect::left() const
{
	return x1;
}

int Rect::right() const
{
	return x2-1;
}

int Rect::top() const
{
	return y1;
}

int Rect::bottom() const
{
	return y2-1;
}

int Rect::width() const
{
	return x2-x1;
}

int Rect::height() const
{
	return y2-y1;
}

Size Rect::size() const
{
	return Size(x2-x1,y2-y1);
}

Point Rect::topLeft() const
{
	return Point(x1,y1);
}

Point Rect::topRight() const
{
	return Point(x2,y1);
}

Point Rect::bottomLeft() const
{
	return Point(x1,y2);
}

Point Rect::bottomRight() const
{
	return Point(x2,y2);
}

Rect Rect::normalized () const
{
	Rect r;
	if (x2>=x1) {
		r.x1=x1;
		r.x2=x2;
	} else {
		r.x1=x2;
		r.x2=x1;
	}
	if (y2>=y1) {
		r.y1=y1;
		r.y2=y2;
	} else {
		r.y1=y2;
		r.y2=y1;
	}
	return r;
}

bool Rect::intersects(const Rect &other)
{
	return (other.x1< x2
			&& other.x2 > x1
			&& other.y1 < y2
			&& other.y2 > y1);

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
	r.x1=max(x1,other.x1);
	r.y1=max(y1,other.y1);
	r.x2=min(x2,other.x2);
	r.y2=min(y2,other.y2);
	return r;
}

void Rect::setTopLeft(const Point &topLeft)
{
	x1=topLeft.x;
	y1=topLeft.y;
}

void Rect::setBottomRight(const Point &bottomRight)
{
	x2=bottomRight.x;
	y2=bottomRight.y;
}

void Rect::setRect(int x, int y, int width, int height)
{
	x1=x;
	y1=y;
	x2=x+width;
	y2=y+height;
}

void Rect::setRect(const RECT &r)
{
	x1=r.left;
	y1=r.top;
	x2=r.right;
	y2=r.bottom;
}


void Rect::setRect(const Rect &other)
{
	x1=other.x1;
	y1=other.y1;
	x2=other.x2;
	y2=other.y2;
}


void Rect::setCoords(int x1, int y1, int x2, int y2)
{
	this->x1=x1;
	this->y1=y1;
	this->x2=x2;
	this->y2=y2;
}

void Rect::setCoords(const Point &p1, const Point &p2)
{
	x1=p1.x;
	y1=p1.y;
	x2=p2.x;
	y2=p2.y;
}

void Rect::setLeft(int left)
{
	x1=left;
}

void Rect::setRight(int right)
{
	x2=right;
}

void Rect::setTop(int top)
{
	y1=top;
}

void Rect::setBottom(int bottom)
{
	y2=bottom;
}

void Rect::setX(int x)
{
	x1=x;
}

void Rect::setY(int y)
{
	y1=y;
}

void Rect::setSize(const Size &size)
{
	x2=x1+size.width;
	y2=y1+size.height;
}

void Rect::setWidth(int width)
{
	x2=x1+width;
}

void Rect::setHeight(int height)
{
	y2=y1+height;
}


bool operator!= (const Rect &r1, const Rect &r2)
{
	if (r1.x1!=r2.x1) return true;
	if (r1.y1!=r2.y1) return true;
	if (r1.x2!=r2.x2) return true;
	if (r1.y2!=r2.y2) return true;
	return false;
}

bool operator== (const Rect &r1, const Rect &r2)
{
	if (r1.x1!=r2.x1) return false;
	if (r1.y1!=r2.y1) return false;
	if (r1.x2!=r2.x2) return false;
	if (r1.y2!=r2.y2) return false;
	return true;
}


}	// EOF namespace grafix
}	// EOF namespace ppl7
