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

#ifndef PPLIB_TYPES_DATETIME_H_
#define PPLIB_TYPES_DATETIME_H_

#include <stdint.h>
#include "pplib/types/string.h"
#include "pplib/types/date.h"
#include "pplib/types/time.h"

namespace pplib
{

class String;

/// Eine Struktur zum Erfassen von Uhrzeit und Datum
typedef struct tagTime
{
    int64_t epoch;
    int16_t year;
    int8_t month;
    int8_t day;
    int8_t hour;
    int8_t min;
    int8_t sec;
    int8_t day_of_week;
    int16_t day_of_year;
    int8_t gmt_offset;
    bool have_gmt_offset;
    bool summertime;
} PPLTIME;

/// Datentyp für Unix-Timestamps in 64 Bit
typedef uint64_t ppl_time_t;

/** @class DateTime
 * @ingroup PPLGroupDataTypes
 * @ingroup PPLGroupDateTime
 * @brief Datenobjekt zum Speichern von Datum und Uhrzeit
 *
 * Dies ist eine Klasse zum Speichern von Datum und Uhrzeit. Mit den Funktionen
 * \ref DateTime::set(const String &datetime) "set", \ref DateTime::setDate "setDate"
 * und \ref DateTime::setTime "setTime" können Datum und/oder Uhrzeit gesetzt werden,
 * mit \ref DateTime::get "get", \ref DateTime::getDate "getDate",
 * \ref DateTime::getTime "getTime" und \ref DateTime::getISO8601 "getISO8601" kann der Wert ausgelesen werden.
 * Alternativ kann mit \ref DateTime::setTime_t "setTime_t" und \ref DateTime::time_t "time_t"
 * ein Unix-Timestamp gesetzt oder gelesen werden (Sekunden seit 1970),
 * oder mit \ref DateTime::setLongInt "setLongInt" und \ref DateTime::longInt "longInt"
 * ein 64-Bit Wert gesetzt oder gelesen werden, in dem die einzelnen Bestandteile bitweise kodiert sind.
 *
 */
class DateTime
{
private:
    Date my_date; //!< Datumsobjekt
    Time my_time; //!< Uhrzeitobjekt

public:
    /** @brief Konstruktor mit Initialisierung auf 0
     *
     * Mit diesem Konstruktor ohne Parameter wird der Wert der Datumsklasse auf 0 gesetzt. Die Funktion
     * DateTime::isEmpty "isEmpty" würde \c true zurückliefern.
     */
    DateTime() noexcept = default;

    /** @brief Konstruktor mit Datumsinitialisierung aus einem String
     *
     * Über diesen Konstruktor wird ein DateTime Objekt anhand des im String \p datetime enthaltenen
     * Datums und Uhrzeit erstellt. Die unterstützten Formate sind in der Funktion
     * \ref DateTime::set(const String &datetime) "set" beschrieben.
     *
     * @param[in] datetime String mit Datum und Uhrzeit
     *
     * @exception IllegalArgumentException: Wird geworfen, wenn der String \p datetime
     * ein ungültiges oder unbekanntes Datumsformat hat.
     * Ausnahmen: Ist der String leer oder enthält nur den
     * Buchstaben "T" oder den Wert "0" wird keine Exception geworfen, sondern der Datumswert auf 0 gesetzt.
     */
    explicit DateTime(const String& datetime)
    {
        set(datetime);
    }

    /** @brief Konstruktor mit Angabe von Unix-Timestamp
     *
     * Mit dieser Konstruktor werden Datum und Uhrzeit aus einem Unix-Timestamp übernommen (Sekunden seit 1970),
     * wie ihn Beispielsweise die C-Funktion "time()" zurückliefert. Es ist daher nicht möglich ein Datum vor
     * 1970 zu setzen.
     *
     * @param t 64-Bit Integer mit den Sekunden seit 1970.
     */
    explicit DateTime(uint64_t epoch_seconds, uint32_t microseconds = 0)
    {
        setEpoch(epoch_seconds);
        my_time.setMicroseconds(microseconds);
    }

    DateTime(const DateTime& other) noexcept = default;
    DateTime(DateTime&& other) noexcept = default;

    DateTime(const Date& date, const Time& time) noexcept
        : my_date(date),
          my_time(time)
    {
    }
    DateTime(const Date& date) noexcept
        : my_date(date),
          my_time()
    {
    }

