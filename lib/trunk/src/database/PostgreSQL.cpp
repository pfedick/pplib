/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.8 $
 * $Date: 2010/10/16 13:35:42 $
 * $Id: Database.cpp,v 1.8 2010/10/16 13:35:42 pafe Exp $
 *
 *******************************************************************************
 * Copyright (c) 2012, Patrick Fedick <patrick@pfp.de>
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
#include "ppl7.h"
#include "ppl7-db.h"

#ifdef HAVE_POSTGRESQL
#include <libpq-fe.h>
#endif

namespace ppl7 {
namespace db {



class Postgres92Result : public ResultSet
{
	friend class Postgres92;
	private:
#ifdef HAVE_POSTGRESQL
		PGresult	*res;			//!\brief Postgres-spezifisches Result-Handle
		PGconn		*conn;			//!\brief Postgres-spezifisches Handle des Datenbank-Connects, das den Result erzeugt hat
#endif
		PostgreSQL	*postgres_class;	//!\brief Die ppl6::db::MySQL-Klasse, die das Result erzeugt hat
		pplint64	rows;			//!\brief Anzahl Zeilen im Ergebnis
		pplint64 	lastinsertid;	//!\brief Falls es sich um einen Insert mit einem Autoincrement-Index handelte, steht hier die vergebene ID
		pplint64	affectedrows;	//!\brief Falls es sich um ein Update/Insert/Replace handelte, steht hier die Anzahl betroffender Datensätze
		int			num_fields;		//!\brief Anzahl Spalten im Ergebnis
		pplint64	nextrow;
		bool 		bRowsCounted;

