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


/*!\class ByteArray
 * \ingroup PPLGroupDataTypes
 * \brief Verwaltung eines einzelnen Speicherblocks
 *
 * \desc
 * Mit dieser Klasse kann ein einzelner Speicherblock verwaltet werden.
 * Dieser besteht immer aus einer Speicheraddresse und der Größe des
 * Speicherbereichs in Bytes. Die Klasse stellt sicher, dass am Ende des
 * allokierten Speicherbereichs immer ein 0-Byte angehangen wird, ausser bei Verwendung
 * der Funktion ByteArray::useadr. Dadurch kann bei Verwendung der Pointer-Operatoren
 * der Speicher direkt als Eingabe für C-Strings verwendet werden.
 *
 * \par
 * Der Speicherbereich kann mit ByteArray::set gesetzt werden und mit ByteArray::adr und ByteArray::size
 * ausgelesen werden. Außerdem kann mit dem Operator [] ein bestimmtes Byte aus dem
 * Speicherbereich ausgelesen werden.
 * \par
 * Im Gegensatz zu ByteArrayPtr verwaltet die Klasse den Speicherbereich selbst. Das heisst
 * der verwaltete Speicher wird bei Veränderungen oder Beendigung der Klasse automatisch freigegeben.
 * \par
 * Mit ByteArray::malloc oder ByteArray::calloc wird Speicher allokiert, mit ByteArray::free kann er
 * vorzeitig wieder freigegeben werden. Mit ByteArray::copy wird der Speicher eines anderen ByteArray-
 * oder ByteArrayPtr-Objekts oder aus einer angegebenen Speicheradresse dupliziert.
 * \par
 * Mit ByteArray::set oder durch den Konstruktor angegebener Speicherbereich wird nicht dupliziert,
 * wird aber fortan von der Klasse verwaltet, das heisst die Anwendung darf diesen nicht selbst
 * freigeben.
 *
 * \see
 * - ByteArrayPtr
 *
 * \since
 * Diese Klasse wurde mit Version 6.3.3 eingeführt
 */

/*!\var ByteArray::manage
 * \brief Flag, das die Verwaltung des Speichers regelt.
 *
 * \desc
 * Enthält dieses Flag \c true, wird der Speicher von der ByteArray-Klasse verwaltet. Das heisst
 * beim Aufruf des Konstruktors oder Aufruf einer Funktion, die eine Änderung des Speicherblocks
 * bewirkt, wird der zuvor referenzierte Speicher freigegeben.
 */

/*!\brief Konstruktor ohne Argumente
 *
 * \desc
 * Dieser Konstruktor erstellt eine Inszanz der Klasse ohne einen zugeweisenen Speicherbereich
 *
 */
ByteArray::ByteArray()
{
	type=BYTEARRAY;
	ptr=NULL;
	s=0;
}

/*!\brief Destruktor der Klasse
 *
 * \desc
 * Der Destruktor sort dafür, dass der allokierte Speicher mittes "free" wieder freigegeben wird.
 *
 */
ByteArray::~ByteArray()
{
	::free(ptr);
	ptr=NULL;
}

/*!\brief Copy-Konstruktor
 *
 * \desc
 * Mit diesem Konstruktor wird der Speicherbereich eines anderen ByteArray- oder ByteArrayPtr-
 * Objekts kopiert.
 *
 * @param[in] other Referenz auf eine andere ByteArray- oder ByteArrayPtr-Klasse
 */
ByteArray::ByteArray(const ByteArrayPtr &other)
{
	type=BYTEARRAY;
	if (other.ptr) {
		ptr=::malloc(other.s+1);
		if (!ptr) throw OutOfMemoryException();
		memcpy(ptr,other.ptr,other.s);
		s=other.s;
		((char*)ptr)[s]=0;
	} else {
		ptr=NULL;
		s=0;
	}
}

/*!\brief Copy-Konstruktor
 *
 * \desc
 * Mit diesem Konstruktor wird der Speicherbereich eines anderen ByteArray- oder ByteArrayPtr-
 * Objekts kopiert.
 *
 * @param[in] other Referenz auf eine andere ByteArray- oder ByteArrayPtr-Klasse
 */
