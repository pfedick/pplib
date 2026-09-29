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

#ifndef PPLIB_CORE_FUNCTIONS_H_
#define PPLIB_CORE_FUNCTIONS_H_

#include <stdint.h>
#include <pplib/types/string.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/types/datetime.h>
#include <pplib/types/assocarray.h>
#include <pplib/core/random.h>
#include <pplib/core/args.h>
#include <pplib/core/stringfunctions.h>

namespace pplib
{

/** @brief Interne Funktion zur Ausgabe von Text
 *
 * Diese Funktion dient als Ersatz für "printf" und wird intern von einigen Funktionen/Klasse zur
 * Ausgabe von Text verwendet. Über die Funktion SetGlobalOutput kann bestimmt werden, ob dieser
 * Text per STDOUT auf die Konsole ausgegeben werden soll oder beispielsweise im Debugger von
 * VisualStudio unter Windows.
 *
 * \param[in] format Formatstring für den Text
 * \param[in] ...    Optionale Parameter, die im Formatstring eingesetzt werden sollen
 */
void PrintDebug(const char* format, ...);

/** @brief Interne Funktion zur Ausgabe von Text
 *
 * Diese Funktion dient als Ersatz für "printf" und wird intern von einigen Funktionen/Klasse zur
 * Ausgabe von Text verwendet. Über die Funktion SetGlobalOutput kann bestimmt werden, ob dieser
 * Text per STDOUT auf die Konsole ausgegeben werden soll oder beispielsweise im Debugger von
 * VisualStudio unter Windows.
 *
 * \param[in] format Formatstring für den Text
 * \param[in] ...    Optionale Parameter, die im Formatstring eingesetzt werden sollen
 */
void PrintDebugTime(const char* format, ...);

/** @brief Speicherinhalt in hexadezimaler Form ausgeben
 * @ingroup PPLGroupPeekPoke
 *
 * Gibt den Inhalt des Speichers ab der angegebenen Adresse für die angegebene Anzahl von Bytes
 * in hexadezimaler Form aus. Optional kann die Kopfzeile übersprungen werden.
 *
 * @param address Speicheradresse, deren Inhalt ausgegeben werden soll
 * @param bytes Anzahl der auszugebenden Bytes
 * @param skipheader Wenn true, wird die Kopfzeile übersprungen (Default=false)
 */
void HexDump(const void* address, size_t bytes, bool skipheader = false);

/** @brief Speicherinhalt in hexadezimaler Form ausgeben
 * @ingroup PPLGroupPeekPoke
 *
 * Gibt den Inhalt des Speichers ab der angegebenen Adresse für die angegebene Anzahl von Bytes
 * in hexadezimaler Form aus. Optional kann die Kopfzeile übersprungen werden.
 *
 * @param data ByteArrayPtr, dessen Inhalt ausgegeben werden soll
 * @param skipheader Wenn true, wird die Kopfzeile übersprungen (Default=false)
 */
void HexDump(const ByteArrayPtr& data, bool skipheader = false);

// Speicherzugriff

/** @brief 8-Bit-Wert schreiben
 * @ingroup PPLGroupPeekPoke
 *
 * Die ersten 8 Bit des Wertes werden in die angegebene Speicheradresse
 * im Little-Endian-Format geschrieben. Es spielt keine Rolle, ob die CPU des
 * Rechners mit Little- oder Big-Endian arbeitet.
 *
 * @param Adresse Speicheradresse, in die geschrieben werden soll
 * @param Wert Wert, der gespeichert werden soll
 * @see Beschreibung von \ref PPLGroupPeekPoke
 */
void Poke8(void* Adresse, uint8_t Wert);

/** @brief 16-Bit-Wert schreiben im Little-Endian-Format schreiben
 * @ingroup PPLGroupPeekPoke
 *
 * Die ersten 16 Bit des Wertes werden in die angegebene Speicheradresse
 * im Little-Endian-Format geschrieben. Es spielt keine Rolle, ob die CPU des
 * Rechners mit Little- oder Big-Endian arbeitet.
 *
 * @param Adresse Speicheradresse, in die geschrieben werden soll
 * @param Wert Wert, der gespeichert werden soll
 * @see Beschreibung von \ref PPLGroupPeekPoke
 */
void Poke16(void* Adresse, uint16_t Wert);

/** @brief 24-Bit-Wert schreiben im Little-Endian-Format schreiben
 * \ingroup PPLGroupPeekPoke
 *
 * \desc
 * Die ersten 24 Bit des Wertes werden in die angegebene Speicheradresse
 * im Little-Endian-Format geschrieben. Es spielt keine Rolle, ob die CPU des
 * Rechners mit Little- oder Big-Endian arbeitet.
 *
 * @param Adresse Speicheradresse, in die geschrieben werden soll
 * @param Wert Wert, der gespeichert werden soll
 * @see Beschreibung von \ref PPLGroupPeekPoke
 */
void Poke24(void* Adresse, uint32_t Wert);

/** @brief 32-Bit-Wert schreiben im Little-Endian-Format schreiben
 * @ingroup PPLGroupPeekPoke
 *
 * Die ersten 32 Bit des Wertes werden in die angegebene Speicheradresse
 * im Little-Endian-Format geschrieben. Es spielt keine Rolle, ob die CPU des
 * Rechners mit Little- oder Big-Endian arbeitet.
 *
 * @param Adresse Speicheradresse, in die geschrieben werden soll
 * @param Wert Wert, der gespeichert werden soll
 * @see Beschreibung von \ref PPLGroupPeekPoke
 */
void Poke32(void* Adresse, uint32_t Wert);

/** @brief 64-Bit-Wert schreiben im Little-Endian-Format schreiben
 * @ingroup PPLGroupPeekPoke
 *
 * Die ersten 64 Bit des Wertes werden in die angegebene Speicheradresse
 * im Little-Endian-Format geschrieben. Es spielt keine Rolle, ob die CPU des
 * Rechners mit Little- oder Big-Endian arbeitet.
 *
 * @param Adresse Speicheradresse, in die geschrieben werden soll
 * @param Wert Wert, der gespeichert werden soll
 * @see Beschreibung von \ref PPLGroupPeekPoke
 */
void Poke64(void* Adresse, uint64_t Wert);

/** @brief 32-Bit-Float-Wert schreiben
 * @ingroup PPLGroupPeekPoke
 *
 * Die Inhalt des Floats \p Wert wird in die angegebene Speicheradresse geschrieben.
 *
 * @param Adresse Speicheradresse, in die geschrieben werden soll
 * @param Wert Wert, der gespeichert werden soll
 * @see Beschreibung von \ref PPLGroupPeekPoke
 */
void PokeFloat(void* Adresse, float Wert);

/** @brief 8-Bit-Wert auslesen
 * @ingroup PPLGroupPeekPoke
 *
 * Die ersten 8 Bit der angegebenen Adresse werden im Little-Endian-Format
 * ausgelesen und als Wert zurückgegeben
 *
 * @param Adresse Speicheradresse, aus der gelesen werden soll
 * @return Ausgelesener Wert
 * @see Beschreibung von \ref PPLGroupPeekPoke
 */
uint8_t Peek8(const void* Adresse);

/** @brief 16-Bit-Wert auslesen
 * @ingroup PPLGroupPeekPoke
 *
 * Die ersten 16 Bit der angegebenen Adresse werden im Little-Endian-Format
 * ausgelesen und als Wert zurückgegeben
 *
 * @param Adresse Speicheradresse, aus der gelesen werden soll
 * @return Ausgelesener Wert
 * @see Beschreibung von \ref PPLGroupPeekPoke
 */
uint16_t Peek16(const void* Adresse);

/** @brief 24-Bit-Wert auslesen
 * @ingroup PPLGroupPeekPoke
 *
 * Die ersten 24 Bit der angegebenen Adresse werden im Little-Endian-Format
 * ausgelesen und als Wert zurückgegeben
 *
 * @param Adresse Speicheradresse, aus der gelesen werden soll
 * @return Ausgelesener Wert
 * @see Beschreibung von \ref PPLGroupPeekPoke
 */
uint32_t Peek24(const void* Adresse);

/** @brief 32-Bit-Wert auslesen
 * @ingroup PPLGroupPeekPoke
 *
 * Die ersten 32 Bit der angegebenen Adresse werden im Little-Endian-Format
 * ausgelesen und als Wert zurückgegeben
 *
 * @param Adresse Speicheradresse, aus der gelesen werden soll
 * @return Ausgelesener Wert
 * @see Beschreibung von \ref PPLGroupPeekPoke
 */
uint32_t Peek32(const void* Adresse);

/** @brief 64-Bit-Wert auslesen
 * @ingroup PPLGroupPeekPoke
 *
 * Die ersten 64 Bit der angegebenen Adresse werden im Little-Endian-Format
 * ausgelesen und als Wert zurückgegeben
 *
 * @param Adresse Speicheradresse, aus der gelesen werden soll
 * @return Ausgelesener Wert
 * @see Beschreibung von \ref PPLGroupPeekPoke
 */
uint64_t Peek64(const void* Adresse);

/** @brief 32-Bit-Float-Wert auslesen
 * @ingroup PPLGroupPeekPoke
 *
 * Die ersten 32 Bit der angegebenen Adresse werden im Little-Endian-Format
 * ausgelesen und als Float-Wert zurückgegeben.
 *
 * @param Adresse Speicheradresse, aus der gelesen werden soll
 * @return Ausgelesener Wert
 * @see Beschreibung von \ref PPLGroupPeekPoke
 */
float PeekFloat(const void* Adresse);

// Network-Byte-Order

/** @brief 8-Bit-Wert in Network-Byteorder / Big-Endian-Format schreiben
 * @ingroup PPLGroupPeekPoke
 *
 * @param Adresse Speicheradresse, in die geschrieben werden soll
 * @param Wert Zu schreibender Wert
 */
void PokeN8(void* Adresse, uint8_t Wert);

/** @brief 16-Bit-Wert in Network-Byteorder / Big-Endian-Format schreiben
 * @ingroup PPLGroupPeekPoke
 *
 * @param Adresse Speicheradresse, in die geschrieben werden soll
 * @param Wert Zu schreibender Wert
 */
void PokeN16(void* Adresse, uint16_t Wert);

/** @brief 24-Bit-Wert in Network-Byteorder / Big-Endian-Format schreiben
 * @ingroup PPLGroupPeekPoke
 *
 * @param Adresse Speicheradresse, in die geschrieben werden soll
 * @param Wert Zu schreibender Wert
 */
void PokeN24(void* Adresse, uint32_t Wert);

/** @brief 32-Bit-Wert in Network-Byteorder / Big-Endian-Format schreiben
 * @ingroup PPLGroupPeekPoke
 *
 * @param Adresse Speicheradresse, in die geschrieben werden soll
 * @param Wert Zu schreibender Wert
 */
void PokeN32(void* Adresse, uint32_t Wert);

/** @brief 64-Bit-Wert in Network-Byteorder / Big-Endian-Format schreiben
 * @ingroup PPLGroupPeekPoke
 *
 * @param Adresse Speicheradresse, in die geschrieben werden soll
 * @param Wert Zu schreibender Wert
 */
void PokeN64(void* Adresse, uint64_t Wert);

/** @brief 8-Bit-Wert aus Network-Byteorder / Big-Endian-Format auslesen
 * @ingroup PPLGroupPeekPoke
 *
 * @param Adresse Speicheradresse, aus der gelesen werden soll
 * @return Ausgelesener Wert
 */
uint8_t PeekN8(const void* Adresse);

/** @brief 16-Bit-Wert aus Network-Byteorder / Big-Endian-Format auslesen
 * @ingroup PPLGroupPeekPoke
 *
 * @param Adresse Speicheradresse, aus der gelesen werden soll
 * @return Ausgelesener Wert
 */
uint16_t PeekN16(const void* Adresse);

/** @brief 24-Bit-Wert aus Network-Byteorder / Big-Endian-Format auslesen
 * @ingroup PPLGroupPeekPoke
 *
 * @param Adresse Speicheradresse, aus der gelesen werden soll
 * @return Ausgelesener Wert
 */
uint32_t PeekN24(const void* Adresse);

/** @brief 32-Bit-Wert aus Network-Byteorder / Big-Endian-Format auslesen
 * @ingroup PPLGroupPeekPoke
 *
 * @param Adresse Speicheradresse, aus der gelesen werden soll
 * @return Ausgelesener Wert
 */
uint32_t PeekN32(const void* Adresse);

/** @brief 64-Bit-Wert aus Network-Byteorder / Big-Endian-Format auslesen
 * @ingroup PPLGroupPeekPoke
 *
 * @param Adresse Speicheradresse, aus der gelesen werden soll
 * @return Ausgelesener Wert
 */
uint64_t PeekN64(const void* Adresse);

//********************************************************************************************* */

/**
 * @ingroup PPLGroupMath
 * @brief Berechnet den polynomischen CRC32-Wert eines Buffers
 *
 * Berechnet die zyklisch redundante polynomische Prüfsumme mit einer Länge von 32-Bit.
 *
 *
 * @param buffer Pointer auf den Beginn der Daten
 * @param size Länge der Daten in Byte
 * @param initial_crc Initialwert für die Berechnung. Standardwert ist 0xFFFFFFFF
 * \return Integer mit der Prüfsumme
 */
uint32_t Crc32(const void* buffer, size_t size, uint32_t initial_crc = 0xFFFFFFFF);

/**
 * @ingroup PPLGroupMath
 * @brief Berechnet den CRC-16-CCITT Wert eines Buffers
 *
 * Berechnet die zyklisch redundante polynomische Prüfsumme mit einer Länge von 16-Bit
 * (CCITT-Polynom 0x1021).
 *
 * @param buffer Pointer auf den Beginn der Daten
 * @param size Länge der Daten in Byte
 * @param initial_crc Initialwert (Standard 0xFFFF). Nützlich zum Fortsetzen einer Checksumme.
 * @return Integer mit der Prüfsumme
 */
uint16_t Crc16(const void* buffer, size_t size, uint16_t initial_crc = 0xFFFF);

String Md5(const void* buffer, size_t size);
String Md5(const ByteArrayPtr& buffer);
String Sha256(const void* buffer, size_t size);
String Sha256(const ByteArrayPtr& buffer);
double Calc(const String& expression);

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

class PerlHelper
{
public:
    static String escapeString(const String& s);
    static String escapeRegExp(const String& s);
    static String toHash(const AssocArray& a, const String& s);
};

class PythonHelper
{
public:
    static String escapeString(const String& s);
    static String escapeRegExp(const String& s);
    static String toHash(const AssocArray& a, const String& name, int indention = 0);
};

}; // namespace pplib

#endif // PPLIB_CORE_FUNCTIONS_H_
