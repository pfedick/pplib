/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
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

#include <list>


namespace ppl7 {
namespace tk {

PPL7EXCEPTION(NoWindowManagerException, Exception);
PPL7EXCEPTION(DuplicateWindowManagerException, Exception);
PPL7EXCEPTION(InitializationException, Exception);
PPL7EXCEPTION(ExistingPrimaryWindowException, Exception);
PPL7EXCEPTION(WindowCreateException, Exception);
PPL7EXCEPTION(WindowAlreadyCreatedException, Exception);
PPL7EXCEPTION(SurfaceCreateException, Exception);
PPL7EXCEPTION(EventLoopException, Exception);
PPL7EXCEPTION(UnknownEventException, Exception);
PPL7EXCEPTION(NoWindowException, Exception);
PPL7EXCEPTION(SDLException, Exception);

using namespace ppl7::grafix;

class WidgetStyle
{
public:
	enum PredefinedStyle {
		Light=0,
		Dark=1
	};
	WidgetStyle(PredefinedStyle style=Light);
	Color	windowBackgroundColor;
	Color	frameBackgroundColor;
	Color	frameBorderColorLight;
	Color	frameBorderColorShadow;
	Color	labelFontColor;
	Font	labelFont;
	Color	buttonBackgroundColor;
	Color	buttonFontColor;
	Font	buttonFont;
	Color	surfaceBackgroundColor;
	Color	inputFontColor;
	Font	inputFont;
	Color	inputBackgroundColor;
	Color	inputInvalidBackgroundColor;
	Color   inputSelectedBackgroundColor;
	Color	sliderHighlightColor;
	Color	sliderSelectedColor;
	Color	buttonSymbolColor;
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
		Quit,
		ValueChanged,
		Toggled,
		TextChanged,
		SelectionChanged,
		GameControllerAxisMotion,
		GameControllerButtonDown,
		GameControllerButtonUp,
		GameControllerDeviceAdded,
		GameControllerDeviceRemoved,
		DropFileEvent,
		DropTextEvent
	};
private:
	Type t;
	bool accepted;
	bool ignored;
	Widget* eventWidget;
	int custom_id;


public:
	Event();
	Event(Type t);
	virtual ~Event();

	Type type() const;
	void setType(Type t);
	void setCustomId(int id);
	void ignore();
	void accept();
	bool isIgnored() const;
	bool isAccepted() const;
	int customId() const;
	String name() const;
	Widget* widget() const;
	void setWidget(Widget* w);


};

class MouseState
{
public:
	MouseState();
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
	Point wheel;
	MouseButton buttonMask;
	MouseButton button;
};

class MouseEvent : public Event, public MouseState
{
private:

public:
};


class KeyEvent : public Event
{
public:
	enum keycode {
		KEY_UNKNOWN = 0,
		KEY_0			=1,
		KEY_1			=2,
		KEY_2			=3,
		KEY_3			=4,
		KEY_4			=5,
		KEY_5			=6,
		KEY_6			=7,
		KEY_7			=8,
		KEY_8			=9,
		KEY_9			=10,
		KEY_a			=11,
		KEY_b			=12,
		KEY_c			=13,
		KEY_d			=14,
		KEY_e			=15,
		KEY_f			=16,
		KEY_g			=17,
		KEY_h			=18,
		KEY_i			=19,
		KEY_j			=20,
		KEY_k			=21,
		KEY_l			=22,
		KEY_m			=23,
		KEY_n			=24,
		KEY_o			=25,
		KEY_p			=26,
		KEY_q			=27,
		KEY_r			=28,
		KEY_s			=29,
		KEY_t			=30,
		KEY_u			=31,
		KEY_v			=32,
		KEY_w			=33,
		KEY_x			=34,
		KEY_y			=35,
		KEY_z			=36,
		KEY_RETURN		= 40,
		KEY_ESCAPE		= 41,
		KEY_BACKSPACE	= 42,
		KEY_TAB			= 43,
		KEY_SPACE		= 44,
		KEY_CAPSLOCK	= 57,
		KEY_F1			= 58,
		KEY_F2			= 59,
		KEY_F3			= 60,
		KEY_F4			= 61,
		KEY_F5			= 62,
		KEY_F6			= 63,
		KEY_F7			= 64,
		KEY_F8			= 65,
		KEY_F9			= 66,
		KEY_F10			= 67,
		KEY_F11			= 68,
		KEY_F12			= 69,
		KEY_PRINTSCREEN	= 70,
		KEY_SCROLLLOCK	= 71,
		KEY_PAUSE		= 72,
		KEY_INSERT		= 73,
		KEY_HOME		= 74,
		KEY_PAGEUP		= 75,
		KEY_DELETE		= 76,
		KEY_END			= 77,
		KEY_PAGEDOWN	= 78,
		KEY_RIGHT		= 79,
		KEY_LEFT		= 80,
		KEY_DOWN		= 81,
		KEY_UP			= 82,
		KEY_NUMLOCK		= 83,
		KEY_ENTER		= 88,
		KEY_LEFTCTRL	= 224,
		KEY_LEFTSHIFT	= 225,
		KEY_LEFTALT		= 226,
		KEY_LEFTGUI		= 227,
		KEY_RIGHTCTRL	= 224,
		KEY_RIGHTSHIFT	= 225,
		KEY_RIGHTALT	= 226,
		KEY_RIGHTGUI	= 227,
		KEY_MODE		= 257,
	};

