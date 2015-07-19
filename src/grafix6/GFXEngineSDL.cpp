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
#ifdef HAVE_MATH_H
#include <math.h>
#endif

#ifdef HAVE_SDL
#include <SDL/SDL.h>
#endif


#include "ppl6-grafix.h"
#include "ppl6-tk.h"

using namespace ppl6;
using namespace ppl6::grafix;
using namespace ppl6::tk;

namespace ppl6 {
namespace grafix {


/*!\class GFXEngineSDL
 * \ingroup PPLGroupGrafik
 * \brief Grafik-Engine auf SDL-Basis
 *
 * \see http://www.libsdl.org
 */

#ifdef HAVE_SDL
typedef struct tagSDLprivate
{
	const SDL_VideoInfo *vinfo;
	Window				*PrimaryWindow;
} SDLprivate;

typedef struct tagSDLsurface
{
	SDL_Surface		*surface;
	ppluint32		flags;
	CDrawable			lock;
	int					lockcount;

} SDLsurface;

static RGBFormat SDL2RGBFormat(const SDL_PixelFormat *f)
{
	if (!f) return RGBFormat::unknown;
	//printf ("Bitdepth: %i, Mask RGBA=",f->BitsPerPixel);
	//printf ("%x:%x:%x:%x\n",f->Rmask,f->Gmask,f->Bmask,f->Amask);

	if (f->BitsPerPixel==32) {
		if (f->Rmask==0xff0000 && f->Gmask==0xff00 && f->Bmask==0xff && f->Amask==0) return RGBFormat::X8R8G8B8;
		if (f->Rmask==0xff0000 && f->Gmask==0xff00 && f->Bmask==0xff && f->Amask==0xff000000) return RGBFormat::A8R8G8B8;
		if (f->Rmask==0xff && f->Gmask==0xff00 && f->Bmask==0xff0000 && f->Amask==0) return RGBFormat::X8B8G8R8;
		if (f->Rmask==0xff && f->Gmask==0xff00 && f->Bmask==0xff0000 && f->Amask==0xff000000) return RGBFormat::A8B8G8R8;
	} else if (f->BitsPerPixel==16 || f->BitsPerPixel==15) {
		// f800:7e0:1f:0
		if (f->Rmask==0xf800 && f->Gmask==0x07e0 && f->Bmask==0x1f && f->Amask==0) return RGBFormat::R5G6B5;
		if (f->Rmask==0x1f && f->Gmask==0x07e0 && f->Bmask==0xf800 && f->Amask==0) return RGBFormat::B5G6R5;
		// 7c00:3e0:1f:0
		if (f->Rmask==0x7c00 && f->Gmask==0x03e0 && f->Bmask==0x1f && f->Amask==0) return RGBFormat::X1R5G5B5;
		if (f->Rmask==0x1f && f->Gmask==0x03e0 && f->Bmask==0x7c00 && f->Amask==0) return RGBFormat::X1B5G5R5;
	} else if (f->BitsPerPixel==8) return RGBFormat::Palette;
	return RGBFormat::unknown;
}
#endif

GFXEngineSDL::GFXEngineSDL()
{
#ifndef HAVE_SDL
	SetError(1049,"DX9");
#else
	priv=NULL;
#endif
}

GFXEngineSDL::~GFXEngineSDL()
{
#ifdef HAVE_SDL
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	ppluint32 subsystem;
	subsystem=SDL_WasInit(SDL_INIT_EVERYTHING);
	if (!subsystem) {
		SDL_Quit();
	}
	if (priv) delete (SDLprivate*)priv;
#endif
}

CString GFXEngineSDL::name() const
{
	return CString("SDL");
}

int GFXEngineSDL::Init()
{
#ifndef HAVE_SDL
	SetError(1049,"SDL");
	return 0;
#else
	if (priv) {
		SetError(1047);
		return 0;
	}
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SetError(1000,"SDL: %s",SDL_GetError());
		return 0;
	}

