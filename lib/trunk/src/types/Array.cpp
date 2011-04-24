/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.7 $
 * $Date: 2011/04/23 10:34:49 $
 * $Id: Array.cpp,v 1.7 2011/04/23 10:34:49 patrick Exp $
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


/*!\class Array
 * \ingroup PPLGroupDataTypes
 * \brief Ein Array mit Strings
 *
 * \desc
 * Diese Klasse repräsentiert ein Array aus Strings, die über einen Index angesprochen werden
 * können. Die Zählung der Elemente beginnt dabei bei 0, das heisst das erste Element hat den
 * Index 0 (vergleichbar mit Arrays in C/C++).
 *
 */


typedef struct {
	String *value;
} ROW;

static String EmptyString;


/*!\brief Konstruktor
 *
 * \desc
 * Der Standard-Konstruktor erstellt ein leeres Array
 */
Array::Array()
{
	type=ARRAY;
	numElements=0;
	numCapacity=0;
	pos=0;
	rows=NULL;
}

/*!\brief Copy-Konstruktor
 *
 * \desc
 * Mit dem Copy-Konstruktor wird der Inhalt des Arrays \p other 1:1 kopiert.
 *
 * @param other Anderes Array
 */
Array::Array(const Array &other)
{
	type=ARRAY;
	numElements=0;
	numCapacity=0;
	pos=0;
	rows=NULL;
	add(other);
}

/*!\brief Konstruktor aus String
 *
 * \desc
 * Mit diesem Konstruktor wird der String \p str anhand des Trennzeichens
 * \p delimiter in einzelne Elemente zerlegt
 *
 * @param str String
 * @param delimiter Trennzeichen oder Trennstring
 * @param limit Maximale Anzahl Elemente, normalerweise unbegrenzt
 * @param skipemptylines Leere Elemente überspringen. Folgen zwei Trennzeichen hintereinander, würde
 * normalerweise ein leeres Element in das Array eingefügt. Durch setzen dieses Parameters auf \c true
 * werden keine leeren Elemente eingefügt.
 *
 * \see
 * Dieser Konstruktor verwendet die Funktion Array::explode zum Zerlegen des Strings.
 */
Array::Array(const String &str, const String &delimiter, size_t limit, bool skipemptylines)
{
	type=ARRAY;
	numElements=0;
	numCapacity=0;
	pos=0;
	rows=NULL;
	explode(str,delimiter,limit,skipemptylines);
}

/*!\brief Destruktor
 *
 * \desc
 * Das Array und aller dadurch belegter Speicher wird wieder freigegeben.
 */
Array::~Array()
{
	clear();
}


/*!\brief Array löschen
 *
 * \desc
 * Der durch das Array belegte Speicher wird freigegeben. Das Array ist danach leer und kann erneut befüllt werden.
 *
 */
void Array::clear()
{
	ROW *r=(ROW*)rows;
	if (r) {
		for (size_t i=0;i<numCapacity;i++) {
			if (r[i].value) {
				delete(r[i].value);
				r[i].value=NULL;
			}
		}
		free(rows);
	}
	rows=NULL;
	numCapacity=0;
	numElements=0;
	pos=0;
}

/*!\brief Array kopieren
 *
 * \desc
 * Der bisherige Inhalt des Arrays wird gelöscht und mit dem das Arrays \p other gefüllt. Falls der bestehende
 * Inhalt nicht gelöscht werden soll, muss die Funktion Array::add(const Array &other) verwendet werden.
 *
 * @param other Zu kopierendes Array
 */
void Array::copy(const Array &other)
{
	clear();
	ROW *r=(ROW*)other.rows;
	for (size_t i=0;i<other.numElements;i++) {
		if (r[i].value!=NULL) set(i,*r[i].value);
	}
}

/*!\brief Array hinzufügen
 *
 * \desc
 * Der Inhalt des Arrays \p other wird an das Array angehangen.
 *
 * @param other Zu kopierendes Array
 */
void Array::add(const Array &other)
{
	ROW *r=(ROW*)other.rows;
	size_t first=numElements;
	for (size_t i=0;i<other.numElements;i++) {
		if (r[i].value!=NULL) set(first+i,*r[i].value);
	}
}

/*!\brief String anhängen
 *
 * \desc
 * Der Inhalt des Strings \p value wird dem Array hinzugefügt.
 *
 * @param value String
 */
