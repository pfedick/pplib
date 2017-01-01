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
class GrafixRectTest : public ::testing::Test {
    protected:
        GrafixRectTest() {
        if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
            printf ("setlocale fehlgeschlagen\n");
            throw std::exception();
        }

    }
    virtual ~GrafixRectTest() {

    }
};


TEST_F(GrafixRectTest, constructor_withoutParams) {

    ASSERT_NO_THROW({
        ppl7::grafix::Rect r;
        ASSERT_EQ((int)0,r.x1);
        ASSERT_EQ((int)0,r.y1);
        ASSERT_EQ((int)0,r.x2);
        ASSERT_EQ((int)0,r.y2);
    });
}

TEST_F(GrafixRectTest, constructor_fromPoints) {
    ppl7::grafix::Point p1(10,20);
    ppl7::grafix::Point p2(60,70);

    ASSERT_NO_THROW({
        ppl7::grafix::Rect r(p1,p2);
        ASSERT_EQ((int)10,r.x1);
        ASSERT_EQ((int)20,r.y1);
        ASSERT_EQ((int)60,r.x2);
        ASSERT_EQ((int)70,r.y2);
    });
}

TEST_F(GrafixRectTest, constructor_fromPointAndSize) {
    ppl7::grafix::Point p1(10,20);
    ppl7::grafix::Size s1(50,50);

    ASSERT_NO_THROW({
        ppl7::grafix::Rect r(p1,s1);
        ASSERT_EQ((int)10,r.x1);
        ASSERT_EQ((int)20,r.y1);
        ASSERT_EQ((int)60,r.x2);
        ASSERT_EQ((int)70,r.y2);
    });
}

TEST_F(GrafixRectTest, constructor_fromIntegers) {

    ASSERT_NO_THROW({
        ppl7::grafix::Rect r(10,20,50,50);
        ASSERT_EQ((int)10,r.x1);
        ASSERT_EQ((int)20,r.y1);
        ASSERT_EQ((int)60,r.x2);
        ASSERT_EQ((int)70,r.y2);
    });
}

TEST_F(GrafixRectTest, constructor_fromOtherRect) {
    ppl7::grafix::Rect r1(10,20,50,50);
    ASSERT_NO_THROW({
        ppl7::grafix::Rect r(r1);
        ASSERT_EQ((int)10,r.x1);
        ASSERT_EQ((int)20,r.y1);
        ASSERT_EQ((int)60,r.x2);
        ASSERT_EQ((int)70,r.y2);
    });
}

TEST_F(GrafixRectTest, isNull) {
    ppl7::grafix::Rect r1;
    ASSERT_TRUE(r1.isNull());
    ppl7::grafix::Rect r2(10,20,0,0);
    ASSERT_TRUE(r2.isNull());

    ppl7::grafix::Rect r3(10,20,0,30);
    ASSERT_FALSE(r3.isNull());

    ppl7::grafix::Rect r4(10,20,20,0);
    ASSERT_FALSE(r4.isNull());

    ppl7::grafix::Rect r5(10,20,50,50);
    ASSERT_FALSE(r5.isNull());
}

TEST_F(GrafixRectTest, left) {
    ppl7::grafix::Rect r(10,20,50,50);
    ASSERT_EQ((int)10,r.left());
}

TEST_F(GrafixRectTest, right) {
    ppl7::grafix::Rect r(10,20,50,50);
    ASSERT_EQ((int)59,r.right());
}

TEST_F(GrafixRectTest, top) {
    ppl7::grafix::Rect r(10,20,50,50);
    ASSERT_EQ((int)20,r.top());
}

TEST_F(GrafixRectTest, bottom) {
    ppl7::grafix::Rect r(10,20,50,50);
    ASSERT_EQ((int)69,r.bottom());
}

TEST_F(GrafixRectTest, width) {
    ppl7::grafix::Rect r(10,20,51,52);
    ASSERT_EQ((int)51,r.width());
}

TEST_F(GrafixRectTest, height) {
    ppl7::grafix::Rect r(10,20,51,52);
    ASSERT_EQ((int)52,r.height());
}

TEST_F(GrafixRectTest, size) {
    ppl7::grafix::Rect r(10,20,51,52);
    ASSERT_EQ(ppl7::grafix::Size(51,52),r.size());
}

}	// EOF namespace

