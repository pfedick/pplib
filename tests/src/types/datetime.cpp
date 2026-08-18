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
#include <gtest/gtest.h>

#include <pplib/types/datetime.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

#include "pplib-tests.h"

namespace
{
// The fixture for testing class Foo.
class DateTimeTest : public ::testing::Test
{
protected:
    DateTimeTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~DateTimeTest()
    {
    }
};

TEST_F(DateTimeTest, ConstructorSimple)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1;
        ASSERT_TRUE(d1.isEmpty() == true) << "Class is not Empty";
    });
}

TEST_F(DateTimeTest, ConstructorWithDate)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18");
        ASSERT_TRUE(d1.notEmpty()) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18"), d1.getDate()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 00:00:00"), d1.get()) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, ConstructorWithDateTime)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18 11:50:11");
        ASSERT_TRUE(d1.notEmpty() == true) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18"), d1.getDate()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11"), d1.get()) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, ConstructorWithDateTimeMsec1)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18 11:50:11.1");
        ASSERT_TRUE(d1.notEmpty() == true) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18"), d1.getDate()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11"), d1.get()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11.000001"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, ConstructorWithDateTimeMsec2)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18 11:50:11.15");
        ASSERT_TRUE(d1.notEmpty() == true) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18"), d1.getDate()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11"), d1.get()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11.000015"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, ConstructorWithDateTimeMsec3)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18 11:50:11.159");
        ASSERT_TRUE(d1.notEmpty() == true) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18"), d1.getDate()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11"), d1.get()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11.000159"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, ConstructorWithDateTimeUsec4)
{
    ASSERT_EQ(pplib::String("2012-05-18 11:50:11.001594"), pplib::DateTime("2012-05-18 11:50:11.1594").get("%Y-%m-%d %H:%M:%S.%u"))
        << "Unexpected date";
}

TEST_F(DateTimeTest, ConstructorWithDateTimeUsec5)
{
    ASSERT_EQ(pplib::String("2012-05-18 11:50:11.015947"), pplib::DateTime("2012-05-18 11:50:11.15947").get("%Y-%m-%d %H:%M:%S.%u"))
        << "Unexpected date";
}

