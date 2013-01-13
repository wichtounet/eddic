//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "likely.hpp"

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
                    auto goto_ = std::make_shared<mtac::Quadruple>(quadruple.label(), mtac::Operator::GOTO);
                    goto_->block = quadruple.block;

                    quadruple = goto_;
                    optimized = true;

                    mtac::remove_edge(block, block->next);
                } else if(value == 1){
                    mtac::remove_edge(block, quadruple.block);

                    quadruple = std::make_shared<mtac::Quadruple>(mtac::Operator::NOP);
                    optimized = true;
                }
            } else if(quadruple.op == mtac::Operator::IF_UNARY && boost::get<int>(&*quadruple.arg1)){
                int value = boost::get<int>(*quadruple.arg1);

                if(value == 0){
                    mtac::remove_edge(block, quadruple.block);

                    quadruple = std::make_shared<mtac::Quadruple>(mtac::Operator::NOP);
                    optimized = true;
                } else if(value == 1){
                    auto goto_ = std::make_shared<mtac::Quadruple>(quadruple.label(), mtac::Operator::GOTO);
                    goto_->block = quadruple.block;

                    quadruple = goto_;
                    optimized = true;

                    mtac::remove_edge(block, block->next);
                }
            }
        }
    }

    return optimized;
}
