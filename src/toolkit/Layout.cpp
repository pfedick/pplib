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

HorizontalLayout::HorizontalLayout()
{
	spacing=2;
	lastTotal=0;
}

HorizontalLayout::~HorizontalLayout()
{

}

String HorizontalLayout::widgetType() const
{
	return "HorizontalLayout";
}

void HorizontalLayout::paint(Drawable &draw)
{

}


void HorizontalLayout::geometryChangedEvent(Event *event)
{
	//printf ("HorizontalLayout::geometryChangedEvent, %i Childs\n",numChilds());
	size_t num=numChilds();
	if (num) {
		typedef struct {
				Widget *widget;
				int		strategy;
				int		preferedWidth;
		} ITEM;
		std::list<Widget*>::iterator it;

		ITEM *item=(ITEM*)malloc(sizeof(ITEM)*num);
		if (!item) throw OutOfMemoryException();
		Widget *w;
		size_t c=0;
		int total=0;
		int hh=height();
		int available=width();
		int maxc=0;

		for (it=childsBegin();it!=childsEnd();++it) {
			w=*it;
			item[c].widget=w;
			item[c].strategy=w->sizeStrategyWidth();
			if (item[c].strategy==Widget::MAXIMUM_EXPANDING) maxc++;
			item[c].preferedWidth=w->preferedSize().width+spacing;
			total+=item[c].preferedWidth;
			//printf ("Widtget %i, strategy=%i, preferedWidth=%i\n",c,item[c].strategy,item[c].preferedWidth);
			c++;
		}
		if (total!=lastTotal) {
			lastTotal=total;
			available-=total;
			int x=0;
			for (size_t i=0;i<num;i++) {
				int ww=item[i].preferedWidth;
				if (item[i].strategy==Widget::MAXIMUM_EXPANDING) {
					if (available>0) {
						ww+=(available/maxc);
					}
				}
				item[i].widget->setPos(x,0);
				item[i].widget->setSize(ww-spacing,hh);
				x+=ww;
			}
		}

		free(item);
	}
	event->accept();
}


HorizontalSpacer::HorizontalSpacer()
{
	setSizeStrategyWidth(Widget::MAXIMUM_EXPANDING);
}


String HorizontalSpacer::widgetType() const
{
	return "HorizontalSpacer";
}

void HorizontalSpacer::paint(Drawable &draw)
{

}

}	// EOF namespace tk
}	// EOF namespace ppl7
