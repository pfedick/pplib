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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <errno.h>
#include <limits.h>

#include "config_pplib.h"

#include <vector>

#include <pplib/types/bytearray.h>
#include <pplib/types/array.h>
#include <pplib/core/stringfunctions.h>
#include <pplib/core/iconv.h>
#include <pplib/exceptions.h>

namespace pplib
{

/*
** Translation Table as described in RFC1113
*/
static const char cb64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
** Translation Table to decode (created by author)
*/
static const char cd64[] = "|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
** encodeblock
**
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
static void encodeblock(unsigned char in[3], unsigned char out[4], int len)
{
    out[0] = cb64[in[0] >> 2];
    out[1] = cb64[((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)];
    out[2] = (unsigned char)(len > 1 ? cb64[((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)] : '=');
    out[3] = (unsigned char)(len > 2 ? cb64[in[2] & 0x3f] : '=');
}

/*
** encode
**
** base64 encode a stream adding padding and line breaks as per spec.
*/
String ToBase64(const ByteArrayPtr& bin)
{
    String res;
    unsigned char in[3], out[4];
    size_t p = 0, filelen = bin.size();
    res.reserve(((filelen + 2) / 3) * 4);

    while (p < filelen) {
        int len = 0;
        for (int i = 0; i < 3; i++) {
            if (p < filelen) {
                in[i] = (unsigned char)bin.get(p++);
                len++;
            }
        }
        if (len) {
            encodeblock(in, out, len);
            res.append((const char*)out, 4);
        }
    }
    return res;
}

/*
** decodeblock
**
** decode 4 '6-bit' characters into 3 8-bit binary bytes
*/
static void decodeblock(unsigned char in[4], unsigned char out[3])
{
    out[0] = (unsigned char)(in[0] << 2 | in[1] >> 4);
    out[1] = (unsigned char)(in[1] << 4 | in[2] >> 2);
    out[2] = (unsigned char)(((in[2] << 6) & 0xc0) | in[3]);
}

/*
** decode
**
** decode a base64 encoded stream discarding padding, line breaks and noise
*/
ByteArray FromBase64(const String& str)
{
    ByteArray res;
    unsigned char in[4], out[3], v;
    int i, len;
    size_t p = 0, filelen = str.len();
    while (p < filelen) {
        for (len = 0, i = 0; i < 4 && p < filelen; i++) {
            v = 0;
            while (p < filelen && v == 0) {
                v = str.get(p++);
                v = ((v < 43 || v > 122) ? 0 : cd64[v - 43]);
                if (v) {
                    v = ((v == '$') ? 0 : v - 61);
                }
            }
            if (p < filelen + 1) {
                if (v) {
                    len++;
                    in[i] = (unsigned char)(v - 1);
                }
            } else {
                in[i] = 0;
            }
        }
        if (len) {
            decodeblock(in, out);
            res.append(out, len - 1);
            len = 0;
        }
    }
    return res;
}

String StripSlashes(const String& str)
{
    if (str.isEmpty()) return str;
    String ret;
    ret.reserve(str.size());
    const char* ptr = str.c_str();
    while (*ptr) {
        if (*ptr == '\\') {
            ptr++;
            if (*ptr) {
                ret.append(*ptr);
                ptr++;
            }
        } else {
            ret.append(*ptr);
            ptr++;
        }
    }
    return ret;
}

String UpperCaseWords(const String& str)
{
    if (str.isEmpty()) return str;
    WideString ws(str);
    ws.upperCaseWords();
    return String(ws);
}

int StrCmp(const String& s1, const String& s2)
{
    int cmp = s1.strcmp(s2);
    if (cmp < 0) return -1;
    if (cmp > 0) return 1;
    return 0;
}

int StrCaseCmp(const String& s1, const String& s2)
{
    int cmp = s1.strCaseCmp(s2);
    if (cmp < 0) return -1;
    if (cmp > 0) return 1;
    return 0;
}

ssize_t Instr(const char* haystack, const char* needle, size_t start)
{
    if (!haystack || !needle) return -1;
    size_t hlen = strlen(haystack);
    size_t nlen = strlen(needle);
    if (nlen == 0) return -1;
    if (start >= hlen || nlen > hlen - start) return -1;
    const char* p = strstr(haystack + start, needle);
    if (p) return (ssize_t)(p - haystack);
    return -1;
}

ssize_t Instrcase(const char* haystack, const char* needle, size_t start)
{
    if (!haystack || !needle) return -1;
    String hs(haystack);
    String ns(needle);
    return hs.instrCase(ns, start);
}

ssize_t Instr(const wchar_t* haystack, const wchar_t* needle, size_t start)
{
    if (!haystack || !needle) return -1;
    size_t hlen = wcslen(haystack);
    size_t nlen = wcslen(needle);
    if (nlen == 0) return -1;
    if (start >= hlen || nlen > hlen - start) return -1;
    const wchar_t* p = wcsstr(haystack + start, needle);
    if (p) return (ssize_t)(p - haystack);
    return -1;
}

ssize_t Instrcase(const wchar_t* haystack, const wchar_t* needle, size_t start)
{
    if (!haystack || !needle) return -1;
    size_t hlen = wcslen(haystack);
    size_t nlen = wcslen(needle);
    if (nlen == 0) return -1;
    if (start >= hlen || nlen > hlen - start) return -1;

    for (size_t i = start; i <= hlen - nlen; i++) {
        size_t j = 0;
        while (j < nlen && towlower(haystack[i + j]) == towlower(needle[j])) {
            j++;
        }
        if (j == nlen) {
            return (ssize_t)i;
        }
    }
    return -1;
}

String ToString(const char* fmt, ...)
{
    if (!fmt) return String();
    String str;
    va_list args;
    va_start(args, fmt);
    str.vasprintf(fmt, args);
    va_end(args);
    return str;
}

String Replace(const String& string, const String& search, const String& replace)
{
    String Tmp = string;
    Tmp.replace(search, replace);
    return Tmp;
}

Array StrTok(const String& string, const String& div)
{
    Array ret;
    StrTok(ret, string, div);
    return ret;
}

void StrTok(Array& result, const String& string, const String& div)
{
    result.clear();
    if (string.isEmpty()) return;
    result.explode(string, div.isEmpty() ? "\n" : div, 0, true);
}

String EscapeHTMLTags(const String& html)
{
    String s;
    s = html;
    s.replace("&", "&amp;");
    s.replace("<", "&lt;");
    s.replace(">", "&gt;");
    return s;
}

String UnescapeHTMLTags(const String& html)
{
    String s;
    s = html;
    s.replace("&lt;", "<");
    s.replace("&gt;", ">");
    s.replace("&amp;", "&");
    return s;
}

ByteArray Hex2ByteArray(const String& hex)
{
    return ByteArray::fromHex(hex);
}

String ToHex(const ByteArrayPtr& bin)
{
    return bin.toHex();
}

String UrlEncode(const String& text)
{
    const char* source = text.getPtr();
    String ret;
    ret.reserve(text.size() * 3);
    static const char* digits = "0123456789ABCDEF";
    while (*source) {
        unsigned char ch = (unsigned char)*source;
        if (ch == ' ') {
            ret.append('+');
        } else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || (strchr("-_.!~*'()", ch))) {
            ret.append((char)ch);
        } else {
            ret.append('%');
            ret.append(digits[(ch >> 4) & 0x0F]);
            ret.append(digits[ch & 0x0F]);
        }
        source++;
    }
    return ret;
}

static int HexPairValue(const char* code)
{
    int val = 0;
    for (int i = 0; i < 2; i++) {
        val <<= 4;
        char c = code[i];
        if (c >= '0' && c <= '9')
            val += c - '0';
        else if (c >= 'A' && c <= 'F')
            val += c - 'A' + 10;
        else if (c >= 'a' && c <= 'f')
            val += c - 'a' + 10;
        else
            return -1;
    }
    return val;
}

String UrlDecode(const String& text)
{
    const char* source = text.getPtr();
    String ret;
    ret.reserve(text.size());

    while (*source) {
        switch (*source) {
        case '+':
            ret.append(' ');
            break;
        case '%':
            if (source[1] && source[2]) {
                int value = HexPairValue(source + 1);
                if (value >= 0) {
                    ret.append((char)(unsigned char)value);
                    source += 2;
                } else {
                    ret.append('?');
                }
            } else {
                ret.append('?');
            }
            break;
        default:
            ret.append(*source);
            break;
        }
        source++;
    }
    return ret;
}

String Transcode(const char* str, size_t size, const String& fromEncoding, const String& toEncoding)
{
#ifndef HAVE_ICONV
    throw UnsupportedFeatureException("Iconv");
#else
    pplib::Iconv iconv(fromEncoding, toEncoding);
    pplib::ByteArrayPtr source(str, size);
    pplib::ByteArray target;
    iconv.transcode(source, target);
    return String((const char*)target.ptr(), target.size());
#endif
}

String Transcode(const String& str, const String& fromEncoding, const String& toEncoding)
{
#ifndef HAVE_ICONV
    throw UnsupportedFeatureException("Iconv");
#else
    pplib::Iconv iconv(fromEncoding, toEncoding);
    String to;
    iconv.transcode(str, to);
    return to;
#endif
}

} // namespace pplib
