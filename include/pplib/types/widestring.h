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
#ifndef PPLIB_TYPES_WIDESTRING_H_
#define PPLIB_TYPES_WIDESTRING_H_

#include <string>
#include <stdint.h>

#ifdef PPL_WITH_QT6
#include <QAnyStringView>
#ifndef WITH_QT
#define WITH_QT
#endif
#endif

#ifdef WITH_QT
#include <QString>
#include <QVariant>
#include <QByteArray>
#endif

namespace pplib
{

class String;
class ByteArrayPtr;
class ByteArray;
class Array;

class WideString
{
private:
    wchar_t* ptr;     // Pointer auf den Speicherbereich, in dem der String gespeichert ist
    size_t s;         // Anzahl Zeichen, für die Speicher reserviert wurde, exkl. 0-Byte am Ende
    size_t stringlen; // Länge des Strings, exkl. 0-Byte am Ende

public:
    static const size_t npos = (size_t)-1; // Ergebnis von find, wenn nichts gefunden wurde

    //! @name Konstruktoren und Destruktor
    //@{

    WideString() noexcept;
    WideString(const wchar_t* str);
    WideString(const wchar_t* str, size_t size);
    explicit WideString(const char* str, size_t size = (size_t)-1);
    WideString(const WideString& str);
    explicit WideString(const String& str);
    explicit WideString(const std::string& str);
    WideString(const std::wstring& str);

    /** @brief Konstruktor mit ByteArrayPtr
     *
     * Ein String wird aus einem ByteArrayPtr erstellt.
     *
     * @param str Referenz auf einen ByteArrayPtr
     */
    explicit WideString(const ByteArrayPtr& str);

    WideString(WideString&& other) noexcept;
    ~WideString() noexcept;
#ifdef WITH_QT
    WideString(const QString& q)
    {
        ptr = NULL;
        stringlen = 0;
        s = 0;
#ifdef PPL_QT_STRING_UTF8
        QByteArray a = q.toUtf8();
#else
        QByteArray a = q.toLocal8Bit();
#endif
        set((const char*)a);
    }
    WideString(QString* q)
    {
        ptr = NULL;
        stringlen = 0;
        s = 0;
#ifdef PPL_QT_STRING_UTF8
        QByteArray a = q->toUtf8();
#else
        QByteArray a = q->toLocal8Bit();
#endif
        set((const char*)a);
    }
#endif
    //@}

    //! @name Statische Funktionen
    //@{
    static WideString format(const char* fmt, ...);
    //@}

    void clear() noexcept;
    size_t capacity() const;
    void reserve(size_t size);
    size_t len() const;
    size_t length() const;
    size_t size() const;
    size_t byteLength() const;
    bool isEmpty() const;
    bool notEmpty() const;
    bool isNumeric() const;
    bool isInteger() const;
    bool isTrue() const;
    bool isFalse() const;

    int strcmp(const WideString& str, size_t size = (size_t)-1) const;
    int strCaseCmp(const WideString& str, size_t size = (size_t)-1) const;
    WideString left(size_t len) const;
    WideString right(size_t len) const;
    WideString mid(size_t start, size_t len = (size_t)-1) const;
    WideString substr(size_t start, size_t len = (size_t)-1) const;

    //! @name String setzen und verändern
    //@{

    WideString& set(const char* str, size_t size = (size_t)-1);
    WideString& set(const wchar_t* str, size_t size = (size_t)-1);
    WideString& set(const WideString& str, size_t size = (size_t)-1);
    WideString& set(const String& str, size_t size = (size_t)-1);
    WideString& set(const std::string& str, size_t size = (size_t)-1);
    WideString& set(const std::wstring& str, size_t size = (size_t)-1);
    WideString& set(wchar_t c);
    WideString& set(size_t position, wchar_t c);
    WideString& setf(const char* fmt, ...);

