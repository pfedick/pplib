CC		= gcc
EXTRA_CFLAGS =
INCLUDE	= -I.
#CFLAGS	= -ggdb -Wall  $(INCLUDE) $(EXTRA_CFLAGS) `ppl6-config --cflags debug`
#LIB		= -L/local/lib   -lpthread `ppl6-config --libs debug` -mingw32  -lstdc++
CFLAGS	= -ggdb -Wall  `ppl7-config --cflags debug` $(EXTRA_CFLAGS)
LIB		= `ppl7-config --libs debug` -lgtest -lstdc++


run: bin bin/strings bin/array
	bin/strings
	bin/array

bin:
	mkdir -p bin
	
bin/strings: bin src/strings.cpp Makefile
	$(CC) -o bin/strings src/strings.cpp $(CFLAGS) $(LIB)

bin/array: bin src/array.cpp Makefile
	$(CC) -o bin/array src/array.cpp $(CFLAGS) $(LIB)

clean:
	-rm -rf bin

