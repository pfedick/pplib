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
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

#include "pplib-tests.h"

namespace
{

TEST(ByteArrayPtrTest, ConstructorSimple)
{
    ASSERT_NO_THROW({
        pplib::ByteArrayPtr b1;
        ASSERT_TRUE(b1.isNull() == true) << "Array is not empty";
    });
}

TEST(ByteArrayPtrTest, ConstructorFromPointerAndSize)
{
    void* adr = this;
    const void* adr2 = this;
    size_t size = 1234567;

    ASSERT_NO_THROW({
        pplib::ByteArrayPtr b1(adr, size);
        ASSERT_FALSE(b1.isNull()) << "Array is not empty";
        ASSERT_EQ(adr, b1.ptr());
        ASSERT_EQ(size, b1.size());
    });

    ASSERT_NO_THROW({
        pplib::ByteArrayPtr b1(adr2, size);
        ASSERT_FALSE(b1.isNull()) << "Array is not empty";
        ASSERT_EQ(adr2, b1.ptr());
        ASSERT_EQ(size, b1.size());
    });
}

TEST(ByteArrayPtrTest, ConstructorFromByteArrayPtr)
{
    pplib::ByteArrayPtr b1(this, 1234567);
    ASSERT_NO_THROW({
        pplib::ByteArrayPtr b2(b1);
        ASSERT_FALSE(b2.isNull()) << "Array is not empty";
        ASSERT_EQ(this, b2.ptr());
        ASSERT_EQ(1234567, b2.size());
    });
}

TEST(ByteArrayPtrTest, ConstructorFromString)
{
    pplib::String s1("Hello World");
    ASSERT_NO_THROW({
        pplib::ByteArrayPtr b2(s1);
        ASSERT_FALSE(b2.isNull()) << "Array is not empty";
        ASSERT_EQ(s1.getPtr(), b2.ptr());
        ASSERT_EQ(s1.size(), b2.size());
    });
}

TEST(ByteArrayPtrTest, ConstructorFromWideString)
{
    pplib::WideString s1(L"Hello World");
    ASSERT_NO_THROW({
        pplib::ByteArrayPtr b2(s1);
        ASSERT_FALSE(b2.isNull()) << "Array is not empty";
        ASSERT_EQ(s1.getPtr(), b2.ptr());
        ASSERT_EQ(s1.size(), b2.size());
    });
}

// 0d,0e,37,42,81,ff,42,00,4c,17,12
static unsigned char binarydata[] = {13, 14, 55, 66, 129, 255, 66, 0, 76, 23, 18};

TEST(ByteArrayPtrTest, isNull)
{
    pplib::ByteArrayPtr b2;
    ASSERT_TRUE(b2.isNull());
    b2.use(binarydata, sizeof(binarydata));
    ASSERT_FALSE(b2.isNull());
    b2.use(binarydata, 0);
    ASSERT_TRUE(b2.isNull());
}
TEST(ByteArrayPtrTest, isEmpty)
{
    pplib::ByteArrayPtr b2;
    ASSERT_TRUE(b2.isEmpty());
    b2.use(binarydata, sizeof(binarydata));
    ASSERT_FALSE(b2.isEmpty());
    b2.use(binarydata, 0);
    ASSERT_TRUE(b2.isEmpty());
}

TEST(ByteArrayPtrTest, NotEmpty)
{
    pplib::ByteArrayPtr b2;
    ASSERT_FALSE(b2.notEmpty());
    b2.use(binarydata, sizeof(binarydata));
    ASSERT_TRUE(b2.notEmpty());
    b2.use(binarydata, 0);
    ASSERT_FALSE(b2.notEmpty());
}

TEST(ByteArrayPtrTest, size)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    ASSERT_EQ(sizeof(binarydata), b2.size());
    pplib::ByteArrayPtr b1;
    ASSERT_EQ((size_t)0, b1.size());
}

TEST(ByteArrayPtrTest, ptr)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    ASSERT_EQ((const void*)&binarydata, b2.ptr());
    pplib::ByteArrayPtr b1;
    ASSERT_EQ((const void*)NULL, b1.ptr());
}

TEST(ByteArrayPtrTest, adr)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    ASSERT_EQ((const void*)&binarydata, b2.adr());
    pplib::ByteArrayPtr b1;
    ASSERT_EQ((const void*)NULL, b1.adr());
}

