/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.4 $
 * $Date: 2010/10/21 09:53:12 $
 * $Id: Postgres.cpp,v 1.4 2010/10/21 09:53:12 pafe Exp $
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

#ifdef HAVE_POSTGRESQL
#include <libpq-fe.h>
#endif

namespace ppl6 {
namespace db {


#ifdef HAVE_POSTGRESQL
class PostgresResult : public Result
{
	friend class Postgres;
	private:
		PGresult	*res;			//!\brief Postgres-spezifisches Result-Handle
		Postgres	*postgres_class;	//!\brief Die ppl6::db::MySQL-Klasse, die das Result erzeugt hat
		PGconn		*conn;			//!\brief Postgres-spezifisches Handle des Datenbank-Connects, das den Result erzeugt hat
		pplint64	rows;			//!\brief Anzahl Zeilen im Ergebnis
		pplint64 	lastinsertid;	//!\brief Falls es sich um einen Insert mit einem Autoincrement-Index handelte, steht hier die vergebene ID
		pplint64	affectedrows;	//!\brief Falls es sich um ein Update/Insert/Replace handelte, steht hier die Anzahl betroffender Datensätze
		int			num_fields;		//!\brief Anzahl Spalten im Ergebnis
		pplint64	nextrow;

	public:
		PostgresResult();
		virtual ~PostgresResult();
		virtual	void Clear();
		virtual pplint64 Rows();
		virtual pplint64 Affected();
		virtual int Fields();
		virtual const char *Get(pplint64 row, const char *fieldname);
		virtual const char *Get(pplint64 row, int field);
		virtual int FieldNum(const char *fieldname);
		virtual const char *FieldName(int field);
		virtual Result::Type	FieldType(int field);
		virtual Result::Type	FieldType(const char *fieldname);
		virtual CAssocArray  FetchArray(pplint64 row=-1);
		virtual int		FetchArray(CAssocArray &array, pplint64 row=-1);
		virtual CArray  FetchFields(pplint64 row=-1);
		virtual int		FetchFields(CArray &array, pplint64 row=-1);
		virtual int		Seek(pplint64 row);
		virtual void    PrintResult();
};

PostgresResult::PostgresResult()
{
	res=NULL;
	postgres_class=NULL;
	conn=NULL;
	rows=0;
	lastinsertid=0;
	affectedrows=0;
	num_fields=0;
	nextrow=0;
}

PostgresResult::~PostgresResult()
{
	Clear();
}

void PostgresResult::Clear()
{
	rows=0;
	if (res) PQclear(res);
	res=NULL;
	postgres_class=NULL;
	conn=NULL;
	lastinsertid=0;
	affectedrows=0;
	num_fields=0;
}


pplint64 PostgresResult::Rows()
{
	return rows;
}

pplint64 PostgresResult::Affected()
{
	return affectedrows;
}

int PostgresResult::Fields()
{
	return num_fields;
}

const char *PostgresResult::Get(pplint64 row, const char *fieldname)
{
	int field=FieldNum(fieldname);
	if (field<0) return NULL;
	return Get(row,field);
}

const char *PostgresResult::Get(pplint64 row, int field)
{
	if (!res) {
		SetError(182);
		return 0;
	}
	if (field>num_fields) {
		SetError(183,"%i",field);
		return NULL;
	}
	nextrow=row+1;
	int r=(int)(row&0xffffffff);
	const char *value=PQgetvalue(res,r,field);
	if (!value) {
		SetError(222,"%lli",row);
	}
	return value;
}

int PostgresResult::FieldNum(const char *fieldname)
{
	if (!res) {
		SetError(182);
		return 0;
	}
	if (!fieldname) {
		SetError(194);
		return NULL;
	}
	int n=PQfnumber(res,fieldname);
	if (n==-1) {
		SetError(183,"%s",fieldname);
		return -1;
	}
	return n;
}

const char *PostgresResult::FieldName(int field)
{
	if (!res) {
		SetError(182);
		return 0;
	}
	if (field>num_fields) {
		SetError(183,"%i",field);
		return NULL;
	}
	const char *name=PQfname(res,field);
	if (!name) {
		SetError(183,"%i",field);
	}
	return name;
}

Result::Type PostgresResult::FieldType(int field)
{
	Oid o=PQftype(res,field);
	switch (o) {
		default:
			return Result::Unknown;
	}
	return Result::Unknown;
}

Result::Type PostgresResult::FieldType(const char *fieldname)
{
	int num=FieldNum(fieldname);
	if (num<0) return Result::Unknown;
	return FieldType(num);
}

CAssocArray PostgresResult::FetchArray(pplint64 row)
{
	CAssocArray a;
	FetchArray(a,row);
	return a;
}

int	PostgresResult::FetchArray(CAssocArray &array, pplint64 row)
{
	if (!res) {
		SetError(182);
		return 0;
	}
	if (row>=0) nextrow=row;

	int nr=(int)(nextrow&0xffffffff);
	nextrow++;
	if (nr>PQntuples(res)) {
		SetError(222);
		return 0;
	}
	array.Clear();
	for(int i=0; i<num_fields; i++) {
		array.Set(PQfname(res,i),PQgetvalue(res,nr,i));
	}
	return 1;
}

CArray  PostgresResult::FetchFields(pplint64 row)
{
	CArray a;
	FetchFields(a,row);
	return a;
}

int	PostgresResult::FetchFields(CArray &array, pplint64 row)
{
	if (!res) {
		SetError(182);
		return 0;
	}
	if (row>=0) nextrow=row;

	int nr=(int)(nextrow&0xffffffff);
	nextrow++;
	if (nr>PQntuples(res)) {
		SetError(222);
		return 0;
	}
	array.Clear();
	const char *tmp;
	for(int i=0; i<num_fields; i++) {
		tmp=PQgetvalue(res,nr,i);
		if (tmp) array.Add(tmp);
		else array.Add("");
	}
	return 1;
}

int	PostgresResult::Seek(pplint64 row)
{
	if (!res) {
		SetError(182);
		return 0;
	}
	if (row<=rows) {
		nextrow=row;
		return 1;
	}
	SetError(222);
	return 0;
}

void PostgresResult::PrintResult()
{

}


#endif



/*!\class Postgres
 * \ingroup PPLGroupDatabases
 * \brief Implementierung einer Postgres-Datenbank
 *
 * \header \#include <ppl6-db.h>
 *
 * \descr
 * Mit dieser Klasse kann eine Verbindung zu einer Postgres-Datenbank aufgebaut werden, um darüber
 * SQL-Queries durchzuführen.
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_Postgres_Example1
 * \until EOF
 */




Postgres::Postgres()
{
	conn=NULL;
	lastinsertid=0;
	affectedrows=0;
	maxrows=0;
	transactiondepth=0;
}

Postgres::~Postgres()
{
#ifdef HAVE_POSTGRESQL
	PushError();
	if (conn) Disconnect();
	PopError();
#endif
}

int Postgres::Connect(const CAssocArray &params)
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres");
	return 0;
#else
	if (conn) Disconnect();
	mutex.Lock();
	condata=params;
	CString conninfo;
	if (params["host"]) conninfo.Concatf("host=%s ",params["host"]);
	if (params["port"]) conninfo.Concatf("port=%s ",params["port"]);
	if (params["dbname"]) conninfo.Concatf("dbname='%s' ",params["dbname"]);
	if (params["user"]) conninfo.Concatf("user='%s' ",params["user"]);
	if (params["password"]) conninfo.Concatf("password='%s' ",params["password"]);

