//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <set>
#include <boost/range/adaptors.hpp>

#include "assert.hpp"
#include "FunctionContext.hpp"
#include "Type.hpp"
#include "Options.hpp"
#include "logging.hpp"
#include "GlobalContext.hpp"
#include "Variable.hpp"

#include "mtac/Statement.hpp"

#include "ltac/Utils.hpp"
#include "ltac/RegisterManager.hpp"
#include "ltac/StatementCompiler.hpp"
#include "ltac/Statement.hpp"
#include "ltac/Register.hpp"
#include "ltac/FloatRegister.hpp"
#include "ltac/PseudoRegister.hpp"
#include "ltac/PseudoFloatRegister.hpp"

#include "mtac/Utils.hpp"
#include "mtac/Printer.hpp"

using namespace eddic;

namespace {

template<typename Reg> 
Reg get_pseudo_reg(as::PseudoRegisters<Reg>& registers, std::shared_ptr<Variable> variable){
    //The variable is already in a register
    if(registers.inRegister(variable)){
        return registers[variable];
    }

    return registers.get_new_reg();
}

} //end of anonymous namespace
    
ltac::RegisterManager::RegisterManager(mtac::Function& function, std::shared_ptr<FloatPool> float_pool) : function(function), float_pool(float_pool){
        //Nothing else to init
}

void ltac::RegisterManager::reset(){
    written.clear();
    local.clear();
}

void ltac::RegisterManager::copy(mtac::Argument argument, ltac::PseudoFloatRegister reg){
    assert(ltac::is_variable(argument) || mtac::isFloat(argument));

    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
        auto variable = *ptr;
        
        //If the variable is hold in a register, just move the register value
        if(pseudo_float_registers.inRegister(variable)){
            auto old_reg = pseudo_float_registers[variable];
            ltac::add_instruction(bb, ltac::Operator::FMOV, reg, old_reg);
        } else {
            auto position = variable->position();

            assert(position.isStack() || position.isGlobal() || position.isParameter());

            if(position.isParameter() || position.isStack()){
                ltac::add_instruction(bb, ltac::Operator::FMOV, reg, ltac::Address(ltac::BP, position.offset()));
            } else if(position.isGlobal()){
                ltac::add_instruction(bb, ltac::Operator::FMOV, reg, ltac::Address("V" + position.name()));
            } 
        }
    } else {
        auto label = float_pool->label(boost::get<double>(argument));
        ltac::add_instruction(bb, ltac::Operator::FMOV, reg, ltac::Address(label));
    }
}

void ltac::RegisterManager::copy(mtac::Argument argument, ltac::PseudoRegister reg){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
        auto variable = *ptr;
        
        //If the variable is hold in a register, just move the register value
        if(pseudo_registers.inRegister(variable)){
            auto old_reg = pseudo_registers[variable];
            ltac::add_instruction(bb, ltac::Operator::MOV, reg, old_reg);
        } else {
            auto position = variable->position();

            assert(position.isStack() || position.isGlobal() || position.isParameter());

            if(position.isParameter() || position.isStack()){
                ltac::add_instruction(bb, ltac::Operator::MOV, reg, ltac::Address(ltac::BP, position.offset()));
            } else if(position.isGlobal()){
                ltac::add_instruction(bb, ltac::Operator::MOV, reg, ltac::Address("V" + position.name()));
            }
        }
    } else {
        //If it's a constant (int, double, string), just move it
        ltac::add_instruction(bb, ltac::Operator::MOV, reg, to_arg(argument, *this));
    }
}

void ltac::RegisterManager::move(mtac::Argument argument, ltac::PseudoRegister reg){
    copy(argument, reg);

    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
        //The variable is now held in the new register
        pseudo_registers.setLocation(*ptr, reg);
    }
}

void ltac::RegisterManager::move(mtac::Argument argument, ltac::PseudoFloatRegister reg){
    assert(ltac::is_variable(argument) || mtac::isFloat(argument));
    
    copy(argument, reg);

    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
        //The variable is now held in the new register
        pseudo_float_registers.setLocation(*ptr, reg);
    } 
}

bool is_local(std::shared_ptr<Variable> var, ltac::RegisterManager& manager){
    return var->position().isParameter() || (manager.is_escaped(var) && !var->position().isParamRegister()) || var->position().isStack();
}
        