    inline const Date& date() const noexcept
    {
        return my_date;
    }
    inline const Time& time() const noexcept
    {
        return my_time;
    }
    inline Date& date() noexcept
    {
        return my_date;
    }
    inline Time& time() noexcept
    {
        return my_time;
    }

    /** @brief Datum aus Unix-Timestamp übernehmen
     *
     * Mit dieser Funktion werden Datum und Uhrzeit aus einem Unix-Timestamp übernommen (Sekunden seit 1970),
     * wie ihn Beispielsweise die C-Funktion "time()" zurückliefert. Es ist daher nicht möglich ein Datum vor
     * 1970 zu setzen.
     *
     * @param t 64-Bit Integer mit den Sekunden seit 1970.
     */
    DateTime& setTime_t(uint64_t t);

    /** @brief Datum aus Unix-Timestamp übernehmen
     *
     * Mit dieser Funktion werden Datum und Uhrzeit aus einem Unix-Timestamp übernommen (Sekunden seit 1970),
     * wie ihn Beispielsweise die C-Funktion "time()" zurückliefert. Es ist daher nicht möglich ein Datum vor
     * 1970 zu setzen.
     *
     * @param t 64-Bit Integer mit den Sekunden seit 1970.
     * @see https://de.wikipedia.org/wiki/Unixzeit
     */
    DateTime& setEpoch(uint64_t epoch_seconds);

    /** @brief Datum aus einem 64-Bit-Integer übernehmen
     *
     * Mit dieser Funktion werden Datum, Uhrzeit und Millisekunden aus einem Long Integer (64 Bit) übernommen,
     * wie ihn die Funktion CDateTime::longInt zurückgibt. Der Aufbau des Integer-Wertes ist intern und kann
     * sich von Version zu Version ändern.
     *
     * @param i 64-Bit Integer
     */
    DateTime& setLongInt(uint64_t i);

    /** @brief Datum anhand eines Strings setzen
     *
     * Mit dieser Funktion wird das Datum anhand des Strings \p datetime gesetzt. Dabei versucht die Funktion
     * anhand mehrerer Regular Expressions zu erkennen, in welchem Format die Datumsangabe vorliegt. Es werden
     * folgende Formate erkannt:
     *
     *   - yyyy-mm-dd hh:ii:ss[.mms]
     *
     *   - yyyy.mm.dd hh:ii:ss[.mms]
     *
     *   - dd-mm-yyyy hh:ii:ss[.mms]
     *
     *   - dd.mm.yyyy hh:ii:ss[.mms]
     *
     *   - yyyy-mm-ddThh:ii:ss[.mms]+oo:oo (ISO 8601-Format)
     *
     *   - yyyy-mm-dd
     *
     *   - yyyy.mm.dd
     *
     *   - dd-mm-yyyy
     *
     *   - dd.mm.yyyy
     *
     *   - T: wird als Leerstring interpretiert und setzt das Datum auf 0
     *
     *   - Leerstring: setzt das Datum auf 0
     *
     * @param[in] datetime String mit dem zu setzenden Datum und optional der Uhrzeit
     * @exception IllegalArgumentException: Wird geworfen, wenn der String \p datetime
     * ein ungültiges oder unbekanntes Datumsformat hat.
     * Ausnahmen: Ist der String leer oder enthält nur den
     * Buchstaben "T" oder den Wert "0" wird keine Exception geworfen, sondern der Datumswert auf 0 gesetzt.
     *
     * @par Legende:
     * - yyyy: 4-Stellige Jahreszahl. Muss zwingend 4-stellig sein, da sonst nicht erkannt wird ob die Jahreszahl
     *   an erster oder dritter Stelle steht. Jahreszahlen < 1000 müssen daher mit führenden Nullen aufgefüllt
     *   werden, z.B. "0500" statt "500". Es werden keine negativen Jahreszahlen unterstützt.
     * - mm: Monatszahl zwischen 1 und 12. Kann ein- oder zweistellig sein
     * - dd: Monatstag zwischen 1 und 31, kann ein- oder zweistellig sein.
     * - hh: Stunden zwischen 0 und 23, kann ein- oder zweistellig sein
     * - ii: Minuten zwischen 0 und 59, kann ein- oder zweistellig sein
     * - ss: Sekunden zwischen 0 und 59, kann ein- oder zweistellig sein
     * - mms: Mikrosekunden: Ein bis sechsstelliger Wert, der die Mikrosekunden angibt.
     * @par
     * Bei der Datumsangabe kann als Trennzeichen wahlweise Punkt oder Minus verwendet werden. Es muss mindestens ein
     * vollständiges Datum angegeben werden und optional eine vollständige Uhrzeit (hh:ii:ss), wobei die Millisekunden
     * optional sind.
     *
     */
    DateTime& set(const String& datetime);

