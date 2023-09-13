/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 *******************************************************************************
 * Copyright (c) 2022, Patrick Fedick <patrick@pfp.de>
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

#include "prolog_ppl7.h"

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
static WindowManager_SDL2* wm=NULL;

typedef struct {
	SDL_DisplayMode* mode;
	SDL_Window* win;
	SDL_Renderer* renderer;
	SDL_Texture* gui;
	int width, height;
	RGBFormat format;
} SDL_WINDOW_PRIVATE;


typedef struct {
	Window* win;
	SDL_Texture* tex;
} SDL_SURFACE_PRIVATE;


typedef struct {
	int depth;
	uint32_t Rmask;
	uint32_t Gmask;
	uint32_t Bmask;
	uint32_t Amask;
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
	throw UnsupportedColorFormatException("format=%d", f);
}


static SDL_COLORFORMAT RGBFormat2SDLStruct(const RGBFormat& format)
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

static Uint32 RGBFormat2SDLFormat(const ppl7::grafix::RGBFormat& format)
{
	switch (format) {
	case ppl7::grafix::RGBFormat::Palette:
		return SDL_PIXELFORMAT_INDEX8;
	case ppl7::grafix::RGBFormat::A8R8G8B8:
		return SDL_PIXELFORMAT_ARGB8888;
	case ppl7::grafix::RGBFormat::X8R8G8B8:
		return SDL_PIXELFORMAT_ARGB8888;
	case ppl7::grafix::RGBFormat::A8B8G8R8:
		return SDL_PIXELFORMAT_ABGR8888;
	case ppl7::grafix::RGBFormat::X8B8G8R8:
		return SDL_PIXELFORMAT_ABGR8888;

	case ppl7::grafix::RGBFormat::R8G8B8:
		return SDL_PIXELFORMAT_RGB888;
	case ppl7::grafix::RGBFormat::B8G8R8:
		return SDL_PIXELFORMAT_BGR888;
	default:
		throw ppl7::grafix::UnsupportedColorFormatException();
	}
	throw ppl7::grafix::UnsupportedColorFormatException();
}


static void sdlSetWindowTitle(void* privatedata, const String& Title)
{
	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) return;
	SDL_SetWindowTitle(priv->win, (const char*)Title);

}

static void sdlSetWindowIcon(void* privatedata, const Drawable& Icon)
{
	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) return;
	if (Icon.isEmpty()) {
		SDL_SetWindowIcon(priv->win, NULL);
	} else {
		SDL_COLORFORMAT cf=RGBFormat2SDLStruct(Icon.rgbformat());

		SDL_Surface* s=SDL_CreateRGBSurfaceFrom(Icon.adr(),
			Icon.width(),
			Icon.height(),
			Icon.bitdepth(),
			Icon.pitch(),
			cf.Rmask, cf.Gmask, cf.Bmask, cf.Amask);
		if (!s) throw SurfaceCreateException("sdlSetWindowIcon ERROR: %s", SDL_GetError());
		SDL_SetWindowIcon(priv->win, s);
		SDL_FreeSurface(s);
	}
}

static void sdlCreateSurface(void* privatedata)
{

}

static void sdlCreateTexture(void* privatedata)
{

}

static Drawable sdlLockWindowSurface(void* privatedata)
{
	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) throw NullPointerException();
	void* pixels;
	int pitch;
	SDL_LockTexture(priv->gui, NULL, &pixels, &pitch);
	//printf("gui texture gelockt: %d x %d\n", priv->width, priv->height);
	return ppl7::grafix::Drawable(pixels, pitch, priv->width, priv->height, priv->format);
}

static void sdlUnlockWindowSurface(void* privatedata)
{
	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) throw NullPointerException();
	SDL_UnlockTexture(priv->gui);
}

static void sdlDrawWindowSurface(void* privatedata)
{
	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) throw NullPointerException();
	SDL_RenderCopy(priv->renderer, priv->gui, NULL, NULL);
}

