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
#include <windows.h>
#endif
#include "ppl6.h"
#include "ppl6-db.h"

#ifdef HAVE_FREETDS
#include <ctpublic.h>
#endif

namespace ppl6 {
namespace db {

/*!\class Sybase
 * \ingroup PPLGroupDatabases
 * \brief Implementierung einer Sybase-Datenbank
 *
 * \header \#include <ppl6-db.h>
 *
 * \descr
 * Mit dieser Klasse kann eine Verbindung zu einer Sybase-Datenbank aufgebaut werden, um darüber
 * SQL-Queries durchzuführen.
 *
 * \attention
 * Eine Reihe von Funktionen in einem Result werden durch Sybase nicht unterstützt und daher
 * von der Klasse emuliert. Dazu wird das komplette Result-Set in den Hauptspeicher geladen.
 * Dies kann bei Selects mit vielen Ergebniszeilen sehr langsam sein und eine
 * große Menge an Speicher kosten.
 * \par
 * Folgende Funktionen sind davon betroffen und sollten nicht verwendet werden:
 * - db::Result::Rows
 * - db::Result::Get
 * - db::Result::FetchArray mit Angabe einer Zeilennummer
 * - db::Result::FetchFields mit Angabe einer Zeilennummer
 * - db::Result::Seek
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_Sybase_Example1
 * \until EOF
 * \par
 * Liefert ein Select eine große Ergebnismenge, sollte man auf den Aufruf von db::Result::Rows verzichten und stattdessen
 * durch das Ergebnis durchiterieren:
 * \dontinclude db_examples.cpp
 * \skip DB_Sybase_Example2
 * \until EOF
 *
 */


#define PPLSYB_DONE 1
#define PPLSYB_FAILURE 2
#define PPLSYB_RESULTS 3

#ifdef HAVE_FREETDS
static int syb_refcount=0;
static unsigned int ctx_version=CS_VERSION_100;
static CMutex refmutex;
static int maxconnects=0;
static CString default_locale;
static CString default_dateformat;
static size_t resultBufferGrowSize=2048;
#endif


#ifdef HAVE_FREETDS
static CS_CONTEXT				*context=NULL;
static CS_LOCALE				*locale=NULL;

typedef struct tagSybConnect {
	struct tagSybConnect	*previous;
	struct tagSybConnect	*next;
	CS_CONNECTION			*conn;
	CS_COMMAND				*cmd;
	Sybase					*c;
} SYBCONNECT;

typedef struct {
    CS_INT *lengths;
    CS_SMALLINT *indicators;
    char **tmp_buffer;
    unsigned char *numerics;
	int *ppl_type;
    CS_INT *types;
    CS_DATAFMT *datafmt;
    CS_RETCODE last_retcode;
} sybase_result;

static SYBCONNECT *first=NULL;
static SYBCONNECT *last=NULL;

static SYBCONNECT *FindSybClass(CS_CONNECTION *conn)
{
	if (!conn) return NULL;
	refmutex.Lock();
	SYBCONNECT *s=first;
	while (s) {
		if (s->conn==conn) {
			refmutex.Unlock();
			return s;
		}
		s=s->next;
	}
	refmutex.Unlock();
	return NULL;
}

/* Handler for server messages. Client-Library will call this
 * routine when it receives a messages from the server.
 */

CS_RETCODE servermsg_callback(CS_CONTEXT *cp, CS_CONNECTION *chp, CS_SERVERMSG *msgp)
{
	SYBCONNECT *sc=FindSybClass(chp);
	CString msg;
	msg.Sprintf("SYBASE Server Msg %ld, Severity %ld, State %ld, Line %ld: %s",
		(long)msgp->msgnumber,
		(long)msgp->severity,
		(long)msgp->state,
		(long)msgp->line,
		msgp->text);
	if (msgp->svrnlen>0) {
		msg.Concatf(", Server ’%s’, ", msgp->svrname);
	}
	if (msgp->proclen>0) {
		msg.Concatf(", Procedure ’%s’, ", msgp->proc);
	}
	if (sc) {
		CLog *log=sc->c->GetLogfile();
		if (log) log->Print(LOG::DEBUG,5,"ppl6::db::Sybase","servermsg_callback",__FILE__,__LINE__,(const char*)msg.GetPtr());
		sc->c->lastServerMsgNumber=msgp->msgnumber;
		sc->c->syberrorLong=msg;
		sc->c->syberrorLong.Replace("\n"," ");
		sc->c->syberror.Sprintf("%s",msgp->text);
		sc->c->syberror.Replace("\n"," ");
		//printf ("servermsg_callback: syberrorLong: >>%s<<, syberror: >>%s<<\n",(const char*)sc->c->syberrorLong, (const char*)sc->c->syberror);
	}

	return (CS_SUCCEED);
}

/* Client-Library error handler. This function will be invoked
 * when a Client-Library has detected an error. Before Client-
 * Library routines return CS_FAIL, this handler will be called
 * with additional error information.
 */

CS_RETCODE clientmsg_callback(CS_CONTEXT *cp, CS_CONNECTION *chp, CS_CLIENTMSG *msgp)
{
	SYBCONNECT *sc=FindSybClass(chp);
	CString msg;
	msg.Sprintf("SYBASE Client Msg %ld, Severity %ld, Origin %ld, Layer %ld: %s",
		(long) CS_NUMBER(msgp->msgnumber),
		(long) CS_SEVERITY(msgp->severity),
		(long) CS_ORIGIN(msgp->msgnumber),
		(long) CS_LAYER(msgp->msgnumber),
		msgp->msgstring);
	if (msgp->osstringlen>0) {
		msg.Concatf(", Operation System error number %ld: ’%s’, ", (long)msgp->osnumber, msgp->osstring);
	}
	if (sc) {
		CLog *log=sc->c->GetLogfile();
		if (log) log->Print(LOG::DEBUG,5,"ppl6::db::Sybase","clientmsg_callback",__FILE__,__LINE__,(const char*)msg.GetPtr());
		sc->c->lastClientMsgNumber=CS_NUMBER(msgp->msgnumber);
		sc->c->syberrorLong=msg;
		sc->c->syberrorLong.Replace("\n"," ");
		sc->c->syberror.Sprintf("%s",msgp->msgstring);
		sc->c->syberror.Replace("\n"," ");
		//printf ("clientmsg_callback: syberrorLong: >>%s<<, syberror: >>%s<<\n",(const char*)sc->c->syberrorLong, (const char*)sc->c->syberror);
	}
	return (CS_SUCCEED);
}

/* CS-Library error handler. This function will be invoked
 * when CS-Library has detected an error.
 */

CS_RETCODE csmsg_callback(CS_CONTEXT *cp, CS_CLIENTMSG *msgp)
{
	CString msg;
	msg.Sprintf("SYBASE Client Msg %ld, Severity %ld, Origin %ld, Layer %ld: %s",
		(long) CS_NUMBER(msgp->msgnumber),
		(long) CS_SEVERITY(msgp->severity),
		(long) CS_ORIGIN(msgp->msgnumber),
		(long) CS_LAYER(msgp->msgnumber),
		msgp->msgstring);
	if (msgp->osstringlen>0) {
		msg.Concatf(", Operation System error number %ld: ’%s’, ", (long)msgp->osnumber, msgp->osstring);
	}
	msg.Print();
	return (CS_SUCCEED);
}


class SybaseResult : public ppl6::db::Result
{
	friend class Sybase;
	private:
		pplint64	num_rows, num_affected, num_fields;
		pplint64	current_row;
		bool 		bRowsCounted;
		CS_COMMAND	*cmd;
		CLog		*Log;
		CString		Query;
		CMemMan		mem;
		CArray		FieldNames;
		CAssocArray	ResultSet;

