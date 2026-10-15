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
 *       this list of conditions, and the following disclaimer.
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
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <cmath>
#include <limits>
#include <pplib/core/yaml.h>
#include <pplib/core/memfile.h>
#include <pplib/exceptions.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/date.h>
#include <pplib/types/datetime.h>
#include <pplib/types/time.h>
#include <pplib/types/timedelta.h>
#include <pplib/types/timezone.h>
#include <gtest/gtest.h>
#include "pplib-tests.h"

namespace
{

class YamlTest : public ::testing::Test
{
protected:
    YamlTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~YamlTest()
    {
    }
};

// ============================================================================
// Parser: einfache Strukturen
// ============================================================================

TEST_F(YamlTest, ParseSimpleMapping)
{
    pplib::AssocArray data;
    pplib::Yaml::loads(data, "name: Patrick\nage: 42\nactive: true\nempty:\n");
    EXPECT_EQ((size_t)4, data.size());
    EXPECT_EQ(pplib::String("Patrick"), data.getString("name"));
    EXPECT_EQ((int64_t)42, data.getInt("age"));
    EXPECT_TRUE(data.get("active").toBool());
    EXPECT_TRUE(data.get("empty").isNull());
}

TEST_F(YamlTest, ParseNestedMapping)
{
    pplib::Variant v = pplib::Yaml::loads("root:\n  child:\n    key: value\n  other: 1\nsecond: 2\n");
    ASSERT_TRUE(v.isAssocArray());
    const pplib::AssocArray& doc = v.toAssocArray();
    EXPECT_EQ((size_t)2, doc.size());
    ASSERT_TRUE(doc.get("root").isAssocArray());
    const pplib::AssocArray& root = doc.get("root").toAssocArray();
    EXPECT_EQ((size_t)2, root.size());
    ASSERT_TRUE(root.get("child").isAssocArray());
    const pplib::AssocArray& child = root.get("child").toAssocArray();
    EXPECT_EQ(pplib::String("value"), child.getString("key"));
    EXPECT_EQ((int64_t)1, root.get("other").toInt64());
    EXPECT_EQ((int64_t)2, doc.get("second").toInt64());
}

TEST_F(YamlTest, ParseSequence)
{
    pplib::VariantArray arr;
    pplib::Yaml::loads(arr, "- 1\n- 2\n- text\n");
    EXPECT_EQ((size_t)3, arr.size());
    EXPECT_EQ((int64_t)1, arr[0].toInt64());
    EXPECT_EQ((int64_t)2, arr[1].toInt64());
    EXPECT_EQ(pplib::String("text"), arr[2].toString());
}

TEST_F(YamlTest, ParseSequenceUnderKeySameIndent)
{
    pplib::Variant v = pplib::Yaml::loads("list:\n- 1\n- 2\nafter: 3\n");
    ASSERT_TRUE(v.isAssocArray());
    const pplib::AssocArray& root = v.toAssocArray();
    EXPECT_EQ((size_t)2, root.size());
    ASSERT_TRUE(root.get("list").isVariantArray());
    EXPECT_EQ((size_t)2, root.get("list").toVariantArray().size());
    EXPECT_EQ((int64_t)3, root.get("after").toInt64());
}

TEST_F(YamlTest, ParseSequenceOfMappings)
{
    pplib::Variant v = pplib::Yaml::loads("- name: a\n  id: 1\n- name: b\n  id: 2\n");
    ASSERT_TRUE(v.isVariantArray());
    const pplib::VariantArray& arr = v.toVariantArray();
    EXPECT_EQ((size_t)2, arr.size());
    ASSERT_TRUE(arr[0].isAssocArray());
    EXPECT_EQ(pplib::String("a"), arr[0].toAssocArray().getString("name"));
    EXPECT_EQ((int64_t)2, arr[1].toAssocArray().get("id").toInt64());
}

TEST_F(YamlTest, ParseCommentsAndBlanks)
{
    pplib::Variant v = pplib::Yaml::loads("# comment\n\nkey: value # trailing\n# end\n");
    ASSERT_TRUE(v.isAssocArray());
    EXPECT_EQ((size_t)1, v.toAssocArray().size());
    EXPECT_EQ(pplib::String("value"), v.toAssocArray().getString("key"));
}

TEST_F(YamlTest, ParseDocMarker)
{
    pplib::Variant v = pplib::Yaml::loads("---\nkey: value\n");
    ASSERT_TRUE(v.isAssocArray());
    EXPECT_EQ(pplib::String("value"), v.toAssocArray().getString("key"));
}

TEST_F(YamlTest, ParseMultiLinePlainScalar)
{
    pplib::Variant v = pplib::Yaml::loads("key: first\n  second\n  third\nnext: 1\n");
    ASSERT_TRUE(v.isAssocArray());
    EXPECT_EQ(pplib::String("first second third"), v.toAssocArray().getString("key"));
    EXPECT_EQ((int64_t)1, v.toAssocArray().get("next").toInt64());
}

// ============================================================================
// Parser: Skalar-Typen (parametrisiert)
// ============================================================================

struct ScalarCase
{
    const char* yaml;
    pplib::Variant::DataType type;
};

class YamlScalarTest : public YamlTest, public ::testing::WithParamInterface<ScalarCase>
{
};

TEST_P(YamlScalarTest, ResolveType)
{
    ScalarCase c = GetParam();
    pplib::Variant v = pplib::Yaml::loads(std::string("key: ") + c.yaml);
    ASSERT_TRUE(v.isAssocArray());
    const pplib::Variant& val = v.toAssocArray().get("key");
    EXPECT_EQ(c.type, val.type()) << "yaml value: " << c.yaml;
}

INSTANTIATE_TEST_SUITE_P(
    ScalarTypes, YamlScalarTest,
    ::testing::Values(ScalarCase{"null", pplib::Variant::TYPE_NULL}, ScalarCase{"~", pplib::Variant::TYPE_NULL},
                      ScalarCase{"", pplib::Variant::TYPE_NULL}, ScalarCase{"true", pplib::Variant::TYPE_BOOL},
                      ScalarCase{"False", pplib::Variant::TYPE_BOOL}, ScalarCase{"TRUE", pplib::Variant::TYPE_BOOL},
                      ScalarCase{"42", pplib::Variant::TYPE_INT64}, ScalarCase{"-17", pplib::Variant::TYPE_INT64},
                      ScalarCase{"+9", pplib::Variant::TYPE_INT64}, ScalarCase{"0x1F", pplib::Variant::TYPE_INT64},
                      ScalarCase{"0o17", pplib::Variant::TYPE_INT64}, ScalarCase{"1_000", pplib::Variant::TYPE_INT64},
                      ScalarCase{"3.14", pplib::Variant::TYPE_DOUBLE}, ScalarCase{"-2.5e3", pplib::Variant::TYPE_DOUBLE},
                      ScalarCase{".inf", pplib::Variant::TYPE_DOUBLE}, ScalarCase{"-.Inf", pplib::Variant::TYPE_DOUBLE},
                      ScalarCase{".nan", pplib::Variant::TYPE_DOUBLE}, ScalarCase{"1e5", pplib::Variant::TYPE_DOUBLE},
                      ScalarCase{"2021-01-02", pplib::Variant::TYPE_DATE},
                      ScalarCase{"2021-01-02T03:04:05", pplib::Variant::TYPE_DATETIME},
                      ScalarCase{"2021-01-02T03:04:05.123456", pplib::Variant::TYPE_DATETIME},
                      ScalarCase{"03:04:05", pplib::Variant::TYPE_TIME}, ScalarCase{"03:04", pplib::Variant::TYPE_TIME},
                      ScalarCase{"hello world", pplib::Variant::TYPE_STRING},
                      ScalarCase{"12.34.56", pplib::Variant::TYPE_STRING},
                      ScalarCase{"2021-13-45", pplib::Variant::TYPE_STRING}, // kein gültiges Datum
                      ScalarCase{"1.5.2", pplib::Variant::TYPE_STRING}));

TEST_F(YamlTest, ScalarValues)
{
    pplib::Variant v = pplib::Yaml::loads("i: 42\nf: 3.14\nt: true\nn: null\ns: hallo\n");
    const pplib::AssocArray& d = v.toAssocArray();
    EXPECT_EQ((int64_t)42, d.get("i").toInt64());
    EXPECT_DOUBLE_EQ(3.14, d.get("f").toDouble());
    EXPECT_TRUE(d.get("t").toBool());
    EXPECT_TRUE(d.get("n").isNull());
    EXPECT_EQ(pplib::String("hallo"), d.getString("s"));

    pplib::Variant inf = pplib::Yaml::loads("key: .inf").toAssocArray().get("key");
    EXPECT_TRUE(std::isinf(inf.toDouble()));
    EXPECT_TRUE(inf.toDouble() > 0);
    pplib::Variant nan = pplib::Yaml::loads("key: .nan").toAssocArray().get("key");
    EXPECT_TRUE(std::isnan(nan.toDouble()));
}

TEST_F(YamlTest, DateDateTimeTimeValues)
{
    pplib::Variant v = pplib::Yaml::loads("d: 2021-01-02\ndt: 2021-01-02T03:04:05\nt: 03:04:05\n");
    const pplib::AssocArray& d = v.toAssocArray();
    EXPECT_EQ((int64_t)2021, d.get("d").toDate().year());
    EXPECT_EQ(pplib::String("2021-01-02"), d.get("dt").toDateTime().getDate());
    EXPECT_EQ((int64_t)3, d.get("t").toTime().hour());
    EXPECT_EQ((int64_t)4, d.get("t").toTime().minute());
}

// ============================================================================
// Parser: Strings und Escapes
// ============================================================================

TEST_F(YamlTest, QuotedStringsAndEscapes)
{
    pplib::Variant v =
        pplib::Yaml::loads("a: \"line\\nbreak\\ttab\\\"quote\\\\slash\\u0041\\x42\"\nb: 'single ''escaped'' ok'\n");
    const pplib::AssocArray& d = v.toAssocArray();
    EXPECT_EQ(pplib::String("line\nbreak\ttab\"quote\\slashAB"), d.getString("a"));
    EXPECT_EQ(pplib::String("single 'escaped' ok"), d.getString("b"));
}

TEST_F(YamlTest, QuotedStringPreservesTypeLookalikes)
{
    pplib::Variant v = pplib::Yaml::loads("a: \"42\"\nb: \"true\"\nc: \"null\"\nd: \"2021-01-02\"\n");
    const pplib::AssocArray& d = v.toAssocArray();
    EXPECT_EQ(pplib::Variant::TYPE_STRING, d.get("a").type());
    EXPECT_EQ(pplib::Variant::TYPE_STRING, d.get("b").type());
    EXPECT_EQ(pplib::Variant::TYPE_STRING, d.get("c").type());
    EXPECT_EQ(pplib::Variant::TYPE_STRING, d.get("d").type());
    EXPECT_EQ(pplib::String("42"), d.getString("a"));
}

TEST_F(YamlTest, MultilineQuotedStringFolding)
{
    pplib::Variant v = pplib::Yaml::loads("key: \"first\n  second\"\nnext: 1\n");
    EXPECT_EQ(pplib::String("first second"), v.toAssocArray().getString("key"));
    EXPECT_EQ((int64_t)1, v.toAssocArray().get("next").toInt64());
}

// ============================================================================
// Parser: Block-Scalar
// ============================================================================

TEST_F(YamlTest, BlockScalarLiteralClip)
{
    pplib::Variant v = pplib::Yaml::loads("key: |\n  line1\n  line2\nnext: 1\n");
    EXPECT_EQ(pplib::String("line1\nline2\n"), v.toAssocArray().getString("key"));
    EXPECT_EQ((int64_t)1, v.toAssocArray().get("next").toInt64());
}

TEST_F(YamlTest, BlockScalarStripAndKeep)
{
    pplib::Variant v = pplib::Yaml::loads("a: |-\n  x\n  y\nb: |+\n  x\n\nnext: 1\n");
    EXPECT_EQ(pplib::String("x\ny"), v.toAssocArray().getString("a"));
    EXPECT_EQ(pplib::String("x\n\n"), v.toAssocArray().getString("b"));
    EXPECT_EQ((int64_t)1, v.toAssocArray().get("next").toInt64());
}

TEST_F(YamlTest, BlockScalarFolded)
{
    pplib::Variant v = pplib::Yaml::loads("key: >\n  line a\n  line b\n\n  new paragraph\nnext: 1\n");
    EXPECT_EQ(pplib::String("line a line b\nnew paragraph\n"), v.toAssocArray().getString("key"));
    EXPECT_EQ((int64_t)1, v.toAssocArray().get("next").toInt64());
}

// ============================================================================
// Parser: Flow-Strukturen
// ============================================================================

TEST_F(YamlTest, FlowCollections)
{
    pplib::Variant v = pplib::Yaml::loads("seq: [1, 2, \"three\"]\nmap: {a: 1, b: x}\nempty1: []\nempty2: {}\n");
    const pplib::AssocArray& d = v.toAssocArray();
    ASSERT_TRUE(d.get("seq").isVariantArray());
    EXPECT_EQ((size_t)3, d.get("seq").toVariantArray().size());
    EXPECT_EQ(pplib::String("three"), d.get("seq").toVariantArray()[2].toString());
    ASSERT_TRUE(d.get("map").isAssocArray());
    EXPECT_EQ((int64_t)1, d.get("map").toAssocArray().get("a").toInt64());
    EXPECT_EQ(pplib::String("x"), d.get("map").toAssocArray().getString("b"));
    EXPECT_EQ((size_t)0, d.get("empty1").toVariantArray().size());
    EXPECT_EQ((size_t)0, d.get("empty2").toAssocArray().size());
}

TEST_F(YamlTest, RootScalar)
{
    pplib::Variant v = pplib::Yaml::loads("just a text\n");
    EXPECT_EQ(pplib::Variant::TYPE_STRING, v.type());
    EXPECT_EQ(pplib::String("just a text"), v.toString());
    pplib::Variant n = pplib::Yaml::loads("42\n");
    EXPECT_EQ((int64_t)42, n.toInt64());
}

// ============================================================================
// Parser: Tags, Anchors, Aliase
// ============================================================================

TEST_F(YamlTest, Tags)
{
    pplib::Variant v = pplib::Yaml::loads("s: !!str 42\ni: !!int 17\nf: !!float 1e3\nb: !!bool true\nn: !!null x\n"
                                          "bin: !!binary SGVsbG8=\n");
    const pplib::AssocArray& d = v.toAssocArray();
    EXPECT_EQ(pplib::Variant::TYPE_STRING, d.get("s").type());
    EXPECT_EQ(pplib::String("42"), d.getString("s"));
    EXPECT_EQ((int64_t)17, d.get("i").toInt64());
    EXPECT_DOUBLE_EQ(1000.0, d.get("f").toDouble());
    EXPECT_TRUE(d.get("b").toBool());
    EXPECT_TRUE(d.get("n").isNull());
    ASSERT_TRUE(d.get("bin").isByteArray());
    pplib::ByteArray ba = d.get("bin").toByteArray();
    EXPECT_EQ((size_t)5, ba.size());
    EXPECT_EQ(0, memcmp(ba.ptr(), "Hello", 5));
}

TEST_F(YamlTest, PplibTags)
{
    pplib::Variant v = pplib::Yaml::loads("t: !Time 12:34:56\nd: !TimeDelta 1:02:03:04.000005\nz: !TimeZone +01:00\n");
    const pplib::AssocArray& d = v.toAssocArray();
    ASSERT_TRUE(d.get("t").isTime());
    EXPECT_EQ((int64_t)12, d.get("t").toTime().hour());
    ASSERT_TRUE(d.get("d").isTimeDelta());
    EXPECT_EQ(pplib::String("1:02:03:04.000005"), d.get("d").toTimeDelta().toString());
    ASSERT_TRUE(d.get("z").isTimeZone());
}

TEST_F(YamlTest, AnchorsAndAliases)
{
    pplib::Variant v = pplib::Yaml::loads("base: &anchor value\nref: *anchor\n");
    EXPECT_EQ(pplib::String("value"), v.toAssocArray().getString("ref"));

    pplib::Variant v2 = pplib::Yaml::loads("list: &l [1, 2]\nagain: *l\n");
    ASSERT_TRUE(v2.toAssocArray().get("again").isVariantArray());
    EXPECT_EQ((size_t)2, v2.toAssocArray().get("again").toVariantArray().size());

    EXPECT_THROW(pplib::Yaml::loads("a: *missing\n"), pplib::ItemNotFoundException);
}

TEST_F(YamlTest, UnknownTagIgnored)
{
    pplib::Variant v = pplib::Yaml::loads("key: !custom some text\n");
    EXPECT_EQ(pplib::String("some text"), v.toAssocArray().getString("key"));
}

// ============================================================================
// Parser: Fehlerfälle
// ============================================================================

struct ErrorCase
{
    const char* yaml;
};

class YamlErrorTest : public YamlTest, public ::testing::WithParamInterface<ErrorCase>
{
};

TEST_P(YamlErrorTest, ThrowsException)
{
    EXPECT_THROW(pplib::Yaml::loads(GetParam().yaml), pplib::Exception);
}

INSTANTIATE_TEST_SUITE_P(
    ErrorCases, YamlErrorTest,
    ::testing::Values(
        // ErrorCase{"key value without colon\n"},  // wird als Plain-Scalar-Document interpretiert (legal)
        ErrorCase{"key: 1\n  bad_indent: 2\n"}, ErrorCase{"key: value\n---\nother: 1\n"},
        ErrorCase{"seq: [1, 2\n"}, ErrorCase{"map: {a: 1\n"}, ErrorCase{"a: \"unterminated\n"},
        ErrorCase{"a: 'unterminated\n"}, ErrorCase{"a: \"bad \\x escape\"\n"}, ErrorCase{"a: !!int notanumber\n"},
        ErrorCase{"a: !!bool maybe\n"}, ErrorCase{"a: !!binary ###not-base64###\n"}, ErrorCase{"a: *undefined\n"},
        ErrorCase{""}, ErrorCase{"   \n#\n"}, ErrorCase{"key: value\n  \tnext: [1,\n"}));

TEST_F(YamlErrorTest, WrongRootType)
{
    pplib::AssocArray obj;
    EXPECT_THROW(pplib::Yaml::loads(obj, "1\n"), pplib::TypeConversionException);
    pplib::VariantArray arr;
    EXPECT_THROW(pplib::Yaml::loads(arr, "key: value\n"), pplib::TypeConversionException);
}

TEST_F(YamlErrorTest, NestingDepthLimit)
{
    std::string yaml;
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < i; j++) yaml += "  ";
        yaml += "k" + std::to_string(i) + ":\n";
    }
    EXPECT_THROW(pplib::Yaml::loads(yaml), pplib::OverflowException);
}

