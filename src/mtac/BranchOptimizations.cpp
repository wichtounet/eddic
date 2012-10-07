//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "likely.hpp"

#include "mtac/BranchOptimizations.hpp"

using namespace eddic;

bool mtac::optimize_branches::operator()(std::shared_ptr<mtac::Function> function){
    bool optimized = false;
    
    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
                if(!(*ptr)->op && boost::get<int>(&(*ptr)->arg1)){
                    int value = boost::get<int>((*ptr)->arg1);

                    if(value == 0){
                        auto goto_ = std::make_shared<mtac::Goto>();

                        goto_->label = (*ptr)->label;
                        goto_->block = (*ptr)->block;

                        statement = goto_;
                        optimized = true;
                    } else if(value == 1){
                        statement = std::make_shared<mtac::NoOp>();
                        optimized = true;
                    }
                }
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
                if(!(*ptr)->op && boost::get<int>(&(*ptr)->arg1)){
                    int value = boost::get<int>((*ptr)->arg1);

                    if(value == 0){
                        statement = std::make_shared<mtac::NoOp>();
                        optimized = true;
                    } else if(value == 1){
                        auto goto_ = std::make_shared<mtac::Goto>();

                        goto_->label = (*ptr)->label;
                        goto_->block = (*ptr)->block;

                        statement = goto_;
                        optimized = true;
                    }
                }
            }
        }
    }

    return optimized;
}
