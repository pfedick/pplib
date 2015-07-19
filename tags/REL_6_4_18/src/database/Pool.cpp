/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
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


/*!\class Pool
 * \ingroup PPLGroupDatabases
 * \brief Klasse zur Verwaltung von Datenbank-Verbindungen
 *
 * \header \#include <ppl6-db.h>
 *
 * \desc
 * Mit dieser Klasse können beliebig viele Connects zu einer bestimmten Datenbank innerhalb
 * eines Pools verwaltet werden.
 * \par Verwendung:
 * Bevor der Pool verwendet werden kann, muss er zunächst einmal wissen, welche Datenbank er verwenden soll. Dazu muss
 * die Funktion Pool::SetConnectParams mit den entsprechenden Parametern in einem assoziativen Array aufgerufen
 * werden. Die möglichen Parameter sind dabei die gleichen, wie beim Aufruf der Funktion Database::Connect.
 * Optional können mit den Funktion Pool::SetOption oder Pool::SetOptions verschiedene Einstellungen zum verhalten
 * des Pools verändert werden.
 * \par
 * Nach der Initialisierung können anschließend mit der Funktion Pool::Get Datenbank-Verbindungen aus dem Pool
 * geholt werden (der Pool sorgt selbst dafür, dass neue Verbindungen automatisch erstellt werden) und mit
 * Pool::Release wieder in den Pool zurückgegeben werden. Mit Pool::Destroy kann eine Verbindung auch gelöscht
 * werden, was zum Beispiel dann sinnvoll ist, wenn die Anwendung festgestellt hat, dass irgendwas nicht in
 * Ordnung ist.
 * \par
 * In regelmäßigen Abständen muß die Funktion Pool::CheckPool aufgerufen werden, am besten innerhalb eines
 * Überwachungsthreads. Die Funktion prüft die vorhandenen Verbindungen, löscht überflüssige Verbindungen oder
 * baut neue auf, wenn nicht mehr genug freie im Pool vorhanden sind.
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_Pool_Example1
 * \until EOF

 */


/*!\var Pool::Min
 * \brief Gibt an, wieviele Connects mindestens im Pool enthalten sein sollen
 */

/*!\var Pool::Max
 * \brief Gibt an, wieviele Connects maximal geöffnet werden dürfen. Default: 0 (uneingeschränkt)
 */


/*!\var Pool::MinSpare
 * \brief Gibt an, wieviele Freie Connects minimal vorgehalten werden sollen. Default: 0
 */

/*!\var Pool::MaxSpare
 * \brief Gibt an, wieviele Freie Connects maximal vorgehalten werden sollen. Sind mehr Connects
 * frei, werden diese nach erreichen des Timeouts abgebaut. Default: 0
 */

/*!\var Pool::Grow
 * \brief Gibt an, wieviele neue Connects gleichzeitig erstellt werden sollen, wenn keine freien
 *   Connects mehr im Pool sind oder Pool::MinSpare unterschritten wurde. Es wird immer mindestens eine neue
 *   Verbindung erstellt, aber nicht mehr als Pool::MaxSpare. Default: 1
 */


/*!\var Pool::Timeout
 * \brief Gibt an, nach welcher Zeit in Sekunden eine unbenutzte Verbindung abgebaut werden
 *   soll. Default: 300 (5 Minuten)
 */

/*!\var Pool::KeepAlive
 * \brief Gibt an, nach wieviel Sekunden ein Ping an eine freie Datenbank-Verbindung
 *   geschickt werden soll, um sicherzustellen, dass die Verbindung aufrecht erhalten bleibt.
 *   Ein Wert von 0 bedeutet, dass kein KeepAlive geschickt wird. Default: 60 (1 Minute).
 */

/*!\var Pool::IsInit
 * \brief Wird auf \p true gesetzt, wenn die Connect-Parameter über die Funktion Pool::SetConnectParams
 * übergeben wurden.
 */

/*!\var Pool::Name
 * \brief Der Name des Pools, der über die Funktion Pool::SetName festgelegt werden kann.
 */

/*!\var Pool::Hash
 * \brief Ein MD5-Hash über die Connect-Parameter, der bei Aufruf der Funktion Pool::SetConnectParams gebildet wird.
 */

/*!\var Pool::Mutex
 * \brief Ein Muetx, der zur Gewährleistung der Threadsicherheit von allen Funktionen verwendet wird.
 */

/*!\var Pool::Id
 * \brief Eine eindeutige Id, die bei Verwendung des Pools durch die Klasse PoolEx vergeben wird.
 */

