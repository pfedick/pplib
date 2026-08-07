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
#include <pplib/types/array.h>
#include <pplib/types/assocarray.h>

namespace pplib
{

struct ParserState
{
    enum state
    {
        ExpectingKey,
        ExpectingColon,
        ExpectingValue,
        ExpectingNextOrEnd,
    };
};

static void readDict(pplib::AssocArray& data, pplib::FileObject& file);
static void readArray(pplib::AssocArray& data, pplib::FileObject& file);

static pplib::String getString(pplib::FileObject& file)
{
    pplib::String str;
    int c;
    while (!file.eof()) {
        c = file.fgetc();
        if (c == '\\') {
            c = file.fgetc();
            if (c == 'n')
                str.append("\n");
            else if (c == 'r')
                str.append("\r");
            else if (c == 't')
                str.append("\t");
            else if (c == 'b')
                str.append("\b");
            else if (c == 'f')
                str.append("\f");
            else if (c == '"')
                str.append("\"");
            else if (c == '\\')
                str.append("\\");
            else if (c == '/')
                str.append("/");
            else if (c == 'u') {
                char hex[5];
                for (int i = 0; i < 4; i++) {
                    int h = file.fgetc();
                    if (h == EOF) throw pplib::UnexpectedEndOfDataException();
                    hex[i] = (char)h;
                }
                hex[4] = 0;
                unsigned int codePoint = strtoul(hex, NULL, 16);

                // Handle surrogate pairs
                if (codePoint >= 0xD800 && codePoint <= 0xDBFF) {
                    int nextC = file.fgetc();
                    if (nextC == '\\') {
                        int nextNextC = file.fgetc();
                        if (nextNextC == 'u') {
                            for (int i = 0; i < 4; i++) {
                                int h = file.fgetc();
                                if (h == EOF) throw pplib::UnexpectedEndOfDataException();
                                hex[i] = (char)h;
                            }
                            hex[4] = 0;
                            unsigned int lowSurrogate = strtoul(hex, NULL, 16);
                            if (lowSurrogate >= 0xDC00 && lowSurrogate <= 0xDFFF) {
                                codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (lowSurrogate - 0xDC00);
                            } else {
                                // Not a valid low surrogate? Backup and treat high surrogate as is
                                file.seek(-6, pplib::File::SEEKCUR);
                            }
                        } else {
                            file.seek(-2, pplib::File::SEEKCUR);
                        }
                    } else {
                        file.seek(-1, pplib::File::SEEKCUR);
                    }
                }

                pplib::WideString ws;
                ws.append((wchar_t)codePoint);
                str.append(ws.toUtf8().toString());
            } else
                throw InvalidEscapeSequenceException("\\%c", c);
        } else if (c == '"') {
            return str;
        } else {
            str.append(c);
        }
    }
    throw pplib::UnexpectedEndOfDataException();
}

static pplib::String getNumber(pplib::FileObject& file)
{
    pplib::String str;
    int c;
    while (!file.eof()) {
        c = file.fgetc();
        if (c == EOF) break;
        if (c == '-' || c == '+' || c == '.' || c == 'e' || c == 'E' || (c >= '0' && c <= '9')) {
            str.append(c);
        } else {
            file.seek(-1, pplib::File::SEEKCUR);
            return str;
        }
    }
    return str;
}

static void readChars(pplib::FileObject& file, const char* chars)
{
    int c, p = 0;
    while (chars[p] != 0) {
        c = file.fgetc();
        if (c == EOF) throw pplib::UnexpectedEndOfDataException();
        if (c != chars[p])
            throw pplib::UnexpectedCharacterException("#1: Expected: >>%s<<, character: >>%c<<, got: >>%c<<", chars, chars[p], c);
        p++;
    }
}

static void skipToEOL(pplib::FileObject& file)
{
    while (!file.eof()) {
        int c = file.fgetc();
        if (c == '\n') return;
    }
}

static bool readValue(pplib::AssocArray& data, const pplib::String& key, pplib::FileObject& file, int c)
{
    if (c == '"') {
        pplib::String value = getString(file);
        data.set(key, value);
        return true;
    } else if (c == '[') { // Array
        pplib::AssocArray value;
        readArray(value, file);
        data.set(key, value);
        return true;
    } else if (c == '{') { // dict
        pplib::AssocArray value;
        readDict(value, file);
        data.set(key, value);
        return true;
    } else if (c == '-' || (c >= '0' && c <= '9')) {
        file.seek(-1, pplib::File::SEEKCUR);
        pplib::String value = getNumber(file);
        data.set(key, value);
        return true;
    } else if (c == 't') { // true
        file.seek(-1, pplib::File::SEEKCUR);
        readChars(file, "true");
        data.set(key, pplib::String("true"));
        return true;
    } else if (c == 'f') { // false
        file.seek(-1, pplib::File::SEEKCUR);
        readChars(file, "false");
        data.set(key, pplib::String("false"));
        return true;
    } else if (c == 'n') { // null
        file.seek(-1, pplib::File::SEEKCUR);
        readChars(file, "null");
        data.set(key, pplib::String("null"));
        return true;
    }
    return false;
}

static void readArray(pplib::AssocArray& data, pplib::FileObject& file)
{
    int c;
    ParserState::state state = ParserState::ExpectingValue;
    while (!file.eof()) {
        c = file.fgetc();
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') continue;
        if (state == ParserState::ExpectingValue && readValue(data, "[]", file, c) == true) {
            state = ParserState::ExpectingNextOrEnd;
        } else if (c == ',' && state == ParserState::ExpectingNextOrEnd) {
            state = ParserState::ExpectingValue;
        } else if (c == ']' && (state == ParserState::ExpectingValue || state == ParserState::ExpectingNextOrEnd)) {
            return;
        } else {
            throw pplib::UnexpectedCharacterException("#2: >>%c<< at position %lld while parsing array", c, file.tell());
        }
    }
    throw pplib::UnexpectedEndOfDataException();
}

static void readDict(pplib::AssocArray& data, pplib::FileObject& file)
{
    int c;
    pplib::String key;
    ParserState::state state = ParserState::ExpectingKey;
    while (!file.eof()) {
        c = file.fgetc();
        if (c == EOF) break;
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == 0) continue;
        if (c == '"' && state == ParserState::ExpectingKey) {
            key = getString(file);
            if (key.isEmpty()) key = "_empty_";
            state = ParserState::ExpectingColon;
        } else if (c == ':' && state == ParserState::ExpectingColon) {
            state = ParserState::ExpectingValue;
        } else if (c == ',' && state == ParserState::ExpectingNextOrEnd) {
            state = ParserState::ExpectingKey;
        } else if (state == ParserState::ExpectingValue) {
            if (readValue(data, key, file, c) == true) {
                state = ParserState::ExpectingNextOrEnd;
            } else {
                throw pplib::UnexpectedCharacterException(
                    "#3: >>%c<< at position %lld while parsing dict (expecting value), state ExpectingValue", c, file.tell());
            }
        } else if (c == '}' && (state == ParserState::ExpectingNextOrEnd || state == ParserState::ExpectingKey)) {
            return;
        } else if (c == '/' && (state == ParserState::ExpectingKey || state == ParserState::ExpectingNextOrEnd)) {
            int c2 = file.fgetc();
            if (c2 == '/') {
                skipToEOL(file);
            } else {
                file.seek(-1, pplib::File::SEEKCUR);
                throw pplib::UnexpectedCharacterException(">>%c<< at position %lld while parsing dict (slash), state %d", c, file.tell(),
                                                          state);
            }
        } else {
            throw pplib::UnexpectedCharacterException("#4: >>%c<< (ASCII %d) at position %lld while parsing dict (general), state %d", c, c,
                                                      file.tell(), state);
        }
    }
    throw pplib::UnexpectedEndOfDataException();
}

