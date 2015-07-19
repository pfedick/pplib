/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.35 $
 * $Date: 2009/12/09 18:25:59 $
 * $Id: CEngineSDL.cpp,v 1.35 2009/12/09 18:25:59 patrick Exp $
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
#ifdef HAVE_SDL
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_ICONV
#include <iconv.h>
#endif
#ifdef WIN32
#undef HAVE_STDIO_H
#undef HAVE_SIGNAL_H
#endif
#include <SDL/SDL.h>
#include "ppl6.h"
#include "ppl6-grafix.h"
#include "ppl6-tk.h"


namespace ppl6 {
namespace grafix {

/*!\class CEngineSDL
 * \brief Grafik-Engine auf SDL-Basis
 * \ingroup PPLGroupGrafik
 *
 * \see http://www.libsdl.org
 */


using namespace ppl6::tk;

typedef struct tagSDLprivate
{
	const SDL_VideoInfo *vinfo;
	CWindow				*PrimaryWindow;
} SDLprivate;

typedef struct tagSDLsurface
{
	SDL_Surface		*surface;
	ppluint32		flags;
} SDLsurface;


// Prototypes
static int DestroySurface (struct tagSURFACE *surface);
static int SetCaption (struct tagSURFACE *surface, const char *title);
static int LockSurface (struct tagSURFACE *surface);
static int UnlockSurface (struct tagSURFACE *surface);
static int FlipSurface (struct tagSURFACE *surface);
static int FindRGBFormat(SDL_Surface *sdl);
static int ClearSurface(struct tagSURFACE *surface, COLOR c);
static int Blt (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y);


CEngineSDL::CEngineSDL()
{
	priv=new SDLprivate;
	((SDLprivate*)priv)->vinfo=NULL;
	((SDLprivate*)priv)->PrimaryWindow=NULL;
}

CEngineSDL::~CEngineSDL()
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	ppluint32 subsystem;
	subsystem=SDL_WasInit(SDL_INIT_EVERYTHING);
	if (!subsystem) {
		SDL_Quit();
	}
	if (priv) delete (SDLprivate*)priv;
}

int CEngineSDL::Init()
{
	ppluint32 subsystem;
	subsystem=SDL_WasInit(SDL_INIT_EVERYTHING);
	if (!subsystem) {
		if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
			SetError(1000,"Engine: SDL, Errors: %s",SDL_GetError());
			return 0;
		}
	} else {
		if(!(subsystem&SDL_INIT_VIDEO)) {
			if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0 ) {
				SetError(1000,"Engine: SDL, Errors: %s",SDL_GetError());
				return 0;
			}
		}
	}
	SDLprivate *p=(SDLprivate*)priv;
	p->vinfo=SDL_GetVideoInfo();
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(300, 20);

	return CEngine::Init();
}

const char *CEngineSDL::EngineName()
{
	return "SDL";
}

int CEngineSDL::GetPrivateData(CAssocArray *ret)
{
	if (!ret) {
		SetError(194);
		return 0;
	}
	ret->Clear();
	SDLprivate *p=(SDLprivate*)priv;
	if (!p) {
		SetError(195);
		return 0;
	}
	ret->Setf("vinfo/hw_available","%u",p->vinfo->hw_available);
	ret->Setf("vinfo/wm_available","%u",p->vinfo->wm_available);
	ret->Setf("vinfo/blit_hw","%u",p->vinfo->blit_hw);
	ret->Setf("vinfo/blit_hw_CC","%u",p->vinfo->blit_hw_CC);
	ret->Setf("vinfo/blit_hw_A","%u",p->vinfo->blit_hw_A);
	ret->Setf("vinfo/blit_sw","%u",p->vinfo->blit_sw);
	ret->Setf("vinfo/blit_sw_CC","%u",p->vinfo->blit_sw_CC);
	ret->Setf("vinfo/blit_sw_A","%u",p->vinfo->blit_sw_A);
	ret->Setf("vinfo/video_mem","%u",p->vinfo->video_mem);
	ret->Setf("vinfo/current_w","%u",p->vinfo->current_w);
	ret->Setf("vinfo/current_h","%u",p->vinfo->current_h);
	ret->Setf("vinfo/pixelformat/BitsPerPixel","%u",p->vinfo->vfmt->BitsPerPixel);
	ret->Setf("vinfo/pixelformat/BytesPerPixel","%u",p->vinfo->vfmt->BytesPerPixel);
	ret->Setf("vinfo/pixelformat/Rloss","%u",p->vinfo->vfmt->Rloss);
	ret->Setf("vinfo/pixelformat/Gloss","%u",p->vinfo->vfmt->Gloss);
	ret->Setf("vinfo/pixelformat/Bloss","%u",p->vinfo->vfmt->Bloss);
	ret->Setf("vinfo/pixelformat/Aloss","%u",p->vinfo->vfmt->Aloss);
	ret->Setf("vinfo/pixelformat/Rshift","%u",p->vinfo->vfmt->Rshift);
	ret->Setf("vinfo/pixelformat/Gshift","%u",p->vinfo->vfmt->Gshift);
	ret->Setf("vinfo/pixelformat/Bshift","%u",p->vinfo->vfmt->Bshift);
	ret->Setf("vinfo/pixelformat/Ashift","%u",p->vinfo->vfmt->Ashift);
	ret->Setf("vinfo/pixelformat/Rmask","%u",p->vinfo->vfmt->Rmask);
	ret->Setf("vinfo/pixelformat/Gmask","%u",p->vinfo->vfmt->Gmask);
	ret->Setf("vinfo/pixelformat/Bmask","%u",p->vinfo->vfmt->Bmask);
	ret->Setf("vinfo/pixelformat/Amask","%u",p->vinfo->vfmt->Amask);
	ret->Setf("vinfo/pixelformat/colorkey","%u",p->vinfo->vfmt->colorkey);
	ret->Setf("vinfo/pixelformat/alpha","%u",p->vinfo->vfmt->alpha);
	return 1;
}