static void* sdlGetRenderer(void* privatedata)
{
	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) throw NullPointerException();
	return priv->renderer;
}

static void sdlClearScreen(void* privatedata)
{
	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) throw NullPointerException();
	SDL_RenderClear(priv->renderer);
}

static void sdlPresentScreen(void* privatedata)
{
	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)privatedata;
	if (!priv) throw NullPointerException();
	SDL_RenderPresent(priv->renderer);
}

static void getButtonMask(MouseState& ev)
{
	uint8_t state=SDL_GetMouseState(NULL, NULL);
	if (state & 1) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask | MouseEvent::Left);
	if (state & 2) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask | MouseEvent::Middle);
	if (state & 4) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask | MouseEvent::Right);
	if (state & 8) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask | MouseEvent::WheelUp);
	if (state & 16) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask | MouseEvent::WheelDown);
	if (state & 32) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask | MouseEvent::X1);
	if (state & 64) ev.buttonMask=(MouseEvent::MouseButton)(ev.buttonMask | MouseEvent::X2);
}


static PRIV_WINDOW_FUNCTIONS sdlWmFunctions ={
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
	if (wm != NULL && wm != this) throw DuplicateWindowManagerException();
	wm=this;
	/* Get init data on all the subsystems */
	uint32_t subsystem_init;
	subsystem_init=SDL_WasInit(SDL_INIT_EVERYTHING);

	if (!(subsystem_init & SDL_INIT_VIDEO)) {
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) throw InitializationException("WindowManager_SDL2: SDL_INIT_VIDEO");
	}
	if (!(subsystem_init & SDL_INIT_TIMER)) {
		if (SDL_InitSubSystem(SDL_INIT_TIMER) < 0) throw InitializationException("WindowManager_SDL2: SDL_INIT_TIMER");
	}

	atexit(SDL_Quit);

	SDL_DisplayMode current;
	memset(&current, 0, sizeof(SDL_DisplayMode));
	current.driverdata=0;
	if (SDL_GetDesktopDisplayMode(0, &current) != 0) {
		throw InitializationException("SDL-ERROR: %s\n", SDL_GetError());
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
	if (wm == this) wm=NULL;
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
}

const Size& WindowManager_SDL2::desktopResolution() const
{
	return screenSize;
}

const RGBFormat& WindowManager_SDL2::desktopRGBFormat() const
{
	return screenRGBFormat;
}