static void expectEof(pplib::FileObject& file)
{
    int c;
    while (!file.eof()) {
        c = file.fgetc();
        if (c == EOF) return;
        if (c != ' ' && c != '\n' && c != '\r' && c != '\t' && c != 0) {
            throw pplib::UnexpectedCharacterException("#5: >>%c<< at position %lld while parsing dict 2", c, file.tell());
        }
    }
}

void Json::loads(pplib::AssocArray& data, const pplib::String& json)
{
    pplib::MemFile file((void*)json.getPtr(), json.size());
    Json::load(data, file);
}

void Json::load(pplib::AssocArray& data, pplib::FileObject& file)
{
    int c;
    while (!file.eof()) {
        c = file.fgetc();
        if (c == '{') {
            readDict(data, file);
            expectEof(file);
            return;
        } else if (c == '[') {
            readArray(data, file);
            expectEof(file);
            return;
        } else if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
            throw pplib::UnexpectedCharacterException("#6: >>%c<< at position %lld while parsing dict 1", c, file.tell());
        }
    }
}

pplib::AssocArray Json::loads(const pplib::String& json)
{
    pplib::AssocArray result;
    Json::loads(result, json);
    return result;
}

pplib::AssocArray Json::load(pplib::FileObject& file)
{
    pplib::AssocArray result;
    Json::load(result, file);
    return result;
}