/*!\var Pool::LastCheck
 * \brief Wird von der Klasse PoolEx verwendet und enthält den Timestamp der letzten Überprüfung des Pools.
 */

/*!\var Pool::Used
 * \brief Eine Liste, die alle gerade in Verwendung befindlichen Datenbank-Connects enthält.
 */

/*!\var Pool::Free
 * \brief Eine Liste, die alle derzeit freien Datenbank-Connects enthält.
 */

/*!\var Pool::Log
 * \brief Pointer auf eine Logklasse, der über die Funktion Pool::SetLogfile gesetzt wird. Enthält die Variable
 * einen Wert != 0 ist das Logging aktiviert.
 */

/*!\var Pool::ConnectParam
 * \brief Enthält eine Kopie der Connect-Parameter, die über die Funktion Pool::SetConnectParams gesetzt werden.
 */


/*!\brief Löschen von Datenbank-Connects im Pool
 * \relates Pool
 *
 * \desc
 * Diese interne statische Funktion wird benutzt, um Datenbank-Connects im Pool zu löschen. Sie wird von den
 * Listen Pool::Used und Pool::Free aufgerufen.
 *
 * @param[in] item Der zu löschende Datenbank-Connect
 * @param[in] data Pointer auf die Pool-Klasse, wird jedoch nicht verwendet
 * @return Liefert immer 1 zurück
 */
static int PoolDestroyFunction(void *item,void *data)
{
	Database *db=(Database*)item;
	if (db) delete db;
	return 1;
}

/*!\brief Konstruktor der Klasse
 *
 * \desc
 * Der Konstruktor setzt diverse Parameter auf ihren Default-Wert:
 *
 * - \b min=0: Gibt an, wieviele Connects mindestens im Pool enthalten sein sollen
 * - \b max=0: Gibt an, wieviele Connects maximal geöffnet werden dürfen. Default = uneingeschränkt
 * - \b minspare=0: Gibt an, wieviele Freie Connects minimal vorgehalten werden sollen
 * - \b maxspare=0: Gibt an, wieviele Freie Connects maximal vorgehalten werden sollen. Sind mehr Connects
 *   frei, werden diese nach erreichen des Timeouts abgebaut.
 * - \b grow=1: Gibt an, wieviele neue Connects gleichzeitig erstellt werden sollen, wenn keine freien
 *   Connects mehr im Pool sind oder MinSpare unterschritten wurde. Es wird immer mindestens eine neue
 *   Verbindung erstellt, aber nicht mehr als \b maxspare.
 * - \b timeout=300: Gibt an, nach welcher Zeit in Sekunden eine unbenutzte Verbindung abgebaut werden
 *   soll. Default = 300 (5 Minuten)
 * - \b keepalive=60: Gibt an, nach wieviel Sekunden ein Ping an eine freie Datenbank-Verbindung
 *   geschickt werden soll, um sicherzustellen, dass die Verbindung aufrecht erhalten bleibt.
 *   Ein Wert von 0 bedeutet, dass kein KeepAlive geschickt wird. Default = 60 (1 Minute).
 */
Pool::Pool()
{
	Id=-1;
	Min=0;
	Max=0;
	MinSpare=0;
	MaxSpare=0;
	Grow=1;
	Timeout=300;
	KeepAlive=60;
	IsInit=false;
	Log=NULL;
	LastCheck=0;
	Used.SetDestroyFunction(PoolDestroyFunction,this);
	Free.SetDestroyFunction(PoolDestroyFunction,this);
}

/*!\brief Destruktor der Klasse
 *
 * \desc
 * Der Destruktor sorgt dafür, dass alle Datenbank-Connects sauber getrennt werden.
 */
Pool::~Pool()
{
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::db::Pool","~Pool",__FILE__,__LINE__,"Destruktor aufgerufen fuer Pool id=%i, name=\"%s\", loesche Pools",Id,(const char*)Name);
	if (Free.Num()) {
		if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::Pool","~Pool",__FILE__,__LINE__,"Loesche Free-Pool, %i Connects",Free.Num());
		Free.Clear(true);
		if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::Pool","~Pool",__FILE__,__LINE__,"Done");
	}
	if (Used.Num()) {
		if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::Pool","~Pool",__FILE__,__LINE__,"Loesche Used-Pool, %i Connects",Used.Num());
		if (Log) Log->Printf(ppl6::LOG::ERROR,1,"ppl6::db::Pool","~Pool",__FILE__,__LINE__,"Im Used Pool des Pools \"%s\", ID: %i sind noch %i Verbindungen!",(const char*)Name, Id,Used.Num());
		Used.Clear(true);
		if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::Pool","~Pool",__FILE__,__LINE__,"Done");
	}


}


