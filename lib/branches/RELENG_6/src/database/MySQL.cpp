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
#include <winsock2.h>
#endif


#include "ppl6.h"
#include "ppl6-db.h"

#ifdef HAVE_MYSQL
#ifdef MINGW32
#define __LCC__
#endif
#include <mysql.h>
#include <errmsg.h>
#endif

namespace ppl6 {
#include "thread.h"
namespace db {


#ifdef HAVE_MYSQL

static CMutex MySQLGlobalMutex;
static int __mysql_init=0;

static void MySQLThreadEnd(THREADDATA *t)
{
	//printf ("Thread: %x (%u) MySQL_thread_end\n",t, t->threadid);
	mysql_thread_end();
}

static int MySQLThreadStart()
{
	THREADDATA * d=GetThreadData();
	if (!d) return 0;
	if (d->mysql_thread_end) return 1;
	if (mysql_thread_init()==0) {
		d->mysql_thread_end=MySQLThreadEnd;
		//printf ("Thread: %x (%u) verwendet MySQL\n",d, d->threadid);
		return 1;
	}
	return 0;
}



static int pplMySQLExit(void *data)
{
	mysql_library_end();
	return 1;
}

class MySQLResult : public Result
{
	friend class MySQL;
	private:
		MYSQL_RES	*res;			//!\brief MySQL-spezifisches Result-Handle
		MySQL		*mysql_class;	//!\brief Die ppl6::db::MySQL-Klasse, die das Result erzeugt hat
		MYSQL		*mysql;			//!\brief MySQL-spezifisches Handle des Datenbank-Connects, das den Result erzeugt hat
		pplint64	rows;			//!\brief Anzahl Zeilen im Ergebnis
		pplint64 	lastinsertid;	//!\brief Falls es sich um einen Insert mit einem Autoincrement-Index handelte, steht hier die vergebene ID
		pplint64	affectedrows;	//!\brief Falls es sich um ein Update/Insert/Replace handelte, steht hier die Anzahl betroffender Datensätze
		int			num_fields;		//!\brief Anzahl Spalten im Ergebnis

