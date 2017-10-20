/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
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
#ifdef HAVE_SDL2
#include <SDL.h>
#endif

namespace ppl7 {
namespace tk {

using namespace ppl7;
using namespace ppl7::grafix;

#ifdef HAVE_SDL2
static WindowManager_SDL2 *wm=NULL;

typedef struct {
	SDL_DisplayMode *mode;
	SDL_Window *win;
	SDL_Renderer *renderer;
	SDL_Texture *gui;
	int width, height;
	RGBFormat format;
} SDL_WINDOW_PRIVATE;


typedef struct {
	Window *win;
	SDL_Texture *tex;
} SDL_SURFACE_PRIVATE;


typedef struct {
	int depth;
	ppluint32 Rmask;
	ppluint32 Gmask;
	ppluint32 Bmask;
	ppluint32 Amask;
} SDL_COLORFORMAT;

static RGBFormat SDL2RGBFormat(const Uint32 f)
{
	switch (f) {
		case SDL_PIXELFORMAT_INDEX8: return RGBFormat::Palette;
		case SDL_PIXELFORMAT_RGB332: return RGBFormat::R3G3B2;
		case SDL_PIXELFORMAT_RGB444: return RGBFormat::X4R4G4B4;
		case SDL_PIXELFORMAT_RGB555: return RGBFormat::X1R5G5B5;
		case SDL_PIXELFORMAT_BGR555: return RGBFormat::X1B5G5R5;
		case SDL_PIXELFORMAT_ARGB4444: return RGBFormat::A4R4G4B4;
		case SDL_PIXELFORMAT_ABGR4444: return RGBFormat::A4B4G4R4;
		case SDL_PIXELFORMAT_ARGB1555: return RGBFormat::A1R5G5B5;
		case SDL_PIXELFORMAT_ABGR1555: return RGBFormat::A1B5G5R5;
		case SDL_PIXELFORMAT_RGB565: return RGBFormat::R5G6B5;
		case SDL_PIXELFORMAT_BGR565: return RGBFormat::B5G6R5;
		case SDL_PIXELFORMAT_RGB24: return RGBFormat::R8G8B8;
		case SDL_PIXELFORMAT_BGR24: return RGBFormat::B8G8R8;
		case SDL_PIXELFORMAT_RGB888: return RGBFormat::X8R8G8B8;
		case SDL_PIXELFORMAT_BGR888: return RGBFormat::X8B8G8R8;
		case SDL_PIXELFORMAT_ARGB8888: return RGBFormat::A8R8G8B8;
		case SDL_PIXELFORMAT_ABGR8888: return RGBFormat::A8B8G8R8;
	}

	switch (f) {
		case SDL_PIXELFORMAT_RGB888: printf ("SDL_PIXELFORMAT_RGB888\n"); break;
		case SDL_PIXELFORMAT_RGBX8888: printf ("SDL_PIXELFORMAT_RGBX8888\n"); break;
		case SDL_PIXELFORMAT_BGR888: printf ("SDL_PIXELFORMAT_BGR888\n"); break;
		case SDL_PIXELFORMAT_BGRX8888: printf ("SDL_PIXELFORMAT_BGRX8888\n"); break;
		case SDL_PIXELFORMAT_ARGB8888: printf ("SDL_PIXELFORMAT_ARGB8888\n"); break;
		case SDL_PIXELFORMAT_RGBA8888: printf ("SDL_PIXELFORMAT_RGBA8888\n"); break;
		case SDL_PIXELFORMAT_ABGR8888: printf ("SDL_PIXELFORMAT_ABGR8888\n"); break;
		case SDL_PIXELFORMAT_BGRA8888: printf ("SDL_PIXELFORMAT_BGRA8888\n"); break;
		case SDL_PIXELFORMAT_ARGB2101010: printf ("SDL_PIXELFORMAT_ARGB2101010\n"); break;
		default: printf ("Verdammt!\n"); break;

	}

	throw UnsupportedColorFormatException("format=%d",f);
}


static SDL_COLORFORMAT RGBFormat2SDLStruct(const RGBFormat &format)
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
			break;
		case RGBFormat::A8R3G3B2:
			f.depth=16;
			f.Amask=0x0000ff00;
			f.Rmask=0x00000003;
			f.Gmask=0x0000001c;
			f.Bmask=0x000000e0;
			break;
		default:
			throw UnsupportedColorFormatException();
	}

