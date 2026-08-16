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

namespace pplib
{

TimeZone TimeZone::fromString(const String& str)
{
    String s = Trim(str);
    if (s == "Z" || s == "UTC") {
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

String TimeZone::toISO8601String(bool colon) const
{
    int hours = offset_minutes / 60;
    int minutes = abs(offset_minutes % 60);
    char buffer[16];
    if (offset_minutes == 0) {
        return "Z";
    } else {
        if (colon) {
            snprintf(buffer, sizeof(buffer), "%+03d:%02d", hours, minutes);
        } else {
            snprintf(buffer, sizeof(buffer), "%+03d%02d", hours, minutes);
        }
        return String(buffer);
    }
}

} // namespace pplib
