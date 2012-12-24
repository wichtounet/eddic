//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/remove_dead_basic_blocks.hpp"
#include "mtac/Function.hpp"

using namespace eddic;

bool mtac::remove_dead_basic_blocks::operator()(mtac::Function& function){
    unsigned int before = function.bb_count();

    if(before <= 2){
        return false;
    }

    auto it = iterate(function);

    //ENTRY is always accessed
    ++it;

    while(it.has_next()){
        auto& block = *it;

        if(block->predecessors.empty()){
            it.erase();
        } else {
            ++it;
        }
    }

    return function.bb_count() < before;
}
