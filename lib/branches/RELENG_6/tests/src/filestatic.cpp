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

class CFileStaticTest : public ::testing::Test {
	protected:
	CFileStaticTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl6::SetGlobalEncoding("UTF-8");
	}
	virtual ~CFileStaticTest() {

	}
};


TEST_F(CFileStaticTest, StatOnUSAscii) {
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("testdata/filenameUSASCII.txt",d));

	ASSERT_EQ((size_t)1962,d.Size);
	ASSERT_EQ(ppl6::CString("testdata/filenameUSASCII.txt"),d.File);
}

TEST_F(CFileStaticTest, StatOnUtf8) {
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("testdata/filenameUTF8äöü.txt",d));

	ASSERT_EQ((size_t)1962,d.Size);
	ASSERT_EQ(ppl6::CString("testdata/filenameUTF8äöü.txt"),d.File);
}


TEST_F(CFileStaticTest, StatOnNonexistingFile) {
	ppl6::CDirEntry d;
	ASSERT_EQ(0,ppl6::CFile::Stat("testdata/nonexisting.txt",d));

}


TEST_F(CFileStaticTest, TruncateOnUSASCII) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","truncate.tmp"));
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("truncate.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);
	ASSERT_EQ(1,ppl6::CFile::Truncate("truncate.tmp",1024));
	ASSERT_EQ(1,ppl6::CFile::Stat("truncate.tmp",d));
	ASSERT_EQ((size_t)1024,d.Size);
	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("truncate.tmp"));
	ASSERT_EQ(ppl6::CString("657351fba38e20fb0a4713e605f1d6a4"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("truncate.tmp");
}

TEST_F(CFileStaticTest, TruncateOnUtf8) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","truncateäöüß.tmp"));
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("truncateäöüß.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);
	ASSERT_EQ(1,ppl6::CFile::Truncate("truncateäöüß.tmp",1024));
	ASSERT_EQ(1,ppl6::CFile::Stat("truncateäöüß.tmp",d));
	ASSERT_EQ((size_t)1024,d.Size);
	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("truncateäöüß.tmp"));
	ASSERT_EQ(ppl6::CString("657351fba38e20fb0a4713e605f1d6a4"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("truncateäöüß.tmp");
}

TEST_F(CFileStaticTest, ExistsOnUSAscii) {
	ASSERT_EQ(1,ppl6::CFile::Exists("testdata/filenameUSASCII.txt"));
}

TEST_F(CFileStaticTest, ExistsOnUtf8) {
	ASSERT_EQ(1,ppl6::CFile::Exists("testdata/filenameUTF8äöü.txt"));
}

TEST_F(CFileStaticTest, ExistsNot) {
	ASSERT_EQ(0,ppl6::CFile::Exists("testdata/nonexistänt.txt"));
}

TEST_F(CFileStaticTest, ExistsfOnUSAscii) {
	ASSERT_EQ(1,ppl6::CFile::Existsf("testdata/%s","filenameUSASCII.txt"));
}

TEST_F(CFileStaticTest, ExistsfOnUtf8) {
	EXPECT_EQ(1,ppl6::CFile::Existsf("testdata/%s","filenameUTF8äöü.txt"));
	EXPECT_EQ(1,ppl6::CFile::Existsf("testdata/%s.%s","filenameUTF8äöü","txt"));
}

TEST_F(CFileStaticTest, CopyFileUSAscii2USAscii) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","copy.tmp"));
	ASSERT_EQ(1,ppl6::CFile::Exists("testdata/filenameUSASCII.txt")) << "Old file still exists";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("copy.tmp",d)) << "New file exists";
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("copy.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("copy.tmp");
}

TEST_F(CFileStaticTest, CopyFileUSAscii2Utf8) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","copyäöüß.tmp"));
	ASSERT_EQ(1,ppl6::CFile::Exists("testdata/filenameUSASCII.txt")) << "Old file still exists";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("copyäöüß.tmp",d)) << "New file exists";
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("copyäöüß.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("copyäöüß.tmp");
}

TEST_F(CFileStaticTest, CopyFileUtf82Utf8) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUTF8äöü.txt","copyäöüß.tmp"));
	ASSERT_EQ(1,ppl6::CFile::Exists("testdata/filenameUTF8äöü.txt")) << "Old file still exists";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("copyäöüß.tmp",d)) << "New file exists";
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("copyäöüß.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("copyäöüß.tmp");
}

