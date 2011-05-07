/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2010, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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
#include <ppl7.h>
#include <gtest/gtest.h>

namespace {

// The fixture for testing class Foo.
class ArrayTest : public ::testing::Test {
	protected:
	ArrayTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~ArrayTest() {

	}
};

TEST_F(ArrayTest, ConstructorSimple) {
	ASSERT_NO_THROW({
			ppl7::Array a1;
			ASSERT_TRUE(a1.count()==0) << "Array is not empty";
	}
	);
}

TEST_F(ArrayTest, ConstructorFromString) {
	ASSERT_NO_THROW({
			ppl7::Array a1(L"red green blue yellow black white",L" ");
			ASSERT_EQ((size_t)6,a1.count()) << "Array does not contain 6 elements";
	}
	);
}

TEST_F(ArrayTest, ConstructorfromOtherArray) {
	ASSERT_NO_THROW({
			ppl7::Array a1(L"red green blue yellow black white",L" ");
			ppl7::Array a2(a1);
			ASSERT_EQ((size_t)6,a2.count()) << "Array does not contain 6 elements";
	}
	);
}

TEST_F(ArrayTest, OperatorEQ) {
	ASSERT_NO_THROW({
			ppl7::Array a1(L"red green blue yellow black white",L" ");
			ppl7::Array a2(a1);
			ASSERT_TRUE(a1==a2) << "Array is not equal";
			ASSERT_FALSE(a1!=a2) << "Array is equal";
	}
	);
}

TEST_F(ArrayTest, OperatorNE) {
	ASSERT_NO_THROW({
			ppl7::Array a1(L"red green blue yellow black white",L" ");
			ppl7::Array a2(a1);
			a2.set(2,"cyan");
			ASSERT_TRUE(a1!=a2) << "Array is equal";
			ASSERT_FALSE(a1==a2) << "Array is not equal";
	}
	);
}

TEST_F(ArrayTest, OperatorCopy) {
	ASSERT_NO_THROW({
			ppl7::Array a1(L"red green blue yellow black white",L" ");
			ppl7::Array a2;
			a2=a1;
			ASSERT_TRUE(a1==a2) << "Array is not equal";
	}
	);
}

TEST_F(ArrayTest, OperatorPlusEqual) {
	ASSERT_NO_THROW({
			ppl7::Array a1(L"red green blue yellow black white",L" ");
			ppl7::Array a2(L"cyan grey magenta mint",L" ");
			a1+=a2;
			ASSERT_EQ((size_t)10,a1.count()) << "Array does not contain 10 elements";
			ppl7::String merged=a1.implode(" ");
			ASSERT_EQ(ppl7::String(L"red green blue yellow black white cyan grey magenta mint"),merged) << "Elements with wrong value";
	}
	);
}

TEST_F(ArrayTest, add1Element) {
	ASSERT_NO_THROW({
			ppl7::Array a1;
			a1.add(L"First Line");
			ASSERT_EQ((size_t)1,a1.count()) << "Array does not contain 1 elements";
	}
	);
}

TEST_F(ArrayTest, add3Elements) {
	ASSERT_NO_THROW({
			ppl7::Array a1;
			a1.add(L"First Line");
			a1.add(L"Second Line");
			a1.add(L"Third Line");
			ASSERT_EQ((size_t)3,a1.count()) << "Array does not contain 3 elements";
			ASSERT_EQ(ppl7::String(L"First Line"),a1.get(0)) << "Element 1 has wrong value";
			ASSERT_EQ(ppl7::String(L"Second Line"),a1.get(1)) << "Element 2 has wrong value";
			ASSERT_EQ(ppl7::String(L"Third Line"),a1.get(2)) << "Element 3 has wrong value";
	}
	);
}

TEST_F(ArrayTest, add100000Elements) {
	ppl7::Array a1;
	ppl7::PrintDebugTime ("add 100000 Elements\n");
	for (int i=0;i<100000;i++) {
		a1.add(L"Bart Simpson");
	}
	ppl7::PrintDebugTime ("done\n");
	ASSERT_EQ((size_t)100000,a1.count()) << "Array does not contain 100000 elements";
}


TEST_F(ArrayTest, count) {
	ASSERT_NO_THROW({
			ppl7::Array a1;
			a1.add(L"First Line");
			a1.add(L"Second Line");
			a1.add(L"Third Line");
			ASSERT_EQ((size_t)3,a1.count()) << "Array does not contain 3 elements";
	}
	);
}

TEST_F(ArrayTest, countAfterSet) {
	ASSERT_NO_THROW({
			ppl7::Array a1;
			a1.set(11,L"First Line");
			ASSERT_EQ((size_t)12,a1.count()) << "Array does not contain 12 elements";
	}
	);
}

TEST_F(ArrayTest, set) {
	ASSERT_NO_THROW({
			ppl7::Array a1;
			a1.set(9,L"Element Ten");
			ASSERT_EQ((size_t)10,a1.count()) << "Array does not contain 10 elements";
			a1.set(2,L"Element Three");
			ASSERT_EQ((size_t)10,a1.count()) << "Array does not contain 10 elements";
			ASSERT_EQ(ppl7::String(L"Element Ten"),a1.get(9)) << "Element 9 has wrong value";
			ASSERT_EQ(ppl7::String(L""),a1.get(3)) << "Element 3 has wrong value";
			ASSERT_EQ(ppl7::String(L"Element Three"),a1.get(2)) << "Element 2 has wrong value";
	}
	);
}

TEST_F(ArrayTest, get) {
	ppl7::Array a1(L"red green blue yellow black white",L" ");
	ASSERT_EQ(ppl7::String(L"red"),a1.get(0)) << "Element 0 has wrong value";
	ASSERT_EQ(ppl7::String(L"green"),a1.get(1)) << "Element 1 has wrong value";
	ASSERT_EQ(ppl7::String(L"blue"),a1.get(2)) << "Element 2 has wrong value";
	ASSERT_EQ(ppl7::String(L"yellow"),a1.get(3)) << "Element 3 has wrong value";
	ASSERT_EQ(ppl7::String(L"black"),a1.get(4)) << "Element 4 has wrong value";
	ASSERT_EQ(ppl7::String(L"white"),a1.get(5)) << "Element 5 has wrong value";
}

TEST_F(ArrayTest, getOutOfBounds) {
	ppl7::Array a1(L"red green blue yellow black white",L" ");
	ASSERT_THROW(a1.get(30),ppl7::OutOfBoundsEception);
}


TEST_F(ArrayTest, capacityTest) {
	ASSERT_NO_THROW({
			ppl7::Array a1;
			a1.reserve(20);
			ASSERT_EQ((size_t)20,a1.capacity()) << "Capacity is not 20";
			a1.set(20,L"Element 21");	// Capacity must be increased
			ASSERT_GT(a1.capacity(),(size_t)20) << "Capacity was not increased";
			ASSERT_EQ(ppl7::String(L"Element 21"),a1.get(20)) << "Element 20 has wrong value";
	}
	);
}

TEST_F(ArrayTest, empty) {
	ASSERT_NO_THROW({
			ppl7::Array a1;
			ASSERT_EQ(true,a1.empty()) << "Array should be empty";
			a1.add(L"Element 0");
			ASSERT_EQ(false,a1.empty()) << "Array should not be empty";
	}
	);
}

TEST_F(ArrayTest, insertStart) {
	ASSERT_NO_THROW({
			ppl7::Array a1(L"red green blue yellow black white",L" ");
			a1.insert(0,"grey");
			ASSERT_EQ(ppl7::String(L"grey"),a1.get(0)) << "Element 0 has wrong value";
			ASSERT_EQ(ppl7::String(L"red"),a1.get(1)) << "Element 1 has wrong value";
			ASSERT_EQ(ppl7::String(L"green"),a1.get(2)) << "Element 2 has wrong value";
			ASSERT_EQ(ppl7::String(L"blue"),a1.get(3)) << "Element 3 has wrong value";
			ASSERT_EQ(ppl7::String(L"yellow"),a1.get(4)) << "Element 4 has wrong value";
			ASSERT_EQ(ppl7::String(L"black"),a1.get(5)) << "Element 5 has wrong value";
			ASSERT_EQ(ppl7::String(L"white"),a1.get(6)) << "Element 6 has wrong value";
			ASSERT_EQ((size_t)7,a1.count()) << "Array does not contain 7 elements";
	}
	);
}

TEST_F(ArrayTest, insertMiddle) {
	ASSERT_NO_THROW({
			ppl7::Array a1(L"red green blue yellow black white",L" ");
			a1.insert(3,"grey");
			ASSERT_EQ(ppl7::String(L"red"),a1.get(0)) << "Element 0 has wrong value";
			ASSERT_EQ(ppl7::String(L"green"),a1.get(1)) << "Element 1 has wrong value";
			ASSERT_EQ(ppl7::String(L"blue"),a1.get(2)) << "Element 2 has wrong value";
			ASSERT_EQ(ppl7::String(L"grey"),a1.get(3)) << "Element 3 has wrong value";
			ASSERT_EQ(ppl7::String(L"yellow"),a1.get(4)) << "Element 4 has wrong value";
			ASSERT_EQ(ppl7::String(L"black"),a1.get(5)) << "Element 5 has wrong value";
			ASSERT_EQ(ppl7::String(L"white"),a1.get(6)) << "Element 6 has wrong value";
			ASSERT_EQ((size_t)7,a1.count()) << "Array does not contain 7 elements";
	}
	);
}

TEST_F(ArrayTest, insertEnd) {
	ASSERT_NO_THROW({
			ppl7::Array a1(L"red green blue yellow black white",L" ");
			a1.insert(6,"grey");
			ASSERT_EQ(ppl7::String(L"red"),a1.get(0)) << "Element 0 has wrong value";
			ASSERT_EQ(ppl7::String(L"green"),a1.get(1)) << "Element 1 has wrong value";
			ASSERT_EQ(ppl7::String(L"blue"),a1.get(2)) << "Element 2 has wrong value";
			ASSERT_EQ(ppl7::String(L"yellow"),a1.get(3)) << "Element 3 has wrong value";
			ASSERT_EQ(ppl7::String(L"black"),a1.get(4)) << "Element 4 has wrong value";
			ASSERT_EQ(ppl7::String(L"white"),a1.get(5)) << "Element 5 has wrong value";
			ASSERT_EQ(ppl7::String(L"grey"),a1.get(6)) << "Element 6 has wrong value";
			ASSERT_EQ((size_t)7,a1.count()) << "Array does not contain 7 elements";
	}
	);
}

TEST_F(ArrayTest, insertAfterEnd) {
	ASSERT_NO_THROW({
			ppl7::Array a1(L"red green blue yellow black white",L" ");
			a1.insert(7,"grey");
			ASSERT_EQ(ppl7::String(L"red"),a1.get(0)) << "Element 0 has wrong value";
			ASSERT_EQ(ppl7::String(L"green"),a1.get(1)) << "Element 1 has wrong value";
			ASSERT_EQ(ppl7::String(L"blue"),a1.get(2)) << "Element 2 has wrong value";
			ASSERT_EQ(ppl7::String(L"yellow"),a1.get(3)) << "Element 3 has wrong value";
			ASSERT_EQ(ppl7::String(L"black"),a1.get(4)) << "Element 4 has wrong value";
			ASSERT_EQ(ppl7::String(L"white"),a1.get(5)) << "Element 5 has wrong value";
			ASSERT_EQ(ppl7::String(L""),a1.get(6)) << "Element 6 has wrong value";
			ASSERT_EQ(ppl7::String(L"grey"),a1.get(7)) << "Element 7 has wrong value";
			ASSERT_EQ((size_t)8,a1.count()) << "Array does not contain 8 elements";
	}
	);
}

TEST_F(ArrayTest, forwardWalkIterator) {
	ppl7::Array a1(L"red green blue yellow black white",L" ");
	ppl7::Array::Iterator it;
	a1.reset(it);
	ASSERT_EQ(ppl7::String(L"red"),a1.getNext(it)) << "Element 0 has wrong value";
	ASSERT_EQ(ppl7::String(L"green"),a1.getNext(it)) << "Element 1 has wrong value";
	ASSERT_EQ(ppl7::String(L"blue"),a1.getNext(it)) << "Element 2 has wrong value";
	ASSERT_EQ(ppl7::String(L"yellow"),a1.getNext(it)) << "Element 3 has wrong value";
	ASSERT_EQ(ppl7::String(L"black"),a1.getNext(it)) << "Element 4 has wrong value";
	ASSERT_EQ(ppl7::String(L"white"),a1.getNext(it)) << "Element 5 has wrong value";
	ASSERT_THROW(a1.getNext(it),ppl7::OutOfBoundsEception);
}

TEST_F(ArrayTest, explode) {
	ASSERT_NO_THROW({
			ppl7::Array a1;
			a1.explode(L"red green blue yellow black white",L" ");
			ASSERT_EQ((size_t)6,a1.count()) << "Array does not contain 6 elements";
			ppl7::String merged=a1.implode(" ");
			ASSERT_EQ(ppl7::String(L"red green blue yellow black white"),merged) << "Elements with wrong value";
	}
	);
}

TEST_F(ArrayTest, explodeWithEmptyElements) {
	ASSERT_NO_THROW({
			ppl7::Array a1;
			a1.explode(L"red green blue   yellow black white",L" ",0,false);
			ASSERT_EQ((size_t)8,a1.count()) << "Array does not contain 8 elements";
			ASSERT_EQ(ppl7::String(L"blue"),a1.get(2)) << "Element 2 has wrong value";
			ASSERT_EQ(ppl7::String(L""),a1.get(3)) << "Element 3 has wrong value";
			ASSERT_EQ(ppl7::String(L""),a1.get(4)) << "Element 4 has wrong value";
			ASSERT_EQ(ppl7::String(L"yellow"),a1.get(5)) << "Element 5 has wrong value";
			ppl7::String merged=a1.implode(" ");
			ASSERT_EQ(ppl7::String(L"red green blue   yellow black white"),merged) << "Elements with wrong value";
	}
	);
}

TEST_F(ArrayTest, explodeWithoutEmptyElements) {
	ASSERT_NO_THROW({
			ppl7::Array a1;
			a1.explode(L"red green blue   yellow black white",L" ",0,true);
			ASSERT_EQ((size_t)6,a1.count()) << "Array does not contain 6 elements";
			ASSERT_EQ(ppl7::String(L"blue"),a1.get(2)) << "Element 2 has wrong value";
			ASSERT_EQ(ppl7::String(L"yellow"),a1.get(3)) << "Element 3 has wrong value";
			ppl7::String merged=a1.implode(" ");
			ASSERT_EQ(ppl7::String(L"red green blue yellow black white"),merged) << "Elements with wrong value";
	}
	);
}

TEST_F(ArrayTest, explodeWithLimitWithinEmptyLines1) {
	ASSERT_NO_THROW({
			ppl7::Array a1;
			a1.explode(L"red green blue   yellow black white",L" ",4,true);
			ASSERT_EQ((size_t)4,a1.count()) << "Array does not contain 6 elements";
			ASSERT_EQ(ppl7::String(L"blue"),a1.get(2)) << "Element 2 has wrong value";
			ASSERT_EQ(ppl7::String(L"yellow"),a1.get(3)) << "Element 3 has wrong value";
	}
	);
}

TEST_F(ArrayTest, explodeWithLimitWithinEmptyLines2) {
	ASSERT_NO_THROW({
			ppl7::Array a1;
			a1.explode(L"red green blue   yellow black white",L" ",4,false);
			ASSERT_EQ((size_t)4,a1.count()) << "Array does not contain 6 elements";
			ASSERT_EQ(ppl7::String(L"blue"),a1.get(2)) << "Element 2 has wrong value";
			ASSERT_EQ(ppl7::String(L""),a1.get(3)) << "Element 3 has wrong value";
	}
	);
}


TEST_F(ArrayTest, implode) {
	ASSERT_NO_THROW({
			ppl7::Array a1(L"red green blue yellow black white",L" ");
			ppl7::String merged=a1.implode(" ");
			ASSERT_EQ(ppl7::String(L"red green blue yellow black white"),merged) << "Elements with wrong value";
	}
	);
}

TEST_F(ArrayTest, sortWithDupes) {
	ASSERT_NO_THROW({
			ppl7::Array a1(L"red white green blue red yellow black white",L" ");
			ASSERT_EQ((size_t)8,a1.count()) << "Array does not contain 8 elements";
			a1.sort();
			ASSERT_EQ((size_t)8,a1.count()) << "Array does not contain 8 elements";
			ppl7::String merged=a1.implode(" ");
			ASSERT_EQ(ppl7::String(L"black blue green red red white white yellow"),merged) << "Elements with wrong value";
	}
	);
}

TEST_F(ArrayTest, sortReverse) {
	ASSERT_NO_THROW({
			ppl7::Array a1(L"red white green blue red yellow black white",L" ");
			ASSERT_EQ((size_t)8,a1.count()) << "Array does not contain 8 elements";
			a1.sortReverse();
			ASSERT_EQ((size_t)8,a1.count()) << "Array does not contain 8 elements";
			ppl7::String merged=a1.implode(" ");
			ASSERT_EQ(ppl7::String(L"yellow white white red red green blue black"),merged) << "Elements with wrong value";
	}
	);
}

TEST_F(ArrayTest, sortWithoutDupes) {
	ppl7::Array a1(L"red white green blue red yellow black white",L" ");
	ASSERT_EQ((size_t)8,a1.count()) << "Array does not contain 8 elements";
	ASSERT_NO_THROW({
		a1.sortUnique();
	});
	ASSERT_EQ((size_t)6,a1.count()) << "Array does not contain 6 elements";
	ppl7::String merged=a1.implode(" ");
	ASSERT_EQ(ppl7::String(L"black blue green red white yellow"),merged) << "Elements with wrong value";

}

TEST_F(ArrayTest, makeUnique) {
	ppl7::Array a1(L"red white green blue red yellow black white",L" ");
	ASSERT_EQ((size_t)8,a1.count()) << "Array does not contain 8 elements";
	ASSERT_NO_THROW({
		a1.makeUnique();
	});
	ASSERT_EQ((size_t)6,a1.count()) << "Array does not contain 6 elements";
	ppl7::String merged=a1.implode(" ");
	ASSERT_EQ(ppl7::String(L"red white green blue yellow black"),merged) << "Elements with wrong value";
}



}


int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

