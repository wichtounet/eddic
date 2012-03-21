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
#include "Platform.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Eddic Tests
#include <boost/test/unit_test.hpp>

#define TEST_SAMPLE(file)\
BOOST_AUTO_TEST_CASE( samples_##file ){\
    assertCompiles("samples/" #file ".eddi", "-32");\
    assertCompiles("samples/" #file ".eddi", "-64");\
}

void assertCompiles(const std::string& file, const std::string& param){
    const char* options[1] = {param.c_str()};
    eddic::parseOptions(1, options);

    eddic::Compiler compiler;

    eddic::Platform platform = eddic::Platform::INTEL_X86;
    int code = compiler.compileOnly(file, platform);

    BOOST_CHECK_EQUAL (code, 0);
}

void assertOutputEquals(const std::string& file, const std::string& output, const std::string& param){
    assertCompiles("test/cases/" + file, param);

    std::string out = eddic::execCommand("./a.out"); 
    
    BOOST_CHECK_EQUAL (output, out);
}

/* Compiles all the samples */

TEST_SAMPLE(arrays)
TEST_SAMPLE(asm)
TEST_SAMPLE(assembly)
TEST_SAMPLE(bool)
TEST_SAMPLE(compound)
TEST_SAMPLE(concat)
TEST_SAMPLE(const)
TEST_SAMPLE(functions)
TEST_SAMPLE(float)
TEST_SAMPLE(casts)
TEST_SAMPLE(inc)
TEST_SAMPLE(includes)
TEST_SAMPLE(optimize)
TEST_SAMPLE(problem)
TEST_SAMPLE(sort)
TEST_SAMPLE(identifiers)

/* Specific tests */ 

BOOST_AUTO_TEST_CASE( if_ ){
    assertOutputEquals("if.eddi", "Cool", "-32");
    assertOutputEquals("if.eddi", "Cool", "-64");
}

BOOST_AUTO_TEST_CASE( while_ ){
    assertOutputEquals("while.eddi", "01234", "-32");
    assertOutputEquals("while.eddi", "01234", "-64");
}

BOOST_AUTO_TEST_CASE( do_while_ ){
    assertOutputEquals("do_while.eddi", "01234", "-32");
    assertOutputEquals("do_while.eddi", "01234", "-64");
}

BOOST_AUTO_TEST_CASE( for_ ){
    assertOutputEquals("for.eddi", "01234", "-32");
    assertOutputEquals("for.eddi", "01234", "-64");
}

BOOST_AUTO_TEST_CASE( foreach_ ){
    assertOutputEquals("foreach.eddi", "012345", "-32");
    assertOutputEquals("foreach.eddi", "012345", "-64");
}

BOOST_AUTO_TEST_CASE( globals_ ){
    assertOutputEquals("globals.eddi", "1000a2000aa", "-32");
    assertOutputEquals("globals.eddi", "1000a2000aa", "-64");
}

BOOST_AUTO_TEST_CASE( void_functions ){
    assertOutputEquals("void.eddi", "4445", "-32");
    assertOutputEquals("void.eddi", "4445", "-64");
}

BOOST_AUTO_TEST_CASE( string_functions ){
    assertOutputEquals("return_string.eddi", "abcdef", "-32");
    assertOutputEquals("return_string.eddi", "abcdef", "-64");
}

BOOST_AUTO_TEST_CASE( int_functions ){
    assertOutputEquals("return_int.eddi", "484", "-32");
    assertOutputEquals("return_int.eddi", "484", "-64");
}

BOOST_AUTO_TEST_CASE( recursive_functions ){
    assertOutputEquals("recursive.eddi", "362880", "-32");
    assertOutputEquals("recursive.eddi", "362880", "-64");
}

BOOST_AUTO_TEST_CASE( math ){
    assertOutputEquals("math.eddi", "333|111|-111|0|24642|2|-2|-1|1|2|0|-111|", "-32");
    assertOutputEquals("math.eddi", "333|111|-111|0|24642|2|-2|-1|1|2|0|-111|", "-64");
}

BOOST_AUTO_TEST_CASE( builtin ){
    assertOutputEquals("builtin.eddi", "10|11|12|13|12|13|10|11|4|8|13|8|0|3|", "-32");
    assertOutputEquals("builtin.eddi", "10|11|12|13|12|13|10|11|4|8|13|8|0|3|", "-64");
}

BOOST_AUTO_TEST_CASE( assign_value ){
    assertOutputEquals("assign_value.eddi", "66779921", "-32");
    assertOutputEquals("assign_value.eddi", "66779921", "-64");
}

BOOST_AUTO_TEST_CASE( args ){
    assertCompiles("test/cases/args.eddi", "-32");

    std::string out = eddic::execCommand("./a.out"); 
    BOOST_CHECK_EQUAL ("./a.out|", out);
    
    out = eddic::execCommand("./a.out arg1 arg2 arg3"); 
    BOOST_CHECK_EQUAL ("./a.out|arg1|arg2|arg3|", out);
    
    assertCompiles("test/cases/args.eddi", "-64");

    out = eddic::execCommand("./a.out"); 
    BOOST_CHECK_EQUAL ("./a.out|", out);
    
    out = eddic::execCommand("./a.out arg1 arg2 arg3"); 
    BOOST_CHECK_EQUAL ("./a.out|arg1|arg2|arg3|", out);
}
    
/* Unit test for bug fixes regression */
BOOST_AUTO_TEST_CASE( while_bug ){
    assertOutputEquals("while_bug.eddi", "W1W2W3W4W5", "-32");
    assertOutputEquals("while_bug.eddi", "W1W2W3W4W5", "-64");
}