TEST_F(CFileStaticTest, CopyFileOverExistingUSAscii) {
	ASSERT_EQ(1,ppl6::CFile::TouchFile("tmp/existing.tmp"));
	ASSERT_EQ(1,ppl6::CFile::Exists("tmp/existing.tmp"));

	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","tmp/existing.tmp"));
	ASSERT_EQ(1,ppl6::CFile::Exists("testdata/filenameUSASCII.txt")) << "Old file still exists";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/existing.tmp",d)) << "New file exists";
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("tmp/existing.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("tmp/existing.tmp");
}

TEST_F(CFileStaticTest, CopyFileOverExistingUtf8) {
	ASSERT_EQ(1,ppl6::CFile::TouchFile("tmp/existingäöü.tmp"));
	ASSERT_EQ(1,ppl6::CFile::Exists("tmp/existingäöü.tmp"));

	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","tmp/existingäöü.tmp"));
	ASSERT_EQ(1,ppl6::CFile::Exists("testdata/filenameUSASCII.txt")) << "Old file still exists";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/existingäöü.tmp",d)) << "New file exists";
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("tmp/existingäöü.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("tmp/existingäöü.tmp");
}


TEST_F(CFileStaticTest, MoveFileUSAscii2USAscii) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","copy.tmp"));
	ASSERT_EQ(1,ppl6::CFile::MoveFile("copy.tmp","move.tmp"));
	ASSERT_EQ(0,ppl6::CFile::Exists("copy.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl6::CFile::Exists("move.tmp")) << "New file exists";

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("move.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("move.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("move.tmp");
}

TEST_F(CFileStaticTest, MoveFileUSAscii2Utf8) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","copy.tmp"));
	ASSERT_EQ(1,ppl6::CFile::MoveFile("copy.tmp","moveäöü.tmp"));
	ASSERT_EQ(0,ppl6::CFile::Exists("copy.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl6::CFile::Exists("moveäöü.tmp")) << "New file exists";

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("moveäöü.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("moveäöü.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("moveäöü.tmp");
}


TEST_F(CFileStaticTest, MoveFileUtf82USAscii) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","copyäöü.tmp"));
	ASSERT_EQ(1,ppl6::CFile::MoveFile("copyäöü.tmp","move.tmp"));
	ASSERT_EQ(0,ppl6::CFile::Exists("copyäöü.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl6::CFile::Exists("move.tmp")) << "New file exists";

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("move.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("move.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("move.tmp");
}

TEST_F(CFileStaticTest, MoveFileUtf82Utf8) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","copyäöü.tmp"));
	ASSERT_EQ(1,ppl6::CFile::MoveFile("copyäöü.tmp","moveäöüß.tmp"));
	ASSERT_EQ(0,ppl6::CFile::Exists("copyäöü.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl6::CFile::Exists("moveäöüß.tmp")) << "New file exists";

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("moveäöüß.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("moveäöüß.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("moveäöüß.tmp");
}

TEST_F(CFileStaticTest, MoveFileOverExistingUSAscii) {
	ASSERT_EQ(1,ppl6::CFile::TouchFile("tmp/existing2.tmp"));
	ASSERT_EQ(1,ppl6::CFile::Exists("tmp/existing2.tmp"));

	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","tmp/copy.tmp"));
	ASSERT_EQ(1,ppl6::CFile::MoveFile("tmp/copy.tmp","tmp/existing2.tmp"));
	ASSERT_EQ(0,ppl6::CFile::Exists("tmp/copy.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl6::CFile::Exists("tmp/existing2.tmp")) << "New file exists";

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/existing2.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("tmp/existing2.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("tmp/existing2.tmp");
}

TEST_F(CFileStaticTest, MoveFileOverExistingUtf8) {
	ASSERT_EQ(1,ppl6::CFile::TouchFile("tmp/existing2äöü.tmp"));
	ASSERT_EQ(1,ppl6::CFile::Exists("tmp/existing2äöü.tmp"));

	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","tmp/copy.tmp"));
	ASSERT_EQ(1,ppl6::CFile::MoveFile("tmp/copy.tmp","tmp/existing2äöü.tmp"));
	ASSERT_EQ(0,ppl6::CFile::Exists("tmp/copy.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl6::CFile::Exists("tmp/existing2äöü.tmp")) << "New file exists";

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/existing2äöü.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("tmp/existing2äöü.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("tmp/existing2äöü.tmp");
}


TEST_F(CFileStaticTest, LoadFileUSAscii2CString) {
	ppl6::CString s;
	ASSERT_EQ(1,ppl6::CFile::LoadFile(s,"testdata/filenameUSASCII.txt")) << "Loading file to string";
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),s.GetMD5()) << "Checking MD5-sum";
	ASSERT_EQ((size_t)1962,s.Size()) << "Checking Size";
}

TEST_F(CFileStaticTest, LoadFileUSAscii2CBinary) {
	ppl6::CBinary s;
	ASSERT_EQ(1,ppl6::CFile::LoadFile(s,"testdata/filenameUSASCII.txt")) << "Loading file to string";
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),s.GetMD5Sum()) << "Checking MD5-sum";
	ASSERT_EQ((size_t)1962,s.Size()) << "Checking Size";

}

TEST_F(CFileStaticTest, LoadFileUtf82CString) {
	ppl6::CString s;
	ASSERT_EQ(1,ppl6::CFile::LoadFile(s,"testdata/filenameUTF8äöü.txt")) << "Loading file to string";
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),s.GetMD5()) << "Checking MD5-sum";
	ASSERT_EQ((size_t)1962,s.Size()) << "Checking Size";
}

/*
TEST_F(CFileStaticTest, LoadFileUtf82CWString) {
	ppl6::CWString s;
	ASSERT_EQ(1,ppl6::CFile::LoadFile(s,"testdata/unicodeUtf8äöü.txt")) << "Loading file to string";
	ASSERT_EQ((size_t)1994,s.Size()) << "Checking Size";
	ASSERT_EQ(ppl6::CWString("c6767ca956f9e3b65164bed63b93d811"),s.GetMD5()) << "Checking MD5-sum";
}
*/

TEST_F(CFileStaticTest, LoadFileUtf82CBinary) {
	ppl6::CBinary s;
	ASSERT_EQ(1,ppl6::CFile::LoadFile(s,"testdata/filenameUTF8äöü.txt")) << "Loading file to string";
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),s.GetMD5Sum()) << "Checking MD5-sum";
	ASSERT_EQ((size_t)1962,s.Size()) << "Checking Size";
}

TEST_F(CFileStaticTest, LoadFileUSAscii2Ptr) {
	void *adr=NULL;
	size_t size;
	adr=ppl6::CFile::LoadFile("testdata/filenameUSASCII.txt",&size);
	ASSERT_TRUE(adr!=NULL) << "Loading file to ptr";
	EXPECT_EQ((size_t)1962,size) << "Checking Size";
	EXPECT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ppl6::MD5(adr,size)) << "Checking MD5-sum";
	free(adr);
}

TEST_F(CFileStaticTest, LoadFileUtf82Ptr) {
	void *adr=NULL;
	size_t size;
	adr=ppl6::CFile::LoadFile("testdata/unicodeUtf8äöü.txt",&size);
	ASSERT_TRUE(adr!=NULL) << "Loading file to ptr";
	EXPECT_EQ((size_t)7976,size) << "Checking Size";
	EXPECT_EQ(ppl6::CString("c6767ca956f9e3b65164bed63b93d811"),ppl6::MD5(adr,size)) << "Checking MD5-sum";
	free(adr);
}

TEST_F(CFileStaticTest, DeleteFileUSAscii) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","todelete.tmp"));
	ASSERT_EQ(1,ppl6::CFile::DeleteFile("todelete.tmp")) << "deleting file";
	ASSERT_EQ(0,ppl6::CFile::Exists("todelete.tmp")) << "file does not exists anymore";
}

TEST_F(CFileStaticTest, DeleteFileUtf8) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","todeleteäöü.tmp"));
	ASSERT_EQ(1,ppl6::CFile::DeleteFile("todeleteäöü.tmp")) << "deleting file";
	ASSERT_EQ(0,ppl6::CFile::Exists("todeleteäöü.tmp")) << "file does not exists anymore";
}

TEST_F(CFileStaticTest, DeleteFilefUSAscii) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","todelete.tmp"));
	ASSERT_EQ(1,ppl6::CFile::DeleteFilef("%s","todelete.tmp")) << "deleting file";
	ASSERT_EQ(0,ppl6::CFile::Exists("todelete.tmp")) << "file does not exists anymore";
}

TEST_F(CFileStaticTest, DeletefFileUtf8) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","todeleteäöü.tmp"));
	ASSERT_EQ(1,ppl6::CFile::DeleteFilef("%s","todeleteäöü.tmp")) << "deleting file";
	ASSERT_EQ(0,ppl6::CFile::Exists("todeleteäöü.tmp")) << "file does not exists anymore";
}

TEST_F(CFileStaticTest, TouchFileUSAscii) {
	ASSERT_EQ(1,ppl6::CFile::TouchFile("touched.tmp")) << "touching file";
	ASSERT_EQ(1,ppl6::CFile::Exists("touched.tmp")) << "file exists";
	ASSERT_EQ(1,ppl6::CFile::DeleteFile("touched.tmp")) << "deleting file";
}

TEST_F(CFileStaticTest, TouchFileUtf8) {
	ASSERT_EQ(1,ppl6::CFile::TouchFile("touchedäöü.tmp")) << "touching file";
	ASSERT_EQ(1,ppl6::CFile::Exists("touchedäöü.tmp")) << "file exists";
	ASSERT_EQ(1,ppl6::CFile::DeleteFile("touchedäöü.tmp")) << "deleting file";
}

TEST_F(CFileStaticTest, TouchFilefUSAscii) {
	ASSERT_EQ(1,ppl6::CFile::TouchFilef("%s.%s","touched2","tmp")) << "touching file";
	ASSERT_EQ(1,ppl6::CFile::Exists("touched2.tmp")) << "file exists";
	ASSERT_EQ(1,ppl6::CFile::DeleteFile("touched2.tmp")) << "deleting file";
}

TEST_F(CFileStaticTest, TouchFilefUtf8) {
	ASSERT_EQ(1,ppl6::CFile::TouchFilef("%s.%s","touchedäöü2","tmp")) << "touching file";
	ASSERT_EQ(1,ppl6::CFile::Exists("touchedäöü2.tmp")) << "file exists";
	ASSERT_EQ(1,ppl6::CFile::DeleteFile("touchedäöü2.tmp")) << "deleting file";
}

TEST_F(CFileStaticTest, WriteFileUSAscii) {
	ASSERT_EQ(1,ppl6::CFile::WriteFile(loremipsum,strlen(loremipsum),"writetest.tmp")) << "writing file";
	ASSERT_EQ(1,ppl6::CFile::Exists("writetest.tmp")) << "file exists";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("writetest.tmp",d)) << "Stat on file is working";
	ASSERT_EQ((size_t)strlen(loremipsum),d.Size) << "filesize as expected";

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("writetest.tmp"));
	ASSERT_EQ(ppl6::CString(loremipsum_md5),ff.MD5Sum()) << "md5sum of file is correct";
	ff.Close();
	ASSERT_EQ(1,ppl6::CFile::DeleteFile("writetest.tmp")) << "deleting file";
}

TEST_F(CFileStaticTest, WriteFileUtf8) {
	ASSERT_EQ(1,ppl6::CFile::WriteFile(loremipsum,strlen(loremipsum),"writetestäöü.tmp")) << "writing file";
	ASSERT_EQ(1,ppl6::CFile::Exists("writetestäöü.tmp")) << "file exists";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("writetestäöü.tmp",d)) << "Stat on file is working";
	ASSERT_EQ((size_t)strlen(loremipsum),d.Size) << "filesize as expected";

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("writetestäöü.tmp"));
	ASSERT_EQ(ppl6::CString(loremipsum_md5),ff.MD5Sum()) << "md5sum of file is correct";
	ff.Close();
	ASSERT_EQ(1,ppl6::CFile::DeleteFile("writetestäöü.tmp")) << "deleting file";
}

