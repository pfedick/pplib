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

#ifndef PPLIB_TYPES_TIME_H_
#define PPLIB_TYPES_TIME_H_

#include <stdint.h>
#include <ostream>
#include "pplib/types/string.h"

namespace pplib
{

/** @class Time
 * @brief Datentyp für Uhrzeit
 *
 * Mit diesem Datentyp kann eine Uhrzeit mit Stunden, Minuten, Sekunden und Mikrosekunden gespeichert werden.
 * Die Uhrzeit wird intern in Mikrosekunden gespeichert. Die maximale Uhrzeit ist 23:59:59.999999.
 */
class Time
{
private:
    uint32_t us = 0; //!< Mikrosekunden
    uint8_t hh = 0;  //!< Stunde
    uint8_t ii = 0;  //!< Minute
    uint8_t ss = 0;  //!< Sekunde

public:
    /** @brief Konstruktor mit Initialisierung auf 0
     *
     * Mit diesem Konstruktor ohne Parameter wird der Wert der Uhrzeitklasse auf 0 gesetzt. Die Funktion
     * Time::toMicroseconds() würde \c 0 zurückliefern.
     */
    Time() noexcept = default;

    /** @brief Konstruktor mit Initialisierung aus einem String
     *
     * Über diesen Konstruktor wird ein Time Objekt anhand des im String \p time enthaltenen
     * Zeitwertes erstellt. Das unterstützte Format ist in der Funktion
     * \ref Time::set(const String &time) "set" beschrieben.
     *
     * @param[in] time String mit der Uhrzeit
     *
     * @exception IllegalArgumentException: Wird geworfen, wenn der String \p time
     * ein ungültiges oder unbekanntes Zeitformat hat.
     */
    inline Time(const String& time)
    {
        set(time);
    }

    /** @brief Copy-Konstruktor
     *
     * Über diesen Konstruktor wird die Uhrzeit eines anderen Time-Wertes übernommen.
     *
     * @param[in] other Referenz auf den zu kopierenden Time-Wert
     */
    Time(const Time& other) noexcept;

    /** @brief Move-Konstruktor
     *
     * Über diesen Konstruktor wird die Uhrzeit eines anderen Time-Wertes übernommen.
     *
     * @param[in] other Rvalue-Referenz auf den zu kopierenden Time-Wert
     */
    Time(Time&& other) noexcept;

    /** @brief Konstruktor mit Angabe von Sekunden
     *
     * Mit dieser Konstruktor werden Stunden, Minuten und Sekunden aus einem Wert in Sekunden übernommen.
     * Die Mikrosekunden werden auf 0 gesetzt. Der Wert in Sekunden wird modulo 86400 genommen, um sicherzustellen, dass er innerhalb eines
     * Tages liegt.
     * @param[in] seconds Sekunden seit Mitternacht (0-86399)
     */
    inline Time(uint32_t seconds) noexcept
    {
        setFromSeconds(seconds);
    }

    /** @brief Konstruktor mit Angabe von Stunden, Minuten, Sekunden und Mikrosekunden
     *
     * Mit dieser Konstruktor werden Stunden, Minuten, Sekunden und Mikrosekunden aus den angegebenen Werten übernommen.
     * @param[in] hour Stunde (0-23)
     * @param[in] minute Minute (0-59)
     * @param[in] second Sekunde (0-59)
     * @param[in] microseconds Mikrosekunden (0-999999)
     * @exception IllegalArgumentException: Wird geworfen, wenn einer der Parameter außerhalb des gültigen Bereichs liegt.
     */
    inline Time(uint8_t hour, uint8_t minute, uint8_t second, uint32_t microseconds = 0)
    {
        set(hour, minute, second, microseconds);
    }

    /** @brief Setzt die Uhrzeit auf 00:00:00.000000
     *
     * Mit dieser Funktion wird die Uhrzeit auf 0 Uhr gesetzt.
     */
    inline void clear() noexcept
    {
        hh = 0;
        ii = 0;
        ss = 0;
        us = 0;
    }

    /** @brief Gibt die Uhrzeit in Mikrosekunden zurück
     *
     * Mit dieser Funktion wird die Uhrzeit in Mikrosekunden zurückgegeben. Die maximale Uhrzeit ist 23:59:59.999999, was 86399999999
     * Mikrosekunden entspricht.
     *
     * @return Uhrzeit in Mikrosekunden
     */
    inline uint64_t toMicroseconds() const noexcept
    {
        return ((uint64_t)hh * 3600000000ULL) + ((uint64_t)ii * 60000000ULL) + ((uint64_t)ss * 1000000ULL) + (uint64_t)us;
    }

