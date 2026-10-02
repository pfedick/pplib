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
#include "config_pplib.h"
#include <pplib/core/regex.h>

#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/array.h>
#include <pplib/types/bytearray.h>
#include <pplib/core/functions.h>
#include <pplib/exceptions.h>
#ifdef HAVE_PCRE2
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#endif

#if defined(HAVE_PCRE2_BITS_16) && (WCHAR_MAX <= 0xffff)
#define HAVE_PCRE2_WIDE
#define pcre2_code_wide pcre2_code_16
#define pcre2_code_copy_wide pcre2_code_copy_16
#define pcre2_code_free_wide pcre2_code_free_16
#define pcre2_compile_wide pcre2_compile_16
#define pcre2_match_data_create_from_pattern_wide pcre2_match_data_create_from_pattern_16
#define pcre2_match_wide pcre2_match_16
#define pcre2_match_data_free_wide pcre2_match_data_free_16
#define pcre2_get_ovector_pointer_wide pcre2_get_ovector_pointer_16
#define pcre2_match_data_wide pcre2_match_data_16
#define PCRE2_SPTR_WIDE PCRE2_SPTR16
#define pcre2_bits_wide 16

#elif defined(HAVE_PCRE2_BITS_32) && (WCHAR_MAX > 0xffff)
#define HAVE_PCRE2_WIDE
#define pcre2_code_wide pcre2_code_32
#define pcre2_code_copy_wide pcre2_code_copy_32
#define pcre2_code_free_wide pcre2_code_free_32
#define pcre2_compile_wide pcre2_compile_32
#define pcre2_match_data_create_from_pattern_wide pcre2_match_data_create_from_pattern_32
#define pcre2_match_wide pcre2_match_32
#define pcre2_match_data_free_wide pcre2_match_data_free_32
#define pcre2_get_ovector_pointer_wide pcre2_get_ovector_pointer_32
#define pcre2_match_data_wide pcre2_match_data_32
#define PCRE2_SPTR_WIDE PCRE2_SPTR32
#define pcre2_bits_wide 32
#endif

