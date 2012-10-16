//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"

#include "ltac/Utils.hpp"

#include "mtac/Utils.hpp"

using namespace eddic;

bool eddic::ltac::is_float_operator(mtac::BinaryOperator op){
    return op >= mtac::BinaryOperator::FE && op <= mtac::BinaryOperator::FL;
}

bool eddic::ltac::is_float_var(std::shared_ptr<Variable> variable){
    return variable->type() == FLOAT;
}

bool eddic::ltac::is_int_var(std::shared_ptr<Variable> variable){
    return variable->type() == INT;
}

std::shared_ptr<ltac::Instruction> eddic::ltac::add_instruction(std::shared_ptr<ltac::Function> function, ltac::Operator op){
    auto instruction = std::make_shared<ltac::Instruction>(op);
    function->add(instruction);
    return instruction;
}

std::shared_ptr<ltac::Instruction> eddic::ltac::add_instruction(std::shared_ptr<ltac::Function> function, ltac::Operator op, ltac::Argument arg1){
    auto instruction = std::make_shared<ltac::Instruction>(op, arg1);
    function->add(instruction);
    return instruction;
}

std::shared_ptr<ltac::Instruction> eddic::ltac::add_instruction(std::shared_ptr<ltac::Function> function, ltac::Operator op, ltac::Argument arg1, ltac::Argument arg2){
    auto instruction = std::make_shared<ltac::Instruction>(op, arg1, arg2);
    function->add(instruction);
    return instruction;
}

std::shared_ptr<ltac::Instruction> eddic::ltac::add_instruction(std::shared_ptr<ltac::Function> function, ltac::Operator op, ltac::Argument arg1, ltac::Argument arg2, ltac::Argument arg3){
    auto instruction = std::make_shared<ltac::Instruction>(op, arg1, arg2, arg3);
    function->add(instruction);
    return instruction;
}

ltac::Register eddic::ltac::to_register(std::shared_ptr<Variable> var, ltac::RegisterManager& manager){
    if(var->position().is_temporary()){
        return manager.get_reg_no_move(var);
    } else {
        return manager.get_reg(var);
    }
}

namespace {

struct ToArgVisitor : public boost::static_visitor<ltac::Argument> {
    ltac::RegisterManager& manager;

    ToArgVisitor(ltac::RegisterManager& manager) : manager(manager) {}

    ltac::Argument operator()(int& arg) const {
        return arg;
    }

    ltac::Argument operator()(double& arg) const {
        return arg;
    }

    ltac::Argument operator()(std::string& arg) const {
        return arg;
    }

    ltac::Argument operator()(std::shared_ptr<Variable> variable) const {
        if(ltac::is_float_var(variable)){
            if(variable->position().is_temporary()){
                return manager.get_float_reg_no_move(variable);
            } else {
                return manager.get_float_reg(variable);
            }
        } else {
            return to_register(variable, manager);
        }
    }

    template<typename T>
    ltac::Argument operator()(T&) const {
        ASSERT_PATH_NOT_TAKEN("Unhandled arg type");
    }
};

} //end of anonymous

ltac::Argument eddic::ltac::to_arg(mtac::Argument argument, ltac::RegisterManager& manager){
    return visit(ToArgVisitor(manager), argument);
}
