/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.4 $
 * $Date: 2009/08/19 22:33:12 $
 * $Id: CDBPostgres.cpp,v 1.4 2009/08/19 22:33:12 patrick Exp $
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
#ifdef _WIN32
#include <winsock.h>
#endif

#include "ppl6.h"

#ifdef _HAVE_POSTGRES
#include <libpq-fe.h>

namespace ppl {

#define MAXQUERYBUFFER	65536

CDBPostgres::CDBPostgres()
{
	lastinsertid=0;
	maxquerybuffer=MAXQUERYBUFFER;
	connected=false;
	conn=NULL;
	options=NULL;
	InitLogging();
}

CDBPostgres::CDBPostgres(ppldd buffersize)
{
	lastinsertid=0;
	maxquerybuffer=buffersize;
	connected=false;
	conn=NULL;
	InitLogging();
}

CDBPostgres::~CDBPostgres()
{
	if (connected==true) Close();
	if (conn!=NULL) {
		PQfinish((PGconn*)conn);
	}
	conn=NULL;
	SetLogfile(NULL);
}


int	CDBPostgres::Connect(ppl_DATABASE *dbdata)
{
	char port[64];
	if (dbdata==NULL) { SetError(6); return 0; }
	if (connected==true) Close();

	// Postgres erwartet den Port auch als char*

	sprintf(port,"%u",dbdata->port);

	conn=PQsetdbLogin(dbdata->host,port,options,NULL,dbdata->name,dbdata->login,dbdata->password);
	if (!conn) { SetError(77); return 0; }

	// Pruefen, ob auch wirklich eine Verbindung da ist
	if (PQstatus((PGconn*)conn) == CONNECTION_BAD) {
		SetError(77,0,PQerrorMessage((PGconn*)conn));
		PQfinish((PGconn*)conn);
		conn=NULL;
		return 0;
	}

	connected=true;
	SetError(0);
	return 1;
}

int	CDBPostgres::Close()
{
	SetError(0);
	if (connected==false) return 0;
	if (conn!=NULL) PQfinish((PGconn*)conn);
	conn=NULL;
	connected=false;
	return 0;
}


ppldd CDBPostgres::Exec(char * query)
{
	double t_start;
	PGresult *res;
	if (connected==false) { SetError(181); return 0; }
	t_start=ppl_getmicrotime();

	ExecStatusType status;
	res=PQexec((PGconn *)conn,query);

	status=PQresultStatus(res);

	if (status==PGRES_COMMAND_OK ||
		status==PGRES_TUPLES_OK ) {

		// Wo kommt die insertid her, falls die Tabelle einen automatischen counter hat?
		//lastinsertid=(ppld64)mysql_insert_id((MYSQL *)mysql);
		lastinsertid=0;
		LogQuery(query,(float)(ppl_getmicrotime()-t_start));
		SetError(0);
		return (ppldd) res;
	}

	// Was war der Fehler?
	SetError(138,0,PQresultErrorMessage(res));
	PQclear(res);
	return 0;
}


ppldd CDBPostgres::ExecEx(char *query, ... )
{
	double t_start;
    char *buff;
	ppldd res;
	if (connected==false) { SetError(181); return 0; }

	buff=(char*)malloc(maxquerybuffer);
	if (buff==NULL) { SetError(2); return 0; }
    va_list args;

    va_start(args, query);
    #ifdef _WIN32
    wvsprintf(buff, query, args);
    #else
    vsprintf (buff, query, args);
    #endif
    va_end(args);

	res=Exec(buff);
	free(buff);
	return res;
}


int CDBPostgres::FreeResult (ppldd res)
{
	PGresult* r=(PGresult *)res;
	if (r!=NULL) PQclear(r);
	SetError(0);
	return 0;
}

ppldd CDBPostgres::NumRows (ppldd res)
{
	PGresult* r=(PGresult*)res;
	if (r!=NULL) return (ppldd) PQntuples(r);
	SetError(182);
	return 0;
}

ppldds CDBPostgres::FieldNum(ppldd res, char * fieldname)
{
	ppldds f;

	PGresult* r=(PGresult*)res;
	if (r==0 || fieldname==0) { SetError(6); return -1; }

	// Das Feld suchen
	f=PQfnumber(r,fieldname);
	if (f>=0) {
		SetError(0);
		return f;
	}
	SetError(183);
	return -1;
}


char * CDBPostgres::Result (ppldd res, ppldd nr, char * feld)
{
	int field_num;
	PGresult* r=(PGresult*)res;
	if (r==0) { SetError(6); return NULL; }

	field_num=PQfnumber(r,feld);
	if (field_num<0) {
		SetError(183);
		return NULL;
	}

	char *rr=PQgetvalue(r,nr,field_num);
	if (!rr) {
		SetError(183);
		return NULL;
	}
	SetError(0);
	return rr;
}



char * CDBPostgres::ResultByNo (ppldd res, ppldd nr, ppldd feld)
{
	ppldd ll;
	PGresult* r=(PGresult*)res;
	if (r==0 || feld <0) { SetError(6); return NULL; }
	char *rr=PQgetvalue(r,nr,feld);
	if (!rr) {
		SetError(183);
		return NULL;
	}
	SetError(0);
	return rr;
}


ppldd CDBPostgres::Errno()
{
	if (!connected) return 0;
	/*
	SetError(mysql_errno((MYSQL *)mysql),mysql_error((MYSQL *)mysql));
	return mysql_errno((MYSQL *)mysql);
	*/
	return 0;
}

#ifdef UNTRANSLATED

const char *CDBPostgres::Error()
{
	if (!connected) return "not connected!";
	//SetError(mysql_errno((MYSQL *)mysql),mysql_error((MYSQL *)mysql));
	return mysql_error((MYSQL *)mysql);
}

#endif

void CDBPostgres::PrintResult(ppldd result)
{
	#ifdef UNTRANSLATED

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

	#endif
}



CAssocArray *CDBPostgres::FetchArray(ppldd res, ppldds nr)
{
	return NULL;

	int num_fields;
	PGresult* r=(PGresult*)res;
	if (r==0) {SetError(6); return NULL;}

	num_fields=PQnfields(r);
	if (!num_fields) {
		SetError(182);
		return NULL;
	}

	if (nr>PQntuples(r)) {
		SetError(222);
		return NULL;
	}

	CAssocArray *array=new CAssocArray();
	for(int i=0; i<num_fields; i++) {
		array->Add(PQfname(r,i),PQgetvalue(r,nr,i));
	}
	SetError(0);
	return array;
}

int CDBPostgres::Ping() {			// 1=Server ist connected, 0=Connection lost
	#ifdef UNTRANSLATED
	if (!connected) return 0;
	if (mysql_ping((MYSQL *)mysql)==0) {		// Server ist noch connected
        SetError(0);
		return 1;
	}
	SetError(181);
	#endif
	return 0;
}

char *CDBPostgres::EscapeString(char *string, size_t length)
{
	if (!string) { SetError(6); return NULL; }
	if (!length) length=strlen(string);
	char *buf=(char *)malloc(length*2+1);   // Buffer reservieren
	ppldd newlength=PQescapeString(buf,string,length);
	return buf;
}


ppld64 CDBPostgres::GetInsertID()
{
	if (!connected) return 0;
	//printf("GetInsertID: %llu\n",lastinsertid);
	return lastinsertid;
}




} // end of namespace ppl


#endif // _HAVE_POSTGRES
