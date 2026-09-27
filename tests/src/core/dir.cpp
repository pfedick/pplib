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
#include <pplib/types/assocarray.h>
#include <pplib/exceptions.h>
#include <pplib/core/dir.h>
#include <pplib/core/regex.h>

#include "pplib-tests.h"

namespace
{

class DirTest : public ::testing::Test
{
protected:
    size_t expectedNum;
    DirTest()
    {

        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
        // printf ("current locale: %s\n",setlocale(LC_ALL,NULL));

        expectedNum = 10;
        if (pplib::File::exists("testdata/dirwalk/.svn")) expectedNum++;
        // if (pplib::File::exists("testdata/dirwalk/.")) expectedNum++;
        // if (pplib::File::exists("testdata/dirwalk/..")) expectedNum++;
    }
    virtual ~DirTest()
    {
    }
};

TEST_F(DirTest, ConstructorSimple)
{
    ASSERT_NO_THROW({ pplib::Dir d1; });
}

TEST_F(DirTest, ConstructorWithDir)
{
    ASSERT_NO_THROW({ pplib::Dir d1("testdata"); });
}

TEST_F(DirTest, open)
{
    pplib::Dir d1;
    ASSERT_NO_THROW({ d1.open("testdata"); });
}

TEST_F(DirTest, count)
{
    pplib::Dir d1("testdata/dirwalk");
    ASSERT_EQ(expectedNum, d1.size());
}

TEST_F(DirTest, clear)
{
    pplib::Dir d1("testdata");
    ASSERT_NO_THROW({ d1.clear(); });
    ASSERT_EQ((size_t)0, d1.size());
}

TEST_F(DirTest, print)
{
    pplib::Dir d1("testdata");
    testing::internal::CaptureStdout();
    ASSERT_NO_THROW({ d1.print(); });
    pplib::String output = testing::internal::GetCapturedStdout();
    // output.printnl();

    // Stichproben machen
    ASSERT_TRUE(output.contains("jsontest1.json"));
    ASSERT_TRUE(output.contains("test.bmp"));
    ASSERT_TRUE(output.contains("unicodeUtf8äöü.txt"));
}

TEST_F(DirTest, resortByFilenameIgnoreCase)
{
    pplib::Dir d1("testdata");
    ASSERT_NO_THROW({ d1.resort(pplib::Dir::Sort::FilenameIgnoreCase); });
}

TEST_F(DirTest, resortByMTime)
{
    pplib::Dir d1("testdata");
    ASSERT_NO_THROW({ d1.resort(pplib::Dir::Sort::MTime); });
}

TEST_F(DirTest, resortByCTime)
{
    pplib::Dir d1("testdata");
    ASSERT_NO_THROW({ d1.resort(pplib::Dir::Sort::CTime); });
}

TEST_F(DirTest, resortByATime)
{
    pplib::Dir d1("testdata");
    ASSERT_NO_THROW({ d1.resort(pplib::Dir::Sort::ATime); });
}

TEST_F(DirTest, resortNone)
{
    pplib::Dir d1("testdata");
    ASSERT_NO_THROW({ d1.resort(pplib::Dir::Sort::None); });
}

TEST_F(DirTest, resortBySize)
{
    pplib::Dir d1("testdata");
    ASSERT_NO_THROW({ d1.resort(pplib::Dir::Sort::Size); });
}

TEST_F(DirTest, dirWalkFilename)
{
    // printf ("äöü => %s\n",pplib::String::getGlobalEncoding());
    pplib::Dir d1("testdata/dirwalk", pplib::Dir::Sort::Filename);

    // d1.print();

    ASSERT_EQ(10, d1.size());

    auto it = d1.begin();
    const pplib::Array skipList(".,..,.git,.svn", ",");

    while (it != d1.end() && skipList.has(it->Filename))
        ++it;

    ASSERT_EQ(pplib::String("LICENSE.TXT"), it->Filename);
    ASSERT_EQ((size_t)1330, it->Size);

    ++it;
    ASSERT_EQ(pplib::String("afile.txt"), it->Filename);
    ASSERT_EQ((size_t)13040, it->Size);

    ++it;
    ASSERT_EQ(pplib::String("file1.txt"), it->Filename);
    ASSERT_EQ((size_t)6519, it->Size);

    ++it;
    ASSERT_EQ(pplib::String("file2.txt"), it->Filename);
    ASSERT_EQ((size_t)6519, it->Size);

    ++it;
    ASSERT_EQ(pplib::String("file3.txt"), it->Filename);
    ASSERT_EQ((size_t)6519, it->Size);

    ++it;
    ASSERT_EQ(pplib::WideString(L"file4äöü.txt"), pplib::WideString(it->Filename));
    ASSERT_EQ((size_t)5281, it->Size);

    ++it;
    ASSERT_EQ(pplib::WideString(L"file4✼.txt"), pplib::WideString(it->Filename));
    ASSERT_EQ((size_t)5287, it->Size);

    ++it;
    ASSERT_EQ(pplib::String("testfile.txt"), it->Filename);
    ASSERT_EQ((size_t)1592096, it->Size);

    ++it;
    ASSERT_EQ(pplib::String("zfile.txt"), it->Filename);
    ASSERT_EQ((size_t)9819, it->Size);

    ++it;
    ASSERT_EQ(pplib::WideString(L"èxôtíŒ.txt"), pplib::WideString(it->Filename));
    ASSERT_EQ((size_t)1356, it->Size);

    ++it;
    ASSERT_EQ(it, d1.end());
}

TEST_F(DirTest, dirWalkSize)
{

    pplib::Dir d1("testdata/dirwalk", pplib::Dir::Sort::Size);

    // d1.print();

    ASSERT_EQ(10, d1.size());

    auto it = d1.begin();
    const pplib::Array skipList(".,..,.git,.svn", ",");

    while (it != d1.end() && skipList.has(it->Filename))
        ++it;

    ASSERT_EQ(pplib::String("LICENSE.TXT"), it->Filename);
    ASSERT_EQ((size_t)1330, it->Size);

    ++it;
    ASSERT_EQ(pplib::WideString(L"èxôtíŒ.txt"), pplib::WideString(it->Filename));
    ASSERT_EQ((size_t)1356, it->Size);

    ++it;
    ASSERT_EQ(pplib::WideString(L"file4äöü.txt"), pplib::WideString(it->Filename)) << "Real Filename 2: " << it->Filename;
    ASSERT_EQ((size_t)5281, it->Size);

    ++it;
    ASSERT_EQ(pplib::WideString(L"file4✼.txt"), pplib::WideString(it->Filename));
    ASSERT_EQ((size_t)5287, it->Size);

    ++it;
    ASSERT_TRUE(pplib::RegEx::match("/^file[123].txt$/", it->Filename)) << "Real Filename 3: " << it->Filename;
    ASSERT_EQ((size_t)6519, it->Size);

    ++it;
    ASSERT_TRUE(pplib::RegEx::match("/^file[123].txt$/", it->Filename)) << "Real Filename 4: " << it->Filename;
    ASSERT_EQ((size_t)6519, it->Size);

    ++it;
    ASSERT_TRUE(pplib::RegEx::match("/^file[123].txt$/", it->Filename)) << "Real Filename 5: " << it->Filename;
    ASSERT_EQ((size_t)6519, it->Size);

    ++it;
    ASSERT_EQ(pplib::String("zfile.txt"), it->Filename) << "Real Filename 6: " << it->Filename;
    ASSERT_EQ((size_t)9819, it->Size);

    ++it;
    ASSERT_EQ(pplib::String("afile.txt"), it->Filename);
    ASSERT_EQ((size_t)13040, it->Size);

    ++it;
    ASSERT_EQ(pplib::String("testfile.txt"), it->Filename);
    ASSERT_EQ((size_t)1592096, it->Size);

    ++it;
    ASSERT_EQ(it, d1.end());
}

TEST_F(DirTest, currentPath)
{
    ASSERT_TRUE(pplib::Dir::currentPath().contains("tests"));
}

TEST_F(DirTest, homePath)
{
    pplib::String path = pplib::Dir::homePath();
    path.printnl();
#ifdef _WIN32
    ASSERT_TRUE(path.contains("Users"));
#else
    path.printnl();
#endif
}

TEST_F(DirTest, tempPath)
{
    pplib::String path = pplib::Dir::tempPath();
    ASSERT_TRUE(path.notEmpty());
}

TEST_F(DirTest, applicationDataPath)
{
    pplib::String path = pplib::Dir::applicationDataPath();
    // path.printnl();
    ASSERT_TRUE(path.notEmpty());
#ifdef _WIN32
    ASSERT_TRUE(path.contains("AppData"));
    ASSERT_TRUE(path.contains("User"));
#else
    ASSERT_TRUE(path.contains(".config"));
#endif
}

TEST_F(DirTest, applicationDataPathWithParams)
{
    pplib::String path = pplib::Dir::applicationDataPath("MyCompany", "MyApp");
    // path.printnl();
    ASSERT_TRUE(path.notEmpty());
    ASSERT_TRUE(path.contains("MyCompany"));
    ASSERT_TRUE(path.contains("MyApp"));

#ifdef _WIN32
    ASSERT_TRUE(path.contains("AppData"));
    ASSERT_TRUE(path.contains("User"));
#else
    ASSERT_TRUE(path.contains(".config"));
#endif
}

TEST_F(DirTest, documentsPath)
{
    pplib::String docPath = pplib::Dir::documentsPath();
    EXPECT_TRUE(docPath.notEmpty());
#ifdef _WIN32
    EXPECT_TRUE(docPath.contains("Documents") || docPath.contains("Dokumente") || docPath.contains("Users"));
#else
    EXPECT_TRUE(docPath.contains("Documents"));
#endif

    pplib::String appDoc = pplib::Dir::documentsPath("MyCompany", "MyApp");
    EXPECT_TRUE(appDoc.contains("MyCompany"));
    EXPECT_TRUE(appDoc.contains("MyApp"));
}

TEST_F(DirTest, AccessorsAndIterators)
{
    pplib::Dir d1("testdata/dirwalk", pplib::Dir::Sort::Filename);
    EXPECT_FALSE(d1.empty());
    EXPECT_EQ(d1.size(), 10);
    EXPECT_EQ(d1.skippedEntries(), 0);

    // operator[] und at()
    EXPECT_EQ(d1[0].Filename, "LICENSE.TXT");
    EXPECT_EQ(d1.at(0).Filename, "LICENSE.TXT");
    EXPECT_THROW(d1.at(100), std::out_of_range);

    // cbegin / cend
    size_t count = 0;
    for (auto it = d1.cbegin(); it != d1.cend(); ++it) {
        count++;
    }
    EXPECT_EQ(count, 10);

    d1.clear();
    EXPECT_TRUE(d1.empty());
    EXPECT_EQ(d1.size(), 0);
}

TEST_F(DirTest, FilterPattern)
{
    pplib::Dir d1("testdata/dirwalk");

    // Case sensitive
    auto txtUpper = d1.filterPattern("*.TXT", false);
    EXPECT_EQ(txtUpper.size(), 1);
    if (!txtUpper.empty()) {
        EXPECT_EQ(txtUpper[0].Filename, "LICENSE.TXT");
    }

    // Case insensitive
    auto txtAll = d1.filterPattern("*.TXT", true);
    EXPECT_EQ(txtAll.size(), 10);

    // Wildcard mit ?
    auto fileSingleDigit = d1.filterPattern("file?.txt");
    EXPECT_EQ(fileSingleDigit.size(), 3);

    // Pattern ohne Treffer
    auto none = d1.filterPattern("*.nonexistent");
    EXPECT_TRUE(none.empty());

    // Pattern mit Metazeichen
    auto specialPattern = d1.filterPattern("file+*.txt");
    EXPECT_TRUE(specialPattern.empty());
}

TEST_F(DirTest, FilterRegExp)
{
    pplib::Dir d1("testdata/dirwalk");

    auto matched = d1.filterRegExp("^file[123]\\.txt$");
    EXPECT_EQ(matched.size(), 3);

    auto none = d1.filterRegExp("^xyz.*");
    EXPECT_TRUE(none.empty());
}

TEST_F(DirTest, FilterPredicate)
{
    pplib::Dir d1("testdata/dirwalk");

    auto largeFiles = d1.filter([](const pplib::DirEntry& e) { return e.Size > 1000000; });
    ASSERT_EQ(largeFiles.size(), 1);
    EXPECT_EQ(largeFiles[0].Filename, "testfile.txt");
}

TEST_F(DirTest, FindPattern)
{
    pplib::Dir d1("testdata/dirwalk");

    auto found = d1.findPattern("LICENSE.*");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->Filename, "LICENSE.TXT");

