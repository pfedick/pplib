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
#include <pplib/core/dir.h>
#include <pplib/core/functions.h>
// #include <pplib/core/regex.h>
#include <pplib/exceptions.h>

#include "pplib-tests.h"

namespace
{

class FileStaticTest : public ::testing::Test
{
protected:
    FileStaticTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~FileStaticTest()
    {
    }
};

TEST_F(FileStaticTest, StatOnUSAscii)
{
    pplib::DirEntry d;
    ASSERT_NO_THROW({ (pplib::File::statFile)("testdata/filenameUSASCII.txt", d); });
    ASSERT_EQ((size_t)1787, d.Size);
    ASSERT_EQ(pplib::String("testdata/filenameUSASCII.txt"), d.File);
}

TEST_F(FileStaticTest, StatOnUtf8)
{
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("testdata/filenameUTF8äöü.txt", d));
    ASSERT_EQ((size_t)1340, d.Size);
    ASSERT_EQ(pplib::String("testdata/filenameUTF8äöü.txt"), d.File);
}

TEST_F(FileStaticTest, StatOnNonexistingFile)
{
    pplib::DirEntry d;
    ASSERT_THROW(pplib::File::statFile("testdata/nonexisting.txt", d), pplib::FileNotFoundException);
}

TEST_F(FileStaticTest, ExistsOnUSAscii)
{
    ASSERT_EQ(true, pplib::File::exists("testdata/filenameUSASCII.txt"));
    ASSERT_THROW(pplib::File::exists(""), pplib::IllegalArgumentException);
}

TEST_F(FileStaticTest, ExistsOnUtf8)
{
    ASSERT_EQ(true, pplib::File::exists("testdata/filenameUTF8äöü.txt"));
}

TEST_F(FileStaticTest, ExistsNot)
{
    ASSERT_EQ(false, pplib::File::exists("testdata/nonexistänt.txt"));
}

TEST_F(FileStaticTest, md5Hash)
{
    ASSERT_EQ(pplib::String("4a9012048a27082a811aba76a18153e0"), pplib::File::md5Hash("testdata/filenameUSASCII.txt"));
}

TEST_F(FileStaticTest, sha256Hash)
{
    ASSERT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"),
              pplib::File::sha256Hash("testdata/filenameUSASCII.txt"));
}

TEST_F(FileStaticTest, copyEdgeCases)
{
    ASSERT_THROW(pplib::File::copy("", "tmp/copy.tmp"), pplib::IllegalArgumentException);
    ASSERT_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", ""), pplib::IllegalArgumentException);
    ASSERT_NO_THROW(pplib::File::copy("tmp/copy.tmp", "tmp/copy.tmp"));
    pplib::File::remove("tmp/copy.tmp");
}

TEST_F(FileStaticTest, CopyFileUSAscii2USAscii)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/copy.tmp"));
    ASSERT_TRUE(pplib::File::exists("testdata/filenameUSASCII.txt")) << "Old file still exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/copy.tmp", d)) << "New file exists";
    ASSERT_EQ((size_t)1787, d.Size);

    pplib::File ff;
    ASSERT_NO_THROW(ff.open("tmp/copy.tmp"));
    ASSERT_EQ(pplib::String("4a9012048a27082a811aba76a18153e0"), ff.md5());
    ASSERT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"), ff.sha256());
    ff.close();
    pplib::File::remove("tmp/copy.tmp");
}

TEST_F(FileStaticTest, CopyFileUSAscii2Local)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/copyäöüß.tmp"));
    ASSERT_TRUE(pplib::File::exists("testdata/filenameUSASCII.txt")) << "Old file still exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/copyäöüß.tmp", d)) << "New file exists";
    ASSERT_EQ((size_t)1787, d.Size);

    pplib::File ff;
    ASSERT_NO_THROW(ff.open("tmp/copyäöüß.tmp"));
    ASSERT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"), ff.sha256());
    ff.close();
    pplib::File::remove("tmp/copyäöüß.tmp");
}

