/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.2 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: CSurface.cpp,v 1.2 2010/02/12 19:43:48 pafe Exp $
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
//#ifdef HAVE_MATH_H
#include <math.h>
//#endif
#include "ppl6.h"
#include "ppl6-grafix.h"

namespace ppl6 {
namespace grafix {

CSurface::CSurface()
{
	engineprivate=NULL;
	gfxengine=NULL;
	lockcount=0;
	flags=DefaultFlags;
}

CSurface::CSurface(const CSurface &other)
{
	engineprivate=NULL;
	gfxengine=NULL;
	lockcount=0;
	flags=DefaultFlags;

	if (!copy(other)) throw Exception::CopyFailed();
}

CSurface::CSurface(int width, int height, const RGBFormat &format, SurfaceFlags flags)
{
	engineprivate=NULL;
	gfxengine=NULL;
	lockcount=0;
	s.setSize(width,height);
	this->flags=flags;
	this->format=format;
	if (!create()) throw Exception::FunctionFailed();
}

CSurface::~CSurface()
{
	if (!engineprivate) return;
	if (gfxengine) {
		gfxengine->destroySurface(*this);
	}
}

int CSurface::copy(const CSurface &other)
{
	if (engineprivate) {
		// Bisheriges Surface löschen
		clear();
	}
	// Neues Surface mit gleichen Parametern wie other erstellen
	s=other.s;
	format=other.format;
	flags=other.flags;
	if (!create()) return 0;
	// Beide Surfaces locken
	bool selflocked=false;
	if (other.gfxengine) {
		CDrawable olddraw,newdraw;
		if (other.lockcount>0) {
			olddraw=other.draw;
		} else {
			// Wir müssen die lock-Funktion der Klasse umgehen, da wir sonst
			// das zu kopierende Surface nicht als const deklarieren können
			if (!other.gfxengine->lockSurface((CSurface&)other,olddraw)) return 0;
			selflocked=true;
		}
		if (!lock(newdraw)) {
			if (selflocked) {
				PushError();
				other.gfxengine->unlockSurface((CSurface&)other);
				PopError();
			}
			return 0;
		}
		// Grafik kopieren
		newdraw.blt(olddraw);

		// Beide Surfaces entsperren
		if (selflocked) {
			other.gfxengine->unlockSurface((CSurface&)other);
		}
		unlock();
	}
	return 1;
}

void CSurface::clear()
{
	if (!engineprivate) return;
	if (gfxengine) {
		if (lockcount>0) {
			gfxengine->unlockSurface(*this);
		}
		gfxengine->destroySurface(*this);
	}

	engineprivate=NULL;
	gfxengine=NULL;
	lockcount=0;
	flags=DefaultFlags;
}

Size CSurface::size() const
{
	return s;
}

int CSurface::width() const
{
	return s.width();
}

int CSurface::height() const
{
	return s.height();
}

void CSurface::setWidth(int width)
{
	if (!engineprivate)	s.setWidth(width);
}

void CSurface::setHeight(int height)
{
	if (!engineprivate) s.setHeight(height);
}

void CSurface::setSize(int width, int height)
{
	if (!engineprivate) s.setSize(width,height);
}

void CSurface::setSize(const Size &s)
{
	if (!engineprivate) this->s=s;
}

CSurface::SurfaceFlags CSurface::surfaceFlags() const
{
	return flags;
}

void CSurface::setSurfaceFlags(SurfaceFlags f)
{
	if (!engineprivate) flags=f;
}

RGBFormat CSurface::rgbFormat() const
{
	return format;
}

void CSurface::setRGBFormat(const RGBFormat &format)
{
	if (!engineprivate) this->format=format;
}

void *CSurface::enginePrivate() const
{
	return engineprivate;
}

void CSurface::setEnginePrivate(void *ptr)
{
	engineprivate=ptr;
}

GFXEngine *CSurface::gfxEngine() const
{
	return gfxengine;
}

void CSurface::setGfxEngine(GFXEngine *engine)
{
	gfxengine=engine;
}


int CSurface::create(int width, int height, const RGBFormat &format, SurfaceFlags flags)
{
	if (engineprivate) {
		SetError(1061);
		return 0;
	}
	s.setSize(width,height);
	this->format=format;
	this->flags=flags;
	return create();
}

int CSurface::create()
{
	if (engineprivate) {
		SetError(1061);
		return 0;
	}
	CGrafix *gfx=GetGrafix();
	if (!gfx) return 0;
	if (!gfx->CreateSurface(*this)) return 0;
	return 1;
}



int CSurface::lock(CDrawable &draw)
{
	if (!gfxengine) {
		SetError(1072);
		return 0;
	}
	if (lockcount==0) {
		if (!gfxengine->lockSurface(*this,this->draw)) return 0;
	}
	lockcount++;
	draw=this->draw;
	return 1;
}

int CSurface::unlock()
{
	if (!gfxengine) {
		SetError(1072);
		return 0;
	}
	if (lockcount>0) {
		lockcount--;
		if (lockcount==0) {
			if (!gfxengine->unlockSurface(*this)) {
				lockcount=1;
				return 0;
			}
		}
	}
	return 1;
}



}	// EOF namespace grafix
}	// EOF namespace ppl6
