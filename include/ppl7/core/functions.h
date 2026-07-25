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

#ifndef PPL7_FUNCTIONS_H_
#define PPL7_FUNCTIONS_H_

#include <stdint.h>
#include <ppl7/types/string.h>
#include <ppl7/types/bytearrayptr.h>
#include <ppl7/types/datetime.h>

namespace ppl7
{

// Functions
void PrintDebug(const char* format, ...);
void PrintDebugTime(const char* format, ...);
void SetGlobalOutput(int type);

void HexDump(const void* address, size_t bytes, bool skipheader);
void HexDump(const void* address, size_t bytes);
String ToBase64(const ByteArrayPtr& bin);
ByteArray FromBase64(const String& str);
uint32_t Crc32(const void* buffer, size_t size);
String Md5(const void* buffer, size_t size);
String Md5(const ByteArrayPtr& buffer);
double Calc(const String& expression);

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

String Left(const String& str, size_t num);
String Right(const String& str, size_t num);
String Mid(const String& str, size_t start, size_t num = (size_t)-1);
String SubStr(const String& str, size_t start, size_t num = (size_t)-1);
String ToString(const char* fmt, ...);
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
Array StrTok(const String& string, const String& div = String("\n"));
void StrTok(Array& result, const String& string, const String& div = String("\n"));

String GetArgv(int argc, char* argv[], const String& argument);
bool HaveArgv(int argc, char* argv[], const String& argument);

Array Sort(const Array& array, bool unique = false);
Array SortReverse(const Array& array, bool unique = false);
size_t rand(size_t min, size_t max);
double randf(double min, double max);
ByteArray Random(size_t bytes);
ByteArray& Random(ByteArray& buffer, size_t bytes);

// Speicherzugriff
void Poke8(void* Adresse, uint8_t Wert);
void Poke16(void* Adresse, uint16_t Wert);
void Poke24(void* Adresse, uint32_t Wert);
void Poke32(void* Adresse, uint32_t Wert);
void Poke64(void* Adresse, uint64_t Wert);
void PokeFloat(void* Adresse, float Wert);
uint8_t Peek8(const void* Adresse);
uint16_t Peek16(const void* Adresse);
uint32_t Peek24(const void* Adresse);
uint32_t Peek32(const void* Adresse);
uint64_t Peek64(const void* Adresse);
float PeekFloat(const void* Adresse);

// Network-Byte-Order
void PokeN8(void* Adresse, uint8_t Wert);
void PokeN16(void* Adresse, uint16_t Wert);
void PokeN24(void* Adresse, uint32_t Wert);
void PokeN32(void* Adresse, uint32_t Wert);
void PokeN64(void* Adresse, uint64_t Wert);

uint8_t PeekN8(const void* Adresse);
uint16_t PeekN16(const void* Adresse);
uint32_t PeekN24(const void* Adresse);
uint32_t PeekN32(const void* Adresse);
uint64_t PeekN64(const void* Adresse);

// cpu.cpp
namespace CPUCAPS
{
enum
{
    CPU_NONE = 0x00000000,
    CPU_HAVE_CPUID = 0x00000001,
    CPU_HAVE_MMX = 0x00000002,
    CPU_HAVE_MMXExt = 0x00000004,
    CPU_HAVE_3DNow = 0x00000008,
    CPU_HAVE_3DNowExt = 0x00000010,
    CPU_HAVE_SSE = 0x00000020,
    CPU_HAVE_SSE2 = 0x00000040,
    CPU_HAVE_AMD64 = 0x00000080,
    CPU_HAVE_SSE3 = 0x00000100,
    CPU_HAVE_SSSE3 = 0x00000200,
    CPU_HAVE_SSE4a = 0x00000400,
    CPU_HAVE_SSE41 = 0x00000800,
    CPU_HAVE_SSE42 = 0x00001000,
    CPU_HAVE_AES = 0x00002000,
    CPU_HAVE_AVX = 0x00004000,
    CPU_HAVE_AVX2 = 0x00008000,
    CPU_HAVE_AVX512 = 0x00010000,
    CPU_HAVE_SHA = 0x00020000,
};
} // namespace CPUCAPS

typedef struct
{
    uint32_t caps; // Struktur kann um weitere Informationen erweitert werden
    uint32_t bits;

} CPUCaps;

uint32_t GetCPUCaps(CPUCaps& cpu);
uint32_t GetCPUCaps();

// Time
ppl_time_t GetTime(PPLTIME* t);
ppl_time_t GetTime(PPLTIME* t, ppl_time_t tt);
ppl_time_t GetTime(PPLTIME& t, ppl_time_t tt);
ppl_time_t GetTime();
int USleep(uint64_t microseconds); // 1 sec = 1000000 microseconds
int MSleep(uint64_t milliseconds); // 1 sec = 1000 milliseconds
int SSleep(uint64_t seconds);
double GetMicrotime();
uint64_t GetMilliSeconds();

ppl_time_t MkTime(
    const String& year, const String& month, const String& day, const String& hour = "0", const String& min = "0", const String& sec = "0");
ppl_time_t MkTime(int year, int month, int day, int hour = 0, int min = 0, int sec = 0);
ppl_time_t MkTime(const String& iso8601date, PPLTIME* t = NULL);
ppl_time_t MkTime(const PPLTIME& t);

String MkISO8601Date(ppl_time_t sec = 0);
String MkISO8601Date(const PPLTIME& t);
String MkRFC822Date(ppl_time_t sec = 0);
String MkRFC822Date(const PPLTIME& t);
String MkDate(const String& format, ppl_time_t sec);
String MkDate(const String& format, const PPLTIME& t);

}; // namespace ppl7

#endif // PPL7_FUNCTIONS_H_