namespace pplib
{

RegEx::Pattern::Pattern()
{
    p = NULL;
    bits = 0;
}

RegEx::Pattern::Pattern(const Pattern& other)
{
    p = NULL;
    bits = other.bits;
    if (other.p) {
        if (bits == 8) {
#ifdef HAVE_PCRE2_BITS_8
            p = pcre2_code_copy_8((pcre2_code_8*)other.p);
#endif
        } else if (bits == 16 || bits == 32) {
#ifdef HAVE_PCRE2_WIDE
            p = pcre2_code_copy_wide((pcre2_code_wide*)other.p);
#endif
        }
        if (!p) throw OutOfMemoryException("copy of RegEx::Pattern failed");
    }
}

RegEx::Pattern::Pattern(Pattern&& other) noexcept
{
    p = other.p;
    bits = other.bits;
    other.p = NULL;
    other.bits = 0;
}

RegEx::Pattern::~Pattern()
{
    if (p) {
#ifdef HAVE_PCRE2_BITS_8
        if (bits == 8) pcre2_code_free_8((pcre2_code_8*)p);
#endif
#ifdef HAVE_PCRE2_WIDE
        if (bits == 16 || bits == 32) pcre2_code_free_wide((pcre2_code_wide*)p);
#endif
    }
    p = NULL;
    bits = 0;
}

void RegEx::Pattern::swap(Pattern& other) noexcept
{
    std::swap(p, other.p);
    std::swap(bits, other.bits);
}

RegEx::Pattern& RegEx::Pattern::operator=(const Pattern& other)
{
    if (this != &other) {
        Pattern tmp(other);
        swap(tmp);
    }
    return *this;
}

RegEx::Pattern& RegEx::Pattern::operator=(Pattern&& other) noexcept
{
    if (this != &other) {
        Pattern tmp(std::move(other));
        swap(tmp);
    }
    return *this;
}

RegEx::Pattern RegEx::compile(const String& regex, int flags)
{
#ifndef HAVE_PCRE2
    throw UnsupportedFeatureException("PCRE2");
#endif
#ifndef HAVE_PCRE2_BITS_8
    throw UnsupportedFeatureException("PCRE2 with 8 bits character width");
#else
    PCRE2_SIZE erroffset;
    int errorcode;
    int options = PCRE2_UTF;
    if (flags & Flags::CASELESS) options |= PCRE2_CASELESS;
    if (flags & Flags::ANCHORED) options |= PCRE2_ANCHORED;
    if (flags & Flags::MULTILINE) options |= PCRE2_MULTILINE;
    if (flags & Flags::EXTENDED) options |= PCRE2_EXTENDED;
    if (flags & Flags::DOTALL) options |= PCRE2_DOTALL;
    if (flags & Flags::UNGREEDY) options |= PCRE2_UNGREEDY;

    const char* r = regex.c_str();
    pcre2_code_8* re = NULL;
    if (r[0] == '/') { // PerlRegEx
        const char* last_slash = ::strrchr(r, '/');
        if (last_slash > r) {
            String expr = regex.mid(1, (last_slash - r) - 1);
            const char* oo = last_slash + 1;
            if (::strchr(oo, 'i')) options |= PCRE2_CASELESS;
            if (::strchr(oo, 'm')) options |= PCRE2_MULTILINE;
            if (::strchr(oo, 'x')) options |= PCRE2_EXTENDED;
            if (::strchr(oo, 's')) options |= PCRE2_DOTALL;
            if (::strchr(oo, 'a')) options |= PCRE2_ANCHORED;
            if (::strchr(oo, 'u')) options |= PCRE2_UNGREEDY;
            re = pcre2_compile_8((PCRE2_SPTR8)expr.c_str(), PCRE2_ZERO_TERMINATED, options, &errorcode, &erroffset, NULL);
        } else {
            re = pcre2_compile_8((PCRE2_SPTR8)regex.c_str(), PCRE2_ZERO_TERMINATED, options, &errorcode, &erroffset, NULL);
        }
    } else {
        re = pcre2_compile_8((PCRE2_SPTR8)regex.c_str(), PCRE2_ZERO_TERMINATED, options, &errorcode, &erroffset, NULL);
    }
    if (!re) throw IllegalRegularExpressionException();

    RegEx::Pattern pattern;
    pattern.bits = 8;
    pattern.p = re;
    return pattern;
#endif
}

RegEx::Pattern RegEx::compile(const WideString& regex, int flags)
{
#ifndef HAVE_PCRE2
    throw UnsupportedFeatureException("PCRE2");
#endif
#ifndef HAVE_PCRE2_WIDE
    throw UnsupportedFeatureException("PCRE2 with wide character width");
#else
    PCRE2_SIZE erroffset;
    int errorcode;
    int options = 0;
#if defined(HAVE_PCRE2_BITS_16) && (WCHAR_MAX <= 0xffff)
    options |= PCRE2_UTF;
#endif
    if (flags & Flags::CASELESS) options |= PCRE2_CASELESS;
    if (flags & Flags::ANCHORED) options |= PCRE2_ANCHORED;
    if (flags & Flags::MULTILINE) options |= PCRE2_MULTILINE;
    if (flags & Flags::EXTENDED) options |= PCRE2_EXTENDED;
    if (flags & Flags::DOTALL) options |= PCRE2_DOTALL;
    if (flags & Flags::UNGREEDY) options |= PCRE2_UNGREEDY;

    const wchar_t* r = regex.getPtr();
    pcre2_code_wide* re = NULL;

    if (r[0] == L'/') { // PerlRegEx
        const wchar_t* last_slash = ::wcsrchr(r, L'/');
        if (last_slash > r) {
            WideString expr = regex.mid(1, (last_slash - r) - 1);
            const wchar_t* oo = last_slash + 1;
            if (::wcschr(oo, L'i')) options |= PCRE2_CASELESS;
            if (::wcschr(oo, L'm')) options |= PCRE2_MULTILINE;
            if (::wcschr(oo, L'x')) options |= PCRE2_EXTENDED;
            if (::wcschr(oo, L's')) options |= PCRE2_DOTALL;
            if (::wcschr(oo, L'a')) options |= PCRE2_ANCHORED;
            if (::wcschr(oo, L'u')) options |= PCRE2_UNGREEDY;
            re = pcre2_compile_wide((PCRE2_SPTR_WIDE)expr.getPtr(), PCRE2_ZERO_TERMINATED, options, &errorcode, &erroffset, NULL);
        } else {
            re = pcre2_compile_wide((PCRE2_SPTR_WIDE)regex.getPtr(), PCRE2_ZERO_TERMINATED, options, &errorcode, &erroffset, NULL);
        }
    } else {
        re = pcre2_compile_wide((PCRE2_SPTR_WIDE)regex.getPtr(), PCRE2_ZERO_TERMINATED, options, &errorcode, &erroffset, NULL);
    }
    if (!re) throw IllegalRegularExpressionException();

    RegEx::Pattern pattern;
    pattern.bits = pcre2_bits_wide;
    pattern.p = re;
    return pattern;
#endif
}

bool RegEx::match(const String& regex, const String& subject, int flags)
{
    RegEx::Pattern pattern = RegEx::compile(regex, flags);
    return RegEx::match(pattern, subject);
}

bool RegEx::match(const WideString& regex, const WideString& subject, int flags)
{
    RegEx::Pattern pattern = RegEx::compile(regex, flags);
    return RegEx::match(pattern, subject);
}

bool RegEx::match(const Pattern& pattern, const String& subject)
{
    if (pattern.p == NULL) throw IllegalRegularExpressionException();
    if (pattern.bits != 8) throw IllegalArgumentException("Pattern was compiled for a different character width");
#ifndef HAVE_PCRE2_BITS_8
    throw UnsupportedFeatureException("PCRE2 with 8 bits character width");
#else
    pcre2_match_data_8* md = pcre2_match_data_create_from_pattern_8((pcre2_code_8*)pattern.p, NULL);
    if (!md) throw OutOfMemoryException();
    int rc = pcre2_match_8((pcre2_code_8*)pattern.p, (PCRE2_SPTR8)subject.c_str(), subject.size(), 0, 0, md, NULL);
    pcre2_match_data_free_8(md);
    if (rc < 0) {
        if (rc == PCRE2_ERROR_NOMATCH) return false;
        throw OperationFailedException();
    }
    return true;
#endif
}

bool RegEx::match(const Pattern& pattern, const WideString& subject)
{
    if (pattern.p == NULL) throw IllegalRegularExpressionException();
#ifndef HAVE_PCRE2_WIDE
    throw UnsupportedFeatureException("PCRE2 with wide character width");
#else
    if (pattern.bits != pcre2_bits_wide) throw IllegalArgumentException("Pattern was compiled for a different character width");
    pcre2_match_data_wide* md = pcre2_match_data_create_from_pattern_wide((pcre2_code_wide*)pattern.p, NULL);
    if (!md) throw OutOfMemoryException();
    int rc = pcre2_match_wide((pcre2_code_wide*)pattern.p, (PCRE2_SPTR_WIDE)subject.getPtr(), subject.size(), 0, 0, md, NULL);
    pcre2_match_data_free_wide(md);
    if (rc < 0) {
        if (rc == PCRE2_ERROR_NOMATCH) return false;
        throw OperationFailedException();
    }
    return true;
#endif
}

bool RegEx::capture(const String& regex, const String& subject, std::vector<String>& matches, int flags)
{
    RegEx::Pattern pattern = RegEx::compile(regex, flags);
    return RegEx::capture(pattern, subject, matches);
}

bool RegEx::capture(const Pattern& pattern, const String& subject, std::vector<String>& matches)
{
    if (pattern.p == NULL) throw IllegalRegularExpressionException();
    if (pattern.bits != 8) throw IllegalArgumentException("Pattern was compiled for a different character width");
#ifndef HAVE_PCRE2_BITS_8
    throw UnsupportedFeatureException("PCRE2 with 8 bits character width");
#else
    pcre2_match_data_8* md = pcre2_match_data_create_from_pattern_8((pcre2_code_8*)pattern.p, NULL);
    if (!md) throw OutOfMemoryException();
    PCRE2_SPTR8 subj = (PCRE2_SPTR8)subject.c_str();
    int rc = pcre2_match_8((pcre2_code_8*)pattern.p, subj, subject.size(), 0, 0, md, NULL);
    if (rc < 0) {
        pcre2_match_data_free_8(md);
        if (rc == PCRE2_ERROR_NOMATCH) return false;
        throw OperationFailedException();
    }
    matches.clear();
    PCRE2_SIZE* ovector = pcre2_get_ovector_pointer_8(md);
    for (int i = 0; i < rc; i++) {
        PCRE2_SPTR8 substring_start = subj + ovector[2 * i];
        PCRE2_SIZE substring_length = ovector[2 * i + 1] - ovector[2 * i];
        matches.push_back(String((const char*)substring_start, substring_length));
    }
    pcre2_match_data_free_8(md);
    return true;
#endif
}

bool RegEx::capture(const WideString& regex, const WideString& subject, std::vector<WideString>& matches, int flags)
{
    RegEx::Pattern pattern = RegEx::compile(regex, flags);
    return RegEx::capture(pattern, subject, matches);
}

bool RegEx::capture(const Pattern& pattern, const WideString& subject, std::vector<WideString>& matches)
{
    if (pattern.p == NULL) throw IllegalRegularExpressionException();
#ifndef HAVE_PCRE2_WIDE
    throw UnsupportedFeatureException("PCRE2 with wide character width");
#else
    if (pattern.bits != pcre2_bits_wide) throw IllegalArgumentException("Pattern was compiled for a different character width");
    pcre2_match_data_wide* md = pcre2_match_data_create_from_pattern_wide((pcre2_code_wide*)pattern.p, NULL);
    if (!md) throw OutOfMemoryException();
    PCRE2_SPTR_WIDE subj = (PCRE2_SPTR_WIDE)subject.getPtr();
    int rc = pcre2_match_wide((pcre2_code_wide*)pattern.p, subj, subject.size(), 0, 0, md, NULL);
    if (rc < 0) {
        pcre2_match_data_free_wide(md);
        if (rc == PCRE2_ERROR_NOMATCH) return false;
        throw OperationFailedException();
    }
    matches.clear();
    PCRE2_SIZE* ovector = pcre2_get_ovector_pointer_wide(md);
    for (int i = 0; i < rc; i++) {
        PCRE2_SPTR_WIDE substring_start = subj + ovector[2 * i];
        PCRE2_SIZE substring_length = ovector[2 * i + 1] - ovector[2 * i];
        matches.push_back(WideString((const wchar_t*)substring_start, substring_length));
    }
    pcre2_match_data_free_wide(md);
    return true;
#endif
}

String RegEx::replace(const String& regex, const String& subject, const String& replacement, int flags, int max)
{
    RegEx::Pattern pattern = RegEx::compile(regex, flags);
    return RegEx::replace(pattern, subject, replacement, max);
}

String RegEx::replace(const Pattern& pattern, const String& subject, const String& replacement, int max)
{
    if (pattern.p == NULL) throw IllegalRegularExpressionException();
    if (pattern.bits != 8) throw IllegalArgumentException("Pattern was compiled for a different character width");
#ifndef HAVE_PCRE2_BITS_8
    throw UnsupportedFeatureException("PCRE2 with 8 bits character width");
#else
    pcre2_match_data_8* md = pcre2_match_data_create_from_pattern_8((pcre2_code_8*)pattern.p, NULL);
    if (!md) throw OutOfMemoryException();
    String result;
    PCRE2_SIZE offset = 0;
    int count = 0;
    const char* subj_ptr = subject.c_str();
    size_t subj_len = subject.size();

    while (offset <= subj_len) {
        int rc = pcre2_match_8((pcre2_code_8*)pattern.p, (PCRE2_SPTR8)subj_ptr, subj_len, offset, 0, md, NULL);
        if (rc < 0) {
            pcre2_match_data_free_8(md);
            if (rc == PCRE2_ERROR_NOMATCH) {
                result += subject.mid(offset);
                return result;
            }
            throw OperationFailedException();
        }
        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer_8(md);
        result += subject.mid(offset, ovector[0] - offset);
        result += replacement;
        count++;

        if (max > 0 && count >= max) {
            result += subject.mid(ovector[1]);
            pcre2_match_data_free_8(md);
            return result;
        }

        if (ovector[0] == ovector[1]) {
            // Zero-length match: Mindestens 1 UTF-8 Zeichen unverändert kopieren
            if (ovector[1] >= subj_len) {
                pcre2_match_data_free_8(md);
                return result;
            }
            size_t skip = 1;
            unsigned char c = (unsigned char)subj_ptr[ovector[1]];
            if ((c & 0xE0) == 0xC0)
                skip = 2;
            else if ((c & 0xF0) == 0xE0)
                skip = 3;
            else if ((c & 0xF8) == 0xF0)
                skip = 4;
            if (ovector[1] + skip > subj_len) skip = subj_len - ovector[1];
            result += subject.mid(ovector[1], skip);
            offset = ovector[1] + skip;
        } else {
            offset = ovector[1];
        }
    }
    result += subject.mid(offset);
    pcre2_match_data_free_8(md);
    return result;
#endif
}

WideString RegEx::replace(const WideString& regex, const WideString& subject, const WideString& replacement, int flags, int max)
{
    RegEx::Pattern pattern = RegEx::compile(regex, flags);
    return RegEx::replace(pattern, subject, replacement, max);
}

WideString RegEx::replace(const Pattern& pattern, const WideString& subject, const WideString& replacement, int max)
{
    if (pattern.p == NULL) throw IllegalRegularExpressionException();
#ifndef HAVE_PCRE2_WIDE
    throw UnsupportedFeatureException("PCRE2 with wide character width");
#else
    if (pattern.bits != pcre2_bits_wide) throw IllegalArgumentException("Pattern was compiled for a different character width");
    pcre2_match_data_wide* md = pcre2_match_data_create_from_pattern_wide((pcre2_code_wide*)pattern.p, NULL);
    if (!md) throw OutOfMemoryException();
    WideString result;
    PCRE2_SIZE offset = 0;
    int count = 0;
    const wchar_t* subj_ptr = subject.getPtr();
    size_t subj_len = subject.size();

    while (offset <= subj_len) {
        int rc = pcre2_match_wide((pcre2_code_wide*)pattern.p, (PCRE2_SPTR_WIDE)subj_ptr, subj_len, offset, 0, md, NULL);
        if (rc < 0) {
            pcre2_match_data_free_wide(md);
            if (rc == PCRE2_ERROR_NOMATCH) {
                result += subject.mid(offset);
                return result;
            }
            throw OperationFailedException();
        }
        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer_wide(md);
        result += subject.mid(offset, ovector[0] - offset);
        result += replacement;
        count++;

        if (max > 0 && count >= max) {
            result += subject.mid(ovector[1]);
            pcre2_match_data_free_wide(md);
            return result;
        }

        if (ovector[0] == ovector[1]) {
            // Zero-length match: Mindestens 1 UTF-16/32 Zeichen unverändert kopieren
            if (ovector[1] >= subj_len) {
                pcre2_match_data_free_wide(md);
                return result;
            }
            size_t skip = 1;
#if defined(HAVE_PCRE2_BITS_16) && (WCHAR_MAX <= 0xffff)
            if (ovector[1] + 1 < subj_len && subj_ptr[ovector[1]] >= 0xD800 && subj_ptr[ovector[1]] <= 0xDBFF &&
                subj_ptr[ovector[1] + 1] >= 0xDC00 && subj_ptr[ovector[1] + 1] <= 0xDFFF) {
                skip = 2;
            }
#endif
            result += subject.mid(ovector[1], skip);
            offset = ovector[1] + skip;
        } else {
            offset = ovector[1];
        }
    }
    result += subject.mid(offset);
    pcre2_match_data_free_wide(md);
    return result;
#endif
}

/*! \brief Fügt dem String Escape-Zeichen zu, zur Verwendung in einem Regulären Ausdruck
 *
 * \desc
 * Der Befehl scannt den String nach Zeichen mit besonderer Bedeutung in einer Perl-Regular-Expression und
 * escaped diese mit einem Backslash. Das Ergebnis kann dann in einer Regular Expression verwendet werden.
 *
 * Folgende Zeichen werden escaped: \ . ^ $ * + - ? ( ) [ ] { } | /
 */
String RegEx::escape(const String& subject)
{
    if (subject.isEmpty()) return subject;

    String t;
    t.reserve(subject.size() * 2);
    const char* ptr = subject.c_str();
    for (size_t i = 0; i < subject.size(); i++) {
        char c = ptr[i];
        switch (c) {
        case '\\':
        case '.':
        case '^':
        case '$':
        case '*':
        case '+':
        case '-':
        case '?':
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case '|':
        case '/':
            t += '\\';
            t += c;
            break;
        default:
            t += c;
            break;
        }
    }
    return t;
}

/*! \brief Fügt dem String Escape-Zeichen zu, zur Verwendung in einem Regulären Ausdruck
 *
 * \desc
 * Der Befehl scannt den String nach Zeichen mit besonderer Bedeutung in einer Perl-Regular-Expression und
 * escaped diese mit einem Backslash. Das Ergebnis kann dann in einer Regular Expression verwendet werden.
 *
 * Folgende Zeichen werden escaped: \ . ^ $ * + - ? ( ) [ ] { } | /
 */
WideString RegEx::escape(const WideString& subject)
{
    if (subject.isEmpty()) return subject;

    WideString t;
    t.reserve(subject.size() * 2);
    const wchar_t* ptr = subject.getPtr();
    for (size_t i = 0; i < subject.size(); i++) {
        wchar_t c = ptr[i];
        switch (c) {
        case L'\\':
        case L'.':
        case L'^':
        case L'$':
        case L'*':
        case L'+':
        case L'-':
        case L'?':
        case L'(':
        case L')':
        case L'[':
        case L']':
        case L'{':
        case L'}':
        case L'|':
        case L'/':
            t += L'\\';
            t += c;
            break;
        default:
            t += c;
            break;
        }
    }
    return t;
}

} // namespace pplib