    auto foundCase = d1.findPattern("license.*", true);
    ASSERT_TRUE(foundCase.has_value());
    EXPECT_EQ(foundCase->Filename, "LICENSE.TXT");

    auto notFound = d1.findPattern("does_not_exist_*");
    EXPECT_FALSE(notFound.has_value());
}

TEST_F(DirTest, FindRegExp)
{
    pplib::Dir d1("testdata/dirwalk");

    auto found = d1.findRegExp("^testfile\\..*");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->Filename, "testfile.txt");

    auto notFound = d1.findRegExp("^notfound_.*");
    EXPECT_FALSE(notFound.has_value());
}

TEST_F(DirTest, canOpen)
{
    EXPECT_TRUE(pplib::Dir::canOpen("testdata"));
    EXPECT_TRUE(pplib::Dir::canOpen(""));
    EXPECT_FALSE(pplib::Dir::canOpen("testdata/does_not_exist_xyz_123"));
}

TEST_F(DirTest, tryOpen)
{
    pplib::Dir d1;
    EXPECT_TRUE(d1.tryOpen("testdata"));
    EXPECT_FALSE(d1.empty());

    EXPECT_FALSE(d1.tryOpen("testdata/does_not_exist_xyz_123"));
}

TEST_F(DirTest, openExceptions)
{
    pplib::Dir d1;
    EXPECT_THROW(d1.open("testdata/does_not_exist_xyz_123"), pplib::FileNotFoundException);
    EXPECT_THROW(pplib::Dir d2("testdata/does_not_exist_xyz_123"), pplib::FileNotFoundException);

    // Leerer Pfad öffnet aktuelles Verzeichnis
    ASSERT_NO_THROW(d1.open(""));
    EXPECT_FALSE(d1.empty());
}

