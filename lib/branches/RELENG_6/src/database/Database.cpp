/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.8 $
 * $Date: 2010/10/16 13:35:42 $
 * $Id: Database.cpp,v 1.8 2010/10/16 13:35:42 pafe Exp $
 *
 *******************************************************************************
 * Copyright (c) 2010, Patrick Fedick <patrick@pfp.de>
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


Database *Connect(const CAssocArray &params)
/*!\ingroup PPLGroupDatabases
 * \brief Verbindung zu einer Datenbank herstellen
 *
 * \header \#include <ppl6-db.h>
 *
 * \descr
 * Mit dieser Funktion wird eine neue Datenbank-Klasse erstellt und eine Verbindung zu der gewünschten
 * Datenbank aufgebaut. Die für die Erstellung der Klasse und die für den Connect erforderlichen oder
 * optionalen Parameter werden mit dem Array \p params übergeben.
 * \par
 * Die erforderlichen oder optionalen Parameter sind Datenbank-abhängig und sind in der
 * jeweiligen Dokumentation zu finden. Es gibt jedoch eine Reihe von Parametern, die bei allen
 * Datenbanken identisch sind:
 * - \b host: Der Hostname oder die IP-Adresse des Datenbank-Servers
 * - \b port: Der TCP-Port des Datenbank-Servers
 * - \b dbname: Der Name der intialen Datenbank. Dieser Parameter kann optional sein,
 *   da mit der Funktion Database::SelectDB die Datenbank auch später noch gewechselt werden kann.
 * - \b user: Der Name des Benutzers, mit dem sich an der Datenbank authentifiziert werden soll
 * - \b password: Das Passwort des Benutzers im Klartext
 * \par
 * Hinzu kommt noch der Parameter \b type, der den Typ der gewünschten Datenbank angibt.
 * Unterstützt werden zur Zeit folgende Datenbanken:
 * - \b mysql: MySQL-Datenbank
 * - \b postgres: Postgres-Datenbank
 * - \b sybase: Sybase-Datenbank
 * Die tatsächlich unterstützten Datenbanken hängen davon ab, wie PPL6 kompiliert wurde.
 * \par
 * Postgres unterstützt zusätzlich noch die Angabe des Suchpfades:
 * - \b searchpath: Kommaseparierte Liste mit den Schemata, die in den Suchpfad
 *      aufgenommen werden sollen
 *
 * \param params Ein Assoziatives Array mit den für den Connect erforderlichen Parameter.
 *
 * \return Bei Erfolg liefert die 1 zurück, im Fehlerfall 0.
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_Example1
 * \until EOF
 */
{
	CString type=params["type"];
	if (type.IsEmpty()) {
		SetError(341,"type");
		return NULL;
	}
	type.LCase();
	Database *db=NULL;
	#ifdef HAVE_MYSQL
		if (type=="mysql") {
			db=new MySQL;
		}
	#endif
	#ifdef HAVE_SYBASE
		if (type=="sybase") {
			db=new Sybase;
		}
	#endif
	#ifdef HAVE_POSTGRESQL
		if (type=="postgres") {
			db=new Postgres;
		}
	#endif
	#ifdef HAVE_SQLITE
		if (type=="sqlite") {
			db=new SQLite;
		}
	#endif
	if (!db) {
		SetError(342,type);
		return NULL;
	}
	if (!db->Connect(params)) {
		PushError();
		delete db;
		PopError();
		return NULL;
	}
	return db;
}

void GetSupportedDatabases(CAssocArray &a)
{
	a.Clear();
#ifdef HAVE_MYSQL
	a.Add("mysql/type","mysql");
	a.Add("mysql/name","MySQL");
#endif
#ifdef HAVE_SYBASE
	a.Add("sybase/type","sybase");
	a.Add("sybase/name","Sybase Open Client / ASE");
#endif
#ifdef HAVE_POSTGRESQL
	a.Add("postgres/type","postgres");
	a.Add("postgres/name","PostgreSQL");
#endif
#ifdef HAVE_SQLITE
	a.Add("sqlite/type","sqlite");
	a.Add("sqlite/name","SQLite");
#endif
}


/*!\class Database
 * \ingroup PPLGroupDatabases
 * \brief Basisklasse für verschiedene Datenbanken
 *
 * \header \#include <ppl6-db.h>
 *
 * \descr
 * Die Klasse \b Database ist eine abstrakte Basisklasse, von der die eigentliche Datenbank-spezifische Implementierung
 * abgeleitet wird. Sie enthält neben rein virtuellen Funktionen, deren Implementierung in der abgeleiteten
 * Klasse erfolgen muss, auch Funktionen, die Datenbank-unabhängig sind.
 * \par
 * Die Klasse kann nicht direkt instanziiert werden, stattdessen sollte immer die jeweilige
 * Datenbank-Klasse (siehe MySQL, Sybase, Postgres) oder die Funktion ppl6::db::Connect verwendet
 * werden.
 *
 * \example Verwendung der Funktion "ConnectDatabase"
 * \dontinclude db_examples.cpp
 * \skip DB_Example1
 * \until EOF
 *
 * \example Verwendung der Klasse "MySQL"
 * \dontinclude db_examples.cpp
 * \skip DB_Example3
 * \until EOF
 *
 */


Database::Database()
/*!\brief Konstruktor der Klasse
 *
 * \descr
 * Konstruktor der Klasse
 */
{
	lastuse=0;
	lastping=0;
	Log=NULL;
	pool=NULL;
	poolex=NULL;
}

Database::~Database()
/*!\brief Destruktor der Klasse
 *
 * \descr
 * Destruktor der Klasse.
 */
{
	Disconnect();
	if (Log) Log->Printf(LOG::DEBUG,1,"ppl6::db::Database","SetLogfile",__FILE__,__LINE__,"Stop logging of Database-Queries");
}

void Database::SetLogfile(CLog *log)
/*!\brief Querylog aktivieren oder deaktivieren
 *
 * \descr
 * Mit dieser Funktion kann ein Querylog aktiviert oder deaktiviert werden. Ist es aktiviert, wird bei jedem
 * Query ein Datensatz in das Logfile geschrieben, dem man neben Datum, Uhrzeit und Query auch entnehmen
 * kann, wie lang der Query gebraucht hat.
 *
 * @param[in] log Pointer auf eine Klasse vom Typ CLog, um das Logging zu aktivieren, oder NULL
 * um es zu deaktivieren.
 */
{
	if (log==NULL && Log!=NULL) {
		Log->Printf(LOG::DEBUG,1,"ppl6::db::Database","SetLogfile",__FILE__,__LINE__,"Stop logging of Database-Queries");
	}
	Log=log;
	if (Log) Log->Printf(LOG::DEBUG,1,"ppl6::db::Database","SetLogfile",__FILE__,__LINE__,"Start logging of Database-Queries");
}

