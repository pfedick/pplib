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

#include <pplib/types/string.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/core/functions.h>
#include <pplib/core/file.h>

#include "pplib-tests.h"

namespace
{

TEST(ByteArrayTest, ConstructorSimple)
{
    ASSERT_NO_THROW({
        pplib::ByteArray b1;
        ASSERT_TRUE(b1.isNull() == true) << "Array is not empty";
    });
}

TEST(ByteArrayTest, CopyConstructorWithEmptyObject)
{
    pplib::ByteArray b1;
    ASSERT_NO_THROW({ pplib::ByteArray b2(b1); });
}

TEST(ByteArrayTest, AssignmentWithEmptyObject)
{
    pplib::ByteArray b1;
    ASSERT_NO_THROW({ pplib::ByteArray b2 = b1; });
    ASSERT_NO_THROW({
        pplib::ByteArray b3;
        b3 = b1;
    });

    pplib::ByteArray b4;
    b4 = b1;
}

TEST(ByteArrayTest, load_and_base64)
{
    pplib::ByteArray ba;
    pplib::File::load(ba, "testdata/screenshot1.png");
    ASSERT_EQ((size_t)59627, ba.size());
    pplib::String ba64 = ba.toBase64();
    pplib::ByteArray ba2 = pplib::FromBase64(ba64);
    ASSERT_EQ((size_t)59627, ba2.size());
}

// memBlock1 > MemBlock2
// memBlock2 < MemBlock1
static unsigned char memBlock1[] = {123, 77, 42, 255, 192, 16, 9};
static unsigned char memBlock2[] = {123, 77, 42, 200, 192, 16, 9};

TEST(ByteArrayTest, operators_equal)
{
    pplib::ByteArrayPtr b1(memBlock1, sizeof(memBlock1));
    pplib::ByteArrayPtr b2(memBlock1, sizeof(memBlock1));
    ASSERT_FALSE(b1 < b2);
    ASSERT_TRUE(b1 <= b2);
    ASSERT_TRUE(b1 == b2);
    ASSERT_FALSE(b1 != b2);
    ASSERT_TRUE(b1 >= b2);
    ASSERT_FALSE(b1 > b2);
}

TEST(ByteArrayTest, operators_lower)
{
    pplib::ByteArrayPtr b1(memBlock2, sizeof(memBlock2));
    pplib::ByteArrayPtr b2(memBlock1, sizeof(memBlock1));
    ASSERT_TRUE(b1 < b2);
    ASSERT_TRUE(b1 <= b2);
    ASSERT_FALSE(b1 == b2);
    ASSERT_TRUE(b1 != b2);
    ASSERT_FALSE(b1 >= b2);
    ASSERT_FALSE(b1 > b2);
}

TEST(ByteArrayTest, operators_greater)
{
    pplib::ByteArrayPtr b1(memBlock1, sizeof(memBlock1));
    pplib::ByteArrayPtr b2(memBlock2, sizeof(memBlock2));
    ASSERT_FALSE(b1 < b2);
    ASSERT_FALSE(b1 <= b2);
    ASSERT_FALSE(b1 == b2);
    ASSERT_TRUE(b1 != b2);
    ASSERT_TRUE(b1 >= b2);
    ASSERT_TRUE(b1 > b2);
}

} // namespace
