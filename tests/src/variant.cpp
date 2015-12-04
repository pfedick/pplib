/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 *******************************************************************************
 * Copyright (c) 2015, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include "ppl7.h"
#include "ppl7-types.h"
#include <gtest/gtest.h>
#include "ppl7-tests.h"

extern ppl7::Array Wordlist;

namespace {

// The fixture for testing class Foo.
class VariantTest : public ::testing::Test {
	protected:
		VariantTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~VariantTest() {

	}
};

TEST_F(VariantTest, ConstructorWithoutParam) {
	ASSERT_NO_THROW({
			ppl7::NewVariant var1;
			ASSERT_EQ(ppl7::NewVariant::TYPE_UNKNOWN,var1.type()) << "Variant has unexcpected type";
			ASSERT_THROW({
				ppl7::String s1=var1.toString();
			},ppl7::EmptyDataException);
	}
	);
}

TEST_F(VariantTest, TestWithString) {
	ppl7::String s1("Hello World");
	ASSERT_NO_THROW({
			ppl7::NewVariant var1(s1);
			ppl7::NewVariant var2(var1);
			ppl7::NewVariant *var3=new ppl7::NewVariant(var1);
			delete var3;
			ASSERT_EQ(ppl7::NewVariant::TYPE_STRING,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::NewVariant::TYPE_STRING)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::NewVariant::TYPE_WIDESTRING)) << "Variant has unexcpected type";

			ASSERT_TRUE(var2.isString()) << "Variant has unexcpected type";
			ppl7::String s2=var2.toString();
			ASSERT_EQ(s1,s2) << "Variant has unexcpected value";
			const ppl7::String &cs2=var1.toString();
			ASSERT_EQ(s1,cs2) << "Variant has unexcpected value";
			ASSERT_THROW({
				ppl7::WideString s3=var1.toWideString();
			},ppl7::TypeConversionException);
	});
}

TEST_F(VariantTest, TestWithWideString) {
	ppl7::WideString s1(L"Hello World");
	ASSERT_NO_THROW({
			ppl7::NewVariant var1(s1);
			ppl7::NewVariant var2(var1);
			ASSERT_EQ(ppl7::NewVariant::TYPE_WIDESTRING,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::NewVariant::TYPE_WIDESTRING)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::NewVariant::TYPE_STRING)) << "Variant has unexcpected type";

			ASSERT_TRUE(var1.isWideString()) << "Variant has unexcpected type";
			ppl7::WideString s2=var2.toWideString();
			ASSERT_EQ(s1,s2) << "Variant has unexcpected value";
			const ppl7::WideString &cs2=var1.toWideString();
			ASSERT_EQ(s1,cs2) << "Variant has unexcpected value";
			ASSERT_THROW({
				ppl7::String s3=var1.toString();
			},ppl7::TypeConversionException);
	});
}

TEST_F(VariantTest, TestWithArray) {
	//ppl7::Array  s1(L"Hello World");
	ASSERT_NO_THROW({
			ppl7::NewVariant var1(Wordlist);
			ppl7::NewVariant var2(var1);
			ASSERT_EQ(ppl7::NewVariant::TYPE_ARRAY,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::NewVariant::TYPE_ARRAY)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::NewVariant::TYPE_STRING)) << "Variant has unexcpected type";

			ASSERT_TRUE(var2.isArray()) << "Variant has unexcpected type";
			ppl7::Array s2=var2.toArray();
			ASSERT_EQ(Wordlist.size(),s2.size()) << "Variant has unexcpected value";
			const ppl7::Array &cs2=var1.toArray();
			ASSERT_EQ(Wordlist.size(),cs2.size()) << "Variant has unexcpected value";
			ASSERT_THROW({
				ppl7::String s3=var1.toString();
			},ppl7::TypeConversionException);
	});
}

TEST_F(VariantTest, TestWithAssocArray) {
	ppl7::AssocArray a;
	a.set("key1","Dieser Wert geht über\nmehrere Zeilen");
	a.set("key2","value6");
	a.set("array1/unterkey1","value2");
	a.set("array1/unterkey2","value3");
	a.set("array1/noch ein array/unterkey1","value4");
	a.set("array1/unterkey2","value5");
	a.set("key2","value7");
	a.set("array2/unterkey1","value7");
	a.set("array2/unterkey2","value8");
	a.set("array2/unterkey1","value9");

	ASSERT_NO_THROW({
			ppl7::NewVariant var1(a);
			ppl7::NewVariant var2(var1);
			ASSERT_EQ(ppl7::NewVariant::TYPE_ASSOCARRAY,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::NewVariant::TYPE_ASSOCARRAY)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::NewVariant::TYPE_STRING)) << "Variant has unexcpected type";

			ASSERT_TRUE(var2.isAssocArray()) << "Variant has unexcpected type";
			ppl7::AssocArray a2=var2.toAssocArray();
			ASSERT_EQ(a.size(),a2.size()) << "Variant has unexcpected value";
			const ppl7::AssocArray &a2c=var1.toAssocArray();
			ASSERT_EQ(a.size(),a2c.size()) << "Variant has unexcpected value";
			ASSERT_THROW({
				ppl7::String s3=var1.toString();
			},ppl7::TypeConversionException);
			ASSERT_EQ(ppl7::String("value5"),a2.getString("array1/unterkey2")) << "unexpected value";
	});
}

