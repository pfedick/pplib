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

/** @brief String zur Verwendung in einer URL umwandeln
 *
 * Mit dieser Funktion kann ein beliebiger String so umkodiert werden, dass er als
 * Parameter in einer URL verwendet werden kann. Dabei werden alle Spaces durch "+" ersetzt
 * und alle nicht alphanummerischen Zeichen mit Ausnahme von "-_.!~*'()" in ihre Hex-Werte
 * mit vorangestelltem Prozentzeichen umgewandelt.
 *
 * @param text Der zu kodierende Text
 * @return Der URL-kodierte Text
 *
 * \example
 * \code
 * pplib::String text="Hallo Welt! 1+1=2";
 * printf("%s\n",(const char*)pplib::UrlEncode(text));
 * \endcode
 * ergibt:
 * \verbatim
Hallo+Welt!+1%2B1%3D2
\endverbatim
 * \see
 * Mit UrlDecode kann der Kodierte String wieder dekodiert werden
 */
String UrlEncode(const String& text);

/** @brief URL-kodierten String dekodieren
 *
 * Mit dieser statischen Funktion kann ein URL-kodierter String dekodiert werden.
 *
 * @param text Der zu URL-kodierte String
 * @return Der dekodierte String
 *
 * \example
 * \code
 * pplib::String text=L"Hallo+Welt!+1%2B1%3D2";
 * printf("%s\n",(const char*)pplib::UrlDecode(text));
 * \endcode
 * ergibt:
 * \verbatim
Hallo Welt! 1+1=2";
\endverbatim
 * \see
 * Mit UrlEncode kann ein unkodierter String kodiert werden.
 */
String UrlDecode(const String& text);

/** @brief Schneidet Leerzeichen, Tabs Returns und Linefeeds am Anfang und Ende des Strings ab
 * @relates String
 *
 * Schneidet Leerzeichen, Tabs Returns und Linefeeds am Anfang und Ende des Strings ab
 * @param str Eingabe-String
 * @return Neuer String
 */
inline String Trim(const String& str)
{
    return str.trimmed();
}
inline String UpperCase(const String& str)
{
    return str.toUpperCase();
}

inline String LowerCase(const String& str)
{
    return str.toLowerCase();
}

/** @brief Anfangsbuchstaben der Wörter groß
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
inline ssize_t Instr(const String& haystack, const String& needle, size_t start = 0)
{
    return haystack.instr(needle, start);
}

inline ssize_t InstrCase(const String& haystack, const String& needle, size_t start = 0)
{
    return haystack.instrCase(needle, start);
}

inline ssize_t Instrcase(const String& haystack, const String& needle, size_t start = 0)
{
    return haystack.instrCase(needle, start);
}

ssize_t Instr(const char* haystack, const char* needle, size_t start = 0);
ssize_t Instrcase(const char* haystack, const char* needle, size_t start = 0);
ssize_t Instr(const wchar_t* haystack, const wchar_t* needle, size_t start = 0);
ssize_t Instrcase(const wchar_t* haystack, const wchar_t* needle, size_t start = 0);

String ToString(const char* fmt, ...);
inline String Left(const String& str, size_t num)
{
    return str.left(num);
}
inline String Right(const String& str, size_t num)
{
    return str.right(num);
}

inline String Mid(const String& str, size_t start, size_t num = (size_t)-1)
{
    return str.mid(start, num);
}

inline String SubStr(const String& str, size_t start, size_t num = (size_t)-1)
{
    return str.substr(start, num);
}

String Replace(const String& string, const String& search, const String& replace);

/**@brief Wiederholt einen String
 * @relates String
 *
 * Wiederholt einen String \p count mal und gibt den neuen String zurück.
 * @param str Eingabe-String
 * @param count Anzahl der Wiederholungen
 * @return Neuer String
 */
inline String Repeat(const String& str, size_t count)
{
    return str.repeated(count);
}

String Transcode(const char* str, size_t size, const String& fromEncoding, const String& toEncoding);
String Transcode(const String& str, const String& fromEncoding, const String& toEncoding);

inline bool IsTrue(const String& str)
{
    return str.isTrue();
}

inline bool IsDigits(const String& str)
{
    return str.isDigits();
}

inline bool IsInteger(const String& str)
{
    return str.isInteger();
}

inline bool IsNumeric(const String& str)
{
    return str.isNumeric();
}

/** @brief String anhand eines Trennzeichens zerlegen
 *
 * Die StrTok-Funktion zerlegt den String \p string in mehrere Teile, wobei
 * \div als Trenner verwendet und das Ergebnis als Array zurückgegeben wird
 * Der Trenner \p div kann aus einem oder mehreren Zeichen bestehen und
 * wird nicht im Ergebnis übernommen. Eine Sequenz von mehreren Trennern
 * hintereinander wird als ein Trenner interpretiert. Trenner am Anfang und Ende
 * des Strings werden ignoriert. Mit anderen Worten: im Ergebnis gibt es keine
 * leeren Strings.
 * \note
 * Das Verhalten der Funktion entspricht dem Verhalten der C-Funktion strtok
 *
 * @param[in] string String, der zerlegt werden soll
 * @param[in] div String, der als Trenner verwendet wird
 * @return Array mit den Bestandteilen des zerlegten Strings
 */
Array StrTok(const String& string, const String& div = String("\n"));

/** @brief String anhand eines Trennzeichens zerlegen
 *
 * Die StrTok-Funktion zerlegt den String \p string in mehrere Teile, wobei
 * \div als Trenner verwendet und das Ergebnis im Array \p result gespeichert
 * wird. Der Trenner \p div kann aus einem oder mehreren Zeichen bestehen und
 * wird nicht im Ergebnis übernommen. Eine Sequenz von mehreren Trennern
 * hintereinander wird als ein Trenner interpretiert. Trenner am Anfang und Ende
 * des Strings werden ignoriert. Mit anderen Worten: im Ergebnis gibt es keine
 * leeren Strings.
 * \note
 * Das Verhalten der Funktion entspricht dem Verhalten der C-Funktion strtok
 *
 * @param[out] result Array, in dem die Ergebnisstrings gespeichert werden
 * @param[in] string String, der zerlegt werden soll
 * @param[in] div String, der als Trenner verwendet wird. Default=Newline
 */
void StrTok(Array& result, const String& string, const String& div = String("\n"));

}; // namespace pplib

#endif // PPLIB_CORE_STRINGFUNCTIONS_H_
