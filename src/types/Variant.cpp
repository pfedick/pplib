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
    }
    value = nullptr;
    t = TYPE_UNKNOWN;
}

Variant& Variant::set(const Variant& value)
{
    clear();
    if (!value.value) return *this;
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
    case TYPE_DATE:
        this->value = new Date(*static_cast<Date*>(value.value));
        t = TYPE_DATE;
        break;
    case TYPE_TIME:
        this->value = new Time(*static_cast<Time*>(value.value));
        t = TYPE_TIME;
        break;
    case TYPE_TIMEDELTA:
        this->value = new TimeDelta(*static_cast<TimeDelta*>(value.value));
        t = TYPE_TIMEDELTA;
        break;
    case TYPE_TIMEZONE:
        this->value = new TimeZone(*static_cast<TimeZone*>(value.value));
        t = TYPE_TIMEZONE;
        break;
    }
    return *this;
}

Variant& Variant::set(Variant&& value)
{
    clear();
    this->value = value.value;
    t = value.t;
    value.value = nullptr;
    value.t = TYPE_UNKNOWN;
    return *this;
}

Variant& Variant::set(const String& value)
{
    clear();
    this->value = new String(value);
    t = TYPE_STRING;
    return *this;
}

Variant& Variant::set(String&& value)
{
    clear();
    this->value = new String(std::move(value));
    t = TYPE_STRING;
    return *this;
}

Variant& Variant::set(const WideString& value)
{
    clear();
    this->value = new WideString(value);
    t = TYPE_WIDESTRING;
    return *this;
}

Variant& Variant::set(WideString&& value)
{
    clear();
    this->value = new WideString(std::move(value));
    t = TYPE_WIDESTRING;
    return *this;
}

Variant& Variant::set(const Array& value)
{
    clear();
    this->value = new Array(value);
    t = TYPE_ARRAY;
    return *this;
}

Variant& Variant::set(Array&& value)
{
    clear();
    this->value = new Array(std::move(value));
    t = TYPE_ARRAY;
    return *this;
}

Variant& Variant::set(const AssocArray& value)
{
    clear();
    this->value = new AssocArray(value);
    t = TYPE_ASSOCARRAY;
    return *this;
}

Variant& Variant::set(AssocArray&& value)
{
    clear();
    this->value = new AssocArray(std::move(value));
    t = TYPE_ASSOCARRAY;
    return *this;
}

Variant& Variant::set(const ByteArray& value)
{
    clear();
    this->value = new ByteArray(value);
    t = TYPE_BYTEARRAY;
    return *this;
}

Variant& Variant::set(ByteArray&& value)
{
    clear();
    this->value = new ByteArray(std::move(value));
    t = TYPE_BYTEARRAY;
    return *this;
}

Variant& Variant::set(const ByteArrayPtr& value)
{
    clear();
    this->value = new ByteArrayPtr(value);
    t = TYPE_BYTEARRAYPTR;
    return *this;
}

Variant& Variant::set(ByteArrayPtr&& value)
{
    clear();
    this->value = new ByteArrayPtr(std::move(value));
    t = TYPE_BYTEARRAYPTR;
    return *this;
}

Variant& Variant::set(const DateTime& value)
{
    clear();
    this->value = new DateTime(value);
    t = TYPE_DATETIME;
    return *this;
}

Variant& Variant::set(DateTime&& value)
{
    clear();
    this->value = new DateTime(std::move(value));
    t = TYPE_DATETIME;
    return *this;
}

Variant& Variant::set(const Date& value)
{
    clear();
    this->value = new Date(value);
    t = TYPE_DATE;
    return *this;
}

Variant& Variant::set(Date&& value)
{
    clear();
    this->value = new Date(std::move(value));
    t = TYPE_DATE;
    return *this;
}

Variant& Variant::set(const Time& value)
{
    clear();
    this->value = new Time(value);
    t = TYPE_TIME;
    return *this;
}

Variant& Variant::set(Time&& value)
{
    clear();
    this->value = new Time(std::move(value));
    t = TYPE_TIME;
    return *this;
}

Variant& Variant::set(const TimeDelta& value)
{
    clear();
    this->value = new TimeDelta(value);
    t = TYPE_TIMEDELTA;
    return *this;
}

Variant& Variant::set(TimeDelta&& value)
{
    clear();
    this->value = new TimeDelta(std::move(value));
    t = TYPE_TIMEDELTA;
    return *this;
}

Variant& Variant::set(const TimeZone& value)
{
    clear();
    this->value = new TimeZone(value);
    t = TYPE_TIMEZONE;
    return *this;
}

Variant& Variant::set(TimeZone&& value)
{
    clear();
    this->value = new TimeZone(std::move(value));
    t = TYPE_TIMEZONE;
    return *this;
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
    }
    // es bleibt nur noch TYPE_UNKNOWN übrig, als ein leerer Variant
    return true;
}

bool Variant::operator!=(const Variant& other) const
{
    if (*this == other) return false;
    return true;
}

} // namespace pplib
