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
class String;

class TimeDelta
{
private:
    int64_t us = 0;

    explicit constexpr TimeDelta(int64_t microseconds) noexcept
        : us(microseconds)
    {
    }

public:
    TimeDelta() noexcept = default;

    explicit TimeDelta(
        int64_t days, int64_t hours, int64_t minutes = 0, int64_t seconds = 0, int64_t milliseconds = 0, int64_t microseconds = 0) noexcept
    {
        set(days, hours, minutes, seconds, milliseconds, microseconds);
    }

    /** @brief Konstruktor mit String
     *
     * Ein TimeDelta wird aus einem String erzeugt. Der String muss im Format "HH:MM" oder "HH:MM:SS" oder "HH:MM:SS.UUUUUU" vorliegen.
     * Es werden auch die Trennzeichen "," oder "." oder "-" akzeptiert.
     *
     * @param str String mit der Zeitangabe
     * @exception IllegalArgumentException
     */ // TODO: Days mit einbringen
    TimeDelta(const String& str)
    {
        us = fromString(str).us;
    }

    int64_t toMicroseconds() const noexcept
    {
        return us;
    }
    int64_t toMilliseconds() const noexcept
    {
        return us / 1000;
    }
    int64_t toSeconds() const noexcept
    {
        return us / 1000000;
    }

    // TODO: toString() mit optionalen Parametern (nur HH:MM, HH:MM:SS, HH:MM:SS.UUUUUU, optional mit Vorzeichen)

    // Setter
    TimeDelta& set(int64_t days,
                   int64_t hours = 0,
                   int64_t minutes = 0,
                   int64_t seconds = 0,
                   int64_t milliseconds = 0,
                   int64_t microseconds = 0) noexcept
    {
        us = (days * 86400000000LL) + (hours * 3600000000LL) + (minutes * 60000000LL) + (seconds * 1000000LL) + (milliseconds * 1000LL) +
             microseconds;
        return *this;
    }

    // Named Factories

    /** @brief Erzeugt ein TimeDelta aus einem String
     *
     * Ein TimeDelta wird aus einem String erzeugt. Der String muss im Format "HH:MM" oder "HH:MM:SS" oder "HH:MM:SS.UUUUUU" vorliegen.
     * Es werden auch die Trennzeichen "," oder "." oder "-" akzeptiert.
     *
     * @param str String mit der Zeitangabe
     * @return TimeDelta Objekt
     * @exception IllegalArgumentException
     */
    static TimeDelta fromString(const String& str);

    static TimeDelta fromWeeks(int64_t weeks) noexcept
    {
        return fromDays(weeks * 7);
    }
    static TimeDelta fromDays(int64_t days) noexcept
    {
        return TimeDelta(days * 86400000000LL);
    }
    static TimeDelta fromHours(int64_t hours) noexcept
    {
        return TimeDelta(hours * 3600000000LL);
    }
    static TimeDelta fromMinutes(int64_t mins) noexcept
    {
        return TimeDelta(mins * 60000000LL);
    }
    static TimeDelta fromSeconds(int64_t secs) noexcept
    {
        return TimeDelta(secs * 1000000LL);
    }
    static TimeDelta fromMilliseconds(int64_t ms) noexcept
    {
        return TimeDelta(ms * 1000LL);
    }
    static TimeDelta fromMicroseconds(int64_t us) noexcept
    {
        return TimeDelta(us);
    }
    // Operatoren
    TimeDelta operator+(const TimeDelta& other) const noexcept
    {
        return TimeDelta(us + other.us);
    }
    TimeDelta operator-(const TimeDelta& other) const noexcept
    {
        return TimeDelta(us - other.us);
    }
    TimeDelta operator-() const noexcept
    {
        return TimeDelta(-us);
    }

    TimeDelta& operator+=(const TimeDelta& other) noexcept
    {
        us += other.us;
        return *this;
    }
    TimeDelta& operator-=(const TimeDelta& other) noexcept
    {
        us -= other.us;
        return *this;
    }

    TimeDelta operator*(int64_t factor) const noexcept
    {
        return TimeDelta(us * factor);
    }
    TimeDelta operator/(int64_t divisor) const noexcept
    {
        return TimeDelta(us / divisor);
    }

    bool operator==(const TimeDelta& other) const noexcept
    {
        return us == other.us;
    }
    bool operator!=(const TimeDelta& other) const noexcept
    {
        return us != other.us;
    }
    bool operator<(const TimeDelta& other) const noexcept
    {
        return us < other.us;
    }
    bool operator<=(const TimeDelta& other) const noexcept
    {
        return us <= other.us;
    }
    bool operator>(const TimeDelta& other) const noexcept
    {
        return us > other.us;
    }
    bool operator>=(const TimeDelta& other) const noexcept
    {
        return us >= other.us;
    }
};

} // namespace pplib

#endif // PPLIB_TYPES_TIMEDELTA_H_