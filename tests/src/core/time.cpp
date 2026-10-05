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
#include "pplib-tests.h"

#include <time.h>

#include <pplib/types/string.h>
// #include <pplib/types/bytearray.h>

#include <pplib/core/time.h>
#include <pplib/core/timer.h>
#include <pplib/exceptions.h>

namespace
{

// The fixture for testing class Foo.
class TimeFunctionTest : public ::testing::Test
{
protected:
    TimeFunctionTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen: LC_CTYPE\n");
            throw std::exception();
        }
        if (setlocale(LC_TIME, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen: LC_TIME\n");
            throw std::exception();
        }
    }
    virtual ~TimeFunctionTest()
    {
    }
};

TEST_F(TimeFunctionTest, GetTime)
{
    time_t before;
    time(&before);

    pplib::ppl_time_t t = pplib::GetTime();
    EXPECT_GT(t, 0) << "GetTime should return a positive timestamp";

    time_t after;
    time(&after);

    EXPECT_GE(t, before) << "GetTime should return a timestamp not earlier than before";
    EXPECT_LE(t, after) << "GetTime should return a timestamp not later than after";
}

TEST_F(TimeFunctionTest, LocalTime)
{
    pplib::PPLTIME ppltime = pplib::LocalTime(1791017663); // = Sat Oct  3 10:54:23 CEST 2026
    EXPECT_EQ(2026, ppltime.year) << "LocalTime should return the correct year";
    EXPECT_EQ(10, ppltime.hour) << "LocalTime should return the correct hour";
    EXPECT_EQ(54, ppltime.min) << "LocalTime should return the correct minute";
    EXPECT_EQ(23, ppltime.sec) << "LocalTime should return the correct second";
    EXPECT_EQ(3, ppltime.day) << "LocalTime should return the correct day";
    EXPECT_EQ(10, ppltime.month) << "LocalTime should return the correct month";
    EXPECT_EQ(1791017663, ppltime.epoch) << "LocalTime should return the correct Epoch";
    EXPECT_EQ(6, ppltime.day_of_week) << "LocalTime should return the correct day of the week";
    EXPECT_EQ(275, ppltime.day_of_year) << "LocalTime should return the correct day of the year";
#if defined(STRUCT_TM_HAS_GMTOFF) || defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__)
    EXPECT_EQ(7200, ppltime.gmt_offset) << "LocalTime should return the correct GMT offset";
    EXPECT_TRUE(ppltime.have_gmt_offset) << "LocalTime should indicate no GMT offset";
#else
    EXPECT_EQ(0, ppltime.gmt_offset) << "LocalTime should return the correct GMT offset";
    EXPECT_FALSE(ppltime.have_gmt_offset) << "LocalTime should indicate no GMT offset";
#endif
}

TEST_F(TimeFunctionTest, GmTime)
{
    pplib::PPLTIME ppltime = pplib::GMTime(1791017663); // = Sat Oct  3 08:54:23 UTC 2026
    EXPECT_EQ(2026, ppltime.year) << "LocalTime should return the correct year";
    EXPECT_EQ(8, ppltime.hour) << "LocalTime should return the correct hour";
    EXPECT_EQ(54, ppltime.min) << "LocalTime should return the correct minute";
    EXPECT_EQ(23, ppltime.sec) << "LocalTime should return the correct second";
    EXPECT_EQ(3, ppltime.day) << "LocalTime should return the correct day";
    EXPECT_EQ(10, ppltime.month) << "LocalTime should return the correct month";
    EXPECT_EQ(1791017663, ppltime.epoch) << "LocalTime should return the correct Epoch";
    EXPECT_EQ(6, ppltime.day_of_week) << "LocalTime should return the correct day of the week";
    EXPECT_EQ(275, ppltime.day_of_year) << "LocalTime should return the correct day of the year";
#if defined(STRUCT_TM_HAS_GMTOFF) || defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__)
    EXPECT_EQ(0, ppltime.gmt_offset) << "LocalTime should return the correct GMT offset";
    EXPECT_TRUE(ppltime.have_gmt_offset) << "LocalTime should indicate no GMT offset";
#else
    EXPECT_EQ(0, ppltime.gmt_offset) << "LocalTime should return the correct GMT offset";
    EXPECT_FALSE(ppltime.have_gmt_offset) << "LocalTime should indicate no GMT offset";
#endif
}

