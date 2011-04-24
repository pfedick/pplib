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
}


int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

