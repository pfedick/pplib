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

/** @class Compression
 * @ingroup PPLIB_COMPRESSION
 * @brief Komprimierung und Dekomprimierung von Daten
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
 * @section Compression_Prefix Komprimierungsprefix
 *
 * Über die Funktion Compression::UsePrefix kann eingestellt werden, ob bei der Komprimierung noch ein
 * Header vorangestellt werden soll oder nicht. Der Header hat den Vorteil, dass man ihm die Komprimierungs-
 * Methode und die Länge der ursprünglichen unkomprimierten Daten entnehmen kann. Nicht alle Variationen
 * von Compress und Uncompress unterstützen den Prefix, daher ist bei der jeweiligen Funktion vermerkt,
 * ob der Prefix beachtet wird oder nicht.
 *
 * Es gibt zwei Versionen des Headers:
 *
 * @par Version 1 Prefix
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
    /** @enum Algorithm
     * @brief Unterstütze Komprimierungsmethoden
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

    /** @enum Compression::Level
     * @brief Kompressionsrate
     *
     * Es werden verschiedene Einstellungen unterstützt, die Einfluß auf die Kompressionsrate
     * aber auch Speicherverbrauch und Geschwindigkeit haben:
     */
    enum Level
    {
        Level_Fast = 0, ///< Schnellste Kompression, geringste Kompressionsrate
        Level_Normal,   ///< Normale Kompression, ausgewogenes Verhältnis zwischen Geschwindigkeit und Kompressionsrate
        Level_Default,  ///< Standardkompression, wird verwendet, wenn keine spezifische Einstellung gewählt wurde
        Level_High      ///< Höchste Kompression, langsamste Geschwindigkeit
    };

    /** @enum Compression::Prefix
     * @brief Prefix voranstellen
     *
     * Verwendung eines Prefix, der den komprimierten Daten vorangestellt wird.
     * Siehe dazu auch \ref Compression_Prefix
     */
    enum Prefix
    {
        /** @brief Kein Prefix voranstellen
         *
         * Die Anwendung muß sich selbst darum
         * kümmern, dass die Information über Größe der komprimierten und
         * unkomprimierten Daten erhalten bleibt.
         */
        Prefix_None = 0,

        /** @brief Version 1 Prefix voranstellen
         *
         * Es wird ein 9-Byte langer Version 1 Prefix vorangestellt.
         */
        Prefix_V1,

        /** @brief Version 2 Prefix voranstellen
         *
         * Es wird ein Version 2 Prefix mit variabler Länge vorangestellt.
         */
        Prefix_V2,
    };

