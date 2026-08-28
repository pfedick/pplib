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
    ASSERT_TRUE(output.contains("test_192cbr.mp3"));
    ASSERT_TRUE(output.contains("unicodeUtf8äöü.txt"));
}

TEST_F(DirTest, resortByFilenameIgnoreCase)
{
    pplib::Dir d1("testdata");
    ASSERT_NO_THROW({ d1.resort(pplib::Dir::Sort::Filename); });
}

TEST_F(DirTest, resortByMTime)
{
    pplib::Dir d1("testdata");
    ASSERT_NO_THROW({ d1.resort(pplib::Dir::Sort::MTime); });
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

} // namespace
