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
class RegExTest : public ::testing::Test
{
protected:
    RegExTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~RegExTest()
    {
    }
};

TEST_F(RegExTest, bool_compile_match)
{
    ASSERT_NO_THROW({
        pplib::RegEx::Pattern p = pplib::RegEx::compile("^Hello.*$");
        ASSERT_TRUE(pplib::RegEx::match(p, "Hello World"));
        ASSERT_FALSE(pplib::RegEx::match(p, "Helleo World"));
    });
    ASSERT_NO_THROW({ pplib::RegEx::compile("^.*\\.json$"); });
}

TEST_F(RegExTest, bool_match)
{

    ASSERT_NO_THROW({
        ASSERT_TRUE(pplib::RegEx::match("^Hello.*$", "Hello World"));
        ASSERT_FALSE(pplib::RegEx::match("^Hello.*$", "Helleo World"));
    });
}

TEST_F(RegExTest, MatchPositive)
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

TEST_F(RegExTest, MatchNegativ)
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

TEST_F(RegExTest, MatchPerlRegExPositive)
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

TEST_F(RegExTest, capture)
{
    std::vector<pplib::String> m;
    pplib::String s1("2012-05-18");
    ASSERT_TRUE(pplib::RegEx::capture("/^([0-9]{4})[\\.-]([0-9]{1,2})[\\.-]([0-9]{1,2})$/", s1, m));
    ASSERT_EQ((size_t)4, m.size()) << "Unexpected number auf captures";
    ASSERT_EQ(2012, m[1].toInt()) << "Unexpected value in capture";
    ASSERT_EQ(5, m[2].toInt()) << "Unexpected value in capture";
    ASSERT_EQ(18, m[3].toInt()) << "Unexpected value in capture";
}

TEST_F(RegExTest, replace)
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

TEST_F(RegExTest, escape)
{
    pplib::String s1("Lorem ipsum dolor sit amet.");
    ASSERT_EQ(pplib::String("Hello \\+Wor\\/ld"), pplib::RegEx::escape("Hello +Wor/ld"));
}

TEST_F(RegExTest, PatternLifecycle)
{
    // Default constructor
    pplib::RegEx::Pattern empty;
    EXPECT_THROW(pplib::RegEx::match(empty, "test"), pplib::IllegalRegularExpressionException);
    std::vector<pplib::String> m;
    EXPECT_THROW(pplib::RegEx::capture(empty, "test", m), pplib::IllegalRegularExpressionException);
    EXPECT_THROW(pplib::RegEx::replace(empty, "test", "X"), pplib::IllegalRegularExpressionException);

    // Copy constructor
    pplib::RegEx::Pattern p1 = pplib::RegEx::compile("^foo$");
    pplib::RegEx::Pattern p2(p1);
    EXPECT_TRUE(pplib::RegEx::match(p1, "foo"));
    EXPECT_TRUE(pplib::RegEx::match(p2, "foo"));
    EXPECT_FALSE(pplib::RegEx::match(p2, "bar"));

    // Move constructor: Quelle muss geleert werden und wirft bei Benutzung
    pplib::RegEx::Pattern p3(std::move(p1));
    EXPECT_TRUE(pplib::RegEx::match(p3, "foo"));
    EXPECT_THROW(pplib::RegEx::match(p1, "foo"), pplib::IllegalRegularExpressionException);

    // Copy assignment
    pplib::RegEx::Pattern p4;
    p4 = p2;
    EXPECT_TRUE(pplib::RegEx::match(p4, "foo"));
    EXPECT_TRUE(pplib::RegEx::match(p2, "foo"));

    // Move assignment
    pplib::RegEx::Pattern p5;
    p5 = std::move(p2);
    EXPECT_TRUE(pplib::RegEx::match(p5, "foo"));
    EXPECT_THROW(pplib::RegEx::match(p2, "foo"), pplib::IllegalRegularExpressionException);

    // Self-assignment
    p4 = p4;
    EXPECT_TRUE(pplib::RegEx::match(p4, "foo"));
    p4 = std::move(p4);
    EXPECT_TRUE(pplib::RegEx::match(p4, "foo"));

    // Swap
    pplib::RegEx::Pattern pA = pplib::RegEx::compile("^A$");
    pplib::RegEx::Pattern pB = pplib::RegEx::compile("^B$");
    pA.swap(pB);
    EXPECT_TRUE(pplib::RegEx::match(pA, "B"));
    EXPECT_TRUE(pplib::RegEx::match(pB, "A"));
}

