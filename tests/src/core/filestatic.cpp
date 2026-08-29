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

TEST_F(FileStaticTest, CopyFileUSAscii2USAscii)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "copy.tmp"));
    ASSERT_TRUE(pplib::File::exists("testdata/filenameUSASCII.txt")) << "Old file still exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("copy.tmp", d)) << "New file exists";
    ASSERT_EQ((size_t)1787, d.Size);

    pplib::File ff;
    ASSERT_NO_THROW(ff.open("copy.tmp"));
    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), ff.md5());
    ASSERT_EQ(pplib::String("c0736be5fcbe7bf5a19a21045ea437c397643202dd4660fc0487ea63917e2bbc"), ff.sha256());
    ff.close();
    pplib::File::remove("copy.tmp");
}

TEST_F(FileStaticTest, CopyFileUSAscii2Local)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "copyäöüß.tmp"));
    ASSERT_TRUE(pplib::File::exists("testdata/filenameUSASCII.txt")) << "Old file still exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("copyäöüß.tmp", d)) << "New file exists";
    ASSERT_EQ((size_t)1787, d.Size);

    pplib::File ff;
    ASSERT_NO_THROW(ff.open("copyäöüß.tmp"));
    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), ff.md5());
    ff.close();
    pplib::File::remove("copyäöüß.tmp");
}

TEST_F(FileStaticTest, CopyFileLocal2Local)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUTF8äöü.txt", "copyäöüß.tmp"));
    ASSERT_TRUE(pplib::File::exists("testdata/filenameUTF8äöü.txt")) << "Old file still exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("copyäöüß.tmp", d)) << "New file exists";
    ASSERT_EQ((size_t)1340, d.Size);

    pplib::File ff;
    ASSERT_NO_THROW(ff.open("copyäöüß.tmp"));
    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), ff.md5());
    ff.close();
    pplib::File::remove("copyäöüß.tmp");
}

TEST_F(FileStaticTest, CopyFileLocal2UsAscii)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUTF8äöü.txt", "copy2.tmp"));
    ASSERT_TRUE(pplib::File::exists("testdata/filenameUTF8äöü.txt")) << "Old file still exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("copy2.tmp", d)) << "New file exists";
    ASSERT_EQ((size_t)1787, d.Size);

    pplib::File ff;
    ASSERT_NO_THROW(ff.open("copy2.tmp"));
    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), ff.md5());
    ff.close();
    pplib::File::remove("copy2.tmp");
}

TEST_F(FileStaticTest, TruncateOnUSASCII)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "truncate.tmp"));
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("truncate.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);
    ASSERT_NO_THROW(pplib::File::truncate("truncate.tmp", 1024));
    ASSERT_NO_THROW(pplib::File::statFile("truncate.tmp", d));
    ASSERT_EQ((size_t)1024, d.Size);
    pplib::File ff;
    ASSERT_NO_THROW(ff.open("truncate.tmp"));
    ASSERT_EQ(pplib::String("657351fba38e20fb0a4713e605f1d6a4"), ff.md5());
    ff.close();
    pplib::File::remove("truncate.tmp");
}

TEST_F(FileStaticTest, TruncateOnLocal)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "truncateäöüß.tmp"));
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("truncateäöüß.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);
    ASSERT_NO_THROW(pplib::File::truncate("truncateäöüß.tmp", 1024));
    ASSERT_NO_THROW(pplib::File::statFile("truncateäöüß.tmp", d));
    ASSERT_EQ((size_t)1024, d.Size);
    pplib::File ff;
    ASSERT_NO_THROW(ff.open("truncateäöüß.tmp"));
    ASSERT_EQ(pplib::String("657351fba38e20fb0a4713e605f1d6a4"), ff.md5());
    ff.close();
    pplib::File::remove("truncateäöüß.tmp");
}

