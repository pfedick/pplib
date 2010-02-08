/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.26 $
 * $Date: 2009/09/05 17:06:20 $
 * $Id: CDBPool.cpp,v 1.26 2009/09/05 17:06:20 patrick Exp $
 *
 *******************************************************************************
 * Copyright (c) 2008, Patrick Fedick <patrick@pfp.de>
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
#include "ppl6.h"

namespace ppl6 {

/*!\class CDBPool
 * \ingroup PPLGroupOldDatabases
 * \brief Klasse zur Verwaltung von Datenbank-Verbindungen
 *
 * \desc
 * Mit dieser Klasse können beliebig viele Connects zu einer bestimmten Datenbank innerhalb
 * eines Pools verwaltet werden.
 */


/** @name Konstruktor, Destruktor, Initialisierung
 */
//@{

CDBPool::CDBPool()
/*!\brief Konstruktor der Klasse
 *
 * \desc
 * Der Konstruktor setzt diverse Parameter auf ihren Default-Wert:
 *
 * - \b min=0: Gibt an, wieviele Connects mindestens im Pool enthalten sein sollen
 * - \b max=0: Gibt an, wieviele Connects maximal geöffnet werden dürfen. Default = uneingeschränkt
 * - \b minspare=0: Gibt an, wieviele Freie Connects minimal vorgehalten werden sollen
 * - \b maxspare=0: Gibt an, wieviele Freie Connects maximal vorgehalten werden sollen. Sin mehr Connects
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
}

CDBPool::~CDBPool()
/*!\brief Destruktor der Klasse
 *
 * \desc
 * Der Destruktor sorgt dafür, dass alle Datenbank-Connects sauber getrennt werden.
 */
{
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::CDBPool","~CDBPool",__FILE__,__LINE__,"Destruktor aufgerufen fuer CDBPool id=%i, name=\"%s\", loesche Pools",Id,(const char*)Name);
	if (Free.Num()) {
		if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPool","~CDBPool",__FILE__,__LINE__,"Loesche Free-Pool, %i Connects",Free.Num());
		Free.Clear(true);
		if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPool","~CDBPool",__FILE__,__LINE__,"Done");
	}
	if (Used.Num()) {
		if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPool","~CDBPool",__FILE__,__LINE__,"Loesche Used-Pool, %i Connects",Used.Num());
		if (Log) Log->Printf(ppl6::LOG::ERROR,1,"ppl6::CDBPool","~CDBPool",__FILE__,__LINE__,"Im Used Pool des Pools \"%s\", ID: %i sind noch %i Verbindungen!",(const char*)Name, Id,Used.Num());
		Used.Clear(true);
		if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPool","~CDBPool",__FILE__,__LINE__,"Done");
	}


}

int CDBPool::CompareNode(CTreeItem *item)
/*!\copydoc CTreeItem::CompareNode
 */
{
	if (Id<((CDBPool*)item)->Id) return 1;
	if (Id>((CDBPool*)item)->Id) return -1;
	return 0;
}

int CDBPool::CompareValue(void *value)
/*!\copydoc CTreeItem::CompareValue
 */
{
	if (Id<((int)(ppliptr)value)) return 1;
	if (Id>((int)(ppliptr)value)) return -1;
	return 0;
}


void CDBPool::SetLogfile(CLog *log)
{
	Mutex.Lock();
	Log=log;
	Mutex.Unlock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::CDBPool","SetLogfile",__FILE__,__LINE__,"Debugging fuer CDBPool id=%i, name=\"%s\" aktiviert",Id,(const char*)Name);
}

int CDBPool::Init(CAssocArray *connect, CAssocArray *options, const char *name)
/*!\brief Datenbank-Pool initialisieren
 *
 * \desc
 * Mit dieser Funktion werden die Connect-Parameter für die Datenbank festgelegt, sowie
 * einige zusätzliche Optionen.
 * \param[in] connect Die Connect-Parameter in einem Assoziativen Array, wie sie von der Funktion
 * ppl6::ConnectDatabase unterstützt werden.
 * \param[in] options Ein optionales Assoziatives Array mit zusätzlichen Optionen für die Verwaltung
 * des Pools. Dieses kann folgende Werte enthalten:
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
 * \param[in] name Legt einen Namen für diesen Pool fest (optional)
 * \returns Die Funktion liefert bei Erfolg true (1) zurück, sonst false (0).
 */
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPool","Init",__FILE__,__LINE__,"Initialisiere CDBPool id=%i, name=\"%s\"",Id,(name!=NULL?name:(const char*)Name));
	if (!connect) {
		SetError(194,"int CDBPool::Init(==> CAssocArray *connect <==, CAssocArray *options)");
		if (Log) Log->LogError("ppl6::CDBPool","Init",__FILE__,__LINE__);
		return 0;
	}
	CString NewHash;
	if (!CalcHash(&NewHash,connect)) {
		if (Log) Log->LogError("ppl6::CDBPool","Init",__FILE__,__LINE__);
		return 0;
	}
	Mutex.Lock();
	if (name) Name=name;
	// Falls sich die Connect-Parameter geändert haben, löschen wir alle noch vorhandenen
	// Parameter aus dem Freien Pool
	if (NewHash!=Hash && Hash.Len()>0) {
		if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::CDBPool","Init",__FILE__,__LINE__,"%s: Connect-Parameter haben sich geaendert, loesche Free-Pool",(const char*)Name);
		Free.Clear(true);
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
		Log->PrintArraySingleLine(ppl6::LOG::DEBUG,10,"ppl6::CDBPool","Init",__FILE__,__LINE__,&a,"Connect-Parameter");
	}
	Min=0;
	Max=0;
	MinSpare=0;
	MaxSpare=0;
	Grow=1;
	Timeout=300;
	KeepAlive=60;
	if (options) {
		const char *tmp;
		if ((tmp=options->Get("min"))) Min=ppl6::atoi(tmp);
		if ((tmp=options->Get("max"))) Max=ppl6::atoi(tmp);
		if ((tmp=options->Get("minspare"))) MinSpare=ppl6::atoi(tmp);
		if ((tmp=options->Get("maxspare"))) MaxSpare=ppl6::atoi(tmp);
		if ((tmp=options->Get("grow"))) Grow=ppl6::atoi(tmp);
		if ((tmp=options->Get("timeout"))) Timeout=ppl6::atoi(tmp);
		if ((tmp=options->Get("keepalive"))) KeepAlive=ppl6::atoi(tmp);
	}
	if (Log) {
		Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPool","Init",__FILE__,__LINE__,"Pool %i:%s: Intialisierung komplett",Id,(const char*)Name);
		Log->Printf(ppl6::LOG::DEBUG,10,"ppl6::CDBPool","Init",__FILE__,__LINE__,"min: %i, max: %i, minspare: %i, maxspare: %i, grow: %i",Min,Max,MinSpare,MaxSpare,Grow);
		Log->Printf(ppl6::LOG::DEBUG,10,"ppl6::CDBPool","Init",__FILE__,__LINE__,"KeepAlive: %i s, Timeout: %i s",KeepAlive,Timeout);
	}
	IsInit=true;
	Mutex.Unlock();
	return 1;
}

