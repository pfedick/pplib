/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
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


extern ppl6::CArray Wordlist;

namespace {

// The fixture for testing class Foo.
class AssocArrayTest : public ::testing::Test {
	protected:

	AssocArrayTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl6::SetGlobalEncoding("UTF-8");

	}
	virtual ~AssocArrayTest() {

	}
};

TEST_F(AssocArrayTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl6::CAssocArray a;
		//ASSERT_EQ(ppl6::CString("success"),res) << "Unexpected return value";
	});

}

TEST_F(AssocArrayTest, addStringsLevel1) {
	ppl6::CAssocArray a;
	ASSERT_NO_THROW({
	a.Set("key1","Dieser Wert geht über\nmehrere Zeilen");
	a.Set("key2","value6");

	});
	ASSERT_EQ((int)2,a.Count()) << "Unexpected size of AssocArray";
	//a.list();
}

TEST_F(AssocArrayTest, addStringsMultiLevels) {
	ppl6::CAssocArray a;
	ASSERT_NO_THROW({
		a.Set("key1","Dieser Wert geht über\nmehrere Zeilen");
		a.Set("key2","value6");
		a.Set("array1/unterkey1","value2");
		a.Set("array1/unterkey2","value3");
		a.Set("array1/noch ein array/unterkey1","value4");
		a.Set("array1/unterkey2","value5");
		a.Set("key2","value7");
		a.Set("array2/unterkey1","value7");
		a.Set("array2/unterkey2","value8");
		a.Set("array2/unterkey1","value9");
	});
	//a.list();
	ASSERT_EQ((int)4,a.Count()) << "Unexpected size of AssocArray";
	ASSERT_EQ((int)10,a.Count(true)) << "Unexpected size of AssocArray";
	ASSERT_EQ(ppl6::CString("Dieser Wert geht über\nmehrere Zeilen"),a.GetString("key1")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("value7"),a.GetString("key2")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("value5"),a.GetString("array1/unterkey2")) << "unexpected value";
}

TEST_F(AssocArrayTest, addMixed) {
	ppl6::CAssocArray a;
	ppl6::CDateTime now;
	ASSERT_NO_THROW({
		a.Set("key1","Dieser Wert geht über\nmehrere Zeilen");
		a.Set("key2","value6");
		now.setCurrentTime();
		a.Set("time",now);
		ppl6::CBinary ba(1234);
		a.Set("bytearray",ba);
		ppl6::CArray a1("red green blue yellow black white"," ");
		a.Set("array1",a1);
	});
	ASSERT_EQ(now,a.GetDateTime("time")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("Dieser Wert geht über\nmehrere Zeilen"),a.GetString("key1")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("value6"),a.GetString("key2")) << "unexpected value";
	ppl6::CString s=a.ToCArray("array1").Implode(" ");
	ASSERT_EQ(ppl6::CString("red green blue yellow black white"),s) << "unexpected value";

	//a.list();
}

TEST_F(AssocArrayTest, append) {
	ppl6::CAssocArray a;
	ASSERT_NO_THROW({
		a.Set("key","Dieser Wert geht über\nmehrere Zeilen");
		a.Concat("key","Noch eine Zeile","\n");
	});
	ASSERT_EQ((int)1,a.Count()) << "Unexpected size of AssocArray";
	ASSERT_EQ(ppl6::CString("Dieser Wert geht über\nmehrere Zeilen\nNoch eine Zeile"),a.GetString("key")) << "unexpected value";
	//a.list();
}

TEST_F(AssocArrayTest, getAssocArray) {
	ppl6::CAssocArray a;
	ASSERT_NO_THROW({
		a.Set("key1","Dieser Wert geht über\nmehrere Zeilen");
		a.Set("key2","value6");
		a.Set("array1/unterkey1","value2");
		a.Set("array1/unterkey2","value3");
		a.Set("array1/noch ein array/unterkey1","value4");
		a.Set("array1/unterkey2","value5");
		a.Set("key2","value6");
		//a.set("dateien/main.cpp",&bin);
		a.Set("array2/unterkey1","value7");
		a.Set("array2/unterkey2","value8");
		a.Set("array2/unterkey1","value9");
	});
	ASSERT_NO_THROW({
		ppl6::CAssocArray a2=a.ToCAssocArray("array1");
		ASSERT_EQ((int)3,a2.Count()) << "Unexpected size of AssocArray";
		ppl6::CAssocArray a3=a.ToCAssocArray("array2");
		ASSERT_EQ((int)2,a3.Count()) << "Unexpected size of AssocArray";
	});
}

