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

namespace {

// The fixture for testing class Foo.
class ConfigParserTest : public ::testing::Test {
	protected:
	ConfigParserTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~ConfigParserTest() {

	}
};


TEST_F(ConfigParserTest, loadFromFile) {
	ASSERT_NO_THROW({
		ppl7::ConfigParser conf;
		conf.load("testdata/example.conf");
	}
	);
}

TEST_F(ConfigParserTest, selectExistingSection_global) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
	});

	ASSERT_NO_THROW({
		conf.selectSection("global");
	});
}

TEST_F(ConfigParserTest, selectExistingSectionCaseInsensitiv_global) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
	});

	ASSERT_NO_THROW({
		conf.selectSection("GloBal");
	});
}

TEST_F(ConfigParserTest, selectExistingSection_section1) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
	});

	ASSERT_NO_THROW({
		conf.selectSection("section1");
	});
}

TEST_F(ConfigParserTest, selectExistingSection_foobar) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
	});

	ASSERT_NO_THROW({
		conf.selectSection("foo=[bar]");
	});
}

TEST_F(ConfigParserTest, selectExistingSection_emptysection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
	});

	ASSERT_NO_THROW({
		conf.selectSection("empty section");
	});
}

TEST_F(ConfigParserTest, selectExistingSection_lastsection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
	});

	ASSERT_NO_THROW({
		conf.selectSection("last section");
	});
}

TEST_F(ConfigParserTest, selectNonExistingSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
	});

	ASSERT_THROW({
		conf.selectSection("unknown");
	},ppl7::UnknownSectionException);
}

TEST_F(ConfigParserTest, getSectionName) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("Global");
	});
	ppl7::String s;
	ASSERT_NO_THROW({
		s=conf.getSectionName();
	});
	ASSERT_EQ(ppl7::String("global"),s);
}

TEST_F(ConfigParserTest, getSection) {
	ppl7::ConfigParser conf;
	ppl7::String s;
	ppl7::String expected="# This is the first section of the configuration file with key-value pairs.\n"
		"# Comments like this are ignored.\n"
		"key1=value1\n"
		"key2 = value2\n"
		"# Spaces before and after \"=\" don't matter \n"
		"key3   =      value3\n"
		"key7= 321\n"
		"\n"
		"# values with identic keys should get concatenated together with newline\n"
		"key4 = First line\n"
		"key4 = second line\n"
		"key4 = third line\n\n";
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			s=conf.getSection("Global");
	});
	//expected.hexDump();
	//s.hexDump();
	ASSERT_EQ(expected,s);
}

TEST_F(ConfigParserTest, getEmptySection) {
	ppl7::ConfigParser conf;
	ppl7::String s;
	ppl7::String expected="";
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			s=conf.getSection("empty section");
	});
	ASSERT_EQ(expected,s);
}

TEST_F(ConfigParserTest, getLastSection) {
	ppl7::ConfigParser conf;
	ppl7::String s;
	ppl7::String expected="final=true\n\n\n";
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			s=conf.getSection("last section");
	});
	ASSERT_EQ(expected,s);
}

TEST_F(ConfigParserTest, iterate) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.firstSection());
		ASSERT_EQ(ppl7::String("global"),conf.getSectionName());
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.nextSection());
		ASSERT_EQ(ppl7::String("section1"),conf.getSectionName());
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.nextSection());
		ASSERT_EQ(ppl7::String("foo=[bar]"),conf.getSectionName());
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.nextSection());
		ASSERT_EQ(ppl7::String("case_insensitive_test"),conf.getSectionName());
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.nextSection());
		ASSERT_EQ(ppl7::String("empty section"),conf.getSectionName());
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.nextSection());
		ASSERT_EQ(ppl7::String("last section"),conf.getSectionName());
	});
	ASSERT_NO_THROW({
			ASSERT_EQ(0,conf.nextSection());
	});
	ASSERT_THROW({conf.getSectionName();},ppl7::NoSectionSelectedException);
}

