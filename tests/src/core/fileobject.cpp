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
 *       documentation/ intellectual property materials provided with the distribution.
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

#include <gtest/gtest.h>

#include <pplib/core/file.h>
#include <pplib/core/memfile.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/exceptions.h>

#include "pplib-tests.h"

namespace
{

class FileObjectTest : public ::testing::Test
{
protected:
    FileObjectTest()
    {
    }
    virtual ~FileObjectTest()
    {
    }
};

TEST_F(FileObjectTest, WriteEmptyByteArrayPtrReturnsZero)
{
    pplib::MemFile f;
    pplib::ByteArrayPtr emptyPtr;
    ASSERT_EQ((size_t)0, f.write(emptyPtr));
    ASSERT_EQ((uint64_t)0, f.size());
}

TEST_F(FileObjectTest, ReadIntoByteArrayPreservesTargetWhenNotOpen)
{
    pplib::MemFile f;
    f.close();
    pplib::ByteArray target("Original Content 12345");
    size_t origSize = target.size();

    ASSERT_THROW(f.read(target, 50), pplib::FileNotOpenException);
    ASSERT_EQ(origSize, target.size());
    ASSERT_STREQ("Original Content 12345", (const char*)target.ptr());
}

TEST_F(FileObjectTest, CopyFromSelfThrowsIllegalArgumentException)
{
    const char data[] = "1234567890abcdef";
    pplib::MemFile f((void*)data, sizeof(data) - 1, false);

    ASSERT_THROW(f.copyFrom(f, 0, 5, 10), pplib::IllegalArgumentException);
    ASSERT_THROW(f.copyFrom(f, 5), pplib::IllegalArgumentException);
}

TEST_F(FileObjectTest, CopyFromAvailableUnderflowHandledGracefully)
{
    const char data[] = "Hello World";
    pplib::MemFile src((void*)data, 11, false);
    src.seek(11); // at EOF

    pplib::MemFile dst;
    uint64_t copied = dst.copyFrom(src, 20);
    ASSERT_EQ((uint64_t)0, copied);
}

TEST_F(FileObjectTest, CopyFromPartial)
{
    const char data[] = "0123456789";
    pplib::MemFile src((void*)data, 10, false);
    pplib::MemFile dst;

    uint64_t copied = dst.copyFrom(src, 2, 5, 0);
    ASSERT_EQ((uint64_t)5, copied);
    ASSERT_EQ((uint64_t)5, dst.size());

    pplib::ByteArray ba;
    dst.load(ba);
    ASSERT_EQ(0, memcmp(ba.ptr(), "23456", 5));
}

TEST_F(FileObjectTest, GetwsNoDoubleFree)
{
    wchar_t text[] = L"Test Zeile 1\nTest Zeile 2\n";
    pplib::MemFile f((void*)text, sizeof(text) - sizeof(wchar_t), false);

    pplib::WideString ws;
    int res = f.getws(ws, 100);
    ASSERT_EQ(1, res);
    ASSERT_EQ(pplib::WideString(L"Test Zeile 1\n"), ws);
}

TEST_F(FileObjectTest, FilenameHandling)
{
    pplib::MemFile f;
    EXPECT_TRUE(f.filename().isEmpty());

    f.setFilename("my_test_file.dat");
    EXPECT_EQ(pplib::String("my_test_file.dat"), f.filename());

    f.setFilename(pplib::String("another_name.txt"));
    EXPECT_EQ(pplib::String("another_name.txt"), f.filename());

    // Setting nullptr clears filename
    f.setFilename(static_cast<const char*>(nullptr));
    EXPECT_TRUE(f.filename().isEmpty());
}

TEST_F(FileObjectTest, WriteOverloads)
{
    pplib::MemFile f;

    // write(const void*, size_t)
    EXPECT_EQ(5u, f.write("01234", 5));
    EXPECT_EQ(5u, f.size());

    // write(const void*, size_t, uint64_t fileposition)
    EXPECT_EQ(3u, f.write("ABC", 3, 1));
    EXPECT_EQ(4u, f.tell());
    EXPECT_EQ(5u, f.size());

    char buf[6] = {0};
    f.seek(0);
    f.read(buf, 5);
    EXPECT_STREQ("0ABC4", buf);

    // write(const ByteArrayPtr&, size_t)
    pplib::ByteArray ba("XYZ123", 6);

    // bytes == 0 with non-empty -> writes all
    EXPECT_EQ(6u, f.write(ba, 0));

    // bytes > object.size() -> clamped to object.size()
    EXPECT_EQ(6u, f.write(ba, 100));

    // bytes < object.size() -> writes exact count
    EXPECT_EQ(3u, f.write(ba, 3));

    // empty ByteArrayPtr -> returns 0
    pplib::ByteArrayPtr emptyPtr;
    EXPECT_EQ(0u, f.write(emptyPtr, 0));
    EXPECT_EQ(0u, f.write(emptyPtr, 10));
}

TEST_F(FileObjectTest, ReadOverloads)
{
    const char data[] = "0123456789ABCDEF";
    pplib::MemFile f((void*)data, sizeof(data) - 1, false);

    // read(void*, size_t)
    char buf1[5] = {0};
    EXPECT_EQ(4u, f.read(buf1, 4));
    EXPECT_STREQ("0123", buf1);
    EXPECT_EQ(4u, f.tell());

    // read(void*, size_t, uint64_t fileposition)
    char buf2[5] = {0};
    EXPECT_EQ(4u, f.read(buf2, 4, 10));
    EXPECT_STREQ("ABCD", buf2);
    EXPECT_EQ(14u, f.tell());

    // read(ByteArray&, size_t)
    f.seek(0);
    pplib::ByteArray baTarget;
    EXPECT_EQ(6u, f.read(baTarget, 6));
    EXPECT_EQ(6u, baTarget.size());
    EXPECT_EQ(0, memcmp("012345", baTarget.ptr(), 6));

    // read with bytes == 0 throws IllegalArgumentException
    EXPECT_THROW(f.read(baTarget, 0), pplib::IllegalArgumentException);
}

// Mocks for edge-case coverage
class EofOnReadMockFile : public pplib::FileObject
{
private:
    bool called{false};

public:
    bool isOpen() const override
    {
        return true;
    }
    uint64_t size() const override
    {
        return 100;
    }
    uint64_t tell() override
    {
        return 0;
    }
    void seek(uint64_t) override
    {
    }
    size_t fread(void* ptr, size_t size, size_t nmemb) override
    {
        if (called) throw pplib::EndOfFileException();
        called = true;
        memcpy(ptr, "ABCDE", 5);
        return 5;
    }
};

class LargeMockFile : public pplib::FileObject
{
public:
    bool isOpen() const override
    {
        return true;
    }
    uint64_t size() const override
    {
        return 2 * 1024 * 1024;
    }
    uint64_t tell() override
    {
        return 0;
    }
    void seek(uint64_t) override
    {
    }
    size_t fread(void* ptr, size_t size, size_t nmemb) override
    {
        return nmemb * size;
    }
};

class DummySinkFile : public pplib::FileObject
{
public:
    bool isOpen() const override
    {
        return true;
    }
    size_t fwrite(const void* ptr, size_t size, size_t nmemb) override
    {
        return nmemb * size;
    }
};

TEST_F(FileObjectTest, CopyFromChunkingAndExceptions)
{
    // Test copyFrom when quellfile throws EndOfFileException during loop
    EofOnReadMockFile eofSrc;
    pplib::MemFile dst1;
    uint64_t copied1 = dst1.copyFrom(eofSrc, 100);
    EXPECT_EQ(5u, copied1);

    // Test copyFrom when copying more than COPYBYTES_BUFFERSIZE (1 MB)
    LargeMockFile largeSrc;
    DummySinkFile sink;
    uint64_t copied2 = sink.copyFrom(largeSrc, 2 * 1024 * 1024);
    EXPECT_EQ((uint64_t)(2 * 1024 * 1024), copied2);
}

class MockStringIoFile : public pplib::FileObject
{
private:
    int getsCount{0};
    int getwsCount{0};

public:
    bool isOpen() const override
    {
        return true;
    }