TEST(ByteArrayPtrTest, map)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    ASSERT_EQ((const char*)&binarydata, b2.map(0, sizeof(binarydata)));
    ASSERT_EQ((const char*)&binarydata[3], b2.map(3, 5));
    ASSERT_THROW(b2.map(11, 1), pplib::OutOfBoundsException);
    ASSERT_THROW(b2.map(10, 2), pplib::OutOfBoundsException);
    ASSERT_THROW(b2.map(12, 0), pplib::OutOfBoundsException);

    pplib::ByteArrayPtr b3;
    ASSERT_THROW(b3.map(0, 1), pplib::OutOfBoundsException);
}

TEST(ByteArrayPtrTest, truncate)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    ASSERT_EQ(sizeof(binarydata), b2.size());
    b2.truncate(5);
    ASSERT_EQ((size_t)5, b2.size());
    b2.truncate(10);
    ASSERT_EQ((size_t)5, b2.size());
    b2.truncate(0);
    ASSERT_EQ((size_t)0, b2.size());
}

TEST(ByteArrayPtrTest, toCharPtr)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    ASSERT_EQ((const char*)&binarydata, b2.toCharPtr());
    pplib::ByteArrayPtr b1;
    ASSERT_EQ((const char*)NULL, b1.toCharPtr());
}

TEST(ByteArrayPtrTest, operatorVoidPtr)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    ASSERT_EQ((const void*)&binarydata, (const void*)b2);
}

TEST(ByteArrayPtrTest, operatorCharPtr)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    ASSERT_EQ((const char*)&binarydata, (const char*)b2);
}

TEST(ByteArrayPtrTest, operatorUnsignedCharPtr)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    ASSERT_EQ((const unsigned char*)&binarydata, (const unsigned char*)b2);
}

TEST(ByteArrayPtrTest, operatorCopy)
{
    pplib::ByteArrayPtr b1(binarydata, sizeof(binarydata));
    pplib::ByteArrayPtr b2;
    b2 = b1;
    ASSERT_EQ((const void*)&binarydata, b2.adr());
    ASSERT_EQ(sizeof(binarydata), b2.size());
}

TEST(ByteArrayPtrTest, usePtrAndSize)
{
    pplib::ByteArrayPtr b2;
    b2.use(binarydata, sizeof(binarydata));
    ASSERT_EQ((const void*)&binarydata, b2.adr());
    ASSERT_EQ(sizeof(binarydata), b2.size());
}

TEST(ByteArrayPtrTest, useOtherByteArrayPtr)
{
    pplib::ByteArrayPtr b1(binarydata, sizeof(binarydata));
    pplib::ByteArrayPtr b2;
    b2.use(b1);
    ASSERT_EQ((const void*)&binarydata, b2.adr());
    ASSERT_EQ(sizeof(binarydata), b2.size());
}

TEST(ByteArrayPtrTest, Get)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    EXPECT_EQ(13, b2.get(0));
    EXPECT_EQ(14, b2.get(1));
    EXPECT_EQ(55, b2.get(2));
    EXPECT_EQ(0, b2.get(7));
    EXPECT_EQ(23, b2.get(9));
    EXPECT_EQ(18, b2.get(10));

    ASSERT_THROW(b2.get(11), pplib::OutOfBoundsException);
}

TEST(ByteArrayPtrTest, GetOnEmptyObject)
{
    pplib::ByteArrayPtr b1;
    ASSERT_THROW(b1.get(0), pplib::OutOfBoundsException);
}

TEST(ByteArrayPtrTest, OperatorGet)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    unsigned char val = b2[0];
    EXPECT_EQ(13, val);
    EXPECT_EQ(13, b2[0]);
    EXPECT_EQ(14, b2[1]);
    EXPECT_EQ(55, b2[2]);
    EXPECT_EQ(0, b2[7]);
    EXPECT_EQ(23, b2[9]);
    EXPECT_EQ(18, b2[10]);
    ASSERT_THROW(b2[11], pplib::OutOfBoundsException);

    const pplib::ByteArrayPtr b3(binarydata, sizeof(binarydata));
    val = b3[0];
    EXPECT_EQ(13, val);
    ASSERT_THROW(b3[11], pplib::OutOfBoundsException);
}

TEST(ByteArrayPtrTest, OperatorGetOnEmptyObject)
{
    pplib::ByteArrayPtr b1;
    ASSERT_THROW(b1[0], pplib::OutOfBoundsException);
    const pplib::ByteArrayPtr b2;
    ASSERT_THROW(b2[0], pplib::OutOfBoundsException);
}

