/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2024, Patrick Fedick <patrick@pfp.de>
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

#include "prolog_ppl7.h"
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
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "ppl7.h"
#include "ppl7-db.h"


namespace ppl7 {
namespace db {

/*!\ingroup PPLGroupDatabases
 * \brief Verbindung zu einer Datenbank herstellen
 *
 * \header \#include <ppl7-db.h>
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
 * - \b sqlite3: SQLite3-Datenbank
 * Die tatsächlich unterstützten Datenbanken hängen davon ab, wie PPL7 kompiliert wurde.
 * \par
 * Postgres unterstützt zusätzlich noch die Angabe des Suchpfades:
 * - \b searchpath: Kommaseparierte Liste mit den Schemata, die in den Suchpfad
 *      aufgenommen werden sollen
 *
 * \param params Ein Assoziatives Array mit den für den Connect erforderlichen Parameter.
 *
 * \return Bei Erfolg liefert die Funktion einen Pointer auf ein Database-Objekt zurück.
 * Im Fehlerfall wird eine Exception geworfen.
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_Example1
 * \until EOF
 */
Database* Connect(const AssocArray& params)
{
	String type=params["type"];
	if (type.isEmpty()) throw MissingArgumentException("type");
	type.lowerCase();
	Database* db=NULL;
#ifdef HAVE_MYSQL
	throw UnsupportedFeatureException("MySQL");
	/*
	if (type=="mysql") {
		db=new MySQL;
	}
	*/
#endif
#ifdef HAVE_FREETDS
	throw UnsupportedFeatureException("Sybase");
	/*
	if (type=="sybase") {
		db=new Sybase;
	}
	*/
#endif
#ifdef HAVE_POSTGRESQL
	if (type == "postgres" || type == "postgresql") {
		db=new PostgreSQL;
	}
#endif
#ifdef HAVE_SQLITE
	throw UnsupportedFeatureException("sqlite");
	/*
	if (type=="sqlite") {
		db=new SQLite;
	}
	*/
#endif
	if (!db) {
		throw UnsupportedFeatureException("Database-Type: %s", (const char*)type);
	}
	try {
		db->connect(params);
	} catch (...) {
		delete db;
		throw;
	}
	return db;
}

void GetSupportedDatabases(AssocArray& a)
{
	a.clear();
#ifdef HAVE_MYSQL
	a.set("mysql/type", "mysql");
	a.set("mysql/name", "MySQL");
#endif
#ifdef HAVE_FREETDS
	a.set("sybase/type", "sybase");
	a.set("sybase/name", "Sybase Open Client / ASE");
#endif
#ifdef HAVE_POSTGRESQL
	a.set("postgres/type", "postgres");
	a.set("postgres/name", "PostgreSQL");
#endif
#ifdef HAVE_SQLITE3
	a.set("sqlite/type", "sqlite");
	a.set("sqlite/name", "SQLite");
#endif
}


/*!\class Database
 * \ingroup PPLGroupDatabases
 * \brief Basisklasse für verschiedene Datenbanken
 *
 * \header \#include <ppl7-db.h>
 *
 * \descr
 * Die Klasse \b Database ist eine abstrakte Basisklasse, von der die eigentliche Datenbank-spezifische Implementierung
 * abgeleitet wird. Sie enthält neben rein virtuellen Funktionen, deren Implementierung in der abgeleiteten
 * Klasse erfolgen muss, auch Funktionen, die Datenbank-unabhängig sind.
 * \par
 * Die Klasse kann nicht direkt instanziiert werden, stattdessen sollte immer die jeweilige
 * Datenbank-Klasse (siehe MySQL, SQLite, Postgres) oder die Funktion ppl7::db::Connect verwendet
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


/*!\brief Konstruktor der Klasse
 *
 * \descr
 * Konstruktor der Klasse
 */
Database::Database()
{
	lastuse=0;
	lastping=0;
	Log=NULL;
}

/*!\brief Destruktor der Klasse
 *
 * \descr
 * Destruktor der Klasse.
 */
Database::~Database()
{
	//close();
	if (Log) Log->print(Logger::DEBUG, 1, "ppl7::db::Database", "SetLogfile", __FILE__, __LINE__, "Stop logging of Database-Queries");
}

/*!\brief Querylog aktivieren
 *
 * \descr
 * Mit dieser Funktion kann ein Querylog aktiviert werden. Ist es aktiviert, wird bei jedem
 * Query ein Datensatz in das Logfile geschrieben, dem man neben Datum, Uhrzeit und Query auch entnehmen
 * kann, wie lang der Query gebraucht hat.
 *
 * @param[in] log Referenz auf eine Instanz der Klasse Logger.
 * \see Database::getLogger
 * \see Database::removeLogger
 */
void Database::setLogger(Logger& logger)
{
	Log=&logger;
	if (Log) Log->print(Logger::DEBUG, 1, "ppl7::db::Database", "setLogger", __FILE__, __LINE__, "Start logging of Database-Queries");
}

/*!\brief Querylog deaktivieren
 *
 * \descr
 * Mit dieser Funktion kann das Loggen aller Queries deaktiviert werden.
 *
 * \see Database::setLogger
 * \see Database::getLogger
 */
void Database::removeLogger()
{
	if (Log != NULL) {
		Log->print(Logger::DEBUG, 1, "ppl7::db::Database", "removeLogger", __FILE__, __LINE__, "Stop logging of Database-Queries");
		Log=NULL;
	}
}

/*!\brief Pointer auf den konfigurierten Logger zurückgeben
 *
 * @return Gibt einen Pointer auf die konfigurierte Logger-Klasse zurück, oder NULL, wenn kein Logger
 * konfiguriert ist.
 * \see Database::setLogger
 * \see Database::removeLogger
 */
Logger* Database::getLogger()
{
	return Log;
}

void Database::logQuery(const String& query, float duration)
/*!\brief Interne Funktion zum Loggen von Queries
 *
 * \descr
 * Diese Funktion wird intern aufgerufen, um einen Query in das Logfile zu schreiben, sofern dieses
 * über Database::setLogger aktiviert wurde.
 *
 * @param[in] query Der durchgeführte Query
 * @param[in] duration Laufzeit des Queries in Sekunden.
 */
{
	if (!Log) return;
	Log->print(Logger::DEBUG, 4, "ppl7::db::Database", "logQuery", __FILE__, __LINE__, ToString("Querytime: %0.6f, Query: %s", duration, (const char*)query));
}

/*!\brief Uhrzeit der letzten Datenbank-Kommunikation aktualisieren
 *
 * \descr
 * Diese interne Funktion wird immer dann aufgerufen, wenn eine erfolgreiche Kommunikation mit dem Datenbank-Server
 * stattgefunden hat. Dabei kann es sich um einen Query der Anwendung gehandelt haben aber auch um einen
 * Ping, der automatisch durch die Verwaltung des Datenbank-Pools abgeschickt wurde (siehe Pool und PoolEx).
 * Sie aktualisiert den Wert Database::lastping mit dem aktuellen Timestamp. Dieser Wert enthält somit immer
 * einen Zeitstempel, zu dem zuletzt eine Verbindung zum Server bestand.
 */
void Database::updateLastPing()
{
	lastping=GetTime();
}

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
void Database::updateLastUse()
{
	lastping=GetTime();
	lastuse=lastping;
}

/*!\brief Timestamps auf 0 setzen
 *
 * \descr
 * Die Werte Database::lastping und Database::lastuse werden auf 0 gesetzt. Die Methode wird
 * aufgerufen, wenn die Verbindung zur Datenbank getrennt wird.
 */
void Database::clearLastUse()
{
	lastping=lastuse=0;
}

/*!\brief Parameter für den Connect setzen
 *
 * \descr
 * Die Datenbank-Klasse unterstützt zwei Connect-Funktionen: eine ohne Parameter und eine mit einem
 * Assoziativen Array als Parameter. Damit die Connect-Funktion ohne Parameter funktioniert, müssen
 * diese zuvor mit setParam gesetzt werden.
 *
 * @param[in] name Name des Parameters
 * @param[in] value Wert des Parameters als String
 */
void Database::setParam(const String& name, const String& value)
{
	ConnectParam.set(name, value);
}

/*!\brief Parameter für den Connect setzen
 *
 * \descr
 * Die Datenbank-Klasse unterstützt zwei Connect-Funktionen: eine ohne Parameter und eine mit einem
 * Assoziativen Array als Parameter. Damit die Connect-Funktion ohne Parameter funktioniert, müssen
 * diese zuvor mit setParam gesetzt werden.
 *
 * @param[in] name Name des Parameters
 * @param[in] value Wert des Parameters als Integer
 */
void Database::setParam(const String& name, int value)
{
	ConnectParam.setf(name, "%d", value);
}

/*!\brief Parameter für den Connect setzen
 *
 * \descr
 * Die Datenbank-Klasse unterstützt zwei Connect-Funktionen: eine ohne Parameter und eine mit einem
 * Assoziativen Array als Parameter. Damit die Connect-Funktion ohne Parameter funktioniert, müssen
 * diese zuvor mit setParam gesetzt werden.
 *
 * @param[in] params Assoziatives Array mit den Connect-Parametern
 */
void Database::setParam(const AssocArray& params)
{
	ConnectParam.add(params);
}

/*!\brief Connect auf eine Datenbank erstellen
 *
 * \descr
 * Mit dieser Funktion wird eine Verbindung zu einem Datenbank-Server hergestellt. Die
 * dafür erforderlichen Parameter müssen zuvor mit der Funktion Database::setParam gesetzt worden
 * sein.
 *
 * \example
 * Bei dem nachfolgenden Beispiel wird eine MySQL-Datenbank verwendet.
 *
 * \dontinclude db_examples.cpp
 * \skip DB_Example2
 * \until EOF
 *
 */
void Database::connect()
{
	connect(ConnectParam);
}

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
 * \exception diverse
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_Example4
 * \until EOF
 */
void Database::connectCreate(const AssocArray& params)
{
	AssocArray a=params;
	a.remove("dbname");
	String dbname=params["dbname"];
	// Versuch ohne Datenbank-Name zu connecten
	connect(a);
	// Wir versuchen die Datenbank auszuwählen
	try {
		selectDB(dbname);
		return;
	} catch (...) {

	}
	createDatabase(dbname);
	selectDB(dbname);
}

/*!\brief Ergebnislosen SQL-Query anhand eines Formatierungsstrings bauen und ausführen
 *
 * \descr
 * Diese Funktion ist identisch mit Database::exec, erwartet jedoch als Parameter einen Formatierungsstring
 * \p query und eine variable Anzahl von Parametern, die in den Formatierungsstring eingesetzt werden.
 * \par
 * Die Funktion ist für INSERT, UPDATE und andere SQL-Befehle gedacht, die keine Ergebniszeilen zurückliefern.
 * Bei SELECT-Befehlen sollte stattdessen die Funktion Database::query bzw. Database::queryf verwendet werden.
 *
 * \param[in] query Formatierungsstring für den SQL-Befehl
 * \param[in] ... Optionale Parameter, die in den Formatierungsstring eingesetzt werden.
 * \exception QueryFailedException und andere
 */
void Database::execf(const char* query, ...)
{
	String str;
	va_list args;
	va_start(args, query);
	str.vasprintf(query, args);
	va_end(args);
	exec(str);
}

ResultSet* Database::queryf(const char* query, ...)
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
	String str;
	va_list args;
	va_start(args, query);
	str.vasprintf(query, args);
	va_end(args);
	return this->query(str);
}

AssocArray Database::execArray(const String& query)
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
	AssocArray a;
	execArray(a, query);
	return a;
}

