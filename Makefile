default: release

.PHONY: default release debug all clean cppcheck doc

BOOST_PREFIX=$(HOME)/build/modular-boost/

DEBUG_TEST_EXE=debug/bin/test
RELEASE_TEST_EXE=release/bin/test

CC=clang++
LD=clang++

WARNING_FLAGS=-Werror -Wextra -Wall -Qunused-arguments -Wuninitialized -Wsometimes-uninitialized -Wno-long-long -Winit-self -Wdocumentation -pedantic
CXX_FLAGS=-use-gold -Iinclude -std=c++1y -stdlib=libc++ $(WARNING_FLAGS) -isystem $(BOOST_PREFIX)/include
LD_FLAGS=$(CXX_FLAGS) -L $(BOOST_PREFIX)/lib -lboost_program_options
LD_TEST_FLAGS=$(LD_FLAGS) -lboost_unit_test_framework

DEBUG_FLAGS=-g
RELEASE_FLAGS=-g -DLOGGING_DISABLE -DNDEBUG -O3 -flto -march=native -fvectorize -fslp-vectorize-aggressive -fomit-frame-pointer
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

DEBUG_TEST_O_FILES_NON_EXEC=$(TEST_CPP_FILES:%.cpp=debug/%.cpp.o) $(DEBUG_O_FILES_NON_EXEC)
RELEASE_TEST_O_FILES_NON_EXEC=$(TEST_CPP_FILES:%.cpp=release/%.cpp.o) $(RELEASE_O_FILES_NON_EXEC)

CPP_FILES_ALL=$(SRC_CPP_FILES_ALL) $(TEST_CPP_FILES)

DEBUG_D_FILES=$(CPP_FILES_ALL:%.cpp=debug/%.cpp.d)
RELEASE_D_FILES=$(CPP_FILES_ALL:%.cpp=release/%.cpp.d)

# Actual compilation of all the files

