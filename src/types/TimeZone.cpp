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
#include "pplib/types/timezone.h"
#include "pplib/types/string.h"
#include "pplib/types/array.h"
#include "pplib/exceptions.h"
#include "pplib/core/functions.h"
#include <chrono>
#include <atomic>

namespace pplib
{

int16_t get_local_utc_offset_minutes(time_t t = 0)
{
    if (t == 0) {
        t = ::time(nullptr);
    }
    struct tm local_tm, gm_tm;
#ifdef _WIN32
    localtime_s(&local_tm, &t);
    gmtime_s(&gm_tm, &t);
#else
    localtime_r(&t, &local_tm);
    gmtime_r(&t, &gm_tm);
#endif

#if defined(STRUCT_TM_HAS_GMTOFF) || defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__)
    return static_cast<int16_t>(local_tm.tm_gmtoff / 60);
#else
    // Portabler Fallback (z. B. Windows MSVCRT / UCRT)
    time_t local_sec = mktime(&local_tm);
    time_t gm_sec = mktime(&gm_tm);
    if (local_tm.tm_isdst) {
        // Sommerzeit in der lokalen Zeitzone
        local_sec += 3600;
    }
    // printf("t=%llu, local_sec=%llu, gm_sec=%llu\n", t, local_sec, gm_sec);
    return static_cast<int16_t>(difftime(local_sec, gm_sec) / 60);
#endif
}

TimeZone::TimeZone() noexcept
{
    offset_minutes = 0;
    tz_name = String("UTC");
}

TimeZone& TimeZone::setOffsetMinutes(int offset_min)
{
    if (offset_min < -720 || offset_min > 840) throw IllegalArgumentException("Invalid time offset");
    offset_minutes = (int16_t)offset_min;
    return *this;
}

TimeZone& TimeZone::setOffset(int hours, int minutes)
{
    int sign = (hours < 0 || minutes < 0) ? -1 : 1;
    if (hours < -12 || hours > 14 || minutes < -59 || minutes > 59) throw IllegalArgumentException("Invalid time offset");
    return setOffsetMinutes(sign * (abs(hours) * 60 + abs(minutes)));
}

TimeZone TimeZone::fromLocalTime() noexcept
{
    return TimeZone(get_local_utc_offset_minutes());
}

TimeZone TimeZone::fromEpoch(time_t t) noexcept
{
    return TimeZone(get_local_utc_offset_minutes(t));
}

TimeZone TimeZone::fromString(const String& str)
{
    if (str.isEmpty()) return TimeZone::utc();
    String s = UpperCase(Trim(str));
    // printf("TimeZone::fromString: >>%s<<\n", (const char*)str);
    s.trimLeft("[");
    s.trimRight("]");
    if (s == "Z" || s == "UTC" || s == "-Z" || s == "+Z" || s == "-UTC" || s == "+UTC") {
        return TimeZone::utc();
    }
    if (s.size() < 3) {
        throw IllegalArgumentException("TimeZone::fromString: invalid timezone format (%s)", str.c_str());
    }
    int sign = 1;
    if (s[0] == '-') {
        sign = -1;
        s = s.substr(1);
    } else if (s[0] == '+') {
        s = s.substr(1);
    }
    int hours = 0;
    int minutes = 0;
    if (s.find(":") != -1) {
        Array parts(s, ":");
        if (parts.size() != 2) {
            throw IllegalArgumentException("TimeZone::fromString: invalid timezone format (%s)", str.c_str());
        }
        hours = parts[0].toInt();
        minutes = parts[1].toInt();
    } else {
        if (s.size() == 4) {
            hours = s.substr(0, 2).toInt();
            minutes = s.substr(2, 2).toInt();
        } else if (s.size() == 2) {
            hours = s.toInt();
            minutes = 0;
        } else {
            throw IllegalArgumentException("TimeZone::fromString: invalid timezone format (%s)", str.c_str());
        }
    }
    return TimeZone(sign * (hours * 60 + minutes));
}

String TimeZone::toString(bool colon) const
{
    if (offset_minutes == 0) return pplib::String("Z");
    char sign = (offset_minutes < 0) ? '-' : '+';
    int hours = abs(offset_minutes / 60);
    int minutes = abs(offset_minutes) % 60;
    char buffer[16];
    if (colon) {
        snprintf(buffer, sizeof(buffer), "%c%02d:%02d", sign, hours, minutes);
    } else {
        snprintf(buffer, sizeof(buffer), "%c%02d%02d", sign, hours, minutes);
    }
    return String(buffer);
}

} // namespace pplib
