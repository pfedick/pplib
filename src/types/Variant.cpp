/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 *
 *******************************************************************************
 * Copyright (c) 2015, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
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

#include <ppl7/types/variant.h>
#include <ppl7/types/pointer.h>
#include <ppl7/types/bytearrayptr.h>
#include <ppl7/types/bytearray.h>
#include <ppl7/types/string.h>
#include <ppl7/types/widestring.h>
#include <ppl7/types/array.h>
#include <ppl7/types/assocarray.h>
#include <ppl7/types/datetime.h>
#include <ppl7/exceptions.h>

namespace ppl7
{

Variant::Variant()
{
    value = nullptr;
    t = TYPE_UNKNOWN;
}

Variant::~Variant()
{
    clear();
}

Variant::Variant(const Variant& other)
{
    this->value = nullptr;
    t = TYPE_UNKNOWN;
    set(other);
}

Variant::Variant(Variant&& other)
{
    value = other.value;
    t = other.t;
    other.value = nullptr;
    other.t = TYPE_UNKNOWN;
}

Variant::Variant(const String& value)
{
    this->value = nullptr;
    t = TYPE_UNKNOWN;
    set(value);
}

Variant::Variant(const WideString& value)
{
    this->value = nullptr;
    t = TYPE_UNKNOWN;
    set(value);
}

Variant::Variant(const Array& value)
{
    this->value = nullptr;
    t = TYPE_UNKNOWN;
    set(value);
}

Variant::Variant(const AssocArray& value)
{
    this->value = nullptr;
    t = TYPE_UNKNOWN;
    set(value);
}

Variant::Variant(const ByteArray& value)
{
    this->value = nullptr;
    t = TYPE_UNKNOWN;
    set(value);
}

Variant::Variant(const ByteArrayPtr& value)
{
    this->value = nullptr;
    t = TYPE_UNKNOWN;
    set(value);
}

Variant::Variant(const DateTime& value)
{
    this->value = nullptr;
    t = TYPE_UNKNOWN;
    set(value);
}

Variant::Variant(const Pointer& value)
{
    this->value = nullptr;
    t = TYPE_UNKNOWN;
    set(value);
}

void Variant::clear()
{
    if (!value) return;
    switch (t) {
    case TYPE_STRING:
        delete (static_cast<String*>(value));
        break;
    case TYPE_ASSOCARRAY:
        delete (static_cast<AssocArray*>(value));
        break;
    case TYPE_BYTEARRAY:
        delete (static_cast<ByteArray*>(value));
        break;
    case TYPE_POINTER:
        delete (static_cast<Pointer*>(value));
        break;
    case TYPE_WIDESTRING:
        delete (static_cast<WideString*>(value));
        break;
    case TYPE_ARRAY:
        delete (static_cast<Array*>(value));
        break;
    case TYPE_DATETIME:
        delete (static_cast<DateTime*>(value));
        break;
    case TYPE_BYTEARRAYPTR:
        delete (static_cast<ByteArrayPtr*>(value));
        break;
    default:
        break;
    }
    value = nullptr;
    t = TYPE_UNKNOWN;
}

void Variant::set(const Variant& value)
{
    clear();
    if (!value.value) return;
    switch (value.t) {
    case TYPE_STRING:
        this->value = new String(*static_cast<String*>(value.value));
        t = TYPE_STRING;
        break;
    case TYPE_ASSOCARRAY:
        this->value = new AssocArray(*static_cast<AssocArray*>(value.value));
        t = TYPE_ASSOCARRAY;
        break;
    case TYPE_BYTEARRAY:
        this->value = new ByteArray(*static_cast<ByteArray*>(value.value));
        t = TYPE_BYTEARRAY;
        break;
    case TYPE_POINTER:
        this->value = new Pointer(*static_cast<Pointer*>(value.value));
        t = TYPE_POINTER;
        break;
    case TYPE_WIDESTRING:
        this->value = new WideString(*static_cast<WideString*>(value.value));
        t = TYPE_WIDESTRING;
        break;
    case TYPE_ARRAY:
        this->value = new Array(*static_cast<Array*>(value.value));
        t = TYPE_ARRAY;
        break;
    case TYPE_DATETIME:
        this->value = new DateTime(*static_cast<DateTime*>(value.value));
        t = TYPE_DATETIME;
        break;
    case TYPE_BYTEARRAYPTR:
        this->value = new ByteArrayPtr(*static_cast<ByteArrayPtr*>(value.value));
        t = TYPE_BYTEARRAYPTR;
        break;
    default:
        break;
    }
}

void Variant::set(const String& value)
{
    clear();
    this->value = new String(value);
    t = TYPE_STRING;
}

void Variant::set(const WideString& value)
{
    clear();
    this->value = new WideString(value);
    t = TYPE_WIDESTRING;
}

void Variant::set(const Array& value)
{
    clear();
    this->value = new Array(value);
    t = TYPE_ARRAY;
}

void Variant::set(const AssocArray& value)
{
    clear();
    this->value = new AssocArray(value);
    t = TYPE_ASSOCARRAY;
}

void Variant::set(const ByteArray& value)
{
    clear();
    this->value = new ByteArray(value);
    t = TYPE_BYTEARRAY;
}

void Variant::set(const ByteArrayPtr& value)
{
    clear();
    this->value = new ByteArrayPtr(value);
    t = TYPE_BYTEARRAYPTR;
}

void Variant::set(const DateTime& value)
{
    clear();
    this->value = new DateTime(value);
    t = TYPE_DATETIME;
}

void Variant::set(const Pointer& value)
{
    clear();
    this->value = new Pointer(value);
    t = TYPE_POINTER;
}

bool Variant::isType(DataType type) const
{
    if (this->t == type) return true;
    return false;
}

bool Variant::isString() const
{
    if (t == TYPE_STRING) return true;
    return false;
}

bool Variant::isWideString() const
{
    if (t == TYPE_WIDESTRING) return true;
    return false;
}

bool Variant::isArray() const
{
    if (t == TYPE_ARRAY) return true;
    return false;
}

bool Variant::isAssocArray() const
{
    if (t == TYPE_ASSOCARRAY) return true;
    return false;
}

bool Variant::isByteArray() const
{
    if (t == TYPE_BYTEARRAY) return true;
    return false;
}

bool Variant::isByteArrayPtr() const
{
    if (t == TYPE_BYTEARRAYPTR) return true;
    if (t == TYPE_BYTEARRAY) return true;
    return false;
}

bool Variant::isDateTime() const
{
    if (t == TYPE_DATETIME) return true;
    return false;
}

bool Variant::isPointer() const
{
    if (t == TYPE_POINTER) return true;
    return false;
}

/*!\brief Typkonvertierung zu: const String
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
 * String zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf String
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen String handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
const String& Variant::toString() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_STRING) throw TypeConversionException();
    return *static_cast<String*>(value);
}

/*!\brief Typkonvertierung zu: String
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
 * String zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf String
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen String handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
String& Variant::toString()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_STRING) throw TypeConversionException();
    return *static_cast<String*>(value);
}

/*!\brief Typkonvertierung zu: const WideString
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
 * WideString zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf WideString
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen WideString handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
const WideString& Variant::toWideString() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_WIDESTRING) throw TypeConversionException();
    return *static_cast<WideString*>(value);
}

/*!\brief Typkonvertierung zu: WideString
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
 * WideString zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf WideString
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen WideString handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
WideString& Variant::toWideString()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_WIDESTRING) throw TypeConversionException();
    return *static_cast<WideString*>(value);
}

/*!\brief Typkonvertierung zu: const Array
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf das gespeicherten
 * Array zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf Array
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein Array handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
const Array& Variant::toArray() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_ARRAY) throw TypeConversionException();
    return *static_cast<Array*>(value);
}

/*!\brief Typkonvertierung zu: Array
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine Referenz auf das gespeicherten
 * Array zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf Array
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein Array handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
Array& Variant::toArray()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_ARRAY) throw TypeConversionException();
    return *static_cast<Array*>(value);
}

/*!\brief Typkonvertierung zu: const AssocArray
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf das gespeicherten
 * AssocArray zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf AssocArray
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein AssocArray handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
const AssocArray& Variant::toAssocArray() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_ASSOCARRAY) throw TypeConversionException();
    return *static_cast<AssocArray*>(value);
}

/*!\brief Typkonvertierung zu: AssocArray
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine Referenz auf das gespeicherten
 * AssocArray zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf AssocArray
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein AssocArray handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
AssocArray& Variant::toAssocArray()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_ASSOCARRAY) throw TypeConversionException();
    return *static_cast<AssocArray*>(value);
}

/*!\brief Typkonvertierung zu: const ByteArray
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf das gespeicherten
 * ByteArray zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf ByteArray
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein ByteArray handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
const ByteArray& Variant::toByteArray() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_BYTEARRAY) throw TypeConversionException();
    return *static_cast<ByteArray*>(value);
}

/*!\brief Typkonvertierung zu: ByteArray
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine Referenz auf das gespeicherten
 * ByteArray zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf ByteArray
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um ein ByteArray handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
ByteArray& Variant::toByteArray()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_BYTEARRAY) throw TypeConversionException();
    return *static_cast<ByteArray*>(value);
}

/*!\brief Typkonvertierung zu: const ByteArrayPtr
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
 * ByteArrayPtr zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf ByteArrayPtr
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen ByteArrayPtr handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
const ByteArrayPtr& Variant::toByteArrayPtr() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_BYTEARRAYPTR && t != TYPE_BYTEARRAY) throw TypeConversionException();
    return *static_cast<ByteArrayPtr*>(value);
}

/*!\brief Typkonvertierung zu: ByteArrayPtr
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
 * ByteArrayPtr zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf ByteArrayPtr
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen ByteArrayPtr handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
ByteArrayPtr& Variant::toByteArrayPtr()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_BYTEARRAYPTR && t != TYPE_BYTEARRAY) throw TypeConversionException();
    return *static_cast<ByteArrayPtr*>(value);
}

/*!\brief Typkonvertierung zu: const DateTime
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
 * DateTime zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf DateTime
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen DateTime handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
const DateTime& Variant::toDateTime() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_DATETIME) throw TypeConversionException();
    return *static_cast<DateTime*>(value);
}

/*!\brief Typkonvertierung zu: DateTime
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
 * DateTime zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf DateTime
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen DateTime handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
DateTime& Variant::toDateTime()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_DATETIME) throw TypeConversionException();
    return *static_cast<DateTime*>(value);
}

/*!\brief Typkonvertierung zu: const Pointer
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
 * Pointer zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf Pointer
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Pointer handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
const Pointer& Variant::toPointer() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_POINTER) throw TypeConversionException();
    return *static_cast<Pointer*>(value);
}

/*!\brief Typkonvertierung zu: Pointer
 *
 * \desc
 * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
 * Pointer zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
 * wird eine Exception geworfen.
 *
 * @return Referenz auf Pointer
 * \exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Pointer handelt.
 * \exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
 */
Pointer& Variant::toPointer()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_POINTER) throw TypeConversionException();
    return *static_cast<Pointer*>(value);
}

