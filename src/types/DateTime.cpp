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

DateTime::DateTime()
{
    clear();
}

DateTime::DateTime(const String& datetime)
{
    set(datetime);
}

DateTime::DateTime(const DateTime& other)
{
    yy = other.yy;
    us = other.us;
    mm = other.mm;
    dd = other.dd;
    hh = other.hh;
    ii = other.ii;
    ss = other.ss;
}

DateTime::DateTime(DateTime&& other)
{
    yy = other.yy;
    us = other.us;
    mm = other.mm;
    dd = other.dd;
    hh = other.hh;
    ii = other.ii;
    ss = other.ss;
    other.clear();
}

DateTime::DateTime(uint64_t t)
{
    setTime_t(t);
}

void DateTime::clear()
{
    yy = 0;
    us = 0;
    mm = 0;
    dd = 0;
    hh = 0;
    ii = 0;
    ss = 0;
}

// Hilfsfunktion zum einfachen Konvertieren von Ziffern in Integer
static int parse_int(const char* p, size_t len)
{
    int val = 0;
    for (size_t i = 0; i < len; ++i) {
        if (p[i] >= '0' && p[i] <= '9')
            val = val * 10 + (p[i] - '0');
        else
            throw IllegalArgumentException("DateTime::set(" + String(p, len) + ")");
    }
    return val;
}

DateTime& DateTime::set(const String& datetime)
{
    String d = UpperCase(Trim(datetime));
    d.replace(",", ".");

    if (d.isEmpty() || d == "T" || d == "0" || d == "NULL") {
        clear();
        return *this;
    }

    const char* str = d.c_str();

    // Datum und Zeit trennen ('T' oder Leerzeichen)
    const char* sep = strpbrk(str, " T");
    const char* time_part = sep ? sep + 1 : nullptr;

    int y = 0, m = 0, day = 0;

    // 1. Datum parsen
    const char* d1 = strpbrk(str, ".-");
    if (!d1 || (sep && d1 > sep)) {
        clear();
        throw IllegalArgumentException("DateTime::set(" + datetime + ")");
    }

    size_t first_len = d1 - str;
    const char* d2 = strpbrk(d1 + 1, ".-");

    if (first_len == 4) {
        // YYYY-MM-DD
        y = parse_int(str, 4);
        m = parse_int(d1 + 1, d2 ? (d2 - d1 - 1) : 2);
        day = parse_int(d2 ? d2 + 1 : d1 + 1, sep ? (sep - (d2 ? d2 + 1 : d1 + 1)) : 2);
    } else if (first_len == 1 || first_len == 2) {
        // DD.MM.YYYY
        day = parse_int(str, first_len);
        if (!d2) {
            clear();
            throw IllegalArgumentException("DateTime::set(" + datetime + ")");
        }
        m = parse_int(d1 + 1, d2 - d1 - 1);
        y = parse_int(d2 + 1, sep ? (sep - d2 - 1) : 4);
    } else {
        clear();
        throw IllegalArgumentException("DateTime::set(" + datetime + ")");
    }

    if (y == 0 && m == 0 && day == 0) {
        clear();
        throw IllegalArgumentException("DateTime::set(" + datetime + ")");
    }

    // 2. Uhrzeit parsen (falls vorhanden)
    int h = 0, min = 0, s = 0, msec = 0, usec = 0;
    if (time_part && *time_part != '\0') {
        int consumed = 0;
        if (sscanf(time_part, "%d:%d:%d%n", &h, &min, &s, &consumed) >= 3) {
            const char* frac_ptr = time_part + consumed;
            if (*frac_ptr == '.' || *frac_ptr == ':') {
                frac_ptr++;
                size_t frac_digits = 0;
                while (isdigit(frac_ptr[frac_digits]))
                    frac_digits++;

                if (frac_digits >= 1 && frac_digits <= 3) {
                    msec = parse_int(frac_ptr, frac_digits);
                } else if (frac_digits == 6) {
                    msec = parse_int(frac_ptr, 3);
                    usec = parse_int(frac_ptr + 3, 3);
                } else if (frac_digits > 0) {
                    clear();
                    throw IllegalArgumentException("DateTime::set(" + datetime + ")");
                }
            }
        }
    }

    return set(y, m, day, h, min, s, msec, usec);
}