/*!\brief Logging aktivieren
 *
 * \descr
 * Mit dieser Funktion wird das Logging der Klasse aktiviert oder deaktiviert werden. Ist es aktiviert,
 * schreibt jeder Funktionsaufruf eine oder mehrere Zeilen in die angegebene Logfile-Klasse.
 *
 * @param[in] log Pointer auf eine Klasse vom Typ CLog, um das Logging zu aktivieren, oder NULL
 * um es zu deaktivieren.
 *
 */
void Pool::SetLogfile(CLog *log)
{
	Mutex.Lock();
	Log=log;
	Mutex.Unlock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::db::Pool","SetLogfile",__FILE__,__LINE__,"Debugging fuer Pool id=%i, name=\"%s\" aktiviert",Id,(const char*)Name);
}

/*!\brief Berechnet Hash aus Connect-Parametern
 * \desc
 * Diese Funktion berechnet aus den Connect-Parametern einen eindeutigen
 * MD5-Hash.
 *
 * \param[out] hash Zielstring, in dem der Hash gespeichert werden
 * soll
 * \param[in] param Ein Assoziatives Array, in dem die Connect-Parameter
 * enthalten sind.
 *
 * \returns Bei Erfolg liefert die Funktion true (1) zurück, sonst false(0).
 *
 * \remarks
 * Die Funktion verwendet folgende Keys, um den MD5-Hash zu erstellen:
 * - type
 * - interface (sofern angegeben)
 * - host
 * - port
 * - user
 * - password
 * - dbname
 * - charset
 */
int Pool::CalcHash(CString &hash, CAssocArray &param)
{
	hash.Clear();
	hash.Concatf("type:%s|",param["type"]);
	hash.Concatf("interface:%s|",param["interface"]);
	hash.Concatf("host:%s|",param["host"]);
	hash.Concatf("port:%s|",param["port"]);
	hash.Concatf("user:%s|",param["user"]);
	hash.Concatf("password:%s|",param["password"]);
	hash.Concatf("dbname:%s|",param["dbname"]);
	hash.Concatf("charset:%s|",param["charset"]);
	hash.MD5();
	return 1;
}

/*!\brief Namen des Pools festlegen
 *
 * \desc
 * Mit Aufruf dieser Funktion wird der Name des Datenbank-Pools festgelegt. Der Name wird an verschiedenen
 * Stellen verwendet, meist in Listen oder Status-Ausgaben.
 *
 * @param[in] Name Der Name des Pools
 * @return Die Funktion gibt immer 1 zurück
 */
int Pool::SetName(const CString &Name)
{
	this->Name=Name;
	return 1;
}

/*!\brief Datenbank-Pool mit den Connect-Parametern initialisieren
 *
 * \desc
 * Mit dieser Funktion werden die Connect-Parameter für die Datenbank festgelegt und der Pool somit
 * initialisiert. Mit der Funktionen Pool::SetOption oder Pool::SetOptions können weitere Pool-spezifische
 * Optionen gesetzt werden. Mit der Funktion Pool::SetName kann dem Pool ein Name zugewiesen werden.
 *
 * \param[in] connect Die Connect-Parameter in einem Assoziativen Array, wie sie von der Funktion
 * ppl6::db::Connect, bzw. der jeweiligen Datenbank-Klasse unterstützt werden.
 * \returns Die Funktion liefert bei Erfolg true (1) zurück, sonst false (0).
 */
int Pool::SetConnectParams(CAssocArray &connect)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::Pool","SetConnectParams",__FILE__,__LINE__,"Setze Connect Parameter für Pool id=%i, name=\"%s\"",Id,(const char*)Name);
	CString NewHash;
	if (!CalcHash(NewHash,connect)) {
		if (Log) Log->LogError("ppl6::db::Pool","Init",__FILE__,__LINE__);
		return 0;
	}
	Mutex.Lock();
	// Falls sich die Connect-Parameter geändert haben, löschen wir alle noch vorhandenen
	// Parameter aus dem Freien Pool
	if (NewHash!=Hash && Hash.Len()>0) {
		if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::db::Pool","Init",__FILE__,__LINE__,"%s: Connect-Parameter haben sich geaendert, loesche Free-Pool",(const char*)Name);
		// TODO: Free.Clear(true);
		Hash=NewHash;
	} else if (Hash.Len()==0) {
		Hash=NewHash;
	}
	ConnectParam.Clear();
	ConnectParam.Copy(connect);
	if (Log) {
		ppl6::CAssocArray a;
		a.Copy(connect);
		a.Set("password","xxxxxxxxxxx");
		Log->PrintArraySingleLine(ppl6::LOG::DEBUG,10,"ppl6::db::Pool","Init",__FILE__,__LINE__,&a,"Connect-Parameter: ");
	}
	IsInit=true;
	Mutex.Unlock();
	return 1;
}

