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

#ifndef PPLIB_TYPES_TIMEDELTA_H_
#define PPLIB_TYPES_TIMEDELTA_H_

#include <stdint.h>

namespace pplib
{

class TimeDelta
{
private:
    int64_t microseconds = 0;

public:
    TimeDelta() noexcept = default;
    TimeDelta(int64_t microseconds) noexcept
        : microseconds(microseconds)
    {
    }
    TimeDelta(const TimeDelta& other) noexcept
        : microseconds(other.microseconds)
    {
    }
    TimeDelta(TimeDelta&& other) noexcept
        : microseconds(other.microseconds)
    {
        other.microseconds = 0;
    }

    int64_t toMicroseconds() const noexcept
    {
        return microseconds;
    }
    int64_t toMilliseconds() const noexcept
    {
        return microseconds / 1000;
    }
    int64_t toSeconds() const noexcept
    {
        return microseconds / 1000000;
    }

    TimeDelta& operator=(const TimeDelta& other) noexcept
    {
        microseconds = other.microseconds;
        return *this;
    }

    TimeDelta& operator=(TimeDelta&& other) noexcept
    {
        microseconds = other.microseconds;
        other.microseconds = 0;
        return *this;
    }
};

} // namespace pplib

#endif /* PPLIB_TYPES_TIME_H_ */