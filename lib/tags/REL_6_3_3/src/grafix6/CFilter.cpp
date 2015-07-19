/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.14 $
 * $Date: 2009/12/12 18:53:08 $
 * $Id: CFilter.cpp,v 1.14 2009/12/12 18:53:08 patrick Exp $
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

/*!\class CFilter
 * \ingroup PPLGroupGrafik
 * \brief Basisklasse für Import-/Export-Filter verschiedener Grafikformate
 */


int CGrafix::AddFilter(CFilter *filter)
{
	int ret;
	Mutex.Lock();
	ret=Filter.Add(filter);
	Mutex.Unlock();
	if (!ret) {
		ExtendError(1003);
	}
	return ret;
}

int CGrafix::DeleteFilter(CFilter *filter)
{
	int ret;
	Mutex.Lock();
	ret=Filter.Delete(filter);
	Mutex.Unlock();
	if (!ret) {
		ExtendError(1004);
	}
	return ret;
}

CFilter *CGrafix::FindFilter(const char *name)
{
	CFilter *f;
	Mutex.Lock();
	// Wir gehen die Liste rückwärts durch
	Filter.Reset();
	while ((f=(CFilter*)Filter.GetPrevious())) {
		if (strcasecmp((char*)name,(char*)f->GetName())==0) {
			Mutex.Unlock();
			return f;
		}
	}
	Mutex.Unlock();
	SetError(1006,(char*)name);
	return NULL;
}


CSurface *CGrafix::Load(CResource *res, int id, int flags, int rgbformat, CFilter *filter)
{
	if (!res) {
		SetError(194);
		return NULL;
	}
	CFileObject *ff;
	ff=res->GetFile(id);
	if (!ff) return NULL;
	CSurface *s=Load(ff,flags,rgbformat,filter);
	PushError();
	delete ff;
	PopError();
	return s;
}

CSurface *CGrafix::Load(CResource *res, const char *name, int flags, int rgbformat, CFilter *filter)
{
	if (!res) {
		SetError(194);
		return NULL;
	}
	if (!name) {
		SetError(194);
		return NULL;
	}
	CFileObject *ff;
	ff=res->GetFile((char*)name);
	if (!ff) return NULL;
	CSurface *s=Load(ff,flags,rgbformat,filter);
	PushError();
	delete ff;
	PopError();
	return s;
}

CSurface *CGrafix::Load(const char *filename, int flags, int rgbformat, CFilter *filter)
{
	CFile ff;
	if (!ff.Open(filename,"rb")) return NULL;
	return Load(&ff,flags,rgbformat, filter);
}

CSurface *CGrafix::Load(CFileObject *ff, int flags, int rgbformat, CFilter *filter)
{
	IMAGE img;
	if (!filter) {
		Mutex.Lock();
		// Wir gehen die Liste rückwärts durch
		Filter.Reset();
		while ((filter=(CFilter*)Filter.GetPrevious())) {
			if (filter->Ident(ff,&img)==1) break;
		}
		Mutex.Unlock();
		if (!filter) {
			SetError(1017);
			return NULL;
		}
	} else {
		if (!filter->Ident(ff,&img)) return 0;
	}
	if ((flags&Surface::Flags::ConvertColors) && rgbformat==0) rgbformat=PrimaryRGBFormat;
	if (!rgbformat) rgbformat=img.format;
	CSurface *surface=CreateSurface("img",img.width,img.height,rgbformat,flags);
	if (!surface) return NULL;
	if (!filter->Load(ff,surface,&img)) {
		PushError();
		delete surface;
		PopError();
		return NULL;
	}
	return surface;
}



/*!\class CFilter
 * \brief Basisklasse für Grafikformat-Filter
 */

CFilter::CFilter()
{
}

CFilter::~CFilter()
{
}


int CFilter::Ident(CFileObject *file, IMAGE *img)
{
	SetError(1005);
	return 0;
}

int CFilter::Load(CFileObject *file, CSurface *surface, IMAGE *img)
{
	SetError(1005);
	return 0;
}

int CFilter::Save (CSurface *surface, CFileObject * file, RECT *area, CAssocArray *param)
{
	SetError(1005);
	return 0;
}

int CFilter::SaveFile (CSurface *surface, const char * filename, RECT *area, CAssocArray *param)
{
	CFile ff;
	if (!ff.Open(filename,"wb")) return 0;
	return Save(surface, &ff,area,param);
}

const char *CFilter::GetName()
{
	return "unknown";
}

const char *CFilter::GetDescription()
{
	return "Basisklasse für Grafikfilter";
}



} // EOF namespace grafix
} // EOF namespace ppl6

