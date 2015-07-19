/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.18 $
 * $Date: 2009/11/21 20:01:31 $
 * $Id: CEngineDX9.cpp,v 1.18 2009/11/21 20:01:31 patrick Exp $
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

#ifdef WIN32
#undef HAVE_STDIO_H
#undef HAVE_SIGNAL_H
#endif
#ifdef HAVE_DX9
//#include <d3d9types.h>
#include <D3D9.h>
#include <Dxerr.h>
#include <D3dx9core.h>
#endif
#include "ppl6.h"
#include "ppl6-grafix.h"
#include "ppl6-tk.h"
#include "grafix6.h"


namespace ppl6 {
namespace grafix {

/*!\class CEngineDX9
 * \brief Grafik-Engines auf DirectX 9-Basis
 * \ingroup PPLGroupGrafik
 *
 */

using namespace ppl6::tk;

CEngineDX9 *MyEngine=NULL;

#ifdef HAVE_DX9

enum DX9SurfaceType {
	SurfaceTypeDevice,
	SurfaceTypeOffscreenPlain,
	SurfaceTypeTexture
};

typedef struct {
	LPDIRECT3D9 g_pD3D;
	HINSTANCE 	hInstance;
	CAssocArray *adapters;
	tk::CWindow		*PrimaryWindow;
	CSurface	*PrimarySurface;
	struct tagDX9Surface	*first;		// Es werden nur Fenster/Fullscreens gespeichert
} DX9private;

typedef struct tagDX9Surface {
	struct tagDX9Surface	*next, *previous;	// Nur f�r Fenster/Fullscreens
	DX9private			*engine;			// Private Datenstruktur der Grafik-Engine
	tk::CWindow			*window;			// Nur bei Fenster/Fullscreens
	CSurface			*surface;			// Pointer auf die Surface-Klasse
	IDirect3DDevice9	*g_pd3dDevice;		// Wird nur bei Windows und Fullscreen verwendet, bei Offscreen-Surfaces und Texturen zeigt er auf das Device, mit dem es erstellt wurde
	IDirect3DSurface9	*backbuffer;		// Bei Devices zeigt der Ptr auf den Backbuffer, bei Surfaces/Textures auf das Surface
	ID3DXSprite 		*pD3DSprite;		// Nur bei Fenster/Fullscreens
	D3DCAPS9			caps;				// Nur bei Fenster/Fullscreens

	DX9SurfaceType	Type;				// Gibt den Typ des Surface an
	D3DPOOL			Pool;
	// Die Folgenden Parameter werden nur bei Fenstern verwendet
	HWND hwnd;
	D3DPRESENT_PARAMETERS d3dpp;
	DWORD BehaviorFlags;
	D3DDISPLAYMODE pMode;
	POINT	maus;
	bool	InScene;
	bool	InSprite;
} DX9Surface;


typedef struct {
	HWND	hwnd;
	UINT	uMsg;
	WPARAM	wParam;
	LPARAM	lParam;
	POINT	pt;
} DX9_MSG;


// Prototypes
// 3D-Devices (Window/Fullscreen)
static int DestroyDevice (struct tagSURFACE *surface);
static int SetCaption (struct tagSURFACE *surface, const char *title);
static int LockDevice (struct tagSURFACE *surface);
static int UnlockDevice (struct tagSURFACE *surface);
static int FlipDevice (struct tagSURFACE *surface);
static int ClearDevice(struct tagSURFACE *surface, COLOR c);
static int BeginScene(struct tagSURFACE *surface);
static int EndScene(struct tagSURFACE *surface);
static int PostMessage(struct tagSURFACE *surface, unsigned int msg, void *param1, void *param2);


static int DestroySurface (struct tagSURFACE *surface);
static int LockSurface (struct tagSURFACE *surface);
static int UnlockSurface (struct tagSURFACE *surface);
static int Blt (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y);

static int DestroyTexture (struct tagSURFACE *surface);
static int LockTexture (struct tagSURFACE *surface);
static int UnlockTexture (struct tagSURFACE *surface);


static int BeginSprites (struct tagSURFACE *surface);
static int DrawSprite (struct tagSURFACE *target, int x, int y, struct tagSURFACE *source, const RECT *SrcRect, COLOR c);
static int EndSprites (struct tagSURFACE *surface);


static D3DFORMAT RGBFORMATtoDX9(int format)
{
	switch (format) {
		case RGBFormat::Palette:
			RETVAL(D3DFMT_P8);
		case RGBFormat::R8G8B8:
			RETVAL(D3DFMT_R8G8B8);
		case RGBFormat::A8R8G8B8:
			RETVAL(D3DFMT_A8R8G8B8);
		case RGBFormat::X8R8G8B8:
			RETVAL(D3DFMT_X8R8G8B8);
		case RGBFormat::A8B8G8R8:
			return D3DFMT_A8B8G8R8;
		case RGBFormat::X8B8G8R8:
			return D3DFMT_X8B8G8R8;
		case RGBFormat::R5G6B5:
			RETVAL(D3DFMT_R5G6B5);
		case RGBFormat::R5G5B5:
		case RGBFormat::X1R5G5B5:
			RETVAL(D3DFMT_X1R5G5B5);
		case RGBFormat::A1R5G5B5:
			RETVAL(D3DFMT_A1R5G5B5);
		case RGBFormat::A4R4G4B4:
			RETVAL(D3DFMT_A4R4G4B4);
		case RGBFormat::A8:
		case RGBFormat::GREY8:
			RETVAL(D3DFMT_A8);
		case RGBFormat::R3G3B2:
			RETVAL(D3DFMT_R3G3B2);
		case RGBFormat::A8R3G3B2:
			RETVAL(D3DFMT_A8R3G3B2);
		default:
			SetError(199,"%i",format);
			RETVAL (D3DFMT_UNKNOWN);
	}
}

static int DX9toRGBFORMAT(D3DFORMAT format)
{
	switch (format) {
		case D3DFMT_P8:				// 8-bit Color Indexed
			return RGBFormat::Palette;
			break;
		case D3DFMT_R8G8B8:			// 24-bit RGB pixel format with 8 bits per channel
			return RGBFormat::R8G8B8;
			break;
		case D3DFMT_A8R8G8B8:		// 32-bit ARGB pixel format with alpha, using 8 bits per channel
			return RGBFormat::A8R8G8B8;
			break;
		case D3DFMT_X8R8G8B8:		// 32-bit RGB pixel format, where 8 bits are reserved for each color.
			return RGBFormat::X8R8G8B8;
			break;
		case D3DFMT_A8B8G8R8:		// 32-bit ABGR pixel format with alpha, using 8 bits per channel
			return RGBFormat::A8R8G8B8;
			break;
		case D3DFMT_X8B8G8R8:		// 32-bit RGB pixel format, where 8 bits are reserved for each color.
			return RGBFormat::X8B8G8R8;
			break;
		case D3DFMT_R5G6B5:			// 16-bit RGB pixel format with 5 bits for red, 6 bits for green, and 5 bits for blue
			return RGBFormat::R5G6B5;
			break;
		case D3DFMT_X1R5G5B5:		// 16-bit pixel format where 5 bits are reserved for each color.
			return RGBFormat::R5G5B5;
			break;
		case D3DFMT_A1R5G5B5:		// 16-bit pixel format where 5 bits are reserved for each color and 1 bit is reserved for alpha.
			return RGBFormat::A1R5G5B5;
			break;
		case D3DFMT_A4R4G4B4:		// 16-bit ARGB pixel format with 4 bits for each channel
			return RGBFormat::A4R4G4B4;
			break;
		case D3DFMT_A8:				// 8-bit alpha only.
			return RGBFormat::A8;
			break;
		case D3DFMT_R3G3B2:			// 8-bit RGB texture format using 3 bits for red, 3 bits for green, and 2 bits for blue.
			return RGBFormat::R3G3B2;
			break;
		case D3DFMT_A8R3G3B2:		// 16-bit ARGB texture format using 8 bits for alpha, 3 bits each for red and green, and 2 bits for blue.
			return RGBFormat::A8R3G3B2;
			break;
		default:
			SetError(199,"%i",format);
			return RGBFormat::unknown;
	}
	SetError(199,"%i",format);
	return RGBFormat::unknown;
}


static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!MyEngine) {
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}
	DX9_MSG m;
	m.hwnd=hwnd;
	m.uMsg=uMsg;
	m.wParam=wParam;
	m.lParam=lParam;
	GetCursorPos(&m.pt);
	if (!MyEngine->DispatchDX9Message(&m)) return DefWindowProc(hwnd,uMsg,wParam,lParam);
	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

#endif

CEngineDX9::CEngineDX9()
{
	priv=NULL;
}

CEngineDX9::~CEngineDX9()
{
#ifdef HAVE_DX9
	if (priv) {
		DX9private *p=(DX9private *)priv;
		if (p->adapters) delete p->adapters;
		if (p->g_pD3D) p->g_pD3D->Release();
		free(priv);
	}
#endif
	if (MyEngine==this) MyEngine=NULL;
}

const char *CEngineDX9::EngineName()
{
	return "DX9";
}

int CEngineDX9::EnumAdapters(CAssocArray *result)
{
#ifdef HAVE_DX9
	if (!result) {
		SetError(194,"int CEngineDX9::EnumAdapters(==> CAssocArray *result <==)");
		return 0;
	}
	if (!priv) {
		SetError(1002,"ppl6::grafix::CEngineDX9");
		return 0;
	}
	DX9private *p=(DX9private *)priv;
	if (p->g_pD3D==NULL) {
		SetError(1002,"ppl6::grafix::CEngineDX9");
		return 0;
	}

	UINT count=p->g_pD3D->GetAdapterCount();
	HRESULT res;
	D3DCAPS9 Caps;
	D3DADAPTER_IDENTIFIER9 a;
	D3DDISPLAYMODE pMode;

	CAssocArray row, modes;
	for (UINT i=0;i<count;i++) {
		res=p->g_pD3D->GetAdapterIdentifier(i,0,&a);
		if (res==D3D_OK) {
			row.Clear();
			row.Setf("adapter/Id","%i",i);
			row.Setf("adapter/Driver",a.Driver);
			row.Setf("adapter/Description",a.Description);
			row.Setf("adapter/Name",a.DeviceName);
			int Product = HIWORD(a.DriverVersion.HighPart);
			int Version = LOWORD(a.DriverVersion.HighPart);
			int SubVersion = HIWORD(a.DriverVersion.LowPart);
			int Build = LOWORD(a.DriverVersion.LowPart);
			row.Setf("adapter/Version","%i.%i.%i",Product,Version,SubVersion);
			row.Setf("adapter/Build","%i",Build);
			row.Setf("adapter/VendorId","%i",a.VendorId);
			row.Setf("adapter/DeviceId","%i",a.DeviceId);
			row.Setf("adapter/SubSysId","%i",a.SubSysId);
			row.Setf("adapter/Revision","%i",a.Revision);

			res=p->g_pD3D->GetAdapterDisplayMode(i,&pMode);
			if (res==D3D_OK) {
				row.Setf("vinfo/current_w","%i",pMode.Width);
				row.Setf("vinfo/current_h","%i",pMode.Height);
				row.Setf("vinfo/current_refresh","%i",pMode.RefreshRate);
				row.Setf("vinfo/current_rgb","%i",DX9toRGBFORMAT(pMode.Format));
			}



			res=p->g_pD3D->GetDeviceCaps(i,D3DDEVTYPE_HAL,&Caps);
			if (res==D3D_OK) {
				row.Setf("vinfo/max_texture_width","%i",Caps.MaxTextureWidth);
				row.Setf("vinfo/max_texture_height","%i",Caps.MaxTextureHeight);
				row.Setf("vinfo/max_active_lights","%i",Caps.MaxActiveLights);
				row.Setf("vinfo/pixel_shader_version","%i.%i",(Caps.PixelShaderVersion>>8)&0xff,
					Caps.PixelShaderVersion&0xff);

			}

			modes.Clear();
			if (EnumAdapterModes(&modes,RGBFormat::X8R8G8B8,i)) {
				row.Set("Modes/X8R8G8B8",&modes,true);
			}
			modes.Clear();
			if (EnumAdapterModes(&modes,RGBFormat::R5G6B5,i)) {
				row.Set("Modes/R5G6B5",&modes,true);
			}
			modes.Clear();
			if (EnumAdapterModes(&modes,RGBFormat::R5G5B5,i)) {
				row.Set("Modes/X1R5G5B5",&modes,true);
			}

			result->Set("[]",&row);
		}
	}
	return 1;
#else
	SetError(1049,"DX9");
	return 0;
#endif
}

int CEngineDX9::EnumAdapterModes(CAssocArray *result, int format, int adapter)
{
#ifdef HAVE_DX9
	if (!result) {
		SetError(194,"int CGrafixEngineDX9::EnumAdapterModes( ==> CpplAssocArray *result <== , RGBFormat::RGBFormat format, int adapter)");
		return 0;
	}
	if (!priv) {
		SetError(1002,"ppl6::grafix::CEngineDX9");
		return 0;
	}
	DX9private *p=(DX9private *)priv;
	if (p->g_pD3D==NULL) {
		SetError(1002,"ppl6::grafix::CEngineDX9");
		return 0;
	}

	if (adapter<0) adapter=D3DADAPTER_DEFAULT;
	D3DFORMAT f=RGBFORMATtoDX9(format);
	if (!f) {
		return 0;
	}
	D3DDISPLAYMODE m;
	CAssocArray row;
	UINT count=p->g_pD3D->GetAdapterModeCount(adapter,f);
	for (UINT i=0;i<count;i++) {
		HRESULT ret=p->g_pD3D->EnumAdapterModes(adapter,f,i,&m);
		row.Setf("Width","%i",m.Width);
		row.Setf("Height","%i",m.Height);
		row.Setf("RefreshRate","%i",m.RefreshRate);
		row.Setf("Mode","%i",i);
		row.Setf("RGBFormat","%i",DX9toRGBFORMAT(m.Format));
		result->Set("[]",&row);
	}
	return 1;
#else
	SetError(1049,"DX9");
	return 0;
#endif
}

int CEngineDX9::Init()
{
#ifdef HAVE_DX9
	if (!priv) {
		priv=calloc(1,sizeof(DX9private));
		if (!priv) {
			SetError(1000,"Memory allocation for private data failed");
			return 0;
		}
	}
	DX9private *p=(DX9private *)priv;
	if (p->g_pD3D!=NULL) {
		SetError(1047,"ppl6::grafix::CEngineDX9");
		return 0;
	}
	p->hInstance=GetModuleHandle(NULL);
	// Fenster-Klasse registrieren
	WNDCLASSEX	winclass;
	winclass.cbSize=sizeof(WNDCLASSEX);
	winclass.style=CS_DBLCLKS|CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	winclass.lpfnWndProc=(WNDPROC)WindowProc;
	//winclass.lpfnWndProc=NULL;
	winclass.cbClsExtra=0;
	winclass.cbWndExtra=0;
	winclass.hInstance=p->hInstance;
	winclass.hIcon = NULL;
	winclass.hIconSm = NULL;
	winclass.hCursor=NULL;
	winclass.hbrBackground=NULL;
	winclass.lpszMenuName=NULL;
	winclass.lpszClassName="PPL6Window";
	if (!RegisterClassEx(&winclass)) {
		SetError(96,"RegisterClassEx failed");
		return 0;
	}

	p->g_pD3D=Direct3DCreate9( D3D_SDK_VERSION );
	if (!p->g_pD3D) {
		SetError(1000);
		return 0;
	}
	if (!p->adapters) p->adapters=new CAssocArray;
	if (!EnumAdapters(p->adapters)) {
		p->g_pD3D->Release();
		p->g_pD3D=NULL;
		return 0;
	}
	MyEngine=this;
	return CEngine::Init();
#else
	SetError(1049,"DX9");
	return 0;
#endif
}

int CEngineDX9::GetPrivateData(CAssocArray *ret)
{
#ifdef HAVE_DX9
	if (!ret) {
		SetError(194);
		return 0;
	}
	if (!priv) {
		SetError(195);
		return 0;
	}
	ret->Clear();
	DX9private *p=(DX9private *)priv;
	ret->Set("adapters",p->adapters,true);
	return 1;
#else
	SetError(1049,"DX9");
	return 0;
#endif
}

int CEngineDX9::CreateFullscreen(CSurface *surface, ppl6::tk::CWindow *window)
{
	if (!surface) {
		SetError(194,"int CEngineDX9::CreateWindow(==> CSurface *surface <==, CWindow *window)");
		return 0;
	}
	SURFACE *s=surface->GetSurfaceParams();
	s->flags|=Surface::Flags::Fullscreen;
	return CreateWindow(surface,window);
}


int CEngineDX9::CreateWindow(CSurface *surface, ppl6::tk::CWindow *window)
{
#ifdef HAVE_DX9
	HRESULT hr;
	if (!surface) {
		SetError(194,"int CEngineDX9::CreateWindow(==> CSurface *surface <==, CWindow *window)");
		return 0;
	}
	DX9private *p=(DX9private *)priv;
	if (!p->g_pD3D) {
		SetError(1002,"ppl6::grafix::CEngineDX9");
		return 0;
	}

	SURFACE *s=surface->GetSurfaceParams();
	s->EngineSpecific=(void*)calloc(1,sizeof(DX9Surface));
	if (!s->EngineSpecific) {
		SetError(2);
		return 0;
	}
	DX9Surface *dx9=(DX9Surface*)s->EngineSpecific;
	// Create the application's window. PPL6Window
	RECT wRect;
	DWORD dwExStyle=0;
	if (s->style&WindowStyle::AllowDrop) dwExStyle|=WS_EX_ACCEPTFILES;
	if ((s->style&WindowStyle::TopMost) || (s->flags&Surface::Flags::Fullscreen)) {
		dwExStyle|=WS_EX_TOPMOST;
	}
	DWORD dwStyle=WS_OVERLAPPED|WS_VISIBLE|WS_MINIMIZEBOX|WS_SYSMENU;
	//DWORD dwStyle=WS_POPUP|WS_VISIBLE|WS_MINIMIZEBOX|WS_SYSMENU;
	if (s->style&WindowStyle::HasCaption) {
		dwStyle|=WS_CAPTION|WS_SYSMENU;
	}
	if (s->style&WindowStyle::Resizeable) {
		dwStyle|=WS_SIZEBOX;
	}

	// Die gewuenschte Groesse des ClientRects angeben
	wRect.left=0;
	wRect.top=0;
	wRect.bottom=wRect.top+s->height;
	wRect.right=wRect.left+s->width;


	if (!AdjustWindowRectEx(&wRect,dwStyle,false,dwExStyle)) {
		free(s->EngineSpecific);
		s->EngineSpecific=NULL;
		SetError(1008,"AdjustWindowRectEx failed");
		return 0;
	}
	if (wRect.top<0) {
		wRect.bottom+=abs(wRect.top);
		wRect.top=0;
	}
	if (wRect.left<0) {
		wRect.right+=abs(wRect.left);
		wRect.left=0;
	}
	int width=wRect.right-wRect.left;
	int height=wRect.bottom-wRect.top;

	int x=CW_USEDEFAULT;
	int y=CW_USEDEFAULT;
	if (window) {
		x=window->X();
		y=window->Y();
	}
	if (s->style&WindowStyle::PositionDefault) {
		x=CW_USEDEFAULT;
		y=CW_USEDEFAULT;
	}

	// DirectX-Device erstellen
	// Behavior-Flags
	dx9->BehaviorFlags=0;
	if (s->flags&Surface::Flags::Multithreaded) dx9->BehaviorFlags|=D3DCREATE_MULTITHREADED;
	if (s->flags&Surface::Flags::SoftwareVertexProcessing) dx9->BehaviorFlags|=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	else dx9->BehaviorFlags|=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	if (s->flags&Surface::Flags::FPUPreserve) dx9->BehaviorFlags|=D3DCREATE_FPU_PRESERVE;
	// D3DPRESENT_PARAMETERS
	ZeroMemory( &dx9->d3dpp, sizeof(dx9->d3dpp) );
	if (s->flags&Surface::Flags::Fullscreen) {
		dx9->d3dpp.BackBufferFormat=(D3DFORMAT)RGBFORMATtoDX9(s->rgbformat);
		if (s->rgbformat==0 || dx9->d3dpp.BackBufferFormat==D3DFMT_UNKNOWN) {
			free(s->EngineSpecific);
			s->EngineSpecific=NULL;
			SetError(1051);
			return 0;
		}

		dx9->d3dpp.Windowed = FALSE;
	} else {
		dx9->d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		//d3dpp.BackBufferFormat=(D3DFORMAT)RGBFORMATtoDX9(s->rgbformat);
		dx9->d3dpp.Windowed = TRUE;
	}
	dx9->d3dpp.BackBufferWidth=s->width;
	dx9->d3dpp.BackBufferHeight=s->height;
	if (s->flags&Surface::Flags::Backbuffer) dx9->d3dpp.BackBufferCount=1;
	if (s->flags&Surface::Flags::TrippleBuffer) dx9->d3dpp.BackBufferCount=2;

	dx9->d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;

	if (s->flags&Surface::Flags::Lockable)	dx9->d3dpp.Flags|=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	if ((s->flags&Surface::Flags::ZBuffer) || (s->flags&Surface::Flags::Stencil) ) {
		dx9->d3dpp.EnableAutoDepthStencil=true;
		dx9->d3dpp.AutoDepthStencilFormat=D3DFMT_D16;		// 32-Bit
		if (s->flags&Surface::Flags::Stencil) {
			//d3dpp.AutoDepthStencilFormat=D3DFMT_D24S8;	// 24-Bit Z-Buffer, 8-Bit Stencil
		}
	}
	// Beim Flip auf VSYNC des Monitors warten?
	if (s->flags&Surface::Flags::NoVsync) dx9->d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;		// Nein
	else if (s->flags&Surface::Flags::WaitVsync) dx9->d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_ONE;		// Ja, zwingend
	else dx9->d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_DEFAULT;										// Default der Grafikkarte
	if (s->frequency) dx9->d3dpp.FullScreen_RefreshRateInHz=s->frequency;


	// Fenster erstellen

	HWND hWnd = ::CreateWindowEx(dwExStyle, "PPL6Window",
		(const char*)(window!=NULL?window->Caption:surface->GetName()),
		dwStyle, x,y,
		width, height, GetDesktopWindow(), NULL,
		p->hInstance, NULL );
	if (!hWnd) {
		free(s->EngineSpecific);
		s->EngineSpecific=NULL;
		SetError(1008,"CreateWindowEx failed");
		return 0;
	}
	::ShowWindow(hWnd,SW_SHOW);
	dx9->hwnd=hWnd;
	dx9->d3dpp.hDeviceWindow=hWnd;

	// Device erstellen
	hr=p->g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		dx9->BehaviorFlags,
		&dx9->d3dpp, &dx9->g_pd3dDevice );
	if (hr!=D3D_OK) {
		::DestroyWindow(hWnd);
		free(s->EngineSpecific);
		s->EngineSpecific=NULL;
		SetError(1050,hr,"%s",DXGetErrorDescription(hr));
		return 0;
	}

