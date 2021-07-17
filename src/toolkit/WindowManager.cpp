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


static WindowManager *wm=NULL;

WindowManager *GetWindowManager()
{
	if (!wm) throw NoWindowManagerException();
	return wm;
}

const WidgetStyle &GetWidgetStyle()
{
	if (!wm) throw NoWindowManagerException();
	return wm->getWidgetStyle();
}

WindowManager::WindowManager()
{
	if (wm!=NULL) throw DuplicateWindowManagerException();
	wm=this;
	LastMouseDown=NULL;
	LastMouseEnter=NULL;
	clickCount=0;
	doubleClickIntervall=200;
	KeyboardFocus=NULL;
	updateButtonSymbols();
}

WindowManager::~WindowManager()
{
	if (wm==this) wm=NULL;
}

const WidgetStyle &WindowManager::getWidgetStyle() const
{
	return Style;
}

void WindowManager::setWidgetStyle(const WidgetStyle &style)
{
	Style=style;
	updateButtonSymbols();
}

void WindowManager::updateButtonSymbols()
{
	Resource *res=GetPPLResource();
	ppl7::grafix::ImageList tmp;
	tmp.load(res->getMemory(16),15,15,ImageList::DIFFUSE);
	ppl7::grafix::Size s=tmp.size();
	ButtonSymbols.create(s.width, s.height);
	ButtonSymbols.bltDiffuse(tmp, 0, 0, Style.buttonFontColor);
	ButtonSymbols.setIconSize(15, 15);

}


Widget *WindowManager::findMouseWidget(Widget *window, Point &p)
{
	if (!window) return NULL;
	/*
	printf ("Iterate: %s:%s, %d:%d, Point: %d:%d\n", (const char*)window->widgetType(),
			(const char*)window->name(), window->x(), window->y(),
			p.x, p.y);
	*/
	std::list<Widget*>::const_reverse_iterator it;
	Point p2;
	if (window->childs.size()>0) {
		for(it=window->childs.rbegin();it!=window->childs.rend();++it) {
			Widget *w=*it;
			if (p.x>=w->p.x
					&& p.y>=w->p.y
					&& p.x < w->p.x+w->s.width
					&& p.y < w->p.y+w->s.height) {
				// Passendes Widget gefunden, Koordinaten des Events auf das Widget umrechnen
				p2.x=p.x-w->p.x-w->myClientOffset.x1;
				p2.y=p.y-w->p.y-w->myClientOffset.y1;
				p=p2;
				return findMouseWidget(w,p);	// Iterieren
			}
		}
	}
	//printf ("MATCH\n");
	return window;
}

void WindowManager::dispatchEvent(Window *window, Event &event)
{
	Widget *w;
	switch (event.type()) {
		case Event::MouseEnter:
			window->mouseState=(MouseEvent&)event;
			event.setWidget(window);
			LastMouseEnter=window;
			window->mouseEnterEvent((MouseEvent*)&event);
			return;
		case Event::MouseLeave:
			window->mouseState=(MouseEvent&)event;
			if (LastMouseEnter) {
				event.setWidget(LastMouseEnter);
				LastMouseEnter->mouseLeaveEvent((MouseEvent*)&event);
				LastMouseEnter=NULL;
			}
			event.setWidget(window);
			window->mouseLeaveEvent((MouseEvent*)&event);
			return;
		case Event::MouseMove:
			window->mouseState=(MouseEvent&)event;
			//printf ("window->mouseState.p.x=%i\n",window->mouseState.p.x);
			w=findMouseWidget(window,((MouseEvent*)&event)->p);
			if (w) {
				if (w!=LastMouseEnter) {
					if (LastMouseEnter) {
						event.setWidget(LastMouseEnter);
						LastMouseEnter->mouseLeaveEvent((MouseEvent*)&event);
					}
					event.setWidget(w);
					w->mouseEnterEvent((MouseEvent*)&event);
					LastMouseEnter=w;
				}
				event.setWidget(w);
				w->mouseMoveEvent((MouseEvent*)&event);
			} else if (LastMouseEnter) {
				event.setWidget(LastMouseEnter);
				LastMouseEnter->mouseLeaveEvent((MouseEvent*)&event);
				LastMouseEnter=NULL;
			}
			return;
		case Event::MouseDown:
			window->mouseState=(MouseEvent&)event;
			w=findMouseWidget(window,((MouseEvent*)&event)->p);
			if (w) {
				if (w!=LastMouseDown) {
					clickCount=0;
				}
				LastMouseDown=w;
				event.setWidget(w);
				w->mouseDownEvent((MouseEvent*)&event);
			}
			return;

		case Event::MouseUp:
			window->mouseState=(MouseEvent&)event;
			w=findMouseWidget(window,((MouseEvent*)&event)->p);
			if (w) {
				event.setWidget(w);
				w->mouseUpEvent((MouseEvent*)&event);
				if (LastMouseDown==w) {
					clickCount++;
					clickEvent=*((MouseEvent*)&event);
					if (clickCount==1) startClickEvent(static_cast<Window*>(window));
					//LastMouseDown->mouseClickEvent((MouseEvent*)&event);
				} else {
					clickCount=0;
					LastMouseDown=NULL;
				}
			}
			return;
		case Event::MouseWheel:
			window->mouseState=(MouseEvent&)event;
			w=findMouseWidget(window,((MouseEvent*)&event)->p);
			if (w) {
				event.setWidget(w);
				w->mouseWheelEvent((MouseEvent*)&event);
			}
			return;


		default:
			printf("WindowManager::dispatchEvent(%tu, %s)  ==> Unhandled Event\n",(std::ptrdiff_t)window,event.name().toChar());
			return;
	}

}

void WindowManager::setDoubleClickIntervall(int ms)
{
	doubleClickIntervall=ms;
}

int WindowManager::getDoubleClickIntervall() const
{
	return doubleClickIntervall;
}



void WindowManager::dispatchClickEvent(Window *window)
{
	if (!window) return;
	if (!LastMouseDown) return;
	if (clickCount==1) LastMouseDown->mouseClickEvent(&clickEvent);
	else if (clickCount>1) LastMouseDown->mouseDblClickEvent(&clickEvent);
	clickCount=0;
	LastMouseDown=NULL;
}

void WindowManager::setKeyboardFocus(Widget *w)
{
	if (w==KeyboardFocus) return;
	if (KeyboardFocus) {
		FocusEvent e;
		KeyboardFocus->lostFocusEvent(&e);
	}
	KeyboardFocus=w;
	FocusEvent e;
	KeyboardFocus->gotFocusEvent(&e);
}

Widget *WindowManager::getKeyboardFocus() const
{
	return KeyboardFocus;
}


}	// EOF namespace tk
}	// EOF namespace ppl7
