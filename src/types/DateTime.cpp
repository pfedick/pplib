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
#include <cctype>
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

DateTime& DateTime::set(const String& datetime)
{
    String parse = UpperCase(Trim(datetime));
    // TODO: Timezone am Ende vorhanden? Würde in Eckigen Klammern stehen, z. B. "[+02:00]" oder "[-02:00]"
    int tz_start = parse.find("[", -1);
    if (tz_start >= 0) {
        int tz_end = parse.find("]", tz_start);
        if (tz_end < 0) {
            throw IllegalArgumentException("DateTime::set: invalid datetime format (%s)", parse.c_str());
        }
        String tz_str = parse.substr(tz_start + 1, tz_end - tz_start - 1);
        my_tz = TimeZone::fromString(tz_str);
        parse = parse.substr(0, tz_start) + parse.substr(tz_end + 1);
    } else if (parse.length() >= 5) {
        // Könnte auch ohne Klammern sein, z. B. "+0200" oder "-0200" am Ende
        String end = parse.right(5);
        if (end[0] == '+' || end[0] == '-') {
            my_tz = TimeZone::fromString(end);
            parse = parse.left(parse.length() - 5);
        } else {
            end = parse.right(1);
            if (end[0] == 'Z') {
                my_tz = TimeZone::utc();
                parse = parse.left(parse.length() - 1);
            } else if (parse.right(3) == "UTC") {
                my_tz = TimeZone::utc();
                parse = parse.left(parse.length() - 3);
            }
        }
    }

    parse.replace(" ", "T");
    if (parse.isEmpty() || parse == "T" || parse == "0" || parse == "NULL") {
        clear();
        return *this;
    }
    // Wie müssen in Datum und Zeit trennen
    Array a(parse, "T");
    if (a.size() == 1) {
        // Kein T gefunden, nur Datum
        my_date.set(a[0]);
        my_time.set(0, 0, 0, 0);
    } else if (a.size() == 2) {
        my_date.set(a[0]);
        my_time.set(a[1]);
    } else {
        throw IllegalArgumentException("DateTime::set: invalid datetime format (%s)", parse.c_str());
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
    pt.hour = my_time.hours();
    pt.min = my_time.minutes();
    pt.sec = my_time.seconds();
    pt.day_of_week = my_date.dayOfWeek();
    pt.day_of_year = my_date.dayOfYear();
    pt.epoch = epoch();
    return pt;
}

DateTime& DateTime::setTime_t(uint64_t time)
{
    if (time == 0) {
        clear();
        return *this;
    }
    struct tm tt;
    if (!safe_localtime((::time_t)time, &tt)) throw InvalidDateException();

    set(tt.tm_year + 1900, tt.tm_mon + 1, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec, 0);
    return *this;
}

DateTime& DateTime::setEpoch(uint64_t time)
{
    return setTime_t(time);
}

uint64_t DateTime::longInt() const
{
    uint64_t r = my_date.year() * 12 + (my_date.month() - 1);
    r = r * 31 + (my_date.day() - 1);
    r = r * 24 + my_time.hours();
    r = r * 60 + my_time.minutes();
    r = r * 60 + my_time.seconds();
    r = r * 1000000 + my_time.microseconds();
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
    int yy = (uint16_t)i / 12;
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
    r.setf("%04i-%02i-%02iT%02i:%02i:%02i", my_date.year(), my_date.month(), my_date.day(), my_time.hours(), my_time.minutes(),
           my_time.seconds());

#if defined(STRUCT_TM_HAS_GMTOFF) || defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__)
    if (my_date.year() >= 1900) {
        struct tm tt;
        tt.tm_sec = my_time.seconds();
        tt.tm_min = my_time.minutes();
        tt.tm_hour = my_time.hours();
        tt.tm_mday = my_date.day();
        tt.tm_mon = my_date.month() - 1;
        tt.tm_year = my_date.year() - 1900;
        tt.tm_isdst = -1;
        mktime(&tt);
        int s = abs(tt.tm_gmtoff / 60);
        if (tt.tm_gmtoff >= 0) {
            r.appendf("+%02i:%02i", (int)(s / 60), tt.tm_gmtoff % 60);
        } else {
            r.appendf("-%02i:%02i", (int)(s / 60), tt.tm_gmtoff % 60);
        }
    }
#endif
    return r;
}

String DateTime::getISO8601withMsec() const
{
    String r;
    r.setf("%04i-%02i-%02iT%02i:%02i:%02i.%03i", my_date.year(), my_date.month(), my_date.day(), my_time.hours(), my_time.minutes(),
           my_time.seconds(), my_time.microseconds() / 1000);
#if defined(STRUCT_TM_HAS_GMTOFF) || defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__)
    if (my_date.year() >= 1900) {
        struct tm tt;
        tt.tm_sec = my_time.seconds();
        tt.tm_min = my_time.minutes();
        tt.tm_hour = my_time.hours();
        tt.tm_mday = my_date.day();
        tt.tm_mon = my_date.month() - 1;
        tt.tm_year = my_date.year() - 1900;
        tt.tm_isdst = -1;
        mktime(&tt);
        int s = abs(tt.tm_gmtoff / 60);
        if (tt.tm_gmtoff >= 0) {
            r.appendf("+%02i:%02i", (int)(s / 60), tt.tm_gmtoff % 60);
        } else {
            r.appendf("-%02i:%02i", (int)(s / 60), tt.tm_gmtoff % 60);
        }
    }
#endif
    return r;
}

String DateTime::getISO8601withUsec() const
{
    String r;
    r.setf("%04i-%02i-%02iT%02i:%02i:%02i.%06i", my_date.year(), my_date.month(), my_date.day(), my_time.hours(), my_time.minutes(),
           my_time.seconds(), my_time.microseconds());

#if defined(STRUCT_TM_HAS_GMTOFF) || defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__)
    if (my_date.year() >= 1900) {
        struct tm tt;
        tt.tm_sec = my_time.seconds();
        tt.tm_min = my_time.minutes();
        tt.tm_hour = my_time.hours();
        tt.tm_mday = my_date.day();
        tt.tm_mon = my_date.month() - 1;
        tt.tm_year = my_date.year() - 1900;
        tt.tm_isdst = -1;
        mktime(&tt);
        int s = abs(tt.tm_gmtoff / 60);
        if (tt.tm_gmtoff >= 0) {
            r.appendf("+%02i:%02i", (int)(s / 60), tt.tm_gmtoff % 60);
        } else {
            r.appendf("-%02i:%02i", (int)(s / 60), tt.tm_gmtoff % 60);
        }
    }
#endif
    return r;
}

String DateTime::getRFC822Date() const
{
    PPLTIME t;
    if (!GetTime(t, time_t())) throw DateOutOfRangeException();
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

String DateTime::strftime(const String& format) const
{
    size_t s = format.size() * 4 + 64;
    if (s < 1024) s = 1024;
    std::vector<char> buf(s);

    struct tm tt;
    ::time_t tp = time_t();
    if (!safe_localtime(tp, &tt)) throw InvalidDateException();

    size_t res = ::strftime(buf.data(), s, (const char*)format, &tt);
    if (res == 0) throw InvalidFormatException();

    return String(buf.data());
}

uint64_t DateTime::epoch() const
{
    if (my_date.year() < 1970) return 0;
    struct tm tt;
    tt.tm_sec = my_time.seconds();
    tt.tm_min = my_time.minutes();
    tt.tm_hour = my_time.hours();
    tt.tm_mday = my_date.day();
    tt.tm_mon = my_date.month() - 1;
    tt.tm_year = my_date.year() - 1900;
    tt.tm_isdst = -1;
    return (uint64_t)mktime(&tt);
}

DateTime DateTime::currentTime()
{
    DateTime dt;
    dt.setCurrentTime();
    return dt;
}

int64_t DateTime::diffSeconds(const DateTime& other) const
{
    int64_t mySecs = (int64_t)time_t();
    int64_t otherSecs = (int64_t)other.time_t();
    return otherSecs - mySecs;
}

bool DateTime::compareSeconds(const DateTime& other, unsigned int tolerance) const
{
    int64_t mySecs = (int64_t)time_t();
    int64_t otherSecs = (int64_t)other.time_t();
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
    return String::format("%04i-%02i-%02i %02i:%02i:%02i.%06i", my_date.year(), my_date.month(), my_date.day(), my_time.hours(),
                          my_time.minutes(), my_time.seconds(), my_time.microseconds());
}

String DateTime::toString(const String& format_string) const
{
    return format(format_string);
}

DateTime::operator String() const
{
    return String::format("%04i-%02i-%02i %02i:%02i:%02i.%06i", my_date.year(), my_date.month(), my_date.day(), my_time.hours(),
                          my_time.minutes(), my_time.seconds(), my_time.microseconds());
}

std::ostream& operator<<(std::ostream& s, const DateTime& dt)
{
    String str = dt.format("%Y-%m-%d %H:%M:%S.%u");
    return s.write((const char*)str, str.size());
}

} // namespace pplib