int CEngineSDL::CreateWindow(CSurface *surface, ppl6::tk::CWindow *window)
{
	if (!surface) {
		SetError(194,"int CEngineSDL::CreateWindow(==> CSurface *surface <==, CWindow *window)");
		return 0;
	}
	SDLprivate *p=(SDLprivate*)priv;
	if (!p->vinfo) {
		SetError(1002);
		return 0;
	}
	SURFACE *s=surface->GetSurfaceParams();
	s->EngineSpecific=(void*)malloc(sizeof(SDLsurface));
	if (!s->EngineSpecific) {
		SetError(2);
		return 0;
	}
	SDLsurface *sdl=(SDLsurface*)s->EngineSpecific;
	sdl->flags=0;
	s->bitdepth=p->vinfo->vfmt->BitsPerPixel;
	// Flags initialisieren
	if (s->style&WindowStyle::NoFrame) sdl->flags|=SDL_NOFRAME;
	if (s->style&WindowStyle::Resizeable) sdl->flags|=SDL_RESIZABLE;
	if (s->flags&Surface::Flags::Hardware) sdl->flags|=SDL_HWSURFACE;
	else sdl->flags|=SDL_SWSURFACE;
	if (s->flags&Surface::Flags::Backbuffer) sdl->flags|=SDL_DOUBLEBUF;
	if (s->flags&Surface::Flags::TrippleBuffer) sdl->flags|=SDL_DOUBLEBUF;
	if (s->flags&Surface::Flags::Fullscreen) sdl->flags|=SDL_FULLSCREEN;

	sdl->surface=SDL_SetVideoMode(s->width, s->height, s->bitdepth, sdl->flags);
	if (!sdl->surface) {
		SetError(1008,"Engine: %s, Error: %s",EngineName(),SDL_GetError());
		return 0;
	}
	// Farbformat herausfinden
	s->rgbformat=FindRGBFormat(sdl->surface);
	if (!s->rgbformat) {
		// Unbekanntes Farbformat
		SetError(1013,"Engine: %s",EngineName());
		return 0;
	}

	// Pointer und Pitches initialisieren
	s->base=sdl->surface->pixels;
	s->base8=(ppluint8*)s->base;
	s->base16=(ppluint16*)s->base;
	s->base32=(ppluint32*)s->base;
	s->bitdepth=sdl->surface->format->BitsPerPixel;
	s->bytes_per_pixel=sdl->surface->format->BytesPerPixel;
	s->pitch=sdl->surface->pitch;
	s->pitch8=s->pitch;
	s->pitch16=(s->pitch>>1);
	s->pitch32=(s->pitch>>2);

	// Funktionen initialisieren
	if (SDL_MUSTLOCK(sdl->surface)) {
		printf("MustLock\n");
		s->Lock=LockSurface;
		s->Unlock=UnlockSurface;
	}
	s->Flip=FlipSurface;
	// SDL kann zur Zeit nur ein Fenster erstellen und das wird gelöscht, wenn
	// SDL_Quit aufgerufen wird
	//s->DestroySurface=DestroySurface;
	s->SetCaption=SetCaption;
	s->Blt=Blt;

	s->CLS=ClearSurface;
	p->PrimaryWindow=window;
	return 1;
}

