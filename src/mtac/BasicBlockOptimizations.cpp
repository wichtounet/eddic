//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_set>

#include "FunctionContext.hpp"
#include "likely.hpp"

#include "mtac/BasicBlockOptimizations.hpp"
#include "mtac/Function.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Statement.hpp"

using namespace eddic;

bool mtac::merge_basic_blocks::operator()(std::shared_ptr<mtac::Function> function){
    bool optimized = false;

    std::unordered_set<std::shared_ptr<mtac::BasicBlock>> usage;

    computeBlockUsage(function, usage);

    auto it = iterate(function);

    //The ENTRY Basic block should not be merged
    ++it;

    while(it.has_next()){
        auto& block = *it;
        
        if(block->index == -1){
            ++it;
            continue;
        } else if(block->index == -2){
            break;
        }
                
        auto next = block->next;

        if(unlikely(block->statements.empty())){
            if(usage.find(block) == usage.end()){
                it.erase();
                optimized = true;

                --it;
                continue;
            } else {
                if(next && next->index != -2 && usage.find(next) == usage.end()){
                    it.merge_in(next);
                    optimized = true;

                    --it;
                    continue;
                }
            }
        } else {
            auto& last = block->statements.back();

            bool merge = false;

            if(boost::get<std::shared_ptr<mtac::Quadruple>>(&last)){
                merge = true;
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&last)){
                merge = safe(*ptr); 
            } else if(boost::get<std::shared_ptr<mtac::NoOp>>(&last)){
                merge = true;
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Goto>>(&last)){
                merge = (*ptr)->block == next;

                if(merge){
                    block->statements.pop_back();
                    computeBlockUsage(function, usage);
                }
            }

            if(merge && next && next->index != -2){
                //Only if the next block is not used because we will remove its label
                if(usage.find(next) == usage.end()){
                    if(!next->statements.empty()){
                        if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&(next->statements.front()))){
                            if(!safe(*ptr)){
                                ++it;
                                continue;
                            }
                        }
                    }

                    it.merge_in(next);

                    optimized = true;

                    --it;
                    continue;
                }
            }
        }

        ++it;
    }

    if(optimized){
        (*this)(function);
    }
   
    return optimized; 
}

bool mtac::remove_dead_basic_blocks::operator()(std::shared_ptr<mtac::Function> function){
    unsigned int before = function->bb_count();

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

    return function->bb_count() < before;
}
