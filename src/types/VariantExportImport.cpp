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
#include <string.h>
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
#include <pplib/types/variantarray.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

namespace pplib
{
size_t exportVariantBinary(const Variant& v, char* buffer, size_t buffersize)
{
    if (!buffer) buffersize = 0;
    size_t vallen = 0;
    Variant::DataType t = v.type();
    // Sonderfall: ByteArrayPtr wird als ByteArray exportiert
    if (t == Variant::TYPE_BYTEARRAYPTR) t = Variant::TYPE_BYTEARRAY;

    if (1 <= buffersize) PokeN8(buffer, (uint8_t)t);
    size_t p = 1;
    switch (t) {
    case Variant::TYPE_STRING: {
        const String& string = v.toString();
        vallen = string.size();
        if (p + 4 <= buffersize) PokeN32(buffer + p, (int)vallen);
        p += 4;
        if (p + vallen <= buffersize) memcpy(buffer + p, (const char*)string, vallen);
        p += vallen;
    } break;
    case Variant::TYPE_WIDESTRING: {
        ByteArray ba = v.toWideString().toUtf8(); // Wird zu UTF-8 konvertiert
        vallen = ba.size();
        if (p + 4 <= buffersize) PokeN32(buffer + p, (int)vallen);
        p += 4;
        if (p + vallen <= buffersize) memcpy(buffer + p, ba.adr(), vallen);
        p += vallen;
    } break;
    case Variant::TYPE_ASSOCARRAY: {
        const AssocArray& aa = v.toAssocArray();
        if (!buffersize) {
            p += aa.exportBinary(nullptr, 0);
        } else {
            size_t remaining = (buffersize > p) ? (buffersize - p) : 0;
            p += aa.exportBinary(buffer + p, remaining);
        }
    } break;
    case Variant::TYPE_VARIANTARRAY: {
        const VariantArray& va = v.toVariantArray();
        if (!buffersize) {
            p += va.exportBinary(nullptr, 0);
        } else {
            size_t remaining = (buffersize > p) ? (buffersize - p) : 0;
            p += va.exportBinary(buffer + p, remaining);
        }
    } break;
    case Variant::TYPE_ARRAY: {
        const Array& arr = v.toArray();
        if (p + 4 <= buffersize) PokeN32(buffer + p, (int)arr.size());
        p += 4;
        for (ssize_t i = 0; i < (ssize_t)arr.size(); i++) {
            const String s = arr.get(i);
            vallen = s.size();
            if (p + 4 < buffersize) PokeN32(buffer + p, (int)vallen);
            p += 4;
            if (p + vallen <= buffersize) memcpy(buffer + p, (const char*)s, vallen);
            p += vallen;
        }
    } break;
    case Variant::TYPE_DATETIME: {
        const DateTime& dt = v.toDateTime();
        // DateTime könnte invalid sein
        if (dt.isEmpty()) {
            if (p + 4 <= buffersize) PokeN32(buffer + p, 0);
            p += 4;
        } else {
            vallen = 10;                           // PPL8 speichert Microseconds in 8 und Zeitzone in 2 Bytes,
            vallen += dt.timeZone().name().size(); // plus die Länge des Zeitzonen-Namens, der aber leer sein kann.
            if (p + 4 <= buffersize) PokeN32(buffer + p, (int)vallen);
            p += 4;
            if (p + vallen <= buffersize) {
                PokeN64(buffer + p, dt.toMicroseconds());
                PokeN16(buffer + p + 8, dt.timeZone().offsetMinutes());
                memcpy(buffer + p + 10, (const char*)dt.timeZone().name(), dt.timeZone().name().size());
            }
            p += vallen;
        }
    } break;
    case Variant::TYPE_BYTEARRAY:
    case Variant::TYPE_BYTEARRAYPTR: {
        const ByteArrayPtr& baptr = v.toByteArrayPtr();
        vallen = baptr.size();
        if (p + 4 <= buffersize) PokeN32(buffer + p, (int)vallen);
        p += 4;
        if (p + vallen <= buffersize) memcpy(buffer + p, baptr.adr(), vallen);
        p += vallen;
    } break;
    case Variant::TYPE_DATE: {
        const Date& date = v.toDate();
        vallen = 4; // Date exportiert das Datum als 32Bit Integer (YYYYMMDD)
        if (p + vallen <= buffersize) {
            PokeN32(buffer + p, date.toInt());
        }
        p += vallen;
    } break;
    case Variant::TYPE_TIME: {
        const Time& time = v.toTime();
        vallen = 8; // Time exportiert die Zeit in Microseconds als 64Bit Integer
        if (p + vallen <= buffersize) {
            PokeN64(buffer + p, time.toMicroseconds());
        }
        p += vallen;
    } break;
    case Variant::TYPE_TIMEDELTA: {
        const TimeDelta& td = v.toTimeDelta();
        vallen = 8; // TimeDelta exportiert die Zeit in Microseconds als 64Bit Integer
        if (p + vallen <= buffersize) {
            PokeN64(buffer + p, td.toMicroseconds());
        }
        p += vallen;
    } break;
    case Variant::TYPE_TIMEZONE: {
        const TimeZone& tz = v.toTimeZone();
        vallen = 2;                 // TimeZone exportiert die OffsetMinutes als 16Bit Integer,
        vallen += tz.name().size(); // und den Namen als String, der aber leer sein kann
        if (p + 4 <= buffersize) PokeN32(buffer + p, (int)vallen);
        p += 4;
        if (p + vallen <= buffersize) {
            PokeN16(buffer + p, tz.offsetMinutes());
            memcpy(buffer + p + 2, (const char*)tz.name(), tz.name().size());
        }
        p += vallen;
    } break;
    case Variant::TYPE_NULL: {
        // keine Payload für Null-Werte
    } break;
    case Variant::TYPE_BOOL: {
        vallen = 1; // Boolean exportiert als 1 Byte
        if (p + vallen <= buffersize) {
            PokeN8(buffer + p, v.toBool() ? 1 : 0);
        }
        p += vallen;
    } break;
    case Variant::TYPE_INT64: {
        vallen = 8;
        if (p + vallen <= buffersize) {
            PokeN64(buffer + p, v.toInt64());
        }
        p += vallen;
    } break;
    case Variant::TYPE_DOUBLE: {
        vallen = 8; // Double exportiert als 64Bit IEEE 754 Floating Point
        if (p + vallen <= buffersize) {
            uint64_t bits = std::bit_cast<uint64_t>(v.toDouble());
            PokeN64(buffer + p, bits);
        }
        p += vallen;
    } break;
    // LCOV_EXCL_START
    default:
        // Unbekannter Typ oder TYPE_UNKNOWN, keine Aktion
        break;
        // LCOV_EXCL_STOP
    }
    return p;
}

size_t importVariantBinary(Variant& v, const char* ptr, size_t buffersize)
{
    if (buffersize < 1) throw ImportFailedException("Truncated variant data");
    size_t p = 0;
    auto requireBytes = [&](size_t n) {
        if (buffersize - p < n) throw ImportFailedException("Buffer too small for variant");
    };

    requireBytes(1);
    uint8_t t = PeekN8(ptr + p);
    p++;

    switch (t) {
    case Variant::TYPE_NULL:
        v.setNull();
        break;

    case Variant::TYPE_BOOL:
        requireBytes(1);
        v.set(PeekN8(ptr + p) != 0);
        p += 1;
        break;

    case Variant::TYPE_INT64:
        requireBytes(8);
        v.set((int64_t)PeekN64(ptr + p));
        p += 8;
        break;

    case Variant::TYPE_DOUBLE:
        requireBytes(8);
        v.set(std::bit_cast<double>(PeekN64(ptr + p)));
        p += 8;
        break;

    case Variant::TYPE_DATE:
        requireBytes(4);
        v.set(Date::fromInt(PeekN32(ptr + p)));
        p += 4;
        break;

    case Variant::TYPE_TIME:
        requireBytes(8);
        v.set(Time::fromMicroseconds(PeekN64(ptr + p)));
        p += 8;
        break;

    case Variant::TYPE_TIMEDELTA:
        requireBytes(8);
        v.set(TimeDelta::fromMicroseconds((int64_t)PeekN64(ptr + p)));
        p += 8;
        break;

    case Variant::TYPE_STRING: {
        requireBytes(4);
        size_t vallen = PeekN32(ptr + p);
        p += 4;
        requireBytes(vallen);
        v.set(String(ptr + p, vallen));
        p += vallen;
    } break;

    case Variant::TYPE_WIDESTRING: {
        requireBytes(4);
        size_t vallen = PeekN32(ptr + p);
        p += 4;
        requireBytes(vallen);
        // UTF-8 Rückkonvertierung:
        WideString ws;
        ws.fromUtf8(ByteArrayPtr(ptr + p, vallen));
        v.set(std::move(ws));
        p += vallen;
    } break;

    case Variant::TYPE_BYTEARRAY: {
        requireBytes(4);
        size_t vallen = PeekN32(ptr + p);
        p += 4;
        requireBytes(vallen);
        v.set(ByteArray(ptr + p, vallen));
        p += vallen;
    } break;

    case Variant::TYPE_DATETIME: {
        requireBytes(4);
        size_t vallen = PeekN32(ptr + p);
        p += 4;
        if (vallen == 0) {
            v.set(DateTime());
        } else {
            requireBytes(vallen);
            if (vallen < 10) throw ImportFailedException("Invalid DateTime length");
            int64_t us = (int64_t)PeekN64(ptr + p);
            int16_t tz_offset = (int16_t)PeekN16(ptr + p + 8);
            DateTime dt;
            dt.setMicroseconds(us, TimeZone(tz_offset));
            if (vallen > 10) {
                dt.timeZone().setName(String(ptr + p + 10, vallen - 10));
            }
            v.set(std::move(dt));
        }
        p += vallen;
    } break;

    case Variant::TYPE_TIMEZONE: {
        requireBytes(4);
        size_t vallen = PeekN32(ptr + p);
        p += 4;
        requireBytes(vallen);
        if (vallen < 2) throw ImportFailedException("Invalid TimeZone length");
        int16_t offset = (int16_t)PeekN16(ptr + p);
        String name;
        if (vallen > 2) name.set(ptr + p + 2, vallen - 2);
        v.set(TimeZone(offset, name));
        p += vallen;
    } break;

    case Variant::TYPE_ARRAY: {
        requireBytes(4);
        size_t elements = PeekN32(ptr + p);
        p += 4;
        Array arr;
        arr.reserve(elements);
        for (size_t i = 0; i < elements; i++) {
            requireBytes(4);
            size_t vallen = PeekN32(ptr + p);
            p += 4;
            requireBytes(vallen);
            arr.add(String(ptr + p, vallen));
            p += vallen;
        }
        v.set(std::move(arr));
    } break;

    case Variant::TYPE_ASSOCARRAY: {
        AssocArray na;
        size_t bytes = na.importBinary(ptr + p, buffersize - p);
        p += bytes;
        v.set(std::move(na));
    } break;

    case Variant::TYPE_VARIANTARRAY: {
        VariantArray va;
        size_t bytes = va.importBinary(ptr + p, buffersize - p);
        p += bytes;
        v.set(std::move(va));
    } break;
    case Variant::TYPE_UNKNOWN: {
        // TYPE_UNKNOWN, keine Aktion
    } break;
    // LCOV_EXCL_START
    default:
        throw ImportFailedException("Unknown datatype %d", t);
        // LCOV_EXCL_STOP
    }
    return p;
}

} // namespace pplib