	return f;
}



static void sdlSetWindowTitle (void *privatedata, const String &Title)
{
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) return;
	SDL_SetWindowTitle(priv->win,(const char*)Title);

}

static void sdlSetWindowIcon (void *privatedata, const Drawable &Icon)
{
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) return;
	if (Icon.isEmpty()) {
		SDL_SetWindowIcon(priv->win, NULL);
	} else {
		SDL_COLORFORMAT cf=RGBFormat2SDLStruct(Icon.rgbformat());

		SDL_Surface* s=SDL_CreateRGBSurfaceFrom (Icon.adr(),
				Icon.width(),
				Icon.height(),
				Icon.bitdepth(),
				Icon.pitch(),
				cf.Rmask, cf.Gmask, cf.Bmask, cf.Amask);
		if (!s) throw SurfaceCreateException("sdlSetWindowIcon ERROR: %s",SDL_GetError());
		SDL_SetWindowIcon(priv->win,s);
		SDL_FreeSurface(s);
	}
}

static void sdlCreateSurface (void *privatedata)
{

}

static void sdlCreateTexture (void *privatedata)
{

}

static Drawable sdlLockWindowSurface (void *privatedata)
{
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) throw NullPointerException();
	void *pixels;
	int pitch;
	SDL_LockTexture(priv->gui,NULL,&pixels,&pitch);
	return ppl7::grafix::Drawable(pixels,pitch,priv->width,priv->height,priv->format);
}

static void sdlUnlockWindowSurface (void *privatedata)
{
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) throw NullPointerException();
	SDL_UnlockTexture(priv->gui);
}

static void sdlDrawWindowSurface (void *privatedata)
{
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) throw NullPointerException();
	SDL_RenderCopy(priv->renderer,priv->gui,NULL,NULL);
}

static void *sdlGetRenderer (void *privatedata)
{
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) throw NullPointerException();
	return priv->renderer;
}

static void sdlClearScreen (void *privatedata)
{
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) throw NullPointerException();
	SDL_RenderClear(priv->renderer);
}

static void sdlPresentScreen (void *privatedata)
{
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) throw NullPointerException();
	SDL_RenderPresent(priv->renderer);
}


static PRIV_WINDOW_FUNCTIONS sdlWmFunctions = {
		sdlSetWindowTitle,
		sdlSetWindowIcon,
		sdlCreateSurface,
		sdlCreateTexture,
		sdlLockWindowSurface,
		sdlUnlockWindowSurface,
		sdlDrawWindowSurface,
		sdlGetRenderer,
		sdlClearScreen,
		sdlPresentScreen
};

#endif

WindowManager_SDL2::WindowManager_SDL2()
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	if (wm!=NULL && wm!=this) throw DuplicateWindowManagerException();
	wm=this;
	/* Get init data on all the subsystems */
	ppluint32 subsystem_init;
	subsystem_init=SDL_WasInit(SDL_INIT_EVERYTHING);

	if(!(subsystem_init&SDL_INIT_VIDEO)) {
		if (SDL_InitSubSystem(SDL_INIT_VIDEO)<0) throw InitializationException("WindowManager_SDL2: SDL_INIT_VIDEO");
	}
	if(!(subsystem_init&SDL_INIT_TIMER)) {
		if (SDL_InitSubSystem(SDL_INIT_TIMER)<0) throw InitializationException("WindowManager_SDL2: SDL_INIT_TIMER");
	}

	atexit(SDL_Quit);

	SDL_DisplayMode current;
	memset(&current,0,sizeof(SDL_DisplayMode));
	current.driverdata=0;
	if (SDL_GetDesktopDisplayMode(0, &current)!=0) {
		throw InitializationException("SDL-ERROR: %s\n",SDL_GetError());
	}
	screenRGBFormat=SDL2RGBFormat(current.format);
	screenSize.setSize(current.w, current.h);
	screenRefreshRate=current.refresh_rate;
	//printf ("Auflösung: %i x %i, Format: %s, Refresh: %i\n",screenSize.width,screenSize.height,(const char*)screenRGBFormat.name(),screenRefreshRate);
#endif
}

WindowManager_SDL2::~WindowManager_SDL2()
{
#ifdef HAVE_SDL2
	if (wm==this) wm=NULL;
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
}

const Size &WindowManager_SDL2::desktopResolution() const
{
	return screenSize;
}

