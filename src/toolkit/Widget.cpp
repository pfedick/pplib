/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.1 $
 * $Date: 2010/02/12 19:43:47 $
 * $Id: Widget.cpp,v 1.1 2010/02/12 19:43:47 pafe Exp $
 *
 *******************************************************************************
 * Copyright (c) 2010, Patrick Fedick <patrick@pfp.de>
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
#include "ppl6.h"
#include "ppl6-grafix.h"
#include "ppl6-tk.h"

using namespace ppl6::grafix;

namespace ppl6 {
namespace tk {

/*!\class Widget
 * \ingroup PPLGroupToolkit
 * \brief Basisklasse für alle Elemente der Benutzeroberfläche
 */

Widget::Widget(Widget *parent)
{
	engineprivate=NULL;
	gfxengine=NULL;
	this->parent=parent;
	visible=true;
	enabled=true;
	iswindow=false;
	lockcount=0;
	needsredraw=true;
	child_needsredraw=false;
	topMost=false;
	if (parent) parent->addChild(this);
}

Widget::~Widget()
{
	if (gfxengine) gfxengine->destroyWindow(*this);
}

Point Widget::pos() const
{
	return p;
}

Size Widget::size() const
{
	return s;
}

int Widget::x() const
{
	return p.x();
}

int Widget::y() const
{
	return p.y();
}

int Widget::width() const
{
	return s.width();
}

int Widget::height() const
{
	return s.height();
}

bool Widget::isVisible() const
{
	return visible;
}

bool Widget::isEnabled() const
{
	return enabled;
}

Widget *Widget::parentWidget() const
{
	return parent;
}

Widget *Widget::window() const
{
	if (iswindow) return (Widget*)this;
	if (parent) return parent->window();
	return NULL;
}

void Widget::setX(int x)
{
	p.setX(x);
}

void Widget::setY(int y)
{
	p.setY(y);
}

void Widget::setPos(int x, int y)
{
	p.setPoint(x,y);
}

void Widget::setPos(const Point &p)
{
	this->p=p;
}

void Widget::setWidth(int width)
{
	s.setWidth(width);
}

void Widget::setHeight(int height)
{
	s.setHeight(height);
}

void Widget::setSize(int width, int height)
{
	s.setSize(width,height);
}

void Widget::setSize(const Size &s)
{
}

void Widget::setWindowFlags(WindowFlags f)
{
	windowflags=f;
}

Widget::WindowFlags Widget::windowFlags() const
{
	return windowflags;
}

void *Widget::enginePrivate() const
{
	return engineprivate;
}

void Widget::setEnginePrivate(void *ptr)
{
	engineprivate=ptr;
}

GFXEngine *Widget::gfxEngine() const
{
	return gfxengine;
}

void Widget::setGfxEngine(GFXEngine *engine)
{
	gfxengine=engine;
}

CString Widget::windowTitle() const
{
	return WindowTitle;
}

void Widget::setWindowTitle(const CString &title)
{
	WindowTitle=title;
	if (gfxengine) gfxengine->setWindowTitle(*this);
}

CImage Widget::windowIcon() const
{
	return WindowIcon;
}

void Widget::setWindowIcon(const CDrawable &icon)
{
	WindowIcon=icon;
	if (gfxengine) gfxengine->setWindowIcon(*this);
}

RGBFormat Widget::rgbFormat() const
{
	return format;
}

void Widget::setRGBFormat(const RGBFormat &format)
{
	this->format=format;
}

CDrawable Widget::getDrawable()
{
	CDrawable draw;
	if (engineprivate) {
		if (lockcount==0) {
			if (!gfxengine) {
				SetError(1072);
				return CDrawable();
			}
			if (!gfxengine->lockDevice(*this,draw)) return CDrawable();
		}
		lockcount++;

	} else if (parent) {
		draw=parent->getDrawable().getDrawable(Rect(p.x(),p.y(),s.width(),s.height()));
	}
	return draw;

}

void Widget::releaseDrawable(const CDrawable &draw)
{
	if (engineprivate) {
		if (lockcount>0) lockcount--;
		if (lockcount==0) {
			if (!gfxengine) {
				SetError(1072);
				return;
			}
			gfxengine->unlockDevice(*this);
		}
	} else if (parent) {
		parent->releaseDrawable(draw);
	}
}

int Widget::addChild(Widget *child)
{
	if (!child) {
		SetError(194);
		return 0;
	}
	if (!childs.Add(child)) return 0;
	child->parent=this;
	return 1;
}

Widget *Widget::getFirstChild()
{
	return (Widget*)childs.GetFirst();
}

Widget *Widget::getNextChild()
{
	return (Widget*)childs.GetNext();
}

void Widget::toTop(Widget *widget)
{
	if (widget) {
		if (childs.Delete(widget)) childs.Add(widget);
	} else if (parent) {
		parent->toTop(this);
	}
}

void Widget::paintChilds()
{
	//printf ("CWidget::PaintChilds, Name=%s, Visible=%i\n",(char*)Name,(int)Visible);
	Widget *child;
	bool top=false;
	while (1) {
		childs.Reset();
		while ((child=(Widget*)childs.GetNext())) {
			if (child->topMost==top) child->redraw();
		}
		if (top==true) break;
		top=true;
	}
}

void Widget::paintNormalChilds()
{
	Widget *child;
	childs.Reset();
	while ((child=(Widget*)childs.GetNext())) {
		if (child-topMost==false) child->redraw();
	}
}

void Widget::paintTopMostChilds()
{
	Widget *child;
	childs.Reset();
	while ((child=(Widget*)childs.GetNext())) {
		if (child->topMost==true) child->redraw();
	}
}

void Widget::needRedraw()
{
	if (!needsredraw) {
		needsredraw=true;
		if (parent) parent->clientNeedsRedraw();
	}
}

void Widget::clientNeedsRedraw()
{
	if (!child_needsredraw) {
		//printf ("CWidget::ClientNeedsRedraw %s\n",(char*)Name);
		child_needsredraw=true;
		if (parent) parent->clientNeedsRedraw();
	}
}

int Widget::redrawIfNeeded()
{
	//printf ("CWidget::RedrawIfNeeded, Name=%s, Visible=%i\n",(char*)Name,(int)Visible);
	//PrintStatus(10);
	if (!visible) return 1;
	//printf("Redraw 1\n");
	if (needsredraw) {
		//printf("Redraw 2\n");
		paint();
		// Alle Clients müssen ebenfalls neugezeichnet werden
		paintChilds();
		needsredraw=false;
		child_needsredraw=false;
		return 1;
	} else if (child_needsredraw) {
		//printf("Redraw 3, childs: %i\n",childs.Num());
		childs.Reset();
		Widget *child;
		while ((child=(Widget*)childs.GetNext())) {
			child->redrawIfNeeded();
		}
		child_needsredraw=false;
		// Eigenes Surface?
		/*
		if (ownsurface!=NULL && parent!=NULL) {
			ppl6::grafix::CSurface *target=parent->GetSurface();
			target->Blt(ownsurface,NULL,w.x,w.y);
		}
		*/
		return 1;
	}
	return 0;
}

int Widget::redraw()
{
	//printf ("CWidget::RedrawIfNeeded, Name=%s, Visible=%i\n",(char*)Name,(int)Visible);
	//PrintStatus(10);
	if (!visible) return 1;
	//printf("Redraw 1\n");
	paint();
	// Alle Clients müssen ebenfalls neugezeichnet werden
	paintChilds();
	needsredraw=false;
	child_needsredraw=false;
	return 1;
}

int Widget::paint()
{
	CDrawable draw=getDrawable();
	draw.drawRect(draw.rect(),Color(255,0,0));
	releaseDrawable(draw);
	return 1;
}


} // EOF namespace tk
} // EOF namespace ppl6