TEST_F(ConfigParserTest, iterateAndCheckValue) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.firstSection());
		ASSERT_EQ(ppl7::String("global"),conf.getSectionName());
		ASSERT_EQ(ppl7::String("value1"),conf.get("key1"));
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.nextSection());
		ASSERT_EQ(ppl7::String("section1"),conf.getSectionName());
		ASSERT_EQ(ppl7::String("another value1"),conf.get("key1"));
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.nextSection());
		ASSERT_EQ(ppl7::String("foo=[bar]"),conf.getSectionName());
		ASSERT_EQ(ppl7::String("value 1"),conf.get("key 1"));
		ASSERT_EQ(ppl7::String(""),conf.get("key1"));
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.nextSection());
		ASSERT_EQ(ppl7::String("case_insensitive_test"),conf.getSectionName());
		ASSERT_EQ(ppl7::String(""),conf.get("key1"));
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.nextSection());
		ASSERT_EQ(ppl7::String("empty section"),conf.getSectionName());
		ASSERT_EQ(ppl7::String(""),conf.get("key1"));
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.nextSection());
		ASSERT_EQ(ppl7::String("last section"),conf.getSectionName());
		ASSERT_EQ(ppl7::String("true"),conf.get("final"));
	});
	ASSERT_NO_THROW({
			ASSERT_EQ(0,conf.nextSection());
	});
	ASSERT_THROW({conf.getSectionName();},ppl7::NoSectionSelectedException);
}

TEST_F(ConfigParserTest, copySection) {
	ppl7::ConfigParser conf;
	ppl7::AssocArray s;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.copySection(s,"foo=[bar]");
	});
	ASSERT_EQ((size_t)4,s.size());
	ASSERT_EQ(ppl7::String("value 1"),s.get("key 1").toString());
	ASSERT_EQ(ppl7::String("value 2"),s.get("key 2").toString());
	ASSERT_EQ(ppl7::String("value 3"),s.get("key 3").toString());
	ASSERT_EQ(ppl7::String("yes"),s.get("key4").toString());

}

TEST_F(ConfigParserTest, trimmedValue) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_EQ(ppl7::String("value3"),conf.get("key3"));

}

TEST_F(ConfigParserTest, getSingleKeyFirstSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_EQ(ppl7::String("value1"),conf.get("key1"));

}

TEST_F(ConfigParserTest, getKeyCaseInsensitive) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_EQ(ppl7::String("value1"),conf.get("KeY1"));

}

TEST_F(ConfigParserTest, getDuplicateKey) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_EQ(ppl7::String("First line\nsecond line\nthird line"),conf.get("key4"));

}

TEST_F(ConfigParserTest, getSingleKeySecondSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_EQ(ppl7::String("another value1"),conf.get("key1"));

}

TEST_F(ConfigParserTest, spacesInKeyName) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_EQ(ppl7::String("42"),conf.get("answer to all questions"));
}

TEST_F(ConfigParserTest, nonExistentKeyWithoutDefaultValueString) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_EQ(ppl7::String(""),conf.get("unknown key"));
}

TEST_F(ConfigParserTest, nonExistentKeyWithDefaultValueString) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_EQ(ppl7::String("default value"),conf.get("unknown key","default value"));
}

TEST_F(ConfigParserTest, nonExistentKeyWithoutDefaultValueBool) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_EQ(false,conf.getBool("unknown key"));
}

TEST_F(ConfigParserTest, nonExistentKeyWithDefaultValueBool) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_EQ(true,conf.getBool("unknown key",true));
}

TEST_F(ConfigParserTest, nonExistentKeyWithoutDefaultValueInteger) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_EQ(0,conf.getInt("unknown key"));
}

TEST_F(ConfigParserTest, nonExistentKeyWithDefaultValueInteger) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_EQ(42,conf.getInt("unknown key",42));
}


