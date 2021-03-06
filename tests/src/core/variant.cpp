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
extern ppl7::AssocArray TestAssocArray;

namespace {

// The fixture for testing class Foo.
class VariantTest : public ::testing::Test {
	protected:
		VariantTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~VariantTest() {

	}
};

TEST_F(VariantTest, ConstructorWithoutParam) {
	ASSERT_NO_THROW({
			ppl7::Variant var1;
			ASSERT_EQ(ppl7::Variant::TYPE_UNKNOWN,var1.type()) << "Variant has unexcpected type";
			ASSERT_THROW({
				ppl7::String s1=var1.toString();
			},ppl7::EmptyDataException);
	}
	);
}

TEST_F(VariantTest, TestWithString) {
	ppl7::String s1("Hello World");
	ASSERT_NO_THROW({
			ppl7::Variant var1(s1);
			ppl7::Variant var2(var1);
			ppl7::Variant *var3=new ppl7::Variant(var1);
			delete var3;
			ASSERT_EQ(ppl7::Variant::TYPE_STRING,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::Variant::TYPE_STRING)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::Variant::TYPE_WIDESTRING)) << "Variant has unexcpected type";

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
			ppl7::Variant var1(s1);
			ppl7::Variant var2(var1);
			ASSERT_EQ(ppl7::Variant::TYPE_WIDESTRING,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::Variant::TYPE_WIDESTRING)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::Variant::TYPE_STRING)) << "Variant has unexcpected type";

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
			ppl7::Variant var1(Wordlist);
			ppl7::Variant var2(var1);
			ASSERT_EQ(ppl7::Variant::TYPE_ARRAY,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::Variant::TYPE_ARRAY)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::Variant::TYPE_STRING)) << "Variant has unexcpected type";

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
	ASSERT_NO_THROW({
			ppl7::Variant var1(TestAssocArray);
			ppl7::Variant var2(var1);
			ASSERT_EQ(ppl7::Variant::TYPE_ASSOCARRAY,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::Variant::TYPE_ASSOCARRAY)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::Variant::TYPE_STRING)) << "Variant has unexcpected type";

			ASSERT_TRUE(var2.isAssocArray()) << "Variant has unexcpected type";
			ppl7::AssocArray a2=var2.toAssocArray();
			ASSERT_EQ(TestAssocArray.size(),a2.size()) << "Variant has unexcpected value";
			const ppl7::AssocArray &a2c=var1.toAssocArray();
			ASSERT_EQ(TestAssocArray.size(),a2c.size()) << "Variant has unexcpected value";
			ASSERT_THROW({
				ppl7::String s3=var1.toString();
			},ppl7::TypeConversionException);
			ASSERT_EQ(ppl7::String("value5"),a2.getString("array1/unterkey2")) << "unexpected value";
	});
}

TEST_F(VariantTest, TestWithByteArray) {
	ppl7::ByteArray b1=ppl7::Random(2048);
	ASSERT_NO_THROW({
			ppl7::Variant var1(b1);
			ppl7::Variant var2(var1);
			ASSERT_EQ(ppl7::Variant::TYPE_BYTEARRAY,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::Variant::TYPE_BYTEARRAY)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::Variant::TYPE_STRING)) << "Variant has unexcpected type";

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
			ppl7::Variant var1(p1);
			ppl7::Variant var2(var1);
			ASSERT_EQ(ppl7::Variant::TYPE_BYTEARRAYPTR,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::Variant::TYPE_BYTEARRAYPTR)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::Variant::TYPE_STRING)) << "Variant has unexcpected type";

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
			ppl7::Variant var1(d1);
			ppl7::Variant var2(var1);
			ASSERT_EQ(ppl7::Variant::TYPE_DATETIME,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::Variant::TYPE_DATETIME)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::Variant::TYPE_STRING)) << "Variant has unexcpected type";

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
			ppl7::Variant var1(p1);
			ppl7::Variant var2(var1);
			ASSERT_EQ(ppl7::Variant::TYPE_POINTER,var2.type()) << "Variant has unexcpected type";
			ASSERT_TRUE(var2.isType(ppl7::Variant::TYPE_POINTER)) << "Variant has unexcpected type";
			ASSERT_FALSE(var2.isType(ppl7::Variant::TYPE_STRING)) << "Variant has unexcpected type";

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
	ppl7::Variant var1;
	ppl7::Variant var2;
	var2.set(var1);
	ASSERT_EQ(ppl7::Variant::TYPE_UNKNOWN,var2.type()) << "Variant has unexcpected type";
}

