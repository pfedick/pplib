/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
 *
 *******************************************************************************
 * Copyright (c) 2012, Patrick Fedick <patrick@pfp.de>
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
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#include "ppl7.h"
#include "ppl7-grafix.h"
#include "ppl7-tk.h"




namespace ppl7 {
namespace tk {

using namespace ppl7;
using namespace ppl7::grafix;

#ifdef OLDCODE

static void defaultSurfaceDestroy (void *data)
{

}

static void defaultSurfaceLock (void *data, Drawable &draw)
{

}

static void defaultSurfaceUnlock (void *data)
{

}

static void defaultSurfaceDraw (void *target, void *data, int x, int y)
{

}


Surface::Surface()
{
	myFlags=DefaultSurface;
	myData=NULL;
	privDestroy=defaultSurfaceDestroy;
	privLock=defaultSurfaceLock;
	privUnlock=defaultSurfaceUnlock;
	privDraw=defaultSurfaceDraw;
	w=0;
	h=0;
}

Surface::~Surface()
{
	if (myData) {
		privDestroy(myData);
		free(myData);
	}
}

const RGBFormat &Surface::rgbFormat() const
{
	return myFormat;
}

Surface::SurfaceFlags Surface::flags() const
{
	return (SurfaceFlags)myFlags;
}

int Surface::width() const
{
	return w;
}

int Surface::height() const
{
	return h;
}

void Surface::lock(Drawable &draw)
{
	privLock(myData,draw);
}

void Surface::unlock()
{
	privUnlock(myData);
}

void Surface::draw(Surface *target, int x, int y)
{
	if (!target) throw NullPointerException();
	privDraw(target->myData, myData,x,y);
}

void Surface::setLockFunction( void (*fnLock) (void *, Drawable &))
{
	privLock=fnLock;
}

void Surface::setUnlockFunction( void (*fnUnlock) (void *))
{
	privUnlock=fnUnlock;
}

void Surface::setDestroyFunction( void (*fnDestroy) (void *))
{
	privDestroy=fnDestroy;
}

void Surface::setDrawFunction( void (*fnDraw) (void *, void *, int, int))
{
	privDraw=fnDraw;
}

void Surface::setFlags(SurfaceFlags flags)
{
	myFlags=flags;
}

void Surface::setRGBFormat(const RGBFormat &format)
{
	myFormat=format;
}

void Surface::setSize(int width, int height)
{
	w=width;
	h=height;
}

void Surface::setPrivateData(void *data)
{
	myData=data;
}

#endif // OLDCODE

}	// EOF namespace tk
}	// EOF namespace ppl7
