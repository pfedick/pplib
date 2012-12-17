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
#ifdef HAVE_SDL12
#include <SDL/SDL.h>
#endif

namespace ppl7 {
namespace tk {

using namespace ppl7;
using namespace ppl7::grafix;

#ifdef OLDCODE

#ifdef HAVE_SDL12
static WindowManager_SDL *wm=NULL;

typedef struct {
	SDL_Surface *surface;

} SDL_WINDOW_PRIVATE;

typedef struct {
	int depth;
	ppluint32 Rmask;
	ppluint32 Gmask;
	ppluint32 Bmask;
	ppluint32 Amask;
} SDL_COLORFORMAT;


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

static SDL_COLORFORMAT RGBFormat2SDL(const RGBFormat &format)
{
	SDL_COLORFORMAT f;
	f.depth=0;
	f.Rmask=0;
	f.Gmask=0;
	f.Bmask=0;
	f.Amask=0;
	switch (format) {
		case RGBFormat::Palette:
			f.depth=8;
			break;
		case RGBFormat::A8R8G8B8:
		case RGBFormat::X8R8G8B8:
			f.depth=32;
			f.Amask=0xff000000;
			f.Rmask=0x00ff0000;
			f.Gmask=0x0000ff00;
			f.Bmask=0x000000ff;
			break;
		case RGBFormat::A8B8G8R8:
		case RGBFormat::X8B8G8R8:
			f.depth=32;
			f.Amask=0xff000000;
			f.Rmask=0x000000ff;
			f.Gmask=0x0000ff00;
			f.Bmask=0x00ff0000;
			break;
		case RGBFormat::R5G6B5:
			f.depth=16;
			f.Amask=0x00000000;
			f.Rmask=0x0000f800;
			f.Gmask=0x000007e0;
			f.Bmask=0x0000001f;
			break;
		case RGBFormat::B5G6R5:
			f.depth=16;
			f.Amask=0x00000000;
			f.Rmask=0x0000001f;
			f.Gmask=0x000007e0;
			f.Bmask=0x0000f800;
			break;
		case RGBFormat::X1R5G5B5:
		case RGBFormat::A1R5G5B5:
			f.depth=16;
			f.Amask=0x00008000;
			f.Rmask=0x00007c00;
			f.Gmask=0x000003e0;
			f.Bmask=0x0000001f;
			break;
		case RGBFormat::X1B5G5R5:
		case RGBFormat::A1B5G5R5:
			f.depth=16;
			f.Amask=0x00008000;
			f.Rmask=0x0000001f;
			f.Gmask=0x000003e0;
			f.Bmask=0x00007c00;
			break;
		case RGBFormat::X4R4G4B4:
		case RGBFormat::A4R4G4B4:
			f.depth=16;
			f.Amask=0x0000f000;
			f.Rmask=0x00000f00;
			f.Gmask=0x000000f0;
			f.Bmask=0x0000000f;
			break;
		case RGBFormat::R8G8B8:
			f.depth=24;
			f.Rmask=0x00ff0000;
			f.Gmask=0x0000ff00;
			f.Bmask=0x000000ff;
			break;
		case RGBFormat::B8G8R8:
			f.depth=24;
			f.Rmask=0x000000ff;
			f.Gmask=0x0000ff00;
			f.Bmask=0x00ff0000;
			break;
		case RGBFormat::R3G3B2:
			f.depth=8;
			f.Rmask=0x00000003;
			f.Gmask=0x0000001c;
			f.Bmask=0x000000e0;
		case RGBFormat::A8R3G3B2:
			f.depth=16;
			f.Amask=0x0000ff00;
			f.Rmask=0x00000003;
			f.Gmask=0x0000001c;
			f.Bmask=0x000000e0;
		default:
			throw UnsupportedColorFormatException();
	}

	return f;
}



WindowManager_SDL::WindowManager_SDL()
{
	pw=NULL;
	if (wm!=NULL && wm!=this) throw DuplicateWindowManagerException();
	wm=this;
	/* Get init data on all the subsystems */
	ppluint32 subsystem_init;
	subsystem_init=SDL_WasInit(SDL_INIT_EVERYTHING);

	if(!(subsystem_init&SDL_INIT_VIDEO)) {
		if (SDL_InitSubSystem(SDL_INIT_VIDEO)<0) throw InitializationException("WindowManager_SDL");
	}
	const SDL_VideoInfo *info=SDL_GetVideoInfo();
	screenRGBFormat=SDL2RGBFormat(info->vfmt);
	screenSize.setSize(info->current_w, info->current_h);
	//printf ("Auflösung: %i x %i\n",screenSize.width,screenSize.height);
}

WindowManager_SDL::~WindowManager_SDL()
{
	if (wm==this) wm=NULL;
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

const Size &WindowManager_SDL::desktopResolution() const
{
	return screenSize;
}

const RGBFormat &WindowManager_SDL::desktopRGBFormat() const
{
	return screenRGBFormat;
}

void WindowManager_SDL::createWindow(Window &w, int width, int height, const RGBFormat &format, int wf)
{
	if (pw!=NULL) {
		throw ExistingPrimaryWindowException();
	}
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)malloc(sizeof(SDL_WINDOW_PRIVATE));
	if (!priv) throw OutOfMemoryException();
	ppluint32 flags=0;
	int bits=screenRGBFormat.bitdepth();
	if (w.rgbFormat()!=RGBFormat::unknown) bits=w.rgbFormat().bitdepth();
	if (wf&Window::NoBorder) flags|=SDL_NOFRAME;
	if (wf&Window::Resizeable) flags|=SDL_RESIZABLE;
	if (wf&Window::TopMost) w.setTopmost(true);
	if (wf&Window::Fullscreen) flags|=SDL_FULLSCREEN;
	if (wf&Window::Backbuffer) flags|=SDL_DOUBLEBUF;
	if (wf&Window::TrippleBuffer) flags|=SDL_DOUBLEBUF;
	if (wf&Window::Hardware) flags|=SDL_HWSURFACE;
	if (wf&Window::VideoMemory) flags|=SDL_HWSURFACE;
	else flags|=SDL_SWSURFACE;
	if (wf&Window::OpenGL) flags|=SDL_OPENGL;

	w.setFlags(wf);
	w.setSize(width, height);
	priv->surface=SDL_SetVideoMode(width, height, bits, flags);
	if (!priv->surface) {
		free(priv);
		throw WindowCreateException();
	}
	w.setPrivateData(priv);
	w.setRGBFormat(SDL2RGBFormat(priv->surface->format));
	pw=&w;
}

void WindowManager_SDL::DispatchEvent(void *e)
{
	SDL_Event *event=(SDL_Event*)e;
	switch (event->type) {
		case SDL_ACTIVEEVENT:
			DispatchSdlActiveEvent(event);
			break;
		case SDL_KEYDOWN:
			printf ("SDL_KEYDOWN\n");
			DispatchSdlKeyEvent(event);
			break;
		case SDL_KEYUP:
			printf ("SDL_KEYUP\n");
			DispatchSdlKeyEvent(event);
			break;
		case SDL_MOUSEMOTION:
			//printf ("SDL_MOUSEMOTION\n");
			DispatchSdlMouseEvent(event);
			break;
		case SDL_MOUSEBUTTONDOWN:
			//printf ("SDL_MOUSEBUTTONDOWN\n");
			DispatchSdlMouseEvent(event);
			break;
		case SDL_MOUSEBUTTONUP:
			//printf ("SDL_MOUSEBUTTONUP\n");
			DispatchSdlMouseEvent(event);
			break;
		case SDL_QUIT:
			printf ("SDL_QUIT\n");
			return;
		case SDL_SYSWMEVENT:
			printf ("SDL_SYSWMEVENT\n");
			break;
		case SDL_VIDEORESIZE:
			DispatchSdlResizeEvent(event);
			break;
		case SDL_USEREVENT:
			printf ("SDL_USEREVENT\n");
			break;
		default:
			printf ("UNKNOWN EVENT\n");
			break;
	}
}

void WindowManager_SDL::startEventLoop()
{
	if (!pw) throw NoWindowException();
	SDL_Event event;
	drawIfNeeded();
	while (1) {
		if (SDL_WaitEvent(&event)==0) throw EventLoopException();
		if (event.type==SDL_QUIT) return;
		DispatchEvent(&event);
		drawIfNeeded();
	}
}

/*!\brief Ausstehende Events abarbeiten
 *
 * \desc
 * Diese Funktion arbeitet alle ausstehenden Ereignisse ab.
 *
 * \return
 * Die Funktion gibt 1 zurück, wenn alle Ereignisse verarbeitet wurden,
 * 0, wenn die Anwendung geschlossen werden soll.
 */
int WindowManager_SDL::handleEvents()
{
	if (!pw) throw NoWindowException();
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type==SDL_QUIT) return 0;
		DispatchEvent(&event);
	}
	return 1;
}