	conn=PQconnectdb((const char *)conninfo);
	if (!conn) {
		SetError(2);
		mutex.Unlock();
		return 0;
	}
	// Pruefen, ob auch wirklich eine Verbindung da ist
	if (PQstatus((PGconn*)conn) == CONNECTION_OK) {
		UpdateLastUse();
		mutex.Unlock();
		return 1;
	}
	ClearLastUse();

	// Was war der Fehler?
	SetError(77,0,PQerrorMessage((PGconn*)conn));
	PQfinish((PGconn*)conn);
	conn=NULL;
	mutex.Unlock();
	return 0;
#endif
}


int Postgres::ConnectCreate(const CAssocArray &params)
{
	CAssocArray a=params;
	a.Delete("dbname");
	a.Set("dbname","postgres");
	const char *dbname=params["dbname"];
	// Versuch auf die immer vorhandene Datenbank "postgres" zuzugreifen
	if (!Connect(a)) return 0;
	// Wir versuchen die Datenbank auszuwählen
	if (SelectDB(dbname)) return 1;
	if (!CreateDatabase(dbname)) return 0;
	if (SelectDB(dbname)) return 1;
	return 0;
}

int Postgres::Reconnect()
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres");
	return 0;
#else
	mutex.Lock();
	if (conn) {
		PQreset((PGconn *)conn);
		if (PQstatus((PGconn*)conn) == CONNECTION_OK) {
			UpdateLastUse();
			mutex.Unlock();
			return 1;
		}
	}
	mutex.Unlock();
	// Hat nicht geklappt, wir versuchen einen normalen Connect
	Close();
	CAssocArray a;
	a.Copy(&condata);
	if (Connect(a)) return 1;
	SetError(299,"Reconnect failed");
	return 0;
#endif
}

