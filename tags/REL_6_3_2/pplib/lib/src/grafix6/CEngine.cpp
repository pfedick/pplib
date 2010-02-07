/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6). 
 * Web: http://www.pfp.de/ppl/
 *  
 * $Author: patrick $
 * $Revision: 1.15 $
 * $Date: 2009/01/08 12:16:57 $
 * $Id: CEngine.cpp,v 1.15 2009/01/08 12:16:57 patrick Exp $
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


namespace ppl6 {
namespace grafix {

/*!\class CEngine
 * \ingroup PPLGroupGrafik
 * \brief Basisklasse für Grafik-Engines
 */

CEngine::CEngine()
{
	grafix=NULL;
	IsInit=false;
}

CEngine::~CEngine()
{
}

int CEngine::Init()
{
	IsInit=true;
	return 1;
}

const char *CEngine::EngineName()
{
	return "NONE";
}

int CEngine::GetPrivateData(CAssocArray *ret)
{
	SetError(1007,"Engine: %s, Function: GetPrivateData", EngineName());
	return 0;
}

int CEngine::CreateWindow(CSurface *surface, ppl6::tk::CWindow *window)
{
	SetError(1007,"Engine: %s, Function: CreateWindow", EngineName());
	return 0;
}

int CEngine::CreateFullscreen(CSurface *surface, ppl6::tk::CWindow *window)
{
	SetError(1007,"Engine: %s, Function: CreateFullscreen", EngineName());
	return 0;
}

int CEngine::CreateSurface(CSurface *surface)
{
	SetError(1007,"Engine: %s, Function: CreateSurface", EngineName());
	return 0;
}

int CEngine::StartMessageLoop()
{
	SetError(1007,"Engine: %s, Function: StartMessageLoop", EngineName());
	return 0;
}

int CEngine::GetMousePosition(CSurface *surface, POINT *p)
{
	SetError(1007,"Engine: %s, Function: GetMousePosition", EngineName());
	return 0;
}

int CEngine::SendEvent(CSurface *surface, void *object, ppl6::CAssocArray *msg)
{
	SetError(1007,"Engine: %s, Function: SendEvent", EngineName());
	return 0;
}

int CEngine::LockWidgets()
{
	if (!IsInit) return 0;
	if (!grafix) return 0;
	return grafix->LockWidgets();
}

int CEngine::UnlockWidgets()
{
	if (!IsInit) return 0;
	if (!grafix) return 0;
	return grafix->UnlockWidgets();
}

} // EOF namespace grafix
} // EOF namespace ppl6