    char* fgets(char* buffer, size_t num) override
    {
        if (getsCount == 0) {
            getsCount++;
            strncpy(buffer, "LineOne\n", num);
            return buffer;
        } else if (getsCount == 1) {
            getsCount++;
            strncpy(buffer, "LineTwo\n", num);
            return buffer;
        }
        return nullptr; // EOF
    }

    wchar_t* fgetws(wchar_t* buffer, size_t num) override
    {
        if (getwsCount == 0) {
            getwsCount++;
            wcsncpy(buffer, L"WideOne\n", num);
            return buffer;
        } else if (getwsCount == 1) {
            getwsCount++;
            wcsncpy(buffer, L"WideTwo\n", num);
            return buffer;
        }
        return nullptr; // EOF
    }
};

TEST_F(FileObjectTest, GetsAndGetwsOverloads)
{
    MockStringIoFile f;

    // gets(String&, size_t)
    pplib::String line;
    EXPECT_THROW(f.gets(line, 0), pplib::IllegalArgumentException);

    EXPECT_EQ(1, f.gets(line, 100));
    EXPECT_EQ(pplib::String("LineOne\n"), line);

    // gets(size_t)
    EXPECT_EQ(pplib::String("LineTwo\n"), f.gets(100));

    // at EOF: gets(String&, size_t) returns 0
    EXPECT_EQ(0, f.gets(line, 100));

    // at EOF: gets(size_t) throws EndOfFileException
    EXPECT_THROW(f.gets(100), pplib::EndOfFileException);

    // WideString gets
    MockStringIoFile wf;

    pplib::WideString wline;
    EXPECT_THROW(wf.getws(wline, 0), pplib::IllegalArgumentException);

    EXPECT_EQ(1, wf.getws(wline, 100));
    EXPECT_EQ(pplib::WideString(L"WideOne\n"), wline);

    EXPECT_EQ(pplib::WideString(L"WideTwo\n"), wf.getws(100));

    // at EOF
    EXPECT_EQ(0, wf.getws(wline, 100));
    EXPECT_THROW(wf.getws(100), pplib::EndOfFileException);
}

TEST_F(FileObjectTest, PutsOverloads)
{
    pplib::MemFile f;

    // putsf
    EXPECT_THROW(f.putsf(nullptr), pplib::IllegalArgumentException);
    f.putsf("Value: %d, String: %s\n", 42, "hello");

    // puts
    f.puts(pplib::String("Second Line\n"));

    // putws
    f.putws(pplib::WideString(L"Third Wide Line\n"));

    EXPECT_GT(f.size(), 30u);
}

TEST_F(FileObjectTest, MapParameterless)
{
    const char sample[] = "MappedDataBuffer";
    pplib::MemFile f((void*)sample, sizeof(sample), false);

    pplib::ByteArrayPtr mapped = f.map();
    EXPECT_EQ(sizeof(sample), mapped.size());
    EXPECT_EQ(0, memcmp(sample, mapped.ptr(), sizeof(sample)));
}

class PipeMockFile : public pplib::FileObject
{
private:
    std::string data;
    size_t pos{0};
    bool throwEofOnFinish{false};

public:
    PipeMockFile(const std::string& d, bool throwEof = false)
        : data(d),
          throwEofOnFinish(throwEof)
    {
    }
    bool isOpen() const override
    {
        return true;
    }
    void seek(uint64_t) override
    {
        throw pplib::IllegalOperationOnPipeException();
    }
    size_t fread(void* ptr, size_t size, size_t nmemb) override
    {
        if (pos >= data.size()) {
            if (throwEofOnFinish) throw pplib::EndOfFileException();
            return 0;
        }
        size_t toRead = std::min(nmemb * size, data.size() - pos);
        memcpy(ptr, data.data() + pos, toRead);
        pos += toRead;
        return toRead;
    }
};

class TruncatedReadMockFile : public pplib::FileObject
{
private:
    bool throwEof{false};
    bool called{false};

public:
    TruncatedReadMockFile(bool throwEof)
        : throwEof(throwEof)
    {
    }
    bool isOpen() const override
    {
        return true;
    }
    uint64_t size() const override
    {
        return 10;
    }
    void seek(uint64_t) override
    {
    }
    size_t fread(void* ptr, size_t size, size_t nmemb) override
    {
        if (called) {
            if (throwEof) throw pplib::EndOfFileException();
            return 0;
        }
        called = true;
        memcpy(ptr, "12345", 5);
        return 5;
    }
};

TEST_F(FileObjectTest, LoadOverloads)
{
    // load(ByteArray&) on empty open file
    char dummy = 0;
    pplib::MemFile emptyFile((void*)&dummy, 0, false);
    pplib::ByteArray baTarget("WillBeCleared");
    EXPECT_EQ(0u, emptyFile.load(baTarget));
    EXPECT_EQ(0u, baTarget.size());

    // load(ByteArray&) on closed file throws
    pplib::MemFile closedFile;
    closedFile.close();
    EXPECT_THROW(closedFile.load(baTarget), pplib::FileNotOpenException);

    // load() parameterless
    const char content[] = "FileObjectLoadContent";
    pplib::MemFile contentFile((void*)content, sizeof(content), false);
    pplib::ByteArray loaded = contentFile.load();
    EXPECT_EQ(sizeof(content), loaded.size());
    EXPECT_EQ(0, memcmp(content, loaded.ptr(), sizeof(content)));

    // Pipe reading with 0 return at EOF
    std::string pipeStr = "PipeStreamDataPayload";
    PipeMockFile pipeFile1(pipeStr, false);
    pplib::ByteArray pipeBa1;
    EXPECT_EQ(pipeStr.size(), pipeFile1.load(pipeBa1));
    EXPECT_EQ(pipeStr.size(), pipeBa1.size());
    EXPECT_EQ(0, memcmp(pipeStr.data(), pipeBa1.ptr(), pipeStr.size()));

    // Pipe reading with EndOfFileException at EOF
    PipeMockFile pipeFile2(pipeStr, true);
    pplib::ByteArray pipeBa2;
    EXPECT_EQ(pipeStr.size(), pipeFile2.load(pipeBa2));
    EXPECT_EQ(pipeStr.size(), pipeBa2.size());

    // Pipe reading > 4096 bytes (looping through chunk read)
    std::string longPipeStr(10000, 'x');
    PipeMockFile pipeFile3(longPipeStr, false);
    pplib::ByteArray pipeBa3;
    EXPECT_EQ(10000u, pipeFile3.load(pipeBa3));
    EXPECT_EQ(10000u, pipeBa3.size());

    // Non-pipe where fread reads less than size()
    TruncatedReadMockFile truncFile1(false);
    pplib::ByteArray truncBa1;
    EXPECT_EQ(5u, truncFile1.load(truncBa1));
    EXPECT_EQ(5u, truncBa1.size());
    EXPECT_EQ(0, memcmp("12345", truncBa1.ptr(), 5));

    // Non-pipe where fread throws EndOfFileException
    TruncatedReadMockFile truncFile2(true);
    pplib::ByteArray truncBa2;
    EXPECT_EQ(5u, truncFile2.load(truncBa2));
    EXPECT_EQ(5u, truncBa2.size());
}

TEST_F(FileObjectTest, HashFunctions)
{
    pplib::MemFile f((void*)loremipsum, strlen(loremipsum), false);
    EXPECT_EQ(pplib::String(loremipsum_md5), f.md5());

    const char abc[] = "abc";
    pplib::MemFile fAbc((void*)abc, 3, false);
    EXPECT_EQ(pplib::String("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"), fAbc.sha256());
}

TEST_F(FileObjectTest, DefaultVirtualFunctionsThrow)
{
    pplib::FileObject fo;
    char buf[10];
    wchar_t wbuf[10];

    EXPECT_THROW(fo.close(), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.rewind(), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.seek(0), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.seek(0, pplib::FileObject::SEEKSET), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.tell(), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.fread(buf, 1, 1), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.fwrite(buf, 1, 1), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.fgets(buf, 10), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.fgetws(wbuf, 10), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.fputs("test"), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.fputws(L"test"), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.fputc('a'), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.fgetc(), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.fputwc(L'a'), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.fgetwc(), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.eof(), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.size(), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.getFileNo(), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.flush(), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.sync(), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.truncate(0), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.isOpen(), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.lockShared(), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.lockExclusive(), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.unlock(), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.setMapReadAhead(0), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.map(0, 0), pplib::UnimplementedVirtualFunctionException);
    EXPECT_THROW(fo.unmap(), pplib::UnimplementedVirtualFunctionException);
}

} // namespace
