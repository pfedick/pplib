/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.2 $
 * $Date: 2010/02/12 19:43:47 $
 * $Id: Tab.cpp,v 1.2 2010/02/12 19:43:47 pafe Exp $
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

#ifdef TODO
/*!\class TabItem
 * \ingroup PPLGroupToolkit
 * \brief Element eines Tab-Elements
 */

TabItem::TabItem()
{
	Name="TabItem";
	CID=0xffff0009;
	Caption.SetCallback(this,&Caption);
	Font.Orientation=ORIENTATION::TOP;
	Font.Bold=true;
	Font.Size=12;
	Font.Antialias=true;
	Font.SetCallback(this,&Font);
	Image.SetCallback(this,&Image);
}

TabItem::~TabItem()
{
}

void TabItem::Callback(void *data)
{
	NeedRedraw();
	CWidget::Callback(data);
}


/*!\class Tab
 * \ingroup PPLGroupToolkit
 * \brief Tab-Elements
 */

Tab::Tab()
{
	activetab=0;
	CID=0xffff0010;
	Name="TabItem";
	Style.SetCallback(this,&Style);

}

Tab::~Tab()
{
}

void Tab::Callback(void *data)
{
	NeedRedraw();
	CWidget::Callback(data);
}

int Tab::AddTab(TabItem *item)
{
	return AddChild(item);
}

int Tab::Paint()
{
	ppl6::grafix::CSurface *s=GetSurface();
	if (!s) return 0;
	COLORSTYLE *c=GetColors();
	if (!c) return 0;
	int w=Width();
	int h=Height();
	// Zun�chst berechnen wir, wie hoch die Tabkante sein muss
	CWidget *child;
	TabItem *tab;
	DIMENSION d;
	int y1=0;
	child=GetFirstChild();
	while (child!=NULL) {
		if (child->CID==0xffff0009) {
			// Wir haben ein TabItem
			tab=(TabItem*)child;
			// Schaun wir mal, wieviel Platz die �berschrift braucht
			s->TextSize(&tab->Font,&d,&tab->Caption);
			if (d.height>y1) y1=d.height;
		}
		child=GetNextChild();
	}
	y1+=8;
	int x1=0;
	int x2;
	int tabnum=0;
	s->Lock();
	s->FillRect(0,y1,w-1,h-1,c->tab.background);
	s->Line(0,y1,w-1,y1,c->tab.border_light);
	s->Line(0,y1+1,0,h-1,c->tab.border_light);
	s->Line(w-1,y1+1,w-1,h-1,c->tab.border_shadow);
	s->Line(1,h-1,w-1,h-1,c->tab.border_shadow);

	child=GetFirstChild();
	while (child!=NULL) {
		if (child->CID==0xffff0009) {
			tab=(TabItem*)child;
			s->TextSize(&tab->Font,&d,&tab->Caption);
			x2=x1+d.width+10;
			if (activetab==tabnum) {
				tab->Font.color=c->tab.caption;
				s->FillRect(x1,0,x2,y1,c->tab.background);
				s->Line(x1,0,x2,0,c->tab.border_light);
				s->Line(x1,0,x1,y1,c->tab.border_light);
				s->Line(x2,1,x2,y1-1,c->tab.border_shadow);
			} else {
				tab->Font.color=c->tab.caption_inactive;
				s->FillRect(x1,5,x2,y1-1,c->tab.background_inactive);
				s->Line(x1,4,x2,4,c->tab.border_light);
				s->Line(x2,5,x2,y1-1,c->tab.border_shadow);
			}


			s->Print(&tab->Font,x1+2,4,&tab->Caption);


			x1=x2+1;
			tabnum++;
		}
		child=GetNextChild();
	}



	s->Unlock();
	return 1;
}

int Tab::MouseDown(CEvent *object, int x, int y, int buttons)
{
	return 1;
}

#endif
}	// EOF namespace tk
}	// EOF namespace ppl6