#ifdef OLD_REGEX_CODE
DateTime& DateTime::set(const String& datetime)
{
    return setWithoutRegex(datetime);
    String d = UpperCase(Trim(datetime));
    std::vector<String> m;
    d.replace(",", " ");
    if (d.isEmpty() == true || d == "T" || d == "0" || d == "NULL") {
        clear();
        return *this;
    }
    if (RegEx::capture("/^([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})T([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})[\\.:]([0-9]{3})([0-9]{3})/", d, m)) {
        // yyyy-mm-ddThh:ii:ss.msecusec[[+-]oo:00]
        return set(m.at(1).toInt(), m.at(2).toInt(), m.at(3).toInt(), m.at(4).toInt(), m.at(5).toInt(), m.at(6).toInt(), m.at(7).toInt(),
                   m.at(8).toInt());
    } else if (RegEx::capture("/^([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})T([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})[\\.:]([0-9]{1,3})/", d, m)) {
        // yyyy-mm-ddThh:ii:ss.msec[[+-]oo:00]
        return set(m.at(1).toInt(), m.at(2).toInt(), m.at(3).toInt(), m.at(4).toInt(), m.at(5).toInt(), m.at(6).toInt(), m.at(7).toInt());

    } else if (RegEx::capture("/^([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})T([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})/", d, m)) {
        // yyyy-mm-ddThh:ii:ss[[+-]oo:00]
        return set(m.at(1).toInt(), m.at(2).toInt(), m.at(3).toInt(), m.at(4).toInt(), m.at(5).toInt(), m.at(6).toInt());
    } else if (RegEx::capture(
                   "/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})\\s+([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})[\\.:]([0-9]{3})([0-9]{3})$/",
                   d, m)) {
        // yyyy.mm.dd hh:ii:ss.msecusec
        return set(m.at(1).toInt(), m.at(2).toInt(), m.at(3).toInt(), m.at(4).toInt(), m.at(5).toInt(), m.at(6).toInt(), m.at(7).toInt(),
                   m.at(8).toInt());
    } else if (RegEx::capture(
                   "/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})\\s+([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})[\\.:]([0-9]{1,3})$/", d,
                   m)) {
        // yyyy.mm.dd hh:ii:ss.msec
        return set(m.at(1).toInt(), m.at(2).toInt(), m.at(3).toInt(), m.at(4).toInt(), m.at(5).toInt(), m.at(6).toInt(), m.at(7).toInt());
    } else if (RegEx::capture(
                   "/^([0-9]{1,2})[\\.-]([0-9]{1,2})[\\.-]([0-9]{4})\\s+([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})[\\.:]([0-9]{3})([0-9]{3})$/",
                   d, m)) {
        // dd.mm.yyyy hh:ii:ss.msecusec
        return set(m.at(3).toInt(), m.at(2).toInt(), m.at(1).toInt(), m.at(4).toInt(), m.at(5).toInt(), m.at(6).toInt(), m.at(7).toInt(),
                   m.at(8).toInt());
    } else if (RegEx::capture(
                   "/^([0-9]{1,2})[\\.-]([0-9]{1,2})[\\.-]([0-9]{4})\\s+([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})[\\.:]([0-9]{1,3})$/", d,
                   m)) {
        // dd.mm.yyyy hh:ii:ss.msec
        return set(m.at(3).toInt(), m.at(2).toInt(), m.at(1).toInt(), m.at(4).toInt(), m.at(5).toInt(), m.at(6).toInt(), m.at(7).toInt());
    } else if (RegEx::capture("/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})\\s+([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})$/", d, m)) {
        // yyyy.mm.dd hh:ii:ss
        return set(m.at(1).toInt(), m.at(2).toInt(), m.at(3).toInt(), m.at(4).toInt(), m.at(5).toInt(), m.at(6).toInt());
    } else if (RegEx::capture("/^([0-9]{1,2})[\\.-]([0-9]{1,2})[\\.-]([0-9]{4})\\s+([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})$/", d, m)) {
        // dd.mm.yyyy hh:ii:ss
        return set(m.at(3).toInt(), m.at(2).toInt(), m.at(1).toInt(), m.at(4).toInt(), m.at(5).toInt(), m.at(6).toInt());
    } else if (RegEx::capture("/^([0-9]{1,2})[\\.-]([0-9]{1,2})[\\.-]([0-9]{4})$/", d, m)) {
        // dd.mm.yyyy
        return set(m.at(3).toInt(), m.at(2).toInt(), m.at(1).toInt());
    } else if (RegEx::capture("/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})$/", d, m)) {
        // yyyy.mm.dd
        return set(m.at(1).toInt(), m.at(2).toInt(), m.at(3).toInt());
    } else {
        clear();
        throw IllegalArgumentException("DateTime::set(" + datetime + ")");
    }
    return *this;
}
#endif