ByteArray::ByteArray(const ByteArray &other)
{
	type=BYTEARRAY;
	if (other.ptr) {
		ptr=::malloc(other.s+1);
		if (!ptr) throw OutOfMemoryException();
		memcpy(ptr,other.ptr,other.s);
		s=other.s;
		((char*)ptr)[s]=0;
	} else {
		ptr=NULL;
		s=0;
	}
}


/*!\brief Konstruktor mit Angabe einer Speicheradresse und Größe
 *
 * \desc
 * Mit diesem Konstruktor wird eine neue Instanz der Klasse erzeugt, und der
 * mit \p adr und \p size angegebenen Speicherbereich kopiert.
 *
 * @param[in] adr Pointer auf den Beginn des Speicherbereichs
 * @param[in] size Größe des Speicherbereichs in Bytes
 */
ByteArray::ByteArray(void *adr, size_t size)
{
	type=BYTEARRAY;
	ptr=NULL;
	s=0;
	copy(adr,size);
}

/*!\brief Konstruktor mit Speicher-Allokierung
 *
 * \desc
 * Mit diesem Konstruktor wird eine Instanz der Klasse erzeugt, die einen Speicherblock mit
 * der Größe \p size allokiert und diesen selbst verwaltet.
 *
 * @param[in] size Gewünschte Größe des Speicherblocks in Bytes
 * \exception OutOfMemoryException Diese Exception wird geworfen, wenn der Speicher nicht allokiert werden konnte
 */
ByteArray::ByteArray(size_t size)
{
	type=BYTEARRAY;
	s=0;
	ptr=::malloc(size+1);
	if (!ptr) throw OutOfMemoryException();
	s=size;
	((char*)ptr)[s]=0;
}

/*!\brief Speicherverwaltung übernehmen
 *
 * \desc
 * Mit dieser Funktion wird der Klasse die Verwaltung des Speicherbereich mit der Adresse \p adr und der
 * Größe \p size übergeben. Der Speicher muss zuvor mit "malloc" bzw. "calloc" allokiert worden sein
 * und darf von der Anwendung selbst nicht mehr freigegeben werden.
 *
 * @param[in] adr Startadresse des Speicherbereichs
 * @param[in] size Größe des Speicherbereichs in Bytes
 *
 * \note
 * Falls die Klasse vor Aufruf bereits Speicher verwaltet hat, wird dieser zuerst freigegeben.
 *
 * \attention
 * Bei Verwendung dieser Funktion wird kein 0-Byte am Ende des Speicherbereichs hinzugefügt.
 * Fall das Objekt als Eingabe für eine C-String-Funktion verwendet werden soll, muss die
 * Anwendung selbst dafür sorgen, dass am Ende ein 0-Byte vorhanden ist.
 */
void ByteArray::useadr(void *adr, size_t size)
{
	::free(ptr);
	ptr=adr;
	s=size;
}


/*!\brief Speicherbereich kopieren
 *
 * \desc
 * Mit dieser Funktion wird der durch \p adr und \p size angegebene Speicherbereich in
 * die ByteArray-Klasse kopiert. Die Klasse allokiert dafür zunächst den erforderlichen
 * Speicher und übernimmt dessen Verwaltung.
 *
 * @param[in] adr
 * @param[in] size
 * \return Bei erfolgreichem Kopieren liefert die Funktion einen Pointer auf den
 * neu allokierten Speicherbereich zurück. Im Fehlerfall wird eine Exception geworfen.
 * \exception OutOfMemoryException Speicher konnte nicht allokiert werden
 * \exception NullPointerException Ein übergebener Parameter war 0
 * \exception Exception Speicherbereich konnte nicht kopiert werden
 * \note
 * Falls die Klasse vor Aufruf bereits Speicher verwaltet hat, wird dieser zuerst freigegeben.
 */
void *ByteArray::copy(const void *adr, size_t size)
{
	::free(ptr);
	s=0;
	ptr=NULL;
	if (adr==NULL || size==0) {
		throw NullPointerException();
	}
	ptr=::malloc(size+1);
	if (!ptr) {
		throw OutOfMemoryException();
	}
	if (memcpy(ptr,adr,size)!=ptr) {
		::free(ptr);
		ptr=NULL;
		s=0;
		throw Exception();
	}
	s=size;
	((char*)ptr)[s]=0;
	return ptr;
}

