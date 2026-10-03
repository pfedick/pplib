/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <gtest/gtest.h>
#include "pplib-tests.h"

#include <pplib/types/string.h>

#include <pplib/core/time.h>
#include <pplib/core/timer.h>
#include <pplib/exceptions.h>

namespace
{

TEST(TimerTest, Constructor)
{
    pplib::Timer t;
    ASSERT_EQ((double)0.0f, t.duration());
    pplib::Sleep(0.1f);
    ASSERT_EQ((double)0.0f, t.duration());
    ASSERT_GT(t.currentDuration(), 0.09f);
}

TEST(TimerTest, startStopReset)
{
    pplib::Timer t;
    t.start();
    pplib::Sleep(0.1);
    t.stop();
    ASSERT_GT(t.duration(), 0.09);
    t.start();
    pplib::Sleep(0.1);
    ASSERT_GT(t.stop(), 0.19);
    t.reset();
    ASSERT_EQ((double)0.0, t.duration());
}

TEST(TimerTest, addDuration)
{
    pplib::Timer t1;
    t1.addDuration(0.1);
    ASSERT_EQ((double)0.1, t1.duration());
}

TEST(TimerTest, operatorAdd)
{
    pplib::Timer t1;
    t1.addDuration(0.1);
    pplib::Timer t2;
    t2.addDuration(0.2);
    pplib::Timer t3 = t1 + t2;
    ASSERT_EQ((double)0.1, t1.duration());
    ASSERT_EQ((double)0.2, t2.duration());
    ASSERT_DOUBLE_EQ((double)0.3, t3.duration());
}

TEST(TimerTest, operatorPlusEqual)
{
    pplib::Timer t1;
    t1.addDuration(0.1);
    pplib::Timer t2;
    t2.addDuration(0.2);
    t1 += t2;
    ASSERT_DOUBLE_EQ((double)0.3, t1.duration());
    ASSERT_EQ((double)0.2, t2.duration());
}

TEST(TimerTest, operatorPlusEqualWithDouble)
{
    pplib::Timer t1;
    t1.addDuration(0.1);
    t1 += 0.2;
    ASSERT_DOUBLE_EQ((double)0.3, t1.duration());
}

} // namespace