		// Der Speicher der nachfolgenden Variablen wird in CMemMan allokiert
	    CS_INT *lengths;
	    CS_SMALLINT *indicators;
	    char **tmp_buffer;
	    unsigned char *numerics;
		int *ppl_type;
	    CS_INT *types;
	    CS_DATAFMT *datafmt;
	    CS_RETCODE last_retcode;


		int			FetchResultFields();
		int			FetchResultSet();

	public:
		SybaseResult();
		~SybaseResult();
		virtual	void Clear();
		virtual pplint64 Affected();
		virtual int Fields();
		virtual const char *FieldName(int field);
		virtual Result::Type	FieldType(int field);
		virtual int FieldNum(const char *fieldname);
		virtual Result::Type	FieldType(const char *fieldname);
		virtual int		Seek(pplint64 row);
		virtual CAssocArray  FetchArray(pplint64 row=-1);
		virtual int		FetchArray(CAssocArray &array, pplint64 row=-1);
		virtual pplint64 Rows();
		virtual CArray  FetchFields(pplint64 row=-1);
		virtual int		FetchFields(CArray &array, pplint64 row=-1);
		virtual const char *Get(pplint64 row, const char *fieldname);
		virtual const char *Get(pplint64 row, int field);

		/*
		virtual void    PrintResult();
		*/
};

SybaseResult::SybaseResult()
{
	bRowsCounted=false;
	num_rows=0;
	num_affected=0;
	num_fields=0;
	cmd=NULL;
	Log=NULL;
	lengths=NULL;
	indicators=NULL;
	numerics=NULL;
	tmp_buffer=NULL;
	ppl_type=NULL;
	types=NULL;
	datafmt=NULL;
	last_retcode=0;
	current_row=0;
}

SybaseResult::~SybaseResult()
{
	Clear();
}

void SybaseResult::Clear()
{
	bRowsCounted=false;
	num_rows=0;
	num_affected=0;
	num_fields=0;
	Log=NULL;
	if (cmd) {
		CS_RETCODE retcode;
		CS_INT restype;

		//ct_cancel(NULL, cmd, CS_CANCEL_CURRENT);
		ct_cancel(NULL, cmd, CS_CANCEL_ALL);
		while ((retcode = ct_results(cmd, &restype))==CS_SUCCEED) {
			ct_cancel(NULL, cmd, CS_CANCEL_ALL);
		}
		cmd=NULL;
	}
	mem.Clear();
	lengths=NULL;
	indicators=NULL;
	numerics=NULL;
	tmp_buffer=NULL;
	ppl_type=NULL;
	types=NULL;
	datafmt=NULL;
	last_retcode=0;
	FieldNames.Clear();
	ResultSet.Clear();
	current_row=0;
}



int SybaseResult::FetchResultFields()
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	CS_INT fields;
	//CS_INT retcode;
	if (Log) Log->Printf(LOG::DEBUG,5,"ppl6::db::SybaseResult","FetchFields",__FILE__,__LINE__,"Query=%s)",(const char*)Query);
	if (ct_res_info(cmd, CS_NUMDATA, &fields, CS_UNUSED, NULL)!=CS_SUCCEED) {
		return 0;
	}
	num_fields=fields;

	tmp_buffer = (char **) mem.Calloc(num_fields,sizeof(char *));
	datafmt = (CS_DATAFMT *) mem.Calloc(num_fields,sizeof(CS_DATAFMT));
	ppl_type = (int *) mem.Calloc(num_fields,sizeof(int));
	lengths = (CS_INT *) mem.Calloc(num_fields,sizeof(CS_INT));
	indicators = (CS_SMALLINT *) mem.Calloc(num_fields,sizeof(CS_INT));
	numerics = (unsigned char *) mem.Calloc(num_fields,sizeof(unsigned char));

