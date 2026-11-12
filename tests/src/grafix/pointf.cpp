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

#include <gtest/gtest.h>

#include <cstdio>
#include <string>
#include <type_traits>

#include <pplib/types/string.h>
#include <pplib/grafix/point.h>
#include <pplib/grafix/rect.h>
#include "pplib-tests.h"

namespace
{

TEST(GrafixPointFTest, DefaultConstructors)
{
    ASSERT_NO_THROW({
        pplib::grafix::PointF p1;
        EXPECT_EQ(0.0f, p1.x);
        EXPECT_EQ(0.0f, p1.y);
    });
    pplib::grafix::PointF p2(19.123, 42.34);
    EXPECT_EQ(19.123f, p2.x);
    EXPECT_EQ(42.34f, p2.y);

    // Copy
    pplib::grafix::PointF p3(p2);
    EXPECT_EQ(19.123f, p3.x);
    EXPECT_EQ(42.34f, p3.y);
    // Move

    pplib::grafix::PointF p4(std::move(p3));
    EXPECT_EQ(19.123f, p4.x);
    EXPECT_EQ(42.34f, p4.y);
}

} // namespace