	dx9->g_pd3dDevice->GetDeviceCaps(&dx9->caps);

	// DisplayMode

	hr=dx9->g_pd3dDevice->GetDisplayMode(0,&dx9->pMode);
	if (hr!=D3D_OK) {
		dx9->g_pd3dDevice->Release();	// Device l�schen
		::DestroyWindow(hWnd);		// Fenster l�schen
		free(s->EngineSpecific);	// Speicher freigeben
		s->EngineSpecific=NULL;
		SetError(1014,hr,"%s",DXGetErrorDescription(hr));
		return 0;

	}
	s->rgbformat=DX9toRGBFORMAT(dx9->pMode.Format);
	if (s->rgbformat==RGBFormat::unknown) {	// Hier stimmt was nicht
		dx9->g_pd3dDevice->Release();	// Device l�schen
		::DestroyWindow(hWnd);		// Fenster l�schen
		free(s->EngineSpecific);	// Speicher freigeben
		s->EngineSpecific=NULL;
		SetError(1013);
		return 0;
	}

	s->bitdepth=GetBitdepth(s->rgbformat);
	s->bytes_per_pixel=s->bitdepth/8;
	dx9->Type=SurfaceTypeDevice;

	// Viewport setzen
	D3DVIEWPORT9	pViewport;
	pViewport.X=0;
	pViewport.Y=0;
	pViewport.Width=s->width;
	pViewport.Height=s->height;
	pViewport.MinZ=0.0;
	pViewport.MaxZ=1.0;

