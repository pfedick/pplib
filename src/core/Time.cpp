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

#include "config_pplib.h"
#include <pplib/core/functions.h>
#include <pplib/core/timer.h>
#include <pplib/core/regex.h>

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

/*!\ingroup PPLGroupDateTime
 * \brief Liefert die aktuelle Unixtime in einer Struktur zurück
 *
 * Liefert die aktuelle Unix-Zeit als Return-Wert zurück, sowie aufgeschlüsselt in der
 * Struktur PPLTIME.
 *
 * \param t ist ein Pointer auf eine Struktur vom Typ PPLTIME oder NULL.
 * \returns Bei Erfolg wird die Zeit seit 1.1.1970, 00:00 Uhr in Sekunden zurückgegeben,
 * sowie die Struktur PPLTIME gefüllt, sofert der Parameter \a t nicht NULL ist.
 * Tritt ein Fehler auf, wird ((uint64_t)-1) zurückgegeben und errno entsprechend gesetzt.
 *
 * \see pplib::GetTime()
 * \see pplib::GetTime(PPLTIME *t, uint64_t now)
 *
 */
uint64_t GetTime(PPLTIME* t)
{
    time_t now;
    time(&now);
    if (t) GetTime(t, now);
    return (uint64_t)now;
}

/*!\ingroup PPLGroupDateTime
 * \brief Liefert die aktuelle Unixtime in einer Struktur zurück
 *
 * Liefert die aktuelle Unix-Zeit als Return-Wert zurück, sowie aufgeschlüsselt in der
 * Struktur PPLTIME.
 *
 * \param t Referenz aif eine Struktur vom Typ PPLTIME.
 * \returns Bei Erfolg wird die Zeit seit 1.1.1970, 00:00 Uhr in Sekunden zurückgegeben,
 * sowie die Struktur PPLTIME gefüllt.
 * Tritt ein Fehler auf, wird ((uint64_t)-1) zurückgegeben und errno entsprechend gesetzt.
 *
 */
uint64_t GetTime(PPLTIME& t)
{
    time_t now;
    time(&now);
    return GetTime(t, now);
}

/*! \fn pplib::GetTime (PPLTIME *t, uint64_t now)
 * \ingroup PPLGroupDateTime
 * \brief Wandelt Unix-Zeit in die Struktur PPLTIME um
 *
 * Wandelt die angegebene Unix-Zeit in eine Struktur vom Typ PPLTIME um.
 *
 * \param t ist ein Pointer auf eine Struktur vom Typ PPLTIME oder NULL.
 * \param now enthält die Sekunden seit 1970, die in die PPLTIME-Struktur umgewandelt werden
 * sollen.
 * \returns Bei Erfolg werden die über den Parameter \a now angegebenen Sekunden
 * zurückgeliefert und die Struktur PPLTIME wird gefüllt,
 * \exception Bei Auftreten eines Fehlers wird eine InvalidDateException geworfen.
 *
 * \see pplib::GetTime()
 * \see pplib::GetTime(PPLTIME *t)
 *
 */
ppl_time_t GetTime(PPLTIME* t, ppl_time_t now)
{
    if (!t) return now;
    struct tm tmstruct;
    time_t n = (time_t)now;
    if (!safe_localtime(n, &tmstruct)) throw InvalidDateException();

    t->year = tmstruct.tm_year + 1900;
    t->month = tmstruct.tm_mon + 1;
    t->day = tmstruct.tm_mday;
    t->hour = tmstruct.tm_hour;
    t->min = tmstruct.tm_min;
    t->sec = tmstruct.tm_sec;
    t->epoch = now;
    t->day_of_week = tmstruct.tm_wday;
    t->day_of_year = tmstruct.tm_yday;
    t->summertime = tmstruct.tm_isdst;
#if defined(STRUCT_TM_HAS_GMTOFF) || defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__)
    t->gmt_offset = tmstruct.tm_gmtoff;
    t->have_gmt_offset = 1;
#else
    t->gmt_offset = 0;
    t->have_gmt_offset = 0;
#endif
    return now;
}

