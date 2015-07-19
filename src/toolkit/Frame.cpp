/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6). 
 * Web: http://www.pfp.de/ppl/
 *  
 * $Author: patrick $
 * $Revision: 1.11 $
 * $Date: 2009/01/07 19:32:14 $
 * $Id: Frame.cpp,v 1.11 2009/01/07 19:32:14 patrick Exp $
 * 
 ******************************************************************************* 
 * Copyright (c) 2008, Patrick Fedick <patrick@pfp.de>
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

/*!\class Frame
 * \ingroup PPLGroupToolkit
 * \brief Ein Rahmen-Element
 */


Frame::Frame()
{
	Name="Frame";
	Style=0;
	Style.SetCallback(this,&Style);
	Caption.SetCallback(this,&Caption);
	Font.Orientation=ORIENTATION::TOP;
	Font.SetCallback(this,&Font);
	Transparent=false;
	Transparent.SetCallback(this,&Transparent);
}

Frame::~Frame()
{
}

void Frame::Callback(void *data)
{
	NeedRedraw();
	CWidget::Callback(data);
}

int Frame::Paint()
{
	ppl6::grafix::CSurface *s=GetSurface();
	if (!s) return 0;
	COLORSTYLE *c=GetColors();
	if (!c) return 0;
	Font.color=c->frame.caption;
	int w=Width();
	int h=Height();
	s->Lock();
	if (!(bool)Transparent) {
		s->FillRect(0,0,w-1,h-1,c->frame.background);
	}
	int y1=0;
	switch ((int)Style) {
		case WIDGETSTYLE::UPSET:
			if (Caption.Len()) {
				DIMENSION d;
				s->TextSize(&Font,&d,&Caption);
				y1=d.height+4;
				s->FillRect(0,0,w-1,y1,c->frame.caption_background);
				s->Print(&Font,12,2,&Caption);
				
				s->Line(1,y1,w-2,y1,s->Brightness(c->frame.caption_background,0.5f));
				SetClientRect(2,d.height+4,2,2);
			} else {
				SetClientRect(2,2,2,2);
			}
			s->Line(0,0,w-1,0,c->frame.border_light);
			s->Line(0,1,0,h-1,c->frame.border_light);
			s->Line(w-1,1,w-1,h-1,c->frame.border_shadow);
			s->Line(1,h-1,w-1,h-1,c->frame.border_shadow);

			break;
		case WIDGETSTYLE::INSET:
			if (Caption.Len()) {
				DIMENSION d;
				s->TextSize(&Font,&d,&Caption);
				y1=d.height+4;
				s->FillRect(0,0,w-1,y1,c->frame.caption_background);
				s->Print(&Font,12,2,&Caption);
				
				s->Line(1,y1,w-2,y1,s->Brightness(c->frame.caption_background,0.5f));
				SetClientRect(2,d.height+2,2,2);
			} else {
				SetClientRect(2,2,2,2);
			}
			s->Line(0,0,w-1,0,c->frame.border_shadow);
			s->Line(0,1,0,h-1,c->frame.border_shadow);
			s->Line(w-1,1,w-1,h-1,c->frame.border_light);
			s->Line(1,h-1,w-1,h-1,c->frame.border_light);

			break;
		case WIDGETSTYLE::SIMPLE:
			if (Caption.Len()) {
				DIMENSION d;
				s->TextSize(&Font,&d,&Caption);
				y1=d.height+4;
				s->FillRect(0,0,w-1,y1,c->frame.caption_background);
				s->Print(&Font,12,2,&Caption);
				SetClientRect(2,d.height+4,2,2);
			} else {
				SetClientRect(2,2,2,2);
			}
			s->Rect(0,0,w-1,h-1,c->frame.border_shadow);
			break;
		case WIDGETSTYLE::FLAT:
			if (Caption.Len()) {
				DIMENSION d;
				s->TextSize(&Font,&d,&Caption);
				y1=d.height+4;
				s->FillRect(0,0,w-1,y1,c->frame.caption_background);
				s->Print(&Font,12,2,&Caption);
				SetClientRect(2,d.height+4,2,2);
			} else {
				SetClientRect(2,2,2,2);
			}
			break;
		default:
			
			if (Caption.Len()) {
				DIMENSION d;
				s->TextSize(&Font,&d,&Caption);
				s->Print(&Font,12,0,&Caption);
				y1=(d.height/2)-1;
				s->Line(0,y1,8,y1,c->frame.border_shadow);
				s->Line(14+d.width,y1,w-1,y1,c->frame.border_shadow);
				s->Line(1,y1+1,8,y1+1,c->frame.border_light);
				s->Line(14+d.width,y1+1,w-1,y1+1,c->frame.border_light);
				SetClientRect(5,d.height+2,5,5);
			} else {
				s->Line(0,y1,w-1,y1,c->frame.border_shadow);
				s->Line(1,y1+1,w-1,y1+1,c->frame.border_light);
				SetClientRect(5,5,5,5);
			}
			s->Line(0,y1+1,0,h-1,c->frame.border_shadow);
			s->Line(1,y1+1,1,h-2,c->frame.border_light);

			s->Line(0,h-2,w-1,h-2,c->frame.border_shadow);
			s->Line(0,h-1,w-1,h-1,c->frame.border_light);

			s->Line(w-2,y1,w-2,h-2,c->frame.border_shadow);
			s->Line(w-1,y1,w-1,h-1,c->frame.border_light);
			
	}
	s->Unlock();
	return 1;
}


}	// EOF namespace tk
}	// EOF namespace ppl6
