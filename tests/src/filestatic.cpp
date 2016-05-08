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
	}
	virtual ~FileStaticTest() {

	}
};


TEST_F(FileStaticTest, StatOnUSAscii) {
	ppl7::DirEntry d;
	ASSERT_NO_THROW({
		(ppl7::File::statFile)("testdata/filenameUSASCII.txt",d);
	});
	ASSERT_EQ((size_t)1962,d.Size);
	ASSERT_EQ(ppl7::String("testdata/filenameUSASCII.txt"),d.File);
}


TEST_F(FileStaticTest, StatOnUtf8) {
	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::statFile("testdata/filenameUTF8äöü.txt",d));
	ASSERT_EQ((size_t)1962,d.Size);
	ASSERT_EQ(ppl7::String("testdata/filenameUTF8äöü.txt"),d.File);
}


TEST_F(FileStaticTest, StatOnNonexistingFile) {
	ppl7::DirEntry d;
	ASSERT_THROW(ppl7::File::statFile("testdata/nonexisting.txt",d),ppl7::FileNotFoundException);
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

TEST_F(FileStaticTest, md5Hash) {
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ppl7::File::md5Hash("testdata/filenameUSASCII.txt"));
}


TEST_F(FileStaticTest, CopyFileUSAscii2USAscii) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","copy.tmp"));
	ASSERT_TRUE(ppl7::File::exists("testdata/filenameUSASCII.txt")) << "Old file still exists";
	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::statFile("copy.tmp",d)) << "New file exists";
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
	ASSERT_NO_THROW(ppl7::File::statFile("copyäöüß.tmp",d)) << "New file exists";
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
	ASSERT_NO_THROW(ppl7::File::statFile("copyäöüß.tmp",d)) << "New file exists";
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
	ASSERT_NO_THROW(ppl7::File::statFile("copy2.tmp",d)) << "New file exists";
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
	ASSERT_NO_THROW(ppl7::File::statFile("truncate.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);
	ASSERT_NO_THROW(ppl7::File::truncate("truncate.tmp",1024));
	ASSERT_NO_THROW(ppl7::File::statFile("truncate.tmp",d));
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
	ASSERT_NO_THROW(ppl7::File::statFile("truncateäöüß.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);
	ASSERT_NO_THROW(ppl7::File::truncate("truncateäöüß.tmp",1024));
	ASSERT_NO_THROW(ppl7::File::statFile("truncateäöüß.tmp",d));
	ASSERT_EQ((size_t)1024,d.Size);
	ppl7::File ff;
	ASSERT_NO_THROW(ff.open("truncateäöüß.tmp"));
	ASSERT_EQ(ppl7::String("657351fba38e20fb0a4713e605f1d6a4"),ff.md5());
	ff.close();
	ppl7::File::remove("truncateäöüß.tmp");
}


TEST_F(FileStaticTest, MoveFileUSAscii2USAscii) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","move1a.tmp"));
	ASSERT_NO_THROW(ppl7::File::move("move1a.tmp","move1b.tmp"));
	ASSERT_FALSE(ppl7::File::exists("move1a.tmp")) << "Old file has disappeared";
	ASSERT_TRUE(ppl7::File::exists("move1b.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::statFile("move1b.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ppl7::File::md5Hash("move1b.tmp"));
	ppl7::File::remove("move1b.tmp");
}

TEST_F(FileStaticTest, MoveFileUSAscii2Utf8) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","move2a.tmp"));
	ASSERT_NO_THROW(ppl7::File::move("move2a.tmp","move2bäöü.tmp"));
	ASSERT_FALSE(ppl7::File::exists("move2a.tmp")) << "Old file has disappeared";
	ASSERT_TRUE(ppl7::File::exists("move2bäöü.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::statFile("move2bäöü.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ppl7::File::md5Hash("move2bäöü.tmp"));
	ppl7::File::remove("move2bäöü.tmp");
}

TEST_F(FileStaticTest, MoveFileUtf82USAscii) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","move3aäöü.tmp"));
	ASSERT_NO_THROW(ppl7::File::move("move3aäöü.tmp","move3b.tmp"));
	ASSERT_FALSE(ppl7::File::exists("move3aäöü.tmp")) << "Old file has disappeared";
	ASSERT_TRUE(ppl7::File::exists("move3b.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::statFile("move3b.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ppl7::File::md5Hash("move3b.tmp"));
	ppl7::File::remove("move3b.tmp");
}

TEST_F(FileStaticTest, MoveFileUtf82Utf8) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","move4aäöü.tmp"));
	ASSERT_NO_THROW(ppl7::File::move("move4aäöü.tmp","move4bäöü.tmp"));
	ASSERT_FALSE(ppl7::File::exists("move4aäöü.tmp")) << "Old file has disappeared";
	ASSERT_TRUE(ppl7::File::exists("move4bäöü.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::statFile("move4bäöü.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ppl7::File::md5Hash("move4bäöü.tmp"));
	ppl7::File::remove("move4bäöü.tmp");
}

TEST_F(FileStaticTest, RenameFileUSAscii2USAscii) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","move1a.tmp"));
	ASSERT_NO_THROW(ppl7::File::rename("move1a.tmp","move1b.tmp"));
	ASSERT_FALSE(ppl7::File::exists("move1a.tmp")) << "Old file has disappeared";
	ASSERT_TRUE(ppl7::File::exists("move1b.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::statFile("move1b.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ppl7::File::md5Hash("move1b.tmp"));
	ppl7::File::remove("move1b.tmp");
}

TEST_F(FileStaticTest, RenameFileUSAscii2Utf8) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","move2a.tmp"));
	ASSERT_NO_THROW(ppl7::File::rename("move2a.tmp","move2bäöü.tmp"));
	ASSERT_FALSE(ppl7::File::exists("move2a.tmp")) << "Old file has disappeared";
	ASSERT_TRUE(ppl7::File::exists("move2bäöü.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::statFile("move2bäöü.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ppl7::File::md5Hash("move2bäöü.tmp"));
	ppl7::File::remove("move2bäöü.tmp");
}

TEST_F(FileStaticTest, RenameFileUtf82USAscii) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","move3aäöü.tmp"));
	ASSERT_NO_THROW(ppl7::File::rename("move3aäöü.tmp","move3b.tmp"));
	ASSERT_FALSE(ppl7::File::exists("move3aäöü.tmp")) << "Old file has disappeared";
	ASSERT_TRUE(ppl7::File::exists("move3b.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::statFile("move3b.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ppl7::File::md5Hash("move3b.tmp"));
	ppl7::File::remove("move3b.tmp");
}

TEST_F(FileStaticTest, RenameFileUtf82Utf8) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","move4aäöü.tmp"));
	ASSERT_NO_THROW(ppl7::File::rename("move4aäöü.tmp","move4bäöü.tmp"));
	ASSERT_FALSE(ppl7::File::exists("move4aäöü.tmp")) << "Old file has disappeared";
	ASSERT_TRUE(ppl7::File::exists("move4bäöü.tmp")) << "New file exists";

	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::statFile("move4bäöü.tmp",d));
	ASSERT_EQ((size_t)1962,d.Size);

	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ppl7::File::md5Hash("move4bäöü.tmp"));
	ppl7::File::remove("move4bäöü.tmp");
}



