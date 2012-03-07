cmake_minimum_required(VERSION 2.8)

project("EDDI Compiler")

#Set the version number
set(VERSION_MAJOR 0)
set(VERSION_MINOR 9)
set(VERSION_PATCH 0)

#Activate warnings
if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++0x -pedantic -Wall -Wextra -Wno-long-long")
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

target_link_libraries (eddic boost_chrono)
target_link_libraries (eddic boost_system)
target_link_libraries (eddic boost_program_options)

add_executable (test ${test_files})

target_link_libraries (test boost_chrono)
target_link_libraries (test boost_system)
target_link_libraries (test boost_program_options)
target_link_libraries (test boost_unit_test_framework)
