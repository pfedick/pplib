/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.10 $
 * $Date: 2009/09/28 14:34:41 $
 * $Id: CDBResult.cpp,v 1.10 2009/09/28 14:34:41 patrick Exp $
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
#ifdef _WIN32
#include <winsock.h>
#endif

#include "ppl6.h"

namespace ppl6 {

typedef struct tagFieldnames {
	char *name;
	int type;
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

/*!\class CDBResult
 * \ingroup PPLGroupOldDatabases
 */

CDBResult::CDBResult()
{
	affected_rows=0;
	rows=0;
	fields=0;
	fieldnames=NULL;
	firstrow=NULL;
	lastrow=NULL;
	index=NULL;
	row=NULL;
	rowpointer=0;
	res_lastrow=0;
	res_lastfetch=NULL;
	res_lastfieldfetch=NULL;
	mem=new CMemMan();
	if (mem) {
		mem->SetDefaultGrow(8192);
	}
}

CDBResult::~CDBResult()
{
	PushError();
	Clear();
	if (mem) delete mem;
	PopError();
}

void CDBResult::Clear()
{
	if (mem) mem->Clear();
	if (res_lastfetch) delete res_lastfetch;
	if (res_lastfieldfetch) delete res_lastfieldfetch;
	res_lastrow=0;
	res_lastfetch=NULL;
	fieldnames=NULL;
	firstrow=NULL;
	lastrow=NULL;
	index=NULL;
	affected_rows=0;
	rows=0;
	fields=0;
	row=NULL;
	rowpointer=0;
	res_lastfieldfetch=NULL;
}

void CDBResult::SetAffectedRows(int rows)
{
	affected_rows=rows;
}

int CDBResult::SetNumFields(int num)
{
	if (!mem) {
		SetError(2);
		return 0;
	}
	fields=num;
	fieldnames=mem->Malloc(sizeof(FIELDNAMES)*num);
	if (!fieldnames) {
		fields=0;
		SetError(2);
		return 0;
	}
	FIELDNAMES *f=(FIELDNAMES*)fieldnames;
    for (int i=0;i<fields;i++) {
        f[i].name=NULL;
        f[i].type=DBFIELDTYPE::INVALID;
    }
	return 1;
}

int CDBResult::SetFieldName(int num, const char* name, int type)
{
	if (!mem) {
		SetError(2);
		return 0;
	}
	if (num<fields && fieldnames!=NULL) {
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

int CDBResult::SetFieldName(int num, const char* name, int namelength, int type)
{
	if (!mem) {
		SetError(2);
		return 0;
	}
	if (num<fields && fieldnames!=NULL) {
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

void *CDBResult::Malloc(size_t size)
{
	if (!mem) {
		SetError(2);
		return NULL;
	}
	return mem->Malloc(size);
}

void *CDBResult::Calloc(size_t size)
{
	if (!mem) {
		SetError(2);
		return NULL;
	}
	return mem->Calloc(size);
}

void CDBResult::Free(void *ptr)
{
	if (!mem) {
		SetError(2);
		return;
	}
	mem->Free(ptr);
}

ppluint64 CDBResult::GetUsedMem()
{
	if (!mem) {
		return 0;
	}
	return mem->GetUsedMem();
}

const char *CDBResult::GetFieldName(int num)
{
	if (!mem) {
		SetError(2);
		return NULL;
	}
	if (num>=fields || fieldnames==NULL) {
		SetError(297,"%i",num);
		return NULL;
	}
	FIELDNAMES *f=(FIELDNAMES*)fieldnames;
	return f[num].name;
}

int CDBResult::GetFieldType(int num)
{
	if (!mem) {
		SetError(2);
		return 0;
	}
	if (num>=fields || fieldnames==NULL) {
		SetError(297,"%i",num);
		return 0;
	}
	FIELDNAMES *f=(FIELDNAMES*)fieldnames;
	return f[num].type;
}

int CDBResult::GetFieldNum(const char *name)
{
	if (!mem) {
		SetError(2);
		return 0;
	}
	if (!name) {
		SetError(194,"const char *name");
		return DBFIELDTYPE::INVALID;
	}
	FIELDNAMES *f=(FIELDNAMES*)fieldnames;
	for (int i=0;i<fields;i++) {
		if (f[i].name) {
			if (strcmp(f[i].name,name)==0) return i;
		}
	}
	return -1;
}

int CDBResult::GetFieldType(const char *name)
{
	if (!mem) {
		SetError(2);
		return 0;
	}
	if (!name) {
		SetError(194,"const char *name");
		return DBFIELDTYPE::INVALID;
	}
	FIELDNAMES *f=(FIELDNAMES*)fieldnames;
	for (int i=0;i<fields;i++) {
		if (f[i].name) {
			if (strcmp(f[i].name,name)==0) return f[i].type;
		}
	}
	return -1;
}

int CDBResult::NewRow()
{
	if (!mem) {
		SetError(2);
		return 0;
	}
	int size=sizeof(ROW)+sizeof(FIELD)*fields;
	//printf ("size: %i, fields: %i\n",size,fields);
	row=mem->Malloc(size);
	if (!row) {
		SetError(2);
		return 0;
	}
	ROW *r=(ROW*)row;
	r->fields=(FIELD*)((char*)r+sizeof(ROW));
	//printf("row=%u, fields=%u\n",(ppluint32)r, (ppluint32)r->fields);

	for (int i=0;i<fields;i++) {
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

int CDBResult::BuildIndex()
{
	//printf ("CDBResult::BuildIndex\n");
	if (!mem) {
		SetError(2);
		return 0;
	}
	INDEX *idx=(INDEX*)mem->Malloc(sizeof(INDEX)*rows);
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



int CDBResult::StoreField(int num, void *data, int size)
{
	if (!mem) {
		SetError(2);
		return 0;
	}
	if (num>=fields) {
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




int CDBResult::NumRows()
{
	return rows;
}

int CDBResult::NumFields()
{
	return fields;
}

int CDBResult::NumAffectedRows()
{
	return affected_rows;
}

int CDBResult::SetRow(int row)
{
	if (row<rows) {
		rowpointer=row;
		return 1;
	}
	SetError(222,"Anzahl Rows im Result: %i, versucht zu lesen: Row %i", rows,row);
	return 0;
}

int CDBResult::Reset()
{
	rowpointer=0;
	if (rows) return 1;
	SetError(182);
	return 0;
}


int CDBResult::ToString(CString *res, void *field, int type)
{
	if (!res) return 0;
	if (!field) return 0;
	FIELD *f=(FIELD*) field;
	void *data;
	//printf ("Key von i=%i: %s\n",i,key);
	data=f->data;
	if (!data) return 0;
	switch(type) {
		case DBFIELDTYPE::STRING:
			//printf ("Key: %s, Data: %u, Size: %i\n",key,(ppluint32)data,field->size);
			res->Set((const char*)data,f->size);
			return 1;
		case DBFIELDTYPE::INT:
			res->Setf("%i",((int*)data)[0]);
			return 1;
		case DBFIELDTYPE::FLOAT:
			res->Setf("%i",((double*)data)[0]);
			// Nullen am Ende entfernen
			res->RTrim("0");
			// Punkt am Ende entfernen
			res->RTrim(".");
			return 1;
		case DBFIELDTYPE::BINARY:
			break;
		case DBFIELDTYPE::BIT:
			res->Setf("%i",((int*)data)[0]);
			return 1;
	}
	return 0;

}


CAssocArray *CDBResult::FetchArray(int row)
{
	if (!SetRow(row)) return NULL;
	return FetchArray();
}

CAssocArray *CDBResult::FetchArray()
{
	CAssocArray *a=new CAssocArray();
	if (!FetchArray(a)) {
		delete a;
		return NULL;
	}
	return a;
}

int CDBResult::FetchArray(CAssocArray *a, int row)
{
	if (!SetRow(row)) return 0;
	return FetchArray(a);
}

int CDBResult::FetchArray(CAssocArray *a)
{
	if (!a) {
		SetError(194,"CAssocArray *a");
		return 0;
	}
	if (rowpointer>=rows) {
		SetError(300,"Anzahl Rows im Result: %i, versucht zu lesen: Row %i", rows,rowpointer);
		return 0;
	}
	CString s;
	FIELDNAMES *f=(FIELDNAMES*)fieldnames;
	char *key;
	FIELD *field;
	INDEX *idx=(INDEX*)index;
	//printf ("Index: %u\n",(ppluint32)index);
	idx+=rowpointer;
	rowpointer++;
	//printf ("Row: %u\n",(ppluint32)idx);
	for (int i=0;i<fields;i++) {
		key=f[i].name;
		field=&idx->row->fields[i];
		if (!ToString(&s,field,f[i].type)) {
			a->Set(key,(char*)"");
		} else {
			a->Set(key,s);
		}
		//printf ("Key: %s\n",key);
	}
	return 1;
}

CArray *CDBResult::FetchFields()
{
	CArray *a=new CArray();
	if (!FetchFields(a)) {
		delete a;
		return NULL;
	}
	return a;
}

CArray *CDBResult::FetchFields(int row)
{
	if (!SetRow(row)) return NULL;
	return FetchFields();
}

int CDBResult::FetchFields(CArray *result)
{
	if (!result) {
		SetError(194,"int CDBResult::FetchFields(CArray *result)");
		return 0;
	}
	if (rowpointer>=rows) {
		SetError(300,"Anzahl Rows im Result: %i, versucht zu lesen: Row %i", rows,rowpointer);
		return 0;
	}
	CString s;
	FIELDNAMES *f=(FIELDNAMES*)fieldnames;
	FIELD *field;
	INDEX *idx=(INDEX*)index;
	idx+=rowpointer;
	rowpointer++;
	for (int i=0;i<fields;i++) {
		field=&idx->row->fields[i];
		if (!ToString(&s,field,f[i].type)) {
			result->Add((const char*)"");
		} else {
			result->Add(s);
		}
	}
	return 1;

}


int CDBResult::FetchFields(CArray *result, int row)
{
	if (!SetRow(row)) return 0;
	return FetchFields(result);
}



const char *CDBResult::Result(int row, const char *field)
{
	if (res_lastfieldfetch) {
		delete res_lastfieldfetch;
		res_lastfieldfetch=NULL;
	}

	if (row!=res_lastrow || res_lastfetch==NULL) {
		if (res_lastfetch) delete res_lastfetch;
		res_lastfetch=FetchArray(row);
		res_lastrow=row;
	}
	if (!res_lastfetch) return NULL;
	return res_lastfetch->GetChar(field);
}

const char *CDBResult::Result(int row, int field)
{
	if (res_lastfetch) {
		delete res_lastfetch;
		res_lastfetch=NULL;
	}

	if (row!=res_lastrow || res_lastfieldfetch==NULL) {
		if (res_lastfieldfetch) delete res_lastfieldfetch;
		res_lastfieldfetch=FetchFields(row);
		res_lastrow=row;
	}
	if (!res_lastfieldfetch) return NULL;
	return (const char*)res_lastfieldfetch->Get(field);
}



} // EOF namespace ppl6


