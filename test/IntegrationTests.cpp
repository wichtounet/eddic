//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <iostream>
#include <memory>

#include <boost/algorithm/string.hpp>

#include "Options.hpp"
#include "Compiler.hpp"
#include "Utils.hpp"
#include "Platform.hpp"
#include "GlobalContext.hpp"

#include "mtac/Program.hpp"

#define BOOST_TEST_MODULE eddic_test_suite
#include <BoostTestTargetConfig.h>

/*
 * \def TEST_APPLICATION(file) 
 * Generate a test case that verify that the sample compiles in both 32 and 64 bits mode. 
 */
#define TEST_APPLICATION(file)\
BOOST_AUTO_TEST_CASE( applications_##file ){\
    assert_compiles("eddi_applications/" #file "/" #file ".eddi", "--32", "--O0", #file ".1.out");\
    assert_compiles("eddi_applications/" #file "/" #file ".eddi", "--32", "--O1", #file ".2.out");\
    assert_compiles("eddi_applications/" #file "/" #file ".eddi", "--32", "--O3", #file ".3.out");\
    assert_compiles("eddi_applications/" #file "/" #file ".eddi", "--64", "--O0", #file ".4.out");\
    assert_compiles("eddi_applications/" #file "/" #file ".eddi", "--64", "--O1", #file ".5.out");\
    assert_compiles("eddi_applications/" #file "/" #file ".eddi", "--64", "--O3", #file ".6.out");\
}

/*
 * \def TEST_SAMPLE(file) 
 * Generate a test case that verify that the sample compiles in both 32 and 64 bits mode. 
 */
#define TEST_SAMPLE(file)\
BOOST_AUTO_TEST_CASE( samples_##file ){\
    assert_compiles("eddi_samples/" #file ".eddi", "--32", "--O0", #file ".1.out");\
    assert_compiles("eddi_samples/" #file ".eddi", "--32", "--O1", #file ".2.out");\
    assert_compiles("eddi_samples/" #file ".eddi", "--32", "--O3", #file ".3.out");\
    assert_compiles("eddi_samples/" #file ".eddi", "--64", "--O0", #file ".4.out");\
    assert_compiles("eddi_samples/" #file ".eddi", "--64", "--O1", #file ".5.out");\
    assert_compiles("eddi_samples/" #file ".eddi", "--64", "--O3", #file ".6.out");\
}

inline void remove(const std::string& file){
    remove(file.c_str());
}

inline std::shared_ptr<eddic::Configuration> parse_options(const std::string& file, const std::string& param1, const std::string& param2, const std::string& param3){
    std::string output_file = "--output=" + param3;

    const char* argv[6];
    argv[0] = "./bin/test";
    argv[1] = param1.c_str();
    argv[2] = "--quiet";
    argv[3] = param2.c_str();
    argv[4] = output_file.c_str();
    argv[5] = file.c_str();
    
    BOOST_TEST_MESSAGE( std::string("Compile with options ") + argv[1] + " " + argv[2] + " " + argv[3] + " " + argv[4] + " " + argv[5]); 

    auto configuration = eddic::parseOptions(6, argv);

    BOOST_REQUIRE (configuration);

    return configuration;
}

void assert_compiles(const std::string& file, const std::string& param1, const std::string& param2, const std::string& param3){
    auto configuration = parse_options(file, param1, param2, param3);

    eddic::Compiler compiler;
    int code = compiler.compile(file, configuration);

    BOOST_REQUIRE_EQUAL (code, 0);
    
    remove("./" + param3);
}

void assert_compilation_error(const std::string& file, const std::string& param1, const std::string& param2, const std::string& param3){
    auto configuration = parse_options("test/cases/" + file, param1, param2, param3);

    eddic::Compiler compiler;
    int code = compiler.compile("test/cases/" + file, configuration);

    BOOST_REQUIRE_EQUAL (code, 1);
    
    remove("./" + param3);
}

std::vector<std::string> split(std::string value){
    std::vector<std::string> parts;
    boost::split(parts, value, boost::is_any_of("|"));
    return parts;
}

std::string get_output(const std::string& file, const std::string& param1, const std::string& param2, const std::string& param3){
    auto configuration = parse_options("test/cases/" + file, param1, param2, param3);

    eddic::Compiler compiler;
    int code = compiler.compile("test/cases/" + file, configuration);

    BOOST_REQUIRE_EQUAL (code, 0);

    std::string out = eddic::execCommand("./" + param3); 
    remove("./" + param3);
    
    return out;
}

template<typename T>
void validate_output(std::vector<std::string>& parts, int index, T first){
    auto value = boost::lexical_cast<T>(parts[index]);

    BOOST_CHECK_EQUAL(value, first);
}

template<>
void validate_output(std::vector<std::string>& parts, int index, double first){
    auto value = boost::lexical_cast<double>(parts[index]);

    BOOST_CHECK_CLOSE(value, first, 0.005);
}

template<>
void validate_output(std::vector<std::string>& parts, int index, const char* first){
    auto value = parts[index];

    BOOST_CHECK_EQUAL(value, first);
}

template<typename First, typename ...T>
void validate_output(std::vector<std::string>& parts, int index, First first, T... rest){
    validate_output(parts, index, first);
    validate_output(parts, index + 1, rest...);
}

template<typename ...T>
void validate_output(const std::string& file, const std::string& param1, const std::string& param2, const std::string& param3, T... arguments){
    auto out = get_output(file, param1, param2, param3);
    auto parts = split(out);
    validate_output(parts, 0, arguments...);
}

template<typename ...T>
void validate(const std::string& file, T... arguments){
    validate_output(file, "--32", "--O0", file + ".1.out", arguments...);
    validate_output(file, "--32", "--O1", file + ".2.out", arguments...);
    validate_output(file, "--32", "--O3", file + ".3.out", arguments...);
    validate_output(file, "--64", "--O0", file + ".4.out", arguments...);
    validate_output(file, "--64", "--O1", file + ".5.out", arguments...);
    validate_output(file, "--64", "--O3", file + ".6.out", arguments...);
}

void assert_output_equals(const std::string& file, const std::string& output, const std::string& param1, const std::string& param2, const std::string& param3){
    auto out = get_output(file, param1, param2, param3);
    
    BOOST_CHECK_EQUAL (output, out);
}

void assert_output_32(const std::string& file, const std::string& output){
    assert_output_equals(file, output, "--32", "--O0", file + ".1.out");
    assert_output_equals(file, output, "--32", "--O1", file + ".2.out");
    assert_output_equals(file, output, "--32", "--O3", file + ".3.out");
}

void assert_output_64(const std::string& file, const std::string& output){
    assert_output_equals(file, output, "--64", "--O0", file + ".4.out");
    assert_output_equals(file, output, "--64", "--O1", file + ".5.out");
    assert_output_equals(file, output, "--64", "--O3", file + ".6.out");
}

void assert_output(const std::string& file, const std::string& output){
    assert_output_32(file, output);
    assert_output_64(file, output);
}

/* Compiles all the applications */

BOOST_AUTO_TEST_SUITE( ApplicationsSuite )

TEST_APPLICATION(hangman)

BOOST_AUTO_TEST_SUITE_END()

/* Compiles all the samples */

BOOST_AUTO_TEST_SUITE( SamplesSuite )

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
TEST_SAMPLE(registers)
TEST_SAMPLE(structures)

BOOST_AUTO_TEST_SUITE_END()

/* Specific tests */ 

BOOST_AUTO_TEST_SUITE(SpecificSuite)

BOOST_AUTO_TEST_CASE( addressof ){
    assert_output("addressof.eddi", "44|44|55|55|");
}

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

BOOST_AUTO_TEST_CASE( arrays_in_struct ){
    assert_output("arrays_in_struct.eddi", "5|55|66|77|66|166|177|66|166|177|5|");
}

BOOST_AUTO_TEST_CASE( char_type ){
    assert_output("char_type.eddi", "a|0|z|e|e|u|u|");
}

BOOST_AUTO_TEST_CASE( char_at ){
    assert_output("char_at.eddi", "asdf|a|s|d|1|");
}

BOOST_AUTO_TEST_CASE( ctor_dtor_heap ){
    assert_output("ctor_dtor_heap.eddi", "CA|0|DA|CAI|55|DA|CAII|3300|DA|CAS|666|DA|");
}

BOOST_AUTO_TEST_CASE( ctor_dtor_stack ){
    assert_output("ctor_dtor_stack.eddi", "CA|0|CAI|55|DA|CAII|3300|CAS|666|0|DA|DA|DA|");
}

BOOST_AUTO_TEST_CASE( copy_constructors ){
    assert_output("copy_constructors.eddi", "9|5|99|55|9|5|99|55|");
}

BOOST_AUTO_TEST_CASE( casts ){
    validate("casts.eddi", 5.0, 5, 4, 333, 5.0, 8.33, 'B', 'B', 90);
}

BOOST_AUTO_TEST_CASE( compound ){
    assert_output("compound.eddi", "6|9|6|18|6|0|");
}

BOOST_AUTO_TEST_CASE( delete_any ){
    assert_output("delete_any.eddi", "99|");
}

BOOST_AUTO_TEST_CASE( if_ ){
    assert_output("if.eddi", "1|1|1|");
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

BOOST_AUTO_TEST_CASE( string_foreach ){
    assert_output("string_foreach.eddi", "a|s|d|f|");
}

BOOST_AUTO_TEST_CASE( string_pointers ){
    assert_output("string_pointers.eddi", "a|a|b|b|c|c|c|");
}

BOOST_AUTO_TEST_CASE( int_pointers ){
    assert_output("int_pointers.eddi", "44|44|55|55|66|66|66|");
}

BOOST_AUTO_TEST_CASE( bool_pointers ){
    assert_output("bool_pointers.eddi", "0|0|1|1|0|0|1|");
}

BOOST_AUTO_TEST_CASE( cmove ){
    assert_output("cmov.eddi", "8|4|99|77|");
}

BOOST_AUTO_TEST_CASE( dynamic ){
    assert_output("dynamic.eddi", "5|55|555|5555|55555|0|-9|666|9999|1000|0|1|2|3|4|");
}

BOOST_AUTO_TEST_CASE( dynamic_arrays_in_struct ){
    assert_output("dynamic_arrays_in_struct.eddi", "5|55|66|77|66|166|177|66|166|177|5|");
}

BOOST_AUTO_TEST_CASE( dynamic_struct ){
    assert_output("dynamic_struct.eddi", "0|-9|55|asdf|999|-9|0||0|666|777|666|777|1000|");
}

BOOST_AUTO_TEST_CASE( float_pointers ){
    validate("float_pointers.eddi", 44.4, 44.4, 55.5, 55.5, 66.6, 66.6, 66.6);
}

BOOST_AUTO_TEST_CASE( struct_pointers ){
    assert_output("struct_pointers.eddi", "44|44|44.0|44|44|44|44|44.0|44|44|55|55|55.0|55|55|55|55|55.0|55|55|66|66|66.0|66|66|66|66|66.0|66|66|66|66|66.0|66|66|");
}

BOOST_AUTO_TEST_CASE( member_pointers ){
    assert_output("member_pointers.eddi", "44|44|55|55|66|66|66|44|44|55|55|66|66|66|");
}

BOOST_AUTO_TEST_CASE( member_function_calls ){
    assert_output("member_function_calls.eddi", "0|5|5|10|10|0|5|10|15|0|5|15|20|0|5|20|25|");
}

BOOST_AUTO_TEST_CASE( member_functions ){
    assert_output("member_functions.eddi", "0|1|100|180|260|");
}

BOOST_AUTO_TEST_CASE( member_functions_param_stack ){
    assert_output("member_functions_param_stack.eddi", "0|1|100|180|260|");
}

BOOST_AUTO_TEST_CASE( memory ){
    assert_output("memory.eddi", "4|4|4|1|1|1|5|6|7|8|5|6|7|8|5|6|7|8|1|2|3|4|1|2|3|4|1|2|3|4|1|2|3|4|1|2|3|4|1|2|3|4|1|2|3|4|1|2|3|4|");
}

BOOST_AUTO_TEST_CASE( pass_member_by_value ){
    assert_output("pass_member_by_value.eddi", "77.7699|66|66|55|66|");
}

BOOST_AUTO_TEST_CASE( ternary ){
    validate("ternary.eddi", 44, 66, 44, 66, 1, 0, 44.4, 66.6);
}

BOOST_AUTO_TEST_CASE( while_ ){
    assert_output("while.eddi", "01234");
}

BOOST_AUTO_TEST_CASE( do_while_ ){
    assert_output("do_while.eddi", "01234");
}

BOOST_AUTO_TEST_CASE( defaults ){
    assert_output("defaults.eddi", "0|0|0.0||0|0|0|0|0.0|0.0|||");
}

BOOST_AUTO_TEST_CASE( float_1 ){
    validate("float_1.eddi", 5.499, 100.0, -100.0, 100.0, 2.0889, 4.2, 3.33, 1.5, 3.0, 5.0, 4.5, 5.75, 1.5, -2.0, 7.5, 2.27, 7.5590, 14.4927, 3.0, 8.0);
}

BOOST_AUTO_TEST_CASE( float_2 ){
    validate("float_2.eddi", 3.0910, 2.0934, 5.1844, 1, 1, 11111, 8.0, 13.75, 2.5, 5.5, 2.5, 5.5, 2.5, 5.5, 2.5, 5.5);
}

BOOST_AUTO_TEST_CASE( for_ ){
    assert_output("for.eddi", "01234");
}

BOOST_AUTO_TEST_CASE( foreach_ ){
    assert_output("foreach.eddi", "012345");
}

BOOST_AUTO_TEST_CASE( globals_ ){
    assert_output("globals.eddi", "1000a2000");
}

BOOST_AUTO_TEST_CASE( inc ){
    assert_output("inc.eddi", "0|1|2|1|0|1|1|1|1|2|1|1|0|");
}

BOOST_AUTO_TEST_CASE( void_ ){
    assert_output("void.eddi", "4445");
}

BOOST_AUTO_TEST_CASE( return_string ){
    assert_output("return_string.eddi", "abcdef");
}

BOOST_AUTO_TEST_CASE( return_by_value ){
    assert_output("return_by_value.eddi", "99|66|11|88|");
}

BOOST_AUTO_TEST_CASE( return_int ){
    assert_output("return_int.eddi", "484|");
}

BOOST_AUTO_TEST_CASE( return_pointers ){
    assert_output("return_pointers.eddi", "66");
}

BOOST_AUTO_TEST_CASE( pointer_arrays ){
    assert_output("pointer_arrays.eddi", "55|66|555|666|55|66|555|666|");
}

BOOST_AUTO_TEST_CASE( recursive_functions ){
    assert_output("recursive.eddi", "362880");
}

BOOST_AUTO_TEST_CASE( single_inheritance ){
    validate("single_inheritance.eddi", 99, 55, 66, 77, 'B', 55, 66, 55.2, 55, 56.3, 55, 'B', 55, 66, 57.4, 55, 58.5, 55, 55, 66, 77);
}

BOOST_AUTO_TEST_CASE( math ){
    assert_output("math.eddi", "333|111|-111|0|24642|2|-2|-1|1|2|0|-111|232|40|");
}

BOOST_AUTO_TEST_CASE( builtin ){
    assert_output("builtin.eddi", "10|11|12|13|12|13|10|11|4|8|13|0|3|");
}

BOOST_AUTO_TEST_CASE( assign_value ){
    assert_output("assign_value.eddi", "66779921");
}

BOOST_AUTO_TEST_CASE( println ){
    assert_output("println.eddi", "\n");
}

BOOST_AUTO_TEST_CASE( prints ){
    validate("prints.eddi", 111, 0, -111, 0, 1, 999.9899, 1.0089, 0.0, -1.0089, 0.0, -1.0089, -999.9899, "", -0, "asdf", "1234asdf");
}

BOOST_AUTO_TEST_CASE( structures ){
    validate("structures.eddi", 222, 666, 3.23, 0, "asdf", 333, 888, 4.33, 1, "ertz", 333, 888, 4.33, 1, "ertz");
}

BOOST_AUTO_TEST_CASE( struct_member_pointers ){
    assert_output("struct_member_pointers.eddi", "55|44|66|66|");
}

BOOST_AUTO_TEST_CASE( struct_array ){
    assert_output("struct_arrays.eddi", "99|111|999|1111|99|111|999|1111|");
}

BOOST_AUTO_TEST_CASE( switch_ ){
    assert_output("switch.eddi", "5|5|3|6|default|4|");
}

BOOST_AUTO_TEST_CASE( switch_string ){
    assert_output("switch_string.eddi", "5|5|3|6|default|4|");
}

BOOST_AUTO_TEST_CASE( nested ){
    validate("nested.eddi", 222, 555, 333, 444, 2222, 5555, 3333, 4444, "", 222, 555,333, 444, 2222, 5555, 3333, 4444);
}

static void test_args(const std::string& arg1, const std::string& arg2, const std::string& arg3){
    auto configuration = parse_options("test/cases/args.eddi", arg1, arg2, arg3);

    eddic::Compiler compiler;
    int code = compiler.compile("test/cases/args.eddi", configuration);

    BOOST_REQUIRE_EQUAL (code, 0);

    std::string out = eddic::execCommand("./" + arg3); 
    BOOST_CHECK_EQUAL ("./" + arg3 + "|", out);
    
    out = eddic::execCommand("./" + arg3 + " arg1 arg2 arg3"); 
    BOOST_CHECK_EQUAL ("./" + arg3 + "|arg1|arg2|arg3|", out);
    
    remove("./" + arg3);
}

BOOST_AUTO_TEST_CASE( args ){
    test_args("--32", "--O0", "args.1.out");
    test_args("--32", "--O1", "args.2.out");
    test_args("--32", "--O3", "args.3.out");

    test_args("--64", "--O0", "args.4.out");
    test_args("--64", "--O1", "args.5.out");
    test_args("--64", "--O3", "args.6.out");
}

BOOST_AUTO_TEST_SUITE_END()

/* Template tests */ 

BOOST_AUTO_TEST_SUITE(TemplateSuite)

BOOST_AUTO_TEST_CASE( class_templates ){
    assert_output("class_templates.eddi", "C1|C1|C2|100|100|13.3000|13.3000|7|7|88|88|55.2500|55.2500|1.0|10|D|D|D|");
}

BOOST_AUTO_TEST_CASE( function_templates ){
    assert_output_32("function_templates.eddi", "9|5.5000|9|99|9.8999|100|a|b|9|5.5000|a|9|9|a|a|");
    assert_output_64("function_templates.eddi", "9|5.5000|9|99|9.9000|100|a|b|9|5.5000|a|9|9|a|a|");
}

BOOST_AUTO_TEST_CASE( member_function_templates ){
    assert_output("member_function_templates.eddi", "1|5|2|5|3|5.5000|4|5|5|100|6|1|");
}

BOOST_AUTO_TEST_SUITE_END()

/* Verify that the compilation fails for invalid statements */

BOOST_AUTO_TEST_SUITE(CompilationErrorsSuite)

BOOST_AUTO_TEST_CASE( params_assign ){
    assert_compilation_error("params_assign.eddi", "--32", "--O3", "params_assign.out");
    assert_compilation_error("params_assign.eddi", "--64", "--O3", "params_assign.out");
}

BOOST_AUTO_TEST_CASE( wrong_print ){
    assert_compilation_error("wrong_print.eddi", "--32", "--O3", "wrong_print.out");
    assert_compilation_error("wrong_print.eddi", "--64", "--O3", "wrong_print.out");
}

BOOST_AUTO_TEST_CASE( invalid_inheritance ){
    assert_compilation_error("invalid_inheritance.eddi", "--32", "--O3", "invalid_inheritance.out");
    assert_compilation_error("invalid_inheritance.eddi", "--64", "--O3", "invalid_inheritance.out");
}

BOOST_AUTO_TEST_SUITE_END()

/* Standard library test suite  */

BOOST_AUTO_TEST_SUITE(StandardLibSuite)

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

BOOST_AUTO_TEST_CASE( std_lib_str_equals ){
    assert_output("stdlib_str_equals.eddi", "1|0|0|0|1|1|");
}

BOOST_AUTO_TEST_CASE( std_lib_string ){
    assert_output("stdlib_string.eddi", "adsf|4|adsf|8|dddddddd|4|adsf|4|adsf|1|0|1|0|1|0|1|0|1|");
}

BOOST_AUTO_TEST_CASE( std_lib_string_concat ){
    assert_output("stdlib_string_concat.eddi", "asdf1234|1234|asdf1234|1234asdf1234|asdf1234|1234asdf1234|you1234asdf1234|");
}

BOOST_AUTO_TEST_CASE( std_lib_string_concat_int ){
    assert_output("stdlib_string_concat_int.eddi", "test987|test987-561|asdf98655|asdf986551|");
}

BOOST_AUTO_TEST_CASE( std_linked_list ){
    assert_output("stdlib_linked_list.eddi", "0||1|55|55|2|55|11|3|33|11|4|99|11|{99|33|55|11|}{11|33|99|}4|99|11|3|33|11|2|33|88|1|88|88|");
}

BOOST_AUTO_TEST_CASE( std_vector ){
    assert_output("stdlib_vector.eddi", "0|0||1|1|55|55|2|2|55|66|3|4|55|77|66|2|4|55|66|5|8|55|111|{55|66|88|99|111|}");
}

BOOST_AUTO_TEST_SUITE_END()
    
/* Unit test for bug fixes regression */

BOOST_AUTO_TEST_SUITE(BugFixesSuite)

BOOST_AUTO_TEST_CASE( while_bug ){
    assert_output("while_bug.eddi", "W1W2W3W4W5");
}

BOOST_AUTO_TEST_SUITE_END()
    
/* Unit test for optimization regression */

BOOST_AUTO_TEST_SUITE(OptimizationSuite)

eddic::statistics& compute_stats(const std::string& file){
    auto configuration = parse_options("test/cases/" + file, "--64", "--O3", "test/cases/" + file + ".out");

    eddic::Compiler compiler;
    auto pair = compiler.compile_mtac("test/cases/" + file, eddic::Platform::INTEL_X86_64, configuration);

    auto global_context = pair.first->context;
    return global_context->stats();
}

BOOST_AUTO_TEST_CASE( parameter_propagation ){
    auto& stats = compute_stats("parameter_propagation.eddi");

    BOOST_REQUIRE_EQUAL(stats.counter("propagated_parameter"), 5);
}

BOOST_AUTO_TEST_CASE( remove_empty_functions ){
    auto& stats = compute_stats("remove_empty_functions.eddi");

    BOOST_REQUIRE_EQUAL(stats.counter("empty_function_removed"), 1);
}

BOOST_AUTO_TEST_CASE( remove_empty_loops ){
    auto& stats = compute_stats("remove_empty_loops.eddi");

    BOOST_REQUIRE_EQUAL(stats.counter("empty_loop_removed"), 1);
}

BOOST_AUTO_TEST_CASE( invariant_code_motion ){
    auto& stats = compute_stats("invariant_code_motion.eddi");

    BOOST_REQUIRE_EQUAL(stats.counter("invariant_moved"), 1);
}

BOOST_AUTO_TEST_CASE( complete_loop_peeling ){
    auto& stats = compute_stats("complete_loop_peeling.eddi");

    BOOST_REQUIRE_EQUAL(stats.counter("loop_peeled"), 1);
}

BOOST_AUTO_TEST_CASE( loop_unrolling ){
    auto& stats = compute_stats("loop_unrolling.eddi");

    BOOST_REQUIRE_EQUAL(stats.counter("loop_unrolled"), 1);
}

BOOST_AUTO_TEST_CASE( common_subexpr_elimination ){
    auto& stats = compute_stats("common_subexpr_elim.eddi");

    BOOST_REQUIRE_EQUAL(stats.counter("common_subexpr_eliminated"), 4);
}

BOOST_AUTO_TEST_SUITE_END()