void WindowManager_SDL2::createWindow(Window& w)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	if (w.getPrivateData() != NULL) throw WindowAlreadyCreatedException();

	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)calloc(sizeof(SDL_WINDOW_PRIVATE), 1);
	if (!priv) throw OutOfMemoryException();
	int flags=SDL_WINDOW_SHOWN;
	uint32_t wf=w.flags();
	if (wf & Window::NoBorder) flags|=SDL_WINDOW_BORDERLESS;
	if (wf & Window::Resizeable) flags|=SDL_WINDOW_RESIZABLE;
	if (wf & Window::Maximized) flags|=SDL_WINDOW_MAXIMIZED;
	if (wf & Window::Minimized) flags|=SDL_WINDOW_MINIMIZED;
	if (wf & Window::Fullscreen) flags|=SDL_WINDOW_FULLSCREEN;
	if (wf & Window::FullscreenDesktop) flags|=SDL_WINDOW_FULLSCREEN_DESKTOP;
	if (wf & Window::OpenGL) flags|=SDL_WINDOW_OPENGL;


	priv->win=SDL_CreateWindow((const char*)w.windowTitle(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		w.width(), w.height(), flags);
	if (priv->win == 0) {
		free(priv);
		throw WindowCreateException("SDL_CreateWindow ERROR: %s", SDL_GetError());
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

	SDL_SetWindowData(priv->win, "WindowClass", &w);
	flags=SDL_RENDERER_ACCELERATED;
	if (wf & Window::WaitVsync) flags|=SDL_RENDERER_PRESENTVSYNC;
	priv->renderer = SDL_CreateRenderer(priv->win, -1, flags);
	if (priv->renderer == 0) {
		const char* e=SDL_GetError();
		SDL_DestroyWindow(priv->win);
		free(priv);
		throw WindowCreateException("SDL_CreateWindow ERROR: %s", e);
	}
	int width, height;
	SDL_GetWindowSize(priv->win, &width, &height);
	w.setSize(width, height);

	priv->gui=SDL_CreateTexture(priv->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w.width(), w.height());
	if (priv->gui == 0) {
		const char* e=SDL_GetError();
		SDL_DestroyRenderer(priv->renderer);
		SDL_DestroyWindow(priv->win);
		free(priv);
		throw WindowCreateException("SDL_CreateWindow ERROR: %s", e);

	}
	if (SDL_SetTextureBlendMode(priv->gui, SDL_BLENDMODE_BLEND) != 0) {
		const char* e=SDL_GetError();
		SDL_DestroyTexture(priv->gui);
		SDL_DestroyRenderer(priv->renderer);
		SDL_DestroyWindow(priv->win);
		free(priv);
		throw WindowCreateException("SDL_SetTextureBlendMode ERROR: %s", e);
	}

	priv->format=RGBFormat::A8R8G8B8;
	priv->width=w.width();
	priv->height=w.height();
	w.setPrivateData(priv, this, &sdlWmFunctions);
	sdlSetWindowIcon(priv, w.windowIcon());
	windows.add(&w);
#endif
}

void WindowManager_SDL2::destroyWindow(Window& w)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)w.getPrivateData();
	if (!priv) return;
	windows.erase(&w);
	if (priv->gui) SDL_DestroyTexture(priv->gui);
	if (priv->renderer) SDL_DestroyRenderer(priv->renderer);
	if (priv->win) SDL_DestroyWindow(priv->win);
	free(priv);
	w.setPrivateData(NULL, NULL, NULL);
#endif
}

void WindowManager_SDL2::setWindowPosition(Window& w, int x, int y)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)w.getPrivateData();
	if (!priv) return;
	SDL_SetWindowPosition(priv->win, x, y);
#endif
}

size_t WindowManager_SDL2::numWindows()
{
	return windows.size();
}

void WindowManager_SDL2::getMouseState(Point& p, int& buttonMask) const
{

}

MouseState WindowManager_SDL2::getMouseState() const
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	//MouseEvent ev;
	MouseState mouse;
	getButtonMask(mouse);
	SDL_GetMouseState(&mouse.p.x, &mouse.p.y);
	//mouse.buttonMask=ev.buttonMask;
	mouse.button=mouse.buttonMask;
	return mouse;
#endif
}

void WindowManager_SDL2::startEventLoop()
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	while (numWindows() > 0) {
		handleEvents();
		ppl7::MSleep(5);
	}
#endif
}

