/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
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

#ifndef PPL7_TYPES_VARIANT_H_
#define PPL7_TYPES_VARIANT_H_
namespace ppl7
{

class String;
class WideString;
class Array;
class AssocArray;
class ByteArray;
class ByteArrayPtr;
class DateTime;
class Pointer;

class Variant
{
public:
    enum DataType
    {
        TYPE_UNKNOWN = 0,
        TYPE_STRING = 4,
        TYPE_ASSOCARRAY = 5,
        TYPE_BYTEARRAY = 6,
        TYPE_POINTER = 7,
        TYPE_WIDESTRING = 8,
        TYPE_ARRAY = 9,
        TYPE_DATETIME = 10,
        TYPE_BYTEARRAYPTR = 12
    };

private:
    void* value;
    DataType t;

public:
    Variant();
    ~Variant();
    Variant(const Variant& value);
    Variant(const String& value);
    Variant(const WideString& value);
    Variant(const Array& value);
    Variant(const AssocArray& value);
    Variant(const ByteArray& value);
    Variant(const ByteArrayPtr& value);
    Variant(const DateTime& value);
    Variant(const Pointer& value);

    void clear();
    void set(const Variant& value);
    void set(const String& value);
    void set(const WideString& value);
    void set(const Array& value);
    void set(const AssocArray& value);
    void set(const ByteArray& value);
    void set(const ByteArrayPtr& value);
    void set(const DateTime& value);
    void set(const Pointer& value);

    DataType type() const;
    bool isType(DataType type) const;
    bool isString() const;
    bool isWideString() const;
    bool isArray() const;
    bool isAssocArray() const;
    bool isByteArray() const;
    bool isByteArrayPtr() const;
    bool isDateTime() const;
    bool isPointer() const;

    const String& toString() const;
    String& toString();
    const WideString& toWideString() const;
    WideString& toWideString();
    const Array& toArray() const;
    Array& toArray();
    const AssocArray& toAssocArray() const;
    AssocArray& toAssocArray();
    const ByteArray& toByteArray() const;
    ByteArray& toByteArray();
    const ByteArrayPtr& toByteArrayPtr() const;
    ByteArrayPtr& toByteArrayPtr();
    const DateTime& toDateTime() const;
    DateTime& toDateTime();
    const Pointer& toPointer() const;
    Pointer& toPointer();

    operator String() const;
    operator WideString() const;
    operator Array() const;
    operator AssocArray() const;
    operator ByteArray() const;
    operator ByteArrayPtr() const;
    operator DateTime() const;
    operator Pointer() const;

    Variant& operator=(const Variant& other);
    Variant& operator=(const String& other);
    Variant& operator=(const WideString& other);
    Variant& operator=(const Array& other);
    Variant& operator=(const AssocArray& other);
    Variant& operator=(const ByteArray& other);
    Variant& operator=(const ByteArrayPtr& other);
    Variant& operator=(const DateTime& other);
    Variant& operator=(const Pointer& other);

    bool operator==(const Variant& other) const;
    bool operator!=(const Variant& other) const;
};

} // namespace ppl7

#endif // PPL7_TYPES_VARIANT_H_