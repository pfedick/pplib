#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <ppl7.h>
#include <gtest/gtest.h>

namespace {

// The fixture for testing class Foo.
class StringTest : public ::testing::Test {
	protected:
	StringTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~StringTest() {

	}
};

TEST_F(StringTest, ConstructorSimple) {
	ASSERT_NO_THROW({
			ppl7::String s1;
			ASSERT_TRUE(s1.len()==0) << "String does not habe zero length";
			const wchar_t *buf=s1.getPtr();
			ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a widecharacter string";
			ASSERT_TRUE(0==buf[0]) << "Class did not return an empty widecharacter string";

	}
	);
}

TEST_F(StringTest, ConstructorFromCharPtr) {
	ASSERT_NO_THROW({
			ppl7::String s1("A test string with unicode characters: äöü");
			ASSERT_TRUE(s1.len()==42) << "String does not length of 42";
			const wchar_t *buf=s1.getPtr();
			ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a widecharacter string";
			ASSERT_TRUE(L'A'==buf[0]) << "Class did not return an empty widecharacter string";
			ASSERT_TRUE(L'ü'==buf[41]) << "unexpected unicode character";

	}
	);
}

TEST_F(StringTest, ConstructorFromWideCharPtr) {
	ASSERT_NO_THROW({
			ppl7::String s1(L"A test string with unicode characters: äöü");
			ASSERT_TRUE(s1.len()==42) << "String does not length of 42";
			const wchar_t *buf=s1.getPtr();
			ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a widecharacter string";
			ASSERT_TRUE(L'A'==buf[0]) << "Class did not return an empty widecharacter string";
			ASSERT_TRUE(L'ü'==buf[41]) << "unexpected unicode character";

	}
	);
}

TEST_F(StringTest, ConstructorFromStringPtr) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	ppl7::String s2(&s1);
	ASSERT_TRUE(s2.len()==42) << "String does not length of 42";
	const wchar_t *buf=s2.getPtr();
	ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a widecharacter string";
	ASSERT_TRUE(L'A'==buf[0]) << "Class did not return an empty widecharacter string";
	ASSERT_TRUE(L'ü'==buf[41]) << "unexpected unicode character";
}

TEST_F(StringTest, ConstructorFromString) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	ppl7::String s2(s1);
	ASSERT_TRUE(s2.len()==42) << "String does not length of 42";
	const wchar_t *buf=s2.getPtr();
	ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a widecharacter string";
	ASSERT_TRUE(L'A'==buf[0]) << "Class did not return an empty widecharacter string";
	ASSERT_TRUE(L'ü'==buf[41]) << "unexpected unicode character";
}

TEST_F(StringTest, ConstructorFromStdString) {
	std::string s1("A test string with unicode characters: äöü");
	ppl7::String s2(s1);
	ASSERT_TRUE(s2.len()==42) << "String does not length of 42";
	const wchar_t *buf=s2.getPtr();
	ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a widecharacter string";
	ASSERT_TRUE(L'A'==buf[0]) << "Class did not return an empty widecharacter string";
	ASSERT_TRUE(L'ü'==buf[41]) << "unexpected unicode character";
}

TEST_F(StringTest, ConstructorFromStdWString) {
	std::wstring s1(L"A test string with unicode characters: äöü");
	ppl7::String s2(s1);
	ASSERT_TRUE(s2.len()==42) << "String does not length of 42";
	const wchar_t *buf=s2.getPtr();
	ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a widecharacter string";
	ASSERT_TRUE(L'A'==buf[0]) << "Class did not return an empty widecharacter string";
	ASSERT_TRUE(L'ü'==buf[41]) << "unexpected unicode character";
}

TEST_F(StringTest, clear) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	s1.clear();
	ASSERT_TRUE(s1.len()==0) << "String does not have zero length";
	const wchar_t *buf=s1.getPtr();
	ASSERT_TRUE(NULL!=buf) << "Class did not return a pointer to a widecharacter string";
	ASSERT_TRUE(0==buf[0]) << "Class did not return an empty widecharacter string";
}

TEST_F(StringTest, CapacityAndReserve) {
	ppl7::String s1;
	ASSERT_TRUE(s1.capacity()==0) << "capacity did not return 0";
	s1.reserve(0);
	ASSERT_TRUE(s1.capacity()==0) << "capacity did not return 0";

	s1.reserve(32);
	ASSERT_TRUE(s1.capacity()==32) << "capacity did not return 32";
	s1.reserve(128);
	ASSERT_TRUE(s1.capacity()==128) << "capacity did not return 128";
	s1.reserve(32);
	ASSERT_TRUE(s1.capacity()==128) << "capacity did not return 128";
}

TEST_F(StringTest, len) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.len(),(size_t)42) << "String has unexpected size";
}

TEST_F(StringTest, length) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.length(),(size_t)42) << "String has unexpected size";
}

TEST_F(StringTest, size) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.size(),(size_t)42) << "String has unexpected size";
}

