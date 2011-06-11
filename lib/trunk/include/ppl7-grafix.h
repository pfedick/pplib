/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 38 $
 * $Date: 2011-04-26 21:39:41 +0200 (Di, 26. Apr 2011) $
 * $Id: ppl7-sound.h 38 2011-04-26 19:39:41Z pafe $
 * $Url$
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
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

#ifndef _PPL7_INCLUDE_GRAFIX
#define _PPL7_INCLUDE_GRAFIX
#ifndef _PPL7_INCLUDE
    #ifdef PPL7LIB
        #include "ppl7.h"
    #else
        #include <ppl7.h>
    #endif
#endif



namespace ppl7 {
namespace grafix {


PPLPARAMETERISEDEXCEPTION(UnknownColorFormatException);
PPLPARAMETERISEDEXCEPTION(UnsupportedColorFormatException);
PPLNORMALEXCEPTION(NoGrafixEngineException);
PPLNORMALEXCEPTION(EmptyDrawableException);
PPLNORMALEXCEPTION(UnknownBltMethodException);
PPLNORMALEXCEPTION(DuplicateGrafixEngineException);
PPLPARAMETERISEDEXCEPTION(FunctionUnavailableException);
PPLNORMALEXCEPTION(InvalidImageSizeException);
PPLNORMALEXCEPTION(UnknownImageFormatException);
PPLNORMALEXCEPTION(FontEngineInitializationException);
PPLNORMALEXCEPTION(InvalidFontException);
PPLNORMALEXCEPTION(NoSuitableFontEngineException);
PPLNORMALEXCEPTION(FontNotFoundException);



// Microsoft kompatible Strukturen
typedef struct _RECT {
		long left;
		long top;
		long right;
		long bottom;
} RECT, *PRECT;

typedef struct tagPOINT {
		long x;
		long y;
} POINT, *PPOINT;



typedef struct tagSIZE {
		long cx;
		long cy;
} SIZE, *PSIZE;


typedef ppluint32 SurfaceColor;

class Point
{
	public:
		int x,y;

		Point();
		Point(int x, int y);
		Point(const Point &other);
		bool isNull() const;
		double vectorLength() const;
		int manhattanLength() const;
		void setX(int x);
		void setY(int y);
		void setPoint(int x, int y);
		void setPoint(const Point &other);
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

	public:
		int x,y,z;
		Point3D();
		Point3D(int x, int y, int z);
		Point3D(const Point3D &other);
		bool isNull() const;
		void setX(int x);
		void setY(int y);
		void setZ(int z);
		void setPoint(int x, int y, int z);
		void setPoint(const Point3D &other);
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
	public:
		int width,height;

		Size();
		Size(int width, int height);
		Size(const Size &other);
		bool isNull() const;
		bool isEmpty() const;
		bool isValid() const;
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
	public:
		int x1,y1;
		int x2,y2;
		Rect();
		Rect(const Point &p1, const Point &p2);
		Rect(int x, int y, int width, int height);
		Rect(const Rect &r);

		bool isNull() const;
		int left() const;
		int right() const;
		int top() const;
		int bottom() const;
		int width() const;
		int height() const;
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
		void setRect(const Rect &r);
		void setRect(int x, int y, int width, int height);
		void setCoords(int x1, int y1, int x2, int y2);
		void setCoords(const Point &p1, const Point &p2);
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
	RGBFormat(const String &Identifier);

	void setFormat(Identifier id);
	void setFormat(const String &Identifier);

	int format() const;
	String name() const;
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


typedef struct {
	int			width;
	int			height;
	int			pitch;
	int			bitdepth;
	int			colors;
	RGBFormat	format;
} IMAGE;


class Color
{
	friend bool operator!= (const Color &c1, const Color &c2);
	friend bool operator== (const Color &c1, const Color &c2);
	friend const Color operator* (const Color &size, float factor);
	friend const Color operator* (float factor, const Color &size);

	private:
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

} GRAFIX_FUNCTIONS;


class ImageList;
class Sprite;

class Font
{
	friend bool operator!= (const Font &f1, const Font &f2);
	friend bool operator== (const Font &f1, const Font &f2);

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

		String Name;
		Color	cForeground;
		Color	cBorder;
		Color	cShadow;
		ppluint16	fontSize;
		ppluint8	flags;
		ppluint8	ori;
	public:
		enum Orientation {
			LEFT =1,
			MIDDLE,
			RIGHT,
			TOP,
			BOTTOM,
			BASE
		};