    /** @brief Gibt die Uhrzeit in Sekunden zurück
     *
     * Mit dieser Funktion wird die Uhrzeit in Sekunden zurückgegeben. Die maximale Uhrzeit ist 23:59:59, was 86399 Sekunden entspricht.
     *
     * @return Uhrzeit in Sekunden
     */
    inline uint32_t toSeconds() const noexcept
    {
        return hh * 3600 + ii * 60 + ss;
    }

    /** @brief Setzt die Uhrzeit in Mikrosekunden
     *
     * Mit dieser Funktion wird die Uhrzeit auf den angegebenen Wert in Mikrosekunden gesetzt. Der Wert wird modulo 86400000000 genommen, um
     * sicherzustellen, dass er innerhalb eines Tages liegt.
     *
     * @param microseconds Uhrzeit in Mikrosekunden
     * @return Referenz auf das aktuelle Objekt
     */
    Time& setFromMicroseconds(uint64_t microseconds) noexcept;

    /** @brief Setzt die Uhrzeit in Sekunden
     *
     * Mit dieser Funktion wird die Uhrzeit auf den angegebenen Wert in Sekunden gesetzt. Der Wert wird modulo 86400 genommen, um
     * sicherzustellen, dass er innerhalb eines Tages liegt.
     *
     * @param seconds Uhrzeit in Sekunden
     * @return Referenz auf das aktuelle Objekt
     */
    Time& setFromSeconds(uint32_t seconds) noexcept;

    /** @brief Setzt die Uhrzeit
     *
     * Mit dieser Funktion wird die Uhrzeit auf den angegebenen Wert gesetzt.
     *
     * @param hour Stunde (0-23)
     * @param minute Minute (0-59)
     * @param second Sekunde (0-59)
     * @param microseconds Mikrosekunden (0-999999)
     * @return Referenz auf das aktuelle Objekt
     * @exception IllegalArgumentException: Wird geworfen, wenn einer der Parameter außerhalb des gültigen Bereichs liegt.
     */
    Time& set(uint8_t hour, uint8_t minute, uint8_t second, uint32_t microseconds = 0);

    /** @brief Setzt die Uhrzeit aus einem String
     *
     * Mit dieser Funktion wird die Uhrzeit aus dem angegebenen String gesetzt. Der String muss im Format "hh:mm:ss[.micses]" vorliegen.
     * Stunden, Minuten und Sekunden können ein- oder zweistellig sein. Statt Doppelpunkt kann auch Komma, Punkt oder Minus als Trennzeichen
     * verwendet werden.
     *
     * @param time String mit der Uhrzeit
     * @return Referenz auf das aktuelle Objekt
     * @exception IllegalArgumentException: Wird geworfen, wenn der String kein gültiges Zeitformat hat.
     */
    Time& set(const String& time);

    Time& setHours(uint8_t hour);

    Time& setMinutes(uint8_t minute);

    Time& setSeconds(uint8_t second);

    Time& setMicroseconds(uint32_t microseconds);

    /** @brief Setzt die Uhrzeit aus einem anderen Time-Objekt
     *
     * Mit dieser Funktion wird die Uhrzeit aus einem anderen Time-Objekt übernommen.
     *
     * @param other Referenz auf das andere Time-Objekt
     * @return Referenz auf das aktuelle Objekt
     */
    Time& operator=(const Time& other) noexcept;

    /** @brief Setzt die Uhrzeit aus einem anderen Time-Objekt (Move)
     *
     * Mit dieser Funktion wird die Uhrzeit aus einem anderen Time-Objekt übernommen. Das andere Objekt wird dabei auf 0 gesetzt.
     *
     * @param other Rvalue-Referenz auf das andere Time-Objekt
     * @return Referenz auf das aktuelle Objekt
     */
    Time& operator=(Time&& other) noexcept;

    /** @brief Gibt die Uhrzeit als String zurück
     *
     * Mit dieser Funktion wird die Uhrzeit als String im Format "hh:mm:ss.micses" zurückgegeben.
     * Stunden, Minuten und Sekunden werden immer zweistellig ausgegeben, Mikrosekunden immer sechsstellig.
     *
     * @return Uhrzeit als String
     */
    String toString() const;

