/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6). 
 * Web: http://www.pfp.de/ppl/
 *  
 * $Author: patrick $
 * $Revision: 1.8 $
 * $Date: 2008/04/15 19:39:31 $
 * $Id: GrafixEngineDX9.h,v 1.8 2008/04/15 19:39:31 patrick Exp $
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

#include <windows.h>
#include <D3d9.h>
#include <dxerr9.h>
#include <d3dx9math.h>
#include <d3dx9core.h>

namespace ppl6 {

class CGrafixEngineDX9;

typedef struct {
	HWND hwnd;
	UINT uMsg;
	WPARAM wParam;
	LPARAM lParam;
} WINDOWS_MESSAGE;

typedef struct tagWindowPrivate{
	struct tagWindowPrivate *previous, *next;
	CWindow					*window;
	HWND					hwnd;
	CSurface				*surface;
} WINDOW_PRIVATE;

typedef struct {
	UINT		Levels;
	DWORD		Usage;
	D3DFORMAT	Format;
	DWORD		Pool;
	ppldd		BehaviorFlags;
} PPLDX9CreateStruct;

typedef struct {
	CGrafixEngineDX9		*engine;
	HWND					hwnd;
	IDirect3D9				*pD3D;			// Kopie aus EnginePrivate
	IDirect3DSurface9		*surface;
	IDirect3DTexture9		*texture;
	IDirect3DDevice9		*pD3DD9;
	PPLDX9CreateStruct		create;
	D3DPRESENT_PARAMETERS	d3dpp;
	D3DVIEWPORT9			pViewport;
	int						SurfaceType;
	//ppluint32				lockcounter;
} SURFACE_PRIVATE;


class CGrafixEngineDX9 : public CGrafixEngine
{
	private:
		CpplMutex				Mutex;

		HINSTANCE				instance;
		LPDIRECT3D9				g_pD3D;
		unsigned int			adapter;
		D3DDISPLAYMODE			d3ddm;
		WINDOW_PRIVATE			*windowchain;
		int						ScreenRGBFormat;

		int				GenerateCreateParameter(SURFACE *surface);



	public:
		CGrafixEngineDX9();
		virtual ~CGrafixEngineDX9();
		virtual int		Init(CApplication *app, CGrafix *grafix);
		virtual int		EnumAdapterModes(CpplAssocArray *result, RGBFormat::RGBFormat format, int adapter=-1);
		virtual int		EnumAdapters(CpplAssocArray *result);
		virtual int		DisplayWindow(CWindow *window);
		virtual int		DestroyWindow(CWindow *window);
		virtual int		ProcessMessage(void *msg);
		virtual int		MessageLoop();
		//virtual int		CreateSurface(SURFACE *surface);
		virtual int		CreatePrimarySurface(SURFACE *surface);

		int				DX9toRGBFORMAT(D3DFORMAT format);
		D3DFORMAT		RGBFORMATtoDX9(int format);

};



} // EOF Namespace ppl6
