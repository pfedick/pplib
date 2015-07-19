/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.1 $
 * $Date: 2010/02/12 19:43:47 $
 * $Id: Window.cpp,v 1.1 2010/02/12 19:43:47 pafe Exp $
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

Window::Window(Widget *parent, WindowFlags flags)
: Widget(parent)
{
	engineprivate=NULL;
	windowflags=flags;
	gfxengine=NULL;
}

Window::~Window()
{

}


bool Window::show()
{
	CGrafix *gfx=GetGrafix();
	if (!gfx) return false;
	if (!gfx->CreateWindow(*this)) return false;
	return true;
}

bool Window::close()
{
	// Event auslösen, nur wenn dieser angenommen wird,
	// wird das Fenster tatsächlich geschlossen

	return true;
}




} // EOF namespace tk
} // EOF namespace ppl6
