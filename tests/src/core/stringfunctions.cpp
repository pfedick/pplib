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
#include <pplib/types/widestring.h>
#include <pplib/types/array.h>
#include <pplib/types/bytearray.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

#include "pplib-tests.h"

namespace
{

// The fixture for testing class Foo.
class StringFunctionTest : public ::testing::Test
{
protected:
    StringFunctionTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~StringFunctionTest()
    {
    }
};

// 0d,0e,37,42,81,ff,42,00,4c,17,12
static unsigned char binarydata[] = {13, 14, 55, 66, 129, 255, 66, 0, 76, 23, 18};

TEST_F(StringFunctionTest, ToBase64)
{
    pplib::ByteArrayPtr b2;
    b2.use(binarydata, 7);
    EXPECT_EQ(pplib::String("DQ43QoH/Qo=="), ToBase64(b2));
    b2.use(binarydata, 8);
    EXPECT_EQ(pplib::String("DQ43QoH/QgD="), ToBase64(b2));
    b2.use(binarydata, 9);
    EXPECT_EQ(pplib::String("DQ43QoH/QgBM"), ToBase64(b2));
    b2.use(binarydata, 10);
    EXPECT_EQ(pplib::String("DQ43QoH/QgBMFw=="), ToBase64(b2));
    b2.use(binarydata, 11);
    EXPECT_EQ(pplib::String("DQ43QoH/QgBMFxJ="), ToBase64(b2));
}

TEST_F(StringFunctionTest, FromBase64)
{
    pplib::ByteArrayPtr b1;
    try {
        b1.use(binarydata, 7);
        EXPECT_EQ(b1, pplib::FromBase64(pplib::String("DQ43QoH/Qo==")));
        b1.use(binarydata, 8);
        EXPECT_EQ(b1, pplib::FromBase64(pplib::String("DQ43QoH/QgD=")));
        b1.use(binarydata, 9);
        EXPECT_EQ(b1, pplib::FromBase64(pplib::String("DQ43QoH/QgBM")));
        b1.use(binarydata, 10);
        EXPECT_EQ(b1, pplib::FromBase64(pplib::String("DQ43QoH/QgBMFw==")));
        b1.use(binarydata, 11);
        EXPECT_EQ(b1, pplib::FromBase64(pplib::String("DQ43QoH/QgBMFxJ=")));
    }
    catch (const pplib::Exception& e) {
        e.print();
    }
    catch (const std::exception& e) {
        printf("std::exception: %s\n", e.what());
    }
}

TEST_F(StringFunctionTest, UpperCaseWords)
{
    pplib::String s1("the quick brown fox jumps over äöü");
    pplib::String expected("The Quick Brown Fox Jumps Over Äöü");
    pplib::String result = UpperCaseWords(s1);
    ASSERT_EQ(expected, result);
}

TEST_F(StringFunctionTest, StripSlashes)
{
    EXPECT_EQ(pplib::String(""), pplib::StripSlashes(pplib::String("")));
    EXPECT_EQ(pplib::String("Hallo Welt"), pplib::StripSlashes(pplib::String("Hallo Welt")));
    EXPECT_EQ(pplib::String("Hallo Welt"), pplib::StripSlashes(pplib::String("Hallo\\ Welt")));
    EXPECT_EQ(pplib::String("Hallon Welt"), pplib::StripSlashes(pplib::String("Hallo\\n Welt")));
    EXPECT_EQ(pplib::String("Hallo\\Welt"), pplib::StripSlashes(pplib::String("Hallo\\\\Welt")));
    EXPECT_EQ(pplib::String("Hallo Welt"), pplib::StripSlashes(pplib::String("\\Hallo Welt")));
    EXPECT_EQ(pplib::String("Hallo Welt"), pplib::StripSlashes(pplib::String("Hallo Welt\\")));
}

// ByteArray fromBase64(const String &base64);

} // namespace
