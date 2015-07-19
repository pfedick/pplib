/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6). 
 * Web: http://www.pfp.de/ppl/
 *  
 * $Author: patrick $
 * $Revision: 1.43 $
 * $Date: 2009/01/08 16:51:48 $
 * $Id: ppl6-tk.h,v 1.43 2009/01/08 16:51:48 patrick Exp $
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

#ifndef PPL6TK_H_
#define PPL6TK_H_

#ifndef _PPL6_INCLUDE_GRAFIX6
#ifndef _PPL6_INCLUDE
	#ifdef PPL6LIB
		#include "ppl6-grafix.h"
	#else
		#include <ppl6-grafix.h>
		//#include <ppl6-tk.h>
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

namespace SYSKEY {
enum {
	CONTROL_LEFT=		0x00000001,
	CONTROL_RIGHT=		0x00000002,
	CONTROL=			0x00000003,
	SHIFT_LEFT=			0x00000004,
	SHIFT_RIGHT=		0x00000008,
	SHIFT=				0x0000000c,
	ALT=				0x00000010,
	ALTGR=				0x00000020,
	LBUTTON=			0x00000040,
	MBUTTON=			0x00000080,
	RBUTTON=			0x00000100,
	XBUTTON1=			0x00000200,
	XBUTTON2=			0x00000400,
	CAPSLOCK=			0x00001000,
	INSERT=				0x00002000,
	NUMLOCK=			0x00010000,
};
}

namespace EVENTTYPE {
enum {
	MOUSEMOTION=1,
	MOUSEDOWN,
	MOUSEUP,
	MOUSEENTER,
	MOUSELEAVE,
	KEYDOWN,
	KEYUP
};
}

namespace ALIGNMENT {
enum ALIGNMENT {
	NONE=0,
	LEFT=1,
	RIGHT=2,
	CENTER=3
};
}

namespace KEY {
enum KEY {
	UNKNOWN=0,
	Left,
	Right,
	Up,
	Down,
	Home,
	End,
	Insert,
	Delete,
	PageUp,
	PageDown,
	Backspace,
	Esc,
	Tab,
	Print,
	ScrollLock,
	Pause,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	ShiftLeft,
	ShiftRight,
	ControlLeft,
	ControlRight,
	Alt,
	AltGr,
	CapsLock,
	NumLock,
	Return,
	Break
};
}

namespace WIDGETSTYLE {
	enum WIDGETSTYLE {
		DEFAULT=0,
		UPSET,
		INSET,
		SIMPLE,
		BUTTON,
		PPLWINDOW,
		FLAT
	};
}

typedef struct {
	ppl6::grafix::COLOR background;
	ppl6::grafix::COLOR border;
	ppl6::grafix::COLOR caption;
	ppl6::grafix::COLOR background_inactive;
	ppl6::grafix::COLOR border_inactive;
	ppl6::grafix::COLOR caption_inactive;
	ppl6::grafix::COLOR text;
} WINDOWCOLOR;

typedef struct {
	ppl6::grafix::COLOR caption;
	ppl6::grafix::COLOR caption_background;
	ppl6::grafix::COLOR background;
	ppl6::grafix::COLOR border_light;
	ppl6::grafix::COLOR border_shadow;
} WIDGETCOLOR;

typedef struct {
	ppl6::grafix::COLOR background;
	ppl6::grafix::COLOR caption;
	ppl6::grafix::COLOR caption_inactive;
	ppl6::grafix::COLOR background_inactive;
	ppl6::grafix::COLOR border_light;
	ppl6::grafix::COLOR border_shadow;
} TABCOLOR;

typedef struct {
	WINDOWCOLOR window;
	WIDGETCOLOR button;
	WIDGETCOLOR	frame;
	WIDGETCOLOR	panel;
	WIDGETCOLOR	input;
	TABCOLOR	tab;
	ppl6::grafix::COLOR scrollbar_background;
	ppl6::grafix::COLOR scrollbar_highlight;
} COLORSTYLE;


typedef struct tagEvent {
	int type;
	int syskey;
	int ascii;
	int x,y,buttons;
} EVENT;



class CEvent
{
	private:
		CEvent	*handler;
	public:
		CEvent();
		virtual ~CEvent();
		int SetEventHandler(CEvent *handler);
		
