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
#include <limits>
#include <pplib/core/json.h>
#include <pplib/core/file.h>
#include <pplib/core/memfile.h>
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
    pplib::String expected = "{\n"
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
    pplib::String expected = "{\n"
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

TEST_F(JsonTest, readJsonTest5)
{
    pplib::File file("testdata/jsontest5.json");
    pplib::Variant data = pplib::Json::load(file);
    ASSERT_TRUE(data.isAssocArray());
    const pplib::AssocArray& a = data.toAssocArray();
    ASSERT_EQ(12345, a.getInt("integer"));
    ASSERT_EQ(pplib::String("hello world"), a.getString("string"));
    ASSERT_TRUE(a.get("boolean_true").toBool());
    ASSERT_FALSE(a.get("boolean_false").toBool());
    ASSERT_TRUE(a.get("null").isNull());
    ASSERT_FALSE(a.exists("non_existent_key"));
    ASSERT_TRUE(a.get("double").isDouble());
    ASSERT_EQ(3.14159, a.get("double").toDouble());
    ASSERT_TRUE(a.get("array").isVariantArray());
    const pplib::VariantArray& arr = a.getVariantArray("array");
    ASSERT_EQ(3, arr.size());
    ASSERT_TRUE(arr[0].isAssocArray());
    ASSERT_TRUE(arr[1].isAssocArray());
    ASSERT_TRUE(arr[2].isInt64());
    ASSERT_EQ(pplib::String("patrick"), a.getString("array/0/name"));
    ASSERT_EQ(1001, a.getInt("array/0/uid"));
    ASSERT_EQ(pplib::String("nick"), a.getString("array/1/name"));
    ASSERT_EQ(1002, a.getInt("array/1/uid"));
    ASSERT_EQ(12345, arr[2].toInt64());
    ASSERT_EQ(pplib::String("value10"), a.getString("subkey/subarray/0/key10"));
    pplib::Array string_array = a.getArray("string_array");
    ASSERT_EQ(4, string_array.size());
    ASSERT_EQ(pplib::String("red"), string_array[0]);
    ASSERT_EQ(pplib::String("green"), string_array[1]);
    ASSERT_EQ(pplib::String("blue"), string_array[2]);
    ASSERT_EQ(pplib::String("white"), string_array[3]);
}

TEST_F(JsonTest, readBeatportDataOneLineer)
{
    // Example test for reading Beatport data in one line
    pplib::File file("testdata/data_one_line.json");
    pplib::Variant data = pplib::Json::load(file);
    ASSERT_TRUE(data.isAssocArray());
    const pplib::AssocArray& a = data.toAssocArray();
    ASSERT_TRUE(a.exists("props/pageProps/track"));
    const pplib::AssocArray& track = a.getAssocArray("props/pageProps/track");
    // track.list();
    ASSERT_TRUE(track.exists("track_name"));
    ASSERT_EQ(pplib::String("Firefly "), track.getString("track_name"));
    ASSERT_EQ(pplib::String("Extended Mix"), track.getString("mix_name"));
    ASSERT_EQ(pplib::String("D Minor"), track.getString("key"));
    ASSERT_TRUE(track.get("bpm").isInt64());
    ASSERT_EQ(140, track.getInt64t("bpm"));
    ASSERT_EQ(179000, track.getInt("sample_start_ms"));
    ASSERT_TRUE(track.exists("artists"));
    ASSERT_TRUE(track.get("artists").isVariantArray());
    const pplib::VariantArray& artists = track.getVariantArray("artists");
    ASSERT_EQ(1, artists.size());
    const pplib::AssocArray& artist = artists[0].toAssocArray();
    ASSERT_TRUE(artist.exists("name"));
    ASSERT_EQ(pplib::String("Rob Binner"), artist.getString("name"));
    ASSERT_TRUE(artist.exists("id"));
    ASSERT_EQ(595934, artist.getInt("id"));
    ASSERT_EQ(pplib::String("Extrema Global Music"), track.getString("label/name"));
    ASSERT_EQ(pplib::String("Trance (Main Floor)"), track.getString("genre/name"));
    ASSERT_EQ(pplib::String("Uplifting Trance"), track.getString("genre/sub_genre/name"));
}