//@}

int CDBPool::CalcHash(CString *hash, CAssocArray *param)
/*!\brief Berechnet Hash aus Connect-Parametern
 * \desc
 * Diese Funktion berechnet aus den Connect-Parametern einen eindeutigen
 * MD5-Hash.
 *
 * \param[out] hash Pointer auf den Zielstring, in dem der Hash gespeichert werden
 * soll
 * \param[in] param Pointer auf ein Assoziatives Array, in dem die Connect-Parameter
 * enthalten sind.
 *
 * \returns Bei Erfolg liefert die Funktion true (1) zurück, sonst false(0). Ein Fehler
 * kann auftreten, wenn einer der Parameter NULL ist oder in den Connect-Parametern
 * wichtige Informationen fehlen.
 *
 * \remarks
 * Die Funktion verwendet folgende Keys, um den MD5-Hash zu erstellen:
 * - type
 * - host
 * - port
 * - user
 * - password
 * - dbname
 * - charset
 */
{
	if (!hash) {
		SetError(194,"int CDBPool::CalcHash(==>CString *hash <==, CAssocArray *param)");
		return 0;
	}
	if (!param) {
		SetError(194,"int CDBPool::CalcHash(CString *hash, ==> CAssocArray *param <==)");
		return 0;
	}
	hash->Clear();
	hash->Concatf("type:%s|",param->Get("type"));
	hash->Concatf("host:%s|",param->Get("host"));
	hash->Concatf("port:%s|",param->Get("port"));
	hash->Concatf("user:%s|",param->Get("user"));
	hash->Concatf("password:%s|",param->Get("password"));
	hash->Concatf("dbname:%s|",param->Get("dbname"));
	hash->Concatf("charset:%s|",param->Get("charset"));
	hash->MD5();
	return 1;
}

CDBWrapper *CDBPool::New()
/*!\brief Interne Funktion zum erstellen eines neuen Datenbank-Connects
 *
 * \desc
 * Diese Funktion wird intern von CDBPool::Get und CDBPool::CheckPool aufgerufen, um eine
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
{
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::CDBPool","New",__FILE__,__LINE__,"%i:%s: Neuen Connect erstellen",Id,(const char*)Name);
	// Einen neuen Connect gibts nur, wenn Max==0 ist
	if (Max>0) {
		// Oder die Anzahl Connects insgesammt kleiner als Max sind
		if (Used.Num()+Free.Num()>=Max) {
			SetError(442,"%i",Max);
			return NULL;
		}
	}
	CDBWrapper *p=ConnectDatabase(&ConnectParam);
	if (p) {
		p->pool=this;
		p->Hash=Hash;
	}
	return p;
}



/** @name Verbindungen holen/zurückgeben
 */
//@{

CDBWrapper *CDBPool::Get(bool wait, int ms)
/*!\brief Datenbank-Connect holen
 *
 * \desc
 * Mit dieser Funktion wird eine Verbindung aus dem Datenbank-Pool geholt oder eine neue erstellt.
 *
 * \param[in] wait Optionales Flag, das angibt, ob die Funktion warten soll, wenn das Maximum an
 * Connects für diesen Pool bereits erreicht ist.
 * \param[in] ms Ein optionaler Wert, der in Kombination mit dem \p wait-Flag angibt, wieviele
 * Millisekunden gewartet werden soll, bevor mit einem Timeout abgebrochen wird.
 * \returns Die Funktion gibt entweder ein gültiges Handle für die Datenbank zurück, oder im
 * Fehlerfall NULL.
 *
 * \remarks
 * Die Funktion prüft immer, ob die Datenbankverbindung noch funktionsfähig ist, indem ein
 * Ping geschickt wird. Es ist daher sichergestellt, dass nur funktionierende Verbindungen
 * zurückgegeben werden.
 */
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPool","Get",__FILE__,__LINE__,"%i:%s: Verbindung aus dem Pool holen, wait=%i, ms=%i",Id,(const char*)Name,(int)wait,ms);
	if (!IsInit) {
		SetError(443);
		if (Log) Log->LogError("ppl6::CDBPool","Get",__FILE__,__LINE__);
		return NULL;
	}
	CDBWrapper *p;
	Mutex.Lock();
	// Zuerst schauen wir mal, ob wir noch etwas im Pool haben
	Free.Reset();
	while ((p=(CDBWrapper *)Free.GetNext())) {
		if (p->poollock==false) {
			// Ist die Datenbank auch noch benutzbar?
			p->poollock=true;
			Mutex.Unlock();
			if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPool","Get",__FILE__,__LINE__,"Freie Verbindung im Pool gefunden, prüfe Funktionalität (Ping)");
			if (!p->Ping()) {				// Datenbank noch verbunden?
				if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPool","Get",__FILE__,__LINE__,"Ping fehlgeschlagen, trenne Verbindung");
				Mutex.Lock();
				delete p;
				Free.Reset();
				continue;
			}
			Mutex.Lock();
			p->poollock=false;
			Free.Delete(p);
			Used.Add(p);
			Mutex.Unlock();
			if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPool","Get",__FILE__,__LINE__,"Verwende Verbindung aus dem Pool");
			return p;
		}
	}
	if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPool","Get",__FILE__,__LINE__,"Keine Verbindung im Pool, erstelle neue Verbindung");
	// Wir brauchen eine neue Verbindung
	double start=ppl6::getmicrotime();
	double diff=(double)ms/1000;
	while (1) {
		p=New();
		if (p) break;
		if (p==NULL && GetErrorCode()!=442) {
			Mutex.Unlock();
			if (Log) Log->LogError("ppl6::CDBPool","Get",__FILE__,__LINE__);
			return NULL;
		}
		if (ms) {
			if (start+diff>=ppl6::getmicrotime()) {
				SetError(442,"Timeout");
				Mutex.Unlock();
				if (Log) Log->LogError("ppl6::CDBPool","Get",__FILE__,__LINE__);
				return NULL;
			}
		}
		ppl6::MSleep(10);
	}
	Used.Add(p);
	Mutex.Unlock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPool","Get",__FILE__,__LINE__,"Neue Verbindung erstellt");
	return p;
}