TEST_F(TimeFunctionTest, USleep)
{
    pplib::Timer timer;
    timer.start();
    pplib::USleep(10000);
    timer.stop();
    EXPECT_GE(timer.duration(), 0.009) << "USleep should sleep for at least the specified duration";

    EXPECT_LE(timer.duration(), 0.011) << "USleep should not sleep significantly longer than the specified duration";
}

TEST_F(TimeFunctionTest, MSleep)
{
    pplib::Timer timer;
    timer.start();
    pplib::MSleep(10);
    timer.stop();
    EXPECT_GE(timer.duration(), 0.009) << "MSleep should sleep for at least the specified duration";
    EXPECT_LE(timer.duration(), 0.011) << "MSleep should not sleep significantly longer than the specified duration";
}

TEST_F(TimeFunctionTest, SSleep)
{
    pplib::Timer timer;
    timer.start();
    pplib::SSleep(1);
    timer.stop();
    EXPECT_GE(timer.duration(), 0.9) << "MSleep should sleep for at least the specified duration";
    EXPECT_LE(timer.duration(), 1.1) << "MSleep should not sleep significantly longer than the specified duration";
}

TEST_F(TimeFunctionTest, Sleep)
{
    pplib::Timer timer;
    timer.start();
    pplib::Sleep(0.01);
    timer.stop();
    EXPECT_GE(timer.duration(), 0.009) << "MSleep should sleep for at least the specified duration";
    EXPECT_LE(timer.duration(), 0.011) << "MSleep should not sleep significantly longer than the specified duration";
}

TEST_F(TimeFunctionTest, GetMicrotime)
{
    double t1 = pplib::GetMicrotime();
    pplib::USleep(1000);
    double t2 = pplib::GetMicrotime();
    ASSERT_GE(t2, t1 + 0.001) << "GetMicrotime should return increasing values";
}

TEST_F(TimeFunctionTest, GetMilliseconds)
{
    uint64_t t1 = pplib::GetMilliSeconds();
    pplib::MSleep(1);
    uint64_t t2 = pplib::GetMilliSeconds();
    ASSERT_GE(t2, t1 + 1) << "GetMilliseconds should return increasing values";
}
TEST_F(TimeFunctionTest, GetMicroSeconds)
{
    uint64_t t1 = pplib::GetMicroSeconds();
    pplib::USleep(1000);
    uint64_t t2 = pplib::GetMicroSeconds();
    ASSERT_GE(t2, t1 + 1000) << "GetMicroSeconds should return increasing values";
}

TEST_F(TimeFunctionTest, MkTime)
{
    EXPECT_EQ(1791024720, pplib::MkTime(2026, 10, 3, 11, 52, 0)) << "MkTime returns unexpected value";
    EXPECT_EQ(0, pplib::MkTime(1899, 10, 3, 11, 52, 0)) << "MkTime returns unexpected value";
    EXPECT_EQ(0, pplib::MkTime(2026, 0, 3, 11, 52, 0)) << "MkTime returns unexpected value";
}

TEST_F(TimeFunctionTest, MkTimeFromPPLTIME)
{
    pplib::PPLTIME t;
    t.year = 2026;
    t.month = 10;
    t.day = 3;
    t.hour = 11;
    t.min = 52;
    t.sec = 0;
    EXPECT_EQ(pplib::MkTime(t), 1791024720) << "MkTimeFromPPLTIME returns unexpected value";

    t.year = 1899;
    EXPECT_EQ(pplib::MkTime(t), 0) << "MkTimeFromPPLTIME returns unexpected value";
    t.year = 2026; // Reset year to a valid value for further tests if needed
    t.month = 0;
    EXPECT_EQ(pplib::MkTime(t), 0) << "MkTimeFromPPLTIME returns unexpected value";
}

TEST_F(TimeFunctionTest, MkRFC822Date)
{
    pplib::PPLTIME t;
    t.year = 2026;
    t.month = 10;
    t.day = 3;
    t.hour = 11;
    t.min = 52;
    t.sec = 0;
    t.day_of_week = 6; // Saturday
    t.have_gmt_offset = true;
    t.gmt_offset = 3600; // +01:00
    EXPECT_EQ(pplib::MkRFC822Date(t), "Sat, 3 Oct 2026 11:52:00 +0100") << "MkRFC822Date returns unexpected value";
}

