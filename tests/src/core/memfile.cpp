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
 *       documentation/intellectual property materials provided with the distribution.
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

#include <pplib/core/memfile.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/exceptions.h>

#include "pplib-tests.h"

namespace
{

class MemFileTest : public ::testing::Test
{
protected:
    MemFileTest()
    {
    }
    virtual ~MemFileTest()
    {
    }
};

TEST_F(MemFileTest, SeekUint64)
{
    const char data[] = "0123456789";
    pplib::MemFile f((void*)data, 10, false);

    ASSERT_NO_THROW(f.seek((uint64_t)0));
    ASSERT_EQ((uint64_t)0, f.tell());

    ASSERT_NO_THROW(f.seek((uint64_t)5));
    ASSERT_EQ((uint64_t)5, f.tell());

    // Seeking exactly to EOF is legal
    ASSERT_NO_THROW(f.seek((uint64_t)10));
    ASSERT_EQ((uint64_t)10, f.tell());

    // Seeking past EOF throws OverflowException
    ASSERT_THROW(f.seek((uint64_t)11), pplib::OverflowException);
}

TEST_F(MemFileTest, SeekOriginSeekSet)
{
    const char data[] = "0123456789";
    pplib::MemFile f((void*)data, 10, false);

    ASSERT_EQ((uint64_t)0, f.seek(0, pplib::FileObject::SEEKSET));
    ASSERT_EQ((uint64_t)0, f.tell());

    ASSERT_EQ((uint64_t)5, f.seek(5, pplib::FileObject::SEEKSET));
    ASSERT_EQ((uint64_t)5, f.tell());

    ASSERT_EQ((uint64_t)10, f.seek(10, pplib::FileObject::SEEKSET));
    ASSERT_EQ((uint64_t)10, f.tell());

    ASSERT_THROW(f.seek(11, pplib::FileObject::SEEKSET), pplib::FileSeekException);
    ASSERT_THROW(f.seek(-1, pplib::FileObject::SEEKSET), pplib::InvalidArgumentsException);
}

TEST_F(MemFileTest, SeekOriginSeekCur)
{
    const char data[] = "0123456789";
    pplib::MemFile f((void*)data, 10, false);

    f.seek(5);
    ASSERT_EQ((uint64_t)7, f.seek(2, pplib::FileObject::SEEKCUR));
    ASSERT_EQ((uint64_t)3, f.seek(-4, pplib::FileObject::SEEKCUR));

    // Seek to exactly EOF via SEEKCUR
    ASSERT_EQ((uint64_t)10, f.seek(7, pplib::FileObject::SEEKCUR));

    // Seek beyond EOF
    ASSERT_THROW(f.seek(1, pplib::FileObject::SEEKCUR), pplib::FileSeekException);

    // Seek before beginning
    ASSERT_THROW(f.seek(-11, pplib::FileObject::SEEKCUR), pplib::InvalidArgumentsException);
}

TEST_F(MemFileTest, SeekOriginSeekEnd)
{
    const char data[] = "0123456789";
    pplib::MemFile f((void*)data, 10, false);

    // offset 0 from end is exact EOF
    ASSERT_EQ((uint64_t)10, f.seek(0, pplib::FileObject::SEEKEND));
    ASSERT_EQ((uint64_t)10, f.tell());

    // offset -5 is position 5
    ASSERT_EQ((uint64_t)5, f.seek(-5, pplib::FileObject::SEEKEND));
    ASSERT_EQ((uint64_t)5, f.tell());

    // offset -10 is position 0
    ASSERT_EQ((uint64_t)0, f.seek(-10, pplib::FileObject::SEEKEND));
    ASSERT_EQ((uint64_t)0, f.tell());

    // offset -11 is before beginning
    ASSERT_THROW(f.seek(-11, pplib::FileObject::SEEKEND), pplib::InvalidArgumentsException);

    // offset 1 is beyond EOF
    ASSERT_THROW(f.seek(1, pplib::FileObject::SEEKEND), pplib::FileSeekException);
}

TEST_F(MemFileTest, SeekInvalidOrigin)
{
    const char data[] = "0123456789";
    pplib::MemFile f((void*)data, 10, false);
    ASSERT_THROW(f.seek(0, (pplib::FileObject::SeekOrigin)99), pplib::IllegalArgumentException);
}

TEST_F(MemFileTest, FgetcThrowsEndOfFileException)
{
    const char data[] = "AB";
    pplib::MemFile f((void*)data, 2, false);

    ASSERT_EQ('A', f.fgetc());
    ASSERT_EQ('B', f.fgetc());
    ASSERT_THROW(f.fgetc(), pplib::EndOfFileException);
}

TEST_F(MemFileTest, FgetwcThrowsEndOfFileException)
{
    const wchar_t data[] = L"AB";
    pplib::MemFile f((void*)data, 2 * sizeof(wchar_t), false);

    ASSERT_EQ(L'A', f.fgetwc());
    ASSERT_EQ(L'B', f.fgetwc());
    ASSERT_THROW(f.fgetwc(), pplib::EndOfFileException);
}

TEST_F(MemFileTest, FgetwsClampingAndIllegalArgument)
{
    const wchar_t data[] = L"Hello\nWorld";
    pplib::MemFile f((void*)data, sizeof(data) - sizeof(wchar_t), false);

    wchar_t buf[64];
    ASSERT_THROW(f.fgetws(buf, 0), pplib::IllegalArgumentException);

    ASSERT_NE(nullptr, f.fgetws(buf, 64));
    ASSERT_STREQ(L"Hello\n", buf);

    ASSERT_NE(nullptr, f.fgetws(buf, 64));
    ASSERT_STREQ(L"World", buf);

    ASSERT_THROW(f.fgetws(buf, 64), pplib::EndOfFileException);
}

TEST_F(MemFileTest, MapAndMapRW)
{
    const char data[] = "0123456789";
    pplib::MemFile f((void*)data, 10, false);

    // Valid map
    const char* ptr = f.map(0, 10);
    ASSERT_NE(nullptr, ptr);
    ASSERT_EQ('0', *ptr);

    // Out of bounds map
    ASSERT_THROW(f.map(5, 6), pplib::OverflowException);
    ASSERT_THROW(f.map(11, 1), pplib::OverflowException);
    ASSERT_THROW(f.map(UINT64_MAX - 5, 10), pplib::OverflowException);

    // mapRW on read-only throws ReadOnlyException
    ASSERT_THROW(f.mapRW(0, 5), pplib::ReadOnlyException);
}

TEST_F(MemFileTest, DynamicWriteAndClose)
{
    pplib::MemFile f;
    f.fwrite("Test1234", 1, 8);
    ASSERT_EQ((uint64_t)8, f.size());
    ASSERT_TRUE(f.isOpen());

    f.close();
    ASSERT_FALSE(f.isOpen());
    ASSERT_EQ((uint64_t)0, f.size());
}

TEST_F(MemFileTest, FreadPosixPartialRead)
{
    const char data[] = "1234567"; // 7 Bytes
    pplib::MemFile f((void*)data, 7, false);

    char buf[10];
    // Asking for 10 elements of size 1: returns 7 (partial read), does not throw
    size_t by = f.fread(buf, 1, 10);
    ASSERT_EQ((size_t)7, by);
    ASSERT_EQ(0, memcmp(buf, "1234567", 7));

    // Next read at EOF throws EndOfFileException
    ASSERT_THROW(f.fread(buf, 1, 10), pplib::EndOfFileException);
}

TEST_F(MemFileTest, FwriteNullAndZeroArgs)
{
    pplib::MemFile f;
    ASSERT_EQ((size_t)0, f.fwrite(nullptr, 0, 10));
    ASSERT_EQ((size_t)0, f.fwrite(nullptr, 1, 0));
    ASSERT_THROW(f.fwrite(nullptr, 1, 10), pplib::IllegalArgumentException);
}

} // namespace