    DateTime& set(uint64_t epoch_seconds, uint32_t microseconds = 0)
    {
        setEpoch(epoch_seconds);
        my_time.setMicroseconds(microseconds);
        return *this;
    }

    /** @brief Datum aus einem anderen DateTime-Objekt übernehmen
     *
     * Mit dieser Funktion wird der Wert eines anderen DateTime-Wertes übernommen.
     *
     * @param[in] other Referenz auf den zu kopierenden DateTime-Wert
     * @return Gibt eine Referenz auf den DateTime-Wert zurück
     */
    inline DateTime& set(const DateTime& other) noexcept
    {
        my_date = other.my_date;
        my_time = other.my_time;
        return *this;
    }

    /** @brief Datum und Uhrzeit aus unterschiedlichen Strings importieren
     *
     * Mit dieser Funktion kann das Datum und die Uhrzeit aus zwei unterschiedlichen Strings
     * übernommen werden. Dazu werden beide Strings einfach mit Space getrennt hintereinander
     * gehangen und dann die \ref DateTime::set(const String &datetime) "set-Funktion"
     * aufgerufen, die nur einen String-Parameter erwartet.
     *
     * @param[in] date Referenz auf den String mit dem Datum. Dieses kann folgende Formate haben:
     * - yyyy-mm-dd
     * - yyyy.mm.dd
     * - dd-mm-yyyy
     * - dd.mm.yyyy
     * - Die Jahreszahl muss 4-stellig sein, Tag und Monat können ein- oder zweistellig sein. Statt Punkt oder Minus
     *   kann auch noch Doppelpunkt oder Komma als Trennzeichen verwendet werden.
     * @param[in] time Referenz auf den String mit der Uhrzeit. Diese muss folgendes Format haben:
     * - hh:ii:ss[.mms]
     * - Stunde, Minute und Sekunde können ein- oder zweistellig sein, Statt Doppelpunkt kann auch Komma, Punkt oder
     *   Minus als Trennzeichen verwendet werden. Die
     * @exception IllegalArgumentException: Wird geworfen, wenn der String \p datetime
     * ein ungültiges oder unbekanntes Datumsformat hat.
     * Ausnahmen: Ist der String leer oder enthält nur den
     * Buchstaben "T" oder den Wert "0" wird keine Exception geworfen, sondern der Datumswert auf 0 gesetzt.
     */
    inline DateTime& set(const String& date, const String& time)
    {
        my_date.set(date);
        my_time.set(time);
        return *this;
    }

    /** @brief Datum und Uhrzeit aus unterschiedlichen Objekten übernehmen
     *
     * Mit dieser Funktion kann das Datum und die Uhrzeit aus zwei unterschiedlichen Objekten übernommen werden.
     *
     * @param[in] date Referenz auf ein Date-Objekt mit dem Datum
     * @param[in] time Referenz auf ein Time-Objekt mit der Uhrzeit
     * @return Gibt eine Referenz auf den DateTime-Wert zurück
     */
    inline DateTime& set(const Date& date, const Time& time)
    {
        my_date = date;
        my_time = time;
        return *this;
    }

    /** @brief Datum aus PPLTIME-Struktur übernehmen
     *
     * Mit dieser Funktion wird Datum und Zeit aus einer PPLTIME-Struktur übernommen.
     *
     * @param[in] t Referenz auf eine PPLTIME-Struktur
     *
     * @attention
     * Gegenwärtig werden Werte ausserhalb des Gültigkeitsbereiches abgeschnitten! Aus dem Monat 0 oder -10 würde 1
     * werden, aus 13 oder 12345 würde 12 werden.
     */
    DateTime& set(const PPLTIME& t);

    /** @brief Datum setzen, Uhrzeit bleibt unverändert
     *
     * Mit dieser Funktion wird nur das Datum der Klasse verändert, die Uhrzeit bleibt erhalten.
     *
     * @param[in] date Referenz auf den String mit dem zu setzenden Datum. Das Format wird bei der
     * @ref DateTime::set(const String &date, const String &time) "set-Funktion" genauer beschrieben.
     * @exception IllegalArgumentException: Wird geworfen, wenn der String \p datetime
     * ein ungültiges oder unbekanntes Datumsformat hat.
     * Ausnahmen: Ist der String leer oder enthält nur den
     * Buchstaben "T" oder den Wert "0" wird keine Exception geworfen, sondern der Datumswert auf 0 gesetzt.
     */
    inline DateTime& setDate(const String& date)
    {
        my_date.set(date);
        return *this;
    }

