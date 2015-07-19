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
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#include "ppl6.h"




namespace ppl6 {

typedef struct tagMemSlot {
	ppluint32	sig;
	ppluint32	crc;
	ppluint8 *adr;
	ppluint32 size;
	struct tagMemSlot *previous, *next;
	struct tagMemMan *block;
} MEMSLOT;

typedef struct tagMemMan {
	ppluint8 *adr;
	ppluint32 size;
	ppluint32 bytesfree, biggest_block;
	MEMSLOT *biggest;
	MEMSLOT *free, *allocated;
	struct tagMemMan *previous, *next;
} MEMMAN;


/*!\class CMemMan
 * \ingroup PPLGroupMemory
 * \brief Speicherverwaltung: MemMan
 *
 * CMemMan geht einen Schritt weiter. Sie allokiert von vorne
 * herein mehr Speicher und verwaltet diesen selbst innerhalb einer
 * Liste. Die Klasse ist ideal dafür geeignet, wenn viele kleine
 * Speicherblöcke mit unterschiedlichen Größen allokiert werden
 * müssen, diese aber selten wieder freigegeben werden.
 */


CMemMan::CMemMan()
{
	first=last=NULL;
	growth=64*1024;		// 64 KByte
	size=0;
	size_used=0;
	size_free=0;
	// Auf 16 Byte runden
	sizeof_memslot=(sizeof(MEMSLOT)+15)&(0xffffffff-15);
	sizeof_memman=(sizeof(MEMMAN)+15)&(0xffffffff-15);
	sig=ppl6::rand(0,0xffffffff);
}

CMemMan::~CMemMan()
{
	Clear();
}

void CMemMan::Clear()
{
	MEMMAN *d,*m=(MEMMAN*)first;
	while (m) {
		d=m;
		m=m->next;
		free(d);
	}
	first=last=NULL;
	size=0;
	size_used=0;
	size_free=0;
}

void CMemMan::SetDefaultGrow(ppluint32 bytes)
{
	growth=bytes;
}

void *CMemMan::Malloc(ppldd size, bool clear)
{
	if (!size) return NULL;
	// size wird auf 16 Byte gerundet
	size=(size+15)&(0xffffffff-15);
	// Gibt es einen Speicherblock, der gro� genug ist?
	void *ret=NULL;
	MEMSLOT *slot;
	MEMMAN *block=(MEMMAN*)last;
	while (block) {
		if (block->biggest_block>=size) {
			// Wir haben einen, jetzt m�ssen wir die passende
			// Stelle finden
			slot=block->free;
			while (slot) {
				if (slot->size>=size) {
					// Passende Stelle gefunden
					ret=AllocateSlot(block,slot,size,clear);
					return ret;
				}
				slot=slot->next;
			}
		}
		block=block->previous;
	}
	// Nix freies gefunden, wir allokieren neuen Speicher
	ppluint32 bytes=growth;
	if (bytes<=size+sizeof_memslot) bytes=size+sizeof_memslot+sizeof_memman;
	ppluint8 *adr=(ppluint8*)malloc(bytes);
	if (!adr) return NULL;
	this->size+=bytes;
	size_used+=sizeof_memman+sizeof_memslot;
	size_free=this->size-size_used;
	block=(MEMMAN*)adr;
	// Block in die Kette h�ngen
	block->next=NULL;
	block->previous=NULL;
	if (!last) {
		first=last=block;
	} else {
		((MEMMAN*)last)->next=block;
		block->previous=(MEMMAN*)last;
		last=block;
	}
	// Block und Slot intialisieren
	block->adr=adr+sizeof_memman;
	block->allocated=NULL;
	slot=(MEMSLOT*)block->adr;
	block->free=slot;
	block->allocated=NULL;
	block->bytesfree=bytes-sizeof_memman-sizeof_memslot;
	block->biggest_block=block->bytesfree;
	block->biggest=slot;
	slot->adr=((ppluint8*)slot)+sizeof_memslot;
	slot->next=NULL;
	slot->previous=NULL;
	slot->size=block->bytesfree;
	slot->sig=sig;
	slot->block=block;
	slot->crc=CRC(slot);
	ret=AllocateSlot(block,slot,size,clear);
	return ret;
}

void *CMemMan::AllocateSlot(void *b, void *s, ppldd size, bool clear)
{
	MEMMAN *block=(MEMMAN*)b;
	MEMSLOT *slot=(MEMSLOT*)s;
	MEMSLOT *slot1=NULL;
	MEMSLOT *slot2=NULL;
	// wenn slot->size > size ist, machen wir zwei Bl�cke
	// daraus, sofern f�r den zweiten noch genug Platz ist.
	if (slot->size>(size+sizeof_memslot+32)) {
		slot1=slot;
		slot2=(MEMSLOT*)(((ppluint8*)slot)+size+sizeof_memslot);
		slot2->adr=((ppluint8*)slot2)+sizeof_memslot;
		slot2->size=slot1->size-sizeof_memslot-size;
		slot1->size=size;
		// Neuen Slot in die Kette h�ngen
		slot2->next=slot1->next;
		slot2->previous=slot1;
		slot1->next=slot2;
		slot1->sig=sig;
		slot2->sig=sig;
		slot1->block=block;
		slot2->block=block;
		if (block->biggest==slot) {
			if (slot1->size>slot2->size) {
				block->biggest=slot1;
				block->biggest_block=slot1->size;
			} else {
				block->biggest=slot2;
				block->biggest_block=slot2->size;
			}
		}
		size_used+=sizeof_memslot;
		size_free-=sizeof_memslot;
		block->bytesfree-=sizeof_memslot;
		slot1->crc=CRC(slot1);
		slot2->crc=CRC(slot2);
		slot=slot1;
	} else {
		// Wenn nicht genug Platz f�r einen zweiten Slot ist,
		// allokieren wir den gesamten verbliebenen Speicher
		size=slot->size;
	}
	// Jetzt nehmen wir den slot aus der free-Kette raus
	if (slot->previous) slot->previous->next=slot->next;
	if (slot->next) slot->next->previous=slot->previous;
	if (block->free==slot) block->free=slot->next;
	// und h�ngen ihn in die allocated-Kette
	slot->previous=NULL;
	if (block->allocated) block->allocated->previous=slot;
	slot->next=block->allocated;
	block->allocated=slot;
	slot->crc=CRC(slot);

	// Speichercounter korrigieren
	block->bytesfree-=size;
	size_used+=size;
	size_free-=size;

	// Den gr��ten verbleibenden freien Slot finden
	if (block->biggest==slot) {
		FindBiggestSlot(block);
	}
	if (clear) bzero (slot->adr,slot->size);
	return slot->adr;
}

void CMemMan::FindBiggestSlot(void *b)
{
	MEMMAN *block=(MEMMAN*)b;
	MEMSLOT *slot;

	block->biggest=NULL;
	block->biggest_block=0;
	slot=block->free;
	while(slot) {
		if (slot->size>block->biggest_block) {
			block->biggest=slot;
			block->biggest_block=slot->size;
		}
		slot=slot->next;
	}
}

ppluint32 CMemMan::CRC(void *s)
{
	ppluint32 c=0;
	MEMSLOT *slot=(MEMSLOT*)s;
	c+=(ppluint32)(slot->sig+slot->size+((ppliptr)(slot->adr)&0xffffffff));
	return c;
}

void *CMemMan::Calloc(ppldd size)
{
	return Malloc(size,true);
}

void *CMemMan::Calloc(size_t nmemb, size_t size)
{
	return Malloc(nmemb*size,true);
}

char *CMemMan::Strdup(const char *str)
{
	if (!str) return NULL;
	size_t s=strlen(str)+1;
	char *m=(char*)Malloc((ppluint32)s);
	if (!m) return NULL;
	memcpy(m,str,s);
	return m;
}

char *CMemMan::Strndup(const char *str, size_t size)
{
	if (!str) return NULL;
	char *m=(char*)Malloc((ppluint32)size+1);
	if (!m) return NULL;
	memcpy(m,str,size);
	m[size]=0;
	return m;
}



void CMemMan::Free(void *a)
{
	ppluint8 *adr=(ppluint8*)a;
	if (!adr) return;
	MEMSLOT *slot=(MEMSLOT*)(adr-sizeof_memslot);
	MEMMAN *block=slot->block;

	// Handelt es sich um g�ltigen Speicherbereich?
	try {
		if (slot->sig==sig && slot->crc==CRC(slot)) {
			// Den Slot aus der allocated-Kette rausnehmen
			if (slot->previous) slot->previous->next=slot->next;
			if (slot->next) slot->next->previous=slot->previous;
			if (slot==block->allocated) block->allocated=slot->next;

			// Und in die Free-Kette einh�ngen
			if (block->free) block->free->previous=slot;
			slot->previous=NULL;
			slot->next=block->free;
			block->free=slot;

			// Speichercounter korrigieren
			block->bytesfree+=slot->size;
			size_used-=slot->size;
			size_free+=slot->size;
			// FindBiggestSlot
			if (slot->size>block->biggest_block) {
				block->biggest=slot;
				block->biggest_block=slot->size;
			}
		}
	}
	catch (...) {
		return;
	}
}

ppluint64 CMemMan::GetFreeMem()
{
	return size_free;
}

ppluint64 CMemMan::GetUsedMem()
{
	return size_used;
}


}	// EOF Namespace ppl6

