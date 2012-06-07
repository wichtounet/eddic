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
#include <boost/test/detail/unit_test_parameters.hpp>

/*
 * \def TEST_SAMPLE(file) 
 * Generate a test case that verify that the sample compiles in both 32 and 64 bits mode. 
 */
#define TEST_SAMPLE(file)\
BOOST_AUTO_TEST_CASE( samples_##file ){\
    assertCompiles("samples/" #file ".eddi", "--32");\
    assertCompiles("samples/" #file ".eddi", "--64");\
}

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

inline void parse_options(const std::string& file, const std::string& param){
    const char* argv[4];
    argv[0] = "./bin/test";
    argv[1] = param.c_str();
    argv[2] = "--quiet";
    argv[3] = file.c_str();

    BOOST_REQUIRE (eddic::parseOptions(4, argv));
}

void assertCompiles(const std::string& file, const std::string& param){
    parse_options(file, param);

    eddic::Compiler compiler;
    int code = compiler.compile(file);

    BOOST_REQUIRE_EQUAL (code, 0);
}

void assert_compilation_error(const std::string& file, const std::string& param){
    parse_options("test/cases/" + file, param);

    eddic::Compiler compiler;
    int code = compiler.compile("test/cases/" + file);

    BOOST_REQUIRE_EQUAL (code, 1);
}

void assertOutputEquals(const std::string& file, const std::string& output, const std::string& param){
    assertCompiles("test/cases/" + file, param);

    std::string out = eddic::execCommand("./a.out"); 
    
    BOOST_CHECK_EQUAL (output, out);
}

void assert_output_32(const std::string& file, const std::string& output){
    assertOutputEquals(file, output, "--32");
}

void assert_output_64(const std::string& file, const std::string& output){
    assertOutputEquals(file, output, "--64");
}

void assert_output(const std::string& file, const std::string& output){
    assert_output_32(file, output);
    assert_output_64(file, output);
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
TEST_SAMPLE(little_float)
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
    assert_output("array_foreach_local.eddi", "43210");
}

BOOST_AUTO_TEST_CASE( array_foreach_global ){
    assert_output("array_foreach_global.eddi", "43210");
}

BOOST_AUTO_TEST_CASE( array_foreach_param_local ){
    assert_output("array_foreach_param_local.eddi", "43210");
}

BOOST_AUTO_TEST_CASE( array_foreach_param_global ){
    assert_output("array_foreach_param_global.eddi", "43210");
}

BOOST_AUTO_TEST_CASE( array_foreach_param_param ){
    assert_output("array_foreach_param_param.eddi", "43210");
}

BOOST_AUTO_TEST_CASE( casts ){
    assert_output_32("casts.eddi", "5.0|5|4|333|5.0|8.3299|");
    assert_output_64("casts.eddi", "5.0|5|4|333|5.0|8.3300|");
}

BOOST_AUTO_TEST_CASE( compound ){
    assert_output("compound.eddi", "6|9|6|18|6|0|");
}

BOOST_AUTO_TEST_CASE( if_ ){
    assert_output("if.eddi", "Cool");
}

BOOST_AUTO_TEST_CASE( includes ){
    assert_output("includes.eddi", "45");
}

BOOST_AUTO_TEST_CASE( int_arrays ){
    assert_output("int_arrays.eddi", "1|1|1|0|0|0|0|0|2|2|0|0|0|0|0|4|9|4|1|9|9|0|0|0|4|9|4|2|9|9|0|0|0|");
}

BOOST_AUTO_TEST_CASE( string_arrays ){
    assert_output("string_arrays.eddi", "5|6|7|7|5|6|7|7||||a|a|a|a|a||||||2|2|2|7|7||||4|9|4|a|9|9||||4|9|4|2|9|9||||");
}

BOOST_AUTO_TEST_CASE( while_ ){
    assert_output("while.eddi", "01234");
}

BOOST_AUTO_TEST_CASE( do_while_ ){
    assert_output("do_while.eddi", "01234");
}

BOOST_AUTO_TEST_CASE( float_ ){
    //TODO Could be better to split this test
    assert_output_32("float.eddi", "5.4990|100.0|-100.0|100.0|2.0889|4.1999|3.3299|1.5000|3.0|5.0|4.5000|5.7500|1.5000|-2.0|7.5000|2.2699|7.5590|14.4927|3.0|8.0|3.0910|2.0934|5.1844|1|1|11111|8.0|13.7500|2.5000|5.5000|2.5000|5.5000|2.5000|5.5000|2.5000|5.5000|3.3299|");
    assert_output_64("float.eddi", "5.4989|100.0|-100.0|100.0|2.0889|4.2000|3.3300|1.5000|3.0|5.0|4.5000|5.7500|1.5000|-2.0|7.5000|2.2700|7.5590|14.4927|3.0|8.0|3.0910|2.0934|5.1844|1|1|11111|8.0|13.7500|2.5000|5.5000|2.5000|5.5000|2.5000|5.5000|2.5000|5.5000|3.3300|");
}

BOOST_AUTO_TEST_CASE( for_ ){
    assert_output("for.eddi", "01234");
}

BOOST_AUTO_TEST_CASE( foreach_ ){
    assert_output("foreach.eddi", "012345");
}

BOOST_AUTO_TEST_CASE( globals_ ){
    assert_output("globals.eddi", "1000a2000aa");
}

BOOST_AUTO_TEST_CASE( inc ){
    assert_output("inc.eddi", "0|1|2|1|0|1|2|");
}

BOOST_AUTO_TEST_CASE( void_functions ){
    assert_output("void.eddi", "4445");
}

BOOST_AUTO_TEST_CASE( string_functions ){
    assert_output("return_string.eddi", "abcdef");
}

BOOST_AUTO_TEST_CASE( int_functions ){
    assert_output("return_int.eddi", "484");
}

BOOST_AUTO_TEST_CASE( recursive_functions ){
    assert_output("recursive.eddi", "362880");
}

BOOST_AUTO_TEST_CASE( math ){
    assert_output("math.eddi", "333|111|-111|0|24642|2|-2|-1|1|2|0|-111|");
}

BOOST_AUTO_TEST_CASE( builtin ){
    assert_output("builtin.eddi", "10|11|12|13|12|13|10|11|4|8|13|8|0|3|");
}

BOOST_AUTO_TEST_CASE( assign_value ){
    assert_output("assign_value.eddi", "66779921");
}

BOOST_AUTO_TEST_CASE( concat ){
    assert_output("concat.eddi", "asdf1234|1234asdf|asdfasdf|12341234|");
}

BOOST_AUTO_TEST_CASE( prints ){
    assert_output_32("prints.eddi", "111|0|-111|0|1|999.9899|1.0089|0.0|-1.0089|-999.9899||-0|asdf|1234asdf|");
    assert_output_64("prints.eddi", "111|0|-111|0|1|999.9900|1.0089|0.0|-1.0089|-999.9900||-0|asdf|1234asdf|");
}

BOOST_AUTO_TEST_CASE( structures ){
    assert_output_32("structures.eddi", "222|666|3.2300|0|asdf|333|888|4.3299|1|ertz|333|888|4.3299|1|ertz|");
    assert_output_64("structures.eddi", "222|666|3.2300|0|asdf|333|888|4.3300|1|ertz|333|888|4.3300|1|ertz|");
}

BOOST_AUTO_TEST_CASE( nested ){
    assert_output_32("nested.eddi", "100|");
    assert_output_64("nested.eddi", "100|");
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

/* Verify that the compilation fails for invalid statements */

BOOST_FIXTURE_TEST_SUITE(CompilationErrorsSuite, DeleteOutFixture)

BOOST_AUTO_TEST_CASE( params_assign ){
    assert_compilation_error("params_assign.eddi", "--32");
    assert_compilation_error("params_assign.eddi", "--64");
}

BOOST_AUTO_TEST_SUITE_END()

/* Standard library test suite  */

BOOST_FIXTURE_TEST_SUITE(StandardLibSuite, DeleteOutFixture)

BOOST_AUTO_TEST_CASE( std_lib_arrays_sum ){
    assert_output("stdlib_array_sum.eddi", "100");
}

BOOST_AUTO_TEST_CASE( std_lib_math_min ){
    assert_output("stdlib_math_min.eddi", "999|0|0|-1|0|-1");
}

BOOST_AUTO_TEST_CASE( std_lib_math_max ){
    assert_output("stdlib_math_max.eddi", "1000|1|1|0|0|0");
}

BOOST_AUTO_TEST_CASE( std_lib_math_factorial ){
    assert_output("stdlib_math_factorial.eddi", "1|1|2|362880");
}

BOOST_AUTO_TEST_CASE( std_lib_math_pow ){
    assert_output("stdlib_math_pow.eddi", "0|1|10|100|1024|1");
}

BOOST_AUTO_TEST_SUITE_END()
    
/* Unit test for bug fixes regression */

BOOST_FIXTURE_TEST_SUITE(BugFixesSuite, DeleteOutFixture)

BOOST_AUTO_TEST_CASE( while_bug ){
    assert_output("while_bug.eddi", "W1W2W3W4W5");
}

BOOST_AUTO_TEST_SUITE_END()
