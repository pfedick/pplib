/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.44 $
 * $Date: 2009/12/09 18:25:59 $
 * $Id: ppl6-grafix.h,v 1.44 2009/12/09 18:25:59 patrick Exp $
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

#ifndef _PPL6_INCLUDE_GRAFIX6
#define _PPL6_INCLUDE_GRAFIX6
#ifndef _PPL6_INCLUDE
	#ifdef PPL6LIB
		#include "ppl6.h"
		//#include "ppl6-tk.h"
	#else
		#include <ppl6.h>
		//#include <ppl6-tk.h>
	#endif
#endif

#ifdef RGB
#undef RGB
#endif

#ifdef CreateWindow
#undef CreateWindow
#endif

#ifdef SendMessage
#undef SendMessage
#endif

namespace ppl6 {

namespace tk {
class CWindow;
}

namespace grafix {

#ifdef __LITTLE_ENDIAN__
union RGBA {
	struct { ppldb red,green,blue,alpha; };
	ppldd color;
};
#else
union RGBA {
	struct { ppldb alpha,red,green,blue; };
	ppldd color;
};
#endif

#ifdef COLOR
#undef COLOR
#endif

namespace RGBFormat {
	enum RGBFormat {
		unknown=0,
		Palette,
		R5G6B5,
		B5G6R5,
		R5G5B5,
		B5G5R5,
		R4G4B4,
		R8G8B8,
		B8G8R8,
		A8R8G8B8,
		A8B8G8R8,
		R3G3B2,
		A4R4G4B4,
		A1R5G5B5,
		X1R5G5B5,
		A8,
		A8R3G3B2,
		X8R8G8B8,
		X8B8G8R8,
		GREY8,
		GREYALPHA32,
		X16R16G16B16,
		A16R16G16B16
	};
}

namespace DRAWMETHOD {
enum DRAWMETHOD {
	BLT=1,
	COLORKEY,
	ALPHABLT,
	DIFFUSE,
	SPRITE
};
}


namespace COLORTYPE {
	enum COLORTYPE {
		unknown=		0x0000,
		PALETTE=		0x0001,
		RGB=			0x0002,
		RGB_ALPHA=		0x0004,
		GRAY=			0x0008,
		GRAY_ALPHA=		0x0010
	};
}

namespace WindowStyle {
	enum Style {
		NoFrame				=	0x00000001,
		Resizeable			=	0x00000002,
		AllowDrop			=	0x00000004,
		TopMost				=	0x00000008,
		PositionDefault		=	0x00000010,
		HasCaption			=	0x00000020,
	};
}

namespace Surface {
	namespace Flags {
		enum {
			Backbuffer					=	0x00000001,
			TrippleBuffer				=	0x00000002,
			Fullscreen					=	0x00000004,
			Hardware					=	0x00000008,
			Software					=	0x00000010,
			Lockable					=	0x00000020,
			VideoMemory					=	0x00000040,
			LocalMemory					=	0x00000080,
			Texture						=	0x00000100,
			ConvertColors				=	0x00000200,
			NoVsync						=	0x00000400,
			WaitVsync					=	0x00000800,
			ZBuffer						=	0x00001000,
			Stencil						=	0x00002000,
			SecurePrimitives			=	0x00004000,
			FPUPreserve					=	0x00008000,
			SoftwareVertexProcessing	=	0x00010000,
			Multithreaded				=	0x00020000,
			PrimarySurface				=	0x00040000
		};
	}
	namespace Types {
		enum {
			Unknown=0,
			Primary=1,
			Surface=2,
			Texture=3
		};
	}
}

typedef ppluint32 COLOR;

typedef struct {
	float red;
	float green;
	float blue;
	float alpha;
} RGBfloat;

typedef struct {
	int		width;
	int		height;
	int		pitch;
	int		bitdepth;
	int		colors;
	RGBFormat::RGBFormat	format;
} IMAGE;

class CSurface;
class CGrafix;
class CFilter;
class CFontEngine;
class CFontFile;
class CImageList;
class CImage;


// ACHTUNG: Bei Änderungen in der Reihenfolge muss auch die Assembler-Struktur
// in src/asm/common.asminc angepasst werden!
typedef struct tagSURFACE {
	//!\name Variablen
	//@{

	void *EngineSpecific;
	CSurface *Surface;
	void *base;
	ppluint8 *base8;
	ppluint16 *base16;
	ppluint32 *base32;
	int pitch;
	int pitch8;
	int pitch16;
	int pitch32;
	int width;
	int height;
	RECT clipper;
	// bis hierhin könnnen die Variablen in Assembler verwendet werden

	int flags;
	int style;
	int type;
	int frequency;
	int bytes_per_pixel;
	int lockcount;
	int bitdepth;
	int rgbformat;
	COLOR lastcolor;
	COLOR colorkey;
	int hascolorkey;
	int lastx, lasty;
	int originx, originy;
	char *alphatab;
	//@}

	//!\name Funktionen
	//@{
	int (*DestroySurface) (struct tagSURFACE *surface);
	int (*SetCaption) (struct tagSURFACE *surface, const char *title);
	int (*Lock) (struct tagSURFACE *surface);
	int (*Unlock) (struct tagSURFACE *surface);
	int (*Flip) (struct tagSURFACE *surface);

	int (*CLS) (struct tagSURFACE *surface, COLOR c);
	void (*PutPixel) (struct tagSURFACE *surface, int x, int y, COLOR c);
	void (*PutPixelSecure) (struct tagSURFACE *surface, int x, int y, COLOR c);
	void (*PutPixelFast) (struct tagSURFACE *surface, int x, int y, COLOR c);
	int (*AlphaPixel) (struct tagSURFACE *surface, int x, int y, COLOR c);
	int (*AlphaPixelSecure) (struct tagSURFACE *surface, int x, int y, COLOR c);
	int (*AlphaPixelFast) (struct tagSURFACE *surface, int x, int y, COLOR c);
	COLOR (*BlendPixel) (struct tagSURFACE *surface, int x, int y, COLOR c, float brightness);
	COLOR (*BlendPixel255) (struct tagSURFACE *surface, int x, int y, COLOR c, int brightness);
	COLOR (*GetPixel) (struct tagSURFACE *surface, int x, int y);
	COLOR (*GetPixelSecure) (struct tagSURFACE *surface, int x, int y);
	COLOR (*GetPixelFast) (struct tagSURFACE *surface, int x, int y);

	int (*Rect)(struct tagSURFACE *surface, RECT *r, COLOR c);
	int (*FillRect)(struct tagSURFACE *surface, RECT *r, COLOR c);
	int (*Xchange)(struct tagSURFACE *surface, int x1, int y1, int x2, int y2, COLOR farbe, COLOR ersatzfarbe);
	int (*Invert)(struct tagSURFACE *surface, int x1, int y1, int x2, int y2, COLOR farbe1, COLOR farbe2);
	int (*Negativ)(struct tagSURFACE *surface, int x1, int y1, int x2, int y2);

	COLOR (*RGB) (int r, int g, int b, int a);
	COLOR (*RGBBlend) (COLOR ground, COLOR top, float intensity);
	COLOR (*RGBBlend255) (COLOR ground, COLOR top, int intensity);
	COLOR (*Surface2RGB) (COLOR color);
	int (*LineAA) (struct tagSURFACE *surface, int x1, int y1, int x2, int y2,COLOR color, int strength);
	int (*Line) (struct tagSURFACE *surface, int x1, int y1, int x2, int y2,COLOR color);

	int (*Blt) (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y);
	int (*BltSoftware) (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y);
	int (*BltDiffuse) (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y, COLOR c);
	int (*BltDiffuseSoftware) (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y, COLOR c);
	int (*BltColorKey) (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y, COLOR c);
	int (*BltColorKeySoftware) (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y, COLOR c);
	int (*AlphaBlt) (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y);
	int (*AlphaBltSoftware) (struct tagSURFACE *target, struct tagSURFACE *source, RECT *srect, int x, int y);
	int (*BeginScene) (struct tagSURFACE *target);
	int (*EndScene) (struct tagSURFACE *target);
	int	(*PostMessage) (struct tagSURFACE *surface, unsigned int msg, void *param1, void *param2);
	int (*BeginSprites) (struct tagSURFACE *surface);
	int (*DrawSprite) (struct tagSURFACE *target, int x, int y, struct tagSURFACE *source, const RECT *SrcRect, COLOR c);
	int (*EndSprites) (struct tagSURFACE *surface);
	//@}

} SURFACE;

int		GetBitdepth(int rgbFormat);
const char *GetRGBFormatName(int rgbformat);



class CEngine
{
	friend class CGrafix;
	private:
		CGrafix *grafix;
		bool IsInit;
	public:
		CEngine();
		virtual ~CEngine();
		int LockWidgets();
		int UnlockWidgets();
		int Init();
		virtual const char *EngineName();
		virtual int GetPrivateData(CAssocArray *ret);
		virtual int CreateWindow(CSurface *surface, ppl6::tk::CWindow *window=NULL);
		virtual int CreateFullscreen(CSurface *surface, ppl6::tk::CWindow *window=NULL);
		virtual int CreateSurface(CSurface *surface);
		virtual int StartMessageLoop();
		virtual int GetMousePosition(CSurface *surface, POINT *p);
		virtual int SendEvent(CSurface *surface, void *object, ppl6::CAssocArray *msg);

};

class CEngineSDL : public CEngine
{
	private:
		void *priv;

