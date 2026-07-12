/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
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
#ifndef PPL7_TYPES_DATETIME_H_
#define PPL7_TYPES_DATETIME_H_

#include <stdint.h>
#include "ppl7/types/string.h"

namespace ppl7
{

class String;

/// Eine Struktur zum Erfassen von Uhrzeit und Datum
typedef struct tagTime
{
    uint64_t epoch;
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
    int day_of_week;
    int day_of_year;
    int summertime;
    int gmt_offset;
    int have_gmt_offset;
} PPLTIME;

/// Datentyp für Unix-Timestamps in 64 Bit
typedef uint64_t ppl_time_t;

class DateTime
{
private:
    uint32_t us;
    uint16_t yy;
    uint8_t mm;
    uint8_t dd;
    uint8_t hh;
    uint8_t ii;
    uint8_t ss;

public:
    DateTime();
    DateTime(const String& datetime);
    DateTime(const DateTime& other);
    DateTime(uint64_t t);

    void setTime_t(uint64_t t);
    void setEpoch(uint64_t t);
    void setLongInt(uint64_t i);
    void set(const String& datetime);
    void set(const DateTime& other);
    void set(const String& date, const String& time);
    void set(const PPLTIME& t);
    void setDate(const String& date);
    void setTime(const String& time);
    void set(int year, int month, int day, int hour = 0, int minute = 0, int sec = 0, int msec = 0, int usec = 0);
    void setCurrentTime();
    void print() const;
    void clear();
    bool notEmpty() const;
    bool isEmpty() const;
    bool isLeapYear() const;

    String get(const String& format = "%Y-%m-%d %H:%M:%S") const;
    String getDate(const String& format = "%Y-%m-%d") const;
    String getTime(const String& format = "%H:%M:%S") const;
    String getISO8601() const;
    String getISO8601withMsec() const;
    String getISO8601withUsec() const;
    String getRFC822Date() const;
    String strftime(const String& format) const;
    uint64_t time_t() const;
    uint64_t epoch() const;
    uint64_t longInt() const;

    int year() const;
    int month() const;
    int day() const;
    int hour() const;
    int minute() const;
    int second() const;
    int millisecond() const;
    int microsecond() const;
    int week() const;
    int weekISO8601() const;

    int64_t diffSeconds(const DateTime& other) const;
    int compareSeconds(const DateTime& other, int tolerance = 0) const;

    DateTime& operator=(const String& datetime);
    DateTime& operator=(const DateTime& other);

    static bool isLeapYear(int year);
    static DateTime currentTime();
    String toString() const;
    String toString(const String& format) const;
    operator String() const;

    bool operator<(const DateTime& other) const;
    bool operator<=(const DateTime& other) const;
    bool operator==(const DateTime& other) const;
    bool operator!=(const DateTime& other) const;
    bool operator>=(const DateTime& other) const;
    bool operator>(const DateTime& other) const;
};

} // namespace ppl7

#endif /* PPL7_TYPES_DATETIME_H_ */