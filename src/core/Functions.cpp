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
#include <stdarg.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <errno.h>
#include <limits.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // Keine MFCs
#include <windows.h>
#endif

#include <pplib/core/functions.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/bytearrayptr.h>

namespace pplib
{

void PrintDebug(const char* format, ...)
{
    if (!format) return;
    va_list args;
    va_start(args, format);
    String buff;
    buff.vasprintf(format, args);
    va_end(args);
    buff.print();
    fflush(stdout);
}

void PrintDebugTime(const char* format, ...)
{
    if (!format) return;
    va_list args;
    va_start(args, format);
    String buff;
    buff.vasprintf(format, args);
    va_end(args);
    DateTime now;
    now.setCurrentTime();
    String Time = now.getISO8601withMsec();
    Time += ": ";

    printf("%s%s", (const char*)Time, (const char*)buff);
    fflush(stdout);
}

void HexDump(const ByteArrayPtr& data, bool skipheader)
{
    HexDump(data.ptr(), data.size(), skipheader);
}

void HexDump(const void* address, size_t bytes, bool skipheader)
{
    char buff[1024], tmp[10], cleartext[20];
    if (!skipheader) {
        printf("HEXDUMP: %zu Bytes starting at Address %p:\n", bytes, address);
    }

    const char* _adresse = (const char*)address;
    const char* start_adr = _adresse;
    int spalte = 0;
    // sprintf (buff,"%p: ",_adresse);
    buff[0] = 0;
    memset(cleartext, 0, 20);
    for (size_t i = 0; i < bytes; i++) {
        sprintf(tmp, "%02X ", (uint8_t)_adresse[i]);
        strcat(buff, tmp);
        if ((uint8_t)_adresse[i] > 31 && (uint8_t)_adresse[i] < 128)
            cleartext[spalte] = (uint8_t)_adresse[i];
        else
            cleartext[spalte] = '.';
        spalte++;
        if (spalte > 15) {
            buff[16 * 3 - 1] = 0;
            printf("%p: %s: %s\n", start_adr, buff, cleartext);
            buff[0] = 0;
            memset(cleartext, 0, 20);
            spalte = 0;
            start_adr = _adresse + i + 1;
        }
    }

    if (spalte > 0) {
        strcat(buff, "                                                               ");
        buff[16 * 3 - 1] = 0;
        printf("%p: %s: %s\n", start_adr, buff, cleartext);
    }
    if (!skipheader) printf("\n");
}

/*!\defgroup PPLGroupPeekPoke Peek und Poke
 * \brief Funktionen zum Zugriff auf den Speicher
 * \ingroup PPLGroupMemory
 *
 * \desc
 * Bei den Peek- und Poke-Funktionen handelt es sich um Funktionen zum Platform-unabhängigem
 * Schreiben und Lesen von Werten im Hauptspeicher. Die Funktionsnamen sind eine Homage an den
 * <a href="http://http://de.wikipedia.org/wiki/Commodore_64">Commodore 64</a>,
 * zu dessen BASIC-Wortschatz Peek und Poke gehörten.
 * \par
 * Bei den Poke-Funktionen handelt es sich um Funktionen zum Schreiben in den Speicher im
 * Little-Endian-Format, was beispielsweise von Intel und AMD verwendet wird. Mit den Peek-Funktionen
 * kann der Wert wieder ausgelesen werden.
 * \par
 * Bei PokeN und PeekN handelt es sich um identische Funktionen, die die Werte jedoch im
 * Big-Endian-Format verarbeiten, was auf vielen RISC-CPUs zu finden ist, z.B. Motorola, Sparc und
 * PowerPC. Da derartige CPUs häufig in Netzwerk-Equipment zu finden ist (Routern), spricht man
 * auch von "Network-Byte-Order", woher auch das "N" im Namen der Funktionen kommt.
 * \par
 * Mehr Informationen zu Big- und Little-Endian sind in der Wikipedia zu finden:
 * http://de.wikipedia.org/wiki/Byte-Reihenfolge
 * \par Verwendung
 * In der Regel wird man mit den durch die Sprache C/C++ bereitgestellten Mitteln
 * auf den Speicher zugreifen, da dies die optimalste und performanteste Art und Weise ist,
 * und man sich um die Architektur des Rechners keine Gedanken machen muss.
 * \par
 * Aber immer dann, wenn man binäre Daten mit anderen Rechnern austauschen muss, deren
 * Architektur man nicht kenn, empfiehlt es sich ein einheitliches Speicherformat zu verwenden.
 * So verwendet beispielsweise die Klasse AssocArray bei ihren Im- und Export-Funktionen
 * PeekN und PokeN zur Darstellung aller Integer-Werte.
 *
 */

void Poke8(void* Adresse, uint8_t Wert)
{
    ((uint8_t*)Adresse)[0] = Wert;
}

void Poke16(void* Adresse, uint16_t Wert)
{
    ((uint8_t*)Adresse)[0] = (uint8_t)(Wert & 255);
    ((uint8_t*)Adresse)[1] = (uint8_t)((Wert >> 8) & 255);
}

void Poke24(void* Adresse, uint32_t Wert)
{
    ((uint8_t*)Adresse)[0] = (uint8_t)(Wert & 255);
    ((uint8_t*)Adresse)[1] = (uint8_t)((Wert >> 8) & 255);
    ((uint8_t*)Adresse)[2] = (uint8_t)((Wert >> 16) & 255);
}

void Poke32(void* Adresse, uint32_t Wert)
{
    ((uint8_t*)Adresse)[0] = (uint8_t)(Wert & 255);
    ((uint8_t*)Adresse)[1] = (uint8_t)((Wert >> 8) & 255);
    ((uint8_t*)Adresse)[2] = (uint8_t)((Wert >> 16) & 255);
    ((uint8_t*)Adresse)[3] = (uint8_t)((Wert >> 24) & 255);
}

void Poke64(void* Adresse, uint64_t Wert)
{
    ((uint8_t*)Adresse)[0] = (uint8_t)(Wert & 255);
    ((uint8_t*)Adresse)[1] = (uint8_t)((Wert >> 8) & 255);
    ((uint8_t*)Adresse)[2] = (uint8_t)((Wert >> 16) & 255);
    ((uint8_t*)Adresse)[3] = (uint8_t)((Wert >> 24) & 255);
    ((uint8_t*)Adresse)[4] = (uint8_t)((Wert >> 32) & 255);
    ((uint8_t*)Adresse)[5] = (uint8_t)((Wert >> 40) & 255);
    ((uint8_t*)Adresse)[6] = (uint8_t)((Wert >> 48) & 255);
    ((uint8_t*)Adresse)[7] = (uint8_t)((Wert >> 56) & 255);
}

void PokeFloat(void* Adresse, float Wert)
{
    // Immer Little-Endian ablegen, Byte für Byte
    uint8_t* dst = (uint8_t*)Adresse;
    uint8_t* src = (uint8_t*)&Wert;
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
}

uint8_t Peek8(const void* Adresse)
{
    return (uint32_t)((uint8_t*)Adresse)[0];
}

uint16_t Peek16(const void* Adresse)
{
    uint8_t wert1 = ((uint8_t*)Adresse)[0];
    uint8_t wert2 = ((uint8_t*)Adresse)[1];
    return ((uint16_t)wert1 | ((uint16_t)wert2 << 8));
}

uint32_t Peek24(const void* Adresse)
{
    uint8_t wert1, wert2, wert3;
    wert1 = ((uint8_t*)Adresse)[0];
    wert2 = ((uint8_t*)Adresse)[1];
    wert3 = ((uint8_t*)Adresse)[2];
    return ((uint32_t)wert1 | (wert2 << 8) | (wert3 << 16));
}

uint32_t Peek32(const void* Adresse)
{
    uint8_t wert1, wert2, wert3, wert4;
    wert1 = ((uint8_t*)Adresse)[0];
    wert2 = ((uint8_t*)Adresse)[1];
    wert3 = ((uint8_t*)Adresse)[2];
    wert4 = ((uint8_t*)Adresse)[3];

    return ((uint32_t)(uint32_t)wert1 | ((uint32_t)wert2 << 8) | ((uint32_t)wert3 << 16) | ((uint32_t)wert4 << 24));
}

uint64_t Peek64(const void* Adresse)
{
    uint8_t wert1, wert2, wert3, wert4, wert5, wert6, wert7, wert8;
    wert1 = ((uint8_t*)Adresse)[0];
    wert2 = ((uint8_t*)Adresse)[1];
    wert3 = ((uint8_t*)Adresse)[2];
    wert4 = ((uint8_t*)Adresse)[3];
    wert5 = ((uint8_t*)Adresse)[4];
    wert6 = ((uint8_t*)Adresse)[5];
    wert7 = ((uint8_t*)Adresse)[6];
    wert8 = ((uint8_t*)Adresse)[7];

    return ((uint64_t)(uint64_t)wert1 | ((uint64_t)wert2 << 8) | ((uint64_t)wert3 << 16) | ((uint64_t)wert4 << 24) |
            ((uint64_t)wert5 << 32) | ((uint64_t)wert6 << 40) | ((uint64_t)wert7 << 48) | ((uint64_t)wert8 << 56));
}

float PeekFloat(const void* Adresse)
{
    // Immer aus Little-Endian zusammensetzen
    float Wert;
    uint8_t* dst = (uint8_t*)&Wert;
    const uint8_t* src = (const uint8_t*)Adresse;
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    return Wert;
}

void PokeN8(void* Adresse, uint8_t Wert)
{
    ((uint8_t*)Adresse)[0] = (uint8_t)(Wert & 255);
}

void PokeN16(void* Adresse, uint16_t Wert)
{
    ((uint8_t*)Adresse)[1] = (uint8_t)(Wert & 255);
    ((uint8_t*)Adresse)[0] = (uint8_t)((Wert >> 8) & 255);
}

void PokeN24(void* Adresse, uint32_t Wert)
{
    // Immer als Big-Endian (Network Order) ablegen: [0]=MSB ... [2]=LSB
    ((uint8_t*)Adresse)[0] = (uint8_t)((Wert >> 16) & 255);
    ((uint8_t*)Adresse)[1] = (uint8_t)((Wert >> 8) & 255);
    ((uint8_t*)Adresse)[2] = (uint8_t)(Wert & 255);
}

void PokeN32(void* Adresse, uint32_t Wert)
{
    ((uint8_t*)Adresse)[0] = (uint8_t)((Wert >> 24) & 255);
    ((uint8_t*)Adresse)[1] = (uint8_t)((Wert >> 16) & 255);
    ((uint8_t*)Adresse)[2] = (uint8_t)((Wert >> 8) & 255);
    ((uint8_t*)Adresse)[3] = (uint8_t)(Wert & 255);
}

void PokeN64(void* Adresse, uint64_t Wert)
{
    ((uint8_t*)Adresse)[0] = (uint8_t)((Wert >> 56) & 255);
    ((uint8_t*)Adresse)[1] = (uint8_t)((Wert >> 48) & 255);
    ((uint8_t*)Adresse)[2] = (uint8_t)((Wert >> 40) & 255);
    ((uint8_t*)Adresse)[3] = (uint8_t)((Wert >> 32) & 255);
    ((uint8_t*)Adresse)[4] = (uint8_t)((Wert >> 24) & 255);
    ((uint8_t*)Adresse)[5] = (uint8_t)((Wert >> 16) & 255);
    ((uint8_t*)Adresse)[6] = (uint8_t)((Wert >> 8) & 255);
    ((uint8_t*)Adresse)[7] = (uint8_t)(Wert & 255);
}

uint8_t PeekN8(const void* Adresse)
{
    return (uint8_t)((uint8_t*)Adresse)[0];
}

uint16_t PeekN16(const void* Adresse)
{
    uint8_t wert1, wert2;
    wert1 = ((uint8_t*)Adresse)[1];
    wert2 = ((uint8_t*)Adresse)[0];
    return ((uint16_t)(uint16_t)wert1 | ((uint16_t)wert2 << 8));
}

uint32_t PeekN24(const void* Adresse)
{
    uint8_t msb = ((uint8_t*)Adresse)[0];
    uint8_t mid = ((uint8_t*)Adresse)[1];
    uint8_t lsb = ((uint8_t*)Adresse)[2];
    return ((uint32_t)msb << 16) | ((uint32_t)mid << 8) | (uint32_t)lsb;
}

uint32_t PeekN32(const void* Adresse)
{
    uint8_t wert1, wert2, wert3, wert4;
    wert1 = ((uint8_t*)Adresse)[3];
    wert2 = ((uint8_t*)Adresse)[2];
    wert3 = ((uint8_t*)Adresse)[1];
    wert4 = ((uint8_t*)Adresse)[0];
    return ((uint32_t)(uint32_t)wert1 | ((uint32_t)wert2 << 8) | ((uint32_t)wert3 << 16) | ((uint32_t)wert4 << 24));
}

uint64_t PeekN64(const void* Adresse)
{
    uint8_t wert1, wert2, wert3, wert4, wert5, wert6, wert7, wert8;
    wert1 = ((uint8_t*)Adresse)[7];
    wert2 = ((uint8_t*)Adresse)[6];
    wert3 = ((uint8_t*)Adresse)[5];
    wert4 = ((uint8_t*)Adresse)[4];
    wert5 = ((uint8_t*)Adresse)[3];
    wert6 = ((uint8_t*)Adresse)[2];
    wert7 = ((uint8_t*)Adresse)[1];
    wert8 = ((uint8_t*)Adresse)[0];
    return ((uint64_t)(uint64_t)wert1 | ((uint64_t)wert2 << 8) | ((uint64_t)wert3 << 16) | ((uint64_t)wert4 << 24) |
            ((uint64_t)wert5 << 32) | ((uint64_t)wert6 << 40) | ((uint64_t)wert7 << 48) | ((uint64_t)wert8 << 56));
}

String GetArgv(int argc, char* argv[], const String& argument)
{
    if (argc > 1) {
        size_t argl = strlen(argument);
        for (int i = 1; i < argc; i++) {
            if (strncmp(argv[i], argument, argl) == 0) {
                size_t l = strlen(argv[i]);
                if (l > argl || argv[i + 1] == NULL) {
                    const char* ret = (argv[i] + argl);
                    // if (ret[0]=='-') return (char*)"";
                    // if (ret[0]=='\\' && ret[1]=='-') return ret+1;
                    return String(ret);
                } else {
                    const char* ret = (argv[i + 1]);
                    if (ret[0] == '-') return String();
                    if (ret[0] == '\\' && ret[1] == '-') return ret + 1;
                    return String(ret);
                }
            }
        }
    }
    return String();
}

bool HaveArgv(int argc, char* argv[], const String& argument)
{
    if (argc > 1) {
        size_t argl = strlen(argument);
        for (int i = 1; i < argc; i++) {
            if (strncmp(argv[i], argument, argl) == 0) {
                return true;
            }
        }
    }
    return false;
}

} // namespace pplib