TEST_F(DateTimeTest, ConstructorWithDateTimeUsec6)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18 11:50:11.159473");
        ASSERT_TRUE(d1.notEmpty() == true) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18"), d1.getDate()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11"), d1.get()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11.159473"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, ConstructorWithISO8601DateTimeMsec1)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.1");
        ASSERT_TRUE(d1.notEmpty() == true) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18"), d1.getDate()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11"), d1.get()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11.000001"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, ConstructorWithISO8601DateTimeMsec2)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.15");
        ASSERT_TRUE(d1.notEmpty() == true) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18"), d1.getDate()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11"), d1.get()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11.000015"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, ConstructorWithISO8601DateTimeMsec3)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159");
        ASSERT_TRUE(d1.notEmpty() == true) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18"), d1.getDate()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11"), d1.get()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11.000159"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, ConstructorWithISO8601DateTimeUsec6)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        ASSERT_TRUE(d1.notEmpty() == true) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18"), d1.getDate()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11"), d1.get()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11.159473"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, ConstructorWithISO8601DateTimeUsec6TZ)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473+0200");
        ASSERT_TRUE(d1.notEmpty() == true) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18"), d1.getDate()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11"), d1.get()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11.159473"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, ConstructorWithISO8601DateTimeUsec6TZ1)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473-0200");
        ASSERT_TRUE(d1.notEmpty() == true) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18"), d1.getDate()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11"), d1.get()) << "Unexpected date";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11.159473"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, CopyConstructor)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18 11:50:11.159473");
        pplib::DateTime d2 = d1;
        ASSERT_TRUE(d2.notEmpty() == true) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11.159473"), d2.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, MoveConstructor)
{
    pplib::DateTime d1("2012-05-18 11:50:11.159473");
    pplib::DateTime d2 = std::move(d1);
    ASSERT_TRUE(d2.notEmpty() == true) << "Class is empty";
    ASSERT_EQ(pplib::String("2012-05-18 11:50:11.159473"), d2.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
}

TEST_F(DateTimeTest, ConstructorWithTime_t)
{
    uint64_t t = 1337342550; // 2012-05-18 12:02:30 UTC
    ASSERT_NO_THROW({
        pplib::DateTime d1(t);
        ASSERT_TRUE(d1.notEmpty() == true) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18 12:02:30.000000"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, Assignment)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18 11:50:11.159473");
        pplib::DateTime d2;
        d2 = d1;
        ASSERT_TRUE(d2.notEmpty() == true) << "Class is empty";
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11.159473"), d2.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, set)
{
    pplib::DateTime d1;
    EXPECT_EQ(pplib::String("0000-00-00 00:00:00.000000"), d1.set("null").get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    EXPECT_EQ(pplib::String("0000-00-00 00:00:00.000000"), d1.set("").get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    EXPECT_EQ(pplib::String("0000-00-00 00:00:00.000000"), d1.set("T").get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    EXPECT_EQ(pplib::String("0000-00-00 00:00:00.000000"), d1.set("0").get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 08:35:01.012345"), d1.set("2026-08-16 08:35:01.012345").get("%Y-%m-%d %H:%M:%S.%u"))
        << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 08:35:01.012345"), d1.set("16.08.2026 08:35:01.012345").get("%Y-%m-%d %H:%M:%S.%u"))
        << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 08:35:01.000123"), d1.set("16.08.2026 08:35:01.123").get("%Y-%m-%d %H:%M:%S.%u"))
        << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 08:07:01.000000"), d1.set("16.8.2026 8:7:1").get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 00:00:00.000000"), d1.set("16.8.2026").get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 00:00:00.000000"), d1.set("2026.08.16").get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 00:00:00.000000"), d1.set("2026-08-16").get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";

    EXPECT_EQ(pplib::String("2026-08-16 08:35:01.012345"), d1.set("2026-08-16T08:35:01.012345").get("%Y-%m-%d %H:%M:%S.%u"))
        << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 08:35:01.000012"), d1.set("2026-08-16T08:35:01.012").get("%Y-%m-%d %H:%M:%S.%u"))
        << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 08:35:01.000000"), d1.set("2026-08-16T08:35:01").get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 08:35:01.123456"), d1.set("2026.08.16 08:35:01.123456").get("%Y-%m-%d %H:%M:%S.%u"))
        << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 08:35:01.123456"), d1.set("2026.8.16 8:35:1.123456").get("%Y-%m-%d %H:%M:%S.%u"))
        << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 08:35:01.000123"), d1.set("2026.8.16 8:35:1.123").get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 08:35:01.000000"), d1.set("2026.8.16 8:35:1").get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
}

TEST_F(DateTimeTest, setWithTimeZone)
{
    pplib::DateTime d1;
    EXPECT_EQ(pplib::String("2026-08-16 08:35:01.123456"), d1.set("2026-08-16T08:35:01.123456[+02:00]").get("%Y-%m-%d %H:%M:%S.%u"))
        << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 08:35:01.123456"), d1.set("2026-08-16T08:35:01.123456[+00:00]").get("%Y-%m-%d %H:%M:%S.%u"))
        << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 08:35:01.000123"), d1.set("2026-08-16T08:35:01.123[+02:00]").get("%Y-%m-%d %H:%M:%S.%u"))
        << "Unexpected date";
    EXPECT_EQ(pplib::String("2026-08-16 08:35:01.000000"), d1.set("2026-08-16T08:35:01[+02:00]").get("%Y-%m-%d %H:%M:%S.%u"))
        << "Unexpected date";
}

TEST_F(DateTimeTest, setWithTimeZoneVariations)
{
    pplib::DateTime d1;
    d1.set("2026-08-16T08:35:01.123456[-02:00]");
    EXPECT_EQ(pplib::String("2026-08-16T08:35:01.123456-02:00"), d1.getISO8601withUsec());
    d1.set("2026-08-16T08:35:01.123456[+02:00]");
    EXPECT_EQ(pplib::String("2026-08-16T08:35:01.123456+02:00"), d1.getISO8601withUsec());

    d1.set("2026-08-16T08:35:01.123456[-0200]");
    EXPECT_EQ(pplib::String("2026-08-16T08:35:01.123456-02:00"), d1.getISO8601withUsec());
    d1.set("2026-08-16T08:35:01.123456[+0200]");
    EXPECT_EQ(pplib::String("2026-08-16T08:35:01.123456+02:00"), d1.getISO8601withUsec());

    d1.set("2026-08-16T08:35:01.123456-02:00");
    EXPECT_EQ(pplib::String("2026-08-16T08:35:01.123456-02:00"), d1.getISO8601withUsec());
    d1.set("2026-08-16T08:35:01.123456+02:00");
    EXPECT_EQ(pplib::String("2026-08-16T08:35:01.123456+02:00"), d1.getISO8601withUsec());

    d1.set("2026-08-16T08:35:01.123456-0200");
    EXPECT_EQ(pplib::String("2026-08-16T08:35:01.123456-02:00"), d1.getISO8601withUsec());
    d1.set("2026-08-16T08:35:01.123456+0200");
    EXPECT_EQ(pplib::String("2026-08-16T08:35:01.123456+02:00"), d1.getISO8601withUsec());

    d1.set("2026-08-16T08:35:01.123456-Z");
    EXPECT_EQ(pplib::String("2026-08-16T08:35:01.123456+00:00"), d1.getISO8601withUsec());
    d1.set("2026-08-16T08:35:01.123456Z");
    EXPECT_EQ(pplib::String("2026-08-16T08:35:01.123456+00:00"), d1.getISO8601withUsec());
    d1.set("2026-08-16T08:35:01.123456-UTC");
    EXPECT_EQ(pplib::String("2026-08-16T08:35:01.123456+00:00"), d1.getISO8601withUsec());

    d1.set("2026-08-16T08:35:01Z");
    EXPECT_EQ(pplib::String("2026-08-16T08:35:01.000000+00:00"), d1.getISO8601withUsec());
}

TEST_F(DateTimeTest, setWithWrongFormatThrowsException)
{
    pplib::DateTime d1;
    ASSERT_THROW({ d1.set("2026:12-16T08:35:01.012345"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set("2026:12:16T08:35:01.012345"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set("16T08.35:01.012345"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set("2a26.12.16T08:35:01.012345"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set("2A26.12.16T08:35:01.012345"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set("2!26.12.16T08:35:01.012345"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set("2026.1a.16T08:35:01.012345"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set("2026.01.aaT08:35:01.012345"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set("0000.00.00T08:35:01.012345"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set("00.00.0000T08:35:01.012345"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set("0a.01.2026T08:35:01.012345"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set("01.0a.2026T08:35:01.012345"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set("01.01.2a26T08:35:01.012345"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set("01T"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set("01 01"); }, pplib::IllegalArgumentException);
}

TEST_F(DateTimeTest, getLongInt)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18 11:50:11.159473");
        uint64_t t = d1.longInt();
        ASSERT_EQ((uint64_t)64679514611159473, t) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, setLongInt)
{
    uint64_t v = 64679514611159473;
    ASSERT_NO_THROW({
        pplib::DateTime d1;
        d1.setLongInt(v);
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11.159473"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, toStringWithoutFormat)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18 11:50:11.159473");
        ASSERT_EQ(pplib::String("2012-05-18 11:50:11.159473"), d1.toString()) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, toStringWithFormat)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18 11:50:11.159473");
        ASSERT_EQ(pplib::String("2012-05-18T11:50:11.159473"), d1.toString("%Y-%m-%dT%H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, getISO8601)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18 11:50:11.159473+02:00");
        ASSERT_EQ(pplib::String("2012-05-18T11:50:11+02:00"), d1.getISO8601()) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, getISO8601withMsec)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18 11:50:11.159473+02:00");
        ASSERT_EQ(pplib::String("2012-05-18T11:50:11.159+02:00"), d1.getISO8601withMsec()) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, getISO8601withMsecNoRoundUp)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18 11:50:11.159999+02:00");
        ASSERT_EQ(pplib::String("2012-05-18T11:50:11.159+02:00"), d1.getISO8601withMsec()) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, ReassignmentWithoutMsec)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        d1.set("2011-11-23T22:14:39");
        ASSERT_EQ(pplib::String("2011-11-23 22:14:39.000000"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, ReassignmentWithMsec)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        d1.set("2011-11-23T22:14:39.946210");
        ASSERT_EQ(pplib::String("2011-11-23 22:14:39.946210"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, clear)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        d1.clear();
        ASSERT_EQ(pplib::String("0000-00-00 00:00:00.000000"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    });
}

// Lower

TEST_F(DateTimeTest, LowerThanYear)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2011-05-18T11:50:11.159473");
        ASSERT_LT(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, LowerThanMonth)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-04-18T11:50:11.159473");
        ASSERT_LT(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, LowerThanDay)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-17T11:50:11.159473");
        ASSERT_LT(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, LowerThanHour)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T10:50:11.159473");
        ASSERT_LT(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, LowerThanMinute)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:48:11.159473");
        ASSERT_LT(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, LowerThanSecond)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:50:05.159473");
        ASSERT_LT(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, LowerThanMicroSecond)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:50:11.159470");
        ASSERT_LT(d2, d1) << "Unexpected date";
    });
}

// Lower or Equal

TEST_F(DateTimeTest, LowerThanEqualYear)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2011-05-18T11:50:11.159473");
        ASSERT_LE(d2, d1) << "Unexpected date";
        ASSERT_LE(d1, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, LowerThanEqualMonth)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-04-18T11:50:11.159473");
        ASSERT_LE(d2, d1) << "Unexpected date";
        ASSERT_LE(d1, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, LowerThanEqualDay)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-17T11:50:11.159473");
        ASSERT_LE(d2, d1) << "Unexpected date";
        ASSERT_LE(d1, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, LowerThanEqualHour)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T10:50:11.159473");
        ASSERT_LE(d2, d1) << "Unexpected date";
        ASSERT_LE(d1, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, LowerThanEqualMinute)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:48:11.159473");
        ASSERT_LE(d2, d1) << "Unexpected date";
        ASSERT_LE(d1, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, LowerThanEqualSecond)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:50:05.159473");
        ASSERT_LE(d2, d1) << "Unexpected date";
        ASSERT_LE(d1, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, LowerThanEqualMicroSecond)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:50:11.159470");
        ASSERT_LE(d2, d1) << "Unexpected date";
        ASSERT_LE(d1, d1) << "Unexpected date";
    });
}

// Equal

TEST_F(DateTimeTest, Equal)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:50:11.159473");
        ASSERT_EQ(d2, d1) << "Unexpected date";
    });
}

// NotEqual

TEST_F(DateTimeTest, NotEqualYear)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2011-05-18T11:50:11.159473");
        ASSERT_NE(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, EqualMonth)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-04-18T11:50:11.159473");
        ASSERT_NE(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, EqualDay)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-17T11:50:11.159473");
        ASSERT_NE(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, EqualHour)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T10:50:11.159473");
        ASSERT_NE(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, EqualMinute)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:48:11.159473");
        ASSERT_NE(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, EqualSecond)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:50:05.159473");
        ASSERT_NE(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, EqualMicroSecond)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:50:11.159470");
        ASSERT_NE(d2, d1) << "Unexpected date";
    });
}

// Greater

TEST_F(DateTimeTest, GreaterThanYear)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2013-05-18T11:50:11.159473");
        ASSERT_GT(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, GreaterThanMonth)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-06-18T11:50:11.159473");
        ASSERT_GT(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, GreaterThanDay)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-20T11:50:11.159473");
        ASSERT_GT(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, GreaterThanHour)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T13:50:11.159473");
        ASSERT_GT(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, GreaterThanMinute)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:52:11.159473");
        ASSERT_GT(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, GreaterThanSecond)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:50:29.159473");
        ASSERT_GT(d2, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, GreaterThanMicroSecond)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:50:11.159474");
        ASSERT_GT(d2, d1) << "Unexpected date";
    });
}