		virtual int AppActivate(CEvent *object);
		virtual int AppInactivate(CEvent *object);
		virtual int WindowCloseRequest(CEvent *object);
		virtual int WindowClose(CEvent *object);
		virtual int WindowResize(CEvent *object, int width, int height);
		virtual int WindowMove(CEvent *object, int x, int y);
		virtual int GotFocus(CEvent *object);
		virtual int LostFocus(CEvent *object);
		virtual int MouseEnter(CEvent *object);
		virtual int MouseLeave(CEvent *object);
		virtual int MouseMove(CEvent *object, int x, int y, int buttons);
		virtual int MouseDown(CEvent *object, int x, int y, int buttons);
		virtual int MouseUp(CEvent *object, int x, int y, int buttons);
		virtual int MouseClick(CEvent *object, int x, int y, int buttons);
		virtual int MouseDblClick(CEvent *object, int x, int y, int buttons);
		virtual int MouseEvent(CEvent *object, int x, int y, int buttons);
		virtual int KeyDown(CEvent *object, int unicode, int buttons, int keycode);
		virtual int KeyUp(CEvent *object, int unicode, int buttons, int keycode);
		virtual int Message(CEvent *object, ppl6::CAssocArray *msg);
};

typedef struct tagWIDGET
{
	int x,y;
	int width, height;
} WIDGET;

class CWidget : public CEvent, public CListItem, public CCallback
{
	friend class ppl6::grafix::CGrafix;
	friend class CWindow;
	friend class SubWindow;
	private:
		WIDGET	w;
		RECT	client;
		POINT	absolute;
		CWidget *parent;
		CWidget *lastevent;
		CWidget	*moveevent;
		int		movebuttons;
		POINT	lastmovepoint;
		COLORSTYLE	*colors;
		
		CList	childs;
		//CGrafix	*widgetgfx;
		ppl6::grafix::CSurface *surface, *ownsurface;
		bool	needsredraw;
		bool	child_needsredraw;
		bool	created;
		
		void ClientNeedsRedraw();
		int Redraw();

		void StartMove(CWidget *w, int buttons);
		
	public:
		int			CID;
		CString		Name;
		CBool		Enabled;
		CBool		Visible;
		CBool		TopMost;
		CInt		Flags;
		CInt		RGBFormat;
		
		ppl6::grafix::CGrafix		*gfx;
		
		CWidget();
		virtual ~CWidget();
		//int InitGrafix(CGrafix *g);
		void PaintChilds();
		void PaintNormalChilds();
		void PaintTopMostChilds();
		int AddChild(CWidget *w);
		CWidget* GetFirstChild();
		CWidget* GetNextChild();
		
		int DispatchMouseEvent(EVENT *e);
		int DispatchKeyEvent(int event, int unicode, int buttons, int keycode);
		
		int RedrawIfNeeded();
		
		void NeedRedraw();
		
		int SetPosition (int x, int y);
		int SetSize(int width, int height);
		int Create(int x, int y, int width, int height);
		virtual int CreateOwnSurface();
		int CreateOwnSurface(int flags, int rgbformat);
		ppl6::grafix::CSurface *GetSurface();
		ppl6::grafix::CSurface *GetParentSurface();
		COLORSTYLE	*GetColors();
		int X();
		int Y();
		int Width();
		int Height();
		int ClientX();
		int ClientY();
		int ClientWidth();
		int ClientHeight();
		virtual int Paint();
		virtual void Callback(void *data);
		
		void GetClientRect(RECT *r);
		void GetObjectRect(RECT *r);
		void SetClientRect(int left, int top, int right, int bottom);
		void ToTop(CWidget *widget=NULL);
		void SetKeyFocus();
		void ReleaseKeyFocus();
		int HaveKeyFocus();

		void PrintStatus(int ebene);
		int SendMessageToSiblings(ppl6::CAssocArray *msg);
		int SendMessageToChilds(ppl6::CAssocArray *msg);
		int SendMessage(CWidget *target, ppl6::CAssocArray *msg);
		int SendMessage(unsigned int msg, void *param1, void *param2);
		void SetModal();
		int CheckModal();
};

class Button;
class Label;
class Input;
class Frame;
class CheckBox;
class RadioButton;
class SubWindow;

class CWindow : public CWidget
{
	friend class SubWindow;
	private:
		EVENT lastevent;
		COLORSTYLE	defaultcolors;
		POINT captionend;
		Button *close_button;

