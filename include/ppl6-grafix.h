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
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
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
    #else
        #include <ppl6.h>
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
	class Surface;
	class Window;
	class Widget;
};

namespace grafix {

// Font6 Exceptions
PPLPARAMETERISEDEXCEPTION(InvalidFontFormatException);
PPLPARAMETERISEDEXCEPTION(InvalidFontFaceException);
PPLPARAMETERISEDEXCEPTION(UnknownFontFaceException);
PPLNORMALEXCEPTION(InvalidFontException);


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

class Point
{
	// Freunde der Klasse
	friend bool operator== (const Point &p1, const Point &p2);
	friend bool operator!= (const Point &p1, const Point &p2);
	friend const Point operator* (const Point &point, double factor);
	friend const Point operator* (double factor, const Point &point);
	friend const Point operator+ (const Point &p1, const Point &p2);
	friend const Point operator- (const Point &p1, const Point &p2);
	friend const Point operator- (const Point &point);
	friend const Point operator/ (const Point &point, double divisor);
	friend double Distance(const Point &p1, const Point &p2);

	private:
		int px,py;
	public:
		Point();
		Point(int x, int y);
		bool isNull() const;
		double vectorLength() const;
		int manhattanLength() const;
		int &rx();
		int &ry();
		void setX(int x);
		void setY(int y);
		void setPoint(int x, int y);
		int x() const;
		int y() const;
		Point &operator*= (double factor);
		Point &operator+= (const Point &point);
		Point &operator-= (const Point &point);
		Point &operator/= (double divisor);
};

bool operator!= (const Point &p1, const Point &p2);
bool operator== (const Point &p1, const Point &p2);
const Point operator* (const Point &point, double factor);
const Point operator* (double factor, const Point &point);
const Point operator+ (const Point &p1, const Point &p2);
const Point operator- (const Point &p1, const Point &p2);
const Point operator- (const Point &point);
const Point operator/ (const Point &point, double divisor);
double Distance(const Point &p1, const Point &p2);

class Point3D
{
	// Freunde der Klasse
	friend bool operator== (const Point3D &p1, const Point3D &p2);
	friend bool operator!= (const Point3D &p1, const Point3D &p2);
	friend const Point3D operator* (const Point3D &point, double factor);
	friend const Point3D operator* (double factor, const Point3D &point);
	friend const Point3D operator+ (const Point3D &p1, const Point3D &p2);
	friend const Point3D operator- (const Point3D &p1, const Point3D &p2);
	friend const Point3D operator- (const Point3D &point);
	friend const Point3D operator/ (const Point3D &point, double divisor);

	private:
		int px,py,pz;
	public:
		Point3D();
		Point3D(int x, int y, int z);
		bool isNull() const;
		int &rx();
		int &ry();
		int &rz();
		void setX(int x);
		void setY(int y);
		void setZ(int z);
		void setPoint(int x, int y, int z);
		int x() const;
		int y() const;
		int z() const;
		Point3D &operator*= (double factor);
		Point3D &operator+= (const Point3D &point);
		Point3D &operator-= (const Point3D &point);
		Point3D &operator/= (double divisor);
};

bool operator!= (const Point3D &p1, const Point3D &p2);
bool operator== (const Point3D &p1, const Point3D &p2);
const Point3D operator* (const Point3D &point, double factor);
const Point3D operator* (double factor, const Point3D &point);
const Point3D operator+ (const Point3D &p1, const Point3D &p2);
const Point3D operator- (const Point3D &p1, const Point3D &p2);
const Point3D operator- (const Point3D &point);
const Point3D operator/ (const Point3D &point, double divisor);


class Size
{
	friend bool operator!= (const Size &s1, const Size &s2);
	friend const Size operator* (const Size &size, double factor);
	friend const Size operator* (double factor, const Size &size);
	friend const Size operator+ (const Size &s1, const Size &s2);
	friend const Size operator- (const Size &s1, const Size &s2);
	friend const Size operator- (const Size &size);
	friend const Size operator/ (const Size &size, double divisor);
	friend bool operator== (const Size & s1, const Size &s2);

	private:
		int w,h;
	public:
		Size();
		Size(int width, int height);
		int height() const;
		int width() const;
		bool isNull() const;
		bool isEmpty() const;
		bool isValid() const;
		int &rheight();
		int &rwidth();
		void setHeight(int height);
		void setWidth(int width);
		void setSize(int width, int height);
		void setSize(const Size &s);
		Size &operator*= (double factor);
		Size &operator+= (const Size &size);
		Size &operator-= (const Size &size);
		Size &operator/= (double divisor);
};

bool operator!= (const Size &s1, const Size &s2);
bool operator== (const Size & s1, const Size &s2);
const Size operator* (const Size &size, double factor);
const Size operator* (double factor, const Size &size);
const Size operator+ (const Size &s1, const Size &s2);
const Size operator- (const Size &s1, const Size &s2);
const Size operator- (const Size &size);
const Size operator/ (const Size &size, double divisor);



class Rect
{
	friend bool operator!= (const Rect &r1, const Rect &r2);
	friend bool operator== (const Rect &r1, const Rect &r2);

