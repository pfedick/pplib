/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.24 $
 * $Date: 2009/06/29 12:16:20 $
 * $Id: CWidget.cpp,v 1.24 2009/06/29 12:16:20 patrick Exp $
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
#include "ppl6.h"
#include "ppl6-grafix.h"
#include "ppl6-tk.h"

using namespace ppl6::grafix;

namespace ppl6 {
namespace tk {

/*!\class CWidget
 * \ingroup PPLGroupToolkit
 * \brief Basisklasse für alle Elemente der Benutzeroberfläche
 */


/*!\var CWidget::client
 * \brief Enthält die Breiten der jeweiligen Ränder
 */

static CWidget *keyfocus=NULL;
static CWidget *mousedown=NULL;
static CWidget *modal=NULL;
static int mousebuttons=0;

CWidget::CWidget()
{
	Name="CWidget";
	CID=0;
	lastevent=NULL;
	parent=NULL;
	gfx=GetGrafix();
	surface=NULL;
	ownsurface=NULL;
	colors=NULL;
	//gfx=NULL;
	needsredraw=true;
	child_needsredraw=false;
	absolute.x=0;
	absolute.y=0;
	w.x=0;
	w.y=0;
	w.width=0;
	w.height=0;
	client.left=0;
	client.top=0;
	client.right=0;
	client.bottom=0;
	Flags=ppl6::grafix::Surface::Flags::Hardware |
			ppl6::grafix::Surface::Flags::Lockable |
			ppl6::grafix::Surface::Flags::VideoMemory;
	RGBFormat=ppl6::grafix::RGBFormat::unknown;


	Enabled=true;
	Visible=true;
	Enabled.SetCallback(this,&Enabled);
	Visible.SetCallback(this,&Visible);
	TopMost.SetCallback(this,&TopMost);
	moveevent=NULL;
	created=false;
}

CWidget::~CWidget()
{
	if (parent) {
		parent->lastevent=NULL;
		parent->moveevent=NULL;
		parent->NeedRedraw();
	}
	// Kinder löschen
	CWidget *child;
	childs.Reset();
	while ((child=(CWidget*)childs.GetNext())) {
		delete child;
		childs.Reset();
	}
	keyfocus=NULL;
	mousedown=NULL;
	mousebuttons=0;
	if (this==modal) modal=NULL;
}

void CWidget::SetModal()
{
	modal=this;
}

int CWidget::CheckModal()
/*!\brief Prüft, ob das Widget zu einem modalen Widget/Fenster gehört
 *
 * \returns Die Funktion liefert true zurück, wenn entweder kein Modales Widget exisitiert, das Widget selbst
 * modal ist oder das Widget einen Modalen Parent hat. Ist das nicht der Fall, wird false zurückgegeben.
 */
{
	if (!modal) return 1;
	if (this==modal) return 1;
	CWidget *p=parent;
	while (p!=NULL) {
		if (p==modal) return 1;
		p=p->parent;
	}
	return 0;
}

void CWidget::PrintStatus(int ebene)
{
	if (!ebene) PrintDebug("CWidget::PrintStatus %llu\n",ppl6::GetTime());
	CString s;
	s.Repeat(32,ebene*3);
	s.Concatf("%s: Visible: %i, needsredraw: %i, child_needsredraw: %i",
		(const char*)Name,(int)Visible, (int)needsredraw, (int)child_needsredraw);
	PrintDebug(s);
	CWidget *c;
	childs.Reset();
	while ((c=(CWidget*)childs.GetNext())) c->PrintStatus(ebene+1);
}

void CWidget::PaintChilds()
{
	//printf ("CWidget::PaintChilds, Name=%s, Visible=%i\n",(char*)Name,(int)Visible);
	CWidget *child;
	bool top=false;
	while (1) {
		childs.Reset();
		while ((child=(CWidget*)childs.GetNext())) {
			if (child->TopMost==top) child->Redraw();
		}
		if (top==true) break;
		top=true;
	}
}

void CWidget::PaintNormalChilds()
{
	CWidget *child;
	childs.Reset();
	while ((child=(CWidget*)childs.GetNext())) {
		if (child->TopMost==(const bool)false) child->Redraw();
	}
}

void CWidget::PaintTopMostChilds()
{
	CWidget *child;
	childs.Reset();
	while ((child=(CWidget*)childs.GetNext())) {
		if (child->TopMost==(const bool)true) child->Redraw();
	}
}

void CWidget::NeedRedraw()
{
	if (!needsredraw) {
		needsredraw=true;
		if (parent) parent->ClientNeedsRedraw();
	}
}

void CWidget::ClientNeedsRedraw()
{
	if (!child_needsredraw) {
		//printf ("CWidget::ClientNeedsRedraw %s\n",(char*)Name);
		child_needsredraw=true;
		if (parent) parent->ClientNeedsRedraw();
	}
}

int CWidget::Paint()
{
	return 1;
}


int CWidget::RedrawIfNeeded()
{
	//printf ("CWidget::RedrawIfNeeded, Name=%s, Visible=%i\n",(char*)Name,(int)Visible);
	//PrintStatus(10);
	if ((bool)Visible!=true) return 1;
	RECT r;
	if (!surface) surface=GetSurface();
	if (!surface) return 0;

	//printf("Redraw 1\n");
	if (needsredraw) {
		//printf("Redraw 2\n");
		GetObjectRect(&r);
		surface->SetClipper(&r);
		surface->SetOrigin(r.left,r.top);
		Paint();
		surface->SetOrigin(0,0);
		surface->UnsetClipper();
		// Alle Clients müssen ebenfalls neugezeichnet werden
		PaintChilds();
		needsredraw=false;
		child_needsredraw=false;
		return 1;
	} else if (child_needsredraw) {
		//printf("Redraw 3, childs: %i\n",childs.Num());
		childs.Reset();
		CWidget *child;
		while ((child=(CWidget*)childs.GetNext())) {
			child->RedrawIfNeeded();
		}
		child_needsredraw=false;
		// Eigenes Surface?
		if (ownsurface!=NULL && parent!=NULL) {
			ppl6::grafix::CSurface *target=parent->GetSurface();
			target->Blt(ownsurface,NULL,w.x,w.y);
		}
		return 1;
	}
	return 0;
}

int CWidget::Redraw()
{
	//printf ("CWidget::Redraw, Name=%s, Visible=%i\n",(char*)Name,(int)Visible);
	if (!(bool)Visible) return 1;
	RECT r;
	// Haben wir ein eigenes Surface?
	if (ownsurface) {
		if (needsredraw==false && child_needsredraw==false) {
			// Hier ist nichts zu tun, ausser das Surface über den Parent zu blitten
			if (!parent) return 1;
			//GetObjectRect(&r);
			ppl6::grafix::CSurface *target=parent->GetSurface();
			target->Blt(ownsurface,NULL,w.x,w.y);

			return 1;
		}
	}
	if (!surface) surface=GetSurface();
	if (!surface) return 0;
	GetObjectRect(&r);
	// Falls wir ein eigenes Surface haben
	surface->SetClipper(&r);
	surface->SetOrigin(r.left,r.top);
	Paint();
	surface->SetOrigin(0,0);
	surface->UnsetClipper();
	// Alle Clients müssen ebenfalls neugezeichnet werden
	PaintChilds();
	needsredraw=false;
	child_needsredraw=false;
	return 1;
}

CSurface *CWidget::GetSurface()
{
	if (!surface) {
		if (ownsurface) {
			surface=ownsurface;
			return surface;
		}
		if (parent) surface=parent->GetSurface();
		if (!surface) {
			SetError(1041);
			return NULL;
		}
	}
	return surface;
}

CSurface *CWidget::GetParentSurface()
{
	CSurface *s;
	if (parent) s=parent->GetSurface();
	if (!s) {
		SetError(1041);
		return NULL;
	}
	if (!ownsurface) surface=s;
	return s;
}

COLORSTYLE *CWidget::GetColors()
{
	if (!colors) {
		if (parent) colors=parent->GetColors();
		else {
			SetError(1041);
			return NULL;
		}
	}
	return colors;
}


int CWidget::SetPosition(int x, int y)
{
	w.x=x;
	w.y=y;
	if (parent) parent->NeedRedraw();
	return 1;
}

int CWidget::SetSize(int width, int height)
{
	w.width=width;
	w.height=height;
	if (parent) parent->NeedRedraw();
	return 1;
}

int CWidget::Create(int x, int y, int width, int height)
{
	w.x=x;
	w.y=y;
	w.width=width;
	w.height=height;
	if (parent) parent->NeedRedraw();
	return 1;
}

int CWidget::CreateOwnSurface(int flags, int rgbformat)
{
	Flags=flags;
	RGBFormat=rgbformat;
	return CreateOwnSurface();
}

int CWidget::CreateOwnSurface()
{
	CGrafix *gfx=GetGrafix();
	if (!gfx) return 0;
	if (created==true || ownsurface!=NULL) {
		SetError(1061,"CWidget::CreateOwnSurface");
		return 0;
	}
	ownsurface=gfx->CreateSurface((const char*)Name,w.width, w.height, RGBFormat, Flags);
	if (!ownsurface) {
		return 0;
	}
	surface=ownsurface;
	surface->CLS();
	created=true;
	return 1;
}


int CWidget::AddChild(CWidget *child)
{
	int ret;
	child->parent=this;
	ret=childs.Add(child);
	return ret;
}

CWidget *CWidget::GetFirstChild()
{
	return (CWidget*)childs.GetFirst();
}

CWidget *CWidget::GetNextChild()
{
	return (CWidget*)childs.GetNext();
}

int CWidget::X()
{
	return w.x;
}

int CWidget::Y()
{
	return w.y;
}

int CWidget::Width()
{
	return w.width;
}

int CWidget::Height()
{
	return w.height;
}

int CWidget::ClientWidth()
{
	RECT r;
	GetClientRect(&r);
	return r.right-r.left+1;
}

int CWidget::ClientHeight()
{
	RECT r;
	GetClientRect(&r);
	return r.bottom-r.top+1;
}

int CWidget::ClientX()
{
	return w.x+client.left;
}

int CWidget::ClientY()
{
	return w.y+client.top;
}

void CWidget::GetClientRect(RECT *r)
{
	if (ownsurface==NULL) {
		if (parent) {
			parent->GetClientRect(r);
			r->left+=w.x+client.left;
			r->top+=w.y+client.top;
			if (r->left+w.width<r->right)r->right=r->left+w.width-1;
			if (r->top+w.height<r->bottom)r->bottom=r->top+w.height-1;
			r->right-=client.right;
			r->bottom-=client.bottom;
			return;
		}
	}
	r->left=client.left;
	r->top=client.top;
	r->right=w.width-1-client.right;
	r->bottom=w.height-1-client.bottom;
}

void CWidget::GetObjectRect(RECT *r)
{
	if (!ownsurface) {
		if (parent) {
			parent->GetClientRect(r);
			r->left+=w.x;
			r->top+=w.y;
			SURFACE *s=surface->GetSurfaceParams();
			if (r->left+w.width<r->right)r->right=r->left+w.width-1;
			if (r->top+w.height<r->bottom)r->bottom=r->top+w.height-1;
			if (r->bottom>=s->height) r->bottom=s->height-1;
			if (r->right>=s->width) r->right=s->width-1;
			return;
		}
	}
	r->left=0;
	r->top=0;
	r->right=w.width-1;
	r->bottom=w.height-1;
}

void CWidget::SetClientRect(int left, int top, int right, int bottom)
{
	client.left=left;
	client.top=top;
	client.right=right;
	client.bottom=bottom;
}

void CWidget::Callback(void *data)
{
	if (data==&Enabled) NeedRedraw();
	if (data==&Visible) {
		NeedRedraw();
		if ((!(bool)Visible) && parent!=NULL) parent->NeedRedraw();
	}

}


void CWidget::ToTop(CWidget *widget)
{
	if (widget) {
		if (childs.Delete(widget)) childs.Add(widget);
	} else if (parent) {
		parent->ToTop(this);
	}
}


int CWidget::DispatchMouseEvent(EVENT *e)
{
	//printf ("DispatchMouseEvent %s: e.x=%i, e.y=%i\n",(char*)Name,e->x,e->y);
	EVENT ee;
	CWidget *child=NULL;
	if (!Enabled) return 0;
	if (moveevent) {
		if (e->buttons==movebuttons && moveevent->surface!=NULL) {
			POINT p;
			if (moveevent->surface->GetMousePosition(&p)) {
				int diffx, diffy;
				diffx=p.x-lastmovepoint.x;
				diffy=p.y-lastmovepoint.y;

				if (diffx!=0 || diffy!=0) {
					moveevent->w.x+=diffx;
					moveevent->w.y+=diffy;
					NeedRedraw();
					lastmovepoint.x=p.x;
					lastmovepoint.y=p.y;
					return 1;
				}
			}
		} else {
			moveevent=NULL;
			return 1;
		}
	}

	bool topmost=true;
	int x,y;
	// Gibt es Childs?
	if (childs.Num()) {
		while (1) {
			childs.Reset();
			while ((child=(CWidget*)childs.GetPrevious()) && child->TopMost==topmost) {
				//printf("%s: w.x=%i, w.y=%i, client.left=%i, client.top=%i, e.x=%i, e.y=%i\n",(char*)child->Name,
				//	child->w.x, child->w.y,child->client.left, child->client.top,e->x, e->y);
				x=e->x-client.left;
				y=e->y-client.top;
				if (child->w.x<=x &&
					child->w.y<=y &&
					child->w.x+child->w.width>x &&
					child->w.y+child->w.height > y && child->Enabled==(bool)true) {
					memcpy(&ee,e,sizeof(EVENT));
					ee.x=x-child->w.x;
					ee.y=y-child->w.y;
					if (lastevent!=child) {
						if (lastevent) lastevent->MouseLeave(lastevent);
						child->MouseEnter(child);
					}
					lastevent=child;
					return child->DispatchMouseEvent(&ee);
				}
			}
			if (topmost==false) break;
			topmost=false;
		}
	}
	if (lastevent!=this) {
		if (lastevent) lastevent->MouseLeave(lastevent);
		this->MouseEnter(child);
	}
	lastevent=this;
	if (!CheckModal()) return 1;
	switch (e->type) {
		case EVENTTYPE::MOUSEDOWN:
			mousedown=this;
			mousebuttons=e->buttons;
			return MouseDown(this,e->x, e->y, e->buttons);
		case EVENTTYPE::MOUSEUP:
			MouseUp(this,e->x, e->y, e->buttons);
			if (mousedown==this) MouseClick(this,e->x,e->y, mousebuttons);
			mousedown=NULL;
			mousebuttons=0;
			return 1;
		case EVENTTYPE::MOUSEMOTION:
			return MouseMove(this,e->x, e->y, e->buttons);
	}
	return 1;
}

void CWidget::SetKeyFocus()
{
	if (keyfocus!=this) {
		if (keyfocus) keyfocus->LostFocus(keyfocus);
		this->GotFocus(this);
	}
	keyfocus=this;
}

void CWidget::ReleaseKeyFocus()
{
	if (keyfocus) keyfocus->LostFocus(keyfocus);
	keyfocus=NULL;
}

int CWidget::HaveKeyFocus()
{
	if (keyfocus==this) return 1;
	return 0;
}


int CWidget::DispatchKeyEvent(int event, int unicode, int buttons, int keycode)
{
	if (keyfocus) {
		if (event==EVENTTYPE::KEYDOWN) return keyfocus->KeyDown(keyfocus,unicode,buttons, keycode);
		if (event==EVENTTYPE::KEYUP) return keyfocus->KeyUp(keyfocus,unicode,buttons, keycode);
	}
	return 0;
}

int CWidget::SendMessageToSiblings(ppl6::CAssocArray *msg)
{
	if (!parent) return 0;
	return parent->SendMessageToChilds(msg);
}

int CWidget::SendMessageToChilds(ppl6::CAssocArray *msg)
{
	childs.Reset();
	CWidget *child;
	while ((child=(CWidget*)childs.GetNext())) {
		child->Message(child,msg);
	}
	return 1;
}

int CWidget::SendMessage(CWidget *target, ppl6::CAssocArray *msg)
{
	return surface->SendMessage(target,msg);
}

int CWidget::SendMessage(unsigned int msg, void *param1, void *param2)
{
	return surface->SendMessage(msg,param1,param2);
}

void CWidget::StartMove(CWidget *w, int buttons)
{
	moveevent=NULL;
	if (!surface) return;
	if (!surface->GetMousePosition(&lastmovepoint)) return;
	moveevent=w;
	movebuttons=buttons;
}


}
}