	public:
		CInt	Style;
		CBool	Transparent;
		bool	ExternalGraphicThread;
		CWString Caption;
		ppl6::grafix::CFont		Font;
		ppl6::grafix::CImage	IconSmall;
		
		CWindow();
		virtual ~CWindow();
		
		void InitColors();
		virtual int	CreateOwnSurface();
		virtual int Load();
		virtual void Callback(void *data);
		virtual int Paint();
		virtual int MouseClick(CEvent *object, int x, int y, int buttons);
		
};

class SubWindow : public CWindow
{
	private:
		
	public:
		SubWindow();
		virtual ~SubWindow();
		int CreateOwnSurface();
		virtual void Callback(void *data);
		virtual int MouseDown(CEvent *object, int x, int y, int buttons);
		//virtual int Paint();
};


class Frame : public CWidget
{
	public:
		Frame();
		virtual ~Frame();
		CWString Caption;
		CInt	Style;
		ppl6::grafix::CFont	Font;
		CBool	Transparent;
		virtual int Paint();
		virtual void Callback(void *data);
};

class Area : public CWidget
{
	public:
		CBool	ShowBorder;
		ppl6::grafix::COLOR	Color;

		Area();
		virtual ~Area();
		virtual int Paint();
		virtual void Callback(void *data);
};


class Label : public CWidget
{
	public:
		Label();
		virtual ~Label();
		CWString Caption;
		CInt	Style;
		ppl6::grafix::CFont	Font;
		CBool	Transparent;
	virtual int Paint();
	virtual void Callback(void *data);
};
	
	class Button : public CWidget
	{
		private:
			bool pressed;
			
		public:
			Button();
			virtual ~Button();
			virtual void Callback(void *data);
			virtual int Paint();
			virtual int MouseDown(CEvent *object, int x, int y, int buttons);
			virtual int MouseUp(CEvent *object, int x, int y, int buttons);
			virtual int MouseLeave(CEvent *object);
			
			
			ppl6::grafix::CFont	Font;
			CWString Caption;
			CInt	Align;
			ppl6::grafix::CImage	Image;	
	};
	
	class Input : public CWidget
	{
		private:
			ppl6::CCronjob cron;
			int	cursor;
			bool insertmode;
			bool blink;

		public:
			Input();
			virtual ~Input();
			virtual int MouseDown(CEvent *object, int x, int y, int buttons);
			virtual void Callback(void *data);
			virtual int Paint();
			virtual int KeyDown(CEvent *object, int unicode, int buttons, int keycode);
			virtual int GotFocus(CEvent *object);
			virtual int LostFocus(CEvent *object);
			virtual int Message(CEvent *object, ppl6::CAssocArray *msg);

			
			ppl6::grafix::CFont	Font;
			CWString Text;			
			
	};

class RadioButton : public  CWidget
{
	private:
	public:
		RadioButton();
		virtual ~RadioButton();
		CBool	Checked;
		ppl6::grafix::CFont	Font;
		CWString Caption;			

		virtual int Paint();
		virtual int Message(CEvent *object, ppl6::CAssocArray *msg);
		virtual void Callback(void *data);
		virtual int MouseDown(CEvent *object, int x, int y, int buttons);

};

class CheckBox : public  CWidget
{
	private:
	public:
		CheckBox();
		virtual ~CheckBox();
		CBool	Checked;
		ppl6::grafix::CFont	Font;
		CWString Caption;			

		virtual int Paint();
		virtual void Callback(void *data);
		virtual int MouseDown(CEvent *object, int x, int y, int buttons);

};

class Picture : public CWidget
{
	public:
		Picture();
		virtual ~Picture();
		ppl6::grafix::CImage	Image;
		CInt	Style;
		virtual int Paint();
		virtual void Callback(void *data);
};


class TabItem : public CWidget
{
	public:
		ppl6::grafix::CImage	Image;
		ppl6::grafix::CFont	Font;
		CWString Caption;

		TabItem();
		virtual ~TabItem();
		virtual void Callback(void *data);
};

class Tab : public CWidget
{
	private:
		int	activetab;

	public:
		Tab();
		virtual ~Tab();
		CInt	Style;
		virtual void Callback(void *data);
		int AddTab(TabItem *item);

		virtual int Paint();
		virtual int MouseDown(CEvent *object, int x, int y, int buttons);
};

	
}	// EOF namespace tk
}	// EOF namespace ppl6

#endif /*PPL6TK_H_*/
