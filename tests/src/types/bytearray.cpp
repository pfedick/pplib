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
#include <pplib/core/functions.h>
#include <pplib/core/file.h>
#include <pplib/exceptions.h>

#include "pplib-tests.h"

namespace
{

TEST(ByteArrayTest, ConstructorWithoutParameters)
{
    ASSERT_NO_THROW({
        pplib::ByteArray b1;
        ASSERT_TRUE(b1.isNull() == true) << "Array is not empty";
        ASSERT_EQ(b1.size(), (size_t)0) << "Array is not empty";
        ASSERT_EQ(b1.ptr(), nullptr) << "Array is not empty";
    });
}

TEST(ByteArrayTest, CopyConstructorWithEmptyObject)
{
    pplib::ByteArray b1;
    ASSERT_NO_THROW({
        pplib::ByteArray b2(b1);
        ASSERT_TRUE(b2.isNull() == true) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)0) << "Array is not empty";
        ASSERT_EQ(b2.ptr(), nullptr) << "Array is not empty";
    });
}

TEST(ByteArrayTest, CopyConstructorWithData)
{
    const pplib::ByteArray testdata = pplib::Random(2048);
    ASSERT_NO_THROW({
        pplib::ByteArray b2(testdata);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)2048) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, CopyConstructorWithEmptyByteArrayPtrObject)
{
    pplib::ByteArrayPtr b1;
    ASSERT_NO_THROW({
        pplib::ByteArray b2(b1);
        ASSERT_TRUE(b2.isNull() == true) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)0) << "Array is not empty";
        ASSERT_EQ(b2.ptr(), nullptr) << "Array is not empty";
    });
}

TEST(ByteArrayTest, CopyConstructorWithByteArrayPtrObjectWithData)
{
    const pplib::ByteArray testdata = pplib::Random(2048);
    const pplib::ByteArrayPtr b1(testdata);
    ASSERT_NO_THROW({
        pplib::ByteArray b2(b1);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)2048) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, MoveConstructorWithEmptyObject)
{
    pplib::ByteArray b1;
    ASSERT_NO_THROW({
        pplib::ByteArray b2(std::move(b1));
        ASSERT_TRUE(b2.isNull() == true) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)0) << "Array is not empty";
        ASSERT_EQ(b2.ptr(), nullptr) << "Array is not empty";
    });
}

TEST(ByteArrayTest, MoveConstructorWithData)
{
    const pplib::ByteArray testdata = pplib::Random(2048);
    pplib::ByteArray b1 = testdata;
    ASSERT_NO_THROW({
        pplib::ByteArray b2(std::move(b1));
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)2048) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, ConstructorFromMemoryPtr)
{
    const pplib::ByteArray testdata = pplib::Random(2048);
    ASSERT_NO_THROW({
        pplib::ByteArray b2(testdata.ptr(), testdata.size());
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)2048) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, ConstructorFromString)
{
    const pplib::String testdata = "Hello World";
    ASSERT_NO_THROW({
        pplib::ByteArray b2(testdata);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)11) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2.toString()) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, ConstructorFromWideString)
{
    const pplib::WideString testdata = L"Hello World";
    ASSERT_NO_THROW({
        pplib::ByteArray b2(testdata);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), 11 * sizeof(wchar_t)) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2.toWideString()) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, ConstructorWithSize)
{
    ASSERT_NO_THROW({
        pplib::ByteArray b1(1024);
        ASSERT_TRUE(b1.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b1.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b1.ptr(), nullptr) << "Array is not empty";
    });
}

TEST(ByteArrayTest, copyFromMemory)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata.ptr(), testdata.size());
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, copyWithNullPtr)
{
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(nullptr, 10);
        ASSERT_TRUE(b2.isNull() == true) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)0) << "Array is not empty";
        ASSERT_EQ(b2.ptr(), nullptr) << "Array is not empty";
    });
}

TEST(ByteArrayTest, copyWithNullSize)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata.ptr(), 0);
        ASSERT_TRUE(b2.isNull() == true) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)0) << "Array is not empty";
        ASSERT_EQ(b2.ptr(), nullptr) << "Array is not empty";
    });
}