const RGBFormat &WindowManager_SDL2::desktopRGBFormat() const
{
	return screenRGBFormat;
}

void WindowManager_SDL2::createWindow(Window &w)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	if (w.getPrivateData()!=NULL) throw WindowAlreadyCreatedException();

	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)calloc(sizeof(SDL_WINDOW_PRIVATE),1);
	if (!priv) throw OutOfMemoryException();
	int flags=SDL_WINDOW_SHOWN;
	ppluint32 wf=w.flags();
	if (wf&Window::NoBorder) flags|=SDL_WINDOW_BORDERLESS;
	if (wf&Window::Resizeable) flags|=SDL_WINDOW_RESIZABLE;
	if (wf&Window::Maximized) flags|=SDL_WINDOW_MAXIMIZED;
	if (wf&Window::Minimized) flags|=SDL_WINDOW_MINIMIZED;
	if (wf&Window::Fullscreen) flags|=SDL_WINDOW_FULLSCREEN;
	if (wf&Window::OpenGL) flags|=SDL_WINDOW_OPENGL;


	priv->win=SDL_CreateWindow((const char*)w.windowTitle(),
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			w.width(), w.height(), flags);
	if (priv->win==0) {
		free(priv);
		throw WindowCreateException("SDL_CreateWindow ERROR: %s",SDL_GetError());
	}

	/*
	if (wf&Window::Fullscreen) {
		if (SDL_SetWindowFullscreen(priv->win, SDL_TRUE)!=0) {
			const char *e=SDL_GetError();
			SDL_DestroyWindow(priv->win);
			free(priv);
			throw WindowCreateException("SDL_SetWindowFullscreen ERROR: %s",e);
		}
	}
	*/

	SDL_SetWindowData(priv->win,"WindowClass",&w);
	flags=SDL_RENDERER_ACCELERATED;
	if (wf&Window::WaitVsync) flags|=SDL_RENDERER_PRESENTVSYNC;
	priv->renderer = SDL_CreateRenderer(priv->win, -1, flags);
	if (priv->renderer==0) {
		const char *e=SDL_GetError();
		SDL_DestroyWindow(priv->win);
		free(priv);
		throw WindowCreateException("SDL_CreateWindow ERROR: %s",e);
	}
	int width,height;
	SDL_GetWindowSize(priv->win,&width,&height);
	w.setSize(width,height);

	priv->gui=SDL_CreateTexture(priv->renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,w.width(),w.height());
    if (priv->gui==0) {
		const char *e=SDL_GetError();
		SDL_DestroyRenderer(priv->renderer);
		SDL_DestroyWindow(priv->win);
		free(priv);
		throw WindowCreateException("SDL_CreateWindow ERROR: %s",e);

    }
    if (SDL_SetTextureBlendMode(priv->gui,SDL_BLENDMODE_BLEND)!=0) {
		const char *e=SDL_GetError();
		SDL_DestroyTexture(priv->gui);
		SDL_DestroyRenderer(priv->renderer);
		SDL_DestroyWindow(priv->win);
		free(priv);
		throw WindowCreateException("SDL_SetTextureBlendMode ERROR: %s",e);
    }

    priv->format=RGBFormat::A8R8G8B8;
    priv->width=w.width();
    priv->height=w.height();
	w.setPrivateData(priv, this,&sdlWmFunctions);
	sdlSetWindowIcon(priv,w.windowIcon());
	windows.add(&w);
#endif
}

void WindowManager_SDL2::destroyWindow(Window &w)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)w.getPrivateData();
	if (!priv) return;
	windows.erase(&w);
	if (priv->gui) SDL_DestroyTexture(priv->gui);
	if (priv->renderer) SDL_DestroyRenderer(priv->renderer);
	if (priv->win) SDL_DestroyWindow(priv->win);
	free(priv);
	w.setPrivateData(NULL,NULL,NULL);
#endif
}

size_t WindowManager_SDL2::numWindows()
{
	return windows.size();
}

void WindowManager_SDL2::getMouseState(Point &p, int &buttonMask)
{

}

MouseState WindowManager_SDL2::getMouseState() const
{
	MouseState mouse;
	int buttons;
	SDL_PumpEvents();
	buttons=SDL_GetMouseState(&mouse.p.x, &mouse.p.y);
	return mouse;
}

void WindowManager_SDL2::startEventLoop()
{
	while (numWindows()>0) {
		handleEvents();
		ppl7::MSleep(5);
	}
}

