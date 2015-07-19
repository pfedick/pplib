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


class PoolCheck : public CThread
{
	private:
	public:
		PoolEx	*pool;
		int			intervall;
		virtual void ThreadMain(void *param) {
			ppluint64 now,next=ppl6::GetMilliSeconds()+intervall;

			while (!ThreadShouldStop()) {
				now=ppl6::GetMilliSeconds();
				if (now>next) {
					pool->CheckPool();
					next=ppl6::GetMilliSeconds()+intervall;
				}
				MSleep(200);
			}
		};
};

/*!\class PoolTree
 * \ingroup PPLGroupDatabases
 * \brief AVL-Baum zum Verwalten der Datenbankpools innerhalb der Klasse PoolEx
 *
 */

int PoolTree::Compare (const void *value1, const void *value2) const
{
	Pool *p1=(Pool*)value1;
	Pool *p2=(Pool*)value2;
	if (p2->Id > p1->Id) return 1;
	if (p2->Id < p1->Id) return -1;
	return 0;
}

int PoolTree::DestroyValue (void *item) const
{
	Pool *p=(Pool*)item;
	if (p) delete p;
	return 1;
}

int PoolTree::GetValue (const void *item, CString &buffer) const
{
	Pool *p=(Pool*)item;
	if (!p) return 0;
	buffer.Setf("Pool %s",(const char*)p->Name);
	return 1;
}



/*!\class PoolEx
 * \ingroup PPLGroupDatabases
 * \brief Datenbank-Pool für multiple Datenbanken
 *
 * \header \#include <ppl6-db.h>
 *
 * \desc
 * Diese Klasse fast beliebig viele einzelne Datenbank-Pools zu einem gemeinsamen Pool
 * zusammen. Eine Datenbank-Verbindung kann dann anhand der ID des Pools oder dessen Namen
 * angefordert werden.
 * \par Verwendung:
 * Um die Klasse verwenden zu können, müssen zunächst die zu verwendeten Datenbanken definiert werden.
 * Dazu wird die Funktion PoolEx::CreatePool mit einer eindeutigen Id, einem Namen und den Connect-Parametern
 * aufgerufen. Für jede Datenbank wird dadurch ein eigener Pool angelegt, der über seine Id oder seinen
 * Namen angesprochen werden kann.
 * \par
 * Mittels PoolEx::SetPoolOption oder PoolEx::SetPoolOptions können verschiedene Parameter des
 * jeweiligen Pools verändert werden.
 * \par
 * Mit PoolEx::Get kann eine Datenbank-Verbindung aus dem Pool angefordert werden, wobei der Pool
 * selbst dafür sorgt, dass neue Verbindungen automatisch erstellt werden). Mit PoolEx::Release
 * kann die Verbindung wieder in den Pool zurückgegeben werden, mit Pool::Destroy wird sie gelöscht.
 * Das kann zum Beispiel dann sinnvoll sein, wenn die Anwendung festgestellt hat, dass irgendwas nicht in
 * Ordnung ist.
 * \par
 * In regelmäßigen Abständen muß die Funktion PoolEx::CheckPool aufgerufen werden, am besten innerhalb eines
 * Überwachungsthreads. Die Funktion prüft die vorhandenen Verbindungen in allen Pools, löscht überflüssige
 * Verbindungen oder baut neue auf, wenn nicht mehr genug freie im Pool vorhanden sind.
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_PoolEx_Example1
 * \until EOF
 */

/*!\var PoolEx::DefaultMin
 * \brief Gibt an, wieviele Connects mindestens im Pool enthalten sein sollen
 */

/*!\var PoolEx::DefaultMax
 * \brief Gibt an, wieviele Connects maximal geöffnet werden dürfen. Default: 0 (uneingeschränkt)
 */


/*!\var PoolEx::DefaultMinSpare
 * \brief Gibt an, wieviele Freie Connects minimal vorgehalten werden sollen. Default: 0
 */

/*!\var PoolEx::DefaultMaxSpare
 * \brief Gibt an, wieviele Freie Connects maximal vorgehalten werden sollen. Sind mehr Connects
 * frei, werden diese nach erreichen des Timeouts abgebaut. Default: 0
 */

/*!\var PoolEx::DefaultGrow
 * \brief Gibt an, wieviele neue Connects gleichzeitig erstellt werden sollen, wenn keine freien
 *   Connects mehr im Pool sind oder PoolEx::DefaultMinSpare unterschritten wurde. Es wird immer mindestens eine neue
 *   Verbindung erstellt, aber nicht mehr als PoolEx::DefaultMaxSpare. Default: 1
 */


/*!\var PoolEx::DefaultTimeout
 * \brief Gibt an, nach welcher Zeit in Sekunden eine unbenutzte Verbindung abgebaut werden
 *   soll. Default: 300 (5 Minuten)
 */

/*!\var PoolEx::DefaultKeepAlive
 * \brief Gibt an, nach wieviel Sekunden ein Ping an eine freie Datenbank-Verbindung
 *   geschickt werden soll, um sicherzustellen, dass die Verbindung aufrecht erhalten bleibt.
 *   Ein Wert von 0 bedeutet, dass kein KeepAlive geschickt wird. Default: 60 (1 Minute).
 */


