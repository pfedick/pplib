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

#include <cmath>
#include <locale.h>
#include <gtest/gtest.h>
#include "pplib-tests.h"
#include <pplib/core/yaml.h>
#include <pplib/core/fileobject.h>
#include <pplib/core/memfile.h>
#include <pplib/core/file.h>
#include <pplib/exceptions.h>
#include <pplib/types/variant.h>
#include <pplib/types/variantarray.h>
#include <pplib/types/assocarray.h>
#include <pplib/types/array.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/date.h>
#include <pplib/types/datetime.h>
#include <pplib/types/time.h>
#include <limits>

using namespace pplib;

class YamlTest : public ::testing::Test
{
public:
    YamlTest()
    {
        setlocale(LC_CTYPE, DEFAULT_LOCALE);
    }
};

// ============================================================================
// 1. Integrationstests mit Testdateien
// ============================================================================

TEST_F(YamlTest, LoadTest1File)
{
    String yaml;
    File::load(yaml, "testdata/test1.yaml");
    AssocArray root;
    Yaml::loads(root, yaml);

    EXPECT_EQ(12345, root["integer"].toInt64());
    EXPECT_TRUE(root["string"].isString());
    EXPECT_EQ(String("hello world"), root["string"].toString());
    EXPECT_TRUE(root["boolean_true"].toBool());
    EXPECT_FALSE(root["boolean_false"].toBool());
    EXPECT_TRUE(root["null_value"].isNull());
    EXPECT_DOUBLE_EQ(3.14159, root["float"].toDouble());
    EXPECT_DOUBLE_EQ(3.141592653589793, root["double"].toDouble());

    const VariantArray& ints = root["integer_array"].toVariantArray();
    ASSERT_EQ(3u, ints.size());
    EXPECT_EQ(1, ints[0].toInt64());
    EXPECT_EQ(2, ints[1].toInt64());
    EXPECT_EQ(3, ints[2].toInt64());

    const VariantArray& colors = root["string_array"].toVariantArray();
    ASSERT_EQ(3u, colors.size());
    EXPECT_EQ(String("red"), colors[0].toString());
    EXPECT_EQ(String("green"), colors[1].toString());
    EXPECT_EQ(String("blue"), colors[2].toString());

    const AssocArray& obj = root["object"].toAssocArray();
    EXPECT_EQ(String("value1"), obj["key1"].toString());
    EXPECT_EQ(String("value2"), obj["key2"].toString());
    EXPECT_EQ(String("value3"), obj["key3"].toString());

    const VariantArray& objs = obj["sub_array_with_objects"].toVariantArray();
    ASSERT_EQ(2u, objs.size());
    EXPECT_EQ(String("sub_value1"), objs[0].toAssocArray()["sub_key1"].toString());
    EXPECT_EQ(String("sub_value4"), objs[1].toAssocArray()["sub_key4"].toString());

    const VariantArray& sub = obj["sub_array"].toVariantArray();
    ASSERT_EQ(4u, sub.size());
    EXPECT_EQ(String("item1"), sub[0].toString());
    EXPECT_TRUE(sub[1].isNull());
    EXPECT_EQ(String("item3"), sub[2].toString());
    const AssocArray& nested = sub[3].toAssocArray();
    ASSERT_EQ(1u, nested.size());
    EXPECT_EQ(String("sub_value1"), nested["sub_object"].toAssocArray()["sub_key1"].toString());

    const VariantArray& users = root["users"].toVariantArray();
    ASSERT_EQ(2u, users.size());
    EXPECT_EQ(String("Alice"), users[0].toAssocArray()["name"].toString());
    EXPECT_EQ(30, users[0].toAssocArray()["age"].toInt64());
    EXPECT_EQ(String("bob@example.com"), users[1].toAssocArray()["email"].toString());

    EXPECT_TRUE(root["date"].isDate());
    EXPECT_EQ(String("2024-06-05"), root["date"].toDate().toString());
    EXPECT_TRUE(root["datetime"].isDateTime());
    EXPECT_NE(-1, root["datetime"].toDateTime().getISO8601().find("12:34:56"));
    EXPECT_TRUE(root["timestamp"].isDateTime());
    EXPECT_NE(-1, root["timestamp"].toDateTime().getISO8601withUsec().find(".789"));
    EXPECT_TRUE(root["time"].isTime());
    EXPECT_EQ(String("12:34:56.000000"), root["time"].toTime().toString());

    EXPECT_TRUE(root["binary"].isByteArray());
    EXPECT_EQ(String("SGVsbG8gd29ybGQ="), root["binary"].toByteArrayPtr().toBase64());
    EXPECT_TRUE(root["long_binary"].isByteArray());
    EXPECT_EQ(53u, root["long_binary"].toByteArrayPtr().size());

    EXPECT_TRUE(root["multiline_string"].isString());
    EXPECT_EQ(String("This is a\nmultiline string\nexample.\n"), root["multiline_string"].toString());
}

TEST_F(YamlTest, LoadAndDumpTest2FileObjectRoundTrip)
{
    String yaml;
    File::load(yaml, "testdata/test2.yaml");
    AssocArray root;
    Yaml::loads(root, yaml);

    MemFile memfile;
    Yaml::dump(memfile, pplib::Variant(root), 2);
    memfile.seek(0);

    AssocArray parsed = Yaml::load(memfile);
    EXPECT_EQ(root["multiline_string"].toString(), parsed["multiline_string"].toString());
    EXPECT_EQ(root["integer"], parsed["integer"]);
    EXPECT_EQ(root["string"], parsed["string"]);
    EXPECT_EQ(root["boolean_true"], parsed["boolean_true"]);
    EXPECT_EQ(root["boolean_false"], parsed["boolean_false"]);
    EXPECT_FLOAT_EQ(root["float"].toFloat(), parsed["float"].toFloat());
    EXPECT_DOUBLE_EQ(3.141592653589793, parsed["double"].toDouble());
    EXPECT_EQ(root["double"], parsed["double"]);
    EXPECT_EQ(root["null_value"], parsed["null_value"]);
    EXPECT_EQ(root["integer_array"], parsed["integer_array"]);
    EXPECT_EQ(root["string_array"], parsed["string_array"]);
    EXPECT_EQ(root["users/0"], parsed["users/0"]);
    EXPECT_EQ(root["users/1"], parsed["users/1"]);
    EXPECT_EQ(root["datetime"], parsed["datetime"]);
    EXPECT_EQ(root["timestamp"], parsed["timestamp"]);
    EXPECT_EQ(root["time"], parsed["time"]);

    parsed["datetime"] = root["datetime"];
    parsed["timestamp"] = root["timestamp"];
    EXPECT_EQ(root, parsed);
}

