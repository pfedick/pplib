CC		= gcc
EXTRA_CFLAGS =
INCLUDE	= -I../include
CFLAGS	= -ggdb -Wall $(INCLUDE) $(EXTRA_CFLAGS)
LIBS	= `../ppl7-config --libs debug source` -lgtest -lstdc++
PROGNAME = testsuite
OBJECTS = memoryheap.o memorygroup.o array.o strings.o avltree.o \
		assocarray.o pointer.o datetime.o bytearray.o bytearrayptr.o \
		functions.o file.o dir.o list.o inet.o \
		grafix_drawable.o grafix_imagefilter.o gfxreftest.o


$(PROGNAME): $(OBJECTS)
	$(CC) -O -ggdb -o $(PROGNAME) $(CFLAGS) $(OBJECTS) $(LIBS)

all: $(PROGNAME)
	./$(PROGNAME)

clean:
	-rm -rf compile tmp.grafixtest.png
	-rm *.core


comile/functions: src/functions.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/functions src/functions.cpp $(CFLAGS) $(LIB)

comile/pointer: src/pointer.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/pointer src/pointer.cpp $(CFLAGS) $(LIB)

comile/datetime: src/datetime.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/datetime src/datetime.cpp $(CFLAGS) $(LIB)

comile/bytearray: src/bytearray.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/bytearray src/bytearray.cpp $(CFLAGS) $(LIB)

comile/bytearrayptr: src/bytearrayptr.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/bytearrayptr src/bytearrayptr.cpp $(CFLAGS) $(LIB)

comile/strings: src/strings.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/strings src/strings.cpp $(CFLAGS) $(LIB)

comile/array: src/array.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/array src/array.cpp $(CFLAGS) $(LIB)

comile/memoryheap: src/memoryheap.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/memoryheap src/memoryheap.cpp $(CFLAGS) $(LIB)

comile/memorygroup: src/memorygroup.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/memorygroup src/memorygroup.cpp $(CFLAGS) $(LIB)

comile/avltree: src/avltree.cpp Makefile comile/wordlist.o
	mkdir -p comile
	$(CC) -o comile/avltree comile/wordlist.o src/avltree.cpp $(CFLAGS) $(LIB)

comile/wordlist.o: src/wordlist.cpp
	mkdir -p comile
	$(CC) -Wall -O -o comile/wordlist.o -c src/wordlist.cpp $(CFLAGS)

comile/assocarray: src/assocarray.cpp Makefile comile/wordlist.o
	mkdir -p comile
	$(CC) -o comile/assocarray comile/wordlist.o src/assocarray.cpp $(CFLAGS) $(LIB)

comile/file: src/file.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/file src/file.cpp $(CFLAGS) $(LIB)

comile/dir: src/dir.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/dir src/dir.cpp $(CFLAGS) $(LIB)

comile/inet: src/inet.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/inet src/inet.cpp $(CFLAGS) $(LIB)

comile/list: src/list.cpp Makefile comile/wordlist.o
	mkdir -p comile
	$(CC) -o comile/list comile/wordlist.o src/list.cpp $(CFLAGS) $(LIB)

comile/gfxreftest: src/gfxreftest.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/gfxreftest src/gfxreftest.cpp $(CFLAGS) $(LIB)

comile/grafix_imagefilter: src/grafix_imagefilter.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/grafix_imagefilter src/grafix_imagefilter.cpp $(CFLAGS) $(LIB)
	
comile/grafix_drawable: src/grafix_drawable.cpp Makefile
	mkdir -p comile
	$(CC) -o comile/grafix_drawable src/grafix_drawable.cpp $(CFLAGS) $(LIB)