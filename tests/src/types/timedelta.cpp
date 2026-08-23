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

#include <pplib/types/timedelta.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

#include "pplib-tests.h"

namespace
{

TEST(TimeDeltaTest, ConstructorSimple)
{
    ASSERT_NO_THROW({
        pplib::TimeDelta td1;
        ASSERT_EQ((uint64_t)0, td1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST(TimeDeltaTest, ConstructorWithValues)
{
    ASSERT_NO_THROW({
        pplib::TimeDelta td1(1, 2, 3, 4, 5, 6);
        ASSERT_EQ((uint64_t)93784005006, td1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST(TimeDeltaTest, ConstructorWithValuesNegative)
{
    ASSERT_NO_THROW({
        pplib::TimeDelta td1(-1, -2, -3, -4, -5, -6);
        ASSERT_EQ((uint64_t)-93784005006, td1.toMicroseconds()) << "Class has unexpected value";
    });
}

TEST(TimeDeltaTest, SetValues)
{
    pplib::TimeDelta td1;
    td1.set(1, 2, 3, 4, 5, 6);
    ASSERT_EQ((uint64_t)93784005006, td1.toMicroseconds()) << "Class has unexpected value";
    td1.set(-1, -2, -3, -4, -5, -6);
    ASSERT_EQ((uint64_t)-93784005006, td1.toMicroseconds()) << "Class has unexpected value";
}

TEST(TimeDeltaTest, toMicroseconds)
{
    pplib::TimeDelta td1(1, 2, 3, 4, 5, 6);
    ASSERT_EQ((uint64_t)93784005006, td1.toMicroseconds()) << "Class has unexpected value";
}

TEST(TimeDeltaTest, toMilliseconds)
{
    pplib::TimeDelta td1(1, 2, 3, 4, 5, 6);
    ASSERT_EQ((uint64_t)93784005, td1.toMilliseconds()) << "Class has unexpected value";
}

TEST(TimeDeltaTest, toSeconds)
{
    pplib::TimeDelta td1(1, 2, 3, 4, 5, 6);
    ASSERT_EQ((uint64_t)93784, td1.toSeconds()) << "Class has unexpected value";
}

TEST(TimeDeltaTest, fromWeeks)
{
    pplib::TimeDelta td1 = pplib::TimeDelta::fromWeeks(1);
    ASSERT_EQ((uint64_t)604800000000, td1.toMicroseconds()) << "Class has unexpected value";
}

TEST(TimeDeltaTest, fromDays)
{
    pplib::TimeDelta td1 = pplib::TimeDelta::fromDays(1);
    ASSERT_EQ((uint64_t)86400000000, td1.toMicroseconds()) << "Class has unexpected value";
}

TEST(TimeDeltaTest, fromHours)
{
    pplib::TimeDelta td1 = pplib::TimeDelta::fromHours(1);
    ASSERT_EQ((uint64_t)3600000000, td1.toMicroseconds()) << "Class has unexpected value";
}

TEST(TimeDeltaTest, fromMinutes)
{
    pplib::TimeDelta td1 = pplib::TimeDelta::fromMinutes(1);
    ASSERT_EQ((uint64_t)60000000, td1.toMicroseconds()) << "Class has unexpected value";
}

TEST(TimeDeltaTest, fromSeconds)
{
    pplib::TimeDelta td1 = pplib::TimeDelta::fromSeconds(1);
    ASSERT_EQ((uint64_t)1000000, td1.toMicroseconds()) << "Class has unexpected value";
}

TEST(TimeDeltaTest, fromMilliseconds)
{
    pplib::TimeDelta td1 = pplib::TimeDelta::fromMilliseconds(1);
    ASSERT_EQ((uint64_t)1000, td1.toMicroseconds()) << "Class has unexpected value";
}

TEST(TimeDeltaTest, fromMicroseconds)
{
    pplib::TimeDelta td1 = pplib::TimeDelta::fromMicroseconds(1);
    ASSERT_EQ((uint64_t)1, td1.toMicroseconds()) << "Class has unexpected value";
}

TEST(TimeDeltaTest, ComparisonOperators)
{
    ASSERT_NO_THROW({
        pplib::TimeDelta td1(1, 2, 3, 4, 5, 6);
        pplib::TimeDelta td2(1, 2, 3, 4, 5, 6);
        pplib::TimeDelta td3(2, 3, 4, 5, 6, 7);

        ASSERT_TRUE(td1 == td2) << "Expected TimeDeltas to be equal";
        ASSERT_FALSE(td1 != td2) << "Expected TimeDeltas to be equal";
        ASSERT_TRUE(td1 < td3) << "Expected td1 to be less than td3";
        ASSERT_TRUE(td3 > td1) << "Expected td3 to be greater than td1";
        ASSERT_TRUE(td1 <= td2) << "Expected td1 to be less than or equal to td2";
        ASSERT_TRUE(td3 >= td2) << "Expected td3 to be greater than or equal to td2";
    });
}

TEST(TimeDeltaTest, ArithmeticOperators)
{
    ASSERT_NO_THROW({
        pplib::TimeDelta td1(1, 2, 3, 4, 5, 6);
        pplib::TimeDelta td2(2, 3, 4, 5, 6, 7);

        ASSERT_EQ((uint64_t)93784005006, td1.toMicroseconds()) << "Unexpected value for td1";
        ASSERT_EQ((uint64_t)183845006007, td2.toMicroseconds()) << "Unexpected value for td2";

        ASSERT_EQ((uint64_t)277629011013, (td1 + td2).toMicroseconds()) << "Unexpected value for td1 + td2";
        ASSERT_EQ((uint64_t)-90061001001, (td1 - td2).toMicroseconds()) << "Unexpected value for td1 - td2";
        ASSERT_EQ((uint64_t)-93784005006, (-td1).toMicroseconds()) << "Unexpected value for -td1";

        pplib::TimeDelta td3 = td1;
        td3 += td2;
        ASSERT_EQ((uint64_t)277629011013, td3.toMicroseconds()) << "Unexpected value for td3 after td3 += td2";

        pplib::TimeDelta td4 = td1;
        td4 -= td2;
        ASSERT_EQ((uint64_t)-90061001001, td4.toMicroseconds()) << "Unexpected value for td4 after td4 -= td2";

        ASSERT_EQ((uint64_t)187568010012, (td1 * 2).toMicroseconds()) << "Unexpected value for td1 * 2";
        ASSERT_EQ((uint64_t)46892002503, (td1 / 2).toMicroseconds()) << "Unexpected value for td1 / 2";
    });
}

TEST(TimeDeltaTest, MoreOperators)
{
    pplib::TimeDelta td = pplib::TimeDelta::fromDays(1) + pplib::TimeDelta::fromHours(2) + pplib::TimeDelta::fromMinutes(3) +
                          pplib::TimeDelta::fromSeconds(4) + pplib::TimeDelta::fromMilliseconds(5) + pplib::TimeDelta::fromMicroseconds(6);
    ASSERT_EQ((uint64_t)93784005006, td.toMicroseconds()) << "Unexpected value for td";
    td.set(0, 0, 0, 0, 0, 0);
    ASSERT_EQ((uint64_t)0, td.toMicroseconds()) << "Unexpected value for td after set to zero";
}

TEST(TimeDeltaTest, fromString)
{
    ASSERT_NO_THROW({
        pplib::TimeDelta td1 = pplib::TimeDelta::fromString("01:02:03.004005");
        ASSERT_EQ((uint64_t)3723004005, td1.toMicroseconds()) << "Unexpected value for td1";

        pplib::TimeDelta td2 = pplib::TimeDelta::fromString("01:02:03,004005");
        ASSERT_EQ((uint64_t)3723004005, td2.toMicroseconds()) << "Unexpected value for td2";

        pplib::TimeDelta td3 = pplib::TimeDelta::fromString("01:02:03-004005");
        ASSERT_EQ((uint64_t)3723004005, td3.toMicroseconds()) << "Unexpected value for td3";
        pplib::TimeDelta td4 = pplib::TimeDelta::fromString("01:02:03");
        ASSERT_EQ((uint64_t)3723000000, td4.toMicroseconds()) << "Unexpected value for td4";
        pplib::TimeDelta td5 = pplib::TimeDelta::fromString("01:02");
        ASSERT_EQ((uint64_t)3720000000, td5.toMicroseconds()) << "Unexpected value for td5";
    });

    ASSERT_THROW({ pplib::TimeDelta td4 = pplib::TimeDelta::fromString("invalid"); }, pplib::IllegalArgumentException);

    ASSERT_THROW({ pplib::TimeDelta td5 = pplib::TimeDelta::fromString("01:02:03.004005.006"); }, pplib::IllegalArgumentException);

    ASSERT_THROW({ pplib::TimeDelta td5 = pplib::TimeDelta::fromString("01"); }, pplib::IllegalArgumentException);
    ASSERT_THROW({ pplib::TimeDelta td5 = pplib::TimeDelta::fromString("aa.aa"); }, pplib::IllegalArgumentException);
}

} // namespace