	public:
		MySQLResult();
		virtual ~MySQLResult();
		virtual	void Clear();
		virtual pplint64 Rows();
		virtual pplint64 Affected();
		virtual int Fields();
		virtual const char *Get(pplint64 row, const char *fieldname);
		virtual const char *Get(pplint64 row, int field);
		virtual int FieldNum(const char *fieldname);
		virtual const char *FieldName(int num);
		virtual Result::Type	FieldType(int num);
		virtual Result::Type	FieldType(const char *fieldname);
		virtual CAssocArray  FetchArray(pplint64 row=-1);
		virtual int		FetchArray(CAssocArray &array, pplint64 row=-1);
		virtual CArray  FetchFields(pplint64 row=-1);
		virtual int		FetchFields(CArray &array, pplint64 row=-1);
		virtual int		Seek(pplint64 row);
		virtual void    PrintResult();
};

/*!\class MySQLResult
 * \ingroup PPLGroupDatabases
 * \brief Klasse, die das Ergebnis eines MySQL-Results aufnimmt
 *
 * \descr
 * Dies ist eine interne Klasse des MySQL-Datenbankmoduls. Sie ist abgeleitet von ppl6::db::Result
 * und nimmt das Ergebnis eines Datenbank-Selects auf. Mit Ihren Funktionen kann anschließend
 * auf das Ergebnis zugegriffen werden.
 *
 * Die Klasse wird durch Aufruf verschiedener Funktionen innerhalb der Klasse ppl6::db::MySQL erstellt
 * und muss von der aufrufenden Anwendung selbst mit \c delete gelöscht werden.
 *
 */

MySQLResult::MySQLResult()
{
	res=NULL;
	mysql_class=NULL;
	mysql=NULL;
	rows=0;
	lastinsertid=0;
	affectedrows=0;
	num_fields=0;
}

MySQLResult::~MySQLResult()
{
	Clear();
}

void MySQLResult::Clear()
{
	rows=0;
	if (res) mysql_free_result(res);
	res=NULL;
	mysql_class=NULL;
	mysql=NULL;
	lastinsertid=0;
	affectedrows=0;
	num_fields=0;
}

pplint64 MySQLResult::Rows()
{
	return rows;
}

pplint64 MySQLResult::Affected()
{
	return affectedrows;
}

int MySQLResult::Fields()
{
	return num_fields;
}

const char *MySQLResult::Get(pplint64 row, const char *fieldname)
{
	if (!res) {
		SetError(182);
		return NULL;
	}
	if (!fieldname) {
		SetError(194);
		return NULL;
	}
	MySQLThreadStart();
	MYSQL_ROW mrow;
	// Das Feld suchen
	MYSQL_FIELD *field;
	field = mysql_fetch_fields(res);
	for(int i=0; i<num_fields; i++) {
		if (strcmp(field[i].name,fieldname)==0) {
			mysql_data_seek(res,row);
			mrow=mysql_fetch_row(res) ;
			if (!mrow) {
				SetError(222,"%llu",row);
				return NULL;
			}
			return mrow[i];
		}
	}
	SetError(183,"%s",fieldname);
	return NULL;
}

const char *MySQLResult::Get(pplint64 row, int field)
{
	if (!res) {
		SetError(182);
		return 0;
	}
	if (field>num_fields) {
		SetError(183,"%i",field);
		return NULL;
	}
	MySQLThreadStart();
	MYSQL_ROW mrow;
	mysql_data_seek(res,row);
	mrow=mysql_fetch_row(res) ;
	if (!mrow) {
		SetError(222,"%llu",row);
		return NULL;
	}
	return mrow[field];
}

int MySQLResult::FieldNum(const char *fieldname)
{
	if (!res) {
		SetError(182);
		return 0;
	}
	if (!fieldname) {
		SetError(194);
		return 0;
	}
	MySQLThreadStart();
	// Das Feld suchen
	MYSQL_FIELD *field;
	field = mysql_fetch_fields(res);
	for(int i=0; i<num_fields; i++) {
		if (strcmp(field[i].name,fieldname)==0) {
			return i;
		}
	}
	SetError(183,"%s",fieldname);
	return -1;
}

const char *MySQLResult::FieldName(int field)
{
	if (!res) {
		SetError(182);
		return 0;
	}
	if (field>num_fields) {
		SetError(183,"%i",field);
		return NULL;
	}
	MySQLThreadStart();
	// Das Feld suchen
	MYSQL_FIELD *mf;
	mf = mysql_fetch_fields(res);
	return mf[field].name;
}

Result::Type MySQLResult::FieldType(int field)
{
	if (!res) {
		SetError(182);
		return Result::Error;
	}
	if (field>num_fields) {
		SetError(183,"%i",field);
		return Result::Error;
	}
	MySQLThreadStart();
	// Das Feld suchen
	MYSQL_FIELD *mf;
	mf = mysql_fetch_fields(res);
	switch (mf[field].type) {
#ifdef FIELD_TYPE_TINY
		case MYSQL_TYPE_TINY:
			return Result::Integer;
#endif
#ifdef FIELD_TYPE_SHORT
		case MYSQL_TYPE_SHORT:
			return Result::Integer;
#endif
#ifdef FIELD_TYPE_LONG
		case MYSQL_TYPE_LONG:
			return Result::Integer;
#endif
#ifdef FIELD_TYPE_INT24
		case MYSQL_TYPE_INT24:
			return Result::Integer;
#endif
#ifdef FIELD_TYPE_LONGLONG
		case MYSQL_TYPE_LONGLONG:
			return Result::Integer;
#endif
#ifdef FIELD_TYPE_DECIMAL
		case MYSQL_TYPE_DECIMAL:
			return Result::Decimal;
#endif
#ifdef  FIELD_TYPE_NEWDECIMAL
		case MYSQL_TYPE_NEWDECIMAL:
			return Result::Decimal;
#endif
#ifdef FIELD_TYPE_FLOAT
		case MYSQL_TYPE_FLOAT:
			return Result::Decimal;
#endif
#ifdef FIELD_TYPE_DOUBLE
		case MYSQL_TYPE_DOUBLE:
			return Result::Decimal;
#endif
#ifdef FIELD_TYPE_BIT
		case MYSQL_TYPE_BIT:
			return Result::Bit;
#endif
#ifdef FIELD_TYPE_TIMESTAMP
		case MYSQL_TYPE_TIMESTAMP:
			return Result::Timestamp;
#endif
#ifdef FIELD_TYPE_DATE
		case MYSQL_TYPE_DATE:
			return Result::Date;
#endif
#ifdef FIELD_TYPE_YEAR
		case MYSQL_TYPE_YEAR:
			return Result::Date;
#endif
#ifdef FIELD_TYPE_TIME
		case MYSQL_TYPE_TIME:
			return Result::Time;
#endif
#ifdef FIELD_TYPE_DATETIME
		case MYSQL_TYPE_DATETIME:
			return Result::DateTime;
#endif
#ifdef FIELD_TYPE_STRING
		case MYSQL_TYPE_STRING:
			return Result::String;
#endif
#ifdef FIELD_TYPE_VAR_STRING
		case MYSQL_TYPE_VAR_STRING:
			return Result::String;
#endif
#if MYSQL_TYPE_VARCHAR > 0
		case MYSQL_TYPE_VARCHAR:
			return Result::String;
#endif
#ifdef FIELD_TYPE_BLOB
		case MYSQL_TYPE_BLOB:
			return Result::Binary;
#endif
#ifdef FIELD_TYPE_ENUM
		case MYSQL_TYPE_ENUM:
			return Result::Enum;
#endif
		default:
			return Result::Unknown;
	}
	return Result::Unknown;
}

Result::Type MySQLResult::FieldType(const char *fieldname)
{
	int field=FieldNum(fieldname);
	if (field<0) return Result::Unknown;
	return FieldType(field);
}

int MySQLResult::FetchArray(CAssocArray &array, pplint64 row)
{
	array.Clear();
	if (!res) {
		SetError(182);
		return 0;
	}
	MySQLThreadStart();
	MYSQL_ROW mrow;

	if (row>=0) mysql_data_seek(res,row);

	// Zeile holen
	mrow=mysql_fetch_row(res);
	if(!mrow) { SetError(300,"mysql_fetch_row"); return 0; }	// Keine Zeile vorhanden
	// Die Feldnamen holen
	MYSQL_FIELD *fields;
	fields = mysql_fetch_fields(res);
	if (!fields) { SetError(300,"mysql_fetch_fields"); return 0; }	// Keine Felder
	array.Clear();
	for(int i=0; i<num_fields; i++) {
		if (mrow[i]) array.Set(fields[i].name,mrow[i]);
		else array.Set(fields[i].name,"");
	}
	return 1;
}

CAssocArray MySQLResult::FetchArray(pplint64 row)
{
	CAssocArray a;
	FetchArray(a,row);
	return a;
}

int MySQLResult::FetchFields(CArray &array, pplint64 row)
{
	array.Clear();
	if (!res) {
		SetError(182);
		return 0;
	}
	MySQLThreadStart();
	MYSQL_ROW mrow;

	if (row>=0) mysql_data_seek(res,row);

	// Zeile holen
	mrow=mysql_fetch_row(res);
	if(!mrow) { SetError(300,"mysql_fetch_row"); return 0; }	// Keine Zeile vorhanden
	array.Clear();
	for(int i=0; i<num_fields; i++) {
		if (mrow[i]) array.Add(mrow[i]);
		else array.Add("");
	}
	return 1;
}

CArray MySQLResult::FetchFields(pplint64 row)
{
	CArray a;
	FetchFields(a,row);
	return a;
}

int MySQLResult::Seek(pplint64 row)
{
	if (!res) {
		SetError(182);
		return 0;
	}
	MySQLThreadStart();
	mysql_data_seek(res,row);
	return 1;
}


void MySQLResult::PrintResult()
{
	if (!res) {
		return;
	}
	MySQLThreadStart();
	MYSQL_ROW row, end_row;
	mysql_data_seek(res,0);
	while ((row = mysql_fetch_row(res))) {
		(void)fputs(">> ", stdout);
		for (end_row = row + num_fields; row < end_row; ++row)
			(void)printf("%s\t", row ? (char*)*row : "NULL");
		(void)fputc('\n', stdout);
	}
	(void)fputc('\n', stdout);
}

#endif


/*!\class MySQL
 * \ingroup PPLGroupDatabases
 * \brief Implementierung einer MySQL-Datenbank
 *
 * \header \#include <ppl6-db.h>
 *
 * \descr
 * Mit dieser Klasse kann eine Verbindung zu einer MySQL-Datenbank aufgebaut werden, um darüber
 * SQL-Queries durchzuführen.
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_MySQL_Example1
 * \until EOF
 */


MySQL::MySQL()
{
#ifdef HAVE_MYSQL
	MySQLGlobalMutex.Lock();
	if (__mysql_init==0) {
		__mysql_init=1;
		Cppl6Core *core=PPLInit();
		core->AtExit(pplMySQLExit,NULL);
	}
	MySQLGlobalMutex.Unlock();
	MySQLThreadStart();
#endif
	lastinsertid=0;
	mysql=NULL;
	maxrows=0;
	affectedrows=0;
	transactiondepth=0;
}

MySQL::~MySQL()
{
#ifdef HAVE_MYSQL
	PushError();
	if (mysql) Disconnect();
	PopError();
#endif
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
 *
 *
 * \returns Bei Erfolg liefert die 1 zurück, im Fehlerfall 0.
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_Example3
 * \until EOF
 *
 */
int MySQL::Connect(const CAssocArray &params)
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
	return 0;
#else
	MySQLThreadStart();
	if (mysql) Disconnect();
	condata=params;

	mysql=mysql_init(NULL);					// mySQL-Handle erzeugen
	if (mysql==NULL) { SetError(77); return 0; }
	MYSQL *conn=mysql_real_connect((MYSQL *)mysql, params["host"], params["user"],
			params["password"], params["dbname"], ppl6::atoi(params["port"]), NULL,0);
	if (conn!=NULL) {
		UpdateLastUse();
		const char *charset=params["charset"];
		if (!charset) charset="latin1";
		int ret=mysql_set_character_set((MYSQL *)mysql, charset);
		if (ret!=0) {
			printf ("WARNING: %i: %s\n",mysql_errno((MYSQL *)mysql),mysql_error((MYSQL *)mysql));
		}

		return 1;
	}
	// Was war der Fehler?
	int merr=mysql_errno((MYSQL *)mysql);
	switch (merr) {
		case 1044:
			SetError(539,mysql_errno((MYSQL *)mysql),"MySQL-Error: %s",
					mysql_error((MYSQL *)mysql));
			break;
		case 1045:
			SetError(538,mysql_errno((MYSQL *)mysql),"MySQL-Error: %s",
					mysql_error((MYSQL *)mysql));
			break;
		case 2003:
			SetError(77,mysql_errno((MYSQL *)mysql),"MySQL-Server: %s:%i",(params["host"]?params["host"]:"NULL"),
					ppl6::atoi(params["port"]));
			break;

		case 2005:
			SetError(273,mysql_errno((MYSQL *)mysql),"MySQL-Server: %s",(params["host"]?params["host"]:"NULL"));
			break;
		default:
			SetError(238,mysql_errno((MYSQL *)mysql),"Host: %s:%i, User: %s, Database: %s, MySQL-Error: %u, %s",
					(params["host"]?params["host"]:"NULL"),ppl6::atoi(params["port"]),
					(params["user"]?params["user"]:"NULL"),
					(params["dbname"]?params["dbname"]:"NULL"),
					mysql_errno((MYSQL *)mysql),mysql_error((MYSQL *)mysql));
			break;
	}
	mysql_close((MYSQL *)mysql);
	mysql=NULL;
	ClearLastUse();
	return 0;
#endif
}

int MySQL::Reconnect()
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
	return 0;
#else
	MySQLThreadStart();
	int ret;
	if (mysql) {
		ret=mysql_ping((MYSQL *)mysql);
		if (ret==0) {
			return 1;
		}
	}
	Close();
	CAssocArray a;
	a.Copy(&condata);
	if (Connect(a)) return 1;
	SetError(299,"Reconnect failed");
	return 0;
#endif
}

int MySQL::Disconnect()
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
	return 0;
#else
	MySQLThreadStart();
	if (!mysql) {
		return 1;
	}
	mysql_close((MYSQL *)mysql);
	mysql=NULL;
	ClearLastUse();
	return 1;
#endif
}

