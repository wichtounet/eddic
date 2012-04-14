//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <iostream>

#include "Options.hpp"
#include "Compiler.hpp"
#include "Utils.hpp"
#include "Platform.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE EddicTestSuites
#include <boost/test/unit_test.hpp>

/* Macros to ease the tests  */

#define TEST_SAMPLE(file)\
BOOST_AUTO_TEST_CASE( samples_##file ){\
    assertCompiles("samples/" #file ".eddi", "--32");\
    assertCompiles("samples/" #file ".eddi", "--64");\
}

#define ASSERT_OUTPUT(file, output)\
    assertOutputEquals(file, output, "--32");\
    assertOutputEquals(file, output, "--64");

#include <boost/test/detail/unit_test_parameters.hpp>

/* Config Fixture  */

struct ConfigFixture {
    ConfigFixture(){
        //TODO Configure the show progress parameter
    }

    ~ConfigFixture(){
        /* Nothing to teard down */
    }
};

/* Fixture to delete the a.out file after the compilation */

struct DeleteOutFixture {
    DeleteOutFixture(){
        /* Nothing to setup  */    
    }

    ~DeleteOutFixture(){ 
        BOOST_TEST_MESSAGE( "Delete the a.out file" ); 
        remove("a.out"); 
    }
};

void assertCompiles(const std::string& file, const std::string& param){
    const char* argv[4];
    argv[0] = "./bin/test";
    argv[1] = param.c_str();
    argv[2] = "--quiet";
    argv[3] = file.c_str();

    BOOST_REQUIRE (eddic::parseOptions(4, argv));

    eddic::Compiler compiler;
    int code = compiler.compile(file);

    BOOST_REQUIRE_EQUAL (code, 0);
}

void assertOutputEquals(const std::string& file, const std::string& output, const std::string& param){
    assertCompiles("test/cases/" + file, param);

    std::string out = eddic::execCommand("./a.out"); 
    
    BOOST_CHECK_EQUAL (output, out);
}

/* Configure a global fixture for the configuration */

BOOST_GLOBAL_FIXTURE ( ConfigFixture  )

/* Compiles all the samples */

BOOST_FIXTURE_TEST_SUITE( SamplesSuite, DeleteOutFixture )

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
TEST_SAMPLE(structures)

BOOST_AUTO_TEST_SUITE_END()

/* Specific tests */ 

BOOST_FIXTURE_TEST_SUITE(SpecificSuite, DeleteOutFixture)

BOOST_AUTO_TEST_CASE( array_foreach_local ){
    ASSERT_OUTPUT("array_foreach_local.eddi", "43210");
}

BOOST_AUTO_TEST_CASE( array_foreach_global ){
    ASSERT_OUTPUT("array_foreach_global.eddi", "43210");
}

BOOST_AUTO_TEST_CASE( array_foreach_param_local ){
    ASSERT_OUTPUT("array_foreach_param_local.eddi", "43210");
}

BOOST_AUTO_TEST_CASE( array_foreach_param_global ){
    ASSERT_OUTPUT("array_foreach_param_global.eddi", "43210");
}

BOOST_AUTO_TEST_CASE( array_foreach_param_param ){
    ASSERT_OUTPUT("array_foreach_param_param.eddi", "43210");
}

BOOST_AUTO_TEST_CASE( if_ ){
    ASSERT_OUTPUT("if.eddi", "Cool");
}

BOOST_AUTO_TEST_CASE( while_ ){
    ASSERT_OUTPUT("while.eddi", "01234");
}

BOOST_AUTO_TEST_CASE( do_while_ ){
    ASSERT_OUTPUT("do_while.eddi", "01234");
}

BOOST_AUTO_TEST_CASE( for_ ){
    ASSERT_OUTPUT("for.eddi", "01234");
}

BOOST_AUTO_TEST_CASE( foreach_ ){
    ASSERT_OUTPUT("foreach.eddi", "012345");
}

BOOST_AUTO_TEST_CASE( globals_ ){
    ASSERT_OUTPUT("globals.eddi", "1000a2000aa");
}

BOOST_AUTO_TEST_CASE( void_functions ){
    ASSERT_OUTPUT("void.eddi", "4445");
}

BOOST_AUTO_TEST_CASE( string_functions ){
    ASSERT_OUTPUT("return_string.eddi", "abcdef");
}

BOOST_AUTO_TEST_CASE( int_functions ){
    ASSERT_OUTPUT("return_int.eddi", "484");
}

BOOST_AUTO_TEST_CASE( recursive_functions ){
    ASSERT_OUTPUT("recursive.eddi", "362880");
}

BOOST_AUTO_TEST_CASE( math ){
    ASSERT_OUTPUT("math.eddi", "333|111|-111|0|24642|2|-2|-1|1|2|0|-111|");
}

BOOST_AUTO_TEST_CASE( builtin ){
    ASSERT_OUTPUT("builtin.eddi", "10|11|12|13|12|13|10|11|4|8|13|8|0|3|");
}

BOOST_AUTO_TEST_CASE( assign_value ){
    ASSERT_OUTPUT("assign_value.eddi", "66779921");
}

BOOST_AUTO_TEST_CASE( concat ){
    ASSERT_OUTPUT("concat.eddi", "asdf1234|1234asfd|asdfasdf|12341234|");
}

BOOST_AUTO_TEST_CASE( args ){
    assertCompiles("test/cases/args.eddi", "--32");

    std::string out = eddic::execCommand("./a.out"); 
    BOOST_CHECK_EQUAL ("./a.out|", out);
    
    out = eddic::execCommand("./a.out arg1 arg2 arg3"); 
    BOOST_CHECK_EQUAL ("./a.out|arg1|arg2|arg3|", out);
    
    assertCompiles("test/cases/args.eddi", "--64");

    out = eddic::execCommand("./a.out"); 
    BOOST_CHECK_EQUAL ("./a.out|", out);
    
    out = eddic::execCommand("./a.out arg1 arg2 arg3"); 
    BOOST_CHECK_EQUAL ("./a.out|arg1|arg2|arg3|", out);
}

BOOST_AUTO_TEST_SUITE_END()

/* Standard library test suite  */

BOOST_FIXTURE_TEST_SUITE(StandardLibSuite, DeleteOutFixture)

BOOST_AUTO_TEST_CASE( std_lib_arrays_sum ){
    ASSERT_OUTPUT("stdlib_array_sum.eddi", "100");
}

BOOST_AUTO_TEST_CASE( std_lib_math_min ){
    ASSERT_OUTPUT("stdlib_math_min.eddi", "999|0|0|-1|0|-1");
}

BOOST_AUTO_TEST_CASE( std_lib_math_max ){
    ASSERT_OUTPUT("stdlib_math_max.eddi", "1000|1|1|0|0|0");
}

BOOST_AUTO_TEST_CASE( std_lib_math_factorial ){
    ASSERT_OUTPUT("stdlib_math_factorial.eddi", "1|1|2|362880");
}

BOOST_AUTO_TEST_CASE( std_lib_math_pow ){
    ASSERT_OUTPUT("stdlib_math_pow.eddi", "0|1|10|100|1024|1");
}

BOOST_AUTO_TEST_SUITE_END()
    
/* Unit test for bug fixes regression */

BOOST_FIXTURE_TEST_SUITE(BugFixesSuite, DeleteOutFixture)

BOOST_AUTO_TEST_CASE( while_bug ){
    ASSERT_OUTPUT("while_bug.eddi", "W1W2W3W4W5");
}

BOOST_AUTO_TEST_SUITE_END()