// ============================================================================
// 2. Skalare Werte (Parsen)
// ============================================================================

TEST_F(YamlTest, RootScalars)
{
    EXPECT_EQ(42, Yaml::loads("42").toInt64());
    EXPECT_EQ(String("hello"), Yaml::loads("\"hello\"").toString());
    EXPECT_TRUE(Yaml::loads("true").toBool());
    EXPECT_FALSE(Yaml::loads("false").toBool());
    EXPECT_DOUBLE_EQ(3.14, Yaml::loads("3.14").toDouble());
    EXPECT_TRUE(Yaml::loads("null").isNull());
    EXPECT_TRUE(Yaml::loads("~").isNull());
}

TEST_F(YamlTest, IntegerFormats)
{
    // Dezimal
    EXPECT_EQ(12345, Yaml::loads("v: 12345").toAssocArray()["v"].toInt64());
    EXPECT_EQ(-999, Yaml::loads("v: -999").toAssocArray()["v"].toInt64());
    EXPECT_EQ(42, Yaml::loads("v: +42").toAssocArray()["v"].toInt64());

    // Hexadezimal
    EXPECT_EQ(31, Yaml::loads("v: 0x1F").toAssocArray()["v"].toInt64());
    EXPECT_EQ(-16, Yaml::loads("v: -0x10").toAssocArray()["v"].toInt64());
    EXPECT_EQ(16, Yaml::loads("v: +0x10").toAssocArray()["v"].toInt64());

    // Oktal
    EXPECT_EQ(63, Yaml::loads("v: 0o77").toAssocArray()["v"].toInt64());
    EXPECT_EQ(-63, Yaml::loads("v: -0o77").toAssocArray()["v"].toInt64());
    EXPECT_EQ(8, Yaml::loads("v: +0o10").toAssocArray()["v"].toInt64());

    // Binär
    EXPECT_EQ(10, Yaml::loads("v: 0b1010").toAssocArray()["v"].toInt64());
    EXPECT_EQ(-3, Yaml::loads("v: -0b11").toAssocArray()["v"].toInt64());

    // Ungültige Präfix-Literale bleiben Strings
    EXPECT_EQ(String("0xxyz"), Yaml::loads("v: 0xxyz").toAssocArray()["v"].toString());
    EXPECT_EQ(String("0o99"), Yaml::loads("v: 0o99").toAssocArray()["v"].toString());
    EXPECT_EQ(String("0binvalid"), Yaml::loads("v: 0binvalid").toAssocArray()["v"].toString());

    // '-' und '+' als alleinstehende Skalare bleiben Strings
    EXPECT_EQ(String("-"), Yaml::loads("v: -").toAssocArray()["v"].toString());
    EXPECT_EQ(String("+"), Yaml::loads("v: +").toAssocArray()["v"].toString());
}

TEST_F(YamlTest, FloatFormats)
{
    EXPECT_DOUBLE_EQ(3.14159, Yaml::loads("v: 3.14159").toAssocArray()["v"].toDouble());
    EXPECT_DOUBLE_EQ(-0.5, Yaml::loads("v: -0.5").toAssocArray()["v"].toDouble());
    EXPECT_DOUBLE_EQ(3.14, Yaml::loads("v: +3.14").toAssocArray()["v"].toDouble());

    // Exponenten
    EXPECT_DOUBLE_EQ(1e-3, Yaml::loads("v: 1e-3").toAssocArray()["v"].toDouble());
    EXPECT_DOUBLE_EQ(2.5e4, Yaml::loads("v: 2.5E+4").toAssocArray()["v"].toDouble());
    EXPECT_DOUBLE_EQ(-1.2e2, Yaml::loads("v: -1.2e2").toAssocArray()["v"].toDouble());

    // .inf, -.inf, .nan
    Variant vInf = Yaml::loads("v: .inf").toAssocArray()["v"];
    EXPECT_TRUE(std::isinf(vInf.toDouble()));
    EXPECT_GT(vInf.toDouble(), 0);

    Variant vNegInf = Yaml::loads("v: -.inf").toAssocArray()["v"];
    EXPECT_TRUE(std::isinf(vNegInf.toDouble()));
    EXPECT_LT(vNegInf.toDouble(), 0);

    Variant vNan = Yaml::loads("v: .nan").toAssocArray()["v"];
    EXPECT_TRUE(std::isnan(vNan.toDouble()));

    // inf/nan ohne führenden Punkt
    EXPECT_TRUE(std::isinf(Yaml::loads("v: inf").toAssocArray()["v"].toDouble()));
    EXPECT_TRUE(std::isnan(Yaml::loads("v: nan").toAssocArray()["v"].toDouble()));

    // Ungültige Floats bleiben Strings
    EXPECT_EQ(String("1.2.3"), Yaml::loads("v: 1.2.3").toAssocArray()["v"].toString());
    EXPECT_EQ(String("192.168.1.1"), Yaml::loads("v: 192.168.1.1").toAssocArray()["v"].toString());
    EXPECT_EQ(String("3.foo"), Yaml::loads("v: 3.foo").toAssocArray()["v"].toString());
}