	enum keymodifier {
		KEYMOD_NONE			= 0,
		KEYMOD_LEFTSHIFT	= 0x0001,
		KEYMOD_RIGHTSHIFT	= 0x0002,
		KEYMOD_LEFTCTRL		= 0x0004,
		KEYMOD_RIGHTCTRL	= 0x0008,
		KEYMOD_LEFTALT		= 0x0010,
		KEYMOD_RIGHTALT		= 0x0020,
		KEYMOD_LEFTGUI		= 0x0040,
		KEYMOD_RIGHTGUI		= 0x0080,
		KEYMOD_MODIFIER		= 0x00ff,
		KEYMOD_NUM			= 0x0100,
		KEYMOD_CAPS			= 0x0200,
		KEYMOD_MODE			= 0x0400,

		KEYMOD_SHIFT		= KEYMOD_LEFTSHIFT | KEYMOD_RIGHTSHIFT,
		KEYMOD_CTRL			= KEYMOD_LEFTCTRL | KEYMOD_RIGHTCTRL,
		KEYMOD_ALT			= KEYMOD_LEFTALT | KEYMOD_RIGHTALT,
		KEYMOD_GUI			= KEYMOD_LEFTGUI | KEYMOD_RIGHTGUI,


	};
	KeyEvent();
	int key;
	int	modifier;
	bool repeat;
};

class TextInputEvent : public Event
{
public:
	WideString text;
};



class ResizeEvent : public Event
{
public:
	ResizeEvent();
	int width, height;
};

class FocusEvent : public Event
{
private:
	Widget* new_widget;
	bool bAccepted;
public:
	FocusEvent(Event::Type t, Widget* old_widget, Widget* new_widget);
	Widget* oldWidget() const;
	Widget* newWidget() const;
	void accept(bool flag);
	bool accepted() const;
};

class GameControllerAxisEvent : public Event
{
public:
	int which;
	int axis;
	int value;
};

class GameControllerButtonEvent : public Event
{
public:
	int which;
	int button;
	int state;
};

class GameControllerEvent : public Event
{
public:
	int which;
};

class DropEvent : public Event
{
public:
	ppl7::String text;
};


class EventHandler
{
private:
	EventHandler* handler;
public:
	EventHandler();
	virtual ~EventHandler();
	void setEventHandler(EventHandler* handler);

	virtual void closeEvent(Event* event);
	virtual void quitEvent(Event* event);
	virtual void resizeEvent(ResizeEvent* event);

	virtual void mouseMoveEvent(MouseEvent* event);
	virtual void mouseDownEvent(MouseEvent* event);
	virtual void mouseUpEvent(MouseEvent* event);
	virtual void mouseDblClickEvent(MouseEvent* event);
	virtual void mouseClickEvent(MouseEvent* event);
	virtual void mouseEnterEvent(MouseEvent* event);
	virtual void mouseLeaveEvent(MouseEvent* event);
	virtual void mouseWheelEvent(MouseEvent* event);
	virtual void geometryChangedEvent(Event* event);

	virtual void gotFocusEvent(FocusEvent* event);
	virtual void lostFocusEvent(FocusEvent* event);

	virtual void keyDownEvent(KeyEvent* event);
	virtual void keyUpEvent(KeyEvent* event);
	virtual void textInputEvent(TextInputEvent* event);

	virtual void timerEvent(Event* event);

	virtual void valueChangedEvent(Event* event, int value);
	virtual void valueChangedEvent(Event* event, double value);
	virtual void valueChangedEvent(Event* event, int64_t value);
	virtual void toggledEvent(Event* event, bool checked);
	virtual void textChangedEvent(Event* event, const ppl7::String& text);
	virtual void selectionChangedEvent(Event* event);

	virtual void gameControllerAxisMotionEvent(GameControllerAxisEvent* event);
	virtual void gameControllerButtonDownEvent(GameControllerButtonEvent* event);
	virtual void gameControllerButtonUpEvent(GameControllerButtonEvent* event);
	virtual void gameControllerDeviceAdded(GameControllerEvent* event);
	virtual void gameControllerDeviceRemoved(GameControllerEvent* event);

