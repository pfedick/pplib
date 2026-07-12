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
#ifndef PPL7_TYPES_STRING_H_
#define PPL7_TYPES_STRING_H_

#include <string>
#include <stdint.h>

namespace ppl7
{

class WideString;
class ByteArrayPtr;
class ByteArray;
class Array;

class String
{
private:
    char* ptr;
    size_t s, stringlen;

public:
    //! @name Konstruktoren und Destruktor
    //@{

    String() noexcept;
    String(const char* str);
    String(const char* str, size_t size);
    String(const String& str);
    String(const WideString& str);
    String(const ByteArrayPtr& str);
    String(const std::string& str);
    String(const std::wstring& str);
    String(String&& other) noexcept;
    ~String() noexcept;
#ifdef WITH_QT
    String(const QString& q)
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
    String(QString* q)
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
    static void setGlobalEncoding(const char* encoding);
    static const char* getGlobalEncoding();
    //@}

    void clear() noexcept;
    size_t capacity() const;
    void reserve(size_t size);
    size_t len() const;
    size_t length() const;
    size_t size() const;
    bool isEmpty() const;
    bool notEmpty() const;
    bool isNumeric() const;
    bool isInteger() const;
    bool isTrue() const;
    bool isFalse() const;

    int strcmp(const String& str, size_t size = (size_t)-1) const;
    int strCaseCmp(const String& str, size_t size = (size_t)-1) const;
    int strcmp(const char* str, size_t size = (size_t)-1) const;
    int strCaseCmp(const char* str, size_t size = (size_t)-1) const;
    String left(size_t len) const;
    String right(size_t len) const;
    String mid(size_t start, size_t len = (size_t)-1) const;
    String substr(size_t start, size_t len = (size_t)-1) const;

    //! @name String setzen und verändern
    //@{
    String& set(const char* str, size_t size = (size_t)-1);
    String& set(const String& str, size_t size = (size_t)-1);
    String& set(const ByteArrayPtr& str, size_t size = (size_t)-1);
    String& set(const WideString& str, size_t size = (size_t)-1);
    String& set(const std::string& str, size_t size = (size_t)-1);
    String& set(const std::wstring& str, size_t size = (size_t)-1);
    String& set(const wchar_t* str, size_t size = (size_t)-1);
    String& set(char c);
    String& set(size_t position, char c);
    String& setf(const char* fmt, ...);
    String& useadr(void* adr, size_t size, size_t stringlen = (size_t)-1);

    String& append(const char* str, size_t size = (size_t)-1);
    String& append(const String& str, size_t size = (size_t)-1);
    String& append(const std::string& str, size_t size = (size_t)-1);
    String& append(const std::wstring& str, size_t size = (size_t)-1);
    String& append(const wchar_t* str, size_t size = (size_t)-1);
    String& appendf(const char* fmt, ...);
    String& append(char c);

    String& prepend(const char* str, size_t size = (size_t)-1);
    String& prepend(const String& str, size_t size = (size_t)-1);
    String& prepend(const std::string& str, size_t size = (size_t)-1);
    String& prepend(const std::wstring& str, size_t size = (size_t)-1);
    String& prepend(const wchar_t* str, size_t size = (size_t)-1);
    String& prependf(const char* fmt, ...);
    String& prepend(char c);

    String& vasprintf(const char* fmt, va_list args);

    String& repeat(size_t num);
    String& repeat(char code, size_t num);
    String& repeat(const String& str, size_t num);
    String repeated(size_t num) const;

    void lowerCase();
    void upperCase();
    void upperCaseWords();
    void trim();
    String trimmed() const;
    String toLowerCase() const;
    String toUpperCase() const;
    String toUpperCaseWords() const;
    void trimLeft();
    void trimRight();
    void trim(const String& chars);
    void trimLeft(const String& chars);
    void trimRight(const String& chars);
    void chopRight(size_t num = 1);
    void chop(size_t num = 1);
    void chopLeft(size_t num = 1);
    void chomp();
    void cut(size_t pos);
    void cut(const String& letter);

    void shl(char c, size_t size);
    void shr(char c, size_t size);

    String strchr(char c) const;
    String strrchr(char c) const;
    String strstr(const String& needle) const;
    ssize_t find(const String& needle, ssize_t start = 0) const;
    ssize_t findCase(const String& needle, ssize_t start = 0) const;
    ssize_t instr(const String& needle, size_t start = 0) const;
    ssize_t instrCase(const String& needle, size_t start = 0) const;
    bool has(const String& needle, bool ignoreCase = false) const;

    bool startsWith(const String& prefix, size_t start = 0, size_t end = (size_t)-1) const;
    bool endsWith(const String& suffix, size_t start = 0, size_t end = (size_t)-1) const;
    String join(const ppl7::Array& iterable) const;

    String& stripSlashes();

    String& replace(const String& search, const String& replacement);

