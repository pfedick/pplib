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

/*!\class AssocArray
 * \ingroup PPLGroupDataTypes
 * \brief Komplexes mehrdimensionales Array mit Strings als Schlüssel
 *
 * \desc
 * Die Klasse AssocArray dient als Container für beliebige Key-Value-Paare. Ein Schlüssel
 * (Key) besteht aus einem String, der aus beliebigen Zeichen bestehen kann. Ein Value kann
 * veschiedene Datentypen enthalten. Gegenwärtig werden neben Strings auch Arrays, Binaries und
 * Pointer unterstützt.
 * \par
 * Die Schlüssel werden sortiert in einem AVL-Baum verwaltet (siehe CTree, CTreeItem, CArrayItem), so dass auch bei
 * sehr großen Arrays eine schnelle Verarbeitung gewährleistet ist. Gross-/Kleinschreibung wird dabei
 * ignoriert, der Schlüssel "TEST" wäre also identisch mit "test" oder "Test".
 * \par
 * Mehrdimensionale Arrays sind möglich, indem einem Schlüssel als Wert einfach ein anderes Array
 * zugeordnet wird. In einem solchen Array kann jedes Element direkt angesprochen werden, indem man
 * die einzelnen Schlüssel durch Slash (/) getrennt zu einem einzigen Schlüssel zusammenfasst.
 * \par
 * Mehrdimensionale Arrays werden automatisch generiert. Gibt man bei einem leeren Array dem Schlüssel
 * <tt>"ebene1/ebene2/key"</tt> einen Wert, werden automatisch folgende Aktionen ausgeführt:
 * - Es wird ein neues Array generiert und mit dem Schlüssel "ebene1" in das Array eingefügt
 * - In das Array "ebene1" wird ein weiteres neues Array mit dem Schlüssel "ebene2" eingefügt
 * - In das Array "ebene2" wird der eigentliche Wert unter dem Schlüssel "key" eingefügt
 *
 * \par Beispiel:
 * \code
 * ppl7::AssocArray a;
 * // Wert setzen
 * a.set("ebene1/ebene2/key","Ein Wert");
 * // Wert auslesen
 * a.get("ebene1/ebene2/key").toString().printnl();
 * \endcode
 *
 */


/*!\class AssocArray::ArrayKey
 * \brief Datentyp für Schlüssel
 *
 * \desc
 * Das AssocArray verwendet einen eigenen von der String-Klasse abgeleiteten Datentyp. Dieser
 * unterscheidet sich von der String-Klasse nur durch die Vergleichoperatoren. Diese behandelt
 * rein nummerische Schlüssel anders als alphanummerische.
 *
 */

AssocArray::ArrayKey::ArrayKey()
{

}

AssocArray::ArrayKey::ArrayKey(const String &other)
{
	set(other);
}

/*!\brief Generische Vergleichfunktion
 *
 * \desc
 * Diese Funktion vergleicht den eigenen Wert mit dem aus \p str. Sind Beide Strings nummerisch,
 * wird ein nummerischer Vergleich durchgeführt, andernfalls ein Stringvergleich. Die Funktion wird
 * von den Vergleichoperatoren aufgerufen.
 *
 * @param str Vergleichswert
 * @return Liefert folgende Werte zurück:
 * - 0: Beide Werte sind identisch
 * - -1: Der angegebene Wert \p str ist kleiner als der eigene
 * - 1: Der angegebene Wert \p str ist größer als der eigene
 */
int AssocArray::ArrayKey::compare(const ArrayKey &str) const
{
	if (isNumeric()==true && str.isNumeric()==true) {
		pplint64 v1=toInt64();
		pplint64 v2=str.toInt64();
		if (v2>v1) return 1;
		if (v2<v1) return -1;
		return 0;
	}
	int cmp=strcasecmp(str);
	if (cmp==0) return 0;
	if (cmp<0) return 1;
	return -1;
}

AssocArray::ArrayKey& AssocArray::ArrayKey::operator=(const String &str)
{
	set(str);
	return *this;
}

bool AssocArray::ArrayKey::operator<(const ArrayKey &str) const
{
	int c=compare(str);
	if (c<0) return true;
	return false;
}
bool AssocArray::ArrayKey::operator<=(const ArrayKey &str) const
{
	int c=compare(str);
	if (c<=0) return true;
	return false;
}

bool AssocArray::ArrayKey::operator==(const ArrayKey &str) const
{
	int c=compare(str);
	if (c==0) return true;
	return false;
}