TEST(ByteArrayTest, copyWithSamePtrAndSize)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2 = testdata;
        b2.copy(b2.ptr(), b2.size());
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
}
TEST(ByteArrayTest, copyWithPartOfSamePtrAndSize)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2 = testdata;
        b2.copy((const char*)b2.ptr() + 100, 200);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)200) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata.mid(100, 200), b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, copyThrowsOutOfMemoryException)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_THROW(
        {
            pplib::ByteArray b2;
            b2.copy(testdata.ptr(), SIZE_MAX - 4);
        },
        pplib::OutOfMemoryException);

    ASSERT_THROW(
        {
            pplib::ByteArray b2;
            b2.copy(testdata.ptr(), SIZE_MAX);
        },
        pplib::OutOfMemoryException);
}

TEST(ByteArrayTest, copyFromByteArrayPtr)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    pplib::ByteArrayPtr b1(testdata);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(b1);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, useAddr)
{
    char* data = (char*)malloc(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.useadr(data, 1024);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_EQ((const char*)b2.ptr(), data) << "Array is not empty";
    });
}

TEST(ByteArrayTest, append)
{
    pplib::ByteArray testdata1 = pplib::Random(1024);
    pplib::ByteArray testdata2 = pplib::Random(512);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata1.ptr(), testdata1.size());
        b2.append(testdata2);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1536) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata1, pplib::ByteArray(b2.ptr(), 1024)) << "First part of array does not match original data";
        ASSERT_EQ(testdata2, pplib::ByteArray((const char*)b2.ptr() + 1024, 512)) << "Second part of array does not match original data";
    });
}

TEST(ByteArrayTest, appendOnEmptyObject)
{
    pplib::ByteArray testdata1 = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.append(testdata1);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata1, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, appendWithNullPtr)
{
    pplib::ByteArray testdata1 = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata1.ptr(), testdata1.size());
        b2.append(nullptr, 512);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata1, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, appendWithNullSize)
{
    pplib::ByteArray testdata1 = pplib::Random(1024);
    pplib::ByteArray testdata2 = pplib::Random(512);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata1.ptr(), testdata1.size());
        b2.append(testdata2.ptr(), 0);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata1, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, appendWithSamePtrAndSize)
{
    pplib::ByteArray testdata1 = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata1.ptr(), testdata1.size());
        b2.append(b2.ptr(), b2.size());
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)2048) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata1, pplib::ByteArray(b2.ptr(), 1024)) << "First part of array does not match original data";
        ASSERT_EQ(testdata1, pplib::ByteArray((const char*)b2.ptr() + 1024, 1024)) << "Second part of array does not match original data";
    });
}

TEST(ByteArrayTest, appendThrowsOutOfMemoryException)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_THROW(
        {
            pplib::ByteArray b2 = pplib::Random(1024);
            b2.append(testdata.ptr(), SIZE_MAX - 4 - 1024);
        },
        pplib::OutOfMemoryException);

    ASSERT_THROW(
        {
            pplib::ByteArray b2 = pplib::Random(1024);
            b2.append(testdata.ptr(), SIZE_MAX - 4);
        },
        pplib::OutOfMemoryException);
}

TEST(ByteArrayTest, prepend)
{
    pplib::ByteArray testdata1 = pplib::Random(1024);
    pplib::ByteArray testdata2 = pplib::Random(512);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata1.ptr(), testdata1.size());
        b2.prepend(testdata2);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1536) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata2, pplib::ByteArray(b2.ptr(), 512)) << "First part of array does not match original data";
        ASSERT_EQ(testdata1, pplib::ByteArray((const char*)b2.ptr() + 512, 1024)) << "Second part of array does not match original data";
    });
}

TEST(ByteArrayTest, prependOnEmptyObject)
{
    pplib::ByteArray testdata1 = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.prepend(testdata1);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata1, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, prependWithNullPtr)
{
    pplib::ByteArray testdata1 = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata1.ptr(), testdata1.size());
        b2.prepend(nullptr, 512);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata1, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, prependWithNullSize)
{
    pplib::ByteArray testdata1 = pplib::Random(1024);
    pplib::ByteArray testdata2 = pplib::Random(512);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata1.ptr(), testdata1.size());
        b2.prepend(testdata2.ptr(), 0);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata1, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, prependWithSamePtrAndSize)
{
    pplib::ByteArray testdata1 = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata1.ptr(), testdata1.size());
        b2.prepend(b2.ptr(), b2.size());
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)2048) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata1, pplib::ByteArray((const char*)b2.ptr() + 1024, 1024)) << "First part of array does not match original data";
        ASSERT_EQ(testdata1, pplib::ByteArray(b2.ptr(), 1024)) << "Second part of array does not match original data";
    });
}