// ============================================================================
// Writer
// ============================================================================

TEST_F(YamlTest, DumpSimpleMapping)
{
    pplib::AssocArray data;
    data.set("name", pplib::Variant(pplib::String("Patrick")));
    data.set("age", pplib::Variant((int64_t)42));
    EXPECT_EQ(pplib::String("age: 42\nname: Patrick\n"), pplib::Yaml::dumps(data));
}

TEST_F(YamlTest, DumpNestedStructures)
{
    pplib::AssocArray child;
    child.set("key", pplib::Variant(pplib::String("value")));
    pplib::AssocArray data;
    data.set("child", pplib::Variant(child));
    pplib::VariantArray arr;
    arr.add(pplib::Variant((int64_t)1));
    arr.add(pplib::Variant(pplib::String("text")));
    data.set("list", pplib::Variant(arr));
    EXPECT_EQ(pplib::String("child:\n  key: value\nlist:\n- 1\n- text\n"), pplib::Yaml::dumps(data));
}

TEST_F(YamlTest, DumpEmptyCollectionsInline)
{
    pplib::AssocArray data;
    data.set("map", pplib::Variant(pplib::AssocArray()));
    data.set("list", pplib::Variant(pplib::VariantArray()));
    EXPECT_EQ(pplib::String("list: []\nmap: {}\n"), pplib::Yaml::dumps(data));
}