TEST_F(VariantTest, OperatorVariant) {
	ppl7::String s1("Hello World");
	ppl7::Variant var1(s1);
	ppl7::Variant var2;
	var2=var1;
	ASSERT_EQ(s1,var2.toString()) << "Variant has unexcpected value";
}


TEST_F(VariantTest, OperatorString) {
	ppl7::String p1("Hello World");
	ppl7::Variant var1;
	var1=p1;
	const ppl7::String &p2=var1;
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorWideString) {
	ppl7::WideString p1(L"Hello World");
	ppl7::Variant var1;
	var1=p1;
	const ppl7::WideString &p2=var1;
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorArray) {
	ppl7::Variant var1;
	var1=Wordlist;
	const ppl7::Array &p2=var1;
	ASSERT_EQ(Wordlist.size(),p2.size()) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorAssocArray) {
	ppl7::Variant var1;
	var1=TestAssocArray;
	const ppl7::AssocArray &p2=var1;
	ASSERT_EQ(TestAssocArray.count(true),p2.count(true)) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorByteArray) {
	ppl7::ByteArray p1=ppl7::Random(2048);
	ppl7::Variant var1;
	var1=p1;
	const ppl7::ByteArray &p2=var1;
	ASSERT_EQ(p1.size(),p2.size()) << "Variant has unexcpected value";
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorByteArrayPtr) {
	ppl7::ByteArray b1=ppl7::Random(2048);
	ppl7::ByteArrayPtr p1=b1;
	ppl7::Variant var1;
	var1=p1;
	const ppl7::ByteArrayPtr &p2=var1;
	ASSERT_EQ(p1.size(),p2.size()) << "Variant has unexcpected value";
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorDateTime) {
	ppl7::DateTime p1("2015-12-03 15:52:40");
	ppl7::Variant var1;
	var1=p1;
	const ppl7::DateTime &p2=var1;
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, OperatorPointer) {
	ppl7::Pointer p1(this);
	ppl7::Variant var1;
	var1=p1;
	const ppl7::Pointer &p2=var1;
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
}

TEST_F(VariantTest, isString) {
	ppl7::String p1("Hello World");
	ppl7::WideString p1a(L"Hello World");
	ppl7::Variant var1(p1);
	ppl7::Variant var2(p1a);
	ASSERT_TRUE(var1.isString());
	ASSERT_FALSE(var2.isString());
	ASSERT_FALSE(var1.isWideString());
	ASSERT_FALSE(var1.isArray());
	ASSERT_FALSE(var1.isAssocArray());
	ASSERT_FALSE(var1.isByteArray());
	ASSERT_FALSE(var1.isByteArrayPtr());
	ASSERT_FALSE(var1.isDateTime());
	ASSERT_FALSE(var1.isPointer());
}

TEST_F(VariantTest, isWideString) {
	ppl7::WideString p1(L"Hello World");
	ppl7::String p1a("Hello World");
	ppl7::Variant var1(p1);
	ppl7::Variant var2(p1a);
	ASSERT_TRUE(var1.isWideString());
	ASSERT_FALSE(var2.isWideString());
	ASSERT_FALSE(var1.isString());
	ASSERT_FALSE(var1.isArray());
	ASSERT_FALSE(var1.isAssocArray());
	ASSERT_FALSE(var1.isByteArray());
	ASSERT_FALSE(var1.isByteArrayPtr());
	ASSERT_FALSE(var1.isDateTime());
	ASSERT_FALSE(var1.isPointer());
}

