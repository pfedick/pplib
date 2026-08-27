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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pplib/types/array.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/assocarray.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include "pplib/types/datetime.h"
#include "pplib/types/date.h"
#include "pplib/types/time.h"
#include "pplib/types/timedelta.h"
#include "pplib/types/timezone.h"
#include "pplib/types/variant.h"
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>
#include <pplib/core/iconv.h>

#include <config_pplib.h>

namespace pplib
{

AssocArray::AssocArray()
{
    maxint = 0;
}

AssocArray::AssocArray(const AssocArray& other)
{
    maxint = 0;
    add(other);
}

AssocArray::AssocArray(AssocArray&& other)
{
    maxint = other.maxint;
    Tree = std::move(other.Tree);
    other.maxint = 0;
    other.Tree.clear();
}

AssocArray::~AssocArray()
{
    clear();
}

void AssocArray::clear()
{
    iterator it;
    for (it = Tree.begin(); it != Tree.end(); ++it) {
        delete (*it).second;
    }
    Tree.clear();
    maxint = 0;
}

Variant* AssocArray::findInternal(const String& key) const
{
    // printf ("AssocArray::findInternal (key=%ls)\n",(const wchar_t*)key);
    Array tok(key, "/", 0, true);
    if (tok.count() == 0) throw InvalidKeyException(key);
    String firstkey = tok.shift();
    String rest = tok.implode("/");

    const_iterator it = Tree.find(firstkey);
    if (it == Tree.end()) return NULL;
    // Ist noch was im Pfad rest?
    if (tok.count() > 0) { // Ja, koennen wir iterieren?
        if (it->second->isAssocArray()) {
            return it->second->toAssocArray().findInternal(rest);
        } else {
            return NULL;
        }
    }
    // Der Value könnte ein leerer Variant sein (TYPE_UNKNOWN). Das ist ein Fall,
    // der bei createTree() vorkommen kann, wenn ein BadAlloc in set() geworfen wurde.
    if (it->second->type() == Variant::TYPE_UNKNOWN) return NULL;
    return it->second;
}

Variant* AssocArray::createTree(const String& key)
{
    // TODO: das sollte keine Exception werfen, sondern mit Returncode arbeiten!
    Array tok(key, "/", 0, true);
    if (tok.count() == 0) throw InvalidKeyException(key);
    String firstkey = tok.shift();
    String rest = tok.implode("/");
    // printf ("firstkey=%ls, rest=%ls\n",(const wchar_t *)firstkey,(const wchar_t *)rest);
    if (firstkey == "[]") {
        firstkey.setf("%llu", maxint);
        maxint++;
    }
    // Beginnt Firstkey mit einer Zahl?
    if (firstkey.isNumeric()) {
        uint64_t keyint = firstkey.toInt64();
        if (keyint >= maxint) maxint = keyint + 1;
        firstkey.setf("%llu", keyint);
    }

    iterator it = Tree.find(firstkey);
    if (it != Tree.end()) {
        // Ist noch was im Pfad rest?
        if (tok.count() > 0) { // Ja, koennen wir iterieren?
            if (it->second->isAssocArray() == false) {
                // Nein, wir loeschen daher diesen Zweig und machen ein Array draus
                it->second->set(pplib::AssocArray());
            }
            return it->second->toAssocArray().createTree(rest);
        }
        // Nein, wir haben die Zielposition gefunden
        it->second->clear();
        return it->second;
    }

    // Key ist nicht in diesem Array, wir legen ihn an

    // Ist noch was im Pfad rest?
    if (tok.count() > 0) { // Ja, wir erstellen ein Array und iterieren
        // printf ("Iteration\n");
        Variant* newnode = new Variant(pplib::AssocArray());
        Tree.insert(std::pair<String, Variant*>(firstkey, newnode));
        return newnode->toAssocArray().createTree(rest);
    } else {
        Variant* newnode = new Variant();
        Tree.insert(std::pair<String, Variant*>(firstkey, newnode));
        return newnode;
    }
}

size_t AssocArray::count(bool recursive) const
{
    if (!recursive) return Tree.size();
    const_iterator it;
    size_t num = Tree.size();
    for (it = Tree.begin(); it != Tree.end(); ++it) {
        if (it->second->isAssocArray()) num += it->second->toAssocArray().count(recursive);
    }
    return num;
}

size_t AssocArray::size() const
{
    return Tree.size();
}

size_t AssocArray::count(const String& key, bool recursive) const
{
    const Variant* p = findInternal(key);
    if (!p) return (size_t)0;
    if (p->isAssocArray()) return p->toAssocArray().count(recursive);
    return 1;
}

void AssocArray::setf(const String& key, const char* fmt, ...)
{
    String value;
    va_list args;
    va_start(args, fmt);
    value.vasprintf(fmt, args);
    va_end(args);
    createTree(key)->set(value);
}

void AssocArray::append(const String& key, const String& value, const String& concat)
{
    Variant* node = findInternal(key);
    if (!node) {
        set(key, value);
        return;
    }
    String& str = node->toString();
    if (concat.notEmpty()) str.append(concat);
    str.append(value);
}

void AssocArray::appendf(const String& key, const String& concat, const char* fmt, ...)
{
    String var;
    va_list args;
    va_start(args, fmt);
    var.vasprintf(fmt, args);
    va_end(args);
    append(key, var, concat);
}

void AssocArray::add(const AssocArray& other)
{
    if (this == &other) return; // Self-Reference, nichts zu tun
    const_iterator it;
    for (it = other.Tree.begin(); it != other.Tree.end(); ++it) {
        Variant* existing = findInternal(it->first);
        if (existing && existing->isAssocArray() && it->second->isAssocArray()) {
            existing->toAssocArray().add(it->second->toAssocArray());

        } else
            set(it->first, *it->second);
    }
    // if (other.maxint > maxint) maxint = other.maxint; // Tritt nicht ein, da set den maxint bereits aktualisiert
}

const Variant& AssocArray::get(const String& key, Variant::DataType type) const
{
    Variant* node = findInternal(key);
    if (!node) throw KeyNotFoundException(key);
    if (type != Variant::DataType::TYPE_UNKNOWN && !node->isType(type))
        throw TypeConversionException("%s is not of the requested type", (const char*)key);
    return (*node);
}

Variant& AssocArray::get(const String& key, Variant::DataType type)
{
    Variant* node = findInternal(key);
    if (!node) throw KeyNotFoundException(key);
    if (type != Variant::DataType::TYPE_UNKNOWN && !node->isType(type))
        throw TypeConversionException("%s is not of the requested type", (const char*)key);
    return (*node);
}

bool AssocArray::exists(const String& key) const
{
    return findInternal(key) != nullptr;
}

String& AssocArray::getString(const String& key)
{
    return get(key, Variant::DataType::TYPE_STRING).toString();
}

const String& AssocArray::getString(const String& key) const
{
    return get(key, Variant::DataType::TYPE_STRING).toString();
}

String AssocArray::getString(const String& key, const String& default_value) const
{
    Variant* node = findInternal(key);
    if (!node) return default_value;
    if (node->isString()) return node->toString();
    if (node->isWideString()) return String(node->toWideString());
    return default_value;
}

bool AssocArray::getBoolean(const String& key, bool default_value) const
{
    Variant* node = findInternal(key);
    if (!node) return default_value;
    if (node->isString()) return node->toString().isTrue();
    if (node->isWideString()) return node->toWideString().isTrue();
    return default_value;
}

int AssocArray::getInt(const String& key, int default_value) const
{
    Variant* node = findInternal(key);
    if (!node) return default_value;
    if (node->isString()) return node->toString().toInt();
    if (node->isWideString()) return node->toWideString().toInt();
    return default_value;
}

int64_t AssocArray::getInt64t(const String& key, int64_t default_value) const
{
    Variant* node = findInternal(key);
    if (!node) return default_value;
    if (node->isString()) return node->toString().toInt64();
    if (node->isWideString()) return node->toWideString().toInt64();
    return default_value;
}

bool AssocArray::isTrue(const String& key) const
{
    Variant* node = findInternal(key);
    if (!node) return false;
    if (node->isString()) return node->toString().isTrue();
    if (node->isWideString()) return node->toWideString().isTrue();
    return false;
}

AssocArray& AssocArray::getAssocArray(const String& key)
{
    return get(key, Variant::DataType::TYPE_ASSOCARRAY).toAssocArray();
}

const AssocArray& AssocArray::getAssocArray(const String& key) const
{
    return get(key, Variant::DataType::TYPE_ASSOCARRAY).toAssocArray();
}

Array& AssocArray::getArray(const String& key)
{
    return get(key, Variant::DataType::TYPE_ARRAY).toArray();
}

const Array& AssocArray::getArray(const String& key) const
{
    return get(key, Variant::DataType::TYPE_ARRAY).toArray();
}

void AssocArray::erase(const String& key)
{
    Array tok(key, "/", 0, true);
    if (tok.count() == 0) throw InvalidKeyException(key);
    String firstkey = tok.shift();
    String rest = tok.implode("/");
    iterator it = Tree.find(firstkey);
    if (it == Tree.end()) return; // nothing to do
    // Ist noch was im Pfad rest?
    if (tok.count() > 0) { // Ja, koennen wir iterieren?
        if (it->second->isAssocArray()) {
            it->second->toAssocArray().erase(rest);
            return;
        } else {
            return;
        }
    }
    delete it->second;
    Tree.erase(it);
}

void AssocArray::remove(const String& key)
{
    erase(key);
}

AssocArray::iterator AssocArray::begin()
{
    return Tree.begin();
}

AssocArray::const_iterator AssocArray::begin() const
{
    return Tree.begin();
}

AssocArray::iterator AssocArray::end()
{
    return Tree.end();
}

AssocArray::const_iterator AssocArray::end() const
{
    return Tree.end();
}

AssocArray::reverse_iterator AssocArray::rbegin()
{
    return Tree.rbegin();
}

AssocArray::const_reverse_iterator AssocArray::rbegin() const
{
    return Tree.rbegin();
}

AssocArray::reverse_iterator AssocArray::rend()
{
    return Tree.rend();
}

AssocArray::const_reverse_iterator AssocArray::rend() const
{
    return Tree.rend();
}

AssocArray::const_iterator AssocArray::cbegin() const
{
    return Tree.cbegin();
}

AssocArray::const_iterator AssocArray::cend() const
{
    return Tree.cend();
}

AssocArray::const_reverse_iterator AssocArray::crbegin() const
{
    return Tree.crbegin();
}

AssocArray::const_reverse_iterator AssocArray::crend() const
{
    return Tree.crend();
}

size_t AssocArray::binarySize() const
{
    return exportBinary(NULL, 0);
}

size_t AssocArray::exportBinary(void* buffer, size_t buffersize) const
{
    char* ptr = (char*)buffer;
    size_t p = 0;
    size_t vallen = 0;
    String key;
    ByteArray ba;
    if (!buffer) buffersize = 0;
    if (p + 8 < buffersize) memcpy(ptr, "PPL8ASOC", 8);
    p += 8;
    if (p + 1 < buffersize) PokeN8(ptr + p, 1); // Version
    p++;
    if (p + 8 < buffersize) PokeN64(ptr + p, maxint);
    p += 8;
    AssocArray::const_iterator it;
    for (it = Tree.begin(); it != Tree.end(); ++it) {
        const Variant* a = it->second;
        if (p < buffersize) {
            if (a->isByteArrayPtr())
                PokeN8(ptr + p, Variant::TYPE_BYTEARRAY);
            else
                PokeN8(ptr + p, a->type());
        }
        p++;
        key = it->first;
        size_t keylen = key.size();
        if (p + 4 < buffersize) PokeN16(ptr + p, (int)keylen);
        p += 2;
        if (p + keylen < buffersize) strncpy(ptr + p, (const char*)key, (int)keylen);
        p += keylen;
        if (a->isString()) {
            String string = a->toString();
            vallen = string.size();
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
            p += 4;
            if (p + vallen < buffersize) strncpy(ptr + p, (const char*)string, vallen);
            p += vallen;
        } else if (a->isWideString()) {
            String string(a->toWideString()); // Konvertierung in UTF-8
            vallen = string.size();
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
            p += 4;
            if (p + vallen < buffersize) strncpy(ptr + p, (const char*)string, vallen);
            p += vallen;

        } else if (a->isAssocArray()) {
            if (!buffer)
                p += a->toAssocArray().exportBinary(NULL, 0);
            else
                p += a->toAssocArray().exportBinary(ptr + p, buffersize - p);
        } else if (a->isArray()) {
            pplib::Array aaa(a->toArray());
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)aaa.size());
            p += 4;
            for (ssize_t i = 0; i < (ssize_t)aaa.size(); i++) {
                const String s = aaa.get(i);
                if (p + 4 < buffersize) PokeN32(ptr + p, (int)s.size());
                p += 4;
                vallen = s.size();
                if (p + vallen < buffersize) strncpy(ptr + p, (const char*)s, vallen);
                p += vallen;
            }
        } else if (a->isDateTime()) {
            const DateTime& dt = a->toDateTime();
            // DateTime könnte invalid sein
            if (dt.isEmpty()) {
                if (p + 4 < buffersize) PokeN32(ptr + p, 0);
                p += 4;
            } else {
                vallen = 10;                           // PPL8 speichert Microseconds in 8 und Zeitzone in 2 Bytes,
                vallen += dt.timeZone().name().size(); // plus die Länge des Zeitzonen-Namens, der aber leer sein kann.
                if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
                p += 4;
                if (p + vallen < buffersize) {
                    PokeN64(ptr + p, dt.toMicroseconds());
                    PokeN16(ptr + p + 8, dt.timeZone().offsetMinutes());
                    memcpy(ptr + p + 10, (const char*)dt.timeZone().name(), dt.timeZone().name().size());
                }
                p += vallen;
            }
        } else if (a->isByteArray() == true || a->isByteArrayPtr() == true) {
            vallen = a->toByteArrayPtr().size();
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
            p += 4;
            if (p + vallen < buffersize) memcpy(ptr + p, a->toByteArrayPtr().adr(), vallen);
            p += vallen;

        } else if (a->isDate() == true) {
            vallen = 4; // Date exportiert das Datum als 32Bit Integer (YYYYMMDD)
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
            p += 4;
            if (p + vallen < buffersize) {
                PokeN32(ptr + p, a->toDate().toInt());
            }
            p += vallen;
        } else if (a->isTime() == true) {
            vallen = 8; // Time exportiert die Zeit in Microseconds als 64Bit Integer
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
            p += 4;
            if (p + vallen < buffersize) {
                PokeN64(ptr + p, a->toTime().toMicroseconds());
            }
            p += vallen;
        } else if (a->isTimeDelta()) {
            vallen = 8; // TimeDelta exportiert die Zeit in Microseconds als 64Bit Integer
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
            p += 4;
            if (p + vallen < buffersize) {
                PokeN64(ptr + p, a->toTimeDelta().toMicroseconds());
            }
            p += vallen;
        } else if (a->isTimeZone()) { // keine vollständige Coverage hier erreichbar, da keine anderen Typen exportiert werden
            const TimeZone& tz = a->toTimeZone();
            vallen = 2;                 // TimeZone exportiert die OffsetMinutes als 16Bit Integer,
            vallen += tz.name().size(); // und den Namen als String, der aber leer sein kann
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
            p += 4;
            if (p + vallen < buffersize) {
                PokeN16(ptr + p, tz.offsetMinutes());
                memcpy(ptr + p + 2, (const char*)tz.name(), tz.name().size());
            }
            p += vallen;
        }
    }
    if (p < buffersize) PokeN8(ptr + p, 0);
    p++;
    if (buffersize == 0 || p <= buffersize) return p;
    throw ExportBufferToSmallException("%zd < %zd", buffersize, p);
}