int MySQL::SelectDB(const char *databasename)
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
	return 0;
#else
	MySQLThreadStart();
	if (!databasename) {
		SetError(194,"SelectDB(const char *databasename)");
		return 0;
	}
	if (!mysql) { SetError(181); return 0; }
	if ( mysql_select_db((MYSQL *)mysql, databasename)==0 ) {
		UpdateLastUse();
		return 1;
	}
	int e=mysql_errno((MYSQL *)mysql);
	if (e==CR_SERVER_GONE_ERROR || e==CR_SERVER_LOST) {
		if (!Reconnect()) return 0;
		if ( mysql_select_db((MYSQL *)mysql, databasename)==0 ) {
			return 1;
		}
	}
	// Was war der Fehler?
	SetError(295,mysql_errno((MYSQL *)mysql),"MySQL-Error: %u, %s",mysql_errno((MYSQL *)mysql),mysql_error((MYSQL *)mysql));
	return 0;
#endif
}

int	MySQL::Mysql_Query(const CString &query)
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
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
	return 0;
#else
	int ret=mysql_real_query((MYSQL *)mysql, query.GetPtr(), query.Len());
	if (ret==0) return 1;
	// Vielleicht ist die DB-Verbindung weg?
	int e=mysql_errno((MYSQL *)mysql);
	if (e==CR_SERVER_GONE_ERROR || e==CR_SERVER_LOST) {
		if (!Reconnect()) {
			return 0;
		}
		ret=mysql_real_query((MYSQL *)mysql, query.GetPtr(), query.Len());
		if (ret==0) return 1;
		e=mysql_errno((MYSQL *)mysql);
	}
	// Was war der Fehler?
	int err=138;
	if (e==1146) err=78;
	SetError(err,mysql_errno((MYSQL *)mysql),"MySQL-Error: %u, %s, Query: %s",e,mysql_error((MYSQL *)mysql), (const char*)query);

	return 0;
