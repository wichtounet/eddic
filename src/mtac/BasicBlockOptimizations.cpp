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
        
        if(block->index == -1){
            ++it;
            continue;
        } else if(block->index == -2){
            break;
        }
                
        auto next = it;
        ++next;

        if(unlikely(block->statements.empty())){
            if(usage.find(*it) == usage.end()){
                it = blocks.erase(it);
                optimized = true;

                --it;
                continue;
            } else {
                if(next != blocks.end() && (*next)->index != -2 && usage.find(*next) == usage.end()){
                    block->statements = (*next)->statements;
                    
                    it = blocks.erase(next);
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
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Goto>>(&last)){
                merge = (next != blocks.end() && (*ptr)->block == *next);

                if(merge){
                    block->statements.pop_back();
                    computeBlockUsage(function, usage);
                }
            }

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

    if(optimized){
        merge_basic_blocks(function);
    }
   
    return optimized; 
}

bool mtac::remove_dead_basic_blocks(std::shared_ptr<mtac::Function> function){
    auto& blocks = function->getBasicBlocks();
    unsigned int before = blocks.size();

    if(before <= 2){
        return false;
    }
    
    std::unordered_set<std::shared_ptr<mtac::BasicBlock>> usage;
    std::list<std::shared_ptr<mtac::BasicBlock>> queue;
    
    //ENTRY is always accessed
    queue.push_back(blocks.front());

    while(!queue.empty()){
        auto block = queue.back();
        queue.pop_back();

        if(usage.find(block) == usage.end()){
            usage.insert(block);
        
            if(likely(!block->statements.empty())){
                auto& last = block->statements.back();

                if(auto* ptr = boost::get<std::shared_ptr<mtac::Goto>>(&last)){
                    queue.push_back((*ptr)->block);

                    continue;
                } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&last)){
                    queue.push_back((*ptr)->block); 
                } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&last)){
                    queue.push_back((*ptr)->block); 
                }
            }

            //EXIT has no next block
            if(block->index != -2){
                //Add the next block
                auto it = std::find(blocks.begin(), blocks.end(), block);
                ++it;
                queue.push_back(*it);
            }
        }
    }

    auto it = blocks.begin();
    auto end = blocks.end();

    blocks.erase(
            std::remove_if(it, end, 
                [&](std::shared_ptr<mtac::BasicBlock>& b){ return usage.find(b) == usage.end(); }), 
            end);

    return blocks.size() < before;
}
