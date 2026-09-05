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

#include <chrono>
#include <string.h>
#include <pplib/types/string.h>
#include <pplib/types/datetime.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/array.h>

#include <pplib/core/functions.h>
#include <pplib/core/regex.h>
#include <pplib/exceptions.h>
#include <config_pplib.h>

/*
       The glibc version of struct tm has additional fields
              long tm_gmtoff;           // Seconds east of UTC
              const char *tm_zone;      // Timezone abbreviation

       defined  when _BSD_SOURCE was set before including <time.h>.  This is a
       BSD extension, present in 4.3BSD-Reno.
*/
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

namespace pplib
{

static bool safe_localtime(time_t t, struct tm* tmstruct)
{
#ifdef _WIN32
    return (localtime_s(tmstruct, &t) == 0);
#else
    return (localtime_r(&t, tmstruct) != nullptr);
#endif
}

static bool safe_gmtime(time_t t, struct tm* tmstruct)
{
#ifdef _WIN32
    return (gmtime_s(tmstruct, &t) == 0);
#else
    return (gmtime_r(&t, tmstruct) != nullptr);
#endif
}

TimeZone parse_time(String& parse)
{
    TimeZone tz = TimeZone::utc();
    int tz_start = parse.find("[", -1);
    if (tz_start >= 0) {
        int tz_end = parse.find("]", tz_start);
        if (tz_end < 0) {
            throw IllegalArgumentException("DateTime::set: invalid datetime format (%s)", parse.c_str());
        }
        String tz_str = parse.substr(tz_start + 1, tz_end - tz_start - 1);
        parse = parse.substr(0, tz_start) + parse.substr(tz_end + 1);
        return TimeZone::fromString(tz_str);

    } else if (parse.length() >= 6) {
        // printf("parse=>>%s<<\n", (const char*)parse);
        tz_start = parse.find("+", -1);
        if (tz_start != String::npos) {
            tz = TimeZone::fromString(parse.substr(tz_start));
            parse = parse.substr(0, tz_start);
            // printf("rest parse: >>%s<<\n", (const char*)parse);
            return tz;
        }
        tz_start = parse.find("-", -1);
        if (tz_start != String::npos) {
            tz = TimeZone::fromString(parse.substr(tz_start));
            parse = parse.substr(0, tz_start);
            // printf("rest parse: >>%s<<\n", (const char*)parse);
            return tz;
        }
        String end = parse.right(1);
        if (end[0] == 'Z') {
            parse = parse.left(parse.length() - 1);
        } else if (parse.right(3) == "UTC") {
            parse = parse.left(parse.length() - 3);
        }
        // printf("rest parse: >>%s<<\n", (const char*)parse);
    }
    return TimeZone::utc();
}

DateTime& DateTime::set(const String& datetime)
{
    String parse = UpperCase(Trim(datetime));
    parse.replace(" ", "T");
    if (parse.isEmpty() || parse == "T" || parse == "0" || parse == "NULL") {
        clear();
        return *this;
    }
    // Wie müssen in Datum und Zeit trennen
    Array a;
    size_t pt = parse.find("T");
    if (pt == String::npos) {
        // Kein T gefunden, nur Datum
        my_date.set(parse);
        my_time.set(0, 0, 0, 0);
        my_tz = TimeZone::utc(); // Default auf UTC, falls keine Zeitzone angegeben ist

    } else {
        my_date.set(parse.substr(0, pt));
        parse = parse.substr(pt + 1);
        my_tz = parse_time(parse);
        my_time.set(parse);
    }
    return *this;
}

DateTime& DateTime::set(const PPLTIME& pt)
{
    return set(pt.year, pt.month, pt.day, pt.hour, pt.min, pt.sec, 0);
}

PPLTIME DateTime::toPPLTIME() const
{
    PPLTIME pt;
    memset(&pt, 0, sizeof(PPLTIME));
    pt.year = my_date.year();
    pt.month = my_date.month();
    pt.day = my_date.day();
    pt.hour = my_time.hour();
    pt.min = my_time.minute();
    pt.sec = my_time.second();
    pt.day_of_week = my_date.dayOfWeek();
    pt.day_of_year = my_date.dayOfYear();
    pt.epoch = epoch();
    pt.have_gmt_offset = true;
    pt.gmt_offset = my_tz.offsetMinutes();
    return pt;
}

uint64_t DateTime::epoch() const
{
    if (my_date.year() < 1970) return 0;
    // Wir berechnen die Sekunden von 1970 bis zum gepspeicherten Zeitstempel
    uint64_t total_days = 0;

    // 1. Tage für alle vollen Jahre seit 1970 berechnen
    for (int y = 1970; y < my_date.year(); ++y) {
        // Nutze die isLeapYear Logik aus der Date-Klasse
        total_days += (Date::isLeapYear(y) ? 366 : 365);
    }
    // 2. Tage für die Monate im aktuellen Jahr berechnen
    static const int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (int m = 1; m < my_date.month(); ++m) {
        total_days += days_in_month[m];
        // Schaltjahr-Check für Februar
        if (m == 2 && my_date.isLeapYear(my_date.year())) {
            total_days += 1;
        }
    }
    // 3. Tage des aktuellen Monats addieren (minus 1, da wir am 1. starten)
    total_days += (my_date.day() - 1);
    return total_days * 86400 + my_time.toSeconds() - my_tz.offsetSeconds();
}

DateTime& DateTime::setEpoch(uint64_t time)
{
    my_tz = TimeZone::utc();            // Zeitzone setzen wir auf UTC
    uint64_t total_days = time / 86400; // Ein Tag hat 86400 Sekunden
    // Uhrzeit können wir relativ leicht anhand des Modulos setzen
    my_time.setFromSeconds(time % 86400);
    // 1. Das passende Jahr finden
    int year = 1970;
    while (true) {
        uint32_t days_for_year = (Date::isLeapYear(year) ? 366 : 365); // Tage für das Jahr
        if (total_days < days_for_year) break;
        total_days -= days_for_year;
        year++;
    }
    // 2. Den Monat finden
    static const int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int month = 1;
    while (month <= 12) {
        uint32_t days_for_month = days_in_month[month];
        // Schaltjahr-Check für Februar
        if (month == 2 && my_date.isLeapYear(year)) {
            days_for_month += 1;
        }
        if (total_days < days_for_month) break;
        total_days -= days_for_month;
        month++;
    }
    my_date.set(year, month, total_days + 1);
    return *this;
}

uint64_t DateTime::longInt() const
{
    uint64_t r = my_date.year() * 12 + (my_date.month() - 1);
    r = r * 31 + (my_date.day() - 1);
    r = r * 24 + my_time.hour();
    r = r * 60 + my_time.minute();
    r = r * 60 + my_time.second();
    r = r * 1000000 + my_time.microsecond();
    return r;
}

DateTime& DateTime::setLongInt(uint64_t i)
{
    int us = i % 1000000;
    i = i / 1000000;
    int ss = i % 60;
    i = i / 60;
    int ii = i % 60;
    i = i / 60;
    int hh = i % 24;
    i = i / 24;
    int dd = (i % 31) + 1;
    i = i / 31;
    int mm = (i % 12) + 1;
    int yy = i / 12;
    set(yy, mm, dd, hh, ii, ss, us);
    return *this;
}

DateTime& DateTime::setCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();

