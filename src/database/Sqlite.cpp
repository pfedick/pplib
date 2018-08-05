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
#include <winsock2.h>
#endif


#include "ppl6.h"
#include "ppl6-db.h"

#ifdef HAVE_SQLITE
#include <sqlite3.h>
#endif

namespace ppl6 {
//#include "thread.h"
namespace db {


#ifdef HAVE_SQLITE

class SQLiteResult : public Result
{
	friend class SQLite;
	private:

		sqlite3			*sql;		//!\brief Handle zur SQLite-Datenbank
		SQLite			*db;		//!\brief Pointer auf die SQLite-Klassem die diesen Result erzeugt hat
		sqlite3_stmt	*stmt;		//!\brief SQLite-spezifisches Result-Handle
		pplint64	rows;			//!\brief Anzahl Zeilen im Ergebnis
		pplint64 	lastinsertid;	//!\brief Falls es sich um einen Insert mit einem Autoincrement-Index handelte, steht hier die vergebene ID
		pplint64	affectedrows;	//!\brief Falls es sich um ein Update/Insert/Replace handelte, steht hier die Anzahl betroffender Datensätze
		int			num_fields;		//!\brief Anzahl Spalten im Ergebnis
		int			last_res;		//!\brief letzter Returncode von sqlite3_step()