	private:
		int rx,ry,rw,rh;
	public:
		Rect();
		Rect(const Point &topLeft, const Point &bottomRight);
		Rect(int x, int y, int width, int height);
		Rect(const RECT &r);

		bool isNull() const;
		int left() const;
		int right() const;
		int top() const;
		int bottom() const;
		int width() const;
		int height() const;
		int x() const;
		int y() const;
		int x1() const;
		int y1() const;
		int x2() const;
		int y2() const;
		Size size() const;

		Point topLeft() const;
		Point topRight() const;
		Point bottomLeft() const;
		Point bottomRight() const;

		Rect normalized () const;
		Rect intersected(const Rect &other);
		bool intersects(const Rect &other);

		void setTopLeft(const Point &topLeft);
		void setBottomRight(const Point &bottomRight);
		void setRect(const RECT &r);
		void setRect(int x, int y, int width, int height);
		void setCoords(int x1, int y1, int x2, int y2);
		void setLeft(int left);
		void setRight(int right);
		void setTop(int top);
		void setBottom(int bottom);
		void setX(int x);
		void setY(int y);
		void setSize(const Size &size);
		void setWidth(int width);
		void setHeight(int height);


};

bool operator!= (const Rect &r1, const Rect &r2);
bool operator== (const Rect &r1, const Rect &r2);

class RGBFormat
{
private:
	int f;
public:
	enum Identifier {
		unknown=0,
		Palette,
		R5G6B5,
		B5G6R5,
		X1R5G5B5,
		X1B5G5R5,
		X4R4G4B4,
		A1R5G5B5,
		A1B5G5R5,
		A4R4G4B4,
		R8G8B8,
		B8G8R8,
		A8R8G8B8,
		A8B8G8R8,
		R3G3B2,
		A8,
		A8R3G3B2,
		X8R8G8B8,
		X8B8G8R8,
		GREY8,
		GREYALPHA32,
		X16R16G16B16,
		A16R16G16B16,

		MaxIdentifiers
	};

	RGBFormat();
	RGBFormat(Identifier id);
	RGBFormat(const CString &Identifier);

	void setFormat(Identifier id);
	int setFormat(const CString &Identifier);

	int format() const;
	CString name() const;
	int bitdepth() const;
	int bytesPerPixel() const;
	int bitsPerPixel() const;

	RGBFormat &operator=(Identifier id);
	operator int() const;

	friend bool operator!= (const RGBFormat &r1, const RGBFormat &r2);
	friend bool operator== (const RGBFormat &r1, const RGBFormat &r2);
	friend bool operator== (const RGBFormat &r1, RGBFormat::Identifier r2);
	friend bool operator!= (const RGBFormat &r1, RGBFormat::Identifier r2);
	friend bool operator== (RGBFormat::Identifier r1, const RGBFormat &r2);
	friend bool operator!= (RGBFormat::Identifier r1, const RGBFormat &r2);

};

bool operator!= (const RGBFormat &r1, const RGBFormat &r2);
bool operator== (const RGBFormat &r1, const RGBFormat &r2);
bool operator== (const RGBFormat &r1, RGBFormat::Identifier r2);
bool operator!= (const RGBFormat &r1, RGBFormat::Identifier r2);
bool operator== (RGBFormat::Identifier r1, const RGBFormat &r2);
bool operator!= (RGBFormat::Identifier r1, const RGBFormat &r2);



class Color
{
	friend bool operator!= (const Color &c1, const Color &c2);
	friend bool operator== (const Color &c1, const Color &c2);
	friend const Color operator* (const Color &size, float factor);
	friend const Color operator* (float factor, const Color &size);

	private:
		int Clamp(int value);
#ifdef __LITTLE_ENDIAN__
union  {
	struct { ppluint8 r,g,b,a; };
	ppluint32 c;
};
#else
union  {
	struct { ppluint8 a,r,g,b; };
	ppluint32 c;
};
#endif

	public:
		Color();
		Color(int red, int green, int blue, int alpha=255);
		Color(ppluint32 rgba);
		Color(ppluint32 rgb, int alpha);

		ppluint32 color() const;
		ppluint32 rgb() const;
		ppluint32 rgba() const;
		int red() const;
		int green() const;
		int blue() const;
		int alpha() const;
		int brightness() const;
		Color grey() const;
		bool match(const Color &other, int tolerance=0) const;

		void setRed(int red);
		void setGreen(int green);
		void setBlue(int blue);
		void setAlpha(int alpha);
		void setColor(int red, int green, int blue, int alpha=255);
		void setColor(ppluint32 rgba);
		void setColor(ppluint32 rgb, int alpha);

		Color &blend(const Color &background, const Color &foreground, int intensity);
		Color &blendf(const Color &background, const Color &foreground, float intensity);