	for (int i=0; i<num_fields; i++) {
		ct_describe(cmd, i+1, &datafmt[i]);
		ppl_type[i]=ppl6::db::Result::Unknown;
		switch (datafmt[i].datatype) {
			case CS_CHAR_TYPE:
			case CS_VARCHAR_TYPE:
			case CS_TEXT_TYPE:
			case CS_IMAGE_TYPE:
				datafmt[i].maxlength++;
				ppl_type[i]=ppl6::db::Result::String;
				//result->numerics[i] = 0;
				break;
			case CS_BINARY_TYPE:
	            case CS_VARBINARY_TYPE:
	                datafmt[i].maxlength *= 2;
	                datafmt[i].maxlength++;
	                //result->numerics[i] = 0;
					ppl_type[i]=ppl6::db::Result::Binary;
	                break;
	            case CS_BIT_TYPE:
	            case CS_TINYINT_TYPE:
	                datafmt[i].maxlength = 4;
					ppl_type[i]=ppl6::db::Result::Integer;
	                //result->numerics[i] = 1;
	                break;
	            case CS_SMALLINT_TYPE:
	                datafmt[i].maxlength = 7;
					ppl_type[i]=ppl6::db::Result::Integer;
	                //result->numerics[i] = 1;
	                break;
	            case CS_INT_TYPE:
	                datafmt[i].maxlength = 12;
					ppl_type[i]=ppl6::db::Result::Integer;
	                //result->numerics[i] = 1;
	                break;
	            case CS_REAL_TYPE:
	            case CS_FLOAT_TYPE:
	                datafmt[i].maxlength = 24;
					ppl_type[i]=ppl6::db::Result::Decimal;
	                //result->numerics[i] = 2;
	                break;
	            case CS_MONEY_TYPE:
	            case CS_MONEY4_TYPE:
	                datafmt[i].maxlength = 24;
					ppl_type[i]=ppl6::db::Result::Decimal;
	                //result->numerics[i] = 2;
	                break;
	            case CS_DATETIME_TYPE:
	            case CS_DATETIME4_TYPE:
	                datafmt[i].maxlength = 30;
					ppl_type[i]=ppl6::db::Result::Integer;
	                //result->numerics[i] = 0;
	                break;
	            case CS_NUMERIC_TYPE:
	            case CS_DECIMAL_TYPE:
	                datafmt[i].maxlength = datafmt[i].precision + 3;
					ppl_type[i]=ppl6::db::Result::Decimal;
	                /* numeric(10) vs numeric(10, 1) */
	                //result->numerics[i] = (result->datafmt[i].scale == 0) ? 3 : 2;
	                break;
	            default:
	                datafmt[i].maxlength++;
					ppl_type[i]=ppl6::db::Result::String;
	                //result->numerics[i] = 0;
	                break;
	        }
			tmp_buffer[i] = (char *)mem.Calloc(1,datafmt[i].maxlength);
	        datafmt[i].datatype = CS_CHAR_TYPE;
	        datafmt[i].format = CS_FMT_NULLTERM;
	        ct_bind(cmd, i+1, &datafmt[i], tmp_buffer[i], &lengths[i], &indicators[i]);
			//sybres->SetFieldName(i,result->datafmt[i].name,result->datafmt[i].namelen,ppl6::db::Result::String);
	        FieldNames.Set(i,datafmt[i].name,datafmt[i].namelen);

			if (Log) Log->Printf(LOG::DEBUG,5,"ppl6::db::Sybase","FetchResult",__FILE__,__LINE__,"Field %i: Type: %i, Name: %s, Maxlength: %i",
				i,ppl_type[i],FieldName(i),datafmt[i].maxlength);
		}
		return 1;
#endif

}

pplint64 SybaseResult::Affected()
{
	return num_affected;
}

int SybaseResult::Fields()
{
	return num_fields;
}

const char *SybaseResult::FieldName(int field)
{
	if (field>=num_fields) {
		SetError(183,"%i",field);
		return NULL;
	}
	return FieldNames.Get(field);
}

ppl6::db::Result::Type SybaseResult::FieldType(int field)
{
	if (field>=num_fields) {
		SetError(183,"%i",field);
		return Error;
	}
	return (Type)ppl_type[field];
}

int SybaseResult::FieldNum(const char *fieldname)
{
	for (int i=0;i<num_fields;i++) {
		if (strcmp(FieldNames.Get(i),fieldname)==0) return i;
	}
	SetError(183,"%s",fieldname);
	return -1;
}

ppl6::db::Result::Type SybaseResult::FieldType(const char *fieldname)
{
	int field=FieldNum(fieldname);
	if (field<0) return Result::Unknown;
	return FieldType(field);
}

/*!\brief Internen Zeiger auf die gewünschte Ergebniszeile setzen
 *
 * Mit dieser Funktion wird der interne Datenzeiger auf die gewünschte Zeile \p row gesetzt,
 * so dass diese beim nächsten Aufruf von Result::FetchArray zurückgeliefert wird.
 *
 * @param[in] row Die gewünschte Zeile
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \attention
 * Sybase verfügt über keine Seek-Funktion. Daher wird das komplette Result-Set in
 * den Speicher geladen um das Kommando dort ausführen zu können.
 * Dies kann bei Selects mit vielen Ergebniszeilen sehr langsam sein und eine
 * große Menge an Speicher kosten.
 * \par
 * Wenn möglich sollte der Aufruf dieser Funktion vermieden werden!
 *
 */
int SybaseResult::Seek(pplint64 row)
{
	if (!bRowsCounted) {
		if (!FetchResultSet()) return 0;
	}
	if (row>=0 && row<num_rows) {
		current_row=row;
		return 1;
	}
	SetError(362,"SybaseResult::Seek(row=%llu)",row);
	return 0;
}

CAssocArray SybaseResult::FetchArray(pplint64 row)
{
	CAssocArray a;
	FetchArray(a,row);
	return a;
}

/*!\brief Anzahl Zeilen im Ergebnis
 *
 * Diese Funktion liefert die Anzahl Zeilen im Ergebnis des vorhergehenden Selects zurück.
 * @return Anzahl Zeilen (rows). Im Fehlerfall wird -1 zurückgegeben.
 *
 * \attention
 * Sybase verfügt über keine Funktion, mit der die Anzahl Zeilen im Ergebnis ermittelt werden
 * können. Daher wird das komplette Result-Set in den Speicher geladen und so die Zeilen
 * gezählt. Dies kann bei Selects mit vielen Ergebniszeilen sehr langsam sein und eine
 * große Menge an Speicher kosten.
 * \par
 * Wenn möglich sollte der Aufruf dieser Funktion vermieden werden!
 *
 * \see
 * Result::Fields gibt die Anzahl Spalten im Ergebnis zurück
 */
pplint64 SybaseResult::Rows()
{
	if (!bRowsCounted) {
		if (!FetchResultSet()) return 0;
	}
	return ResultSet.Count();
}

int SybaseResult::FetchResultSet()
{
	ResultSet.Clear();
	num_rows=0;
	current_row=0;
	bRowsCounted=false;
	if (!cmd) {
		ppl6::SetError(300,"SybaseResult::FetchArray");
		return 0;
	}

	// Zeile holen
	ppl6::CString Key;
	CS_RETCODE retcode;
	CS_INT restype;
	while ((retcode = ct_results(cmd, &restype))==CS_SUCCEED) {
		switch ((int) restype) {
			case CS_COMPUTE_RESULT:
			case CS_CURSOR_RESULT:
			case CS_PARAM_RESULT:
			case CS_ROW_RESULT:
			case CS_STATUS_RESULT:
				if (ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, NULL)==CS_SUCCEED) {
					ppl6::CAssocArray array;
					for (int i=0; i<num_fields; i++) {
						array.Set(FieldNames.Get(i),tmp_buffer[i],lengths[i]);
					}
					Key.Setf("%llu",num_rows);
					ResultSet.Set(Key,array);
					num_rows++;
				}
				break;
			case CS_CMD_FAIL: {
				CS_INT row_count;
				if (ct_res_info(cmd, CS_ROW_COUNT, &row_count, CS_UNUSED, NULL)==CS_SUCCEED) {
					num_affected=(pplint64)row_count;
				}
				SetError(138,"SybaseResult::FetchArray");
				return 0;
			}
			case CS_CMD_SUCCEED:
			case CS_CMD_DONE: {
				CS_INT row_count;
				if (ct_res_info(cmd, CS_ROW_COUNT, &row_count, CS_UNUSED, NULL)==CS_SUCCEED) {
					num_affected=(pplint64)row_count;					}
				bRowsCounted=true;
				/* no break */
			}
			/* no break */
			/* Fall through */
			case CS_COMPUTEFMT_RESULT:
			case CS_ROWFMT_RESULT:
			case CS_DESCRIBE_RESULT:
			case CS_MSG_RESULT:
				return 1;
				break;
			default:
				printf ("Ups, restype=%i\n",restype);
				break;
		}
	}
	ppl6::SetError(300,"SybaseResult::FetchArray");
	return 0;
}

int SybaseResult::FetchArray(CAssocArray &array, pplint64 row)
{
	array.Clear();
	if (row>=0) {
		if (!bRowsCounted) {
			if (!FetchResultSet()) return 0;
		}
	}
	if (ResultSet.Count()) {
		CString Key;
		if (row<0) row=current_row;
		Key.Setf("%llu",row);
		ppl6::CAssocArray *r=ResultSet.GetArray(Key);
		if (!r) {
			ppl6::SetError(300,"SybaseResult::FetchArray");
			return 0;
		}
		array.Copy(r);
		current_row=row+1;
		return 1;
	}
	// Zeile holen
	CS_RETCODE retcode;
	CS_INT restype;
	while ((retcode = ct_results(cmd, &restype))==CS_SUCCEED) {
		switch ((int) restype) {
			case CS_COMPUTE_RESULT:
			case CS_CURSOR_RESULT:
			case CS_PARAM_RESULT:
			case CS_ROW_RESULT:
			case CS_STATUS_RESULT:
				if (ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, NULL)==CS_SUCCEED) {
					for (int i=0; i<num_fields; i++) {
						array.Set(FieldNames.Get(i),tmp_buffer[i],lengths[i]);
						//if (Log) Log->Printf(LOG::DEBUG,10,"ppl6::db::Sybase","FetchResult",__FILE__,__LINE__,"Row: %i, Feld %i, Length: %i: >>>%s<<<, UsedMem: %llu",
						//		z,i,result->lengths[i],result->tmp_buffer[i],sybres->GetUsedMem());
					}
					return 1;
				}
				return 0;
				break;
			case CS_CMD_FAIL: {
				CS_INT row_count;
				if (ct_res_info(cmd, CS_ROW_COUNT, &row_count, CS_UNUSED, NULL)==CS_SUCCEED) {
					num_affected=(pplint64)row_count;
				}
				SetError(138,"SybaseResult::FetchArray");
				return 0;
			}
            case CS_CMD_SUCCEED:
            case CS_CMD_DONE: {
					CS_INT row_count;
					if (ct_res_info(cmd, CS_ROW_COUNT, &row_count, CS_UNUSED, NULL)==CS_SUCCEED) {
						num_affected=(pplint64)row_count;					}
				}
				/* no break */
				/* Fall through */
			case CS_COMPUTEFMT_RESULT:
			case CS_ROWFMT_RESULT:
			case CS_DESCRIBE_RESULT:
			case CS_MSG_RESULT:
				ppl6::SetError(300,"SybaseResult::FetchArray");
				return 0;
				break;
			default:
				printf ("Ups, restype=%i\n",restype);
				break;
		}
	}
	ppl6::SetError(300,"SybaseResult::FetchArray");
	return 0;
}