	public:
		CEngineSDL();
		virtual ~CEngineSDL();
		int Init();
		virtual const char *EngineName();
		virtual int GetPrivateData(CAssocArray *ret);
		virtual int CreateWindow(CSurface *surface, ppl6::tk::CWindow *window=NULL);
		virtual int CreateFullscreen(CSurface *surface, ppl6::tk::CWindow *window=NULL);
		virtual int CreateSurface(CSurface *surface);
		virtual int StartMessageLoop();
		virtual int GetMousePosition(CSurface *surface, POINT *p);
		virtual int SendEvent(CSurface *surface, void *object, ppl6::CAssocArray *msg);
};

class CEngineDX9 : public CEngine
{
	private:
		void *priv;

		int EnumAdapters(CAssocArray *result);
		int EnumAdapterModes(CAssocArray *result, int rgbformat, int adapter);

	public:
		CEngineDX9();
		virtual ~CEngineDX9();
		int Init();
		virtual const char *EngineName();
		virtual int CreateFullscreen(CSurface *surface, ppl6::tk::CWindow *window=NULL);
		virtual int CreateWindow(CSurface *surface, ppl6::tk::CWindow *window=NULL);
		virtual int GetPrivateData(CAssocArray *ret);
		virtual int StartMessageLoop();
		int	DispatchDX9Message(void *msg);

