/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.1 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: GFXEngineDX9.cpp,v 1.1 2010/02/12 19:43:48 pafe Exp $
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

#ifdef HAVE_DX9
//#include <d3d9types.h>
#include <D3D9.h>
#include <Dxerr.h>
#include <D3dx9core.h>
#endif


#include "ppl6-grafix.h"
#include "ppl6-tk.h"

using namespace ppl6;
using namespace ppl6::grafix;
using namespace ppl6::tk;

namespace ppl6 {
namespace grafix {

/*!\class GFXEngineDX9
 * \ingroup PPLGroupGrafik
 * \brief Grafik-Engines auf Basis von DirectX 9
 */


static GFXEngineDX9 *MyEngine=NULL;

#ifdef HAVE_DX9
/*!\typedef struct DX9private
 * \relates GFXEngineDX9
 * \brief Private Daten der DirectX 9 - Engine
 */
typedef struct {
	LPDIRECT3D9 g_pD3D;
	HINSTANCE 	hInstance;
	struct tagDX9Surface *primaryWindow;
//	CAssocArray *adapters;
//	tk::CWindow		*PrimaryWindow;
//	CSurface	*PrimarySurface;
	struct tagDX9Surface	*first;		// Es werden nur Fenster/Fullscreens gespeichert
} DX9private;

/*!\typedef DX9_MSG
 * \relates GFXEngineDX9
 * \brief Inhalt einer Windows-Nachricht
 */
typedef struct {
	HWND	hwnd;
	UINT	uMsg;
	WPARAM	wParam;
	LPARAM	lParam;
	POINT	pt;
} DX9_MSG;

/*!\relates GFXEngineDX9
 * \brief Surface-Typen
 */
enum DX9SurfaceType {
	SurfaceTypeDevice,
	SurfaceTypeOffscreenPlain,
	SurfaceTypeTexture
};


/*!\typedef tagDX9Surface
 * \relates GFXEngineDX9
 * \brief Private Daten eines DX9-Surface
 */
typedef struct tagDX9Surface {
	struct tagDX9Surface	*next, *previous;	//!\brief Nur für Fenster/Fullscreens
	RGBFormat			rgbformat;
	tk::Widget			*widget;			// Nur bei Fenster/Fullscreens
	CSurface			*surface;
	IDirect3DDevice9	*g_pd3dDevice;		// Wird nur bei Windows und Fullscreen verwendet, bei Offscreen-Surfaces und Texturen zeigt er auf das Device, mit dem es erstellt wurde
	IDirect3DSurface9	*backbuffer;		// Bei Devices zeigt der Ptr auf den Backbuffer, bei Surfaces/Textures auf das Surface
	D3DCAPS9			caps;				// Nur bei Fenster/Fullscreens
	DX9SurfaceType		Type;				// Gibt den Typ des Surface an
	D3DPOOL				Pool;
	CDrawable			lock;
	int					lockcount;
	int					adapter;
	// Die Folgenden Parameter werden nur bei Fenstern verwendet
	HWND hwnd;
	D3DPRESENT_PARAMETERS d3dpp;
	DWORD BehaviorFlags;
	D3DDISPLAYMODE pMode;
	POINT	maus;
	bool	InScene;
	bool	InSprite;
} DX9Surface;

#endif

#ifdef HAVE_DX9

static D3DFORMAT RGBFORMATtoDX9(const RGBFormat &format)
{
	switch (format.format()) {
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

static RGBFormat DX9toRGBFORMAT(D3DFORMAT format)
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
			return RGBFormat::X1R5G5B5;
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
	if (!MyEngine->dispatchDX9Message(&m)) return DefWindowProc(hwnd,uMsg,wParam,lParam);
	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

static HWND CreateWindow(tk::Widget &widget, HINSTANCE instance)
{
	// Create the application's window. PPL6Window
	int flags=widget.windowFlags();
	RECT wRect;
	DWORD dwExStyle=0;
	if (flags&Widget::AllowDrop) dwExStyle|=WS_EX_ACCEPTFILES;
	if ((flags&Widget::TopMost) || (flags&Widget::Fullscreen)) {
		dwExStyle|=WS_EX_TOPMOST;
	}
	DWORD dwStyle=WS_OVERLAPPED|WS_VISIBLE;
	if (flags&Widget::MinimizeButton) dwStyle|=WS_MINIMIZEBOX;
	if (flags&Widget::MaximizeButton) dwStyle|=WS_MAXIMIZEBOX;
	if (flags&Widget::SystemMenue) dwStyle|=WS_SYSMENU;
	if (flags&Widget::HasCaption) dwStyle|=WS_CAPTION;
	if (flags&Widget::Resizeable) dwStyle|=WS_SIZEBOX;

	// Die gewuenschte Groesse des ClientRects angeben
	wRect.left=0;
	wRect.top=0;
	wRect.bottom=widget.height();
	wRect.right=widget.width();


	if (!AdjustWindowRectEx(&wRect,dwStyle,false,dwExStyle)) {
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


	// Fenster erstellen
	HWND hWnd = ::CreateWindowEx(dwExStyle, "PPL6Window",
		(const char*)widget.windowTitle(),
		dwStyle, widget.x(),widget.y(),
		width, height, GetDesktopWindow(), NULL,
		instance, NULL );
	if (!hWnd) {
		SetError(1008,"CreateWindowEx failed");
		return 0;
	}
	return hWnd;
}


static int CreateDevice(LPDIRECT3D9 g_pD3D, DX9Surface *dx9, tk::Widget &widget)
{
	int flags=widget.windowFlags();
	dx9->rgbformat=widget.rgbFormat();
	// Behavior-Flags
	dx9->BehaviorFlags=0;
	if (flags&Widget::Multithreaded) dx9->BehaviorFlags|=D3DCREATE_MULTITHREADED;
	if (flags&Widget::SoftwareVertexProcessing) dx9->BehaviorFlags|=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	else dx9->BehaviorFlags|=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	if (flags&Widget::FPUPreserve) dx9->BehaviorFlags|=D3DCREATE_FPU_PRESERVE;
	// D3DPRESENT_PARAMETERS
	ZeroMemory( &dx9->d3dpp, sizeof(dx9->d3dpp) );
	if (flags&Widget::Fullscreen) {
		dx9->d3dpp.BackBufferFormat=(D3DFORMAT)RGBFORMATtoDX9(widget.rgbFormat());
		if (dx9->rgbformat==RGBFormat::unknown || dx9->d3dpp.BackBufferFormat==D3DFMT_UNKNOWN) {
			SetError(1051);
			return 0;
		}
		dx9->d3dpp.Windowed = FALSE;
	} else {
		dx9->d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		//d3dpp.BackBufferFormat=(D3DFORMAT)RGBFORMATtoDX9(s->rgbformat);
		dx9->d3dpp.Windowed = TRUE;
	}
	dx9->d3dpp.BackBufferWidth=widget.width();
	dx9->d3dpp.BackBufferHeight=widget.height();
	if (flags&Widget::Backbuffer) dx9->d3dpp.BackBufferCount=1;
	if (flags&Widget::TrippleBuffer) dx9->d3dpp.BackBufferCount=2;

	dx9->d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;

	if (flags&Widget::Lockable)	dx9->d3dpp.Flags|=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	if ((flags&Widget::ZBuffer) || (flags&Widget::StencilBuffer) ) {
		dx9->d3dpp.EnableAutoDepthStencil=true;
		dx9->d3dpp.AutoDepthStencilFormat=D3DFMT_D16;		// 32-Bit
		if (flags&Widget::StencilBuffer) {
			//d3dpp.AutoDepthStencilFormat=D3DFMT_D24S8;	// 24-Bit Z-Buffer, 8-Bit Stencil
		}
	}
	// Beim Flip auf VSYNC des Monitors warten?
	if (flags&Widget::WaitVsync) dx9->d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_ONE;		// Ja, zwingend
	else dx9->d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;		// Nein
	//if (s->frequency) dx9->d3dpp.FullScreen_RefreshRateInHz=s->frequency;

	// Device erstellen
	HRESULT hr;
	hr=g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, dx9->hwnd,
		dx9->BehaviorFlags,
		&dx9->d3dpp, &dx9->g_pd3dDevice );
	if (hr!=D3D_OK) {
		SetError(1050,hr,"%s",DXGetErrorDescription(hr));
		return 0;
	}
	dx9->g_pd3dDevice->GetDeviceCaps(&dx9->caps);


	// DisplayMode
	hr=dx9->g_pd3dDevice->GetDisplayMode(0,&dx9->pMode);
	if (hr!=D3D_OK) {
		dx9->g_pd3dDevice->Release();	// Device l�schen
		SetError(1014,hr,"%s",DXGetErrorDescription(hr));
		return 0;
	}

	dx9->rgbformat=DX9toRGBFORMAT(dx9->pMode.Format);
	if (dx9->rgbformat==RGBFormat::unknown) {	// Hier stimmt was nicht
		dx9->g_pd3dDevice->Release();	// Device l�schen
		SetError(1013);
		return 0;
	}
	widget.setRGBFormat(dx9->rgbformat);
	
	dx9->Type=SurfaceTypeDevice;

	// Viewport setzen
	D3DVIEWPORT9	pViewport;
	pViewport.X=0;
	pViewport.Y=0;
	pViewport.Width=widget.width();
	pViewport.Height=widget.height();
	pViewport.MinZ=0.0;
	pViewport.MaxZ=1.0;

	dx9->g_pd3dDevice->SetViewport(&pViewport);

	return 1;
}


#endif


GFXEngineDX9::GFXEngineDX9()
{
	priv=NULL;
}

GFXEngineDX9::~GFXEngineDX9()
{
	#ifdef HAVE_DX9
	if (priv) {
		DX9private *p=(DX9private *)priv;
		if (p->g_pD3D) p->g_pD3D->Release();
		free(priv);
	}
	#endif
	if (MyEngine==this) MyEngine=NULL;
}

CString GFXEngineDX9::name() const
{
	return CString("DX9");
}



int GFXEngineDX9::Init()
{
#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return 0;
#else
	if (MyEngine!=NULL) {
		SetError(1062,"DX9");
		return 0;
	}
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
	MyEngine=this;
	return 1;
#endif
}

bool GFXEngineDX9::isInit() const
{
#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return false;
#else
	if (priv==NULL || ((DX9private *)priv)->g_pD3D==NULL) {
		SetError(1002,"ppl6::grafix::GFXEngineDX9");
		return false;
	}
	return true;
#endif
}

#ifdef _D3D9_H_
IDirect3D9 * GFXEngineDX9::device()
{
	if (!isInit()) return NULL;
	DX9private *p=(DX9private *)priv;
	return p->g_pD3D;
}
#else
void * GFXEngineDX9::device()
{
	return NULL;
}
#endif


int GFXEngineDX9::getAdapterCount()
{
#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return 0;
#else
	if (!isInit()) return 0;
	DX9private *p=(DX9private *)priv;
	UINT count=p->g_pD3D->GetAdapterCount();
	if (count==0) {
		SetError(1063,"DX9");
	}
	return count;
#endif
}

int GFXEngineDX9::getAdapterInfo(int adapter, AdapterIdentifier &info)
{
#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return 0;
#else
	if (!isInit()) return 0;
	DX9private *p=(DX9private *)priv;
	HRESULT res;
	D3DADAPTER_IDENTIFIER9 a;
	if (adapter<0) adapter=D3DADAPTER_DEFAULT;
	res=p->g_pD3D->GetAdapterIdentifier(adapter,0,&a);
	if (res!=D3D_OK) {
		SetError(1064);
		return 0;
	}
	info.adapterId=adapter;
	info.DriverName=a.Driver;
	info.Description=a.Description;
	info.version=a.DriverVersion.QuadPart;
	info.major_version = HIWORD(a.DriverVersion.HighPart);
	info.minor_version=LOWORD(a.DriverVersion.HighPart);
	info.sub_version = HIWORD(a.DriverVersion.LowPart);
	info.build = LOWORD(a.DriverVersion.LowPart);

	// Aktueller Grafikmodus
	D3DDISPLAYMODE pMode;
	res=p->g_pD3D->GetAdapterDisplayMode(adapter,&pMode);
	if (res==D3D_OK) {
		info.currentHeight=pMode.Height;
		info.currentWidth=pMode.Width;
		info.currentRefreshRate=pMode.RefreshRate;
		info.currentRGBFormat=DX9toRGBFORMAT(pMode.Format);
	}

	// Adapter-Eigenschaften
	D3DCAPS9 Caps;
	res=p->g_pD3D->GetDeviceCaps(adapter,D3DDEVTYPE_HAL,&Caps);
	if (res==D3D_OK) {
		info.Caps.MaxActiveLights=Caps.MaxActiveLights;
		info.Caps.MaxTextureWidth=Caps.MaxTextureWidth;
		info.Caps.MaxTextureHeight=Caps.MaxTextureHeight;
		info.Caps.MaxTextureAspectRatio=Caps.MaxTextureAspectRatio;
		info.Caps.PixelShaderVersion=Caps.PixelShaderVersion;
		info.Caps.VertexShaderVersion=Caps.VertexShaderVersion;
	}

	return 1;
#endif
}

int GFXEngineDX9::getAdapterModeCount(int adapter, const RGBFormat &format)
{
	#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return 0;
#else
	if (!isInit()) return 0;
	DX9private *p=(DX9private *)priv;
	if (adapter<0) adapter=D3DADAPTER_DEFAULT;

	D3DFORMAT f=RGBFORMATtoDX9(format);
	if (f==D3DFMT_UNKNOWN) {
		return 0;
	}
	UINT count=p->g_pD3D->GetAdapterModeCount(adapter,f);
	if (count==0) {
		SetError(1064);
	}
	return count;
#endif
}



int GFXEngineDX9::getAdapterModeInfo(int adapter, const RGBFormat &format, int mode, AdapterMode &info)
{
#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return 0;
#else
	if (!isInit()) return 0;
	DX9private *p=(DX9private *)priv;
	if (adapter<0) adapter=D3DADAPTER_DEFAULT;
	HRESULT ret;
	D3DDISPLAYMODE m;
	D3DFORMAT f=RGBFORMATtoDX9(format);
	if (f==D3DFMT_UNKNOWN) {
		return 0;
	}
	if (adapter!=D3DADAPTER_DEFAULT && (UINT)adapter>p->g_pD3D->GetAdapterModeCount(adapter,f)) {
		SetError(1064);
		return 0;
	}
	ret=p->g_pD3D->EnumAdapterModes(adapter,f,mode,&m);
	if (ret!=D3D_OK) {
		SetError(1066,"adapter=%i, mode=%i, format=%s",adapter,mode,(const char*)format.name());
		return 0;
	}
	info.format=DX9toRGBFORMAT(m.Format);
	info.width=m.Width;
	info.height=m.Height;
	info.refreshRate=m.RefreshRate;
	return 1;
#endif
}


bool GFXEngineDX9::createWindow(tk::Widget &widget)
{
#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return false;
#else
	if (!isInit()) return 0;
	DX9private *p=(DX9private *)priv;
	if (!p->g_pD3D) {
		SetError(1002,"ppl6::grafix::CEngineDX9");
		return false;
	}
	DX9Surface *dx9=(DX9Surface*)calloc(1,sizeof(DX9Surface));
	if (!dx9) {
		SetError(2);
		return false;
	}
	dx9->adapter=adapter();

	dx9->hwnd=CreateWindow(widget,p->hInstance);
	if (!dx9->hwnd) {
		free(dx9);
		return false;
	}
	::ShowWindow(dx9->hwnd,SW_SHOW);
	dx9->d3dpp.hDeviceWindow=dx9->hwnd;
	if (!CreateDevice(p->g_pD3D,dx9,widget)) {
		::DestroyWindow(dx9->hwnd);
		free(dx9);
		return 0;
	}
	p->primaryWindow=dx9;
	if (p->first) {
		p->first->previous=dx9;
		dx9->next=p->first;
	}
	p->first=dx9;
	widget.setGfxEngine(this);
	widget.setEnginePrivate(dx9);
	return true;

#endif

	SetError(1007,"Engine: %s, Function: createWindow", (const char*)name());
	return 0;
}

bool GFXEngineDX9::destroyWindow(tk::Widget &widget)
{
#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return false;
#else
	if (!isInit()) return 0;
	DX9private *p=(DX9private *)priv;
	if (!p->g_pD3D) {
		SetError(1002,"ppl6::grafix::CEngineDX9");
		return false;
	}

	DX9Surface *dx9=(DX9Surface*)widget.enginePrivate();
	if (!dx9) {
		SetError(1053);
		return false;
	}
	HRESULT hr;
	// Falls Device noch gelockt ist, unlocken wir es zuerst
	if (dx9->lockcount>0) {
		hr=dx9->backbuffer->UnlockRect();
		if (hr!=D3D_OK) {
			SetError(1010,hr,"UnlockRect: %s",DXGetErrorDescription(hr));
			return false;
		}
		dx9->lockcount=0;
	}
	// Falls wir einen Backbuffer haben, geben wir diesen zuerst frei
	if (dx9->backbuffer) {
		dx9->backbuffer->Release();
		dx9->backbuffer=NULL;
	}
	// Nun nehmen wir das Fenster aus der Fensterkette
	if (dx9->next) dx9->next->previous=dx9->previous;
	if (dx9->previous) dx9->previous->next=dx9->next;
	if (p->first==dx9) p->first=dx9->next;

	if (p->primaryWindow==dx9) p->primaryWindow=NULL;
	
	// Device zerstören
	dx9->g_pd3dDevice->Release();
	// Fenster löschen
	::DestroyWindow(dx9->hwnd);
	// Speicher freigeben
	free(dx9);
	widget.setGfxEngine(NULL);
	widget.setEnginePrivate(NULL);
	return true;
#endif
}


bool GFXEngineDX9::lockDevice(tk::Widget &widget, CDrawable &draw)
{
#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return false;
#else
	if (!isInit()) return 0;
	DX9Surface *dx9=(DX9Surface*)widget.enginePrivate();
	if (!dx9) {
		SetError(1053);
		return false;
	}
	if (dx9->lockcount>0) {
		dx9->lockcount++;
		draw=dx9->lock;
		return true;
	}

	HRESULT hr;
	if (!dx9->backbuffer) {
		hr=dx9->g_pd3dDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&dx9->backbuffer);
		if (hr!=D3D_OK) {
			SetError(1009,hr,"GetBackBuffer: %s",DXGetErrorDescription(hr));
			return false;
		}
	}
	D3DLOCKED_RECT r;
	hr=dx9->backbuffer->LockRect(&r,NULL,0);
	if (hr!=D3D_OK) {
		SetError(1009,hr,"LockRect: %s",DXGetErrorDescription(hr));
		return false;
	}
	dx9->lock.create(r.pBits,r.Pitch,widget.width(),widget.height(),widget.rgbFormat());
	dx9->lockcount=1;
	draw=dx9->lock;
	return true;
#endif
}

bool GFXEngineDX9::unlockDevice(tk::Widget &widget)
{
#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return false;
#else
	if (!isInit()) return 0;
	DX9Surface *dx9=(DX9Surface*)widget.enginePrivate();
	if (!dx9) {
		SetError(1053);
		return false;
	}
	if (dx9->lockcount>1) {
		dx9->lockcount--;
		return true;
	}
	if (dx9->lockcount==0) return true;
	HRESULT hr;
	if (dx9->backbuffer) {
		hr=dx9->backbuffer->UnlockRect();
		if (hr!=D3D_OK) {
			SetError(1010,hr,"UnlockRect: %s",DXGetErrorDescription(hr));
			return false;
		}
	}
	dx9->lockcount=0;
	return true;
#endif
}

bool GFXEngineDX9::updateDevice(tk::Widget &widget)
{
#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return false;
#else
	if (!isInit()) return 0;
	DX9Surface *dx9=(DX9Surface*)widget.enginePrivate();
	if (!dx9) {
		SetError(1053);
		return false;
	}
	HRESULT hr;
	// Falls wir einen Backbuffer haben, geben wir diesen zuerst frei
	if (dx9->backbuffer) {
		dx9->backbuffer->Release();
		dx9->backbuffer=NULL;
	}
	// Falls noch kein EndScene erfolgte, machen wir das nun
	if (dx9->InScene) dx9->g_pd3dDevice->EndScene();
	dx9->InScene=false;


	hr=dx9->g_pd3dDevice->Present(NULL,NULL,NULL,NULL);
	if (hr!=D3D_OK) {
		SetError(1011,hr,"Present: %s",DXGetErrorDescription(hr));
		return 0;
	}
	return true;
#endif
}



bool GFXEngineDX9::setWindowTitle(tk::Widget &widget)
{
	SetError(1007,"Engine: %s, Function: setWindowTitle", (const char*)name());
	return 0;
}

bool GFXEngineDX9::setWindowIcon(tk::Widget &widget)
{
	SetError(1007,"Engine: %s, Function: setWindowIcon", (const char*)name());
	return 0;
}


bool GFXEngineDX9::createSurface(CSurface &surface)
{
#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return false;
#else
	if (!isInit()) return 0;
	DX9private *p=(DX9private *)priv;
	if (!p->g_pD3D) {
		SetError(1002,"ppl6::grafix::CEngineDX9");
		return false;
	}
	if (!p->primaryWindow) {
		SetError(1052);
		return false;
	}
	DX9Surface *primarydx9=p->primaryWindow;

	// Bekanntes Farbformat?
	if (surface.rgbFormat()==RGBFormat::unknown) surface.setRGBFormat(p->primaryWindow->rgbformat);
	D3DFORMAT format=RGBFORMATtoDX9(surface.rgbFormat());
	if (format==D3DFMT_UNKNOWN) {
		SetError(1013);
		return false;
	}


	HRESULT hr;
	DX9Surface *dx9=(DX9Surface*)calloc(1,sizeof(DX9Surface));
	if (!dx9) {
		SetError(2);
		return false;
	}
	int flags=surface.surfaceFlags();
	dx9->rgbformat=surface.rgbFormat();

	// Welchen Pool verwenden wir?
	dx9->Pool=D3DPOOL_DEFAULT;
	if (!(flags&CSurface::VideoMemory)) dx9->Pool=D3DPOOL_SYSTEMMEM;
	if (flags&CSurface::Texture) {
		// Höhe und Breite müssen eine 2er Potenz sein, also 1,2,4,8,16,32,64,128....
		int width=2, height=2;
		if (primarydx9->caps.TextureCaps&D3DPTEXTURECAPS_POW2) {			// Surface breite und hoehe muss eine 2er Potenz sein
			while (width<surface.width()) width<<=1;
			while (height<surface.height()) height<<=1;
			DLOG ("   Textur angepasst (POW2): (%u/%u) -> (%u/%u)",surface.width(),surface.height(),width,height);
			surface.setSize(width,height);
		}
		if (primarydx9->caps.TextureCaps&D3DPTEXTURECAPS_SQUAREONLY) {			// Nur Rechtecke sind erlaubt
			width=surface.width();
			height=surface.height();
			if (width<height) width=height;
			else height=width;
			DLOG ("   Textur angepasst (SQUARE): (%u/%u) -> (%u/%u)",surface.width(),surface.height(),width,height);
			surface.setSize(width,height);
		}
		DLOG("   maxwidth=%u, maxheight=%u",primarydx9->caps.MaxTextureWidth,primarydx9->caps.MaxTextureHeight);
		if (surface.width()>(int)primarydx9->caps.MaxTextureWidth || surface.height()>(int)primarydx9->caps.MaxTextureHeight) {
			free(dx9);
			SetError(1057,"maxwidth=%u, maxheight=%u",primarydx9->caps.MaxTextureWidth,primarydx9->caps.MaxTextureHeight);
			return false;
		}
		dx9->Pool=D3DPOOL_MANAGED;
		dx9->Type=SurfaceTypeTexture;
		// Funktionen initialisieren
		hr=primarydx9->g_pd3dDevice->CreateTexture(surface.width(),surface.height(),1,0,format,dx9->Pool,
			(IDirect3DTexture9**)&dx9->backbuffer,NULL);
		if (hr!=D3D_OK) {
			free(dx9);
			SetError(1050,hr,"CreateTexture: %s",DXGetErrorDescription(hr));
			return false;
		}
	} else {
		dx9->Type=SurfaceTypeOffscreenPlain;
		// Funktionen initialisieren
		hr=primarydx9->g_pd3dDevice->CreateOffscreenPlainSurface(surface.width(),surface.height(),format,dx9->Pool,
			&dx9->backbuffer,NULL);
		if (hr!=D3D_OK) {
			free(dx9);
			SetError(1050,hr,"CreateOffscreenPlainSurface: %s",DXGetErrorDescription(hr));
			return false;
		}
	}
	dx9->g_pd3dDevice=primarydx9->g_pd3dDevice;
	dx9->surface=&surface;
	surface.setGfxEngine(this);
	surface.setEnginePrivate(dx9);
	return true;
#endif
}

bool GFXEngineDX9::destroySurface(CSurface &surface)
{
#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return false;
#else
	if (!isInit()) return 0;
	DX9private *p=(DX9private *)priv;
	if (!p->g_pD3D) {
		SetError(1002,"ppl6::grafix::CEngineDX9");
		return false;
	}
	if (surface.gfxEngine()!=this) {
		SetError(1073);
		return false;
	}
	DX9Surface *dx9=(DX9Surface*)surface.enginePrivate();
	if (!dx9) {
		SetError(1074);
		return false;
	}
	HRESULT hr;
	if (dx9->backbuffer) {
		if (dx9->Type==SurfaceTypeTexture) {
			// Fall die Textur noch gelockt ist, unlocken wir sie zuerst
			if (dx9->lockcount>0) {
				hr=dx9->backbuffer->UnlockRect();
				if (hr!=D3D_OK) {
					SetError(1010,hr,"UnlockRect: %s",DXGetErrorDescription(hr));
					return false;
				}
				dx9->lockcount=0;
			}
			dx9->backbuffer->Release();
		} else if (dx9->Type=SurfaceTypeOffscreenPlain) {
			// Falls Surface noch gelockt ist, unlocken wir es zuerst
			if (dx9->lockcount>0) {
				hr=dx9->backbuffer->UnlockRect();
				if (hr!=D3D_OK) {
					SetError(1010,hr,"UnlockRect: %s",DXGetErrorDescription(hr));
					return false;
				}
				dx9->lockcount=0;
			}
			dx9->backbuffer->Release();
		}
	}
	free(dx9);
	surface.setEnginePrivate(NULL);
	surface.setGfxEngine(NULL);
	return true;
#endif
}

bool GFXEngineDX9::lockSurface(CSurface &surface, CDrawable &draw)
{
#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return false;
#else
	if (surface.gfxEngine()!=this) {
		SetError(1073);
		return false;
	}
	DX9Surface *dx9=(DX9Surface*)surface.enginePrivate();
	if (!dx9) {
		SetError(1074);
		return false;
	}
	if (dx9->lockcount>0) {
		dx9->lockcount++;
		draw=dx9->lock;
		return true;
	}
	if (!dx9->backbuffer) {
		SetError(1053);
		return 0;
	}
	D3DLOCKED_RECT r;
	HRESULT hr;
	if (dx9->Type==SurfaceTypeTexture) {					// Texture
		hr=((IDirect3DTexture9*)dx9->backbuffer)->LockRect(0,&r,NULL,0);
	} else if (dx9->Type=SurfaceTypeOffscreenPlain) {		// Surface
		hr=dx9->backbuffer->LockRect(&r,NULL,0);
	} else {
		SetError(1075);
		return false;
	}
	if (hr!=D3D_OK) {
		SetError(1009,hr,"LockRect: %s",DXGetErrorDescription(hr));
		return false;
	}
	if (!dx9->lock.create(r.pBits,r.Pitch,surface.width(),surface.height(),surface.rgbFormat())) {
		PushError();
		if (dx9->Type==SurfaceTypeTexture) {					// Texture
			hr=((IDirect3DTexture9*)dx9->backbuffer)->UnlockRect(0);
		} else if (dx9->Type=SurfaceTypeOffscreenPlain) {		// Surface
			hr=dx9->backbuffer->UnlockRect();
		}
		PopError();
		return false;
	}
	draw=dx9->lock;
	dx9->lockcount=1;
	return true;
#endif
}

bool GFXEngineDX9::unlockSurface(CSurface &surface)
{
#ifndef HAVE_DX9
	SetError(1049,"DX9");
	return false;
#else
	if (surface.gfxEngine()!=this) {
		SetError(1073);
		return false;
	}
	DX9Surface *dx9=(DX9Surface*)surface.enginePrivate();
	if (!dx9) {
		SetError(1074);
		return false;
	}
	if (dx9->lockcount>1) {
		dx9->lockcount--;
		return true;
	}
	if (dx9->lockcount==0) return 1;
	HRESULT hr;
	if (dx9->Type==SurfaceTypeTexture) {					// Texture
		hr=((IDirect3DTexture9*)dx9->backbuffer)->UnlockRect(0);
	} else if (dx9->Type=SurfaceTypeOffscreenPlain) {		// Surface
		hr=dx9->backbuffer->UnlockRect();
	} else {
		SetError(1075);
		return false;
	}
	if (hr!=D3D_OK) {
		SetError(1010,hr,"UnlockRect: %s",DXGetErrorDescription(hr));
		return false;
	}
	dx9->lockcount=0;
	return true;
#endif
}

/*
 * Message-Loop
 */

bool GFXEngineDX9::startMessageLoop()
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
/*
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
*/

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


int GFXEngineDX9::dispatchDX9Message(void *msg)
{
#ifdef HAVE_DX9
	MSG wmsg;
	DX9_MSG *m=(DX9_MSG *)msg;
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
	ppl6::tk::Widget *win=dx9->widget;
	switch (m->uMsg) {
		case WM_DESTROY:
			//if (dx9->widget->WindowCloseRequest(dx9->window)) {
			//	dx9->widget->WindowClose(dx9->window);
				PostQuitMessage(0);
			//}
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


#endif
	return 1;
}


} // EOF namespace newgrafix
} // EOF namespace ppl6