Window *WindowManager_SDL2::getWindow(ppluint32 id)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	SDL_Window* win=SDL_GetWindowFromID(id);
	if (!win) return NULL;
	return (Window*)SDL_GetWindowData(win,"WindowClass");
#endif
}

void WindowManager_SDL2::handleEvents()
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	SDL_Event sdl_event;
	while (SDL_PollEvent(&sdl_event)) {		// Alle Events verarbeiten
		//printf ("event vorhanden: %d\n",sdl_event.type);
		switch (sdl_event.type) {
			case SDL_QUIT:
				DispatchQuitEvent(&sdl_event);
				break;
			case SDL_WINDOWEVENT:
				DispatchWindowEvent(&sdl_event);
				break;
			case SDL_MOUSEMOTION:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				DispatchMouseEvent(&sdl_event);
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				//printf ("SDL_KEYDOWN or SDL_KEYUP Event\n");
				DispatchKeyEvent(&sdl_event);
				break;
			case SDL_TEXTINPUT:
			{
				Widget *keyFocusWidget=getKeyboardFocus();
				if (keyFocusWidget) {
					SDL_TextInputEvent *event=(SDL_TextInputEvent*)&sdl_event;
					TextInputEvent te;
					te.text.set(event->text);
					keyFocusWidget->textInputEvent(&te);

				}

				//printf ("Event: SDL_TEXTINPUT\n");
				break;
			}
			case SDL_TEXTEDITING:
				printf ("Event: SDL_TEXTEDITING\n");
				break;
			case SDL_USEREVENT:

				if (sdl_event.user.code==1) {	// ClickTimer
					//printf ("SDL_USEREVENT 1\n");
					dispatchClickEvent((Window*)sdl_event.user.data1);
				} else if (sdl_event.user.code==2) {	// TimerEvent
					//printf ("SDL_USEREVENT 2\n");
					Event e;
					e.setWidget((Widget*)sdl_event.user.data1);
					((Widget*)sdl_event.user.data1)->timerEvent(&e);
				}

	            break;

		}
	}
#endif
}

void WindowManager_SDL2::DispatchQuitEvent(void *e)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	const SDL_Event * event=(SDL_Event*)e;
	Window *w=getWindow(event->window.windowID);
	if (!w) return;
	Event ev(Event::Quit);
	ev.setWidget(w);
	w->quitEvent(&ev);
#endif
}


void WindowManager_SDL2::DispatchWindowEvent(void *e)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	const SDL_Event * event=(SDL_Event*)e;

	Window *w=getWindow(event->window.windowID);
	if (!w) return;

    switch (event->window.event) {
    case SDL_WINDOWEVENT_SHOWN:
        //printf("Window %d shown", event->window.windowID);
        w->clearScreen();
        w->drawWidgets();
        w->presentScreen();
        break;
    case SDL_WINDOWEVENT_HIDDEN:
        //fprintf(stderr, "Window %d hidden", event->window.windowID);
        break;
    case SDL_WINDOWEVENT_EXPOSED:
        //fprintf(stderr, "Window %d exposed", event->window.windowID);
        break;
    case SDL_WINDOWEVENT_MOVED:
        //fprintf(stderr, "Window %d moved to %d,%d",
        //        event->window.windowID, event->window.data1,
        //        event->window.data2);
        break;
    case SDL_WINDOWEVENT_RESIZED:
        // fprintf(stderr, "Window %d resized to %dx%d",
        //        event->window.windowID, event->window.data1,
        //        event->window.data2);
        break;
    case SDL_WINDOWEVENT_MINIMIZED:
        //fprintf(stderr, "Window %d minimized", event->window.windowID);
        break;
    case SDL_WINDOWEVENT_MAXIMIZED:
        //fprintf(stderr, "Window %d maximized", event->window.windowID);
        break;
    case SDL_WINDOWEVENT_RESTORED:
        //fprintf(stderr, "Window %d restored", event->window.windowID);
        break;
    case SDL_WINDOWEVENT_ENTER:
        //fprintf(stderr, "Mouse entered window %d",
        //        event->window.windowID);
        break;
    case SDL_WINDOWEVENT_LEAVE:
        //fprintf(stderr, "Mouse left window %d", event->window.windowID);
        break;
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        //fprintf(stderr, "Window %d gained keyboard focus",
        //        event->window.windowID);
        break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
        //fprintf(stderr, "Window %d lost keyboard focus",
        //        event->window.windowID);
        break;
    case SDL_WINDOWEVENT_CLOSE:
    	{
    		Event e;
    		e.setWidget(w);
    		w->closeEvent(&e);
    	}
        break;
    default:
        printf("SDL Window %d got unknown event %d\n",
                event->window.windowID, event->window.event);
        break;
    }