TEST_F(FileStaticTest, CopyFileLocal2Local)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUTF8äöü.txt", "tmp/copyäöüß.tmp"));
    ASSERT_TRUE(pplib::File::exists("testdata/filenameUTF8äöü.txt")) << "Old file still exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/copyäöüß.tmp", d)) << "New file exists";
    ASSERT_EQ((size_t)1340, d.Size);

    pplib::File ff;
    ASSERT_NO_THROW(ff.open("tmp/copyäöüß.tmp"));
    ASSERT_EQ(pplib::String("c12c31daf277de4d8e738e6a5e96fe6316e196eca087b4c691ef16d9da914c92"), ff.sha256());
    ff.close();
    pplib::File::remove("tmp/copyäöüß.tmp");
}

TEST_F(FileStaticTest, CopyFileLocal2UsAscii)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUTF8äöü.txt", "tmp/copy2.tmp"));
    ASSERT_TRUE(pplib::File::exists("testdata/filenameUTF8äöü.txt")) << "Old file still exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/copy2.tmp", d)) << "New file exists";
    ASSERT_EQ((size_t)1340, d.Size);

    pplib::File ff;
    ASSERT_NO_THROW(ff.open("tmp/copy2.tmp"));
    ASSERT_EQ(pplib::String("c12c31daf277de4d8e738e6a5e96fe6316e196eca087b4c691ef16d9da914c92"), ff.sha256());
    ff.close();
    pplib::File::remove("tmp/copy2.tmp");
}

TEST_F(FileStaticTest, TruncateOnUSASCII)
{
    ASSERT_THROW(pplib::File::truncate("", 0), pplib::IllegalArgumentException);
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/truncate.tmp"));
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/truncate.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);
    ASSERT_NO_THROW(pplib::File::truncate("tmp/truncate.tmp", 1024));
    ASSERT_NO_THROW(pplib::File::statFile("tmp/truncate.tmp", d));
    ASSERT_EQ((size_t)1024, d.Size);
    pplib::File ff;
    ASSERT_NO_THROW(ff.open("tmp/truncate.tmp"));
    ASSERT_EQ(pplib::String("7bb155b6d31fbdf30a1db55a8f0f48189aecf27a11e51020ba49d08ce7d2fd84"), ff.sha256());
    ff.close();
    pplib::File::remove("tmp/truncate.tmp");
}

TEST_F(FileStaticTest, TruncateOnLocal)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/truncateäöüß.tmp"));
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/truncateäöüß.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);
    ASSERT_NO_THROW(pplib::File::truncate("tmp/truncateäöüß.tmp", 1024));
    ASSERT_NO_THROW(pplib::File::statFile("tmp/truncateäöüß.tmp", d));
    ASSERT_EQ((size_t)1024, d.Size);
    pplib::File ff;
    ASSERT_NO_THROW(ff.open("tmp/truncateäöüß.tmp"));
    ASSERT_EQ(pplib::String("7bb155b6d31fbdf30a1db55a8f0f48189aecf27a11e51020ba49d08ce7d2fd84"), ff.sha256());
    ff.close();
    pplib::File::remove("tmp/truncateäöüß.tmp");
}

TEST_F(FileStaticTest, MoveFileUSAscii2USAscii)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/move1a.tmp"));
    ASSERT_NO_THROW(pplib::File::move("tmp/move1a.tmp", "tmp/move1b.tmp"));
    ASSERT_FALSE(pplib::File::exists("tmp/move1a.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("tmp/move1b.tmp")) << "New file exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/move1b.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);
    ASSERT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"), pplib::File::sha256Hash("tmp/move1b.tmp"));
    pplib::File::remove("tmp/move1b.tmp");
}

TEST_F(FileStaticTest, MoveFileUSAscii2Local)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/move2a.tmp"));
    ASSERT_NO_THROW(pplib::File::move("tmp/move2a.tmp", "tmp/move2bäöü.tmp"));
    ASSERT_FALSE(pplib::File::exists("tmp/move2a.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("tmp/move2bäöü.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/move2bäöü.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"),
              pplib::File::sha256Hash("tmp/move2bäöü.tmp"));
    pplib::File::remove("tmp/move2bäöü.tmp");
}

