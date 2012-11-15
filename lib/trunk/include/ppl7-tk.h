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

#ifndef _PPL7_INCLUDE_TK
#define _PPL7_INCLUDE_TK
#ifndef _PPL7_INCLUDE
    #ifdef PPL7LIB
        #include "ppl7.h"
    #else
        #include <ppl7.h>
    #endif
#endif

#ifndef _PPL7_GRAFIX
    #ifdef PPL7LIB
        #include "ppl7-grafix.h"
    #else
        #include <ppl7-grafix.h>
    #endif
#endif


namespace ppl7 {
namespace tk {

PPLNORMALEXCEPTION(NoWindowManagerException);
PPLNORMALEXCEPTION(DuplicateWindowManagerException);
PPLPARAMETERISEDEXCEPTION(InitializationException);
PPLNORMALEXCEPTION(ExistingPrimaryWindowException);
PPLPARAMETERISEDEXCEPTION(WindowCreateException);
PPLNORMALEXCEPTION(WindowAlreadyCreatedException);
PPLPARAMETERISEDEXCEPTION(SurfaceCreateException);
PPLNORMALEXCEPTION(EventLoopException);
PPLNORMALEXCEPTION(UnknownEventException);
PPLNORMALEXCEPTION(NoWindowException);

using namespace ppl7::grafix;

class WidgetStyle
{
	public:
		Color	colorWindowBackground;
		Color	frameBackgroundColor;
		Color	frameBorderColor;
		Color	labelFontColor;
		Font	labelFont;
		Color	buttonBackgroundColor;
		Color	buttonFontColor;
		Font	buttonFont;
		Color	surfaceBackgroundColor;
};

class Widget;

class Event
{
		friend class EventHandler;
	public:
		enum Type {
			Unknown=0,
			MouseEnter,
			MouseMove,
			MouseLeave,
			MouseDown,
			MouseUp,
			MouseDblClick,
			MouseWheel,
			KeyDown,
			KeyUp,
			Show,
			Close,
			Resize,
			Move,
			FocusIn,
			FocusOut,
			GeometryChanged,
		};
	private:
		Type t;
		bool accepted;
		bool ignored;
		Widget *eventWidget;

	public:
		Event();
		Event(Type t);
		virtual ~Event();

		Type type() const;
		void setType(Type t);
		void ignore();
		void accept();
		bool isIgnored() const;
		bool isAccepted() const;
		String name() const;
		Widget *widget() const;
		void setWidget(Widget *w);


};

class MouseEvent : public Event
{
	private:

	public:
		MouseEvent();
		enum MouseButton {
			Left=1,
			Middle=2,
			Right=4,
			WheelUp=8,
			WheelDown=16,
			X1=32,
			X2=64
		};
		Point p;
		MouseButton buttonMask;
		MouseButton button;

};

class ResizeEvent : public Event
{
	public:
		ResizeEvent();
		int width, height;
};


class EventHandler
{
	private:
		EventHandler *handler;
	public:
		EventHandler();
		virtual ~EventHandler();
		void setEventHandler(EventHandler *handler);

		virtual void mouseMoveEvent(MouseEvent *event);
		virtual void mouseDownEvent(MouseEvent *event);
		virtual void mouseUpEvent(MouseEvent *event);
		virtual void mouseDblClickEvent(MouseEvent *event);
		virtual void mouseClickEvent(MouseEvent *event);
		virtual void mouseEnterEvent(MouseEvent *event);
		virtual void mouseLeaveEvent(MouseEvent *event);
		virtual void geometryChangedEvent(Event *event);
};

/*
class Surface
{
	private:
		void		*myData;
		int			myFlags;
		int			w,h;
		RGBFormat	myFormat;
		void (*privLock) (void *data, Drawable &draw);
		void (*privUnlock) (void *data);
		void (*privDestroy) (void *data);
		void (*privDraw) (void *target, void *data, int x, int y);

	public:
		enum SurfaceFlags {
			Hardware					=	0x00000001,
			Lockable					=	0x00000002,
			VideoMemory					=	0x00000004,
			ZBuffer						=	0x00000008,
			StencilBuffer				=	0x00000010,
			SoftwareVertexProcessing	=	0x00000020,
			Multithreaded				=	0x00000040,
			FPUPreserve					=	0x00000080,
			OpenGL						=	0x00000100,
			Texture						=	0x00000200,
			DefaultSurface				=	Hardware|Lockable|VideoMemory,
		};
		Surface();
		~Surface();
		void lock(Drawable &draw);
		void unlock();
		void draw(Surface *target, int x, int y);
		SurfaceFlags flags() const;
		const RGBFormat &rgbFormat() const;
		int width() const;
		int height() const;

		void setLockFunction( void (*fnLock) (void *, Drawable &));
		void setUnlockFunction( void (*fnUnlock) (void *));
		void setDestroyFunction( void (*fnDestroy) (void *));
		void setDrawFunction( void (*fnDraw) (void *, void *, int, int));
		void setFlags(SurfaceFlags flags);
		void setRGBFormat(const RGBFormat &format);
		void setSize(int width, int height);
		void setPrivateData(void *data);

};
*/


class Widget : public EventHandler
{
		friend class WindowManager;
	private:
		Widget		*parent;
		//Surface		*surface;
		RGBFormat	format;
		Point		p;
		Size		s;
		Size		MaxSize, MinSize;
		Rect		myClientOffset;
		List<Widget*>	childs;
		ppluint32	lockcount;
		bool		visible;
		bool		enabled;
		bool		needsredraw;
		bool		child_needsredraw;
		bool		topMost;
		bool		transparent;
		Size		strategy;
		String		myName;