/*!\brief Speicherbereich aus einem anderen ByteArray-Objekt kopieren
 *
 * \desc
 * Mit dieser Funktion wird der durch \p other referenzierte Speicherbereich
 * in diese Instanz der ByteArray-Klasse kopiert. Die Klasse allokiert dafür zunächst
 * den dafür erforderlichen Speicher und übernimmt dessen Verwaltung.
 *
 * @param[in] other Referenz auf das ByteArray- oder ByteArrayPtr-Objekt, von dem der
 * Speicher kopiert werden soll.
 * \return Bei erfolgreichem Kopieren liefert die Funktion einen Pointer auf den
 * neu allokierten Speicherbereich zurück. Im Fehlerfall wird eine Exception
 * geworfen.
 *
 * \exception OutOfMemoryException Speicher konnte nicht allokiert werden
 * \exception NullPointerException Ein übergebener Parameter war 0
 * \exception Exception Speicherbereich konnte nicht kopiert werden
 *
 */
void *ByteArray::copy(const ByteArrayPtr &other)
{
	return copy(other.ptr, other.s);
}

/*!\brief Speicherbereich anhängen
 *
 * \desc
 * Mit dieser Funktion wird der durch \p adr und \p size angegebene Speicherbereich an die
 * bereits in der Klasse vorhandenen Daten angehangen. Die Klasse allokiert dafür zunächst
 * den erforderlichen Speicher und kopiert dann die Daten.
 *
 * @param[in] adr
 * @param[in] size
 * \return Bei erfolgreichem Kopieren liefert die Funktion einen Pointer auf den
 * neu allokierten Speicherbereich zurück. Im Fehlerfall wird eine Exception geworfen.
 * \exception OutOfMemoryException Speicher konnte nicht allokiert werden
 * \exception NullPointerException Ein übergebener Parameter war 0
 * \exception Exception Speicherbereich konnte nicht kopiert werden
 * \note
 * Falls die Klasse vor Aufruf bereits Speicher verwaltet hat, wird dieser zuerst freigegeben.
 */
void *ByteArray::append(void *adr, size_t size)
{
	if (adr==NULL || size==0) {
		throw NullPointerException();
	}

	if (!ptr) return copy(adr,size);
	size_t newsize=s+size;
	void *p=::realloc(ptr,newsize+1);
	if (!p) throw OutOfMemoryException();
	ptr=p;
	void *target=(char*)ptr+s;
	if (memcpy(target,adr,size)!=target) {
		::free(ptr);
		ptr=NULL;
		s=0;
		throw Exception();
	}
	s=newsize;
	((char*)ptr)[s]=0;
	return ptr;
}

/*!\brief Speicherbereich aus einem ByteArray-Objekt anhängen
 *
 * \desc
 * Mit dieser Funktion wird der durch \p other referenzierte Speicherbereich
 * an die Daten dieser Instanz der ByteArray-Klasse angehangen. Dazu wird
 * Speicher reallokiert und kopiert.
 *
 * @param[in] other Referenz auf das ByteArray- oder ByteArrayPtr-Objekt, von dem der
 * Speicher kopiert werden soll.
 * \return Bei erfolgreichem Kopieren liefert die Funktion einen Pointer auf den
 * neu allokierten Speicherbereich zurück. Im Fehlerfall wird eine Exception
 * geworfen.
 *
 * \exception OutOfMemoryException Speicher konnte nicht allokiert werden
 * \exception NullPointerException Ein übergebener Parameter war 0
 * \exception Exception Speicherbereich konnte nicht kopiert werden
 *
 */
void *ByteArray::append(const ByteArrayPtr &other)
{
	return append(other.ptr, other.s);
}

/*!\brief Speicherbereich davor hängen
 *
 * \desc
 * Mit dieser Funktion wird der durch \p adr und \p size angegebene Speicherbereich vor die
 * bereits in der Klasse vorhandenen Daten gehangen. Die Klasse allokiert dafür zunächst
 * den erforderlichen Speicher und kopiert dann die Daten.
 *
 * @param[in] adr
 * @param[in] size
 * \return Bei erfolgreichem Kopieren liefert die Funktion einen Pointer auf den
 * neu allokierten Speicherbereich zurück. Im Fehlerfall wird eine Exception geworfen.
 * \exception OutOfMemoryException Speicher konnte nicht allokiert werden
 * \exception NullPointerException Ein übergebener Parameter war 0
 * \exception Exception Speicherbereich konnte nicht kopiert werden
 * \note
 * Falls die Klasse vor Aufruf bereits Speicher verwaltet hat, wird dieser zuerst freigegeben.
 */
