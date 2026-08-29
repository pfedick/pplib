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

#include <wchar.h>
#include <time.h>
#include <unistd.h>

#ifndef _WIN32
#include <sys/mman.h>
#endif

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stdarg.h>
#include <errno.h>

#ifdef _WIN32
#include <io.h>
#define WIN32_LEAN_AND_MEAN // Keine MFCs
#define _CRT_SECURE_NO_WARNINGS 1
#define popen _popen
#define pclose _pclose
#define fileno _fileno
#include <windows.h>

#ifdef _MSC_VER
#define fseeko _fseeki64
#define ftello _ftelli64
#endif

#endif // _WIN32

#include <pplib/exceptions.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/array.h>

#include <pplib/core/file.h>
#include <pplib/core/dir.h>

namespace pplib
{

File::File()
{
    MapBase = NULL;
    ff = NULL;
    mysize = pos = 0;
    LastMapStart = LastMapSize = 0;
    LastMapProtection = 0;
    ReadAhead = 0;
    isPopen = false;
}

File::File(const String& filename, FileMode mode)
    : File()
{
    open(filename, mode);
}

File::File(FILE* handle)
    : File()
{
    if (handle != NULL) {
        ff = handle;
        mysize = size();
        this->seek((uint64_t)0);
    }
}

File::~File()
{
    if (ff != NULL) {
        close();
    }
}

/*!\brief C-Filemode-String
 *
 * \desc
 * Diese interne Funktion gibt den zum Datemodus \p mode passenden C-Filemode-String
 * zurück.
 *
 * @param mode Filemodus aus der Enumeration FileMode
 * @return C-String
 */
#ifdef _WIN32
static const wchar_t* fmode(File::FileMode mode)
{
    switch (mode) {
    case File::FileMode::READ:
        return L"rb";
    case File::FileMode::WRITE:
        return L"wb";
    case File::FileMode::READWRITE:
        return L"r+b";
    case File::FileMode::APPEND:
        return L"ab";
    default:
        throw IllegalArgumentException();
    }
}
#else
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
        throw IllegalArgumentException();
    }
}
#endif

/*!\brief C-Filemode-String für popen
 *
 * \desc
 * Diese interne Funktion gibt den zum Datemodus \p mode passenden C-Filemode-String
 * für die popen-Funktion zurück.
 *
 * @param mode Filemodus aus der Enumeration FileMode
 * @return C-String
 */
#ifdef _WIN32
const wchar_t* fmodepopen(File::FileMode mode)
{
    switch (mode) {
    case File::FileMode::READ:
        return L"r";
    case File::FileMode::WRITE:
        return L"w";
    case File::FileMode::READWRITE:
        return L"r+";
    default:
        throw IllegalArgumentException();
    }
}
#else
const char* fmodepopen(File::FileMode mode)
{
    switch (mode) {
    case File::FileMode::READ:
        return "r";
    case File::FileMode::WRITE:
        return "w";
    case File::FileMode::READWRITE:
        return "r+";
    default:
        throw IllegalArgumentException();
    }
}
#endif

void File::throwErrno(int e, const String& filename)
{
    throwExceptionFromErrno(e, filename);
}

void File::throwErrno(int e)
{
    throwExceptionFromErrno(e, filename());
}

void File::open(const String& filename, FileMode mode)
{
    close();
    if (filename.isEmpty()) throw IllegalArgumentException();
#ifdef _WIN32
    if ((ff = (FILE*)::_wfopen((const wchar_t*)WideString(filename), fmode(mode))) == NULL) {
        throwErrno(errno, filename);
    }

#else
    if ((ff = (FILE*)::fopen((const char*)filename, fmode(mode))) == NULL) {
        throwErrno(errno, filename);
    }
#endif
    mysize = size();
    seek(0);
    setFilename(filename);
}

void File::openTemp(const String& filetemplate)
{
    close();
    String tmpname = filetemplate;
    int f = ::mkstemp((char*)((const char*)tmpname));
    if (f < 0) throwErrno(errno, filetemplate);
    FILE* ff = ::fdopen(f, "r+b");
    if (!ff) throwErrno(errno, filetemplate);
    try {
        open(ff);
    }
    catch (...) {
        try {
            ::fclose(ff);
            ::close(f);
        }
        catch (...) {
        }
        throw;
    }
    setFilename((const char*)tmpname);
}

