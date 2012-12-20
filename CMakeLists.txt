cmake_minimum_required(VERSION 2.8.8)

project("EDDI Compiler")

#Set the version number
set(VERSION_MAJOR 1)
set(VERSION_MINOR 2)
set(VERSION_PATCH 0)

#Boost is necessary
FIND_PACKAGE(Boost REQUIRED) 
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIRS})

# Activate warnings and C++11 support
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS                 "-pipe -pthread -std=c++11")
    set(CMAKE_EXE_LINKER_FLAGS          "")
    
    set(CMAKE_CXX_FLAGS_DEBUG           "-g -pedantic -Wuninitialized -Wall -Wextra -Wno-long-long -Winit-self -fstack-protector-all")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG    "")
    
    set(CMAKE_CXX_FLAGS_RELEASE         "-g -DLOGGING_DISABLE -DNDEBUG -O3 -fomit-frame-pointer -march=native")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE  "")
    
    set(CMAKE_CXX_FLAGS_COVERAGE        "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS           "-std=c++11 -Qunused-arguments")
    set(CMAKE_CXX_FLAGS_DEBUG     "-g -pedantic -Wuninitialized -Wall -Wextra -Wno-long-long -Winit-self -fsanitize=address")
    set(CMAKE_CXX_FLAGS_RELEASE   "-g -DLOGGING_DISABLE -DNDEBUG -O3 -fomit-frame-pointer -march=native")
else()
    MESSAGE(FATAL_ERROR "ERROR: eddic can only be compiled on gcc and clang for now")
endif()

# Add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile @ONLY)
    add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
        WORKING_DIRECTORY doc
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

# Add a target to inspect the whole code with cppcheck
add_custom_target(cppcheck cppcheck --platform=unix64 --enable=all --std=c++11 -I ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Analysing the code with cppcheck" VERBATIM
)

# Add a target to profile the whole code with oprofile
add_custom_target(oprofile 
    sudo opcontrol --reset
    && sudo opcontrol --event=CPU_CLK_UNHALTED:50000:0:1:1
    && sudo opcontrol --vmlinux=/usr/src/linux/arch/x86/boot/compressed/vmlinux
    && sudo opcontrol --start
    && ${CMAKE_CURRENT_BINARY_DIR}/bin/eddic ${CMAKE_CURRENT_SOURCE_DIR}/eddi_samples/big.eddi
    && sudo opcontrol --stop
    && sudo opcontrol --shutdown
    && sudo opreport -a -e _fini -l ${CMAKE_CURRENT_BINARY_DIR}/bin/eddic > ${CMAKE_CURRENT_BINARY_DIR}/profile
    
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Analysing the code with cppcheck" VERBATIM
)

# Add a custom target to gather stats
add_custom_target(stats bash ${CMAKE_CURRENT_SOURCE_DIR}/tools/stats.sh ${CMAKE_CURRENT_BINARY_DIR}/bin/eddic ${CMAKE_CURRENT_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Gathering stats" VERBATIM
)

# Add a custom target to gather cases
add_custom_target(cases bash ${CMAKE_CURRENT_SOURCE_DIR}/tools/cases.sh ${CMAKE_CURRENT_BINARY_DIR}/bin/eddic ${CMAKE_CURRENT_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling cases" VERBATIM
)

# Add a custom target to gather git stats
add_custom_target(gitstats gitstats ${CMAKE_CURRENT_SOURCE_DIR} stats
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Gathering git stats" VERBATIM
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

# Enable testing
INCLUDE(CTest)

ENABLE_TESTING()

# Create the test executable

file(
	GLOB_RECURSE
	test_files
	test/*
)

include(BoostTestTargets.cmake)
add_boost_test(eddic_boost_test
    SOURCES ${test_files}
    LIBRARIES boost_program_options
    TESTS 
    ApplicationsSuite/applications_hangman
    SamplesSuite/samples_arrays
    SamplesSuite/samples_asm
    SamplesSuite/samples_assembly
    SamplesSuite/samples_bool
    SamplesSuite/samples_compound
    SamplesSuite/samples_concat
    SamplesSuite/samples_const
    SamplesSuite/samples_functions
    SamplesSuite/samples_float
    SamplesSuite/samples_little_float
    SamplesSuite/samples_casts
    SamplesSuite/samples_inc
    SamplesSuite/samples_includes
    SamplesSuite/samples_optimize
    SamplesSuite/samples_problem
    SamplesSuite/samples_sort
    SamplesSuite/samples_identifiers
    SamplesSuite/samples_registers
    SamplesSuite/samples_structures
    SpecificSuite/addressof
    SpecificSuite/array_foreach_local
    SpecificSuite/array_foreach_global
    SpecificSuite/array_foreach_param_local
    SpecificSuite/array_foreach_param_global
    SpecificSuite/array_foreach_param_param
    SpecificSuite/arrays_in_struct
    SpecificSuite/char_type
    SpecificSuite/char_at
    SpecificSuite/ctor_dtor_heap
    SpecificSuite/ctor_dtor_stack
    SpecificSuite/casts
    SpecificSuite/compound
    SpecificSuite/copy_constructors
    SpecificSuite/if_
    SpecificSuite/includes
    SpecificSuite/int_arrays
    SpecificSuite/string_arrays
    SpecificSuite/string_foreach
    SpecificSuite/string_pointers
    SpecificSuite/int_pointers
    SpecificSuite/bool_pointers
    SpecificSuite/cmove
    SpecificSuite/delete_any
    SpecificSuite/dynamic
    SpecificSuite/dynamic_arrays_in_struct
    SpecificSuite/dynamic_struct
    SpecificSuite/float_pointers
    SpecificSuite/struct_pointers
    SpecificSuite/member_pointers
    SpecificSuite/member_function_calls
    SpecificSuite/member_functions
    SpecificSuite/member_functions_param_stack
    SpecificSuite/memory
    SpecificSuite/pass_member_by_value
    SpecificSuite/ternary
    SpecificSuite/while_
    SpecificSuite/do_while_
    SpecificSuite/defaults
    SpecificSuite/float_1
    SpecificSuite/float_2
    SpecificSuite/for_
    SpecificSuite/foreach_
    SpecificSuite/globals_
    SpecificSuite/inc
    SpecificSuite/void_
    SpecificSuite/return_string
    SpecificSuite/return_by_value
    SpecificSuite/return_int
    SpecificSuite/return_pointers
    SpecificSuite/pointer_arrays
    SpecificSuite/recursive_functions
    SpecificSuite/math
    SpecificSuite/builtin
    SpecificSuite/assign_value
    SpecificSuite/concat
    SpecificSuite/println
    SpecificSuite/prints
    SpecificSuite/single_inheritance
    SpecificSuite/structures
    SpecificSuite/struct_member_pointers
    SpecificSuite/struct_array
    SpecificSuite/switch_
    SpecificSuite/switch_string
    SpecificSuite/nested
    SpecificSuite/args
    TemplateSuite/class_templates
    TemplateSuite/function_templates
    TemplateSuite/member_function_templates
    CompilationErrorsSuite/params_assign
    CompilationErrorsSuite/wrong_print
    CompilationErrorsSuite/invalid_inheritance
    StandardLibSuite/std_lib_arrays_sum
    StandardLibSuite/std_lib_math_min
    StandardLibSuite/std_lib_math_max
    StandardLibSuite/std_lib_math_factorial
    StandardLibSuite/std_lib_math_pow
    StandardLibSuite/std_lib_str_equals
    StandardLibSuite/std_linked_list
    BugFixesSuite/while_bug
    #Unit tests
    toString
    toNumber
    bb_entry_exit_count 
    bb_iterators
    bb_new_bb
    bb_remove_bb
    bb_at
    )

target_link_libraries (${eddic_boost_test_TARGET_NAME} boost_program_options)

# Specifications for the installation of eddic

INSTALL(TARGETS eddic
    RUNTIME DESTINATION bin
)
