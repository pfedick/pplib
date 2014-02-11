/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 887 $
 * $Date: 2014-02-04 14:24:47 +0100 (Di, 04 Feb 2014) $
 * $Id: CDir.cpp 887 2014-02-04 13:24:47Z pafe $
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <ppl6.h>
#include <gtest/gtest.h>
#include "ppl6-tests.h"

extern const wchar_t *wordlist;

extern ppl6::CArray Wordlist;

namespace {

// The fixture for testing class Foo.
class ListTest : public ::testing::Test {
	protected:

	ListTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl6::SetGlobalEncoding("UTF-8");

	}
	virtual ~ListTest() {

	}
};

TEST_F(ListTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl6::List<ppl6::CString> myList;
	});
}

TEST_F(ListTest, add) {
	ppl6::List<ppl6::CString> myList;
	myList.add("Item 1");
	ASSERT_EQ((size_t)1,myList.count());
	myList.add("Item 2");
	ASSERT_EQ((size_t)2,myList.count());
}

TEST_F(ListTest, clear) {
	ppl6::List<ppl6::CString> myList;
	myList.add("Item 1");
	myList.add("Item 2");
	myList.add("Item 3");

	ASSERT_EQ((size_t)3,myList.count());
	myList.clear();
	ASSERT_EQ((size_t)0,myList.count());
}

TEST_F(ListTest, reset) {
	ppl6::List<ppl6::CString> myList;
	myList.add("Item 1");
	myList.add("Item 2");
	myList.add("Item 3");

	ppl6::List<ppl6::CString>::Iterator it;
	myList.reset(it);

}

TEST_F(ListTest, getNext) {
	ppl6::List<ppl6::CString> myList;
	myList.add("Item 1");
	myList.add("Item 2");
	myList.add("Item 3");

	ppl6::List<ppl6::CString>::Iterator it;
	myList.reset(it);
	ASSERT_EQ(true,myList.getNext(it));
	ASSERT_EQ(ppl6::CString("Item 1"),it.value());

	ASSERT_EQ(true,myList.getNext(it));
	ASSERT_EQ(ppl6::CString("Item 2"),it.value());

	ASSERT_EQ(true,myList.getNext(it));
	ASSERT_EQ(ppl6::CString("Item 3"),it.value());

	ASSERT_EQ(false,myList.getNext(it));
}

TEST_F(ListTest, getPrevious) {
	ppl6::List<ppl6::CString> myList;
	myList.add("Item 1");
	myList.add("Item 2");
	myList.add("Item 3");

	ppl6::List<ppl6::CString>::Iterator it;
	myList.reset(it);
	ASSERT_EQ(true,myList.getPrevious(it));
	ASSERT_EQ(ppl6::CString("Item 3"),it.value());

	ASSERT_EQ(true,myList.getPrevious(it));
	ASSERT_EQ(ppl6::CString("Item 2"),it.value());

	ASSERT_EQ(true,myList.getPrevious(it));
	ASSERT_EQ(ppl6::CString("Item 1"),it.value());

	ASSERT_EQ(false,myList.getPrevious(it));
}

TEST_F(ListTest, addWordlist) {
	ppl6::List<ppl6::CString> myList;
	size_t total=Wordlist.Size();
	myList.reserve(total+10);
	ASSERT_EQ((size_t)124332,Wordlist.Size()) << "List has unexpected size";
	//ppl7::PrintDebugTime ("Wortliste in List laden\n");
	for (size_t i=0;i<total;i++) {
		myList.add(Wordlist[i]);
	}
	ASSERT_EQ(total,myList.count()) << "List has unexpected size";
	//ppl7::PrintDebugTime ("done\n");
}




}	// EOF namespace

