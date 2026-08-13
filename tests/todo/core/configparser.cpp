/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
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
#include <pplib.h>
#include <gtest/gtest.h>
#include "pplib-tests.h"

namespace
{

// The fixture for testing class Foo.
class ConfigParserTest : public ::testing::Test
{
protected:
    ConfigParserTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~ConfigParserTest()
    {
    }
};

TEST_F(ConfigParserTest, loadFromFile)
{
    ASSERT_NO_THROW({
        pplib::ConfigParser conf;
        conf.load("testdata/example.conf");
    });
}

TEST_F(ConfigParserTest, selectExistingSection_global)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({ conf.load("testdata/example.conf"); });

    ASSERT_NO_THROW({ conf.selectSection("global"); });
}

TEST_F(ConfigParserTest, selectExistingSectionCaseInsensitiv_global)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({ conf.load("testdata/example.conf"); });

    ASSERT_NO_THROW({ conf.selectSection("GloBal"); });
}

TEST_F(ConfigParserTest, selectExistingSection_section1)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({ conf.load("testdata/example.conf"); });

    ASSERT_NO_THROW({ conf.selectSection("section1"); });
}

TEST_F(ConfigParserTest, selectExistingSection_foobar)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({ conf.load("testdata/example.conf"); });

    ASSERT_NO_THROW({ conf.selectSection("foo=[bar]"); });
}

TEST_F(ConfigParserTest, selectExistingSection_emptysection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({ conf.load("testdata/example.conf"); });

    ASSERT_NO_THROW({ conf.selectSection("empty section"); });
}

TEST_F(ConfigParserTest, selectExistingSection_lastsection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({ conf.load("testdata/example.conf"); });

    ASSERT_NO_THROW({ conf.selectSection("last section"); });
}

TEST_F(ConfigParserTest, selectNonExistingSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({ conf.load("testdata/example.conf"); });

    ASSERT_THROW({ conf.selectSection("unknown"); }, pplib::UnknownSectionException);
}

TEST_F(ConfigParserTest, getSectionName)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("Global");
    });
    pplib::String s;
    ASSERT_NO_THROW({ s = conf.getSectionName(); });
    ASSERT_EQ(pplib::String("global"), s);
}

TEST_F(ConfigParserTest, getSection)
{
    pplib::ConfigParser conf;
    pplib::String s;
    pplib::String expected = "# This is the first section of the configuration file with key-value pairs.\n"
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
        s = conf.getSection("Global");
    });
    // expected.hexDump();
    // s.hexDump();
    ASSERT_EQ(expected, s);
}

TEST_F(ConfigParserTest, getEmptySection)
{
    pplib::ConfigParser conf;
    pplib::String s;
    pplib::String expected = "";
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        s = conf.getSection("empty section");
    });
    ASSERT_EQ(expected, s);
}

TEST_F(ConfigParserTest, getLastSection)
{
    pplib::ConfigParser conf;
    pplib::String s;
    pplib::String expected = "final=true\n\n\n";
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        s = conf.getSection("last section");
    });
    ASSERT_EQ(expected, s);
}

TEST_F(ConfigParserTest, iterate)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({ conf.load("testdata/example.conf"); });
    ASSERT_NO_THROW({
        ASSERT_EQ(1, conf.firstSection());
        ASSERT_EQ(pplib::String("global"), conf.getSectionName());
    });
    ASSERT_NO_THROW({
        ASSERT_EQ(1, conf.nextSection());
        ASSERT_EQ(pplib::String("section1"), conf.getSectionName());
    });
    ASSERT_NO_THROW({
        ASSERT_EQ(1, conf.nextSection());
        ASSERT_EQ(pplib::String("foo=[bar]"), conf.getSectionName());
    });
    ASSERT_NO_THROW({
        ASSERT_EQ(1, conf.nextSection());
        ASSERT_EQ(pplib::String("case_insensitive_test"), conf.getSectionName());
    });
    ASSERT_NO_THROW({
        ASSERT_EQ(1, conf.nextSection());
        ASSERT_EQ(pplib::String("empty section"), conf.getSectionName());
    });
    ASSERT_NO_THROW({
        ASSERT_EQ(1, conf.nextSection());
        ASSERT_EQ(pplib::String("last section"), conf.getSectionName());
    });
    ASSERT_NO_THROW({ ASSERT_EQ(0, conf.nextSection()); });
    ASSERT_THROW({ conf.getSectionName(); }, pplib::NoSectionSelectedException);
}

