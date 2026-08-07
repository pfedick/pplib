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

#ifndef PPLIB_CORE_COMPRESSION_H_
#define PPLIB_CORE_COMPRESSION_H_

#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>

namespace pplib
{

/*!\class Compression
 * \ingroup PPLIB_COMPRESSION
 * \brief Komprimierung und Dekomprimierung von Daten
 *
 * Mit dieser Klasse können Daten komprimiert und dekomprimiert werden. Zur Zeit werden zwei
 * verschiedene Komprimierungsmethoden unterstüzt:
 * - ZLib (siehe http://www.zlib.net/)
 * - BZip2 (siehe http://www.bzip.org/)
 *
 * Um die gewünschte Methode auszuwählen, muss diese entweder im Konstruktor übergeben werden, oder durch
 * Aufruf von Compression::Init, was den Vorteil hat, das man hier auch gleich einen Fehlercode
 * gemeldet bekommt, wenn die gewünschte Methode nicht einkompiliert ist.
 *
 * Anschließend können durch Aufrufe von Compress und Uncompress Daten komprimiert bzw. entpackt
 * werden.
 *
 * \section Compression_Prefix Komprimierungsprefix
 *
 * Über die Funktion Compression::UsePrefix kann eingestellt werden, ob bei der Komprimierung noch ein
 * Header vorangestellt werden soll oder nicht. Der Header hat den Vorteil, dass man ihm die Komprimierungs-
 * Methode und die Länge der ursprünglichen unkomprimierten Daten entnehmen kann. Nicht alle Variationen
 * von Compress und Uncompress unterstützen den Prefix, daher ist bei der jeweiligen Funktion vermerkt,
 * ob der Prefix beachtet wird oder nicht.
 *
 * Es gibt zwei Versionen des Headers:
 *
 * \par Version 1 Prefix
 * Bei Version 1 gibt es einen 9-Byte großen Header mit folgendem Aufbau:
 *
\verbatim
Byte 0: Kompressions-Flag (siehe oben)
        Bits 0-2: Kompressionsart
                  0=keine
                  1=Zlib
                  2=Bzip2
        Bits 3-7: unbenutzt, müssen 0 sein
Byte 1: Bytes Unkomprimiert (4 Byte)
Byte 5: Bytes Komprimiert (4 Byte)
\endverbatim
 * Der erste Wert gibt an, wieviele Bytes der Datenblock unkomprimiert benötigt, der zweite gibt an,
 * wie gross er komprimiert ist. Nach dem Header folgen dann soviele Bytes, wie in "Bytes Komprimiert"
 * angegeben ist.
 *
 * \par Version 2 Prefix
 * Die Länge des Version 2 Headers ist variabel. Er beginnt wieder mit dem Kompressionsflag, diesmal
 * ist jedoch Bit 3 gesetzt und die Bits 4-7 werden ebenfalls verwendet:
 *
\verbatim
Byte 0: Kompression-Flag
        Bits 0-2: Kompressionsart
                  0=keine
                  1=Zlib
                  2=Bzip2
        Bit 3:    Headerversion
        Bits 4-5: Bytezahl Uncompressed Value
                  0=1 Byte, 1=2 Byte, 2=3 Byte, 3=4 Byte
        Bits 6-7: Bytezahl Compressed Value
                  0=1 Byte, 1=2 Byte, 2=3 Byte, 3=4 Byte
Byte 1: Bytes Unkomprimiert (1-4 Byte)
Byte n: Bytes Komprimiert (1-4 Byte)
\endverbatim
 * Bei Version 2 folgen eine variable Anzahl von Bytes für die beiden Werte "Bytes Unkomprimiert" und
 * "Bytes Komprimiert". Wieviele Bytes das sind, ist jeweils den Bits 4-5 und 6-7 des
 * Kompressions-Flags zu entnehmen. Bei kleinen Datenblöcken, die unkomprimiert weniger als 255 Bytes
 * benötigen, schrumpft der Prefix somit von 9 auf 3 Byte im Vergleich zum Version 1 Prefix.
 *
 */
class Compression
{
public:
    /*!\enum Algorithm
     * \brief Unterstütze Komprimierungsmethoden
     *
     * Die Klasse unterstützt folgende Komprimierungsmethoden:
     */
    enum Algorithm
    {
        /**@brief Compression::Algorithm Compression::Algo_NONE
         * Keine Komprimierung. Bei Verwendung dieser Methode werden die Daten einfach nur unverändert kopiert.
         */
        Algo_NONE = 0,

