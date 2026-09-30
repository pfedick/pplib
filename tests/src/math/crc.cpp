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

#include <pplib/core/functions.h>

#include "pplib-tests.h"

namespace
{

TEST(CrcTest, Crc32)
{
    const char* input = "123456789";
    uint32_t crc = pplib::Crc32(input, strlen(input));
    ASSERT_EQ(0xCBF43926, crc);
}

TEST(CrcTest, Crc32Context)
{
    const char* input = "123456789";
    pplib::Crc32Context ctx;
    ctx.update(input, strlen(input));
    ASSERT_EQ(0xCBF43926, ctx.get());
    ctx.update("abcdefghijklmnop", 16);
    ASSERT_EQ(0xE0DFBC17, ctx.get());
    ctx.reset();
    ASSERT_EQ(0xFFFFFFFF ^ 0xFFFFFFFF, ctx.get());
    ctx.update(input, strlen(input));
    ASSERT_EQ(0xCBF43926, ctx.get());
}

TEST(CrcTest, Crc16)
{
    const char* input = "123456789";
    uint32_t crc = pplib::Crc16(input, strlen(input));
    ASSERT_EQ(0x29B1, crc);
    ASSERT_EQ(0x29B1, pplib::Crc16("56789", 5, pplib::Crc16("1234", 4)));
}

TEST(CrcTest, Crc16Context)
{
    const char* input = "123456789";
    pplib::Crc16Context ctx;
    ctx.update(input, strlen(input));
    ASSERT_EQ(0x29B1, ctx.get());
    ctx.update("56789", 5);
    ASSERT_EQ(0x6817, ctx.get());
    ctx.reset();
    ASSERT_EQ(0xFFFF, ctx.get());
    ctx.update(input, strlen(input));
    ASSERT_EQ(0x29B1, ctx.get());
}

} // namespace