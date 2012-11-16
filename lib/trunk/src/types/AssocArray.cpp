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
 * Copyright (c) 2012, Patrick Fedick <patrick@pfp.de>
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
#include <ostream>

#include "ppl7.h"


namespace ppl7 {

/*!\class AssocArray
 * \ingroup PPLGroupDataTypes
 * \brief Komplexes mehrdimensionales %Array mit Strings als Schlüssel
 *
 * \desc
 * Die Klasse AssocArray dient als Container für beliebige Key-Value-Paare. Ein Schlüssel
 * (Key) besteht aus einem String, der aus beliebigen Zeichen bestehen kann. Ein Value kann
 * veschiedene Datentypen enthalten. Gegenwärtig werden folgende Datentypen unterstützt:
 * - String
 * - Array
 * - ByteArray
 * - ByteArrayPtr
 * - AssocArray
 * - DateTime
 * \par
 * Die Schlüssel werden sortiert in einem AVL-Baum verwaltet (siehe AVLTree), so dass auch bei
 * sehr großen Arrays eine schnelle Verarbeitung gewährleistet ist. Gross-/Kleinschreibung wird
 * ignoriert, der Schlüssel "TEST" wäre also identisch mit "test" oder "Test".
 * \par
 * Mehrdimensionale Arrays sind möglich, indem einem Schlüssel als Wert einfach ein anderes Array
 * zugeordnet wird. In einem solchen Array kann jedes Element direkt angesprochen werden, indem man
 * die einzelnen Schlüssel durch Slash (/) getrennt zu einem einzigen Schlüssel zusammenfasst.
 * \par
 * Mehrdimensionale Arrays werden automatisch generiert. Gibt man bei einem leeren %Array dem Schlüssel
 * <tt>"ebene1/ebene2/key"</tt> einen Wert, werden automatisch folgende Aktionen ausgeführt:
 * - Es wird ein neues AssocArray generiert und mit dem Schlüssel "ebene1" in das %Array eingefügt
 * - In das %Array "ebene1" wird ein weiteres neues %Array mit dem Schlüssel "ebene2" eingefügt
 * - In das %Array "ebene2" wird der eigentliche Wert unter dem Schlüssel "key" eingefügt
 *
 * \par Beispiel:
 * Einen Wert setzen und wieder auslesen:
 * \code
 * ppl7::AssocArray a;
 * // Wert setzen
 * a.set("ebene1/ebene2/key","Ein Wert");
 * // Wert auslesen
 * a.get("ebene1/ebene2/key").toString().printnl();
 * \endcode
 * \par
 * Durch ein AssocArray durchiterieren:
 * \code
void IterateArray(const ppl7::AssocArray &a)
{
	ppl7::AssocArray::Iterator it;
	a.reset(it);
	try {
		while (1) {
			const ppl7::Variant &var=a.getNext(it);
			if (var.isString()) {
				cout << "Key: " << it.key() << ", Value: " << var.toString() << endl;
			}
		}
	} catch (OutOfBoundsEception &) {
		cout << "Ende erreicht" << endl;
	}
}
 * \endcode
 * \par
 * Wenn von vorneherein bekannt ist, dass im Array nur Strings vorhanden sind, kann man auch noch auf diese
 * Weise durchiterieren:
 * \code
void IterateArray(const ppl7::AssocArray &a)
{
	ppl7::AssocArray::Iterator it;
	ppl7::String Key, Value;
	a.reset(it);
	while (a.GetNext(it,Key,Value)) {
		cout << "Key: " << Key << ", Value: " << Value << endl;
	}
}
 * \endcode
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
	if (cmp<0) return -1;
	return 1;
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


AssocArray::ValueNode::ValueNode()
{
	value=NULL;
}

AssocArray::ValueNode::ValueNode(const ValueNode &other)
{
	value=other.value;
}

AssocArray::ValueNode::~ValueNode()
{
	if (value!=NULL) delete value;
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

/*!\brief Copy-Konstruktor des Assoziativen Arrays
 *
 * \desc
 * Macht eine Kopie des Assoziativen Arrays \p other.
 *
 * \param[in] other Referenz auf zu kopierendes Assoziatives Array
 *
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
AssocArray::AssocArray(const AssocArray &other)
{
	type=Variant::ASSOCARRAY;
	maxint=0;
	num=0;
	add(other);
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
AssocArray::ValueNode *AssocArray::findInternal(const ArrayKey &key) const
{
	//printf ("AssocArray::findInternal (key=%ls)\n",(const wchar_t*)key);
	Array tok(key,"/",0,true);
	if (tok.count()==0) throw InvalidKeyException(key);
	ArrayKey firstkey=tok.shift();
	ArrayKey rest=tok.implode("/");
	ValueNode *p;
	try {
		ValueNode &node=Tree.find(firstkey);
		p=&node;

	} catch (ItemNotFoundException &) {
		throw KeyNotFoundException();
	}
	// Ist noch was im Pfad rest?
	if (tok.count()>0) {			// Ja, koennen wir iterieren?
		if (p->value->isAssocArray()) {
			return ((AssocArray*)p->value)->findInternal(rest);
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
 * \param[in] var Pointer auf die Daten, die unter diesem Schlüssel abgelegt werden sollen
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
AssocArray::ValueNode *AssocArray::createTree(const ArrayKey &key, Variant *var)
{
	Array tok(key,"/",0,true);
	if (tok.count()==0) throw InvalidKeyException(key);
	String firstkey=tok.shift();
	ArrayKey rest=tok.implode("/");
	//printf ("firstkey=%ls, rest=%ls\n",(const wchar_t *)firstkey,(const wchar_t *)rest);
	if (firstkey=="[]") {
		firstkey.setf("%llu",maxint);
		maxint++;
	}
	// Begint Firstkey mit einer Zahl?
	if (firstkey.isNumeric()) {
		ppluint64 keyint=firstkey.toInt64();
		if (keyint>=maxint) maxint=keyint+1;
	}

	ValueNode *p=NULL;
	try {
		ValueNode &node=Tree.find(firstkey);
		p=&node;
	} catch (ItemNotFoundException &) {
		//printf ("Item not found\n");
		p=NULL;
	}
	if (p) {
		// Ist noch was im Pfad rest?
		if (tok.count()>0) {          // Ja, koennen wir iterieren?
			if (p->value->isAssocArray()==false) {
				delete (p->value);		// Nein, wir loeschen daher diesen Zweig und machen ein Array draus
				p->value=new AssocArray;
			}
			return ((AssocArray*)p->value)->createTree(rest,var);
		}
		// Nein, wir haben die Zielposition gefunden
		delete p->value;
		p->value=var;
		return p;
	}

	// Key ist nicht in diesem Array, wir legen ihn an
	ValueNode newnode;
	// Ist noch was im Pfad rest?
	if (tok.count()>0) {          // Ja, wir erstellen ein Array und iterieren
		//printf ("Iteration\n");
		newnode.value=NULL;
		ValueNode &node=Tree.add(firstkey,newnode);
		node.value=new AssocArray;
		p=((AssocArray*)node.value)->createTree(rest,var);
	} else {
		//printf ("Neuen Variant anlegen\n");
		newnode.value=NULL;
		ValueNode &node=Tree.add(firstkey,newnode);
		node.value=var;
		//printf ("AssocArray::createTree, node adr=%tu\n",(ptrdiff_t)&node);
		p=(ValueNode*)&node;
		//printf ("AssocArray::createTree, p=%tu\n",(ptrdiff_t)p);
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
	ppl7::AVLTree<ArrayKey, ValueNode>::Iterator it;
	Variant *p;
	Tree.reset(it);
	size_t c=num;
	while (Tree.getNext(it)) {
		p=it.value().value;
		if (p->isAssocArray()) c+=((AssocArray*)p)->count(recursive);
	}
	return c;
}

/*!\brief Anzahl Elemente
 *
 * \desc
 * Diese Funktion liefert die Anzahl Elemente auf dieser Ebene des Arrays zurück.
 *
 * \returns Anzahl Elemente
 */
size_t AssocArray::size() const
{
	return num;
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
	ValueNode *p;
	try {
		p=findInternal(key);
	} catch (KeyNotFoundException &) {
		return 0;
	}
	if (p->value->isAssocArray()) return ((AssocArray*)p->value)->count(recursive);
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
	if (prefix.notEmpty()) key=prefix+"/";
	ppl7::AVLTree<ArrayKey, ValueNode>::Iterator it;
	Tree.reset(it);
	Variant *p;

	while ((Tree.getNext(it))) {
		//printf ("AssocArray::list(%ls)\n",(const wchar_t*)prefix);
		p=it.value().value;
		if (p->isString()) {
			PrintDebug("%s%s=%s\n",(const char*)key,(const char*)it.key(),(const char*)((String*)p)->getPtr());
		} else if (p->isByteArray()) {
			PrintDebug("%s%s=ByteArray, %zu Bytes\n",(const char*)key,(const char*)it.key(),((ByteArray*)p)->size());
		} else if (p->isByteArrayPtr()) {
			PrintDebug("%s%s=ByteArrayPtr, %zu Bytes\n",(const char*)key,(const char*)it.key(),((ByteArrayPtr*)p)->size());
		} else if (p->isAssocArray()) {
			pre.setf("%s%s",(const char*)key,(const char*)it.key());
			((AssocArray*)p)->list(pre);
		} else if (p->isPointer()) {
			PrintDebug("%s%s=Pointer, %tu\n",(const char*)key,(const char*)it.key(),(std::ptrdiff_t)((Pointer*)p)->ptr());
		} else if (p->isArray()) {
			const Array &a=(const Array &)*p;
			for (size_t i=0;i<a.size();i++) {
				PrintDebug("%s%s/Array(%zu)=%s\n",(const char*)key,(const char*)it.key(),i,(const char*)a[i]);
			}
		} else if (p->isDateTime()) {
			PrintDebug("%s%s=DateTime %s\n",(const char*)key,(const char*)it.key(), (const char*) ((DateTime*)p)->getISO8601withMsec());
		} else {
			PrintDebug("%s%s=UnknownDataType Id=%i\n",(const char*)key,(const char*)it.key(),p->dataType());
		}
	}
}

/*!\brief Speicher reservieren
 *
 * \desc
 * Mit dieser Funktion kann vorab Speicher für eine bestimmte Anzahl Elemente reserviert werden.
 * Der Aufruf dieser Funktion ist immer dann sinnvoll, wenn schon vorher bekannt ist, wieviele
 * Elemente benötigt werden, insbesondere, wenn sehr viele Elemente benötigt werden.
 *
 * @param num Anzahl Elemente, für die Speicher vorab allokiert werden soll
 *
 * \note Falls schon Speicher allokiert wurde, wird die Anzahl der bereits allokierten Elemente
 * mit \p num verrechnet und nur die Differenz zusätzlich reserviert.
 */
void AssocArray::reserve(size_t num)
{
	Tree.reserve(num);
}

/*!\brief Aktuelle Kapazität des %AssocArrays
 *
 * \desc
 * Gibt zurück, wieviele Elemente diese Ebene des Assoziativen Arrays verwalten kann,
 * ohne dass neuer Speicher allokiert werden muss.
 *
 * \return Anzahl Elemente
 */
size_t AssocArray::capacity() const
{
	return Tree.capacity();
}

/*!\brief %String hinzufügen
 *
 * \desc
 * Diese Funktion fügt den Inhalt eines Strings dem Array hinzu.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] value Wert
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
void AssocArray::set(const String &key, const String &value)
{
	String *var=new String(value);
	try {
		//std::cout << L"key="<<key << ", value="<<value << std::endl;
		createTree(key,var);
	} catch (...) {
		delete var;
		throw;
	}
}

/*!\brief %String mit bestimmter Länge hinzufügen
 *
 * \desc
 * Diese Funktion fügt die ersten \p size Zeichen des Strings \p value unter dem
 * Schlüssel \p key in das Assoziative Array ein.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] value Wert
 * \param[in] size Anzahl Zeichen, die aus dem String \p value übernommen werden sollen
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
void AssocArray::set(const String &key, const String &value, size_t size)
{
	String *var=new String(value);
	var->set(value,size);
	try {
		createTree(key,var);
	} catch (...) {
		delete var;
		throw;
	}
}

/*!\brief %DateTime hinzufügen
 *
 * \desc
 * Diese Funktion fügt den in \p value angegebenen Zeitstempel unter dem
 * Schlüssel \p key in das Assoziative Array ein.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] value Zeitstempel
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
void AssocArray::set(const String &key, const DateTime &value)
{
	DateTime *var=new DateTime(value);
	try {
		createTree(key,var);
	} catch (...) {
		delete var;
		throw;
	}
}

/*!\brief %ByteArray hinzufügen
 *
 * \desc
 * Diese Funktion fügt den Inhalt des ByteArrays \p value
 * unter dem Schlüssel \p key in das Assoziative Array ein.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] value Daten
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
void AssocArray::set(const String &key, const ByteArray &value)
{
	ByteArray *var=new ByteArray(value);
	try {
		createTree(key,var);
	} catch (...) {
		delete var;
		throw;
	}
}

/*!\brief %ByteArrayPtr hinzufügen
 *
 * \desc
 * Diese Funktion fügt den Inhalt des ByteArrayPtrs \p value
 * unter dem Schlüssel \p key in das Assoziative Array ein.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] value Daten
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
void AssocArray::set(const String &key, const ByteArrayPtr &value)
{
	ByteArrayPtr *var=new ByteArrayPtr(value);
	try {
		createTree(key,var);
	} catch (...) {
		delete var;
		throw;
	}
}

/*!\brief %Array hinzufügen
 *
 * \desc
 * Diese Funktion fügt den Inhalt des Arrays \p value
 * unter dem Schlüssel \p key in das Assoziative Array ein.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] value Daten
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
void AssocArray::set(const String &key, const Array &value)
{
	Array *var=new Array(value);
	try {
		createTree(key,var);
	} catch (...) {
		delete var;
		throw;
	}
}

/*!\brief %AssocArray hinzufügen
 *
 * \desc
 * Diese Funktion fügt den Inhalt des AssocArrays \p value
 * unter dem Schlüssel \p key in das Assoziative Array ein.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] value Daten
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
void AssocArray::set(const String &key, const AssocArray &value)
{
	AssocArray *var=new AssocArray(value);
	try {
		createTree(key,var);
	} catch (...) {
		delete var;
		throw;
	}
}

/*!\brief %Pointer hinzufügen
 *
 * \desc
 * Diese Funktion fügt den Inhalt des Pointers \p value
 * unter dem Schlüssel \p key in das Assoziative Array ein.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] value Daten
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
void AssocArray::set(const String &key, const Pointer &value)
{
	Pointer *var=new Pointer(value);
	try {
		createTree(key,var);
	} catch (...) {
		delete var;
		throw;
	}
}

/*!\brief %Variant hinzufügen
 *
 * \desc
 * Diese Funktion fügt den Inhalt des Variants \p value
 * unter dem Schlüssel \p key in das Assoziative Array ein.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] value Daten
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 * \exception TypeConversionException: Der Datentyp des Variants wurde nicht
 * erkannt oder wird nicht unterstützt.
 */
void AssocArray::set(const String &key, const Variant &value)
{
	switch (value.dataType()) {
		case Variant::STRING:
			set(key,static_cast<const String&>(value));
			return;
		case Variant::ASSOCARRAY:
			set(key,static_cast<const AssocArray&>(value));
			return;
		case Variant::BYTEARRAY:
			set(key,static_cast<const ByteArray&>(value));
			return;
		case Variant::ARRAY:
			set(key,static_cast<const Array&>(value));
			return;
		case Variant::DATETIME:
			set(key,static_cast<const DateTime&>(value));
			return;
		case Variant::BYTEARRAYPTR:
			set(key,static_cast<const ByteArrayPtr&>(value));
			return;
		case Variant::POINTER:
			set(key,static_cast<const Pointer&>(value));
			return;
	}
	throw TypeConversionException();
}

/*!\brief Formatierten String hinzufügen
 *
 * \desc
 * Diese Funktion fügt den Inhalt eines formatierten Strings dem Array hinzu.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] fmt Pointer auf den Format-String des Wertes
 * \param[in] ... Beliebig viele Parameter, die vom Formatstring verwendet werden
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
void AssocArray::setf(const String &key, const char *fmt, ...)
{
	String *var=new String;
	va_list args;
	va_start(args, fmt);
	var->vasprintf(fmt,args);
	va_end(args);
	try {
		createTree(key,var);
	} catch (...) {
		delete var;
		throw;
	}
}

/*!\brief %String verlängern
 *
 * \desc
 * Diese Funktion fügt den Inhalt des Strings \p value an den bereits vorhandenen
 * Wert des Schlüssels \p key an. Falls der optionale Parameter \p concat einen Wert
 * enthält, wird dieser als Trennung zwischen bestehendem und neuem String verwendet.
 * War der Schlüssel bisher nicht vorhanden, wird ein neuer angelegt.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] value Wert
 * \param[in] concat Trennzeichen (Optional, Default=keins)
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 * \exception TypeConversionException: Schlüssel ist bereits vorhanden, enthält aber keinen String
 */
void AssocArray::append(const String &key, const String &value, const String &concat)
{
	ValueNode *node=NULL;
	try {
		node=findInternal(key);
	} catch (KeyNotFoundException &) {
		set(key,value);
		return;
	}
	/*
	if (node->value==NULL) {
		set(key,value);
		return;
	}
	*/
	if (node->value->isString()==false) {
		throw TypeConversionException();
	}
	if (concat.notEmpty()) ((String*)node->value)->append(concat);
	((String*)node->value)->append(value);
}

/*!\brief %String mit Formatiertem String verlängern
 *
 * \desc
 * Diese Funktion erstellt zuerst einen neuen String anhand des Formatstrings
 * \p fmt und der zusätzlichen optionalen Parameter. Dieser wird an den bereits vorhandenen
 * Wert des Schlüssels \p key angehangen. Falls der optionale Parameter \p concat einen Wert
 * enthält, wird dieser als Trennung zwischen bestehendem und neuem String verwendet.
 * War der Schlüssel bisher nicht vorhanden, wird ein neuer angelegt.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] concat Trennzeichen (Optional, Default=keins)
 * \param[in] fmt Formatstring
 * \param[in] ... Optionale Parameter
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 * \exception TypeConversionException: Schlüssel ist bereits vorhanden, enthält aber keinen String
 */
void AssocArray::appendf(const String &key, const String &concat, const char *fmt, ...)
{
	String var;
	va_list args;
	va_start(args, fmt);
	var.vasprintf(fmt,args);
	va_end(args);

	ValueNode *node=NULL;
	try {
		node=findInternal(key);
	} catch (KeyNotFoundException &) {
		set(key,var);
	}
	if (node->value->isString()==false) {
		throw TypeConversionException();
	}
	if (concat.notEmpty()) ((String*)node->value)->append(concat);
	((String*)node->value)->append(var);
}

/*!\brief %AssocArray kopieren
 *
 * \desc
 * Mit dieser Funktion wird der komplette Inhalt des Assoziativen Arrays \p other
 * in dieses hineinkopiert. Das Array wird vorher nicht gelöscht, so dass vorhandene
 * Schlüssel erhalten bleiben. Gibt es in \p other jedoch gleichnamige Schlüssel,
 * werden die bisherigen Werte überschrieben.
 *
 * \param[in] a Das zu kopierende AssocArray
 *
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
void AssocArray::add(const AssocArray &other)
{
	Tree.reserve(num+other.num);	// Speicher vorab reservieren
	ppl7::AVLTree<ArrayKey, ValueNode>::Iterator it;
	other.Tree.reset(it);
	Variant *p;
	while ((other.Tree.getNext(it))) {
		p=it.value().value;
		set(it.key(),*p);
	}
}




/*!\brief Schlüssel auslesen
 *
 * \desc
 * Diese Funktion liefert den Wert des Schlüssels \p key als Variant zurück. Dieser kann
 * von der aufrufenden Anwendung in den jeweiligen Datentyp umgewandelt werden.
 *
 * @param key Name des Schlüssels
 * @return Referenz auf einen Variant mit dem Wert des Schlüssels
 * \exception InvalidKeyException: Ungültiger Schlüssel
 * \exception KeyNotFoundException: Schlüssel wurde nicht gefunden
 * \example
 * Der Variant kann z.B. folgendermaßen in einen String umgewandelt werden:
 * \code
ppl7::String &str=a.get(L"key1").toString();
\endcode
 */
Variant& AssocArray::get(const String &key) const
{
	ValueNode *node=findInternal(key);
	return *node->value;
}

/*!\brief Schlüssel vorhanden
 *
 * \desc
 * Diese Funktion prüft, ob der Schlüssels \p key im Assoziativen Array enthalten ist.
 *
 * @param key Name des Schlüssels
 * @return Liefert \c true zurück, wenn der Schlüssel vorhanden ist, sonst \c false
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
bool AssocArray::exists(const String &key) const
{
	try {
		findInternal(key);
	} catch (KeyNotFoundException &) {
		return false;
	}
	return true;
}

/*!\brief String auslesen
 *
 * \desc
 * Diese Funktion liefert den Wert des Schlüssels \p key als String zurück, sofern
 * der Schlüssel auch tatsächlich einen String enthält.
 *
 * @param key Name des Schlüssels
 * @return Referenz auf einen String mit dem Wert des Schlüssels
 * \exception InvalidKeyException: Ungültiger Schlüssel
 * \exception KeyNotFoundException: Schlüssel wurde nicht gefunden
 */
String& AssocArray::getString(const String &key) const
{
	ValueNode *node=findInternal(key);
	Variant *p=node->value;
	if (p->isString()) return p->toString();
	//printf ("String mit key %s ist vom Typ: %i\n",(const char*)key,p->dataType());
	throw TypeConversionException();
}


/*!\brief Einzelnen Schlüssel löschen
 *
 * \desc
 * Mit dieser Funktion wird ein einzelner Schlüssel aus dem Array gelöscht.
 *
 * \param[in] key Pointer auf den Namen des zu löschenden Schlüssels
 *
 * \returns Bei Erfolg liefert die die Funktion true (1) zurück, im Fehlerfall false (0).*
 */
void AssocArray::erase(const String &key)
{
	Array tok(key,"/",0,true);
	if (tok.count()==0) throw InvalidKeyException(key);
	ArrayKey firstkey=tok.shift();
	ArrayKey rest=tok.implode("/");
	ValueNode *p;
	try {
		ValueNode &node=Tree.find(firstkey);
		p=&node;

	} catch (ItemNotFoundException &) {
		throw KeyNotFoundException();
	}
	// Ist noch was im Pfad rest?
	if (tok.count()>1) {			// Ja, koennen wir iterieren?
		if (p->value->isAssocArray()) {
			return ((AssocArray*)p)->erase(rest);
		} else {
			throw KeyNotFoundException();
		}
	}
	Tree.erase(firstkey);
	num--;
}

/*!\brief Zeiger für das Durchwandern des Arrays zurücksetzen
 *
 * \desc
 * Mit dieser Funktion wird der Iterator \p it, der zum Durchwandern des Arrays mit den
 * Funktion AssocArray::getNext und AssocArray::getPrevious benötigt wird,
 * auf den Anfang zurückgesetzt.
 *
 * \param[in] it Iterator. Dieser muss vom Typ ppl7::AssocArray::Iterator sein.
 *
 */
void AssocArray::reset(Iterator &it) const
{
	Tree.reset(it);
}

/*!\brief Erstes Element zurückgeben
 *
 * \desc
 * Diese Funktion liefert das erste Element des Arrays zurück. Falls der optionale Parameter
 * \p type verwendet wird, liefert die Funktion das erste Element dieses Typs zurück.
 *
 * @param it Iterator. Dieser muss vom Typ ppl7::AssocArray::Iterator sein.
 * @param type Optional der gewünschte Datentyp (siehe Variant::Type)
 * @return Referenz auf das gefundene Element
 * \exception OutOfBoundsEception: Wird geworfen, wenn das Array vollständig durchwandert
 * wurde und keine weiteren Elemente mehr vorhanden sind.
 */
Variant &AssocArray::getFirst(Iterator &it, Variant::Type type) const
{
	Tree.reset(it);
	return getNext(it,type);
}

/*!\brief Nächstes Element zurückgeben
 *
 * \desc
 * Diese Funktion liefert das nächste Element des Arrays zurück. Falls der optionale Parameter
 * \p type verwendet wird, liefert die Funktion das nächste Element dieses Typs zurück.
 *
 * @param it Iterator. Dieser muss vom Typ ppl7::AssocArray::Iterator sein.
 * @param type Optional der gewünschte Datentyp (siehe Variant::Type)
 * @return Referenz auf das gefundene Element
 * \exception OutOfBoundsEception: Wird geworfen, wenn das Array vollständig durchwandert
 * wurde und keine weiteren Elemente mehr vorhanden sind.
 */
Variant &AssocArray::getNext(Iterator &it, Variant::Type type) const
{
	while (1) {
		if (!Tree.getNext(it)) throw OutOfBoundsEception();
		if (type==Variant::UNKNOWN) break;
		if (type==it.value().value->dataType()) break;
	}
	return *it.value().value;
}

/*!\brief Letztes Element zurückgeben
 *
 * \desc
 * Diese Funktion liefert das letzte Element des Arrays zurück. Falls der optionale Parameter
 * \p type verwendet wird, liefert die Funktion das letzte Element dieses Typs zurück.
 *
 * @param it Iterator. Dieser muss vom Typ ppl7::AssocArray::Iterator sein.
 * @param type Optional der gewünschte Datentyp (siehe Variant::Type)
 * @return Referenz auf das gefundene Element
 * \exception OutOfBoundsEception: Wird geworfen, wenn das Array vollständig durchwandert
 * wurde und keine weiteren Elemente mehr vorhanden sind.
 */
Variant &AssocArray::getLast(Iterator &it, Variant::Type type) const
{
	Tree.reset(it);
	return getPrevious(it,type);
}

/*!\brief Vorhergehendes Element zurückgeben
 *
 * \desc
 * Diese Funktion liefert das vorhergehende Element des Arrays zurück. Falls der optionale Parameter
 * \p type verwendet wird, liefert die Funktion das vorhergehende Element dieses Typs zurück.
 *
 * @param it Iterator. Dieser muss vom Typ ppl7::AssocArray::Iterator sein.
 * @param type Optional der gewünschte Datentyp (siehe Variant::Type)
 * @return Referenz auf das gefundene Element
 * \exception OutOfBoundsEception: Wird geworfen, wenn das Array vollständig durchwandert
 * wurde und keine weiteren Elemente mehr vorhanden sind.
 */
Variant &AssocArray::getPrevious(Iterator &it, Variant::Type type) const
{
	while (1) {
		if (!Tree.getPrevious(it)) throw OutOfBoundsEception();
		if (type==Variant::UNKNOWN) break;
		if (type==it.value().value->dataType()) break;
	}
	return *it.value().value;
}

AssocArray &AssocArray::getFirstArray(Iterator &it) const
{
	Tree.reset(it);
	return getNextArray(it);
}

AssocArray &AssocArray::getNextArray(Iterator &it) const
{
	while (1) {
		if (!Tree.getNext(it)) throw OutOfBoundsEception();
		if (Variant::ASSOCARRAY==it.value().value->dataType()) break;
	}
	return it.value().value->toAssocArray();
}

AssocArray &AssocArray::getLastArray(Iterator &it) const
{
	Tree.reset(it);
	return getPreviousArray(it);
}

AssocArray &AssocArray::getPreviousArray(Iterator &it) const
{
	while (1) {
		if (!Tree.getPrevious(it)) throw OutOfBoundsEception();
		if (Variant::ASSOCARRAY==it.value().value->dataType()) break;
	}
	return it.value().value->toAssocArray();
}


/*!\brief Ersten %String im %Array finden und Key und Value in Strings speichern
 *
 * \desc
 * Diese Funktion sucht den ersten %String im %Array und speichert dessen
 * Schlüssel im Parameter \p key und den Wert in \p value;
 *
 * @param[in,out] it Iterator. Dieser muss vom Typ ppl7::AssocArray::Iterator sein.
 * @param[out] key String, in dem der Name des Schlüssels gespeichert werden soll
 * @param[out] value String, in dem der Wert gespeichert werden soll.
 * \return Solange Elemente gefunden werden, liefert die Funktion \c true zurück, sonst \c false.
 */
bool AssocArray::getFirst(Iterator &it, String &key, String &value) const
{
	Tree.reset(it);
	return getNext(it,key,value);
}

/*!\brief Nächsten %String im %Array finden und Key und Value in Strings speichern
 *
 * \desc
 * Diese Funktion sucht den nächsten %String im %Array und speichert dessen
 * Schlüssel im Parameter \p key und den Wert in \p value;
 *
 * @param[in,out] it Iterator. Dieser muss vom Typ ppl7::AssocArray::Iterator sein.
 * @param[out] key String, in dem der Name des Schlüssels gespeichert werden soll
 * @param[out] value String, in dem der Wert gespeichert werden soll.
 * \return Solange Elemente gefunden werden, liefert die Funktion \c true zurück, sonst \c false.
 */
bool AssocArray::getNext(Iterator &it, String &key, String &value) const
{
	while (1) {
		if (!Tree.getNext(it)) return false;
		if (it.value().value->isString()) {
			key.set(it.key());
			value.set(it.value().value->toString());
			return true;
		}
	}
	return false;
}

/*!\brief Letzten %String im %Array finden und Key und Value in Strings speichern
 *
 * \desc
 * Diese Funktion sucht den letzten %String im %Array und speichert dessen
 * Schlüssel im Parameter \p key und den Wert in \p value;
 *
 * @param[in,out] it Iterator. Dieser muss vom Typ ppl7::AssocArray::Iterator sein.
 * @param[out] key String, in dem der Name des Schlüssels gespeichert werden soll
 * @param[out] value String, in dem der Wert gespeichert werden soll.
 * \return Solange Elemente gefunden werden, liefert die Funktion \c true zurück, sonst \c false.
 */
bool AssocArray::getLast(Iterator &it, String &key, String &value) const
{
	Tree.reset(it);
	return getPrevious(it,key,value);
}

/*!\brief Vorhergehenden %String im %Array finden und Key und Value in Strings speichern
 *
 * \desc
 * Diese Funktion sucht den vorhergehenden %String im %Array und speichert dessen
 * Schlüssel im Parameter \p key und den Wert in \p value;
 *
 * @param[in,out] it Iterator. Dieser muss vom Typ ppl7::AssocArray::Iterator sein.
 * @param[out] key String, in dem der Name des Schlüssels gespeichert werden soll
 * @param[out] value String, in dem der Wert gespeichert werden soll.
 * \return Solange Elemente gefunden werden, liefert die Funktion \c true zurück, sonst \c false.
 */
bool AssocArray::getPrevious(Iterator &it, String &key, String &value) const
{
	while (1) {
		if (!Tree.getPrevious(it)) return false;
		if (it.value().value->isString()) {
			key.set(it.key());
			value.set(it.value().value->toString());
			return true;
		}
	}
	return false;
}

/*! \brief Wandelt ein Key-Value Template in ein Assoziatives Array um
 *
 * \desc
 * Diese Funktion wandelt einen Text mit Key-Value-Paaren in ein
 * Assoziatives Array um. Leere Zeilen oder Zeilen mit Raute (#)
 * am Anfang (Kommentarzeilen) werden ignoriert.
 *
 * \param[in] templ String mit den Key-Value-Paaren
 * \param[in] linedelimiter Das Zeichen, was als Zeilenende interpretiert werden soll. Default ist \c Newline
 * \param[in] splitchar Das Zeichen, was als Trennzeichen zwischen Schlüssel (Key) und Wert (Value)
 * interpretiert werden soll. Der Default ist das Gleichheitszeichen (=)
 * \param[in] concat Ist concat gesetzt und kommen im Text mehrere identische Schlüssel vor, werden die Werte
 * zu einem String zusammengeführt, wobei als Trennzeichen \c concat verwendet wird. Ist concat leer,
 * wird ein vorhandener Schlüssel überschrieben. Der Default ist, dass Werte mit gleichem Schlüssel mit
 * Newline aneinander gehangen werden.
 * \param[in] dotrim Ist \c dotrim=true, werden einzelnen Werte vor dem Einfügen ins Array mit der Funktion
 * Trim getrimmt, also Leerzeilen, Tabs und Zeilenumbrüche am Anfang und Ende gelöscht. Der Default
 * ist \c false.
 *
 * \return Die Funktion gibt die Anzahl gelesener Key-Value-Paare zurück, oder 0, wenn der Text
 * keine verwertbaren Zeilen enthielt.
 *
 * \note Falls das %Array vor dem Aufruf dieser Funktion bereits Datensätze enthielt, werden diese
 * nicht gelöscht. Die Funktion kann also benutzt werden, um Werte aus verschiedenen Templates in ein
 * einziges %Array einzulesen. Soll das %Array geleert werden, muß vorher die Funktion AssocArray::clear
 * aufgerufen werden.
 *
 * \see Um Konfigurationsdateien mit verschiedenen Abschnitten (z.B. .ini-Dateien) in ein
 * Assoziatives %Array einzulesen, gibt es die Member-Funktion
 * AssocArray::fromConfig
 *
 */
size_t AssocArray::fromTemplate(const String &templ, const String &linedelimiter, const String &splitchar, const String &concat, bool dotrim)
{
	String Row, Line;
	Array a;
	Array::Iterator it;
	String Key,Value;
	size_t rows=0;
	ssize_t p;
	size_t ssc=splitchar.size();
	a.explode(templ,linedelimiter);
	a.reset(it);
	while (1) {
		try {
			Line=a.getNext(it);
		} catch (OutOfBoundsEception &) {
			return rows;
		}
		Row=Trim(Line);
		if (Row.len()>0 && Row[0]!=L'#') { // Leere Zeilen und Kommentare ignorieren
			Row=Line;
			p=Row.instr(splitchar);
			if (p>0) {
				Key=Trim(Row.left(p));
				Value=Row.mid(p+ssc);
				if (dotrim) Value.trim();
				//printf ("Key=%ls\nValue=%ls\n",(const wchar_t *)Key, (const wchar_t *)Value);
				if (concat.notEmpty()) {
					append(Key, Value, concat);
				} else {
					set(Key,Value);
				}
				rows++;
			}
		}
	}
	return rows;
}


/*! \brief Wandelt eine Konfigurationsdatei in ein Assoziatives Array um
 *
 * \desc
 * Diese Funktion wandelt einen Konfigurations-Text mit mehreren Abschnitten
 * im Key-Value-Format in ein Assoziatives %Array um. Ein Abschnitt beginnt immer mit einem Keywort
 * in Eckigen klammern und enthält Key-Value-Paare. Zeilen mit Raute (#) am Anfang werden als
 * Kommentarzeilen interpretiert und ignoriert.
 * \par Beispiel einer Konfigurationsdatei
 * \code
[Abschnitt_1]
# Kommentarzeile, die überlesen wird
key1: value1
key2: value2
[Abschnitt_2]
key1: value1
key2: value2
\endcode
 *
 * \param[in] content Ein String, dre die zu parsende Konfiguration enthält.
 * \param[in] linedelimiter Das Zeichen, was als Zeilenende interpretiert werden soll. Default ist \c Newline
 * \param[in] splitchar Das Zeichen, was als Trennzeichen zwischen Schlüssel (Key) und Wert (Value)
 * interpretiert werden soll. Der Default ist das Gleichheitszeichen (=)
 * \param[in] concat Ist concat gesetzt und kommen im Text mehrere identische Schlüssel vor, werden die Werte
 * zu einem String zusammengeführt, wobei als Trennzeichen \c concat verwendet wird. Ist concat NULL,
 * wird ein vorhandener Schlüssel überschrieben. Der Default ist, dass gleiche Schlüssel mit Newline
 * aneinander gehangen werden.
 * \param[in] dotrim Ist \c dotrim=true, werden einzelnen Werte vor dem Einfügen ins Array mit der Funktion
 * Trim getrimmt, also Leerzeilen, Tabs und Zeilenumbrüche am Anfang und Ende gelöscht. Der Default
 * ist \c false.
 *
 * \return Die Funktion gibt die Anzahl gelesener Key-Value-Paare zurück, oder 0, wenn der Text
 * keine verwertbaren Zeilen enthielt.
 *
 * \note Falls das %Array vor dem Aufruf dieser Funktion bereits Datensätze enthielt, werden diese
 * nicht gelöscht. Die Funktion kann also benutzt werden, um Werte aus verschiedenen Templates in ein
 * einziges %Array einzulesen. Soll das %Array geleert werden, muß vorher die Funktion AssocArray::clear
 * aufgerufen werden.
 */
size_t AssocArray::fromConfig(const String &content, const String &linedelimiter, const String &splitchar, const String &concat, bool dotrim)
{
	String Row, Line, Section;
	Array a;
	Array::Iterator it;
	String Key,Value;
	size_t rows=0;
	ssize_t p;
	size_t ssc=splitchar.size();
	a.explode(content,linedelimiter);
	a.reset(it);
	while (1) {
		try {
			Line=a.getNext(it);
		} catch (OutOfBoundsEception &) {
			return rows;
		}
		Row=Trim(Line);
		if (Row.len()>0 && Row[0]!=L'#') { // Leere Zeilen und Kommentare ignorieren
			if (Row[0]==L'[' && Row[-1]==L']') {
				Section=Row.mid(1,Row.len()-2);
				Section.lowerCase();
				Section.trim();
			} else {
				Row=Line;
				p=Row.instr(splitchar);
				if (p>0) {
					Key=Section;
					if (Key.notEmpty()) Key+="/";
					Key+=Trim(Row.left(p));
					Value=Row.mid(p+ssc);
					if (dotrim) Value.trim();
					//printf ("Key=%ls\nValue=%ls\n",(const wchar_t *)Key, (const wchar_t *)Value);
					if (concat.notEmpty()) {
						append(Key, Value, concat);
					} else {
						set(Key,Value);
					}
					rows++;
				}
			}
		}
	}
	return rows;
}


/*!\brief Inhalt des Assoziativen Arrays in ein Template exportieren
 *
 * \desc
 * Mit dieser Funktion wird der textuelle Inhalt des Arrays als Template im Key-Value-Format in einem String
 * abgelegt.
 * Pointer oder Binäre Daten werden ignoriert.
 *
 * \param[out] s %String, in dem das Template gespeichert werden soll. Der %String wird von der Funktion nicht gelöscht,
 * der Inhalt des Arrays wird angehangen!
 * \param[in] prefix Optionaler Prefix, der jedem Key vorangestellt werden soll
 * \param[in] linedelimiter Optionaler Pointer auf einen String, der am Zeilenende ausgegeben werden soll. Der
 *            Default ist ein einzelnes Newline.
 * \param[in] splitchar Optionaler Pointer auf einen String, der als Trennzeichen zwischen Schlüssel und Wert
 *            verwendet werden soll. Der Default ist ein Gleichheitszeichen.
 * \par Beispiel
\code
#include <stdio.h>
#include <string.h>
#include <ppl7.h>

int main(int argc, char **argv)
{
	ppl7::AssocArray a;
	ppl7::ByteArray bin;
	ppl7::String out;
	bin.load("main.cpp");
	a.set("key1","Dieser Wert geht über\nmehrere Zeilen");
	a.set("array1/unterkey1","value2");
	a.set("array1/unterkey2","value3");
	a.set("array1/noch ein array/unterkey1","value4");
	a.set("array1/unterkey2","value5");
	a.set("key2","value6");
	a.set("dateien/main.cpp",&bin);
	a.set("array2/unterkey1","value7");
	a.set("array2/unterkey2","value8");
	a.set("array2/unterkey1","value9");
	a.toTemplate(&out,"foo");
	out.printnl();
}
\endcode
Ergebnis:
\code
foo/array1/noch ein array/unterkey1=value4
foo/array1/unterkey1=value2
foo/array1/unterkey2=value5
foo/array2/unterkey1=value9
foo/array2/unterkey2=value8
foo/key1=Dieser Wert geht über
foo/key1=mehrere Zeilen
foo/key2=value6
\endcode
	An diesem Beispiel sieht man, dass Pointer- und ByteArray-Werte nicht exportiert werden und Werte, die Zeilenumbrüche
	enthalten, werden auf mehrere Key-Value-Paare aufgesplittet. Die Importfunktion (AssocArray::fromTemplate,
	AssocArray::fromConfig) fügen diese wieder zu einer einzelnen Variable mit Zeilenumbruch
	zusammen.
 */
void AssocArray::toTemplate(String &s, const String &prefix, const String &linedelimiter, const String &splitchar)
{
	String	key, pre, value, index;
	Array		Tok;
	if (prefix.notEmpty()) key=prefix+"/";
	ppl7::AVLTree<ArrayKey, ValueNode>::Iterator it;
	Tree.reset(it);
	Variant *p;
	while ((Tree.getNext(it))) {
		p=it.value().value;
		if (p->isString()) {
			Tok.clear();
			Tok.explode(p->toString(),"\n");
			for (size_t i=0;i<Tok.size();i++) {
				s+=key+it.key()+splitchar+Tok[i]+linedelimiter;
			}
		} else if (p->isAssocArray()) {
			pre.setf("%s%s",(const char*)key,(const char*)it.key());
			((AssocArray*)p)->toTemplate(s,pre,linedelimiter,splitchar);
		} else if (p->isArray()) {
			const Array &a=(const Array &)*p;
			for (size_t i=0;i<a.size();i++) {
				Tok.clear();
				Tok.explode(a[i],"\n");
				index.setf("%zu",i);
				for (size_t z=0;z<Tok.size();z++) {
					s+=key+it.key()+"/"+index+splitchar+Tok[z]+linedelimiter;
				}
			}
		} else if (p->isDateTime()) {
			s+=key+it.key()+splitchar+((DateTime*)p)->getISO8601withMsec()+linedelimiter;
		}
	}
}

/*!\brief Liefert Anzahl Bytes, die für exportBinary erforderlich sind
 *
 * \desc
 * Diese Funktion liefert die Anzahl Bytes zurück, die für den Buffer der Funktion AssocArray::exportBinary
 * erforderlich sind. Es kann dadurch ein ausreichend großer Puffer vor Aufruf der Funktion exportBinary
 * angelegt werden.
 *
 * \return Anzahl Bytes oder 0 im Fehlerfall
 *
 * \see
 * - AssocArray::exportBinary
 * - AssocArray::importBinary
 */
size_t AssocArray::binarySize() const
{
	size_t size;
	exportBinary(NULL,0, &size);
	return size;
}

/*!\brief Inhalt des Arrays in einem plattform-unabhängigen Binären-Format exportieren
 *
 * \desc
 * Mit dieser Funktion kann der komplette Inhalt des Arrays in einem plattform-unabhängigem binären Format abgelegt
 * werden, das sich zum Speichern in einer Datei oder zum Übertragen über das Internet eignet.
 *
 * \param[in] buffer Pointer auf einen ausreichend großen Puffer. Die Größe des benötigten Puffers
 *            kann zuvor mit der Funktion AssocArray::binarySize ermittelt werden. Wird als Buffer NULL
 *            übergeben, wird in der Variable \p realsize ebenfalls die Anzahl Bytes zurückgegeben
 * \param[in] buffersize Die Größe des Puffers in Bytes
 * \param[out] realsize In dieser Variable wird gespeichert, wieviele Bytes tatsächlich für den Export
 *            verwendet wurden
 * \exception ExportBufferToSmallException: Wird geworfen, wenn \p buffersize nicht groß genug ist, um
 * das Assoziative Array vollständig exportieren zu können.
 *
 * \attention
 * Es muss daran gedacht werden, dass nicht alle Datentypen exportiert werden können. Gegenwärtig
 * werden folgende Typen unterstützt:
 * - String (Wird als UTF-8 exportiert)
 * - Array
 * - AssocArray
 * - ByteArray
 * - ByteArrayPtr (wird in ein ByteArray umgewandelt!)
 * - DateTime
 * \see
 * - AssocArray::binarySize
 * - AssocArray::importBinary
 *
 * \note
 * Das exportierte Binary ist komptibel mit dem Assoziativen Array der PPL-Version 6
 */
void AssocArray::exportBinary(void *buffer, size_t buffersize, size_t *realsize) const
{
	char *ptr=(char*)buffer;
	*realsize=0;
	size_t p=0;
	size_t keylen;
	size_t vallen=0;
	ByteArray key;
	String string;
	if (!buffer) buffersize=0;
	if (p+7<buffersize) strncpy(ptr,"PPLASOC",7);
	p+=7;
	ppl7::AVLTree<ArrayKey, ValueNode>::Iterator it;
	Tree.reset(it);
	Variant *a;
	while ((Tree.getNext(it))) {
		a=it.value().value;
		if (p<buffersize) {
			if (a->isByteArrayPtr()) PokeN8(ptr+p,Variant::BYTEARRAY);
			else PokeN8(ptr+p,a->dataType());
		}
		p++;
		key=it.key();
		keylen=key.size();
		if (p+4<buffersize) PokeN16(ptr+p,keylen);
		p+=2;
		if (p+keylen<buffersize) strncpy(ptr+p,(const char*)key,keylen);
		p+=keylen;
		if (a->isString()) {
			string=*((String*)a);
			vallen=string.size();
			if (p+4<buffersize) PokeN32(ptr+p,vallen);
			p+=4;
			if (p+vallen<buffersize) strncpy(ptr+p,(const char*)string,vallen);
			p+=vallen;
		} else if (a->isAssocArray()) {
			size_t asize=0;
			((AssocArray*)a)->exportBinary(ptr+p,buffersize-p,&asize);
			p+=asize;
		} else if (a->isDateTime()) {
			vallen=8;
			if (p+4<buffersize) PokeN32(ptr+p,vallen);
			p+=4;
			if (p+vallen<buffersize) PokeN64(ptr+p,((DateTime*)a)->longInt());
			p+=vallen;
		} else if (a->isByteArray()==true || a->isByteArrayPtr()==true) {
			vallen=((ByteArrayPtr*)a)->size();
			if (p+4<buffersize) PokeN32(ptr+p,vallen);
			p+=4;
			if (p+vallen<buffersize) memcpy(ptr+p,((ByteArrayPtr*)a)->adr(),vallen);
			p+=vallen;
		} else {
			vallen=0;
			if (p+4<buffersize) PokeN32(ptr+p,0);
			p+=4;
		}
	}
	if (p<buffersize) PokeN8(ptr+p,0);
	p++;
	if (realsize)*realsize=p;
	if (buffersize==0 || buffersize<=p) return;
	throw ExportBufferToSmallException();
}

/*!\brief Inhalt des Arrays in einem plattform-unabhängigen Binären-Format exportieren
 *
 * \desc
 * Mit dieser Funktion kann der komplette Inhalt des Arrays in einem plattform-unabhängigem binären Format abgelegt
 * werden, das sich zum Speichern in einer Datei oder zum Übertragen über das Internet eignet.
 *
 * \param[in,out] buffer %ByteArray, in dem die exportierten Daten gespeichert werden sollen
 *
 * \attention
 * Es muss daran gedacht werden, dass nicht alle Datentypen exportiert werden können. Gegenwärtig
 * werden folgende Typen unterstützt:
 * - String (Wird als UTF-8 exportiert)
 * - Array
 * - AssocArray
 * - ByteArray
 * - ByteArrayPtr (wird in ein ByteArray umgewandelt!)
 * - DateTime
 * \see
 * - AssocArray::binarySize
 * - AssocArray::importBinary
 *
 * \note
 * Das exportierte Binary ist komptibel mit dem Assoziativen Array der PPL-Version 6
 */
void AssocArray::exportBinary(ByteArray &buffer) const
{
	buffer.free();
	size_t size;
	exportBinary(NULL,0, &size);
	buffer.malloc(size);
	exportBinary((void*)buffer.adr(),buffer.size(),NULL);
}

/*!\brief Daten aus einem vorherigen Export wieder importieren
 *
 * \desc
 * Mit dieser Funktion kann ein zuvor mit AssocArray::exportBinary exportiertes Assoziatives %Array wieder
 * importiert werden. Falls im %Array bereits Daten vorhanden sind, werden diese nicht gelöscht, können aber
 * überschrieben werden, wenn es im Export gleichnamige Schlüssel gibt.
 *
 * \param[in] bin Referenz auf ByteArray oder ByteArrayPtr mit den zu importierenden Daten
 *
 * \see
 * - CAssocArray::exportBinary
 * - CAssocArray::binarySize
 */
void AssocArray::importBinary(const ByteArrayPtr &bin)
{
	importBinary(bin.adr(),bin.size());
}

/*!\brief Daten aus einem vorherigen Export wieder importieren
 *
 * \desc
 * Mit dieser Funktion kann ein zuvor mit AssocArray::exportBinary exportiertes Assoziatives %Array wieder
 * importiert werden. Falls im %Array bereits Daten vorhanden sind, werden diese nicht gelöscht, können aber
 * überschrieben werden, wenn es im Export gleichnamige Schlüssel gibt.
 *
 * \param[in] buffer Pointer auf den Puffer, der die zu importierenden Daten enthält
 * \param[in] buffersize Größe des Puffers
 * \exception ImportFailedException
 *
 * \see
 * - AssocArray::exportBinary
 * - AssocArray::binarySize
 */
size_t AssocArray::importBinary(const void *buffer, size_t buffersize)
{
	if (!buffer) throw IllegalArgumentException();
	if (buffersize==0) throw IllegalArgumentException();
	const char *ptr=(const char*)buffer;
	size_t p=0;
	if (buffersize<8 || strncmp((const char*)ptr,"PPLASOC",7)!=0) {
		throw ImportFailedException();
	}
	p+=7;
	int type;
	size_t keylen,vallen,bytes;
	String key;
	DateTime dt;
	AssocArray na;
	ByteArray nb;
	while (p<buffersize && (type=PeekN8(ptr+p))!=0) {
		p++;
		keylen=PeekN16(ptr+p);
		p+=2;
		key.set(ptr+p,keylen);
		p+=keylen;
		switch (type) {
			case Variant::STRING:
				vallen=PeekN32(ptr+p);
				p+=4;
				set(key,(const char*)ptr+p,vallen);
				p+=vallen;
				break;
			case Variant::ASSOCARRAY:
				na.clear();
				bytes=na.importBinary(ptr+p,buffersize-p);
				p+=bytes;
				set(key,na);
				break;
			case Variant::BYTEARRAY:
				vallen=PeekN32(ptr+p);
				p+=4;
				nb.free();
				nb.copy(ptr+p,vallen);
				p+=vallen;
				set(key,nb);
				break;
			case Variant::DATETIME:
				vallen=PeekN32(ptr+p);
				p+=4;
				dt.setLongInt(PeekN64(ptr+p));
				p+=vallen;
				set(key,dt);
				break;
			default:
				return 0;
		};
	}
	p++;
	return p;
}



/*!\brief Schlüssel auslesen
 *
 * \desc
 * Dieser Operator liefert den Wert des Schlüssels \p key als Variant zurück. Dieser kann
 * von der aufrufenden Anwendung in den jeweiligen Datentyp umgewandelt werden.
 *
 * @param key Name des Schlüssels
 * @return Referenz auf den einen Variant mit dem Wert des Schlüssels
 * \exception InvalidKeyException: Ungültiger Schlüssel
 * \exception KeyNotFoundException: Schlüssel wurde nicht gefunden
 */
Variant &AssocArray::operator[](const String &key) const
{
	ValueNode *node=findInternal(key);
	return *node->value;
}

/*!\brief Assoziatives Array kopieren
 *
 * \desc
 * Mit diesem Operator wird der Inhalt das Assoziativen Arrays \p other übernommen.
 * Der bisherige Inhalt dieses Arrays geht verloren.
 *
 * @param other Zu kopierendes assoziatives Array
 * @return Referenz auf dieses Array
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 *
 */
AssocArray& AssocArray::operator=(const AssocArray& other)
{
	clear();
	add(other);
	return *this;
}

/*!\brief Assoziatives Array hinzufügen
 *
 * \desc
 * Mit diesem Operator wird der Inhalt das Assoziativen Arrays \p other dem eigenen
 * Array hinzugefügt. Das Array wird vorher nicht gelöscht, so dass vorhandene
 * Schlüssel erhalten bleiben. Gibt es in \p other jedoch gleichnamige Schlüssel,
 * werden die bisherigen Werte überschrieben.
 *
 * @param other Zu kopierendes assoziatives Array
 * @return Referenz auf dieses Array
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 *
 */
AssocArray& AssocArray::operator+=(const AssocArray& other)
{
	add(other);
	return *this;
}

} // EOF namespace ppl7