	virtual void dropEvent(DropEvent* event);

};

typedef struct PRIV_SURFACE_FUNCTIONS
{
	void (*lock) (void* privatedata, Drawable& draw);
	void (*unlock) (void* privatedata);
	void (*destroy) (void* privatedata);
	void (*update) (void* privatedata, const Drawable& source);
} PRIV_SURFACE_FUNCTIONS;

class Surface
{
private:
	PRIV_SURFACE_FUNCTIONS* fn;
	void* privatedata;
	int			myFlags;
	int			w, h;
	RGBFormat	myFormat;

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
		DefaultSurface				=	Hardware | Lockable | VideoMemory,
	};
	Surface();
	~Surface();
	bool isLoackable() const;
	void lock(Drawable& draw);
	void unlock();
	void update(const Drawable& source);
	SurfaceFlags flags() const;
	const RGBFormat& rgbFormat() const;
	int width() const;
	int height() const;

	void setFlags(SurfaceFlags flags);
	void setRGBFormat(const RGBFormat& format);
	void setSize(int width, int height);
	void setPrivateData(void* data, PRIV_SURFACE_FUNCTIONS* fn);
	void* getPrivateData();
};



class Widget : public EventHandler
{
	friend class WindowManager;
private:
	Widget* parent;
	Image 	drawbuffer;
	//Surface		*surface;
	RGBFormat	format;
	Point		p;
	Size		s;
	Size		MaxSize, MinSize;
	Rect		myClientOffset;
	std::list<Widget*>	childs;
	uint32_t	lockcount;
	bool		visible;
	bool		enabled;
	bool		needsredraw;
	bool		child_needsredraw;
	bool		topMost;
	bool		transparent;
	bool		deleteRequested;
	bool		modal;
	bool        use_own_drawbuffer;
	Size		strategy;
	String		myName;
	void updateDrawbuffer();
	void markWidgetsAboveForRedraw(Widget* widget);

public:
	enum SizeStrategy {
		FIXED=1,
		MAXIMUM_EXPANDING,
		MINIMUM_EXPANDING,
	};

	Widget();
	Widget(int x, int y, int width, int height);
	virtual ~Widget();
	size_t numChilds() const;
	std::list<Widget*>::iterator childsBegin();
	std::list<Widget*>::iterator childsEnd();
	std::list<Widget*>::const_iterator childsBegin() const;
	std::list<Widget*>::const_iterator childsEnd() const;

	void deleteLater();
	Widget* getParent() const;
	Widget* getTopmostParent() const;

	void setUseOwnDrawbuffer(bool enable);
	void destroyChilds();

	const Point& pos() const;
	Point absolutePosition() const;
	const Size& size() const;
	Rect rect() const;
	Rect clientRect() const;
	Size clientSize() const;
	const Size maxSize() const;
	const Size minSize() const;
	void setMaxSize(const Size& s);
	void setMaxSize(int width=65535, int height=65535);
	void setMinSize(const Size& s);
	void setMinSize(int width=65535, int height=65535);
	int width() const;
	int height() const;
	int x() const;
	int y() const;
	void create(int x, int y, int width, int height);
	void setX(int x);
	void setY(int y);
	void setPos(int x, int y);
	void setPos(const Point& p);
	void setWidth(int width);
	void setHeight(int height);
	void setSize(int width, int height);
	void setSize(const Size& s);
	void setTopmost(bool flag);
	void setClientOffset(int left, int top, int right, int bottom);
	void setSizeStrategyWidth(SizeStrategy s);
	SizeStrategy sizeStrategyWidth() const;
	void setSizeStrategyHeight(SizeStrategy s);
	SizeStrategy sizeStrategyHeight() const;

	void setTransparent(bool flag);
	bool isTransparent() const;
	void setEnabled(bool flag);
	bool isEnabled() const;
	void setVisible(bool flag);
	bool isVisible() const;
	void setFocus();
	bool hasFocus() const;


	void setModal(bool flag);
	bool isModal() const;

	Drawable drawable(const Drawable& parent) const;
	Drawable clientDrawable(const Drawable& parent) const;

	void addChild(Widget* w);
	void removeChild(Widget* w);

	void needsRedraw();
	void childNeedsRedraw();
	void parentMustRedraw();
	void geometryChanged();
	void toTop(Widget* w=NULL);
	void toTop(Widget& w);
	void draw(Drawable& d);
	void redraw(Drawable& d);
	bool redrawRequired() const;
	Size preferedSize() const;
	void setName(const String& name);
	String name() const;

	bool isChildOf(Widget* other) const;


	virtual String widgetType() const;

	virtual void paint(Drawable& draw);
	virtual Size contentSize() const;
};

class WindowManager;
class Window;

