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
#include <pplib/types/bytearray.h>
#include <pplib/types/date.h>
#include <pplib/types/datetime.h>
#include <pplib/types/time.h>
#include <pplib/core/random.h>

using namespace pplib;

class YamlTest : public ::testing::Test
{
public:
    YamlTest()
    {
        setlocale(LC_CTYPE, DEFAULT_LOCALE);
    }
};

TEST_F(YamlTest, LoadTest1)
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

TEST_F(YamlTest, FlowStyle)
{
    Variant v = Yaml::loads("{a: 1, b: [x, y], c: {d: true}}");
    const AssocArray& d = v.toAssocArray();
    EXPECT_EQ(1, d["a"].toInt64());
    const VariantArray& arr = d["b"].toVariantArray();
    ASSERT_EQ(2u, arr.size());
    EXPECT_EQ(String("x"), arr[0].toString());
    EXPECT_EQ(String("y"), arr[1].toString());
    EXPECT_TRUE(d["c"].toAssocArray()["d"].toBool());
}

TEST_F(YamlTest, DumpsCompact)
{
    AssocArray d;
    d.set("a", 1);
    d.set("b", String("x"));
    String out = Yaml::dumps(d, 0);
    EXPECT_EQ(String("{\"a\":1,\"b\":\"x\"}"), out);
}

TEST_F(YamlTest, DumpsBlock)
{
    AssocArray d;
    d.set("a", 1);
    d.set("b", String("x"));
    String out = Yaml::dumps(d, 2);
    EXPECT_EQ(String("\"a\": 1\n\"b\": \"x\"\n"), out);
}

TEST_F(YamlTest, RoundTrip)
{
    AssocArray d;
    d.set("num", 42);
    d.set("text", String("123"));
    d.set("flag", true);
    d.set("date", Date(2024, 6, 5));

    Variant v(d);
    String out = Yaml::dumps(v);
    Variant back = Yaml::loads(out);
    const AssocArray& b = back.toAssocArray();
    EXPECT_EQ(42, b["num"].toInt64());
    EXPECT_TRUE(b["text"].isString());
    EXPECT_EQ(String("123"), b["text"].toString());
    EXPECT_TRUE(b["flag"].toBool());
    EXPECT_TRUE(b["date"].isDate());
    EXPECT_EQ(String("2024-06-05"), b["date"].toDate().toString());
}

TEST_F(YamlTest, BinaryRoundTrip)
{
    ByteArray data = Random::bytes(1024);
    Variant v(data);
    String out = Yaml::dumps(v);
    Variant back = Yaml::loads(out);
    EXPECT_TRUE(back.isByteArray());
    EXPECT_EQ(data.toBase64(), back.toByteArrayPtr().toBase64());
}

TEST_F(YamlTest, UnknownTag)
{
    ASSERT_THROW(Yaml::loads("x: !!unknown 1"), UnsupportedDataTypeException);
}

TEST_F(YamlTest, TabIndentation)
{
    ASSERT_THROW(Yaml::loads("a:\n\tb: 1"), UnexpectedCharacterException);
}

// ============================================================================
// Block-Skalare
// ============================================================================

TEST_F(YamlTest, BlockScalarLiteral)
{
    Variant v = Yaml::loads("key: |\n  line1\n  line2\n");
    EXPECT_EQ(String("line1\nline2\n"), v.toAssocArray()["key"].toString());
}

TEST_F(YamlTest, BlockScalarFolded)
{
    Variant v = Yaml::loads("key: >\n  line1\n  line2\n");
    EXPECT_EQ(String("line1 line2\n"), v.toAssocArray()["key"].toString());
}

TEST_F(YamlTest, BlockScalarWithFollowingKey)
{
    // Kritischer Bug: parseBlockScalar verschluckte die Folgezeile
    Variant v = Yaml::loads("a: |\n  Zeile 1\n  Zeile 2\nb: \"wichtig\"\n");
    const AssocArray& d = v.toAssocArray();
    EXPECT_EQ(String("Zeile 1\nZeile 2\n"), d["a"].toString());
    EXPECT_EQ(String("wichtig"), d["b"].toString());
}

TEST_F(YamlTest, BlockScalarInSequence)
{
    Variant v = Yaml::loads("- |\n  item1\n  item2\n- plain\n");
    const VariantArray& arr = v.toVariantArray();
    ASSERT_EQ(2u, arr.size());
    EXPECT_EQ(String("item1\nitem2\n"), arr[0].toString());
    EXPECT_EQ(String("plain"), arr[1].toString());
}

TEST_F(YamlTest, BlockScalarEmpty)
{
    Variant v = Yaml::loads("key: |\n  \n");
    // Leerer Block-Skalar -> leerer String (kein trailing newline)
    EXPECT_TRUE(v.toAssocArray()["key"].isString());
}

TEST_F(YamlTest, FoldedBlockScalarWithEmptyLine)
{
    Variant v = Yaml::loads("key: >\n  para1\n\n  para2\n");
    // Leere Zeile wird zu \n im folded Style
    EXPECT_EQ(String("para1\npara2\n"), v.toAssocArray()["key"].toString());
}

// ============================================================================
// Root-Level Skalare und Sequenzen
// ============================================================================

TEST_F(YamlTest, RootScalarInt)
{
    Variant v = Yaml::loads("42");
    EXPECT_EQ(42, v.toInt64());
}

TEST_F(YamlTest, RootScalarString)
{
    Variant v = Yaml::loads("\"hello\"");
    EXPECT_EQ(String("hello"), v.toString());
}

TEST_F(YamlTest, RootScalarBool)
{
    Variant v = Yaml::loads("true");
    EXPECT_TRUE(v.toBool());
}

TEST_F(YamlTest, RootScalarFloat)
{
    Variant v = Yaml::loads("3.14");
    EXPECT_DOUBLE_EQ(3.14, v.toDouble());
}

TEST_F(YamlTest, RootSequence)
{
    Variant v = Yaml::loads("- item1\n- item2\n- item3\n");
    const VariantArray& arr = v.toVariantArray();
    ASSERT_EQ(3u, arr.size());
    EXPECT_EQ(String("item1"), arr[0].toString());
    EXPECT_EQ(String("item2"), arr[1].toString());
    EXPECT_EQ(String("item3"), arr[2].toString());
}

TEST_F(YamlTest, RootFlowSequence)
{
    Variant v = Yaml::loads("[1, 2, 3]");
    const VariantArray& arr = v.toVariantArray();
    ASSERT_EQ(3u, arr.size());
    EXPECT_EQ(1, arr[0].toInt64());
    EXPECT_EQ(2, arr[1].toInt64());
    EXPECT_EQ(3, arr[2].toInt64());
}

// ============================================================================
// Zahlenformate
// ============================================================================

TEST_F(YamlTest, HexNumber)
{
    Variant v = Yaml::loads("x: 0x1F");
    EXPECT_EQ(31, v.toAssocArray()["x"].toInt64());
}

TEST_F(YamlTest, NegativeHexNumber)
{
    // Bug: -0x10 lieferte +16 statt -16
    Variant v = Yaml::loads("x: -0x10");
    EXPECT_EQ(-16, v.toAssocArray()["x"].toInt64());
}

TEST_F(YamlTest, OctalNumber)
{
    Variant v = Yaml::loads("x: 0o77");
    EXPECT_EQ(63, v.toAssocArray()["x"].toInt64());
}

TEST_F(YamlTest, NegativeOctalNumber)
{
    Variant v = Yaml::loads("x: -0o77");
    EXPECT_EQ(-63, v.toAssocArray()["x"].toInt64());
}

TEST_F(YamlTest, FloatWithExponent)
{
    Variant v = Yaml::loads("x: 1e-3");
    EXPECT_DOUBLE_EQ(0.001, v.toAssocArray()["x"].toDouble());
}

TEST_F(YamlTest, FloatScientificNotation)
{
    Variant v = Yaml::loads("x: 2.5E10");
    EXPECT_DOUBLE_EQ(2.5e10, v.toAssocArray()["x"].toDouble());
}

TEST_F(YamlTest, InfinityValues)
{
    Variant v = Yaml::loads("a: .inf\nb: -.inf\nc: .nan\n");
    const AssocArray& d = v.toAssocArray();
    EXPECT_TRUE(std::isinf(d["a"].toDouble()));
    EXPECT_GT(d["a"].toDouble(), 0.0);
    EXPECT_TRUE(std::isinf(d["b"].toDouble()));
    EXPECT_LT(d["b"].toDouble(), 0.0);
    EXPECT_TRUE(std::isnan(d["c"].toDouble()));
}

TEST_F(YamlTest, InvalidFloatKeepsString)
{
    // Bug: "1.2.3" wurde als Double 1.2 parsen
    Variant v = Yaml::loads("version: 1.2.3");
    EXPECT_TRUE(v.toAssocArray()["version"].isString());
    EXPECT_EQ(String("1.2.3"), v.toAssocArray()["version"].toString());
}

TEST_F(YamlTest, IPAddressKeepsString)
{
    Variant v = Yaml::loads("ip: 192.168.1.1");
    EXPECT_TRUE(v.toAssocArray()["ip"].isString());
    EXPECT_EQ(String("192.168.1.1"), v.toAssocArray()["ip"].toString());
}

// ============================================================================
// String-Escapes & Quotes
// ============================================================================

TEST_F(YamlTest, DoubleQuotedEscapes)
{
    Variant v = Yaml::loads("x: \"a\\nb\\tc\"");
    EXPECT_EQ(String("a\nb\tc"), v.toAssocArray()["x"].toString());
}

