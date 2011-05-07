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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <ppl7.h>
#include <gtest/gtest.h>


extern const wchar_t *wordlist;

ppl7::Array Wordlist;

namespace {

// The fixture for testing class Foo.
class AssocArrayTest : public ::testing::Test {
	protected:

	AssocArrayTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");

	}
	virtual ~AssocArrayTest() {

	}
};

TEST_F(AssocArrayTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl7::AssocArray a;
		//ASSERT_EQ(ppl7::String(L"success"),res) << "Unexpected return value";
	});

}

TEST_F(AssocArrayTest, addStringsLevel1) {
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
	a.set(L"key1",L"Dieser Wert geht über\nmehrere Zeilen");
	a.set(L"key2",L"value6");

	});
	ASSERT_EQ((size_t)2,a.count()) << "Unexpected size of AssocArray";
	//a.list();
}

TEST_F(AssocArrayTest, addStringsMultiLevels) {
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		a.set(L"key1",L"Dieser Wert geht über\nmehrere Zeilen");
		a.set(L"key2",L"value6");
		a.set(L"array1/unterkey1",L"value2");
		a.set(L"array1/unterkey2",L"value3");
		a.set(L"array1/noch ein array/unterkey1",L"value4");
		a.set(L"array1/unterkey2",L"value5");
		a.set(L"key2",L"value7");
		a.set(L"array2/unterkey1",L"value7");
		a.set(L"array2/unterkey2",L"value8");
		a.set(L"array2/unterkey1",L"value9");
	});
	//a.list();
	ASSERT_EQ((size_t)4,a.count()) << "Unexpected size of AssocArray";
	ASSERT_EQ((size_t)10,a.count(true)) << "Unexpected size of AssocArray";
	ASSERT_EQ(ppl7::String(L"Dieser Wert geht über\nmehrere Zeilen"),a.getString(L"key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"value7"),a.getString(L"key2")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"value5"),a.getString(L"array1/unterkey2")) << "unexpected value";
}

TEST_F(AssocArrayTest, addMixed) {
	ppl7::AssocArray a;
	ppl7::DateTime now;
	ASSERT_NO_THROW({
		a.set(L"key1",L"Dieser Wert geht über\nmehrere Zeilen");
		a.set(L"key2",L"value6");
		now.setCurrentTime();
		a.set(L"time",now);
		ppl7::ByteArray ba(1234);
		ppl7::ByteArrayPtr bap=ba;
		a.set(L"bytearray",ba);
		a.set(L"bytearrayptr",bap);
		ppl7::Array a1(L"red green blue yellow black white",L" ");
		a.set(L"array1",a1);
	});
	ASSERT_EQ(now,a.get(L"time").toDateTime()) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"Dieser Wert geht über\nmehrere Zeilen"),a.getString(L"key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"value6"),a.getString(L"key2")) << "unexpected value";
	ppl7::String s=a.get(L"array1").toArray().implode(" ");
	ASSERT_EQ(ppl7::String(L"red green blue yellow black white"),s) << "unexpected value";

	//a.list();
}

TEST_F(AssocArrayTest, append) {
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		a.set(L"key",L"Dieser Wert geht über\nmehrere Zeilen");
		a.append(L"key",L"Noch eine Zeile",L"\n");
	});
	ASSERT_EQ((size_t)1,a.count()) << "Unexpected size of AssocArray";
	ASSERT_EQ(ppl7::String(L"Dieser Wert geht über\nmehrere Zeilen\nNoch eine Zeile"),a.getString(L"key")) << "unexpected value";
	//a.list();
}


TEST_F(AssocArrayTest, getAssocArray) {
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		a.set(L"key1",L"Dieser Wert geht über\nmehrere Zeilen");
		a.set(L"key2",L"value6");
		a.set(L"array1/unterkey1",L"value2");
		a.set(L"array1/unterkey2",L"value3");
		a.set(L"array1/noch ein array/unterkey1",L"value4");
		a.set(L"array1/unterkey2",L"value5");
		a.set(L"key2",L"value6");
		//a.set(L"dateien/main.cpp",&bin);
		a.set(L"array2/unterkey1",L"value7");
		a.set(L"array2/unterkey2",L"value8");
		a.set(L"array2/unterkey1",L"value9");
	});
	ASSERT_NO_THROW({
		ppl7::AssocArray &a2=a.get(L"array1").toAssocArray();
		ASSERT_EQ((size_t)3,a2.count()) << "Unexpected size of AssocArray";
		ppl7::AssocArray &a3=a.get(L"array2").toAssocArray();
		ASSERT_EQ((size_t)2,a3.count()) << "Unexpected size of AssocArray";
	});
}