TEST_F(StringTest, isEmpty) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	ppl7::String s2;
	ASSERT_EQ(s1.isEmpty(),false) << "String is empty";
	ASSERT_EQ(s2.isEmpty(),true) << "String not empty";
}

TEST_F(StringTest, notEmpty) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	ppl7::String s2;
	ASSERT_EQ(s1.notEmpty(),true) << "String is not empty";
	ASSERT_EQ(s2.notEmpty(),false) << "String is empty";
}

TEST_F(StringTest, isNumeric) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.isNumeric(),false) << "String should not be numeric";
	s1.set(L"");
	ASSERT_EQ(s1.isNumeric(),false) << "String should not be numeric";
	s1.set(L"12345abcd");
	ASSERT_EQ(s1.isNumeric(),false) << "String should not be numeric";
	s1.set(L"12345");
	ASSERT_EQ(s1.isNumeric(),true) << "String should be numeric";
	s1.set(L"123.451,12");
	ASSERT_EQ(s1.isNumeric(),true) << "String should be numeric";
	s1.set(L"-123.451,12");
	ASSERT_EQ(s1.isNumeric(),true) << "String should be numeric";
	s1.set(L"-123.451,12-9");
	ASSERT_EQ(s1.isNumeric(),false) << "String should not be numeric";
}

TEST_F(StringTest, isInteger) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.isInteger(),false) << "String should not be an integer";
	s1.set(L"");
	ASSERT_EQ(s1.isInteger(),false) << "String should not be an integer";
	s1.set(L"12345abcd");
	ASSERT_EQ(s1.isInteger(),false) << "String should not be an integer";
	s1.set(L"12345");
	ASSERT_EQ(s1.isInteger(),true) << "String should be an integer";
	s1.set(L"-12345");
	ASSERT_EQ(s1.isInteger(),true) << "String should be an integer";
	s1.set(L"123-451");
	ASSERT_EQ(s1.isInteger(),false) << "String should be an integer";
	s1.set(L"123.451,12");
	ASSERT_EQ(s1.isInteger(),false) << "String should be an integer";
	s1.set(L"-123.451,12");
	ASSERT_EQ(s1.isInteger(),false) << "String should be an integer";
	s1.set(L"-123.451,12-9");
	ASSERT_EQ(s1.isInteger(),false) << "String should not be an integer";
}

TEST_F(StringTest, isTrue) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set(L"");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set(L"12345abcd");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set(L"1");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set(L"12345");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set(L"true");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set(L"wahr");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set(L"ja");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set(L"yes");
	ASSERT_EQ(s1.isTrue(),true) << "String should be true";
	s1.set(L"false");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set(L"falsch");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set(L"nein");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
	s1.set(L"no");
	ASSERT_EQ(s1.isTrue(),false) << "String should not be true";
}

TEST_F(StringTest, isFalse) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"12345abcd");
	ASSERT_EQ(s1.isFalse(),true) << "String should not be false";
	s1.set(L"12345");
	ASSERT_EQ(s1.isFalse(),true) << "String should not be false";
	s1.set(L"0");
	ASSERT_EQ(s1.isFalse(),true) << "String should not be false";
	s1.set(L"true");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"wahr");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"ja");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"yes");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"false");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"falsch");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"nein");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
	s1.set(L"no");
	ASSERT_EQ(s1.isFalse(),true) << "String should be false";
}


TEST_F(StringTest, chopRight) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	ppl7::String s2(L"A test string with unicode characters: ä");
	ppl7::String s3(L"A test string with unicode characters: ");
	s1.chopRight(2);
	ASSERT_TRUE(s1.len()==40) << "String does not have length of 40";
	ASSERT_TRUE(s1==s2) << "String has unexpected value";
	s1.chopRight();
	ASSERT_TRUE(s1.len()==39) << "String does not have length of 39";
	ASSERT_TRUE(s1==s3) << "String has unexpected value";
	s1.chopRight(44);
	ASSERT_TRUE(s1.len()==0) << "String does not have length of 0";
	s2.clear();
	ASSERT_TRUE(s1==s2) << "String has unexpected value";
}

TEST_F(StringTest, chopLeft) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	ppl7::String s2(L"est string with unicode characters: äöü");
	ppl7::String s3(L"st string with unicode characters: äöü");
	s1.chopLeft(3);
	ASSERT_TRUE(s1.len()==39) << "String does not have length of 39";
	ASSERT_TRUE(s1==s2) << "String has unexpected value";
	s1.chopLeft();
	ASSERT_TRUE(s1.len()==38) << "String does not have length of 38";
	ASSERT_TRUE(s1==s3) << "String has unexpected value";
	s1.chopRight(44);
	ASSERT_TRUE(s1.len()==0) << "String does not have length of 0";
	s2.clear();
	ASSERT_TRUE(s1==s2) << "String has unexpected value";
}