void Array::add(const String &value)
{
	set(numElements,value);
}

/*!\brief Teilstring anhängen
 *
 * \desc
 * Der Inhalt des Strings \p value und der Länge \p size wird dem Array hinzugefügt.
 *
 * @param value String
 * @param size Größe des Strings
 */
void Array::add(const String &value, size_t size)
{
	set(numElements,value.left(size));
}

/*!\brief Formatierten String anhängen
 *
 * \desc
 * Mittels Formatstring \p fmt und der optionalen Parameter wird ein neuer String gebildet, der am
 * Ende des Array angehangen wird.
 *
 * @param fmt Formatstring
 * @param ... Optionale Parameter
 */
void Array::addf(const char *fmt, ...)
{
	String value;
	va_list args;
	va_start(args, fmt);
	value.vasprintf(fmt,args);
	va_end(args);
	set(numElements,value);
}

/*!\brief Wert eines Elements setzen
 *
 * \desc
 * Der Wert des Strings \p value wird an der Position \p index des Arrays gesetzt. Der vorherige Wert
 * des Arrays an dieser Stelle wird dadurch überschrieben.
 *
 * @param index Position innerhalb des Arrays, beginnend mit 0
 * @param value String
 */
void Array::set(size_t index, const String &value)
{
	ROW *r;
	if (index>=numCapacity) {
		// Array muss vergroessert werden
		reserve(index+10);
	}
	r=(ROW*)rows;
	if ((index+1)>numElements) numElements=index+1;
	if (value.notEmpty()) {
		if (r[index].value==NULL) {
			r[index].value=new String;
			if (!r[index].value) throw OutOfMemoryException();
		}
		r[index].value->set(value);
	} else {
		if (r[index].value==NULL) {
			delete r[index].value;
			r[index].value=NULL;
		}
	}
}

/*!\brief Formatierten String setzen
 *
 * \desc
 * Mittels Formatstring \p fmt und der optionalen Parameter wird ein neuer String gebildet, der an
 * die Position \p index des Arrays gesetzt wird. Der vorherige Wert
 * des Arrays an dieser Stelle wird überschrieben.
 *
 * @param index Position innerhalb des Arrays, beginnend mit 0
 * @param fmt Formatstring
 * @param ... Optionale Parameter
 */
void Array::setf(size_t index, const char *fmt, ...)
{
	String value;
	va_list args;
	va_start(args, fmt);
	value.vasprintf(fmt,args);
	va_end(args);
	set(index,value);
}


/*!\brief Element im Array einfügen
 *
 * \desc
 * Alle vorhandenen Elemente des Arrays ab Position \p index werden um eins nach hinten
 * verschoben. Anschließend wird das neue Element \p value an der Position \p index
 * eingefügt.
 *
 * @param index Position, an der das Element eingefügt werden soll
 * @param value Wert des Elements
 */
void Array::insert(size_t index, const String &value)
{
	ROW *r=(ROW*)rows;
	// Zunächst sorgen wir dafür, dass im Array genug Platz ist
	reserve(numElements+2);
	// Nun verschieben wir alle Elemente ab Position index um eins nach hinten
	if (numElements>index) {
		for (size_t i=numElements;i>index;i--) {
			r[i].value=r[i-1].value;
		}
		numElements++;
	}
	// Den neuen Wert einfügen
	set(index,value);
}

/*!\brief Array einfügen
 *
 * \desc
 * Das komplette Array \p other wird ab der Position \p index eingefügt. Alle bisher
 * vorhandenen Elemente werden ab dieser Position um die Größe des einzufügenden
 * Arrays nach hinten verschoben.
 *
 * @param index Position, an der das Element eingefügt werden soll
 * @param other Einzufügendes Array
 */
void Array::insert(size_t index, const Array &other)
{
	if (other.numElements==0) return;	// Anderes Array ist leer
	ROW *r=(ROW*)rows;
	// Zunächst sorgen wir dafür, dass im Array genug Platz ist
	reserve(numElements+other.numElements+2);
	// Nun verschieben wir alle Elemente ab Position index um die größe des anderen
	// Arrays nach hinten
	if (numElements>index) {
		for (size_t i=numElements-1;i>=index;i--) {
			r[i+other.numElements].value=r[i].value;
			r[i].value=NULL;
		}
		numElements+=other.numElements;
	}
	// Die neuen Werte einfügen
	ROW *r2=(ROW*)other.rows;
	for (size_t i=0;i<other.numElements;i++) {
		if (r2[i].value!=NULL) {
			r[index+i].value=new String;
			if (!r[index+i].value) throw OutOfMemoryException();
			r[index+i].value->set(r2[i].value);
		}
	}
}