TEST_F(DirTest, exists)
{
    EXPECT_FALSE(pplib::Dir::exists(""));
    EXPECT_FALSE(pplib::Dir::exists("testdata/does_not_exist_xyz"));
    EXPECT_TRUE(pplib::Dir::exists("testdata"));
    EXPECT_FALSE(pplib::Dir::exists("testdata/dirwalk/file1.txt")); // Ist Datei, kein Verzeichnis
}

TEST_F(DirTest, mkDirAndRmDir)
{
    EXPECT_THROW(pplib::Dir::mkDir(""), pplib::IllegalArgumentException);
    EXPECT_THROW(pplib::Dir::rmDir(""), pplib::IllegalArgumentException);

    pplib::String testDir = "tmp/test_mkdir_dir/sub1/sub2";
    ASSERT_NO_THROW(pplib::Dir::rmDir("tmp/test_mkdir_dir", true));

    // Rekursives mkDir
    ASSERT_NO_THROW(pplib::Dir::mkDir(testDir, true));
    EXPECT_TRUE(pplib::Dir::exists(testDir));

    // Erneuter Aufruf auf existierendem Verzeichnis steigt frühzeitig aus
    ASSERT_NO_THROW(pplib::Dir::mkDir(testDir, true));

    // Rekursives rmDir
    ASSERT_NO_THROW(pplib::Dir::rmDir("tmp/test_mkdir_dir", true));
    EXPECT_FALSE(pplib::Dir::exists(testDir));
    EXPECT_FALSE(pplib::Dir::exists("tmp/test_mkdir_dir"));

    // Einfaches mkDir ohne Rekursion
    pplib::String singleDir = "tmp/test_single_dir";
    ASSERT_NO_THROW(pplib::Dir::mkDir(singleDir, false));
    EXPECT_TRUE(pplib::Dir::exists(singleDir));
    ASSERT_NO_THROW(pplib::Dir::mkDir(singleDir, false)); // bereits vorhanden
    ASSERT_NO_THROW(pplib::Dir::rmDir(singleDir, false));
    EXPECT_FALSE(pplib::Dir::exists(singleDir));
}

