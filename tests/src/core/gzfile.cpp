/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 *******************************************************************************
 * Copyright (c) 2015, Patrick Fedick <patrick@pfp.de>
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
class GzFileTest : public ::testing::Test {
	protected:
	GzFileTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~GzFileTest() {

	}
};


TEST_F(GzFileTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl7::GzFile f1;
		ASSERT_FALSE(f1.isOpen()) << "File seems to be open, but it shouldn't";
	}
	);
}

TEST_F(GzFileTest, openNonexisting) {
	ppl7::GzFile f1;
	ASSERT_THROW(f1.open("nonexisting.txt"), ppl7::FileNotFoundException);
}

TEST_F(GzFileTest, openExistingUncompressed) {
	ppl7::GzFile f1;
	f1.open("testdata/compression.txt");
}

TEST_F(GzFileTest, openExistingCompressed) {
	ppl7::GzFile f1;
	f1.open("testdata/compression.txt.gz");
}

TEST_F(GzFileTest, sizeThrowsUnimplementedVirtualFunctionException) {
	ppl7::GzFile f1;
	f1.open("testdata/compression.txt");
	ASSERT_THROW(f1.size(), ppl7::UnimplementedVirtualFunctionException);
}

TEST_F(GzFileTest, fread1024_UncompressedFile) {
	ppl7::GzFile f1;
	ppl7::ByteArray ba;
	ba.malloc(1024);
	f1.open("testdata/compression.txt");
	f1.fread((void*)ba.adr(),1,1024);
	//ba.hexDump();
	ASSERT_EQ(ppl7::String("21ab51148e28167d5ce13bee07493a56"),ba.md5());
	// load the next chunk
	f1.fread((void*)ba.adr(),1,1024);
	//ba.hexDump();
	ASSERT_EQ(ppl7::String("468f6fd12d69be054643ef2ca1a19cba"),ba.md5());
}

TEST_F(GzFileTest, fread1024_CompressedFile) {
	ppl7::GzFile f1;
	ppl7::ByteArray ba;
	ba.malloc(1024);
	f1.open("testdata/compression.txt.gz");
	f1.fread((void*)ba.adr(),1,1024);
	//ba.hexDump();
	ASSERT_EQ(ppl7::String("21ab51148e28167d5ce13bee07493a56"),ba.md5());
	// load the next chunk
	f1.fread((void*)ba.adr(),1,1024);
	//ba.hexDump();
	ASSERT_EQ(ppl7::String("468f6fd12d69be054643ef2ca1a19cba"),ba.md5());
}

TEST_F(GzFileTest, md5_UncompressedFile) {
	ppl7::GzFile f1;
	f1.open("testdata/compression.txt");
	ppl7::String digest=f1.md5();
	ASSERT_EQ(ppl7::String("f386e5ea10bc186b633eaf6ba9a20d8c"),digest);
}

TEST_F(GzFileTest, md5_CompressedFile) {
	ppl7::GzFile f1;
	f1.open("testdata/compression.txt.gz");
	ppl7::String digest=f1.md5();
	ASSERT_EQ(ppl7::String("f386e5ea10bc186b633eaf6ba9a20d8c"),digest);
}

TEST_F(GzFileTest, seekAndTell) {
	ppl7::GzFile f1;
	f1.open("testdata/compression.txt.gz");
	f1.seek(45678);
	ASSERT_EQ((uint64_t)45678,f1.tell());
	f1.seek(100);
	ASSERT_EQ((uint64_t)100,f1.tell());
	f1.seek(1024*1024);
	ASSERT_EQ((uint64_t)1024*1024,f1.tell());
}

TEST_F(GzFileTest, rewind) {
	ppl7::GzFile f1;
	f1.open("testdata/compression.txt.gz");
	f1.seek(45678);
	f1.rewind();
	ASSERT_EQ((uint64_t)0,f1.tell());
}

TEST_F(GzFileTest, isOpen) {
	ppl7::GzFile f1;
	ASSERT_FALSE(f1.isOpen());
	f1.open("testdata/compression.txt.gz");
	ASSERT_TRUE(f1.isOpen());
}

TEST_F(GzFileTest, freadUntilEof) {
	ppl7::GzFile f1;
	ppl7::ByteArray ba;
	ba.malloc(1024);
	f1.open("testdata/compression.txt.gz");
	uint64_t bytes=0;
	ASSERT_THROW({
		while (1) {
			bytes+=f1.fread((void*)ba.adr(),1,1024);
		}
	}, ppl7::EndOfFileException);
	ASSERT_EQ((uint64_t) 1592096, bytes);
}

