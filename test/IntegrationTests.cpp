//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>

#define CLEAR
#include "Compiler.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Eddic Tests
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( first ){
    BOOST_CHECK_EQUAL ("33", "33");
}
