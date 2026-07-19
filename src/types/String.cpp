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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <vector>

#include <ppl7/types/string.h>
#include <ppl7/types/widestring.h>
#include <ppl7/types/bytearrayptr.h>
#include <ppl7/types/bytearray.h>
#include <ppl7/types/array.h>
#include <ppl7/exceptions.h>
#include <ppl7/functions.h>
#include <ppl7/core/iconv.h>

#include <config_ppl7.h>

namespace ppl7
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
    size_t bytes = size + 1;
    if (s >= bytes) return; // Nothing to do
    char* p;
    p = (char*)realloc(ptr, bytes);
    if (!p) throw OutOfMemoryException();
    ptr = p;
    s = bytes;
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

    size_t outbytes = inbytes + 1;
    if (outbytes > s) {
        free(ptr);
        stringlen = 0;
        s = outbytes;
        ptr = (char*)malloc(s);
        if (!ptr) {
            s = 0;
            throw OutOfMemoryException();
        }
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
    reserve(size + 1);

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
    if (ptr == nullptr) {
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
    if (ptr == nullptr) {
        return set(str, size);
    }
    String a;
    a.set(str, size);
    return prepend(a.ptr, a.stringlen);
}

String& String::prepend(const std::string& str, size_t size)
{
    if (ptr == nullptr) {
        return set(str, size);
    }
    String a;
    a.set(str, size);
    return prepend(a.ptr, a.stringlen);
}

String& String::prepend(const std::wstring& str, size_t size)
{
    if (ptr == nullptr) {
        return set(str, size);
    }
    String a;
    a.set(str, size);
    return prepend(a.ptr, a.stringlen);
}

String& String::prepend(const char* str, size_t size)
{
    if (str == NULL || size == 0) return *this;
    if (ptr == nullptr) {
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

/*!\brief String in eine beliebige lokale Kodierung umwandeln
 *
 * \desc
 * Mit dieser Funktion wird der Inhalt des Strings in eine beliebige lokale
 * Kodierung umgewandelt und als ByteArray zurückgegeben.
 *
 * \param[in] encoding Das gewünschte Encoding
 *
 * @return ByteArray mit der UTF8-Repräsentation des Strings.
 *
 * \attention
 * Für diese Funktion wird "Iconv" benötigt. Ist keine Iconv-Bibliothek auf dem
 * System vorhanden, wird eine UnsupportedFeatureException geworfen.
 */
ByteArray String::toEncoding(const char* encoding) const
{
#ifndef HAVE_ICONV
    throw UnsupportedFeatureException();
#else
    Iconv iconv("UTF-8", encoding);
    ByteArray to;
    iconv.transcode(ByteArrayPtr(ptr, stringlen), to);
    return to;
#endif
}

/*!\brief Einzelnes Zeichen auslesen
 *
 * \desc
 * Mit dieser Funktion kann Bytewert eines einzelnen Zeichens an der Position
 * \p pos ausgelesen werden. Enthält \p pos einen positiven Wert, wird die Position des
 * Zeichens vom Anfang des Strings ermittelt, wobei 0 dem ersten Zeichen entspricht.
 * Ist der Wert negativ, wird das Zeichen vom Ende des Strings ermittelt, wobei -1
 * dem letzten Zeichen des Strings entspricht.
 *
 * @param pos Position des Zeichens innerhalb des Strings
 * @return Bytewert des Zeichens
 * \exception OutOfBoundsException Wird geworfen, wenn die angegebene Position \p pos
 * ausserhalb des Strings liegt oder der String leer ist.
 */
char String::get(ssize_t pos) const
{
    if (pos >= 0 && stringlen > (size_t)pos) return ptr[pos];
    if (pos < 0 && (size_t)(0 - pos) <= stringlen) return ptr[stringlen + pos];
    throw OutOfBoundsException();
}

/*!\brief Einzelnes Zeichen auslesen
 *
 * \desc
 * Mit diesem Operator kann der Bytewert eines einzelnen Zeichens an der Position
 * \p pos ausgelesen werden. Enthält \p pos einen positiven Wert, wird die Position des
 * Zeichens vom Anfang des Strings ermittelt, wobei 0 dem ersten Zeichen entspricht.
 * Ist der Wert negativ, wird das Zeichen vom Ende des Strings ermittelt, wobei -1
 * dem letzten Zeichen des Strings entspricht.
 *
 * @param pos Position des Zeichens innerhalb des Strings
 * @return Bytewert des Zeichens
 * \exception OutOfBoundsException Wird geworfen, wenn die angegebene Position \p pos
 * ausserhalb des Strings liegt oder der String leer ist.
 */
char String::operator[](ssize_t pos) const
{
    if (pos >= 0 && stringlen > (size_t)pos) return ((char*)ptr)[pos];
    if (pos < 0 && (size_t)(0 - pos) <= stringlen) return ((char*)ptr)[stringlen + pos];
    throw OutOfBoundsException();
}

/*!\brief Einzelnes Zeichen referenzieren
 *
 * \desc
 * Mit diesem Operator kann eine Referenz auf ein einzelnes Zeichen an der Position
 * \p pos erhalten werden. Dies ermöglicht Lese- und Schreibzugriff.
 *
 * @param pos Position des Zeichens innerhalb des Strings
 * @return Referenz auf das Zeichen
 * \exception OutOfBoundsException Wird geworfen, wenn die angegebene Position \p pos
 * ausserhalb des Strings liegt oder der String leer ist.
 */
char& String::operator[](ssize_t pos)
{
    if (pos >= 0 && stringlen > (size_t)pos) return ((char*)ptr)[pos];
    if (pos < 0 && (size_t)(0 - pos) <= stringlen) return ((char*)ptr)[stringlen + pos];
    throw OutOfBoundsException();
}

/*!\brief String auf STDOUT ausgeben
 *
 * Diese Funktion gibt den aktuellen String auf STDOUT aus. Dazu ist es notwendig den String vom internen
 * Unicode-Format in das Encoding des Betriebssystems umzurechnen. Da dieses von den lokalen Einstellungen
 * des Betriebssystems und des Users abhängig ist, wird die Environment-Variable "LANG" ausgewertet.
 * Ist diese nicht gesetzt oder enthält ein unbekanntes Encoding, wird der String immer in UTF-8 ausgegeben.
 *
 * \param withNewline Ein optionaler Parameter, der angibt, ob nach der Ausgabe ein Zeilenumbruch
 * angehangen werden soll (default=false)
 * \par Exceptions:
 * Keine
 *
 */
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

/*!\brief String auf STDOUT mit abschließendem Zeilenumbruch ausgeben
 *
 * Diese Funktion gibt den aktuellen String mit abschließendem Zeilenumbruch auf STDOUT aus.
 * Dazu ist es notwendig den String vom internen
 * Unicode-Format in das Encoding des Betriebssystems umzurechnen. Da dieses von den lokalen Einstellungen
 * des Betriebssystems und des Users abhängig ist, wird die Environment-Variable "LANG" ausgewertet.
 * Ist diese nicht gesetzt oder enthält ein unbekanntes Encoding, wird der String immer in UTF-8 ausgegeben.
 *
 * \par Exceptions:
 * Keine
 *
 */
void String::printnl() const noexcept
{
    print(true);
}

/*!\brief Hexdump des Strings ausgeben
 *
 * \desc
 * Mit dieser zu Debug-Zwecken gedachten Funktion wird der Inhalt des
 * Strings als HexDump auf der Konsole ausgegeben.
 */
void String::hexDump() const
{
    PrintDebug("HEXDUMP of String %p: %zi Bytes starting at Address %p:\n", this, stringlen * sizeof(char), ptr);
    if (stringlen) HexDump(ptr, stringlen * sizeof(char), true);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion String::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator=(const char* str)
{
    return set(str);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion String::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator=(const wchar_t* str)
{
    return set(str);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion String::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator=(const String& str)
{
    return set(str);
}

/*!\brief String übernehmen (Move-Operator)
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str übernommen. Der Operator
 * verschiebt die internen Daten des Strings in das aktuelle Objekt und setzt
 * den Quellstring in einen leeren Zustand zurück.
 *
 * @param[in] str Zu übernehmender String
 * @return Referenz auf diese Instanz der Klasse
 */
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

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion String::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator=(const std::string& str)
{
    return set(str);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion String::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator=(const std::wstring& str)
{
    return set(str);
}

/*!\brief Zeichen übernehmen
 *
 * \desc
 * Mit diesem Operator wird ein einzelnes Zeichen in den String kopiert.
 *
 * @param[in] c Unicode Wert des zu übernehmenden Zeichens
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator=(char c)
{
    return set(c);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion String::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator+=(const char* str)
{
    return append(str);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion String::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator+=(const wchar_t* str)
{
    return append(str);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion String::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator+=(const String& str)
{
    return append(str);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion String::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator+=(const std::string& str)
{
    return append(str);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion String::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator+=(const std::wstring& str)
{
    return append(str);
}

/*!\brief Zeichen anhängen
 *
 * \desc
 * Mit diesem Operator wird das angegebene Zeichen \p c an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion String::append.
 *
 * @param[in] c Unicode-Wert des anzuhängenden Zeichens
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator+=(char c)
{
    return append(c);
}

/*!\brief Führt einen Vergleich mit einem anderen String durch
 *
 * \desc
 * Führt einen Vergleich mit einem anderen String durch.
 *
 * \param str String, mit dem verglichen werden soll
 * \param size Optionaler Parameter, der die Anzahl zu berücksichtigender Zeichen innerhalb des
 * Strings \p str angibt. Wird er nicht angegeben, wird ein vergleich mit dem kompletten String
 * \p str durchgeführt.
 *
 * \return Ist der String innerhalb dieses Objekts kleiner als der mit \a str angegebene, wird ein
 * negativer Wert zurückgegeben, ist er größer, erfolgt ein positiver Return-Wert,
 * sind beide identisch, wird 0 zurückgegeben.
 *
 * \see strCaseCmp Vergleich zweier Strings unter Ignorierung der Gross-/Kleinschreibung
 */
int String::strcmp(const String& str, size_t size) const
{
    if (size != (size_t)-1) return ::strncmp(ptr, str.ptr, size);
    return ::strcmp(ptr, str.ptr);
}

int String::strcmp(const char* str, size_t size) const
{
    if (size != (size_t)-1) return ::strncmp(ptr, str, size);
    return ::strcmp(ptr, str);
}

/*!\brief Stringvergleich mit Ignorierung von Gross-/Kleinschreibung
 *
 * \desc
 * Führt einen Vergleich mit einem anderen String durch, unter Ignorierung der
 * Gross-/Kleinschreibung.
 *
 * \param str String, mit dem verglichen werden soll
 * \param size Optionaler Parameter, der die Anzahl zu berücksichtigender Zeichen innerhalb des
 * Strings \p str angibt. Wird er nicht angegeben, wird ein vergleich mit dem kompletten String
 * \p str durchgeführt.
 *
 *
 * \return Ist der String innerhalb dieses Objekts kleiner als der mit \a str angegebene, wird ein
 * negativer Wert zurückgegeben, ist er größer, erfolgt ein positiver Return-Wert,
 * sind beide identisch, wird 0 zurückgegeben.
 *
 * \see strcmp Vergleich zweier Strings unter Berücksichtigung der Gross-/Kleinschreibung
 */
int String::strCaseCmp(const String& str, size_t size) const
{
    if (size != (size_t)-1) return strncasecmp(ptr, str.ptr, size);
    return strcasecmp(ptr, str.ptr);
}

int String::strCaseCmp(const char* str, size_t size) const
{
    if (size != (size_t)-1) return strncasecmp(ptr, str, size);
    return strcasecmp(ptr, str);
}

String String::left(size_t len) const
{
    if (stringlen > 0) {
        if (len > stringlen) len = stringlen;
        return String(ptr, len);
    }
    return String();
}

String String::right(size_t len) const
{
    if (stringlen > 0) {
        if (len > stringlen) len = stringlen;
        return String(ptr + stringlen - len, len);
    }
    return String();
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
    if (len == (size_t)-1) len = stringlen;
    if (start < stringlen && stringlen > 0 && len > 0) {
        if (start + len > stringlen) len = stringlen - start;
        return String(ptr + start, len);
    }
    return String();
}

void String::lowerCase()
{
    if (stringlen == 0) return;
    // Wir wandeln den String zunächst nach Unicode um
    std::vector<wchar_t> buffer(stringlen + 1);
    size_t l;
#ifdef HAVE_MBSTOWCS_S
    if (::mbstowcs_s(&l, buffer.data(), buffer.size(), ptr, stringlen) != 0) {
        throw CharacterEncodingException();
    }
    if (l > 0) l--; // Nullbyte abziehen, da mbstowcs_s dieses mitzählt
#else
    l = ::mbstowcs(buffer.data(), ptr, stringlen);
    if (l == (size_t)-1) {
        throw CharacterEncodingException();
    }
#endif
    // Umwandeln mittels towlower, das die aktuelle Locale berücksichtigt
    for (size_t i = 0; i < l; i++) {
        wchar_t wc = buffer[i];
        wchar_t c = towlower(wc);
        if (c != (wchar_t)WEOF) {
            buffer[i] = c;
        }
    }
    // Zurück im String speichern
    set(buffer.data(), l);
}

void String::upperCase()
{
    if (stringlen == 0) return;
    // Wir wandeln den String zunächst nach Unicode um
    std::vector<wchar_t> buffer(stringlen + 1);
    size_t l;
#ifdef HAVE_MBSTOWCS_S
    if (::mbstowcs_s(&l, buffer.data(), buffer.size(), ptr, stringlen) != 0) {
        throw CharacterEncodingException();
    }
    if (l > 0) l--; // Nullbyte abziehen, da mbstowcs_s dieses mitzählt
#else
    l = ::mbstowcs(buffer.data(), ptr, stringlen);
    if (l == (size_t)-1) {
        throw CharacterEncodingException();
    }
#endif
    // Umwandeln mittels towlower, das die aktuelle Locale berücksichtigt
    for (size_t i = 0; i < l; i++) {
        wchar_t wc = buffer[i];
        wchar_t c = towupper(wc);
        if (c != (wchar_t)WEOF) {
            buffer[i] = c;
        }
    }
    // Zurück im String speichern
    set(buffer.data(), l);
}

void String::trim()
{
    if (stringlen == 0) return;

    size_t start = 0;
    while (start < stringlen && (ptr[start] == ' ' || ptr[start] == '\t' || ptr[start] == '\r' || ptr[start] == '\n')) {
        start++;
    }

    if (start == stringlen) {
        clear();
        return;
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
void String::trimLeft()
{
    if (stringlen == 0) return;
    size_t start = 0;
    while (start < stringlen && (ptr[start] == ' ' || ptr[start] == '\t' || ptr[start] == '\r' || ptr[start] == '\n')) {
        start++;
    }

    if (start == stringlen) {
        clear();
        return;
    }
    if (start > 0) {
        size_t new_len = stringlen - start;
        memmove(ptr, ptr + start, new_len + 1); // +1 kopiert das Nullbyte direkt mit
        stringlen = new_len;
    }
}

//! \brief Schneidet Leerzeichen, Tabs Returns und Linefeeds am Ende des Strings ab
void String::trimRight()
{
    if (stringlen == 0) return;
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
}

//! \brief Schneidet die definierten Zeichen am Anfang des Strings ab
void String::trimLeft(const String& chars)
{
    if (stringlen == 0 || chars.isEmpty()) return;

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
        return;
    }
    if (start > 0) {
        size_t new_len = stringlen - start;
        memmove(ptr, ptr + start, new_len + 1); // Kopiert das Nullbyte direkt mit
        stringlen = new_len;
    }
}

//! \brief Schneidet die definierten Zeichen am Ende des Strings ab
void String::trimRight(const String& chars)
{
    if (stringlen == 0 || chars.isEmpty()) return;

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
}

//! \brief Schneidet die definierten Zeichen am Anfang und Ende des Strings ab
void String::trim(const String& chars)
{
    trimLeft(chars);
    trimRight(chars);
}

/*!\brief Schneidet Zeichen am Ende des Strings ab
 *
 * \desc
 * Diese Funktion schneidet \p num Zeichen vom Ende des Strings ab. Falls \p num
 * größer als der String ist, bleibt ein leerer String zurück.
 *
 * @param num Anzahl Zeichen, die abgeschnitten werden sollen
 */
void String::chopRight(size_t num)
{
    if (stringlen > 0) {
        if (stringlen < num) num = stringlen;
        stringlen -= num;
        ptr[stringlen] = 0;
    }
}

/*!\brief Schneidet Zeichen am Ende des Strings ab
 *
 * \desc
 * Diese Funktion schneidet \p num Zeichen vom Ende des Strings ab. Falls \p num
 * größer als der String ist, bleibt ein leerer String zurück.
 *
 * @param num Anzahl Zeichen, die abgeschnitten werden sollen
 *
 * \see
 * Die Funktion ist identisch zu String::chopRight
 */
void String::chop(size_t num)
{
    if (stringlen > 0) {
        if (stringlen < num) num = stringlen;
        stringlen -= num;
        ptr[stringlen] = 0;
    }
}

/*!\brief Schneidet Zeichen am Anfang des Strings ab
 *
 * \desc
 * Diese Funktion schneidet \p num Zeichen vom Anfang des Strings ab. Falls \p num
 * größer als der String ist, bleibt ein leerer String zurück.
 *
 * @param num Anzahl Zeichen, die abgeschnitten werden sollen
 */
void String::chopLeft(size_t num)
{
    if (stringlen > 0) {
        if (stringlen < num) num = stringlen;
        memmove(ptr, ptr + num, (stringlen - num));
        stringlen -= num;
        ptr[stringlen] = 0;
    }
}

/*!\brief Schneidet Returns und Linefeeds am Anfanng und Ende des Strings ab
 *
 * \desc
 * Schneidet Returns und Linefeeds am Anfanng und Ende des Strings ab
 */
void String::chomp()
{
    trim("\n\r");
}

/*!\brief Schneidet den String an einer bestimmten Stelle ab
 *
 * \desc
 * Der String wird an einer bestimmten Stelle einfach abgeschnitten
 * \param pos Die Position, an der der String abgeschnitten wird. Bei Angabe von 0 ist der String anschließend
 * komplett leer. Ist \c pos größer als die Länge des Strings, passiert nichts.
 */
void String::cut(size_t pos)
{
    if (stringlen == 0) return;
    if (pos > stringlen) return;
    ptr[pos] = 0;
    stringlen = pos;
}

/*! \brief Schneidet den String beim ersten Auftauchen eines Zeichens/Strings ab
 *
 * Der String wird beim ersten Auftauchen eines Zeichens oder eines Strings abgeschnitten.
 * \param[in] letter Buchstabe oder Buchstabenkombination, an der der String abgeschnitten werden
 * soll. Zeigt der Pointer auf NULL oder ist der String leer, passiert nichts.
 */
void String::cut(const String& letter)
{
    if (stringlen == 0) return;
    if (letter.isEmpty()) return;
    ssize_t p = instr(letter, 0);
    if (p >= 0) {
        ptr[p] = 0;
        stringlen = p;
    }
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

/*!\brief Teilstring finden
 *
 * \desc
 * Diese Funktion findet die erste Position der Zeichenfolge \p needle
 * innerhalb des Strings. Abschließende `\0'-Zeichen werden nicht
 * miteinander verglichen.
 *
 * @param needle
 * @return
 * Die Funktion gibt einen String zurück, der mit der gefundenen
 * Zeichenkette beginnt und den Rest des Strings bis zum Ende enthält.
 * Wurde die Zeichenkette nicht gefunden, wird ein leerer String
 * zurückgegeben.
 * \note
 * Ein Sonderfall besteht, wenn \p needle leer ist. In diesem Fall wird
 * der komplette String zurückgegeben.
 */
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

/*! \brief Sucht nach einem String
 *
 * Find sucht nach dem Suchstring \a needle ab der gewünschten Position \a start.
 *
 * \param[in] needle Gesuchter Teilstring
 * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0 oder wird er weggelassen,
 * wird der String vom Anfang an durchsucht. Ist der Wert jedoch negativ, wird rückwärts vom
 * Ende des Strings gesucht.
 *
 * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
 * oder -1 wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
 * Funktion immer 0 zurück.
 */
ssize_t String::find(const String& needle, ssize_t start) const
{
    if (stringlen == 0) return -1;
    if (needle.stringlen == 0) return 0;
    if (start > 0 && (size_t)start >= stringlen) return -1;
    if (start < 0 && ((size_t)((ssize_t)stringlen + start)) >= stringlen) return -1;

    // Position to return
    size_t p = -1;
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

/*! \brief Sucht nach einem String, Gross-/Kleinschreibung wird ignoriert
 *
 * \desc
 * Diese Funktion sucht innerhalb des aktuellen String nach \p needle ab der
 * gewünschten Position \a start. Gross-/Kleinschreibung wird dabei ignoriert.
 *
 * \param[in] needle Gesuchter Teilstring
 * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0 oder wird er weggelassen,
 * wird der String vom Anfang an durchsucht. Ist der Wert jedoch negativ, wird rückwärts vom
 * Ende des Strings gesucht.
 *
 * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
 * oder -1 wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
 * Funktion immer 0 zurück.
 */
ssize_t String::findCase(const String& needle, ssize_t start) const
{
    String CaseNeedle(needle);
    String CaseSearch(ptr, stringlen);
    CaseNeedle.lowerCase();
    CaseSearch.lowerCase();
    return CaseSearch.find(CaseNeedle, start);
}

/*! \brief Sucht nach einem String
 *
 * \desc
 * Diese Funktion sucht nach dem Suchstring \a needle ab der gewünschten Position \a start.
 *
 * \param[in] needle Gesuchter Teilstring
 * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0
 * oder nicht angegeben, wird der String vom Anfang an gesucht.
 *
 * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
 * oder -1 wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
 * Funktion immer 0 zurück.
 */
ssize_t String::instr(const String& needle, size_t start) const
{
    if (stringlen == 0) return -1;
    if (needle.stringlen == 0) return 0;
    if (start >= stringlen) return -1;
    const char* p;
    p = ::strstr((ptr + start), needle.ptr);
    if (p != NULL) {
        return ((ssize_t)(p - ptr));
    }
    return -1;
}

/*! \brief Sucht nach einem String, Gross-/Kleinschreibung wird ignoriert
 *
 * \desc
 * Diese Funktion sucht nach dem Suchstring \a needle ab der gewünschten Position \a start.
 * Gross-/Kleinschreibung wird dabei ignoriert.
 *
 * \param[in] needle Gesuchter Teilstring
 * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0
 * oder nicht angegeben, wird der String vom Anfang an gesucht.
 *
 * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
 * oder -1 wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
 * Funktion immer 0 zurück.
 */
ssize_t String::instrCase(const String& needle, size_t start) const
{
    String CaseNeedle(needle);
    String CaseSearch(ptr, stringlen);
    CaseNeedle.lowerCase();
    CaseSearch.lowerCase();
    return CaseSearch.instr(CaseNeedle, start);
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
    const char* p;
    p = ::strstr(ptr, needle.ptr);
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
    size_t new_len = stringlen * num;
    size_t newsize = new_len + 1;
    char* buf = (char*)malloc(newsize);
    if (!buf) throw OutOfMemoryException();

    char* tmp = buf;
    for (size_t i = 0; i < num; i++) {
        memcpy(tmp, ptr, stringlen);
        tmp += stringlen;
    }
    buf[new_len] = 0;

    free(ptr);
    ptr = buf;
    stringlen = new_len;
    s = newsize;
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

    size_t new_len = str.stringlen * num;
    size_t newsize = new_len + 1;
    char* buf = (char*)malloc(newsize);
    if (!buf) throw OutOfMemoryException();

    char* tmp = buf;
    for (size_t i = 0; i < num; i++) {
        memcpy(tmp, src_ptr, str.stringlen);
        tmp += str.stringlen;
    }
    buf[new_len] = 0;

    free(ptr);
    ptr = buf;
    stringlen = new_len;
    s = newsize;
    return *this;
}

String& String::repeat(char code, size_t num)
{
    if (!code) {
        throw IllegalArgumentException();
    }
    if (!num) {
        clear();
        return *this;
    }
    size_t newsize = (num + 1);
    char* buf = (char*)malloc(newsize);
    if (!buf) throw OutOfMemoryException();
    for (size_t i = 0; i < num; i++)
        buf[i] = code;
    free(ptr);
    ptr = buf;
    stringlen = num;
    ptr[stringlen] = 0;
    s = newsize;
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

/*! \brief Schiebt den String nach links
 *
 * Der String wird um die mit \c size angegebenen Zeichen nach links verschoben und rechts mit dem durch \c c angegebenen
 * Zeichen aufgefüllt.
 * \param c Das Zeichen, mit dem der String auf der rechten Seite aufgefüllt werden soll. Wird der Wert 0 verwendet, findet keine
 * Auffüllung statt, d.h. der String verkürzt sich einfach.
 * \param size Die Anzahl Zeichen, um die der String nach links verschoben werden soll. Ist \c size größer als die Länge
 * des Strings, wird der String komplett geleert und ist anschließend so groß wie size, sofern c>0 war.
 */
void String::shl(char c, size_t size)
{
    if (!stringlen) return;
    if (!size) return;
    if (size > stringlen) size = stringlen;
    String t = mid(size);
    if (c) {
        String a;
        a.repeat(c, size);
        t += a;
    }
    set(t);
}

/*! \brief Schiebt den String nach rechts
 *
 * Der String wird um die mit \c size angegebenen Zeichen nach rechts verschoben und links mit dem durch \c c angegebenen
 * Zeichen aufgefüllt.
 * \param c Das Zeichen, mit dem der String auf der linken Seite aufgefüllt werden soll. \c c muß größer 0 sein.
 * \param size Die Anzahl Zeichen, um die der String nach rechts verschoben werden soll. Ist \c size größer als die Länge
 * des Strings, wird der String komplett geleert und ist anschließend so groß wie size.
 */
void String::shr(char c, size_t size)
{
    if (!stringlen) return;
    if (!size) return;
    if (!c) return;
    if (size > stringlen) size = stringlen;
    String t;
    t.repeat(c, size);
    t += left(stringlen - size);
    t.cut(size);
    set(t);
}

/*!\brief Kleiner als
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters kleiner dem des
 * rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator<(const String& str) const
{
    if (::strcmp(ptr, str.ptr) < 0) return true;
    return false;
}

/*!\brief Kleiner oder gleich
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters kleiner oder gleich
 * dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator<=(const String& str) const
{
    if (strcmp(str) <= 0) return true;
    return false;
}

/*!\brief Gleich
 *
 * \desc
 * Dieser Operator liefert \c true zurück, wenn der Wert des linken Parameters mit dem des
 * rechten identisch ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator==(const String& str) const
{
    if (strcmp(str) == 0) return true;
    return false;
}

/*!\brief Ungleich
 *
 * \desc
 * Dieser Operator liefert \c true zurück, wenn der Wert des linken Parameters nicht dem des
 * rechten entspricht.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator!=(const String& str) const
{
    if (strcmp(str) == 0) return false;
    return true;
}

/*!\brief Größer oder gleich
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters größer oder
 * gleich dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator>=(const String& str) const
{
    if (strcmp(str) >= 0) return true;
    return false;
}

/*!\brief Größer als
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters größer
 * dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator>(const String& str) const
{
    if (strcmp(str) > 0) return true;
    return false;
}

/*!\brief Kleiner als
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters kleiner dem des
 * rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator<(const char* str) const
{
    if (strcmp(str) < 0) return true;
    return false;
}

/*!\brief Kleiner oder gleich
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters kleiner oder gleich
 * dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator<=(const char* str) const
{
    if (strcmp(str) <= 0) return true;
    return false;
}

/*!\brief Gleich
 *
 * \desc
 * Dieser Operator liefert \c true zurück, wenn der Wert des linken Parameters mit dem des
 * rechten identisch ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator==(const char* str) const
{
    if (strcmp(str) == 0) return true;
    return false;
}

/*!\brief Ungleich
 *
 * \desc
 * Dieser Operator liefert \c true zurück, wenn der Wert des linken Parameters nicht dem des
 * rechten entspricht.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator!=(const char* str) const
{
    if (strcmp(str) == 0) return false;
    return true;
}

/*!\brief Größer oder gleich
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters größer oder
 * gleich dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator>=(const char* str) const
{
    if (strcmp(str) >= 0) return true;
    return false;
}

/*!\brief Größer als
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters größer
 * dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator>(const char* str) const
{
    if (strcmp(str) > 0) return true;
    return false;
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

String::operator long() const
{
    return toLong();
}

String::operator unsigned long() const
{
    return toUnsignedLong();
}

String::operator long long() const
{
    return toLongLong();
}

String::operator unsigned long long() const
{
    return toUnsignedLongLong();
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

    // std::vector verwaltet Speicher automatisch (RAII)
    std::vector<wchar_t> w(stringlen + 1);

    // Korrekte Übergabe der Element-Anzahl (w.size()) statt Byte-Größe!
    size_t wlen = ::mbstowcs(w.data(), ptr, w.size());
    if (wlen == (size_t)-1) {
        throw CharacterEncodingException();
    }

    return std::wstring(w.data(), wlen);
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
#ifdef HAVE_STRTOLL
    return (int64_t)strtoll(ptr, NULL, 10);
#elif defined WIN32
    return (int64_t)_strtoi64(ptr, NULL, 10);
#else
    throw TypeConversionException();
#endif
}

uint64_t String::toUnsignedInt64() const
{
    if (!stringlen) return 0;
#ifdef HAVE_STRTOULL
    return (uint64_t)strtoull(ptr, NULL, 10);
#elif defined HAVE_STRTOLL
    return (uint64_t)strtoll(ptr, NULL, 10);
#elif defined WIN32
    return (uint64_t)_strtoi64(ptr, NULL, 10);
#else
    throw TypeConversionException();
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
#ifdef HAVE_STRTOLL
    return (long long)strtoll(ptr, NULL, 10);
#elif defined WIN32
    return (long long)_strtoi64(ptr, NULL, 10);
#else
    throw TypeConversionException();
#endif
}

unsigned long long String::toUnsignedLongLong() const
{
    if (!stringlen) return 0;
#ifdef HAVE_STRTOULL
    return (unsigned long long)strtoull(ptr, NULL, 10);
#elif defined HAVE_STRTOLL
    return (unsigned long long)strtoll(ptr, NULL, 10);
#elif defined WIN32
    return (unsigned long long)_strtoi64(ptr, NULL, 10);
#else
    throw TypeConversionException();
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

String String::join(const ppl7::Array& iterable) const
{
    return iterable.implode(*this);
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const String& str1, const String& str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const char* str1, const String& str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const String& str1, const char* str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const wchar_t* str1, const String& str2)
{
    String s;
    s.set(str1);
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const String& str1, const wchar_t* str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const std::string& str1, const String& str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const String& str1, const std::string& str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const std::wstring& str1, const String& str2)
{
    String s = str1;
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
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

} // namespace ppl7