bool AssocArray::ArrayKey::operator!=(const ArrayKey &str) const
{
	int c=compare(str);
	if (c!=0) return true;
	return false;
}

bool AssocArray::ArrayKey::operator>=(const ArrayKey &str) const
{
	int c=compare(str);
	if (c>=0) return true;
	return false;
}

bool AssocArray::ArrayKey::operator>(const ArrayKey &str) const
{
	int c=compare(str);
	if (c>0) return true;
	return false;
}


/*!\brief Konstruktor des Assoziativen Arrays
 *
 * \desc
 * Initialisiert die Instanz mit 0 und initialisiert den AVL-Baum.
 */
AssocArray::AssocArray()
{
	type=Variant::ASSOCARRAY;
	maxint=0;
	num=0;
}

/*!\brief Destruktor der Klasse
 *
 * \desc
 * Der Destruktor ruft die Funktion AssocArray::clear auf, um alle vorhandenen Elemente zu
 * löschen.
 */
AssocArray::~AssocArray()
{
	clear();
}

/*!\brief Inhalt des Arrays löschen
 *
 * \desc
 * Mit dieser Funktion wird der komplette Inhalt des Arrays gelöscht. Dabei der Destruktor für jedes
 * vorhandene Element aufgerufen, der wiederum sicherstellt, dass die darin enthaltenen Daten
 * ordnungsgemäß gelöscht werden.
 *
 */
void AssocArray::clear()
{
	Tree.clear();
	num=0;
	maxint=0;
}


/*!\brief Interne Funktion zum Suchen eines Elements
 *
 * \desc
 * Diese Funktion zerlegt den angegebenen Schlüssel (\p key) in seine einzelnen Elemente.
 * Als Trennzeichen wird wie bei einer Unix-Pfadangabe der Slash (/) verwendet. Die Funktion
 * sucht zunächst nach dem erste Element des Schlüssels im eigenen Baum. Ist dies vorhanden
 * und handelt es sich bei dessen Datentyp wieder um ein AssocArray, wird deren
 * findInternal-Funktion mit dem restlichen Schlüssel rekursiv aufgerufen. Dies geschieht
 * solange, bis das letzte Element des Keys gefunden wurde.
 *
 * \param[in] key String mit dem gesuchten Schlüssel
 * \return Konnte der Schlüssel gefunden werden, wir der Pointer auf das Element (Variant)
 * zurückgegeben. Wurde der Schlüssel nicht gefunden, wird eine Exception geworfen
 * \exception InvalidKeyException: Wird geworfen, wenn der Schlüssel ungültig oder leer ist
 * \exception KeyNotFoundException: Wird geworfen, wenn der Schlüssel nicht vorhanden ist
 * \note
 * Die Funktion wird von allen Get...- und Concat-Funktionen verwendet.
 */
Variant *AssocArray::findInternal(const ArrayKey &key) const
{
	Array tok(key,L"/",0,true);
	if (tok.count()==0) throw InvalidKeyException(key);
	ArrayKey firstkey=tok.shift();
	ArrayKey rest=tok.implode("/");
	Variant *p;
	try {
		p=Tree.find(firstkey);
	} catch (ItemNotFoundException) {
		throw KeyNotFoundException();
	}
	// Ist noch was im Pfad rest?
	if (tok.count()>1) {			// Ja, koennen wir iterieren?
		if (p->isAssocArray()) {
			return ((AssocArray*)p)->findInternal(rest);
		} else {
			throw KeyNotFoundException();
		}
	}
	return p;
}


/*!\brief Interne Funktion, die ein Element im Baum sucht oder anlegt
 *
 * \desc
 * Diese Funktion durchsucht den Baum nach dem gewünschten Element. Ist es vorhanden, wird dessen Pointer
 * zurückgeliefert, wenn nicht, wird es angelegt, jedoch ohne Daten. Bei verschachtelten Schlüsseln wird
 * die Funktion rekursiv aufgerufen, bis das letzte Element erreicht ist. Die Funktion wird intern von allen
 * Funktionen verwendet, die Daten in das Array speichern.
 *
 * \param[in] key Pointer auf den Namen des Schlüssels
 * \return Bei Erfolg liefert die Funktion einen Pointer auf das gewünschte Element zurück.
 * Im Fehlerfall wird eine Exception geworfen.
 *
 * \exception InvalidKeyException: Wird geworfen, wenn der Schlüssel ungültig oder leer ist
 * \exception std::bad_alloc: Kein Speicher mehr frei
 *
 * \remarks Bei der Angabe eines verschachtelten Schlüssels kann es vorkommen, dass bereits vorhandene
 * Elemente überschrieben werden. Beispiel:
 *
 * Das Element <tt>ebene1/schlüssel1</tt> ist im Baum bereits vorhanden und beinhaltet einen String. Nun wird
 * das neue Element <tt>ebene1/schlüssel1/unterschlüssel1</tt> angelegt. Da Schlüssel eindeutig sein müssen,
 * wird der String <tt>ebene1/schlüssel1</tt> gelöscht und in ein Array umgewandelt.
 */