    /*
    String& pregReplace(const String& expression, const String& replacement, int max=0);
    String& pregEscape();
    bool pregMatch(const String& expression) const;
    bool pregMatch(const String& expression, Array& matches, size_t maxmatches=16) const;
    */
    //@}

    //! @name String ausgeben und auslesen
    //@{
    void print(bool withNewline = false) const noexcept;
    void printnl() const noexcept;
    void hexDump() const;
    char get(ssize_t pos) const;
    const char* getPtr() const;
    const char* c_str() const;

    ByteArray toEncoding(const char* encoding) const;
    ByteArray toUCS4() const;
    ByteArray toUtf8() const;
    String& fromUCS4(const uint32_t* str, size_t size = (size_t)-1);
    String& fromUCS4(const ByteArrayPtr& bin);
    String md5() const;

    int toInt() const;
    unsigned int toUnsignedInt() const;
    int64_t toInt64() const;
    uint64_t toUnsignedInt64() const;
    WideString toWideString() const;
    bool toBool() const;
    long toLong() const;
    unsigned long toUnsignedLong() const;
    long long toLongLong() const;
    unsigned long long toUnsignedLongLong() const;
    float toFloat() const;
    double toDouble() const;
    const char* toChar() const;

    //@}

    //! @name Operatoren
    //@{
    operator const char*() const;
    operator const unsigned char*() const;
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

    char operator[](ssize_t pos) const;
    char& operator[](ssize_t pos);

    String& operator=(const char* str);
    String& operator=(const wchar_t* str);
    String& operator=(const String& str);
    String& operator=(const WideString& str);
    String& operator=(const std::string& str);
    String& operator=(const std::wstring& str);
    String& operator=(char c);
    String& operator=(String&& other) noexcept;
    String& operator+=(const char* str);
    String& operator+=(const wchar_t* str);
    String& operator+=(const String& str);
    String& operator+=(const std::string& str);
    String& operator+=(const std::wstring& str);
    String& operator+=(char c);
    bool operator<(const String& str) const;
    bool operator<=(const String& str) const;
    bool operator==(const String& str) const;
    bool operator!=(const String& str) const;
    bool operator>=(const String& str) const;
    bool operator>(const String& str) const;

    bool operator<(const char* str) const;
    bool operator<=(const char* str) const;
    bool operator==(const char* str) const;
    bool operator!=(const char* str) const;
    bool operator>=(const char* str) const;
    bool operator>(const char* str) const;

    //@}

    //! @name Iteratoren
    //@{
    typedef char* iterator;
    typedef const char* const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;

    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    const_reverse_iterator crbegin() const noexcept;
    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;
    const_reverse_iterator crend() const noexcept;
    //@}

#ifdef PPL_WITH_QT6
    operator QAnyStringView() const
    {
#ifdef PPL_QT_STRING_UTF8
        return QAnyStringView(ptr, stringlen);
#else
        return QAnyStringView(ptr, stringlen);
#endif
    }
#endif

#ifdef WITH_QT
    //! @name Operatoren zur Verwendung der Klasse mit Qt
    //@{
    operator const QString() const
    {
#ifdef PPL_QT_STRING_UTF8
        return QString::fromUtf8(ptr, stringlen);
#else
        return QString::fromLocal8Bit(ptr, stringlen);
#endif
    }

    operator const QVariant() const
    {
#ifdef PPL_QT_STRING_UTF8
        QVariant v = QString::fromUtf8(ptr, stringlen);
#else
        QVariant v = QString::fromLocal8Bit(ptr, stringlen);
#endif
        return v;
    }

    String& operator=(const QString& q)
    {
#ifdef PPL_QT_STRING_UTF8
        QByteArray a = q.toUtf8();
#else
        QByteArray a = q.toLocal8Bit();
#endif
        set((const char*)a);
        return *this;
    }
    String& operator=(const QString* q)
    {
#ifdef PPL_QT_STRING_UTF8
        QByteArray a = q->toUtf8();
#else
        QByteArray a = q->toLocal8Bit();
#endif
        set((const char*)a);
        return *this;
    }
    String& operator+=(const QString& q)
    {
#ifdef PPL_QT_STRING_UTF8
        QByteArray a = q.toUtf8();
#else
        QByteArray a = q.toLocal8Bit();
#endif
        append((const char*)a);
        return *this;
    }
    //@}
#endif
};

String operator+(const String& str1, const String& str2);
String operator+(const char* str1, const String& str2);
String operator+(const String& str1, const char* str2);
String operator+(const wchar_t* str1, const String& str2);
String operator+(const String& str1, const wchar_t* str2);
String operator+(const std::string& str1, const String& str2);
String operator+(const String& str1, const std::string& str2);
String operator+(const std::wstring& str1, const String& str2);
String operator+(const String& str1, const std::wstring& str2);

std::ostream& operator<<(std::ostream& s, const String& str);

} // namespace ppl7

#endif /* PPL7_TYPES_STRING_H_ */