/*!\var PoolEx::Pools
 * \brief Ein AVL-Baum, in dem die einzelnen Pools verwaltet werden. Als Schlüssel wird dabei die
 * Id verwendet.
 */

/*!\var PoolEx::Log
 * \brief Pointer auf eine Logklasse, der über die Funktion PoolEx::SetLogfile gesetzt wird. Enthält die Variable
 * einen Wert != 0 ist das Logging aktiviert. Das Logging wird automatich an alle neuen Pools weitervererbt,
 * die nach Aufruf der Funktion angelegt werden.
 */


/*!\var PoolEx::Mutex
 * \brief Ein Muetx, der zur Gewährleistung der Threadsicherheit von allen Funktionen verwendet wird.
 */


/*!\var PoolEx::CheckThread
 * \brief Bei Aufruf der Funktion PoolEx::StartPoolCheck wird ein eigener Thread gestartet, dessen
 * Pointer in dieser Variablen gespeichert wird.
 */

/*!\brief Konstruktor der Klasse
 *
 * \desc
 * Der Destruktor initialisiert verschiedene Variablen, wie Timeout, KeepAlive. Diese
 * können mit folgenden Funktionen überschrieben werden:
 * - PoolEx::SetDefaultOption
 * - PoolEx::SetDefaultOptions
 * - PoolEx::SetDefaultTimeout
 * - PoolEx::SetDefaultKeepAlive
 * - PoolEx::SetDefaultMin
 * - PoolEx::SetDefaultMax
 * - PoolEx::SetDefaultMinSpare
 * - PoolEx::SetDefaultMaxSpare
 * - PoolEx::SetDefaultGrow
 */
PoolEx::PoolEx()
{
	DefaultTimeout=60*5;		// Timeout auf 5 Minuten setzen
	DefaultKeepAlive=60;		// Keepalive auf 1 Minute setzen
	DefaultMin=0;
	DefaultMax=0;
	DefaultMinSpare=0;
	DefaultMaxSpare=0;
	DefaultGrow=1;
	CheckThread=NULL;
	Log=NULL;
}

/*!\brief Destruktor der Klasse
 */
PoolEx::~PoolEx()
{
	if (Log) Log->Printf(ppl6::LOG::INFO,2,"ppl6::db::PoolEx","~PoolEx",__FILE__,__LINE__,"Destruktor aufgerufen");
	if (CheckThread) {
		if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::db::PoolEx","~PoolEx",__FILE__,__LINE__,"Stoppe Überwachungsthread");
		CheckThread->ThreadSignalStop();
		CheckThread->ThreadStop();
		delete CheckThread;
	}
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::db::PoolEx","~PoolEx",__FILE__,__LINE__,"Lösche vorhandene Datenbanken");
	Pools.Clear();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::db::PoolEx","~PoolEx",__FILE__,__LINE__,"Done");
}

/*!\brief Verbindungen in den Pools löschen
 *
 * \desc
 * Mit dieser Funktion werden Verbindungen in den definierten Pools gelöscht. Abhängig von den gesetzten Parametern
 * werden entweder nur freie Verbindungen, benutzte Verbindungen oder beides gelöscht.
 *
 * \param[in] free Optionales Flag, was angibt, ob die freien Verbindungen gelöscht werden sollen (Default=true)
 * \param[in] used Optionales Flag, was angibt, ob die benutzten Verbindungen gelöscht werden sollen.
 * Es wird davon abgeraten, in Benutzung befindliche Verbindungen zu löschen, da es dadurch zum
 * Programmabsturz oder unvorhersehbarem Verhalten kommen kann. (Default=false)
 */
void PoolEx::ClearPools(bool free, bool used)
{
	if (free) {
		if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::db::PoolEx","ClearPools",__FILE__,__LINE__,"Lösche freie Verbindungen in allen Pools");
		Mutex.Lock();
		Pools.Reset();
		Pool *p;
		while ((p=(Pool*)Pools.GetNext())) {
			p->ClearFreePool(true);
		}
		Mutex.Unlock();
		if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::db::PoolEx","ClearPools",__FILE__,__LINE__,"Done");
	}
	if (used) {
		if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::db::PoolEx","ClearPools",__FILE__,__LINE__,"Lösche benutzte Verbindungen in allen Pools");
		Mutex.Lock();
		Pools.Reset();
		Pool *p;
		while ((p=(Pool*)Pools.GetNext())) {
			p->ClearUsedPool(true);
		}
		Mutex.Unlock();
		if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::db::PoolEx","ClearPools",__FILE__,__LINE__,"Done");
	}


}

/*!\brief Pool anhand seines Namens finden
 *
 * \descr
 * Eine interne Funktion, um einen bestimmten Pool anhand seines Namens zu finden.
 *
 * \param[in] name Name des gesuchten Pools
 * \return Liefert einen Pointer auf den gesuchten Pool zurück, oder NULL, wenn kein
 * passender Pool gefunden wurde.
 */
Pool *PoolEx::FindPoolByName(const char *name)
{
	Pools.Reset();
	Pool *p;
	while ((p=(Pool *)Pools.GetNext())) {
		if (p->Name==name) return p;
	}
	SetError(440,"%s",name);
	return NULL;
}