TEST_F(VariantTest, isArray) {
	ppl7::String p1a("Hello World");
	ppl7::Variant var1(Wordlist);
	ppl7::Variant var2(p1a);
	ASSERT_TRUE(var1.isArray());
	ASSERT_FALSE(var2.isArray());
	ASSERT_FALSE(var1.isString());
	ASSERT_FALSE(var1.isWideString());
	ASSERT_FALSE(var1.isAssocArray());
	ASSERT_FALSE(var1.isByteArray());
	ASSERT_FALSE(var1.isByteArrayPtr());
	ASSERT_FALSE(var1.isDateTime());
	ASSERT_FALSE(var1.isPointer());
}

TEST_F(VariantTest, isAssocArray) {
	ppl7::String p1a("Hello World");
	ppl7::Variant var1(TestAssocArray);
	ppl7::Variant var2(p1a);
	ASSERT_TRUE(var1.isAssocArray());
	ASSERT_FALSE(var2.isAssocArray());
	ASSERT_FALSE(var1.isString());
	ASSERT_FALSE(var1.isWideString());
	ASSERT_FALSE(var1.isArray());
	ASSERT_FALSE(var1.isByteArray());
	ASSERT_FALSE(var1.isByteArrayPtr());
	ASSERT_FALSE(var1.isDateTime());
	ASSERT_FALSE(var1.isPointer());
}

TEST_F(VariantTest, isByteArray) {
	ppl7::String p1a("Hello World");
	ppl7::ByteArray p1=ppl7::Random(2048);
	ppl7::Variant var1(p1);
	ppl7::Variant var2(p1a);
	ASSERT_TRUE(var1.isByteArray());
	ASSERT_FALSE(var2.isByteArray());
	ASSERT_FALSE(var1.isString());
	ASSERT_FALSE(var1.isWideString());
	ASSERT_FALSE(var1.isArray());
	ASSERT_FALSE(var1.isAssocArray());
	ASSERT_TRUE(var1.isByteArrayPtr());
	ASSERT_FALSE(var1.isDateTime());
	ASSERT_FALSE(var1.isPointer());
}

TEST_F(VariantTest, isByteArrayPtr) {
	ppl7::String p1a("Hello World");
	ppl7::ByteArray b1=ppl7::Random(2048);
	ppl7::ByteArrayPtr p1=b1;
	ppl7::Variant var1(p1);
	ppl7::Variant var2(p1a);
	ASSERT_TRUE(var1.isByteArrayPtr());
	ASSERT_FALSE(var2.isByteArrayPtr());
	ASSERT_FALSE(var1.isString());
	ASSERT_FALSE(var1.isWideString());
	ASSERT_FALSE(var1.isArray());
	ASSERT_FALSE(var1.isAssocArray());
	ASSERT_FALSE(var1.isByteArray());
	ASSERT_FALSE(var1.isDateTime());
	ASSERT_FALSE(var1.isPointer());
}

TEST_F(VariantTest, isDateTime) {
	ppl7::DateTime p1("2015-12-03 15:52:40");
	ppl7::String p1a("Hello World");
	ppl7::Variant var1(p1);
	ppl7::Variant var2(p1a);
	ASSERT_TRUE(var1.isDateTime());
	ASSERT_FALSE(var2.isDateTime());
	ASSERT_FALSE(var1.isString());
	ASSERT_FALSE(var1.isWideString());
	ASSERT_FALSE(var1.isArray());
	ASSERT_FALSE(var1.isAssocArray());
	ASSERT_FALSE(var1.isByteArray());
	ASSERT_FALSE(var1.isByteArrayPtr());
	ASSERT_FALSE(var1.isPointer());
}

