/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 837 $
 * $Date: 2014-01-23 08:31:34 +0100 (Do, 23 Jan 2014) $
 * $Id: file.cpp 837 2014-01-23 07:31:34Z pafe $
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

#include "../include/prolog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include "../include/ppl7.h"
#include <gtest/gtest.h>
#include "ppl7-tests.h"


namespace {

class FileStaticTest : public ::testing::Test {
	protected:
	FileStaticTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~FileStaticTest() {

	}
};


TEST_F(FileStaticTest, StatOnUSAscii) {
	ppl7::DirEntry d;
	ASSERT_NO_THROW({
		ppl7::File::stat("testdata/filenameUSASCII.txt",d);
	});
	ASSERT_EQ((size_t)1962,d.Size);
	ASSERT_EQ(ppl7::String("testdata/filenameUSASCII.txt"),d.File);
}


TEST_F(FileStaticTest, StatOnUtf8) {
	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::stat("testdata/filenameUTF8äöü.txt",d));
	ASSERT_EQ((size_t)1962,d.Size);
	ASSERT_EQ(ppl7::String("testdata/filenameUTF8äöü.txt"),d.File);
}


TEST_F(FileStaticTest, StatOnNonexistingFile) {
	ppl7::DirEntry d;
	ASSERT_THROW(ppl7::File::stat("testdata/nonexisting.txt",d),ppl7::FileNotFoundException);
}

TEST_F(FileStaticTest, ExistsOnUSAscii) {
	ASSERT_EQ(true,ppl7::File::exists("testdata/filenameUSASCII.txt"));
}

TEST_F(FileStaticTest, ExistsOnUtf8) {
	ASSERT_EQ(true,ppl7::File::exists("testdata/filenameUTF8äöü.txt"));
}

TEST_F(FileStaticTest, ExistsNot) {
	ASSERT_EQ(false,ppl7::File::exists("testdata/nonexistänt.txt"));
}

TEST_F(FileStaticTest, CopyFileUSAscii2USAscii) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","copy.tmp"));
	ASSERT_TRUE(ppl7::File::exists("testdata/filenameUSASCII.txt")) << "Old file still exists";
	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::stat("copy.tmp",d)) << "New file exists";
	ASSERT_EQ((size_t)1962,d.Size);

	ppl7::File ff;
	ASSERT_NO_THROW(ff.open("copy.tmp"));
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ff.md5());
	ff.close();
	ppl7::File::remove("copy.tmp");
}

TEST_F(FileStaticTest, CopyFileUSAscii2Utf8) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","copyäöüß.tmp"));
	ASSERT_TRUE(ppl7::File::exists("testdata/filenameUSASCII.txt")) << "Old file still exists";
	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::stat("copyäöüß.tmp",d)) << "New file exists";
	ASSERT_EQ((size_t)1962,d.Size);

	ppl7::File ff;
	ASSERT_NO_THROW(ff.open("copyäöüß.tmp"));
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ff.md5());
	ff.close();
	ppl7::File::remove("copyäöüß.tmp");
}

TEST_F(FileStaticTest, CopyFileUtf82Utf8) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUTF8äöü.txt","copyäöüß.tmp"));
	ASSERT_TRUE(ppl7::File::exists("testdata/filenameUTF8äöü.txt")) << "Old file still exists";
	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::stat("copyäöüß.tmp",d)) << "New file exists";
	ASSERT_EQ((size_t)1962,d.Size);

	ppl7::File ff;
	ASSERT_NO_THROW(ff.open("copyäöüß.tmp"));
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ff.md5());
	ff.close();
	ppl7::File::remove("copyäöüß.tmp");
}

TEST_F(FileStaticTest, CopyFileUtf82UsAscii) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUTF8äöü.txt","copy2.tmp"));
	ASSERT_TRUE(ppl7::File::exists("testdata/filenameUTF8äöü.txt")) << "Old file still exists";
	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::stat("copy2.tmp",d)) << "New file exists";
	ASSERT_EQ((size_t)1962,d.Size);

	ppl7::File ff;
	ASSERT_NO_THROW(ff.open("copy2.tmp"));
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ff.md5());
	ff.close();
	ppl7::File::remove("copy2.tmp");
}


