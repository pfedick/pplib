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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <pplib/core/json.h>
#include <pplib/core/file.h>
#include <pplib/exceptions.h>
#include <pplib/types/array.h>
#include <pplib/core/functions.h>
#include <gtest/gtest.h>
#include "pplib-tests.h"

namespace
{

// The fixture for testing class Foo.
class JsonTest : public ::testing::Test
{
protected:
    JsonTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~JsonTest()
    {
    }
};

TEST_F(JsonTest, ParseFromStringWithDictToAssocArray)
{
    pplib::String text;
    pplib::File::load(text, "testdata/jsontest1.json");

    pplib::AssocArray data;
    try {
        pplib::Json::loads(data, text);
    }
    catch (const pplib::Exception& exp) {
        exp.print();
        FAIL() << "unexpected exception occured";
    }

    EXPECT_EQ((size_t)15, data.size());
    EXPECT_EQ(pplib::String("Root Value"), data.getString("RootKey"));
    EXPECT_EQ(pplib::String("newline\n, tab\ttab, backs\\ash"), data.getString("EscapeSecences"));
    EXPECT_EQ(12345, data.get("integer").toInt64());
    EXPECT_DOUBLE_EQ(17.999, data.get("float").toDouble());
    EXPECT_TRUE(data.exists("dict"));
    EXPECT_EQ(pplib::String("value1"), data.getString("dict/innerdict_key1"));
    EXPECT_EQ(pplib::String("value2"), data.getString("dict/key2"));
    EXPECT_TRUE(data.exists("empty_array"));
    EXPECT_TRUE(data.get("empty_array").isVariantArray());
    EXPECT_EQ((size_t)0, data.get("empty_array").toVariantArray().size());
    EXPECT_TRUE(data.exists("empty_dict"));
    EXPECT_TRUE(data.get("empty_dict").isAssocArray());
    EXPECT_EQ((size_t)0, data.get("empty_dict").toAssocArray().size());
    EXPECT_TRUE(data.exists("empty_string"));
    EXPECT_EQ(pplib::String(""), data.getString("empty_string"));
    EXPECT_EQ(pplib::String("String1"), data.getString("array_same_line/0"));
    EXPECT_EQ(pplib::String("String2"), data.getString("array_same_line/1"));
    EXPECT_EQ(pplib::String("String3"), data.getString("array_same_line/2"));
    EXPECT_EQ(pplib::String("String1"), data.getString("array_multiline/0"));
    EXPECT_EQ(pplib::String("String2"), data.getString("array_multiline/1"));
    EXPECT_DOUBLE_EQ(22.443, data.get("array_multiline/2").toDouble());
    EXPECT_EQ(pplib::String("schachtel1"), data.getString("array_multiline/3/0"));
    EXPECT_EQ(pplib::String("schachtel2"), data.getString("array_multiline/3/1"));
    EXPECT_EQ(pplib::String("value1"), data.getString("array_multiline/4/schachteldict1"));
    EXPECT_EQ(pplib::String("value2"), data.getString("array_multiline/4/schachteldict2"));
    EXPECT_EQ(12345, data.get("array_multiline/5").toInt64());
    EXPECT_EQ(pplib::String("Dieser String geht über\nmehrere Zeilen."), data.getString("Zeilenumbruch"));
    EXPECT_EQ(pplib::String("value2"), data.getString("key2"));
    EXPECT_TRUE(data.get("true").toBool());
    EXPECT_FALSE(data.get("false").toBool());
    EXPECT_TRUE(data.get("null").isNull());
}

TEST_F(JsonTest, ParseFromStringWithArrayToVariantArray)
{
    pplib::String text;
    pplib::File::load(text, "testdata/jsontest2.json");

    pplib::VariantArray data;
    try {
        pplib::Json::loads(data, text);
    }
    catch (const pplib::Exception& exp) {
        exp.print();
        FAIL() << "unexpected exception occured";
    }
    EXPECT_EQ((size_t)3, data.size());
    EXPECT_EQ(pplib::String("value1"), data[0].toString());
    ASSERT_TRUE(data[1].isAssocArray());
    EXPECT_EQ(pplib::String("inner_value1"), data[1].toAssocArray().getString("key1"));
    EXPECT_EQ(pplib::String("inner_value2"), data[1].toAssocArray().getString("key2"));
    EXPECT_EQ(pplib::String("value3"), data[2].toString());

    // Root Array nach AssocArray laden muss fehlschlagen
    pplib::AssocArray assoc;
    EXPECT_THROW(pplib::Json::loads(assoc, text), pplib::TypeConversionException);
}

TEST_F(JsonTest, NegativTest_GarbageBeforeBegin)
{
    pplib::String text;
    pplib::File::load(text, "testdata/jsontest3.json");

    pplib::Variant data;
    ASSERT_THROW(pplib::Json::loads(data, text), pplib::UnexpectedCharacterException);
}
TEST_F(JsonTest, NegativTest_GarbageAfterEnd)
{
    pplib::String text;
    pplib::File::load(text, "testdata/jsontest4.json");
    pplib::Variant data;
    ASSERT_THROW(pplib::Json::loads(data, text), pplib::UnexpectedCharacterException);
}
TEST_F(JsonTest, NegativTest_InvalidKey)
{
    pplib::String text("{ \"key1\": \"value1\", must_fail: \"value2\", \"key3\": \"value3\"}");
    pplib::AssocArray data;
    ASSERT_THROW(pplib::Json::loads(data, text), pplib::UnexpectedCharacterException);
}

TEST_F(JsonTest, NegativTest_MissingValue)
{
    pplib::String text("{ \"key1\": \"value1\", \"key2\":, \"key3\": \"value3\"}");
    pplib::AssocArray data;
    ASSERT_THROW(pplib::Json::loads(data, text), pplib::UnexpectedCharacterException);
}

TEST_F(JsonTest, DumpsEmptyArrayToString)
{
    pplib::AssocArray data;
    pplib::String str;
    ASSERT_NO_THROW({ str = pplib::Json::dumps(data); });
    ASSERT_EQ(pplib::String("{}"), str);
}

TEST_F(JsonTest, DumpsSimpleKeyValue)
{
    pplib::AssocArray data;
    data.set("key1", "value1");
    data.set("key2", int64_t(12345));
    data.set("true", true);
    data.set("false", false);
    data.set("null", pplib::Variant(nullptr));
    data.set("ipaddress", "127.0.0.1");
    data.set("float", -344.123);
    data.set("notfloat", "344,123");
    data.set("wide", pplib::Variant(pplib::WideString(L"widestring")));
    pplib::String str;
    ASSERT_NO_THROW({ str = pplib::Json::dumps(data); });
    ASSERT_EQ(pplib::String("{\"false\":false,\"float\":-344.123,\"ipaddress\":\"127.0.0.1\",\"key1\":\"value1\","
                            "\"key2\":12345,\"notfloat\":\"344,123\",\"null\":null,\"true\":true,"
                            "\"wide\":\"widestring\"}"),
              str);
}

TEST_F(JsonTest, DumpsNestetAssocArray)
{
    pplib::AssocArray data;
    data.set("key1", "value1");
    data.set("key2/innerkey1", "value2");
    data.set("key2/innerkey2/innerst1", "value3");
    data.set("key2/innerkey2/innerst2", "value4");
    data.set("key3", "value1");
    pplib::String str;
    ASSERT_NO_THROW({ str = pplib::Json::dumps(data); });
    ASSERT_EQ(pplib::String("{\"key1\":\"value1\",\"key2\":{\"innerkey1\":\"value2\",\"innerkey2\":"
                            "{\"innerst1\":\"value3\",\"innerst2\":\"value4\"}},\"key3\":\"value1\"}"),
              str);

    // str.printnl();
}

TEST_F(JsonTest, DumpsSimpleListAtFirstLevel)
{
    pplib::VariantArray data;
    data.add(pplib::String("value1"));
    data.add(pplib::String("value2"));
    data.add(pplib::String("value3"));
    data.add(pplib::String("value4"));
    data.add(pplib::String("value1"));
    pplib::String str;
    ASSERT_NO_THROW({ str = pplib::Json::dumps(data); });
    ASSERT_EQ(pplib::String("[\"value1\",\"value2\",\"value3\",\"value4\",\"value1\"]"), str);
}

TEST_F(JsonTest, DumpsNestedListAtFirstLevel)
{
    pplib::VariantArray data;
    data.add(pplib::String("value1"));
    pplib::AssocArray sub1;
    sub1.set("innerkey1", "value2");
    data.add(sub1);
    pplib::AssocArray sub2;
    sub2.set("innerkey2/innerst1", "value3");
    sub2.set("innerkey2/innerst2", "value4");
    data.add(sub2);
    data.add(pplib::String("value1"));
    pplib::String str;
    ASSERT_NO_THROW({
        try {
            str = pplib::Json::dumps(data);
        }
        catch (const pplib::Exception& exp) {
            exp.print();
            throw;
        }
    });
    ASSERT_EQ(pplib::String("[\"value1\",{\"innerkey1\":\"value2\"},{\"innerkey2\":{\"innerst1\":"
                            "\"value3\",\"innerst2\":\"value4\"}},\"value1\"]"),
              str);
}

TEST_F(JsonTest, DumpsNestedListAtSecondLevel)
{
    pplib::AssocArray data;
    data.set("key1", "value1");
    data.set("key2/[]", "value2");
    data.set("key2/[]", "value3");
    data.set("key2/[]", "value4");
    data.set("key3", "value5");
    pplib::String str;
    ASSERT_NO_THROW({ str = pplib::Json::dumps(data); });
    ASSERT_EQ(pplib::String("{\"key1\":\"value1\",\"key2\":[\"value2\",\"value3\","
                            "\"value4\"],\"key3\":\"value5\"}"),
              str);
}

TEST_F(JsonTest, DumpsNestedListWithRealArray)
{
    pplib::AssocArray data;
    pplib::Array a;
    a.add("str1");
    a.add("str2");
    a.add("str3");
    data.set("key1", "value1");
    data.set("key2", a);
    data.set("key3", "value5");
    pplib::String str;
    ASSERT_NO_THROW({ str = pplib::Json::dumps(data); });
    ASSERT_EQ(pplib::String("{\"key1\":\"value1\",\"key2\":[\"str1\",\"str2\",\"str3\"],"
                            "\"key3\":\"value5\"}"),
              str);
}

TEST_F(JsonTest, DumpsWithBinary)
{
    pplib::AssocArray data;
    pplib::ByteArray ba = pplib::Random::bytes(1024);
    pplib::String b64 = ba.toBase64();
    data.set("key1", "value1");
    data.set("bytearray", ba);
    data.set("key3", "value3");
    pplib::String str;
    ASSERT_NO_THROW({ str = pplib::Json::dumps(data); });
    ASSERT_EQ(pplib::String("{\"bytearray\":\"" + b64 + "\",\"key1\":\"value1\",\"key3\":\"value3\"}"), str);
}

// DateTime, Date, Time, TimeDelta, TimeZone serialization tests would go here.

TEST_F(JsonTest, DumpsWithDateTime)
{
    pplib::AssocArray data;
    pplib::DateTime dt(2024, 6, 5, 12, 34, 56, 789000, pplib::TimeZone(+2, 0));
    data.set("key1", "value1");
    data.set("datetime", dt);
    data.set("key3", "value3");
    pplib::String str;
    ASSERT_NO_THROW({ str = pplib::Json::dumps(data); });
    ASSERT_EQ(pplib::String("{\"datetime\":\"" + dt.getISO8601withUsec() + "\",\"key1\":\"value1\",\"key3\":\"value3\"}"), str);
}

TEST_F(JsonTest, DumpsWithDate)
{
    pplib::AssocArray data;
    pplib::Date date(2024, 6, 5);
    data.set("key1", "value1");
    data.set("date", date);
    data.set("key3", "value3");
    pplib::String str;
    ASSERT_NO_THROW({ str = pplib::Json::dumps(data); });
    ASSERT_EQ(pplib::String("{\"date\":\"" + date.toString() + "\",\"key1\":\"value1\",\"key3\":\"value3\"}"), str);
}

TEST_F(JsonTest, DumpsWithTime)
{
    pplib::AssocArray data;
    pplib::Time time(12, 34, 56, 789000);
    data.set("key1", "value1");
    data.set("time", time);
    data.set("key3", "value3");
    pplib::String str;
    ASSERT_NO_THROW({ str = pplib::Json::dumps(data); });
    ASSERT_EQ(pplib::String("{\"key1\":\"value1\",\"key3\":\"value3\",\"time\":\"" + time.toString() + "\"}"), str);
}

TEST_F(JsonTest, DumpsWithTimeDelta)
{
    pplib::AssocArray data;
    pplib::TimeDelta td(1, 2, 3, 456000);
    data.set("key1", "value1");
    data.set("timedelta", td);
    data.set("key3", "value3");
    pplib::String str;
    ASSERT_NO_THROW({ str = pplib::Json::dumps(data); });
    ASSERT_EQ(pplib::String("{\"key1\":\"value1\",\"key3\":\"value3\",\"timedelta\":\"" + td.toString() + "\"}"), str);
}

TEST_F(JsonTest, DumpsWithTimeZone)
{
    pplib::AssocArray data;
    pplib::TimeZone tz(+2, 0);
    data.set("key1", "value1");
    data.set("timezone", tz);
    data.set("key3", "value3");
    pplib::String str;
    ASSERT_NO_THROW({ str = pplib::Json::dumps(data); });
    ASSERT_EQ(pplib::String("{\"key1\":\"value1\",\"key3\":\"value3\",\"timezone\":\"" + tz.toString() + "\"}"), str);
}

TEST_F(JsonTest, DumpsWithIndentation)
{
    pplib::AssocArray data;
    data.set("key1", "value1");
    data.set("key2/[]", int64_t(1));
    data.set("key2/[]", int64_t(2));
    pplib::String str = pplib::Json::dumps(data, 4);
    pplib::String expected =
        "{\n"
        "    \"key1\": \"value1\",\n"
        "    \"key2\": [\n"
        "        1,\n"
        "        2\n"
        "    ]\n"
        "}\n";
    ASSERT_EQ(expected, str);
}

TEST_F(JsonTest, PrettyPrint)
{
    pplib::String json = "{\"a\":1,\"b\":[true,false,null]}";
    pplib::String pp = pplib::Json::pp(json, 2);
    pplib::String expected =
        "{\n"
        "  \"a\": 1,\n"
        "  \"b\": [\n"
        "    true,\n"
        "    false,\n"
        "    null\n"
        "  ]\n"
        "}\n";
    ASSERT_EQ(expected, pp);
}

TEST_F(JsonTest, ParseRootPrimitives)
{
    EXPECT_EQ(pplib::Json::loads("12345").toInt64(), 12345);
    EXPECT_DOUBLE_EQ(pplib::Json::loads("123.456").toDouble(), 123.456);
    EXPECT_EQ(pplib::Json::loads("\"hello world\"").toString(), "hello world");
    EXPECT_TRUE(pplib::Json::loads("true").toBool());
    EXPECT_FALSE(pplib::Json::loads("false").toBool());
    EXPECT_TRUE(pplib::Json::loads("null").isNull());
}

TEST_F(JsonTest, TrailingCommaInObjectThrows)
{
    pplib::String json = "{\"a\": 1, }";
    EXPECT_THROW(pplib::Json::loads(json), pplib::UnexpectedCharacterException);
}

TEST_F(JsonTest, TrailingCommaInArrayThrows)
{
    pplib::String json = "[1, 2, ]";
    EXPECT_THROW(pplib::Json::loads(json), pplib::UnexpectedCharacterException);
}

TEST_F(JsonTest, StringControlCharacterEscaping)
{
    pplib::VariantArray arr;
    arr.add(pplib::String("line1\nline2\ttab\bback\fform\rreturn\"quote\\slash"));
    pplib::String dumped = pplib::Json::dumps(arr);
    pplib::Variant loaded = pplib::Json::loads(dumped);
    ASSERT_TRUE(loaded.isVariantArray());
    EXPECT_EQ(loaded.toVariantArray()[0].toString(), "line1\nline2\ttab\bback\fform\rreturn\"quote\\slash");
}

} // namespace