Variant *AssocArray::createTree(const ArrayKey &key)
{
	Array tok(key,L"/",0,true);
	if (tok.count()==0) throw InvalidKeyException(key);
	String firstkey=tok.shift();
	ArrayKey rest=tok.implode("/");
	if (firstkey=="[]") {
		firstkey.setf("%llu",maxint);
		maxint++;
	}
	// Begint Firstkey mit einer Zahl?
	if (firstkey.isNumeric()) {
		ppluint64 keyint=firstkey.toInt64();
		if (keyint>=maxint) maxint=keyint+1;
	}

	Variant *p;
	try {
		p=Tree.find(firstkey);
	} catch (ItemNotFoundException) {
		p=NULL;
	}
	if (p) {
		// Ist noch was im Pfad rest?
		if (tok.count()>0) {          // Ja, koennen wir iterieren?
			if (p->isAssocArray()==false) {
				delete (p);		// Nein, wir loeschen daher diesen Zweig und machen ein Array draus
				p=new AssocArray;
			}
			p=((AssocArray*)p)->createTree(rest);
		}
		// Nein, wir haben die Zielposition gefunden
		return p;
	}

	// Key ist nicht in diesem Array, wir legen ihn an

	// Ist noch was im Pfad rest?
	if (tok.count()>0) {          // Ja, wir erstellen ein Array und iterieren
		p=new AssocArray;
		Tree.add(firstkey,p);
		p=((AssocArray*)p)->createTree(rest);
	} else {
		p=new Variant;
		Tree.add(firstkey,p);
	}
	num++;
	return p;
}

/*!\brief Anzahl Schlüssel zählen
 *
 * \desc
 * Diese Funktion liefert die Anzahl Schlüssel auf dieser Ebene des Array zurück.
 *
 * \param[in] recursive Falls recursive auf true gesetzt wird die Funktion rekusriv für jeden
 * Schlüssel aufgerufen, dessen Wert ebenfalls ein Array ist.
 *
 * \returns Anzahl Schlüssel
 */
size_t AssocArray::count(bool recursive) const
{
	if (!recursive) return num;
	ppl7::AVLTree<ArrayKey, Variant *>::Iterator it;
	Variant *p;
	Tree.reset(it);
	size_t c=num;
	while (Tree.getNext(it)) {
		p=it.value();
		if (p->isAssocArray()) c+=((AssocArray*)p)->count(recursive);
	}
	return c;
}

/*!\brief Anzahl Schlüssel für ein bestimmtes Element zählen
 *
 * \desc
 * Diese Funktion liefert die Anzahl Schlüssel zurück, die in dem angegebenen Key enthalten sind.
 *
 * \param[in] key Schlüssel-Name eines Assoziativen Arrays innerhalb dieses Arrays
 * \param[in] recursive Falls recursive auf true gesetzt wird die Funktion rekusriv für jeden
 * Schlüssel aufgerufen, dessen Wert ebenfalls ein Array ist.
 *
 * \returns Anzahl Schlüssel
 */
size_t AssocArray::count(const String &key, bool recursive) const
{
	Variant *p;
	try {
		p=findInternal(key);
	} catch (KeyNotFoundException) {
		return 0;
	}
	if (p->isAssocArray()) return ((AssocArray*)p)->count(recursive);
	return 1;
}

