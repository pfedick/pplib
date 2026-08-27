/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <gtest/gtest.h>

#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/array.h>
#include <pplib/types/bytearray.h>
#include <pplib/exceptions.h>
#include <pplib/core/regex.h>

#include "pplib-tests.h"

namespace
{

// The fixture for testing class Foo.
class PcreTest : public ::testing::Test
{
protected:
    PcreTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~PcreTest()
    {
    }
};

TEST_F(PcreTest, bool_compile_match)
{

    ASSERT_NO_THROW({
        pplib::RegEx::Pattern p = pplib::RegEx::compile("^Hello.*$");
        ASSERT_TRUE(pplib::RegEx::match(p, "Hello World"));
        ASSERT_FALSE(pplib::RegEx::match(p, "Helleo World"));
    } pplib::RegEx::compile("^.*\\.json$");

    );
}

TEST_F(PcreTest, bool_match)
{

    ASSERT_NO_THROW({
        ASSERT_TRUE(pplib::RegEx::match("^Hello.*$", "Hello World"));
        ASSERT_FALSE(pplib::RegEx::match("^Hello.*$", "Helleo World"));
    });
}

TEST_F(PcreTest, MatchPositive)
{
    pplib::String s1(
        "Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis "
        "natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
    pplib::String expr("^Lorem.*$");
    ASSERT_TRUE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::DOTALL));
    expr.set("^Lorem.*$");
    ASSERT_TRUE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::DOTALL | pplib::RegEx::Flags::CASELESS));
    expr.set("consectetuer");
    ASSERT_TRUE(pplib::RegEx::match(expr, s1));
    expr.set("^.*consectetuer.*$");
    ASSERT_TRUE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::DOTALL));
    expr.set("^.*mus\\.$");
    ASSERT_TRUE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::MULTILINE));
    ASSERT_TRUE(pplib::RegEx::match("^.*\\.json$", "blah.json"));
}

TEST_F(PcreTest, MatchNegativ)
{
    pplib::String s1(
        "Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis "
        "natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
    pplib::String expr("^Looorem.*$");
    ASSERT_FALSE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::DOTALL));
    expr.set("^ipsum.*$");
    ASSERT_FALSE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::DOTALL | pplib::RegEx::Flags::CASELESS));
    expr.set("patrick");
    ASSERT_FALSE(pplib::RegEx::match(expr, s1));
    expr.set("^.*patrick.*$");
    ASSERT_FALSE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::DOTALL));
    expr.set("^.*mus\\.$");
    ASSERT_FALSE(pplib::RegEx::match(expr, s1));
    ASSERT_FALSE(pplib::RegEx::match("^.*\\.json$", "."));
}

TEST_F(PcreTest, MatchPerlRegExPositive)
{
    pplib::String s1(
        "Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum sociis "
        "natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
    pplib::String expr("/^Lorem.*$/s");
    ASSERT_TRUE(pplib::RegEx::match(expr, s1));
    expr.set("/^Lorem.*$/is");
    ASSERT_TRUE(pplib::RegEx::match(expr, s1));
    expr.set("/consectetuer/");
    ASSERT_TRUE(pplib::RegEx::match(expr, s1));
    expr.set("/^.*consectetuer.*$/s");
    ASSERT_TRUE(pplib::RegEx::match(expr, s1));
    expr.set("/^.*mus\\.$/m");
    ASSERT_TRUE(pplib::RegEx::match(expr, s1));
}

TEST_F(PcreTest, capture)
{
    std::vector<pplib::String> m;
    pplib::String s1("2012-05-18");
    ASSERT_TRUE(pplib::RegEx::capture("/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})$/", s1, m));
    ASSERT_EQ((size_t)4, m.size()) << "Unexpected number auf captures";
    ASSERT_EQ(2012, m[1].toInt()) << "Unexpected value in capture";
    ASSERT_EQ(5, m[2].toInt()) << "Unexpected value in capture";
    ASSERT_EQ(18, m[3].toInt()) << "Unexpected value in capture";
}

TEST_F(PcreTest, replace)
{
    pplib::String s1("Lorem ipsum dolor sit amet.");
    pplib::String expected("Lor3m ipsum dolor sit am3t.");
    pplib::String result = pplib::RegEx::replace("/e/", s1, "3");
    ASSERT_EQ(expected, result) << "Unexpected result from pregReplace";

    ASSERT_EQ(pplib::String("Lorem --- amet."), pplib::RegEx::replace("ip.*sit", s1, "---"));
    ASSERT_EQ(pplib::String("Lorem  amet."), pplib::RegEx::replace("ip.*sit", s1, ""));
    ASSERT_EQ(pplib::String("Lor4m ipsum dolor sit amet."), pplib::RegEx::replace("e", s1, "4", 0, 1));

    // Test gegen den Bug: Ersetzen am Ende des Strings, das den String kürzt
    pplib::String s2("208.aiff");
    ASSERT_EQ(pplib::String("208"), pplib::RegEx::replace("/.aiff$/i", s2, ""));
}

TEST_F(PcreTest, escape)
{
    pplib::String s1("Lorem ipsum dolor sit amet.");
    ASSERT_EQ(pplib::String("Hello \\+Wor\\/ld"), pplib::RegEx::escape("Hello +Wor/ld"));
}

class PcreTestWideChar : public ::testing::Test
{
protected:
    PcreTestWideChar()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~PcreTestWideChar()
    {
    }
};

TEST_F(PcreTestWideChar, bool_compile)
{

    ASSERT_NO_THROW({ pplib::RegEx::Pattern p = pplib::RegEx::compile(L"^Hello.*$"); });
    ASSERT_NO_THROW({ pplib::RegEx::compile(L"^.*\\.json$"); });
}

TEST_F(PcreTestWideChar, bool_match)
{

    ASSERT_NO_THROW({
        pplib::RegEx::Pattern p = pplib::RegEx::compile(L"^Hello.*$");
        ASSERT_TRUE(pplib::RegEx::match(p, L"Hello World"));
        ASSERT_FALSE(pplib::RegEx::match(p, L"Helleo World"));
    });
}

TEST_F(PcreTestWideChar, MatchPositive)
{
    pplib::WideString s1(L"Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum "
                         L"sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
    pplib::WideString expr(L"^Lorem.*$");
    ASSERT_TRUE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::DOTALL));
    expr.set(L"^Lorem.*$");
    ASSERT_TRUE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::DOTALL | pplib::RegEx::Flags::CASELESS));
    expr.set(L"consectetuer");
    ASSERT_TRUE(pplib::RegEx::match(expr, s1));
    expr.set(L"^.*consectetuer.*$");
    ASSERT_TRUE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::DOTALL));
}