    /** @brief Datum setzen, Uhrzeit bleibt unverändert
     *
     * Mit dieser Funktion wird nur das Datum der Klasse verändert, die Uhrzeit bleibt erhalten.
     *
     * @param[in] date Referenz auf ein Date-Objekt mit dem zu setzenden Datum.
     */
    DateTime& setDate(const Date& date)
    {
        my_date = date;
        return *this;
    }

    /** @brief Uhrzeit setzen, Datum bleibt unverändert
     *
     * Mit dieser Funktion wird nur die Uhrzeit der Klasse verändert, das Datum bleibt erhalten.
     *
     * @param[in] time Referenz auf den String mit der zu setzenden Uhrzeit. Das Format wird bei der
     * \ref DateTime::set(const String &date, const String &time) "set-Funktion" genauer beschrieben.
     * \exception IllegalArgumentException: Wird geworfen, wenn der String \p datetime
     * ein ungültiges oder unbekanntes Datumsformat hat.
     * Ausnahmen: Ist der String leer oder enthält nur den
     * Buchstaben "T" oder den Wert "0" wird keine Exception geworfen, sondern der Datumswert auf 0 gesetzt.
     */
    inline DateTime& setTime(const String& time)
    {
        my_time.set(time);
        return *this;
    }

    /** @brief Uhrzeit setzen, Datum bleibt unverändert
     *
     * Mit dieser Funktion wird nur die Uhrzeit der Klasse verändert, das Datum bleibt erhalten.
     *
     * @param[in] time Referenz auf ein Time-Objekt mit der zu setzenden Uhrzeit.
     */
    DateTime& setTime(const Time& time)
    {
        my_time = time;
        return *this;
    }

    /** @brief Datum und Uhrzeit anhand einzelner Integer-Wert setzen
     *
     * Mit dieser Funktion wird das Datum anhand einzelner Integer-Werten gesetzt.
     *
     * @param[in] year Jahreszahl zwischen 0 und 9999
     * @param[in] month Monat zwischen 1 und 12
     * @param[in] day Tag zwischen 1 und 31
     * @param[in] hour Stunde zwischen 0 und 23. Optionaler Wert, Default ist 0.
     * @param[in] minute Minute zwischen 0 und 59. Optionaler Wert, Default ist 0.
     * @param[in] sec Sekunde zwischen 0 und 59. Optionaler Wert, Default ist 0.
     * @param[in] usec Mikrosekunde zwischen 0 und 999999. Optionaler Wert, Default ist 0.
     * @exception IllegalArgumentException: Wird geworfen, wenn einer der Parameter außerhalb des gültigen Bereichs liegt.
     */
    inline DateTime& set(int year, int month, int day, int hour = 0, int minute = 0, int sec = 0, int usec = 0)
    {
        my_date.set(day, month, year);
        my_time.set(hour, minute, sec, usec);
        return *this;
    }

    /** @brief Aktuelles Datum und Uhrzeit übernehmen
     *
     * Mit dieser Funktion wird die Variable auf das aktuelle Datum und die aktuelle Uhrzeit gesetzt.
     * Es gibt sie auch als statische Funktion \ref DateTime::currentTime "currentTime".
     */
    DateTime& setCurrentTime();

    /** @brief Datum auf 0 setzen
     *
     * Mit dieser Funktion wird der Datumswert der Klasse auf 0 gesetzt. Die Funktion
     * \ref DateTime::isEmpty "isEmpty" würde \c true zurückliefern. Die Klasse wird somit
     * wieder in den Ausgangszustand versetzt.
     */
    inline void clear() noexcept
    {
        my_date.clear();
        my_time.clear();
    }

    /** @brief Prüfen ob Datum gesetzt ist
     *
     * Mit dieser Funktion kann geprüft werden, ob die Klasse ein Datum enthält. Ist der Wert 0, wird \c false zurückgegeben,
     * andernfalls \c true.
     *
     * @return \c true, wenn die Klasse ein Datum enthält, sonst \c false
     */
    inline bool notEmpty() const noexcept
    {
        return my_date.notEmpty();
    }