TEST_F(VariantTest, isPointer) {
	ppl7::Pointer p1(this);
	ppl7::String p1a("Hello World");
	ppl7::Variant var1(p1);
	ppl7::Variant var2(p1a);
	ASSERT_TRUE(var1.isPointer());
	ASSERT_FALSE(var2.isPointer());
	ASSERT_FALSE(var1.isString());
	ASSERT_FALSE(var1.isWideString());
	ASSERT_FALSE(var1.isArray());
	ASSERT_FALSE(var1.isAssocArray());
	ASSERT_FALSE(var1.isByteArray());
	ASSERT_FALSE(var1.isByteArrayPtr());
	ASSERT_FALSE(var1.isDateTime());
}


TEST_F(VariantTest, toString) {
	ppl7::String p1("Hello World");
	ppl7::WideString p1a(L"Hello World");
	ppl7::Variant var1(p1);
	ppl7::Variant var2;
	ppl7::Variant var3(p1a);
	const ppl7::String &p2=var1.toString();
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
	ASSERT_THROW({
		ppl7::String s3=var2.toString();
	},ppl7::EmptyDataException);

	ASSERT_THROW({
		ppl7::String s3=var3.toString();
	},ppl7::TypeConversionException);
}


TEST_F(VariantTest, toStringConst) {
	ppl7::String p1("Hello World");
	ppl7::WideString p1a(L"Hello World");
	const ppl7::Variant var1(p1);
	const ppl7::Variant var2;
	const ppl7::Variant var3(p1a);
	const ppl7::String &p2=var1.toString();
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
	ASSERT_THROW({
		const ppl7::String &s3=var2.toString();
		ASSERT_TRUE(s3.isEmpty());
	},ppl7::EmptyDataException);

	ASSERT_THROW({
		const ppl7::String &s3=var3.toString();
		ASSERT_TRUE(s3.isEmpty());
	},ppl7::TypeConversionException);
}



TEST_F(VariantTest, toWideString) {
	ppl7::WideString p1(L"Hello World");
	ppl7::String p1a("Hello World");
	ppl7::Variant var1(p1);
	ppl7::Variant var2;
	ppl7::Variant var3(p1a);
	const ppl7::WideString &p2=var1.toWideString();
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
	ASSERT_THROW({
		ppl7::WideString s3=var2.toWideString();
	},ppl7::EmptyDataException);

	ASSERT_THROW({
		ppl7::WideString s3=var3.toWideString();
	},ppl7::TypeConversionException);
}

TEST_F(VariantTest, toWideStringConst) {
	ppl7::WideString p1(L"Hello World");
	ppl7::String p1a("Hello World");
	const ppl7::Variant var1(p1);
	const ppl7::Variant var2;
	const ppl7::Variant var3(p1a);
	const ppl7::WideString &p2=var1.toWideString();
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
	ASSERT_THROW({
		const ppl7::WideString &s3=var2.toWideString();
		ASSERT_TRUE(s3.isEmpty());
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		const ppl7::WideString &s3=var3.toWideString();
		ASSERT_TRUE(s3.isEmpty());
	},ppl7::TypeConversionException);
}


TEST_F(VariantTest, toArray) {
	ppl7::String p1a("Hello World");
	ppl7::Variant var1(Wordlist);
	ppl7::Variant var2;
	ppl7::Variant var3(p1a);
	const ppl7::Array &p2=var1.toArray();
	ASSERT_EQ(Wordlist.size(),p2.size()) << "Variant has unexcpected value";
	ASSERT_THROW({
		ppl7::Array s3=var2.toArray();
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		ppl7::Array s3=var3.toArray();
	},ppl7::TypeConversionException);
}

