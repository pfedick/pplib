CC		= gcc
EXTRA_CFLAGS =
INCLUDE	= -I.
#CFLAGS	= -ggdb -Wall  $(INCLUDE) $(EXTRA_CFLAGS) `ppl6-config --cflags debug`
#LIB		= -L/local/lib   -lpthread `ppl6-config --libs debug` -mingw32  -lstdc++
CFLAGS	= -ggdb -Wall  `../ppl7-config --cflags debug source` $(EXTRA_CFLAGS)
LIB		= `../ppl7-config --libs debug source` -lgtest -lstdc++


all: heap array strings avltree

heap: bin/heap
	bin/heap

array: bin/array
	bin/array

strings: bin/strings
	bin/strings

avltree: bin/avltree
	bin/avltree

	
bin/strings: src/strings.cpp Makefile
	mkdir -p bin
	$(CC) -o bin/strings src/strings.cpp $(CFLAGS) $(LIB)

bin/array: src/array.cpp Makefile
	mkdir -p bin
	$(CC) -o bin/array src/array.cpp $(CFLAGS) $(LIB)

bin/heap: src/heap.cpp Makefile
	mkdir -p bin
	$(CC) -o bin/heap src/heap.cpp $(CFLAGS) $(LIB)

bin/avltree: src/avltree.cpp Makefile bin/wordlist.o
	mkdir -p bin
	$(CC) -o bin/avltree bin/wordlist.o src/avltree.cpp $(CFLAGS) $(LIB)

bin/wordlist.o: src/wordlist.cpp
	mkdir -p bin
	$(CC) -Wall -O -o bin/wordlist.o -c src/wordlist.cpp $(CFLAGS)

clean:
	-rm -rf bin