TEST_F(FileStaticTest, LoadUSAscii2CString) {
	ppl7::String s;
	ASSERT_NO_THROW(ppl7::File::load(s,"testdata/filenameUSASCII.txt")) << "Loading file to string";
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),s.md5()) << "Checking MD5-sum";
	ASSERT_EQ((size_t)1962,s.size()) << "Checking Size";
}

TEST_F(FileStaticTest, LoadFileUSAscii2ByteArray) {
	ppl7::ByteArray s;
	ASSERT_NO_THROW(ppl7::File::load(s,"testdata/filenameUSASCII.txt")) << "Loading file to string";
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),s.md5()) << "Checking MD5-sum";
	ASSERT_EQ((size_t)1962,s.size()) << "Checking Size";

}

TEST_F(FileStaticTest, LoadFileUtf82CString) {
	ppl7::String s;
	ASSERT_NO_THROW(ppl7::File::load(s,"testdata/filenameUTF8äöü.txt")) << "Loading file to string";
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),s.md5()) << "Checking MD5-sum";
	ASSERT_EQ((size_t)1962,s.size()) << "Checking Size";
}

TEST_F(FileStaticTest, LoadFileUtf82ByteArray) {
	ppl7::ByteArray s;
	ASSERT_NO_THROW(ppl7::File::load(s,"testdata/filenameUTF8äöü.txt")) << "Loading file to string";
	ASSERT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),s.md5()) << "Checking MD5-sum";
	ASSERT_EQ((size_t)1962,s.size()) << "Checking Size";

}

TEST_F(FileStaticTest, LoadFileUSAscii2Ptr) {
	void *adr=NULL;
	size_t size;
	ASSERT_NO_THROW({adr=ppl7::File::load("testdata/filenameUSASCII.txt",&size);});
	ASSERT_TRUE(adr!=NULL) << "Loading file to ptr";
	EXPECT_EQ((size_t)1962,size) << "Checking Size";
	EXPECT_EQ(ppl7::String("978fd668b5755ce6017256d0ff9e36b2"),ppl7::Md5(adr,size)) << "Checking MD5-sum";
	free(adr);
}

TEST_F(FileStaticTest, LoadFileUtf82Ptr) {
	void *adr=NULL;
	size_t size;
	ASSERT_NO_THROW({adr=ppl7::File::load("testdata/unicodeUtf8äöü.txt",&size);});
	ASSERT_TRUE(adr!=NULL) << "Loading file to ptr";
	EXPECT_EQ((size_t)7976,size) << "Checking Size";
	EXPECT_EQ(ppl7::String("c6767ca956f9e3b65164bed63b93d811"),ppl7::Md5(adr,size)) << "Checking MD5-sum";
	free(adr);
}