TEST_F(YamlTest, DumpQuotsAmbiguousStrings)
{
    pplib::AssocArray data;
    data.set("a", pplib::Variant(pplib::String("42")));
    data.set("b", pplib::Variant(pplib::String("true")));
    data.set("c", pplib::Variant(pplib::String("null")));
    data.set("d", pplib::Variant(pplib::String("2021-01-02")));
    data.set("e", pplib::Variant(pplib::String(" leading space")));
    data.set("f", pplib::Variant(pplib::String("has \"quote\"")));
    data.set("g", pplib::Variant(pplib::String("normal text")));
    pplib::String yaml = pplib::Yaml::dumps(data);
    EXPECT_EQ(pplib::String("a: \"42\"\nb: \"true\"\nc: \"null\"\nd: \"2021-01-02\"\n"
                           "e: \" leading space\"\nf: \"has \\\"quote\\\"\"\ng: normal text\n"),
              yaml);
}

TEST_F(YamlTest, DumpSpecialScalars)
{
    pplib::AssocArray data;
    data.set("nan", pplib::Variant(std::numeric_limits<double>::quiet_NaN()));
    data.set("pinf", pplib::Variant(std::numeric_limits<double>::infinity()));
    data.set("ninf", pplib::Variant(-std::numeric_limits<double>::infinity()));
    data.set("d", pplib::Variant(3.5));
    data.set("n", pplib::Variant(nullptr));
    data.set("b", pplib::Variant(true));
    EXPECT_EQ(pplib::String("b: true\nd: 3.5\nn: null\nnan: .nan\nninf: -.inf\npinf: .inf\n"), pplib::Yaml::dumps(data));
}

