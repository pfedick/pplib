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
#include <pplib/core/file.h>
#include <pplib/core/memfile.h>
#include <pplib/exceptions.h>
#include <pplib/types/array.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/date.h>
#include <pplib/types/datetime.h>
#include <pplib/types/time.h>
#include <pplib/types/timedelta.h>
#include <pplib/types/timezone.h>
#include <pplib/types/widestring.h>
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
                      ScalarCase{"2021-02-30", pplib::Variant::TYPE_STRING}, // existierender Tag
                      ScalarCase{"2021-01-02T", pplib::Variant::TYPE_STRING},
                      ScalarCase{"1.5.2", pplib::Variant::TYPE_STRING},
                      ScalarCase{"yes", pplib::Variant::TYPE_STRING},   // YAML 1.2 core: kein Boolean
                      ScalarCase{"on", pplib::Variant::TYPE_STRING},
                      // Datums- und Zeitformen
                      ScalarCase{"2024-06-05T12:34:56Z", pplib::Variant::TYPE_DATETIME},
                      ScalarCase{"2024-06-05 12:34:56", pplib::Variant::TYPE_DATETIME},
                      ScalarCase{"2024-06-05T12:34:56.789Z", pplib::Variant::TYPE_DATETIME},
                      ScalarCase{"12:34:56.789", pplib::Variant::TYPE_TIME},
                      // numerische Grenzen: int64 reicht, danach Double ( Präzisionsverlust)
                      ScalarCase{"9223372036854775807", pplib::Variant::TYPE_INT64},
                      ScalarCase{"-9223372036854775808", pplib::Variant::TYPE_INT64},
                      ScalarCase{"9223372036854775808", pplib::Variant::TYPE_DOUBLE},
                      ScalarCase{"-9223372036854775809", pplib::Variant::TYPE_DOUBLE},
                      ScalarCase{"99999999999999999999999999", pplib::Variant::TYPE_DOUBLE},
                      ScalarCase{"4111111111111111", pplib::Variant::TYPE_INT64},
                      ScalarCase{"DE89370400560556190100", pplib::Variant::TYPE_STRING},
                      ScalarCase{"007", pplib::Variant::TYPE_INT64},
                      ScalarCase{"-0", pplib::Variant::TYPE_INT64},
                      ScalarCase{"0b101", pplib::Variant::TYPE_INT64},
                      ScalarCase{"1_000_000", pplib::Variant::TYPE_INT64},
                      ScalarCase{"1_000.5", pplib::Variant::TYPE_DOUBLE},
                      ScalarCase{"1.0", pplib::Variant::TYPE_DOUBLE},
                      ScalarCase{"1.5e-3", pplib::Variant::TYPE_DOUBLE},
                      // undekorierte / fehlerhafte Zahlen bleiben Strings
                      ScalarCase{"_123", pplib::Variant::TYPE_STRING},
                      ScalarCase{"123_", pplib::Variant::TYPE_STRING},
                      ScalarCase{"1__2", pplib::Variant::TYPE_STRING},
                      ScalarCase{"0x_FF", pplib::Variant::TYPE_STRING},
                      ScalarCase{"0x", pplib::Variant::TYPE_STRING},
                      ScalarCase{"1e", pplib::Variant::TYPE_STRING},
                      ScalarCase{"1e+", pplib::Variant::TYPE_STRING},
                      ScalarCase{"-", pplib::Variant::TYPE_STRING}));

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
    pplib::Variant v = pplib::Yaml::loads("t: !Time 12:34:56\nd: !TimeDelta 1:02:03:04.000005\nz: !TimeZone +01:00\n"
                                          "z2: !TimeZone +0130\nz3: !TimeZone -0030\n");
    const pplib::AssocArray& d = v.toAssocArray();
    ASSERT_TRUE(d.get("t").isTime());
    EXPECT_EQ((int64_t)12, d.get("t").toTime().hour());
    ASSERT_TRUE(d.get("d").isTimeDelta());
    EXPECT_EQ(pplib::String("1:02:03:04.000005"), d.get("d").toTimeDelta().toString());
    ASSERT_TRUE(d.get("z").isTimeZone());
    EXPECT_EQ(60, d.get("z").toTimeZone().offsetMinutes());
    // HHMM ohne Trennzeichen
    EXPECT_EQ(90, d.get("z2").toTimeZone().offsetMinutes());
    EXPECT_EQ(-30, d.get("z3").toTimeZone().offsetMinutes());
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
        ErrorCase{"a: !TimeZone 123\n"}, ErrorCase{"a: !TimeZone abcd\n"}, ErrorCase{"a: !TimeZone 30\n"},
        ErrorCase{""}, ErrorCase{"   \n#\n"}, ErrorCase{"key: value\n  \tnext: [1,\n"},
        // ':' an Wertposition in einer Flow-Sequenz: führte früher zu einer Endlosschleife
        ErrorCase{"key: [1, 2\nb: 3\n"}, ErrorCase{"key: [a: b]\n"}, ErrorCase{"key: [1,\nb: 3]\n"},
        ErrorCase{"key: {a: [1,\nb: 2]}\n"}));

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

// ============================================================================
// Testdaten-Dateien aus tests/testdata
// ============================================================================

TEST_F(YamlTest, Test1FileScalarTypes)
{
    pplib::File file("testdata/test1.yaml");
    pplib::Variant v = pplib::Yaml::load(file);
    ASSERT_TRUE(v.isAssocArray());
    const pplib::AssocArray& d = v.toAssocArray();
    EXPECT_EQ((size_t)18, d.size());
    EXPECT_EQ((int64_t)12345, d.get("integer").toInt64());
    EXPECT_EQ(pplib::String("hello world"), d.getString("string"));
    EXPECT_TRUE(d.get("boolean_true").toBool());
    EXPECT_FALSE(d.get("boolean_false").toBool());
    EXPECT_TRUE(d.get("null_value").isNull());
    EXPECT_DOUBLE_EQ(3.14159, d.get("float").toDouble());
    EXPECT_DOUBLE_EQ(3.141592653589793, d.get("double").toDouble());
}

TEST_F(YamlTest, Test1FileBinaryTemporalAndBlockScalar)
{
    pplib::File file("testdata/test1.yaml");
    pplib::Variant v = pplib::Yaml::load(file);
    const pplib::AssocArray& d = v.toAssocArray();

    pplib::ByteArray bin = d.get("binary").toByteArray();
    EXPECT_EQ((size_t)11, bin.size());
    EXPECT_EQ(0, memcmp(bin.ptr(), "Hello world", 11));

    pplib::ByteArray longBin = d.get("long_binary").toByteArray();
    EXPECT_EQ((size_t)53, longBin.size());
    EXPECT_EQ(0, memcmp(longBin.ptr(), "Hello world world world world world world world world", 53));

    EXPECT_EQ((int64_t)2024, d.get("date").toDate().year());
    EXPECT_EQ(pplib::String("2024-06-05"), d.get("date").toDate().toString());
    EXPECT_EQ(pplib::String("2024-06-05"), d.get("datetime").toDateTime().getDate());
    EXPECT_EQ(pplib::String("12:34:56"), d.get("datetime").toDateTime().getTime());
    EXPECT_EQ((int64_t)34, d.get("time").toTime().minute());
    EXPECT_EQ(pplib::String("12:34:56.000000"), d.get("time").toTime().toString());

    EXPECT_EQ(pplib::String("This is a\nmultiline string\nexample.\n"), d.getString("multiline_string"));
}