/*!\brief Pool-spezifische Optionen anhand eines Assoziativen Arrays setzen
 *
 * \desc
 * Mit dieser Funktion können Pool-spezifische Optionen mittels eines Assoziativen Arrays
 * gesetzt werden. Dabei wird das Array durchwandert und für jedes Element die Funktion
 * Pool::SetOption aufgerufen
 *
 * \param[in] options Ein Assoziatives Array mit zusätzlichen Optionen für die Verwaltung
 * des Pools. Die möglichen Werte sind in der Funktion Pool::SetOption beschrieben.
 * \return Die Funktion liefert bei Erfolg true (1) zurück, sonst false (0).
 */
int Pool::SetOptions(CAssocArray &options)
{
	CString Key, Value;
	options.Reset();
	while ((options.GetNext(Key,Value))) {
		if (!SetOption(Key,Value)) return 0;
	}
	return 1;
}

/*!\brief Pool-spezifische Option setzen
 *
 * \desc
 * Mit dieser Funktion können Pool-spezifische Optionen gesetzt werden. Folgende Optionen
 * können verändert werden:
 * - \b min: Gibt an, wieviele Connects mindestens im Pool enthalten sein sollen. Default = 0
 * - \b max: Gibt an, wieviele Connects maximal geöffnet werden dürfen. Default = 0 = uneingeschränkt
 * - \b minspare: Gibt an, wieviele Freie Connects minimal vorgehalten werden sollen. Default = 0
 * - \b maxspare: Gibt an, wieviele Freie Connects maximal vorgehalten werden sollen. Sin mehr Connects
 *   frei, werden diese nach erreichen des Timeouts abgebaut. Default = 0
 * - \b grow: Gibt an, wieviele neue Connects gleichzeitig erstellt werden sollen, wenn keine freien
 *   Connects mehr im Pool sind oder MinSpare unterschritten wurde. Es wird immer mindestens eine neue
 *   Verbindung erstellt, aber nicht mehr als \b maxspare. Default = 1
 * - \b timeout: Gibt an, nach welcher Zeit in Sekunden eine unbenutzte Verbindung abgebaut werden
 *   soll. Default = 300 (5 Minuten)
 * - \b keepalive: Gibt an, nach wieviel Sekunden ein Ping an eine freie Datenbank-Verbindung
 *   geschickt werden soll, um sicherzustellen, dass die Verbindung aufrecht erhalten bleibt.
 *   Ein Wert von 0 bedeutet, dass kein KeepAlive geschickt wird. Default = 60 (1 Minute).
 *
 * \param[in] Name Ein String, der den Namen der Option enthält.
 * \param[in] Value Ein String mit dem Wert der Option.
 * \returns Die Funktion liefert bei Erfolg true (1) zurück, sonst false (0).
 */
int Pool::SetOption(const CString &Name, const CString &Value)
{
	Mutex.Lock();
	CString Opt=Name;
	Opt.LCase();
	if (Opt=="min") Min=Value.ToInt();
	else if (Opt=="max") Max=Value.ToInt();
	else if (Opt=="minspare") MinSpare=Value.ToInt();
	else if (Opt=="maxspare") MaxSpare=Value.ToInt();
	else if (Opt=="grow") Grow=Value.ToInt();
	else if (Opt=="timeout") Timeout=Value.ToInt();
	else if (Opt=="keepalive") KeepAlive=Value.ToInt();
	else {
		Mutex.Unlock();
		SetError(541,"Pool::SetOption Key=>>%s<<",(const char *)Name);
		return 0;
	}
	if (Log) {
		Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::Pool","SetOption",__FILE__,__LINE__,"Pool %i:%s: Option %s wurde auf %s gesetzt.",
				Id,(const char*)Name, (const char *)Opt,(const char*)Value);
	}
	Mutex.Unlock();
	return 1;
}