/*!\ingroup PPLGroupDateTime
 * \brief Wandelt Unix-Zeit in die Struktur PPLTIME um
 *
 * Wandelt die angegebene Unix-Zeit in eine Struktur vom Typ PPLTIME um.
 *
 * \param t Referenz auf Eine PPLTIME-Struktur
 * \param now enthält die Sekunden seit 1970, die in die PPLTIME-Struktur umgewandelt werden
 * sollen.
 * \returns Bei Erfolg werden die über den Parameter \a now angegebenen Sekunden
 * zurückgeliefert und die Struktur PPLTIME wird gefüllt.
 * \exception Bei Auftreten eines Fehlers wird eine InvalidDateException geworfen.
 *
 * \see pplib::GetTime()
 * \see pplib::GetTime(PPLTIME *t)
 *
 */
ppl_time_t GetTime(PPLTIME& t, ppl_time_t now)
{
    return GetTime(&t, now);
}

void USleep(uint64_t microseconds)
{
    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

void MSleep(uint64_t milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void SSleep(uint64_t seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

double GetMicrotime()
/*!\ingroup PPLGroupDateTime
 */
{
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration<double>(duration).count();
}

uint64_t GetMilliSeconds()
/*!\ingroup PPLGroupDateTime
 * \brief Aktuelle Zeit in Millisekunden
 *
 * \desc
 * Diese Funktion liefert die Anzahl Millisekunden, die seit dem 1.1.1970 0 Uhr vergangen
 * sind. (1000 Millisekunden = 1 Sekunde).
 *
 * \return Anzahl Millisekunden seit 1970.
 *
 */
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

ppl_time_t MkTime(const String& year, const String& month, const String& day, const String& hour, const String& min, const String& sec)
/*!\ingroup PPLGroupDateTime
 */
{
    struct tm Time;
    memset(&Time, 0, sizeof(Time));
    Time.tm_mday = day.toInt();
    Time.tm_mon = month.toInt() - 1;
    Time.tm_year = year.toInt() - 1900;
    Time.tm_hour = hour.toInt();
    Time.tm_min = min.toInt();
    Time.tm_sec = sec.toInt();
    time_t LTime = mktime(&Time);
    return (ppl_time_t)LTime;
}

ppl_time_t MkTime(int year, int month, int day, int hour, int min, int sec)
/*!\ingroup PPLGroupDateTime
 */
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
    if (t.year < 1900 || t.month < 1 || t.month > 12) return 0;
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

ppl_time_t MkTime(const String& iso8601date, PPLTIME* t)
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
    if (t) GetTime(t, (uint64_t)LTime);
    return (ppl_time_t)LTime;
}

String MkRFC822Date(const PPLTIME& t)
/*!\ingroup PPLGroupDateTime
 * \brief Datumstring nach RFC-822 (Mailformat) erzeugen
 *
 * \desc
 * Mit dieser Funktion wird ein Datummstring nach RFC-822 erzeugt, wie er im Header einer Email verwendet wird.
 * Das Format lautet:
 * \code
 * weekday, day month year time zone
 * \endcode
 * und hat folgende Bedeutung:
 * - weekday: Name des Wochentags ("Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat")
 * - day: Tag des Monats mit ein oder zwei Ziffern
 * - month: Name des Monats ("Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec")
 * - year: Das Jahr mit 4 Ziffern
 * - time: Stunde:Minute:Sekunde (hh:mm:ss), jeweils mit zwei Ziffern und Doppelpunkt getrennt
 * - zone: Offset zu UTC in Stunden und Minuten (+|-HHMM)
 *
 * \param[in] t Eine PPLTIME-Struktur, der die Datumsinformationen entnommen werden
 *
 * \exception Exception::FunctionFailed Die Funktion wirft eine Exception, wenn die Datumsinformation in der PPLTIME-Struktur ungültig ist.
 */
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
    s.appendf(" %04i %02i:%02i:%02i ", t.year, t.hour, t.min, t.sec);
    if (t.have_gmt_offset) {
        if (t.gmt_offset >= 0)
            s.appendf("+%02i%02i", abs(t.gmt_offset / 3600), abs(t.gmt_offset % 3600));
        else
            s.appendf("-%02i%02i", abs(t.gmt_offset / 3600), abs(t.gmt_offset % 3600));
    }
    return s;
}

String MkRFC822Date(ppl_time_t sec)
/*!\ingroup PPLGroupDateTime
 * \brief Datumstring nach RFC-822 (Mailformat) erzeugen
 *
 * \desc
 * Mit dieser Funktion wird ein Datummstring nach RFC-822 erzeugt, wie er im Header einer Email verwendet wird.
 * Das Format lautet:
 * \code
 * weekday, day month year time zone
 * \endcode
 * und hat folgende Bedeutung:
 * - weekday: Name des Wochentags ("Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat")
 * - day: Tag des Monats mit ein oder zwei Ziffern
 * - month: Name des Monats ("Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec")
 * - year: Das Jahr mit 4 Ziffern
 * - time: Stunde:Minute:Sekunde (hh:mm:ss), jeweils mit zwei Ziffern und Doppelpunkt getrennt
 * - zone: Offset zu UTC in Stunden und Minuten (+|-HHMM)
 *
 * \param[in] sec Ein Optionaler Parameter mit Sekunden seit 1970. Ist er 0 oder wird nicht angegeben, wird die
 * aktuelle Zeit verwendet.
 *
 * \exception Exception::FunctionFailed Die Funktion wirft eine Exception, wenn die Datumsinformation in der PPLTIME-Struktur ungültig ist.
 */
{
    PPLTIME t;
    if (!sec) sec = GetTime();
    if (GetTime(t, sec) != sec) throw OperationFailedException();
    return MkRFC822Date(t);
}

String MkISO8601Date(ppl_time_t sec)
/*!\ingroup PPLGroupDateTime
 */
{
    PPLTIME t;
    if (!sec) sec = GetTime();
    if (GetTime(t, sec) != sec) throw OperationFailedException();
    return MkISO8601Date(t);
}

String MkISO8601Date(const PPLTIME& t)
/*!\ingroup PPLGroupDateTime
 */
{
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
    } else {
        if (t.summertime)
            buffer.append("+01:00");
        else
            buffer.append("+02:00");
    }
    return buffer;
}