Window* WindowManager_SDL2::getWindow(uint32_t id)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	SDL_Window* win=SDL_GetWindowFromID(id);
	if (!win) return NULL;
	return (Window*)SDL_GetWindowData(win, "WindowClass");
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
		case SDL_MOUSEWHEEL:
			DispatchMouseEvent(&sdl_event);
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			//printf ("SDL_KEYDOWN or SDL_KEYUP Event\n");
			DispatchKeyEvent(&sdl_event);
			break;
		case SDL_TEXTINPUT:
		{
			Widget* keyFocusWidget=getKeyboardFocus();
			if (keyFocusWidget) {
				SDL_TextInputEvent* event=(SDL_TextInputEvent*)&sdl_event;
				TextInputEvent te;
				te.text.set(event->text);
				keyFocusWidget->textInputEvent(&te);

			}

			//printf ("Event: SDL_TEXTINPUT\n");
			break;
		}
		case SDL_TEXTEDITING:
			//printf("Event: SDL_TEXTEDITING\n");
			break;
		case SDL_USEREVENT:

			if (sdl_event.user.code == 1) {	// ClickTimer
				//printf ("SDL_USEREVENT 1\n");
				dispatchClickEvent((Window*)sdl_event.user.data1);
			} else if (sdl_event.user.code == 2) {	// TimerEvent
				//printf("SDL_USEREVENT 2, windowid=%d\n", sdl_event.user.windowID);
				Widget* w=(Widget*)sdl_event.user.data1;
				Event e;
				e.setWidget(w);
				w->timerEvent(&e);
			}
			break;
		case SDL_CONTROLLERAXISMOTION:
		{
			Widget* gcWidget=getGameControllerFocus();
			if (gcWidget) {
				SDL_ControllerAxisEvent* event=(SDL_ControllerAxisEvent*)&sdl_event;
				GameControllerAxisEvent ev;
				ev.setType(Event::GameControllerAxisMotion);
				ev.setWidget(gcWidget);
				ev.axis=event->axis;
				ev.value=event->value;
				gcWidget->gameControllerAxisMotionEvent(&ev);
				//printf("Event: SDL_CONTROLLERAXISMOTION id=%d, axis=%d, value=%d\n", event->which, event->axis, event->value);

			}
			break;
		}
		case SDL_CONTROLLERBUTTONDOWN:
		{
			Widget* gcWidget=getGameControllerFocus();
			if (gcWidget) {
				SDL_ControllerButtonEvent* event=(SDL_ControllerButtonEvent*)&sdl_event;
				GameControllerButtonEvent ev;
				ev.setType(Event::GameControllerButtonDown);
				ev.setWidget(gcWidget);
				ev.button=event->button;
				ev.state=event->state;
				gcWidget->gameControllerButtonDownEvent(&ev);
				//printf("Event: SDL_CONTROLLERBUTTONDOWN, id=%d, button=%d, state=%d\n", event->which, event->button, event->state);
			}
			break;
		}
		case SDL_CONTROLLERBUTTONUP:
		{
			Widget* gcWidget=getGameControllerFocus();
			if (gcWidget) {
				SDL_ControllerButtonEvent* event=(SDL_ControllerButtonEvent*)&sdl_event;
				GameControllerButtonEvent ev;
				ev.setType(Event::GameControllerButtonUp);
				ev.setWidget(gcWidget);
				ev.button=event->button;
				ev.state=event->state;
				gcWidget->gameControllerButtonUpEvent(&ev);
				//printf("Event: SDL_CONTROLLERBUTTONDOWN, id=%d, button=%d, state=%d\n", event->which, event->button, event->state);
			}
			break;
		}
		case SDL_CONTROLLERDEVICEADDED:
		{
			Widget* gcWidget=getGameControllerFocus();
			if (gcWidget) {
				SDL_ControllerDeviceEvent* event=(SDL_ControllerDeviceEvent*)&sdl_event;
				GameControllerEvent ev;
				ev.setType(Event::GameControllerButtonUp);
				ev.setWidget(gcWidget);
				ev.which=event->which;
				gcWidget->gameControllerDeviceAdded(&ev);
			}

		}
		break;
		case SDL_CONTROLLERDEVICEREMOVED:
		{
			Widget* gcWidget=getGameControllerFocus();
			if (gcWidget) {
				SDL_ControllerDeviceEvent* event=(SDL_ControllerDeviceEvent*)&sdl_event;
				GameControllerEvent ev;
				ev.setType(Event::GameControllerButtonUp);
				ev.setWidget(gcWidget);
				ev.which=event->which;
				gcWidget->gameControllerDeviceRemoved(&ev);
			}
		}
		}
	}
#endif
}

void WindowManager_SDL2::DispatchQuitEvent(void* e)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	const SDL_Event* event=(SDL_Event*)e;
	Window* w=getWindow(event->window.windowID);
	if (!w) return;
	Event ev(Event::Quit);
	ev.setWidget(w);
	w->quitEvent(&ev);
#endif
}


