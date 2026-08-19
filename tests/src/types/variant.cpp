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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <gtest/gtest.h>

#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/array.h>
#include <pplib/types/assocarray.h>
#include <pplib/types/variant.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

#include "pplib-tests.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

extern pplib::Array Wordlist;
extern pplib::AssocArray TestAssocArray;

namespace
{

// The fixture for testing class Foo.
class VariantTest : public ::testing::Test
{
protected:
    VariantTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~VariantTest()
    {
    }
};

TEST_F(VariantTest, ConstructorWithoutParam)
{
    ASSERT_NO_THROW({
        pplib::Variant var1;
        ASSERT_EQ(pplib::Variant::TYPE_UNKNOWN, var1.type()) << "Variant has unexcpected type";
        ASSERT_THROW({ pplib::String s1 = var1.toString(); }, pplib::EmptyDataException);
    });
}

TEST_F(VariantTest, TestWithString)
{
    pplib::String s1("Hello World");
    ASSERT_NO_THROW({
        pplib::Variant var1(s1);
        pplib::Variant var2(var1);
        pplib::Variant* var3 = new pplib::Variant(var1);
        delete var3;
        ASSERT_EQ(pplib::Variant::TYPE_STRING, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";
        ASSERT_FALSE(var2.isType(pplib::Variant::TYPE_WIDESTRING)) << "Variant has unexcpected type";

        ASSERT_TRUE(var2.isString()) << "Variant has unexcpected type";
        pplib::String s2 = var2.toString();
        ASSERT_EQ(s1, s2) << "Variant has unexcpected value";
        const pplib::String& cs2 = var1.toString();
        ASSERT_EQ(s1, cs2) << "Variant has unexcpected value";
        ASSERT_THROW({ pplib::WideString s3 = var1.toWideString(); }, pplib::TypeConversionException);
    });
}

TEST_F(VariantTest, TestWithWideString)
{
    pplib::WideString s1(L"Hello World");
    ASSERT_NO_THROW({
        pplib::Variant var1(s1);
        pplib::Variant var2(var1);
        ASSERT_EQ(pplib::Variant::TYPE_WIDESTRING, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_WIDESTRING)) << "Variant has unexcpected type";
        ASSERT_FALSE(var2.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";

        ASSERT_TRUE(var1.isWideString()) << "Variant has unexcpected type";
        pplib::WideString s2 = var2.toWideString();
        ASSERT_EQ(s1, s2) << "Variant has unexcpected value";
        const pplib::WideString& cs2 = var1.toWideString();
        ASSERT_EQ(s1, cs2) << "Variant has unexcpected value";
        ASSERT_THROW({ pplib::String s3 = var1.toString(); }, pplib::TypeConversionException);
    });
}

TEST_F(VariantTest, TestWithArray)
{
    // pplib::Array  s1(L"Hello World");
    ASSERT_NO_THROW({
        pplib::Variant var1(Wordlist);
        pplib::Variant var2(var1);
        ASSERT_EQ(pplib::Variant::TYPE_ARRAY, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_ARRAY)) << "Variant has unexcpected type";
        ASSERT_FALSE(var2.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";

        ASSERT_TRUE(var2.isArray()) << "Variant has unexcpected type";
        pplib::Array s2 = var2.toArray();
        ASSERT_EQ(Wordlist.size(), s2.size()) << "Variant has unexcpected value";
        const pplib::Array& cs2 = var1.toArray();
        ASSERT_EQ(Wordlist.size(), cs2.size()) << "Variant has unexcpected value";
        ASSERT_THROW({ pplib::String s3 = var1.toString(); }, pplib::TypeConversionException);
    });
}

TEST_F(VariantTest, TestWithAssocArray)
{
    ASSERT_NO_THROW({
        pplib::Variant var1(TestAssocArray);
        pplib::Variant var2(var1);
        ASSERT_EQ(pplib::Variant::TYPE_ASSOCARRAY, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_ASSOCARRAY)) << "Variant has unexcpected type";
        ASSERT_FALSE(var2.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";

        ASSERT_TRUE(var2.isAssocArray()) << "Variant has unexcpected type";
        pplib::AssocArray a2 = var2.toAssocArray();
        ASSERT_EQ(TestAssocArray.size(), a2.size()) << "Variant has unexcpected value";
        const pplib::AssocArray& a2c = var1.toAssocArray();
        ASSERT_EQ(TestAssocArray.size(), a2c.size()) << "Variant has unexcpected value";
        ASSERT_THROW({ pplib::String s3 = var1.toString(); }, pplib::TypeConversionException);
        ASSERT_EQ(pplib::String("value5"), a2.getString("array1/unterkey2")) << "unexpected value";
    });
}

TEST_F(VariantTest, TestWithByteArray)
{
    pplib::ByteArray b1 = pplib::Random(2048);
    ASSERT_NO_THROW({
        pplib::Variant var1(b1);
        pplib::Variant var2(var1);
        ASSERT_EQ(pplib::Variant::TYPE_BYTEARRAY, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_BYTEARRAY)) << "Variant has unexcpected type";
        ASSERT_FALSE(var2.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";

        ASSERT_TRUE(var2.isByteArray()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isByteArrayPtr()) << "Variant has unexcpected type";
        pplib::ByteArray b2 = var2.toByteArray();
        ASSERT_EQ(b1, b2) << "Variant has unexcpected value";
        ASSERT_EQ(2048, b2.size()) << "Variant has unexcpected size";
        const pplib::ByteArray& b2c = var2.toByteArray();
        ASSERT_EQ(b1, b2c) << "Variant has unexcpected value";
        ASSERT_EQ(2048, b2c.size()) << "Variant has unexcpected size";

        ASSERT_THROW({ pplib::String s3 = var1.toString(); }, pplib::TypeConversionException);
    });
}

TEST_F(VariantTest, TestWithByteArrayPtr)
{
    pplib::ByteArray b1 = pplib::Random(2048);
    pplib::ByteArrayPtr p1 = b1;
    ASSERT_NO_THROW({
        pplib::Variant var1(p1);
        pplib::Variant var2(var1);
        ASSERT_EQ(pplib::Variant::TYPE_BYTEARRAYPTR, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_BYTEARRAYPTR)) << "Variant has unexcpected type";
        ASSERT_FALSE(var2.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";

        ASSERT_TRUE(var2.isByteArrayPtr()) << "Variant has unexcpected type";
        ASSERT_FALSE(var2.isByteArray()) << "Variant has unexcpected type";
        pplib::ByteArrayPtr p2 = var2.toByteArrayPtr();
        ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
        ASSERT_EQ(2048, p2.size()) << "Variant has unexcpected size";
        const pplib::ByteArrayPtr& p2c = var2.toByteArrayPtr();
        ASSERT_EQ(p1, p2c) << "Variant has unexcpected value";
        ASSERT_EQ(2048, p2c.size()) << "Variant has unexcpected size";

        ASSERT_THROW({ pplib::String s3 = var1.toString(); }, pplib::TypeConversionException);
    });
}

TEST_F(VariantTest, TestWithDateTime)
{
    pplib::DateTime d1("2015-12-03 15:52:40");
    ASSERT_NO_THROW({
        pplib::Variant var1(d1);
        pplib::Variant var2(var1);
        ASSERT_EQ(pplib::Variant::TYPE_DATETIME, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_DATETIME)) << "Variant has unexcpected type";
        ASSERT_FALSE(var2.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";

        ASSERT_TRUE(var2.isDateTime()) << "Variant has unexcpected type";
        pplib::DateTime d2 = var2.toDateTime();
        ASSERT_EQ(d1, d2) << "Variant has unexcpected value";
        const pplib::DateTime& d2c = var2.toDateTime();
        ASSERT_EQ(d1, d2c) << "Variant has unexcpected value";

        ASSERT_THROW({ pplib::String s3 = var1.toString(); }, pplib::TypeConversionException);
    });
}

TEST_F(VariantTest, TestSetWithEmptyVariant)
{
    pplib::Variant var1;
    pplib::Variant var2;
    var2.set(var1);
    ASSERT_EQ(pplib::Variant::TYPE_UNKNOWN, var2.type()) << "Variant has unexcpected type";
}

TEST_F(VariantTest, OperatorVariant)
{
    pplib::String s1("Hello World");
    pplib::Variant var1(s1);
    pplib::Variant var2;
    var2 = var1;
    ASSERT_EQ(s1, var2.toString()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorString)
{
    pplib::String p1("Hello World");
    pplib::Variant var1;
    var1 = p1;
    const pplib::String& p2 = var1;
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorWideString)
{
    pplib::WideString p1(L"Hello World");
    pplib::Variant var1;
    var1 = p1;
    const pplib::WideString& p2 = var1;
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorArray)
{
    pplib::Variant var1;
    var1 = Wordlist;
    const pplib::Array& p2 = var1;
    ASSERT_EQ(Wordlist.size(), p2.size()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorAssocArray)
{
    pplib::Variant var1;
    var1 = TestAssocArray;
    const pplib::AssocArray& p2 = var1;
    ASSERT_EQ(TestAssocArray.count(true), p2.count(true)) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorByteArray)
{
    pplib::ByteArray p1 = pplib::Random(2048);
    pplib::Variant var1;
    var1 = p1;
    const pplib::ByteArray& p2 = var1;
    ASSERT_EQ(p1.size(), p2.size()) << "Variant has unexcpected value";
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorByteArrayPtr)
{
    pplib::ByteArray b1 = pplib::Random(2048);
    pplib::ByteArrayPtr p1 = b1;
    pplib::Variant var1;
    var1 = p1;
    const pplib::ByteArrayPtr& p2 = var1;
    ASSERT_EQ(p1.size(), p2.size()) << "Variant has unexcpected value";
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorDateTime)
{
    pplib::DateTime p1("2015-12-03 15:52:40");
    pplib::Variant var1;
    var1 = p1;
    const pplib::DateTime& p2 = var1;
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, isString)
{
    pplib::String p1("Hello World");
    pplib::WideString p1a(L"Hello World");
    pplib::Variant var1(p1);
    pplib::Variant var2(p1a);
    ASSERT_TRUE(var1.isString());
    ASSERT_FALSE(var2.isString());
    ASSERT_FALSE(var1.isWideString());
    ASSERT_FALSE(var1.isArray());
    ASSERT_FALSE(var1.isAssocArray());
    ASSERT_FALSE(var1.isByteArray());
    ASSERT_FALSE(var1.isByteArrayPtr());
    ASSERT_FALSE(var1.isDateTime());
}

TEST_F(VariantTest, isWideString)
{
    pplib::WideString p1(L"Hello World");
    pplib::String p1a("Hello World");
    pplib::Variant var1(p1);
    pplib::Variant var2(p1a);
    ASSERT_TRUE(var1.isWideString());
    ASSERT_FALSE(var2.isWideString());
    ASSERT_FALSE(var1.isString());
    ASSERT_FALSE(var1.isArray());
    ASSERT_FALSE(var1.isAssocArray());
    ASSERT_FALSE(var1.isByteArray());
    ASSERT_FALSE(var1.isByteArrayPtr());
    ASSERT_FALSE(var1.isDateTime());
}

TEST_F(VariantTest, isArray)
{
    pplib::String p1a("Hello World");
    pplib::Variant var1(Wordlist);
    pplib::Variant var2(p1a);
    ASSERT_TRUE(var1.isArray());
    ASSERT_FALSE(var2.isArray());
    ASSERT_FALSE(var1.isString());
    ASSERT_FALSE(var1.isWideString());
    ASSERT_FALSE(var1.isAssocArray());
    ASSERT_FALSE(var1.isByteArray());
    ASSERT_FALSE(var1.isByteArrayPtr());
    ASSERT_FALSE(var1.isDateTime());
}

TEST_F(VariantTest, isAssocArray)
{
    pplib::String p1a("Hello World");
    pplib::Variant var1(TestAssocArray);
    pplib::Variant var2(p1a);
    ASSERT_TRUE(var1.isAssocArray());
    ASSERT_FALSE(var2.isAssocArray());
    ASSERT_FALSE(var1.isString());
    ASSERT_FALSE(var1.isWideString());
    ASSERT_FALSE(var1.isArray());
    ASSERT_FALSE(var1.isByteArray());
    ASSERT_FALSE(var1.isByteArrayPtr());
    ASSERT_FALSE(var1.isDateTime());
}

TEST_F(VariantTest, isByteArray)
{
    pplib::String p1a("Hello World");
    pplib::ByteArray p1 = pplib::Random(2048);
    pplib::Variant var1(p1);
    pplib::Variant var2(p1a);
    ASSERT_TRUE(var1.isByteArray());
    ASSERT_FALSE(var2.isByteArray());
    ASSERT_FALSE(var1.isString());
    ASSERT_FALSE(var1.isWideString());
    ASSERT_FALSE(var1.isArray());
    ASSERT_FALSE(var1.isAssocArray());
    ASSERT_TRUE(var1.isByteArrayPtr());
    ASSERT_FALSE(var1.isDateTime());
}

TEST_F(VariantTest, isByteArrayPtr)
{
    pplib::String p1a("Hello World");
    pplib::ByteArray b1 = pplib::Random(2048);
    pplib::ByteArrayPtr p1 = b1;
    pplib::Variant var1(p1);
    pplib::Variant var2(p1a);
    ASSERT_TRUE(var1.isByteArrayPtr());
    ASSERT_FALSE(var2.isByteArrayPtr());
    ASSERT_FALSE(var1.isString());
    ASSERT_FALSE(var1.isWideString());
    ASSERT_FALSE(var1.isArray());
    ASSERT_FALSE(var1.isAssocArray());
    ASSERT_FALSE(var1.isByteArray());
    ASSERT_FALSE(var1.isDateTime());
}

TEST_F(VariantTest, isDateTime)
{
    pplib::DateTime p1("2015-12-03 15:52:40");
    pplib::String p1a("Hello World");
    pplib::Variant var1(p1);
    pplib::Variant var2(p1a);
    ASSERT_TRUE(var1.isDateTime());
    ASSERT_FALSE(var2.isDateTime());
    ASSERT_FALSE(var1.isString());
    ASSERT_FALSE(var1.isWideString());
    ASSERT_FALSE(var1.isArray());
    ASSERT_FALSE(var1.isAssocArray());
    ASSERT_FALSE(var1.isByteArray());
    ASSERT_FALSE(var1.isByteArrayPtr());
}

TEST_F(VariantTest, toString)
{
    pplib::String p1("Hello World");
    pplib::WideString p1a(L"Hello World");
    pplib::Variant var1(p1);
    pplib::Variant var2;
    pplib::Variant var3(p1a);
    const pplib::String& p2 = var1.toString();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW({ pplib::String s3 = var2.toString(); }, pplib::EmptyDataException);

    ASSERT_THROW({ pplib::String s3 = var3.toString(); }, pplib::TypeConversionException);
}

TEST_F(VariantTest, toStringConst)
{
    pplib::String p1("Hello World");
    pplib::WideString p1a(L"Hello World");
    const pplib::Variant var1(p1);
    const pplib::Variant var2;
    const pplib::Variant var3(p1a);
    const pplib::String& p2 = var1.toString();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW(
        {
            const pplib::String& s3 = var2.toString();
            ASSERT_TRUE(s3.isEmpty());
        },
        pplib::EmptyDataException);

    ASSERT_THROW(
        {
            const pplib::String& s3 = var3.toString();
            ASSERT_TRUE(s3.isEmpty());
        },
        pplib::TypeConversionException);
}

TEST_F(VariantTest, toWideString)
{
    pplib::WideString p1(L"Hello World");
    pplib::String p1a("Hello World");
    pplib::Variant var1(p1);
    pplib::Variant var2;
    pplib::Variant var3(p1a);
    const pplib::WideString& p2 = var1.toWideString();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW({ pplib::WideString s3 = var2.toWideString(); }, pplib::EmptyDataException);

    ASSERT_THROW({ pplib::WideString s3 = var3.toWideString(); }, pplib::TypeConversionException);
}

TEST_F(VariantTest, toWideStringConst)
{
    pplib::WideString p1(L"Hello World");
    pplib::String p1a("Hello World");
    const pplib::Variant var1(p1);
    const pplib::Variant var2;
    const pplib::Variant var3(p1a);
    const pplib::WideString& p2 = var1.toWideString();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW(
        {
            const pplib::WideString& s3 = var2.toWideString();
            ASSERT_TRUE(s3.isEmpty());
        },
        pplib::EmptyDataException);
    ASSERT_THROW(
        {
            const pplib::WideString& s3 = var3.toWideString();
            ASSERT_TRUE(s3.isEmpty());
        },
        pplib::TypeConversionException);
}

TEST_F(VariantTest, toArray)
{
    pplib::String p1a("Hello World");
    pplib::Variant var1(Wordlist);
    pplib::Variant var2;
    pplib::Variant var3(p1a);
    const pplib::Array& p2 = var1.toArray();
    ASSERT_EQ(Wordlist.size(), p2.size()) << "Variant has unexcpected value";
    ASSERT_THROW({ pplib::Array s3 = var2.toArray(); }, pplib::EmptyDataException);
    ASSERT_THROW({ pplib::Array s3 = var3.toArray(); }, pplib::TypeConversionException);
}

TEST_F(VariantTest, toArrayConst)
{
    pplib::String p1a("Hello World");
    const pplib::Variant var1(Wordlist);
    const pplib::Variant var2;
    const pplib::Variant var3(p1a);
    const pplib::Array& p2 = var1.toArray();
    ASSERT_EQ(Wordlist.size(), p2.size()) << "Variant has unexcpected value";
    ASSERT_THROW(
        {
            const pplib::Array& s3 = var2.toArray();
            ASSERT_TRUE(s3.size() > 0);
        },
        pplib::EmptyDataException);
    ASSERT_THROW(
        {
            const pplib::Array& s3 = var3.toArray();
            ASSERT_TRUE(s3.size() > 0);
        },
        pplib::TypeConversionException);
}

TEST_F(VariantTest, toAssocArray)
{
    pplib::String p1a("Hello World");
    pplib::Variant var1(TestAssocArray);
    pplib::Variant var2;
    pplib::Variant var3(p1a);
    const pplib::AssocArray& p2 = var1.toAssocArray();
    ASSERT_EQ(TestAssocArray.size(), p2.size()) << "Variant has unexcpected value";
    ASSERT_THROW({ pplib::AssocArray s3 = var2.toAssocArray(); }, pplib::EmptyDataException);
    ASSERT_THROW({ pplib::AssocArray s3 = var3.toAssocArray(); }, pplib::TypeConversionException);
}

TEST_F(VariantTest, toAssocArrayConst)
{
    pplib::String p1a("Hello World");
    const pplib::Variant var1(TestAssocArray);
    const pplib::Variant var2;
    const pplib::Variant var3(p1a);
    const pplib::AssocArray& p2 = var1.toAssocArray();
    ASSERT_EQ(TestAssocArray.size(), p2.size()) << "Variant has unexcpected value";
    ASSERT_THROW(
        {
            const pplib::AssocArray& s3 = var2.toAssocArray();
            ASSERT_TRUE(s3.size() > 0);
        },
        pplib::EmptyDataException);
    ASSERT_THROW(
        {
            const pplib::AssocArray& s3 = var3.toAssocArray();
            ASSERT_TRUE(s3.size() > 0);
        },
        pplib::TypeConversionException);
}

TEST_F(VariantTest, toByteArray)
{
    pplib::String p1a("Hello World");
    pplib::ByteArray p1 = pplib::Random(2048);
    pplib::Variant var1(p1);
    pplib::Variant var2;
    pplib::Variant var3(p1a);
    pplib::ByteArray& p2 = var1.toByteArray();
    pplib::ByteArrayPtr& p4 = var1.toByteArrayPtr();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_EQ(p1, p4) << "Variant has unexcpected value";
    ASSERT_THROW({ pplib::ByteArray s3 = var2.toByteArray(); }, pplib::EmptyDataException);
    ASSERT_THROW({ pplib::ByteArrayPtr s3 = var2.toByteArrayPtr(); }, pplib::EmptyDataException);
    ASSERT_THROW({ pplib::ByteArray s3 = var3.toByteArray(); }, pplib::TypeConversionException);
    ASSERT_THROW({ pplib::ByteArrayPtr s3 = var3.toByteArrayPtr(); }, pplib::TypeConversionException);
}

TEST_F(VariantTest, toByteArrayConst)
{
    pplib::String p1a("Hello World");
    pplib::ByteArray p1 = pplib::Random(2048);
    const pplib::Variant var1(p1);
    const pplib::Variant var2;
    const pplib::Variant var3(p1a);
    const pplib::ByteArray& p2 = var1.toByteArray();
    const pplib::ByteArrayPtr& p4 = var1.toByteArrayPtr();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_EQ(p1, p4) << "Variant has unexcpected value";
    ASSERT_THROW(
        {
            const pplib::ByteArray& s3 = var2.toByteArray();
            ASSERT_TRUE(s3.isEmpty());
        },
        pplib::EmptyDataException);
    ASSERT_THROW(
        {
            const pplib::ByteArrayPtr& s3 = var2.toByteArrayPtr();
            ASSERT_TRUE(s3.isEmpty());
        },
        pplib::EmptyDataException);
    ASSERT_THROW(
        {
            const pplib::ByteArray& s3 = var3.toByteArray();
            ASSERT_TRUE(s3.isEmpty());
        },
        pplib::TypeConversionException);
    ASSERT_THROW(
        {
            const pplib::ByteArrayPtr& s3 = var3.toByteArrayPtr();
            ASSERT_TRUE(s3.isEmpty());
        },
        pplib::TypeConversionException);
}

TEST_F(VariantTest, toByteArrayPtr)
{
    pplib::String p1a("Hello World");
    pplib::ByteArray b1 = pplib::Random(2048);
    pplib::ByteArrayPtr p1 = b1;
    pplib::Variant var1(p1);
    pplib::Variant var2;
    pplib::Variant var3(p1a);
    const pplib::ByteArrayPtr& p2 = var1.toByteArrayPtr();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW({ pplib::ByteArrayPtr s3 = var2.toByteArrayPtr(); }, pplib::EmptyDataException);
    ASSERT_THROW({ pplib::ByteArrayPtr s3 = var3.toByteArrayPtr(); }, pplib::TypeConversionException);
}

TEST_F(VariantTest, toByteArrayPtrConst)
{
    pplib::String p1a("Hello World");
    pplib::ByteArray b1 = pplib::Random(2048);
    pplib::ByteArrayPtr p1 = b1;
    const pplib::Variant var1(p1);
    const pplib::Variant var2;
    const pplib::Variant var3(p1a);
    const pplib::ByteArrayPtr& p2 = var1.toByteArrayPtr();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW(
        {
            const pplib::ByteArrayPtr& s3 = var2.toByteArrayPtr();
            ASSERT_TRUE(s3.isEmpty());
        },
        pplib::EmptyDataException);
    ASSERT_THROW(
        {
            const pplib::ByteArrayPtr& s3 = var3.toByteArrayPtr();
            ASSERT_TRUE(s3.isEmpty());
        },
        pplib::TypeConversionException);
}

TEST_F(VariantTest, toDateTime)
{
    pplib::String p1a("Hello World");
    pplib::DateTime p1("2015-12-03 15:52:40");
    pplib::Variant var1(p1);
    pplib::Variant var2;
    pplib::Variant var3(p1a);
    const pplib::DateTime& p2 = var1.toDateTime();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW({ pplib::DateTime s3 = var2.toDateTime(); }, pplib::EmptyDataException);
    ASSERT_THROW({ pplib::DateTime s3 = var3.toDateTime(); }, pplib::TypeConversionException);
}

TEST_F(VariantTest, toDateTimeConst)
{
    pplib::String p1a("Hello World");
    pplib::DateTime p1("2015-12-03 15:52:40");
    const pplib::Variant var1(p1);
    const pplib::Variant var2;
    const pplib::Variant var3(p1a);
    const pplib::DateTime& p2 = var1.toDateTime();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW(
        {
            const pplib::DateTime& s3 = var2.toDateTime();
            ASSERT_TRUE(s3.isEmpty());
        },
        pplib::EmptyDataException);
    ASSERT_THROW(
        {
            const pplib::DateTime& s3 = var3.toDateTime();
            ASSERT_TRUE(s3.isEmpty());
        },
        pplib::TypeConversionException);
}

} // namespace
