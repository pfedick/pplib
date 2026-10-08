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

#ifndef PPLIB_CORE_MEMFILE_H_
#define PPLIB_CORE_MEMFILE_H_

#include <pplib/core/fileobject.h>
#include <pplib/types/bytearray.h>

namespace pplib
{

/** @class MemFile
 * @ingroup PPLGroupFileIO
 * @brief Simulation von Dateizugriffen im Hauptspeicher
 *
 * \desc
 * Mit dieser von FileObject abgeleiteten Klasse werden Dateizugriffe im Hauptspeicher simuliert.
 * Sie kann immer dann verwendet werden, wenn sich die zu lesende Datei bereits im Hauptspeicher
 * befindet, oder Daten temporär im Hauptspeicher abgelegt werden sollen.
 *
 * Der zu verwendende Speicherbereich kann entweder über den Konstruktor abgegeben werden (siehe
 * MemFile::MemFile (void * adresse, size_t size) ) oder über die Funktion MemFile::open. Soll
 * der Speicherbereich auch beschrieben werden, muss als dritter Parameter "true" angegeben
 * oder die Funktion MemFile::openReadWrite verwendet werden.
 */
class MemFile : public FileObject
{
private:
    ByteArray writebuffer;
    size_t mysize;
    size_t pos;
    size_t maxsize;
    // size_t buffersize;
    char* MemBase;
    bool readonly;
    bool is_open;

    void resizeBuffer(size_t size);

public:
    /** @brief Konstruktor der Klasse
     *
     * Durch Verwendung dieses Konstruktors wird die Klasse zum Lesen und Schreiben geöffnet, wobei
     * der Speicherbereich initial 0 Byte gross ist. Beim ersten Schreibzugriff wird der notwendige
     * Speicher allokiert.
     */
    MemFile() noexcept;

    /** @brief Konstruktor der Klasse mit Angabe eines Speicherbereichs
     *
     * Mit diesem Konstruktor wird gleichzeitig ein Pointer auf den Speicherbereich \p adresse mit einer
     * Größe von \p size Bytes übergeben. Sämtliche Dateizugriffe werden in diesem Speicherbereich
     * simuliert.
     * @param adresse Pointer auf den zu verwendenden Speicherbereich
     * @param size Größe des Speicherbereichs
     * @param writeable Gibt an, ob der Speicherbereich auch beschreibbar sein soll.
     * @attention Wird der Parameter @p writeable auf "true" gesetzt, geht die Verwaltung des
     * Speichers an die MemFile-Klasse über. Der Speicher darf nicht mehr von der Applikation verändert
     * oder freigegeben werden!
     */
    MemFile(void* adresse, size_t size, bool writeable = false);

    /** @brief Konstruktor der Klasse mit Angabe eines Speicherbereichs
     *
     * Mit diesem Konstruktor wird gleichzeitig ein Pointer auf den Speicherbereich @p adresse mit einer
     * Größe von @p size Bytes übergeben. Sämtliche Dateizugriffe werden in diesem Speicherbereich
     * simuliert. Ein Schreibzugriff auf diesen Speicherbereich ist nicht möglich.
     *
     * @param adresse Pointer auf den zu verwendenden Speicherbereich
     * @param size Größe des Speicherbereichs
     */
    MemFile(const ByteArrayPtr& memory);
    ~MemFile();

    MemFile(const MemFile&) = delete;
    MemFile& operator=(const MemFile&) = delete;
    MemFile(MemFile&& other) noexcept;
    MemFile& operator=(MemFile&& other) noexcept;

    /** @brief Speicherbereich zum Lesen öffnen
     *
     * Mit dieser Funktion wird die simulierte Datei im Hauptspeicher geöffnet. Dazu muss mit
     * @p adresse ein Pointer auf den Beginn des zu verwendenden Hauptspeichers angegeben werden,
     * sowie mit @p size seine Größe. Sämtliche nachfolgenden Dateizugriffe werden dann in diesem
     * Speicherbereich simuliert. Ein Schreibender Zugriff ist nicht möglich.
     *
     * @param adresse Pointer auf den zu verwendenden Speicherbereich
     * @param size Größe des Speicherbereichs
     * @attention Wird der Parameter @p writeable auf "true" gesetzt, geht die Verwaltung des
     * Speichers an die MemFile-Klasse über. Der Speicher darf nicht mehr von der Applikation verändert
     * oder freigegeben werden!
     */
    void open(void* adresse, size_t size, bool writeable = false);

    /** @brief Speicherbereich zum Lesen öffnen
     *
     * Mit dieser Funktion wird die simulierte Datei im Hauptspeicher zum Lesen geöffnet. Dazu muss
     * mit @p adresse ein Pointer auf den Beginn des zu verwendenden Hauptspeichers angegeben werden,
     * sowie mit @p size seine Größe. Sämtliche nachfolgenden Dateizugriffe werden dann in diesem
     * Speicherbereich simuliert. Ein Schreibender Zugriff ist nicht möglich.
     *
     * @param memory Referenz auf eine ByteArrayPtr-Klasse, die den zu verwendenden Speicherbereich enthält
     * @see openReadWrite: Datei zum Lesen und Schreiben öffnen
     */
    void open(const ByteArrayPtr& memory);

