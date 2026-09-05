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

#ifndef PPLIB_TYPES_TIMEZONE_H_
#define PPLIB_TYPES_TIMEZONE_H_

#include <stdint.h>
#include <ostream>
#include "pplib/types/string.h"

namespace pplib
{

/** @class TimeZone
 * @brief Datentyp für Zeitzonen
 * @ingroup PPLGroupDataTypes
 *
 * Diese Klasse stellt Methoden zum Verwalten und Formatieren von Zeitzonen bereit.
 * Der Offset zu UTC wird ausschließlich durch die Minutenangabe bestimmt, der Name der Zeitzone spielt keine Rolle.
 *
 * @note Der Name der Zeitzone ist optional und kann z. B. "UTC", "CET", "CEST" oder "Europe/Berlin" sein.
 */
class TimeZone
{
private:
    int16_t offset_minutes = 0; //!< Offset zu UTC in Minuten (-720 bis +840)
    String tz_name;             //!< Optional: "UTC", "CET", "CEST", "Europe/Berlin"

public:
    /** @brief Standardkonstruktor, erstellt eine UTC-Zeitzone */
    TimeZone() noexcept;

    /** @brief Konstruktor mit Offset in Minuten und optionalem Zeitzonennamen
     *
     * @param offset_min Offset zu UTC in Minuten (-720 bis +840)
     * @param name Optionaler Name der Zeitzone (z. B. "UTC", "CET", "CEST", "Europe/Berlin")
     * @exception IllegalArgumentException Wenn der Offset ungültig ist (-720 bis +840)
     */
    explicit TimeZone(int offset_min, const String& name = String())
        : tz_name(name)
    {
        setOffsetMinutes(offset_min);
    }

    /** @brief Konstruktor mit Stunden- und Minutenangabe und optionalem Zeitzonennamen
     *
     * @param hours Stundenanteil des Offsets (-12 bis +14)
     * @param minutes Minutenanteil des Offsets (-59 bis 59)
     * @param name Optionaler Name der Zeitzone (z. B. "UTC", "CET", "CEST", "Europe/Berlin")
     * @exception IllegalArgumentException Wenn der Offset ungültig ist (-12 bis +14 Stunden, -59 bis +59 Minuten)
     */
    TimeZone(int hours, int minutes, const String& name = String())
        : tz_name(name)
    {
        setOffset(hours, minutes);
    }

    /** @brief Setzt den Offset in Minuten
     *
     * @param offset_min Offset zu UTC in Minuten (-720 bis +840)
     * @return Referenz auf das aktuelle TimeZone-Objekt
     * @exception IllegalArgumentException Wenn der Offset ungültig ist (-720 bis +840)
     */
    TimeZone& setOffsetMinutes(int offset_min);

    /** @brief Setzt den Offset in Sekunden
     *
     * @param offset_seconds Offset zu UTC in Sekunden (-43200 bis +50400)
     * @return Referenz auf das aktuelle TimeZone-Objekt
     * @note Der Offset wird intern in Minuten umgerechnet. Sekunden werden verworfen.
     */
    TimeZone& setOffsetSeconds(int offset_seconds)
    {
        return setOffsetMinutes(offset_seconds / 60);
    }

    /** @brief Setzt den Offset in Stunden und Minuten
     *
     * @param hours Stundenanteil des Offsets (-12 bis +14)
     * @param minutes Minutenanteil des Offsets (-59 bis 59)
     * @return Referenz auf das aktuelle TimeZone-Objekt
     * @exception IllegalArgumentException Wenn der Offset ungültig ist (-12 bis +14 Stunden, -59 bis +59 Minuten)
     */
    TimeZone& setOffset(int hours, int minutes);

    /** @brief Setzt den Offset in Stunden und Minuten (Inline-Version)
     *
     * @param hours Stundenanteil des Offsets (-12 bis +14)
     * @param minutes Minutenanteil des Offsets (-59 bis 59)
     * @return Referenz auf das aktuelle TimeZone-Objekt
     * @exception IllegalArgumentException Wenn der Offset ungültig ist (-12 bis +14 Stunden, -59 bis +59 Minuten)
     */
    inline TimeZone& set(int hours, int minutes)
    {
        return setOffset(hours, minutes);
    }

    /** @brief Setzt den Namen der Zeitzone
     *
     * @param name Name der Zeitzone (z. B. "UTC", "CET", "CEST", "Europe/Berlin")
     * @note Der Name der Zeitzone ist ein Freitext-Feld, welches nicht in die Berechnung der
     * Zeitzone einfließt.
     */
    void setName(const String& name)
    {
        tz_name = name;
    }

    // Fabriken

    /** @brief Fabrikmethoden für TimeZone-Objekte
     *
     * @return Ein neues TimeZone-Objekt
     */
    static TimeZone utc() noexcept
    {
        return TimeZone(0, "UTC");
    }

    /** @brief Erstellt ein TimeZone-Objekt aus einer Stundenangabe
     *
     * @param hours Stundenanteil des Offsets (-12 bis +14)
     * @return Ein neues TimeZone-Objekt
     * @exception IllegalArgumentException Wenn der Offset ungültig ist (-12 bis +14 Stunden)
     */
    static TimeZone fromHours(int hours) noexcept
    {
        return TimeZone(hours * 60);
    }