/*!\brief Logging aktivieren
 *
 * \descr
 * Mit dieser Funktion wird das Logging der Klasse aktiviert oder deaktiviert. Ist es aktiviert,
 * schreibt jeder Funktionsaufruf eine oder mehrere Zeilen in die angegebene Logfile-Klasse.
 * Außerdem wird das Logging automatich an alle neuen Pools weitervererbt, die nach Aufruf
 * dieser Funktion angelegt werden.
 *
 * @param[in] log Pointer auf eine Klasse vom Typ CLog, um das Logging zu aktivieren, oder NULL
 * um es zu deaktivieren.
 *
 */
void PoolEx::SetLogfile(CLog *log)
{
	Mutex.Lock();
	Log=log;
	Mutex.Unlock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::db::PoolEx","SetLogfile",__FILE__,__LINE__,"Debugging fuer PoolEx aktiviert");
}


/*!\brief Neuen Datenbank-Pool definieren
 *
 * \desc
 * Mit dieser Funktion wird ein neuer Datenbank-Pool definiert. Jeder Datenbank muss dabei
 * eine eindeutige ID und ein eindeutiger Name zugeordnet werden, sowie die Connect-
 * Parameter und optionale zusätzliche Optionen.
 * \param[in] id Eindeutige ID der Datenbank. Ist eine ID bereits vorhanden, werden die
 * alten Daten überschrieben. Die ID "0" hat eine besondere Bedeutung. Sie definiert
 * quasi eine Default-Datenbank, die immer dann verwendet wird, wnn die Funktion
 * Pool::Get ohne Parameter oder mit NULL aufgerufen wird.
 * \param[in] name Ein eindeutiger Name für diese Datenbank.
 * \param[in] connect Die Connect-Parameter in einem Assoziativen Array, wie sie von der Funktion
 * ppl6::db::Connect unterstützt werden.
 *
 * \returns Die Funktion liefert bei Erfolg true (1) zurück, sonst false (0).
 * \remarks
 * Die Funktion kann im laufenden Betrieb mehrfach für die gleiche Datenbank aufgerufen werden.
 * Die alten Daten werden dabei überschrieben. Sofern sich die Connect-Parameter ändern, werden
 * alle Connects im Free-Pool sofort gelöscht, Connects, die in Benutzung sind, werden nach deren
 * Freigabe gelöscht. Neu angeforderte Connects werden mit den neuen Parametern erstellt.
 *
 * \note
 * Der neue Pool wird mit den Default-Optionen konfiguriert, die über die "SetDefault..."-Funktionen
 * dieser Klasse gesetzt wurden. Durch Aufruf der Funktionen PoolEx::SetPoolOption und PoolEx::SetPoolOptions
 * können diese individuell angepasst werden.
 */
int PoolEx::CreatePool(int id, const char *name, CAssocArray &connect)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::PoolEx","DefineDatabase",__FILE__,__LINE__,"Definiere Datenbank ID=%i, Name=%s",id,name);
	if (!name) {
		SetError(194,"");
		if (Log) Log->LogError("ppl6::db::PoolEx","DefineDatabase",__FILE__,__LINE__);
		return 0;
	}

	Mutex.Lock();
	Pool *dupname=NULL;
	Pools.Reset();
	while ((dupname=(Pool*)Pools.GetNext())) {
		if (dupname->Name==name) break;
	}
	Pool findpool;
	findpool.Id=id;

	Pool *pool=(Pool *)Pools.Find(&findpool);
	// Wenn es diese Datenbank noch nicht gibt, legen wir sie an.
	if (!pool) {
		// Allerdings nur, wenn der Name noch nicht existiert
		if (dupname) {
			SetError(441,"Name: %s, Id: %i",name,dupname->Id);
			Mutex.Unlock();
			if (Log) Log->LogError("ppl6::db::PoolEx","DefineDatabase",__FILE__,__LINE__);
			return 0;
		}
		pool=new Pool;
		if (!pool) {
			Mutex.Unlock();
			SetError(2);
			if (Log) Log->LogError("ppl6::db::PoolEx","DefineDatabase",__FILE__,__LINE__);
			return 0;
		}
		// Default-Optionen kopieren
		pool->Min=DefaultMin;
		pool->Max=DefaultMax;
		pool->MinSpare=DefaultMinSpare;
		pool->MaxSpare=DefaultMaxSpare;
		pool->Grow=DefaultGrow;
		pool->Timeout=DefaultTimeout;
		pool->KeepAlive=DefaultKeepAlive;
		pool->Id=id;
		if (!pool->SetConnectParams(connect)) {
			PushError();
			delete pool;
			Mutex.Unlock();
			PopError();
			if (Log) Log->LogError("ppl6::db::PoolEx","DefineDatabase",__FILE__,__LINE__);
			return 0;
		}

		if (!Pools.Add(pool)) {
			PushError();
			delete pool;
			Mutex.Unlock();
			PopError();
			if (Log) Log->LogError("ppl6::db::PoolEx","DefineDatabase",__FILE__,__LINE__);
			return 0;
		}
		pool->SetLogfile(Log);

	} else {
		// Der Name darf noch nicht bei einer anderen Datenbank verwendet worden sein.
		if (dupname!=NULL && dupname!=pool) {
			SetError(441,"Name: %s, Id: %i",name,dupname->Id);
			Mutex.Unlock();
			if (Log) Log->LogError("ppl6::db::PoolEx","DefineDatabase",__FILE__,__LINE__);
			return 0;
		}
	}
	pool->Name=name;
	if (!pool->SetConnectParams(connect)) {
		Mutex.Unlock();
		if (Log) Log->LogError("ppl6::db::PoolEx","DefineDatabase",__FILE__,__LINE__);
		return 0;
	}
	Mutex.Unlock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::db::PoolEx","DefineDatabase",__FILE__,__LINE__,"Datenbankpool wurde erfolgreich erstellt");
	return 1;
}