		virtual int CreateSurface(CSurface *surface);
		virtual int SendEvent(CSurface *surface, void *object, ppl6::CAssocArray *msg);

		/*
		virtual void GetPrivateData(CAssocArray *ret);



		virtual int GetMousePosition(CSurface *surface, POINT *p);
		virtual int SendEvent(CSurface *surface, void *object, ppl6::CAssocArray *msg);
		*/
};


typedef struct tagDimension {
	int width;
	int height;
} DIMENSION;

namespace ORIENTATION {
enum ORIENTATION {
	LEFT =1,
	MIDDLE,
	RIGHT,
	TOP,
	BOTTOM,
	BASE
};
}

class CFont : public CCallback, public CVar
{
	friend class CSurface;
	private:
		CGrafix *gfx;
		CString	OldName;

	public:
		CFontFile *file;
		CFontEngine *engine;
		void *priv;
		CFont();
		virtual void Callback(void *data);

		CString	Name;
		COLOR	color;
		COLOR	shadow_color;
		COLOR	border_color;
		CInt	Orientation;
		CInt	Size;
		CBool	Bold;
		CBool	Italic;
		CBool	Antialias;
		CBool	Underline;
		CBool	Shadow;
		CBool	Border;

		int		LoadFont(const char *name);
		int		Reset();


};

class CGrafix
{
	private:
		CMutex	Mutex;
		CMutex	Widgets;
		CEngine *engine;
		CList	Filter;
		CList	Surfaces;
		CList	FontEngines;
		CList	Fonts;
		int		PrimaryRGBFormat;
		CSurface	*PrimarySurface;
		CFilter		*filter_png;
		CFilter		*filter_jpeg;
		CFilter		*filter_bmp;
		CFilter		*filter_gif;
		CFilter		*filter_ppm;
		CFilter		*filter_tga;

		char		*alphatab;


		void InitErrors();
		int InitSurface(SURFACE *s);
		int InitPixel(SURFACE *s);
		int InitColors(SURFACE *s);
		int InitLines(SURFACE *s);
		int InitShapes(SURFACE *s);
		int InitBlits(SURFACE *s);
		void InitAlphatab();

