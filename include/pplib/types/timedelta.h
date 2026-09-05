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

#ifndef PPLIB_TYPES_TIMEDELTA_H_
#define PPLIB_TYPES_TIMEDELTA_H_

#include <stdint.h>
#include <ostream>

#include "pplib/types/string.h"

namespace pplib
{
class String;

/** @class TimeDelta
 * @brief Datentyp für Zeitintervalle
 * @ingroup PPLGroupDataTypes
 *
 * Diese Klasse stellt Methoden zum Verwalten und Formatieren von Zeitintervallen bereit.
 */
class TimeDelta
{
private:
    int64_t us = 0;

    explicit constexpr TimeDelta(int64_t microseconds) noexcept
        : us(microseconds)
    {
    }

public:
    /** @brief Standardkonstruktor
     *
     * Erstellt ein TimeDelta mit dem Wert 0.
     */
    TimeDelta() noexcept = default;

    /** @brief Konstruktor mit Tagen, Stunden, Minuten, Sekunden, Millisekunden und Mikrosekunden
     *
     * Erstellt ein TimeDelta mit den angegebenen Werten. Werte dürfen negativ sein.
     *
     * @param days Anzahl der Tage
     * @param hours Anzahl der Stunden
     * @param minutes Anzahl der Minuten (optional, Standard: 0)
     * @param seconds Anzahl der Sekunden (optional, Standard: 0)
     * @param milliseconds Anzahl der Millisekunden (optional, Standard: 0)
     * @param microseconds Anzahl der Mikrosekunden (optional, Standard: 0)
     */
    explicit TimeDelta(
        int64_t days, int64_t hours, int64_t minutes = 0, int64_t seconds = 0, int64_t milliseconds = 0, int64_t microseconds = 0) noexcept
    {
        set(days, hours, minutes, seconds, milliseconds, microseconds);
    }

    /** @brief Konstruktor mit String
     *
     * Ein TimeDelta wird aus einem String erzeugt. Der String muss in folgendem Format vorliegen:
     *   - "HH:MM"
     *   - "HH:MM:SS" oder "HH:MM:SS.UUUUUU"
     *   - "DD:HH:MM:SS" oder "DD:HH:MM:SS.UUUUUU"
     * Negative Zeitintervalle werden durch ein führendes "-" dargestellt.
     *
     * @param str String mit der Zeitangabe
     * @exception IllegalArgumentException
     */
    TimeDelta(const String& str)
    {
        us = fromString(str).us;
    }

    /** @brief Gibt die Zeit in Mikrosekunden zurück
     *
     * @return Zeit in Mikrosekunden
     */
    int64_t toMicroseconds() const noexcept
    {
        return us;
    }

    /** @brief Gibt die Zeit in Millisekunden zurück
     *
     * @return Zeit in Millisekunden
     */
    int64_t toMilliseconds() const noexcept
    {
        return us / 1000;
    }

    /** @brief Gibt die Zeit in Sekunden zurück
     *
     * @return Zeit in Sekunden
     */
    int64_t toSeconds() const noexcept
    {
        return us / 1000000;
    }

    /** @brief Setzt die Zeit mit Tagen, Stunden, Minuten, Sekunden, Millisekunden und Mikrosekunden
     *
     * Werte dürfen negativ sein.
     *
     * @param days Anzahl der Tage
     * @param hours Anzahl der Stunden
     * @param minutes Anzahl der Minuten (optional, Standard: 0)
     * @param seconds Anzahl der Sekunden (optional, Standard: 0)
     * @param milliseconds Anzahl der Millisekunden (optional, Standard: 0)
     * @param microseconds Anzahl der Mikrosekunden (optional, Standard: 0)
     * @return Referenz auf das aktuelle TimeDelta-Objekt
     */
    TimeDelta& set(
        int64_t days, int64_t hours, int64_t minutes = 0, int64_t seconds = 0, int64_t milliseconds = 0, int64_t microseconds = 0) noexcept
    {
        us = (days * 86400000000LL) + (hours * 3600000000LL) + (minutes * 60000000LL) + (seconds * 1000000LL) + (milliseconds * 1000LL) +
             microseconds;
        return *this;
    }

