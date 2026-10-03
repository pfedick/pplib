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
struct PPLTIME
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
};

/// Datentyp für Unix-Timestamps in 64 Bit

/**
 * @brief Datentyp für Unix-Timestamps in 64 Bit
 * @ingroup PPLGroupDateTime
 */
typedef uint64_t ppl_time_t;

// Time

/** @ingroup PPLGroupDateTime
 * @brief Liefert die aktuelle Unixtime
 *
 * Liefert die aktuelle Unix-Zeit als Return-Wert zurück
 * @return Gibt Sekunden seit 1.1.1970, 00:00 Uhr zurück (Unix-Timestamp)
 */
ppl_time_t GetTime();

/** @ingroup PPLGroupDateTime
 * @brief Wandelt Unix-Zeit in die lokale Zeit um
 *
 * Wandelt die angegebene Unix-Zeit in eine Struktur vom Typ PPLTIME um, die die lokale Zeit repräsentiert.
 *
 * @param tt Unix-Zeit in Sekunden seit 1.1.1970, 00:00 Uhr
 * @return Struktur vom Typ PPLTIME mit der lokalen Zeit
 */
PPLTIME LocalTime(ppl_time_t tt);

/** @ingroup PPLGroupDateTime
 * @brief Wandelt Unix-Zeit in die GMT/UTC-Zeit um
 *
 * Wandelt die angegebene Unix-Zeit in eine Struktur vom Typ PPLTIME um, die die GMT/UTC-Zeit repräsentiert.
 *
 * @param tt Unix-Zeit in Sekunden seit 1.1.1970, 00:00 Uhr
 * @return Struktur vom Typ PPLTIME mit der GMT/UTC-Zeit
 */
PPLTIME GMTime(ppl_time_t tt);

/** @ingroup PPLGroupDateTime
 * @brief Liefert die aktuelle Zeit in Sekunden mit Mikrosekundenauflösung
 *
 * @return Aktuelle Zeit in Sekunden als Gleitkommazahl, mit Mikrosekundenauflösung
 */
double GetMicrotime();

/** @ingroup PPLGroupDateTime
 * @brief Liefert die aktuelle Zeit in Millisekunden
 *
 * @return Aktuelle Zeit in Millisekunden seit 1.1.1970, 00:00 Uhr
 */
uint64_t GetMilliSeconds();

/** @ingroup PPLGroupDateTime
 * @brief Liefert die aktuelle Zeit in Mikrosekunden
 *
 * @return Aktuelle Zeit in Mikrosekunden seit 1.1.1970, 00:00 Uhr
 */
uint64_t GetMicroSeconds();

/** @ingroup PPLGroupDateTime
 * @brief Erstellt einen Unix-Timestamp aus den angegebenen Datums- und Zeitangaben
 *
 * @param year Jahr
 * @param month Monat (1-12)
 * @param day Tag des Monats (1-31)
 * @param hour Stunde (0-23)
 * @param min Minute (0-59)
 * @param sec Sekunde (0-59)
 * @return Unix-Timestamp (Sekunden seit 1.1.1970, 00:00 Uhr)
 */
ppl_time_t MkTime(int year, int month, int day, int hour = 0, int min = 0, int sec = 0);

/** @ingroup PPLGroupDateTime
 * @brief Erstellt einen Unix-Timestamp aus einem ISO-8601-Datumstring
 *
 * @param iso8601date ISO-8601-Datumstring
 * @return Unix-Timestamp (Sekunden seit 1.1.1970, 00:00 Uhr)
 */
ppl_time_t MkTime(const String& iso8601date);

/** @ingroup PPLGroupDateTime
 * @brief Wandelt ein PPLTIME-Objekt in einen Unix-Timestamp um
 *
 * @param t Struktur vom Typ PPLTIME, die die Datumsinformationen enthält
 * @return Unix-Timestamp (Sekunden seit 1.1.1970, 00:00 Uhr)
 */
ppl_time_t MkTime(const PPLTIME& t);

/** @ingroup PPLGroupDateTime
 * @brief Erstellt einen ISO-8601-Datumstring aus einem Unix-Timestamp
 *
 * @param sec Unix-Timestamp (Sekunden seit 1.1.1970, 00:00 Uhr)
 * @return ISO-8601-Datumstring
 */
String MkISO8601Date(ppl_time_t sec);
/** @ingroup PPLGroupDateTime
 * @brief Erstellt einen ISO-8601-Datumstring aus einem PPLTIME-Objekt
 *
 * @param t Struktur vom Typ PPLTIME, die die Datumsinformationen enthält
 * @return ISO-8601-Datumstring
 */
String MkISO8601Date(const PPLTIME& t);

/** @ingroup PPLGroupDateTime
 * @brief Datumstring nach RFC-822 (Mailformat) erzeugen
 *
 * Mit dieser Funktion wird ein Datummstring nach RFC-822 erzeugt, wie er im Header einer Email verwendet wird.
 * Das Format lautet:
 * @code
 * weekday, day month year time zone
 * @endcode
 * und hat folgende Bedeutung:
 * - weekday: Name des Wochentags ("Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat")
 * - day: Tag des Monats mit ein oder zwei Ziffern
 * - month: Name des Monats ("Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec")
 * - year: Das Jahr mit 4 Ziffern
 * - time: Stunde:Minute:Sekunde (hh:mm:ss), jeweils mit zwei Ziffern und Doppelpunkt getrennt
 * - zone: Offset zu UTC in Stunden und Minuten (+|-HHMM)
 *
 * @param[in] sec Sekunden seit 1970.
 * @return Datumstring nach RFC-822 (Mailformat)
 * @exception InvalidDateException Die Funktion wirft eine Exception, wenn das Datum ungültig ist.
 */