	public:
		CGrafix();
		~CGrafix();
		CImageList	*Toolbar;
		CImageList	*ButtonSymbolsSmall;
		CImageList	*Icons32;
		ppl6::CCrontab	Crontab;

		int LockWidgets();
		int UnlockWidgets();
		int Init(CEngine *engine=NULL);
		int AddFilter(CFilter *filter);
		int DeleteFilter(CFilter *filter);
		CFilter *FindFilter(const char *name);

		CSurface *CreateWindow(const char *title, int width, int height, int rgbformat=0, int style=0, int flags=0, ppl6::tk::CWindow *window=NULL);
		CSurface *CreateFullscreen(const char *name, int width, int height, int rgbformat=0, int style=0, int flags=0, int frequency=0, ppl6::tk::CWindow *window=NULL);
		CSurface *CreateSurface(const char *name, int width, int height, int rgbformat=0, int flags=0);
		CSurface *Load(const char *filename, int flags=0, int rgbformat=0, CFilter *filter=NULL);
		CSurface *Load(CFileObject *ff, int flags=0, int rgbformat=0, CFilter *filter=NULL);
		CSurface *Load(CResource *res, int id, int flags=0, int rgbformat=0, CFilter *filter=NULL);
		CSurface *Load(CResource *res, const char *name, int flags=0, int rgbformat=0, CFilter *filter=NULL);

		int AddFontEngine(CFontEngine *engine);
		int LoadFont(const char *filename, const char *fontname=NULL);
		int LoadFont(CFileObject *ff, const char *fontname=NULL);
		int LoadFont(CResource *res, int i, const char *fontname=NULL);
		int LoadFont(CResource *res, const char *name, const char *fontname=NULL);
		int DeleteFont(const char *fontname);
		CFontFile *FindFont(const char *fontname);

		int StartMessageLoop();


};

CGrafix *GetGrafix();


class CSurface : public ppl6::CListItem
{
	friend class CGrafix;
	private:
		CString Name;
		CString Caption;
		SURFACE s;
		CEngine *engine;
		CGrafix *gfx;

	public:
		CFont	Font;

		CSurface();
		virtual ~CSurface();
		SURFACE *GetSurfaceParams();
		const char *GetName();
		const char *GetCaption();
		int SetCaption(const char *title);
		CGrafix *GetGrafix();

		int Flip();
		unsigned char* Lock();
		int Unlock();
		int IsLocked();

		int		GetWidth();
		int		GetHeight();
		int		GetBitdepth();
		int		GetRGBFormat();

		void	SetOrigin(int x, int y);
		void	SetClipper(int x1, int y1, int x2, int y2);
		void	SetClipper(RECT *r);
		void	SetClipper(POINT *p1, POINT *p2);
		void	GetClipper(RECT *r);
		void	UnsetClipper();
		int		CheckClipping(int x, int y);
		int		CheckClipping(POINT *p);
		int		CheckClipping (int x, int y, RECT * r);
		int		CheckClipping (POINT *p, RECT * r);
		int		CheckBltClipping (int *x, int *y, RECT * r);

		int		MoveCursor(int x, int y);

		int CLS(COLOR c=0);

		int FitRect(RECT *r);
		COLOR RGB(int r, int g, int b, int a=255);
		COLOR RGB(RGBA *c);
		COLOR RGB(RGBA c);
		COLOR RGBBlend(COLOR background, COLOR foreground, int alpha);
		COLOR	Brightness(COLOR color, float factor=1.0);
		COLOR	Surface2RGB (COLOR color);
		COLOR	Surface2RGB (COLOR color, RGBA *c);
		void	SetColorKey(COLOR c);
		void	SetColor(COLOR c);
		COLOR	GetColorKey();
		void	DeleteColorKey();
		int		HasColorKey(COLOR *c=NULL);
		int		FastPixel(int flag);
		void	PutPixel(int x, int y, COLOR c);
		int AlphaPixel(int x, int y, COLOR c);
		COLOR BlendPixel(int x, int y, COLOR c, float brightness);
		COLOR BlendPixel(int x, int y, COLOR c, int brightness);
		COLOR GetPixel(int x, int y);
		int FillRect(int x1, int y1, int x2, int y2, COLOR c);
		int Rect(int x1, int y1, int x2, int y2, COLOR c);
		int FillRect(int x1, int y1, int x2, int y2);
		int Rect(int x1, int y1, int x2, int y2);
		int FillRect(RECT *r);
		int Rect(RECT *r);
		int FillRect(RECT *r, COLOR c);
		int Rect(RECT *r, COLOR c);