/*!\brief Neuen Datenbank-Pool definieren
 *
 * \desc
 * Mit dieser Funktion wird ein neuer Datenbank-Pool definiert. Jeder Datenbank muss dabei
 * eine eindeutige ID und ein eindeutiger Name zugeordnet werden, sowie die Connect-
 * Parameter und optionale zusätzliche Optionen.
 * \param[in] id Eindeutige ID der Datenbank. Ist eine ID bereits vorhanden, werden die
 * alten Daten überschrieben. Die ID "0" hat eine besondere Bedeutung. Sie definiert
 * quasi eine Default-Datenbank, die immer dann verwendet wird, wnn die Funktion
 * Pool::Get ohne Parameter oder mit NULL aufgerufen wird.
 * \param[in] name Ein eindeutiger Name für diese Datenbank.
 * \param[in] connect Die Connect-Parameter in einem Assoziativen Array, wie sie von der Funktion
 * ppl6::db::Connect unterstützt werden.
 * \param[in] options Zusätzliche Optionen für den Pool in einem Assoziativen Array. Die Möglichen
 * Optionen sind der Funktion PoolEx:SetPoolOptions zu entnehmen
 *
 * \returns Die Funktion liefert bei Erfolg true (1) zurück, sonst false (0).
 * \remarks
 * Die Funktion kann im laufenden Betrieb mehrfach für die gleiche Datenbank aufgerufen werden.
 * Die alten Daten werden dabei überschrieben. Sofern sich die Connect-Parameter ändern, werden
 * alle Connects im Free-Pool sofort gelöscht, Connects, die in Benutzung sind, werden nach deren
 * Freigabe gelöscht. Neu angeforderte Connects werden mit den neuen Parametern erstellt.
 *
 * \note
 * Der neue Pool wird mit den Default-Optionen konfiguriert, die über die "SetDefault..."-Funktionen
 * dieser Klasse gesetzt wurden. Durch Aufruf der Funktionen PoolEx::SetPoolOption und PoolEx::SetPoolOptions
 * können diese individuell angepasst werden.
 */
int PoolEx::CreatePool(int id, const char *name, CAssocArray &connect, CAssocArray &options)
{
	if (!CreatePool(id,name,connect)) return 0;
	return SetPoolOptions(id,options);
}

/*!\brief Default-Optionen für neue Pools anhand eines Assoziativen Arrays setzen
 *
 * \desc
 * Mit dieser Funktion können die Default-Optionen für neue Pools mittels eines Assoziativen Arrays
 * gesetzt werden. Dabei wird das Array durchwandert und für jedes Element die Funktion
 * PoolEx::SetDefaultOption aufgerufen
 *
 * \param[in] options Ein Assoziatives Array mit den zu setzenden Default-Werten.
 * Die möglichen Werte sind in der Funktion PoolEx::SetDefaultOption beschrieben.
 * \return Die Funktion liefert bei Erfolg true (1) zurück, sonst false (0).
 */
int PoolEx::SetDefaultOptions(CAssocArray &options)
{
	options.Reset();
	CString Key, Value;
	while ((options.GetNext(Key,Value))) {
		if (!SetDefaultOption(Key,Value)) return 0;
	}
	return 1;
}

/*!\brief Pool-spezifische Option setzen
 *
 * \desc
 * Mit dieser Funktion können die Default-Optionen für neue Pools gesetzt werden. Folgende Optionen
 * können verändert werden:
 * - \b min: Gibt an, wieviele Connects mindestens im Pool enthalten sein sollen. Default = 0
 * - \b max: Gibt an, wieviele Connects maximal geöffnet werden dürfen. Default = 0 = uneingeschränkt
 * - \b minspare: Gibt an, wieviele freie Connects minimal vorgehalten werden sollen. Default = 0
 * - \b maxspare: Gibt an, wieviele freie Connects maximal vorgehalten werden sollen. Sin mehr Connects
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
int PoolEx::SetDefaultOption(const CString &Name, const CString &Value)
{
	Mutex.Lock();
	CString Opt=Name;
	Opt.LCase();
	if (Opt=="min") DefaultMin=Value.ToInt();
	else if (Opt=="max") DefaultMax=Value.ToInt();
	else if (Opt=="minspare") DefaultMinSpare=Value.ToInt();
	else if (Opt=="maxspare") DefaultMaxSpare=Value.ToInt();
	else if (Opt=="grow") DefaultGrow=Value.ToInt();
	else if (Opt=="timeout") DefaultTimeout=Value.ToInt();
	else if (Opt=="keepalive") DefaultKeepAlive=Value.ToInt();
	else {
		Mutex.Unlock();
		SetError(541,"%s",(const char *)Name);
		return 0;
	}
	if (Log) {
		Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::PoolEx","SetDefaultOption",__FILE__,__LINE__,"Option %s wurde auf %s gesetzt.",
				(const char *)Opt,(const char*)Value);
	}
	Mutex.Unlock();
	return 1;
}

/*!\brief Option eines bestimmten Pools setzen
 *
 * \desc
 * Mit dieser Funktion können Optionen eines bestimmten Pools gesetzt werden. Die möglichen Optionen
 * sind in der Funktion Pool::SetOption dokumentiert.
 *
 * \param[in] id Die Id des gewünschten Pools
 * \param[in] Name Ein String, der den Namen der Option enthält.
 * \param[in] Value Ein String mit dem Wert der Option.
 * \returns Die Funktion liefert bei Erfolg true (1) zurück, sonst false (0).
 */
