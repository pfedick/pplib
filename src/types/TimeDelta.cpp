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

#include <stdint.h>
#include "pplib/types/timedelta.h"
#include "pplib/types/string.h"
#include "pplib/types/array.h"
#include <pplib/core/functions.h>
#include <pplib/exceptions.h>

namespace pplib
{
TimeDelta TimeDelta::fromString(const String& str)
{
    String s = Trim(str);
    int sign = 1;
    if (s.startsWith("-")) {
        sign = -1;
        s = s.substr(1);
    }
    Array parts(s, ":");

    // parts.list("parts");
    int days = 0, hours = 0, minutes = 0, seconds = 0, microseconds = 0;
    if (parts.size() < 2 || parts.size() > 4) {
        throw IllegalArgumentException("TimeDelta::fromString: invalid time delta format (%s)", str.c_str());
    }
    // wenn wir 3 oder 4 Teile haben, dann ist der letzte Teil die Sekunden, die
    // optional auch Mikrosekunden enthalten können.
    if (parts.size() == 3 || parts.size() == 4) {
        Array secParts(parts.pop(), ".");
        pplib::String first = secParts.shift();
        if (first.isEmpty()) first = "0";
        parts.add(first);
        if (secParts.size() == 1) {
            if (!secParts.at(0).isInteger() || secParts.at(0).toInt() < 0) {
                throw IllegalArgumentException("TimeDelta::fromString: invalid time delta format (%s)", str.c_str());
            }
            microseconds = secParts.at(0).toInt();
        } else if (secParts.size() > 1) {
            throw IllegalArgumentException("TimeDelta::fromString: invalid time delta format (%s)", str.c_str());
        }
    }
    for (size_t i = 0; i < parts.size(); ++i) {
        if (!parts[i].isInteger() || parts[i].toInt() < 0) {
            throw IllegalArgumentException("TimeDelta::fromString: invalid time delta format (%s)", str.c_str());
        }
    }
    if (parts.size() == 4) {
        days = parts.shift().toInt();
        if (days > 106751991LL) {
            throw IllegalArgumentException("TimeDelta::fromString: value out of range (%s)", str.c_str());
        }
    }

    hours = parts.at(0).toInt();
    minutes = parts.at(1).toInt();
    if (parts.size() > 2) {
        seconds = parts.at(2).toInt();
    }
    int64_t newus = (days * 86400000000LL) + (hours * 3600000000LL) + (minutes * 60000000LL) + (seconds * 1000000LL) + microseconds;
    return TimeDelta(newus * sign);
}

String TimeDelta::toString() const
{
    int64_t total_microseconds = us;
    int sign = 1;
    if (total_microseconds < 0) {
        sign = -1;
        total_microseconds = -total_microseconds;
    }
    int64_t days = total_microseconds / 86400000000LL;
    total_microseconds %= 86400000000LL;
    int64_t hours = total_microseconds / 3600000000LL;
    total_microseconds %= 3600000000LL;
    int64_t minutes = total_microseconds / 60000000LL;
    total_microseconds %= 60000000LL;
    int64_t seconds = total_microseconds / 1000000LL;
    int64_t microseconds = total_microseconds % 1000000LL;
    if (days) {
        return String::format("%s%lld:%02lld:%02lld:%02lld.%06lld", sign < 0 ? "-" : "", days, hours, minutes, seconds, microseconds);
    }
    return String::format("%s%02lld:%02lld:%02lld.%06lld", sign < 0 ? "-" : "", hours, minutes, seconds, microseconds);
}

TimeDelta TimeDelta::operator/(int64_t divisor) const
{
    if (divisor == 0) throw IllegalArgumentException("TimeDelta::operator/: division by zero");
    return TimeDelta(us / divisor);
}

} // namespace pplib