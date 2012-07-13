cmake_minimum_required(VERSION 2.8.8)

project("EDDI Compiler")

#Set the version number
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 3)

# Activate warnings and C++11 support
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS           "-pipe -pthread -std=c++11 -g -pedantic -Wall -Wextra -Wno-long-long")
    set(CMAKE_CXX_FLAGS_DEBUG     "-pipe -pthread -std=c++11 -g -pedantic -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wsuggest-attribute=const -Wall -Wextra -Wno-long-long")
    set(CMAKE_CXX_FLAGS_RELEASE   "-pipe -pthread -std=c++11 -DNDEBUG -O2 -march=native")
else()
    MESSAGE(FATAL_ERROR "ERROR: eddic cannot be compiled on another compiler than gcc for now")
endif()

# Add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in doc/Doxyfile @ONLY)
    add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile
        WORKING_DIRECTORY doc
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

# Add a target to inspect the whole code with cppcheck
add_custom_target(cppcheck cppcheck --platform=unix64 --enable=all --std=c++11 -I ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Analysing the code with cppcheck" VERBATIM
)

# Add a custom target to gather stats
add_custom_target(stats bash ${CMAKE_CURRENT_SOURCE_DIR}/tools/stats.sh ${CMAKE_CURRENT_BINARY_DIR}/bin/eddic ${CMAKE_CURRENT_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Gathering stats" VERBATIM
)
 
# The compiler and the tests should go to the bin directory
set(EXECUTABLE_OUTPUT_PATH bin)

# All the headers are in the include directory
include_directories(include)

# Create a pseudo library grouping all object files

file(
	GLOB_RECURSE
	compiler_files
	src/*.cpp
)

file(GLOB to_remove src/eddi.cpp)
list(REMOVE_ITEM compiler_files ${to_remove})

add_library(Compiler OBJECT ${compiler_files})

# Create the eddic executable

add_executable(eddic $<TARGET_OBJECTS:Compiler> src/eddi.cpp)

target_link_libraries (eddic boost_program_options)

# Create the test executable

file(
	GLOB_RECURSE
	test_files
	test/*
)

add_executable(eddic_test $<TARGET_OBJECTS:Compiler> ${test_files})

target_link_libraries (eddic_test boost_program_options)
target_link_libraries (eddic_test boost_unit_test_framework)

# Specifications for the installation of eddic

INSTALL(TARGETS eddic
    RUNTIME DESTINATION bin
)

# Enable and configure testing
INCLUDE(CTest)

ENABLE_TESTING()

ADD_TEST(
    NAME integration_tests 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND eddic_test --show_progress)