void *ByteArray::prepend(void *adr, size_t size)
{
	if (adr==NULL || size==0) {
		throw NullPointerException();
	}

	if (!ptr) return copy(adr,size);
	size_t newsize=s+size;
	void *p=::malloc(newsize+1);
	if (!p) throw OutOfMemoryException();
	if (memcpy(p,adr,size)!=p) {
		::free(ptr);
		ptr=NULL;
		s=0;
		throw Exception();
	}
	void *target=(char*)p+size;
	if (memcpy(target,ptr,s)!=target) {
		::free(ptr);
		ptr=NULL;
		s=0;
		throw Exception();
	}
	::free(ptr);
	ptr=p;
	s=newsize;
	((char*)ptr)[s]=0;
	return ptr;
}

/*!\brief Speicherbereich aus einem ByteArray-Objekt davorhängen
 *
 * \desc
 * Mit dieser Funktion wird der durch \p other referenzierte Speicherbereich
 * vor die Daten dieser Instanz der ByteArray-Klasse gehangen. Dazu wird
 * Speicher reallokiert und kopiert.
 *
 * @param[in] other Referenz auf das ByteArray- oder ByteArrayPtr-Objekt, von dem der
 * Speicher kopiert werden soll.
 * \return Bei erfolgreichem Kopieren liefert die Funktion einen Pointer auf den
 * neu allokierten Speicherbereich zurück. Im Fehlerfall wird eine Exception
 * geworfen.
 *
 * \exception OutOfMemoryException Speicher konnte nicht allokiert werden
 * \exception NullPointerException Ein übergebener Parameter war 0
 * \exception Exception Speicherbereich konnte nicht kopiert werden
 *
 */
void *ByteArray::prepend(const ByteArrayPtr &other)
{
	return prepend(other.ptr, other.s);
}


/*!\brief Speicherreferenz von anderem ByteArray-Objekt kopieren
 *
 * \desc
 * Mit diesem Operator wird der Speicherbereich eines anderen ByteArray- oder ByteArrayPtr-Objekts
 * kopiert. Er ist identisch zur Funktion ByteArray::copy.
 *
 * @param[in] other Referenz auf ein anderes ByteArray- oder ByteArrayPtr-Objekt.
 * @return Referenz auf das Objekt
 * \exception OutOfMemoryException Speicher konnte nicht allokiert werden
 * \exception NullPointerException Ein übergebener Parameter war 0
 * \exception Exception Speicherbereich konnte nicht kopiert werden
 *
 */
ByteArray &ByteArray::operator=(const ByteArrayPtr &other)
{
	copy(other.ptr, other.s);
	return *this;
}

/*!\brief Adresse des Speicherblocks auslesen
 *
 * \desc
 * Mit diesem Operator wird die Adresse des Speicherblocks ausgelesen.
 *
 * @return Adresse des Speicherblocks
 * \attention
 * Die Adresse kann NULL sein, wenn kein Speicherblock allokiert ist!
 */
ByteArray::operator const void*() const
{
	return ptr;
}

/*!\brief Adresse des Speicherblocks auslesen
 *
 * \desc
 * Mit diesem Operator wird die Adresse des Speicherblocks ausgelesen.
 *
 * @return Adresse des Speicherblocks
 * \attention
 * Die Adresse kann NULL sein, wenn kein Speicherblock allokiert ist!
 */
ByteArray::operator const unsigned char*() const
{
	return (const unsigned char*)ptr;
}

/*!\brief Adresse des Speicherblocks auslesen
 *
 * \desc
 * Mit diesem Operator wird die Adresse des Speicherblocks ausgelesen.
 *
 * @return Adresse des Speicherblocks
 * \attention
 * Die Adresse kann NULL sein, wenn kein Speicherblock allokiert ist!
 */
