/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.12 $
 * $Date: 2009/12/12 18:53:08 $
 * $Id: CImage.cpp,v 1.12 2009/12/12 18:53:08 patrick Exp $
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

/*!\class CImage
 * \ingroup PPLGroupGrafik
 * \brief Kontainerklasse für Surfaces, Teilsurfaces und ImageLists
 */


CImage::CImage()
{
	gfx=GetGrafix();
	surface=NULL;
	iml=NULL;
	ownsurface=false;
	r.left=r.right=r.top=r.bottom=0;
	drawmethod=DRAWMETHOD::BLT;
	colorkey=0;
	diffuse=0;
}

CImage::~CImage()
{
	Clear();
}

void CImage::Clear()
{
	if (ownsurface) {
		if (surface) delete surface;
		ownsurface=false;
	}
	surface=NULL;
	iml=NULL;
	r.left=r.right=r.top=r.bottom=0;
	Change();
}

int CImage::SetDrawMethod(int m)
{
	if (m<DRAWMETHOD::BLT || m>DRAWMETHOD::SPRITE) {
		SetError(1042,"%i",m);
		return 0;
	}
	drawmethod=m;
	return 1;
}

int CImage::SetColorKey(COLOR c)
{
	colorkey=c;
	return 1;
}

int CImage::SetColorDiffuse(COLOR c)
{
	diffuse=c;
	return 1;
}

int CImage::Load(const char *file, int flags, int drawmethod)
{
	if (!file) {
		SetError(194,"int CImage::Load(==> const char *file <==, int flags, int drawmethod)");
		return 0;
	}
	CFile ff;
	if (!ff.Open((char*)file,"rb")) return 0;
	return Load(&ff,flags,drawmethod);
}

int CImage::Load(CFileObject *file, int flags, int drawmethod)
{
	if (!file) {
		SetError(194,"int CImage::Load(==> CFileObject *file <==, int flags, int drawmethod)");
		return 0;
	}
	if (!gfx) gfx=GetGrafix();
	if (!gfx) return 0;
	Clear();
	surface=gfx->Load(file,flags);
	if (!surface) return 0;
	r.left=0;
	r.right=surface->GetWidth()-1;
	r.top=0;
	r.bottom=surface->GetHeight()-1;
	ownsurface=true;
	if (drawmethod) this->drawmethod=drawmethod;
	Change();
	return 1;
}

int CImage::Set(CSurface *surface, RECT *r, int drawmethod)
{
	if (!surface) {
		SetError(194,"int CImage::Set(CSurface *surface, RECT *r, int drawmethod)");
		return 0;
	}
	Clear();
	this->surface=surface;
	if (drawmethod) this->drawmethod=drawmethod;
	if (r) {
		CopyRect(&this->r,r);
	} else {
		this->r.left=0;
		this->r.right=surface->GetWidth()-1;
		this->r.top=0;
		this->r.bottom=surface->GetHeight()-1;
	}
	Change();
	return 1;
}

int CImage::Load(CResource *res, int id, int flags, int drawmethod)
{
	if (!res) {
		SetError(194,"int CImage::Load(==> CResource *res <==, int id, int flags, int drawmethod)");
		return 0;
	}
	CFileObject *ff=res->GetFile(id);
	if (!ff) return 0;
	int ret=Load(ff,flags,drawmethod);
	PushError();
	delete ff;
	PopError();
	return ret;
}

int CImage::Load(CResource *res, const char *name, int flags, int drawmethod)
{
	if (!res) {
		SetError(194,"int CImage::Load(==> CResource *res <==, const char *name, int flags, int drawmethod)");
		return 0;
	}
	CFileObject *ff=res->GetFile((char*)name);
	if (!ff) return 0;
	int ret=Load(ff,flags,drawmethod);
	PushError();
	delete ff;
	PopError();
	return ret;
}

int CImage::Set(CImageList *iml, int nr)
{
	if (!iml) {
		SetError(194,"int CImage::Set(CImageList *iml, int nr)");
		return 0;
	}
	Clear();
	this->iml=iml;
	this->nr=nr;
	Change();
	return 1;
}


int CImage::GetRect(RECT *r)
{
	if (!r) {
		SetError(194,"int CImage::GetRect(RECT *r)");
	}
	CopyRect(r,&this->r);
	return 1;
}

CSurface *CImage::GetSurface()
{
	if (surface) return surface;
	SetError(1045);
	return NULL;
}

int CImage::GetDimension(DIMENSION *d)
{
	if (!d) {
		SetError(194,"int CImage::GetDimension(DIMENSION *d);");
		return 0;
	}
	if (surface) {
		d->width=r.right-r.left+1;
		d->height=r.bottom-r.top+1;
		return 1;
	} else if (iml) {
		return iml->GetDimension(d);
	}
	SetError(1046);
	return 0;
}

int CImage::GetWidth()
{
	if (surface) {
		return r.right-r.left+1;
	} else if (iml) {
		DIMENSION d;
		if (!iml->GetDimension(&d)) return 0;
		return d.width;
	}
	SetError(1046);
	return 0;
}

int CImage::GetHeight()
{
	if (surface) {
		return r.bottom-r.top+1;
	} else if (iml) {
		DIMENSION d;
		if (!iml->GetDimension(&d)) return 0;
		return d.height;
	}
	SetError(1046);
	return 0;
}

int CImage::IsLoaded()
{
	if (surface) return 1;
	if (iml) return 1;
	return 0;
}

int CImage::Draw(CSurface *target, int x, int y)
{
	if (!target) {
		SetError(194,"int CImage::Draw(==> CSurface *target <==, int x, int y)");
		return 0;
	}
	if (surface) {
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
	} else if (iml) {
		return iml->Draw(nr,target,x,y);
	}
	SetError(1046);
	return 0;
}


int CImage::ManageSurface()
/*!\brief Verwaltung des Surface übernehmen
 *
 * Ein zuvor an CImage::Set übergebenes Surface wird beim Löschen der
 * der CImage-Klasse normalerweise nicht freigegeben, da die Verwaltung bei der
 * Anwendung liegt. Wird jedoch nach dem Load diese Funktion aufgerufen,
 * wird die Verwaltung des Surface an die CImage-Klasse abgegeben. Beim Löschen
 * der Klasse wird dann auch automatisch das Surface gelöscht.
 */
{
	ownsurface=true;
	return 1;
}

int CSurface::Draw(CImage *img, int x, int y)
{
	if (!img) {
		SetError(194,"int CSurface::Draw(==> CImage *img <==, int x, int y)");
		return 0;
	}
	return img->Draw(this,x,y);
}


}
}