TEST_F(ConfigParserTest, iterateAndCheckValue)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({ conf.load("testdata/example.conf"); });
    ASSERT_NO_THROW({
        ASSERT_EQ(1, conf.firstSection());
        ASSERT_EQ(pplib::String("global"), conf.getSectionName());
        ASSERT_EQ(pplib::String("value1"), conf.get("key1"));
    });
    ASSERT_NO_THROW({
        ASSERT_EQ(1, conf.nextSection());
        ASSERT_EQ(pplib::String("section1"), conf.getSectionName());
        ASSERT_EQ(pplib::String("another value1"), conf.get("key1"));
    });
    ASSERT_NO_THROW({
        ASSERT_EQ(1, conf.nextSection());
        ASSERT_EQ(pplib::String("foo=[bar]"), conf.getSectionName());
        ASSERT_EQ(pplib::String("value 1"), conf.get("key 1"));
        ASSERT_EQ(pplib::String(""), conf.get("key1"));
    });
    ASSERT_NO_THROW({
        ASSERT_EQ(1, conf.nextSection());
        ASSERT_EQ(pplib::String("case_insensitive_test"), conf.getSectionName());
        ASSERT_EQ(pplib::String(""), conf.get("key1"));
    });
    ASSERT_NO_THROW({
        ASSERT_EQ(1, conf.nextSection());
        ASSERT_EQ(pplib::String("empty section"), conf.getSectionName());
        ASSERT_EQ(pplib::String(""), conf.get("key1"));
    });
    ASSERT_NO_THROW({
        ASSERT_EQ(1, conf.nextSection());
        ASSERT_EQ(pplib::String("last section"), conf.getSectionName());
        ASSERT_EQ(pplib::String("true"), conf.get("final"));
    });
    ASSERT_NO_THROW({ ASSERT_EQ(0, conf.nextSection()); });
    ASSERT_THROW({ conf.getSectionName(); }, pplib::NoSectionSelectedException);
}

TEST_F(ConfigParserTest, copySection)
{
    pplib::ConfigParser conf;
    pplib::AssocArray s;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.copySection(s, "foo=[bar]");
    });
    ASSERT_EQ((size_t)4, s.size());
    ASSERT_EQ(pplib::String("value 1"), s.get("key 1").toString());
    ASSERT_EQ(pplib::String("value 2"), s.get("key 2").toString());
    ASSERT_EQ(pplib::String("value 3"), s.get("key 3").toString());
    ASSERT_EQ(pplib::String("yes"), s.get("key4").toString());
}

TEST_F(ConfigParserTest, trimmedValue)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("global");
    });
    ASSERT_EQ(pplib::String("value3"), conf.get("key3"));
}

TEST_F(ConfigParserTest, getSingleKeyFirstSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("global");
    });
    ASSERT_EQ(pplib::String("value1"), conf.get("key1"));
}

TEST_F(ConfigParserTest, getKeyCaseInsensitive)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("global");
    });
    ASSERT_EQ(pplib::String("value1"), conf.get("KeY1"));
}

TEST_F(ConfigParserTest, getDuplicateKey)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("global");
    });
    ASSERT_EQ(pplib::String("First line\nsecond line\nthird line"), conf.get("key4"));
}

TEST_F(ConfigParserTest, getSingleKeySecondSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_EQ(pplib::String("another value1"), conf.get("key1"));
}

TEST_F(ConfigParserTest, spacesInKeyName)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_EQ(pplib::String("42"), conf.get("answer to all questions"));
}

TEST_F(ConfigParserTest, nonExistentKeyWithoutDefaultValueString)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_EQ(pplib::String(""), conf.get("unknown key"));
}

TEST_F(ConfigParserTest, nonExistentKeyWithDefaultValueString)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_EQ(pplib::String("default value"), conf.get("unknown key", "default value"));
}

TEST_F(ConfigParserTest, nonExistentKeyWithoutDefaultValueBool)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_EQ(false, conf.getBool("unknown key"));
}