TEST_F(CFileStaticTest, WriteFilefUSAscii) {
	ASSERT_EQ(1,ppl6::CFile::WriteFilef(loremipsum,strlen(loremipsum),"%s.%s","writetest","tmp")) << "writing file";
	ASSERT_EQ(1,ppl6::CFile::Exists("writetest.tmp")) << "file exists";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("writetest.tmp",d)) << "Stat on file is working";
	ASSERT_EQ((size_t)strlen(loremipsum),d.Size) << "filesize as expected";

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("writetest.tmp"));
	ASSERT_EQ(ppl6::CString(loremipsum_md5),ff.MD5Sum()) << "md5sum of file is correct";
	ff.Close();
	ASSERT_EQ(1,ppl6::CFile::DeleteFile("writetest.tmp")) << "deleting file";
}

TEST_F(CFileStaticTest, WriteFilefUtf8) {
	ASSERT_EQ(1,ppl6::CFile::WriteFilef(loremipsum,strlen(loremipsum),"%s.%s","writetestäöü","tmp")) << "writing file";
	ASSERT_EQ(1,ppl6::CFile::Exists("writetestäöü.tmp")) << "file exists";
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("writetestäöü.tmp",d)) << "Stat on file is working";
	ASSERT_EQ((size_t)strlen(loremipsum),d.Size) << "filesize as expected";

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("writetestäöü.tmp"));
	ASSERT_EQ(ppl6::CString(loremipsum_md5),ff.MD5Sum()) << "md5sum of file is correct";
	ff.Close();
	ASSERT_EQ(1,ppl6::CFile::DeleteFile("writetestäöü.tmp")) << "deleting file";
}