DateTime& DateTime::set(const DateTime& other)
{
    yy = other.yy;
    us = other.us;
    mm = other.mm;
    dd = other.dd;
    hh = other.hh;
    ii = other.ii;
    ss = other.ss;
    return *this;
}

DateTime& DateTime::set(const String& date, const String& time)
{
    String d, dd = Trim(date), tt = Trim(time);
    dd.replace(",", ".");
    dd.replace(":", ".");
    tt.replace(",", ":");
    tt.replace(".", ":");
    tt.replace("-", ":");

    d = dd + " " + tt;
    return set(d);
}

DateTime& DateTime::setDate(const String& date)
{
    String time = getTime();
    return set(date, time);
}

DateTime& DateTime::setTime(const String& time)
{
    String date = getDate();
    return set(date, time);
}

DateTime& DateTime::set(int year, int month, int day, int hour, int minute, int sec, int msec, int usec)
{

    yy = year;
    if (year < 0) yy = 0;
    if (year > 9999) yy = 9999;
    mm = month;
    if (month < 1) mm = 1;
    if (month > 12) mm = 12;
    dd = day;
    if (day < 1) dd = 1;
    if (day > 31) dd = 31;
    hh = hour;
    if (hour < 0) hh = 0;
    if (hour > 23) hh = 23;
    ii = minute;
    if (minute < 0) ii = 0;
    if (minute > 59) ii = 59;
    ss = sec;
    if (sec < 0) ss = 0;
    if (sec > 59) ss = 59;
    if (msec < 0) msec = 0;
    if (msec > 999) msec = 999;
    if (usec < 0) usec = 0;
    if (usec > 999999) usec = 999999;
    us = msec * 1000 + usec;
    return *this;
}

DateTime& DateTime::set(const PPLTIME& t)
{
    return set(t.year, t.month, t.day, t.hour, t.min, t.sec, 0, 0);
}

DateTime& DateTime::setTime_t(uint64_t t)
{
    if (t == 0) {
        clear();
        return *this;
    }
    struct tm tt;
    ::time_t tp = (::time_t)t;

    if (!safe_localtime(tp, &tt)) throw InvalidDateException();
    ss = tt.tm_sec;
    ii = tt.tm_min;
    hh = tt.tm_hour;
    dd = tt.tm_mday;
    mm = tt.tm_mon + 1;
    yy = tt.tm_year + 1900;
    us = 0;
    return *this;
}

DateTime& DateTime::setEpoch(uint64_t t)
{
    return setTime_t(t);
}

DateTime& DateTime::setLongInt(uint64_t i)
{
    us = i % 1000000;
    i = i / 1000000;
    ss = i % 60;
    i = i / 60;
    ii = i % 60;
    i = i / 60;
    hh = i % 24;
    i = i / 24;
    dd = (i % 31) + 1;
    i = i / 31;
    mm = (i % 12) + 1;
    yy = (uint16_t)i / 12;
    return *this;
}

DateTime& DateTime::setCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();

    // Mikrosekunden-Anteil berechnen
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    us = static_cast<uint32_t>(micros % 1000000);

    // Sekunden seit Epoche für localtime
    ::time_t tp = std::chrono::system_clock::to_time_t(now);
    struct tm tt;
    if (!safe_localtime(tp, &tt)) throw InvalidDateException();

    ss = tt.tm_sec;
    ii = tt.tm_min;
    hh = tt.tm_hour;
    dd = tt.tm_mday;
    mm = tt.tm_mon + 1;
    yy = tt.tm_year + 1900;
    return *this;
}

String DateTime::get(const String& format) const
{
    String Tmp;
    String r = format;
    Tmp.setf("%03i", us / 1000);
    r.replace("%*", Tmp);
    Tmp.setf("%06i", us);
    r.replace("%u", Tmp);

    if (yy < 1900) {
        Tmp.setf("%04i", yy);
        r.replace("%Y", Tmp);
        Tmp.setf("%02i", yy % 100);
        r.replace("%y", Tmp);

        Tmp.setf("%02i", mm);
        r.replace("%m", Tmp);

        Tmp.setf("%02i", dd);
        r.replace("%d", Tmp);

        Tmp.setf("%02i", hh);
        r.replace("%H", Tmp);

        Tmp.setf("%02i", ii);
        r.replace("%M", Tmp);

        Tmp.setf("%02i", ss);
        r.replace("%S", Tmp);

        return r;
    }

    struct tm t;
    t.tm_sec = ss;
    t.tm_min = ii;
    t.tm_hour = hh;
    t.tm_mday = dd;
    t.tm_mon = mm - 1;
    t.tm_year = yy - 1900;
    t.tm_isdst = -1;
    mktime(&t);

    size_t size = r.len() * 2 + 32;
    char* b = (char*)malloc(size);
    if (!b) {
        throw OutOfMemoryException();
    }
    if (::strftime(b, size, (const char*)r, &t) == 0) {
        free(b);
        throw IllegalArgumentException("DateTime::get(\"%s\")", (const char*)r);
    }
    r.set(b);
    free(b);
    return r;
}

