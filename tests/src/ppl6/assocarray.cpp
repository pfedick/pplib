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
	a6.Set("time",ppl6::CDateTime("03.12.2018 08:39:00"));
	a6.Set("bytearray",bin);
	ppl6::CArray ar6("red green blue yellow black white"," ");
	a6.Set("array1",ar6);
	a6.Set("widestring",w6);

	int requiredsize=0;
	ASSERT_EQ(1,a6.ExportBinary(NULL,0,&requiredsize));
	ASSERT_EQ((int)257,requiredsize);
	void *buffer=malloc(requiredsize+1);
	ASSERT_TRUE(buffer!=NULL);
	int realsize=0;
	ASSERT_EQ(1,a6.ExportBinary(buffer,requiredsize,&realsize));

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
	a7.list();


}


}

