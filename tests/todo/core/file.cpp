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
#include <locale.h>
#include <pplib.h>
#include <gtest/gtest.h>
#include "pplib-tests.h"

namespace
{

// The fixture for testing class Foo.
class FileReadTest : public ::testing::Test
{
protected:
    FileReadTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~FileReadTest()
    {
    }
};

TEST_F(FileReadTest, ConstructorSimple)
{
    ASSERT_NO_THROW({
        pplib::File f1;
        ASSERT_FALSE(f1.isOpen()) << "File seems to be open, but it shouldn't";
    });
}

TEST_F(FileReadTest, openNonexisting)
{
    pplib::File f1;
    ASSERT_THROW(f1.open("nonexisting.txt"), pplib::FileNotFoundException);
}

TEST_F(FileReadTest, openNonexistingUtf8)
{
    pplib::File f1;
    ASSERT_THROW(f1.open("noneäxisting.txt"), pplib::FileNotFoundException);
}

TEST_F(FileReadTest, openExisting)
{
    pplib::File f1;
    ASSERT_NO_THROW(f1.open("testdata/filenameUSASCII.txt"));
}

TEST_F(FileReadTest, openExistingLocal)
{
    pplib::File f1;
    ASSERT_NO_THROW(f1.open(pplib::Iconv::Utf8ToLocal("testdata/filenameUTF8äöü.txt")));
}

TEST_F(FileReadTest, size)
{
    pplib::File f1;
    f1.open("testdata/dirwalk/LICENSE.TXT");
    ASSERT_EQ((uint64_t)1330, f1.size());
    pplib::File f2;
    f2.open("testdata/dirwalk/testfile.txt");
    ASSERT_EQ((uint64_t)1592096, f2.size());
}

TEST_F(FileReadTest, openAndClose)
{
    pplib::File f1;
    f1.open("testdata/dirwalk/LICENSE.TXT");
    ASSERT_EQ((uint64_t)1330, f1.size());
    f1.close();
    f1.open("testdata/dirwalk/testfile.txt");
    ASSERT_EQ((uint64_t)1592096, f1.size());
}

TEST_F(FileReadTest, openAndCloseNonUSASCIIFilenames)
{
    pplib::File f1;
    f1.open("testdata/dirwalk/èxôtíŒ.txt");
    ASSERT_EQ((uint64_t)1356, f1.size());
    f1.close();
    f1.open("testdata/dirwalk/file4✼.txt");
    ASSERT_EQ((uint64_t)5281, f1.size());
}

TEST_F(FileReadTest, md5)
{
    pplib::File f1;
    f1.open("testdata/dirwalk/testfile.txt");
    pplib::String digest = f1.md5();
    ASSERT_EQ(pplib::String("f386e5ea10bc186b633eaf6ba9a20d8c"), digest);
}

TEST_F(FileReadTest, seekAndTell)
{
    pplib::File f1;
    f1.open("testdata/dirwalk/testfile.txt");
    f1.seek(45678);
    ASSERT_EQ((uint64_t)45678, f1.tell());
    f1.seek(100);
    ASSERT_EQ((uint64_t)100, f1.tell());
    f1.seek(1024 * 1024);
    ASSERT_EQ((uint64_t)1024 * 1024, f1.tell());
}

TEST_F(FileReadTest, seek)
{
    pplib::File f1;
    f1.open("testdata/dirwalk/testfile.txt");
    f1.seek(45678, pplib::File::SEEKSET);
    ASSERT_EQ((uint64_t)45678, f1.tell());
    f1.seek(100, pplib::File::SEEKCUR);
    ASSERT_EQ((uint64_t)45778, f1.tell());
    f1.seek(-1000, pplib::File::SEEKCUR);
    ASSERT_EQ((uint64_t)44778, f1.tell());
    f1.seek(-1000, pplib::File::SEEKEND);
    ASSERT_EQ((uint64_t)1591096, f1.tell());
}

TEST_F(FileReadTest, fread1024)
{
    pplib::File f1;
    pplib::ByteArray ba;
    ba.malloc(1024);
    f1.open("testdata/dirwalk/testfile.txt");
    f1.fread((void*)ba.adr(), 1, 1024);
    // ba.hexDump();
    ASSERT_EQ(pplib::String("21ab51148e28167d5ce13bee07493a56"), pplib::Md5(ba));
    // load the next chunk
    f1.fread((void*)ba.adr(), 1, 1024);
    // ba.hexDump();
    ASSERT_EQ(pplib::String("468f6fd12d69be054643ef2ca1a19cba"), pplib::Md5(ba));
}

TEST_F(FileReadTest, freadUntilEof)
{
    pplib::File f1;
    pplib::ByteArray ba;
    ba.malloc(1024);
    f1.open("testdata/dirwalk/testfile.txt");
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

TEST_F(FileReadTest, fgets)
{
    pplib::File f1;
    pplib::ByteArray ba;
    ba.malloc(1024);
    char* buffer = (char*)ba.adr();
    f1.open("testdata/dirwalk/testfile.txt");
    char* ret;
    ASSERT_NO_THROW({ ret = f1.fgets(buffer, 1024); });
    ASSERT_EQ(ret, buffer);
    size_t len = strlen(ret);
    // printf (">>%s<< len=%zi\n",ret,strlen(ret));
    ASSERT_EQ((size_t)47, len);
}

TEST_F(FileReadTest, getsAsString)
{
    pplib::File f1;
    pplib::String s;
    f1.open("testdata/dirwalk/testfile.txt");
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

} // namespace