    /** @brief Prüfen ob Datum nicht gesetzt ist
     *
     * Mit dieser Funktion kann geprüft werden, ob die Klasse ein Datum enthält. Ist der Wert 0, wird \c true zurückgegeben,
     * andernfalls \c false.
     *
     * @return \c true, wenn die Klasse kein Datum enthält, sonst \c false
     */
    inline bool isEmpty() const noexcept
    {
        return my_date.isEmpty();
    }

    /** @brief Prüfen ob Jahr ein Schaltjahr ist
     *
     * Mit dieser Funktion kann geprüft werden, ob das in der Klasse gespeicherte Jahr ein Schaltjahr ist.
     *
     * @return \c true, wenn es sich um ein Schaltjahr handelt, sonst \c false
     */
    inline bool isLeapYear() const
    {
        return my_date.isLeapYear();
    }

    /** @brief Datum als String im angegebenen Format zurückgeben
     *
     * Datum als String im angegebenen Format zurückgeben
     *
     * @param[in] format Formatierungsstring. Wird dieser nicht angegeben, wird das Datum in folgendem Format zurückgegeben:
     * "%Y-%m-%d %H:%M:%S"
     *
     * @return String mit dem Datum im gewünschten Format
     *
     * Erlaubt sind folgende Formatzeichen:
     *   - %Y: Das Jahr als 4-stellige Angabe (z.B. 2010)
     *   - %y: Das Jahr als 2-stellige Angabe ohne Jahrhundert (z.B. 10)
     *   - %m: Der Monat als zweistellige Zahl (01 bis 12)
     *   - %d: Der Tag als zweistellige Zahl (01 bis 31)
     *   - %H: Stunde (00-23)
     *   - %I: Stunde (00-11)
     *   - %M: Minute (00-59)
     *   - %S: Sekunde (00-59)
     *   - %f: Millisekunden (000-999)
     *   - %u: Mikrosekunden als sechstellige Zahl (000000 bis 999999)
     */
    inline String format(const String& format = "%Y-%m-%d %H:%M:%S") const
    {
        return my_date.format(my_time.format(format));
    }

    /** @brief Datum als String im angegebenen Format zurückgeben
     *
     * Datum als String im angegebenen Format zurückgeben
     *
     * @param[in] format Formatierungsstring. Wird dieser nicht angegeben, wird das Datum in folgendem Format zurückgegeben:
     * "%Y-%m-%d %H:%M:%S"
     *
     * @return String mit dem Datum im gewünschten Format
     *
     * Erlaubt sind folgende Formatzeichen:
     *   - %Y: Das Jahr als 4-stellige Angabe (z.B. 2010)
     *   - %y: Das Jahr als 2-stellige Angabe ohne Jahrhundert (z.B. 10)
     *   - %m: Der Monat als zweistellige Zahl (01 bis 12)
     *   - %d: Der Tag als zweistellige Zahl (01 bis 31)
     *   - %H: Stunde (00-23)
     *   - %I: Stunde (00-11)
     *   - %M: Minute (00-59)
     *   - %S: Sekunde (00-59)
     *   - %f: Millisekunden (000-999)
     *   - %u: Mikrosekunden als sechstellige Zahl (000000 bis 999999)
     */
    inline String get(const String& format = "%Y-%m-%d %H:%M:%S") const
    {
        return my_date.format(my_time.format(format));
    }

    /** @brief Datum als String zurückgeben
     *
     * Diese Funktion ist identisch zu DateTime::get, hat aber einen anderen Default für den optionalen
     * Formatstring.
     *
     * @param[in] format Formatierungsstring. Wird dieser nicht angegeben, wird das Datum in folgendem Format zurückgegeben:
     * "%Y-%m-%d"
     *
     * @return String mit dem Datum im gewünschten Format
     *
     * @see
     * Siehe DateTime::get
     */
    inline String getDate(const String& format = "%Y-%m-%d") const
    {
        return my_date.format(format);
    }

    /** @brief Uhrzeit als String zurückgeben
     *
     * Diese Funktion ist identisch zu DateTime::get, hat aber einen anderen Default für den optionalen
     * Formatstring.
     *
     * @param[in] format Formatierungsstring. Wird dieser nicht angegeben, wird die Uhrzeit in folgendem Format zurückgegeben:
     * "%H-%M-%S"
     *
     * @return String mit der Uhrzeit im gewünschten Format
     *
     * \see
     * Siehe DateTime::get
     */
    inline String getTime(const String& format = "%H:%M:%S") const
    {
        return my_time.format(format);
    }