TEST_F(YamlTest, BooleansAndNullVariants)
{
    // Booleans
    EXPECT_TRUE(Yaml::loads("v: true").toAssocArray()["v"].toBool());
    EXPECT_TRUE(Yaml::loads("v: True").toAssocArray()["v"].toBool());
    EXPECT_TRUE(Yaml::loads("v: TRUE").toAssocArray()["v"].toBool());
    EXPECT_FALSE(Yaml::loads("v: false").toAssocArray()["v"].toBool());
    EXPECT_FALSE(Yaml::loads("v: False").toAssocArray()["v"].toBool());
    EXPECT_FALSE(Yaml::loads("v: FALSE").toAssocArray()["v"].toBool());

    // Null-Varianten
    EXPECT_TRUE(Yaml::loads("v: null").toAssocArray()["v"].isNull());
    EXPECT_TRUE(Yaml::loads("v: Null").toAssocArray()["v"].isNull());
    EXPECT_TRUE(Yaml::loads("v: NULL").toAssocArray()["v"].isNull());
    EXPECT_TRUE(Yaml::loads("v: ~").toAssocArray()["v"].isNull());
    EXPECT_TRUE(Yaml::loads("v:").toAssocArray()["v"].isNull());
}

TEST_F(YamlTest, DateAndTimeParsing)
{
    // Reines Datum
    Variant vDate = Yaml::loads("d: 2025-12-24");
    EXPECT_TRUE(vDate.toAssocArray()["d"].isDate());
    EXPECT_EQ(String("2025-12-24"), vDate.toAssocArray()["d"].toDate().toString());

    // Reine Zeit
    Variant vTime = Yaml::loads("t: 12:34:56");
    EXPECT_TRUE(vTime.toAssocArray()["t"].isTime());
    EXPECT_EQ(String("12:34:56.000000"), vTime.toAssocArray()["t"].toTime().toString());

    // DateTime mit Timezone-Varianten
    Variant vDtZ = Yaml::loads("dt: 2026-03-30T14:00:00Z");
    EXPECT_TRUE(vDtZ.toAssocArray()["dt"].isDateTime());

    Variant vDtTz = Yaml::loads("dt: 2026-03-30T14:00:00+02:00");
    EXPECT_TRUE(vDtTz.toAssocArray()["dt"].isDateTime());

    Variant vDtTzNoColon = Yaml::loads("dt: 2026-03-30T14:00:00+0200");
    EXPECT_TRUE(vDtTzNoColon.toAssocArray()["dt"].isDateTime());

    Variant vDtNegTz = Yaml::loads("dt: 2026-03-30T14:00:00-05:00");
    EXPECT_TRUE(vDtNegTz.toAssocArray()["dt"].isDateTime());

    // DateTime mit Mikrosekunden
    Variant vDtUsec = Yaml::loads("dt: 2026-03-30T14:00:00.123456Z");
    EXPECT_TRUE(vDtUsec.toAssocArray()["dt"].isDateTime());

    // Ungültige Datums-/Zeitwerte werfen keine Exception, sondern parsen als String
    EXPECT_EQ(String("2026-99-99"), Yaml::loads("d: 2026-99-99").toAssocArray()["d"].toString());
    EXPECT_EQ(String("12:75"), Yaml::loads("t: 12:75").toAssocArray()["t"].toString());
    EXPECT_EQ(String("2026-02-31T12:00:00"), Yaml::loads("dt: 2026-02-31T12:00:00").toAssocArray()["dt"].toString());
}

