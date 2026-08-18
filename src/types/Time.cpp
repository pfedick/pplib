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
#include "pplib/types/time.h"
#include "pplib/types/string.h"
#include "pplib/types/array.h"
#include "pplib/exceptions.h"
#include "pplib/core/functions.h"
#include <chrono>

namespace pplib
{

Time::Time(const Time& other) noexcept
{
    hh = other.hh;
    ii = other.ii;
    ss = other.ss;
    us = other.us;
}

Time::Time(Time&& other) noexcept
{
    hh = other.hh;
    ii = other.ii;
    ss = other.ss;
    us = other.us;
    other.hh = 0;
    other.ii = 0;
    other.ss = 0;
    other.us = 0;
}

Time& Time::setFromSeconds(uint32_t seconds) noexcept
{
    seconds %= 86400;
    hh = seconds / 3600;
    ii = (seconds % 3600) / 60;
    ss = seconds % 60;
    us = 0;
    return *this;
}

Time& Time::setFromMicroseconds(uint64_t microseconds) noexcept
{
    uint64_t seconds = microseconds / 1000000;
    setFromSeconds(seconds);
    us = microseconds % 1000000;
    return *this;
}

Time& Time::set(uint8_t hour, uint8_t minute, uint8_t second, uint32_t microseconds)
{
    if (hour > 23 || minute > 59 || second > 59 || microseconds > 999999) {
        throw IllegalArgumentException("Time::set: invalid time components (%u:%u:%u.%u)", hour, minute, second, microseconds);
    }
    hh = hour;
    ii = minute;
    ss = second;
    us = microseconds;
    return *this;
}

Time& Time::set(const String& time)
{
    // Format: HH:MM:SS[.uuuuuu]
    // HH, MM, SS und uuuuuu können auch weniger als 2 bzw. 6 Stellen haben, führende Nullen sind optional.
    // Trennzeichen kann auch ein Punkt, Komma oder Minus sein.
    String t = UpperCase(Trim(time));
    t.replace(",", ".");
    t.replace(".", ".");
    t.replace(":", ".");
    t.replace("-", ".");
    pplib::Array parts(t, ".");

    // Prüfen, ob alle Werte Numerisch sind
    for (size_t i = 0; i < parts.size(); ++i) {
        if (!parts[i].isNumeric()) {
            throw IllegalArgumentException("Date::set: invalid date format (%s)", time.c_str());
        }
    }
    if (parts.size() < 3 || parts.size() > 4) {
        throw IllegalArgumentException("Time::set: invalid time format (%s)", time.c_str());
    }
    if (parts.size() < 4) {
        parts.add("000000"); // Add microseconds part if missing
    }

    return set(parts.at(0).toInt(), parts.at(1).toInt(), parts.at(2).toInt(), parts.at(3).toInt());
}

Time& Time::setHour(uint8_t hour)
{
    if (hour > 23) throw IllegalArgumentException("Time::setHours: hour > 23 (%u)", hour);
    hh = hour;
    return *this;
}

Time& Time::setMinute(uint8_t minute)
{
    if (minute > 59) throw IllegalArgumentException("Time::setMinutes: minute > 59 (%u)", minute);
    ii = minute;
    return *this;
}

Time& Time::setSecond(uint8_t second)
{
    if (second > 59) throw IllegalArgumentException("Time::setSeconds: second > 59 (%u)", second);
    ss = second;
    return *this;
}

Time& Time::setMicrosecond(uint32_t microsecond)
{
    if (microsecond > 999999) throw IllegalArgumentException("Time::setMicroseconds: usec > 999999 (%u)", microsecond);
    us = microsecond;
    return *this;
}

Time& Time::operator=(const Time& other) noexcept
{
    hh = other.hh;
    ii = other.ii;
    ss = other.ss;
    us = other.us;
    return *this;
}

Time& Time::operator=(Time&& other) noexcept
{
    hh = other.hh;
    ii = other.ii;
    ss = other.ss;
    us = other.us;
    other.hh = 0;
    other.ii = 0;
    other.ss = 0;
    other.us = 0;
    return *this;
}

String Time::toString() const
{
    String result;
    result.setf("%02u:%02u:%02u.%06u", hh, ii, ss, us);
    return result;
}

String Time::format(const String& format) const
{
    String r = format;
    String Tmp;
    Tmp.setf("%02d", hh);
    r.replace("%H", Tmp);
    Tmp.setf("%02d", hh % 12);
    r.replace("%I", Tmp);
    Tmp.setf("%02d", ii);
    r.replace("%M", Tmp);
    Tmp.setf("%02d", ss);
    r.replace("%S", Tmp);
    Tmp.setf("%06d", us);
    r.replace("%u", Tmp);
    Tmp.setf("%03d", us / 1000);
    r.replace("%f", Tmp);
    return r;
}

Time Time::now()
{
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = *std::localtime(&now_time_t);
    auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000000;

    return Time(now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec, static_cast<uint32_t>(now_us));
}

Time Time::utcNow()
{
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = *std::gmtime(&now_time_t);
    auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000000;

    return Time(now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec, static_cast<uint32_t>(now_us));
}

} // namespace pplib