TEST_F(FileStaticTest, MoveFileUSAscii2USAscii)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "move1a.tmp"));
    ASSERT_NO_THROW(pplib::File::move("move1a.tmp", "move1b.tmp"));
    ASSERT_FALSE(pplib::File::exists("move1a.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("move1b.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("move1b.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), pplib::File::md5Hash("move1b.tmp"));
    pplib::File::remove("move1b.tmp");
}

TEST_F(FileStaticTest, MoveFileUSAscii2Local)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "move2a.tmp"));
    ASSERT_NO_THROW(pplib::File::move("move2a.tmp", "move2bäöü.tmp"));
    ASSERT_FALSE(pplib::File::exists("move2a.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("move2bäöü.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("move2bäöü.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), pplib::File::md5Hash("move2bäöü.tmp"));
    pplib::File::remove("move2bäöü.tmp");
}

TEST_F(FileStaticTest, MoveFileLocal2USAscii)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "move3aäöü.tmp"));
    ASSERT_NO_THROW(pplib::File::move("move3aäöü.tmp", "move3b.tmp"));
    ASSERT_FALSE(pplib::File::exists("move3aäöü.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("move3b.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("move3b.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), pplib::File::md5Hash("move3b.tmp"));
    pplib::File::remove("move3b.tmp");
}

TEST_F(FileStaticTest, MoveFileLocal2Local)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "move4aäöü.tmp"));
    ASSERT_NO_THROW(pplib::File::move("move4aäöü.tmp", "move4bäöü.tmp"));
    ASSERT_FALSE(pplib::File::exists("move4aäöü.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("move4bäöü.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("move4bäöü.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), pplib::File::md5Hash("move4bäöü.tmp"));
    pplib::File::remove("move4bäöü.tmp");
}

TEST_F(FileStaticTest, RenameFileUSAscii2USAscii)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "move1a.tmp"));
    ASSERT_NO_THROW(pplib::File::rename("move1a.tmp", "move1b.tmp"));
    ASSERT_FALSE(pplib::File::exists("move1a.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("move1b.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("move1b.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), pplib::File::md5Hash("move1b.tmp"));
    pplib::File::remove("move1b.tmp");
}

TEST_F(FileStaticTest, RenameFileUSAscii2Local)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "move2a.tmp"));
    ASSERT_NO_THROW(pplib::File::rename("move2a.tmp", "move2bäöü.tmp"));
    ASSERT_FALSE(pplib::File::exists("move2a.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("move2bäöü.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("move2bäöü.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), pplib::File::md5Hash("move2bäöü.tmp"));
    pplib::File::remove("move2bäöü.tmp");
}

TEST_F(FileStaticTest, RenameFileLocal2USAscii)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "move3aäöü.tmp"));
    ASSERT_NO_THROW(pplib::File::rename("move3aäöü.tmp", "move3b.tmp"));
    ASSERT_FALSE(pplib::File::exists("move3aäöü.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("move3b.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("move3b.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), pplib::File::md5Hash("move3b.tmp"));
    pplib::File::remove("move3b.tmp");
}

TEST_F(FileStaticTest, RenameFileLocal2Local)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "move4aäöü.tmp"));
    ASSERT_NO_THROW(pplib::File::rename("move4aäöü.tmp", "move4bäöü.tmp"));
    ASSERT_FALSE(pplib::File::exists("move4aäöü.tmp")) << "Old file has disappeared";
    ASSERT_TRUE(pplib::File::exists("move4bäöü.tmp")) << "New file exists";

    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("move4bäöü.tmp", d));
    ASSERT_EQ((size_t)1787, d.Size);

    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), pplib::File::md5Hash("move4bäöü.tmp"));
    pplib::File::remove("move4bäöü.tmp");
}

TEST_F(FileStaticTest, LoadUSAscii2CString)
{
    pplib::String s;
    ASSERT_NO_THROW(pplib::File::load(s, "testdata/filenameUSASCII.txt")) << "Loading file to string";
    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), pplib::Md5(s)) << "Checking MD5-sum";
    ASSERT_EQ((size_t)1787, s.size()) << "Checking Size";
}

TEST_F(FileStaticTest, LoadFileUSAscii2ByteArray)
{
    pplib::ByteArray s;
    ASSERT_NO_THROW(pplib::File::load(s, "testdata/filenameUSASCII.txt")) << "Loading file to string";
    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), pplib::Md5(s)) << "Checking MD5-sum";
    ASSERT_EQ((size_t)1787, s.size()) << "Checking Size";
}

TEST_F(FileStaticTest, LoadFileLocal2CString)
{
    pplib::String s;
    ASSERT_NO_THROW(pplib::File::load(s, "testdata/filenameUTF8äöü.txt")) << "Loading file to string";
    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), pplib::Md5(s)) << "Checking MD5-sum";
    ASSERT_EQ((size_t)1787, s.size()) << "Checking Size";
}

TEST_F(FileStaticTest, LoadFileLocal2ByteArray)
{
    pplib::ByteArray s;
    ASSERT_NO_THROW(pplib::File::load(s, "testdata/filenameUTF8äöü.txt")) << "Loading file to string";
    ASSERT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), pplib::Md5(s)) << "Checking MD5-sum";
    ASSERT_EQ((size_t)1787, s.size()) << "Checking Size";
}

TEST_F(FileStaticTest, LoadFileUSAscii2Ptr)
{
    pplib::ByteArray data;

    ASSERT_NO_THROW({ data = pplib::File::load("testdata/filenameUSASCII.txt"); });
    ASSERT_TRUE(data.ptr() != NULL) << "Loading file to ptr";
    EXPECT_EQ((size_t)1787, data.size()) << "Checking Size";
    EXPECT_EQ(pplib::String("978fd668b5755ce6017256d0ff9e36b2"), pplib::Md5(data)) << "Checking MD5-sum";
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
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "todelete.tmp"));
    ASSERT_NO_THROW(pplib::File::remove("todelete.tmp")) << "deleting file";
    ASSERT_FALSE(pplib::File::exists("todelete.tmp")) << "file does not exists anymore";
}