	dx9->g_pd3dDevice->SetViewport(&pViewport);


	// Sprite-Interface
	dx9->pD3DSprite=NULL;
	D3DXCreateSprite(dx9->g_pd3dDevice,&dx9->pD3DSprite);




	// Funktionen initialisieren
	s->Lock=LockDevice;
	s->Unlock=UnlockDevice;
	s->Flip=FlipDevice;
	s->DestroySurface=DestroyDevice;
	s->SetCaption=SetCaption;
	s->Blt=Blt;
	s->BeginScene=BeginScene;
	s->EndScene=EndScene;
	s->PostMessage=PostMessage;
	s->CLS=ClearDevice;
	s->BeginSprites=BeginSprites;
	s->EndSprites=EndSprites;
	s->DrawSprite=DrawSprite;

	if (!p->PrimaryWindow) p->PrimaryWindow=window;
	if (!p->PrimarySurface) p->PrimarySurface=surface;

	dx9->surface=surface;
	dx9->window=window;
	dx9->engine=p;

	// Fenster in die Kette h�ngen
	if (!p->first) {
		p->first=dx9;
	} else {
		dx9->next=p->first;
		p->first=dx9;
	}


	return 1;
#else
	SetError(1049,"DX9");
	return 0;
#endif

}


int CEngineDX9::CreateSurface(CSurface *surface)
{
#ifdef HAVE_DX9
	HRESULT hr;
	if (!surface) {
		SetError(194,"int CEngineDX9::CreateSurface(==> CSurface *surface <==)");
		return 0;
	}
	DX9private *p=(DX9private *)priv;
	if (!p->g_pD3D) {
		SetError(1002,"ppl6::grafix::CEngineDX9");
		return 0;
	}
	// Wir ben�tigen ein PrimarySurface
	if (!p->PrimarySurface) {
		SetError(1052);
		return 0;
	}
	SURFACE *primary=p->PrimarySurface->GetSurfaceParams();
	if (!primary) {
		SetError(1052);
		return 0;
	}
	DX9Surface *primarydx9=(DX9Surface*)primary->EngineSpecific;

	SURFACE *s=surface->GetSurfaceParams();
	// Wenn kein Farbformat angegeben wurde, wird das des Primary Surface verwendet
	if (!s->rgbformat) s->rgbformat=primary->rgbformat;

	// Bekanntes Farbformat?
	D3DFORMAT format=RGBFORMATtoDX9(s->rgbformat);
	if (format==D3DFMT_UNKNOWN) {
		SetError(1013);
		return 0;
	}

	s->EngineSpecific=(void*)calloc(1,sizeof(DX9Surface));
	if (!s->EngineSpecific) {
		SetError(2);
		return 0;
	}
	DX9Surface *dx9=(DX9Surface*)s->EngineSpecific;

	// Welchen Pool verwenden wir?
	dx9->Pool=D3DPOOL_DEFAULT;
	if (s->flags&Surface::Flags::LocalMemory) dx9->Pool=D3DPOOL_SYSTEMMEM;

	if (s->flags&Surface::Flags::Texture) {
		// H�he und Breite m�ssen eine 2er Potenz sein, also 1,2,4,8,16,32,64,128....
		int width=2, height=2;
		if (primarydx9->caps.TextureCaps&D3DPTEXTURECAPS_POW2) {			// Surface breite und hoehe muss eine 2er Potenz sein
			while (width<s->width) width<<=1;
			while (height<s->height) height<<=1;
			DLOG ("   Textur angepasst (POW2): (%u/%u) -> (%u/%u)",s->width,s->height,width,height);
			s->width=width;
			s->height=height;
		}
		if (primarydx9->caps.TextureCaps&D3DPTEXTURECAPS_SQUAREONLY) {			// Nur Rechtecke sind erlaubt
			width=s->width;
			height=s->height;
			if (width<height) width=height;
			else height=width;
			DLOG ("   Textur angepasst (SQUARE): (%u/%u) -> (%u/%u)",s->width,s->height,width,height);
			s->width=width;
			s->height=height;
		}
		DLOG("   maxwidth=%u, maxheight=%u",primarydx9->caps.MaxTextureWidth,primarydx9->caps.MaxTextureHeight);
		if (s->width>(int)primarydx9->caps.MaxTextureWidth || s->height>(int)primarydx9->caps.MaxTextureHeight) {
			free(s->EngineSpecific);
			s->EngineSpecific=NULL;
			SetError(1057,"maxwidth=%u, maxheight=%u",primarydx9->caps.MaxTextureWidth,primarydx9->caps.MaxTextureHeight);
			return 0;
		}
		dx9->Pool=D3DPOOL_MANAGED;
		dx9->Type=SurfaceTypeTexture;
		// Funktionen initialisieren
		s->Lock=LockTexture;
		s->Unlock=UnlockTexture;
		s->DestroySurface=DestroyTexture;
		hr=primarydx9->g_pd3dDevice->CreateTexture(s->width,s->height,1,0,format,dx9->Pool,
			(IDirect3DTexture9**)&dx9->backbuffer,NULL);


	} else {
		dx9->Type=SurfaceTypeOffscreenPlain;
		// Funktionen initialisieren
		s->Blt=Blt;
		s->Lock=LockSurface;
		s->Unlock=UnlockSurface;
		s->DestroySurface=DestroySurface;

		hr=primarydx9->g_pd3dDevice->CreateOffscreenPlainSurface(s->width,s->height,format,dx9->Pool,
			&dx9->backbuffer,NULL);
	}
	if (hr!=D3D_OK) {
		free(s->EngineSpecific);
		s->EngineSpecific=NULL;
		SetError(1050,hr,"CreateOffscreenPlainSurface: %s",DXGetErrorDescription(hr));
		return 0;
	}
	s->bitdepth=GetBitdepth(s->rgbformat);
	s->bytes_per_pixel=s->bitdepth/8;
	dx9->surface=surface;
	dx9->g_pd3dDevice=primarydx9->g_pd3dDevice;
	dx9->engine=p;
	dx9->window=NULL;




	return 1;

#else
	SetError(1049,"DX9");
	return 0;
#endif
}