TEST_F(TimeFunctionTest, MkRFC822Date_withoutGmtOffset)
{
    pplib::PPLTIME t;
    t.year = 2026;
    t.month = 10;
    t.day = 3;
    t.hour = 11;
    t.min = 52;
    t.sec = 0;
    t.day_of_week = 6; // Saturday
    t.have_gmt_offset = false;
    EXPECT_EQ(pplib::MkRFC822Date(t), "Sat, 3 Oct 2026 11:52:00") << "MkRFC822Date without GMT offset returns unexpected value";
}

TEST_F(TimeFunctionTest, MkRFC822Date_withNegativeGmtOffset)
{
    pplib::PPLTIME t;
    t.year = 2026;
    t.month = 10;
    t.day = 3;
    t.hour = 11;
    t.min = 52;
    t.sec = 0;
    t.day_of_week = 6; // Saturday
    t.have_gmt_offset = true;
    t.gmt_offset = -3600; // -01:00
    EXPECT_EQ(pplib::MkRFC822Date(t), "Sat, 3 Oct 2026 11:52:00 -0100") << "MkRFC822Date with negative GMT offset returns unexpected value";
}
TEST_F(TimeFunctionTest, MkRFC822Date_Throws)
{
    pplib::PPLTIME t;
    t.year = 2026;
    t.month = 10;
    t.day = 3;
    t.hour = 11;
    t.min = 52;
    t.sec = 0;
    t.day_of_week = -1; // Invalid day of week
    t.have_gmt_offset = true;
    t.gmt_offset = 3600; // +01:00
    EXPECT_THROW(pplib::MkRFC822Date(t), pplib::IllegalArgumentException) << "MkRFC822Date did not throw for invalid day_of_week";

    t.day_of_week = 7; // Valid day of week
    EXPECT_THROW(pplib::MkRFC822Date(t), pplib::IllegalArgumentException) << "MkRFC822Date did not throw for invalid day_of_week";
    t.day_of_week = 1; // Valid day of week
    t.month = 0;       // Invalid month
    EXPECT_THROW(pplib::MkRFC822Date(t), pplib::IllegalArgumentException) << "MkRFC822Date did not throw for invalid month";
    t.month = 13; // Invalid month
    EXPECT_THROW(pplib::MkRFC822Date(t), pplib::IllegalArgumentException) << "MkRFC822Date did not throw for invalid month";
}

TEST_F(TimeFunctionTest, MkRFC822DateWithUnixTime)
{
    pplib::ppl_time_t unix_time = 1791024720; // Corresponds to Sat, 3 Oct 2026 11:52:00 GMT
#if defined(STRUCT_TM_HAS_GMTOFF) || defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__)
    EXPECT_EQ(pplib::MkRFC822Date(unix_time), "Sat, 3 Oct 2026 12:52:00 +0200") << "MkRFC822Date with Unix time returns unexpected value";
#else
    EXPECT_EQ(pplib::MkRFC822Date(unix_time), "Sat, 3 Oct 2026 12:52:00") << "MkRFC822Date with Unix time returns unexpected value";
#endif
}

TEST_F(TimeFunctionTest, MkISO8601Date)
{
    pplib::PPLTIME t;
    t.year = 2026;
    t.month = 10;
    t.day = 3;
    t.hour = 11;
    t.min = 52;
    t.sec = 0;
    t.day_of_week = 6; // Saturday
    t.have_gmt_offset = true;
    t.gmt_offset = 3600; // +01:00
    EXPECT_EQ(pplib::MkISO8601Date(t), "2026-10-03T11:52:00+01:00") << "MkISO8601Date returns unexpected value";
}
TEST_F(TimeFunctionTest, MkISO8601DateWithoutTimezone)
{
    pplib::PPLTIME t;
    t.year = 2026;
    t.month = 10;
    t.day = 3;
    t.hour = 11;
    t.min = 52;
    t.sec = 0;
    t.day_of_week = 6; // Saturday
    t.have_gmt_offset = false;
    t.summertime = false;
    EXPECT_EQ(pplib::MkISO8601Date(t), "2026-10-03T11:52:00") << "MkISO8601Date without timezone returns unexpected value";
}