TEST_F(AssocArrayTest, addAndDeleteWordlist) {
	ppl7::AssocArray a;
	size_t total=Wordlist.count();
	a.reserve(total+10);
	ppl7::PrintDebugTime ("Loading wordlist\n");
	ppl7::String empty;
	for (size_t i=0;i<total;i++) {
		a.set(Wordlist[i],empty);
	}
	ppl7::PrintDebugTime ("done\n");
	//ASSERT_EQ(total,a.count()) << "Tree has unexpected size";

	ppl7::PrintDebugTime ("Wortliste aus AVLTree loeschen\n");
	for (size_t i=0;i<total;i++) {
		try {
			a.erase(Wordlist[i]);
		} catch (ppl7::AssocArray::KeyNotFoundException) {

		}
	}
	ppl7::PrintDebugTime ("done\n");
	ASSERT_EQ((size_t)0,a.count()) << "Tree has unexpected size";

}

TEST_F(AssocArrayTest, fromTemplate) {
	ppl7::AssocArray a1, a2;
	ppl7::String Template(L"key=line1\n"
			"key = line2 \n"
			"foo  =  bar\n"
			"words=20\n"
			" blah=blubb\n"
			"hello=world");
	ASSERT_NO_THROW({
		a1.fromTemplate(Template,L"\n",L"=",L"\n",false);
		a2.fromTemplate(Template,L"\n",L"=",L"\n",true);
	});
	ASSERT_EQ(ppl7::String(L"line1\n line2 "),a1.getString(L"key")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"  bar"),a1.getString(L"foo")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"20"),a1.getString(L"words")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"blubb"),a1.getString(L"blah")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"world"),a1.getString(L"hello")) << "unexpected value";


	ASSERT_EQ(ppl7::String(L"line1\nline2"),a2.getString(L"key")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"bar"),a2.getString(L"foo")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"20"),a2.getString(L"words")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"blubb"),a2.getString(L"blah")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"world"),a2.getString(L"hello")) << "unexpected value";

}

TEST_F(AssocArrayTest, fromConfig) {
	ppl7::AssocArray a1, a2;
	ppl7::String Template(L"[Abschnitt_1]\n"
			"key =line1\n"
			"key = line2 \n"
			"foo  =  bar\n"
			"words=20\n"
			"# Kommentarzeile, die überlesen wird\n"
			"key1=value1\n"
			"key2=value2\n"
			"[Abschnitt_2]\n"
			"key1=value3\n"
			"key2=value4\n"
			" blah=  blubb \n"
			"hello=world");
	ASSERT_NO_THROW({
		a1.fromConfig(Template,L"\n",L"=",L"\n",false);
		a2.fromConfig(Template,L"\n",L"=",L"\n",true);
	});
	//a1.list(L"a1");

	ASSERT_EQ(ppl7::String(L"line1\n line2 "),a1.getString(L"Abschnitt_1/key")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"  bar"),a1.getString(L"Abschnitt_1/foo")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"20"),a1.getString(L"Abschnitt_1/words")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"value1"),a1.getString(L"Abschnitt_1/key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"value2"),a1.getString(L"Abschnitt_1/key2")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"  blubb "),a1.getString(L"Abschnitt_2/blah")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"world"),a1.getString(L"Abschnitt_2/hello")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"value3"),a1.getString(L"Abschnitt_2/key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"value4"),a1.getString(L"Abschnitt_2/key2")) << "unexpected value";

	ASSERT_EQ(ppl7::String(L"line1\nline2"),a2.getString(L"Abschnitt_1/key")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"bar"),a2.getString(L"Abschnitt_1/foo")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"20"),a2.getString(L"Abschnitt_1/words")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"value1"),a2.getString(L"Abschnitt_1/key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"value2"),a2.getString(L"Abschnitt_1/key2")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"blubb"),a2.getString(L"Abschnitt_2/blah")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"world"),a2.getString(L"Abschnitt_2/hello")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"value3"),a2.getString(L"Abschnitt_2/key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String(L"value4"),a2.getString(L"Abschnitt_2/key2")) << "unexpected value";
}


}	// EOF namespace

int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);

	ppl7::String w(wordlist);
	Wordlist.reserve(130000);
	Wordlist.explode(w,L"\n");

	return RUN_ALL_TESTS();
}

