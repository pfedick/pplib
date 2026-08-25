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

#ifndef PPLIB_TYPES_DATE_H_
#define PPLIB_TYPES_DATE_H_

#include <stdint.h>
#include <ostream>
#include "pplib/types/string.h"

namespace pplib
{

class Date
{
private:
    uint8_t dd = 0;  //!< Tag
    uint8_t mm = 0;  //!< Monat
    uint16_t yy = 0; //!< Jahr

public:
    Date() noexcept = default;
    Date(const Date& other) noexcept
        : dd(other.dd),
          mm(other.mm),
          yy(other.yy)
    {
    }
    Date(Date&& other) noexcept
        : dd(other.dd),
          mm(other.mm),
          yy(other.yy)
    {
        other.dd = 0;
        other.mm = 0;
        other.yy = 0;
    }

    inline Date(uint16_t year, uint8_t month, uint8_t day)
    {
        set(year, month, day);
    }

    inline Date(const String& date)
    {
        set(date);
    }

    Date& set(uint16_t year, uint8_t month, uint8_t day);
    Date& set(const String& date);
    String toString() const;
    String format(const String& format) const;

    Date operator=(const Date& other) noexcept
    {
        dd = other.dd;
        mm = other.mm;
        yy = other.yy;
        return *this;
    }

    Date operator=(Date&& other) noexcept
    {
        dd = other.dd;
        mm = other.mm;
        yy = other.yy;
        other.dd = 0;
        other.mm = 0;
        other.yy = 0;
        return *this;
    }

    operator String() const
    {
        return toString();
    }

    inline void clear()
    {
        dd = 0;
        mm = 0;
        yy = 0;
    }

    uint32_t toInt() const
    {
        return (yy * 10000) + (mm * 100) + dd;
    }

    static Date fromInt(uint32_t date)
    {
        Date d;
        d.yy = date / 10000;
        d.mm = (date / 100) % 100;
        d.dd = date % 100;
        return d;
    }

    inline bool isEmpty() const
    {
        return dd == 0;
    }

    inline bool isValid() const
    {
        return !isEmpty();
    }

    inline bool notEmpty() const
    {
        return !isEmpty();
    }

    inline bool operator==(const Date& other) const
    {
        return toInt() == other.toInt();
    }

    inline bool operator!=(const Date& other) const
    {
        return toInt() != other.toInt();
    }

    inline bool operator<(const Date& other) const
    {
        return toInt() < other.toInt();
    }

    inline bool operator<=(const Date& other) const
    {
        return toInt() <= other.toInt();
    }

    inline bool operator>(const Date& other) const
    {
        return toInt() > other.toInt();
    }

    inline bool operator>=(const Date& other) const
    {
        return toInt() >= other.toInt();
    }

    uint8_t day() const noexcept
    {
        return dd;
    }

    uint8_t month() const noexcept
    {
        return mm;
    }

    uint16_t year() const noexcept
    {
        return yy;
    }

    int dayOfYear() const;
    int dayOfWeek() const;
    int dayOfWeekISO8601() const;

    /** @brief Liefert die Kalenderwoche des Datums zurück
     *
     * Diese Funktion berechnet anhand des Datums die Wochennummer innerhalb
     * des Jahres und gibt diese als Integer zurück. Die Zählweise entspricht der in
     * den USA, Australien und vielen weiteren Ländern, in der sich die Tradition des
     * Judentums, Christentums und Islams erhalten hat. Dabei gilt folgende Regel:
     * - Jeden Sonntag beginnt eine neue Kalenderwoche
     * - Am 1. Januar beginnt stets – unabhängig vom Wochentag – die 1. Kalenderwoche
     *
     * @return Integer-Wert mit dem Jahr
     */
    int week() const;
    int weekISO8601() const;
    bool isLeapYear() const;
    static bool isLeapYear(int year);

    static int daysInMonth(int month, int year);

    static Date today();
};

inline std::ostream& operator<<(std::ostream& s, const Date& d)
{
    pplib::String tmp = d.toString();
    return s.write(tmp.c_str(), tmp.length());
}

} // namespace pplib

#endif /* PPLIB_TYPES_DATE_H_ */