TEST(ByteArrayTest, prependThrowsOutOfMemoryException)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_THROW(
        {
            pplib::ByteArray b2 = pplib::Random(1024);
            b2.prepend(testdata.ptr(), SIZE_MAX - 4 - 1024);
        },
        pplib::OutOfMemoryException);
    ASSERT_THROW(
        {
            pplib::ByteArray b2 = pplib::Random(1024);
            b2.prepend(testdata.ptr(), SIZE_MAX - 4);
        },
        pplib::OutOfMemoryException);
}

TEST(ByteArrayTest, truncate)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata.ptr(), testdata.size());
        b2.truncate(512);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)512) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata.mid(0, 512), b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, truncateBiggerThanCurrentSize)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata.ptr(), testdata.size());
        b2.truncate(2048);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, malloc)
{
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.malloc(1024);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
    });
}

TEST(ByteArrayTest, mallocThrowsOutOfMemoryException)
{
    ASSERT_THROW(
        {
            pplib::ByteArray b2;
            b2.malloc(SIZE_MAX - 4);
        },
        pplib::OutOfMemoryException);

    ASSERT_THROW(
        {
            pplib::ByteArray b2;
            b2.malloc(SIZE_MAX);
        },
        pplib::OutOfMemoryException);
}

TEST(ByteArrayTest, calloc)
{
    pplib::ByteArray empty(1024);
    for (size_t i = 0; i < empty.size(); ++i) {
        ((char*)empty.ptr())[i] = 0x00;
    }

    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.calloc(1024);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(empty, b2) << "Array contains nonempty bytes after calloc, expected all bytes to be zero";
    });
}

TEST(ByteArrayTest, callocThrowsOutOfMemoryException)
{
    ASSERT_THROW(
        {
            pplib::ByteArray b2;
            b2.calloc(SIZE_MAX - 4);
        },
        pplib::OutOfMemoryException);

    ASSERT_THROW(
        {
            pplib::ByteArray b2;
            b2.calloc(SIZE_MAX);
        },
        pplib::OutOfMemoryException);
}

TEST(ByteArrayTest, free)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata.ptr(), testdata.size());
        b2.free();
        ASSERT_TRUE(b2.isNull() == true) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)0) << "Array is not empty";
        ASSERT_EQ(b2.ptr(), nullptr) << "Array is not empty";
    });
}

TEST(ByteArrayTest, clear)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata.ptr(), testdata.size());
        b2.clear();
        ASSERT_TRUE(b2.isNull() == true) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)0) << "Array is not empty";
        ASSERT_EQ(b2.ptr(), nullptr) << "Array is not empty";
    });
}

TEST(ByteArrayTest, realloc)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata.ptr(), testdata.size());
        b2.realloc(2048);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)2048) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, pplib::ByteArray(b2.ptr(), 1024)) << "First part of array does not match original data";
    });
}

TEST(ByteArrayTest, reallocThrowsOutOfMemoryException)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_THROW(
        {
            pplib::ByteArray b2;
            b2.copy(testdata.ptr(), testdata.size());
            b2.realloc(SIZE_MAX - 4);
        },
        pplib::OutOfMemoryException);

    ASSERT_THROW(
        {
            pplib::ByteArray b2;
            b2.copy(testdata.ptr(), testdata.size());
            b2.realloc(SIZE_MAX);
        },
        pplib::OutOfMemoryException);
}

