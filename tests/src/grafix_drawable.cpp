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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include "../include/ppl7.h"
#include "../include/ppl7-grafix.h"
#include <gtest/gtest.h>
#include "ppl7-tests.h"

using namespace ppl7;
//using namespace ppl7::grafix;

namespace {

// The fixture for testing class Foo.
class GrafixDrawableTest : public ::testing::Test {
	protected:
    ppl7::grafix::Grafix *gfx;

    ppl7::grafix::Color transparent,black,white,red,green,blue;


	GrafixDrawableTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		gfx=NULL;
		transparent.setColor(0,0,0,0);
		black.setColor(0,0,0,255);
		white.setColor(255,255,255,255);
		red.setColor(93,0,0,255);
		green.setColor(0,93,0,255);
		blue.setColor(0,0,93,255);


	}
	virtual ~GrafixDrawableTest() {

	}
    virtual void SetUp() {
        gfx=new ppl7::grafix::Grafix();
    }
    virtual void TearDown() {
        delete gfx;
    }
};


TEST_F(GrafixDrawableTest, cls) {

    ppl7::grafix::Image surface(640,480,ppl7::grafix::RGBFormat::A8R8G8B8);
	surface.cls(transparent);
	EXPECT_EQ(transparent,surface.getPixel(0,0));
	EXPECT_EQ(transparent,surface.getPixel(333,111));
	EXPECT_EQ(transparent,surface.getPixel(639,479));

	surface.cls(black);
	EXPECT_EQ(black,surface.getPixel(0,0));
	EXPECT_EQ(black,surface.getPixel(333,111));
	EXPECT_EQ(black,surface.getPixel(639,479));

	surface.cls(white);
	EXPECT_EQ(white,surface.getPixel(0,0));
	EXPECT_EQ(white,surface.getPixel(333,111));
	EXPECT_EQ(white,surface.getPixel(639,479));

	surface.cls(red);
	EXPECT_EQ(red,surface.getPixel(0,0));
	EXPECT_EQ(red,surface.getPixel(333,111));
	EXPECT_EQ(red,surface.getPixel(639,479));

	surface.cls(green);
	EXPECT_EQ(green,surface.getPixel(0,0));
	EXPECT_EQ(green,surface.getPixel(333,111));
	EXPECT_EQ(green,surface.getPixel(639,479));

	surface.cls(blue);
	EXPECT_EQ(blue,surface.getPixel(0,0));
	EXPECT_EQ(blue,surface.getPixel(333,111));
	EXPECT_EQ(blue,surface.getPixel(639,479));
}