void File::close()
{
    if (MapBase != NULL) {
        this->munmap(MapBase, (size_t)LastMapSize);
    }

    setFilename("");
    if (ff != NULL) {
        int ret = 1;
        if (isPopen) {
            if (::pclose((FILE*)ff) != 0) ret = 0;
        } else {
            if (::fclose((FILE*)ff) != 0) ret = 0;
        }
        isPopen = false;
        ff = NULL;
        mysize = 0;
        pos = 0;
        if (ret == 0) throwErrno(errno, filename());
        return;
    }
    // throw FileNotOpenException();
}

bool File::isOpen() const
{
    if (ff != NULL) return true;
    return false;
}

uint64_t File::size() const
{
    if (ff != NULL) {
#ifdef _WIN32
        struct _stat buf;
        if ((::_fstat(_fileno((FILE*)ff), &buf)) == 0) return ((uint64_t)buf.st_size);
        throwErrno(errno, filename());
#else
        struct stat buf;
        if ((::fstat(fileno((FILE*)ff), &buf)) == 0) return ((uint64_t)buf.st_size);
        throwErrno(errno, filename());
#endif
    }
    throw FileNotOpenException();
}

void File::popen(const String& command, FileMode mode)
{
    close();
    if (command.isEmpty()) throw IllegalArgumentException();
#ifdef _WIN32
    if ((ff = (FILE*)::_wpopen((const wchar_t*)WideString(command), fmodepopen(mode))) == NULL) {
        throwErrno(errno, command);
    }

#else
    if ((ff = (FILE*)::popen((const char*)command, fmodepopen(mode))) == NULL) {
        throwErrno(errno, command);
    }
#endif
    isPopen = true;
    mysize = size();
    setFilename(command);
}

void File::open(FILE* handle)
{
    if (handle == NULL) throw IllegalArgumentException();
    close();
    ff = handle;
    mysize = size();
    seek(0);
    setFilename("FILE");
}

void File::rewind()
{
    if (ff != NULL) {
        pos = 0;
        return;
    }
    throw FileNotOpenException();
}

void File::seek(uint64_t position)
{
    if (ff == NULL) {
        throw FileNotOpenException();
    }
    if (::fseeko((FILE*)ff, (off_t)position, SEEK_SET) != 0) {
        throwErrno(errno, filename());
    }
    pos = tell();
    if (pos != position) throw FileSeekException();
    return;
}

uint64_t File::seek(int64_t offset, SeekOrigin origin)
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
        o = SEEK_END;
        break;
    default:
        throw IllegalArgumentException();
    }
    int suberr = ::fseek((FILE*)ff, (long)offset, o);
    if (suberr == 0) {
        pos = tell();
        return pos;
    }
    throwErrno(errno, filename());
    return 0;
}

uint64_t File::tell()
{
    if (ff != NULL) {
        off_t p = ::ftello((FILE*)ff);
        if (p != (off_t)-1) return p;
        throwErrno(errno, filename());
    }
    throw FileNotOpenException();
}

size_t File::fread(void* ptr, size_t size, size_t nmemb)
{
    if (ff == NULL) throw FileNotOpenException();
    if (ptr == NULL) throw IllegalArgumentException();
    size_t by = ::fread(ptr, size, nmemb, (FILE*)ff);
    pos += (by * size);
    if (by != nmemb) {
        if (::ferror((FILE*)ff)) throwErrno(errno, filename());
    }
    if (by == 0) {
        if (::feof((FILE*)ff)) throw EndOfFileException();
        throwErrno(errno, filename());
    }
    return by;
}

size_t File::fwrite(const void* ptr, size_t size, size_t nmemb)
{
    if (ff == NULL) throw FileNotOpenException();
    if (ptr == NULL) throw IllegalArgumentException();
    size_t by = ::fwrite(ptr, size, nmemb, (FILE*)ff);
    pos += (by * size);
    if (pos > this->mysize) this->mysize = pos;
    if (by < nmemb) throwErrno(errno, filename());
    return by;
}

char* File::fgets(char* buffer, size_t num)
{
    if (ff == NULL) throw FileNotOpenException();
    if (buffer == NULL) throw IllegalArgumentException();
    // int suberr;
    char* res;
    res = ::fgets(buffer, num, (FILE*)ff);
    if (res == NULL) {
        // suberr=::ferror((FILE*)ff);
        if (::feof((FILE*)ff))
            return NULL;
        else
            throwErrno(errno, filename());
    }
    uint64_t by = (uint64_t)strlen(buffer);
    pos += by;
    return buffer;
}

