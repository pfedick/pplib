/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 108 $
 * $Date: 2011-05-20 16:55:16 +0200 (Fr, 20. Mai 2011) $
 * $Id: file.cpp 108 2011-05-20 14:55:16Z pafe $
 *
 *******************************************************************************
 * Copyright (c) 2010, Patrick Fedick <patrick@pfp.de>
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
#include <pthread.h>
#include <locale.h>
#include "../include/ppl7.h"
#include <gtest/gtest.h>

namespace {

// The fixture for testing class Foo.
class DirTest : public ::testing::Test {
	protected:
	size_t	expectedNum;
	DirTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
		expectedNum=23;
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
	ppl7::Dir d1("testdata");
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
	ppl7::Dir d1("testdata", ppl7::Dir::SORT_FILENAME);
	ppl7::Dir::Iterator it;
	//d1.print();
	d1.reset(it);
	ppl7::DirEntry e;
	while (1) {
		e=d1.getNext(it);
		if (e.Filename!="." && e.Filename!=".." && e.Filename!=".svn") break;
	}
	ASSERT_EQ(ppl7::String(L"File1.txt"),e.Filename);
	ASSERT_EQ((size_t)13719,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"LICENSE.TXT"),e.Filename);
	ASSERT_EQ((size_t)1540,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"LiberationSans-Bold.ttf"),e.Filename);
	ASSERT_EQ((size_t)140252,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"afile.txt"),e.Filename);
	ASSERT_EQ((size_t)13040,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"file1.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"file2.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"file3.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"ppl7-icon-64x64.png"),e.Filename);
	ASSERT_EQ((size_t)8685,e.Size);


	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"reference.png"),e.Filename);
	ASSERT_EQ((size_t)239844,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test-pal-trans.png"),e.Filename);
	ASSERT_EQ((size_t)8957,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test-pal.png"),e.Filename);
	ASSERT_EQ((size_t)8976,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.bmp"),e.Filename);
	ASSERT_EQ((size_t)34254,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.gif"),e.Filename);
	ASSERT_EQ((size_t)10117,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.jpg"),e.Filename);
	ASSERT_EQ((size_t)7835,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.pcx"),e.Filename);
	ASSERT_EQ((size_t)33872,e.Size);


	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.png"),e.Filename);
	ASSERT_EQ((size_t)23150,e.Size);


	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.ppm"),e.Filename);
	ASSERT_EQ((size_t)34244,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.tga"),e.Filename);
	ASSERT_EQ((size_t)30032,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.tif"),e.Filename);
	ASSERT_EQ((size_t)26704,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"testfile.txt"),e.Filename);
	ASSERT_EQ((size_t)1592096,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"unittest.bmp"),e.Filename);
	ASSERT_EQ((size_t)16438,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"unittest.png"),e.Filename);
	ASSERT_EQ((size_t)1572,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"zfile.txt"),e.Filename);
	ASSERT_EQ((size_t)9819,e.Size);

	// We expect an EndOfListException next
	ASSERT_THROW(e=d1.getNext(it), ppl7::EndOfListException);

}

TEST_F(DirTest, dirWalkFilename2) {
	ppl7::Dir d1("testdata", ppl7::Dir::SORT_FILENAME);
	ppl7::Dir::Iterator it;
	//d1.print();
	d1.reset(it);
	ppl7::DirEntry e;
	while (d1.getNext(e,it)) {
		if (e.Filename!="." && e.Filename!=".." && e.Filename!=".svn") break;
	}
	ASSERT_EQ(ppl7::String(L"File1.txt"),e.Filename);
	ASSERT_EQ((size_t)13719,e.Size);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"LICENSE.TXT"),e.Filename);
	ASSERT_EQ((size_t)1540,e.Size);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"LiberationSans-Bold.ttf"),e.Filename);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"afile.txt"),e.Filename);
	ASSERT_EQ((size_t)13040,e.Size);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"file1.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"file2.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"file3.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"ppl7-icon-64x64.png"),e.Filename);
	ASSERT_EQ((size_t)8685,e.Size);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"reference.png"),e.Filename);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"test-pal-trans.png"),e.Filename);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"test-pal.png"),e.Filename);


	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"test.bmp"),e.Filename);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"test.gif"),e.Filename);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"test.jpg"),e.Filename);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"test.pcx"),e.Filename);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"test.png"),e.Filename);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"test.ppm"),e.Filename);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"test.tga"),e.Filename);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"test.tif"),e.Filename);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"testfile.txt"),e.Filename);
	ASSERT_EQ((size_t)1592096,e.Size);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"unittest.bmp"),e.Filename);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"unittest.png"),e.Filename);

	ASSERT_TRUE(d1.getNext(e,it));
	ASSERT_EQ(ppl7::String(L"zfile.txt"),e.Filename);
	ASSERT_EQ((size_t)9819,e.Size);

	// We expect an EndOfList next
	ASSERT_FALSE(d1.getNext(e,it));


}

