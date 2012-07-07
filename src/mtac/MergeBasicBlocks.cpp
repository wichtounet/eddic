//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_set>

#include "FunctionContext.hpp"

#include "mtac/MergeBasicBlocks.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

bool mtac::merge_basic_blocks(std::shared_ptr<mtac::Function> function){
    bool optimized = false;

    std::unordered_set<std::shared_ptr<mtac::BasicBlock>> usage;

    computeBlockUsage(function, usage);

    auto& blocks = function->getBasicBlocks();

    auto it = blocks.begin();

    //The ENTRY Basic block should not been merged
    ++it;

    while(it != blocks.end()){
        auto& block = *it;
        if(likely(!block->statements.empty())){
            auto& last = block->statements[block->statements.size() - 1];

            bool merge = false;

            if(boost::get<std::shared_ptr<mtac::Quadruple>>(&last)){
                merge = true;
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&last)){
                merge = safe(*ptr); 
            } else if(boost::get<std::shared_ptr<mtac::NoOp>>(&last)){
                merge = true;
            }

            auto next = it;
            ++next;

            if(merge && next != blocks.end() && (*next)->index != -2){
                //Only if the next block is not used because we will remove its label
                if(usage.find(*next) == usage.end()){
                    if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&(*(*next)->statements.begin()))){
                        if(!safe(*ptr)){
                            ++it;
                            continue;
                        }
                    }

                    block->statements.insert(block->statements.end(), (*next)->statements.begin(), (*next)->statements.end());

                    it = blocks.erase(next);
                    optimized = true;

                    --it;
                    continue;
                }
            }
        }

        ++it;
    }
   
    return optimized; 
}