TEST_F(YamlTest, Test1FileNestedCollections)
{
    pplib::File file("testdata/test1.yaml");
    pplib::Variant v = pplib::Yaml::load(file);
    const pplib::AssocArray& d = v.toAssocArray();

    EXPECT_EQ((size_t)3, d.get("integer_array").toVariantArray().size());
    EXPECT_EQ((int64_t)1, d.get("integer_array").toVariantArray()[0].toInt64());
    EXPECT_EQ(pplib::String("blue"), d.get("string_array").toVariantArray()[2].toString());

    const pplib::AssocArray& obj = d.get("object").toAssocArray();
    EXPECT_EQ((size_t)5, obj.size());
    EXPECT_EQ(pplib::String("value2"), obj.getString("key2"));

    const pplib::VariantArray& withObjects = obj.get("sub_array_with_objects").toVariantArray();
    EXPECT_EQ((size_t)2, withObjects.size());
    EXPECT_EQ(pplib::String("sub_value2"), withObjects[0].toAssocArray().getString("sub_key2"));
    EXPECT_EQ(pplib::String("sub_value4"), withObjects[1].toAssocArray().getString("sub_key4"));

    const pplib::VariantArray& sub = obj.get("sub_array").toVariantArray();
    EXPECT_EQ((size_t)4, sub.size());
    EXPECT_EQ(pplib::String("item1"), sub[0].toString());
    EXPECT_TRUE(sub[1].isNull());
    ASSERT_TRUE(sub[3].isAssocArray());
    EXPECT_EQ(pplib::String("sub_value1"),
              sub[3].toAssocArray().get("sub_object").toAssocArray().getString("sub_key1"));

    const pplib::VariantArray& users = d.get("users").toVariantArray();
    EXPECT_EQ((size_t)2, users.size());
    EXPECT_EQ(pplib::String("alice@example.com"), users[0].toAssocArray().getString("email"));
    EXPECT_EQ((int64_t)25, users[1].toAssocArray().get("age").toInt64());
}

TEST_F(YamlTest, Test2FileRoundTripAndTypedOverloads)
{
    pplib::File file("testdata/test2.yaml");
    pplib::Variant v = pplib::Yaml::load(file);
    ASSERT_TRUE(v.isAssocArray());
    EXPECT_EQ((size_t)18, v.toAssocArray().size());

    // Dump -> Load -> Dump: unveränderte Ausgabe
    pplib::String first = pplib::Yaml::dumps(v);
    pplib::Variant again = pplib::Yaml::loads(first);
    EXPECT_EQ(first, pplib::Yaml::dumps(again));

    // Load direkt in ein AssocArray
    pplib::File file2("testdata/test2.yaml");
    pplib::AssocArray obj;
    pplib::Yaml::load(obj, file2);
    EXPECT_EQ((size_t)18, obj.size());

    // Root passt nicht zum Zieltyp
    pplib::File file3("testdata/test2.yaml");
    pplib::VariantArray arr;
    EXPECT_THROW(pplib::Yaml::load(arr, file3), pplib::TypeConversionException);
}

// ============================================================================
// Parser: weitere Grenzfälle
// ============================================================================

TEST_F(YamlTest, NestedBlockSequences)
{
    // "- - 1" ist eine verschachtelte Sequenz, kein String
    pplib::Variant v = pplib::Yaml::loads("list:\n  - - 1\n    - 2\n  - - x\n");
    const pplib::VariantArray& list = v.toAssocArray().get("list").toVariantArray();
    ASSERT_EQ((size_t)2, list.size());
    ASSERT_TRUE(list[0].isVariantArray());
    EXPECT_EQ((size_t)2, list[0].toVariantArray().size());
    EXPECT_EQ((int64_t)2, list[0].toVariantArray()[1].toInt64());
    ASSERT_TRUE(list[1].isVariantArray());
    EXPECT_EQ(pplib::String("x"), list[1].toVariantArray()[0].toString());
}

TEST_F(YamlTest, EmptyValuesAndComments)
{
    pplib::Variant v = pplib::Yaml::loads("a: # Kommentar\nb:\nc: ''\n");
    const pplib::AssocArray& d = v.toAssocArray();
    EXPECT_EQ((size_t)3, d.size());
    EXPECT_TRUE(d.get("a").isNull());
    EXPECT_TRUE(d.get("b").isNull());
    EXPECT_EQ(pplib::Variant::TYPE_STRING, d.get("c").type());
    EXPECT_EQ(pplib::String(""), d.getString("c"));
}

TEST_F(YamlTest, QuotedKeys)
{
    pplib::Variant v = pplib::Yaml::loads("\"quoted key\": 1\n'single key': 2\n\"a: b\": 3\n");
    const pplib::AssocArray& d = v.toAssocArray();
    EXPECT_EQ((size_t)3, d.size());
    EXPECT_EQ((int64_t)1, d.get("quoted key").toInt64());
    EXPECT_EQ((int64_t)2, d.get("single key").toInt64());
    EXPECT_EQ((int64_t)3, d.get("a: b").toInt64());

    pplib::Variant seq = pplib::Yaml::loads("- \"3\": x\n");
    EXPECT_EQ((size_t)1, seq.toVariantArray().size());
    EXPECT_EQ(pplib::String("x"), seq.toVariantArray()[0].toAssocArray().getString("3"));
}

TEST_F(YamlTest, FlowAnchorsTagsAndMultilineQuoted)
{
    pplib::Variant v = pplib::Yaml::loads("a: &x [1, 2]\nb: *x\nc: [!!str 42, \"first\n  second\"]\n");
    const pplib::AssocArray& d = v.toAssocArray();
    EXPECT_EQ((size_t)2, d.get("b").toVariantArray().size());
    EXPECT_EQ((int64_t)2, d.get("b").toVariantArray()[1].toInt64());
    EXPECT_EQ(pplib::Variant::TYPE_STRING, d.get("c").toVariantArray()[0].type());
    EXPECT_EQ(pplib::String("42"), d.get("c").toVariantArray()[0].toString());
    EXPECT_EQ(pplib::String("first second"), d.get("c").toVariantArray()[1].toString());
}

TEST_F(YamlTest, RootFlowContainers)
{
    EXPECT_EQ(pplib::Variant::TYPE_ASSOCARRAY, pplib::Yaml::loads("{}\n").type());
    EXPECT_EQ(pplib::Variant::TYPE_VARIANTARRAY, pplib::Yaml::loads("[]\n").type());
    pplib::AssocArray obj;
    pplib::Yaml::loads(obj, "{}\n");
    EXPECT_EQ((size_t)0, obj.size());
    pplib::VariantArray arr;
    pplib::Yaml::loads(arr, "[]\n");
    EXPECT_EQ((size_t)0, arr.size());
}

TEST_F(YamlTest, QuotedNulByte)
{
    pplib::Variant v = pplib::Yaml::loads("a: \"x\\0y\"\n");
    pplib::String s = v.toAssocArray().getString("a");
    EXPECT_EQ((size_t)3, s.length());
    EXPECT_EQ(0, memcmp(s.getPtr(), "x\0y", 3));
}

TEST_F(YamlTest, EscapeMatrix)
{
    pplib::Variant v = pplib::Yaml::loads(
        "a: \"AB\\u00E9\\U0001F600\\x1B\\N\\_\\xA0\\u2028\\u2029\\a\\b\\v\\f\\r\\/\\\\\\\"\\t\"\n");
    // '\_' ist laut YAML 1.1 das Escape für U+00A0 (No-Break-Space), nicht für '_'
    EXPECT_EQ(pplib::String("AB" "\xC3\xA9" "\xF0\x9F\x98\x80" "\x1B" "\xC2\x85" "\xC2\xA0" "\xC2\xA0"
                            "\xE2\x80\xA8" "\xE2\x80\xA9" "\x07\x08\x0B\x0C\x0D" "/\\\"\t"),
              v.toAssocArray().getString("a"));
}

TEST_F(YamlTest, QuotedWhitespaceIsPartOfValue)
{
    pplib::Variant v = pplib::Yaml::loads("a: \" x \"\nb: \"y\\t\"\nc: ' z '\nd: \"x\\t\"\ne: 1 \n");
    ASSERT_TRUE(v.isAssocArray());
    pplib::AssocArray& d = v.toAssocArray();
    EXPECT_EQ(pplib::String(" x "), d.getString("a"));
    EXPECT_EQ(pplib::String("y\t"), d.getString("b"));
    EXPECT_EQ(pplib::String(" z "), d.getString("c"));
    EXPECT_EQ(pplib::String("x\t"), d.getString("d"));
    EXPECT_EQ(1, d.getInt64t("e"));
}

TEST_F(YamlTest, FractionalSecondsAreMicroseconds)
{
    // pplib interpretiert die Nachkommastellen als Mikrosekunden (Bibliotheks-Konvention),
    // nicht als Sekundenanteil: ".789" sind 789 us, ".789000" sind 789000 us.
    pplib::Variant v = pplib::Yaml::loads("a: 2024-06-05T12:34:56.789Z\nb: 12:34:56.789\n");
    EXPECT_EQ(pplib::String("2024-06-05T12:34:56.000789+00:00"),
              v.toAssocArray().get("a").toDateTime().getISO8601withUsec());
    EXPECT_EQ(pplib::String("12:34:56.000789"), v.toAssocArray().get("b").toTime().toString());
}

