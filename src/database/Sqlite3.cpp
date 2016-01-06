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

#ifdef HAVE_SQLITE3
//#include <libpq-fe.h>
#endif

namespace ppl7 {
namespace db {

#ifdef HAVE_SQLITE3

#ifdef TODO
class Postgres92Result : public ResultSet
{
	friend class PostgreSQL;
	private:
		PGresult	*res;			//!\brief Postgres-spezifisches Result-Handle
		PGconn		*conn;			//!\brief Postgres-spezifisches Handle des Datenbank-Connects, das den Result erzeugt hat
		PostgreSQL	*postgres_class;	//!\brief Die ppl6::db::MySQL-Klasse, die das Result erzeugt hat
		ppluint64	result_rows;		//!\brief Anzahl Zeilen im Ergebnis
		ppluint64	affectedrows;	//!\brief Falls es sich um ein Update/Insert/Replace handelte, steht hier die Anzahl betroffender Datensätze
		int			num_fields;		//!\brief Anzahl Spalten im Ergebnis

	public:
		Postgres92Result();
		virtual ~Postgres92Result();
		virtual	void		clear();
		virtual ppluint64	rows() const;
		virtual ppluint64	affected() const;
		virtual int			fields() const;
		virtual String		getString(const String &fieldname);
		virtual String		getString(int field);
		virtual int			fieldNum(const String &fieldname);
		virtual String		fieldName(int field);
		virtual FieldType	fieldType(int field);
		virtual FieldType	fieldType(const String &fieldname);
		virtual AssocArray	fetchArray();
		virtual void		fetchArray(AssocArray &array);
		virtual Array		fetchFields();
		virtual void		fetchFields(Array &array);
		virtual void		nextRow();
		virtual bool		eof();
};


Postgres92Result::Postgres92Result()
{
	res=NULL;
	postgres_class=NULL;
	conn=NULL;
	result_rows=0;
	affectedrows=0;
	num_fields=0;
}

Postgres92Result::~Postgres92Result()
{
	clear();
}

void Postgres92Result::clear()
{
	result_rows=0;
	if (conn) {
		while (res) {
			PQclear(res);
			res=PQgetResult(conn);
		}
	}
	res=NULL;
	postgres_class=NULL;
	conn=NULL;
	affectedrows=0;
	num_fields=0;
}


ppluint64 Postgres92Result::rows() const
{
	return result_rows;
}

ppluint64 Postgres92Result::affected() const
{
	return affectedrows;
}

int Postgres92Result::fields() const
{
	return num_fields;
}


int Postgres92Result::fieldNum(const String &fieldname)
{
	if (!res) throw NoResultException();
	if (fieldname.isEmpty()) throw IllegalArgumentException();
	int n=PQfnumber(res,(const char*) fieldname);
	if (n==-1) throw FieldNotInResultSetException(fieldname);
	return n;
}

String Postgres92Result::fieldName(int field)
{
	if (!res) throw NoResultException();
	if (field>num_fields) throw FieldNotInResultSetException("%d",field);
	const char *name=PQfname(res,field);
	if (!name) FieldNotInResultSetException("%d",field);
	return String(name);
}


ResultSet::FieldType Postgres92Result::fieldType(int field)
{
	if (field>num_fields) throw FieldNotInResultSetException("%d",field);
	Oid o=PQftype(res,field);
	switch (o) {
		default:
			return ResultSet::TYPE_UNKNOWN;
	}
	return ResultSet::TYPE_UNKNOWN;
}

ResultSet::FieldType Postgres92Result::fieldType(const String &fieldname)
{
	int num=fieldNum(fieldname);
	return fieldType(num);
}

AssocArray Postgres92Result::fetchArray()
{
	AssocArray a;
	fetchArray(a);
	return a;
}

void Postgres92Result::fetchArray(AssocArray &array)
{
	if (!res) throw NoResultException();
	if (PQntuples(res)==0) {
		PQclear(res);
		res=NULL;
		throw NoResultException();
	}
	array.clear();
	for(int i=0; i<num_fields; i++) {
		array.set(String(PQfname(res,i)),String(PQgetvalue(res,0,i)));
	}
	PQclear(res);
	res=PQgetResult(conn);
}


String Postgres92Result::getString(const String &fieldname)
{
	int num=fieldNum(fieldname);
	return getString(num);
}

String Postgres92Result::getString(int field)
{
	if (!res) throw NoResultException();
	if (PQntuples(res)==0) {
		PQclear(res);
		res=NULL;
		throw NoResultException();
	}
	if (field>num_fields) throw FieldNotInResultSetException("%d",field);
	return String(PQgetvalue(res,0,field));
}



Array Postgres92Result::fetchFields()
{
	Array a;
	fetchFields(a);
	return a;
}


void Postgres92Result::fetchFields(Array &array)
{
	if (!res) throw NoResultException();
	if (PQntuples(res)==0) {
		PQclear(res);
		res=NULL;
		throw NoResultException();
	}
	array.clear();
	const char *tmp;
	for(int i=0; i<num_fields; i++) {
		tmp=PQgetvalue(res,0,i);
		if (tmp) array.add(tmp);
		else array.add("");
	}
	PQclear(res);
	res=PQgetResult(conn);
}

void Postgres92Result::nextRow()
{
	if (!res) throw NoResultException();
	if (PQntuples(res)==0) {
		PQclear(res);
		res=NULL;
		throw NoResultException();
	}
	PQclear(res);
	res=PQgetResult(conn);
}

bool Postgres92Result::eof()
{
	if (res) {
		if (PQntuples(res)>0) return false;
	}
	return true;
}

#endif //TODO

#endif	// HAVE_SQLITE3


#ifdef TODO
/*!\class PostgreSQL
 * \ingroup PPLGroupDatabases
 * \brief Implementierung einer Postgres-Datenbank
 *
 * \header \#include <ppl7-db.h>
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




PostgreSQL::PostgreSQL()
{
	conn=NULL;
	affectedrows=0;
	transactiondepth=0;
}

PostgreSQL::~PostgreSQL()
{
#ifdef HAVE_POSTGRESQL
	if (conn) close();
#endif
}


void PostgreSQL::connect()
{
	Database::connect();
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
 * - \b timeout: Timeout für den Connect in Sekunden (optional)
 * - \b searchpath: Kommaseparierte Liste mit den Schemata, die in den Suchpfad
 *      aufgenommen werden sollen (optional)
 * \param params Ein Assoziatives Array mit den für den Connect erforderlichen Parameter.
 *
 * \exception OutOfMemoryException
 * \exception ConnectionFailedException
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_Example3
 * \until EOF
 *
 */
void PostgreSQL::connect(const AssocArray &params)
{
#ifndef HAVE_POSTGRESQL
	throw UnsupportedFeatureException("PostgreSQL");
#else
	if (conn) close();
	condata=params;
	String conninfo;
	if (params.exists("host")) conninfo.appendf("host=%s ",(const char*)params["host"].toString());
	if (params.exists("port")) conninfo.appendf("port=%s ",(const char*)params["port"].toString());
	if (params.exists("dbname")) conninfo.appendf("dbname='%s' ",(const char*)params["dbname"].toString());
	if (params.exists("user")) conninfo.appendf("user='%s' ",(const char*)params["user"].toString());
	if (params.exists("password")) conninfo.appendf("password='%s' ",(const char*)params["password"].toString());
	if (params.exists("timeout")) conninfo.appendf("connect_timeout='%s' ",(const char*)params["timeout"].toString());

	conn=PQconnectdb((const char *)conninfo);
	if (!conn) {
		throw OutOfMemoryException();
	}
	// Pruefen, ob auch wirklich eine Verbindung da ist
	if (PQstatus((PGconn*)conn) == CONNECTION_OK) {
		try {
			if (params.exists("searchpath")) {
				String SearchPath=escape(params["searchpath"]);
				if (SearchPath.notEmpty()) {
					execf("set search_path to %s",(const char*)SearchPath);
				}
			}
			updateLastUse();
		} catch (...) {
			PQfinish((PGconn*)conn);
			conn=NULL;
			throw;
		}
		return;
	}

	// Was war der Fehler?
	String err(PQerrorMessage((PGconn*)conn));
	PQfinish((PGconn*)conn);
	conn=NULL;
	throw ConnectionFailedException(err);
#endif
}


void PostgreSQL::connectCreate(const AssocArray &params)
{
	AssocArray a=params;
	a.remove("dbname");
	a.set("dbname","postgres");
	String dbname=params["dbname"];
	// Versuch auf die immer vorhandene Datenbank "postgres" zuzugreifen
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

void PostgreSQL::close()
{
#ifndef HAVE_POSTGRESQL
	throw UnsupportedFeatureException("PostgreSQL");
#else
	if (!conn) {
		return;
	}
	PQfinish((PGconn*)conn);
	conn=NULL;
#endif
}



void PostgreSQL::reconnect()
{
#ifndef HAVE_POSTGRESQL
	throw UnsupportedFeatureException("PostgreSQL");
#else
	if (conn) {
		PQreset((PGconn *)conn);
		if (PQstatus((PGconn*)conn) == CONNECTION_OK) {
			updateLastUse();
			return;
		}
	}
	// Hat nicht geklappt, wir versuchen einen normalen Connect
	close();
	AssocArray a=condata;
	connect(a);
#endif
}

void PostgreSQL::selectDB(const String &databasename)
{
#ifndef HAVE_POSTGRESQL
	throw UnsupportedFeatureException("PostgreSQL");
#else
	// Wir koennen die Datenbank nicht innerhalb der laufenden Verbindung wechseln
	// Daher bauen wir eine neue Verbindung auf
	if (!conn) throw NoConnectionException();
	AssocArray a;
	a=condata;
	a.set("dbname",databasename);
	PostgreSQL newDB;
	try {
		newDB.connect(a);
	} catch (...) {
		throw;
	}
	close();
	this->conn=newDB.conn;
	transactiondepth=0;
	condata.set("dbname",databasename);
	newDB.conn=NULL;
#endif
}

/*!\brief Query ausführen
 *
 * \descr
 * Dies ist eine interne Funktion, die einen Query an die Postgres-Datenbank schickt.
 * Schlägt dies fehl, weil die Verbindung zur Datenbank zwischenzeitlich verloren ging,
 * wird ein Reconnect versucht und bei Erfolg der Query wiederholt.
 *
 * \param[in] query String mit dem abzusetzenden Query
 *
 * \return Konnte der Query erfolgreich ausgeführt werden, liefert die Funktion ein
 * Postgres-spezifisches Result-Handle vom Typ PGresult* zurück. Im Fehlerfall wird
 * eine Exception geworfen.
 *
 * \exception QueryFailedException
 *
 */
void *PostgreSQL::pgsqlQuery(const String &query)
{
#ifndef HAVE_POSTGRESQL
	throw UnsupportedFeatureException("PostgreSQL");
#else
	PGresult *res=NULL;
	ExecStatusType status;
	int count=0;
	ppl7::String err;
	while (count<2) {
		count++;
		if (PQsendQuery((PGconn *)conn,(const char*)query)!=1) {
			throw QueryFailedException("PQsendQuery failed: %s",PQerrorMessage((PGconn*)conn));
		} else {
			if (PQsetSingleRowMode((PGconn *)conn)!=1) {
				ppl7::String err;
				err.setf("PQsetSingleRowMode failed: %s",PQerrorMessage((PGconn*)conn));
				if (res) PQclear(res);
				throw QueryFailedException(err);
			}
			res=PQgetResult((PGconn *)conn);
			status=PQresultStatus(res);
			if (status==PGRES_COMMAND_OK
					|| status==PGRES_SINGLE_TUPLE
					|| status==PGRES_TUPLES_OK ) {
				affectedrows=atoll(PQcmdTuples(res));
				return res;
			} else if (status==PGRES_FATAL_ERROR) {
				err.setf("%s",PQresultErrorMessage(res));
				/* Laut Doku: Even when PQresultStatus indicates a fatal error, PQgetResult should
				 * be called until it returns a null pointer, to allow libpq to process the error
				 * information completely.
				 */
				while ((res=PQgetResult((PGconn *)conn))!=NULL) {
					PQclear(res);
					res=NULL;
				}
				throw QueryFailedException(err);
			} else {
				err.setf("%s",PQresultErrorMessage(res));
			}
		}
		if (res) PQclear(res);
		res=NULL;
		if (PQstatus((PGconn*)conn) != CONNECTION_OK) {
			reconnect();
		} else break;
	}
	throw QueryFailedException(err);
#endif
}


void PostgreSQL::exec(const String &query)
{
#ifndef HAVE_POSTGRESQL
	throw UnsupportedFeatureException("PostgreSQL");
#else
	if (!conn) throw NoConnectionException();
	double t_start;
	PGresult *res;
	t_start=GetMicrotime();
	res=(PGresult*)pgsqlQuery(query);
	updateLastUse();
	if (res) {
		// Result-Handle freigeben
		PQclear(res);
		while ((res=PQgetResult((PGconn *)conn))!=NULL) {
			PQclear(res);
		}
		logQuery(query,(float)(GetMicrotime()-t_start));
		return;
	}
	throw QueryFailedException();
#endif
}


ResultSet *PostgreSQL::query(const String &query)
{
#ifndef HAVE_POSTGRESQL
	throw UnsupportedFeatureException("PostgreSQL");
#else
	if (!conn) throw NoConnectionException();
	double t_start;
	PGresult *res;
	t_start=GetMicrotime();
	res=(PGresult*)pgsqlQuery(query);
	updateLastUse();

	// Query loggen
	logQuery(query,(float)(GetMicrotime()-t_start));
	// Result-Klasse erstellen
	Postgres92Result *pr=new Postgres92Result;
	if (!pr) {
		PQclear(res);
		throw OutOfMemoryException();
	}
	pr->res=res;
	pr->postgres_class=this;
	pr->conn=(PGconn *)conn;
	pr->result_rows=PQntuples(res);
	pr->affectedrows=affectedrows;
	pr->num_fields=PQnfields(res);
	return pr;
#endif
}


bool PostgreSQL::ping()
{
#ifndef HAVE_POSTGRESQL
	return false;
#else
	if (!conn) return false;
	PGresult *res;
	try {
		res=(PGresult*)pgsqlQuery("select 1 as result");
		if (res) {
			// Result-Handle freigeben
			PQclear(res);
			return true;
		}
	} catch (...) {
		return false;
	}
	return false;
#endif
}

String PostgreSQL::escape(const String &str) const
{
#ifndef HAVE_POSTGRESQL
	throw UnsupportedFeatureException("PostgreSQL");
#else
	if (!conn) throw NoConnectionException();
	size_t l=str.size()*2+1;
	char *buf=(char *)malloc(l);   // Buffer reservieren
	if (!buf) {
		throw OutOfMemoryException();
	}
	int error;
	size_t newlength=PQescapeStringConn((PGconn*)conn,buf,(const char*)str,str.size(),&error);
	if (error==0) {
		//ppl6::HexDump(buf,newlength);
		String ret(buf,newlength);
		free(buf);
		return ret;
	}
	free(buf);
	throw EscapeFailedException("%s",PQerrorMessage((PGconn*)conn));
#endif
}

ppluint64 PostgreSQL::getAffectedRows()
{
	return affectedrows;
}

void PostgreSQL::startTransaction()
{
#ifndef HAVE_POSTGRESQL
	throw UnsupportedFeatureException("PostgreSQL");
#else
	if (transactiondepth==0) {	// Neue Transaktion
		exec("BEGIN");
		transactiondepth++;
	} else {
		execf("SAVEPOINT LEVEL%i",transactiondepth);
		transactiondepth++;
	}
#endif
}

void PostgreSQL::endTransaction()
{
#ifndef HAVE_POSTGRESQL
	throw UnsupportedFeatureException("PostgreSQL");
#else
	if (transactiondepth==1) {
		exec("COMMIT");
		transactiondepth=0;
	} else {
		execf("RELEASE SAVEPOINT LEVEL%i",transactiondepth-1);
		transactiondepth--;
	}
#endif
}

void PostgreSQL::cancelTransaction()
{
#ifndef HAVE_POSTGRESQL
	throw UnsupportedFeatureException("PostgreSQL");
#else
	if (transactiondepth==1) {
		exec("ROLLBACK");
		transactiondepth=0;
	} else {
		execf("ROLLBACK TO SAVEPOINT LEVEL%i",transactiondepth-1);
		transactiondepth--;
	}
#endif
}

void PostgreSQL::cancelTransactionComplete()
{
#ifndef HAVE_POSTGRESQL
	throw UnsupportedFeatureException("PostgreSQL");
#else
	exec("ROLLBACK");
	transactiondepth=0;
#endif
}

void PostgreSQL::createDatabase(const String &name)
{
#ifndef HAVE_POSTGRESQL
	throw UnsupportedFeatureException("PostgreSQL");
#else
	throw UnsupportedFeatureException("PostgreSQL::createDatabase");
#endif
}

String PostgreSQL::databaseType() const
{
	return String("PostgreSQL");
}

String PostgreSQL::getQuoted(const String &value, const String &type) const
{
	String Type=type;
	String s=escape(value);
	Type.lowerCase();
	if (Type=="int" || Type=="integer") return s;
	if (Type=="bit" || Type=="boolean") return "'"+s+"'";
	return "'"+s+"'";
}

#endif

}	// EOF namespace db
}	// EOF namespace ppl7


