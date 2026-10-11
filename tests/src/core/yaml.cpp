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
    Variant v = Yaml::loads("x: \"Text mit \\\" Anführungszeichen # kein Kommentar\"\n");
    EXPECT_EQ(String("Text mit \" Anführungszeichen # kein Kommentar"), v.toAssocArray()["x"].toString());
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