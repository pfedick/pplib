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
class FileReadTest : public ::testing::Test {
	protected:
	FileReadTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
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
	ASSERT_THROW(f1.open("nonexisting.txt"), ppl7::File::FileNotFoundException);
}

TEST_F(FileReadTest, openExisting) {
	ppl7::File f1;
	ASSERT_NO_THROW(f1.open("../LICENSE.TXT"));
}

TEST_F(FileReadTest, size) {
	ppl7::File f1;
	f1.open("../LICENSE.TXT");
	ASSERT_EQ((ppluint64)1540,f1.size());
	ppl7::File f2;
	f2.open("src/testfile.txt");
	ASSERT_EQ((ppluint64)1592096,f2.size());

}

TEST_F(FileReadTest, openAndClose) {
	ppl7::File f1;
	f1.open("../LICENSE.TXT");
	ASSERT_EQ((ppluint64)1540,f1.size());
	f1.close();
	f1.open("src/testfile.txt");
	ASSERT_EQ((ppluint64)1592096,f1.size());

}

TEST_F(FileReadTest, md5) {
	ppl7::File f1;
	f1.open("src/testfile.txt");
	ppl7::String digest=f1.md5();
	ASSERT_EQ(ppl7::String(L"f386e5ea10bc186b633eaf6ba9a20d8c"),digest);
}

TEST_F(FileReadTest, seekAndTell) {
	ppl7::File f1;
	f1.open("src/testfile.txt");
	f1.seek(45678);
	ASSERT_EQ((ppluint64)45678,f1.ftell());
	f1.seek(100);
	ASSERT_EQ((ppluint64)100,f1.ftell());
	f1.seek(1024*1024);
	ASSERT_EQ((ppluint64)1024*1024,f1.ftell());
}

TEST_F(FileReadTest, fseekAndTell) {
	ppl7::File f1;
	f1.open("src/testfile.txt");
	f1.fseek(45678,ppl7::File::SEEKSET);
	ASSERT_EQ((ppluint64)45678,f1.ftell());
	f1.fseek(100,ppl7::File::SEEKCUR);
	ASSERT_EQ((ppluint64)45778,f1.ftell());
	f1.fseek(-1000,ppl7::File::SEEKCUR);
	ASSERT_EQ((ppluint64)44778,f1.ftell());
	f1.fseek(-1000,ppl7::File::SEEKEND);
	ASSERT_EQ((ppluint64)1591096,f1.ftell());
}



}


int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

