/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 *******************************************************************************
 * Copyright (c) 2022, Patrick Fedick <patrick@pfp.de>
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
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#include "ppl7.h"
#include "ppl7-grafix.h"
#include "ppl7-tk.h"



namespace ppl7 {
namespace tk {

using namespace ppl7;
using namespace ppl7::grafix;

#define DEVIDER_SIZE 6

VerticalDivider::VerticalDivider()
{
	this->setWidth(6);
}

VerticalDivider::VerticalDivider(int x, int y, int width, int height)
{
	this->setSize(6, height);
	this->setPos(x, y);
}


void VerticalDivider::paint(Drawable& draw)
{
	Color white(255, 255, 255, 255);
	Color dark(92, 92, 92, 255);
	Color bg;

	//int w=width()-1;
	int h=height() - 1;

	draw.line(2, 0, 2, h - 1, white);
	draw.line(3, 1, 3, h, dark);
}

String VerticalDivider::widgetType() const
{
	return "VerticalDivider";
}

Size VerticalDivider::contentSize() const
{
	return Size(6, height());
}

HorizontalDivider::HorizontalDivider()
{
	this->setHeight(6);
}

HorizontalDivider::HorizontalDivider(int x, int y, int width, int height)
{
	this->setSize(width, 6);
	this->setPos(x, y);
}

void HorizontalDivider::paint(Drawable& draw)
{
	Color white(255, 255, 255, 255);
	Color dark(92, 92, 92, 255);
	Color bg;

	//int w=width()-1;
	int h=height() - 1;

	draw.line(2, 0, 2, h - 1, white);
	draw.line(3, 1, 3, h, dark);
}

String HorizontalDivider::widgetType() const
{
	return "HorizontalDevider";
}

Size HorizontalDivider::contentSize() const
{
	return Size(width(), 6);
}



}	// EOF namespace tk
}	// EOF namespace ppl7