AssocArray Database::execArrayf(const char* query, ...)
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
	AssocArray a;
	String str;
	va_list args;
	va_start(args, query);
	str.vasprintf(query, args);
	va_end(args);
	execArray(a, str);
	return a;
}

uint64_t Database::count(const String& table, const String& where)
{
	String query;
	query.set("select count(*) as num from ");
	query+=table;
	if (where.notEmpty()) {
		query+=" where " + where;
	}
	ResultSet* res=this->query(query);
	uint64_t num=res->getString("num").toUnsignedInt64();
	delete res;
	return num;
}

void Database::execArray(AssocArray& result, const String& query)
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
	ResultSet* res=this->query(query);
	res->fetchArray(result);
	delete res;
}

void Database::execArrayf(AssocArray& result, const char* query, ...)
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
	String q;
	va_list args;
	va_start(args, query);
	q.vasprintf(query, args);
	va_end(args);
	execArray(result, q);
}

void Database::execArrayAll(AssocArray& result, const String& query)
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
	ResultSet* res=this->query(query);
	copyResultToAssocArray(res, result);
	delete res;
}

AssocArray Database::execArrayAll(const String& query)
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
	AssocArray a;
	ResultSet* res=this->query(query);
	copyResultToAssocArray(res, a);
	delete res;
	return a;
}