CArray SybaseResult::FetchFields(pplint64 row)
{
	CArray a;
	FetchFields(a,row);
	return a;
}

int SybaseResult::FetchFields(CArray &array, pplint64 row)
{
	array.Clear();
	if (row>=0) {
		if (!bRowsCounted) {
			if (!FetchResultSet()) return 0;
		}
	}
	if (ResultSet.Count()) {
		CString Key;
		if (row<0) row=current_row;
		Key.Setf("%llu",row);
		ppl6::CAssocArray *r=ResultSet.GetArray(Key);
		if (!r) {
			ppl6::SetError(300,"SybaseResult::FetchArray");
			return 0;
		}
		for (int i=0; i<num_fields; i++) {
			array.Set(i,r->Get(FieldNames.Get(i)));
		}
		current_row=row+1;
		return 1;
	}
	// Zeile holen
	CS_RETCODE retcode;
	CS_INT restype;
	while ((retcode = ct_results(cmd, &restype))==CS_SUCCEED) {
		switch ((int) restype) {
			case CS_COMPUTE_RESULT:
			case CS_CURSOR_RESULT:
			case CS_PARAM_RESULT:
			case CS_ROW_RESULT:
			case CS_STATUS_RESULT:
				if (ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, NULL)==CS_SUCCEED) {
					for (int i=0; i<num_fields; i++) {
						array.Set(i,tmp_buffer[i],lengths[i]);
						//if (Log) Log->Printf(LOG::DEBUG,10,"ppl6::db::Sybase","FetchResult",__FILE__,__LINE__,"Row: %i, Feld %i, Length: %i: >>>%s<<<, UsedMem: %llu",
						//		z,i,result->lengths[i],result->tmp_buffer[i],sybres->GetUsedMem());
					}
					return 1;
				}
				return 0;
				break;
			case CS_CMD_FAIL: {
				CS_INT row_count;
				if (ct_res_info(cmd, CS_ROW_COUNT, &row_count, CS_UNUSED, NULL)==CS_SUCCEED) {
					num_affected=(pplint64)row_count;
				}
				SetError(138,"SybaseResult::FetchArray");
				return 0;
			}
            case CS_CMD_SUCCEED:
            case CS_CMD_DONE: {
					CS_INT row_count;
					if (ct_res_info(cmd, CS_ROW_COUNT, &row_count, CS_UNUSED, NULL)==CS_SUCCEED) {
						num_affected=(pplint64)row_count;					}
				}
				/* no break */
				/* Fall through */
			case CS_COMPUTEFMT_RESULT:
			case CS_ROWFMT_RESULT:
			case CS_DESCRIBE_RESULT:
			case CS_MSG_RESULT:
				ppl6::SetError(300,"SybaseResult::FetchArray");
				return 0;
				break;
			default:
				printf ("Ups, restype=%i\n",restype);
				break;
		}
	}
	ppl6::SetError(300,"SybaseResult::FetchArray");
	return 0;
}

const char *SybaseResult::Get(pplint64 row, const char *fieldname)
{
	if (!Seek(row)) return 0;
	CString Key;
	if (row<0) row=current_row;
	Key.Setf("%llu",row);
	ppl6::CAssocArray *r=ResultSet.GetArray(Key);
	if (!r) {
		ppl6::SetError(300,"SybaseResult::FetchArray");
		return NULL;
	}
	return r->Get(fieldname);
}

const char *SybaseResult::Get(pplint64 row, int field)
{
	if (!Seek(row)) return 0;
	CString Key;
	if (row<0) row=current_row;
	Key.Setf("%llu",row);
	ppl6::CAssocArray *r=ResultSet.GetArray(Key);
	if (!r) {
		ppl6::SetError(300,"SybaseResult::FetchArray");
		return NULL;
	}
	return r->Get(FieldName(field));
}



#endif

