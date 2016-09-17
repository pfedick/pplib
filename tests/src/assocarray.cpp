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
class AssocArrayTest : public ::testing::Test {
	protected:

	AssocArrayTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~AssocArrayTest() {

	}
};

TEST_F(AssocArrayTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl7::AssocArray a;
		//ASSERT_EQ(ppl7::String("success"),res) << "Unexpected return value";
	});

}

TEST_F(AssocArrayTest, addStringsLevel1) {
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
	a.set("key1","Dieser Wert geht über\nmehrere Zeilen");
	a.set("key2","value6");

	});
	ASSERT_EQ((size_t)2,a.count()) << "Unexpected size of AssocArray";
	//a.list();
}

TEST_F(AssocArrayTest, addStringsMultiLevels) {
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		a.set("key1","Dieser Wert geht über\nmehrere Zeilen");
		a.set("key2","value6");
		a.set("array1/unterkey1","value2");
		a.set("array1/unterkey2","value3");
		a.set("array1/noch ein array/unterkey1","value4");
		a.set("array1/unterkey2","value5");
		a.set("key2","value7");
		a.set("array2/unterkey1","value7");
		a.set("array2/unterkey2","value8");
		a.set("array2/unterkey1","value9");
	});
	ASSERT_EQ((size_t)4,a.count()) << "Unexpected size of AssocArray";
	ASSERT_EQ((size_t)10,a.count(true)) << "Unexpected size of AssocArray";
	ASSERT_EQ(ppl7::String("Dieser Wert geht über\nmehrere Zeilen"),a.getString("key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value7"),a.getString("key2")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value5"),a.getString("array1/unterkey2")) << "unexpected value";
}

TEST_F(AssocArrayTest, addMixed) {
	ppl7::AssocArray a;
	ppl7::DateTime now;
	ASSERT_NO_THROW({
		a.set("key1","Dieser Wert geht über\nmehrere Zeilen");
		a.set("key2","value6");
		now.setCurrentTime();
		a.set("time",now);
		ppl7::ByteArray ba(1234);
		ppl7::ByteArrayPtr bap=ba;
		a.set("bytearray",ba);
		a.set("bytearrayptr",bap);
		ppl7::Array a1("red green blue yellow black white"," ");
		a.set("array1",a1);
	});
	ASSERT_EQ(now,a.get("time").toDateTime()) << "unexpected value";
	ASSERT_EQ(ppl7::String("Dieser Wert geht über\nmehrere Zeilen"),a.getString("key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value6"),a.getString("key2")) << "unexpected value";
	ppl7::String s=a.get("array1").toArray().implode(" ");
	ASSERT_EQ(ppl7::String("red green blue yellow black white"),s) << "unexpected value";

	//a.list();
}

TEST_F(AssocArrayTest, append) {
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		a.set("key","Dieser Wert geht über\nmehrere Zeilen");
		a.append("key","Noch eine Zeile","\n");
	});
	ASSERT_EQ((size_t)1,a.count()) << "Unexpected size of AssocArray";
	ASSERT_EQ(ppl7::String("Dieser Wert geht über\nmehrere Zeilen\nNoch eine Zeile"),a.getString("key")) << "unexpected value";
	//a.list();
}


TEST_F(AssocArrayTest, getAssocArray) {
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		a.set("key1","Dieser Wert geht über\nmehrere Zeilen");
		a.set("key2","value6");
		a.set("array1/unterkey1","value2");
		a.set("array1/unterkey2","value3");
		a.set("array1/noch ein array/unterkey1","value4");
		a.set("array1/unterkey2","value5");
		a.set("key2","value6");
		//a.set("dateien/main.cpp",&bin);
		a.set("array2/unterkey1","value7");
		a.set("array2/unterkey2","value8");
		a.set("array2/unterkey1","value9");
	});
	//a.list();
	ASSERT_NO_THROW({
		const ppl7::AssocArray &a2=a.get("array1").toAssocArray();
		ASSERT_EQ((size_t)3,a2.count()) << "Unexpected size of AssocArray";
		ppl7::AssocArray &a3=a.get("array2").toAssocArray();
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
		} catch (ppl7::KeyNotFoundException &) {

		}
	}
	ppl7::PrintDebugTime ("done\n");
	ASSERT_EQ((size_t)0,a.count()) << "Tree has unexpected size";

}

TEST_F(AssocArrayTest, fromTemplate) {
	ppl7::AssocArray a1, a2;
	ppl7::String Template("key=line1\n"
			"key = line2 \n"
			"foo  =  bar\n"
			"words=20\n"
			" blah=blubb\n"
			"hello=world");
	ASSERT_NO_THROW({
		a1.fromTemplate(Template,"\n","=","\n",false);
		a2.fromTemplate(Template,"\n","=","\n",true);
	});
	ASSERT_EQ(ppl7::String("line1\n line2 "),a1.getString("key")) << "unexpected value";
	ASSERT_EQ(ppl7::String("  bar"),a1.getString("foo")) << "unexpected value";
	ASSERT_EQ(ppl7::String("20"),a1.getString("words")) << "unexpected value";
	ASSERT_EQ(ppl7::String("blubb"),a1.getString("blah")) << "unexpected value";
	ASSERT_EQ(ppl7::String("world"),a1.getString("hello")) << "unexpected value";


	ASSERT_EQ(ppl7::String("line1\nline2"),a2.getString("key")) << "unexpected value";
	ASSERT_EQ(ppl7::String("bar"),a2.getString("foo")) << "unexpected value";
	ASSERT_EQ(ppl7::String("20"),a2.getString("words")) << "unexpected value";
	ASSERT_EQ(ppl7::String("blubb"),a2.getString("blah")) << "unexpected value";
	ASSERT_EQ(ppl7::String("world"),a2.getString("hello")) << "unexpected value";

}