/*!\brief Interne Funktion zum Erstellen eines neuen Datenbank-Connects
 *
 * \desc
 * Diese Funktion wird intern von Pool::Get und Pool::CheckPool aufgerufen, um eine
 * neue Datenbank-Verbindung zu erstellen.
 *
 * \returns Die Funktion liefert einen Pointer auf eine neue Datenbankverbindung
 * zurück, sofern eine neue Verbindung aufgebaut werden konnte, oder NULL im Fehlerfall.
 * Dabei wird zunächst geprüft, ob das Maximum an Verbindungen - sofern definiert - bereits
 * erreicht ist. Ist dies der Fall, wird eine Fehlermeldung zurückgegeben.
 *
 * \remarks
 * Die Funktion geht davon aus, dass der Mutex der Klasse bereits gesetzt ist.
 */
Database *Pool::New()
{
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::db::Pool","New",__FILE__,__LINE__,"%i:%s: Neuen Connect erstellen",Id,(const char*)Name);
	// Einen neuen Connect gibts nur, wenn Max>0 ist
	if (Max>0) {
		// Oder die Anzahl Connects insgesammt kleiner als Max sind
		if (Used.Num()+Free.Num()>=Max) {
			SetError(442,"%i",Max);
			return NULL;
		}
	}
	Database *p=Connect(ConnectParam);
	if (p) {
		p->pool=this;
		p->Hash=Hash;
	}
	return p;
}

/*!\brief Datenbank-Connect holen
 *
 * \desc
 * Mit dieser Funktion wird eine Verbindung aus dem Datenbank-Pool geholt oder eine neue erstellt.
 *
 * \param[in] wait Optionales Flag, das angibt, ob die Funktion warten soll, wenn das Maximum an
 * Connects für diesen Pool bereits erreicht ist.
 * \param[in] ms Ein optionaler Wert, der in Kombination mit dem \p wait-Flag angibt, wieviele
 * Millisekunden gewartet werden soll, bevor mit einem Timeout abgebrochen wird. Ist der Wert 0,
 * wird endlos gewartet.
 * \returns Die Funktion gibt entweder ein gültiges Handle für die Datenbank zurück, oder im
 * Fehlerfall NULL.
 *
 * \remarks
 * Die Funktion prüft immer, ob die Datenbankverbindung noch funktionsfähig ist, indem ein
 * Ping geschickt wird. Es ist daher sichergestellt, dass nur funktionierende Verbindungen
 * zurückgegeben werden.
 */
Database *Pool::Get(bool wait, int ms)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::Pool","Get",__FILE__,__LINE__,"%i:%s: Verbindung aus dem Pool holen, wait=%i, ms=%i",Id,(const char*)Name,(int)wait,ms);
	if (!IsInit) {
		SetError(443);
		if (Log) Log->LogError("ppl6::db::Pool","Get",__FILE__,__LINE__);
		return NULL;
	}
	Database *p;
	Mutex.Lock();
	// Zuerst schauen wir mal, ob wir noch etwas im Pool haben
	Free.Reset();
	while ((p=(Database *)Free.GetNext())) {
		if (p->poollock==false) {
			// Ist die Datenbank auch noch benutzbar?
			p->poollock=true;
			Mutex.Unlock();
			if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::Pool","Get",__FILE__,__LINE__,"Freie Verbindung im Pool gefunden, prüfe Funktionalität (Ping)");
			if (!p->Ping()) {				// Datenbank noch verbunden?
				if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::Pool","Get",__FILE__,__LINE__,"Ping fehlgeschlagen, trenne Verbindung, Fehler: %i: %s[%s]",ppl6::GetErrorCode(),ppl6::GetError(), ppl6::GetExtendedError());
				Mutex.Lock();
				Free.Delete(p);
				delete p;
				Free.Reset();
				continue;
			}
			Mutex.Lock();
			p->poollock=false;
			Free.Delete(p);
			Used.Add(p);
			Mutex.Unlock();
			if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::Pool","Get",__FILE__,__LINE__,"Verwende Verbindung aus dem Pool");
			return p;
		}
	}
	if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::Pool","Get",__FILE__,__LINE__,"Keine Verbindung im Pool, erstelle neue Verbindung");
	// Wir brauchen eine neue Verbindung
	double start=ppl6::getmicrotime();
	double diff=(double)ms/1000;

	// Wir wollen den Mutex nicht gesperrt halten, während wir eine neue Verbindung
	// aufbauen, da dies länger dauern kann (im ungünstigsten Fall bis zum Timeout)
	Mutex.Unlock();
	while (1) {
		p=New();
		if (p) break;
		if (!wait) {
			if (Log) Log->LogError("ppl6::db::Pool","Get",__FILE__,__LINE__);
			return NULL;
		}
		if (p==NULL && GetErrorCode()!=442) {
			if (Log) Log->LogError("ppl6::db::Pool","Get",__FILE__,__LINE__);
			return NULL;
		}
		if (ms) {
			if (start+diff<ppl6::getmicrotime()) {
				SetError(442,"Timeout");
				if (Log) Log->LogError("ppl6::db::Pool","Get",__FILE__,__LINE__);
				return NULL;
			}
		}
		ppl6::MSleep(10);
	}
	Mutex.Lock();
	Used.Add(p);
	Mutex.Unlock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::Pool","Get",__FILE__,__LINE__,"Neue Verbindung erstellt");
	return p;
}