TEST_F(YamlTest, DumpBlockScalarForMultilineString)
{
    pplib::AssocArray data;
    data.set("key", pplib::Variant(pplib::String("line1\nline2")));
    EXPECT_EQ(pplib::String("key: |-\n  line1\n  line2\n"), pplib::Yaml::dumps(data));
}

TEST_F(YamlTest, DumpLoadFileRoundTrip)
{
    pplib::AssocArray data;
    data.set("x", pplib::Variant((int64_t)7));
    pplib::MemFile out;
    pplib::Yaml::dump(out, data);
    size_t size = out.tell();
    pplib::MemFile in(const_cast<char*>(reinterpret_cast<const char*>(out.map(0, size))), size, false);
    pplib::AssocArray back;
    pplib::Yaml::load(back, in);
    EXPECT_EQ((int64_t)7, back.get("x").toInt64());
}

// ============================================================================
// Round-Trip über alle Datentypen
// ============================================================================

TEST_F(YamlTest, RoundTripStringTypes)
{
    pplib::AssocArray data;
    data.set("plain", pplib::Variant(pplib::String("hello world")));
    data.set("number", pplib::Variant(pplib::String("42")));
    data.set("bool", pplib::Variant(pplib::String("true")));
    data.set("date", pplib::Variant(pplib::String("2021-01-02")));
    data.set("time", pplib::Variant(pplib::String("03:04:05")));
    data.set("escapes", pplib::Variant(pplib::String("a\nb\tc\\d\"e")));
    data.set("umlaut", pplib::Variant(pplib::String("Ärger mit Straße ✓")));
    data.set("multiline", pplib::Variant(pplib::String("line1\nline2\nline3")));

    pplib::String yaml = pplib::Yaml::dumps(data);
    pplib::Variant back = pplib::Yaml::loads(yaml);
    ASSERT_TRUE(back.isAssocArray());
    const pplib::AssocArray& d = back.toAssocArray();
    EXPECT_EQ(pplib::String("hello world"), d.getString("plain"));
    EXPECT_EQ(pplib::Variant::TYPE_STRING, d.get("number").type());
    EXPECT_EQ(pplib::String("42"), d.getString("number"));
    EXPECT_EQ(pplib::String("true"), d.getString("bool"));
    EXPECT_EQ(pplib::String("2021-01-02"), d.getString("date"));
    EXPECT_EQ(pplib::String("03:04:05"), d.getString("time"));
    EXPECT_EQ(pplib::String("a\nb\tc\\d\"e"), d.getString("escapes"));
    EXPECT_EQ(pplib::String("Ärger mit Straße ✓"), d.getString("umlaut"));
    EXPECT_EQ(pplib::String("line1\nline2\nline3"), d.getString("multiline"));
}

