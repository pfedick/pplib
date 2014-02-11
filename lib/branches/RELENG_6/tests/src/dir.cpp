/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2014, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include "../include/prolog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include "../include/ppl6.h"
#include <gtest/gtest.h>
#include "ppl6-tests.h"
#include "../include/config.h"

namespace {

// The fixture for testing class Foo.
class DirTest : public ::testing::Test {
	protected:
	int	expectedNum;
	DirTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl6::SetGlobalEncoding("UTF-8");
		expectedNum=8;
		if (ppl6::CFile::Exists("testdata/dirwalk/.svn")) expectedNum++;
		if (ppl6::CFile::Exists("testdata/dirwalk/.")) expectedNum++;
		if (ppl6::CFile::Exists("testdata/dirwalk/..")) expectedNum++;

	}
	virtual ~DirTest() {

	}
};


TEST_F(DirTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl6::CDir d1;
	}
	);
}

TEST_F(DirTest, open) {
	ppl6::CDir d1;
	ASSERT_EQ(1,d1.Open("testdata/dirwalk"));
}

TEST_F(DirTest, ConstructorWithDir) {
	ASSERT_NO_THROW({
		ppl6::CDir d1("testdata/dirwalk");
	}
	);
}



TEST_F(DirTest, count) {
	ppl6::CDir d1("testdata/dirwalk");
	ASSERT_EQ(expectedNum,d1.Num());
}

TEST_F(DirTest, clear) {
	ppl6::CDir d1("testdata");
	ASSERT_NO_THROW({
		d1.Clear();
	});
	ASSERT_EQ((int)0,d1.Num());
}

TEST_F(DirTest, print) {
	ppl6::CDir d1("testdata");
	ASSERT_NO_THROW({
		d1.Print();
	}
	);
}

TEST_F(DirTest, resortByFilenameIgnoreCase) {
	ppl6::CDir d1("testdata");
	ASSERT_NO_THROW({
		d1.Resort(ppl6::CDir::Sort_Filename_IgnoreCase);
	});
}


TEST_F(DirTest, resortByDate) {
	ppl6::CDir d1("testdata");
	ASSERT_NO_THROW({
		d1.Resort(ppl6::CDir::Sort_Date);
	});
}

TEST_F(DirTest, dirWalkFilename) {
	ppl6::CDir d1("testdata/dirwalk", ppl6::CDir::Sort_Filename);
	ppl6::CDirEntry *e;
	while (1) {
		e=d1.GetNext();
		ASSERT_TRUE(e!=NULL);
		if (e->Filename!="." && e->Filename!=".." && e->Filename!=".svn") break;
	}
	ASSERT_EQ(ppl6::CString("LICENSE.TXT"),e->Filename);
	ASSERT_EQ((size_t)1330,e->Size);

	e=d1.GetNext();
	ASSERT_TRUE(e!=NULL);
	ASSERT_EQ(ppl6::CString("afile.txt"),e->Filename);
	ASSERT_EQ((size_t)13040,e->Size);

	e=d1.GetNext();
	ASSERT_TRUE(e!=NULL);
	ASSERT_EQ(ppl6::CString("file1.txt"),e->Filename);
	ASSERT_EQ((size_t)6519,e->Size);

	e=d1.GetNext();
	ASSERT_TRUE(e!=NULL);
	ASSERT_EQ(ppl6::CString("file2.txt"),e->Filename);
	ASSERT_EQ((size_t)6519,e->Size);

	e=d1.GetNext();
	ASSERT_TRUE(e!=NULL);
	ASSERT_EQ(ppl6::CString("file3.txt"),e->Filename);
	ASSERT_EQ((size_t)6519,e->Size);

	e=d1.GetNext();
	ASSERT_TRUE(e!=NULL);
	ASSERT_EQ(ppl6::CString("file4äöü.txt"),e->Filename);
	ASSERT_EQ((size_t)6519,e->Size);

	e=d1.GetNext();
	ASSERT_TRUE(e!=NULL);
	ASSERT_EQ(ppl6::CString("testfile.txt"),e->Filename);
	ASSERT_EQ((size_t)1592096,e->Size);

	e=d1.GetNext();
	ASSERT_TRUE(e!=NULL);
	ASSERT_EQ(ppl6::CString("zfile.txt"),e->Filename);
	ASSERT_EQ((size_t)9819,e->Size);

	// Nix mehr in der Liste
	ASSERT_TRUE(NULL==d1.GetNext());

}


ppl6::CDirEntry *getNextFile(ppl6::CDir &d)
{
	ppl6::CDirEntry *e;
	while (1) {
		e=d.GetNext();
		if (!e) break;
		if (e->Filename!="." && e->Filename!=".." && e->Filename!=".svn") break;
	}
	return e;
}

TEST_F(DirTest, patternWalk) {
	ppl6::CDir d1("testdata/dirwalk", ppl6::CDir::Sort_Filename);
	ppl6::CDirEntry *e;
	e=d1.GetNextPattern("file*");
	ASSERT_TRUE(e!=NULL);
	ASSERT_EQ(ppl6::CString("file1.txt"),e->Filename);
	ASSERT_EQ((size_t)6519,e->Size);

	e=d1.GetNextPattern("file*");
	ASSERT_TRUE(e!=NULL);
	ASSERT_EQ(ppl6::CString("file2.txt"),e->Filename);
	ASSERT_EQ((size_t)6519,e->Size);

	e=d1.GetNextPattern("file*");
	ASSERT_TRUE(e!=NULL);
	ASSERT_EQ(ppl6::CString("file3.txt"),e->Filename);
	ASSERT_EQ((size_t)6519,e->Size);

	e=d1.GetNextPattern("file*");
	ASSERT_TRUE(e!=NULL);
	ASSERT_EQ(ppl6::CString("file4äöü.txt"),e->Filename);
	ASSERT_EQ((size_t)6519,e->Size);

	e=d1.GetNextPattern("file*");
	ASSERT_TRUE(e==NULL);


	// We expect an EndOfListException next
	//ASSERT_THROW(e=d1.getNextPattern(it,"file*"), ppl7::EndOfListException);

}


TEST_F(DirTest, regExpWalk) {
	ppl6::CDir d1("testdata/dirwalk", ppl6::CDir::Sort_Filename);
	ppl6::CString expr("/^.file.*/i");
	ppl6::CDirEntry *e;
	e=d1.GetNextRegExp(expr);
	ASSERT_TRUE(e!=NULL);
	ASSERT_EQ(ppl6::CString("afile.txt"),e->Filename);
	ASSERT_EQ((size_t)13040,e->Size);

	e=d1.GetNextRegExp(expr);
	ASSERT_TRUE(e!=NULL);
	ASSERT_EQ(ppl6::CString("zfile.txt"),e->Filename);
	ASSERT_EQ((size_t)9819,e->Size);

	e=d1.GetNextRegExp(expr);
	ASSERT_TRUE(e==NULL);

}



}