/*!\brief Datenbankverbindung in den Pool zurückgeben
 *
 * \desc
 * Mit dieser Funktion wird eine zuvor mit CDBPool::Get geholte Verbindung wieder
 * zurück in den Pool gestellt.
 *
 * \param[in] db Pointer auf eine Datenbankverbindung. Diese muss zuvor mit der
 * Funktion CDBPool::Get geholt worden sein.
 * \returns Konnte die Verbindung erfolgreich in den Pool zurückgestellt werden,
 * liefert die Funktion true (1) zurück, sonst false (0).
 * \remarks Wurde zwischenzeitlich die Init-Funktion mit veränderten Connect-Parametern
 * aufgerufen, wird die Verbindung nicht zurück in den Pool gestellt, sondern gelöscht.
 */
int CDBPool::Release(CDBWrapper *db)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPool","Release",__FILE__,__LINE__,"%i:%s: Verbindung in den Pool zurückstellen",Id,(const char*)Name);
	if (!IsInit) {
		SetError(443);
		if (Log) Log->LogError("ppl6::CDBPool","Release",__FILE__,__LINE__);
		return 0;
	}
	if (!db) {
		if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPool","Release",__FILE__,__LINE__,"Parameter ist NULL, nichts zu tun");
		return 1;
	}
	if (db->pool!=this) {
		if (Log) Log->LogError("ppl6::CDBPool","Release",__FILE__,__LINE__);
		SetError(439);
		return 0;
	}
	if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPool","Release",__FILE__,__LINE__,"Pruefe, ob Datenbank noch antwortet (Ping)");
	if (!db->Ping()) {				// Datenbank noch verbunden?
		Mutex.Lock();
		Used.Delete(db);
		delete db;
		Mutex.Unlock();
		if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPool","Release",__FILE__,__LINE__,"Verbindung war unbrauchbar und wurde geloescht");
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
		if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPool","Release",__FILE__,__LINE__,"Datenbank-Parameter haben sich geaendert, Verbindung wird geloescht");
	} else {
		Free.Add(db);
		db->poollock=false;
		if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPool","Release",__FILE__,__LINE__,"Verbindung wurde zurueck in den freien Pool gestellt");
	}
	Mutex.Unlock();
	return 1;
}

/*!\brief Datenbankverbindung löschen
 * \desc
 * Mit dieser Funktion wird eine zuvor mit CDBPool::Get geholte Verbindung getrennt
 * und aus dem Pool gelöscht.
 *
 * \param[in] db Pointer auf eine Datenbankverbindung. Diese muss zuvor mit der
 * Funktion CDBPool::Get geholt worden sein.
 * \returns Konnte die Verbindung erfolgreich gelöscht werden, liefert die Funktion
 * true (1) zurück, sonst false (0). Ein Fehler kann nur auftreten, wenn die Verbindung
 * nicht aus dem gleichen Pool stammt.
 */
int CDBPool::Destroy(CDBWrapper *db)
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPool","Destroy",__FILE__,__LINE__,"%i:%s: Verbindung löschen",Id,(const char*)Name);
	if (!IsInit) {
		SetError(443);
		if (Log) Log->LogError("ppl6::CDBPool","Destroy",__FILE__,__LINE__);
		return 0;
	}
	if (db) {
		if (db->pool!=this) {
			SetError(439);
			if (Log) Log->LogError("ppl6::CDBPool","Destroy",__FILE__,__LINE__);
			return 0;
		}
		Mutex.Lock();
		Used.Delete(db);
		delete db;
		Mutex.Unlock();
		if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPool","Destroy",__FILE__,__LINE__,"Verbindung wurde gelöscht");
	} else {
		if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPool","Destroy",__FILE__,__LINE__,"Parameter ist NULL, nichts zu tun");
	}
	return 1;
}

//@}

/** @name Verschiedenes
 */
//@{

int CDBPool::ClearUsedPool(bool destroy)
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
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPool","ClearUsedPool",__FILE__,__LINE__,"Pool: %i:%s, %i Connects: Pool wird geleert",Id,(const char*)Name, Used.Num());
	Mutex.Lock();
	if (Used.Num()>0 && Log!=NULL) Log->Printf(ppl6::LOG::ERROR,1,"ppl6::CDBPool","ClearUsedPool",__FILE__,__LINE__,"Im UsedPool \"%i:%s\" waren noch %i Connects, Programm könnte crashen!",Id,(const char*)Name,Used.Num());
	Used.Clear(destroy);
	Mutex.Unlock();
	return 1;
}

int CDBPool::ClearFreePool(bool destroy)
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
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPool","ClearFreePool",__FILE__,__LINE__,"Pool: %i:%s, %i Connects: Pool wird geleert",Id,(const char*)Name, Free.Num());
	Mutex.Lock();
	Free.Clear(destroy);
	Mutex.Unlock();
	return 1;
}


