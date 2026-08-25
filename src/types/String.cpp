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

#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <vector>

#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/array.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>
#include <pplib/core/iconv.h>

#include <config_pplib.h>

namespace pplib
{

String::String() noexcept
{
    ptr = nullptr;
    stringlen = 0;
    s = 0;
}

String::String(const char* str)
{
    ptr = nullptr;
    stringlen = 0;
    s = 0;
    set(str);
}

String::String(const char* str, size_t size)
{
    ptr = nullptr;
    stringlen = 0;
    s = 0;
    set(str, size);
}

String::String(const wchar_t* str, size_t size)
{
    ptr = nullptr;
    stringlen = 0;
    s = 0;
    set(str, size);
}

String::String(const String& str)
{
    ptr = nullptr;
    stringlen = 0;
    s = 0;
    set(str);
}

String::String(String&& other) noexcept
{
    ptr = other.ptr;
    stringlen = other.stringlen;
    s = other.s;
    other.ptr = nullptr;
    other.stringlen = 0;
    other.s = 0;
}

String::String(const ByteArrayPtr& str)
{
    ptr = nullptr;
    stringlen = 0;
    s = 0;
    set(str);
}

String::String(const std::string& str)
{
    ptr = nullptr;
    stringlen = 0;
    s = 0;
    set(str.data(), str.size());
}

String::String(const std::wstring& str)
{
    ptr = nullptr;
    stringlen = 0;
    s = 0;
    set(str.data(), str.size());
}

String::String(const WideString& str)
{
    ptr = nullptr;
    stringlen = 0;
    s = 0;
    set(str, str.size());
}

String::~String() noexcept
{
    free(ptr);
}

void String::clear() noexcept
{
    free(ptr);
    ptr = nullptr;
    stringlen = 0;
    s = 0;
}

size_t String::capacity() const
{
    if (s == 0) return 0;
    return s - 1;
}

void String::reserve(size_t size)
{
    if (size == std::numeric_limits<size_t>::max()) throw IllegalArgumentException();
    size_t bytes = size + 1;
    if (s >= bytes) return; // Nothing to do
    char* p;
    p = (char*)realloc(ptr, bytes);
    if (!p) throw OutOfMemoryException();
    ptr = p;
    s = bytes;
    ptr[stringlen] = 0;
}

bool String::isNumeric() const
{
    if (!stringlen) return false;
    size_t dotcount = 0;
    for (size_t i = 0; i < stringlen; i++) {
        int c = ((char*)ptr)[i];
        if (c < '0' || c > '9') {
            if (c != '.' && c != ',' && c != '-') return false;
            if (c == '-' && i > 0) return false;
            if (c == '.' || c == ',') {
                dotcount++;
                if (dotcount > 1) return false;
            }
        }
    }
    if (ptr[stringlen - 1] == '.') return false;
    return (true);
}

bool String::isInteger() const
{
    if (!stringlen) return false;
    for (size_t i = 0; i < stringlen; i++) {
        int c = ((char*)ptr)[i];
        if (c < '0' || c > '9') {
            if (c == '-' && i == 0) continue; // Minus am Anfang ist erlaubt
            return false;
        }
    }
    return true;
}

bool String::isTrue() const
{
    if (!stringlen) return false;
    if (atol(ptr) != 0) return true;
    if (strCaseCmp("true") == 0) return true;
    if (strCaseCmp("wahr") == 0) return true;
    if (strCaseCmp("ja") == 0) return true;
    if (strCaseCmp("yes") == 0) return true;
    if (strCaseCmp("t") == 0) return true;
    return false;
}

bool String::isFalse() const
{
    if (isTrue()) return false;
    return true;
}

String& String::set(const char* str, size_t size)
{
    if (!str) {
        clear();
        return *this;
    }
    size_t inbytes = (size != (size_t)-1) ? size : ::strlen(str);
    if (size > strlen(str)) inbytes = strlen(str);
    if (inbytes == 0) {
        clear();
        return *this;
    }

    // Self-Assignment Schutz
    String temp_holder;
    if (str >= ptr && str < ptr + stringlen) {
        temp_holder.set(str, inbytes);
        str = temp_holder.c_str();
    }

    if (inbytes >= s) {
        reserve(inbytes);
    }
    memmove((char*)ptr, str, inbytes);
    stringlen = inbytes;
    ((char*)ptr)[stringlen] = 0;
    return *this;
}

String& String::set(const wchar_t* str, size_t size)
{
    if (str == NULL || size == 0) {
        clear();
        return *this;
    }
    size_t inchars = (size != (size_t)-1) ? size : ::wcslen(str);
    if (size > ::wcslen(str)) inchars = ::wcslen(str);

    // Schutz vor Buffer Overread bei Teil-Wide-Strings
    std::vector<wchar_t> temp_wstr;
    if (size != (size_t)-1) {
        temp_wstr.assign(str, str + inchars);
        temp_wstr.push_back(0);
        str = temp_wstr.data();
    }

    // Abschätzung der maximalen UTF-8 Bytegröße (1 wchar_t kann maximal 4 UTF-8 Bytes erzeugen)
    size_t outbytes = inchars * 4 + 1;
    reserve(outbytes);
    size_t formatted_bytes = ::wcstombs(ptr, str, outbytes);
    if (formatted_bytes == (size_t)-1) {
        clear();
        throw CharacterEncodingException();
    }

    stringlen = formatted_bytes;
    ptr[stringlen] = 0;
    return *this;
}

String& String::set(const String& str, size_t size)
{
    size_t inbytes;
    if (size != (size_t)-1)
        inbytes = size;
    else
        inbytes = str.stringlen;
    if (inbytes > str.stringlen) inbytes = str.stringlen;
    return set(str.ptr, inbytes);
}

String& String::set(const ByteArrayPtr& str, size_t size)
{
    size_t inbytes;
    if (size != (size_t)-1)
        inbytes = size;
    else
        inbytes = str.size();
    if (inbytes > str.size()) inbytes = str.size();
    return set((const char*)str.adr(), inbytes);
}

String& String::set(const WideString& str, size_t size)
{
    size_t inbytes;
    if (size != (size_t)-1)
        inbytes = size;
    else
        inbytes = str.size();
    if (inbytes > str.size()) inbytes = str.size();
    return set(str.getPtr(), inbytes);
}

String& String::set(const std::string& str, size_t size)
{
    size_t inbytes;
    if (size != (size_t)-1)
        inbytes = size;
    else
        inbytes = str.length();
    if (inbytes > str.length()) inbytes = str.length();
    return set((const char*)str.c_str(), inbytes);
}

String& String::set(const std::wstring& str, size_t size)
{
    size_t inbytes;
    if (size != (size_t)-1)
        inbytes = size;
    else
        inbytes = str.length();
    if (inbytes > str.length()) inbytes = str.length();
    return set(str.c_str(), inbytes);
}

String& String::set(size_t position, char c)
{
    if (position >= stringlen) throw OutOfBoundsException();
    ptr[position] = c;
    return *this;
}

String& String::setf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    this->vasprintf(fmt, args);
    va_end(args);
    return *this;
}

