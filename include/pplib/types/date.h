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

/** @class Date
 * @brief Datentyp für Datum
 * @ingroup PPLGroupDataTypes
 *
 * Diese Klasse stellt Methoden zum Verwalten und Formatieren von Datumswerten bereit.
 */
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

    /** @brief Konstruktor für Datum mit Jahr, Monat und Tag
     *
     * Setzt ein Datum mit den angegebenen Werten für Jahr, Monat und Tag.
     * Die einzelnen Werte werden validiert, so dass kein ungültiges Datum entstehen kann.
     *
     * @param year Jahr (0-9999). Die Jahreszählung folgt dem proleptisch gregorianischen
     * Kalender, das Jahr 0 entspricht also 1 v. Chr.
     * @param month Monat (1-12)
     * @param day Tag (1-31)
     * @exception IllegalArgumentException Die übergebenen Werte für Jahr, Monat oder Tag sind ungültig.
     */
    inline Date(int year, int month, int day)
    {
        set(year, month, day);
    }

    inline Date(const String& date)
    {
        set(date);
    }

    /** @brief Konstruktor für Datum mit Jahr, Monat und Tag
     *
     * Setzt ein Datum mit den angegebenen Werten für Jahr, Monat und Tag.
     * Die einzelnen Werte werden validiert, so dass kein ungültiges Datum entstehen kann.
     *
     * @param year Jahr (0-9999). Die Jahreszählung folgt dem proleptisch gregorianischen
     * Kalender, das Jahr 0 entspricht also 1 v. Chr.
     * @param month Monat (1-12)
     * @param day Tag (1-31)
     * @exception IllegalArgumentException Die übergebenen Werte für Jahr, Monat oder Tag sind ungültig.
     */
    Date& set(int year, int month, int day);
    Date& set(const String& date);

    /** @brief Konvertiert das Datum in einen String im Standardformat "YYYY-MM-DD"
     *
     * @return String im Format "YYYY-MM-DD"
     * @note Falls das Datum leer ist, wird "0000-00-00" zurückgegeben.
     */
    String toString() const;
    /** @brief Formatiert das Datum gemäß dem angegebenen Formatstring
     *
     * @param format Formatstring (z.B. "YYYY-MM-DD")
     * @return Formatierter Datumsstring
     * @note Falls das Datum leer ist, wird "0000-00-00" zurückgegeben.
     */
    String format(const String& format) const;

    Date& operator=(const Date& other) noexcept
    {
        dd = other.dd;
        mm = other.mm;
        yy = other.yy;
        return *this;
    }

    Date& operator=(Date&& other) noexcept
    {
        if (this == &other) {
            return *this;
        }
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
        if (date == 0) return Date();
        return Date(date / 10000, (date / 100) % 100, date % 100);
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
     * Judentums, Christentums und Islams erhalten hat, und ist identisch zur
     * Formatangabe "%U" von \c strftime. Dabei gilt folgende Regel:
     * - Jeden Sonntag beginnt eine neue Kalenderwoche
     * - Alle Tage vor dem ersten Sonntag des Jahres liegen in Woche 0. Nur wenn der
     *   1. Januar selbst ein Sonntag ist, beginnt er direkt die Woche 1.
     *
     * @return Wochennummer im Bereich 0 bis 53
     * @exception IllegalStateException Das Datum ist leer
     * @see Date::weekISO8601 für die in Europa übliche Zählweise nach ISO 8601
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