    // Mikrosekunden-Anteil berechnen
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    int us = static_cast<uint32_t>(micros % 1000000);

    // Sekunden seit Epoche für localtime
    ::time_t tp = std::chrono::system_clock::to_time_t(now);
    struct tm tt;
    my_tz = TimeZone::fromEpoch(tp);
    if (!safe_localtime(tp, &tt)) throw InvalidDateException();

    set(tt.tm_year + 1900, tt.tm_mon + 1, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec, us);
    return *this;
}

String DateTime::getISO8601() const
{
    String r;
    r.setf("%04i-%02i-%02iT%02i:%02i:%02i", my_date.year(), my_date.month(), my_date.day(), my_time.hour(), my_time.minute(),
           my_time.second());
    int s = abs(my_tz.offsetMinutes());
    if (my_tz.offsetMinutes() >= 0) {
        r.appendf("+%02i:%02i", (int)(s / 60), s % 60);
    } else {
        r.appendf("-%02i:%02i", (int)(s / 60), s % 60);
    }
    return r;
}

String DateTime::getISO8601withMsec() const
{
    String r;
    r.setf("%04i-%02i-%02iT%02i:%02i:%02i.%03i", my_date.year(), my_date.month(), my_date.day(), my_time.hour(), my_time.minute(),
           my_time.second(), my_time.microsecond() / 1000);
    int s = abs(my_tz.offsetMinutes());
    if (my_tz.offsetMinutes() >= 0) {
        r.appendf("+%02i:%02i", (int)(s / 60), s % 60);
    } else {
        r.appendf("-%02i:%02i", (int)(s / 60), s % 60);
    }
    return r;
}

