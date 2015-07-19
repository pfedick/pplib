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


Button::Button(Widget *parent, int style)
: Widget(parent)
{
	background.setColor(230,235,230);
	foreground.setColor(255,255,255);
	myStyle=style;
	Text="Button";
	Font.setAntialias(true);
	Font.setSize(14);
	Font.setBold(false);
	Font.setOrientation(CFont::BOTTOM);
}

Button::~Button()
{
}


int Button::paint()
{
	CDrawable draw=getDrawable();
	Rect r=draw.rect();
	draw.drawRect(r,Color(66,66,66));
	int x1=r.x1()+1;
	int y1=r.y1()+1;
	int x2=r.x2()-1;
	int y2=r.y2()-1;
	int m=(y2-y1)/2+y1;
	draw.line(x1,y1,x2,y1,Color(219,219,219));
	draw.colorGradient(x1,y1+1,x2,m,Color(152,152,152),Color(125,125,125),1);
	draw.colorGradient(x1,m+1,x2,y2,Color(67,67,67),Color(43,43,43),1);

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


	Font.setColor(foreground);
	draw.print(Font,x1+2,m+5,Text);



	/*

	Rect inside;
	inside.setLeft(r.left()+1);
	inside.setTop(r.top()+1);
	inside.setBottom(r.bottom()-1);
	inside.setRight(r.right()-1);

	draw.FillRect(r,background);
	draw.Line(1,0,r.right()-1,0,bright);
	draw.PutPixel(2,2,background*1.2f);
	draw.Line(0,1,0,r.bottom()-1,bright);
	draw.Line(r.right(),1,r.right(),r.bottom()-1,dark);
	draw.Line(1,r.bottom(),r.right()-1,r.bottom(),dark);
	*/

	releaseDrawable(draw);
	return 1;

}

} // EOF namespace tk
} // EOF namespace ppl6