int CDBPool::GetStatus(CAssocArray *status)
{
	if (Log) Log->Printf(ppl6::LOG::DEBUG,6,"ppl6::CDBPool","GetStatus",__FILE__,__LINE__,"%i:%s",Id,(const char*)Name);
	if (!IsInit) {
		SetError(443);
		if (Log) Log->LogError("ppl6::CDBPool","GetStatus",__FILE__,__LINE__);
		return 0;
	}
	CAssocArray item;
	CAssocArray *a;
	if (!status) a=new CAssocArray;
	else a=status;
	Mutex.Lock();
	//a->Set("pools",&Databases,true);
	if (Id>=0) a->Setf("Id","%i",Id);
	if (Name.Len()>0) a->Setf("Name","%s",(const char*)Name);
	if (Hash.Len()>0) a->Setf("Hash","%s",(const char*)Hash);
	a->Setf("PoolUsed","%i",Used.Num());
	a->Setf("PoolFree","%i",Free.Num());
	a->Set("Connect",&ConnectParam);
	a->Setf("Options/Min","%i",Min);
	a->Setf("Options/Max","%i",Max);
	a->Setf("Options/MinSpare","%i",MinSpare);
	a->Setf("Options/MaxSpare","%i",MaxSpare);
	a->Setf("Options/Grow","%i",Grow);
	a->Setf("Options/Timeout","%i",Timeout);
	a->Setf("Options/KeepAlive","%i",KeepAlive);
	CDBWrapper *p;
	Used.Reset();
	while ((p=(CDBWrapper *)Used.GetNext())) {
		item.Clear();
		item.Set("status","used");
		if (p->Hash.Len()>0) item.Set("hash",p->Hash);
		item.Setf("lastping","%i",p->lastping);
		item.Setf("lastuse","%i",p->lastuse);
		a->Set("Pool/[]",&item,true);
	}
	Free.Reset();
	while ((p=(CDBWrapper *)Free.GetNext())) {
		item.Clear();
		item.Set("status","free");
		if (p->Hash.Len()>0) item.Set("hash",p->Hash);
		item.Setf("lastping","%i",p->lastping);
		item.Setf("lastuse","%i",p->lastuse);
		a->Set("Pool/[]",&item,true);
	}


	Mutex.Unlock();
	if (!status) {
		printf("=========================================================\n");
		printf("Status CDBPool:\n");
		a->List();
		printf("=========================================================\n");
		if (Log) Log->PrintArray(ppl6::LOG::DEBUG,10,"ppl6::CDBPool","GetStatus",__FILE__,__LINE__,a,"Status des Datenbank-Pools \"%i:%s\"",Id,(const char*)Name);
		delete a;
	}
	return 1;
}

/*!\brief Datenbank-Pools überprüfen
 *
 * \desc
 * Diese Funktion sollte regelmäßig innerhalb eines Überwachungsthreads aufgerufen werden,
 * um die im Pool vorhandenen Verbindungen zu überprüfen und zu expiren. Die Funktion schickt
 * regelmäßig an alle unbenutzen Verbindungen einen Ping (KeepAlive) und
 * löscht Verbindungen, die nicht mehr reagieren.
 *
 * Bei sehr großen Pools wird empfohlen diese Funktion innerhalb eines eigenen Threads
 * aufzurufen, um andere Funktionen nicht zu blockieren.
 */
void CDBPool::CheckPool()
{
	if (Log) Log->Printf(ppl6::LOG::DEBUG,6,"ppl6::CDBPool","CheckPool",__FILE__,__LINE__,"%i:%s, Connects: %i, Free: %i, Used: %i",Id,(const char*)Name, Free.Num()+Used.Num(),Free.Num(), Used.Num());
	ppluint64 now;
	CDBWrapper *p;
	// Timeout
	if (Log) Log->Printf(ppl6::LOG::DEBUG,9,"ppl6::CDBPool","CheckPool",__FILE__,__LINE__,"Prüfe auf Timeout");

	now=ppl6::GetTime();
	Mutex.Lock();
	Free.Reset();
	while ((p=(CDBWrapper*)Free.GetNext())) {
		if (Log) Log->Printf(ppl6::LOG::DEBUG,10,"ppl6::CDBPool","CheckPool",__FILE__,__LINE__,"Server: %s, lastuse: %llu, now: %llu, Timeout: %llu, locked: %i\n",(const char*)Name, p->lastuse,now, p->lastuse+Timeout,(int)p->poollock);
		if (p->poollock==false && p->lastuse+Timeout<now) {
			if (Log) Log->Printf(ppl6::LOG::DEBUG,7,"ppl6::CDBPool","CheckPool",__FILE__,__LINE__,"Timeout erreicht, lösche Verbindung\n");
			delete p;
			Free.Reset();
		}
	}

	// Keepalive
	if (Log) Log->Printf(ppl6::LOG::DEBUG,9,"ppl6::CDBPool","CheckPool",__FILE__,__LINE__,"Prüfe auf KeepAlive");
	Free.Reset();
	while ((p=(CDBWrapper*)Free.GetNext())) {
		now=ppl6::GetTime();
		if (p->poollock==false && p->lastping+KeepAlive<now) {
			p->poollock=true;
			Mutex.Unlock();
			if (Log) Log->Printf(ppl6::LOG::DEBUG,10,"ppl6::CDBPool","CheckPool",__FILE__,__LINE__,"Schicke Ping\n");
			//printf ("Ping: %s\n",(char*)p->ServerName);
			if (p->Ping()) {
				//printf ("Ping ok: %s\n",(char*)p->ServerName);
				if (Log) Log->Printf(ppl6::LOG::DEBUG,10,"ppl6::CDBPool","CheckPool",__FILE__,__LINE__,"Ping ok\n");
				p->lastping=ppl6::GetTime();
				Mutex.Lock();
				p->poollock=false;
			} else {				// Connect ist weg, wir löschen ihn aus dem Pool
				if (Log) Log->Printf(ppl6::LOG::DEBUG,7,"ppl6::CDBPool","CheckPool",__FILE__,__LINE__,"Ping fehlgeschlagen, lösche Verbindung\n");
				//printf ("Ping failed: %s\n",(char*)p->ServerName);
				Mutex.Lock();
				delete p;
			}
			break;
		}
	}
	Mutex.Unlock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,8,"ppl6::CDBPool","CheckPool",__FILE__,__LINE__,"CheckPool fertig");
}

//@}