    WideString& append(const char* str, size_t size = (size_t)-1);
    WideString& append(const wchar_t* str, size_t size = (size_t)-1);
    WideString& append(const WideString& str, size_t size = (size_t)-1);
    WideString& append(const String& str, size_t size = (size_t)-1);
    WideString& append(const std::string& str, size_t size = (size_t)-1);
    WideString& append(const std::wstring& str, size_t size = (size_t)-1);
    WideString& appendf(const char* fmt, ...);
    WideString& append(wchar_t c);

    WideString& prepend(const char* str, size_t size = (size_t)-1);
    WideString& prepend(const wchar_t* str, size_t size = (size_t)-1);
    WideString& prepend(const WideString& str, size_t size = (size_t)-1);
    WideString& prepend(const String& str, size_t size = (size_t)-1);
    WideString& prepend(const std::string& str, size_t size = (size_t)-1);
    WideString& prepend(const std::wstring& str, size_t size = (size_t)-1);
    WideString& prependf(const char* fmt, ...);
    WideString& prepend(wchar_t c);

    WideString& vasprintf(const char* fmt, va_list args);
    WideString& repeat(size_t num);
    WideString& repeat(wchar_t unicode, size_t num);
    WideString& repeat(const WideString& str, size_t num);
    WideString repeated(size_t num) const;

    WideString& lowerCase();
    WideString& upperCase();
    WideString& upperCaseWords();
    WideString& trim();
    WideString& trimLeft();
    WideString& trimRight();
    WideString& trim(const WideString& chars);
    WideString& trimLeft(const WideString& chars);
    WideString& trimRight(const WideString& chars);
    WideString& chopRight(size_t num = 1);
    WideString& chop(size_t num = 1);
    WideString& chopLeft(size_t num = 1);
    WideString& chomp();
    WideString& cut(size_t pos);
    WideString& cut(const WideString& letter);

    WideString trimmed() const;

    WideString toLowerCase() const;
    WideString toUpperCase() const;

    WideString strchr(wchar_t c) const;
    WideString strrchr(wchar_t c) const;
    WideString strstr(const WideString& needle) const;
    size_t find(const WideString& needle, ssize_t start = 0) const;
    size_t findCase(const WideString& needle, ssize_t start = 0) const;
    size_t instr(const WideString& needle, size_t start = 0) const;
    size_t instrCase(const WideString& needle, size_t start = 0) const;

    bool has(const WideString& needle, bool ignoreCase = false) const;
    inline bool contains(const WideString& needle, bool ignoreCase = false) const
    {
        return has(needle, ignoreCase);
    }

    bool startsWith(const WideString& prefix, size_t start = 0, size_t end = (size_t)-1) const;
    bool endsWith(const WideString& suffix, size_t start = 0, size_t end = (size_t)-1) const;
    WideString join(const pplib::Array& iterable) const;

    WideString& replace(const WideString& search, const WideString& replacement);

    //@}

    //! @name String ausgeben und auslesen
    //@{
    void print(bool withNewline = false) const noexcept;
    void printnl() const noexcept;
    void hexDump() const;
    wchar_t get(ssize_t pos) const;
    const wchar_t* getPtr() const;
    const wchar_t* c_str() const
    {
        return ptr ? ptr : L"";
    }

    /**@brief String in UTF-8 umwandeln
     *
     * Mit dieser Funktion wird der Inhalt des Strings in UTF-8 umgewandelt und als
     * ByteArray zurückgegeben. Dabei ist garantiert, dass unabhängig von den eingestellten
     * locales immer UTF-8 zurückgegeben wird.
     *
     * @return ByteArray mit der UTF8-Repräsentation des Strings.
     */
    ByteArray toUtf8() const;

    /**@brief String in die lokale Kodierung umwandeln
     *
     * Mit dieser Funktion wird der Inhalt des Strings in die lokale Kodierung umgewandelt und als
     * String zurückgegeben. Die lokale Kodierung wird mit der Funktion setlocale() aus der
     * C-Standardbibliothek festgelegt.
     *
     * @return String mit der lokalen Repräsentation des Strings.
     */
    String toString() const;