void Database::SetLogfile(CLog &log)
/*!\brief Querylog aktivieren
 *
 * \descr
 * Mit dieser Funktion kann ein Querylog aktiviert werden. Ist es aktiviert, wird bei jedem
 * Query ein Datensatz in das Logfile geschrieben, dem man neben Datum, Uhrzeit und Query auch entnehmen
 * kann, wie lang der Query gebraucht hat.
 *
 * @param[in] log Referenz auf eine Klasse vom Typ CLog.
 *
 * \note
 * Soll das Logging deaktiviert werden, muss die Funktion mit NULL oder ohne Parameter aufgerufen werden.
 */
{
	Log=&log;
	if (Log) Log->Printf(LOG::DEBUG,1,"ppl6::db::Database","SetLogfile",__FILE__,__LINE__,"Start logging of Database-Queries");
}


CLog *Database::GetLogfile()
{
	return Log;
}


void Database::LogQuery(const char *query, float duration)
/*!\brief Interne Funktion zum Loggen von Queries
 *
 * \descr
 * Diese Funktion wird intern aufgerufen, um einen Query in das Logfile zu schreiben, sofern dieses
 * über Database::SetLogfile aktibviert wurde.
 *
 * @param[in] query Der durchgeführte Query
 * @param[in] duration Laufzeit des Queries in Sekunden.
 */
{
	if (!Log) return;
	Log->Printf(LOG::DEBUG,4,__FILE__,__LINE__,(const char*)NULL,0,"Querytime: %0.6f, Query: %s",duration,query);
}

void Database::UpdateLastPing()
/*!\brief Uhrzeit der letzten Datenbank-Kommunikation aktualisieren
 *
 * \descr
 * Diese interne Funktion wird immer dann aufgerufen, wenn eine erfolgreiche Kommunikation mit dem Datenbank-Server
 * stattgefunden hat. Dabei kann es sich um einen Query der Anwendung gehandelt haben aber auch um einen
 * Ping, der automatisch durch die Verwaltung des Datenbank-Pools abgeschickt wurde (siehe Pool und PoolEx).
 * Sie aktualisiert den Wert Database::lastping mit dem aktuellen Timestamp. Dieser Wert enthält somit immer
 * einen Zeitstempel, zu dem zuletzt eine Verbindung zum Server bestand.
 */
{
	lastping=ppl6::GetTime();
}

void Database::UpdateLastUse()
/*!\brief Uhrzeit der letzten Datenbank-Verwendung aktualisieren
 *
 * \descr
 * Diese interne Funktion wird immer dann aufgerufen, wenn die Anwendung einen Query erfolgreich auf dem
 * Datenbank-Server durchgeführt hat. Sie aktualisiert den Wert Database::lastuse mit dem aktuellen Timestamp.
 * Dieser Wert enthält somit immer einen Zeitstempel, zu dem die diese Instanz der Datenbank zuletzt
 * durch die Anwendung verwendet wurde. Diese Information wird von den Datenbank-Pools Pool und PoolEx verwendet,
 * um zu entscheiden, wann eine Datenbank-Verbindung nicht mehr gebraucht wird und aus dem Pool entfernt werden
 * kann.
 */
{
	lastping=ppl6::GetTime();
	lastuse=lastping;
}

void Database::ClearLastUse()
/*!\brief Timestamps auf 0 setzen
 *
 * \descr
 * Die Werte Database::lastping und Database::lastuse werden auf 0 gesetzt. Es ist unklar, wann diese Funktion
 * aufgerufen wird.
 */
{
	lastping=lastuse=0;
}

void Database::SetParam(const char *name, const char *value)
/*!\brief Parameter für den Connect setzen
 *
 * \descr
 * Die Datenbank-Klasse unterstützt zwei Connect-Funktionen: eine ohne Parameter und eine mit einem
 * Assoziativen Array als Parameter. Damit die Connect-Funktion ohne Parameter überhaupt funktioniert, müssen
 * diese zuvor mit SetParam gesetzt werden.
 *
 * @param[in] name Name des Parameters
 * @param[in] value Wert des Parameters als String
 */
{
	ConnectParam.Set(name,value);
}

void Database::SetParam(const char *name, int value)
/*!\brief Parameter für den Connect setzen
 *
 * \descr
 * Die Datenbank-Klasse unterstützt zwei Connect-Funktionen: eine ohne Parameter und eine mit einem
 * Assoziativen Array als Parameter. Damit die Connect-Funktion ohne Parameter überhaupt funktioniert, müssen
 * diese zuvor mit SetParam gesetzt werden.
 *
 * @param[in] name Name des Parameters
 * @param[in] value Wert des Parameters als Integer
 */
{
	ConnectParam.Setf(name,"%i",value);
}

int Database::Connect()
/*!\brief Connect auf eine Datenbank erstellen
 *
 * \descr
 * Mit dieser Funktion wird eine Verbindung zu einem Datenbank-Server hergestellt. Die
 * dafür erforderlichen Parameter müssen zuvor mit der Funktion Database::SetParam gesetzt worden
 * sein.
 *
 * @return Bei Erfolg liefert die 1 zurück, im Fehlerfall 0.
 *
 * \example
 * Bei dem nachfolgenden Beispiel wird eine MySQL-Datenbank verwendet. Die Klasse "Database" kann selbst nicht
 * verwendet werden, da es sich hierbei nur um eine abstrakte Basisklasse handelt.
 *
 * \dontinclude db_examples.cpp
 * \skip DB_Example2
 * \until EOF
 *
 */
{
	return Connect(ConnectParam);
}

int Database::ConnectCreate(const CAssocArray &params)
/*!\brief Connect zum Server aufbauen und Datenbank anlegen
 *
 * \descr
 * Mit dieser Funktion wird eine Verbindung zum Datenbankserver aufgebaut und - sofern
 * nicht schon vorhanden - eine Datenbank angelegt. Die dafür notwendigen Parameter werden
 * dem Array \p params entnommen. Diese Parameter sind abhängig vom Datenbank-Typ, es gibt
 * jedoch eine Reihe von Parametern, die bei allen Datenbanken identisch sind:
 * - \b host: Der Hostname oder die IP-Adresse des Datenbank-Servers
 * - \b port: Der TCP-Port des Datenbank-Servers
 * - \b dbname: Der Name der intialen Datenbank. Dieser Parameter kann optional sein,
 *   da mit der Funktion Database::SelectDB die Datenbank auch später noch gewechselt werden kann.
 * - \b user: Der Name des Benutzers, mit dem sich an der Datenbank authentifiziert werden soll
 * - \b password: Das Passwort des Benutzers im Klartext
 *
 * \param params Ein Assoziatives Array mit den für den Connect erforderlichen Parameter.
 *
 * \returns Bei Erfolg liefert die 1 zurück, im Fehlerfall 0.
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_Example4
 * \until EOF
 */
{
	CAssocArray a=params;
	a.Delete("dbname");
	const char *dbname=params["dbname"];
	// Versuch ohne Datenbank-Name zu connecten
	if (!Connect(a)) return 0;
	// Wir versuchen die Datenbank auszuwählen
	if (SelectDB(dbname)) return 1;
	if (!CreateDatabase(dbname)) return 0;
	if (SelectDB(dbname)) return 1;
	return 0;
}

