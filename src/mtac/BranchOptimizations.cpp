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
#include "mtac/Statement.hpp"

using namespace eddic;

bool mtac::optimize_branches::operator()(mtac::Function& function){
    bool optimized = false;
    
    for(auto& block : function){
        for(auto& statement : block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                if(boost::get<int>(&*(*ptr)->arg1)){
                    int value = boost::get<int>(*(*ptr)->arg1);

                    if((*ptr)->op == mtac::Operator::IF_FALSE_UNARY){

                        if(value == 0){
                            auto goto_ = std::make_shared<mtac::Quadruple>((*ptr)->label(), mtac::Operator::GOTO);
                            goto_->block = (*ptr)->block;

                            statement = goto_;
                            optimized = true;

                            mtac::remove_edge(block, block->next);
                        } else if(value == 1){
                            mtac::remove_edge(block, (*ptr)->block);

                            statement = std::make_shared<mtac::Quadruple>(mtac::Operator::NOP);
                            optimized = true;
                        }
                    } else if((*ptr)->op == mtac::Operator::IF_UNARY){
                        if(value == 0){
                            mtac::remove_edge(block, (*ptr)->block);

                            statement = std::make_shared<mtac::Quadruple>(mtac::Operator::NOP);
                            optimized = true;
                        } else if(value == 1){
                            auto goto_ = std::make_shared<mtac::Quadruple>((*ptr)->label(), mtac::Operator::GOTO);
                            goto_->block = (*ptr)->block;

                            statement = goto_;
                            optimized = true;

                            mtac::remove_edge(block, block->next);
                        }
                    }
                }
            } 
        }
    }

    return optimized;
}