    /** @brief Erzeugt ein TimeDelta aus einem String
     *
     * Ein TimeDelta wird aus einem String erzeugt. Der String muss in folgendem Format vorliegen:
     *   - "HH:MM"
     *   - "HH:MM:SS" oder "HH:MM:SS.UUUUUU"
     *   - "DD:HH:MM:SS" oder "DD:HH:MM:SS.UUUUUU"
     * Negative Zeitintervalle werden durch ein führendes "-" dargestellt.
     *
     * @param str String mit der Zeitangabe
     * @return TimeDelta Objekt
     * @exception IllegalArgumentException
     */
    static TimeDelta fromString(const String& str);

    /** @brief Wandelt das TimeDelta in einen String um
     *
     * Der String wird im Format "HH:MM:SS.UUUUUU" zurückgegeben.
     * Ein optionales Vorzeichen "-" wird für negative TimeDeltas hinzugefügt.
     *
     * @return String mit der Zeitangabe
     */
    String toString() const;

    /** @brief Erzeugt ein TimeDelta aus einer Anzahl von Wochen
     *
     * @param weeks Anzahl der Wochen
     * @return TimeDelta Objekt
     */
    static TimeDelta fromWeeks(int64_t weeks) noexcept
    {
        return fromDays(weeks * 7);
    }

    /** @brief Erzeugt ein TimeDelta aus einer Anzahl von Tagen
     *
     * @param days Anzahl der Tage
     * @return TimeDelta Objekt
     */
    static TimeDelta fromDays(int64_t days) noexcept
    {
        return TimeDelta(days * 86400000000LL);
    }

    /** @brief Erzeugt ein TimeDelta aus einer Anzahl von Stunden
     *
     * @param hours Anzahl der Stunden
     * @return TimeDelta Objekt
     */
    static TimeDelta fromHours(int64_t hours) noexcept
    {
        return TimeDelta(hours * 3600000000LL);
    }

    /** @brief Erzeugt ein TimeDelta aus einer Anzahl von Minuten
     *
     * @param mins Anzahl der Minuten
     * @return TimeDelta Objekt
     */
    static TimeDelta fromMinutes(int64_t mins) noexcept
    {
        return TimeDelta(mins * 60000000LL);
    }

    /** @brief Erzeugt ein TimeDelta aus einer Anzahl von Sekunden
     *
     * @param secs Anzahl der Sekunden
     * @return TimeDelta Objekt
     */
    static TimeDelta fromSeconds(int64_t secs) noexcept
    {
        return TimeDelta(secs * 1000000LL);
    }

    /** @brief Erzeugt ein TimeDelta aus einer Anzahl von Millisekunden
     *
     * @param ms Anzahl der Millisekunden
     * @return TimeDelta Objekt
     */
    static TimeDelta fromMilliseconds(int64_t ms) noexcept
    {
        return TimeDelta(ms * 1000LL);
    }

    /** @brief Erzeugt ein TimeDelta aus einer Anzahl von Mikrosekunden
     *
     * @param us Anzahl der Mikrosekunden
     * @return TimeDelta Objekt
     */
    static TimeDelta fromMicroseconds(int64_t us) noexcept
    {
        return TimeDelta(us);
    }

    // Operatoren

    /** @brief Addition zweier TimeDelta-Objekte
     *
     * @param other Das andere TimeDelta-Objekt
     * @return Neues TimeDelta-Objekt, das die Summe darstellt
     */
    TimeDelta operator+(const TimeDelta& other) const noexcept
    {
        return TimeDelta(us + other.us);
    }

    /** @brief Subtraktion zweier TimeDelta-Objekte
     *
     * @param other Das andere TimeDelta-Objekt
     * @return Neues TimeDelta-Objekt, das die Differenz darstellt
     */
    TimeDelta operator-(const TimeDelta& other) const noexcept
    {
        return TimeDelta(us - other.us);
    }