/*!\brief Datenbankverbindung in den Pool zurückgeben
 *
 * \desc
 * Mit dieser Funktion wird eine zuvor mit Pool::Get geholte Verbindung wieder
 * zurück in den Pool gestellt.
 *
 * \param[in] db Pointer auf eine Datenbankverbindung. Diese muss zuvor mit der
 * Funktion Pool::Get geholt worden sein.
 * \returns Konnte die Verbindung erfolgreich in den Pool zurückgestellt werden,
 * liefert die Funktion true (1) zurück, sonst false (0).
 * \remarks Wurde zwischenzeitlich die Init-Funktion mit veränderten Connect-Parametern
 * aufgerufen, wird die Verbindung nicht zurück in den Pool gestellt, sondern gelöscht.
 */
int Pool::Release(Database *db)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::Pool","Release",__FILE__,__LINE__,"%i:%s: Verbindung in den Pool zurückstellen",Id,(const char*)Name);
	if (!IsInit) {
		SetError(443);
		if (Log) Log->LogError("ppl6::db::Pool","Release",__FILE__,__LINE__);
		return 0;
	}
	if (!db) {
		if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::Pool","Release",__FILE__,__LINE__,"Parameter ist NULL, nichts zu tun");
		return 1;
	}
	if (db->pool!=this) {
		if (Log) Log->LogError("ppl6::db::Pool","Release",__FILE__,__LINE__);
		SetError(439);
		return 0;
	}
	if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::Pool","Release",__FILE__,__LINE__,"Pruefe, ob Datenbank noch antwortet (Ping)");
	if (!db->Ping()) {				// Datenbank noch verbunden?
		Mutex.Lock();
		Used.Delete(db);
		delete db;
		Mutex.Unlock();
		if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::Pool","Release",__FILE__,__LINE__,"Verbindung war unbrauchbar und wurde geloescht");
		return 1;
	}
	db->lastuse=ppl6::GetTime();
	db->lastping=db->lastuse;
	Mutex.Lock();
	Used.Delete(db);
	// Falls sich die Connect-Parameter geändert haben, geben wir die Verbindung nicht zurück in
	// den Pool, sondern löschen sie.
	if (Hash!=db->Hash) {
		delete db;
		if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::Pool","Release",__FILE__,__LINE__,"Datenbank-Parameter haben sich geaendert, Verbindung wird geloescht");
	} else {
		Free.Add(db);
		db->poollock=false;
		if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::Pool","Release",__FILE__,__LINE__,"Verbindung wurde zurueck in den freien Pool gestellt");
	}
	Mutex.Unlock();
	return 1;
}

/*!\brief Datenbankverbindung löschen
 * \desc
 * Mit dieser Funktion wird eine zuvor mit Pool::Get geholte Verbindung getrennt
 * und aus dem Pool gelöscht.
 *
 * \param[in] db Pointer auf eine Datenbankverbindung. Diese muss zuvor mit der
 * Funktion Pool::Get geholt worden sein.
 * \returns Konnte die Verbindung erfolgreich gelöscht werden, liefert die Funktion
 * true (1) zurück, sonst false (0). Ein Fehler kann nur auftreten, wenn die Verbindung
 * nicht aus dem gleichen Pool stammt.
 */
int Pool::Destroy(Database *db)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::Pool","Destroy",__FILE__,__LINE__,"%i:%s: Verbindung löschen",Id,(const char*)Name);
	if (!IsInit) {
		SetError(443);
		if (Log) Log->LogError("ppl6::db::Pool","Destroy",__FILE__,__LINE__);
		return 0;
	}
	if (db) {
		if (db->pool!=this) {
			SetError(439);
			if (Log) Log->LogError("ppl6::db::Pool","Destroy",__FILE__,__LINE__);
			return 0;
		}
		Mutex.Lock();
		Used.Delete(db);
		delete db;
		Mutex.Unlock();
		if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::Pool","Destroy",__FILE__,__LINE__,"Verbindung wurde gelöscht");
	} else {
		if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::Pool","Destroy",__FILE__,__LINE__,"Parameter ist NULL, nichts zu tun");
	}
	return 1;
}


