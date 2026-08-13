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

extern const wchar_t* wordlist;

extern pplib::Array Wordlist;

namespace
{

// The fixture for testing class Foo.
class ListTest : public ::testing::Test
{
protected:
    ListTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~ListTest()
    {
    }
};

TEST_F(ListTest, ConstructorSimple)
{
    ASSERT_NO_THROW({ pplib::List<pplib::String> myList; });
}

TEST_F(ListTest, add)
{
    pplib::List<pplib::String> myList;
    myList.add("Item 1");
    ASSERT_EQ((size_t)1, myList.count());
    myList.add("Item 2");
    ASSERT_EQ((size_t)2, myList.count());
}

TEST_F(ListTest, clear)
{
    pplib::List<pplib::String> myList;
    myList.add("Item 1");
    myList.add("Item 2");
    myList.add("Item 3");

    ASSERT_EQ((size_t)3, myList.count());
    myList.clear();
    ASSERT_EQ((size_t)0, myList.count());
}

TEST_F(ListTest, reset)
{
    pplib::List<pplib::String> myList;
    myList.add("Item 1");
    myList.add("Item 2");
    myList.add("Item 3");

    pplib::List<pplib::String>::Iterator it;
    myList.reset(it);
}

TEST_F(ListTest, getNext)
{
    pplib::List<pplib::String> myList;
    myList.add("Item 1");
    myList.add("Item 2");
    myList.add("Item 3");

    pplib::List<pplib::String>::Iterator it;
    myList.reset(it);
    ASSERT_EQ(true, myList.getNext(it));
    ASSERT_EQ(pplib::String("Item 1"), it.value());

    ASSERT_EQ(true, myList.getNext(it));
    ASSERT_EQ(pplib::String("Item 2"), it.value());

    ASSERT_EQ(true, myList.getNext(it));
    ASSERT_EQ(pplib::String("Item 3"), it.value());

    ASSERT_EQ(false, myList.getNext(it));
}

TEST_F(ListTest, getPrevious)
{
    pplib::List<pplib::String> myList;
    myList.add("Item 1");
    myList.add("Item 2");
    myList.add("Item 3");

    pplib::List<pplib::String>::Iterator it;
    myList.reset(it);
    ASSERT_EQ(true, myList.getPrevious(it));
    ASSERT_EQ(pplib::String("Item 3"), it.value());

    ASSERT_EQ(true, myList.getPrevious(it));
    ASSERT_EQ(pplib::String("Item 2"), it.value());

    ASSERT_EQ(true, myList.getPrevious(it));
    ASSERT_EQ(pplib::String("Item 1"), it.value());

    ASSERT_EQ(false, myList.getPrevious(it));
}

TEST_F(ListTest, addWordlist)
{
    pplib::List<pplib::String> myList;
    size_t total = Wordlist.count();
    myList.reserve(total + 10);
    ASSERT_EQ((size_t)124332, Wordlist.count()) << "List has unexpected size";
    // pplib::PrintDebugTime ("Wortliste in List laden\n");
    for (size_t i = 0; i < total; i++) {
        myList.add(Wordlist[i]);
    }
    ASSERT_EQ(total, myList.count()) << "List has unexpected size";
    // pplib::PrintDebugTime ("done\n");
}

} // namespace