String DateTime::getISO8601withUsec() const
{
    String r;
    r.setf("%04i-%02i-%02iT%02i:%02i:%02i.%06i", my_date.year(), my_date.month(), my_date.day(), my_time.hour(), my_time.minute(),
           my_time.second(), my_time.microsecond());
    int s = abs(my_tz.offsetMinutes());
    if (my_tz.offsetMinutes() >= 0) {
        r.appendf("+%02i:%02i", (int)(s / 60), s % 60);
    } else {
        r.appendf("-%02i:%02i", (int)(s / 60), s % 60);
    }
    return r;
}

String DateTime::getRFC822Date() const
{
    if (this->isEmpty()) throw IllegalStateException("DateTime is invalid");

    PPLTIME t = toPPLTIME();
    // if (!GetTime(t, time_t())) throw DateOutOfRangeException();
    String s;
    const char* day[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char* month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    // PPLTIME prüfen
    if (t.day_of_week < 0 || t.day_of_week > 6) throw IllegalArgumentException("DateTime::getRFC822Date: week<0 order week>6");
    if (t.month < 1 || t.month > 12) throw IllegalArgumentException("DateTime::getRFC822Date: month<0 order month>12");

    s = day[t.day_of_week];
    s += ", ";
    s.appendf("%02i ", t.day);
    s += month[t.month - 1];
    s.appendf(" %04i %02i:%02i:%02i", t.year, t.hour, t.min, t.sec);
    if (t.have_gmt_offset) {
        if (t.gmt_offset >= 0)
            s.appendf(" +%02i%02i", abs(t.gmt_offset / 3600), abs(t.gmt_offset % 3600));
        else
            s.appendf(" -%02i%02i", abs(t.gmt_offset / 3600), abs(t.gmt_offset % 3600));
    }
    return s;
}

String DateTime::get(const String& format) const
{
    String fmt = format;
    String tz = my_tz.toString();
    fmt.replace("%z", tz);
    return my_date.format(my_time.format(fmt));
}

String DateTime::strftime(const String& format) const
{
    size_t s = format.size() * 4 + 64;
    if (s < 1024) s = 1024;
    std::vector<char> buf(s);

    struct tm tt;
    ::time_t tp = time_t();
    // Zeitzone fixen
    tp += my_tz.offsetSeconds();

    if (!safe_gmtime(tp, &tt)) throw InvalidDateException();

    size_t res = ::strftime(buf.data(), s, (const char*)format, &tt);
    if (res == 0) throw InvalidFormatException();

    return String(buf.data());
}

DateTime DateTime::currentTime()
{
    DateTime dt;
    dt.setCurrentTime();
    return dt;
}

int64_t DateTime::diffSeconds(const DateTime& other) const
{
    int64_t mySecs = (int64_t)toMicroseconds() / 1000000;
    int64_t otherSecs = (int64_t)other.toMicroseconds() / 1000000;
    return otherSecs - mySecs;
}

bool DateTime::compareSeconds(const DateTime& other, unsigned int tolerance) const
{
    int64_t mySecs = (int64_t)toMicroseconds() / 1000000;
    int64_t otherSecs = (int64_t)other.toMicroseconds() / 1000000;
    int64_t diff = otherSecs - mySecs;
    if (diff < 0) diff = mySecs - otherSecs;
    if (diff <= tolerance) return true;
    return false;
}

DateTime& DateTime::operator=(const String& datetime)
{
    set(datetime);
    return *this;
}

String DateTime::toString() const
{
    return String::format("%04i-%02i-%02i %02i:%02i:%02i.%06i", my_date.year(), my_date.month(), my_date.day(), my_time.hour(),
                          my_time.minute(), my_time.second(), my_time.microsecond());
}

String DateTime::toString(const String& format_string) const
{
    return format(format_string);
}

DateTime::operator String() const
{
    return String::format("%04i-%02i-%02i %02i:%02i:%02i.%06i", my_date.year(), my_date.month(), my_date.day(), my_time.hour(),
                          my_time.minute(), my_time.second(), my_time.microsecond());
}

std::ostream& operator<<(std::ostream& s, const DateTime& dt)
{
    String str = dt.format("%Y-%m-%d %H:%M:%S.%u");
    return s.write((const char*)str, str.size());
}

// Gibt die Tage seit 0000-03-01 zurück (funktioniert für alle Jahre)
static constexpr int64_t daysFromCivil(int y, unsigned m, unsigned d) noexcept
{
    y -= m <= 2;
    const int64_t era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);            // [0, 399]
    const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1; // [0, 365]
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;           // [0, 146096]
    return era * 146097 + static_cast<int64_t>(doe) - 719468;             // 0 = 1970-01-01
}

