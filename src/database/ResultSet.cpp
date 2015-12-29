/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
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
#include "ppl7.h"
#include "ppl7-db.h"


namespace ppl7 {
namespace db {


/*!\class ResultSet
 * \ingroup PPLGroupDatabases
 * \brief Ergebnis eines Datenbank-Selects
 *
 * \header \#include <ppl7-db.h>
 *
 * \descr
 * Dies ist eine virtuelle Basisklasse für das Aufnehmen des Ergebnisses eines Datenbank-Selects.
 * Die tatsächliche Implementierung ist Datenbank-spezifisch, daher hat jede Datenbank-Klasse eine
 * eigene abgeleitete Implementierung davon.
 * Mit Ihren Funktionen können die Ergebniszeilen aus einem Select durchwandert und ausgelesen werden.
 * \par
 * Das ResultSet wird durch Aufruf verschiedener Funktionen innerhalb der Klasse ppl7::db::Database (bzw.
 * davon abgeleiteten Klassen) erstellt und muss von der aufrufenden Anwendung selbst mit
 * \c delete freigegeben werden.
 *
 * \example
 * Das nachfolgende Beispiel zeigt unter Verwendung einer MySQL-Datenbank, wie die ResultSet-Klasse
 * verwendet werden kann.
 * \dontinclude db_examples.cpp
 * \skip DB_Result_Example1
 * \until EOF
 *
 */

/*!\enum ResultSet::Type
 * \brief Mögliche Datentypen
 */

/*!\var ResultSet::Type ResultSet::ERROR
 * Dieser Wert wird von der Funktion ResultSet::fieldType im Fehlerfall zurückgegeben.
 */

/*!\var ResultSet::Type ResultSet::UNKNOWN
 * Der Datentyp ist unbekannt
 */

/*!\var ResultSet::Type ResultSet::INTEGER
 * Ein 32-Bit Zahlenwert vom Typ Integer
 */

/*!\var ResultSet::Type ResultSet::LONGINTEGER
 * Ein 64-Bit Zahlenwert vom Typ Integer
 */

/*!\var ResultSet::Type ResultSet::FLOAT
 * Ein Dezimalwert einfacher Genauigkeit
 */

/*!\var ResultSet::Type ResultSet::DOUBLE
 * Ein Dezimalwert doppelter Genauigkeit
 */


/*!\var ResultSet::Type ResultSet::BOOLEAN
 * Ein boolscher Wert, "true" oder "false"
 */

/*!\var ResultSet::Type ResultSet::Timestamp
 * Ein Zeitstempel, meist UTC
 */

/*!\var ResultSet::Type ResultSet::DATETIME
 * Kombination aus Datum und Uhrzeit
 */

/*!\var ResultSet::Type ResultSet::STRING
 * Ein String
 */

/*!\var ResultSet::Type ResultSet::BINARY
 * Ein Binärwert
 */


/*!\fn db::ResultSet::ResultSet
 * \brief Konstruktor der Klasse
 *
 * Dies ist der Konstruktor der Klasse. Er sorgt dafür, dass die internen Variablen initialisiert
 * werden.
 */

/*!\fn db::ResultSet::~ResultSet
 * \brief Destruktor der Klasse
 *
 * Der Destruktor sorgt dafür, dass sämtlicher durch die Klasse allokierter Speicher wieder freigegeben
 * wird. Dazu wird die Funktion ResultSet::clear aufgerufen.
 */



/*!\fn db::ResultSet::clear
 * \brief Speicher freigeben, Result löschen
 *
 * \descr
 * Durch Aufruf dieser Funktion wird der interne Speicher und Datenbank- oder Result-spezifische
 * Handles freigegeben. Nach Aufruf der Funktion kann daher nicht mehr auf die vormals vorhandenen
 * Ergebnisse des Datenbank-Selects zugegriffen werden. Die Funktion braucht in der Regel nicht
 * aufgerufen zu werden, da dies der Destruktor der Klasse automatisch macht.
 */

/*!\fn ppl7::db::ResultSet::rows
 * \brief Anzahl Zeilen im Ergebnis
 *
 * Diese Funktion liefert die Anzahl Zeilen im Ergebnis des vorhergehenden Selects zurück.
 * @return Anzahl Zeilen (rows).
 *
 * \note Je nach Datenbank kann es sein, dass die Funktion nicht die Gesamtzahl der Zeilen im
 * Ergebniss zurückliefert, sondern nur einen Teil oder sogar nur 1.
 *
 * \exception UnavailableException Wird geworfen, wenn der Datenbank-Treiber die Funktion nicht
 * unterstützt.
 *
 * \see
 * ResultSet::fields gibt die Anzahl Spalten im Ergebnis zurück
 */

/*!\fn ResultSet::affected
 * \brief Betroffene Zeilen
 *
 * Liefert nach einem Insert, Update oder Replace die Anzahl Datensätze zurück, die durch das Update
 * verändert wurden. Die Information kann auch über Funktion Database::getAffectedRows der
 * Datenbank-Klasse ausgelesen werden.
 *
 * @return Anzahl betroffender Datensätze
 */

/*!\fn ResultSet::fields
 * \brief Anzahl Spalten im Ergebnis
 *
 * Diese Funktion liefert die Anzahl Spalten im Ergebnis des vorhergehenden Selects zurück.
 *
 * @return Anzahl Spalten.
 */


/*!\fn ResultSet::getString(const String &fieldname)
 * \brief Wert eines bestimmten Feldes
 *
 * \descr
 * Mit dieser Funktion wird der Wert des Feldes mit dem Namen \p fieldname aus der
 * aktuellem Ergebniszeile als String zurückgegeben.
 *
 * @param[in] fieldname Der Name des auszulesenden Feldes
 * @return Ist das Feld vorhanden, wird der Wert des Feldes als String
 * zurückgegeben. Andernfalls wird eine Exception geworfen.
 * @exception FieldNotInResultSetException Wird geworfen, wenn das Feld nicht im Resultset vorhanden ist.
 */

/*!\fn ResultSet::getString(int field)
 * \brief Wert eines bestimmten Feldes
 *
 * \descr
 * Mit dieser Funktion wird der Wert des Feldes in Spalte \p field aus der
 * aktuellem Ergebniszeile als String zurückgegeben.
 *
 * @param[in] field Der Nummer der auszulesenden Spalte
 * @return Ist das Feld vorhanden, wird der Wert des Feldes als String
 * zurückgegeben. Andernfalls wird eine Exception geworfen.
 * @exception FieldNotInResultSetException Wird geworfen, wenn das Feld nicht im Resultset vorhanden ist.
 */


/*!\fn ResultSet::fieldNum
 * \brief Spalte eines bestimmten Feldes herausfinden
 *
 * Diese Funktion liefert die Spaltennummer des Feldes mit dem Namen \p fieldname zurück, sofern
 * dieses vorhanden ist.
 *
 * @param[in] fieldname Name des gesuchten Feldes
 * @return Ist das Feld vorhanden, wird dessen Spaltennummer zurückgegeben. Die Zählung der Spalten
 * fängt dabei bei 0 an. Im Fehlerfall wird eine Exception geworfen.
 * @exception FieldNotInResultSetException Wird geworfen, wenn das Feld nicht im Resultset vorhanden ist.
 */

/*!\fn ResultSet::fieldName
 * \brief Name einer bestimmten Spalte herausfinden
 *
 * Dies ist die Umkehrfunktion zu ResultSet::FieldNum. Sie liefert den Namen eines bestimmten Feldes
 * anhand dessen Spaltennummer \p num zurück, sofern sie vorhanden ist.
 *
 * @param[in] field Gewünschte Spaltennummer
 * @return Ist die Spalte vorhanden, wird deren Namen als String zurückgegeben,
 * im Fehlerfall wird eine Exception geworfen.
 * @exception FieldNotInResultSetException Wird geworfen, wenn das Feld nicht im Resultset vorhanden ist.
 */

/*!\fn ResultSet::fieldType(int field)
 * \brief Typ eines Feldes auslesen
 *
 * Mit dieser Funktion kann man abfragen, was für ein Datentyp das Feld \p num ist.
 *
 * @param[in] field Die gewünschte Spalte
 * @return Liefert einen Wert vom Typ ResultSet::FieldType zurück,
 * im Fehlerfall wird eine Exception geworfen.
 *
 * @note Gegenwärtig ist diese Methode noch nicht implementiert und liefert daher TYPE_STRING
 * zurück, sofern das Feld vorhanden ist.
 */

/*!\fn ResultSet::fieldType(const String &fieldname)
 * \brief Typ eines Feldes auslesen
 *
 * Mit dieser Funktion kann man abfragen, was für ein Datentyp das Feld mit dem Namen \p fieldname ist.
 *
 * @param[in] fieldname Die gewünschte Spalte
 * @return Liefert einen Wert vom Typ ResultSet::FieldType zurück, im Fehlerfall wird eine Exception geworfen.
 * @note Gegenwärtig ist diese Methode noch nicht implementiert und liefert daher TYPE_STRING
 * zurück, sofern das Feld vorhanden ist.

 */

/*!\fn ResultSet::fetchArray()
 * \brief Zeile in ein Assoziatives Array kopieren
 *
 * Mit dieser Funktion wird eine komplette Ergebniszeile in ein Assoziatives Array vom Typ
 * AssocArray kopiert und der interne Zeiger auf den nächsten Datensatz vorgerückt. Durch
 * wiederholtes Aufrufen der Funktion kann man somit nach und nach alle Ergebniszeilen
 * durchwandern.
 *
 * \return Liefert ein AssocArray mit Feldern und Werten der aktuellen Ergebniszeile zurück.
 * Im Fehlerfall oder falls keine weiteren Ergebniszeilen vorhanden sind, wird eine Exception
 * geworfen.
 * \exception NoResultException Wird geworfen, wenn keine weitere Zeile im Resultset vorhanden ist.
 * \note Um das werfen einer Exception am Ende des Resultsets zu vermeiden, kann vorher mit der
 * Methode ResultSet::eof geprüft werden, ob eine weitere Zeile bereitsteht.
 * \remarks
 * Bei Aufruf dieser Funktion werden die Daten der aktuellen Zeile mehrfach kopiert. Die Funktion
 * ResultSet::FetchArray(AssocArray &array) ist daher vorzuziehen.
 */

/*!\fn ResultSet::FetchArray(AssocArray &array)
 * \brief Zeile in ein Assoziatives Array kopieren
 *
 * Mit dieser Funktion wird eine komplette Ergebniszeile in das Assoziative Array
 * \p array kopiert und der interne Zeiger auf den nächsten Datensatz vorgerückt. Durch
 * wiederholtes Aufrufen der Funktion kann man somit nach und nach alle Ergebniszeilen
 * durchwandern.
 *
 * \param[out] array Ein Objekt vom Typ AssocArray, in dem das Ergebnis gespeichert werden
 * soll. Daten, die vor Aufruf in \p array enthalten sind, werden durch die Funktion gelöscht.
 * \note Um das werfen einer Exception am Ende des Resultsets zu vermeiden, kann vorher mit der
 * Methode ResultSet::eof geprüft werden, ob eine weitere Zeile bereitsteht.
 *
 * \exception NoResultException Wird geworfen, wenn keine weitere Zeile im Resultset vorhanden ist.
 */

/*!\fn ResultSet::fetchFields()
 * \brief Zeile in ein Array kopieren
 *
 * Mit dieser Funktion wird eine komplette Ergebniszeile in ein Array vom Typ
 * Array kopiert und der interne Zeiger auf den nächsten Datensatz vorgerückt. Durch
 * wiederholtes Aufrufen der Funktion kann man somit nach und nach alle Ergebniszeilen
 * durchwandern.
 *
 * \return Liefert ein Array mit Feldern und Werten der aktuellen Ergebniszeile zurück.
 * Im Fehlerfall oder falls keine weiteren Ergebniszeilen vorhanden sind, wird eine Exception geworfen.
 * \exception NoResultException Wird geworfen, wenn keine weitere Zeile im Resultset vorhanden ist.
 * \note Um das werfen einer Exception am Ende des Resultsets zu vermeiden, kann vorher mit der
 * Methode ResultSet::eof geprüft werden, ob eine weitere Zeile bereitsteht.
 *
 * \remarks
 * Bei Aufruf dieser Funktion werden die Daten der aktuellen Zeile mehrfach kopiert. Die Funktion
 * ResultSet::FetchFields(Array &array) ist daher vorzuziehen.
 */

/*!\fn ResultSet::fetchFields(Array &array)
 * \brief Zeile in ein Array kopieren
 *
 * Mit dieser Funktion wird eine komplette Ergebniszeile in das Array
 * \p array kopiert und der interne Zeiger auf den nächsten Datensatz vorgerückt. Durch
 * wiederholtes Aufrufen der Funktion kann man somit nach und nach alle Ergebniszeilen
 * durchwandern.
 *
 * \param[out] array Ein Objekt vom Typ Array, in dem das Ergebnis gespeichert werden
 * soll. Daten, die vor Aufruf in \p array enthalten sind, werden durch die Funktion gelöscht.
 * \exception NoResultException Wird geworfen, wenn keine weitere Zeile im Resultset vorhanden ist.
 * \note Um das werfen einer Exception am Ende des Resultsets zu vermeiden, kann vorher mit der
 * Methode ResultSet::eof geprüft werden, ob eine weitere Zeile bereitsteht.
 */

/*!\fn ResultSet::nextRow
 * \brief Nächste Result-Zeile
 *
 * Durch Aufruf dieser Funktion wird die nächste Zeile innerhalb des
 * ResultSets geladen.
 *
 * \note Bei Verwendung der Methoden ResultSet::fetchArray oder ResultSet::fetchFields wird
 * automatisch die nächste Zeile geladen.
 *
 */


/*!\fn ResultSet::eof
 * \brief Prüfen, ob eine (weitere) Ergebniszeile vorhanden ist
 *
 * Mit dieser Methode kann geprüft werden, ob eine weitere Ergebniszeile vorhanden ist.
 *
 * \return \p true = weitere Ergebniszeile steht bereit, \p false = keine weitere Zeilen vorhanden.
 */

/*!\brief Result als Liste mit assoziativen Arrays exportieren
 *
 * \descr
 * Mit diesem Befehl wird das Result-Set dieses Objekts als Strings mit
 * Key-Value-Paaren in ein AssocArray kopiert.
 *
 * \param[in] res Pointer auf ein ResultSet
 * \param[out] array Ein AssocArray, das das ResultSet aufnehmen soll
 * \exception IllegalArgumentException Wird geworfen, wenn \p res auf NULL zeigt
 */
void copyResultToAssocArray(ResultSet *res, AssocArray &array)
{
	if (!res) throw IllegalArgumentException();
	array.clear();
	AssocArray row;
	while (!res->eof()) {
		res->fetchArray(row);
		array.set("[]",row);
	}
}




}	// EOF namespace db
}	// EOF namespace ppl6
