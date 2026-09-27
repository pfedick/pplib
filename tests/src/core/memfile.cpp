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
    ASSERT_THROW(f.map(0, 5, pplib::FileObject::MapProtection::READWRITE), pplib::ReadOnlyException);

    // Valid mapRW on writeable MemFile
    pplib::MemFile fw;
    fw.fwrite("WriteableMap", 1, 12);
    char* rwPtr = fw.map(0, 12, pplib::FileObject::MapProtection::READWRITE);
    ASSERT_NE(nullptr, rwPtr);
    rwPtr[0] = 'w';
    char readCheck[1];
    fw.seek(0);
    fw.fread(readCheck, 1, 1);
    EXPECT_EQ('w', readCheck[0]);
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

    // Reading with element size > remaining bytes returns 0 elements -> throws EndOfFileException
    f.seek(5);
    ASSERT_THROW(f.fread(buf, 10, 1), pplib::EndOfFileException);
}

TEST_F(MemFileTest, FwriteNullAndZeroArgs)
{
    pplib::MemFile f;
    ASSERT_EQ((size_t)0, f.fwrite(nullptr, 0, 10));
    ASSERT_EQ((size_t)0, f.fwrite(nullptr, 1, 0));
    ASSERT_THROW(f.fwrite(nullptr, 1, 10), pplib::IllegalArgumentException);
}

TEST_F(MemFileTest, ConstructorAndOpenWithByteArrayPtr)
{
    const char text[] = "ByteArrayPtrTestData";
    pplib::ByteArray ba(text, sizeof(text));

    // Valid constructor
    pplib::MemFile f1(ba);
    EXPECT_TRUE(f1.isOpen());
    EXPECT_EQ(sizeof(text), f1.size());
    EXPECT_THROW(f1.fwrite("x", 1, 1), pplib::ReadOnlyException);

    char readBuf[sizeof(text)];
    EXPECT_EQ(1u, f1.fread(readBuf, sizeof(text), 1));
    EXPECT_EQ(0, memcmp(text, readBuf, sizeof(text)));

    // Empty ByteArrayPtr in constructor throws
    pplib::ByteArrayPtr emptyPtr;
    EXPECT_THROW(pplib::MemFile fEmpty(emptyPtr), pplib::IllegalArgumentException);

    // open(ByteArrayPtr)
    pplib::MemFile f2;
    f2.open(ba);
    EXPECT_TRUE(f2.isOpen());
    EXPECT_EQ(sizeof(text), f2.size());

    // open with empty throws
    EXPECT_THROW(f2.open(emptyPtr), pplib::IllegalArgumentException);
}

TEST_F(MemFileTest, OpenReadWrite)
{
    // openReadWrite assumes ownership and frees the buffer in close()
    char* buffer = (char*)malloc(32);
    ASSERT_NE(nullptr, buffer);
    memset(buffer, 0, 32);

    pplib::MemFile f;
    f.openReadWrite(buffer, 32);
    EXPECT_TRUE(f.isOpen());

    const char sample[] = "ReadWriteTest";
    EXPECT_EQ(sizeof(sample), f.fwrite(sample, 1, sizeof(sample)));
    EXPECT_EQ(0, memcmp(buffer, sample, sizeof(sample)));
}

TEST_F(MemFileTest, SetMaxSizeAndExceedLimit)
{
    pplib::MemFile f;
    f.setMaxSize(30);
    EXPECT_EQ(30u, f.maxSize());

    char data20[20] = {0};
    EXPECT_EQ(20u, f.fwrite(data20, 1, 20));

    char data15[15] = {0};
    // Exceeds maxsize (20 + 15 = 35 > 30) -> throws BufferExceedsLimitException
    EXPECT_THROW(f.fwrite(data15, 1, 15), pplib::BufferExceedsLimitException);

    // Unlimit
    f.setMaxSize(0);
    EXPECT_EQ(0u, f.maxSize());
    EXPECT_NO_THROW(f.fwrite(data15, 1, 15));
    EXPECT_EQ(35u, f.size());
}

