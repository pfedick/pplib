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

#include <pplib/types/timezone.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

#include "pplib-tests.h"

namespace
{
// The fixture for testing class Foo.
class TimeZoneTest : public ::testing::Test
{
protected:
    TimeZoneTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~TimeZoneTest()
    {
    }
};

TEST_F(TimeZoneTest, ConstructorSimple)
{
    ASSERT_NO_THROW({
        pplib::TimeZone tz1;
        ASSERT_EQ((int16_t)0, tz1.offsetMinutes()) << "TimeZone has unexpected offset";
        ASSERT_EQ(pplib::String("UTC"), tz1.name()) << "TimeZone has unexpected name";
    });
}

TEST_F(TimeZoneTest, fromStringUTC)
{
    ASSERT_EQ((int16_t)0, pplib::TimeZone::fromString("").offsetMinutes());
    ASSERT_EQ((int16_t)0, pplib::TimeZone::fromString("Z").offsetMinutes());
    ASSERT_EQ((int16_t)0, pplib::TimeZone::fromString("UTC").offsetMinutes());
}

TEST_F(TimeZoneTest, fromStringWithDoppelColumn)
{
    ASSERT_EQ((int16_t)120, pplib::TimeZone::fromString("+2:0").offsetMinutes());
    ASSERT_EQ((int16_t)-120, pplib::TimeZone::fromString("-2:0").offsetMinutes());
    ASSERT_EQ((int16_t)130, pplib::TimeZone::fromString("+02:10").offsetMinutes());
    ASSERT_EQ((int16_t)-130, pplib::TimeZone::fromString("-02:10").offsetMinutes());
}

TEST_F(TimeZoneTest, fromStringWithoutDoppelColumn)
{
    ASSERT_EQ((int16_t)130, pplib::TimeZone::fromString("+0210").offsetMinutes());
    ASSERT_EQ((int16_t)-130, pplib::TimeZone::fromString("-0210").offsetMinutes());
}

TEST_F(TimeZoneTest, fromStringOnlyHours)
{
    ASSERT_EQ((int16_t)120, pplib::TimeZone::fromString("+02").offsetMinutes());
    ASSERT_EQ((int16_t)-120, pplib::TimeZone::fromString("-02").offsetMinutes());
}

TEST_F(TimeZoneTest, fromStringWithBrackets)
{
    ASSERT_EQ((int16_t)130, pplib::TimeZone::fromString("[+0210]").offsetMinutes());
    ASSERT_EQ((int16_t)-130, pplib::TimeZone::fromString("[-0210]").offsetMinutes());
    ASSERT_EQ((int16_t)130, pplib::TimeZone::fromString("[+02:10]").offsetMinutes());
    ASSERT_EQ((int16_t)-130, pplib::TimeZone::fromString("[-02:10]").offsetMinutes());
}

TEST_F(TimeZoneTest, setOffsetMinutes)
{
    ASSERT_EQ((int16_t)120, pplib::TimeZone().setOffsetMinutes(120).offsetMinutes());
    ASSERT_EQ((int16_t)-120, pplib::TimeZone().setOffsetMinutes(-120).offsetMinutes());
    ASSERT_EQ((int16_t)840, pplib::TimeZone().setOffsetMinutes(840).offsetMinutes());
    ASSERT_EQ((int16_t)-720, pplib::TimeZone().setOffsetMinutes(-720).offsetMinutes());

    ASSERT_THROW({ pplib::TimeZone().setOffsetMinutes(-721); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ pplib::TimeZone().setOffsetMinutes(841); }, pplib::IllegalArgumentException);
}

TEST_F(TimeZoneTest, setOffsetSeconds)
{
    ASSERT_EQ((int16_t)120, pplib::TimeZone().setOffsetSeconds(120 * 60).offsetMinutes());
    ASSERT_EQ((int16_t)-120, pplib::TimeZone().setOffsetSeconds(-120 * 60).offsetMinutes());
}

TEST_F(TimeZoneTest, setOffset)
{
    ASSERT_EQ((int16_t)130, pplib::TimeZone().setOffset(2, 10).offsetMinutes());
    ASSERT_EQ((int16_t)-130, pplib::TimeZone().setOffset(-2, 10).offsetMinutes());
}

TEST_F(TimeZoneTest, setName)
{
    pplib::TimeZone tz1;
    tz1.setName("CET");
    ASSERT_EQ(pplib::String("CET"), tz1.name()) << "TimeZone has unexpected name";
}

TEST_F(TimeZoneTest, fromLocalTime)
{
    pplib::TimeZone tz = pplib::TimeZone::fromLocalTime();

    ASSERT_EQ(120, tz.offsetMinutes());
    ASSERT_GE(tz.offsetMinutes(), 60);
    ASSERT_LE(tz.offsetMinutes(), 120);
}

TEST_F(TimeZoneTest, fromEpoch)
{
    pplib::TimeZone tz = pplib::TimeZone::fromEpoch(1717588211);

    ASSERT_EQ(120, tz.offsetMinutes());
    ASSERT_GE(tz.offsetMinutes(), 60);
    ASSERT_LE(tz.offsetMinutes(), 120);
}

TEST_F(TimeZoneTest, toString)
{
    pplib::TimeZone tz1(120, "CET");
    ASSERT_EQ(pplib::String("+02:00"), tz1.toString(true));
    ASSERT_EQ(pplib::String("+0200"), tz1.toString(false));

    pplib::TimeZone tz2(-90, "CST");
    ASSERT_EQ(pplib::String("-01:30"), tz2.toString(true));
    ASSERT_EQ(pplib::String("-0130"), tz2.toString(false));

    pplib::TimeZone tz3(0, "UTC");
    ASSERT_EQ(pplib::String("Z"), tz3.toString(true));
}

TEST_F(TimeZoneTest, offsetSeconds)
{
    pplib::TimeZone tz1(120, "CET");
    ASSERT_EQ((int32_t)7200, tz1.offsetSeconds());

    pplib::TimeZone tz2(-90, "CST");
    ASSERT_EQ((int32_t)-5400, tz2.offsetSeconds());

    pplib::TimeZone tz3(0, "UTC");
    ASSERT_EQ((int32_t)0, tz3.offsetSeconds());
}

TEST_F(TimeZoneTest, fromHours)
{
    pplib::TimeZone tz1 = pplib::TimeZone::fromHours(2);
    ASSERT_EQ((int16_t)120, tz1.offsetMinutes());

    pplib::TimeZone tz2 = pplib::TimeZone::fromHours(-3);
    ASSERT_EQ((int16_t)-180, tz2.offsetMinutes());
}

TEST_F(TimeZoneTest, fromHoursAndMinutes)
{
    pplib::TimeZone tz1 = pplib::TimeZone::fromHoursAndMinutes(2, 30);
    ASSERT_EQ((int16_t)150, tz1.offsetMinutes());

    pplib::TimeZone tz2 = pplib::TimeZone::fromHoursAndMinutes(-3, -15);
    ASSERT_EQ((int16_t)-195, tz2.offsetMinutes());

    pplib::TimeZone tz3 = pplib::TimeZone::fromHoursAndMinutes(3, -15);
    ASSERT_EQ((int16_t)-195, tz3.offsetMinutes());
}

TEST_F(TimeZoneTest, isUTC)
{
    pplib::TimeZone tz1(0, "UTC");
    ASSERT_TRUE(tz1.isUTC());

    pplib::TimeZone tz2(120, "CET");
    ASSERT_FALSE(tz2.isUTC());
}

} // namespace
