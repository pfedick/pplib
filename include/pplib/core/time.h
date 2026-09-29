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

#ifndef PPLIB_CORE_TIME_H_
#define PPLIB_CORE_TIME_H_

#include <stdint.h>
#include <pplib/types/string.h>

namespace pplib
{

/// Eine Struktur zum Erfassen von Uhrzeit, Datum und Zeitzone

/**
 * @struct tagTime
 * @brief Struktur zum Erfassen von Uhrzeit, Datum und Zeitzone
 * @ingroup PPLGroupDateTime
 */
typedef struct tagTime
{
    int64_t epoch;        //!< Unix-Timestamp in Sekunden. Vor 1970 immer 0.
    int32_t year;         //!< Jahr (Gregorianischer Kalender)
    int32_t gmt_offset;   //!< Offset zur GMT in Sekunden
    int16_t day_of_year;  //!< Der Tag im Jahr (1-366)
    int8_t month;         //!< Monat (1-12)
    int8_t day;           //!< Tag im Monat (1-31)
    int8_t hour;          //!< Stunde (0-23)
    int8_t min;           //!< Minute (0-59)
    int8_t sec;           //!< Sekunde (0-59)
    int8_t day_of_week;   //!< Wochentag (0=Sonntag, 1=Montag, ..., 6=Samstag)
    bool have_gmt_offset; //!< Gibt an, ob ein GMT-Offset vorhanden ist
    bool summertime;      //!< Gibt an, ob Sommerzeit aktiv ist
} PPLTIME;

/// Datentyp für Unix-Timestamps in 64 Bit

/**
 * @brief Datentyp für Unix-Timestamps in 64 Bit
 * @ingroup PPLGroupDateTime
 */
typedef uint64_t ppl_time_t;

// Time
ppl_time_t GetTime(PPLTIME* t = nullptr);
ppl_time_t GetTime(PPLTIME* t, ppl_time_t tt);
ppl_time_t GetTime(PPLTIME& t, ppl_time_t tt);

void USleep(uint64_t microseconds); // 1 sec = 1000000 microseconds
void MSleep(uint64_t milliseconds); // 1 sec = 1000 milliseconds
void SSleep(uint64_t seconds);
double GetMicrotime();
uint64_t GetMilliSeconds();

ppl_time_t MkTime(
    const String& year, const String& month, const String& day, const String& hour = "0", const String& min = "0", const String& sec = "0");
ppl_time_t MkTime(int year, int month, int day, int hour = 0, int min = 0, int sec = 0);
ppl_time_t MkTime(const String& iso8601date, PPLTIME* t = NULL);
ppl_time_t MkTime(const PPLTIME& t);

String MkISO8601Date(ppl_time_t sec = 0);
String MkISO8601Date(const PPLTIME& t);
String MkRFC822Date(ppl_time_t sec = 0);
String MkRFC822Date(const PPLTIME& t);
String MkDate(const String& format, ppl_time_t sec);
String MkDate(const String& format, const PPLTIME& t);

}; // namespace pplib

#endif // PPLIB_CORE_TIME_H_