TEST_F(DirTest, rmDirDirectoryNotEmpty)
{
    pplib::String notEmptyDir = "tmp/test_not_empty_dir";
    pplib::String subDir = "tmp/test_not_empty_dir/sub";
    ASSERT_NO_THROW(pplib::Dir::mkDir(subDir, true));
    EXPECT_TRUE(pplib::Dir::exists(subDir));

    // Nicht rekursiv auf nicht-leerem Verzeichnis muss DirectoryNotEmptyException werfen
    EXPECT_THROW(pplib::Dir::rmDir(notEmptyDir, false), pplib::DirectoryNotEmptyException);
    EXPECT_TRUE(pplib::Dir::exists(notEmptyDir));

    // Rekursiv muss es erfolgreich gelöscht werden
    ASSERT_NO_THROW(pplib::Dir::rmDir(notEmptyDir, true));
    EXPECT_FALSE(pplib::Dir::exists(notEmptyDir));
}

TEST_F(DirTest, mkDirErrors)
{
    // Nicht-rekursiv fehlschlagen, wenn übergeordnetes Verzeichnis nicht existiert
    EXPECT_THROW(pplib::Dir::mkDir("tmp/non_existing_parent_dir_xyz/sub", false), pplib::FileNotFoundException);

    // Konflikt mit bestehender regulärer Datei
    pplib::String conflictFile = "tmp/test_conflict_file.txt";
    pplib::File f;
    ASSERT_NO_THROW(f.open(conflictFile, pplib::File::FileMode::WRITE));
    f.puts("test content");
    f.close();
    EXPECT_TRUE(pplib::File::exists(conflictFile));

    // mkDir auf bestehender Datei (nicht-rekursiv) wirft FileExistsException
    EXPECT_THROW(pplib::Dir::mkDir(conflictFile, false), pplib::FileExistsException);

    // mkDir mit Datei als Zwischenpfad (rekursiv) wirft Exception
    EXPECT_THROW(pplib::Dir::mkDir(conflictFile + "/sub", true), pplib::Exception);

    pplib::File::unlink(conflictFile);
    EXPECT_FALSE(pplib::File::exists(conflictFile));

#ifdef _WIN32
    // Unter Windows sind Zeichen wie '?' oder '*' in Pfadnamen ungültig
    EXPECT_THROW(pplib::Dir::mkDir("tmp/test_invalid?dir", false), pplib::Exception);
    EXPECT_THROW(pplib::Dir::mkDir("tmp/test_invalid?dir/sub", true), pplib::Exception);
#endif
}