TEST_F(FileStaticTest, TruncateOnUSASCII) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","truncate.tmp"));
	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::stat("truncate.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);
	ASSERT_NO_THROW(ppl7::File::truncate("truncate.tmp",1024));
	ASSERT_NO_THROW(ppl7::File::stat("truncate.tmp",d));
	ASSERT_EQ((size_t)1024,d.Size);
	ppl7::File ff;
	ASSERT_NO_THROW(ff.open("truncate.tmp"));
	ASSERT_EQ(ppl7::String("657351fba38e20fb0a4713e605f1d6a4"),ff.md5());
	ff.close();
	ppl7::File::remove("truncate.tmp");
}

TEST_F(FileStaticTest, TruncateOnUtf8) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","truncateäöüß.tmp"));
	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::stat("truncateäöüß.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);
	ASSERT_NO_THROW(ppl7::File::truncate("truncateäöüß.tmp",1024));
	ASSERT_NO_THROW(ppl7::File::stat("truncateäöüß.tmp",d));
	ASSERT_EQ((size_t)1024,d.Size);
	ppl7::File ff;
	ASSERT_NO_THROW(ff.open("truncateäöüß.tmp"));
	ASSERT_EQ(ppl7::String("657351fba38e20fb0a4713e605f1d6a4"),ff.md5());
	ff.close();
	ppl7::File::remove("truncateäöüß.tmp");
}

#ifdef TODO