TEST_F(VariantTest, TestWithByteArray) {
	ppl7::ByteArray b1=ppl7::Random(2048);
	ASSERT_NO_THROW({
			ppl7::NewVariant var1(b1);
			ppl7::NewVariant var2(var1);
			ASSERT_EQ(ppl7::NewVariant::TYPE_BYTEARRAY,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::NewVariant::TYPE_BYTEARRAY)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::NewVariant::TYPE_STRING)) << "Variant has unexcpected type";

			ASSERT_TRUE(var2.isByteArray()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isByteArrayPtr()) << "Variant has unexcpected type";
			ppl7::ByteArray b2=var2.toByteArray();
			ASSERT_EQ(b1,b2) << "Variant has unexcpected value";
			ASSERT_EQ(2048,b2.size()) << "Variant has unexcpected size";
			const ppl7::ByteArray &b2c=var2.toByteArray();
			ASSERT_EQ(b1,b2c) << "Variant has unexcpected value";
			ASSERT_EQ(2048,b2c.size()) << "Variant has unexcpected size";

			ASSERT_THROW({
				ppl7::String s3=var1.toString();
			},ppl7::TypeConversionException);
	});
}

TEST_F(VariantTest, TestWithByteArrayPtr) {
	ppl7::ByteArray b1=ppl7::Random(2048);
	ppl7::ByteArrayPtr p1=b1;
	ASSERT_NO_THROW({
			ppl7::NewVariant var1(p1);
			ppl7::NewVariant var2(var1);
			ASSERT_EQ(ppl7::NewVariant::TYPE_BYTEARRAYPTR,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::NewVariant::TYPE_BYTEARRAYPTR)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::NewVariant::TYPE_STRING)) << "Variant has unexcpected type";

			ASSERT_TRUE(var2.isByteArrayPtr()) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isByteArray()) << "Variant has unexcpected type";
			ppl7::ByteArrayPtr p2=var2.toByteArrayPtr();
			ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
			ASSERT_EQ(2048,p2.size()) << "Variant has unexcpected size";
			const ppl7::ByteArrayPtr &p2c=var2.toByteArrayPtr();
			ASSERT_EQ(p1,p2c) << "Variant has unexcpected value";
			ASSERT_EQ(2048,p2c.size()) << "Variant has unexcpected size";

			ASSERT_THROW({
				ppl7::String s3=var1.toString();
			},ppl7::TypeConversionException);
	});
}

TEST_F(VariantTest, TestWithDateTime) {
	ppl7::DateTime d1("2015-12-03 15:52:40");
	ASSERT_NO_THROW({
			ppl7::NewVariant var1(d1);
			ppl7::NewVariant var2(var1);
			ASSERT_EQ(ppl7::NewVariant::TYPE_DATETIME,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::NewVariant::TYPE_DATETIME)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::NewVariant::TYPE_STRING)) << "Variant has unexcpected type";

			ASSERT_TRUE(var2.isDateTime()) << "Variant has unexcpected type";
			ppl7::DateTime d2=var2.toDateTime();
			ASSERT_EQ(d1,d2) << "Variant has unexcpected value";
			const ppl7::DateTime &d2c=var2.toDateTime();
			ASSERT_EQ(d1,d2c) << "Variant has unexcpected value";

			ASSERT_THROW({
				ppl7::String s3=var1.toString();
			},ppl7::TypeConversionException);
	});
}

TEST_F(VariantTest, TestWithPointer) {
	ppl7::Pointer p1(this);
	ASSERT_NO_THROW({
			ppl7::NewVariant var1(p1);
			ppl7::NewVariant var2(var1);
			ASSERT_EQ(ppl7::NewVariant::TYPE_POINTER,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::NewVariant::TYPE_POINTER)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::NewVariant::TYPE_STRING)) << "Variant has unexcpected type";

			ASSERT_TRUE(var2.isPointer()) << "Variant has unexcpected type";
			ppl7::Pointer p2=var2.toPointer();
			ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
			const ppl7::Pointer &p2c=var2.toPointer();
			ASSERT_EQ(p1,p2c) << "Variant has unexcpected value";
			ASSERT_THROW({
				ppl7::String s3=var1.toString();
			},ppl7::TypeConversionException);
	});
}


TEST_F(VariantTest, TestSetWithEmptyVariant) {
	ppl7::NewVariant var1;
	ppl7::NewVariant var2;
	var2.set(var1);
	ASSERT_EQ(ppl7::NewVariant::TYPE_UNKNOWN,var2.type()) << "Variant has unexcpected type";
}


} // EOF namespace