TEST_F(FileStaticTest, MoveFileLocal2USAscii)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/move3aäöü.tmp"));
    ASSERT_NO_THROW(pplib::File::move("tmp/move3aäöü.tmp", "tmp/move3b.tmp"));
    ASSERT_FALSE(pplib::File::exists("tmp/move3aäöü.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("tmp/move3b.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/move3b.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"), pplib::File::sha256Hash("tmp/move3b.tmp"));
    pplib::File::remove("tmp/move3b.tmp");
}

TEST_F(FileStaticTest, MoveFileLocal2Local)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/move4aäöü.tmp"));
    ASSERT_NO_THROW(pplib::File::move("tmp/move4aäöü.tmp", "tmp/move4bäöü.tmp"));
    ASSERT_FALSE(pplib::File::exists("tmp/move4aäöü.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("tmp/move4bäöü.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/move4bäöü.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"),
              pplib::File::sha256Hash("tmp/move4bäöü.tmp"));
    pplib::File::remove("tmp/move4bäöü.tmp");
}

TEST_F(FileStaticTest, RenameFileUSAscii2USAscii)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/move1a.tmp"));
    ASSERT_NO_THROW(pplib::File::rename("tmp/move1a.tmp", "tmp/move1b.tmp"));
    ASSERT_FALSE(pplib::File::exists("tmp/move1a.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("tmp/move1b.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/move1b.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"), pplib::File::sha256Hash("tmp/move1b.tmp"));
    pplib::File::remove("tmp/move1b.tmp");
}

TEST_F(FileStaticTest, RenameFileUSAscii2Local)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/move2a.tmp"));
    ASSERT_NO_THROW(pplib::File::rename("tmp/move2a.tmp", "tmp/move2bäöü.tmp"));
    ASSERT_FALSE(pplib::File::exists("tmp/move2a.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("tmp/move2bäöü.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/move2bäöü.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"),
              pplib::File::sha256Hash("tmp/move2bäöü.tmp"));
    pplib::File::remove("tmp/move2bäöü.tmp");
}

TEST_F(FileStaticTest, RenameFileLocal2USAscii)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/move3aäöü.tmp"));
    ASSERT_NO_THROW(pplib::File::rename("tmp/move3aäöü.tmp", "tmp/move3b.tmp"));
    ASSERT_FALSE(pplib::File::exists("tmp/move3aäöü.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("tmp/move3b.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/move3b.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"), pplib::File::sha256Hash("tmp/move3b.tmp"));
    pplib::File::remove("tmp/move3b.tmp");
}

TEST_F(FileStaticTest, RenameFileLocal2Local)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/move4aäöü.tmp"));
    ASSERT_NO_THROW(pplib::File::rename("tmp/move4aäöü.tmp", "tmp/move4bäöü.tmp"));
    ASSERT_FALSE(pplib::File::exists("tmp/move4aäöü.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("tmp/move4bäöü.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/move4bäöü.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"),
              pplib::File::sha256Hash("tmp/move4bäöü.tmp"));
    pplib::File::remove("tmp/move4bäöü.tmp");
}

TEST_F(FileStaticTest, erase_unlink_remove)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/erase.tmp"));
    // tmp kann nicht gelöscht werden, da nicht leer
    ASSERT_THROW(pplib::File::remove("tmp"), pplib::PermissionDeniedException);

    ASSERT_TRUE(pplib::File::exists("tmp/erase.tmp")) << "File should exist before removal";
    ASSERT_NO_THROW(pplib::File::remove("tmp/erase.tmp"));
    ASSERT_FALSE(pplib::File::exists("tmp/erase.tmp")) << "File should not exist after removal";

    ASSERT_NO_THROW(pplib::File::erase(""));
    ASSERT_NO_THROW(pplib::File::unlink("tmp/nonexisting.tmp"));
}

TEST_F(FileStaticTest, LoadUSAscii2String)
{
    pplib::String s;
    ASSERT_NO_THROW(pplib::File::load(s, "testdata/filenameUSASCII.txt")) << "Loading file to string";
    ASSERT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"), pplib::Sha256(s)) << "Checking MD5-sum";
    ASSERT_EQ((size_t)1787, s.size()) << "Checking Size";
}

TEST_F(FileStaticTest, LoadFileUSAscii2ByteArray)
{
    pplib::ByteArray s;
    ASSERT_NO_THROW(pplib::File::load(s, "testdata/filenameUSASCII.txt")) << "Loading file to string";
    ASSERT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"), pplib::Sha256(s)) << "Checking MD5-sum";
    ASSERT_EQ((size_t)1787, s.size()) << "Checking Size";
}

TEST_F(FileStaticTest, LoadFileLocal2String)
{
    pplib::String s;
    ASSERT_NO_THROW(pplib::File::load(s, "testdata/filenameUTF8äöü.txt")) << "Loading file to string";
    ASSERT_EQ(pplib::String("c12c31daf277de4d8e738e6a5e96fe6316e196eca087b4c691ef16d9da914c92"), pplib::Sha256(s)) << "Checking MD5-sum";
    ASSERT_EQ((size_t)1340, s.size()) << "Checking Size";
}

TEST_F(FileStaticTest, LoadFileLocal2ByteArray)
{
    pplib::ByteArray s;
    ASSERT_NO_THROW(pplib::File::load(s, "testdata/filenameUTF8äöü.txt")) << "Loading file to string";
    ASSERT_EQ(pplib::String("c12c31daf277de4d8e738e6a5e96fe6316e196eca087b4c691ef16d9da914c92"), pplib::Sha256(s)) << "Checking MD5-sum";
    ASSERT_EQ((size_t)1340, s.size()) << "Checking Size";
}

TEST_F(FileStaticTest, LoadFileUSAscii2Ptr)
{
    pplib::ByteArray data;

    ASSERT_NO_THROW({ data = pplib::File::load("testdata/filenameUSASCII.txt"); });
    ASSERT_TRUE(data.ptr() != NULL) << "Loading file to ptr";
    EXPECT_EQ((size_t)1787, data.size()) << "Checking Size";
    EXPECT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"), pplib::Sha256(data)) << "Checking MD5-sum";
}

TEST_F(FileStaticTest, LoadFileLocal2Ptr)
{
    pplib::ByteArray data;
    ASSERT_NO_THROW({ data = pplib::File::load("testdata/unicodeUtf8äöü.txt"); });
    ASSERT_TRUE(data.ptr() != NULL) << "Loading file to ptr";
    EXPECT_EQ((size_t)7976, data.size()) << "Checking Size";
    EXPECT_EQ(pplib::String("c6767ca956f9e3b65164bed63b93d811"), pplib::Md5(data)) << "Checking MD5-sum";
}

TEST_F(FileStaticTest, DeleteFileUSAscii)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/todelete.tmp"));
    ASSERT_NO_THROW(pplib::File::remove("tmp/todelete.tmp")) << "deleting file";
    ASSERT_FALSE(pplib::File::exists("tmp/todelete.tmp")) << "file does not exists anymore";
}

TEST_F(FileStaticTest, DeleteFileUtf8)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "tmp/todeleteäöü.tmp"));
    ASSERT_NO_THROW(pplib::File::remove("tmp/todeleteäöü.tmp")) << "deleting file";
    ASSERT_FALSE(pplib::File::exists("tmp/todeleteäöü.tmp")) << "file does not exists anymore";
}