TEST_F(GrafixDrawableTest, lines) {
    ppl7::grafix::Image surface(640,480,ppl7::grafix::RGBFormat::A8R8G8B8);
    ppl7::grafix::Color c[10];
    surface.cls();
	c[0]=transparent;
	c[1]=black;
	c[2]=white;
	c[3]=red;
	c[4]=green;
	c[5]=blue;
	for (int i=0;i<=5;i++) {
        ppl7::grafix::Color color=c[i];
		int x=i*50;
		int y;

		// Senkrechte Linie
		y=25;
		surface.line(x,y-20,x,y+20,color);
		EXPECT_EQ(transparent,surface.getPixel(x,y-21));
		EXPECT_EQ(transparent,surface.getPixel(x-1,y-20));
		EXPECT_EQ(transparent,surface.getPixel(x+1,y-20));
		EXPECT_EQ(color,surface.getPixel(x,y-20));
		EXPECT_EQ(color,surface.getPixel(x,y));
		EXPECT_EQ(color,surface.getPixel(x,y+20));
		EXPECT_EQ(transparent,surface.getPixel(x,y+21));
		EXPECT_EQ(transparent,surface.getPixel(x-1,y+20));
		EXPECT_EQ(transparent,surface.getPixel(x+1,y+20));

		// Waagerechte linie
		y=50;
		surface.line(x-20,y,x+20,y,color);
		EXPECT_EQ(transparent,surface.getPixel(x-21,y));
		EXPECT_EQ(transparent,surface.getPixel(x-20,y-1));
		EXPECT_EQ(transparent,surface.getPixel(x-20,y+1));
		EXPECT_EQ(color,surface.getPixel(x-20,y));
		EXPECT_EQ(color,surface.getPixel(x,y));
		EXPECT_EQ(color,surface.getPixel(x+20,y));
		EXPECT_EQ(transparent,surface.getPixel(x+21,y));
		EXPECT_EQ(transparent,surface.getPixel(x+20,y-1));
		EXPECT_EQ(transparent,surface.getPixel(x+20,y+1));

		// x1
		y=75;
		surface.line(x-20,y-20,x+20,y+20,color);
		EXPECT_EQ(transparent,surface.getPixel(x-21,y-21));
		EXPECT_EQ(transparent,surface.getPixel(x-21,y-20));
		EXPECT_EQ(transparent,surface.getPixel(x-19,y-20));
		EXPECT_EQ(color,surface.getPixel(x-20,y-20));
		EXPECT_EQ(color,surface.getPixel(x,y));
		EXPECT_EQ(color,surface.getPixel(x+20,y+20));
		EXPECT_EQ(transparent,surface.getPixel(x+21,y+21));
		EXPECT_EQ(transparent,surface.getPixel(x+21,y+20));
		EXPECT_EQ(transparent,surface.getPixel(x+19,y+20));

		// x2
		y=75;
		surface.line(x+20,y-20,x-20,y+20,color);
		EXPECT_EQ(transparent,surface.getPixel(x+21,y-21));
		EXPECT_EQ(transparent,surface.getPixel(x+21,y-20));
		EXPECT_EQ(transparent,surface.getPixel(x+19,y-20));
		EXPECT_EQ(color,surface.getPixel(x+20,y-20));
		EXPECT_EQ(color,surface.getPixel(x,y));
		EXPECT_EQ(color,surface.getPixel(x-20,y+20));
		EXPECT_EQ(transparent,surface.getPixel(x-21,y+21));
		EXPECT_EQ(transparent,surface.getPixel(x-21,y+20));
		EXPECT_EQ(transparent,surface.getPixel(x-19,y+20));


		// x
		y=125;
		surface.line(x,y,x+5,y-20,color);
		EXPECT_EQ(color,surface.getPixel(x,y));
		EXPECT_EQ(color,surface.getPixel(x+5,y-20));

		y=175;
		surface.line(x,y,x+20,y-5,color);
		EXPECT_EQ(color,surface.getPixel(x,y));
		EXPECT_EQ(color,surface.getPixel(x+20,y-5));

		y=200;
		surface.line(x,y,x+20,y+7,color);
		EXPECT_EQ(color,surface.getPixel(x,y));
		EXPECT_EQ(color,surface.getPixel(x+20,y+7));

		y=225;
		surface.line(x,y,x+5,y+20,color);
		EXPECT_EQ(color,surface.getPixel(x,y));
		EXPECT_EQ(color,surface.getPixel(x+5,y+20));

		y=275;
		surface.line(x,y,x-7,y+20,color);
		EXPECT_EQ(color,surface.getPixel(x,y));
		EXPECT_EQ(color,surface.getPixel(x-7,y+20));

		y=325;
		surface.line(x,y,x-20,y+8,color);
		EXPECT_EQ(color,surface.getPixel(x,y));
		EXPECT_EQ(color,surface.getPixel(x-20,y+8));

		y=375;
		surface.line(x,y,x-20,y-7,color);
		EXPECT_EQ(color,surface.getPixel(x,y));
		EXPECT_EQ(color,surface.getPixel(x-20,y-7));

		y=425;
		surface.line(x,y,x-4,y-20,color);
		EXPECT_EQ(color,surface.getPixel(x,y));
		EXPECT_EQ(color,surface.getPixel(x-4,y-20));

	}

	/*
	ImageFilter_PNG png;
	png.saveFile("tmp.grafix.drawable.lines.png", surface);
	*/
}

