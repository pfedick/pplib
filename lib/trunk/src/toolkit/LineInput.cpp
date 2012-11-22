/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 505 $
 * $Date: 2012-11-17 15:16:53 +0100 (Sa, 17 Nov 2012) $
 * $Id: LineInput.cpp 505 2012-11-17 14:16:53Z pafe $
 * $URL: https://pplib.svn.sourceforge.net/svnroot/pplib/lib/trunk/src/toolkit/LineInput.cpp $
 *
 *******************************************************************************
 * Copyright (c) 2012, Patrick Fedick <patrick@pfp.de>
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


LineInput::LineInput()
	:Frame()
{
	const WidgetStyle *style=GetWidgetStyle();
	setBorderStyle(Inset);
	myColor=style->inputFontColor;
	myFont=style->inputFont;
	setBackgroundColor(style->inputBackgroundColor);
	setSizeStrategyWidth(Widget::MINIMUM_EXPANDING);
	setTransparent(false);

}

LineInput::LineInput(int x, int y, int width, int height, const String &text)
	:Frame(x,y,width,height)
{
	const WidgetStyle *style=GetWidgetStyle();
	setBorderStyle(Inset);
	myColor=style->inputFontColor;
	myFont=style->inputFont;
	setBackgroundColor(style->inputBackgroundColor);
	setSizeStrategyWidth(Widget::MINIMUM_EXPANDING);
	setTransparent(false);
	myText=text;
}

LineInput::~LineInput()
{

}

const String &LineInput::text() const
{
	return myText;
}

void LineInput::setText(const String &text)
{
	myText=text;
	needsRedraw();
	geometryChanged();
}

const Color &LineInput::color() const
{
	return myColor;
}

void LineInput::setColor(const Color &c)
{
	myColor=c;
	needsRedraw();
}

const Font &LineInput::font() const
{
	return myFont;
}

void LineInput::setFont(const Font &font)
{
	myFont=font;
	needsRedraw();
	geometryChanged();
}


Size LineInput::contentSize() const
{
	return myFont.measure(myText);
}

String LineInput::widgetType() const
{
	return "LineInput";
}


void LineInput::paint(Drawable &draw)
{
	Frame::paint(draw);
	Drawable d=clientDrawable(draw);
	//printf ("Text: %s, width: %i, height: %i\n",(const char*)myText, d.width(), d.height());
	int x=0;
	myFont.setColor(myColor);
	myFont.setOrientation(Font::TOP);
	Size s=myFont.measure(myText);
	d.print(myFont,x,(draw.height()-s.height)>>1,myText);
}

void LineInput::mouseDownEvent(MouseEvent *event)
{
	printf ("LineInput::mouseDownEvent\n");
	GetWindowManager()->setKeyboardFocus(this);
}

void LineInput::gotFocusEvent(FocusEvent *event)
{

}

void LineInput::lostFocusEvent(FocusEvent *event)
{

}

void LineInput::textInputEvent(TextInputEvent *event)
{
	printf ("LineInput::textInputEvent(%s, %s), text=%s\n",
				this->widgetType().toChar(),
				this->name().toChar(),(const char*)event->text);
}


void LineInput::keyDownEvent(KeyEvent *event)
{
	printf ("LineInput::keyDownEvent(keycode=%i, repeat=%i, modifier: %i)\n",event->key, event->repeat, event->modifier);
}

void LineInput::keyUpEvent(KeyEvent *event)
{
	printf ("LineInput::keyUpEvent(keycode=%i, repeat=%i, modifier: %i)\n",event->key, event->repeat, event->modifier);
}



}	// EOF namespace tk
}	// EOF namespace ppl7
