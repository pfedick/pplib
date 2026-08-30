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

#include <gtest/gtest.h>

#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/array.h>
#include <pplib/exceptions.h>
#include <pplib/core/file.h>
#include <pplib/core/functions.h>
// #include <pplib/core/regex.h>
#include <pplib/exceptions.h>

#include "pplib-tests.h"

namespace
{

// The fixture for testing class Foo.
class FileTest : public ::testing::Test
{
protected:
    FileTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~FileTest()
    {
    }
};

TEST_F(FileTest, ConstructorSimple)
{
    ASSERT_NO_THROW({
        pplib::File f1;
        ASSERT_FALSE(f1.isOpen()) << "File seems to be open, but it shouldn't";
    });
}

// ####################################################################
// File::File(const String& filename, FileMode mode)
// ####################################################################
TEST_F(FileTest, ConstructorFilename_ReadExisting)
{
    pplib::File f("testdata/dirwalk/LICENSE.TXT", pplib::File::FileMode::READ);
    ASSERT_TRUE(f.isOpen());
    ASSERT_EQ((uint64_t)1330, f.size());
    ASSERT_EQ((uint64_t)0, f.tell());
    ASSERT_EQ(pplib::String("testdata/dirwalk/LICENSE.TXT"), f.filename());
    // Daten müssen direkt nach dem Konstruktor lesbar sein
    char buffer[8];
    ASSERT_EQ((size_t)8, f.fread(buffer, 1, 8));
}

TEST_F(FileTest, ConstructorFilename_DefaultMode)
{
    // Ohne Angabe des Modus muss FileMode::READ die Default-Einstellung sein
    pplib::File f("testdata/dirwalk/LICENSE.TXT");
    ASSERT_TRUE(f.isOpen());
    ASSERT_EQ((uint64_t)1330, f.size());
    ASSERT_EQ((uint64_t)0, f.tell());
}

TEST_F(FileTest, ConstructorFilename_ReadExistingUtf8)
{
    pplib::File f("testdata/filenameUTF8äöü.txt", pplib::File::FileMode::READ);
    ASSERT_TRUE(f.isOpen());
    ASSERT_EQ(pplib::String("testdata/filenameUTF8äöü.txt"), f.filename());
}

TEST_F(FileTest, ConstructorFilename_WriteNew)
{
    const char* filename = "tmp/file_ctor_write.txt";
    pplib::File::erase(filename);
    {
        pplib::File f(filename, pplib::File::FileMode::WRITE);
        ASSERT_TRUE(f.isOpen());
        ASSERT_EQ((uint64_t)0, f.size());
        ASSERT_EQ((uint64_t)0, f.tell());
        const char* content = "Hallo Welt\n";
        f.fwrite(content, 1, strlen(content));
    }
    // Datei muss jetzt existieren und den Inhalt enthalten
    ASSERT_TRUE(pplib::File::exists(filename));
    {
        pplib::File r(filename, pplib::File::FileMode::READ);
        ASSERT_EQ((uint64_t)strlen("Hallo Welt\n"), r.size());
    }
    pplib::File::erase(filename);
}

TEST_F(FileTest, ConstructorFilename_AppendNew)
{
    const char* filename = "tmp/file_ctor_append.txt";
    pplib::File::erase(filename);
    {
        pplib::File f(filename, pplib::File::FileMode::APPEND);
        ASSERT_TRUE(f.isOpen());
        ASSERT_EQ((uint64_t)0, f.size());
    }
    // Datei erst schließen, dann löschen (Windows kann offene Dateien nicht löschen)
    pplib::File::erase(filename);
}

TEST_F(FileTest, ConstructorFilename_AppendExisting)
{
    const char* filename = "tmp/file_ctor_append.txt";
    {
        pplib::File w(filename, pplib::File::FileMode::WRITE);
        w.fputs("12345");
    }
    {
        pplib::File f(filename, pplib::File::FileMode::APPEND);
        ASSERT_TRUE(f.isOpen());
        ASSERT_EQ((uint64_t)5, f.size());
        // Schreiben muss trotz seek(0) im open() ans Dateiende erfolgen
        f.fputs("67");
    }
    {
        pplib::File r(filename, pplib::File::FileMode::READ);
        ASSERT_EQ((uint64_t)7, r.size());
    }
    pplib::File::erase(filename);
}

TEST_F(FileTest, ConstructorFilename_ReadwriteExisting)
{
    const char* filename = "tmp/file_ctor_readwrite.txt";
    {
        pplib::File w(filename, pplib::File::FileMode::WRITE);
        w.fputs("Hallo");
    }
    {
        pplib::File f(filename, pplib::File::FileMode::READWRITE);
        ASSERT_TRUE(f.isOpen());
        ASSERT_EQ((uint64_t)5, f.size());
        ASSERT_EQ((uint64_t)0, f.tell());
    }
    // Datei erst schließen, dann löschen (Windows kann offene Dateien nicht löschen)
    pplib::File::erase(filename);
}

TEST_F(FileTest, ConstructorFilename_Nonexisting_Read)
{
    ASSERT_THROW(pplib::File f("tmp/nonexisting_file_ctor.txt", pplib::File::FileMode::READ), pplib::FileNotFoundException);
}

TEST_F(FileTest, ConstructorFilename_Nonexisting_Readwrite)
{
    // r+b legt keine neue Datei an
    ASSERT_THROW(pplib::File f("tmp/nonexisting_file_ctor.txt", pplib::File::FileMode::READWRITE), pplib::FileNotFoundException);
}

TEST_F(FileTest, ConstructorFilename_Empty)
{
    ASSERT_THROW(pplib::File f("", pplib::File::FileMode::READ), pplib::IllegalArgumentException);
}

TEST_F(FileTest, ConstructorFilename_InvalidMode)
{
    // Ungültiger Modus-Wert => IllegalArgumentException aus fmode()
    ASSERT_THROW(pplib::File f("testdata/dirwalk/LICENSE.TXT", (pplib::File::FileMode)99), pplib::IllegalArgumentException);
}

TEST_F(FileTest, ConstructorFilehandle)
{
    // Test the constructor that takes a FILE* handle
    FILE* fh = fopen("testdata/dirwalk/LICENSE.TXT", "rb");
    ASSERT_NE(nullptr, fh);
    pplib::File f(fh);
    ASSERT_TRUE(f.isOpen());
    ASSERT_EQ((uint64_t)1330, f.size());
    f.close();
}

TEST_F(FileTest, ConstructorFilehandle_isNull)
{
    ASSERT_THROW(pplib::File f(nullptr), pplib::IllegalArgumentException);
}

TEST_F(FileTest, openNonexisting)
{
    pplib::File f1;
    ASSERT_THROW(f1.open("nonexisting.txt"), pplib::FileNotFoundException);
}

TEST_F(FileTest, openNonexistingUtf8)
{
    pplib::File f1;
    ASSERT_THROW(f1.open("noneäxisting.txt"), pplib::FileNotFoundException);
}

TEST_F(FileTest, openExisting)
{
    pplib::File f1;
    ASSERT_NO_THROW(f1.open("testdata/filenameUSASCII.txt"));
}

TEST_F(FileTest, openExistingUTF8)
{
    pplib::File f1;
    ASSERT_NO_THROW(f1.open("testdata/filenameUTF8äöü.txt"));
}

TEST_F(FileTest, openTemp)
{
    pplib::String filename = "tmp/tmpfileXXXXXX";
    pplib::File f;
    ASSERT_NO_THROW(f.openTemp(filename));
    ASSERT_TRUE(f.isOpen());
    ASSERT_TRUE(pplib::File::exists(f.filename()));
    ASSERT_TRUE(f.filename().contains("tmp/tmpfile"));
    f.close();
    pplib::File::erase(filename);
}

TEST_F(FileTest, openTempIllegalTemplate)
{
    pplib::String filename = "nonexisting/tmpfileXXXXXX";
    pplib::File f;
    ASSERT_THROW(f.openTemp(filename), pplib::FileNotFoundException);
}

TEST_F(FileTest, size)
{
    pplib::File f1;
    f1.open("testdata/dirwalk/LICENSE.TXT");
    ASSERT_EQ((uint64_t)1330, f1.size());
    pplib::File f2;
    f2.open("testdata/dirwalk/testfile.txt");
    ASSERT_EQ((uint64_t)1592096, f2.size());
}

TEST_F(FileTest, sizeOnNonOpenFile)
{
    pplib::File f;
    ASSERT_THROW(f.size(), pplib::FileNotOpenException);
}

TEST_F(FileTest, openAndClose)
{
    pplib::File f1;
    f1.open("testdata/dirwalk/LICENSE.TXT");
    ASSERT_EQ((uint64_t)1330, f1.size());
    f1.close();
    f1.open("testdata/dirwalk/testfile.txt");
    ASSERT_EQ((uint64_t)1592096, f1.size());
}

TEST_F(FileTest, openAndCloseNonUSASCIIFilenames)
{
    pplib::File f1;
    f1.open("testdata/dirwalk/èxôtíŒ.txt");
    ASSERT_EQ((uint64_t)1356, f1.size());
    f1.close();
    f1.open("testdata/dirwalk/file4✼.txt");
    ASSERT_EQ((uint64_t)5287, f1.size());
}

TEST_F(FileTest, popenTest)
{
    pplib::File f;
#ifdef _WIN32
    // ASSERT_NO_THROW(f.popen("testdata\\scripts\\echo.sh ", pplib::File::FileMode::READ));
    ASSERT_NO_THROW(f.popen("cmd.exe /c echo Hallo Welt", pplib::File::FileMode::READ));
#else
    ASSERT_NO_THROW(f.popen("echo Hallo Welt", pplib::File::FileMode::READ));
#endif
    ASSERT_TRUE(f.isOpen());
    pplib::String output = f.gets().trim();
    ASSERT_EQ(pplib::String("Hallo Welt"), output);
    f.close();
}

TEST_F(FileTest, popenThrows)
{
    pplib::File f;
    ASSERT_THROW(f.popen("", pplib::File::FileMode::READ), pplib::IllegalArgumentException);
    ASSERT_THROW(f.popen("echo test", (pplib::File::FileMode)99), pplib::IllegalArgumentException);

#ifdef _WIN32
    ASSERT_NO_THROW(f.popen("cmd.exe /c exit 1", pplib::File::FileMode::READ));
#else
    ASSERT_NO_THROW(f.popen("false", pplib::File::FileMode::READ));
#endif
    ASSERT_THROW(f.close(), pplib::Exception);
}

TEST_F(FileTest, openWithFileHandle)
{
    pplib::File f;
    FILE* fh = fopen("testdata/dirwalk/LICENSE.TXT", "rb");
    ASSERT_NE(nullptr, fh);

    ASSERT_NO_THROW(f.open(fh));
    ASSERT_TRUE(f.isOpen());
    ASSERT_EQ((uint64_t)1330, f.size());
    f.close();

    ASSERT_THROW(f.open(nullptr), pplib::IllegalArgumentException);
}

TEST_F(FileTest, rewind)
{
    pplib::File f1;
    f1.open("testdata/dirwalk/testfile.txt");
    f1.seek(512);
    ASSERT_EQ((uint64_t)512, f1.tell());
    f1.rewind();
    ASSERT_EQ((uint64_t)0, f1.tell());
    // Wir lesen vom Anfang der Datei nach dem Rewind
    pplib::String s = f1.gets(1024).trim();
    ASSERT_EQ(pplib::String("GNU GENERAL PUBLIC LICENSE"), s);
}

TEST_F(FileTest, md5)
{
    pplib::File f1;
    f1.open("testdata/dirwalk/testfile.txt");
    pplib::String digest = f1.md5();
    ASSERT_EQ(pplib::String("f386e5ea10bc186b633eaf6ba9a20d8c"), digest);
}

TEST_F(FileTest, seekAndTell)
{
    pplib::File f1;
    f1.open("testdata/dirwalk/testfile.txt");
    f1.seek(45678);
    ASSERT_EQ((uint64_t)45678, f1.tell());
    f1.seek(100);
    ASSERT_EQ((uint64_t)100, f1.tell());
    f1.seek(1024 * 1024);
    ASSERT_EQ((uint64_t)1024 * 1024, f1.tell());

    pplib::File f2;
    ASSERT_THROW(f2.tell(), pplib::FileNotOpenException);
}

TEST_F(FileTest, tellOnPipeThrows)
{
    pplib::File f;
    f.open("testdata/dirwalk/testfile.txt");
    ASSERT_TRUE(f.isOpen());
    int fh = f.getFileNo();
    ASSERT_NE(-1, fh);
    ::close(fh);
    ASSERT_THROW(f.tell(), pplib::BadFiledescriptorException);
}

TEST_F(FileTest, seekThrows)
{
    pplib::File f1;
    ASSERT_THROW(f1.seek(0), pplib::FileNotOpenException);
    ASSERT_THROW(f1.seek(0, pplib::File::SEEKSET), pplib::FileNotOpenException);

    f1.open("testdata/dirwalk/LICENSE.TXT");
    ASSERT_THROW(f1.seek(-1), pplib::InvalidArgumentsException);
    // ASSERT_THROW(f1.seek(12345), pplib::FileNotOpenException);

    ASSERT_THROW(f1.seek(0, static_cast<pplib::File::SeekOrigin>(123)), pplib::IllegalArgumentException);
    ASSERT_THROW(f1.seek(0, static_cast<pplib::File::SeekOrigin>(123)), pplib::IllegalArgumentException);
    ASSERT_THROW(f1.seek(-1, pplib::File::SEEKSET), pplib::InvalidArgumentsException);
}

TEST_F(FileTest, seekWithOrigin)
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

TEST_F(FileTest, fread1024)
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

TEST_F(FileTest, freadZeroReturnsZero)
{
    pplib::File f1;
    pplib::ByteArray ba;
    ba.malloc(1024);
    f1.open("testdata/dirwalk/testfile.txt");
    ASSERT_EQ((size_t)0, f1.fread((void*)ba.adr(), 0, 1024));
    ASSERT_EQ((size_t)0, f1.fread((void*)ba.adr(), 1, 0));
}

TEST_F(FileTest, freadUntilEof)
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

TEST_F(FileTest, freadThrows)
{
    pplib::File f1;
    pplib::ByteArray ba;
    ba.malloc(1024);
    ASSERT_THROW(f1.fread((void*)ba.adr(), 1, 1024), pplib::FileNotOpenException);
    ASSERT_THROW(f1.fread(NULL, 1, 1024), pplib::IllegalArgumentException);

    pplib::File f2;
    f2.open("testdata/dirwalk/testfile.txt");
    // Wir schließen "heimlich" die Datei und prüfen, ob fread danach eine Ausnahme wirft
    ::close(f2.getFileNo());
    ASSERT_THROW(f2.fread((void*)ba.adr(), 1, 1024), pplib::BadFiledescriptorException);
}

TEST_F(FileTest, freadAfterFileEnd)
{
    pplib::File f1;
    pplib::ByteArray ba;
    ba.malloc(1024);
    f1.open("testdata/dirwalk/testfile.txt");
    f1.seek(-512, pplib::File::SeekOrigin::SEEKEND);
    size_t r = f1.fread((void*)ba.adr(), 1, 1024);
    ASSERT_EQ((size_t)512, r);
    ASSERT_THROW(f1.fread((void*)ba.adr(), 1, 1), pplib::EndOfFileException);
}

TEST_F(FileTest, fwrite)
{
    pplib::File f1;
    pplib::File::remove("tmp/fwrite.txt");
    f1.open("tmp/fwrite.txt", pplib::File::FileMode::READWRITE_CREATE);
    pplib::ByteArray ba1 = pplib::Random(512);
    pplib::ByteArray ba2 = pplib::Random(512);
    pplib::ByteArray ba3 = pplib::Random(512);
    pplib::ByteArray expected = ba1 + ba3;
    ASSERT_EQ((size_t)ba1.size(), f1.fwrite((void*)ba1.adr(), 1, ba1.size()));
    ASSERT_EQ((size_t)ba2.size(), f1.fwrite((void*)ba2.adr(), 1, ba2.size()));

    f1.seek(ba1.size());
    ASSERT_EQ((size_t)ba3.size(), f1.fwrite((void*)ba3.adr(), 1, ba3.size()));
    f1.seek(0);
    pplib::ByteArray actual;
    actual.malloc(ba1.size() + ba3.size());

    f1.fread((void*)actual.adr(), 1, actual.size());

    ASSERT_EQ(expected, actual);
    f1.close();
    pplib::File::remove("tmp/fwrite.txt");
}

TEST_F(FileTest, fwriteNullReturnsZero)
{
    pplib::File f1;
    f1.open("tmp/fwriteNullReturnsZero.txt", pplib::File::FileMode::WRITE);
    pplib::ByteArray ba;
    ba.malloc(1024);
    ASSERT_EQ((size_t)0, f1.fwrite((void*)ba.adr(), 0, 1024));
    ASSERT_EQ((size_t)0, f1.fwrite((void*)ba.adr(), 1, 0));
    f1.close();
    pplib::File::remove("tmp/fwriteNullReturnsZero.txt");
}

TEST_F(FileTest, fwriteThrows)
{
    pplib::File f1;
    pplib::ByteArray ba;
    ba.malloc(1024);
    ASSERT_THROW(f1.fwrite((void*)ba.adr(), 1, 1024), pplib::FileNotOpenException);
    ASSERT_THROW(f1.fwrite(NULL, 1, 1024), pplib::IllegalArgumentException);
}

TEST_F(FileTest, fgets)
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

TEST_F(FileTest, fgets_loop)
{
    pplib::File f1;
    pplib::ByteArray ba;
    ba.malloc(1024);
    char* buffer = (char*)ba.adr();
    f1.open("testdata/dirwalk/LICENSE.TXT", pplib::File::FileMode::READ);
    pplib::String content;
    char* ret;
    while ((ret = f1.fgets(buffer, 1024)) != NULL) {
        content += ret;
    }
    ASSERT_EQ(content.size(), f1.size());
}

TEST_F(FileTest, fgetsThrows)
{
    pplib::File f1;
    pplib::ByteArray ba;
    ba.malloc(1024);
    char* buffer = (char*)ba.adr();
    ASSERT_THROW(f1.fgets(buffer, 1024), pplib::FileNotOpenException);
    ASSERT_THROW(f1.fgets(NULL, 1024), pplib::IllegalArgumentException);
    ASSERT_THROW(f1.fgets(buffer, 0), pplib::IllegalArgumentException);

    f1.open("testdata/dirwalk/testfile.txt", pplib::File::FileMode::READ);
    // Wir schließen "heimlich" die Datei und prüfen, ob fgets danach eine Ausnahme wirft
    ::close(f1.getFileNo());
    ASSERT_THROW(f1.fgets(buffer, 1024), pplib::BadFiledescriptorException);
}

TEST_F(FileTest, fgetws_loop)
{
    pplib::File f1;
    pplib::ByteArray ba;
    ba.malloc(1024 * sizeof(wchar_t));
    wchar_t* buffer = (wchar_t*)ba.adr();
    f1.open("testdata/dirwalk/LICENSE.TXT", pplib::File::FileMode::READ);
    pplib::WideString content;
    wchar_t* ret;
    while ((ret = f1.fgetws(buffer, 1024)) != NULL) {
        content += ret;
    }
    ASSERT_EQ(content.size() * sizeof(wchar_t), f1.size());
}

TEST_F(FileTest, fputwsAndFgetwsRoundtrip)
{
    const char* tmpfile = "tmp/test_wide.txt";
    pplib::File::erase(tmpfile);

    // 1. Schreiben mit fputws
    {
        pplib::File f(tmpfile, pplib::File::FileMode::WRITE);
        ASSERT_NO_THROW(f.fputws(L"Erste Zeile\n"));
        ASSERT_NO_THROW(f.fputws(L"Zweite Zeile: äöü - WideChar\n"));
    }

    // 2. Lesen mit fgetws
    {
        pplib::File f(tmpfile, pplib::File::FileMode::READ);
        const size_t bufElements = 256;
        wchar_t buffer[bufElements];

        pplib::WideString content;
        while (f.fgetws(buffer, bufElements) != NULL) {
            content += buffer;
        }

        pplib::WideString expected = L"Erste Zeile\nZweite Zeile: äöü - WideChar\n";
        ASSERT_EQ(expected, content);
    }

    pplib::File::erase(tmpfile);
}

TEST_F(FileTest, getsAsString)
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
