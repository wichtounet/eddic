default: release

.PHONY: default release debug all clean

BOOST_PREFIX=$(HOME)/build/modular-boost/

TEST_EXE=bin/boosttest--eddic_boost_test

CC=clang++
LD=clang++

WARNING_FLAGS=-Wextra -Wall -Qunused-arguments -Wuninitialized -Wsometimes-uninitialized -Wno-long-long -Winit-self -Wdocumentation -pedantic
CXX_FLAGS=-Iinclude -std=c++1y -stdlib=libc++ $(WARNING_FLAGS) -isystem $(BOOST_PREFIX)/include
LD_FLAGS=$(CXX_FLAGS) -L $(BOOST_PREFIX)/lib -lboost_program_options

DEBUG_FLAGS=-g
RELEASE_FLAGS=-g -DLOGGING_DISABLE -DNDEBUG -Ofast -march=native -fvectorize -fslp-vectorize-aggressive -fomit-frame-pointer
RELEASE_2_FLAGS=$(RELEASE_FLAGS) -fno-exceptions -fno-rtti

# Search the source files

SRC_CPP_FILES=$(wildcard src/*.cpp)
SRC_CPP_FILES_AST=$(wildcard src/ast/*.cpp)
SRC_CPP_FILES_ASM=$(wildcard src/asm/*.cpp)
SRC_CPP_FILES_LTAC=$(wildcard src/ltac/*.cpp)
SRC_CPP_FILES_MTAC=$(wildcard src/mtac/*.cpp)
SRC_CPP_FILES_LEXER=$(wildcard src/lexer/*.cpp)
SRC_CPP_FILES_PARSER=$(wildcard src/parser/*.cpp)

SRC_CPP_FILES_ALL=$(SRC_CPP_FILES) $(SRC_CPP_FILES_AST) $(SRC_CPP_FILES_ASM) $(SRC_CPP_FILES_LTAC) $(SRC_CPP_FILES_MTAC) $(SRC_CPP_FILES_LEXER) $(SRC_CPP_FILES_PARSER)

SRC_CPP_FILES_NON_EXEC := $(filter-out src/eddi.cpp,$(SRC_CPP_FILES_ALL))
SRC_CPP_FILES_NON_EXEC := $(filter-out src/parser/main.cpp,$(SRC_CPP_FILES_NON_EXEC))
SRC_CPP_FILES_NON_EXEC := $(filter-out src/lexer/main.cpp,$(SRC_CPP_FILES_NON_EXEC))

DEBUG_O_FILES_NON_EXEC=$(SRC_CPP_FILES_NON_EXEC:%.cpp=debug/%.cpp.o)
RELEASE_O_FILES_NON_EXEC=$(SRC_CPP_FILES_NON_EXEC:%.cpp=release/%.cpp.o)

TEST_CPP_FILES=$(wildcard test/*.cpp)

DEBUG_TEST_O_FILES_NON_EXEC=$(TEST_CPP_FILES:%.cpp=debug/%.cpp.o)
RELEASE_TEST_O_FILES_NON_EXEC=$(TEST_CPP_FILES:%.cpp=release/%.cpp.o)

# Actual compilation of all the files

debug/src/%.cpp.o: src/%.cpp
	@ mkdir -p debug/src/
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/%.cpp.o: src/%.cpp
	@ mkdir -p release/src/
	$(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -o $@ -c $<

debug/src/ast/%.cpp.o: src/ast/%.cpp
	@ mkdir -p debug/src/ast/
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/ast/%.cpp.o: src/ast/%.cpp
	@ mkdir -p release/src/ast/
	$(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -o $@ -c $<

debug/src/lexer/%.cpp.o: src/lexer/%.cpp
	@ mkdir -p debug/src/lexer/
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/lexer/%.cpp.o: src/lexer/%.cpp
	@ mkdir -p release/src/lexer/
	$(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -o $@ -c $<

debug/src/parser/%.cpp.o: src/parser/%.cpp
	@ mkdir -p debug/src/parser/
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/parser/%.cpp.o: src/parser/%.cpp
	@ mkdir -p release/src/parser/
	$(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -o $@ -c $<

debug/src/asm/%.cpp.o: src/asm/%.cpp
	@ mkdir -p debug/src/asm/
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/asm/%.cpp.o: src/asm/%.cpp
	@ mkdir -p release/src/asm/
	$(CC) $(CXX_FLAGS) $(RELEASE_2_FLAGS) -o $@ -c $<

debug/src/mtac/%.cpp.o: src/mtac/%.cpp
	@ mkdir -p debug/src/mtac/
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/mtac/%.cpp.o: src/mtac/%.cpp
	@ mkdir -p release/src/mtac/
	$(CC) $(CXX_FLAGS) $(RELEASE_2_FLAGS) -o $@ -c $<

debug/src/ltac/%.cpp.o: src/ltac/%.cpp
	@ mkdir -p debug/src/ltac/
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/ltac/%.cpp.o: src/ltac/%.cpp
	@ mkdir -p release/src/ltac/
	$(CC) $(CXX_FLAGS) $(RELEASE_2_FLAGS) -o $@ -c $<

debug/test/%.cpp.o: test/%.cpp
	@ mkdir -p debug/test/
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/test/%.cpp.o: test/%.cpp
	@ mkdir -p release/test/
	$(CC) $(CXX_FLAGS) $(RELEASE_2_FLAGS) -o $@ -c $<

# Link the various binaries

debug/bin/eddic: debug/src/eddi.cpp.o $(DEBUG_O_FILES_NON_EXEC)
	@ mkdir -p debug/bin/
	$(LD) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ $? $(LD_FLAGS)

release/bin/eddic: release/src/eddi.cpp.o $(RELEASE_O_FILES_NON_EXEC)
	@ mkdir -p release/bin/
	$(LD) $(LD_FLAGS) $(RELEASE_FLAGS) -o $@ $?

debug/bin/time_parse: debug/src/parser/main.cpp.o $(DEBUG_O_FILES_NON_EXEC)
	@ mkdir -p debug/bin/
	$(LD) $(LD_FLAGS) $(DEBUG_FLAGS) -o $@ $?

release/bin/time_parse: release/src/parser/main.cpp.o $(RELEASE_O_FILES_NON_EXEC)
	@ mkdir -p release/bin/
	$(LD) $(LD_FLAGS) $(RELEASE_FLAGS) -o $@ $?

debug/bin/generate_lexer: debug/src/lexer/main.cpp.o $(DEBUG_O_FILES_NON_EXEC)
	@ mkdir -p debug/bin/
	$(LD) $(LD_FLAGS) $(DEBUG_FLAGS) -o $@ $?

release/bin/generate_lexer: release/src/lexer/main.cpp.o $(RELEASE_O_FILES_NON_EXEC)
	@ mkdir -p release/bin/
	$(LD) $(LD_FLAGS) $(RELEASE_FLAGS) -o $@ $?

# Generate the dependency files

# TODO

# Management targets

debug: debug/bin/eddic debug/bin/time_parse debug/bin/generate_lexer
release: release/bin/eddic release/bin/time_parse release/bin/generate_lexer

all: debug release 

clean:
	rm -rf release/
	rm -rf debug/

# Custom targets

update_test_list: bin/boosttest--eddic_boost_test
	./$(TEST_EXE) --log_level=test_suite --log_sink=stdout > tests.tmp.log
	bash tools/generate_tests.sh

-include tests.mk