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
#include "threads.h"

#ifdef HAVE_MYSQL
#ifdef MINGW32
#define __LCC__
#endif
#include <mysql.h>
#include <errmsg.h>
#endif

namespace ppl7 {
namespace db {

#ifdef HAVE_MYSQL

static Mutex MySQLGlobalMutex;
static int __mysql_init=0;


static inline void pplMySQLThreadStart()
{
	THREADDATA * d=GetThreadData();
	if (d->mysql_thread_end) return;
	if (mysql_thread_init()==0) {
		d->mysql_thread_end=mysql_thread_end;
		return;
	}
	throw InitializationFailedException("MySQL [mysql_thread_init]");
}


class MySQLResult : public ResultSet
{
	friend class MySQL;
	private:
		MYSQL_RES	*res;			//!\brief MySQL-spezifisches Result-Handle
		MYSQL_ROW	row;			//!\brief Pointer auf die aktuelle Result-Zeile
		MYSQL		*conn;			//!\brief MySQL-spezifisches Handle des Datenbank-Connects, das den Result erzeugt hat
		MySQL		*mysql_class;	//!\brief Die ppl7::db::MySQL-Klasse, die das Result erzeugt hat
		ppluint64	affectedrows;	//!\brief Falls es sich um ein Update/Insert/Replace handelte, steht hier die Anzahl betroffender Datensätze
		int			num_fields;		//!\brief Anzahl Spalten im Ergebnis

