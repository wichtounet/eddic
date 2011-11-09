cmake_minimum_required(VERSION 2.8)

project("EDDI Compiler")

#Set the version number
set(VERSION_MAJOR 0)
set(VERSION_MINOR 5)
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
)

add_executable(eddic ${source_files})

target_link_libraries (eddic boost_chrono)
target_link_libraries (eddic boost_system)
target_link_libraries (eddic boost_program_options)

add_executable (test ${test_files})

target_link_libraries (test boost_unit_test_framework)