TEST_F(RegExTest, WidthMismatch)
{
    pplib::RegEx::Pattern p = pplib::RegEx::compile("^foo$");
    EXPECT_THROW(pplib::RegEx::match(p, pplib::WideString(L"foo")), pplib::IllegalArgumentException);
    std::vector<pplib::WideString> wm;
    EXPECT_THROW(pplib::RegEx::capture(p, pplib::WideString(L"foo"), wm), pplib::IllegalArgumentException);
    EXPECT_THROW(pplib::RegEx::replace(p, pplib::WideString(L"foo"), pplib::WideString(L"bar")), pplib::IllegalArgumentException);
}

TEST_F(RegExTest, CompileErrors)
{
    EXPECT_THROW(pplib::RegEx::compile("[unterminated"), pplib::IllegalRegularExpressionException);
    EXPECT_THROW(pplib::RegEx::compile("*startingWithQuantifier"), pplib::IllegalRegularExpressionException);
    EXPECT_THROW(pplib::RegEx::compile("/[/"), pplib::IllegalRegularExpressionException);
    // Ungültiges UTF-8 Bytefolge muss abgefangen werden
    EXPECT_THROW(pplib::RegEx::compile(pplib::String("\xFF\xFF")), pplib::IllegalRegularExpressionException);
}

TEST_F(RegExTest, Flags)
{
    // CASELESS
    EXPECT_TRUE(pplib::RegEx::match("^abc$", "ABC", pplib::RegEx::Flags::CASELESS));
    EXPECT_FALSE(pplib::RegEx::match("^abc$", "ABC"));

    // ANCHORED
    EXPECT_TRUE(pplib::RegEx::match("hello", "hello world", pplib::RegEx::Flags::ANCHORED));
    EXPECT_FALSE(pplib::RegEx::match("world", "hello world", pplib::RegEx::Flags::ANCHORED));

    // EXTENDED
    EXPECT_TRUE(pplib::RegEx::match("hello # comment\n  world", "helloworld", pplib::RegEx::Flags::EXTENDED));

    // UNGREEDY
    EXPECT_EQ(pplib::String("Xa2b"), pplib::RegEx::replace("a.*b", "a1ba2b", "X", pplib::RegEx::Flags::UNGREEDY, 1));

    // PerlRegEx Flags: i, m, s, x, a, u
    EXPECT_TRUE(pplib::RegEx::match("/^abc$/i", "ABC"));
    EXPECT_TRUE(pplib::RegEx::match("/hello # comment\n  world/x", "helloworld"));
    EXPECT_TRUE(pplib::RegEx::match("/hello/a", "hello world"));
    EXPECT_FALSE(pplib::RegEx::match("/world/a", "hello world"));
    EXPECT_EQ(pplib::String("Xa2b"), pplib::RegEx::replace("/a.*b/u", "a1ba2b", "X", 0, 1));
}

TEST_F(RegExTest, PerlRegExWithoutClosingSlash)
{
    // String fängt mit '/' an, hat aber keinen schließenden '/' -> normales Regex
    EXPECT_TRUE(pplib::RegEx::match("/usr/local/bin", "/usr/local/bin"));
    EXPECT_FALSE(pplib::RegEx::match("/usr/local/bin", "other"));
    EXPECT_TRUE(pplib::RegEx::match("/bin", "/bin"));
    EXPECT_FALSE(pplib::RegEx::match("/bin", "other"));
}

TEST_F(RegExTest, MatchSubjectRuntimeError)
{
    // Ungültiges UTF-8 im Subject führt zu Laufzeitfehler in pcre2_match -> OperationFailedException
    EXPECT_THROW(pplib::RegEx::match("^abc$", pplib::String("\xFF\xFF")), pplib::OperationFailedException);
    pplib::RegEx::Pattern p = pplib::RegEx::compile("^abc$");
    EXPECT_THROW(pplib::RegEx::match(p, pplib::String("\xFF\xFF")), pplib::OperationFailedException);
    std::vector<pplib::String> m;
    EXPECT_THROW(pplib::RegEx::capture(p, pplib::String("\xFF\xFF"), m), pplib::OperationFailedException);
    EXPECT_THROW(pplib::RegEx::replace(p, pplib::String("\xFF\xFF"), "X"), pplib::OperationFailedException);
}