#ifdef HAVE_DX9

static int BeginScene(struct tagSURFACE *surface)
{
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	if (dx9->InScene) return 1;
	hr=dx9->g_pd3dDevice->BeginScene();
	if (hr!=D3D_OK) {
		SetError(1054,hr,"BeginScene: %s",DXGetErrorDescription(hr));
		return 0;
	}
	dx9->InScene=true;
	return 1;
}

static int EndScene(struct tagSURFACE *surface)
{
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	if (!dx9->InScene) return 1;
	hr=dx9->g_pd3dDevice->EndScene();
	dx9->InScene=false;
	return 1;
}

static int BeginSprites (struct tagSURFACE *surface)
{
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	if (!dx9->pD3DSprite) {
		SetError(1055);
		return 0;
	}
	hr=dx9->pD3DSprite->Begin(D3DXSPRITE_ALPHABLEND);
	if (hr!=D3D_OK) {
		SetError(1056,hr,"BeginSprite: %s",DXGetErrorDescription(hr));
		return 0;
	}
	dx9->InSprite=true;
	return 1;

}

static int DrawSprite (struct tagSURFACE *target, int x, int y, struct tagSURFACE *source, const RECT *SrcRect, COLOR c)
{
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)target->EngineSpecific;
	if (!dx9->pD3DSprite) {
		SetError(1055);
		return 0;
	}
	DX9Surface *src=(DX9Surface*)source->EngineSpecific;
	if (src->Type!=SurfaceTypeTexture) {
		return 0;
	}
	D3DXVECTOR3 pos;
	pos.x=(float)x;
	pos.y=(float)y;
	pos.z=0.0f;
	if (!dx9->InSprite) {
		if (!BeginSprites(target)) return 0;
		hr=dx9->pD3DSprite->Draw((LPDIRECT3DTEXTURE9)src->backbuffer,SrcRect,NULL,&pos,c);
		EndSprites(target);
	} else {
		hr=dx9->pD3DSprite->Draw((LPDIRECT3DTEXTURE9)src->backbuffer,SrcRect,NULL,&pos,c);
	}
	if (hr!=D3D_OK) {
		SetError(1056,hr,"DrawSprite: %s",DXGetErrorDescription(hr));
		return 0;
	}
	return 1;
}

