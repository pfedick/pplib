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
		if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
			printf("setlocale fehlgeschlagen\n");
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
	ASSERT_EQ((size_t)2, a.count()) << "Unexpected size of AssocArray";
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
	ASSERT_EQ((size_t)4, a.count()) << "Unexpected size of AssocArray";
	ASSERT_EQ((size_t)10, a.count(true)) << "Unexpected size of AssocArray";
	ASSERT_EQ(ppl7::String("Dieser Wert geht über\nmehrere Zeilen"), a.getString("key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value7"), a.getString("key2")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value5"), a.getString("array1/unterkey2")) << "unexpected value";
}

TEST_F(AssocArrayTest, addMixed) {
	ppl7::AssocArray a;
	ppl7::DateTime now=ppl7::DateTime::currentTime();
	ppl7::ByteArray ba(1234);
	ppl7::ByteArrayPtr bap=ba;
	ppl7::Random(ba, 1234);

	ASSERT_NO_THROW({
		a.set("key1","Dieser Wert geht über\nmehrere Zeilen");
		a.set("key2","value6");
		now.setCurrentTime();
		a.set("time",now);
		a.set("bytearray",ba);
		a.set("bytearrayptr",bap);
		ppl7::Array a1("red green blue yellow black white"," ");
		a.set("array1",a1);
		});
	ASSERT_EQ(now, a.get("time").toDateTime()) << "unexpected value";
	ASSERT_EQ(ppl7::String("Dieser Wert geht über\nmehrere Zeilen"), a.getString("key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value6"), a.getString("key2")) << "unexpected value";
	ppl7::String s=a.get("array1").toArray().implode(" ");
	ASSERT_EQ(ppl7::String("red green blue yellow black white"), s) << "unexpected value";
	ASSERT_EQ(ba, a.get("bytearray").toByteArray()) << "unexpected value";
	ASSERT_EQ(bap, a.get("bytearrayptr").toByteArrayPtr()) << "unexpected value";

	//a.list();
}

TEST_F(AssocArrayTest, append) {
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		a.set("key","Dieser Wert geht über\nmehrere Zeilen");
		a.append("key","Noch eine Zeile","\n");
		});
	ASSERT_EQ((size_t)1, a.count()) << "Unexpected size of AssocArray";
	ASSERT_EQ(ppl7::String("Dieser Wert geht über\nmehrere Zeilen\nNoch eine Zeile"), a.getString("key")) << "unexpected value";
	//a.list();
}

TEST_F(AssocArrayTest, appendRecursive) {
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		a.set("level1/key","First Value");
		a.append("level1/key","Second Value","\n");
		});
	ASSERT_EQ((size_t)1, a.count()) << "Unexpected size of AssocArray";
	ASSERT_EQ(ppl7::String("First Value\nSecond Value"), a.getString("level1/key")) << "unexpected value";
	//a.list();
}

TEST_F(AssocArrayTest, appendNonExisting) {
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		a.append("key","First Value","\n");
		a.append("level1/key","Second Value","\n");
		});
	ASSERT_EQ((size_t)2, a.count()) << "Unexpected size of AssocArray";
	ASSERT_EQ(ppl7::String("First Value"), a.getString("key")) << "unexpected value";
	ASSERT_EQ(ppl7::String("Second Value"), a.getString("level1/key")) << "unexpected value";
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
		const ppl7::AssocArray & a2=a.get("array1").toAssocArray();
		ASSERT_EQ((size_t)3,a2.count()) << "Unexpected size of AssocArray";
		ppl7::AssocArray& a3=a.get("array2").toAssocArray();
		ASSERT_EQ((size_t)2,a3.count()) << "Unexpected size of AssocArray";
		});
}

