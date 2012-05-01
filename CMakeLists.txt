cmake_minimum_required(VERSION 2.8)

project("EDDI Compiler")

#Set the version number
set(VERSION_MAJOR 0)
set(VERSION_MINOR 9)
set(VERSION_PATCH 1)

#Activate warnings and C++11 support
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS           "-pipe -g -std=c++11 -pedantic -Wall -Wextra -Wno-long-long")
    set(CMAKE_CXX_FLAGS_DEBUG     "-pipe -g -std=c++11 -pedantic -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wsuggest-attribute=const -Wall -Wextra -Wno-long-long")
    set(CMAKE_CXX_FLAGS_RELEASE   "-pipe -DNDEBUG -O2 -march=native -std=c++11")
else()
    MESSAGE(FATAL_ERROR "ERROR: eddic cannot be compiled on another compiler than gcc for now")
endif()

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in doc/Doxyfile @ONLY)
    add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile
        WORKING_DIRECTORY doc
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)
 
set(EXECUTABLE_OUTPUT_PATH bin)

include_directories(include)

file(
	GLOB_RECURSE
	source_files
	src/*
)

file(
	GLOB_RECURSE
	test_files
	test/*
    src/*
)

file(GLOB to_remove src/eddi.cpp)
list(REMOVE_ITEM test_files ${to_remove})

add_executable(eddic ${source_files})

target_link_libraries (eddic boost_program_options)

add_executable (test ${test_files})

target_link_libraries (test boost_program_options)
target_link_libraries (test boost_unit_test_framework)
