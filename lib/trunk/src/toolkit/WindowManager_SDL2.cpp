#include "ppl7.h""
#include "ppl7-grafix.h"
#include "ppl7-tk.h"
#ifdef HAVE_SDL2
#include <SDL2/SDL.h>
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
} SDL_WINDOW_PRIVATE;


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
	throw UnsupportedColorFormatException();
}

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





WindowManager_SDL2::WindowManager_SDL2()
{
	if (wm!=NULL && wm!=this) throw DuplicateWindowManagerException();
	wm=this;
	/* Get init data on all the subsystems */
	ppluint32 subsystem_init;
	subsystem_init=SDL_WasInit(SDL_INIT_EVERYTHING);

	if(!(subsystem_init&SDL_INIT_VIDEO)) {
		if (SDL_InitSubSystem(SDL_INIT_VIDEO)<0) throw InitializationException("WindowManager_SDL2");
	}
	atexit(SDL_Quit);

	SDL_DisplayMode current;
	current.driverdata=0;
	if (SDL_GetDesktopDisplayMode(0, &current)!=0) {
		throw InitializationException("SDL-ERROR: %s\n",SDL_GetError());
	}
	screenRGBFormat=SDL2RGBFormat(current.format);
	screenSize.setSize(current.w, current.h);
	screenRefreshRate=current.refresh_rate;
	//printf ("Auflösung: %i x %i, Format: %s, Refresh: %i\n",screenSize.width,screenSize.height,(const char*)screenRGBFormat.name(),screenRefreshRate);
}

WindowManager_SDL2::~WindowManager_SDL2()
{
	if (wm==this) wm=NULL;
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
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
	flags=SDL_RENDERER_ACCELERATED;
	if (wf&Window::WaitVsync) flags|=SDL_RENDERER_PRESENTVSYNC;
	priv->renderer = SDL_CreateRenderer(priv->win, -1, flags);
	if (priv->renderer==0) {
		const char *e=SDL_GetError();
		SDL_DestroyWindow(priv->win);
		free(priv);
		throw WindowCreateException("SDL_CreateWindow ERROR: %s",e);
	}

	priv->gui=SDL_CreateTexture(priv->renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,w.width(),w.height());
    if (priv->gui==0) {
		const char *e=SDL_GetError();
		SDL_DestroyRenderer(priv->renderer);
		SDL_DestroyWindow(priv->win);
		free(priv);
		throw WindowCreateException("SDL_CreateWindow ERROR: %s",e);

    }
	w.setPrivateData(priv, this);
}

void WindowManager_SDL2::destroyWindow(Window &w)
{
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)w.getPrivateData();
	if (!priv) return;
	if (priv->gui) SDL_DestroyTexture(priv->gui);
	if (priv->renderer) SDL_DestroyRenderer(priv->renderer);
	if (priv->win) SDL_DestroyWindow(priv->win);
	free(priv);
	w.setPrivateData(NULL,NULL);
}

void WindowManager_SDL2::setWindowTitle(Window &w)
{
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)w.getPrivateData();
	if (!priv) return;
	SDL_SetWindowTitle(priv->win,(const char*)w.windowTitle());

}

void WindowManager_SDL2::setWindowIcon(Window &w)
{
	SDL_WINDOW_PRIVATE *priv=(SDL_WINDOW_PRIVATE*)w.getPrivateData();
	if (!priv) return;
	const Drawable d=w.windowIcon();
	if (d.isEmpty()) {
		SDL_SetWindowIcon(priv->win, NULL);
	} else {
		SDL_COLORFORMAT cf=RGBFormat2SDLStruct(d.rgbformat());

		SDL_Surface* s=SDL_CreateRGBSurfaceFrom (d.adr(),
				d.width(),
				d.height(),
				d.bitdepth(),
				d.pitch(),
				cf.Rmask, cf.Gmask, cf.Bmask, cf.Amask);
		if (!s) throw SurfaceCreateException("SDL_CreateRGBSurfaceFrom ERROR: %s",SDL_GetError());
		SDL_SetWindowIcon(priv->win,s);
		SDL_FreeSurface(s);
	}

}



void WindowManager_SDL2::getMouseState(Point &p, int &buttonMask)
{

}

void WindowManager_SDL2::startEventLoop()
{

}

int WindowManager_SDL2::handleEvents()
{
	return 0;
}

void WindowManager_SDL2::drawIfNeeded() const
{

}

void WindowManager_SDL2::draw() const
{

}


#ifdef OLDCODE

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