int Database::Close()
/*!\brief Verbindung zu Datenbank trennen
 *
 * \descr
 * Mit diesem Befehl wird die Verbindung zur Datenbank getrennt. Zu diesem Zeitpunkt sollten keine Query-
 * Results (ppl6::db::Result) mehr vorhanden sein, die mit diesem Datenbank-Connect erstellt wurden. Es
 * könnte sonst zu Fehlern kommen.
 *
 * @return Die Funktion liefert 1 zurück, wenn die Verbindung zur Datenbank erfolgreich getrennt wurde,
 * im Fehlerfall 0.
 */
{
	return Disconnect();
}

int Database::Execf(const char *query, ...)
/*!\brief Ergebnislosen SQL-Query anhand eines Formatierungsstrings bauen und ausführen
 *
 * \descr
 * Diese Funktion ist identisch mit Database::Exec, erwartet jedoch als Parameter einen Formatierungsstring
 * \p query und eine variable Anzahl von Parametern, die in den Formatierungsstring eingesetzt werden.
 * \par
 * Die Funktion ist für INSERT, UPDATE und andere SQL-Befehle gedachtm die keine Ergebniszeilen zurückliefern.
 * Bei SELECT-Befehlen sollte stattdessen die Funktion Database::Queryf verwendet werden.
 *
 * \param[in] query Formatierungsstring für den SQL-Befehl
 * \param[in] ... Optionale Parameter, die in den Formatierungsstring eingesetzt werden.
 * @return War die SQL-Abfrage erfolgreich, liefert die Funktion 1 zurück, im Fehlerfall 0. Über die Funktion
 * Database::GetAffectedRows kann ausgelesen werden, wieviele Datensätze durch den Query verändert wurden.
 */
{
	CString String;
	va_list args;
	va_start(args, query);
	String.VaSprintf(query,args);
	va_end(args);
	return Exec(String);
}

Result *Database::Queryf(const char *query, ...)
/*!\brief SQL-Query mit Ergebnis ausführen
 *
 * \descr
 * Dieser Befehl ist für SELECT und andere SQL-Befehle gedacht, die Ergebniszeilen zurückliefern. Im Gegensatz
 * zu Database::Exec wird hier eine Result-Klasse zurückgeliefert, aus der die Ergebniszeilen ausgelesen werden
 * können (siehe Result). Bei anderen Befehlen, die kein Ergebnis zurückliefern (z.B. INSERT oder UPDATE) sollte
 * stattdessen der etwas schnellere Befehl Database::Exec verwendet werden.
 * \par
 * Diese Funktion ist identisch mit Database::Query, erwartet jedoch als Parameter einen Formatierungsstring
 * \p query und eine variable Anzahl von Parametern, die in den Formatierungsstring eingesetzt werden.
 *
 * \param[in] query Formatierungsstring für den SQL-Befehl
 * \param[in] ... Optionale Parameter, die in den Formatierungsstring eingesetzt werden.
 * @return War die SQL-Abfrage erfolgreich, liefert die Funktion einen Pointer auf eine Result-Klasse zurück.
 * Diese muss nach Gebrauch von der aufrufenden Anwendung mit delete oder durch Aufruf von Database::FreeResult
 * gelöscht werden. Im Fehlerfall wird NULL zurückgeliefert.
 */
{
	CString String;
	va_list args;
	va_start(args, query);
	String.VaSprintf(query,args);
	va_end(args);
	return Query(String);
}

void Database::FreeResult(Result *res)
/*!\brief Result-Klasse freigeben
 *
 * \descr
 * Mit dieser Funktion wird die von den Funktionen Database::Query oder Database::Queryf zurückgegebene
 * Result-Klasse wieder gelöscht. Es wird jedoch empfphlen die Result-Klasse einfach mit \c delete zu
 * löschen, da dies etwas schneller ist.

 * @param[in] res Pointer auf eine Result-Klasse
 */
{
	if (res) delete res;
}

CAssocArray Database::ExecArray(const CString &query)
/*!\brief SQL-Query ausführen und ersten Datensatz als Array zurückgeben
 *
 * \descr
 * Mit dieser Funktion wird der SQL-Query \p query ausgeführt und die erste Zeile der Ergebnisdaten
 * als Assoziatives Array zurückgegeben. Die Funktion bietet sich daher für Selects an, die
 * genau einen Datensatz zurückliefern. Liefert ein Select mehrere Datensätze zurück, kann
 * stattdessen die Funktion Database::ExecArrayAll verwendet werden.
 * \note
 * Die Funktion ruft ihrerseits die Funktion Database::ExecArray(CAssocArray &result, const CString &query)
 * auf und die Daten des Arrays müssen mehrfach kopiert werden. Es wird daher empfohlen, direkt
 * Database::ExecArray(CAssocArray &result, const CString &query) aufzurufen
 *
 * \param[in] query Der Select-Befehl
 * \return Bei Erfolg liefert die Funktion die Daten in einem Assoziativen Array zurück.
 * Im Fehlerfall ist das Array leer.
 */
{
	CAssocArray a;
	ExecArray(a,query);
	return a;
}

CAssocArray Database::ExecArrayf(const char *query, ...)
/*!\brief SQL-Query bauen, ausführen und ersten Datensatz als Array zurückgeben
 *
 * \descr
 * Mit dieser Funktion wird zunächst der SQL-Query anhand des Formatierungsstrings \p query gebaut,
 * ausgeführt und die erste Zeile der Ergebnisdaten
 * als Assoziatives Array zurückgegeben. Die Funktion bietet sich daher für Selects an, die
 * genau einen Datensatz zurückliefern. Liefert ein Select mehrere Datensätze zurück, kann
 * stattdessen die Funktion Database::ExecArrayAll verwendet werden.
 * \note
 * Die Funktion ruft ihrerseits die Funktion Database::ExecArray(CAssocArray &result, const CString &query)
 * auf und die Daten des Arrays müssen mehrfach kopiert werden. Es wird daher empfohlen, direkt
 * Database::ExecArray(CAssocArray &result, const CString &query) aufzurufen
 *
 * \param[in] query Formatierungsstring für den SQL-Query
 * \param[in] ... Optionale Parameter für den Formatierungsstring
 * \return Bei Erfolg liefert die Funktion die Daten in einem Assoziativen Array zurück.
 * Im Fehlerfall ist das Array leer.
 */
{
	CAssocArray a;
	CString String;
	va_list args;
	va_start(args, query);
	String.VaSprintf(query,args);
	va_end(args);
	ExecArray(a,String);
	return a;
}