ByteArray::operator const char*() const
{
	return (const char*)ptr;
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
 * \exception OutOfBoundsEception Diese Exception wird geworfen, wenn die mit
 * \p pos angegebene Speicherstelle ausseralb des referenzierten Speichers liegt oder
 * kein Speicher referenziert ist.
 */
unsigned char ByteArray::operator[](size_t pos) const
{
	if (ptr!=NULL && pos<s) return ((unsigned char*)ptr)[pos];
	throw OutOfBoundsEception();
}

/*!\brief Speicher allokieren
 *
 * \desc
 * Mit dieser Funktion wird ein neuer Speicherblock mit der Größe von \p size Bytes allokiert
 * und dessen Adresse als Ergebnis zurückgeliefert. Das ByteArray-Objekt übernimmt die Verwaltung
 * des Speicherblocks.
 *
 * @param[in] size Gewünschte Größe des Speicherblocks.
 * \return
 * Pointer auf den Beginn des allokierten Speichers, oder NULL, wenn ein Fehler beim
 * Allokieren aufgetreten ist. Ein entsprechender Fehlercode wird gesetzt.
 */
void *ByteArray::malloc(size_t size)
{
	::free(ptr);
	ptr=::malloc(size+1);
	if (ptr) {
		s=size;
	} else {
		throw OutOfMemoryException();
	}
	((char*)ptr)[s]=0;
	return ptr;
}

/*!\brief Speicher allokieren und mit 0 initialisieren
 *
 * \desc
 * Mit dieser Funktion wird ein neuer Speicherblock mit der Größe von \p size Bytes allokiert,
 * der Inhalt mit 0 initialisiert
 * und dessen Adresse als Ergebnis zurückgeliefert. Das ByteArray-Objekt übernimmt die Verwaltung
 * des Speicherblocks.
 *
 * @param[in] size Gewünschte Größe des Speicherblocks.
 * \return
 * Pointer auf den Beginn des allokierten Speichers, oder NULL, wenn ein Fehler beim
 * Allokieren aufgetreten ist. Ein entsprechender Fehlercode wird gesetzt.
 */
void *ByteArray::calloc(size_t size)
{
	::free(ptr);
	ptr=::calloc(size+1,1);
	if (ptr) {
		s=size;
	} else {
		throw OutOfMemoryException();
	}
	((char*)ptr)[s]=0;
	return ptr;
}

/*!\brief Speicher freigeben
 *
 * \desc
 * Wird der Speicher vom ByteArray-Objekt verwaltet, wird dieser durch Aufruf dieser Funktion
 * wieder freigegeben. Bei nichtverwaltetem Speicher wird lediglich die interne Referenz auf
 * NULL gesetzt, aber der Speicher nicht freigegeben.
 */
void ByteArray::free()
{
	::free(ptr);
	ptr=NULL;
	s=0;
}

void *ByteArray::fromHex(const String &hex)
{
	::free(ptr);
	ptr=NULL;
	s=0;
	if (hex.isEmpty()) {
		throw IllegalArgumentException("Empty string");
	}
	//const unsigned char *p=(const unsigned char *)hex.GetPtr();
	unsigned char *t;
	//size_t bytes=hex.Len();
	size_t chars=hex.size();
	if ((chars&1)==1) {
		throw IllegalArgumentException("uneven number of characters");
	}
	ptr=::malloc((chars>>1)+1);
	if (!ptr) throw OutOfMemoryException();
	t=(unsigned char*)ptr;
	s=chars>>1;
	((char*)ptr)[s]=0;
	unsigned char value;
	for (size_t source=0, target=0;source<chars;source+=2,target++) {
		wchar_t first=hex[source];
		wchar_t second=hex[source+1];
		if (first>='0' && first<='9') value=(first-'0');
		else if (first>='a' && first<='f') value=(first-'a'+10);
		else if (first>='A' && first<='F') value=(first-'A'+10);
		else {
			::free(ptr);
			ptr=NULL;s=0;
			throw IllegalArgumentException("invalid chars in input string");
		}
		value=value<<4;
		if (second>='0' && second<='9') value|=(second-'0');
		else if (second>='a' && second<='f') value|=(second-'a'+10);
		else if (second>='A' && second<='F') value|=(second-'A'+10);
		else {
			::free(ptr);
			ptr=NULL;s=0;
			throw IllegalArgumentException("invalid chars in input string");
		}
		t[target]=value;
	}
	return ptr;
}

}	// EOF namespace ppl7
