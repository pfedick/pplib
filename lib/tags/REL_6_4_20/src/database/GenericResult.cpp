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


namespace ppl6 {
namespace db {

typedef struct tagFieldnames {
	char *name;
	Result::Type type;
} FIELDNAMES;

typedef struct tagField {
	void	*data;
	int		size;
} FIELD;

typedef struct tagRows {
	struct tagRows *next;
	FIELD *fields;
} ROW;

typedef struct tagIndex {
	ROW *row;
} INDEX;


/*!\class GenericResult
 * \ingroup PPLGroupDatabases
 * \brief Klasse zum Speichern von Ergebnissen aus Datenbank-Selects
 *
 * \header \#include <ppl6-db.h>
 *
 * \descr
 * Dies ist eine von ppl6::db::Result abgeleitete Klasse zum Speichern von Ergebnissen aus
 * Datenbank-Selects. Sie wird verwendet, wenn die Datenbank selbst keine Möglichkeit bietet,
 * ein Ergebnis beliebig zu durchwandern, oder wenn diese Funktionalität noch nicht implementiert
 * wurde. Ebenfalls kann sie verwendet werden, um eine Kopie eines Results zu erstellen.
 * \par
 * Mit Ihren Funktionen können die Ergebniszeilen aus einem Select durchwandert und ausgelesen werden.
 * \par Verwendung:
 * Um ein Result in der Klasse zu speichern, muss zunächst eine Instanz von GenericResultmit \b new
 * erstellt werden. Mit GenericResult::SetNumFields wird der Klasse dann mitgeteilt, aus
 * wievielen Feldern das Ergebnis besteht. Danach wird für jedes Feld die Funktion
 * GenericResult::SetFieldName aufgerufen, die als Parameter die Nummer des Feldes (beginnend
 * mit 0), den Namen und den Datentyp vom Typ Result::Type bekommt.
 * \par
 * Nun kann begonnen werden die einzelnen Zeilen des Ergebnisses zu speichern.
 * Dazu wird zu Beginn jeder neuen Zeile die Funktion GenericResult::NewRow
 * aufgerufen. Danach werden die einzelnen Felder der Zeile mit
 * GenericResult::StoreField gespeichert. Die Daten werden dabei in einen
 * eigenen Speicherbereich der Klasse kopiert.
 * \par
 * Sind alle Ergebniszeilen gespeichert, wird zum Schluss noch die Funktion
 * GenericResult::BuildIndex aufgerufen, wodurch ein Index zum schnellen Zugriff
 * auf bestimmte Zeilen erstellt wird. Ausserdem muss noch die Funktion
 * GenericResult::SetAffectedRows aufgerufen werden, um die Anzahl durch den
 * Query veränderter Datensätze zu übergeben.
 * \par
 * Falls während des Vorgangs Speicher benötigt wird, der bis zum Löschen des
 * Result-Objects erhalten bleiben muss, können die Funktionen GenericResult::Malloc,
 * GenericResult::Calloc und GenericResult::Free verwendet werden. Diese allokieren
 * internen Speicher der Klasse, der durch Aufruf des Konstruktors wieder freigegeben
 * wird (verwendet GenericResult::mem).
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_GenericResult_Example1
 * \until EOF
 *
 */



GenericResult::GenericResult()
{
	mem=new CMemMan();
	if (mem) {
		mem->SetDefaultGrow(65536);
	}
	res_lastrow=0;
	fieldnames=NULL;
	firstrow=NULL;
	lastrow=NULL;
	index=NULL;
	row=NULL;
	rowpointer=0;
	rows=0;
	lastinsertid=0;
	affectedrows=0;
	num_fields=0;
	res_lastfieldfetch=NULL;
	res_lastfetch=NULL;

}

GenericResult::~GenericResult()
{
	PushError();
	Clear();
	if (mem) delete mem;
	PopError();
}

int GenericResult::Import(CAssocArray &array)
{
	return 0;
}

void GenericResult::Clear()
{
	if (mem) mem->Clear();
	if (res_lastfetch) delete res_lastfetch;
	if (res_lastfieldfetch) delete res_lastfieldfetch;
	res_lastfieldfetch=NULL;
	res_lastfetch=NULL;
	res_lastrow=0;
	fieldnames=NULL;
	firstrow=NULL;
	lastrow=NULL;
	index=NULL;
	row=NULL;
	rowpointer=0;
	rows=0;
	lastinsertid=0;
	affectedrows=0;
	num_fields=0;
}

void GenericResult::SetBufferGrowSize(size_t bytes)
{
	if (mem) mem->SetDefaultGrow(bytes);
}

void *GenericResult::Malloc(size_t size)
{
	if (!mem) {
		SetError(2);
		return NULL;
	}
	return mem->Malloc(size);
}

void *GenericResult::Calloc(size_t size)
{
	if (!mem) {
		SetError(2);
		return NULL;
	}
	return mem->Calloc(size);
}

void GenericResult::Free(void *ptr)
{
	if (!mem) {
		SetError(2);
		return;
	}
	mem->Free(ptr);
}

ppluint64 GenericResult::GetUsedMem()
{
	if (!mem) {
		return 0;
	}
	return mem->GetUsedMem();
}

void GenericResult::SetAffectedRows(pplint64 rows)
{
	affectedrows=rows;
}

int GenericResult::SetNumFields(int num)
{
	if (!mem) {
		SetError(2);
		return 0;
	}
	num_fields=num;
	fieldnames=mem->Malloc(sizeof(FIELDNAMES)*num);
	if (!fieldnames) {
		num_fields=0;
		SetError(2);
		return 0;
	}
	FIELDNAMES *f=(FIELDNAMES*)fieldnames;
    for (int i=0;i<num_fields;i++) {
        f[i].name=NULL;
        f[i].type=Result::Unknown;
    }
	return 1;
}

int GenericResult::SetFieldName(int num, const char* name, Result::Type type)
{
	if (!mem) {
		SetError(2);
		return 0;
	}
	if (num<num_fields && fieldnames!=NULL) {
		FIELDNAMES *f=(FIELDNAMES*)fieldnames;
		f[num].name=mem->Strdup(name);
		if (!f[num].name) {
			SetError(2);
			return 0;
		}
		f[num].type=type;
		//printf ("Field %i: %s\n",num,name);
		return 1;
	}
	SetError(297,"%i",num);
	return 0;
}

int GenericResult::SetFieldName(int num, const char* name, int namelength, Result::Type type)
{
	if (!mem) {
		SetError(2);
		return 0;
	}
	if (num<num_fields && fieldnames!=NULL) {
		FIELDNAMES *f=(FIELDNAMES*)fieldnames;
		f[num].name=mem->Strndup(name,namelength);
		if (!f[num].name) {
			SetError(2);
			return 0;
		}
		f[num].type=type;
		//printf ("Field %i: %s\n",num,name);
		return 1;
	}
	SetError(297,"%i",num);
	return 0;
}

int GenericResult::NewRow()
{
	if (!mem) {
		SetError(2);
		return 0;
	}
	int size=sizeof(ROW)+sizeof(FIELD)*num_fields;
	//printf ("size: %i, fields: %i\n",size,fields);
	row=mem->Malloc(size);
	if (!row) {
		SetError(2);
		return 0;
	}
	ROW *r=(ROW*)row;
	r->fields=(FIELD*)((char*)r+sizeof(ROW));
	//printf("row=%u, fields=%u\n",(ppluint32)r, (ppluint32)r->fields);

	for (int i=0;i<num_fields;i++) {
		//printf ("debug %i\n",i);
		r->fields[i].data=NULL;
		r->fields[i].size=0;
	}
	r->next=NULL;
	if (!lastrow) {
		firstrow=row;
	} else {
		((ROW*)lastrow)->next=r;
	}
	lastrow=row;
	rows++;
	return 1;
}

int GenericResult::BuildIndex()
{
	//printf ("CDBResult::BuildIndex\n");
	if (!mem) {
		SetError(2);
		return 0;
	}
	INDEX *idx=(INDEX*)mem->Malloc((size_t)(sizeof(INDEX)*rows));
	//printf ("index: %u, rows: %u, size: %u\n",(ppluint32)idx,rows, sizeof(INDEX)*rows);
	index=idx;
	if (!idx) {
		SetError(2);
		return 0;
	}
	ROW *rr=(ROW*)firstrow;
	int i=0;
	while (rr) {
		idx[i].row=rr;
		rr=rr->next;
		i++;
	}
	return 1;
}

int GenericResult::StoreField(int num, void *data, int size)
{
	if (!mem) {
		SetError(2);
		return 0;
	}
	if (num>=num_fields) {
		SetError(297,"%i",num);
		return 0;
	}
	if (!row) {
		SetError(298,"StoreField");
		return 0;
	}
	ROW *r=(ROW*)row;
	char *ptr;
	if (data!=NULL && size>0) {
		ptr=(char*)mem->Malloc(size+1);
		if (ptr==NULL) {
			SetError(2);
			return 0;
		}
		ptr[size]=0;
		r->fields[num].data=ptr;
		r->fields[num].size=size;
		memcpy(ptr,data,size);
	} else {
		r->fields[num].data=NULL;
		r->fields[num].size=0;
	}
	return 1;
}

int GenericResult::ToString(CString &res, void *field, Result::Type type)
{
	if (!field) return 0;
	FIELD *f=(FIELD*) field;
	void *data;
	//printf ("Key von i=%i: %s\n",i,key);
	data=f->data;
	if (!data) return 0;
	switch(type) {
		case Result::String:
			//printf ("Key: %s, Data: %u, Size: %i\n",key,(ppluint32)data,field->size);
			res.Set((const char*)data,f->size);
			return 1;
		case Result::Integer:
			res.Setf("%i",((int*)data)[0]);
			return 1;
		case Result::Decimal:
			res.Setf("%i",((double*)data)[0]);
			// Nullen am Ende entfernen
			res.RTrim("0");
			// Punkt am Ende entfernen
			res.RTrim(".");
			return 1;
		case Result::Binary:
			break;
		case Result::Bit:
			res.Setf("%i",((int*)data)[0]);
			return 1;
		default:
			break;
	}
	return 0;

}

pplint64 GenericResult::Rows()
{
	return rows;
}

pplint64 GenericResult::Affected()
{
	return affectedrows;
}

int GenericResult::Fields()
{
	return num_fields;
}


int GenericResult::FieldNum(const char *fieldname)
{
	if (!mem) {
		SetError(2);
		return 0;
	}
	if (!fieldname) {
		SetError(194,"const char *fieldname");
		return -1;
	}
	FIELDNAMES *f=(FIELDNAMES*)fieldnames;
	for (int i=0;i<num_fields;i++) {
		if (f[i].name) {
			if (strcmp(f[i].name,fieldname)==0) return i;
		}
	}
	SetError(183,"%s",fieldname);
	return -1;
}

const char *GenericResult::FieldName(int num)
{
	if (!mem) {
		SetError(2);
		return NULL;
	}
	if (num>=num_fields || fieldnames==NULL) {
		SetError(297,"%i",num);
		return NULL;
	}
	FIELDNAMES *f=(FIELDNAMES*)fieldnames;
	return f[num].name;
}

Result::Type GenericResult::FieldType(int num)
{
	if (!mem) {
		SetError(2);
		return Result::Error;
	}
	if (num>=num_fields || fieldnames==NULL) {
		SetError(297,"%i",num);
		return Result::Error;
	}
	FIELDNAMES *f=(FIELDNAMES*)fieldnames;
	return f[num].type;

}

Result::Type GenericResult::FieldType(const char *fieldname)
{
	if (!mem) {
		SetError(2);
		return Result::Error;
	}
	if (!fieldname) {
		SetError(194,"const char *name");
		return Result::Error;
	}
	FIELDNAMES *f=(FIELDNAMES*)fieldnames;
	for (int i=0;i<num_fields;i++) {
		if (f[i].name) {
			if (strcmp(f[i].name,fieldname)==0) return f[i].type;
		}
	}
	return Result::Error;
}

const char *GenericResult::Get(pplint64 row, const char *fieldname)
{
	if (res_lastfieldfetch) {
		delete res_lastfieldfetch;
		res_lastfieldfetch=NULL;
	}

	if (row>=rows) {
		SetError(222,"Anzahl Rows im Result: %i, versucht zu lesen: Row %i", rows,row);
		return NULL;
	}

	if (row!=res_lastrow || res_lastfetch==NULL) {
		if (res_lastfetch) {
			res_lastfetch->Clear();
		} else {
			res_lastfetch=new CAssocArray;
			if (!res_lastfetch) {
				SetError(2);
				return NULL;
			}
		}
		if (!FetchArray(*res_lastfetch,row)) {
			res_lastrow=-1;
			return NULL;
		}
		res_lastrow=row;
	}
	const char *ret=res_lastfetch->Get(fieldname);
	if (ret) return ret;
	SetError(183,"Feldname: %s",fieldname);
	return NULL;
}

const char *GenericResult::Get(pplint64 row, int field)
{
	if (res_lastfetch) {
		delete res_lastfetch;
		res_lastfetch=NULL;
	}

	if (row>=rows) {
		SetError(222,"Anzahl Rows im Result: %i, versucht zu lesen: Row %i", rows,row);
		return NULL;
	}
	if (field>num_fields) {
		SetError(183,"Anzahl Felder im Result: %i, versucht zu lesen: %i", num_fields,field);
		return NULL;
	}


	if (row!=res_lastrow || res_lastfieldfetch==NULL) {
		if (res_lastfieldfetch) {
			res_lastfieldfetch->Clear();
		} else {
			res_lastfieldfetch=new CArray;
			if (!res_lastfieldfetch) {
				SetError(2);
				return NULL;
			}
		}
		if (!FetchFields(*res_lastfieldfetch,row)) {
			res_lastrow=-1;
			return NULL;
		}
		res_lastrow=row;
	}
	return res_lastfieldfetch->Get(field);
}


CAssocArray GenericResult::FetchArray(pplint64 row)
{
	CAssocArray a;
	FetchArray(a,row);
	return a;
}

int GenericResult::FetchArray(CAssocArray &array, pplint64 row)
{
	if (row>=0) {
		if (row>=rows) {
			SetError(222,"Anzahl Rows im Result: %i, versucht zu lesen: Row %i", rows,row);
			return 0;
		}
		rowpointer=row;
	}
	if (rowpointer>=rows) {
		SetError(300,"Anzahl Rows im Result: %i, versucht zu lesen: Row %i", rows,rowpointer);
		return 0;
	}
	CString s;
	FIELDNAMES *f=(FIELDNAMES*)fieldnames;
	const char *key;
	FIELD *field;
	INDEX *idx=(INDEX*)index;
	//printf ("Index: %u\n",(ppluint32)index);
	idx+=rowpointer;
	rowpointer++;
	array.Clear();
	//printf ("Row: %u\n",(ppluint32)idx);
	for (int i=0;i<num_fields;i++) {
		key=f[i].name;
		field=&idx->row->fields[i];
		if (!ToString(s,field,f[i].type)) {
			array.Set(key,"");
		} else {
			array.Set(key,s);
		}
	}
	return 1;
}

CArray GenericResult::FetchFields(pplint64 row)
{
	CArray a;
	FetchFields(a,row);
	return a;
}

int GenericResult::FetchFields(CArray &array, pplint64 row)
{
	if (row>=0) {
		if (row>=rows) {
			SetError(222,"Anzahl Rows im Result: %i, versucht zu lesen: Row %i", rows,row);
			return 0;
		}
		rowpointer=row;
	}
	if (rowpointer>=rows) {
		SetError(300,"Anzahl Rows im Result: %i, versucht zu lesen: Row %i", rows,rowpointer);
		return 0;
	}
	CString s;
	FIELDNAMES *f=(FIELDNAMES*)fieldnames;
	//const char *key;
	FIELD *field;
	INDEX *idx=(INDEX*)index;
	//printf ("Index: %u\n",(ppluint32)index);
	idx+=rowpointer;
	rowpointer++;
	array.Clear();
	//printf ("Row: %u\n",(ppluint32)idx);
	for (int i=0;i<num_fields;i++) {
		//key=f[i].name;
		field=&idx->row->fields[i];
		if (!ToString(s,field,f[i].type)) {
			array.Add("");
		} else {
			array.Add(s);
		}
	}
	return 1;
}

int GenericResult::Seek(pplint64 row)
{
	if (row<rows) {
		rowpointer=row;
		return 1;
	}
	SetError(222,"Anzahl Rows im Result: %i, versucht zu lesen: Row %i", rows,row);
	return 0;
}

void GenericResult::PrintResult()
{

}


}	// EOF namespace db
}	// EOF namespace ppl6
