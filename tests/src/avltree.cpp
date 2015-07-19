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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <ppl7.h>
#include <gtest/gtest.h>
#include "ppl7-tests.h"

extern ppl7::Array Wordlist;

namespace {

// The fixture for testing class Foo.
class AVLTreeTest : public ::testing::Test {
	protected:

	AVLTreeTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");

	}
	virtual ~AVLTreeTest() {

	}
};

TEST_F(AVLTreeTest, ConstructorSimple) {
	ppl7::AVLTree<ppl7::String, ppl7::String> myMap;
	//printf ("sizeof(MyMap::Item)=%zu\n",myMap.itemSize());
	// ACHTUNG: itemSize ändert sich, sobald am Knoten des Baums, dem Datentyp des Keys oder
	// dem Datentyp des Values etwas verändert wird
	//ASSERT_EQ((size_t)120,myMap.itemSize()) << "Tree Item has unexpected size";
}

TEST_F(AVLTreeTest, add) {
	ppl7::AVLTree<ppl7::String, ppl7::String> myMap;
	myMap.add("key1","value1");
	ASSERT_EQ((size_t)1,myMap.count()) << "Tree has unexpected size";
	myMap.add("other","value2");
	ASSERT_EQ((size_t)2,myMap.count()) << "Tree has unexpected size";
}

TEST_F(AVLTreeTest, addAndDeleteWordlist) {
	ppl7::AVLTree<ppl7::String, ppl7::String> myMap;
	size_t total=Wordlist.count();
	myMap.reserve(total+10);
	ASSERT_EQ((size_t)124332,Wordlist.count()) << "List has unexpected size";
	ppl7::PrintDebugTime ("Wortliste in AVLTree laden\n");
	for (size_t i=0;i<total;i++) {
		myMap.add(Wordlist[i],"");
	}
	ASSERT_EQ(total,myMap.count()) << "Tree has unexpected size";
	ppl7::PrintDebugTime ("done\n");
	ppl7::PrintDebugTime ("Wortliste aus AVLTree loeschen\n");
	for (size_t i=0;i<total;i++) {
		myMap.erase(Wordlist[i]);
	}
	ppl7::PrintDebugTime ("done\n");
	ASSERT_EQ((size_t)0,myMap.count()) << "Tree has unexpected size";

}

TEST_F(AVLTreeTest, addAndClearWordlist) {
	ppl7::AVLTree<ppl7::String, ppl7::String> myMap;
	size_t total=Wordlist.count();
	myMap.reserve(total+10);
	for (size_t i=0;i<total;i++) {
		myMap.add(Wordlist[i],"");
	}
	myMap.clear();
	ASSERT_EQ((size_t)0,myMap.count()) << "Tree has unexpected size";
}


TEST_F(AVLTreeTest, find) {
	ppl7::AVLTree<ppl7::String, ppl7::String> myMap;
	myMap.add("key1","value1");
	myMap.add("other","value2");
	myMap.add("findme","success");
	myMap.add("key3","value3");
	myMap.add("abc","value4");
	ASSERT_EQ((size_t)5,myMap.count()) << "Tree has unexpected size";
	ASSERT_NO_THROW({
		const ppl7::String &res=myMap.find("findme");
		ASSERT_EQ(ppl7::String("success"),res) << "Unexpected return value";
	});

	ASSERT_THROW({
		myMap.find("findmenot");
	},ppl7::ItemNotFoundException);

}

TEST_F(AVLTreeTest, getNext) {
	ppl7::AVLTree<ppl7::String, ppl7::String> myMap;
	myMap.add("key1","value1");
	myMap.add("other","value2");
	myMap.add("findme","success");
	myMap.add("key3","value3");
	myMap.add("abc","value4");
	ASSERT_EQ((size_t)5,myMap.count()) << "Tree has unexpected size";
	ppl7::AVLTree<ppl7::String, ppl7::String>::Iterator it;
	myMap.reset(it);

	ASSERT_EQ(true, myMap.getNext(it)) << "getNext has not returned true";
	ASSERT_EQ(ppl7::String("abc"),it.key()) << "key has unexpected value";
	ASSERT_EQ(ppl7::String("value4"),it.value()) << "value has unexpected value";

	ASSERT_EQ(true, myMap.getNext(it)) << "getNext has not returned true";
	ASSERT_EQ(ppl7::String("findme"),it.key()) << "key has unexpected value";
	ASSERT_EQ(ppl7::String("success"),it.value()) << "value has unexpected value";

	ASSERT_EQ(true, myMap.getNext(it)) << "getNext has not returned true";
	ASSERT_EQ(ppl7::String("key1"),it.key()) << "key has unexpected value";
	ASSERT_EQ(ppl7::String("value1"),it.value()) << "value has unexpected value";

	ASSERT_EQ(true, myMap.getNext(it)) << "getNext has not returned true";
	ASSERT_EQ(ppl7::String("key3"),it.key()) << "key has unexpected value";
	ASSERT_EQ(ppl7::String("value3"),it.value()) << "value has unexpected value";

	ASSERT_EQ(true, myMap.getNext(it)) << "getNext has not returned true";
	ASSERT_EQ(ppl7::String("other"),it.key()) << "key has unexpected value";
	ASSERT_EQ(ppl7::String("value2"),it.value()) << "value has unexpected value";

	ASSERT_EQ(false, myMap.getNext(it)) << "getNext has not returned false";
}

}	// EOF namespace


