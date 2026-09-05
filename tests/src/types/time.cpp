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

#include <pplib/types/time.h>
#include <pplib/types/timedelta.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

#include "pplib-tests.h"

namespace
{
// The fixture for testing class Foo.
class TimeTest : public ::testing::Test
{
protected:
    TimeTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~TimeTest()
    {
    }
};

TEST_F(TimeTest, ConstructorSimple)
{
    ASSERT_NO_THROW({
        pplib::Time t1;
        ASSERT_EQ((uint64_t)0, t1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, ConstructorWithTimeString)
{
    ASSERT_NO_THROW({
        pplib::Time t1("11:50:11.159473");
        ASSERT_EQ((uint64_t)42611159473, t1.toMicroseconds()) << "Class has unexpected value";
    });

    ASSERT_EQ((uint64_t)15423000000, pplib::Time("4:17-3").toMicroseconds()) << "Class has unexpected value";

    ASSERT_THROW({ pplib::Time t1("11:50"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ pplib::Time t1("11:50:11.159473.123"); }, pplib::IllegalArgumentException);
}

TEST_F(TimeTest, ConstructorWithSeconds)
{
    ASSERT_NO_THROW({
        pplib::Time t1(42611);
        ASSERT_EQ((uint64_t)42611000000, t1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, ConstructorWithHourMinuteSecond)
{
    ASSERT_NO_THROW({
        pplib::Time t1(11, 50, 11, 159473);
        ASSERT_EQ((uint64_t)42611159473, t1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, ConstructorWithHourMinuteSecondNoMicroseconds)
{
    ASSERT_NO_THROW({
        pplib::Time t1(11, 50, 11);
        ASSERT_EQ((uint64_t)42611000000, t1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, ConstructorWithHourMinuteSecondInvalid)
{
    // invalid hour
    ASSERT_THROW({ pplib::Time t1(25, 50, 11, 159473); }, pplib::IllegalArgumentException);

    // invalid minute
    ASSERT_THROW({ pplib::Time t1(11, 60, 11, 159473); }, pplib::IllegalArgumentException);

    // invalid second
    ASSERT_THROW({ pplib::Time t1(11, 50, 60, 159473); }, pplib::IllegalArgumentException);
    // invalid microseconds
    ASSERT_THROW({ pplib::Time t1(11, 50, 11, 1000000); }, pplib::IllegalArgumentException);
}

TEST_F(TimeTest, CopyConstructor)
{
    ASSERT_NO_THROW({
        pplib::Time t1(11, 50, 11, 159473);
        pplib::Time t2 = t1;
        ASSERT_EQ((uint64_t)42611159473, t2.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, MoveConstructor)
{
    ASSERT_NO_THROW({
        pplib::Time t1(11, 50, 11, 159473);
        pplib::Time t2 = std::move(t1);
        ASSERT_EQ((uint64_t)42611159473, t2.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, SetWithHourMinuteSecond)
{
    ASSERT_NO_THROW({
        pplib::Time t1;
        t1.set(11, 50, 11);
        ASSERT_EQ((uint64_t)42611000000, t1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, SetWithHourMinuteSecondAndMicroseconds)
{
    ASSERT_NO_THROW({
        pplib::Time t1;
        t1.set(11, 50, 11, 159473);
        ASSERT_EQ((uint64_t)42611159473, t1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, SetWithSeconds)
{
    ASSERT_NO_THROW({
        pplib::Time t1;
        t1.setFromSeconds(42611);
        ASSERT_EQ((uint64_t)42611000000, t1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, SetWithSecondsWithOverflow)
{
    ASSERT_NO_THROW({
        pplib::Time t1;
        t1.setFromSeconds(90000); // 25 hours, should wrap to 1 hour
        ASSERT_EQ((uint64_t)3600000000, t1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, SetWithMicroseconds)
{
    ASSERT_NO_THROW({
        pplib::Time t1;
        t1.setFromMicroseconds(42611159473);
        ASSERT_EQ((uint64_t)42611159473, t1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, fromMicroseconds)
{
    ASSERT_NO_THROW({
        pplib::Time t1 = pplib::Time::fromMicroseconds(42611159473);
        ASSERT_EQ((uint64_t)42611159473, t1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, SetWithMicrosecondsWithOverflow)
{
    ASSERT_NO_THROW({
        pplib::Time t1;
        t1.setFromMicroseconds(90000000000); // 25 hours in microseconds, should wrap to 1 hour
        ASSERT_EQ((uint64_t)3600000000, t1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, SetWithInvalidChars)
{
    ASSERT_THROW(
        {
            pplib::Time t1;
            t1.set("11:50:11.159473abc");
        },
        pplib::IllegalArgumentException);
}

TEST_F(TimeTest, setHour)
{
    ASSERT_NO_THROW({
        pplib::Time t1;
        t1.setHour(11);
        ASSERT_EQ((uint8_t)11, t1.hour()) << "Unexpected hour value";
    });
    ASSERT_THROW(
        {
            pplib::Time t1;
            t1.setHour(24);
        },
        pplib::IllegalArgumentException);
}

TEST_F(TimeTest, setMinute)
{
    ASSERT_NO_THROW({
        pplib::Time t1;
        t1.setMinute(50);
        ASSERT_EQ((uint8_t)50, t1.minute()) << "Unexpected minute value";
    });
    ASSERT_THROW(
        {
            pplib::Time t1;
            t1.setMinute(60);
        },
        pplib::IllegalArgumentException);
}

TEST_F(TimeTest, setSecond)
{
    ASSERT_NO_THROW({
        pplib::Time t1;
        t1.setSecond(37);
        ASSERT_EQ((uint8_t)37, t1.second()) << "Unexpected second value";
    });
    ASSERT_THROW(
        {
            pplib::Time t1;
            t1.setSecond(60);
        },
        pplib::IllegalArgumentException);
}

TEST_F(TimeTest, setMicrosecond)
{
    ASSERT_NO_THROW({
        pplib::Time t1;
        t1.setMicrosecond(159473);
        ASSERT_EQ((uint32_t)159473, t1.microsecond()) << "Unexpected microsecond value";
    });
    ASSERT_THROW(
        {
            pplib::Time t1;
            t1.setMicrosecond(1000000);
        },
        pplib::IllegalArgumentException);
}

TEST_F(TimeTest, toString)
{
    ASSERT_NO_THROW({
        pplib::Time t1(11, 50, 11, 159473);
        ASSERT_EQ(pplib::String("11:50:11.159473"), t1.toString()) << "Unexpected string representation";
    });
    pplib::Time t2(11, 50, 11, 159473);
    ASSERT_EQ(pplib::String("11:50:11.159473"), static_cast<pplib::String>(t2)) << "Unexpected string representation";
}

TEST_F(TimeTest, format)
{
    pplib::Time t1(19, 50, 11, 159473);

    ASSERT_EQ(pplib::String("19:50:11"), t1.format("%H:%M:%S")) << "Unexpected formatted string";
    ASSERT_EQ(pplib::String("19:50:11.159473"), t1.format("%H:%M:%S.%u")) << "Unexpected formatted string";
    ASSERT_EQ(pplib::String("19:50:11.159"), t1.format("%H:%M:%S.%f")) << "Unexpected formatted string";
    ASSERT_EQ(pplib::String("07:50:11"), t1.format("%I:%M:%S")) << "Unexpected formatted string";

    t1.setHour(0); // In der 12-Stunden-Darstellung entspricht 0 Uhr 12 Uhr Mitternacht
    ASSERT_EQ(pplib::String("12:50:11.159473"), t1.format("%I:%M:%S.%u")) << "Unexpected formatted string";
    t1.setHour(12);
    ASSERT_EQ(pplib::String("12:50:11.159473"), t1.format("%I:%M:%S.%u")) << "Unexpected formatted string";
}

TEST_F(TimeTest, AssignmentOperator)
{
    ASSERT_NO_THROW({
        pplib::Time t1(11, 50, 11, 159473);
        pplib::Time t2;
        t2 = t1;
        ASSERT_EQ((uint64_t)42611159473, t2.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, SelfAssignement)
{
    ASSERT_NO_THROW({
        pplib::Time t1(11, 50, 11, 159473);
        t1 = t1;
        ASSERT_EQ((uint64_t)42611159473, t1.toMicroseconds()) << "Class has unexpected value after self-assignment";
    });
}

TEST_F(TimeTest, MoveOperator)
{
    ASSERT_NO_THROW({
        pplib::Time t1(11, 50, 11, 159473);
        pplib::Time t2;
        t2 = std::move(t1);
        ASSERT_EQ((uint64_t)42611159473, t2.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, MoveSelfAssignement)
{
    ASSERT_NO_THROW({
        pplib::Time t1(11, 50, 11, 159473);
        t1 = std::move(t1);
        ASSERT_EQ((uint64_t)42611159473, t1.toMicroseconds()) << "Class has unexpected value after self-move-assignment";
    });
}

TEST_F(TimeTest, getHour)
{
    pplib::Time t1(11, 50, 37, 159473);
    ASSERT_EQ((uint8_t)11, t1.hour()) << "Unexpected minutes value";
}

TEST_F(TimeTest, getMinute)
{
    pplib::Time t1(11, 50, 37, 159473);
    ASSERT_EQ((uint8_t)50, t1.minute()) << "Unexpected minutes value";
}

TEST_F(TimeTest, getSecond)
{
    pplib::Time t1(11, 50, 37, 159473);
    ASSERT_EQ((uint8_t)37, t1.second()) << "Unexpected minutes value";
}

TEST_F(TimeTest, getMicrosecond)
{
    pplib::Time t1(11, 50, 37, 159473);
    ASSERT_EQ((uint32_t)159473, t1.microsecond()) << "Unexpected minutes value";
}

TEST_F(TimeTest, ComparisonOperators)
{
    ASSERT_NO_THROW({
        pplib::Time t1(11, 50, 11, 159473);
        pplib::Time t2(11, 50, 11, 159473);
        pplib::Time t3(12, 50, 11, 159473);

        ASSERT_TRUE(t1 == t2) << "Expected times to be equal";
        ASSERT_FALSE(t1 != t2) << "Expected times to be equal";
        ASSERT_TRUE(t1 < t3) << "Expected t1 to be less than t3";
        ASSERT_TRUE(t3 > t1) << "Expected t3 to be greater than t1";
        ASSERT_TRUE(t1 <= t2) << "Expected t1 to be less than or equal to t2";
        ASSERT_TRUE(t3 >= t2) << "Expected t3 to be greater than or equal to t2";
    });
}

TEST_F(TimeTest, NowAndUtcNow)
{
    ASSERT_NO_THROW({
        pplib::Time t1 = pplib::Time::now();
        pplib::Time t2 = pplib::Time::utcNow();
        ASSERT_TRUE(t1.toMicroseconds() >= 0) << "Expected now to be non-negative";
        ASSERT_TRUE(t2.toMicroseconds() >= 0) << "Expected utcNow to be non-negative";
    });
}

TEST_F(TimeTest, OutputStreamOperator)
{
    ASSERT_NO_THROW({
        pplib::Time t1(11, 50, 11, 159473);
        std::ostringstream oss;
        oss << t1;
        ASSERT_EQ("11:50:11.159473", oss.str()) << "Unexpected output from stream operator";
    });
}

TEST_F(TimeTest, clear)
{
    ASSERT_NO_THROW({
        pplib::Time t1(11, 50, 11, 159473);
        t1.clear();
        ASSERT_EQ((uint64_t)0, t1.toMicroseconds()) << "Expected time to be cleared to zero";
    });
}

TEST_F(TimeTest, fromSecondsToSecends)
{
    ASSERT_NO_THROW({
        uint32_t seconds = 42611;
        pplib::Time t = pplib::Time::fromSeconds(seconds);
        ASSERT_EQ(seconds, t.toSeconds()) << "Expected time to match the input seconds";
    });
}

TEST_F(TimeTest, operatorPlusTimeDelta)
{
    pplib::Time t(10, 30, 15, 500000);

    // einfache Addition ohne Tagesgrenze
    ASSERT_EQ(pplib::Time(11, 30, 15, 500000), t + pplib::TimeDelta::fromHours(1)) << "Unexpected time";
    ASSERT_EQ(pplib::Time(10, 45, 15, 500000), t + pplib::TimeDelta::fromMinutes(15)) << "Unexpected time";
    ASSERT_EQ(pplib::Time(10, 30, 16, 500000), t + pplib::TimeDelta::fromSeconds(1)) << "Unexpected time";
    ASSERT_EQ(pplib::Time(10, 30, 15, 750000), t + pplib::TimeDelta::fromMicroseconds(250000)) << "Unexpected time";

    // Addition von 0 laesst die Uhrzeit unveraendert
    ASSERT_EQ(t, t + pplib::TimeDelta::fromSeconds(0)) << "Unexpected time";

    // Ueberlauf laeuft bei Mitternacht um
    ASSERT_EQ(pplib::Time(0, 30, 0), pplib::Time(23, 30, 0) + pplib::TimeDelta::fromHours(1)) << "Unexpected time";
    ASSERT_EQ(pplib::Time(0, 0, 0), pplib::Time(23, 59, 59, 999999) + pplib::TimeDelta::fromMicroseconds(1))
        << "Unexpected time";

    // negatives Delta laeuft nach hinten ueber Mitternacht
    ASSERT_EQ(pplib::Time(23, 30, 0), pplib::Time(0, 30, 0) + pplib::TimeDelta::fromHours(-1)) << "Unexpected time";

    // Deltas ueber mehrere Tage werden modulo 24h reduziert
    ASSERT_EQ(pplib::Time(2, 0, 0), pplib::Time(0, 0, 0) + pplib::TimeDelta::fromHours(50)) << "Unexpected time";
    ASSERT_EQ(pplib::Time(12, 0, 0), pplib::Time(12, 0, 0) + pplib::TimeDelta::fromDays(3)) << "Unexpected time";
    ASSERT_EQ(pplib::Time(22, 0, 0), pplib::Time(0, 0, 0) + pplib::TimeDelta::fromHours(-50)) << "Unexpected time";
}

TEST_F(TimeTest, operatorMinusTimeDelta)
{
    pplib::Time t(10, 30, 15, 500000);

    // einfache Subtraktion ohne Tagesgrenze
    ASSERT_EQ(pplib::Time(9, 30, 15, 500000), t - pplib::TimeDelta::fromHours(1)) << "Unexpected time";
    ASSERT_EQ(pplib::Time(10, 15, 15, 500000), t - pplib::TimeDelta::fromMinutes(15)) << "Unexpected time";
    ASSERT_EQ(pplib::Time(10, 30, 15, 250000), t - pplib::TimeDelta::fromMicroseconds(250000)) << "Unexpected time";

    ASSERT_EQ(t, t - pplib::TimeDelta::fromSeconds(0)) << "Unexpected time";

    // Unterlauf laeuft rueckwaerts ueber Mitternacht
    ASSERT_EQ(pplib::Time(23, 30, 0), pplib::Time(0, 30, 0) - pplib::TimeDelta::fromHours(1)) << "Unexpected time";
    ASSERT_EQ(pplib::Time(23, 59, 59, 999999), pplib::Time(0, 0, 0) - pplib::TimeDelta::fromMicroseconds(1))
        << "Unexpected time";

    // negatives Delta addiert entsprechend
    ASSERT_EQ(pplib::Time(0, 30, 0), pplib::Time(23, 30, 0) - pplib::TimeDelta::fromHours(-1)) << "Unexpected time";

    // Deltas ueber mehrere Tage
    ASSERT_EQ(pplib::Time(22, 0, 0), pplib::Time(0, 0, 0) - pplib::TimeDelta::fromHours(50)) << "Unexpected time";
    ASSERT_EQ(pplib::Time(12, 0, 0), pplib::Time(12, 0, 0) - pplib::TimeDelta::fromDays(3)) << "Unexpected time";
}

TEST_F(TimeTest, operatorPlusMinusAssignTimeDelta)
{
    pplib::Time t(10, 0, 0);
    t += pplib::TimeDelta::fromHours(1);
    ASSERT_EQ(pplib::Time(11, 0, 0), t) << "Unexpected time";
    t -= pplib::TimeDelta::fromMinutes(90);
    ASSERT_EQ(pplib::Time(9, 30, 0), t) << "Unexpected time";

    // Umlauf bei Mitternacht auch bei den Zuweisungsoperatoren
    pplib::Time t2(0, 15, 0);
    t2 -= pplib::TimeDelta::fromHours(1);
    ASSERT_EQ(pplib::Time(23, 15, 0), t2) << "Unexpected time";
    t2 += pplib::TimeDelta::fromHours(1);
    ASSERT_EQ(pplib::Time(0, 15, 0), t2) << "Unexpected time";
}

TEST_F(TimeTest, operatorPlusMinusRoundtrip)
{
    // Hin- und Zurueckrechnen muss die Ausgangszeit ergeben, auch ueber Mitternacht hinweg
    pplib::Time t(0, 5, 30, 123456);
    pplib::TimeDelta d = pplib::TimeDelta::fromHours(7);
    ASSERT_EQ(t, (t + d) - d) << "Unexpected time";
    ASSERT_EQ(t, (t - d) + d) << "Unexpected time";
}

TEST_F(TimeTest, operatorMinusTime)
{
    // Die Differenz zweier Uhrzeiten ist eine Zeitspanne, kein Zeitpunkt
    pplib::Time t1800(18, 0, 0);
    pplib::Time t0600(6, 0, 0);

    ASSERT_EQ(pplib::TimeDelta::fromHours(12), t1800 - t0600) << "Unexpected delta";
    // umgekehrte Reihenfolge ergibt eine negative Zeitspanne
    ASSERT_EQ(pplib::TimeDelta::fromHours(-12), t0600 - t1800) << "Unexpected delta";
    // Differenz zu sich selbst ist 0
    ASSERT_EQ(pplib::TimeDelta::fromSeconds(0), t1800 - t1800) << "Unexpected delta";

    // Mikrosekunden-Genauigkeit bleibt erhalten
    ASSERT_EQ(pplib::TimeDelta::fromMicroseconds(1),
              pplib::Time(0, 0, 0, 1) - pplib::Time(0, 0, 0, 0))
        << "Unexpected delta";
    // 1 us - 86399999999 us
    ASSERT_EQ(pplib::TimeDelta::fromMicroseconds(-86399999998LL), pplib::Time(0, 0, 0, 1) - pplib::Time(23, 59, 59, 999999))
        << "Unexpected delta";
}

} // namespace