#endif
}


int MySQL::Exec(const CString &query)
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
	return 0;
#else
	MySQLThreadStart();
	double t_start;
	affectedrows=0;
	if (!mysql) { SetError(181); return 0; }
	t_start=getmicrotime();
	int ret=Mysql_Query(query);
	UpdateLastUse();
	if (ret) {
		affectedrows = mysql_affected_rows((MYSQL *)mysql);
		lastinsertid=(ppld64)mysql_insert_id((MYSQL *)mysql);
		MYSQL_RES *res = mysql_store_result((MYSQL *)mysql);
		LogQuery(query,(float)(getmicrotime()-t_start));
		if (res) {
			mysql_free_result(res);
			return 1;
		}
		if (mysql_errno((MYSQL *)mysql)==0) {	// Query hat kein Ergebnis zurückgeliefert
			return 1;
		}
	}
	return 0;
#endif
}

Result *MySQL::Query(const CString &query)
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
	return NULL;
#else
	MySQLThreadStart();
	double t_start;
	affectedrows=0;
	if (!mysql) { SetError(181); return NULL; }
	t_start=getmicrotime();
	int ret=Mysql_Query(query);
	UpdateLastUse();
	if (ret) {
		MySQLResult *res=new MySQLResult;
		if (!res) {
			SetError(2);
			return 0;
		}
		res->mysql=(MYSQL *)mysql;
		res->mysql_class=this;
		affectedrows = mysql_affected_rows((MYSQL *)mysql);
		res->affectedrows=affectedrows;
		res->res = mysql_store_result((MYSQL *)mysql);
		LogQuery(query,(float)(getmicrotime()-t_start));
		lastinsertid=(ppld64)mysql_insert_id((MYSQL *)mysql);
		res->lastinsertid=lastinsertid;
		if (res->res) {
			res->rows=mysql_num_rows(res->res);
			res->num_fields=mysql_num_fields(res->res);
			return res;
		}
		if (mysql_errno((MYSQL *)mysql)==0) {	// Query hat kein Ergebnis zurückgeliefert
			return res;
		}
		PushError();
		delete res;
		PopError();
	}
	return NULL;
