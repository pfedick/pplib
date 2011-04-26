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
 * Diese Klasse kann verwendet werden, wenn häufig kleine gleichgroße Speicherblöcke allokiert
 * und wieder freigegeben werden müssen, z.B. von einer häufig verwendeten Klasse oder
 * Kontrollstrukturen für Listen und binäre Bäume. Statt den Speicherbereich jedesmal
 * per Malloc oder new vom Betriebssystem anzufordern, verwaltet diese Klasse eigene
 * größere Speicherblöcke, aus denen die malloc-Anfragen bedient werden. Dadurch wird
 * verhindert, dass der Speicher zu sehr fragmentiert wird.
 *
 * Bevor die Klasse verwendet werden kann, muss sie zunächst mittels Konstruktor oder
 * der Funktion Heap::init initialisiert werden. Dabei muss insbesondere die Größe
 * der Elemente angegeben werden und die Anzahl Elemente, um die der Heap jeweils wachsen
 * soll, wenn kein Speicher mehr frei ist. Initial kann dabei auch schon Speicher allokiert
 * werden.
 *
 */


/*!\brief Konstruktor
 *
 * \desc
 * Bei Verwendung dieses Konstruktors muss anschließend die Funktion Heap::init
 * aufgerufen werden.
 *
 */
Heap::Heap()
{
	blocks=NULL;
	elementSize=0;
	increaseSize=0;
	blocksAllocated=0;
	blocksUsed=0;
	freeCount=0;
	mem_allocated=sizeof(Heap);
	mem_used=sizeof(Heap);
}

/*!\brief Konstruktor mit Initialisierung
 *
 * \desc
 * Bei Verwendung dieses Konstruktors wird die Klasse gleichzeitig auch initialisiert.
 *
 * @param elementsize Die Größe der Elemente in Bytes (wird auf 4 Byte aufgerundet)
 * @param startnum Anzahl Elemente, für die sofort Speicher allokiert werden soll
 * @param increase Anzahl Elemente, um die der Heap wachsen soll, wenn keine Elemente mehr
 * frei sind.
 * \exception OutOfMemoryException: Wird geworfen, wenn nicht genug Speicher verfügbar ist, um den
 * Heap anzulegen.
 */
Heap::Heap(size_t elementsize, size_t startnum, size_t increase)
{
	blocks=NULL;
	elementSize=0;
	increaseSize=0;
	blocksAllocated=0;
	blocksUsed=0;
	freeCount=0;
	mem_allocated=sizeof(Heap);
	mem_used=sizeof(Heap);
	init(elementsize, startnum, increase);
}

/*!\brief Destruktor
 *
 * \desc
 * Der Destruktor sorgt dafür, dass der komplette durch den Heap belegte Speicher
 * freigegeben wird.
 */
Heap::~Heap()
{
	clear();
}

/*!\brief Gesamten Speicher freigeben
 *
 * \desc
 * Sämtlicher durch den Heap belegte Speicher wird freigegeben. Alle durch
 * Heap::malloc oder Heap:calloc allokierten Speicherblöcke verlieren ihre Gültigkeit und
 * dürfen nicht mehr verwendet werden.
 *
 */
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
	freeCount=0;
	blocksAllocated=0;
	blocksUsed=0;
	blocks=NULL;
	mem_allocated=sizeof(Heap);
	mem_used=sizeof(Heap);
}

/*!\brief Derzeitige Kapazität des Heaps
 *
 * \desc
 * Mit dieser Funktion kann abgefragt werden wieviele Elemente insgesamt allokiert werden
 * können, ohne dass neue Speicherblöcke vom Betriebssystem angefordert werden müssen.
 * Wieviele davon tatsächlich schon verbraucht sind, kann mittels Heap::count() abgefragt
 * werden.
 *
 * @return Anzahl Elemente, für die Speicher vorrätig ist
 */
size_t Heap::capacity() const
{
	return blocksAllocated;
}

/*!\brief Anzahl belegterElemente
 *
 * \desc
 * Liefert die Anzahl Elemente zurück, die derzeit in Verwendung sind.
 *
 * @return Anzahl Elemente
 */
size_t Heap::count() const
{
	return blocksUsed;
}

/*!\Speicher reservieren
 *
 * \desc
 * Mit dieser Funktion kann vorab Speicher für eine bestimmte Anzahl Elemente reserviert werden.
 * Der Aufruf dieser Funktion ist immer dann sinnvoll, wenn schon vorher bekannt ist, wieviele
 * Elemente benötigt werden, insbesondere, wenn sehr viele Elemente benötigt werden
 * (z.B. Aufbau eines binären Baums).
 *
 * @param num Anzahl Elemente, für die Speicher vorab allokiert werden soll
 *
 * \note Falls schon Speicher allokiert wurde, wird die Anzahl der bereits allokierten Elemente
 * mit \p num verreichnet und nur die Differenz zusätzlich reserviert.
 */
void Heap::reserve(size_t num)
{
	if (num>blocksAllocated) {
		size_t grow=num-blocksAllocated;
		increase(grow);
	}
}

/*!\brief Initialisierung der Klasse
 *
 * \desc
 * Diese Funktion muss vor allen anderen aufgerufen werden, um die Klasse zu initialisieren.
 * Dabei muss insbesondere die Größe der Elemente angegeben werden und die Anzahl Elemente,
 * um die der Heap jeweils wachsen soll, wenn kein Speicher mehr frei ist. Initial kann dabei
 * auch schon Speicher allokiert werden.
 *
 * @param elementsize Die Größe der Elemente in Bytes (wird auf 4 Byte aufgerundet)
 * @param startnum Anzahl Elemente, für die sofort Speicher allokiert werden soll
 * @param increase Anzahl Elemente, um die der Heap wachsen soll, wenn keine Elemente mehr
 * frei sind.
 * \exception OutOfMemoryException: Wird geworfen, wenn nicht genug Speicher verfügbar ist, um den
 * Heap anzulegen.
 */
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