/*!\brief Element mittels Formatstring einfügen
 *
 * \desc
 * Alle vorhandenen Elemente des Arrays ab Position \p index werden um eins nach hinten
 * verschoben. Anschließend wird ein neuer Wert mittels des Formatstrings \p fmt
 * und den zusätzlichen Parametern gebildet und an der Position \p index
 * eingefügt.
 *
 * @param index Position, an der das Element eingefügt werden soll
 * @param fmt Formatstring
 * \param ... Zusätzliche optionale Parameter
 */
void Array::insertf(size_t index, const char *fmt, ...)
{
	String value;
	va_list args;
	va_start(args, fmt);
	value.vasprintf(fmt,args);
	va_end(args);
	insert(index,value);
}


/*!\brief Platz reservieren
 *
 * \desc
 * Durch Aufruf dieser Funktion wird vorab Speicher allokiert, um die durch \p size angegebene
 * Anzahl Elemente aufnehmen zu können. Die Funktion sollte immer dann aufgerufen werden, wenn
 * schon vor dem Befüllen des Array bekannt ist, wieviele Elemente es aufnehmen soll. Insbesondere
 * bei großen Arrays ist dies sinnvoll, da dadurch das Reallokieren und Kopieren von Speicher
 * während der Befüllung reduziert wird.
 *
 * @param size Anzahl Elemente, die das Array aufnehmen soll
 *
 * \note
 * Ist die Kapazität des Arrays bei Aufruf der Funktion bereits höher als der angegebene Wert
 * \p size, bleibt das Array unverändert. Die Kapazität kann nachträglich nicht verkleinert werden.
 *
 * \see
 * Mit der Funktion Array::capacity kann abgefragt werden, für wieviele Elemente derzeit Speicher
 * reserviert ist.
 */
void Array::reserve (size_t size)
{
	if (size>numCapacity) {
		ROW *r;
		// Array muss vergroessert werden
		void *newrows=realloc(rows,(size)*sizeof(ROW));
		if (!newrows) {
			throw OutOfMemoryException();
		}
		r=(ROW*)newrows;
		for (size_t i=numCapacity;i<size;i++) {
			r[i].value=NULL;
		}
		rows=newrows;
		numCapacity=size;
	}
}

/*!\brief Anzahl Elemente, für die Speicher reserviert ist
 *
 * \desc
 * Diese Funktion gibt aus, wieviele Elemente das Array aufnehmen kann, ohne dass Speicher reallokiert
 * und kopiert werden muss.
 *
 * @return Anzahl Elemente
 * \see
 * Mit der Funktion Array::reserve kann die Kapazität des Arrays vorab bestimmt werden.
 */
size_t Array::capacity() const
{
	return numCapacity;
}

/*!\brief Anzahl Elemente im Array
 *
 * \desc
 * Diese Funktion gibt die Anzahl Elemente im Array zurück.
 *
 * @return Anzahl Elemente
 *
 * \note
 * Wird bei einem leeren Array ein String an der Position 5 eingefügt, werden die Positionen 0 bis 4 automatisch
 * als leere Elemente interpretiert. Array::count wird daher 6 zurückgeben.
 * \see
 * Die Funktionen Array::size und Array::count sind identisch.
 */
size_t Array::count() const
{
	return numElements;
}

/*!\brief Anzahl Elemente im Array
 *
 * \desc
 * Diese Funktion gibt die Anzahl Elemente im Array zurück.
 *
 * @return Anzahl Elemente
 *
 * \note
 * Wird bei einem leeren Array ein String an der Position 5 eingefügt, werden die Positionen 0 bis 4 automatisch
 * als leere Elemente interpretiert. Array::size wird daher 6 zurückgeben.
 * \see
 * Die Funktionen Array::size und Array::count sind identisch.
 */
size_t Array::size() const
{
	return numElements;
}

/*!\brief Array leer?
 *
 * \desc
 * Prüft, ob das Array leer ist.
 * @return Gibt \c true zurück, wenn das Array leer ist, also keine Elemente enthält, andernfalls \c true.
 */
bool Array::empty() const
{
	if (numElements==0) return true;
	return false;
}