wchar_t* File::fgetws(wchar_t* buffer, size_t num)
{
    if (ff == NULL) throw FileNotOpenException();
    if (buffer == NULL) throw IllegalArgumentException();
    // int suberr;
    wchar_t* res;
    res = ::fgetws(buffer, num, (FILE*)ff);
    if (res == NULL) {
        // suberr=::ferror((FILE*)ff);
        if (::feof((FILE*)ff))
            return NULL;
        else
            throwErrno(errno, filename());
    }
    uint64_t by = (uint64_t)wcslen(buffer) * sizeof(wchar_t);
    pos += by;
    return buffer;
}

void File::fputs(const char* str)
{
    if (ff == NULL) throw FileNotOpenException();
    if (str == NULL) throw IllegalArgumentException();
    if (::fputs(str, (FILE*)ff) != EOF) {
        pos += strlen(str);
        if (pos > mysize) mysize = pos;
        return;
    }
    throwErrno(errno, filename());
}

void File::fputws(const wchar_t* str)
{
    if (ff == NULL) throw FileNotOpenException();
    if (str == NULL) throw IllegalArgumentException();
    if (::fputws(str, (FILE*)ff) != -1) {
        pos += wcslen(str) * sizeof(wchar_t);
        if (pos > mysize) mysize = pos;
        return;
    }
    throwErrno(errno, filename());
}

void File::fputc(int c)
{
    if (ff == NULL) throw FileNotOpenException();
    int ret = ::fputc(c, (FILE*)ff);
    if (ret != EOF) {
        pos++;
        if (pos > mysize) mysize = pos;
        return;
    }
    throwErrno(errno);
}

int File::fgetc()
{
    if (ff == NULL) throw FileNotOpenException();
    int ret = ::fgetc((FILE*)ff);
    if (ret != EOF) {
        pos++;
        return ret;
    }
    if (ret == EOF) return EOF;
    if (errno != 0) throwErrno(errno);
    return 0;
}

void File::fputwc(wchar_t c)
{
    if (ff == NULL) throw FileNotOpenException();
    wint_t ret = ::fputwc(c, (FILE*)ff);
    if (ret != WEOF) {
        pos += sizeof(wchar_t);
        if (pos > mysize) mysize = pos;
        return;
    }
    throwErrno(errno);
}

wchar_t File::fgetwc()
{
    if (ff == NULL) throw FileNotOpenException();
    wint_t ret = ::fgetwc((FILE*)ff);
    if (ret != WEOF) {
        pos += sizeof(wchar_t);
        return (wchar_t)ret;
    }
    if (errno != 0) throwErrno(errno);
    return 0;
}

bool File::eof() const
{
    if (ff == NULL) return true;
    if (!isPopen && pos >= mysize) return true; // reguläre Datei: positionsbasiert
    if (::feof((FILE*)ff)) return true;         // Pipes & Fallback
    return false;
}

int File::getFileNo() const
{
    if (ff == NULL) throw FileNotOpenException();
    return fileno((FILE*)ff);
}

void File::flush()
{
    if (ff == NULL) throw FileNotOpenException();
    if (fflush((FILE*)ff) == 0) return;
    throwErrno(errno);
}

void File::sync()
{
    if (ff == NULL) throw FileNotOpenException();
#ifndef _WIN32
    int ret = fsync(fileno((FILE*)ff));
    if (ret == 0) return;
    throwErrno(errno);
#else
    ::fflush((FILE*)ff);
    if (::_commit(fileno((FILE*)ff)) == 0) return;
    throwErrno(errno);
#endif
}

void File::truncate(uint64_t length)
{
    if (ff == NULL) throw FileNotOpenException();
#ifndef _WIN32
    int fd = fileno((FILE*)ff);
    int ret = ::ftruncate(fd, (off_t)length);
    if (ret == 0) {
        mysize = length;
        if (pos > mysize) seek(mysize);
        return;
    }
    throwErrno(errno);
#else
    int fd = fileno((FILE*)ff);
    if (_chsize_s(fd, (long)length) == 0) {
        mysize = length;
        if (pos > mysize) seek(mysize);
        return;
    }
    throwErrno(errno);
#endif
}

void File::lockExclusive(bool block)
{
    if (ff == NULL) throw FileNotOpenException();
#ifndef _WIN32
    int fd = fileno((FILE*)ff);
    int flags = LOCK_EX;
    if (!block) flags |= LOCK_NB;
    int ret = flock(fd, flags);
    if (ret == 0) return;
    throwErrno(errno);
#else
    // TODO : Implement file locking for Windows
    throw UnsupportedFeatureException("pplib::File::unlock: No file locking available");
#endif
}