		int Xchange (int x1, int y1, int x2, int y2, COLOR color, COLOR exchange);
		int Invert(int x1, int y1, int x2, int y2, COLOR color1, COLOR color2);
		int Negativ(int x1, int y1, int x2, int y2);
		void ColorGradient(int x1, int y1, int x2, int y2, COLOR color1, COLOR color2, int direction);
		void ColorGradient(int x1, int y1, int x2, int y2, RGBA *color1, RGBA *color2, int direction);



		int Line(int x1, int y1, int x2, int y2, COLOR c);
		int Line(int x2, int y2, COLOR c);

		int LineAA(int x1, int y1, int x2, int y2, COLOR c, int strength=1);
		int LineAA(int x2, int y2, COLOR c, int strength=1);

		void FloodFill (int x, int y, COLOR color, COLOR border);
		void Elipse (int x, int y, int radiusx, int radiusy, COLOR Farbe, int Flags=0);
		void Elipse (int x, int y, int radiusx, int radiusy, COLOR Farbe, int Flags, COLOR Fuellfarbe, int startwinkel, int endwinkel);
		void Circle (int x, int y, int radius, COLOR Farbe, int Flags=0);
		void Polygon (int count, POINT *points, COLOR color, int Flags=0);

		int BeginScene();
		int EndScene();

		int Blt(CSurface *source, RECT *srect=NULL, int x=0, int y=0);
		int BltDiffuse(CSurface *source, COLOR c=0, RECT *srect=NULL, int x=0, int y=0);
		int BltColorKey(CSurface *source, COLOR c=0, RECT *srect=NULL, int x=0, int y=0);
		int AlphaBlt(CSurface *source, RECT *srect=NULL, int x=0, int y=0);
		int Draw(CImageList *iml, int nr, int x, int y);
		int Draw(CImageList *iml, int nr, int x, int y, COLOR diffuse);
		int Draw(CImage *img, int x, int y);

		int Print(CFont *font, int x, int y, const char *text);
		int Print(CFont *font, int x, int y, CWString *text);
		int Printf(CFont *font, int x, int y, const char *text, ...);
		int Print(int x, int y, const char *text);
		int Print(int x, int y, CWString *text);
		int Printf(int x, int y, const char *text, ...);
		int Print(const char *text);
		int Print(CWString *text);
		int Printf(const char *text, ...);

		int TextSize(CFont *font, DIMENSION *d, const char *text);
		int TextSize(CFont *font, DIMENSION *d, CWString *text);
		int TextSize(DIMENSION *d, const char *text);
		int TextSizef(CFont *font, DIMENSION *d, const char *text, ...);
		int TextSizef(DIMENSION *d, const char *text, ...);
		int CharWidth(CFont *font, wchar_t c);
		int CharHeight(CFont *font, wchar_t c);

		int GetMousePosition(POINT *p);
		int SendMessage(void *object=NULL, CAssocArray *msg=NULL);
		int SendMessage(unsigned int msg, void *param1, void *param2);


		int BeginSprites();
		int DrawSprite(CSurface *source, RECT *srect=NULL, int x=0, int y=0, COLOR c=0xffffffff);
		int EndSprites();
};

class CImageList
{
	private:
		CSurface *surface;
		CGrafix *gfx;
		bool ownsurface;
		int width, height, num;
		int iconsX, iconsY;
		int drawmethod;
		COLOR colorkey;
		COLOR diffuse;

	public:
		CImageList();
		~CImageList();
		void Clear();
		int SetDrawMethod(int m);
		int SetColorKey(COLOR c);
		int SetColorDiffuse(COLOR c);
		int Load(const char *file, int icon_width, int icon_height, int flags=0, int drawmethod=DRAWMETHOD::BLT);
		int Load(CFileObject *file, int icon_width, int icon_height, int flags=0, int drawmethod=DRAWMETHOD::BLT);
		int Load(CSurface *surface, int icon_width, int icon_height, int drawmethod=DRAWMETHOD::BLT);
		int Load(CResource *res, int id, int icon_width, int icon_height, int flags=0, int drawmethod=DRAWMETHOD::BLT);
		int Load(CResource *res, const char *name, int icon_width, int icon_height, int flags=0, int drawmethod=DRAWMETHOD::BLT);