AssocArray Database::execArrayAllf(const char* query, ...)
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
	AssocArray a;
	String q;
	va_list args;
	va_start(args, query);
	q.vasprintf(query, args);
	va_end(args);
	ResultSet* res=this->query(q);
	copyResultToAssocArray(res, a);
	delete res;
	return a;
}

void Database::execArrayAllf(AssocArray& result, const char* query, ...)
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
	String q;
	va_list args;
	va_start(args, query);
	q.vasprintf(query, args);
	va_end(args);
	ResultSet* res=this->query(q);
	copyResultToAssocArray(res, result);
	delete res;
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
String Database::getQuoted(const String& value, const String& type) const
{
	String Type=type;
	String s=value;
	Type.lowerCase();
	escape(s);
	if (Type == "int" || Type == "integer" || Type == "bit" || Type == "boolean") return s;
	return "'" + s + "'";
}


#ifdef TODO

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
int Database::SaveGenQuery(CString& Query, const char* method, const char* table, CAssocArray& a, const char* clause, const CAssocArray& exclude, const CAssocArray& types)
{
	if (!method) {
		SetError(194, "const char *method");
		return 0;
	}
	if (!table) {
		SetError(194, "const char *table");
		return 0;
	}
	if (a.Count() == 0) {
		SetError(343);
		return 0;
	}
	CString Keys, Vals, Key, Value, Method, Table, Clause, Type;
	Query.Clear();
	Method=method;
	Table=table;
	Method.LCase();
	if (Table.Len() == 0) {
		SetError(344);
		return 0;
	}
	if (clause) Clause=clause;
	a.Reset();
	if (Method == "insert" || Method == "replace") {
		while (a.GetNext(Key, Value)) {
			if (exclude.GetChar(Key) == NULL) {
				Keys.Concat(Key);
				Keys.Concat(",");
				//printf ("Key=%s, Value=%s\n",(const char*)Key,(const char*)Value);
				Type=types.ToCString(Key);
				Vals+=getQuoted(Value, Type);
				Vals+=",";
			}
		}
		Keys.Chop();
		Vals.Chop();

		Query.Setf("%s into %s (%s) values (%s)", (const char*)Method, (const char*)Table, (const char*)Keys, (const char*)Vals);
		//printf ("Query: %s\n",(const char*)Query);
		return 1;
	} else if (Method == "update") {
		Query.Setf("%s %s set ", (const char*)Method, (const char*)Table);
		while (a.GetNext(Key, Value)) {
			if (exclude.GetChar(Key) == NULL) {
				Type=types.ToCString(Key);
				Query+=Key + "=" + getQuoted(Value, Type) + ",";
			}
		}
		Query.Chop();
		Clause.Trim();
		if (Clause.Len() > 0) {
			Query.Concat(" ");
			if (!Clause.PregMatch("/^where\\s/i")) Query.Concat("where ");
			Query.Concat(Clause);
		}
		return 1;
	}
	SetError(345, (const char*)Method);
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
int Database::Save(const char* method, const char* table, CAssocArray& a, const char* clause, const CAssocArray& exclude, const CAssocArray& types)
{
	CString Query;
	if (!SaveGenQuery(Query, method, table, a, clause, exclude, types)) return 0;
	return Exec(Query);
}

int Database::ReadKeyValue(CAssocArray& res, const char* query, const char* keyname, const char* valname)
{
	if (!query) {
		SetError(194, "int Database::ReadKeyValue(CAssocArray *res, ==> const char *query <==, const char *keyname, const char *valname)");
		return 0;
	}
	if (!keyname) {
		SetError(194, "int Database::ReadKeyValue(CAssocArray *res, const char *query, ==> const char *keyname <==, const char *valname)");
		return 0;
	}
	Result* r=Query(query);
	if (!r) return 0;
	int32_t nr=(int32_t)r->Rows();
	CAssocArray row;
	for (int32_t i=0;i < nr;i++) {
		row.Clear();
		if (r->FetchArray(row, i)) {
			const char* key=row[keyname];
			if (!key) {
				delete r;
				SetError(294, "%s", keyname);
				return 0;
			}
			if (valname) {
				//$a[$r[$keyfield]]=$r[$valfield];
				const char* val=row[valname];
				if (val) res.Set(key, val);
			} else {
				res.Set(key, row);
			}
		}
	}
	delete r;
	return 1;
}

uint64_t Database::InsertKey(const char* table, CAssocArray& a, const char* keyname, const CAssocArray& exclude)
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
	if (!Execf("lock tables %s write", table)) {
		PushError();
		Exec("unlock tables");
		PopError();
		return 0;
	}
	uint64_t id;
	CAssocArray r;
	if (!ExecArrayf(r, "select max(%s)+1 as newid from %s", keyname, table)) {
		PushError();
		Exec("unlock tables");
		PopError();
		return 0;
	}
	id=atoll(r.Get("newid"));
	if (!id) id=1;
	a.Setf(keyname, "%llu", id);
	int ret=Save("insert", table, a, NULL, exclude);
	//PrintError();
	PushError();
	Exec("unlock tables");
	PopError();
	if (!ret) return 0;
	return id;
}