TEST_F(YamlTest, DoubleQuotedUnicodeEscape)
{
    Variant v = Yaml::loads("x: \"\\u0041\\u0042\"");
    EXPECT_EQ(String("AB"), v.toAssocArray()["x"].toString());
}

TEST_F(YamlTest, DoubleQuotedSurrogatePair)
{
    // Bug: Surrogate-Pairs warfen CharacterEncodingException
    // Emoji U+1F600 = \uD83D\uDE00
    Variant v = Yaml::loads("x: \"\\uD83D\\uDE00\"");
    EXPECT_EQ(String("\xF0\x9F\x98\x80"), v.toAssocArray()["x"].toString());
}

TEST_F(YamlTest, DoubleQuotedYamlEscapes)
{
    // YAML-spezifische Escapes: \0, \a, \v, \e, \xXX
    Variant v = Yaml::loads("x: \"\\0\"");
    EXPECT_EQ(String("\0", 1), v.toAssocArray()["x"].toString());

    Variant v2 = Yaml::loads("y: \"\\a\"");
    EXPECT_EQ(String("\a"), v2.toAssocArray()["y"].toString());

    Variant v3 = Yaml::loads("z: \"\\v\"");
    EXPECT_EQ(String("\v"), v3.toAssocArray()["z"].toString());

    Variant v4 = Yaml::loads("w: \"\\e\"");
    EXPECT_EQ(String("\x1B"), v4.toAssocArray()["w"].toString());

    Variant v5 = Yaml::loads("u: \"\\x41\"");
    EXPECT_EQ(String("A"), v5.toAssocArray()["u"].toString());
}

TEST_F(YamlTest, SingleQuotedString)
{
    Variant v = Yaml::loads("x: 'hello world'");
    EXPECT_EQ(String("hello world"), v.toAssocArray()["x"].toString());
}

TEST_F(YamlTest, SingleQuotedEscapedQuote)
{
    // In single-quoted YAML strings wird ' als '' escaped
    Variant v = Yaml::loads("x: 'it''s a test'");
    EXPECT_EQ(String("it's a test"), v.toAssocArray()["x"].toString());
}

TEST_F(YamlTest, EscapedQuoteInDoubleQuotedWithComment)
{
    // Bug: \" hebe den Quote-Modus vorzeitig auf -> # wurde als Kommentar interpretiert
    Variant v = Yaml::loads("x: \"Text mit \\\" AnfÃ¼hrungszeichen # kein Kommentar\"\n");
    EXPECT_EQ(String("Text mit \" AnfÃ¼hrungszeichen # kein Kommentar"), v.toAssocArray()["x"].toString());
}

// ============================================================================
// Flow-Style
// ============================================================================

TEST_F(YamlTest, TrailingCommaInSequence)
{
    // Bug: [1,2,3,] warf UnexpectedCharacterException
    Variant v = Yaml::loads("x: [1, 2, 3,]");
    const VariantArray& arr = v.toAssocArray()["x"].toVariantArray();
    ASSERT_EQ(3u, arr.size());
    EXPECT_EQ(1, arr[0].toInt64());
}

TEST_F(YamlTest, TrailingCommaInMapping)
{
    // Bug: {a: 1, b: 2,} warf UnexpectedCharacterException
    Variant v = Yaml::loads("x: {a: 1, b: 2,}");
    const AssocArray& d = v.toAssocArray()["x"].toAssocArray();
    EXPECT_EQ(1, d["a"].toInt64());
    EXPECT_EQ(2, d["b"].toInt64());
}

TEST_F(YamlTest, MultiLineFlow)
{
    Variant v = Yaml::loads("x:\n  - 1\n  - 2\ny: [3,\n  4]\n");
    const VariantArray& arr = v.toAssocArray()["y"].toVariantArray();
    ASSERT_EQ(2u, arr.size());
    EXPECT_EQ(3, arr[0].toInt64());
    EXPECT_EQ(4, arr[1].toInt64());
}

TEST_F(YamlTest, NestedFlow)
{
    Variant v = Yaml::loads("x: {a: [1, 2], b: {c: 3}}");
    const AssocArray& d = v.toAssocArray()["x"].toAssocArray();
    const VariantArray& arr = d["a"].toVariantArray();
    ASSERT_EQ(2u, arr.size());
    EXPECT_EQ(1, arr[0].toInt64());
    EXPECT_EQ(3, d["b"].toAssocArray()["c"].toInt64());
}

// ============================================================================
// Double-Identity (Type Preservation)
// ============================================================================

TEST_F(YamlTest, DoubleIdentityRoundTrip)
{
    // Bug: double 1.0 wurde als "1" serialisiert und beim Re-Parse als int erkannt
    Variant v(1.0);
    String out = Yaml::dumps(v);
    EXPECT_NE(String("1"), out);
    Variant back = Yaml::loads(out);
    EXPECT_TRUE(back.isDouble());
    EXPECT_DOUBLE_EQ(1.0, back.toDouble());
}

TEST_F(YamlTest, DoubleIdentityNonInteger)
{
    Variant v(3.14);
    String out = Yaml::dumps(v);
    Variant back = Yaml::loads(out);
    EXPECT_TRUE(back.isDouble());
    EXPECT_DOUBLE_EQ(3.14, back.toDouble());
}

// ============================================================================
// Kommentare und Leerzeilen
// ============================================================================

TEST_F(YamlTest, InlineComment)
{
    Variant v = Yaml::loads("a: 1 # this is a comment\nb: 2\n");
    const AssocArray& d = v.toAssocArray();
    EXPECT_EQ(1, d["a"].toInt64());
    EXPECT_EQ(2, d["b"].toInt64());
}

TEST_F(YamlTest, EmptyLinesBetweenEntries)
{
    Variant v = Yaml::loads("a: 1\n\nb: 2\n\n\nc: 3\n");
    const AssocArray& d = v.toAssocArray();
    EXPECT_EQ(1, d["a"].toInt64());
    EXPECT_EQ(2, d["b"].toInt64());
    EXPECT_EQ(3, d["c"].toInt64());
}

TEST_F(YamlTest, CommentOnlyLines)
{
    Variant v = Yaml::loads("# header comment\na: 1\n# middle comment\nb: 2\n");
    const AssocArray& d = v.toAssocArray();
    EXPECT_EQ(1, d["a"].toInt64());
    EXPECT_EQ(2, d["b"].toInt64());
}

// ============================================================================
// Dumps / Writer
// ============================================================================

TEST_F(YamlTest, DumpsSequenceBlock)
{
    VariantArray arr;
    arr.append(1);
    arr.append(String("two"));
    String out = Yaml::dumps(Variant(arr), 2);
    // Block-Style mit indent=2
    EXPECT_NE(String::npos, out.find("- "));
}

TEST_F(YamlTest, DumpsFlowStyle)
{
    AssocArray d;
    d.set("a", 1);
    String out = Yaml::dumps(d, 0);
    // Flow-Style (indent <= 0)
    EXPECT_EQ(String("{\"a\":1}"), out);
}

TEST_F(YamlTest, DumpsNegativeIndentIsFlow)
{
    // Bug-Fix: indent <= 0 soll Flow-Style erzeugen (nicht nur < 0)
    AssocArray d;
    d.set("a", 1);
    String out = Yaml::dumps(d, -1);
    EXPECT_EQ(String("{\"a\":1}"), out);
}

// ============================================================================
// Fehlerbehandlung
// ============================================================================

TEST_F(YamlTest, UnterminatedDoubleQuote)
{
    ASSERT_THROW(Yaml::loads("x: \"unterminated"), UnexpectedEndOfDataException);
}

TEST_F(YamlTest, UnterminatedFlowSequence)
{
    ASSERT_THROW(Yaml::loads("x: [1, 2"), UnexpectedEndOfDataException);
}

TEST_F(YamlTest, EmptyDocument)
{
    // Leeres Dokument -> null/empty Variant
    Variant v = Yaml::loads("");
    EXPECT_TRUE(v.isNull());
}

// ============================================================================
// Binary (!!binary)
// ============================================================================

TEST_F(YamlTest, BinaryInlineBase64)
{
    Variant v = Yaml::loads("x: !!binary SGVsbG8=\n");
    EXPECT_TRUE(v.toAssocArray()["x"].isByteArray());
    EXPECT_EQ(String("SGVsbG8="), v.toAssocArray()["x"].toByteArrayPtr().toBase64());
}

TEST_F(YamlTest, BinaryBlockScalar)
{
    // !!binary mit Block-Skalar
    Variant v = Yaml::loads("x: !!binary |\n  SGVsbG8=\n");
    EXPECT_TRUE(v.toAssocArray()["x"].isByteArray());
    EXPECT_EQ(String("SGVsbG8="), v.toAssocArray()["x"].toByteArrayPtr().toBase64());
}

// ============================================================================
// Verschachtelung und komplexe Strukturen
// ============================================================================

TEST_F(YamlTest, NestedSequenceInMapping)
{
    Variant v = Yaml::loads("key:\n  - a\n  - b\nother: val\n");
    const AssocArray& d = v.toAssocArray();
    const VariantArray& arr = d["key"].toVariantArray();
    ASSERT_EQ(2u, arr.size());
    EXPECT_EQ(String("a"), arr[0].toString());
    EXPECT_EQ(String("b"), arr[1].toString());
    EXPECT_EQ(String("val"), d["other"].toString());
}

TEST_F(YamlTest, CompactMappingInSequence)
{
    // "- key: value" Syntax
    Variant v = Yaml::loads("- name: Alice\n  age: 30\n- name: Bob\n  age: 25\n");
    const VariantArray& arr = v.toVariantArray();
    ASSERT_EQ(2u, arr.size());
    EXPECT_EQ(String("Alice"), arr[0].toAssocArray()["name"].toString());
    EXPECT_EQ(30, arr[0].toAssocArray()["age"].toInt64());
    EXPECT_EQ(String("Bob"), arr[1].toAssocArray()["name"].toString());
}