TEST_F(CFileStaticTest, RenameFileUSAscii2USAscii) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","copy.tmp"));
	ASSERT_EQ(1,ppl6::CFile::RenameFile("copy.tmp","move.tmp"));
	ASSERT_EQ(0,ppl6::CFile::Exists("copy.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl6::CFile::Exists("move.tmp")) << "New file exists";

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("move.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("move.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("move.tmp");
}

TEST_F(CFileStaticTest, RenameFileUSAscii2Utf8) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","copy.tmp"));
	ASSERT_EQ(1,ppl6::CFile::RenameFile("copy.tmp","moveäöü.tmp"));
	ASSERT_EQ(0,ppl6::CFile::Exists("copy.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl6::CFile::Exists("moveäöü.tmp")) << "New file exists";

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("moveäöü.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("moveäöü.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("moveäöü.tmp");
}


TEST_F(CFileStaticTest, RenameFileUtf82USAscii) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","copyäöü.tmp"));
	ASSERT_EQ(1,ppl6::CFile::RenameFile("copyäöü.tmp","move.tmp"));
	ASSERT_EQ(0,ppl6::CFile::Exists("copyäöü.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl6::CFile::Exists("move.tmp")) << "New file exists";

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("move.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("move.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("move.tmp");
}

TEST_F(CFileStaticTest, RenameFileUtf82Utf8) {
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","copyäöü.tmp"));
	ASSERT_EQ(1,ppl6::CFile::RenameFile("copyäöü.tmp","moveäöüß.tmp"));
	ASSERT_EQ(0,ppl6::CFile::Exists("copyäöü.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl6::CFile::Exists("moveäöüß.tmp")) << "New file exists";

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("moveäöüß.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ppl6::CFile::MD5("moveäöüß.tmp"));
	ppl6::CFile::DeleteFile("moveäöüß.tmp");
}

TEST_F(CFileStaticTest, RenameFileOverExistingUSAscii) {
	ASSERT_EQ(1,ppl6::CFile::TouchFile("tmp/existing3.tmp"));
	ASSERT_EQ(1,ppl6::CFile::Exists("tmp/existing3.tmp"));

	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","copy.tmp"));
	ASSERT_EQ(1,ppl6::CFile::RenameFile("copy.tmp","tmp/existing3.tmp"));
	ASSERT_EQ(0,ppl6::CFile::Exists("copy.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl6::CFile::Exists("tmp/existing3.tmp")) << "New file exists";

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/existing3.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("tmp/existing3.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("tmp/existing3.tmp");
}

TEST_F(CFileStaticTest, RenameFileOverExistingUtf8) {
	ASSERT_EQ(1,ppl6::CFile::TouchFile("tmp/existing3äöü.tmp"));
	ASSERT_EQ(1,ppl6::CFile::Exists("tmp/existing3äöü.tmp"));

	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","copy.tmp"));
	ASSERT_EQ(1,ppl6::CFile::RenameFile("copy.tmp","tmp/existing3äöü.tmp"));
	ASSERT_EQ(0,ppl6::CFile::Exists("copy.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl6::CFile::Exists("tmp/existing3äöü.tmp")) << "New file exists";

	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("tmp/existing3äöü.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl6::CFile ff;
	ASSERT_EQ(1,ff.Open("tmp/existing3äöü.tmp"));
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl6::CFile::DeleteFile("tmp/existing3äöü.tmp");
}



TEST_F(CFileStaticTest, MD5) {
	ASSERT_EQ(ppl6::CString("978fd668b5755ce6017256d0ff9e36b2"),ppl6::CFile::MD5("testdata/filenameUSASCII.txt"));
}

/*
 * TODO: Chmod und Stat interpretieren die Attribute unterschiedlich
 */
/*
TEST_F(CFileStaticTest, ChmodUSAscii) {
	ppl6::CFile::Chmod("chmod1.tmp",ppl6::FILEATTR::USR_READ|ppl6::FILEATTR::USR_WRITE);
	ASSERT_EQ(1,ppl6::CFile::CopyFile("testdata/filenameUSASCII.txt","chmod1.tmp"));
	ASSERT_EQ(1,ppl6::CFile::Chmod("chmod1.tmp",ppl6::FILEATTR::USR_READ));
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("chmod1.tmp",d)) << "Stat on file";
	d.Print();
	ASSERT_EQ(ppl6::FILEATTR::USR_READ,d.Attrib&ppl6::FILEATTR::USR_READ);
	ASSERT_NE(ppl6::FILEATTR::USR_WRITE,d.Attrib&ppl6::FILEATTR::USR_WRITE);
	ppl6::CFile::DeleteFile("chmod1.tmp");

}
*/


} // EOF namespace