TEST_F(JsonTest, readBeatportDataIndented)
{
    // Example test for reading Beatport data in one line
    pplib::File file("testdata/data_indented.json");
    pplib::Variant data = pplib::Json::load(file);
    ASSERT_TRUE(data.isAssocArray());
    const pplib::AssocArray& a = data.toAssocArray();
    ASSERT_TRUE(a.exists("props/pageProps/track"));
    const pplib::AssocArray& track = a.getAssocArray("props/pageProps/track");
    // track.list();
    ASSERT_TRUE(track.exists("track_name"));
    ASSERT_EQ(pplib::String("Firefly "), track.getString("track_name"));
    ASSERT_EQ(pplib::String("Extended Mix"), track.getString("mix_name"));
    ASSERT_EQ(pplib::String("D Minor"), track.getString("key"));
    ASSERT_TRUE(track.get("bpm").isInt64());
    ASSERT_EQ(140, track.getInt64t("bpm"));
    ASSERT_EQ(179000, track.getInt("sample_start_ms"));
    ASSERT_TRUE(track.exists("artists"));
    ASSERT_TRUE(track.get("artists").isVariantArray());
    const pplib::VariantArray& artists = track.getVariantArray("artists");
    ASSERT_EQ(1, artists.size());
    const pplib::AssocArray& artist = artists[0].toAssocArray();
    ASSERT_TRUE(artist.exists("name"));
    ASSERT_EQ(pplib::String("Rob Binner"), artist.getString("name"));
    ASSERT_TRUE(artist.exists("id"));
    ASSERT_EQ(595934, artist.getInt("id"));
    ASSERT_EQ(pplib::String("Extrema Global Music"), track.getString("label/name"));
    ASSERT_EQ(pplib::String("Trance (Main Floor)"), track.getString("genre/name"));
    ASSERT_EQ(pplib::String("Uplifting Trance"), track.getString("genre/sub_genre/name"));
}

TEST_F(JsonTest, writeClassicArray)
{
    pplib::Array array("red,green,blue,white", ",");
    pplib::Variant v(array);
    pplib::String json = pplib::Json::dumps(v);
    ASSERT_EQ(pplib::String("[\"red\",\"green\",\"blue\",\"white\"]"), json);

    // Empty Array
    pplib::Array emptyArray;
    pplib::Variant emptyVariant(emptyArray);
    pplib::String emptyJson = pplib::Json::dumps(emptyVariant);
    ASSERT_EQ(pplib::String("[]"), emptyJson);
}

TEST_F(JsonTest, SurrogatePairsFromFile)
{
    pplib::File file("testdata/jsontest_surrogates.json");
    pplib::Variant data = pplib::Json::load(file);
    ASSERT_TRUE(data.isAssocArray());
    const pplib::AssocArray& a = data.toAssocArray();
    EXPECT_EQ(pplib::String("😀"), a.getString("emoji_direct"));
    EXPECT_EQ(pplib::String("😀"), a.getString("emoji_surrogate"));
    EXPECT_EQ(a.getString("emoji_direct"), a.getString("emoji_surrogate"));
    EXPECT_EQ(pplib::String("𝄞"), a.getString("gclef_surrogate"));
    EXPECT_EQ(pplib::String("Hello 😃 World!"), a.getString("mixed"));
    EXPECT_EQ(pplib::String("A"), a.getString("ascii_escape"));
    EXPECT_EQ(pplib::String("ä€"), a.getString("bmp_escape"));
}

