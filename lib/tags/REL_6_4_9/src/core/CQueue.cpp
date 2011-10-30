/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.2 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: CQueue.cpp,v 1.2 2010/02/12 19:43:48 pafe Exp $
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
#ifdef HAVE_STRING_H
	#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
	#include <stdlib.h>
#endif
#include "ppl6.h"

namespace ppl6 {

class CQueueItem
{
	friend class CQueue;
	private:
		CQueueItem *previous, *next;
		const void *value;
};

/*!\class CQueue
 * \brief Eine Queue-Klasse
 *
 * Dies ist eine Klasse zum "parken" von Objekten in einer Queue. Sie arbeitet nach dem FiFo-Prinzip
 * (First In, First Out), das heißt was zuerst hineingegeben wurde, kommt auch zuerst wieder
 * hinaus.
 *
 * Die Anzahl Objekte in der Queue kann man durch Aufruf von CQueue::SetMaximum limitieren.
 *
 * \since
 * Diese Klasse wurde mit Version 6.2.6 eingeführt und in 6.3.0 überarbeitet und dokumentiert.
 *
 */

CQueue::CQueue()
/*!\brief Konstruktor der Klasse
 *
 * Der Konstruktor initialisiert die Liste und setzt den Counter auf 0. Die Anzahl Objekte
 * ist unlimitiert.
 */
{
	count=0;
	max=0;
	first=last=NULL;
	usemutex=false;
}

CQueue::CQueue(int max)
/*!\brief Konstruktor der Klasse mit Limit
 *
 * Der Konstruktor initialisiert die Liste und setzt den Counter auf 0. Die maximale
 * Anzahl Objekte wird auf den angegebenen Wert \p max gesetzt.
 *
 * \param[in] max Limit für die Anzahl Objekte in der Queue
 */
{
	count=0;
	this->max=max;
	first=last=NULL;
}

CQueue::~CQueue()
/*!\brief Destruktor der Klasse
 *
 * Der Destruktor ruft die Clear-Funktion der Klasse auf, womit die Queue gelöscht wird.
 * Dabei muß berücksichtigt werden, dass nur die Queue selbst gelöscht wird, nicht aber die
 * darin eventuell noch vorhandenen Objekte. Dies ist nicht möglich, da die Queue den Typ
 * der Objekte nicht kennt und so deren Speicher nicht freigeben kann.
 */
{
	Clear();
}

void CQueue::Clear()
/*!\brief Queue löschen
 *
 * Mit diesem Befehl wird die Queue gelöscht.
 * Dabei muß berücksichtigt werden, dass nur die Queue selbst gelöscht wird, nicht aber die
 * darin eventuell noch vorhandenen Objekte. Dies ist nicht möglich, da die Queue den Typ
 * der Objekte nicht kennt und so deren Speicher nicht freigeben kann.
 *
 * Vor Aufruf der Funktion sollten daher alle verbliebenen Objekte von der Anwendung
 * mit CQueue::Get entfernt und ordnungsgemäß gelöscht werden.
 *
 */
{
	if (usemutex) Mutex.Lock();
	CQueueItem *item=(CQueueItem*)first;
	while (item) {
		first=item;
		item=item->next;
		delete (CQueueItem*)first;
	}

	first=last=NULL;
	count=0;
	if (usemutex) Mutex.Unlock();
}

void CQueue::SetMaximum(int max)
/*!\brief Limit setzen
 *
 * Mit diesem Befehl wird die maximale Anzahl Objekte in der Queue auf den angegebenen
 * Wert \p max begrenzt. Dies ist allerdings nur möglich, wenn die Liste noch leer ist oder
 * weniger Elemente als \p max enthält.
 *
 * @param[in] max Maximale Anzahl Objekte. Die Angabe von 0 hebt das Limit auf.
 */
{
	if (count>max) return;
	this->max=max;
}
void CQueue::UseMutex(bool flag)
/*!\brief Threadsicherheit
 *
 * Wird diese Funktion mit \p true aufgerufen, wird bei jeder Operation mit Put, Add oder Get
 * ein Mutex gesetzt, womit die Klasse Threadsicher wird. Defaultmäßig wird kein Mutex verwendet,
 * wodurch die Klasse etwas schneller ist. Die Anwendung muss dann selbst sicherstellen, dass
 * nicht mehrere Threads parallel zugreifen.
 *
 * @param[in] flag Mutex ein oder ausschalten (true/false)
 */
{
	usemutex=flag;
}

int CQueue::Put(const void *item)
/*!\brief Element in die Queue legen
 *
 * Mit diesem Befehl wird der Zeiger auf ein beliebiges Objekt in der Queue abgelegt.
 *
 * \param[in] item Pointer auf das Objekt oder den Speicherbereich, der in die Queue gelegt
 * werden soll.
 * \return Bei Erfolg liefert die Funktion 1 zurück, sonst 0. Ein Fehler kann auftreten, wenn
 * ein Limit auf die Anzahl Objekte angegeben wurde und dieses erreicht ist, oder wenn kein
 * Speicher mehr zur Verfügung steht. Auch darf \p item nicht NULL sein.
 *
 */
{
	if (!item) {
		SetError(194);
		return 0;
	}
	if (usemutex) Mutex.Lock();
	if (max>0 && count>=max) {
		if (usemutex) Mutex.Unlock();
		SetError(514);
		return 0;
	}
	CQueueItem *it = new CQueueItem;
	if (!it) {
		if (usemutex) Mutex.Unlock();
		SetError(2);
		return 0;
	}
	it->next=NULL;
	it->previous=NULL;
	it->value=item;

	it->previous=(CQueueItem*)last;
	if (last) ((CQueueItem*)last)->next=it;
	if (!first) first=it;
	last=it;
	count++;
	if (usemutex) Mutex.Unlock();
	return 1;
}

int CQueue::Add(const void *item)
/*!\brief Element in die Queue legen
 *
 * Mit diesem Befehl wird der Zeiger auf ein beliebiges Objekt in der Queue abgelegt.
 *
 * \param[in] item Pointer auf das Objekt oder den Speicherbereich, der in die Queue gelegt
 * werden soll.
 * \return Bei Erfolg liefert die Funktion 1 zurück, sonst 0. Ein Fehler kann auftreten, wenn
 * ein Limit auf die Anzahl Objekte angegeben wurde und dieses erreicht ist, oder wenn kein
 * Speicher mehr zur Verfügung steht. Auch darf \p item nicht NULL sein.
 *
 */
{
	return Put(item);
}

const void *CQueue::Get()
/*!\brief Objekt aus der Queue nehmen.
 *
 * Mit diesem Befehl wird ein Objekt aus der Queue entfernt und dessen Pointer zurückgegeben.
 *
 * \return Pointer auf das älteste Objekt aus der Queue oder NULL, wenn die Queue leer ist.
 *
 */
{
	const void *ret=NULL;
	if (usemutex) Mutex.Lock();
	if (first) {
		ret=((CQueueItem*)first)->value;
		if (((CQueueItem*)first)->next) ((CQueueItem*)first)->next->previous=NULL;
		first=((CQueueItem*)first)->next;
		if (!first) last=NULL;
		count--;
	} else {
		ret=NULL;
		SetError(516);
	}
	if (usemutex) Mutex.Unlock();
	return ret;
}

int CQueue::Count()
/*!\brief Anzahl Elemente in der Queue
 *
 * Dieser Befehl liefert die Anzahl Elemente zurück, die sich aktuell in der Queue befinden.
 *
 * \return Anzahl Elemente in der Queue
 */
{
	int ret=0;
	if (usemutex) Mutex.Lock();
	ret=count;
	if (usemutex) Mutex.Unlock();
	return ret;
}



}	// EOF namespace ppl6