void WindowManager_SDL::drawIfNeeded() const
{
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)pw->getPrivateData();
	if (!priv) return;

	if (pw->redrawRequired()==true) {
		SDL_LockSurface(priv->surface);
		ppl7::grafix::Drawable draw;
		draw.create(priv->surface->pixels,priv->surface->pitch,priv->surface->w, priv->surface->h,pw->rgbFormat());
		pw->draw(draw);
		SDL_UnlockSurface(priv->surface);
		SDL_Flip(priv->surface);
	}

}

void WindowManager_SDL::draw() const
{
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)pw->getPrivateData();
	if (!priv) return;
	SDL_LockSurface(priv->surface);
	ppl7::grafix::Drawable draw;
	draw.create(priv->surface->pixels,priv->surface->pitch,priv->surface->w, priv->surface->h,pw->rgbFormat());
	//printf ("WindowManager_SDL::draw\n");
	pw->draw(draw);
	SDL_UnlockSurface(priv->surface);
	SDL_Flip(priv->surface);
}


void WindowManager_SDL::DispatchSdlActiveEvent(void *e)
{
	if (!pw) return;
	//printf ("DispatchSdlActiveEvent\n");
	SDL_ActiveEvent *event=(SDL_ActiveEvent*)e;

	if (event->state==SDL_APPMOUSEFOCUS) {
		// SDL_APPMOUSEFOCUS if mouse focus was gained or lost
		if (event->gain==0) {
			Event ev(Event::MouseLeave);
			dispatchEvent(pw,ev);
		} else if (event->gain==1) {
			Event ev(Event::MouseEnter);
			dispatchEvent(pw,ev);
		}
	} else if (event->state==SDL_APPINPUTFOCUS) {
		// SDL_APPINPUTFOCUS if input focus was gained or lost
	} else if (event->state==SDL_APPACTIVE) {
		// SDL_APPACTIVE if the application was iconified (gain=0) or restored(gain=1)
		if (event->gain==0) {
			Event ev(Event::Close);
			dispatchEvent(pw,ev);
		} else if (event->gain==1) {
			Event ev(Event::Show);
			dispatchEvent(pw,ev);
		}
	}
}

