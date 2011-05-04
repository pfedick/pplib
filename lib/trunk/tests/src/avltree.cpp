#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <ppl7.h>
#include <gtest/gtest.h>


extern const wchar_t *wordlist;

ppl7::Array Wordlist;

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
	//ASSERT_EQ((size_t)120,myMap.itemSize()) << "Tree Item has unexpected size";
}

TEST_F(AVLTreeTest, add) {
	ppl7::AVLTree<ppl7::String, ppl7::String> myMap;
	myMap.add(L"key1",L"value1");
	ASSERT_EQ((size_t)1,myMap.count()) << "Tree has unexpected size";
	myMap.add(L"other",L"value2");
	ASSERT_EQ((size_t)2,myMap.count()) << "Tree has unexpected size";
}

TEST_F(AVLTreeTest, addAndDeleteWordlist) {
	ppl7::AVLTree<ppl7::String, ppl7::String> myMap;
	size_t total=Wordlist.count();
	myMap.reserve(total+10);
	ASSERT_EQ((size_t)125346,Wordlist.count()) << "List has unexpected size";
	ppl7::PrintDebugTime ("Wortliste in AVLTree laden\n");
	for (size_t i=0;i<total;i++) {
		myMap.add(Wordlist[i],L"");
	}
	ASSERT_EQ(total,myMap.count()) << "Tree has unexpected size";
	ppl7::PrintDebugTime ("done\n");
	ppl7::PrintDebugTime ("Wortliste aus AVLTree loeschen\n");
	for (size_t i=0;i<total;i++) {
		myMap.erase(Wordlist[i]);
	}
	ppl7::PrintDebugTime ("done\n");
	ASSERT_EQ((size_t)0,myMap.count()) << "Tree has unexpected size";

}

TEST_F(AVLTreeTest, addAndClearWordlist) {
	ppl7::AVLTree<ppl7::String, ppl7::String> myMap;
	size_t total=Wordlist.count();
	myMap.reserve(total+10);
	for (size_t i=0;i<total;i++) {
		myMap.add(Wordlist[i],L"");
	}
	myMap.clear();
	ASSERT_EQ((size_t)0,myMap.count()) << "Tree has unexpected size";
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
	myMap.reset(it);

	ASSERT_EQ(true, myMap.getNext(it)) << "getNext has not returned true";
	ASSERT_EQ(ppl7::String(L"abc"),it.key()) << "key has unexpected value";
	ASSERT_EQ(ppl7::String(L"value4"),it.value()) << "value has unexpected value";

	ASSERT_EQ(true, myMap.getNext(it)) << "getNext has not returned true";
	ASSERT_EQ(ppl7::String(L"findme"),it.key()) << "key has unexpected value";
	ASSERT_EQ(ppl7::String(L"success"),it.value()) << "value has unexpected value";

	ASSERT_EQ(true, myMap.getNext(it)) << "getNext has not returned true";
	ASSERT_EQ(ppl7::String(L"key1"),it.key()) << "key has unexpected value";
	ASSERT_EQ(ppl7::String(L"value1"),it.value()) << "value has unexpected value";

	ASSERT_EQ(true, myMap.getNext(it)) << "getNext has not returned true";
	ASSERT_EQ(ppl7::String(L"key3"),it.key()) << "key has unexpected value";
	ASSERT_EQ(ppl7::String(L"value3"),it.value()) << "value has unexpected value";

	ASSERT_EQ(true, myMap.getNext(it)) << "getNext has not returned true";
	ASSERT_EQ(ppl7::String(L"other"),it.key()) << "key has unexpected value";
	ASSERT_EQ(ppl7::String(L"value2"),it.value()) << "value has unexpected value";

	ASSERT_EQ(false, myMap.getNext(it)) << "getNext has not returned false";
}

}	// EOF namespace

int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);

	ppl7::PrintDebugTime ("Wortliste in String laden\n");
	ppl7::String w(wordlist);
	Wordlist.reserve(130000);
	ppl7::PrintDebugTime ("Wortliste in Array laden\n");
	Wordlist.explode(w,L"\n");
	ppl7::PrintDebugTime ("done\n");



	return RUN_ALL_TESTS();
}