    ByteArray toUCS4() const;
    WideString& fromUCS4(const uint32_t* str, size_t size = (size_t)-1);
    WideString& fromUCS4(const ByteArrayPtr& bin);

    int toInt() const;
    unsigned int toUnsignedInt() const;
    int64_t toInt64() const;
    uint64_t toUnsignedInt64() const;
    bool toBool() const;
    long toLong() const;
    unsigned long toUnsignedLong() const;
    long long toLongLong() const;
    unsigned long long toUnsignedLongLong() const;
    float toFloat() const;
    double toDouble() const;
    const wchar_t* toWchart() const;

    //@}

    //! @name Operatoren
    //@{
    operator const wchar_t*() const;
    operator int() const;
    operator unsigned int() const;
    operator bool() const;
    operator long() const;
    operator unsigned long() const;
    operator long long() const;
    operator unsigned long long() const;
    operator float() const;
    operator double() const;
    operator std::string() const;
    operator std::wstring() const;

    wchar_t operator[](ssize_t pos) const;
    wchar_t& operator[](ssize_t pos);

    WideString& operator=(const char* str);
    WideString& operator=(const wchar_t* str);
    WideString& operator=(const WideString& str);
    WideString& operator=(const String& str);
    WideString& operator=(const std::string& str);
    WideString& operator=(const std::wstring& str);
    WideString& operator=(wchar_t c);
    WideString& operator=(WideString&& other) noexcept;
    WideString& operator+=(const char* str);
    WideString& operator+=(const wchar_t* str);
    WideString& operator+=(const WideString& str);
    WideString& operator+=(const std::string& str);
    WideString& operator+=(const std::wstring& str);
    WideString& operator+=(wchar_t c);

    bool operator<(const WideString& str) const;
    bool operator<=(const WideString& str) const;
    bool operator==(const WideString& str) const;
    bool operator!=(const WideString& str) const;
    bool operator>=(const WideString& str) const;
    bool operator>(const WideString& str) const;

    bool operator<(const wchar_t* str) const;
    bool operator<=(const wchar_t* str) const;
    bool operator==(const wchar_t* str) const;
    bool operator!=(const wchar_t* str) const;
    bool operator>=(const wchar_t* str) const;
    bool operator>(const wchar_t* str) const;

    //@}

#ifdef WITH_QT
    //! @name Operatoren zur Verwendung der Klasse mit Qt
    //@{
    operator const QString() const
    {
        return QString::fromWCharArray(ptr, stringlen);
    }

    operator const QVariant() const
    {
        QVariant v = QString::fromWCharArray(ptr, stringlen);
        return v;
    }

    WideString& operator=(const QString& q)
    {
#ifdef PPL_QT_STRING_UTF8
        QByteArray a = q.toUtf8();
#else
        QByteArray a = q.toLocal8Bit();
#endif
        set((const char*)a);
        return *this;
    }
    WideString& operator=(const QString* q)
    {
#ifdef PPL_QT_STRING_UTF8
        QByteArray a = q->toUtf8();
#else
        QByteArray a = q->toLocal8Bit();
#endif
        set((const char*)a);
        return *this;
    }
    //@}
#endif
};

WideString operator+(const WideString& str1, const WideString& str2);
WideString operator+(const char* str1, const WideString& str2);
WideString operator+(const WideString& str1, const char* str2);
WideString operator+(const wchar_t* str1, const WideString& str2);
WideString operator+(const WideString& str1, const wchar_t* str2);
WideString operator+(const std::string& str1, const WideString& str2);
WideString operator+(const WideString& str1, const std::string& str2);
WideString operator+(const std::wstring& str1, const WideString& str2);
WideString operator+(const WideString& str1, const std::wstring& str2);

std::ostream& operator<<(std::ostream& s, const WideString& str);

} // namespace pplib

#endif /* PPLIB_TYPES_WIDESTRING_H_ */