TEST_F(FileStaticTest, TouchFileUSAscii)
{
    ASSERT_NO_THROW(pplib::File::touch("tmp/touched.tmp")) << "touching file";
    ASSERT_TRUE(pplib::File::exists("tmp/touched.tmp")) << "file exists";
    ASSERT_NO_THROW(pplib::File::remove("tmp/touched.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, TouchFileLocal)
{
    ASSERT_NO_THROW(pplib::File::touch("tmp/touchedäöü.tmp")) << "touching file";
    ASSERT_TRUE(pplib::File::exists("tmp/touchedäöü.tmp")) << "file exists";
    ASSERT_NO_THROW(pplib::File::remove("tmp/touchedäöü.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, SavePtrUSAscii)
{
    ASSERT_NO_THROW(pplib::File::save(loremipsum, strlen(loremipsum), "tmp/writetest.tmp")) << "writing file";
    ASSERT_TRUE(pplib::File::exists("tmp/writetest.tmp")) << "file exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/writetest.tmp", d)) << "Stat on file is working";
    ASSERT_EQ((size_t)strlen(loremipsum), d.Size) << "filesize as expected";
    ASSERT_EQ(pplib::String(loremipsum_md5), pplib::File::md5Hash("tmp/writetest.tmp"));
    ASSERT_NO_THROW(pplib::File::remove("tmp/writetest.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, SavePtrLocal)
{
    ASSERT_NO_THROW(pplib::File::save(loremipsum, strlen(loremipsum), "tmp/writetestäöü.tmp")) << "writing file";
    ASSERT_TRUE(pplib::File::exists("tmp/writetestäöü.tmp")) << "file exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/writetestäöü.tmp", d)) << "Stat on file is working";
    ASSERT_EQ((size_t)strlen(loremipsum), d.Size) << "filesize as expected";
    ASSERT_EQ(pplib::String(loremipsum_md5), pplib::File::md5Hash("tmp/writetestäöü.tmp"));
    ASSERT_NO_THROW(pplib::File::remove("tmp/writetestäöü.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, SaveByteArrayPtrUSAscii)
{
    pplib::ByteArrayPtr ba;
    ba.use((void*)loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW(pplib::File::save(ba, "tmp/writetest.tmp")) << "writing file";
    ASSERT_TRUE(pplib::File::exists("tmp/writetest.tmp")) << "file exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/writetest.tmp", d)) << "Stat on file is working";
    ASSERT_EQ((size_t)strlen(loremipsum), d.Size) << "filesize as expected";
    ASSERT_EQ(pplib::String(loremipsum_md5), pplib::File::md5Hash("tmp/writetest.tmp"));
    ASSERT_NO_THROW(pplib::File::remove("tmp/writetest.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, SaveByteArrayPtrLocal)
{
    pplib::ByteArrayPtr ba;
    ba.use((void*)loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW(pplib::File::save(ba, "tmp/writetestäöü.tmp")) << "writing file";
    ASSERT_TRUE(pplib::File::exists("tmp/writetestäöü.tmp")) << "file exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("tmp/writetestäöü.tmp", d)) << "Stat on file is working";
    ASSERT_EQ((size_t)strlen(loremipsum), d.Size) << "filesize as expected";
    ASSERT_EQ(pplib::String(loremipsum_md5), pplib::File::md5Hash("tmp/writetestäöü.tmp"));
    ASSERT_NO_THROW(pplib::File::remove("tmp/writetestäöü.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, statFile)
{
    pplib::DirEntry d;
    ASSERT_NO_THROW(d = pplib::File::statFile("testdata/dirwalk/LICENSE.TXT")) << "Stat on file is working";
    ASSERT_EQ((size_t)1330, d.Size) << "filesize as expected"; // Replace 0 with the actual expected size if known
}

TEST_F(FileStaticTest, tryStatFile)
{
    pplib::DirEntry d;
    ASSERT_TRUE(pplib::File::tryStatFile("testdata/dirwalk/LICENSE.TXT", d));
    ASSERT_FALSE(pplib::File::tryStatFile("testdata/nonexisting.txt", d));
    ASSERT_FALSE(pplib::File::tryStatFile("", d));
}

TEST_F(FileStaticTest, getPath)
{
    EXPECT_EQ(pplib::String("c:\\windows\\system32"), pplib::File::getPath("c:\\windows\\system32\\test.dll"));
    EXPECT_EQ(pplib::String("c:/windows/system32"), pplib::File::getPath("c:/windows/system32/test.dll"));
    EXPECT_EQ(pplib::String("/usr/bin"), pplib::File::getPath("/usr/bin/bash"));
}

TEST_F(FileStaticTest, getFilename)
{
    EXPECT_EQ(pplib::String("test.dll"), pplib::File::getFilename("c:\\windows\\system32\\test.dll"));
    EXPECT_EQ(pplib::String("test.dll"), pplib::File::getFilename("c:/windows/system32/test.dll"));
    EXPECT_EQ(pplib::String("bash"), pplib::File::getFilename("/usr/bin/bash"));
}

TEST_F(FileStaticTest, getSuffix)
{
    EXPECT_EQ(pplib::String("dll"), pplib::File::getSuffix("c:\\windows\\system32\\test.dll"));
    EXPECT_EQ(pplib::String("dll"), pplib::File::getSuffix("c:/windows/system32/test.dll"));
    EXPECT_EQ(pplib::String("sh"), pplib::File::getSuffix("/usr/bin/bash.sh"));
}

TEST_F(FileStaticTest, isDir)
{
    EXPECT_TRUE(pplib::File::isDir("testdata"));
    EXPECT_FALSE(pplib::File::isDir("nonexisting"));
    EXPECT_FALSE(pplib::File::isDir("testdata/dirwalk/LICENSE.TXT"));
}

TEST_F(FileStaticTest, isFile)
{
    EXPECT_TRUE(pplib::File::isFile("testdata/dirwalk/LICENSE.TXT"));
    EXPECT_FALSE(pplib::File::isFile("testdata/dirwalk"));
    EXPECT_FALSE(pplib::File::isFile("nonexisting"));
}

TEST_F(FileStaticTest, isLink)
{
    EXPECT_FALSE(pplib::File::isLink("testdata/dirwalk/LICENSE.TXT"));
    EXPECT_FALSE(pplib::File::isLink("testdata/dirwalk"));
    EXPECT_FALSE(pplib::File::isLink("nonexisting"));
}

TEST_F(FileStaticTest, isReadable)
{
    EXPECT_TRUE(pplib::File::isReadable("testdata/dirwalk/LICENSE.TXT"));
    EXPECT_FALSE(pplib::File::isReadable("nonexisting"));
}

TEST_F(FileStaticTest, isWritable)
{
    EXPECT_TRUE(pplib::File::isWritable("testdata/dirwalk/LICENSE.TXT"));
    EXPECT_FALSE(pplib::File::isWritable("nonexisting"));
}

TEST_F(FileStaticTest, isExecutable)
{
    EXPECT_FALSE(pplib::File::isExecutable("testdata/dirwalk/LICENSE.TXT"));
    EXPECT_FALSE(pplib::File::isExecutable("nonexisting"));
}

TEST_F(FileStaticTest, rename)
{
    pplib::File::copy("testdata/dirwalk/LICENSE.TXT", "tmp/rename_test.TXT");
    EXPECT_NO_THROW(pplib::File::rename("tmp/rename_test.TXT", "tmp/rename_test_renamed.TXT"));
    EXPECT_TRUE(pplib::File::isFile("tmp/rename_test_renamed.TXT"));
    EXPECT_FALSE(pplib::File::isFile("tmp/rename_test.TXT"));
    // pplib::File::remove("tmp/rename_test_renamed.TXT");
}

TEST_F(FileStaticTest, rename_throws)
{
    pplib::File::copy("testdata/dirwalk/LICENSE.TXT", "tmp/rename_test2.TXT");
    EXPECT_NO_THROW(pplib::File::rename("tmp/rename_test2.TXT", "tmp/rename_test2.TXT"));
    EXPECT_TRUE(pplib::File::isFile("tmp/rename_test2.TXT"));

    EXPECT_THROW(pplib::File::rename("tmp/rename_test2.TXT", "tmp/non_existing_dir/rename_test2.TXT"), pplib::IOException);
    EXPECT_TRUE(pplib::File::isFile("tmp/rename_test2.TXT"));

    EXPECT_THROW(pplib::File::rename("", "tmp/rename_test2.TXT"), pplib::IllegalArgumentException);
    EXPECT_THROW(pplib::File::rename("tmp/rename_test2.TXT", ""), pplib::IllegalArgumentException);
}

TEST_F(FileStaticTest, chmod)
{
    pplib::File::copy("testdata/dirwalk/LICENSE.TXT", "tmp/chmod_test.TXT");
    int attr = pplib::FileAttr::Attributes::USR_READ | pplib::FileAttr::Attributes::USR_WRITE;
    pplib::File::chmod("tmp/chmod_test.TXT", static_cast<pplib::FileAttr::Attributes>(attr));

    pplib::DirEntry d = pplib::File::statFile("tmp/chmod_test.TXT");
    EXPECT_EQ(static_cast<pplib::FileAttr::Attributes>(attr), d.Attrib);

    // pplib::File::remove("tmp/chmod_test.TXT");
}

} // namespace