int PoolEx::SetPoolOption(int id, const CString &Name, const CString &Value)
{
	Pool *p=GetPool(id);
	if (!p) return 0;
	return p->SetOption(Name,Value);
}

/*!\brief Optionen eines bestimmten Pools anhand eines assoziativen Arrays setzen
 *
 * \desc
 * Mit dieser Funktion können Optionen eines bestimmten Pools mittels eines Assoziativen Arrays
 * gesetzt werden. Dabei wird das Array durchwandert und für jedes Element die Funktion
 * Pool::SetOption aufgerufen
 *
 * \param[in] id Die Id des gewünschten Pools
 * \param[in] options Ein Assoziatives Array mit zusätzlichen Optionen für die Verwaltung
 * des Pools. Dieses kann folgende Werte enthalten:
 * - \b min: Gibt an, wieviele Connects mindestens im Pool enthalten sein sollen. Default = 0
 * - \b max: Gibt an, wieviele Connects maximal geöffnet werden dürfen. Default = 0 = uneingeschränkt
 * - \b minspare: Gibt an, wieviele Freie Connects minimal vorgehalten werden sollen. Default = 0
 * - \b maxspare: Gibt an, wieviele Freie Connects maximal vorgehalten werden sollen. Sind mehr Connects
 *   frei, werden diese nach Erreichen des Timeouts abgebaut. Default = 0
 * - \b grow: Gibt an, wieviele neue Connects gleichzeitig erstellt werden sollen, wenn keine freien
 *   Connects mehr im Pool sind oder MinSpare unterschritten wurde. Es wird immer mindestens eine neue
 *   Verbindung erstellt, aber nicht mehr als \b maxspare. Default = 1
 * - \b timeout: Gibt an, nach welcher Zeit in Sekunden eine unbenutzte Verbindung abgebaut werden
 *   soll. Default = 300 (5 Minuten)
 * - \b keepalive: Gibt an, nach wieviel Sekunden ein Ping an eine freie Datenbank-Verbindung
 *   geschickt werden soll, um sicherzustellen, dass die Verbindung aufrecht erhalten bleibt.
 *   Ein Wert von 0 bedeutet, dass kein KeepAlive geschickt wird. Default = 60 (1 Minute).
 *
 * \return Die Funktion liefert bei Erfolg true (1) zurück, sonst false (0).
 */
int PoolEx::SetPoolOptions(int id, CAssocArray &options)
{
	Pool *p=GetPool(id);
	if (!p) return 0;
	return p->SetOptions(options);
}


/*!\brief Datenbank aus dem Pool löschen
 *
 * \desc
 * Mit dieser Funktion wird eine Datenbank aus dem Pool gelöscht. Dabei werden sämtliche noch
 * vorhandenen Connects ebenfalls gelöscht, egal ob frei oder in Benutzung. Die Funktion ist
 * daher im laufenden Betrieb mit Vorsicht zu genießen.
 *
 * \param[in] id Die Id des Datenbank-Pools, der gelöscht werden soll
 * \returns Bei Erfolg gibt die Funktion true (1) zurück, sonst false (0).
 */
int PoolEx::DeletePool(int id)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::PoolEx","DeleteDatabase",__FILE__,__LINE__,"Lösche Datenbank ID=%i",id);
	Mutex.Lock();
	Pool findpool;
	findpool.Id=id;

	Pool *pool=(Pool *)Pools.Find(&findpool);
	if (!pool) {
		Mutex.Unlock();
		SetError(440,"%i",id);
		if (Log) Log->LogError("ppl6::db::PoolEx","DeleteDatabase",__FILE__,__LINE__);
		return 0;
	}
	delete pool;
	Mutex.Unlock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::db::PoolEx","DeleteDatabase",__FILE__,__LINE__,"Datenbankpool wurde gelöscht");
	return 1;
}

/*!\brief Default Timeout verändern
 *
 * \desc
 * Mit dieser Funktion kann der Timeout für unbenutzte Verbindungen im Pool verändert werden.
 * Die Funktion Pool::CheckPool prüft bei regelmäßigem Aufruf innerhalb eines Wartungsthreads,
 * ob der Timeout erreicht ist und löscht die Verbindung, falls dies der Fall ist.
 *
 * \param[in] seconds Anzahl Sekunden, nach denen der Timeout auftreten soll.
 * \returns Die Funktion liefert immer true (1) zurück.
 */