TEST_F(YamlErrorTest, SpecificExceptionTypes)
{
    EXPECT_THROW(pplib::Yaml::loads("a: \"x\\qy\"\n"), pplib::InvalidEscapeSequenceException);
    EXPECT_THROW(pplib::Yaml::loads("a: \"\\uD800\"\n"), pplib::CharacterEncodingException);
    EXPECT_THROW(pplib::Yaml::loads("a: \"\\U00110000\"\n"), pplib::CharacterEncodingException);
    EXPECT_THROW(pplib::Yaml::loads("key: 1\n\tnext: 2\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a: {[1, 2]: x}\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a: !!timestamp 2021-13-45T00:00:00\n"), pplib::TypeConversionException);
    EXPECT_THROW(pplib::Yaml::loads("a: !Time 99:99:99\n"), pplib::TypeConversionException);
    EXPECT_THROW(pplib::Yaml::loads("a: !TimeZone abc\n"), pplib::TypeConversionException);
    EXPECT_THROW(pplib::Yaml::loads("a: !!binary\n"), pplib::TypeConversionException);
    EXPECT_THROW(pplib::Yaml::dumps(pplib::Variant()), pplib::UnsupportedDataTypeException);
}

// ============================================================================
// Writer: weitere Grenzfälle
// ============================================================================

TEST_F(YamlTest, DumpOversizedNumericStringsStayStrings)
{
    // "Ein String bleibt ein String": auch überlange Ziffernfolgen werden nicht interpretiert
    pplib::AssocArray data;
    data.set("big", pplib::Variant(pplib::String("99999999999999999999999999")));
    data.set("hex", pplib::Variant(pplib::String("0x1F")));
    data.set("iban", pplib::Variant(pplib::String("DE89370400560556190100")));
    data.set("oct", pplib::Variant(pplib::String("017")));
    pplib::String yaml = pplib::Yaml::dumps(data);
    EXPECT_EQ(pplib::String("big: \"99999999999999999999999999\"\nhex: \"0x1F\"\n"
                            "iban: DE89370400560556190100\noct: \"017\"\n"),
              yaml);

    pplib::Variant back = pplib::Yaml::loads(yaml);
    const pplib::AssocArray& d = back.toAssocArray();
    EXPECT_EQ(pplib::Variant::TYPE_STRING, d.get("big").type());
    EXPECT_EQ(pplib::Variant::TYPE_STRING, d.get("hex").type());
    EXPECT_EQ(pplib::Variant::TYPE_STRING, d.get("iban").type());
    EXPECT_EQ(pplib::Variant::TYPE_STRING, d.get("oct").type());
}

TEST_F(YamlTest, DumpReservedIndicatorPrefixesQuoted)
{
    pplib::AssocArray data;
    data.set("alias", pplib::Variant(pplib::String("*ref")));
    data.set("anchor", pplib::Variant(pplib::String("&a")));
    data.set("at", pplib::Variant(pplib::String("@user")));
    data.set("colon", pplib::Variant(pplib::String("a: b")));
    data.set("dash", pplib::Variant(pplib::String("-5x")));
    data.set("flow", pplib::Variant(pplib::String("[x]")));
    data.set("hash", pplib::Variant(pplib::String("x #y")));
    data.set("tag", pplib::Variant(pplib::String("!tag")));
    EXPECT_EQ(pplib::String("alias: \"*ref\"\nanchor: \"&a\"\nat: \"@user\"\ncolon: \"a: b\"\n"
                            "dash: \"-5x\"\nflow: \"[x]\"\nhash: \"x #y\"\ntag: \"!tag\"\n"),
              pplib::Yaml::dumps(data));
}

TEST_F(YamlTest, DumpIntegralDoublesAndExponentials)
{
    pplib::AssocArray data;
    data.set("a", pplib::Variant(5.0));
    data.set("b", pplib::Variant(1e20));
    data.set("c", pplib::Variant(-0.5));
    pplib::String yaml = pplib::Yaml::dumps(data);
    EXPECT_EQ(pplib::String("a: 5.0\nb: 1e+20\nc: -0.5\n"), yaml);
    EXPECT_DOUBLE_EQ(1e20, pplib::Yaml::loads(yaml).toAssocArray().get("b").toDouble());
}

TEST_F(YamlTest, DumpControlCharsInMultilineUseQuoted)
{
    pplib::AssocArray data;
    data.set("key", pplib::Variant(pplib::String("line1\nline2\x01" "tail")));
    EXPECT_EQ(pplib::String("key: \"line1\\nline2\\u0001tail\"\n"), pplib::Yaml::dumps(data));
}

TEST_F(YamlTest, DumpEmptyByteArrayAndWideString)
{
    pplib::AssocArray data;
    data.set("emptybin", pplib::Variant(pplib::ByteArray()));
    data.set("wide", pplib::Variant(pplib::WideString(L"wide")));
    EXPECT_EQ(pplib::String("emptybin: !!binary \"\"\nwide: \"wide\"\n"), pplib::Yaml::dumps(data));
}

TEST_F(YamlTest, DumpClassicArray)
{
    pplib::Array arr;
    arr.add(pplib::String("x"));
    arr.add(pplib::String("42"));
    EXPECT_EQ(pplib::String("- x\n- \"42\"\n"), pplib::Yaml::dumps(pplib::Variant(arr)));
}

TEST_F(YamlTest, DumpNestedSequencesAndEmptyDictsInSequence)
{
    pplib::VariantArray inner;
    inner.add(pplib::Variant((int64_t)1));
    inner.add(pplib::Variant((int64_t)2));
    pplib::VariantArray sub;
    sub.add(pplib::Variant((int64_t)1));
    pplib::AssocArray subDict;
    subDict.set("sub", pplib::Variant(sub));

    pplib::VariantArray list;
    list.add(pplib::Variant(inner));
    list.add(pplib::Variant(pplib::AssocArray()));
    list.add(pplib::Variant(subDict));
    pplib::AssocArray data;
    data.set("list", pplib::Variant(list));

    pplib::String yaml = pplib::Yaml::dumps(data);
    EXPECT_EQ(pplib::String("list:\n-\n  - 1\n  - 2\n- {}\n-\n  sub:\n  - 1\n"), yaml);

    pplib::Variant back = pplib::Yaml::loads(yaml);
    const pplib::VariantArray& backList = back.toAssocArray().get("list").toVariantArray();
    EXPECT_EQ((size_t)3, backList.size());
    EXPECT_EQ((size_t)2, backList[0].toVariantArray().size());
    EXPECT_EQ((size_t)0, backList[1].toAssocArray().size());
    EXPECT_EQ((size_t)1, backList[2].toAssocArray().get("sub").toVariantArray().size());
}

TEST_F(YamlTest, DumpOverloadsToFileAndVariantArray)
{
    pplib::VariantArray arr;
    arr.add(pplib::Variant((int64_t)1));
    arr.add(pplib::Variant(pplib::String("z")));

    pplib::String yaml;
    pplib::Yaml::dumps(yaml, arr);
    EXPECT_EQ(pplib::String("- 1\n- z\n"), yaml);
    EXPECT_EQ(pplib::String("- 1\n- z\n"), pplib::Yaml::dumps(arr));

    pplib::MemFile out;
    pplib::Yaml::dump(out, arr);
    size_t size = out.tell();
    pplib::MemFile in(const_cast<char*>(reinterpret_cast<const char*>(out.map(0, size))), size, false);
    pplib::VariantArray back;
    pplib::Yaml::load(back, in);
    EXPECT_EQ((size_t)2, back.size());
    EXPECT_EQ(pplib::String("z"), back[1].toString());

    pplib::AssocArray inner;
    inner.set("k", pplib::Variant((int64_t)1));
    pplib::AssocArray data;
    data.set("o", pplib::Variant(inner));
    EXPECT_EQ(pplib::String("o:\n    k: 1\n"), pplib::Yaml::dumps(data, 4));
}

// ============================================================================
// Parser: Zeilen- und Skalar-Grenzfälle
// ============================================================================