int CEngineSDL::CreateFullscreen(CSurface *surface, ppl6::tk::CWindow *window)
{
	if (!surface) {
		SetError(194,"int CEngineSDL::CreateWindow(==> CSurface *surface <==, CWindow *window)");
		return 0;
	}
	SURFACE *s=surface->GetSurfaceParams();
	s->flags|=Surface::Flags::Fullscreen;
	return CreateWindow(surface,window);
}


int CEngineSDL::CreateSurface(CSurface *surface)
{
	if (!surface) {
		SetError(194,"int CEngineSDL::CreateSurface(CSurface *surface)");
		return 0;
	}
	SDLprivate *p=(SDLprivate*)priv;
	if (!p->vinfo) {
		SetError(1002);
		return 0;
	}
	SURFACE *s=surface->GetSurfaceParams();
	ppluint32 rmask, gmask, bmask, amask;
	switch (s->rgbformat) {
		case RGBFormat::A8R8G8B8:
			rmask=0xff0000;
			gmask=0xff00;
			bmask=0xff;
			amask=0xff000000;
			break;
		case RGBFormat::A8B8G8R8:
			rmask=0xff;
			gmask=0xff00;
			bmask=0xff0000;
			amask=0xff000000;
			break;
		case RGBFormat::X8R8G8B8:
			rmask=0xff0000;
			gmask=0xff00;
			bmask=0xff;
			amask=0;
			break;
		case RGBFormat::X8B8G8R8:
			rmask=0xff;
			gmask=0xff00;
			bmask=0xff0000;
			amask=0;
			break;
		default:
			SetError(1013,"CreateSurface, Engine: SDL");
			return 0;
	}

	s->EngineSpecific=(void*)malloc(sizeof(SDLsurface));
	if (!s->EngineSpecific) {
		SetError(2);
		return 0;
	}
	SDLsurface *sdl=(SDLsurface*)s->EngineSpecific;
	sdl->flags=0;
	s->bitdepth=GetBitdepth(s->rgbformat);
	// Flags initialisieren
	if (s->flags&Surface::Flags::Hardware) sdl->flags|=SDL_HWSURFACE;
	else sdl->flags|=SDL_SWSURFACE;

	sdl->surface=SDL_CreateRGBSurface(sdl->flags,s->width, s->height, s->bitdepth, rmask, gmask, bmask, amask);
	if (!sdl->surface) {
		SetError(1008,"Engine: %s, Error: %s",EngineName(),SDL_GetError());
		return 0;
	}
	// Farbformat herausfinden
	s->rgbformat=FindRGBFormat(sdl->surface);
	if (!s->rgbformat) {
		// Unbekanntes Farbformat
		SetError(1013,"Engine: %s",EngineName());
		return 0;
	}

	// Pointer und Pitches initialisieren
	s->base=sdl->surface->pixels;
	s->base8=(ppluint8*)s->base;
	s->base16=(ppluint16*)s->base;
	s->base32=(ppluint32*)s->base;
	s->bitdepth=sdl->surface->format->BitsPerPixel;
	s->bytes_per_pixel=sdl->surface->format->BytesPerPixel;
	s->pitch=sdl->surface->pitch;
	s->pitch8=s->pitch;
	s->pitch16=(s->pitch>>1);
	s->pitch32=(s->pitch>>2);

	// Funktionen initialisieren
	if (SDL_MUSTLOCK(sdl->surface)) {
		printf("MustLock\n");
		s->Lock=LockSurface;
		s->Unlock=UnlockSurface;
	}
	// SDL kann zur Zeit nur ein Fenster erstellen und das wird gelöscht, wenn
	// SDL_Quit aufgerufen wird
	s->DestroySurface=DestroySurface;
	s->Blt=Blt;
	s->CLS=ClearSurface;
	return 1;

}