CAssocArray Database::ExecArrayAll(const CString &query)
/*!\brief SQL-Query ausführen und alle Datensätze in Array speichern
 *
 * \descr
 * Mit dieser Funktion wird der SQL-Query \p query ausgeführt und alle Zeilen des Ergebnisses
 * als Assoziatives Array zurückgegeben. Die Funktion bietet sich daher für Selects an, die
 * mehrere Datensätze zurückliefern. Liefert ein Select nur einen einzigen Datensätze zurück oder wird nur
 * der erste Datensatz benötigt, kann stattdessen die Funktion Database::ExecArray verwendet werden.
 * \note
 * Die Funktion ruft ihrerseits die Funktion Database::ExecArrayAll(CAssocArray &result, const CString &query)
 * auf und die Daten des Arrays müssen mehrfach kopiert werden. Es wird daher empfohlen, direkt
 * Database::ExecArrayAll(CAssocArray &result, const CString &query) aufzurufen
 *
 * \param[in] query Der Select-Befehl
 * \return Bei Erfolg liefert die Funktion die Daten in einem Assoziativen Array zurück.
 * Das Array besteht dabei aus zwei Ebenen. Die erste Ebene ist durchnummeriert, jeder Datensatz
 * erhält also eine Nummer. Auf der zweiten Ebene befinden sich dann die Key-Value-Paare des jeweiligen Datensatzes.
 * Im Fehlerfall ist das Array leer.
 */
{
	CAssocArray a;
	ExecArrayAll(a,query);
	return a;
}

CAssocArray Database::ExecArrayAllf(const char *query, ...)
/*!\brief SQL-Query bauen, ausführen und alle Datensätze in Array speichern
 *
 * \descr
 * Mit dieser Funktion wird zunächst der SQL-Query anhand des Formatierungsstrings \p query gebaut,
 * ausgeführt und alle Zeilen des Ergebnisses als Assoziatives Array zurückgegeben. Die Funktion bietet
 * sich daher für Selects an, die mehrere Datensätze zurückliefern. Liefert ein Select nur einen
 * einzigen Datensätze zurück oder wird nur der erste Datensatz benötigt, kann stattdessen die
 * Funktion Database::ExecArrayf verwendet werden.
 * \note
 * Die Funktion ruft ihrerseits die Funktion Database::ExecArrayAll(CAssocArray &result, const CString &query)
 * auf und die Daten des Arrays müssen mehrfach kopiert werden. Es wird daher empfohlen, direkt
 * Database::ExecArrayAllf(CAssocArray &result, const CString &query) aufzurufen
 *
 * \param[in] query Der Select-Befehl
 * \param[in] ... Optionale Parameter für den Formatierungsstring
 * \return Bei Erfolg liefert die Funktion die Daten in einem Assoziativen Array zurück.
 * Das Array besteht dabei aus zwei Ebenen. Die erste Ebene ist durchnummeriert, jeder Datensatz
 * erhält also eine Nummer. Auf der zweiten Ebene befinden sich dann die Key-Value-Paare des jeweiligen Datensatzes.
 * Im Fehlerfall ist das Array leer.
 */
{
	CAssocArray a;
	CString String;
	va_list args;
	va_start(args, query);
	String.VaSprintf(query,args);
	va_end(args);
	ExecArrayAll(a,String);
	return a;
}

int Database::ExecArray(CAssocArray &result, const CString &query)
/*!\brief SQL-Query ausführen und ersten Datensatz in Array speichern
 *
 * \descr
 * Mit dieser Funktion wird der SQL-Query \p query ausgeführt und die erste Zeile der Ergebnisdaten
 * im Assoziativen Array \p result gespeichert. Die Funktion bietet sich daher für Selects an, die
 * genau einen Datensatz zurückliefern. Liefert ein Select mehrere Datensätze zurück, kann
 * stattdessen die Funktion Database::ExecArrayAll verwendet werden.
 *
 * \param[out] result Ein Assoziatives Array, in dem das Ergebnis gespeichert wird. Der ursprüngliche
 * Inhalt des Arrays wird durch Aufruf dieser Funktion gelöscht. Falls der SQL-Befehl
 * kein Ergebnis geliefert hat, bleibt das Array leer.
 * \param[in] query Der Select-Befehl
 * \return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	result.Clear();
	Result *res=Query(query);
	if (!res) return 0;
	if (res->Rows()==0) {
		delete res;
		ppl6::SetError(182,"%s",(const char*)query);
		return 0;
	}
	if (res->FetchArray(result)) {
		delete res;
		return 1;
	}
	PushError();
	delete res;
	PopError();
	return 0;
}

int Database::ExecArrayf(CAssocArray &result, const char *query, ...)
/*!\brief SQL-Query bauen, ausführen und ersten Datensatz in Array speichern
 *
 * \descr
 * Mit dieser Funktion wird zunächst der SQL-Query anhand des Formatierungsstrings \p query gebaut,
 * ausgeführt und die erste Zeile der Ergebnisdaten
 * im Assoziativen Array \p result gespeichert. Die Funktion bietet sich daher für Selects an, die
 * genau einen Datensatz zurückliefern. Liefert ein Select mehrere Datensätze zurück, kann
 * stattdessen die Funktion Database::ExecArrayAll verwendet werden.
 *
 * \param[out] result Ein Assoziatives Array, in dem das Ergebnis gespeichert wird. Der ursprüngliche
 * Inhalt des Arrays wird durch Aufruf dieser Funktion gelöscht. Falls der SQL-Befehl
 * kein Ergebnis geliefert hat, bleibt das Array leer.
 * \param[in] query Formatierungsstring für den SQL-Query
 * \param[in] ... Optionale Parameter für den Formatierungsstring
 *
 * \return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	CString String;
	va_list args;
	va_start(args, query);
	String.VaSprintf(query,args);
	va_end(args);
	return ExecArray(result,String);
}

int Database::ExecArrayAll(CAssocArray &result, const CString &query)
/*!\brief SQL-Query ausführen und alle Datensätze in Array speichern
 *
 * \descr
 * Mit dieser Funktion wird der SQL-Query \p query ausgeführt und alle Zeilen des Ergebnisses
 * im Assoziativen Array \p result gespeichert. Die Funktion bietet sich daher für Selects an, die
 * mehrere Datensätze zurückliefern. Liefert ein Select nur einen einzigen Datensätze zurück oder wird nur
 * der erste Datensatz benötigt, kann stattdessen die Funktion Database::ExecArray verwendet werden.
 *
 * \param[out] result Ein Assoziatives Array, in dem das Ergebnis gespeichert wird. Der ursprüngliche
 * Inhalt des Arrays wird durch Aufruf dieser Funktion gelöscht. Falls der SQL-Befehl
 * kein Ergebnis geliefert hat, bleibt das Array leer. Das Array besteht dabei aus zwei Ebenen. Die erste
 * Ebene ist durchnummeriert, jeder Datensatz erhält also eine Nummer. Auf der zweiten Ebene befinden sich
 * dann die Key-Value-Paare des jeweiligen Datensatzes.
 * \param[in] query Der Select-Befehl
 * \return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	result.Clear();
	Result *res=Query(query);
	if (!res) return 0;
	pplint64 nr=res->Rows();
	for(pplint64 i=0;i<nr;i++) {
		CAssocArray *row=new CAssocArray;
		if (res->FetchArray(*row)) {
			result.Set("[]",row,false);
		} else {
			delete row;
		}
	}
	delete res;
	return 1;
}

int Database::ExecArrayAllf(CAssocArray &result, const char *query, ...)
/*!\brief SQL-Query bauen, ausführen und alle Datensätze in Array speichern
 *
 * \descr
 * Mit dieser Funktion wird zunächst der SQL-Query anhand des Formatierungsstrings \p query gebaut,
 * ausgeführt und alle Zeilen des Ergebnisses
 * im Assoziativen Array \p result gespeichert. Die Funktion bietet sich daher für Selects an, die
 * mehrere Datensätze zurückliefern. Liefert ein Select nur einen einzigen Datensätze zurück oder wird nur
 * der erste Datensatz benötigt, kann stattdessen die Funktion Database::ExecArray verwendet werden.
 *
 * \param[out] result Ein Assoziatives Array, in dem das Ergebnis gespeichert wird. Der ursprüngliche
 * Inhalt des Arrays wird durch Aufruf dieser Funktion gelöscht. Falls der SQL-Befehl
 * kein Ergebnis geliefert hat, bleibt das Array leer. Das Array besteht dabei aus zwei Ebenen. Die erste
 * Ebene ist durchnummeriert, jeder Datensatz erhält also eine Nummer. Auf der zweiten Ebene befinden sich
 * dann die Key-Value-Paare des jeweiligen Datensatzes.
 * \param[in] query Der Select-Befehl
 * \param[in] ... Optionale Parameter für den Formatierungsstring
 * \return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	CString String;
	va_list args;
	va_start(args, query);
	String.VaSprintf(query,args);
	va_end(args);
	return ExecArrayAll(result,String);
}


/*!\brief Wert Datenbank-konform quoten
 *
 * \descr
 * Diese Funktion escaped und quoted den String \p value Datenbank-konform abhängig vom Datentyp
 * \p type. Der Rückgabewert enthält je nach Datentyp bereits Hochkommata oder Anführungszeichen
 * und kann somit ohne weitere Quotes in einen Query eingesetzt werden.
 *
 * @param value Zu quotender String
 * @param type Datentyp. Wird nichts angegeben, wird der Wert \p value als String interpretiert.
 * @return Escapeter und Gequoteter String
 */