int PoolEx::SetDefaultTimeout(int seconds)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::PoolEx","SetDefaultTimeout",__FILE__,__LINE__,"Timeout=%i",seconds);
	DefaultTimeout=seconds;
	return 1;
}


/*!\brief Default KeepAlive verändern
 *
 * \desc
 * Um die Verbindung zur Datenbank aufrecht zu erhalten, schickt die Funktion Pool::CheckPool
 * bei regelmäßigem Aufruf innerhalb eines Wartungsthreads einen Ping an die Datenbank. Besteht die
 * Verbindung noch, wird sie weiter aufrecht erhalten, war der Ping nicht erfolgreich, wird sie
 * gelöscht.
 *
 * \param[in] seconds Anzahl Sekunden, nach denen ein KeepAlive auftreten soll.
 * \returns Die Funktion liefert immer true (1) zurück.
 */
int PoolEx::SetDefaultKeepAlive(int seconds)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::PoolEx","SetDefaultKeepAlive",__FILE__,__LINE__,"KeepAlive=%i",seconds);
	DefaultKeepAlive=seconds;
	return 1;
}

/*!\brief Default Min verändern
 *
 * \desc
 * Legt den Default für die minimale Anzahl Connects zur Datenbank fest.
 *
 * \param[in] nr Anzahl Connects
 * \returns Die Funktion liefert immer true (1) zurück.
 */
int PoolEx::SetDefaultMin(int nr)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::PoolEx","SetDefaultMin",__FILE__,__LINE__,"Min=%i",nr);
	DefaultMin=nr;
	return 1;
}

/*!\brief Default Max verändern
 *
 * \desc
 * Legt den Default für die maximale Anzahl Connects zur Datenbank fest.
 *
 * \param[in] nr Anzahl Connects. Ein Wert von 0 bedeutet "keine Limitierung".
 * \returns Die Funktion liefert immer true (1) zurück.
 */
int PoolEx::SetDefaultMax(int nr)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::PoolEx","SetDefaultMax",__FILE__,__LINE__,"Max=%i",nr);
	DefaultMax=nr;
	return 1;
}

/*!\brief Default MinSpare verändern
 *
 * \desc
 * Legt den Default für die minimale Anzahl freier Connects zur Datenbank fest.
 *
 * \param[in] nr Anzahl Connects
 * \returns Die Funktion liefert immer true (1) zurück.
 */
int PoolEx::SetDefaultMinSpare(int nr)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::PoolEx","SetDefaultMinSpare",__FILE__,__LINE__,"MinSpare=%i",nr);
	DefaultMinSpare=nr;
	return 1;
}

/*!\brief Default MinSpare verändern
 *
 * \desc
 * Legt den Default für die maximale Anzahl freier Connects zur Datenbank fest.
 *
 * \param[in] nr Anzahl Connects
 * \returns Die Funktion liefert immer true (1) zurück.
 */
int PoolEx::SetDefaultMaxSpare(int nr)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::PoolEx","SetDefaultMaxSpare",__FILE__,__LINE__,"MaxSpare=%i",nr);
	DefaultMaxSpare=nr;
	return 1;
}

/*!\brief Default Grow verändern
 *
 * \desc
 * Legt fest, wieviele Datenbank-Connects gleichzeitig aufgebaut werden sollen, wenn
 * ein neuer Connect angefordert wird, aber keine freien Connects im Pool sind, oder wenn
 * MinSpare unterschritten wird.
 *
 * \param[in] nr Anzahl Connects
 * \returns Die Funktion liefert immer true (1) zurück.
 */
int PoolEx::SetDefaultGrow(int nr)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::PoolEx","SetDefaultGrow",__FILE__,__LINE__,"Grow=%i",nr);
	DefaultGrow=nr;
	return 1;
}



/*!\brief Datenbank-Verbindung anhand ihres Namens aus dem Pool holen
 *
 * \desc
 * Die Funktion sucht zuerst den passenden Pool und holt dort entweder einen freien Connect heraus
 * oder erstellt einen neuen Connect. Dazu muss die entsprechenden Datenbank vorher mit
 * PoolEx::CreatePool definiert worden sein.
 *
 * \param[in] name Pointer auf den Namen der Datenbank-Verbindung. Mit
 * PoolEx::DefineDatabase muss zuvor eine passende Datenbank definiert worden sein.
 * \param[in] wait Optionales Flag, das angibt, ob die Funktion warten soll, wenn das Maximum an
 * Connects für diesen Pool bereits erreicht ist.
 * \param[in] ms Ein optionaler Wert, der in Kombination mit dem \p wait-Flag angibt, wieviele
 * Millisekunden gewartet werden soll, bevor mit einem Timeout abgebrochen wird.
 *
 * \returns Die Funktion liefert einen Pointer auf die Datenbank-Verbindung zurück
 * oder NULL im Fehlerfall.
 * \remarks Die Verbindung muss nach Gebrauch entweder mit der Funktion PoolEx::Release
 * zurück in den Pool gegeben oder mit PoolEx::Destroy gelöscht werden. Falls kein
 * Wartungsthread läuft, der die Funktion CheckPool aufruft, kann das Handle auch
 * einfach mit "delete" gelöscht werden, es wird jedoch empfohlen die Destroy-Funktion
 * zu verwenden.
 * \note
 * Aus Performancegründen sollte die Funktion statt mit dem Namen der Datenbank mit deren ID
 * aufgerufen werden, da dies etwas schneller ist.
 *
 */