void WindowManager_SDL::DispatchSdlKeyEvent(void *e)
{
	printf ("DispatchSdlKeyEvent\n");
	SDL_Event *event=(SDL_Event*)e;
}

void WindowManager_SDL::DispatchSdlMouseEvent(void *e)
{
	MouseEvent ev;
	ppluint8 type=((SDL_Event*)e)->type;
	if (type==SDL_MOUSEMOTION) {
		SDL_MouseMotionEvent *event=(SDL_MouseMotionEvent*)e;
		ev.setType(Event::MouseMove);
		ev.p.x=event->x;
		ev.p.y=event->y;
		ev.buttonMask=(MouseEvent::MouseButton)0;
		ev.button=(MouseEvent::MouseButton)0;
		if (event->state&1) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::Left);
		if (event->state&2) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::Middle);
		if (event->state&4) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::Right);
		if (event->state&8) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::WheelUp);
		if (event->state&16) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::WheelDown);
		if (event->state&32) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::X1);
		if (event->state&64) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::X2);
		dispatchEvent(pw,ev);

	} else if (type==SDL_MOUSEBUTTONDOWN) {
		SDL_MouseButtonEvent *event=(SDL_MouseButtonEvent*)e;
		ev.setType(Event::MouseDown);
		ev.p.x=event->x;
		ev.p.y=event->y;
		ppluint8 state=SDL_GetMouseState(NULL,NULL);
		if (state&1) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::Left);
		if (state&2) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::Middle);
		if (state&4) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::Right);
		if (state&8) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::WheelUp);
		if (state&16) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::WheelDown);
		if (state&32) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::X1);
		if (state&64) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::X2);
		if (event->button==1) ev.button=MouseEvent::Left;
		if (event->button==2) ev.button=MouseEvent::Middle;
		if (event->button==3) ev.button=MouseEvent::Right;
		if (event->button==4) ev.button=MouseEvent::WheelUp;
		if (event->button==5) ev.button=MouseEvent::WheelDown;
		if (event->button==6) ev.button=MouseEvent::X1;
		if (event->button==7) ev.button=MouseEvent::X2;
		dispatchEvent(pw,ev);
	} else if (type==SDL_MOUSEBUTTONUP) {
		SDL_MouseButtonEvent *event=(SDL_MouseButtonEvent*)e;
		ev.setType(Event::MouseUp);
		ev.p.x=event->x;
		ev.p.y=event->y;
		ppluint8 state=SDL_GetMouseState(NULL,NULL);
		if (state&1) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::Left);
		if (state&2) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::Middle);
		if (state&4) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::Right);
		if (state&8) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::WheelUp);
		if (state&16) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::WheelDown);
		if (state&32) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::X1);
		if (state&64) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::X2);
		if (event->button==1) ev.button=MouseEvent::Left;
		if (event->button==2) ev.button=MouseEvent::Middle;
		if (event->button==3) ev.button=MouseEvent::Right;
		if (event->button==4) ev.button=MouseEvent::WheelUp;
		if (event->button==5) ev.button=MouseEvent::WheelDown;
		if (event->button==6) ev.button=MouseEvent::X1;
		if (event->button==7) ev.button=MouseEvent::X2;
		dispatchEvent(pw,ev);
	}
}

