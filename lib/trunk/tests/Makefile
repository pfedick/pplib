CC		= gcc
EXTRA_CFLAGS =
INCLUDE	= -I.
#CFLAGS	= -ggdb -Wall  $(INCLUDE) $(EXTRA_CFLAGS) `ppl6-config --cflags debug`
#LIB		= -L/local/lib   -lpthread `ppl6-config --libs debug` -mingw32  -lstdc++
CFLAGS	= -ggdb -Wall  `ppl7-config --cflags debug` $(EXTRA_CFLAGS)
LIB		= `ppl7-config --libs debug` -lgtest -lstdc++


all: heap array strings

heap: bin/heap
	bin/heap

array: bin/array
	bin/array

strings: bin/strings
	bin/strings

bin:
	mkdir -p bin
	
bin/strings: bin src/strings.cpp Makefile
	$(CC) -o bin/strings src/strings.cpp $(CFLAGS) $(LIB)

bin/array: bin src/array.cpp Makefile
	$(CC) -o bin/array src/array.cpp $(CFLAGS) $(LIB)

bin/heap: bin src/heap.cpp Makefile
	$(CC) -o bin/heap src/heap.cpp $(CFLAGS) $(LIB)

clean:
	-rm -rf bin