TEST_F(JsonTest, SurrogatePairsRoundtripAndEdgeCases)
{
    // Valid surrogate pairs via loads
    pplib::Variant v1 = pplib::Json::loads("\"\\uD83D\\uDE00\"");
    EXPECT_EQ(pplib::String("😀"), v1.toString());

    pplib::Variant v2 = pplib::Json::loads("\"\\uD834\\uDD1E\"");
    EXPECT_EQ(pplib::String("𝄞"), v2.toString());

    // High surrogate followed by non-escape character
    EXPECT_THROW(pplib::Json::loads("\"\\uD83Dabc\""), pplib::CharacterEncodingException);

    // High surrogate followed by escape other than \u
    EXPECT_THROW(pplib::Json::loads("\"\\uD83D\\n\""), pplib::CharacterEncodingException);

    // High surrogate followed by invalid low surrogate (e.g. BMP char)
    EXPECT_THROW(pplib::Json::loads("\"\\uD83D\\u0041\""), pplib::CharacterEncodingException);

    // High surrogate followed by another high surrogate
    EXPECT_THROW(pplib::Json::loads("\"\\uD83D\\uD83D\""), pplib::CharacterEncodingException);

    // Lone high surrogate at end of string
    EXPECT_THROW(pplib::Json::loads("\"\\uD83D\""), pplib::CharacterEncodingException);

    // Lone low surrogate
    EXPECT_THROW(pplib::Json::loads("\"\\uDE00\""), pplib::CharacterEncodingException);

    // Invalid hex digit in first \u
    EXPECT_THROW(pplib::Json::loads("\"\\u12G4\""), pplib::InvalidEscapeSequenceException);

    // Invalid hex digit in low surrogate \u
    EXPECT_THROW(pplib::Json::loads("\"\\uD83D\\uDE0G\""), pplib::InvalidEscapeSequenceException);

    // Incomplete escape sequence at EOF
    EXPECT_THROW(pplib::Json::loads("\"\\u12"), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Json::loads("\"\\uD83D\\u12"), pplib::UnexpectedEndOfDataException);
}

TEST_F(JsonTest, CommentsParsing)
{
    pplib::File file("testdata/jsontest_comments.json");
    pplib::Variant data = pplib::Json::load(file);
    ASSERT_TRUE(data.isAssocArray());
    const pplib::AssocArray& a = data.toAssocArray();
    EXPECT_EQ(pplib::String("Patrick"), a.getString("name"));
    ASSERT_TRUE(a.get("items").isVariantArray());
    const pplib::VariantArray& items = a.getVariantArray("items");
    ASSERT_EQ(3, items.size());
    EXPECT_EQ(1, items[0].toInt64());
    EXPECT_EQ(2, items[1].toInt64());
    EXPECT_EQ(3, items[2].toInt64());

    // Single slash not followed by slash throws UnexpectedCharacterException
    EXPECT_THROW(pplib::Json::loads("{\"a\": /}"), pplib::UnexpectedCharacterException);
}

TEST_F(JsonTest, EscapeSequencesAndControlChars)
{
    // Escaped forward slash
    pplib::Variant v = pplib::Json::loads("\"http:\\/\\/example.com\\/path\"");
    EXPECT_EQ(pplib::String("http://example.com/path"), v.toString());

    // Invalid escape sequences
    EXPECT_THROW(pplib::Json::loads("\"\\a\""), pplib::InvalidEscapeSequenceException);
    EXPECT_THROW(pplib::Json::loads("\"\\x\""), pplib::InvalidEscapeSequenceException);
    EXPECT_THROW(pplib::Json::loads("\"\\1\""), pplib::InvalidEscapeSequenceException);

    // Backslash at EOF
    EXPECT_THROW(pplib::Json::loads("\"abc\\"), pplib::UnexpectedEndOfDataException);

    // Unclosed string at EOF (from memory and from file)
    EXPECT_THROW(pplib::Json::loads("\"unterminated"), pplib::UnexpectedEndOfDataException);
    {
        pplib::File f("testdata/jsontest_unterminated.json");
        EXPECT_THROW(pplib::Json::load(f), pplib::UnexpectedEndOfDataException);
    }

    // Control characters < 32 escaping in dumps and re-parsing
    pplib::VariantArray arr;
    arr.add(pplib::String("\x01\x02\x1f"));
    pplib::String dumped = pplib::Json::dumps(arr);
    EXPECT_EQ(pplib::String("[\"\\u0001\\u0002\\u001f\"]"), dumped);
    pplib::Variant reloaded = pplib::Json::loads(dumped);
    ASSERT_TRUE(reloaded.isVariantArray());
    EXPECT_EQ(pplib::String("\x01\x02\x1f"), reloaded.toVariantArray()[0].toString());
}

