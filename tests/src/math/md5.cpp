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
#include <pplib/exceptions.h>
#include <pplib/core/file.h>
#include <pplib/core/dir.h>
#include <pplib-crypto.h>
#include <pplib/core/functions.h>
// #include <pplib/core/regex.h>
#include <pplib/exceptions.h>

#include "pplib-tests.h"

namespace
{

class Md5Test : public ::testing::Test
{
protected:
    Md5Test()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~Md5Test()
    {
    }
};

TEST_F(Md5Test, Md5WithByteArrayPtr)
{
    pplib::ByteArrayPtr bap("Hello World", 11);
    ASSERT_EQ(pplib::String("b10a8db164e0754105b7a99be72e3fe5"), pplib::Md5(bap));
}

TEST_F(Md5Test, Md5WithPtr)
{
    pplib::ByteArrayPtr bap("Hello World", 11);
    ASSERT_EQ(pplib::String("b10a8db164e0754105b7a99be72e3fe5"), pplib::Md5(bap.adr(), bap.size()));

    ASSERT_EQ(pplib::String("901736df3fbc807121c46f9eaed8ff28"), pplib::Md5(loremipsum, strlen(loremipsum)));
    ASSERT_EQ(pplib::String("d41d8cd98f00b204e9800998ecf8427e"), pplib::Md5(nullptr, strlen(loremipsum)));
    ASSERT_EQ(pplib::String("d41d8cd98f00b204e9800998ecf8427e"), pplib::Md5(loremipsum, 0));
}

} // namespace