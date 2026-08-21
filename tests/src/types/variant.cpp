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
        ASSERT_EQ(pplib::Variant::TYPE_STRING, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";
        ASSERT_FALSE(var2.isType(pplib::Variant::TYPE_WIDESTRING)) << "Variant has unexcpected type";

        ASSERT_TRUE(var2.isString()) << "Variant has unexcpected type";
        pplib::String s2 = var2.toString();
        ASSERT_EQ(s1, s2) << "Variant has unexcpected value";

        ASSERT_THROW({ pplib::WideString s3 = var1.toWideString(); }, pplib::TypeConversionException);

        const pplib::Variant var3 = var1;
        ASSERT_EQ(s1, var3.toString()) << "Variant has unexcpected value";
        ASSERT_EQ(s1, (const pplib::String&)var3) << "Variant has unexcpected value";
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

        const pplib::Variant var3 = var1;
        ASSERT_EQ(s1, var3.toWideString()) << "Variant has unexcpected value";
        ASSERT_EQ(s1, (const pplib::WideString&)var3) << "Variant has unexcpected value";
    });
}

TEST_F(VariantTest, TestWithArray)
{
    // pplib::Array  s1(L"Hello World");
    ASSERT_NO_THROW({
        pplib::Array testdata("the quick brown fox jumps over the lazy dog", " ");
        pplib::Variant var1(testdata);
        pplib::Variant var2(var1);
        ASSERT_EQ(pplib::Variant::TYPE_ARRAY, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_ARRAY)) << "Variant has unexcpected type";
        ASSERT_FALSE(var2.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";

        ASSERT_TRUE(var2.isArray()) << "Variant has unexcpected type";
        pplib::Array s2 = var2.toArray();
        ASSERT_EQ(testdata.size(), s2.size()) << "Variant has unexcpected value";
        const pplib::Array& cs2 = var1.toArray();
        ASSERT_EQ(testdata.size(), cs2.size()) << "Variant has unexcpected value";
        ASSERT_THROW({ pplib::String s3 = var1.toString(); }, pplib::TypeConversionException);

        const pplib::Variant var3 = var1;
        ASSERT_EQ(testdata, var3.toArray()) << "Variant has unexcpected value";
        ASSERT_EQ(testdata, (const pplib::Array&)var3) << "Variant has unexcpected value";
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

        const pplib::Variant var3 = var1;
        ASSERT_EQ(TestAssocArray, var3.toAssocArray()) << "Variant has unexcpected value";
        ASSERT_EQ(TestAssocArray, (const pplib::AssocArray&)var3) << "Variant has unexcpected value";
    });
}

TEST_F(VariantTest, TestWithByteArray)
{
    pplib::ByteArray testdata = pplib::Random(2048);
    ASSERT_NO_THROW({
        pplib::Variant var1(testdata);
        pplib::Variant var2(var1);
        ASSERT_EQ(pplib::Variant::TYPE_BYTEARRAY, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_BYTEARRAY)) << "Variant has unexcpected type";
        ASSERT_FALSE(var2.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";

        ASSERT_TRUE(var2.isByteArray()) << "Variant has unexcpected type";
        ASSERT_FALSE(var2.isByteArrayPtr()) << "Variant has unexcpected type";
        pplib::ByteArray b2 = var2.toByteArray();
        ASSERT_EQ(testdata, b2) << "Variant has unexcpected value";
        ASSERT_EQ(2048, b2.size()) << "Variant has unexcpected size";
        const pplib::ByteArray& b2c = var2.toByteArray();
        ASSERT_EQ(testdata, b2c) << "Variant has unexcpected value";
        ASSERT_EQ(2048, b2c.size()) << "Variant has unexcpected size";

        ASSERT_THROW({ pplib::String s3 = var1.toString(); }, pplib::TypeConversionException);

        const pplib::Variant var3 = var1;
        ASSERT_EQ(testdata, var3.toByteArray()) << "Variant has unexcpected value";
        ASSERT_EQ(testdata, (const pplib::ByteArray&)var3) << "Variant has unexcpected value";
    });
}

TEST_F(VariantTest, TestWithByteArrayPtr)
{
    pplib::ByteArray testdata = pplib::Random(2048);
    pplib::ByteArrayPtr p1 = testdata;
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

        const pplib::Variant var3 = var1;
        ASSERT_EQ(testdata, var3.toByteArrayPtr()) << "Variant has unexcpected value";
        ASSERT_EQ(testdata, (const pplib::ByteArrayPtr&)var3) << "Variant has unexcpected value";
    });
}

