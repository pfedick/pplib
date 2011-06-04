/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
 *
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
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
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#include "ppl7.h"


namespace ppl7 {

/*!\class MemoryGroup
 * \ingroup PPLGroupMemory
 * \brief Speicherverwaltung: MemSpace
 *
 * \desc
 * MemoryGroup ist ein Wrapper für Malloc & Co. der libc-Bibliothek und dient zur Gruppierung
 * von Speicher. Die Klasse merkt sich jeden über sie allokierten Speicherbereich in einer
 * Liste und kann dadurch sämtlichen Speicher auf einmal über den Destruktor oder durch
 * Aufruf von MemoryGroup::clear wieder löschen.
 * \par
 * Durch Aufruf von MemoryGroup::malloc, MemoryGroup:calloc oder MemoryGroup::strdup wird
 * Speicher allokiert. Mit MemoryGroup:realloc kann ein existierender Speicherblock vergrößert
 * oder verkleinert werden. Mit MemoryGroup::free wird ein Speicherblock wieder freigegeben.
 * Mit MemoryGroup::clear werden alle über diese Klasse allokierten Speicherbereiche auf einmal
 * freigegeben.
 * \attention
 * Speicher, der durch diese Klasse allokiert wurde, darf nicht mit der %free-Funktion der
 * libc freigegeben werden und umgekehrt.
 * \note
 * Die Klasse entspricht funktional der Klasse CMemSpace aus Version 6 der PPLib.
 */

typedef struct tagMemGroup {
	void *adr;
	size_t size;
	struct tagMemGroup *previous, *next;
} MEMGROUP;

static size_t mysize=(sizeof(MEMGROUP)+15)^15;


MemoryGroup::MemoryGroup()
{
	first=last=NULL;
	totalSize=0;
	totalBlocks=0;
}

MemoryGroup::~MemoryGroup()
{
	clear();
}

void MemoryGroup::clear()
{
	MEMGROUP *m;
	while (first) {
		m=(MEMGROUP *)first;
		first=m->next;
		free(m);
	}
	first=last=NULL;
	totalSize=0;
	totalBlocks=0;
}

void MemoryGroup::addToList(void *block)
{
	MEMGROUP* m=(MEMGROUP*)block;
	m->previous=(MEMGROUP*)last;
	m->next=NULL;
	if (last) ((MEMGROUP*)last)->next=m;
	last=m;
	if (!first) first=m;
	totalSize+=m->size+mysize;
	totalBlocks++;
}

void MemoryGroup::removeFromList(void *block)
{
	MEMGROUP* m=(MEMGROUP*)block;
	if (m->previous) m->previous->next=m->next;
	if (m->next) m->next->previous=m->previous;
	if (m==first) first=m->next;
	if (m==last) last=m->previous;
	totalSize-=(m->size+mysize);
	totalBlocks--;
}


void *MemoryGroup::malloc(size_t size)
{
	if (!size) throw IllegalArgumentException();
	MEMGROUP *m=(MEMGROUP*)::malloc(mysize+size);
	if (!m) throw OutOfMemoryException();
	m->adr=(char*)m+mysize;
	m->size=size;
	addToList(m);
	return m->adr;
}

void *MemoryGroup::calloc(size_t size)
{
	if (!size) throw IllegalArgumentException();
	MEMGROUP *m=(MEMGROUP*)::calloc(1,mysize+size);
	if (!m) throw OutOfMemoryException();
	m->adr=(char*)m+mysize;
	m->size=size;
	addToList(m);
	return m->adr;
}

char *MemoryGroup::strdup(const char *str)
{
	if (!str) throw NullPointerException();
	size_t size=strlen(str)+1;
	MEMGROUP *m=(MEMGROUP*)::malloc(mysize+size);
	if (!m) throw OutOfMemoryException();
	m->adr=(char*)m+mysize;
	memcpy(m->adr,str,size);
	((char*)m->adr)[size]=0;
	m->size=size;
	addToList(m);
	return (char*)m->adr;
}


void *MemoryGroup::realloc(void *adr, size_t size)
{
	MEMGROUP *m=(MEMGROUP*)((char*)adr-mysize);
	// Sicherstellen, dass der Speicherblock von dieser Klasse verwaltet wird
	if (m->adr!=adr)  throw IllegalMemoryAddressException();
	MEMGROUP *old=m;
	removeFromList(old);
	m=(MEMGROUP*)::realloc(m,mysize+size);
	if (!m) {
		addToList(old);
		throw OutOfMemoryException();
	}
	m->size=size;
	m->adr=(char*)m+mysize;
	addToList(m);
	return m->adr;
}

void MemoryGroup::free(void *adr)
{
	MEMGROUP *m=(MEMGROUP*)((char*)adr-mysize);
	// Sicherstellen, dass der Speicherblock von dieser Klasse verwaltet wird
	if (m->adr!=adr)  throw IllegalMemoryAddressException();
	removeFromList(m);
	free(m);
}

size_t MemoryGroup::count() const
{
	return totalBlocks;
}

size_t MemoryGroup::size() const
{
	return totalSize;
}


}	// EOF namespace ppl7
