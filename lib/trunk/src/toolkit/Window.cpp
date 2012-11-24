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


static void setWindowTitle (void *privatedata, const String &Title) {}
static void setWindowIcon (void *privatedata, const Drawable &Icon)  {}
static void createSurface (void *privatedata) {}
static void createTexture (void *privatedata) {}
static Drawable lockWindowSurface (void *privatedata) { return Drawable();}
static void unlockWindowSurface (void *privatedata) {}
static void drawWindowSurface (void *privatedata) {}
static void* getRenderer (void *privatedata) {return NULL;}
static void clearScreen (void *privatedata) {}
static void presentScreen (void *privatedata) {}

static PRIV_WINDOW_FUNCTIONS defWmFunctions = {
		setWindowTitle,
		setWindowIcon,
		createSurface,
		createTexture,
		lockWindowSurface,
		unlockWindowSurface,
		drawWindowSurface,
		getRenderer,
		clearScreen,
		presentScreen
};


Window::Window()
{
	const WidgetStyle *style=GetWidgetStyle();
	windowFlags=DefaultWindow;
	WindowTitle="PPL7 Window";
	fn=&defWmFunctions;
	setSize(640,480);
	privateData=NULL;
	wm=NULL;
	myBackground=style->windowBackgroundColor;
	keyfocus=NULL;
}

Window::~Window()
{
	if (wm!=NULL && privateData!=NULL) wm->destroyWindow(*this);
}

/*!\brief Private Daten des Window-Managers
 *
 * \desc
 * Diese Funktion wird intern vom Window-Manager aufgerufen und sollte vom der Anwendung
 * nicht verwendet werden.
 *
 * \return Liefert einen Pointer auf die private Datenstruktur des Window-Managers für dieses
 * Fenster zurück
 *
 */
void *Window::getPrivateData()
{
	return privateData;
}

/*!\brief Private Daten des Window-Managers setzen
 *
 * \desc
 * Diese Funktion wird intern vom Window-Manager aufgerufen und sollte vom der Anwendung
 * nicht verwendet werden. Der Window-Manager verwendet sie, um einen Pointer auf die
 * interne Datenstruktur für das Fenster zu setzen. Der Window-Manager ist für die Freigabe des
 * Speichers verantwortlich.
 *
 * \param data Pointer auf die interne Datenstruktur
 * \param wm Pointer auf die Klasse des Window-Manager
 */
void Window::setPrivateData(void *data, WindowManager *wm, PRIV_WINDOW_FUNCTIONS *fn)
{
	privateData=data;
	this->wm=wm;
	if (fn==NULL) this->fn=&defWmFunctions;
	else this->fn=fn;
}

void *Window::getRenderer()
{
	return fn->getRenderer(privateData);
}

void Window::clearScreen()
{
	return fn->clearScreen(privateData);
}

void Window::presentScreen()
{
	return fn->presentScreen(privateData);
}

const RGBFormat &Window::rgbFormat() const
{
	return WindowRGBFormat;
}

void Window::setRGBFormat(const RGBFormat &format)
{
	WindowRGBFormat=format;
}


ppluint32 Window::flags() const
{
	return windowFlags;
}

void Window::setFlags(ppluint32 flags)
{
	windowFlags=flags;
}

const String &Window::windowTitle() const
{
	return WindowTitle;
}

void Window::setWindowTitle(const String &title)
{
	WindowTitle=title;
	fn->setWindowTitle(privateData,title);
}

const Drawable &Window::windowIcon() const
{
	return WindowIcon;
}

void Window::setWindowIcon(const Drawable &icon)
{
	WindowIcon=icon;
	fn->setWindowIcon(privateData,icon);
}

const Color &Window::backgroundColor() const
{
	return myBackground;
}

void Window::setBackgroundColor(const Color &c)
{
	myBackground=c;
	needsRedraw();
}

String Window::widgetType() const
{
	return "Window";
}

void Window::drawWidgets()
{
	if (redrawRequired()) {
		Drawable d=fn->lockWindowSurface(privateData);
		draw(d);
		fn->unlockWindowSurface(privateData);
	}
	fn->drawWindowSurface(privateData);
}

MouseState Window::getMouseState()
{
	return mouseState;
}

void Window::redrawWidgets()
{
	if (redrawRequired()) {
		Drawable d=fn->lockWindowSurface(privateData);
		redraw(d);
		fn->unlockWindowSurface(privateData);
	}
	fn->drawWindowSurface(privateData);
}

void Window::paint(Drawable &draw)
{
	draw.cls(myBackground);
}

}	// EOF namespace tk
}	// EOF namespace ppl7
