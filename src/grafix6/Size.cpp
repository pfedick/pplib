/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.1 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: Size.cpp,v 1.1 2010/02/12 19:43:48 pafe Exp $
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
//#ifdef HAVE_MATH_H
#include <math.h>
//#endif
#include "ppl6-grafix.h"

namespace ppl6 {
namespace grafix {

/*!\class Size
 * \ingroup PPLGroupGrafik
 * \brief Repräsentiert Breite und Höhe eines beliebigen Objekts in einem zweidimensionalen Koordinatensystem
 */


Size::Size()
{
	w=0;
	h=0;
}

Size::Size(int width, int height)
{
	w=width;
	h=height;
}

int Size::height() const
{
	return h;
}

int Size::width() const
{
	return w;
}

bool Size::isNull() const
{
	if (h==0 && w==0) return true;
	return false;
}

bool Size::isEmpty() const
{
	if (h==0 || w==0) return true;
	return false;
}

bool Size::isValid() const
{
	if (w<0) return false;
	if (h<0) return false;
	return true;
}

int &Size::rheight()
{
	return h;
}

int &Size::rwidth()
{
	return w;
}

void Size::setHeight(int height)
{
	h=height;
}

void Size::setWidth(int width)
{
	w=width;
}

void Size::setSize(int width, int height)
{
	w=width;
	h=height;
}

void Size::setSize(const Size &s)
{
	w=s.w;
	h=s.h;
}


Size &Size::operator*= (double factor)
{
	w=(int)(w*factor);
	h=(int)(h*factor);
	return *this;
}

Size &Size::operator+= (const Size &size)
{
	w+=size.w;
	h+=size.h;
	return *this;
}

Size &Size::operator-= (const Size &size)
{
	w-=size.w;
	h-=size.h;
	return *this;
}

Size &Size::operator/= (double divisor)
{
	w=(int)(w/divisor);
	h=(int)(h/divisor);
	return *this;
}

bool operator!= (const Size &s1, const Size &s2)
{
	if (s1.w!=s2.w) return true;
	if (s1.h!=s2.h) return true;
	return false;
}

bool operator== (const Size & s1, const Size &s2)
{
	if (s1.w!=s2.w) return false;
	if (s1.h!=s2.h) return false;
	return true;
}

const Size operator* (const Size &size, double factor)
{
	return Size((int)(size.w*factor), (int)(size.h*factor));
}

const Size operator* (double factor, const Size &size)
{
	return Size((int)(size.w*factor), (int)(size.h*factor));
}

const Size operator+ (const Size &s1, const Size &s2)
{
	return Size(s1.w+s2.w,s1.h+s2.h);
}

const Size operator- (const Size &s1, const Size &s2)
{
	return Size(s1.w-s2.w,s1.h-s2.h);
}

const Size operator- (const Size &size)
{
	return Size(0-size.w,0-size.h);
}

const Size operator/ (const Size &size, double divisor)
{
	return Size((int)(size.w/divisor), (int)(size.h*divisor));
}




} // EOF namespace grafix
} // EOF namespace ppl6