TEST_F(VariantTest, TestWithDateTime)
{
    pplib::DateTime testdata("2015-12-03 15:52:40");
    ASSERT_NO_THROW({
        pplib::Variant var1(testdata);
        pplib::Variant var2(var1);
        ASSERT_EQ(pplib::Variant::TYPE_DATETIME, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_DATETIME)) << "Variant has unexcpected type";
        ASSERT_FALSE(var2.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";

        ASSERT_TRUE(var2.isDateTime()) << "Variant has unexcpected type";
        pplib::DateTime d2 = var2.toDateTime();
        ASSERT_EQ(testdata, d2) << "Variant has unexcpected value";
        const pplib::DateTime& d2c = var2.toDateTime();
        ASSERT_EQ(testdata, d2c) << "Variant has unexcpected value";

        ASSERT_THROW({ pplib::String s3 = var1.toString(); }, pplib::TypeConversionException);

        const pplib::Variant var3 = var1;
        ASSERT_EQ(testdata, var3.toDateTime()) << "Variant has unexcpected value";
        ASSERT_EQ(testdata, (const pplib::DateTime&)var3) << "Variant has unexcpected value";
    });
}

TEST_F(VariantTest, TestWithDate)
{
    pplib::Date testdata("2015-12-03");
    ASSERT_NO_THROW({
        pplib::Variant var1(testdata);
        pplib::Variant var2(var1);
        ASSERT_EQ(pplib::Variant::TYPE_DATE, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_DATE)) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isDate()) << "Variant has unexcpected type";
        pplib::Date d2 = var2.toDate();
        ASSERT_EQ(testdata, d2) << "Variant has unexcpected value";
        ASSERT_EQ(testdata, (pplib::Date&)var2) << "Variant has unexcpected value";
        ASSERT_THROW({ pplib::String s3 = var1.toString(); }, pplib::TypeConversionException);

        const pplib::Variant var3 = var1;
        ASSERT_EQ(testdata, var3.toDate()) << "Variant has unexcpected value";
        ASSERT_EQ(testdata, (const pplib::Date&)var3) << "Variant has unexcpected value";
    });
}

TEST_F(VariantTest, TestWithTime)
{
    pplib::Time testdata("15:52:40");
    ASSERT_NO_THROW({
        pplib::Variant var1(testdata);
        pplib::Variant var2(var1);
        ASSERT_EQ(pplib::Variant::TYPE_TIME, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_TIME)) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isTime()) << "Variant has unexcpected type";
        pplib::Time t2 = var2.toTime();
        ASSERT_EQ(testdata, t2) << "Variant has unexcpected value";
        ASSERT_EQ(testdata, (pplib::Time&)var2) << "Variant has unexcpected value";

        const pplib::Variant var3 = var1;
        ASSERT_EQ(testdata, var3.toTime()) << "Variant has unexcpected value";
        ASSERT_EQ(testdata, (const pplib::Time&)var3) << "Variant has unexcpected value";
    });
}

TEST_F(VariantTest, TestWithTimeDelta)
{
    pplib::TimeDelta testdata("15:52:40");
    ASSERT_NO_THROW({
        pplib::Variant var1(testdata);
        pplib::Variant var2(var1);
        ASSERT_EQ(pplib::Variant::TYPE_TIMEDELTA, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_TIMEDELTA)) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isTimeDelta()) << "Variant has unexcpected type";
        pplib::TimeDelta td2 = var2.toTimeDelta();
        ASSERT_EQ(testdata, td2) << "Variant has unexcpected value";
        ASSERT_EQ(testdata, (pplib::TimeDelta&)var2) << "Variant has unexcpected value";

        const pplib::Variant var3 = var1;
        ASSERT_EQ(testdata, var3.toTimeDelta()) << "Variant has unexcpected value";
        ASSERT_EQ(testdata, (const pplib::TimeDelta&)var3) << "Variant has unexcpected value";
    });
}