TEST_F(AssocArrayTest, addAndDeleteWordlist) {
	ppl7::AssocArray a;
	size_t total=Wordlist.count();
	ppl7::PrintDebugTime("Loading wordlist\n");
	ppl7::String empty;
	for (size_t i=0;i < total;i++) {
		a.set(Wordlist[i], empty);
	}
	ppl7::PrintDebugTime("done\n");
	//ASSERT_EQ(total,a.count()) << "Tree has unexpected size";

	ppl7::PrintDebugTime("Deleting wordlist\n");
	for (size_t i=0;i < total;i++) {
		try {
			a.erase(Wordlist[i]);
		} catch (ppl7::KeyNotFoundException&) {

		}
	}
	ppl7::PrintDebugTime("done\n");
	ASSERT_EQ((size_t)0, a.count()) << "Tree has unexpected size";

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
	ASSERT_EQ(ppl7::String("line1\n line2 "), a1.getString("key")) << "unexpected value";
	ASSERT_EQ(ppl7::String("  bar"), a1.getString("foo")) << "unexpected value";
	ASSERT_EQ(ppl7::String("20"), a1.getString("words")) << "unexpected value";
	ASSERT_EQ(ppl7::String("blubb"), a1.getString("blah")) << "unexpected value";
	ASSERT_EQ(ppl7::String("world"), a1.getString("hello")) << "unexpected value";


	ASSERT_EQ(ppl7::String("line1\nline2"), a2.getString("key")) << "unexpected value";
	ASSERT_EQ(ppl7::String("bar"), a2.getString("foo")) << "unexpected value";
	ASSERT_EQ(ppl7::String("20"), a2.getString("words")) << "unexpected value";
	ASSERT_EQ(ppl7::String("blubb"), a2.getString("blah")) << "unexpected value";
	ASSERT_EQ(ppl7::String("world"), a2.getString("hello")) << "unexpected value";

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

	ASSERT_EQ(ppl7::String("line1\n line2 "), a1.getString("Abschnitt_1/key")) << "unexpected value";
	ASSERT_EQ(ppl7::String("  bar"), a1.getString("Abschnitt_1/foo")) << "unexpected value";
	ASSERT_EQ(ppl7::String("20"), a1.getString("Abschnitt_1/words")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value1"), a1.getString("Abschnitt_1/key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value2"), a1.getString("Abschnitt_1/key2")) << "unexpected value";
	ASSERT_EQ(ppl7::String("  blubb "), a1.getString("Abschnitt_2/blah")) << "unexpected value";
	ASSERT_EQ(ppl7::String("world"), a1.getString("Abschnitt_2/hello")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value3"), a1.getString("Abschnitt_2/key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value4"), a1.getString("Abschnitt_2/key2")) << "unexpected value";

	ASSERT_EQ(ppl7::String("line1\nline2"), a2.getString("Abschnitt_1/key")) << "unexpected value";
	ASSERT_EQ(ppl7::String("bar"), a2.getString("Abschnitt_1/foo")) << "unexpected value";
	ASSERT_EQ(ppl7::String("20"), a2.getString("Abschnitt_1/words")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value1"), a2.getString("Abschnitt_1/key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value2"), a2.getString("Abschnitt_1/key2")) << "unexpected value";
	ASSERT_EQ(ppl7::String("blubb"), a2.getString("Abschnitt_2/blah")) << "unexpected value";
	ASSERT_EQ(ppl7::String("world"), a2.getString("Abschnitt_2/hello")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value3"), a2.getString("Abschnitt_2/key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value4"), a2.getString("Abschnitt_2/key2")) << "unexpected value";
}

static void createDefaultAssocArray(ppl7::AssocArray& a)
{
	ppl7::AssocArray data;
	a.set("key1", "Dieser Wert geht ueber\nmehrere Zeilen");
	a.set("key2", "value6");
	a.set("array1/unterkey1", "value2");
	a.set("array1/unterkey2", "value3");
	a.set("array1/noch ein array/unterkey1", "value4");
	a.set("array1/unterkey2", "value5");
	a.set("key2", "value7");
	a.set("array2/unterkey1", "value7");
	a.set("array2/unterkey2", "value8");
	a.set("array2/unterkey1", "value9");
	a.set("stringarray", ppl7::Array("red green blue white", " "));


	a.set("widestring", ppl7::WideString(L"this is a widestring - äöü"));
	a.set("datetime", ppl7::DateTime("2018-12-03 13:49:10.123456"));


	data.setf("sampleTime", "%0.6f", 22362546.32543);
	data.setf("net_receive/bytes", "%lu", (unsigned long)1);
	data.setf("net_receive/packets", "%lu", (unsigned long)2);
	data.setf("net_receive/errs", "%lu", (unsigned long)3);
	data.setf("net_receive/drop", "%lu", (unsigned long)4);
	data.setf("net_transmit/bytes", "%lu", (unsigned long)5);
	data.setf("net_transmit/packets", "%lu", (unsigned long)6);
	data.setf("net_transmit/errs", "%lu", (unsigned long)7);
	data.setf("net_transmit/drop", "%lu", (unsigned long)8);

	data.setf("cpu/user", "%d", 1);
	data.setf("cpu/nice", "%d", 2);
	data.setf("cpu/system", "%d", 3);
	data.setf("cpu/idle", "%d", 4);
	data.setf("cpu/iowait", "%d", 5);

	data.setf("sysinfo/uptime", "%lld", (long long)32324234213);
	data.setf("sysinfo/freeswap", "%lld", (long long)2345215545);
	data.setf("sysinfo/totalswap", "%lld", (long long)65463635);
	data.setf("sysinfo/freeram", "%lld", (long long)5467254523);
	data.setf("sysinfo/bufferram", "%lld", (long long)549153452345);
	data.setf("sysinfo/totalram", "%lld", (long long)24346579);
	data.setf("sysinfo/sharedram", "%lld", (long long)232356657);
	data.setf("sysinfo/procs", "%d", 12321);
	a.set("data/[]", data);
	a.set("data/[]", data);
	//a.list();
}

TEST_F(AssocArrayTest, binarySize) {
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		createDefaultAssocArray(a);
		});
	ASSERT_EQ((size_t)1335, a.binarySize());
}

TEST_F(AssocArrayTest, exportAndImportBinary) {
	ppl7::AssocArray a;
	ppl7::AssocArray b;
	ASSERT_NO_THROW({
		createDefaultAssocArray(a);
		});
	void* buffer=malloc(8192);
	ASSERT_TRUE(buffer != NULL) << "out of memory";
	size_t realsize=0;
	EXPECT_NO_THROW({
		try {
			a.exportBinary(buffer,8192,&realsize);
		} catch (const ppl7::Exception& exp) {
			exp.print();
			throw;
		}
		});
		//ppl7::HexDump(buffer,realsize);
	ASSERT_EQ((size_t)1335, realsize);
	EXPECT_NO_THROW({
		b.importBinary(buffer,realsize);
		});
	free(buffer);

	ASSERT_EQ(a.count(), b.count()) << "Unexpected size of AssocArray";
	ASSERT_EQ(a.count(true), b.count(true)) << "Unexpected size of AssocArray";
	ASSERT_EQ(ppl7::String("Dieser Wert geht ueber\nmehrere Zeilen"), b.getString("key1")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value7"), b.getString("key2")) << "unexpected value";
	ASSERT_EQ(ppl7::String("value5"), b.getString("array1/unterkey2")) << "unexpected value";
	ASSERT_EQ(ppl7::String("32324234213"), b.getString("data/1/sysinfo/uptime")) << "unexpected value";
	ASSERT_EQ(ppl7::String("3"), b.getString("data/0/cpu/system")) << "unexpected value";
	ASSERT_EQ(ppl7::WideString(L"this is a widestring - äöü"), b.get("widestring").toWideString()) << "unexpected value";
	ASSERT_EQ(ppl7::DateTime("2018-12-03 13:49:10.123456"), b.get("datetime").toDateTime()) << "unexpected value";
	ASSERT_EQ(ppl7::Array("red green blue white", " "), b.get("stringarray").toArray()) << "unexpected value";
}

static void createWalkingArray(ppl7::AssocArray& a)
{
	ppl7::DateTime now=ppl7::DateTime::currentTime();
	ppl7::ByteArray ba(1234);
	ppl7::ByteArrayPtr bap=ba;
	ppl7::Random(ba, 1234);
	a.set("time", now);
	a.set("aaaa", "first element");
	a.set("blah", "blubb");
	a.set("bytearray", ba);
	a.set("bytearrayptr", bap);
	ppl7::Array a1("red green blue yellow black white", " ");
	a.set("array0", a1);
	a.set("key1", "Dieser Wert geht über\nmehrere Zeilen");
	a.set("key2", "value6");
	a.set("array1/unterkey1", "value2");
	a.set("array1/unterkey2", "value3");
	a.set("array1/noch ein array/unterkey1", "value4");
	a.set("array1/unterkey2", "value5");
	a.set("key3", "value7");
	a.set("array2/unterkey1", "value7");
	a.set("array2/unterkey2", "value8");
	a.set("array2/unterkey1", "value9");
}

TEST_F(AssocArrayTest, IterateResetGetNextWithoutDatatype) {
	ppl7::AssocArray a;
	createWalkingArray(a);
	ppl7::AssocArray::Iterator it;
	a.reset(it);
	ASSERT_TRUE(a.getNext(it));
	ASSERT_EQ(ppl7::String("aaaa"), it.key());
	ASSERT_TRUE(it.value().isString());
}


TEST_F(AssocArrayTest, IterateGetFirstGetNextWithoutDatatype) {
	ppl7::AssocArray a;
	createWalkingArray(a);
	ppl7::AssocArray::Iterator it;

	ASSERT_TRUE(a.getFirst(it));
	ASSERT_EQ(ppl7::String("aaaa"), it.key());
	ASSERT_TRUE(it.value().isString());

	ASSERT_TRUE(a.getNext(it));
	ASSERT_EQ(ppl7::String("array0"), it.key());
	ASSERT_TRUE(it.value().isArray());

	ASSERT_TRUE(a.getNext(it));
	ASSERT_EQ(ppl7::String("array1"), it.key());
	ASSERT_TRUE(it.value().isAssocArray());

	ASSERT_TRUE(a.getNext(it));
	ASSERT_EQ(ppl7::String("array2"), it.key());
	ASSERT_TRUE(it.value().isAssocArray());

	ASSERT_TRUE(a.getNext(it));
	ASSERT_EQ(ppl7::String("blah"), it.key());
	ASSERT_TRUE(it.value().isString());
	ASSERT_EQ(ppl7::String("blubb"), it.value().toString());

	ASSERT_TRUE(a.getNext(it));
	ASSERT_EQ(ppl7::String("bytearray"), it.key());
	ASSERT_TRUE(it.value().isByteArray());

	ASSERT_TRUE(a.getNext(it));
	ASSERT_EQ(ppl7::String("bytearrayptr"), it.key());
	ASSERT_TRUE(it.value().isByteArrayPtr());

	ASSERT_TRUE(a.getNext(it));
	ASSERT_EQ(ppl7::String("key1"), it.key());
	ASSERT_TRUE(it.value().isString());

	ASSERT_TRUE(a.getNext(it));
	ASSERT_EQ(ppl7::String("key2"), it.key());
	ASSERT_TRUE(it.value().isString());

	ASSERT_TRUE(a.getNext(it));
	ASSERT_EQ(ppl7::String("key3"), it.key());
	ASSERT_TRUE(it.value().isString());

	ASSERT_TRUE(a.getNext(it));
	ASSERT_EQ(ppl7::String("time"), it.key());
	ASSERT_TRUE(it.value().isDateTime());

	ASSERT_FALSE(a.getNext(it));
}

TEST_F(AssocArrayTest, IterateResetGetNextWithDatatypeString) {
	ppl7::AssocArray a;
	createWalkingArray(a);
	ppl7::AssocArray::Iterator it;
	a.reset(it);
	ASSERT_TRUE(a.getNext(it, ppl7::Variant::TYPE_STRING));
	ASSERT_EQ(ppl7::String("aaaa"), it.key());
	ASSERT_TRUE(it.value().isString());
	ASSERT_EQ(ppl7::String("first element"), it.value().toString());
}

TEST_F(AssocArrayTest, IterateGetFirstGetNextWithDatatypeString) {
	ppl7::AssocArray a;
	createWalkingArray(a);
	ppl7::AssocArray::Iterator it;

	ASSERT_TRUE(a.getFirst(it, ppl7::Variant::TYPE_STRING));
	ASSERT_EQ(ppl7::String("aaaa"), it.key());
	ASSERT_TRUE(it.value().isString());
	ASSERT_EQ(ppl7::String("first element"), it.value().toString());

	ASSERT_TRUE(a.getNext(it, ppl7::Variant::TYPE_STRING));
	ASSERT_EQ(ppl7::String("blah"), it.key());
	ASSERT_TRUE(it.value().isString());
	ASSERT_EQ(ppl7::String("blubb"), it.value().toString());

	ASSERT_TRUE(a.getNext(it, ppl7::Variant::TYPE_STRING));
	ASSERT_EQ(ppl7::String("key1"), it.key());
	ASSERT_TRUE(it.value().isString());

	ASSERT_TRUE(a.getNext(it, ppl7::Variant::TYPE_STRING));
	ASSERT_EQ(ppl7::String("key2"), it.key());
	ASSERT_TRUE(it.value().isString());

	ASSERT_TRUE(a.getNext(it, ppl7::Variant::TYPE_STRING));
	ASSERT_EQ(ppl7::String("key3"), it.key());
	ASSERT_TRUE(it.value().isString());

	ASSERT_FALSE(a.getNext(it, ppl7::Variant::TYPE_STRING));
}

TEST_F(AssocArrayTest, IterateResetGetNextWithKeyValueParams) {
	ppl7::AssocArray a;
	createWalkingArray(a);
	ppl7::AssocArray::Iterator it;
	ppl7::String key, value;
	a.reset(it);
	ASSERT_TRUE(a.getNext(it, key, value));
	ASSERT_EQ(ppl7::String("aaaa"), key);
	ASSERT_EQ(ppl7::String("first element"), value);
}

TEST_F(AssocArrayTest, IterateGetFirstGetNextWithKeyValueParams) {
	ppl7::AssocArray a;
	createWalkingArray(a);
	ppl7::AssocArray::Iterator it;
	ppl7::String key, value;
	ASSERT_TRUE(a.getFirst(it, key, value));
	ASSERT_EQ(ppl7::String("aaaa"), key);
	ASSERT_EQ(ppl7::String("first element"), value);

	ASSERT_TRUE(a.getNext(it, key, value));
	ASSERT_EQ(ppl7::String("blah"), key);
	ASSERT_EQ(ppl7::String("blubb"), value);

	ASSERT_TRUE(a.getNext(it, key, value));
	ASSERT_EQ(ppl7::String("key1"), it.key());
	ASSERT_EQ(ppl7::String("Dieser Wert geht über\nmehrere Zeilen"), value);

	ASSERT_TRUE(a.getNext(it, key, value));
	ASSERT_EQ(ppl7::String("key2"), it.key());
	ASSERT_EQ(ppl7::String("value6"), value);

	ASSERT_TRUE(a.getNext(it, key, value));
	ASSERT_EQ(ppl7::String("key3"), it.key());
	ASSERT_EQ(ppl7::String("value7"), value);

	ASSERT_FALSE(a.getNext(it, key, value));
}


TEST_F(AssocArrayTest, IterateResetGetPreviousWithoutDatatype) {
	ppl7::AssocArray a;
	createWalkingArray(a);
	ppl7::AssocArray::ReverseIterator it;
	a.reset(it);
	ASSERT_TRUE(a.getPrevious(it));
	ASSERT_EQ(ppl7::String("time"), it.key());
	ASSERT_TRUE(it.value().isDateTime());
}

TEST_F(AssocArrayTest, IterateGetLastGetPreviousWithoutDatatype) {
	ppl7::AssocArray a;
	createWalkingArray(a);
	ppl7::AssocArray::ReverseIterator it;

	ASSERT_TRUE(a.getLast(it));
	ASSERT_EQ(ppl7::String("time"), it.key());
	ASSERT_TRUE(it.value().isDateTime());

	ASSERT_TRUE(a.getPrevious(it));
	ASSERT_EQ(ppl7::String("key3"), it.key());
	ASSERT_TRUE(it.value().isString());

	ASSERT_TRUE(a.getPrevious(it));
	ASSERT_EQ(ppl7::String("key2"), it.key());
	ASSERT_TRUE(it.value().isString());

	ASSERT_TRUE(a.getPrevious(it));
	ASSERT_EQ(ppl7::String("key1"), it.key());
	ASSERT_TRUE(it.value().isString());

	ASSERT_TRUE(a.getPrevious(it));
	ASSERT_EQ(ppl7::String("bytearrayptr"), it.key());
	ASSERT_TRUE(it.value().isByteArrayPtr());

	ASSERT_TRUE(a.getPrevious(it));
	ASSERT_EQ(ppl7::String("bytearray"), it.key());
	ASSERT_TRUE(it.value().isByteArray());

	ASSERT_TRUE(a.getPrevious(it));
	ASSERT_EQ(ppl7::String("blah"), it.key());
	ASSERT_TRUE(it.value().isString());
	ASSERT_EQ(ppl7::String("blubb"), it.value().toString());

	ASSERT_TRUE(a.getPrevious(it));
	ASSERT_EQ(ppl7::String("array2"), it.key());
	ASSERT_TRUE(it.value().isAssocArray());

	ASSERT_TRUE(a.getPrevious(it));
	ASSERT_EQ(ppl7::String("array1"), it.key());
	ASSERT_TRUE(it.value().isAssocArray());

	ASSERT_TRUE(a.getPrevious(it));
	ASSERT_EQ(ppl7::String("array0"), it.key());
	ASSERT_TRUE(it.value().isArray());

	ASSERT_TRUE(a.getPrevious(it));
	ASSERT_EQ(ppl7::String("aaaa"), it.key());
	ASSERT_TRUE(it.value().isString());


	ASSERT_FALSE(a.getPrevious(it));

}

TEST_F(AssocArrayTest, IterateResetGetPreviousWithDatatypeString) {
	ppl7::AssocArray a;
	createWalkingArray(a);
	ppl7::AssocArray::ReverseIterator it;
	a.reset(it);
	ASSERT_TRUE(a.getPrevious(it, ppl7::Variant::TYPE_STRING));
	ASSERT_EQ(ppl7::String("key3"), it.key());
	ASSERT_TRUE(it.value().isString());
}

TEST_F(AssocArrayTest, IterateGetLastGetPreviousWithDatatypeString) {
	ppl7::AssocArray a;
	createWalkingArray(a);
	ppl7::AssocArray::ReverseIterator it;


	ASSERT_TRUE(a.getLast(it, ppl7::Variant::TYPE_STRING));
	ASSERT_EQ(ppl7::String("key3"), it.key());
	ASSERT_TRUE(it.value().isString());

	ASSERT_TRUE(a.getPrevious(it, ppl7::Variant::TYPE_STRING));
	ASSERT_EQ(ppl7::String("key2"), it.key());
	ASSERT_TRUE(it.value().isString());

	ASSERT_TRUE(a.getPrevious(it, ppl7::Variant::TYPE_STRING));
	ASSERT_EQ(ppl7::String("key1"), it.key());
	ASSERT_TRUE(it.value().isString());

	ASSERT_TRUE(a.getPrevious(it, ppl7::Variant::TYPE_STRING));
	ASSERT_EQ(ppl7::String("blah"), it.key());
	ASSERT_TRUE(it.value().isString());
	ASSERT_EQ(ppl7::String("blubb"), it.value().toString());

	ASSERT_TRUE(a.getPrevious(it, ppl7::Variant::TYPE_STRING));
	ASSERT_EQ(ppl7::String("aaaa"), it.key());
	ASSERT_TRUE(it.value().isString());
	ASSERT_EQ(ppl7::String("first element"), it.value().toString());

	ASSERT_FALSE(a.getPrevious(it, ppl7::Variant::TYPE_STRING));
}

TEST_F(AssocArrayTest, IterateResetGetPreviousWithKeyValueParams) {
	ppl7::AssocArray a;
	createWalkingArray(a);
	ppl7::AssocArray::ReverseIterator it;
	ppl7::String key, value;

	a.reset(it);
	ASSERT_TRUE(a.getPrevious(it, key, value));
	ASSERT_EQ(ppl7::String("key3"), it.key());
	ASSERT_EQ(ppl7::String("value7"), value);
}

TEST_F(AssocArrayTest, IterateGetLastGetPreviousWithKeyValueParams) {
	ppl7::AssocArray a;
	createWalkingArray(a);
	ppl7::AssocArray::ReverseIterator it;
	ppl7::String key, value;
	ASSERT_TRUE(a.getLast(it, key, value));
	ASSERT_EQ(ppl7::String("key3"), it.key());
	ASSERT_EQ(ppl7::String("value7"), value);

	ASSERT_TRUE(a.getPrevious(it, key, value));
	ASSERT_EQ(ppl7::String("key2"), it.key());
	ASSERT_EQ(ppl7::String("value6"), value);

	ASSERT_TRUE(a.getPrevious(it, key, value));
	ASSERT_EQ(ppl7::String("key1"), it.key());
	ASSERT_EQ(ppl7::String("Dieser Wert geht über\nmehrere Zeilen"), value);

	ASSERT_TRUE(a.getPrevious(it, key, value));
	ASSERT_EQ(ppl7::String("blah"), key);
	ASSERT_EQ(ppl7::String("blubb"), value);

	ASSERT_TRUE(a.getPrevious(it, key, value));
	ASSERT_EQ(ppl7::String("aaaa"), key);
	ASSERT_EQ(ppl7::String("first element"), value);

	ASSERT_FALSE(a.getPrevious(it, key, value));
}

TEST_F(AssocArrayTest, CountNonRecursive)
{
	ppl7::AssocArray a1;
	ASSERT_EQ((size_t)0, a1.count(false));
	a1.set("key1", "value1");
	ASSERT_EQ((size_t)1, a1.count(false));
	a1.set("array1/key1", "value2");
	ASSERT_EQ((size_t)2, a1.count(false));
	a1.set("array1/key2", "value3");
	ASSERT_EQ((size_t)2, a1.count(false));
	a1.set("array2/key1", "value3a");
	ASSERT_EQ((size_t)3, a1.count(false));
	a1.set("key2", "value4");
	ASSERT_EQ((size_t)4, a1.count(false));
	a1.set("key3", "value5");
	ASSERT_EQ((size_t)5, a1.count(false));

	ASSERT_EQ((size_t)5, a1.size());
}

TEST_F(AssocArrayTest, CountRecursive)
{
	ppl7::AssocArray a1;
	ASSERT_EQ((size_t)0, a1.count(true));
	a1.set("key1", "value1");
	ASSERT_EQ((size_t)1, a1.count(true));
	a1.set("array1/key1", "value2");
	ASSERT_EQ((size_t)3, a1.count(true));
	a1.set("array1/key2", "value3");
	ASSERT_EQ((size_t)4, a1.count(true));
	a1.set("array2/key1", "value3a");
	ASSERT_EQ((size_t)6, a1.count(true));
	a1.set("key2", "value4");
	ASSERT_EQ((size_t)7, a1.count(true));
	a1.set("key3", "value5");
	ASSERT_EQ((size_t)8, a1.count(true));

	ASSERT_EQ((size_t)5, a1.size());
}

TEST_F(AssocArrayTest, OperatorPlus)
{
	ppl7::AssocArray a1, a2;
	a1.set("key1", "value1");
	a1.set("array1/key1", "value2");
	a1.set("array1/key2", "value3");
	a1.set("array2/key1", "value3a");
	a1.set("key2", "value4");
	a1.set("key3", "value5");
	ASSERT_EQ((size_t)5, a1.count(false));
	ASSERT_EQ((size_t)8, a1.count(true));

	a2.set("key3", "value6");
	a2.set("array1/key3", "value7");
	a2.set("array2/key1", "value8");
	a2.set("array2/key2", "value8a");
	a2.set("array2/key3", "value8b");
	a2.set("array3/key1", "value9");
	a2.set("key4", "value10");
	ASSERT_EQ((size_t)5, a2.count(false));
	ASSERT_EQ((size_t)10, a2.count(true));

	ppl7::AssocArray ret=a1 + a2;
	//ret.list();

	ASSERT_EQ((size_t)7, ret.size());
	ASSERT_EQ((size_t)14, ret.count(true));


	ASSERT_EQ(ppl7::String("value1"), ret.getString("key1"));
	ASSERT_EQ(ppl7::String("value4"), ret.getString("key2"));
	ASSERT_EQ(ppl7::String("value6"), ret.getString("key3"));
	ASSERT_EQ(ppl7::String("value10"), ret.getString("key4"));

	//ASSERT_THROW({ ret.getString("array1/key1"); }, ppl7::KeyNotFoundException);
	ASSERT_EQ(ppl7::String("value7"), ret.getString("array1/key3"));
	ASSERT_EQ(ppl7::String("value8"), ret.getString("array2/key1"));
	ASSERT_EQ(ppl7::String("value9"), ret.getString("array3/key1"));


}

TEST_F(AssocArrayTest, OperatorPlusEqual)
{
	ppl7::AssocArray a1, a2;
	a1.set("key1", "value1");
	a1.set("array1/key1", "value2");
	a1.set("array1/key2", "value3");
	a1.set("array2/key1", "value3a");
	a1.set("key2", "value4");
	a1.set("key3", "value5");


	a2.set("key3", "value6");
	a2.set("array1/key3", "value7");
	a2.set("array2/key1", "value8");
	a2.set("array2/key2", "value8a");
	a2.set("array2/key3", "value8b");
	a2.set("array3/key1", "value9");
	a2.set("key4", "value10");

	a1+=a2;

	ASSERT_EQ((size_t)14, a1.count(true));
	ASSERT_EQ((size_t)7, a1.size());

	ASSERT_EQ(ppl7::String("value1"), a1.getString("key1"));
	ASSERT_EQ(ppl7::String("value4"), a1.getString("key2"));
	ASSERT_EQ(ppl7::String("value6"), a1.getString("key3"));
	ASSERT_EQ(ppl7::String("value10"), a1.getString("key4"));

	//ASSERT_THROW({ a1.getString("array1/key1"); }, ppl7::KeyNotFoundException);
	ASSERT_EQ(ppl7::String("value7"), a1.getString("array1/key3"));
	ASSERT_EQ(ppl7::String("value8"), a1.getString("array2/key1"));
	ASSERT_EQ(ppl7::String("value9"), a1.getString("array3/key1"));
}

TEST_F(AssocArrayTest, OperatorEqualEqual)
{
	ppl7::AssocArray a1, a2;
	a1.set("key1", "value1");
	a1.set("array1/key1", "value2");
	a1.set("array1/key2", "value3");
	a1.set("array2/key1", "value3a");
	a1.set("key2", "value4");
	a1.set("key3", "value5");

	a2.set("key1", "value1");
	a2.set("array1/key1", "value2");
	a2.set("array1/key2", "value3");
	a2.set("array2/key1", "value3a");
	a2.set("key2", "value4");
	a2.set("key3", "value5");

	ASSERT_TRUE(a1 == a2);
	ASSERT_FALSE(a1 != a2);
}

TEST_F(AssocArrayTest, OperatorNotEqual)
{
	ppl7::AssocArray a1, a2;
	a1.set("key1", "value1");
	a1.set("array1/key1", "value2");
	a1.set("array1/key2", "value3");
	a1.set("array2/key1", "value3a");
	a1.set("key2", "value4");
	a1.set("key3", "value5");

	a2.set("key1", "value1");
	a2.set("array1/key1", "value7");
	a2.set("array1/key2", "value3");
	a2.set("array2/key1", "value3a");
	a2.set("key2", "value4");
	a2.set("key3", "value5");

	ASSERT_TRUE(a1 != a2);
	ASSERT_FALSE(a1 == a2);
}

TEST_F(AssocArrayTest, OperatorElement)
{
	ppl7::AssocArray a1, a2;
	a1.set("key1", "value1");
	a1.set("array1/key1", "value2");
	a1.set("array1/key2", "value3");
	a1.set("array2/key1", "value3a");
	a1.set("key2", "value4");
	a1.set("key3", "value5");

	ASSERT_EQ(ppl7::Variant(ppl7::String("value2")), a1["array1/key1"]);
	ASSERT_EQ(ppl7::Variant(ppl7::String("value4")), a1["key2"]);

}

TEST_F(AssocArrayTest, ToTemplate)
{
	ppl7::AssocArray a;
	ppl7::ByteArray bin;
	ppl7::String out;
	ppl7::File::load(bin, "src/main.cpp");
	a.set("key1", "Dieser Wert geht über\nmehrere Zeilen");
	a.set("array1/unterkey1", "value2");
	a.set("array1/unterkey2", "value3");
	a.set("array1/noch ein array/unterkey1", "value4");
	a.set("array1/unterkey2", "value5");
	a.set("key2", "value6");
	a.set("dateien/main.cpp", &bin);
	a.set("array2/unterkey1", "value7");
	a.set("array2/unterkey2", "value8");
	a.set("array2/unterkey1", "value9");
	a.toTemplate(out, "foo");
	//out.printnl();
	ASSERT_EQ(ppl7::String(
		"foo/array1/noch ein array/unterkey1=value4\n"
		"foo/array1/unterkey1=value2\n"
		"foo/array1/unterkey2=value5\n"
		"foo/array2/unterkey1=value9\n"
		"foo/array2/unterkey2=value8\n"
		"foo/key1=Dieser Wert geht über\n"
		"foo/key1=mehrere Zeilen\n"
		"foo/key2=value6\n"
	), out);
}




}	// EOF namespace