String String::format(const char* fmt, ...)
{
    String s;
    va_list args;
    va_start(args, fmt);
    s.vasprintf(fmt, args);
    va_end(args);
    return s;
}

String& String::set(char c)
{
    char buffer[2];
    buffer[0] = c;
    buffer[1] = 0;
    return set(buffer, 1);
}

String& String::vasprintf(const char* fmt, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);

    // 1. Benötigte Größe bestimmen (ohne in einen echten Puffer zu schreiben)
    int size = ::vsnprintf(nullptr, 0, fmt, args_copy);
    va_end(args_copy);

    if (size < 0) {
        throw Exception("String::vasprintf failed to format");
    }

    // 2. Speicher direkt im String-Objekt reservieren (+1 für das Nullbyte)
    size_t required_bytes = size + 1;
    reserve(required_bytes);

    // 3. Direkt in den eigenen Puffer schreiben
    ::vsnprintf(ptr, required_bytes, fmt, args);
    stringlen = size;
    ptr[stringlen] = 0;
    return *this;
}

String& String::append(const wchar_t* str, size_t size)
{
    String a;
    a.set(str, size);
    return append((const char*)a, a.size());
}

String& String::append(const char* str, size_t size)
{
    if (str == NULL || size == 0) return *this;
    if (stringlen == 0) {
        return set(str, size);
    }

    size_t inchars = (size != (size_t)-1) ? size : ::strlen(str);
    // Self-Append Schutz: Zeigt "str" auf unseren eigenen Speicher block?
    String temp_holder;
    if (str >= ptr && str < ptr + stringlen) {
        temp_holder.set(str, inchars);
        str = temp_holder.c_str(); // Zeigt jetzt auf einen sicheren Stack-Puffer
    }

    size_t required_bytes = stringlen + inchars + 1;

    if (required_bytes >= s) {
        // Geometrisches Wachstum: Wir verdoppeln die Kapazität
        size_t newbuffersize = s * 2;
        if (newbuffersize < required_bytes) {
            newbuffersize = required_bytes + 16; // Fallback, falls Verdopplung nicht reicht
        }
        char* t = (char*)realloc(ptr, newbuffersize);
        if (!t) throw OutOfMemoryException();
        ptr = t;
        s = newbuffersize;
    }
    memcpy(ptr + stringlen, str, inchars);
    stringlen += inchars;
    ptr[stringlen] = 0;
    return *this;
}

