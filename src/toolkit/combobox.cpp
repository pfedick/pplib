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
namespace ppl7::tk {

ComboBox::ComboBox(int x, int y, int width, int height)
	: ppl7::tk::Widget()
{
	selection=NULL;
	create(x, y, width, height);
	ppl7::tk::WindowManager* wm=ppl7::tk::GetWindowManager();
	myCurrentIndex=0;

	dropdown_button=new ppl7::tk::Label(width - 24, 0, 20, height);
	dropdown_button->setIcon(wm->ButtonSymbols.getDrawable(4));
	dropdown_button->setEventHandler(this);
	this->addChild(dropdown_button);


}

ComboBox::~ComboBox()
{
	if (selection) delete selection;
}


void ComboBox::setCurrentText(const ppl7::String& text)
{
	if (text != myCurrentText) {
		std::list<ComboBoxItem>::iterator it;
		for (it=items.begin();it != items.end();++it) {
			if ((*it).text == text) {
				myCurrentIndex=(*it).index;
				myCurrentText=(*it).text;
				myCurrentIdentifier=(*it).identifier;
				needsRedraw();
			}
		}
	}
}

ppl7::String ComboBox::currentText() const
{
	return myCurrentText;
}

ppl7::String ComboBox::currentIdentifier() const
{
	return myCurrentIdentifier;
}

void ComboBox::setCurrentIndex(size_t index)
{
	if (index >= items.size()) return;
	std::list<ComboBoxItem>::iterator it;
	for (it=items.begin();it != items.end();++it) {
		if ((*it).index == index) {
			myCurrentIndex=index;
			myCurrentText=(*it).text;
			myCurrentIdentifier=(*it).identifier;
			needsRedraw();
		}
	}
}

void ComboBox::setCurrentIdentifier(const ppl7::String& identifier)
{
	std::list<ComboBoxItem>::iterator it;
	for (it=items.begin();it != items.end();++it) {
		if ((*it).identifier == identifier) {
			myCurrentIndex=(*it).index;
			myCurrentText=(*it).text;
			myCurrentIdentifier=(*it).identifier;
			needsRedraw();
		}
	}
}

size_t ComboBox::currentIndex() const
{
	return myCurrentIndex;
}

void ComboBox::add(const ppl7::String& text, const ppl7::String& identifier)
{
	ComboBoxItem item;
	item.text=text;
	item.identifier=identifier;
	item.index=items.size();
	items.push_back(item);
	if (items.size() == 1) setCurrentIndex(0);
	needsRedraw();
}


void ComboBox::clear()
{
	myCurrentText.clear();
	myCurrentIdentifier.clear();
	items.clear();
	if (selection) delete selection;
	selection=NULL;

	needsRedraw();
}

ppl7::String ComboBox::widgetType() const
{
	return "ComboBox";
}

void ComboBox::paint(ppl7::grafix::Drawable& draw)
{
	const ppl7::tk::WidgetStyle& style=ppl7::tk::GetWidgetStyle();
	int w=width() - 1;
	int h=height() - 1;
	draw.cls(style.buttonBackgroundColor);
	draw.drawRect(0, 0, w, h, style.frameBorderColorLight);
	ppl7::grafix::Font myFont=style.buttonFont;
	myFont.setColor(style.labelFontColor);
	myFont.setOrientation(ppl7::grafix::Font::TOP);
	ppl7::grafix::Size s=myFont.measure(myCurrentText);
	draw.print(myFont, 4, (draw.height() - s.height) >> 1, myCurrentText);


}

void ComboBox::mouseWheelEvent(ppl7::tk::MouseEvent* event)
{
	//printf ("Wheel: %d\n", event->wheel.y);
	if (event->wheel.y < 0 && myCurrentIndex < items.size() - 1) {
		setCurrentIndex(myCurrentIndex + 1);
		ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
		ev.setWidget(this);
		valueChangedEvent(&ev, myCurrentIndex);

	} else if (event->wheel.y > 0 && myCurrentIndex > 0) {
		setCurrentIndex(myCurrentIndex - 1);
		ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
		ev.setWidget(this);
		valueChangedEvent(&ev, myCurrentIndex);
	}
}

void ComboBox::mouseDownEvent(ppl7::tk::MouseEvent* event)
{
	if (event->widget() == this || event->widget() == dropdown_button) {
		//printf("ComboBox::mouseDownEvent\n");
		if (selection) {
			delete(selection);
			selection=NULL;
		} else {
			// we need the absolute coordinates of this widget on window
			// and create a new Frame with window as parent and topmost
			ppl7::grafix::Point p=absolutePosition();
			ppl7::tk::Widget* window=getTopmostParent();
			size_t maxsize=items.size();
			if (maxsize > 10) maxsize=10;
			selection=new ListWidget(p.x, p.y + this->height(), this->width(), maxsize * 30);
			selection->setTopmost(true);
			selection->setEventHandler(this);
			std::list<ComboBoxItem>::const_iterator it;
			for (it=items.begin();it != items.end();++it) {
				selection->add((*it).text, (*it).identifier);
			}
			selection->setCurrentIndex(myCurrentIndex);
			window->addChild(selection);
			GetWindowManager()->setMouseFocus(selection);
		}
	} else {
		EventHandler::mouseDownEvent(event);
	}
}

void ComboBox::valueChangedEvent(ppl7::tk::Event* event, int value)
{
	//printf("ComboBox::valueChangedEvent\n");
	if (selection != NULL && event->widget() == selection) {
		setCurrentIndex(selection->currentIndex());
		selection->deleteLater();
		selection=NULL;
		needsRedraw();
		ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
		ev.setWidget(this);
		valueChangedEvent(&ev, myCurrentIndex);
	} else {
		EventHandler::valueChangedEvent(event, value);
	}
}

void ComboBox::lostFocusEvent(ppl7::tk::FocusEvent* event)
{
	if (selection != NULL && event->newWidget() != selection) {
		selection->deleteLater();
		selection=NULL;
	}
}



} //EOF namespace