TEST_F(YamlTest, RoundTripBinaryAndTemporalTypes)
{
    const char raw[] = "\x00\x01\xFE\xFFHello";
    pplib::ByteArray ba(raw, sizeof(raw) - 1);

    pplib::AssocArray data;
    data.set("bin", pplib::Variant(ba));
    data.set("date", pplib::Variant(pplib::Date(2021, 1, 2)));
    data.set("datetime", pplib::Variant(pplib::DateTime(2021, 1, 2, 3, 4, 5, 6, pplib::TimeZone(0, "UTC"))));
    data.set("time", pplib::Variant(pplib::Time(12, 34, 56, 789010)));
    data.set("delta", pplib::Variant(pplib::TimeDelta("1:02:03:04.000005")));
    data.set("zone", pplib::Variant(pplib::TimeZone(60, "")));

    pplib::String yaml = pplib::Yaml::dumps(data);
    pplib::Variant back = pplib::Yaml::loads(yaml);
    ASSERT_TRUE(back.isAssocArray());
    const pplib::AssocArray& d = back.toAssocArray();

    ASSERT_TRUE(d.get("bin").isByteArray());
    pplib::ByteArray rb = d.get("bin").toByteArray();
    EXPECT_EQ(sizeof(raw) - 1, rb.size());
    EXPECT_EQ(0, memcmp(raw, rb.ptr(), sizeof(raw) - 1));

    ASSERT_TRUE(d.get("date").isDate());
    EXPECT_EQ((int64_t)2021, d.get("date").toDate().year());
    ASSERT_TRUE(d.get("datetime").isDateTime());
    EXPECT_EQ(data.get("datetime").toDateTime().getISO8601withUsec(), d.get("datetime").toDateTime().getISO8601withUsec());
    ASSERT_TRUE(d.get("time").isTime());
    EXPECT_EQ((int64_t)12, d.get("time").toTime().hour());
    EXPECT_EQ((int64_t)789010, d.get("time").toTime().microsecond());
    ASSERT_TRUE(d.get("delta").isTimeDelta());
    ASSERT_TRUE(d.get("zone").isTimeZone());
}