		Color &operator*= (float factor);
		operator ppluint32() const;
		operator pplint32() const;
};

bool operator!= (const Color &c1, const Color &c2);
bool operator== (const Color &c1, const Color &c2);
const Color operator* (const Color &color, float factor);
const Color operator* (float factor, const Color &color);

class AdapterCaps
{
public:
	AdapterCaps();
	int MaxActiveLights;
	int MaxTextureWidth;
	int MaxTextureHeight;
	int MaxTextureAspectRatio;
	int PixelShaderVersion;
	int VertexShaderVersion;
	bool hwSurface;
	bool hwBlit;
	bool hwColorKeyBlit;
	bool hwAlphaBlit;
	bool hwFill;


};


class AdapterIdentifier
{
public:
	AdapterIdentifier();
	void	print() const;
	int		adapterId;
	CString DriverName;
	CString Description;
	ppluint64	version;
	int			major_version;
	int			minor_version;
	int			sub_version;
	int			build;
	int			currentWidth;
	int			currentHeight;
	int			currentRefreshRate;
	RGBFormat	currentRGBFormat;
	AdapterCaps	Caps;
};

class AdapterMode
{
public:
	AdapterMode();
	int width;
	int height;
	int refreshRate;
	RGBFormat format;
};



typedef struct {
	int			width;
	int			height;
	int			pitch;
	int			bitdepth;
	int			colors;
	RGBFormat	format;
} IMAGE;

class CFontEngine;
class CFontFile;
class CFilter;
class CImage;
class CImageList;
class CDrawable;
class CSurface;
class CSprite;



class GFXEngine
{
private:
	int adapterNo;

public:
	GFXEngine();
	virtual ~GFXEngine();

	virtual bool isInit() const;
	virtual CString name() const;
	int adapter() const;
	virtual int getAdapterCount();
	virtual int getAdapterInfo(int adapter, AdapterIdentifier &info);
	virtual int getAdapterModeCount(int adapter, const RGBFormat &format);
	virtual int getAdapterModeInfo(int adapter, const RGBFormat &format, int mode, AdapterMode &info);
	virtual int useAdapter(int adapter);

	virtual bool createWindow(tk::Widget &widget);
	virtual bool destroyWindow(tk::Widget &widget);
	virtual bool setWindowTitle(tk::Widget &widget);
	virtual bool setWindowIcon(tk::Widget &widget);

	virtual bool createSurface(CSurface &surface);
	virtual bool destroySurface(CSurface &surface);
	virtual bool lockSurface(CSurface &surface, CDrawable &draw);
	virtual bool unlockSurface(CSurface &surface);

	virtual bool lockDevice(tk::Widget &widget, CDrawable &draw);
	virtual bool unlockDevice(tk::Widget &widget);
	virtual bool updateDevice(tk::Widget &widget);

	virtual bool beginScene();
	virtual bool endScene();
	virtual bool blt(tk::Widget &widget, const CSurface &source, const Rect &srect=Rect(), int x=0, int y=0);
	virtual bool bltAlpha(tk::Widget &widget, const CSurface &source, const Rect &srect=Rect(), int x=0, int y=0);

	virtual bool startMessageLoop();
};


class GFXEngineSoftware : public GFXEngine
{
	private:
	public:
		GFXEngineSoftware();
		virtual ~GFXEngineSoftware();
		virtual bool isInit() const;
		virtual CString name() const;
		virtual bool createSurface(CSurface &surface);
		virtual bool destroySurface(CSurface &surface);
		virtual bool lockSurface(CSurface &surface, CDrawable &draw);
		virtual bool unlockSurface(CSurface &surface);

};

class GFXEngineDX9 : public GFXEngine
{
private:
	void *priv;

public:
	GFXEngineDX9();
	virtual ~GFXEngineDX9();

	int Init();

#ifdef _D3D9_H_
	IDirect3D9 *device();
#else
	void *device();
#endif

	virtual bool isInit() const;
	virtual CString name() const;
	virtual int getAdapterCount();
	virtual int getAdapterInfo(int adapter, AdapterIdentifier &info);
	virtual int getAdapterModeCount(int adapter, const RGBFormat &format);
	virtual int getAdapterModeInfo(int adapter, const RGBFormat &format, int mode, AdapterMode &info);

	virtual bool createWindow(tk::Widget &widget);
	virtual bool destroyWindow(tk::Widget &widget);
	virtual bool setWindowTitle(tk::Widget &widget);
	virtual bool setWindowIcon(tk::Widget &widget);

	virtual bool createSurface(CSurface &surface);
	virtual bool destroySurface(CSurface &surface);
	virtual bool lockSurface(CSurface &surface, CDrawable &draw);
	virtual bool unlockSurface(CSurface &surface);


	virtual bool lockDevice(tk::Widget &widget, CDrawable &draw);
	virtual bool unlockDevice(tk::Widget &widget);
	virtual bool updateDevice(tk::Widget &widget);


