/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 790 $
 * $Date: 2013-10-21 15:50:51 +0200 (Mo, 21 Okt 2013) $
 * $Id: CApplication.cpp 790 2013-10-21 13:50:51Z pafe $
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

namespace {

class CFileTest : public ::testing::Test {
	protected:
	CFileTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl6::SetGlobalEncoding("UTF-8");
	}
	virtual ~CFileTest() {

	}
};

TEST_F(CFileTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl6::CFile f1;
		ASSERT_FALSE(f1.IsOpen()) << "File seems to be open, but it shouldn't";
	}
	);
}

TEST_F(CFileTest, openNonexisting) {
	ppl6::CFile f1;
	ASSERT_EQ(0,f1.Open("nonexisting.txt"));
}

TEST_F(CFileTest, openNonexistingUtf8) {
	ppl6::CFile f1;
	ASSERT_EQ(0,f1.Open("noneäxisting.txt"));
}

TEST_F(CFileTest, openExistingUsAscii) {
	ppl6::CFile f1;
	ASSERT_EQ(1,f1.Open("testdata/filenameUSASCII.txt"));
}

TEST_F(CFileTest, openExistingUtf8) {
	ppl6::CFile f1;
	ASSERT_EQ(1,f1.Open("testdata/filenameUTF8äöü.txt"));
}

TEST_F(CFileTest, openfNonexisting) {
	ppl6::CFile f1;
	ASSERT_EQ(0,f1.Openf("testdata/%s.%s","rb","nonexisting","txt"));
}

TEST_F(CFileTest, openfNonexistingUtf8) {
	ppl6::CFile f1;
	ASSERT_EQ(0,f1.Openf("testdata/%s.%s","rb","noneäxisting","txt"));
}

TEST_F(CFileTest, openfExistingUsAscii) {
	ppl6::CFile f1;
	ASSERT_EQ(1,f1.Openf("testdata/%s.%s","rb","filenameUSASCII","txt"));
}

TEST_F(CFileTest, openfExistingUtf8) {
	ppl6::CFile f1;
	ASSERT_EQ(1,f1.Openf("testdata/%s.%s","rb","filenameUTF8äöü","txt"));
}

TEST_F(CFileTest, Puts) {
	ppl6::CFile f1;
	ASSERT_EQ(1,f1.Open("file_puts.tmp","wb"));
	ASSERT_EQ(1,f1.Puts(loremipsum));
	ASSERT_EQ((pplint64)strlen(loremipsum),f1.Size());
	f1.Close();
	ASSERT_EQ(ppl6::CString(loremipsum_md5),ppl6::CFile::MD5("file_puts.tmp")) << "md5sum of file is correct";
	ASSERT_EQ(1,ppl6::CFile::DeleteFile("file_puts.tmp")) << "deleting file";
}

TEST_F(CFileTest, Write) {
	ppl6::CFile f1;
	ASSERT_EQ(1,f1.Open("file_write.tmp","wb"));
	ASSERT_EQ((size_t)strlen(loremipsum),f1.Write(loremipsum,strlen(loremipsum)));
	ASSERT_EQ((pplint64)strlen(loremipsum),f1.Size());
	f1.Close();
	ASSERT_EQ(ppl6::CString(loremipsum_md5),ppl6::CFile::MD5("file_write.tmp")) << "md5sum of file is correct";
	ASSERT_EQ(1,ppl6::CFile::DeleteFile("file_write.tmp")) << "deleting file";
}

TEST_F(CFileTest, OpenTempUSAscii) {
	ppl6::CFile f1;
	ASSERT_EQ(1,f1.OpenTemp("tmpXXXXXX"));
	ppl6::CString filename=f1.Filename();
	ASSERT_EQ((size_t)9,filename.Len());
	ASSERT_EQ((size_t)strlen(loremipsum),f1.Write(loremipsum,strlen(loremipsum)));
	ASSERT_EQ((pplint64)strlen(loremipsum),f1.Size());
	f1.Close();
	ASSERT_EQ(ppl6::CString(loremipsum_md5),ppl6::CFile::MD5(filename)) << "md5sum of file is correct";
	ASSERT_EQ(1,ppl6::CFile::DeleteFile(filename)) << "deleting file";
}

TEST_F(CFileTest, OpenTempUtf8) {
	ppl6::CFile f1;
	ASSERT_EQ(1,f1.OpenTemp("tmp_äöü_XXXXXX"));
	ppl6::CString filename=f1.Filename();
	ASSERT_EQ((size_t)17,filename.Len());
	ASSERT_EQ((size_t)strlen(loremipsum),f1.Write(loremipsum,strlen(loremipsum)));
	ASSERT_EQ((pplint64)strlen(loremipsum),f1.Size());
	f1.Close();
	ASSERT_EQ(ppl6::CString(loremipsum_md5),ppl6::CFile::MD5(filename)) << "md5sum of file is correct";
	ASSERT_EQ(1,ppl6::CFile::DeleteFile(filename)) << "deleting file";
}

TEST_F(CFileTest, PopenTempUSAscii) {
	ppl6::CFile f1;
	ppl6::CFile::DeleteFile("popentest1.tmp");
	ASSERT_EQ(1,f1.Popen("bin/popentest.sh","w")) << "could not open pipe";
	ASSERT_EQ(1,f1.Puts("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod\n"));
	ASSERT_EQ(1,f1.Puts("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod\n"));
	ASSERT_EQ(1,f1.Puts("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod\n"));
	ASSERT_EQ(1,f1.Puts("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod\n"));
	ASSERT_EQ(1,f1.Puts("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod\n"));
	ASSERT_EQ(1,f1.Puts("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod\n"));
	ASSERT_EQ(1,f1.Close());
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("popentest1.tmp",d))  << "file popentest1.tmp was not created";
	ASSERT_EQ((size_t)480,d.Size) << "file popentest1.tmp has unexpected size";
	ASSERT_EQ(ppl6::CString("ae02ae6bdd7036d0bb1515f6b5b3ecd0"),ppl6::CFile::MD5("popentest1.tmp")) << "md5sum of file is correct";
	ASSERT_EQ(1,ppl6::CFile::DeleteFile("popentest1.tmp")) << "deleting file";
}

TEST_F(CFileTest, PopenTempUtf8) {
	ppl6::CFile f1;
	ppl6::CFile::DeleteFile("popentest2.tmp");
	ASSERT_EQ(1,f1.Popen("bin/popentäst.sh","w")) << "could not open pipe";
	ASSERT_EQ(1,f1.Puts("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod\n"));
	ASSERT_EQ(1,f1.Puts("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod\n"));
	ASSERT_EQ(1,f1.Puts("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod\n"));
	ASSERT_EQ(1,f1.Puts("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod\n"));
	ASSERT_EQ(1,f1.Puts("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod\n"));
	ASSERT_EQ(1,f1.Puts("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod\n"));

	ASSERT_EQ(1,f1.Close());
	ppl6::CDirEntry d;
	ASSERT_EQ(1,ppl6::CFile::Stat("popentest2.tmp",d)) << "file popentest2.tmp was not created";
	ASSERT_EQ((size_t)480,d.Size) << "file popentest2.tmp has unexpected size";
	ASSERT_EQ(ppl6::CString("ae02ae6bdd7036d0bb1515f6b5b3ecd0"),ppl6::CFile::MD5("popentest2.tmp")) << "md5sum of file is correct";
	ASSERT_EQ(1,ppl6::CFile::DeleteFile("popentest2.tmp")) << "deleting file";
}


}


