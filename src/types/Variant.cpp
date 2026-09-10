/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include "pplib/types/date.h"
#include "pplib/types/time.h"
#include <pplib/types/variant.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/array.h>
#include <pplib/types/assocarray.h>
#include <pplib/types/datetime.h>
#include <pplib/exceptions.h>

namespace pplib
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

Variant::Variant(Variant&& other) noexcept
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

Variant::Variant(const Date& value)
{
    this->value = nullptr;
    t = TYPE_UNKNOWN;
    set(value);
}

Variant::Variant(const Time& value)
{
    this->value = nullptr;
    t = TYPE_UNKNOWN;
    set(value);
}

Variant::Variant(const TimeDelta& value)
{
    this->value = nullptr;
    t = TYPE_UNKNOWN;
    set(value);
}

Variant::Variant(const TimeZone& value)
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
    case TYPE_DATE:
        delete (static_cast<Date*>(value));
        break;
    case TYPE_TIME:
        delete (static_cast<Time*>(value));
        break;
    case TYPE_TIMEDELTA:
        delete (static_cast<TimeDelta*>(value));
        break;
    case TYPE_TIMEZONE:
        delete (static_cast<TimeZone*>(value));
        break;
    default:
        break;
    }
    value = nullptr;
    t = TYPE_UNKNOWN;
}

Variant& Variant::set(const Variant& value)
{
    // Self-Assignment: clear() würde den eigenen Inhalt löschen, bevor kopiert wird
    if (this == &value) return *this;
    if (!value.value) {
        clear();
        return *this;
    }
    // Neues Objekt zuerst konstruieren (wirft ggf.), alten Wert erst danach freigeben.
    // Damit bleibt bei Aliasing (value referenziert den eigenen Inhalt) alles gültig.
    switch (value.t) {
    case TYPE_STRING:
        return setCopy<String, TYPE_STRING>(*static_cast<String*>(value.value));
    case TYPE_ASSOCARRAY:
        return setCopy<AssocArray, TYPE_ASSOCARRAY>(*static_cast<AssocArray*>(value.value));
    case TYPE_BYTEARRAY:
        return setCopy<ByteArray, TYPE_BYTEARRAY>(*static_cast<ByteArray*>(value.value));
    case TYPE_WIDESTRING:
        return setCopy<WideString, TYPE_WIDESTRING>(*static_cast<WideString*>(value.value));
    case TYPE_ARRAY:
        return setCopy<Array, TYPE_ARRAY>(*static_cast<Array*>(value.value));
    case TYPE_DATETIME:
        return setCopy<DateTime, TYPE_DATETIME>(*static_cast<DateTime*>(value.value));
    case TYPE_BYTEARRAYPTR:
        return setCopy<ByteArrayPtr, TYPE_BYTEARRAYPTR>(*static_cast<ByteArrayPtr*>(value.value));
    case TYPE_DATE:
        return setCopy<Date, TYPE_DATE>(*static_cast<Date*>(value.value));
    case TYPE_TIME:
        return setCopy<Time, TYPE_TIME>(*static_cast<Time*>(value.value));
    case TYPE_TIMEDELTA:
        return setCopy<TimeDelta, TYPE_TIMEDELTA>(*static_cast<TimeDelta*>(value.value));
    case TYPE_TIMEZONE:
        return setCopy<TimeZone, TYPE_TIMEZONE>(*static_cast<TimeZone*>(value.value));
    default:
        break;
    }
    clear();
    return *this;
}

Variant& Variant::set(Variant&& value)
{
    if (this == &value) return *this; // Self-Move: nichts zu tun
    clear();
    this->value = value.value;
    t = value.t;
    value.value = nullptr;
    value.t = TYPE_UNKNOWN;
    return *this;
}

Variant& Variant::set(const String& value)
{
    return setCopy<String, TYPE_STRING>(value);
}

Variant& Variant::set(String&& value)
{
    return setMove<String, TYPE_STRING>(std::move(value));
}

Variant& Variant::set(const WideString& value)
{
    return setCopy<WideString, TYPE_WIDESTRING>(value);
}

Variant& Variant::set(WideString&& value)
{
    return setMove<WideString, TYPE_WIDESTRING>(std::move(value));
}

Variant& Variant::set(const Array& value)
{
    return setCopy<Array, TYPE_ARRAY>(value);
}

Variant& Variant::set(Array&& value)
{
    return setMove<Array, TYPE_ARRAY>(std::move(value));
}

Variant& Variant::set(const AssocArray& value)
{
    return setCopy<AssocArray, TYPE_ASSOCARRAY>(value);
}

Variant& Variant::set(AssocArray&& value)
{
    return setMove<AssocArray, TYPE_ASSOCARRAY>(std::move(value));
}

Variant& Variant::set(const ByteArray& value)
{
    return setCopy<ByteArray, TYPE_BYTEARRAY>(value);
}

Variant& Variant::set(ByteArray&& value)
{
    return setMove<ByteArray, TYPE_BYTEARRAY>(std::move(value));
}

Variant& Variant::set(const ByteArrayPtr& value)
{
    return setCopy<ByteArrayPtr, TYPE_BYTEARRAYPTR>(value);
}

Variant& Variant::set(ByteArrayPtr&& value)
{
    return setMove<ByteArrayPtr, TYPE_BYTEARRAYPTR>(std::move(value));
}

Variant& Variant::set(const DateTime& value)
{
    return setCopy<DateTime, TYPE_DATETIME>(value);
}

Variant& Variant::set(DateTime&& value)
{
    return setMove<DateTime, TYPE_DATETIME>(std::move(value));
}

