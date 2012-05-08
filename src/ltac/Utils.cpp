//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"

#include "ltac/Utils.hpp"

using namespace eddic;

bool eddic::ltac::is_float_operator(mtac::BinaryOperator op){
    return op >= mtac::BinaryOperator::FE && op <= mtac::BinaryOperator::FL;
}

bool eddic::ltac::is_float_var(std::shared_ptr<Variable> variable){
    return variable->type() == BaseType::FLOAT;
}

bool eddic::ltac::is_int_var(std::shared_ptr<Variable> variable){
    return variable->type() == BaseType::INT;
}

void eddic::ltac::add_instruction(std::shared_ptr<ltac::Function> function, ltac::Operator op){
    function->add(std::make_shared<ltac::Instruction>(op));
}

void eddic::ltac::add_instruction(std::shared_ptr<ltac::Function> function, ltac::Operator op, ltac::Argument arg1){
    function->add(std::make_shared<ltac::Instruction>(op, arg1));
}

void eddic::ltac::add_instruction(std::shared_ptr<ltac::Function> function, ltac::Operator op, ltac::Argument arg1, ltac::Argument arg2){
    function->add(std::make_shared<ltac::Instruction>(op, arg1, arg2));
}

void eddic::ltac::add_instruction(std::shared_ptr<ltac::Function> function, ltac::Operator op, ltac::Argument arg1, ltac::Argument arg2, ltac::Argument arg3){
    function->add(std::make_shared<ltac::Instruction>(op, arg1, arg2, arg3));
}

ltac::Register eddic::ltac::to_register(std::shared_ptr<Variable> var, ltac::RegisterManager& manager){
    if(var->position().isTemporary()){
        return manager.get_reg_no_move(var);
    } else {
        return manager.get_reg(var);
    }
}

ltac::Argument eddic::ltac::to_arg(mtac::Argument argument, ltac::RegisterManager& manager){
    if(auto* ptr = boost::get<int>(&argument)){
        return *ptr;
    } else if(auto* ptr = boost::get<double>(&argument)){
        return manager.float_pool->label(*ptr);
    } else if(auto* ptr = boost::get<std::string>(&argument)){
        return *ptr;
    } else if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
        if(ltac::is_float_var(*ptr)){
            if((*ptr)->position().isTemporary()){
                return manager.get_float_reg_no_move(*ptr);
            } else {
                return manager.get_float_reg(*ptr);
            }
        } else {
            return to_register(*ptr, manager);
        }
    }

    ASSERT_PATH_NOT_TAKEN("Should never get there");
}