void WindowManager_SDL2::DispatchWindowEvent(void* e)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	const SDL_Event* event=(SDL_Event*)e;

	Window* w=getWindow(event->window.windowID);
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
		/*
		w->clearScreen();
		w->drawWidgets();
		w->presentScreen();
		*/
		break;
	case SDL_WINDOWEVENT_MOVED:
		//fprintf(stderr, "Window %d moved to %d,%d",
		//        event->window.windowID, event->window.data1,
		//        event->window.data2);
		break;
	case SDL_WINDOWEVENT_RESIZED:
		//printf("SDL_WINDOWEVENT_RESIZED on window %d\n", event->window.windowID);
		//printf("Window %d resized to %dx%d",
		//	event->window.windowID, event->window.data1,
		//	event->window.data2);
	{
		w->setSize(event->window.data1, event->window.data2);
		resizeWindow(*w, event->window.data1, event->window.data2);
		ResizeEvent e;
		e.setWidget(w);
		w->resizeEvent(&e);

	}

	break;
	case SDL_WINDOWEVENT_SIZE_CHANGED:
		//printf("SDL_WINDOWEVENT_SIZE_CHANGED on window %d\n", event->window.windowID);
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
		Event e(Event::Close);
		e.setWidget(w);
		w->closeEvent(&e);
	}
	break;
	case SDL_WINDOWEVENT_TAKE_FOCUS:
		break;
	case SDL_WINDOWEVENT_HIT_TEST:
		break;
	default:
		printf("SDL Window %d got unknown event %d\n",
			event->window.windowID, event->window.event);
		break;
	}
#endif
}


void WindowManager_SDL2::DispatchMouseEvent(void* e)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	MouseEvent ev;
	Uint32 type=((SDL_Event*)e)->type;

	if (type == SDL_MOUSEMOTION) {
		SDL_MouseMotionEvent* event=(SDL_MouseMotionEvent*)e;

		Window* w=getWindow(event->windowID);
		if (!w) return;


		ev.setType(Event::MouseMove);
		ev.p.x=event->x;
		ev.p.y=event->y;
		ev.buttonMask=(MouseEvent::MouseButton)0;
		ev.button=(MouseEvent::MouseButton)0;
		getButtonMask(ev);
		dispatchEvent(w, ev);

	} else if (type == SDL_MOUSEBUTTONDOWN) {
		SDL_MouseButtonEvent* event=(SDL_MouseButtonEvent*)e;

		Window* w=getWindow(event->windowID);
		if (!w) return;

		ev.setType(Event::MouseDown);
		ev.p.x=event->x;
		ev.p.y=event->y;
		ev.buttonMask=(MouseEvent::MouseButton)0;
		ev.button=(MouseEvent::MouseButton)0;
		getButtonMask(ev);
		if (event->button == 1) ev.button=MouseEvent::Left;
		if (event->button == 2) ev.button=MouseEvent::Middle;
		if (event->button == 3) ev.button=MouseEvent::Right;
		if (event->button == 4) ev.button=MouseEvent::WheelUp;
		if (event->button == 5) ev.button=MouseEvent::WheelDown;
		if (event->button == 6) ev.button=MouseEvent::X1;
		if (event->button == 7) ev.button=MouseEvent::X2;
		dispatchEvent(w, ev);
	} else if (type == SDL_MOUSEBUTTONUP) {
		SDL_MouseButtonEvent* event=(SDL_MouseButtonEvent*)e;

		Window* w=getWindow(event->windowID);
		if (!w) return;

		ev.setType(Event::MouseUp);
		ev.p.x=event->x;
		ev.p.y=event->y;
		ev.buttonMask=(MouseEvent::MouseButton)0;
		ev.button=(MouseEvent::MouseButton)0;
		getButtonMask(ev);
		if (event->button == 1) ev.button=MouseEvent::Left;
		if (event->button == 2) ev.button=MouseEvent::Middle;
		if (event->button == 3) ev.button=MouseEvent::Right;
		if (event->button == 4) ev.button=MouseEvent::WheelUp;
		if (event->button == 5) ev.button=MouseEvent::WheelDown;
		if (event->button == 6) ev.button=MouseEvent::X1;
		if (event->button == 7) ev.button=MouseEvent::X2;
		dispatchEvent(w, ev);
	} else if (type == SDL_MOUSEWHEEL) {
		SDL_MouseWheelEvent* event=(SDL_MouseWheelEvent*)e;
		Window* w=getWindow(event->windowID);
		if (!w) return;
		ev.setType(Event::MouseWheel);
		MouseState mouse;
		ev.buttonMask=(MouseEvent::MouseButton)0;
		ev.button=(MouseEvent::MouseButton)0;
		SDL_GetMouseState(&ev.p.x, &ev.p.y);
		getButtonMask(ev);
		ev.wheel.x=event->x;
		ev.wheel.y=event->y;
		//printf ("MouseWheelEvent %d:%d\n", ev.wheel.x, ev.wheel.y);
		dispatchEvent(w, ev);

		//ev.setType(Event::MouseUp);

	}