TEST(DirEntryTest, DefaultsAndMethods)
{
    pplib::DirEntry e;
    EXPECT_EQ(e.Filename, "");
    EXPECT_EQ(e.Path, "");
    EXPECT_EQ(e.File, "");
    EXPECT_EQ(e.Size, 0);
    EXPECT_EQ(e.Uid, 0);
    EXPECT_EQ(e.Gid, 0);
    EXPECT_EQ(e.Blocks, 0);
    EXPECT_EQ(e.BlockSize, 0);
    EXPECT_EQ(e.NumLinks, 0);
    EXPECT_EQ(e.Attrib, pplib::FileAttr::NONE);

    EXPECT_FALSE(e.isDir());
    EXPECT_FALSE(e.isFile());
    EXPECT_FALSE(e.isLink());
    EXPECT_FALSE(e.isReadable());
    EXPECT_FALSE(e.isWritable());
    EXPECT_FALSE(e.isExecutable());

    e.Attrib = pplib::FileAttr::IFDIR | pplib::FileAttr::USR_READ | pplib::FileAttr::USR_WRITE | pplib::FileAttr::USR_EXECUTE;
    EXPECT_TRUE(e.isDir());
    EXPECT_FALSE(e.isFile());
    EXPECT_TRUE(e.isReadable());
    EXPECT_TRUE(e.isWritable());
    EXPECT_TRUE(e.isExecutable());

    e.Attrib = pplib::FileAttr::IFFILE;
    EXPECT_FALSE(e.isDir());
    EXPECT_TRUE(e.isFile());

    e.Attrib = pplib::FileAttr::IFLINK;
    EXPECT_TRUE(e.isLink());
}