TEST_F(YamlTest, CarriageReturnOnlyLineEndings)
{
    pplib::Variant v = pplib::Yaml::loads("a: 1\rb: 2\r");
    ASSERT_TRUE(v.isAssocArray());
    EXPECT_EQ((int64_t)1, v.toAssocArray().get("a").toInt64());
    EXPECT_EQ((int64_t)2, v.toAssocArray().get("b").toInt64());
}

TEST_F(YamlTest, LineContinuationEscapeInDoubleQuotes)
{
    // '\' am Zeilenende verbindet die Fortsetzung ohne Leerzeichen
    pplib::Variant v = pplib::Yaml::loads("a: \"abc\\\n   def\"\nb: \"x\n   y\"\n");
    EXPECT_EQ(pplib::String("abcdef"), v.toAssocArray().get("a").toString());
    EXPECT_EQ(pplib::String("x y"), v.toAssocArray().get("b").toString());
}

TEST_F(YamlTest, QuotedContinuationLineStartingWithHashKeepsFoldingSpace)
{
    // Ein '#' am Zeilenanfang innerhalb eines doppeltquotierten Mehrzeilen-Strings
    // ist Inhalt, kein Kommentar: das Faltungs-Leerzeichen muss bleiben
    pplib::Variant v = pplib::Yaml::loads("a: \"Abschnitt 1\n# Ueberschrift\"\n");
    EXPECT_EQ(pplib::String("Abschnitt 1 # Ueberschrift"), v.toAssocArray().get("a").toString());

    pplib::Variant v2 = pplib::Yaml::loads("a: \"abc\\\n#xyz\"\n");
    EXPECT_EQ(pplib::String("abc#xyz"), v2.toAssocArray().get("a").toString());
}

TEST_F(YamlTest, PlainScalarContinuationOverBlankAndCommentLines)
{
    pplib::Variant v = pplib::Yaml::loads("a: first\n\n  second\nb: 2\n");
    EXPECT_EQ(pplib::String("first\nsecond"), v.toAssocArray().get("a").toString());
    EXPECT_EQ((int64_t)2, v.toAssocArray().get("b").toInt64());

    v = pplib::Yaml::loads("c: one\n# Kommentar\n  two\n\n  three\n");
    EXPECT_EQ(pplib::String("one\ntwo\nthree"), v.toAssocArray().get("c").toString());

    // Leerzeile beendet den Skalar, wenn keine tiefer eingerückte Fortsetzung folgt
    v = pplib::Yaml::loads("a: first\n\nb: 2\n");
    EXPECT_EQ(pplib::String("first"), v.toAssocArray().get("a").toString());

    v = pplib::Yaml::loads("a: first\n\n\n");
    EXPECT_EQ(pplib::String("first"), v.toAssocArray().get("a").toString());
}

TEST_F(YamlTest, BlockScalarBlankLinesAndChomping)
{
    pplib::Variant v = pplib::Yaml::loads("a: |\n  l1\n\n  l2\n");
    EXPECT_EQ(pplib::String("l1\n\nl2\n"), v.toAssocArray().get("a").toString());

    v = pplib::Yaml::loads("a: |-\n  l1\n\n  l2\n");
    EXPECT_EQ(pplib::String("l1\n\nl2"), v.toAssocArray().get("a").toString());

    v = pplib::Yaml::loads("a: >\n  f1\n\n  f2\n");
    EXPECT_EQ(pplib::String("f1\nf2\n"), v.toAssocArray().get("a").toString());

    v = pplib::Yaml::loads("a: >-\n  f1\n\n  f2\n");
    EXPECT_EQ(pplib::String("f1\nf2"), v.toAssocArray().get("a").toString());

    v = pplib::Yaml::loads("a: >+\n  f1\n\n  f2\n\n");
    EXPECT_EQ(pplib::String("f1\nf2\n\n"), v.toAssocArray().get("a").toString());

    // Writer: Block-Skalar-Werte werden als |- geschrieben und lesen sich identisch zurück
    pplib::AssocArray data;
    data.set("block", pplib::Variant(pplib::String("a\nb")));
    pplib::String yaml = pplib::Yaml::dumps(data);
    EXPECT_EQ(pplib::String("block: |-\n  a\n  b\n"), yaml);
    EXPECT_EQ(pplib::String("a\nb"), pplib::Yaml::loads(yaml).toAssocArray().get("block").toString());
}

TEST_F(YamlTest, FlowCollectionsMultilineAndComment)
{
    pplib::Variant v = pplib::Yaml::loads("a: [1, # c\n  2]\n");
    EXPECT_EQ((size_t)2, v.toAssocArray().get("a").toVariantArray().size());

    v = pplib::Yaml::loads("a: {x: 1,\n  y: 2}\n");
    EXPECT_EQ((int64_t)1, v.toAssocArray().get("a").toAssocArray().get("x").toInt64());
    EXPECT_EQ((int64_t)2, v.toAssocArray().get("a").toAssocArray().get("y").toInt64());

    v = pplib::Yaml::loads("a: [1,\n\n2]\n");
    EXPECT_EQ((size_t)2, v.toAssocArray().get("a").toVariantArray().size());

    v = pplib::Yaml::loads("a: [\n]\nb: {\n}\nc: [1,2,]\nd: {x: 1,}\n");
    EXPECT_EQ((size_t)0, v.toAssocArray().get("a").toVariantArray().size());
    EXPECT_EQ((size_t)0, v.toAssocArray().get("b").toAssocArray().size());
    EXPECT_EQ((size_t)2, v.toAssocArray().get("c").toVariantArray().size());
    EXPECT_EQ((int64_t)1, v.toAssocArray().get("d").toAssocArray().get("x").toInt64());
}

TEST_F(YamlTest, FlowAnchorsAliasesAndQuotedKeys)
{
    pplib::Variant v = pplib::Yaml::loads("a: [&x 1, *x]\n");
    pplib::VariantArray arr = v.toAssocArray().get("a").toVariantArray();
    EXPECT_EQ((size_t)2, arr.size());
    EXPECT_EQ((int64_t)1, arr[0].toInt64());
    EXPECT_EQ((int64_t)1, arr[1].toInt64());

    v = pplib::Yaml::loads("a: {\"k 1\": v}\n");
    EXPECT_EQ(pplib::String("v"), v.toAssocArray().get("a").toAssocArray().get("k 1").toString());

    // escaptes Anführungszeichen im Schlüssel
    v = pplib::Yaml::loads("\"a\\\"b\": 1\n");
    EXPECT_EQ((int64_t)1, v.toAssocArray().get("a\"b").toInt64());

    EXPECT_THROW(pplib::Yaml::loads("a: [*nope]\n"), pplib::ItemNotFoundException);
}

TEST_F(YamlTest, NamedEscapesAndSurrogates)
{
    pplib::Variant v = pplib::Yaml::loads("a: \"\\e\\L\\P\"\n");
    pplib::String s = v.toAssocArray().get("a").toString();
    EXPECT_EQ((size_t)7, s.length());
    EXPECT_EQ((unsigned char)0x1B, (unsigned char)s[0]);
    EXPECT_EQ(0, memcmp(s.getPtr() + 1, "\xe2\x80\xa8\xe2\x80\xa9", 6));

    v = pplib::Yaml::loads("a: \"\\uD83D\\uDE00\"\n");
    s = v.toAssocArray().get("a").toString();
    EXPECT_EQ((size_t)4, s.length());
    EXPECT_EQ((unsigned char)0xF0, (unsigned char)s[0]);
    EXPECT_EQ((unsigned char)0x9F, (unsigned char)s[1]);
    EXPECT_EQ((unsigned char)0x98, (unsigned char)s[2]);
    EXPECT_EQ((unsigned char)0x80, (unsigned char)s[3]);
}

TEST_F(YamlTest, TaggedNullCollectionsAndUntaggedInvalidTime)
{
    pplib::Variant v = pplib::Yaml::loads("a: !!seq\nb: !!map\nc: !!seq x\nd: !!map y\ne: 12:99:00\n");
    EXPECT_TRUE(v.toAssocArray().get("a").isNull());
    EXPECT_TRUE(v.toAssocArray().get("b").isNull());
    EXPECT_EQ(pplib::String("x"), v.toAssocArray().get("c").toString());
    EXPECT_EQ(pplib::String("y"), v.toAssocArray().get("d").toString());
    EXPECT_EQ(pplib::String("12:99:00"), v.toAssocArray().get("e").toString());
}