TEST_F(ConfigParserTest, iterateThruSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
			conf.reset();
	});
	ppl7::String key,value;

	ASSERT_EQ(true,conf.getNext(key,value));
	ASSERT_EQ(ppl7::String("answer to all questions"),key);
	ASSERT_EQ(ppl7::String("42"),value);

	ASSERT_EQ(true,conf.getNext(key,value));
	ASSERT_EQ(ppl7::String("key1"),key);
	ASSERT_EQ(ppl7::String("another value1"),value);

	ASSERT_EQ(true,conf.getNext(key,value));
	ASSERT_EQ(ppl7::String("key2"),key);
	ASSERT_EQ(ppl7::String("another value2"),value);

	ASSERT_EQ(true,conf.getNext(key,value));
	ASSERT_EQ(ppl7::String("key3"),key);
	ASSERT_EQ(ppl7::String("yes"),value);

	ASSERT_EQ(true,conf.getNext(key,value));
	ASSERT_EQ(ppl7::String("key4"),key);
	ASSERT_EQ(ppl7::String("no"),value);

	ASSERT_EQ(true,conf.getNext(key,value));
	ASSERT_EQ(ppl7::String("key7"),key);
	ASSERT_EQ(ppl7::String("123"),value);

	ASSERT_EQ(false,conf.getNext(key,value));
}

TEST_F(ConfigParserTest, getFromSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_EQ(ppl7::String("another value1"),conf.getFromSection("section1","key1"));
}

TEST_F(ConfigParserTest, getBoolFromSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_EQ(true,conf.getBoolFromSection("foo=[bar]","key4"));
}

TEST_F(ConfigParserTest, getIntFromSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_EQ(123,conf.getIntFromSection("section1","key7"));
}


TEST_F(ConfigParserTest, deleteSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_NO_THROW({
		conf.deleteSection("section1");
	});

	// Auf die Section darf kein Zugriff mehr erfolgen
	ASSERT_THROW({conf.getSectionName();},ppl7::NoSectionSelectedException);
	ASSERT_THROW({conf.get("key1");},ppl7::NoSectionSelectedException);
	ASSERT_THROW({conf.add("key9","value9");},ppl7::NoSectionSelectedException);
}

TEST_F(ConfigParserTest, unload) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_NO_THROW({
		conf.unload();
	});

	// Auf die Section darf kein Zugriff mehr erfolgen
	ASSERT_THROW({conf.getSectionName();},ppl7::NoSectionSelectedException);
	ASSERT_THROW({conf.get("key1");},ppl7::NoSectionSelectedException);
	ASSERT_THROW({conf.add("key9","value9");},ppl7::NoSectionSelectedException);
}

TEST_F(ConfigParserTest, setAndGetSeparator) {
	ppl7::ConfigParser conf;
	ASSERT_EQ(ppl7::String("="),conf.getSeparator());
	ASSERT_NO_THROW({
		conf.setSeparator("");
	});
	ASSERT_EQ(ppl7::String("="),conf.getSeparator());
	ASSERT_NO_THROW({
		conf.setSeparator(":");
	});
	ASSERT_EQ(ppl7::String(":"),conf.getSeparator());
}

TEST_F(ConfigParserTest, addWithoutPreviousSectionSelection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
	});
	ASSERT_THROW({conf.add("key9","value9");},ppl7::NoSectionSelectedException);
}

TEST_F(ConfigParserTest, addAndGetString) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_NO_THROW({
		conf.add("key9","value9");
		//conf.print();
		ASSERT_EQ(ppl7::String("value9"),conf.get("key9"));
	});
}

TEST_F(ConfigParserTest, addAndGetStringOtherSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_NO_THROW({
		conf.add("section1","key9","value9");
		//conf.print();
		conf.selectSection("section1");
		ASSERT_EQ(ppl7::String("value9"),conf.get("key9"));
	});
}

TEST_F(ConfigParserTest, addAndGetInteger) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_NO_THROW({
		conf.add("key9",42);
		//conf.print();
		ASSERT_EQ(ppl7::String("42"),conf.get("key9"));
	});
}