static int FindRGBFormat(SDL_Surface *sdl)
{
	SDL_PixelFormat *f=sdl->format;
	/*
	printf ("Rmask: %i\n",f->Rmask);
	printf ("Gmask: %i\n",f->Gmask);
	printf ("Bmask: %i\n",f->Bmask);
	printf ("Amask: %i\n",f->Amask);
	*/
	if (f->BitsPerPixel==32) {
		if (f->Rmask==0xff0000 && f->Gmask==0xff00 && f->Bmask==0xff && f->Amask==0) return RGBFormat::X8R8G8B8;
		if (f->Rmask==0xff && f->Gmask==0xff00 && f->Bmask==0xff0000 && f->Amask==0) return RGBFormat::X8B8G8R8;
		if (f->Rmask==0xff0000 && f->Gmask==0xff00 && f->Bmask==0xff && f->Amask==0xff000000) return RGBFormat::A8R8G8B8;
		if (f->Rmask==0xff && f->Gmask==0xff00 && f->Bmask==0xff0000 && f->Amask==0xff000000) return RGBFormat::A8B8G8R8;
	}
	return 0;
}

static int DestroySurface (struct tagSURFACE *surface)
{
	SDLsurface *sdl=(SDLsurface*)surface->EngineSpecific;
	if (!sdl) return 1;

	return 1;
}

static int SetCaption (struct tagSURFACE *surface, const char *title)
{
	SDL_WM_SetCaption(title,title);
	return 1;
}

static int LockSurface (struct tagSURFACE *surface)
{
	SDLsurface *sdl=(SDLsurface*)surface->EngineSpecific;
	if (SDL_LockSurface(sdl->surface)==0) {
		surface->base=sdl->surface->pixels;
		surface->base8=(ppluint8*)surface->base;
		surface->base16=(ppluint16*)surface->base;
		surface->base32=(ppluint32*)surface->base;

		surface->pitch=sdl->surface->pitch;
		surface->pitch8=surface->pitch;
		surface->pitch16=(surface->pitch>>1);
		surface->pitch32=(surface->pitch>>2);

		return 1;
	}
	SetError(1009,"Engine: %s, Error: %s","SDL",SDL_GetError());
	return 0;
}

static int UnlockSurface (struct tagSURFACE *surface)
{
	SDLsurface *sdl=(SDLsurface*)surface->EngineSpecific;
	// SDL_UnlockSurface hat keinen Returnwert
	SDL_UnlockSurface(sdl->surface);
	return 1;
}

static int FlipSurface (struct tagSURFACE *surface)
{
	SDLsurface *sdl=(SDLsurface*)surface->EngineSpecific;
	if (SDL_Flip(sdl->surface)==0) return 1;
	SetError(1011,"Engine: %s, Error: %s","SDL",SDL_GetError());
	return 0;
}

static int ClearSurface(struct tagSURFACE *surface, COLOR c)
{
	SDLsurface *sdl=(SDLsurface*)surface->EngineSpecific;
	if (SDL_FillRect(sdl->surface,NULL, c)==0) return 1;
	SetError(1014,"Operation: CLS, Engine: %s, Error: %s","SDL",SDL_GetError());
	return 0;
}

static int Blt(struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y)
{
	SDLsurface *s=(SDLsurface*)source->EngineSpecific;
	SDLsurface *t=(SDLsurface*)target->EngineSpecific;
	if (s==NULL || t==NULL) {
		SetError(1014,"Operation: Blt, Engine: %s, Error: %s","SDL",SDL_GetError());
		return 0;
	}
	SDL_Rect r,d;
	r.x=(Sint16)srect->left;
	r.y=(Sint16)srect->top;
	r.w=(Sint16)(srect->right-srect->left+1);
	r.h=(Sint16)(srect->bottom-srect->top+1);
	d.x=x;
	d.y=y;
	if (SDL_BlitSurface(s->surface, &r, t->surface,&d)==0) return 1;
	SetError(1014,"Operation: Blt, Engine: %s, Error: %s","SDL",SDL_GetError());
	return 0;
}


