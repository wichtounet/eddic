//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_set>

#include "FunctionContext.hpp"
#include "Function.hpp"
#include "likely.hpp"

#include "mtac/merge_basic_blocks.hpp"
#include "mtac/Function.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

bool mtac::merge_basic_blocks::operator()(mtac::Function& function){
    bool optimized = false;

    std::unordered_set<mtac::basic_block_p> usage;

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
            auto& quadruple = block->statements.back();

            bool merge = false;

            if(quadruple.op == mtac::Operator::GOTO){
                merge = quadruple.block == next;

                if(merge){
                    block->statements.pop_back();
                    computeBlockUsage(function, usage);
                }
            } else if(quadruple.op == mtac::Operator::CALL){
                auto& target_function = quadruple.function();

                if(target_function.standard()){
                    merge = safe(target_function.mangled_name());
                } else {
                    merge = false;

                    for(auto& f : program.functions){
                        if(f.definition() == target_function){
                            merge = f.pure();
                        }
                    }
                }
            } else if(!quadruple.is_if() && !quadruple.is_if_false()){
                merge = true;
            }

            if(merge && next && next->index != -2){
                //Only if the next block is not used because we will remove its label
                if(usage.find(next) == usage.end()){
                    if(!next->statements.empty()){
                        auto next_quadruple = next->statements.front();
                        if(next_quadruple.op == mtac::Operator::CALL){
                            auto& target_function = next_quadruple.function();

                            if(target_function.standard()){
                                if(!safe(target_function.mangled_name())){
                                    ++it;
                                    continue;
                                }
                            } else {
                                auto safe = false;

                                for(auto& f : program.functions){
                                    if(f.definition() == target_function){
                                        safe = f.pure();
                                    }
                                }

                                if(!safe){
                                    ++it;
                                    continue;
                                }
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
