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
class FileReadTest : public ::testing::Test {
	protected:
	FileReadTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~FileReadTest() {

	}
};


TEST_F(FileReadTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl7::File f1;
		ASSERT_FALSE(f1.isOpen()) << "File seems to be open, but it shouldn't";
	}
	);
}

TEST_F(FileReadTest, openNonexisting) {
	ppl7::File f1;
	ASSERT_THROW(f1.open("nonexisting.txt"), ppl7::FileNotFoundException);
}

TEST_F(FileReadTest, openNonexistingUtf8) {
	ppl7::File f1;
	ASSERT_THROW(f1.open("noneäxisting.txt"), ppl7::FileNotFoundException);
}

TEST_F(FileReadTest, openExisting) {
	ppl7::File f1;
	ASSERT_NO_THROW(f1.open("testdata/filenameUSASCII.txt"));
}

TEST_F(FileReadTest, openExistingLocal) {
	ppl7::File f1;
	ASSERT_NO_THROW(f1.open(ppl7::Iconv::Utf8ToLocal("testdata/filenameUTF8äöü.txt")));
}

TEST_F(FileReadTest, size) {
	ppl7::File f1;
	f1.open("testdata/dirwalk/LICENSE.TXT");
	ASSERT_EQ((uint64_t)1330,f1.size());
	ppl7::File f2;
	f2.open("testdata/dirwalk/testfile.txt");
	ASSERT_EQ((uint64_t)1592096,f2.size());

}

TEST_F(FileReadTest, openAndClose) {
	ppl7::File f1;
	f1.open("testdata/dirwalk/LICENSE.TXT");
	ASSERT_EQ((uint64_t)1330,f1.size());
	f1.close();
	f1.open("testdata/dirwalk/testfile.txt");
	ASSERT_EQ((uint64_t)1592096,f1.size());

}

TEST_F(FileReadTest, md5) {
	ppl7::File f1;
	f1.open("testdata/dirwalk/testfile.txt");
	ppl7::String digest=f1.md5();
	ASSERT_EQ(ppl7::String("f386e5ea10bc186b633eaf6ba9a20d8c"),digest);
}

TEST_F(FileReadTest, seekAndTell) {
	ppl7::File f1;
	f1.open("testdata/dirwalk/testfile.txt");
	f1.seek(45678);
	ASSERT_EQ((uint64_t)45678,f1.tell());
	f1.seek(100);
	ASSERT_EQ((uint64_t)100,f1.tell());
	f1.seek(1024*1024);
	ASSERT_EQ((uint64_t)1024*1024,f1.tell());
}

TEST_F(FileReadTest, seek) {
	ppl7::File f1;
	f1.open("testdata/dirwalk/testfile.txt");
	f1.seek(45678,ppl7::File::SEEKSET);
	ASSERT_EQ((uint64_t)45678,f1.tell());
	f1.seek(100,ppl7::File::SEEKCUR);
	ASSERT_EQ((uint64_t)45778,f1.tell());
	f1.seek(-1000,ppl7::File::SEEKCUR);
	ASSERT_EQ((uint64_t)44778,f1.tell());
	f1.seek(-1000,ppl7::File::SEEKEND);
	ASSERT_EQ((uint64_t)1591096,f1.tell());
}

TEST_F(FileReadTest, fread1024) {
	ppl7::File f1;
	ppl7::ByteArray ba;
	ba.malloc(1024);
	f1.open("testdata/dirwalk/testfile.txt");
	f1.fread((void*)ba.adr(),1,1024);
	//ba.hexDump();
	ASSERT_EQ(ppl7::String("21ab51148e28167d5ce13bee07493a56"),ba.md5());
	// load the next chunk
	f1.fread((void*)ba.adr(),1,1024);
	//ba.hexDump();
	ASSERT_EQ(ppl7::String("468f6fd12d69be054643ef2ca1a19cba"),ba.md5());
}

TEST_F(FileReadTest, freadUntilEof) {
	ppl7::File f1;
	ppl7::ByteArray ba;
	ba.malloc(1024);
	f1.open("testdata/dirwalk/testfile.txt");
	uint64_t bytes=0;
	ASSERT_THROW({
		while (1) {
			bytes+=f1.fread((void*)ba.adr(),1,1024);
		}
	}, ppl7::EndOfFileException);
	ASSERT_EQ((uint64_t) 1592096, bytes);
}

TEST_F(FileReadTest, fgets) {
	ppl7::File f1;
	ppl7::ByteArray ba;
	ba.malloc(1024);
	char *buffer=(char*)ba.adr();
	f1.open("testdata/dirwalk/testfile.txt");
	char *ret;
	ASSERT_NO_THROW({
		ret=f1.fgets(buffer,1024);
	});
	ASSERT_EQ(ret, buffer);
	size_t len=strlen(ret);
	//printf (">>%s<< len=%zi\n",ret,strlen(ret));
	ASSERT_EQ((size_t) 47, len);
}

TEST_F(FileReadTest, getsAsString) {
	ppl7::File f1;
	ppl7::String s;
	f1.open("testdata/dirwalk/testfile.txt");
	ASSERT_NO_THROW({
		s=f1.gets(1024);
	});
	s.trimRight();
	ASSERT_EQ(ppl7::String("                    GNU GENERAL PUBLIC LICENSE"),s);
	ASSERT_NO_THROW({
		s=f1.gets();
	});
	s.trimRight();
	ASSERT_EQ(ppl7::String("                       Version 2, June 1991"),s);
	ASSERT_NO_THROW({
		s=f1.gets();
	});
	s.trimRight();
	ASSERT_EQ(ppl7::String(""),s);
	ASSERT_NO_THROW({
		s=f1.gets();
	});
	s.trimRight();
	ASSERT_EQ(ppl7::String(" Copyright (C) 1989, 1991 Free Software Foundation, Inc.,"),s);
}





}


