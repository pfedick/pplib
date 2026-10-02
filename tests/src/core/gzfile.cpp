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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include "pplib-tests.h"

#include <pplib/types/string.h>
// #include <pplib/types/bytearray.h>

#include <pplib/core/gzfile.h>
#include <pplib/core/dir.h>
#include <pplib/core/functions.h>
#include <pplib/exceptions.h>

namespace
{

// The fixture for testing class Foo.
class GzFileTest : public ::testing::Test
{
protected:
    GzFileTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~GzFileTest()
    {
    }
};

TEST_F(GzFileTest, ConstructorSimple)
{
    ASSERT_NO_THROW({
        pplib::GzFile f1;
        ASSERT_FALSE(f1.isOpen()) << "File seems to be open, but it shouldn't";
    });
}

TEST_F(GzFileTest, openNonexisting)
{
    pplib::GzFile f1;
    ASSERT_THROW(f1.open("nonexisting.txt"), pplib::FileNotFoundException);
}

TEST_F(GzFileTest, openExistingUncompressed)
{
    pplib::GzFile f1;
    f1.open("testdata/compression.txt");
}

TEST_F(GzFileTest, openExistingCompressed)
{
    pplib::GzFile f1;
    f1.open("testdata/compression.txt.gz");
}

TEST_F(GzFileTest, sizeThrowsUnimplementedVirtualFunctionException)
{
    pplib::GzFile f1;
    f1.open("testdata/compression.txt");
    ASSERT_THROW(f1.size(), pplib::UnimplementedVirtualFunctionException);
}

TEST_F(GzFileTest, fread1024_UncompressedFile)
{
    pplib::GzFile f1;
    pplib::ByteArray ba;
    ba.malloc(1024);
    f1.open("testdata/compression.txt");
    f1.fread((void*)ba.adr(), 1, 1024);
    // ba.hexDump();
    ASSERT_EQ(pplib::String("21ab51148e28167d5ce13bee07493a56"), pplib::Md5(ba));
    // load the next chunk
    f1.fread((void*)ba.adr(), 1, 1024);
    // ba.hexDump();
    ASSERT_EQ(pplib::String("468f6fd12d69be054643ef2ca1a19cba"), pplib::Md5(ba));
}

TEST_F(GzFileTest, fread1024_CompressedFile)
{
    pplib::GzFile f1;
    pplib::ByteArray ba;
    ba.malloc(1024);
    f1.open("testdata/compression.txt.gz");
    f1.fread((void*)ba.adr(), 1, 1024);
    // ba.hexDump();
    ASSERT_EQ(pplib::String("21ab51148e28167d5ce13bee07493a56"), pplib::Md5(ba));
    // load the next chunk
    f1.fread((void*)ba.adr(), 1, 1024);
    // ba.hexDump();
    ASSERT_EQ(pplib::String("468f6fd12d69be054643ef2ca1a19cba"), pplib::Md5(ba));
}

TEST_F(GzFileTest, md5_UncompressedFile)
{
    pplib::GzFile f1;
    f1.open("testdata/compression.txt");
    pplib::String digest = f1.md5();
    ASSERT_EQ(pplib::String("f386e5ea10bc186b633eaf6ba9a20d8c"), digest);
}

TEST_F(GzFileTest, md5_CompressedFile)
{
    pplib::GzFile f1;
    f1.open("testdata/compression.txt.gz");
    pplib::String digest = f1.md5();
    ASSERT_EQ(pplib::String("f386e5ea10bc186b633eaf6ba9a20d8c"), digest);
}

TEST_F(GzFileTest, seekAndTell)
{
    pplib::GzFile f1;
    f1.open("testdata/compression.txt.gz");
    f1.seek(45678);
    ASSERT_EQ((uint64_t)45678, f1.tell());
    f1.seek(100);
    ASSERT_EQ((uint64_t)100, f1.tell());
    f1.seek(1024 * 1024);
    ASSERT_EQ((uint64_t)1024 * 1024, f1.tell());
}

TEST_F(GzFileTest, rewind)
{
    pplib::GzFile f1;
    f1.open("testdata/compression.txt.gz");
    f1.seek(45678);
    f1.rewind();
    ASSERT_EQ((uint64_t)0, f1.tell());
}

TEST_F(GzFileTest, isOpen)
{
    pplib::GzFile f1;
    ASSERT_FALSE(f1.isOpen());
    f1.open("testdata/compression.txt.gz");
    ASSERT_TRUE(f1.isOpen());
}

TEST_F(GzFileTest, freadUntilEof)
{
    pplib::GzFile f1;
    pplib::ByteArray ba;
    ba.malloc(1024);
    f1.open("testdata/compression.txt.gz");
    uint64_t bytes = 0;
    ASSERT_THROW(
        {
            while (1) {
                bytes += f1.fread((void*)ba.adr(), 1, 1024);
            }
        },
        pplib::EndOfFileException);
    ASSERT_EQ((uint64_t)1592096, bytes);
}

TEST_F(GzFileTest, fgets)
{
    pplib::GzFile f1;
    pplib::ByteArray ba;
    ba.malloc(1024);
    char* buffer = (char*)ba.adr();
    f1.open("testdata/compression.txt.gz");
    char* ret;
    ASSERT_NO_THROW({ ret = f1.fgets(buffer, 1024); });
    ASSERT_EQ(ret, buffer);
    size_t len = strlen(ret);
    // printf (">>%s<< len=%zi\n",ret,strlen(ret));
    ASSERT_EQ((size_t)47, len);
}

TEST_F(GzFileTest, fgetsUntilEof)
{
    pplib::GzFile f1;
    pplib::ByteArray ba;
    ba.malloc(1024);
    char* buffer = (char*)ba.adr();
    f1.open("testdata/compression.txt.gz");
    char* ret;
    uint64_t bytes = 0;
    ASSERT_THROW(
        {
            while (1) {
                ret = f1.fgets(buffer, 1024);
                bytes += strlen(ret);
            }
        },
        pplib::EndOfFileException);
    ASSERT_EQ((uint64_t)1592096, bytes);
}

TEST_F(GzFileTest, getsAsString)
{
    pplib::GzFile f1;
    pplib::String s;
    f1.open("testdata/compression.txt.gz");
    ASSERT_NO_THROW({ s = f1.gets(1024); });
    s.trimRight();
    ASSERT_EQ(pplib::String("                    GNU GENERAL PUBLIC LICENSE"), s);
    ASSERT_NO_THROW({ s = f1.gets(); });
    s.trimRight();
    ASSERT_EQ(pplib::String("                       Version 2, June 1991"), s);
    ASSERT_NO_THROW({ s = f1.gets(); });
    s.trimRight();
    ASSERT_EQ(pplib::String(""), s);
    ASSERT_NO_THROW({ s = f1.gets(); });
    s.trimRight();
    ASSERT_EQ(pplib::String(" Copyright (C) 1989, 1991 Free Software Foundation, Inc.,"), s);
}

TEST_F(GzFileTest, fgetc)
{
    pplib::GzFile f1;
    f1.open("testdata/compression.txt.gz");
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ(32, f1.fgetc());
    ASSERT_EQ('G', f1.fgetc());
    ASSERT_EQ('N', f1.fgetc());
    ASSERT_EQ('U', f1.fgetc());
}

TEST_F(GzFileTest, fgetcUntilEof)
{
    pplib::GzFile f1;
    f1.open("testdata/compression.txt.gz");
    uint64_t bytes = 0;
    ASSERT_THROW(
        {
            while (1) {
                f1.fgetc();
                bytes++;
            }
        },
        pplib::EndOfFileException);
    ASSERT_EQ((uint64_t)1592096, bytes);
}

TEST_F(GzFileTest, ConstructorWithString)
{
    pplib::GzFile f(pplib::String("testdata/compression.txt.gz"));
    ASSERT_TRUE(f.isOpen());
    EXPECT_EQ(pplib::String("testdata/compression.txt.gz"), f.filename());
}

TEST_F(GzFileTest, ConstructorWithFd)
{
    pplib::File base("testdata/compression.txt.gz", pplib::File::FileMode::READ);
    int dupfd = dup(base.getFileNo());
    pplib::GzFile f(dupfd);
    ASSERT_TRUE(f.isOpen());
    EXPECT_EQ(pplib::String("FILE"), f.filename());
}

TEST_F(GzFileTest, ConstructorWithFdZeroThrows)
{
    ASSERT_THROW(pplib::GzFile f(0), pplib::IllegalArgumentException);
}

TEST_F(GzFileTest, DestructorViaFileObject)
{
    pplib::FileObject* fo = new pplib::GzFile("testdata/compression.txt.gz");
    ASSERT_TRUE(fo->isOpen());
    delete fo;
}

TEST_F(GzFileTest, openWithString)
{
    pplib::GzFile f;
    f.open(pplib::String("testdata/compression.txt.gz"));
    ASSERT_TRUE(f.isOpen());
    EXPECT_EQ(pplib::String("testdata/compression.txt.gz"), f.filename());
}

TEST_F(GzFileTest, openWithStringGzdopenFails)
{
    pplib::String filename = "tmp/gzfile_str_gzdopen_fail.gz";
    {
        pplib::File create(filename, pplib::File::FileMode::WRITE);
    }
    {
        pplib::GzFile f;
        ASSERT_THROW(f.open(filename, pplib::File::FileMode::READWRITE), pplib::Exception);
    }
    pplib::File::remove(filename);
}

TEST_F(GzFileTest, openWithStringEmptyThrows)
{
    pplib::GzFile f;
    ASSERT_THROW(f.open(pplib::String("")), pplib::IllegalArgumentException);
}

TEST_F(GzFileTest, openWithStringNonexistingThrows)
{
    pplib::GzFile f;
    ASSERT_THROW(f.open(pplib::String("nonexisting.gz")), pplib::FileNotFoundException);
}

TEST_F(GzFileTest, openWithNullOrEmptyCString)
{
    pplib::GzFile f;
    ASSERT_THROW(f.open((const char*)nullptr), pplib::IllegalArgumentException);
    ASSERT_THROW(f.open(""), pplib::IllegalArgumentException);
}

TEST_F(GzFileTest, openWithFd)
{
    pplib::File base("testdata/compression.txt.gz", pplib::File::FileMode::READ);
    int dupfd = dup(base.getFileNo());
    pplib::GzFile f;
    f.open(dupfd);
    ASSERT_TRUE(f.isOpen());
    EXPECT_EQ(pplib::String("FILE"), f.filename());
    ASSERT_THROW(f.open(0), pplib::IllegalArgumentException);
}

TEST_F(GzFileTest, openWithInvalidFdThrows)
{
    pplib::GzFile f;
    ASSERT_THROW(f.open(-1), pplib::Exception);
}

TEST_F(GzFileTest, openWithInvalidModeThrows)
{
    pplib::GzFile f;
    ASSERT_THROW(f.open(1, (pplib::File::FileMode)999), pplib::IllegalArgumentException);
}

TEST_F(GzFileTest, openWithReadWriteModeThrows)
{
    pplib::GzFile f;
    ASSERT_THROW(f.open("testdata/compression.txt", pplib::File::FileMode::READWRITE), pplib::Exception);
}

TEST_F(GzFileTest, openReopenClosesPrevious)
{
    pplib::GzFile f;
    f.open("testdata/compression.txt");
    ASSERT_TRUE(f.isOpen());
    EXPECT_EQ(pplib::String("testdata/compression.txt"), f.filename());
    f.open("testdata/compression.txt.gz");
    ASSERT_TRUE(f.isOpen());
    EXPECT_EQ(pplib::String("testdata/compression.txt.gz"), f.filename());
}

TEST_F(GzFileTest, openExistingUtf8)
{
    pplib::GzFile f;
    ASSERT_NO_THROW(f.open("testdata/filenameUTF8äöü.txt"));
    ASSERT_TRUE(f.isOpen());
}

TEST_F(GzFileTest, operationsWhenNotOpen)
{
    pplib::GzFile f;
    ASSERT_THROW(f.rewind(), pplib::FileNotOpenException);
    ASSERT_THROW(f.seek(0), pplib::FileNotOpenException);
    ASSERT_THROW(f.seek(0, pplib::File::SEEKSET), pplib::FileNotOpenException);
    ASSERT_THROW(f.tell(), pplib::FileNotOpenException);
    ASSERT_THROW(f.eof(), pplib::FileNotOpenException);
    char buf[16];
    ASSERT_THROW(f.fread(buf, 1, sizeof(buf)), pplib::FileNotOpenException);
    ASSERT_THROW(f.fgets(buf, sizeof(buf)), pplib::FileNotOpenException);
    ASSERT_THROW(f.fgetc(), pplib::FileNotOpenException);
    ASSERT_THROW(f.fwrite("data", 1, 4), pplib::FileNotOpenException);
}

TEST_F(GzFileTest, closeWhenNotOpen)
{
    pplib::GzFile f;
    ASSERT_NO_THROW(f.close());
    ASSERT_FALSE(f.isOpen());
    EXPECT_EQ(pplib::String(""), f.filename());
}

TEST_F(GzFileTest, closeMultipleTimes)
{
    pplib::GzFile f("testdata/compression.txt.gz");
    ASSERT_TRUE(f.isOpen());
    f.close();
    ASSERT_FALSE(f.isOpen());
    f.close();
    ASSERT_FALSE(f.isOpen());
}

TEST_F(GzFileTest, seekOrigins)
{
    pplib::GzFile f("testdata/compression.txt.gz");
    ASSERT_EQ((uint64_t)100, f.seek(100, pplib::File::SEEKSET));
    ASSERT_EQ((uint64_t)100, f.tell());
    ASSERT_EQ((uint64_t)150, f.seek(50, pplib::File::SEEKCUR));
    ASSERT_EQ((uint64_t)150, f.tell());
    ASSERT_THROW(f.seek(0, pplib::File::SEEKEND), pplib::UnsupportedFeatureException);
    ASSERT_THROW(f.seek(0, (pplib::File::SeekOrigin)999), pplib::IllegalArgumentException);
    ASSERT_THROW(f.seek(-10, pplib::File::SEEKSET), pplib::Exception);
}

TEST_F(GzFileTest, nullBufferArguments)
{
    pplib::GzFile f("testdata/compression.txt.gz");
    ASSERT_THROW(f.fread(nullptr, 1, 10), pplib::IllegalArgumentException);
    ASSERT_THROW(f.fgets(nullptr, 10), pplib::IllegalArgumentException);
    ASSERT_THROW(f.fwrite(nullptr, 1, 10), pplib::IllegalArgumentException);
}

TEST_F(GzFileTest, writeAndReadBack)
{
    pplib::String filename = "tmp/gzfile_unit_test.gz";
    const char* content = "This is a compressed GzFile test string with multiple lines.\nSecond line.\n";
    size_t len = strlen(content);
    {
        pplib::GzFile f(filename, pplib::File::FileMode::WRITE);
        ASSERT_TRUE(f.isOpen());
        size_t written = f.fwrite(content, 1, len);
        ASSERT_EQ(len, written);
        ASSERT_THROW(f.fwrite(content, 0, 0), pplib::EndOfFileException);
    }
    {
        pplib::GzFile f(filename, pplib::File::FileMode::READ);
        ASSERT_TRUE(f.isOpen());
        char buf[256];
        memset(buf, 0, sizeof(buf));
        size_t readBytes = f.fread(buf, 1, sizeof(buf) - 1);
        ASSERT_EQ(len, readBytes);
        EXPECT_STREQ(content, buf);
    }
    pplib::File::remove(filename);
}

TEST_F(GzFileTest, writeAppendMode)
{
    pplib::String filename = "tmp/gzfile_append_test.gz";
    {
        pplib::GzFile f(filename, pplib::File::FileMode::WRITE);
        f.fwrite("Part1\n", 1, 6);
    }
    {
        pplib::GzFile f(filename, pplib::File::FileMode::APPEND);
        ASSERT_TRUE(f.isOpen());
        f.fwrite("Part2\n", 1, 6);
    }
    {
        pplib::GzFile f(filename, pplib::File::FileMode::READ);
        char buf[64];
        memset(buf, 0, sizeof(buf));
        size_t bytes = f.fread(buf, 1, sizeof(buf) - 1);
        ASSERT_EQ((size_t)12, bytes);
        EXPECT_STREQ("Part1\nPart2\n", buf);
    }
    pplib::File::remove(filename);
}

TEST_F(GzFileTest, readOnWriteOnlyAndWriteOnReadOnly)
{
    pplib::String filename = "tmp/gzfile_mode_mismatch.gz";
    {
        pplib::GzFile f(filename, pplib::File::FileMode::WRITE);
        char buf[16];
        ASSERT_THROW(f.fread(buf, 1, sizeof(buf)), pplib::CompressionFailedException);
        ASSERT_THROW(f.fgets(buf, sizeof(buf)), pplib::Exception);
    }
    {
        pplib::GzFile f(filename, pplib::File::FileMode::READ);
        ASSERT_THROW(f.fwrite("data", 1, 4), pplib::EndOfFileException);
    }
    pplib::File::remove(filename);
}

TEST_F(GzFileTest, closeWithBadFileDescriptorThrows)
{
    pplib::String filename = "tmp/gzfile_bad_fd.gz";
    {
        int dupfd = -1;
        {
            pplib::File base(filename, pplib::File::FileMode::WRITE);
            dupfd = dup(base.getFileNo());
        }
        pplib::GzFile f;
        f.open(dupfd, pplib::File::FileMode::WRITE);
        ::close(dupfd);
        char largeBuf[16384];
        memset(largeBuf, 'A', sizeof(largeBuf));
        f.fwrite(largeBuf, 1, sizeof(largeBuf));
        ASSERT_THROW(f.close(), pplib::Exception);
    }
    pplib::File::remove(filename);
}

TEST_F(GzFileTest, eofDetection)
{
    pplib::GzFile f("testdata/compression.txt.gz");
    ASSERT_FALSE(f.eof());
    char buf[1024];
    try {
        while (true) {
            f.fread(buf, 1, sizeof(buf));
        }
    }
    catch (const pplib::EndOfFileException&) {
    }
    ASSERT_TRUE(f.eof());
}

} // namespace