TEST_F(AssocArrayTest, fromConfig) {
	ppl7::AssocArray a1, a2;
	ppl7::String Template("[Abschnitt_1]\n"
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
		a1.fromConfig(Template,"\n","=","\n",false);
		a2.fromConfig(Template,"\n","=","\n",true);
	});
	//a1.list("a1");

	ASSERT_EQ(ppl7::String("line1\n line2 "),a1.getString("Abschnitt_1/key")) << "unexpected value";
	ASSERT_EQ(ppl7::String("  bar"),a1.getString("Abschnitt_1/foo")) << "unexpected value";
	ASSERT_EQ(ppl7::String("20"),a1.getString("Abschnitt_1/words")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value1"),a1.getString("Abschnitt_1/key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value2"),a1.getString("Abschnitt_1/key2")) << "unexpected value";
	ASSERT_EQ(ppl7::String("  blubb "),a1.getString("Abschnitt_2/blah")) << "unexpected value";
	ASSERT_EQ(ppl7::String("world"),a1.getString("Abschnitt_2/hello")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value3"),a1.getString("Abschnitt_2/key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value4"),a1.getString("Abschnitt_2/key2")) << "unexpected value";

	ASSERT_EQ(ppl7::String("line1\nline2"),a2.getString("Abschnitt_1/key")) << "unexpected value";
	ASSERT_EQ(ppl7::String("bar"),a2.getString("Abschnitt_1/foo")) << "unexpected value";
	ASSERT_EQ(ppl7::String("20"),a2.getString("Abschnitt_1/words")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value1"),a2.getString("Abschnitt_1/key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value2"),a2.getString("Abschnitt_1/key2")) << "unexpected value";
	ASSERT_EQ(ppl7::String("blubb"),a2.getString("Abschnitt_2/blah")) << "unexpected value";
	ASSERT_EQ(ppl7::String("world"),a2.getString("Abschnitt_2/hello")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value3"),a2.getString("Abschnitt_2/key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value4"),a2.getString("Abschnitt_2/key2")) << "unexpected value";
}

static void createDefaultAssocArray(ppl7::AssocArray &a)
{
	ppl7::AssocArray data;
	a.set("key1","Dieser Wert geht über\nmehrere Zeilen");
	a.set("key2","value6");
	a.set("array1/unterkey1","value2");
	a.set("array1/unterkey2","value3");
	a.set("array1/noch ein array/unterkey1","value4");
	a.set("array1/unterkey2","value5");
	a.set("key2","value7");
	a.set("array2/unterkey1","value7");
	a.set("array2/unterkey2","value8");
	a.set("array2/unterkey1","value9");

	data.setf("sampleTime","%0.6f",22362546.32543);
	data.setf("net_receive/bytes","%lu",(unsigned long)1);
	data.setf("net_receive/packets","%lu",(unsigned long)2);
	data.setf("net_receive/errs","%lu",(unsigned long)3);
	data.setf("net_receive/drop","%lu",(unsigned long)4);
	data.setf("net_transmit/bytes","%lu",(unsigned long)5);
	data.setf("net_transmit/packets","%lu",(unsigned long)6);
	data.setf("net_transmit/errs","%lu",(unsigned long)7);
	data.setf("net_transmit/drop","%lu",(unsigned long)8);

	data.setf("cpu/user","%d",1);
	data.setf("cpu/nice","%d",2);
	data.setf("cpu/system","%d",3);
	data.setf("cpu/idle","%d",4);
	data.setf("cpu/iowait","%d",5);

	data.setf("sysinfo/uptime","%ld",(long)32324234213);
	data.setf("sysinfo/freeswap","%ld",(long)2345215545);
	data.setf("sysinfo/totalswap","%ld",(long)65463635);
	data.setf("sysinfo/freeram","%ld",(long)5467254523);
	data.setf("sysinfo/bufferram","%ld",(long)549153452345);
	data.setf("sysinfo/totalram","%ld",(long)24346579);
	data.setf("sysinfo/sharedram","%ld",(long)232356657);
	data.setf("sysinfo/procs","%d",12321);
	a.set("data/[]",data);
	a.set("data/[]",data);
}

TEST_F(AssocArrayTest, binarySize) {
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		createDefaultAssocArray(a);
	});
	ASSERT_EQ((size_t)1215,a.binarySize());
}

TEST_F(AssocArrayTest, exportAndImportBinary) {
	ppl7::AssocArray a;
	ppl7::AssocArray b;
	ASSERT_NO_THROW({
		createDefaultAssocArray(a);
	});
	void *buffer=malloc(8192);
	ASSERT_TRUE(buffer!=NULL) << "out of memory";
	size_t realsize=0;
	EXPECT_NO_THROW({
		a.exportBinary(buffer,8192,&realsize);
		ASSERT_EQ((size_t)1215,realsize);
		b.importBinary(buffer,realsize);
	});
	free(buffer);

	ASSERT_EQ(a.count(),b.count()) << "Unexpected size of AssocArray";
	ASSERT_EQ(a.count(true), b.count(true)) << "Unexpected size of AssocArray";
	ASSERT_EQ(ppl7::String("Dieser Wert geht über\nmehrere Zeilen"),b.getString("key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value7"),b.getString("key2")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value5"),b.getString("array1/unterkey2")) << "unexpected value";
	ASSERT_EQ(ppl7::String("32324234213"),b.getString("data/1/sysinfo/uptime")) << "unexpected value";
	ASSERT_EQ(ppl7::String("3"),b.getString("data/0/cpu/system")) << "unexpected value";
}

}	// EOF namespace

