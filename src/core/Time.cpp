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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <chrono>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "config_pplib.h"
#include <pplib/core/time.h>
#include <pplib/core/timer.h>
#include <pplib/core/regex.h>
#include <pplib/exceptions.h>

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

ppl_time_t GetTime()
{
    time_t now;
    time(&now);
    return (uint64_t)now;
}

static void fill_ppltime_from_tm(PPLTIME& t, const struct tm& tmstruct, ppl_time_t now)
{
    t.year = tmstruct.tm_year + 1900;
    t.month = tmstruct.tm_mon + 1;
    t.day = tmstruct.tm_mday;
    t.hour = tmstruct.tm_hour;
    t.min = tmstruct.tm_min;
    t.sec = tmstruct.tm_sec;
    t.epoch = now;
    t.day_of_week = tmstruct.tm_wday;
    t.day_of_year = tmstruct.tm_yday;
    t.summertime = tmstruct.tm_isdst != 0;
#if defined(STRUCT_TM_HAS_GMTOFF) || defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__)
    t.gmt_offset = tmstruct.tm_gmtoff;
    t.have_gmt_offset = true;
#else
    t.gmt_offset = 0;
    t.have_gmt_offset = false;
#endif
}

PPLTIME LocalTime(ppl_time_t tt)
{
    struct tm tmstruct;
    time_t n = (time_t)tt;
    if (!safe_localtime(n, &tmstruct)) throw InvalidDateException();
    PPLTIME t;
    fill_ppltime_from_tm(t, tmstruct, tt);
    return t;
}

PPLTIME GMTime(ppl_time_t tt)
{
    struct tm tmstruct;
    time_t n = (time_t)tt;
    if (!safe_gmtime(n, &tmstruct)) throw InvalidDateException();
    PPLTIME t;
    fill_ppltime_from_tm(t, tmstruct, tt);
    return t;
}

void USleep(uint64_t microseconds)
{
#if defined(_WIN32) && defined(CREATE_WAITABLE_TIMER_HIGH_RESOLUTION)
    // CREATE_WAITABLE_TIMER_HIGH_RESOLUTION gibt es seit Windows 10 (1803)
    HANDLE timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);

    if (timer) {
        LARGE_INTEGER due_time;
        // Negative Zahl = relative Zeit in 100ns-Einheiten
        due_time.QuadPart = -static_cast<LONGLONG>(microseconds * 10);

        SetWaitableTimer(timer, &due_time, 0, NULL, NULL, 0);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
    } else {
        // LCOV_EXCL_START
        // Fallback für sehr alte Windows-Versionen
        std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
        // LCOV_EXCL_STOP
    }
#else
    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
#endif
}

void MSleep(uint64_t milliseconds)
{
    USleep(milliseconds * 1000);
}

void SSleep(uint64_t seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void Sleep(double seconds)
{
    USleep(static_cast<uint64_t>(seconds * 1000000));
}

double GetMicrotime()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration<double>(duration).count();
}

uint64_t GetMilliSeconds()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

uint64_t GetMicroSeconds()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

ppl_time_t MkTime(int year, int month, int day, int hour, int min, int sec)
{
    struct tm Time;
    if (year < 1900 || month < 1) return 0;
    memset(&Time, 0, sizeof(Time));
    Time.tm_mday = day;
    Time.tm_mon = month - 1;
    Time.tm_year = year - 1900;
    Time.tm_hour = hour;
    Time.tm_min = min;
    Time.tm_sec = sec;
    time_t LTime = mktime(&Time);
    return (ppl_time_t)LTime;
}

ppl_time_t MkTime(const PPLTIME& t)
/*!\ingroup PPLGroupDateTime
 */
{
    struct tm Time;
    if (t.year < 1900 || t.month < 1) return 0;
    memset(&Time, 0, sizeof(Time));
    Time.tm_mday = t.day;
    Time.tm_mon = t.month - 1;
    Time.tm_year = t.year - 1900;
    Time.tm_hour = t.hour;
    Time.tm_min = t.min;
    Time.tm_sec = t.sec;
    time_t LTime = mktime(&Time);
    return (ppl_time_t)LTime;
}