    /** @brief Datum als String im ISO8601-Format zurückgeben
     *
     * Diese Funktion gibt das Datum als String im ISO8601-Format zurück, das folgenden Aufbau hat:
     * "yyyy-mm-ddThh:ii:ss+zz:zz"
     *
     * Der Wert "+zz:zz" gibt den Offset zu GMT in Stunden und Minuten an und kann auch negativ sein.
     * Er wird allerdings nur ergänzt, wenn das Jahr >=1900 ist und das Betriebssystem den Wert "tm_gmtoff" in
     * seiner tm-Structure hat (siehe "man ctime").
     *
     * @return String mit dem Datum im ISO8601-Format
     */
    String getISO8601() const;

    /** @brief Datum als String im ISO8601-Format mit Millisekunden zurückgeben
     *
     * Diese Funktion gibt das Datum als String im ISO8601-Format mit Millisekunden zurück, das folgenden Aufbau hat:
     * "yyyy-mm-ddThh:ii:ss.xxx+zz:zz"
     *
     * Der Wert "xxx" stellt die Millisekunden dar.
     *
     * Der Wert "+zz:zz" gibt den Offset zu GMT in Stunden und Minuten an und kann auch negativ sein.
     * Er wird allerdings nur ergänzt, wenn das Jahr >=1900 ist und das Betriebssystem den Wert "tm_gmtoff" in
     * seiner tm-Structure hat (siehe "man ctime").
     *
     * @return String mit dem Datum im ISO8601-Format
     */
    String getISO8601withMsec() const;

    /** @brief Datum als String im ISO8601-Format mit Mikrosekunden zurückgeben
     *
     * Diese Funktion gibt das Datum als String im ISO8601-Format mit Mikrosekunden zurück, das folgenden Aufbau hat:
     * "yyyy-mm-ddThh:ii:ss.xxxxxx+zz:zz"
     *
     * Der Wert "xxxxxx" stellt die Mikrosekunden dar.
     *
     * Der Wert "+zz:zz" gibt den Offset zu GMT in Stunden und Minuten an und kann auch negativ sein.
     * Er wird allerdings nur ergänzt, wenn das Jahr >=1900 ist und das Betriebssystem den Wert "tm_gmtoff" in
     * seiner tm-Structure hat (siehe "man ctime").
     *
     * @return String mit dem Datum im ISO8601-Format
     */
    String getISO8601withUsec() const;

    /** @ingroup PPLGroupDateTime
     * @brief Datumstring nach RFC-822 (Mailformat) erzeugen
     *
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
     * @return String mit dem Datum im RFC-822-Format
     * \exception Exception::FunctionFailed Die Funktion wirft eine Exception, wenn die Datumsinformation in der PPLTIME-Struktur ungültig
     * ist.
     */
    String getRFC822Date() const;

    /** @brief Datum mit der Funktion strftime der Standard C Bibliothek formatieren
     *
     * Mit dieser Funktion wird das Datum mittels der Funktion strftime aus der Standard C Bibliothek
     * formatiert.
     *
     * @param[in] format Siehe Manpage zu strftime: man strftime
     * @return String im gewünschten Format
     */
    String strftime(const String& format) const;

    /** @brief Datum in Unix-Timestamp umrechnen
     *
     * Mit dieser Funktion wird das in der Variablen enthaltene Datum und Uhrzeit in einen
     * Unix-Timestamp umgerechnet (Sekunden seit 1970).
     *
     * @return Sekunden seit 1970 oder 0, wenn das Datum sich nicht umrechnen läßt, z.B. wenn das Jahr vor 1970 liegt.
     */
    inline uint64_t time_t() const
    {
        return epoch();
    }

    /** @brief Datum in Unix-Timestamp umrechnen
     *
     * Mit dieser Funktion wird das in der Variablen enthaltene Datum und Uhrzeit in einen
     * Unix-Timestamp umgerechnet (Sekunden seit 1970).
     *
     * @return Sekunden seit 1970 oder 0, wenn das Datum sich nicht umrechnen läßt, z.B. wenn das Jahr vor 1970 liegt.
     *
     * \see https://de.wikipedia.org/wiki/Unixzeit
     */

    uint64_t epoch() const;

