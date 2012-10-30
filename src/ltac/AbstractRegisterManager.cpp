//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Type.hpp"
#include "logging.hpp"
#include "Variable.hpp"

#include "mtac/Utils.hpp"
#include "mtac/Argument.hpp"

#include "ltac/Utils.hpp"
#include "ltac/RegisterManager.hpp"
#include "ltac/StatementCompiler.hpp"

using namespace eddic;

ltac::AbstractRegisterManager::AbstractRegisterManager(const std::vector<ltac::Register>& registers, const std::vector<ltac::FloatRegister>& float_registers) : 
    registers(registers, std::make_shared<Variable>("__fake_int__", INT, Position(PositionType::TEMPORARY))),
    float_registers(float_registers, std::make_shared<Variable>("__fake_float__", FLOAT, Position(PositionType::TEMPORARY))){
        //Nothing else to init
}

bool ltac::AbstractRegisterManager::in_register(std::shared_ptr<Variable> variable, ltac::Register reg){
    return registers.inRegister(variable, reg);
}

bool ltac::AbstractRegisterManager::in_register(std::shared_ptr<Variable> variable, ltac::FloatRegister reg){
    return float_registers.inRegister(variable, reg);
}
        
void ltac::AbstractRegisterManager::setLocation(std::shared_ptr<Variable> variable, ltac::Register reg){
   registers.setLocation(variable, reg); 
}

void ltac::AbstractRegisterManager::setLocation(std::shared_ptr<Variable> variable, ltac::FloatRegister reg){
   float_registers.setLocation(variable, reg); 
}

bool ltac::AbstractRegisterManager::is_reserved(ltac::Register reg){
    return registers.reserved(reg);
}

bool ltac::AbstractRegisterManager::is_reserved(ltac::FloatRegister reg){
    return float_registers.reserved(reg);
}

void ltac::AbstractRegisterManager::reserve(ltac::Register reg){
    registers.reserve(reg);
    log::emit<Trace>("Registers") << "Int Register " << reg  << " reserved" << log::endl;
}

void ltac::AbstractRegisterManager::release(ltac::Register reg){
    registers.release(reg);
    log::emit<Trace>("Registers") << "Int Register " << reg  << " released" << log::endl;
}

void ltac::AbstractRegisterManager::reserve(ltac::FloatRegister reg){
    float_registers.reserve(reg);
    log::emit<Trace>("Registers") << "Float Register " << reg  << " reserved" << log::endl;
}

void ltac::AbstractRegisterManager::release(ltac::FloatRegister reg){
    float_registers.release(reg);
    log::emit<Trace>("Registers") << "Float Register " << reg  << " released" << log::endl;
}