TEST_F(YamlTest, DocumentMarkersAndComments)
{
    pplib::Variant v = pplib::Yaml::loads("a: 1\n...\n");
    EXPECT_EQ((int64_t)1, v.toAssocArray().get("a").toInt64());

    v = pplib::Yaml::loads("---\na: 2\n...\n");
    EXPECT_EQ((int64_t)2, v.toAssocArray().get("a").toInt64());

    v = pplib::Yaml::loads("# Kommentar\na: 1\n");
    EXPECT_EQ((int64_t)1, v.toAssocArray().get("a").toInt64());

    // '#' beendet die Schlüssel-Erkennung: Zeile ist ein Skalar, kein Mapping
    v = pplib::Yaml::loads("a #b: 1\n");
    EXPECT_TRUE(v.isString());
    EXPECT_EQ(pplib::String("a"), v.toString());

    v = pplib::Yaml::loads("- a #b: 1\n");
    ASSERT_TRUE(v.isVariantArray());
    EXPECT_EQ(pplib::String("a"), v.toVariantArray()[0].toString());

    // Tabulator direkt nach '-' ist als Sequenz-Eintrag erlaubt
    v = pplib::Yaml::loads("a:\n  -\tb\n");
    EXPECT_EQ((size_t)1, v.toAssocArray().get("a").toVariantArray().size());
    EXPECT_EQ(pplib::String("b"), v.toAssocArray().get("a").toVariantArray()[0].toString());
}

TEST_F(YamlErrorTest, DocumentTerminationErrors)
{
    EXPECT_THROW(pplib::Yaml::loads("---\n...\nfoo\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a: 1\n...\nb: 2\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a: 1\n...\n...\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a: 1\n---\nb: 2\n"), pplib::Exception);
    EXPECT_THROW(pplib::Yaml::loads("a:\n---\n"), pplib::Exception);
    EXPECT_THROW(pplib::Yaml::loads("a:\n  - 1\n---\n"), pplib::Exception);
    EXPECT_THROW(pplib::Yaml::loads("# nur ein Kommentar\n"), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Yaml::loads("---\n"), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Yaml::loads("---"), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Yaml::loads("a: 1\n...\n---\nb: 2\n"), pplib::UnexpectedCharacterException);
}

TEST_F(YamlErrorTest, IndentationAndKeyErrors)
{
    EXPECT_THROW(pplib::Yaml::loads("a: 1\n  bad: 2\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a: 1\n b: 2\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a:\n  b: 1\n c: 2\n"), pplib::UnexpectedCharacterException);
    // eine Sequenzzeile mit zu großer Einrückung darf nicht stillschweigend zu
    // einem einzigen Skalar-Wert '1 - 2' verschmolzen werden
    EXPECT_THROW(pplib::Yaml::loads("a:\n  - 1\n   - 2\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a:\n  - 1\n   b: 2\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("\"key\" x\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a: 1\n- 2\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a: \"a\\\"b\": 1\n"), pplib::UnexpectedCharacterException);
}

TEST_F(YamlErrorTest, FlowAndQuotedValueErrors)
{
    EXPECT_THROW(pplib::Yaml::loads("a: [x:]\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a: [}]\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a: [1\n"), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Yaml::loads("a: {x: 1\n"), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Yaml::loads("a: [1, # x: 2]\n"), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Yaml::loads("a: \"x\" y\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a: [1] [2]\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a: \"abc\n"), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Yaml::loads("a: \"x\n\ny\"\n"), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Yaml::loads("a: \"x\\\n"), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Yaml::loads("a: \"abc\\\ndef\"\n"), pplib::UnexpectedEndOfDataException);
}