Variant::operator String() const
{
    return toString();
}

Variant::operator WideString() const
{
    return toWideString();
}

Variant::operator Array() const
{
    return toArray();
}

Variant::operator AssocArray() const
{
    return toAssocArray();
}

Variant::operator ByteArray() const
{
    return toByteArray();
}

Variant::operator ByteArrayPtr() const
{
    return toByteArrayPtr();
}

Variant::operator DateTime() const
{
    return toDateTime();
}

Variant::operator Pointer() const
{
    return toPointer();
}

Variant& Variant::operator=(const Variant& other)
{
    set(other);
    return *this;
}

Variant& Variant::operator=(const String& other)
{
    set(other);
    return *this;
}

Variant& Variant::operator=(const WideString& other)
{
    set(other);
    return *this;
}

Variant& Variant::operator=(const Array& other)
{
    set(other);
    return *this;
}

Variant& Variant::operator=(const AssocArray& other)
{
    set(other);
    return *this;
}

Variant& Variant::operator=(const ByteArray& other)
{
    set(other);
    return *this;
}

Variant& Variant::operator=(const ByteArrayPtr& other)
{
    set(other);
    return *this;
}

Variant& Variant::operator=(const DateTime& other)
{
    set(other);
    return *this;
}

Variant& Variant::operator=(const Pointer& other)
{
    set(other);
    return *this;
}