String& String::append(const String& str, size_t size)
{
    return append(str.ptr, size);
}

String& String::append(const WideString& str, size_t size)
{
    String a;
    a.set(str, size);
    return append(a.ptr, a.stringlen);
}

String& String::append(const std::string& str, size_t size)
{
    if (size == (size_t)-1) return append(str.data(), str.size());
    return append(str.data(), size);
}

String& String::append(const std::wstring& str, size_t size)
{
    String a;
    a.set(str, size);
    return append(a.ptr, a.stringlen);
}

String& String::appendf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String s;
    s.vasprintf(fmt, args);
    va_end(args);
    return append(s.ptr, s.stringlen);
}

String& String::append(char c)
{
    char buffer[2];
    buffer[0] = c;
    buffer[1] = 0;
    return append(buffer, 1);
}

String& String::prepend(const wchar_t* str, size_t size)
{
    String a;
    a.set(str, size);
    return prepend((const char*)a.ptr, size);
}

String& String::prepend(const String& str, size_t size)
{
    return prepend(str.ptr, size);
}

String& String::prepend(const WideString& str, size_t size)
{
    String a;
    a.set(str, size);
    return prepend(a.ptr, a.stringlen);
}

String& String::prepend(const std::string& str, size_t size)
{
    if (size == (size_t)-1) return prepend(str.data(), str.size());
    return prepend(str.data(), size);
}

String& String::prepend(const std::wstring& str, size_t size)
{
    if (stringlen == 0) {
        return set(str, size);
    }
    String a;
    a.set(str, size);
    return prepend(a.ptr, a.stringlen);
}

String& String::prepend(const char* str, size_t size)
{
    if (str == NULL || size == 0) return *this;
    if (stringlen == 0) {
        return set(str, size);
    }
    size_t inchars = (size != (size_t)-1) ? size : ::strlen(str);
    // Self-Prepend Schutz: Zeigt "str" auf unseren eigenen Speicher block?
    String temp_holder;
    if (str >= ptr && str < ptr + stringlen) {
        temp_holder.set(str, inchars);
        str = temp_holder.c_str(); // Zeigt jetzt auf einen sicheren Stack-Puffer
    }

    size_t required_bytes = stringlen + inchars + 1;

    if (required_bytes >= s) {
        // Geometrisches Wachstum: Wir verdoppeln die Kapazität
        size_t newbuffersize = s * 2;
        if (newbuffersize < required_bytes) {
            newbuffersize = required_bytes + 16; // Fallback, falls Verdopplung nicht reicht
        }
        char* t = (char*)realloc(ptr, newbuffersize);
        if (!t) throw OutOfMemoryException();
        ptr = t;
        s = newbuffersize;
    }
    // Bestehenden Speicherblock nach hinten moven
    memmove(((char*)ptr) + inchars, ptr, stringlen);
    // Neuen Speicherblock davor kopieren
    memcpy(ptr, str, inchars);
    stringlen += inchars;
    ptr[stringlen] = 0;
    return *this;
}