/*******************************************************************************************************
 * Die Nachfolgenden Funktionen sind virtuell und müssen von den einzelnen Datenbank-Klassen
 * implementiert werden
 *******************************************************************************************************
 */
#endif

void Database::connect(const AssocArray& params)
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
	throw UnimplementedVirtualFunctionException("Database::connect");
}

/*!\brief Verbindung zu Datenbank trennen
 *
 * \descr
 * Mit diesem Befehl wird die Verbindung zur Datenbank getrennt. Zu diesem Zeitpunkt sollten keine Query-
 * Results (ppl7::db::Result) mehr vorhanden sein, die mit diesem Datenbank-Connect erstellt wurden. Es
 * könnte sonst zu Fehlern kommen.
 *
 * @return Die Funktion liefert 1 zurück, wenn die Verbindung zur Datenbank erfolgreich getrennt wurde,
 * im Fehlerfall 0.
 */
void Database::close()
{
	throw UnimplementedVirtualFunctionException("Database::close");
}

void Database::reconnect()
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
	throw UnimplementedVirtualFunctionException("Database::reconnect");
}


void Database::selectDB(const String& databasename)
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
	throw UnimplementedVirtualFunctionException("Database::selectDB");
}

void Database::exec(const String& query)
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
	throw UnimplementedVirtualFunctionException("Database::exec");
}