		Font();
		Font(const Font &other);

		const String &name() const;
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
		Size measure(const String &text) const;

		int setName(const String &name);
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

		Font &operator=(const Font &other);
};
bool operator!= (const Font &f1, const Font &f2);
bool operator== (const Font &f1, const Font &f2);



class Drawable
{
	friend class Image;
	private:
		GRAFIX_FUNCTIONS	*fn;
		DRAWABLE_DATA		data;
		void initFunctions(const RGBFormat &format);

	public:
		/** @name Konstruktoren
		 */
		//@{
		Drawable();
		Drawable(const Drawable &other);
		Drawable(const Drawable &other, const Rect &rect);
		Drawable(void *base, ppluint32 pitch, int width, int height, const RGBFormat &format);
		~Drawable();
		//@}

		/** @name Verschiedenes
		 */
		//@{
		GRAFIX_FUNCTIONS *getFunctions();
		DRAWABLE_DATA *getData();

		void copy(const Drawable &other);
		void copy(const Drawable &other, const Rect &rect);
		void create(void *base, ppluint32 pitch, int width, int height, const RGBFormat &format);
		Drawable &operator=(const Drawable &other);

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
		Drawable getDrawable() const;
		Drawable getDrawable(const Rect &rect) const;
		Drawable getDrawable(int x1, int y1, int x2, int y2) const;
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
		void	print(const Font &font, int x, int y, const String &text);
		void	printf(const Font &font, int x, int y, const char *fmt, ...);
		//@}

		/** @name Blit-Funktionen
		 * Kopieren von Grafiken mit verschiedenen Methoden
		 */
		//@{
		int fitRect(int &x, int &y, Rect &r);
		void blt(const Drawable &source, int x=0, int y=0);
		void blt(const Drawable &source, const Rect &srect, int x=0, int y=0);
		void bltDiffuse(const Drawable &source, int x=0, int y=0, const Color &c=Color());
		void bltDiffuse(const Drawable &source, const Rect &srect, int x=0, int y=0, const Color &c=Color());
		void bltColorKey(const Drawable &source, int x=0, int y=0, const Color &c=Color());
		void bltColorKey(const Drawable &source, const Rect &srect, int x=0, int y=0, const Color &c=Color());
		void bltAlpha(const Drawable &source, int x=0, int y=0);
		void bltAlpha(const Drawable &source, const Rect &srect, int x=0, int y=0);

		void draw(const ImageList &iml, int nr, int x, int y);
		void draw(const ImageList &iml, int nr, int x, int y, const Color &diffuse);
		void draw(const Sprite &sprite, int nr, int x, int y);
		//@}
};

class Image : public Drawable
{
	private:
		ByteArray	myMemory;
	public:
		/** @name Konstruktoren
		 */
		//@{
		Image();
		Image(const Image &other);
		Image(const Drawable &other);
		Image(int width, int height, const RGBFormat &format);
		Image(const String &Filename, const RGBFormat &format=RGBFormat::unknown);
		Image(FileObject &file, const RGBFormat &format=RGBFormat::unknown);
		Image(const ByteArrayPtr &mem, const RGBFormat &format=RGBFormat::unknown);
		~Image();
		//@}

		/** @name Verschiedenes
		 */
		//@{
		void create(int width, int height, const RGBFormat &format);
		void create(void *base, ppluint32 pitch, int width, int height, const RGBFormat &format);
		void load(const String &Filename, const RGBFormat &format=RGBFormat::unknown);
		void load(FileObject &file, const RGBFormat &format=RGBFormat::unknown);
		void load(const ByteArrayPtr &Mem, const RGBFormat &format=RGBFormat::unknown);
		void copy(const Drawable &other);
		void copy(const Drawable &other, const Rect &rect);
		void copy(const Image &other);
		Image &operator=(const Drawable &other);
		Image &operator=(const Image &other);
		size_t numBytes() const;
		ByteArrayPtr memory() const;
		operator ByteArrayPtr() const;
		//@}

};

class ImageList : public Image
{
	friend class Drawable;
	public:
		enum DRAWMETHOD {
			BLT=1,
			COLORKEY,
			ALPHABLT,
			DIFFUSE
		};
	private:
		size_t	numIcons;
		int		width, height;
		int		numX, numY;
		Color	colorkey;
		Color	diffuse;
		DRAWMETHOD method;

	public:

		ImageList();
		ImageList(const ImageList &other);
		ImageList(const Drawable &draw, int icon_width, int icon_height, DRAWMETHOD method);
		ImageList(const String &Filename, int icon_width, int icon_height, DRAWMETHOD method);
		ImageList(FileObject &file, int icon_width, int icon_height, DRAWMETHOD method);
		ImageList(const ByteArrayPtr &mem, int icon_width, int icon_height, DRAWMETHOD method);
		~ImageList();

		void clear();
		void setDrawMethod(DRAWMETHOD method);
		void setColorKey(const Color &key);
		void setDiffuseColor(const Color &c);
		void setIconSize(int width, int height);

		void copy(const ImageList &other);
		void load(const Drawable &draw,int icon_width, int icon_height, DRAWMETHOD method);
		void load(const String &Filename, int icon_width, int icon_height, DRAWMETHOD method);
		void load(FileObject &file, int icon_width, int icon_height, DRAWMETHOD method);
		void load(const ByteArrayPtr &mem, int icon_width, int icon_height, DRAWMETHOD method);

		size_t num() const;
		Size iconSize() const;
		Rect getRect(size_t nr) const;
		DRAWMETHOD drawMethod() const;
		Drawable getDrawable(size_t nr) const;
		Color colorKey() const;
		Color diffuseColor() const;

		ImageList &operator=(const ImageList &other);

};


class ImageFilter;
class FontEngine;
class FontFile;

class Grafix
{
	private:
		Mutex		myMutex;
		/*
		CList		FontEngines;
		CTree		FontList;
		CList		ImageFilter;
		*/
		List<ImageFilter*>	ImageFilterList;
		List<FontEngine*>	FontEngineList;
		AVLTree<String, FontFile*> FontList;

		ImageFilter		*filter_png;
		ImageFilter		*filter_jpeg;
		ImageFilter		*filter_bmp;
		ImageFilter		*filter_gif;
		ImageFilter		*filter_ppm;
		ImageFilter		*filter_tga;
		RGBFormat	PrimaryRGBFormat;


		void initAlphatab();

		void initFunctions(const RGBFormat &format, GRAFIX_FUNCTIONS *fn);
		void initColors(const RGBFormat &format, GRAFIX_FUNCTIONS *fn);
		void initPixel(const RGBFormat &format, GRAFIX_FUNCTIONS *fn);
		void initShapes(const RGBFormat &format, GRAFIX_FUNCTIONS *fn);
		void initLines(const RGBFormat &format, GRAFIX_FUNCTIONS *fn);
		void initBlits(const RGBFormat &format, GRAFIX_FUNCTIONS *fn);


	public:
		Grafix();
		~Grafix();

		/*
		ImageList	Toolbar;
		ImageList	ButtonSymbolsSmall;
		ImageList	Icons32;
		*/


		GRAFIX_FUNCTIONS *getGrafixFunctions(const RGBFormat &format);

		// Image-Filter und Loader
		void addImageFilter(ImageFilter *filter);
		void unloadImageFilter(ImageFilter *filter);
		ImageFilter *findImageFilter(const String &name);
		ImageFilter *findImageFilter(FileObject &ff, IMAGE &img);

		// Fonts
		void addFontEngine(FontEngine *engine);
		void loadFont(const String &filename, const String &fontname=String());
		void loadFont(FileObject &ff, const String &fontname=String());
		void loadFont(const ByteArrayPtr &memory, const String &fontname=String());
		void unloadFont(const String &fontname);
		FontFile *findFont(const String &fontname);
		FontFile *findFont(const Font &font);
};

Grafix *GetGrafix();




class FontFile
{
	friend class Grafix;
	private:
	public:
		String Name;
		ByteArray Memory;
		FontEngine *engine;
		void *priv;

		FontFile();
		virtual ~FontFile();
};

class FontEngine
{
	friend class Grafix;
	private:
	public:
		FontEngine();
		virtual ~FontEngine();

		virtual int init();
		virtual int ident(FileObject &file);
		virtual FontFile *loadFont(FileObject &file, const String &fontname);
		virtual void deleteFont(FontFile *file);
		virtual void render(const FontFile &file, const Font &font, Drawable &draw, int x, int y, const String &text, const Color &color);
		virtual Size measure(const FontFile &file, const Font &font, const String &text);
};

class FontEngineFont5 : public FontEngine
{
	private:
		PFPChunk *selectFont(FontFile *file, const Font &font);
		int renderInternal(PFPChunk *c, const Font &font, Drawable &draw, int x, int y, const String &text, const Color &color);

