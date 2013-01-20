//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_set>

#include "mtac/remove_dead_basic_blocks.hpp"
#include "mtac/Function.hpp"

using namespace eddic;

bool mtac::remove_dead_basic_blocks::operator()(mtac::Function& function){
    bool optimized;
    bool optimized_once = false;

    do {
        unsigned int before = function.bb_count();

        if(before <= 2){
            return optimized_once;
        }

        std::unordered_set<basic_block_p> live_basic_blocks;

        for(auto& basic_block : function){
            if(basic_block->index < 0){
                live_basic_blocks.insert(basic_block);
            } else {
                for(auto& predecessor : basic_block->predecessors){
                    if(live_basic_blocks.find(predecessor) != live_basic_blocks.end()){
                        live_basic_blocks.insert(basic_block);
                        break;
                    }
                }
            }
        }

        auto it = iterate(function);

        while(it.has_next()){
            auto& block = *it;

            if(live_basic_blocks.find(block) == live_basic_blocks.end()){
                it.erase();
            } else {
                ++it;
            }
        }

        optimized = function.bb_count() < before;
        if(optimized){
            optimized_once = true;
        }
    } while(optimized);

    return optimized_once;
}