		int GetRect(RECT *r, int nr);
		CSurface *GetSurface();
		int Num();
		int GetDimension(DIMENSION *d);
		int Draw(int nr, CSurface *target, int x, int y);
		int Draw(int nr, CSurface *target, int x, int y, COLOR diffuse);
		int ManageSurface();

};

class CImage : public CVar
{
	private:
		CSurface *surface;
		CImageList *iml;
		CGrafix *gfx;
		RECT r;
		bool ownsurface;
		int drawmethod;
		COLOR colorkey;
		COLOR diffuse;
		int nr;

	public:
		CImage();
		~CImage();
		void Clear();
		int SetDrawMethod(int m);
		int SetColorKey(COLOR c);
		int SetColorDiffuse(COLOR c);
		int Load(const char *file, int flags=0, int drawmethod=DRAWMETHOD::BLT);
		int Load(CFileObject *file, int flags=0, int drawmethod=DRAWMETHOD::BLT);
		int Load(CResource *res, int id, int flags=0, int drawmethod=DRAWMETHOD::BLT);
		int Load(CResource *res, const char *name, int flags=0, int drawmethod=DRAWMETHOD::BLT);
		int Set(CSurface *surface, RECT *r=NULL, int drawmethod=DRAWMETHOD::BLT);
		int Set(CImageList *iml, int nr);
		int GetRect(RECT *r);
		CSurface *GetSurface();
		int GetDimension(DIMENSION *d);
		int GetWidth();
		int GetHeight();
		int Draw(CSurface *target, int x, int y);
		int ManageSurface();
		int IsLoaded();
};


class CFilter : public ppl6::CListItem
{
	private:

	public:
		CFilter();
		virtual ~CFilter();
		virtual int Ident(CFileObject *file, IMAGE *img);
		virtual int Load(CFileObject *file, CSurface *surface, IMAGE *img);
		virtual int Save (CSurface *surface, CFileObject * file, RECT *area=NULL, CAssocArray *param=NULL);
		int SaveFile (CSurface *surface, const char * filename, RECT *area=NULL, CAssocArray *param=NULL);
		virtual const char *GetName();
		virtual const char *GetDescription();
};


class CFilter_PNG : public CFilter
{
	public:
		CFilter_PNG();
		virtual ~CFilter_PNG();
		virtual int Ident(CFileObject *file, IMAGE *img);
		virtual int Load(CFileObject *file, CSurface *surface, IMAGE *img);
		virtual int Save (CSurface *surface, CFileObject * file, RECT *area=NULL, CAssocArray *param=NULL);
		virtual const char *GetName();
		virtual const char *GetDescription();
};

class CFilter_JPEG : public CFilter
{
	public:
		CFilter_JPEG();
		virtual ~CFilter_JPEG();
		virtual int Ident(CFileObject *file, IMAGE *img);
		virtual int Load(CFileObject *file, CSurface *surface, IMAGE *img);
		virtual int Save (CSurface *surface, CFileObject * file, RECT *area=NULL, CAssocArray *param=NULL);
		virtual const char *GetName();
		virtual const char *GetDescription();
};

class CFilter_GIF : public CFilter
{
	public:
		CFilter_GIF();
		virtual ~CFilter_GIF();
		virtual int Ident(CFileObject *file, IMAGE *img);
		virtual int Load(CFileObject *file, CSurface *surface, IMAGE *img);
		virtual int Save (CSurface *surface, CFileObject * file, RECT *area=NULL, CAssocArray *param=NULL);
		virtual const char *GetName();
		virtual const char *GetDescription();
};

class CFilter_BMP : public CFilter
{
	public:
		CFilter_BMP();
		virtual ~CFilter_BMP();
		virtual int Ident(CFileObject *file, IMAGE *img);
		virtual int Load(CFileObject *file, CSurface *surface, IMAGE *img);
		virtual int Save (CSurface *surface, CFileObject * file, RECT *area=NULL, CAssocArray *param=NULL);
		virtual const char *GetName();
		virtual const char *GetDescription();
};

class CFilter_PPM : public CFilter
{
	public:
		CFilter_PPM();
		virtual ~CFilter_PPM();
		virtual int Ident(CFileObject *file, IMAGE *img);
		virtual int Load(CFileObject *file, CSurface *surface, IMAGE *img);
		virtual int Save (CSurface *surface, CFileObject * file, RECT *area=NULL, CAssocArray *param=NULL);
		virtual const char *GetName();
		virtual const char *GetDescription();
};

class CFilter_TGA : public CFilter
{
	public:
		CFilter_TGA();
		virtual ~CFilter_TGA();
		virtual int Ident(CFileObject *file, IMAGE *img);
		virtual int Load(CFileObject *file, CSurface *surface, IMAGE *img);
		virtual int Save (CSurface *surface, CFileObject * file, RECT *area=NULL, CAssocArray *param=NULL);
		virtual const char *GetName();
		virtual const char *GetDescription();
};

class CFontFile : public CListItem
{
	friend class CGrafix;
	public:
		CFontFile();
		virtual ~CFontFile();
		CString Name;
		void *ptr;
		int	size;
		CFontEngine *engine;
		void *priv;
};

class CFontEngine : public CListItem
{
	friend class CGrafix;
	private:
		CGrafix *gfx;