class CDBPoolCheck : public CThread
{
	private:
	public:
		CDBPoolEx	*pool;
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

/*!\class CDBPoolEx
 * \brief Datenbank-Pool für multiple Datenbanken
 *
 * \ingroup PPLGroupOldDatabases
 *
 * \desc
 * Mit dieser Klasse können multiple Datenbank-Pools zu verschiedenen Datenbanken
 * verwaltet werden.
 *
 */

CDBPoolEx::CDBPoolEx()
/*!\brief Konstruktor der Klasse
 *
 * \desc Der Destruktor initialisiert verschiedene Variablen, wie Timeout, KeepAlive. Diese
 * können mit folgenden Funktionen überschrieben werden:
 * - CDBPoolEx::SetDefaultTimeout
 * - CDBPoolEx::SetDefaultKeepAlive
 * - CDBPoolEx::SetDefaultMin
 * - CDBPoolEx::SetDefaultMax
 * - CDBPoolEx::SetDefaultMinSpare
 * - CDBPoolEx::SetDefaultMaxSpare
 * - CDBPoolEx::SetDefaultGrow
 */
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

CDBPoolEx::~CDBPoolEx()
/*!\brief Destruktor der Klasse
 */
{
	if (Log) Log->Printf(ppl6::LOG::INFO,2,"ppl6::CDBPoolEx","~CDBPoolEx",__FILE__,__LINE__,"Destruktor aufgerufen");
	if (CheckThread) {
		if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::CDBPoolEx","~CDBPoolEx",__FILE__,__LINE__,"Stoppe Überwachungsthread");
		CheckThread->ThreadSignalStop();
		CheckThread->ThreadStop();
		delete CheckThread;
	}
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::CDBPoolEx","~CDBPoolEx",__FILE__,__LINE__,"Lösche vorhandene Datenbanken");
	Databases.Clear(true);
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::CDBPoolEx","~CDBPoolEx",__FILE__,__LINE__,"Done");
}

void CDBPoolEx::ClearPools(bool free, bool used)
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
{
	if (free) {
		if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::CDBPoolEx","ClearPools",__FILE__,__LINE__,"Lösche freie Verbindungen in allen Pools");
		Mutex.Lock();
		Databases.Reset();
		CDBPool *p;
		while ((p=(CDBPool*)Databases.GetNext())) {
			p->ClearFreePool(true);
		}
		Mutex.Unlock();
		if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::CDBPoolEx","ClearPools",__FILE__,__LINE__,"Done");
	}
	if (used) {
		if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::CDBPoolEx","ClearPools",__FILE__,__LINE__,"Lösche benutzte Verbindungen in allen Pools");
		Mutex.Lock();
		Databases.Reset();
		CDBPool *p;
		while ((p=(CDBPool*)Databases.GetNext())) {
			p->ClearUsedPool(true);
		}
		Mutex.Unlock();
		if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::CDBPoolEx","ClearPools",__FILE__,__LINE__,"Done");
	}


}

CDBPool *CDBPoolEx::FindPoolByName(const char *name)
{
	Databases.Reset();
	CDBPool *p;
	while ((p=(CDBPool *)Databases.GetNext())) {
		if (p->Name==name) return p;
	}
	SetError(440,"%s",name);
	return NULL;
}

void CDBPoolEx::SetLogfile(CLog *log)
{
	Mutex.Lock();
	Log=log;
	Mutex.Unlock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::CDBPoolEx","SetLogfile",__FILE__,__LINE__,"Debugging fuer CDBPoolEx aktiviert");
}


int CDBPoolEx::DefineDatabase(int id, const char *name, CAssocArray *connect, CAssocArray *options)
/*!\brief Neuen Datenbank-Pool definieren
 *
 * \desc
 * Mit dieser Funktion wird ein neuer Datenbank-Pool definiert. Jeder Datenbank muss dabei
 * eine eindeutige ID und ein eindeutiger Name zugeordnet werden, sowie die Connect-
 * Parameter und optionale zusätzliche Optionen.
 * \param[in] id Eindeutige ID der Datenbank. Ist eine ID bereits vorhanden, werden die
 * alten Daten überschrieben. Die ID "0" hat eine besondere Bedeutung. Sie definiert
 * quasi eine Default-Datenbank, die immer dann verwendet wird, wnn die Funktion
 * CDBPool::Get ohne Parameter oder mit NULL aufgerufen wird.
 * \param[in] name Ein eindeutiger Name für diese Datenbank.
 * \param[in] connect Die Connect-Parameter in einem Assoziativen Array, wie sie von der Funktion
 * ppl6::ConnectDatabase unterstützt werden.
 * \param[in] options Ein optionales Assoziatives Array mit zusätzlichen Optionen für die Verwaltung
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
 * \returns Die Funktion liefert bei Erfolg true (1) zurück, sonst false (0).
 * \remarks
 * Die Funktion kann im laufenden Betrieb mehrfach für die gleiche Datenbank aufgerufen werden.
 * Die alten Daten werden dabei überschrieben. Sofern sich die Connect-Parameter ändern, werden
 * alle Connects im Free-Pool sofort gelöscht, Connects, die in Benutzung sind, werden nach deren
 * Freigabe gelöscht. Neu angeforderte Connects werden mit den neuen Parametern erstellt.
 */
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPoolEx","DefineDatabase",__FILE__,__LINE__,"Definiere Datenbank ID=%i, Name=%s",id,name);
	if (!name) {
		SetError(194,"");
		if (Log) Log->LogError("ppl6::CDBPoolEx","DefineDatabase",__FILE__,__LINE__);
		return 0;
	}

	if (!connect) {
		SetError(194,"");
		if (Log) Log->LogError("ppl6::CDBPoolEx","DefineDatabase",__FILE__,__LINE__);
		return 0;
	}
	Mutex.Lock();
	CDBPool *dupname=NULL;
	Databases.Reset();
	while ((dupname=(CDBPool*)Databases.GetNext())) {
		if (dupname->Name==name) break;
	}

	CDBPool *pool=(CDBPool *)Databases.Find((void*)id);
	// Wenn es diese Datenbank noch nicht gibt, legen wir sie an.
	if (!pool) {
		// Allerdings nur, wenn der Name noch nicht existiert
		if (dupname) {
			SetError(441,"Name: %s, Id: %i",name,dupname->Id);
			Mutex.Unlock();
			if (Log) Log->LogError("ppl6::CDBPoolEx","DefineDatabase",__FILE__,__LINE__);
			return 0;
		}
		pool=new CDBPool;
		if (!pool) {
			Mutex.Unlock();
			SetError(2);
			if (Log) Log->LogError("ppl6::CDBPoolEx","DefineDatabase",__FILE__,__LINE__);
			return 0;
		}
		pool->Min=DefaultMin;
		pool->Max=DefaultMax;
		pool->MinSpare=DefaultMinSpare;
		pool->MaxSpare=DefaultMaxSpare;
		pool->Grow=DefaultGrow;
		pool->Timeout=DefaultTimeout;
		pool->KeepAlive=DefaultKeepAlive;
		pool->Id=id;
		if (!pool->Init(connect,options)) {
			PushError();
			delete pool;
			Mutex.Unlock();
			PopError();
			if (Log) Log->LogError("ppl6::CDBPoolEx","DefineDatabase",__FILE__,__LINE__);
			return 0;
		}
		if (!Databases.Add(pool)) {
			PushError();
			delete pool;
			Mutex.Unlock();
			PopError();
			if (Log) Log->LogError("ppl6::CDBPoolEx","DefineDatabase",__FILE__,__LINE__);
			return 0;
		}
		pool->SetLogfile(Log);

	} else {
		// Der Name darf noch nicht bei einer anderen Datenbank verwendet worden sein.
		if (dupname!=NULL && dupname!=pool) {
			SetError(441,"Name: %s, Id: %i",name,dupname->Id);
			Mutex.Unlock();
			if (Log) Log->LogError("ppl6::CDBPoolEx","DefineDatabase",__FILE__,__LINE__);
			return 0;
		}
	}
	pool->Name=name;
	if (!pool->Init(connect,options)) {
		Mutex.Unlock();
		if (Log) Log->LogError("ppl6::CDBPoolEx","DefineDatabase",__FILE__,__LINE__);
		return 0;
	}
	Mutex.Unlock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::CDBPoolEx","DefineDatabase",__FILE__,__LINE__,"Datenbankpool wurde erfolgreich erstellt");
	return 1;
}

