//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/Utils.hpp"

using namespace eddic;

void eddic::tac::computeBlockUsage(std::shared_ptr<tac::Function> function, std::unordered_set<std::shared_ptr<tac::BasicBlock>>& usage){
    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<tac::Goto>>(&statement)){
                usage.insert((*ptr)->block);
            } else if(auto* ptr = boost::get<std::shared_ptr<tac::IfFalse>>(&statement)){
                usage.insert((*ptr)->block);
            } else if(auto* ptr = boost::get<std::shared_ptr<tac::If>>(&statement)){
                usage.insert((*ptr)->block);
            }
        }
    }
}

bool eddic::tac::safe(std::shared_ptr<tac::Call> call){
    auto function = call->function;

    //These three functions are considered as safe because they save/restore all the registers and does not return anything 
    return function == "print_integer" || function == "print_string" || function == "print_line"; 
}
