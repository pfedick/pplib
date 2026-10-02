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

#ifndef PPLIB_CORE_GZFILE_H_
#define PPLIB_CORE_GZFILE_H_

#include <pplib/types/string.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/core/fileobject.h>
#include <pplib/core/file.h>

/**
 * @file pplib/core/gzfile.h
 * @brief Test 1
 */

namespace pplib
{

/** @class GzFile
 * @ingroup PPLGroupFileIO
 * @brief Zugriff auf eine mit gzip komprimierte Datei
 *
 * Mit dieser Klasse können mit gzip-komprimierte Dateien geladen, verändert und
 * gespeichert werden. Sie dient als Wrapper-Klasse für die Methoden aus der zlib-Bibliothek.
 *
 * @headerfile <pplib/core/gzfile.h>
 */
class GzFile : public FileObject
{
private:
    File* fh;
    void* ff;

public:
private:
    /** @brief %Exception anhand errno-Variable werfen
     *
     * Diese Funktion wird intern verwendet, um nach Auftreten eines Fehlers, anhand der globalen
     * "errno"-Variablen die passende Exception zu werfen.
     * @param e Errorcode aus der errno-Variablen
     * @param filename Dateiname, bei der der Fehler aufgetreten ist
     */
    void throwErrno(int e, const String& filename);

public:
    /** @brief Konstruktor der Klasse
     *
     * Konstruktor der Klasse
     */
    GzFile();

    /** @brief Konstruktor der Klasse mit gleichzeitigem Öffnen einer Datei
     *
     * Konstruktor der Klasse, mit dem gleichzeitig eine Datei geöffnet wird.
     * @param[in] filename Name der zu öffnenden Datei
     * @param[in] mode Zugriffsmodus. Defaultmäßig wird die Datei zum binären Lesen
     * geöffnet (siehe @ref pplib_File_Filemodi)
     */
    GzFile(const String& filename, File::FileMode mode = File::FileMode::READ);

    /** @brief Konstruktor mit Übernahme eines C-Filehandles
     *
     * Konstruktor der Klasse mit Übernahme eines C-Filehandles einer bereits mit ::fopen geöffneten Datei.
     * @param[in] handle File-Handle
     */
    GzFile(int fd);
    virtual ~GzFile();

    /** @brief Datei öffnen
     *
     * Mit dieser Funktion wird eine Datei zum Lesen, Schreiben oder beides geöffnet.
     * @param[in] filename Dateiname
     * @param mode Zugriffsmodus
     *
     * @return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
     */
    void open(const String& filename, File::FileMode mode = File::FileMode::READ);

    /** @brief Datei zum Lesen oder Schreiben öffnen
     *
     * Mit dieser Funktion wird eine Datei zum Lesen, Schreiben oder beides geöffnet.
     * @param filename Dateiname als C-String
     * @param mode String, der angibt, wie die Datei geöffnet werden soll (siehe @ref pplib_File_Filemodi)
     *
     * @return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
     */
    void open(const char* filename, File::FileMode mode = File::FileMode::READ);

    /** @brief Bereits geöffnete Datei übernehmen
     *
     * Mit dieser Funktion kann eine mit der C-Funktion @c fopen bereits geöffnete Datei
     * übernommen werden.
     *
     * @param[in] handle Das Filehandle
     * @return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
     */
    void open(int fd, File::FileMode mode = File::FileMode::READ);

    // Virtuelle Funktionen

    /** @brief Datei schließen
     *
     * Diese Funktion schließt die aktuell geöffnete Datei. Sie wird automatisch vom Destruktor der
     * Klasse aufgerufen, so dass ihr expliziter Aufruf nicht erforderlich ist.
     *
     * Wenn  der  Stream  zur  Ausgabe  eingerichtet  war,  werden  gepufferte  Daten  zuerst  durch
     * FileObject::flush
     * geschrieben. Der zugeordnete Datei-Deskriptor wird geschlossen, alle Systemressourcen werden
     * freigegeben.
     *
     * @return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
     */
    virtual void close();
    virtual void rewind();
    virtual void seek(uint64_t position);
    virtual uint64_t seek(int64_t offset, SeekOrigin origin);
    virtual uint64_t tell();
    virtual bool eof() const;
    virtual bool isOpen() const;
    virtual size_t fread(void* ptr, size_t size, size_t nmemb);
    virtual char* fgets(char* buffer, size_t num);
    virtual int fgetc();
    virtual size_t fwrite(const void* ptr, size_t size, size_t nmemb);
};

} // namespace pplib

#endif /* PPLIB_CORE_GZFILE_H_ */