TEST_F(AssocArrayTest, addAndDeleteWordlist) {
	ppl6::CAssocArray a;
	int total=Wordlist.Size();
	//a.reserve(total+10);
	ppl6::PrintDebugTime ("Loading wordlist\n");
	ppl6::CString empty;
	for (int i=0;i<total;i++) {
		ASSERT_EQ(1,a.Set(Wordlist[i],empty));
	}
	ppl6::PrintDebugTime ("done\n");
	ASSERT_EQ(total,a.Count()) << "Tree has unexpected size";

	ppl6::PrintDebugTime ("Wortliste loeschen\n");
	for (int i=0;i<total;i++) {
		ASSERT_EQ(1,a.Delete(Wordlist[i])) << "Keyword was not deleted: " << Wordlist.GetString(i);
	}
	ppl6::PrintDebugTime ("done\n");
	ASSERT_EQ((int)0,a.Count()) << "Tree has unexpected size";

}

TEST_F(AssocArrayTest, fromTemplate) {
	ppl6::CAssocArray a1, a2;
	ppl6::CString Template("key=line1\n"
			"key = line2 \n"
			"foo  =  bar\n"
			"words=20\n"
			" blah=blubb\n"
			"hello=world");
	ASSERT_NO_THROW({
		a1.CreateFromTemplate(Template,"\n","=","\n",false);
		a2.CreateFromTemplate(Template,"\n","=","\n",true);
	});
	ASSERT_EQ(ppl6::CString("line1\n line2 "),a1.ToCString("key")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("  bar"),a1.ToCString("foo")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("20"),a1.ToCString("words")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("blubb"),a1.ToCString("blah")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("world"),a1.ToCString("hello")) << "unexpected value";


	ASSERT_EQ(ppl6::CString("line1\nline2"),a2.ToCString("key")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("bar"),a2.ToCString("foo")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("20"),a2.ToCString("words")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("blubb"),a2.ToCString("blah")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("world"),a2.ToCString("hello")) << "unexpected value";

}

TEST_F(AssocArrayTest, fromConfig) {
	ppl6::CAssocArray a1, a2;
	ppl6::CString Template("[Abschnitt_1]\n"
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
		a1.CreateFromConfig(Template,"=","\n",false);
		a2.CreateFromConfig(Template,"=","\n",true);
	});
	//a1.list("a1");

	ASSERT_EQ(ppl6::CString("line1\n line2 "),a1.ToCString("Abschnitt_1/key")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("  bar"),a1.ToCString("Abschnitt_1/foo")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("20"),a1.ToCString("Abschnitt_1/words")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("value1"),a1.ToCString("Abschnitt_1/key1")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("value2"),a1.ToCString("Abschnitt_1/key2")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("  blubb "),a1.ToCString("Abschnitt_2/blah")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("world"),a1.ToCString("Abschnitt_2/hello")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("value3"),a1.ToCString("Abschnitt_2/key1")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("value4"),a1.ToCString("Abschnitt_2/key2")) << "unexpected value";

	ASSERT_EQ(ppl6::CString("line1\nline2"),a2.ToCString("Abschnitt_1/key")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("bar"),a2.ToCString("Abschnitt_1/foo")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("20"),a2.ToCString("Abschnitt_1/words")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("value1"),a2.ToCString("Abschnitt_1/key1")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("value2"),a2.ToCString("Abschnitt_1/key2")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("blubb"),a2.ToCString("Abschnitt_2/blah")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("world"),a2.ToCString("Abschnitt_2/hello")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("value3"),a2.ToCString("Abschnitt_2/key1")) << "unexpected value";
	ASSERT_EQ(ppl6::CString("value4"),a2.ToCString("Abschnitt_2/key2")) << "unexpected value";
}


}	// EOF namespace