TEST_F(FileStaticTest, MoveFileUSAscii2USAscii) {
	ASSERT_EQ(1,ppl7::File::CopyFile("testdata/filenameUSASCII.txt","copy.tmp"));
	ASSERT_EQ(1,ppl7::File::MoveFile("copy.tmp","move.tmp"));
	ASSERT_EQ(0,ppl7::File::Exists("copy.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl7::File::Exists("move.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_EQ(1,ppl7::File::Stat("move.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl7::File ff;
	ASSERT_EQ(1,ff.Open("move.tmp"));
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl7::File::DeleteFile("move.tmp");
}

TEST_F(FileStaticTest, MoveFileUSAscii2Utf8) {
	ASSERT_EQ(1,ppl7::File::CopyFile("testdata/filenameUSASCII.txt","copy.tmp"));
	ASSERT_EQ(1,ppl7::File::MoveFile("copy.tmp","moveäöü.tmp"));
	ASSERT_EQ(0,ppl7::File::Exists("copy.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl7::File::Exists("moveäöü.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_EQ(1,ppl7::File::Stat("moveäöü.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl7::File ff;
	ASSERT_EQ(1,ff.Open("moveäöü.tmp"));
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl7::File::DeleteFile("moveäöü.tmp");
}


TEST_F(FileStaticTest, MoveFileUtf82USAscii) {
	ASSERT_EQ(1,ppl7::File::CopyFile("testdata/filenameUSASCII.txt","copyäöü.tmp"));
	ASSERT_EQ(1,ppl7::File::MoveFile("copyäöü.tmp","move.tmp"));
	ASSERT_EQ(0,ppl7::File::Exists("copyäöü.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl7::File::Exists("move.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_EQ(1,ppl7::File::Stat("move.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl7::File ff;
	ASSERT_EQ(1,ff.Open("move.tmp"));
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl7::File::DeleteFile("move.tmp");
}

TEST_F(FileStaticTest, MoveFileUtf82Utf8) {
	ASSERT_EQ(1,ppl7::File::CopyFile("testdata/filenameUSASCII.txt","copyäöü.tmp"));
	ASSERT_EQ(1,ppl7::File::MoveFile("copyäöü.tmp","moveäöüß.tmp"));
	ASSERT_EQ(0,ppl7::File::Exists("copyäöü.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl7::File::Exists("moveäöüß.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_EQ(1,ppl7::File::Stat("moveäöüß.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl7::File ff;
	ASSERT_EQ(1,ff.Open("moveäöüß.tmp"));
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl7::File::DeleteFile("moveäöüß.tmp");
}

TEST_F(FileStaticTest, LoadFileUSAscii2CString) {
	ppl7::String s;
	ASSERT_EQ(1,ppl7::File::LoadFile(s,"testdata/filenameUSASCII.txt")) << "Loading file to string";
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),s.GetMD5()) << "Checking MD5-sum";
	ASSERT_EQ((size_t)1962,s.Size()) << "Checking Size";
}

TEST_F(FileStaticTest, LoadFileUSAscii2CBinary) {
	ppl6::CBinary s;
	ASSERT_EQ(1,ppl7::File::LoadFile(s,"testdata/filenameUSASCII.txt")) << "Loading file to string";
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),s.GetMD5Sum()) << "Checking MD5-sum";
	ASSERT_EQ((size_t)1962,s.Size()) << "Checking Size";

}

TEST_F(FileStaticTest, LoadFileUtf82CString) {
	ppl7::String s;
	ASSERT_EQ(1,ppl7::File::LoadFile(s,"testdata/filenameUTF8äöü.txt")) << "Loading file to string";
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),s.GetMD5()) << "Checking MD5-sum";
	ASSERT_EQ((size_t)1962,s.Size()) << "Checking Size";
}

/*
TEST_F(FileStaticTest, LoadFileUtf82CWString) {
	ppl6::CWString s;
	ASSERT_EQ(1,ppl7::File::LoadFile(s,"testdata/unicodeUtf8äöü.txt")) << "Loading file to string";
	ASSERT_EQ((size_t)1994,s.Size()) << "Checking Size";
	ASSERT_EQ(ppl6::CWString("c6767ca956f9e3b65164bed63b93d811"),s.GetMD5()) << "Checking MD5-sum";
}
*/

TEST_F(FileStaticTest, LoadFileUtf82CBinary) {
	ppl6::CBinary s;
	ASSERT_EQ(1,ppl7::File::LoadFile(s,"testdata/filenameUTF8äöü.txt")) << "Loading file to string";
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),s.GetMD5Sum()) << "Checking MD5-sum";
	ASSERT_EQ((size_t)1962,s.Size()) << "Checking Size";
}

TEST_F(FileStaticTest, LoadFileUSAscii2Ptr) {
	void *adr=NULL;
	size_t size;
	adr=ppl7::File::LoadFile("testdata/filenameUSASCII.txt",&size);
	ASSERT_TRUE(adr!=NULL) << "Loading file to ptr";
	EXPECT_EQ((size_t)1962,size) << "Checking Size";
	EXPECT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ppl6::MD5(adr,size)) << "Checking MD5-sum";
	free(adr);
}

TEST_F(FileStaticTest, LoadFileUtf82Ptr) {
	void *adr=NULL;
	size_t size;
	adr=ppl7::File::LoadFile("testdata/unicodeUtf8äöü.txt",&size);
	ASSERT_TRUE(adr!=NULL) << "Loading file to ptr";
	EXPECT_EQ((size_t)7976,size) << "Checking Size";
	EXPECT_EQ(ppl7::String("c6767ca956f9e3b65164bed63b93d811"),ppl6::MD5(adr,size)) << "Checking MD5-sum";
	free(adr);
}

TEST_F(FileStaticTest, DeleteFileUSAscii) {
	ASSERT_EQ(1,ppl7::File::CopyFile("testdata/filenameUSASCII.txt","todelete.tmp"));
	ASSERT_EQ(1,ppl7::File::DeleteFile("todelete.tmp")) << "deleting file";
	ASSERT_EQ(0,ppl7::File::Exists("todelete.tmp")) << "file does not exists anymore";
}

TEST_F(FileStaticTest, DeleteFileUtf8) {
	ASSERT_EQ(1,ppl7::File::CopyFile("testdata/filenameUSASCII.txt","todeleteäöü.tmp"));
	ASSERT_EQ(1,ppl7::File::DeleteFile("todeleteäöü.tmp")) << "deleting file";
	ASSERT_EQ(0,ppl7::File::Exists("todeleteäöü.tmp")) << "file does not exists anymore";
}

TEST_F(FileStaticTest, DeleteFilefUSAscii) {
	ASSERT_EQ(1,ppl7::File::CopyFile("testdata/filenameUSASCII.txt","todelete.tmp"));
	ASSERT_EQ(1,ppl7::File::DeleteFilef("%s","todelete.tmp")) << "deleting file";
	ASSERT_EQ(0,ppl7::File::Exists("todelete.tmp")) << "file does not exists anymore";
}

TEST_F(FileStaticTest, DeletefFileUtf8) {
	ASSERT_EQ(1,ppl7::File::CopyFile("testdata/filenameUSASCII.txt","todeleteäöü.tmp"));
	ASSERT_EQ(1,ppl7::File::DeleteFilef("%s","todeleteäöü.tmp")) << "deleting file";
	ASSERT_EQ(0,ppl7::File::Exists("todeleteäöü.tmp")) << "file does not exists anymore";
}

TEST_F(FileStaticTest, TouchFileUSAscii) {
	ASSERT_EQ(1,ppl7::File::TouchFile("touched.tmp")) << "touching file";
	ASSERT_EQ(1,ppl7::File::Exists("touched.tmp")) << "file exists";
	ASSERT_EQ(1,ppl7::File::DeleteFile("touched.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, TouchFileUtf8) {
	ASSERT_EQ(1,ppl7::File::TouchFile("touchedäöü.tmp")) << "touching file";
	ASSERT_EQ(1,ppl7::File::Exists("touchedäöü.tmp")) << "file exists";
	ASSERT_EQ(1,ppl7::File::DeleteFile("touchedäöü.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, TouchFilefUSAscii) {
	ASSERT_EQ(1,ppl7::File::TouchFilef("%s.%s","touched2","tmp")) << "touching file";
	ASSERT_EQ(1,ppl7::File::Exists("touched2.tmp")) << "file exists";
	ASSERT_EQ(1,ppl7::File::DeleteFile("touched2.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, TouchFilefUtf8) {
	ASSERT_EQ(1,ppl7::File::TouchFilef("%s.%s","touchedäöü2","tmp")) << "touching file";
	ASSERT_EQ(1,ppl7::File::Exists("touchedäöü2.tmp")) << "file exists";
	ASSERT_EQ(1,ppl7::File::DeleteFile("touchedäöü2.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, WriteFileUSAscii) {
	ASSERT_EQ(1,ppl7::File::WriteFile(loremipsum,strlen(loremipsum),"writetest.tmp")) << "writing file";
	ASSERT_EQ(1,ppl7::File::Exists("writetest.tmp")) << "file exists";
	ppl7::DirEntry d;
	ASSERT_EQ(1,ppl7::File::Stat("writetest.tmp",d)) << "Stat on file is working";
	ASSERT_EQ((size_t)strlen(loremipsum),d.Size) << "filesize as expected";

	ppl7::File ff;
	ASSERT_EQ(1,ff.Open("writetest.tmp"));
	ASSERT_EQ(ppl7::String(loremipsum_md5),ff.MD5Sum()) << "md5sum of file is correct";
	ff.Close();
	ASSERT_EQ(1,ppl7::File::DeleteFile("writetest.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, WriteFileUtf8) {
	ASSERT_EQ(1,ppl7::File::WriteFile(loremipsum,strlen(loremipsum),"writetestäöü.tmp")) << "writing file";
	ASSERT_EQ(1,ppl7::File::Exists("writetestäöü.tmp")) << "file exists";
	ppl7::DirEntry d;
	ASSERT_EQ(1,ppl7::File::Stat("writetestäöü.tmp",d)) << "Stat on file is working";
	ASSERT_EQ((size_t)strlen(loremipsum),d.Size) << "filesize as expected";

	ppl7::File ff;
	ASSERT_EQ(1,ff.Open("writetestäöü.tmp"));
	ASSERT_EQ(ppl7::String(loremipsum_md5),ff.MD5Sum()) << "md5sum of file is correct";
	ff.Close();
	ASSERT_EQ(1,ppl7::File::DeleteFile("writetestäöü.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, WriteFilefUSAscii) {
	ASSERT_EQ(1,ppl7::File::WriteFilef(loremipsum,strlen(loremipsum),"%s.%s","writetest","tmp")) << "writing file";
	ASSERT_EQ(1,ppl7::File::Exists("writetest.tmp")) << "file exists";
	ppl7::DirEntry d;
	ASSERT_EQ(1,ppl7::File::Stat("writetest.tmp",d)) << "Stat on file is working";
	ASSERT_EQ((size_t)strlen(loremipsum),d.Size) << "filesize as expected";

	ppl7::File ff;
	ASSERT_EQ(1,ff.Open("writetest.tmp"));
	ASSERT_EQ(ppl7::String(loremipsum_md5),ff.MD5Sum()) << "md5sum of file is correct";
	ff.Close();
	ASSERT_EQ(1,ppl7::File::DeleteFile("writetest.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, WriteFilefUtf8) {
	ASSERT_EQ(1,ppl7::File::WriteFilef(loremipsum,strlen(loremipsum),"%s.%s","writetestäöü","tmp")) << "writing file";
	ASSERT_EQ(1,ppl7::File::Exists("writetestäöü.tmp")) << "file exists";
	ppl7::DirEntry d;
	ASSERT_EQ(1,ppl7::File::Stat("writetestäöü.tmp",d)) << "Stat on file is working";
	ASSERT_EQ((size_t)strlen(loremipsum),d.Size) << "filesize as expected";

	ppl7::File ff;
	ASSERT_EQ(1,ff.Open("writetestäöü.tmp"));
	ASSERT_EQ(ppl7::String(loremipsum_md5),ff.MD5Sum()) << "md5sum of file is correct";
	ff.Close();
	ASSERT_EQ(1,ppl7::File::DeleteFile("writetestäöü.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, RenameFileUSAscii2USAscii) {
	ASSERT_EQ(1,ppl7::File::CopyFile("testdata/filenameUSASCII.txt","copy.tmp"));
	ASSERT_EQ(1,ppl7::File::RenameFile("copy.tmp","move.tmp"));
	ASSERT_EQ(0,ppl7::File::Exists("copy.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl7::File::Exists("move.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_EQ(1,ppl7::File::Stat("move.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl7::File ff;
	ASSERT_EQ(1,ff.Open("move.tmp"));
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl7::File::DeleteFile("move.tmp");
}

TEST_F(FileStaticTest, RenameFileUSAscii2Utf8) {
	ASSERT_EQ(1,ppl7::File::CopyFile("testdata/filenameUSASCII.txt","copy.tmp"));
	ASSERT_EQ(1,ppl7::File::RenameFile("copy.tmp","moveäöü.tmp"));
	ASSERT_EQ(0,ppl7::File::Exists("copy.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl7::File::Exists("moveäöü.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_EQ(1,ppl7::File::Stat("moveäöü.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl7::File ff;
	ASSERT_EQ(1,ff.Open("moveäöü.tmp"));
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl7::File::DeleteFile("moveäöü.tmp");
}


TEST_F(FileStaticTest, RenameFileUtf82USAscii) {
	ASSERT_EQ(1,ppl7::File::CopyFile("testdata/filenameUSASCII.txt","copyäöü.tmp"));
	ASSERT_EQ(1,ppl7::File::RenameFile("copyäöü.tmp","move.tmp"));
	ASSERT_EQ(0,ppl7::File::Exists("copyäöü.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl7::File::Exists("move.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_EQ(1,ppl7::File::Stat("move.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ppl7::File ff;
	ASSERT_EQ(1,ff.Open("move.tmp"));
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ff.MD5Sum());
	ff.Close();
	ppl7::File::DeleteFile("move.tmp");
}

TEST_F(FileStaticTest, RenameFileUtf82Utf8) {
	ASSERT_EQ(1,ppl7::File::CopyFile("testdata/filenameUSASCII.txt","copyäöü.tmp"));
	ASSERT_EQ(1,ppl7::File::RenameFile("copyäöü.tmp","moveäöüß.tmp"));
	ASSERT_EQ(0,ppl7::File::Exists("copyäöü.tmp")) << "Old file has disappeared";
	ASSERT_EQ(1,ppl7::File::Exists("moveäöüß.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_EQ(1,ppl7::File::Stat("moveäöüß.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ppl7::File::MD5("moveäöüß.tmp"));
	ppl7::File::DeleteFile("moveäöüß.tmp");
}

TEST_F(FileStaticTest, MD5) {
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ppl7::File::MD5("testdata/filenameUSASCII.txt"));
}

#endif

/*
 * TODO: Chmod und Stat interpretieren die Attribute unterschiedlich
 */
/*
TEST_F(FileStaticTest, ChmodUSAscii) {
	ppl7::File::Chmod("chmod1.tmp",ppl6::FILEATTR::USR_READ|ppl6::FILEATTR::USR_WRITE);
	ASSERT_EQ(1,ppl7::File::CopyFile("testdata/filenameUSASCII.txt","chmod1.tmp"));
	ASSERT_EQ(1,ppl7::File::Chmod("chmod1.tmp",ppl6::FILEATTR::USR_READ));
	ppl7::DirEntry d;
	ASSERT_EQ(1,ppl7::File::Stat("chmod1.tmp",d)) << "Stat on file";
	d.Print();
	ASSERT_EQ(ppl6::FILEATTR::USR_READ,d.Attrib&ppl6::FILEATTR::USR_READ);
	ASSERT_NE(ppl6::FILEATTR::USR_WRITE,d.Attrib&ppl6::FILEATTR::USR_WRITE);
	ppl7::File::DeleteFile("chmod1.tmp");

}
*/


} // EOF namespace

