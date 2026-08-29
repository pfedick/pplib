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

#include <pplib/core/fileobject.h>
#include <pplib/exceptions.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/types/bytearray.h>
#include <pplib/core/functions.h>

namespace pplib
{

#define COPYBYTES_BUFFERSIZE 1024 * 1024

FileObject::FileObject()
{
    buffer = NULL;
}

FileObject::~FileObject()
{
    if (buffer) {
        free(buffer);
        buffer = NULL;
    }
}

void FileObject::setFilename(const char* filename)
{
    if (!filename) {
        MyFilename.clear();
        return;
    }
    MyFilename = filename;
}

void FileObject::setFilename(const String& filename)
{
    MyFilename = filename;
}

const String& FileObject::filename() const
{
    return MyFilename;
}

size_t FileObject::write(const void* source, size_t bytes, uint64_t fileposition)
{
    seek(fileposition);
    return fwrite(source, 1, bytes);
}

size_t FileObject::write(const void* source, size_t bytes)
{
    return fwrite(source, 1, bytes);
}

size_t FileObject::write(const ByteArrayPtr& object, size_t bytes)
{
    if (bytes == 0 || bytes > object.size()) bytes = object.size();
    return fwrite(object.ptr(), 1, bytes);
}

size_t FileObject::read(void* target, size_t bytes, uint64_t fileposition)
{
    seek(fileposition);
    return fread(target, 1, bytes);
}

size_t FileObject::read(void* target, size_t bytes)
{
    return fread(target, 1, bytes);
}

size_t FileObject::read(ByteArray& target, size_t bytes)
{
    if (!bytes) throw IllegalArgumentException();
    if (target.size() < bytes) target.realloc(bytes);
    return fread((void*)target.ptr(), 1, bytes);
}

uint64_t FileObject::copyFrom(FileObject& quellfile, uint64_t quelloffset, uint64_t bytes, uint64_t zieloffset)
{
    quellfile.seek(quelloffset);
    seek(zieloffset);
    return FileObject::copyFrom(quellfile, bytes);
}

uint64_t FileObject::copyFrom(FileObject& quellfile, uint64_t bytes)
{
    if (buffer == NULL) {
        buffer = (char*)malloc(COPYBYTES_BUFFERSIZE);
        if (buffer == NULL) throw OutOfMemoryException();
    }
    if (quellfile.size() > quellfile.tell()) {
        if ((quellfile.tell() + (uint64_t)bytes) > quellfile.size()) {
            bytes = quellfile.size() - quellfile.tell();
        }
        uint64_t rest = bytes;
        while (rest > 0) {
            uint64_t by = rest;
            if (by > COPYBYTES_BUFFERSIZE) by = COPYBYTES_BUFFERSIZE;
            by = quellfile.read(buffer, (size_t)by);
            write(buffer, (size_t)by);
            rest -= by;
        }
    }
    return bytes;
}

int FileObject::gets(String& buffer, size_t num)
{
    if (!num) throw IllegalArgumentException();
    char* b = (char*)malloc(num + 1);
    if (!b) throw OutOfMemoryException();
    char* ret;
    try {
        ret = fgets(b, num);
    }
    catch (...) {
        free(b);
        throw;
    }
    if (ret == NULL) {
        free(b);
        return 0;
    }
    buffer.set(b);
    free(b);
    return 1;
}

String FileObject::gets(size_t num)
{
    String s;
    if (!gets(s, num)) throw EndOfFileException();
    return s;
}

int FileObject::getws(String& buffer, size_t num)
{
    if (!num) throw IllegalArgumentException();
    wchar_t* b = (wchar_t*)malloc((num + 1) * sizeof(wchar_t));
    if (!b) throw OutOfMemoryException();
    wchar_t* ret;
    try {
        ret = fgetws(b, num);
    }
    catch (...) {
        free(b);
        throw;
    }
    if (ret == NULL) {
        free(b);
        return 0;
    }
    buffer.set(b);
    free(b);
    return 1;
}

String FileObject::getws(size_t num)
{
    String s;
    if (!getws(s, num)) throw EndOfFileException();
    return s;
}

void FileObject::putsf(const char* fmt, ...)
{
    if (!fmt) throw IllegalArgumentException();
    String str;
    va_list args;
    va_start(args, fmt);
    str.vasprintf(fmt, args);
    va_end(args);
    fputs(str);
}

void FileObject::puts(const String& str)
{
    return fputs((const char*)str);
}

void FileObject::putws(const WideString& str)
{
    return fputws((const wchar_t*)str);
}

const char* FileObject::map()
{
    return map(0, (size_t)size());
}

char* FileObject::load()
{
    uint64_t s = size();
    char* b = (char*)malloc((size_t)s + 1);
    if (!b) throw OutOfMemoryException();
    uint64_t r = 0;
    try {
        r = read(b, (size_t)s, 0);
    }
    catch (...) {
        free(b);
        throw;
    }
    if (r != s) {
        free(b);
        return NULL;
    }
    b[s] = 0;
    return b;
}

int FileObject::load(ByteArray& object)
{
    if (!isOpen()) throw FileNotOpenException();
    uint64_t mysize = size();
    seek(0);
    char* buffer = (char*)malloc((size_t)mysize + 1);
    if (!buffer) throw OutOfMemoryException();
    size_t by = 0;
    try {
        by = fread(buffer, 1, (size_t)mysize);
    }
    catch (...) {
        free(buffer);
        throw;
    }
    if (by != mysize) {
        free(buffer);
        return 0;
    }
    buffer[by] = 0;
    object.clear();
    object.use(buffer, mysize);
    return 1;
}

// Virtuelle Funktionen

void FileObject::close()
{
    throw UnimplementedVirtualFunctionException();
}

void FileObject::rewind()
{
    throw UnimplementedVirtualFunctionException();
}

void FileObject::seek(uint64_t position)
{
    throw UnimplementedVirtualFunctionException();
}

uint64_t FileObject::seek(int64_t offset, SeekOrigin origin)
{
    throw UnimplementedVirtualFunctionException();
}

uint64_t FileObject::tell()
{
    throw UnimplementedVirtualFunctionException();
}

size_t FileObject::fread(void* ptr, size_t size, size_t nmemb)
{
    throw UnimplementedVirtualFunctionException();
}

size_t FileObject::fwrite(const void* ptr, size_t size, size_t nmemb)
{
    throw UnimplementedVirtualFunctionException();
}

char* FileObject::fgets(char* buffer, size_t num)
{
    throw UnimplementedVirtualFunctionException();
}

wchar_t* FileObject::fgetws(wchar_t* buffer, size_t num)
{
    throw UnimplementedVirtualFunctionException();
}

void FileObject::fputs(const char* str)
{
    throw UnimplementedVirtualFunctionException();
}

void FileObject::fputws(const wchar_t* str)
{
    throw UnimplementedVirtualFunctionException();
}

void FileObject::fputc(int c)
{
    throw UnimplementedVirtualFunctionException();
}

int FileObject::fgetc()
{
    throw UnimplementedVirtualFunctionException();
}

void FileObject::fputwc(wchar_t c)
{
    throw UnimplementedVirtualFunctionException();
}

wchar_t FileObject::fgetwc()
{
    throw UnimplementedVirtualFunctionException();
}

bool FileObject::eof() const
{
    throw UnimplementedVirtualFunctionException();
}

uint64_t FileObject::size() const
{
    throw UnimplementedVirtualFunctionException();
}

int FileObject::getFileNo() const
{
    throw UnimplementedVirtualFunctionException();
}

void FileObject::flush()
{
    throw UnimplementedVirtualFunctionException();
}

void FileObject::sync()
{
    throw UnimplementedVirtualFunctionException();
}

void FileObject::truncate(uint64_t length)
{
    throw UnimplementedVirtualFunctionException();
}

bool FileObject::isOpen() const
{
    throw UnimplementedVirtualFunctionException();
}

void FileObject::lockShared(bool block)
{
    throw UnimplementedVirtualFunctionException();
}

void FileObject::lockExclusive(bool block)
{
    throw UnimplementedVirtualFunctionException();
}

void FileObject::unlock()
{
    throw UnimplementedVirtualFunctionException();
}

void FileObject::setMapReadAhead(size_t bytes)
{
    throw UnimplementedVirtualFunctionException();
}

const char* FileObject::map(uint64_t position, size_t size)
{
    throw UnimplementedVirtualFunctionException();
}

char* FileObject::mapRW(uint64_t position, size_t size)
{
    throw UnimplementedVirtualFunctionException();
}

void FileObject::unmap()
{
    throw UnimplementedVirtualFunctionException();
}

} // end of namespace pplib
