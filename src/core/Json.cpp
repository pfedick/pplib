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
#include <pplib/types/bytearray.h>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace pplib
{

static constexpr int MAX_JSON_DEPTH = 128;

// ============================================================================
// Gepufferter Reader mit 1-Zeichen-Lookahead (Stream- und Pipe-tauglich)
// ============================================================================

class JsonReader
{
private:
    static constexpr size_t BUFFER_SIZE = 4096;
    pplib::FileObject& file;
    ByteArray ba;
    char* buffer = nullptr;
    size_t bufPos = 0;
    size_t bufLen = 0;
    int64_t totalRead = 0;
    int peekChar = -1;

    bool refill()
    {
        if (bufPos < bufLen) return true;
        bufPos = 0;
        bufLen = 0;
        try {
            size_t n = file.fread(buffer, 1, BUFFER_SIZE);
            bufLen = n;
            return bufLen > 0;
        }
        catch (const pplib::EndOfFileException&) {
            return false;
        }
    }

public:
    explicit JsonReader(pplib::FileObject& f)
        : file(f)
    {
        buffer = static_cast<char*>(ba.malloc(BUFFER_SIZE));
    }

    int getChar()
    {
        if (peekChar != -1) {
            int c = peekChar;
            peekChar = -1;
            totalRead++;
            return c;
        }
        if (bufPos >= bufLen) {
            if (!refill()) return EOF;
        }
        totalRead++;
        return static_cast<unsigned char>(buffer[bufPos++]);
    }

    void ungetChar(int c)
    {
        if (c == EOF) return;
        peekChar = c;
        totalRead--;
    }

    int64_t tell() const
    {
        return totalRead;
    }

    bool eof()
    {
        if (peekChar != -1) return false;
        if (bufPos < bufLen) return false;
        return !refill();
    }
};

// ============================================================================
// Forward Declarations
// ============================================================================
static pplib::Variant parseValue(JsonReader& reader, int c, int depth);
static pplib::AssocArray parseDict(JsonReader& reader, int depth);
static pplib::VariantArray parseArray(JsonReader& reader, int depth);

static void writeVariant(pplib::FileObject& file, const pplib::Variant& value, int indent, int level, int depth);
static void writeDict(pplib::FileObject& file, const pplib::AssocArray& data, int indent, int level, int depth);
static void writeArray(pplib::FileObject& file, const pplib::VariantArray& data, int indent, int level, int depth);
static void writeClassicArray(pplib::FileObject& file, const pplib::Array& data, int indent, int level);

// ============================================================================
// Hilfsfunktionen fürs Parsen
// ============================================================================

static void skipToEOL(JsonReader& reader)
{
    while (!reader.eof()) {
        int c = reader.getChar();
        if (c == '\n' || c == EOF) return;
    }
}

static void skipBlockComment(JsonReader& reader)
{
    while (!reader.eof()) {
        int c = reader.getChar();
        if (c == EOF) throw pplib::UnexpectedEndOfDataException();
        if (c == '*') {
            int c2 = reader.getChar();
            if (c2 == '/') return;
            if (c2 != EOF) reader.ungetChar(c2);
        }
    }
    throw pplib::UnexpectedEndOfDataException();
}

static int nextNonWhitespace(JsonReader& reader)
{
    while (!reader.eof()) {
        int c = reader.getChar();
        if (c == EOF) return EOF;
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;
        if (c == '/') {
            int c2 = reader.getChar();
            if (c2 == '/') {
                skipToEOL(reader);
                continue;
            } else if (c2 == '*') {
                skipBlockComment(reader);
                continue;
            } else {
                if (c2 != EOF) {
                    reader.ungetChar(c2);
                }
                return c;
            }
        }
        return c;
    }
    return EOF;
}

static void readChars(JsonReader& reader, const char* chars)
{
    int c, p = 0;
    while (chars[p] != 0) {
        c = reader.getChar();
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
    } else if (codePoint <= 0x10FFFF) {
        char buf[4];
        buf[0] = static_cast<char>(0xF0 | (codePoint >> 18));
        buf[1] = static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
        buf[2] = static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        buf[3] = static_cast<char>(0x80 | (codePoint & 0x3F));
        str.append(buf, 4);
    } else {
        throw pplib::CharacterEncodingException("Unicode code point 0x%X out of valid range (max 0x10FFFF)", codePoint);
    }
}

static pplib::String getString(JsonReader& reader)
{
    pplib::String str;
    while (!reader.eof()) {
        int c = reader.getChar();
        if (c == '\\') {
            c = reader.getChar();
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
                    int h = reader.getChar();
                    if (h == EOF) throw pplib::UnexpectedEndOfDataException();
                    if (!isxdigit(h)) {
                        throw pplib::InvalidEscapeSequenceException("Invalid hex digit in \\u escape sequence: >>%c<<", h);
                    }
                    hex[i] = static_cast<char>(h);
                }
                hex[4] = 0;
                unsigned int codePoint = strtoul(hex, nullptr, 16);

                // Handle surrogate pairs
                if (codePoint >= 0xD800 && codePoint <= 0xDBFF) {
                    int nextC = reader.getChar();
                    if (nextC == EOF) throw pplib::UnexpectedEndOfDataException();
                    if (nextC == '\\') {
                        int nextNextC = reader.getChar();
                        if (nextNextC == EOF) throw pplib::UnexpectedEndOfDataException();
                        if (nextNextC == 'u') {
                            for (int i = 0; i < 4; i++) {
                                int h = reader.getChar();
                                if (h == EOF) throw pplib::UnexpectedEndOfDataException();
                                if (!isxdigit(h)) {
                                    throw pplib::InvalidEscapeSequenceException("Invalid hex digit in \\u escape sequence: >>%c<<", h);
                                }
                                hex[i] = static_cast<char>(h);
                            }
                            hex[4] = 0;
                            unsigned int lowSurrogate = strtoul(hex, nullptr, 16);
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
            } else {
                throw pplib::InvalidEscapeSequenceException("\\%c", c);
            }
        } else if (c == '"') {
            return str;
        } else if (c == EOF) {
            break;
        } else if (static_cast<unsigned char>(c) < 0x20) {
            throw pplib::UnexpectedCharacterException("Unescaped control character (ASCII %d) in string at position %lld", c,
                                                      reader.tell());
        } else {
            str.append(static_cast<char>(c));
        }
    }
    throw pplib::UnexpectedEndOfDataException();
}

static pplib::Variant getNumber(JsonReader& reader, int firstChar)
{
    std::string str;
    str.push_back(static_cast<char>(firstChar));

    while (!reader.eof()) {
        int c = reader.getChar();
        if (c == EOF) break;
        if (c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-' || (c >= '0' && c <= '9')) {
            str.push_back(static_cast<char>(c));
        } else {
            reader.ungetChar(c);
            break;
        }
    }

    // Validate RFC 8259 number format:
    // [ minus ] int [ frac ] [ exp ]
    size_t i = 0;
    size_t n = str.size();
    if (i < n && str[i] == '-') {
        i++;
    }
    if (i >= n) {
        throw pplib::UnexpectedCharacterException("Invalid number >>%s<< at position %lld", str.c_str(), reader.tell());
    }

    if (str[i] == '0') {
        i++;
        // No leading zeros allowed (e.g. "01" or "-01" is invalid)
        if (i < n && str[i] >= '0' && str[i] <= '9') {
            throw pplib::UnexpectedCharacterException("Leading zero in number >>%s<< at position %lld", str.c_str(), reader.tell());
        }
    } else if (str[i] >= '1' && str[i] <= '9') {
        while (i < n && str[i] >= '0' && str[i] <= '9') {
            i++;
        }
    } else {
        throw pplib::UnexpectedCharacterException("Invalid number >>%s<< at position %lld", str.c_str(), reader.tell());
    }

    bool isFloat = false;

    // Optional fraction
    if (i < n && str[i] == '.') {
        isFloat = true;
        i++;
        if (i >= n || str[i] < '0' || str[i] > '9') {
            throw pplib::UnexpectedCharacterException("Decimal point without digits in >>%s<< at position %lld", str.c_str(),
                                                      reader.tell());
        }
        while (i < n && str[i] >= '0' && str[i] <= '9') {
            i++;
        }
    }

    // Optional exponent
    if (i < n && (str[i] == 'e' || str[i] == 'E')) {
        isFloat = true;
        i++;
        if (i < n && (str[i] == '+' || str[i] == '-')) {
            i++;
        }
        if (i >= n || str[i] < '0' || str[i] > '9') {
            throw pplib::UnexpectedCharacterException("Exponent without digits in >>%s<< at position %lld", str.c_str(), reader.tell());
        }
        while (i < n && str[i] >= '0' && str[i] <= '9') {
            i++;
        }
    }

    if (i != n) {
        throw pplib::UnexpectedCharacterException("Invalid characters in number >>%s<< at position %lld", str.c_str(), reader.tell());
    }

    // Parse with std::from_chars (completely locale-independent!)
    if (isFloat) {
        double val = 0.0;
        auto [p, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
        if (ec != std::errc()) {
            throw pplib::UnexpectedCharacterException("Invalid floating point number: >>%s<< at position %lld", str.c_str(), reader.tell());
        }
        return pplib::Variant(val);
    } else {
        int64_t val = 0;
        auto [p, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
        if (ec == std::errc()) {
            return pplib::Variant(val);
        } else if (ec == std::errc::result_out_of_range) {
            double d = 0.0;
            auto [pd, ecd] = std::from_chars(str.data(), str.data() + str.size(), d);
            if (ecd == std::errc()) {
                return pplib::Variant(d);
            }
            throw pplib::UnexpectedCharacterException("Number out of range: >>%s<< at position %lld", str.c_str(), reader.tell());
        } else {
            throw pplib::UnexpectedCharacterException("Invalid integer: >>%s<< at position %lld", str.c_str(), reader.tell());
        }
    }
}

static pplib::Variant parseValue(JsonReader& reader, int c, int depth)
{
    if (depth > MAX_JSON_DEPTH) {
        throw pplib::UnexpectedCharacterException("JSON nesting depth exceeded maximum limit (%d) at position %lld", MAX_JSON_DEPTH,
                                                  reader.tell());
    }
    if (c == '"') {
        return pplib::Variant(getString(reader));
    } else if (c == '{') {
        return pplib::Variant(parseDict(reader, depth));
    } else if (c == '[') {
        return pplib::Variant(parseArray(reader, depth));
    } else if (c == '-' || (c >= '0' && c <= '9')) {
        return getNumber(reader, c);
    } else if (c == 't') {
        readChars(reader, "rue");
        return pplib::Variant(true);
    } else if (c == 'f') {
        readChars(reader, "alse");
        return pplib::Variant(false);
    } else if (c == 'n') {
        readChars(reader, "ull");
        return pplib::Variant(nullptr);
    }
    throw pplib::UnexpectedCharacterException("Unexpected character >>%c<< (ASCII %d) at position %lld", c, c, reader.tell());
}

static pplib::VariantArray parseArray(JsonReader& reader, int depth)
{
    pplib::VariantArray arr;
    int c = nextNonWhitespace(reader);
    if (c == EOF) throw pplib::UnexpectedEndOfDataException();
    if (c == ']') return arr;

    while (true) {
        pplib::Variant val = parseValue(reader, c, depth + 1);
        arr.add(std::move(val));

        c = nextNonWhitespace(reader);
        if (c == EOF) throw pplib::UnexpectedEndOfDataException();
        if (c == ',') {
            c = nextNonWhitespace(reader);
            if (c == EOF) throw pplib::UnexpectedEndOfDataException();
            if (c == ']') {
                throw pplib::UnexpectedCharacterException("Trailing comma in array at position %lld", reader.tell());
            }
            continue;
        } else if (c == ']') {
            return arr;
        } else {
            throw pplib::UnexpectedCharacterException("Expected ',' or ']' in array at position %lld, got >>%c<<", reader.tell(), c);
        }
    }
}

static pplib::AssocArray parseDict(JsonReader& reader, int depth)
{
    pplib::AssocArray dict;
    int c = nextNonWhitespace(reader);
    if (c == EOF) throw pplib::UnexpectedEndOfDataException();
    if (c == '}') return dict;

    while (true) {
        if (c != '"') {
            throw pplib::UnexpectedCharacterException("Expected '\"' for object key at position %lld, got >>%c<<", reader.tell(), c);
        }
        pplib::String key = getString(reader);
        if (key.isEmpty()) key = "_empty_";

        c = nextNonWhitespace(reader);
        if (c != ':') {
            throw pplib::UnexpectedCharacterException("Expected ':' after key >>%s<< at position %lld, got >>%c<<", (const char*)key,
                                                      reader.tell(), c);
        }

        c = nextNonWhitespace(reader);
        if (c == EOF) throw pplib::UnexpectedEndOfDataException();
        pplib::Variant val = parseValue(reader, c, depth + 1);
        dict.set(key, std::move(val));

        c = nextNonWhitespace(reader);
        if (c == EOF) throw pplib::UnexpectedEndOfDataException();
        if (c == ',') {
            c = nextNonWhitespace(reader);
            if (c == EOF) throw pplib::UnexpectedEndOfDataException();
            if (c == '}') {
                throw pplib::UnexpectedCharacterException("Trailing comma in object at position %lld", reader.tell());
            }
            continue;
        } else if (c == '}') {
            return dict;
        } else {
            throw pplib::UnexpectedCharacterException("Expected ',' or '}' in object at position %lld, got >>%c<<", reader.tell(), c);
        }
    }
}

static void expectEof(JsonReader& reader)
{
    int c = nextNonWhitespace(reader);
    if (c != EOF) {
        throw pplib::UnexpectedCharacterException("Garbage after JSON end: >>%c<< at position %lld", c, reader.tell());
    }
}

// ============================================================================
// Public Parse-Methoden
// ============================================================================

void Json::load(pplib::Variant& data, pplib::FileObject& file)
{
    try {
        JsonReader reader(file);
        int c = nextNonWhitespace(reader);
        if (c == EOF) throw pplib::UnexpectedEndOfDataException();
        data = parseValue(reader, c, 0);
        expectEof(reader);
    }
    catch (const pplib::EndOfFileException&) {
        throw pplib::UnexpectedEndOfDataException();
    }
}

void Json::loads(pplib::Variant& data, const pplib::String& json)
{
    if (json.isEmpty()) throw pplib::UnexpectedEndOfDataException();
    pplib::MemFile file(const_cast<char*>(json.c_str()), json.size(), false);
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
    if (json.isEmpty()) throw pplib::UnexpectedEndOfDataException();
    pplib::MemFile file(const_cast<char*>(json.c_str()), json.size(), false);
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
    if (json.isEmpty()) throw pplib::UnexpectedEndOfDataException();
    pplib::MemFile file(const_cast<char*>(json.c_str()), json.size(), false);
    load(data, file);
}

// ============================================================================
// Hilfsfunktionen fürs Serialisieren
// ============================================================================

static void writeEscapedString(pplib::FileObject& file, const char* p, size_t len)
{
    file.fputc('"');
    for (size_t i = 0; i < len; ++i) {
        char c = p[i];
        switch (c) {
        case '"':
            file.fputs("\\\"");
            break;
        case '\\':
            file.fputs("\\\\");
            break;
        case '\b':
            file.fputs("\\b");
            break;
        case '\f':
            file.fputs("\\f");
            break;
        case '\n':
            file.fputs("\\n");
            break;
        case '\r':
            file.fputs("\\r");
            break;
        case '\t':
            file.fputs("\\t");
            break;
        default:
            if (static_cast<unsigned char>(c) < 32) {
                char ubuf[8];
                snprintf(ubuf, sizeof(ubuf), "\\u%04x", static_cast<unsigned char>(c));
                file.fputs(ubuf);
            } else {
                file.fputc(c);
            }
            break;
        }
    }
    file.fputc('"');
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

static void writeDict(pplib::FileObject& file, const pplib::AssocArray& data, int indent, int level, int depth)
{
    if (depth > MAX_JSON_DEPTH) {
        throw pplib::OverflowException("JSON nesting depth exceeded maximum limit (%d)", MAX_JSON_DEPTH);
    }
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
        writeEscapedString(file, (*it).first.c_str(), (*it).first.size());
        file.fputc(':');
        if (indent >= 0) file.fputc(' ');
        writeVariant(file, *(*it).second, indent, level + 1, depth + 1);
    }
    writeIndent(file, indent, level);
    file.fputc('}');
}

static void writeArray(pplib::FileObject& file, const pplib::VariantArray& data, int indent, int level, int depth)
{
    if (depth > MAX_JSON_DEPTH) {
        throw pplib::OverflowException("JSON nesting depth exceeded maximum limit (%d)", MAX_JSON_DEPTH);
    }
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
        writeVariant(file, data[i], indent, level + 1, depth + 1);
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
        const pplib::String& s = data.get(i);
        writeEscapedString(file, s.c_str(), s.size());
    }
    writeIndent(file, indent, level);
    file.fputc(']');
}

static void writeVariant(pplib::FileObject& file, const pplib::Variant& value, int indent, int level, int depth)
{
    if (depth > MAX_JSON_DEPTH) {
        throw pplib::OverflowException("JSON nesting depth exceeded maximum limit (%d)", MAX_JSON_DEPTH);
    }
    if (value.isString()) {
        const pplib::String& s = value.toString();
        writeEscapedString(file, s.c_str(), s.size());
    } else if (value.isWideString()) {
        pplib::ByteArray ba = value.toWideString().toUtf8();
        writeEscapedString(file, reinterpret_cast<const char*>(ba.ptr()), ba.size());
    } else if (value.isInt64()) {
        char buf[32];
        auto [p, ec] = std::to_chars(buf, buf + sizeof(buf), value.toInt64());
        if (ec == std::errc()) {
            file.fwrite(buf, 1, p - buf);
        } else {
            file.putsf("%lld", static_cast<long long>(value.toInt64()));
        }
    } else if (value.isDouble()) {
        double d = value.toDouble();
        if (std::isnan(d) || std::isinf(d)) {
            file.fputs("null");
        } else {
            char buf[64];
            auto [p, ec] = std::to_chars(buf, buf + sizeof(buf), d);
            if (ec == std::errc()) {
                file.fwrite(buf, 1, p - buf);
            } else {
                snprintf(buf, sizeof(buf), "%.17g", d);
                for (char* cp = buf; *cp; ++cp) {
                    if (*cp == ',') *cp = '.';
                }
                file.fputs(buf);
            }
        }
    } else if (value.isBool()) {
        file.fputs(value.toBool() ? "true" : "false");
    } else if (value.isNull()) {
        file.fputs("null");
    } else if (value.isVariantArray()) {
        writeArray(file, value.toVariantArray(), indent, level, depth + 1);
    } else if (value.isArray()) {
        writeClassicArray(file, value.toArray(), indent, level);
    } else if (value.isAssocArray()) {
        writeDict(file, value.toAssocArray(), indent, level, depth + 1);
    } else if (value.isByteArrayPtr() || value.isByteArray()) {
        const pplib::ByteArrayPtr& ba = value.toByteArrayPtr();
        pplib::String str = ba.toBase64();
        writeEscapedString(file, str.c_str(), str.size());
    } else if (value.isDateTime()) {
        pplib::String str = value.toDateTime().getISO8601withUsec();
        writeEscapedString(file, str.c_str(), str.size());
    } else if (value.isDate()) {
        pplib::String str = value.toDate().toString();
        writeEscapedString(file, str.c_str(), str.size());
    } else if (value.isTime()) {
        pplib::String str = value.toTime().toString();
        writeEscapedString(file, str.c_str(), str.size());
    } else if (value.isTimeDelta()) {
        pplib::String str = value.toTimeDelta().toString();
        writeEscapedString(file, str.c_str(), str.size());
    } else if (value.isTimeZone()) {
        pplib::String str = value.toTimeZone().toString();
        writeEscapedString(file, str.c_str(), str.size());
    } else {
        throw pplib::UnsupportedDataTypeException("Variant Type >>%d<< cannot be serialized to JSON", value.type());
    }
}

// ============================================================================
// Public Dump-Methoden
// ============================================================================

void Json::dump(pplib::FileObject& file, const pplib::Variant& data, int indent)
{
    writeVariant(file, data, indent, 0, 0);
    if (indent >= 0) {
        file.fputc('\n');
    }
}

void Json::dumps(pplib::String& json, const pplib::Variant& data, int indent)
{
    pplib::MemFile file;
    Json::dump(file, data, indent);
    size_t size = file.tell();
    if (size == 0)
        json.clear();
    else
        json.set(reinterpret_cast<const char*>(file.map(0, size)), size);
}

pplib::String Json::dumps(const pplib::Variant& data, int indent)
{
    pplib::String result;
    Json::dumps(result, data, indent);
    return result;
}

void Json::dump(pplib::FileObject& file, const pplib::AssocArray& data, int indent)
{
    dump(file, pplib::Variant(data), indent);
}

void Json::dumps(pplib::String& json, const pplib::AssocArray& data, int indent)
{
    dumps(json, pplib::Variant(data), indent);
}

pplib::String Json::dumps(const pplib::AssocArray& data, int indent)
{
    return dumps(pplib::Variant(data), indent);
}

void Json::dump(pplib::FileObject& file, const pplib::VariantArray& data, int indent)
{
    dump(file, pplib::Variant(data), indent);
}

void Json::dumps(pplib::String& json, const pplib::VariantArray& data, int indent)
{
    dumps(json, pplib::Variant(data), indent);
}

pplib::String Json::dumps(const pplib::VariantArray& data, int indent)
{
    return dumps(pplib::Variant(data), indent);
}

pplib::String Json::pp(const pplib::String& json, int indent)
{
    pplib::Variant v = loads(json);
    return dumps(v, indent);
}

} // end of namespace pplib
