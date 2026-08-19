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
    String s = UpperCase(Trim(str));
    s.replace(",", ":");
    s.replace(".", ":");
    s.replace("-", ":");
    Array parts(str, ":");
    int hours = 0, minutes = 0, seconds = 0, microseconds = 0;
    if (parts.size() < 2 || parts.size() > 4) {
        throw IllegalArgumentException("TimeDelta::fromString: invalid time delta format (%s)", str.c_str());
    }
    for (size_t i = 0; i < parts.size(); ++i) {
        if (!parts[i].isNumeric()) {
            throw IllegalArgumentException("TimeDelta::fromString: invalid time delta format (%s)", str.c_str());
        }
    }

    hours = parts.at(0).toInt();
    minutes = parts.at(1).toInt();
    if (parts.size() > 2) {
        seconds = parts.at(2).toInt();
    }
    if (parts.size() > 3) {
        microseconds = parts.at(3).toInt();
    }
    return TimeDelta(0, hours, minutes, seconds, 0, microseconds);
}

} // namespace pplib