typedef struct PRIV_WINDOW_FUNCTIONS {
	void (*setWindowTitle) (void* privatedata, const String& Title);
	void (*setWindowIcon) (void* privatedata, const Drawable& Icon);
	void (*createSurface) (void* privatedata);
	void (*createTexture) (void* privatedata);
	Drawable(*lockWindowSurface) (void* privatedata);
	void (*unlockWindowSurface) (void* privatedata);
	void (*drawWindowSurface) (void* privatedata);
	void* (*getRenderer) (void* privatedata);
	void (*clearScreen) (void* privatedata);
	void (*presentScreen) (void* privatedata);
} PRIV_WINDOW_FUNCTIONS;


class Window : public Widget
{
	friend class WindowManager;
private:
	void* privateData;
	PRIV_WINDOW_FUNCTIONS* fn;
	WindowManager* wm;
	Widget* keyfocus;
	uint32_t windowFlags;
	String WindowTitle;
	Image WindowIcon;
	RGBFormat WindowRGBFormat;
	Color	myBackground;

	MouseState	mouseState;

public:
	enum WindowFlags {
		NoBorder					=	0x00000001,
		Resizeable					=	0x00000002,
		Maximized					=	0x00000004,
		Minimized					=	0x00000008,
		TopMost						=	0x00000010,
		Fullscreen					=	0x00000020,
		FullscreenDesktop			=	0x00000040,
		WaitVsync					=	0x00200000,
		ZBuffer						=	0x00400000,
		StencilBuffer				=	0x00800000,
		SoftwareVertexProcessing	=	0x01000000,
		OpenGL						=	0x08000000,
		DefaultWindow				=	WaitVsync,
		DefaultFullscreen			=	NoBorder | WaitVsync | Fullscreen | TopMost,
		DefaultFullscreenDesktop	=	NoBorder | WaitVsync | FullscreenDesktop | TopMost,
	};

	enum class WindowMode {
		Window=0,
		Fullscreen,
		FullscreenDesktop
	};

	class DisplayMode {
	public:
		DisplayMode();
		DisplayMode(const ppl7::grafix::RGBFormat& format, int width, int height, int refresh_rate);
		ppl7::grafix::RGBFormat	format;
		int			width;
		int			height;
		int			refresh_rate;
	};

	Window();
	~Window();
	uint32_t flags() const;
	void setFlags(uint32_t flags);
	const String& windowTitle() const;
	void setWindowTitle(const String& title);
	const Drawable& windowIcon() const;
	void setWindowIcon(const Drawable& icon);
	const RGBFormat& rgbFormat() const;
	void setRGBFormat(const RGBFormat& format);

	void setPos(int x, int y);
	void setPos(const Point& p);

	const Color& backgroundColor() const;
	void setBackgroundColor(const Color& c);

	void setWindowDisplayMode(const DisplayMode& mode);

	void drawWidgets();
	void redrawWidgets();

	void* getPrivateData();
	void setPrivateData(void* data, WindowManager* wm, PRIV_WINDOW_FUNCTIONS* fn);

	void* getRenderer();
	void clearScreen();
	void presentScreen();
	MouseState getMouseState();

	virtual String widgetType() const;
	virtual void paint(Drawable& draw);
};

class WindowManager
{
private:
	WidgetStyle Style;
	Widget* LastMouseDown;
	Widget* LastMouseEnter;
	Widget* LastMouseFocus;

	MouseEvent	clickEvent;
	int			clickCount;
	int			doubleClickIntervall;

	Widget* KeyboardFocus;
	Widget* GameControllerFocus;
	Widget* grabMouseWidget;
	void updateButtonSymbols();
	void deferedDeleteWidgets(Widget* widget);

public:
	ImageList	ButtonSymbols;

	WindowManager();
	virtual ~WindowManager();
	const WidgetStyle& getWidgetStyle() const;
	void setWidgetStyle(const WidgetStyle& style);
	void dispatchEvent(Window* window, Event& event);
	void dispatchClickEvent(Window* window);
	void setDoubleClickIntervall(int ms);
	void setKeyboardFocus(Widget* w);
	void setGameControllerFocus(Widget* w);
	void setMouseFocus(Widget* w);
	void grabMouse(Widget* w);
	void releaseMouse(Widget* w);
	void unregisterWidget(Widget* w);
	Widget* getKeyboardFocus() const;
	Widget* getGameControllerFocus() const;
	int getDoubleClickIntervall() const;
	Widget* findMouseWidget(Widget* window, Point& p);

	virtual void createWindow(Window& w) = 0;
	virtual void destroyWindow(Window& w) = 0;
	virtual void setWindowPosition(Window& w, int x, int y) =0;
	virtual void resizeWindow(Window& w, int width, int height)=0;
	virtual void setWindowDisplayMode(Window& w, const Window::DisplayMode& mode)=0;

	virtual const Size& desktopResolution() const =0;
	virtual const RGBFormat& desktopRGBFormat() const =0;