CString Database::getQuoted(const CString &value, const CString &type) const
{
	CString Type=type;
	CString s=value;
	Type.LCase();
	Escape(s);
	if (Type=="int" || Type=="integer" || Type=="bit" || Type=="boolean") return s;
	return "'"+s+"'";
}


/*!\brief Query zum Speichern des Datensatzes generieren
 *
 * \descr
 * Diese Funktion generiert aus den im Assoziativen Array \p a vorhandenen Feldern und Werten in Abhängigkeit der
 * Methode \p method einen SQL-Query für die Tabelle \p table. Soll ein
 * Update durchgeführt werden, muss zusätzlich noch eine Where-Klausel mit dem Parameter \p clause angegeben werden.
 * Enthalten die Daten in \p a Felder, die in der Tabelle \p table nicht vorhanden sind, muss zusätzlich noch das
 * Array \p exclude angegeben werden, dass die zu ignorierenden Feldnamen enthält.
 * \par
 * Die Daten in \p a werden vor dem Einsetzen in den Query korrekt Escaped.
 *
 * \param[out] Query String, in dem der Query gespeichert werden soll
 * \param[in] method Die Methode, mit der der Datensatz in die Datenbank geschrieben werden soll. In Frage kommen:
 * - \b INSERT
 * - \b UPDATE
 * - \b REPLACE
 * \param[in] table Name der Tabelle
 * \param[in] a Ein Assoziatives Array mit den zu speichernden Feldern
 * \param[in] clause Ein Optionaler Parameter, der bei \b UPDATE angegeben werden muss und die Where-Klausel
 * enthält. Das Keywort "where" muss nicht angegeben werden
 * \param[in] exclude Ein optionales assoziatives Array, was die Namen der Felder enthält, die nicht gespeichert werden
 * sollen. Muss verwendet werden, wenn in \p a Felder enthalten sind, die in der Tabelle \p table nicht existieren.
 * \param[in] types Ein optionales assoziatives Array, was die Typen der zu speichernden Daten angibt.
 * Hier wird zur Zeit nur "int" und "bit" interpretiert, was dazu führt, dass die Werte nicht in
 * Anführungszeichen in den SQL-Query eingebaut werden. Alle anderen Typen werden wie bisher als String
 * behandelt. Bei dem ASE von Sybase gibt es Probleme, wenn man versucht einen Ziffer in Anführungszeichen
 * anzugeben, wenn das Feld als "numeric" oder "bit" definiert ist.
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \example
 * Das folgende Beispiel zeigt, wie mit Hilfe von Save ein Datensatz mit UPDATE aktualisiert wird. Es wird
 * von folgender Tabellendefinition ausgegangen:
\verbatim
   userid    int4 not null primary key,
   vorname   varchar(64) nut null,
   nachname  varchar(64) nut null,
   email     varchar(128) nut null
\endverbatim
 * \par
 * \dontinclude db_examples.cpp
 * \skip DB_Save_Example1
 * \until EOF
 */