TEST_F(YamlErrorTest, EscapeAndControlCharErrors)
{
    EXPECT_THROW(pplib::Yaml::loads("a: \"\\x4\"\n"), pplib::InvalidEscapeSequenceException);
    EXPECT_THROW(pplib::Yaml::loads("a: \"x\x01y\"\n"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Yaml::loads("a: \"\x1b\"\n"), pplib::UnexpectedCharacterException);
}

TEST_F(YamlErrorTest, TagTypeConversionErrors)
{
    EXPECT_THROW(pplib::Yaml::loads("a: !!timestamp notadate\n"), pplib::TypeConversionException);
    EXPECT_THROW(pplib::Yaml::loads("a: !!timestamp 2024-13-45\n"), pplib::TypeConversionException);
    EXPECT_THROW(pplib::Yaml::loads("a: !!timestamp 2024-06-05T99:00\n"), pplib::TypeConversionException);
    EXPECT_THROW(pplib::Yaml::loads("a: !TimeDelta xyz\n"), pplib::TypeConversionException);
    EXPECT_THROW(pplib::Yaml::loads("a: !TimeZone +99999\n"), pplib::TypeConversionException);
    EXPECT_THROW(pplib::Yaml::loads("a: !TimeZone ab:cd\n"), pplib::TypeConversionException);
    EXPECT_THROW(pplib::Yaml::loads("a: !TimeZone +99:00\n"), pplib::IllegalArgumentException);
    EXPECT_THROW(pplib::Yaml::loads("a: !timestamp 12:34:56\n"), pplib::TypeConversionException);
    EXPECT_THROW(pplib::Yaml::loads("a: !timestamp 1.5\n"), pplib::TypeConversionException);
}

TEST_F(YamlErrorTest, NestingDepthLimitSequenceAndFlow)
{
    std::string seq;
    for (int i = 0; i < 130; i++) seq += std::string(i + 1, ' ') + "-\n";
    EXPECT_THROW(pplib::Yaml::loads(pplib::String(seq.c_str(), seq.size())), pplib::OverflowException);

    std::string flow(130, '[');
    EXPECT_THROW(pplib::Yaml::loads(pplib::String(flow.c_str(), flow.size())), pplib::OverflowException);

    std::string map;
    for (int i = 0; i < 130; i++) map += std::string(i, ' ') + "k" + std::to_string(i) + ":\n";
    EXPECT_THROW(pplib::Yaml::loads(pplib::String(map.c_str(), map.size())), pplib::OverflowException);

    EXPECT_THROW(pplib::Yaml::loads("a: {[1, 2]: x}\n"), pplib::UnexpectedCharacterException);
}

// ============================================================================
// Writer: Root-Typen, Escapes und Schlüssel
// ============================================================================

TEST_F(YamlTest, DumpRootScalarsAndEmptyCollections)
{
    EXPECT_EQ(pplib::String("-9223372036854775808\n"),
              pplib::Yaml::dumps(pplib::Variant(std::numeric_limits<int64_t>::min())));
    EXPECT_EQ(pplib::String("0.1\n"), pplib::Yaml::dumps(pplib::Variant(0.1)));
    EXPECT_EQ(pplib::String("5.0\n"), pplib::Yaml::dumps(pplib::Variant(5.0)));
    EXPECT_EQ(pplib::String("true\n"), pplib::Yaml::dumps(pplib::Variant(true)));
    EXPECT_EQ(pplib::String("{}\n"), pplib::Yaml::dumps(pplib::Variant(pplib::AssocArray())));
    EXPECT_EQ(pplib::String("[]\n"), pplib::Yaml::dumps(pplib::Variant(pplib::VariantArray())));
    EXPECT_EQ(pplib::String("[]\n"), pplib::Yaml::dumps(pplib::Variant(pplib::Array())));
    EXPECT_EQ(pplib::String("!!timestamp 2024-06-05\n"),
              pplib::Yaml::dumps(pplib::Variant(pplib::Date(2024, 6, 5))));
}

TEST_F(YamlTest, DumpControlCharacterEscapes)
{
    pplib::AssocArray data;
    data.set("bs", pplib::Variant(pplib::String("a\bb")));
    data.set("ff", pplib::Variant(pplib::String("a\fb")));
    data.set("cr", pplib::Variant(pplib::String("a\rb")));
    data.set("nul", pplib::Variant(pplib::String("a\0b", 3)));
    pplib::String yaml = pplib::Yaml::dumps(data);
    EXPECT_EQ(pplib::String("bs: \"a\\bb\"\ncr: \"a\\rb\"\nff: \"a\\fb\"\nnul: \"a\\u0000b\"\n"), yaml);

    pplib::Variant back = pplib::Yaml::loads(yaml);
    EXPECT_EQ((size_t)3, back.toAssocArray().get("bs").toString().length());
    EXPECT_EQ(0, memcmp(back.toAssocArray().get("bs").toString().getPtr(), "a\bb", 3));
    EXPECT_EQ(0, memcmp(back.toAssocArray().get("cr").toString().getPtr(), "a\rb", 3));
    EXPECT_EQ(0, memcmp(back.toAssocArray().get("nul").toString().getPtr(), "a\0b", 3));
}

TEST_F(YamlTest, DumpNumericLookingStringsStayStrings)
{
    static const char* quoted[] = {"2024-06-05", "1_2",  "1_",      "1__2",    "_1",     "1.",
                                   "1._5",        ".5",   "1e5",     "1.2e3",   "1.e5",   "0x1F",
                                   "+5",          "-5",   "true",    "null",    "~",      "- x",
                                   "a: b",        "12:99:00", "",    " ",       "#c",     "x #c",
                                   "[a]",         "{a}",  "*a",      "&a",      "!",      "|",
                                   "%x",          "@x",   ":x",      "x:",      "'x",     "2024-06-05t12:34:56",
                                   "2024-06-05T12:34", "True",  "TRUE",  "False", "FALSE", "Null",
                                   "NULL",        ".inf", ".Inf",    ".INF",    ".nan",   ".NaN",
                                   ".NAN",        "1E5",  "1e+5",    "1e-5",    "123_",   "0X1F",
                                   "0o17",        "0O17", "0b101",   "0B101",   "-0x1F",  "1_2.5",
                                   "12:34",       "12:34.5", "2024-06-05 12:34:56", "a\"b",
                                   "123456789012345678901234567890"};
    for (const char* value : quoted) {
        pplib::AssocArray data;
        data.set("k", pplib::Variant(pplib::String(value)));
        pplib::String yaml = pplib::Yaml::dumps(data);
        // der Wert steht in Anführungszeichen: 'k: "..."'
        EXPECT_EQ((char)'"', yaml[3]) << value;
        pplib::Variant back = pplib::Yaml::loads(yaml);
        EXPECT_EQ(pplib::Variant::TYPE_STRING, back.toAssocArray().get("k").type()) << value;
        EXPECT_EQ(pplib::String(value), back.toAssocArray().get("k").toString()) << value;
    }

    // diese Werte sind eindeutig und bleiben unquotiert
    static const char* plain[] = {"x y", "yes", "no", "on", "off", "Y",
                                  "0.1.2", "1,5", "x#c", ".e5", "1:2", "a-b-c",
                                  "1.2.3", "http://x", "ab:cd", "a\\b", "\\a"};
    for (const char* value : plain) {
        pplib::AssocArray data;
        data.set("k", pplib::Variant(pplib::String(value)));
        pplib::String yaml = pplib::Yaml::dumps(data);
        EXPECT_EQ(pplib::String().setf("k: %s\n", value), yaml) << value;
        pplib::Variant back = pplib::Yaml::loads(yaml);
        EXPECT_EQ(pplib::Variant::TYPE_STRING, back.toAssocArray().get("k").type()) << value;
        EXPECT_EQ(pplib::String(value), back.toAssocArray().get("k").toString()) << value;
    }
}

TEST_F(YamlTest, DumpMultiLineStringsStayQuoted)
{
    // mehrzeilige Strings sind nur als Wert eines Schlüssels als Block-Skalar darstellbar
    EXPECT_EQ(pplib::String("\"a\\nb\"\n"), pplib::Yaml::dumps(pplib::Variant(pplib::String("a\nb"))));

    pplib::AssocArray data;
    data.set("s1", pplib::Variant(pplib::String("a\nb\n")));
    data.set("s2", pplib::Variant(pplib::String("\na")));
    data.set("s3", pplib::Variant(pplib::String("a \nb")));
    pplib::String yaml = pplib::Yaml::dumps(data);
    EXPECT_EQ(pplib::String("s1: \"a\\nb\\n\"\ns2: \"\\na\"\ns3: \"a \\nb\"\n"), yaml);
    EXPECT_EQ(pplib::String("a\nb\n"), pplib::Yaml::loads(yaml).toAssocArray().get("s1").toString());
}

TEST_F(YamlTest, DumpMapKeyNeedingQuotes)
{
    pplib::AssocArray data;
    data.set("k:", pplib::Variant(pplib::String("v")));
    data.set("a b", pplib::Variant((int64_t)1));
    data.set("1", pplib::Variant(pplib::String("x")));
    pplib::String yaml = pplib::Yaml::dumps(data);
    EXPECT_EQ(pplib::String("1: x\na b: 1\n\"k:\": v\n"), yaml);

    pplib::Variant back = pplib::Yaml::loads(yaml);
    EXPECT_EQ((size_t)3, back.toAssocArray().size());
    EXPECT_EQ(pplib::String("v"), back.toAssocArray().get("k:").toString());
}

TEST_F(YamlTest, DumpCollectionsAsValues)
{
    pplib::AssocArray data;
    data.set("a", pplib::Variant(pplib::VariantArray()));
    data.set("b", pplib::Variant(pplib::AssocArray()));
    pplib::Array classic;
    classic.add(pplib::String("x"));
    data.set("c", pplib::Variant(classic));
    pplib::String yaml = pplib::Yaml::dumps(data);
    EXPECT_EQ(pplib::String("a: []\nb: {}\nc:\n- x\n"), yaml);

    pplib::Variant back = pplib::Yaml::loads(yaml);
    EXPECT_EQ((size_t)0, back.toAssocArray().get("a").toVariantArray().size());
    EXPECT_EQ((size_t)0, back.toAssocArray().get("b").toAssocArray().size());
    EXPECT_EQ(pplib::String("x"), back.toAssocArray().get("c").toVariantArray()[0].toString());
}

TEST_F(YamlTest, DumpClassicArrayInsideVariantArray)
{
    pplib::Array classic;
    classic.add(pplib::String("x"));
    classic.add(pplib::String("42"));
    pplib::VariantArray list;
    pplib::Variant value(classic);
    list.add(value);
    EXPECT_EQ(pplib::String("-\n  - x\n  - \"42\"\n"), pplib::Yaml::dumps(pplib::Variant(list)));
}

TEST_F(YamlTest, WriterDepthLimitVariantArray)
{
    pplib::VariantArray cur;
    for (int i = 0; i < 130; i++) {
        pplib::VariantArray inner;
        pplib::Variant var(cur);
        inner.add(var);
        cur = inner;
    }
    EXPECT_THROW(pplib::Yaml::dumps(pplib::Variant(cur)), pplib::OverflowException);
}

TEST_F(YamlTest, TypedOverloadsStringOutAndAssocArray)
{
    pplib::AssocArray data;
    data.set("k", pplib::Variant((int64_t)1));
    pplib::String yaml;
    pplib::Yaml::dumps(yaml, data);
    EXPECT_EQ(pplib::String("k: 1\n"), yaml);

    pplib::VariantArray list;
    pplib::Variant one((int64_t)7);
    list.add(one);
    pplib::String yaml2;
    pplib::Yaml::dumps(yaml2, list);
    EXPECT_EQ(pplib::String("- 7\n"), yaml2);

    pplib::MemFile out;
    pplib::Yaml::dump(out, data);
    size_t size = out.tell();
    pplib::MemFile in(const_cast<char*>(reinterpret_cast<const char*>(out.map(0, size))), size, false);
    pplib::Variant v;
    pplib::Yaml::load(v, in);
    EXPECT_EQ((int64_t)1, v.toAssocArray().get("k").toInt64());
}

TEST_F(YamlTest, ShortTagForms)
{
    pplib::Variant v = pplib::Yaml::loads(
        "s: !str 123\n"
        "i: !int 42\n"
        "f: !float 1.5\n"
        "b: !bool True\n"
        "n: !null ~\n"
        "e: !null\n"
        "bin: !binary \"aGVsbG8=\"\n"
        "bin2: !binary aGVsbG8=\n"
        "d: !timestamp 2024-06-05\n"
        "dt1: !timestamp 2024-06-05t12:34:56\n"
        "dt2: !timestamp 2024-06-05 12:34:56\n"
        "empty: !str\n"
        "unknown: !unknown x\n");
    EXPECT_EQ(pplib::String("123"), v.toAssocArray().get("s").toString());
    EXPECT_EQ((int64_t)42, v.toAssocArray().get("i").toInt64());
    EXPECT_EQ(1.5, v.toAssocArray().get("f").toDouble());
    EXPECT_TRUE(v.toAssocArray().get("b").toBool());
    EXPECT_TRUE(v.toAssocArray().get("n").isNull());
    EXPECT_TRUE(v.toAssocArray().get("e").isNull());
    EXPECT_EQ((size_t)5, v.toAssocArray().get("bin").toByteArray().size());
    EXPECT_EQ(0, memcmp(v.toAssocArray().get("bin2").toByteArray().ptr(), "hello", 5));
    EXPECT_EQ(pplib::String("2024-06-05"), v.toAssocArray().get("d").toDate().toString());
    EXPECT_TRUE(v.toAssocArray().get("dt1").isDateTime());
    EXPECT_TRUE(v.toAssocArray().get("dt2").isDateTime());
    EXPECT_EQ(pplib::String(""), v.toAssocArray().get("empty").toString());
    EXPECT_EQ(pplib::String("x"), v.toAssocArray().get("unknown").toString());

    // base64 darf über mehrere Zeilen (quoted oder block scalar) verteilt sein
    EXPECT_EQ((size_t)5, pplib::Yaml::loads("a: !binary \"aGVs\n  bG8=\"\n").toAssocArray().get("a").toByteArray().size());
    EXPECT_EQ((size_t)5, pplib::Yaml::loads("a: !binary |\n  aGVs\n  bG8=\n").toAssocArray().get("a").toByteArray().size());
}

TEST_F(YamlTest, DumpBlockScalarEmptyLineAndQuotedFallbacks)
{
    // Leerzeile im Block-Skalar bleibt leer (kein Leerzeichen-Indent)
    pplib::AssocArray data;
    data.set("block", pplib::Variant(pplib::String("a\n\nb")));
    pplib::String yaml = pplib::Yaml::dumps(data);
    EXPECT_EQ(pplib::String("block: |-\n  a\n\n  b\n"), yaml);
    EXPECT_EQ(pplib::String("a\n\nb"), pplib::Yaml::loads(yaml).toAssocArray().get("block").toString());

    // Zeilen mit leading/trailing Whitespace oder Tabs gehen nur quotiert verlustfrei
    pplib::AssocArray lossy;
    lossy.set("lead", pplib::Variant(pplib::String("a\n b")));
    lossy.set("tab", pplib::Variant(pplib::String("a\tb\nc")));
    lossy.set("trail", pplib::Variant(pplib::String("a\nb\t")));
    lossy.set("sp", pplib::Variant(pplib::String("a \nb\n")));
    lossy.set("deep", pplib::Variant(pplib::String("a\n  b\nc")));
    pplib::String out = pplib::Yaml::dumps(lossy);
    EXPECT_EQ(pplib::String("deep: \"a\\n  b\\nc\"\nlead: \"a\\n b\"\n"
                            "sp: \"a \\nb\\n\"\ntab: \"a\\tb\\nc\"\ntrail: \"a\\nb\\t\"\n"),
              out);
    EXPECT_EQ(pplib::String("a\n b"), pplib::Yaml::loads(out).toAssocArray().get("lead").toString());
    EXPECT_EQ(pplib::String("a\tb\nc"), pplib::Yaml::loads(out).toAssocArray().get("tab").toString());
    EXPECT_EQ(pplib::String("a\nb\t"), pplib::Yaml::loads(out).toAssocArray().get("trail").toString());
}

TEST_F(YamlErrorTest, TypedOverloadsEmptyInput)
{
    pplib::Variant v;
    EXPECT_THROW(pplib::Yaml::loads(v, pplib::String("")), pplib::UnexpectedEndOfDataException);
    pplib::AssocArray obj;
    EXPECT_THROW(pplib::Yaml::loads(obj, pplib::String("")), pplib::UnexpectedEndOfDataException);
    pplib::VariantArray arr;
    EXPECT_THROW(pplib::Yaml::loads(arr, pplib::String("")), pplib::UnexpectedEndOfDataException);
}

TEST_F(YamlTest, LoadVariantFromFileReturnValueOverload)
{
    char buf[] = "k: 1\n";
    pplib::MemFile in(buf, sizeof(buf) - 1, false);
    pplib::Variant v = pplib::Yaml::load(in);
    EXPECT_EQ((int64_t)1, v.toAssocArray().get("k").toInt64());
}

TEST_F(YamlTest, DumpNestedCollectionsInsideSequenceEntries)
{
    // Ein Sequenz-Eintrag, dessen Mapping nicht-leere Sammlungen enthält, kann
    // nicht inline ("- key: value") geschrieben werden
    pplib::Array classic;
    classic.add(pplib::String("x"));
    pplib::AssocArray withClassic;
    withClassic.set("k", pplib::Variant(classic));

    pplib::AssocArray innerDict;
    innerDict.set("b", pplib::Variant((int64_t)1));
    pplib::AssocArray withDict;
    withDict.set("j", pplib::Variant(innerDict));

    pplib::VariantArray innerArray;
    innerArray.add(pplib::Variant((int64_t)1));
    pplib::AssocArray withArray;
    withArray.set("m", pplib::Variant(innerArray));

    pplib::VariantArray list;
    list.add(pplib::Variant(withClassic));
    list.add(pplib::Variant(withDict));
    list.add(pplib::Variant(withArray));

    pplib::AssocArray data;
    data.set("list", pplib::Variant(list));
    data.set("emptyClassic", pplib::Variant(pplib::Array()));
    pplib::String yaml = pplib::Yaml::dumps(data);
    EXPECT_EQ(pplib::String("emptyClassic: []\nlist:\n-\n  k:\n  - x\n-\n  j:\n    b: 1\n-\n  m:\n  - 1\n"), yaml);

    pplib::Variant back = pplib::Yaml::loads(yaml);
    EXPECT_EQ((size_t)0, back.toAssocArray().get("emptyClassic").toVariantArray().size());
    EXPECT_EQ((size_t)3, back.toAssocArray().get("list").toVariantArray().size());
}

TEST_F(YamlTest, DumpEmptyClassicArrayAndMultilineStringsAsSequenceElements)
{
    // Leeres klassisches Array und mehrzeilige Strings als direktes Sequenz-Element
    pplib::VariantArray seq;
    seq.add(pplib::Variant(pplib::Array()));
    seq.add(pplib::Variant(pplib::String("line1\nline2")));
    seq.add(pplib::Variant(pplib::String("a\n\nb")));
    pplib::String yaml = pplib::Yaml::dumps(pplib::Variant(seq), 2);
    EXPECT_EQ(pplib::String("- []\n- |-\n  line1\n  line2\n- |-\n  a\n\n  b\n"), yaml);

    pplib::Variant back = pplib::Yaml::loads(yaml);
    ASSERT_TRUE(back.isVariantArray());
    EXPECT_EQ((size_t)3, back.toVariantArray().size());
    EXPECT_EQ((size_t)0, back.toVariantArray()[0].toVariantArray().size());
    EXPECT_EQ(pplib::String("line1\nline2"), back.toVariantArray()[1].toString());
    EXPECT_EQ(pplib::String("a\n\nb"), back.toVariantArray()[2].toString());
}

TEST_F(YamlTest, DumpDeleteCharacterStaysVisible)
{
    // DEL (0x7f) darf in einem mehrzeiligen String nicht verlorengehen
    pplib::AssocArray data;
    data.set("del", pplib::Variant(pplib::String("a\nb\x7f" "c")));
    pplib::String yaml = pplib::Yaml::dumps(data);
    EXPECT_EQ((size_t)14, yaml.length());
    EXPECT_EQ(pplib::String("del: \"a\\n" "b\x7f" "c\"\n"), yaml);
}

// ============================================================================
// Writer: Base64-Block-Scalaren für ByteArray
// ============================================================================

static pplib::ByteArray makeByteArrayForSize(size_t n)
{
    unsigned char* raw = new unsigned char[n > 0 ? n : 1];
    for (size_t i = 0; i < n; i++) raw[i] = static_cast<unsigned char>(i * 7 + 3);
    pplib::ByteArray ba(raw, n);
    delete[] raw;
    return ba;
}

// Breite der breitesten Fortsetzungszeile (alle Zeilen außer der ersten)
static size_t widestContinuationLine(const pplib::String& s)
{
    size_t widest = 0;
    size_t len = 0;
    bool first = true;
    for (size_t i = 0; i <= s.size(); i++) {
        if (i == s.size() || s.c_str()[i] == '\n') {
            if (!first && len > widest) widest = len;
            first = false;
            len = 0;
        } else {
            len++;
        }
    }
    return widest;
}

static size_t countLines(const pplib::String& s)
{
    size_t lines = 0;
    for (size_t i = 0; i < s.size(); i++)
        if (s.c_str()[i] == '\n') lines++;
    return lines;
}

static void expectSameBytes(const pplib::ByteArray& a, const pplib::ByteArray& b, size_t n)
{
    ASSERT_EQ(n, b.size());
    EXPECT_EQ(0, memcmp(b.ptr(), a.ptr(), n));
}

TEST_F(YamlTest, DumpByteArrayStaysInlineUpToColumnWidth)
{
    pplib::AssocArray data;
    data.set("bin", pplib::Variant(makeByteArrayForSize(3)));
    EXPECT_EQ(pplib::String("bin: !!binary AwoR\n"), pplib::Yaml::dumps(data));

    // 57 Bytes -> Base64 ist exakt 76 Zeichen lang und passt noch in eine Zeile
    data.set("bin", pplib::Variant(makeByteArrayForSize(57)));
    EXPECT_EQ(pplib::String("bin: !!binary AwoRGB8mLTQ7QklQV15lbHN6gYiPlp2kq7K5wMfO1dzj6vH4/wYNFBsiKTA3PkVMU1phaG92fYSL\n"),
              pplib::Yaml::dumps(data));
}

TEST_F(YamlTest, DumpByteArrayFoldsBase64IntoBlockScalar)
{
    // 58 Bytes -> Base64 ist 80 Zeichen lang und muss nach 76 Zeichen umbrochen werden
    pplib::ByteArray ba = makeByteArrayForSize(58);
    pplib::AssocArray data;
    data.set("bin", pplib::Variant(ba));
    pplib::String yaml = pplib::Yaml::dumps(data);
    EXPECT_EQ(pplib::String("bin: !!binary |-\n"
                            "  AwoRGB8mLTQ7QklQV15lbHN6gYiPlp2kq7K5wMfO1dzj6vH4/wYNFBsiKTA3PkVMU1phaG92fYSL\n"
                            "  kg==\n"),
              yaml);

    pplib::Variant back = pplib::Yaml::loads(yaml);
    ASSERT_TRUE(back.toAssocArray().get("bin").isByteArray());
    expectSameBytes(ba, back.toAssocArray().get("bin").toByteArray(), 58);
}

TEST_F(YamlTest, DumpByteArrayRoundTripOverSizeMatrix)
{
    for (size_t n : {size_t(58), size_t(76), size_t(77), size_t(114), size_t(152), size_t(300), size_t(1000)}) {
        SCOPED_TRACE(std::to_string(n));
        pplib::ByteArray ba = makeByteArrayForSize(n);
        pplib::AssocArray data;
        data.set("bin", pplib::Variant(ba));
        pplib::String yaml = pplib::Yaml::dumps(data);

        const size_t b64len = ba.toBase64().size();
        EXPECT_EQ(pplib::String("bin: !!binary |-\n"), yaml.substr(0, 17));
        // Jede volle Fortsetzungszeile besteht aus Einrückung + 76 Zeichen
        EXPECT_TRUE(widestContinuationLine(yaml) <= 76 + 2);
        EXPECT_EQ((size_t)(1 + (b64len + 75) / 76), countLines(yaml));

        pplib::Variant back = pplib::Yaml::loads(yaml);
        ASSERT_TRUE(back.toAssocArray().get("bin").isByteArray());
        expectSameBytes(ba, back.toAssocArray().get("bin").toByteArray(), n);
    }
}

TEST_F(YamlTest, DumpByteArrayFoldedInNestedPositions)
{
    const size_t n = 300;
    pplib::ByteArray ba = makeByteArrayForSize(n);

    // ByteArray direkt als Dokument
    {
        pplib::Variant back = pplib::Yaml::loads(pplib::Yaml::dumps(pplib::Variant(ba), 4));
        ASSERT_TRUE(back.isByteArray());
        expectSameBytes(ba, back.toByteArray(), n);
    }

    // ByteArray als Sequenz-Eintrag
    {
        pplib::VariantArray seq;
        seq.add(pplib::Variant(ba));
        pplib::Variant back = pplib::Yaml::loads(pplib::Yaml::dumps(pplib::Variant(seq), 4));
        ASSERT_TRUE(back.isVariantArray());
        expectSameBytes(ba, back.toVariantArray()[0].toByteArray(), n);
    }

    // ByteArray in einem Mapping, das selbst Sequenz-Eintrag ist: die
    // Blockfortsetzung muss tiefer als der Key einrücken
    {
        pplib::AssocArray dict;
        dict.set("bin", pplib::Variant(ba));
        pplib::VariantArray outer;
        outer.add(pplib::Variant(dict));
        pplib::String yaml = pplib::Yaml::dumps(pplib::Variant(outer), 4);
        EXPECT_EQ((size_t)82, widestContinuationLine(yaml));
        pplib::Variant back = pplib::Yaml::loads(yaml);
        ASSERT_TRUE(back.isVariantArray());
        ASSERT_TRUE(back.toVariantArray()[0].isAssocArray());
        expectSameBytes(ba, back.toVariantArray()[0].toAssocArray().get("bin").toByteArray(), n);
    }

    // ByteArray in einer Sequenz unter einem Key, indent variiert
    {
        for (int indent : {2, 1, 4}) {
            SCOPED_TRACE(std::string("indent=") + std::to_string(indent));
            pplib::VariantArray inner;
            inner.add(pplib::Variant(ba));
            pplib::AssocArray data;
            data.set("arr", pplib::Variant(inner));
            pplib::Variant back = pplib::Yaml::loads(pplib::Yaml::dumps(pplib::Variant(data), indent));
            ASSERT_TRUE(back.toAssocArray().get("arr").isVariantArray());
            expectSameBytes(ba, back.toAssocArray().get("arr").toVariantArray()[0].toByteArray(), n);
        }
    }
}

TEST_F(YamlTest, LoadFoldedBase64BlockScalarForms)
{
    // Block-Scalar über mehrere Zeilen, in Mapping- und Sequenz-Position.
    // Padding darf nur am Ende des (zusammenhängenden) Base64-Stroms stehen.
    pplib::Variant v = pplib::Yaml::loads("a: !!binary |-\n  QUJDREFC\n  Q0Q=\n");
    pplib::ByteArray a = v.toAssocArray().get("a").toByteArray();
    ASSERT_EQ((size_t)8, a.size());
    EXPECT_EQ(0, memcmp(a.ptr(), "ABCDABCD", 8));

    v = pplib::Yaml::loads("- k: !!binary |-\n    QUJD\n    REVG\n");
    ASSERT_TRUE(v.isVariantArray());
    pplib::ByteArray b = v.toVariantArray()[0].toAssocArray().get("k").toByteArray();
    ASSERT_EQ((size_t)6, b.size());
    EXPECT_EQ(0, memcmp(b.ptr(), "ABCDEF", 6));

    v = pplib::Yaml::loads("c: !binary >-\n  QUJD\n  RA==\n");
    pplib::ByteArray c = v.toAssocArray().get("c").toByteArray();
    ASSERT_EQ((size_t)4, c.size());
    EXPECT_EQ(0, memcmp(c.ptr(), "ABCD", 4));
}

TEST_F(YamlTest, LoadBase64WithPaddingInsideStreamIsRejected)
{
    // Padding in der Mitte ist kein gültiger Base64-Stream. ByteArray::fromBase64
    // würde daraus stillschweigend Initialisierungsmüll erzeugen, daher lehnt
    // der Parser solche Werte ab.
    EXPECT_THROW(pplib::Yaml::loads("a: !!binary QUJDRA==QUJDRA==\n"), pplib::TypeConversionException);
    EXPECT_THROW(pplib::Yaml::loads("a: !!binary |-\n  QUJDRA==\n  QUJDRA==\n"), pplib::TypeConversionException);
}

} // namespace