TEST_F(PcreTestWideChar, MatchMultiline)
{
    pplib::WideString s1(L"Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum "
                         L"sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
    pplib::WideString expr(L"^.*mus\\.$");
    ASSERT_TRUE(pplib::RegEx::match(L"^.*\\.json$", L"blah.json"));
    ASSERT_TRUE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::MULTILINE));
}

TEST_F(PcreTestWideChar, MatchNegativ)
{
    pplib::WideString s1(L"Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum "
                         L"sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
    pplib::WideString expr(L"^Looorem.*$");
    ASSERT_FALSE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::DOTALL));
    expr.set(L"^ipsum.*$");
    ASSERT_FALSE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::DOTALL | pplib::RegEx::Flags::CASELESS));
    expr.set(L"patrick");
    ASSERT_FALSE(pplib::RegEx::match(expr, s1));
    expr.set(L"^.*patrick.*$");
    ASSERT_FALSE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::DOTALL));
    expr.set(L"^.*mus\\.$");
    ASSERT_FALSE(pplib::RegEx::match(expr, s1));
    ASSERT_FALSE(pplib::RegEx::match(L"^.*\\.json$", L"."));
}

TEST_F(PcreTestWideChar, capture)
{
    std::vector<pplib::WideString> m;
    pplib::WideString s1(L"2012-05-18");
    pplib::RegEx::Pattern p = pplib::RegEx::compile(L"/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})$/i");
    ASSERT_TRUE(pplib::RegEx::capture(L"/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})$/", s1, m));
    ASSERT_EQ((size_t)4, m.size()) << "Unexpected number auf captures";
    ASSERT_EQ(2012, m[1].toInt()) << "Unexpected value in capture";
    ASSERT_EQ(5, m[2].toInt()) << "Unexpected value in capture";
    ASSERT_EQ(18, m[3].toInt()) << "Unexpected value in capture";
}

TEST_F(PcreTestWideChar, replace)
{
    pplib::WideString s1(L"Lorem ipsum dolor sit amet.");
    pplib::WideString expected(L"Lor3m ipsum dolor sit am3t.");
    pplib::WideString result = pplib::RegEx::replace(L"/e/", s1, L"3");
    ASSERT_EQ(expected, result) << "Unexpected result from pregReplace";

    ASSERT_EQ(pplib::WideString(L"Lorem --- amet."), pplib::RegEx::replace(L"ip.*sit", s1, L"---"));
    ASSERT_EQ(pplib::WideString(L"Lorem  amet."), pplib::RegEx::replace(L"ip.*sit", s1, L""));
    ASSERT_EQ(pplib::WideString(L"Lor4m ipsum dolor sit amet."), pplib::RegEx::replace(L"e", s1, L"4", 0, 1));
}

TEST_F(PcreTestWideChar, escape)
{
    pplib::WideString s1(L"Lorem ipsum dolor sit amet.");
    ASSERT_EQ(pplib::WideString(L"Hello \\+Wor\\/ld"), pplib::RegEx::escape(L"Hello +Wor/ld"));
}

} // namespace
