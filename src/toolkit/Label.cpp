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


Label::Label()
	:Frame()
{
	const WidgetStyle *style=GetWidgetStyle();
	setBorderStyle(NoBorder);
	myColor=style->labelFontColor;
	myFont=style->labelFont;
	setSizeStrategyWidth(Widget::MINIMUM_EXPANDING);
	setTransparent(true);

}

Label::Label(int x, int y, int width, int height, const String &text, BorderStyle style)
	:Frame(x,y,width,height)
{
	const WidgetStyle *wstyle=GetWidgetStyle();
	setBorderStyle(style);
	myColor=wstyle->labelFontColor;
	myFont=wstyle->labelFont;
	setSizeStrategyWidth(Widget::MINIMUM_EXPANDING);
	setTransparent(true);
	myText=text;
}

Label::~Label()
{

}

const String &Label::text() const
{
	return myText;
}

void Label::setText(const String &text)
{
	myText=text;
	needsRedraw();
	geometryChanged();
}

const Color &Label::color() const
{
	return myColor;
}

void Label::setColor(const Color &c)
{
	myColor=c;
	needsRedraw();
}

const Drawable &Label::icon() const
{
	return myIcon;
}

void Label::setIcon(const Drawable &icon)
{
	myIcon=icon;
	needsRedraw();
	geometryChanged();
}

const Font &Label::font() const
{
	return myFont;
}

void Label::setFont(const Font &font)
{
	myFont=font;
	needsRedraw();
	geometryChanged();
}


Size Label::contentSize() const
{
	Size s;
	s=myFont.measure(myText);
	if (myIcon.isEmpty()==false) {
		s.width+=4+myIcon.width();
		int h=2+myIcon.height();
		if (s.height<h) s.height=h;
	}
	return s;
}

String Label::widgetType() const
{
	return "Label";
}


void Label::paint(Drawable &draw)
{
	Frame::paint(draw);
	Drawable d=clientDrawable(draw);
	//printf ("Text: %s, width: %i, height: %i\n",(const char*)myText, d.width(), d.height());
	int x=0;
	if (myIcon.isEmpty()==false) {
		d.bltAlpha(myIcon,x,(d.height())/2-myIcon.height()/2);
		x+=4+myIcon.width();
	}
	myFont.setColor(myColor);
	myFont.setOrientation(Font::TOP);
	Size s=myFont.measure(myText);
	d.print(myFont,x,(d.height()-s.height)>>1,myText);
}


}	// EOF namespace tk
}	// EOF namespace ppl7
