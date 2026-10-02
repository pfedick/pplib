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

#include <string.h>
#include <pplib/core/fileobject.h>
#include <pplib/core/file.h>
#include <pplib/core/gzfile.h>
#include <pplib/exceptions.h>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

namespace pplib
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

GzFile::GzFile()
{
    ff = NULL;
    fh = NULL;
}

GzFile::GzFile(const String& filename, File::FileMode mode)
{
    ff = NULL;
    fh = NULL;
    open(filename, mode);
}

GzFile::GzFile(int fd)
{
    ff = NULL;
    fh = NULL;
    open(fd);
}

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

void GzFile::throwErrno(int e, const String& filename)
{
    throwExceptionFromErrno(e, filename);
}

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

void GzFile::close()
{
    setFilename("");
    if (ff != NULL) {
        int ret = gzclose((gzFile)ff);
        ff = NULL;
        if (ret != Z_OK) {
            if (ret == Z_ERRNO)
                throwErrno(errno, filename());
            else if (ret == Z_MEM_ERROR)
                throw pplib::OutOfMemoryException();
            throw pplib::CompressionFailedException();
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
        throw pplib::UnsupportedFeatureException("GzFile::SEEKEND");
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
    if (by == 0) throw pplib::EndOfFileException();
    int err = 0;
    const char* msg = gzerror((gzFile)ff, &err);
    throw pplib::CompressionFailedException("gzread: %s [%d]", msg, err);
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
            throw pplib::EndOfFileException();
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
    throw pplib::EndOfFileException();
}

size_t GzFile::fwrite(const void* ptr, size_t size, size_t nmemb)
{
    if (ff == NULL) throw FileNotOpenException();
    if (ptr == NULL) throw IllegalArgumentException();
    int by = ::gzwrite((gzFile)ff, ptr, (unsigned int)(size * nmemb));
    if (by > 0) return by;
    if (by == 0) throw pplib::EndOfFileException();
    int err = 0;
    const char* msg = gzerror((gzFile)ff, &err);
    throw pplib::CompressionFailedException("gzread: %s [%d]", msg, err);
}

} // end of namespace pplib
