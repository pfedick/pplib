/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
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
/*!\class Label
 * \ingroup PPLGroupToolkit
 * \brief Ein Beschriftungselement
 */
Label::Label()
{
	Name="Label";
	Style=0;
	Style.SetCallback(this,&Style);
	Caption.SetCallback(this,&Caption);
	Font.Orientation=ORIENTATION::TOP;
	Font.SetCallback(this,&Font);
	Transparent=true;
	Transparent.SetCallback(this,&Transparent);
}


Label::~Label()
{
}

void Label::Callback(void *data)
{
	NeedRedraw();
	CWidget::Callback(data);
}

int Label::Paint()
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
	//int y1=0;
	switch ((int)Style) {
		case WIDGETSTYLE::UPSET:
			if (Caption.Len()) {
				DIMENSION d;
				s->TextSize(&Font,&d,&Caption);
				s->Print(&Font,2,((h-d.height)>>1)-2,&Caption);

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
				s->Print(&Font,2,((h-d.height)>>1)-2,&Caption);

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
				s->Print(&Font,2,((h-d.height)>>1)-2,&Caption);

			}
			s->Rect(0,0,w-1,h-1,c->frame.border_shadow);
			break;

		default:
			if (Caption.Len()) {
				DIMENSION d;
				s->TextSize(&Font,&d,&Caption);
				s->Print(&Font,2,((h-d.height)>>1)-2,&Caption);
			}
	}
	s->Unlock();
	return 1;
}


#endif
}	// EOF namespace tk
}	// EOF namespace ppl6