TEST_F(RegExTest, ZeroLengthMatchReplace)
{
    // Alltägliches Zero-Length Match (a* auf nicht-a String) darf nicht hängen
    ASSERT_EQ(pplib::String("XtXeXsXtX"), pplib::RegEx::replace("a*", "test", "X"));

    // Mit max
    ASSERT_EQ(pplib::String("XtXest"), pplib::RegEx::replace("a*", "test", "X", 0, 2));

    // Mit UTF-8 Multibyte Codepoints
    ASSERT_EQ(pplib::String("XäXöXüX"), pplib::RegEx::replace("a*", "äöü", "X"));

    // Am Stringanfang / Stringende
    ASSERT_EQ(pplib::String("!test"), pplib::RegEx::replace("^", "test", "!"));
    ASSERT_EQ(pplib::String("test!"), pplib::RegEx::replace("$", "test", "!"));

    // Ohne Match
    ASSERT_EQ(pplib::String("hello"), pplib::RegEx::replace("xyz", "hello", "X"));
}

TEST_F(RegExTest, CaptureDetails)
{
    std::vector<pplib::String> m;
    EXPECT_FALSE(pplib::RegEx::capture("^([0-9]+)$", "abc", m));
    EXPECT_TRUE(m.empty());

    EXPECT_TRUE(pplib::RegEx::capture("^([a-z]+)-([0-9]+)$", "abc-123", m));
    ASSERT_EQ((size_t)3, m.size());
    EXPECT_EQ(pplib::String("abc-123"), m[0]);
    EXPECT_EQ(pplib::String("abc"), m[1]);
    EXPECT_EQ(pplib::String("123"), m[2]);
}

TEST_F(RegExTest, EscapeAllMetacharacters)
{
    EXPECT_EQ(pplib::String(""), pplib::RegEx::escape(""));
    EXPECT_EQ(pplib::String("Hello World 123"), pplib::RegEx::escape("Hello World 123"));

    // Alle Metazeichen: \ . ^ $ * + - ? ( ) [ ] { } | /
    pplib::String allMeta("\\.^$*+-?()[]{}|/");
    pplib::String escaped = pplib::RegEx::escape(allMeta);
    EXPECT_EQ(pplib::String("\\\\\\.\\^\\$\\*\\+\\-\\?\\(\\)\\[\\]\\{\\}\\|\\/"), escaped);

    // Literal Matching Test: Eingebetteter escaped String matcht exakt sich selbst
    pplib::String literal = "foo[bar](1+2)*{x}?$test^/baz\\qux-end.";
    pplib::String pattern = "^" + pplib::RegEx::escape(literal) + "$";
    EXPECT_TRUE(pplib::RegEx::match(pattern, literal));
    EXPECT_FALSE(pplib::RegEx::match(pattern, "foo bar 1 2"));
}

class RegExTestWideChar : public ::testing::Test
{
protected:
    RegExTestWideChar()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~RegExTestWideChar()
    {
    }
};

TEST_F(RegExTestWideChar, bool_compile)
{

    ASSERT_NO_THROW({ pplib::RegEx::Pattern p = pplib::RegEx::compile(L"^Hello.*$"); });
    ASSERT_NO_THROW({ pplib::RegEx::compile(L"^.*\\.json$"); });
}

TEST_F(RegExTestWideChar, bool_match)
{

    ASSERT_NO_THROW({
        pplib::RegEx::Pattern p = pplib::RegEx::compile(L"^Hello.*$");
        ASSERT_TRUE(pplib::RegEx::match(p, L"Hello World"));
        ASSERT_FALSE(pplib::RegEx::match(p, L"Helleo World"));
    });
}

TEST_F(RegExTestWideChar, MatchPositive)
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

TEST_F(RegExTestWideChar, MatchMultiline)
{
    pplib::WideString s1(L"Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean commodo ligula eget dolor. Aenean massa. Cum "
                         L"sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.");
    pplib::WideString expr(L"^.*mus\\.$");
    ASSERT_TRUE(pplib::RegEx::match(L"^.*\\.json$", L"blah.json"));
    ASSERT_TRUE(pplib::RegEx::match(expr, s1, pplib::RegEx::Flags::MULTILINE));
}