	virtual bool startMessageLoop();
	int	dispatchDX9Message(void *msg);
};


class GFXEngineSDL : public GFXEngine
{
private:
	void *priv;

public:
	GFXEngineSDL();
	virtual ~GFXEngineSDL();

	int Init();

	virtual bool isInit() const;
	virtual CString name() const;

	virtual int getAdapterCount();
	virtual int getAdapterInfo(int adapter, AdapterIdentifier &info);
	//virtual int getAdapterModeCount(int adapter, const RGBFormat &format);
	//virtual int getAdapterModeInfo(int adapter, const RGBFormat &format, int mode, AdapterMode &info);

	virtual bool createWindow(tk::Widget &widget);
	virtual bool setWindowTitle(tk::Widget &widget);
	virtual bool setWindowIcon(tk::Widget &widget);

	virtual bool lockDevice(tk::Widget &widget, CDrawable &draw);
	virtual bool unlockDevice(tk::Widget &widget);
	virtual bool updateDevice(tk::Widget &widget);

};


typedef ppluint32 SurfaceColor;



class CFont
{
	friend bool operator!= (const CFont &f1, const CFont &f2);
	friend bool operator== (const CFont &f1, const CFont &f2);

	private:
		enum FontFlags {
			fBold=1,
			fItalic=2,
			fAntialias=4,
			fDrawBorder=8,
			fDrawShadow=16,
			fUnderline=32,
			fMonospace=64
		};

		CString Name;
		Color	cForeground;
		Color	cBorder;
		Color	cShadow;
		ppluint16	fontSize;
		ppluint8	flags;
		ppluint8	ori;
		double		rotationDegrees;
	public:
		enum Orientation {
			LEFT =1,
			MIDDLE,
			RIGHT,
			TOP,
			BOTTOM,
			BASE
		};


		CFont();
		CFont(const CFont &other);

		CString name() const;
		Color color() const;
		Color borderColor() const;
		Color shadowColor() const;
		bool bold() const;
		bool italic() const;
		bool antialias() const;
		bool drawBorder() const;
		bool drawShadow() const;
		bool drawUnderline() const;
		bool monospace() const;
		int size() const;
		Orientation orientation() const;
		double rotation() const;
		Size measure(const CWString &text) const;
		Size measure(const char *fmt, ...) const;

		int setName(const CString &name);
		void setColor(const Color &c);
		void setBorderColor(const Color &c);
		void setShadowColor(const Color &c);
		void setColors(const Color &Foreground, const Color &Border, const Color Shadow);
		void setBold(bool enable);
		void setItalic(bool enable);
		void setAntialias(bool enable);
		void setDrawBorder(bool enable);
		void setDrawShadow(bool enable);
		void setDrawUnderline(bool enable);
		void setMonospace(bool enable);
		void setSize(int size);
		void setOrientation(Orientation o);
		void setRotation(double degrees);

		CFont &operator=(const CFont &other);
};
bool operator!= (const CFont &f1, const CFont &f2);
bool operator== (const CFont &f1, const CFont &f2);


// ACHTUNG: Bei Änderungen in der Reihenfolge muss auch die Assembler-Struktur
// in src/asm/common.asminc angepasst werden!
struct GRAFIX_FUNCTIONS;

typedef struct {
	GRAFIX_FUNCTIONS *fn;
	union {
		void		*base;
		ppluint8	*base8;
		ppluint16	*base16;
		ppluint32	*base32;
	};
	ppluint32	pitch;
	int			width;
	int			height;
	RGBFormat	rgbformat;
} DRAWABLE_DATA;

typedef struct GRAFIX_FUNCTIONS {
	void (*CLS) (DRAWABLE_DATA &data, SurfaceColor c);
	void (*PutPixel) (const DRAWABLE_DATA &data, int x, int y, SurfaceColor c);
	void (*BlendPixel) (const DRAWABLE_DATA &data, int x, int y, SurfaceColor c, int brightness);
	void (*AlphaPixel) (const DRAWABLE_DATA &data, int x, int y, SurfaceColor c);
	SurfaceColor (*GetPixel) (const DRAWABLE_DATA &data, int x, int y);

	void (*DrawRect)(DRAWABLE_DATA &data, const Rect &r, SurfaceColor c);
	void (*FillRect)(DRAWABLE_DATA &data, const Rect &r, SurfaceColor c);
	void (*Xchange)(DRAWABLE_DATA &data, const Rect &r, SurfaceColor farbe, SurfaceColor ersatzfarbe);
	void (*Invert)(DRAWABLE_DATA &data, const Rect &r, SurfaceColor farbe1, SurfaceColor farbe2);
	void (*Negativ)(DRAWABLE_DATA &data, const Rect &r);

	SurfaceColor (*RGB) (int r, int g, int b, int a);
	SurfaceColor (*RGBBlend) (SurfaceColor ground, SurfaceColor top, float intensity);
	SurfaceColor (*RGBBlend255) (SurfaceColor ground, SurfaceColor top, int intensity);
	Color (*Surface2RGB) (SurfaceColor color);
	void (*LineAA) (DRAWABLE_DATA &data, int x1, int y1, int x2, int y2,SurfaceColor color, int strength);
	void (*Line) (DRAWABLE_DATA &data, int x1, int y1, int x2, int y2, SurfaceColor color);

	int (*Blt) (DRAWABLE_DATA &target, const DRAWABLE_DATA &source, const Rect &srect, int x, int y);
	int (*BltDiffuse) (DRAWABLE_DATA &target, const DRAWABLE_DATA &source, const Rect &srect, int x, int y, SurfaceColor c);
	int (*BltColorKey) (DRAWABLE_DATA &target, const DRAWABLE_DATA &source, const Rect &srect, int x, int y, SurfaceColor c);
	int (*BltAlpha) (DRAWABLE_DATA &target, const DRAWABLE_DATA &source, const Rect &srect, int x, int y);
	//int supported;

} GRAFIX_FUNCTIONS;

class CDrawable
{
	friend class CImage;
	private:
		GRAFIX_FUNCTIONS	*fn;
		DRAWABLE_DATA		data;
		int initFunctions(const RGBFormat &format);

