/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 421 $
 * $Date: 2012-06-07 11:29:25 +0200 (Do, 07. Jun 2012) $
 * $Id: array.cpp 421 2012-06-07 09:29:25Z pafe $
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
		ASSERT_EQ(ppl7::String("value1"),conf.get("key1").toString());
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.nextSection());
		ASSERT_EQ(ppl7::String("section1"),conf.getSectionName());
		ASSERT_EQ(ppl7::String("another value1"),conf.get("key1").toString());
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.nextSection());
		ASSERT_EQ(ppl7::String("foo=[bar]"),conf.getSectionName());
		ASSERT_EQ(ppl7::String("value 1"),conf.get("key 1").toString());
		ASSERT_EQ(ppl7::String(""),conf.get("key1").toString());
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.nextSection());
		ASSERT_EQ(ppl7::String("empty section"),conf.getSectionName());
		ASSERT_EQ(ppl7::String(""),conf.get("key1").toString());
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(1,conf.nextSection());
		ASSERT_EQ(ppl7::String("last section"),conf.getSectionName());
		ASSERT_EQ(ppl7::String("true"),conf.get("final").toString());
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
	ASSERT_EQ((size_t)3,s.size());
	ASSERT_EQ(ppl7::String("value 1"),s.get("key 1").toString());
	ASSERT_EQ(ppl7::String("value 2"),s.get("key 2").toString());
	ASSERT_EQ(ppl7::String("value 3"),s.get("key 3").toString());

}

TEST_F(ConfigParserTest, trimmedValue) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_EQ(ppl7::String("value3"),conf.get("key3").toString());

}

TEST_F(ConfigParserTest, singleKeyFirstSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_EQ(ppl7::String("value1"),conf.get("key1").toString());

}

TEST_F(ConfigParserTest, duplicateKey) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("global");
	});
	ASSERT_EQ(ppl7::String("First line\nsecond line\nthird line"),conf.get("key4").toString());

}

TEST_F(ConfigParserTest, singleKeySecondSection) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_EQ(ppl7::String("another value1"),conf.get("key1").toString());

}

TEST_F(ConfigParserTest, spacesInKeyName) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_EQ(ppl7::String("42"),conf.get("answer to all questions").toString());
}

TEST_F(ConfigParserTest, nonExistentKeyWithoutDefaultValueString) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_EQ(ppl7::String(""),conf.get("unknown key").toString());
}

TEST_F(ConfigParserTest, nonExistentKeyWithDefaultValueString) {
	ppl7::ConfigParser conf;
	ASSERT_NO_THROW({
			conf.load("testdata/example.conf");
			conf.selectSection("section1");
	});
	ASSERT_EQ(ppl7::String("default value"),conf.get("unknown key","default value").toString());
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

	ASSERT_EQ(false,conf.getNext(key,value));


}


}
