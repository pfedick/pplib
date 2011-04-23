/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6). 
 * Web: http://www.pfp.de/ppl/
 *  
 * $Author: patrick $
 * $Revision: 1.10 $
 * $Date: 2008/04/15 20:36:32 $
 * $Id: CEvent.cpp,v 1.10 2008/04/15 20:36:32 patrick Exp $
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


namespace ppl6 {
namespace tk {

/*!\class CEvent
 * \ingroup PPLGroupToolkit
 * \brief Basisklasse für Fenster- und Widget-Ereignisse
 */


CEvent::CEvent()
{
	handler=NULL;
}

CEvent::~CEvent()
{
}

int CEvent::SetEventHandler(CEvent *handler)
{
	this->handler=handler;
	return 1;
}

int CEvent::WindowCloseRequest(CEvent *object)
{
	if (handler) return handler->WindowCloseRequest(object);
	return 1;
}

int CEvent::WindowClose(CEvent *object)
{
	if (handler) return handler->WindowClose(object);
	return 0;
}

int CEvent::WindowResize(CEvent *object, int width, int height)
{
	if (handler) return handler->WindowResize(object,width,height);
	return 0;
}

int CEvent::WindowMove(CEvent *object, int x, int y)
{
	if (handler) return handler->WindowMove(object,x,y);
	return 0;
}

int CEvent::GotFocus(CEvent *object)
{
	//printf ("GotFocus\n");
	if (handler) return handler->GotFocus(object);
	return 0;
}

int CEvent::LostFocus(CEvent *object)
{
	//printf ("LostFocus\n");
	if (handler) return handler->LostFocus(object);
	return 0;
}

int CEvent::MouseEnter(CEvent *object)
{
	//printf ("MouseEnter\n");
	if (handler) return handler->MouseEnter(object);
	return 0;
}

int CEvent::MouseLeave(CEvent *object)
{
	//printf ("MouseLeave\n");
	if (handler) return handler->MouseLeave(object);
	return 0;
}

int CEvent::MouseMove(CEvent *object, int x, int y, int buttons)
{
	if (handler) return handler->MouseMove(object,x,y,buttons);
	return 0;
}

int CEvent::MouseEvent(CEvent *object, int x, int y, int buttons)
{
	if (handler) return handler->MouseEvent(object,x,y,buttons);
	return 0;
}

int CEvent::MouseDown(CEvent *object, int x, int y, int buttons)
{
	if (handler) return handler->MouseDown(object,x,y,buttons);
	return 0;
}

int CEvent::MouseUp(CEvent *object, int x, int y, int buttons)
{
	if (handler) return handler->MouseUp(object,x,y,buttons);
	return 0;
}

int CEvent::MouseClick(CEvent *object, int x, int y, int buttons)
{
	if (handler) return handler->MouseClick(object,x,y,buttons);
	return 0;
}

int CEvent::MouseDblClick(CEvent *object, int x, int y, int buttons)
{
	if (handler) return handler->MouseDblClick(object,x,y,buttons);
	return 0;
}

int CEvent::KeyDown(CEvent *object, int unicode, int buttons, int keycode)
{
	if (handler) return handler->KeyDown(object,unicode,buttons, keycode);
	return 0;
}

int CEvent::KeyUp(CEvent *object, int unicode, int buttons, int keycode)
{
	if (handler) return handler->KeyUp(object,unicode,buttons, keycode);
	return 0;
}

int CEvent::Message(CEvent *object, ppl6::CAssocArray *msg)
{
	if (handler) return handler->Message(object,msg);
	return 0;
}

int CEvent::AppActivate(CEvent *object)
{
	if (handler) return handler->AppActivate(object);
	return 0;
}

int CEvent::AppInactivate(CEvent *object)
{
	if (handler) return handler->AppInactivate(object);
	return 0;
}


}
}