	public:
		Postgres92Result();
		virtual ~Postgres92Result();
		virtual	void Clear();
		virtual pplint64 Rows();
		virtual pplint64 Affected();
		virtual int Fields();
		virtual int FieldNum(const char *fieldname);
		virtual const char *FieldName(int field);
		virtual ResultSet::Type	FieldType(int field);
		virtual ResultSet::Type	FieldType(const char *fieldname);
		virtual AssocArray  FetchArray(pplint64 row=-1);
		virtual int		FetchArray(AssocArray &array, pplint64 row=-1);
		virtual Array  FetchFields(pplint64 row=-1);
		virtual int		FetchFields(Array &array, pplint64 row=-1);
		virtual int		Seek(pplint64 row);
		virtual void    PrintResult();
};

#ifdef OLDCODE

Postgres92Result::Postgres92Result()
{
	res=NULL;
	postgres_class=NULL;
	conn=NULL;
	rows=0;
	lastinsertid=0;
	affectedrows=0;
	num_fields=0;
	nextrow=0;
	bRowsCounted=false;
}

Postgres92Result::~Postgres92Result()
{
	Clear();
}

void Postgres92Result::Clear()
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


pplint64 Postgres92Result::Rows()
{
	return rows;
}

pplint64 Postgres92Result::Affected()
{
	return affectedrows;
}

int Postgres92Result::Fields()
{
	return num_fields;
}

int Postgres92Result::FieldNum(const char *fieldname)
{
	if (!res) {
		SetError(182);
		return 0;
	}
	if (!fieldname) {
		SetError(194);
		return 0;
	}
	int n=PQfnumber(res,fieldname);
	if (n==-1) {
		SetError(183,"%s",fieldname);
		return -1;
	}
	return n;
}

const char *Postgres92Result::FieldName(int field)
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

Result::Type Postgres92Result::FieldType(int field)
{
	Oid o=PQftype(res,field);
	switch (o) {
		default:
			return Result::Unknown;
	}
	return Result::Unknown;
}

Result::Type Postgres92Result::FieldType(const char *fieldname)
{
	int num=FieldNum(fieldname);
	if (num<0) return Result::Unknown;
	return FieldType(num);
}

CAssocArray Postgres92Result::FetchArray(pplint64 row)
{
	CAssocArray a;
	FetchArray(a,row);
	return a;
}

int	Postgres92Result::FetchArray(CAssocArray &array, pplint64 row)
{
	if (!res) {
		SetError(182);
		return 0;
	}
	if (PQntuples(res)==0) {
		PQclear(res);
		res=NULL;
		SetError(182);
		return 0;
	}
	array.Clear();
	for(int i=0; i<num_fields; i++) {
		array.Set(PQfname(res,i),PQgetvalue(res,0,i));
	}
	PQclear(res);
	res=PQgetResult(conn);
	return 1;
}

CArray  Postgres92Result::FetchFields(pplint64 row)
{
	CArray a;
	FetchFields(a,row);
	return a;
}

int	Postgres92Result::FetchFields(CArray &array, pplint64 row)
{
	if (!res) {
		SetError(182);
		return 0;
	}
	if (PQntuples(res)==0) {
		SetError(182);
		PQclear(res);
		res=NULL;
		return 0;
	}

	array.Clear();
	const char *tmp;
	for(int i=0; i<num_fields; i++) {
		tmp=PQgetvalue(res,0,i);
		if (tmp) array.Add(tmp);
		else array.Add("");
	}
	PQclear(res);
	res=PQgetResult(conn);
	return 1;
}

int	Postgres92Result::Seek(pplint64 row)
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

void Postgres92Result::PrintResult()
{

}


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




Postgres92::Postgres92()
{
	conn=NULL;
	lastinsertid=0;
	affectedrows=0;
	maxrows=0;
	transactiondepth=0;
}

Postgres92::~Postgres92()
{
#ifdef HAVE_POSTGRESQL
	PushError();
	if (conn) Disconnect();
	PopError();
#endif
}

/*!\brief Connect auf eine Postgres-Datenbank erstellen
 *
 * \descr
 * Mit dieser Funktion wird eine Verbindung zu einem Postgres Datenbank-Server hergestellt, wobei
 * die dafür notwendigen Parameter dem Array \p params entnommen werden.
 *
 * Die für den Connect erforderlichen oder optionalen Parameter hängen von der jeweiligen
 * Datenbank ab und sind in der jeweiligen Dokumentation zu finden. Es gibt jedoch eine
 * Reihe von Parametern, die bei allen Datenbanken identisch sind:
 * - \b host: Der Hostname oder die IP-Adresse des Datenbank-Servers
 * - \b port: Der TCP-Port des Datenbank-Servers
 * - \b dbname: Der Name der intialen Datenbank.
 * - \b user: Der Name des Benutzers, mit dem sich an der Datenbank authentifiziert werden soll
 * - \b password: Das Passwort des Benutzers im Klartext
 * - \b searchpath: Kommaseparierte Liste mit den Schemata, die in den Suchpfad
 *      aufgenommen werden sollen
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
int Postgres92::Connect(const CAssocArray &params)
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres92");
	return 0;
#else
	if (conn) Disconnect();
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
		return 0;
	}
	// Pruefen, ob auch wirklich eine Verbindung da ist
	if (PQstatus((PGconn*)conn) == CONNECTION_OK) {
		ppl6::CString SearchPath=params["searchpath"];
		if (SearchPath.NotEmpty()) {
			Escape(SearchPath);
			if (!Execf("set search_path to %s",(const char*)SearchPath)) {
				PQfinish((PGconn*)conn);
				conn=NULL;
				return 0;
			}
		}
		UpdateLastUse();
		return 1;
	}
	ClearLastUse();

	// Was war der Fehler?
	SetError(77,0,PQerrorMessage((PGconn*)conn));
	PQfinish((PGconn*)conn);
	conn=NULL;
	return 0;
#endif
}


int Postgres92::ConnectCreate(const CAssocArray &params)
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

int Postgres92::Reconnect()
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres92");
	return 0;
#else
	if (conn) {
		PQreset((PGconn *)conn);
		if (PQstatus((PGconn*)conn) == CONNECTION_OK) {
			UpdateLastUse();
			return 1;
		}
	}
	// Hat nicht geklappt, wir versuchen einen normalen Connect
	Close();
	CAssocArray a;
	a.Copy(&condata);
	if (Connect(a)) return 1;
	SetError(299,"Reconnect failed");
	return 0;
#endif
}

int Postgres92::Disconnect()
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres92");
	return 0;
#else
	if (!conn) {
		return 1;
	}
	PQfinish((PGconn*)conn);
	conn=NULL;
	ClearLastUse();
	return 1;
#endif
}



int Postgres92::SelectDB(const char *databasename)
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres92");
	return 0;
#else
	// Scheinbar können wir die Datenbank nicht innerhalb der laufenden Verbindung
	// wechseln
	if (!conn) { SetError(181); return 0; }

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
void *Postgres92::Pgsql_Query(const CString &query)
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres92");
	return NULL;
#else
	PGresult *res=NULL;
	ExecStatusType status;
	int count=0;
	while (count<2) {
		count++;
		if (PQsendQuery((PGconn *)conn,query)!=1) {
			SetError(138,0,"PQsendQuery failed: %s",PQerrorMessage((PGconn*)conn));
		} else {
			if (PQsetSingleRowMode((PGconn *)conn)!=1) {
				SetError(138,0,"PQsetSingleRowMode failed: %s",PQerrorMessage((PGconn*)conn));
				if (res) PQclear(res);
				return NULL;
			}
			res=PQgetResult((PGconn *)conn);
			status=PQresultStatus(res);
			if (status==PGRES_COMMAND_OK
					|| status==PGRES_SINGLE_TUPLE
					|| status==PGRES_TUPLES_OK ) {
				// InsertId eines Autoincrement holen, sofern vorhanden
				// TODO: Das kann Postgres nicht, PQoidValue liefert die letzte OID zurück,
				// falls vorhanden, was nichts mit dem autoincrement eines "serial"-Datentypes
				// der Tabelle zu tun hat!
				Oid oid=PQoidValue(res);
				if (oid==InvalidOid) lastinsertid=0;
				else lastinsertid=oid;
				//printf ("Oid=%i, lastinsertId=%i\n",oid,lastinsertid);
				// Anzahl veränderter Datensätze
				affectedrows=ppl6::atoll(PQcmdTuples(res));
				return res;
			} else if (status==PGRES_FATAL_ERROR) {
				SetError(138,0,"%s",PQresultErrorMessage(res));
				/* Laut Doku: Even when PQresultStatus indicates a fatal error, PQgetResult should
				 * be called until it returns a null pointer, to allow libpq to process the error
				 * information completely.
				 */
				while ((res=PQgetResult((PGconn *)conn))!=NULL) {
					PQclear(res);
					res=NULL;
				}
				return NULL;
			} else {
				SetError(138,0,"%s",PQresultErrorMessage(res));
			}
		}
		if (res) PQclear(res);
		if (PQstatus((PGconn*)conn) != CONNECTION_OK) {
			if (!Reconnect()) {
				return NULL;
			}
		} else break;
	}
	return NULL;