/*
-rw-r--r--  1 patrick  admins     1540 22 Mai 11:31 LICENSE.TXT
-rw-r--r--  1 patrick  admins     1572 12 Jun 16:37 unittest.png
-rw-r--r--  1 patrick  admins     6519 28 Mai 11:04 file3.txt
-rw-r--r--  1 patrick  admins     6519 28 Mai 11:04 file2.txt
-rw-r--r--  1 patrick  admins     6519 28 Mai 11:04 file1.txt
-rw-r--r--  1 patrick  admins     7835 12 Jun 16:37 test.jpg
-rwxr-xr-x  1 patrick  admins     8685 22 Mai 11:31 ppl7-icon-64x64.png
-rw-r--r--  1 patrick  admins     9819 28 Mai 11:09 zfile.txt
-rw-r--r--  1 patrick  admins    10117 12 Jun 16:37 test.gif
-rw-r--r--  1 patrick  admins    13040 28 Mai 11:08 afile.txt
-rw-r--r--  1 patrick  admins    13719 28 Mai 11:03 File1.txt
-rw-r--r--  1 patrick  admins    16438 12 Jun 16:37 unittest.bmp
-rw-r--r--  1 patrick  admins    23150 12 Jun 16:37 test.png
-rw-r--r--  1 patrick  admins    26704 12 Jun 16:37 test.tif
-rw-r--r--  1 patrick  admins    30032 12 Jun 16:37 test.tga
-rw-r--r--  1 patrick  admins    33872 12 Jun 16:37 test.pcx
-rw-r--r--  1 patrick  admins    34244 12 Jun 16:37 test.ppm
-rw-r--r--  1 patrick  admins    34254 12 Jun 16:37 test.bmp
-rw-r--r--  1 patrick  admins   140252 12 Jun 16:37 LiberationSans-Bold.ttf
-rw-r--r--  1 patrick  admins   231923 12 Jun 16:37 reference.png
-rw-r--r--  1 patrick  admins  1592096 22 Mai 11:31 testfile.txt
*/