void File::lockShared(bool block)
{
    if (ff == NULL) throw FileNotOpenException();
#ifndef _WIN32
    int fd = fileno((FILE*)ff);
    int flags = LOCK_SH;
    if (!block) flags |= LOCK_NB;
    int ret = flock(fd, flags);
    if (ret == 0) return;
    throwErrno(errno);
#else
    // TODO : Implement file locking for Windows
    throw UnsupportedFeatureException("pplib::File::unlock: No file locking available");

#endif
}

void File::unlock()
{
    if (ff == NULL) throw FileNotOpenException();
#ifndef _WIN32
    int fd = fileno((FILE*)ff);
    int ret = flock(fd, LOCK_UN);
    if (ret == 0) return;
    throwErrno(errno);
#else
    // TODO : Implement file locking for Windows
    throw UnsupportedFeatureException("pplib::File::unlock: No file locking available");
#endif
}

// TODO: macht der ReadAhead überhaupt Sinn?
void File::setMapReadAhead(size_t bytes)
{
    ReadAhead = bytes;
}

const char* File::map(uint64_t position, size_t bytes)
{
    if (ff == NULL) throw FileNotOpenException();
    if (position + bytes <= mysize) {
        if (MapBase != NULL) {
            if (LastMapStart == position) { // Dateiausschnitt schon gemapped?
                if (bytes <= LastMapSize) return MapBase;
            }
            if (position > LastMapStart && (bytes + position - LastMapStart) <= LastMapSize) return MapBase + position - LastMapStart;
            this->munmap(MapBase, (size_t)LastMapSize);
        }
        LastMapStart = position;
        if (ReadAhead > 0 && bytes < ReadAhead) {
            bytes = ReadAhead;
            if (position + (uint64_t)bytes > mysize) bytes = (size_t)(mysize - position);
        }
        LastMapSize = bytes;
        return (const char*)this->mmap(position, bytes, 1, 0);
    }
    throw OverflowException();
}

char* File::mapRW(uint64_t position, size_t bytes)
{
    if (ff == NULL) throw FileNotOpenException();
    if (position + bytes <= mysize) {
        if (MapBase != NULL) {
            if ((LastMapProtection & 2)) {      // Schon als read/write gemapped?
                if (LastMapStart == position) { // Dateiausschnitt schon gemapped?
                    if (bytes <= LastMapSize) return MapBase;
                }
                if (position > LastMapStart && (bytes + position - LastMapStart) <= LastMapSize) return MapBase + position - LastMapStart;
            }
            this->munmap(MapBase, (size_t)LastMapSize);
        }
        if (ReadAhead > 0 && bytes < ReadAhead) {
            bytes = ReadAhead;
            if (position + bytes > mysize) bytes = (size_t)(mysize - position);
        }
        return (char*)this->mmap(position, bytes, 3, 0);
    }
    throw OverflowException();
}

void File::unmap()
{
    this->munmap(MapBase, (size_t)LastMapSize);
}

void File::munmap(void* addr, size_t len)
{
    if (!addr) return;
#ifndef _WIN32
    ::munmap(addr, len);
#else
    UnmapViewOfFile(addr);
#endif
    LastMapStart = LastMapSize = 0;
    MapBase = NULL;
    LastMapProtection = 0;
    return;
}

