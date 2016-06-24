default: release

.PHONY: default release debug all clean cppcheck doc

DEBUG_TEST_EXE=debug/bin/test
RELEASE_TEST_EXE=release/bin/test

include make-utils/flags.mk
include make-utils/cpp-utils.mk

CXX_FLAGS += -ftemplate-depth-2048 -use-gold -Iinclude -Icxxopts/src
LD_FLAGS += -lboost_system

# Enable coverage if enabled for the user
ifeq (,$(EDDIC_COVERAGE))
$(eval $(call enable_coverage))
endif

# Compile the sources files

$(eval $(call auto_folder_compile,src))
$(eval $(call auto_folder_compile,src/ast))
$(eval $(call auto_folder_compile,src/lexer))
$(eval $(call auto_folder_compile,src/parser_x3,-fno-rtti))
$(eval $(call auto_folder_compile,src/asm,-fno-rtti -fno-exceptions))
$(eval $(call auto_folder_compile,src/mtac,-fno-rtti -fno-exceptions))
$(eval $(call auto_folder_compile,src/ltac,-fno-rtti -fno-exceptions))
$(eval $(call auto_folder_compile,test))

# Gather files

SRC_CPP_FILES_ALL=$(wildcard src/*.cpp) $(wildcard src/ast/*.cpp) $(wildcard src/asm/*.cpp) $(wildcard src/ltac/*.cpp) $(wildcard src/mtac/*.cpp) $(wildcard src/lexer/*.cpp) $(wildcard src/parser_x3/*.cpp)

# Remove executables
SRC_CPP_FILES_NON_EXEC := $(filter-out src/eddi.cpp,$(SRC_CPP_FILES_ALL))
SRC_CPP_FILES_NON_EXEC := $(filter-out src/lexer/main.cpp,$(SRC_CPP_FILES_NON_EXEC))
SRC_CPP_FILES_NON_EXEC := $(filter-out src/parser_x3/main.cpp,$(SRC_CPP_FILES_NON_EXEC))

TEST_CPP_FILES=$(wildcard test/*.cpp) $(SRC_CPP_FILES_NON_EXEC)

# Link the various binaries

$(eval $(call add_executable,eddic,src/eddi.cpp $(SRC_CPP_FILES_NON_EXEC)))
$(eval $(call add_executable,x3_test,src/parser_x3/main.cpp $(SRC_CPP_FILES_NON_EXEC)))
$(eval $(call add_executable,generate_lexer,src/lexer/main.cpp $(SRC_CPP_FILES_NON_EXEC)))
$(eval $(call add_executable,test,$(TEST_CPP_FILES), -lboost_unit_test_framework))

# Management targets

debug: debug/bin/eddic debug/bin/test
release: release/bin/eddic release/bin/test

all: debug release

# Custom targets

update_test_list: release/bin/test
	./release/bin/test --log_level=test_suite --log_sink=stdout > tests.tmp.log
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

clean: base_clean

include make-utils/cpp-utils-finalize.mk
