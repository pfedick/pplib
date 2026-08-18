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

#include <pplib/types/date.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

#include "pplib-tests.h"

namespace
{
// The fixture for testing class Foo.
class DateTest : public ::testing::Test
{
protected:
    DateTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~DateTest()
    {
    }
};

TEST_F(DateTest, ConstructorSimple)
{
    ASSERT_NO_THROW({
        pplib::Date d1;
        ASSERT_FALSE(d1.isValid()) << "Class has unexpected value";
    });
}

TEST_F(DateTest, ConstructorWithValues)
{
    ASSERT_NO_THROW({
        pplib::Date d1(2020, 2, 1);
        ASSERT_TRUE(d1.isValid()) << "Class has unexpected value";
        ASSERT_EQ((uint16_t)2020, d1.year()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)2, d1.month()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)1, d1.day()) << "Class has unexpected value";
    });
}

TEST_F(DateTest, ConstructorWithDateString)
{
    ASSERT_NO_THROW({
        pplib::Date d1("2020-02-01");
        ASSERT_TRUE(d1.isValid()) << "Class has unexpected value";
        ASSERT_EQ((uint16_t)2020, d1.year()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)2, d1.month()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)1, d1.day()) << "Class has unexpected value";
    });

    ASSERT_NO_THROW({
        pplib::Date d2("01.02.2020");
        ASSERT_TRUE(d2.isValid()) << "Class has unexpected value";
        ASSERT_EQ((uint16_t)2020, d2.year()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)2, d2.month()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)1, d2.day()) << "Class has unexpected value";
    });

    ASSERT_THROW({ pplib::Date d3("2020-01"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ pplib::Date d4("2020-01-01-01"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ pplib::Date d4("01-2020-01"); }, pplib::IllegalArgumentException);
}

TEST_F(DateTest, setWithValues)
{
    ASSERT_NO_THROW({
        pplib::Date d1;
        d1.set(2020, 2, 1);
        ASSERT_TRUE(d1.isValid()) << "Class has unexpected value";
        ASSERT_EQ((uint16_t)2020, d1.year()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)2, d1.month()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)1, d1.day()) << "Class has unexpected value";
    });
}

TEST_F(DateTest, setWithWrongValues)
{
    pplib::Date d1;
    ASSERT_THROW({ d1.set(2020, 1, 0); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set(2020, 0, 1); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set(2020, 13, 1); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set(10000, 1, 1); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set(2020, 2, 31); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ d1.set(2020, 3, 32); }, pplib::IllegalArgumentException);
}

TEST_F(DateTest, setWithDateString)
{
    ASSERT_NO_THROW({
        pplib::Date d1;
        d1.set("2020-02-01");
        ASSERT_TRUE(d1.isValid()) << "Class has unexpected value";
        ASSERT_EQ((uint16_t)2020, d1.year()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)2, d1.month()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)1, d1.day()) << "Class has unexpected value";
    });

    ASSERT_NO_THROW({
        pplib::Date d2;
        d2.set("01.02.2020");
        ASSERT_TRUE(d2.isValid()) << "Class has unexpected value";
        ASSERT_EQ((uint16_t)2020, d2.year()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)2, d2.month()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)1, d2.day()) << "Class has unexpected value";
    });

    ASSERT_THROW(
        {
            pplib::Date d3;
            d3.set("2020-01");
        },
        pplib::IllegalArgumentException);
    ASSERT_THROW(
        {
            pplib::Date d4;
            d4.set("2020-01-01-01");
        },
        pplib::IllegalArgumentException);
    ASSERT_THROW(
        {
            pplib::Date d5;
            d5.set("01-2020-01");
        },
        pplib::IllegalArgumentException);
}

TEST_F(DateTest, toString)
{
    ASSERT_EQ(pplib::String("2020-02-01"), pplib::Date(2020, 2, 1).toString()) << "Class has unexpected value";
    ASSERT_EQ(pplib::String("0000-00-00"), pplib::Date().toString()) << "Class has unexpected value";
}

TEST_F(DateTest, format)
{
    ASSERT_EQ(pplib::String("2020-02-01"), pplib::Date(2020, 2, 1).format("%Y-%m-%d")) << "Class has unexpected value";
    ASSERT_EQ(pplib::String("20-02-01"), pplib::Date(2020, 2, 1).format("%y-%m-%d")) << "Class has unexpected value";
    ASSERT_EQ(pplib::String("01.02.2020"), pplib::Date(2020, 2, 1).format("%d.%m.%Y")) << "Class has unexpected value";
    ASSERT_EQ(pplib::String("0000-00-00"), pplib::Date().format("%Y-%m-%d")) << "Class has unexpected value";
}