/*!\brief Alle benutzen Datenbank-Verbindungen trennen
 *
 * \desc
 * Mit dieser Funktion werden alle Datenbank-Verbindungen getrennt, die gerade in
 * Benutzung sind. Diese Funktion sollte - wenn überhaupt - nur bei Beendigung eines
 * Programms aufgerufen werden. Bei Aufruf im Laufenden Betrieb könnte es zu
 * unvorhersehbaren Abstürzen kommen.
 *
 * \param[in] destroy Optionales Flag. Ist es auf true gesetzt, werden die Datenbank-
 * Klassen mit delete gelöscht. Falls nicht (default), werden die Connects nur aus
 * dem Pool gelöscht, bleiben aber bestehen.
 * \returns Die Funktion liefert immer true (1) zurück.
 */
int Pool::ClearUsedPool(bool destroy)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::Pool","ClearUsedPool",__FILE__,__LINE__,"Pool: %i:%s, %i Connects: Pool wird geleert",Id,(const char*)Name, Used.Num());
	Mutex.Lock();
	if (Used.Num()>0 && Log!=NULL) Log->Printf(ppl6::LOG::ERROR,1,"ppl6::db::Pool","ClearUsedPool",__FILE__,__LINE__,"Im UsedPool \"%i:%s\" waren noch %i Connects, Programm könnte crashen!",Id,(const char*)Name,Used.Num());
	Used.Clear(destroy);
	Mutex.Unlock();
	return 1;
}

/*!\brief Alle freien Datenbank-Verbindungen trennen
 *
 * \desc
 * Mit dieser Funktion werden alle unbenutzen Datenbank-Verbindungen getrennt.
 * Diese Funktion sollte bei Beendigung eines Programms aufgerufen werden, um alle
 * Verbindungen korrekt zu trennen. Sie kann aber auch im Laufenden Betrieb aufgerufen
 * werden, um den Pool zu leeren.
 *
 * \param[in] destroy Optionales Flag. Ist es auf true gesetzt, werden die Datenbank-
 * Klassen mit delete gelöscht. Falls nicht (default), werden die Connects nur aus
 * dem Pool gelöscht, bleiben aber bestehen.
 * \returns Die Funktion liefert immer true (1) zurück.
 */
int Pool::ClearFreePool(bool destroy)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::Pool","ClearFreePool",__FILE__,__LINE__,"Pool: %i:%s, %i Connects: Pool wird geleert",Id,(const char*)Name, Free.Num());
	Mutex.Lock();
	Free.Clear(destroy);
	Mutex.Unlock();
	return 1;
}


/*!\brief Status sämtlicher Datenbank-Verbindungen auslesen
 *
 * \desc
 * Mit dieser Funktion kann der Status sämtlicher Datenbank-Verbindungen ausgelesen und in das
 * übergebene assoziative Array \p status gespeichert werden.
 *
 * @param[out] status Ein assoziatives Array, in dem der Status aller Connects gespeichert werden soll
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0
 */
int Pool::GetStatus(CAssocArray &status)
{
	if (Log) Log->Printf(ppl6::LOG::DEBUG,6,"ppl6::db::Pool","GetStatus",__FILE__,__LINE__,"%i:%s",Id,(const char*)Name);
	status.Clear();
	if (!IsInit) {
		SetError(443);
		if (Log) Log->LogError("ppl6::db::Pool","GetStatus",__FILE__,__LINE__);
		return 0;
	}
	CAssocArray item;
	Mutex.Lock();
	//status.Set("pools",&Databases,true);
	if (Id>=0) status.Setf("Id","%i",Id);
	if (Name.Len()>0) status.Setf("Name","%s",(const char*)Name);
	if (Hash.Len()>0) status.Setf("Hash","%s",(const char*)Hash);
	status.Setf("PoolUsed","%i",Used.Num());
	status.Setf("PoolFree","%i",Free.Num());
	status.Set("Connect",&ConnectParam);
	status.Setf("Options/Min","%i",Min);
	status.Setf("Options/Max","%i",Max);
	status.Setf("Options/MinSpare","%i",MinSpare);
	status.Setf("Options/MaxSpare","%i",MaxSpare);
	status.Setf("Options/Grow","%i",Grow);
	status.Setf("Options/Timeout","%i",Timeout);
	status.Setf("Options/KeepAlive","%i",KeepAlive);
	Database *p;
	Used.Reset();
	while ((p=(Database *)Used.GetNext())) {
		item.Clear();
		item.Set("status","used");
		if (p->Hash.Len()>0) item.Set("hash",p->Hash);
		item.Setf("lastping","%i",p->lastping);
		item.Setf("lastuse","%i",p->lastuse);
		status.Set("Pool/[]",&item,true);
	}
	Free.Reset();
	while ((p=(Database *)Free.GetNext())) {
		item.Clear();
		item.Set("status","free");
		if (p->Hash.Len()>0) item.Set("hash",p->Hash);
		item.Setf("lastping","%i",p->lastping);
		item.Setf("lastuse","%i",p->lastuse);
		status.Set("Pool/[]",&item,true);
	}


	Mutex.Unlock();
	return 1;
}