	public:
		/** @name Konstruktoren
		 */
		//@{
		CDrawable();
		CDrawable(const CDrawable &other);
		CDrawable(const CDrawable &other, const Rect &rect);
		CDrawable(void *base, ppluint32 pitch, int width, int height, const RGBFormat &format);
		~CDrawable();
		//@}

		/** @name Verschiedenes
		 */
		//@{
		GRAFIX_FUNCTIONS *getFunctions();
		DRAWABLE_DATA *getData();

		int copy(const CDrawable &other);
		int copy(const CDrawable &other, const Rect &rect);
		int create(void *base, ppluint32 pitch, int width, int height, const RGBFormat &format);
		CDrawable &operator=(const CDrawable &other);

		Rect rect() const;
		Size size() const;
		int width() const;
		int height() const;
		int pitch() const;
		int bytesPerPixel() const;
		int bitdepth() const;
		RGBFormat rgbformat() const;
		bool isEmpty() const;
		void *adr() const;
		void *adr(int x, int y) const;
		void	cls(const Color &c);
		void	cls();
		CDrawable getDrawable() const;
		CDrawable getDrawable(const Rect &rect) const;
		CDrawable getDrawable(int x1, int y1, int x2, int y2) const;
		//@}

		/** @name Farben
		 */
		//@{
		SurfaceColor rgb(const Color &c) const;
		SurfaceColor rgb(int r, int g, int b, int alpha) const;
		//@}

		/** @name Pixel
		 */
		//@{
		void	putPixel(int x, int y, const Color &c);
		void	putPixel(const Point &p, const Color &c);
		void	alphaPixel(int x, int y, const Color &c);
		void 	alphaPixel(const Point &p, const Color &c);
		Color	blendPixel(int x, int y, const Color &c, float brightness);
		Color	blendPixel(int x, int y, const Color &c, int brightness);
		Color	getPixel(int x, int y) const;
		Color	getPixel(const Point &p) const;
		//@}


		/** @name Geometrische Formen
		 */
		//@{
		void	drawRect(const Rect &rect, const Color &c);
		void	drawRect(int x1, int y1, int x2, int y2, const Color &c);
		void	fillRect(const Rect &rect, const Color &c);
		void	fillRect(int x1, int y1, int x2, int y2, const Color &c);
		void	floodFill(int x, int y, const Color &color, const Color &border);
		void	elipse(int x, int y, int radx, int rady, const Color &c, bool fill=false);
		void	elipse(int x, int y, int radx, int rady, const Color &c, bool fill, const Color &fillcolor, int startangle, int endangle);
		void	circle(int x, int y, int rad, const Color &c, bool fill=false);
		//@}

		/** @name Effekte
		 */
		//@{
		void	xchange(const Rect &rect, const Color &color, const Color &replace);
		void	invert(const Rect &rect, const Color &color1, const Color &color2);
		void	negativ(const Rect &rect);
		void	colorGradient(const Rect &rect, const Color &c1, const Color &c2, int direction);
		void	colorGradient(int x1, int y1, int x2, int y2, const Color &c1, const Color &c2, int direction);
		//@}

		/** @name Linien zeichnen
		 */
		//@{
		void	line(int x1, int y1, int x2, int y2, const Color &c);
		void	line(const Point &start, const Point &end, const Color &c);
		void	lineAA(int x1, int y1, int x2, int y2, const Color &c, int strength=1);
		void	lineAA(const Point &start, const Point &end, const Color &c, int strength=1);
		//@}

		/** @name Textausgabe
		 */
		//@{
		void	print(const CFont &font, int x, int y, const CWString &text);
		void	printf(const CFont &font, int x, int y, const char *fmt, ...);
		//@}