ltac::PseudoRegister ltac::RegisterManager::get_pseudo_reg(std::shared_ptr<Variable> var){
    auto reg = ::get_pseudo_reg(pseudo_registers, var);
    move(var, reg);
    LOG<Trace>("Registers") << "Get pseudo reg for " << var->name() << " => " << reg << log::endl;

    if(is_local(var, *this)){
        local.insert(var);
    }

    return reg;
}

ltac::PseudoRegister ltac::RegisterManager::get_pseudo_reg_no_move(std::shared_ptr<Variable> var){
    auto reg = ::get_pseudo_reg(pseudo_registers, var);
    pseudo_registers.setLocation(var, reg);
    
    LOG<Trace>("Registers") << "Get pseudo reg for " << var->name() << " => " << reg << log::endl;

    if(is_local(var, *this)){
        local.insert(var);
    }

    return reg;
}

ltac::PseudoFloatRegister ltac::RegisterManager::get_pseudo_float_reg(std::shared_ptr<Variable> var){
    auto reg = ::get_pseudo_reg(pseudo_float_registers, var);
    move(var, reg);
    LOG<Trace>("Registers") << "Get pseudo reg for " << var->name() << " => " << reg << log::endl;

    if(is_local(var, *this)){
        local.insert(var);
    }

    return reg;
}

ltac::PseudoFloatRegister ltac::RegisterManager::get_pseudo_float_reg_no_move(std::shared_ptr<Variable> var){
    auto reg = ::get_pseudo_reg(pseudo_float_registers, var);
    pseudo_float_registers.setLocation(var, reg);
    
    LOG<Trace>("Registers") << "Get pseudo reg for " << var->name() << " => " << reg << log::endl;

    if(is_local(var, *this)){
        local.insert(var);
    }

    return reg;
}

ltac::PseudoRegister ltac::RegisterManager::get_bound_pseudo_reg(unsigned short hard){
    return pseudo_registers.get_bound_reg(hard);
}

ltac::PseudoFloatRegister ltac::RegisterManager::get_bound_pseudo_float_reg(unsigned short hard){
    return pseudo_float_registers.get_bound_reg(hard);
}

ltac::PseudoRegister ltac::RegisterManager::get_free_pseudo_reg(){
    return pseudo_registers.get_new_reg();
}

ltac::PseudoFloatRegister ltac::RegisterManager::get_free_pseudo_float_reg(){
    return pseudo_float_registers.get_new_reg();
}

bool ltac::RegisterManager::is_escaped(std::shared_ptr<Variable> variable){
    if(pointer_escaped->count(variable)){
        LOG<Trace>("Registers") << variable->name() << " is escaped " << log::endl;

        return true;
    }

    LOG<Trace>("Registers") << variable->name() << " is not escaped " << log::endl;

    return false;
}
    
void ltac::RegisterManager::collect_parameters(eddic::Function& definition, const PlatformDescriptor* descriptor){
    for(auto& parameter : definition.parameters()){
        auto param = definition.context()->getVariable(parameter.name());

        if(param->position().isParamRegister()){
            if(mtac::is_single_int_register(param->type())){
                auto reg = get_bound_pseudo_reg(descriptor->int_param_register(param->position().offset()));
                pseudo_registers.setLocation(param, reg);
            } else if(mtac::is_single_float_register(param->type())){
                auto reg = get_bound_pseudo_float_reg(descriptor->float_param_register(param->position().offset()));
                pseudo_float_registers.setLocation(param, reg);
            }
        }
    }
}

bool ltac::RegisterManager::is_written(std::shared_ptr<Variable> variable){
    return written.find(variable) != written.end();
}

void ltac::RegisterManager::set_written(std::shared_ptr<Variable> variable){
    written.insert(variable);
}

int ltac::RegisterManager::last_pseudo_reg(){
    return pseudo_registers.last_reg();
}
 
int ltac::RegisterManager::last_float_pseudo_reg(){
    return pseudo_float_registers.last_reg();
}

void ltac::RegisterManager::remove_from_pseudo_reg(std::shared_ptr<Variable> variable){
    return pseudo_registers.remove_from_reg(variable);
}

void ltac::RegisterManager::remove_from_pseudo_float_reg(std::shared_ptr<Variable> variable){
    return pseudo_float_registers.remove_from_reg(variable);
}
