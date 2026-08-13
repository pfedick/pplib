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

#ifndef PPLIB_CORE_FILE_H_
#define PPLIB_CORE_FILE_H_

#include <pplib/types/string.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/core/fileobject.h>

namespace pplib
{
class DirEntry;

class FileAttr
{
public:
    enum Attributes
    {
        IFFILE = 0x10000,
        IFSOCK = 0x20000,
        IFDIR = 0x40000,
        IFLINK = 0x80000,
        ISUID = 0x4000,
        ISGID = 0x2000,
        ISVTX = 0x1000,
        STICKY = 0x1000,
        USR_READ = 0x0400,
        USR_WRITE = 0x0200,
        USR_EXECUTE = 0x0100,
        GRP_READ = 0x0040,
        GRP_WRITE = 0x0020,
        GRP_EXECUTE = 0x0010,
        OTH_READ = 0x0004,
        OTH_WRITE = 0x0002,
        OTH_EXECUTE = 0x0001,
        CHMOD_755 = 0x0755,
        CHMOD_644 = 0x0644,
        NONE = 0
    };
};

class File : public FileObject
{
private:
    void* ff;
    char* MapBase;
    uint64_t LastMapStart;
    uint64_t LastMapSize;
    int LastMapProtection;
    uint64_t ReadAhead;
    uint64_t mysize;
    uint64_t pos;
    bool isPopen;

    int munmap(void* addr, size_t len);
    void* mmap(uint64_t position, size_t size, int prot, int flags);

public:
    enum class FileMode
    {
        READ = 1,
        WRITE,
        READWRITE,
        APPEND,
    };

private:
    void throwErrno(int e);

public:
    static void throwErrno(int e, const String& filename);

    File();
    File(const String& filename, FileMode mode = FileMode::READ);
    File(FILE* handle);
    virtual ~File();

    void open(const String& filename, FileMode mode = FileMode::READ);
    void open(FILE* handle);
    void openTemp(const String& filetemplate);
    void popen(const String& command, FileMode mode = FileMode::READ);
    void erase();

    // Virtuelle Funktionen
    virtual void close();
    virtual void rewind();
    virtual void seek(uint64_t position);
    virtual uint64_t seek(int64_t offset, SeekOrigin origin);
    virtual uint64_t tell();
    virtual uint64_t size() const;
    virtual bool isOpen() const;
    virtual size_t fread(void* ptr, size_t size, size_t nmemb);
    virtual size_t fwrite(const void* ptr, size_t size, size_t nmemb);
    virtual char* fgets(char* buffer, size_t num);
    virtual wchar_t* fgetws(wchar_t* buffer, size_t num = 1024);
    virtual void fputs(const char* str);
    virtual void fputws(const wchar_t* str);
    virtual void fputc(int c);
    virtual int fgetc();
    virtual void fputwc(wchar_t c);
    virtual wchar_t fgetwc();
    virtual bool eof() const;
    virtual int getFileNo() const;
    virtual void flush();
    virtual void sync();
    virtual void truncate(uint64_t length);
    virtual void lockShared(bool block = true);
    virtual void lockExclusive(bool block = true);
    virtual void unlock();
    virtual void setMapReadAhead(size_t bytes);
    virtual const char* map(uint64_t position, size_t size);
    virtual char* mapRW(uint64_t position, size_t size);
    virtual void unmap();

    // Static Functions
    static void load(ByteArray& object, const String& filename);
    static void load(String& object, const String& filename);
    // static void* load(const String& filename, size_t* size = NULL);
    static ByteArray load(const String& filename);
    static void truncate(const String& filename, uint64_t bytes);
    static bool exists(const String& filename);
    static void copy(const String& oldfile, const String& newfile);
    static void move(const String& oldfile, const String& newfile);
    static void rename(const String& oldfile, const String& newfile);

    static void unlink(const String& filename);
    static void remove(const String& filename);
    static void erase(const String& filename);
    static void touch(const String& filename);
    static void save(const void* content, size_t size, const String& filename);
    static void save(const ByteArrayPtr& object, const String& filename);
    static void chmod(const String& filename, FileAttr::Attributes attr);
    static void statFile(const String& filename, DirEntry& result);
    static bool tryStatFile(const String& filename, DirEntry& result);
    static DirEntry statFile(const String& filename);
    static String getPath(const String& path);
    static String getFilename(const String& path);
    static String getSuffix(const String& path);
    static String md5Hash(const String& filename);
    static bool isDir(const String& filename);
    static bool isFile(const String& filename);
    static bool isLink(const String& filename);
    static bool isReadable(const String& filename);
    static bool isWritable(const String& filename);
    static bool isExecutable(const String& filename);
};

} // namespace pplib

#endif /* PPLIB_CORE_MEMFILE_H_ */