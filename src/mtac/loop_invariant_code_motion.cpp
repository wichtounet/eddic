//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <map>
#include <unordered_map>

#include "iterators.hpp"
#include "VisitorUtils.hpp"
#include "Type.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "logging.hpp"
#include "Variable.hpp"

#include "mtac/Loop.hpp"
#include "mtac/loop_invariant_code_motion.hpp"
#include "mtac/loop_analysis.hpp"
#include "mtac/Function.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Utils.hpp"
#include "mtac/variable_usage.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

namespace {

bool is_invariant(boost::optional<mtac::Argument>& argument, mtac::Usage& usage){
    if(argument){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*argument)){
            return usage.written[*ptr] == 0;
        }
    }

    return true;
}

bool is_invariant(std::shared_ptr<mtac::Quadruple>& quadruple, mtac::Usage& usage){
    //TODO Relax this rule by making a more powerful memory analysis
    if(quadruple.op == mtac::Operator::DOT || quadruple.op == mtac::Operator::FDOT || quadruple.op == mtac::Operator::PDOT){
        return false;
    }

    if(mtac::erase_result(quadruple.op)){
        //If there are more than one write to this variable, the computation is not invariant
        if(usage.written[quadruple.result] > 1){
            return false;
        }

        return is_invariant(quadruple.arg1, usage) && is_invariant(quadruple.arg2, usage);
    }

    return false;
}

mtac::basic_block_p create_pre_header(mtac::Loop& loop, mtac::Function& function){
    auto first_bb = *loop.begin();

    //Remove the fall through edge
    mtac::remove_edge(first_bb->prev, first_bb);
    
    auto pre_header = function.new_bb();
    
    function.insert_before(function.at(first_bb), pre_header);

    //Create the fall through edge
    mtac::make_edge(pre_header, pre_header->next);
    mtac::make_edge(pre_header->prev, pre_header);
    
    return pre_header;
}

/*!
 * \brief Test if an invariant is valid or not. 
 * An invariant defining v is valid if: 
 * 1. It is in a basic block that dominates all other uses of v
 * 2. It is in a basic block that dominates all exit blocks of the loop
 * 3. It is not an NOP
 */
bool is_valid_invariant(mtac::basic_block_p source_bb, std::shared_ptr<mtac::Quadruple> quadruple, mtac::Loop& loop){
    //It is not necessary to move statements with no effects. 
    if(quadruple.op == mtac::Operator::NOP){
        return false;
    }

    auto var = quadruple.result;

    for(auto& bb : loop){
        //A bb always dominates itself => no need to consider the source basic block
        if(bb != source_bb){
            if(use_variable(bb, var)){
                auto dominator = bb->dominator;

                //If the bb is not dominated by the source bb, it is not valid
                if(dominator != source_bb){
                    return false;
                }
            }
        }
    }
    
    auto exit_block = *loop.blocks().rbegin();

    if(exit_block == source_bb){
        return true;
    }
                
    auto dominator = exit_block->dominator;

    //If the exit bb is not dominated by the source bb, it is not valid
    if(dominator != source_bb){
        return false;
    }
    
    return true;
}

bool loop_invariant_code_motion(mtac::Loop& loop, mtac::Function& function){
    mtac::basic_block_p pre_header;

    bool optimized = false;

    auto usage = compute_write_usage(loop);

    for(auto& bb : loop){
        auto it = iterate(bb->statements); 

        while(it.has_next()){
            auto statement = *it;

            if(is_invariant(statement, usage)){
                if(is_valid_invariant(bb, statement, loop)){
                    //Create the preheader if necessary
                    if(!pre_header){
                        pre_header = create_pre_header(loop, function);
                    }

                    function.context->global()->stats().inc_counter("invariant_moved");

                    it.erase();
                    pre_header->statements.push_back(statement);

                    optimized = true;

                    continue;
                } 
            }

            ++it;
        }
    }

    return optimized;
}

} //end of anonymous namespace

bool mtac::loop_invariant_code_motion::operator()(mtac::Function& function){
    if(function.loops().empty()){
        return false;
    }

    bool optimized = false;

    for(auto& loop : function.loops()){
        optimized |= ::loop_invariant_code_motion(loop, function);
    }
    
    return optimized;
}
