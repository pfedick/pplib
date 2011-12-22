/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
 *
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include "prolog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#ifdef HAVE_SYS_TIME_H
	#include <sys/time.h>
#endif
#ifdef HAVE_SYS_TYPES_H
	#include <sys/types.h>
#endif
#include "ppl7.h"
#include "ppl7-types.h"


namespace ppl7 {

#ifndef HAVE_LOCALTIME_R
// In diesem Fall ist ein Workaround für localtime_r in core/time.cpp definiert
struct tm * localtime_r(const time_t *clock, struct tm *result);
#endif


/*!\class DateTime
 * \ingroup PPLGroupDataTypes
 * \ingroup PPLGroupDateTime
 * \brief Datenobjekt zum Speichern von Datum und Uhrzeit
 *
 * \desc
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
 * \since
 * Die Klasse wurde mit Version 6.4.1 eingeführt.
 */


/*!\var DateTime::yy
 * \brief Jahr
 */

/*!\var DateTime::mm
 * \brief Monat
 */

/*!\var DateTime::dd
 * \brief Tag
 */

/*!\var DateTime::hh
 * \brief Stunden
 */

/*!\var DateTime::ii
 * \brief Minuten
 */

/*!\var DateTime::ss
 * \brief Sekunden
 */

/*!\var DateTime::ms
 * \brief Millisekunden
 */


//!\name Konstruktoren

/*!\brief Konstruktor mit Initialisierung auf 0
 *
 * \desc
 * Mit diesem Konstruktor ohne Parameter wird der Wert der Datumsklasse auf 0 gesetzt. Die Funktion
 * DateTime::isEmpty "isEmpty" würde \c true zurückliefern.
 */
DateTime::DateTime()
{
	type=Variant::DATETIME;
	clear();
}


/*!\brief Konstruktor mit Datumsinitialisierung aus einem String
 *
 * \desc
 * Über diesen Konstruktor wird ein DateTime Objekt anhand des im String \p datetime enthaltenen
 * Datums und Uhrzeit erstellt. Die unterstützten Formate sind in der Funktion
 * \ref DateTime::set(const String &datetime) "set" beschrieben.
 *
 * @param[in] datetime String mit Datum und Uhrzeit
 *
 * \exception IllegalArgumentException: Wird geworfen, wenn der String \p datetime
 * ein ungültiges oder unbekanntes Datumsformat hat.
 * Ausnahmen: Ist der String leer oder enthält nur den
 * Buchstaben "T" oder den Wert "0" wird keine Exception geworfen, sondern der Datumswert auf 0 gesetzt.
 */
DateTime::DateTime(const String &datetime)
{
	type=Variant::DATETIME;
	if (!set(datetime)) throw IllegalArgumentException();
}

/*!\brief Copy-Konstruktor
 *
 * \desc
 * Über diesen Konstruktor wird das Datum eines anderen DateTime-Wertes übernommen.
 *
 * @param[in] other Referenz auf den zu kopierenden DateTime-Wert
 */
DateTime::DateTime(const DateTime &other)
{
	type=Variant::DATETIME;
	yy=other.yy;
	ms=other.ms;
	mm=other.mm;
	dd=other.dd;
	hh=other.hh;
	ii=other.ii;
	ss=other.ss;
}

//@}


//!\name Datum setzen

/*!\brief Datum auf 0 setzen
 *
 * \desc
 * Mit dieser Funktion wird der Datumswert der Klasse auf 0 gesetzt. Die Funktion
 * \ref DateTime::isEmpty "isEmpty" würde \c true zurückliefern. Die Klasse wird somit
 * wieder in den Ausgangszustand versetzt.
 */
void DateTime::clear()
{
	yy=0;
	ms=0;
	mm=0;
	dd=0;
	hh=0;
	ii=0;
	ss=0;
}

/*!\brief Datum anhand eines Strings setzen
 *
 * \desc
 * Mit dieser Funktion wird das Datum anhand des Strings \p datetime gesetzt. Dabei versucht die Funktion
 * anhand mehrerer Regular Expressions zu erkennen, in welchem Format die Datumsangabe vorliegt. Es werden
 * folgende Formate erkannt:
 * - yyyy-mm-dd hh:ii:ss[.mms]
 * - yyyy.mm.dd hh:ii:ss[.mms]
 * - dd-mm-yyyy hh:ii:ss[.mms]
 * - dd.mm.yyyy hh:ii:ss[.mms]
 * - yyyy-mm-ddThh:ii:ss[.mms]+oo:oo (ISO 8601-Format)
 * - yyyy-mm-dd
 * - yyyy.mm.dd
 * - dd-mm-yyyy
 * - dd.mm.yyyy
 * - T: wird als Leerstring interpretiert und setzt das Datum auf 0
 * - Leerstring: setzt das Datum auf 0
 * \par Legende:
 * - yyyy: 4-Stellige Jahreszahl. Muss zwingen 4-stellig sein, da sonst nicht erkannt wird ob die Jahreszahl
 *   an erster oder dritter Stelle steht. Jahreszahlen < 1000 müssen daher mit führenden Nullen aufgefüllt
 *   werden, z.B. "0500" statt "500".
 * - mm: Monatszahl zwischen 1 und 12. Kann ein- oder zweistellig sein
 * - dd: Monatstag zwischen 1 und 31, kann ein- oder zweistellig sein.
 * - hh: Stunden zwischen 0 und 23, kann ein- oder zweistellig sein
 * - ii: Minuten zwischen 0 und 59, kann ein- oder zweistellig sein
 * - ss: Sekunden zwischen 0 und 59, kann ein- oder zweistellig sein
 * - mms: Millisekunden zwischen 0 und 999, kann ein- bis dreistellig sein, Wert ist optional und kann statt mit einem Punkt
 *   auch mit einem Doppelpunkt von den Sekunden der Uhrzeit getrennt sein.
 * \par
 * Bei der Datumsangabe kann als Trennzeichen wahlweise Punkt oder Minus verwendet werden. Es muss mindestens ein
 * vollständiges Datum angegeben werden und optional eine vollständige Uhrzeit (hh:ii:ss), wobei die Millisekunden
 * optional sind.
 *
 * @param[in] datetime String mit dem zu setzenden Datum und optional der Uhrzeit
 * @return Wurde der String erfolgreich erkannt, liefert die Funktion 1 zurück, andernfalls 0. Ferner wird der
 * Fehlercode 558 gesetzt.
 */
int DateTime::set(const String &datetime)
{
	String d=UpperCase(Trim(datetime));
	Array m;
	d.replace(","," ");
	if (d.isEmpty()==true || d=="T" || d=="0") {
		clear();
		return 1;
	}


	if (d.pregMatch("/^([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})T([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})[\\.:]([0-9]{1,4})/",m)) {
		// yyyy-mm-ddThh:ii:ss.msec[[+-]oo:00]
		set(m.get(1).toInt(),
				m.get(2).toInt(),
				m.get(3).toInt(),
				m.get(4).toInt(),
				m.get(5).toInt(),
				m.get(6).toInt(),
				m.get(7).toInt());

	} else if (d.pregMatch("/^([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})T([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})/",m)) {
		// yyyy-mm-ddThh:ii:ss[[+-]oo:00]
		set(m.get(1).toInt(),
				m.get(2).toInt(),
				m.get(3).toInt(),
				m.get(4).toInt(),
				m.get(5).toInt(),
				m.get(6).toInt());
	} else if (d.pregMatch("/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})\\s+([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})[\\.:]([0-9]{1,4})$/",m)) {
		// yyyy.mm.dd hh:ii:ss.msec
		set(m.get(1).toInt(),
				m.get(2).toInt(),
				m.get(3).toInt(),
				m.get(4).toInt(),
				m.get(5).toInt(),
				m.get(6).toInt(),
				m.get(7).toInt());
	} else if (d.pregMatch("/^([0-9]{1,2})[\\.-]([0-9]{1,2})[\\.-]([0-9]{4})\\s+([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})[\\.:]([0-9]{1,4})$/",m)) {
		// dd.mm.yyyy hh:ii:ss.msec
		set(m.get(3).toInt(),
				m.get(2).toInt(),
				m.get(1).toInt(),
				m.get(4).toInt(),
				m.get(5).toInt(),
				m.get(6).toInt(),
				m.get(7).toInt());
	} else if (d.pregMatch("/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})\\s+([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})$/",m)) {
		// yyyy.mm.dd hh:ii:ss
		set(m.get(1).toInt(),
				m.get(2).toInt(),
				m.get(3).toInt(),
				m.get(4).toInt(),
				m.get(5).toInt(),
				m.get(6).toInt());
	} else if (d.pregMatch("/^([0-9]{1,2})[\\.-]([0-9]{1,2})[\\.-]([0-9]{4})\\s+([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})$/",m)) {
		// dd.mm.yyyy hh:ii:ss
		set(m.get(3).toInt(),
				m.get(2).toInt(),
				m.get(1).toInt(),
				m.get(4).toInt(),
				m.get(5).toInt(),
				m.get(6).toInt());
	} else if (d.pregMatch("/^([0-9]{1,2})[\\.-]([0-9]{1,2})[\\.-]([0-9]{4})$/",m)) {
		// dd.mm.yyyy
		set(m.get(3).toInt(),
				m.get(2).toInt(),
				m.get(1).toInt());
	} else if (d.pregMatch("/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})$/",m)) {
		// yyyy.mm.dd
		set(m.get(1).toInt(),
				m.get(2).toInt(),
				m.get(3).toInt());
	} else {
		clear();
		throw InvalidDateException(datetime);
		return 0;
	}
	return 1;
}

/*!\brief Datum aus einer anderen DateTime-Variablen übernehmen
 *
 * \desc
 * Mit dieser Funktion wird der Wert einer anderen DateTime-Variablen übernommen.
 *
 * @param[in] other Referenz auf eine andere DateTime-Variable, dessen Wert kopiert werden soll.
 */
void DateTime::set(const DateTime &other)
{
	yy=other.yy;
	ms=other.ms;
	mm=other.mm;
	dd=other.dd;
	hh=other.hh;
	ii=other.ii;
	ss=other.ss;
}

/*!\brief Datum und Uhrzeit aus unterschiedlichen Strings importieren
 *
 * \desc
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
 * @return Wurden Datum und Uhrzeit erfolgreich erkannt, liefert die Funktion 1 zurück, andernfalls 0. Ferner wird der
 * Fehlercode 558 gesetzt.
 * \see
 * Eine genauere Beschreibung der Formate samt Legende ist \ref DateTime::set(const String &datetime) "hier"
 * zu finden.
 */
int DateTime::set(const String &date, const String &time)
{
	String d,dd=Trim(date),tt=Trim(time);
	dd.replace(",",".");
	dd.replace(":",".");
	tt.replace(",",":");
	tt.replace(".",":");
	tt.replace("-",":");

	d=dd+" "+tt;
	return set(d);
}

/*!\brief Datum setzen, Uhrzeit bleibt unverändert
 *
 * \desc
 * Mit dieser Funktion wird nur das Datum der Klasse verändert, die Uhrzeit bleibt erhalten.
 *
 * @param[in] date Referenz auf den String mit dem zu setzenden Datum. Das Format wird bei der
 * \ref DateTime::set(const String &date, const String &time) "set-Funktion" genauer beschrieben.
 * @return Wurde das Datum erfolgreich erkannt, liefert die Funktion 1 zurück, andernfalls 0. Ferner wird der
 * Fehlercode 558 gesetzt.
 */
int DateTime::setDate(const String &date)
{
	String time=getTime();
	return set(date,time);
}

/*!\brief Uhrzeit setzen, Datum bleibt unverändert
 *
 * \desc
 * Mit dieser Funktion wird nur die Uhrzeit der Klasse verändert, das Datum bleibt erhalten.
 *
 * @param[in] time Referenz auf den String mit der zu setzenden Uhrzeit. Das Format wird bei der
 * \ref DateTime::set(const String &date, const String &time) "set-Funktion" genauer beschrieben.
 * @return Wurde die Uhrzeit erfolgreich erkannt, liefert die Funktion 1 zurück, andernfalls 0. Ferner wird der
 * Fehlercode 558 gesetzt.
 */
int DateTime::setTime(const String &time)
{
	String date=getDate();
	return set(date,time);
}

/*!\brief Datum und Uhrzeit anhand einzelner Integer-Wert setzen
 *
 * \desc
 * Mit dieser Funktion wird das Datum anhand einzelner Integer-Werten gesetzt.
 *
 * @param[in] year Jahreszahl zwischen 0 und 9999
 * @param[in] month Monat zwischen 1 und 12
 * @param[in] day Tag zwischen 1 und 31
 * @param[in] hour Stunde zwischen 0 und 23. Optionaler Wert, Default ist 0.
 * @param[in] minute Minute zwischen 0 und 59. Optionaler Wert, Default ist 0.
 * @param[in] sec Sekunde zwischen 0 und 59. Optionaler Wert, Default ist 0.
 * @param[in] msec Millisekunde zwischen 0 und 999. Optionaler Wert, Default ist 0.
 * \attention
 * Gegenwärtig werden Werte ausserhalb des Gültigkeitsbereiches abgeschnitten! Aus dem Monat 0 oder -10 würde 1
 * werden, aus 13 oder 12345 würde 12 werden. Dieses Verhalten wird sich in einer späteren Version noch ändern!
 * Geplant ist, dass bei Überlauf eines Wertes die anderen automatisch angepasst werden, so dass z.B. aus
 * dem 32.12.2010 automatisch der 01.01.2011 wird.
 */
void DateTime::set(int year, int month, int day, int hour, int minute, int sec, int msec)
{
	yy=year;
	if (year<0) yy=0;
	if (year>9999) yy=9999;
	mm=month;
	if (month<1) mm=1;
	if (month>12) mm=12;
	dd=day;
	if (day<1) dd=1;
	if (day>31) dd=31;
	hh=hour;
	if (hour<0) hh=0;
	if (hour>23) hh=23;
	ii=minute;
	if (minute<0) ii=0;
	if (minute>59) ii=59;
	ss=sec;
	if (sec<0) ss=0;
	if (sec>59) ss=59;
	ms=msec;
	if (msec<0) ms=0;
	if (msec>999) ms=999;
}

/*!\brief Datum aus Unix-Timestamp übernehmen
 *
 * \desc
 * Mit dieser Funktion werden Datum und Uhrzeit aus einem Unix-Timestamp übernommen (Sekunden seit 1970),
 * wie ihn Beispielsweise die C-Funktion "time()" zurückliefert. Es ist daher nicht möglich ein Datum vor
 * 1970 zu setzen.
 *
 * @param t 64-Bit Integer mit den Sekunden seit 1970.
 */
void DateTime::setTime_t(ppluint64 t)
{
	struct tm tt, *r;
	if (t==0) {
		clear();
		return;
	}
	::time_t tp=(::time_t)t;
	r=localtime_r(&tp,&tt);
	ss=tt.tm_sec;
	ii=tt.tm_min;
	hh=tt.tm_hour;
	dd=tt.tm_mday;
	mm=tt.tm_mon+1;
	yy=tt.tm_year+1900;
	ms=0;
}

/*!\brief Datum aus Unix-Timestamp übernehmen
 *
 * \desc
 * Mit dieser Funktion werden Datum und Uhrzeit aus einem Unix-Timestamp übernommen (Sekunden seit 1970),
 * wie ihn Beispielsweise die C-Funktion "time()" zurückliefert. Es ist daher nicht möglich ein Datum vor
 * 1970 zu setzen.
 *
 * @param t 64-Bit Integer mit den Sekunden seit 1970.
 * \see http://de.wikipedia.org/wiki/Unixzeit
 */
void DateTime::setEpoch(ppluint64 t)
{
	struct tm tt, *r;
	if (t==0) {
		clear();
		return;
	}
	::time_t tp=(::time_t)t;
	r=localtime_r(&tp,&tt);
	ss=tt.tm_sec;
	ii=tt.tm_min;
	hh=tt.tm_hour;
	dd=tt.tm_mday;
	mm=tt.tm_mon+1;
	yy=tt.tm_year+1900;
	ms=0;
}

/*!\brief Datum aus einem 64-Bit-Integer übernehmen
 *
 * \desc
 * Mit dieser Funktion werden Datum, Uhrzeit und Millisekunden aus einem Long Integer (64 Bit) übernommen,
 * wie ihn die Funktion DateTime::longInt zurückgibt. Der Integer-Wert muss folgenden Aufbau haben:
 *
 * - Bit 0-10: Millisekunden (0-999)
 * - Bit 11-17: Sekunden (0-59)
 * - Bit 18-24: Minuten (0-59)
 * - Bit 25-30: Stunden (0-23)
 * - Bit 31-36: Tag (1-31)
 * - Bit 37-41: Monat (1-12)
 * - Bit 42-63: Jahr (0-16383)
 *
 * @param i 64-Bit Integer
 */
void DateTime::setLongInt(ppluint64 i)
{
	ms=(ppluint16)(i&1023);
	ss=(ppluint8)(i>>11)&63;
	ii=(ppluint8)(i>>18)&63;
	hh=(ppluint8)(i>>25)&31;
	dd=(ppluint8)(i>>31)&31;
	mm=(ppluint8)(i>>37)&15;
	yy=(ppluint16)(i>>42)&16383;
	if (ms>999) ms=999;
	if (ss>59) ss=59;
	if (ii>59) ii=59;
	if (hh>23) hh=23;
	if (dd>31) dd=31;
	if (dd<1) dd=1;
	if (mm<1) mm=1;
	if (mm>12) mm=12;
}


/*!\brief Aktuelles Datum und Uhrzeit übernehmen
 *
 * \desc
 * Mit dieser Funktion wird die Variable auf das aktuelle Datum und die aktuelle Uhrzeit gesetzt.
 * Es gibt sie auch als statische Funktion \ref DateTime::currentTime "currentTime".
 */
void DateTime::setCurrentTime()
{
	struct tm tt, *r;
	::time_t tp=time(NULL);
	r=localtime_r(&tp,&tt);
	ss=tt.tm_sec;
	ii=tt.tm_min;
	hh=tt.tm_hour;
	dd=tt.tm_mday;
	mm=tt.tm_mon+1;
	yy=tt.tm_year+1900;
	ms=0;
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tv;
	if (gettimeofday(&tv,NULL)==0) {
		ms=tv.tv_usec/1000;
	}

#endif
}



//@}

//!\name Datum auslesen


/*!\brief Datum als String im angegebenen Format zurückgeben
 *
 * \desc
 * Datum als String im angegebenen Format zurückgeben
 *
 * @param[in] format Formatierungsstring. Wird dieser nicht angegeben, wird das Datum in folgendem Format zurückgegeben:
 * "%Y-%m-%d %H:%M:%S"
 *
 * @return String mit dem Datum im gewünschten Format
 * \par Formatierung
 *  Erlaubt sind folgende Formatzeichen:
 * - \%Y: Das Jahr als 4-stellige Angabe (z.B. 2010)
 * - \%y: Das Jahr als 2-stellige Angabe ohne Jahrhundert (z.B. 10)
 * - \%m: Der Monat als zweistellige Zahl (01 bis 12)
 * - \%d: Der Tag als zweistellige Zahl (01 bis 31)
 * - \%H: Stunden als zweistellige Zahl (00 bis 23)
 * - \%M: Minuten als zweistellige Zahl (00 bis 59)
 * - \%S: Sekunden als zweistellige Zahl (00 bis 59)
 * - \%*: Millisekunden als dreistellige Zahl (00 bis 999)
 * \par
 * Falls das im Objekt enthaltene Datum > 1900 ist, können weitere Formatanweisungen verwendet werden.
 * \par
 * \copydoc strftime.dox
 *
 */
String DateTime::get(const String &format) const
{
	String Tmp;
	String r=format;
	Tmp.setf("%03i",ms);
	r.replace("%*",Tmp);

	if (yy<1900) {
		Tmp.setf("%04i",yy);
		r.replace("%Y",Tmp);
		Tmp.setf("%02i",yy%100);
		r.replace("%y",Tmp);

		Tmp.setf("%02i",mm);
		r.replace("%m",Tmp);

		Tmp.setf("%02i",dd);
		r.replace("%d",Tmp);

		Tmp.setf("%02i",hh);
		r.replace("%H",Tmp);

		Tmp.setf("%02i",ii);
		r.replace("%M",Tmp);

		Tmp.setf("%02i",ss);
		r.replace("%S",Tmp);

		return r;
	}

	struct tm t;
	t.tm_sec=ss;
	t.tm_min=ii;
	t.tm_hour=hh;
	t.tm_mday=dd;
	t.tm_mon=mm-1;
	t.tm_year=yy-1900;
	t.tm_isdst=-1;
	mktime(&t);

	size_t size=r.len()*2+32;
	char *b=(char*)malloc(size);
	if (!b) {
		throw OutOfMemoryException();
	}
	if (strftime(b, size,(const char*)r.toUtf8(), &t)==0) {
		throw IllegalArgumentException("DateTime::get(\"%s\")",(const char*)r.toUtf8());
	}
	r.set(b);
	free(b);
	return r;
}

/*!\brief Datum als String zurückgeben
 *
 * \desc
 * Diese Funktion ist identisch zu DateTime::get, hat aber einen anderen Default für den optionalen
 * Formatstring.
 *
 * @param[in] format Formatierungsstring. Wird dieser nicht angegeben, wird das Datum in folgendem Format zurückgegeben:
 * "%Y-%m-%d"
 *
 * @return String mit dem Datum im gewünschten Format
 *
 * \see
 * Siehe DateTime::get
 */
String DateTime::getDate(const String &format) const
{
	return get(format);
}

/*!\brief Uhrzeit als String zurückgeben
 *
 * \desc
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
String DateTime::getTime(const String &format) const
{
	return get(format);
}

/*!\brief Datum als String im ISO8601-Format zurückgeben
 *
 * \desc
 * Diese Funktion gibt das Datum als String im ISO8601-Format zurück, das folgenden Aufbau hat:
 * "yyyy-mm-ddThh:ii:ss+zz:zz"
 * \par
 * Der Wert "+zz:zz" gibt den Offset zu GMT in Stunden und Minuten an und kann auch negativ sein.
 * Er wird allerdings nur ergänzt, wenn das Jahr >=1900 ist und das Betriebssystem den Wert "tm_gmtoff" in
 * seiner tm-Structure hat (siehe "man ctime").
 *
 * @return String mit dem Datum im ISO8601-Format
 */
String DateTime::getISO8601() const
{
	String r;
	r.setf("%04i-%02i-%02iT%02i:%02i:%02i",yy,mm,dd,hh,ii,ss);

#ifdef STRUCT_TM_HAS_GMTOFF
	if (yy>=1900) {
		struct tm t;
		t.tm_sec=ss;
		t.tm_min=ii;
		t.tm_hour=hh;
		t.tm_mday=dd;
		t.tm_mon=mm-1;
		t.tm_year=yy-1900;
		t.tm_isdst=-1;
		mktime(&t);

		int s=abs(t.tm_gmtoff/60);
		if (t.tm_gmtoff>=0) {
			r.appendf("+%02i:%02i",(int)(s/60),t.tm_gmtoff%60);
		} else {
			r.appendf("-%02i:%02i",(int)(s/60),t.tm_gmtoff%60);
		}
	}
#endif
	return r;
}

/*!\brief Datum als String im ISO8601-Format mit Millisekunden zurückgeben
 *
 * \desc
 * Diese Funktion gibt das Datum als String im ISO8601-Format mit Millisekunden zurück, das folgenden Aufbau hat:
 * "yyyy-mm-ddThh:ii:ss.xxx+zz:zz"
 * \par
 * Der Wert "xxx" stellt die Millisekunden dar.
 * \par
 * Der Wert "+zz:zz" gibt den Offset zu GMT in Stunden und Minuten an und kann auch negativ sein.
 * Er wird allerdings nur ergänzt, wenn das Jahr >=1900 ist und das Betriebssystem den Wert "tm_gmtoff" in
 * seiner tm-Structure hat (siehe "man ctime").
 *
 * @return String mit dem Datum im ISO8601-Format
 */
String DateTime::getISO8601withMsec() const
{
	String r;
	r.setf("%04i-%02i-%02iT%02i:%02i:%02i.%03i",yy,mm,dd,hh,ii,ss,ms);

#ifdef STRUCT_TM_HAS_GMTOFF
	if (yy>=1900) {
		struct tm t;
		t.tm_sec=ss;
		t.tm_min=ii;
		t.tm_hour=hh;
		t.tm_mday=dd;
		t.tm_mon=mm-1;
		t.tm_year=yy-1900;
		t.tm_isdst=-1;
		mktime(&t);

		int s=abs(t.tm_gmtoff/60);
		if (t.tm_gmtoff>=0) {
			r.appendf("+%02i:%02i",(int)(s/60),t.tm_gmtoff%60);
		} else {
			r.appendf("-%02i:%02i",(int)(s/60),t.tm_gmtoff%60);
		}
	}
#endif
	return r;
}


/*!\brief Datum in Unix-Timestamp umrechnen
 *
 * \desc
 * Mit dieser Funktion wird das in der Variablen enthaltene Datum und Uhrzeit in einen
 * Unix-Timestamp umgerechnet (Sekunden seit 1970).
 *
 * @return Sekunden seit 1970 oder 0, wenn das Datum sich nicht umrechnen läßt, z.B. wenn das Jahr vor 1970 liegt.
 */
ppluint64 DateTime::time_t() const
{
	if (yy<1900) return 0;
	struct tm t;
	t.tm_sec=ss;
	t.tm_min=ii;
	t.tm_hour=hh;
	t.tm_mday=dd;
	t.tm_mon=mm-1;
	t.tm_year=yy-1900;
	t.tm_isdst=-1;
	return (ppluint64)mktime(&t);
}

/*!\brief Datum in Unix-Timestamp umrechnen
 *
 * \desc
 * Mit dieser Funktion wird das in der Variablen enthaltene Datum und Uhrzeit in einen
 * Unix-Timestamp umgerechnet (Sekunden seit 1970).
 *
 * @return Sekunden seit 1970 oder 0, wenn das Datum sich nicht umrechnen läßt, z.B. wenn das Jahr vor 1970 liegt.
 *
 * \see http://de.wikipedia.org/wiki/Unixzeit
 */
ppluint64 DateTime::epoch() const
{
	if (yy<1900) return 0;
	struct tm t;
	t.tm_sec=ss;
	t.tm_min=ii;
	t.tm_hour=hh;
	t.tm_mday=dd;
	t.tm_mon=mm-1;
	t.tm_year=yy-1900;
	t.tm_isdst=-1;
	return (ppluint64)mktime(&t);
}

/*!\brief Datum als 64-Bit-Integer auslesen
 *
 * Mit dieser Funktion werden Datum, Uhrzeit und Millisekunden als Long Integer (64 Bit) zurückgegeben,
 * wie er von der Funktion DateTime::setLongInt eingelesen werden kann. Der Integer-Wert hat folgenden Aufbau:
 *
 * - Bit 0-10: Millisekunden (0-999)
 * - Bit 11-17: Sekunden (0-59)
 * - Bit 18-24: Minuten (0-59)
 * - Bit 25-30: Stunden (0-23)
 * - Bit 31-36: Tag (1-31)
 * - Bit 37-41: Monat (1-12)
 * - Bit 42-63: Jahr (0-16383)
 *
 * @return 64-Bit-Integer mit dem Timestamp
 */
ppluint64 DateTime::longInt() const
{
	ppluint64 r=0;
	r=(ppluint64)(yy&16383)<<42;
	r|=(ppluint64)(mm&15)<<37;
	r|=(ppluint64)(dd&31)<<31;
	r|=(ppluint64)(hh&31)<<25;
	r|=(ppluint64)(ii&63)<<18;
	r|=(ppluint64)(ss&63)<<11;
	r|=(ppluint64)(ms&1023);
	return r;
}

/*!\brief Das Jahr als Integer auslesen
 *
 * \desc
 * Diese Funktion gibt die Jahreszahl als Integer zurück.
 *
 * @return Integer-Wert mit dem Jahr
 */
int DateTime::year() const
{
	return yy;
}

/*!\brief Den Monat als Integer auslesen
 *
 * \desc
 * Diese Funktion gibt den Monat als Integer zurück.
 *
 * @return Integer-Wert mit dem Monat
 */
int DateTime::month() const
{
	return mm;
}

/*!\brief Den Tag als Integer auslesen
 *
 * \desc
 * Diese Funktion gibt den Tag als Integer zurück.
 *
 * @return Integer-Wert mit dem Tag
 */
int DateTime::day() const
{
	return dd;
}

/*!\brief Die Stunde als Integer auslesen
 *
 * \desc
 * Diese Funktion gibt die Stunde als Integer zurück.
 *
 * @return Integer-Wert mit der Stunde
 */
int DateTime::hour() const
{
	return hh;
}

/*!\brief Die Minute als Integer auslesen
 *
 * \desc
 * Diese Funktion gibt die Minute als Integer zurück.
 *
 * @return Integer-Wert mit der Minute
 */
int DateTime::minute() const
{
	return ii;
}

/*!\brief Die Sekunde als Integer auslesen
 *
 * \desc
 * Diese Funktion gibt die Sekunde als Integer zurück.
 *
 * @return Integer-Wert mit der Sekunde
 */
int DateTime::second() const
{
	return ss;
}

/*!\brief Die Millisekunden als Integer auslesen
 *
 * \desc
 * Diese Funktion gibt die Millisekunden als Integer zurück.
 *
 * @return Integer-Wert mit den Millisekunden
 */
int DateTime::millisecond() const
{
	return ms;
}

//@}

//!\name Verschiedenes

/*!\brief Datum und Uhrzeit auf STDOUT ausgeben
 *
 * \desc
 * Mit dieser Funktion wird Datum und Uhrzeit auf der Konsole (STDOUT) ausgegeben. Sie ist
 * nur zu Debug-Zwecken gedacht.
 */
void DateTime::print() const
{
	printf ("%04i-%02i-%02i %02i:%02i:%02i\n",yy,mm,dd,hh,ii,ss);
}

/*!\brief Prüft, ob ein Datum oder Uhrzeit vorhanden ist
 *
 * \desc
 * Diese Funktion liefert \c true zurück, wenn ein Datum oder Uhrzeit gesetzt ist, der Wert also \b nicht Null ist.
 * Sie ist somit das Gegenteil zu DateTime::isEmpty.
 * @return \c true oder \c false
 *
 */
bool DateTime::notEmpty() const
{
	if (yy>0) return 1;
	if (mm>0) return 1;
	if (dd>0) return 1;
	if (hh>0) return 1;
	if (ii>0) return 1;
	if (ss>0) return 1;
	if (ms>0) return 1;
	return 0;
}

/*!\brief Prüft, ob ein Datum oder Uhrzeit vorhanden ist
 *
 * \desc
 * Diese Funktion liefert \c true zurück, wenn \b kein Datum und \b keine Uhrzeit gesetzt ist,
 * der Wert also Null ist. Sie ist somit das Gegenteil zu DateTime::notEmpty.
 *
 * @return \c true oder \c false
 */
bool DateTime::isEmpty() const
{
	if (yy>0) return 0;
	if (mm>0) return 0;
	if (dd>0) return 0;
	if (hh>0) return 0;
	if (ii>0) return 0;
	if (ss>0) return 0;
	if (ms>0) return 0;
	return 1;
}




/*!\brief Datum auf Schaltjahr prüfen
 *
 * \desc
 * Mit dieser Funktion wird geprüft, ob es sich bei dem in der Variable gespeicherten Jahr um ein
 * Schaltjahr handelt oder nicht.
 *
 * @return Liefert \c true zurück, wenn es sich um ein Schaltjahr handelt, andernfalls \c false.
 */
bool DateTime::isLeapYear() const
{
	return isLeapYear(yy);
}

/*!\brief Jahreszahl auf Schaltjahr prüfen
 *
 * \desc
 * Mit dieser statischen Funktion kann geprüft werden, ob es sich bei dem angegebenen Jahr \p year um ein
 * Schaltjahr handelt oder nicht.
 *
 * @param[in] year Das zu prüfende Jahr
 * @return Liefert \c true zurück, wenn es sich um ein Schaltjahr handelt, andernfalls \c false.
 */
bool DateTime::isLeapYear(int year)
{
	if (year%4!=0) return 0;
	if (year%400==0) return 1;
	if (year%100==0) return 0;
	return 1;
}

/*!\brief Aktuelles Datum zurückgeben
 *
 * \desc
 * Diese statische Funktion liefert das aktuelle Datum und die aktuelle Uhrzeit in Form einer
 * DateTime-Variablen zurück.
 * @return DateTime-Variable mit dem aktuellen Datum und Uhrzeit.
 */
DateTime DateTime::currentTime()
{
	DateTime d;
	d.setCurrentTime();
	return d;
}


/*!\brief Differenz in Sekunden
 *
 * \desc
 * Diese Funktion gibt die Differenz dieses DateTime zu dem angegebenen DateTime \p other in
 * Sekunden zurück. Liegt der Zeitpunkt von \p other vor diesem, ist der Rückgabewert negativ.
 *
 * Vor dem Vergleich werden beide Zeitwerte in UTC umgewandelt.
 * @param[in] other Zu vergleichender Zeitwert
 * @return Differenz in Sekunden
 */
pplint64 DateTime::diffSeconds(const DateTime &other) const
{
	pplint64 mySecs=(pplint64)time_t();
	pplint64 otherSecs=(pplint64)other.time_t();
	return otherSecs-mySecs;
}

/*!\brief Differenz in Sekunden mit Toleranz vergleichen
 *
 * \desc
 * Mit dieser Funktion wird die Differenz des Zeitwerts dieses DateTime mit der angegebenen DateTime \p other
 * auf Sekundenbasis berechnet und anschließend mit der angegebenen Toleranz \p tolerance verglichen.
 *
 * @param[in] other Zu vergleichender Zeitwert
 * @param[in] tolerance Optionaler Wert, der die akzeptable Toleranz beider Werte in Sekunden angibt
 * @return Sind beide Zeitwerte identisch oder liegen im Bereich der angegebenen Toleranz, gibt die Funktion
 * 1 zurück, andernfalls 0. Es wird kein Fehlercode gesetzt.
 */
int DateTime::compareSeconds(const DateTime &other, int tolerance) const
{
	pplint64 mySecs=(pplint64)time_t();
	pplint64 otherSecs=(pplint64)other.time_t();
	pplint64 diff=otherSecs-mySecs;
	if (diff<0) diff=mySecs-otherSecs;
	if (diff<=tolerance) return 1;
	return 0;
}
//@}



//!\name Operatoren


/*!\brief Datum aus einem String übernehmen
 *
 * \desc
 * Mit diesem Operator werden Datum und Uhrzeit aus dem String \p datetime übernommen.
 * Die unterstützten Formate sind in der Funktion
 * \ref DateTime::set(const String &datetime) "set" beschrieben.
 *
 * @param[in] datetime String mit Datum und Uhrzeit
 * @return Gibt eine Referenz auf den DateTime-Wert zurück
 *
 * \exception
 * Enthält der String \p datetime ein ungültiges oder unbekanntes Datumsformat, wird eine
 * "InvalidFormat" Exception geworfen. Ausnahmen: Ist der String leer oder enthält nur den
 * Buchstaben "T" oder den Wert "0" wird keine Exception geworfen, sondern der Datumswert auf 0 gesetzt.
 */
DateTime& DateTime::operator=(const String &datetime)
{
	if (!set(datetime)) throw InvalidDateException(datetime);
	return *this;
}

/*!\brief Datum aus einem anderen DateTime-Wert übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Wert eines anderen DateTime-Wertes übernommen.
 *
 * @param[in] other Referenz auf den zu kopierenden DateTime-Wert
 * @return Gibt eine Referenz auf den DateTime-Wert zurück
 */
DateTime& DateTime::operator=(const DateTime &other)
{
	set(other);
	return *this;
}

/*!\brief Operator, der einen String zurückliefert
 *
 * \desc
 * Dieser Operator liefert den Inhalt der Variablen als String in folgendem Format zurück:
 * "yyyy-mm-dd hh:ii:ss.mms".
 * @return Datums-String
 */
DateTime::operator String() const
{
	String r;
	r.setf("%04i-%02i-%02i %02i:%02i:%02i.%03i",yy,mm,dd,hh,ii,ss,ms);
	return r;
}


/*!\brief Vergleichsoperator "kleiner": <
 *
 * \desc
 * Mit diesem Operator werden zwei DateTime Werte miteinander verglichen. Die Funktion gibt \c true
 * zurück, wenn der erste Wert kleiner ist als der Zweite.
 *
 * @param other Der zweite Wert, mit dem der Vergleich durchgeführt werden soll
 * @return Gibt \c true zurück, wenn der erste Wert kleiner ist als der Zweite.
 */
bool DateTime::operator<(const DateTime &other) const
{
	ppluint64 v1=longInt();
	ppluint64 v2=other.longInt();
	if (v1<v2) return true;
	return false;
}

/*!\brief Vergleichsoperator "kleiner oder gleich": <=
 *
 * \desc
 * Mit diesem Operator werden zwei DateTime Werte miteinander verglichen. Die Funktion gibt \c true
 * zurück, wenn der erste Wert kleiner oder gleich groß ist, wie der Zweite.
 *
 * @param other Der zweite Wert, mit dem der Vergleich durchgeführt werden soll
 * @return Gibt \c true zurück, wenn der erste Wert kleiner oder gleich gross ist wie der Zweite.
 */
bool DateTime::operator<=(const DateTime &other) const
{
	ppluint64 v1=longInt();
	ppluint64 v2=other.longInt();
	if (v1<=v2) return true;
	return false;
}

/*!\brief Vergleichsoperator "gleich": ==
 *
 * \desc
 * Mit diesem Operator werden zwei DateTime Werte miteinander verglichen. Die Funktion gibt \c true
 * zurück, wenn beide Werte identisch sind.
 *
 * @param other Der zweite Wert, mit dem der Vergleich durchgeführt werden soll
 * @return Gibt \c true zurück, wenn beide Werte identisch sind.
 */
bool DateTime::operator==(const DateTime &other) const
{
	ppluint64 v1=longInt();
	ppluint64 v2=other.longInt();
	if (v1==v2) return true;
	return false;
}

/*!\brief Vergleichsoperator "ungleich": !=
 *
 * \desc
 * Mit diesem Operator werden zwei DateTime Werte miteinander verglichen. Die Funktion gibt \c true
 * zurück, wenn die Werte nicht übereinstimmen.
 *
 * @param other Der zweite Wert, mit dem der Vergleich durchgeführt werden soll
 * @return Gibt \c true zurück, wenn die Werte nicht übereinstimmen.
 */
bool DateTime::operator!=(const DateTime &other) const
{
	ppluint64 v1=longInt();
	ppluint64 v2=other.longInt();
	if (v1!=v2) return true;
	return false;
}

/*!\brief Vergleichsoperator "größer oder gleich": >=
 *
 * \desc
 * Mit diesem Operator werden zwei DateTime Werte miteinander verglichen. Die Funktion gibt \c true
 * zurück, wenn der erste Wert größer oder gleich groß ist, wie der Zweite.
 *
 * @param other Der zweite Wert, mit dem der Vergleich durchgeführt werden soll
 * @return Gibt \c true zurück, wenn der erste Wert größer oder gleich groß ist, wie der Zweite.
 */
bool DateTime::operator>=(const DateTime &other) const
{
	ppluint64 v1=longInt();
	ppluint64 v2=other.longInt();
	if (v1>=v2) return true;
	return false;
}

/*!\brief Vergleichsoperator "größer": >
 *
 * \desc
 * Mit diesem Operator werden zwei DateTime Werte miteinander verglichen. Die Funktion gibt \c true
 * zurück, wenn der erste Wert größer ist als der Zweite.
 *
 * @param other Der zweite Wert, mit dem der Vergleich durchgeführt werden soll
 * @return Gibt \c true zurück, wenn der erste Wert größer ist als der Zweite.
 */
bool DateTime::operator>(const DateTime &other) const
{
	ppluint64 v1=longInt();
	ppluint64 v2=other.longInt();
	if (v1>v2) return true;
	return false;
}


//@}

}		// EOF namespace ppl7

