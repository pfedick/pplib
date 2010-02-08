/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6). 
 * Web: http://www.pfp.de/ppl/
 *  
 * $Author: patrick $
 * $Revision: 1.7 $
 * $Date: 2008/04/15 20:36:32 $
 * $Id: RadioButton.cpp,v 1.7 2008/04/15 20:36:32 patrick Exp $
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

/*!\class RadioButton
 * \ingroup PPLGroupToolkit
 */

RadioButton::RadioButton()
{
	Name="RadioButton";
	Font.Orientation=ORIENTATION::TOP;
	Font.SetCallback(this,&Font);
	Checked=false;
	Checked.SetCallback(this,&Checked);

}

RadioButton::~RadioButton()
{
}

void RadioButton::Callback(void *data)
{
	if (data==&Checked) {
		if ((bool)Checked==true) {
			Checked.DeleteCallback();
			ppl6::CAssocArray a;
			a.Set("target","RadioButton");
			a.Set("action","uncheck");
			SendMessageToSiblings(&a);
			Checked=true;
			Checked.SetCallback(this,&Checked);
			NeedRedraw();
		}
	}
	CWidget::Callback(data);
}

int RadioButton::Paint()
{
	ppl6::grafix::CSurface *s=GetSurface();
	if (!s) return 0;
	COLORSTYLE *c=GetColors();
	if (!c) return 0;
	s->Lock();
	int x1=0;
	int y1=0;
	int x2=13;
	int y2=Height()-1;
	y1=((Height()-13)/2)-1;
	y2=y1+13;
		// Innenraum
	s->FillRect(x1+2,y1+3,x2-3,y2-3,s->RGB(255,255,255));

	COLOR shade1=s->RGB(128,128,128,255);
	s->PutPixel(x1+4,y1+1,shade1);
	s->PutPixel(x1+5,y1+1,shade1);
	s->PutPixel(x1+6,y1+1,shade1);
	s->PutPixel(x1+7,y1+1,shade1);

	s->PutPixel(x1+2,y1+2,shade1);
	s->PutPixel(x1+3,y1+2,shade1);
	s->PutPixel(x1+8,y1+2,shade1);
	s->PutPixel(x1+9,y1+2,shade1);

	s->PutPixel(x1+1,y1+3,shade1);
	s->PutPixel(x1+1,y1+4,shade1);

	s->PutPixel(x1+0,y1+5,shade1);
	s->PutPixel(x1+0,y1+6,shade1);
	s->PutPixel(x1+0,y1+7,shade1);
	s->PutPixel(x1+0,y1+8,shade1);

	s->PutPixel(x1+1,y1+9,shade1);
	s->PutPixel(x1+1,y1+10,shade1);

	shade1=s->RGB(64,64,64,255);
	s->PutPixel(x1+4,y1+2,shade1);
	s->PutPixel(x1+5,y1+2,shade1);
	s->PutPixel(x1+6,y1+2,shade1);
	s->PutPixel(x1+7,y1+2,shade1);

	s->PutPixel(x1+2,y1+3,shade1);
	s->PutPixel(x1+3,y1+3,shade1);
	s->PutPixel(x1+8,y1+3,shade1);
	s->PutPixel(x1+9,y1+3,shade1);

	s->PutPixel(x1+2,y1+4,shade1);

	s->PutPixel(x1+1,y1+5,shade1);
	s->PutPixel(x1+1,y1+6,shade1);
	s->PutPixel(x1+1,y1+7,shade1);
	s->PutPixel(x1+1,y1+8,shade1);

	s->PutPixel(x1+2,y1+9,shade1);

	shade1=s->RGB(212,208,200,255);
	s->PutPixel(x1+9,y1+4,shade1);

	s->PutPixel(x1+10,y1+5,shade1);
	s->PutPixel(x1+10,y1+6,shade1);
	s->PutPixel(x1+10,y1+7,shade1);
	s->PutPixel(x1+10,y1+8,shade1);

	s->PutPixel(x1+9,y1+9,shade1);
	s->PutPixel(x1+2,y1+10,shade1);
	s->PutPixel(x1+3,y1+10,shade1);
	s->PutPixel(x1+8,y1+10,shade1);
	s->PutPixel(x1+9,y1+10,shade1);

	s->PutPixel(x1+4,y1+11,shade1);
	s->PutPixel(x1+5,y1+11,shade1);
	s->PutPixel(x1+6,y1+11,shade1);
	s->PutPixel(x1+7,y1+11,shade1);

	shade1=s->RGB(255,255,255,255);
	s->PutPixel(x1+10,y1+3,shade1);
	s->PutPixel(x1+10,y1+4,shade1);
	s->PutPixel(x1+11,y1+5,shade1);
	s->PutPixel(x1+11,y1+6,shade1);
	s->PutPixel(x1+11,y1+7,shade1);
	s->PutPixel(x1+11,y1+8,shade1);

	s->PutPixel(x1+10,y1+9,shade1);
	s->PutPixel(x1+10,y1+10,shade1);

	s->PutPixel(x1+2,y1+11,shade1);
	s->PutPixel(x1+3,y1+11,shade1);
	s->PutPixel(x1+8,y1+11,shade1);
	s->PutPixel(x1+9,y1+11,shade1);

	s->PutPixel(x1+4,y1+12,shade1);
	s->PutPixel(x1+5,y1+12,shade1);
	s->PutPixel(x1+6,y1+12,shade1);
	s->PutPixel(x1+7,y1+12,shade1);


	if (Checked) {
		// Symbol 7
		s->Draw(gfx->ButtonSymbolsSmall,8,x1+2,y1+2);
	}
	if (Caption.Len()) {
		DIMENSION d;
		s->TextSize(&Font,&d,&Caption);
		s->Print(&Font,15,((Height()-d.height)>>1)-2,&Caption);
	}
	s->Unlock();
	return 1;
}


int RadioButton::MouseDown(CEvent *object, int x, int y, int buttons)
{
	if (buttons&SYSKEY::LBUTTON) {
		if (!Checked) Checked=true;
		Checked=true;
	}
	return 1;
}

int RadioButton::Message(CEvent *object, ppl6::CAssocArray *msg)
{
	if (!msg) return 1;
	CString s=msg->Get("target");
	if (s!="RadioButton") return 1;
	CString a=msg->Get("action");
	if (a=="uncheck") {
		if (Checked) {
			Checked=false;
			NeedRedraw();
		}
	}
	return 1;
}

}
}
