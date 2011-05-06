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
#ifdef HAVE_WIDEC_H
#include <widec.h>
#endif

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
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include "ppl7.h"

namespace ppl7 {

/*!\class Variant
 * \ingroup PPLGroupDataTypes
 * \brief Basisklasse für alle weiteren Datentypen
 *
 * \desc
 * Diese Klasse wird als Basisklasse für alle weiteren Datentypen verwendet. Als einzige
 * Information speichert sie den Typ der Variablen. Dieser wird von allen
 * abgeleiteten Klassen mit einem Wert belegt, der aussagt, um welchen Datentyp es sich handelt.
 * Dazu wurde die Klasse ebenfalls um die Enumeration Variant::Type erweitert.
 *
 * \see
 * - String
 * - Array
 * - AssocArray
 * - ByteArray
 * - ByteArrayPtr
 * - DateTime
 *
 * \example
 * \code
void Machwas(const Variant &object)
{
	int t=object.DataType();
	if (t==Variant::BYTEARRAY) {
			const ppl7::ByteArray &bin= static_cast<const ppl7::ByteArray&>(object);  // Objekt zu ByteArray umwandeln
			printf ("Es ist ein ByteArray mit %i Bytes an Daten\n",bin.size());
			return;
	} else if (t==Variant::STRING) {
			const String &str= static_cast<const String&>(object);  // Objekt zu String umwandeln
			printf ("Es ist ein String mit folgendem Inhalt: %ls\n",(const wchar_t*)str);
			return;
	} else if (t==Variant::ARRAY) {
			const Array &array= static_cast<const Array&>(object);  // Objekt zu Array umwandeln
			printf ("Es ist ein Array mit %i Elementen\n",array.count());
			return;
	}
	printf ("Datentyp wird nicht unterstützt\n");
}
\endcode
 *
 */

/*!\enum Variant::Type
 * \brief Enumeration der verschiedenen Datenobjekte, die in der Library verwendet werden
 *
 * Enumeration der verschiedenen Datenobjekte, die in der Library verwendet werden
 */

/*!\var Variant::Type Variant::UNKNOWN
 * \brief Unbekannter Datentyp
 */

/*!\var Variant::Type Variant::STRING
 * \brief Klasse vom Typ String
 */

/*!\var Variant::Type Variant::BYTEARRAY
 * \brief Klasse vom Typ ByteArray
 */

/*!\var Variant::Type Variant::BYTEARRAYPTR
 * \brief Klasse vom Typ ByteArrayPtr
 */

/*!\var Variant::Type Variant::ASSOCARRAY
 * \brief Klasse vom Typ AssocArray
 */

/*!\var Variant::Type Variant::ARRAY
 * \brief Klasse vom Typ Array
 */

/*!\var Variant::Type Variant::DATETIME
 * \brief Klasse vom Typ DateTime
 */


/*!\var Variant::type
 * \brief Variable, zum Speichern des Datentyps
 */

/*!\brief Konstruktor der Klasse
 *
 * \desc
 * Der Konstruktor initialisiert den Typ der Klasse mit Variant::UNKNOWN.
 * Es ist aufgabe der abgeleiteten Klasse den korrekten Datentyp zu setzen.
 */
Variant::Variant()
{
	type=UNKNOWN;
}


/*!\brief Liefert den Datentyp des Objekts zurück
 *
 * \desc
 * Diese Funktion liefert den Datentyp des Objekts zurück.
 * @return ID aus der Enumeration Variant::Type
 */
int Variant::dataType() const
{
	return type;
}

/*!\brief Prüft auf einen bestimmten Datentyp
 *
 * \desc
 * Prüft, ob es sich bei diesem Objekt um den Datentyp \p type handelt.
 *
 * @param type Zu überprüfender Typ. Wert aus der Enumerationr Variant::Type
 * @return Liefert \c true zurück, wenn es sich um den angegebenen Datentyp \p type handelt,
 * sonst \c false.
 */
bool Variant::isType(int type) const
{
	if (this->type==type) return true;
	return false;
}

/*!\brief Prüft, ob es sich um den Datentyp String handelt
 *
 * \desc
 * Prüft, ob es sich um den Datentyp String handelt
 *
 * @return Liefert \c true zurück, wenn es such um den Datentyp String handelt, sonst \c false.
 */
bool Variant::isString() const
{
	if (type==STRING) return true;
	return false;
}

/*!\brief Prüft, ob es sich um den Datentyp Array handelt
 *
 * \desc
 * Prüft, ob es sich um den Datentyp Array handelt
 *
 * @return Liefert \c true zurück, wenn es such um den Datentyp Array handelt, sonst \c false.
 */
bool Variant::isArray() const
{
	if (type==ARRAY) return true;
	return false;
}

/*!\brief Prüft, ob es sich um den Datentyp AssocArray handelt
 *
 * \desc
 * Prüft, ob es sich um den Datentyp AssocArray handelt
 *
 * @return Liefert \c true zurück, wenn es such um den Datentyp AssocArray handelt, sonst \c false.
 */
bool Variant::isAssocArray() const
{
	if (type==ASSOCARRAY) return true;
	return false;
}

/*!\brief Prüft, ob es sich um den Datentyp ByteArray handelt
 *
 * \desc
 * Prüft, ob es sich um den Datentyp ByteArray handelt
 *
 * @return Liefert \c true zurück, wenn es such um den Datentyp ByteArray handelt, sonst \c false.
 */
bool Variant::isByteArray() const
{
	if (type==BYTEARRAY) return true;
	return false;
}

/*!\brief Prüft, ob es sich um den Datentyp ByteArrayPtr handelt
 *
 * \desc
 * Prüft, ob es sich um den Datentyp ByteArrayPtr handelt
 *
 * @return Liefert \c true zurück, wenn es such um den Datentyp ByteArrayPtr handelt, sonst \c false.
 */
bool Variant::isByteArrayPtr() const
{
	if (type==BYTEARRAYPTR) return true;
	if (type==BYTEARRAY) return true;
	return false;
}

/*!\brief Prüft, ob es sich um den Datentyp DateTime handelt
 *
 * \desc
 * Prüft, ob es sich um den Datentyp DateTime handelt
 *
 * @return Liefert \c true zurück, wenn es such um den Datentyp DateTime handelt, sonst \c false.
 */
bool Variant::isDateTime() const
{
	if (type==DATETIME) return true;
	return false;
}

/*!\brief Prüft, ob es sich um den Datentyp Pointer handelt
 *
 * \desc
 * Prüft, ob es sich um den Pointer DateTime handelt
 *
 * @return Liefert \c true zurück, wenn es such um den Pointer DateTime handelt, sonst \c false.
 */
bool Variant::isPointer() const
{
	if (type==POINTER) return true;
	return false;
}

/*!\brief Typkonvertierung zu: const String
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Variant zu einem String konvertiert, der nicht verändert werden
 * kann. Dies ist jedoch nur erfolgreich, wenn es sich bei diesem Datentyp auch um einen String handelt.
 * Ist dies nicht der Fall, wird eine Exception geworfen.
 *
 * @return Referenz auf String
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen String handelt.
 */
const String& Variant::toString() const
{
	if (type!=STRING) throw TypeConversionException();
	return static_cast<const String&>(*this);
}

/*!\brief Typkonvertierung zu: String
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Variant zu einem String konvertiert.
 * Dies ist jedoch nur erfolgreich, wenn es sich bei diesem Datentyp auch um einen String handelt.
 * Ist dies nicht der Fall, wird eine Exception geworfen.
 *
 * @return Referenz auf String
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen String handelt.
 */
String& Variant::toString()
{
	if (type!=STRING) throw TypeConversionException();
	return static_cast<String&>(*this);
}

/*!\brief Typkonvertierung zu: const Array
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Variant zu einem Array konvertiert, das nicht verändert werden
 * kann. Dies ist jedoch nur erfolgreich, wenn es sich bei diesem Datentyp auch um ein Array handelt.
 * Ist dies nicht der Fall, wird eine Exception geworfen.
 *
 * @return Referenz auf Array
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein Array handelt.
 */
const Array& Variant::toArray() const
{
	if (type!=ARRAY) throw TypeConversionException();
	return static_cast<const Array&>(*this);
}

/*!\brief Typkonvertierung zu: Array
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Variant zu einem Array konvertiert.
 * Dies ist jedoch nur erfolgreich, wenn es sich bei diesem Datentyp auch um ein Array handelt.
 * Ist dies nicht der Fall, wird eine Exception geworfen.
 *
 * @return Referenz auf Array
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein Array handelt.
 */
Array& Variant::toArray()
{
	if (type!=ARRAY) throw TypeConversionException();
	return static_cast<Array&>(*this);
}

/*!\brief Typkonvertierung zu: const AssocArray
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Variant zu einem AssocArray konvertiert, das nicht verändert werden kann.
 * Dies ist jedoch nur erfolgreich, wenn es sich bei diesem Datentyp auch um ein AssocArray handelt.
 * Ist dies nicht der Fall, wird eine Exception geworfen.
 *
 * @return Referenz auf AssocArray
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein AssocArray handelt.
 */
const AssocArray& Variant::toAssocArray() const
{
	if (type!=ASSOCARRAY) throw TypeConversionException();
	return static_cast<const AssocArray&>(*this);
}

/*!\brief Typkonvertierung zu: AssocArray
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Variant zu einem AssocArray konvertiert.
 * Dies ist jedoch nur erfolgreich, wenn es sich bei diesem Datentyp auch um ein AssocArray handelt.
 * Ist dies nicht der Fall, wird eine Exception geworfen.
 *
 * @return Referenz auf AssocArray
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein AssocArray handelt.
 */
AssocArray& Variant::toAssocArray()
{
	if (type!=ASSOCARRAY) throw TypeConversionException();
	return static_cast<AssocArray&>(*this);
}


/*!\brief Typkonvertierung zu: const ByteArray
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Variant zu einem ByteArray konvertiert, das nicht verändert werden kann.
 * Dies ist jedoch nur erfolgreich, wenn es sich bei diesem Datentyp auch um ein ByteArray handelt.
 * Ist dies nicht der Fall, wird eine Exception geworfen.
 *
 * @return Referenz auf ByteArray
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein ByteArray handelt.
 */
const ByteArray& Variant::toByteArray() const
{
	if (type!=BYTEARRAY) throw TypeConversionException();
	return static_cast<const ByteArray&>(*this);
}

/*!\brief Typkonvertierung zu: ByteArray
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Variant zu einem ByteArray konvertiert.
 * Dies ist jedoch nur erfolgreich, wenn es sich bei diesem Datentyp auch um ein ByteArray handelt.
 * Ist dies nicht der Fall, wird eine Exception geworfen.
 *
 * @return Referenz auf ByteArray
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein ByteArray handelt.
 */
ByteArray& Variant::toByteArray()
{
	if (type!=BYTEARRAY) throw TypeConversionException();
	return static_cast<ByteArray&>(*this);
}

/*!\brief Typkonvertierung zu: const ByteArrayPtr
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Variant zu einem ByteArrayPtr konvertiert, das nicht verändert werden kann.
 * Dies ist jedoch nur erfolgreich, wenn es sich bei diesem Datentyp auch um ein ByteArrayPtr oder
 * ByteArray handelt.
 * Ist dies nicht der Fall, wird eine Exception geworfen.
 *
 * @return Referenz auf ByteArrayPtr
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein ByteArrayPtr oder ByteArray handelt.
 */
const ByteArrayPtr& Variant::toByteArrayPtr() const
{
	if (type!=BYTEARRAYPTR && type!=BYTEARRAY) throw TypeConversionException();
	return static_cast<const ByteArrayPtr&>(*this);
}

/*!\brief Typkonvertierung zu: ByteArrayPtr
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Variant zu einem ByteArrayPtr konvertiert.
 * Dies ist jedoch nur erfolgreich, wenn es sich bei diesem Datentyp auch um ein ByteArrayPtr oder
 * ByteArray handelt.
 * Ist dies nicht der Fall, wird eine Exception geworfen.
 *
 * @return Referenz auf ByteArrayPtr
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein ByteArrayPtr oder ByteArray handelt.
 */
ByteArrayPtr& Variant::toByteArrayPtr()
{
	if (type!=BYTEARRAYPTR && type!=BYTEARRAY) throw TypeConversionException();
	return static_cast<ByteArrayPtr&>(*this);
}

/*!\brief Typkonvertierung zu: const DateTime
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Variant zu einem DateTime konvertiert, das nicht verändert werden kann.
 * Dies ist jedoch nur erfolgreich, wenn es sich bei diesem Datentyp auch um ein DateTime handelt.
 * Ist dies nicht der Fall, wird eine Exception geworfen.
 *
 * @return Referenz auf DateTime
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein DateTime handelt.
 */
const DateTime& Variant::toDateTime() const
{
	if (type!=DATETIME) throw TypeConversionException();
	return static_cast<const DateTime&>(*this);
}

/*!\brief Typkonvertierung zu: DateTime
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Variant zu einem DateTime konvertiert.
 * Dies ist jedoch nur erfolgreich, wenn es sich bei diesem Datentyp auch um ein DateTime handelt.
 * Ist dies nicht der Fall, wird eine Exception geworfen.
 *
 * @return Referenz auf ByteArrayPtr
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein DateTime handelt.
 */
DateTime& Variant::toDateTime()
{
	if (type!=DATETIME) throw TypeConversionException();
	return static_cast<DateTime&>(*this);
}

/*!\brief Typkonvertierung zu: const Pointer
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Variant zu einem Pointer konvertiert, das nicht verändert werden kann.
 * Dies ist jedoch nur erfolgreich, wenn es sich bei diesem Datentyp auch um ein Pointer handelt.
 * Ist dies nicht der Fall, wird eine Exception geworfen.
 *
 * @return Referenz auf DateTime
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein Pointer handelt.
 */
const Pointer& Variant::toPointer() const
{
	if (type!=POINTER) throw TypeConversionException();
	return static_cast<const Pointer&>(*this);
}

/*!\brief Typkonvertierung zu: Pointer
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Variant zu einem Pointer konvertiert.
 * Dies ist jedoch nur erfolgreich, wenn es sich bei diesem Datentyp auch um ein Pointer handelt.
 * Ist dies nicht der Fall, wird eine Exception geworfen.
 *
 * @return Referenz auf ByteArrayPtr
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein Pointer handelt.
 */
Pointer& Variant::toPointer()
{
	if (type!=POINTER) throw TypeConversionException();
	return static_cast<Pointer&>(*this);
}


} // EOF namespace ppl7


