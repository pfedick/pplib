/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 108 $
 * $Date: 2011-05-20 16:55:16 +0200 (Fr, 20. Mai 2011) $
 * $Id: file.cpp 108 2011-05-20 14:55:16Z pafe $
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
#include <gtest/gtest.h>

using namespace ppl7;
using namespace ppl7::grafix;

namespace {

// The fixture for testing class Foo.
class GrafixDrawableTest : public ::testing::Test {
	protected:
	ppl7::grafix::Grafix gfx;
	ppl7::grafix::Image surface;
	Color transparent,black,white,red,green,blue;


	GrafixDrawableTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
		surface.create(640,480,ppl7::grafix::RGBFormat::A8R8G8B8);
		surface.cls();
		transparent.setColor(0,0,0,0);
		black.setColor(0,0,0,255);
		white.setColor(255,255,255,255);
		red.setColor(93,0,0,255);
		green.setColor(0,93,0,255);
		blue.setColor(0,0,93,255);


	}
	virtual ~GrafixDrawableTest() {

	}
};


TEST_F(GrafixDrawableTest, cls) {
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
	Color c[10];
	c[0]=transparent;
	c[1]=black;
	c[2]=white;
	c[3]=red;
	c[4]=green;
	c[5]=blue;
	for (int i=0;i<=5;i++) {
		Color color=c[i];
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


}	// EOF namespace

int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}