Sybase::Sybase()
{
	lastinsertid=0;
	lastServerMsgNumber=0;
	lastClientMsgNumber=0;
	connected=false;
	transactiondepth=0;
	maxrows=0;
	rows_affected=0;
	conn=NULL;
#ifdef HAVE_FREETDS
	conn=malloc(sizeof(SYBCONNECT));
	if (!conn) {
		SetError(2,"malloc conn");
		return;
	}
	SYBCONNECT *sc=(SYBCONNECT*)conn;
	sc->previous=NULL;
	sc->next=NULL;
	sc->conn=NULL;
	sc->cmd=NULL;
	sc->c=this;

	refmutex.Lock();
	if (syb_refcount==0) {
		CS_RETCODE ret;
		// eine Context-Struktur allokieren
		context=NULL;
		ret=cs_ctx_alloc(ctx_version,&context);
		if (ret!=CS_SUCCEED) {
			SetError(2,"cs_ctx_alloc");
			refmutex.Unlock();
			return;
		};
		// Install a callback function to handle CS-Library errors
		ret=cs_config(context, CS_SET, CS_MESSAGE_CB,(CS_VOID*)csmsg_callback, CS_UNUSED, NULL);
		if (ret!=CS_SUCCEED) {
			//ct_exit(context, CS_FORCE_EXIT);
			cs_ctx_drop(context);
			context=NULL;
			refmutex.Unlock();
			SetError(296,"cs_config, csmsg_callback");
			return;
		}
		// Install a callback function to handle Client-Library errors
		ret=ct_callback(context, NULL, CS_SET, CS_CLIENTMSG_CB,(CS_VOID*) clientmsg_callback);
		if (ret!=CS_SUCCEED) {
			//ct_exit(context, CS_FORCE_EXIT);
			cs_ctx_drop(context);
			context=NULL;
			refmutex.Unlock();
			SetError(296,"ct_callback, clientmsg_callback");
			return;
		}
		// Install a callback function to handle server messages
		ret=ct_callback(context, NULL, CS_SET, CS_SERVERMSG_CB,(CS_VOID*) servermsg_callback);
		if (ret!=CS_SUCCEED) {
			//ct_exit(context, CS_FORCE_EXIT);
			cs_ctx_drop(context);
			context=NULL;
			refmutex.Unlock();
			SetError(296,"ct_callback, servermsg_callback");
			return;
		}

		/*
		 * Locales setzen
		 */

		ret=cs_loc_alloc(context, &locale);
		if (ret!=CS_SUCCEED) {
			cs_ctx_drop(context);
			context=NULL;
			refmutex.Unlock();
			SetError(296,"cs_loc_alloc");
			return;
		}
		if (default_locale.NotEmpty()) {
			// CS_LC_ALL ist in FreeTDS 0.82 nicht implementiert, und liefert immer CS_FAIL zurück.
			// Daher prüfen wir den erstmal Returncode nicht.
			// TODO: Configure soll ein Define zur Erkennung von FreeTDS liefern, dann kann die
			// Prüfung abhängig vom Flag erfolgen

			ret=cs_locale(context,CS_SET,locale,CS_SYB_LANG_CHARSET,(CS_VOID*)default_locale.GetPtr(),CS_NULLTERM,NULL);
			//ret=cs_locale(context,CS_SET,locale,CS_SYB_LANG,(CS_VOID*)"german",CS_NULLTERM,NULL);

			//ret=cs_locale(context,CS_SET,locale,CS_LC_ALL,(CS_VOID*)default_locale.GetPtr(),CS_NULLTERM,NULL);
			//ret=cs_locale(context,CS_SET,locale,CS_SYB_CHARSET,(CS_VOID*)default_locale.GetPtr(),CS_NULLTERM,NULL);
			//ret=cs_locale(context,CS_SET,locale,CS_SYB_LANG,(CS_VOID*)default_locale.GetPtr(),CS_NULLTERM,NULL);
			//ret=cs_locale(context,CS_SET,locale,CS_SYB_CHARSET,(CS_VOID*)default_locale.GetPtr(),CS_NULLTERM,NULL);

			if (ret!=CS_SUCCEED) {
				cs_loc_drop(context,locale);
				locale=NULL;
				cs_ctx_drop(context);
				refmutex.Unlock();
				SetError(296,"cs_locale: %s",(const char*)default_locale);
				return;
			}

		}


		/* Set datetime conversion format to "Nov  3 1998  8:06PM".
		 * This is the default format for the ct-lib that comes with
		 * Sybase ASE 11.5.1 for Solaris, but the Linux libraries that
		 * come with 11.0.3.3 default to "03/11/98" which is singularly
		 * useless.  This levels the playing field for all platforms.
		 */
		CS_INT dt_convfmt = CS_DATES_SHORT;
		//CS_INT dt_convfmt = CS_DATES_MDYHMS;

		// Auch diese Funktion ist in FreeTDS nicht implementiert, liefert aber immer SUCCEED zurück
		ret=cs_dt_info(context, CS_SET, NULL, CS_DT_CONVFMT, CS_UNUSED, &dt_convfmt, sizeof(dt_convfmt), NULL);
		if (ret!=CS_SUCCEED) {
			//ct_exit(context, CS_FORCE_EXIT);
			cs_loc_drop(context,locale);
			cs_ctx_drop(context);
			refmutex.Unlock();
			context=NULL;
			SetError(296,"Setting date format");
			return;
		}


		// Datenbank-Context initialisieren
		ret=ct_init(context, ctx_version);
		if (ret!=CS_SUCCEED) {
			cs_loc_drop(context,locale);
			cs_ctx_drop(context);
			context=NULL;
			locale=NULL;
			refmutex.Unlock();
			SetError(296,"ct_init");
			return;
		}

		#ifdef CS_MAX_CONNECT
		if (maxconnects==0) {
			ret=ct_config(context,CS_GET,CS_MAX_CONNECT,&maxconnects,CS_UNUSED,NULL);
		} else {
			ret=ct_config(context,CS_SET,CS_MAX_CONNECT,&maxconnects,CS_UNUSED,NULL);
		}
		#endif
		int timeout=5;
		ret=ct_config(context,CS_SET,CS_LOGIN_TIMEOUT,&timeout,CS_UNUSED,NULL);
		/*
		if (ret==CS_FAIL) {
			printf ("Error\n");
		} else {
			printf ("ok\n");
		}
		ret=ct_config(context,CS_GET,CS_LOGIN_TIMEOUT,&timeout,CS_UNUSED,NULL);
		if (ret==CS_FAIL) {
			printf ("Error\n");
		} else {
			printf ("ok: %i\n",timeout);
		}
		*/

	}
	SetError(0);
	syb_refcount++;
	refmutex.Unlock();
#endif
}

Sybase::~Sybase()
{
#ifdef HAVE_FREETDS
	PushError();
	Close();
	CLog *Log=GetLogfile();
	if (Log) Log->Printf(LOG::DEBUG,4,"ppl6::db::Sybase","~Sybase",__FILE__,__LINE__,"Objekt wird gelöscht");
	if (conn) free(conn);
	conn=NULL;
	refmutex.Lock();
	syb_refcount--;
	if (syb_refcount==0) {
		ct_exit(context, CS_FORCE_EXIT);
		cs_loc_drop(context,locale);
		cs_ctx_drop(context);
		context=NULL;
		locale=NULL;
	}
	refmutex.Unlock();
	PopError();
#endif
}

int Sybase::Connect(const CAssocArray &params)
/*!\brief Verbindung zur Datenbank herstellen
 *
 * \desc
 * Mit dieser Funktion wird eine Verbindung zu einer Sybase-Datenbank hergestellt.
 *
 * \param[in] params Ein Assoziatives Array, das sämtliche Connect-Parameter enthält.
 * Folgende Paramater sind anzugeben:
 * <table border=1 cellspacing=0 cellpadding=2>
 * <tr class=table-row><td class=table-header>interface</td><td>optional</td><td>Der Name des Datenbank-Servers, wie er in der freetds.conf oder der Sybase Interface-Datei
 * definiert ist. Statt des Interface-Namens kann auch eine Kombination aus \p host und \p port verwendet werden.
 * Wird weder \p host noch \p interface angegeben, wird mit dem in der Interface-Datei angegebenen Default-Server verbunden oder
 * es kommt zu einer Fehlermeldung</td></tr>
 * <tr class=table-row><td class=table-header>host</td><td>optional</td><td>Falls der Interface-Name des Datenbank-Servers nicht mit \p interface angegeben wird,
 * kann hiermit alternativ ein FQDN oder die IP des Servers angegeben werden. In diesem Fall muss auch die Variable \p port
 * angegeben werden</td></tr>
 * <tr class=table-row><td class=table-header>port</td><td>optional</td><td>Falls \p host verwendet wird, muss auch \p port angegeben werden</td></tr>
 * <tr class=table-row><td class=table-header>user</td><td>erforderlich</td><td>Der Benutzername (Login), mit dem man sich am Sybase-Server anmelden möchte</td></tr>
 * <tr class=table-row><td class=table-header>password</td><td>erforderlich</td><td>Das Passwort des Benutzers</td></tr>
 * <tr class=table-row><td class=table-header>dbname</td><td>optional</td><td>Name der Datenbank. Kann auch später noch mit Sybase::SelectDB
 * oder per SQL Query "use DATENBANKNAME" gewählt werden.</td></tr>
 * <tr class=table-row><td class=table-header>appname</td><td>optional</td><td>Name der Applikation. Dieser wird an den Sybase-Server
 * übermittelt und kann dort eventuell zu einer einfacheren identifizierung der Prozesse beitragen</td></tr>
 * <tr class=table-row><td class=table-header>packetsize</td><td>optional</td><td>Legt die Maximale Größe für Datenpakete fest, die der Server
 * dem Client schicken darf. Muss ein Wert zwischen 512 und 65535 sein, je nach Konfiguration des Servers kann das
 * Limit jedoch auch weitaus niedriger sein.</td></tr>
 * <tr class=table-row><td class=table-header>tdsversion</td><td>optional</td><td>Überschreibt die in der Interface-Datei bzw. freetds.conf
 * angegebene TDS-Version. Kann folgende Werte enthalten: 4.0, 4.2, 4.6, 4.95 oder 5.0 enthalten</td></tr>
 * <tr class=table-row><td class=table-header>language</td><td>optional</td><td>Legt fest, welche Sprache verwendet
 * werden soll, beispielswiese "us_english" oder "german". Mit der Funktion ppl6::SybaseSetLocale kann aber auch ein
 * Prozessweiter Default gesetzt werden.</td></tr>
 * <tr class=table-row><td class=table-header>dateformat</td><td>optional</td><td>Legt fest, in welcher Reihenfolge
 * Tag, Monat und Jahr angegeben werden. Mögliche Werte sind: mdy, dmy, ymd, ydm, myd, und dym.
 * Mit der Funktion ppl6::SybaseSetLocale kann aber auch ein
 * Prozessweiter Default gesetzt werden.</td></tr>
 *
 * </table>
 *
 * \returns Bei einem erfolgreichen Connect gibt die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \note
 * Das Verhalten des Parameters \p host hat sich im Vergleich zu der bisherigen Klasse CDBSybase verändert. Früher wurde hier der Name
 * des Interfaces erwartet (dieser muss nun mit dem Parameter \p interface angegeben werden), jetzt ist es der Hostname bzw. IP-Adresse
 * des Servers. Diese Änderung wurde gemacht, um die Connect-Parameter zu vereinheitlichen.
 */
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	refmutex.Lock();
	if (syb_refcount==0) {
		SetError(296);
		refmutex.Unlock();
		return 0;
	}
	refmutex.Unlock();
	Close();
	condata=params;
	if (!conn) {
		SetError(2);
		return 0;
	}
	SYBCONNECT *sc=(SYBCONNECT*)conn;
	const char *tmp;
	const char *interface=params.Get("interface");
	const char *host=params.Get("host");
	tmp=params.Get("port");
	unsigned int port=0;
	if (tmp) port=ppl6::atoi(tmp);
	const char *user=params.Get("user");
	const char *password=params.Get("password");
	const char *dbname=params.Get("dbname");
	const char *appname=params.Get("appname");
	int packetsize=ppl6::atoi(params.Get("packetsize"));
	const char *tdsversion=params.Get("tdsversion");
	const char *language=params.Get("language");
	const char *dateformat=params.Get("dateformat");


	//char *charset=data->Get("charset");
	CS_RETCODE ret;
	sc->conn=NULL;
	// Zuerst eine connection structure allokieren
	lastServerMsgNumber=0;
	ret=ct_con_alloc(context,&sc->conn);
	if (ret!=CS_SUCCEED) {
		SetError(77,"ct_con_alloc failed");
		return 0;
	}

	if (user) {
		ret=ct_con_props(sc->conn,CS_SET,CS_USERNAME,(CS_VOID*)user,CS_NULLTERM,NULL);
		if (ret!=CS_SUCCEED) {
			SetError(77,"ct_con_props for CS_USERNAME failed");
			return 0;
		}
	}
	if (password) {
		ret=ct_con_props(sc->conn,CS_SET,CS_PASSWORD,(CS_VOID*)password,CS_NULLTERM,NULL);
		if (ret!=CS_SUCCEED) {
			SetError(77,"ct_con_props for CS_PASSWORD failed");
			return 0;
		}
	}
	//if (host) DBSETLHOST(login, host);
	if (appname) {
		ret=ct_con_props(sc->conn,CS_SET,CS_APPNAME,(CS_VOID*)appname,CS_NULLTERM,NULL);
		if (ret!=CS_SUCCEED) {
			SetError(77,"ct_con_props for CS_APPNAME failed");
			return 0;
		}

	}