TEST(ByteArrayTest, operatorAssign)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2 = testdata;
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, operatorAssigWithByteArrayPtr)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    pplib::ByteArrayPtr b1(testdata);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2 = b1;
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, operatorMoveAssign)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    pplib::ByteArray b1 = testdata;
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2 = std::move(b1);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, operatorMoveAssignWithSameObject)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    pplib::ByteArray b1 = testdata;
    ASSERT_NO_THROW({
        b1 = std::move(b1);
        ASSERT_TRUE(b1.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b1.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b1.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b1) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, operatorAssignWithString)
{
    pplib::String testdata = "Hello World";
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2 = testdata;
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)11) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2.toString()) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, operatorAssignWithWideString)
{
    pplib::WideString testdata = L"Hello World";
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2 = testdata;
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), 11 * sizeof(wchar_t)) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2.toWideString()) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, opperatorPlus)
{
    pplib::ByteArray testdata1 = pplib::Random(1024);
    pplib::ByteArray testdata2 = pplib::Random(512);
    ASSERT_NO_THROW({
        pplib::ByteArray b2 = testdata1 + testdata2;
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1536) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata1, pplib::ByteArray(b2.ptr(), 1024)) << "First part of array does not match original data";
        ASSERT_EQ(testdata2, pplib::ByteArray((const char*)b2.ptr() + 1024, 512)) << "Second part of array does not match original data";
    });
}

TEST(ByteArrayTest, operatorPlusAssign)
{
    pplib::ByteArray testdata1 = pplib::Random(1024);
    pplib::ByteArray testdata2 = pplib::Random(512);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata1.ptr(), testdata1.size());
        b2 += testdata2;
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1536) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata1, pplib::ByteArray(b2.ptr(), 1024)) << "First part of array does not match original data";
        ASSERT_EQ(testdata2, pplib::ByteArray((const char*)b2.ptr() + 1024, 512)) << "Second part of array does not match original data";
    });
}

TEST(ByteArrayTest, operatorPlusAssignWithString)
{
    pplib::String testdata1 = "Hello ";
    pplib::String testdata2 = "World";
    ASSERT_NO_THROW({
        pplib::ByteArray b2(testdata1);
        b2 += testdata2;
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)11) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata1 + testdata2, b2.toString()) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, operatorPlusAssignWithWideString)
{
    pplib::WideString testdata1 = L"Hello ";
    pplib::WideString testdata2 = L"World";
    ASSERT_NO_THROW({
        pplib::ByteArray b2(testdata1);
        b2 += testdata2;
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)11 * sizeof(wchar_t)) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata1 + testdata2, b2.toWideString()) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, left)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata.ptr(), testdata.size());
        pplib::ByteArray b3 = b2.left(512);
        ASSERT_TRUE(b3.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b3.size(), (size_t)512) << "Array is not empty";
        ASSERT_NE(b3.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata.mid(0, 512), b3) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, leftIsBiggerThanCurrentSize)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata.ptr(), testdata.size());
        pplib::ByteArray b3 = b2.left(2048);
        ASSERT_TRUE(b3.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b3.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b3.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b3) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, right)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata.ptr(), testdata.size());
        pplib::ByteArray b3 = b2.right(512);
        ASSERT_TRUE(b3.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b3.size(), (size_t)512) << "Array is not empty";
        ASSERT_NE(b3.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata.mid(512, 512), b3) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, rightIsBiggerThanCurrentSize)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata.ptr(), testdata.size());
        pplib::ByteArray b3 = b2.right(2048);
        ASSERT_TRUE(b3.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b3.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b3.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b3) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, mid)
{
    pplib::ByteArray left = pplib::Random(512);
    pplib::ByteArray mid = pplib::Random(512);
    pplib::ByteArray right = pplib::Random(512);
    pplib::ByteArray testdata = left + mid + right;
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata.ptr(), testdata.size());
        pplib::ByteArray b3 = b2.mid(512, 512);
        ASSERT_TRUE(b3.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b3.size(), (size_t)512) << "Array is not empty";
        ASSERT_NE(b3.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(mid, b3) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, midOffsetIsBiggerThanCurrentSize)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata.ptr(), testdata.size());
        pplib::ByteArray b3 = b2.mid(2048, 512);
        ASSERT_TRUE(b3.isNull() == true) << "Array is not empty";
        ASSERT_EQ(b3.size(), (size_t)0) << "Array is not empty";
        ASSERT_EQ(b3.ptr(), nullptr) << "Array is not empty";
    });
}

