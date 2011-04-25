/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 32 $
 * $Date: 2011-04-24 20:16:39 +0000 (So, 24 Apr 2011) $
 * $Id: Array.cpp 32 2011-04-24 20:16:39Z pafe $
 * $URL: https://pplib.svn.sourceforge.net/svnroot/pplib/lib/trunk/src/types/Array.cpp $
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

typedef struct tagHeapElement {
	struct tagHeapElement	*next, *previous;
	void					*ptr;
} HEAPELEMENT;

typedef struct tagHeapBlock {
	struct tagHeapBlock	*previous, *next;
	void				*buffer;
	void				*bufferend;
	size_t				elements;
	size_t				num_free;
	HEAPELEMENT			*free;
	HEAPELEMENT			*elbuffer;

} HEAPBLOCK;


/*!\class Heap
 * \ingroup PPLGroupMemory
 * \brief Speicherverwaltung in Heaps
 *
 */


Heap::Heap()
{
	blocks=NULL;
	elementSize=0;
	increaseSize=0;
	incCount=0;
	mem_allocated=sizeof(Heap);
	mem_used=sizeof(Heap);
}

Heap::Heap(size_t elementsize, size_t startnum, size_t increase)
{
	blocks=NULL;
	elementSize=0;
	increaseSize=0;
	incCount=0;
	mem_allocated=sizeof(Heap);
	mem_used=sizeof(Heap);
	init(elementsize, startnum, increase);
}

Heap::~Heap()
{
	clear();
}

void Heap::clear()
{
	HEAPBLOCK *next, *bl=(HEAPBLOCK*)blocks;
	while (bl) {
		next=bl->next;
		free(bl->elbuffer);
		free(bl->buffer);
		free(bl);
		bl=next;
	}
	incCount=0;
	blocks=NULL;
	mem_allocated=sizeof(Heap);
	mem_used=sizeof(Heap);
}

void Heap::init(size_t elementsize, size_t startnum, size_t increase)
{
	if (elementSize) throw AlreadyInitializedException();
	//Elementsize auf 4 Byte aufrunden
	if (!elementsize) throw IllegalArgumentException("elementsize");
	elementsize=(elementsize+3)&0xfffffffc;

	this->elementSize=elementsize;
	this->increaseSize=increase;
	if (startnum) this->increase(startnum);
}

void Heap::increase(size_t num)
{
	HEAPBLOCK *bl=(HEAPBLOCK*)::malloc(sizeof(HEAPBLOCK));
	if (!bl) throw OutOfMemoryException();
	bl->elements=num;
	bl->next=NULL;
	bl->num_free=num;
	bl->previous=NULL;
	bl->buffer=::malloc(elementSize*num);
	if (!bl->buffer) {
		free(bl);
		throw OutOfMemoryException();
	}
	bl->bufferend=(ppluint8*)bl->buffer+elementSize*num;
	bl->free=(HEAPELEMENT *)::malloc(sizeof(HEAPELEMENT)*num);
	if (!bl->free) {
		free(bl->buffer);
		free(bl);
		throw OutOfMemoryException();
	}
	bl->elbuffer=bl->free;
	HEAPELEMENT *t,*prev=NULL;
	ppluint8 *buffer=(ppluint8*)bl->buffer;
	for(size_t i=0;i<num;i++) {
		t=&bl->free[i];
		t->previous=prev;
		t->next=&bl->free[i+1];
		t->ptr=buffer;
		buffer+=elementSize;
		prev=t;
	}
	bl->free[num-1].next=NULL;
	bl->next=(HEAPBLOCK*)blocks;
	if (bl->next) bl->next->previous=bl;
	blocks=bl;
	incCount++;
	increaseSize+=(increaseSize*30/100);
	mem_allocated+=sizeof(HEAPBLOCK)+elementSize*num+sizeof(HEAPELEMENT)*num;
	mem_used+=sizeof(HEAPBLOCK);
}

void *Heap::calloc()
{
	void *block=malloc();
	memset(block,0,elementSize);
	return block;
}

void *Heap::malloc()
{
	if (!elementSize) throw NotInitializedException();
	while (1) {
		// Den nächsten freien Block suchen
		HEAPBLOCK *bl=(HEAPBLOCK*)blocks;
		while (bl) {
			if (bl->num_free) {
				HEAPELEMENT *el=bl->free;
				// Element aus der Free-Kette nehmen
				bl->free=bl->free->next;
				if(bl->free) bl->free->previous=NULL;
				bl->num_free--;
				mem_used+=elementSize+sizeof(HEAPELEMENT);
				return el->ptr;
			}
			bl=bl->next;
		}
		// Speicher muss vergroessert werden
		increase(increaseSize);
	}
}

void Heap::free(void *mem)
{
	if (!elementSize) throw NotInitializedException();
	HEAPBLOCK *bl=(HEAPBLOCK*)blocks;
	while (bl) {
		if (mem>=bl->buffer && mem<=bl->bufferend) {
			// Nummer des Blocks errechnen
			HEAPELEMENT *el=bl->elbuffer;
			int element=(ppluint32)((ppluint8*)mem-(ppluint8*)bl->buffer)/elementSize;
			if(el[element].ptr!=mem) {
				// Hier stimmt was nicht!!!!
				throw HeapCorruptedException();
			}
			// Element in die Free-Kette hängen
			el[element].next=bl->free;
			el[element].previous=NULL;
			if (bl->free) bl->free->previous=&el[element];
			bl->free=&el[element];
			bl->num_free++;
			mem_used-=(elementSize+sizeof(HEAPELEMENT));
			if (bl->num_free==bl->elements) cleanup();
			return;
		}
		bl=bl->next;
	}
	throw ElementNotInHeapException();
}

void Heap::cleanup()
{
	// Wenn mehr als ein Block komplett leer ist, geben wir ihn frei
	HEAPBLOCK *next, *bl=(HEAPBLOCK*)blocks;
	bool flag=false;
	while (bl) {
		next=bl->next;
		if (bl->num_free==bl->elements) {
			if (flag) {
				// Block wird gelöscht
				if (bl->previous) bl->previous->next=bl->next;
				if (bl->next) bl->next->previous=bl->previous;
				mem_allocated-=sizeof(HEAPBLOCK)+elementSize*bl->elements+sizeof(HEAPELEMENT)*bl->elements;
				mem_used-=sizeof(HEAPBLOCK);
				free(bl->buffer);
				free(bl->elbuffer);
				free(bl);
			}
			flag=true;
		}
		bl=next;
	}
}

size_t Heap::memoryUsed() const
{
	return mem_used;
}

size_t Heap::memoryAllocated() const
{
	return mem_allocated;
}

void Heap::dump() const
{
	HEAPBLOCK *bl=(HEAPBLOCK*)blocks;
	printf ("Dump Heap (0x%tx, ",(ptrdiff_t)this);
	printf ("Elementsize: %zu):\n", elementSize);
	printf ("Memory allocated: %zu Bytes, Memory used: %zu Bytes, Memory free: %zu Bytes\n",
			mem_allocated, mem_used, (mem_allocated-mem_used));
	while (bl) {
		printf ("HEAPBLOCK: elements: %zu, free: %zu, Bytes allocated: %zu\n",bl->elements, bl->num_free, bl->elements*elementSize);
		bl=bl->next;
	}
}

}	// EOF namespace ppl7
