/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 *******************************************************************************
 * Copyright (c) 2016, Patrick Fedick <patrick@pfp.de>
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



/*!\class DBPool
 * \ingroup PPLGroupDatabases
 * \brief Klasse zur Verwaltung von Datenbank-Verbindungen
 *
 * \header \#include <ppl7-db.h>
 *
 * \desc
 * Mit dieser Klasse können beliebig viele Connects zu einer bestimmten Datenbank innerhalb
 * eines Pools verwaltet werden.
 * \par Verwendung:
 * Bevor der Pool verwendet werden kann, muss er zunächst einmal wissen, welche Datenbank er verwenden soll. Dazu muss
 * die Funktion SinglePool::setConnectParams mit den entsprechenden Parametern in einem assoziativen Array aufgerufen
 * werden. Die möglichen Parameter sind dabei die gleichen, wie beim Aufruf der Funktion Database::connect.
 * \par
 * Nach der Initialisierung können anschließend mit der Funktion SinglePool::get Datenbank-Verbindungen aus dem Pool
 * geholt werden (der Pool sorgt selbst dafür, dass neue Verbindungen automatisch erstellt werden) und mit
 * SinglePool::release wieder in den Pool zurückgegeben werden. Mit SinglePool::destroy kann eine Verbindung auch gelöscht
 * werden, was zum Beispiel dann sinnvoll ist, wenn die Anwendung festgestellt hat, dass irgendwas nicht in
 * Ordnung ist.
 * \par
 * In regelmäßigen Abständen muß die Funktion SinglePool::checkPool aufgerufen werden.
 * Die Funktion prüft die vorhandenen Verbindungen, löscht überflüssige Verbindungen oder
 * baut neue auf, wenn nicht mehr genug freie im Pool vorhanden sind.
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_Pool_Example1
 * \until EOF

 */


/*!\var DBPool::Min
 * \brief Gibt an, wieviele Connects mindestens im Pool enthalten sein sollen
 */

/*!\var DBPool::Max
 * \brief Gibt an, wieviele Connects maximal geöffnet werden dürfen. Default: 0 (uneingeschränkt)
 */


/*!\var DBPool::MinSpare
 * \brief Gibt an, wieviele Freie Connects minimal vorgehalten werden sollen. Default: 0
 */

/*!\var DBPool::MaxSpare
 * \brief Gibt an, wieviele Freie Connects maximal vorgehalten werden sollen. Sind mehr Connects
 * frei, werden diese nach erreichen des Timeouts abgebaut. Default: 0
 */

/*!\var DBPool::Grow
 * \brief Gibt an, wieviele neue Connects gleichzeitig erstellt werden sollen, wenn keine freien
 *   Connects mehr im Pool sind oder Pool::MinSpare unterschritten wurde. Es wird immer mindestens eine neue
 *   Verbindung erstellt, aber nicht mehr als Pool::MaxSpare. Default: 1
 */


/*!\var DBPool::IdleTimeout
 * \brief Gibt an, nach welcher Zeit in Sekunden eine unbenutzte Verbindung abgebaut werden
 *   soll. Default: 300 (5 Minuten)
 */

/*!\var DBPool::UsedTimeout
 * \brief Gibt an, nach welcher Zeit in Sekunden eine benutzte Verbindung abgebaut werden
 *   soll. Default: 0 (nie)
 */

/*!\var DBPool::KeepAlive
 * \brief Gibt an, nach wieviel Sekunden ein Ping an eine freie Datenbank-Verbindung
 *   geschickt werden soll, um sicherzustellen, dass die Verbindung aufrecht erhalten bleibt.
 *   Ein Wert von 0 bedeutet, dass kein KeepAlive geschickt wird. Default: 60 (1 Minute).
 */

/*!\var DBPool::IsInit
 * \brief Wird auf \p true gesetzt, wenn die Connect-Parameter über die Funktion Pool::SetConnectParams
 * übergeben wurden.
 */

/*!\var DBPool::Name
 * \brief Der Name des Pools, der über die Funktion Pool::SetName festgelegt werden kann.
 */