#endif
}


void WindowManager_SDL2::DispatchMouseEvent(void *e)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	MouseEvent ev;
	Uint32 type=((SDL_Event*)e)->type;

	if (type==SDL_MOUSEMOTION) {
		SDL_MouseMotionEvent *event=(SDL_MouseMotionEvent*)e;

		Window *w=getWindow(event->windowID);
		if (!w) return;


		ev.setType(Event::MouseMove);
		ev.p.x=event->x;
		ev.p.y=event->y;
		ev.buttonMask=(MouseEvent::MouseButton)0;
		ev.button=(MouseEvent::MouseButton)0;
		ppluint8 state=event->state;
		if (state&1) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::Left);
		if (state&2) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::Middle);
		if (state&4) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::Right);
		if (state&8) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::WheelUp);
		if (state&16) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::WheelDown);
		if (state&32) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::X1);
		if (state&64) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask|MouseEvent::X2);
		dispatchEvent(w,ev);

	} else if (type==SDL_MOUSEBUTTONDOWN) {
		SDL_MouseButtonEvent *event=(SDL_MouseButtonEvent*)e;

		Window *w=getWindow(event->windowID);
		if (!w) return;

		ev.setType(Event::MouseDown);
		ev.p.x=event->x;
		ev.p.y=event->y;
		ev.buttonMask=(MouseEvent::MouseButton)0;
		ev.button=(MouseEvent::MouseButton)0;
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
		dispatchEvent(w,ev);
	} else if (type==SDL_MOUSEBUTTONUP) {
		SDL_MouseButtonEvent *event=(SDL_MouseButtonEvent*)e;

		Window *w=getWindow(event->windowID);
		if (!w) return;

		ev.setType(Event::MouseUp);
		ev.p.x=event->x;
		ev.p.y=event->y;
		ev.buttonMask=(MouseEvent::MouseButton)0;
		ev.button=(MouseEvent::MouseButton)0;
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
		dispatchEvent(w,ev);
	}
#endif
}

typedef struct {

} CLICK_EVENT;

#ifdef HAVE_SDL2
static Uint32 clickTimer(Uint32 interval, void *param)
{
	//printf("clickTimer\n");
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = SDL_USEREVENT;
    userevent.code = 1;
    userevent.data1 = param;
    userevent.data2 = NULL;
    userevent.windowID=0;
    userevent.timestamp=0;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return(0);
}
#endif

void WindowManager_SDL2::startClickEvent(Window *win)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	//printf ("WindowManager_SDL2::startClickEvent\n");
	SDL_AddTimer(getDoubleClickIntervall(), clickTimer, win);
#endif
}


typedef struct {
		int (*timercallback)(void *data);
		void *data;
		int intervall;
} TIMER_EVENT;

#ifdef HAVE_SDL2
static Uint32 timerEvent(Uint32 interval, void *param)
{
	//printf("clickTimer\n");
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = SDL_USEREVENT;
    userevent.code = 2;
    userevent.data1 = param;
    userevent.data2 = (void*)interval;
    userevent.windowID=0;
    userevent.timestamp=0;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return(interval);
}
#endif

void WindowManager_SDL2::startTimer(Widget *w, int intervall)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	SDL_AddTimer(intervall,timerEvent,w);
#endif
}