TEST_F(YamlTest, SingleQuotedStrings)
{
    EXPECT_EQ(String("hello world"), Yaml::loads("s: 'hello world'").toAssocArray()["s"].toString());
    EXPECT_EQ(String("it's nice"), Yaml::loads("s: 'it''s nice'").toAssocArray()["s"].toString());
    EXPECT_EQ(String(""), Yaml::loads("s: ''").toAssocArray()["s"].toString());

    bool threw = false;
    try { (void)Yaml::loads("s: 'unterminated"); }
    catch (const pplib::UnexpectedEndOfDataException&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, DoubleQuotedStringsAndEscapes)
{
    // Standard Escapes
    Variant v = Yaml::loads("s: \"a\\nb\\tc\\rd\\be\\ff\\\\\\\"\\/\\a\\v\\e\\ space\"");
    String s = v.toAssocArray()["s"].toString();
    EXPECT_NE(s.find("\n"), String::npos);
    EXPECT_NE(s.find("\t"), String::npos);
    EXPECT_NE(s.find("\r"), String::npos);
    EXPECT_NE(s.find("\b"), String::npos);
    EXPECT_NE(s.find("\f"), String::npos);
    EXPECT_NE(s.find("\\"), String::npos);
    EXPECT_NE(s.find("\""), String::npos);
    EXPECT_NE(s.find("/"), String::npos);
    EXPECT_NE(s.find("\a"), String::npos);
    EXPECT_NE(s.find("\v"), String::npos);
    EXPECT_NE(s.find("\x1b"), String::npos);
    EXPECT_NE(s.find(" space"), String::npos);

    // Null Byte
    Variant vNull = Yaml::loads("s: \"a\\0b\"");
    EXPECT_EQ(3u, vNull.toAssocArray()["s"].toString().size());

    // Escaped Quote mit Kommentar
    Variant vEsc = Yaml::loads("s: \"Text mit \\\" Anfuehrungszeichen # kein Kommentar\"\nk: 1\n");
    EXPECT_EQ(1, vEsc.toAssocArray()["k"].toInt());

    // Ungültige Escape-Sequenz wirft Exception
    bool threw = false;
    try { (void)Yaml::loads("s: \"\\q\""); }
    catch (const pplib::InvalidEscapeSequenceException&) { threw = true; }
    EXPECT_TRUE(threw);

    // Nicht geschlossener doppelter String wirft Exception
    threw = false;
    try { (void)Yaml::loads("s: \"unterminated"); }
    catch (const pplib::UnexpectedEndOfDataException&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, UnicodeEscapesAndSurrogatePairs)
{
    // \xXX (inkl. 8-Bit UTF-8)
    Variant vX = Yaml::loads("s: \"\\x41\\xE4\"");
    EXPECT_TRUE(vX.toAssocArray()["s"].isString());

    // \uXXXX (BMP)
    EXPECT_EQ(String("Héllo"), Yaml::loads("s: \"H\\u00e9llo\"").toAssocArray()["s"].toString());
    EXPECT_EQ(String("€"), Yaml::loads("s: \"\\u20AC\"").toAssocArray()["s"].toString());

    // \uXXXX Surrogate Pair mit Folgezeichen (darf Folgezeichen nicht überspringen!)
    Variant vSurr = Yaml::loads("emoji: \"\\uD83D\\uDE00Hello\"");
    String text = vSurr.toAssocArray()["emoji"].toString();
    EXPECT_TRUE(text.endsWith("Hello"));
    EXPECT_EQ(9u, text.size());

    // \UXXXXXXXX (4-Byte Codepoints)
    EXPECT_EQ(9u, Yaml::loads("emoji: \"\\U0001F600Hello\"").toAssocArray()["emoji"].toString().size());

    // Fehlerhafte Surrogate / Hex / Out of Range
    bool threw = false;
    try { (void)Yaml::loads("s: \"\\uD800\""); } // Einsames High-Surrogate
    catch (const pplib::CharacterEncodingException&) { threw = true; }
    EXPECT_TRUE(threw);

    threw = false;
    try { (void)Yaml::loads("s: \"\\uD800abc\""); } // Einsames High-Surrogate gefolgt von Text
    catch (const pplib::CharacterEncodingException&) { threw = true; }
    EXPECT_TRUE(threw);

    threw = false;
    try { (void)Yaml::loads("s: \"\\uDC00\""); } // Einsames Low-Surrogate
    catch (const pplib::CharacterEncodingException&) { threw = true; }
    EXPECT_TRUE(threw);

    threw = false;
    try { (void)Yaml::loads("s: \"\\uD800\\u0020\""); } // Ungültiges Low-Surrogate
    catch (const pplib::CharacterEncodingException&) { threw = true; }
    EXPECT_TRUE(threw);

    threw = false;
    try { (void)Yaml::loads("s: \"\\uD800\\uDC0Z\""); } // Ungültiges Hex im Low-Surrogate
    catch (const pplib::InvalidEscapeSequenceException&) { threw = true; }
    EXPECT_TRUE(threw);

    threw = false;
    try { (void)Yaml::loads("s: \"\\u000Z\""); } // Ungültiges Hex in \u
    catch (const pplib::InvalidEscapeSequenceException&) { threw = true; }
    EXPECT_TRUE(threw);

    threw = false;
    try { (void)Yaml::loads("s: \"\\U0000000Z\""); } // Ungültiges Hex in \U
    catch (const pplib::InvalidEscapeSequenceException&) { threw = true; }
    EXPECT_TRUE(threw);

    threw = false;
    try { (void)Yaml::loads("s: \"\\U00110000\""); } // Codepoint > 0x10FFFF
    catch (const pplib::CharacterEncodingException&) { threw = true; }
    EXPECT_TRUE(threw);

    threw = false;
    try { (void)Yaml::loads("s: \"\\xZZ\""); } // Ungültiges Hex in \x
    catch (const pplib::InvalidEscapeSequenceException&) { threw = true; }
    EXPECT_TRUE(threw);
}

// ============================================================================
// 3. Block-Strukturen (Mappings, Sequenzen, Block-Skalare)
// ============================================================================

TEST_F(YamlTest, BlockMappings)
{
    const char* yaml =
        "name: Alice\n"
        "age: 30\n"
        "# Kommentarzeile\n"
        "active: true # Inline-Kommentar\n"
        "title: Patrick's Library # Apostroph im Text stoppt Kommentar nicht\n";
    Variant v = Yaml::loads(yaml);
    const AssocArray& m = v.toAssocArray();
    EXPECT_EQ(String("Alice"), m["name"].toString());
    EXPECT_EQ(30, m["age"].toInt());
    EXPECT_TRUE(m["active"].toBool());
    EXPECT_EQ(String("Patrick's Library"), m["title"].toString());

    // Single-quoted key
    Variant vQuotedKey = Yaml::loads("'single_quoted_key': 123");
    EXPECT_EQ(123, vQuotedKey.toAssocArray()["single_quoted_key"].toInt());

    // Leeres !!binary in Mapping
    Variant vEmptyBin = Yaml::loads("b: !!binary\n");
    EXPECT_TRUE(vEmptyBin.toAssocArray()["b"].isByteArray());
}

TEST_F(YamlTest, BlockSequences)
{
    const char* yaml =
        "- item1\n"
        "- item2\n"
        "-   extra spaces\n"
        "-\n"
        "  nested_key: value\n";
    Variant v = Yaml::loads(yaml);
    const VariantArray& arr = v.toVariantArray();
    ASSERT_EQ(4u, arr.size());
    EXPECT_EQ(String("item1"), arr[0].toString());
    EXPECT_EQ(String("item2"), arr[1].toString());
    EXPECT_EQ(String("extra spaces"), arr[2].toString());
    EXPECT_EQ(String("value"), arr[3].toAssocArray()["nested_key"].toString());

    // Sequenz beendet durch Key auf gleicher Einrückung
    Variant vSeqEnd = Yaml::loads("- 1\n- 2\nkey: value");
    EXPECT_EQ(2u, vSeqEnd.toVariantArray().size());

    // Block-Skalar in Sequenz
    Variant vSeqScalar = Yaml::loads("- |\n  hello\n- 2\n");
    EXPECT_EQ(String("hello\n"), vSeqScalar.toVariantArray()[0].toString());
}

TEST_F(YamlTest, CompactMappingsInSequence)
{
    const char* yaml =
        "- name: Alice\n"
        "  age: 30\n"
        "- name: Bob\n"
        "  age: 25\n";
    Variant v = Yaml::loads(yaml);
    const VariantArray& arr = v.toVariantArray();
    ASSERT_EQ(2u, arr.size());
    EXPECT_EQ(String("Alice"), arr[0].toAssocArray()["name"].toString());
    EXPECT_EQ(30, arr[0].toAssocArray()["age"].toInt());
    EXPECT_EQ(String("Bob"), arr[1].toAssocArray()["name"].toString());
    EXPECT_EQ(25, arr[1].toAssocArray()["age"].toInt());
}

TEST_F(YamlTest, BlockScalarsLiteralAndFolded)
{
    // Literal |
    Variant vLit = Yaml::loads("k: |\n  line 1\n  line 2\n");
    EXPECT_EQ(String("line 1\nline 2\n"), vLit.toAssocArray()["k"].toString());

    // Folded >
    Variant vFold = Yaml::loads("k: >\n  line 1\n  line 2\n");
    EXPECT_EQ(String("line 1 line 2\n"), vFold.toAssocArray()["k"].toString());

    // Folded mit Leerzeile
    Variant vFoldEmpty = Yaml::loads("k: >\n  para 1\n\n  para 2\n");
    EXPECT_EQ(String("para 1\npara 2\n"), vFoldEmpty.toAssocArray()["k"].toString());

    // Relative Einrückung von Folgezeilen bleibt erhalten
    const char* yamlIndent =
        "code: |\n"
        "  line 1\n"
        "    indented line 2\n"
        "  line 3\n";
    Variant vIndent = Yaml::loads(yamlIndent);
    EXPECT_EQ(String("line 1\n  indented line 2\nline 3\n"), vIndent.toAssocArray()["code"].toString());

    // Block-Skalar mit nachfolgendem Key (kein Datenverlust!)
    Variant vFollow = Yaml::loads("a: |\n  text 1\n  text 2\nb: important\n");
    EXPECT_EQ(String("text 1\ntext 2\n"), vFollow.toAssocArray()["a"].toString());
    EXPECT_EQ(String("important"), vFollow.toAssocArray()["b"].toString());

    // Block-Skalar durch weniger eingerückten Folge-Key beendet
    const char* yamlParent =
        "parent:\n"
        "  block: |\n"
        "      line 1\n"
        "      line 2\n"
        "  next: 42\n";
    Variant vParent = Yaml::loads(yamlParent);
    EXPECT_EQ(String("line 1\nline 2\n"), vParent.toAssocArray()["parent/block"].toString());
    EXPECT_EQ(42, vParent.toAssocArray()["parent/next"].toInt());

    // Leerer Block-Skalar
    Variant vEmpty = Yaml::loads("k: |\n  \n");
    EXPECT_TRUE(vEmpty.toAssocArray()["k"].isString());
}

TEST_F(YamlTest, IndentationAndStructureErrors)
{
    // Tab in Einrückung wirft Exception
    bool threw = false;
    try { (void)Yaml::loads("a:\n\tb: 1"); }
    catch (const pplib::UnexpectedCharacterException&) { threw = true; }
    EXPECT_TRUE(threw);

    // Unerwartete Einrückung im Mapping
    threw = false;
    try { (void)Yaml::loads("a: 1\n  b: 2"); }
    catch (const pplib::UnexpectedCharacterException&) { threw = true; }
    EXPECT_TRUE(threw);

    // Unerwartete Einrückung in Sequenz
    threw = false;
    try { (void)Yaml::loads("- 1\n  - 2"); }
    catch (const pplib::UnexpectedCharacterException&) { threw = true; }
    EXPECT_TRUE(threw);

    // Zeile ohne Doppelpunkt im Mapping
    threw = false;
    try { (void)Yaml::loads("a: 1\nkein_key_value\nc: 3"); }
    catch (const pplib::UnexpectedCharacterException&) { threw = true; }
    EXPECT_TRUE(threw);
}

// ============================================================================
// 4. Flow-Strukturen (Flow-Style Mappings und Sequenzen)
// ============================================================================

TEST_F(YamlTest, FlowSequences)
{
    // Einzeilig
    Variant v1 = Yaml::loads("[1, 2, 3]");
    EXPECT_EQ(3u, v1.toVariantArray().size());

    // Trailing Comma
    Variant v2 = Yaml::loads("[1, 2, 3,]");
    EXPECT_EQ(3u, v2.toVariantArray().size());

    // Mehrzeilig mit Newlines
    Variant v3 = Yaml::loads("[\n  10,\n  20,\n  30\n]");
    ASSERT_EQ(3u, v3.toVariantArray().size());
    EXPECT_EQ(10, v3.toVariantArray()[0].toInt());
    EXPECT_EQ(20, v3.toVariantArray()[1].toInt());
    EXPECT_EQ(30, v3.toVariantArray()[2].toInt());

    // Flow mit Kommentaren
    Variant v4 = Yaml::loads("[\n  hello # kommentar\n, world\n]");
    ASSERT_EQ(2u, v4.toVariantArray().size());
    EXPECT_EQ(String("hello"), v4.toVariantArray()[0].toString());
    EXPECT_EQ(String("world"), v4.toVariantArray()[1].toString());

    // Single-Quotes mit '' in Flow
    Variant v5 = Yaml::loads("['it''s', 'fine']");
    ASSERT_EQ(2u, v5.toVariantArray().size());
    EXPECT_EQ(String("it's"), v5.toVariantArray()[0].toString());

    // Escaped Quote in Flow-Skalar
    Variant v6 = Yaml::loads("[\"escaped \\\" quote\"]");
    ASSERT_EQ(1u, v6.toVariantArray().size());
    EXPECT_EQ(String("escaped \" quote"), v6.toVariantArray()[0].toString());
}

TEST_F(YamlTest, FlowMappings)
{
    // Einzeilig
    Variant v1 = Yaml::loads("{a: 1, b: 2}");
    EXPECT_EQ(1, v1.toAssocArray()["a"].toInt());
    EXPECT_EQ(2, v1.toAssocArray()["b"].toInt());

    // Trailing Comma
    Variant v2 = Yaml::loads("{a: 1, b: 2,}");
    EXPECT_EQ(2, v2.toAssocArray()["b"].toInt());

    // Mehrzeilig
    Variant v3 = Yaml::loads("{\n  x: 100,\n  y: 200\n}");
    EXPECT_EQ(100, v3.toAssocArray()["x"].toInt());
    EXPECT_EQ(200, v3.toAssocArray()["y"].toInt());

    // Quoted Keys mit Escapes
    Variant v4 = Yaml::loads("{\"key\\\"name\": 42, 'key''two': 99}");
    EXPECT_EQ(42, v4.toAssocArray()["key\"name"].toInt());
    EXPECT_EQ(99, v4.toAssocArray()["key'two"].toInt());

    // Verschachtelter Flow
    Variant vNested = Yaml::loads("{list: [1, 2], map: {sub: true}}");
    EXPECT_EQ(2u, vNested.toAssocArray()["list"].toVariantArray().size());
    EXPECT_TRUE(vNested.toAssocArray()["map"].toAssocArray()["sub"].toBool());
}

TEST_F(YamlTest, FlowErrors)
{
    bool threw = false;
    try { (void)Yaml::loads("[1, 2"); } // Nicht geschlossen
    catch (const pplib::UnexpectedEndOfDataException&) { threw = true; }
    EXPECT_TRUE(threw);

    threw = false;
    try { (void)Yaml::loads("{key}"); } // Fehlender Doppelpunkt
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);

    threw = false;
    try { (void)Yaml::loads("[\"a\" \"b\"]"); } // Fehlendes Komma in Flow-Sequenz
    catch (const pplib::UnexpectedCharacterException&) { threw = true; }
    EXPECT_TRUE(threw);

    threw = false;
    try { (void)Yaml::loads("{\"a\": [1] \"b\": 2}"); } // Fehlendes Komma in Flow-Mapping
    catch (const pplib::UnexpectedCharacterException&) { threw = true; }
    EXPECT_TRUE(threw);
}

// ============================================================================
// 5. Tags, Binärdaten und Dokumentgrenzen
// ============================================================================

TEST_F(YamlTest, BinaryTagHandling)
{
    // Inline Base64
    Variant v1 = Yaml::loads("b: !!binary SGVsbG8=");
    EXPECT_TRUE(v1.toAssocArray()["b"].isByteArray());
    EXPECT_EQ(String("Hello"), v1.toAssocArray()["b"].toByteArray().toString());

    // Block-Skalar !!binary |
    Variant v2 = Yaml::loads("b: !!binary |\n  SGVsbG8=\n");
    EXPECT_TRUE(v2.toAssocArray()["b"].isByteArray());
    EXPECT_EQ(String("Hello"), v2.toAssocArray()["b"].toByteArray().toString());

    // Root-Level !!binary
    Variant v3 = Yaml::loads("!!binary SGk=");
    EXPECT_TRUE(v3.isByteArray());
    EXPECT_EQ(String("Hi"), v3.toByteArray().toString());

    // Root-Level Custom Tag wirft UnsupportedDataTypeException
    bool threwTag = false;
    try { (void)Yaml::loads("!!str hello"); }
    catch (const pplib::UnsupportedDataTypeException&) { threwTag = true; }
    EXPECT_TRUE(threwTag);

    // Root-Level !!binary |
    Variant v4 = Yaml::loads("!!binary |\n  SGk=\n");
    EXPECT_TRUE(v4.isByteArray());
    EXPECT_EQ(String("Hi"), v4.toByteArray().toString());

    // In Sequenz
    Variant v5 = Yaml::loads("- !!binary |\n  SGVsbG8=\n- !!binary SGk=\n- !!binary \"\"\n");
    const VariantArray& arr = v5.toVariantArray();
    ASSERT_EQ(3u, arr.size());
    EXPECT_EQ(String("Hello"), arr[0].toByteArray().toString());
    EXPECT_EQ(String("Hi"), arr[1].toByteArray().toString());
    EXPECT_TRUE(arr[2].isByteArray());
    EXPECT_EQ(0u, arr[2].toByteArray().size());

    // Leeres !!binary ""
    Variant vEmpty = Yaml::loads("b: !!binary \"\"");
    EXPECT_TRUE(vEmpty.toAssocArray()["b"].isByteArray());
    EXPECT_EQ(0u, vEmpty.toAssocArray()["b"].toByteArray().size());

    // Unbekannter Tag wirft Exception
    bool threw = false;
    try { (void)Yaml::loads("x: !!unknown 1"); }
    catch (const pplib::UnsupportedDataTypeException&) { threw = true; }
    EXPECT_TRUE(threw);

    threw = false;
    try { (void)Yaml::loads("- !!unknownTag 42"); }
    catch (const pplib::UnsupportedDataTypeException&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, DocumentBoundariesAndEmptyFiles)
{
    // Vollständig leere Eingabe liefert null
    EXPECT_TRUE(Yaml::loads("").isNull());
    EXPECT_TRUE(Yaml::loads("   \n# Nur Kommentar\n").isNull());

    // '---' alleinstehend liefert null
    EXPECT_TRUE(Yaml::loads("---").isNull());
    EXPECT_TRUE(Yaml::loads("---\n").isNull());

    // '---' als Dokument-Header und '...' als Footer
    Variant v = Yaml::loads("---\nname: Patrick\nage: 40\n...\n");
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_EQ(String("Patrick"), v.toAssocArray()["name"].toString());
    EXPECT_EQ(40, v.toAssocArray()["age"].toInt());

    // '---' mit Sequenz
    Variant vSeq = Yaml::loads("---\n- apple\n- banana\n...\n");
    EXPECT_TRUE(vSeq.isVariantArray());
    EXPECT_EQ(2u, vSeq.toVariantArray().size());
}

// ============================================================================
// 6. Serialisierung (Dumping)
// ============================================================================

TEST_F(YamlTest, DumpScalarTypes)
{
    // Strings (immer doppelzitiert)
    EXPECT_EQ(String("\"hello world\"\n"), Yaml::dumps(Variant(String("hello world"))));
    EXPECT_EQ(String("\"line1\\\\line2\"\n"), Yaml::dumps(Variant("line1\\line2")));
    EXPECT_EQ(String("\"a\\tb\\nc\"\n"), Yaml::dumps(Variant("a\tb\nc")));
    EXPECT_EQ(String("\"a\\bb\\fc\"\n"), Yaml::dumps(Variant("a\bb\fc")));
    EXPECT_EQ(String("\"a\\rb\"\n"), Yaml::dumps(Variant("a\rb")));

    // WideString (wird UTF-8)
    Variant vWide(WideString("Héllo"));
    String outWide = Yaml::dumps(vWide);
    EXPECT_EQ(String("Héllo"), Yaml::loads(outWide).toString());

    // Zahlen
    EXPECT_EQ(String("42\n"), Yaml::dumps(Variant(42)));
    EXPECT_EQ(String("42.0\n"), Yaml::dumps(Variant(42.0))); // Typ-Erhalt für double!
    EXPECT_EQ(String("3.14\n"), Yaml::dumps(Variant(3.14)));
    EXPECT_EQ(String(".nan\n"), Yaml::dumps(Variant(NAN)));
    EXPECT_EQ(String(".inf\n"), Yaml::dumps(Variant(std::numeric_limits<double>::infinity())));
    EXPECT_EQ(String("-.inf\n"), Yaml::dumps(Variant(-std::numeric_limits<double>::infinity())));

    // Verlustfreier Roundtrip von 17-stelligen IEEE-754 Doubles
    double precise = 1.2345678901234567;
    String outPrecise = Yaml::dumps(Variant(precise));
    Variant backPrecise = Yaml::loads(outPrecise);
    EXPECT_EQ(precise, backPrecise.toDouble());

    // Strings mit Anführungszeichen und Control-Chars
    Variant vQuote(String("He said \"hello\"\x01"));
    String outQuote = Yaml::dumps(vQuote);
    EXPECT_NE(outQuote.find("\\\""), String::npos);
    EXPECT_NE(outQuote.find("\\u0001"), String::npos);

    // Booleans & Null
    EXPECT_EQ(String("true\n"), Yaml::dumps(Variant(true)));
    EXPECT_EQ(String("false\n"), Yaml::dumps(Variant(false)));
    EXPECT_EQ(String("null\n"), Yaml::dumps(Variant(nullptr)));

    // Datum & Zeit
    EXPECT_EQ(String("2025-12-24\n"), Yaml::dumps(Variant(Date(2025, 12, 24))));
    EXPECT_EQ(String("01:01:01.000000\n"), Yaml::dumps(Variant(Time(3661))));
}

TEST_F(YamlTest, DumpCollectionsBlockAndFlow)
{
    // Leeres Mapping / Sequenz
    EXPECT_EQ(String("{}\n"), Yaml::dumps(Variant(AssocArray{}), 2));
    EXPECT_EQ(String("[]\n"), Yaml::dumps(Variant(VariantArray{}), 2));
    EXPECT_EQ(String("{}"), Yaml::dumps(Variant(AssocArray{}), 0));
    EXPECT_EQ(String("[]"), Yaml::dumps(Variant(VariantArray{}), 0));

    // FileObject Dumps für leere Mappings / Sequenzen
    MemFile mfEmpty;
    AssocArray emptyMap;
    Yaml::dump(mfEmpty, emptyMap, 2);
    Yaml::dump(mfEmpty, emptyMap, 0);
    VariantArray emptyArr;
    Yaml::dump(mfEmpty, emptyArr, 2);

    // Verschachteltes Mapping
    AssocArray d;
    d.set("a", 1);
    d.set("sub", AssocArray{});
    d.set("empty_arr", VariantArray{});
    String outMap = Yaml::dumps(d, 2);
    EXPECT_NE(outMap.find("\"a\": 1"), String::npos);
    EXPECT_NE(outMap.find("\"sub\": {}"), String::npos);
    EXPECT_NE(outMap.find(" []"), String::npos);

    // Sequenz mit verschachtelten leeren und gefüllten Arrays / Mappings / Binärdaten
    VariantArray seq;
    ByteArray ba;
    ba.append("hi", 2);
    seq.add(Variant(ba));
    seq.add(Variant(VariantArray{}));
    VariantArray innerV;
    innerV.add(Variant(1));
    seq.add(Variant(innerV));
    seq.add(Variant(AssocArray{}));
    String outSeq = Yaml::dumps(seq, 2);
    EXPECT_FALSE(outSeq.isEmpty());

    // Mappings in Sequenzen bei indent != 2 (Konsistente Einrückung!)
    VariantArray arr;
    AssocArray m1, m2;
    m1.set("name", "Alice");
    m1.set("age", 30);
    m2.set("name", "Bob");
    m2.set("age", 25);
    arr.add(m1);
    arr.add(m2);

    for (int ind : {2, 3, 4}) {
        String yaml = Yaml::dumps(arr, ind);
        Variant back = Yaml::loads(yaml);
        ASSERT_EQ(2u, back.toVariantArray().size());
        EXPECT_EQ(String("Alice"), back.toVariantArray()[0].toAssocArray()["name"].toString());
        EXPECT_EQ(30, back.toVariantArray()[0].toAssocArray()["age"].toInt());
    }

    // Flow-Style Mappings und Sequenzen
    String flowArr = Yaml::dumps(arr, 0);
    EXPECT_TRUE(flowArr.startsWith("["));
    EXPECT_TRUE(flowArr.endsWith("]"));
}

TEST_F(YamlTest, DumpArrayTypeSupport)
{
    // pplib::Array (String-Liste)
    Array arr;
    arr.add("apple");
    arr.add("banana");

    // Root Block (leer und gefüllt)
    EXPECT_EQ(String("[]\n"), Yaml::dumps(Variant(Array{}), 2));
    String outBlock = Yaml::dumps(Variant(arr), 2);
    Variant backBlock = Yaml::loads(outBlock);
    ASSERT_EQ(2u, backBlock.toVariantArray().size());
    EXPECT_EQ(String("apple"), backBlock.toVariantArray()[0].toString());

    // Root Flow
    String outFlow = Yaml::dumps(Variant(arr), 0);
    EXPECT_EQ(String("[\"apple\",\"banana\"]"), outFlow);

    // In AssocArray als explizites Variant(Array)
    AssocArray d;
    d.set("empty", Variant(Array{}));
    d.set("fruits", Variant(arr));
    String outDict = Yaml::dumps(d, 2);
    Variant backDict = Yaml::loads(outDict);
    EXPECT_EQ(String("banana"), backDict.toAssocArray()["fruits/1"].toString());

    // In VariantArray
    VariantArray varr;
    varr.add(Variant(Array{}));
    varr.add(Variant(arr));
    String outVarr = Yaml::dumps(varr, 2);
    Variant backVarr = Yaml::loads(outVarr);
    EXPECT_EQ(2u, backVarr.toVariantArray().size());
}

TEST_F(YamlTest, DumpBinaryData)
{
    ByteArray ba;
    ba.append("Hello world", 11);
    Variant v(ba);

    // Flow: !!binary "Base64"
    String outFlow = Yaml::dumps(v, 0);
    EXPECT_TRUE(outFlow.startsWith("!!binary \""));

    // Block: !!binary | mit Zeilenumbruch
    AssocArray d;
    d.set("bin", ba);
    String outBlock = Yaml::dumps(d, 2);
    EXPECT_NE(outBlock.find("!!binary |"), String::npos);

    // Langes ByteArray (> 76 Zeichen) bricht um
    ByteArray longBa;
    char a = 'A';
    for (int i = 0; i < 200; ++i) longBa.append(&a, 1);
    d.set("long", longBa);
    String outLong = Yaml::dumps(d, 2);
    EXPECT_TRUE(Yaml::loads(outLong).toAssocArray()["long"].isByteArray());
}

TEST_F(YamlTest, DumpUnsupportedTypeThrows)
{
    AssocArray d;
    d.set("bad", Variant()); // TYPE_UNKNOWN
    bool threw = false;
    try { (void)Yaml::dumps(d); }
    catch (const pplib::UnsupportedDataTypeException&) { threw = true; }
    EXPECT_TRUE(threw);
}

// ============================================================================
// 7. API-Überladungen (FileObject, AssocArray, VariantArray)
// ============================================================================

TEST_F(YamlTest, ApiOverloadsAndErrorHandling)
{
    // MemFile für AssocArray
    MemFile mfMap;
    mfMap.puts("a: 1\nb: 2\n");
    mfMap.seek(0);
    AssocArray d;
    Yaml::load(d, mfMap);
    EXPECT_EQ(2u, d.size());

    // MemFile für VariantArray
    MemFile mfArr;
    mfArr.puts("- 1\n- 2\n");
    mfArr.seek(0);
    VariantArray a;
    Yaml::load(a, mfArr);
    EXPECT_EQ(2u, a.size());

    // dumps Überladungen mit Rückgabewert
    String sD = Yaml::dumps(d);
    EXPECT_FALSE(sD.isEmpty());
    String sA = Yaml::dumps(a);
    EXPECT_FALSE(sA.isEmpty());

    // TypeConversionException wenn Wurzel nicht übereinstimmt
    bool threw = false;
    try { AssocArray wrong; Yaml::loads(wrong, "- 1\n- 2\n"); }
    catch (const pplib::TypeConversionException&) { threw = true; }
    EXPECT_TRUE(threw);

    threw = false;
    try { VariantArray wrong; Yaml::loads(wrong, "a: 1\n"); }
    catch (const pplib::TypeConversionException&) { threw = true; }
    EXPECT_TRUE(threw);

    // Auch für FileObject
    MemFile mfWrongAssoc;
    mfWrongAssoc.puts("- 1\n- 2\n");
    mfWrongAssoc.seek(0);
    threw = false;
    try { AssocArray wrong; Yaml::load(wrong, mfWrongAssoc); }
    catch (const pplib::TypeConversionException&) { threw = true; }
    EXPECT_TRUE(threw);

    MemFile mfWrongVarr;
    mfWrongVarr.puts("a: 1\n");
    mfWrongVarr.seek(0);
    threw = false;
    try { VariantArray wrong; Yaml::load(wrong, mfWrongVarr); }
    catch (const pplib::TypeConversionException&) { threw = true; }
    EXPECT_TRUE(threw);

    // dump in FileObject
    MemFile outMf;
    Yaml::dump(outMf, d, 2);
    EXPECT_GT(outMf.size(), 0u);

    MemFile outMfArr;
    Yaml::dump(outMfArr, a, 2);
    EXPECT_GT(outMfArr.size(), 0u);
}

TEST_F(YamlTest, ReentrancyAndNoExceptionPoisoning)
{
    // Exception provozieren
    try { (void)Yaml::loads("a:\n\tb: 1"); }
    catch (...) {}

    // Nachfolgendes Parsen muss sofort wieder fehlerfrei klappen
    Variant v = Yaml::loads("ok: true");
    EXPECT_TRUE(v.toAssocArray()["ok"].toBool());

    // Verschachtelter Parse-Aufruf (Reentranz)
    String innerYaml = "inner: 123";
    Variant vOuter = Yaml::loads("outer: true");
    Variant vInner = Yaml::loads(innerYaml);
    EXPECT_EQ(123, vInner.toAssocArray()["inner"].toInt());
    EXPECT_TRUE(vOuter.toAssocArray()["outer"].toBool());
}