TEST_F(FileStaticTest, DeleteFileUtf8)
{
    ASSERT_NO_THROW(pplib::File::copy("testdata/filenameUSASCII.txt", "todeleteäöü.tmp"));
    ASSERT_NO_THROW(pplib::File::remove("todeleteäöü.tmp")) << "deleting file";
    ASSERT_FALSE(pplib::File::exists("todeleteäöü.tmp")) << "file does not exists anymore";
}

TEST_F(FileStaticTest, TouchFileUSAscii)
{
    ASSERT_NO_THROW(pplib::File::touch("touched.tmp")) << "touching file";
    ASSERT_TRUE(pplib::File::exists("touched.tmp")) << "file exists";
    ASSERT_NO_THROW(pplib::File::remove("touched.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, TouchFileLocal)
{
    ASSERT_NO_THROW(pplib::File::touch("touchedäöü.tmp")) << "touching file";
    ASSERT_TRUE(pplib::File::exists("touchedäöü.tmp")) << "file exists";
    ASSERT_NO_THROW(pplib::File::remove("touchedäöü.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, SavePtrUSAscii)
{
    ASSERT_NO_THROW(pplib::File::save(loremipsum, strlen(loremipsum), "writetest.tmp")) << "writing file";
    ASSERT_TRUE(pplib::File::exists("writetest.tmp")) << "file exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("writetest.tmp", d)) << "Stat on file is working";
    ASSERT_EQ((size_t)strlen(loremipsum), d.Size) << "filesize as expected";
    ASSERT_EQ(pplib::String(loremipsum_md5), pplib::File::md5Hash("writetest.tmp"));
    ASSERT_NO_THROW(pplib::File::remove("writetest.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, SavePtrLocal)
{
    ASSERT_NO_THROW(pplib::File::save(loremipsum, strlen(loremipsum), "writetestäöü.tmp")) << "writing file";
    ASSERT_TRUE(pplib::File::exists("writetestäöü.tmp")) << "file exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("writetestäöü.tmp", d)) << "Stat on file is working";
    ASSERT_EQ((size_t)strlen(loremipsum), d.Size) << "filesize as expected";
    ASSERT_EQ(pplib::String(loremipsum_md5), pplib::File::md5Hash("writetestäöü.tmp"));
    ASSERT_NO_THROW(pplib::File::remove("writetestäöü.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, SaveByteArrayPtrUSAscii)
{
    pplib::ByteArrayPtr ba;
    ba.use((void*)loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW(pplib::File::save(ba, "writetest.tmp")) << "writing file";
    ASSERT_TRUE(pplib::File::exists("writetest.tmp")) << "file exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("writetest.tmp", d)) << "Stat on file is working";
    ASSERT_EQ((size_t)strlen(loremipsum), d.Size) << "filesize as expected";
    ASSERT_EQ(pplib::String(loremipsum_md5), pplib::File::md5Hash("writetest.tmp"));
    ASSERT_NO_THROW(pplib::File::remove("writetest.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, SaveByteArrayPtrLocal)
{
    pplib::ByteArrayPtr ba;
    ba.use((void*)loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW(pplib::File::save(ba, "writetestäöü.tmp")) << "writing file";
    ASSERT_TRUE(pplib::File::exists("writetestäöü.tmp")) << "file exists";
    pplib::DirEntry d;
    ASSERT_NO_THROW(pplib::File::statFile("writetestäöü.tmp", d)) << "Stat on file is working";
    ASSERT_EQ((size_t)strlen(loremipsum), d.Size) << "filesize as expected";
    ASSERT_EQ(pplib::String(loremipsum_md5), pplib::File::md5Hash("writetestäöü.tmp"));
    ASSERT_NO_THROW(pplib::File::remove("writetestäöü.tmp")) << "deleting file";
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

/*
 * TODO: Chmod und Stat interpretieren die Attribute unterschiedlich
 */
/*
TEST_F(FileStaticTest, ChmodUSAscii) {
    pplib::File::Chmod("chmod1.tmp",ppl6::FILEATTR::USR_READ|ppl6::FILEATTR::USR_WRITE);
    ASSERT_EQ(1,pplib::File::CopyFile("testdata/filenameUSASCII.txt","chmod1.tmp"));
    ASSERT_EQ(1,pplib::File::Chmod("chmod1.tmp",ppl6::FILEATTR::USR_READ));
    pplib::DirEntry d;
    ASSERT_EQ(1,pplib::File::Stat("chmod1.tmp",d)) << "Stat on file";
    d.Print();
    ASSERT_EQ(ppl6::FILEATTR::USR_READ,d.Attrib&ppl6::FILEATTR::USR_READ);
    ASSERT_NE(ppl6::FILEATTR::USR_WRITE,d.Attrib&ppl6::FILEATTR::USR_WRITE);
    pplib::File::DeleteFile("chmod1.tmp");

}
*/

} // namespace