void WindowManager_SDL2::DispatchKeyEvent(void *e)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	KeyEvent ev;
	SDL_KeyboardEvent *event=(SDL_KeyboardEvent*)e;

	//WideString st;
	//st.set((wchar_t)event->keysym.sym);
	//printf ("KeyEvent: State: %i, Repeat: %i, ",event->state,event->repeat);
	//printf ("Scancode: %i, Keycode: %i, Modifier: %i\n",event->keysym.scancode, event->keysym.sym, event->keysym.mod);
	//st.printnl();
	Widget *keyFocusWidget=getKeyboardFocus();
	if (!keyFocusWidget) return;

	KeyEvent kev;
	kev.setWidget(keyFocusWidget);
	kev.repeat=(bool)event->repeat;

	switch (event->keysym.sym) {
		case SDLK_UNKNOWN: kev.key=KeyEvent::KEY_UNKNOWN; break;
		case SDLK_RETURN: kev.key=KeyEvent::KEY_RETURN; break;
		case SDLK_ESCAPE: kev.key=KeyEvent::KEY_ESCAPE; break;
		case SDLK_BACKSPACE: kev.key=KeyEvent::KEY_BACKSPACE; break;
		case SDLK_TAB: kev.key=KeyEvent::KEY_TAB; break;
		case SDLK_SPACE: kev.key=KeyEvent::KEY_SPACE; break;
		case SDLK_CAPSLOCK: kev.key=KeyEvent::KEY_CAPSLOCK; break;
		case SDLK_F1: kev.key=KeyEvent::KEY_F1; break;
		case SDLK_F2: kev.key=KeyEvent::KEY_F2; break;
		case SDLK_F3: kev.key=KeyEvent::KEY_F3; break;
		case SDLK_F4: kev.key=KeyEvent::KEY_F4; break;
		case SDLK_F5: kev.key=KeyEvent::KEY_F5; break;
		case SDLK_F6: kev.key=KeyEvent::KEY_F6; break;
		case SDLK_F7: kev.key=KeyEvent::KEY_F7; break;
		case SDLK_F8: kev.key=KeyEvent::KEY_F8; break;
		case SDLK_F9: kev.key=KeyEvent::KEY_F9; break;
		case SDLK_F10: kev.key=KeyEvent::KEY_F10; break;
		case SDLK_F11: kev.key=KeyEvent::KEY_F11; break;
		case SDLK_F12: kev.key=KeyEvent::KEY_F12; break;
		case SDLK_PRINTSCREEN: kev.key=KeyEvent::KEY_PRINTSCREEN; break;
		case SDLK_SCROLLLOCK: kev.key=KeyEvent::KEY_SCROLLLOCK; break;
		case SDLK_PAUSE: kev.key=KeyEvent::KEY_PAUSE; break;
		case SDLK_INSERT: kev.key=KeyEvent::KEY_INSERT; break;
		case SDLK_HOME: kev.key=KeyEvent::KEY_HOME; break;
		case SDLK_PAGEUP: kev.key=KeyEvent::KEY_PAGEUP; break;
		case SDLK_DELETE: kev.key=KeyEvent::KEY_DELETE; break;
		case SDLK_END: kev.key=KeyEvent::KEY_END; break;
		case SDLK_PAGEDOWN: kev.key=KeyEvent::KEY_PAGEDOWN; break;
		case SDLK_RIGHT: kev.key=KeyEvent::KEY_RIGHT; break;
		case SDLK_LEFT: kev.key=KeyEvent::KEY_LEFT; break;
		case SDLK_DOWN: kev.key=KeyEvent::KEY_DOWN; break;
		case SDLK_UP: kev.key=KeyEvent::KEY_UP; break;
		case SDLK_LCTRL: kev.key=KeyEvent::KEY_LEFTCTRL; break;
		case SDLK_LSHIFT: kev.key=KeyEvent::KEY_LEFTSHIFT; break;
		case SDLK_LALT: kev.key=KeyEvent::KEY_LEFTALT; break;
		case SDLK_LGUI: kev.key=KeyEvent::KEY_LEFTGUI; break;
		case SDLK_RCTRL: kev.key=KeyEvent::KEY_RIGHTCTRL; break;
		case SDLK_RSHIFT: kev.key=KeyEvent::KEY_RIGHTSHIFT; break;
		case SDLK_RALT: kev.key=KeyEvent::KEY_RIGHTALT; break;
		case SDLK_RGUI: kev.key=KeyEvent::KEY_RIGHTGUI; break;
		case SDLK_KP_ENTER: kev.key=KeyEvent::KEY_ENTER; break;
		case SDLK_NUMLOCKCLEAR: kev.key=KeyEvent::KEY_NUMLOCK; break;
		case SDLK_MODE: kev.key=KeyEvent::KEY_MODE; break;


		default: kev.key=KeyEvent::KEY_UNKNOWN; break;
	}
	kev.modifier=KeyEvent::KEYMOD_NONE;
	if (event->keysym.mod & KMOD_LSHIFT) kev.modifier|=KeyEvent::KEYMOD_LEFTSHIFT;
	if (event->keysym.mod & KMOD_RSHIFT) kev.modifier|=KeyEvent::KEYMOD_RIGHTSHIFT;
	if (event->keysym.mod & KMOD_LCTRL) kev.modifier|=KeyEvent::KEYMOD_LEFTCTRL;
	if (event->keysym.mod & KMOD_RCTRL) kev.modifier|=KeyEvent::KEYMOD_RIGHTCTRL;
	if (event->keysym.mod & KMOD_LALT) kev.modifier|=KeyEvent::KEYMOD_LEFTALT;
	if (event->keysym.mod & KMOD_RALT) kev.modifier|=KeyEvent::KEYMOD_RIGHTALT;
	if (event->keysym.mod & KMOD_LGUI) kev.modifier|=KeyEvent::KEYMOD_LEFTGUI;
	if (event->keysym.mod & KMOD_RGUI) kev.modifier|=KeyEvent::KEYMOD_RIGHTGUI;
	if (event->keysym.mod & KMOD_NUM) kev.modifier|=KeyEvent::KEYMOD_NUM;
	if (event->keysym.mod & KMOD_CAPS) kev.modifier|=KeyEvent::KEYMOD_CAPS;
	if (event->keysym.mod & KMOD_MODE) kev.modifier|=KeyEvent::KEYMOD_MODE;


	if (kev.key==KeyEvent::KEY_UNKNOWN) return;

	if (event->type==SDL_KEYDOWN) {
		keyFocusWidget->keyDownEvent(&kev);

	} else if (event->type==SDL_KEYUP) {
		keyFocusWidget->keyUpEvent(&kev);
	}