#ifndef _WIN32
static int __pagesize = 0;
#endif
void* File::mmap(uint64_t position, size_t size, int prot, int flags)
{
#ifndef _WIN32
    int mflags = 0;
    if (prot & 1) mflags |= PROT_READ;
    if (prot & 2) mflags |= PROT_WRITE;
    if (prot & 4) mflags |= PROT_EXEC;
    size_t rest = 0;
    if (!__pagesize) __pagesize = sysconf(_SC_PAGE_SIZE);
    // position muss an einer pagesize aligned sein
    rest = position % __pagesize;
    if (rest) {
        // Wir müssen alignen
        size_t multiplyer = position / __pagesize;
        position = multiplyer * __pagesize;
        size += rest;
    }
    void* adr = ::mmap(NULL, size, mflags, MAP_PRIVATE, fileno((FILE*)ff), (off_t)position);
    if (adr == MAP_FAILED) {
        MapBase = NULL;
        LastMapSize = 0;
        throwErrno(errno);
        return NULL;
    }
    LastMapSize = size;
    LastMapProtection = prot;
    LastMapStart = position;
    MapBase = (char*)adr;
    return (MapBase + rest);

#else
    HANDLE hFile = (HANDLE)_get_osfhandle(fileno((FILE*)ff));
    if (hFile == INVALID_HANDLE_VALUE) throwErrno(errno);
    DWORD prot_flag = (prot & 2) ? PAGE_READWRITE : PAGE_READONLY;
    DWORD map_access = (prot & 2) ? FILE_MAP_WRITE : FILE_MAP_READ;

    // Datei muss mit GENERIC_READ | (optional) GENERIC_WRITE geöffnet sein.
    // _get_osfhandle liefert den HANDLE, aber CreateFileMapping braucht
    // einen HANDLE mit FileAccess-Rechten. Da wir über FILE* kommen,
    // ist der Handle bereits korrekt geöffnet (rb / r+b).

    ULARGE_INTEGER mapSize;
    mapSize.QuadPart = size;

    HANDLE hMap = CreateFileMapping(hFile, NULL, prot_flag, mapSize.HighPart, mapSize.LowPart, NULL);
    if (hMap == NULL) throwErrno(GetLastError());

    // Offset in High/Low DWORDs splitten (max. 4 GB pro View auf 32-bit,
    // auf 64-bit ist das kein Problem)
    ULARGE_INTEGER offset;
    offset.QuadPart = position;

    void* pView = MapViewOfFile(hMap, map_access, offset.HighPart, offset.LowPart,
                                0 // 0 = gesamte Mapping-Größe
    );
    CloseHandle(hMap); // View hält die Referenz, Handle kann zu

    if (pView == NULL) throwErrno(GetLastError());

    MapBase = (char*)pView;
    LastMapSize = size;
    LastMapProtection = prot;
    LastMapStart = position;
    return pView;
#endif
}

void File::erase()
{
    if (ff == NULL) throw FileNotOpenException();
    String Filename = filename();
    close();
    if (Filename.size() > 0) {
        remove(Filename);
    }
}

// ####################################################################
// Statische Funktionen
// ####################################################################

void File::load(ByteArray& object, const String& filename)
{
    if (filename.isEmpty()) throw IllegalArgumentException();
    File ff;
    ff.open(filename);
    char* buffer = (char*)malloc((size_t)ff.mysize + 1);
    if (!buffer) throw OutOfMemoryException();
    size_t by = ff.fread(buffer, 1, ff.mysize);
    if (by != ff.mysize) {
        free(buffer);
        throw ReadException();
    }
    buffer[by] = 0;
    object.use(buffer, by);
}

void File::load(String& object, const String& filename)
{
    if (filename.isEmpty()) throw IllegalArgumentException();
    File ff;
    ff.open(filename);
    char* buffer = (char*)malloc((size_t)ff.mysize + 1);
    if (!buffer) throw OutOfMemoryException();
    size_t by = ff.fread(buffer, 1, ff.mysize);
    if (by != ff.mysize) {
        free(buffer);
        throw ReadException();
    }
    buffer[by] = 0;
    try {
        object.set(buffer, by);
    }
    catch (...) {
        free(buffer);
        throw;
    }
    free(buffer);
}

ByteArray File::load(const String& filename)
{
    if (filename.isEmpty()) throw IllegalArgumentException();
    File ff;
    ff.open(filename);
    ByteArray ba;
    void* buffer = ba.malloc((size_t)ff.mysize);
    size_t by = ff.fread(buffer, 1, ff.mysize);
    if (by != ff.mysize) {
        throw ReadException();
    }
    return ba;
}

void File::truncate(const String& filename, uint64_t bytes)
{
    if (filename.isEmpty()) throw IllegalArgumentException();
#ifndef _WIN32
    // truncate-Funktion vorhanden
    if (::truncate((const char*)filename, (off_t)bytes) == 0) return;
    throwErrno(errno, filename);
#else
    File ff;
    ff.open(filename, FileMode::READWRITE);
    ff.truncate(bytes);
#endif
}

bool File::exists(const String& filename)
{
    if (filename.isEmpty()) throw IllegalArgumentException("filename is empty");

#ifdef _WIN32
    struct _stat buffer;
    WideString wide_filename(filename);
    return (_wstat((const wchar_t*)wide_filename, &buffer) == 0);
#else
    struct stat buffer;
    return (stat((const char*)filename, &buffer) == 0);
#endif
}

