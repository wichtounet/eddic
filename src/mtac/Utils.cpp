//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/Utils.hpp"

using namespace eddic;

void eddic::mtac::computeBlockUsage(std::shared_ptr<mtac::Function> function, std::unordered_set<std::shared_ptr<mtac::BasicBlock>>& usage){
    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Goto>>(&statement)){
                usage.insert((*ptr)->block);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
                usage.insert((*ptr)->block);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
                usage.insert((*ptr)->block);
            }
        }
    }
}

bool eddic::mtac::safe(const std::string& function){
    //These functions are considered as safe because they save/restore all the registers and does not return anything 
    return 
        function == "_F5printB" || function == "_F5printI" || function == "_F5printF" || function == "_F5printS" ||
        function == "_F7printlnB" || function == "_F7printlnI" || function == "_F7printlnF" || function == "_F7printlnS" || 
        function == "_F7println"; 
}

bool eddic::mtac::safe(std::shared_ptr<mtac::Call> call){
    auto function = call->function;

    return safe(function);
}

bool eddic::mtac::erase_result(mtac::Operator op){
   return 
            op != mtac::Operator::DOT_ASSIGN 
        &&  op != mtac::Operator::DOT_FASSIGN 
        &&  op != mtac::Operator::DOT_PASSIGN 
        &&  op != mtac::Operator::RETURN; 
}

bool eddic::mtac::is_distributive(mtac::Operator op){
    return op == mtac::Operator::ADD || op == mtac::Operator::FADD || op == mtac::Operator::MUL || op == mtac::Operator::FMUL;
}

bool eddic::mtac::is_expression(mtac::Operator op){
    return op >= mtac::Operator::ADD && op <= mtac::Operator::FDIV;
}