TEST_F(DirTest, dirWalkSize) {
	ppl7::Dir d1("testdata", ppl7::Dir::SORT_SIZE);
	ppl7::Dir::Iterator it;
	//d1.print();
	d1.reset(it);
	ppl7::DirEntry e;
	while (1) {
		e=d1.getNext(it);
		if (e.Filename!="." && e.Filename!=".." && e.Filename!=".svn") break;
	}
	ASSERT_EQ(ppl7::String(L"LICENSE.TXT"),e.Filename);
	ASSERT_EQ((size_t)1540,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"unittest.png"),e.Filename);
	ASSERT_EQ((size_t)1572,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"file2.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"file3.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"file1.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.jpg"),e.Filename);
	ASSERT_EQ((size_t)7835,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"ppl7-icon-64x64.png"),e.Filename);
	ASSERT_EQ((size_t)8685,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test-pal-trans.png"),e.Filename);
	ASSERT_EQ((size_t)8957,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test-pal.png"),e.Filename);
	ASSERT_EQ((size_t)8976,e.Size);


	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"zfile.txt"),e.Filename);
	ASSERT_EQ((size_t)9819,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.gif"),e.Filename);
	ASSERT_EQ((size_t)10117,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"afile.txt"),e.Filename);
	ASSERT_EQ((size_t)13040,e.Size);


	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"File1.txt"),e.Filename);
	ASSERT_EQ((size_t)13719,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"unittest.bmp"),e.Filename);
	ASSERT_EQ((size_t)16438,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.png"),e.Filename);
	ASSERT_EQ((size_t)23150,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.tif"),e.Filename);
	ASSERT_EQ((size_t)26704,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.tga"),e.Filename);
	ASSERT_EQ((size_t)30032,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.pcx"),e.Filename);
	ASSERT_EQ((size_t)33872,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.ppm"),e.Filename);
	ASSERT_EQ((size_t)34244,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"test.bmp"),e.Filename);
	ASSERT_EQ((size_t)34254,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"LiberationSans-Bold.ttf"),e.Filename);
	ASSERT_EQ((size_t)140252,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"reference.png"),e.Filename);
	ASSERT_EQ((size_t)239844,e.Size);

	e=d1.getNext(it);
	ASSERT_EQ(ppl7::String(L"testfile.txt"),e.Filename);
	ASSERT_EQ((size_t)1592096,e.Size);


	// We expect an EndOfListException next
	ASSERT_THROW(e=d1.getNext(it), ppl7::EndOfListException);

}


TEST_F(DirTest, patternWalk) {
	ppl7::Dir d1("testdata", ppl7::Dir::SORT_FILENAME);
	ppl7::Dir::Iterator it;
	//d1.print();
	d1.reset(it);
	ppl7::DirEntry e;
	e=d1.getNextPattern(it,L"file*");
	ASSERT_EQ(ppl7::String(L"file1.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	e=d1.getNextPattern(it,L"file*");
	ASSERT_EQ(ppl7::String(L"file2.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	e=d1.getNextPattern(it,L"file*");
	ASSERT_EQ(ppl7::String(L"file3.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	// We expect an EndOfListException next
	ASSERT_THROW(e=d1.getNextPattern(it,L"file*"), ppl7::EndOfListException);

}


TEST_F(DirTest, patternWalk2) {
	ppl7::Dir d1("testdata", ppl7::Dir::SORT_FILENAME);
	ppl7::Dir::Iterator it;
	//d1.print();
	d1.reset(it);
	ppl7::DirEntry e;
	ASSERT_TRUE(d1.getNextPattern(e,it,L"file*"));
	ASSERT_EQ(ppl7::String(L"file1.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	ASSERT_TRUE(d1.getNextPattern(e,it,L"file*"));
	ASSERT_EQ(ppl7::String(L"file2.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	ASSERT_TRUE(d1.getNextPattern(e,it,L"file*"));
	ASSERT_EQ(ppl7::String(L"file3.txt"),e.Filename);
	ASSERT_EQ((size_t)6519,e.Size);

	// We expect an EndOfListException next
	ASSERT_FALSE(d1.getNextPattern(e,it,L"file*"));

}

TEST_F(DirTest, regExpWalk) {
	ppl7::Dir d1("testdata", ppl7::Dir::SORT_FILENAME);
	ppl7::Dir::Iterator it;
	ppl7::String expr(L"/^.file.*/i");
	//d1.print();
	d1.reset(it);
	ppl7::DirEntry e;
	e=d1.getNextRegExp(it,expr);
	ASSERT_EQ(ppl7::String(L"afile.txt"),e.Filename);
	ASSERT_EQ((size_t)13040,e.Size);

	e=d1.getNextRegExp(it,expr);
	ASSERT_EQ(ppl7::String(L"zfile.txt"),e.Filename);
	ASSERT_EQ((size_t)9819,e.Size);


	// We expect an EndOfListException next
	ASSERT_THROW(e=d1.getNextRegExp(it,L"file*"), ppl7::EndOfListException);

}

TEST_F(DirTest, regExpWalk2) {
	ppl7::Dir d1("testdata", ppl7::Dir::SORT_FILENAME);
	ppl7::Dir::Iterator it;
	ppl7::String expr(L"/^.file.*/i");
	//d1.print();
	d1.reset(it);
	ppl7::DirEntry e;
	ASSERT_TRUE(d1.getNextRegExp(e,it,expr));
	ASSERT_EQ(ppl7::String(L"afile.txt"),e.Filename);
	ASSERT_EQ((size_t)13040,e.Size);

	ASSERT_TRUE(d1.getNextRegExp(e,it,expr));
	ASSERT_EQ(ppl7::String(L"zfile.txt"),e.Filename);
	ASSERT_EQ((size_t)9819,e.Size);


	// We expect an EndOfListException next
	ASSERT_FALSE(d1.getNextRegExp(e,it,expr));

}

}


int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