void File::copy(const String& oldfile, const String& newfile)
{
    if (oldfile == newfile) return;
    if (oldfile.isEmpty()) throw IllegalArgumentException();
    if (newfile.isEmpty()) throw IllegalArgumentException();

    File f1, f2;
    f1.open(oldfile, FileMode::READ);
    f2.open(newfile, FileMode::WRITE);
    uint64_t bsize = 1024 * 1024;
    if (f1.mysize < bsize) bsize = f1.mysize;
    void* buffer = malloc((size_t)bsize);
    if (!buffer) throw OutOfMemoryException();
    uint64_t rest = f1.mysize;
    while (rest) {
        uint64_t bytes = bsize;
        if (bytes > rest) bytes = rest;
        uint64_t done = f1.fread(buffer, 1, bytes);
        if (done != bytes) {
            // Sollte eigentlich nicht vorkommen
            f2.close();
            remove(newfile);
            free(buffer);
            throw ReadException();
        }
        done = f2.fwrite(buffer, 1, bytes);
        rest -= bytes;
    }
    f1.close();
    f2.close();
    free(buffer);
}

void File::move(const String& oldfile, const String& newfile)
{
    File::rename(oldfile, newfile);
}

void File::rename(const String& oldfile, const String& newfile)
{
    if (oldfile == newfile) return;
    if (oldfile.isEmpty()) throw IllegalArgumentException();
    if (newfile.isEmpty()) throw IllegalArgumentException();

    String desc;
    desc.setf("rename %s => %s", (const char*)oldfile, (const char*)newfile);
#ifdef _WIN32
    WideString wOld(oldfile);
    WideString wNew(newfile);
    if (::MoveFileExW((const wchar_t*)wOld, (const wchar_t*)wNew, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED) != 0) {
        return;
    }
    /* OLDCODE
    if (::_wrename((const wchar_t*)WideString(oldfile), (const wchar_t*)WideString(newfile)) == 0) {
        FILE* fd = NULL;
        // printf ("buffer=%s\n",buff);
        fd = _wfopen((const wchar_t*)WideString(oldfile), L"rb"); // Ist die alte Datei noch da?
        if (fd) {
            // Ja, wir löschen sie manuell
            fclose(fd);
            pplib::String o1 = oldfile.toLowerCase();
            pplib::String n1 = newfile.toLowerCase();
            if (n1 == o1) return;
            if (::_wunlink((const wchar_t*)WideString(oldfile)) == 0) return;
            int saveerrno = errno;
            ::_wunlink((const wchar_t*)WideString(newfile));
            errno = saveerrno;
            throwErrno(errno, desc);
        }
        return;
    }
        */
    if (errno == EXDEV) { // oldfile und newfile befinden sich nicht im gleichen Filesystem.
        copy(oldfile, newfile);
        if (::_wunlink((const wchar_t*)wOld) == 0) return;
    }
    throwErrno(errno, desc);
#else
    if (::rename((const char*)oldfile, (const char*)newfile) == 0) {
        FILE* fd = NULL;
        // printf ("buffer=%s\n",buff);
        fd = fopen((const char*)oldfile, "rb"); // Ist die alte Datei noch da?
        if (fd) {
            // Ja, wir löschen sie manuell
            fclose(fd);
            if (::unlink((const char*)oldfile) == 0) return;
            int saveerrno = errno;
            ::unlink((const char*)newfile);
            errno = saveerrno;
            throwErrno(errno, desc);
        }
        return;
    }
    if (errno == EXDEV) { // oldfile und newfile befinden sich nicht im gleichen Filesystem.
        copy(oldfile, newfile);
        if (::unlink((const char*)oldfile) == 0) return;
    }
    throwErrno(errno, desc);
#endif
}

void File::erase(const String& filename)
{
    if (filename.isEmpty()) return;
#ifdef _WIN32
    if (::_wunlink((const wchar_t*)WideString(filename)) == 0) return;
    if (errno == ENOENT) return;
    throwErrno(errno, filename);
#else
    if (::unlink((const char*)filename) == 0) return;
    if (errno == ENOENT) return;
    throwErrno(errno, filename);
#endif
}

void File::unlink(const String& filename)
{
    File::erase(filename);
}

void File::remove(const String& filename)
{
    File::erase(filename);
}

void File::touch(const String& filename)
{
    if (filename.isEmpty()) throw IllegalArgumentException();
    File ff;
    ff.open(filename, FileMode::APPEND);
    ff.close();
}

void File::save(const void* content, size_t size, const String& filename)
{
    if (filename.isEmpty()) throw IllegalArgumentException();
    File ff;
    ff.open(filename, FileMode::WRITE);
    ff.fwrite(content, 1, size);
}

void File::save(const ByteArrayPtr& object, const String& filename)
{
    if (filename.isEmpty()) throw IllegalArgumentException();
    File ff;
    ff.open(filename, FileMode::WRITE);
    ff.fwrite(object.ptr(), 1, object.size());
}