String& String::prependf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String s;
    s.vasprintf(fmt, args);
    va_end(args);
    return prepend(s.ptr, s.stringlen);
}

String& String::prepend(char c)
{
    char buffer[2];
    buffer[0] = c;
    buffer[1] = 0;
    return prepend(buffer, 1);
}

char String::get(ssize_t pos) const
{
    if (pos >= 0 && stringlen > (size_t)pos) return ptr[pos];
    if (pos < 0 && (size_t)(0 - pos) <= stringlen) return ptr[stringlen + pos];
    throw OutOfBoundsException();
}

char String::operator[](ssize_t pos) const
{
    if (pos >= 0 && stringlen > (size_t)pos) return ((char*)ptr)[pos];
    if (pos < 0 && (size_t)(0 - pos) <= stringlen) return ((char*)ptr)[stringlen + pos];
    throw OutOfBoundsException();
}

char& String::operator[](ssize_t pos)
{
    if (pos >= 0 && stringlen > (size_t)pos) return ((char*)ptr)[pos];
    if (pos < 0 && (size_t)(0 - pos) <= stringlen) return ((char*)ptr)[stringlen + pos];
    throw OutOfBoundsException();
}

void String::print(bool withNewline) const noexcept
{
    if (stringlen > 0) {
        if (withNewline)
            printf("%s\n", (char*)ptr);
        else
            printf("%s", (char*)ptr);
    } else if (withNewline) {
        printf("\n");
    }
}

void String::printnl() const noexcept
{
    print(true);
}

void String::hexDump() const
{
    PrintDebug("HEXDUMP of String %p: %zi Bytes starting at Address %p:\n", this, stringlen * sizeof(char), ptr);
    if (stringlen) HexDump(ptr, stringlen * sizeof(char), true);
}

String& String::operator=(const char* str)
{
    return set(str);
}

String& String::operator=(const wchar_t* str)
{
    return set(str);
}

String& String::operator=(const String& str)
{
    return set(str);
}

String& String::operator=(String&& other) noexcept
{
    if (this != &other) {
        free(ptr);
        ptr = other.ptr;
        s = other.s;
        stringlen = other.stringlen;
        other.ptr = nullptr;
        other.s = 0;
        other.stringlen = 0;
    }
    return *this;
}

String& String::operator=(const WideString& str)
{
    return set(str);
}

String& String::operator=(const std::string& str)
{
    return set(str);
}

String& String::operator=(const std::wstring& str)
{
    return set(str);
}

String& String::operator=(char c)
{
    return set(c);
}

String& String::operator+=(const char* str)
{
    return append(str);
}

String& String::operator+=(const wchar_t* str)
{
    return append(str);
}

String& String::operator+=(const String& str)
{
    return append(str);
}

String& String::operator+=(const WideString& str)
{
    return append(str);
}

String& String::operator+=(const std::string& str)
{
    return append(str);
}

String& String::operator+=(const std::wstring& str)
{
    return append(str);
}

String& String::operator+=(char c)
{
    return append(c);
}

int String::strcmp(const String& str, size_t size) const
{
    return strcmp(str.ptr, size);
}

int String::strcmp(const char* str, size_t size) const
{
    const char* p1 = ptr ? ptr : "";
    const char* p2 = str ? str : "";
    if (size != (size_t)-1) return ::strncmp(p1, p2, size);
    return ::strcmp(p1, p2);
}

int String::strCaseCmp(const String& str, size_t size) const
{
    return strCaseCmp(str.ptr, size);
}

int String::strCaseCmp(const char* str, size_t size) const
{
    const char* p1 = ptr ? ptr : "";
    const char* p2 = str ? str : "";
    if (size != (size_t)-1) return ::strncasecmp(p1, p2, size);
    return ::strcasecmp(p1, p2);
}

String String::left(size_t len) const
{
    if (len > stringlen) len = stringlen;
    return String(ptr, len);
}