	virtual MouseState getMouseState() const =0;
	virtual void getMouseState(Point& p, int& buttonMask) const=0;
	virtual void startEventLoop() = 0;
	virtual void handleEvents() = 0;
	virtual size_t numWindows() = 0;
	virtual void startClickEvent(Window* win) = 0;
	virtual int startTimer(Widget* w, int intervall) =0;
	virtual void removeTimer(int timer_id)=0;
	//virtual void createSurface(Widget &w, int width, int height, const RGBFormat &format=RGBFormat(), int flags=Surface::DefaultSurface) = 0;

	virtual void setClipboardText(const ppl7::String& text)=0;
	virtual bool hasClipboardText() const=0;
	virtual String getClipboardText() const=0;
};

WindowManager* GetWindowManager();
const WidgetStyle& GetWidgetStyle();


class WindowManager_SDL2 : public WindowManager
{
private:
	RGBFormat	screenRGBFormat;
	Size		screenSize;
	int			screenRefreshRate;

	List<Window*>	windows;
	Window* lastWindowEnterEvent;

	void DispatchSdlActiveEvent(void* e);
	void DispatchSdlKeyEvent(void* e);
	void DispatchMouseEvent(void* e);
	void DispatchKeyEvent(void* e);
	void DispatchSdlResizeEvent(void* e);
	void DispatchEvent(void* e);
	void DispatchQuitEvent(void* e);

	void DispatchWindowEvent(void* e);

	Window* getWindow(uint32_t id);

public:
	WindowManager_SDL2();
	~WindowManager_SDL2();
	//virtual void createSurface(Widget &w, int width, int height, const RGBFormat &format=RGBFormat(), int flags=Surface::DefaultSurface);
	virtual void createWindow(Window& w);
	virtual void destroyWindow(Window& w);
	virtual void setWindowPosition(Window& w, int x, int y);
	virtual void resizeWindow(Window& w, int width, int height);
	virtual void setWindowDisplayMode(Window& w, const Window::DisplayMode& mode);

	virtual const Size& desktopResolution() const;
	virtual const RGBFormat& desktopRGBFormat() const;
	virtual MouseState getMouseState() const;
	virtual void getMouseState(Point& p, int& buttonMask) const;
	virtual void startEventLoop();
	virtual void handleEvents();
	virtual size_t numWindows();
	virtual void startClickEvent(Window* win);

	virtual void setClipboardText(const ppl7::String& text);
	virtual bool hasClipboardText() const;
	virtual String getClipboardText() const;


	int startTimer(Widget* w, int intervall);
	void removeTimer(int timer_id);

	void changeWindowMode(Window& w, Window::WindowMode mode);
	Window::WindowMode getWindowMode(Window& w);
	void* getSDLWindow(Window& w);

};


class Button : public Widget
{
private:
	Font	myFont;
	String	Text;
	Image	Icon;
	Color	foreground, background;
	bool	isDown;
	bool	is_checkable;
	bool	is_checked;
public:
	Button();
	Button(int x, int y, int width, int height, const String& text=String(), const Drawable& icon=Drawable());
	virtual ~Button();
	const String& text() const;
	void setText(const String& text);
	const Drawable& icon() const;
	void setIcon(const Drawable& icon);
	int style() const;
	const Color& color() const;
	void setColor(const Color& c);
	const Color& backgroundColor() const;
	void setBackgroundColor(const Color& c);
	const Font& font() const;
	void setFont(const Font& font);

	bool isCheckable() const;
	void setCheckable(bool);
	bool isChecked() const;
	void setChecked(bool);



	virtual String widgetType() const;
	virtual void paint(Drawable& draw);
	virtual Size contentSize() const;

	virtual void mouseDownEvent(MouseEvent* event);
	virtual void mouseUpEvent(MouseEvent* event);
	virtual void mouseLeaveEvent(MouseEvent* event);

};

class Frame : public Widget
{
private:
	int		myBorderStyle;
	Color	myBackground, myBorderColorLight, myBorderColorShadow;
public:
	enum BorderStyle {
		NoBorder=0,
		Normal,
		Upset,
		Inset
	};
	Frame();
	Frame(int x, int y, int width, int height, BorderStyle style=Upset);
	~Frame();
	BorderStyle borderStyle() const;
	void setBorderStyle(BorderStyle s);
	const Color& backgroundColor() const;
	void setBackgroundColor(const Color& c);
	const Color& borderColorLight() const;
	void setBorderColorLight(const Color& c);
	const Color& borderColorShadow() const;
	void setBorderColorShadow(const Color& c);

	virtual String widgetType() const;
	virtual void paint(Drawable& draw);
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
	Label(int x, int y, int width, int height, const String& text=String(), BorderStyle style=NoBorder);
	~Label();
	const String& text() const;
	void setText(const String& text);
	const Drawable& icon() const;
	void setIcon(const Drawable& icon);
	const Color& color() const;
	void setColor(const Color& c);
	const Font& font() const;
	void setFont(const Font& font);