static mode_t translate_FileAttr(FileAttr::Attributes attr)
{
    mode_t m = 0;
#ifdef _WIN32
    if (attr & FileAttr::USR_READ) m |= _S_IREAD;
    if (attr & FileAttr::USR_WRITE) m |= _S_IWRITE;
    if (attr & FileAttr::GRP_READ) m |= _S_IREAD;
    if (attr & FileAttr::GRP_WRITE) m |= _S_IWRITE;
    if (attr & FileAttr::OTH_READ) m |= _S_IREAD;
    if (attr & FileAttr::OTH_WRITE) m |= _S_IWRITE;

#else
    if (attr & FileAttr::ISUID) m += S_ISUID;
    if (attr & FileAttr::ISGID) m += S_ISGID;
    if (attr & FileAttr::STICKY) m += S_ISVTX;
    if (attr & FileAttr::USR_READ) m += S_IRUSR;
    if (attr & FileAttr::USR_WRITE) m += S_IWUSR;
    if (attr & FileAttr::USR_EXECUTE) m += S_IXUSR;
    if (attr & FileAttr::GRP_READ) m += S_IRGRP;
    if (attr & FileAttr::GRP_WRITE) m += S_IWGRP;
    if (attr & FileAttr::GRP_EXECUTE) m += S_IXGRP;
    if (attr & FileAttr::OTH_READ) m += S_IROTH;
    if (attr & FileAttr::OTH_WRITE) m += S_IWOTH;
    if (attr & FileAttr::OTH_EXECUTE) m += S_IXOTH;
#endif
    return m;
}

void File::chmod(const String& filename, FileAttr::Attributes attr)
{
    if (filename.isEmpty()) throw IllegalArgumentException();
    mode_t m = translate_FileAttr(attr);
#ifdef _WIN32
    if (_wchmod((const wchar_t*)WideString(filename), m) == 0) return;
#else
    if (::chmod((const char*)filename, m) == 0) return;
#endif
    throwErrno(errno, filename);
}

String File::md5Hash(const String& filename)
{
    File ff;
    ff.open(filename);
    return ff.md5();
}

String File::sha256Hash(const String& filename)
{
    File ff;
    ff.open(filename);
    return ff.sha256();
}

#ifdef _WIN32
static void getResultFromStat(struct _stat& st, DirEntry& result, const pplib::String& filename)
#else
static void getResultFromStat(struct stat& st, DirEntry& result, const pplib::String& filename)
#endif
{
    result.ATime.setTime_t(st.st_atime);
    result.CTime.setTime_t(st.st_ctime);
    result.MTime.setTime_t(st.st_mtime);
    result.Attrib = FileAttr::NONE;
    result.Size = st.st_size;
    result.File.set(filename);
    result.Path = File::getPath(result.File);
    result.Filename = File::getFilename(result.File);
    result.AttrStr.set(L"----------");
    result.Uid = st.st_uid;
    result.Gid = st.st_gid;
#ifndef _WIN32
    result.Blocks = st.st_blocks;
    result.BlockSize = st.st_blksize;
#else
    result.Blocks = 0;
    result.BlockSize = 0;
#endif
    result.NumLinks = st.st_nlink;

    if ((st.st_mode & S_IFDIR) == S_IFDIR) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::IFDIR);
    if ((st.st_mode & S_IFREG) == S_IFREG) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::IFFILE);
#ifdef S_IFLNK
    if ((st.st_mode & S_IFLNK) == S_IFLNK) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::IFLINK);
#endif
#ifdef S_IFSOCK
    if ((st.st_mode & S_IFSOCK) == S_IFSOCK) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::IFSOCK);
#endif

#ifdef _WIN32
    if (st.st_mode & _S_IREAD) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::USR_READ);
    if (st.st_mode & _S_IWRITE) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::USR_WRITE);
    if (st.st_mode & _S_IEXEC) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::USR_EXECUTE);
#else
    if (st.st_mode & S_IRUSR) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::USR_READ);
    if (st.st_mode & S_IWUSR) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::USR_WRITE);
    if (st.st_mode & S_IXUSR) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::USR_EXECUTE);
    if (st.st_mode & S_ISUID) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::ISUID);

    if (st.st_mode & S_IRGRP) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::GRP_READ);
    if (st.st_mode & S_IWGRP) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::GRP_WRITE);
    if (st.st_mode & S_IXGRP) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::GRP_EXECUTE);
    if (st.st_mode & S_ISGID) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::ISGID);

    if (st.st_mode & S_IROTH) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::OTH_READ);
    if (st.st_mode & S_IWOTH) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::OTH_WRITE);
    if (st.st_mode & S_IXOTH) result.Attrib = (FileAttr::Attributes)(result.Attrib | FileAttr::OTH_EXECUTE);