/*!\brief Inhalt des Arrays ausgeben
 *
 * \desc
 * Der Inhalt des Arrays wird auf der Konsole ausgegeben
 *
 * @param prefix Optionaler String, der jedem Element vorangestellt wird.
 *
 * \example
\code
ppl7::Array a;
a.add(L"Value 1");
a.add(L"Value 2");
a.add(L"Value 3");
a.add(L"Value 4");
a.add(L"Value 5");
a.list("Mein Array");
\endcode
Ausgabe:
\verbatim
MeinArray,      0: Value 1
MeinArray,      1: Value 2
MeinArray,      2: Value 3
MeinArray,      3: Value 4
MeinArray,      4: Value 5
\endverbatim
 */
void Array::list(const String &prefix) const
{
	ROW *r=(ROW*)rows;
	if (prefix.isEmpty()) {
		if ((!rows) || numElements==0) {
			PrintDebug("Array ist leer\n");
		}
		for (size_t i=0;i<numElements;i++) {
			if (r[i].value!=NULL) PrintDebug ("%6zu: %ls\n",i,(const wchar_t*)r[i].value);

		}
	} else {
		if ((!rows) || numElements==0) {
			PrintDebug("Array \"%ls\" ist leer\n", (const wchar_t*)prefix);
		}
		for (size_t i=0;i<numElements;i++) {
			if (r[i].value!=NULL) PrintDebug ("%ls, %6zu: %ls\n",(const wchar_t*)prefix, i,(const wchar_t*)r[i].value);
		}

	}
}

/*!\brief Element auslesen
 *
 * \desc
 * Gibt das Element an Position \p index des Arrays als Referenz zurück. Ist \p index größer als die Anzahl
 * Elemente des Arrays, wird eine Exception geworfen.
 *
 * @param index Gewünschtes Element
 * @return Referenz auf den Inhalt des Elements
 * \exception OutOfBoundsEception: Wird geworfen, wenn \p index größer als die Anzahl Elemente des Arrays ist
 */
const String &Array::get(size_t index) const
{
	ROW *r=(ROW*)rows;
	if (index>=numElements) throw OutOfBoundsEception();
	if (r[index].value!=NULL) return *r[index].value;
	return EmptyString;
}

/*!\brief wchar_t Pointer auf ein Element auslesen
 *
 * \desc
 * Gibt einen wchar_t Pointer auf das Element \p index zurück. Liegt \p index ausserhalb des Arrays wird
 * eine Exception geworfen.
 *
 * @param index Gewünschtes Element
 * @return Pointer auf den Wide-Character-String (Unicode) des gewünschten Elements.
 * \exception OutOfBoundsEception: Wird geworfen, wenn \p index größer als die Anzahl Elemente des Arrays ist
 */
const wchar_t *Array::getPtr(size_t index) const
{
	ROW *r=(ROW*)rows;
	if (index>=numElements) throw OutOfBoundsEception();
	if (r[index].value!=NULL) return r[index].value->getPtr();
	return L"";
}

const wchar_t *Array::getRandomPtr() const
{
	if (!numElements) return String();
	ROW *r=(ROW*)rows;
	size_t index=ppl7::rand(0,numElements-1);
	if (index<numElements && r[index].value!=NULL) return r[index].value->getPtr();
	return L"";
}


const String &Array::operator[](size_t index) const
{
	ROW *r=(ROW*)rows;
	if (index<numElements && r[index].value!=NULL) return *r[index].value;
	return EmptyString;
}

Array& Array::operator=(const Array &other)
{
	copy(other);
	return *this;
}

Array& Array::operator+=(const Array &other)
{
	add(other);
	return *this;
}


void Array::reset()
{
	pos=0;
}

const String &Array::getFirst()
{
	pos=0;
	return getNext();
}

const String &Array::getNext()
{
	ROW *r=(ROW*)rows;
	if (pos<numElements) {
		String *s=r[pos].value;
		pos++;
		if (s!=NULL) return *s;
		return EmptyString;
	}
	throw OutOfBoundsEception();
}

String Array::shift()
{
	String ret;;
	if (numElements) {
		ROW *r=(ROW*)rows;
		if (numElements>0 && r[0].value!=NULL) {
			ret=r[0].value;
			delete r[0].value;
		}
		for (size_t i=0;i<numElements-1;i++) {
			r[i].value=r[i+1].value;
		}
		numElements--;
		r[numElements].value=NULL;
	}
	return ret;
}

