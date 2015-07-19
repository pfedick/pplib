/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.16 $
 * $Date: 2009/08/25 18:06:03 $
 * $Id: Input.cpp,v 1.16 2009/08/25 18:06:03 patrick Exp $
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

/*!\class Input
 * \ingroup PPLGroupToolkit
 * \brief Ein Eingabefeld
 */


Input::Input()
{
	Name="Input";
	Text.SetCallback(this,&Text);
	Font.Orientation=ORIENTATION::TOP;
	Font.Bold=true;
	cursor=0;
	insertmode=false;
	cron.SetCallback(this,&cron);
	blink=true;
}

Input::~Input()
{
}

void Input::Callback(void *data)
{
	if (data==&Text) {
		NeedRedraw();
	}
	if (data==&cron) {
		blink=!blink;
		ppl6::grafix::CSurface *s=GetSurface();
		if (s) s->SendMessage(this,NULL);
	}
	CWidget::Callback(data);
}

int Input::Message(CEvent *object, ppl6::CAssocArray *msg)
{
	NeedRedraw();
	return CWidget::Message(object,msg);
}

int Input::MouseDown(CEvent *object, int x, int y, int buttons)
{
	SetKeyFocus();
	if (Text.Len()) {
		ppl6::grafix::CSurface *s=GetSurface();
		int xx=4;
		int p=0;
		int w;
		wchar_t c;
		while ((c=Text[p]&0xff)!=0) {
			w=s->CharWidth(&Font,c);
			if (x<xx+w) {
				cursor=p;
				NeedRedraw();
				break;
			}
			xx+=w;
			p++;
			cursor=p;
		}
	}
	return 1;
}

int Input::Paint()
{
	ppl6::grafix::CSurface *s=GetSurface();
	if (!s) return 0;
	int x;
	//printf ("Paint Info Widget: %i/%i\n",x,y);
	s->Lock();
	int width=Width()-1;
	int height=Height()-1;
	s->Lock();
	s->Rect(0,0,width,height,s->RGB(128,128,128));
	s->FillRect(1,1,width-1,height-1,s->RGB(255,255,255));
	s->Line(1,1,width,1,s->RGB(64,64,64));
	s->Line(1,2,1,height-1,s->RGB(64,64,64));
	s->Line(2,height-2,width-2,height-2,s->RGB(240,240,240));
	s->Line(width-2,2,width-2,height-2,s->RGB(240,240,240));
	x=4;
	int w=1;
	if (Text.Len()) {
		DIMENSION d;
		s->Print(&Font,4,4,&Text);
		CWString tmp=Text;
		tmp.Cut(cursor);
		s->TextSize(&Font,&d,&tmp);
		x+=d.width;
		if (insertmode) {
			tmp.Set("");
			tmp.SetChar((const wchar_t)Text.GetChar(cursor));
			if (tmp.Len()) {
				s->TextSize(&Font,&d,&tmp);
				w=d.width;
				if (w<1) w=1;
			} else w=5;
		}
	}
	//s->FillRect(x,3,x+1,height-3,0);
	if (HaveKeyFocus()==true && blink==true)s->Negativ(x,3,x+w,height-3);
	s->Unlock();
	return 1;
}

int Input::KeyDown(CEvent *object, int unicode, int buttons, int keycode)
{
	blink=true;
	//printf ("Buttons: %05x, unicode: %i, keycode: %i\n",buttons,unicode, keycode);
	if (buttons&SYSKEY::INSERT) insertmode=true;
	else insertmode=false;

	switch (keycode) {
		case KEY::Left:
			if (cursor>0) cursor--;
			break;
		case KEY::Right:
			if (cursor<(int)Text.Len()) cursor++;
			break;
		case KEY::Home:
			cursor=0;
			break;
		case KEY::End:
			cursor=Text.Len();
			break;
		case KEY::Backspace:
			if (cursor) {
				cursor--;
				Text.Delete(cursor,1);
			}
			break;
		case KEY::Delete:
			Text.Delete(cursor,1);
			break;

		default:
			if (unicode>31) {
				if (insertmode) {
					if (Text.SetChar(unicode,cursor)) {
						cursor++;
					}
				} else {
					if (Text.InsertChar(unicode,cursor)) {
						cursor++;
					}
				}
			}
	};
	NeedRedraw();
	return 1;
}

int Input::GotFocus(CEvent *object)
{
	cron.StartIntervall(500);
	gfx->Crontab.Add(&cron);
	blink=true;
	NeedRedraw();
	return 1;
}

int Input::LostFocus(CEvent *object)
{
	gfx->Crontab.Delete(&cron);
	blink=false;
	NeedRedraw();
	return 1;
}

}	// EOF namespace tk
}	// EOF namespace ppl6


