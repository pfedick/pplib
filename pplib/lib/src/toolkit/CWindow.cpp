/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6). 
 * Web: http://www.pfp.de/ppl/
 *  
 * $Author: patrick $
 * $Revision: 1.18 $
 * $Date: 2009/01/08 16:51:48 $
 * $Id: CWindow.cpp,v 1.18 2009/01/08 16:51:48 patrick Exp $
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

/*!\class CWindow
 * \ingroup PPLGroupToolkit
 * \brief Fenster-Klasse
 * 
 * Diese Klasse wird sowohl für Fenster als auch Vollbildschirm-Anwendungen verwendet.
 * 
 */

CWindow::CWindow()
{
	Name="CWindow";
	colors=NULL;
	memset(&lastevent,0,sizeof(EVENT));
	Caption.SetCallback(this,&Caption);
	Font.Bold=true;
	Font.Shadow=true;
	Font.Orientation=ORIENTATION::TOP;
	Font.SetCallback(this,&Font);
	Transparent=false;
	Transparent.SetCallback(this,&Transparent);
	IconSmall.Set(gfx->Toolbar,20);
	IconSmall.SetCallback(this,&IconSmall);

	Style=WIDGETSTYLE::DEFAULT;
	Style.SetCallback(this,&Style);
	close_button=NULL;
	Flags=ppl6::grafix::Surface::Flags::Hardware |
			ppl6::grafix::Surface::Flags::Lockable |
			ppl6::grafix::Surface::Flags::VideoMemory |
			ppl6::grafix::Surface::Flags::PrimarySurface;
	RGBFormat=ppl6::grafix::RGBFormat::unknown;
}

CWindow::~CWindow()
{
	if (close_button) delete close_button;
	if (created==true && surface!=NULL) delete surface;
}

void CWindow::Callback(void *data)
{
	if (data==&Caption) {
		if (GetSurface()) GetSurface()->SetCaption(Caption);
		NeedRedraw();
	}
	if (data==&IconSmall) NeedRedraw();
}


int CWindow::Load()
{
	return 1;
}

void CWindow::InitColors()
{
	if (!surface) return;
	if (!colors) {
		colors=&defaultcolors;
		colors->window.background=surface->RGB(192,192,192,255);
		colors->window.border=surface->RGB(67,85,146,255);
		colors->window.caption=surface->RGB(255,255,255,255);
		colors->window.background_inactive=colors->window.background;
		colors->window.border_inactive=surface->Brightness(colors->window.border,0.8f);
		colors->window.caption_inactive=surface->Brightness(colors->window.caption,0.7f);
		colors->window.text=surface->RGB(0,0,0,255);



		colors->button.background=surface->RGB(220,220,220,255);
		colors->button.caption=surface->RGB(0,0,0,255);
		colors->button.border_light=surface->RGB(192,192,192,255);
		colors->button.border_shadow=surface->RGB(64,64,64,255);

		colors->frame.background=surface->RGB(220,220,220,255);
		colors->frame.caption=surface->RGB(0,0,0,255);
		colors->frame.caption_background=surface->RGB(230,230,240,255);
		colors->frame.border_light=surface->RGB(255,255,255,255);
		colors->frame.border_shadow=surface->RGB(64,64,64,255);

		colors->panel.background=surface->RGB(220,220,220,255);
		colors->panel.caption=surface->RGB(0,0,0,255);
		colors->panel.border_light=surface->RGB(192,192,192,255);
		colors->panel.border_shadow=surface->RGB(64,64,64,255);

		colors->tab.background=surface->RGB(220,220,220,255);
		colors->tab.caption=surface->RGB(0,0,0,255);
		colors->tab.caption_inactive=surface->RGB(100,100,140,255);
		colors->tab.background_inactive=surface->RGB(200,200,200,255);
		colors->tab.border_light=surface->RGB(255,255,255,255);
		colors->tab.border_shadow=surface->RGB(64,64,64,255);

	}
}

int CWindow::CreateOwnSurface()
{
	CGrafix *gfx=GetGrafix();
	if (!gfx) return 0;
	if (created) {
		SetError(1048,"CWindow::CreateOwnSurface");
		return 0;
	}
	if (!gfx->CreateWindow((const char*)Name,w.width, w.height, RGBFormat, Style, Flags, this)) {
		return 0;
	}
	created=true;
	return 1;
}