	public:
		enum SizeStrategy {
			FIXED=1,
			MAXIMUM_EXPANDING,
			MINIMUM_EXPANDING,
		};

		Widget();
		virtual ~Widget();
		size_t numChilds() const;
		void resetIterator(List<Widget*>::Iterator &it);
		Widget *getNextChild(List<Widget*>::Iterator &it);

		const Point &pos() const;
		const Size &size() const;
		Rect rect() const;
		Rect clientRect() const;
		const Size maxSize() const;
		const Size minSize() const;
		void setMaxSize(const Size &s);
		void setMaxSize(int width=65535, int height=65535);
		void setMinSize(const Size &s);
		void setMinSize(int width=65535, int height=65535);
		int width() const;
		int height() const;
		int x() const;
		int y() const;
		void create(int x, int y, int width, int height);
		void setX(int x);
		void setY(int y);
		void setPos(int x, int y);
		void setPos(const Point &p);
		void setWidth(int width);
		void setHeight(int height);
		void setSize(int width, int height);
		void setSize(const Size &s);
		void setTopmost(bool flag);
		void setClientOffset(int left, int top, int right, int bottom);
		void setSizeStrategyWidth(SizeStrategy s);
		SizeStrategy sizeStrategyWidth() const;
		void setSizeStrategyHeight(SizeStrategy s);
		SizeStrategy sizeStrategyHeight() const;

		void setTransparent(bool flag);
		bool isTransparent() const;
		void setEnabled(bool flag);
		bool isEnabed() const;
		void setVisible(bool flag);
		bool isVisible() const;

		Drawable drawable(const Drawable &parent) const;
		Drawable clientDrawable(const Drawable &parent) const;

		void addChild(Widget *w);
		void removeChild(Widget *w);

		void needsRedraw();
		void childNeedsRedraw();
		void parentMustRedraw();
		void geometryChanged();
		void toTop(Widget *w=NULL);
		void toTop(Widget &w);
		void draw(Drawable &d);
		void redraw(Drawable &d);
		bool redrawRequired() const;
		Size preferedSize() const;
		void setName(const String &name);
		String name() const;
		virtual String widgetType() const;

		virtual void paint(Drawable &draw);
		virtual Size contentSize() const;
};

class WindowManager;

class Window : public Widget
{
	private:
		void			*privateData;
		WindowManager	*wm;
		ppluint32 windowFlags;
		String WindowTitle;
		Image WindowIcon;
		RGBFormat WindowRGBFormat;

	public:
		enum WindowFlags {
			NoBorder					=	0x00000001,
			Resizeable					=	0x00000002,
			Maximized					=	0x00000004,
			Minimized					=	0x00000008,
			TopMost						=	0x00000010,
			Fullscreen					=	0x00000020,
			WaitVsync					=	0x00200000,
			ZBuffer						=	0x00400000,
			StencilBuffer				=	0x00800000,
			SoftwareVertexProcessing	=	0x01000000,
			OpenGL						=	0x08000000,
			DefaultWindow				=	WaitVsync,
			DefaultFullscreen			=	NoBorder|WaitVsync|Fullscreen|TopMost,
		};
		Window();
		~Window();
		ppluint32 flags() const;
		void setFlags(ppluint32 flags);
		const String &windowTitle() const;
		void setWindowTitle(const String &title);
		const Drawable &windowIcon() const;
		void setWindowIcon(const Drawable &icon);
		const RGBFormat &rgbFormat() const;
		void setRGBFormat(const RGBFormat &format);

		void *getPrivateData();
		void setPrivateData(void *data, WindowManager *wm);

		virtual String widgetType() const;
		virtual void paint(Drawable &draw);
};

class WindowManager
{
	private:
		WidgetStyle Style;
		Widget	*LastMouseDown;
		Widget	*LastMouseEnter;
		Widget	*LastMouseClick;