    /** @brief Datum als 64-Bit-Integer auslesen
     *
     * Mit dieser Funktion werden Datum, Uhrzeit und Millisekunden als Long Integer (64 Bit) zurückgegeben,
     * wie er von der Funktion DateTime::setLongInt eingelesen werden kann. Der Aufbau des Integer-Wertes ist intern und kann
     * sich von Version zu Version ändern.
     *
     * @return 64-Bit-Integer mit dem Timestamp
     */
    uint64_t longInt() const;

    /** @brief Das Jahr als Integer auslesen
     *
     * @return Integer-Wert mit dem Jahr
     */
    inline int year() const noexcept
    {
        return my_date.year();
    }

    /** @brief Den Monat als Integer auslesen
     *
     * @return Integer-Wert mit dem Monat
     */
    inline int month() const noexcept
    {
        return my_date.month();
    }

    /** @brief Den Tag als Integer auslesen
     *
     * @return Integer-Wert mit dem Tag
     */
    inline int day() const noexcept
    {
        return my_date.day();
    }

    /** @brief Die Stunde als Integer auslesen
     *
     * @return Integer-Wert mit der Stunde
     */
    inline int hour() const noexcept
    {
        return my_time.hours();
    }

    /** @brief Die Minute als Integer auslesen
     *
     * @return Integer-Wert mit der Minute
     */
    inline int minute() const noexcept
    {
        return my_time.minutes();
    }

    /** @brief Die Sekunde als Integer auslesen
     *
     * @return Integer-Wert mit der Sekunde
     */
    inline int second() const noexcept
    {
        return my_time.seconds();
    }

    /** @brief Die Millisekunde als Integer auslesen
     *
     * @return Integer-Wert mit der Millisekunde
     */
    inline int millisecond() const noexcept
    {
        return my_time.microseconds() / 1000;
    }

    /** @brief Die Mikrosekunde als Integer auslesen
     *
     * @return Integer-Wert mit der Mikrosekunde
     */
    inline int microsecond() const noexcept
    {
        return my_time.microseconds();
    }

    /** @brief Die Wochennummer als Integer auslesen
     *
     * Diese Funktion berechnet anhand des Datums die Wochennummer innerhalb
     * des Jahres und gibt diese als Integer zurück. Die Zählweise entspricht der in
     * den USA, Australien und vielen weiteren Ländern, in der sich die Tradition des
     * Judentums, Christentums und Islams erhalten hat. Dabei gilt folgende Regel:
     * - Jeden Sonntag beginnt eine neue Kalenderwoche
     * - Am 1. Januar beginnt stets – unabhängig vom Wochentag – die 1. Kalenderwoche
     *
     * @return Integer-Wert mit dem Jahr
     */
    inline int week() const
    {
        return my_date.week();
    }

    /** @brief Die Wochennummer als Integer auslesen, Berechnung nach ISO 8601
     *
     * Diese Funktion berechnet anhand des Datums die Wochennummer innerhalb
     * des Jahres und gibt diese als Integer zurück. Die Zählweise richtet sich
     * dabei nach ISO 8601:
     *   - Jeden Montag und nur montags beginnt eine neue Kalenderwoche.
     *   - Die erste Kalenderwoche ist diejenige, die mindestens vier Tage des neuen Jahres enthält.
     *
     * Diese Zählweise ist die in Europa gebräuchliche.
     *
     * @see CDateTime::week
     *
     * @return Integer-Wert mit dem Jahr
     */
    inline int weekISO8601() const
    {
        return my_date.weekISO8601();
    }

    /** @brief Differenz in Sekunden
     *
     * Diese Funktion gibt die Differenz dieses DateTime zu dem angegebenen DateTime \p other in
     * Sekunden zurück. Liegt der Zeitpunkt von \p other vor diesem, ist der Rückgabewert negativ.
     *
     * Vor dem Vergleich werden beide Zeitwerte in UTC umgewandelt.
     * @param[in] other Zu vergleichender Zeitwert
     * @return Differenz in Sekunden
     */
    int64_t diffSeconds(const DateTime& other) const;

    /** @brief Differenz in Sekunden mit Toleranz vergleichen
     *
     * Mit dieser Funktion wird die Differenz des Zeitwerts dieses DateTime mit der angegebenen DateTime \p other
     * auf Sekundenbasis berechnet und anschließend mit der angegebenen Toleranz \p tolerance verglichen.
     *
     * @param[in] other Zu vergleichender Zeitwert
     * @param[in] tolerance Optionaler Wert, der die akzeptable Toleranz beider Werte in Sekunden angibt
     * @return Sind beide Zeitwerte identisch oder liegen im Bereich der angegebenen Toleranz, gibt die Funktion
     * 1 zurück, andernfalls 0. Es wird kein Fehlercode gesetzt.
     */
    bool compareSeconds(const DateTime& other, unsigned int tolerance = 0) const;