	public:
		SQLiteResult();
		virtual ~SQLiteResult();
		virtual	void Clear();
		virtual pplint64 Rows();
		virtual pplint64 Affected();
		virtual int Fields();
		virtual int FieldNum(const char *fieldname);
		virtual const char *FieldName(int num);
		virtual int		FetchArray(CAssocArray &array, pplint64 row=-1);
		virtual CAssocArray  FetchArray(pplint64 row=-1);
		virtual Result::Type	FieldType(int num);
		virtual Result::Type	FieldType(const char *fieldname);
		virtual int		Seek(pplint64 row);
		virtual const char *Get(pplint64 row, const char *fieldname);
		virtual const char *Get(pplint64 row, int field);
		virtual CArray  FetchFields(pplint64 row=-1);
		virtual int		FetchFields(CArray &array, pplint64 row=-1);
		/*
		virtual void    PrintResult();
		*/
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
	sql=NULL;
	db=NULL;
	stmt=NULL;
	rows=0;
	lastinsertid=0;
	affectedrows=0;
	num_fields=0;
}

SQLiteResult::~SQLiteResult()
{
	Clear();
}

void SQLiteResult::Clear()
{
	if (stmt) sqlite3_finalize(stmt);
	sql=NULL;
	db=NULL;
	stmt=NULL;
	rows=0;
	lastinsertid=0;
	affectedrows=0;
	num_fields=0;
}

pplint64 SQLiteResult::Rows()
{
	return rows;
}

pplint64 SQLiteResult::Affected()
{
	return affectedrows;
}

int SQLiteResult::Fields()
{
	return num_fields;
}


int SQLiteResult::FieldNum(const char *fieldname)
{
	if (!stmt) {
		SetError(182);
		return 0;
	}
	if (!fieldname) {
		SetError(194);
		return 0;
	}
	if (last_res!=SQLITE_ROW) {
		SetError(300);
		return -1;
	}
	for (int i=0;i<num_fields;i++) {
		if (strcmp(fieldname,sqlite3_column_name(stmt,i))==0) {
			return i;
		}
	}
	SetError(183,"%s",fieldname);
	return -1;
}

const char *SQLiteResult::FieldName(int field)
{
	if (!stmt) {
		SetError(182);
		return 0;
	}
	if (last_res!=SQLITE_ROW) {
		SetError(300);
		return NULL;
	}

	if (field>=sqlite3_column_count(stmt)) {
		SetError(183,"%i",field);
		return NULL;
	}
	const char *name=sqlite3_column_name(stmt,field);
	if (!name) {
		SetError(183,"%i",field);
	}
	return name;
}

Result::Type SQLiteResult::FieldType(int field)
{
	if (!stmt) {
		SetError(182);
		return Error;
	}
	if (last_res!=SQLITE_ROW) {
		SetError(300);
		return Error;
	}
	int type=sqlite3_column_type(stmt, field);
	switch (type) {
		case SQLITE_INTEGER: return Integer;
		case SQLITE_FLOAT: return Decimal;
		case SQLITE_BLOB: return Binary;
		case SQLITE_NULL: return Integer;
		case SQLITE_TEXT: return String;
	}
	return Result::Unknown;
}

Result::Type SQLiteResult::FieldType(const char *fieldname)
{
	int field=FieldNum(fieldname);
	if (field<0) {
		return Error;
	}
	return FieldType(field);
}

int SQLiteResult::FetchArray(CAssocArray &array, pplint64 row)
{
	array.Clear();
	if (!stmt) {
		SetError(182);
		return 0;
	}
	if (row!=-1) {
		SetError(565,"SQLiteResult::Seek");
		return 0;
	}
	if (last_res!=SQLITE_ROW) {
		SetError(300);
		return 0;
	}

	// Felder holen
	for(int i=0; i<sqlite3_column_count(stmt); i++) {
		const char *name=sqlite3_column_name(stmt,i);
		const unsigned char *value=sqlite3_column_text(stmt,i);
		int length=sqlite3_column_bytes(stmt,i);
		array.Set(name,(const char*)value,length);
	}
	last_res=sqlite3_step(stmt);
	return 1;
}

CAssocArray SQLiteResult::FetchArray(pplint64 row)
{
	CAssocArray a;
	FetchArray(a,row);
	return a;
}

int SQLiteResult::Seek(pplint64 row)
{
	SetError(565,"SQLiteResult::Seek");
	return 0;
}

const char *SQLiteResult::Get(pplint64 row, const char *fieldname)
{
	int field=FieldNum(fieldname);
	if (field<0) return NULL;
	return Get(row,field);
}

const char *SQLiteResult::Get(pplint64 row, int field)
{
	if (!stmt) {
		SetError(182);
		return 0;
	}
	if (row!=-1) {
		SetError(565,"SQLiteResult::Seek");
		return 0;
	}
	if (last_res!=SQLITE_ROW) {
		SetError(300);
		return NULL;
	}

	if (field>=sqlite3_column_count(stmt)) {
		SetError(183,"%i",field);
		return NULL;
	}
	const unsigned char *value=sqlite3_column_text(stmt,field);
	return (const char*)value;
}

int SQLiteResult::FetchFields(CArray &array, pplint64 row)
{
	array.Clear();
	if (!stmt) {
		SetError(182);
		return 0;
	}
	if (row!=-1) {
		SetError(565,"SQLiteResult::Seek");
		return 0;
	}
	if (last_res!=SQLITE_ROW) {
		SetError(300);
		return 0;
	}

	for(int i=0; i<sqlite3_column_count(stmt); i++) {
		const unsigned char *value=sqlite3_column_text(stmt,i);
		int length=sqlite3_column_bytes(stmt,i);
		array.Add((const char*)value,length);
	}
	last_res=sqlite3_step(stmt);
	return 1;
}

CArray SQLiteResult::FetchFields(pplint64 row)
{
	CArray a;
	FetchFields(a,row);
	return a;
}

#endif





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
	lastinsertid=0;
	sql=NULL;
	maxrows=0;
	affectedrows=0;
	transactiondepth=0;
}

SQLite::~SQLite()
{
#ifdef HAVE_SQLITE
	PushError();
	if (sql) Disconnect();
	PopError();
#endif
}


CString	SQLite::databaseType() const
{
	return CString("SQLite");
}

int SQLite::Connect(const CAssocArray &params)
{
#ifdef HAVE_SQLITE
	if (sql) Disconnect();
	condata=params;
	ppl6::CString filename=params["filename"];
	if (filename.IsEmpty()) filename=params["host"];

	int ret=sqlite3_open((const char*)filename,(sqlite3 **)&sql);
	if (!sql) {
		ppl6::SetError(2);
		return 0;
	}
	if (ret != SQLITE_OK) {
		ppl6::SetError(77,"%s",sqlite3_errmsg((sqlite3*)sql));
		sqlite3_close((sqlite3*)sql);
		sql=NULL;
		return 0;
	}
	return 1;
#else
	SetError(511,"SQLite");
	return 0;
#endif
}

int SQLite::Disconnect()
{
	if (sql) {
#ifdef HAVE_SQLITE
		sqlite3_close((sqlite3*)sql);
#endif
	}
	sql=NULL;
	return 1;
}

int SQLite::Reconnect()
{
#ifdef HAVE_SQLITE
	Close();
	CAssocArray a;
	a.Copy(&condata);
	if (Connect(a)) return 1;
	SetError(299,"Reconnect failed");
	return 0;
#else
	SetError(511,"SQLite");
	return 0;
#endif

}

int SQLite::Ping()
{
	if (sql) return 1;
	return 0;
}

int SQLite::SelectDB(const char *databasename)
{
	ppl6::SetError(565,"SQLite::SelectDB");
	return 0;
}

int SQLite::CreateDatabase(const char *name)
{
	ppl6::SetError(565,"SQLite::CreateDatabase");
	return 0;
}

int SQLite::Exec(const CString &query)
{
#ifndef HAVE_SQLITE
	SetError(511,"SQLite");
	return 0;
#else
	double t_start;
	affectedrows=0;
	if (!sql) { SetError(181); return 0; }
	t_start=getmicrotime();
	sqlite3_stmt *stmt=NULL;
	int ret=sqlite3_prepare_v2((sqlite3*)sql, (const char*)query, query.Len(),&stmt,NULL);
	if (ret!=SQLITE_OK) {
		SetError(138,"Error: %s (sqlite3_prepare_v2), Query: %s",sqlite3_errmsg((sqlite3*)sql),(const char*)query);
		return 0;
	}
	if (stmt==NULL) {
		SetError(2);
		return 0;
	}

	ret=sqlite3_step(stmt);
	if (ret!=SQLITE_DONE && ret!=SQLITE_ROW) {
		SetError(138,"Error: %s (sqlite3_prepare_v2), Query: %s",sqlite3_errmsg((sqlite3*)sql),(const char*)query);
		sqlite3_finalize(stmt);
		return 0;
	}
	ret=sqlite3_finalize(stmt);
	if (ret !=SQLITE_OK) {
		SetError(138,"Error: %s (sqlite3_finalize), Query: %s",sqlite3_errmsg((sqlite3*)sql),(const char*)query);
		return 0;
	}
	lastinsertid=sqlite3_last_insert_rowid((sqlite3*)sql);
	affectedrows=sqlite3_changes((sqlite3*)sql);
	UpdateLastUse();
	LogQuery(query,(float)(getmicrotime()-t_start));
	return 1;
#endif
}

Result *SQLite::Query(const CString &query)
{
#ifndef HAVE_SQLITE
	SetError(511,"SQLite");
	return NULL;
#else
	double t_start;
	affectedrows=0;
	if (!sql) { SetError(181); return 0; }
	t_start=getmicrotime();
	sqlite3_stmt *stmt=NULL;
	int ret=sqlite3_prepare_v2((sqlite3*)sql, (const char*)query, query.Len(),&stmt,NULL);
	if (ret!=SQLITE_OK) {
		SetError(138,"Error: %s (sqlite3_prepare_v2), Query: %s",sqlite3_errmsg((sqlite3*)sql),(const char*)query);
		return 0;
	}
	if (stmt==NULL) {
		SetError(2);
		return 0;
	}
	ret=sqlite3_step(stmt);
	if (ret!=SQLITE_DONE && ret!=SQLITE_ROW) {
		SetError(138,"Error: %s (sqlite3_prepare_v2), Query: %s",sqlite3_errmsg((sqlite3*)sql),(const char*)query);
		sqlite3_finalize(stmt);
		return 0;
	}

	LogQuery(query,(float)(getmicrotime()-t_start));
	UpdateLastUse();
	SQLiteResult *r=new SQLiteResult;
	if (!r) {
		sqlite3_finalize(stmt);
		ppl6::SetError(2);
		return NULL;
	}
	r->last_res=ret;
	r->sql=(sqlite3*)sql;
	r->db=this;
	r->stmt=stmt;
	r->rows=0;
	r->lastinsertid=sqlite3_last_insert_rowid((sqlite3*)sql);
	r->affectedrows=sqlite3_changes((sqlite3*)sql);
	r->num_fields=sqlite3_column_count(stmt);
	return r;
#endif
}

ppluint64 SQLite::GetInsertID()
{
	return lastinsertid;
}

pplint64 SQLite::GetAffectedRows()
{
	return affectedrows;
}

void SQLite::SetMaxRows(ppluint64 rows)
{
	maxrows=rows;
}


int SQLite::StartTransaction()
{
#ifndef HAVE_SQLITE
	SetError(511,"SQLite");
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

int SQLite::EndTransaction()
{
#ifndef HAVE_SQLITE
	SetError(511,"SQLite");
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

int SQLite::CancelTransaction()
{
#ifndef HAVE_SQLITE
	SetError(511,"SQLite");
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

int SQLite::CancelTransactionComplete()
{
#ifndef HAVE_SQLITE
	SetError(511,"SQLite");
	return 0;
#else
	if (Exec("ROLLBACK")) {
		transactiondepth=0;
		return 1;
	}
	return 0;
#endif
}

int SQLite::Escape(CString &str) const
{
#ifndef HAVE_SQLITE
	SetError(511,"SQLite");
	return 0;
#else
	// SQLite hat keine Escape-Funktion, daher müssen wir das selbst machen
	CString n;
	const char *tmp=str.GetPtr();
	int c;
	while ((c=tmp[0])) {
		if (c=='\'') n+="'";
		n+=c;
		tmp++;
	}
	str=n;
	return 1;
#endif
}

}	// EOF namespace db
}	// EOF namespace ppl6