TEST_F(ConfigParserTest, nonExistentKeyWithDefaultValueBool)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_EQ(true, conf.getBool("unknown key", true));
}

TEST_F(ConfigParserTest, nonExistentKeyWithoutDefaultValueInteger)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_EQ(0, conf.getInt("unknown key"));
}

TEST_F(ConfigParserTest, nonExistentKeyWithDefaultValueInteger)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_EQ(42, conf.getInt("unknown key", 42));
}

TEST_F(ConfigParserTest, iterateThruSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
        conf.reset();
    });
    pplib::String key, value;

    ASSERT_EQ(true, conf.getNext(key, value));
    ASSERT_EQ(pplib::String("answer to all questions"), key);
    ASSERT_EQ(pplib::String("42"), value);

    ASSERT_EQ(true, conf.getNext(key, value));
    ASSERT_EQ(pplib::String("key1"), key);
    ASSERT_EQ(pplib::String("another value1"), value);

    ASSERT_EQ(true, conf.getNext(key, value));
    ASSERT_EQ(pplib::String("key2"), key);
    ASSERT_EQ(pplib::String("another value2"), value);

    ASSERT_EQ(true, conf.getNext(key, value));
    ASSERT_EQ(pplib::String("key3"), key);
    ASSERT_EQ(pplib::String("yes"), value);

    ASSERT_EQ(true, conf.getNext(key, value));
    ASSERT_EQ(pplib::String("key4"), key);
    ASSERT_EQ(pplib::String("no"), value);

    ASSERT_EQ(true, conf.getNext(key, value));
    ASSERT_EQ(pplib::String("key7"), key);
    ASSERT_EQ(pplib::String("123"), value);

    ASSERT_EQ(false, conf.getNext(key, value));
}

TEST_F(ConfigParserTest, getFromSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("global");
    });
    ASSERT_EQ(pplib::String("another value1"), conf.getFromSection("section1", "key1"));
}

TEST_F(ConfigParserTest, getBoolFromSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_EQ(true, conf.getBoolFromSection("foo=[bar]", "key4"));
}

TEST_F(ConfigParserTest, getIntFromSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("global");
    });
    ASSERT_EQ(123, conf.getIntFromSection("section1", "key7"));
}

TEST_F(ConfigParserTest, deleteSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_NO_THROW({ conf.deleteSection("section1"); });

    // Auf die Section darf kein Zugriff mehr erfolgen
    ASSERT_THROW({ conf.getSectionName(); }, pplib::NoSectionSelectedException);
    ASSERT_THROW({ conf.get("key1"); }, pplib::NoSectionSelectedException);
    ASSERT_THROW({ conf.add("key9", "value9"); }, pplib::NoSectionSelectedException);
}

TEST_F(ConfigParserTest, unload)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_NO_THROW({ conf.unload(); });

    // Auf die Section darf kein Zugriff mehr erfolgen
    ASSERT_THROW({ conf.getSectionName(); }, pplib::NoSectionSelectedException);
    ASSERT_THROW({ conf.get("key1"); }, pplib::NoSectionSelectedException);
    ASSERT_THROW({ conf.add("key9", "value9"); }, pplib::NoSectionSelectedException);
}

TEST_F(ConfigParserTest, setAndGetSeparator)
{
    pplib::ConfigParser conf;
    ASSERT_EQ(pplib::String("="), conf.getSeparator());
    ASSERT_NO_THROW({ conf.setSeparator(""); });
    ASSERT_EQ(pplib::String("="), conf.getSeparator());
    ASSERT_NO_THROW({ conf.setSeparator(":"); });
    ASSERT_EQ(pplib::String(":"), conf.getSeparator());
}

TEST_F(ConfigParserTest, addWithoutPreviousSectionSelection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({ conf.load("testdata/example.conf"); });
    ASSERT_THROW({ conf.add("key9", "value9"); }, pplib::NoSectionSelectedException);
}

TEST_F(ConfigParserTest, addAndGetString)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_NO_THROW({
        conf.add("key9", "value9");
        // conf.print();
        ASSERT_EQ(pplib::String("value9"), conf.get("key9"));
    });
}