static void civilFromDays(int64_t z, int& y, int& m, int& d) noexcept
{
    z += 719468;
    const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);              // [0, 146096]
    const unsigned yoe = (doe - doe / 1024 + doe / 1461 - doe / 142400) / 365; // [0, 399]
    const int64_t y_temp = static_cast<int64_t>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100); // [0, 365]
    const unsigned mp = (5 * doy + 2) / 153;                      // [0, 11]
    d = doy - (153 * mp + 2) / 5 + 1;                             // [1, 31]
    m = mp < 10 ? mp + 3 : mp - 9;                                // [1, 12]
    y = y_temp + (m <= 2);
}

int64_t DateTime::toMicroseconds() const
{
    if (isEmpty()) throw IllegalStateException("DateTime is invalid");

    // 1. Tage seit 1970-01-01 (kann für Daten vor 1970 auch negativ sein)
    int64_t days = daysFromCivil(my_date.year(), my_date.month(), my_date.day());

    // 2. Zeitanteil am Tag in Mikrosekunden
    int64_t time_us = static_cast<int64_t>(my_time.toMicroseconds());

    // 3. Zeitzonen-Offset in Mikrosekunden abziehen (um auf UTC zu normieren)
    int64_t tz_offset_us = static_cast<int64_t>(my_tz.offsetSeconds()) * 1000000LL;

    return (days * 86400000000LL) + time_us - tz_offset_us;
}

DateTime& DateTime::setMicroseconds(int64_t epoch_microseconds, const TimeZone& tz)
{
    my_tz = tz;
    // Lokale Mikrosekunden berechnen: UTC + TimeZone-Offset
    int64_t local_us = epoch_microseconds + (static_cast<int64_t>(tz.offsetSeconds()) * 1000000LL);

    constexpr int64_t US_PER_DAY = 86400000000LL;
    int64_t days = local_us / US_PER_DAY;
    int64_t rem_us = local_us % US_PER_DAY;

    if (rem_us < 0) {
        rem_us += US_PER_DAY;
        days -= 1;
    }

    int y, m, d;
    civilFromDays(days, y, m, d);
    my_date.set(y, m, d);
    my_time.setFromMicroseconds(static_cast<uint64_t>(rem_us));

    return *this;
}

int DateTime::compare(const DateTime& other) const noexcept
{
    if (my_date.isEmpty() && other.my_date.isEmpty()) return 0;
    if (my_date.isEmpty()) return -1;
    if (other.my_date.isEmpty()) return 1;
    const int64_t this_us = toMicroseconds();
    const int64_t other_us = other.toMicroseconds();
    if (this_us < other_us) return -1;
    if (this_us > other_us) return 1;
    return 0;
}

} // namespace pplib