debug/src/%.cpp.o: src/%.cpp
	@ mkdir -p debug/src/
	@ $(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > debug/src/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/%.cpp.o: src/%.cpp
	@ mkdir -p release/src/
	@ $(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > release/src/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -o $@ -c $<

debug/src/ast/%.cpp.o: src/ast/%.cpp
	@ mkdir -p debug/src/ast/
	@ $(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > debug/src/ast/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/ast/%.cpp.o: src/ast/%.cpp
	@ mkdir -p release/src/ast/
	@ $(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > release/src/ast/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -o $@ -c $<

debug/src/lexer/%.cpp.o: src/lexer/%.cpp
	@ mkdir -p debug/src/lexer/
	@ $(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > debug/src/lexer/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/lexer/%.cpp.o: src/lexer/%.cpp
	@ mkdir -p release/src/lexer/
	@ $(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > release/src/lexer/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -o $@ -c $<

debug/src/parser/%.cpp.o: src/parser/%.cpp
	@ mkdir -p debug/src/parser/
	@ $(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > debug/src/parser/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/parser/%.cpp.o: src/parser/%.cpp
	@ mkdir -p release/src/parser/
	@ $(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > release/src/parser/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -o $@ -c $<

debug/src/asm/%.cpp.o: src/asm/%.cpp
	@ mkdir -p debug/src/asm/
	@ $(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > debug/src/asm/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/asm/%.cpp.o: src/asm/%.cpp
	@ mkdir -p release/src/asm/
	@ $(CC) $(CXX_FLAGS) $(RELEASE_2_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > release/src/asm/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(RELEASE_2_FLAGS) -o $@ -c $<

debug/src/mtac/%.cpp.o: src/mtac/%.cpp
	@ mkdir -p debug/src/mtac/
	@ $(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > debug/src/mtac/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/mtac/%.cpp.o: src/mtac/%.cpp
	@ mkdir -p release/src/mtac/
	@ $(CC) $(CXX_FLAGS) $(RELEASE_2_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > release/src/mtac/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(RELEASE_2_FLAGS) -o $@ -c $<

debug/src/ltac/%.cpp.o: src/ltac/%.cpp
	@ mkdir -p debug/src/ltac/
	@ $(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > debug/src/ltac/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/ltac/%.cpp.o: src/ltac/%.cpp
	@ mkdir -p release/src/ltac/
	@ $(CC) $(CXX_FLAGS) $(RELEASE_2_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > release/src/ltac/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(RELEASE_2_FLAGS) -o $@ -c $<

debug/test/%.cpp.o: test/%.cpp
	@ mkdir -p debug/test/
	@ $(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > debug/test/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/test/%.cpp.o: test/%.cpp
	@ mkdir -p release/test/
	@ $(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -MM -MT $@ $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > release/test/$*.cpp.d
	$(CC) $(CXX_FLAGS) $(RELEASE_FLAGS) -o $@ -c $<

# Link the various binaries

debug/bin/eddic: debug/src/eddi.cpp.o $(DEBUG_O_FILES_NON_EXEC)
	@ mkdir -p debug/bin/
	$(LD) $(LD_FLAGS) $(DEBUG_FLAGS) -o $@ $+

release/bin/eddic: release/src/eddi.cpp.o $(RELEASE_O_FILES_NON_EXEC)
	@ mkdir -p release/bin/
	$(LD) $(LD_FLAGS) $(RELEASE_FLAGS) -o $@ $+

debug/bin/time_parse: debug/src/parser/main.cpp.o $(DEBUG_O_FILES_NON_EXEC)
	@ mkdir -p debug/bin/
	$(LD) $(LD_FLAGS) $(DEBUG_FLAGS) -o $@ $+

release/bin/time_parse: release/src/parser/main.cpp.o $(RELEASE_O_FILES_NON_EXEC)
	@ mkdir -p release/bin/
	$(LD) $(LD_FLAGS) $(RELEASE_FLAGS) -o $@ $+

debug/bin/generate_lexer: debug/src/lexer/main.cpp.o $(DEBUG_O_FILES_NON_EXEC)
	@ mkdir -p debug/bin/
	$(LD) $(LD_FLAGS) $(DEBUG_FLAGS) -o $@ $+

release/bin/generate_lexer: release/src/lexer/main.cpp.o $(RELEASE_O_FILES_NON_EXEC)
	@ mkdir -p release/bin/
	$(LD) $(LD_FLAGS) $(RELEASE_FLAGS) -o $@ $+

debug/bin/test: $(DEBUG_TEST_O_FILES_NON_EXEC)
	@ mkdir -p debug/bin/
	$(LD) $(LD_TEST_FLAGS) $(DEBUG_FLAGS) -o $@ $+

release/bin/test: $(RELEASE_TEST_O_FILES_NON_EXEC)
	@ mkdir -p release/bin/
	$(LD) $(LD_TEST_FLAGS) $(RELEASE_FLAGS) -o $@ $+

# Management targets

debug: debug/bin/eddic debug/bin/time_parse debug/bin/generate_lexer debug/bin/test
release: release/bin/eddic release/bin/time_parse release/bin/generate_lexer release/bin/test

all: debug release 

clean:
	rm -rf release/
	rm -rf debug/

# Custom targets

update_test_list: $(RELEASE_TEST_EXE)
	./$(RELEASE_TEST_EXE) --log_level=test_suite --log_sink=stdout > tests.tmp.log
	bash tools/generate_tests.sh

cppcheck:
	cppcheck --platform=unix64 --enable=all --std=c++11 -I include src
    
doc:
	doxygen doc/Doxyfile

stats:
	bash tools/stats.sh release/bin/eddic .

cases:
	bash tools/cases.sh release/bin/eddic .

timing:
	bash tools/timing.sh release/bin/eddic .

time_parsing:
	bash tools/time_parsing.sh release/bin/eddic .

gitstats:
	gitstats .

sloccount:
	sloccount .

-include tests.mk

-include $(RELEASE_D_FILES)
-include $(DEBUG_D_FILES)