static int TranslateKey (int key)
{
	//printf ("Key: %i\n",key);
	switch (key) {
		case SDLK_UP: return KEY::Up;
		case SDLK_DOWN: return KEY::Down;
		case SDLK_LEFT: return KEY::Left;
		case SDLK_RIGHT: return KEY::Right;
		case SDLK_INSERT: return KEY::Insert;
		case SDLK_DELETE: return KEY::Delete;
		case SDLK_HOME: return KEY::Home;
		case SDLK_END: return KEY::End;
		case SDLK_PAGEUP: return KEY::PageUp;
		case SDLK_PAGEDOWN: return KEY::PageDown;
		case SDLK_NUMLOCK: return KEY::NumLock;
		case SDLK_CAPSLOCK: return KEY::CapsLock;
		case SDLK_SCROLLOCK: return KEY::ScrollLock;
		case SDLK_LSHIFT: return KEY::ShiftLeft;
		case SDLK_RSHIFT: return KEY::ShiftRight;
		case SDLK_LCTRL: return KEY::ControlLeft;
		case SDLK_RCTRL: return KEY::ControlRight;
		case SDLK_LALT: return KEY::Alt;
		case SDLK_RALT: return KEY::AltGr;
		case SDLK_PAUSE: return KEY::Pause;
		case SDLK_BREAK: return KEY::Break;
		case 3: return KEY::Break;
		case SDLK_PRINT: return KEY::Print;
		case SDLK_RETURN: return KEY::Return;
		case SDLK_BACKSPACE: return KEY::Backspace;
		case SDLK_TAB: return KEY::Tab;
		case SDLK_ESCAPE: return KEY::Esc;

		case SDLK_F1: return KEY::F1;
		case SDLK_F2: return KEY::F2;
		case SDLK_F3: return KEY::F3;
		case SDLK_F4: return KEY::F4;
		case SDLK_F5: return KEY::F5;
		case SDLK_F6: return KEY::F6;
		case SDLK_F7: return KEY::F7;
		case SDLK_F8: return KEY::F8;
		case SDLK_F9: return KEY::F9;
		case SDLK_F10: return KEY::F10;
		case SDLK_F11: return KEY::F11;
		case SDLK_F12: return KEY::F12;

	};
	return KEY::UNKNOWN;
}

