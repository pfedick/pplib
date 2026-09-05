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
#include "pplib/types/date.h"
#include "pplib/types/string.h"
#include "pplib/types/array.h"
#include "pplib/exceptions.h"
#include "pplib/core/functions.h"
#include <chrono>

namespace pplib
{

Date& Date::set(int year, int month, int day)
{
    if (year < 0 || year > 9999 || month < 1 || month > 12 || day < 1 || day > 31) {
        throw IllegalArgumentException("Date::set: invalid date (%04d-%02d-%02d)", year, month, day);
    }
    if (day > daysInMonth(month, year)) {
        throw IllegalArgumentException("Date::set: invalid date (%04d-%02d-%02d)", year, month, day);
    }
    yy = (uint16_t)year;
    mm = (uint8_t)month;
    dd = (uint8_t)day;
    return *this;
}

Date& Date::set(const String& date)
{
    String d = Trim(date);
    d.replace("-", ".");
    Array a(d, ".");
    if (a.size() != 3) {
        throw IllegalArgumentException("Date::set: invalid date format (%s)", d.c_str());
    }
    // Prüfen, ob alle Werte Numerisch sind
    for (size_t i = 0; i < a.size(); ++i) {
        if (!a[i].isNumeric()) {
            throw IllegalArgumentException("Date::set: invalid date format (%s)", d.c_str());
        }
    }
    if (a[0].length() == 4) {
        // YYYY.MM.DD
        return set(a[0].toInt(), a[1].toInt(), a[2].toInt());
    } else if (a[2].length() == 4) {
        // DD.MM.YYYY
        return set(a[2].toInt(), a[1].toInt(), a[0].toInt());
    } else {
        throw IllegalArgumentException("Date::set: invalid date format (%s)", d.c_str());
    }
}

String Date::toString() const
{
    return String::format("%04u-%02u-%02u", yy, mm, dd);
}

String Date::format(const String& format) const
{
    String f = format;
    f.replace("%Y", String::format("%04u", yy));
    f.replace("%y", String::format("%02u", yy % 100));
    f.replace("%m", String::format("%02u", mm));
    f.replace("%d", String::format("%02u", dd));
    return f;
}

bool Date::isLeapYear(int year)
{
    if (year % 400 == 0) return true;
    if (year % 100 == 0) return false;
    if (year % 4 != 0) return false;
    return true;
}

int Date::daysInMonth(int month, int year)
{
    switch (month) {
    case 1:
        return 31;
    case 2:
        return isLeapYear(year) ? 29 : 28;
    case 3:
        return 31;
    case 4:
        return 30;
    case 5:
        return 31;
    case 6:
        return 30;
    case 7:
        return 31;
    case 8:
        return 31;
    case 9:
        return 30;
    case 10:
        return 31;
    case 11:
        return 30;
    case 12:
        return 31;
    default:
        throw IllegalArgumentException("Date::daysInMonth(): invalid month (%d)", month);
    }
}

int Date::dayOfYear() const
{
    if (isEmpty()) {
        throw IllegalStateException("Date::dayOfYear() called on empty Date object");
    }
    int days = 0;
    for (int m = 1; m < mm; ++m) {
        days += daysInMonth(m, yy);
    }
    days += dd;
    return days;
}

int Date::dayOfWeek() const
{
    if (isEmpty()) {
        throw IllegalStateException("Date::dayOfWeek() called on empty Date object");
    }
    // Sakamoto's algorithm
    static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    // Der Gregorianische Kalender wiederholt sich exakt alle 400 Jahre (146097 Tage = genau
    // 20871 Wochen). Der Shift um einen vollen Zyklus laesst den Wochentag also unveraendert,
    // verhindert aber, dass y fuer Jahr 0 negativ wird - dort wuerde die zur Null hin
    // trunkierende Division von C++ (statt der benoetigten Floor-Division) falsch rechnen.
    int y = yy + 400;
    if (mm < 3) {
        y -= 1;
    }
    return (y + y / 4 - y / 100 + y / 400 + t[mm - 1] + dd) % 7;
}

int Date::dayOfWeekISO8601() const
{
    int dow = dayOfWeek();
    return dow == 0 ? 7 : dow; // 1 = Monday, ..., 7 = Sunday
}

int Date::week() const
{
    if (isEmpty()) {
        throw IllegalStateException("Date::week() called on empty Date object");
    }
    // Wochennummer nach der Zaehlweise von strftime "%U": jeder Sonntag beginnt eine neue
    // Woche, alle Tage vor dem ersten Sonntag des Jahres liegen in Woche 0.
    int wday = dayOfWeek();    // 0 = Sonntag
    int doy = dayOfYear() - 1; // 0-basiert
    return (doy - wday + 7) / 7;
}

int Date::weekISO8601() const
{
    if (isEmpty()) {
        throw IllegalStateException("Date::weekISO8601() called on empty Date object");
    }
    // ISO-Wochentag: 1 = Montag, ..., 7 = Sonntag
    int iso_dow = dayOfWeekISO8601();

    // Finde den Donnerstag dieser Woche
    // Ein Tag-Offset reicht: Donnerstag ist Tag 4
    int approx_doy = dayOfYear();
    int thu_doy = approx_doy + 4 - iso_dow;

    // Fällt der Donnerstag ins Vorjahr?
    if (thu_doy < 1) {
        // Gehört zur letzten Woche des Vorjahres (Woche 52 oder 53). Das Vorjahr wird bewusst
        // nicht als Date-Objekt konstruiert, da es für yy==0 das nicht darstellbare Jahr -1 wäre.
        // Ein Jahr hat genau dann 53 ISO-Wochen, wenn sein 31.12. ein Donnerstag ist, oder ein
        // Freitag und das Jahr ein Schaltjahr war.
        int prev_year = (int)yy - 1;
        // ISO-Wochentag (0 = Montag) des 1. Januars dieses Jahres ...
        int jan1_iso = ((iso_dow - 1 - (approx_doy - 1)) % 7 + 7) % 7;
        // ... und daraus der Tag davor, also der 31.12. des Vorjahres (1 = Montag ... 7 = Sonntag)
        int dec31_iso = ((jan1_iso - 1) % 7 + 7) % 7 + 1;
        if (dec31_iso == 4 || (dec31_iso == 5 && isLeapYear(prev_year))) return 53;
        return 52;
    }

    // Fällt der Donnerstag ins nächste Jahr?
    int days_in_cur_year = isLeapYear(yy) ? 366 : 365;
    if (thu_doy > days_in_cur_year) {
        return 1; // Erste Woche des Folgejahres
    }

    // Woche = Donnerstag-Tag-des-Jahres / 7 + 1
    return (thu_doy - 1) / 7 + 1;
}

bool Date::isLeapYear() const
{
    if (isEmpty()) {
        throw IllegalStateException("Date::isLeapYear() called on empty Date object");
    }
    return isLeapYear(yy);
}

Date Date::today()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);
    return Date(now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);
}

} // namespace pplib