/*!\var DBPool::Hash
 * \brief Ein MD5-Hash über die Connect-Parameter, der bei Aufruf der Funktion Pool::SetConnectParams gebildet wird.
 */

/*!\var DBPool::Mutex
 * \brief Ein Muetx, der zur Gewährleistung der Threadsicherheit von allen Funktionen verwendet wird.
 */

/*!\var DBPool::Id
 * \brief Eine eindeutige Id, die bei Verwendung des Pools durch die Klasse PoolEx vergeben wird.
 */

/*!\var DBPool::LastCheck
 * \brief Wird von der Klasse PoolEx verwendet und enthält den Timestamp der letzten Überprüfung des Pools.
 */

/*!\var DBPool::Used
 * \brief Eine Liste, die alle gerade in Verwendung befindlichen Datenbank-Connects enthält.
 */

/*!\var DBPool::Free
 * \brief Eine Liste, die alle derzeit freien Datenbank-Connects enthält.
 */

/*!\var DBPool::Log
 * \brief Pointer auf eine Logklasse, der über die Funktion Pool::SetLogfile gesetzt wird. Enthält die Variable
 * einen Wert != 0 ist das Logging aktiviert.
 */

/*!\var DBPool::ConnectParam
 * \brief Enthält eine Kopie der Connect-Parameter, die über die Funktion Pool::SetConnectParams gesetzt werden.
 */

DBPool::DBPool()
{
	Id=-1;
	Min=0;
	Max=0;
	MinSpare=0;
	MaxSpare=0;
	Grow=1;
	IdleTimeout=300;
	UsedTimeout=600;
	KeepAlive=60;
	IsInit=false;
	Log=NULL;
	LastCheck=0;
	ConnectParamVersion=0;
}

DBPool::~DBPool()
{
	clearFreePool();
	clearUsedPool();
}

static String calcHash(const AssocArray& param)
{
	ByteArray buffer;
	param.exportBinary(buffer);
	return buffer.md5();
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
void DBPool::setConnectParams(const AssocArray& connect)
{
	if (Log) Log->print(ppl7::Logger::INFO, 4, "ppl7::db::DBPool", "setConnectParams", __FILE__, __LINE__,
		ppl7::ToString("Setze Connect Parameter für Pool id=%i, name=\"%s\"", Id, (const char*)Name));
	PoolMutex.lock();
	if (calcHash(connect) != calcHash(this->ConnectParam)) {
		// increment version and remove remaining connections from free pool
		ConnectParamVersion++;
		std::list<Database*>::iterator it;
		for (it=Free.begin();it != Free.end();++it) {
			delete(*it);
		}
		Free.clear();
	}
	this->ConnectParam=connect;
	IsInit=true;
	PoolMutex.unlock();
}

void DBPool::setOptions(const AssocArray& options)
{
	AssocArray::Iterator it;
	String Key, Value;
	options.reset(it);
	while (options.getNext(it, Key, Value)) {
		setOption(Key, Value);
	}
}

void DBPool::setOption(const String& Name, const String& Value)
{
	/*
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
		*/
}

void DBPool::setName(const String& Name)
{
	this->Name=Name;
}

void DBPool::setId(int id)
{
	this->Id=id;
}

void DBPool::setMinimumSize(int num)
{
	this->Min=num;
}

void DBPool::setMaximumSize(int num)
{
	this->Max=num;
}

void DBPool::setMinSpare(int num)
{
	this->MinSpare=num;
}

void DBPool::setMaxSpare(int num)
{
	this->MaxSpare=num;
}

void DBPool::setGrowth(int num)
{
	this->Grow=num;
}

void DBPool::setIdleTimeout(int seconds)
{
	this->IdleTimeout=seconds;
}

void DBPool::setUsedTimeout(int seconds)
{
	this->UsedTimeout=seconds;
}

void DBPool::setKeepAliveIntervall(int seconds)
{
	this->KeepAlive=seconds;
}

void DBPool::setLogger(Logger& logger)
{
	this->Log=&logger;
}

}	// EOF namespace db
}	// EOF namespace ppl7