static int EndSprites (struct tagSURFACE *surface)
{
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	if (!dx9->pD3DSprite) {
		SetError(1055);
		return 0;
	}
	hr=dx9->pD3DSprite->End();
	if (hr!=D3D_OK) {
		SetError(1056,hr,"EndSprite: %s",DXGetErrorDescription(hr));
		return 0;
	}
	dx9->InSprite=false;
	return 1;
}




static int SetCaption (struct tagSURFACE *surface, const char *title)
{
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	SetWindowText(dx9->hwnd,title);
    return 1;
}

static int PostMessage(struct tagSURFACE *surface, unsigned int msg, void *param1, void *param2)
{
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	if (!dx9) return 0;
	if (PostMessage(dx9->hwnd,msg,(WPARAM)param1,(LPARAM)param2)) return 1;
	return 0;
}
#endif

int CEngineDX9::SendEvent(CSurface *surface, void *object, CAssocArray *msg)
{

#ifdef HAVE_DX9
	SURFACE *s=surface->GetSurfaceParams();
	if (!s) return 0;
	DX9Surface *dx9=(DX9Surface*)s->EngineSpecific;
	if (!dx9) return 0;

	if (PostMessage(dx9->hwnd,WM_USER,(WPARAM)object,(LPARAM)msg)) return 1;
#endif
	return 0;
}

