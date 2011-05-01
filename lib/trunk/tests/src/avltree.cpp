#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <ppl7.h>
#include <gtest/gtest.h>

namespace {

// The fixture for testing class Foo.
class AVLTreeTest : public ::testing::Test {
	protected:
	AVLTreeTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~AVLTreeTest() {

	}
};

TEST_F(AVLTreeTest, ConstructorSimple) {
	ppl7::AVLTree<ppl7::String, ppl7::String> myMap;
	//printf ("sizeof(MyMap::Item)=%zu\n",myMap.itemSize());
	// ACHTUNG: itemSize ändert sich, sobald am Knoten des Baums, dem Datentyp des Keys oder
	// dem Datentyp des Values etwas verändert wird
	ASSERT_EQ((size_t)120,myMap.itemSize()) << "Tree Item has unexpected size";
}

TEST_F(AVLTreeTest, add) {
	ppl7::AVLTree<ppl7::String, ppl7::String> myMap;
	myMap.add(L"key1",L"value1");
	ASSERT_EQ((size_t)1,myMap.count()) << "Tree has unexpected size";
	myMap.add(L"other",L"value2");
	ASSERT_EQ((size_t)2,myMap.count()) << "Tree has unexpected size";
}

TEST_F(AVLTreeTest, find) {
	ppl7::AVLTree<ppl7::String, ppl7::String> myMap;
	myMap.add(L"key1",L"value1");
	myMap.add(L"other",L"value2");
	myMap.add(L"findme",L"success");
	myMap.add(L"key3",L"value3");
	myMap.add(L"abc",L"value4");
	ASSERT_EQ((size_t)5,myMap.count()) << "Tree has unexpected size";
	ASSERT_NO_THROW({
		const ppl7::String &res=myMap.find(L"findme");
		ASSERT_EQ(ppl7::String(L"success"),res) << "Unexpected return value";
	});

	ASSERT_THROW({
		myMap.find(L"findmenot");
	},ppl7::ItemNotFoundException);

}

TEST_F(AVLTreeTest, getNext) {
	ppl7::AVLTree<ppl7::String, ppl7::String> myMap;
	myMap.add(L"key1",L"value1");
	myMap.add(L"other",L"value2");
	myMap.add(L"findme",L"success");
	myMap.add(L"key3",L"value3");
	myMap.add(L"abc",L"value4");
	ASSERT_EQ((size_t)5,myMap.count()) << "Tree has unexpected size";
	ppl7::AVLTree<ppl7::String, ppl7::String>::Iterator it;
	ppl7::AVLTree<ppl7::String, ppl7::String>::Pair pair;
	myMap.reset(it);

	ASSERT_NO_THROW({
		pair=myMap.getNext(it);
		printf ("Found Key >>%ls<< with Value >>%ls<<\n",
				(const wchar_t*)pair.key(), (const wchar_t*)pair.value());
		pair=myMap.getNext(it);
		printf ("Found Key >>%ls<< with Value >>%ls<<\n",
				(const wchar_t*)pair.key(), (const wchar_t*)pair.value());
		pair=myMap.getNext(it);
		printf ("Found Key >>%ls<< with Value >>%ls<<\n",
				(const wchar_t*)pair.key(), (const wchar_t*)pair.value());
		pair=myMap.getNext(it);
		printf ("Found Key >>%ls<< with Value >>%ls<<\n",
				(const wchar_t*)pair.key(), (const wchar_t*)pair.value());
		pair=myMap.getNext(it);
		printf ("Found Key >>%ls<< with Value >>%ls<<\n",
				(const wchar_t*)pair.key(), (const wchar_t*)pair.value());

	});

	ASSERT_THROW({
		myMap.getNext(it);
	},ppl7::OutOfBoundsEception);

}

}	// EOF namespace

int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
