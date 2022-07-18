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

//#define EVENT_DEBUG

Event::Event()
{
	t=Unknown;
	accepted=false;
	ignored=false;
	eventWidget=NULL;
}

Event::Event(Type t)
{
	this->t=t;
	accepted=false;
	ignored=false;
	eventWidget=NULL;
}

Event::~Event()
{

}

Event::Type Event::type() const
{
	return t;
}

void Event::setType(Type t)
{
	this->t=t;
}

void Event::ignore()
{
	ignored=false;
}

void Event::accept()
{
	accepted=true;
}

bool Event::isIgnored() const
{
	return ignored;
}

bool Event::isAccepted() const
{
	return accepted;
}

String Event::name() const
{
	switch (t) {
	case Unknown: return "Unknown";
	case MouseEnter: return "MouseEnter";
	case MouseMove: return "MouseMove";
	case MouseLeave: return "MouseLeave";
	case MouseDown: return "MouseDown";
	case MouseUp: return "MouseUp";
	case MouseDblClick: return "MouseDblClick";
	case MouseWheel: return "MouseWheel";
	case KeyDown: return "KeyDown";
	case KeyUp: return "KeyUp";
	case Show: return "Show";
	case Close: return "Close";
	case Quit: return "Quit";
	case Resize: return "Resize";
	case Move: return "Move";
	case FocusIn: return "FocusIn";
	case FocusOut: return "FocusOut";
	case GeometryChanged: return "GeometryChanged";
	case ValueChanged: return "ValueChanged";
	case Toggled: return "Toggled";
	case TextChanged: return "TextChanged";
	}
	throw UnknownEventException();
}

Widget* Event::widget() const
{
	return eventWidget;
}

void Event::setWidget(Widget* w)
{
	eventWidget=w;
}

MouseState::MouseState()
{
	p.x=0;
	p.y=0;
	button=(MouseState::MouseButton)0;
	buttonMask=(MouseState::MouseButton)0;
}

ResizeEvent::ResizeEvent()
{
	setType(Event::Resize);
	width=0;
	height=0;
}


KeyEvent::KeyEvent()
{
	key=KEY_UNKNOWN;
	modifier=0;
	repeat=false;
}

FocusEvent::FocusEvent(Event::Type t, Widget* old_widget, Widget* new_widget)
{
	bAccepted=true;
	setType(t);
	this->setWidget(old_widget);
	this->new_widget=new_widget;

}

Widget* FocusEvent::oldWidget() const
{
	return widget();
}

Widget* FocusEvent::newWidget() const
{
	return new_widget;
}

void FocusEvent::accept(bool flag)
{
	bAccepted=flag;
}

bool FocusEvent::accepted() const
{
	return bAccepted;
}


EventHandler::EventHandler()
{
	handler=NULL;
}

EventHandler::~EventHandler()
{

}

void EventHandler::setEventHandler(EventHandler* handler)
{
	this->handler=handler;
}

void EventHandler::mouseMoveEvent(MouseEvent* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::mouseMoveEvent(%s, %s)\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar());
#endif
	if (handler) handler->mouseMoveEvent(event);
}

void EventHandler::mouseDownEvent(MouseEvent* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::mouseDownEvent(%s, %s)\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar());
#endif

	if (handler) handler->mouseDownEvent(event);
}

void EventHandler::mouseUpEvent(MouseEvent* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::mouseUpEvent(%s, %s)\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar());
#endif

	if (handler) handler->mouseUpEvent(event);
}

void EventHandler::mouseDblClickEvent(MouseEvent* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::mouseDblClickEvent(%s, %s)\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar());
#endif

	if (handler) handler->mouseDblClickEvent(event);
}

void EventHandler::mouseClickEvent(MouseEvent* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::mouseClickEvent(%s, %s)\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar());
#endif

	if (handler) handler->mouseClickEvent(event);
}

void EventHandler::mouseEnterEvent(MouseEvent* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::mouseEnterEvent(%s, %s)\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar());
#endif

	if (handler) handler->mouseEnterEvent(event);
}

void EventHandler::mouseLeaveEvent(MouseEvent* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::mouseLeaveEvent(%s, %s)\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar());
#endif

	if (handler) handler->mouseLeaveEvent(event);
}

void EventHandler::mouseWheelEvent(MouseEvent* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::mouseWheelEvent(%s, %s)\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar());
#endif

	if (handler) handler->mouseWheelEvent(event);
}


void EventHandler::quitEvent(Event* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::quitEvent(%s, %s)\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar());
#endif

	if (handler) handler->quitEvent(event);
}

void EventHandler::closeEvent(Event* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::closeEvent(%s, %s)\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar());
#endif

	if (handler) handler->closeEvent(event);
}

void EventHandler::resizeEvent(ResizeEvent* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::resizeEvent(%s, %s)\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar());
#endif

	if (handler) handler->resizeEvent(event);
}


void EventHandler::geometryChangedEvent(Event* event)
{
#ifdef EVENT_DEBUG
	/*
	if (event!=NULL && event->eventWidget!=NULL) {
		printf ("EventHandler::geometryChangedEvent(%s, %s)\n",
				event->eventWidget->widgetType().toChar(),
				event->eventWidget->name().toChar());
	}
	*/
#endif
	if (handler) handler->geometryChangedEvent(event);
}

void EventHandler::gotFocusEvent(FocusEvent* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::gotFocusEvent(%s, %s)\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar());
#endif
	if (handler) handler->gotFocusEvent(event);
}

void EventHandler::lostFocusEvent(FocusEvent* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::lostFocusEvent(%s, %s)\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar());
#endif
	if (handler) handler->lostFocusEvent(event);
}

void EventHandler::textInputEvent(TextInputEvent* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::textInputEvent(%s, %s), text=%ls\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar(), (const wchar_t*)event->text);
#endif
	if (handler) handler->textInputEvent(event);
}

void EventHandler::keyDownEvent(KeyEvent* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::keyDownEvent(%s, %s), key: %i\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar(), event->key);
#endif
	if (handler) handler->keyDownEvent(event);
}

void EventHandler::keyUpEvent(KeyEvent* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::keyUpEvent(%s, %s), key: %i\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar(), event->key);
#endif
	if (handler) handler->keyUpEvent(event);
}

void EventHandler::timerEvent(Event* event)
{
#ifdef EVENT_DEBUG
	printf("EventHandler::timerEvent(%s, %s)\n",
		event->eventWidget->widgetType().toChar(),
		event->eventWidget->name().toChar());
#endif
	if (handler) handler->timerEvent(event);
}

void EventHandler::valueChangedEvent(Event* event, int value)
{
	if (handler) handler->valueChangedEvent(event, value);
}

void EventHandler::valueChangedEvent(Event* event, int64_t value)
{
	if (handler) handler->valueChangedEvent(event, value);
}


void EventHandler::valueChangedEvent(Event* event, double value)
{
	if (handler) handler->valueChangedEvent(event, value);
}


void EventHandler::toggledEvent(Event* event, bool checked)
{
	if (handler) handler->toggledEvent(event, checked);
}

void EventHandler::textChangedEvent(Event* event, const ppl7::String& text)
{
	if (handler) handler->textChangedEvent(event, text);
}



}	// EOF namespace tk
}	// EOF namespace ppl7
