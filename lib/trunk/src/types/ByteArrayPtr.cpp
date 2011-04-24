/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ppl7.h"


namespace ppl7 {


/*!\class ByteArrayPtr
 * \ingroup PPLGroupMemory
 * \brief Referenz auf einen Speicherbereich
 *
 * \desc
 * Mit dieser Klasse kann ein einzelner Speicherblock repräsentiert werden.
 * Dieser besteht immer aus einer Speicheraddresse und der Größe des
 * Speicherbereichs in Bytes.
 * \par
 * Der Speicherbereich kann mit ByteArray::set gesetzt werden und mit ByteArray::adr und ByteArray::size
 * ausgelesen werden. Außerdem kann mit dem Operator [] ein bestimmtes Byte aus dem
 * Speicherbereich ausgelesen werden.
 * \par
 * Die Klasse enthält immer nur eine Referenz auf einen bestehenen Speicherbereich,
 * nicht den Speicher selbst. Soll auch der Speicher verwaltet werden, kann man
 * stattdessen die abgeleitete Klasse ByteArray verwenden.
 *
 * \see
 * - ByteArray
 */

/*!\var ByteArrayPtr::ptr
 * \brief Pointer auf den referenzierten Speicherbereich
 *
 * \desc
 * Pointer auf den referenzierten Speicherbereich
 */

/*!\var ByteArrayPtr::s
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
ByteArrayPtr::ByteArrayPtr()
{
	type=BYTEARRAYPTR;
	ptr=NULL;
	s=0;
}

/*!\brief Copy-Konstruktor
 *
 * \desc
 * Mit diesem Konstruktor wird eine Referenz auf den Speicherbereich einer anderen
 * ByteArrayPtr- oder ByteArray-Klasse übernommen.
 *
 * @param[in] other Referenz auf eine andere ByteArrayReferenz-Klasse
 */
ByteArrayPtr::ByteArrayPtr(const ByteArrayPtr &other)
{
	type=BYTEARRAYPTR;
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
ByteArrayPtr::ByteArrayPtr(void *adr, size_t size)
{
	type=BYTEARRAYPTR;
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
bool ByteArrayPtr::isNull() const
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
bool ByteArrayPtr::isEmpty() const
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
size_t ByteArrayPtr::size() const
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
const void *ByteArrayPtr::adr() const
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
void ByteArrayPtr::use(void *adr, size_t size)
{
	ptr=adr;
	s=size;
}

/*!\brief Referenz auf Speicherbereich von einer anderen ByteArrayPtr-Instanz kopieren
 *
 * \desc
 * Mit dieser Funktion wird eine Referenz auf einen Speicherbereich von einer anderen
 * ByteArrayPtr- oder ByteArray- Instanz kopiert.
 *
 * @param[in] other Referenz auf ein anderes ByteArray-Objekt.
 */
void ByteArrayPtr::use(const ByteArrayPtr &other)
{
	ptr=other.ptr;
	s=other.s;
}

/*!\brief Speicherreferenz von anderem ByteArrayPtr-Objekt übernehmen
 *
 * \desc
 * Mit diesem Operator wird eine Referenz auf einen Speicherbereich von einer anderen
 * ByteArrayPtr- oder ByteArray Instanz übernommen.
 *
 * @param[in] other Referenz auf ein anderes ByteArrayPtr-Objekt.
 * @return Referenz auf das Objekt
 */
ByteArrayPtr &ByteArrayPtr::operator=(const ByteArrayPtr &other)
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
ByteArrayPtr::operator const void*() const
{
	return ptr;
}

/*!\brief Adresse des Speicherblocks auslesen
 *
 * \desc
 * Mit diesem Operator wird die Adresse des Speicherblocks ausgelesen
 *
 * @return Adresse des Speicherblocks
 */
ByteArrayPtr::operator const unsigned char*() const
{
	return (const unsigned char*)ptr;
}

/*!\brief Adresse des Speicherblocks auslesen
 *
 * \desc
 * Mit diesem Operator wird die Adresse des Speicherblocks ausgelesen
 *
 * @return Adresse des Speicherblocks
 */
ByteArrayPtr::operator const char*() const
{
	return (const char*)ptr;
}

/*!\brief Einzelnes Byte aus dem Speicherbereich auslesen
 *
 * \desc
 * Mit dem Operator [] kann ein bestimmtes Byte \p pos aus dem Speicherbereich
 * ausgelesen werden. Ist kein Speicher referenziert oder ist \p pos größer als
 * der Speicherblock, wird eine Exception ausgelöst.
 *
 * @param [in] pos Auszulesendes Byte, beginnend mit 0.
 * @return Wert der Speicherstelle
 * \exception OutOfBoundsEception Diese Exception wird geworfen, wenn die mit
 * \p pos angegebene Speicherstelle ausseralb des referenzierten Speichers liegt oder
 * kein Speicher referenziert ist.
 */
unsigned char ByteArrayPtr::operator[](size_t pos) const
{
	if (ptr!=NULL && pos<s) return ((unsigned char*)ptr)[pos];
	throw OutOfBoundsEception();
}


void ByteArrayPtr::set(size_t pos, unsigned char value)
{
	if (pos<s) ((unsigned char*)ptr)[pos]=value;
	else throw OutOfBoundsEception();
}

unsigned char ByteArrayPtr::get(size_t pos) const
{
	if (ptr!=NULL && pos<s) return ((unsigned char*)ptr)[pos];
	throw OutOfBoundsEception();
}


/*!\brief Speicherbereich als Hexwerte in einen String exportieren
 *
 * \desc
 * Der referenzierte Speicherbereich wird als String mit Hexadezimalwerten
 * exportiert, wobei jedes Byte als zwei Zeichen langer Hexadezimalwert
 * dargestellt wird.
 *
 * @return String mit Hexadezimal-Werten
 */
String ByteArrayPtr::toHex() const
{
	unsigned char *buff=(unsigned char*)ptr;
	String str;
	for(size_t i=0;i<s;i++) str.appendf("%02x",buff[i]);
	return str;
}

/*!\brief Speicherbereich als Base64 in einen String exportieren
 *
 * \desc
 * Der referenzierte Speicherbereich wird im Base64-Format als String
 * exportiert.
 *
 * @return Inhalt des Speicherbereichs als Base64-String.
 */
String ByteArrayPtr::toBase64() const
{
	return ToBase64(*this);
}


ppluint32 ByteArrayPtr::crc32() const
{
	if (s==0) throw EmptyDataException();
	return Crc32(ptr,s);
}

void ByteArrayPtr::hexDump() const
{
	if (s==0) throw EmptyDataException();
	printf ("HEXDUMP of ByteArray: %zi Bytes starting at Address %p:\n",s,ptr);
	HexDump(ptr,s,true);
}


}	// EOF namespace ppl7
