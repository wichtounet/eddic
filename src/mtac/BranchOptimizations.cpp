//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "mtac/BranchOptimizations.hpp"
#include "mtac/Function.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

bool mtac::optimize_branches::operator()(mtac::Function& function){
    bool optimized = false;
    
    for(auto& block : function){
        for(auto& quadruple : block->statements){
            if(quadruple.op == mtac::Operator::IF_FALSE_UNARY && boost::get<int>(&*quadruple.arg1)){
                int value = boost::get<int>(*quadruple.arg1);

                if(value == 0){
                    mtac::Quadruple goto_(quadruple.label(), mtac::Operator::GOTO);
                    goto_.block = quadruple.block;

                    quadruple = std::move(goto_);
                    optimized = true;

                    mtac::remove_edge(block, block->next);
                } else if(value == 1){
                    mtac::remove_edge(block, quadruple.block);

                    mtac::transform_to_nop(quadruple);
                    optimized = true;
                }
            } else if(quadruple.op == mtac::Operator::IF_UNARY && boost::get<int>(&*quadruple.arg1)){
                int value = boost::get<int>(*quadruple.arg1);

                if(value == 0){
                    mtac::remove_edge(block, quadruple.block);

                    mtac::transform_to_nop(quadruple);
                    optimized = true;
                } else if(value == 1){
                    auto goto_ = mtac::Quadruple(quadruple.label(), mtac::Operator::GOTO);
                    goto_.block = quadruple.block;

                    quadruple = std::move(goto_);
                    optimized = true;

                    mtac::remove_edge(block, block->next);
                }
            }
        }
    }

    return optimized;
}