TEST(ByteArrayPtrTest, OperatorSet)
{
    unsigned char localdata[] = {13, 14, 55, 66, 129, 255, 66, 0, 76, 23, 18};
    pplib::ByteArrayPtr b2(localdata, sizeof(localdata));
    EXPECT_EQ(13, b2[0]);
    b2[0] = 69;
    EXPECT_EQ(69, b2[0]);
    b2[10] = 123;
    EXPECT_EQ(123, b2[10]);
    ASSERT_THROW(b2[11] = 42, pplib::OutOfBoundsException);
}

TEST(ByteArrayPtrTest, Set)
{
    unsigned char localdata[] = {13, 14, 55, 66, 129, 255, 66, 0, 76, 23, 18};
    pplib::ByteArrayPtr b2(localdata, sizeof(localdata));
    EXPECT_EQ(13, b2[0]);
    b2.set(0, 69);
    EXPECT_EQ(69, b2[0]);
    b2.set(10, 123);
    EXPECT_EQ(123, b2[10]);
    ASSERT_THROW(b2.set(11, 42), pplib::OutOfBoundsException);
}

TEST(ByteArrayPtrTest, SetOnEmptyObject)
{
    pplib::ByteArrayPtr b1;
    ASSERT_THROW(b1.set(0, 42), pplib::OutOfBoundsException);
}

TEST(ByteArrayPtrTest, toString)
{
    pplib::ByteArrayPtr b2("Hello World", 11);
    ASSERT_EQ(pplib::String("Hello World"), b2.toString());

    pplib::ByteArrayPtr empty;
    ASSERT_EQ(pplib::String(""), empty.toString());
}

TEST(ByteArrayPtrTest, toWideString)
{
    pplib::ByteArrayPtr b2(L"Hello World", 11 * sizeof(wchar_t));
    ASSERT_EQ(pplib::WideString(L"Hello World"), b2.toWideString());

    pplib::ByteArrayPtr empty;
    ASSERT_EQ(pplib::WideString(L""), empty.toWideString());
}

TEST(ByteArrayPtrTest, toHex)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    ASSERT_EQ(pplib::String("0d0e374281ff42004c1712"), b2.toHex());
}

TEST(ByteArrayPtrTest, toHexOnEmptyObject)
{
    pplib::ByteArrayPtr b1;
    ASSERT_EQ(pplib::String(""), b1.toHex());
    b1.use(binarydata, 0);
    ASSERT_EQ(pplib::String(""), b1.toHex());
}

TEST(ByteArrayPtrTest, toBase64)
{
    ////0d,0e,37,42,81,ff,42,00,4c,17,12
    pplib::ByteArrayPtr b2;
    b2.use(binarydata, 7);
    EXPECT_EQ(pplib::String("DQ43QoH/Qo=="), b2.toBase64());
    b2.use(binarydata, 8);
    EXPECT_EQ(pplib::String("DQ43QoH/QgD="), b2.toBase64());
    b2.use(binarydata, 9);
    EXPECT_EQ(pplib::String("DQ43QoH/QgBM"), b2.toBase64());
    b2.use(binarydata, 10);
    EXPECT_EQ(pplib::String("DQ43QoH/QgBMFw=="), b2.toBase64());
    b2.use(binarydata, 11);
    EXPECT_EQ(pplib::String("DQ43QoH/QgBMFxJ="), b2.toBase64());
}

static const char* teststring = "Hello World";

TEST(ByteArrayPtrTest, md5)
{
    pplib::ByteArrayPtr b2(teststring, 11);
    EXPECT_EQ(pplib::String("b10a8db164e0754105b7a99be72e3fe5"), pplib::Md5(b2));
}

TEST(ByteArrayPtrTest, crc32)
{
    pplib::ByteArrayPtr b2(teststring, 11);
    EXPECT_EQ((uint32_t)1243066710, b2.crc32());
    pplib::ByteArrayPtr empty;
    ASSERT_THROW(empty.crc32(), pplib::EmptyDataException);
}