#endif

    if (result.Attrib & FileAttr::IFLINK) result.AttrStr.set(0, 'l');
    if (result.Attrib & FileAttr::IFDIR) result.AttrStr.set(0, 'd');

    if (result.Attrib & FileAttr::USR_READ) result.AttrStr.set(1, 'r');
    if (result.Attrib & FileAttr::USR_WRITE) result.AttrStr.set(2, 'w');
    if (result.Attrib & FileAttr::USR_EXECUTE) result.AttrStr.set(3, 'x');
    if (result.Attrib & FileAttr::ISUID) result.AttrStr.set(3, 's');

    if (result.Attrib & FileAttr::GRP_READ) result.AttrStr.set(4, 'r');
    if (result.Attrib & FileAttr::GRP_WRITE) result.AttrStr.set(5, 'w');
    if (result.Attrib & FileAttr::GRP_EXECUTE) result.AttrStr.set(6, 'x');
    if (result.Attrib & FileAttr::ISGID) result.AttrStr.set(6, 's');

    if (result.Attrib & FileAttr::OTH_READ) result.AttrStr.set(7, 'r');
    if (result.Attrib & FileAttr::OTH_WRITE) result.AttrStr.set(8, 'w');
    if (result.Attrib & FileAttr::OTH_EXECUTE) result.AttrStr.set(9, 'x');
}

void File::statFile(const String& filename, DirEntry& result)
{
    if (filename.isEmpty()) throw IllegalArgumentException();
#ifdef _WIN32
    struct _stat st;
    String File = filename;
    File.replace("/", "\\");
    if (_wstat((const wchar_t*)WideString(File), &st) != 0) throwErrno(errno, filename);
#else
    struct stat st;
    if (::stat((const char*)filename, &st) != 0) throwErrno(errno, filename);
#endif
    getResultFromStat(st, result, filename);
}

DirEntry File::statFile(const String& filename)
{
    DirEntry e;
    File::statFile(filename, e);
    return e;
}

bool File::tryStatFile(const String& filename, DirEntry& result)
{
    if (filename.isEmpty()) return false;
#ifdef _WIN32
    struct _stat st;
    String File = filename;
    File.replace("/", "\\");
    if (_wstat((const wchar_t*)WideString(File), &st) != 0) return false;
#else
    struct stat st;
    if (::stat((const char*)filename, &st) != 0) return false;
#endif
    getResultFromStat(st, result, filename);
    return true;
}

String File::getPath(const String& path)
{
    size_t i, l, pos;
    l = path.len();
    pos = 0;
    for (i = 0; i < l; i++) {
        char c = path[i];
        if (c == '/' || c == ':' || c == '\\') pos = i;
    }
    return path.left(pos);
}

String File::getFilename(const String& path)
{
    size_t i, l, pos;
    l = path.len();
    pos = 0;
    for (i = 0; i < l; i++) {
        char c = path[i];
        if (c == '/' || c == ':' || c == '\\') pos = i + 1;
    }
    return path.mid(pos);
}

String File::getSuffix(const String& path)
{
    Array Token(path, ".");
    return Token.get(-1);
}

bool File::isDir(const String& filename)
{
    if (File::exists(filename)) {
        DirEntry stat;
        statFile(filename, stat);
        return stat.isDir();
    }
    return false;
}

bool File::isFile(const String& filename)
{
    if (File::exists(filename)) {
        DirEntry stat;
        statFile(filename, stat);
        return stat.isFile();
    }
    return false;
}

bool File::isLink(const String& filename)
{
    if (File::exists(filename)) {
        DirEntry stat;
        statFile(filename, stat);
        return stat.isLink();
    }
    return false;
}

bool File::isReadable(const String& filename)
{
    if (File::exists(filename)) {
        DirEntry stat;
        statFile(filename, stat);
        return stat.isReadable();
    }
    return false;
}

bool File::isWritable(const String& filename)
{
    if (File::exists(filename)) {
        DirEntry stat;
        statFile(filename, stat);
        return stat.isWritable();
    }
    return false;
}

bool File::isExecutable(const String& filename)
{
    if (File::exists(filename)) {
        DirEntry stat;
        statFile(filename, stat);
        return stat.isExecutable();
    }
    return false;
}

} // end of namespace pplib