TEST_F(StringTest, chop) {
	ppl7::String s1(L"A test string with unicode characters: äöü");
	ppl7::String s2(L"A test string with unicode characters: ä");
	ppl7::String s3(L"A test string with unicode characters: ");
	s1.chop(2);
	ASSERT_TRUE(s1.len()==40) << "String does not have length of 40";
	ASSERT_TRUE(s1==s2) << "String has unexpected value";
	s1.chopRight();
	ASSERT_TRUE(s1.len()==39) << "String does not have length of 39";
	ASSERT_TRUE(s1==s3) << "String has unexpected value";
	s1.chop(44);
	ASSERT_TRUE(s1.len()==0) << "String does not have length of 0";
	s2.clear();
	ASSERT_TRUE(s1==s2) << "String has unexpected value";
}

TEST_F(StringTest, chomp) {
	ppl7::String s1(L"\n\r\nA test string\näöü\n\r\n");
	ppl7::String s2(L"A test string\näöü");
	s1.chomp();
	ASSERT_TRUE(s1.len()==17) << "String does not have length of 17";
	ASSERT_TRUE(s1==s2) << "String has unexpected value";
	s1.chomp();
	ASSERT_TRUE(s1.len()==17) << "String does not have length of 17";
	ASSERT_TRUE(s1==s2) << "String has unexpected value";
}

TEST_F(StringTest, strstr) {
	ppl7::String s1(L"A test haystack string");
	ppl7::String s3(L"haystack string");
	ppl7::String s2;
	s2=s1.strstr("haystack");
	ASSERT_TRUE(s2.len()==15) << "String does not have length of 15";
	ASSERT_TRUE(s2==s3) << "String has unexpected value";
	s2=s1.strstr("beer");
	s3="";
	ASSERT_TRUE(s2.len()==0) << "String does not have length of 0";
	ASSERT_TRUE(s2==s3) << "String has unexpected value";

	s2=s1.strstr("");
	ASSERT_TRUE(s2==s1) << "String has unexpected value";

}

TEST_F(StringTest, repeated) {
	ppl7::String s1(L"_repeat_");
	ppl7::String s3(L"_repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat__repeat_");
	ppl7::String s2;
	s2=s1.repeated(10);
	ASSERT_TRUE(s2.len()==80) << "String does not have length of 80";
	ASSERT_TRUE(s2==s3) << "String has unexpected value";
}


}


int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

#ifdef old
	printf ("sizeof String: %zd\n",sizeof(ppl7::String));
	if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
		printf ("setlocale fehlgeschlagen\n");
		return 1;
	}

	try {
		//ppl7::String::setGlobalEncoding("iso-8859-1");
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	catch (ppl7::NullPointerException) {
		printf ("NPE\n");
		return 1;
	}
	catch (ppl7::UnsupportedFeatureException) {
		printf ("Transkodierung wird nicht unterstützt\n");
		return 1;
	}
	catch (ppl7::UnsupportedCharacterEncodingException) {
		printf ("UnsupportedFeatureException: Transkodierung wird nicht unterstützt\n");
		return 1;
	}
	catch(...) {
		printf ("Unbekannte Exception\n");
		throw;
	}

	ppl7::String s1;
	s1.reserve(128);

	s1.setf("Test mit UTF-8 Umlaütän ß!");
	printf("capacity: %zd, length: %zd\n",s1.capacity(),s1.length());

	//s1.set(L"Test mit ISO-8859-1 Umla�t�n �!");
	s1.printnl();

	s1.append("Noch mehr UTF-8");
	printf("capacity: %zd, length: %zd\n",s1.capacity(),s1.length());
	s1.printnl();

	s1.appendf("capacity: %zd\n",s1.capacity());

	printf("capacity: %zd, length: %zd\n",s1.capacity(),s1.length());
	s1.printnl();

	//printf ("UTF-8 String als const char*: >>>%s<<<\n",(const char*)s1.toUtf8());

	printf ("prepend\n");
	s1.prependf("Davor :-)  =>");
	s1.printnl();

	printf ("ok\n");

	s1=".String1.";
	ppl7::String s2;
	//s2=s1+"Hallo Welt"+s1+"Noch mehr"+s1;
	s2=s1+"Hallo Welt"+s1+"Noch mehr";
	s2.printnl();

	ppl7::String s3="Eiñ Täśt zµm Kôdiéren";
	ppl7::ByteArray b=s3.toEncoding("UTF-8");
	b.hexDump();

	printf ("left: %ls\n",(const wchar_t*)s3.left(3));
	printf ("right: %ls\n",(const wchar_t*)s3.right(4));
	printf ("mid (2): %ls\n",(const wchar_t*)s3.mid(2));
	printf ("mid (2,5): %ls\n",(const wchar_t*)s3.mid(2,5));
	printf ("mid (40,5): %ls\n",(const wchar_t*)s3.mid(40,5));

	//s1.hexDump();
	return 0;
}
#endif