private:
    void* buffer;    ///< Interner Speicher, der nach Aufruf von Compress die komprimierten Daten enthält
    void* uncbuffer; ///< Interner Speicher, der nach Aufruf von Uncompress die entpackten Daten enthält
    Algorithm aaa;   ///< Enthält die durch Init oder den Konstruktor eingestellten Kompressionsmethode
    Level lll;       ///< Enthält den durch Init oder den Konstruktor eingestellten Komprimierungslevel
    Prefix prefix;   ///< Flag, ob und welcher Prefix beim Komprimieren vorangestellt wird

    /** @brief Keine Komprimierung verwenden
     *
     * Diese interne Funktion wird aufgerufen, wenn die Daten garnicht komprimiert werden sollen.
     * Sie ruft daher nun memcpy auf, um die Quelldaten von @p src nach @p dst zu kopieren.
     *
     * @param[in,out] dst Pointer auf den Speicherbereich, in dem die komprimierten Daten abgelegt werden sollen
     * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs @p dst
     * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
     * @param[in] src Pointer auf den Speicherbereich, den komprimiert werden soll
     * @param[in] size Länge des zu komprimierenden Speicherbereichs
     * @exception BufferTooSmallException Der Puffer @p dst ist zu klein, um die komprimierten Daten aufzunehmen.
     * Der Parameter @p dstlen enthält nach Auftreten der Exception die tatsächlich benötigten Bytes.
     */
    void doNone(void* dst, size_t* dstlen, const void* src, size_t size);

    /** @brief Zlib-Komprimierung verwenden
     *
     * Mit dieser internen Funktion werden die Quelldaten aus @p src mit Zlib komprimiert und
     * in @p dst abgelegt.
     *
     * @param[in,out] dst Pointer auf den Speicherbereich, in dem die komprimierten Daten abgelegt werden sollen
     * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs @p dst
     * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
     * @param[in] src Pointer auf den Speicherbereich, den komprimiert werden soll
     * @param[in] size Länge des zu komprimierenden Speicherbereichs
     * @exception UnsupportedFeatureException Zlib wird nicht unterstützt
     * @exception OutOfMemoryException Nicht genug Speicher verfügbar
     * @exception BufferTooSmallException Der Puffer @p dst ist zu klein, um die komprimierten Daten aufzunehmen.
     * Der Parameter @p dstlen enthält nach Auftreten der Exception die tatsächlich benötigten Bytes.
     * @exception CompressionFailedException Ein unerwarteter Fehler ist aufgetreten, die Daten konnten nicht komprimiert werden
     */
    void doZlib(void* dst, size_t* dstlen, const void* src, size_t size);

    /** @brief Bzip2-Komprimierung verwenden
     *
     * Mit dieser internen Funktion werden die Quelldaten aus @p src mit BZip2 komprimiert und
     * in @p dst abgelegt.
     *
     * @param[in,out] dst Pointer auf den Speicherbereich, in dem die komprimierten Daten abgelegt werden sollen
     * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs @p dst
     * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
     * @param[in] src Pointer auf den Speicherbereich, den komprimiert werden soll
     * @param[in] size Länge des zu komprimierenden Speicherbereichs
     * @exception UnsupportedFeatureException Bzip2 wird nicht unterstützt
     * @exception OutOfMemoryException Nicht genug Speicher verfügbar
     * @exception BufferTooSmallException Der Puffer @p dst ist zu klein, um die komprimierten Daten aufzunehmen.
     * Der Parameter @p dstlen enthält nach Auftreten der Exception die tatsächlich benötigten Bytes.
     * @exception CompressionFailedException Ein unerwarteter Fehler ist aufgetreten, die Daten konnten nicht komprimiert werden
     */
    void doBzip2(void* dst, size_t* dstlen, const void* src, size_t size);

    /** @brief Speicherbereich ohne Dekompression kopieren
     *
     * Diese interne Funktion wird aufgerufen, wenn die zu dekomprimierenden Daten garnicht
     * komprimiert sind. Sie führt daher lediglich ein memcpy aus.
     *
     * @param[in,out] dst Pointer auf den Speicherbereich, in dem die dekomprimierten Daten abgelegt werden sollen
     * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs @p dst
     * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
     * @param[in] src Pointer auf den Anfang des Speicherbereichs, der die komprimierten Daten enthält
     * @param[in] srclen Länge der komprimierten Daten
     * @exception BufferTooSmallException Der Puffer @p dst ist zu klein, um die komprimierten Daten aufzunehmen.
     * Der Parameter @p dstlen enthält nach Auftreten der Exception die tatsächlich benötigten Bytes.
     */
    void unNone(void* dst, size_t* dstlen, const void* src, size_t srclen);

    /** @brief Zlib-Komprimierte Daten entpacken
     *
     * Diese interne Funktion wird aufgerufen, wenn die zu dekomprimierenden Daten mit Zlib
     * komprimiert sind.
     *
     * @param[in,out] dst Pointer auf den Speicherbereich, in dem die dekomprimierten Daten abgelegt werden sollen
     * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs @p dst
     * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
     * @param[in] src Pointer auf den Anfang des Speicherbereichs, der die komprimierten Daten enthält
     * @param[in] srclen Länge der komprimierten Daten
     * @exception UnsupportedFeatureException Zlib wird nicht unterstützt
     * @exception OutOfMemoryException Nicht genug Speicher verfügbar
     * @exception BufferTooSmallException Der Puffer @p dst ist zu klein, um die komprimierten Daten aufzunehmen.
     * Der Parameter @p dstlen enthält nach Auftreten der Exception die tatsächlich benötigten Bytes.
     * @exception CorruptedDataException Die zu dekomprimierenden Daten sind korrupt, unvollständig
     * oder nicht mit erwarteten Algorithmus komprimiert.
     * @exception DecompressionFailedException Ein unerwarteter Fehler ist aufgetreten, die Daten konnten nicht dekomprimiert werden
     */
    void unZlib(void* dst, size_t* dstlen, const void* src, size_t srclen);

    /** @brief Bzip2-Komprimierte Daten entpacken
     *
     * Diese interne Funktion wird aufgerufen, wenn die zu dekomprimierenden Daten mit Bzip2
     * komprimiert sind.
     *
     * @param[in,out] dst Pointer auf den Speicherbereich, in dem die dekomprimierten Daten abgelegt werden sollen
     * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs @p dst
     * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
     * @param[in] src Pointer auf den Anfang des Speicherbereichs, der die komprimierten Daten enthält
     * @param[in] srclen Länge der komprimierten Daten
     * @exception UnsupportedFeatureException Bzip2 wird nicht unterstützt
     * @exception OutOfMemoryException Nicht genug Speicher verfügbar
     * @exception BufferTooSmallException Der Puffer @p dst ist zu klein, um die komprimierten Daten aufzunehmen.
     * Der Parameter @p dstlen enthält nach Auftreten der Exception die tatsächlich benötigten Bytes.
     * @exception CorruptedDataException Die zu dekomprimierenden Daten sind korrupt, unvollständig
     * oder nicht mit erwarteten Algorithmus komprimiert.
     * @exception DecompressionFailedException Ein unerwarteter Fehler ist aufgetreten, die Daten konnten nicht dekomprimiert werden
     */
    void unBzip2(void* dst, size_t* dstlen, const void* src, size_t srclen);

