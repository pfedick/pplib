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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
	#include <strings.h>
#endif

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN		// Keine MFCs
#endif


#include "ppl6.h"

namespace ppl6 {

typedef struct tagMemSpace {
	void *adr;
	ppldd size;
	struct tagMemSpace *last, *next;
} MEMSPACE;


/*!\class CMemSpace
 * \ingroup PPLGroupMemory
 * \brief Speicherverwaltung: MemSpace
 *
 * CMemSpace ist ein Wrapper für Malloc & Co.. Die Klasse
 * tut nichts anderes als die entsprechenden libc-Funktionen
 * aufzurufen, merkt sich allerdings den reservierten Speicher.
 * Durch Löschen der Klasse oder Aufruf von Clear werden alle
 * Speicherblöcke wieder freigegeben
 */


CMemSpace::CMemSpace()
{
	first=last=NULL;
}

CMemSpace::~CMemSpace()
{
	Clear();
}

void CMemSpace::Clear()
{
	MEMSPACE *d,*m=(MEMSPACE*)first;
	while (m) {
		free(m->adr);
		d=m;
		m=m->next;
		free(d);
	}
	first=last=NULL;
}

void *CMemSpace::Malloc(ppldd size, bool clear)
{
	if (!size) return NULL;
	MEMSPACE *m=(MEMSPACE*)malloc(sizeof(MEMSPACE));
	if (!m) return NULL;

	if (clear) {
		m->adr=(void *)calloc(1,size);
	} else {
		m->adr=(void *) malloc(size);
	}
	if (!m->adr) {
		free (m);
		return NULL;
	}
	m->size=size;
	m->last=(MEMSPACE*)last;
	m->next=NULL;
	if (!first) {
		first=last=m;
	} else {
		((MEMSPACE*)last)->next=m;
		last=m;
	}

	blocks++;
	this->size+=size;
	return m->adr;
}

void *CMemSpace::Calloc(ppldd size)
{
	return Malloc(size,true);
}

void CMemSpace::Free(void *adr)
{
	MEMSPACE *m=(MEMSPACE*)first;
	// Den Adressblock finden
	while (m) {
		if (m->adr==adr) {
			free(m->adr);
			size-=m->size;
			blocks--;
			if (m->last) m->last->next=m->next;
			if (m->next) m->next->last=m->last;
			free(m);
			return;
		}
		m=m->next;
	}
}

char *CMemSpace::StrDup(const char *string)
{
	if (!string) {
		SetError(194);
		return NULL;
	}
	ppldd size=(ppldd)strlen(string)+1;
	char *res=(char*)Malloc(size,false);
	if (!res) return NULL;
	strncpy(res,string,size);
	res[size-1]=0;
	return res;
}

ppldd CMemSpace::GetNumBlocks()
{
	return blocks;
}

ppldd CMemSpace::GetUsedMem()
{
	return size;
}


}	// EOF Namespace ppl6