int CEngineSDL::StartMessageLoop()
{
	SDLprivate *p=(SDLprivate*)priv;
	SDL_Event event;
	EVENT e;
	ppluint8 sdl_buttons;
	int key;
	SDLMod m;
	bool insert=false;

	if (!p->PrimaryWindow) return 1;

	while (SDL_WaitEvent(&event)) {
		sdl_buttons=SDL_GetMouseState(&e.x,&e.y);
		m=SDL_GetModState();
		e.buttons=0;
		if (sdl_buttons&1) e.buttons|=SYSKEY::LBUTTON;
		if (sdl_buttons&2) e.buttons|=SYSKEY::MBUTTON;
		if (sdl_buttons&4) e.buttons|=SYSKEY::RBUTTON;
		if (m&KMOD_LSHIFT) e.buttons|=SYSKEY::SHIFT_LEFT;
		if (m&KMOD_RSHIFT) e.buttons|=SYSKEY::SHIFT_RIGHT;
		if (m&KMOD_LCTRL) e.buttons|=SYSKEY::CONTROL_LEFT;
		if (m&KMOD_RCTRL) e.buttons|=SYSKEY::CONTROL_RIGHT;
		if (m&KMOD_LALT) e.buttons|=SYSKEY::ALT;
		if (m&KMOD_MODE) e.buttons|=SYSKEY::ALTGR;
		if (m&KMOD_CAPS) e.buttons|=SYSKEY::CAPSLOCK;
		if (m&KMOD_NUM) e.buttons|=SYSKEY::NUMLOCK;

		if (event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_INSERT) {
			// Sofern keine andere Taste gedrückt wurde, switchen wir den Insert-Mode
			if ((e.buttons&(SYSKEY::SHIFT_LEFT|SYSKEY::SHIFT_RIGHT|SYSKEY::CONTROL_LEFT|SYSKEY::CONTROL_RIGHT|SYSKEY::ALT|SYSKEY::ALTGR))==0)
				insert=!insert;
		}

		if (insert) e.buttons|=SYSKEY::INSERT;
		//printf ("Buttons: 0x%05x\n",e.buttons);
		switch (event.type) {
			case SDL_QUIT:
				if (p->PrimaryWindow->WindowCloseRequest(p->PrimaryWindow)) {
					p->PrimaryWindow->WindowClose(p->PrimaryWindow);
					return 1;
				}
				break;
			case SDL_VIDEOEXPOSE:
				//printf ("Paint\n");
				p->PrimaryWindow->NeedRedraw();
				break;
			case SDL_ACTIVEEVENT:
				if (event.active.state&SDL_APPMOUSEFOCUS) {
					if (event.active.gain==0) p->PrimaryWindow->MouseLeave(p->PrimaryWindow);
					if (event.active.gain==1) p->PrimaryWindow->MouseEnter(p->PrimaryWindow);
				}
				if (event.active.state&SDL_APPINPUTFOCUS) {
					if (event.active.gain==0) p->PrimaryWindow->LostFocus(p->PrimaryWindow);
					if (event.active.gain==1) p->PrimaryWindow->GotFocus(p->PrimaryWindow);
				}
				if (event.active.state&SDL_APPACTIVE) {
					if (event.active.gain==1) p->PrimaryWindow->AppActivate(p->PrimaryWindow);
					else p->PrimaryWindow->AppInactivate(p->PrimaryWindow);
					//printf ("AppActive: %i\n",event.active.gain);
				}
				//printf ("SDL_ACTIVEEVENT, gain: %i, state: %i\n",event.active.gain, event.active.state);
				break;
			case SDL_MOUSEMOTION:
				e.type=EVENTTYPE::MOUSEMOTION;
				p->PrimaryWindow->MouseEvent(p->PrimaryWindow,e.x,e.y,e.buttons);
				p->PrimaryWindow->DispatchMouseEvent(&e);
				break;
			case SDL_MOUSEBUTTONDOWN:
				p->PrimaryWindow->ReleaseKeyFocus();
				e.type=EVENTTYPE::MOUSEDOWN;
				p->PrimaryWindow->MouseEvent(p->PrimaryWindow,e.x,e.y,e.buttons);
				p->PrimaryWindow->DispatchMouseEvent(&e);
				break;
			case SDL_MOUSEBUTTONUP:
				e.type=EVENTTYPE::MOUSEUP;
				p->PrimaryWindow->MouseEvent(p->PrimaryWindow,e.x,e.y,e.buttons);
				p->PrimaryWindow->DispatchMouseEvent(&e);
				break;
			case SDL_KEYDOWN:
				key=TranslateKey(event.key.keysym.sym);
				if (key==KEY::F4 && (e.buttons&SYSKEY::ALT)>0) {
					if (p->PrimaryWindow->WindowCloseRequest(p->PrimaryWindow)) {
						p->PrimaryWindow->WindowClose(p->PrimaryWindow);
						return 1;
					}
				}
				p->PrimaryWindow->DispatchKeyEvent(EVENTTYPE::KEYDOWN,event.key.keysym.unicode,e.buttons,key);
				break;
			case SDL_KEYUP:
				key=TranslateKey(event.key.keysym.sym);
				p->PrimaryWindow->DispatchKeyEvent(EVENTTYPE::KEYUP,event.key.keysym.unicode,e.buttons,key);
				break;
			case SDL_USEREVENT:
				CWidget *target=(CWidget*)event.user.data1;
				CAssocArray *msg=(CAssocArray *)event.user.data2;
				if (msg) {
					CString Event=msg->Get("EVENT");
					if (Event=="CLOSE") {
						if (target->WindowCloseRequest(target)) {
							target->WindowClose(target);
							delete target;
							delete msg;
							break;
						}
					}
				}
				if (target) target->Message(target,msg);
				if (msg) delete msg;
				break;

		};
		// Wenn es keine weiteren Nachrichten mehr gibt, prüfen wir, ob ein Redraw nötig ist
		if (SDL_PollEvent(NULL)==0) {
			if (p->PrimaryWindow->RedrawIfNeeded()) {
				//printf("Flip\n");
				CSurface *s=p->PrimaryWindow->GetSurface();
				if (s) s->Flip();
			}
		}
		//p->PrimaryWindow->PrintStatus(0);
	}
	return 1;
}

int CEngineSDL::GetMousePosition(CSurface *surface, POINT *p)
{
	SDLprivate *sp=(SDLprivate*)priv;
	if (!sp) {
		SetError(1002);
		return 0;
	}
	if (!surface) {
		SetError(194,"int CEngineSDL::GetMousePosition(==> CSurface *surface <==, POINT *p)");
		return 0;
	}
	if (!p) {
		SetError(194,"int CEngineSDL::GetMousePosition(CSurface *surface, ==> POINT *p <==)");
		return 0;
	}
	/*
	SURFACE *s=surface->GetSurfaceParams();
	SDLsurface *sdl=(SDLsurface*)s->EngineSpecific;
	if (!sdl) {
		return 0;
	}
	*/
	int x,y;
	SDL_GetMouseState(&x,&y);
	p->x=(int)x;
	p->y=(int)y;
	return 1;
}

int CEngineSDL::SendEvent(CSurface *surface, void *object, CAssocArray *msg)
{
	SDL_Event e;
	e.type=SDL_USEREVENT;
	e.user.code=1;
	e.user.data1=object;
	e.user.data2=msg;
	if (SDL_PushEvent(&e)==0) return 1;
	return 0;
}



} // EOF namespace grafix
} // EOF namespace ppl6
#endif // HAVE_SDL

