all:test

CPPFLAGS+=-std=c++0x
CPPFLAGS+=-g -O3
CPPFLAGS+=-isystem ~/custom/boost/
CPPFLAGS+=-I ../../include/

# CPPFLAGS+=-fopenmp
# CPPFLAGS+=-march=native
CPPFLAGS+=-fomit-frame-pointer -march=native

# LDFLAGS+=-L ~/custom/boost/stage/lib/
# LDFLAGS+=-lboost_system -lboost_regex -lboost_thread -lpthread

CXX=/usr/lib/gcc-snapshot/bin/g++
CC=/usr/lib/gcc-snapshot/bin/gcc
CXX=clang++
CC=clang

%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $^ -o $@

test: EDDIGrammar.o SpiritParser.o TypeGrammar.o ValueGrammar.o main.o
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)