#ifdef CS_PACKETSIZE
	int p=2048;
	if (packetsize>=512) p=packetsize;
	ret=ct_con_props(sc->conn,CS_SET,CS_PACKETSIZE,&p,CS_UNUSED,NULL);
	if (ret!=CS_SUCCEED) {
		SetError(77,"ct_con_props for CS_PACKETSIZE failed");
		return 0;
	}
#endif

#ifdef CS_TDS_VERSION
	if (tdsversion) {
		CString t=tdsversion;
		CS_INT v=0;
		if (t=="4.0") v=CS_TDS_40;
		if (t=="4.2") v=CS_TDS_42;
		if (t=="4.6") v=CS_TDS_46;
		if (t=="4.95") v=CS_TDS_495;
		if (t=="5.0") v=CS_TDS_50;
		if (v) {
			ret=ct_con_props(sc->conn,CS_SET,CS_TDS_VERSION,&v,CS_UNUSED,NULL);
			if (ret!=CS_SUCCEED) {
				SetError(77,"ct_con_props for CS_TDS_VERSION failed");
				return 0;
			}
		}

	}

#endif

#ifdef CS_SERVERADDR
	if (host!=NULL && port>0) {
		CString t;
		t.Setf("%s %i",host,port);
		ret=ct_con_props(sc->conn,CS_SET,CS_SERVERADDR,(CS_VOID*)t.GetPtr(),CS_NULLTERM,NULL);
		if (ret!=CS_SUCCEED) {
			SetError(77,"ct_con_props for CS_SERVERADDR failed");
			return 0;
		}
		//ret=ct_con_props(sc->conn,CS_SET,CS_HOSTNAME,(CS_VOID*)host,CS_NULLTERM,NULL);

	}
#endif

	refmutex.Lock();
	if (last) last->next=sc;
	sc->next=NULL;
	sc->previous=last;
	last=sc;
	if (!first) first=sc;
	refmutex.Unlock();
	lastServerMsgNumber=0;
	lastClientMsgNumber=0;
	ret = ct_connect(sc->conn, (CS_CHAR*)interface, CS_NULLTERM);
	if (ret!=CS_SUCCEED) {
		if (ret==CS_FAIL) printf ("CS_FAIL\n");
		if (ret==CS_PENDING) printf ("CS_PENDING\n");
		if (ret==CS_BUSY) printf ("CS_BUSY\n");

		int e=77;
		switch (lastServerMsgNumber) {
			case 4002:
				e=542;
				syberror="Login failed";
				break;
			default:
				e=77;
				break;
		};
		//printf ("conn 3\n");
		SetError(e,"Host: %s, Port: %i, Interface: %s, Sybase-Error: %s",host,port,interface,(const char*)syberror);
		return 0;
	}
	sc->cmd=NULL;
	lastServerMsgNumber=0;
	lastClientMsgNumber=0;
	ret = ct_cmd_alloc(sc->conn, &sc->cmd);
	if (ret!=CS_SUCCEED) {
		Close();
		SetError(77,"Command Buffer ct_cmd_alloc");
		return 0;
	}
	CLog *log=GetLogfile();
	if (log) log->Printf(LOG::DEBUG,3,__FILE__,__LINE__,"SYBASE Connected to %s",host);
	connected=true;
	UpdateLastUse();
	if (language) Execf("set language %s",language);
	else if (default_locale.NotEmpty()) Execf("set language %s",(const char*)default_locale);
	if (dateformat) Execf("set dateformat %s",dateformat);
	else if (default_dateformat.NotEmpty()) Execf("set dateformat %s", (const char*)default_dateformat);

	if (dbname) return SelectDB(dbname);
	return 1;
#endif
}

int Sybase::Reconnect()
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	Close();
	CAssocArray a;
	a.Copy(&condata);
	if (Connect(a)) return 1;
	SetError(299,"Reconnect failed");
	return 0;
#endif
}