bool Variant::operator==(const Variant& other) const
{
    if (t != other.t) return false;
    switch (t) {
    case TYPE_STRING:
        return (*static_cast<String*>(value) == *static_cast<String*>(other.value));
    case TYPE_ASSOCARRAY:
        return (*static_cast<AssocArray*>(value) == *static_cast<AssocArray*>(other.value));
    case TYPE_BYTEARRAY:
        return (*static_cast<ByteArray*>(value) == *static_cast<ByteArray*>(other.value));
    case TYPE_POINTER:
        return (*static_cast<Pointer*>(value) == *static_cast<Pointer*>(other.value));
    case TYPE_WIDESTRING:
        return (*static_cast<WideString*>(value) == *static_cast<WideString*>(other.value));
    case TYPE_ARRAY:
        return (*static_cast<Array*>(value) == *static_cast<Array*>(other.value));
    case TYPE_DATETIME:
        return (*static_cast<DateTime*>(value) == *static_cast<DateTime*>(other.value));
    case TYPE_BYTEARRAYPTR:
        return (*static_cast<ByteArrayPtr*>(value) == *static_cast<ByteArrayPtr*>(other.value));
    default:
        break;
    }
    return false;
}

bool Variant::operator!=(const Variant& other) const
{
    if (*this == other) return false;
    return true;
}

} // namespace ppl7
