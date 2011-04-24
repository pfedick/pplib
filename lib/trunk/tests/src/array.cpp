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
			ASSERT_EQ(ppl7::String(L"First Line"),a1.get(1)) << "Element 1 has wrong value";
			ASSERT_EQ(ppl7::String(L"Second Line"),a1.get(2)) << "Element 2 has wrong value";
			ASSERT_EQ(ppl7::String(L"Third Line"),a1.get(3)) << "Element 3 has wrong value";
	}
	);
}

}


int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

