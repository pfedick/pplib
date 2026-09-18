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
 *       documentation/ intellectual property materials provided with the distribution.
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

#include <pplib/core/file.h>
#include <pplib/core/memfile.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/exceptions.h>

#include "pplib-tests.h"

namespace
{

class FileObjectTest : public ::testing::Test
{
protected:
    FileObjectTest()
    {
    }
    virtual ~FileObjectTest()
    {
    }
};

TEST_F(FileObjectTest, WriteEmptyByteArrayPtrReturnsZero)
{
    pplib::MemFile f;
    pplib::ByteArrayPtr emptyPtr;
    ASSERT_EQ((size_t)0, f.write(emptyPtr));
    ASSERT_EQ((uint64_t)0, f.size());
}

TEST_F(FileObjectTest, ReadIntoByteArrayPreservesTargetWhenNotOpen)
{
    pplib::MemFile f;
    f.close();
    pplib::ByteArray target("Original Content 12345");
    size_t origSize = target.size();

    ASSERT_THROW(f.read(target, 50), pplib::FileNotOpenException);
    ASSERT_EQ(origSize, target.size());
    ASSERT_STREQ("Original Content 12345", (const char*)target.ptr());
}

TEST_F(FileObjectTest, CopyFromSelfThrowsIllegalArgumentException)
{
    const char data[] = "1234567890abcdef";
    pplib::MemFile f((void*)data, sizeof(data) - 1, false);

    ASSERT_THROW(f.copyFrom(f, 0, 5, 10), pplib::IllegalArgumentException);
    ASSERT_THROW(f.copyFrom(f, 5), pplib::IllegalArgumentException);
}

TEST_F(FileObjectTest, CopyFromAvailableUnderflowHandledGracefully)
{
    const char data[] = "Hello World";
    pplib::MemFile src((void*)data, 11, false);
    src.seek(11); // at EOF

    pplib::MemFile dst;
    uint64_t copied = dst.copyFrom(src, 20);
    ASSERT_EQ((uint64_t)0, copied);
}

TEST_F(FileObjectTest, CopyFromPartial)
{
    const char data[] = "0123456789";
    pplib::MemFile src((void*)data, 10, false);
    pplib::MemFile dst;

    uint64_t copied = dst.copyFrom(src, 2, 5, 0);
    ASSERT_EQ((uint64_t)5, copied);
    ASSERT_EQ((uint64_t)5, dst.size());

    pplib::ByteArray ba;
    dst.load(ba);
    ASSERT_EQ(0, memcmp(ba.ptr(), "23456", 5));
}

TEST_F(FileObjectTest, GetwsNoDoubleFree)
{
    wchar_t text[] = L"Test Zeile 1\nTest Zeile 2\n";
    pplib::MemFile f((void*)text, sizeof(text) - sizeof(wchar_t), false);

    pplib::WideString ws;
    int res = f.getws(ws, 100);
    ASSERT_EQ(1, res);
    ASSERT_EQ(pplib::WideString(L"Test Zeile 1\n"), ws);
}

} // namespace