TEST_F(VariantTest, toArrayConst) {
	ppl7::String p1a("Hello World");
	const ppl7::Variant var1(Wordlist);
	const ppl7::Variant var2;
	const ppl7::Variant var3(p1a);
	const ppl7::Array &p2=var1.toArray();
	ASSERT_EQ(Wordlist.size(),p2.size()) << "Variant has unexcpected value";
	ASSERT_THROW({
		const ppl7::Array &s3=var2.toArray();
		ASSERT_TRUE(s3.size()>0);
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		const ppl7::Array &s3=var3.toArray();
		ASSERT_TRUE(s3.size()>0);
	},ppl7::TypeConversionException);
}

TEST_F(VariantTest, toAssocArray) {
	ppl7::String p1a("Hello World");
	ppl7::Variant var1(TestAssocArray);
	ppl7::Variant var2;
	ppl7::Variant var3(p1a);
	const ppl7::AssocArray &p2=var1.toAssocArray();
	ASSERT_EQ(TestAssocArray.size(),p2.size()) << "Variant has unexcpected value";
	ASSERT_THROW({
		ppl7::AssocArray s3=var2.toAssocArray();
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		ppl7::AssocArray s3=var3.toAssocArray();
	},ppl7::TypeConversionException);
}

TEST_F(VariantTest, toAssocArrayConst) {
	ppl7::String p1a("Hello World");
	const ppl7::Variant var1(TestAssocArray);
	const ppl7::Variant var2;
	const ppl7::Variant var3(p1a);
	const ppl7::AssocArray &p2=var1.toAssocArray();
	ASSERT_EQ(TestAssocArray.size(),p2.size()) << "Variant has unexcpected value";
	ASSERT_THROW({
		const ppl7::AssocArray &s3=var2.toAssocArray();
		ASSERT_TRUE(s3.size()>0);
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		const ppl7::AssocArray &s3=var3.toAssocArray();
		ASSERT_TRUE(s3.size()>0);
	},ppl7::TypeConversionException);
}

TEST_F(VariantTest, toByteArray) {
	ppl7::String p1a("Hello World");
	ppl7::ByteArray p1=ppl7::Random(2048);
	ppl7::Variant var1(p1);
	ppl7::Variant var2;
	ppl7::Variant var3(p1a);
	ppl7::ByteArray &p2=var1.toByteArray();
	ppl7::ByteArrayPtr &p4=var1.toByteArrayPtr();
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
	ASSERT_EQ(p1,p4) << "Variant has unexcpected value";
	ASSERT_THROW({
		ppl7::ByteArray s3=var2.toByteArray();
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		ppl7::ByteArrayPtr s3=var2.toByteArrayPtr();
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		ppl7::ByteArray s3=var3.toByteArray();
	},ppl7::TypeConversionException);
	ASSERT_THROW({
		ppl7::ByteArrayPtr s3=var3.toByteArrayPtr();
	},ppl7::TypeConversionException);
}

TEST_F(VariantTest, toByteArrayConst) {
	ppl7::String p1a("Hello World");
	ppl7::ByteArray p1=ppl7::Random(2048);
	const ppl7::Variant var1(p1);
	const ppl7::Variant var2;
	const ppl7::Variant var3(p1a);
	const ppl7::ByteArray &p2=var1.toByteArray();
	const ppl7::ByteArrayPtr &p4=var1.toByteArrayPtr();
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
	ASSERT_EQ(p1,p4) << "Variant has unexcpected value";
	ASSERT_THROW({
		const ppl7::ByteArray &s3=var2.toByteArray();
		ASSERT_TRUE(s3.isEmpty());
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		const ppl7::ByteArrayPtr &s3=var2.toByteArrayPtr();
		ASSERT_TRUE(s3.isEmpty());
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		const ppl7::ByteArray &s3=var3.toByteArray();
		ASSERT_TRUE(s3.isEmpty());
	},ppl7::TypeConversionException);
	ASSERT_THROW({
		const ppl7::ByteArrayPtr &s3=var3.toByteArrayPtr();
		ASSERT_TRUE(s3.isEmpty());
	},ppl7::TypeConversionException);
}

