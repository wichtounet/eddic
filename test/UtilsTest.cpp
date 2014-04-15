//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <string>

#include "Utils.hpp"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( toString ){
    std::string result = eddic::toString(33);

    BOOST_CHECK_EQUAL (result, "33");
}

BOOST_AUTO_TEST_CASE( toNumber ){
    int value = eddic::toNumber<int>("22");

    BOOST_CHECK_EQUAL (value, 22);
}