	public:
		FontEngineFont5();
		virtual ~FontEngineFont5();
		virtual int init();
		virtual int ident(FileObject *file);
		virtual FontFile *loadFont(FileObject &file, const String &fontname);
		virtual void deleteFont(FontFile *file);
		virtual void render(const FontFile &file, const Font &font, Drawable &draw, int x, int y, const String &text, const Color &color);
		virtual Size measure(const FontFile &file, const Font &font, const String &text);
};

class FontEngineFreeType : public FontEngine
{
	private:
		void *ft;
	public:
		FontEngineFreeType();
		virtual ~FontEngineFreeType();
		virtual int init();
		virtual int ident(FileObject &file);
		virtual FontFile *loadFont(FileObject &file, const String &fontname);
		virtual void deleteFont(FontFile *file);
		virtual void render(const FontFile &file, const Font &font, Drawable &draw, int x, int y, const String &text, const Color &color);
		virtual Size measure(const FontFile &file, const Font &font, const String &text);
};




class ImageFilter
{
	private:

	public:
		PPLNORMALEXCEPTION(IllegalImageFormatException);
		PPLNORMALEXCEPTION(EmptyImageException);

		ImageFilter();
		virtual ~ImageFilter();
		virtual int ident(FileObject &file, IMAGE &img);
		virtual void load(FileObject &file, Drawable &surface, IMAGE &img);
		virtual void save (const Drawable &surface, FileObject &file, const Rect &area, const AssocArray &param=AssocArray());
		virtual void save (const Drawable &surface, FileObject &file, const AssocArray &param=AssocArray());
		virtual String name();
		virtual String description();

		void saveFile (const Drawable &surface, const String &filename, const Rect &area, const AssocArray &param=AssocArray());
		void saveFile (const Drawable &surface, const String &filename, const AssocArray &param=AssocArray());
};

class ImageFilter_PNG : public ImageFilter
{
	public:
		ImageFilter_PNG();
		virtual ~ImageFilter_PNG();
		virtual int ident(FileObject &file, IMAGE &img);
		virtual void load(FileObject &file, Drawable &surface, IMAGE &img);
		virtual void save (const Drawable &surface, FileObject &file, const AssocArray &param=AssocArray());
		virtual String name();
		virtual String description();
};

#ifdef DONE

class ImageFilter_BMP : public ImageFilter
{
	public:
		ImageFilter_BMP();
		virtual ~ImageFilter_BMP();
		virtual int Ident(FileObject &file, IMAGE &img);
		virtual int Load(FileObject &file, Drawable &surface, IMAGE &img);
		virtual int Save (const Drawable &surface, FileObject &file, CAssocArray *param=NULL);
		virtual String Name();
		virtual String Description();
};

class ImageFilter_GIF : public ImageFilter
{
	public:
		ImageFilter_GIF();
		virtual ~ImageFilter_GIF();
		virtual int Ident(FileObject &file, IMAGE &img);
		virtual int Load(FileObject &file, Drawable &surface, IMAGE &img);
		virtual int Save (const Drawable &surface, FileObject &file, CAssocArray *param=NULL);
		virtual String Name();
		virtual String Description();
};

class ImageFilter_JPEG : public ImageFilter
{
	public:
		ImageFilter_JPEG();
		virtual ~ImageFilter_JPEG();
		virtual int Ident(FileObject &file, IMAGE &img);
		virtual int Load(FileObject &file, Drawable &surface, IMAGE &img);
		virtual int Save (const Drawable &surface, FileObject &file, CAssocArray *param=NULL);
		virtual String Name();
		virtual String Description();
};

class ImageFilter_PPM : public ImageFilter
{
	public:
		ImageFilter_PPM();
		virtual ~ImageFilter_PPM();
		virtual int Ident(FileObject &file, IMAGE &img);
		virtual int Load(FileObject &file, Drawable &surface, IMAGE &img);
		virtual int Save (const Drawable &surface, FileObject &file, CAssocArray *param=NULL);
		virtual String Name();
		virtual String Description();
};

class ImageFilter_TGA : public ImageFilter
{
	public:
		ImageFilter_TGA();
		virtual ~ImageFilter_TGA();
		virtual int Ident(FileObject &file, IMAGE &img);
		virtual int Load(FileObject &file, Drawable &surface, IMAGE &img);
		virtual int Save (const Drawable &surface, FileObject &file, CAssocArray *param=NULL);
		virtual String Name();
		virtual String Description();
};

#endif



} // EOF namespace grafix
} // end of namespace ppl7


#endif // _PPL7_INCLUDE_GRAFIX