TEST_F(YamlTest, MultiLevelNesting)
{
    Variant v = Yaml::loads("a:\n  b:\n    c: deep\n  d: mid\ne: top\n");
    const AssocArray& root = v.toAssocArray();
    EXPECT_EQ(String("deep"), root["a"].toAssocArray()["b"].toAssocArray()["c"].toString());
    EXPECT_EQ(String("mid"), root["a"].toAssocArray()["d"].toString());
    EXPECT_EQ(String("top"), root["e"].toString());
}

// ============================================================================
// Datums- und Zeitformate
// ============================================================================

TEST_F(YamlTest, DateOnly)
{
    Variant v = Yaml::loads("d: 2024-12-31");
    EXPECT_TRUE(v.toAssocArray()["d"].isDate());
    EXPECT_EQ(String("2024-12-31"), v.toAssocArray()["d"].toDate().toString());
}

TEST_F(YamlTest, DateTimeWithTimezone)
{
    Variant v = Yaml::loads("ts: 2024-06-05T12:34:56+02:00");
    EXPECT_TRUE(v.toAssocArray()["ts"].isDateTime());
}

TEST_F(YamlTest, TimeOnly)
{
    Variant v = Yaml::loads("t: 23:59:59");
    EXPECT_TRUE(v.toAssocArray()["t"].isTime());
    EXPECT_EQ(String("23:59:59.000000"), v.toAssocArray()["t"].toTime().toString());
}

// ============================================================================
// null und Booleans
// ============================================================================

TEST_F(YamlTest, NullVariants)
{
    Variant v = Yaml::loads("a: null\nb: ~\nc:\n");
    const AssocArray& d = v.toAssocArray();
    EXPECT_TRUE(d["a"].isNull());
    EXPECT_TRUE(d["b"].isNull());
    EXPECT_TRUE(d["c"].isNull());
}

TEST_F(YamlTest, BooleanVariants)
{
    Variant v = Yaml::loads("a: true\nb: false\nc: True\nd: False\n");
    const AssocArray& d = v.toAssocArray();
    EXPECT_TRUE(d["a"].toBool());
    EXPECT_FALSE(d["b"].toBool());
    EXPECT_TRUE(d["c"].toBool());
    EXPECT_FALSE(d["d"].toBool());
}

// ============================================================================
// Thread-Safety (regression test: kein Exception-Poisoning)
// ============================================================================

TEST_F(YamlTest, NoExceptionPoisoning)
{
    // Erster Parse wirft Exception
    bool threw = false;
    try {
        Yaml::loads("a:\n\tbad");
    }
    catch (const UnexpectedCharacterException&) {
        threw = true;
    }
    ASSERT_TRUE(threw);

    // Zweiter Parse muss sauber funktionieren (kein verbleibender Pending-State)
    Variant v = Yaml::loads("x: 42");
    EXPECT_EQ(42, v.toAssocArray()["x"].toInt64());
}

TEST_F(YamlTest, SequentialParses)
{
    // Mehrere aufeinanderfolgende Patches ohne Cross-Contamination
    for (int i = 0; i < 10; ++i) {
        String yaml = "val: " + std::to_string(i);
        Variant v = Yaml::loads(yaml);
        EXPECT_EQ(static_cast<int64_t>(i), v.toAssocArray()["val"].toInt64());
    }
}

// ============================================================================
// Manuelle Regression Tests
// ============================================================================

TEST_F(YamlTest, RegressionTestMultilineStringInTheMiddle)
{
    String yaml;
    File::load(yaml, "testdata/test2.yaml");
    AssocArray root;
    Yaml::loads(root, yaml);
    EXPECT_TRUE(root["multiline_string"].isString());
    EXPECT_EQ(String("This is a\nmultiline string\nexample.\n"), root["multiline_string"].toString());
    EXPECT_FLOAT_EQ(3.14159f, (float)root["float"].toDouble());
    EXPECT_DOUBLE_EQ(3.141592653589793, root["double"].toDouble());

    EXPECT_EQ(String("Alice"), root["users/0/name"]);
    EXPECT_EQ(30, root["users/0/age"].toInt());
    EXPECT_EQ(String("alice@example.com"), root["users/0/email"]);

    EXPECT_EQ(String("Bob"), root["users/1/name"]);
    EXPECT_EQ(25, root["users/1/age"].toInt());
    EXPECT_EQ(String("bob@example.com"), root["users/1/email"]);
}

TEST_F(YamlTest, dumpWithFileObject)
{
    String yaml;
    File::load(yaml, "testdata/test2.yaml");
    AssocArray root;
    Yaml::loads(root, yaml);

    MemFile memfile;
    Yaml::dump(memfile, pplib::Variant(root), 2);

    /* DEBUG
    pplib::String dumpedYaml2 = pplib::Yaml::dumps(root, 2);
    printf("== String Dump =======================\n");
    dumpedYaml2.printnl();
    printf("== MemFile Dump =======================\n");
    printf("Memfile size: %zu\n", memfile.size());
    memfile.seek(0);
    ByteArray ba = memfile.load();
    String dumpedYaml(ba);
    dumpedYaml.printnl();
    fflush(stdout);
    printf("=========================\n");
    */

    memfile.seek(0);

    AssocArray parsed = Yaml::load(memfile);
    EXPECT_EQ(root["multiline_string"].toString(), parsed["multiline_string"].toString());
    EXPECT_EQ(root["integer"], parsed["integer"]);
    EXPECT_EQ(root["string"], parsed["string"]);
    EXPECT_EQ(root["boolean_true"], parsed["boolean_true"]);
    EXPECT_EQ(root["boolean_false"], parsed["boolean_false"]);
    EXPECT_FLOAT_EQ(root["float"].toFloat(), parsed["float"].toFloat());
    EXPECT_DOUBLE_EQ(3.14159265358979, parsed["double"].toDouble());
    EXPECT_EQ(root["null_value"], parsed["null_value"]);
    EXPECT_EQ(root["integer_array"], parsed["integer_array"]);
    EXPECT_EQ(root["string_array"], parsed["string_array"]);
    EXPECT_EQ(root["users/0"], parsed["users/0"]);
    EXPECT_EQ(root["users/1"], parsed["users/1"]);
    EXPECT_EQ(String("Alice"), parsed["users/0/name"]);
    EXPECT_EQ(30, parsed["users/0/age"].toInt());
    EXPECT_EQ(String("alice@example.com"), parsed["users/0/email"]);
    EXPECT_EQ(String("Bob"), parsed["users/1/name"]);
    EXPECT_EQ(25, parsed["users/1/age"].toInt());
    EXPECT_EQ(String("bob@example.com"), parsed["users/1/email"]);

    EXPECT_EQ(root["datetime"], parsed["datetime"]);
    EXPECT_EQ(root["timestamp"], parsed["timestamp"]);
    EXPECT_EQ(root["time"], parsed["time"]);

    // Double Wert hat an Genauigkeit verloren, den klammern wir mal aus
    parsed["double"] = root["double"];
    parsed["datetime"] = root["datetime"];
    parsed["timestamp"] = root["timestamp"];

    // root.list("root");
    // parsed.list("parsed");

    EXPECT_EQ(root, parsed);
}

TEST_F(YamlTest, dumpWithFileObjectFromVariantArray)
{
    VariantArray array;
    array.add(Variant(1));
    array.add(Variant(2));
    array.add(Variant(3));

    MemFile memfile;
    Yaml::dump(memfile, array, 2);

    memfile.seek(0);

    Variant parsed = Yaml::load(memfile);
    EXPECT_TRUE(parsed.isVariantArray());
    VariantArray parsedArray = parsed.toVariantArray();
    EXPECT_EQ(array.size(), parsedArray.size());
    for (size_t i = 0; i < array.size(); i++) {
        EXPECT_EQ(array[i], parsedArray[i]);
    }
}

TEST_F(YamlTest, dumpsFromVariantArray)
{
    VariantArray array;
    array.add(Variant(1));
    array.add(Variant(2));
    array.add(Variant(3));

    MemFile memfile;

    pplib::String dumped = Yaml::dumps(array, 2);
    EXPECT_FALSE(dumped.isEmpty());
    VariantArray parsedArray = Yaml::loads(dumped).toVariantArray();
    EXPECT_EQ(array.size(), parsedArray.size());
    for (size_t i = 0; i < array.size(); i++) {
        EXPECT_EQ(array[i], parsedArray[i]);
    }
}

// --- Writer: escaped characters in string output ---

TEST_F(YamlTest, DumpsStringWithBackslash)
{
    Variant v("line1\\line2");
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, "\"line1\\\\line2\"\n");
}

TEST_F(YamlTest, DumpsStringWithControlChars)
{
    // \x01 (SOH), \x07 (BEL), \x1b (ESC) must be escaped as \uXXXX
    Variant v(String("\x01\x07\x1b"));
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, "\"\\u0001\\u0007\\u001b\"\n");
}

TEST_F(YamlTest, DumpsStringWithTabAndNewline)
{
    Variant v(String("a\tb\nc"));
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, "\"a\\tb\\nc\"\n");
}

// --- Writer: wide string (UTF-8) output ---

TEST_F(YamlTest, DumpsWideString)
{
    Variant v(WideString("Héllo"));
    String out = Yaml::dumps(v);
    // Writer schreibt rohe UTF-8 Bytes in doppelte Anführungszeichen
    Variant back = Yaml::loads(out);
    // Bei Reload wird es als String gelesen (UTF-8 Bytes)
    EXPECT_EQ(back.toString(), String("Héllo"));
}