	virtual String widgetType() const;
	virtual void paint(Drawable& draw);
	virtual Size contentSize() const;
};

class VerticalDivider : public Widget
{
public:
	VerticalDivider();
	VerticalDivider(int x, int y, int width, int height);
	virtual String widgetType() const;
	virtual void paint(Drawable& draw);
	virtual Size contentSize() const;
};

class HorizontalDivider : public Widget
{
public:
	HorizontalDivider();
	HorizontalDivider(int x, int y, int width, int height);
	virtual String widgetType() const;
	virtual void paint(Drawable& draw);
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
	virtual void paint(Drawable& draw);
	virtual void geometryChangedEvent(Event* event);
};

class HorizontalSpacer : public Widget
{
public:
	HorizontalSpacer();
	virtual String widgetType() const;
	virtual void paint(Drawable& draw);

};

class InputValidator
{
public:
	virtual bool validateText(const ppl7::WideString& text)=0;
	virtual bool validateInput(const ppl7::WideString& text)=0;
};

class LineInput : public Frame
{
private:
	class Selection
	{
	public:
		int x1;
		int x2;
		int start, end;

		Selection();
		bool exists() const;
		void clear();
		void begin(int position);


	};
	WideString	myText;
	WideString validatedText;
	Font	myFont;
	Image	myIcon;
	Color	myColor;
	Color	myBackgroundColor;
	size_t	cursorpos;
	size_t	startpos;
	Selection selection;
	int		cursorx;
	int		cursorwidth;
	bool	blinker;
	bool 	drag_started;
	bool	overwrite;
	int		drag_start_position;
	int		timerId;
	InputValidator* validator;

	void calcSelectionPosition();
	void calcCursorPosition();
	int calcPosition(int x);
	int getDrawStartPositionOfChar(size_t pos);
	void validateAndSendEvent(const WideString& text);
	void deleteSelection();
public:
	LineInput();
	LineInput(int x, int y, int width, int height, const String& text=String());
	~LineInput();
	const WideString& text() const;
	void setText(const String& text);
	const Color& color() const;
	void setColor(const Color& c);
	const Font& font() const;
	void setFont(const Font& font);
	const Color& backgroundColor() const;
	void setBackgroundColor(const Color& c);


	void setInputValidator(InputValidator* validator);

	virtual String widgetType() const;
	virtual void paint(Drawable& draw);
	virtual Size contentSize() const;

	virtual void mouseDownEvent(MouseEvent* event);
	virtual void mouseMoveEvent(ppl7::tk::MouseEvent* event);
	virtual void mouseUpEvent(ppl7::tk::MouseEvent* event);
	virtual void gotFocusEvent(FocusEvent* event);
	virtual void lostFocusEvent(FocusEvent* event);
	virtual void textInputEvent(TextInputEvent* event);
	virtual void keyDownEvent(KeyEvent* event);
	virtual void keyUpEvent(KeyEvent* event);
	virtual void timerEvent(Event* event);
	virtual void mouseDblClickEvent(MouseEvent* event);

};


class Scrollbar : public ppl7::tk::Widget
{
private:
	ppl7::tk::Button* up_button;
	ppl7::tk::Button* down_button;
	int size;
	int pos;
	int visibleItems;

	ppl7::grafix::Rect slider_pos;
	bool drag_started;
	int drag_offset;
	ppl7::grafix::Point drag_start_pos;

public:
	Scrollbar(int x, int y, int width, int height);
	~Scrollbar();
	void setSize(int size);
	void setPosition(int position);
	void setVisibleItems(int items);
	int position() const;
	virtual ppl7::String widgetType() const;
	virtual void paint(ppl7::grafix::Drawable& draw);
	void mouseDownEvent(ppl7::tk::MouseEvent* event);
	void mouseUpEvent(ppl7::tk::MouseEvent* event);
	void lostFocusEvent(ppl7::tk::FocusEvent* event);
	void mouseMoveEvent(ppl7::tk::MouseEvent* event);
	void mouseWheelEvent(ppl7::tk::MouseEvent* event);

};

class CheckBox : public ppl7::tk::Label
{
private:
	bool	ischecked;

public:
	CheckBox();
	CheckBox(int x, int y, int width, int height, const ppl7::String& text=ppl7::String(), bool checked=false);
	~CheckBox();
	bool checked() const;
	void setChecked(bool checked);

	virtual ppl7::String widgetType() const;
	virtual void paint(ppl7::grafix::Drawable& draw);
	virtual void mouseDownEvent(ppl7::tk::MouseEvent* event);
};