String DateTime::getDate(const String& format) const
{
    return get(format);
}

String DateTime::getTime(const String& format) const
{
    return get(format);
}

String DateTime::getISO8601() const
{
    String r;
    r.setf("%04i-%02i-%02iT%02i:%02i:%02i", yy, mm, dd, hh, ii, ss);

#if defined(STRUCT_TM_HAS_GMTOFF) || defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__)
    if (yy >= 1900) {
        struct tm t;
        t.tm_sec = ss;
        t.tm_min = ii;
        t.tm_hour = hh;
        t.tm_mday = dd;
        t.tm_mon = mm - 1;
        t.tm_year = yy - 1900;
        t.tm_isdst = -1;
        mktime(&t);

        int s = abs(t.tm_gmtoff / 60);
        if (t.tm_gmtoff >= 0) {
            r.appendf("+%02i:%02i", (int)(s / 60), t.tm_gmtoff % 60);
        } else {
            r.appendf("-%02i:%02i", (int)(s / 60), t.tm_gmtoff % 60);
        }
    }
#endif
    return r;
}

String DateTime::getISO8601withMsec() const
{
    String r;
    r.setf("%04i-%02i-%02iT%02i:%02i:%02i.%03i", yy, mm, dd, hh, ii, ss, us / 1000);

#if defined(STRUCT_TM_HAS_GMTOFF) || defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__)
    if (yy >= 1900) {
        struct tm t;
        t.tm_sec = ss;
        t.tm_min = ii;
        t.tm_hour = hh;
        t.tm_mday = dd;
        t.tm_mon = mm - 1;
        t.tm_year = yy - 1900;
        t.tm_isdst = -1;
        mktime(&t);

        int s = abs(t.tm_gmtoff / 60);
        if (t.tm_gmtoff >= 0) {
            r.appendf("+%02i:%02i", (int)(s / 60), t.tm_gmtoff % 60);
        } else {
            r.appendf("-%02i:%02i", (int)(s / 60), t.tm_gmtoff % 60);
        }
    }
#endif
    return r;
}