    /** @brief Negation eines TimeDelta-Objekts
     *
     * @return Neues TimeDelta-Objekt, das die Negation darstellt
     */
    TimeDelta operator-() const noexcept
    {
        return TimeDelta(-us);
    }

    /** @brief Addition eines TimeDelta-Objekts mit einem anderen
     *
     * @param other Das andere TimeDelta-Objekt
     * @return Referenz auf das aktuelle TimeDelta-Objekt nach der Addition
     */
    TimeDelta& operator+=(const TimeDelta& other) noexcept
    {
        us += other.us;
        return *this;
    }

    /** @brief Subtraktion eines TimeDelta-Objekts mit einem anderen
     *
     * @param other Das andere TimeDelta-Objekt
     * @return Referenz auf das aktuelle TimeDelta-Objekt nach der Subtraktion
     */
    TimeDelta& operator-=(const TimeDelta& other) noexcept
    {
        us -= other.us;
        return *this;
    }

    /** @brief Multiplikation eines TimeDelta-Objekts mit einem Faktor
     *
     * @param factor Der Multiplikationsfaktor
     * @return Neues TimeDelta-Objekt, das das Ergebnis der Multiplikation darstellt
     */
    TimeDelta operator*(int64_t factor) const noexcept
    {
        return TimeDelta(us * factor);
    }
    /** @brief Division eines TimeDelta-Objekts durch einen Divisor
     *
     * @param divisor Der Divisor
     * @return Neues TimeDelta-Objekt, das das Ergebnis der Division darstellt
     * @exception IllegalArgumentException Wenn der Divisor null ist
     */
    TimeDelta operator/(int64_t divisor) const;

    /** @brief Vergleich, ob zwei TimeDelta-Objekte gleich sind
     *
     * @param other Das andere TimeDelta-Objekt
     * @return true, wenn die TimeDelta-Objekte gleich sind, sonst false
     */
    bool operator==(const TimeDelta& other) const noexcept
    {
        return us == other.us;
    }

    /** @brief Vergleich, ob zwei TimeDelta-Objekte ungleich sind
     *
     * @param other Das andere TimeDelta-Objekt
     * @return true, wenn die TimeDelta-Objekte ungleich sind, sonst false
     */
    bool operator!=(const TimeDelta& other) const noexcept
    {
        return us != other.us;
    }

    /** @brief Vergleich, ob ein TimeDelta-Objekt kleiner als ein anderes ist
     *
     * @param other Das andere TimeDelta-Objekt
     * @return true, wenn das aktuelle TimeDelta-Objekt kleiner ist, sonst false
     */
    bool operator<(const TimeDelta& other) const noexcept
    {
        return us < other.us;
    }

    /** @brief Vergleich, ob ein TimeDelta-Objekt kleiner oder gleich einem anderen ist
     *
     * @param other Das andere TimeDelta-Objekt
     * @return true, wenn das aktuelle TimeDelta-Objekt kleiner oder gleich ist, sonst false
     */
    bool operator<=(const TimeDelta& other) const noexcept
    {
        return us <= other.us;
    }

    /** @brief Vergleich, ob ein TimeDelta-Objekt größer als ein anderes ist
     *
     * @param other Das andere TimeDelta-Objekt
     * @return true, wenn das aktuelle TimeDelta-Objekt größer ist, sonst false
     */
    bool operator>(const TimeDelta& other) const noexcept
    {
        return us > other.us;
    }

    /** @brief Vergleich, ob ein TimeDelta-Objekt größer oder gleich einem anderen ist
     *
     * @param other Das andere TimeDelta-Objekt
     * @return true, wenn das aktuelle TimeDelta-Objekt größer oder gleich ist, sonst false
     */
    bool operator>=(const TimeDelta& other) const noexcept
    {
        return us >= other.us;
    }
};

inline std::ostream& operator<<(std::ostream& s, const TimeDelta& t)
{
    pplib::String tmp = t.toString();
    return s.write(tmp.c_str(), tmp.length());
}

} // namespace pplib

#endif // PPLIB_TYPES_TIMEDELTA_H_