#ifdef HAVE_DX9
static int DestroyDevice (struct tagSURFACE *surface)
{
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	DX9private *engine=dx9->engine;
	// Falls Surface noch gelockt ist, unlocken wir es zuerst
	if (surface->lockcount>0) {
		hr=dx9->backbuffer->UnlockRect();
		if (hr!=D3D_OK) {
			SetError(1010,hr,"UnlockRect: %s",DXGetErrorDescription(hr));
			return 0;
		}
		surface->lockcount=0;
	}
	// Falls wir einen Backbuffer haben, geben wir diesen zuerst frei
	if (dx9->backbuffer) {
		dx9->backbuffer->Release();
		dx9->backbuffer=NULL;
	}

	// Falls wir ein Sprite-Interface haben, l�schen wir es
	if (dx9->pD3DSprite) {
		dx9->pD3DSprite->Release();
		dx9->pD3DSprite=NULL;
	}

	// Nun nehmen wir das Surface aus der Fensterkette
	if (dx9->next) dx9->next->previous=dx9->previous;
	if (dx9->previous) dx9->previous->next=dx9->next;
	if (engine->first==dx9) engine->first=dx9->next;

	if (engine->PrimarySurface==dx9->surface) engine->PrimarySurface=NULL;
	if (engine->PrimaryWindow==dx9->window) engine->PrimaryWindow=NULL;
	// Device zerst�ren
	dx9->g_pd3dDevice->Release();
	// Fenster l�schen
	::DestroyWindow(dx9->hwnd);
	// Speicher freigeben
	free(dx9);
	surface->EngineSpecific=NULL;
	return 1;
}

static int LockDevice (SURFACE *surface)
{
	if (surface->lockcount>0) {
		surface->lockcount++;
		return 1;
	}
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;

	if (!dx9->backbuffer) {
		hr=dx9->g_pd3dDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&dx9->backbuffer);
		if (hr!=D3D_OK) {
			SetError(1009,hr,"GetBackBuffer: %s",DXGetErrorDescription(hr));
			return 0;
		}
	}

	D3DLOCKED_RECT r;
	hr=dx9->backbuffer->LockRect(&r,NULL,0);
	if (hr!=D3D_OK) {
		SetError(1009,hr,"LockRect: %s",DXGetErrorDescription(hr));
		return 0;
	}

	surface->base=r.pBits;
	surface->base8=(ppluint8*)surface->base;
	surface->base16=(ppluint16*)surface->base;
	surface->base32=(ppluint32*)surface->base;

	surface->pitch=r.Pitch;
	surface->pitch8=surface->pitch;
	surface->pitch16=(surface->pitch>>1);
	surface->pitch32=(surface->pitch>>2);
	surface->lockcount=1;
	return 1;
}

static int UnlockDevice (SURFACE *surface)
{
	if (surface->lockcount>1) {
		surface->lockcount--;
		return 1;
	}
	if (surface->lockcount==0) return 1;
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	if (dx9->backbuffer) {
		hr=dx9->backbuffer->UnlockRect();
		if (hr!=D3D_OK) {
			SetError(1010,hr,"UnlockRect: %s",DXGetErrorDescription(hr));
			return 0;
		}
	}
	surface->lockcount=0;
	return 1;
}

static int FlipDevice (SURFACE *surface)
{
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	// Falls Surface noch gelockt ist, unlocken wir es zuerst
	if (surface->lockcount>0) {
		hr=dx9->backbuffer->UnlockRect();
		if (hr!=D3D_OK) {
			SetError(1010,hr,"UnlockRect: %s",DXGetErrorDescription(hr));
			return 0;
		}
		surface->lockcount=0;
	}

	// Falls wir einen Backbuffer haben, geben wir diesen zuerst frei
	if (dx9->backbuffer) {
		dx9->backbuffer->Release();
		dx9->backbuffer=NULL;
	}
	// Falls wir noch Sprites offen haben, schliessen wir diese
	if (dx9->InSprite) {
		dx9->pD3DSprite->End();
		dx9->InSprite=false;
	}
	// Falls noch kein EndScene erfolgte, machen wir das nun
	if (dx9->InScene) dx9->g_pd3dDevice->EndScene();
	dx9->InScene=false;


	hr=dx9->g_pd3dDevice->Present(NULL,NULL,NULL,NULL);
	if (hr!=D3D_OK) {
		SetError(1011,hr,"Present: %s",DXGetErrorDescription(hr));
		return 0;
	}
	return 1;
}

static int ClearDevice(struct tagSURFACE *surface, COLOR c)
{
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	hr=dx9->g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, c, 1.0f, 0 );
	if (hr!=D3D_OK) {
		SetError(1014,hr,"Clear: %s",DXGetErrorDescription(hr));
		return 0;
	}
	return 1;
}

/*******************************************************************************************
 * Surface Operations                                                                      *
 *******************************************************************************************/
static int DestroySurface (struct tagSURFACE *surface)
{
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	if (!dx9) return 1;
	// Falls Surface noch gelockt ist, unlocken wir es zuerst
	if (dx9->backbuffer) {
		if (surface->lockcount>0) {
			hr=dx9->backbuffer->UnlockRect();
			if (hr!=D3D_OK) {
				SetError(1010,hr,"UnlockRect: %s",DXGetErrorDescription(hr));
				return 0;
			}
			surface->lockcount=0;
		}
		dx9->backbuffer->Release();
	}
	free(dx9);
	surface->EngineSpecific=NULL;
	return 1;
}

static int LockSurface (struct tagSURFACE *surface)
{
	if (surface->lockcount>0) {
		surface->lockcount++;
		return 1;
	}
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	if (!dx9->backbuffer) {
		SetError(1053);
		return 0;
	}
	D3DLOCKED_RECT r;
	hr=dx9->backbuffer->LockRect(&r,NULL,0);
	if (hr!=D3D_OK) {
		SetError(1009,hr,"LockRect: %s",DXGetErrorDescription(hr));
		return 0;
	}
	surface->base=r.pBits;
	surface->base8=(ppluint8*)surface->base;
	surface->base16=(ppluint16*)surface->base;
	surface->base32=(ppluint32*)surface->base;

	surface->pitch=r.Pitch;
	surface->pitch8=surface->pitch;
	surface->pitch16=(surface->pitch>>1);
	surface->pitch32=(surface->pitch>>2);
	surface->lockcount=1;
	return 1;
}

static int UnlockSurface (struct tagSURFACE *surface)
{
	if (surface->lockcount>1) {
		surface->lockcount--;
		return 1;
	}
	if (surface->lockcount==0) return 1;
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	hr=dx9->backbuffer->UnlockRect();
	if (hr!=D3D_OK) {
		SetError(1010,hr,"UnlockRect: %s",DXGetErrorDescription(hr));
		return 0;
	}
	surface->lockcount=0;
	return 1;
}

