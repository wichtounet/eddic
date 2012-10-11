//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>

#include "mtac/Function.hpp"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using namespace eddic;

BOOST_AUTO_TEST_CASE( bb_entry_exit_count ){
    auto function = std::make_shared<mtac::Function>(nullptr, "test_function");

    function->create_entry_bb();
    function->create_exit_bb();

    BOOST_CHECK_EQUAL(function->entry_bb()->index, -1);
    BOOST_CHECK_EQUAL(function->exit_bb()->index, -2);
    
    BOOST_CHECK(function->entry_bb()->next == function->exit_bb());
    BOOST_CHECK(function->exit_bb()->prev == function->entry_bb());

    BOOST_CHECK_EQUAL(function->bb_count(), 2u);
}

BOOST_AUTO_TEST_CASE( bb_iterators ){
    auto function = std::make_shared<mtac::Function>(nullptr, "test_function");

    function->create_entry_bb();
    function->append_bb();
    function->append_bb();
    function->append_bb();
    function->create_exit_bb();

    BOOST_CHECK_EQUAL(function->entry_bb()->index, -1);
    BOOST_CHECK_EQUAL(function->exit_bb()->index, -2);

    auto it = function->begin();
    auto end = function->end();
    
    BOOST_CHECK_EQUAL((*it)->index, -1);
    BOOST_CHECK(it != end);

    ++it;
    BOOST_CHECK_EQUAL((*it)->index, 1);
    ++it;
    BOOST_CHECK_EQUAL((*it)->index, 2);
    ++it;
    BOOST_CHECK_EQUAL((*it)->index, 3);
    
    ++it;
    BOOST_CHECK_EQUAL((*it)->index, -2);

    ++it;
    BOOST_CHECK(it == end);

    --it;
    BOOST_CHECK_EQUAL((*it)->index, -2);

    BOOST_CHECK_EQUAL(function->bb_count(), 5u);
}

BOOST_AUTO_TEST_CASE( bb_new_bb ){
    auto function = std::make_shared<mtac::Function>(nullptr, "test_function");
    
    BOOST_CHECK_EQUAL(function->bb_count(), 0u);

    function->create_entry_bb();
    function->create_exit_bb();

    auto bb = function->new_bb();
    
    BOOST_CHECK_EQUAL(function->bb_count(), 2u);

    auto it = function->begin();
    ++it;

    function->insert_before(it, bb);
    
    BOOST_CHECK_EQUAL(function->bb_count(), 3u);
    
    BOOST_CHECK(bb->next == function->exit_bb());
    BOOST_CHECK(bb->prev == function->entry_bb());
    BOOST_CHECK(function->entry_bb()->next == bb);
    BOOST_CHECK(function->exit_bb()->prev == bb);


}