TEST_F(RegExTestWideChar, MatchNegativ)
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

TEST_F(RegExTestWideChar, capture)
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

TEST_F(RegExTestWideChar, replace)
{
    pplib::WideString s1(L"Lorem ipsum dolor sit amet.");
    pplib::WideString expected(L"Lor3m ipsum dolor sit am3t.");
    pplib::WideString result = pplib::RegEx::replace(L"/e/", s1, L"3");
    ASSERT_EQ(expected, result) << "Unexpected result from pregReplace";

    ASSERT_EQ(pplib::WideString(L"Lorem --- amet."), pplib::RegEx::replace(L"ip.*sit", s1, L"---"));
    ASSERT_EQ(pplib::WideString(L"Lorem  amet."), pplib::RegEx::replace(L"ip.*sit", s1, L""));
    ASSERT_EQ(pplib::WideString(L"Lor4m ipsum dolor sit amet."), pplib::RegEx::replace(L"e", s1, L"4", 0, 1));
}

TEST_F(RegExTestWideChar, escape)
{
    pplib::WideString s1(L"Lorem ipsum dolor sit amet.");
    ASSERT_EQ(pplib::WideString(L"Hello \\+Wor\\/ld"), pplib::RegEx::escape(L"Hello +Wor/ld"));
}

TEST_F(RegExTestWideChar, PatternLifecycle)
{
    // Default constructor
    pplib::RegEx::Pattern empty;
    EXPECT_THROW(pplib::RegEx::match(empty, pplib::WideString(L"test")), pplib::IllegalRegularExpressionException);
    std::vector<pplib::WideString> m;
    EXPECT_THROW(pplib::RegEx::capture(empty, pplib::WideString(L"test"), m), pplib::IllegalRegularExpressionException);
    EXPECT_THROW(pplib::RegEx::replace(empty, pplib::WideString(L"test"), pplib::WideString(L"X")),
                 pplib::IllegalRegularExpressionException);

    // Copy constructor
    pplib::RegEx::Pattern p1 = pplib::RegEx::compile(pplib::WideString(L"^foo$"));
    pplib::RegEx::Pattern p2(p1);
    EXPECT_TRUE(pplib::RegEx::match(p1, pplib::WideString(L"foo")));
    EXPECT_TRUE(pplib::RegEx::match(p2, pplib::WideString(L"foo")));
    EXPECT_FALSE(pplib::RegEx::match(p2, pplib::WideString(L"bar")));

    // Move constructor
    pplib::RegEx::Pattern p3(std::move(p1));
    EXPECT_TRUE(pplib::RegEx::match(p3, pplib::WideString(L"foo")));
    EXPECT_THROW(pplib::RegEx::match(p1, pplib::WideString(L"foo")), pplib::IllegalRegularExpressionException);

    // Copy assignment
    pplib::RegEx::Pattern p4;
    p4 = p2;
    EXPECT_TRUE(pplib::RegEx::match(p4, pplib::WideString(L"foo")));
    EXPECT_TRUE(pplib::RegEx::match(p2, pplib::WideString(L"foo")));

    // Move assignment
    pplib::RegEx::Pattern p5;
    p5 = std::move(p2);
    EXPECT_TRUE(pplib::RegEx::match(p5, pplib::WideString(L"foo")));
    EXPECT_THROW(pplib::RegEx::match(p2, pplib::WideString(L"foo")), pplib::IllegalRegularExpressionException);

    // Self-assignment
    p4 = p4;
    EXPECT_TRUE(pplib::RegEx::match(p4, pplib::WideString(L"foo")));
    p4 = std::move(p4);
    EXPECT_TRUE(pplib::RegEx::match(p4, pplib::WideString(L"foo")));
}

TEST_F(RegExTestWideChar, WidthMismatch)
{
    pplib::RegEx::Pattern wp = pplib::RegEx::compile(pplib::WideString(L"^foo$"));
    EXPECT_THROW(pplib::RegEx::match(wp, "foo"), pplib::IllegalArgumentException);
    std::vector<pplib::String> m;
    EXPECT_THROW(pplib::RegEx::capture(wp, "foo", m), pplib::IllegalArgumentException);
    EXPECT_THROW(pplib::RegEx::replace(wp, "foo", "bar"), pplib::IllegalArgumentException);
}

