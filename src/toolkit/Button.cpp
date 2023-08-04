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


Button::Button()
{
	const WidgetStyle& style=GetWidgetStyle();
	background=style.buttonBackgroundColor;
	foreground=style.buttonFontColor;
	myFont=style.buttonFont;
	setClientOffset(3, 3, 3, 3);
	isDown=false;
	is_checkable=false;
	is_checked=false;
}

Button::Button(int x, int y, int width, int height, const String& text, const Drawable& icon)
{
	const WidgetStyle& style=GetWidgetStyle();
	background=style.buttonBackgroundColor;
	foreground=style.buttonFontColor;
	myFont=style.buttonFont;
	create(x, y, width, height);
	setClientOffset(3, 3, 3, 3);
	isDown=false;
	Text=text;
	Icon=icon;
	is_checkable=false;
	is_checked=false;

}

Button::~Button()
{

}

const String& Button::text() const
{
	return Text;
}

void Button::setText(const String& text)
{
	Text=text;
	needsRedraw();
	geometryChanged();
}

const Drawable& Button::icon() const
{
	return Icon;
}

void Button::setIcon(const Drawable& icon)
{
	Icon=icon;
	needsRedraw();
	geometryChanged();

}

int Button::style() const
{
	return 0;
}

bool Button::isCheckable() const
{
	return is_checkable;
}

void Button::setCheckable(bool flag)
{
	is_checkable=flag;
}

bool Button::isChecked() const
{
	return is_checked;
}

void Button::setChecked(bool flag)
{
	is_checked=flag;
}


const Color& Button::color() const
{
	return foreground;
}

void Button::setColor(const Color& c)
{
	foreground=c;
	needsRedraw();
}

const Color& Button::backgroundColor() const
{
	return background;
}

void Button::setBackgroundColor(const Color& c)
{
	background=c;
	needsRedraw();
}

const Font& Button::font() const
{
	return myFont;
}

void Button::setFont(const Font& font)
{
	myFont=font;
	needsRedraw();
	geometryChanged();
}

String Button::widgetType() const
{
	return "Button";
}


Size Button::contentSize() const
{
	Size s;
	s=myFont.measure(Text);
	if (Icon.isEmpty() == false) {
		s.width+=4 + Icon.width();
		int h=2 + Icon.height();
		if (s.height < h) s.height=h;
	}
	s.width+=6;
	return s;
}

void Button::mouseDownEvent(MouseEvent* event)
{
	if (is_checkable) {
		is_checked=!is_checked;
		ppl7::tk::Event ev(ppl7::tk::Event::Toggled);
		ev.setWidget(this);
		toggledEvent(&ev, is_checked);
	}
	isDown=true;
	needsRedraw();
	EventHandler::mouseDownEvent(event);
}

void Button::mouseUpEvent(MouseEvent* event)
{
	if (is_checkable) {
		isDown=is_checked;
	} else {
		isDown=false;
	}
	needsRedraw();
	EventHandler::mouseUpEvent(event);
}

void Button::mouseLeaveEvent(MouseEvent* event)
{
	if (!is_checkable) {
		if (isDown) {
			isDown=false;
			needsRedraw();
		}
	}
	EventHandler::mouseLeaveEvent(event);
}

void Button::paint(Drawable& draw)
{
	Color light=background * 1.8f;
	Color shadow=background * 0.4f;
	Color shade1=background * 1.05f;
	Color shade2=background * 0.95f;
	Color shade3=background * 0.90f;
	Color shade4=background * 0.85f;

	int w=width() - 1;
	int h=height() - 1;
	//draw.cls(background);

	if (isDown) {
		shade1=background * 1.00f;
		shade2=background * 0.90f;
		shade3=background * 0.85f;
		shade4=background * 0.80f;
	}

	Rect r1=draw.rect();
	Rect r2=draw.rect();
	r1.y2-=((r1.y2 - r1.y1) / 2);
	if (isDown) r1.y2++;
	r2.y1=r1.y2;


	draw.colorGradient(r1, shade1, shade2, 1);
	draw.colorGradient(r2, shade3, shade4, 1);

	int x=0;
	int y=0;
	if (isDown) {
		draw.line(0, 0, w, 0, shadow);
		draw.line(0, 0, 0, h, shadow);
		draw.line(0, h, w, h, light);
		draw.line(w, 0, w, h, light);
		x++;
		y++;
	} else {
		draw.line(0, 0, w, 0, light);
		draw.line(0, 0, 0, h, light);
		draw.line(0, h, w, h, shadow);
		draw.line(w, 0, w, h, shadow);
	}
	Drawable d=clientDrawable(draw);
	if (Icon.isEmpty() == false) {
		int icon_x=x + 2;
		if (Text.isEmpty()) icon_x=(d.width() - Icon.width()) / 2;

		if (this->isEnabled()) d.bltAlpha(Icon, icon_x, y + (d.height() - Icon.height()) / 2);
		else d.bltBlend(Icon, 0.5f, icon_x, y + (d.height() - Icon.height()) / 2);
		x+=6 + Icon.width();
	}
	if (Text.notEmpty()) {
		if (this->isEnabled()) myFont.setColor(foreground);
		else myFont.setColor(Color::getBlendedf(background, foreground, 0.5f));
		myFont.setOrientation(Font::TOP);

		Size s=myFont.measure(Text);
		d.print(myFont, x, y + ((d.height() - s.height) >> 1), Text);
	}

}


}	// EOF namespace tk
}	// EOF namespace ppl7