Variant& Variant::set(const Date& value)
{
    return setCopy<Date, TYPE_DATE>(value);
}

Variant& Variant::set(Date&& value)
{
    return setMove<Date, TYPE_DATE>(std::move(value));
}

Variant& Variant::set(const Time& value)
{
    return setCopy<Time, TYPE_TIME>(value);
}

Variant& Variant::set(Time&& value)
{
    return setMove<Time, TYPE_TIME>(std::move(value));
}

Variant& Variant::set(const TimeDelta& value)
{
    return setCopy<TimeDelta, TYPE_TIMEDELTA>(value);
}

Variant& Variant::set(TimeDelta&& value)
{
    return setMove<TimeDelta, TYPE_TIMEDELTA>(std::move(value));
}

Variant& Variant::set(const TimeZone& value)
{
    return setCopy<TimeZone, TYPE_TIMEZONE>(value);
}

Variant& Variant::set(TimeZone&& value)
{
    return setMove<TimeZone, TYPE_TIMEZONE>(std::move(value));
}

const String& Variant::toString() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_STRING) throw TypeConversionException();
    return *static_cast<String*>(value);
}

String& Variant::toString()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_STRING) throw TypeConversionException();
    return *static_cast<String*>(value);
}

const WideString& Variant::toWideString() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_WIDESTRING) throw TypeConversionException();
    return *static_cast<WideString*>(value);
}

WideString& Variant::toWideString()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_WIDESTRING) throw TypeConversionException();
    return *static_cast<WideString*>(value);
}

const Array& Variant::toArray() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_ARRAY) throw TypeConversionException();
    return *static_cast<Array*>(value);
}

Array& Variant::toArray()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_ARRAY) throw TypeConversionException();
    return *static_cast<Array*>(value);
}

const AssocArray& Variant::toAssocArray() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_ASSOCARRAY) throw TypeConversionException();
    return *static_cast<AssocArray*>(value);
}

AssocArray& Variant::toAssocArray()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_ASSOCARRAY) throw TypeConversionException();
    return *static_cast<AssocArray*>(value);
}

const ByteArray& Variant::toByteArray() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_BYTEARRAY) throw TypeConversionException();
    return *static_cast<ByteArray*>(value);
}

ByteArray& Variant::toByteArray()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_BYTEARRAY) throw TypeConversionException();
    return *static_cast<ByteArray*>(value);
}

const ByteArrayPtr& Variant::toByteArrayPtr() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_BYTEARRAYPTR && t != TYPE_BYTEARRAY) throw TypeConversionException();
    return *static_cast<ByteArrayPtr*>(value);
}

ByteArrayPtr& Variant::toByteArrayPtr()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_BYTEARRAYPTR && t != TYPE_BYTEARRAY) throw TypeConversionException();
    return *static_cast<ByteArrayPtr*>(value);
}

const DateTime& Variant::toDateTime() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_DATETIME) throw TypeConversionException();
    return *static_cast<DateTime*>(value);
}

DateTime& Variant::toDateTime()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_DATETIME) throw TypeConversionException();
    return *static_cast<DateTime*>(value);
}

const Date& Variant::toDate() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_DATE) throw TypeConversionException();
    return *static_cast<Date*>(value);
}

Date& Variant::toDate()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_DATE) throw TypeConversionException();
    return *static_cast<Date*>(value);
}

const Time& Variant::toTime() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_TIME) throw TypeConversionException();
    return *static_cast<Time*>(value);
}

Time& Variant::toTime()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_TIME) throw TypeConversionException();
    return *static_cast<Time*>(value);
}

const TimeDelta& Variant::toTimeDelta() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_TIMEDELTA) throw TypeConversionException();
    return *static_cast<TimeDelta*>(value);
}

TimeDelta& Variant::toTimeDelta()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_TIMEDELTA) throw TypeConversionException();
    return *static_cast<TimeDelta*>(value);
}

const TimeZone& Variant::toTimeZone() const
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_TIMEZONE) throw TypeConversionException();
    return *static_cast<TimeZone*>(value);
}

TimeZone& Variant::toTimeZone()
{
    if (!value) throw EmptyDataException();
    if (t != TYPE_TIMEZONE) throw TypeConversionException();
    return *static_cast<TimeZone*>(value);
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
    case TYPE_WIDESTRING:
        return (*static_cast<WideString*>(value) == *static_cast<WideString*>(other.value));
    case TYPE_ARRAY:
        return (*static_cast<Array*>(value) == *static_cast<Array*>(other.value));
    case TYPE_DATETIME:
        return (*static_cast<DateTime*>(value) == *static_cast<DateTime*>(other.value));
    case TYPE_BYTEARRAYPTR:
        return (*static_cast<ByteArrayPtr*>(value) == *static_cast<ByteArrayPtr*>(other.value));
    case TYPE_DATE:
        return (*static_cast<Date*>(value) == *static_cast<Date*>(other.value));
    case TYPE_TIME:
        return (*static_cast<Time*>(value) == *static_cast<Time*>(other.value));
    case TYPE_TIMEDELTA:
        return (*static_cast<TimeDelta*>(value) == *static_cast<TimeDelta*>(other.value));
    case TYPE_TIMEZONE:
        return (*static_cast<TimeZone*>(value) == *static_cast<TimeZone*>(other.value));
    default:
        break;
    }
    // es bleibt nur noch TYPE_UNKNOWN übrig, also ein leerer Variant
    return true;
}

bool Variant::operator!=(const Variant& other) const
{
    if (*this == other) return false;
    return true;
}

} // namespace pplib