	public:
		MySQLResult();
		virtual ~MySQLResult();
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


MySQLResult::MySQLResult()
{
	res=NULL;
	row=NULL;
	mysql_class=NULL;
	conn=NULL;
	affectedrows=0;
	num_fields=0;
}

MySQLResult::~MySQLResult()
{
	clear();
}

void MySQLResult::clear()
{
	if (res) mysql_free_result(res);
	res=NULL;
	row=NULL;
	mysql_class=NULL;
	conn=NULL;
	affectedrows=0;
	num_fields=0;
}

ppluint64 MySQLResult::affected() const
{
	return affectedrows;
}

int MySQLResult::fields() const
{
	return num_fields;
}

int MySQLResult::fieldNum(const String &fieldname)
{
	if (!res) throw NoResultException();
	if (fieldname.isEmpty()) throw IllegalArgumentException();
	pplMySQLThreadStart();

	// Das Feld suchen
	MYSQL_FIELD *field;
	field = mysql_fetch_fields(res);
	const char *searchname=(const char*)fieldname;
	for(int i=0; i<num_fields; i++) {
		if (strcmp(field[i].name,searchname)==0) {
			return i;
		}
	}
	throw FieldNotInResultSetException(fieldname);
}

String MySQLResult::fieldName(int field)
{
	if (!res) throw NoResultException();
	if (field>num_fields) throw FieldNotInResultSetException("%d",field);
	pplMySQLThreadStart();

	MYSQL_FIELD *mf;
	mf = mysql_fetch_fields(res);
	return String(mf[field].name);
}

ResultSet::FieldType MySQLResult::fieldType(int field)
{
	if (!res) throw NoResultException();
	if (field>num_fields) throw FieldNotInResultSetException("%d",field);
	pplMySQLThreadStart();
	MYSQL_FIELD *mf;
	mf = mysql_fetch_fields(res);
	switch (mf[field].type) {
#ifdef FIELD_TYPE_TINY
		case MYSQL_TYPE_TINY:
			return ResultSet::TYPE_INTEGER;
#endif
#ifdef FIELD_TYPE_SHORT
		case MYSQL_TYPE_SHORT:
			return ResultSet::TYPE_INTEGER;
#endif
#ifdef FIELD_TYPE_LONG
		case MYSQL_TYPE_LONG:
			return ResultSet::TYPE_INTEGER;
#endif
#ifdef FIELD_TYPE_INT24
		case MYSQL_TYPE_INT24:
			return ResultSet::TYPE_INTEGER;
#endif
#ifdef FIELD_TYPE_LONGLONG
		case MYSQL_TYPE_LONGLONG:
			return ResultSet::TYPE_INTEGER;
#endif
#ifdef FIELD_TYPE_DECIMAL
		case MYSQL_TYPE_DECIMAL:
			return ResultSet::TYPE_FLOAT;
#endif
#ifdef  FIELD_TYPE_NEWDECIMAL
		case MYSQL_TYPE_NEWDECIMAL:
			return ResultSet::TYPE_FLOAT;
#endif
#ifdef FIELD_TYPE_FLOAT
		case MYSQL_TYPE_FLOAT:
			return ResultSet::TYPE_FLOAT;
#endif
#ifdef FIELD_TYPE_DOUBLE
		case MYSQL_TYPE_DOUBLE:
			return ResultSet::TYPE_DOUBLE;
#endif
#ifdef FIELD_TYPE_BIT
		case MYSQL_TYPE_BIT:
			return ResultSet::TYPE_BOOLEAN;
#endif
#ifdef FIELD_TYPE_TIMESTAMP
		case MYSQL_TYPE_TIMESTAMP:
			return ResultSet::TYPE_DATETIME;
#endif
#ifdef FIELD_TYPE_DATE
		case MYSQL_TYPE_DATE:
			return ResultSet::TYPE_DATETIME;
#endif
#ifdef FIELD_TYPE_YEAR
		case MYSQL_TYPE_YEAR:
			return ResultSet::TYPE_DATETIME;
#endif
#ifdef FIELD_TYPE_TIME
		case MYSQL_TYPE_TIME:
			return ResultSet::TYPE_DATETIME;
#endif
#ifdef FIELD_TYPE_DATETIME
		case MYSQL_TYPE_DATETIME:
			return ResultSet::TYPE_DATETIME;
#endif
#ifdef FIELD_TYPE_STRING
		case MYSQL_TYPE_STRING:
			return ResultSet::TYPE_STRING;
#endif
#ifdef FIELD_TYPE_VAR_STRING
		case MYSQL_TYPE_VAR_STRING:
			return ResultSet::TYPE_STRING;
#endif
#if MYSQL_TYPE_VARCHAR > 0
		case MYSQL_TYPE_VARCHAR:
			return ResultSet::TYPE_STRING;
#endif
#ifdef FIELD_TYPE_BLOB
		case MYSQL_TYPE_BLOB:
			return ResultSet::TYPE_BINARY;
#endif
		default:
			return ResultSet::TYPE_UNKNOWN;
	}
	return ResultSet::TYPE_UNKNOWN;
}

ResultSet::FieldType MySQLResult::fieldType(const String &fieldname)
{
	int num=fieldNum(fieldname);
	return fieldType(num);
}

AssocArray MySQLResult::fetchArray()
{
	AssocArray a;
	fetchArray(a);
	return a;
}

void MySQLResult::fetchArray(AssocArray &array)
{
	if (!res) throw NoResultException();
	if (!row) throw NoResultException();
	pplMySQLThreadStart();

	MYSQL_FIELD *fields;
	fields = mysql_fetch_fields(res);
	if (!fields) throw NoResultException();
	array.clear();
	for(int i=0; i<num_fields; i++) {
		if (row[i]) array.set(String(fields[i].name),String(row[i]));
		else array.set(String(fields[i].name),String(""));
	}
	// Nächste Zeile holen
	row=mysql_fetch_row(res);
}

String MySQLResult::getString(const String &fieldname)
{
	int num=fieldNum(fieldname);
	return getString(num);
}

String MySQLResult::getString(int field)
{
	if (!res) throw NoResultException();
	if (!row) throw NoResultException();
	if (field>num_fields) throw FieldNotInResultSetException("%d",field);

	if (row[field]) return String(row[field]);
	return String();
}

Array MySQLResult::fetchFields()
{
	Array a;
	fetchFields(a);
	return a;
}


void MySQLResult::fetchFields(Array &array)
{
	if (!res) throw NoResultException();
	if (!row) throw NoResultException();
	pplMySQLThreadStart();

	MYSQL_FIELD *fields;
	fields = mysql_fetch_fields(res);
	if (!fields) throw NoResultException();
	array.clear();
	for(int i=0; i<num_fields; i++) {
		if (row[i]) array.add(String(row[i]));
		else array.add(String(""));
	}
	// Nächste Zeile holen
	row=mysql_fetch_row(res);
}

void MySQLResult::nextRow()
{
	if (!res) throw NoResultException();
	if (!row) throw NoResultException();
	pplMySQLThreadStart();
	row=mysql_fetch_row(res);
	if (!row) throw NoResultException();
}

bool MySQLResult::eof()
{
	if (res) {
		if (row) return false;
	}
	return true;
}


#endif	// HAVE_MYSQL


/*!\class MySQL
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




MySQL::MySQL()
{
#ifdef HAVE_MYSQL
	MySQL::libraryInit();
	pplMySQLThreadStart();
#endif
	conn=NULL;
	affectedrows=0;
	transactiondepth=0;
}

MySQL::~MySQL()
{
#ifdef HAVE_MYSQL
	if (conn) close();
#endif
}

void MySQL::libraryInit(int argc, char **argv, char **groups)
{
#ifdef HAVE_MYSQL
	if (__mysql_init==1) return;
	MySQLGlobalMutex.lock();
	if (__mysql_init==0) {
		if (mysql_library_init(argc, argv, groups)==0) {
			if (atexit(mysql_library_end)!=0) {
				mysql_library_end();
				MySQLGlobalMutex.unlock();
				throw InitializationFailedException("MySQL::libraryInit [atexit]");
			}
			__mysql_init=1;
			MySQLGlobalMutex.unlock();
			return;
		}
	}
	MySQLGlobalMutex.unlock();
	throw InitializationFailedException("MySQL::libraryInit [mysql_library_init]");
#else
	throw UnsupportedFeatureException("MySQL");
#endif
}


void MySQL::connect()
{
	Database::connect();
}

/*!\brief Connect auf eine MySQL-Datenbank erstellen
 *
 * \descr
 * Mit dieser Funktion wird eine Verbindung zu einem MySQL-Datenbank-Server hergestellt, wobei
 * die dafür notwendigen Parameter dem Array \p params entnommen werden.
 *
 * \param[in] params Ein Assoziatives Array mit den für den Connect erforderlichen Parameter.
 * Folgende Paramater sind anzugeben:
 * - \b host: Der Hostname oder die IP-Adresse des Datenbank-Servers
 * - \b port: Der TCP-Port des Datenbank-Servers
 * - \b dbname: Der Name der intialen Datenbank. Dieser Parameter kann optional sein,
 *   da mit der Funktion Database::SelectDB die Datenbank auch später noch gewechselt werden kann.
 * - \b user: Der Name des Benutzers, mit dem sich an der Datenbank authentifiziert werden soll
 * - \b password: Das Passwort des Benutzers im Klartext
 * - \b charset: Die zu verwendende Zeichensatz-Kodierung. Wird dieser Parameter nicht angegeben,
 *   wird defaultmäßig "latin1" verwendet, was auch bei MySQL der Default ist, sofern bei der
 *   Tabellendefinition nichts anderes angegeben wurde.
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_Example3
 * \until EOF
 *
 */
void MySQL::connect(const AssocArray &params)
{
#ifndef HAVE_MYSQL
	throw UnsupportedFeatureException("MySQL");
#else
	if (conn) close();
	pplMySQLThreadStart();
	condata=params;
	conn=mysql_init(NULL);					// MySQL-Handle erzeugen
	if (!conn) throw ConnectionFailedException("mysql_init");
	MYSQL *result=mysql_real_connect((MYSQL *)conn,
			(const char*)params["host"].toString(),
			(const char*)params["user"].toString(),
			(const char*)params["password"].toString(),
			(const char*)params["dbname"].toString(),
			params["port"].toString().toInt(),
			NULL,0);
	if (result!=NULL) {
		updateLastUse();
		String charset="latin1";
		if (params.exists("charset")) {
			charset=params["charset"];
		}
		int ret=mysql_set_character_set((MYSQL *)conn, (const char*)charset);
		if (ret!=0) {
			String err;
			err.setf("mysql_set_character_set %s => %i: %s",
					(const char*)charset,
					mysql_errno((MYSQL *)conn),mysql_error((MYSQL *)conn));
			try {
				close();
			} catch (...) {

			}
			throw CharacterEncodingException(err);
		}
		return;
	}
	// Was war der Fehler?
	String err;
	err.setf("MySQL-Error: %d: %s, Host: %s:%d, User: %s, Database: %s",
			mysql_errno((MYSQL *)conn),
			mysql_error((MYSQL *)conn),
			(const char*) params["host"].toString(),
			params["port"].toString().toInt(),
			(const char*) params["user"].toString(),
			(const char*) params["dbname"].toString());
	mysql_close((MYSQL *)conn);
	clearLastUse();
	conn=NULL;
	throw ConnectionFailedException(err);
#endif
}

void MySQL::connectCreate(const AssocArray &params)
{
	AssocArray a=params;
	a.remove("dbname");
	a.set("dbname","mysql");
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


void MySQL::close()
{
#ifndef HAVE_MYSQL
	throw UnsupportedFeatureException("MySQL");
#else
	if (!conn) {
		return;
	}
	pplMySQLThreadStart();
	mysql_close((MYSQL *)conn);
	conn=NULL;
	clearLastUse();
#endif
}

void MySQL::reconnect()
{
#ifndef HAVE_MYSQL
	throw UnsupportedFeatureException("MySQL");
#else
	if (conn) {
		pplMySQLThreadStart();
		if (mysql_ping((MYSQL *)conn)==0) return;
	}
	close();
	AssocArray a=condata;
	connect(a);
#endif
}

void MySQL::selectDB(const String &databasename)
{
#ifndef HAVE_MYSQL
	throw UnsupportedFeatureException("MySQL");
#else
	if (!conn) throw NoConnectionException();
	pplMySQLThreadStart();
	if ( mysql_select_db((MYSQL *)conn, (const char*)databasename)==0 ) {
		updateLastUse();
		return;
	}
	int e=mysql_errno((MYSQL *)conn);
	if (e==CR_SERVER_GONE_ERROR || e==CR_SERVER_LOST) {
		reconnect();
		if ( mysql_select_db((MYSQL *)conn, (const char*) databasename)==0 ) {
			updateLastUse();
			return;
		}
	}
	// Was war der Fehler?
	String err;
	err.setf("MySQL-Error: %u, %s",mysql_errno((MYSQL *)conn),mysql_error((MYSQL *)conn));
	throw OperationFailedException(err);
#endif
}

/*!\brief Query ausführen
 *
 * \descr
 * Dies ist eine interne Funktion, die einen Query an die MySQL-Datenbank schickt.
 * Schlägt dies fehl, weil die Verbindung zur Datenbank zwischenzeitlich verloren ging,
 * wird ein Reconnect versucht und bei Erfolg der Query wiederholt. Die Klasse erwartet,
 * dass der Mutex bereits gelockt ist und die Variable MySQL::mysql ein gültiges MySQL-Handle
 * enthält.
 *
 * \param[in] query String mit dem abzusetzenden Query
 *
 * \return Konnte der Query erfolgreich ausgeführt werden, liefert die Funktion 1
 * zurück, im Fehlerfall 0. Der Mutex ist bei Verlassen der Funktion auf jeden Fall gesetzt.
 *
 */
void MySQL::mysqlQuery(const String &query)
{
#ifndef HAVE_MYSQL
	throw UnsupportedFeatureException("MySQL");
#else
	if (mysql_real_query((MYSQL *)conn, query.c_str(), query.size())==0) return;
	// Vielleicht ist die DB-Verbindung weg?
	int e=mysql_errno((MYSQL *)conn);
	if (e==CR_SERVER_GONE_ERROR || e==CR_SERVER_LOST) {
		reconnect();
		if (mysql_real_query((MYSQL *)conn, query.c_str(), query.size())==0) return;
	}
	// Was war der Fehler?
	String err;
	err.setf("MySQL-Error: %u, %s",mysql_errno((MYSQL *)conn),mysql_error((MYSQL *)conn));
	throw QueryFailedException(err);
#endif
}

void MySQL::exec(const String &query)
{
#ifndef HAVE_MYSQL
	throw UnsupportedFeatureException("MySQL");
#else
	if (!conn) throw NoConnectionException();
	pplMySQLThreadStart();
	double t_start;
	affectedrows=0;
	t_start=GetMicrotime();
	mysqlQuery(query);
	updateLastUse();
	affectedrows = mysql_affected_rows((MYSQL *)conn);
	//lastinsertid=(ppld64)mysql_insert_id((MYSQL *)conn);
	MYSQL_RES *res = mysql_store_result((MYSQL *)conn);
	logQuery(query,(float)(GetMicrotime()-t_start));
	if (res) {
		mysql_free_result(res);
		return;
	}
	if (mysql_errno((MYSQL *)conn)==0) {	// Query hat kein Ergebnis zurückgeliefert
		return;
	}
	throw QueryFailedException(query);
#endif
}

ResultSet *MySQL::query(const String &query)
{
#ifndef HAVE_MYSQL
	throw UnsupportedFeatureException("MySQL");
#else
	if (!conn) throw NoConnectionException();
	pplMySQLThreadStart();
	double t_start;
	affectedrows=0;
	t_start=GetMicrotime();
	mysqlQuery(query);
	updateLastUse();
	affectedrows = mysql_affected_rows((MYSQL *)conn);
	//lastinsertid=(ppld64)mysql_insert_id((MYSQL *)conn);
	MYSQL_RES *res = mysql_store_result((MYSQL *)conn);
	logQuery(query,(float)(GetMicrotime()-t_start));
	// Result-Klasse erstellen
	if (!res) {
		throw NoResultException(query);
	}
	MySQLResult *pr=new MySQLResult;
	if (!pr) {
		mysql_free_result(res);
		throw OutOfMemoryException();
	}
	pr->res=res;
	pr->row=mysql_fetch_row(res);
	pr->mysql_class=this;
	pr->conn=(MYSQL *)conn;
	pr->affectedrows=affectedrows;
	pr->num_fields=mysql_num_fields(res);
	return pr;
#endif
}

bool MySQL::ping()
{
#ifndef HAVE_MYSQL
	return false;
#else
	if (!conn) return false;
	pplMySQLThreadStart();
	if (mysql_ping((MYSQL *)conn)==0) {		// Server ist noch connected
		updateLastPing();
		return true;
	}
	return false;
#endif
}

String MySQL::escape(const String &str) const
{
#ifndef HAVE_MYSQL
	throw UnsupportedFeatureException("MySQL");
#else
	if (!conn) throw NoConnectionException();
	pplMySQLThreadStart();
	size_t l=str.size()*2+1;
	char *buf=(char *)malloc(l);   // Buffer reservieren
	if (!buf) {
		throw OutOfMemoryException();
	}
	size_t newlength=mysql_real_escape_string((MYSQL *)conn,buf,str.c_str(),(unsigned long)str.size());
	if (newlength!=(size_t)-1) {
		String ret(buf,newlength);
		free(buf);
		return ret;
	}
	free(buf);
	throw EscapeFailedException();
#endif
}

ppluint64 MySQL::getAffectedRows()
{
	return affectedrows;
}

void MySQL::startTransaction()
{
#ifndef HAVE_MYSQL
	throw UnsupportedFeatureException("MySQL");
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

void MySQL::endTransaction()
{
#ifndef HAVE_MYSQL
	throw UnsupportedFeatureException("MySQL");
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

void MySQL::cancelTransaction()
{
#ifndef HAVE_MYSQL
	throw UnsupportedFeatureException("MySQL");
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

void MySQL::cancelTransactionComplete()
{
#ifndef HAVE_MYSQL
	throw UnsupportedFeatureException("MySQL");
#else
	exec("ROLLBACK");
	transactiondepth=0;
#endif
}

void MySQL::createDatabase(const String &name)
{
#ifndef HAVE_MYSQL
	throw UnsupportedFeatureException("MySQL");
#else
	throw UnsupportedFeatureException("MySQL::createDatabase");
#endif
}

String MySQL::databaseType() const
{
	return String("MySQL");
}

String MySQL::getQuoted(const String &value, const String &type) const
{
	String Type=type;
	String s=escape(value);
	Type.lowerCase();
	if (Type=="int" || Type=="integer") return s;
	if (Type=="bit" || Type=="boolean") return "'"+s+"'";
	return "'"+s+"'";
}


}	// EOF namespace db
}	// EOF namespace ppl7


