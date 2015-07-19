/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6). 
 * Web: http://www.pfp.de/ppl/
 *  
 * $Author: patrick $
 * $Revision: 1.5 $
 * $Date: 2008/04/15 20:36:32 $
 * $Id: Picture.cpp,v 1.5 2008/04/15 20:36:32 patrick Exp $
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

/*!\class Picture
 * \ingroup PPLGroupToolkit
 */

Picture::Picture()
{
	Style=0;
	Style.SetCallback(this,&Style);
	Image.SetCallback(this,&Image);
}

Picture::~Picture()
{
}

void Picture::Callback(void *data)
{
	NeedRedraw();
	CWidget::Callback(data);
}

int Picture::Paint()
{
	ppl6::grafix::CSurface *s=GetSurface();
	if (!s) return 0;
	COLORSTYLE *c=GetColors();
	if (!c) return 0;
	int w=Width();
	int h=Height();
	s->Lock();
	//s->FillRect(0,0,w-1,h-1,c->frame.background);
	switch ((int)Style) {
		case WIDGETSTYLE::UPSET:
			s->Line(0,0,w-1,0,c->frame.border_light);
			s->Line(0,1,0,h-1,c->frame.border_light);
			s->Line(w-1,1,w-1,h-1,c->frame.border_shadow);
			s->Line(1,h-1,w-1,h-1,c->frame.border_shadow);
			SetClientRect(2,2,2,2);
			break;
		case WIDGETSTYLE::INSET:
			s->Line(0,0,w-1,0,c->frame.border_shadow);
			s->Line(0,1,0,h-1,c->frame.border_shadow);
			s->Line(w-1,1,w-1,h-1,c->frame.border_light);
			s->Line(1,h-1,w-1,h-1,c->frame.border_light);
			SetClientRect(2,2,2,2);
			break;
		case WIDGETSTYLE::SIMPLE:
			s->Rect(0,0,w-1,h-1,c->frame.border_shadow);
			SetClientRect(1,1,1,1);
			break;
	}
	s->Unlock();
	if (Image.IsLoaded()) {
		RECT r;
		GetClientRect(&r);
		s->SetClipper(&r);
		s->SetOrigin(r.left,r.top);
		s->Draw(&Image,0,0);
	}
	return 1;
}



}
}