#endif
}


#ifdef OLDCODE
static void surfaceLock (void *privatedata, Drawable &draw) {};
static void surfaceUnlock (void *privatedata) {};
static void surfaceDestroy (void *privatedata) {};
static void surfaceUpdate (void *privatedata, const Drawable &source) {};

static void textureLock (void *privatedata, Drawable &draw) {};
static void textureUnlock (void *privatedata) {};
static void textureDestroy (void *privatedata) {};
static void textureUpdate (void *privatedata, const Drawable &source) {};


static Uint32 RGBFormat2SDL(const RGBFormat &format)
{
	switch (format) {
		case RGBFormat::Palette: return SDL_PIXELFORMAT_INDEX8;
		case RGBFormat::R3G3B2: return 0;

		case RGBFormat::R5G6B5: return SDL_PIXELFORMAT_RGB565;
		case RGBFormat::B5G6R5: return SDL_PIXELFORMAT_BGR565;
		case RGBFormat::X1R5G5B5: return SDL_PIXELFORMAT_RGB555;
		case RGBFormat::X1B5G5R5: return SDL_PIXELFORMAT_BGR555;
		case RGBFormat::A1R5G5B5: return SDL_PIXELFORMAT_ARGB1555;
		case RGBFormat::A1B5G5R5: return SDL_PIXELFORMAT_ABGR1555;
		case RGBFormat::X4R4G4B4: return SDL_PIXELFORMAT_RGB444;
		case RGBFormat::A4B4G4R4: return SDL_PIXELFORMAT_ARGB4444;
		case RGBFormat::A4R4G4B4: return SDL_PIXELFORMAT_ABGR4444;

		case RGBFormat::R8G8B8: return SDL_PIXELFORMAT_RGB24;
		case RGBFormat::B8G8R8: return SDL_PIXELFORMAT_BGR24;

		case RGBFormat::A8R8G8B8: return SDL_PIXELFORMAT_ARGB8888;
		case RGBFormat::A8B8G8R8: return SDL_PIXELFORMAT_ABGR8888;
		case RGBFormat::X8R8G8B8: return SDL_PIXELFORMAT_RGB888;
		case RGBFormat::X8B8G8R8: return SDL_PIXELFORMAT_BGR888;

		default:
			throw UnsupportedColorFormatException();
	}

}


static PRIV_SURFACE_FUNCTIONS sdlSurfaceFunctions = {
		surfaceLock,
		surfaceUnlock,
		surfaceDestroy,
		surfaceUpdate};


static PRIV_SURFACE_FUNCTIONS sdlTextureFunctions = {
		textureLock,
		textureUnlock,
		textureDestroy,
		textureUpdate};


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


#endif	// OLDCODE

}	// EOF namespace tk
}	// EOF namespace ppl7

