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
class PerlHelperTest : public ::testing::Test
{
protected:
    PerlHelperTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen: LC_CTYPE\n");
            throw std::exception();
        }
        if (setlocale(LC_TIME, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen: LC_TIME\n");
            throw std::exception();
        }
    }
    virtual ~PerlHelperTest()
    {
    }
};

TEST_F(PerlHelperTest, escapeString)
{
    pplib::String res;

    ASSERT_NO_THROW({ res = pplib::PerlHelper::escapeString("Emailadresse: \"test@test.de\""); });
    ASSERT_EQ(pplib::String("Emailadresse: \\\"test\\@test.de\\\""), res);
}

TEST_F(PerlHelperTest, escapeStringWithBackslashes)
{
    pplib::String res;

    ASSERT_NO_THROW({ res = pplib::PerlHelper::escapeString("test\\,test"); });
    ASSERT_EQ(pplib::String("test\\\\,test"), res);
}

TEST_F(PerlHelperTest, toHashWithEmptyArray)
{
    pplib::AssocArray a;
    pplib::String res;

    ASSERT_NO_THROW({ res = pplib::PerlHelper::toHash(a, "key"); });
    ASSERT_EQ(pplib::String("my %key;\n"), res);
}

TEST_F(PerlHelperTest, toHashWithEmptyArrayEmptyKey)
{
    pplib::AssocArray a;
    pplib::String res;
    ASSERT_NO_THROW({ res = pplib::PerlHelper::toHash(a, ""); });
    ASSERT_EQ(pplib::String(""), res);
}

TEST_F(PerlHelperTest, toHashNonRecursive)
{
    pplib::AssocArray a;
    pplib::String res;
    pplib::String expected = "my %key;\n"
                             "$key{key1}=\"value1\";\n"
                             "$key{key2}=\"20\";\n"
                             "$key{key3}=\"Geht\n"
                             "ueber\n"
                             "mehrere\n"
                             "Zeilen\n"
                             "\";\n";

    a.set("key1", "value1");
    a.set("key2", "20");
    a.set("key3", "Geht\nueber\nmehrere\nZeilen\n");
    ASSERT_NO_THROW({ res = pplib::PerlHelper::toHash(a, "key"); });
    ASSERT_EQ(expected, res);
}

TEST_F(PerlHelperTest, toHashNonRecursiveWithQuotationMark)
{
    pplib::AssocArray a;
    pplib::String res;
    pplib::String expected = "my %key;\n"
                             "$key{key1}=\"Ein \\\"besonderes\\\" Zeichen\";\n"
                             "$key{key2}=\"20\";\n"
                             "$key{key3}=\"Geht\n"
                             "ueber\n"
                             "mehrere\n"
                             "Zeilen\n"
                             "\";\n";

    a.set("key1", "Ein \"besonderes\" Zeichen");
    a.set("key2", "20");
    a.set("key3", "Geht\nueber\nmehrere\nZeilen\n");
    ASSERT_NO_THROW({ res = pplib::PerlHelper::toHash(a, "key"); });
    ASSERT_EQ(expected, res);
}

TEST_F(PerlHelperTest, toHashNonRecursiveWithAtChar)
{
    pplib::AssocArray a;
    pplib::String res;
    pplib::String expected = "my %key;\n"
                             "$key{email}=\"ppl\\@pfp.de\";\n"
                             "$key{key1}=\"Ein \\\"besonderes\\\" Zeichen\";\n"
                             "$key{key2}=\"20\";\n"
                             "$key{key3}=\"Geht\n"
                             "ueber\n"
                             "mehrere\n"
                             "Zeilen\n"
                             "\";\n";

    a.set("key1", "Ein \"besonderes\" Zeichen");
    a.set("key2", "20");
    a.set("key3", "Geht\nueber\nmehrere\nZeilen\n");
    a.set("email", "ppl@pfp.de");
    ASSERT_NO_THROW({ res = pplib::PerlHelper::toHash(a, "key"); });
    ASSERT_EQ(expected, res);
}

TEST_F(PerlHelperTest, toHashRecursive)
{
    pplib::AssocArray a;
    pplib::String res;
    pplib::String expected = "my %key;\n"
                             "$key{key1}=\"value1\";\n"
                             "$key{key2}=\"20\";\n"
                             "$key{key3}=\"Geht\n"
                             "ueber\n"
                             "mehrere\n"
                             "Zeilen\n"
                             "\";\n"
                             "$key{tree1}{key1}=\"subvalue1\";\n"
                             "$key{tree1}{key2}=\"subvalue2\";\n";

    a.set("key1", "value1");
    a.set("key2", "20");
    a.set("key3", "Geht\nueber\nmehrere\nZeilen\n");
    a.set("tree1/key1", "subvalue1");
    a.set("tree1/key2", "subvalue2");
    ASSERT_NO_THROW({ res = pplib::PerlHelper::toHash(a, "key"); });
    ASSERT_EQ(expected, res);
}

TEST_F(PerlHelperTest, toHashRecursiveWithQuotationMark)
{
    pplib::AssocArray a;
    pplib::String res;
    pplib::String expected = "my %key;\n"
                             "$key{key1}=\"Ein \\\"besonderes\\\" Zeichen\";\n"
                             "$key{key2}=\"20\";\n"
                             "$key{key3}=\"Geht\n"
                             "ueber\n"
                             "mehrere\n"
                             "Zeilen\n"
                             "\";\n"
                             "$key{tree1}{key1}=\"subvalue1\";\n"
                             "$key{tree1}{key2}=\"subvalue2\";\n"
                             "$key{tree1}{key3}=\"Ein \\\"besonderes\\\" Zeichen\";\n";

    a.set("key1", "Ein \"besonderes\" Zeichen");
    a.set("key2", "20");
    a.set("key3", "Geht\nueber\nmehrere\nZeilen\n");
    a.set("tree1/key1", "subvalue1");
    a.set("tree1/key2", "subvalue2");
    a.set("tree1/key3", "Ein \"besonderes\" Zeichen");

    ASSERT_NO_THROW({ res = pplib::PerlHelper::toHash(a, "key"); });
    ASSERT_EQ(expected, res);
}

TEST_F(PerlHelperTest, toHashRecursiveWithAtChar)
{
    pplib::AssocArray a;
    pplib::String res;
    pplib::String expected = "my %key;\n"
                             "$key{email}=\"ppl\\@pfp.de\";\n"
                             "$key{key1}=\"Ein \\\"besonderes\\\" Zeichen\";\n"
                             "$key{key2}=\"20\";\n"
                             "$key{key3}=\"Geht\n"
                             "ueber\n"
                             "mehrere\n"
                             "Zeilen\n"
                             "\";\n"
                             "$key{tree1}{email}=\"test\\@test.de\";\n"
                             "$key{tree1}{key1}=\"subvalue1\";\n"
                             "$key{tree1}{key2}=\"subvalue2\";\n";

    a.set("key1", "Ein \"besonderes\" Zeichen");
    a.set("key2", "20");
    a.set("key3", "Geht\nueber\nmehrere\nZeilen\n");
    a.set("email", "ppl@pfp.de");
    a.set("tree1/key1", "subvalue1");
    a.set("tree1/key2", "subvalue2");
    a.set("tree1/email", "test@test.de");

    ASSERT_NO_THROW({ res = pplib::PerlHelper::toHash(a, "key"); });
    ASSERT_EQ(expected, res);
}

} // namespace
