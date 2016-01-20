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
#include <sqlite3.h>
#endif

namespace ppl7 {
namespace db {

#ifdef HAVE_SQLITE3

class SQLiteResult : public ResultSet
{
	friend class SQLite;
	private:
		sqlite3			*conn;		//!\brief SQLite-spezifisches Handle des Datenbank-Connects
		sqlite3_stmt	*stmt;		//!\brief SQLite-spezifisches Result-Handle
		SQLite			*sqlite_class;		//!\brief Pointer auf die SQLite-Klassem die diesen Result erzeugt hat
		ppluint64	affectedrows;	//!\brief Falls es sich um ein Update/Insert/Replace handelte, steht hier die Anzahl betroffender Datensätze
		int			num_fields;		//!\brief Anzahl Spalten im Ergebnis
		int			last_res;		//!\brief letzter Returncode von sqlite3_step()

	public:
		SQLiteResult();
		virtual ~SQLiteResult();
		virtual	void		clear();
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

/*!\class SQLiteResult
 * \ingroup PPLGroupDatabases
 * \brief Klasse, die das Ergebnis eines SQLite-Results aufnimmt
 *
 * \descr
 * Dies ist eine interne Klasse des SQLite-Datenbankmoduls. Sie ist abgeleitet von ppl6::db::Result
 * und nimmt das Ergebnis eines Datenbank-Selects auf. Mit Ihren Funktionen kann anschließend
 * auf das Ergebnis zugegriffen werden.
 *
 * Die Klasse wird durch Aufruf verschiedener Funktionen innerhalb der Klasse ppl6::db::SQLite erstellt
 * und muss von der aufrufenden Anwendung selbst mit \c delete gelöscht werden.
 *
 */

SQLiteResult::SQLiteResult()
{
	conn=NULL;
	stmt=NULL;
	sqlite_class=NULL;
	affectedrows=0;
	num_fields=0;
	last_res=0;
}

SQLiteResult::~SQLiteResult()
{
	clear();
}

void SQLiteResult::clear()
{
	if (stmt) sqlite3_finalize(stmt);
	stmt=NULL;
	sqlite_class=NULL;
	conn=NULL;
	affectedrows=0;
	num_fields=0;
	last_res=0;
}

ppluint64 SQLiteResult::affected() const
{
	return affectedrows;
}

int SQLiteResult::fields() const
{
	return num_fields;
}

int SQLiteResult::fieldNum(const String &fieldname)
{
	if (!stmt) throw NoResultException();
	if (fieldname.isEmpty()) throw IllegalArgumentException();
	if (last_res!=SQLITE_ROW) throw NoResultException();
	for (int i=0;i<num_fields;i++) {
		if (fieldname.strcmp(sqlite3_column_name(stmt,i))==0) {
			return i;
		}
	}
	throw FieldNotInResultSetException(fieldname);
}

String SQLiteResult::fieldName(int field)
{
	if (!stmt) throw NoResultException();
	if (field>num_fields) throw FieldNotInResultSetException("%d",field);
	if (last_res!=SQLITE_ROW) throw NoResultException();
	const char *name=sqlite3_column_name(stmt,field);
	if (!name) FieldNotInResultSetException("%d",field);
	return String(name);
}

ResultSet::FieldType SQLiteResult::fieldType(int field)
{
	if (field>num_fields) throw FieldNotInResultSetException("%d",field);
	if (!stmt) throw NoResultException();
	if (last_res!=SQLITE_ROW) throw NoResultException();
	int type=sqlite3_column_type(stmt, field);
	switch (type) {
		case SQLITE_INTEGER: return ResultSet::TYPE_INTEGER;
		case SQLITE_FLOAT: return ResultSet::TYPE_FLOAT;
		case SQLITE_BLOB: return ResultSet::TYPE_BINARY;
		case SQLITE_NULL: return ResultSet::TYPE_INTEGER;
		case SQLITE_TEXT: return ResultSet::TYPE_STRING;
	}
	return ResultSet::TYPE_UNKNOWN;
}

ResultSet::FieldType SQLiteResult::fieldType(const String &fieldname)
{
	int num=fieldNum(fieldname);
	return fieldType(num);
}


AssocArray SQLiteResult::fetchArray()
{
	AssocArray a;
	fetchArray(a);
	return a;
}

void SQLiteResult::fetchArray(AssocArray &array)
{
	if (!stmt) throw NoResultException();
	if (last_res!=SQLITE_ROW) throw NoResultException();

	array.clear();
	for(int i=0; i<num_fields; i++) {
		const char *name=sqlite3_column_name(stmt,i);
		const unsigned char *value=sqlite3_column_text(stmt,i);
		int length=sqlite3_column_bytes(stmt,i);
		array.set(String(name),String((const char*)value,length));
	}
	last_res=sqlite3_step(stmt);
}

String SQLiteResult::getString(const String &fieldname)
{
	int num=fieldNum(fieldname);
	return getString(num);
}

String SQLiteResult::getString(int field)
{
	if (!stmt) throw NoResultException();
	if (last_res!=SQLITE_ROW) throw NoResultException();
	if (field>num_fields) throw FieldNotInResultSetException("%d",field);
	return String((const char*)sqlite3_column_text(stmt,field),sqlite3_column_bytes(stmt,field));
}

Array SQLiteResult::fetchFields()
{
	Array a;
	fetchFields(a);
	return a;
}

void SQLiteResult::fetchFields(Array &array)
{
	if (!stmt) throw NoResultException();
	if (last_res!=SQLITE_ROW) throw NoResultException();

	for(int i=0; i<num_fields; i++) {
		const char *value=(const char *)sqlite3_column_text(stmt,i);
		int length=sqlite3_column_bytes(stmt,i);
		array.add(String(value,length));
	}
	last_res=sqlite3_step(stmt);
}

void SQLiteResult::nextRow()
{
	if (!stmt) throw NoResultException();
	if (last_res!=SQLITE_ROW) throw NoResultException();
	last_res=sqlite3_step(stmt);
}

bool SQLiteResult::eof()
{
	if (stmt) {
		if (last_res==SQLITE_ROW) return false;
	}
	return true;
}


#endif	// HAVE_SQLITE3


/*!\class SQLite
 * \ingroup PPLGroupDatabases
 * \brief Implementierung einer SQLite-Datenbank
 *
 * \header \#include <ppl6-db.h>
 *
 * \descr
 * Mit dieser Klasse kann aufe eine lokale Datei mit einer SQLite-Datenbank
 * zugegriffen werden
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_SQLite_Example1
 * \until EOF
 */




SQLite::SQLite()
{
	conn=NULL;
	affectedrows=0;
	transactiondepth=0;
}

SQLite::~SQLite()
{
#ifdef HAVE_SQLITE3
	if (conn) close();
#endif
}

void SQLite::connect()
{
	Database::connect();
}

/*!\brief Connect auf eine SQLite-Datenbank erstellen
 *
 * \descr
 * Mit dieser Funktion wird eine Verbindung zu einer SQLite Datenbank hergestellt, wobei
 * die dafür notwendigen Parameter dem Array \p params entnommen werden.
 * \par
 * Mögliche Parameter:
 * - \b filename: Dateiname der Datenbank
 *
 * \param params Ein Assoziatives Array mit den für den Connect erforderlichen Parameter.
 *
 * \exception OutOfMemoryException
 * \exception ConnectionFailedException
 *
 */
void SQLite::connect(const AssocArray &params)
{
#ifndef HAVE_SQLITE3
	throw UnsupportedFeatureException("SQLite");
#else
	if (conn) close();
	condata=params;
	String filename=params["filename"];
	int ret=sqlite3_open((const char*)filename,(sqlite3 **)&conn);
	if (!conn) throw OutOfMemoryException();
	if (ret != SQLITE_OK) {
		String err(sqlite3_errmsg((sqlite3*)conn));
		sqlite3_close((sqlite3*)conn);
		conn=NULL;
		throw ConnectionFailedException(err);
	}
#endif
}

void SQLite::connectCreate(const AssocArray &params)
{
	connect(params);
}

void SQLite::close()
{
#ifndef HAVE_SQLITE3
	throw UnsupportedFeatureException("SQLite3");
#else
	if (!conn) {
		return;
	}
	sqlite3_close((sqlite3*)conn);
	conn=NULL;
#endif
}

void SQLite::reconnect()
{
#ifndef HAVE_SQLITE3
	throw UnsupportedFeatureException("SQLite");
#else
	close();
	AssocArray a=condata;
	connect(a);
#endif
}


void SQLite::selectDB(const String &databasename)
{
#ifndef HAVE_SQLITE3
	throw UnsupportedFeatureException("SQLite");
#else
	throw UnsupportedFeatureException("SQLite:selectDB");
#endif
}

void SQLite::exec(const String &query)
{
#ifndef HAVE_SQLITE3
	throw UnsupportedFeatureException("SQLite");
#else
	if (!conn) throw NoConnectionException();
	String err;
	double t_start;
	affectedrows=0;
	sqlite3_stmt *stmt=NULL;
	t_start=GetMicrotime();
	int ret=sqlite3_prepare_v2((sqlite3*)conn, (const char*)query, query.size(),&stmt,NULL);
	if (ret!=SQLITE_OK) {
		throw QueryFailedException("sqlite3_prepare_v2 failed: %s",sqlite3_errmsg((sqlite3*)conn));
	}
	if (stmt==NULL) {
		throw OutOfMemoryException();
	}
	ret=sqlite3_step(stmt);
	if (ret!=SQLITE_DONE && ret!=SQLITE_ROW) {
		err.setf("sqlite3_step: %s, Query: %s",sqlite3_errmsg((sqlite3*)conn),(const char*)query);
		sqlite3_finalize(stmt);
		throw QueryFailedException(err);
	}
	ret=sqlite3_finalize(stmt);
	if (ret !=SQLITE_OK) {
		err.setf("sqlite3_finalize: %s, Query: %s",sqlite3_errmsg((sqlite3*)conn),(const char*)query);
		throw QueryFailedException(err);
	}
	affectedrows=sqlite3_changes((sqlite3*)conn);
	updateLastUse();
	logQuery(query,(float)(GetMicrotime()-t_start));
#endif
}

ResultSet *SQLite::query(const String &query)
{
#ifndef HAVE_SQLITE3
	throw UnsupportedFeatureException("SQLite");
#else
	if (!conn) throw NoConnectionException();
	String err;
	double t_start;
	affectedrows=0;
	sqlite3_stmt *stmt=NULL;
	t_start=GetMicrotime();
	int ret=sqlite3_prepare_v2((sqlite3*)conn, (const char*)query, query.size(),&stmt,NULL);
	if (ret!=SQLITE_OK) {
		throw QueryFailedException("sqlite3_prepare_v2 failed: %s",sqlite3_errmsg((sqlite3*)conn));
	}
	if (stmt==NULL) {
		throw OutOfMemoryException();
	}
	ret=sqlite3_step(stmt);
	if (ret!=SQLITE_DONE && ret!=SQLITE_ROW) {
		err.setf("sqlite3_step: %s, Query: %s",sqlite3_errmsg((sqlite3*)conn),(const char*)query);
		sqlite3_finalize(stmt);
		throw QueryFailedException(err);
	}
	affectedrows=sqlite3_changes((sqlite3*)conn);
	updateLastUse();
	logQuery(query,(float)(GetMicrotime()-t_start));

	SQLiteResult *pr=new SQLiteResult;
	if (!pr) {
		sqlite3_finalize(stmt);
		throw OutOfMemoryException();
	}
	pr->stmt=stmt;
	pr->last_res=ret;
	pr->sqlite_class=this;
	pr->conn=(sqlite3 *)conn;
	pr->affectedrows=affectedrows;
	pr->num_fields=sqlite3_column_count(stmt);
	return pr;
#endif
}

bool SQLite::ping()
{
#ifndef HAVE_SQLITE3
	return false;
#else
	if (conn) return true;
	return false;
#endif
}


String SQLite::escape(const String &str) const
{
#ifndef HAVE_SQLITE3
	throw UnsupportedFeatureException("SQLite");
#else
	// SQLite hat keine Escape-Funktion, daher müssen wir das selbst machen
	String n;
	const char *tmp=str.getPtr();
	int c;
	while ((c=tmp[0])) {
		if (c=='\'') n+="'";
		n+=c;
		tmp++;
	}
	return n;
#endif
}

ppluint64 SQLite::getAffectedRows()
{
	return affectedrows;
}

void SQLite::startTransaction()
{
#ifndef HAVE_SQLITE3
	throw UnsupportedFeatureException("SQLite");
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

void SQLite::endTransaction()
{
#ifndef HAVE_SQLITE3
	throw UnsupportedFeatureException("SQLite");
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

void SQLite::cancelTransaction()
{
#ifndef HAVE_SQLITE3
	throw UnsupportedFeatureException("SQLite");
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

void SQLite::cancelTransactionComplete()
{
#ifndef HAVE_SQLITE3
	throw UnsupportedFeatureException("SQLite");
#else
	exec("ROLLBACK");
	transactiondepth=0;
#endif
}

void SQLite::createDatabase(const String &name)
{
#ifndef HAVE_SQLITE3
	throw UnsupportedFeatureException("SQLite");
#else
	throw UnsupportedFeatureException("SQLite::createDatabase");
#endif
}

String SQLite::databaseType() const
{
	return String("SQLite");
}

String SQLite::getQuoted(const String &value, const String &type) const
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


