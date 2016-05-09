/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <ppl7.h>
#include <gtest/gtest.h>
#include "ppl7-tests.h"

namespace {

// The fixture for testing class Foo.
class DirTest : public ::testing::Test {
	protected:
	size_t	expectedNum;
	DirTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		expectedNum=8;
		if (ppl7::File::exists("testdata/.svn")) expectedNum++;
		if (ppl7::File::exists("testdata/.")) expectedNum++;
		if (ppl7::File::exists("testdata/..")) expectedNum++;

	}
	virtual ~DirTest() {

	}
};


TEST_F(DirTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl7::Dir d1;
	}
	);
}

TEST_F(DirTest, ConstructorWithDir) {
	ASSERT_NO_THROW({
		ppl7::Dir d1("testdata");
	}
	);
}

TEST_F(DirTest, open) {
	ppl7::Dir d1;
	ASSERT_NO_THROW({
		d1.open("testdata");
	}
	);
}

TEST_F(DirTest, count) {
	ppl7::Dir d1("testdata/dirwalk");
	ASSERT_EQ(expectedNum,d1.count());
}

TEST_F(DirTest, clear) {
	ppl7::Dir d1("testdata");
	ASSERT_NO_THROW({
		d1.clear();
	});
	ASSERT_EQ((size_t)0,d1.count());
}


TEST_F(DirTest, print) {
	ppl7::Dir d1("testdata");
	ASSERT_NO_THROW({
		d1.print();
	}
	);
}

TEST_F(DirTest, resortByFilenameIgnoreCase) {
	ppl7::Dir d1("testdata");
	ASSERT_NO_THROW({
		d1.resort(ppl7::Dir::SORT_FILENAME);
	});
}

TEST_F(DirTest, resortByMTime) {
	ppl7::Dir d1("testdata");
	ASSERT_NO_THROW({
		d1.resort(ppl7::Dir::SORT_MTIME);
	});
}

TEST_F(DirTest, resortBySize) {
	ppl7::Dir d1("testdata");
	ASSERT_NO_THROW({
		d1.resort(ppl7::Dir::SORT_SIZE);
	});
}