void WindowManager_SDL::DispatchSdlResizeEvent(void *e)
{
	ResizeEvent ev;
	ev.setType(Event::Resize);
	SDL_ResizeEvent *event=(SDL_ResizeEvent*)e;
	ev.width=event->w;
	ev.height=event->h;
	dispatchEvent(pw,ev);
}


void WindowManager_SDL::getMouseStatus(Point &p, int &buttonMask)
{
	ppluint8 state=SDL_GetMouseState(&p.x,&p.y);
	buttonMask=0;
	if (state&1) buttonMask=(MouseEvent::MouseButton)(buttonMask|MouseEvent::Left);
	if (state&2) buttonMask=(MouseEvent::MouseButton)(buttonMask|MouseEvent::Middle);
	if (state&4) buttonMask=(MouseEvent::MouseButton)(buttonMask|MouseEvent::Right);
	if (state&8) buttonMask=(MouseEvent::MouseButton)(buttonMask|MouseEvent::WheelUp);
	if (state&16) buttonMask=(MouseEvent::MouseButton)(buttonMask|MouseEvent::WheelDown);
	if (state&32) buttonMask=(MouseEvent::MouseButton)(buttonMask|MouseEvent::X1);
	if (state&64) buttonMask=(MouseEvent::MouseButton)(buttonMask|MouseEvent::X2);
}



/*************************************************************************************
 * SURFACE-Funktionen
 *************************************************************************************/
typedef struct {
	SDL_Surface *surface;
} SDL_SURFACE_DATA;

static void sdlSurfaceDestroy (void *data)
{
	if (!data) throw NullPointerException();
	SDL_SURFACE_DATA *ssd=(SDL_SURFACE_DATA*)data;
	if (ssd->surface) {
		SDL_FreeSurface(ssd->surface);
	}

}

static void sdlSurfaceLock (void *data, Drawable &draw)
{
	if (!data) throw NullPointerException();
	SDL_SURFACE_DATA *ssd=(SDL_SURFACE_DATA*)data;

}

static void sdlSurfaceUnlock (void *data)
{
	if (!data) throw NullPointerException();
	SDL_SURFACE_DATA *ssd=(SDL_SURFACE_DATA*)data;

}

static void sdlSurfaceDraw (void *target, void *data, int x, int y)
{
	if (!data) throw NullPointerException();
	if (!target) throw NullPointerException();
	SDL_SURFACE_DATA *ssd=(SDL_SURFACE_DATA*)data;
	SDL_SURFACE_DATA *ssdt=(SDL_SURFACE_DATA*)target;
	printf ("sdlSurfaceDraw\n");

}

void WindowManager_SDL::createSurface(Widget &w, int width, int height, const RGBFormat &format, int sf)
{
	Surface *s=new Surface;
	if (!s) throw OutOfMemoryException();
	s->setLockFunction(sdlSurfaceLock);
	s->setUnlockFunction(sdlSurfaceUnlock);
	s->setDestroyFunction(sdlSurfaceDestroy);
	s->setDrawFunction(sdlSurfaceDraw);
	s->setFlags((Surface::SurfaceFlags)sf);

	SDL_SURFACE_DATA *data=(SDL_SURFACE_DATA*)malloc(sizeof(SDL_SURFACE_DATA));
	if (!data) {
		delete s;
		throw OutOfMemoryException();
	}
	s->setPrivateData(data);
	int flags=0;
	if (sf&Surface::VideoMemory) flags|=SDL_HWSURFACE; else flags|=SDL_SWSURFACE;
	RGBFormat rgb=format;
	if (rgb==RGBFormat::unknown) {
		if (pw) rgb=pw->rgbFormat();
		else rgb=screenRGBFormat;
	}
	s->setRGBFormat(rgb);
	s->setSize(width,height);
	SDL_COLORFORMAT cf=RGBFormat2SDL(rgb);
	data->surface=SDL_CreateRGBSurface(flags, width, height, cf.depth, cf.Rmask, cf.Gmask, cf.Bmask, cf.Amask);
	if (!data->surface) {
		delete s;
		throw SurfaceCreateException();
	}
	w.setSurface(s);
	w.setSize(width,height);
}

#endif

#endif	// OLDCODE

}	// EOF namespace tk
}	// EOF namespace ppl7