TEST_F(RegExTestWideChar, CompileErrors)
{
    EXPECT_THROW(pplib::RegEx::compile(pplib::WideString(L"[unterminated")), pplib::IllegalRegularExpressionException);
    EXPECT_THROW(pplib::RegEx::compile(pplib::WideString(L"*startingWithQuantifier")), pplib::IllegalRegularExpressionException);
    EXPECT_THROW(pplib::RegEx::compile(pplib::WideString(L"/[/")), pplib::IllegalRegularExpressionException);
}

TEST_F(RegExTestWideChar, Flags)
{
    // CASELESS
    EXPECT_TRUE(pplib::RegEx::match(pplib::WideString(L"^abc$"), pplib::WideString(L"ABC"), pplib::RegEx::Flags::CASELESS));
    EXPECT_FALSE(pplib::RegEx::match(pplib::WideString(L"^abc$"), pplib::WideString(L"ABC")));

    // ANCHORED
    EXPECT_TRUE(pplib::RegEx::match(pplib::WideString(L"hello"), pplib::WideString(L"hello world"), pplib::RegEx::Flags::ANCHORED));
    EXPECT_FALSE(pplib::RegEx::match(pplib::WideString(L"world"), pplib::WideString(L"hello world"), pplib::RegEx::Flags::ANCHORED));

    // EXTENDED
    EXPECT_TRUE(pplib::RegEx::match(pplib::WideString(L"hello # comment\n  world"), pplib::WideString(L"helloworld"),
                                    pplib::RegEx::Flags::EXTENDED));

    // UNGREEDY
    EXPECT_EQ(pplib::WideString(L"Xa2b"), pplib::RegEx::replace(pplib::WideString(L"a.*b"), pplib::WideString(L"a1ba2b"),
                                                                pplib::WideString(L"X"), pplib::RegEx::Flags::UNGREEDY, 1));

    // PerlRegEx Flags
    EXPECT_TRUE(pplib::RegEx::match(pplib::WideString(L"/^abc$/i"), pplib::WideString(L"ABC")));
    EXPECT_TRUE(pplib::RegEx::match(pplib::WideString(L"/hello # comment\n  world/x"), pplib::WideString(L"helloworld")));
    EXPECT_TRUE(pplib::RegEx::match(pplib::WideString(L"/hello/a"), pplib::WideString(L"hello world")));
    EXPECT_FALSE(pplib::RegEx::match(pplib::WideString(L"/world/a"), pplib::WideString(L"hello world")));
    EXPECT_EQ(pplib::WideString(L"Xa2b"),
              pplib::RegEx::replace(pplib::WideString(L"/a.*b/u"), pplib::WideString(L"a1ba2b"), pplib::WideString(L"X"), 0, 1));
}

TEST_F(RegExTestWideChar, PerlRegExWithoutClosingSlash)
{
    EXPECT_TRUE(pplib::RegEx::match(pplib::WideString(L"/usr/local/bin"), pplib::WideString(L"/usr/local/bin")));
    EXPECT_FALSE(pplib::RegEx::match(pplib::WideString(L"/usr/local/bin"), pplib::WideString(L"other")));
    EXPECT_TRUE(pplib::RegEx::match(pplib::WideString(L"/bin"), pplib::WideString(L"/bin")));
    EXPECT_FALSE(pplib::RegEx::match(pplib::WideString(L"/bin"), pplib::WideString(L"other")));
}

TEST_F(RegExTestWideChar, MatchSubjectRuntimeError)
{
#if defined(HAVE_PCRE2_BITS_16) && (WCHAR_MAX <= 0xffff)
    // Unter Windows / UTF-16 führt ein ungültiges Surrogatpaar zu OperationFailedException
    pplib::WideString invalidSurrogate(L"\xD83D");
    EXPECT_THROW(pplib::RegEx::match(pplib::WideString(L"^abc$"), invalidSurrogate), pplib::OperationFailedException);
    pplib::RegEx::Pattern p = pplib::RegEx::compile(pplib::WideString(L"^abc$"));
    EXPECT_THROW(pplib::RegEx::match(p, invalidSurrogate), pplib::OperationFailedException);
    std::vector<pplib::WideString> m;
    EXPECT_THROW(pplib::RegEx::capture(p, invalidSurrogate, m), pplib::OperationFailedException);
    EXPECT_THROW(pplib::RegEx::replace(p, invalidSurrogate, pplib::WideString(L"X")), pplib::OperationFailedException);
#endif
}

