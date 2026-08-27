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

#ifndef PPLIB_TYPES_TIMEZONE_H_
#define PPLIB_TYPES_TIMEZONE_H_

#include <stdint.h>
#include <ostream>
#include "pplib/types/string.h"

namespace pplib
{

/** @class TimeZone
 * @brief Datentyp für Zeitzonen
 * @ingroup PPLGroupDataTypes
 *
 * Diese Klasse stellt Methoden zum Verwalten und Formatieren von Zeitzonen bereit.
 */
class TimeZone
{
private:
    int16_t offset_minutes = 0; //!< Offset zu UTC in Minuten (-720 bis +840)
    String tz_name;             //!< Optional: "UTC", "CET", "CEST", "Europe/Berlin"

public:
    TimeZone() noexcept;
    explicit TimeZone(int16_t offset_min, const String& name = String())
        : tz_name(name)
    {
        setOffsetMinutes(offset_min);
    }

    TimeZone(int8_t hours, int8_t minutes, const String& name = String())
        : tz_name(name)
    {
        setOffset(hours, minutes);
    }

    // Setter
    TimeZone& setOffsetMinutes(int16_t offset_min);
    TimeZone& setOffsetSeconds(int16_t offset_seconds)
    {
        return setOffsetMinutes(offset_seconds / 60);
    }

    TimeZone& setOffset(int8_t hours, int8_t minutes);
    TimeZone& set(int8_t hours, int8_t minutes);

    void setName(const String& name)
    {
        tz_name = name;
    }

    // Fabriken
    static TimeZone utc() noexcept
    {
        return TimeZone(0, "UTC");
    }
    static TimeZone fromHours(int hours) noexcept
    {
        return TimeZone(hours * 60);
    }
    static TimeZone fromHoursAndMinutes(int hours, int minutes) noexcept
    {
        int sign = (hours < 0 || minutes < 0) ? -1 : 1;
        return TimeZone(sign * (abs(hours) * 60 + abs(minutes)));
    }
    static TimeZone fromString(const String& str); // parst "+02:00", "+0200", "Z", "UTC"

    static TimeZone fromLocalTime() noexcept;
    static TimeZone fromEpoch(time_t t) noexcept;

    // Getter
    int16_t offsetMinutes() const noexcept
    {
        return offset_minutes;
    }
    int32_t offsetSeconds() const noexcept
    {
        return offset_minutes * 60;
    }
    const String& name() const noexcept
    {
        return tz_name;
    }

    inline bool isUTC() const noexcept
    {
        return offset_minutes == 0;
    }

    inline bool operator==(const TimeZone& other) const noexcept
    {
        return offset_minutes == other.offset_minutes;
    }
    inline bool operator!=(const TimeZone& other) const noexcept
    {
        return offset_minutes != other.offset_minutes;
    }
    inline bool operator<(const TimeZone& other) const noexcept
    {
        return offset_minutes < other.offset_minutes;
    }
    inline bool operator<=(const TimeZone& other) const noexcept
    {
        return offset_minutes <= other.offset_minutes;
    }
    inline bool operator>(const TimeZone& other) const noexcept
    {
        return offset_minutes > other.offset_minutes;
    }
    inline bool operator>=(const TimeZone& other) const noexcept
    {
        return offset_minutes >= other.offset_minutes;
    }

    // Formatierung z. B. "+02:00" oder "+0200"
    String toString(bool colon = true) const;
};

} // namespace pplib

#endif // PPLIB_TYPES_TIMEZONE_H_