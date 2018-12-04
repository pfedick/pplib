/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 *******************************************************************************
 * Copyright (c) 2018, Patrick Fedick <patrick@pfp.de>
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
#include <pthread.h>
#include <locale.h>
#include <ppl6.h>
#include <ppl7.h>
#include "ppl7-ppl6compat.h"

#include <gtest/gtest.h>
#include "ppl7-tests.h"

namespace {

// The fixture for testing class Foo.
class PPL6CompatAssocArrayTest : public ::testing::Test {
	protected:
	PPL6CompatAssocArrayTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~PPL6CompatAssocArrayTest() {

	}
};

TEST_F(PPL6CompatAssocArrayTest, 6to7ExportImport) {
	ppl6::CBinary bin("Ein Binary-Object mit reinem ASCII-Text");
	ppl6::CWString w6(L"This as a wide string äöü");

	ppl6::CAssocArray a6;
	a6.Set("key1","Dieser Wert geht über\nmehrere Zeilen");
	a6.Set("key2","value6");
	a6.Set("ebene1/ebene2/key1","value42");
	a6.Set("time",ppl6::CDateTime("03.12.2018 08:39:00.123456"));
	a6.Set("bytearray",bin);
	ppl6::CArray ar6("red green blue yellow black white"," ");
	a6.Set("array1",ar6);
	a6.Set("widestring",w6);

	int requiredsize=0;
	ASSERT_EQ(1,a6.ExportBinary(NULL,0,&requiredsize));
	ASSERT_EQ((int)337,requiredsize);
	void *buffer=malloc(requiredsize+1);
	ASSERT_TRUE(buffer!=NULL);
	int realsize=0;
	ASSERT_EQ(1,a6.ExportBinary(buffer,requiredsize,&realsize));
	//ppl7::HexDump(buffer,realsize);
	ppl7::AssocArray a7;
	ASSERT_NO_THROW({
		try {
			a7.importBinary(buffer,realsize);
		} catch (const ppl7::Exception &exp) {
			exp.print();
			throw;
		}
	});
	free(buffer);
	EXPECT_EQ(ppl7::String("red"),a7.get("array1/0").toString());
	EXPECT_EQ(ppl7::String("white"),a7.get("array1/5").toString());
	EXPECT_EQ(ppl7::String("Dieser Wert geht über\nmehrere Zeilen"),a7.get("key1").toString());
	EXPECT_EQ(ppl7::WideString(L"This as a wide string äöü"),a7.get("widestring").toWideString());
	EXPECT_EQ(ppl7::DateTime("03.12.2018 08:39:00.123456"),a7.get("time").toDateTime());
	EXPECT_EQ(ppl7::String("value42"),a7.get("ebene1/ebene2/key1").toString());
	EXPECT_EQ(ppl7::ByteArray(ppl7::String("Ein Binary-Object mit reinem ASCII-Text")),a7.get("bytearray").toByteArray());
}

TEST_F(PPL6CompatAssocArrayTest, 7to6ExportImport) {
	ppl7::ByteArray bin(ppl7::String("Ein Binary-Object mit reinem ASCII-Text"));
	ppl7::WideString w7(L"This as a wide string äöü");

	ppl7::AssocArray a7;
	a7.set("key1","Dieser Wert geht über\nmehrere Zeilen");
	a7.set("key2","value6");
	a7.set("ebene1/ebene2/key1","value42");
	a7.set("time",ppl7::DateTime("03.12.2018 08:39:00.123456"));
	a7.set("bytearray",bin);
	ppl7::Array ar7("red green blue yellow black white"," ");
	a7.set("array1",ar7);
	a7.set("widestring",w7);
	//a7.list();

	size_t requiredsize=0;
	a7.exportBinary(NULL,0,&requiredsize);
	EXPECT_EQ((size_t)309,requiredsize);
	void *buffer=malloc(requiredsize+1);
	ASSERT_TRUE(buffer!=NULL);
	size_t realsize=0;
	a7.exportBinary(buffer,requiredsize,&realsize);

	//ppl7::HexDump(buffer,realsize);
	ppl6::CAssocArray a6;
	EXPECT_EQ((int)309,a6.ImportBinary(buffer,realsize));
	free(buffer);
	EXPECT_EQ(ppl6::CString("red"),a6.ToCString("array1/0"));
	EXPECT_EQ(ppl6::CString("white"),a6.ToCString("array1/5"));
	EXPECT_EQ(ppl6::CString("Dieser Wert geht über\nmehrere Zeilen"),a6.ToCString("key1"));
	EXPECT_EQ(ppl6::CWString(L"This as a wide string äöü"),a6.ToCWString("widestring"));
	EXPECT_EQ(ppl6::CDateTime("03.12.2018 08:39:00.123456"),a6.GetDateTime("time"));
	EXPECT_EQ(ppl6::CString("value42"),a6.ToCString("ebene1/ebene2/key1"));
	EXPECT_EQ(ppl6::CBinary("Ein Binary-Object mit reinem ASCII-Text"),a6.ToCBinary("bytearray"));
}


}