ppl_time_t MkTime(const String& iso8601date)
/*!\ingroup PPLGroupDateTime
 */
{
    std::vector<String> match;
    struct tm Time;
    memset(&Time, 0, sizeof(Time));
    if (RegEx::capture("/^([0-9]{4})-([0-9]{2})-([0-9]{2})T([0-9]{2}):([0-9]{2}):([0-9]{2})\\+([0-9]{2}):([0-9]{2})$/i", iso8601date,
                       match)) {
        Time.tm_hour = 0 - match[7].toInt();
        Time.tm_min = 0 - match[8].toInt();
    } else if (RegEx::capture("/^([0-9]{4})-([0-9]{2})-([0-9]{2})T([0-9]{2}):([0-9]{2}):([0-9]{2})\\-([0-9]{2}):([0-9]{2})$/i", iso8601date,
                              match)) {
        Time.tm_hour = match[7].toInt();
        Time.tm_min = match[8].toInt();
    } else if (!RegEx::capture("/^([0-9]{4})-([0-9]{2})-([0-9]{2})T([0-9]{2}):([0-9]{2}):([0-9]{2})$/i", iso8601date, match)) {
        throw InvalidFormatException();
    }
    Time.tm_mday = match[3].toInt();
    Time.tm_mon = match[2].toInt() - 1;
    Time.tm_year = match[1].toInt() - 1900;
    Time.tm_hour += match[4].toInt();
    Time.tm_min += match[5].toInt();
    Time.tm_sec = match[6].toInt();

    time_t LTime = ::mktime(&Time);
    if (LTime == (time_t)-1) throw InvalidDateException(iso8601date);
    return (ppl_time_t)LTime;
}

String MkRFC822Date(const PPLTIME& t)
{
    String s;
    const char* day[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char* month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    // PPLTIME prüfen
    if (t.day_of_week < 0 || t.day_of_week > 6) throw IllegalArgumentException("MkRFC822Date: week<0 order week>6");
    if (t.month < 1 || t.month > 12) throw IllegalArgumentException("MkRFC822Date: month<0 order month>12");

    s = day[t.day_of_week];
    s += ", ";
    s.appendf("%i ", t.day);
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

String MkRFC822Date(ppl_time_t sec)
{
    PPLTIME t = LocalTime(sec);
    return MkRFC822Date(t);
}

String MkISO8601Date(ppl_time_t sec)
{
    PPLTIME t = LocalTime(sec);
    return MkISO8601Date(t);
}

String MkISO8601Date(const PPLTIME& t)
/*!\ingroup PPLGroupDateTime
 */
{
    // PPLTIME prüfen
    if (t.month < 1 || t.month > 12) throw IllegalArgumentException("MkRFC822Date: month<0 order month>12");

    String buffer;
    buffer.setf("%04i-%02i-%02iT%02i:%02i:%02i", t.year, t.month, t.day, t.hour, t.min, t.sec);
    if (t.have_gmt_offset) {
        int off = abs(t.gmt_offset) / 60;
        int h = (off / 60);
        int m = (off % 60);
        if (t.gmt_offset < 0)
            buffer.appendf("-%02i:%02i", h, m);
        else
            buffer.appendf("+%02i:%02i", h, m);
    }
    return buffer;
}

String MkDate(const String& format, ppl_time_t sec)
{
    size_t size = strlen(format) * 2 + 32;
    std::vector<char> b(size);
    struct tm t;
    const time_t tt = (const time_t)sec;

    if (!safe_localtime(tt, &t)) throw InvalidDateException();
    if (strftime(b.data(), size, format, &t) == 0) {
        throw OperationFailedException();
    }
    return String(b.data());
}

String MkDate(const String& format, const PPLTIME& t)
{
    return MkDate(format, MkTime(t));
}

} // namespace pplib