class RadioButton : public ppl7::tk::Label
{
private:
	bool	ischecked;

public:
	RadioButton();
	RadioButton(int x, int y, int width, int height, const ppl7::String& text=ppl7::String(), bool checked=false);
	~RadioButton();
	bool checked() const;
	void setChecked(bool checked);
	virtual ppl7::String widgetType() const;
	virtual void paint(ppl7::grafix::Drawable& draw);
	virtual void mouseDownEvent(ppl7::tk::MouseEvent* event);
};

class ListWidget : public ppl7::tk::Frame
{
public:
	class ListWidgetItem
	{
	public:
		ppl7::String text;
		ppl7::String identifier;
		size_t index;
	};
private:
	Scrollbar* scrollbar;
	ppl7::String myCurrentText;
	ppl7::String myCurrentIdentifier;
	size_t myCurrentIndex;
	size_t visibleItems;
	int mouseOverIndex;

	std::list<ListWidgetItem> items;
public:
	ListWidget(int x, int y, int width, int height);
	void setCurrentText(const ppl7::String& text);
	ppl7::String currentText() const;
	ppl7::String currentIdentifier() const;

	void setCurrentIndex(size_t index);
	size_t currentIndex() const;

	std::list<ListWidgetItem> getItems() const;

	void clear();

	void add(const ppl7::String& text, const ppl7::String& identifier=ppl7::String());
	void remove(size_t index);
	void remove(const ppl7::String& identifier);

	virtual ppl7::String widgetType() const;
	virtual void paint(ppl7::grafix::Drawable& draw);
	virtual void valueChangedEvent(ppl7::tk::Event* event, int value);
	virtual void mouseDownEvent(ppl7::tk::MouseEvent* event);
	virtual void mouseWheelEvent(ppl7::tk::MouseEvent* event);
	virtual void lostFocusEvent(ppl7::tk::FocusEvent* event);
	virtual void mouseMoveEvent(ppl7::tk::MouseEvent* event);
};

class ComboBox : public ppl7::tk::Widget
{
private:
	ppl7::tk::Label* dropdown_button;
	ppl7::String myCurrentText;
	ppl7::String myCurrentIdentifier;
	size_t myCurrentIndex;

	class ComboBoxItem
	{
	public:
		ppl7::String text;
		ppl7::String identifier;
		size_t index;
	};
	std::list<ComboBoxItem> items;

	ListWidget* selection;

public:
	ComboBox(int x, int y, int width, int height);
	~ComboBox();

	void setCurrentText(const ppl7::String& text);
	void setCurrentIdentifier(const ppl7::String& identifier);
	ppl7::String currentText() const;
	ppl7::String currentIdentifier() const;

	void setCurrentIndex(size_t index);
	size_t currentIndex() const;

	void clear();

	void add(const ppl7::String& text, const ppl7::String& identifier=ppl7::String());

	virtual ppl7::String widgetType() const;
	virtual void paint(ppl7::grafix::Drawable& draw);
	virtual void mouseDownEvent(ppl7::tk::MouseEvent* event);
	virtual void mouseWheelEvent(ppl7::tk::MouseEvent* event);
	virtual void valueChangedEvent(ppl7::tk::Event* event, int value);
	virtual void lostFocusEvent(ppl7::tk::FocusEvent* event);
};

class AbstractSpinBox : public Widget
{
private:
	Image img_up, img_down;
	Button* up_button;
	Button* down_button;
	LineInput* text_input;


	void createUi();
	void resizeUi();
	void debugEvent(const ppl7::String& name, Event* event);

public:
	AbstractSpinBox();
	AbstractSpinBox(int x, int y, int width, int height, const String& text=String());
	~AbstractSpinBox();

	const Color& color() const;
	void setColor(const Color& c);
	const Font& font() const;
	void setFont(const Font& font);

	void setBackgroundColor(const ppl7::grafix::Color& color);
	const Color& backgroundColor() const;

	void setText(const String& value);
	String text() const;

	void setInputValidator(InputValidator* validator);

	//virtual bool validateText(const ppl7::WideString& text)=0;

	virtual String widgetType() const;
	virtual void paint(Drawable& draw);


	virtual void mouseDownEvent(MouseEvent* event);
	virtual void gotFocusEvent(FocusEvent* event);
	virtual void lostFocusEvent(FocusEvent* event);
	virtual void textInputEvent(TextInputEvent* event);
	virtual void keyDownEvent(KeyEvent* event);
	virtual void keyUpEvent(KeyEvent* event);
	virtual void textChangedEvent(Event* event, const String& text);

	virtual void stepUp()=0;
	virtual void stepDown()=0;

};

class SpinBox : public AbstractSpinBox, public InputValidator
{
private:
	int64_t my_value;
	int64_t step_size;
	int64_t min, max;

public:
	SpinBox();
	SpinBox(int x, int y, int width, int height, int64_t value=0);
	virtual String widgetType() const;

