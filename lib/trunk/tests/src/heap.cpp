#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <ppl7.h>
#include <gtest/gtest.h>

namespace {

// The fixture for testing class Foo.
class HeapTest : public ::testing::Test {
	protected:
	HeapTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~HeapTest() {

	}
};

TEST_F(HeapTest, ConstructorSimple) {
	ppl7::Heap h1;
	ASSERT_EQ(h1.capacity(),(size_t)0) << "capacity is not 0";
	ASSERT_EQ(h1.count(),(size_t)0) << "count is not 0";
	ASSERT_EQ(h1.memoryAllocated(),(size_t)sizeof(ppl7::Heap)) << "memoryAllocated is not sizeof(Heap)";
	ASSERT_EQ(h1.memoryUsed(),(size_t)sizeof(ppl7::Heap)) << "memoryUsed is not sizeof(Heap)";
	ASSERT_EQ(h1.elementSize(),(size_t)0) << "elementSize is not 0";
}

TEST_F(HeapTest, ConstructorWithInit) {
	ppl7::Heap h1(32,100,100);
	ASSERT_EQ(h1.capacity(),(size_t)100) << "capacity is not 100";
	ASSERT_EQ(h1.count(),(size_t)0) << "count is not 0";
	ASSERT_GT(h1.memoryAllocated(),(size_t)sizeof(ppl7::Heap)) << "memoryAllocated is greater than sizeof(Heap)";
	ASSERT_GT(h1.memoryUsed(),(size_t)sizeof(ppl7::Heap)) << "memoryUsed is not greater than sizeof(Heap)";
	ASSERT_EQ(h1.elementSize(),(size_t)32) << "elementSize is not 32";
}

TEST_F(HeapTest, Init) {
	ppl7::Heap h1;
	h1.init(32,100,100);
	ASSERT_EQ(h1.capacity(),(size_t)100) << "capacity is not 100";
	ASSERT_EQ(h1.count(),(size_t)0) << "count is not 0";
	ASSERT_GT(h1.memoryAllocated(),(size_t)sizeof(ppl7::Heap)) << "memoryAllocated is greater than sizeof(Heap)";
	ASSERT_GT(h1.memoryUsed(),(size_t)sizeof(ppl7::Heap)) << "memoryUsed is not greater than sizeof(Heap)";
	ASSERT_EQ(h1.elementSize(),(size_t)32) << "elementSize is not 32";
}

TEST_F(HeapTest, malloc) {
	ppl7::Heap h1(32,100,100);
	void *ptr=h1.malloc();
	ASSERT_TRUE(ptr!=NULL) << "malloc failed";
	ASSERT_EQ(h1.count(),(size_t)1) << "count is not 1";
}

TEST_F(HeapTest, 101malloc) {
	ppl7::Heap h1(32,100,100,30);
	for (int i=0;i<101;i++) {
		void *ptr=h1.malloc();
		ASSERT_TRUE(ptr!=NULL) << "malloc failed";
	}
	//h1.dump();
	ASSERT_EQ(h1.count(),(size_t)101) << "count is not 101";
	ASSERT_EQ(h1.capacity(),(size_t)200) << "capacity is not 200";

}

TEST_F(HeapTest, 1000malloc) {
	ppl7::Heap h1(32,100,100,30);
	for (int i=0;i<1000;i++) {
		void *ptr=h1.malloc();
		ASSERT_TRUE(ptr!=NULL) << "malloc failed";
	}
	//h1.dump();
	ASSERT_EQ(h1.count(),(size_t)1000) << "count is not 1000";
	ASSERT_EQ(h1.capacity(),(size_t)1002) << "capacity is not 1002";

}

TEST_F(HeapTest, 1000calloc) {
	ppl7::Heap h1(32,100,100,30);
	for (int i=0;i<1000;i++) {
		void *ptr=h1.calloc();
		ASSERT_TRUE(ptr!=NULL) << "calloc failed";
	}
	//h1.dump();
	ASSERT_EQ(h1.count(),(size_t)1000) << "count is not 1000";
	ASSERT_EQ(h1.capacity(),(size_t)1002) << "capacity is not 1002";

}

TEST_F(HeapTest, 1000malloc1000free) {
	ppl7::Heap h1(32,100,100,30);
	void *ptr[1000];
	for (int i=0;i<1000;i++) {
		ptr[i]=h1.malloc();
		ASSERT_TRUE(ptr!=NULL) << "malloc failed";
	}
	for (int i=0;i<1000;i++) {
		h1.free(ptr[i]);
	}
	//h1.dump();
	ASSERT_EQ(h1.count(),(size_t)0) << "count is not 0";
	ASSERT_EQ(h1.capacity(),(size_t)1002) << "capacity is not 1002";
}

TEST_F(HeapTest, 2000malloc1500free) {
	ppl7::Heap h1(32,100,100,30);
	void *ptr[2000];
	for (int i=0;i<2000;i++) {
		ptr[i]=h1.malloc();
		ASSERT_TRUE(ptr!=NULL) << "malloc failed";
	}
	//h1.dump();
	for (int i=0;i<1500;i++) {
		h1.free(ptr[i]);
	}
	//h1.dump();
	ASSERT_EQ(h1.count(),(size_t)500) << "count is not 500";
	ASSERT_EQ(h1.capacity(),(size_t)1973) << "capacity is not 1973";
}

TEST_F(HeapTest, 1000mallocfreeCleanup) {
	ppl7::Heap h1(32,100,100,30);
	void *ptr[1000];
	for (int i=0;i<1000;i++) {
		ptr[i]=h1.malloc();
		ASSERT_TRUE(ptr!=NULL) << "malloc failed";
	}
	//h1.dump();
	for (int i=0;i<1000;i++) {
		h1.free(ptr[i]);
	}
	h1.cleanup();
	//h1.dump();
	ASSERT_EQ(h1.count(),(size_t)0) << "count is not 0";
	ASSERT_EQ(h1.capacity(),(size_t)284) << "capacity is not 284";
}

TEST_F(HeapTest, reserve2000) {
	ppl7::Heap h1(32,0,100,30);
	h1.reserve(2000);
	//h1.dump();
	ASSERT_EQ(h1.count(),(size_t)0) << "count is not 0";
	ASSERT_EQ(h1.capacity(),(size_t)2000) << "capacity is not 2000";
}

TEST_F(HeapTest, reserve2000and3000) {
	ppl7::Heap h1(32,0,100,30);
	h1.reserve(2000);
	h1.reserve(3000);
	//h1.dump();
	ASSERT_EQ(h1.count(),(size_t)0) << "count is not 0";
	ASSERT_EQ(h1.capacity(),(size_t)3000) << "capacity is not 2000";
}

TEST_F(HeapTest, clear) {
	ppl7::Heap h1(32,0,100,30);
	for (int i=0;i<1000;i++) {
		void *ptr=h1.calloc();
		ASSERT_TRUE(ptr!=NULL) << "calloc failed";
	}
	h1.clear();
	ASSERT_EQ(h1.count(),(size_t)0) << "count is not 0";
	ASSERT_EQ(h1.capacity(),(size_t)0) << "capacity is not 0";
}


}	// EOF namespace



int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}