TEST(DirEntryTest, AttrString)
{
    pplib::DirEntry e;
    e.Attrib = pplib::FileAttr::NONE;
    EXPECT_EQ(e.getAttrStr(), "----------");

    // dir + rwxrwxrwx
    e.Attrib = pplib::FileAttr::IFDIR | pplib::FileAttr::USR_READ | pplib::FileAttr::USR_WRITE | pplib::FileAttr::USR_EXECUTE |
               pplib::FileAttr::GRP_READ | pplib::FileAttr::GRP_WRITE | pplib::FileAttr::GRP_EXECUTE | pplib::FileAttr::OTH_READ |
               pplib::FileAttr::OTH_WRITE | pplib::FileAttr::OTH_EXECUTE;
    EXPECT_EQ(e.getAttrStr(), "drwxrwxrwx");

    // link + SUID (ohne exec) + SGID (ohne exec) + STICKY (ohne exec)
    e.Attrib = pplib::FileAttr::IFLINK | pplib::FileAttr::ISUID | pplib::FileAttr::ISGID | pplib::FileAttr::STICKY;
    EXPECT_EQ(e.getAttrStr(), "l--S--S--T");

    // SUID (mit exec) + SGID (mit exec) + STICKY (mit exec)
    e.Attrib = pplib::FileAttr::USR_EXECUTE | pplib::FileAttr::ISUID | pplib::FileAttr::GRP_EXECUTE | pplib::FileAttr::ISGID |
               pplib::FileAttr::OTH_EXECUTE | pplib::FileAttr::ISVTX;
    EXPECT_EQ(e.getAttrStr(), "---s--s--t");
}

TEST(DirEntryTest, ToArrayAndPrint)
{
    pplib::DirEntry e;
    e.Filename = "test.txt";
    e.Path = "/path/to";
    e.File = "/path/to/test.txt";
    e.Size = 42;
    e.Attrib = pplib::FileAttr::IFFILE | pplib::FileAttr::USR_READ;
    e.Uid = 1000;
    e.Gid = 1000;
    e.Blocks = 8;
    e.BlockSize = 4096;
    e.NumLinks = 1;
    e.ATime.setCurrentTime();
    e.CTime.setCurrentTime();
    e.MTime.setCurrentTime();

    pplib::AssocArray a;
    e.toArray(a);
    EXPECT_EQ(a.getString("filename"), "test.txt");
    EXPECT_EQ(a.getString("path"), "/path/to");
    EXPECT_EQ(a.getString("file"), "/path/to/test.txt");
    EXPECT_EQ(a.getInt("size"), 42);
    EXPECT_EQ(a.getInt("uid"), 1000);
    EXPECT_EQ(a.getInt("gid"), 1000);
    EXPECT_EQ(a.getInt("blocks"), 8);
    EXPECT_EQ(a.getInt("blocksize"), 4096);
    EXPECT_EQ(a.getInt("numlinks"), 1);
    EXPECT_TRUE(a.exists("attribstr"));
    EXPECT_TRUE(a.exists("atime"));
    EXPECT_TRUE(a.exists("ctime"));
    EXPECT_TRUE(a.exists("mtime"));

    testing::internal::CaptureStdout();
    e.print("MyEntry");
    pplib::String output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.contains("test.txt"));
    EXPECT_TRUE(output.contains("MyEntry"));
}

} // namespace