TEST(ByteArrayPtrTest, hexDump)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    // Wir müssen stdout umleiten, da hexDump() direkt auf stdout schreibt
    testing::internal::CaptureStdout();
    b2.hexDump();
    pplib::String output = testing::internal::GetCapturedStdout();
    ASSERT_TRUE(output.has("HEXDUMP of ByteArray: 11 Bytes starting at Address"));
    ASSERT_TRUE(output.has("0D 0E 37 42 81 FF 42 00 4C 17 12"));
    ASSERT_TRUE(output.has(": ..7B..B.L.."));
}

TEST(ByteArrayPtrTest, hexDumpWithSize)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    // Wir müssen stdout umleiten, da hexDump() direkt auf stdout schreibt
    testing::internal::CaptureStdout();
    b2.hexDump(7);
    pplib::String output = testing::internal::GetCapturedStdout();
    ASSERT_TRUE(output.has("HEXDUMP of ByteArray: 7 Bytes starting at Address"));
    ASSERT_TRUE(output.has("0D 0E 37 42 81 FF 42"));
    ASSERT_TRUE(output.has(": ..7B..B"));
}

TEST(ByteArrayPtrTest, hexDumpWithEmptyObject)
{
    pplib::ByteArrayPtr b1;
    // Wir müssen stdout umleiten, da hexDump() direkt auf stdout schreibt
    testing::internal::CaptureStdout();
    b1.hexDump();
    pplib::String output = testing::internal::GetCapturedStdout();
    ASSERT_TRUE(output.has("HEXDUMP of ByteArray: No Data, address or size is 0"));
}

TEST(ByteArrayPtrTest, hexDumpOffsetIsOutOfRange)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    // Wir müssen stdout umleiten, da hexDump() direkt auf stdout schreibt
    testing::internal::CaptureStdout();
    b2.hexDump(20, 11);
    pplib::String output = testing::internal::GetCapturedStdout();
    // output.printnl();
    ASSERT_TRUE(output.has("HEXDUMP of ByteArray: Offset 20 exceeds size of ByteArray (11)"));
}

TEST(ByteArrayPtrTest, memset)
{
    static unsigned char localdata[] = {13, 14, 55, 66, 129, 255, 66, 0, 76, 23, 18};
    static unsigned char expected[] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32};
    pplib::ByteArrayPtr b2(localdata, sizeof(localdata));
    b2.memset(32);
    EXPECT_TRUE(memcmp(expected, localdata, sizeof(localdata)) == 0);
    pplib::ByteArrayPtr empty;
    empty.memset(42); // sollte keine Exception werfen
}

TEST(ByteArrayPtrTest, ostream)
{
    pplib::ByteArrayPtr b2(binarydata, sizeof(binarydata));
    std::ostringstream oss;
    oss << b2;
    ASSERT_EQ(pplib::String("0d0e374281ff42004c1712"), oss.str());
}

// memBlock1 > MemBlock2
//           > MemBlock3
//           < MemBlock4
// memBlock2 < MemBlock1
//           > MemBlock3
//           < MemBlock4
// memBlock3 < MemBlock1
//           < MemBlock2
//           < MemBlock4
// memBlock4 > MemBlock1
//           > MemBlock2
//           > MemBlock4
static unsigned char memBlock1[] = {123, 77, 42, 255, 192, 16, 9};
static unsigned char memBlock2[] = {123, 77, 42, 200, 192, 16, 9};
static unsigned char memBlock3[] = {123, 77, 42};
static unsigned char memBlock4[] = {123, 77, 99};

TEST(ByteArrayPtrCompare, b1ZeroSize_b2ZeroSize)
{
    pplib::ByteArrayPtr b1, b2;
    ASSERT_EQ(0, b1.memcmp(b2));
}

TEST(ByteArrayPtrCompare, b1ZeroSize_b2NotZero)
{
    pplib::ByteArrayPtr b1;
    pplib::ByteArrayPtr b2(memBlock1, sizeof(memBlock1));
    ASSERT_EQ((size_t)7, b2.size());
    ASSERT_EQ(-1, b1.memcmp(b2));
}

TEST(ByteArrayPtrCompare, b1NotZero_b2ZeroSize)
{
    pplib::ByteArrayPtr b1(memBlock1, sizeof(memBlock1));
    pplib::ByteArrayPtr b2;
    ASSERT_EQ((size_t)7, b1.size());
    ASSERT_EQ(1, b1.memcmp(b2));
}