	priv=new SDLprivate;
	if (!priv) {
		SetError(2);
		return 0;
	}
	((SDLprivate*)priv)->vinfo=SDL_GetVideoInfo();
	((SDLprivate*)priv)->PrimaryWindow=NULL;


	return 1;
#endif
}

bool GFXEngineSDL::isInit() const
{
#ifndef HAVE_SDL
	SetError(1049,"SDL");
	return false;
#else
	if (!priv) {
		SetError(1002,"ppl6::grafix::GFXEngineSDL");
		return false;
	}
	return true;
#endif
}

int GFXEngineSDL::getAdapterCount()
{
#ifndef HAVE_SDL
	SetError(1049,"SDL");
	return 0;
#else
	if (!priv) {
		SetError(1002,"ppl6::grafix::GFXEngineSDL");
		return 0;
	}
	return 1;
#endif
}

int GFXEngineSDL::getAdapterInfo(int adapter, AdapterIdentifier &info)
{
#ifndef HAVE_SDL
	SetError(1049,"SDL");
	return 0;
#else
	if (!priv) {
		SetError(1002,"ppl6::grafix::GFXEngineSDL");
		return 0;
	}
	SDLprivate *p=(SDLprivate*)priv;
	info.adapterId=adapter;
	info.DriverName="";
	info.Description="";
	//SDL_version v;
	const SDL_version *v= SDL_Linked_Version();
	//SDL_GetVersion (&v);

	info.version=(v->major<<16)+(v->minor<<8)+v->patch;
	info.major_version=v->major;
	info.minor_version=v->minor;
	info.sub_version=v->patch;
	info.build=0;
	info.currentWidth=p->vinfo->current_w;
	info.currentHeight=p->vinfo->current_h;
	info.currentRefreshRate=0;
	info.currentRGBFormat=SDL2RGBFormat(p->vinfo->vfmt);

	char myBuffer[256];
	if (SDL_VideoDriverName(myBuffer, 256) != NULL) {
		info.DriverName=myBuffer;
	}
	if (p->vinfo->blit_hw) info.Caps.hwBlit=true;
	if (p->vinfo->blit_fill) info.Caps.hwFill=true;
	if (p->vinfo->blit_hw_CC) info.Caps.hwColorKeyBlit=true;
	if (p->vinfo->blit_hw_A) info.Caps.hwAlphaBlit=true;
	if (p->vinfo->hw_available) info.Caps.hwSurface=true;



	return 1;
#endif

}

/*
int GFXEngineSDL::getAdapterModeCount(int adapter, const RGBFormat &format)
{

}

int GFXEngineSDL::getAdapterModeInfo(int adapter, const RGBFormat &format, int mode, AdapterMode &info)
{

}
*/

bool GFXEngineSDL::createWindow(tk::Widget &widget)
{
#ifndef HAVE_SDL
	SetError(1049,"SDL");
	return false;
#else
	if (!priv) {
		SetError(1002,"ppl6::grafix::GFXEngineSDL");
		return false;
	}
	SDLprivate *p=(SDLprivate*)priv;
	if (!p->vinfo) {
		SetError(1002);
		return false;
	}
	SDLsurface *sdl=(SDLsurface*)calloc(1,sizeof(SDLsurface));
	if (!sdl) {
		SetError(2);
		return false;
	}
	sdl->flags=0;
	int flags=widget.windowFlags();
	//s->bitdepth=p->vinfo->vfmt->BitsPerPixel;
	// Flags initialisieren
	if (flags&Widget::NoBorder) sdl->flags|=SDL_NOFRAME;
	if (flags&Widget::Resizeable) sdl->flags|=SDL_RESIZABLE;
	if (flags&Widget::Hardware) sdl->flags|=SDL_HWSURFACE;
	else sdl->flags|=SDL_SWSURFACE;
	if (flags&Widget::Backbuffer) sdl->flags|=SDL_DOUBLEBUF;
	if (flags&Widget::TrippleBuffer) sdl->flags|=SDL_DOUBLEBUF;
	if (flags&Widget::Fullscreen) sdl->flags|=SDL_FULLSCREEN;

	RGBFormat rgb=widget.rgbFormat();
	int bitdepth=rgb.bitdepth();
	if (rgb==RGBFormat::X1B5G5R5 || rgb==RGBFormat::X1R5G5B5) bitdepth=15;
	sdl->surface=SDL_SetVideoMode(widget.width(), widget.height(), bitdepth, sdl->flags);
	if (!sdl->surface) {
		free(sdl);
		SetError(1008,"Engine: %s, Error: %s",(const char*)name(),SDL_GetError());
		return false;
	}
	SDL_WM_SetCaption(widget.windowTitle(),widget.windowTitle());

	// Farbformat herausfinden
	rgb=SDL2RGBFormat(sdl->surface->format);
	if (rgb==RGBFormat::unknown) {
		// Unbekanntes Farbformat
		free(sdl);
		SetError(1013,"Engine: %s",(const char*)name());
		return false;
	}
	widget.setRGBFormat(rgb);
	widget.setGfxEngine(this);
	widget.setEnginePrivate(sdl);
	return true;
#endif

}

bool GFXEngineSDL::setWindowTitle(tk::Widget &widget)
{
#ifndef HAVE_SDL
	SetError(1049,"SDL");
	return false;
#else
	if (!priv) {
		SetError(1002,"ppl6::grafix::GFXEngineSDL");
		return false;
	}
	SDL_WM_SetCaption(widget.windowTitle(),widget.windowTitle());
	return true;
#endif
}

bool GFXEngineSDL::setWindowIcon(tk::Widget &widget)
{
	return false;
}

bool GFXEngineSDL::lockDevice(tk::Widget &widget, CDrawable &draw)
{
#ifndef HAVE_SDL
	SetError(1049,"SDL");
	return false;
#else
	if (!priv) {
		SetError(1002,"ppl6::grafix::GFXEngineSDL");
		return false;
	}
	SDLsurface *sdl=(SDLsurface*)widget.enginePrivate();
	if (!sdl) {
		SetError(1053);
		return false;
	}
	if (sdl->lockcount>0) {
		sdl->lockcount++;
		draw=sdl->lock;
		return true;
	}
	if (SDL_MUSTLOCK(sdl->surface)!=0) {
		if (SDL_LockSurface(sdl->surface)!=0) {
			SetError(1009,"%s",SDL_GetError());
			return false;
		}
	}
	sdl->lock.create(sdl->surface->pixels,sdl->surface->pitch,sdl->surface->w,sdl->surface->h,widget.rgbFormat());
	sdl->lockcount=1;
	draw=sdl->lock;
	return true;
#endif
}

bool GFXEngineSDL::unlockDevice(tk::Widget &widget)
{
#ifndef HAVE_SDL
	SetError(1049,"SDL");
	return false;
#else
	if (!priv) {
		SetError(1002,"ppl6::grafix::GFXEngineSDL");
		return false;
	}
	SDLsurface *sdl=(SDLsurface*)widget.enginePrivate();
	if (!sdl) {
		SetError(1053);
		return false;
	}
	if (sdl->lockcount>1) {
		sdl->lockcount--;
		return true;
	}
	if (sdl->lockcount==0) return true;
	if (SDL_MUSTLOCK(sdl->surface)!=0) {
		SDL_UnlockSurface(sdl->surface);
	}

	sdl->lockcount=0;
	return true;
#endif
}

bool GFXEngineSDL::updateDevice(tk::Widget &widget)
{
#ifndef HAVE_SDL
	SetError(1049,"SDL");
	return false;
#else
	if (!priv) {
		SetError(1002,"ppl6::grafix::GFXEngineSDL");
		return false;
	}
	SDLsurface *sdl=(SDLsurface*)widget.enginePrivate();
	if (!sdl) {
		SetError(1053);
		return false;
	}
	SDL_Flip(sdl->surface);
	return true;
#endif
}


} // EOF namespace grafix
} // EOF namespace ppl6