TEST_F(VariantTest, toByteArrayPtr) {
	ppl7::String p1a("Hello World");
	ppl7::ByteArray b1=ppl7::Random(2048);
	ppl7::ByteArrayPtr p1=b1;
	ppl7::Variant var1(p1);
	ppl7::Variant var2;
	ppl7::Variant var3(p1a);
	const ppl7::ByteArrayPtr &p2=var1.toByteArrayPtr();
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
	ASSERT_THROW({
		ppl7::ByteArrayPtr s3=var2.toByteArrayPtr();
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		ppl7::ByteArrayPtr s3=var3.toByteArrayPtr();
	},ppl7::TypeConversionException);
}

TEST_F(VariantTest, toByteArrayPtrConst) {
	ppl7::String p1a("Hello World");
	ppl7::ByteArray b1=ppl7::Random(2048);
	ppl7::ByteArrayPtr p1=b1;
	const ppl7::Variant var1(p1);
	const ppl7::Variant var2;
	const ppl7::Variant var3(p1a);
	const ppl7::ByteArrayPtr &p2=var1.toByteArrayPtr();
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
	ASSERT_THROW({
		const ppl7::ByteArrayPtr &s3=var2.toByteArrayPtr();
		ASSERT_TRUE(s3.isEmpty());
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		const ppl7::ByteArrayPtr &s3=var3.toByteArrayPtr();
		ASSERT_TRUE(s3.isEmpty());
	},ppl7::TypeConversionException);
}

TEST_F(VariantTest, toDateTime) {
	ppl7::String p1a("Hello World");
	ppl7::DateTime p1("2015-12-03 15:52:40");
	ppl7::Variant var1(p1);
	ppl7::Variant var2;
	ppl7::Variant var3(p1a);
	const ppl7::DateTime &p2=var1.toDateTime();
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
	ASSERT_THROW({
		ppl7::DateTime s3=var2.toDateTime();
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		ppl7::DateTime s3=var3.toDateTime();
	},ppl7::TypeConversionException);
}

TEST_F(VariantTest, toDateTimeConst) {
	ppl7::String p1a("Hello World");
	ppl7::DateTime p1("2015-12-03 15:52:40");
	const ppl7::Variant var1(p1);
	const ppl7::Variant var2;
	const ppl7::Variant var3(p1a);
	const ppl7::DateTime &p2=var1.toDateTime();
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
	ASSERT_THROW({
		const ppl7::DateTime &s3=var2.toDateTime();
		ASSERT_TRUE(s3.isEmpty());
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		const ppl7::DateTime &s3=var3.toDateTime();
		ASSERT_TRUE(s3.isEmpty());
	},ppl7::TypeConversionException);
}


TEST_F(VariantTest, toPointer) {
	ppl7::String p1a("Hello World");
	ppl7::Pointer p1(this);
	ppl7::Variant var1(p1);
	ppl7::Variant var2;
	ppl7::Variant var3(p1a);
	const ppl7::Pointer &p2=var1.toPointer();
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
	ASSERT_THROW({
		ppl7::Pointer s3=var2.toPointer();
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		ppl7::Pointer s3=var3.toPointer();
	},ppl7::TypeConversionException);
}

TEST_F(VariantTest, toPointerConst) {
	ppl7::String p1a("Hello World");
	ppl7::Pointer p1(this);
	const ppl7::Variant var1(p1);
	const ppl7::Variant var2;
	const ppl7::Variant var3(p1a);
	const ppl7::Pointer &p2=var1.toPointer();
	ASSERT_EQ(p1,p2) << "Variant has unexcpected value";
	ASSERT_THROW({
		const ppl7::Pointer &s3=var2.toPointer();
		ASSERT_TRUE(s3.ptr()!=NULL);
	},ppl7::EmptyDataException);
	ASSERT_THROW({
		const ppl7::Pointer &s3=var3.toPointer();
		ASSERT_TRUE(s3.ptr()!=NULL);
	},ppl7::TypeConversionException);
}


} // EOF namespace