#endif
}

typedef struct {

} CLICK_EVENT;

#ifdef HAVE_SDL2
static Uint32 clickTimer(Uint32 interval, void* param)
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

void WindowManager_SDL2::startClickEvent(Window* win)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	//printf ("WindowManager_SDL2::startClickEvent\n");
	SDL_AddTimer(getDoubleClickIntervall(), clickTimer, win);
#endif
}


#ifdef HAVE_SDL2
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"


static Uint32 timerEvent(Uint32 interval, void* param)
{

	//printf("SDL timerEvent\n");
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
	return interval;
}
#pragma GCC diagnostic pop
#endif

int WindowManager_SDL2::startTimer(Widget* w, int intervall)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	return SDL_AddTimer(intervall, timerEvent, w);
#endif
}

void WindowManager_SDL2::removeTimer(int timer_id)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	SDL_RemoveTimer((SDL_TimerID)timer_id);
#endif
}


void WindowManager_SDL2::DispatchKeyEvent(void* e)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	KeyEvent ev;
	SDL_KeyboardEvent* event=(SDL_KeyboardEvent*)e;

	//WideString st;
	//st.set((wchar_t)event->keysym.sym);
	//printf ("KeyEvent: State: %i, Repeat: %i, ",event->state,event->repeat);
	//printf ("Scancode: %i, Keycode: %i, Modifier: %i\n",event->keysym.scancode, event->keysym.sym, event->keysym.mod);
	//st.printnl();
	Widget* keyFocusWidget=getKeyboardFocus();
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
	case SDLK_c: kev.key=KeyEvent::KEY_c; break;
	case SDLK_v: kev.key=KeyEvent::KEY_v; break;
	case SDLK_z: kev.key=KeyEvent::KEY_z; break;
	case SDLK_y: kev.key=KeyEvent::KEY_y; break;


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


	if (kev.key == KeyEvent::KEY_UNKNOWN) return;

	if (event->type == SDL_KEYDOWN) {
		keyFocusWidget->keyDownEvent(&kev);

	} else if (event->type == SDL_KEYUP) {
		keyFocusWidget->keyUpEvent(&kev);
	}
#endif
}

void WindowManager_SDL2::changeWindowMode(Window& w, Window::WindowMode mode)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)w.getPrivateData();
	if (!priv) return;
	uint32_t flags=0;
	if (mode == Window::WindowMode::Fullscreen) flags=SDL_WINDOW_FULLSCREEN;
	else if (mode == Window::WindowMode::FullscreenDesktop) flags=SDL_WINDOW_FULLSCREEN_DESKTOP;
	SDL_SetWindowFullscreen(priv->win, flags);

#endif
}

