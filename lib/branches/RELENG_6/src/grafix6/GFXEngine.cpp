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
//#ifdef HAVE_MATH_H
#include <math.h>
//#endif
#include "ppl6-grafix.h"

namespace ppl6 {
namespace grafix {

/*!\class GFXEngine
 * \ingroup PPLGroupGrafik
 * \brief Basisklasse von der alle unterstützen Grafik-Engines abgeleitet werden
 */

AdapterIdentifier::AdapterIdentifier()
{
	adapterId=0;
	major_version=0;
	minor_version=0;
	sub_version=0;
	version=0;
	build=0;
	currentWidth=0;
	currentHeight=0;
	currentRefreshRate=0;
}

void AdapterIdentifier::print() const
{
	printf ("Adapter %i:\n",adapterId);
	printf ("  DriverName: %s\n",(const char*)DriverName);
	printf ("  Description: %s\n",(const char*)Description);
	printf ("  Version: %i.%i.%i, Build: %i\n",major_version,minor_version,sub_version,build);
	printf ("  CurrentResolution: %i x %i (%i Hz), RGBFormat: %i (%s)\n",
        currentWidth,currentHeight,currentRefreshRate,currentRGBFormat.format(),(const char*)currentRGBFormat.name());
	printf ("  Textures: %i x %i, Aspekt: %i\n",Caps.MaxTextureWidth, Caps.MaxTextureHeight, Caps.MaxTextureAspectRatio);
	printf ("  MaxLights: %i\n",Caps.MaxActiveLights);
	printf ("  Hardware Surface: %i, Blit: %i, ColorKeyBlit: %i, AlphaBlit: %i, Fill: %i\n",
			Caps.hwSurface, Caps.hwBlit, Caps.hwColorKeyBlit, Caps.hwAlphaBlit, Caps.hwFill);

}

AdapterCaps::AdapterCaps()
{
	MaxActiveLights=0;
	MaxTextureWidth=0;
	MaxTextureHeight=0;
	MaxTextureAspectRatio=0;
	PixelShaderVersion=0;
	VertexShaderVersion=0;
	hwSurface=false;
	hwBlit=false;
	hwColorKeyBlit=false;
	hwAlphaBlit=false;
	hwFill=false;
}

AdapterMode::AdapterMode()
{
	width=0;
	height=0;
	refreshRate=0;
	format=RGBFormat::unknown;
}

GFXEngine::GFXEngine()
{
	adapterNo=0;
}

GFXEngine::~GFXEngine()
{
}

CString GFXEngine::name() const
{
	return CString("none");
}

bool GFXEngine::isInit() const
{
	SetError(1007,"Engine: %s, Function: isInit", (const char*)name());
	return false;
}

int GFXEngine::getAdapterCount()
{
	SetError(1007,"Engine: %s, Function: getAdapterCount", (const char*)name());
	return 0;
}

int GFXEngine::getAdapterInfo(int adapter, AdapterIdentifier &info)
{
	SetError(1007,"Engine: %s, Function: getAdapterInfo", (const char*)name());
	return 0;
}

int GFXEngine::getAdapterModeCount(int adapter, const RGBFormat &format)
{
	SetError(1007,"Engine: %s, Function: getAdapterModeCount", (const char*)name());
	return 0;
}

int GFXEngine::getAdapterModeInfo(int adapter, const RGBFormat &format, int mode, AdapterMode &info)
{
	SetError(1007,"Engine: %s, Function: getAdapterModeInfo", (const char*)name());
	return 0;
}

int GFXEngine::useAdapter(int adapter)
{
	if (adapter>getAdapterCount()) {
		SetError(1064);
		return 0;
	}
	adapterNo=adapter;
	return 1;
}

int GFXEngine::adapter() const
{
	return adapterNo;
}

bool GFXEngine::createWindow(tk::Widget &widget)
{
	SetError(1007,"Engine: %s, Function: createWindow", (const char*)name());
	return 0;
}

bool GFXEngine::destroyWindow(tk::Widget &widget)
{
	SetError(1007,"Engine: %s, Function: destroyWindow", (const char*)name());
	return 0;
}

bool GFXEngine::setWindowTitle(tk::Widget &widget)
{
	SetError(1007,"Engine: %s, Function: setWindowTitle", (const char*)name());
	return 0;
}

bool GFXEngine::setWindowIcon(tk::Widget &widget)
{
	SetError(1007,"Engine: %s, Function: setWindowIcon", (const char*)name());
	return 0;
}

bool GFXEngine::lockDevice(tk::Widget &widget, CDrawable &draw)
{
	SetError(1007,"Engine: %s, Function: LockDevice", (const char*)name());
	return 0;
}

bool GFXEngine::unlockDevice(tk::Widget &widget)
{
	SetError(1007,"Engine: %s, Function: UnlockDevice", (const char*)name());
	return 0;
}

bool GFXEngine::updateDevice(tk::Widget &widget)
{
	SetError(1007,"Engine: %s, Function: UpdateDevice", (const char*)name());
	return 0;
}


bool GFXEngine::createSurface(CSurface &surface)
{
	SetError(1007,"Engine: %s, Function: createSurface", (const char*)name());
	return 0;
}

bool GFXEngine::destroySurface(CSurface &surface)
{
	SetError(1007,"Engine: %s, Function: destroySurface", (const char*)name());
	return 0;
}

bool GFXEngine::lockSurface(CSurface &surface, CDrawable &draw)
{
	SetError(1007,"Engine: %s, Function: lockSurface", (const char*)name());
	return 0;
}

bool GFXEngine::unlockSurface(CSurface &surface)
{
	SetError(1007,"Engine: %s, Function: unlockSurface", (const char*)name());
	return 0;
}

bool GFXEngine::beginScene()
{
	SetError(1007,"Engine: %s, Function: beginScene", (const char*)name());
	return 0;
}

bool GFXEngine::endScene()
{
	SetError(1007,"Engine: %s, Function: endScene", (const char*)name());
	return 0;
}

bool GFXEngine::blt(tk::Widget &widget, const CSurface &source, const Rect &srect, int x, int y)
{
	SetError(1007,"Engine: %s, Function: blt", (const char*)name());
	return 0;
}

bool GFXEngine::bltAlpha(tk::Widget &widget, const CSurface &source, const Rect &srect, int x, int y)
{
	SetError(1007,"Engine: %s, Function: bltAlpha", (const char*)name());
	return 0;
}

bool GFXEngine::startMessageLoop()
{
	SetError(1007,"Engine: %s, Function: startMessageLoop", (const char*)name());
	return 0;
}

} // EOF namespace grafix
} // EOF namespace ppl6