        /**@brief Compression::Algorithm Compression::Algo_ZLIB
         * Zlib ist eine freie Programmbibliothek von Jean-Loup Gailly und Mark Adler (http://www.zlib.net/).
         * Sie verwendet wie gzip den Deflate-Algorithmus um den Datenstrom blockweise zu komprimieren.
         * Die ausgegebenen Blöcke werden durch Adler-32-Prüfsummen geschützt.
         * Das Format ist in den RFC 1950, RFC 1951 und RFC 1952 definiert und gilt quasi als defakto
         * Standard im Unix- und Netzwerkbereich.
         */
        Algo_ZLIB,

        /**@brief Compression::Algorithm Compression::Algo_BZIP2
         * bzip2 ist ein frei verfügbares Komprimierungsprogramm zur verlustfreien Kompression
         * von Dateien, entwickelt von Julian Seward. Es ist frei von jeglichen patentierten
         * Algorithmen und wird unter einer BSD-ähnlichen Lizenz vertrieben.
         * Die Kompression mit bzip2 ist oft effizienter, aber meist erheblich langsamer als
         * die Kompression mit Zlib.
         */
        Algo_BZIP2,

        Unknown = 256 /// Wird als Defaulteinstellung beim Dekomprimieren verwendet und hat keine eigentliche Funktion.
    };

    /*!\enum Compression::Level
     * \brief Kompressionsrate
     *
     * Es werden verschiedene Einstellungen unterstützt, die Einfluß auf die Kompressionsrate
     * aber auch Speicherverbrauch und Geschwindigkeit haben:
     */
    enum Level
    {
        Level_Fast = 0,
        Level_Normal,
        Level_Default,
        Level_High
    };

    enum Prefix
    {
        Prefix_None = 0,
        Prefix_V1,
        Prefix_V2,
    };

private:
    void* buffer;
    void* uncbuffer;
    Algorithm aaa;
    Level lll;
    Prefix prefix;

    void doNone(void* dst, size_t* dstlen, const void* src, size_t size);
    void doZlib(void* dst, size_t* dstlen, const void* src, size_t size);
    void doBzip2(void* dst, size_t* dstlen, const void* src, size_t size);

    void unNone(void* dst, size_t* dstlen, const void* src, size_t srclen);
    void unZlib(void* dst, size_t* dstlen, const void* src, size_t srclen);
    void unBzip2(void* dst, size_t* dstlen, const void* src, size_t srclen);

public:
    Compression();
    Compression(Algorithm method, Level level = Level_Default);
    ~Compression();
    void init(Algorithm method, Level level = Level_Default);
    void usePrefix(Prefix prefix);

    void compress(void* dst, size_t* dstlen, const void* src, size_t size, Algorithm a = Unknown);
    void compress(ByteArray& out, const void* ptr, size_t size);
    void compress(ByteArray& out, const ByteArrayPtr& in);
    ByteArrayPtr compress(const void* ptr, size_t size);
    ByteArrayPtr compress(const ByteArrayPtr& in);

    void uncompress(void* dst, size_t* dstlen, const void* src, size_t srclen, Algorithm a = Unknown);
    void uncompress(ByteArray& out, const ByteArrayPtr& data);
    void uncompress(ByteArray& out, const void* data, size_t size = 0);
    ByteArrayPtr uncompress(const void* ptr, size_t size);
    ByteArrayPtr uncompress(const ByteArrayPtr& in);
};

void Compress(ByteArray& out, const ByteArrayPtr& in, Compression::Algorithm method, Compression::Level level = Compression::Level_Default);
void CompressZlib(ByteArray& out, const ByteArrayPtr& in, Compression::Level level = Compression::Level_Default);
void CompressBZip2(ByteArray& out, const ByteArrayPtr& in, Compression::Level level = Compression::Level_Default);
void Uncompress(ByteArray& out, const ByteArrayPtr& in);

} // namespace pplib

#endif // PPLIB_CORE_COMPRESSION_H_