int CDBPoolEx::DeleteDatabase(int id)
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
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPoolEx","DeleteDatabase",__FILE__,__LINE__,"Lösche Datenbank ID=%i",id);
	Mutex.Lock();
	CDBPool *pool=(CDBPool *)Databases.Find((void*)id);
	if (!pool) {
		Mutex.Unlock();
		SetError(440,"%i",id);
		if (Log) Log->LogError("ppl6::CDBPoolEx","DeleteDatabase",__FILE__,__LINE__);
		return 0;
	}
	delete pool;
	Mutex.Unlock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,4,"ppl6::CDBPoolEx","DeleteDatabase",__FILE__,__LINE__,"Datenbankpool wurde gelöscht");
	return 1;
}


int CDBPoolEx::SetDefaultTimeout(int seconds)
/*!\brief Default Timeout verändern
 *
 * Mit dieser Funktion kann der Timeout für unbenutzte Verbindungen im Pool verändert werden.
 * Die Funktion CDBPool::CheckPool prüft bei regelmäßigem Aufruf innerhalb eines Wartungsthreads,
 * ob der Timeout erreicht ist und löscht die Verbindung, falls dies der Fall ist.
 *
 * \param[in] seconds Anzahl Sekunden, nach denen der Timeout auftreten soll.
 * \returns Die Funktion liefert immer true (1) zurück.
 */
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPoolEx","SetDefaultTimeout",__FILE__,__LINE__,"Timeout=%i",seconds);
	DefaultTimeout=seconds;
	return 1;
}

int CDBPoolEx::SetDefaultKeepAlive(int seconds)
/*!\brief Default KeepAlive verändern
 *
 * Um die Verbindung zur Datenbank aufrecht zu erhalten, schickt die Funktion CDBPool::CheckPool
 * bei regelmäßigem Aufruf innerhalb eines Wartungsthreads einen Ping an die Datenbank. Besteht die
 * Verbindung noch, wird sie weiter aufrecht erhalten, war der Ping nicht erfolgreich, wird sie
 * gelöscht.
 *
 * \param[in] seconds Anzahl Sekunden, nach denen ein KeepAlive auftreten soll.
 * \returns Die Funktion liefert immer true (1) zurück.
 */
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPoolEx","SetDefaultKeepAlive",__FILE__,__LINE__,"KeepAlive=%i",seconds);
	DefaultKeepAlive=seconds;
	return 1;
}

int CDBPoolEx::SetDefaultMin(int nr)
/*!\brief Default Min verändern
 *
 * Legt den Default für die minimale Anzahl Connects zur Datenbank fest.
 *
 * \param[in] nr Anzahl Connects
 * \returns Die Funktion liefert immer true (1) zurück.
 */
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPoolEx","SetDefaultMin",__FILE__,__LINE__,"Min=%i",nr);
	DefaultMin=nr;
	return 1;
}

int CDBPoolEx::SetDefaultMax(int nr)
/*!\brief Default Max verändern
 *
 * Legt den Default für die maximale Anzahl Connects zur Datenbank fest.
 *
 * \param[in] nr Anzahl Connects. Ein Wert von 0 bedeutet "keine Limitierung".
 * \returns Die Funktion liefert immer true (1) zurück.
 */
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPoolEx","SetDefaultMax",__FILE__,__LINE__,"Max=%i",nr);
	DefaultMax=nr;
	return 1;
}

int CDBPoolEx::SetDefaultMinSpare(int nr)
/*!\brief Default MinSpare verändern
 *
 * Legt den Default für die minimale Anzahl freier Connects zur Datenbank fest.
 *
 * \param[in] nr Anzahl Connects
 * \returns Die Funktion liefert immer true (1) zurück.
 */
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPoolEx","SetDefaultMinSpare",__FILE__,__LINE__,"MinSpare=%i",nr);
	DefaultMinSpare=nr;
	return 1;
}

int CDBPoolEx::SetDefaultMaxSpare(int nr)
/*!\brief Default MinSpare verändern
 *
 * Legt den Default für die maximale Anzahl freier Connects zur Datenbank fest.
 *
 * \param[in] nr Anzahl Connects
 * \returns Die Funktion liefert immer true (1) zurück.
 */
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPoolEx","SetDefaultMaxSpare",__FILE__,__LINE__,"MaxSpare=%i",nr);
	DefaultMaxSpare=nr;
	return 1;
}

int CDBPoolEx::SetDefaultGrow(int nr)
/*!\brief Default Grow verändern
 *
 * Legt fest, wieviele Datenbank-Connects gleichzeitig aufgebaut werden sollen, wenn
 * ein neuer Connect angefordert wird, aber keine freien Connects im Pool sind, oder wenn
 * MinSpare unterschritten wird.
 *
 * \param[in] nr Anzahl Connects
 * \returns Die Funktion liefert immer true (1) zurück.
 */
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPoolEx","SetDefaultGrow",__FILE__,__LINE__,"Grow=%i",nr);
	DefaultGrow=nr;
	return 1;
}



