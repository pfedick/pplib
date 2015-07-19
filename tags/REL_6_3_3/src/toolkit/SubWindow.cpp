/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6). 
 * Web: http://www.pfp.de/ppl/
 *  
 * $Author: patrick $
 * $Revision: 1.10 $
 * $Date: 2009/01/05 16:46:06 $
 * $Id: SubWindow.cpp,v 1.10 2009/01/05 16:46:06 patrick Exp $
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

/*!\class SubWindow
 * \ingroup PPLGroupToolkit
 */

SubWindow::SubWindow()
{
	Name="SubWindow";
	Transparent=false;
	Style=WIDGETSTYLE::PPLWINDOW;
	Transparent.SetCallback(this,&Transparent);
	Style.SetCallback(this,&Style);
}


SubWindow::~SubWindow()
{
	
}

int	SubWindow::CreateOwnSurface()
{
	return CWidget::CreateOwnSurface();
}

void SubWindow::Callback(void *data)
{
	if (data==&Caption) {
		NeedRedraw();
	}
	if (data==&IconSmall) NeedRedraw();
}

int SubWindow::MouseDown(CEvent *object, int x, int y, int buttons)
{
	
	if (buttons & SYSKEY::LBUTTON) {
		
		if ((x<=captionend.x && y<=captionend.y) || (buttons&SYSKEY::ALT)) {
			//printf ("MouseDown: %i/%i, %i\n",x,y,buttons);
			/* Verschieben des Fensters beginnt und muss vom �bergeordneten
			 * Widget abgewickelt werden
			 */
			if (parent) {
				parent->StartMove(this,buttons);
			}
		}
	}
	return CWindow::MouseDown(object,x,y,buttons);
}

/*
int SubWindow::Paint()
{
	CWindow::Paint();
	if (Style!=WIDGETSTYLE::PPLWINDOW) return 1;
	if (!close_button) {
		RECT r;
		close_button=new Button;
		close_button->SetEventHandler(this);
		GetClientRect(&r);
		close_button->Create(Width()-30,-r.top+5,15,15);
		AddChild(close_button);
	}
	
}
*/

}
}