TEST_F(DirTest, dirWalkFilename) {
	ppl7::Dir d1("testdata/dirwalk", ppl7::Dir::SORT_FILENAME);
	ppl7::Dir::Iterator it;
	//d1.print();
	d1.reset(it);
	ppl7::DirEntry e;
	while (1) {
		e=d1.getNext(it);
		if (e.Filename!="." && e.Filename!=".." && e.Filename!=".svn") break;
	}
	ASSERT_EQ(ppl7::String("LICENSE.TXT"),e.Filename);
	ASSERT_EQ((size_t)1330,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String("afile.txt"),e.Filename);
	ASSERT_EQ((size_t)13040,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String("file1.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String("file2.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String("file3.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String("file4äöü.txt"),e.Filename);
	ASSERT_EQ((size_t)5281,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String("testfile.txt"),e.Filename);
	ASSERT_EQ((size_t)1592096,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String("zfile.txt"),e.Filename);
	ASSERT_EQ((size_t)9819,e.Size);

	// We expect an EndOfListException next
	ASSERT_THROW(e=d1.getNext(it), ppl7::EndOfListException);

}


ppl7::DirEntry getNextFile(const ppl7::Dir &d, ppl7::Dir::Iterator &it)
{
	ppl7::DirEntry e;
	while (1) {
		e=d.getNext(it);
		if (e.Filename!="." && e.Filename!=".." && e.Filename!=".svn") break;
	}
	return e;
}

TEST_F(DirTest, dirWalkSize) {
	ppl7::Dir d1;
	ASSERT_NO_THROW({
		d1.open("testdata/dirwalk", ppl7::Dir::SORT_SIZE);
	});
	ppl7::Dir::Iterator it;
	//d1.print();
	d1.reset(it);
	ppl7::DirEntry e;
	ASSERT_NO_THROW({
		e=getNextFile(d1,it);
	});
	ASSERT_EQ(ppl7::String("LICENSE.TXT"),e.Filename )<< "Real Filename 1: "<<e.Filename;
	ASSERT_EQ((size_t)1330,e.Size);

	ASSERT_NO_THROW(e=getNextFile(d1,it));
	ASSERT_EQ(ppl7::String("file4äöü.txt"),e.Filename )<< "Real Filename 2: "<<e.Filename;
	ASSERT_EQ((size_t)5281,e.Size);

	ASSERT_NO_THROW(e=getNextFile(d1,it));
	ASSERT_TRUE(e.Filename.pregMatch("/^file[123].txt$/")) << "Real Filename 3: "<<e.Filename;
	ASSERT_EQ((size_t)6519,e.Size);

	ASSERT_NO_THROW({
		e=getNextFile(d1,it);
	});
	ASSERT_TRUE(e.Filename.pregMatch("/^file[123].txt$/")) << "Real Filename 4: "<<e.Filename;
	ASSERT_EQ((size_t)6519,e.Size);

	ASSERT_NO_THROW({
		e=getNextFile(d1,it);
	});
	ASSERT_TRUE(e.Filename.pregMatch("/^file[123].txt$/")) << "Real Filename 5: "<<e.Filename;
	ASSERT_EQ((size_t)6519,e.Size);

	ASSERT_NO_THROW({
		e=getNextFile(d1,it);
	});
	ASSERT_EQ(ppl7::String("zfile.txt"),e.Filename) << "Real Filename 6: "<<e.Filename;
	ASSERT_EQ((size_t)9819,e.Size);

	ASSERT_NO_THROW({
		e=getNextFile(d1,it);
	});
	ASSERT_EQ(ppl7::String("afile.txt"),e.Filename);
	ASSERT_EQ((size_t)13040,e.Size);

	ASSERT_NO_THROW({
		e=getNextFile(d1,it);
	});
	ASSERT_EQ(ppl7::String("testfile.txt"),e.Filename);
	ASSERT_EQ((size_t)1592096,e.Size);

	// We expect an EndOfListException next
	ASSERT_THROW(e=getNextFile(d1,it);, ppl7::EndOfListException);
}


TEST_F(DirTest, patternWalk) {
	ppl7::Dir d1("testdata/dirwalk", ppl7::Dir::SORT_FILENAME);
	ppl7::Dir::Iterator it;
	//d1.print();
	d1.reset(it);
	ppl7::DirEntry e;
	e=d1.getNextPattern(it,"file*");
	ASSERT_EQ(ppl7::String("file1.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	e=d1.getNextPattern(it,"file*");
	ASSERT_EQ(ppl7::String("file2.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	e=d1.getNextPattern(it,"file*");
	ASSERT_EQ(ppl7::String("file3.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	e=d1.getNextPattern(it,"file*");
	ASSERT_EQ(ppl7::String("file4äöü.txt"),e.Filename);
	ASSERT_EQ((size_t)5281,e.Size);


	// We expect an EndOfListException next
	ASSERT_THROW(e=d1.getNextPattern(it,"file*"), ppl7::EndOfListException);

}


TEST_F(DirTest, patternWalk2) {
	ppl7::Dir d1("testdata/dirwalk", ppl7::Dir::SORT_FILENAME);
	ppl7::Dir::Iterator it;
	//d1.print();
	d1.reset(it);
	ppl7::DirEntry e;
	ASSERT_TRUE(d1.getNextPattern(e,it,"file*"));
	ASSERT_EQ(ppl7::String("file1.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	ASSERT_TRUE(d1.getNextPattern(e,it,"file*"));
	ASSERT_EQ(ppl7::String("file2.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	ASSERT_TRUE(d1.getNextPattern(e,it,"file*"));
	ASSERT_EQ(ppl7::String("file3.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	ASSERT_TRUE(d1.getNextPattern(e,it,"file*"));
	ASSERT_EQ(ppl7::String("file4äöü.txt"),e.Filename);
	ASSERT_EQ((size_t)5281,e.Size);

	// We expect an EndOfListException next
	ASSERT_FALSE(d1.getNextPattern(e,it,"file*"));

}

TEST_F(DirTest, regExpWalk) {
	ppl7::Dir d1("testdata/dirwalk", ppl7::Dir::SORT_FILENAME);
	ppl7::Dir::Iterator it;
	ppl7::String expr("/^.file.*/i");
	//d1.print();
	d1.reset(it);
	ppl7::DirEntry e;
	e=d1.getNextRegExp(it,expr);
	ASSERT_EQ(ppl7::String("afile.txt"),e.Filename);
	ASSERT_EQ((size_t)13040,e.Size);

	e=d1.getNextRegExp(it,expr);
	ASSERT_EQ(ppl7::String("zfile.txt"),e.Filename);
	ASSERT_EQ((size_t)9819,e.Size);


	// We expect an EndOfListException next
	ASSERT_THROW(e=d1.getNextRegExp(it,"file*"), ppl7::EndOfListException);

}

TEST_F(DirTest, regExpWalk2) {
	ppl7::Dir d1("testdata/dirwalk", ppl7::Dir::SORT_FILENAME);
	ppl7::Dir::Iterator it;
	ppl7::String expr("/^.file.*/i");
	//d1.print();
	d1.reset(it);
	ppl7::DirEntry e;
	ASSERT_TRUE(d1.getNextRegExp(e,it,expr));
	ASSERT_EQ(ppl7::String("afile.txt"),e.Filename);
	ASSERT_EQ((size_t)13040,e.Size);

	ASSERT_TRUE(d1.getNextRegExp(e,it,expr));
	ASSERT_EQ(ppl7::String("zfile.txt"),e.Filename);
	ASSERT_EQ((size_t)9819,e.Size);


	// We expect an EndOfListException next
	ASSERT_FALSE(d1.getNextRegExp(e,it,expr));

}

}

