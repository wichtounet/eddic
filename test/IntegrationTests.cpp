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

void assertOutputEquals(const std::string& file, const std::string& output){
    eddic::parseOptions(0, {});

    eddic::Compiler compiler;

    int code = compiler.compileOnly("test/cases/" + file);

    BOOST_CHECK_EQUAL (code, 0);

    std::string out = eddic::execCommand("./a.out"); 
    
    BOOST_CHECK_EQUAL (output, out);
}

BOOST_AUTO_TEST_CASE( if_ ){
    assertOutputEquals("if.eddi", "Cool");
}