String String::right(size_t len) const
{
    if (len > stringlen) len = stringlen;
    return String(ptr + stringlen - len, len);
}

String String::mid(size_t start, size_t len) const
{
    if (len == (size_t)-1) len = stringlen;
    if (start < stringlen && stringlen > 0 && len > 0) {
        if (start + len > stringlen) len = stringlen - start;
        return String(ptr + start, len);
    }
    return String();
}

String String::substr(size_t start, size_t len) const
{
    return mid(start, len);
}

String& String::lowerCase()
{
    if (stringlen == 0) return *this;
    // Wir wandeln den String zunächst nach Unicode um
    WideString ws(ptr, stringlen);
    ws.lowerCase();
    set(ws);
    return *this;
}

String& String::upperCase()
{
    if (stringlen == 0) return *this;
    // Wir wandeln den String zunächst nach Unicode um
    WideString ws(ptr, stringlen);
    ws.upperCase();
    set(ws);
    return *this;
}

String& String::trim()
{
    if (stringlen == 0) return *this;

    size_t start = 0;
    while (start < stringlen && (ptr[start] == ' ' || ptr[start] == '\t' || ptr[start] == '\r' || ptr[start] == '\n')) {
        start++;
    }

    if (start == stringlen) {
        clear();
        return *this;
    }

    size_t end = stringlen - 1;
    while (end > start && (ptr[end] == ' ' || ptr[end] == '\t' || ptr[end] == '\r' || ptr[end] == '\n')) {
        end--;
    }

    size_t new_len = end - start + 1;
    if (start > 0) {
        memmove(ptr, ptr + start, new_len);
    }
    stringlen = new_len;
    ptr[stringlen] = 0;
    return *this;
}

String String::trimmed() const
{
    String ret = *this;
    ret.trim();
    return ret;
}

String String::toLowerCase() const
{
    String res(*this);
    res.lowerCase();
    return res;
}

String String::toUpperCase() const
{
    String res(*this);
    res.upperCase();
    return res;
}

//! \brief Schneidet Leerzeichen, Tabs Returns und Linefeeds am Anfang des Strings ab
String& String::trimLeft()
{
    if (stringlen == 0) return *this;
    size_t start = 0;
    while (start < stringlen && (ptr[start] == ' ' || ptr[start] == '\t' || ptr[start] == '\r' || ptr[start] == '\n')) {
        start++;
    }

    if (start == stringlen) {
        clear();
        return *this;
    }
    if (start > 0) {
        size_t new_len = stringlen - start;
        memmove(ptr, ptr + start, new_len + 1); // +1 kopiert das Nullbyte direkt mit
        stringlen = new_len;
    }
    return *this;
}

//! \brief Schneidet Leerzeichen, Tabs Returns und Linefeeds am Ende des Strings ab
String& String::trimRight()
{
    if (stringlen == 0) return *this;
    size_t end = stringlen;
    while (end > 0 && (ptr[end - 1] == ' ' || ptr[end - 1] == '\t' || ptr[end - 1] == '\r' || ptr[end - 1] == '\n')) {
        end--;
    }
    if (end == 0) {
        clear();
    } else {
        stringlen = end;
        ptr[stringlen] = 0;
    }
    return *this;
}

String& String::trimLeft(const String& chars)
{
    if (stringlen == 0 || chars.isEmpty()) return *this;

    size_t start = 0;
    while (start < stringlen) {
        bool match = false;
        for (size_t z = 0; z < chars.stringlen; z++) {
            if (ptr[start] == chars.ptr[z]) {
                match = true;
                break;
            }
        }
        if (!match) break;
        start++;
    }

    if (start == stringlen) {
        clear();
        return *this;
    }
    if (start > 0) {
        size_t new_len = stringlen - start;
        memmove(ptr, ptr + start, new_len + 1); // Kopiert das Nullbyte direkt mit
        stringlen = new_len;
    }
    return *this;
}