    /** @brief Speicherbereich zum Schreiben und Lesen öffnen
     *
     * Mit dieser Funktion wird die simulierte Datei im Hauptspeicher zum Lesen und Schreiben
     * geöffnet. Dazu muss mit @p adresse ein Pointer auf den Beginn des zu verwendenden
     * Hauptspeichers angegeben werden,
     * sowie mit @p size seine initiale Größe. Sämtliche nachfolgenden Dateizugriffe werden
     * dann in diesem Speicherbereich simuliert. Erfolgt ein schreibender Zugriff über dessen
     * Ende hinaus, wird der Speicherbereich automatisch vergrößert.
     *
     * @param adresse Pointer auf den zu verwendenden Speicherbereich
     * @param size Größe des Speicherbereichs
     * @see open: Datei wird nur zum Lesen geöffnet
     * @see setMaxSize: Legt die maximale Größe der Datei im Speicher fest (Default=unlimitiert)
     * @attention Die Verwaltung des Speichers geht an die MemFile-Klasse über. Der Speicher darf nicht mehr
     * von der Applikation verändert oder freigegeben werden!
     */
    void openReadWrite(void* adresse, size_t size);

    /** @brief Interne Adresse des Speicherbereichs
     *
     * Diese Funktion gibt einen Zeiger auf die interne Speicheradresse zurück. Sie kann verwendet werden,
     * um direkt auf den Speicherbereich zuzugreifen, der von der MemFile-Klasse verwaltet wird.
     *
     * @param adresse Offset innerhalb des Speicherbereichs
     * @return Zeiger auf die interne Speicheradresse ab dem angegebenen Offset
     * @exception FileNotOpenException Wird geworfen, wenn die Datei nicht geöffnet ist.
     * @exception OutOfBoundsException Wird geworfen, wenn der angegebene Offset außerhalb des Speicherbereichs liegt.
     * @attention Der zurückgegebene Zeiger ist nur gültig, solange die Datei geöffnet ist und der interne Speicherbereich nicht verändert
     * wird. Die Methode ist mit Vorsicht zu verwenden, da leicht über den gültigen Speicherbereich hinaus zugegriffen werden kann.
     */
    char* adr(size_t adresse = 0);

    /** @brief Maximale Dateigröße festlegen
     *
     * Mit dieser Funktion wird die maximale Größe einer Datei im Hauptspeicher festgelegt.
     * Damit werden alle Schreibenden Zugriffe begrenzt, die Datei kann nicht größer werden
     * als \p size. Standardmäßig gibt es keine Limitierung, die Datei kann somit so groß werden,
     * wie Hauptspeicher zur Verfügung steht.
     * @param[in] size Maximale Größe in Bytes. Der Wert "0" hebt die Limitierung auf.
     * @see Mit der Funktion MemFile::maxSize kann das derzeitige Limit ausgelesen werden.
     */
    void setMaxSize(size_t size);
    size_t maxSize() const;

    // Virtuelle Funktionen
    void close() override;
    void rewind() override;
    void seek(uint64_t position) override;
    uint64_t seek(int64_t offset, SeekOrigin origin) override;
    uint64_t tell() override;
    size_t fread(void* ptr, size_t size, size_t nmemb) override;
    size_t fwrite(const void* ptr, size_t size, size_t nmemb) override;
    char* fgets(char* buffer, size_t num) override;
    wchar_t* fgetws(wchar_t* buffer, size_t num = 1024) override;
    void fputc(int c) override;
    int fgetc() override;
    void fputwc(wchar_t c) override;
    wchar_t fgetwc() override;
    void fputs(const char* str) override;
    void fputws(const wchar_t* str) override;
    bool eof() const override;
    uint64_t size() const override;
    char* map(uint64_t position, size_t size, MapProtection prot = MapProtection::READ) override;
    using FileObject::map;
    void unmap() override;
    void setMapReadAhead(size_t bytes) override;

    /** @copybrief FileObject::getFileNo
     *
     * Diese Funktion steht bei bei dieser Speicherklasse nicht zur Verfügung. Bei
     * Aufruf der Funktion wird eine OperationUnavailableException geworfen.
     */
    int getFileNo() const override;
    void flush() override;
    void sync() override;
    void truncate(uint64_t length) override;
    inline bool isOpen() const override
    {
        return is_open;
    }

    /** @copybrief FileObject::lockShared
     *
     * Diese Funktion steht bei bei dieser Speicherklasse nicht zur Verfügung. Bei
     * Aufruf der Funktion wird eine OperationUnavailableException geworfen.
     */
    void lockShared(bool block = true) override;

    /** @copybrief FileObject::lockExclusive
     *
     * Diese Funktion steht bei bei dieser Speicherklasse nicht zur Verfügung. Bei
     * Aufruf der Funktion wird eine OperationUnavailableException geworfen.
     */
    void lockExclusive(bool block = true) override;

    /** @copybrief FileObject::unlock
     *
     * Diese Funktion steht bei bei dieser Speicherklasse nicht zur Verfügung. Bei
     * Aufruf der Funktion wird eine OperationUnavailableException geworfen.
     */
    void unlock() override;
};

} // namespace pplib

#endif /* PPLIB_CORE_MEMFILE_H_ */
