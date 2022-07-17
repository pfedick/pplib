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


LineInput::LineInput()
	:Frame()
{
	validator=NULL;
	const WidgetStyle& style=GetWidgetStyle();
	setBorderStyle(Inset);
	myColor=style.inputFontColor;
	myBackgroundColor=style.inputBackgroundColor;
	myFont=style.inputFont;
	setBackgroundColor(style.inputBackgroundColor);
	setSizeStrategyWidth(Widget::MINIMUM_EXPANDING);
	setTransparent(false);
	cursorpos=0;
	startpos=0;
	cursorx=0;
	cursorwidth=2;
	blinker=false;
	timerId=0;
}

LineInput::LineInput(int x, int y, int width, int height, const String& text)
	:Frame(x, y, width, height)
{
	validator=NULL;
	const WidgetStyle& style=GetWidgetStyle();
	setBorderStyle(Inset);
	myColor=style.inputFontColor;
	myFont=style.inputFont;
	setBackgroundColor(style.inputBackgroundColor);
	setSizeStrategyWidth(Widget::MINIMUM_EXPANDING);
	setTransparent(false);
	myText=text;
	if (validator) {
		if (validator->validateText(text) == true) validatedText=text;
	}
	cursorpos=0;
	startpos=0;
	cursorx=0;
	cursorwidth=2;
	blinker=false;
	timerId=0;
}

LineInput::~LineInput()
{
	if (timerId) GetWindowManager()->removeTimer(timerId);
	timerId=0;
}

void LineInput::setInputValidator(InputValidator* validator)
{
	this->validator=validator;
}

const WideString& LineInput::text() const
{
	return validatedText;
}

void LineInput::validateAndSendEvent(const WideString& text)
{
	if (validator) {
		if (validator->validateText(text) == false) return;
	}
	validatedText=text;
	Event ev(Event::Type::TextChanged);
	ev.setWidget(this);
	textChangedEvent(&ev, validatedText);
}

void LineInput::setText(const String& text)
{
	WideString new_text=text;
	if (new_text == myText) return;
	if (validator != NULL && validator->validateText(new_text) == false) return;
	myText=new_text;
	cursorpos=0;
	startpos=0;
	cursorx=0;
	needsRedraw();
	geometryChanged();
	validateAndSendEvent(myText);
}

const Color& LineInput::color() const
{
	return myColor;
}

void LineInput::setColor(const Color& c)
{
	myColor=c;
	needsRedraw();
}

const Color& LineInput::backgroundColor() const
{
	return myBackgroundColor;
}
void LineInput::setBackgroundColor(const Color& c)
{
	myBackgroundColor=c;
}


const Font& LineInput::font() const
{
	return myFont;
}

void LineInput::setFont(const Font& font)
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


void LineInput::paint(Drawable& draw)
{
	const WidgetStyle& style=GetWidgetStyle();
	if (myText == validatedText) Frame::setBackgroundColor(myBackgroundColor);
	else Frame::setBackgroundColor(style.inputInvalidBackgroundColor);
	Frame::paint(draw);
	Drawable d=clientDrawable(draw);
	//printf ("Text: %s, width: %i, height: %i\n",(const char*)myText, d.width(), d.height());
	int x=0;
	myFont.setColor(myColor);
	myFont.setOrientation(Font::TOP);
	Size s=myFont.measure(myText);
	d.print(myFont, x, (d.height() - s.height) >> 1, myText);
	//d.invert(Rect(cursorx,0,cursorx+cursorwidth,d.height()),myColor,backgroundColor());
	if (blinker) d.fillRect(cursorx, 0, cursorx + cursorwidth, d.height(), myColor);
}

void LineInput::mouseDownEvent(MouseEvent* event)
{
	//printf ("LineInput::mouseDownEvent\n");
	GetWindowManager()->setKeyboardFocus(this);
	cursorpos=calcPosition(event->p.x);
	calcCursorPosition();
	needsRedraw();
	blinker=true;
}

