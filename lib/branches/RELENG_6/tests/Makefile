GTEST_DIR = /usr/local/gtest-1.6.0
CC		= gcc
CXX			= g++
EXTRA_CFLAGS = -DPPL6LIB
INCLUDE	= -I. -I../include -I$(GTEST_DIR)/include
CFLAGS	= -ggdb -Wall $(INCLUDE) `../ppl6-config --cflags release source` $(EXTRA_CFLAGS)
LIBS	= `../ppl6-config --libs debug source` -lstdc++
PROGNAME = testsuite
OBJECTS = compile/wordlist.o \
	compile/datetime.o \
	compile/main.o \
	compile/strings.o \
	compile/widestrings.o \
	compile/assocarray.o
	
	
	


$(PROGNAME): compile/libgtest.a $(OBJECTS)
	$(CXX) -O -ggdb -o $(PROGNAME) $(CFLAGS) $(OBJECTS) compile/libgtest.a $(LIBS)

all: $(PROGNAME)
	./$(PROGNAME)

test: $(PROGNAME)
	./$(PROGNAME)
	
xml: $(PROGNAME)
	./$(PROGNAME) --gtest_output=xml
	
clean:
	-rm -rf compile loggertest
	-rm -f *.core

memcheck: $(PROGNAME)
	valgrind --tool=memcheck --leak-check=full --show-reachable=no --suppressions=valgrind.suppressions --xml=yes --xml-file=testsuite.valgrind ./$(PROGNAME)


valgrind: $(PROGNAME)
	valgrind --tool=memcheck --leak-check=full --track-origins=yes --suppressions=valgrind.suppressions ./$(PROGNAME) ;#--gtest_filter=StringTest.*


compile/libgtest.a:
	mkdir -p compile
	$(CXX) -I${GTEST_DIR}/include -I${GTEST_DIR} -o compile/gtest-all.o -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv compile/libgtest.a compile/gtest-all.o

loggertest: src/loggertest.cpp ../release/libppl6.a
	mkdir -p compile
	$(CXX) -O -ggdb -o loggertest $(CFLAGS) src/loggertest.cpp $(LIBS)

compile/main.o: src/main.cpp Makefile
	mkdir -p compile
	$(CXX) $(CFLAGS) -o compile/main.o -c src/main.cpp
	
compile/datetime.o: src/datetime.cpp Makefile
	mkdir -p compile
	$(CXX) $(CFLAGS) -o compile/datetime.o -c src/datetime.cpp
	
compile/widestrings.o: src/widestrings.cpp Makefile
	mkdir -p compile
	$(CXX) $(CFLAGS) -o compile/widestrings.o -c src/widestrings.cpp

compile/strings.o: src/strings.cpp Makefile
	mkdir -p compile
	$(CXX) $(CFLAGS) -o compile/strings.o -c src/strings.cpp

compile/assocarray.o: src/assocarray.cpp Makefile
	mkdir -p compile
	$(CXX) $(CFLAGS) -o compile/assocarray.o -c src/assocarray.cpp

	
compile/wordlist.o: src/wordlist.cpp Makefile
	mkdir -p compile
	$(CXX) $(CFLAGS) -o compile/wordlist.o -c src/wordlist.cpp