TEST_F(GzFileTest, fgets) {
	ppl7::GzFile f1;
	ppl7::ByteArray ba;
	ba.malloc(1024);
	char *buffer=(char*)ba.adr();
	f1.open("testdata/compression.txt.gz");
	char *ret;
	ASSERT_NO_THROW({
		ret=f1.fgets(buffer,1024);
	});
	ASSERT_EQ(ret, buffer);
	size_t len=strlen(ret);
	//printf (">>%s<< len=%zi\n",ret,strlen(ret));
	ASSERT_EQ((size_t) 47, len);
}

TEST_F(GzFileTest, fgetsUntilEof) {
	ppl7::GzFile f1;
	ppl7::ByteArray ba;
	ba.malloc(1024);
	char *buffer=(char*)ba.adr();
	f1.open("testdata/compression.txt.gz");
	char *ret;
	uint64_t bytes=0;
	ASSERT_THROW({
		while (1) {
			ret=f1.fgets(buffer,1024);
			bytes+=strlen(ret);
		}
	}, ppl7::EndOfFileException);
	ASSERT_EQ((uint64_t) 1592096, bytes);
}

TEST_F(GzFileTest, getsAsString) {
	ppl7::GzFile f1;
	ppl7::String s;
	f1.open("testdata/compression.txt.gz");
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

TEST_F(GzFileTest, fgetc) {
	ppl7::GzFile f1;
	f1.open("testdata/compression.txt.gz");
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ(32,f1.fgetc());
	ASSERT_EQ('G',f1.fgetc());
	ASSERT_EQ('N',f1.fgetc());
	ASSERT_EQ('U',f1.fgetc());
}

TEST_F(GzFileTest, fgetcUntilEof) {
	ppl7::GzFile f1;
	f1.open("testdata/compression.txt.gz");
	uint64_t bytes=0;
	ASSERT_THROW({
		while (1) {
			f1.fgetc();
			bytes++;
		}
	}, ppl7::EndOfFileException);
	ASSERT_EQ((uint64_t) 1592096, bytes);
}

#ifdef TODO

TEST_F(GzFileTest, fgetcUntilEof_withoutException) {
	ppl7::GzFile f1;
	f1.open("testdata/compression.txt.gz");
	uint64_t bytes=0;
	while (!f1.eof()) {
		f1.fgetc();
		bytes++;
	}
	ASSERT_EQ((uint64_t) 1592096, bytes);
}

TEST_F(GzFileTest, openNonexistingUtf8) {
	ppl7::GzFile f1;
	ASSERT_THROW(f1.open("noneäxisting.txt"), ppl7::GzFileNotFoundException);
}

TEST_F(GzFileTest, openExisting) {
	ppl7::GzFile f1;
	ASSERT_NO_THROW(f1.open("testdata/filenameUSASCII.txt"));
}

TEST_F(GzFileTest, openExistingUtf8) {
	ppl7::GzFile f1;
	ASSERT_NO_THROW(f1.open("testdata/filenameUTF8äöü.txt"));
}


TEST_F(GzFileTest, openAndClose) {
	ppl7::GzFile f1;
	f1.open("testdata/dirwalk/LICENSE.TXT");
	ASSERT_EQ((uint64_t)1330,f1.size());
	f1.close();
	f1.open("testdata/dirwalk/testfile.txt");
	ASSERT_EQ((uint64_t)1592096,f1.size());

}


TEST_F(GzFileTest, seek) {
	ppl7::GzFile f1;
	f1.open("testdata/dirwalk/testfile.txt");
	f1.seek(45678,ppl7::GzFile::SEEKSET);
	ASSERT_EQ((uint64_t)45678,f1.tell());
	f1.seek(100,ppl7::GzFile::SEEKCUR);
	ASSERT_EQ((uint64_t)45778,f1.tell());
	f1.seek(-1000,ppl7::GzFile::SEEKCUR);
	ASSERT_EQ((uint64_t)44778,f1.tell());
	f1.seek(-1000,ppl7::GzFile::SEEKEND);
	ASSERT_EQ((uint64_t)1591096,f1.tell());
}


#endif



}


