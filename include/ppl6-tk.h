/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.3 $
 * $Date: 2010/02/24 07:58:06 $
 * $Id: ppl6-tk.h,v 1.3 2010/02/24 07:58:06 pafe Exp $
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

#ifndef PPL6TK_H_
#define PPL6TK_H_

#ifndef _PPL6_INCLUDE_GRAFIX6
#ifndef _PPL6_INCLUDE
	#ifdef PPL6LIB
		#include "ppl6-grafix.h"
	#else
		#include <ppl6-grafix.h>
	#endif
#endif

#endif

#ifdef XBUTTON1
#undef XBUTTON1
#endif

#ifdef XBUTTON2
#undef XBUTTON2
#endif

namespace ppl6 {
namespace tk {

using namespace ppl6::grafix;

class Event
{
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
		};
	private:
		Type t;
		bool accepted;

	public:
		Event();
		Event(Type t);
		virtual ~Event();

		Type type() const;
		void ignore();
		void accept();
		bool isIgnored() const;
		bool isAccepted() const;


};

class MouseEvent : public Event
{
	private:
	public:

};


class EventHandler
{
	private:
		EventHandler *handler;
	public:
		EventHandler();
		virtual ~EventHandler();
		int setEventHandler(EventHandler *handler);

		virtual void mouseMoveEvent(MouseEvent *event);
		virtual void mouseDownEvent(MouseEvent *event);
		virtual void mouseUpEvent(MouseEvent *event);
		virtual void mouseDblClickEvent(MouseEvent *event);
		//virtual void mouseClickEvent(MouseEvent *event);
		virtual void mouseEnterEvent(MouseEvent *event);
		virtual void mouseLeaveEvent(MouseEvent *event);
};

class Widget : public CListItem
{
	friend class Window;
	public:
			enum WindowFlags {
			NoBorder					=	0x00000001,
			Resizeable					=	0x00000002,
			AllowDrop					=	0x00000004,
			TopMost						=	0x00000008,
			PositionDefault				=	0x00000010,
			HasCaption					=	0x00000020,
			Fullscreen					=	0x00000040,
			MinimizeButton				=	0x00000080,
			MaximizeButton				=	0x00000100,
			SystemMenue					=	0x00000200,
			Backbuffer					=	0x00010000,
			TrippleBuffer				=	0x00020000,
			Hardware					=	0x00040000,
			Lockable					=	0x00080000,
			VideoMemory					=	0x00100000,
			WaitVsync					=	0x00200000,
			ZBuffer						=	0x00400000,
			StencilBuffer				=	0x00800000,
			SoftwareVertexProcessing	=	0x01000000,
			Multithreaded				=	0x02000000,
			FPUPreserve					=	0x04000000,

			DefaultWindow				=	Resizeable|AllowDrop|PositionDefault|HasCaption|MinimizeButton|MaximizeButton|SystemMenue|Hardware|Lockable|VideoMemory|WaitVsync
		};

	private:
		Widget *parent;
		void *engineprivate;
		GFXEngine *gfxengine;
		RGBFormat	format;
		CString WindowTitle;
		CImage WindowIcon;
		CList childs;

		Point p;
		Size s;
		WindowFlags windowflags;
		bool visible;
		bool enabled;
		bool iswindow;
		ppluint32 lockcount;
		CDrawable draw;

		bool	needsredraw;
		bool	child_needsredraw;
		bool	topMost;

		void clientNeedsRedraw();
		int redraw();

	public:
		Widget(Widget *parent=NULL);
		virtual ~Widget();
		WindowFlags windowFlags() const;
		void setWindowFlags(WindowFlags f);

		Point pos() const;
		Size size() const;
		int width() const;
		int height() const;
		int x() const;
		int y() const;
		bool isVisible() const;
		bool isEnabled() const;
		Widget *parentWidget() const;
		Widget *window() const;
		Rect clientRect() const;
		CDrawable drawable() const;
		void *enginePrivate() const;
		void setEnginePrivate(void *ptr);
		GFXEngine *gfxEngine() const;
		void setGfxEngine(GFXEngine *engine);
		RGBFormat rgbFormat() const;
		void setRGBFormat(const RGBFormat &format);

		CString windowTitle() const;
		void setWindowTitle(const CString &title);
		CImage windowIcon() const;
		void setWindowIcon(const CDrawable &icon);

		void setX(int x);
		void setY(int y);
		void setPos(int x, int y);
		void setPos(const Point &p);
		void setWidth(int width);
		void setHeight(int height);
		void setSize(int width, int height);
		void setSize(const Size &s);

		int beginScene();
		int endScene();
		int blt(const CSurface &source, const Rect &srect=Rect(), int x=0, int y=0);
		int bltAlpha(const CSurface &source, const Rect &srect=Rect(), int x=0, int y=0);
		//int beginSprites();
		//int drawSprite(CSurface &source, const Rect &srect=Rect(), int x=0, int y=0, Color=0xffffffff);
		//int endSprites();


		CDrawable getDrawable();
		void releaseDrawable(const CDrawable &draw);

		int addChild(Widget *w);
		Widget* getFirstChild();
		Widget* getNextChild();
		void toTop(Widget *widget=NULL);
		void paintChilds();
		void paintNormalChilds();
		void paintTopMostChilds();
		int redrawIfNeeded();
		void needRedraw();



		virtual int paint();

};

class Window : public Widget
{
	private:

	public:
	private:


	public:
		Window(Widget *parent=NULL, WindowFlags flags=(WindowFlags)(Window::DefaultWindow));
		virtual ~Window();
		bool show();
		bool close();

};

#ifdef TRANSPARENT
#undef TRANSPARENT
#endif

class Frame : public Widget
{
	public:
		enum Framestyle {
			UPSET,
			INSET,
			FLAT,
			TRANSPARENT
		};

	private:
		CWString	Text;
		CFont		Font;
		Color		foreground, background;
		int	Style;


	public:
		Frame(Widget *parent=NULL, Framestyle style=UPSET);
		virtual ~Frame();
		CString text() const;
		void setText(const CString &text);
		Framestyle style() const;
		void setFramestyle(Framestyle style);
		Color color() const;
		void setColor(const Color &c);
		Color backgroundColor() const;
		void setBackgroundColor(const Color &c);
		CFont font() const;
		void setFont(const CFont &font);

		virtual int paint();

};

class Button : public Widget
{
	public:
		enum Buttonstyle {
			DEFAULT,
			FLAT,
			TRANSPARENT
		};

	private:
		CWString	Text;
		CFont		Font;
		Color		foreground, background;
		int	Style;


	public:
		Button(Widget *parent=NULL, Buttonstyle style=DEFAULT);
		virtual ~Button();
		CString text() const;
		void setText(const CString &text);
		Buttonstyle style() const;
		void setButtonstyle(Buttonstyle style);
		Color color() const;
		void setColor(const Color &c);
		Color backgroundColor() const;
		void setBackgroundColor(const Color &c);
		CFont font() const;
		void setFont(const CFont &font);

		virtual int paint();

};


}	// EOF namespace tk
}	// EOF namespace ppl6

#endif /*PPL6TK_H_*/
