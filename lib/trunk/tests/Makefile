CC		= gcc
EXTRA_CFLAGS =
INCLUDE	= -I.
#CFLAGS	= -ggdb -Wall  $(INCLUDE) $(EXTRA_CFLAGS) `ppl6-config --cflags debug`
#LIB		= -L/local/lib   -lpthread `ppl6-config --libs debug` -mingw32  -lstdc++
CFLAGS	= -ggdb -Wall  `ppl7-config --cflags debug` $(EXTRA_CFLAGS)
LIB		= `ppl7-config --libs debug` -lgtest -lstdc++




strings: strings.cpp Makefile
	$(CC) -o strings strings.cpp $(CFLAGS) $(LIB)


clean:
	-rm -f strings *.core

