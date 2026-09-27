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

#ifndef PPLIB_CORE_STRINGFUNCTIONS_H_
#define PPLIB_CORE_STRINGFUNCTIONS_H_

#include <stdint.h>
#include <pplib/types/string.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/types/datetime.h>
#include <pplib/types/assocarray.h>
#include <pplib/core/random.h>
#include <pplib/core/args.h>

namespace pplib
{

String ToBase64(const ByteArrayPtr& bin);
ByteArray FromBase64(const String& str);

/**@brief Entfernt Backslashes aus einem String
 * @relates String
 *
 * Entfernt Backslashes aus einem String
 * @param str Eingabe-String
 * @return Neuer String
 */
String StripSlashes(const String& str);
String EscapeHTMLTags(const String& html);
String UnescapeHTMLTags(const String& html);
ByteArray Hex2ByteArray(const String& hex);
String ToHex(const ByteArrayPtr& bin);
String UrlEncode(const String& text);
String UrlDecode(const String& text);

String Trim(const String& str);
String UpperCase(const String& str);
String LowerCase(const String& str);

/**@brief Anfangsbuchstaben der Wörter groß
 *
 * Diese Funktion wandelt die Anfangsbuchstaben aller im String enthaltenen Wörter in
 * Großbuchstaben um.
 *
 * @param str Eingabe-String
 * @return Neuer String
 *
 */
String UpperCaseWords(const String& str);
int StrCmp(const String& s1, const String& s2);
int StrCaseCmp(const String& s1, const String& s2);
ssize_t Instr(const String& haystack, const String& needle, size_t start = 0);
ssize_t InstrCase(const String& haystack, const String& needle, size_t start = 0);
ssize_t Instr(const char* haystack, const char* needle, size_t start = 0);
ssize_t Instrcase(const char* haystack, const char* needle, size_t start = 0);
ssize_t Instr(const wchar_t* haystack, const wchar_t* needle, size_t start = 0);
ssize_t Instrcase(const wchar_t* haystack, const wchar_t* needle, size_t start = 0);

String ToString(const char* fmt, ...);
String Left(const String& str, size_t num);
String Right(const String& str, size_t num);
String Mid(const String& str, size_t start, size_t num = (size_t)-1);
String SubStr(const String& str, size_t start, size_t num = (size_t)-1);
String Replace(const String& string, const String& search, const String& replace);

/**@brief Wiederholt einen String
 * @relates String
 *
 * Wiederholt einen String \p count mal und gibt den neuen String zurück.
 * @param str Eingabe-String
 * @param count Anzahl der Wiederholungen
 * @return Neuer String
 */
String Repeat(const String& str, size_t count);

String Transcode(const char* str, size_t size, const String& fromEncoding, const String& toEncoding);
String Transcode(const String& str, const String& fromEncoding, const String& toEncoding);
bool IsTrue(const String& str);
bool IsDigits(const String& str);
bool IsInteger(const String& str);
bool IsNumeric(const String& str);

Array StrTok(const String& string, const String& div = String("\n"));
void StrTok(Array& result, const String& string, const String& div = String("\n"));

}; // namespace pplib

#endif // PPLIB_CORE_STRINGFUNCTIONS_H_
