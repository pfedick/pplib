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

Frame::Frame()
{
	const WidgetStyle *style=GetWidgetStyle();
	myBorderStyle=Upset;
	myBackground=style->frameBackgroundColor;
	myBorderColor=style->frameBorderColor;
	setTransparent(false);
	setClientOffset(3,3,3,3);
}

Frame::Frame(int x, int y, int width, int height, BorderStyle style)
{
	const WidgetStyle *wstyle=GetWidgetStyle();
	myBorderStyle=style;
	myBackground=wstyle->frameBackgroundColor;
	myBorderColor=wstyle->frameBorderColor;
	create(x,y,width,height);
	setTransparent(false);
	setClientOffset(3,3,3,3);
}

Frame::~Frame()
{

}

Frame::BorderStyle Frame::borderStyle() const
{
	return (BorderStyle)myBorderStyle;
}

void Frame::setBorderStyle(BorderStyle s)
{
	myBorderStyle=s;
	needsRedraw();
}

const Color &Frame::backgroundColor() const
{
	return myBackground;
}

void Frame::setBackgroundColor(const Color &c)
{
	myBackground=c;
	needsRedraw();
}

const Color &Frame::borderColor() const
{
	return myBorderColor;
}

void Frame::setBorderColor(const Color &c)
{
	myBorderColor=c;
	needsRedraw();
}

void Frame::paint(Drawable &draw)
{
	Color white(255,255,255,255);
	Color bg;

	int w=width()-1;
	int h=height()-1;
	bool myTransparent=isTransparent();
	switch(myBorderStyle) {
		case NoBorder:
			if (!myTransparent) draw.cls(myBackground);
			break;
		case Normal:
			if (!myTransparent) draw.cls(myBackground);
			draw.drawRect(0,0,w,h,myBorderColor);
			break;
		case Upset:
			if (!myTransparent) {
				bg=myBackground;
				draw.cls(bg);
			}
			draw.line(0,0,w,0,white);
			draw.line(0,0,0,h,white);
			draw.line(0,h,w,h,myBorderColor);
			draw.line(w,0,w,h,myBorderColor);
			break;
		case Inset:
			if (!myTransparent) {
				bg=myBackground;
				draw.cls(bg);
			}
			draw.line(0,0,w,0,myBorderColor);
			draw.line(0,0,0,h,myBorderColor);
			draw.line(0,h,w,h,white);
			draw.line(w,0,w,h,white);
			break;
	}
}

String Frame::widgetType() const
{
	return "Frame";
}



}	// EOF namespace tk
}	// EOF namespace ppl7

