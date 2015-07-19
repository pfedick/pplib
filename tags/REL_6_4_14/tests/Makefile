CC		= gcc
EXTRA_CFLAGS = -DPPL6LIB
INCLUDE	= -I../include
CFLAGS	= -ggdb -Wall $(INCLUDE) $(EXTRA_CFLAGS)
LIBS	= `../ppl6-config --libs debug source` -lstdc++
PROGNAME = testsuite
OBJECTS = compile/datetime.o
GTEST_DIR = /usr/src/gtest

$(PROGNAME): compile/libgtest.a $(OBJECTS)
	$(CC) -O -ggdb -o $(PROGNAME) $(CFLAGS) $(OBJECTS) compile/libgtest.a $(LIBS)

all: $(PROGNAME)
	./$(PROGNAME)

clean:
	-rm -rf compile
	-rm *.core

compile/libgtest.a:
	mkdir -p compile
	g++ -I${GTEST_DIR}/include -I${GTEST_DIR} -o compile/gtest-all.o -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv compile/libgtest.a compile/gtest-all.o

compile/datetime.o: src/datetime.cpp Makefile
	mkdir -p compile
	$(CC) $(CFLAGS) -o compile/datetime.o -c src/datetime.cpp