CDBWrapper *CDBPoolEx::Get(CString *name, bool wait, int ms)
/*!\brief Datenbank-Verbindung anhand ihres Namens aus dem Pool holen
 *
 * \desc
 * Die Funktion sucht zuerst den passenden Pool und holt dort entweder einen freien Connect heraus
 * oder erstellt einen neuen Connect. Dazu muss die entsprechenden Datenbank vorher mit
 * CDBPoolEx::DefineDatabase definiert worden sein.
 *
 * \param[in] name Pointer auf den Namen der Datenbank-Verbindung. Mit
 * CDBPoolEx::DefineDatabase muss zuvor eine passende Datenbank definiert worden sein.
 * \param[in] wait Optionales Flag, das angibt, ob die Funktion warten soll, wenn das Maximum an
 * Connects für diesen Pool bereits erreicht ist.
 * \param[in] ms Ein optionaler Wert, der in Kombination mit dem \p wait-Flag angibt, wieviele
 * Millisekunden gewartet werden soll, bevor mit einem Timeout abgebrochen wird.
 *
 * \returns Die Funktion liefert einen Pointer auf die Datenbank-Verbindung zurück
 * oder NULL im Fehlerfall.
 * \remarks Die Verbindung muss nach Gebrauch entweder mit der Funktion CDBPoolEx::Release
 * zurück in den Pool gegeben oder mit CDBPoolEx::Destroy gelöscht werden. Falls kein
 * Wartungsthread läuft, der die Funktion CheckPool aufruft, kann das Handle auch
 * einfach mit "delete" gelöscht werden, es wird jedoch empfohlen die Destroy-Funktion
 * zu verwenden.
 *
 */
{
	if (!name) return Get(0);
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPoolEx","Get",__FILE__,__LINE__,"Suche Pool für Datenbank \"%s\"",name->GetPtr());
	Mutex.Lock();
	CDBPool *pool=FindPoolByName((const char*)name->GetPtr());
	if (!pool) {
		Mutex.Unlock();
		if (Log) Log->LogError("ppl6::CDBPoolEx","Get",__FILE__,__LINE__);
		return 0;
	}
	CDBWrapper *db=pool->Get(wait,ms);
	Mutex.Unlock();
	if (db) {
		db->poolex=this;
		if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPoolEx","Get",__FILE__,__LINE__,"Datenbank gefunden");
	} else {
		PushError();
		if (Log) Log->Printf(ppl6::LOG::ERROR,4,"ppl6::CDBPoolEx","Get",__FILE__,__LINE__,"Fehlschlag");
		PopError();
	}
	return db;
}

CDBWrapper *CDBPoolEx::Get(const char *name, bool wait, int ms)
/*!\brief Datenbank-Verbindung anhand ihres Namen aus dem Pool holen
 *
 * \copydoc CDBWrapper *CDBPool::Get(CString *name)
 */
{
	if (!name) return Get(0);
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPoolEx","Get",__FILE__,__LINE__,"Suche Pool für Datenbank \"%s\"",name);
	Mutex.Lock();
	CDBPool *pool=FindPoolByName(name);
	if (!pool) {
		Mutex.Unlock();
		if (Log) Log->LogError("ppl6::CDBPoolEx","Get",__FILE__,__LINE__);
		return 0;
	}
	CDBWrapper *db=pool->Get(wait,ms);
	Mutex.Unlock();
	if (db) {
		db->poolex=this;
		if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPoolEx","Get",__FILE__,__LINE__,"Datenbank gefunden");
	} else {
		if (Log) Log->Printf(ppl6::LOG::ERROR,4,"ppl6::CDBPoolEx","Get",__FILE__,__LINE__,"Fehlschlag");
	}

	return db;
}

CDBWrapper *CDBPoolEx::Get(int id, bool wait, int ms)
/*!\brief Datenbank-Verbindung anhand ihrer ID aus dem Pool holen
 *
 * \desc
 * Die Funktion sucht zuerst den passenden Pool und holt dort entweder einen freien Connect heraus
 * oder erstellt einen neuen Connect. Dazu muss die entsprechenden Datenbank vorher mit
 * CDBPoolEx::DefineDatabase definiert worden sein.
 *
 * \param[in] id ID der Datenbank, die mit CDBPool::DefineDatabase definiert wurde.
 * \param[in] wait Optionales Flag, das angibt, ob die Funktion warten soll, wenn das Maximum an
 * Connects für diesen Pool bereits erreicht ist.
 * \param[in] ms Ein optionaler Wert, der in Kombination mit dem \p wait-Flag angibt, wieviele
 * Millisekunden gewartet werden soll, bevor mit einem Timeout abgebrochen wird.
 *
 * \returns Die Funktion liefert einen Pointer auf die Datenbank-Verbindung zurück
 * oder NULL im Fehlerfall.
 * \remarks Die Verbindung muss nach Gebrauch entweder mit der Funktion CDBPoolEx::Release
 * zurück in den Pool gegeben oder mit CDBPoolEx::Destroy gelöscht werden. Falls kein
 * Wartungsthread läuft, der die Funktion CheckPool aufruft, kann das Handle auch
 * einfach mit "delete" gelöscht werden, es wird jedoch empfohlen die Destroy-Funktion
 * zu verwenden.
 */
{
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPoolEx","Get",__FILE__,__LINE__,"Suche Pool für Datenbank ID \"%i\"",id);
	Mutex.Lock();
	CDBPool *pool=(CDBPool *)Databases.Find((void*)id);
	if (!pool) {
		SetError(440,"%i",id);
		Mutex.Unlock();
		if (Log) Log->LogError("ppl6::CDBPoolEx","Get",__FILE__,__LINE__);
		return NULL;
	}
	CDBWrapper *db=pool->Get(wait,ms);
	Mutex.Unlock();
	if (db) {
		db->poolex=this;
		if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPoolEx","Get",__FILE__,__LINE__,"Datenbank gefunden");
	} else {
		if (Log) Log->Printf(ppl6::LOG::ERROR,4,"ppl6::CDBPoolEx","Get",__FILE__,__LINE__,"Fehlschlag");
	}
	return db;
}


/*!\brief Datenbankverbindung in den Pool zurückgeben
 *
 * \desc
 * Mit dieser Funktion wird eine zuvor mit CDBPool::Get geholte Verbindung wieder
 * zurück in den Pool gestellt.
 *
 * \param[in] db Pointer auf eine Datenbankverbindung. Diese muss zuvor mit der
 * Funktion CDBPool::Get geholt worden sein.
 * \returns Konnte die Verbindung erfolgreich in den Pool zurückgestellt werden,
 * liefert die Funktion true (1) zurück, sonst false (0).
 */
int CDBPoolEx::Release(CDBWrapper *db)
{
	if (!db) return 1;
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPoolEx","Release",__FILE__,__LINE__,"Gebe Datebank frei");
	if (db->poolex!=this) {
		SetError(439);
		if (Log) Log->LogError("ppl6::CDBPoolEx","Release",__FILE__,__LINE__);
		return 0;
	}
	if (!db->pool) {
		SetError(439);
		if (Log) Log->LogError("ppl6::CDBPoolEx","Release",__FILE__,__LINE__);
		return 0;
	}
	return db->pool->Release(db);
}

