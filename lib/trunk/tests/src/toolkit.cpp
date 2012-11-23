/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 421 $
 * $Date: 2012-06-07 11:29:25 +0200 (Do, 07 Jun 2012) $
 * $Id: file.cpp 421 2012-06-07 09:29:25Z pafe $
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

#include "../include/prolog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include "../include/ppl7.h"
#include "../include/ppl7-grafix.h"
#include "../include/ppl7-tk.h"
#include "ppl7-tests.h"
#include "toolkit.h"


int main(int agrc, char ** argv)
{

	if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
		printf ("setlocale fehlgeschlagen\n");
		throw std::exception();
	}
	ppl7::String::setGlobalEncoding("UTF-8");

	ppl7::grafix::Grafix gfx;
	ppl7::tk::WindowManager_SDL2 wm;



	MainWindow win;
	win.create(1280,720,false);


	//SDL_Renderer *renderer=(SDL_Renderer*)win.getRenderer();
	while (wm.numWindows()>0) {
		win.updateFrameRate();
		win.clearScreen();
		win.drawWidgets();
		win.presentScreen();

		wm.handleEvents();

	}

    //wm.destroyWindow(MainWindow);

	return 0;
}

MainWindow::MainWindow()
{
	gfx=ppl7::grafix::GetGrafix();
	setWindowTitle("PPL7 Toolkit-Demo");
	setBackgroundColor(ppl7::grafix::Color(0,0,0,0));
	setWindowIcon(gfx->Icons32.getDrawable(30));
	menue=NULL;
	frameCountLabel=NULL;
	fpsLabel=NULL;
	exitButton=NULL;
	toolbar=NULL;
}

MainWindow::~MainWindow()
{

}


void MainWindow::create(int width, int height, bool fullscreen)
{
	setSize(width,height);
	int flags=ppl7::tk::Window::DefaultWindow;
	if (fullscreen) {
		flags=ppl7::tk::Window::DefaultFullscreen;
	}
	setFlags(flags);
	ppl7::tk::GetWindowManager()->createWindow(*this);

	menue=new ppl7::tk::Frame(0,0,this->width(),32);
	addChild(menue);
	ppl7::grafix::Size menueSize=menue->clientSize();

	toolbar=new ppl7::tk::Frame(0,32,256,this->height()-32);
	addChild(toolbar);


	ppl7::tk::Label *label=new ppl7::tk::Label(0,0,54,menueSize.height,"Frame:");
	menue->addChild(label);

	//menue->addChild(new ppl7::tk::VerticalDivider(54,0,6,menueSize.height));

	frameCountLabel=new ppl7::tk::Label(60,0,54,menueSize.height,"0");
	frameCountLabel->setBorderStyle(ppl7::tk::Label::Inset);
	menue->addChild(frameCountLabel);

	menue->addChild(new ppl7::tk::VerticalDivider(114,0,6,menueSize.height));
	label=new ppl7::tk::Label(120,0,60,menueSize.height,"FPS:");
	menue->addChild(label);

	fpsLabel=new ppl7::tk::Label(180,0,60,menueSize.height,"0");
	fpsLabel->setBorderStyle(ppl7::tk::Label::Inset);
	menue->addChild(fpsLabel);

	exitButton=new ppl7::tk::Button(menueSize.width-menueSize.height,0,menueSize.height,menueSize.height,"Exit");
	exitButton->setIcon(gfx->Toolbar.getDrawable(68));
	exitButton->setEventHandler(this);
	menue->addChild(exitButton);

	menue->addChild(new ppl7::tk::Label(240,0,40,menueSize.height,"Text:"));
	input=new ppl7::tk::LineInput(280,0,250,menueSize.height,"Täst gÄy$§ @!# Hello World!");
	menue->addChild(input);

	okButton=new ppl7::tk::Button(250+285,0,80,menueSize.height,"Ok");
	okButton->setIcon(gfx->Toolbar.getDrawable(24));
	okButton->setEventHandler(this);
	menue->addChild(okButton);


}

void MainWindow::closeEvent(ppl7::tk::Event *event)
{
	ppl7::tk::WindowManager *wm=ppl7::tk::GetWindowManager();
	wm->destroyWindow(*this);
	event->accept();
}

void MainWindow::mouseClickEvent(ppl7::tk::MouseEvent *event)
{
	Widget *w=event->widget();
	if (w==exitButton) {
		ppl7::tk::WindowManager *wm=ppl7::tk::GetWindowManager();
		wm->destroyWindow(*this);
		event->accept();
	}
}


void MainWindow::updateFrameRate()
{
	fps.updateFPS();
	frameCountLabel->setText(ppl7::ToString("%d",fps.getFrameCount()));
	fpsLabel->setText(ppl7::ToString("%d",fps.getFPS()));
}



FrameRate::FrameRate()
{
	desiredFrameRate=0;
	fps=0;
	lastFpsTime=ppl7::GetTime();
	framecount=0;
}


void FrameRate::setFrameRate(int fps)
{
	desiredFrameRate=fps;
}


int FrameRate::getFrameCount() const
{
	return framecount;
}

int FrameRate::getFPS() const
{
	return fps;
}

void FrameRate::updateFPS()
{
	framecount++;
	ppl7::ppl_time_t now=ppl7::GetTime();
	if (now>lastFpsTime) {
		lastFpsTime=now;
		fps=framecount;
		framecount=0;
	}
}