static int Blt (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y)
{
	HRESULT hr;
	DX9Surface *tgt=(DX9Surface*)target->EngineSpecific;
	DX9Surface *src=(DX9Surface*)source->EngineSpecific;
	if (!tgt) return 0;
	if (tgt->Type!=SurfaceTypeDevice) return 0;
	if (!src->backbuffer) return 0;


	if (!tgt->backbuffer) {
		hr=tgt->g_pd3dDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&tgt->backbuffer);
		if (hr!=D3D_OK) {
			SetError(1009,hr,"GetBackBuffer: %s",DXGetErrorDescription(hr));
			return 0;
		}

	}
	if (!tgt->InScene) {
		if (!target->BeginScene(target)) return 0;
	}
	if (src->Type==SurfaceTypeTexture) {
		return DrawSprite(target,x,y,source,srect,0xffffffff);
	} else if (src->Pool==D3DPOOL_SYSTEMMEM) {
		POINT d;
		d.x=x;
		d.y=y;

		hr=tgt->g_pd3dDevice->UpdateSurface(src->backbuffer,srect,tgt->backbuffer,&d);
		if (hr!=D3D_OK) {
			SetError(1014,hr,"UpdateSurface: %s",DXGetErrorDescription(hr));
			return 0;
		}
		return 1;
	} else if (src->Pool==D3DPOOL_DEFAULT || src->Pool==D3DPOOL_MANAGED) {
		RECT r;
		r.left=x;
		r.top=y;
		r.right=x+source->width;
		r.bottom=y+source->height;
		hr=tgt->g_pd3dDevice->StretchRect(src->backbuffer,srect,tgt->backbuffer,&r,D3DTEXF_NONE);
		if (hr!=D3D_OK) {
			SetError(1014,hr,"StretchRect: %s",DXGetErrorDescription(hr));
			return 0;
		}
		return 1;
	}
	return 0;
}


/*******************************************************************************************
 * Texture Operations                                                                      *
 *******************************************************************************************/
static int DestroyTexture (struct tagSURFACE *surface)
{
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	// Falls Surface noch gelockt ist, unlocken wir es zuerst
	if (dx9->backbuffer) {
		if (surface->lockcount>0) {
			hr=((IDirect3DTexture9*)dx9->backbuffer)->UnlockRect(1);
			if (hr!=D3D_OK) {
				SetError(1010,hr,"UnlockRect: %s",DXGetErrorDescription(hr));
				return 0;
			}
			surface->lockcount=0;
		}
		((IDirect3DTexture9*)dx9->backbuffer)->Release();
	}
	free(dx9);
	surface->EngineSpecific=NULL;
	return 1;
}

static int LockTexture (struct tagSURFACE *surface)
{
	if (surface->lockcount>0) {
		surface->lockcount++;
		return 1;
	}
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	if (!dx9->backbuffer) {
		SetError(1053);
		return 0;
	}
	D3DLOCKED_RECT r;
	hr=((IDirect3DTexture9*)dx9->backbuffer)->LockRect(0,&r,NULL,0);
	if (hr!=D3D_OK) {
		SetError(1009,hr,"LockRect: %s",DXGetErrorDescription(hr));
		return 0;
	}
	surface->base=r.pBits;
	surface->base8=(ppluint8*)surface->base;
	surface->base16=(ppluint16*)surface->base;
	surface->base32=(ppluint32*)surface->base;

	surface->pitch=r.Pitch;
	surface->pitch8=surface->pitch;
	surface->pitch16=(surface->pitch>>1);
	surface->pitch32=(surface->pitch>>2);
	surface->lockcount=1;
	return 1;
}

static int UnlockTexture (struct tagSURFACE *surface)
{
	if (surface->lockcount>1) {
		surface->lockcount--;
		return 1;
	}
	if (surface->lockcount==0) return 1;
	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)surface->EngineSpecific;
	hr=((IDirect3DTexture9*)dx9->backbuffer)->UnlockRect(0);
	if (hr!=D3D_OK) {
		SetError(1010,hr,"UnlockRect: %s",DXGetErrorDescription(hr));
		return 0;
	}
	surface->lockcount=0;
	return 1;
}



#endif

/*
 * Message-Loop
 */