String& String::trimRight(const String& chars)
{
    if (stringlen == 0 || chars.isEmpty()) return *this;

    size_t end = stringlen;
    while (end > 0) {
        bool match = false;
        for (size_t z = 0; z < chars.stringlen; z++) {
            if (ptr[end - 1] == chars.ptr[z]) {
                match = true;
                break;
            }
        }
        if (!match) break;
        end--;
    }

    if (end == 0) {
        clear();
    } else {
        stringlen = end;
        ptr[stringlen] = 0;
    }
    return *this;
}

//! \brief Schneidet die definierten Zeichen am Anfang und Ende des Strings ab
String& String::trim(const String& chars)
{
    trimLeft(chars);
    trimRight(chars);
    return *this;
}

String& String::chopRight(size_t num)
{
    if (stringlen > 0) {
        if (stringlen < num) num = stringlen;
        stringlen -= num;
        ptr[stringlen] = 0;
    }
    return *this;
}

String& String::chopLeft(size_t num)
{
    if (stringlen > 0) {
        if (stringlen < num) num = stringlen;
        memmove(ptr, ptr + num, (stringlen - num));
        stringlen -= num;
        ptr[stringlen] = 0;
    }
    return *this;
}

String& String::chomp()
{
    trim("\n\r");
    return *this;
}

String& String::cut(size_t pos)
{
    if (stringlen == 0) return *this;
    if (pos > stringlen) return *this;
    ptr[pos] = 0;
    stringlen = pos;
    return *this;
}

String& String::cut(const String& letter)
{
    if (stringlen == 0) return *this;
    if (letter.isEmpty()) return *this;
    ssize_t p = instr(letter, 0);
    if (p >= 0) {
        ptr[p] = 0;
        stringlen = p;
    }
    return *this;
}

String String::strchr(char c) const
{
    String ret;
    if (stringlen > 0) {
        char* p = ::strchr(ptr, c);
        if (p) ret.set(p);
    }
    return ret;
}

String String::strrchr(char c) const
{
    String ret;
    if (stringlen > 0) {
        char* p = ::strrchr(ptr, c);
        if (p) ret.set(p);
    }
    return ret;
}

String String::strstr(const String& needle) const
{
    String ret;
    if (stringlen > 0) {
        if (needle.len() == 0) return *this;
        char* p = ::strstr(ptr, needle.ptr);
        if (p) ret.set(p);
    }
    return ret;
}

ssize_t String::find(const String& needle, ssize_t start) const
{
    if (stringlen == 0 || needle.stringlen == 0) return String::npos;
    if (start > 0 && (size_t)start >= stringlen) return String::npos;
    if (start < 0 && ((size_t)((ssize_t)stringlen + start)) >= stringlen) return String::npos;

    // Position to return
    size_t p = String::npos;
    // Length of the string to search for
    size_t lstr = needle.stringlen;
    // Current position to search from and position of found string
    char *found = NULL, *tmp = NULL;

    // Search forward
    if (start >= 0) {
        // Search first occurence, starting at the given position...
        found = ::strstr(ptr + start, needle.ptr);
        //...and calculate the position to return if str was found
        if (found != NULL) {
            p = found - ptr;
        }
    } else {
        // Search backward
        // Start counting from behind at 0, not -1
        start++;

        /* Beginning at the start of the contained string, start searching for
               every occurence of the str and make it the position last found as long
               as the found string doesn't exceed the defined end of the search */
        while ((found = ::strstr((tmp == NULL ? ptr : tmp + 1), needle.ptr)) != NULL && found - ptr + lstr <= stringlen + start)
            tmp = found;

        // Calculate the position to return if str was found
        if (tmp != NULL) {
            p = tmp - ptr;
        }
    }
    return p;
}

ssize_t String::findCase(const String& needle, ssize_t start) const
{
    String CaseNeedle(needle);
    String CaseSearch(ptr, stringlen);
    CaseNeedle.lowerCase();
    CaseSearch.lowerCase();
    return CaseSearch.find(CaseNeedle, start);
}

bool String::has(const String& needle, bool ignoreCase) const
{
    if (ignoreCase) {
        String CaseSearch(ptr, stringlen);
        String CaseNeedle(needle);
        CaseNeedle.lowerCase();
        CaseSearch.lowerCase();
        return CaseSearch.has(CaseNeedle, false);
    }
    if (stringlen == 0) return false;
    if (needle.stringlen == 0) return false;
    const char* p = ::strstr(ptr, needle.ptr);
    if (p != NULL) return true;
    return false;
}