void putPixels(ppl7::grafix::Drawable &draw, const ppl7::grafix::Color &background)
{
	ppl7::grafix::Color white(255,255,255,255);
	ppl7::grafix::Color red(255,0,0,255);
	ppl7::grafix::Color green(0,255,0,255);
	ppl7::grafix::Color blue(0,0,255,255);

	ppl7::grafix::Color half_white(255,255,255,127);
	ppl7::grafix::Color half_red(255,0,0,127);
	ppl7::grafix::Color half_green(0,255,0,127);
	ppl7::grafix::Color half_blue(0,0,255,127);

	draw.cls(background);

	for (int x=10;x<90;x++) {
		draw.putPixel(x,10,white);
		draw.putPixel(x,11,white);
		draw.putPixel(x,12,white);

		draw.putPixel(x,20,red);
		draw.putPixel(x,21,red);
		draw.putPixel(x,22,red);

		draw.putPixel(x,30,green);
		draw.putPixel(x,31,green);
		draw.putPixel(x,32,green);

		draw.putPixel(x,40,blue);
		draw.putPixel(x,41,blue);
		draw.putPixel(x,42,blue);

		draw.putPixel(x,50,half_white);
		draw.putPixel(x,51,half_white);
		draw.putPixel(x,52,half_white);

		draw.putPixel(x,60,half_red);
		draw.putPixel(x,61,half_red);
		draw.putPixel(x,62,half_red);

		draw.putPixel(x,70,half_green);
		draw.putPixel(x,71,half_green);
		draw.putPixel(x,72,half_green);

		draw.putPixel(x,80,half_blue);
		draw.putPixel(x,81,half_blue);
		draw.putPixel(x,82,half_blue);
	}
}

TEST_F(GrafixDrawableTest, pixelTest) {
	ppl7::grafix::Image surface(640,480,ppl7::grafix::RGBFormat::A8R8G8B8);
	surface.cls(ppl7::grafix::Color(0,0,0,255));
	ppl7::grafix::Image draw(100,100,ppl7::grafix::RGBFormat::A8R8G8B8);

	putPixels(draw, ppl7::grafix::Color(0,0,0,0));
	surface.blt(draw, 0,0);
	putPixels(draw, ppl7::grafix::Color(0,0,0,255));
	surface.blt(draw, 100,0);
	putPixels(draw, ppl7::grafix::Color(255,0,0,255));
	surface.blt(draw, 200,0);
	putPixels(draw, ppl7::grafix::Color(0,255,0,255));
	surface.blt(draw, 300,0);
	putPixels(draw, ppl7::grafix::Color(0,0,255,255));
	surface.blt(draw, 400,0);
	putPixels(draw, ppl7::grafix::Color(255,255,255,255));
	surface.blt(draw, 500,0);

	surface.fillRect(0,150,100,250,ppl7::grafix::Color(0,0,0,0));
	surface.fillRect(100,150,200,250,ppl7::grafix::Color(0,0,0,255));
	surface.fillRect(200,150,300,250,ppl7::grafix::Color(255,0,0,255));
	surface.fillRect(300,150,400,250,ppl7::grafix::Color(0,255,0,255));
	surface.fillRect(400,150,500,250,ppl7::grafix::Color(0,0,255,255));
	surface.fillRect(500,150,600,250,ppl7::grafix::Color(255,255,255,255));
	putPixels(draw, ppl7::grafix::Color(0,0,0,0));
	surface.bltAlpha(draw, 0,150);
	surface.bltAlpha(draw, 100,150);
	surface.bltAlpha(draw, 200,150);
	surface.bltAlpha(draw, 300,150);
	surface.bltAlpha(draw, 400,150);
	surface.bltAlpha(draw, 500,150);


	ppl7::grafix::ImageFilter_PNG png;
		png.saveFile("pixeltest.png", surface);
}


}	// EOF namespace


