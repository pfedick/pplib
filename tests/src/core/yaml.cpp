#include <locale.h>
#include <gtest/gtest.h>
#include "pplib-tests.h"
#include <pplib/core/yaml.h>
#include <pplib/core/fileobject.h>
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