String DateTime::getISO8601withUsec() const
{
    String r;
    r.setf("%04i-%02i-%02iT%02i:%02i:%02i.%03i", yy, mm, dd, hh, ii, ss, us / 1000);

#if defined(STRUCT_TM_HAS_GMTOFF) || defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__)
    if (yy >= 1900) {
        struct tm t;
        t.tm_sec = ss;
        t.tm_min = ii;
        t.tm_hour = hh;
        t.tm_mday = dd;
        t.tm_mon = mm - 1;
        t.tm_year = yy - 1900;
        t.tm_isdst = -1;
        mktime(&t);

        int s = abs(t.tm_gmtoff / 60);
        if (t.tm_gmtoff >= 0) {
            r.appendf("+%02i:%02i", (int)(s / 60), t.tm_gmtoff % 60);
        } else {
            r.appendf("-%02i:%02i", (int)(s / 60), t.tm_gmtoff % 60);
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
    s.appendf("%i ", t.day);
    s += month[t.month - 1];
    s.appendf(" %04i %02i:%02i:%02i ", t.year, t.hour, t.min, t.sec);
    if (t.have_gmt_offset) {
        if (t.gmt_offset >= 0)
            s.appendf("+%02i%02i", abs(t.gmt_offset / 3600), abs(t.gmt_offset % 3600));
        else
            s.appendf("-%02i%02i", abs(t.gmt_offset / 3600), abs(t.gmt_offset % 3600));
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

uint64_t DateTime::time_t() const
{
    if (yy < 1970) return 0;
    struct tm t;
    t.tm_sec = ss;
    t.tm_min = ii;
    t.tm_hour = hh;
    t.tm_mday = dd;
    t.tm_mon = mm - 1;
    t.tm_year = yy - 1900;
    t.tm_isdst = -1;
    return (uint64_t)mktime(&t);
}

uint64_t DateTime::epoch() const
{
    if (yy < 1900) return 0;
    struct tm t;
    t.tm_sec = ss;
    t.tm_min = ii;
    t.tm_hour = hh;
    t.tm_mday = dd;
    t.tm_mon = mm - 1;
    t.tm_year = yy - 1900;
    t.tm_isdst = -1;
    return (uint64_t)mktime(&t);
}

uint64_t DateTime::longInt() const
{
    uint64_t r = yy * 12 + (mm - 1);
    r = r * 31 + (dd - 1);
    r = r * 24 + hh;
    r = r * 60 + ii;
    r = r * 60 + ss;
    r = r * 1000000 + us;
    return r;
}

int DateTime::weekISO8601() const
{
    if (yy < 1900) throw DateOutOfRangeException("year < 1900 [%i]", yy);
    struct tm t{};
    t.tm_hour = 12;
    t.tm_mday = dd;
    t.tm_mon = mm - 1;
    t.tm_year = yy - 1900;
    t.tm_isdst = -1;

    ::time_t clock = mktime(&t);
    if (!safe_gmtime(clock, &t)) throw InvalidDateException();

    char buffer[10];
#ifdef _WIN32
    if (::strftime(buffer, 10, "%V", &t) == 0) throw InvalidDateException();
#else
    if (::strftime(buffer, 10, "%V", &t) == 0) throw InvalidDateException();
#endif
    return atoi(buffer);
}

int DateTime::week() const
{
    if (yy < 1900) throw DateOutOfRangeException("year < 1900 [%i]", yy);
    struct tm t{};
    t.tm_hour = 12;
    t.tm_mday = dd;
    t.tm_mon = mm - 1;
    t.tm_year = yy - 1900;
    t.tm_isdst = -1;

    ::time_t clock = mktime(&t);
    if (!safe_gmtime(clock, &t)) throw InvalidDateException();
    char buffer[10];
    if (::strftime(buffer, 10, "%U", &t) == 0) throw InvalidDateException();

    return atoi(buffer);
}

bool DateTime::notEmpty() const
{
    if (yy > 0) return true;
    if (mm > 0) return true;
    if (dd > 0) return true;
    if (hh > 0) return true;
    if (ii > 0) return true;
    if (ss > 0) return true;
    if (us > 0) return true;
    return false;
}

bool DateTime::isEmpty() const
{
    if (yy > 0) return false;
    if (mm > 0) return false;
    if (dd > 0) return false;
    if (hh > 0) return false;
    if (ii > 0) return false;
    if (ss > 0) return false;
    if (us > 0) return false;
    return true;
}

bool DateTime::isLeapYear() const
{
    return isLeapYear(yy);
}

bool DateTime::isLeapYear(int year)
{
    if (year % 4 != 0) return 0;
    if (year % 400 == 0) return 1;
    if (year % 100 == 0) return 0;
    return 1;
}

DateTime DateTime::currentTime()
{
    DateTime d;
    d.setCurrentTime();
    return d;
}

int64_t DateTime::diffSeconds(const DateTime& other) const
{
    int64_t mySecs = (int64_t)time_t();
    int64_t otherSecs = (int64_t)other.time_t();
    return otherSecs - mySecs;
}

int DateTime::compareSeconds(const DateTime& other, int tolerance) const
{
    int64_t mySecs = (int64_t)time_t();
    int64_t otherSecs = (int64_t)other.time_t();
    int64_t diff = otherSecs - mySecs;
    if (diff < 0) diff = mySecs - otherSecs;
    if (diff <= tolerance) return 1;
    return 0;
}

DateTime& DateTime::operator=(const String& datetime)
{
    set(datetime);
    return *this;
}

DateTime& DateTime::operator=(const DateTime& other)
{
    set(other);
    return *this;
}

DateTime& DateTime::operator=(DateTime&& other)
{
    if (this != &other) {
        yy = other.yy;
        mm = other.mm;
        dd = other.dd;
        hh = other.hh;
        ii = other.ii;
        ss = other.ss;
        us = other.us;

        other.yy = 0;
        other.mm = 0;
        other.dd = 0;
        other.hh = 0;
        other.ii = 0;
        other.ss = 0;
        other.us = 0;
    }
    return *this;
}

String DateTime::toString() const
{
    String r;
    r.setf("%04i-%02i-%02i %02i:%02i:%02i.%06i", yy, mm, dd, hh, ii, ss, us);
    return r;
}

String DateTime::toString(const String& format) const
{
    return get(format);
}

DateTime::operator String() const
{
    String r;
    r.setf("%04i-%02i-%02i %02i:%02i:%02i.%06i", yy, mm, dd, hh, ii, ss, us);
    return r;
}

std::ostream& operator<<(std::ostream& s, const DateTime& dt)
{
    String str = dt.get("%Y-%m-%d %H:%M:%S.%u");
    return s.write((const char*)str, str.size());
}

} // namespace pplib