ResultSet* Database::query(const String& query)
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
	throw UnimplementedVirtualFunctionException("Database::query");
}

bool Database::ping()
/*!\brief Erreichbarkeit der Datenbank prüfen
 *
 * \descr
 * Mit dieser Funktion können Clients, die geraume Zeit untätig waren, prüfen, ob die Verbindung zum
 * Datenbankserver noch zur Verfügung steht. Je nach Implementierung (z.B. bei MySQL) kann es sein, dass
 * durch Aufruf dieser Funktion eine abgebrochene Verbindung automatisch wieder aufgebaut wird.
 *
 * @return Die Funktion liefert \p true zurück, wenn die Datenbankverbindung noch besteht. Ist dies nicht der
 * Fall, wird \p false zurückgegeben. Es kann dann mit der Funktion Database::reconnect versucht werden, die
 * Verbindung wieder herzustellen.
 */
{
	throw UnimplementedVirtualFunctionException("Database::ping");
}

String Database::escape(const String& str) const
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
	throw UnimplementedVirtualFunctionException("Database::escape");
}

uint64_t Database::getAffectedRows()
/*!\brief Betroffene Zeilen
 *
 * \descr
 * War der vorausgehende Datenbank-Query ein Select, liefert diese Funktion die Anzahl
 * Zeilen im Ergebnis zurück. Handelete es sich um ein Update/Insert/Replace, wird die Anzahl
 * betroffener bzw. veränderter Datensätze zurückgegeben.
 *
 * @return Anzahl betroffender Datensätze
 */
{
	throw UnimplementedVirtualFunctionException("Database::getAffectedRows");
}

void Database::startTransaction()
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
	throw UnimplementedVirtualFunctionException("Database::startTransaction");
}

void Database::endTransaction()
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
	throw UnimplementedVirtualFunctionException("Database::endTransaction");
}

void Database::cancelTransaction()
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
	throw UnimplementedVirtualFunctionException("Database::rollbackTransaction");
}

void Database::cancelTransactionComplete()
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
	throw UnimplementedVirtualFunctionException("Database::rollbackTransactionComplete");
}

void Database::createDatabase(const String& name)
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
	throw UnimplementedVirtualFunctionException("Database::createDatabase");
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
String Database::databaseType() const
{
	throw UnimplementedVirtualFunctionException("Database::databaseType");
}

/*
void Database::prepare(const String &preparedStatementName, const String &query)
{
	throw UnimplementedVirtualFunctionException("Database::prepare");
}

ResultSet *Database::execute(const String &preparedStatementName, const Array &params)
{
	throw UnimplementedVirtualFunctionException("Database::execute");
}

void Database::deallocate(const String &preparedStatementName)
{
	throw UnimplementedVirtualFunctionException("Database::deallocate");
}
*/

}	// EOF namespace db
}	// EOF namespace ppl7