Window::WindowMode WindowManager_SDL2::getWindowMode(Window& w)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)w.getPrivateData();
	if (!priv) throw NoWindowException();

	uint32_t flags=SDL_GetWindowFlags(priv->win);
	if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) return Window::WindowMode::FullscreenDesktop;
	if (flags & SDL_WINDOW_FULLSCREEN) return Window::WindowMode::Fullscreen;
	return Window::WindowMode::Window;
#endif
}

void WindowManager_SDL2::setWindowDisplayMode(Window& w, const Window::DisplayMode& mode)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)w.getPrivateData();
	if (!priv) return;
	Uint32 flags=SDL_GetWindowFlags(priv->win);
	SDL_DisplayMode sdlmode;
	sdlmode.driverdata=NULL;

	if (flags & SDL_WINDOW_FULLSCREEN) {

		sdlmode.h=mode.height;
		sdlmode.w=mode.width;
		sdlmode.refresh_rate=mode.refresh_rate;
		sdlmode.format=RGBFormat2SDLFormat(mode.format);
		if (SDL_SetWindowDisplayMode(priv->win, &sdlmode) != 0) {
			throw SDLException("SDL_SetWindowDisplayMode failed with: %s", SDL_GetError());
		}
	} else {
		SDL_SetWindowSize(priv->win, mode.width, mode.height);
		if (SDL_GetWindowDisplayMode(priv->win, &sdlmode) != 0) {
			throw SDLException("SDL_GetWindowDisplayMode failed with: %s", SDL_GetError());
		}
	}
	if (priv->gui) SDL_DestroyTexture(priv->gui);

	w.setRGBFormat(SDL2RGBFormat(sdlmode.format));
	w.setSize(sdlmode.w, sdlmode.h);

	priv->gui=SDL_CreateTexture(priv->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w.width(), w.height());
	if (priv->gui == 0) {
		const char* e=SDL_GetError();
		SDL_DestroyRenderer(priv->renderer);
		SDL_DestroyWindow(priv->win);
		free(priv);
		throw WindowCreateException("SDL_CreateWindow ERROR: %s", e);

	}
	if (SDL_SetTextureBlendMode(priv->gui, SDL_BLENDMODE_BLEND) != 0) {
		const char* e=SDL_GetError();
		SDL_DestroyTexture(priv->gui);
		SDL_DestroyRenderer(priv->renderer);
		SDL_DestroyWindow(priv->win);
		free(priv);
		throw WindowCreateException("SDL_SetTextureBlendMode ERROR: %s", e);
	}
	w.needsRedraw();
#endif
}

void WindowManager_SDL2::resizeWindow(Window& w, int width, int height)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	SDL_WINDOW_PRIVATE* priv=(SDL_WINDOW_PRIVATE*)w.getPrivateData();
	if (!priv) return;
	//printf("recreating gui texture\n");
	if (priv->gui) SDL_DestroyTexture(priv->gui);
	priv->gui=SDL_CreateTexture(priv->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (priv->gui == 0) {
		const char* e=SDL_GetError();
		throw WindowCreateException("SDL_CreateWindow ERROR: %s", e);

	}
	priv->width=width;
	priv->height=height;
	if (SDL_SetTextureBlendMode(priv->gui, SDL_BLENDMODE_BLEND) != 0) {
		const char* e=SDL_GetError();
		throw WindowCreateException("SDL_SetTextureBlendMode ERROR: %s", e);
	}
	w.needsRedraw();
#endif


}

void WindowManager_SDL2::setClipboardText(const ppl7::String& text)
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	SDL_SetClipboardText((const char*)text);
#endif
}

bool WindowManager_SDL2::hasClipboardText() const
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	return SDL_HasClipboardText();
#endif

}

String WindowManager_SDL2::getClipboardText() const
{
#ifndef HAVE_SDL2
	throw UnsupportedFeatureException("SDL2");
#else
	String t;
	char* text=SDL_GetClipboardText();
	if (text) {
		t.set(text);
		SDL_free(text);
	}
	return t;

#endif

}



}	// EOF namespace tk
}	// EOF namespace ppl7