    /** @brief Datum aus einem String übernehmen
     *
     * Mit diesem Operator werden Datum und Uhrzeit aus dem String \p datetime übernommen.
     * Die unterstützten Formate sind in der Funktion
     * \ref DateTime::set(const String &datetime) "set" beschrieben.
     *
     * @param[in] datetime String mit Datum und Uhrzeit
     * @return Gibt eine Referenz auf den DateTime-Wert zurück
     *
     * \exception IllegalArgumentException: Wird geworfen, wenn der String \p datetime
     * ein ungültiges oder unbekanntes Datumsformat hat.
     * Ausnahmen: Ist der String leer oder enthält nur den
     * Buchstaben "T" oder den Wert "0" wird keine Exception geworfen, sondern der Datumswert auf 0 gesetzt.
     */
    DateTime& operator=(const String& datetime);

    /** @brief Datum aus einem anderen DateTime-Objekt übernehmen
     *
     * Mit diesem Operator wird der Wert eines anderen DateTime-Wertes übernommen.
     *
     * @param[in] other Referenz auf den zu kopierenden DateTime-Wert
     * @return Gibt eine Referenz auf den DateTime-Wert zurück
     */
    DateTime& operator=(const DateTime& other) noexcept = default;

    /** @brief Datum aus einem anderen DateTime-Objekt übernehmen
     *
     * Mit diesem Operator wird der Wert eines anderen DateTime-Wertes übernommen.
     *
     * @param[in] other Rvalue-Referenz auf den zu kopierenden DateTime-Wert
     * @return Gibt eine Referenz auf den DateTime-Wert zurück
     */
    DateTime& operator=(DateTime&& other) noexcept = default;

    /** @brief Jahreszahl auf Schaltjahr prüfen
     *
     * Mit dieser statischen Funktion kann geprüft werden, ob es sich bei dem angegebenen Jahr \p year um ein
     * Schaltjahr handelt oder nicht.
     *
     * @param[in] year Das zu prüfende Jahr
     * @return Liefert \c true zurück, wenn es sich um ein Schaltjahr handelt, andernfalls \c false.
     */
    inline static bool isLeapYear(int year)
    {
        return Date::isLeapYear(year);
    }

    /** @brief Aktuelles Datum zurückgeben
     *
     * Diese statische Funktion liefert das aktuelle Datum und die aktuelle Uhrzeit in Form einer
     * DateTime-Variablen zurück.
     * @return DateTime-Variable mit dem aktuellen Datum und Uhrzeit.
     */
    static DateTime currentTime();

    /** @brief Rueckgabe des Timestamps als String
     *
     * Liefert den Timestamp als String in folgendem Format zurück:
     * "yyyy-mm-dd hh:ii:ss.micses".
     * @return Datums-String
     */
    String toString() const;

    /** @brief Rueckgabe des Timestamps als String
     *
     * Liefert den Timestamp als String in folgendem Format zurück:
     * "yyyy-mm-dd hh:ii:ss.micses".
     * @return Datums-String
     */
    String toString(const String& format) const;

    PPLTIME toPPLTIME() const;

    /** @brief Operator, der einen String zurückliefert
     *
     * Dieser Operator liefert den Inhalt der Variablen als String in folgendem Format zurück:
     * "yyyy-mm-dd hh:ii:ss.micses".
     * @return Datums-String
     */
    operator String() const;

    inline bool operator<(const DateTime& other) const
    {
        return this->longInt() < other.longInt();
    }

    inline bool operator<=(const DateTime& other) const
    {
        return this->longInt() <= other.longInt();
    }

    inline bool operator==(const DateTime& other) const
    {
        return this->longInt() == other.longInt();
    }

    inline bool operator!=(const DateTime& other) const
    {
        return this->longInt() != other.longInt();
    }

    inline bool operator>=(const DateTime& other) const
    {
        return this->longInt() >= other.longInt();
    }

    inline bool operator>(const DateTime& other) const
    {
        return this->longInt() > other.longInt();
    }
};

} // namespace pplib

#endif /* PPLIB_TYPES_DATETIME_H_ */