int Postgres::Disconnect()
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres");
	return 0;
#else
	mutex.Lock();
	if (!conn) {
		mutex.Unlock();
		return 1;
	}
	PQfinish((PGconn*)conn);
	conn=NULL;
	ClearLastUse();
	mutex.Unlock();
	return 1;
#endif
}



int Postgres::SelectDB(const char *databasename)
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres");
	return 0;
#else
	// Scheinbar können wir die Datenbank nicht innerhalb der laufenden Verbindung
	// wechseln
	mutex.Lock();
	if (!conn) { SetError(181); mutex.Unlock(); return 0; }
	mutex.Unlock();

	Close();
	CAssocArray a,b;
	a=condata;
	b=condata; // Backup
	a.Set("dbname",databasename);
	if (Connect(a)) return 1;
	Connect(b);	// Mit der ursprünglichen Datenbank verbinden
	SetError(295);
	return 0;
#endif
}

/*!\brief Query ausführen
 *
 * \descr
 * Dies ist eine interne Funktion, die einen Query an die Postgres-Datenbank schickt.
 * Schlägt dies fehl, weil die Verbindung zur Datenbank zwischenzeitlich verloren ging,
 * wird ein Reconnect versucht und bei Erfolg der Query wiederholt. Die Klasse erwartet,
 * dass der Mutex bereits gelockt ist und die Variable Postgres::conn ein gültiges Postgres
 * Connection-Handle enthält.
 *
 * \param[in] query String mit dem abzusetzenden Query
 *
 * \return Konnte der Query erfolgreich ausgeführt werden, liefert die Funktion 1
 * zurück, im Fehlerfall 0. Der Mutex ist bei Verlassen der Funktion auf jeden Fall gesetzt.
 *
 */
void *Postgres::Pgsql_Query(const CString &query)
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres");
	return NULL;
#else
	PGresult *res;
	ExecStatusType status;
	int count=0;
	while (count<2) {
		count++;
		res=PQexec((PGconn *)conn,query);
		status=PQresultStatus(res);
		if (status==PGRES_COMMAND_OK ||
			status==PGRES_TUPLES_OK ) {
			// InsertId eines Autoincrement holen, sofern vorhanden
			Oid oid=PQoidValue(res);
			if (oid==InvalidOid) lastinsertid=0;
			else lastinsertid=oid;
			// Anzahl veränderter Datensätze
			affectedrows=ppl6::atoll(PQcmdTuples(res));
			return res;
		}
		PQclear(res);
		if (PQstatus((PGconn*)conn) != CONNECTION_OK) {
			mutex.Unlock();
			if (!Reconnect()) {
				mutex.Lock();
				return NULL;
			}
			mutex.Lock();
		}
	}
	SetError(138,0,PQresultErrorMessage(res));
	return NULL;
#endif
}

int Postgres::Exec(const CString &query)
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres");
	return 0;
#else
	mutex.Lock();
	if (!conn) { SetError(181); mutex.Unlock(); return 0; }
	double t_start;
	PGresult *res;
	t_start=GetMicrotime();
	res=(PGresult*)Pgsql_Query(query);
	UpdateLastUse();
	if (res) {
		// Result-Handle freigeben
		PQclear(res);
		LogQuery(query,(float)(getmicrotime()-t_start));
		mutex.Unlock();
		return 1;
	}
	mutex.Unlock();
	return 0;
#endif
}

Result *Postgres::Query(const CString &query)
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres");
	return NULL;