    /** @brief Gibt die Uhrzeit in einem bestimmten Format zurück
     *
     * Mit dieser Funktion wird die Uhrzeit in einem bestimmten Format zurückgegeben. Das Format kann Platzhalter für Stunden, Minuten,
     * Sekunden und Mikrosekunden enthalten. Die unterstützten Platzhalter sind:
     * - %H: Stunde (00-23)
     * - %I: Stunde (00-11)
     * - %M: Minute (00-59)
     * - %S: Sekunde (00-59)
     * - %f: Millisekunden (000-999)
     * - %u: Mikrosekunden als sechstellige Zahl (000000 bis 999999)
     *
     * @param format String mit dem gewünschten Format
     * @return Uhrzeit als String im angegebenen Format
     */
    String format(const String& format) const;

    /** @brief Gibt die Uhrzeit als String zurück
     *
     * Mit dieser Funktion wird die Uhrzeit als String im Format "hh:mm:ss.micses" zurückgegeben.
     * Stunden, Minuten und Sekunden werden immer zweistellig ausgegeben, Mikrosekunden immer sechsstellig.
     *
     * @return Uhrzeit als String
     */
    operator String() const
    {
        return toString();
    }

    /** @brief Gibt die Stunde zurück
     *
     * Mit dieser Funktion wird die Stunde der Uhrzeit zurückgegeben.
     *
     * @return Stunde (0-23)
     */
    uint8_t hours() const noexcept
    {
        return hh;
    }

    /** @brief Gibt die Minute zurück
     *
     * Mit dieser Funktion wird die Minute der Uhrzeit zurückgegeben.
     *
     * @return Minute (0-59)
     */
    uint8_t minutes() const noexcept
    {
        return ii;
    }

    /** @brief Gibt die Sekunde zurück
     *
     * Mit dieser Funktion wird die Sekunde der Uhrzeit zurückgegeben.
     *
     * @return Sekunde (0-59)
     */
    uint8_t seconds() const noexcept
    {
        return ss;
    }

    /** @brief Gibt die Mikrosekunden zurück
     *
     * Mit dieser Funktion werden die Mikrosekunden der Uhrzeit zurückgegeben.
     *
     * @return Mikrosekunden (0-999999)
     */
    uint32_t microseconds() const noexcept
    {
        return us;
    }

    /** @brief Vergleichsoperatoren
     *
     * Mit diesen Operatoren können zwei Time-Objekte miteinander verglichen werden.
     */
    inline bool operator==(const Time& other) const noexcept
    {
        return toMicroseconds() == other.toMicroseconds();
    }

    /** @brief Vergleichsoperatoren
     *
     * Mit diesen Operatoren können zwei Time-Objekte miteinander verglichen werden.
     */

    inline bool operator!=(const Time& other) const noexcept
    {
        return toMicroseconds() != other.toMicroseconds();
    }

    /** @brief Vergleichsoperatoren
     *
     * Mit diesen Operatoren können zwei Time-Objekte miteinander verglichen werden.
     */
    inline bool operator<(const Time& other) const noexcept
    {
        return toMicroseconds() < other.toMicroseconds();
    }

    /** @brief Vergleichsoperatoren
     *
     * Mit diesen Operatoren können zwei Time-Objekte miteinander verglichen werden.
     */
    inline bool operator<=(const Time& other) const noexcept
    {
        return toMicroseconds() <= other.toMicroseconds();
    }

    /** @brief Vergleichsoperatoren
     *
     * Mit diesen Operatoren können zwei Time-Objekte miteinander verglichen werden.
     */
    inline bool operator>(const Time& other) const noexcept
    {
        return toMicroseconds() > other.toMicroseconds();
    }

    /** @brief Vergleichsoperatoren
     *
     * Mit diesen Operatoren können zwei Time-Objekte miteinander verglichen werden.
     */
    inline bool operator>=(const Time& other) const noexcept
    {
        return toMicroseconds() >= other.toMicroseconds();
    }

    /** @brief Gibt die aktuelle Uhrzeit zurück
     *
     * Mit dieser Funktion wird die aktuelle Uhrzeit zurückgegeben. Die Uhrzeit wird in der lokalen Zeitzone ermittelt.
     *
     * @return Aktuelle Uhrzeit
     */
    static Time now();

    /** @brief Gibt die aktuelle Uhrzeit in UTC zurück
     *
     * Mit dieser Funktion wird die aktuelle Uhrzeit in UTC zurückgegeben.
     *
     * @return Aktuelle Uhrzeit in UTC
     */
    static Time utcNow();
};

inline std::ostream& operator<<(std::ostream& s, const Time& t)
{
    pplib::String tmp = t.toString();
    return s.write(tmp.c_str(), tmp.length());
}

} // namespace pplib

#endif /* PPLIB_TYPES_TIME_H_ */