/*!\brief Datum/Zeit formatieren
 * \ingroup PPLGroupDateTime
 *
 * \header \#include <pplib.h>
 * \desc
 * Die Funktion MkDate wandelt einen Unix-Timestamp in einen String um.
 *
 * \param format ist ein beliebiger String, der verschiedene  Platzhalter
 * entahlten darf (siehe unten)
 * \param sec
 * \return Bei Erfolg gibt die Funktion einen neuen String mit dem formatierten
 * Zeitpunkt zurück.
 * \exception Im Fehlerfall wird eine Exception geworfen
 *
 * \par Syntax-Formatstring
 * \copydoc strftime.dox
 */
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

/*
 * Timer-Klasse
 */

/*!\class Timer
 * \ingroup PPLGroupDateTime
 */

Timer::Timer()
{
    startzeit = GetMicrotime();
    endzeit = 0.0;
    myduration = 0.0;
}

Timer::~Timer()
{
}

double Timer::start()
{
    startzeit = GetMicrotime();
    return startzeit;
}

double Timer::stop()
{
    endzeit = GetMicrotime();
    myduration = endzeit - startzeit;
    return myduration;
}

double Timer::currentDuration()
{
    return GetMicrotime() - startzeit;
}

double Timer::duration()
{
    return myduration;
}

} // namespace pplib
