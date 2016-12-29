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

namespace {

// The fixture for testing class Foo.
class GrafixImageTest : public ::testing::Test {
protected:
    ppl7::grafix::Grafix *gfx;
    GrafixImageTest() {
        if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
            printf ("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
        gfx=NULL;


    }
    virtual ~GrafixImageTest() {
    }
    virtual void SetUp() {
        gfx=new ppl7::grafix::Grafix();
    }
    virtual void TearDown() {
        delete gfx;
    }
};


TEST_F(GrafixImageTest, constructor_without_parameter) {
    ASSERT_NO_THROW({
        ppl7::grafix::Image img;
    });
}

TEST_F(GrafixImageTest, constructor_create) {
    ASSERT_NO_THROW({
        ppl7::grafix::Image img(320,200);
        ASSERT_EQ((int)320,img.width());
        ASSERT_EQ((int)200,img.height());
        ASSERT_EQ((int)32,img.bitdepth());
        ASSERT_EQ(ppl7::grafix::RGBFormat::A8R8G8B8,img.rgbformat());
    });
}

TEST_F(GrafixImageTest, create) {
    ppl7::grafix::Image img;
    ASSERT_NO_THROW({
        img.create(320,200);
    });
    ASSERT_EQ((int)320,img.width());
    ASSERT_EQ((int)200,img.height());
    ASSERT_EQ((int)32,img.bitdepth());
    ASSERT_EQ(ppl7::grafix::RGBFormat::A8R8G8B8,img.rgbformat());

}

}	// EOF namespace