TEST_F(TimeFunctionTest, MkISO8601DateWithNegativeTimezone)
{
    pplib::PPLTIME t;
    t.year = 2026;
    t.month = 10;
    t.day = 3;
    t.hour = 11;
    t.min = 52;
    t.sec = 0;
    t.day_of_week = 6; // Saturday
    t.have_gmt_offset = true;
    t.gmt_offset = -3600; // -01:00
    EXPECT_EQ(pplib::MkISO8601Date(t), "2026-10-03T11:52:00-01:00") << "MkISO8601Date with negative timezone returns unexpected value";
}

TEST_F(TimeFunctionTest, MkISO8601DateThrows)
{
    pplib::PPLTIME t;
    t.year = 2026;
    t.month = 10;
    t.day = 3;
    t.hour = 11;
    t.min = 52;
    t.sec = 0;
    t.day_of_week = 6; // Saturday
    t.have_gmt_offset = true;
    t.gmt_offset = 3600; // +01:00
    t.month = 13;        // Invalid month
    EXPECT_THROW(pplib::MkISO8601Date(t), pplib::IllegalArgumentException) << "MkISO8601Date did not throw for invalid month";
    t.month = 0; // Another invalid month to trigger the exception again
    EXPECT_THROW(pplib::MkISO8601Date(t), pplib::IllegalArgumentException) << "MkISO8601Date did not throw for invalid month";
}

TEST_F(TimeFunctionTest, MkISO8601DateWithUnixTime)
{
    pplib::ppl_time_t unix_time = 1791024720; // Corresponds to 2026-10-03T11:52:00+01:00
#if defined(STRUCT_TM_HAS_GMTOFF) || defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__)
    EXPECT_EQ(pplib::MkISO8601Date(unix_time), "2026-10-03T12:52:00+02:00") << "MkISO8601Date with Unix time returns unexpected value";
#else
    EXPECT_EQ(pplib::MkISO8601Date(unix_time), "2026-10-03T12:52:00") << "MkISO8601Date with Unix time returns unexpected value";
#endif
}

TEST_F(TimeFunctionTest, MkTime_withIso8601String_withoutTimezone)
{
    EXPECT_EQ((pplib::ppl_time_t)1378388624, pplib::MkTime(pplib::String("2013-09-05T14:43:44"))) << "MkTime returns unexpected value";
}

TEST_F(TimeFunctionTest, MkTime_withIso8601String_withZeroTimeoffset)
{
    EXPECT_EQ((pplib::ppl_time_t)1378388624, pplib::MkTime(pplib::String("2013-09-05T14:43:44+00:00")))
        << "MkTime returns unexpected value";
    EXPECT_EQ((pplib::ppl_time_t)1378388624, pplib::MkTime(pplib::String("2013-09-05T14:43:44-00:00")))
        << "MkTime returns unexpected value";
}

TEST_F(TimeFunctionTest, MkTime_withIso8601String_withPositiveTimeoffset)
{
    EXPECT_EQ((pplib::ppl_time_t)1378388624, pplib::MkTime(pplib::String("2013-09-05T16:43:44+02:00")))
        << "MkTime returns unexpected value";
    EXPECT_EQ((pplib::ppl_time_t)1378388624, pplib::MkTime(pplib::String("2013-09-05T12:43:44-02:00")))
        << "MkTime returns unexpected value";
}

TEST_F(TimeFunctionTest, MkTime_withIso8601String_withNegativeTimeoffset)
{
    EXPECT_EQ((pplib::ppl_time_t)1378388624, pplib::MkTime(pplib::String("2013-09-05T12:43:44-02:00")))
        << "MkTime returns unexpected value";
}

TEST_F(TimeFunctionTest, MkDateWithUnixTime)
{
    pplib::ppl_time_t unix_time = 1791024720; // Corresponds to 2026-10-03T11:52:00+01:00
    EXPECT_EQ(pplib::MkDate("%Y-%m-%dT%H:%M:%S", unix_time), "2026-10-03T12:52:00") << "MkDate with Unix time returns unexpected value";
}

TEST_F(TimeFunctionTest, MkDateWithPPLTIME)
{
    pplib::PPLTIME t;
    t.year = 2026;
    t.month = 10;
    t.day = 3;
    t.hour = 11;
    t.min = 52;
    t.sec = 0;
    t.day_of_week = 6; // Saturday
    t.have_gmt_offset = true;
    t.gmt_offset = 3600; // +01:00
    EXPECT_EQ(pplib::MkDate("%Y-%m-%dT%H:%M:%S", t), "2026-10-03T12:52:00") << "MkDate with PPLTIME returns unexpected value";
}

} // namespace
