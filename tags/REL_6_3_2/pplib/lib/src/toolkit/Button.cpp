/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6). 
 * Web: http://www.pfp.de/ppl/
 *  
 * $Author: patrick $
 * $Revision: 1.15 $
 * $Date: 2009/01/07 19:32:14 $
 * $Id: Button.cpp,v 1.15 2009/01/07 19:32:14 patrick Exp $
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

/*!\class Button
 * \ingroup PPLGroupToolkit
 * \brief Klasse für Buttons
 */

Button::Button()
{
	Name="Button";
	Align=ALIGNMENT::CENTER;
	Caption.SetCallback(this,&Caption);
	Align.SetCallback(this,&Align);
	Image.SetCallback(this,&Image);
	Font.Orientation=ORIENTATION::TOP;
	pressed=false;
}


Button::~Button()
{
}


int Button::Paint()
{
	ppl6::grafix::CSurface *s=GetSurface();
	if (!s) return 0;
	int x,y;
	//printf ("Paint Info Widget: %i/%i\n",x,y);
	s->Lock();
	int x2=Width()-1;
	int y2=Height()-1;
	//s->FillRect(0,0,x2,y2,s->RGB(240,240,240));
	int w=y2-1;
	int h;
	for (y=1;y<y2;y++) {
		if (pressed) h=220-(50*y/w);
		else h=240-(50*y/w);
		s->Line(1,y,x2-1,y,s->RGB(h,h,h));
	}
	
	if (pressed) {
		s->Line(0,0,x2,0,s->RGB(64,64,64));
		s->Line(0,0,0,y2,s->RGB(64,64,64));
		s->Line(x2,1,x2,y2,s->RGB(128,128,128));
		s->Line(1,y2,x2,y2,s->RGB(128,128,128));
		
	} else {
		s->Line(0,0,x2,0,s->RGB(192,192,192));
		s->Line(0,0,0,y2,s->RGB(192,192,192));
		s->Line(x2,1,x2,y2,s->RGB(0,0,0));
		s->Line(1,y2,x2,y2,s->RGB(0,0,0));
		s->Line(1,1,x2-1,1,s->RGB(255,255,255));
		s->Line(1,1,1,y2-1,s->RGB(255,255,255));
		s->Line(x2-1,2,x2-1,y2-1,s->RGB(64,64,64));
		s->Line(2,y2-1,x2-1,y2-1,s->RGB(64,64,64));
	}

	if ((h=Image.GetHeight())) {
		int iw=Image.GetWidth();
		x=5;
		if (!Caption.Len()) x=1+(x2-iw)/2;
		if (pressed) Image.Draw(s,x,1+(y2-h)/2);
		else Image.Draw(s,x-1,(y2-h)/2);
	}
	w=Image.GetWidth();
	if (Caption.Len()) {
		DIMENSION d;
		s->TextSize(&Font,&d,Caption);
		switch ((int)Align) {
			case ALIGNMENT::LEFT:
				x=4;
				break;
			case ALIGNMENT::RIGHT:
				x=x2-2-d.width;
				break;
			case ALIGNMENT::CENTER:
			default:
				x=(x2-d.width)/2;
				break;
		};
		y=(y2-d.height)/2-2;
		if (w) {
			if (x<6+w) x=6+w;
		}
		if (pressed) {
			x++;
			y++;
		}
		s->Print(&Font,x,y,Caption);
	}
	s->Unlock();

	return 1;
}

void Button::Callback(void *data)
{
	NeedRedraw();
	CWidget::Callback(data);
}

int Button::MouseDown(CEvent *object, int x, int y, int buttons)
{
	pressed=true;
	NeedRedraw();
	return 1;
}

int Button::MouseUp(CEvent *object, int x, int y, int buttons)
{
	if (pressed) {
		//MouseClick(this,x,y,buttons);
		pressed=false;
		NeedRedraw();
	}
	return 1;
}

int Button::MouseLeave(CEvent *object)
{
	if (pressed) {
		pressed=false;
		NeedRedraw();
	}
	return 1;
}




}	// EOF namespace tk
}	// EOF namespace ppl6
