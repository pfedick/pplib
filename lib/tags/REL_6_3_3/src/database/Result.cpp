/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.11 $
 * $Date: 2009/09/05 07:33:21 $
 * $Id: Result.cpp,v 1.11 2009/09/05 07:33:21 patrick Exp $
 *
 *******************************************************************************
 * Copyright (c) 2009, Patrick Fedick <patrick@pfp.de>
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
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#include "ppl6.h"
#include "ppl6-db.h"


namespace ppl6 {
namespace db {


/*!\class Result
 * \ingroup PPLGroupDatabases
 * \brief Basisklasse für Ergebnisse aus Datenbank-Selects
 *
 * \header \#include <ppl6-db.h>
 *
 * \descr
 * Dies ist eine virtuelle Basisklasse für das Aufnehmen des Ergebnisses eines Datenbank-Selects.
 * Die tatsächliche Implementierung ist Datenbank-spezifisch, daher hat jede Datenbank-Klasse eine
 * eigene abgeleitete Implementierung davon.
 * Mit Ihren Funktionen können die Ergebniszeilen aus einem Select durchwandert und ausgelesen werden.
 * \par
 * Die Klasse wird durch Aufruf verschiedener Funktionen innerhalb der Klasse ppl6::db::Database (bzw.
 * davon abgeleiteten Klassen) erstellt und muss von der aufrufenden Anwendung selbst mit
 * \c delete gelöscht werden.
 *
 * \example
 * Das nachfolgende Beispiel zeigt unter Verwendung einer MySQL-Datenbank, wie die Result-Klasse
 * verwendet werden kann.
 * \dontinclude db_examples.cpp
 * \skip DB_Result_Example1
 * \until EOF
 *
 */

/*!\enum Result::Type
 * \brief Mögliche Datentypen
 */

/*!\var Result::Type Result::Error
 * Dieser Wert wird von der Funktion Result::FieldType im Fehlerfall zurückgegeben.
 */

/*!\var Result::Type Result::Unknown
 * Der Datentyp ist unbekannt
 */

/*!\var Result::Type Result::Integer
 * Ein Zahlenwert vom Typ Integer
 */

/*!\var Result::Type Result::Decimal
 * Ein Zahlenwert mit Kommastellen. Häufig ein Float oder Double
 */

/*!\var Result::Type Result::Bit
 * Ein boolscher Wert
 */

/*!\var Result::Type Result::Timestamp
 * Ein Zeitstempel, meist UTC
 */

/*!\var Result::Type Result::Date
 * Ein Datumswert
 */

/*!\var Result::Type Result::Time
 * Eine Uhrzeit
 */

/*!\var Result::Type Result::DateTime
 * Kombination aus Datum und Uhrzeit
 */

/*!\var Result::Type Result::String
 * Ein String
 */

/*!\var Result::Type Result::Binary
 * Ein großer String oder Binärwert
 */

/*!\var Result::Type Result::Enum
 * Eine Enumeration
 */



Result::Result()
/*!\brief Konstruktor der Klasse
 *
 * Dies ist der Konstruktor der Klasse. Er sorgt dafür, dass die internen Variablen initialisiert
 * werden.
 */
{
}

Result::~Result()
/*!\brief Destruktor der Klasse
 *
 * Der Destruktor sorgt dafür, dass sämtlicher durch die Klasse allokierter Speicher wieder freigegeben
 * wird. Dazu wird die Funktion Result::Clear aufgerufen.
 */
{
}


int Result::Export(CAssocArray &array)
/*!\brief Result als assoziatives Array exportieren
 *
 * \descr
 * Mit diesem Befehl wird das Result-Set dieses Objekts als Strings mit
 * Key-Value-Paaren in ein CAssocArray kopiert. Das Array kann in ein
 * GenericResult-Objekt mittels GenericResult::Import wieder importiert werden.
 * Da alles als String exportiert wird, gehen die Datentyp-Informationen
 * verloren.
 *
 * \param[out] array Ein assoziatives Array, in dem das Result-Set gespeichert werden soll
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	array.Clear();
	CAssocArray row;
	pplint64 rows=Rows();
	for (pplint64 r=0;r<rows;r++) {
		if (FetchArray(row,r)) array.Set("[]",row);
		else return 0;
	}
	return 1;
}

void Result::Clear()
/*!\brief Speicher freigeben, Result löschen
 *
 * \descr
 * Durch Aufruf dieser Funktion wird der interne Speicher und Datenbank- oder Result-spezifische
 * Handles freigegeben. Nach Aufruf der Funktion kann daher nicht mehr auf die vormals vorhandenen
 * Ergebnisse des Datenbank-Selects zugegriffen werden. Die Funktion braucht in der Regel nicht
 * aufgerufen zu werden, da dies auch der Destruktor der Klasse macht.
 */
{
}

pplint64 Result::Rows()
/*!\brief Anzahl Zeilen im Ergebnis
 *
 * Diese Funktion liefert die Anzahl Zeilen im Ergebnis des vorhergehenden Selects zurück.
 * @return Anzahl Zeilen (rows). Im Fehlerfall wird -1 zurückgegeben.
 *
 * \see
 * Result::Fields gibt die Anzahl Spalten im Ergebnis zurück
 */
{
	SetError(180);
	return -1;
}

pplint64 Result::Affected()
/*!\brief Betroffene Zeilen
 *
 * War der Datenbank-Query ein Select, liefert diese Funktion genau wie Result::Rows die Anzahl
 * Zeilen im Ergebnis zurück. Handelete es sich um ein Update/Insert/Replace, wird die Anzahl
 * betroffener bzw. veränderter Datensätze zurückgegeben. Die Information kann auch über Funktion
 * Database::GetAffectedRows der Datenbank-Klasse ausgelesen werden.
 *
 * @return Anzahl betroffender Datensätze, im Fehlerfall -1
 */
{
	SetError(180);
	return -1;
}

int Result::Fields()
/*!\brief Anzahl Spalten im Ergebnis
 *
 * Diese Funktion liefert die Anzahl Spalten im Ergebnis des vorhergehenden Selects zurück.
 *
 * @return Anzahl Spalten oder -1 im Fehlerfall.
 */
{
	return -1;
}

const char *Result::Get(pplint64 row, const char *fieldname)
/*!\brief Wert eines bestimmten Feldes
 *
 * \descr
 * Mit dieser Funktion wird der Wert des Feldes mit dem Namen \p fieldname aus der
 * Ergebniszeile \p row als String zurückgegeben.
 *
 * @param[in] row Die gewünschte Ergebniszeile
 * @param[in] fieldname Der Name des auszulesenden Feldes
 * @return Ist das Feld vorhanden, wird ein Pointer auf dessen Inhalt zurückgegeben, im
 * Fehlerfall NULL.
 */
{
	SetError(180);
	return NULL;
}

const char *Result::Get(pplint64 row, int field)
/*!\brief Wert eines bestimmten Feldes
 *
 * \descr
 * Mit dieser Funktion wird der Wert des Feldes in Spalte \p field aus der
 * Ergebniszeile \p row als String zurückgegeben.
 *
 * @param[in] row Die gewünschte Ergebniszeile
 * @param[in] field Der Nummer der auszulesenden Spalte
 * @return Ist das Feld vorhanden, wird ein Pointer auf dessen Inhalt zurückgegeben, im
 * Fehlerfall NULL.
 */
{
	SetError(180);
	return NULL;
}

int Result::FieldNum(const char *fieldname)
/*!\brief Spalte eines bestimmten Feldes herausfinden
 *
 * Diese Funktion liefert die Spaltennummer des Feldes mit dem Namen \p fieldname zurück, sofern
 * dieses vorhanden ist.
 *
 * @param[in] fieldname Name des gesuchten Feldes
 * @return Ist das Feld vorhanden, wird dessen Spaltennummer zurückgegeben. Die Zählung der Spalten
 * fängt dabei bei 0 an. Im Fehlerfall wird -1 zurückgegeben.
 */
{
	SetError(180);
	return -1;
}

const char *Result::FieldName(int num)
/*!\brief Name einer bestimmten Spalte herausfinden
 *
 * Dies ist die Umkehrfunktion zu Result::FieldNum. Sie liefert den Namen eines bestimmten Feldes
 * anhand dessen Spaltennummer \p num zurück, sofern sie vorhanden ist.
 *
 * @param[in] num Gewünschte Spaltennummer
 * @return Ist die Spalte vorhanden, wird deren Namen als String zurückgegeben, im Fehlerfall NULL.
 */
{
	SetError(180);
	return NULL;
}

Result::Type Result::FieldType(int num)
/*!\brief Typ eines Feldes auslesen
 *
 * Mit dieser Funktion kann man abfragen, was für ein Datentyp das Feld \p num ist.
 *
 * @param[in] num Die gewünschte Spalte
 * @return Liefert einen Wert vom Typ Result::Type zurück, im Fehlerfall Result:Error.
 */
{
	SetError(180);
	return Result::Error;
}

Result::Type Result::FieldType(const char *fieldname)
/*!\brief Typ eines Feldes auslesen
 *
 * Mit dieser Funktion kann man abfragen, was für ein Datentyp das Feld mit dem Namen \p fieldname ist.
 *
 * @param[in] fieldname Die gewünschte Spalte
 * @return Liefert einen Wert vom Typ Result::Type zurück, im Fehlerfall Result:Error.
 */
{
	SetError(180);
	return Result::Error;
}

CAssocArray Result::FetchArray(pplint64 row)
/*!\brief Zeile in ein Assoziatives Array kopieren
 *
 * Mit dieser Funktion wird eine komplette Ergebniszeile in ein Assoziatives Array vom Typ
 * CAssocArray kopiert und der interne Zeiger auf den nächsten Datensatz vorgerückt. Durch
 * wiederholtes Aufrufen der Funktion kann man somit nach und nach alle Ergebniszeilen
 * durchwandern.
 *
 * \param[in] row Ein Optionaler Parameter, der die gewünschte Ergebniszeile angibt. Wird
 * der Parameter weggelassen, wird der Datensatz zurückgegeben, auf den der interne Zeiger
 * zeigt, und der Zeiger auf den nächsten Datensatz vorgerückt.
 * \return Liefert ein CAssocArray mit Feldern und Werten der aktuellen Ergebniszeile zurück.
 * Im Fehlerfall oder falls keine weiteren Ergebniszeilen vorhanden sind, wird ein leeres
 * CAssocArray zurückgeliefert.
 * \remarks
 * Bei Aufruf dieser Funktion werden die Daten der aktuellen Zeile mehrfach kopiert. Die Funktion
 * Result::FetchArray(CAssocArray &array, ppluint64 row) ist daher vorzuziehen.
 */
{
	SetError(180);
	CAssocArray a;
	return a;
}

int Result::FetchArray(CAssocArray &array, pplint64 row)
/*!\brief Zeile in ein Assoziatives Array kopieren
 *
 * Mit dieser Funktion wird eine komplette Ergebniszeile in das Assoziative Array
 * \p array kopiert und der interne Zeiger auf den nächsten Datensatz vorgerückt. Durch
 * wiederholtes Aufrufen der Funktion kann man somit nach und nach alle Ergebniszeilen
 * durchwandern.
 *
 * \param[out] array Ein Objekt vom Typ CAssocArray, in dem das Ergebnis gespeichert werden
 * soll. Daten, die vor Aufruf in \p array enthalten sind, werden durch die Funktion gelöscht.
 * \param[in] row Ein Optionaler Parameter, der die gewünschte Ergebniszeile angibt. Wird
 * der Parameter weggelassen, wird der Datensatz zurückgegeben, auf den der interne Zeiger
 * zeigt, und der Zeiger auf den nächsten Datensatz vorgerückt.
 * \return Im Erfolgsfall liefert die Funktion 1 zurück, im Fehlerfall oder
 * falls keine weiteren Ergebniszeilen vorhanden sind 0.
 */
{
	SetError(180);
	return 0;
}

CArray Result::FetchFields(pplint64 row)
/*!\brief Zeile in ein Array kopieren
 *
 * Mit dieser Funktion wird eine komplette Ergebniszeile in ein Array vom Typ
 * CArray kopiert und der interne Zeiger auf den nächsten Datensatz vorgerückt. Durch
 * wiederholtes Aufrufen der Funktion kann man somit nach und nach alle Ergebniszeilen
 * durchwandern.
 *
 * \param[in] row Ein Optionaler Parameter, der die gewünschte Ergebniszeile angibt. Wird
 * der Parameter weggelassen, wird der Datensatz zurückgegeben, auf den der interne Zeiger
 * zeigt, und der Zeiger auf den nächsten Datensatz vorgerückt.
 * \return Liefert ein CArray mit Feldern und Werten der aktuellen Ergebniszeile zurück.
 * Im Fehlerfall oder falls keine weiteren Ergebniszeilen vorhanden sind, wird ein leeres
 * CArray zurückgeliefert.
 * \remarks
 * Bei Aufruf dieser Funktion werden die Daten der aktuellen Zeile mehrfach kopiert. Die Funktion
 * Result::FetchFields(CArray &array, ppluint64 row) ist daher vorzuziehen.
 */
{
	SetError(180);
	CArray a;
	return a;
}

int Result::FetchFields(CArray &array, pplint64 row)
/*!\brief Zeile in ein Array kopieren
 *
 * Mit dieser Funktion wird eine komplette Ergebniszeile in das Array
 * \p array kopiert und der interne Zeiger auf den nächsten Datensatz vorgerückt. Durch
 * wiederholtes Aufrufen der Funktion kann man somit nach und nach alle Ergebniszeilen
 * durchwandern.
 *
 * \param[out] array Ein Objekt vom Typ CArray, in dem das Ergebnis gespeichert werden
 * soll. Daten, die vor Aufruf in \p array enthalten sind, werden durch die Funktion gelöscht.
 * \param[in] row Ein Optionaler Parameter, der die gewünschte Ergebniszeile angibt. Wird
 * der Parameter weggelassen, wird der Datensatz zurückgegeben, auf den der interne Zeiger
 * zeigt, und der Zeiger auf den nächsten Datensatz vorgerückt.
 * \return Im Erfolgsfall liefert die Funktion 1 zurück, im Fehlerfall oder
 * falls keine weiteren Ergebniszeilen vorhanden sind 0.
 */
{
	SetError(180);
	return 0;
}

int Result::Seek(pplint64 row)
/*!\brief Internen Zeiger auf die gewünschte Ergebniszeile setzen
 *
 * Mit dieser Funktion wird der interne Datenzeiger auf die gewünschte Zeile \p row gesetzt,
 * so dass diese beim nächsten Aufruf von Result::FetchArray zurückgeliefert wird.
 *
 * @param[in] row Die gewünschte Zeile
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	SetError(180);
	return 0;
}

void Result::PrintResult()
/*!\brief Ergebnis auf STDOUT ausgeben
 *
 * Durch AUfruf dieser Funktion wird das Ergebnis des Selects auf STDOUT ausgegeben.
 *
 */
{
	SetError(180);
}

}	// EOF namespace db
}	// EOF namespace ppl6