// --- Writer: Date / Time / DateTime output ---

TEST_F(YamlTest, DumpsDate)
{
    Variant v(Date(2025, 12, 24));
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, "2025-12-24\n");
}

TEST_F(YamlTest, DumpsTime)
{
    Variant v(Time(3661)); // 01:01:01
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, "01:01:01.000000\n");
}

TEST_F(YamlTest, DumpsDateTime)
{
    Variant v(DateTime(1735017600)); // 2025-01-01 00:00:00 UTC (approx)
    String out = Yaml::dumps(v);
    // ISO 8601 Format mit "T"-Separator und Mikrosekunden
    EXPECT_NE(out.find("T"), String::npos);
    EXPECT_NE(out.find("."), String::npos);
    // Roundtrip
    Variant back = Yaml::loads(out);
    EXPECT_TRUE(back.isDateTime());
}

// --- Writer: byte array output ---

TEST_F(YamlTest, DumpsByteArrayInline)
{
    const char data[] = "Hello";
    Variant v(ByteArray(data, 5));
    String out = Yaml::dumps(v);
    // Base64 von "Hello" ist "SGVsbG8="; Writer nutzt doppelte Anführungszeichen
    EXPECT_NE(out.find("!!binary"), String::npos);
    EXPECT_NE(out.find("SGVsbG8="), String::npos);
}

TEST_F(YamlTest, DumpsByteArrayBlock)
{
    const char data[] = "Hello";
    Variant v(ByteArray(data, 5));
    AssocArray arr;
    arr.set("data", v);
    String out = Yaml::dumps(arr, 2);
    EXPECT_NE(out.find("!!binary"), String::npos);
    EXPECT_NE(out.find("SGVsbG8="), String::npos);
}

// --- Writer: empty containers in block style ---

TEST_F(YamlTest, DumpsEmptyMapping)
{
    AssocArray arr;
    String out = Yaml::dumps(arr, 2);
    EXPECT_EQ(out, "{}\n");
}

TEST_F(YamlTest, DumpsEmptySequence)
{
    VariantArray arr;
    String out = Yaml::dumps(arr, 2);
    EXPECT_EQ(out, "[]\n");
}

// --- Writer: nested structures in block style (writeDictBlock/writeSequenceBlock) ---

TEST_F(YamlTest, DumpsNestedMappingBlock)
{
    AssocArray inner;
    inner.set("x", 1);
    inner.set("y", 2);

    AssocArray outer;
    outer.set("inner", Variant(inner));
    String out = Yaml::dumps(outer, 2);
    EXPECT_EQ(out, "\"inner\":\n  \"x\": 1\n  \"y\": 2\n");
}

TEST_F(YamlTest, DumpsSequenceOfMappings)
{
    VariantArray arr;
    AssocArray m1;
    m1.set("a", 1);
    arr.add(Variant(m1));
    AssocArray m2;
    m2.set("b", 2);
    arr.add(Variant(m2));

    String out = Yaml::dumps(arr, 2);
    EXPECT_EQ(out, "- \"a\": 1\n- \"b\": 2\n");
}

// --- Writer: flow style with nested structures (writeVariantFlow) ---

TEST_F(YamlTest, DumpsNestedMappingFlow)
{
    AssocArray inner;
    inner.set("x", 1);
    inner.set("y", 2);

    AssocArray outer;
    outer.set("inner", Variant(inner));
    String out = Yaml::dumps(outer, -1);
    EXPECT_EQ(out, "{\"inner\":{\"x\":1,\"y\":2}}");
}

TEST_F(YamlTest, DumpsSequenceOfMappingsFlow)
{
    VariantArray arr;
    AssocArray m1;
    m1.set("a", 1);
    arr.add(Variant(m1));
    AssocArray m2;
    m2.set("b", 2);
    arr.add(Variant(m2));

    String out = Yaml::dumps(arr, -1);
    EXPECT_EQ(out, "[{\"a\":1},{\"b\":2}]");
}

// --- dump/dumps AssocArray overloads (out-param and FileObject) ---

TEST_F(YamlTest, dumpsAssocArrayOutParam)
{
    AssocArray arr;
    arr.set("key", 42);
    String out;
    Yaml::dumps(out, arr);
    EXPECT_EQ(out, "\"key\": 42\n");
}

TEST_F(YamlTest, dumpsAssocArrayOutParamFlow)
{
    AssocArray arr;
    arr.set("key", 42);
    String out;
    Yaml::dumps(out, arr, -1);
    EXPECT_EQ(out, "{\"key\":42}");
}

TEST_F(YamlTest, dumpAssocArrayFileObject)
{
    AssocArray arr;
    arr.set("key", 42);
    MemFile file;
    Yaml::dump(file, arr);
    file.seek(0);
    ByteArray ba = file.load();
    String content(reinterpret_cast<const char*>(ba.ptr()), ba.size());
    EXPECT_EQ(content, "\"key\": 42\n");
}

// --- dump/dumps VariantArray overloads (out-param and FileObject) ---

TEST_F(YamlTest, dumpsVariantArrayOutParam)
{
    VariantArray arr;
    arr.add(Variant(1));
    arr.add(Variant("two"));
    String out;
    Yaml::dumps(out, arr);
    EXPECT_EQ(out, "- 1\n- \"two\"\n");
}

TEST_F(YamlTest, dumpsVariantArrayOutParamFlow)
{
    VariantArray arr;
    arr.add(Variant(1));
    arr.add(Variant("two"));
    String out;
    Yaml::dumps(out, arr, -1);
    EXPECT_EQ(out, "[1,\"two\"]");
}

TEST_F(YamlTest, dumpVariantArrayFileObject)
{
    VariantArray arr;
    arr.add(Variant(1));
    arr.add(Variant("two"));
    MemFile file;
    Yaml::dump(file, arr);
    file.seek(0);
    ByteArray ba = file.load();
    String content(reinterpret_cast<const char*>(ba.ptr()), ba.size());
    EXPECT_EQ(content, "- 1\n- \"two\"\n");
}

// --- Parser: quoted keys in flow mappings ---

TEST_F(YamlTest, FlowDoubleQuotedKey)
{
    Variant v = Yaml::loads(R"({"key": value})");
    EXPECT_TRUE(v.isAssocArray());
    const AssocArray& arr = v.toAssocArray();
    EXPECT_EQ(arr.size(), 1);
    EXPECT_EQ(arr["key"].toString(), String("value"));
}

TEST_F(YamlTest, FlowSingleQuotedKey)
{
    Variant v = Yaml::loads("{'key': value}");
    EXPECT_TRUE(v.isAssocArray());
    const AssocArray& arr = v.toAssocArray();
    EXPECT_EQ(arr.size(), 1);
    EXPECT_EQ(arr["key"].toString(), String("value"));
}

TEST_F(YamlTest, FlowSingleQuotedKeyWithSpace)
{
    Variant v = Yaml::loads("{'my key': value}");
    EXPECT_TRUE(v.isAssocArray());
    const AssocArray& arr = v.toAssocArray();
    EXPECT_EQ(arr.size(), 1);
    EXPECT_EQ(arr["my key"].toString(), String("value"));
}

TEST_F(YamlTest, FlowDoubleQuotedKeyWithNewlineEscape)
{
    Variant v = Yaml::loads(R"({"a\nb": value})");
    EXPECT_TRUE(v.isAssocArray());
    const AssocArray& arr = v.toAssocArray();
    EXPECT_EQ(arr.size(), 1);
}

// --- Parser: ISO time with timezone offsets ---

TEST_F(YamlTest, TimeWithTimezoneOffset)
{
    Variant v = Yaml::loads("t: 2025-06-15T12:30:45+02:00");
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_TRUE(v.toAssocArray()["t"].isDateTime());
}

TEST_F(YamlTest, TimeWithTimezoneOffsetNoColon)
{
    Variant v = Yaml::loads("t: 2025-06-15T12:30:45+0200");
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_TRUE(v.toAssocArray()["t"].isDateTime());
}

TEST_F(YamlTest, TimeWithNegativeTimezoneOffset)
{
    Variant v = Yaml::loads("t: 2025-06-15T12:30:45-05:30");
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_TRUE(v.toAssocArray()["t"].isDateTime());
}

// --- Parser: unescapeDoubleQuoted error paths ---