// Greater or Equal

TEST_F(DateTimeTest, GreaterThanEqualYear)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2013-05-18T11:50:11.159473");
        ASSERT_GE(d2, d1) << "Unexpected date";
        ASSERT_GE(d1, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, GreaterThanEqualMonth)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-06-18T11:50:11.159473");
        ASSERT_GE(d2, d1) << "Unexpected date";
        ASSERT_GE(d1, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, GreaterThanEqualDay)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-20T11:50:11.159473");
        ASSERT_GE(d2, d1) << "Unexpected date";
        ASSERT_GE(d1, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, GreaterThanEqualHour)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T13:50:11.159473");
        ASSERT_GE(d2, d1) << "Unexpected date";
        ASSERT_GE(d1, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, GreaterThanEqualMinute)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:52:11.159473");
        ASSERT_GE(d2, d1) << "Unexpected date";
        ASSERT_GE(d1, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, GreaterThanEqualSecond)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:50:29.159473");
        ASSERT_GE(d2, d1) << "Unexpected date";
        ASSERT_GE(d1, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, GreaterThanEqualMicroSecond)
{
    ASSERT_NO_THROW({
        pplib::DateTime d1("2012-05-18T11:50:11.159473");
        pplib::DateTime d2("2012-05-18T11:50:11.159474");
        ASSERT_GE(d2, d1) << "Unexpected date";
        ASSERT_GE(d1, d1) << "Unexpected date";
    });
}

