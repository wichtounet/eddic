//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>

#include "Options.hpp"
#include "Compiler.hpp"
#include "Utils.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Eddic Tests
#include <boost/test/unit_test.hpp>

void assertCompiles(const std::string& file){
    eddic::parseOptions(0, {});

    eddic::Compiler compiler;

    int code = compiler.compileOnly(file);

    BOOST_CHECK_EQUAL (code, 0);
}

void assertOutputEquals(const std::string& file, const std::string& output){
    assertCompiles("test/cases/" + file);

    std::string out = eddic::execCommand("./a.out"); 
    
    BOOST_CHECK_EQUAL (output, out);
}

/* Compiles all the samples */

BOOST_AUTO_TEST_CASE( samples_arrays ){
    assertCompiles("samples/arrays.eddi");
}

BOOST_AUTO_TEST_CASE( samples_asm ){
    assertCompiles("samples/asm.eddi");
}

BOOST_AUTO_TEST_CASE( samples_bool ){
    assertCompiles("samples/bool.eddi");
}

BOOST_AUTO_TEST_CASE( samples_assembly ){
    assertCompiles("samples/assembly.eddi");
}

BOOST_AUTO_TEST_CASE( samples_const ){
    assertCompiles("samples/const.eddi");
}

BOOST_AUTO_TEST_CASE( samples_concat ){
    assertCompiles("samples/concat.eddi");
}

BOOST_AUTO_TEST_CASE( samples_functions ){
    assertCompiles("samples/functions.eddi");
}

BOOST_AUTO_TEST_CASE( samples_includes ){
    assertCompiles("samples/includes.eddi");
}

BOOST_AUTO_TEST_CASE( samples_optimize ){
    assertCompiles("samples/optimize.eddi");
}

BOOST_AUTO_TEST_CASE( samples_problem ){
    assertCompiles("samples/problem.eddi");
}

BOOST_AUTO_TEST_CASE( samples_sort ){
    assertCompiles("samples/sort.eddi");
}

/* Specific tests */ 

BOOST_AUTO_TEST_CASE( if_ ){
    assertOutputEquals("if.eddi", "Cool");
}

BOOST_AUTO_TEST_CASE( while_ ){
    assertOutputEquals("while.eddi", "01234");
}

BOOST_AUTO_TEST_CASE( for_ ){
    assertOutputEquals("for.eddi", "01234");
}

BOOST_AUTO_TEST_CASE( foreach_ ){
    assertOutputEquals("foreach.eddi", "012345");
}

BOOST_AUTO_TEST_CASE( globals_ ){
    assertOutputEquals("globals.eddi", "1000a2000aa");
}

BOOST_AUTO_TEST_CASE( void_functions ){
    assertOutputEquals("void.eddi", "4445");
}

BOOST_AUTO_TEST_CASE( string_functions ){
    assertOutputEquals("return_string.eddi", "abcdef");
}

BOOST_AUTO_TEST_CASE( int_functions ){
    assertOutputEquals("return_int.eddi", "484");
}

BOOST_AUTO_TEST_CASE( recursive_functions ){
    assertOutputEquals("recursive.eddi", "362880");
}