		Widget *findMouseWidget(Widget *window, MouseEvent *event);
	public:
		WindowManager();
		virtual ~WindowManager();
		const WidgetStyle *getWidgetStyle() const;
		void dispatchEvent(Widget *window, Event &event);

		virtual void createWindow(Window &w) = 0;
		virtual void destroyWindow(Window &w) = 0;
		virtual void setWindowTitle(Window &w) = 0;
		virtual void setWindowIcon(Window &w) = 0;


		virtual const Size &desktopResolution() const =0;
		virtual const RGBFormat &desktopRGBFormat() const =0;


		virtual void getMouseState(Point &p, int &buttonMask)=0;
		virtual void startEventLoop() = 0;
		virtual int handleEvents() = 0;
		virtual void drawIfNeeded() const = 0;
		virtual void draw() const = 0;
		//virtual void createSurface(Widget &w, int width, int height, const RGBFormat &format=RGBFormat(), int flags=Surface::DefaultSurface) = 0;

};

WindowManager *GetWindowManager();
const WidgetStyle *GetWidgetStyle();


class WindowManager_SDL2 : public WindowManager
{
	private:
		RGBFormat	screenRGBFormat;
		Size		screenSize;
		int			screenRefreshRate;

		void DispatchSdlActiveEvent(void *e);
		void DispatchSdlKeyEvent(void *e);
		void DispatchSdlMouseEvent(void *e);
		void DispatchSdlResizeEvent(void *e);
		void DispatchEvent(void *e);

	public:
		WindowManager_SDL2();
		~WindowManager_SDL2();
		//virtual void createSurface(Widget &w, int width, int height, const RGBFormat &format=RGBFormat(), int flags=Surface::DefaultSurface);
		virtual void createWindow(Window &w);
		virtual void destroyWindow(Window &w);
		virtual void setWindowTitle(Window &w);
		virtual void setWindowIcon(Window &w);

		virtual const Size &desktopResolution() const;
		virtual const RGBFormat &desktopRGBFormat() const;
		virtual void getMouseState(Point &p, int &buttonMask);
		virtual void startEventLoop();
		virtual int handleEvents();
		virtual void drawIfNeeded() const;
		virtual void draw() const;



};









class Button : public Widget
{
	private:
		Font	myFont;
		String	Text;
		Image	Icon;
		Color	foreground, background;
		bool	isDown;
	public:
		Button();
		virtual ~Button();
		const String &text() const;
		void setText(const String &text);
		const Drawable &icon() const;
		void setIcon(const Drawable &icon);
		int style() const;
		const Color &color() const;
		void setColor(const Color &c);
		const Color &backgroundColor() const;
		void setBackgroundColor(const Color &c);
		const Font &font() const;
		void setFont(const Font &font);

		virtual String widgetType() const;
		virtual void paint(Drawable &draw);
		virtual Size contentSize() const;

		virtual void mouseDownEvent(MouseEvent *event);
		virtual void mouseUpEvent(MouseEvent *event);
		virtual void mouseLeaveEvent(MouseEvent *event);

};

class Frame : public Widget
{
	private:
		int		myBorderStyle;
		Color	myBackground, myBorderColor;
	public:
		enum BorderStyle {
			NoBorder=0,
			Normal,
			Upset,
			Inset
		};
		Frame();
		Frame(int x, int y, int width, int height);
		~Frame();
		BorderStyle borderStyle() const;
		void setBorderStyle(BorderStyle s);
		const Color &backgroundColor() const;
		void setBackgroundColor(const Color &c);
		const Color &borderColor() const;
		void setBorderColor(const Color &c);

		virtual String widgetType() const;
		virtual void paint(Drawable &draw);
};

class Label : public Frame
{
	private:
		String	myText;
		Font	myFont;
		Image	myIcon;
		Color	myColor;
	public:
		Label();
		Label(int x, int y, int width, int height, const String &text=String());
		~Label();
		const String &text() const;
		void setText(const String &text);
		const Drawable &icon() const;
		void setIcon(const Drawable &icon);
		const Color &color() const;
		void setColor(const Color &c);
		const Font &font() const;
		void setFont(const Font &font);

		virtual String widgetType() const;
		virtual void paint(Drawable &draw);
		virtual Size contentSize() const;
};

class HorizontalLayout : public Widget
{
	private:
		int	spacing;
		int lastTotal;
	public:
		HorizontalLayout();
		virtual ~HorizontalLayout();

		virtual String widgetType() const;
		virtual void paint(Drawable &draw);
		virtual void geometryChangedEvent(Event *event);
};

class HorizontalSpacer : public Widget
{
	public:
		HorizontalSpacer();
		virtual String widgetType() const;
		virtual void paint(Drawable &draw);

};


} // EOF namespace tk
} // end of namespace ppl7


#endif // _PPL7_INCLUDE_TK