TEST_F(VariantTest, TestWithTimeZone)
{
    pplib::TimeZone testdata(2, 0, "Europe/Berlin");
    ASSERT_NO_THROW({
        pplib::Variant var1(testdata);
        pplib::Variant var2(var1);
        ASSERT_EQ(pplib::Variant::TYPE_TIMEZONE, var2.type()) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_TIMEZONE)) << "Variant has unexcpected type";
        ASSERT_TRUE(var2.isTimeZone()) << "Variant has unexcpected type";
        pplib::TimeZone tz2 = var2.toTimeZone();
        ASSERT_EQ(testdata, tz2) << "Variant has unexcpected value";
        ASSERT_EQ(testdata, (pplib::TimeZone&)var2) << "Variant has unexcpected value";

        const pplib::Variant var3 = var1;
        ASSERT_EQ(testdata, var3.toTimeZone()) << "Variant has unexcpected value";
        ASSERT_EQ(testdata, (const pplib::TimeZone&)var3) << "Variant has unexcpected value";
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

TEST_F(VariantTest, MoveOperatorVariant)
{
    pplib::String s1("Hello World");
    pplib::Variant var1(s1);
    pplib::Variant var2;
    var2 = std::move(var1);
    ASSERT_EQ(pplib::Variant::TYPE_STRING, var2.type()) << "Variant has unexcpected type";
    ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";

    ASSERT_EQ(s1, var2.toString()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, MoveOperatorVariantEqual)
{
    pplib::String s1("Hello World");
    pplib::Variant var1(s1);
    var1 = std::move(var1);
    ASSERT_EQ(pplib::Variant::TYPE_STRING, var1.type()) << "Variant has unexcpected type";
    ASSERT_TRUE(var1.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";

    ASSERT_EQ(s1, var1.toString()) << "Variant has unexcpected value";
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

TEST_F(VariantTest, OperatorDate)
{
    pplib::Date p1("2015-12-03");
    pplib::Variant var1;
    var1 = p1;
    const pplib::Date& p2 = var1;
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorTime)
{
    pplib::Time p1("15:52:40");
    pplib::Variant var1;
    var1 = p1;
    const pplib::Time& p2 = var1;
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorTimeDelta)
{
    pplib::TimeDelta p1("15:52:40");
    pplib::Variant var1;
    var1 = p1;
    const pplib::TimeDelta& p2 = var1;
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorTimeZone)
{
    pplib::TimeZone p1(2, 0, "Europe/Berlin");
    pplib::Variant var1;
    var1 = p1;
    const pplib::TimeZone& p2 = var1;
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, setWithVariantMoveOperator)
{
    pplib::String s1("Hello World");
    pplib::Variant var1(std::move(s1));
    pplib::Variant var2;
    var2.set(std::move(var1));
    ASSERT_EQ(pplib::Variant::TYPE_STRING, var2.type()) << "Variant has unexcpected type";
    ASSERT_EQ(pplib::String("Hello World"), var2.toString()) << "Variant has unexcpected value";
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
    ASSERT_FALSE(var1.isByteArrayPtr());
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

TEST_F(VariantTest, isDate)
{
    pplib::Date p1("2015-12-03");
    pplib::String p1a("Hello World");
    pplib::Variant var1(p1);
    pplib::Variant var2(p1a);
    ASSERT_TRUE(var1.isDate());
    ASSERT_FALSE(var2.isDate());
}

TEST_F(VariantTest, isTime)
{
    pplib::Time p1("15:52:40");
    pplib::String p1a("Hello World");
    pplib::Variant var1(p1);
    pplib::Variant var2(p1a);
    ASSERT_TRUE(var1.isTime());
    ASSERT_FALSE(var2.isTime());
}

TEST_F(VariantTest, isTimeDelta)
{
    pplib::TimeDelta p1("15:52:40");
    pplib::String p1a("Hello World");
    pplib::Variant var1(p1);
    pplib::Variant var2(p1a);
    ASSERT_TRUE(var1.isTimeDelta());
    ASSERT_FALSE(var2.isTimeDelta());
}

TEST_F(VariantTest, isTimeZone)
{
    pplib::TimeZone p1(2, 0, "Europe/Berlin");
    pplib::String p1a("Hello World");
    pplib::Variant var1(p1);
    pplib::Variant var2(p1a);
    ASSERT_TRUE(var1.isTimeZone());
    ASSERT_FALSE(var2.isTimeZone());
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

TEST_F(VariantTest, toDate)
{
    pplib::String p1a("Hello World");
    pplib::Date p1("2015-12-03");
    pplib::Variant var1(p1);
    pplib::Variant var2;
    pplib::Variant var3(p1a);
    const pplib::Date& p2 = var1.toDate();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW({ pplib::Date s3 = var2.toDate(); }, pplib::EmptyDataException);
    ASSERT_THROW({ pplib::Date s3 = var3.toDate(); }, pplib::TypeConversionException);
}

TEST_F(VariantTest, toDateConst)
{
    pplib::String p1a("Hello World");
    pplib::Date p1("2015-12-03");
    const pplib::Variant var1(p1);
    const pplib::Variant var2;
    const pplib::Variant var3(p1a);
    const pplib::Date& p2 = var1.toDate();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW(
        {
            const pplib::Date& s3 = var2.toDate();
            ASSERT_TRUE(s3.isEmpty());
        },
        pplib::EmptyDataException);
    ASSERT_THROW(
        {
            const pplib::Date& s3 = var3.toDate();
            ASSERT_TRUE(s3.isEmpty());
        },
        pplib::TypeConversionException);
}

TEST_F(VariantTest, toTime)
{
    pplib::String p1a("Hello World");
    pplib::Time p1("15:52:40");
    pplib::Variant var1(p1);
    pplib::Variant var2;
    pplib::Variant var3(p1a);
    const pplib::Time& p2 = var1.toTime();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW({ pplib::Time s3 = var2.toTime(); }, pplib::EmptyDataException);
    ASSERT_THROW({ pplib::Time s3 = var3.toTime(); }, pplib::TypeConversionException);
}

TEST_F(VariantTest, toTimeConst)
{
    pplib::String p1a("Hello World");
    pplib::Time p1("15:52:40");
    const pplib::Variant var1(p1);
    const pplib::Variant var2;
    const pplib::Variant var3(p1a);
    const pplib::Time& p2 = var1.toTime();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW(
        {
            const pplib::Time& s3 = var2.toTime();
            ASSERT_EQ(pplib::Time(), s3) << "Variant has unexcpected value";
        },
        pplib::EmptyDataException);
    ASSERT_THROW(
        {
            const pplib::Time& s3 = var3.toTime();
            ASSERT_EQ(pplib::Time(), s3) << "Variant has unexcpected value";
        },
        pplib::TypeConversionException);
}

TEST_F(VariantTest, toTimeDelta)
{
    pplib::String p1a("Hello World");
    pplib::TimeDelta p1("15:52:40");
    pplib::Variant var1(p1);
    pplib::Variant var2;
    pplib::Variant var3(p1a);
    const pplib::TimeDelta& p2 = var1.toTimeDelta();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW({ pplib::TimeDelta s3 = var2.toTimeDelta(); }, pplib::EmptyDataException);
    ASSERT_THROW({ pplib::TimeDelta s3 = var3.toTimeDelta(); }, pplib::TypeConversionException);
}

TEST_F(VariantTest, toTimeDeltaConst)
{
    pplib::String p1a("Hello World");
    pplib::TimeDelta p1("15:52:40");
    const pplib::Variant var1(p1);
    const pplib::Variant var2;
    const pplib::Variant var3(p1a);
    const pplib::TimeDelta& p2 = var1.toTimeDelta();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW(
        {
            const pplib::TimeDelta& s3 = var2.toTimeDelta();
            ASSERT_EQ(pplib::TimeDelta(), s3) << "Variant has unexcpected value";
        },
        pplib::EmptyDataException);
    ASSERT_THROW(
        {
            const pplib::TimeDelta& s3 = var3.toTimeDelta();
            ASSERT_EQ(pplib::TimeDelta(), s3) << "Variant has unexcpected value";
        },
        pplib::TypeConversionException);
}

TEST_F(VariantTest, toTimeZone)
{
    pplib::String p1a("Hello World");
    pplib::TimeZone p1(2, 0, "Europe/Berlin");
    pplib::Variant var1(p1);
    pplib::Variant var2;
    pplib::Variant var3(p1a);
    const pplib::TimeZone& p2 = var1.toTimeZone();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW({ pplib::TimeZone s3 = var2.toTimeZone(); }, pplib::EmptyDataException);
    ASSERT_THROW({ pplib::TimeZone s3 = var3.toTimeZone(); }, pplib::TypeConversionException);
}

TEST_F(VariantTest, toTimeZoneConst)
{
    pplib::String p1a("Hello World");
    pplib::TimeZone p1(2, 0, "Europe/Berlin");
    const pplib::Variant var1(p1);
    const pplib::Variant var2;
    const pplib::Variant var3(p1a);
    const pplib::TimeZone& p2 = var1.toTimeZone();
    ASSERT_EQ(p1, p2) << "Variant has unexcpected value";
    ASSERT_THROW(
        {
            const pplib::TimeZone& s3 = var2.toTimeZone();
            ASSERT_EQ(pplib::TimeZone(), s3) << "Variant has unexcpected value";
        },
        pplib::EmptyDataException);
    ASSERT_THROW(
        {
            const pplib::TimeZone& s3 = var3.toTimeZone();
            ASSERT_EQ(pplib::TimeZone(), s3) << "Variant has unexcpected value";
        },
        pplib::TypeConversionException);
}

TEST_F(VariantTest, CopyConstructorVariant)
{
    pplib::String s1("Hello World");
    pplib::Variant var1(s1);
    pplib::Variant var2(var1);
    ASSERT_EQ(pplib::Variant::TYPE_STRING, var2.type()) << "Variant has unexcpected type";
    ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";
    ASSERT_TRUE(var2.isString()) << "Variant has unexcpected type";
    pplib::String s2 = var2.toString();
    ASSERT_EQ(s1, s2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, MoveConstructorVariant)
{
    pplib::String s1("Hello World");
    pplib::Variant var1(s1);
    pplib::Variant var2(std::move(var1));
    ASSERT_EQ(pplib::Variant::TYPE_STRING, var2.type()) << "Variant has unexcpected type";
    ASSERT_TRUE(var2.isType(pplib::Variant::TYPE_STRING)) << "Variant has unexcpected type";
    ASSERT_TRUE(var2.isString()) << "Variant has unexcpected type";
    pplib::String s2 = var2.toString();
    ASSERT_EQ(s1, s2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, SetWithMoveOperatorAndString)
{
    pplib::String s1("Hello World");
    pplib::Variant var1;
    var1.set(std::move(s1));
    ASSERT_EQ(pplib::Variant::TYPE_STRING, var1.type()) << "Variant has unexcpected type";
    ASSERT_EQ(pplib::String("Hello World"), var1.toString()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, SetWithMoveOperatorAndWideString)
{
    pplib::WideString s1(L"Hello World");
    pplib::Variant var1;
    var1.set(std::move(s1));
    ASSERT_EQ(pplib::Variant::TYPE_WIDESTRING, var1.type()) << "Variant has unexcpected type";
    ASSERT_EQ(pplib::WideString(L"Hello World"), var1.toWideString()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, SetWithMoveOperatorAndArray)
{
    pplib::Variant var1;
    pplib::Array a1("red green blue white black yellow", " ");
    pplib::Array a2 = a1;
    var1.set(std::move(a2));
    ASSERT_EQ(pplib::Variant::TYPE_ARRAY, var1.type()) << "Variant has unexcpected type";
    ASSERT_EQ(a1, var1.toArray()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, SetWithMoveOperatorAndAssocArray)
{
    pplib::Variant var1;
    pplib::AssocArray a1;
    a1.set("key1", "value1");
    a1.set("key2", "value2");
    pplib::AssocArray a2 = a1;
    var1.set(std::move(a2));
    ASSERT_EQ(pplib::Variant::TYPE_ASSOCARRAY, var1.type()) << "Variant has unexcpected type";
    ASSERT_EQ(a1, var1.toAssocArray()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, SetWithMoveOperatorAndByteArray)
{
    pplib::Variant var1;
    pplib::ByteArray b1 = pplib::Random(2048);
    pplib::ByteArray b2 = b1;
    var1.set(std::move(b2));
    ASSERT_EQ(pplib::Variant::TYPE_BYTEARRAY, var1.type()) << "Variant has unexcpected type";
    ASSERT_EQ(b1, var1.toByteArray()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, SetWithMoveOperatorAndByteArrayPtr)
{
    pplib::Variant var1;
    pplib::ByteArray b1 = pplib::Random(2048);
    pplib::ByteArrayPtr p1 = b1;
    pplib::ByteArrayPtr p2 = p1;
    var1.set(std::move(p2));
    ASSERT_EQ(pplib::Variant::TYPE_BYTEARRAYPTR, var1.type()) << "Variant has unexcpected type";
    ASSERT_EQ(p1, var1.toByteArrayPtr()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, SetWithMoveOperatorAndDateTime)
{
    pplib::Variant var1;
    pplib::DateTime d1("2015-12-03 15:52:40");
    pplib::DateTime d2 = d1;
    var1.set(std::move(d2));
    ASSERT_EQ(pplib::Variant::TYPE_DATETIME, var1.type()) << "Variant has unexcpected type";
    ASSERT_EQ(d1, var1.toDateTime()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, SetWithMoveOperatorAndDate)
{
    pplib::Variant var1;
    pplib::Date d1("2015-12-03");
    pplib::Date d2 = d1;
    var1.set(std::move(d2));
    ASSERT_EQ(pplib::Variant::TYPE_DATE, var1.type()) << "Variant has unexcpected type";
    ASSERT_EQ(d1, var1.toDate()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, SetWithMoveOperatorAndTime)
{
    pplib::Variant var1;
    pplib::Time t1("15:52:40");
    pplib::Time t2 = t1;
    var1.set(std::move(t2));
    ASSERT_EQ(pplib::Variant::TYPE_TIME, var1.type()) << "Variant has unexcpected type";
    ASSERT_EQ(t1, var1.toTime()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, SetWithMoveOperatorAndTimeDelta)
{
    pplib::Variant var1;
    pplib::TimeDelta td1("15:52:40");
    pplib::TimeDelta td2 = td1;
    var1.set(std::move(td2));
    ASSERT_EQ(pplib::Variant::TYPE_TIMEDELTA, var1.type()) << "Variant has unexcpected type";
    ASSERT_EQ(td1, var1.toTimeDelta()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, SetWithMoveOperatorAndTimeZone)
{
    pplib::Variant var1;
    pplib::TimeZone tz1(2, 0, "Europe/Berlin");
    pplib::TimeZone tz2 = tz1;
    var1.set(std::move(tz2));
    ASSERT_EQ(pplib::Variant::TYPE_TIMEZONE, var1.type()) << "Variant has unexcpected type";
    ASSERT_EQ(tz1, var1.toTimeZone()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, CompareOperatorDifferentTypes)
{
    pplib::String s1("Hello World");
    pplib::WideString s2(L"Hello World");
    pplib::Variant var1(s1);
    pplib::Variant var2(s2);
    ASSERT_FALSE(var1 == var2) << "Variant has unexcpected value";
    ASSERT_TRUE(var1 != var2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, CompareOperatorWithString)
{
    pplib::String s1("Hello World");
    pplib::Variant var1(s1);
    pplib::Variant var2(s1);
    ASSERT_TRUE(var1 == var2) << "Variant has unexcpected value";
    ASSERT_FALSE(var1 != var2) << "Variant has unexcpected value";
}
TEST_F(VariantTest, CompareOperatorWithWideString)
{
    pplib::WideString s1(L"Hello World");
    pplib::Variant var1(s1);
    pplib::Variant var2(s1);
    ASSERT_TRUE(var1 == var2) << "Variant has unexcpected value";
    ASSERT_FALSE(var1 != var2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, CompareOperatorWithArray)
{
    pplib::Variant var1(pplib::Array("red green blue white black yellow", " "));
    pplib::Variant var2(pplib::Array("red green blue white black yellow", " "));
    ASSERT_TRUE(var1 == var2) << "Variant has unexcpected value";
    ASSERT_FALSE(var1 != var2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, CompareOperatorWithAssocArray)
{
    pplib::AssocArray a1;
    a1.set("key1", "value1");
    a1.set("key2", "value2");
    pplib::Variant var1(a1);
    pplib::AssocArray a2;
    a2.set("key1", "value1");
    a2.set("key2", "value2");
    pplib::Variant var2(a2);
    ASSERT_TRUE(var1 == var2) << "Variant has unexcpected value";
    ASSERT_FALSE(var1 != var2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, CompareOperatorWithByteArray)
{
    pplib::ByteArray b1 = pplib::Random(2048);
    pplib::Variant var1(b1);
    pplib::ByteArray b2 = b1;
    pplib::Variant var2(b2);
    ASSERT_TRUE(var1 == var2) << "Variant has unexcpected value";
    ASSERT_FALSE(var1 != var2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, CompareOperatorWithByteArrayPtr)
{
    pplib::ByteArray b1 = pplib::Random(2048);
    pplib::ByteArrayPtr p1 = b1;
    pplib::Variant var1(p1);
    pplib::ByteArrayPtr p2 = p1;
    pplib::Variant var2(p2);
    ASSERT_TRUE(var1 == var2) << "Variant has unexcpected value";
    ASSERT_FALSE(var1 != var2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, CompareOperatorWithDateTime)
{
    pplib::DateTime d1("2015-12-03 15:52:40");
    pplib::Variant var1(d1);
    pplib::DateTime d2 = d1;
    pplib::Variant var2(d2);
    ASSERT_TRUE(var1 == var2) << "Variant has unexcpected value";
    ASSERT_FALSE(var1 != var2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, CompareOperatorWithDate)
{
    pplib::Date d1("2015-12-03");
    pplib::Variant var1(d1);
    pplib::Date d2 = d1;
    pplib::Variant var2(d2);
    ASSERT_TRUE(var1 == var2) << "Variant has unexcpected value";
    ASSERT_FALSE(var1 != var2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, CompareOperatorWithTime)
{
    pplib::Time t1("15:52:40");
    pplib::Variant var1(t1);
    pplib::Time t2 = t1;
    pplib::Variant var2(t2);
    ASSERT_TRUE(var1 == var2) << "Variant has unexcpected value";
    ASSERT_FALSE(var1 != var2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, CompareOperatorWithTimeDelta)
{
    pplib::TimeDelta td1("15:52:40");
    pplib::Variant var1(td1);
    pplib::TimeDelta td2 = td1;
    pplib::Variant var2(td2);
    ASSERT_TRUE(var1 == var2) << "Variant has unexcpected value";
    ASSERT_FALSE(var1 != var2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, CompareOperatorWithTimeZone)
{
    pplib::TimeZone tz1(2, 0, "Europe/Berlin");
    pplib::Variant var1(tz1);
    pplib::TimeZone tz2 = tz1;
    pplib::Variant var2(tz2);
    ASSERT_TRUE(var1 == var2) << "Variant has unexcpected value";
    ASSERT_FALSE(var1 != var2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, CompareOperatorWithDifferentValues)
{
    pplib::String s1("Hello World");
    pplib::String s2("Hello World!");
    pplib::Variant var1(s1);
    pplib::Variant var2(s2);
    ASSERT_FALSE(var1 == var2) << "Variant has unexcpected value";
    ASSERT_TRUE(var1 != var2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, CompareOperatorWithUnknownType)
{
    pplib::Variant var1;
    pplib::Variant var2;
    ASSERT_TRUE(var1 == var2) << "Variant has unexcpected value";
    ASSERT_FALSE(var1 != var2) << "Variant has unexcpected value";
}

// Move constructor and move assignment operator tests
TEST_F(VariantTest, MoveString)
{
    // Move constructor
    pplib::String s1("Hello World");
    pplib::Variant var1(std::move(s1));
    ASSERT_EQ(pplib::String("Hello World"), var1.toString()) << "Variant has unexcpected value";

    // Move assignment operator
    pplib::String s2("Hello World");
    pplib::Variant var2;
    var2 = std::move(s2);
    ASSERT_EQ(pplib::String("Hello World"), var2.toString()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, MoveWideString)
{
    // Move constructor
    pplib::WideString s1(L"Hello World");
    pplib::Variant var1(std::move(s1));
    ASSERT_EQ(pplib::WideString(L"Hello World"), var1.toWideString()) << "Variant has unexcpected value";

    // Move assignment operator
    pplib::WideString s2(L"Hello World");
    pplib::Variant var2;
    var2 = std::move(s2);
    ASSERT_EQ(pplib::WideString(L"Hello World"), var2.toWideString()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, MoveArray)
{
    // Move constructor
    pplib::Array a1("red green blue white black yellow", " ");
    pplib::Variant var1(std::move(a1));
    ASSERT_EQ(pplib::Array("red green blue white black yellow", " "), var1.toArray()) << "Variant has unexcpected value";

    // Move assignment operator
    pplib::Array a2("red green blue white black yellow", " ");
    pplib::Variant var2;
    var2 = std::move(a2);
    ASSERT_EQ(pplib::Array("red green blue white black yellow", " "), var2.toArray()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, MoveAssocArray)
{
    // Testdata
    pplib::AssocArray expected;
    expected.set("key1", "value1");
    expected.set("key2", "value2");

    // Move constructor
    pplib::AssocArray a1 = expected;
    pplib::Variant var1(std::move(a1));
    ASSERT_EQ(expected, var1.toAssocArray()) << "Variant has unexcpected value";

    // Move assignment operator
    pplib::AssocArray a2 = expected;
    pplib::Variant var2;
    var2 = std::move(a2);
    ASSERT_EQ(expected, var2.toAssocArray()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, MoveByteArray)
{
    // Testdata
    pplib::ByteArray expected = pplib::Random(2048);

    // Move constructor
    pplib::ByteArray b1 = expected;
    pplib::Variant var1(std::move(b1));
    ASSERT_EQ(expected, var1.toByteArray()) << "Variant has unexcpected value";

    // Move assignment operator
    pplib::ByteArray b2 = expected;
    pplib::Variant var2;
    var2 = std::move(b2);
    ASSERT_EQ(expected, var2.toByteArray()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, MoveByteArrayPtr)
{
    // Testdata
    pplib::ByteArray expected = pplib::Random(2048);
    pplib::ByteArrayPtr p1 = expected;

    // Move constructor
    pplib::ByteArrayPtr p2 = p1;
    pplib::Variant var1(std::move(p2));
    ASSERT_EQ(p1, var1.toByteArrayPtr()) << "Variant has unexcpected value";

    // Move assignment operator
    pplib::ByteArrayPtr p3 = p1;
    pplib::Variant var2;
    var2 = std::move(p3);
    ASSERT_EQ(p1, var2.toByteArrayPtr()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, MoveDateTime)
{
    // Testdata
    pplib::DateTime expected("2015-12-03 15:52:40");

    // Move constructor
    pplib::DateTime d1 = expected;
    pplib::Variant var1(std::move(d1));
    ASSERT_EQ(expected, var1.toDateTime()) << "Variant has unexcpected value";

    // Move assignment operator
    pplib::DateTime d2 = expected;
    pplib::Variant var2;
    var2 = std::move(d2);
    ASSERT_EQ(expected, var2.toDateTime()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, MoveDate)
{
    // Testdata
    pplib::Date expected("2015-12-03");

    // Move constructor
    pplib::Date d1 = expected;
    pplib::Variant var1(std::move(d1));
    ASSERT_EQ(expected, var1.toDate()) << "Variant has unexcpected value";

    // Move assignment operator
    pplib::Date d2 = expected;
    pplib::Variant var2;
    var2 = std::move(d2);
    ASSERT_EQ(expected, var2.toDate()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, MoveTime)
{
    // Testdata
    pplib::Time expected("15:52:40");

    // Move constructor
    pplib::Time t1 = expected;
    pplib::Variant var1(std::move(t1));
    ASSERT_EQ(expected, var1.toTime()) << "Variant has unexcpected value";

    // Move assignment operator
    pplib::Time t2 = expected;
    pplib::Variant var2;
    var2 = std::move(t2);
    ASSERT_EQ(expected, var2.toTime()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, MoveTimeDelta)
{
    // Testdata
    pplib::TimeDelta expected("15:52:40");

    // Move constructor
    pplib::TimeDelta td1 = expected;
    pplib::Variant var1(std::move(td1));
    ASSERT_EQ(expected, var1.toTimeDelta()) << "Variant has unexcpected value";

    // Move assignment operator
    pplib::TimeDelta td2 = expected;
    pplib::Variant var2;
    var2 = std::move(td2);
    ASSERT_EQ(expected, var2.toTimeDelta()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, MoveTimeZone)
{
    // Testdata
    pplib::TimeZone expected(2, 0, "Europe/Berlin");

    // Move constructor
    pplib::TimeZone tz1 = expected;
    pplib::Variant var1(std::move(tz1));
    ASSERT_EQ(expected, var1.toTimeZone()) << "Variant has unexcpected value";

    // Move assignment operator
    pplib::TimeZone tz2 = expected;
    pplib::Variant var2;
    var2 = std::move(tz2);
    ASSERT_EQ(expected, var2.toTimeZone()) << "Variant has unexcpected value";
}

} // namespace
