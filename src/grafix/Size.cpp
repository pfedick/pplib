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

#include "prolog_ppl7.h"
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

/*!\class Size
 * \ingroup PPLGroupGrafik
 * \brief Repräsentiert Breite und Höhe eines beliebigen Objekts in einem zweidimensionalen Koordinatensystem
 */


Size::Size()
{
	width=0;
	height=0;
}

Size::Size(int width, int height)
{
	this->width=width;
	this->height=height;
}

Size::Size(const Size &other)
{
	width=other.width;
	height=other.height;
}


bool Size::isNull() const
{
	if (height==0 && width==0) return true;
	return false;
}

bool Size::isEmpty() const
{
	if (height==0 || width==0) return true;
	return false;
}

bool Size::isValid() const
{
	if (width<0) return false;
	if (height<0) return false;
	return true;
}

void Size::setHeight(int height)
{
	this->height=height;
}

void Size::setWidth(int width)
{
	this->width=width;
}

void Size::setSize(int width, int height)
{
	this->width=width;
	this->height=height;
}

void Size::setSize(const Size &other)
{
	width=other.width;
	height=other.height;
}


Size &Size::operator*= (double factor)
{
	width=(int)(width*factor);
	height=(int)(height*factor);
	return *this;
}

Size &Size::operator+= (const Size &size)
{
	width+=size.width;
	height+=size.height;
	return *this;
}

Size &Size::operator-= (const Size &size)
{
	width-=size.width;
	height-=size.height;
	return *this;
}

Size &Size::operator/= (double divisor)
{
	width=(int)(width/divisor);
	height=(int)(height/divisor);
	return *this;
}

bool operator!= (const Size &s1, const Size &s2)
{
	if (s1.width!=s2.width) return true;
	if (s1.height!=s2.height) return true;
	return false;
}

bool operator== (const Size & s1, const Size &s2)
{
	if (s1.width!=s2.width) return false;
	if (s1.height!=s2.height) return false;
	return true;
}

const Size operator* (const Size &size, double factor)
{
	return Size((int)(size.width*factor), (int)(size.height*factor));
}

const Size operator* (double factor, const Size &size)
{
	return Size((int)(size.width*factor), (int)(size.height*factor));
}

const Size operator+ (const Size &s1, const Size &s2)
{
	return Size(s1.width+s2.width,s1.height+s2.height);
}

const Size operator- (const Size &s1, const Size &s2)
{
	return Size(s1.width-s2.width,s1.height-s2.height);
}

const Size operator- (const Size &size)
{
	return Size(0-size.width,0-size.height);
}

const Size operator/ (const Size &size, double divisor)
{
	return Size((int)(size.width/divisor), (int)(size.height*divisor));
}




} // EOF namespace grafix
} // EOF namespace ppl7