TEST_F(ConfigParserTest, addAndGetIntegerOtherSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_NO_THROW({
		conf.add("section1","key9",42);
		//conf.print();
		conf.selectSection("section1");
		ASSERT_EQ(42,conf.getInt("key9"));
	});
}
TEST_F(ConfigParserTest, addAndGetBool) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_NO_THROW({
		conf.add("key9",true);
		//conf.print();
		ASSERT_EQ(true,conf.getBool("key9"));
	});
}

TEST_F(ConfigParserTest, addAndGetBoolOtherSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_NO_THROW({
		conf.add("section1","key9","value9");
		//conf.print();
		conf.selectSection("section1");
		ASSERT_EQ(ppl7::String("value9"),conf.get("key9"));
	});
}

TEST_F(ConfigParserTest, addToUnknownSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_NO_THROW({
		conf.add("section2","key9","value9");
		conf.selectSection("section2");
		ASSERT_EQ(ppl7::String("value9"),conf.get("key9"));
	});
}

TEST_F(ConfigParserTest, deleteExistingKey) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_NO_THROW({
		conf.deleteKey("key1");
		//conf.print();
		ASSERT_EQ(ppl7::String(""),conf.get("key1"));
	});
}

TEST_F(ConfigParserTest, deleteExistingKeyInExistingSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_NO_THROW({
		conf.deleteKey("section1","key1");
		ASSERT_EQ(ppl7::String(""),conf.getFromSection("section1","key1"));
		ASSERT_EQ(ppl7::String("value1"),conf.get("key1"));
	});
}

TEST_F(ConfigParserTest, deleteNonExistingKey) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_NO_THROW({
		conf.deleteKey("key9");
	});
	//conf.print();
	ASSERT_EQ(ppl7::String(""),conf.get("key9"));

}

TEST_F(ConfigParserTest, deleteNonExistingKeyInExistingSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_NO_THROW({
		conf.deleteKey("section1","key9");
	});
}

TEST_F(ConfigParserTest, deleteNonExistingKeyInNonExistingSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_NO_THROW({
		conf.deleteKey("section7","key9");
	});
}

TEST_F(ConfigParserTest, createSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
	});
	ASSERT_NO_THROW({
		conf.createSection("section2");
		conf.add("key9","value9");
		ASSERT_EQ(ppl7::String("section2"),conf.getSectionName());
		ASSERT_EQ(ppl7::String("value9"),conf.get("key9"));
	});
}

TEST_F(ConfigParserTest, createOnExistingSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
	});
	ASSERT_NO_THROW({
		conf.createSection("section1");
		conf.add("key9","value9");
		ASSERT_EQ(ppl7::String("section1"),conf.getSectionName());
		ASSERT_EQ(ppl7::String("value9"),conf.get("key9"));
	});
}

TEST_F(ConfigParserTest, save) {
	ppl7::String TmpFile=ppl7::Dir::tempPath()+"/ppl7_test_example.conf.saved";
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
	});
	ppl7::String s;
	ppl7::String expected="[global]\n"
"key1=value1\n"
"key2=value2\n"
"key3=value3\n"
"key4=First line\n"
"key4=second line\n"
"key4=third line\n"
"key7=321\n"
"\n"
"[section1]\n"
"answer to all questions=42\n"
"key1=another value1\n"
"key2=another value2\n"
"key3=yes\n"
"key4=no\n"
"key7=123\n"
"\n"
"[foo=[bar]]\n"
"key 1=value 1\n"
"key 2=value 2\n"
"key 3=value 3\n"
"key4=yes\n"
"\n"
"[case_insensitive_test]\n"
"testkey=First line\n"
"testkey=second line\n"
"testkey=third line\n"
"\n"
"[empty section]\n"
"\n"
"[last section]\n"
"final=true\n";

	EXPECT_NO_THROW({
		conf.save(TmpFile);
		//conf.print();
		ppl7::File::load(s,TmpFile);
	});
	EXPECT_EQ(expected,s);

	// Temp-File loeschen
	try {
		ppl7::File::unlink(TmpFile);
	} catch (...) {
		// Do nothing
	}
}

}