TEST(ByteArrayTest, midBytesIsBiggerThanCurrentSize)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::ByteArray b2;
        b2.copy(testdata.ptr(), testdata.size());
        pplib::ByteArray b3 = b2.mid(512, 2048);
        ASSERT_TRUE(b3.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b3.size(), (size_t)512) << "Array is not empty";
        ASSERT_NE(b3.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata.mid(512, 512), b3) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, toHexFromHex)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::String hex = testdata.toHex();
        pplib::ByteArray b2 = pplib::ByteArray::fromHex(hex);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
    // Mit bekannten Daten testen
    ASSERT_NO_THROW({
        pplib::ByteArray b1("Hello World");
        pplib::String hex = b1.toHex();
        ASSERT_EQ(hex, "48656c6c6f20576f726c64");
        pplib::ByteArray b2 = pplib::ByteArray::fromHex(hex);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)11) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(b1, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, toHexFromHexWithUpperCase)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    ASSERT_NO_THROW({
        pplib::String hex = testdata.toHex();
        hex.upperCase();
        pplib::ByteArray b2 = pplib::ByteArray::fromHex(hex);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
    // Mit bekannten Daten testen
    ASSERT_NO_THROW({
        pplib::ByteArray b1("Hello World");
        pplib::String hex = "48656C6C6F20576F726C64";
        pplib::ByteArray b2 = pplib::ByteArray::fromHex(hex);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)11) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(b1, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, fromHexWithInvalidHexString)
{
    ASSERT_THROW({ pplib::ByteArray b2 = pplib::ByteArray::fromHex("48656C6C6F20576F726C6"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ pplib::ByteArray b2 = pplib::ByteArray::fromHex("48656C6C6F20576F726C6G"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ pplib::ByteArray b2 = pplib::ByteArray::fromHex("48656z6c6f20576f726c64"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ pplib::ByteArray b2 = pplib::ByteArray::fromHex("4.656z6c6f20576f726c64"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ pplib::ByteArray b2 = pplib::ByteArray::fromHex(".8656c6c6f20576f726c64"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ pplib::ByteArray b2 = pplib::ByteArray::fromHex("z8656c6c6f20576f726c64"); }, pplib::IllegalArgumentException);
}

TEST(ByteArrayTest, fromHexWithEmptyHexString)
{
    ASSERT_NO_THROW({
        pplib::ByteArray b2 = pplib::ByteArray::fromHex("");
        ASSERT_TRUE(b2.isNull() == true) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)0) << "Array is not empty";
        ASSERT_EQ(b2.ptr(), nullptr) << "Array is not empty";
    });
}

TEST(ByteArrayTest, fromBase64)
{
    ASSERT_NO_THROW({
        pplib::ByteArray testdata = pplib::Random(1024);
        pplib::String base64 = testdata.toBase64();
        pplib::ByteArray b2 = pplib::ByteArray::fromBase64(base64);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1024) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
    ASSERT_NO_THROW({
        pplib::ByteArray testdata = pplib::Random(1023);
        pplib::String base64 = testdata.toBase64();
        pplib::ByteArray b2 = pplib::ByteArray::fromBase64(base64);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1023) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
    ASSERT_NO_THROW({
        pplib::ByteArray testdata = pplib::Random(1022);
        pplib::String base64 = testdata.toBase64();
        pplib::ByteArray b2 = pplib::ByteArray::fromBase64(base64);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1022) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
    ASSERT_NO_THROW({
        pplib::ByteArray testdata = pplib::Random(1021);
        pplib::String base64 = testdata.toBase64();
        pplib::ByteArray b2 = pplib::ByteArray::fromBase64(base64);
        ASSERT_TRUE(b2.isNull() == false) << "Array is not empty";
        ASSERT_EQ(b2.size(), (size_t)1021) << "Array is not empty";
        ASSERT_NE(b2.ptr(), nullptr) << "Array is not empty";
        ASSERT_EQ(testdata, b2) << "Array does not match original data";
    });
}

TEST(ByteArrayTest, fromBase64WithInvalidString)
{
    ASSERT_THROW({ pplib::ByteArray b2 = pplib::ByteArray::fromBase64(".GVsbG8gV29ybGQ"); }, pplib::IllegalArgumentException);
}

TEST(ByteArrayTest, ostream)
{
    pplib::ByteArray testdata = pplib::Random(1024);
    std::ostringstream oss;
    oss << testdata;
    ASSERT_EQ(testdata.toHex(), oss.str());
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