TEST_F(JsonTest, NumberFormatsAndEdgeCases)
{
    // Scientific notation
    EXPECT_DOUBLE_EQ(1e5, pplib::Json::loads("1e5").toDouble());
    EXPECT_DOUBLE_EQ(1E5, pplib::Json::loads("1E5").toDouble());
    EXPECT_DOUBLE_EQ(1e-3, pplib::Json::loads("1e-3").toDouble());
    EXPECT_DOUBLE_EQ(2.5e+2, pplib::Json::loads("2.5e+2").toDouble());
    EXPECT_DOUBLE_EQ(-2.5e-1, pplib::Json::loads("-2.5e-1").toDouble());

    // Negative integers and zero
    EXPECT_EQ(-42, pplib::Json::loads("-42").toInt64());
    EXPECT_EQ(-123456789LL, pplib::Json::loads("-123456789").toInt64());
    EXPECT_EQ(0, pplib::Json::loads("-0").toInt64());

    // Invalid numbers
    EXPECT_THROW(pplib::Json::loads("-"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Json::loads("[-]"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Json::loads("{\"a\": -}"), pplib::UnexpectedCharacterException);
}

TEST_F(JsonTest, LiteralsAndSyntaxEdgeCases)
{
    // Incomplete literals at EOF
    EXPECT_THROW(pplib::Json::loads("tru"), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Json::loads("fal"), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Json::loads("nul"), pplib::UnexpectedEndOfDataException);

    // Invalid literal characters
    EXPECT_THROW(pplib::Json::loads("tree"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Json::loads("fool"), pplib::UnexpectedCharacterException);
    EXPECT_THROW(pplib::Json::loads("none"), pplib::UnexpectedCharacterException);

    // Empty object key maps to "_empty_"
    pplib::AssocArray assoc;
    pplib::Json::loads(assoc, "{\"\": \"empty_key_val\"}");
    EXPECT_EQ(pplib::String("empty_key_val"), assoc.getString("_empty_"));

    // Missing comma in array
    EXPECT_THROW(pplib::Json::loads("[1 2]"), pplib::UnexpectedCharacterException);

    // Missing comma in object
    EXPECT_THROW(pplib::Json::loads("{\"a\": 1 \"b\": 2}"), pplib::UnexpectedCharacterException);

    // Missing colon in object
    EXPECT_THROW(pplib::Json::loads("{\"a\" 1}"), pplib::UnexpectedCharacterException);

    // Non-string key in object
    EXPECT_THROW(pplib::Json::loads("{123: 1}"), pplib::UnexpectedCharacterException);

    // Unclosed structures
    EXPECT_THROW(pplib::Json::loads("[1, 2"), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Json::loads("{\"a\": 1"), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Json::loads("[1, "), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Json::loads("{\"a\": 1, "), pplib::UnexpectedEndOfDataException);

    // Empty structures
    EXPECT_EQ((size_t)0, pplib::Json::loads("[]").toVariantArray().size());
    EXPECT_EQ((size_t)0, pplib::Json::loads("{}").toAssocArray().size());

    // Empty input
    EXPECT_THROW(pplib::Json::loads(""), pplib::UnexpectedEndOfDataException);
    EXPECT_THROW(pplib::Json::loads("   "), pplib::UnexpectedEndOfDataException);
}

TEST_F(JsonTest, APIOverloadsAndSerializers)
{
    // Json::load(AssocArray&, FileObject&)
    pplib::String jsonDict = "{\"key\": \"val\"}";
    pplib::MemFile mfDict((void*)jsonDict.getPtr(), jsonDict.size());
    pplib::AssocArray assocLoaded;
    pplib::Json::load(assocLoaded, mfDict);
    EXPECT_EQ(pplib::String("val"), assocLoaded.getString("key"));

    // Json::load(AssocArray&, FileObject&) with wrong root type
    pplib::String jsonArray = "[1, 2, 3]";
    pplib::MemFile mfArray((void*)jsonArray.getPtr(), jsonArray.size());
    EXPECT_THROW(pplib::Json::load(assocLoaded, mfArray), pplib::TypeConversionException);

    // Json::load(VariantArray&, FileObject&)
    pplib::MemFile mfArray2((void*)jsonArray.getPtr(), jsonArray.size());
    pplib::VariantArray varrLoaded;
    pplib::Json::load(varrLoaded, mfArray2);
    ASSERT_EQ(3, varrLoaded.size());
    EXPECT_EQ(1, varrLoaded[0].toInt64());

    // Json::load(VariantArray&, FileObject&) with wrong root type
    pplib::MemFile mfDict2((void*)jsonDict.getPtr(), jsonDict.size());
    EXPECT_THROW(pplib::Json::load(varrLoaded, mfDict2), pplib::TypeConversionException);

    // Json::loads(VariantArray&, const String&) with wrong root type
    EXPECT_THROW(pplib::Json::loads(varrLoaded, jsonDict), pplib::TypeConversionException);

    // Json::dump with FileObject for Variant, AssocArray and VariantArray
    pplib::MemFile outMf;
    pplib::Json::dump(outMf, assocLoaded, 2);
    EXPECT_GT(outMf.size(), 0);

    pplib::MemFile outMf2;
    pplib::Json::dump(outMf2, varrLoaded, 2);
    EXPECT_GT(outMf2.size(), 0);

    pplib::MemFile outMf3;
    pplib::Json::dump(outMf3, pplib::Variant(varrLoaded), 2);
    EXPECT_GT(outMf3.size(), 0);

    // Json::dumps overloads for VariantArray
    pplib::String vaDumpsStr;
    pplib::Json::dumps(vaDumpsStr, varrLoaded);
    EXPECT_EQ(pplib::String("[1,2,3]"), vaDumpsStr);

    pplib::String vaDumpsReturn = pplib::Json::dumps(varrLoaded);
    EXPECT_EQ(pplib::String("[1,2,3]"), vaDumpsReturn);

    // Empty VariantArray serialization
    pplib::VariantArray emptyVa;
    EXPECT_EQ(pplib::String("[]"), pplib::Json::dumps(emptyVa));

    // Json::dumps overload returning String for AssocArray
    pplib::String assocDumpsReturn = pplib::Json::dumps(assocLoaded);
    EXPECT_EQ(pplib::String("{\"key\":\"val\"}"), assocDumpsReturn);

    // Root primitive serialization via Json::dumps(const Variant&)
    EXPECT_EQ(pplib::String("42"), pplib::Json::dumps(pplib::Variant(int64_t(42))));
    EXPECT_EQ(pplib::String("\"test\""), pplib::Json::dumps(pplib::Variant("test")));
    EXPECT_EQ(pplib::String("true"), pplib::Json::dumps(pplib::Variant(true)));
    EXPECT_EQ(pplib::String("false"), pplib::Json::dumps(pplib::Variant(false)));
    EXPECT_EQ(pplib::String("null"), pplib::Json::dumps(pplib::Variant(nullptr)));
    EXPECT_EQ(pplib::String("3.14"), pplib::Json::dumps(pplib::Variant(3.14)));

    // NaN and Infinity serialize as null
    EXPECT_EQ(pplib::String("null"), pplib::Json::dumps(pplib::Variant(std::numeric_limits<double>::quiet_NaN())));
    EXPECT_EQ(pplib::String("null"), pplib::Json::dumps(pplib::Variant(std::numeric_limits<double>::infinity())));
    EXPECT_EQ(pplib::String("null"), pplib::Json::dumps(pplib::Variant(-std::numeric_limits<double>::infinity())));

    // ByteArrayPtr serialization
    const char rawData[] = "binary_data";
    pplib::ByteArrayPtr bap((void*)rawData, sizeof(rawData) - 1);
    pplib::Variant vBap(bap);
    pplib::String expectedBap = "\"" + bap.toBase64() + "\"";
    EXPECT_EQ(expectedBap, pplib::Json::dumps(vBap));

    // Unsupported Variant type throws UnsupportedDataTypeException
    pplib::Variant unk;
    EXPECT_THROW(pplib::Json::dumps(unk), pplib::UnsupportedDataTypeException);
}

} // namespace