int Sybase::Disconnect()
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	CLog *Log=GetLogfile();
	if (Log) Log->Printf(LOG::DEBUG,4,"ppl6::db::Sybase","Disconnect",__FILE__,__LINE__,"Trenne Verbindung");

	SYBCONNECT *sc=(SYBCONNECT*)conn;
	if (sc) {
		refmutex.Lock();
		if (sc->previous) sc->previous->next=sc->next;
		if (sc->next) sc->next->previous=sc->previous;
		if (sc==first) first=sc->next;
		if (sc==last) last=sc->previous;
		refmutex.Unlock();
		sc->next=NULL;
		sc->previous=NULL;
		if (sc->cmd) {
			ct_cmd_drop(sc->cmd);
			sc->cmd=NULL;
		}
		if (sc->conn) {
			ct_close(sc->conn,CS_UNUSED);
			ct_con_drop(sc->conn);
			sc->conn=NULL;
		}
		if (Log) Log->Printf(LOG::DEBUG,10,"ppl6::db::Sybase","Disconnect",__FILE__,__LINE__,"Verbindung wurde getrennt");
	} else {
		if (Log) Log->Printf(LOG::DEBUG,10,"ppl6::db::Sybase","Disconnect",__FILE__,__LINE__,"Es war keine Verbindung vorhanden");
	}
	connected=false;
	lastinsertid=0;
	ClearLastUse();
	return 1;
#endif
}

int Sybase::SelectDB(const char *databasename)
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	if (!databasename) {
		SetError(194,"SelectDB(CString *databasename)");
		return 0;
	}
	int ret=0;
	SYBCONNECT *sc=(SYBCONNECT*)conn;
	if (!sc->conn) {
		SetError(181);
		return 0;
	} else {
		CString cmdbuf;
		cmdbuf.Sprintf("use %s", databasename);
		if (Exec(cmdbuf.GetPtr())) {
			ret=1;
		}
		else SetError(295,"%s",databasename);
	}
	return ret;
#endif
}

int Sybase::Exec(const CString &query)
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	db::Result *res=Query(query);
	if (!res) return 0;
	delete res;
	return 1;
#endif
}

ppl6::db::Result *Sybase::Query(const CString &query)
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	double t_start;
	CLog *Log=GetLogfile();
	if (Log) Log->Printf(LOG::DEBUG,4,"ppl6::db::Sybase","Query",__FILE__,__LINE__,"Sybase::Query(const char *query=%s)",(const char *)query);

	rows_affected=0;
	SYBCONNECT *sc=(SYBCONNECT*)conn;
	if (!sc->conn) {
		SetError(181);
		if (Log) Log->LogError(4);
		return NULL;
	}
	syberror="";
	t_start=getmicrotime();
	CS_RETCODE cs_ret;
	if (ct_command(sc->cmd, CS_LANG_CMD, (CS_CHAR*)query.GetPtr(), CS_NULLTERM, CS_UNUSED)!=CS_SUCCEED) {
		SetError(138,"ct_command: %s, syberror: %s",(const char*)query,(const char*)syberror);
		if (Log) Log->LogError(4);
		return NULL;
	}
	cs_ret=ct_send(sc->cmd);
	if (cs_ret!=CS_SUCCEED) {
		ppl6::CString s;
		switch (cs_ret) {
			case CS_FAIL:
				s="CS_FAIL";
				break;
			case CS_CANCELED:
				s="CS_CANCELED";
				break;
			case CS_PENDING:
				s="CS_PENDING";
				break;
			case CS_BUSY:
				s="CS_BUSY";
				break;
			default:
				s="unknown";
				break;
		}
		SetError(138,"ct_send: %s, %s, syberror: %s",(const char*)query,(const char*)s, (const char*)syberror);
		if (Log) Log->LogError(4);
		return NULL;
	}
	UpdateLastUse();
	CS_RETCODE retcode;
	CS_INT restype;
	int status=0;
	SybaseResult *ret=new SybaseResult;
	ret->Query=query;
	ret->Log=Log;
	ret->mem.SetDefaultGrow(resultBufferGrowSize);
	while ((retcode = ct_results(sc->cmd, &restype))==CS_SUCCEED) {
		switch ((int) restype) {
			case CS_CMD_FAIL: {
				CS_INT row_count;
				if (ct_res_info(sc->cmd, CS_ROW_COUNT, &row_count, CS_UNUSED, NULL)==CS_SUCCEED) {
					ret->num_affected = (pplint64)row_count;
					rows_affected=ret->num_affected;
				}
			}
			/* no break */
			default:
                status = PPLSYB_FAILURE;
                break;
            case CS_CMD_SUCCEED:
            case CS_CMD_DONE: {
					CS_INT row_count;
					if (ct_res_info(sc->cmd, CS_ROW_COUNT, &row_count, CS_UNUSED, NULL)==CS_SUCCEED) {
						ret->num_affected = (pplint64)row_count;
						rows_affected=ret->num_affected;
					}
				}
				/* no break */
				/* Fall through */
			case CS_COMPUTEFMT_RESULT:
			case CS_ROWFMT_RESULT:
			case CS_DESCRIBE_RESULT:
			case CS_MSG_RESULT:
				//buffered= 0;                /* These queries have no need for buffering */
				status = PPLSYB_DONE;
				break;
			case CS_COMPUTE_RESULT:
			case CS_CURSOR_RESULT:
			case CS_PARAM_RESULT:
			case CS_ROW_RESULT:
			case CS_STATUS_RESULT:
				ret->cmd=sc->cmd;
				LogQuery(query,(float)(getmicrotime()-t_start));
				UpdateLastUse();
				if (Log) Log->Printf(LOG::DEBUG,4,"ppl6::db::Sybase","Query",__FILE__,__LINE__,"Sybase::Query OK)");
				ret->FetchResultFields();
				return (ppl6::db::Result*) ret;

				/*
				if (!FetchResult(ret,query)) {
					ct_cancel(NULL, sc->cmd, CS_CANCEL_ALL);
				}
				*/
				status = PPLSYB_RESULTS;
				break;
		}

		if (status==PPLSYB_FAILURE) {
			SetError(138,"ct_results: %s, syberror: %s",(const char*)query,(const char*)syberror);
			PushError();
			delete ret;
			ct_cancel(NULL, sc->cmd, CS_CANCEL_ALL);
			PopError();
			if (Log) Log->LogError(4);
			return NULL;
        }
    }
	LogQuery(query,(float)(getmicrotime()-t_start));
	UpdateLastUse();
	if (Log) Log->Printf(LOG::DEBUG,4,"ppl6::db::Sybase","Query",__FILE__,__LINE__,"Sybase::Query OK)");
	return (ppl6::db::Result*) ret;

#endif
}


void Sybase::SetMaxRows(ppluint64 rows)
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
#else
	maxrows=rows;
#endif
}

int Sybase::Ping()
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	CString query="select 1 as result";
	CLog *Log=GetLogfile();
	if (Log) Log->Printf(LOG::DEBUG,4,"ppl6::db::Sybase","Ping",__FILE__,__LINE__,"Prüfe Verbindung");
	SYBCONNECT *sc=(SYBCONNECT*)conn;
	if (!sc->conn) {
		SetError(181);
		if (Log) Log->LogError(4);
		return 0;
	}
	syberror="";
	//double t_start=getmicrotime();
	if (ct_command(sc->cmd, CS_LANG_CMD, (CS_CHAR*)query.GetPtr(), CS_NULLTERM, CS_UNUSED)!=CS_SUCCEED) {
		SetError(138,"ct_command: %s, syberror: %s",(const char*)query,(const char*)syberror);
		if (Log) Log->LogError(4);
		return 0;
	}
	if (ct_send(sc->cmd)!=CS_SUCCEED) {
		SetError(138,"ct_send: %s, syberror: %s",(const char*)query,(const char*)syberror);
		if (Log) Log->LogError(4);
		return 0;
	}
	if (ct_cancel(NULL, sc->cmd, CS_CANCEL_ALL) !=CS_SUCCEED) {
		SetError(138,"ct_cancel: %s, syberror: %s",(const char*)query,(const char*)syberror);
		if (Log) Log->LogError(4);
		return 0;
	}

	if (Log) Log->Printf(LOG::DEBUG,4,"ppl6::db::Sybase","Ping",__FILE__,__LINE__,"Ok");
	return 1;