int Database::SaveGenQuery(CString &Query, const char *method, const char *table, CAssocArray &a, const char *clause, const CAssocArray &exclude, const CAssocArray &types)
{
	if (!method) {
		SetError(194,"const char *method");
		return 0;
	}
	if (!table) {
		SetError(194,"const char *table");
		return 0;
	}
	if (a.Count()==0) {
		SetError(343);
		return 0;
	}
	CString Keys, Vals, Key, Value, Method, Table, Clause, Type;
	Query.Clear();
	Method=method;
	Table=table;
	Method.LCase();
	if (Table.Len()==0) {
		SetError(344);
		return 0;
	}
	if (clause) Clause=clause;
	a.Reset();
	if (Method=="insert" || Method=="replace") {
		while (a.GetNext(Key,Value)) {
			if (exclude.GetChar(Key)==NULL) {
				Keys.Concat(Key);
				Keys.Concat(",");
				//printf ("Key=%s, Value=%s\n",(const char*)Key,(const char*)Value);
				Type=types.ToCString(Key);
				Vals+=getQuoted(Value,Type);
				Vals+=",";
			}
		}
		Keys.Chop();
		Vals.Chop();

		Query.Setf("%s into %s (%s) values (%s)",(const char*)Method, (const char*)Table, (const char*)Keys, (const char*)Vals);
		//printf ("Query: %s\n",(const char*)Query);
		return 1;
	} else if (Method=="update") {
		Query.Setf("%s %s set ",(const char*)Method, (const char*)Table);
		while (a.GetNext(Key,Value)) {
			if (exclude.GetChar(Key)==NULL) {
				Type=types.ToCString(Key);
				Query+=Key+"="+getQuoted(Value,Type)+",";
			}
		}
		Query.Chop();
		Clause.Trim();
		if (Clause.Len()>0) {
			Query.Concat(" ");
			if (!Clause.PregMatch("/^where\\s/i")) Query.Concat("where ");
			Query.Concat(Clause);
		}
		return 1;
	}
	SetError(345,(const char*)Method);
	return 0;
}

/*!\brief Datensatz speichern
 *
 * \descr
 * Diese Funktion speichert die im Assoziativen Array \p a vorhandenen Felder und Werte mit der Methode \p method
 * in der Tabelle \p table. Je nach Methode wird dabei aus den Daten ein entprechender SQL-Query gebaut. Soll ein
 * Update durchgeführt werden, muss zusätzlich noch eine Where-Klausel mit dem Parameter \p clause angegeben werden.
 * Enthalten die Daten in \p a Felder, die in der Tabelle \p table nicht vorhanden sind, muss zusätzlich noch das
 * Array \p exclude angegeben werden, dass die zu ignorierenden Feldnamen enthält.
 * \par
 * Die Daten in \p a werden vor dem Einsetzen in den Query korrekt Escaped.
 *
 * \param[in] method Die Methode, mit der der Datensatz in die Datenbank geschrieben werden soll. In Frage kommen:
 * - \b INSERT
 * - \b UPDATE
 * - \b REPLACE
 * \param[in] table Name der Tabelle
 * \param[in] a Ein Assoziatives Array mit den zu speichernden Feldern
 * \param[in] clause Ein Optionaler Parameter, der bei \b UPDATE angegeben werden muss und die Where-Klausel
 * enthält. Das Keywort "where" muss nicht angegeben werden
 * \param[in] exclude Ein optionales assoziatives Array, was die Namen der Felder enthält, die nicht gespeichert werden
 * sollen. Muss verwendet werden, wenn in \p a Felder enthalten sind, die in der Tabelle \p table nicht existieren.
 * \param[in] types Ein optionales assoziatives Array, was die Typen der zu speichernden Daten angibt.
 * Hier wird zur Zeit nur "int" und "bit" interpretiert, was dazu führt, dass die Werte nicht in
 * Anführungszeichen in den SQL-Query eingebaut werden. Alle anderen Typen werden wie bisher als String
 * behandelt. Bei dem ASE von Sybase gibt es Probleme, wenn man versucht einen Ziffer in Anführungszeichen
 * anzugeben, wenn das Feld als "numeric" oder "bit" definiert ist.
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \example
 * Das folgende Beispiel zeigt, wie mit Hilfe von Save ein Datensatz mit UPDATE aktualisiert wird. Es wird
 * von folgender Tabellendefinition ausgegangen:
\verbatim
   userid    int4 not null primary key,
   vorname   varchar(64) nut null,
   nachname  varchar(64) nut null,
   email     varchar(128) nut null
\endverbatim
 * \par
 * \dontinclude db_examples.cpp
 * \skip DB_Save_Example1
 * \until EOF
 */
int Database::Save(const char *method, const char *table, CAssocArray &a, const char *clause, const CAssocArray &exclude, const CAssocArray &types)
{
	CString Query;
	if (!SaveGenQuery(Query,method,table,a,clause,exclude,types)) return 0;
	return Exec(Query);
}

int Database::ReadKeyValue(CAssocArray &res, const char *query, const char *keyname, const char *valname)
{
	if (!query) {
		SetError(194,"int Database::ReadKeyValue(CAssocArray *res, ==> const char *query <==, const char *keyname, const char *valname)");
		return 0;
	}
	if (!keyname) {
		SetError(194,"int Database::ReadKeyValue(CAssocArray *res, const char *query, ==> const char *keyname <==, const char *valname)");
		return 0;
	}
	Result *r=Query(query);
	if (!r) return 0;
	pplint32 nr=(pplint32)r->Rows();
	CAssocArray row;
	for (pplint32 i=0;i<nr;i++) {
		row.Clear();
		if (r->FetchArray(row,i)) {
			const char *key=row[keyname];
			if (!key) {
				delete r;
				SetError(294,"%s",keyname);
				return 0;
			}
			if (valname) {
				//$a[$r[$keyfield]]=$r[$valfield];
				const char *val=row[valname];
				if (val) res.Set(key,val);
			} else {
				res.Set(key,row);
			}
		}
	}
	delete r;
	return 1;
}

ppluint64 Database::InsertKey(const char *table, CAssocArray &a, const char *keyname, const CAssocArray &exclude)
/*!\brief Datensatz mit incrementellem Schlüssel speichern
 *
 * \descr
 * Diese Funktion ähnelt Database::Save, jedoch unterstützt sie nur die INSERT-Methode. Speziell ein Insert auf eine Tabelle, bei
 * der der primäre Schlüssel ein incrementeller Wert ist, der jedoch nicht automatisch von der Datenbank erhöht wird.
 * \par
 * Die Funktion sperrt zunächst die angegebene Tabelle \p table für Schreibzugriffe. Dann wird der höchste Wert des Schlüssels
 * \p keyname gesucht und um 1 erhöht. Mit diesem Schlüssel werden die Daten aus dem Array \p a mit INSERT gespeichert,
 * außer den Feldern, die in \p exclude angegeben wurden. Dazu wird die Funktion Database::Save aufgerufen.
 * Danach wird die Tabelle \p table wieder entsperrt. Der Wert des Schlüssels wird als 64-Bit-Wert zurückgegeben.
 *
 * \param[in] table Name der Datenbank-Tabelle, in die der Datensatz gespeichert werden soll
 * \param[in] a Ein assoziatives Array mit den zu speichernden Daten
 * \param[in] keyname Der Feldname des primären Schlüssels der Tabelle
 * \param[in] exclude Ein optionales Array, das die Feldnamen aus \p a enthält, die nicht gespeichert werden
 * sollen
 * \return Bei Erfolg gibt die Funktion den Wert des Schlüssels zurück, mit dem der Datensatz gespeichert
 * wurde, im Fehlerfall 0.
 */
{
	if (!Execf("lock tables %s write",table)) {
		PushError();
		Exec("unlock tables");
		PopError();
		return 0;
	}
	ppluint64 id;
	CAssocArray r;
	if (!ExecArrayf(r,"select max(%s)+1 as newid from %s",keyname,table)) {
		PushError();
		Exec("unlock tables");
		PopError();
		return 0;
	}
	id=atoll(r.Get("newid"));
	if (!id) id=1;
	a.Setf(keyname,"%llu",id);
	int ret=Save("insert",table,a,NULL,exclude);
	//PrintError();
	PushError();
	Exec("unlock tables");
	PopError();
	if (!ret) return 0;
	return id;
}