/*!\brief Inhalt des Arrays ausgeben
 *
 * \desc
 * Diese Funktion dient Debugging-Zwecken. Der Aufruf bewirkt, dass der Inhalt des kompletten Arrays auf
 * STDOUT ausgegeben wird.
 *
 * \param[in] prefix Optionaler Text, der bei der Ausgabe jedem Element vorangestellt wird
 *
 * \par Beispiel:
 * \code
ppl7::AssocArray a;
ppl7::Binary bin;
bin.load("main.cpp");

a.set("key1","value1");
a.set("array1/unterkey1","value2");
a.set("array1/unterkey2","value3");
a.set("array1/noch ein array/unterkey1","value4");
a.set("array1/unterkey2","value5");
a.set("key2","value6");
a.set("dateien/main.cpp",bin);
a.set("array2/unterkey1","value7");
a.set("array2/unterkey2","value8");
a.set("array2/unterkey1","value9");
a.list("prefix");
\endcode
	Ausgabe:
\code
prefix/array1/noch ein array/unterkey1=value4
prefix/array1/unterkey1=value2
prefix/array1/unterkey2=value5
prefix/array2/unterkey1=value9
prefix/array2/unterkey2=value8
prefix/dateien/main.cpp=Binary, 806 Bytes
prefix/key1=value1
prefix/key2=value6
\endcode
 *
 * \remarks Die Funktion gibt nur "lesbare" Element aus. Enthält das Array Pointer oder Binaries, wird das Element zwar
 * ausgegeben, jedoch werden als Wert nur Meta-Informationen ausgegeben (Datentyp, Pointer, Größe).
 */
void AssocArray::list(const String &prefix) const
{
	String key;
	String pre;
	if (prefix) key=prefix+L"/";
	ppl7::AVLTree<ArrayKey, Variant *>::Iterator it;
	Tree.reset(it);
	Variant *p;
	while ((Tree.getNext(it))) {
		p=it.value();
		if (p->isString()) {
			PrintDebug("%ls%ls=%ls\n",(const wchar_t*)key,(const wchar_t*)it.key(),(const wchar_t*)((String*)p)->getPtr());
		} else if (p->isByteArray()) {
			PrintDebug("%ls%ls=ByteArray, %zu Bytes\n",(const wchar_t*)key,(const wchar_t*)it.key(),((ByteArray*)p)->size());
		} else if (p->isByteArrayPtr()) {
			PrintDebug("%ls%ls=ByteArrayPtr, %zu Bytes\n",(const wchar_t*)key,(const wchar_t*)it.key(),((ByteArrayPtr*)p)->size());
		} else if (p->isAssocArray()) {
			pre.setf("%ls%ls",(const wchar_t*)key,(const wchar_t*)it.key());
			((AssocArray*)p)->list(pre);
			/*
		} else if (p->type==datatype::POINTER) {
			PrintDebug("%s%s=POINTER %llu (0x%llx)\n",(const char*)key,(const char*)p->key.GetPtr(),(ppluint64)(size_t)(p->value), (ppluint64)(size_t)(p->value));
			*/
		} else if (p->isDateTime()) {
			PrintDebug("%ls%ls=DateTime %s\n",(const wchar_t*)key,(const wchar_t*)it.key(), (const wchar_t*) ((DateTime*)p)->getISO8601withMsec());
		}
	}
}

void AssocArray::set(const String &key, const String &value)
{
	Variant *p=createTree(key);
	delete p;
	p=new String(value);
}

void AssocArray::set(const String &key, const String &value, size_t size)
{
	Variant *p=createTree(key);
	delete p;
	p=new String;
	((String*)p)->set(value,size);
}

void AssocArray::set(const String &key, const DateTime &value)
{
	Variant *p=createTree(key);
	delete p;
	p=new DateTime(value);
}

void AssocArray::set(const String &key, const ByteArray &value)
{
	Variant *p=createTree(key);
	delete p;
	p=new ByteArray(value);
}

void AssocArray::set(const String &key, const ByteArrayPtr &value)
{
	Variant *p=createTree(key);
	delete p;
	p=new ByteArrayPtr(value);
}

void AssocArray::set(const String &key, const Array &value)
{
	Variant *p=createTree(key);
	delete p;
	p=new Array(value);
}

void AssocArray::set(const String &key, const AssocArray &value)
{
	/*
	Variant *p=createTree(key);
	delete p;
	p=new AssocArray(value);
	*/
}

/*!\brief Formatierten String hinzufügen
 *
 * \desc
 * Diese Funktion fügt den Inhalt eines formatierten Strings dem Array hinzu.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] fmt Pointer auf den Format-String des Wertes
 * \param[in] ... Beliebig viele Parameter, die vom Formatstring verwendet werden
 */
void AssocArray::setf(const String &key, const char *fmt, ...)
{
	String value;
	va_list args;
	va_start(args, fmt);
	value.vasprintf(fmt,args);
	va_end(args);
	Variant *p=createTree(key);
	delete p;
	p=new String(value);
}

const String& AssocArray::getString(const String &key) const
{
	const Variant *ptr=findInternal(key);
	if (ptr->isString()) return ptr->toString();
	throw TypeConversionException();
}

const Variant& AssocArray::get(const String &key) const
{
	const Variant *ptr=findInternal(key);
	return *ptr;
}


} // EOF namespace ppl7