#endif
}

int Sybase::Escape(CString &str) const
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	// Sybase hat keine Escape-Funktion, daher müssen wir das selbst machen
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

ppluint64 Sybase::GetInsertID()
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	ppl6::db::Result *res=Query("select @@identity as id");
	if (!res) return 0;
	ppluint64 id=(ppluint64) ppl6::atoll(res->Get(0,"id"));
	delete res;
	return id;
#endif
}

pplint64 Sybase::GetAffectedRows()
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	return 0;
#endif
}

int Sybase::StartTransaction()
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	if (transactiondepth==0) {	// Neue Transaktion
		if (Exec("BEGIN TRANSACTION")) {
			transactiondepth++;
			return 1;
		}
	} else {
		if (Execf("SAVE TRANSACTION LEVEL%i",transactiondepth)) {
			transactiondepth++;
			return 1;
		}
	}
	return 0;
#endif
}

int Sybase::EndTransaction()
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	if (transactiondepth==1) {
		if (Exec("COMMIT TRANSACTION")) {
			transactiondepth=0;
			return 1;
		}
	} else {
		transactiondepth--;
		return 1;
	}
	return 0;
#endif
}

int Sybase::CancelTransaction()
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	if (transactiondepth==1) {
		if (Exec("ROLLBACK TRANSACTION")) {
			transactiondepth=0;
			return 1;
		}
	} else {
		if (Execf("ROLLBACK TRANSACTION LEVEL%i",transactiondepth-1)) {
			transactiondepth--;
			return 1;
		}
	}
	return 0;
#endif
}

int Sybase::CancelTransactionComplete()
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	if (Exec("ROLLBACK TRANSACTION")) {
		transactiondepth=0;
		return 1;
	}
	return 0;
#endif
}

int Sybase::CreateDatabase(const char *name)
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	return 0;
#endif
}


CString	Sybase::databaseType() const
{
	return CString("Sybase");
}


/*
 * Statische Member-Funktionen
 */

int	Sybase::GetMaxConnects()
/*!\ingroup PPLGroupDatabases
 * \brief Anzahl maximaler gleichzeitiger Connects auf Sybase-Datenbanken auslesen
 *
 * \descr
 * Anzahl maximaler gleichzeitiger Connects auf Sybase-Datenbanken auslesen
 *
 * \return Liefert die maximale Anzahl gleichzeitig erlaubter Connects zurück.
 *
 */
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	return maxconnects;
#endif
}

int	Sybase::SetMaxConnects(int max)
/*!\ingroup PPLGroupDatabases
 * \brief Anzahl maximaler gleichzeitiger Connects auf Sybase-Datenbanken festlegen
 *
 * \descr
 * Mit dieser Funktion wird die maximale Anzahl gleichzeitiger Connects auf Sybase-Datenbanken
 * festgelegt. Der Default ist unlimitiert.
 *
 * \param[in] max
 *
 * \remarks
 * Die Funktion muss aufgerufen werden, bevor die erste Sybase-Klasse instantiiert wird!
 */
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	if (maxconnects>0) return 0;
	maxconnects=max;
	return 1;
#endif
}

int Sybase::SetVersion(int version)
/*!\brief Zu verwendende Version definieren
 *
 * \ingroup PPLGroupDatabases
 *
 *\desc
 * Legt das grundlegende Verhalten der Sybase CS-Library fest.
 *
 * @param[in] version Kann folgende Werte annehmen:
 * <table border=1 cellspacing=0 cellpadding=2><tr class=table-header><th>Wert</th><th>Verhalten</th><th>Features</th></tr>
 * <tr class=table-row><td>100</td><td>10.0 behavior</td><td>Initial version</td></tr>
 * <tr class=table-row><td>110</td><td>11.1 behavior</td><td>Unicode character set support,
 * Use of external configuration files to control Client-Library property settings.</td></tr>
 * <tr class=table-row><td>120</td><td>12.0 behavior</td><td>All above features</td></tr>
 * <tr class=table-row><td>125</td><td>12.5 behavior</td><td>unichar support, wide data and columns, SSL</td></tr>
 * <tr class=table-row><td>150</td><td>15.0 behavior</td><td>BCP partitions, BCP computed columns,
 * large identifiers, Unilib, ASE default packet size, scrollable cursors,
 * and clusters support. Also, support for unitext, xml, bigint, usmallint,
 * uint, and ubigint datatypes. Note the Sybase library name change.</td></tr>
 * </table>
 * Der Default ist 100
 *
 * @return Bei Erfolg liefert die Funktion 1 zurück, sonst 0.
 *
 * \remarks Die Verfügbarkeit der Versionen hängt von der installierten Library-
 * Version ab. Eine Version 12-Library wird den Wert 150 nicht kennen und
 * FreeTDS bietet gegenwärtig nur Version 10.0 und 11.1 an.
 *
 * Die Funktion muss aufgerufen werden, bevor die erste Sybase-Klasse instantiiert wird!
 */
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
#ifdef CS_VERSION_100
	if (version==100) {
		ctx_version=CS_VERSION_100;
		return 1;
	}
#endif
#ifdef CS_VERSION_110
	if (version==110) {
		ctx_version=CS_VERSION_110;
		return 1;
	}
#endif
#ifdef CS_VERSION_120
	if (version==120) {
		ctx_version=CS_VERSION_120;
		return 1;
	}
#endif
#ifdef CS_VERSION_125
	if (version==125) {
		ctx_version=CS_VERSION_125;
		return 1;
	}
#endif
#ifdef CS_VERSION_150
	if (version==150) {
		ctx_version=CS_VERSION_150;
		return 1;
	}
#endif
	SetError(510,"%i",version);
	return 0;
#endif
}

int Sybase::SetLocale(const char *locale, const char *dateformat)
/*!\brief Legt Ländereinstellungen fest
 *
 * \ingroup PPLGroupDatabases
 * \desc
 * Mit dieser Funktion kann festgelegt werden, welche Ländereinstellungen als
 * Default verwendet werden sollen.
 *
 * @param[in] locale Pointer auf den Namen der Lokalisierung, z.B. "us_english" oder "german". Genaueres
 * ist dem Sybase-Handbuch zu entnehmen.
 * @param[in] dateformat Gibt an, in welcher Reihenfolge bei einem Datum Tag, Monat und Jahr
 * erwartet werden. Mögliche Werte sind: mdy, dmy, ymd, ydm, myd, und dym.
 *
 *
 * @return Bei Erfolg liefert die Funktion 1 zurück, sonst 0.
 *
 * \remarks
 * Die Funktion muss aufgerufen werden, bevor die erste Sybase-Klasse instantiiert wird!
 *
 *
 * \attention
 * Die Funktion wird von FreeTDS nicht unterstützt. Hier empfiehlt es sich eine Default-Einstellung in
 * der freetds.conf vorzunehmen:
 * \code
 * [global]
 * language=german
 * ...
 * \endcode
 */
{
#ifndef HAVE_FREETDS
	SetError(511,"Sybase");
	return 0;
#else
	default_locale.Clear();
	if (locale) default_locale=locale;
	default_dateformat.Clear();
	if (dateformat) default_dateformat=dateformat;
	return 1;
#endif
}


void Sybase::SetResultBufferGrowSize(size_t bytes)
{
#ifdef HAVE_FREETDS
	resultBufferGrowSize=bytes;
#endif
}


}	// EOF namespace db
}	// EOF namespace ppl6
