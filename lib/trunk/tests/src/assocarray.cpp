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
class AssocArrayTest : public ::testing::Test {
	protected:

	AssocArrayTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");

	}
	virtual ~AssocArrayTest() {

	}
};

TEST_F(AssocArrayTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl7::AssocArray a;
		//ASSERT_EQ(ppl7::String(L"success"),res) << "Unexpected return value";
	});

}

TEST_F(AssocArrayTest, addStringsLevel1) {
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
	a.set(L"key1",L"Dieser Wert geht über\nmehrere Zeilen");
	a.set(L"key2",L"value6");
	/*
	a.set(L"array1/unterkey1",L"value2");
	a.set(L"array1/unterkey2",L"value3");
	a.set(L"array1/noch ein array/unterkey1",L"value4");
	a.set(L"array1/unterkey2",L"value5");
	a.set(L"key2",L"value6");
	//a.set(L"dateien/main.cpp",&bin);
	a.set(L"array2/unterkey1",L"value7");
	a.set(L"array2/unterkey2",L"value8");
	a.set(L"array2/unterkey1",L"value9");
	*/
	});
	a.list();
}


}	// EOF namespace

int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);

	ppl7::String w(wordlist);
	Wordlist.reserve(130000);
	Wordlist.explode(w,L"\n");

	return RUN_ALL_TESTS();
}