TEST_F(ConfigParserTest, addAndGetStringOtherSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("global");
    });
    ASSERT_NO_THROW({
        conf.add("section1", "key9", "value9");
        // conf.print();
        conf.selectSection("section1");
        ASSERT_EQ(pplib::String("value9"), conf.get("key9"));
    });
}

TEST_F(ConfigParserTest, addAndGetInteger)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_NO_THROW({
        conf.add("key9", 42);
        // conf.print();
        ASSERT_EQ(pplib::String("42"), conf.get("key9"));
    });
}

TEST_F(ConfigParserTest, addAndGetIntegerOtherSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("global");
    });
    ASSERT_NO_THROW({
        conf.add("section1", "key9", 42);
        // conf.print();
        conf.selectSection("section1");
        ASSERT_EQ(42, conf.getInt("key9"));
    });
}
TEST_F(ConfigParserTest, addAndGetBool)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_NO_THROW({
        conf.add("key9", true);
        // conf.print();
        ASSERT_EQ(true, conf.getBool("key9"));
    });
}

TEST_F(ConfigParserTest, addAndGetBoolOtherSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("global");
    });
    ASSERT_NO_THROW({
        conf.add("section1", "key9", "value9");
        // conf.print();
        conf.selectSection("section1");
        ASSERT_EQ(pplib::String("value9"), conf.get("key9"));
    });
}

TEST_F(ConfigParserTest, addToUnknownSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_NO_THROW({
        conf.add("section2", "key9", "value9");
        conf.selectSection("section2");
        ASSERT_EQ(pplib::String("value9"), conf.get("key9"));
    });
}

TEST_F(ConfigParserTest, deleteExistingKey)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_NO_THROW({
        conf.deleteKey("key1");
        // conf.print();
        ASSERT_EQ(pplib::String(""), conf.get("key1"));
    });
}

TEST_F(ConfigParserTest, deleteExistingKeyInExistingSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("global");
    });
    ASSERT_NO_THROW({
        conf.deleteKey("section1", "key1");
        ASSERT_EQ(pplib::String(""), conf.getFromSection("section1", "key1"));
        ASSERT_EQ(pplib::String("value1"), conf.get("key1"));
    });
}

TEST_F(ConfigParserTest, deleteNonExistingKey)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("section1");
    });
    ASSERT_NO_THROW({ conf.deleteKey("key9"); });
    // conf.print();
    ASSERT_EQ(pplib::String(""), conf.get("key9"));
}

TEST_F(ConfigParserTest, deleteNonExistingKeyInExistingSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("global");
    });
    ASSERT_NO_THROW({ conf.deleteKey("section1", "key9"); });
}

TEST_F(ConfigParserTest, deleteNonExistingKeyInNonExistingSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({
        conf.load("testdata/example.conf");
        conf.selectSection("global");
    });
    ASSERT_NO_THROW({ conf.deleteKey("section7", "key9"); });
}

TEST_F(ConfigParserTest, createSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({ conf.load("testdata/example.conf"); });
    ASSERT_NO_THROW({
        conf.createSection("section2");
        conf.add("key9", "value9");
        ASSERT_EQ(pplib::String("section2"), conf.getSectionName());
        ASSERT_EQ(pplib::String("value9"), conf.get("key9"));
    });
}

TEST_F(ConfigParserTest, createOnExistingSection)
{
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({ conf.load("testdata/example.conf"); });
    ASSERT_NO_THROW({
        conf.createSection("section1");
        conf.add("key9", "value9");
        ASSERT_EQ(pplib::String("section1"), conf.getSectionName());
        ASSERT_EQ(pplib::String("value9"), conf.get("key9"));
    });
}

TEST_F(ConfigParserTest, save)
{
    pplib::String TmpFile = pplib::Dir::tempPath() + "/pplib_test_example.conf.saved";
    pplib::ConfigParser conf;
    ASSERT_NO_THROW({ conf.load("testdata/example.conf"); });
    pplib::String s;
    pplib::String expected = "[global]\n"
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
        // conf.print();
        pplib::File::load(s, TmpFile);
    });
    EXPECT_EQ(expected, s);

    // Temp-File loeschen
    try {
        pplib::File::unlink(TmpFile);
    }
    catch (...) {
        // Do nothing
    }
}

} // namespace