String Array::pop()
{
	if (!numElements) {
		return L"";
	}
	ROW *r=(ROW*)rows;
	String ret;
	if (r[numElements-1].value!=NULL) {
		ret=r[numElements].value;
		delete r[numElements-1].value;
		r[numElements-1].value=NULL;
	}
	numElements--;
	return ret;
}


const String &Array::getRandom() const
{
	if (!numElements) return EmptyString;
	ROW *r=(ROW*)rows;
	size_t index=ppl7::rand(0,numElements-1);
	if (index<numElements && r[index].value!=NULL) return *r[index].value;
	return EmptyString;
}

Array &Array::explode(const String &text, const String &delimiter, size_t limit, bool skipemptylines)
{
	if (text.isEmpty()) return *this;
	if (delimiter.isEmpty()) return *this;
	ssize_t p;
	size_t t=delimiter.len();
	size_t count=0;
	wchar_t *etext=(wchar_t*)text.getPtr();
	while (1) {
		count++;
		p=Instr(etext,(const wchar_t*)delimiter,0);
		if (p>=0) {
			if (p==0 && skipemptylines==true) {
				etext+=t;
				continue;
			}
			if (limit>0 && count==limit) {
				add((wchar_t*)etext);
				return *this;
			}
			add((wchar_t*)etext,p);
			etext=etext+p+t;
		} else {
			if (skipemptylines==false || wcslen(etext)>0) add(etext);
			return *this;
		}
	}

	return *this;
}


String Array::implode(const String &trenn) const
{
	String ret;
	for (size_t i=0;i<numElements;i++) {
		if (i) ret+=trenn;
		ret+=get(i);
	}
	return ret;
}


Array &Array::fromArgs(int argc, const char **argv)
{
	clear();
	for (int i=0;i<argc;i++) {
		add(argv[i]);
	}
	return *this;
}

Array &Array::fromArgs(const String &args)
{
	clear();
	String buffer(args);
	String arg;
	// Kommandozeile in argc und argv[] umwandeln

	size_t l=buffer.len();
	add(args);
	bool inDoubleQuote=false;
	bool inSingleQuote=false;
	size_t start=0;
	for (size_t i=0;i<l;i++) {
		if(buffer[i]==34 && inDoubleQuote==false && inSingleQuote==false) {
			if (i==0) {
				inDoubleQuote=true;
				start=i+1;
			}
			else if (buffer[i-1]!='\\') {
				inDoubleQuote=true;
				start=i+1;
			}
		} else if(buffer[i]=='\'' && inDoubleQuote==false && inSingleQuote==false) {
				if (i==0) {
					inSingleQuote=true;
					start=i+1;
				}
				else if (buffer[i-1]!='\\') {
					inSingleQuote=true;
					start=i+1;
				}

		} else if(buffer[i]==34 && inDoubleQuote==true && buffer[i-1]!='\\') {
			inDoubleQuote=false;
			arg=buffer.mid(start,i-start);
			if (arg.notEmpty()) add(arg);

			//if(argv[argc][0]!=0) argc++;
			start=i+1;
		} else if(buffer[i]=='\'' && inSingleQuote==true && buffer[i-1]!='\\') {
			inSingleQuote=false;
			arg=buffer.mid(start,i-start);
			if (arg.notEmpty()) add(arg);

			//if(argv[argc][0]!=0) argc++;
			start=i+1;
		} else if((buffer[i]==' ' || buffer[i]=='\t') && inDoubleQuote==false && inSingleQuote==false) {
			arg=Trim(buffer.mid(start,i-start));
			if (arg.notEmpty()) add(arg);
			start=i+1;
		}
	}
	if (start<l) {
		arg=Trim(buffer.mid(start,l-start));
		if (arg.notEmpty()) add(arg);
	}
	return *this;
}


/*!\brief Zwei Arrays zusammenaddieren
 * \relates Array
 *
 * \desc
 * Mit diesem Operator wird der Inhalt zweier Arrays zu einem neuen Array
 * zusammenaddiert
 *
 * @param a1 Erstes Array
 * @param a2 Zweites Array
 * @return Neues Array mit den Werten beider Arrays
 */
Array operator+(const Array &a1, const Array& a2)
{
	Array ret(a1);
	ret.add(a2);
	return ret;
}



} // EOF namespace ppl7