String& String::repeat(size_t num)
{
    if (stringlen == 0) return *this;
    if (num == 0) {
        clear();
        return *this;
    }
    reserve(stringlen * num);
    char* tmp = ptr + stringlen;
    for (size_t i = 1; i < num; i++) {
        memcpy(tmp, ptr, stringlen);
        tmp += stringlen;
    }
    stringlen *= num;
    ptr[stringlen] = 0;
    return *this;
}

String& String::repeat(const String& str, size_t num)
{
    if (str.stringlen == 0 || num == 0) {
        clear();
        return *this;
    }

    // Genereller Self-Repeat-Schutz
    String temp_holder;
    const char* src_ptr = str.ptr;
    if (str.ptr >= ptr && str.ptr < ptr + stringlen) {
        temp_holder = str;
        src_ptr = temp_holder.c_str();
    }
    reserve(str.stringlen * num);
    char* dst = ptr;
    for (size_t i = 0; i < num; i++) {
        memcpy(dst, src_ptr, str.stringlen);
        dst += str.stringlen;
    }
    stringlen = str.stringlen * num;
    ptr[stringlen] = 0;
    return *this;
}

String& String::repeat(char code, size_t num)
{
    if (!num || !code) {
        clear();
        return *this;
    }
    reserve(num);
    for (size_t i = 0; i < num; i++)
        ptr[i] = code;
    stringlen = num;
    ptr[stringlen] = 0;
    return *this;
}

String String::repeated(size_t count) const
{
    String ret;
    for (size_t i = 0; i < count; i++)
        ret.append(ptr, stringlen);
    return ret;
}

String& String::replace(const String& search, const String& replacement)
//! \brief Ersetzt einen Teilstring durch einen anderen
{
    if (stringlen == 0 || search.stringlen == 0) return *this;
    size_t start = 0, slen = search.stringlen;
    ssize_t end;
    // collect the result
    String ms;
    // Do while str is found in the contained string
    while ((end = find(search, start)) >= 0) {
        // The result is built from the parts that don't match str and the replacement string
        ms += mid(start, end - start);
        ms += replacement;
        // New start for search is behind the replaced part
        start = end + slen;
    }
    // Add the remaining part of the contained string to the result
    ms += mid(start);
    // The result is assigned to this mstring
    return set(ms);
}

String& String::shl(char c, size_t size)
{
    if (!stringlen || !size) return *this;
    if (size > stringlen) size = stringlen;
    String t = mid(size);
    if (c) {
        String a;
        a.repeat(c, size);
        t += a;
    }
    set(t);
    return *this;
}

String& String::shr(char c, size_t size)
{
    if (!stringlen || !size) return *this;
    if (size > stringlen) size = stringlen;
    String t;
    if (c) {
        t.repeat(c, size);
    }
    t += left(stringlen - size);
    return set(t);
}

String::operator bool() const
{
    if (isTrue()) return true;
    return false;
}

String::operator int() const
{
    if (!stringlen) return 0;
    return strtol(ptr, NULL, 0);
}

String::operator unsigned int() const
{
    if (!stringlen) return 0;
    return strtoul(ptr, NULL, 0);
}

String::operator float() const
{
    return toFloat();
}

String::operator double() const
{
    return toDouble();
}

String::operator std::string() const
{
    return std::string((const char*)ptr, stringlen);
}

String::operator std::wstring() const
{
    if (stringlen == 0) return std::wstring();
    WideString ws(ptr, stringlen);
    return std::wstring(ws.c_str(), ws.size());
}

int String::toInt() const
{
    if (!stringlen) return 0;
    return strtol(ptr, NULL, 10);
}

unsigned int String::toUnsignedInt() const
{
    if (!stringlen) return 0;
    return strtoul(ptr, NULL, 10);
}

int64_t String::toInt64() const
{
    if (!stringlen) return 0;
#ifdef _WIN32
    return (int64_t)_strtoi64(ptr, NULL, 10);
#else
    return (int64_t)strtoll(ptr, NULL, 10);
#endif
}