    /** @brief Erstellt ein TimeZone-Objekt aus Stunden- und Minutenangabe
     *
     * @param hours Stundenanteil des Offsets (-12 bis +14)
     * @param minutes Minutenanteil des Offsets (-59 bis 59)
     * @return Ein neues TimeZone-Objekt
     * @exception IllegalArgumentException Wenn der Offset ungültig ist (-12 bis +14 Stunden, -59 bis +59 Minuten)
     */
    static TimeZone fromHoursAndMinutes(int hours, int minutes) noexcept
    {
        int sign = (hours < 0 || minutes < 0) ? -1 : 1;
        return TimeZone(sign * (abs(hours) * 60 + abs(minutes)));
    }

    /** @brief Erstellt ein TimeZone-Objekt aus einem String
     *
     * @param str String-Darstellung des Offsets (z. B. "+02:00", "+0200", "Z", "UTC", "[]")
     * @return Ein neues TimeZone-Objekt
     * @exception IllegalArgumentException Wenn der String nicht geparst werden kann
     */
    static TimeZone fromString(const String& str);

    /** @brief Erstellt ein TimeZone-Objekt aus der lokalen Zeit
     *
     * @return Ein neues TimeZone-Objekt
     */
    static TimeZone fromLocalTime() noexcept;

    /** @brief Erstellt ein TimeZone-Objekt aus der Epoch-Zeit
     *
     * @param t Epoch-Zeit (Sekunden seit 1970-01-01 00:00:00 UTC)
     * @return Ein neues TimeZone-Objekt
     */
    static TimeZone fromEpoch(time_t t) noexcept;

    /** @brief Gibt den Offset in Minuten zurück
     *
     * @return Offset in Minuten
     */
    int16_t offsetMinutes() const noexcept
    {
        return offset_minutes;
    }

    /** @brief Gibt den Offset in Sekunden zurück
     *
     * @return Offset in Sekunden
     */
    int32_t offsetSeconds() const noexcept
    {
        return offset_minutes * 60;
    }

    /** @brief Gibt den Namen der Zeitzone zurück
     *
     * @return Name der Zeitzone
     */
    const String& name() const noexcept
    {
        return tz_name;
    }

    /** @brief Prüft, ob die Zeitzone UTC ist
     *
     * @return true, wenn die Zeitzone UTC ist, sonst false
     */
    inline bool isUTC() const noexcept
    {
        return offset_minutes == 0;
    }

    /** @brief Vergleichsoperator: Gleichheit
     *
     * @param other Andere Zeitzone
     * @return true, wenn die Zeitzonen gleich sind, sonst false
     */
    inline bool operator==(const TimeZone& other) const noexcept
    {
        return offset_minutes == other.offset_minutes;
    }

    /** @brief Vergleichsoperator: Ungleichheit
     *
     * @param other Andere Zeitzone
     * @return true, wenn die Zeitzonen ungleich sind, sonst false
     */
    inline bool operator!=(const TimeZone& other) const noexcept
    {
        return offset_minutes != other.offset_minutes;
    }

    /** @brief Vergleichsoperator: Kleiner als
     *
     * @param other Andere Zeitzone
     * @return true, wenn diese Zeitzone kleiner ist als die andere, sonst false
     */
    inline bool operator<(const TimeZone& other) const noexcept
    {
        return offset_minutes < other.offset_minutes;
    }

    /** @brief Vergleichsoperator: Kleiner oder gleich
     *
     * @param other Andere Zeitzone
     * @return true, wenn diese Zeitzone kleiner oder gleich der anderen ist, sonst false
     */
    inline bool operator<=(const TimeZone& other) const noexcept
    {
        return offset_minutes <= other.offset_minutes;
    }

    /** @brief Vergleichsoperator: Größer als
     *
     * @param other Andere Zeitzone
     * @return true, wenn diese Zeitzone größer ist als die andere, sonst false
     */
    inline bool operator>(const TimeZone& other) const noexcept
    {
        return offset_minutes > other.offset_minutes;
    }

    /** @brief Vergleichsoperator: Größer oder gleich
     *
     * @param other Andere Zeitzone
     * @return true, wenn diese Zeitzone größer oder gleich der anderen ist, sonst false
     */
    inline bool operator>=(const TimeZone& other) const noexcept
    {
        return offset_minutes >= other.offset_minutes;
    }

    /** @brief Gibt die Zeitzone als String zurück
     *
     * @param colon Gibt an, ob ein Doppelpunkt im Format enthalten sein soll (z. B. "+02:00" vs "+0200")
     * @return Zeitzone als String
     */
    String toString(bool colon = true) const;
};

inline std::ostream& operator<<(std::ostream& s, const TimeZone& t)
{
    pplib::String tmp = t.toString();
    return s.write(tmp.c_str(), tmp.length());
}

} // namespace pplib

#endif // PPLIB_TYPES_TIMEZONE_H_