TEST_F(DateTimeTest, week)
{
    pplib::DateTime d1;
    EXPECT_EQ(d1.set("2024-01-01 00:00:00").week(), 0) << "Unexpected week";
    EXPECT_EQ(d1.set("2024-06-05T11:50:11.159473").week(), 22) << "Unexpected week";
}

TEST_F(DateTimeTest, weekISO8601)
{
    pplib::DateTime d1;
    EXPECT_EQ(d1.set("2024-01-01 00:00:00").weekISO8601(), 1) << "Unexpected week";
    EXPECT_EQ(d1.set("2024-06-05T11:50:11.159473").weekISO8601(), 23) << "Unexpected week";
}

TEST_F(DateTimeTest, setWithDateAndTime)
{
    pplib::DateTime d1;
    EXPECT_EQ(pplib::String("2024-06-05 11:50:11.159473"), d1.set("2024-06-05", "11:50:11.159473").get("%Y-%m-%d %H:%M:%S.%u"))
        << "Unexpected date";
}

TEST_F(DateTimeTest, setDate)
{
    pplib::DateTime d1;
    EXPECT_EQ(pplib::String("2024-06-05 00:00:00.000000"), d1.setDate("2024-06-05").get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
}

TEST_F(DateTimeTest, getDate)
{
    pplib::DateTime d1;
    EXPECT_EQ(pplib::String("0000-00-00"), d1.getDate()) << "Unexpected date";

    EXPECT_EQ(pplib::String("2026-08-16"), d1.setDate("2026-08-16").getDate()) << "Unexpected date";
}

TEST_F(DateTimeTest, setTime)
{
    pplib::DateTime d1("2026-08-16");
    EXPECT_EQ(pplib::String("2026-08-16 19:24:13.123456"), d1.setTime("19:24:13.123456").get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
}

TEST_F(DateTimeTest, setByPPLTIME)
{
    pplib::DateTime d1;
    pplib::PPLTIME t1 = {0, 2024, 6, 5, 11, 50, 11};
    EXPECT_EQ(pplib::String("2024-06-05 11:50:11.000000"), d1.set(t1).get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
}

TEST_F(DateTimeTest, getPPLTIME)
{
    pplib::DateTime d1("2024-06-05 11:50:11.159473");
    pplib::PPLTIME t1 = d1.toPPLTIME();
    EXPECT_EQ(t1.year, 2024) << "Unexpected year";
    EXPECT_EQ(t1.month, 6) << "Unexpected month";
    EXPECT_EQ(t1.day, 5) << "Unexpected day";
    EXPECT_EQ(t1.hour, 11) << "Unexpected hour";
    EXPECT_EQ(t1.min, 50) << "Unexpected minute";
    EXPECT_EQ(t1.sec, 11) << "Unexpected second";
}

TEST_F(DateTimeTest, setTime_t)
{
    pplib::DateTime d1;
    uint64_t t = 1717512611; // 2024-06-04 14:50:11
    EXPECT_EQ(pplib::String("2024-06-04 14:50:11.000000"), d1.set(t).get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";

    EXPECT_EQ((uint64_t)0, d1.setTime_t((uint64_t)0).time_t()) << "Unexpected date";
}

TEST_F(DateTimeTest, setEpoch)
{
    pplib::DateTime d1;
    d1.set("2024-06-04 14:50:11.000000");
    EXPECT_EQ((uint64_t)1717512611, d1.epoch()); // 2024-06-04 14:50:11 UTC
    d1.setEpoch(1717512611);

    EXPECT_EQ(pplib::String("2024-06-04 14:50:11.000000"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";

    EXPECT_EQ((uint64_t)0, d1.setTime_t((uint64_t)0).time_t()) << "Unexpected date";
}

TEST_F(DateTimeTest, getISO8601withUsec)
{
    ASSERT_EQ(pplib::String("2024-06-05T11:50:11.159473+00:00"), pplib::DateTime("2024-06-05 11:50:11.159473").getISO8601withUsec())
        << "Unexpected date";
}

TEST_F(DateTimeTest, getRFC822Date)
{
    ASSERT_EQ(pplib::String("Wed, 05 Jun 2024 11:50:11 +0000"), pplib::DateTime("2024-06-05 11:50:11.159473").getRFC822Date())
        << "Unexpected date";
}

TEST_F(DateTimeTest, strftime)
{
    pplib::DateTime d1("2024-06-05 11:50:11.159473");
    ASSERT_EQ(pplib::String("2024-06-05 11:50:11"), d1.strftime("%Y-%m-%d %H:%M:%S")) << "Unexpected date";

    pplib::DateTime d2("2024-06-05 11:50:11.159473+02:00");
    ASSERT_EQ(pplib::String("2024-06-05 11:50:11"), d2.strftime("%Y-%m-%d %H:%M:%S")) << "Unexpected date";
}

TEST_F(DateTimeTest, epoch)
{
    pplib::DateTime d1("2024-06-05 11:50:11.159473");
    ASSERT_EQ((int32_t)0, d1.timezone().offsetSeconds()) << "Unexpected timezone offset";
    ASSERT_EQ((uint64_t)1717588211, d1.epoch()) << "Unexpected epoch";
    d1.timezone().setOffsetMinutes(120); // GMT+2

    ASSERT_EQ((int32_t)7200, d1.timezone().offsetSeconds()) << "Unexpected timezone offset";
    ASSERT_EQ((uint64_t)1717581011, d1.epoch()) << "Unexpected epoch";
}

TEST_F(DateTimeTest, notEmpty)
{
    pplib::DateTime d1("2024-06-05 11:50:11.159473");
    ASSERT_TRUE(d1.notEmpty()) << "Unexpected date";

    pplib::DateTime d2;
    ASSERT_FALSE(d2.notEmpty()) << "Unexpected date";
}

TEST_F(DateTimeTest, isEmpty)
{
    pplib::DateTime d1("2024-06-05 11:50:11.159473");
    ASSERT_FALSE(d1.isEmpty()) << "Unexpected date";

    pplib::DateTime d2;
    ASSERT_TRUE(d2.isEmpty()) << "Unexpected date";
}

TEST_F(DateTimeTest, isLeapYear)
{
    ASSERT_TRUE(pplib::DateTime("2024-06-05 11:50:11.159473").isLeapYear()) << "Unexpected date";
    ASSERT_FALSE(pplib::DateTime("2023-06-05 11:50:11.159473").isLeapYear()) << "Unexpected date";
    ASSERT_TRUE(pplib::DateTime("2000-06-05 11:50:11.159473").isLeapYear()) << "Unexpected date";
    ASSERT_FALSE(pplib::DateTime("1900-06-05 11:50:11.159473").isLeapYear()) << "Unexpected date";
}

TEST_F(DateTimeTest, currentTime)
{
    pplib::DateTime d1 = pplib::DateTime::currentTime();
    pplib::DateTime d2("2026-08-16 08:35:01.123456");
    ASSERT_TRUE(d1 >= d2) << "Unexpected date";
}

TEST_F(DateTimeTest, diffSeconds)
{
    pplib::DateTime d1("2026-08-16 08:35:01.123456");
    pplib::DateTime d2("2026-08-16 08:35:11.123456");
    ASSERT_EQ(d1.diffSeconds(d2), 10) << "Unexpected date";
}

TEST_F(DateTimeTest, compareSeconds)
{
    pplib::DateTime d1("2026-08-16 08:35:01.123456");
    pplib::DateTime d2("2026-08-16 08:35:03.123456");
    ASSERT_FALSE(d1.compareSeconds(d2, 0)) << "Unexpected date";
    ASSERT_TRUE(d1.compareSeconds(d2, 4)) << "Unexpected date";
}

TEST_F(DateTimeTest, copyAssignmentOperator)
{
    pplib::DateTime d1("2026-08-16 08:35:01.123456");
    pplib::DateTime d2;
    d2 = d1;
    ASSERT_EQ(d1, d2) << "Unexpected date";
}

TEST_F(DateTimeTest, noveAssignmentOperator)
{
    pplib::DateTime d1("2026-08-16 08:35:01.123456");
    pplib::DateTime expected("2026-08-16 08:35:01.123456");
    pplib::DateTime d2;
    d2 = std::move(d1);
    ASSERT_EQ(expected, d2) << "Unexpected date";
}

TEST_F(DateTimeTest, StringOperator)
{
    pplib::DateTime d1("2026-08-16 08:35:01.123456");
    pplib::String expected("2026-08-16 08:35:01.123456");
    pplib::String actual = d1;
    ASSERT_EQ(expected, actual) << "Unexpected date";
}

TEST_F(DateTimeTest, OperatorAssignWithString)
{
    pplib::DateTime d1;
    d1 = "2026-08-16 08:35:01.123456";
    pplib::DateTime expected("2026-08-16 08:35:01.123456");
    ASSERT_EQ(expected, d1) << "Unexpected date";
}

// Microseconds

TEST_F(DateTimeTest, toMicroseconds)
{
    pplib::DateTime d1("2026-08-16 08:35:01.123456");
    ASSERT_EQ((int64_t)1786869301123456, d1.toMicroseconds()) << "Unexpected date";
    pplib::DateTime d2("1970-01-01 00:00:00.0");
    ASSERT_EQ((int64_t)0, d2.toMicroseconds()) << "Unexpected date";
    pplib::DateTime d3("0001-01-01 00:00:00.0Z");
    ASSERT_EQ((int64_t)-62135596800000000, d3.toMicroseconds()) << "Unexpected date";
    pplib::DateTime d4("0000-01-01 00:00:00.0Z");
    ASSERT_EQ((int64_t)-62167219200000000LL, d4.toMicroseconds()) << "Unexpected date";
}

TEST_F(DateTimeTest, toMicrosecondsThrowsException)
{
    pplib::DateTime d1;
    ASSERT_THROW({ d1.toMicroseconds(); }, pplib::IllegalStateException);
}

TEST_F(DateTimeTest, setMicroseconds)
{
    pplib::DateTime d1;
    d1.setMicroseconds((int64_t)1786869301123456);
    ASSERT_EQ(pplib::String("2026-08-16 08:35:01.123456"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    pplib::DateTime d2;
    d2.setMicroseconds((int64_t)0);
    ASSERT_EQ(pplib::String("1970-01-01 00:00:00.000000"), d2.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    pplib::DateTime d3;
    d3.setMicroseconds((int64_t)-62135596800000000);
    ASSERT_EQ(pplib::String("0001-01-01 00:00:00.000000"), d3.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    pplib::DateTime d4;
    d4.setMicroseconds((int64_t)-62167219200000000LL);
    ASSERT_EQ(pplib::String("0000-01-01 00:00:00.000000"), d4.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
}

TEST_F(DateTimeTest, fromMicroseconds)
{
    pplib::DateTime d1 = pplib::DateTime::fromMicroseconds((int64_t)1786869301123456);
    ASSERT_EQ(pplib::String("2026-08-16 08:35:01.123456"), d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    pplib::DateTime d2 = pplib::DateTime::fromMicroseconds((int64_t)0);
    ASSERT_EQ(pplib::String("1970-01-01 00:00:00.000000"), d2.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    pplib::DateTime d3 = pplib::DateTime::fromMicroseconds((int64_t)-62135596800000000);
    ASSERT_EQ(pplib::String("0001-01-01 00:00:00.000000"), d3.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
    pplib::DateTime d4 = pplib::DateTime::fromMicroseconds((int64_t)-62167219200000000LL);
    ASSERT_EQ(pplib::String("0000-01-01 00:00:00.000000"), d4.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
}

} // namespace
