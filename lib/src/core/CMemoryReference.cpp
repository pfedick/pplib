/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.1 $
 * $Date: 2009/12/13 12:53:07 $
 * $Id: CMemoryReference.cpp,v 1.1 2009/12/13 12:53:07 patrick Exp $
 *
 *******************************************************************************
 * Copyright (c) 2009, Patrick Fedick <patrick@pfp.de>
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
#include <stdarg.h>

#include "ppl6.h"


namespace ppl6 {


/*!\class CMemoryReference
 * \ingroup PPLGroupMemory
 * \brief Referenz auf einen Speicherbereich
 *
 * \desc
 * Mit dieser Klasse kann ein einzelner Speicherblock repräsentiert werden.
 * Dieser besteht immer aus einer Speicheraddresse und der Größe des
 * Speicherbereichs in Bytes.
 * \par
 * Der Speicherbereich kann mit CMemory::set gesetzt werden und mit CMemory::adr und CMemory::size
 * ausgelesen werden. Außerdem kann mit dem Operator [] ein bestimmtes Byte aus dem
 * Speicherbereich ausgelesen werden.
 * \par
 * Die Klasse enthält immer nur eine Referenz auf einen bestehenen Speicherbereich,
 * nicht den Speicher selbst. Soll auch der Speicher verwaltet werden, kann man
 * stattdessen die abgeleitete Klasse CMemory verwenden.
 *
 * \see
 * - CMemory
 *
 * \since
 * Diese Klasse wurde mit Version 6.3.3 eingeführt
 */

/*!\var CMemoryReference::ptr
 * \brief Pointer auf den referenzierten Speicherbereich
 *
 * \desc
 * Pointer auf den referenzierten Speicherbereich
 */

/*!\var CMemoryReference::s
 * \brief Größe des referenzierten Speicherbereichs
 *
 * \desc
 * Größe des referenzierten Speicherbereichs
 */

/*!\brief Konstruktor ohne Argumente
 *
 * \desc
 * Dieser Konstruktor erstellt eine Inszanz der Klasse ohne einen zugeweisenen Speicherbereich
 *
 */
CMemoryReference::CMemoryReference()
{
	ptr=NULL;
	s=0;
}

/*!\brief Copy-Konstruktor
 *
 * \desc
 * Mit diesem Konstruktor wird eine Referenz auf den Speicherbereich einer anderen
 * CMemoryReference- oder CMemory-Klasse übernommen.
 *
 * @param[in] other Referenz auf eine andere CMemoryReferenz-Klasse
 */
CMemoryReference::CMemoryReference(const CMemoryReference &other)
{
	ptr=other.ptr;
	s=other.s;
}

/*!\brief Konstruktor mit Angabe einer Speicheradresse und Größe
 *
 * \desc
 * Mit diesem Konstruktor wird eine neue Instanz der Klasse erzeugt, die eine Referenz auf den mit
 * \p adr und \p size angegebenen Speicherbereich enthält.
 *
 * @param[in] adr Pointer auf den Beginn des Speicherbereichs
 * @param[in] size Größe des Speicherbereichs in Bytes
 */
CMemoryReference::CMemoryReference(void *adr, size_t size)
{
	ptr=adr;
	s=size;
}

/*!\brief Prüfen, ob Speicher referenziert ist
 *
 * \desc
 * Mit dieser Funktion kann geprüft werden, ob die Klasse zur Zeit eine Referenz auf einen
 * Speicherbereich enthält.
 *
 * \return
 * Enthält die Klasse keine Referenz auf einen Speicherbereich, liefert die Funktion
 * \c true zurück, andernfalls \c false.
 */
bool CMemoryReference::isNull() const
{
	if (!ptr) return true;
	return false;
}

/*!\brief Prüfen, ob der Referenzierte Speicher eine Größe von 0 hat
 *
 * \desc
 * Mit dieser Funktion kann geprüft werden, ob die Klasse zur Zeit eine Referenz auf einen
 * Speicherbereich enthält und dieser größer als 0 Byte ist.
 *
 * \return
 * Enthält die Klasse keine Referenz auf einen Speicherbereich, der mindestens 1 Byte
 * gross ist, liefert die Funktion \c true zurück, andernfalls \c false.
 */
bool CMemoryReference::isEmpty() const
{
	if (!ptr) return true;
	if (!s) return true;
	return false;
}


/*!\brief Größe des Speicherblocks auslesen
 *
 * \desc
 * Mit dieser Funktion kann die Größe des Speicherblocks ausgelesen werden.
 *
 * @return Größe des Speicherblocks oder 0, wenn kein Speicher zugeordnet ist.
 */
size_t CMemoryReference::size() const
{
	return s;
}

/*!\brief Adresse des Speicherblocks auslesen
 *
 * \desc
 * Mit dieser Funktion wird die Adresse des Speicherblocks ausgelesen
 *
 * @return Adresse des Speicherblocks
 */
void *CMemoryReference::adr() const
{
	return ptr;
}

/*!\brief Referenz auf Speicherbereich setzen
 *
 * \desc
 * Mit dieser Funktion wird der Klasse der Speicherbereich mit der Adresse \p adr und der
 * Größe \p size zugeordnet. Der Speicherbereich selbst wird von der Klasse nicht verwaltet,
 * das heisst die Anwendung muss sich um dessen Freigabe kümmern.
 *
 * @param[in] adr Startadresse des Speicherbereichs
 * @param[in] size Größe des Speicherbereichs in Bytes
 *
 */
void CMemoryReference::set(void *adr, size_t size)
{
	ptr=adr;
	s=size;
}

/*!\brief Referenz auf Speicherbereich von einer anderen CMemoryReference-Instanz kopieren
 *
 * \desc
 * Mit dieser Funktion wird eine Referenz auf einen Speicherbereich von einer anderen
 * CMemoryReference- oder CMemory- Instanz kopiert.
 *
 * @param[in] other Referenz auf ein anderes CMemory-Objekt.
 */
void CMemoryReference::set(const CMemoryReference &other)
{
	ptr=other.ptr;
	s=other.s;
}

/*!\brief Speicherreferenz von anderem CMemoryReference-Objekt übernehmen
 *
 * \desc
 * Mit diesem Operator wird eine Referenz auf einen Speicherbereich von einer anderen
 * CMemoryReference- oder CMemory Instanz übernommen.
 *
 * @param[in] other Referenz auf ein anderes CMemoryReference-Objekt.
 * @return Referenz auf das Objekt
 */
CMemoryReference &CMemoryReference::operator=(const CMemoryReference &other)
{
	ptr=other.ptr;
	s=other.s;
	return *this;
}


/*!\brief Adresse des Speicherblocks auslesen
 *
 * \desc
 * Mit diesem Operator wird die Adresse des Speicherblocks ausgelesen
 *
 * @return Adresse des Speicherblocks
 */
CMemoryReference::operator void*() const
{
	return ptr;
}

/*!\brief Einzelnes Byte aus dem Speicherbereich kopieren
 *
 * \desc
 * Mit dem Operator [] kann ein bestimmtes Byte \p pos aus dem Speicherbereich
 * ausgelesen werden. Ist kein Speicher referenziert oder ist \p pos größer als
 * der Speicherblock, wird eine Exception ausgelöst.
 *
 * @param [in] pos Auszulesendes Byte, beginnend mit 0.
 * @return Wert der Speicherstelle
 * \exception Exception::InvalidAdrdress Diese Exception wird geworfen, wenn die mit
 * \p pos angegebene Speicherstelle ausseralb des referenzierten Speichers liegt oder
 * kein Speicher referenziert ist.
 */
unsigned char CMemoryReference::operator[](size_t pos) const
{
	if (ptr!=NULL && pos<s) return ((unsigned char*)ptr)[pos];
	throw Exception::InvalidAddress();
}


}	// EOF namespace ppl6