int CWindow::Paint()
{
	ppl6::grafix::CSurface *s=GetSurface();
	if (!s) return 0;

	COLORSTYLE *c=GetColors();
	if (!c) return 0;

	if ((bool)Transparent==false) {
		s->FillRect(0,0,Width()-1,Height()-1,c->window.background);
	}
	if (Style!=WIDGETSTYLE::PPLWINDOW) return 1;


	s->Lock();

	int x1=0;
	int y1=0;
	int x2=Width()-1;
	int y2=Height()-1;

	COLOR rahmen_hell, rahmen_dunkel, shadow1, shadow2, shadow3,shadow4;
	COLOR window_color=c->window.background|0xff000000;
	COLOR border=c->window.border;

	/*
	if (!HasFocus()) {
		//window_color=background_inactive;
		border=colors.window_border_inactive;
	}
	*/



	rahmen_hell=s->Brightness(border,1.8f);
	rahmen_dunkel=s->Brightness(border,0.5);
	shadow1=s->Brightness(window_color,0.1f);
	shadow2=s->Brightness(window_color,0.5f);
	shadow3=s->Brightness(window_color,0.8f);
	shadow4=s->Brightness(window_color,0.3f);

	// Aeusserer Rahmen
	s->Line(x1,y1,x2,y1,rahmen_hell);
	s->Line(x1,y1,x1,y2,rahmen_hell);
	s->Line(x1+1,y2,x2,y2,rahmen_dunkel);
	s->Line(x2,y1+1,x2,y2,rahmen_dunkel);
	
	// Dicker Rahmen
	s->FillRect(x1+1,y1+1,x1+2,y2-1,border);
	s->FillRect(x1+3,y1+1,x2-1,y1+2,border);
	s->FillRect(x1+3,y2-2,x2-1,y2-1,border);
	s->FillRect(x2-2,y1+3,x2-1,y2-3,border);

	// Schatten am linken Innenrand
	s->Line(x1+3,y1+3,x1+3,y2-3,shadow1);
	s->Line(x1+4,y1+3,x1+4,y2-3,shadow2);
	s->Line(x1+5,y1+3,x1+5,y2-3,shadow3);

	// Schatten oberer Rand
	s->Line(x1+3,y1+3,x2-3,y1+3,shadow1);
	s->Line(x1+4,y1+4,x2-3,y1+4,shadow2);
	s->Line(x1+5,y1+5,x2-3,y1+5,shadow3);


	
	// Jetzt die Titelzeile
	int w=5;
	int h=8;
	if (Caption.Len()) {
		DIMENSION d;
		s->TextSize(&Font,&d,&Caption);
		w=d.width;
		h+=d.height;
		//y1=d.height+4;
		//s->FillRect(0,0,w-1,y1,c->frame.caption_background);
		//s->Print(&Font,12,2,&Caption);
	}
	if (h<17) h=17;
	int x=x1+6;
	
	if (IconSmall.IsLoaded()) {
		x+=IconSmall.GetWidth()+4;
	}
	int xw=x1+w+x;

	s->FillRect(x1+3,y1+3,xw,y1+h,border);
	Font.color=c->window.caption;
	if (IconSmall.IsLoaded()) {
		IconSmall.Draw(s,x1+3,y1+2);
	}

	s->Print(&Font,x,y1+2,&Caption);
	captionend.x=xw;
	captionend.y=y1+h;

	s->Line(x1+3,y1+h+1,xw,y1+h+1,shadow1);
	s->Line(x1+4,y1+h+2,xw,y1+h+2,shadow2);
	s->Line(x1+5,y1+h+3,xw,y1+h+3,shadow3);

	int xw2=x2-53;
	s->FillRect(xw2,y1+3,x2-3,y1+h,border);
	s->Line(xw2,y1+h+1,x2-3,y1+h+1,shadow1);
	s->Line(xw2,y1+h+2,x2-3,y1+h+2,shadow2);
	s->Line(xw2,y1+h+3,x2-3,y1+h+3,shadow3);
	
	for (int i=0;i<h-2;i++) {
		s->Line(xw+i,y1+3,xw+i,y1+h-i,border);
		s->PutPixel(xw+i,y1+h-i,shadow1);
		s->PutPixel(xw+i,y1+h+1-i,shadow1);
		s->PutPixel(xw+i+1,y1+h+1-i,shadow4);
		s->PutPixel(xw+i+1,y1+h+2-i,shadow2);
		s->PutPixel(xw+i+1,y1+h+3-i,shadow3);

		s->Line(xw2-i,y1+3,xw2-i,y1+h-i,border);
		s->PutPixel(xw2-i,y1+h+1-i,shadow1);
		s->PutPixel(xw2-i,y1+h+2-i,shadow2);
		s->PutPixel(xw2-i,y1+h+3-i,shadow3);

	}
	
	/*
	if (rolledin) {
		s->Line(wr.left,wr.top+19,wr.right,wr.top+19,rahmen_dunkel);
		s->Line(wr.left,wr.top+20,wr.right,wr.top+20,rahmen_dunkel);
	}
	*/
	s->Unlock();

	SetClientRect(7,y1+h+5,7,7);

	if (!close_button) {
		close_button=new Button();
		close_button->SetEventHandler(this);
		close_button->Create(x2-30,-(y1+h+2),18,18);
		close_button->Image.Set(gfx->ButtonSymbolsSmall,0);
		AddChild(close_button);
	}


	return 1;
}

int CWindow::MouseClick(CEvent *object, int x, int y, int buttons)
{
	if (object!=NULL && object==close_button) {
		CAssocArray *msg=new CAssocArray;
		msg->Set("EVENT","CLOSE");
		SendMessage(this,msg);
	}
	return CWidget::MouseClick(object,x,y,buttons);
}

} // EOF namespace grafix
} // EOF namespace ppl6