Database *PoolEx::Get(const char *name, bool wait, int ms)
{
	if (!name) return Get(0);
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::PoolEx","Get",__FILE__,__LINE__,"Suche Pool für Datenbank \"%s\"",name);
	Mutex.Lock();
	Pool *pool=FindPoolByName(name);
	if (!pool) {
		Mutex.Unlock();
		if (Log) Log->LogError("ppl6::db::PoolEx","Get",__FILE__,__LINE__);
		return 0;
	}
	Database *db=pool->Get(wait,ms);
	Mutex.Unlock();
	if (db) {
		db->poolex=this;
		if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::PoolEx","Get",__FILE__,__LINE__,"Datenbank gefunden");
	} else {
		if (Log) Log->Printf(ppl6::LOG::ERROR,4,"ppl6::db::PoolEx","Get",__FILE__,__LINE__,"Fehlschlag");
	}

	return db;
}


/*!\brief Datenbank-Verbindung anhand ihrer ID aus dem Pool holen
 *
 * \desc
 * Die Funktion sucht zuerst den passenden Pool und holt dort entweder einen freien Connect heraus
 * oder erstellt einen neuen Connect. Dazu muss die entsprechenden Datenbank vorher mit
 * PoolEx::CreatePool definiert worden sein.
 *
 * \param[in] id ID der Datenbank, die mit Pool::DefineDatabase definiert wurde.
 * \param[in] wait Optionales Flag, das angibt, ob die Funktion warten soll, wenn das Maximum an
 * Connects für diesen Pool bereits erreicht ist.
 * \param[in] ms Ein optionaler Wert, der in Kombination mit dem \p wait-Flag angibt, wieviele
 * Millisekunden gewartet werden soll, bevor mit einem Timeout abgebrochen wird.
 *
 * \returns Die Funktion liefert einen Pointer auf die Datenbank-Verbindung zurück
 * oder NULL im Fehlerfall.
 * \remarks Die Verbindung muss nach Gebrauch entweder mit der Funktion PoolEx::Release
 * zurück in den Pool gegeben oder mit PoolEx::Destroy gelöscht werden. Falls kein
 * Wartungsthread läuft, der die Funktion CheckPool aufruft, kann das Handle auch
 * einfach mit "delete" gelöscht werden, es wird jedoch empfohlen die Destroy-Funktion
 * zu verwenden.
 */
Database *PoolEx::Get(int id, bool wait, int ms)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::PoolEx","Get",__FILE__,__LINE__,"Suche Pool für Datenbank ID \"%i\"",id);
	Mutex.Lock();
	Pool findpool;
	findpool.Id=id;
	Pool *pool=(Pool *)Pools.Find(&findpool);
	if (!pool) {
		SetError(440,"%i",id);
		Mutex.Unlock();
		if (Log) Log->LogError("ppl6::db::PoolEx","Get",__FILE__,__LINE__);
		return NULL;
	}
	Database *db=pool->Get(wait,ms);
	Mutex.Unlock();
	if (db) {
		db->poolex=this;
		if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::PoolEx","Get",__FILE__,__LINE__,"Datenbank gefunden");
	} else {
		if (Log) Log->Printf(ppl6::LOG::ERROR,4,"ppl6::db::PoolEx","Get",__FILE__,__LINE__,"Fehlschlag");
	}
	return db;
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
 */
