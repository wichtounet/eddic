//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>

#include "Utils.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE UtilsTest
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( toString ){
    std::string result = eddic::toString(33);

    BOOST_CHECK_EQUAL (result, "33");
}

BOOST_AUTO_TEST_CASE( toNumber ){
    int value = eddic::toNumber<int>("22");

    BOOST_CHECK_EQUAL (value, 22);
}
