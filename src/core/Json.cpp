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

#include <pplib/core/json.h>
#include <pplib/core/fileobject.h>
#include <pplib/core/file.h>
#include <pplib/core/memfile.h>
#include <pplib/core/functions.h>
#include <pplib/exceptions.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/variant.h>
#include <pplib/types/variantarray.h>
#include <pplib/types/assocarray.h>
#include <pplib/types/array.h>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace pplib
{

// ============================================================================
// Forward Declarations
// ============================================================================
static pplib::Variant parseValue(pplib::FileObject& file, int c);
static pplib::AssocArray parseDict(pplib::FileObject& file);
static pplib::VariantArray parseArray(pplib::FileObject& file);

static void writeVariant(pplib::FileObject& file, const pplib::Variant& value, int indent, int level);
static void writeDict(pplib::FileObject& file, const pplib::AssocArray& data, int indent, int level);
static void writeArray(pplib::FileObject& file, const pplib::VariantArray& data, int indent, int level);
static void writeClassicArray(pplib::FileObject& file, const pplib::Array& data, int indent, int level);

// ============================================================================
// Hilfsfunktionen fürs Parsen
// ============================================================================

static void skipToEOL(pplib::FileObject& file)
{
    while (!file.eof()) {
        int c = file.fgetc();
        if (c == '\n' || c == EOF) return;
    }
}

static int nextNonWhitespace(pplib::FileObject& file)
{
    while (!file.eof()) {
        int c = file.fgetc();
        if (c == EOF) return EOF;
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;
        if (c == '/') {
            int c2 = file.fgetc();
            if (c2 == '/') {
                skipToEOL(file);
                continue;
            } else {
                if (c2 != EOF) {
                    file.seek(-1, pplib::File::SEEKCUR);
                }
                return c;
            }
        }
        return c;
    }
    return EOF;
}

static void readChars(pplib::FileObject& file, const char* chars)
{
    int c, p = 0;
    while (chars[p] != 0) {
        c = file.fgetc();
        if (c == EOF) throw pplib::UnexpectedEndOfDataException();
        if (c != chars[p])
            throw pplib::UnexpectedCharacterException("Expected: >>%s<<, character: >>%c<<, got: >>%c<<", chars, chars[p], c);
        p++;
    }
}

static void appendUtf8(pplib::String& str, uint32_t codePoint)
{
    if (codePoint >= 0xD800 && codePoint <= 0xDFFF) {
        throw pplib::CharacterEncodingException("Lone surrogate 0x%04X in JSON string", codePoint);
    }
    if (codePoint < 0x80) {
        str.append(static_cast<char>(codePoint));
    } else if (codePoint < 0x800) {
        char buf[2];
        buf[0] = static_cast<char>(0xC0 | (codePoint >> 6));
        buf[1] = static_cast<char>(0x80 | (codePoint & 0x3F));
        str.append(buf, 2);
    } else if (codePoint < 0x10000) {
        char buf[3];
        buf[0] = static_cast<char>(0xE0 | (codePoint >> 12));
        buf[1] = static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        buf[2] = static_cast<char>(0x80 | (codePoint & 0x3F));
        str.append(buf, 3);
    } else {
        char buf[4];
        buf[0] = static_cast<char>(0xF0 | (codePoint >> 18));
        buf[1] = static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
        buf[2] = static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        buf[3] = static_cast<char>(0x80 | (codePoint & 0x3F));
        str.append(buf, 4);
    }
}

static pplib::String getString(pplib::FileObject& file)
{
    pplib::String str;
    int c;
    while (!file.eof()) {
        c = file.fgetc();
        if (c == '\\') {
            c = file.fgetc();
            if (c == EOF) throw pplib::UnexpectedEndOfDataException();
            if (c == 'n')
                str.append('\n');
            else if (c == 'r')
                str.append('\r');
            else if (c == 't')
                str.append('\t');
            else if (c == 'b')
                str.append('\b');
            else if (c == 'f')
                str.append('\f');
            else if (c == '"')
                str.append('"');
            else if (c == '\\')
                str.append('\\');
            else if (c == '/')
                str.append('/');
            else if (c == 'u') {
                char hex[5];
                for (int i = 0; i < 4; i++) {
                    int h = file.fgetc();
                    if (h == EOF) throw pplib::UnexpectedEndOfDataException();
                    if (!isxdigit(h)) {
                        throw pplib::InvalidEscapeSequenceException("Invalid hex digit in \\u escape sequence: >>%c<<", h);
                    }
                    hex[i] = (char)h;
                }
                hex[4] = 0;
                unsigned int codePoint = strtoul(hex, NULL, 16);

                // Handle surrogate pairs
                if (codePoint >= 0xD800 && codePoint <= 0xDBFF) {
                    int nextC = file.fgetc();
                    if (nextC == EOF) throw pplib::UnexpectedEndOfDataException();
                    if (nextC == '\\') {
                        int nextNextC = file.fgetc();
                        if (nextNextC == EOF) throw pplib::UnexpectedEndOfDataException();
                        if (nextNextC == 'u') {
                            for (int i = 0; i < 4; i++) {
                                int h = file.fgetc();
                                if (h == EOF) throw pplib::UnexpectedEndOfDataException();
                                if (!isxdigit(h)) {
                                    throw pplib::InvalidEscapeSequenceException("Invalid hex digit in \\u escape sequence: >>%c<<", h);
                                }
                                hex[i] = (char)h;
                            }
                            hex[4] = 0;
                            unsigned int lowSurrogate = strtoul(hex, NULL, 16);
                            if (lowSurrogate >= 0xDC00 && lowSurrogate <= 0xDFFF) {
                                codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (lowSurrogate - 0xDC00);
                            } else {
                                throw pplib::CharacterEncodingException(
                                    "Expected low surrogate (0xDC00-0xDFFF) after high surrogate 0x%04X, got 0x%04X", codePoint,
                                    lowSurrogate);
                            }
                        } else {
                            throw pplib::CharacterEncodingException("Expected '\\u' low surrogate after high surrogate 0x%04X", codePoint);
                        }
                    } else {
                        throw pplib::CharacterEncodingException("Expected low surrogate escape sequence after high surrogate 0x%04X",
                                                                codePoint);
                    }
                }

                appendUtf8(str, codePoint);
            } else
                throw InvalidEscapeSequenceException("\\%c", c);
        } else if (c == '"') {
            return str;
        } else if (c == EOF) {
            break;
        } else {
            str.append(static_cast<char>(c));
        }
    }
    throw pplib::UnexpectedEndOfDataException();
}

static pplib::Variant getNumber(pplib::FileObject& file, int firstChar)
{
    pplib::String str;
    str.append(static_cast<char>(firstChar));
    bool isFloat = false;
    while (!file.eof()) {
        int c = file.fgetc();
        if (c == EOF) break;
        if (c == '.' || c == 'e' || c == 'E') {
            isFloat = true;
            str.append(static_cast<char>(c));
        } else if (c == '-' || c == '+' || (c >= '0' && c <= '9')) {
            str.append(static_cast<char>(c));
        } else {
            file.seek(-1, pplib::File::SEEKCUR);
            break;
        }
    }
    if (str == "-") {
        throw pplib::UnexpectedCharacterException("Invalid number >>-<< at position %lld", file.tell());
    }
    if (isFloat) {
        return pplib::Variant(str.toDouble());
    } else {
        return pplib::Variant(str.toInt64());
    }
}

static pplib::Variant parseValue(pplib::FileObject& file, int c)
{
    if (c == '"') {
        return pplib::Variant(getString(file));
    } else if (c == '{') {
        return pplib::Variant(parseDict(file));
    } else if (c == '[') {
        return pplib::Variant(parseArray(file));
    } else if (c == '-' || (c >= '0' && c <= '9')) {
        return getNumber(file, c);
    } else if (c == 't') {
        readChars(file, "rue");
        return pplib::Variant(true);
    } else if (c == 'f') {
        readChars(file, "alse");
        return pplib::Variant(false);
    } else if (c == 'n') {
        readChars(file, "ull");
        return pplib::Variant(nullptr);
    }
    throw pplib::UnexpectedCharacterException("Unexpected character >>%c<< (ASCII %d) at position %lld", c, c, file.tell());
}

static pplib::VariantArray parseArray(pplib::FileObject& file)
{
    pplib::VariantArray arr;
    int c = nextNonWhitespace(file);
    if (c == EOF) throw pplib::UnexpectedEndOfDataException();
    if (c == ']') return arr;

    while (true) {
        pplib::Variant val = parseValue(file, c);
        arr.add(std::move(val));

        c = nextNonWhitespace(file);
        if (c == EOF) throw pplib::UnexpectedEndOfDataException();
        if (c == ',') {
            c = nextNonWhitespace(file);
            if (c == EOF) throw pplib::UnexpectedEndOfDataException();
            if (c == ']') {
                throw pplib::UnexpectedCharacterException("Trailing comma in array at position %lld", file.tell());
            }
            continue;
        } else if (c == ']') {
            return arr;
        } else {
            throw pplib::UnexpectedCharacterException("Expected ',' or ']' in array at position %lld, got >>%c<<", file.tell(), c);
        }
    }
}

static pplib::AssocArray parseDict(pplib::FileObject& file)
{
    pplib::AssocArray dict;
    int c = nextNonWhitespace(file);
    if (c == EOF) throw pplib::UnexpectedEndOfDataException();
    if (c == '}') return dict;

    while (true) {
        if (c != '"') {
            throw pplib::UnexpectedCharacterException("Expected '\"' for object key at position %lld, got >>%c<<", file.tell(), c);
        }
        pplib::String key = getString(file);
        if (key.isEmpty()) key = "_empty_";

        c = nextNonWhitespace(file);
        if (c != ':') {
            throw pplib::UnexpectedCharacterException("Expected ':' after key >>%s<< at position %lld, got >>%c<<", (const char*)key,
                                                      file.tell(), c);
        }

        c = nextNonWhitespace(file);
        if (c == EOF) throw pplib::UnexpectedEndOfDataException();
        pplib::Variant val = parseValue(file, c);
        dict.set(key, std::move(val));

        c = nextNonWhitespace(file);
        if (c == EOF) throw pplib::UnexpectedEndOfDataException();
        if (c == ',') {
            c = nextNonWhitespace(file);
            if (c == EOF) throw pplib::UnexpectedEndOfDataException();
            if (c == '}') {
                throw pplib::UnexpectedCharacterException("Trailing comma in object at position %lld", file.tell());
            }
            continue;
        } else if (c == '}') {
            return dict;
        } else {
            throw pplib::UnexpectedCharacterException("Expected ',' or '}' in object at position %lld, got >>%c<<", file.tell(), c);
        }
    }
}

static void expectEof(pplib::FileObject& file)
{
    int c = nextNonWhitespace(file);
    if (c != EOF) {
        throw pplib::UnexpectedCharacterException("Garbage after JSON end: >>%c<< at position %lld", c, file.tell());
    }
}

// ============================================================================
// Public Parse-Methoden
// ============================================================================

void Json::load(pplib::Variant& data, pplib::FileObject& file)
{
    try {
        int c = nextNonWhitespace(file);
        if (c == EOF) throw pplib::UnexpectedEndOfDataException();
        data = parseValue(file, c);
        expectEof(file);
    }
    catch (const pplib::EndOfFileException&) {
        throw pplib::UnexpectedEndOfDataException();
    }
}

void Json::loads(pplib::Variant& data, const pplib::String& json)
{
    pplib::MemFile file((void*)json.getPtr(), json.size());
    load(data, file);
}

pplib::Variant Json::loads(const pplib::String& json)
{
    pplib::Variant result;
    loads(result, json);
    return result;
}

pplib::Variant Json::load(pplib::FileObject& file)
{
    pplib::Variant result;
    load(result, file);
    return result;
}

void Json::load(pplib::AssocArray& data, pplib::FileObject& file)
{
    pplib::Variant v;
    load(v, file);
    if (!v.isAssocArray()) {
        throw pplib::TypeConversionException("JSON root is not an AssocArray (type %d)", v.type());
    }
    data = std::move(v.toAssocArray());
}

void Json::loads(pplib::AssocArray& data, const pplib::String& json)
{
    pplib::MemFile file((void*)json.getPtr(), json.size());
    load(data, file);
}

void Json::load(pplib::VariantArray& data, pplib::FileObject& file)
{
    pplib::Variant v;
    load(v, file);
    if (!v.isVariantArray()) {
        throw pplib::TypeConversionException("JSON root is not a VariantArray (type %d)", v.type());
    }
    data = std::move(v.toVariantArray());
}

void Json::loads(pplib::VariantArray& data, const pplib::String& json)
{
    pplib::MemFile file((void*)json.getPtr(), json.size());
    load(data, file);
}

// ============================================================================
// Hilfsfunktionen fürs Serialisieren
// ============================================================================

static String escapeString(const String& s)
{
    String ret;
    ret.reserve(s.size() + 16);
    const char* p = (const char*)s;
    size_t len = s.size();
    for (size_t i = 0; i < len; ++i) {
        char c = p[i];
        switch (c) {
        case '"':
            ret.append("\\\"");
            break;
        case '\\':
            ret.append("\\\\");
            break;
        case '\b':
            ret.append("\\b");
            break;
        case '\f':
            ret.append("\\f");
            break;
        case '\n':
            ret.append("\\n");
            break;
        case '\r':
            ret.append("\\r");
            break;
        case '\t':
            ret.append("\\t");
            break;
        default:
            if (static_cast<unsigned char>(c) < 32) {
                char ubuf[8];
                snprintf(ubuf, sizeof(ubuf), "\\u%04x", static_cast<unsigned char>(c));
                ret.append(ubuf);
            } else {
                ret.append(c);
            }
            break;
        }
    }
    return ret;
}

static void writeIndent(pplib::FileObject& file, int indent, int level)
{
    if (indent >= 0) {
        file.fputc('\n');
        for (int i = 0; i < level * indent; ++i) {
            file.fputc(' ');
        }
    }
}

static void writeDict(pplib::FileObject& file, const pplib::AssocArray& data, int indent, int level)
{
    file.fputc('{');
    if (data.size() == 0) {
        file.fputc('}');
        return;
    }
    pplib::AssocArray::const_iterator it;
    bool first = true;
    for (it = data.begin(); it != data.end(); ++it) {
        if (!first) {
            file.fputc(',');
        }
        first = false;
        writeIndent(file, indent, level + 1);
        file.putsf("\"%s\":", (const char*)escapeString((*it).first));
        if (indent >= 0) file.fputc(' ');
        writeVariant(file, *(*it).second, indent, level + 1);
    }
    writeIndent(file, indent, level);
    file.fputc('}');
}

static void writeArray(pplib::FileObject& file, const pplib::VariantArray& data, int indent, int level)
{
    file.fputc('[');
    if (data.size() == 0) {
        file.fputc(']');
        return;
    }
    for (size_t i = 0; i < data.size(); ++i) {
        if (i > 0) {
            file.fputc(',');
        }
        writeIndent(file, indent, level + 1);
        writeVariant(file, data[i], indent, level + 1);
    }
    writeIndent(file, indent, level);
    file.fputc(']');
}

static void writeClassicArray(pplib::FileObject& file, const pplib::Array& data, int indent, int level)
{
    file.fputc('[');
    if (data.size() == 0) {
        file.fputc(']');
        return;
    }
    for (size_t i = 0; i < data.size(); ++i) {
        if (i > 0) {
            file.fputc(',');
        }
        writeIndent(file, indent, level + 1);
        file.putsf("\"%s\"", (const char*)escapeString(data.get(i)));
    }
    writeIndent(file, indent, level);
    file.fputc(']');
}

static void writeVariant(pplib::FileObject& file, const pplib::Variant& value, int indent, int level)
{
    if (value.isString()) {
        file.putsf("\"%s\"", (const char*)escapeString(value.toString()));
    } else if (value.isWideString()) {
        pplib::ByteArray ba = value.toWideString().toUtf8();
        pplib::String str(ba);
        file.putsf("\"%s\"", (const char*)escapeString(str));
    } else if (value.isInt64()) {
        file.putsf("%lld", (long long)value.toInt64());
    } else if (value.isDouble()) {
        double d = value.toDouble();
        if (std::isnan(d) || std::isinf(d)) {
            file.puts("null");
        } else {
            char buf[64];
            snprintf(buf, sizeof(buf), "%.15g", d);
            for (char* p = buf; *p; ++p) {
                if (*p == ',') *p = '.';
            }
            file.puts(buf);
        }
    } else if (value.isBool()) {
        file.puts(value.toBool() ? "true" : "false");
    } else if (value.isNull()) {
        file.puts("null");
    } else if (value.isVariantArray()) {
        writeArray(file, value.toVariantArray(), indent, level);
    } else if (value.isArray()) {
        writeClassicArray(file, value.toArray(), indent, level);
    } else if (value.isAssocArray()) {
        writeDict(file, value.toAssocArray(), indent, level);
    } else if (value.isByteArrayPtr() || value.isByteArray()) {
        const pplib::ByteArrayPtr& ba = value.toByteArrayPtr();
        pplib::String str = ba.toBase64();
        file.putsf("\"%s\"", (const char*)str);
    } else if (value.isDateTime()) {
        file.putsf("\"%s\"", (const char*)value.toDateTime().getISO8601withUsec());
    } else if (value.isDate()) {
        file.putsf("\"%s\"", (const char*)value.toDate().toString());
    } else if (value.isTime()) {
        file.putsf("\"%s\"", (const char*)value.toTime().toString());
    } else if (value.isTimeDelta()) {
        file.putsf("\"%s\"", (const char*)value.toTimeDelta().toString());
    } else if (value.isTimeZone()) {
        file.putsf("\"%s\"", (const char*)value.toTimeZone().toString());
    } else {
        throw pplib::UnsupportedDataTypeException("Variant Type >>%d<< cannot be serialized to JSON", value.type());
    }
}

// ============================================================================
// Public Dump-Methoden
// ============================================================================

void Json::dump(pplib::FileObject& file, const pplib::Variant& data, int indent)
{
    file.rewind();
    file.truncate(0);
    writeVariant(file, data, indent, 0);
    if (indent >= 0) {
        file.fputc('\n');
    }
}

void Json::dumps(pplib::String& json, const pplib::Variant& data, int indent)
{
    pplib::MemFile file((void*)NULL, 0, true);
    Json::dump(file, data, indent);
    size_t size = file.tell();
    ByteArray ba;
    unsigned char* str = (unsigned char*)ba.malloc(size + 1);
    file.rewind();
    file.fread(str, size, 1);
    str[size] = 0;
    json.set((const char*)str, size);
}

pplib::String Json::dumps(const pplib::Variant& data, int indent)
{
    pplib::String result;
    Json::dumps(result, data, indent);
    return result;
}

void Json::dump(pplib::FileObject& file, const pplib::AssocArray& data, int indent)
{
    file.rewind();
    file.truncate(0);
    writeDict(file, data, indent, 0);
    if (indent >= 0) {
        file.fputc('\n');
    }
}

void Json::dumps(pplib::String& json, const pplib::AssocArray& data, int indent)
{
    pplib::MemFile file((void*)NULL, 0, true);
    Json::dump(file, data, indent);
    size_t size = file.tell();
    ByteArray ba;
    unsigned char* str = (unsigned char*)ba.malloc(size + 1);
    file.rewind();
    file.fread(str, size, 1);
    str[size] = 0;
    json.set((const char*)str, size);
}

pplib::String Json::dumps(const pplib::AssocArray& data, int indent)
{
    pplib::String result;
    Json::dumps(result, data, indent);
    return result;
}

void Json::dump(pplib::FileObject& file, const pplib::VariantArray& data, int indent)
{
    file.rewind();
    file.truncate(0);
    writeArray(file, data, indent, 0);
    if (indent >= 0) {
        file.fputc('\n');
    }
}

void Json::dumps(pplib::String& json, const pplib::VariantArray& data, int indent)
{
    pplib::MemFile file((void*)NULL, 0, true);
    Json::dump(file, data, indent);
    size_t size = file.tell();
    ByteArray ba;
    unsigned char* str = (unsigned char*)ba.malloc(size + 1);
    if (!str) throw OutOfMemoryException();
    file.rewind();
    file.fread(str, size, 1);
    str[size] = 0;
    json.set((const char*)str, size);
}

pplib::String Json::dumps(const pplib::VariantArray& data, int indent)
{
    pplib::String result;
    Json::dumps(result, data, indent);
    return result;
}

pplib::String Json::pp(const pplib::String& json, int indent)
{
    pplib::Variant v = loads(json);
    return dumps(v, indent);
}

} // end of namespace pplib