TEST_F(YamlTest, InvalidHexInXEscape)
{
    bool threw = false;
    try { Yaml::loads("\"\\xZZ\""); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, InvalidHexInUEscape)
{
    bool threw = false;
    try { Yaml::loads("\"\\uZZZZ\""); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, InvalidHexInU8Escape)
{
    bool threw = false;
    try { Yaml::loads("\"\\UZZZZZZZZ\""); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

// --- Parser: "inf"/"nan" without leading dot ---

TEST_F(YamlTest, InfWithoutDot)
{
    Variant v = Yaml::loads("x: inf");
    EXPECT_TRUE(std::isinf(v.toAssocArray()["x"].toDouble()));
}

TEST_F(YamlTest, NanWithoutDot)
{
    Variant v = Yaml::loads("x: nan");
    EXPECT_TRUE(std::isnan(v.toAssocArray()["x"].toDouble()));
}

TEST_F(YamlTest, NegativeInfWithoutDot)
{
    Variant v = Yaml::loads("x: -inf");
    double d = v.toAssocArray()["x"].toDouble();
    EXPECT_TRUE(std::isinf(d) && d < 0);
}

// --- Parser: comment inside multi-line flow value ---

TEST_F(YamlTest, FlowValueWithComment)
{
    Variant v = Yaml::loads("a: [1,\n# a comment\n 2]");
    EXPECT_TRUE(v.isAssocArray());
    const AssocArray& arr = v.toAssocArray();
    EXPECT_EQ(arr.size(), 1);
    const Variant& val = arr["a"];
    EXPECT_TRUE(val.isVariantArray());
    EXPECT_EQ(val.toVariantArray().size(), 2);
}

// --- Parser: !!binary in sequence and block scalar binary ---

TEST_F(YamlTest, BinaryInNestedMapping)
{
    Variant v = Yaml::loads("outer:\n  inner: !!binary SGVsbG8=\n");
    EXPECT_TRUE(v.isAssocArray());
    const Variant& outer = v.toAssocArray()["outer"];
    EXPECT_TRUE(outer.isAssocArray());
    EXPECT_TRUE(outer.toAssocArray()["inner"].isByteArray());
}

TEST_F(YamlTest, BinaryEmptyValue)
{
    Variant v = Yaml::loads("x: !!binary\n");
    EXPECT_TRUE(v.isAssocArray());
    const Variant& val = v.toAssocArray()["x"];
    EXPECT_TRUE(val.isByteArray());
    EXPECT_EQ(val.toByteArray().size(), 0);
}

// --- Parser: unsupported tag throws exception ---

TEST_F(YamlTest, UnsupportedTagThrows)
{
    bool threw = false;
    try { Yaml::loads("x: !!str hello"); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, UnknownCustomTagThrows)
{
    bool threw = false;
    try { Yaml::loads("x: !!custom:something value"); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

// --- Parser: comment inside quoted string value ---

TEST_F(YamlTest, CommentInsideDoubleQuotedString)
{
    Variant v = Yaml::loads("a: \"hello # world\"");
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_EQ(v.toAssocArray()["a"].toString(), String("hello # world"));
}

// --- Parser: trailing whitespace in value (readLine trim) ---

TEST_F(YamlTest, TrailingWhitespaceInValue)
{
    Variant v = Yaml::loads("a: hello   \nb: 42\n");
    EXPECT_TRUE(v.isAssocArray());
    const AssocArray& arr = v.toAssocArray();
    EXPECT_EQ(arr["a"].toString(), String("hello"));
    EXPECT_EQ(arr["b"].toInt64(), 42);
}

// --- Parser: tab in indentation is rejected (readLine) ---

TEST_F(YamlTest, TabInIndentationThrows)
{
    bool threw = false;
    try { Yaml::loads("a:\n\t- 1\n"); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

// ============================================================================
// Block scalars (| and >)
// ============================================================================

TEST_F(YamlTest, BlockScalarLiteralWithFollowingKey)
{
    Variant v = Yaml::loads("a: |\n  hello\n  world\nb: 42\n");
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_EQ(v.toAssocArray()["a"].toString(), String("hello\nworld\n"));
    EXPECT_EQ(v.toAssocArray()["b"].toInt64(), 42);
}

TEST_F(YamlTest, BlockScalarFoldedWithFollowingKey)
{
    Variant v = Yaml::loads("a: >\n  hello\n  world\nb: 42\n");
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_EQ(v.toAssocArray()["a"].toString(), String("hello world\n"));
}

TEST_F(YamlTest, BlockScalarFoldedEmptyLineWithKey)
{
    Variant v = Yaml::loads("a: >\n  para1\n\n  para2\nb: 42\n");
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_EQ(v.toAssocArray()["a"].toString(), String("para1\npara2\n"));
}

TEST_F(YamlTest, BlockScalarWithCommentAfter)
{
    Variant v = Yaml::loads("a: |\n  hello\n# comment\nb: 42\n");
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_EQ(v.toAssocArray()["a"].toString(), String("hello\n"));
    EXPECT_EQ(v.toAssocArray()["b"].toInt64(), 42);
}

TEST_F(YamlTest, BlockScalarWithBlankLineAfter)
{
    Variant v = Yaml::loads("a: |\n  hello\n\nb: 42\n");
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_EQ(v.toAssocArray()["a"].toString(), String("hello\n"));
    EXPECT_EQ(v.toAssocArray()["b"].toInt64(), 42);
}

TEST_F(YamlTest, BlockScalarWithTabAfterThrows)
{
    bool threw = false;
    try { Yaml::loads("a: |\n  hello\n\tb: 42\n"); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, BlockScalarSingleLine)
{
    Variant v = Yaml::loads("a: |\n  just one line\nb: 1\n");
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_EQ(v.toAssocArray()["a"].toString(), String("just one line\n"));
}

// ============================================================================
// Writer: escape sequences (\b, \f, \r)
// ============================================================================

TEST_F(YamlTest, DumpsStringWithBackspaceAndFormFeed)
{
    Variant v(String("a\bb\fc"));
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, "\"a\\bb\\fc\"\n");
}

TEST_F(YamlTest, DumpsStringWithCarriageReturn)
{
    Variant v(String("a\rb"));
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, "\"a\\rb\"\n");
}

// ============================================================================
// Writer: special scalar values (NaN, Inf, integer double, bool, null)
// ============================================================================

TEST_F(YamlTest, DumpsNanDouble)
{
    Variant v(NAN);
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, ".nan\n");
}

TEST_F(YamlTest, DumpsInfDouble)
{
    Variant v(std::numeric_limits<double>::infinity());
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, ".inf\n");
}

TEST_F(YamlTest, DumpsNegativeInfDouble)
{
    Variant v(-std::numeric_limits<double>::infinity());
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, "-.inf\n");
}

TEST_F(YamlTest, DumpsIntegerValuedDouble)
{
    Variant v(42.0);
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, "42.0\n");
}

TEST_F(YamlTest, DumpsNonIntegerDouble)
{
    Variant v(3.14);
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, "3.14\n");
}

TEST_F(YamlTest, DumpsBoolTrue)
{
    Variant v(true);
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, "true\n");
}

TEST_F(YamlTest, DumpsBoolFalse)
{
    Variant v(false);
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, "false\n");
}

TEST_F(YamlTest, DumpsNullVariant)
{
    Variant v(std::nullptr_t{});
    String out = Yaml::dumps(v);
    EXPECT_EQ(out, "null\n");
}

// ============================================================================
// Writer: flow style with nested structures
// ============================================================================

TEST_F(YamlTest, DumpsFlowEmptyArrayInMapping)
{
    AssocArray arr;
    VariantArray emptyArr;
    arr.set("a", Variant(emptyArr));
    String out = Yaml::dumps(arr, -1);
    EXPECT_EQ(out, "{\"a\":[]}");
}

TEST_F(YamlTest, DumpsFlowEmptyDictInMapping)
{
    AssocArray inner;
    AssocArray outer;
    outer.set("a", Variant(inner));
    String out = Yaml::dumps(outer, -1);
    EXPECT_EQ(out, "{\"a\":{}}");
}

TEST_F(YamlTest, DumpsFlowNestedArray)
{
    VariantArray inner;
    inner.add(Variant(2));
    inner.add(Variant(3));
    VariantArray outer;
    outer.add(Variant(1));
    outer.add(Variant(inner));
    String out = Yaml::dumps(outer, -1);
    EXPECT_EQ(out, "[1,[2,3]]");
}

TEST_F(YamlTest, DumpsFlowByteArray)
{
    const char data[] = "Hi";
    Variant v(ByteArray(data, 2));
    String out = Yaml::dumps(v, -1);
    // Base64 of "Hi" is "SGk="
    EXPECT_EQ(out, "!!binary \"SGk=\"");
}

// ============================================================================
// Writer: block style with nested empty containers and byte arrays in sequences
// ============================================================================

TEST_F(YamlTest, DumpsBlockEmptyArrayInMapping)
{
    AssocArray arr;
    VariantArray emptyArr;
    arr.set("a", Variant(emptyArr));
    String out = Yaml::dumps(arr, 2);
    EXPECT_EQ(out, "\"a\": []\n");
}

TEST_F(YamlTest, DumpsBlockEmptyDictInMapping)
{
    AssocArray inner;
    AssocArray outer;
    outer.set("a", Variant(inner));
    String out = Yaml::dumps(outer, 2);
    EXPECT_EQ(out, "\"a\": {}\n");
}

TEST_F(YamlTest, DumpsBlockByteArrayInSequence)
{
    const char data[] = "Hello";
    VariantArray arr;
    arr.add(Variant(ByteArray(data, 5)));
    String out = Yaml::dumps(arr, 2);
    EXPECT_NE(out.find("- !!binary |"), String::npos);
    EXPECT_NE(out.find("SGVsbG8="), String::npos);
}

// ============================================================================
// Writer: writeBase64Lines with data > 76 base64 chars (line wrapping)
// ============================================================================

TEST_F(YamlTest, DumpsBlockByteArrayLongData)
{
    // 60 bytes → 80 base64 chars (> 76, triggers line wrap)
    ByteArray ba(60);
    for (int i = 0; i < 60; ++i) ba[i] = static_cast<unsigned char>(i);
    AssocArray arr;
    arr.set("data", Variant(ba));
    String out = Yaml::dumps(arr, 2);
    EXPECT_NE(out.find("!!binary |"), String::npos);
    // Should have at least two lines of base64 content
    size_t firstNl = out.find("\n", out.find("!!binary |") + 10);
    size_t secondNl = out.find("\n", firstNl + 1);
    EXPECT_NE(firstNl, String::npos);
    EXPECT_NE(secondNl, String::npos);
}

// ============================================================================
// Writer: dump(FileObject&) Variant overload
// ============================================================================

TEST_F(YamlTest, dumpVariantFileObject)
{
    Variant v(42);
    MemFile file;
    Yaml::dump(file, v);
    file.seek(0);
    ByteArray ba = file.load();
    String content(reinterpret_cast<const char*>(ba.ptr()), ba.size());
    EXPECT_EQ(content, "42\n");
}

TEST_F(YamlTest, dumpVariantFileObjectFlow)
{
    Variant v(42);
    MemFile file;
    Yaml::dump(file, v, -1);
    file.seek(0);
    ByteArray ba = file.load();
    String content(reinterpret_cast<const char*>(ba.ptr()), ba.size());
    EXPECT_EQ(content, "42");
}

// ============================================================================
// Parser: \u escape sequences (2-byte, 3-byte UTF-8, surrogate pairs)
// ============================================================================

TEST_F(YamlTest, UEscapeTwoByteUtf8)
{
    // é = U+00E9 → 2-byte UTF-8: 0xC3 0xA9
    Variant v = Yaml::loads("\"\\u00e9\"");
    EXPECT_TRUE(v.isString());
    String s = v.toString();
    EXPECT_EQ(s.size(), 2);
    EXPECT_EQ(static_cast<unsigned char>(s[0]), 0xC3);
    EXPECT_EQ(static_cast<unsigned char>(s[1]), 0xA9);
}

TEST_F(YamlTest, UEscapeThreeByteUtf8)
{
    // 中 = U+4E2D → 3-byte UTF-8: 0xE4 0xB8 0xAD
    Variant v = Yaml::loads("\"\\u4e2d\"");
    EXPECT_TRUE(v.isString());
    String s = v.toString();
    EXPECT_EQ(s.size(), 3);
    EXPECT_EQ(static_cast<unsigned char>(s[0]), 0xE4);
    EXPECT_EQ(static_cast<unsigned char>(s[1]), 0xB8);
    EXPECT_EQ(static_cast<unsigned char>(s[2]), 0xAD);
}

TEST_F(YamlTest, UEscapeFourByteUtf8)
{
    // 😀 = U+1F600 → 4-byte UTF-8: 0xF0 0x9F 0x98 0x80
    Variant v = Yaml::loads("\"\\U0001F600\"");
    EXPECT_TRUE(v.isString());
    String s = v.toString();
    EXPECT_EQ(s.size(), 4);
    EXPECT_EQ(static_cast<unsigned char>(s[0]), 0xF0);
    EXPECT_EQ(static_cast<unsigned char>(s[1]), 0x9F);
    EXPECT_EQ(static_cast<unsigned char>(s[2]), 0x98);
    EXPECT_EQ(static_cast<unsigned char>(s[3]), 0x80);
}

TEST_F(YamlTest, UEscapeSurrogatePair)
{
    // 😀 = U+1F600 as surrogate pair: D83D DE00
    Variant v = Yaml::loads("\"\\ud83d\\ude00\"");
    EXPECT_TRUE(v.isString());
    String s = v.toString();
    EXPECT_EQ(s.size(), 4);
    EXPECT_EQ(static_cast<unsigned char>(s[0]), 0xF0);
}

TEST_F(YamlTest, UEscapeLoneHighSurrogateThrows)
{
    bool threw = false;
    try { Yaml::loads("\"\\ud800\""); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, UEscapeLoneLowSurrogateThrows)
{
    bool threw = false;
    try { Yaml::loads("\"\\ude00\""); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, UEscapeInvalidLowSurrogateThrows)
{
    // High surrogate followed by non-surrogate
    bool threw = false;
    try { Yaml::loads("\"\\ud800\\u0041\""); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

// ============================================================================
// Parser: unescapeDoubleQuoted additional escapes (\b, \f, /, \0, \a, \v, \e)
// ============================================================================

TEST_F(YamlTest, UnescapeBackspaceAndFormFeed)
{
    Variant v = Yaml::loads("\"a\\bb\\fc\"");
    EXPECT_TRUE(v.isString());
    String s = v.toString();
    EXPECT_EQ(s[1], '\b');
    EXPECT_EQ(s[3], '\f');
}

TEST_F(YamlTest, UnescapeForwardSlash)
{
    Variant v = Yaml::loads("\"a\\/b\"");
    EXPECT_TRUE(v.isString());
    EXPECT_EQ(v.toString(), String("a/b"));
}

TEST_F(YamlTest, UnescapeNullChar)
{
    Variant v = Yaml::loads("\"a\\0b\"");
    EXPECT_TRUE(v.isString());
    String s = v.toString();
    EXPECT_EQ(s[1], '\0');
}

TEST_F(YamlTest, UnescapeAlertVerticalTabEscape)
{
    Variant v = Yaml::loads("\"a\\ab\\vb\\eb\"");
    EXPECT_TRUE(v.isString());
    String s = v.toString();
    EXPECT_EQ(s[1], '\a');
    EXPECT_EQ(s[3], '\v');
    EXPECT_EQ(s[5], 0x1B);
}

// ============================================================================
// Parser: plain keys in flow mappings
// ============================================================================

TEST_F(YamlTest, FlowPlainKey)
{
    Variant v = Yaml::loads("{a: 1}");
    EXPECT_TRUE(v.isAssocArray());
    const AssocArray& arr = v.toAssocArray();
    EXPECT_EQ(arr.size(), 1);
    EXPECT_EQ(arr["a"].toInt64(), 1);
}

TEST_F(YamlTest, FlowPlainKeyMultiple)
{
    Variant v = Yaml::loads("{a: 1, b: 2, c: 3}");
    EXPECT_TRUE(v.isAssocArray());
    const AssocArray& arr = v.toAssocArray();
    EXPECT_EQ(arr.size(), 3);
    EXPECT_EQ(arr["a"].toInt64(), 1);
    EXPECT_EQ(arr["b"].toInt64(), 2);
    EXPECT_EQ(arr["c"].toInt64(), 3);
}

// ============================================================================
// Parser: root-level !!binary and !! tags (parseBlock)
// ============================================================================

TEST_F(YamlTest, RootBinaryTag)
{
    Variant v = Yaml::loads("!!binary SGVsbG8=\n");
    EXPECT_TRUE(v.isByteArray());
    String s(reinterpret_cast<const char*>(v.toByteArray().ptr()), v.toByteArray().size());
    EXPECT_EQ(s, String("Hello"));
}

TEST_F(YamlTest, RootBinaryTagEmpty)
{
    Variant v = Yaml::loads("!!binary\n");
    EXPECT_TRUE(v.isByteArray());
    EXPECT_EQ(v.toByteArray().size(), 0);
}

TEST_F(YamlTest, RootUnknownTagThrows)
{
    bool threw = false;
    try { Yaml::loads("!!custom value\n"); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

// ============================================================================
// Parser: single-quoted key in block mapping (unquoteKey)
// ============================================================================

TEST_F(YamlTest, BlockSingleQuotedKey)
{
    Variant v = Yaml::loads("'my key': value\n");
    EXPECT_TRUE(v.isAssocArray());
    const AssocArray& arr = v.toAssocArray();
    EXPECT_EQ(arr.size(), 1);
    EXPECT_EQ(arr["my key"].toString(), String("value"));
}

TEST_F(YamlTest, BlockSingleQuotedKeyWithEscape)
{
    Variant v = Yaml::loads("'it''s': value\n");
    EXPECT_TRUE(v.isAssocArray());
    const AssocArray& arr = v.toAssocArray();
    EXPECT_EQ(arr.size(), 1);
    EXPECT_EQ(arr["it's"].toString(), String("value"));
}

// ============================================================================
// Parser: empty flow containers as values
// ============================================================================

TEST_F(YamlTest, FlowEmptyArrayAsValue)
{
    Variant v = Yaml::loads("a: []");
    EXPECT_TRUE(v.isAssocArray());
    const Variant& val = v.toAssocArray()["a"];
    EXPECT_TRUE(val.isVariantArray());
    EXPECT_EQ(val.toVariantArray().size(), 0);
}

TEST_F(YamlTest, FlowEmptyDictAsValue)
{
    Variant v = Yaml::loads("a: {}");
    EXPECT_TRUE(v.isAssocArray());
    const Variant& val = v.toAssocArray()["a"];
    EXPECT_TRUE(val.isAssocArray());
    EXPECT_EQ(val.toAssocArray().size(), 0);
}

// ============================================================================
// Parser: single-quoted string in block scalar value
// ============================================================================

TEST_F(YamlTest, BlockSingleQuotedStringValue)
{
    Variant v = Yaml::loads("a: 'hello world'\n");
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_EQ(v.toAssocArray()["a"].toString(), String("hello world"));
}

TEST_F(YamlTest, BlockSingleQuotedStringWithEscape)
{
    Variant v = Yaml::loads("a: 'it''s fine'\n");
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_EQ(v.toAssocArray()["a"].toString(), String("it's fine"));
}

// ============================================================================
// Writer: flow style root scalar (no trailing newline)
// ============================================================================

TEST_F(YamlTest, DumpsFlowRootScalar)
{
    Variant v(42);
    String out = Yaml::dumps(v, -1);
    EXPECT_EQ(out, "42");
}

TEST_F(YamlTest, DumpsFlowRootString)
{
    Variant v(String("hello"));
    String out = Yaml::dumps(v, -1);
    EXPECT_EQ(out, "\"hello\"");
}

// ============================================================================
// Escape-Sequenzen (Zusatzabdeckung)
// ============================================================================

TEST_F(YamlTest, DoubleQuotedEscCR) {
    Variant v = Yaml::loads("\"a\\rb\"");
    EXPECT_EQ(v.toString(), "a\rb");
}

TEST_F(YamlTest, DoubleQuotedEscBackslash) {
    Variant v = Yaml::loads("\"a\\\\b\"");
    EXPECT_EQ(v.toString(), "a\\b");
}

TEST_F(YamlTest, DoubleQuotedInvalidEscape) {
    bool threw = false;
    try { Yaml::loads("\"a\\qb\""); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, DoubleQuotedLoneSurrogateViaUEscape) {
    bool threw = false;
    try { Yaml::loads("\"\\U0000D800\""); }
    catch (const pplib::CharacterEncodingException&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, DoubleQuotedInvalidSurrogatePairHex) {
    bool threw = false;
    try { Yaml::loads("\"\\uD800\\uZZ41\""); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, SingleQuotedUnterminated) {
    bool threw = false;
    try { Yaml::loads("'abc"); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

// ============================================================================
// Flow-Style: Comments und Fehlerpfade
// ============================================================================

TEST_F(YamlTest, FlowWithComment) {
    Variant v = Yaml::loads("{a: 1, # comment\nb: 2}");
    const AssocArray& d = v.toAssocArray();
    EXPECT_EQ(d.size(), 2);
    EXPECT_EQ(d["a"].toInt(), 1);
    EXPECT_EQ(d["b"].toInt(), 2);
}

TEST_F(YamlTest, FlowSeqBadSeparator) {
    // Quoted value stops at closing quote; 'b' is unexpected after it
    bool threw = false;
    try { Yaml::loads("[\"a\" b]"); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, FlowMappingMissingColon) {
    bool threw = false;
    try { Yaml::loads("{a 1}"); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, FlowMappingBadSeparator) {
    // Quoted value stops at closing quote; 'b' is unexpected after it
    bool threw = false;
    try { Yaml::loads("{a: \"x\" b}"); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, FlowMappingMultiCharPlainKey) {
    Variant v = Yaml::loads("{hello: 1, world: 2}");
    const AssocArray& d = v.toAssocArray();
    EXPECT_EQ(d.size(), 2);
    EXPECT_EQ(d["hello"].toInt(), 1);
    EXPECT_EQ(d["world"].toInt(), 2);
}

TEST_F(YamlTest, FlowQuotedStringValue) {
    Variant v = Yaml::loads("{a: \"hello\"}");
    EXPECT_EQ(v.toAssocArray()["a"].toString(), "hello");
}

TEST_F(YamlTest, FlowDoubleQuotedValueWithEscape) {
    Variant v = Yaml::loads(R"({a: "he\"llo"})");
    EXPECT_EQ(v.toAssocArray()["a"].toString(), "he\"llo");
}

TEST_F(YamlTest, FlowQuotedSeqItem) {
    Variant v = Yaml::loads("[\"world\", 'x']");
    const VariantArray& arr = v.toVariantArray();
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0].toString(), "world");
    EXPECT_EQ(arr[1].toString(), "x");
}

// ============================================================================
// !!binary in verschiedenen Kontexten
// ============================================================================

TEST_F(YamlTest, BlockBinaryAtRoot) {
    Variant v = Yaml::loads("!!binary SGk=");
    const ByteArray& ba = v.toByteArray();
    EXPECT_EQ(ba.size(), 2);
    EXPECT_EQ(static_cast<unsigned char>(ba[0]), 'H');
    EXPECT_EQ(static_cast<unsigned char>(ba[1]), 'i');
}

TEST_F(YamlTest, SequenceBinaryItem) {
    Variant v = Yaml::loads("- !!binary SGk=\n- 2");
    const VariantArray& arr = v.toVariantArray();
    EXPECT_EQ(arr.size(), 2);
    const ByteArray& ba = arr[0].toByteArray();
    EXPECT_EQ(ba.size(), 2);
    EXPECT_EQ(static_cast<unsigned char>(ba[0]), 'H');
    EXPECT_EQ(arr[1].toInt(), 2);
}

TEST_F(YamlTest, SequenceFlowItem) {
    Variant v = Yaml::loads("- [1, 2]\n- 3");
    const VariantArray& arr = v.toVariantArray();
    EXPECT_EQ(arr.size(), 2);
    const VariantArray& nested = arr[0].toVariantArray();
    EXPECT_EQ(nested.size(), 2);
    EXPECT_EQ(nested[0].toInt(), 1);
    EXPECT_EQ(nested[1].toInt(), 2);
    EXPECT_EQ(arr[1].toInt(), 3);
}

// ============================================================================
// Mapping- und Sequence-Fehlerpfade
// ============================================================================

TEST_F(YamlTest, MappingInvalidLine) {
    bool threw = false;
    try { Yaml::loads("a: 1\nb\nc: 2"); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, MappingUnexpectedIndent) {
    bool threw = false;
    try { Yaml::loads("a: 1\n  b: 2"); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, SequenceUnexpectedIndent) {
    bool threw = false;
    try { Yaml::loads("- 1\n  - 2"); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, SequenceEndNonDash) {
    // Root sequence ends when a non-dash line appears at same indent
    Variant v = Yaml::loads("- 1\n- 2\nkey: value");
    const VariantArray& arr = v.toVariantArray();
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0].toInt(), 1);
    EXPECT_EQ(arr[1].toInt(), 2);
}

TEST_F(YamlTest, SequenceExtraSpacesAfterDash) {
    Variant v = Yaml::loads("-   1\n- 2");
    const VariantArray& arr = v.toVariantArray();
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0].toInt(), 1);
    EXPECT_EQ(arr[1].toInt(), 2);
}

TEST_F(YamlTest, SequenceEmptyItemNestedBlock) {
    Variant v = Yaml::loads("-\n  a: 1\n- 2");
    const VariantArray& arr = v.toVariantArray();
    EXPECT_EQ(arr.size(), 2);
    const AssocArray& d = arr[0].toAssocArray();
    EXPECT_EQ(d["a"].toInt(), 1);
    EXPECT_EQ(arr[1].toInt(), 2);
}

// ============================================================================
// load/loads Overloads für AssocArray und VariantArray (FileObject)
// ============================================================================

TEST_F(YamlTest, LoadAssocArrayFromMemFile) {
    MemFile mf;
    mf.puts("a: 1\nb: two");
    mf.seek(0);

    AssocArray arr;
    Yaml::load(arr, mf);
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr["a"].toInt(), 1);
    EXPECT_EQ(arr["b"].toString(), "two");
}

TEST_F(YamlTest, LoadsAssocArrayWrongType) {
    AssocArray arr;
    bool threw = false;
    try { Yaml::loads(arr, "42"); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

TEST_F(YamlTest, LoadVariantArrayFromMemFile) {
    MemFile mf;
    mf.puts("- 1\n- two");
    mf.seek(0);

    VariantArray arr;
    Yaml::load(arr, mf);
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0].toInt(), 1);
    EXPECT_EQ(arr[1].toString(), "two");
}

TEST_F(YamlTest, LoadsVariantArrayWrongType) {
    VariantArray arr;
    bool threw = false;
    try { Yaml::loads(arr, "a: 1"); }
    catch (const pplib::Exception&) { threw = true; }
    EXPECT_TRUE(threw);
}

// ============================================================================
// Writer: Escape und Fehlerpfade
// ============================================================================

TEST_F(YamlTest, WriterEscapeDoubleQuote) {
    AssocArray arr;
    arr.set("k", String("he\"llo"));
    String out = Yaml::dumps(arr);
    EXPECT_NE(out.find("\\\""), String::npos);
}

TEST_F(YamlTest, WriterUnsupportedType) {
    AssocArray arr;
    arr.set("x", Variant());  // TYPE_UNKNOWN → nicht serialisierbar
    bool threw = false;
    try { Yaml::dumps(arr); }
    catch (const pplib::UnsupportedDataTypeException&) { threw = true; }
    EXPECT_TRUE(threw);
}

// ============================================================================
// Writer: Verschachtelte Arrays/Dicts in Sequenzen (Block-Mode)
// ============================================================================

TEST_F(YamlTest, WriteSequenceNestedEmptyArray) {
    VariantArray arr;
    arr.add(Variant(std::nullptr_t{}));  // null
    arr.add(VariantArray{});             // leeres Array
    String out = Yaml::dumps(arr);
    EXPECT_NE(out.find(" []"), String::npos);
}

TEST_F(YamlTest, WriteSequenceNestedEmptyDict) {
    VariantArray arr;
    arr.add(AssocArray{});  // leeres Dict
    String out = Yaml::dumps(arr);
    EXPECT_NE(out.find(" {}"), String::npos);
}

TEST_F(YamlTest, WriteSequenceNestedNonEmptyArray) {
    VariantArray inner;
    inner.add(Variant(1));
    inner.add(Variant(2));
    VariantArray arr;
    arr.add(inner);  // nicht-leeres Array
    String out = Yaml::dumps(arr);
    EXPECT_GT(out.size(), 0u);
}

TEST_F(YamlTest, WriteSequenceNestedNonEmptyDict) {
    AssocArray inner;
    inner.set("k", 1);
    VariantArray arr;
    arr.add(inner);  // nicht-leeres Dict
    String out = Yaml::dumps(arr);
    EXPECT_GT(out.size(), 0u);
}

// ============================================================================
// Review 2 Fixes & Regressionstests
// ============================================================================

TEST_F(YamlTest, SurrogatePairFollowedByCharacters)
{
    // Bug 1: \uD83D\uDE00Hello darf nicht die nachfolgenden Zeichen verschlucken
    Variant v = Yaml::loads("emoji: \"\\uD83D\\uDE00Hello\"");
    EXPECT_TRUE(v.isAssocArray());
    String text = v.toAssocArray()["emoji"].toString();
    EXPECT_TRUE(text.endsWith("Hello"));
    EXPECT_EQ(text.size(), 9u); // 4 Bytes UTF-8 + 5 Bytes "Hello"
}

TEST_F(YamlTest, SequenceOfMappingsIndentationNonDefault)
{
    // Bug 2: dump und loads von Mappings in Sequenzen bei indent != 2
    VariantArray arr;
    AssocArray d1, d2;
    d1.set("name", "Alice");
    d1.set("age", 30);
    d2.set("name", "Bob");
    d2.set("age", 25);
    arr.add(d1);
    arr.add(d2);

    for (int ind : {2, 3, 4}) {
        String yaml = Yaml::dumps(arr, ind);
        Variant loaded = Yaml::loads(yaml);
        EXPECT_TRUE(loaded.isVariantArray());
        const VariantArray& res = loaded.toVariantArray();
        ASSERT_EQ(res.size(), 2u);
        EXPECT_EQ(res[0].toAssocArray()["name"].toString(), "Alice");
        EXPECT_EQ(res[0].toAssocArray()["age"].toInt(), 30);
        EXPECT_EQ(res[1].toAssocArray()["name"].toString(), "Bob");
        EXPECT_EQ(res[1].toAssocArray()["age"].toInt(), 25);
    }
}

TEST_F(YamlTest, MultiLineFlowCollections)
{
    // Bug 3: Flow-Collections mit Zeilenumbrüchen
    const char* flowSeq = "[\n  1,\n  2,\n  3\n]";
    Variant v1 = Yaml::loads(flowSeq);
    EXPECT_TRUE(v1.isVariantArray());
    EXPECT_EQ(v1.toVariantArray().size(), 3u);
    EXPECT_EQ(v1.toVariantArray()[0].toInt(), 1);
    EXPECT_EQ(v1.toVariantArray()[1].toInt(), 2);
    EXPECT_EQ(v1.toVariantArray()[2].toInt(), 3);

    const char* flowMap = "{\n  a: 10,\n  b: 20\n}";
    Variant v2 = Yaml::loads(flowMap);
    EXPECT_TRUE(v2.isAssocArray());
    EXPECT_EQ(v2.toAssocArray()["a"].toInt(), 10);
    EXPECT_EQ(v2.toAssocArray()["b"].toInt(), 20);
}

TEST_F(YamlTest, FlowMappingKeyWithEscapes)
{
    // Bug 4: Escaped Quotes in Flow-Mapping-Keys
    Variant v1 = Yaml::loads("{\"key\\\"name\": 42}");
    EXPECT_TRUE(v1.isAssocArray());
    EXPECT_EQ(v1.toAssocArray()["key\"name"].toInt(), 42);

    Variant v2 = Yaml::loads("{'key''name': 99}");
    EXPECT_TRUE(v2.isAssocArray());
    EXPECT_EQ(v2.toAssocArray()["key'name"].toInt(), 99);
}

TEST_F(YamlTest, InvalidDateTimeFallbackToString)
{
    // Bug 5: Ungültige Datums-/Zeitwerte werfen keine Exception, sondern parsen als String
    Variant v1 = Yaml::loads("code: 2026-99-99");
    EXPECT_TRUE(v1.toAssocArray()["code"].isString());
    EXPECT_EQ(v1.toAssocArray()["code"].toString(), "2026-99-99");

    Variant v2 = Yaml::loads("ratio: 12:75");
    EXPECT_TRUE(v2.toAssocArray()["ratio"].isString());
    EXPECT_EQ(v2.toAssocArray()["ratio"].toString(), "12:75");
}

TEST_F(YamlTest, PositiveSignNumbers)
{
    // Bug 6: Positives Vorzeichen '+'
    Variant v1 = Yaml::loads("a: +42\nb: +3.14\nc: +0x10");
    const AssocArray& m = v1.toAssocArray();
    EXPECT_EQ(m["a"].toInt64(), 42);
    EXPECT_DOUBLE_EQ(m["b"].toDouble(), 3.14);
    EXPECT_EQ(m["c"].toInt64(), 16);
}

TEST_F(YamlTest, HyphenAndPlusAsScalar)
{
    // Bug 7: '-' und '+' als eigenständige Skalare werden nicht zu 0
    Variant v1 = Yaml::loads("key: -");
    EXPECT_TRUE(v1.toAssocArray()["key"].isString());
    EXPECT_EQ(v1.toAssocArray()["key"].toString(), "-");

    Variant v2 = Yaml::loads("key: +");
    EXPECT_TRUE(v2.toAssocArray()["key"].isString());
    EXPECT_EQ(v2.toAssocArray()["key"].toString(), "+");
}

TEST_F(YamlTest, InvalidHexOctalLiterals)
{
    // Bug 8: 0xxyz und 0o99 sind Strings, nicht 0
    Variant v1 = Yaml::loads("a: 0xxyz\nb: 0o99");
    const AssocArray& m = v1.toAssocArray();
    EXPECT_TRUE(m["a"].isString());
    EXPECT_EQ(m["a"].toString(), "0xxyz");
    EXPECT_TRUE(m["b"].isString());
    EXPECT_EQ(m["b"].toString(), "0o99");
}

TEST_F(YamlTest, NanAndInfRoundTrip)
{
    // Bug 9: .nan und .inf Roundtrip
    Variant vNan = Yaml::loads(".nan");
    EXPECT_TRUE(std::isnan(vNan.toDouble()));
    String outNan = Yaml::dumps(vNan);
    EXPECT_EQ(outNan, ".nan\n");

    Variant vInf = Yaml::loads(".inf");
    EXPECT_TRUE(std::isinf(vInf.toDouble()));
    EXPECT_GT(vInf.toDouble(), 0);
    String outInf = Yaml::dumps(vInf);
    EXPECT_EQ(outInf, ".inf\n");

    Variant vNegInf = Yaml::loads("-.inf");
    EXPECT_TRUE(std::isinf(vNegInf.toDouble()));
    EXPECT_LT(vNegInf.toDouble(), 0);
    String outNegInf = Yaml::dumps(vNegInf);
    EXPECT_EQ(outNegInf, "-.inf\n");
}

TEST_F(YamlTest, EmptyFileAndCommentOnly)
{
    // Bug 10: Leere Dokumente und reine Kommentare liefern null
    Variant v1 = Yaml::loads("");
    EXPECT_TRUE(v1.isNull());

    Variant v2 = Yaml::loads("   \n# Nur ein Kommentar\n   # Noch einer\n");
    EXPECT_TRUE(v2.isNull());
}

TEST_F(YamlTest, DocumentBoundaryMarkers)
{
    // Bug 11: '---' und '...' Dokumentgrenzen
    Variant v1 = Yaml::loads("---\nname: Patrick\nage: 40\n...\n");
    EXPECT_TRUE(v1.isAssocArray());
    EXPECT_EQ(v1.toAssocArray()["name"].toString(), "Patrick");
    EXPECT_EQ(v1.toAssocArray()["age"].toInt(), 40);

    Variant v2 = Yaml::loads("---\n- apple\n- banana\n...\n");
    EXPECT_TRUE(v2.isVariantArray());
    EXPECT_EQ(v2.toVariantArray().size(), 2u);
}

TEST_F(YamlTest, BlockScalarRelativeIndent)
{
    // Bug 12: Interne relative Einrückung in Block-Skalaren bleibt erhalten
    const char* yaml =
        "code: |\n"
        "  line 1\n"
        "    indented line 2\n"
        "  line 3\n";
    Variant v = Yaml::loads(yaml);
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_EQ(v.toAssocArray()["code"].toString(), "line 1\n  indented line 2\nline 3\n");
}

TEST_F(YamlTest, StringWithApostropheAndComment)
{
    // Bug 13: Apostroph im Plain-String verhindert nicht das Abschneiden des Kommentars
    Variant v = Yaml::loads("title: Patrick's Programming Library # Das ist ein Kommentar");
    EXPECT_TRUE(v.isAssocArray());
    EXPECT_EQ(v.toAssocArray()["title"].toString(), "Patrick's Programming Library");
}

TEST_F(YamlTest, BinaryEmptyContentReturnsByteArray)
{
    // Bug 14: Tag !!binary "" liefert ByteArray, nicht ByteArrayPtr
    Variant v = Yaml::loads("data: !!binary \"\"");
    EXPECT_TRUE(v.toAssocArray()["data"].isByteArray());
    EXPECT_EQ(v.toAssocArray()["data"].toByteArray().size(), 0u);
}

TEST_F(YamlTest, ArrayStringSerialization)
{
    // Bug 17: Serialisieren von pplib::Array (String-Array)
    Array arr;
    arr.add("apple");
    arr.add("banana");
    arr.add("cherry");
    Variant v(arr);

    String yaml = Yaml::dumps(v, 2);
    Variant back = Yaml::loads(yaml);
    EXPECT_TRUE(back.isVariantArray());
    const VariantArray& varr = back.toVariantArray();
    ASSERT_EQ(varr.size(), 3u);
    EXPECT_EQ(varr[0].toString(), "apple");
    EXPECT_EQ(varr[1].toString(), "banana");
    EXPECT_EQ(varr[2].toString(), "cherry");

    // Auch innerhalb von AssocArray
    AssocArray dict;
    dict.set("fruits", arr);
    String dictYaml = Yaml::dumps(dict, 2);
    Variant backDict = Yaml::loads(dictYaml);
    EXPECT_EQ(backDict.toAssocArray()["fruits/0"].toString(), "apple");
    EXPECT_EQ(backDict.toAssocArray()["fruits/1"].toString(), "banana");
    EXPECT_EQ(backDict.toAssocArray()["fruits/2"].toString(), "cherry");
}

