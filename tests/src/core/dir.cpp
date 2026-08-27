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

#include "pplib-tests.h"

namespace
{

class DirTest : public ::testing::Test
{
protected:
    size_t expectedNum;
    DirTest()
    {
        /*
        if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
            printf ("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
        */
        if (setlocale(LC_ALL, "") == NULL) {
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
    ASSERT_EQ((size_t)5281, it->Size);

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

#ifdef TODO
pplib::DirEntry getNextFile(const pplib::Dir& d, pplib::Dir::Iterator& it)
{
    pplib::DirEntry e;
    while (1) {
        e = d.getNext(it);
        if (e.Filename != "." && e.Filename != ".." && e.Filename != ".svn") break;
    }
    return e;
}

TEST_F(DirTest, dirWalkSize)
{
    pplib::Dir d1;
    ASSERT_NO_THROW({ d1.open("testdata/dirwalk", pplib::Dir::Sort::Size); });
    pplib::Dir::Iterator it;
    // d1.print();
    d1.reset(it);
    pplib::DirEntry e;
    ASSERT_NO_THROW({ e = getNextFile(d1, it); });
    ASSERT_EQ(pplib::String("LICENSE.TXT"), e.Filename) << "Real Filename 1: " << e.Filename;
    ASSERT_EQ((size_t)1330, e.Size);

    ASSERT_NO_THROW(e = getNextFile(d1, it));
    ASSERT_EQ(pplib::WideString(L"èxôtíŒ.txt"), pplib::WideString(e.Filename));
    ASSERT_EQ((size_t)1356, e.Size);

    ASSERT_NO_THROW(e = getNextFile(d1, it));
    ASSERT_EQ(pplib::WideString(L"file4✼.txt"), pplib::WideString(e.Filename));
    ASSERT_EQ((size_t)5281, e.Size);

    ASSERT_NO_THROW(e = getNextFile(d1, it));
    ASSERT_EQ(pplib::WideString(L"file4äöü.txt"), pplib::WideString(e.Filename)) << "Real Filename 2: " << e.Filename;
    ASSERT_EQ((size_t)5281, e.Size);

    ASSERT_NO_THROW(e = getNextFile(d1, it));
    ASSERT_TRUE(pplib::RegEx::match("/^file[123].txt$/", e.Filename)) << "Real Filename 3: " << e.Filename;
    ASSERT_EQ((size_t)6519, e.Size);

    ASSERT_NO_THROW({ e = getNextFile(d1, it); });
    ASSERT_TRUE(pplib::RegEx::match("/^file[123].txt$/", e.Filename)) << "Real Filename 4: " << e.Filename;
    ASSERT_EQ((size_t)6519, e.Size);

    ASSERT_NO_THROW({ e = getNextFile(d1, it); });
    ASSERT_TRUE(pplib::RegEx::match("/^file[123].txt$/", e.Filename)) << "Real Filename 5: " << e.Filename;
    ASSERT_EQ((size_t)6519, e.Size);

    ASSERT_NO_THROW({ e = getNextFile(d1, it); });
    ASSERT_EQ(pplib::String("zfile.txt"), e.Filename) << "Real Filename 6: " << e.Filename;
    ASSERT_EQ((size_t)9819, e.Size);

    ASSERT_NO_THROW({ e = getNextFile(d1, it); });
    ASSERT_EQ(pplib::String("afile.txt"), e.Filename);
    ASSERT_EQ((size_t)13040, e.Size);

    ASSERT_NO_THROW({ e = getNextFile(d1, it); });
    ASSERT_EQ(pplib::String("testfile.txt"), e.Filename);
    ASSERT_EQ((size_t)1592096, e.Size);

    // We expect an EndOfListException next
    ASSERT_THROW(e = getNextFile(d1, it);, pplib::EndOfListException);
}

TEST_F(DirTest, patternWalk)
{
    pplib::Dir d1("testdata/dirwalk", pplib::Dir::Sort::Filename);
    pplib::Dir::Iterator it;
    // d1.print();
    d1.reset(it);
    pplib::DirEntry e;
    e = d1.getNextPattern(it, "file*");
    ASSERT_EQ(pplib::String("file1.txt"), e.Filename);
    ASSERT_EQ((size_t)6519, e.Size);

    e = d1.getNextPattern(it, "file*");
    ASSERT_EQ(pplib::String("file2.txt"), e.Filename);
    ASSERT_EQ((size_t)6519, e.Size);

    e = d1.getNextPattern(it, "file*");
    ASSERT_EQ(pplib::String("file3.txt"), e.Filename);
    ASSERT_EQ((size_t)6519, e.Size);

    e = d1.getNextPattern(it, "file*");
    ASSERT_EQ(pplib::WideString(L"file4äöü.txt"), pplib::WideString(e.Filename));
    ASSERT_EQ((size_t)5281, e.Size);

    e = d1.getNextPattern(it, "file*");
    ASSERT_EQ(pplib::WideString(L"file4✼.txt"), pplib::WideString(e.Filename));
    ASSERT_EQ((size_t)5281, e.Size);

    // We expect an EndOfListException next
    ASSERT_THROW(e = d1.getNextPattern(it, "file*"), pplib::EndOfListException);
}

TEST_F(DirTest, patternWalk2)
{
    pplib::Dir d1("testdata/dirwalk", pplib::Dir::Sort::Filename);
    pplib::Dir::Iterator it;
    // d1.print();
    d1.reset(it);
    pplib::DirEntry e;
    ASSERT_TRUE(d1.getNextPattern(e, it, "file*"));
    ASSERT_EQ(pplib::String("file1.txt"), e.Filename);
    ASSERT_EQ((size_t)6519, e.Size);

    ASSERT_TRUE(d1.getNextPattern(e, it, "file*"));
    ASSERT_EQ(pplib::String("file2.txt"), e.Filename);
    ASSERT_EQ((size_t)6519, e.Size);

    ASSERT_TRUE(d1.getNextPattern(e, it, "file*"));
    ASSERT_EQ(pplib::String("file3.txt"), e.Filename);
    ASSERT_EQ((size_t)6519, e.Size);

    ASSERT_TRUE(d1.getNextPattern(e, it, "file*"));
    ASSERT_EQ(pplib::WideString(L"file4äöü.txt"), pplib::WideString(e.Filename));
    ASSERT_EQ((size_t)5281, e.Size);

    ASSERT_TRUE(d1.getNextPattern(e, it, "file*"));
    ASSERT_EQ(pplib::WideString(L"file4✼.txt"), pplib::WideString(e.Filename));
    ASSERT_EQ((size_t)5281, e.Size);

    // We expect an EndOfListException next
    ASSERT_FALSE(d1.getNextPattern(e, it, "file*"));
}

TEST_F(DirTest, regExpWalk)
{
    pplib::Dir d1("testdata/dirwalk", pplib::Dir::Sort::Filename);
    pplib::Dir::Iterator it;
    pplib::String expr("/^.file.*/i");
    // d1.print();
    d1.reset(it);
    pplib::DirEntry e;
    e = d1.getNextRegExp(it, expr);
    ASSERT_EQ(pplib::String("afile.txt"), e.Filename);
    ASSERT_EQ((size_t)13040, e.Size);

    e = d1.getNextRegExp(it, expr);
    ASSERT_EQ(pplib::String("zfile.txt"), e.Filename);
    ASSERT_EQ((size_t)9819, e.Size);

    // We expect an EndOfListException next
    ASSERT_THROW(e = d1.getNextRegExp(it, "file*"), pplib::EndOfListException);
}

TEST_F(DirTest, regExpWalk2)
{
    pplib::Dir d1("testdata/dirwalk", pplib::Dir::Sort::Filename);
    pplib::Dir::Iterator it;
    pplib::String expr("/^.file.*/i");
    // d1.print();
    d1.reset(it);
    pplib::DirEntry e;
    ASSERT_TRUE(d1.getNextRegExp(e, it, expr));
    ASSERT_EQ(pplib::String("afile.txt"), e.Filename);
    ASSERT_EQ((size_t)13040, e.Size);

    ASSERT_TRUE(d1.getNextRegExp(e, it, expr));
    ASSERT_EQ(pplib::String("zfile.txt"), e.Filename);
    ASSERT_EQ((size_t)9819, e.Size);

    // We expect an EndOfListException next
    ASSERT_FALSE(d1.getNextRegExp(e, it, expr));
}
#endif
} // namespace