		/** @name Blit-Funktionen
		 * Kopieren von Grafiken mit verschiedenen Methoden
		 */
		//@{
		int fitRect(int &x, int &y, Rect &r);
		int blt(const CDrawable &source, int x=0, int y=0);
		int blt(const CDrawable &source, const Rect &srect, int x=0, int y=0);
		int bltDiffuse(const CDrawable &source, int x=0, int y=0, const Color &c=Color());
		int bltDiffuse(const CDrawable &source, const Rect &srect, int x=0, int y=0, const Color &c=Color());
		int bltColorKey(const CDrawable &source, int x=0, int y=0, const Color &c=Color());
		int bltColorKey(const CDrawable &source, const Rect &srect, int x=0, int y=0, const Color &c=Color());
		int bltAlpha(const CDrawable &source, int x=0, int y=0);
		int bltAlpha(const CDrawable &source, const Rect &srect, int x=0, int y=0);

		int draw(const CImageList &iml, int nr, int x, int y);
		int draw(const CImageList &iml, int nr, int x, int y, const Color &diffuse);
		int draw(const CSprite &sprite, int nr, int x, int y);
		//@}
};

class CSurface
{
	friend class CGrafix;
	public:
		enum SurfaceFlags {
			Backbuffer					=	0x00000001,
			TrippleBuffer				=	0x00000002,
			Hardware					=	0x00000004,
			Lockable					=	0x00000008,
			VideoMemory					=	0x00000010,
			ZBuffer						=	0x00000020,
			StencilBuffer				=	0x00000040,
			SoftwareVertexProcessing	=	0x00000080,
			Multithreaded				=	0x00000100,
			FPUPreserve					=	0x00000200,
			Texture						=	0x00000400,
			DefaultFlags				=	Hardware|Lockable|VideoMemory
		};
	private:
		void *engineprivate;
		GFXEngine *gfxengine;
		Size s;
		CDrawable draw;
		ppluint32 lockcount;
		RGBFormat	format;
		SurfaceFlags	flags;


	public:
		CSurface();
		CSurface(const CSurface &other);
		CSurface(int width, int height, const RGBFormat &format=RGBFormat::unknown, SurfaceFlags flags=DefaultFlags);
		~CSurface();
		Size size() const;
		int width() const;
		int height() const;
		void setWidth(int width);
		void setHeight(int height);
		void setSize(int width, int height);
		void setSize(const Size &s);
		int lock(CDrawable &draw);
		int unlock();
		SurfaceFlags surfaceFlags() const;
		void setSurfaceFlags(SurfaceFlags f);
		RGBFormat rgbFormat() const;
		void setRGBFormat(const RGBFormat &format);
		void *enginePrivate() const;
		void setEnginePrivate(void *ptr);
		GFXEngine *gfxEngine() const;
		void setGfxEngine(GFXEngine *engine);

		int create();
		int create(int width, int height, const RGBFormat &format=RGBFormat::unknown, SurfaceFlags flags=DefaultFlags);
		int copy(const CSurface &other);
		void clear();
};

class CImage : public CDrawable
{
	private:
		CMemory				myMemory;
	public:
		/** @name Konstruktoren
		 */
		//@{
		CImage();
		CImage(const CImage &other);
		CImage(const CDrawable &other);
		CImage(int width, int height, const RGBFormat &format);
		CImage(const CString &Filename, const RGBFormat &format=RGBFormat::unknown);
		CImage(CFileObject &file, const RGBFormat &format=RGBFormat::unknown);
		CImage(const CMemoryReference &mem, const RGBFormat &format=RGBFormat::unknown);
		~CImage();
		//@}

		/** @name Verschiedenes
		 */
		//@{
		int create(int width, int height, const RGBFormat &format);
		int create(void *base, ppluint32 pitch, int width, int height, const RGBFormat &format);
		int load(const CString &Filename, const RGBFormat &format=RGBFormat::unknown);
		int load(CFileObject &file, const RGBFormat &format=RGBFormat::unknown);
		int load(const CMemoryReference &Mem, const RGBFormat &format=RGBFormat::unknown);
		int copy(const CDrawable &other);
		int copy(const CDrawable &other, const Rect &rect);
		int copy(const CImage &other);
		CImage &operator=(const CDrawable &other);
		CImage &operator=(const CImage &other);
		size_t numBytes() const;
		CMemoryReference memory() const;
		operator CMemoryReference() const;
		//@}

};

class CImageList : public CImage
{
	friend class CDrawable;
	public:
		enum DRAWMETHOD {
			BLT=1,
			COLORKEY,
			ALPHABLT,
			DIFFUSE
		};
	private:
		int		width, height, numIcons;
		int		numX, numY;
		Color	colorkey;
		Color	diffuse;
		DRAWMETHOD method;

	public:

		CImageList();
		CImageList(const CImageList &other);
		CImageList(const CDrawable &draw, int icon_width, int icon_height, DRAWMETHOD method);
		CImageList(const CString &Filename, int icon_width, int icon_height, DRAWMETHOD method);
		CImageList(CFileObject &file, int icon_width, int icon_height, DRAWMETHOD method);
		CImageList(const CMemoryReference &mem, int icon_width, int icon_height, DRAWMETHOD method);
		~CImageList();

