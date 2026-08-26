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
#include <gtest/gtest.h>

#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/datetime.h>
#include <pplib/types/array.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>
#include <pplib/core/file.h>

#include "pplib-tests.h"

extern pplib::Array Wordlist;

namespace
{

TEST(AssocArrayTest, ConstructorSimple)
{
    ASSERT_NO_THROW({
        pplib::AssocArray a;
        // ASSERT_EQ(pplib::String("success"),res) << "Unexpected return value";
    });
}

TEST(AssocArrayTest, addStringsLevel1)
{
    pplib::AssocArray a;
    ASSERT_NO_THROW({
        a.set("key1", "Dieser Wert geht über\nmehrere Zeilen");
        a.set("key2", "value6");
    });
    ASSERT_EQ((size_t)2, a.count()) << "Unexpected size of AssocArray";
    // a.list();
}

TEST(AssocArrayTest, addStringsMultiLevels)
{
    pplib::AssocArray a;
    ASSERT_NO_THROW({
        a.set("key1", "Dieser Wert geht über\nmehrere Zeilen");
        a.set("key2", "value6");
        a.set("array1/unterkey1", "value2");
        a.set("array1/unterkey2", "value3");
        a.set("array1/noch ein array/unterkey1", "value4");
        a.set("array1/unterkey2", "value5");
        a.set("key2", "value7");
        a.set("array2/unterkey1", "value7");
        a.set("array2/unterkey2", "value8");
        a.set("array2/unterkey1", "value9");
    });
    ASSERT_EQ((size_t)4, a.count()) << "Unexpected size of AssocArray";
    ASSERT_EQ((size_t)10, a.count(true)) << "Unexpected size of AssocArray";
    ASSERT_EQ(pplib::String("Dieser Wert geht über\nmehrere Zeilen"), a.getString("key1")) << "unexpected value";
    ASSERT_EQ(pplib::String("value7"), a.getString("key2")) << "unexpected value";
    ASSERT_EQ(pplib::String("value5"), a.getString("array1/unterkey2")) << "unexpected value";
}

TEST(AssocArrayTest, addMixed)
{
    pplib::AssocArray a;
    pplib::DateTime now = pplib::DateTime::currentTime();
    pplib::ByteArray ba(1234);
    pplib::ByteArrayPtr bap = ba;
    pplib::Random(ba, 1234);

    ASSERT_NO_THROW({
        a.set("key1", "Dieser Wert geht über\nmehrere Zeilen");
        a.set("key2", "value6");
        now.setCurrentTime();
        a.set("time", now);
        a.set("bytearray", ba);
        a.set("bytearrayptr", bap);
        pplib::Array a1("red green blue yellow black white", " ");
        a.set("array1", a1);
    });
    ASSERT_EQ(now, a.get("time").toDateTime()) << "unexpected value";
    ASSERT_EQ(pplib::String("Dieser Wert geht über\nmehrere Zeilen"), a.getString("key1")) << "unexpected value";
    ASSERT_EQ(pplib::String("value6"), a.getString("key2")) << "unexpected value";
    pplib::String s = a.get("array1").toArray().implode(" ");
    ASSERT_EQ(pplib::String("red green blue yellow black white"), s) << "unexpected value";
    ASSERT_EQ(ba, a.get("bytearray").toByteArray()) << "unexpected value";
    ASSERT_EQ(bap, a.get("bytearrayptr").toByteArrayPtr()) << "unexpected value";

    // a.list();
}

TEST(AssocArrayTest, append)
{
    pplib::AssocArray a;
    ASSERT_NO_THROW({
        a.set("key", "Dieser Wert geht über\nmehrere Zeilen");
        a.append("key", "Noch eine Zeile", "\n");
    });
    ASSERT_EQ((size_t)1, a.count()) << "Unexpected size of AssocArray";
    ASSERT_EQ(pplib::String("Dieser Wert geht über\nmehrere Zeilen\nNoch eine Zeile"), a.getString("key")) << "unexpected value";
    // a.list();
}

TEST(AssocArrayTest, appendRecursive)
{
    pplib::AssocArray a;
    ASSERT_NO_THROW({
        a.set("level1/key", "First Value");
        a.append("level1/key", "Second Value", "\n");
    });
    ASSERT_EQ((size_t)1, a.count()) << "Unexpected size of AssocArray";
    ASSERT_EQ(pplib::String("First Value\nSecond Value"), a.getString("level1/key")) << "unexpected value";
    // a.list();
}

TEST(AssocArrayTest, appendNonExisting)
{
    pplib::AssocArray a;
    ASSERT_NO_THROW({
        a.append("key", "First Value", "\n");
        a.append("level1/key", "Second Value", "\n");
    });
    ASSERT_EQ((size_t)2, a.count()) << "Unexpected size of AssocArray";
    ASSERT_EQ(pplib::String("First Value"), a.getString("key")) << "unexpected value";
    ASSERT_EQ(pplib::String("Second Value"), a.getString("level1/key")) << "unexpected value";
    // a.list();
}

TEST(AssocArrayTest, getAssocArray)
{
    pplib::AssocArray a;
    ASSERT_NO_THROW({
        a.set("key1", "Dieser Wert geht über\nmehrere Zeilen");
        a.set("key2", "value6");
        a.set("array1/unterkey1", "value2");
        a.set("array1/unterkey2", "value3");
        a.set("array1/noch ein array/unterkey1", "value4");
        a.set("array1/unterkey2", "value5");
        a.set("key2", "value6");
        // a.set("dateien/main.cpp",&bin);
        a.set("array2/unterkey1", "value7");
        a.set("array2/unterkey2", "value8");
        a.set("array2/unterkey1", "value9");
    });
    // a.list();
    ASSERT_NO_THROW({
        const pplib::AssocArray& a2 = a.get("array1").toAssocArray();
        ASSERT_EQ((size_t)3, a2.count()) << "Unexpected size of AssocArray";
        pplib::AssocArray& a3 = a.get("array2").toAssocArray();
        ASSERT_EQ((size_t)2, a3.count()) << "Unexpected size of AssocArray";
    });
}

TEST(AssocArrayTest, addAndDeleteWordlist)
{
    pplib::AssocArray a;
    size_t total = Wordlist.count();
    pplib::PrintDebugTime("Loading wordlist\n");
    pplib::String empty;
    for (size_t i = 0; i < total; i++) {
        a.set(Wordlist[i], empty);
    }
    pplib::PrintDebugTime("done\n");
    // ASSERT_EQ(total,a.count()) << "Tree has unexpected size";

    pplib::PrintDebugTime("Deleting wordlist\n");
    for (size_t i = 0; i < total; i++) {
        try {
            a.erase(Wordlist[i]);
        }
        catch (pplib::KeyNotFoundException&) {
        }
    }
    pplib::PrintDebugTime("done\n");
    ASSERT_EQ((size_t)0, a.count()) << "Tree has unexpected size";
}

static void createDefaultAssocArray(pplib::AssocArray& a)
{
    pplib::AssocArray data;
    a.set("key1", "Dieser Wert geht ueber\nmehrere Zeilen");
    a.set("key2", "value6");
    a.set("array1/unterkey1", "value2");
    a.set("array1/unterkey2", "value3");
    a.set("array1/noch ein array/unterkey1", "value4");
    a.set("array1/unterkey2", "value5");
    a.set("key2", "value7");
    a.set("array2/unterkey1", "value7");
    a.set("array2/unterkey2", "value8");
    a.set("array2/unterkey1", "value9");
    a.set("stringarray", pplib::Array("red green blue white", " "));

    a.set("widestring", pplib::WideString(L"this is a widestring - äöü"));
    a.set("datetime", pplib::DateTime("2018-12-03 13:49:10.123456"));
    a.set("datetimetz", pplib::DateTime("2018-12-03 13:49:10.123456+02:00"));
    a.set("time", pplib::Time("12:45:10.123456"));
    a.set("date", pplib::Date("2018-12-03"));
    a.set("timedelta", pplib::TimeDelta("12:45:10.123456"));
    a.set("timezone", pplib::TimeZone(2, 0, "Europe/Berlin"));
    a.set("bytearray", pplib::ByteArray("this is a bytearray"));

    data.setf("sampleTime", "%0.6f", 22362546.32543);
    data.setf("net_receive/bytes", "%lu", (unsigned long)1);
    data.setf("net_receive/packets", "%lu", (unsigned long)2);
    data.setf("net_receive/errs", "%lu", (unsigned long)3);
    data.setf("net_receive/drop", "%lu", (unsigned long)4);
    data.setf("net_transmit/bytes", "%lu", (unsigned long)5);
    data.setf("net_transmit/packets", "%lu", (unsigned long)6);
    data.setf("net_transmit/errs", "%lu", (unsigned long)7);
    data.setf("net_transmit/drop", "%lu", (unsigned long)8);

    data.setf("cpu/user", "%d", 1);
    data.setf("cpu/nice", "%d", 2);
    data.setf("cpu/system", "%d", 3);
    data.setf("cpu/idle", "%d", 4);
    data.setf("cpu/iowait", "%d", 5);

    data.setf("sysinfo/uptime", "%lld", (long long)32324234213);
    data.setf("sysinfo/freeswap", "%lld", (long long)2345215545);
    data.setf("sysinfo/totalswap", "%lld", (long long)65463635);
    data.setf("sysinfo/freeram", "%lld", (long long)5467254523);
    data.setf("sysinfo/bufferram", "%lld", (long long)549153452345);
    data.setf("sysinfo/totalram", "%lld", (long long)24346579);
    data.setf("sysinfo/sharedram", "%lld", (long long)232356657);
    data.setf("sysinfo/procs", "%d", 12321);
    a.set("data/[]", data);
    a.set("data/[]", data);
    // a.list();
}

TEST(AssocArrayTest, binarySize)
{
    pplib::AssocArray a;
    ASSERT_NO_THROW({ createDefaultAssocArray(a); });
    ASSERT_EQ((size_t)1640, a.binarySize());
}

TEST(AssocArrayTest, exportAndImportBinary)
{
    pplib::AssocArray a;
    pplib::AssocArray b;
    ASSERT_NO_THROW({ createDefaultAssocArray(a); });
    pplib::ByteArray ba;
    EXPECT_NO_THROW({
        try {
            ba = a.exportBinary();
        }
        catch (const pplib::Exception& exp) {
            exp.print();
            throw;
        }
    });
    // pplib::HexDump(buffer,realsize);
    ASSERT_EQ((size_t)1640, ba.size());
    // ba.hexDump();
    EXPECT_NO_THROW({ b.importBinary(ba); });

    ASSERT_EQ(a.count(), b.count()) << "Unexpected size of AssocArray";
    ASSERT_EQ(a.count(true), b.count(true)) << "Unexpected size of AssocArray";
    ASSERT_EQ(pplib::String("Dieser Wert geht ueber\nmehrere Zeilen"), b.getString("key1")) << "unexpected value";
    ASSERT_EQ(pplib::String("value7"), b.getString("key2")) << "unexpected value";
    ASSERT_EQ(pplib::String("value5"), b.getString("array1/unterkey2")) << "unexpected value";
    ASSERT_EQ(pplib::String("32324234213"), b.getString("data/1/sysinfo/uptime")) << "unexpected value";
    ASSERT_EQ(pplib::String("3"), b.getString("data/0/cpu/system")) << "unexpected value";
    ASSERT_EQ(pplib::WideString(L"this is a widestring - äöü"), b.get("widestring").toWideString()) << "unexpected value";
    ASSERT_EQ(pplib::Time("12:45:10.123456"), b.get("time").toTime()) << "unexpected value";
    ASSERT_EQ(pplib::Date("2018-12-03"), b.get("date").toDate()) << "unexpected value";
    ASSERT_EQ(pplib::TimeDelta("12:45:10.123456"), b.get("timedelta").toTimeDelta()) << "unexpected value";
    ASSERT_EQ(pplib::TimeZone(2, 0, "Europe/Berlin"), b.get("timezone").toTimeZone()) << "unexpected value";
    ASSERT_EQ(pplib::DateTime("2018-12-03 13:49:10.123456"), b.get("datetime").toDateTime()) << "unexpected value";
    ASSERT_EQ(pplib::DateTime("2018-12-03 13:49:10.123456+02:00"), b.get("datetimetz").toDateTime()) << "unexpected value";
    ASSERT_EQ(pplib::Array("red green blue white", " "), b.get("stringarray").toArray()) << "unexpected value";
}

TEST(AssocArrayTest, CountNonRecursive)
{
    pplib::AssocArray a1;
    ASSERT_EQ((size_t)0, a1.count(false));
    a1.set("key1", "value1");
    ASSERT_EQ((size_t)1, a1.count(false));
    a1.set("array1/key1", "value2");
    ASSERT_EQ((size_t)2, a1.count(false));
    a1.set("array1/key2", "value3");
    ASSERT_EQ((size_t)2, a1.count(false));
    a1.set("array2/key1", "value3a");
    ASSERT_EQ((size_t)3, a1.count(false));
    a1.set("key2", "value4");
    ASSERT_EQ((size_t)4, a1.count(false));
    a1.set("key3", "value5");
    ASSERT_EQ((size_t)5, a1.count(false));

    ASSERT_EQ((size_t)5, a1.size());
}

TEST(AssocArrayTest, CountRecursive)
{
    pplib::AssocArray a1;
    ASSERT_EQ((size_t)0, a1.count(true));
    a1.set("key1", "value1");
    ASSERT_EQ((size_t)1, a1.count(true));
    a1.set("array1/key1", "value2");
    ASSERT_EQ((size_t)3, a1.count(true));
    a1.set("array1/key2", "value3");
    ASSERT_EQ((size_t)4, a1.count(true));
    a1.set("array2/key1", "value3a");
    ASSERT_EQ((size_t)6, a1.count(true));
    a1.set("key2", "value4");
    ASSERT_EQ((size_t)7, a1.count(true));
    a1.set("key3", "value5");
    ASSERT_EQ((size_t)8, a1.count(true));

    ASSERT_EQ((size_t)5, a1.size());
}

TEST(AssocArrayTest, getVariant)
{
    pplib::AssocArray a1;
    a1.set("key1", "value1");
    a1.set("array1/key1", "value2");
    pplib::Array a2("red green blue yellow black white", " ");
    a1.set("array1/array2", a2);

    ASSERT_THROW(a1.get(""), pplib::AssocArray::InvalidKeyException);
    ASSERT_EQ(pplib::String("value1"), a1.get("key1").toString());
    ASSERT_EQ(pplib::String("value2"), a1.get("array1/key1").toString());
    ASSERT_THROW(a1.get("array1/array2/blah"), pplib::KeyNotFoundException);
    ASSERT_THROW(a1.get("gibtsnicht"), pplib::KeyNotFoundException);
}

TEST(AssocArrayTest, exists)
{
    pplib::AssocArray a1;
    a1.set("key1", "value1");
    a1.set("array1/key1", "value2");
    pplib::Array a2("red green blue yellow black white", " ");
    a1.set("array1/array2", a2);

    ASSERT_TRUE(a1.exists("key1"));
    ASSERT_TRUE(a1.exists("array1/key1"));
    ASSERT_TRUE(a1.exists("array1/array2"));
    ASSERT_FALSE(a1.exists("array1/array2/blah"));
    ASSERT_FALSE(a1.exists("gibtsnicht"));
    ASSERT_THROW(a1.exists(""), pplib::AssocArray::InvalidKeyException);
}

TEST(AssocArrayTest, OperatorPlus)
{
    pplib::AssocArray a1, a2;
    a1.set("key1", "value1");
    a1.set("array1/key1", "value2");
    a1.set("array1/key2", "value3");
    a1.set("array2/key1", "value3a");
    a1.set("key2", "value4");
    a1.set("key3", "value5");
    ASSERT_EQ((size_t)5, a1.count(false));
    ASSERT_EQ((size_t)8, a1.count(true));

    a2.set("key3", "value6");
    a2.set("array1/key3", "value7");
    a2.set("array2/key1", "value8");
    a2.set("array2/key2", "value8a");
    a2.set("array2/key3", "value8b");
    a2.set("array3/key1", "value9");
    a2.set("key4", "value10");
    ASSERT_EQ((size_t)5, a2.count(false));
    ASSERT_EQ((size_t)10, a2.count(true));

    pplib::AssocArray ret = a1 + a2;
    // ret.list();

    ASSERT_EQ((size_t)7, ret.size());
    ASSERT_EQ((size_t)14, ret.count(true));

    ASSERT_EQ(pplib::String("value1"), ret.getString("key1"));
    ASSERT_EQ(pplib::String("value4"), ret.getString("key2"));
    ASSERT_EQ(pplib::String("value6"), ret.getString("key3"));
    ASSERT_EQ(pplib::String("value10"), ret.getString("key4"));

    // ASSERT_THROW({ ret.getString("array1/key1"); }, pplib::KeyNotFoundException);
    ASSERT_EQ(pplib::String("value7"), ret.getString("array1/key3"));
    ASSERT_EQ(pplib::String("value8"), ret.getString("array2/key1"));
    ASSERT_EQ(pplib::String("value9"), ret.getString("array3/key1"));
}

TEST(AssocArrayTest, OperatorPlusEqual)
{
    pplib::AssocArray a1, a2;
    a1.set("key1", "value1");
    a1.set("array1/key1", "value2");
    a1.set("array1/key2", "value3");
    a1.set("array2/key1", "value3a");
    a1.set("key2", "value4");
    a1.set("key3", "value5");

    a2.set("key3", "value6");
    a2.set("array1/key3", "value7");
    a2.set("array2/key1", "value8");
    a2.set("array2/key2", "value8a");
    a2.set("array2/key3", "value8b");
    a2.set("array3/key1", "value9");
    a2.set("key4", "value10");

    a1 += a2;

    ASSERT_EQ((size_t)14, a1.count(true));
    ASSERT_EQ((size_t)7, a1.size());

    ASSERT_EQ(pplib::String("value1"), a1.getString("key1"));
    ASSERT_EQ(pplib::String("value4"), a1.getString("key2"));
    ASSERT_EQ(pplib::String("value6"), a1.getString("key3"));
    ASSERT_EQ(pplib::String("value10"), a1.getString("key4"));

    // ASSERT_THROW({ a1.getString("array1/key1"); }, pplib::KeyNotFoundException);
    ASSERT_EQ(pplib::String("value7"), a1.getString("array1/key3"));
    ASSERT_EQ(pplib::String("value8"), a1.getString("array2/key1"));
    ASSERT_EQ(pplib::String("value9"), a1.getString("array3/key1"));
}

TEST(AssocArrayTest, OperatorEqualEqual)
{
    pplib::AssocArray a1, a2;
    a1.set("key1", "value1");
    a1.set("array1/key1", "value2");
    a1.set("array1/key2", "value3");
    a1.set("array2/key1", "value3a");
    a1.set("key2", "value4");
    a1.set("key3", "value5");

    a2.set("key1", "value1");
    a2.set("array1/key1", "value2");
    a2.set("array1/key2", "value3");
    a2.set("array2/key1", "value3a");
    a2.set("key2", "value4");
    a2.set("key3", "value5");

    ASSERT_TRUE(a1 == a2);
    ASSERT_FALSE(a1 != a2);
}

TEST(AssocArrayTest, OperatorNotEqual)
{
    pplib::AssocArray a1, a2;
    a1.set("key1", "value1");
    a1.set("array1/key1", "value2");
    a1.set("array1/key2", "value3");
    a1.set("array2/key1", "value3a");
    a1.set("key2", "value4");
    a1.set("key3", "value5");

    a2.set("key1", "value1");
    a2.set("array1/key1", "value7");
    a2.set("array1/key2", "value3");
    a2.set("array2/key1", "value3a");
    a2.set("key2", "value4");
    a2.set("key3", "value5");

    ASSERT_TRUE(a1 != a2);
    ASSERT_FALSE(a1 == a2);
}

TEST(AssocArrayTest, OperatorElement)
{
    pplib::AssocArray a1, a2;
    a1.set("key1", "value1");
    a1.set("array1/key1", "value2");
    a1.set("array1/key2", "value3");
    a1.set("array2/key1", "value3a");
    a1.set("key2", "value4");
    a1.set("key3", "value5");

    ASSERT_EQ(pplib::Variant(pplib::String("value2")), a1["array1/key1"]);
    ASSERT_EQ(pplib::Variant(pplib::String("value4")), a1["key2"]);
}

#ifdef OLDCODE

static void createWalkingArray(pplib::AssocArray& a)
{
    pplib::DateTime now = pplib::DateTime::currentTime();
    pplib::ByteArray ba(1234);
    pplib::ByteArrayPtr bap = ba;
    pplib::Random(ba, 1234);
    a.set("time", now);
    a.set("aaaa", "first element");
    a.set("blah", "blubb");
    a.set("bytearray", ba);
    a.set("bytearrayptr", bap);
    pplib::Array a1("red green blue yellow black white", " ");
    a.set("array0", a1);
    a.set("key1", "Dieser Wert geht über\nmehrere Zeilen");
    a.set("key2", "value6");
    a.set("array1/unterkey1", "value2");
    a.set("array1/unterkey2", "value3");
    a.set("array1/noch ein array/unterkey1", "value4");
    a.set("array1/unterkey2", "value5");
    a.set("key3", "value7");
    a.set("array2/unterkey1", "value7");
    a.set("array2/unterkey2", "value8");
    a.set("array2/unterkey1", "value9");
}

TEST(AssocArrayTest, ToTemplate)
{
    pplib::AssocArray a;
    pplib::ByteArray bin;
    pplib::String out;
    pplib::File::load(bin, "src/main.cpp");
    a.set("key1", "Dieser Wert geht über\nmehrere Zeilen");
    a.set("array1/unterkey1", "value2");
    a.set("array1/unterkey2", "value3");
    a.set("array1/noch ein array/unterkey1", "value4");
    a.set("array1/unterkey2", "value5");
    a.set("key2", "value6");
    a.set("dateien/main.cpp", bin);
    a.set("array2/unterkey1", "value7");
    a.set("array2/unterkey2", "value8");
    a.set("array2/unterkey1", "value9");
    a.toTemplate(out, "foo");
    // out.printnl();
    ASSERT_EQ(pplib::String("foo/array1/noch ein array/unterkey1=value4\n"
                            "foo/array1/unterkey1=value2\n"
                            "foo/array1/unterkey2=value5\n"
                            "foo/array2/unterkey1=value9\n"
                            "foo/array2/unterkey2=value8\n"
                            "foo/key1=Dieser Wert geht über\n"
                            "foo/key1=mehrere Zeilen\n"
                            "foo/key2=value6\n"),
              out);
}

TEST(AssocArrayTest, fromTemplate)
{
    pplib::AssocArray a1, a2;
    pplib::String Template("key=line1\n"
                           "key = line2 \n"
                           "foo  =  bar\n"
                           "words=20\n"
                           " blah=blubb\n"
                           "hello=world");
    ASSERT_NO_THROW({
        a1.fromTemplate(Template, "\n", "=", "\n", false);
        a2.fromTemplate(Template, "\n", "=", "\n", true);
    });
    ASSERT_EQ(pplib::String("line1\n line2 "), a1.getString("key")) << "unexpected value";
    ASSERT_EQ(pplib::String("  bar"), a1.getString("foo")) << "unexpected value";
    ASSERT_EQ(pplib::String("20"), a1.getString("words")) << "unexpected value";
    ASSERT_EQ(pplib::String("blubb"), a1.getString("blah")) << "unexpected value";
    ASSERT_EQ(pplib::String("world"), a1.getString("hello")) << "unexpected value";

    ASSERT_EQ(pplib::String("line1\nline2"), a2.getString("key")) << "unexpected value";
    ASSERT_EQ(pplib::String("bar"), a2.getString("foo")) << "unexpected value";
    ASSERT_EQ(pplib::String("20"), a2.getString("words")) << "unexpected value";
    ASSERT_EQ(pplib::String("blubb"), a2.getString("blah")) << "unexpected value";
    ASSERT_EQ(pplib::String("world"), a2.getString("hello")) << "unexpected value";
}

TEST(AssocArrayTest, fromConfig)
{
    pplib::AssocArray a1, a2;
    pplib::String Template("[Abschnitt_1]\n"
                           "key =line1\n"
                           "key = line2 \n"
                           "foo  =  bar\n"
                           "words=20\n"
                           "# Kommentarzeile, die überlesen wird\n"
                           "key1=value1\n"
                           "key2=value2\n"
                           "[Abschnitt_2]\n"
                           "key1=value3\n"
                           "key2=value4\n"
                           " blah=  blubb \n"
                           "hello=world");
    ASSERT_NO_THROW({
        a1.fromConfig(Template, "\n", "=", "\n", false);
        a2.fromConfig(Template, "\n", "=", "\n", true);
    });
    // a1.list("a1");

    ASSERT_EQ(pplib::String("line1\n line2 "), a1.getString("Abschnitt_1/key")) << "unexpected value";
    ASSERT_EQ(pplib::String("  bar"), a1.getString("Abschnitt_1/foo")) << "unexpected value";
    ASSERT_EQ(pplib::String("20"), a1.getString("Abschnitt_1/words")) << "unexpected value";
    ASSERT_EQ(pplib::String("value1"), a1.getString("Abschnitt_1/key1")) << "unexpected value";
    ASSERT_EQ(pplib::String("value2"), a1.getString("Abschnitt_1/key2")) << "unexpected value";
    ASSERT_EQ(pplib::String("  blubb "), a1.getString("Abschnitt_2/blah")) << "unexpected value";
    ASSERT_EQ(pplib::String("world"), a1.getString("Abschnitt_2/hello")) << "unexpected value";
    ASSERT_EQ(pplib::String("value3"), a1.getString("Abschnitt_2/key1")) << "unexpected value";
    ASSERT_EQ(pplib::String("value4"), a1.getString("Abschnitt_2/key2")) << "unexpected value";

    ASSERT_EQ(pplib::String("line1\nline2"), a2.getString("Abschnitt_1/key")) << "unexpected value";
    ASSERT_EQ(pplib::String("bar"), a2.getString("Abschnitt_1/foo")) << "unexpected value";
    ASSERT_EQ(pplib::String("20"), a2.getString("Abschnitt_1/words")) << "unexpected value";
    ASSERT_EQ(pplib::String("value1"), a2.getString("Abschnitt_1/key1")) << "unexpected value";
    ASSERT_EQ(pplib::String("value2"), a2.getString("Abschnitt_1/key2")) << "unexpected value";
    ASSERT_EQ(pplib::String("blubb"), a2.getString("Abschnitt_2/blah")) << "unexpected value";
    ASSERT_EQ(pplib::String("world"), a2.getString("Abschnitt_2/hello")) << "unexpected value";
    ASSERT_EQ(pplib::String("value3"), a2.getString("Abschnitt_2/key1")) << "unexpected value";
    ASSERT_EQ(pplib::String("value4"), a2.getString("Abschnitt_2/key2")) << "unexpected value";
}
#endif

} // namespace