/*!\brief Heap vergrößern
 *
 * \desc
 * Interne Funktion, die aufgerufen wird, um den Heap um eine bestimmte Anzahl Elemente zu
 * vergrößern.
 *
 * @param num Anzahl Elemente, für die neuer Speicher allokiert werden soll
 * \exception OutOfMemoryException: Wird geworfen, wenn nicht genug Speicher verfügbar ist, um den
 * Heap anzulegen.
 *
 */
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
	blocksAllocated+=num;
	increaseSize+=(increaseSize*30/100);
	mem_allocated+=sizeof(HEAPBLOCK)+elementSize*num+sizeof(HEAPELEMENT)*num;
	mem_used+=sizeof(HEAPBLOCK);
}

/*!\brief Mit 0 initialisierten Speicher anfordern
 *
 * \desc
 * Mit dieser Funktion wird ein neuer Speicherblock aus dem Heap allokiert und mit
 * Null-Bytes initialisiert.
 *
 * \return
 * Pointer auf den allokierten Speicherbereich
 *
 * \exception OutOfMemoryException: Wird geworfen, wenn nicht genug Speicher verfügbar ist, um den
 * Heap anzulegen.
 *
 */
void *Heap::calloc()
{
	void *block=malloc();
	memset(block,0,elementSize);
	return block;
}

/*!\brief Speicher anfordern
 *
 * \desc
 * Mit dieser Funktion wird ein neuer Speicherblock aus dem Heap allokiert. Dieser wird
 * nicht initialisiert und kann daher Zufallsdaten enthalten.
 *
 * \return
 * Pointer auf den allokierten Speicherbereich
 *
 * \exception OutOfMemoryException: Wird geworfen, wenn nicht genug Speicher verfügbar ist, um den
 * Heap anzulegen.
 *
 */
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
				blocksUsed++;
				return el->ptr;
			}
			bl=bl->next;
		}
		// Speicher muss vergroessert werden
		increase(increaseSize);
	}
}

/*!\brief Speicher freigeben
 *
 * \desc
 * Speicher, der zuvor mit Heap::malloc oder Heap::calloc allokiert wurde, wird wieder
 * freigegeben.
 *
 * @param mem Pointer auf den freizugebenden Speicherbereich
 *
 * \exception Heap::HeapCorruptedException: könnte auftreten, wenn der interne Speicher des
 * Heaps, in dem die Elemente verwaltet werden, überschrieben wird.
 * \exception Heap::ElementNotInHeapException: Der mit \p mem referenzierte Speicherblock
 * wurde nicht über diesen Heap allokiert.
 */
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
			//if (bl->num_free==bl->elements) cleanup();
			blocksUsed--;
			freeCount++;
			if (freeCount>1000) cleanup();

			return;
		}
		bl=bl->next;
	}
	throw ElementNotInHeapException();
}

/*!\brief Aufräumen
 *
 * \desc
 * Diese Funktion prüft, ob der Heap ungenutze Speicherbereiche verwaltet und gibt diese
 * frei. Das schließt Speicherbereiche, die mit Heap::reserve reserviert wurden, mit ein.
 * Die Funktion wird automatisch nach 1000 Aufrufen von Heap::free aufgerufen.
 */
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
				blocksAllocated-=bl->elements;
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
	freeCount=0;
}

/*!\brief Anzahl Bytes, die verwendet werden
 *
 * \desc
 * Gibt zurück, wieviel Bytes zur Zeit durch den Heap in Verwendung sind, einschließlich
 * der Verwaltungsstrukturen.
 *
 * @return Anzahl Byte
 */
size_t Heap::memoryUsed() const
{
	return mem_used;
}

/*!\brief Anzahl Bytes, die allokiert sind
 *
 * \desc
 * Gibt zurück, wieviel Bytes zur Zeit durch den Heap allokiert sind, einschließlich
 * Verwaltungsstrukturen und Speicherelementen, die noch nicht in Verwendung sind.
 *
 * @return Anzahl Byte
 */
size_t Heap::memoryAllocated() const
{
	return mem_allocated;
}

/*!\brief Debug-Informationen
 *
 * \desc
 * Diese Funktion gibt einige Debug-Informationen zum Heap aus, insbesondere Anzahl
 * und Größe der durch den Heap verwalteten Speicherblöcke.
 */
void Heap::dump() const
{
	HEAPBLOCK *bl=(HEAPBLOCK*)blocks;
	PrintDebug ("Dump Heap (0x%tx, ",(ptrdiff_t)this);
	PrintDebug ("Elementsize: %zu):\n", elementSize);
	PrintDebug ("Memory allocated: %zu Bytes, Memory used: %zu Bytes, Memory free: %zu Bytes\n",
			mem_allocated, mem_used, (mem_allocated-mem_used));
	PrintDebug ("Blocks allocated: %zu, Blocks used: %zu, freeCount: %zu\n",
			blocksAllocated, blocksUsed, freeCount);
	while (bl) {
		PrintDebug ("HEAPBLOCK: elements: %zu, free: %zu, Bytes allocated: %zu\n",bl->elements, bl->num_free, bl->elements*elementSize);
		bl=bl->next;
	}
}

}	// EOF namespace ppl7
