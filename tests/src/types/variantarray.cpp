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
#include <climits>

#include <pplib/types/variant.h>
#include <pplib/types/variantarray.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/types/datetime.h>
#include <pplib/types/date.h>
#include <pplib/types/time.h>
#include <pplib/types/timedelta.h>
#include <pplib/types/timezone.h>
#include <pplib/types/assocarray.h>
#include <pplib/types/array.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

#include "pplib-tests.h"

namespace
{

TEST(VariantArrayTest, ConstructorEmpty)
{
    ASSERT_NO_THROW({
        pplib::VariantArray a1;
        ASSERT_TRUE(a1.size() == 0) << "Array is not empty";
    });
}

TEST(VariantArrayTest, ConstructorWithOtherVariantArray)
{
    pplib::VariantArray a1;
    a1.add(pplib::String("red"));
    a1.add(pplib::String("green"));
    a1.add(pplib::String("blue"));
    a1.add(pplib::String("yellow"));
    a1.add(pplib::String("black"));
    a1.add(pplib::String("white"));
    ASSERT_NO_THROW({
        pplib::VariantArray va1(a1);
        ASSERT_EQ(va1.size(), 6);
        ASSERT_EQ(va1.get(0), pplib::String("red"));
        ASSERT_EQ(va1.get(-1), pplib::String("white"));
    });
}

TEST(VariantArrayTest, ConstructorWithOtherVariantArrayByMove)
{
    pplib::VariantArray a1;
    a1.add(pplib::String("red"));
    a1.add(pplib::String("green"));
    a1.add(pplib::String("blue"));
    a1.add(pplib::String("yellow"));
    a1.add(pplib::String("black"));
    a1.add(pplib::String("white"));
    ASSERT_NO_THROW({
        pplib::VariantArray va1(std::move(a1));
        ASSERT_EQ(va1.size(), 6);
        ASSERT_EQ(va1.get(0), pplib::String("red"));
        ASSERT_EQ(va1.get(-1), pplib::String("white"));
        ASSERT_EQ(a1.size(), 0); // a1 should be empty after move
    });
}

TEST(VariantArrayTest, ConstructorWithArrayCopy)
{
    pplib::Array a1("red green blue yellow black white", " ");
    ASSERT_NO_THROW({
        pplib::VariantArray va1(a1);
        ASSERT_EQ(va1.size(), 6);
        ASSERT_EQ(va1.get(0), pplib::String("red"));
        ASSERT_EQ(va1.get(-1), pplib::String("white"));
        pplib::Array a2 = va1.toArray();
        ASSERT_EQ(a2.size(), 6);
        ASSERT_EQ(a2.get(0), pplib::String("red"));
        ASSERT_EQ(a2.get(-1), pplib::String("white"));
    });
}
TEST(VariantArrayTest, ConstructorWithArrayMove)
{
    pplib::Array a1("red green blue yellow black white", " ");
    ASSERT_NO_THROW({
        pplib::VariantArray va1(std::move(a1));
        ASSERT_EQ(va1.size(), 6);
        ASSERT_EQ(va1.get(0), pplib::String("red"));
        ASSERT_EQ(va1.get(-1), pplib::String("white"));
        ASSERT_EQ(a1.size(), 0); // a1 should be empty after move
    });
}

TEST(VariantArrayTest, add)
{
    pplib::VariantArray a;
    a.add(pplib::String("Hello World"));
    ASSERT_EQ(a.size(), 1);
    ASSERT_EQ(a.get(0), pplib::String("Hello World"));
}

TEST(VariantArrayTest, append)
{
    pplib::VariantArray a;
    a.append(pplib::String("Hello World"));
    ASSERT_EQ(a.size(), 1);
    ASSERT_EQ(a.get(0), pplib::String("Hello World"));
}

TEST(VariantArrayTest, extendWithCopy)
{
    pplib::VariantArray a;
    a.add(pplib::String("Hello"));
    pplib::VariantArray b;
    b.add(pplib::String("World"));
    ASSERT_NO_THROW({ a.extend(b); });
    ASSERT_EQ(a.size(), 2);
    ASSERT_EQ(a.get(0), pplib::String("Hello"));
    ASSERT_EQ(a.get(1), pplib::String("World"));
}
TEST(VariantArrayTest, extendWithMove)
{
    pplib::VariantArray a;
    a.add(pplib::String("Hello"));
    pplib::VariantArray b;
    b.add(pplib::String("World"));
    ASSERT_NO_THROW({ a.extend(std::move(b)); });
    ASSERT_EQ(a.size(), 2);
    ASSERT_EQ(a.get(0), pplib::String("Hello"));
    ASSERT_EQ(a.get(1), pplib::String("World"));
    ASSERT_EQ(b.size(), 0); // b should be empty after move
}

TEST(VariantArrayTest, extendWithSameObject)
{
    pplib::VariantArray a;
    a.add(pplib::String("Hello"));
    ASSERT_NO_THROW({ a.extend(a); });
    ASSERT_EQ(a.size(), 2);
    ASSERT_EQ(a.get(0), pplib::String("Hello"));
    ASSERT_EQ(a.get(1), pplib::String("Hello"));
}
TEST(VariantArrayTest, extendWithSameObjectByMove)
{
    pplib::VariantArray a;
    a.add(pplib::String("Hello"));
    ASSERT_NO_THROW({ a.extend(std::move(a)); });
    ASSERT_EQ(a.size(), 2);
    ASSERT_EQ(a.get(0), pplib::String("Hello"));
    ASSERT_EQ(a.get(1), pplib::String("Hello"));
}

TEST(VariantArrayTest, operatorGet_ReadAndWrite)
{
    pplib::VariantArray a;
    a.add(pplib::String("Red"));
    a.add(pplib::String("Green"));
    a.add(pplib::String("Blue"));
    a.add(pplib::String("Yellow"));
    a.add(pplib::String("Black"));
    a.add(pplib::String("White"));

    a[1] = pplib::String("Purple");

    ASSERT_EQ(a[0], pplib::String("Red"));
    ASSERT_EQ(a[1], pplib::String("Purple"));
    ASSERT_EQ(a[-1], pplib::String("White"));
    ASSERT_EQ(a[5], pplib::String("White"));

    ASSERT_THROW(a[7], pplib::OutOfBoundsException);
    ASSERT_THROW(a[-10], pplib::OutOfBoundsException);
}

TEST(VariantArrayTest, operatorGetConst)
{
    const pplib::VariantArray a = [] {
        pplib::VariantArray temp;
        temp.add(pplib::String("Red"));
        temp.add(pplib::String("Green"));
        temp.add(pplib::String("Blue"));
        temp.add(pplib::String("Yellow"));
        temp.add(pplib::String("Black"));
        temp.add(pplib::String("White"));
        return temp;
    }();

    ASSERT_EQ(a[0], pplib::String("Red"));
    ASSERT_EQ(a[1], pplib::String("Green"));
    ASSERT_EQ(a[-1], pplib::String("White"));
    ASSERT_EQ(a[5], pplib::String("White"));

    ASSERT_THROW(a[7], pplib::OutOfBoundsException);
    ASSERT_THROW(a[-10], pplib::OutOfBoundsException);
}

TEST(VariantArrayTest, set)
{
    pplib::VariantArray a1;
    a1.add(pplib::String("Red"));
    a1.add(pplib::String("Green"));
    a1.add(pplib::String("Blue"));
    a1.add(pplib::String("Yellow"));
    a1.add(pplib::String("Black"));
    a1.add(pplib::String("White"));

    a1.set(1, pplib::String("Purple"));
    a1.set(6, pplib::String("Orange"));
    a1.set(10, pplib::String("Grey"));

    ASSERT_EQ(a1.get(1), pplib::String("Purple"));
    ASSERT_EQ(a1.get(6), pplib::String("Orange"));
    ASSERT_EQ(a1.get(10), pplib::String("Grey"));
    ASSERT_EQ(a1.size(), 11);
    ASSERT_EQ(a1.get(7), pplib::Variant());
}

TEST(VariantArrayTest, setOutOfBounds)
{
    pplib::VariantArray a1;
    a1.add(pplib::String("Red"));
    a1.add(pplib::String("Green"));
    a1.add(pplib::String("Blue"));

    ASSERT_THROW(a1.set(SSIZE_MAX, pplib::String("Yellow")), pplib::OutOfBoundsException);
}

TEST(VariantArrayTest, insert)
{
    pplib::VariantArray a1;
    a1.add(pplib::String("Red"));
    a1.add(pplib::String("Green"));
    a1.add(pplib::String("Blue"));
    a1.add(pplib::String("Yellow"));
    a1.add(pplib::String("Black"));
    a1.add(pplib::String("White"));

    a1.insert(1, pplib::String("Orange"));
    ASSERT_EQ(a1.get(1), pplib::String("Orange"));
    ASSERT_EQ(a1.get(2), pplib::String("Green"));
    ASSERT_EQ(a1.get(6), pplib::String("White"));
    ASSERT_EQ(a1.size(), 7);
    a1.insert(7, pplib::String("Pink"));
    ASSERT_EQ(a1.get(7), pplib::String("Pink"));
    ASSERT_EQ(a1.size(), 8);

    a1.insert(10, pplib::String("Grey"));
    ASSERT_EQ(a1.get(10), pplib::String("Grey"));
    ASSERT_EQ(a1.size(), 11);
}
TEST(VariantArrayTest, insertOutOfBounds)
{
    pplib::VariantArray a1;
    a1.add(pplib::String("Red"));
    a1.add(pplib::String("Green"));
    a1.add(pplib::String("Blue"));

    ASSERT_THROW(a1.insert(SSIZE_MAX, pplib::String("Yellow")), pplib::OutOfBoundsException);
}

TEST(VariantArrayTest, has)
{
    pplib::VariantArray a1;
    a1.add(pplib::String("Red"));
    a1.add(pplib::String("Green"));
    a1.add(pplib::String("Blue"));

    ASSERT_TRUE(a1.has(pplib::String("Red")));
    ASSERT_TRUE(a1.has(pplib::String("Green")));
    ASSERT_TRUE(a1.has(pplib::String("Blue")));
    ASSERT_FALSE(a1.has(pplib::String("Yellow")));
}

TEST(VariantArrayTest, indexOf)
{
    pplib::VariantArray a1;
    a1.add(pplib::String("Red"));
    a1.add(pplib::String("Green"));
    a1.add(pplib::String("Blue"));

    ASSERT_EQ(a1.indexOf(pplib::String("Red")), 0);
    ASSERT_EQ(a1.indexOf(pplib::String("Green")), 1);
    ASSERT_EQ(a1.indexOf(pplib::String("Blue")), 2);
    ASSERT_EQ(a1.indexOf(pplib::String("Yellow")), -1);
}

TEST(VariantArrayTest, erase)
{
    pplib::VariantArray a1;
    a1.add(pplib::String("Red"));
    a1.add(pplib::String("Green"));
    a1.add(pplib::String("Blue"));

    ASSERT_EQ(a1.erase(1), pplib::String("Green"));
    ASSERT_EQ(a1.size(), 2);
    ASSERT_EQ(a1.get(0), pplib::String("Red"));
    ASSERT_EQ(a1.get(1), pplib::String("Blue"));

    ASSERT_THROW(a1.erase(5), pplib::OutOfBoundsException);
}

TEST(VariantArrayTest, pop)
{
    pplib::VariantArray a1;
    a1.add(pplib::String("Red"));
    a1.add(pplib::String("Green"));
    a1.add(pplib::String("Blue"));
    a1.add(pplib::String("Yellow"));
    a1.add(pplib::String("Black"));
    a1.add(pplib::String("White"));

    ASSERT_EQ(a1.pop(), pplib::String("White"));
    ASSERT_EQ(a1.size(), 5);
    ASSERT_EQ(a1.pop(), pplib::String("Black"));
    ASSERT_EQ(a1.size(), 4);
    ASSERT_EQ(a1.pop(), pplib::String("Yellow"));
    ASSERT_EQ(a1.size(), 3);
    ASSERT_EQ(a1.pop(), pplib::String("Blue"));
    ASSERT_EQ(a1.size(), 2);
    ASSERT_EQ(a1.pop(), pplib::String("Green"));
    ASSERT_EQ(a1.size(), 1);
    ASSERT_EQ(a1.pop(), pplib::String("Red"));
    ASSERT_EQ(a1.size(), 0);
    ASSERT_THROW(a1.pop(), pplib::EmptyDataException);
}

TEST(VariantArrayTest, shift)
{
    pplib::VariantArray a1;
    a1.add(pplib::String("Red"));
    a1.add(pplib::String("Green"));
    a1.add(pplib::String("Blue"));
    a1.add(pplib::String("Yellow"));
    a1.add(pplib::String("Black"));
    a1.add(pplib::String("White"));
    ASSERT_EQ(a1.shift(), pplib::String("Red"));
    ASSERT_EQ(a1.size(), 5);
    ASSERT_EQ(a1.shift(), pplib::String("Green"));
    ASSERT_EQ(a1.size(), 4);
    ASSERT_EQ(a1.shift(), pplib::String("Blue"));
    ASSERT_EQ(a1.size(), 3);
    ASSERT_EQ(a1.shift(), pplib::String("Yellow"));
    ASSERT_EQ(a1.size(), 2);
    ASSERT_EQ(a1.shift(), pplib::String("Black"));
    ASSERT_EQ(a1.size(), 1);
    ASSERT_EQ(a1.shift(), pplib::String("White"));
    ASSERT_EQ(a1.size(), 0);
    ASSERT_THROW(a1.shift(), pplib::EmptyDataException);
}

TEST(VariantArrayTest, toArrayStrict)
{
    pplib::VariantArray a1;
    a1.add(pplib::String("Red"));
    a1.add(pplib::String("Green"));
    a1.add(pplib::String("Blue"));
    a1.add(pplib::String("Yellow"));
    a1.add(pplib::String("Black"));
    a1.add(pplib::String("White"));
    pplib::Array arr = a1.toArray();
    ASSERT_EQ(arr.size(), 6);
    ASSERT_EQ(arr.get(0), pplib::String("Red"));
    ASSERT_EQ(arr.get(1), pplib::String("Green"));
    ASSERT_EQ(arr.get(2), pplib::String("Blue"));
    ASSERT_EQ(arr.get(3), pplib::String("Yellow"));
    ASSERT_EQ(arr.get(4), pplib::String("Black"));
    ASSERT_EQ(arr.get(5), pplib::String("White"));
}

TEST(VariantArrayTest, toArrayStrictWithOtherTypes)
{
    pplib::VariantArray a1;
    a1.add(pplib::String("Red"));
    a1.add(pplib::String("Green"));
    a1.add(pplib::ByteArray("Blue"));
    a1.add(pplib::WideString(L"Yellow"));
    a1.add(pplib::String("Black"));
    a1.add(pplib::String("White"));
    ASSERT_THROW(a1.toArray(), pplib::TypeConversionException);
}

TEST(VariantArrayTest, toArrayNonStrictWithOtherTypes)
{
    pplib::VariantArray a1;
    a1.add(pplib::String("Red"));
    a1.add(pplib::String("Green"));
    a1.add(pplib::ByteArray("Blue"));
    a1.add(pplib::WideString(L"Yellow")); // WideString wird zu String
    a1.add(pplib::String("Black"));
    a1.add(pplib::String("White"));
    pplib::Array arr = a1.toArray(false);
    ASSERT_EQ(arr.size(), 6);
    ASSERT_EQ(arr.get(0), pplib::String("Red"));
    ASSERT_EQ(arr.get(1), pplib::String("Green"));
    ASSERT_EQ(arr.get(2), pplib::String());
    ASSERT_EQ(arr.get(3), pplib::String("Yellow"));
    ASSERT_EQ(arr.get(4), pplib::String("Black"));
    ASSERT_EQ(arr.get(5), pplib::String("White"));
}

TEST(VariantArrayTest, operatorEqualWithArray)
{
    pplib::Array a1("red green blue yellow black white", " ");
    pplib::VariantArray va1;
    va1 = a1;
    ASSERT_EQ(va1.size(), a1.size());
    ASSERT_EQ(va1.toArray(), a1);
}

TEST(VariantArrayTest, operatorEqualWithArrayMove)
{
    pplib::Array a1("red green blue yellow black white", " ");
    pplib::VariantArray va1;
    va1 = std::move(a1);
    ASSERT_EQ(va1.size(), 6);
    ASSERT_EQ(va1.toArray().size(), 6);
    ASSERT_EQ(0, a1.size());
}

TEST(VariantArrayTest, reserveAndCapacity)
{
    pplib::VariantArray va1;
    va1.reserve(10);
    ASSERT_GE(va1.capacity(), 10);
}

TEST(VariantArrayTest, Empty)
{
    pplib::VariantArray va1;
    ASSERT_TRUE(va1.isEmpty());
    ASSERT_TRUE(va1.empty());
    va1.add(pplib::String("Red"));
    ASSERT_FALSE(va1.isEmpty());
    ASSERT_FALSE(va1.empty());
}

TEST(VariantArrayTest, operatorPlusEqual)
{
    pplib::VariantArray va1;
    va1.add(pplib::String("Red"));
    va1.add(pplib::String("Green"));
    pplib::VariantArray va2;
    va2.add(pplib::String("Blue"));
    va2.add(pplib::String("Yellow"));
    va1 += va2;
    ASSERT_EQ(va1.size(), 4);
    ASSERT_EQ(va1.get(0), pplib::String("Red"));
    ASSERT_EQ(va1.get(1), pplib::String("Green"));
    ASSERT_EQ(va1.get(2), pplib::String("Blue"));
    ASSERT_EQ(va1.get(3), pplib::String("Yellow"));
}

TEST(VariantArrayTest, operatorPlusEqualWithMove)
{
    pplib::VariantArray va1;
    va1.add(pplib::String("Red"));
    va1.add(pplib::String("Green"));
    pplib::VariantArray va2;
    va2.add(pplib::String("Blue"));
    va2.add(pplib::String("Yellow"));
    va1 += std::move(va2);
    ASSERT_EQ(va1.size(), 4);
    ASSERT_EQ(va1.get(0), pplib::String("Red"));
    ASSERT_EQ(va1.get(1), pplib::String("Green"));
    ASSERT_EQ(va1.get(2), pplib::String("Blue"));
    ASSERT_EQ(va1.get(3), pplib::String("Yellow"));
    ASSERT_EQ(va2.size(), 0);
}

TEST(VariantArrayTest, operatorEqual)
{
    pplib::VariantArray va1;
    va1.add(pplib::String("Red"));
    va1.add(pplib::String("Green"));
    pplib::VariantArray va2;
    va2.add(pplib::String("Red"));
    va2.add(pplib::String("Green"));
    ASSERT_TRUE(va1 == va2);
    va2.add(pplib::String("Blue"));
    ASSERT_FALSE(va1 == va2);
}

TEST(VariantArrayTest, operatorNotEqual)
{
    pplib::VariantArray va1;
    va1.add(pplib::String("Red"));
    va1.add(pplib::String("Green"));
    pplib::VariantArray va2;
    va2.add(pplib::String("Red"));
    va2.add(pplib::String("Green"));
    ASSERT_FALSE(va1 != va2);
    va2.add(pplib::String("Blue"));
    ASSERT_TRUE(va1 != va2);
}

TEST(VariantArrayTest, iteratorForwardNonConst)
{
    pplib::VariantArray va1;
    va1.add(pplib::String("Red"));
    va1.add(pplib::String("Green"));
    va1.add(pplib::String("Blue"));
    va1.add(pplib::String("Yellow"));

    auto it = va1.begin();
    ASSERT_EQ(*it, pplib::String("Red"));
    ++it;
    ASSERT_EQ(*it, pplib::String("Green"));
    ++it;
    ASSERT_EQ(*it, pplib::String("Blue"));
    ++it;
    ASSERT_EQ(*it, pplib::String("Yellow"));
    ++it;
    ASSERT_EQ(it, va1.end());
}

TEST(VariantArrayTest, iteratorForwardConst)
{
    const pplib::VariantArray va1 = [] {
        pplib::VariantArray va;
        va.add(pplib::String("Red"));
        va.add(pplib::String("Green"));
        va.add(pplib::String("Blue"));
        va.add(pplib::String("Yellow"));
        return va;
    }();

    auto it = va1.begin();
    ASSERT_EQ(*it, pplib::String("Red"));
    ++it;
    ASSERT_EQ(*it, pplib::String("Green"));
    ++it;
    ASSERT_EQ(*it, pplib::String("Blue"));
    ++it;
    ASSERT_EQ(*it, pplib::String("Yellow"));
    ++it;
    ASSERT_EQ(it, va1.end());
}

TEST(VariantArrayTest, iteratorReverseNonConst)
{
    pplib::VariantArray va1;
    va1.add(pplib::String("Red"));
    va1.add(pplib::String("Green"));
    va1.add(pplib::String("Blue"));
    va1.add(pplib::String("Yellow"));

    auto it = va1.rbegin();
    ASSERT_EQ(*it, pplib::String("Yellow"));
    ++it;
    ASSERT_EQ(*it, pplib::String("Blue"));
    ++it;
    ASSERT_EQ(*it, pplib::String("Green"));
    ++it;
    ASSERT_EQ(*it, pplib::String("Red"));
    ++it;
    ASSERT_EQ(it, va1.rend());
}

TEST(VariantArrayTest, iteratorReverseConst)
{
    const pplib::VariantArray va1 = [] {
        pplib::VariantArray va;
        va.add(pplib::String("Red"));
        va.add(pplib::String("Green"));
        va.add(pplib::String("Blue"));
        va.add(pplib::String("Yellow"));
        return va;
    }();

    auto it = va1.rbegin();
    ASSERT_EQ(*it, pplib::String("Yellow"));
    ++it;
    ASSERT_EQ(*it, pplib::String("Blue"));
    ++it;
    ASSERT_EQ(*it, pplib::String("Green"));
    ++it;
    ASSERT_EQ(*it, pplib::String("Red"));
    ++it;
    ASSERT_EQ(it, va1.rend());
}

TEST(VariantArrayTest, BinarySizeAndExportEmptyArray)
{
    pplib::VariantArray va;
    EXPECT_EQ(va.binarySize(), (size_t)17); // 8 magic + 1 version + 8 count

    pplib::ByteArray ba = va.exportBinary();
    EXPECT_EQ(ba.size(), (size_t)17);

    pplib::VariantArray vaImported;
    vaImported.importBinary(ba);
    EXPECT_EQ(vaImported.size(), (size_t)0);
}

TEST(VariantArrayTest, BinaryExportImportRoundtrip)
{
    pplib::VariantArray va;
    va.add(int64_t(42));
    va.add(3.14159);
    va.add(true);
    va.add(false);
    va.add(nullptr);
    va.add(pplib::String("Hello World"));
    va.add(pplib::WideString(L"Unicode Test \u00e4\u00f6\u00fc"));
    va.add(pplib::ByteArray("binary data\0with\0zeros", 22));
    const char* rawPtrData = "ptr data";
    va.add(pplib::ByteArrayPtr(rawPtrData, strlen(rawPtrData)));
    va.add(pplib::DateTime("2026-10-06 20:00:00"));
    va.add(pplib::Date("2026-10-06"));
    va.add(pplib::Time("20:00:00"));
    va.add(pplib::TimeDelta("01:23:45.678"));
    va.add(pplib::TimeZone(2, 0, "Europe/Berlin"));

    pplib::AssocArray assoc;
    assoc.set("key1", "value1");
    assoc.set("num", 100);
    va.add(assoc);

    pplib::VariantArray nested;
    nested.add(pplib::String("nested1"));
    nested.add(int64_t(999));
    va.add(nested);

    // Classic Array wrapped in Variant (TYPE_ARRAY)
    va.add(pplib::Variant(pplib::Array("item1,item2,item3", ",")));

    // Unknown Variant (TYPE_UNKNOWN)
    va.add(pplib::Variant());

    size_t expectedSize = va.binarySize();
    EXPECT_GT(expectedSize, (size_t)17);

    // Test exportBinary with ByteArray return
    pplib::ByteArray ba = va.exportBinary();
    EXPECT_EQ(ba.size(), expectedSize);

    // Test exportBinary into pre-allocated ByteArray
    pplib::ByteArray ba2;
    ba2.malloc(expectedSize);
    size_t written = va.exportBinary((void*)ba2.adr(), ba2.size());
    EXPECT_EQ(written, expectedSize);

    // Test importBinary via ByteArrayPtr
    pplib::VariantArray imported;
    imported.importBinary(ba);
    ASSERT_EQ(imported.size(), va.size());

    EXPECT_TRUE(imported[0].isInt64());
    EXPECT_EQ(imported[0].toInt64(), 42);

    EXPECT_TRUE(imported[1].isDouble());
    EXPECT_DOUBLE_EQ(imported[1].toDouble(), 3.14159);

    EXPECT_TRUE(imported[2].isBool());
    EXPECT_EQ(imported[2].toBool(), true);

    EXPECT_TRUE(imported[3].isBool());
    EXPECT_EQ(imported[3].toBool(), false);

    EXPECT_TRUE(imported[4].isNull());

    EXPECT_TRUE(imported[5].isString());
    EXPECT_EQ(imported[5].toString(), "Hello World");

    EXPECT_TRUE(imported[6].isWideString());
    EXPECT_EQ(imported[6].toWideString(), pplib::WideString(L"Unicode Test \u00e4\u00f6\u00fc"));

    EXPECT_TRUE(imported[7].isByteArray());
    EXPECT_EQ(imported[7].toByteArray().size(), (size_t)22);

    // ByteArrayPtr is imported as ByteArray
    EXPECT_TRUE(imported[8].isByteArray());
    EXPECT_EQ(imported[8].toByteArray().size(), strlen(rawPtrData));

    EXPECT_TRUE(imported[9].isDateTime());
    EXPECT_EQ(imported[9].toDateTime(), pplib::DateTime("2026-10-06 20:00:00"));

    EXPECT_TRUE(imported[10].isDate());
    EXPECT_EQ(imported[10].toDate(), pplib::Date("2026-10-06"));

    EXPECT_TRUE(imported[11].isTime());
    EXPECT_EQ(imported[11].toTime(), pplib::Time("20:00:00"));

    EXPECT_TRUE(imported[12].isTimeDelta());
    EXPECT_EQ(imported[12].toTimeDelta(), pplib::TimeDelta("01:23:45.678"));

    EXPECT_TRUE(imported[13].isTimeZone());
    EXPECT_EQ(imported[13].toTimeZone().offsetMinutes(), 120);
    EXPECT_EQ(imported[13].toTimeZone().name(), "Europe/Berlin");

    EXPECT_TRUE(imported[14].isAssocArray());
    EXPECT_EQ(imported[14].toAssocArray().getString("key1"), "value1");
    EXPECT_EQ(imported[14].toAssocArray().get("num").toInt64(), 100);

    EXPECT_TRUE(imported[15].isVariantArray());
    const pplib::VariantArray& importedNested = imported[15].toVariantArray();
    ASSERT_EQ(importedNested.size(), (size_t)2);
    EXPECT_EQ(importedNested[0].toString(), "nested1");
    EXPECT_EQ(importedNested[1].toInt64(), 999);

    // Classic Array (TYPE_ARRAY)
    EXPECT_TRUE(imported[16].isArray());
    EXPECT_EQ(imported[16].toArray().size(), (size_t)3);
    EXPECT_EQ(imported[16].toArray()[0], "item1");
    EXPECT_EQ(imported[16].toArray()[1], "item2");
    EXPECT_EQ(imported[16].toArray()[2], "item3");

    // Unknown Variant (TYPE_UNKNOWN)
    EXPECT_TRUE(imported[17].isEmpty());

    // Test importBinary with raw pointer
    pplib::VariantArray importedRaw;
    size_t consumed = importedRaw.importBinary(ba.adr(), ba.size());
    EXPECT_EQ(consumed, expectedSize);
    EXPECT_EQ(importedRaw.size(), va.size());
}

TEST(VariantArrayTest, ExportBinaryBufferTooSmallThrows)
{
    pplib::VariantArray va;
    va.add(int64_t(123));
    char smallBuffer[5];
    EXPECT_THROW(va.exportBinary(smallBuffer, sizeof(smallBuffer)), pplib::ExportBufferToSmallException);
}

TEST(VariantArrayTest, ImportBinaryInvalidArgumentsThrows)
{
    pplib::VariantArray va;
    EXPECT_THROW(va.importBinary(nullptr, 100), pplib::IllegalArgumentException);
    char dummy[10];
    EXPECT_THROW(va.importBinary(dummy, 0), pplib::IllegalArgumentException);
}

TEST(VariantArrayTest, ImportBinaryInvalidMagicThrows)
{
    pplib::VariantArray va;
    // Buffer too short (< 8 bytes)
    char shortBuf[7] = "PPL8VA";
    EXPECT_THROW(va.importBinary(shortBuf, sizeof(shortBuf)), pplib::ImportFailedException);

    // Buffer with wrong magic
    char wrongMagic[17] = "WRONGMAG\1\0\0\0\0\0\0\0";
    EXPECT_THROW(va.importBinary(wrongMagic, sizeof(wrongMagic)), pplib::ImportFailedException);
}

TEST(VariantArrayTest, ImportBinaryInvalidVersionThrows)
{
    pplib::VariantArray va;
    // Exactly 8 bytes (missing version byte)
    char noVersionBuf[8];
    memcpy(noVersionBuf, "PPL8VAAR", 8);
    EXPECT_THROW(va.importBinary(noVersionBuf, sizeof(noVersionBuf)), pplib::ImportFailedException);

    // Version != 1
    char wrongVersionBuf[17];
    memcpy(wrongVersionBuf, "PPL8VAAR", 8);
    pplib::PokeN8(wrongVersionBuf + 8, 2); // Version 2
    pplib::PokeN64(wrongVersionBuf + 9, 0);
    EXPECT_THROW(va.importBinary(wrongVersionBuf, sizeof(wrongVersionBuf)), pplib::ImportFailedException);
}

TEST(VariantArrayTest, ImportBinaryTruncatedHeaderThrows)
{
    pplib::VariantArray va;
    // Magic (8) + Version (1) = 9 bytes, but missing 8 bytes elementCount
    char truncatedBuf[12];
    memcpy(truncatedBuf, "PPL8VAAR", 8);
    pplib::PokeN8(truncatedBuf + 8, 1);
    EXPECT_THROW(va.importBinary(truncatedBuf, sizeof(truncatedBuf)), pplib::ImportFailedException);
}

TEST(VariantArrayTest, ImportBinaryInvalidElementCountThrows)
{
    pplib::VariantArray va;
    // Buffer specifies 100 elements, but only 17 bytes total (0 bytes for elements)
    char buf[17];
    memcpy(buf, "PPL8VAAR", 8);
    pplib::PokeN8(buf + 8, 1);
    pplib::PokeN64(buf + 9, 100);
    EXPECT_THROW(va.importBinary(buf, sizeof(buf)), pplib::ImportFailedException);

    // elementCount > SSIZE_MAX
    pplib::PokeN64(buf + 9, static_cast<uint64_t>(SSIZE_MAX) + 1);
    EXPECT_THROW(va.importBinary(buf, sizeof(buf)), pplib::ImportFailedException);
}

TEST(VariantArrayTest, ImportBinaryTruncatedElementThrows)
{
    pplib::VariantArray va;
    // ElementCount = 1, but element is truncated (e.g. TYPE_STRING with 4 bytes len saying 100, but buffer ends)
    char buf[23];
    memcpy(buf, "PPL8VAAR", 8);
    pplib::PokeN8(buf + 8, 1);
    pplib::PokeN64(buf + 9, 1); // 1 element
    pplib::PokeN8(buf + 17, pplib::Variant::TYPE_STRING);
    pplib::PokeN32(buf + 18, 100); // 100 bytes string length, but buffer only has 23 bytes
    EXPECT_THROW(va.importBinary(buf, sizeof(buf)), pplib::ImportFailedException);
}

} // namespace