CString	Database::GetEscaped(const CString &str)
/*!\brief String escapen
 *
 * \descr
 * Mit dieser Funktion wird der übergebene String \p str Datenbank-konform escaped und als Return-Wert
 * zurückgegeben. Der escapete String kann gefahrlos in SQL-Statements innerhalb von Anführungszeichen
 * oder Hochkommata verwendet werden.
 *
 * \param[in] str Der zu escapende String
 * \return Liefert den escapten String zurück. Falls ein Fehler aufgetreten ist, kann der String leer sein.
 * \note
 * In der Regel muss eine Verbindung zur Datenbank bestehen, damit der Aufruf erfolgreich ist.
 */
{
	CString s=str;
	Escape(s);
	return s;
}


/*******************************************************************************************************
 * Die Nachfolgenden Funktionen sind virtuell und müssen von den einzelnen Datenbank-Klassen
 * implementiert werden
 *******************************************************************************************************
 */


int Database::Connect(const CAssocArray &params)
/*!\brief Connect auf eine Datenbank erstellen
 *
 * \descr
 * Mit dieser Funktion wird eine Verbindung zu einem Datenbank-Server hergestellt, wobei
 * die dafür notwendigen Parameter dem Array \p params entnommen werden.
 *
 * Die für den Connect erforderlichen oder optionalen Parameter hängen von der jeweiligen
 * Datenbank ab und sind in der jeweiligen Dokumentation zu finden. Es gibt jedoch eine
 * Reihe von Parametern, die bei allen Datenbanken identisch sind:
 * - \b host: Der Hostname oder die IP-Adresse des Datenbank-Servers
 * - \b port: Der TCP-Port des Datenbank-Servers
 * - \b dbname: Der Name der intialen Datenbank. Dieser Parameter kann optional sein,
 *   da mit der Funktion Database::SelectDB die Datenbank auch später noch gewechselt werden kann.
 * - \b user: Der Name des Benutzers, mit dem sich an der Datenbank authentifiziert werden soll
 * - \b password: Das Passwort des Benutzers im Klartext
 *
 * \param params Ein Assoziatives Array mit den für den Connect erforderlichen Parameter.
 *
 * \return Bei Erfolg liefert die 1 zurück, im Fehlerfall 0.
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_Example3
 * \until EOF
 *
 */
{
	SetError(180);
	return 0;
}

int Database::Reconnect()
/*!\brief Verlorene Datenbank-Verbindung wieder herstellen
 *
 * \descr
 * Diese Funktion versucht eine unterbrochene Datenbankverbindung wieder aufzubauen. Dazu werden
 * die gleichen Parameter wie beim früheren Connect verwendet.
 *
 * @return Kann die Datenbank-Verbindung erfolgreich wieder aufgebaut werden, liefert die
 * Funktion 1 zurück, andernfalls 0.
 */
{
	SetError(180);
	return 0;
}

int Database::Disconnect()
/*!\brief Verbindung zur Datenbank trennen
 *
 * \descr
 * Durch Aufruf dieser Funktion wird die Verbindung zur Datenbank getrennt.
 *
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	SetError(180);
	return 0;
}

int Database::SelectDB(const char *databasename)
/*!\brief Aktive Datenbank auswählen
 *
 * \descr
 * In der Regel befinden sich auf einem Datenbank-Server mehrere Datenbanken. Mit dieser Funktion kann
 * die aktive Datenbank ausgewählt werden, die danach in SQL-Befehlen ohne Prefix verwendet werden
 * kann. Die Funktion wird auch beim Connect aufgerufen, sofern der Parameter \b dbname angegeben wurde.
 *
 * @param[in] databasename Name der gewünschten Datenbank
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	SetError(180);
	return 0;
}

int Database::Exec(const CString &query)
/*!\brief SQL-Query ohne Ergebnis ausführen
 *
 * \descr
 * Mit dieser Funktion kann ein SQL-Query an die Datenbank geschickt werden, bei dem das Ergebnis keine
 * Rolle spielt. Sie bietet sich daher für INSERT, UPDATE oder andere SQL-Befehle an, die keine
 * Ergebniszeilen zurückliefern. Bei SELECT-Befehlen sollte stattdessen die Funktion Database::Query
 * verwendet werden.
 *
 * @param[in] query Die gewünschte SQL-Abfrage
 * @return War die SQL-Abfrage erfolgreich, liefert die Funktion 1 zurück, im Fehlerfall 0. Über die Funktion
 * Database::GetAffectedRows kann ausgelesen werden, wieviele Datensätze durch den Query verändert wurden.
 */
{
	SetError(180);
	return 0;
}

Result *Database::Query(const CString &query)
/*!\brief SQL-Query mit Ergebnis ausführen
 *
 * \descr
 * Dieser Befehl ist für SELECT und andere SQL-Befehle gedacht, die Ergebniszeilen zurückliefern. Im Gegensatz
 * zu Database::Exec wird hier eine Result-Klasse zurückgeliefert, aus der die Ergebniszeilen ausgelesen werden
 * können (siehe Result). Bei anderen Befehlen, die kein Ergebnis zurückliefern (z.B. INSERT oder UPDATE) sollte
 * stattdessen der etwas schnellere Befehl Database::Exec verwendet werden.
 *
 * @param[in] query Die gewünschte SQL-Abfrage
 * @return War die SQL-Abfrage erfolgreich, liefert die Funktion einen Pointer auf eine Result-Klasse zurück.
 * Diese muss nach Gebrauch von der aufrufenden Anwendung mit delete oder durch Aufruf von Database::FreeResult
 * gelöscht werden. Im Fehlerfall wird NULL zurückgeliefert.
 */
{
	SetError(180);
	return 0;
}


void Database::SetMaxRows(ppluint64 rows)
/*!\brief Maximale Anzahl Zeilen im Ergebnis eines Selects
 *
 * \descr
 * Mit dieser Funktion wird festgelegt, wieviele Zeilen ein Select-Ergebnis maximal zurückgeben soll.
 * Je nach Datenbank wird dies erreicht, in dem dem SQL-Query noch ein Limit mitgegeben wird.
 *
 * @param[in] rows Anzahl maximaler Zeilen oder 0, wenn es kein Limit geben soll.
 */
{
	SetError(180);
}

