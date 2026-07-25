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

#ifndef PPL7_CORE_DIR_H_
#define PPL7_CORE_DIR_H_

#include <ppl7/types/string.h>
#include <ppl7/types/bytearray.h>
#include <ppl7/types/bytearrayptr.h>
#include <ppl7/types/datetime.h>
#include <ppl7/types/assocarray.h>
#include <ppl7/core/fileobject.h>
#include <ppl7/core/file.h>

namespace ppl7
{
class DirEntry
{
public:
    DirEntry();
    DirEntry(const DirEntry& other);
    String Filename;
    String Path;
    String File;
    uint64_t Size;
    FileAttr::Attributes Attrib;
    uint32_t Uid;
    uint32_t Gid;
    uint32_t Blocks;
    uint32_t BlockSize;
    uint32_t NumLinks;
    String AttrStr;
    DateTime ATime, CTime, MTime;
    bool isDir();
    bool isFile();
    bool isLink();
    bool isReadable();
    bool isWritable();
    bool isExecutable();
    DirEntry& operator=(const DirEntry& other);
    void toArray(AssocArray& a) const;
    void print(const char* label = NULL);
};

// TODO: std containers verwenden!
class Dir
{
public:
    enum Sort
    {
        SORT_NONE,
        SORT_FILENAME,
        SORT_FILENAME_IGNORCASE,
        SORT_ATIME,
        SORT_CTIME,
        SORT_MTIME,
        SORT_SIZE,
    };

private:
    ppl7::List<DirEntry> Files;
    ppl7::List<const DirEntry*> SortedFiles;
    Sort sort;
    String Path;

    void resortMTime();
    void resortCTime();
    void resortATime();
    void resortSize();
    void resortFilename();
    void resortFilenameIgnoreCase();
    void resortNone();

public:
    PPL7EXCEPTION(PathnameTooLongException, Exception);
    PPL7EXCEPTION(NonexistingPathException, Exception);
    PPL7EXCEPTION(PermissionDeniedException, Exception);
    PPL7EXCEPTION(CreateDirectoryFailedException, Exception);

    typedef ppl7::List<const DirEntry*>::Iterator Iterator;

    Dir();
    Dir(const char* path, Sort s = SORT_NONE);
    Dir(const String& path, Sort s = SORT_NONE);
    ~Dir();
    void open(const char* path, Sort s = SORT_NONE);
    void open(const String& path, Sort s = SORT_NONE);
    bool tryOpen(const String& path, Sort s = SORT_NONE);
    void resort(Sort s);
    void clear();
    size_t num() const;
    size_t count() const;
    void reset(Iterator& it) const;
    const DirEntry& getFirst(Iterator& it) const;
    const DirEntry& getNext(Iterator& it) const;
    const DirEntry& getFirstPattern(Iterator& it, const String& pattern, bool ignorecase = false) const;
    const DirEntry& getNextPattern(Iterator& it, const String& pattern, bool ignorecase = false) const;
    const DirEntry& getFirstRegExp(Iterator& it, const String& regexp) const;
    const DirEntry& getNextRegExp(Iterator& it, const String& regexp) const;

    bool getFirst(DirEntry& e, Iterator& it) const;
    bool getNext(DirEntry& e, Iterator& it) const;
    bool getFirstPattern(DirEntry& e, Iterator& it, const String& pattern, bool ignorecase = false) const;
    bool getNextPattern(DirEntry& e, Iterator& it, const String& pattern, bool ignorecase = false) const;
    bool getFirstRegExp(DirEntry& e, Iterator& it, const String& regexp) const;
    bool getNextRegExp(DirEntry& e, Iterator& it, const String& regexp) const;

    void print() const;
    void print(const DirEntry& de) const;
    static String currentPath();
    static String homePath();
    static String tempPath();
    static String applicationDataPath();
    static String applicationDataPath(const String& company, const String& application);
    static String documentsPath();
    static String documentsPath(const String& company, const String& application);

    static bool exists(const String& dirname);
    static bool canOpen(const String& path);
    static void mkDir(const String& path);
    static void mkDir(const String& path, bool recursive);
    static void mkDir(const String& path, mode_t mode, bool recursive);
};

} // namespace ppl7

#endif /* PPL7_CORE_DIR_H_ */