/*!\brief Datenbankverbindung löschen
 * \desc
 * Mit dieser Funktion wird eine zuvor mit CDBPool::Get geholte Verbindung getrennt
 * und aus dem Pool gelöscht.
 *
 * \param[in] db Pointer auf eine Datenbankverbindung. Diese muss zuvor mit der
 * Funktion CDBPool::Get geholt worden sein.
 * \returns Konnte die Verbindung erfolgreich gelöscht werden, liefert die Funktion
 * true (1) zurück, sonst false (0). Ein Fehler kann nur auftreten, wenn die Verbindung
 * nicht aus dem gleichen Pool stammt.
 */
int CDBPoolEx::Destroy(CDBWrapper *db)
{
	if (!db) return 1;
	if (Log) Log->Printf(ppl6::LOG::INFO,4,"ppl6::CDBPoolEx","Destroy",__FILE__,__LINE__,"Lösche Datebankverbindung");
	if (db->poolex!=this) {
		SetError(439);
		if (Log) Log->LogError("ppl6::CDBPoolEx","Destroy",__FILE__,__LINE__);
		return 0;
	}
	if (!db->pool) {
		SetError(439);
		if (Log) Log->LogError("ppl6::CDBPoolEx","Destroy",__FILE__,__LINE__);
		return 0;
	}
	return db->pool->Destroy(db);
}

CDBPool *CDBPoolEx::GetPool(int id)
{
	Mutex.Lock();
	CDBPool *pool=(CDBPool *)Databases.Find((void*)id);
	Mutex.Unlock();
	if (!pool) {
		SetError(440,"%i",id);
		return NULL;
	}
	return pool;
}

CDBPool *CDBPoolEx::GetPool(const char *name)
{
	if (!name) {
		SetError(194,"CDBPool *CDBPoolEx::GetPool(==> const char *name <==)");
		return NULL;
	}
	Mutex.Lock();
	CDBPool *pool=FindPoolByName(name);
	Mutex.Unlock();
	return pool;
}

CDBPool *CDBPoolEx::GetPool(CString *name)
{
	if (!name) {
		SetError(194,"CDBPool *CDBPoolEx::GetPool(==> CString *name <==)");
		return NULL;
	}
	Mutex.Lock();
	CDBPool *pool=FindPoolByName((const char*)name->GetPtr());
	Mutex.Unlock();
	return pool;
}


/*!\brief Datenbank-Pools überprüfen
 *
 * \desc
 * Diese Funktion sollte regelmäßig innerhalb eines Überwachungsthreads aufgerufen werden,
 * um die im Pool vorhandenen Verbindungen zu überprüfen und zu expiren. Die Funktion schickt
 * regelmäßig an alle unbenutzen Verbindungen einen Ping (siehe CDBPool::SetKeepAlive) und
 * löscht Verbindungen, die nicht mehr reagieren.
 *
 * Beinhaltet der Pool sehr viele unbenutzte Verbindungen, wird empfohlen diese Funktion innerhalb
 * eines eigenen Threads zu starten, damit weitere Funktionen nicht blockiert werden.
 */
void CDBPoolEx::CheckPool()
{
	if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPoolEx","CheckPool",__FILE__,__LINE__,"CheckPool gestartet");
	double now=ppl6::GetMicrotime();
	CDBPool *pool=NULL;
	Mutex.Lock();
	Databases.Reset();
	while (1) {
		pool=(CDBPool *)Databases.GetNext();
		if (!pool) break;
		if (pool->LastCheck!=now) {
			Mutex.Unlock();
			pool->CheckPool();
			Mutex.Lock();
			pool->LastCheck=now;
			Databases.Reset();
		}
	}
	Mutex.Unlock();
	if (Log) Log->Printf(ppl6::LOG::DEBUG,5,"ppl6::CDBPoolEx","CheckPool",__FILE__,__LINE__,"CheckPool beendet");
}

int CDBPoolEx::StartPoolCheck(int intervall)
/*!\brief Überwachungsthread für den Pool starten
 *
 * \desc
 * Mit Aufruf dieseer Funktion wird ein eigener Überwachungsthread für diesen Pool gestartet. Dieser
 * tut nichts anderes als alle paar Sekunden die Funktion CDBPoolEx:CheckPool aufzurufen.
 * \param[in] intervall Ein Optionaler Parameter, der das Interavall in Millisekunden angibt, in dem die
 * CheckPool-Funktion aufgerufen werden soll. Der Defauklt ist 10000 (=10 Sekunden)
 * \returns Bei Erfolg liefert die Funktion true (1) zurück, im Fehlerfall false (0).
 * \remarks
 * Die Funktion kann mehrfach aufgerufen werden, um das Intervall nachträglich zu ändern.
 *
 */
{
	Mutex.Lock();
	if (!CheckThread) {
		CheckThread=new CDBPoolCheck;
		if (!CheckThread) {
			SetError(2);
			Mutex.Unlock();
			return 0;
		}
		((CDBPoolCheck*)CheckThread)->pool=this;
		((CDBPoolCheck*)CheckThread)->intervall=intervall;
	}
	if (CheckThread->ThreadIsRunning()) {
		((CDBPoolCheck*)CheckThread)->intervall=intervall;
		Mutex.Unlock();
		return 1;
	}
	int ret=CheckThread->ThreadStart();
	Mutex.Unlock();
	return ret;
}

int CDBPoolEx::StopPoolCheck()
/*!\brief Überwachungsthread stoppen
 *
 * \desc
 * Mit dieser Funktion wird ein zuvor gestarteter Überwachungsthread für diesen Pool wieder gestoppt.
 *
 * \returns Die Funktion liefert immer true (1) zurück.
 *
 * \see CDBPoolEx::StartPoolCheck
 */
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

int CDBPoolEx::GetStatus(CAssocArray *status)
{
	if (Log) Log->Printf(ppl6::LOG::DEBUG,6,"ppl6::CDBPoolEx","GetStatus",__FILE__,__LINE__,"Status abgefragt");
	CAssocArray r,*a=status;
	if (!a) a=new CAssocArray;

	Mutex.Lock();
	Databases.Reset();
	CDBPool *p;
	CString Key;
	while ((p=(CDBPool*)Databases.GetNext())) {
		r.Clear();
		p->GetStatus(&r);
		Key.Setf("%i",p->Id);
		a->Set(Key,r);
	}

	Mutex.Unlock();

	if (!status) {
		a->List();
		delete a;
	}
	return 1;
}



} // EOF ppl6