#endif
}

int Postgres92::Exec(const CString &query)
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres92");
	return 0;
#else
	if (!conn) { SetError(181); return 0; }
	double t_start;
	PGresult *res;
	t_start=GetMicrotime();
	res=(PGresult*)Pgsql_Query(query);
	UpdateLastUse();
	if (res) {
		// Result-Handle freigeben
		PQclear(res);
		LogQuery(query,(float)(getmicrotime()-t_start));
		return 1;
	}
	return 0;
#endif
}

Result *Postgres92::Query(const CString &query)
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres92");
	return NULL;
#else
	if (!conn) { SetError(181); return 0; }
	double t_start;
	PGresult *res;
	t_start=GetMicrotime();
	res=(PGresult*)Pgsql_Query(query);
	UpdateLastUse();
	if (res) {
		// Query loggen
		LogQuery(query,(float)(getmicrotime()-t_start));
		// Result-Klasse erstellen
		Postgres92Result *pr=new Postgres92Result;
		if (!pr) {
			PQclear(res);
			SetError(2);
			return NULL;
		}
		pr->res=res;
		pr->postgres_class=this;
		pr->conn=(PGconn *)conn;
		pr->rows=PQntuples(res);
		pr->lastinsertid=lastinsertid;
		pr->affectedrows=affectedrows;
		pr->num_fields=PQnfields(res);
		return pr;
	}
	return 0;
#endif
}

void Postgres92::SetMaxRows(ppluint64 rows)
{
	maxrows=rows;
}

int Postgres92::Ping()
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres92");
	return 0;
#else
	if (!conn) { SetError(181); return 0; }
	PGresult *res;
	res=(PGresult*)Pgsql_Query("select 1 as result");
	if (res) {
		// Result-Handle freigeben
		PQclear(res);
		return 1;
	}
	SetError(181);
	return 0;

#endif
}

int Postgres92::Escape(CString &str) const
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres92");
	return 0;
#else
	if (!conn) { SetError(181); return 0; }
	size_t l=str.Len()*2+1;
	char *buf=(char *)malloc(l);   // Buffer reservieren
	if (!buf) {
		SetError(2);
		return 0;
	}
	int error;
	size_t newlength=PQescapeStringConn((PGconn*)conn,buf,(const char*)str,str.Len(),&error);
	if (error==0) {
		//ppl6::HexDump(buf,newlength);
		str.Set(buf,newlength);
		free(buf);
		return 1;
	}
	free(buf);
	SetError(355,0,PQerrorMessage((PGconn*)conn));
	return 0;
#endif
}

ppluint64 Postgres92::GetInsertID()
{
	return lastinsertid;
}

pplint64 Postgres92::GetAffectedRows()
{
	return affectedrows;
}

int Postgres92::StartTransaction()
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres92");
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

int Postgres92::EndTransaction()
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres92");
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

int Postgres92::CancelTransaction()
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres92");
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

int Postgres92::CancelTransactionComplete()
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres92");
	return 0;
#else
	if (Exec("ROLLBACK")) {
		transactiondepth=0;
		return 1;
	}
	return 0;
#endif
}

int Postgres92::CreateDatabase(const char *name)
{
#ifndef HAVE_POSTGRESQL
	SetError(511,"Postgres92");
	return 0;
#else
	// CREATE DATABASE dbname OWNER rolename;
	SetError(5);
	return 0;
#endif
}

CString	Postgres92::databaseType() const
{
	return CString("Postgres92");
}

CString Postgres92::getQuoted(const CString &value, const CString &type) const
{
	CString Type=type;
	CString s=value;
	Type.LCase();
	Escape(s);
	if (Type=="int" || Type=="integer") return s;
	if (Type=="bit" || Type=="boolean") return "'"+s+"'";
	return "'"+s+"'";
}

#endif

}	// EOF namespace db
}	// EOF namespace ppl7


