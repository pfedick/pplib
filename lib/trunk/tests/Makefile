GTEST_DIR = /usr/local/gtest-1.6.0
CC		= gcc
EXTRA_CFLAGS = 
INCLUDE	= -I. -I$(GTEST_DIR)/include
CFLAGS	= -ggdb -Wall  $(INCLUDE) `../ppl7-config --cflags debug source` $(EXTRA_CFLAGS)
LIBS	= `../ppl7-config --libs debug source` -lstdc++
PROGNAME = testsuite

OBJECTS = compile/dir.o compile/main.o compile/strings.o compile/datetime.o compile/array.o \
	compile/memoryheap.o compile/memorygroup.o compile/wordlist.o \
	compile/bytearray.o compile/bytearrayptr.o \
	compile/avltree.o compile/assocarray.o compile/pointer.o \
	compile/file.o compile/list.o compile/functions.o \
	compile/inet.o 

# compile/grafix_drawable.o compile/grafix_imagefilter.o
	
# compile/gfxreftest.o

$(PROGNAME): compile/libgtest.a $(OBJECTS) ppl7-tests.h
	$(CC) -O -ggdb -o $(PROGNAME) $(CFLAGS) $(OBJECTS) compile/libgtest.a $(LIBS)

all: $(PROGNAME)
	./$(PROGNAME)
	
xml: $(PROGNAME)
	./$(PROGNAME) --gtest_output=xml
	
memcheck: $(PROGNAME)
	valgrind --tool=memcheck --leak-check=full --show-reachable=yes --undef-value-errors=yes --xml=yes --xml-file=testsuite.valgrind $(PROGNAME)

clean:
	-rm -rf compile tmp.grafixtest.png
	-rm *.core

compile/libgtest.a:
	mkdir -p compile
	g++ -I${GTEST_DIR}/include -I${GTEST_DIR} -o compile/gtest-all.o -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv compile/libgtest.a compile/gtest-all.o


compile/main.o: src/main.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) $(CFLAGS) -o compile/main.o -c src/main.cpp

compile/functions.o: src/functions.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) $(CFLAGS) -o compile/functions.o -c src/functions.cpp

compile/pointer.o: src/pointer.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/pointer.o -c src/pointer.cpp $(CFLAGS) $(LIB)

compile/datetime.o: src/datetime.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/datetime.o -c src/datetime.cpp $(CFLAGS) $(LIB)

compile/bytearray.o: src/bytearray.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/bytearray.o -c src/bytearray.cpp $(CFLAGS) $(LIB)

compile/bytearrayptr.o: src/bytearrayptr.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/bytearrayptr.o -c src/bytearrayptr.cpp $(CFLAGS) $(LIB)

compile/strings.o: src/strings.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/strings.o -c src/strings.cpp $(CFLAGS) $(LIB)

compile/array.o: src/array.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/array.o -c src/array.cpp $(CFLAGS) $(LIB)

compile/memoryheap.o: src/memoryheap.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/memoryheap.o -c src/memoryheap.cpp $(CFLAGS) $(LIB)

compile/memorygroup.o: src/memorygroup.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/memorygroup.o -c src/memorygroup.cpp $(CFLAGS) $(LIB)

compile/avltree.o: src/avltree.cpp Makefile compile/wordlist.o ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/avltree.o -c src/avltree.cpp $(CFLAGS) $(LIB)

compile/wordlist.o: src/wordlist.cpp
	mkdir -p compile
	$(CC) -Wall -O -o compile/wordlist.o -c -c src/wordlist.cpp $(CFLAGS)

compile/assocarray.o: src/assocarray.cpp Makefile compile/wordlist.o ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/assocarray.o -c src/assocarray.cpp $(CFLAGS) $(LIB)

compile/file.o: src/file.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/file.o -c src/file.cpp $(CFLAGS) $(LIB)

compile/dir.o: src/dir.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/dir.o -c src/dir.cpp $(CFLAGS) $(LIB)

compile/inet.o: src/inet.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/inet.o -c src/inet.cpp $(CFLAGS) $(LIB)

compile/list.o: src/list.cpp Makefile compile/wordlist.o ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/list.o -c src/list.cpp $(CFLAGS) $(LIB)

compile/gfxreftest.o: src/gfxreftest.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/gfxreftest.o -c src/gfxreftest.cpp $(CFLAGS) $(LIB)

compile/grafix_imagefilter.o: src/grafix_imagefilter.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/grafix_imagefilter.o -c src/grafix_imagefilter.cpp $(CFLAGS) $(LIB)
	
compile/grafix_drawable.o: src/grafix_drawable.cpp Makefile ppl7-tests.h
	mkdir -p compile
	$(CC) -o compile/grafix_drawable.o -c src/grafix_drawable.cpp $(CFLAGS) $(LIB)