void Json::dumps(pplib::String& json, const pplib::AssocArray& data)
{
    pplib::MemFile file((void*)NULL, 0, true);
    Json::dump(file, data);
    size_t size = file.tell();
    unsigned char* str = (unsigned char*)malloc(size + 1);
    if (!str) throw OutOfMemoryException();
    file.rewind();
    file.fread(str, size, 1);
    str[size] = 0;
    json.set((const char*)str, size);
    free(str);
}

pplib::String Json::dumps(const pplib::AssocArray& data)
{
    pplib::String result;
    Json::dumps(result, data);
    return result;
}

static bool isArray(const pplib::AssocArray& data)
{
    uint64_t v = 0;
    pplib::AssocArray::const_iterator it;
    for (it = data.begin(); it != data.end(); ++it) {
        pplib::String expectedkey;
        expectedkey.setf("%llu", v);
        if ((*it).first != expectedkey) return false;
        v++;
    }
    if (data.begin() == data.end()) return false;
    return true;
}

static void writeArray(const pplib::AssocArray& data, pplib::FileObject& file);
static void writeArray(const pplib::Array& data, pplib::FileObject& file);
static void writeDict(const pplib::AssocArray& data, pplib::FileObject& file);

static void writeValue(pplib::FileObject& file, const pplib::String& key, const pplib::Variant* value)
{
    if (value->isString()) {
        const pplib::String& str = value->toString();
        if (str.isNumeric() && (!str.has(",")))
            file.puts(str);
        else if (str == "true" || str == "false" || str == "null")
            file.puts(str);
        else
            file.putsf("\"%s\"", (const char*)pplib::PythonHelper::escapeString(str));
    } else if (value->isWideString()) {
        const pplib::WideString& wstr = value->toWideString();
        pplib::ByteArray ba = wstr.toUtf8();
        pplib::String str(ba);
        if (str.isNumeric() && (!str.has(",")))
            file.puts(str);
        else if (str == "true" || str == "false" || str == "null")
            file.puts(str);
        else
            file.putsf("\"%s\"", (const char*)pplib::PythonHelper::escapeString(str));
    } else if (value->isArray()) {
        writeArray(value->toArray(), file);
    } else if (value->isAssocArray()) {
        writeDict(value->toAssocArray(), file);
    } else if (value->isByteArrayPtr()) {
        const pplib::ByteArrayPtr& ba = value->toByteArrayPtr();
        pplib::String str = ba.toBase64();
        file.fputc('"');
        file.fputs(str);
        file.fputc('"');

    } else {
        // printf ("Unexpected %s: %d\n",(const char*)key,value->type());
        throw UnsupportedDataTypeException("AssocArray Type >>%d<< at key >>%s<<", value->type(), (const char*)key);
    }
}

static void writeArray(const pplib::AssocArray& data, pplib::FileObject& file)
{
    file.fputc('[');
    pplib::AssocArray::const_iterator it;
    pplib::String key = "array";
    for (it = data.begin(); it != data.end(); ++it) {
        if (it != data.begin()) file.fputc(',');
        writeValue(file, key, (*it).second);
    }
    file.fputc(']');
}

static void writeArray(const pplib::Array& data, pplib::FileObject& file)
{
    file.fputc('[');
    for (size_t i = 0; i < data.size(); i++) {
        if (i > 0) file.fputc(',');
        file.putsf("\"%s\"", (const char*)data.getPtr(i));
    }
    file.fputc(']');
}

static void writeDict(const pplib::AssocArray& data, pplib::FileObject& file)
{
    if (isArray(data)) {
        writeArray(data, file);
        return;
    }
    file.fputc('{');
    pplib::AssocArray::const_iterator it;
    for (it = data.begin(); it != data.end(); ++it) {
        if (it != data.begin()) file.fputc(',');
        const pplib::String& key = (*it).first;
        file.putsf("\"%s\":", (const char*)key);
        writeValue(file, key, (*it).second);
    }
    file.fputc('}');
}

void Json::dump(pplib::FileObject& file, const pplib::AssocArray& data)
{
    file.rewind();
    file.truncate(0);
    writeDict(data, file);
}

} // end of namespace pplib
