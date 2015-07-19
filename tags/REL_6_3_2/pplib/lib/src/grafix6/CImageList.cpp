/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.13 $
 * $Date: 2009/12/12 18:53:08 $
 * $Id: CImageList.cpp,v 1.13 2009/12/12 18:53:08 patrick Exp $
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

/*!\class CImageList
 * \ingroup PPLGroupGrafik
 * \brief Ein Container für eine Grafik mit mehreren gleichgroßen Icons
 */

CImageList::CImageList()
{
	gfx=GetGrafix();
	surface=NULL;
	ownsurface=false;
	width=height=num=0;
	drawmethod=DRAWMETHOD::BLT;
	colorkey=0;
	diffuse=0;
}

CImageList::~CImageList()
{
	Clear();
}

void CImageList::Clear()
{
	if (ownsurface) {
		if (surface) delete surface;
		ownsurface=false;
	}
	surface=NULL;
	width=height=num=0;
}

int CImageList::SetDrawMethod(int m)
{
	if (m<DRAWMETHOD::BLT || m>DRAWMETHOD::SPRITE) {
		SetError(1042,"%i",m);
		return 0;
	}
	drawmethod=m;
	return 1;
}

int CImageList::SetColorKey(COLOR c)
{
	colorkey=c;
	return 1;
}

int CImageList::SetColorDiffuse(COLOR c)
{
	diffuse=c;
	return 1;
}

int CImageList::Load(const char *file, int icon_width, int icon_height, int flags, int drawmethod)
{
	if (!file) {
		SetError(194,"int CImageList::Load(==> const char *file <==, int icon_width, int icon_height, int flags, int drawmethod)");
		return 0;
	}
	CFile ff;
	if (!ff.Open((char*)file,"rb")) return 0;
	return Load(&ff,icon_width,icon_height,flags,drawmethod);
}

int CImageList::Load(CFileObject *file, int icon_width, int icon_height, int flags, int drawmethod)
{
	if (!file) {
		SetError(194,"int CImageList::Load(==>CFileObject *file <==, int icon_width, int icon_height, int flags, int drawmethod)");
		return 0;
	}
	if (!gfx) gfx=GetGrafix();
	if (!gfx) return 0;
	Clear();
	surface=gfx->Load(file,flags);
	if (!surface) return 0;
	ownsurface=true;
	width=icon_width;
	height=icon_height;
	if (drawmethod) this->drawmethod=drawmethod;
	iconsX=surface->GetWidth()/width;
	iconsY=surface->GetHeight()/height;
	num=iconsX*iconsY;

	return 1;
}

int CImageList::Load(CSurface *surface, int icon_width, int icon_height, int drawmethod)
{
	if (!surface) {
		SetError(194,"int CImageList::Load(==> CSurface *surface <==, int icon_width, int icon_height, int drawmethod)");
		return 0;
	}
	Clear();
	this->surface=surface;
	if (drawmethod) this->drawmethod=drawmethod;
	width=icon_width;
	height=icon_height;

	iconsX=surface->GetWidth()/width;
	iconsY=surface->GetHeight()/height;
	num=iconsX*iconsY;
	return 1;
}

int CImageList::Load(CResource *res, int id, int icon_width, int icon_height, int flags, int drawmethod)
{
	if (!res) {
		SetError(194,"int CImageList::Load(==> CResource *res <==, int id, int icon_width, int icon_height, int flags, int drawmethod)");
		return 0;
	}
	CFileObject *ff=res->GetFile(id);
	if (!ff) return 0;
	int ret=Load(ff,icon_width,icon_height,flags,drawmethod);
	PushError();
	delete ff;
	PopError();
	return ret;
}

int CImageList::Load(CResource *res, const char *name, int icon_width, int icon_height, int flags, int drawmethod)
{
	if (!res) {
		SetError(194,"int CImageList::Load(==> CResource *res <==, const char *name, int icon_width, int icon_height, int flags, int drawmethod)");
		return 0;
	}
	CFileObject *ff=res->GetFile((char*)name);
	if (!ff) return 0;
	int ret=Load(ff,icon_width,icon_height,flags,drawmethod);
	PushError();
	delete ff;
	PopError();
	return ret;
}


