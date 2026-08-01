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

#include <ppl7/core/fileobject.h>
#include <ppl7/core/file.h>
#include <ppl7/core/gzfile.h>
#include <ppl7/exceptions.h>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

namespace ppl7
{

static const char* fmode(File::FileMode mode)
{
    switch (mode) {
    case File::FileMode::READ:
        return "rb";
    case File::FileMode::WRITE:
        return "wb";
    case File::FileMode::READWRITE:
        return "r+b";
    case File::FileMode::APPEND:
        return "ab";
    default:
        throw IllegalArgumentException("Filemode");
    }
}

/*!\class GzFile
 * \ingroup PPLGroupFileIO
 * \brief Zugriff auf eine mit gzip komprimierte Datei
 *
 * \header \#include <ppl7.h>
 * \desc
 * Mit dieser Klasse können mit gzip-komprimierte Dateien geladen, verändert und
 * gespeichert werden. Sie dient als Wrapper-Klasse für die Methoden aus der zlib-Bibliothek.
 *
 */

/*!\brief Konstruktor der Klasse
 *
 * \desc
 * Konstruktor der Klasse
 */
GzFile::GzFile()
{
    ff = NULL;
    fh = NULL;
}

/*!\brief Konstruktor der Klasse mit gleichzeitigem Öffnen einer Datei
 *
 * Konstruktor der Klasse, mit dem gleichzeitig eine Datei geöffnet wird.
 * @param[in] filename Name der zu öffnenden Datei
 * @param[in] mode Zugriffsmodus. Defaultmäßig wird die Datei zum binären Lesen
 * geöffnet (siehe \ref ppl7_File_Filemodi)
 */
GzFile::GzFile(const String& filename, File::FileMode mode)
{
    ff = NULL;
    fh = NULL;
    open(filename, mode);
}

/*!\brief Konstruktor mit Übernahme eines C-Filehandles
 *
 * \desc
 * Konstruktor der Klasse mit Übernahme eines C-Filehandles einer bereits mit ::fopen geöffneten Datei.
 *
 * @param[in] handle File-Handle
 */
GzFile::GzFile(int fd)
{
    ff = NULL;
    fh = NULL;
    open(fd);
}

/*!\brief Destruktor der Klasse
 *
 * \desc
 * Der Destruktor der Klasse sorgt dafür, dass eine noch geöffnete Datei geschlossen wird und
 * alle Systemresourcen wieder freigegeben werden.
 */

GzFile::~GzFile()
{
    if (ff != NULL) {
        this->close();
    }
    if (fh != NULL) {
        fh->close();
        delete (fh);
    }
}

/*!\brief %Exception anhand errno-Variable werfen
 *
 * \desc
 * Diese Funktion wird intern verwendet, um nach Auftreten eines Fehlers, anhand der globalen
 * "errno"-Variablen die passende Exception zu werfen.
 *
 * @param e Errorcode aus der errno-Variablen
 * @param filename Dateiname, bei der der Fehler aufgetreten ist
 */
void GzFile::throwErrno(int e, const String& filename)
{
    throwExceptionFromErrno(e, filename);
}

/*!\brief Exception anhand errno-Variable werfen
 *
 * \desc
 * Diese Funktion wird intern verwendet, um nach Auftreten eines Fehlers, anhand der globalen
 * "errno"-Variablen die passende Exception zu werfen.
 *
 * @param e Errorcode aus der errno-Variablen
 */
void GzFile::throwErrno(int e)
{
    throwExceptionFromErrno(e, filename());
}

/*!\brief Datei öffnen
 *
 * \desc
 * Mit dieser Funktion wird eine Datei zum Lesen, Schreiben oder beides geöffnet.
 * @param[in] filename Dateiname
 * @param mode Zugriffsmodus
 *
 * \return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
 */
void GzFile::open(const String& filename, File::FileMode mode)
{
    if (filename.isEmpty()) throw IllegalArgumentException();
    close();
    fh = new File(filename, mode);
    int dupfd = dup(fh->getFileNo());
    if ((ff = gzdopen(dupfd, fmode(mode))) == NULL) {
        int save_errno = errno;
        ::close(dupfd);
        throwErrno(save_errno, filename);
    }
    seek(0);
    setFilename(filename);
}

/*!\brief Datei zum Lesen oder Schreiben öffnen
 *
 * \desc
 * Mit dieser Funktion wird eine Datei zum Lesen, Schreiben oder beides geöffnet.
 *
 * \param filename Dateiname als C-String
 * \param mode String, der angibt, wie die Datei geöffnet werden soll (siehe \ref ppl7_File_Filemodi)
 *
 * \return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
 */
void GzFile::open(const char* filename, File::FileMode mode)
{
    if (filename == NULL || strlen(filename) == 0) throw IllegalArgumentException();
    close();
    fh = new File;
    fh->open(filename, mode);
    int dupfd = dup(fh->getFileNo());
    if ((ff = gzdopen(dupfd, fmode(mode))) == NULL) {
        int save_errno = errno;
        ::close(dupfd);
        throwErrno(save_errno, filename);
    }
    seek(0);
    setFilename(filename);
}

/*
 *!\brief Bereits geöffnete Datei übernehmen
 *
 * Mit dieser Funktion kann eine mit der C-Funktion \c fopen bereits geöffnete Datei
 * übernommen werden.
 *
 * @param[in] handle Das Filehandle
 * @return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
 */
void GzFile::open(int fd, File::FileMode mode)
{
    if (fd == 0) throw IllegalArgumentException();
    close();
    if ((ff = gzdopen(fd, fmode(mode))) == NULL) {
        throwErrno(errno, "FILE");
    }
    seek(0);
    setFilename("FILE");
}

/*!\brief Datei schließen
 *
 * \desc
 * Diese Funktion schließt die aktuell geöffnete Datei. Sie wird automatisch vom Destruktor der
 * Klasse aufgerufen, so dass ihr expliziter Aufruf nicht erforderlich ist.
 * \par
 * Wenn  der  Stream  zur  Ausgabe  eingerichtet  war,  werden  gepufferte  Daten  zuerst  durch
 * FileObject::flush
 * geschrieben. Der zugeordnete Datei-Deskriptor wird geschlossen, alle Systemressourcen werden
 * freigegeben.
 *
 * \return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
 */
void GzFile::close()
{
    setFilename("");
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    if (ff != NULL) {
        int ret = gzclose((gzFile)ff);
        ff = NULL;
        if (ret != Z_OK) {
            if (ret == Z_ERRNO)
                throwErrno(errno, filename());
            else if (ret == Z_MEM_ERROR)
                throw ppl7::OutOfMemoryException();
            throw ppl7::CompressionFailedException();
        }
    }
    if (fh != NULL) {
        fh->close();
        delete (fh);
        fh = NULL;
    }
}

bool GzFile::isOpen() const
{
    if (ff != NULL) return true;
    return false;
}

void GzFile::rewind()
{
    if (ff != NULL) {
        gzrewind((gzFile)ff);
        return;
    }
    throw FileNotOpenException();
}

void GzFile::seek(uint64_t position)
{
    seek(position, SEEKSET);
}

uint64_t GzFile::seek(int64_t offset, SeekOrigin origin)
{
    if (ff == NULL) throw FileNotOpenException();
    int o = 0;
    switch (origin) {
    case File::SEEKCUR:
        o = SEEK_CUR;
        break;
    case File::SEEKSET:
        o = SEEK_SET;
        break;
    case File::SEEKEND:
        throw ppl7::UnsupportedFeatureException("GzFile::SEEKEND");
    default:
        throw IllegalArgumentException();
    }
    int suberr = ::gzseek((gzFile)ff, (long)offset, o);
    if (suberr >= 0) {
        return tell();
    }
    throwErrno(errno, filename());
    return 0;
}

uint64_t GzFile::tell()
{
    if (ff != NULL) {
        return (uint64_t)gztell((gzFile)ff);
    }
    throw FileNotOpenException();
}

bool GzFile::eof() const
{
    if (ff == NULL) throw FileNotOpenException();
    if (gzeof((gzFile)ff) != 0) return true;
    return false;
}

size_t GzFile::fread(void* ptr, size_t size, size_t nmemb)
{
    if (ff == NULL) throw FileNotOpenException();
    if (ptr == NULL) throw IllegalArgumentException();
    int by = ::gzread((gzFile)ff, ptr, (unsigned int)(size * nmemb));
    if (by > 0) return by;
    if (by == 0) throw ppl7::EndOfFileException();
    int err = 0;
    const char* msg = gzerror((gzFile)ff, &err);
    throw ppl7::CompressionFailedException("gzread: %s [%d]", msg, err);
}

char* GzFile::fgets(char* buffer, size_t num)
{
    if (ff == NULL) throw FileNotOpenException();
    if (buffer == NULL) throw IllegalArgumentException();
    // int suberr;
    char* res;
    res = ::gzgets((gzFile)ff, buffer, (int)num);
    if (res == NULL) {
        // suberr=::ferror((FILE*)ff);
        if (gzeof((gzFile)ff))
            throw ppl7::EndOfFileException();
        else
            throwErrno(errno, filename());
    }
    return buffer;
}

int GzFile::fgetc()
{
    if (ff == NULL) throw FileNotOpenException();
    int ret = gzgetc((gzFile)ff);
    if (ret != -1) {
        return ret;
    }
    throw ppl7::EndOfFileException();
}

size_t GzFile::fwrite(const void* ptr, size_t size, size_t nmemb)
{
    if (ff == NULL) throw FileNotOpenException();
    if (ptr == NULL) throw IllegalArgumentException();
    int by = ::gzwrite((gzFile)ff, ptr, (unsigned int)(size * nmemb));
    if (by > 0) return by;
    if (by == 0) throw ppl7::EndOfFileException();
    int err = 0;
    const char* msg = gzerror((gzFile)ff, &err);
    throw ppl7::CompressionFailedException("gzread: %s [%d]", msg, err);
}

} // end of namespace ppl7
