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

bin/avltree: src/avltree.cpp Makefile
	mkdir -p bin
	$(CC) -o bin/avltree src/avltree.cpp $(CFLAGS) $(LIB)

clean:
	-rm -rf bin