TEST_F(RegExTestWideChar, ZeroLengthMatchReplace)
{
    // Alltägliches Zero-Length Match darf nicht hängen
    ASSERT_EQ(pplib::WideString(L"XtXeXsXtX"),
              pplib::RegEx::replace(pplib::WideString(L"a*"), pplib::WideString(L"test"), pplib::WideString(L"X")));

    // Mit max
    ASSERT_EQ(pplib::WideString(L"XtXest"),
              pplib::RegEx::replace(pplib::WideString(L"a*"), pplib::WideString(L"test"), pplib::WideString(L"X"), 0, 2));

    // Mit Umlauten
    ASSERT_EQ(pplib::WideString(L"XäXöXüX"),
              pplib::RegEx::replace(pplib::WideString(L"a*"), pplib::WideString(L"äöü"), pplib::WideString(L"X")));

    // UTF-16 Surrogat-Paar (Emoji: 🚀 \xD83D\xDE80)
    ASSERT_EQ(pplib::WideString(L"X\xD83D\xDE80X"),
              pplib::RegEx::replace(pplib::WideString(L"a*"), pplib::WideString(L"\xD83D\xDE80"), pplib::WideString(L"X")));

    // Am Stringanfang / Stringende
    ASSERT_EQ(pplib::WideString(L"!test"),
              pplib::RegEx::replace(pplib::WideString(L"^"), pplib::WideString(L"test"), pplib::WideString(L"!")));
    ASSERT_EQ(pplib::WideString(L"test!"),
              pplib::RegEx::replace(pplib::WideString(L"$"), pplib::WideString(L"test"), pplib::WideString(L"!")));

    // Ohne Match
    ASSERT_EQ(pplib::WideString(L"hello"),
              pplib::RegEx::replace(pplib::WideString(L"xyz"), pplib::WideString(L"hello"), pplib::WideString(L"X")));
}

TEST_F(RegExTestWideChar, CaptureDetails)
{
    std::vector<pplib::WideString> m;
    EXPECT_FALSE(pplib::RegEx::capture(pplib::WideString(L"^([0-9]+)$"), pplib::WideString(L"abc"), m));
    EXPECT_TRUE(m.empty());

    EXPECT_TRUE(pplib::RegEx::capture(pplib::WideString(L"^([a-z]+)-([0-9]+)$"), pplib::WideString(L"abc-123"), m));
    ASSERT_EQ((size_t)3, m.size());
    EXPECT_EQ(pplib::WideString(L"abc-123"), m[0]);
    EXPECT_EQ(pplib::WideString(L"abc"), m[1]);
    EXPECT_EQ(pplib::WideString(L"123"), m[2]);
}

TEST_F(RegExTestWideChar, EscapeAllMetacharacters)
{
    EXPECT_EQ(pplib::WideString(L""), pplib::RegEx::escape(pplib::WideString(L"")));
    EXPECT_EQ(pplib::WideString(L"Hello World 123"), pplib::RegEx::escape(pplib::WideString(L"Hello World 123")));

    // Alle Metazeichen: \ . ^ $ * + - ? ( ) [ ] { } | /
    pplib::WideString allMeta(L"\\.^$*+-?()[]{}|/");
    pplib::WideString escaped = pplib::RegEx::escape(allMeta);
    EXPECT_EQ(pplib::WideString(L"\\\\\\.\\^\\$\\*\\+\\-\\?\\(\\)\\[\\]\\{\\}\\|\\/"), escaped);

    // Literal Matching Test: Eingebetteter escaped String matcht exakt sich selbst
    pplib::WideString literal = L"foo[bar](1+2)*{x}?$test^/baz\\qux-end.";
    pplib::WideString pattern = L"^" + pplib::RegEx::escape(literal) + L"$";
    EXPECT_TRUE(pplib::RegEx::match(pattern, literal));
    EXPECT_FALSE(pplib::RegEx::match(pattern, L"foo bar 1 2"));
}

} // namespace