int PoolEx::Release(Database *db)
{
	if (!db) return 1;
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::PoolEx","Release",__FILE__,__LINE__,"Gebe Datebank frei");
	if (db->poolex!=this) {
		SetError(439);
		if (Log) Log->LogError("ppl6::db::PoolEx","Release",__FILE__,__LINE__);
		return 0;
	}
	if (!db->pool) {
		SetError(439);
		if (Log) Log->LogError("ppl6::db::PoolEx","Release",__FILE__,__LINE__);
		return 0;
	}
	return db->pool->Release(db);
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
int PoolEx::Destroy(Database *db)
{
	if (!db) return 1;
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::db::PoolEx","Destroy",__FILE__,__LINE__,"Lösche Datebankverbindung");
	if (db->poolex!=this) {
		SetError(439);
		if (Log) Log->LogError("ppl6::db::PoolEx","Destroy",__FILE__,__LINE__);
		return 0;
	}
	if (!db->pool) {
		SetError(439);
		if (Log) Log->LogError("ppl6::db::PoolEx","Destroy",__FILE__,__LINE__);
		return 0;
	}
	return db->pool->Destroy(db);
}


/*!\brief Pool anhand seiner ID finden
 *
 * \descr
 * Mit dieser Funktion kann ein bestimmter Pool anhand seiner ID gefunden werden.
 *
 * \param[in] id ID des gesuchten Pools
 * \return Liefert einen Pointer auf den gesuchten Pool zurück, oder NULL, wenn kein
 * passender Pool gefunden wurde.
 */
Pool *PoolEx::GetPool(int id)
{
	Mutex.Lock();
	Pool findpool;
	findpool.Id=id;

	Pool *pool=(Pool *)Pools.Find(&findpool);
	Mutex.Unlock();
	if (!pool) {
		SetError(440,"%i",id);
		return NULL;
	}
	return pool;
}

/*!\brief Pool anhand seines Namens finden
 *
 * \descr
 * Mit dieser Funktion kann ein bestimmter Pool anhand seines Namens gefunden werden. Sie ruft intern
 * die Funktion PoolEx::FindPoolByName auf.
 *
 * \param[in] name Name des gesuchten Pools
 * \return Liefert einen Pointer auf den gesuchten Pool zurück, oder NULL, wenn kein
 * passender Pool gefunden wurde.
 */
Pool *PoolEx::GetPool(const char *name)
{
	if (!name) {
		SetError(194,"Pool *PoolEx::GetPool(==> const char *name <==)");
		return NULL;
	}
	Mutex.Lock();
	Pool *pool=FindPoolByName(name);
	Mutex.Unlock();
	return pool;
}

/*!\brief Datenbank-Pools überprüfen
 *
 * \desc
 * Diese Funktion sollte regelmäßig innerhalb eines Überwachungsthreads aufgerufen werden,
 * um die in den einzelnen Pools vorhandenen Verbindungen zu überprüfen und zu expiren. Die Funktion schickt
 * dabei an alle unbenutzen Verbindungen einen Ping (siehe Pool::SetKeepAlive) und
 * löscht Verbindungen, die nicht mehr reagieren.
 * \par
 * Beinhaltet der Pool sehr viele unbenutzte Verbindungen, wird empfohlen diese Funktion innerhalb
 * eines eigenen Threads zu starten, damit weitere Funktionen nicht blockiert werden.
 * \par
 * Die Funktion wird auch verwendet, wenn PoolEx::StartPoolCheck aufgerufen wird.
 */
void PoolEx::CheckPool()
{
	if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::PoolEx","CheckPool",__FILE__,__LINE__,"CheckPool gestartet");
	double now=ppl6::GetMicrotime();
	Pool *pool=NULL;
	Mutex.Lock();
	Pools.Reset();
	while (1) {
		pool=(Pool *)Pools.GetNext();
		if (!pool) break;
		if (pool->LastCheck!=now) {
			Mutex.Unlock();
			pool->CheckPool();
			Mutex.Lock();
			pool->LastCheck=now;
			Pools.Reset();
		}
	}
	Mutex.Unlock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::db::PoolEx","CheckPool",__FILE__,__LINE__,"CheckPool beendet");
}

/*!\brief Überwachungsthread für den Pool starten
 *
 * \desc
 * Mit Aufruf dieseer Funktion wird ein eigener Überwachungsthread für diesen Pool gestartet. Dieser
 * tut nichts anderes als alle paar Sekunden die Funktion PoolEx:CheckPool aufzurufen.
 * \param[in] intervall Ein Optionaler Parameter, der das Interavall in Millisekunden angibt, in dem die
 * CheckPool-Funktion aufgerufen werden soll. Der Defauklt ist 10000 (=10 Sekunden)
 * \returns Bei Erfolg liefert die Funktion true (1) zurück, im Fehlerfall false (0).
 * \remarks
 * Die Funktion kann mehrfach aufgerufen werden, um das Intervall nachträglich zu ändern.
 *
 */
int PoolEx::StartPoolCheck(int intervall)
{
	Mutex.Lock();
	if (!CheckThread) {
		CheckThread=new PoolCheck;
		if (!CheckThread) {
			SetError(2);
			Mutex.Unlock();
			return 0;
		}
		((PoolCheck*)CheckThread)->pool=this;
		((PoolCheck*)CheckThread)->intervall=intervall;
	}
	if (CheckThread->ThreadIsRunning()) {
		((PoolCheck*)CheckThread)->intervall=intervall;
		Mutex.Unlock();
		return 1;
	}
	int ret=CheckThread->ThreadStart();
	Mutex.Unlock();
	return ret;
}

/*!\brief Überwachungsthread stoppen
 *
 * \desc
 * Mit dieser Funktion wird ein zuvor gestarteter Überwachungsthread für diesen Pool wieder gestoppt.
 *
 * \returns Die Funktion liefert immer true (1) zurück.
 *
 * \see PoolEx::StartPoolCheck
 */
int PoolEx::StopPoolCheck()
{
	Mutex.Lock();
	if (!CheckThread) {
		Mutex.Unlock();
		return 1;
	}
	CheckThread->ThreadSignalStop();
	CheckThread->ThreadStop();
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
int PoolEx::GetStatus(CAssocArray &status)
{
	if (Log) Log->Printf(ppl6::LOG::DEBUG,6,"ppl6::db::PoolEx","GetStatus",__FILE__,__LINE__,"Status abgefragt");
	CAssocArray r;

	Mutex.Lock();
	Pools.Reset();
	Pool *p;
	CString Key;
	while ((p=(Pool*)Pools.GetNext())) {
		r.Clear();
		p->GetStatus(r);
		Key.Setf("%i",p->Id);
		status.Set(Key,r);
	}

	Mutex.Unlock();
	return 1;
}


}	// EOF namespace db
}	// EOF namespace ppl6