	public:
		CFontEngine();
		virtual ~CFontEngine();

		ppluint16 *ToUTF16(char *text);
		CGrafix *GetGrafix();
		virtual int Init();
		virtual int Ident(CFileObject *file);
		virtual CFontFile *LoadFont(CFileObject *file, const char *fontname);
		virtual int DeleteFont(CFontFile *file);
		virtual int Render(CFont *font, CSurface *surface, int x, int y, const char *text, COLOR color);
		virtual int Measure(CFont *font, const char *text, DIMENSION *measure);
		virtual int Render(CFont *font, CSurface *surface, int x, int y, CWString *text, COLOR color);
		virtual int Measure(CFont *font, CWString *text, DIMENSION *measure);

		virtual int Unselect(CFont *font);

};

class CFontEngineFont5 : public CFontEngine
{
	private:
		PFPChunk *SelectFont(CFont *font);

	public:
		CFontEngineFont5();
		virtual ~CFontEngineFont5();
		virtual int Init();
		virtual int Ident(CFileObject *file);
		virtual CFontFile *LoadFont(CFileObject *file, const char *fontname);
		virtual int DeleteFont(CFontFile *file);
		virtual int Render(CFont *font, CSurface *surface, int x, int y, const char *text, COLOR color);
		virtual int Render(CFont *font, CSurface *surface, int x, int y, CWString *text, COLOR color);
		virtual int Measure(CFont *font, const char *text, DIMENSION *measure);
		virtual int Measure(CFont *font, CWString *text, DIMENSION *measure);
		virtual int Unselect(CFont *font);
};

class CFontEngineFreeType : public CFontEngine
{
	private:
		void *ft;
	public:
		CFontEngineFreeType();
		virtual ~CFontEngineFreeType();
		virtual int Init();
		virtual int Ident(CFileObject *file);
		virtual CFontFile *LoadFont(CFileObject *file, const char *fontname);
		virtual int DeleteFont(CFontFile *file);
		virtual int Render(CFont *font, CSurface *surface, int x, int y, const char *text, COLOR color);
		virtual int Render(CFont *font, CSurface *surface, int x, int y, CWString *text, COLOR color);
		virtual int Measure(CFont *font, const char *text, DIMENSION *measure);
		virtual int Measure(CFont *font, CWString *text, DIMENSION *measure);
		virtual int Unselect(CFont *font);
};

class CSprite
{
	private:
		CList	TextureList;
		CTree	SpriteList;
		int LoadTexture(PFPChunk *chunk, int flags);
		int LoadIndex(PFPChunk *chunk);
		CSurface *FindTexture(int id);

	public:
		CSprite();
		~CSprite();
		int Load(const char *filename, int flags);
		int Load(CFileObject *ff, int flags);
		void Clear();
		int Draw(CSurface *target, int x, int y, int id);
		int NumTextures();
		int NumSprites();
};




} // EOF namespace grafix
} // EOF namespace ppl6

#endif /*GRAFIX_H_*/
