/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <pplib.h>
#include <pplib-grafix.h>
#include <pplib-tk.h>
#include "pplib-tests.h"
#include "toolkit.h"

int main(int agrc, char** argv)
{

    if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
        printf("setlocale fehlgeschlagen\n");
        throw std::exception();
    }

    pplib::grafix::Grafix gfx;
    pplib::tk::WindowManager_SDL2 wm;

    MainWindow win;
    win.create(1280, 720, false);

    // SDL_Renderer *renderer=(SDL_Renderer*)win.getRenderer();
    while (wm.numWindows() > 0) {
        win.updateFrameRate();
        win.clearScreen();
        win.drawWidgets();
        win.presentScreen();

        wm.handleEvents();
    }

    // wm.destroyWindow(MainWindow);

    return 0;
}

MainWindow::MainWindow()
{
    gfx = pplib::grafix::GetGrafix();
    setWindowTitle("PPLIB Toolkit-Demo");
    setBackgroundColor(pplib::grafix::Color(0, 0, 0, 0));
    setWindowIcon(gfx->Icons32.getDrawable(30));
    menue = NULL;
    frameCountLabel = NULL;
    fpsLabel = NULL;
    exitButton = NULL;
    toolbar = NULL;
}

MainWindow::~MainWindow()
{
}

void MainWindow::create(int width, int height, bool fullscreen)
{
    setSize(width, height);
    int flags = pplib::tk::Window::DefaultWindow;
    if (fullscreen) {
        flags = pplib::tk::Window::DefaultFullscreen;
    }
    setFlags(flags);
    pplib::tk::GetWindowManager()->createWindow(*this);

    menue = new pplib::tk::Frame(0, 0, this->width(), 32);
    addChild(menue);
    pplib::grafix::Size menueSize = menue->clientSize();

    toolbar = new pplib::tk::Frame(0, 32, 256, this->height() - 32);
    addChild(toolbar);

    pplib::tk::Label* label = new pplib::tk::Label(0, 0, 54, menueSize.height, "Frame:");
    menue->addChild(label);

    // menue->addChild(new pplib::tk::VerticalDivider(54,0,6,menueSize.height));

    frameCountLabel = new pplib::tk::Label(60, 0, 54, menueSize.height, "0");
    frameCountLabel->setBorderStyle(pplib::tk::Label::Inset);
    menue->addChild(frameCountLabel);

    menue->addChild(new pplib::tk::VerticalDivider(114, 0, 6, menueSize.height));
    label = new pplib::tk::Label(120, 0, 60, menueSize.height, "FPS:");
    menue->addChild(label);

    fpsLabel = new pplib::tk::Label(180, 0, 60, menueSize.height, "0");
    fpsLabel->setBorderStyle(pplib::tk::Label::Inset);
    menue->addChild(fpsLabel);

    exitButton = new pplib::tk::Button(menueSize.width - menueSize.height, 0, menueSize.height, menueSize.height, "Exit");
    exitButton->setIcon(gfx->Toolbar.getDrawable(68));
    exitButton->setEventHandler(this);
    menue->addChild(exitButton);

    menue->addChild(new pplib::tk::Label(240, 0, 40, menueSize.height, "Text:"));
    input = new pplib::tk::LineInput(280, 0, 250, menueSize.height, "Täst gÄy$§ @!# Hello World!");
    menue->addChild(input);

    okButton = new pplib::tk::Button(250 + 285, 0, 80, menueSize.height, "Ok");
    okButton->setIcon(gfx->Toolbar.getDrawable(24));
    okButton->setEventHandler(this);
    menue->addChild(okButton);
}

void MainWindow::closeEvent(pplib::tk::Event* event)
{
    pplib::tk::WindowManager* wm = pplib::tk::GetWindowManager();
    wm->destroyWindow(*this);
    event->accept();
}

void MainWindow::mouseClickEvent(pplib::tk::MouseEvent* event)
{
    Widget* w = event->widget();
    if (w == exitButton) {
        pplib::tk::WindowManager* wm = pplib::tk::GetWindowManager();
        wm->destroyWindow(*this);
        event->accept();
    }
}

void MainWindow::updateFrameRate()
{
    fps.updateFPS();
    frameCountLabel->setText(pplib::ToString("%d", fps.getFrameCount()));
    fpsLabel->setText(pplib::ToString("%d", fps.getFPS()));
}

FrameRate::FrameRate()
{
    desiredFrameRate = 0;
    fps = 0;
    lastFpsTime = pplib::GetTime();
    framecount = 0;
}

void FrameRate::setFrameRate(int fps)
{
    desiredFrameRate = fps;
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
    pplib::ppl_time_t now = pplib::GetTime();
    if (now > lastFpsTime) {
        lastFpsTime = now;
        fps = framecount;
        framecount = 0;
    }
}
