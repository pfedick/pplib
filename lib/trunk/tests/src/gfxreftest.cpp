/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <math.h>
#include "../include/prolog.h"
#include "../include/ppl7.h"
#include "../include/ppl7-grafix.h"

using namespace ppl7;
using namespace ppl7::grafix;

int DoCLS(Drawable &surface, int &num)
{
	num++;
	bool failed=false;
	printf ("Test %i: Clear screen... ", num);
	try  {
		Color c;
		Color transparent(0,0,0,0);
		Color black(0,0,0,255);
		Color red(93,0,0,255);
		surface.cls(Color(0,0,0,0));

		// Prüfen
		c=surface.getPixel(0,0);
		if (c!=transparent) failed=true;
		c=surface.getPixel(333,111);
		if (c!=transparent) failed=true;
		c=surface.getPixel(639,479);
		if (c!=transparent) failed=true;

		surface.cls(Color(93,0,0));
		c=surface.getPixel(0,0);
		if (c!=red) failed=true;
		c=surface.getPixel(333,111);
		if (c!=red) failed=true;
		c=surface.getPixel(639,479);
		if (c!=red) failed=true;

		surface.cls(Color(0,0,0,255));
		surface.fillRect(0,0,surface.width(),30,red);

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed) {
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoLines(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Line drawing... ", num);
	bool failed=false;
	try {
		Color c;
		Color black(0,0,0);
		Color white(255,255,255);

		// Ein paar Linien ohne Antialiasing
		surface.line(0,35,60,90,white);
		surface.line(0,90,60,35,white);
		Color blue(0,0,255);
		surface.line(10,35,10,60,blue);
		surface.line(20,35,10,60,blue);
		surface.line(30,35,10,60,blue);
		surface.line(40,35,90,35,blue);
		surface.line(40,45,90,45,blue);
		surface.line(40,55,90,55,blue);

		float pi=3.1415926535f;
		float rad=pi/180.0f;

		int x,y;
		for (float w=0;w<360;w+=10) {
			x=sinf(w*rad)*50;
			y=cosf(w*rad)*50;
			surface.line(460,60,460+x,60+y,white);
		}

		// Checks
		c=surface.getPixel(0,35);
		if (c!=white) failed=true;
		c=surface.getPixel(10,35);
		if (c!=blue) failed=true;
	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed) {
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoLinesAA(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Antialiased Line drawing... ", num);
	bool failed=false;
	try {
		Color c;
		Color black(0,0,0);
		Color white(255,255,255);
		Color blue(0,0,255);
		// Ein paar Linien mit Antialiasing

		surface.lineAA(100,35,160,90,white);
		surface.lineAA(100,90,160,35,white);
		surface.lineAA(130,50,130,70,white);
		surface.lineAA(120,60,140,60,white);

		surface.lineAA(110,35,110,60,blue);
		surface.lineAA(120,35,110,60,blue);
		surface.lineAA(130,35,110,60,blue);
		surface.lineAA(140,35,190,35,blue);
		surface.lineAA(140,45,190,45,blue);
		surface.lineAA(140,55,190,55,blue);

		// Checks
		c=surface.getPixel(100,35);	// 0xff7e7e7e
		if (c!=Color(0xff7f7f7f)) failed=true;
		c=surface.getPixel(101,35);	// 0xff141414
		if (c!=Color(0xff151515)) failed=true;
		c=surface.getPixel(101,36);	// 0xffe8e8e8
		if (c!=Color(0xffe9e9e9)) failed=true;
		c=surface.getPixel(101,37);	// 0x00000000
		if (c!=Color(0xff000000)) failed=true;
		c=surface.getPixel(103,37);	// 0xff3e3e3e
		if (c!=Color(0xff3f3f3f)) failed=true;
		//printf (" c=%x ",c);

		float pi=3.1415926535f;
		float rad=pi/180.0f;

		int x,y;
		for (float w=0;w<360;w+=10) {
			x=sinf(w*rad)*50;
			y=cosf(w*rad)*50;
			surface.lineAA(560,60,560+x,60+y,white);
		}

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoColorGradient(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Draw color gradient... ", num);
	bool failed=false;
	try {
		surface.colorGradient(0,200,surface.width(),surface.height(),Color(0,0,0),Color(0,0,255),2);
		if (surface.getPixel(13,255)!=Color(0,0,0x32)) failed=true;
		if (surface.getPixel(10,293)!=Color(0,0,0x54)) failed=true;
		if (surface.getPixel(13,346)!=Color(0,0,0x84)) failed=true;
		if (surface.getPixel(07,470)!=Color(0,0,0xf5)) failed=true;

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int AlphaPixelPattern(Drawable &surface, const Color &background)
{
	surface.cls(background);
	surface.putPixel(0,1,Color(255,255,255,255));
	surface.alphaPixel(1,1,Color(255,255,255,255));
	surface.alphaPixel(2,1,Color(255,255,255,254));
	surface.alphaPixel(3,1,Color(255,255,255,250));
	surface.alphaPixel(4,1,Color(255,255,255,200));
	surface.alphaPixel(5,1,Color(255,255,255,150));
	surface.alphaPixel(6,1,Color(255,255,255,100));
	surface.alphaPixel(7,1,Color(255,255,255,50));
	surface.alphaPixel(8,1,Color(255,255,255,1));
	surface.alphaPixel(9,1,Color(255,255,255,0));

	surface.putPixel(0,2,Color(255,0,0,255));
	surface.alphaPixel(1,2,Color(255,0,0,255));
	surface.alphaPixel(2,2,Color(255,0,0,254));
	surface.alphaPixel(3,2,Color(255,0,0,250));
	surface.alphaPixel(4,2,Color(255,0,0,200));
	surface.alphaPixel(5,2,Color(255,0,0,150));
	surface.alphaPixel(6,2,Color(255,0,0,100));
	surface.alphaPixel(7,2,Color(255,0,0,50));
	surface.alphaPixel(8,2,Color(255,0,0,1));
	surface.alphaPixel(9,2,Color(255,0,0,0));

	surface.putPixel(0,3,Color(0,255,0,255));
	surface.alphaPixel(1,3,Color(0,255,0,255));
	surface.alphaPixel(2,3,Color(0,255,0,254));
	surface.alphaPixel(3,3,Color(0,255,0,250));
	surface.alphaPixel(4,3,Color(0,255,0,200));
	surface.alphaPixel(5,3,Color(0,255,0,150));
	surface.alphaPixel(6,3,Color(0,255,0,100));
	surface.alphaPixel(7,3,Color(0,255,0,50));
	surface.alphaPixel(8,3,Color(0,255,0,1));
	surface.alphaPixel(9,3,Color(0,0255,0,0));

	surface.putPixel(0,4,Color(0,0,255,255));
	surface.alphaPixel(1,4,Color(0,0,255,255));
	surface.alphaPixel(2,4,Color(0,0,255,254));
	surface.alphaPixel(3,4,Color(0,0,255,250));
	surface.alphaPixel(4,4,Color(0,0,255,200));
	surface.alphaPixel(5,4,Color(0,0,255,150));
	surface.alphaPixel(6,4,Color(0,0,255,100));
	surface.alphaPixel(7,4,Color(0,0,255,50));
	surface.alphaPixel(8,4,Color(0,0,255,1));
	surface.alphaPixel(9,4,Color(0,0,255,0));

	//surface.alphaPixel(-1,5,Color(255,255,255,128));
	//surface.alphaPixel(5,-1,Color(255,255,255,128));
	surface.alphaPixel(surface.width(),5,Color(255,255,255,128));
	surface.alphaPixel(5,surface.height(),Color(255,255,255,128));
	return 1;
}

int DoAlphaPixel(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Draw pixel with alphablending... ", num);
	bool failed=false;
	try {
		Drawable draw;
		draw=surface.getDrawable(Rect(650,5,11,10));
		AlphaPixelPattern(draw,Color(0,0,0));
		draw=surface.getDrawable(Rect(665,5,11,10));
		AlphaPixelPattern(draw,Color(255,255,255));
		draw=surface.getDrawable(Rect(680,5,11,10));
		AlphaPixelPattern(draw,Color(255,0,0));
		draw=surface.getDrawable(Rect(695,5,11,10));
		AlphaPixelPattern(draw,Color(0,255,0));
		draw=surface.getDrawable(Rect(710,5,11,10));
		AlphaPixelPattern(draw,Color(0,0,255));

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoPutPixel(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Draw pixel... ", num);
	bool failed=false;
	try {
		Color white(255,255,255);
		// Sinuskurve erzeugen
		float s=0;
		float pi=3.1415926535f;
		float rad=pi/180.0f;

		for (int x=0;x<640;x++) {
			float y=sinf(s*rad);
			s+=pi;
			surface.blendPixel(x-1,15+(y*15),white,200);
			surface.blendPixel(x-2,15+(y*15),white,150);
			surface.blendPixel(x-3,15+(y*15),white,100);
			surface.blendPixel(x-4,15+(y*15),white,50);
			surface.blendPixel(x+1,15+(y*15),white,200);
			surface.blendPixel(x+2,15+(y*15),white,150);
			surface.blendPixel(x+3,15+(y*15),white,100);
			surface.blendPixel(x+4,15+(y*15),white,50);
			surface.putPixel(x,15+(y*15),white);
			if (surface.getPixel(x,15+(y*15))!=white) failed=true;
		}

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoRGBBlend(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: RGBBlend... ", num);
	bool failed=false;
	try {
		Color c;
		c.blend(0x000000ff,0xffff7f00,10);
		if (!c.match(0xff0a04f5)) failed=true;
		c.blend(0x000000ff,0xffff7f00,65);
		if (!c.match(0xff4120be)) failed=true;
		c.blend(0x000000ff,0xffff7f00,0);
		if (!c.match(0xff0000ff)) failed=true;
		c.blend(0x000000ff,0xffff7f00,255);
		if (!c.match(0xffff7f00)) failed=true;
		c.blend(0x000000ff,0xffff7f00,176);
		if (!c.match(0xffb0574f)) failed=true;
		c.blend(0x30ca7499,0x9a0fe0cc,83);
		if (!c.match(0xff8d97a9)) failed=true;
		c.blend(0xffe0ff00,0x9a0fe0cc,240);
		if (!c.match(0xff1be1c0)) failed=true;
		//printf ("c=0x%8x\n",c);

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoRects(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Draw rects... ", num);
	bool failed=false;
	try {
		Color c;
		// Ein paar Rechtecke
		Color black(0,0,0);
		Color white(255,255,255);
		Color red(255,0,0);
		Color green(0,255,0);
		Color blue(0,0,255);
		Color magenta(255,0,255);
		Color yellow(255,255,0);
		Color cyan(0,255,255);

		surface.drawRect(200,35,229,65,white);
		surface.drawRect(230,35,259,65,red);
		surface.drawRect(260,35,289,65,green);
		surface.drawRect(290,35,319,65,blue);
		surface.drawRect(320,35,349,65,magenta);
		surface.drawRect(350,35,379,65,yellow);
		surface.drawRect(380,35,409,65,cyan);

		// Checks
		c=surface.getPixel(230,35);	// 0xffff0000
		if (c!=red) failed=true;
		c=surface.getPixel(259,35);	// 0xffff0000
		if (c!=red) failed=true;
		c=surface.getPixel(259,65);	// 0xffff0000
		if (c!=red) failed=true;
		c=surface.getPixel(230,65);	// 0xffff0000
		if (c!=red) failed=true;
		// Die Farben der einzelnen Rechtecke prüfen
		c=surface.getPixel(200,35);
		if (c!=white) failed=true;
		c=surface.getPixel(230,35);
		if (c!=red) failed=true;
		c=surface.getPixel(260,35);
		if (c!=green) failed=true;
		c=surface.getPixel(290,35);
		if (c!=blue) failed=true;
		c=surface.getPixel(320,35);
		if (c!=magenta) failed=true;
		c=surface.getPixel(350,35);
		if (c!=yellow) failed=true;
		c=surface.getPixel(380,35);
		if (c!=cyan) failed=true;

		// Ausserhalb des Rechtecks darf nichts sein
		c=surface.getPixel(230,34);	// 0
		if (c!=black) failed=true;
		c=surface.getPixel(199,35);	// 0
		if (c!=black) failed=true;
		c=surface.getPixel(230,66);	// 0
		if (c!=black) failed=true;
		c=surface.getPixel(410,40);	// 0
		if (c!=black) failed=true;
		// Und innerhalb auch nicht
		c=surface.getPixel(231,36);	// 0
		if (c!=black) failed=true;
		c=surface.getPixel(258,36);	// 0
		if (c!=black) failed=true;
		c=surface.getPixel(258,64);	// 0
		if (c!=black) failed=true;
		c=surface.getPixel(231,64);	// 0
		if (c!=black) failed=true;

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed) {
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoFilledRects(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Draw filled rects... ", num);
	bool failed=false;
	try {
		Color c;

		Color black(0,0,0);
		Color white(255,255,255);
		Color red(255,0,0);
		Color green(0,255,0);
		Color blue(0,0,255);
		Color magenta(255,0,255);
		Color yellow(255,255,0);
		Color cyan(0,255,255);

		// Ein paar ausgefüllte Rechtecke
		surface.fillRect(200,70,229,100,white);
		surface.fillRect(230,70,259,100,red);
		surface.fillRect(260,70,289,100,green);
		surface.fillRect(290,70,319,100,blue);
		surface.fillRect(320,70,349,100,magenta);
		surface.fillRect(350,70,379,100,yellow);
		surface.fillRect(380,70,409,100,cyan);

		// Checks
		c=surface.getPixel(230,70);	// 0xffff0000
		if (c!=red) failed=true;
		c=surface.getPixel(259,70);	// 0xffff0000
		if (c!=red) failed=true;
		c=surface.getPixel(259,100);	// 0xffff0000
		if (c!=red) failed=true;
		c=surface.getPixel(230,100);	// 0xffff0000
		if (c!=red) failed=true;
		// Die Farben der einzelnen Rechtecke prüfen
		c=surface.getPixel(210,80);
		if (c!=white) failed=true;
		c=surface.getPixel(240,90);
		if (c!=red) failed=true;
		c=surface.getPixel(266,83);
		if (c!=green) failed=true;
		c=surface.getPixel(294,85);
		if (c!=blue) failed=true;
		c=surface.getPixel(336,88);
		if (c!=magenta) failed=true;
		c=surface.getPixel(352,98);
		if (c!=yellow) failed=true;
		c=surface.getPixel(398,92);
		if (c!=cyan) failed=true;

		// Ausserhalb des Rechtecks darf nichts sein
		c=surface.getPixel(230,69);	// 0
		if (c!=black) failed=true;
		c=surface.getPixel(199,70);	// 0
		if (c!=black) failed=true;
		c=surface.getPixel(230,101);	// 0
		if (c!=black) failed=true;
		c=surface.getPixel(410,80);	// 0
		if (c!=black) failed=true;

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed) {
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int LoadFonts(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Loading Font-Files... ", num);
	bool failed=false;
	Grafix *gfx=GetGrafix();
	try {
		gfx->loadFont("../resource/liberationsans8.fnt5","PPL Liberation Sans 8");
		gfx->loadFont("../resource/liberationsans2.fnt5","PPL Liberation Sans 2");
		gfx->loadFont("testdata/LiberationSans-Bold.ttf","PPL Liberation Sans Bold TTF");


		/*
	if (!gfx->LoadFont("../../fonts/freesans4.fnt5","PPL FreeSans 4")) {
		failed=true;
		PrintError();
	}

	if (!gfx->LoadFont("../../fonts/segoeui4.fnt5","SegoeUI")) {
		failed=true;
		PrintError();
	}
		 */

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		gfx->listFonts();
		return 0;
	}
	if (failed ){
		printf ("failed!\n");
		gfx->listFonts();
		return 0;
	}
	printf ("ok\n");
	gfx->listFonts();
	return 1;
}

int DoFontsNonAA(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Fonts without antialiasing... ", num);
	bool failed=false;
	try {
		Color c;
		Font Font;
		Font.setAntialias(false);
		Font.setName("PPL Liberation Sans 2");
		Font.setSize(12);
		Font.setColor(Color(255,255,255));
		surface.print(Font,5,120,"Ein Font-Test mit \"Liberation Sans\" ohne Antialiasing");

		c=surface.getPixel(7,119);
		if (!c.match(0xffffffff)) failed=true;
		c=surface.getPixel(5,119);
		if (!c.match(0xff000000)) failed=true;
		c=surface.getPixel(8,119);
		if (!c.match(0xffffffff)) failed=true;
		c=surface.getPixel(9,119);
		if (!c.match(0xffffffff)) failed=true;
		c=surface.getPixel(7,118);
		if (!c.match(0xff000000)) failed=true;
		//printf ("Farbe: %x\n",c);
	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoFontsAA2(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Fonts with 2-Bit antialiasing... ", num);
	bool failed=false;
	try {
		Color c;
		Font Font;
		Font.setAntialias(true);
		Font.setName("PPL Liberation Sans 2");
		Font.setSize(12);
		Font.setColor(Color(255,255,255));
		surface.print(Font,5,140,"Ein Font-Test mit \"Liberation Sans\" und 4-Graustufen Antialiasing");

		c=surface.getPixel(19,134);
		if (!c.match(0xffc8c8c8)) failed=true;
		c=surface.getPixel(18,134);
		if (!c.match(0xffffffff)) failed=true;
		c=surface.getPixel(17,134);
		if (!c.match(0xff000000)) failed=true;

		Font.setName("SegoeUI");
		Font.setBold(false);
		surface.print(Font,5,425,"Ein Font-Test SegoeUI");
		Font.setBold(true);
		surface.print(Font,5,440,"Ein Font-Test SegoeUI");

		Font.setName("PPL FreeSans 4");
		Font.setBold(false);
		surface.print(Font,5,455,"Ein Font-Test FreeSans");
		Font.setBold(true);
		surface.print(Font,5,470,"Ein Font-Test FreeSans");

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}
	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoFontsAA8(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Fonts with 8-Bit antialiasing... ", num);
	bool failed=false;
	try {
		Color c;
		Font Font;
		Font.setAntialias(true);
		Font.setName("PPL Liberation Sans 8");
		Font.setSize(12);
		Font.setColor(Color(255,255,255));
		surface.print(Font,5,160,"Ein Font-Test mit \"Liberation Sans\" und 256-Graustufen Antialiasing");

		c=surface.getPixel(19,153);
		if (!c.match(0xff454545)) failed=true;
		c=surface.getPixel(19,154);
		if (!c.match(0xffa0a0a0)) failed=true;
		c=surface.getPixel(18,155);
		if (!c.match(0xffffffff)) failed=true;
		c=surface.getPixel(17,153);
		if (!c.match(0xff000000)) failed=true;
		//printf ("Farbe: %x\n",c);

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoFontsFreetype(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Fonts with FreeType... ", num);
	bool failed=false;
	try {
		Color c;
		Font Font;
		Font.setAntialias(true);
		Font.setName("PPL Liberation Sans Bold TTF");
		Font.setSize(12);
		Font.setColor(Color(255,255,255));
		surface.print(Font,5,180,"Ein Font-Test mit \"Liberation Sans Bold Truetype\"");
		Font.setRotation(10.0);
		surface.print(Font,12,12,"Ein Font-Test mit \"Liberation Sans Bold Truetype\"");
		Font.setRotation(20.0);
		surface.print(Font,12,12,"Ein Font-Test mit \"Liberation Sans Bold Truetype\"");
		Font.setRotation(30.0);
		surface.print(Font,12,12,"Ein Font-Test mit \"Liberation Sans Bold Truetype\"");
		Font.setRotation(45.0);
		surface.print(Font,12,12,"Ein Font-Test mit \"Liberation Sans Bold Truetype\"");
		Font.setRotation(90.0);
		surface.print(Font,12,12,"Ein Font-Test mit \"Liberation Sans Bold Truetype\"");
		surface.line(700,0,700,500,Color(128,128,128));
		surface.print(Font,700,12,"Ein Font-Test mit \"Liberation Sans Bold Truetype\"");


		/*
	c=surface.getPixel(19,153);
	if (!c.match(0xff454545)) failed=true;
	c=surface.getPixel(19,154);
	if (!c.match(0xffa0a0a0)) failed=true;
	c=surface.getPixel(18,155);
	if (!c.match(0xffffffff)) failed=true;
	c=surface.getPixel(17,153);
	if (!c.match(0xff000000)) failed=true;
	//printf ("Farbe: %x\n",c);
		 */

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoLoadIcons(Image &icons, int &num)
{
	num++;
	printf ("Test %i: Loading 32-Bit BMP-Image... ", num);
	bool failed=false;
	try {
		icons.load("testdata/unittest.png");
	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}
	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoAlphaBlt(Drawable &surface, int &num, Image &Icons)
{
	num++;
	printf ("Test %i: AlphaBlt... ", num);
	bool failed=false;
	try {
		Color c;
		Grafix *gfx=GetGrafix();
		Resource *res=GetPPLResource();
		Image Img;
		Img.load(res->getMemory(5));
		surface.bltAlpha(Img,Rect(),(640-128)/2,340);
		Rect r(0,0,32,32);
		surface.bltAlpha(Icons,r,600,120);


		/*
	// Eckpunkte prüfen
	c=surface.getPixel(600,120);
	if (!Match(c,0xffffffff)) failed=true;
	c=surface.getPixel(631,120);
	if (!Match(c,0xffffffff)) failed=true;
	c=surface.getPixel(631,151);
	if (!Match(c,0xffffffff)) failed=true;
	c=surface.getPixel(600,151);
	if (!Match(c,0xffffffff)) failed=true;

	// Außenbereich prüfen
	c=surface.getPixel(599,120);
	if (!Match(c,0xff000000)) failed=true;
	c=surface.getPixel(632,120);
	if (!Match(c,0xff000000)) failed=true;
	c=surface.getPixel(600,119);
	if (!Match(c,0xff000000)) failed=true;
	c=surface.getPixel(600,152);
	if (!Match(c,0xff000000)) failed=true;
	c=surface.getPixel(631,119);
	if (!Match(c,0xff000000)) failed=true;
	c=surface.getPixel(631,152);
	if (!Match(c,0xff000000)) failed=true;

	// Leeren Innenbereich prüfen
	c=surface.getPixel(601,123);
	if (!Match(c,0xff000000)) failed=true;
	c=surface.getPixel(630,123);
	if (!Match(c,0xff000000)) failed=true;
	c=surface.getPixel(601,148);
	if (!Match(c,0xff000000)) failed=true;
	c=surface.getPixel(630,148);
	if (!Match(c,0xff000000)) failed=true;

	// Farben prüfen
	c=surface.getPixel(601,121);
	if (!Match(c,0xffff0000)) failed=true;
	c=surface.getPixel(630,121);
	if (!Match(c,0xff00ff00)) failed=true;
	c=surface.getPixel(601,150);
	if (!Match(c,0xff0000ff)) failed=true;
	c=surface.getPixel(630,150);
	if (!Match(c,0xffff00ff)) failed=true;
	c=surface.getPixel(603,150);
	if (!Match(c,0xffffff00)) failed=true;
	c=surface.getPixel(604,150);
	if (!Match(c,0xffffc000)) failed=true;
	c=surface.getPixel(605,150);
	if (!Match(c,0xff809000)) failed=true;
	c=surface.getPixel(609,150);
	if (!Match(c,0xff6d00e0)) failed=true;
	c=surface.getPixel(614,150);
	if (!Match(c,0xff00ff8d)) failed=true;

	// Alphablending-Bereich
	c=surface.getPixel(611,127);
	if (!Match(c,0xff00a007)) failed=true;
	c=surface.getPixel(618,130);
	if (!Match(c,0xff003402)) failed=true;
	//printf ("Farbe: %x\n",c);
		 */

		float pi=3.1415926535f;
		float rad=pi/180.0f;
		int x,y;
		int i=0;
		for (float w=0;w<360;w+=15) {
			x=(int)(sinf(w * rad)*60.0f);
			y=(int)(cosf(w * rad)*50.0f);
			surface.draw(gfx->Icons32,i++,460+x,200+y);
		}
		i=0;
		for (float w=0.0f;w<360.0f;w+=6.0f) {
			x=sinf(w*rad)*100;
			y=cosf(w*rad)*90;
			surface.draw(gfx->Toolbar,i++,468+x,208+y);
		}


	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoDiffuseBlt(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Blt diffuse... ", num);
	bool failed=false;
	try {
		Color c;
		Grafix *gfx=GetGrafix();

		surface.fillRect(30,300,230,400,Color(0,0,0));
		surface.fillRect(25,295,225,395,Color(64,64,255));
		surface.line(25,295,225,295,Color(255,255,255));
		surface.line(25,295,25,395,Color(255,255,255));
		surface.line(225,296,225,395,Color(128,128,255));
		surface.line(26,395,225,395,Color(128,128,255));


		surface.draw(gfx->ButtonSymbolsSmall,0,60,300,Color(255,255,255));
		surface.draw(gfx->ButtonSymbolsSmall,1,80,300,Color(255,200,100));
		surface.draw(gfx->ButtonSymbolsSmall,2,100,300,Color(238,85,193));
		surface.draw(gfx->ButtonSymbolsSmall,3,120,300,Color(0,255,0));
		surface.draw(gfx->ButtonSymbolsSmall,4,140,300,Color(0,255,255));
		surface.draw(gfx->ButtonSymbolsSmall,5,160,300,Color(0,0,255));
		surface.draw(gfx->ButtonSymbolsSmall,6,180,300,Color(255,0,255));

		surface.draw(gfx->ButtonSymbolsSmall,7,60,320,Color(255,0,0));
		surface.draw(gfx->ButtonSymbolsSmall,8,80,320,Color(64,128,64));
		surface.draw(gfx->ButtonSymbolsSmall,9,100,320,Color(64,128,255));
		surface.draw(gfx->ButtonSymbolsSmall,21,120,320,Color(255,255,255));
		surface.draw(gfx->ButtonSymbolsSmall,20,140,320,Color(255,255,255));

		c=surface.getPixel(64,304); if (!c.match(0xffffffff)) failed=true;
		c=surface.getPixel(82,303); if (!c.match(0xffd0a689)) failed=true;
		c=surface.getPixel(105,307); if (!c.match(0xffa44cdb)) failed=true;


	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int BltClipping(Drawable &surface, int &num, Image &Icons)
{
	num++;
	printf ("Test %i: Blt Clipping... ", num);
	bool failed=false;
	try {
		Drawable draw=surface.getDrawable(26,296,224,394);
		//draw.CLS(Color(0,128,0));
		//surface.putPixel(224,296,Color(255,0,0));

		Rect r(64,0,32,32);
		draw.bltAlpha(Icons,r,-16,-16);
		draw.bltAlpha(Icons,r,draw.width()-16,-16);
		draw.bltAlpha(Icons,r,-16,draw.height()-16);
		draw.bltAlpha(Icons,r,draw.width()-16,draw.height()-16);


	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;

}

int DoBlt(Drawable &surface, int &num, Image &Icons)
{
	num++;
	printf ("Test %i: Blt... ", num);
	bool failed=false;
	try {
		Color c;
		Rect r(32,0,32,32);
		surface.blt(Icons,r,600,160);
		// Eckpunkte prüfen
		c=surface.getPixel(600,160);
		if (!c.match(0xffffffff)) failed=true;
		c=surface.getPixel(631,160);
		if (!c.match(0xffffffff)) failed=true;
		c=surface.getPixel(631,191);
		if (!c.match(0xffffffff)) failed=true;
		c=surface.getPixel(600,191);
		if (!c.match(0xffffffff)) failed=true;

		// Außenbereich prüfen
		c=surface.getPixel(599,160);
		if (!c.match(0xff000000)) failed=true;
		c=surface.getPixel(632,160);
		if (!c.match(0xff000000)) failed=true;
		c=surface.getPixel(600,159);
		if (!c.match(0xff000000)) failed=true;
		c=surface.getPixel(600,192);
		if (!c.match(0xff000000)) failed=true;
		c=surface.getPixel(631,159);
		if (!c.match(0xff000000)) failed=true;
		c=surface.getPixel(631,192);
		if (!c.match(0xff000000)) failed=true;

		// Leeren Innenbereich prüfen
		c=surface.getPixel(601,163);
		if (!c.match(0xffff00ff)) failed=true;
		c=surface.getPixel(630,163);
		if (!c.match(0xffff00ff)) failed=true;
		c=surface.getPixel(601,188);
		if (!c.match(0xffff00ff)) failed=true;
		c=surface.getPixel(630,188);
		if (!c.match(0xffff00ff)) failed=true;

		// Farben prüfen
		c=surface.getPixel(601,161);
		if (!c.match(0xffff0000)) failed=true;
		c=surface.getPixel(630,161);
		if (!c.match(0xff00ff00)) failed=true;
		c=surface.getPixel(601,190);
		if (!c.match(0xff0000ff)) failed=true;
		c=surface.getPixel(630,190);
		if (!c.match(0xffff00ff)) failed=true;
		c=surface.getPixel(603,190);
		if (!c.match(0xffffff00)) failed=true;
		c=surface.getPixel(604,190);
		if (!c.match(0xffffc000)) failed=true;
		c=surface.getPixel(605,190);
		if (!c.match(0xff809000)) failed=true;
		c=surface.getPixel(609,190);
		if (!c.match(0xff6d00e0)) failed=true;
		c=surface.getPixel(614,190);
		if (!c.match(0xff00ff8d)) failed=true;

		// Alphablending-Bereich
		c=surface.getPixel(611,167);
		if (!c.match(0xff5ea165)) failed=true;
		c=surface.getPixel(618,170);
		if (!c.match(0xffca35cc)) failed=true;

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoColorKeyBlt(Drawable &surface, int &num, Image &Icons)
{
	num++;
	printf ("Test %i: Blt with color key... ", num);
	bool failed=false;
	try {
		Color c;

		Rect r(32,0,32,32);
		surface.fillRect(599,199,633,233,Color(0,0,0));	// Hintergrund schwarz
		surface.bltColorKey(Icons,r,600,200,0xffff00ff);
		//surface->Blt(src,&r,600,200);

		/*
	// Eckpunkte prüfen
	c=surface->GetPixel(600,200);
	if (c!=0xffffffff) failed=true;
	c=surface->GetPixel(631,200);
	if (c!=0xffffffff) failed=true;
	c=surface->GetPixel(631,231);
	if (c!=0xffffffff) failed=true;
	c=surface->GetPixel(600,231);
	if (c!=0xffffffff) failed=true;

	// Außenbereich prüfen
	c=surface->GetPixel(599,200);
	if (c!=0xff000000) failed=true;
	c=surface->GetPixel(632,200);
	if (c!=0xff000000) failed=true;
	c=surface->GetPixel(600,199);
	if (c!=0xff000000) failed=true;
	c=surface->GetPixel(600,232);
	if (c!=0xff000000) failed=true;
	c=surface->GetPixel(631,199);
	if (c!=0xff000000) failed=true;
	c=surface->GetPixel(631,232);
	if (c!=0xff000000) failed=true;

	// Leeren Innenbereich prüfen
	c=surface->GetPixel(601,203);
	if (c!=0xff000000) failed=true;
	c=surface->GetPixel(630,203);
	if (c!=0xff000000) failed=true;
	c=surface->GetPixel(601,228);
	if (c!=0xff000000) failed=true;
	c=surface->GetPixel(630,228);
	if (c!=0xff000000) failed=true;


	// Farben prüfen
	c=surface->GetPixel(601,201);
	if (c!=0xffff0000) failed=true;
	c=surface->GetPixel(630,201);
	if (c!=0xff00ff00) failed=true;
	c=surface->GetPixel(601,230);
	if (c!=0xff0000ff) failed=true;
	c=surface->GetPixel(630,230);		// Die ist jetzt schwarz wegen ColorKey
	if (c!=0xff000000) failed=true;
	c=surface->GetPixel(603,230);
	if (c!=0xffffff00) failed=true;
	c=surface->GetPixel(604,230);
	if (c!=0xffffc000) failed=true;
	c=surface->GetPixel(605,230);
	if (c!=0xff809000) failed=true;
	c=surface->GetPixel(609,230);
	if (c!=0xff6d00e0) failed=true;
	c=surface->GetPixel(614,230);
	if (c!=0xff00ff8d) failed=true;

	// Alphablending-Bereich

	c=surface->GetPixel(611,167);
	if (c!=0xff5ea165) failed=true;
	c=surface->GetPixel(618,170);
	if (c!=0xffca35cc) failed=true;
	//printf ("Farbe: %x\n",c);
		 */

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

void PresentImage(Drawable &surface,const Image &Image, const String &Type, int x, int y)
{
	Font Font;
	Font.setColor(Color(255,255,255));
	Font.setAntialias(true);
	Font.setBold(true);
	surface.fillRect(Rect(x+5,y+5,124,99),Color(0,0,0));
	surface.fillRect(Rect(x,y,124,99),Color(40,50,40));
	surface.blt(Image,x+2,y+2);
	surface.line(x,y,x+122,y,Color(255,255,255));
	surface.line(x,y,x,y+98,Color(255,255,255));
	surface.line(x+123,y,x+123,y+98,Color(128,128,128));
	surface.line(x+1,y+98,x+123,y+98,Color(128,128,128));

	try {
		surface.print(Font,x+70,y+12,Type);
	} catch (...) {

	}

}

int DoPNG(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: PNG-Load... ", num);
	bool failed=false;
	Image Img;
	//PresentImage(surface, Img,"PNG",10,380);

	try {
		Img.load("testdata/test.png",RGBFormat::A8R8G8B8);
		PresentImage(surface, Img,"PNG",10,190);
	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}
	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoBMP(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: BMP-Load... ", num);
	bool failed=false;
	Image Img;
	try {
		Img.load("testdata/test.bmp",RGBFormat::A8R8G8B8);
		PresentImage(surface, Img,"BMP",100,220);
	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}
	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoJPEG(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: JPEG-Load... ", num);
	bool failed=false;
	Image Img;
	try {
		Img.load("testdata/test.jpg",RGBFormat::A8R8G8B8);
		PresentImage(surface, Img,"JPEG",10+2*90,250);
	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}
	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoGIF(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: GIF-Load... ", num);
	bool failed=false;
	Image Img;
	try {
		Img.load("testdata/test.gif",RGBFormat::A8R8G8B8);
		PresentImage(surface, Img,"GIF",10+3*90,280);
	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}
	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoPPM(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: PPM-Load... ", num);
	bool failed=false;
	Image Img;
	try {
		Img.load("testdata/test.ppm",RGBFormat::A8R8G8B8);
		PresentImage(surface, Img,"PPM",10+4*90,310);
	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}
	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoTGA(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: TGA-Load... ", num);
	bool failed=false;
	Image Img;
	try {
		Img.load("testdata/test.tga",RGBFormat::A8R8G8B8);
		PresentImage(surface, Img,"TGA",10+5*90,340);
	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}
	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}


int DoTIFF(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: TIFF-Load... ", num);
	bool failed=false;
	Image Img;
	try {
		Img.load("testdata/test.tif",RGBFormat::A8R8G8B8);
		PresentImage(surface, Img,"TIFF",10+6*90,370);
	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}
	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoButton(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Button... ", num);
	bool failed=false;

	try {
		Grafix *gfx=GetGrafix();
		Drawable draw=surface.getDrawable(80,350,180,380);
		Rect r=draw.rect();
		draw.drawRect(r,Color(66,66,66));
		int x1=r.x1+1;
		int y1=r.y1+1;
		int x2=r.x2-1;
		int y2=r.y2-1;
		int m=(y2-y1)/2+y1;
		draw.line(x1,y1,x2,y1,Color(219,219,219));
		draw.colorGradient(x1,y1+1,x2,m,Color(152,152,152),Color(125,125,125),1);
		draw.colorGradient(x1,m+1,x2,y2,Color(67,67,67),Color(43,43,43),1);

		draw.colorGradient(x1,y1,x1,m,Color(219,219,219),Color(130,130,130),1);
		draw.colorGradient(x2,y1,x2,m,Color(219,219,219),Color(130,130,130),1);
		draw.line(x1,m,x2,m,Color(83,83,83));
		Color c;
		Point mp((x2-x1)/2+x1,y2+(y2-m));
		Point p;
		float d,maxd;
		float brightness;
		p.setPoint(x1,y1);
		maxd=Distance(mp,p)*0.8;
		for (int y=y1+1;y<y2;y++) {
			for (int x=x1+1;x<x2;x++) {
				p.setPoint(x,y);
				c=draw.getPixel(x,y);
				// Entfernung zum Mittelpunkt berechnen
				d=Distance(mp,p);
				brightness=(maxd-d)/maxd;
				if (brightness<0) brightness=0.0f;
				//printf ("Distance von %i:%i zu %i:%i: %.2f\n",p.x(),p.y(),mp.x(),mp.y(),d);
				draw.blendPixel(x,y,Color(128,128,256),brightness);
			}
		}

		draw.draw(gfx->Toolbar,24,x1+4,y1+(y2-y1-16)/2);

		//draw.draw(gfx->Toolbar,0,8,8);

		Font Font;
		//Font.setName("PPL FreeSans 4");
		Font.setAntialias(true);
		Font.setSize(14);
		Font.setBold(false);
		Font.setOrientation(Font::BOTTOM);
		Font.setColor(Color(255,255,255));
		draw.print(Font,x1+24,m+5,"Button");

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed ){
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int DoSprites(Drawable &surface, int &num)
{
	num++;
	printf ("Test %i: Sprite... ", num);
	bool failed=false;
	try {


		/*TODO:
	CSprite Sprite;
	if (!Sprite.load("../../../games/SissisAbenteuer/res/sissi-l.tex",CSurface::Lockable)) {
		PrintError();
		failed=true;
	} else {

	}
		 */

	} catch (Exception &e) {
		printf ("failed! %s\n",e.what());
		return 0;
	}

	if (failed) {
		printf ("failed!\n");
		return 0;
	}
	printf ("ok\n");
	return 1;
}

int main(int argc, char **argv)
{
	int failed=0;
	int succeeded=0;
	Grafix Grafix;
	printf ("sizeof(Drawable)=%i\n",(int)sizeof(Drawable));
	printf ("sizeof(Image)=%i\n",(int)sizeof(Image));
	printf ("sizeof(ImageList)=%i\n",(int)sizeof(ImageList));
	printf ("sizeof(Font)=%i\n",(int)sizeof(Font));

	ImageFilter_PNG png;

	/*
	try {
		Image Image("/home/patrickf/cvs-privat/WinMusik/wm30/resources/aboutbg.png");
		printf ("Image geladen: %i x %i Pixel, Format: %s\n",Image.width(), Image.height(),(const char*) Image.rgbformat().name());

		int x=440;
		int y=30;
		Image.drawRect(Rect(x,y,200,100),Color(255,0,0));
		Image.putPixel(x,y,Color(255,255,255));
		Image.putPixel(x+200,y,Color(255,255,255));
		Image.putPixel(x,y+100,Color(255,255,255));
		Image.putPixel(x+200,y+100,Color(255,255,255));


		printf ("Speichere Bild als PNG...\n");
		png.SaveFile(Image,"test.png");
		printf ("done.\n");
	}
	catch (...) {
		ppl6::PrintError();
		return 1;
	}
	*/
	int count=0;
	Image Icons;
	Image Image(800,480,RGBFormat::A8R8G8B8);
	DoCLS(Image,count);
	DoRGBBlend(Image,count);
	DoColorGradient(Image,count);
	DoPutPixel(Image,count);
	DoAlphaPixel(Image,count);
	DoLines(Image,count);
	DoLinesAA(Image,count);
	DoRects(Image,count);
	DoFilledRects(Image,count);
	LoadFonts(Image,count);
	DoFontsAA2(Image,count);
	DoFontsNonAA(Image,count);
	DoFontsAA8(Image,count);
	DoFontsFreetype(Image,count);

	DoPNG(Image,count);
	DoBMP(Image,count);
	DoJPEG(Image,count);
	DoGIF(Image,count);
	DoPPM(Image,count);
	DoTGA(Image,count);
	DoTIFF(Image,count);

	DoLoadIcons(Icons,count);

	DoAlphaBlt(Image,count,Icons);
	DoBlt(Image,count,Icons);
	DoDiffuseBlt(Image,count);
	DoColorKeyBlt(Image,count,Icons);
	DoButton(Image,count);
	BltClipping(Image,count,Icons);
	DoSprites(Image,count);


	Drawable Draw(Image,Rect(60,60,10,10));
	Draw.cls(Color(255,0,0));
	Image.putPixel(60,60,Color(255,255,255));
	Image.putPixel(70,70,Color(255,255,255));

	png.saveFile("tmp.grafixtest.png", Image);

	return 0;
}