int Database::Ping()
/*!\brief Erreichbarkeit der Datenbank prüfen
 *
 * \descr
 * Mit dieser Funktion können Clients, die geraume Zeit untätig waren, prüfen, ob die Verbindung zum
 * Datenbankserver noch zur Verfügung steht. Je nach Implementierung (z.B. bei MySQL) kann es sein, dass
 * durch Aufruf dieser Funktion eine abgebrochene Verbindung automatisch wieder aufgebaut wird.
 *
 * @return Die Funktion liefert 1 zurück, wenn die Datenbankverbindung noch besteht. Ist dies nicht der
 * Fall, wird 0 zurückgegeben. Es kann dann mit der Funktion Database::Reconnect versucht werden, die
 * Verbindung wieder herzustellen.
 */
{
	SetError(180);
	return 0;
}

int Database::Escape(CString &str) const
/*!\brief String escapen
 *
 * \descr
 * Mit dieser Funktion wird der übergebene String \p str Datenbank-konform escaped. Der escapete
 * String \p str kann anschließend gefahrlos in SQL-Statements innerhalb von Anführungszeichen
 * oder Hochkommata verwendet werden.
 *
 * \param[in,out] str Der zu escapende String
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 * \note
 * In der Regel muss eine Verbindung zur Datenbank bestehen, damit der Aufruf erfolgreich ist.
 */
{
	SetError(180);
	return 0;
}

ppluint64 Database::GetInsertID()
/*!\brief Letzer durch eine AUTO_INCREMENT-Spalte generierten Wert
 *
 * \descr
 * Liefert den von der vorherigen INSERT- oder UPDATE-Anweisung für eine AUTO_INCREMENT-Spalte generierten Wert.
 * Verwenden Sie diese Funktion, wenn Sie eine INSERT-Anweisung auf einer Tabelle mit einem AUTO_INCREMENT-Feld
 * ausgeführt haben.
 *
 * @return Wert
 */
{
	SetError(180);
	return 0;
}

pplint64 Database::GetAffectedRows()
/*!\brief Betroffene Zeilen
 *
 * \descr
 * War der vorausgehende Datenbank-Query ein Select, liefert diese Funktion die Anzahl
 * Zeilen im Ergebnis zurück. Handelete es sich um ein Update/Insert/Replace, wird die Anzahl
 * betroffener bzw. veränderter Datensätze zurückgegeben.
 *
 * @return Anzahl betroffender Datensätze, im Fehlerfall -1
 */
{
	SetError(180);
	return 0;
}

int Database::StartTransaction()
/*!\brief Transaktion starten
 *
 * \descr
 * Als Transaktion (von lat. trans „über“, actio zu agere „(durch)führen“) bezeichnet man in der
 * Informatik eine feste Folge von Operationen, die als eine logische Einheit betrachtet werden.
 * Insbesondere wird für Transaktionen gefordert, dass sie entweder vollständig oder überhaupt
 * nicht ausgeführt werden (Atomizität).
 *
 * Mit diesem Befehl läßt sich eine Transaktion auf der Datenbank starten. Mit dem Befehl
 * Database::EndTransaction wird sie abgeschlossen und mit Database::CancelTransaction
 * abgebrochen.
 *
 * Ab Version 6.4.3 der PPL-Library kann man Transaktionen verschachteln, in dem man
 * Database::StartTransaction mehrfach aufruft. Mit Database::EndTransaction wird dann nur die
 * innerste Transaktionsklammer abgeschlossen, mit Database::CancelTransaction entsprechend nur
 * die innerste Transaktionsklammer zurückgerollt. Mit Database::CancelTransactionComplete
 * läßt sich die komplette Transaktion bis zur äußersten Klammer zurückrollen. Bei der
 * Verschachtelung von Transaktionen muss man darauf achten, dass immer eine gleiche Anzahl
 * von Database::EndTransaction wie Database::StartTransaction geben muss, da sonst die Transaktion
 * nicht vollständig geschlossen wird und es zu Datenbank Blockaden kommen kann.
 *
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	SetError(180);
	return 0;
}

int Database::EndTransaction()
/*!\brief Transaktion beenden
 *
 * \descr
 * Mit diesem Befehl wird die zuletzt mit Database::StartTransaction begonnene Transaktion beendet.
 * Dadurch werden die innerhalb der Transaktion veränderten Daten endgültig in der Datenbank
 * gespeichert.
 *
 * @return Die Funktion liefert 1 zurück, wenn die Transaktion vollständig erfolgreich abgeschlossen
 * wurde. Im Fehlerfall wird 0 zurückgegeben und keine der innerhalb der Transaktion enthaltenen
 * Änderungen wurde durchgeführt.
 */
{
	SetError(180);
	return 0;
}

int Database::CancelTransaction()
/*!\brief Letzte Transaktion abbrechen
 *
 * \descr
 * Mit diesem Befehl wird die letzte mit Database::StartTransaction begonnene Transaktion abgebrochen.
 * Alle darin enthaltenen Änderungen sind unwirksam und werden nicht gespeichert.
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0
 *
 */
{
	SetError(180);
	return 0;
}

int Database::CancelTransactionComplete()
/*!\brief Transaktion vollständig abbrechen
 *
 * \descr
 * Mit diesem Befehl wird die komplette Transaktion bis zur obersten Ebene zurückgerollt.
 * Alle darin enthaltenen Änderungen sind unwirksam und werden nicht gespeichert.
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0
 *
 */
{
	SetError(180);
	return 0;
}

int Database::CreateDatabase(const char *name)
/*!\brief Datenbank erstellen
 *
 * \descr
 * Mit diesem Befehl wird auf dem Server eine eine neue Datenbank mit dem Namen \p name erstellt.
 * Dazu muss jedoch vorher bereits eine Verbindung zu einem Server hergestellt worden sein
 * (siehe Database::Connect) und der Datenbank-User muss die notwendigen Rechte zum Anlegen einer
 * Datenbank besitzen.
 *
 * @param[in] name Name der anzulegenden Datenbank
 * @return Konnte die Datenbank erfolgreich angelegt werden, gibt die Funktion 1 zurück,
 * im Fehlerfall 0.
 */
{
	SetError(180);
	return 0;
}

/*!\brief Typ der Datenbank
 *
 * \desc
 * Diese Funktion gibt einen String mit dem Typ der Datenbank zurück. Der String kann einen der folgenden
 * Werte enthalten:
 * - MySQL
 * - Postgres
 * - Sybase
 * - unknown
 *
 * @return String mit dem Typ der Datenbank
 */
CString	Database::databaseType() const
{
	return CString("unknown");
}

}	// EOF namespace db
}	// EOF namespace ppl6
