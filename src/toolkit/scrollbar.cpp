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


Scrollbar::Scrollbar(int x, int y, int width, int height) // @suppress("Class members should be properly initialized")
	: ppl7::tk::Widget()
{
	this->create(x, y, width, height);
	setClientOffset(0, 0, 0, 0);
	//ppl7::grafix::Grafix *gfx=ppl7::grafix::GetGrafix();
	ppl7::tk::WindowManager* wm=ppl7::tk::GetWindowManager();
	up_button=new ppl7::tk::Button(0, 0, width - 2, 25);
	down_button=new ppl7::tk::Button(0, height - 25, width - 2, 25);
	up_button->setIcon(wm->ButtonSymbols.getDrawable(3));
	down_button->setIcon(wm->ButtonSymbols.getDrawable(4));
	up_button->setEventHandler(this);
	down_button->setEventHandler(this);
	addChild(up_button);
	addChild(down_button);
	size=0;
	pos=0;
	visibleItems=0;
	drag_started=false;
}

Scrollbar::~Scrollbar()
{
	if (drag_started) {
		drag_started=false;
		ppl7::tk::GetWindowManager()->releaseMouse(this);
	}
}

ppl7::String Scrollbar::widgetType() const
{
	return ppl7::String("Scrollbar");
}

void Scrollbar::setSize(int size)
{
	if (size != this->size && size >= 0) {
		this->size=size;
		if (pos > size) pos=size;
		needsRedraw();
	}
}

void Scrollbar::setPosition(int position)
{
	if (position != pos && position < size && position >= 0) {
		pos=position;
		needsRedraw();
	}
}

void Scrollbar::setVisibleItems(int items)
{
	visibleItems=items;
	needsRedraw();
}

int Scrollbar::position() const
{
	return pos;
}

void Scrollbar::paint(ppl7::grafix::Drawable& draw)
{
	const ppl7::tk::WidgetStyle& style=ppl7::tk::GetWidgetStyle();
	ppl7::grafix::Color light=style.buttonBackgroundColor * 1.8f;
	ppl7::grafix::Color shadow=style.buttonBackgroundColor * 0.4f;
	ppl7::grafix::Color shade1=style.buttonBackgroundColor * 1.05f;
	ppl7::grafix::Color shade2=style.buttonBackgroundColor * 0.85f;
	ppl7::grafix::Drawable indicator=draw.getDrawable(0, 25, draw.width(), draw.height() - 26);
	int w=indicator.width() - 1;
	//int h=indicator.height()-1;
	ppl7::grafix::Rect r1=indicator.rect();
	if (visibleItems > 0 && visibleItems < size) {
		int pxi=indicator.height() / size;
		r1.y1=pos * pxi;
		r1.y2=r1.y1 + visibleItems * pxi;
		if (r1.y2 >= indicator.height()) r1.y2=indicator.height() - 1;
	}
	slider_pos=r1;
	slider_pos.y1+=25;
	slider_pos.y2+=25;

	ppl7::grafix::Color scrollarea=style.windowBackgroundColor * 1.2f;
	indicator.cls(scrollarea);
	indicator.colorGradient(r1, shade1, shade2, 1);
	indicator.line(0, r1.y1, w, r1.y1, light);
	indicator.line(0, r1.y1, 0, r1.y2, light);
	indicator.line(0, r1.y2, w, r1.y2, shadow);
	indicator.line(w, r1.y1, w, r1.y2, shadow);

	//ppl7::tk::Widget::paint(draw);
	//draw.fillRect(0,y1,draw.width(),y2,style.frameBorderColorLight);
}

void Scrollbar::mouseDownEvent(ppl7::tk::MouseEvent* event)
{
	//printf("Scrollbar::mouseDownEvent\n");
	if (event->widget() == up_button && pos > 0) {
		pos--;
		needsRedraw();
		ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
		ev.setWidget(this);
		valueChangedEvent(&ev, pos);
		return;

	} else if (event->widget() == down_button && pos < size - 1) {
		pos++;
		needsRedraw();
		ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
		ev.setWidget(this);
		valueChangedEvent(&ev, pos);
		return;
	}
	if (event->buttonMask & ppl7::tk::MouseEvent::MouseButton::Left) {
		//printf("HorizontalSlider::mouseDownEvent: %d, %d\n", event->p.x, event->p.y);
		if (event->p.inside(slider_pos)) {
			drag_started=true;
			drag_offset=event->p.y - slider_pos.y1;
			drag_start_pos=event->p;
			ppl7::tk::GetWindowManager()->grabMouse(this);
		} else if (event->p.y < slider_pos.y1 && pos>0) {
			pos--;
			needsRedraw();
			ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
			ev.setWidget(this);
			valueChangedEvent(&ev, pos);
		} else if (event->p.y > slider_pos.y2 && pos < size - 1) {
			pos++;
			needsRedraw();
			ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
			ev.setWidget(this);
			valueChangedEvent(&ev, pos);
		}
	}
}

void Scrollbar::lostFocusEvent(ppl7::tk::FocusEvent* event)
{
	if (drag_started) {
		drag_started=false;
		ppl7::tk::GetWindowManager()->releaseMouse(this);
	}
}

void Scrollbar::mouseUpEvent(ppl7::tk::MouseEvent* event)
{
	if (drag_started) {
		drag_started=false;
		ppl7::tk::GetWindowManager()->releaseMouse(this);
	}
}

void Scrollbar::mouseMoveEvent(ppl7::tk::MouseEvent* event)
{
	if (event->buttonMask & ppl7::tk::MouseEvent::MouseButton::Left) {
		if (drag_started) {
			int draw_range=height() - 50;
			int64_t v=(event->p.y - drag_offset) * size / draw_range;
			if (v < 0) v=0;
			if (v > size - 1) v=size - 1;
			pos=v;
			ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
			ev.setWidget(this);
			valueChangedEvent(&ev, pos);

		}
	} else if (drag_started) {
		drag_started=false;
		ppl7::tk::GetWindowManager()->releaseMouse(this);
	}
}

void Scrollbar::mouseWheelEvent(ppl7::tk::MouseEvent* event)
{
	if (event->wheel.y < 0 && pos < size - 1) {
		pos++;
		ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
		ev.setWidget(this);
		valueChangedEvent(&ev, pos);
	} else if (event->wheel.y > 0 && pos > 0) {
		pos--;
		ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
		ev.setWidget(this);
		valueChangedEvent(&ev, pos);

	}
}

}
} //EOF namespace