void AssocArray::exportBinary(ByteArray& buffer) const
{
    buffer.free();
    size_t size = binarySize();
    buffer.malloc(size);
    exportBinary((void*)buffer.adr(), buffer.size());
}

ByteArray AssocArray::exportBinary() const
{
    ByteArray buffer;
    exportBinary(buffer);
    return buffer;
}

void AssocArray::importBinary(const ByteArrayPtr& bin)
{
    importBinary(bin.adr(), bin.size());
}

size_t AssocArray::importBinary(const void* buffer, size_t buffersize)
{
    if (!buffer) throw IllegalArgumentException();
    if (buffersize == 0) throw IllegalArgumentException();
    const char* ptr = (const char*)buffer;
    size_t p = 0;
    if (buffersize < 8 || strncmp((const char*)ptr, "PPL8ASOC", 8) != 0) {
        throw ImportFailedException("Not an PPL8 AssocArray binary export");
    }
    p += 8;
    if (p + 1 > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
    int version = PeekN8(ptr + p);
    if (version != 1) throw ImportFailedException("Invalid PPL8 AssocArray binary export version %d", version);
    p++;
    if (p + 8 > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
    maxint = PeekN64(ptr + p);
    p += 8;
    int type;
    size_t vallen, bytes;
    String key;
    while (p + 2 < buffersize && (type = PeekN8(ptr + p)) != 0) {
        p++;
        size_t keylen = PeekN16(ptr + p);
        p += 2;
        if (p + keylen > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
        key.set(ptr + p, keylen);
        p += keylen;
        switch (type) {
        case Variant::TYPE_STRING:
            if (p + 4 > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            vallen = PeekN32(ptr + p);
            p += 4;
            if (p + vallen > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            set(key, String((const char*)ptr + p, vallen));
            p += vallen;
            break;
        case Variant::TYPE_WIDESTRING:
            if (p + 4 > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            vallen = PeekN32(ptr + p);
            p += 4;
            if (p + vallen > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            set(key, WideString((const char*)ptr + p, vallen));
            p += vallen;
            break;
        case Variant::TYPE_ASSOCARRAY: {
            AssocArray na;
            bytes = na.importBinary(ptr + p, buffersize - p);
            p += bytes;
            set(key, na);

        } break;
        case Variant::TYPE_ARRAY: {
            if (p + 4 > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            size_t elements = PeekN32(ptr + p);
            p += 4;
            Array stringarray;
            stringarray.reserve(elements);
            for (size_t i = 0; i < elements; i++) {
                if (p + 4 > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
                vallen = PeekN32(ptr + p);
                p += 4;
                if (p + vallen > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
                String str(ptr + p, vallen);
                p += vallen;
                stringarray.add(str);
            }
            set(key, stringarray);
        } break;
        case Variant::TYPE_BYTEARRAY: {
            if (p + 4 > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            vallen = PeekN32(ptr + p);
            p += 4;
            if (p + vallen > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            ByteArray nb(ptr + p, vallen);
            set(key, nb);
            p += vallen;
        } break;
        case Variant::TYPE_DATETIME: {
            if (p + 4 > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            vallen = PeekN32(ptr + p);
            p += 4;
            DateTime dt;
            if (vallen >= 10) {
                if (p + vallen > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
                int64_t us = (int64_t)PeekN64(ptr + p);
                int16_t tz_offset = (int16_t)PeekN16(ptr + p + 8);
                dt.setMicroseconds(us, TimeZone(tz_offset));
                if (vallen > 10) { // Wir haben auch einen Zeitzonen-Namen
                    dt.timeZone().setName(String(ptr + p + 10, vallen - 10));
                }
            }
            // vallen könnte auch 0 sein, wenn das DateTime invalid ist
            p += vallen;
            set(key, dt);
        } break;
        case Variant::TYPE_DATE: {
            if (p + 4 > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            vallen = PeekN32(ptr + p);
            p += 4;
            if (p + vallen > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            set(key, Date::fromInt(PeekN32(ptr + p)));
            p += vallen;
        } break;
        case Variant::TYPE_TIME: {
            if (p + 4 > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            vallen = PeekN32(ptr + p);
            p += 4;
            if (p + vallen > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            set(key, Time::fromMicroseconds(PeekN64(ptr + p)));
            p += vallen;
        } break;
        case Variant::TYPE_TIMEDELTA: {
            if (p + 4 > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            vallen = PeekN32(ptr + p);
            p += 4;
            if (p + vallen > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            set(key, TimeDelta::fromMicroseconds((int64_t)PeekN64(ptr + p)));
            p += vallen;
        } break;
        case Variant::TYPE_TIMEZONE: {
            if (p + 4 > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            vallen = PeekN32(ptr + p);
            p += 4;
            if (p + vallen > buffersize) throw ImportFailedException("Invalid PPL8 AssocArray binary export");
            int16_t offset = (int16_t)PeekN16(ptr + p);
            String name;
            if (vallen > 2) {
                name.set((const char*)ptr + p + 2, vallen - 2);
            }
            set(key, TimeZone(offset, name));
            p += vallen;
        } break;
        default:
            throw ImportFailedException("unknown datatype in AssocArray binary export [type=%d, size=%zu]", type, vallen);
        };
    }
    p++;
    return p;
}

const Variant& AssocArray::operator[](const String& key) const
{
    return get(key);
}

Variant& AssocArray::operator[](const String& key)
{
    return get(key);
}

AssocArray& AssocArray::operator=(const AssocArray& other)
{
    if (this == &other) return *this;
    clear();
    add(other);
    maxint = other.maxint;
    return *this;
}

AssocArray& AssocArray::operator=(AssocArray&& other) noexcept
{
    if (this == &other) return *this;
    clear();
    Tree = std::move(other.Tree);
    maxint = other.maxint;
    return *this;
}

AssocArray& AssocArray::operator+=(const AssocArray& other)
{
    add(other);
    return *this;
}

bool AssocArray::operator==(const AssocArray& other) const
{
    ByteArray b1, b2;
    exportBinary(b1);
    other.exportBinary(b2);
    if (b1 == b2) return true;
    return false;
}

bool AssocArray::operator!=(const AssocArray& other) const
{
    if (*this == other) return false;
    return true;
}

AssocArray operator+(const AssocArray& a1, const AssocArray& a2)
{
    AssocArray a(a1);
    a.add(a2);
    return a;
}

void AssocArray::list(const String& prefix) const
{
    String key;
    String pre;
    if (prefix.notEmpty()) key = prefix + "/";

    const_iterator it;
    for (it = Tree.begin(); it != Tree.end(); ++it) {
        Variant* p = it->second;
        if (p->isString()) {
            PrintDebug("%s%s=%s\n", (const char*)key, (const char*)it->first, (const char*)p->toString().getPtr());
        } else if (p->isWideString()) {
            PrintDebug("%s%s=%ls\n", (const char*)key, (const char*)it->first, (const wchar_t*)p->toWideString().getPtr());
        } else if (p->isByteArray()) {
            PrintDebug("%s%s=ByteArray, %zu Bytes\n", (const char*)key, (const char*)it->first, p->toByteArray().size());
        } else if (p->isByteArrayPtr()) {
            PrintDebug("%s%s=ByteArrayPtr, %zu Bytes\n", (const char*)key, (const char*)it->first, p->toByteArrayPtr().size());
        } else if (p->isAssocArray()) {
            pre.setf("%s%s", (const char*)key, (const char*)it->first);
            p->toAssocArray().list(pre);
        } else if (p->isArray()) {
            const Array& a = (const Array&)*p;
            for (size_t i = 0; i < a.size(); i++) {
                PrintDebug("%s%s/Array(%zu)=%s\n", (const char*)key, (const char*)it->first, i, (const char*)a[i]);
            }
        } else if (p->isDateTime()) {
            PrintDebug("%s%s=DateTime(%s)\n", (const char*)key, (const char*)it->first, (const char*)p->toDateTime().getISO8601withMsec());
        } else if (p->isDate()) {
            PrintDebug("%s%s=Date(%s)\n", (const char*)key, (const char*)it->first, (const char*)p->toDate().toString());
        } else if (p->isTime()) {
            PrintDebug("%s%s=Time(%s)\n", (const char*)key, (const char*)it->first, (const char*)p->toTime().toString());
        } else if (p->isTimeDelta()) {
            PrintDebug("%s%s=TimeDelta(TODO)\n", (const char*)key, (const char*)it->first);
        } else if (p->isTimeZone()) {
            PrintDebug("%s%s=TimeZone(%s)\n", (const char*)key, (const char*)it->first, (const char*)p->toTimeZone().toString(true));

            //} else {
            //    PrintDebug("%s%s=UnknownDataType Id=%i\n", (const char*)key, (const char*)it->first, p->type());
        }
    }
}

#ifdef OLDCODE
size_t AssocArray::fromTemplate(
    const String& templ, const String& linedelimiter, const String& splitchar, const String& concat, bool dotrim)
{
    String Row, Line;

    String Key, Value;
    size_t rows = 0;
    ssize_t p;
    size_t ssc = splitchar.size();
    Array a;
    a.explode(templ, linedelimiter);

    Array::const_iterator it;
    for (it = a.begin(); it != a.end(); ++it) {
        Line = *it;
        Row = Trim(Line);
        if (Row.len() > 0 && Row[0] != L'#') { // Leere Zeilen und Kommentare ignorieren
            Row = Line;
            p = Row.instr(splitchar);
            if (p > 0) {
                Key = Trim(Row.left(p));
                Value = Row.mid(p + ssc);
                if (dotrim) Value.trim();
                // printf ("Key=%ls\nValue=%ls\n",(const wchar_t *)Key, (const wchar_t *)Value);
                if (concat.notEmpty()) {
                    append(Key, Value, concat);
                } else {
                    set(Key, Value);
                }
                rows++;
            }
        }
    }
    return rows;
}

size_t AssocArray::fromConfig(
    const String& content, const String& linedelimiter, const String& splitchar, const String& concat, bool dotrim)
{
    String Row, Line, Section;
    Array a;
    String Key, Value;
    size_t rows = 0;
    ssize_t p;
    size_t ssc = splitchar.size();
    a.explode(content, linedelimiter);
    pplib::Array::const_iterator it;
    for (it = a.begin(); it != a.end(); ++it) {
        Line = *it;
        Row = Trim(Line);
        if (Row.len() > 0 && Row[0] != L'#') { // Leere Zeilen und Kommentare ignorieren
            if (Row[0] == L'[' && Row[-1] == L']') {
                Section = Row.mid(1, Row.len() - 2);
                Section.lowerCase();
                Section.trim();
            } else {
                Row = Line;
                p = Row.instr(splitchar);
                if (p > 0) {
                    Key = Section;
                    if (Key.notEmpty()) Key += "/";
                    Key += Trim(Row.left(p));
                    Value = Row.mid(p + ssc);
                    if (dotrim) Value.trim();
                    // printf ("Key=%ls\nValue=%ls\n",(const wchar_t *)Key, (const wchar_t *)Value);
                    if (concat.notEmpty()) {
                        append(Key, Value, concat);
                    } else {
                        set(Key, Value);
                    }
                    rows++;
                }
            }
        }
    }
    return rows;
}

void AssocArray::toTemplate(String& s, const String& prefix, const String& linedelimiter, const String& splitchar) const
{
    String key, pre, value, index;
    Array Tok;
    if (prefix.notEmpty()) key = prefix + "/";
    pplib::AssocArray::const_iterator it;
    for (it = Tree.begin(); it != Tree.end(); ++it) {
        Variant* p = it->second;
        if (p->isString()) {
            Tok.clear();
            Tok.explode(p->toString(), "\n");
            for (size_t i = 0; i < Tok.size(); i++) {
                s += key + it->first + splitchar + Tok[i] + linedelimiter;
            }
        } else if (p->isAssocArray()) {
            pre.setf("%s%s", (const char*)key, (const char*)it->first);
            p->toAssocArray().toTemplate(s, pre, linedelimiter, splitchar);
        } else if (p->isArray()) {
            const Array& a = (const Array&)*p;
            for (size_t i = 0; i < a.size(); i++) {
                Tok.clear();
                Tok.explode(a[i], "\n");
                index.setf("%zu", i);
                for (size_t z = 0; z < Tok.size(); z++) {
                    s += key + it->first + "/" + index + splitchar + Tok[z] + linedelimiter;
                }
            }
        } else if (p->isDateTime()) {
            s += key + it->first + splitchar + p->toDateTime().getISO8601withMsec() + linedelimiter;
        }
    }
}

#endif

} // namespace pplib
