default: release

.PHONY: default release debug all clean

TEST_EXE=bin/boosttest--eddic_boost_test

CC=clang++
LD=clang++

WARNING_FLAGS=-Wextra -Wall -Qunused-arguments -Wuninitialized -Wsometimes-uninitialized -Wno-long-long -Winit-self -Wdocumentation -pedantic
CXX_FLAGS=-Iinclude -std=c++1y -stdlib=libc++ $(WARNING_FLAGS)
LD_FLAGS=$(CXX_FLAGS)

DEBUG_FLAGS=-g
RELEASE_FLAGS=-g -DLOGGING_DISABLE -DNDEBUG -Ofast -march=native -fvectorize -fslp-vectorize-aggressive -fomit-frame-pointer
RELEASE_2_FLAGS=$(RELEASE_FLAGS) -fno-exceptions -fno-rtti

# Search the source files

CPP_FILES=$(wildcard src/*.cpp)
CPP_FILES_AST=$(wildcard src/ast/*.cpp)
CPP_FILES_ASM=$(wildcard src/asm/*.cpp)
CPP_FILES_LTAC=$(wildcard src/ltac/*.cpp)
CPP_FILES_MTAC=$(wildcard src/mtac/*.cpp)
CPP_FILES_LEXER=$(wildcard src/lexer/*.cpp)
CPP_FILES_PARSER=$(wildcard src/parser/*.cpp)

CPP_FILES_ALL=$(CPP_FILES) $(CPP_FILES_AST) $(CPP_FILES_ASM) $(CPP_FILES_LTAC) $(CPP_FILES_MTAC) $(CPP_FILES_LEXER) $(CPP_FILES_PARSER)

CPP_FILES_NON_EXEC := $(filter-out src/eddi.cpp,$(CPP_FILES_ALL))
CPP_FILES_NON_EXEC := $(filter-out src/parser/main.cpp,$(CPP_FILES_NON_EXEC))
CPP_FILES_NON_EXEC := $(filter-out src/lexer/main.cpp,$(CPP_FILES_NON_EXEC))

DEBUG_O_FILES_NON_EXEC=$(NONEXEC_CPP_FILES:%.cpp=debug/%.cpp.o)
RELEASE_O_FILES_NON_EXEC=$(NONEXEC_CPP_FILES:%.cpp=release/%.cpp.o)

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

debug/src/paser/%.cpp.o: src/parser/%.cpp
	@ mkdir -p debug/src/parser/
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/parser/%.cpp.o: src/parser/%.cpp
	@ mkdir -p release/src/parser/
	$(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -o $@ -c $<

debug/src/asm/%.cpp.o: src/asm/%.cpp
	@ mkdir -p debug/src/parser/
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/asm/%.cpp.o: src/asm/%.cpp
	@ mkdir -p release/src/asm/
	$(CC) $(CXX_FLAGS) $(RELEASE_2_FLAGS) -o $@ -c $<

debug/src/ltac/%.cpp.o: src/ltac/%.cpp
	@ mkdir -p debug/src/ltac/
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/mtac/%.cpp.o: src/mtac/%.cpp
	@ mkdir -p release/src/mtac/
	$(CC) $(CXX_FLAGS) $(RELEASE_2_FLAGS) -o $@ -c $<

# Link the various binaries

# TODO

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