/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.24 $
 * $Date: 2009/02/08 09:55:35 $
 * $Id: CSurface.cpp,v 1.24 2009/02/08 09:55:35 patrick Exp $
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


namespace ppl6 {
namespace grafix {

/*!\class CSurface
 * \brief Klasse für Zeichenflächen innerhalb der Hardware oder des Hauptspeichers
 * \ingroup PPLGroupGrafik
 */


CSurface::CSurface()
{
	// Wir initialisieren die SURFACE-Struktur mit NULL
	memset(&s,0,sizeof(SURFACE));
	s.Surface=this;
	engine=NULL;
}

CSurface::~CSurface()
{
	if (s.DestroySurface) s.DestroySurface(&s);
	if (!engine) {		// Software-Surface
		if (s.base) free(s.base);
	}
	if (s.EngineSpecific) free(s.EngineSpecific);
}

SURFACE *CSurface::GetSurfaceParams()
{
	return &s;
}

const char *CSurface::GetName()
{
	return (const char*)Name;
}

CGrafix *CSurface::GetGrafix()
{
	return gfx;
}

int CSurface::SetCaption(const char *title)
{
	Caption=title;
	if (s.SetCaption) return s.SetCaption(&s,title);
	return 1;
}

const char *CSurface::GetCaption()
{
	return (const char*)Caption;
}

int CSurface::Flip()
{
	if (s.Flip) return s.Flip(&s);
	return 1;
}

unsigned char *CSurface::Lock()
{
	if (s.Lock) {
		if (s.Lock(&s)) return s.base8;
		return NULL;
	}
	s.lockcount++;
	return s.base8;
}

int CSurface::Unlock()
{
	if (s.Unlock) return s.Unlock(&s);
	if (s.lockcount>0) s.lockcount--;
	return 1;
}

int CSurface::IsLocked()
{
	return s.lockcount;
}

int CSurface::CLS(COLOR c)
{
	s.lastcolor=c;
	if (s.CLS) return s.CLS(&s,c);
	SetError(1012,"CLS");
	return 0;
}

int CSurface::BeginScene()
{
	if (s.BeginScene) return s.BeginScene(&s);
	return 1;
}

int CSurface::EndScene()
{
	if (s.EndScene) return s.EndScene(&s);
	return 1;
}


int CSurface::FitRect (RECT * r)
/*!\brief Passt das Rechteck in das aktuelle Clipping?
 *
 * \desc
 * Diese Funktion prueft, ob die Koordinaten in rect in das
 * Clipping des Surface passen und passt sie ggfs. an.
 * Ausserdem werden die Koordinaten so gedreht, dass die
 * niedrigere in x1, bzw. y2 gespeichert wird
 */
{
	//DLOG ("%u->CSurface::FitRect(RECT=%u)",this,r);
	if (r==NULL) {
		SetError(112);
		return 0;
	}
	//DLOG ("  rect=(%u/%u)-(%u/%u)",r->left,r->top,r->right,r->bottom);

	ppldd _t;
	if (r->right<r->left) {
		_t=r->right;
		r->right=r->left;
		r->left=_t;
	}
	if (r->bottom<r->top) {
		_t=r->bottom;
		r->bottom=r->top;
		r->top=_t;
	}
	if (r->left >= s.clipper.right || r->top >= s.clipper.bottom ||
		r->right < s.clipper.left || r->bottom <s.clipper.top) {
		SetError(107);
		return 0;
	}
	if (r->left<s.clipper.left) r->left=s.clipper.left;
	if (r->top<s.clipper.top) r->top=s.clipper.top;
	if (r->right>s.clipper.right) r->right=s.clipper.right;
	if (r->bottom>s.clipper.bottom) r->bottom=s.clipper.bottom;
	return 1;
}


int CSurface::GetWidth()
{
	return s.width;
}

int CSurface::GetHeight()
{
	return s.height;
}

int CSurface::GetBitdepth()
{
	return s.bitdepth;
}
int CSurface::GetRGBFormat()
{
	return s.rgbformat;
}

void CSurface::SetOrigin(int x, int y)
{
	s.originx=x;
	s.originy=y;
}

void CSurface::SetClipper(int x1, int y1, int x2, int y2)
{
	s.clipper.left=x1;
	s.clipper.top=y1;
	s.clipper.right=x2;
	s.clipper.bottom=y2;
	if (s.clipper.left<0) s.clipper.left=0;
	if (s.clipper.top<0) s.clipper.top=0;
	if (s.clipper.right>s.width) s.clipper.right=s.width;
	if (s.clipper.bottom>s.height) s.clipper.bottom=s.height;
}

void CSurface::SetClipper(RECT *r)
{
	if (!r) return;
	s.clipper.left=r->left;
	s.clipper.top=r->top;
	s.clipper.right=r->right;
	s.clipper.bottom=r->bottom;
	if (s.clipper.left<0) s.clipper.left=0;
	if (s.clipper.top<0) s.clipper.top=0;
	if (s.clipper.right>s.width) s.clipper.right=s.width;
	if (s.clipper.bottom>s.height) s.clipper.bottom=s.height;
}

void CSurface::SetClipper(POINT *p1, POINT *p2)
{
	if (!p1) return;
	if (!p2) return;
	s.clipper.left=p1->x;
	s.clipper.top=p1->y;
	s.clipper.right=p2->x;
	s.clipper.bottom=p2->y;
	if (s.clipper.left<0) s.clipper.left=0;
	if (s.clipper.top<0) s.clipper.top=0;
	if (s.clipper.right>s.width) s.clipper.right=s.width;
	if (s.clipper.bottom>s.height) s.clipper.bottom=s.height;
}

void CSurface::GetClipper(RECT *r)
{
	if (!r) return;
	r->left=s.clipper.left;
	r->top=s.clipper.top;
	r->right=s.clipper.right;
	r->bottom=s.clipper.bottom;
}

void CSurface::UnsetClipper()
{
	s.clipper.left=0;
	s.clipper.top=0;
	s.clipper.right=s.width;
	s.clipper.bottom=s.height;
}

int CSurface::CheckClipping(int x, int y)
{
	x+=s.originx;
	y+=s.originy;
	if (x<s.clipper.left || x>=s.clipper.right || y<s.clipper.top || y>=s.clipper.right) return 0;
	return 1;
}

int CSurface::CheckClipping(POINT *p)
{
	if (!p) return 0;
	return CheckClipping(p->x,p->y);
	//if (p->x<s.clipper.left || p->x>=s.clipper.right || p->y<s.clipper.top || p->y>=s.clipper.right) return 0;
	//return 1;
}

int CSurface::CheckClipping (int x, int y, RECT * r)
{
	if (!r) return 0;
	if (x<r->left || x>=r->right || y<r->top || y>=r->right) return 0;
	return 1;

}

int CSurface::CheckClipping (POINT *p, RECT * r)
{
	if (!p) return 0;
	if (!r) return 0;
	if (p->x<r->left || p->x>=r->right || p->y<r->top || p->y>=r->right) return 0;
	return 1;
}

int CSurface::MoveCursor(int x, int y)
{
	s.lastx=x;
	s.lasty=y;
	return 1;
}

int CSurface::GetMousePosition(POINT *p)
{
	if (!engine) {
		return 0;
	}
	return engine->GetMousePosition(this,p);
}

int CSurface::SendMessage(void *object, CAssocArray *msg)
/*!\brief Nachricht schicken
 * \param[in] object Ein Pointer auf ein beliebiges Objekt. In der Regel zeigt dieses
 * auf ein von ppl6::tk::CWidget abgeleitetes Objekt. Der Parameter ist optional.
 * \param[in] msg Ein optionaler Pointer auf ein Assoziatives Array. Dieses darf nicht
 * statisch deklariert worden sein, sondern mit "new". Es wird nach Verarbeitung
 * der Nachricht automatisch gelöscht.
 */
{
	if (!engine) {
		return 0;
	}
	return engine->SendEvent(this,object,msg);
}

int CSurface::SendMessage(unsigned int msg, void *param1, void *param2)
/*!\brief Nachricht schicken
 *
 * Mit dieser Funktion kann eine Systemspezifische Nachricht an das Fenster geschickt werden.
 * Unter Windows ruft die Funktion "PostMessage" auf, übergibt \p msg als Nachrichtennummer,
 * \p param1 als WPARAM und \p param2 als LPARAM.
 *
 * \param[in] msg Systemspezifische Nachrichtennummer
 * \param[in] param1 Pointer auf optionalen Parameter
 * \param[in] param2 Pointer auf zweiten optionalen Parameter
 */
{
	if (s.PostMessage) return s.PostMessage(&s,msg,param1,param2);
	return 0;
}


} // EOF namespace grafix
} // EOF namespace ppl6