TEST(ByteArrayPtrCompare, b1_eq_b2_samesize)
{
    pplib::ByteArrayPtr b1(memBlock1, sizeof(memBlock1));
    pplib::ByteArrayPtr b2(memBlock1, sizeof(memBlock1));
    ASSERT_EQ(0, b1.memcmp(b2));
}

TEST(ByteArrayPtrCompare, b1_lt_b2_samesize)
{
    pplib::ByteArrayPtr b1(memBlock2, sizeof(memBlock2));
    pplib::ByteArrayPtr b2(memBlock1, sizeof(memBlock1));
    ASSERT_EQ(-1, b1.memcmp(b2));
}

TEST(ByteArrayPtrCompare, b1_gt_b2_samesize)
{
    pplib::ByteArrayPtr b1(memBlock1, sizeof(memBlock1));
    pplib::ByteArrayPtr b2(memBlock2, sizeof(memBlock2));
    ASSERT_EQ(1, b1.memcmp(b2));
}

TEST(ByteArrayPtrCompare, b1_lt_b2__b1_shorter_b2)
{
    pplib::ByteArrayPtr b1(memBlock3, sizeof(memBlock3));
    pplib::ByteArrayPtr b2(memBlock1, sizeof(memBlock1));
    ASSERT_EQ(-1, b1.memcmp(b2));
}

TEST(ByteArrayPtrCompare, b1_gt_b2__b1_shorter_b2)
{
    pplib::ByteArrayPtr b1(memBlock4, sizeof(memBlock4));
    pplib::ByteArrayPtr b2(memBlock2, sizeof(memBlock2));
    ASSERT_EQ(1, b1.memcmp(b2));
}

TEST(ByteArrayPtrCompare, b1_lt_b2__b1_longer_b2)
{
    pplib::ByteArrayPtr b1(memBlock1, sizeof(memBlock1));
    pplib::ByteArrayPtr b2(memBlock4, sizeof(memBlock4));
    ASSERT_EQ(-1, b1.memcmp(b2));
}

TEST(ByteArrayPtrCompare, b1_gt_b2__b1_longer_b2)
{
    pplib::ByteArrayPtr b1(memBlock2, sizeof(memBlock2));
    pplib::ByteArrayPtr b2(memBlock3, sizeof(memBlock3));
    ASSERT_EQ(1, b1.memcmp(b2));
}

TEST(ByteArrayPtrCompare, operators_equal)
{
    pplib::ByteArrayPtr b1(memBlock1, sizeof(memBlock1));
    pplib::ByteArrayPtr b2(memBlock1, sizeof(memBlock1));
    pplib::ByteArrayPtr b3(memBlock2, sizeof(memBlock2));
    ASSERT_TRUE(b1 == b2);
    ASSERT_FALSE(b1 != b2);
    ASSERT_FALSE(b1 == b3);
    ASSERT_TRUE(b1 != b3);
}

TEST(ByteArrayPtrCompare, operators_lower)
{
    pplib::ByteArrayPtr b1(memBlock1, sizeof(memBlock1));
    pplib::ByteArrayPtr b2(memBlock2, sizeof(memBlock2));
    pplib::ByteArrayPtr b3(memBlock3, sizeof(memBlock3));
    pplib::ByteArrayPtr b4(memBlock4, sizeof(memBlock4));
    ASSERT_FALSE(b1 < b2);
    ASSERT_TRUE(b2 < b1);
    ASSERT_TRUE(b2 <= b1);
    ASSERT_TRUE(b3 < b1);
    ASSERT_TRUE(b3 < b4);
    ASSERT_TRUE(b3 <= b4);
    ASSERT_FALSE(b4 < b3);
}

TEST(ByteArrayPtrCompare, operators_greater)
{
    pplib::ByteArrayPtr b1(memBlock1, sizeof(memBlock1));
    pplib::ByteArrayPtr b2(memBlock2, sizeof(memBlock2));
    pplib::ByteArrayPtr b3(memBlock3, sizeof(memBlock3));
    pplib::ByteArrayPtr b4(memBlock4, sizeof(memBlock4));

    ASSERT_TRUE(b1 > b2);
    ASSERT_TRUE(b1 >= b2);
    ASSERT_FALSE(b2 > b1);
    ASSERT_FALSE(b3 > b1);
    ASSERT_FALSE(b3 > b4);
    ASSERT_FALSE(b3 >= b4);
    ASSERT_TRUE(b4 > b3);
    ASSERT_TRUE(b4 >= b3);
}

} // namespace