int CImageList::GetRect(RECT *r, int nr)
{
	if (!num) {
		SetError(1043);
		return 0;
	}
	if (!r) {
		SetError(194,"int CImageList::GetRect(RECT *r, int nr)");
	}
	int h,w;
	r->left=r->top=r->right=r->bottom=0;
	if (nr>=num) {
		SetError(1044,"Nummer: %i, Liste: 0-%i",nr,num-1);
		return 0;
	}
	h=nr/iconsX;
	w=nr%iconsX;
	r->left=w*width;
	r->top=h*height;
	r->right=r->left+width;
	r->bottom=r->top+height;
	return 1;
}

CSurface *CImageList::GetSurface()
{
	if (surface) return surface;
	SetError(1045);
	return NULL;
}

int CImageList::Num()
{
	if (num) return num;
	SetError(1045);
	return 0;
}

int CImageList::GetDimension(DIMENSION *d)
{
	if (!d) {
		SetError(194,"int CImageList::GetDimension(DIMENSION *d);");
		return 0;
	}
	d->width=width;
	d->height=height;
	return 1;
}

int CImageList::Draw(int nr, CSurface *target, int x, int y)
{
	if (!target) {
		SetError(194,"int CImageList::Draw(int nr, ==> CSurface *target <==, int x, int y)");
		return 0;
	}
	RECT r;
	if (!GetRect(&r,nr)) return 0;
	switch (drawmethod) {
		case DRAWMETHOD::BLT:
			return target->Blt(surface,&r,x,y);
			break;
		case DRAWMETHOD::ALPHABLT:
			return target->AlphaBlt(surface,&r,x,y);
			break;
		case DRAWMETHOD::COLORKEY:
			return target->BltColorKey(surface,colorkey,&r,x,y);
			break;
		case DRAWMETHOD::DIFFUSE:
			return target->BltDiffuse(surface,diffuse,&r,x,y);
			break;
	}
	SetError(1042,"%i",drawmethod);
	return 0;
}

int CImageList::Draw(int nr, CSurface *target, int x, int y, COLOR diffuse)
{
	if (!target) {
		SetError(194,"int CImageList::Draw(int nr, ==> CSurface *target <==, int x, int y)");
		return 0;
	}
	RECT r;
	if (!GetRect(&r,nr)) return 0;
	switch (drawmethod) {
		case DRAWMETHOD::BLT:
			return target->Blt(surface,&r,x,y);
			break;
		case DRAWMETHOD::ALPHABLT:
			return target->AlphaBlt(surface,&r,x,y);
			break;
		case DRAWMETHOD::COLORKEY:
			return target->BltColorKey(surface,colorkey,&r,x,y);
			break;
		case DRAWMETHOD::DIFFUSE:
			return target->BltDiffuse(surface,diffuse,&r,x,y);
			break;
	}
	SetError(1042,"%i",drawmethod);
	return 0;
}


int CImageList::ManageSurface()
/*!\brief Verwaltung des Surface übernehmen
 *
 * Ein zuvor an CImageList::Load übergebenes Surface wird beim Löschen der
 * ImageListe normalerweise nicht freigegeben, da die Verwaltung bei der
 * Anwendung liegt. Wird jedoch nach dem Load diese Funktion aufgerufen,
 * wird die Verwaltung des Surface an die ImageList abgegeben. Beim Löschen
 * der Liste wird dann auch automatisch das Surface gelöscht.
 */
{
	ownsurface=true;
	return 1;
}

int CSurface::Draw(CImageList *iml, int nr, int x, int y)
{
	if (!iml) {
		SetError(194,"int CSurface::Draw(==> CImageList *iml <==, int nr, int x, int y)");
		return 0;
	}
	return iml->Draw(nr,this,x,y);
}

int CSurface::Draw(CImageList *iml, int nr, int x, int y, COLOR diffuse)
{
	if (!iml) {
		SetError(194,"int CSurface::Draw(==> CImageList *iml <==, int nr, int x, int y)");
		return 0;
	}
	return iml->Draw(nr,this,x,y,diffuse);
}


}
}