	void setValue(int64_t value);
	int64_t value() const;
	void setMinimum(int64_t value);
	void setMaximum(int64_t value);
	void setLimits(int64_t min, int64_t max);
	int64_t minimum() const;
	int64_t maximum() const;
	void setStepSize(int64_t value);
	int64_t stepSize() const;

	void stepUp();
	void stepDown();

	bool validateText(const ppl7::WideString& text);
	bool validateInput(const ppl7::WideString& text);
	virtual void textChangedEvent(Event* event, const String& text);
};

class DoubleSpinBox : public AbstractSpinBox, public InputValidator
{
private:
	int my_decimals;
	double my_value;
	double step_size;
	double min, max;

public:
	DoubleSpinBox();
	DoubleSpinBox(int x, int y, int width, int height, double value=0.0f, int decimals=2);
	virtual String widgetType() const;

	void setValue(double value);
	double value() const;
	void setMinimum(double value);
	void setMaximum(double value);
	void setLimits(double min, double max);
	double minimum() const;
	double maximum() const;
	void setStepSize(double value);
	double stepSize() const;
	void setDecimals(int decimals);
	int decimals() const;

	void stepUp();
	void stepDown();


	bool validateText(const ppl7::WideString& text);
	bool validateInput(const ppl7::WideString& text);
	virtual void textChangedEvent(Event* event, const String& text);

};

class AbstractSlider : public ppl7::tk::Widget
{
	friend class HorizontalSlider;
private:
	int64_t min;
	int64_t max;
	int64_t current_value;
	int64_t my_steps;
protected:
	virtual void sliderValueChanged(int64_t value)=0;

public:
	AbstractSlider(int x, int y, int width, int height);

	void setMinimum(int64_t value);
	void setMaximum(int64_t value);
	void setLimits(int64_t min, int64_t max);
	void setValue(int64_t value);
	void setSteps(int64_t value);

	int64_t value() const;
	int64_t minimum() const;
	int64_t maximum() const;
	int64_t stepSize() const;
	int64_t steps() const;



};

class DoubleAbstractSlider : public ppl7::tk::Widget
{
	friend class HorizontalSlider;
private:
	double min;
	double max;
	double current_value;
	double my_steps;
protected:
	virtual void sliderValueChanged(double value)=0;

public:
	DoubleAbstractSlider(int x, int y, int width, int height);

	void setMinimum(double value);
	void setMaximum(double value);
	void setLimits(double min, double max);
	void setValue(double value);
	void setSteps(double value);

	double value() const;
	double minimum() const;
	double maximum() const;
	double stepSize() const;
	double steps() const;

};

class HorizontalSlider : public AbstractSlider
{
private:
	ppl7::grafix::Rect slider_pos;
	bool drag_started;
	int drag_offset;
	ppl7::grafix::Point drag_start_pos;
	SpinBox* spinbox;
	int start_x;

	void sliderValueChanged(int64_t value);
	void valueChangedEvent(ppl7::tk::Event* event, int64_t value);

public:
	HorizontalSlider(int x, int y, int width, int height);
	~HorizontalSlider();
	void enableSpinBox(bool enabled, int64_t stepsize=1, int width=100);

	virtual void paint(ppl7::grafix::Drawable& draw);
	virtual void mouseDownEvent(ppl7::tk::MouseEvent* event);
	virtual void mouseUpEvent(ppl7::tk::MouseEvent* event);
	virtual void lostFocusEvent(ppl7::tk::FocusEvent* event);
	virtual void mouseMoveEvent(ppl7::tk::MouseEvent* event);
	virtual void mouseWheelEvent(ppl7::tk::MouseEvent* event);



};

class DoubleHorizontalSlider : public DoubleAbstractSlider
{
private:
	ppl7::grafix::Rect slider_pos;
	bool drag_started;
	int drag_offset;
	ppl7::grafix::Point drag_start_pos;
	DoubleSpinBox* spinbox;
	int start_x;

	void sliderValueChanged(double value);
	void valueChangedEvent(ppl7::tk::Event* event, double value);

public:
	DoubleHorizontalSlider(int x, int y, int width, int height);
	~DoubleHorizontalSlider();
	void enableSpinBox(bool enabled, double stepsize=0.1f, int decimals=2, int width=100);

	virtual void paint(ppl7::grafix::Drawable& draw);
	virtual void mouseDownEvent(ppl7::tk::MouseEvent* event);
	virtual void mouseUpEvent(ppl7::tk::MouseEvent* event);
	virtual void lostFocusEvent(ppl7::tk::FocusEvent* event);
	virtual void mouseMoveEvent(ppl7::tk::MouseEvent* event);
	virtual void mouseWheelEvent(ppl7::tk::MouseEvent* event);


};


} // EOF namespace tk
} // end of namespace ppl7


#endif // _PPL7_INCLUDE_TK