/*!\brief Datenbank-Pools überprüfen
 *
 * \desc
 * Diese Funktion sollte regelmäßig innerhalb eines Überwachungsthreads aufgerufen werden,
 * um die im Pool vorhandenen Verbindungen zu überprüfen und zu expiren. Die Funktion schickt
 * regelmäßig an alle unbenutzen Verbindungen einen Ping (KeepAlive) und
 * löscht Verbindungen, die nicht mehr reagieren.
 * \par
 * Bei sehr großen Pools wird empfohlen diese Funktion innerhalb eines eigenen Threads
 * aufzurufen, um andere Funktionen nicht zu blockieren.
 */
void Pool::CheckPool()
{
	Mutex.Lock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,6,"ppl6::db::Pool","CheckPool",__FILE__,__LINE__,"%i:%s, Connects: %i, Free: %i, Used: %i",Id,(const char*)Name, Free.Num()+Used.Num(),Free.Num(), Used.Num());
	Mutex.Unlock();
	ppluint64 now;
	Database *p;
	// Timeout
	if (Log) Log->Printf(ppl6::LOG::DEBUG,9,"ppl6::db::Pool","CheckPool",__FILE__,__LINE__,"Prüfe auf Timeout");

	now=ppl6::GetTime();
	Mutex.Lock();
	Free.Reset();
	while ((p=(Database*)Free.GetNext())) {
		if (Log) Log->Printf(ppl6::LOG::DEBUG,10,"ppl6::db::Pool","CheckPool",__FILE__,__LINE__,"Server: %s, lastuse: %llu, now: %llu, Timeout: %llu, locked: %i\n",(const char*)Name, p->lastuse,now, p->lastuse+Timeout,(int)p->poollock);
		if (p->poollock==false && p->lastuse+Timeout<now) {
			if (Log) Log->Printf(ppl6::LOG::DEBUG,7,"ppl6::db::Pool","CheckPool",__FILE__,__LINE__,"Timeout erreicht, lösche Verbindung\n");
			Free.Delete(p);
			delete p;
			Free.Reset();
		}
	}

	// Keepalive
	if (Log) Log->Printf(ppl6::LOG::DEBUG,9,"ppl6::db::Pool","CheckPool",__FILE__,__LINE__,"Prüfe auf KeepAlive");
	Free.Reset();
	while ((p=(Database*)Free.GetNext())) {
		now=ppl6::GetTime();
		if (p->poollock==false && p->lastping+KeepAlive<now) {
			p->poollock=true;
			Mutex.Unlock();
			if (Log) Log->Printf(ppl6::LOG::DEBUG,10,"ppl6::db::Pool","CheckPool",__FILE__,__LINE__,"Schicke Ping\n");
			//printf ("Ping: %s\n",(char*)p->ServerName);
			if (p->Ping()) {
				//printf ("Ping ok: %s\n",(char*)p->ServerName);
				if (Log) Log->Printf(ppl6::LOG::DEBUG,10,"ppl6::db::Pool","CheckPool",__FILE__,__LINE__,"Ping ok\n");
				p->lastping=ppl6::GetTime();
				Mutex.Lock();
				p->poollock=false;
			} else {				// Connect ist weg, wir löschen ihn aus dem Pool
				if (Log) Log->Printf(ppl6::LOG::DEBUG,7,"ppl6::db::Pool","CheckPool",__FILE__,__LINE__,"Ping fehlgeschlagen, lösche Verbindung\n");
				//printf ("Ping failed: %s\n",(char*)p->ServerName);
				Mutex.Lock();
				Free.Delete(p);
				delete p;
			}
			break;
		}
	}
	Mutex.Unlock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,8,"ppl6::db::Pool","CheckPool",__FILE__,__LINE__,"CheckPool fertig");
}



}	// EOF namespace db
}	// EOF namespace ppl6