TEST_F(MemFileTest, FgetsReadsLinesAndEof)
{
    const char text[] = "FirstLine\nSecondLine\nThirdNoNewline";
    pplib::MemFile f((void*)text, strlen(text), false);

    char buf[64];
    ASSERT_NE(nullptr, f.fgets(buf, sizeof(buf)));
    EXPECT_STREQ("FirstLine\n", buf);

    ASSERT_NE(nullptr, f.fgets(buf, sizeof(buf)));
    EXPECT_STREQ("SecondLine\n", buf);

    ASSERT_NE(nullptr, f.fgets(buf, sizeof(buf)));
    EXPECT_STREQ("ThirdNoNewline", buf);

    EXPECT_THROW(f.fgets(buf, sizeof(buf)), pplib::EndOfFileException);
}

TEST_F(MemFileTest, FputsAndFputws)
{
    pplib::MemFile f;
    f.fputs("Hello String\n");
    EXPECT_GT(f.tell(), 0u);

    f.fputws(L"Wide String\n");
    EXPECT_GT(f.size(), 13u);

    // Test fputc and fputwc
    f.fputc('A');
    f.fputwc(L'B');
    EXPECT_EQ((uint64_t)(13 + 12 * sizeof(wchar_t) + 1 + sizeof(wchar_t)), f.size());
}

TEST_F(MemFileTest, RewindAndTell)
{
    pplib::MemFile f;
    f.fwrite("12345678", 1, 8);
    EXPECT_EQ(8u, f.tell());

    f.rewind();
    EXPECT_EQ(0u, f.tell());
}

TEST_F(MemFileTest, AdrFunctionality)
{
    const char text[] = "TestingAdr";
    pplib::MemFile f((void*)text, sizeof(text), false);

    EXPECT_EQ((const void*)text, f.adr(0));
    EXPECT_EQ((const void*)(text + 5), f.adr(5));

    // Closed file throws
    f.close();
    EXPECT_THROW(f.adr(0), pplib::FileNotOpenException);
}

TEST_F(MemFileTest, TruncateExpandAndShrink)
{
    pplib::MemFile f;
    f.fwrite("0123456789", 1, 10);
    EXPECT_EQ(10u, f.size());

    // Truncate to same length
    f.truncate(10);
    EXPECT_EQ(10u, f.size());

    // Shrink
    f.truncate(5);
    EXPECT_EQ(5u, f.size());

    // Expand
    f.truncate(12);
    EXPECT_EQ(12u, f.size());

    // Verify expanded bytes are zeroed
    f.seek(5);
    char zeroBuf[7] = {1, 1, 1, 1, 1, 1, 1};
    f.fread(zeroBuf, 1, 7);
    for (int i = 0; i < 7; ++i) {
        EXPECT_EQ(0, zeroBuf[i]);
    }
}

TEST_F(MemFileTest, ClosedFileThrowsFileNotOpenException)
{
    const char data[] = "Hello";
    pplib::MemFile f((void*)data, 5, false);
    f.close();

    char buf[10];
    wchar_t wbuf[10];
    EXPECT_THROW(f.fread(buf, 1, 1), pplib::FileNotOpenException);
    EXPECT_THROW(f.fgetc(), pplib::FileNotOpenException);
    EXPECT_THROW(f.fgetwc(), pplib::FileNotOpenException);
    EXPECT_THROW(f.fgets(buf, sizeof(buf)), pplib::FileNotOpenException);
    EXPECT_THROW(f.fgetws(wbuf, 10), pplib::FileNotOpenException);
    EXPECT_THROW(f.map(0, 1), pplib::FileNotOpenException);
    EXPECT_THROW(f.adr(0), pplib::FileNotOpenException);
    EXPECT_THROW(f.seek(0), pplib::FileNotOpenException);
    EXPECT_THROW(f.seek(0, pplib::FileObject::SEEKSET), pplib::FileNotOpenException);
    EXPECT_THROW(f.tell(), pplib::FileNotOpenException);
    EXPECT_THROW(f.eof(), pplib::FileNotOpenException);
    EXPECT_THROW(f.fputs("test"), pplib::FileNotOpenException);
    EXPECT_THROW(f.fputws(L"test"), pplib::FileNotOpenException);
    EXPECT_THROW(f.fwrite("test", 1, 4), pplib::FileNotOpenException);
}