String MkRFC822Date(ppl_time_t sec);

/** @ingroup PPLGroupDateTime
 * @brief Datumstring nach RFC-822 (Mailformat) erzeugen
 *
 * @param[in] t Eine PPLTIME-Struktur, der die Datumsinformationen entnommen werden
 * @return Datumstring nach RFC-822 (Mailformat)
 * @exception InvalidDateException Die Funktion wirft eine Exception, wenn das Datum ungültig ist.
 */
String MkRFC822Date(const PPLTIME& t);

/** @brief Datum/Zeit formatieren
 * @ingroup PPLGroupDateTime
 *
 * Die Funktion MkDate wandelt einen Unix-Timestamp in einen String um.
 *
 * @param format ist ein beliebiger String, der verschiedene  Platzhalter
 * entahlten darf (siehe unten)
 * @param sec
 * @return Bei Erfolg gibt die Funktion einen neuen String mit dem formatierten
 * Zeitpunkt zurück.
 * @exception Im Fehlerfall wird eine Exception geworfen
 *
 * @par Syntax-Formatstring
 * @copydoc strftime.dox
 */
String MkDate(const String& format, ppl_time_t sec);

/** @brief Datum/Zeit formatieren
 * @ingroup PPLGroupDateTime
 *
 * @param format ist ein beliebiger String, der verschiedene  Platzhalter
 * entahlten darf (siehe unten)
 * @param t Eine PPLTIME-Struktur, der die Datumsinformationen entnommen werden
 * @return Bei Erfolg gibt die Funktion einen neuen String mit dem formatierten
 * Zeitpunkt zurück.
 * @exception Im Fehlerfall wird eine Exception geworfen
 *
 * @par Syntax-Formatstring
 * @copydoc strftime.dox
 */
String MkDate(const String& format, const PPLTIME& t);

/** @ingroup PPLGroupDateTime
 * @brief Mikrosekunden schlafen
 *
 * Diese Funktion pausiert die Ausführung des aktuellen Threads für die angegebene Anzahl von Mikrosekunden.
 * Unter Windows wird versucht, einen hochauflösenden Timer zu verwenden, der ab Windows 10 (1803) verfügbar ist.
 * Ist dies nicht der Fall, wird auf die Standard-Schlaffunktion zurückgegriffen, die sehr ungenau sein kann.
 * Unter Linux und anderen Plattformen wird die Standard-Schlaffunktion verwendet, die in der Regel ausreichend genau ist.
 *
 * @param microseconds Anzahl der Mikrosekunden, die geschlafen werden sollen.  1 Sekunde = 1000000 Mikrosekunden.
 */
void USleep(uint64_t microseconds);

/** @ingroup PPLGroupDateTime
 * @brief Millisekunden schlafen
 *
 * Diese Funktion pausiert die Ausführung des aktuellen Threads für die angegebene Anzahl von Millisekunden.
 * Unter Windows wird versucht, einen hochauflösenden Timer zu verwenden, der ab Windows 10 (1803) verfügbar ist.
 * Ist dies nicht der Fall, wird auf die Standard-Schlaffunktion zurückgegriffen, die sehr ungenau sein kann.
 * Unter Linux und anderen Plattformen wird die Standard-Schlaffunktion verwendet, die in der Regel ausreichend genau ist.
 *
 * @param milliseconds Anzahl der Millisekunden, die geschlafen werden sollen.  1 Sekunde = 1000 Millisekunden.
 */
void MSleep(uint64_t milliseconds);

/** @ingroup PPLGroupDateTime
 * @brief Sekunden schlafen
 *
 * Diese Funktion pausiert die Ausführung des aktuellen Threads für die angegebene Anzahl von Sekunden.
 * Die Funktion verwendet die Standard-Schlaffunktion des Betriebssystems, die insbesondere unter Windows ungenau sein kann.
 * Es ist garantiert, dass die Funktion mindestens die angegebene Zeit pausiert. Unter Windows kann das aber auch 16 Millisekunden länger
 * sein. Unter Linux und anderen Plattformen wird die Standard-Schlaffunktion verwendet, die in der Regel ausreichend genau ist.
 *
 * @param seconds Anzahl der Sekunden, die geschlafen werden sollen.
 */
void SSleep(uint64_t seconds);

/**  @ingroup PPLGroupDateTime
 * @brief Sekunden schlafen (mit Bruchteilen)
 *
 * Diese Funktion pausiert die Ausführung des aktuellen Threads für die angegebene Anzahl von Sekunden, die auch Bruchteile enthalten kann.
 * Wie `USleep` und `MSleep` versucht diese Funktion unter Windows, einen hochauflösenden Timer zu verwenden, der ab Windows 10 (1803)
 * verfügbar ist. Ist dies nicht der Fall, wird auf die Standard-Schlaffunktion zurückgegriffen, die sehr ungenau sein kann. Unter Linux und
 * anderen Plattformen wird die Standard-Schlaffunktion verwendet, die in der Regel ausreichend genau ist.
 *
 * @param seconds Anzahl der Sekunden, die geschlafen werden sollen. Kann auch Bruchteile enthalten.
 */
void Sleep(double seconds);

}; // namespace pplib

#endif // PPLIB_CORE_TIME_H_