int CEngineDX9::StartMessageLoop()
{
#ifdef HAVE_DX9
	MSG msg;
	while( GetMessage( &msg, NULL, 0, 0 ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	return 1;
#else
	SetError(1049,"DX9");
	return 0;
#endif
}

#ifdef HAVE_DX9
static void MouseEvent(DX9Surface *dx9, DX9_MSG *m, EVENT *e)
{
	long k=(long)m->wParam&0xffff;
	dx9->maus.x=(long)m->lParam&0xffff;
	dx9->maus.y=(long)(m->lParam>>16)&0xffffff;
	e->x=dx9->maus.x;
	e->y=dx9->maus.y;
	e->ascii=0;
	e->buttons=0;
	if (k&MK_CONTROL) e->buttons|=SYSKEY::CONTROL;
	if (k&MK_SHIFT) e->buttons|=SYSKEY::SHIFT;
	if (k&MK_LBUTTON) e->buttons|=SYSKEY::LBUTTON;
	if (k&MK_MBUTTON) e->buttons|=SYSKEY::MBUTTON;
	if (k&MK_RBUTTON) e->buttons|=SYSKEY::RBUTTON;
	if (k&MK_XBUTTON1) e->buttons|=SYSKEY::XBUTTON1;
	if (k&MK_XBUTTON2) e->buttons|=SYSKEY::XBUTTON2;

}

#define MSGTRANS(name,text) case name: return text;
static const char *TranslateMessage(UINT uMsg)
{
	switch (uMsg) {
		case WM_MOUSEMOVE: return "WM_MOUSEMOVE";
		case WM_LBUTTONDBLCLK: return "WM_LBUTTONDBLCLK";
		case WM_LBUTTONDOWN: return "WM_LBUTTONDOWN";
		case WM_LBUTTONUP: return "WM_LBUTTONUP";
		case WM_MBUTTONDBLCLK: return "WM_MBUTTONDBLCLK";
		case WM_MBUTTONDOWN: return "WM_MBUTTONDOWN";
		case WM_MBUTTONUP: return "WM_MBUTTONUP";
		case WM_RBUTTONDBLCLK: return "WM_RBUTTONDBLCLK";
		case WM_RBUTTONDOWN: return "WM_RBUTTONDOWN";
		case WM_RBUTTONUP: return "WM_RBUTTONUP";
		case WM_XBUTTONDBLCLK: return "WM_XBUTTONDBLCLK";
		case WM_XBUTTONDOWN: return "WM_XBUTTONDOWN";
		case WM_XBUTTONUP: return "WM_XBUTTONUP";
		case WM_MOUSEHOVER: return "WM_MOUSEHOVER";
		case WM_MOUSELEAVE: return "WM_MOUSELEAVE";
		case WM_MOUSEWHEEL: return "WM_MOUSEWHEEL";
		case WM_MOUSEACTIVATE: return "WM_MOUSEACTIVATE";
		case WM_SETCURSOR: return "WM_SETCURSOR";
		case WM_NCHITTEST: return "WM_NCHITTEST";

		case WM_SYSKEYDOWN: return "WM_SYSKEYDOWN";
		case WM_SYSKEYUP: return "WM_SYSKEYUP";

		case WM_NOTIFY: return "WM_NOTIFY";

		case WM_CONTEXTMENU: return "WM_CONTEXTMENU";

		MSGTRANS(WM_KEYDOWN,"WM_KEYDOWN")
		MSGTRANS(WM_KEYUP,"WM_KEYUP")
		MSGTRANS(WM_CHAR,"WM_CHAR")
		MSGTRANS(WM_DEADCHAR,"WM_DEADCHAR")
		MSGTRANS(WM_ENTERIDLE,"WM_ENTERIDLE");
		MSGTRANS(WM_SYSCHAR,"WM_SYSCHAR");
		MSGTRANS(WM_SYSCOMMAND,"WM_SYSCOMMAND");
		MSGTRANS(WM_INITMENU,"WM_INITMENU");
		MSGTRANS(WM_MENUCHAR,"WM_MENUCHAR");
		MSGTRANS(WM_MENUSELECT,"WM_MENUSELECT");
		MSGTRANS(WM_ENTERMENULOOP,"WM_ENTERMENULOOP");
		MSGTRANS(WM_CAPTURECHANGED,"WM_CAPTURECHANGED");
		MSGTRANS(WM_MOVING,"WM_MOVING");
		MSGTRANS(WM_EXITMENULOOP,"WM_EXITMENULOOP");
		MSGTRANS(WM_CANCELMODE,"WM_CANCELMODE");
		MSGTRANS(WM_NCMOUSEMOVE,"WM_NCMOUSEMOVE");
		MSGTRANS(WM_NCACTIVATE,"WM_NCACTIVATE");
		MSGTRANS(WM_GETTEXT,"WM_GETTEXT");
		MSGTRANS(WM_ACTIVATE,"WM_ACTIVATE");
		MSGTRANS(WM_ACTIVATEAPP,"WM_ACTIVATEAPP");
		MSGTRANS(WM_KILLFOCUS,"WM_KILLFOCUS");
		MSGTRANS(WM_WINDOWPOSCHANGING,"WM_WINDOWPOSCHANGING");
		MSGTRANS(WM_WINDOWPOSCHANGED,"WM_WINDOWPOSCHANGED");
		MSGTRANS(WM_NCPAINT,"WM_NCPAINT");
		MSGTRANS(WM_ERASEBKGND,"WM_ERASEBKGND");
		MSGTRANS(WM_SETFOCUS,"WM_SETFOCUS");
		MSGTRANS(WM_ENABLE,"WM_ENABLE");
		MSGTRANS(WM_PAINT,"WM_PAINT");
		MSGTRANS(WM_CLOSE,"WM_CLOSE");
		MSGTRANS(WM_NCDESTROY,"WM_NCDESTROY");
		MSGTRANS(WM_SYNCPAINT,"WM_SYNCPAINT");
		MSGTRANS(WM_DESTROY,"WM_DESTROY");
		MSGTRANS(WM_DISPLAYCHANGE,"WM_DISPLAYCHANGE");
		MSGTRANS(WM_MOVE,"WM_MOVE");
		MSGTRANS(WM_QUERYOPEN,"WM_QUERYOPEN");
		MSGTRANS(WM_SIZE,"WM_SIZE");
		MSGTRANS(WM_NCCALCSIZE,"WM_NCCALCSIZE");
		MSGTRANS(WM_TIMER,"WM_TIMER");
		MSGTRANS(WM_DROPFILES,"WM_DROPFILES");
		MSGTRANS(WM_NOTIFYFORMAT,"WM_NOTIFYFORMAT");
		MSGTRANS(WM_NCMOUSELEAVE,"WM_NCMOUSELEAVE");

		//MSGTRANS();
		//MSGTRANS();
		//MSGTRANS();
		//MSGTRANS();
		//MSGTRANS();
	}
	return "UNKNOWN";
}

#endif

int CEngineDX9::DispatchDX9Message(void *msg)
{
#ifdef HAVE_DX9
	MSG wmsg;
	DX9_MSG *m=(DX9_MSG *)msg;
	ppl6::tk::EVENT e;

	/*
	if (m->uMsg!=15) {
	ppl6::PrintDebug("Message: hwnd=%x, lParam=%x, wParam=%x, message=%i, x=%u, y=%u\n",
			m->hwnd,
			m->lParam,
			m->wParam,
			m->uMsg,
			m->pt.x,m->pt.y);
	}
	*/
	// Aktives Fenster finden
	DX9private *p=(DX9private *)priv;
	DX9Surface *dx9=p->first;
	while (dx9) {
		if (dx9->hwnd=m->hwnd) break;
		dx9=dx9->next;
	}
	if (!dx9) {
		return 0;
	}
	ppl6::tk::CWindow *win=dx9->window;
	LockWidgets();

	switch (m->uMsg) {
		case WM_DESTROY:
			if (dx9->window->WindowCloseRequest(dx9->window)) {
				dx9->window->WindowClose(dx9->window);
				PostQuitMessage(0);
			}
			break;
		case WM_PAINT:
			dx9->window->NeedRedraw();
			break;
		case WM_MOUSEMOVE:
			MouseEvent(dx9,m,&e);
			e.type=tk::EVENTTYPE::MOUSEMOTION;
			win->MouseEvent(p->PrimaryWindow,e.x,e.y,e.buttons);
			win->DispatchMouseEvent(&e);
			break;
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
			MouseEvent(dx9,m,&e);
			e.type=tk::EVENTTYPE::MOUSEDOWN;
			win->MouseEvent(p->PrimaryWindow,e.x,e.y,e.buttons);
			win->DispatchMouseEvent(&e);
			break;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_XBUTTONUP:
			MouseEvent(dx9,m,&e);
			e.type=tk::EVENTTYPE::MOUSEUP;
			win->MouseEvent(p->PrimaryWindow,e.x,e.y,e.buttons);
			win->DispatchMouseEvent(&e);
			break;
		default:
			ppl6::PrintDebug("%llu: Message %i (0x%X): %s, hwnd=%x, lParam=%x, wParam=%x, x=%u, y=%u\n",
				ppl6::GetTime(),
				m->uMsg,
				m->uMsg,
				TranslateMessage(m->uMsg),
				m->hwnd,
				m->lParam,
				m->wParam,
				m->pt.x,m->pt.y);
	}
	// Falls das Fenster einen eigenen Grafikthread hat, machen wir nichts weiter
	if (!win->ExternalGraphicThread) {
		// Wenn es keine weiteren Nachrichten mehr gibt, pr�fen wir, ob ein Redraw n�tig ist
		if (!PeekMessage(&wmsg,dx9->hwnd,0,0,PM_NOREMOVE)) {
			if (dx9->window->RedrawIfNeeded()) {
				dx9->surface->Flip();
			}
		}
	}
	UnlockWidgets();
	return 1;
#else
	return 1;
#endif
}

} // EOF namespace grafix
} // EOF namespace ppl6