public:
    /** @brief Konstruktor der Klasse
     *
     * Der parameterlose Konstruktor initialisiert die Klasse mit dem Zlib-Algorithmus und
     * dem Default-Level für die Komprimierungsrate.
     */
    Compression();

    /** @brief Konstruktor mit Initialisierung der Komprimierungsmethode
     *
     * Mit diesem Konstruktor kann gleichzeitig bestimmt werden, welche Komprimierungsmethode verwendet werden soll,
     * und wie stark die Komprimierung sein soll. Hier gilt: je höher die Komprimierung, desto langsamer.
     *
     * @param method Komprimierungsmethode (siehe Compression::Algorithm)
     * @param level Komprimierungslevel (siehe Compression::Level)
     */
    Compression(Algorithm method, Level level = Level_Default);

    /** @brief Destruktor der Klasse
     *
     * Der Destruktor sorgt dafür, dass intern allokierter Speicher freigegeben wird.
     * Falls Ergebnisse aus Compress oder Uncompress Aufrufen in einem CBinary-Objekt
     * gespeichert wurden, ohne "copy"-Flag, so ist der darin enthaltene Speicher
     * ebenfalls ungültig und darf nicht mehr verwendet werden.
     */
    ~Compression();

    /** @brief Gewünschte Komprimierungsmethode einstellen
     *
     * Mit dieser Funktion wird eingestellt, welche Komprimierungsmethode verwendet werden soll,
     * und wie stark die Komprimierung sein soll. Hier gilt: je höher die Komprimierung, desto langsamer.
     *
     * @param method Komprimierungsmethode (siehe Compression::Algorithm)
     * @param level Komprimierungslevel (siehe Compression::Level)
     * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0
     */
    void init(Algorithm method, Level level = Level_Default);

    /** @brief Verwendung eines Prefix beim Komprimieren
     *
     * Durch Aufruf dieser Funktion kann festgelegt werden, ob beim Komprimieren
     * den komprimierten Daten ein Prefix vorangestellt wird.
     *
     * Compression::Prefix
     *
     * @param prefix Der gewünschte Prefix
     *
     * @see Compression_Prefix
     */
    void usePrefix(Prefix prefix);

    /** @brief Komprimierung eines Speicherbereiches in einen anderen
     *
     * Mit dieser Version der Compress-Funktion wird ein Speicherbereich @p src mit einer Länge
     * von @p srclen Bytes komprimiert und das Ergebnis mit einer maximalen Länge von
     * @p dstlen Bytes ab der Speicherposition @p dst gespeichert. Der Zielspeicher @p dst
     * muss vorab allokiert worden sein und groß genug sein, um die komprimierten Daten aufzunehmen.
     * Wieviel Bytes tatsächlich verbraucht wurden, ist nach erfolgreichem Aufruf der Variablen
     * @p dstlen zu entnehmen.
     *
     * Diese Funktion führt nur die reine Komprimierung durch und unterstützt keinen Prefix.
     *
     * @param[in,out] dst Pointer auf den Speicherbereich, in dem die komprimierten Daten abgelegt werden sollen
     * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs @p dst
     * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
     * @param[in] src Pointer auf den Speicherbereich, den komprimiert werden soll
     * @param[in] srclen Länge des zu komprimierenden Speicherbereichs
     * @exception NullPointerException Einer der übergebenen Parameter (@p dst, @p dstlen oder @p src) zeigt auf NULL
     * @exception UnsupportedFeatureException Der eingestellte Komprimier-Algorithmus wird nicht unterstützt
     * @exception OutOfMemoryException Nicht genug Speicher verfügbar
     * @exception BufferTooSmallException Der Puffer @p dst ist zu klein, um die komprimierten Daten aufzunehmen.
     * Der Parameter @p dstlen enthält nach Auftreten der Exception die tatsächlich benötigten Bytes.
     * @exception CompressionFailedException Ein unerwarteter Fehler ist aufgetreten, die Daten konnten nicht komprimiert werden
     *
     * @note
     * Die Funktion prüft lediglich welche Komprimierungsmethode eingestellt wurde und ruft dann eine
     * der privaten Funktionen Compression::doNone, Compression::doZlib oder Compression::doBzip2 auf.
     */
    void compress(void* dst, size_t* dstlen, const void* src, size_t size, Algorithm a = Unknown);

    /** @brief Komprimierung eines Speicherbereiches in ein ByteArray Objekt
     *
     * Mit dieser Version der Compress-Funktion wird ein Speicherbereich @p ptr mit einer Länge
     * von @p size Bytes komprimiert und das Ergebnis im ByteArray-Objekt @p out gespeichert.
     * Der optionale Parameter @p copy bestimmt, ob in CBinary eine Kopie der komprimierten
     * Daten abgelegt wird oder nur ein Pointer auf den internen Buffer der Compression-Klasse.
     *
     * Diese Funktion unterstützt das Prefix-Flag (siehe Compression::usePrefix).
     *
     * @param[out] out ByteArray-Objekt, in dem die komprimierten Daten gespeichert werden sollen
     * @param[in] ptr Pointer auf den Speicherbereich, den komprimiert werden soll
     * @param[in] size Länge des zu komprimierenden Speicherbereichs
     * @exception UnsupportedFeatureException Der eingestellte Komprimier-Algorithmus wird nicht unterstützt
     * @exception OutOfMemoryException Nicht genug Speicher verfügbar
     * @exception BufferTooSmallException Der intern zum komprimieren verwendete Puffer
     * ist zu klein. Sollte dieser Fall auftreten, handelt es sich um einen Bug oder die
     * zu komprimierenden Daten lassen sich nicht komprimieren.
     * @exception CompressionFailedException Ein unerwarteter Fehler ist aufgetreten, die Daten konnten nicht komprimiert werden
     */
    void compress(ByteArray& out, const void* ptr, size_t size);

    /** @brief Komprimierung eines Speicherbereichs in ein CMemory-Objekt
     *
     * Mit dieser Version der Compress-Funktion wird der durch @p in referenzierte
     * Speicher komprimiert und das Ergebnis in @p out gespeichert.
     *
     * Diese Funktion unterstützt das Prefix-Flag (siehe Compression::UsePrefix).
     *
     * @param[out] out ByteArray-Objekt, in dem die komprimierten Daten gespeichert werden sollen
     * @param[in] in Ein von ByteArray oder ByteArrayPtr abgeleitetes Objekt, das den zu
     * komprimierenden Speicherbereich repräsentiert.
     * @exception UnsupportedFeatureException Der eingestellte Komprimier-Algorithmus wird nicht unterstützt
     * @exception OutOfMemoryException Nicht genug Speicher verfügbar
     * @exception BufferTooSmallException Der intern zum komprimieren verwendete Puffer
     * ist zu klein. Sollte dieser Fall auftreten, handelt es sich um einen Bug oder die
     * zu komprimierenden Daten lassen sich nicht komprimieren.
     * @exception CompressionFailedException Ein unerwarteter Fehler ist aufgetreten, die Daten konnten nicht komprimiert werden
     */
    void compress(ByteArray& out, const ByteArrayPtr& in);

    /** @brief Komprimierung eines Speicherbereiches
     *
     * Mit dieser Version der Compress-Funktion wird ein Speicherbereich @p ptr mit einer Länge
     * von @p size Bytes komprimiert und das Ergebnis als ByteArrayPtr-Objekt zurückgegeben.
     * Dieses enthält eine Referenz auf Speicherbereich der Compression-Klasse, die nur solange
     * gültig ist, wie die Compression-Klasse existiert und keine neue (De-)Komprimierung
     * durchgeführt wurde.
     *
     * Diese Funktion unterstützt das Prefix-Flag (siehe Compression::usePrefix).
     *
     * @param[in] ptr Pointer auf den Speicherbereich, den komprimiert werden soll
     * @param[in] size Länge des zu komprimierenden Speicherbereichs
     * @return Bei Erfolg wird ein ByteArrayPtr mit einer Referenz auf den komprimierten
     * Speicher zurückgegeben. Im Fehlerfall wird eine Exception geworfen.
     * @exception UnsupportedFeatureException Der eingestellte Komprimier-Algorithmus wird nicht unterstützt
     * @exception OutOfMemoryException Nicht genug Speicher verfügbar
     * @exception BufferTooSmallException Der intern zum komprimieren verwendete Puffer
     * ist zu klein. Sollte dieser Fall auftreten, handelt es sich um einen Bug oder die
     * zu komprimierenden Daten lassen sich nicht komprimieren.
     * @exception CompressionFailedException Ein unerwarteter Fehler ist aufgetreten, die Daten konnten nicht komprimiert werden
     */
    ByteArrayPtr compress(const void* ptr, size_t size);

    /** @brief Komprimierung eines Speicherbereiches
     *
     * Mit dieser Version der Compress-Funktion wird der von @p in referenzierte Speicherbereich
     * komprimiert und das Ergebnis als ByteArrayPtr-Objekt zurückgegeben.
     * Dieses enthält eine Referenz auf Speicherbereich der Compression-Klasse, die nur solange
     * gültig ist, wie die Compression-Klasse existiert und keine neue (De-)Komprimierung
     * durchgeführt wurde.
     *
     * Diese Funktion unterstützt das Prefix-Flag (siehe Compression::usePrefix).
     *
     * @param[in] ptr Pointer auf den Speicherbereich, den komprimiert werden soll
     * @param[in] size Länge des zu komprimierenden Speicherbereichs
     * @return Bei Erfolg wird ein ByteArrayPtr mit einer Referenz auf den komprimierten
     * Speicher zurückgegeben. Im Fehlerfall wird eine Exception geworfen.
     * @exception UnsupportedFeatureException Der eingestellte Komprimier-Algorithmus wird nicht unterstützt
     * @exception OutOfMemoryException Nicht genug Speicher verfügbar
     * @exception BufferTooSmallException Der intern zum komprimieren verwendete Puffer
     * ist zu klein. Sollte dieser Fall auftreten, handelt es sich um einen Bug oder die
     * zu komprimierenden Daten lassen sich nicht komprimieren.
     * @exception CompressionFailedException Ein unerwarteter Fehler ist aufgetreten, die Daten konnten nicht komprimiert werden
     */
    ByteArrayPtr compress(const ByteArrayPtr& in);

    /** @brief Dekomprimierung eines Speicherbereiches in einen anderen
     *
     * Mit dieser Version der Compress-Funktion wird ein komprimierter Speicherbereich @p src mit einer Länge
     * von @p srclen Bytes dekomprimiert und das entpackte Ergebnis mit einer maximalen Länge von
     * @p dstlen Bytes ab der Speicherposition @p dst gespeichert. Der Zielspeicher @p dst
     * muss vorab allokiert worden sein und groß genug sein, um die unkomprimierten Daten aufzunehmen.
     * Wieviel Bytes tatsächlich verbraucht wurden, ist nach erfolgreichem Aufruf der Variablen
     * @p dstlen zu entnehmen.
     *
     * Diese Funktion führt nur die reine Dekomprimierung durch und unterstützt keinen Prefix.
     *
     * @param[in,out] dst Pointer auf den Speicherbereich, in dem die dekomprimierten Daten abgelegt werden sollen
     * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs @p dst
     * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
     * @param[in] src Pointer auf den Anfang des Speicherbereichs, der die komprimierten Daten enthält
     * @param[in] srclen Länge der komprimierten Daten
     * @exception NullPointerException Einer der übergebenen Parameter (@p dst, @p dstlen oder @p src) zeigt auf NULL
     * @exception UnsupportedFeatureException Der eingestellte Komprimier-Algorithmus wird nicht unterstützt
     * @exception OutOfMemoryException Nicht genug Speicher verfügbar
     * @exception BufferTooSmallException Der Puffer @p dst ist zu klein, um die dekomprimierten Daten aufzunehmen.
     * Der Parameter @p dstlen enthält nach Auftreten der Exception die tatsächlich benötigten Bytes.
     * @exception DecompressionFailedException Ein unerwarteter Fehler ist aufgetreten, die Daten konnten nicht dekomprimiert werden
     *
     * @note
     * Die Funktion prüft lediglich welche Komprimierungsmethode eingestellt wurde und ruft dann eine
     * der privaten Funktionen Compression::unNone, Compression::unZlib oder Compression::unBzip2 auf.
     */
    void uncompress(void* dst, size_t* dstlen, const void* src, size_t srclen, Algorithm a = Unknown);

    /** @brief Dekomprimierung eines ByteArrayPtr Objektes
     *
     * Mit dieser Version der Compress-Funktion wird Speicher des Objektes @p object
     * entpackt und das Ergebnis im ByteArray-Objekt @p out gespeichert.
     *
     * Diese Funktion unterstützt das Prefix-Flag (siehe Compression::usePrefix).
     *
     * @param[out] out ByteArray-Objekt, in dem die entpackten Daten gespeichert werden sollen
     * @param[in] object ByteArrayPtr-Objekt, das auf die komprimierten Daten zeigt.
     *
     * @exception UnsupportedFeatureException Der eingestellte Komprimier-Algorithmus wird nicht unterstützt
     * @exception OutOfMemoryException Nicht genug Speicher verfügbar
     * @exception BufferTooSmallException Der intern zum komprimieren verwendete Puffer
     * ist zu klein. Sollte dieser Fall auftreten, handelt es sich um einen Bug.
     * @exception DecompressionFailedException Ein unerwarteter Fehler ist aufgetreten, die Daten konnten nicht dekomprimiert werden
     */
    void uncompress(ByteArray& out, const ByteArrayPtr& data);

    /** @brief Dekomprimierung eines Speicherbereichs in ein CBinary Objekt
     *
     * Mit dieser Version der Compress-Funktion wird der durch @p ptr angegebene
     * Speicherbereich mit einer Länge von @p size Bytes dekomprimiert und die entpackten
     * Daten im CBinary-Objekt @p out gespeichert.
     *
     * Diese Funktion unterstützt das Prefix-Flag (siehe Compression::UsePrefix).
     *
     * @param[out] out CBinary-Objekt, in dem die entpackten Daten gespeichert werden sollen
     * @param[in] ptr Pointer auf den Beginn des zu entpackenden Speicherbereichs
     * @param[in] size Größe des komprimierten Speicherbereichs
     *
     * @exception UnsupportedFeatureException Der eingestellte Komprimier-Algorithmus wird nicht unterstützt
     * @exception OutOfMemoryException Nicht genug Speicher verfügbar
     * @exception BufferTooSmallException Der intern zum komprimieren verwendete Puffer
     * ist zu klein. Sollte dieser Fall auftreten, handelt es sich um einen Bug.
     * @exception DecompressionFailedException Ein unerwarteter Fehler ist aufgetreten, die Daten konnten nicht dekomprimiert werden
     */
    void uncompress(ByteArray& out, const void* data, size_t size = 0);

    /** @brief Dekomprimierung eines Speicherbereichs in ein CBinary Objekt
     *
     * Mit dieser Version der Compress-Funktion wird der durch @p ptr angegebene
     * Speicherbereich mit einer Länge von @p size Bytes dekomprimiert und die entpackten
     * Daten als ByteArrayPtr zurückgegeben.
     *
     * Diese Funktion unterstützt das Prefix-Flag (siehe Compression::UsePrefix).
     *
     * @param[in] ptr Pointer auf den Beginn des zu entpackenden Speicherbereichs
     * @param[in] size Größe des komprimierten Speicherbereichs
     * @return Bei Erfolg wird ein ByteArrayPtr mit einer Referenz auf den dekomprimierten
     * Speicher zurückgegeben. Im Fehlerfall wird eine Exception geworfen.
     *
     * @exception UnsupportedFeatureException Der eingestellte Komprimier-Algorithmus wird nicht unterstützt
     * @exception OutOfMemoryException Nicht genug Speicher verfügbar
     * @exception BufferTooSmallException Der intern zum komprimieren verwendete Puffer
     * ist zu klein. Sollte dieser Fall auftreten, handelt es sich um einen Bug.
     * @exception DecompressionFailedException Ein unerwarteter Fehler ist aufgetreten, die Daten konnten nicht dekomprimiert werden
     */
    ByteArrayPtr uncompress(const void* ptr, size_t size);

    /** @brief Dekomprimierung eines Speicherbereichs in ein CBinary Objekt
     *
     * Mit dieser Version der Compress-Funktion wird der durch @p ptr angegebene
     * Speicherbereich mit einer Länge von @p size Bytes dekomprimiert und die entpackten
     * Daten als ByteArrayPtr zurückgegeben.
     *
     * Diese Funktion unterstützt das Prefix-Flag (siehe Compression::UsePrefix).
     *
     * @param[in] in Referenz auf den zu dekomprimierenden Speicher
     * @return Bei Erfolg wird ein ByteArrayPtr mit einer Referenz auf den dekomprimierten
     * Speicher zurückgegeben. Im Fehlerfall wird eine Exception geworfen.
     *
     * @exception UnsupportedFeatureException Der eingestellte Komprimier-Algorithmus wird nicht unterstützt
     * @exception OutOfMemoryException Nicht genug Speicher verfügbar
     * @exception BufferTooSmallException Der intern zum komprimieren verwendete Puffer
     * ist zu klein. Sollte dieser Fall auftreten, handelt es sich um einen Bug.
     * @exception DecompressionFailedException Ein unerwarteter Fehler ist aufgetreten, die Daten konnten nicht dekomprimiert werden
     */
    ByteArrayPtr uncompress(const ByteArrayPtr& in);
};