		void clear();
		void setDrawMethod(DRAWMETHOD method);
		void setColorKey(const Color &key);
		void setDiffuseColor(const Color &c);
		void setIconSize(int width, int height);

		int copy(const CImageList &other);
		int load(const CDrawable &draw,int icon_width, int icon_height, DRAWMETHOD method);
		int load(const CString &Filename, int icon_width, int icon_height, DRAWMETHOD method);
		int load(CFileObject &file, int icon_width, int icon_height, DRAWMETHOD method);
		int load(const CMemoryReference &mem, int icon_width, int icon_height, DRAWMETHOD method);

		int num() const;
		Size iconSize() const;
		Rect getRect(int nr) const;
		DRAWMETHOD drawMethod() const;
		CDrawable getDrawable(int nr) const;
		Color colorKey() const;
		Color diffuseColor() const;

		CImageList &operator=(const CImageList &other);

};

class CGrafix
{
	private:
		CMutex		Mutex;
		GFXEngine	*engine;
		GFXEngineSoftware	software;
		CList		FontEngines;
		CTree		FontList;
		CList		ImageFilter;
		CFilter		*filter_png;
		CFilter		*filter_jpeg;
		CFilter		*filter_bmp;
		CFilter		*filter_gif;
		CFilter		*filter_ppm;
		CFilter		*filter_tga;
		RGBFormat	PrimaryRGBFormat;


		void InitErrors();
		void InitAlphatab();

		int InitFunctions(const RGBFormat &format, GRAFIX_FUNCTIONS *fn);
		int InitColors(const RGBFormat &format, GRAFIX_FUNCTIONS *fn);
		int InitPixel(const RGBFormat &format, GRAFIX_FUNCTIONS *fn);
		int InitShapes(const RGBFormat &format, GRAFIX_FUNCTIONS *fn);
		int InitLines(const RGBFormat &format, GRAFIX_FUNCTIONS *fn);
		int InitBlits(const RGBFormat &format, GRAFIX_FUNCTIONS *fn);


	public:
		CGrafix();
		~CGrafix();

		CImageList	Toolbar;
		CImageList	ButtonSymbolsSmall;
		CImageList	Icons32;


		int UseEngine(GFXEngine *engine);
		GRAFIX_FUNCTIONS *GetGrafixFunctions(const RGBFormat &format);

		// Image-Filter und Loader
		int AddFilter(CFilter *filter);
		int UnloadFilter(CFilter *filter);
		CFilter *FindFilter(const CString &name);
		CFilter *FindFilter(CFileObject &ff, IMAGE &img);
		CFilter *FindFilter(CFileObject *ff, IMAGE &img);

		// Fonts
		int AddFontEngine(CFontEngine *engine);
		int LoadFont(const char *filename, const char *fontname=NULL);
		int LoadFont(CFileObject *ff, const char *fontname=NULL);
		int LoadFont(const CMemoryReference &memory, const char *fontname=NULL);
		int UnloadFont(const CString &fontname);
		CFontFile *FindFont(const CString &fontname);
		CFontFile *FindFont(const CFont &font);

		bool CreateWindow(tk::Widget &widget);
		bool CreateSurface(CSurface &surface);
};

CGrafix *GetGrafix();


class CFontFile : public CTreeItem
{
	friend class CGrafix;
	private:
	public:
		CString Name;
		CMemory Memory;
		CFontEngine *engine;
		void *priv;

		CFontFile();
		virtual ~CFontFile();
		virtual int CompareNode(CTreeItem *item);
	    virtual int CompareValue(void *value);
};

class CFontEngine : public CListItem
{
	friend class CGrafix;
	private:
		CGrafix *gfx;
	public:
		CFontEngine();
		virtual ~CFontEngine();

		virtual int Init();
		virtual int Ident(CFileObject *file);
		virtual CFontFile *LoadFont(CFileObject *file, const char *fontname);
		virtual int DeleteFont(CFontFile *file);
		virtual int Render(CFontFile *file, const CFont &font, CDrawable &draw, int x, int y, const CWString &text, const Color &color);
		virtual Size Measure(CFontFile *file, const CFont &font, const CWString &text);
};

class CFontEngineFont5 : public CFontEngine
{
	private:
		PFPChunk *SelectFont(CFontFile *file, const CFont &font);
		int RenderInternal(PFPChunk *c, const CFont &font, CDrawable &draw, int x, int y, const CWString &text, const Color &color);

	public:
		CFontEngineFont5();
		virtual ~CFontEngineFont5();
		virtual int Init();
		virtual int Ident(CFileObject *file);
		virtual CFontFile *LoadFont(CFileObject *file, const char *fontname);
		virtual int DeleteFont(CFontFile *file);
		virtual int Render(CFontFile *file, const CFont &font, CDrawable &draw, int x, int y, const CWString &text, const Color &color);
		virtual Size Measure(CFontFile *file, const CFont &font, const CWString &text);
};

class FontEngineFont6 : public CFontEngine
{
	private:

	public:
		FontEngineFont6();
		virtual ~FontEngineFont6();
		virtual int Init();
		virtual int Ident(CFileObject *file);
		virtual CFontFile *LoadFont(CFileObject *file, const char *fontname);
		virtual int DeleteFont(CFontFile *file);
		virtual int Render(CFontFile *file, const CFont &font, CDrawable &draw, int x, int y, const CWString &text, const Color &color);
		virtual Size Measure(CFontFile *file, const CFont &font, const CWString &text);
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
		virtual int Render(CFontFile *file, const CFont &font, CDrawable &draw, int x, int y, const CWString &text, const Color &color);
		virtual Size Measure(CFontFile *file, const CFont &font, const CWString &text);
};



class CFilter : public ppl6::CListItem
{
	private:

	public:
		CFilter();
		virtual ~CFilter();
		virtual int Ident(CFileObject &file, IMAGE &img);
		virtual int Load(CFileObject &file, CDrawable &surface, IMAGE &img);
		virtual int Save (const CDrawable &surface, CFileObject &file, const Rect &area, CAssocArray *param=NULL);
		int SaveFile (const CDrawable &surface, const CString &filename, const Rect &area, CAssocArray *param=NULL);
		virtual int Save (const CDrawable &surface, CFileObject &file, CAssocArray *param=NULL);
		int SaveFile (const CDrawable &surface, const CString &filename, CAssocArray *param=NULL);
		virtual CString Name();
		virtual CString Description();
};

class CFilter_PNG : public CFilter
{
	public:
		CFilter_PNG();
		virtual ~CFilter_PNG();
		virtual int Ident(CFileObject &file, IMAGE &img);
		virtual int Load(CFileObject &file, CDrawable &surface, IMAGE &img);
		virtual int Save (const CDrawable &surface, CFileObject &file, CAssocArray *param=NULL);
		virtual CString Name();
		virtual CString Description();
};

class CFilter_BMP : public CFilter
{
	public:
		CFilter_BMP();
		virtual ~CFilter_BMP();
		virtual int Ident(CFileObject &file, IMAGE &img);
		virtual int Load(CFileObject &file, CDrawable &surface, IMAGE &img);
		virtual int Save (const CDrawable &surface, CFileObject &file, CAssocArray *param=NULL);
		virtual CString Name();
		virtual CString Description();
};

class CFilter_GIF : public CFilter
{
	public:
		CFilter_GIF();
		virtual ~CFilter_GIF();
		virtual int Ident(CFileObject &file, IMAGE &img);
		virtual int Load(CFileObject &file, CDrawable &surface, IMAGE &img);
		virtual int Save (const CDrawable &surface, CFileObject &file, CAssocArray *param=NULL);
		virtual CString Name();
		virtual CString Description();
};

class CFilter_JPEG : public CFilter
{
	public:
		CFilter_JPEG();
		virtual ~CFilter_JPEG();
		virtual int Ident(CFileObject &file, IMAGE &img);
		virtual int Load(CFileObject &file, CDrawable &surface, IMAGE &img);
		virtual int Save (const CDrawable &surface, CFileObject &file, CAssocArray *param=NULL);
		virtual CString Name();
		virtual CString Description();
};

class CFilter_PPM : public CFilter
{
	public:
		CFilter_PPM();
		virtual ~CFilter_PPM();
		virtual int Ident(CFileObject &file, IMAGE &img);
		virtual int Load(CFileObject &file, CDrawable &surface, IMAGE &img);
		virtual int Save (const CDrawable &surface, CFileObject &file, CAssocArray *param=NULL);
		virtual CString Name();
		virtual CString Description();
};

class CFilter_TGA : public CFilter
{
	public:
		CFilter_TGA();
		virtual ~CFilter_TGA();
		virtual int Ident(CFileObject &file, IMAGE &img);
		virtual int Load(CFileObject &file, CDrawable &surface, IMAGE &img);
		virtual int Save (const CDrawable &surface, CFileObject &file, CAssocArray *param=NULL);
		virtual CString Name();
		virtual CString Description();
};


class CSprite
{
	private:
		CList	TextureList;
		CTree	SpriteList;
		int loadTexture(PFPChunk *chunk, CSurface::SurfaceFlags flags);
		int loadIndex(PFPChunk *chunk);
		CSurface *findTexture(int id);

	public:
		CSprite();
		~CSprite();
		int load(const CString &filename, CSurface::SurfaceFlags flags);
		int load(CFileObject &ff, CSurface::SurfaceFlags flags);
		void clear();
		int draw(CSurface &target, int x, int y, int id) const;
		int draw(CDrawable &target, int x, int y, int id) const;
		int numTextures() const;
		int numSprites() const;
};


} // EOF namespace grafix
} // EOF namespace ppl6

#endif /*GRAFIX_H_*/
