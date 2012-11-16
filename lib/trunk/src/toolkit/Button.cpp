/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
 *
 *******************************************************************************
 * Copyright (c) 2012, Patrick Fedick <patrick@pfp.de>
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


Button::Button()
{
	const WidgetStyle *style=GetWidgetStyle();
	background=style->buttonBackgroundColor;
	foreground=style->buttonFontColor;
	myFont=style->buttonFont;
	setClientOffset(2,2,2,2);
	isDown=false;
}

Button::~Button()
{

}

const String &Button::text() const
{
	return Text;
}

void Button::setText(const String &text)
{
	Text=text;
	needsRedraw();
	geometryChanged();
}

const Drawable &Button::icon() const
{
	return Icon;
}

void Button::setIcon(const Drawable &icon)
{
	Icon=icon;
	needsRedraw();
	geometryChanged();

}

int Button::style() const
{
	return 0;
}


const Color &Button::color() const
{
	return foreground;
}

void Button::setColor(const Color &c)
{
	foreground=c;
	needsRedraw();
}

const Color &Button::backgroundColor() const
{
	return background;
}

void Button::setBackgroundColor(const Color &c)
{
	background=c;
	needsRedraw();
}

const Font &Button::font() const
{
	return myFont;
}

void Button::setFont(const Font &font)
{
	myFont=font;
	needsRedraw();
	geometryChanged();
}

String Button::widgetType() const
{
	return "Button";
}


Size Button::contentSize() const
{
	Size s;
	s=myFont.measure(Text);
	if (Icon.isEmpty()==false) {
		s.width+=4+Icon.width();
		int h=2+Icon.height();
		if (s.height<h) s.height=h;
	}
	s.width+=6;
	return s;
}

void Button::mouseDownEvent(MouseEvent *event)
{
	isDown=true;
	needsRedraw();
}

void Button::mouseUpEvent(MouseEvent *event)
{
	isDown=false;
	needsRedraw();
}

void Button::mouseLeaveEvent(MouseEvent *event)
{
	if (isDown) {
		isDown=false;
		needsRedraw();
	}
}

void Button::paint(Drawable &draw)
{
	Color light=background*1.2f;
	Color shadow=background*0.4f;
	Color shade1=background*1.05f;
	Color shade2=background*0.95f;
	int w=width()-1;
	int h=height()-1;
	//draw.cls(background);

	if (isDown) {
		shade1=background*1.00f;
		shade2=background*0.90f;
	}

	draw.colorGradient(draw.rect(),shade1,shade2,1);

	int x=0;
	int y=0;
	if (isDown) {
		draw.line(0,0,w,0,shadow);
		draw.line(0,0,0,h,shadow);
		draw.line(0,h,w,h,light);
		draw.line(w,0,w,h,light);
		x++;
		y++;
	} else {
		draw.line(0,0,w,0,light);
		draw.line(0,0,0,h,light);
		draw.line(0,h,w,h,shadow);
		draw.line(w,0,w,h,shadow);
	}
	Drawable d=clientDrawable(draw);
	if (Icon.isEmpty()==false) {
		d.bltAlpha(Icon,x+2,y+(d.height())/2-Icon.height()/2);
		x+=6+Icon.width();
	}
	myFont.setColor(foreground);
	myFont.setOrientation(Font::TOP);
	d.print(myFont,x,y,Text);
	return;

/*




	Rect r=draw.rect();
	Color bright1=background*1.3f;
	Color bright2=background*1.0f;
	Color dark1=background*0.8f;
	Color dark2=background*0.7f;
	draw.drawRect(r,Color(66,66,66));
	int x1=r.x1+1;
	int y1=r.y1+1;
	int x2=r.x2-1;
	int y2=r.y2-1;
	int m=(y2-y1)/2+y1;
	draw.line(x1,y1,x2,y1,Color(219,219,219));
	draw.colorGradient(x1,y1+1,x2,m,bright1,bright2,1);
	draw.colorGradient(x1,m+1,x2,y2,dark1,dark2,1);

	draw.colorGradient(x1,y1,x1,m,Color(219,219,219),Color(130,130,130),1);
	draw.colorGradient(x2,y1,x2,m,Color(219,219,219),Color(130,130,130),1);
	draw.line(x1,m,x2,m,Color(83,83,83));
	Color c;
	Point mp((x2-x1)/2+x1,y2+(y2-m));
	Point p;
	float d,maxd;
	float brightness;
	p.setPoint(x1,y1);
	maxd=Distance(mp,p)*0.8;
	for (int y=y1;y<y2;y++) {
		for (int x=x1+1;x<x2;x++) {
			p.setPoint(x,y);
			c=draw.getPixel(x,y);
			// Entfernung zum Mittelpunkt berechnen
			d=Distance(mp,p);
			brightness=(maxd-d)/maxd;
			if (brightness<0) brightness=0.0f;
			//printf ("Distance von %i:%i zu %i:%i: %.2f\n",p.x(),p.y(),mp.x(),mp.y(),d);
			draw.blendPixel(x,y,Color(128,128,256),brightness);
		}
	}
	if (Icon.isEmpty()) {
		x1+=2;
	} else {
		draw.bltAlpha(Icon,x1+2,(y2-y1)/2-Icon.height()/2);
		x1+=4+Icon.width();

	}
	myFont.setColor(foreground);


	draw.print(myFont,x1,m+5,Text);
	*/
}


}	// EOF namespace tk
}	// EOF namespace ppl7