/** @ingroup PPLIB_COMPRESSION
 * @brief Speicherbereich komprimieren
 *
 * Mit dieser Funktion wird der durch \p in referenzierte Speicher
 * mit der Komprimierungsmethode \p method und dem Komprimierungslevel \p level komprimiert
 * und das Ergebnis im CMemory-Objekt \p out gespeichert.
 *
 * Speicherbereich komprimieren
 *
 * @param[out] out ByteArray-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] in Ein von ByteArrayPtr abgeleitetes Objekt mit den zu komprimierenden Daten
 * @param[in] method Die gewünschte Komprimierungsmethode (siehe Compression::Algorithm)
 * @param[in] level Der gewünschte Komprimierungslevel (siehe Compression::Level)
 *
 * @see Compression
 */
void Compress(ByteArray& out, const ByteArrayPtr& in, Compression::Algorithm method, Compression::Level level = Compression::Level_Default);

/** @ingroup PPLIB_COMPRESSION
 * @relatesalso Compression
 * @brief Daten dekomprimieren
 *
 * Mit dieser Funktion werden die in \p in enthaltenen komprimierten Daten
 * entpackt und das Ergebnis im CBinary-Objekt \p out gespeichert.
 *
 * Die Funktion geht davon aus, dass die komprimierten Daten mit einem
 * Version 2 Prefix beginnen (siehe \ref Compression_Prefix). Ist dies nicht der
 * Fall, sollte statt dieser Funktion die Klasse Compression verwendet werden,
 * deren Compression::Uncompress-Funktionen auch Dekomprimierung ohne Prefix
 * unterstützen.
 *
 * @param[out] out CBinary-Objekt, in dem die entpackten Daten gespeichert werden sollen
 * @param[in] in Das CBinary-Objekt, das die komprimierten Daten enthält
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0
 *
 * @see Compression
 */