TEST_F(DateTest, toInt)
{
    ASSERT_EQ((uint32_t)20200201, pplib::Date(2020, 2, 1).toInt()) << "Class has unexpected value";
    ASSERT_EQ((uint32_t)0, pplib::Date().toInt()) << "Class has unexpected value";
}

TEST_F(DateTest, ComparisonOperators)
{
    ASSERT_TRUE(pplib::Date(2020, 1, 1) == pplib::Date(2020, 1, 1)) << "Class has unexpected value";
    ASSERT_FALSE(pplib::Date(2020, 1, 1) != pplib::Date(2020, 1, 1)) << "Class has unexpected value";
    ASSERT_TRUE(pplib::Date(2020, 1, 1) < pplib::Date(2020, 1, 2)) << "Class has unexpected value";
    ASSERT_TRUE(pplib::Date(2020, 1, 2) > pplib::Date(2020, 1, 1)) << "Class has unexpected value";
    ASSERT_TRUE(pplib::Date(2020, 1, 1) <= pplib::Date(2020, 1, 1)) << "Class has unexpected value";
    ASSERT_TRUE(pplib::Date(2020, 1, 2) >= pplib::Date(2020, 1, 2)) << "Class has unexpected value";
}

TEST_F(DateTest, CopyAndMoveAssignment)
{
    ASSERT_NO_THROW({
        pplib::Date d1(2020, 1, 1);
        pplib::Date d2;
        d2 = d1;
        ASSERT_TRUE(d2.isValid()) << "Class has unexpected value";
        ASSERT_EQ((uint16_t)2020, d2.year()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)1, d2.month()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)1, d2.day()) << "Class has unexpected value";

        pplib::Date d3;
        d3 = std::move(d1);
        ASSERT_TRUE(d3.isValid()) << "Class has unexpected value";
        ASSERT_EQ((uint16_t)2020, d3.year()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)1, d3.month()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)1, d3.day()) << "Class has unexpected value";
    });
}

TEST_F(DateTest, Clear)
{
    ASSERT_NO_THROW({
        pplib::Date d1(2020, 1, 1);
        d1.clear();
        ASSERT_FALSE(d1.isValid()) << "Class has unexpected value";
        ASSERT_EQ((uint16_t)0, d1.year()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)0, d1.month()) << "Class has unexpected value";
        ASSERT_EQ((uint8_t)0, d1.day()) << "Class has unexpected value";
    });
}

TEST_F(DateTest, Today)
{
    ASSERT_NO_THROW({
        pplib::Date d1 = pplib::Date::today();
        ASSERT_TRUE(d1.isValid()) << "Class has unexpected value";
    });
}

TEST_F(DateTest, isLeapYear)
{
    ASSERT_TRUE(pplib::Date(2020, 1, 1).isLeapYear()) << "Class has unexpected value";
    ASSERT_FALSE(pplib::Date(2021, 1, 1).isLeapYear()) << "Class has unexpected value";
    ASSERT_TRUE(pplib::Date(2000, 1, 1).isLeapYear()) << "Class has unexpected value";
    ASSERT_FALSE(pplib::Date(1900, 1, 1).isLeapYear()) << "Class has unexpected value";
    ASSERT_THROW({ pplib::Date().isLeapYear(); }, pplib::IllegalStateException);
}

TEST_F(DateTest, daysInMonth)
{
    ASSERT_EQ(31, pplib::Date::daysInMonth(1, 2020)) << "Class has unexpected value";
    ASSERT_EQ(29, pplib::Date::daysInMonth(2, 2020)) << "Class has unexpected value";
    ASSERT_EQ(28, pplib::Date::daysInMonth(2, 2021)) << "Class has unexpected value";
    ASSERT_EQ(31, pplib::Date::daysInMonth(3, 2020)) << "Class has unexpected value";
    ASSERT_EQ(30, pplib::Date::daysInMonth(4, 2020)) << "Class has unexpected value";
    ASSERT_EQ(31, pplib::Date::daysInMonth(5, 2020)) << "Class has unexpected value";
    ASSERT_EQ(30, pplib::Date::daysInMonth(6, 2020)) << "Class has unexpected value";
    ASSERT_EQ(31, pplib::Date::daysInMonth(7, 2020)) << "Class has unexpected value";
    ASSERT_EQ(31, pplib::Date::daysInMonth(8, 2020)) << "Class has unexpected value";
    ASSERT_EQ(30, pplib::Date::daysInMonth(9, 2020)) << "Class has unexpected value";
    ASSERT_EQ(31, pplib::Date::daysInMonth(10, 2020)) << "Class has unexpected value";
    ASSERT_EQ(30, pplib::Date::daysInMonth(11, 2020)) << "Class has unexpected value";
    ASSERT_EQ(31, pplib::Date::daysInMonth(12, 2020)) << "Class has unexpected value";
    ASSERT_THROW({ pplib::Date::daysInMonth(0, 2020); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ pplib::Date::daysInMonth(13, 2020); }, pplib::IllegalArgumentException);
}

