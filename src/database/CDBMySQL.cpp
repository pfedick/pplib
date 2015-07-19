/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.2 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: CDBMySQL.cpp,v 1.2 2010/02/12 19:43:48 pafe Exp $
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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
//#include <Ws2tcpip.h>
#endif

#include "ppl6.h"

#ifdef HAVE_MYSQL
#include <mysql.h>
#include <errmsg.h>

namespace ppl6 {
#include "thread.h"

static CMutex MySQLGlobalMutex;
static int __mysql_init=0;


void MySQLThreadEnd(THREADDATA *t)
{
	//printf ("Thread: %x (%u) MySQL_thread_end\n",t, t->threadid);
	mysql_thread_end();
}

int MySQLThreadStart()
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

/*!\class CDBMySQL
 * \ingroup PPLGroupOldDatabases
 */

CDBMySQL::CDBMySQL()
{
	MySQLGlobalMutex.Lock();
	if (__mysql_init==0) {
		__mysql_init=1;
		Cppl6Core *core=PPLInit();
		core->AtExit(pplMySQLExit,NULL);
	}
	MySQLGlobalMutex.Unlock();
	lastinsertid=0;
	connected=false;
	mysql=NULL;
	MySQLThreadStart();
}

CDBMySQL::~CDBMySQL()
{
	PushError();
	if (connected==true) Close();
	if (mysql!=NULL) mysql_close((MYSQL *)mysql);
	mysql=NULL;
	SetLogfile();
	PopError();
}

int	CDBMySQL::Connect(PPL_DATABASE *dbdata)
{
	MySQLThreadStart();
	if (dbdata==NULL) { SetError(6); return 0; }
	CAssocArray a;
	a.Set("host",dbdata->host);
	a.Setf("port","%i",dbdata->port);
	a.Set("user",dbdata->login);
	a.Set("password",dbdata->password);
	a.Set("dbname",dbdata->name);
	return Connect(&a);
}

int CDBMySQL::Connect(CAssocArray *dbdata)
{
	MySQLThreadStart();
	mutex.Lock();
	if (dbdata==NULL) { SetError(6); mutex.Unlock(); return 0; }
	if (connected==true) {
		mutex.Unlock();
		Close();
		mutex.Lock();
	}
	condata.Clear();
	condata.Copy(dbdata);
	mysql=mysql_init(NULL);					// mySQL-Handle erzeugen
	if (mysql==NULL) { SetError(77); mutex.Unlock(); return 0; }
	const char *tmp;
	const char *host=dbdata->Get("host");
	tmp=dbdata->Get("port");
	unsigned int port=0;
	if (tmp) port=atoi(tmp);
	const char *user=dbdata->Get("user");
	const char *password=dbdata->Get("password");
	const char *dbname=dbdata->Get("dbname");
	MYSQL *conn=mysql_real_connect((MYSQL *)mysql, host, user, password, dbname, port, NULL,0);
	if (conn!=NULL) {
		connected=true;
		UpdateLastUse();
		mutex.Unlock();
		return 1;
	}
	// Was war der Fehler?
	SetError(77,mysql_errno((MYSQL *)mysql),"Host: %s:%i, User: %s, Database: %s, MySQL-Error: %u, %s",
		(host?host:"NULL"),port,
		(user?user:"NULL"),
		(dbname?dbname:"NULL"),
		mysql_errno((MYSQL *)mysql),mysql_error((MYSQL *)mysql));
	mysql_close((MYSQL *)mysql);
	mysql=NULL;
	ClearLastUse();
	mutex.Unlock();
	return 0;
}

int CDBMySQL::Reconnect()
{
	MySQLThreadStart();
	int ret,e;
	mutex.Lock();
	if (connected) {
		ret=mysql_ping((MYSQL *)mysql);
		if (ret!=0) e=mysql_errno((MYSQL *)mysql);
		if (ret==0) {
			mutex.Unlock();
			return 1;
		}
	}
	mutex.Unlock();
	Close();
	CAssocArray a;
	a.Copy(&condata);
	ret=Connect(&a);
	if (ret==1) {
		return 1;
	}
	return 0;
}



int	CDBMySQL::ConnectCreate(PPL_DATABASE *dbdata)
{
	MySQLThreadStart();
	mutex.Lock();
	MYSQL *conn;
	if (dbdata==NULL) { SetError(6); mutex.Unlock(); return 0; }
	if (connected==true) {
		mutex.Unlock();
		Close();
		mutex.Lock();
	}
	mysql=mysql_init(NULL);					// mySQL-Handle erzeugen
	if (mysql==NULL) { SetError(77); mutex.Unlock(); return 0; }

	// Zuerst einen direkten connect auf die Datenbank versuchen
	conn=mysql_real_connect((MYSQL *)mysql, dbdata->host, dbdata->login,dbdata->password, dbdata->name, (unsigned int) dbdata->port, NULL,0);
	if (conn!=NULL) {		// erfolgreich
		connected=true;
		UpdateLastUse();
		mutex.Unlock();
		return 1;
	}

	// fehlgeschlagen, vielleicht funktioniert ein Connect auf den Server?
	conn=mysql_real_connect((MYSQL *)mysql, dbdata->host, dbdata->login,dbdata->password, NULL, (unsigned int) dbdata->port, NULL,0);
	if (conn!=NULL) {		// erfolgreich
		connected=true;		// vielleicht gibt es die Datenbank noch nicht?
		// Wir versuchen die Tabelle anzulegen
		Exec("create database %s",dbdata->name);
		mutex.Unlock();
		Close();
		mutex.Lock();
	}

	// Und jetzt nochmal einen Connect auf die Datenbank versuchen
	conn=mysql_real_connect((MYSQL *)mysql, dbdata->host, dbdata->login,dbdata->password, dbdata->name, (unsigned int) dbdata->port, NULL,0);
	if (conn!=NULL) {
		connected=true;
		UpdateLastUse();
		mutex.Unlock();
		return 1;
	}
	// Wieder fehlgeschlagen. Was war der Fehler?
	SetError(77,mysql_errno((MYSQL *)mysql),"MySQL-Error: %u, %s",mysql_errno((MYSQL *)mysql),mysql_error((MYSQL *)mysql));
	mysql_close((MYSQL *)mysql);
	mysql=NULL;
	mutex.Unlock();
	return 0;
}

int	CDBMySQL::Connect(char *dbname)
{
	MySQLThreadStart();
	mutex.Lock();
	// NULL ist erlaubt
	if (connected==true) {
		mutex.Unlock();
		Close();
		mutex.Lock();
	}
	mysql=mysql_init(NULL);					// mySQL-Handle erzeugen
	if (mysql==NULL) { SetError(77); mutex.Unlock(); return 0; }
	mysql_options((MYSQL *)mysql,MYSQL_READ_DEFAULT_GROUP, "test_libmysqld_CLIENT");
	if (!mysql_real_connect((MYSQL *)mysql, NULL, NULL,NULL, dbname, 0, NULL,0)) {
		// Was war der Fehler?
		SetError(77,mysql_errno((MYSQL *)mysql),"MySQL-Error: %u, %s",mysql_errno((MYSQL *)mysql),mysql_error((MYSQL *)mysql));

		mysql_close((MYSQL *)mysql);
		mysql=NULL;
		mutex.Unlock();
		return 0;
	}
	connected=true;
	UpdateLastUse();
	mutex.Unlock();
	return 1;
}

int	CDBMySQL::ConnectCreate(char *dbname)
{
	MySQLThreadStart();
	// Versuchen auf die Datenbank zu connecten
	if (Connect(dbname)) return 1;		// erfolgreich
	// Fehlgeschlagen, weil die Datenbank möglicherweise noch nicht existiert
	// Wir versuchen sie anzulegen
	if (!Connect((char*)NULL)) return 0;	// fehlgeschlagen
	mutex.Lock();
	connected=true;
	Exec("create database %s",dbname);
	mutex.Unlock();
	Close();

	// nun nocheinmal versuchen zu connecten
	if (Connect(dbname)) return 1;		// erfolgreich
	return 0;
}


int	CDBMySQL::Close()
{
	MySQLThreadStart();
	mutex.Lock();
	if (connected==false) {
		mutex.Unlock();
		return 1;
	}
	if (mysql!=NULL) mysql_close((MYSQL *)mysql);
	mysql=NULL;
	connected=false;
	ClearLastUse();
	mutex.Unlock();
	return 1;
}

int CDBMySQL::SelectDB(const char *databasename)
{
	MySQLThreadStart();
	if (!databasename) {
		SetError(194,"SelectDB(const char *databasename)");
		return 0;
	}
	mutex.Lock();
	if (connected==false) { SetError(181); mutex.Unlock(); return 0; }
	if ( mysql_select_db((MYSQL *)mysql, databasename)==0 ) {
		UpdateLastUse();
		mutex.Unlock();
		return 1;
	}
	int e=mysql_errno((MYSQL *)mysql);
	if (e==CR_SERVER_GONE_ERROR || e==CR_SERVER_LOST) {
		mutex.Unlock();
		if (!Reconnect()) return 0;
		mutex.Lock();
		if ( mysql_select_db((MYSQL *)mysql, databasename)==0 ) {
			mutex.Unlock();
			return 1;
		}
	}
	// Was war der Fehler?
	SetError(295,mysql_errno((MYSQL *)mysql),"MySQL-Error: %u, %s",mysql_errno((MYSQL *)mysql),mysql_error((MYSQL *)mysql));
	mutex.Unlock();
	return 0;
}

int CDBMySQL::SelectDB(CString *databasename)
{
	MySQLThreadStart();
	if (!databasename) {
		SetError(194,"SelectDB(CString *databasename)");
		return 0;
	}
	return SelectDB((const char*)databasename->GetPtr());
}

PPL_DB_RESULT CDBMySQL::DoExec(const char *query)
{
	MySQLThreadStart();
	double t_start;
	MYSQL_RES *res;
	mutex.Lock();
	if (connected==false) { SetError(181); mutex.Unlock(); return NULL; }
	UpdateLastUse();
	t_start=getmicrotime();
	int ret=mysql_query((MYSQL *)mysql, query);
	if (ret!=0) {		// Vielleicht ist die DB-Verbindung weg?
		int e=mysql_errno((MYSQL *)mysql);
		if (e==CR_SERVER_GONE_ERROR || e==CR_SERVER_LOST) {
			mutex.Unlock();
			if (!Reconnect()) return NULL;
			mutex.Lock();
			t_start=getmicrotime();
			ret=mysql_query((MYSQL *)mysql, query);
		}
	}
	if (ret==0) {
		UpdateLastUse();
		res = mysql_store_result((MYSQL *)mysql);
		LogQuery(query,(float)(getmicrotime()-t_start));
		if (res) {
			lastinsertid=(ppld64)mysql_insert_id((MYSQL *)mysql);
			UpdateLastUse();
			mutex.Unlock();
			return (PPL_DB_RESULT)res;
		}
		if (mysql_field_count((MYSQL *)mysql)==0) {
			lastinsertid=(ppld64)mysql_insert_id((MYSQL *)mysql);
			UpdateLastUse();
			mutex.Unlock();
			return (PPL_DB_RESULT)1;
		}
	}
	// Was war der Fehler?
	SetError(138,mysql_errno((MYSQL *)mysql),"MySQL-Error: %u, %s, Query: %s",mysql_errno((MYSQL *)mysql),mysql_error((MYSQL *)mysql), query);
	mutex.Unlock();
	return 0;
}

int CDBMySQL::FreeResult (PPL_DB_RESULT res)
{
	MySQLThreadStart();
	MYSQL_RES* r=(MYSQL_RES*)res;
	if (res==(PPL_DB_RESULT)1) return 1;
	mutex.Lock();
	if (r!=NULL) mysql_free_result(r);
	mutex.Unlock();
	return 1;
}

ppldd CDBMySQL::NumRows (PPL_DB_RESULT res)
{
	MySQLThreadStart();
	MYSQL_RES* r=(MYSQL_RES*)res;
	mutex.Lock();
	if (r!=NULL) {
		ppldd ret=(ppldd) mysql_num_rows(r);
		mutex.Unlock();
		return ret;
	}
	SetError(182);
	mutex.Unlock();
	return 0;
}

ppldds CDBMySQL::FieldNum(PPL_DB_RESULT res, char * fieldname)
{
	MySQLThreadStart();
	ppldd num_fields;
	MYSQL_RES* r=(MYSQL_RES*)res;
	if (r==0 || fieldname==0) { SetError(6); return -1; }
	mutex.Lock();
	// Das Feld suchen
	MYSQL_FIELD *fields;
	num_fields = mysql_num_fields(r);
	fields = mysql_fetch_fields(r);
	for(ppldd i=0; i<num_fields; i++) {
		if (strcmp(fields[i].name,fieldname)==0) {
			mutex.Unlock();
			return i;
		}
	}
	SetError(183);
	mutex.Unlock();
	return -1;
}

char * CDBMySQL::Result (PPL_DB_RESULT res, ppldd nr, char * feld)
{
	MySQLThreadStart();
	MYSQL_ROW row;
	int num_fields;
	MYSQL_RES* r=(MYSQL_RES*)res;
	if (r==0) { SetError(6); return NULL; }
	mutex.Lock();
	// Das Feld suchen
	MYSQL_FIELD *fields;
	num_fields = mysql_num_fields(r);
	fields = mysql_fetch_fields(r);
	for(int i=0; i<num_fields; i++) {
		if (strcmp(fields[i].name,feld)==0) {
			mysql_data_seek(r,nr);
			row=mysql_fetch_row(r) ;
			mutex.Unlock();
			return row[i];
		}
	}
	SetError(183);
	mutex.Unlock();
	return NULL;
}

char * CDBMySQL::ResultByNo (PPL_DB_RESULT res, ppldd nr, ppldd feld)
{
	MySQLThreadStart();
	ppldd ll;
	MYSQL_ROW row;
	MYSQL_RES* r=(MYSQL_RES*)res;
	if (r==0 || feld <0) { SetError(6); return NULL; }
	mutex.Lock();
	mysql_data_seek(r,nr);
	row=mysql_fetch_row(r) ;
	ll=(ppldd)strlen(row[feld]);
	//DLOG ("row[feld]=%s, len=%u",row[feld],ll);
	mutex.Unlock();
	return row[feld];
}


ppldd CDBMySQL::Errno()
{
	MySQLThreadStart();
	ppldd ret=0;
	mutex.Lock();
	if (connected) {
		SetError(238,mysql_errno((MYSQL *)mysql),"MySQL-Error: %u, %s",mysql_errno((MYSQL *)mysql),mysql_error((MYSQL *)mysql));
		ret=mysql_errno((MYSQL *)mysql);
	}
	mutex.Unlock();
	return ret;
}

const char *CDBMySQL::Error()
{
	MySQLThreadStart();
	const char *ret="not connected!";
	mutex.Lock();
	if (connected) {
		SetError(238,mysql_errno((MYSQL *)mysql),"MySQL-Error: %u, %s",mysql_errno((MYSQL *)mysql),mysql_error((MYSQL *)mysql));
		ret=mysql_error((MYSQL *)mysql);
	}
	mutex.Unlock();
	return ret;
}


void CDBMySQL::PrintResult(PPL_DB_RESULT result)
{
	MySQLThreadStart();
	mutex.Lock();
	MYSQL_RES *res=(MYSQL_RES *)result;
	MYSQL_ROW row, end_row;
	int num_fields;
	num_fields = mysql_num_fields(res);
	if (mysql_field_count((MYSQL *)mysql) > 0) {
		while ((row = mysql_fetch_row(res))) {
			(void)fputs(">> ", stdout);
			for (end_row = row + num_fields; row < end_row; ++row)
				(void)printf("%s\t", row ? (char*)*row : "NULL");
			(void)fputc('\n', stdout);
		}
		(void)fputc('\n', stdout);
	} else {
		(void)printf("Affected rows: %lld\n", mysql_affected_rows((MYSQL *)mysql));
	}
	mutex.Unlock();
}

CAssocArray *CDBMySQL::FetchArray(PPL_DB_RESULT res, ppldds nr)
{
	MySQLThreadStart();
	CAssocArray *result=new CAssocArray;
	if (!result) {
		SetError(2);
		return NULL;
	}
	if (FetchArray(result,res,nr)) return result;
	delete result;
	return NULL;
}

int CDBMySQL::FetchArray(CAssocArray *array, PPL_DB_RESULT res, ppldds nr)
{
	MySQLThreadStart();
	int num_fields;
	MYSQL_ROW row;
	MYSQL_RES* r=(MYSQL_RES*)res;
	if (r==0) {SetError(6); return 0;}
	if (!array) {
		SetError(194,"CAssocArray *array");
		return 0;
	}
	mutex.Lock();
	if (nr>=0) mysql_data_seek(r,nr);

	// Zeile holen
	row=mysql_fetch_row(r);

	if(!row) { SetError(300,"mysql_fetch_row"); mutex.Unlock(); return 0; }	// Keine Zeile vorhanden
	UpdateLastUse();
	// Die Feldnamen holen
	MYSQL_FIELD *fields;
	num_fields = mysql_num_fields(r);
	fields = mysql_fetch_fields(r);

	if (!fields) { SetError(300,"mysql_fetch_fields"); mutex.Unlock(); return 0; }	// Keine Felder
	array->Clear();
	for(int i=0; i<num_fields; i++) {
		if (row[i]) array->Add(fields[i].name,row[i]);
		else array->Add(fields[i].name,(char*)"");
	}
	mutex.Unlock();
	return 1;
}

int CDBMySQL::Ping() {			// 1=Server ist connected, 0=Connection lost
	MySQLThreadStart();
	mutex.Lock();
	if (connected) {
		if (mysql_ping((MYSQL *)mysql)==0) {		// Server ist noch connected
			UpdateLastPing();
			mutex.Unlock();
			return 1;
		}
	}
	SetError(181);
	mutex.Unlock();
	return 0;
}

char *CDBMySQL::EscapeString(char *string, size_t length)
{
	MySQLThreadStart();
	if (!string) { SetError(6); return NULL; }
	if (!length) length=strlen(string);
	char *buf=(char *)malloc(length*2+1);   // Buffer reservieren
	mutex.Lock();
	if (connected==false) { free(buf); SetError(181); mutex.Unlock(); return NULL; }
	//ppldd newlength=(ppldd)mysql_real_escape_string((MYSQL *)mysql,buf,string,(unsigned long)length);
	mutex.Unlock();
	return buf;
}

int CDBMySQL::EscapeString(CString *string)
{
	MySQLThreadStart();
	if (!string) {
		SetError(194,"CString *string");
		return 0;
	}
	mutex.Lock();
	if (connected==false) { SetError(181); mutex.Unlock(); return 0; }
	char *buf=(char *)malloc(string->Len()*2+1);   // Buffer reservieren
	if (!buf) {
		SetError(2);
		mutex.Unlock();
		return 0;
	}
	mysql_real_escape_string((MYSQL *)mysql,buf,string->GetPtr(),(unsigned long)string->Len());
	mutex.Unlock();
	string->Set(buf);
	free(buf);
	return 1;
}


ppld64 CDBMySQL::GetInsertID()
{
	MySQLThreadStart();
	ppld64 ret=0;
	mutex.Lock();
	if (connected) {
		//printf("GetInsertID: %llu\n",lastinsertid);
		ret=lastinsertid;
	}
	mutex.Unlock();
	return ret;
}





} // end of namespace ppl


#endif // HAVE_MYSQL
