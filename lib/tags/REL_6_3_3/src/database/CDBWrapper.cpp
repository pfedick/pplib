/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.18 $
 * $Date: 2009/09/07 12:28:14 $
 * $Id: CDBWrapper.cpp,v 1.18 2009/09/07 12:28:14 patrick Exp $
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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "ppl6.h"


namespace ppl6 {


CDBWrapper *ConnectDatabase(CAssocArray *dbdata)
/*!\brief Connect auf eine Datenbank erstellen
 * \ingroup PPLGroupOldDatabases
 *
 * Diese Funktion erstellt in Abhängigkeit des Datenbank-Typs ein neues Datenbank-Objekt.
 *
 * \param dbdata Ein Assoziatives Array mit folgendem Inhalt:
 * - \b type \n Der Typ der Datenbank. Derzeit werden folgende Datenbanken unterstützt:
 * 		- mysql
 * 		- sybase
 * Alle weiteren Parameter hängen von der Datenbank ab (siehe unten).
 *
 * \returns Bei Erfolg liefert die Funktion einen Pointer auf das neue Datenbank-Objekt
 * zurück. Im Fehlerfall wird NULL zurückgegeben und ein entsprechender Fehlercode
 * gesetzt.
 *
 * \par MySQL
 *
 * Folgende Paramater sind anzugeben:
 * - \b host
 * - \b port (optional, Default=3306)
 * - \b user
 * - \b password
 * - \b dbname
 *
 */
{
	if (!dbdata) {
		SetError(194,"CAssocArray *dbdata");
		return NULL;
	}
	CString *type=dbdata->GetString("type");
	if (!type) {
		SetError(341,"type");
		return NULL;
	}
	CDBWrapper *db=NULL;
	#ifdef HAVE_MYSQL
		if (type->StrCaseCmp((char*)"mysql")==0) {
			db=new CDBMySQL;
		}
	#endif
	#ifdef HAVE_SYBASE
		if (type->StrCaseCmp((char*)"sybase")==0) {
			db=new CDBSybase;
		}
	#endif
	#ifdef HAVE_POSTGRESS
		if (type->StrCaseCmp((char*)"postgres")==0) {
			db=new CDBPostgres;
		}
	#endif
	if (!db) {
		SetError(342,(char*)type->GetPtr());
		return NULL;
	}
	if (!db->Connect(dbdata)) {
		PushError();
		delete db;
		PopError();
		return NULL;
	}
	return db;
}


/*!\class CDBWrapper
 * \ingroup PPLGroupOldDatabases
 */


/*!\var CDBWrapper::Signature
 * \brief Enthält eine MD5-Summe über die Connect-Parameter zur Datenbank.
 *
 * Mit Hilfe dieser Variablen kann eine existierende Datenbank-Verbindung schneller im Pool gefunden werden,
 * da nurnoch diese Variable verglichen werden muss und nicht jeder einzelne Verbindungsparameter.
 *
 * \remarks
 * Die Variable wird nur bei Verwendung des Datenbank-Pools CDBPool gesetzt.
 *
 * \see
 * - CDBPool
 */

/*!\var CDBWrapper::lastuse
 * \brief Timestamp der letzten Nutzung
 *
 * Diese Variable enthält einen UTC-Timestamp, wann die Klasse zuletzt benutzt wurde. Der Timestamp wird
 * durch einen Connect und alle Datenbank-Operationen (ausser Ping) aktualisiert.
 *
 * \see
 * - CDBPool
 * - CDBPool::CheckPool
 */

/*!\var CDBWrapper::lastping
 * \brief Timestamp des letzten Pings
 *
 * Nicht genutzte Datenbank-Verbindungen werden durch die Funktion COPStt::CheckDBPool regelmäßig
 * angepingt, um festzustellen, ob die Verbindung noch in Ordnung ist. Bei erfolgreichem Ping wird
 * der Timestamp aktualisiert, andernfalls wird die Verbindung gelöscht.
 *
 * \see
 * - CDBPool
 * - CDBPool::CheckPool
 */

/*!\var CDBWrapper::poollock
 * \brief Flag, ob die Verbindung gerade genutzt wird
 *
 * Dieses Flag wird gesetzt, wenn die Datenbank-Verbindung durch CDBPool::Get vergeben wird und wieder
 * gelöscht, wenn sie mit CDBPool::Release wieder freigegeben wird. Ausserdem wird das Flag während
 * eines Ping-Tests gesetzt (siehe CDBWrapper::lastping und CDBPool::CheckPool).
 */


CDBWrapper::CDBWrapper()
{
	connected=false;
	Log=NULL;
	lastuse=0;
	lastping=0;
	poollock=false;
	pool=NULL;
	poolex=NULL;
}

void CDBWrapper::UpdateLastPing()
{
	lastping=ppl6::GetTime();
}

void CDBWrapper::UpdateLastUse()
{
	lastping=ppl6::GetTime();
	lastuse=lastping;
}

void CDBWrapper::ClearLastUse()
{
	lastping=lastuse=0;
}

void CDBWrapper::SetLogfile(CLog *log)
{
	Log=log;
	if (Log) Log->Printf(LOG::DEBUG,1,__FILE__,__LINE__,"CDBWrapper::SetLogfile");
}

void CDBWrapper::LogQuery(const char *query, float duration)
{
	if (!Log) return;
	Log->Printf(LOG::DEBUG,4,__FILE__,__LINE__,(const char*)NULL,0,"Querytime: %0.6f, Query: %s",duration,query);
}

CDBWrapper::~CDBWrapper()
{
	PushError();
	Close();
	SetLogfile(NULL);
	PopError();
}

int	CDBWrapper::Connect(PPL_DATABASE *dbdata)
{
	SetError(180);
	return 0;
}

int	CDBWrapper::Connect(CAssocArray *dbdata)
{
	SetError(180);
	return 0;
}

int	CDBWrapper::Reconnect()
{
	SetError(180);
	return 0;
}

int CDBWrapper::Close()
{
	SetError(180);
	return 0;
}

int CDBWrapper::SelectDB(const char * databasename)
{
	SetError(180);
	return 0;
}

int CDBWrapper::SelectDB(CString * databasename)
{
	SetError(180);
	return 0;
}

PPL_DB_RESULT CDBWrapper::Exec(const char * query, ...)
{
	char *buff=NULL;
	va_list args;
	va_start(args, query);
	if (vasprintf (&buff, (char*)query, args)<1 || buff==NULL) {
		SetError(2);
		return NULL;
	}
	PPL_DB_RESULT ret=DoExec(buff);
	free (buff);
	return ret;
}

PPL_DB_RESULT CDBWrapper::Exec(CString *query)
{
	if (!query) {
		SetError(194,"DoExec(CString *query)");
		return NULL;
	}
	return DoExec(query->GetPtr());
}


CAssocArray *CDBWrapper::ExecArrayIntern(const char *query)
{
	CAssocArray *result=new CAssocArray;
	if (!result) {
		SetError(2);
		return NULL;
	}
	if (ExecArrayIntern(result,query)) return result;
	delete result;
	return NULL;
}

CAssocArray *CDBWrapper::ExecArrayAllIntern(const char *query)
{
	CAssocArray *result=new CAssocArray;
	if (!result) {
		SetError(2);
		return NULL;
	}
	if (ExecArrayAllIntern(result,query)) return result;
	delete result;
	return NULL;
}

int CDBWrapper::ExecArrayIntern(CAssocArray *result, const char * query)
{
	PPL_DB_RESULT ret=DoExec(query);
	if (!ret) {
		//printf ("DoExec failed\n");
		return 0;
	}
	if (NumRows(ret)==0) {
		FreeResult(ret);
		SetError(182,"Der Select hat kein Ergebnis geliefert, wahrscheinlich ist der gewünschte Datensatz nicht vorhanden.");
		return 0;
	}
	PPL_ERROR e;
	int r=FetchArray(result,ret);
	StoreError(&e);
	FreeResult(ret);
	RestoreError(&e,true);
	return r;
}

int CDBWrapper::ExecArrayAllIntern(CAssocArray *result, const char * query)
{
	PPL_DB_RESULT ret=DoExec(query);
	if (!ret) return 0;
	int nr=NumRows(ret);
	for (int i=0;i<nr;i++) {
		CAssocArray *row=FetchArray(ret);
		if (row) result->Set("[]",row);
	}
	FreeResult(ret);
	return 1;
}

CAssocArray *CDBWrapper::ExecArray(const char * query, ...)
{
	CString buff;
	va_list args;
	va_start(args, query);
	buff.Vasprintf(query,args);
	va_end(args);
	return ExecArrayIntern(buff.GetPtr());
}

CAssocArray *CDBWrapper::ExecArray(CString *query)
{
	if (!query) {
		SetError(194,"ExecArray(CString *query)");
		return NULL;
	}
	return ExecArrayIntern(query->GetPtr());
}

CAssocArray *CDBWrapper::ExecArrayAll(const char * query, ...)
{
	CString buff;
	va_list args;
	va_start(args, query);
	buff.Vasprintf(query,args);
	va_end(args);
	return ExecArrayAllIntern(buff.GetPtr());
}

CAssocArray *CDBWrapper::ExecArrayAll(CString *query)
{
	if (!query) {
		SetError(194,"ExecArrayAll(CString *query)");
		return NULL;
	}
	return ExecArrayAllIntern(query->GetPtr());
}

////
int CDBWrapper::ExecArray(CAssocArray *result, const char * query, ...)
{
	CString buff;
	va_list args;
	va_start(args, query);
	buff.Vasprintf(query,args);
	va_end(args);
	return ExecArrayIntern(result,buff.GetPtr());
}

int CDBWrapper::ExecArray(CAssocArray *result, CString *query)
{
	if (!query) {
		SetError(194,"ExecArray(CString *query)");
		return 0;
	}
	return ExecArrayIntern(result,query->GetPtr());
}

int CDBWrapper::ExecArrayAll(CAssocArray *result, const char * query, ...)
{
	CString buff;
	va_list args;
	va_start(args, query);
	buff.Vasprintf(query,args);
	va_end(args);
	return ExecArrayAllIntern(result,buff.GetPtr());
}

int CDBWrapper::ExecArrayAll(CAssocArray *result, CString *query)
{
	if (!query) {
		SetError(194,"ExecArrayAll(CString *query)");
		return 0;
	}
	return ExecArrayAllIntern(result,query->GetPtr());
}

int CDBWrapper::ExecArrayAll(CAssocArray &result, CString &query)
{
	return ExecArrayAllIntern(&result,query.GetPtr());
}



PPL_DB_RESULT CDBWrapper::DoExec(const char *query)
{
	SetError(180);
	return 0;
}

ppldd CDBWrapper::NumRows(PPL_DB_RESULT result)
{
	SetError(180);
	return 0;
}

ppldd CDBWrapper::NumFields(PPL_DB_RESULT result)
{
	SetError(180);
	return 0;
}

int CDBWrapper::FreeResult(PPL_DB_RESULT result)
{
	SetError(180);
	return 180;
}

char *CDBWrapper::Result(PPL_DB_RESULT result, ppldd row, char *field)
{
	SetError(180);
	return NULL;
}

char *CDBWrapper::ResultByNo(PPL_DB_RESULT result, ppldd row, ppldd field)
{
	SetError(180);
	return NULL;
}

ppldds CDBWrapper::FieldNum(PPL_DB_RESULT result, char *field)
{
	SetError(180);
	return -1;
}

const char * CDBWrapper::FieldName(PPL_DB_RESULT result, int num)
{
	SetError(180);
	return NULL;
}

int CDBWrapper::FieldType(PPL_DB_RESULT result, int num)
{
	SetError(180);
	return DBFIELDTYPE::INVALID;
}

int CDBWrapper::FieldType(PPL_DB_RESULT result, char *field)
{
	SetError(180);
	return DBFIELDTYPE::INVALID;
}

void CDBWrapper::SetMaxRows(ppldd rows)
{

}



/*
ppldd CDBWrapper::LastErrno()
{
	return err;
}

const char *CDBWrapper::LastError()
{
	return lasterror;
}
*/

CAssocArray *CDBWrapper::FetchArray(PPL_DB_RESULT res, ppldds nr)
{
	CAssocArray *result=new CAssocArray;
	if (!result) {
		SetError(2);
		return NULL;
	}
	if (FetchArray(result,res,nr)) return result;
	delete result;
	return NULL;
}

int CDBWrapper::FetchArray(CAssocArray *result, PPL_DB_RESULT res, ppldds nr)
{
	SetError(180);
	return 0;
}

void CDBWrapper::PrintResult(PPL_DB_RESULT res)
{
	SetError(180);
}

int CDBWrapper::Ping()
{
	SetError(180);
	return 0;
}

char *CDBWrapper::EscapeString(char *string, size_t length)
{
	SetError(180);
	return NULL;
}

int CDBWrapper::EscapeString(CString *string)
{
	SetError(180);
	return 0;
}

ppld64 CDBWrapper::GetInsertID()
{
	SetError(180);
	return 0;
}

int CDBWrapper::NumAffectedRows(PPL_DB_RESULT result)
{
	SetError(180);
	return 0;
}

int CDBWrapper::Save(const char *method, const char *table, CAssocArray *a, const char *clause, CAssocArray *exclude)
{
	if (!method) {
		SetError(194,"const char *method");
		return 0;
	}
	if (!table) {
		SetError(194,"const char *table");
		return 0;
	}
	if (!a) {
		SetError(194,"CAssocArray *a");
		return 0;
	}
	if (a->Count()==0) {
		SetError(343);
		return 0;
	}
	CString Keys, Vals, Key, Value, Method, Table, Query, Clause;
	Method=method;
	Table=table;
	Method.LCase();
	if (Table.Len()==0) {
		SetError(344);
		return 0;
	}
	if (clause) Clause=clause;
	a->Reset();
	PPL_DB_RESULT res;
	if (Method=="insert" || Method=="replace") {
		while (a->GetNext(&Key,&Value)) {
			if (exclude==NULL || exclude->GetChar(Key)==0) {
				Keys.Concat(Key);
				Keys.Concat(",");
				if (!EscapeString(&Value)) return 0;
				Vals.Concatf("'%s',",(const char*)Value);
			}
		}
		Keys.Chop();
		Vals.Chop();

		Query.Setf("%s into %s (%s) values (%s)",(const char*)Method, (const char*)Table, (const char*)Keys, (const char*)Vals);
		//printf ("Query: %s\n",(char*)Query);
		res=Exec(&Query);
		if (!res) return 0;
		FreeResult(res);
		return 1;
	} else if (Method=="update") {
		Query.Setf("%s %s set ",(const char*)Method, (const char*)Table);
		while (a->GetNext(&Key,&Value)) {
			if (!EscapeString(&Value)) return 0;
			Query.Concatf("%s=\"%s\",",(const char*)Key,(const char*)Value);
		}
		Query.Chop();
		Clause.Trim();
		if (Clause.Len()>0) {
			Query.Concat(" ");
			if (!Clause.PregMatch("/^where\\s/i")) Query.Concat("where ");
			Query.Concat(Clause);
		}
		res=Exec(&Query);
		if (!res) return 0;
		FreeResult(res);
		return 1;
	}
	SetError(345,(const char*)Method);
	return 0;
}

int CDBWrapper::InsertKey(const char *table, CAssocArray *a, const char *keyname, int *id, CAssocArray *exclude)
{
	ppluint64 i;
	if (InsertKey(table,a,keyname,&i,exclude)) {
		*id=(int)i;
	}
	return 0;
}

int CDBWrapper::InsertKey(const char *table, CAssocArray *a, const char *keyname, ppluint64 *id, CAssocArray *exclude)
{
	if (!Exec("lock tables %s write",table)) {
		PushError();
		Exec("unlock tables");
		PopError();
		return 0;
	}
	ppluint64 i;
	CAssocArray r;
	if (!ExecArray(&r,"select max(%s)+1 as newid from %s",keyname,table)) {
		PushError();
		Exec("unlock tables");
		PopError();
		return 0;
	}
	i=atoll(r.Get("newid"));
	if (!i) i=1;
	*id=i;
	a->Setf(keyname,"%llu",i);
	int ret=Save("insert",table,a,NULL,exclude);
	//PrintError();
	PushError();
	Exec("unlock tables");
	PopError();
	return ret;
}



int CDBWrapper::ReadKeyValue(CAssocArray *res, const char *query, const char *keyname, const char *valname)
{
	if (!res) {
		SetError(194,"int CDBWrapper::ReadKeyValue(==> CAssocArray *res <==, const char *query, const char *keyname, const char *valname)");
		return 0;
	}
	if (!query) {
		SetError(194,"int CDBWrapper::ReadKeyValue(CAssocArray *res, ==> const char *query <==, const char *keyname, const char *valname)");
		return 0;
	}
	if (!keyname) {
		SetError(194,"int CDBWrapper::ReadKeyValue(CAssocArray *res, const char *query, ==> const char *keyname <==, const char *valname)");
		return 0;
	}
	PPL_DB_RESULT r=Exec((char*)query);
	if (!r) return 0;
	ppluint32 nr=NumRows(r);
	CAssocArray row;
	for (ppluint32 i=0;i<nr;i++) {
		row.Clear();
		if (FetchArray(&row,r,i)) {
			const char *key=row[keyname];
			if (!key) {
				SetError(294,"%s",keyname);
				return 0;
			}
			if (valname) {
				//$a[$r[$keyfield]]=$r[$valfield];
				const char *val=row[valname];
				if (val) res->Set(key,val);
			} else {
				res->Set(key,&row);
			}
		}
	}
	FreeResult(r);
	return 1;
}


} // end of namespace ppl