#else
	mutex.Lock();
	if (!conn) { SetError(181); mutex.Unlock(); return 0; }
	double t_start;
	PGresult *res;
	t_start=GetMicrotime();
	res=(PGresult*)Pgsql_Query(query);
	UpdateLastUse();
	if (res) {
		// Query loggen
		LogQuery(query,(float)(getmicrotime()-t_start));
		// Result-Klasse erstellen
		PostgresResult *pr=new PostgresResult;
		if (!pr) {
			PQclear(res);
			SetError(2);
			mutex.Unlock();
			return NULL;
		}
		pr->res=res;
		pr->postgres_class=this;
		pr->conn=(PGconn *)conn;
		pr->rows=PQntuples(res);
		pr->lastinsertid=lastinsertid;
		pr->affectedrows=affectedrows;
		pr->num_fields=PQnfields(res);
		mutex.Unlock();
		return pr;
	}
	mutex.Unlock();
	return 0;
#endif
}

void Postgres::SetMaxRows(ppluint64 rows)
{
	maxrows=rows;
}

int Postgres::Ping()
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres");
	return 0;
#else
	mutex.Lock();
	if (!conn) { SetError(181); mutex.Unlock(); return 0; }
	PGresult *res;
	res=(PGresult*)Pgsql_Query("select 1 as result");
	if (res) {
		// Result-Handle freigeben
		PQclear(res);
		mutex.Unlock();
		return 1;
	}
	mutex.Unlock();
	SetError(181);
	return 0;

#endif
}

int Postgres::Escape(CString &str)
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres");
	return 0;
#else
	mutex.Lock();
	if (!conn) { SetError(181); mutex.Unlock(); return 0; }
	size_t l=str.Len()*2+1;
	char *buf=(char *)malloc(l);   // Buffer reservieren
	if (!buf) {
		SetError(2);
		mutex.Unlock();
		return 0;
	}
	int error;
	size_t newlength=PQescapeStringConn((PGconn*)conn,buf,(const char*)str,str.Len(),&error);
	if (error==0) {
		mutex.Unlock();
		//ppl6::HexDump(buf,newlength);
		str.Set(buf,newlength);
		free(buf);
		return 1;
	}
	mutex.Unlock();
	free(buf);
	SetError(355,0,PQerrorMessage((PGconn*)conn));
	return 0;
#endif
}

ppluint64 Postgres::GetInsertID()
{
	return lastinsertid;
}

pplint64 Postgres::GetAffectedRows()
{
	return affectedrows;
}

int Postgres::StartTransaction()
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres");
	return 0;
#else
	if (transactiondepth==0) {	// Neue Transaktion
		if (Exec("BEGIN")) {
			transactiondepth++;
			return 1;
		}
	} else {
		if (Execf("SAVEPOINT LEVEL%i",transactiondepth)) {
			transactiondepth++;
			return 1;
		}
	}
	return 0;
#endif
}

int Postgres::EndTransaction()
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres");
	return 0;
#else
	if (transactiondepth==1) {
		if (Exec("COMMIT")) {
			transactiondepth=0;
			return 1;
		}
	} else {
		if (Execf("RELEASE SAVEPOINT LEVEL%i",transactiondepth-1)) {
			transactiondepth--;
			return 1;
		}
	}
	return 0;
#endif
}

int Postgres::CancelTransaction()
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres");
	return 0;
#else
	if (transactiondepth==1) {
		if (Exec("ROLLBACK")) {
			transactiondepth=0;
			return 1;
		}
	} else {
		if (Execf("ROLLBACK TO SAVEPOINT LEVEL%i",transactiondepth-1)) {
			transactiondepth--;
			return 1;
		}
	}
	return 0;
#endif
}

int Postgres::CancelTransactionComplete()
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres");
	return 0;
#else
	if (Exec("ROLLBACK")) {
		transactiondepth=0;
		return 1;
	}
	return 0;
#endif
}

int Postgres::CreateDatabase(const char *name)
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres");
	return 0;
#else
	// CREATE DATABASE dbname OWNER rolename;
	SetError(5);
	return 0;
#endif
}

CString	Postgres::databaseType() const
{
	return CString("Postgres");
}


}	// EOF namespace db
}	// EOF namespace ppl6