#endif
}

ppluint64 MySQL::GetInsertID()
{
	return lastinsertid;
}

pplint64 MySQL::GetAffectedRows()
{
	return affectedrows;
}

void MySQL::SetMaxRows(ppluint64 rows)
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
#else
	maxrows=rows;
#endif
}

int MySQL::Ping()
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
	return 0;
#else
	MySQLThreadStart();
	if (mysql) {
		if (mysql_ping((MYSQL *)mysql)==0) {		// Server ist noch connected
			UpdateLastPing();
			return 1;
		}
	}
	SetError(181);
	return 0;
#endif
}

int MySQL::Escape(CString &str) const
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
	return 0;
#else
	MySQLThreadStart();
	if (!mysql) { SetError(181); return 0; }
	size_t l=str.Len()*2+1;
	char *buf=(char *)malloc(l);   // Buffer reservieren
	if (!buf) {
		SetError(2);
		return 0;
	}
	mysql_real_escape_string((MYSQL *)mysql,buf,str.GetPtr(),(unsigned long)str.Len());
	str.ImportBuffer(buf,l);
	return 1;
#endif
}

int MySQL::StartTransaction()
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
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

int MySQL::EndTransaction()
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
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

int MySQL::CancelTransaction()
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
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

int MySQL::CancelTransactionComplete()
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
	return 0;
#else
	if (Exec("ROLLBACK")) {
		transactiondepth=0;
		return 1;
	}
	return 0;
#endif
}


int MySQL::CreateDatabase(const char *name)
{
#ifndef HAVE_MYSQL
	SetError(511,"MySQL");
	return 0;
#else
	if (!name) {
		SetError(194,"MySQL::CreateDatabase(const char *name)");
		return 0;
	}
	CString Name=name;
	if (!Escape(Name)) return 0;
	if (!mysql) { SetError(181); return 0; }
	CString Query="create database "+Name;
	if (!Mysql_Query(Query)) {
		SetError(540,mysql_errno((MYSQL *)mysql),"MySQL-Error: %u, %s",mysql_errno((MYSQL *)mysql),mysql_error((MYSQL *)mysql));
		return 0;
	}
	return 1;
#endif
}

CString	MySQL::databaseType() const
{
	return CString("MySQL");
}


}	// EOF namespace db
}	// EOF namespace ppl6