TEST_F(DateTest, dayOfYear)
{
    ASSERT_EQ(1, pplib::Date(2020, 1, 1).dayOfYear()) << "Class has unexpected value";
    ASSERT_EQ(32, pplib::Date(2020, 2, 1).dayOfYear()) << "Class has unexpected value";
    ASSERT_EQ(60, pplib::Date(2020, 2, 29).dayOfYear()) << "Class has unexpected value";
    ASSERT_EQ(366, pplib::Date(2020, 12, 31).dayOfYear()) << "Class has unexpected value";
    ASSERT_EQ(365, pplib::Date(2021, 12, 31).dayOfYear()) << "Class has unexpected value";
    ASSERT_THROW({ pplib::Date().dayOfYear(); }, pplib::IllegalStateException);
}

TEST_F(DateTest, dayOfWeek)
{
    ASSERT_EQ(3, pplib::Date(2020, 1, 1).dayOfWeek()) << "Class has unexpected value"; // Wednesday
    ASSERT_EQ(6, pplib::Date(2020, 1, 4).dayOfWeek()) << "Class has unexpected value"; // Saturday
    ASSERT_EQ(0, pplib::Date(2020, 1, 5).dayOfWeek()) << "Class has unexpected value"; // Sunday
    ASSERT_EQ(1, pplib::Date(2020, 1, 6).dayOfWeek()) << "Class has unexpected value"; // Monday
    ASSERT_THROW({ pplib::Date().dayOfWeek(); }, pplib::IllegalStateException);
}

TEST_F(DateTest, dayOfWeekISO8601)
{
    ASSERT_EQ(3, pplib::Date(2020, 1, 1).dayOfWeekISO8601()) << "Class has unexpected value"; // Wednesday
    ASSERT_EQ(6, pplib::Date(2020, 1, 4).dayOfWeekISO8601()) << "Class has unexpected value"; // Saturday
    ASSERT_EQ(7, pplib::Date(2020, 1, 5).dayOfWeekISO8601()) << "Class has unexpected value"; // Sunday
    ASSERT_EQ(1, pplib::Date(2020, 1, 6).dayOfWeekISO8601()) << "Class has unexpected value"; // Monday
    ASSERT_THROW({ pplib::Date().dayOfWeekISO8601(); }, pplib::IllegalStateException);
}

TEST_F(DateTest, week)
{
    ASSERT_EQ(0, pplib::Date(2020, 1, 1).week()) << "Class has unexpected value";
    ASSERT_EQ(1, pplib::Date(2020, 1, 6).week()) << "Class has unexpected value";
    ASSERT_EQ(52, pplib::Date(2020, 12, 31).week()) << "Class has unexpected value";
    ASSERT_THROW({ pplib::Date().week(); }, pplib::IllegalStateException);

    ASSERT_EQ(0, pplib::Date(1892, 1, 2).week()) << "Class has unexpected value";
    ASSERT_EQ(1, pplib::Date(1892, 1, 3).week()) << "Class has unexpected value";
}

TEST_F(DateTest, weekISO8601)
{
    ASSERT_EQ(1, pplib::Date(2020, 1, 1).weekISO8601()) << "Class has unexpected value";
    ASSERT_EQ(53, pplib::Date(2021, 1, 1).weekISO8601()) << "Class has unexpected value";
    ASSERT_EQ(52, pplib::Date(2022, 1, 1).weekISO8601()) << "Class has unexpected value";

    ASSERT_EQ(2, pplib::Date(2020, 1, 6).weekISO8601()) << "Class has unexpected value";
    ASSERT_EQ(53, pplib::Date(2020, 12, 31).weekISO8601()) << "Class has unexpected value";
    ASSERT_EQ(52, pplib::Date(2021, 12, 31).weekISO8601()) << "Class has unexpected value";
    ASSERT_EQ(52, pplib::Date(2022, 12, 31).weekISO8601()) << "Class has unexpected value";
    ASSERT_EQ(1, pplib::Date(2024, 12, 31).weekISO8601()) << "Class has unexpected value";

    ASSERT_THROW({ pplib::Date().weekISO8601(); }, pplib::IllegalStateException);
}

} // namespace