TEST_F(FileStaticTest, DeleteFileUSAscii) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","todelete.tmp"));
	ASSERT_NO_THROW(ppl7::File::remove("todelete.tmp")) << "deleting file";
	ASSERT_FALSE(ppl7::File::exists("todelete.tmp")) << "file does not exists anymore";
}

TEST_F(FileStaticTest, DeleteFileUtf8) {
	ASSERT_NO_THROW(ppl7::File::copy("testdata/filenameUSASCII.txt","todeleteäöü.tmp"));
	ASSERT_NO_THROW(ppl7::File::remove("todeleteäöü.tmp")) << "deleting file";
	ASSERT_FALSE(ppl7::File::exists("todeleteäöü.tmp")) << "file does not exists anymore";
}

TEST_F(FileStaticTest, TouchFileUSAscii) {
	ASSERT_NO_THROW(ppl7::File::touch("touched.tmp")) << "touching file";
	ASSERT_TRUE(ppl7::File::exists("touched.tmp")) << "file exists";
	ASSERT_NO_THROW(ppl7::File::remove("touched.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, TouchFileUtf8) {
	ASSERT_NO_THROW(ppl7::File::touch("touchedäöü.tmp")) << "touching file";
	ASSERT_TRUE(ppl7::File::exists("touchedäöü.tmp")) << "file exists";
	ASSERT_NO_THROW(ppl7::File::remove("touchedäöü.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, SavePtrUSAscii) {
	ASSERT_NO_THROW(ppl7::File::save(loremipsum,strlen(loremipsum),"writetest.tmp")) << "writing file";
	ASSERT_TRUE(ppl7::File::exists("writetest.tmp")) << "file exists";
	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::statFile("writetest.tmp",d)) << "Stat on file is working";
	ASSERT_EQ((size_t)strlen(loremipsum),d.Size) << "filesize as expected";
	ASSERT_EQ(ppl7::String(loremipsum_md5),ppl7::File::md5Hash("writetest.tmp"));
	ASSERT_NO_THROW(ppl7::File::remove("writetest.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, SavePtrUtf8) {
	ASSERT_NO_THROW(ppl7::File::save(loremipsum,strlen(loremipsum),"writetestäöü.tmp")) << "writing file";
	ASSERT_TRUE(ppl7::File::exists("writetestäöü.tmp")) << "file exists";
	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::statFile("writetestäöü.tmp",d)) << "Stat on file is working";
	ASSERT_EQ((size_t)strlen(loremipsum),d.Size) << "filesize as expected";
	ASSERT_EQ(ppl7::String(loremipsum_md5),ppl7::File::md5Hash("writetestäöü.tmp"));
	ASSERT_NO_THROW(ppl7::File::remove("writetestäöü.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, SaveByteArrayPtrUSAscii) {
	ppl7::ByteArrayPtr ba;
	ba.use((void*)loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW(ppl7::File::save(ba,"writetest.tmp")) << "writing file";
	ASSERT_TRUE(ppl7::File::exists("writetest.tmp")) << "file exists";
	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::statFile("writetest.tmp",d)) << "Stat on file is working";
	ASSERT_EQ((size_t)strlen(loremipsum),d.Size) << "filesize as expected";
	ASSERT_EQ(ppl7::String(loremipsum_md5),ppl7::File::md5Hash("writetest.tmp"));
	ASSERT_NO_THROW(ppl7::File::remove("writetest.tmp")) << "deleting file";
}

TEST_F(FileStaticTest, SaveByteArrayPtrUtf8) {
	ppl7::ByteArrayPtr ba;
	ba.use((void*)loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW(ppl7::File::save(ba,"writetestäöü.tmp")) << "writing file";
	ASSERT_TRUE(ppl7::File::exists("writetestäöü.tmp")) << "file exists";
	ppl7::DirEntry d;
	ASSERT_NO_THROW(ppl7::File::statFile("writetestäöü.tmp",d)) << "Stat on file is working";
	ASSERT_EQ((size_t)strlen(loremipsum),d.Size) << "filesize as expected";
	ASSERT_EQ(ppl7::String(loremipsum_md5),ppl7::File::md5Hash("writetestäöü.tmp"));
	ASSERT_NO_THROW(ppl7::File::remove("writetestäöü.tmp")) << "deleting file";
}


TEST_F(FileStaticTest, getPath) {
	EXPECT_EQ(ppl7::String("c:\\windows\\system32"),ppl7::File::getPath("c:\\windows\\system32\\test.dll"));
	EXPECT_EQ(ppl7::String("c:/windows/system32"),ppl7::File::getPath("c:/windows/system32/test.dll"));
	EXPECT_EQ(ppl7::String("/usr/bin"),ppl7::File::getPath("/usr/bin/bash"));
}

TEST_F(FileStaticTest, getFilename) {
	EXPECT_EQ(ppl7::String("test.dll"),ppl7::File::getFilename("c:\\windows\\system32\\test.dll"));
	EXPECT_EQ(ppl7::String("test.dll"),ppl7::File::getFilename("c:/windows/system32/test.dll"));
	EXPECT_EQ(ppl7::String("bash"),ppl7::File::getFilename("/usr/bin/bash"));
}

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