void LineInput::gotFocusEvent(FocusEvent* event)
{
	//printf ("LineInput::gotFocusEvent\n");
	blinker=true;
	timerId=GetWindowManager()->startTimer(this, 500);
	needsRedraw();
}

void LineInput::lostFocusEvent(FocusEvent* event)
{
	//printf ("LineInput::lostFocusEvent\n");
	blinker=false;
	if (timerId) GetWindowManager()->removeTimer(timerId);
	timerId=0;
	needsRedraw();
}

void LineInput::textInputEvent(TextInputEvent* event)
{
	/*printf("LineInput::textInputEvent(%s, %s), text=%ls\n",
		this->widgetType().toChar(),
		this->name().toChar(), (const wchar_t*)event->text);
		*/
	if (validator) {
		if (validator->validateInput(event->text) == false) return;
	}

	WideString left, right;
	left=myText.left(cursorpos);
	right=myText.mid(cursorpos);
	left+=event->text + right;
	myText=left;
	cursorpos++;
	calcCursorPosition();
	validateAndSendEvent(myText);
	if (validator) {
		if (validator->validateText(myText) == false) return;
	}
}


void LineInput::keyDownEvent(KeyEvent* event)
{
	//printf("LineInput::keyDownEvent(keycode=%i, repeat=%i, modifier: %i)\n", event->key, event->repeat, event->modifier);
	if (isEnabled()) {
		if ((event->modifier & KeyEvent::KEYMOD_MODIFIER) == KeyEvent::KEYMOD_NONE) {
			if (event->key == KeyEvent::KEY_LEFT && cursorpos > 0) {
				cursorpos--;
				calcCursorPosition();
			} else if (event->key == KeyEvent::KEY_RIGHT && cursorpos < myText.size()) {
				cursorpos++;
				calcCursorPosition();
			} else if (event->key == KeyEvent::KEY_HOME && cursorpos > 0) {
				cursorpos=0;
				calcCursorPosition();
			} else if (event->key == KeyEvent::KEY_END && cursorpos < myText.size()) {
				cursorpos=myText.size();
				calcCursorPosition();
			} else if (event->key == KeyEvent::KEY_BACKSPACE && cursorpos > 0) {
				WideString new_text=myText.left(cursorpos - 1) + myText.mid(cursorpos);
				myText=new_text;
				cursorpos--;
				calcCursorPosition();
				validateAndSendEvent(myText);
			} else if (event->key == KeyEvent::KEY_DELETE) {
				WideString new_text=myText.left(cursorpos) + myText.mid(cursorpos + 1);
				myText=new_text;
				calcCursorPosition();
				validateAndSendEvent(myText);
			}
		}
	}
	Frame::keyDownEvent(event);
}

void LineInput::keyUpEvent(KeyEvent* event)
{
	//printf ("LineInput::keyUpEvent(keycode=%i, repeat=%i, modifier: %i)\n",event->key, event->repeat, event->modifier);
	Frame::keyUpEvent(event);
}

void LineInput::timerEvent(Event* event)
{
	blinker=!blinker;
	needsRedraw();
	//if (GetWindowManager()->getKeyboardFocus()==this) GetWindowManager()->startTimer(this,500);
	//else blinker=false;

}


void LineInput::calcCursorPosition()
{
	WideString text=myText, left, right;
	Size s1;
	if ((ssize_t)cursorpos < 0) cursorpos=0;
	if (cursorpos > text.size()) cursorpos=text.size();
	if (cursorpos == 0) {
		cursorx=0;
		startpos=0;
	} else {
		left=text.left(cursorpos);
		right=text.mid(cursorpos);
		s1=myFont.measure(left);
		cursorx=s1.width;
	}
	needsRedraw();
	blinker=true;
}

int LineInput::calcPosition(int x)
{
	WideString text;
	size_t c=0;
	Size s1;
	while (c < myText.size()) {
		text=myText.left(c + 1);
		s1=myFont.measure(text);
		if (x < s1.width) break;
		c++;
	}

	return c;
}



}	// EOF namespace tk
}	// EOF namespace ppl7
