//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_set>

#include "FunctionContext.hpp"
#include "likely.hpp"

#include "mtac/BasicBlockOptimizations.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

bool mtac::merge_basic_blocks(std::shared_ptr<mtac::Function> function){
    bool optimized = false;

    std::unordered_set<std::shared_ptr<mtac::BasicBlock>> usage;

    computeBlockUsage(function, usage);

    auto& blocks = function->getBasicBlocks();

    auto it = blocks.begin();

    //The ENTRY Basic block should not be merged
    ++it;

    while(it != blocks.end()){
        auto& block = *it;
        if(unlikely(block->statements.empty())){
            if(usage.find(*it) == usage.end()){
                it = blocks.erase(it);
                optimized = true;

                --it;
                continue;
            } else {
                auto next = it;
                ++next;
                
                if(next != blocks.end() && (*next)->index != -2 && usage.find(*next) == usage.end()){
                    block->statements = (*next)->statements;
                    
                    it = blocks.erase(it);
                    optimized = true;

                    --it;
                    continue;
                }
            }
        } else {
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
                    if(!(*next)->statements.empty()){
                        if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&(*(*next)->statements.begin()))){
                            if(!safe(*ptr)){
                                ++it;
                                continue;
                            }
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

bool mtac::remove_dead_basic_blocks(std::shared_ptr<mtac::Function> function){
    std::unordered_set<std::shared_ptr<mtac::BasicBlock>> usage;

    auto& blocks = function->getBasicBlocks();

    unsigned int before = blocks.size();

    auto it = blocks.begin();
    auto end = blocks.end();

    while(it != end){
        auto& block = *it;

        usage.insert(block);

        if(likely(!block->statements.empty())){
            auto& last = block->statements[block->statements.size() - 1];

            if(auto* ptr = boost::get<std::shared_ptr<mtac::Goto>>(&last)){
                if(usage.find((*ptr)->block) == usage.end()){
                    it = std::find(blocks.begin(), blocks.end(), (*ptr)->block);
                    continue;
                }
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&last)){
                usage.insert((*ptr)->block); 
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&last)){
                usage.insert((*ptr)->block); 
            }
        }

        ++it;
    }

    //The ENTRY and EXIT blocks should not be removed
    usage.insert(blocks.front());
    usage.insert(blocks.back());

    it = blocks.begin();
    end = blocks.end();

    blocks.erase(
            std::remove_if(it, end, 
                [&](std::shared_ptr<mtac::BasicBlock>& b){ return usage.find(b) == usage.end(); }), 
            end);

    return blocks.size() < before;
}