TEST_F(YamlTest, RoundTripNestedMixed)
{
    pplib::AssocArray inner;
    inner.set("id", pplib::Variant((int64_t)1));
    inner.set("tags", pplib::Variant(pplib::String("a,b")));
    pplib::VariantArray items;
    items.add(pplib::Variant(inner));
    items.add(pplib::Variant((int64_t)2));
    pplib::AssocArray data;
    data.set("items", pplib::Variant(items));
    data.set("meta", pplib::Variant(inner));

    pplib::String yaml = pplib::Yaml::dumps(data);
    pplib::Variant back = pplib::Yaml::loads(yaml);
    ASSERT_TRUE(back.isAssocArray());
    const pplib::AssocArray& d = back.toAssocArray();
    ASSERT_TRUE(d.get("items").isVariantArray());
    EXPECT_EQ((size_t)2, d.get("items").toVariantArray().size());
    EXPECT_EQ(pplib::String("a,b"), d.get("items").toVariantArray()[0].toAssocArray().getString("tags"));
    EXPECT_EQ((int64_t)2, d.get("items").toVariantArray()[1].toInt64());
    EXPECT_EQ((int64_t)1, d.get("meta").toAssocArray().get("id").toInt64());
}

TEST_F(YamlTest, WriterDepthLimit)
{
    pplib::Variant nested;
    for (int i = 0; i < 200; i++) {
        pplib::AssocArray outer;
        outer.set("k", pplib::Variant(nested));
        nested = pplib::Variant(outer);
    }
    EXPECT_THROW(pplib::Yaml::dumps(nested), pplib::OverflowException);
}

} // namespace
