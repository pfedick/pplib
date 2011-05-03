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


}	// EOF namespace

int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);

	ppl7::String w(wordlist);
	Wordlist.reserve(130000);
	Wordlist.explode(w,L"\n");

	return RUN_ALL_TESTS();
}

