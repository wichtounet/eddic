//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"
#include "Variable.hpp"

#include "mtac/Utils.hpp"

#include "ltac/Utils.hpp"
#include "ltac/RegisterManager.hpp"
#include "ltac/Instruction.hpp"

using namespace eddic;

bool eddic::ltac::is_float_var(std::shared_ptr<Variable> variable){
    return variable->type() == FLOAT;
}

bool eddic::ltac::is_int_var(std::shared_ptr<Variable> variable){
    return variable->type() == INT;
}

bool eddic::ltac::transform_to_nop(ltac::Instruction& instruction){
    if(instruction.op == ltac::Operator::NOP){
        return false;
    }
    
    instruction.op = ltac::Operator::NOP;
    instruction.arg1.reset();
    instruction.arg2.reset();
    instruction.arg3.reset();

    return true;
}

ltac::PseudoRegister eddic::ltac::to_register(std::shared_ptr<Variable> var, ltac::RegisterManager& manager){
    if(var->position().is_temporary()){
        return manager.get_pseudo_reg_no_move(var);
    } else {
        return manager.get_pseudo_reg(var);
    }
}

ltac::PseudoFloatRegister eddic::ltac::to_float_register(std::shared_ptr<Variable> var, ltac::RegisterManager& manager){
    if(var->position().is_temporary()){
        return manager.get_pseudo_float_reg_no_move(var);
    } else {
        return manager.get_pseudo_float_reg(var);
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
            return to_float_register(variable, manager);
        } else {
            return to_register(variable, manager);
        }
    }
};

} //end of anonymous

ltac::Argument eddic::ltac::to_arg(mtac::Argument argument, ltac::RegisterManager& manager){
    return visit(ToArgVisitor(manager), argument);
}
