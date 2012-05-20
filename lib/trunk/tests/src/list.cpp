/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 89 $
 * $Date: 2011-05-07 12:03:12 +0200 (Sa, 07. Mai 2011) $
 * $Id: avltree.cpp 89 2011-05-07 10:03:12Z pafe $
 * $URL: https://pplib.svn.sourceforge.net/svnroot/pplib/lib/trunk/tests/src/avltree.cpp $
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
#include <ppl7.h>
#include <gtest/gtest.h>


extern const wchar_t *wordlist;

extern ppl7::Array Wordlist;

namespace {

// The fixture for testing class Foo.
class ListTest : public ::testing::Test {
	protected:

	ListTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");

	}
	virtual ~ListTest() {

	}
};

TEST_F(ListTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl7::List<ppl7::String> myList;
	});
}

TEST_F(ListTest, add) {
	ppl7::List<ppl7::String> myList;
	myList.add("Item 1");
	ASSERT_EQ((size_t)1,myList.count());
	myList.add("Item 2");
	ASSERT_EQ((size_t)2,myList.count());
}

TEST_F(ListTest, clear) {
	ppl7::List<ppl7::String> myList;
	myList.add("Item 1");
	myList.add("Item 2");
	myList.add("Item 3");

	ASSERT_EQ((size_t)3,myList.count());
	myList.clear();
	ASSERT_EQ((size_t)0,myList.count());
}

TEST_F(ListTest, reset) {
	ppl7::List<ppl7::String> myList;
	myList.add("Item 1");
	myList.add("Item 2");
	myList.add("Item 3");

	ppl7::List<ppl7::String>::Iterator it;
	myList.reset(it);

}

TEST_F(ListTest, getNext) {
	ppl7::List<ppl7::String> myList;
	myList.add("Item 1");
	myList.add("Item 2");
	myList.add("Item 3");

	ppl7::List<ppl7::String>::Iterator it;
	myList.reset(it);
	ASSERT_EQ(true,myList.getNext(it));
	ASSERT_EQ(ppl7::String("Item 1"),it.value());

	ASSERT_EQ(true,myList.getNext(it));
	ASSERT_EQ(ppl7::String("Item 2"),it.value());

	ASSERT_EQ(true,myList.getNext(it));
	ASSERT_EQ(ppl7::String("Item 3"),it.value());

	ASSERT_EQ(false,myList.getNext(it));
}

TEST_F(ListTest, getPrevious) {
	ppl7::List<ppl7::String> myList;
	myList.add("Item 1");
	myList.add("Item 2");
	myList.add("Item 3");

	ppl7::List<ppl7::String>::Iterator it;
	myList.reset(it);
	ASSERT_EQ(true,myList.getPrevious(it));
	ASSERT_EQ(ppl7::String("Item 3"),it.value());

	ASSERT_EQ(true,myList.getPrevious(it));
	ASSERT_EQ(ppl7::String("Item 2"),it.value());

	ASSERT_EQ(true,myList.getPrevious(it));
	ASSERT_EQ(ppl7::String("Item 1"),it.value());

	ASSERT_EQ(false,myList.getPrevious(it));
}

TEST_F(ListTest, addWordlist) {
	ppl7::List<ppl7::String> myList;
	size_t total=Wordlist.count();
	myList.reserve(total+10);
	ASSERT_EQ((size_t)125346,Wordlist.count()) << "List has unexpected size";
	//ppl7::PrintDebugTime ("Wortliste in List laden\n");
	for (size_t i=0;i<total;i++) {
		myList.add(Wordlist[i]);
	}
	ASSERT_EQ(total,myList.count()) << "List has unexpected size";
	//ppl7::PrintDebugTime ("done\n");
}




}	// EOF namespace

