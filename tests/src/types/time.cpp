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
        t1.setSeconds(42611);
        ASSERT_EQ((uint64_t)42611000000, t1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, SetWithSecondsWithOverflow)
{
    ASSERT_NO_THROW({
        pplib::Time t1;
        t1.setSeconds(90000); // 25 hours, should wrap to 1 hour
        ASSERT_EQ((uint64_t)3600000000, t1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, SetWithMicroseconds)
{
    ASSERT_NO_THROW({
        pplib::Time t1;
        t1.setMicroseconds(42611159473);
        ASSERT_EQ((uint64_t)42611159473, t1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, SetWithMicrosecondsWithOverflow)
{
    ASSERT_NO_THROW({
        pplib::Time t1;
        t1.setMicroseconds(90000000000); // 25 hours in microseconds, should wrap to 1 hour
        ASSERT_EQ((uint64_t)3600000000, t1.toMicroseconds()) << "Class has unexpected value";
    });
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
    ASSERT_EQ(pplib::String("19:50:11.159473"), t1.format("%H:%M:%S.%f")) << "Unexpected formatted string";
    ASSERT_EQ(pplib::String("07:50:11"), t1.format("%I:%M:%S")) << "Unexpected formatted string";
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

TEST_F(TimeTest, MoveOperator)
{
    ASSERT_NO_THROW({
        pplib::Time t1(11, 50, 11, 159473);
        pplib::Time t2;
        t2 = std::move(t1);
        ASSERT_EQ((uint64_t)42611159473, t2.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST_F(TimeTest, getHours)
{
    pplib::Time t1(11, 50, 37, 159473);
    ASSERT_EQ((uint8_t)11, t1.hours()) << "Unexpected minutes value";
}

TEST_F(TimeTest, getMinutes)
{
    pplib::Time t1(11, 50, 37, 159473);
    ASSERT_EQ((uint8_t)50, t1.minutes()) << "Unexpected minutes value";
}

TEST_F(TimeTest, getSeconds)
{
    pplib::Time t1(11, 50, 37, 159473);
    ASSERT_EQ((uint8_t)37, t1.seconds()) << "Unexpected minutes value";
}

TEST_F(TimeTest, getMicroseconds)
{
    pplib::Time t1(11, 50, 37, 159473);
    ASSERT_EQ((uint32_t)159473, t1.microseconds()) << "Unexpected minutes value";
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

} // namespace