void Uncompress(ByteArray& out, const ByteArrayPtr& in);

/** @ingroup PPLIB_COMPRESSION
 * @relatesalso Compression
 * @brief Daten mit ZLib komprimieren
 *
 * Mit dieser Funktion wird der durch \p in referenzierte Speicherbereich
 * mit der Komprimierungsmethode ZLib und dem Komprimierungslevel \p level komprimiert
 * und das Ergebnis im CMemory-Objekt \p out gespeichert.
 *
 * Die Funktion stellt den komprimierten Daten automatisch einen Version 2 Prefix voran (siehe
 * \ref Compression_Prefix), so dass die komprimierten Daten durch Aufruf der Funktion
 * Uncompress ohne Angabe der Kompressionsmethod wieder entpackt werden kann.
 *
 * @param[out] out ByteArray-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] in Ein ByteArrayPtr-Objekt mit den zu komprimierenden Daten.
 * @param[in] level Der gewünschte Komprimierungslevel (siehe Compression::Level). Der Default ist
 * Compression::Level_High
 *
 * \see Compression
 */
void CompressZlib(ByteArray& out, const ByteArrayPtr& in, Compression::Level level = Compression::Level_Default);

/** @ingroup PPLIB_COMPRESSION
 * @relatesalso Compression
 * @brief Daten mit BZip2 komprimieren
 *
 * Mit dieser Funktion wird der durch \p in referenzierte Speicherbereich
 * mit der Komprimierungsmethode BZip2 und dem Komprimierungslevel \p level komprimiert
 * und das Ergebnis im CMemory-Objekt \p out gespeichert.
 *
 * Die Funktion stellt den komprimierten Daten automatisch einen Version 2 Prefix voran (siehe
 * \ref Compression_Prefix), so dass die komprimierten Daten durch Aufruf der Funktion
 * Uncompress ohne Angabe der Kompressionsmethod wieder entpackt werden kann.
 *
 * @param[out] out CMemory-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] in Ein CMemoryReference-Objekt mit den zu komprimierenden Daten.
 * @param[in] level Der gewünschte Komprimierungslevel (siehe Compression::Level). Der Default ist
 * Compression::Level_High
 *
 * @see Compression
 */
void CompressBZip2(ByteArray& out, const ByteArrayPtr& in, Compression::Level level = Compression::Level_Default);

} // namespace pplib

#endif // PPLIB_CORE_COMPRESSION_H_