TEST_F(MemFileTest, MoveConstructorAndAssignment)
{
    pplib::MemFile f1;
    f1.fwrite("HelloMove", 1, 9);
    EXPECT_EQ(9u, f1.size());
    EXPECT_TRUE(f1.isOpen());

    // Move constructor
    pplib::MemFile f2(std::move(f1));
    EXPECT_EQ(9u, f2.size());
    EXPECT_TRUE(f2.isOpen());
    EXPECT_FALSE(f1.isOpen());
    EXPECT_EQ(0u, f1.size());

    char buf[10] = {0};
    f2.rewind();
    EXPECT_EQ(9u, f2.fread(buf, 1, 9));
    EXPECT_STREQ("HelloMove", buf);

    // Move assignment
    pplib::MemFile f3;
    f3 = std::move(f2);
    EXPECT_EQ(9u, f3.size());
    EXPECT_TRUE(f3.isOpen());
    EXPECT_FALSE(f2.isOpen());

    // Self-move assignment
    f3 = std::move(f3);
    EXPECT_EQ(9u, f3.size());
    EXPECT_TRUE(f3.isOpen());
}

TEST_F(MemFileTest, InvalidArgumentsForStringIO)
{
    const char text[] = "Line1\nLine2";
    pplib::MemFile f((void*)text, strlen(text), false);

    char buf[32];
    wchar_t wbuf[32];

    // fgets
    EXPECT_THROW(f.fgets(nullptr, sizeof(buf)), pplib::IllegalArgumentException);
    EXPECT_THROW(f.fgets(buf, 0), pplib::IllegalArgumentException);

    // fgetws
    EXPECT_THROW(f.fgetws(nullptr, sizeof(wbuf)), pplib::IllegalArgumentException);
    EXPECT_THROW(f.fgetws(wbuf, 0), pplib::IllegalArgumentException);

    // fputs / fputws
    pplib::MemFile fw;
    EXPECT_THROW(fw.fputs(nullptr), pplib::IllegalArgumentException);
    EXPECT_THROW(fw.fputws(nullptr), pplib::IllegalArgumentException);

    // fwrite size overflow
    EXPECT_THROW(fw.fwrite("x", SIZE_MAX, 2), pplib::OverflowException);
}

TEST_F(MemFileTest, EofBehavior)
{
    const char data[] = "12345";
    pplib::MemFile f((void*)data, 5, false);

    EXPECT_FALSE(f.eof());
    f.seek(4);
    EXPECT_FALSE(f.eof());
    f.seek(5);
    EXPECT_TRUE(f.eof());
}

TEST_F(MemFileTest, UnsupportedAndNoOpOperations)
{
    const char data[] = "data";
    pplib::MemFile f((void*)data, 4, false);

    // Unsupported operations
    EXPECT_THROW(f.getFileNo(), pplib::OperationUnavailableException);
    EXPECT_THROW(f.lockShared(), pplib::OperationUnavailableException);
    EXPECT_THROW(f.lockExclusive(), pplib::OperationUnavailableException);
    EXPECT_THROW(f.unlock(), pplib::OperationUnavailableException);

    // No-op operations
    EXPECT_NO_THROW(f.unmap());
    EXPECT_NO_THROW(f.flush());
    EXPECT_NO_THROW(f.sync());
    EXPECT_NO_THROW(f.setMapReadAhead(4096));
}

TEST_F(MemFileTest, ReadOnlyEnforcement)
{
    const char data[] = "ReadOnly";
    pplib::MemFile f((void*)data, 8, false);

    EXPECT_THROW(f.fwrite("a", 1, 1), pplib::ReadOnlyException);
    EXPECT_THROW(f.truncate(4), pplib::ReadOnlyException);
    EXPECT_THROW(f.fputc('x'), pplib::ReadOnlyException);
    EXPECT_THROW(f.fputwc(L'x'), pplib::ReadOnlyException);
}

} // namespace
