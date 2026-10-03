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

} // namespace