uint64_t String::toUnsignedInt64() const
{
    if (!stringlen) return 0;
#ifdef _WIN32
    return (uint64_t)_strtoui64(ptr, NULL, 10);
#else
    return (uint64_t)strtoull(ptr, NULL, 10);
#endif
}

bool String::toBool() const
{
    if (isTrue()) return true;
    return false;
}

long String::toLong() const
{
    if (!stringlen) return 0;
    return strtol(ptr, NULL, 10);
}

unsigned long String::toUnsignedLong() const
{
    if (!stringlen) return 0;
    return strtoul(ptr, NULL, 10);
}

long long String::toLongLong() const
{
    if (!stringlen) return 0;
#ifdef _WIN32
    return (long long)_strtoi64(ptr, NULL, 10);
#else
    return (long long)strtoll(ptr, NULL, 10);
#endif
}

unsigned long long String::toUnsignedLongLong() const
{
    if (!stringlen) return 0;
#ifdef _WIN32
    return (unsigned long long)_strtoui64(ptr, NULL, 10);
#else
    return (unsigned long long)strtoull(ptr, NULL, 10);
#endif
}

float String::toFloat() const
{
    if (!stringlen) return 0.0f;
    return (float)atof(ptr);
}

double String::toDouble() const
{
    if (!stringlen) return 0.0;
    return atof(ptr);
}

bool String::startsWith(const String& prefix, size_t start, size_t end) const
{
    String part;
    if (start > 0 || end != (size_t)-1) {
        part = mid(start, end).left(prefix.size());
    } else {
        part = left(prefix.size());
    }
    return part == prefix;
}

bool String::endsWith(const String& suffix, size_t start, size_t end) const
{
    String part;
    if (start > 0 || end != (size_t)-1) {
        part = mid(start, end).right(suffix.size());
    } else {
        part = right(suffix.size());
    }
    return part == suffix;
}

String String::join(const pplib::Array& iterable) const
{
    return iterable.implode(*this);
}

String operator+(const String& str1, const String& str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

String operator+(const String& str1, const WideString& str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

String operator+(const WideString& str1, const String& str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

String operator+(const char* str1, const String& str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

String operator+(const String& str1, const char* str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

String operator+(const String& str1, const char c)
{
    String s = str1;
    s.append(c);
    return s;
}

String operator+(const char c, const String& str2)
{
    String s;
    s.set(c);
    s.append(str2);
    return s;
}

String operator+(const wchar_t* str1, const String& str2)
{
    String s;
    s.set(str1);
    s.append(str2);
    return s;
}

String operator+(const String& str1, const wchar_t* str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

String operator+(const std::string& str1, const String& str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

String operator+(const String& str1, const std::string& str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

String operator+(const std::wstring& str1, const String& str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

String operator+(const String& str1, const std::wstring& str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

std::ostream& operator<<(std::ostream& s, const String& str)
{
    return s.write((const char*)str, str.size());
}

String::iterator String::begin() noexcept
{
    return ptr;
}

String::const_iterator String::begin() const noexcept
{
    return ptr;
}

String::const_iterator String::cbegin() const noexcept
{
    return ptr;
}

String::iterator String::end() noexcept
{
    return ptr + stringlen;
}

String::const_iterator String::end() const noexcept
{
    return ptr + stringlen;
}

String::const_iterator String::cend() const noexcept
{
    return ptr + stringlen;
}

String::reverse_iterator String::rbegin() noexcept
{
    return reverse_iterator(end());
}

String::const_reverse_iterator String::rbegin() const noexcept
{
    return const_reverse_iterator(end());
}

String::const_reverse_iterator String::crbegin() const noexcept
{
    return const_reverse_iterator(end());
}

String::reverse_iterator String::rend() noexcept
{
    return reverse_iterator(begin());
}

String::const_reverse_iterator String::rend() const noexcept
{
    return const_reverse_iterator(begin());
}

String::const_reverse_iterator String::crend() const noexcept
{
    return const_reverse_iterator(begin());
}

} // namespace pplib
