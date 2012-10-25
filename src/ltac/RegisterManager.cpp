//=======================================================================
// Copyright Baptiste Wicht 2011.
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

#include "ltac/Utils.hpp"
#include "ltac/RegisterManager.hpp"
#include "ltac/StatementCompiler.hpp"

#include "mtac/Utils.hpp"
#include "mtac/Printer.hpp"

using namespace eddic;

namespace {

template<typename Reg>
Reg get_free_reg(as::Registers<Reg>& registers, ltac::RegisterManager& manager){
    //Try to get a free register 
    for(Reg reg : registers){
        if(!registers.used(reg)){
            return reg;
        } 
        
        if(!registers.reserved(reg) && !manager.is_live(registers[reg]) && !registers[reg]->position().isParamRegister() && !registers[reg]->position().is_register()){
            registers.remove(registers[reg]);

            return reg;
        }
    }

    //There are no free register, take one
    Reg reg = registers.first();
    bool found = false;

    //First, try to take a register that doesn't need to be spilled (variable has not modified)
    for(Reg remaining : registers){
        if(!registers.reserved(remaining) && !registers[remaining]->position().is_temporary() && !registers[remaining]->position().isParamRegister() && !registers[remaining]->position().is_register()){
            if(!manager.is_written(registers[remaining])){
                reg = remaining;
                found = true;
                break;
            }
        }
    }

    //If there is no registers that doesn't need to be spilled, take the first one not reserved 
    if(!found){
        for(Reg remaining : registers){
            if(!registers.reserved(remaining) && !registers[remaining]->position().is_temporary() && !registers[remaining]->position().isParamRegister() && !registers[remaining]->position().is_register()){
                reg = remaining;
                found = true;
                break;
            }
        }
    }
    
    if(!found){
        for(Reg r : registers){
            std::cout << "Register " << r << log::endl;
            if(!registers.reserved(r)){
                if(registers.used(r)){
                    std::cout << "  used by " << registers[r]->name() << log::endl;
                } else {
                    std::cout << "  not used" << log::endl;
                }
            } else {
                std::cout << "  reserved" << log::endl;
            }
        }

        ASSERT_PATH_NOT_TAKEN("No register found");
    }

    manager.spills(reg);

    return reg; 
}

template<typename Reg> 
Reg get_reg(as::Registers<Reg>& registers, std::shared_ptr<Variable> variable, bool doMove, ltac::RegisterManager& manager){
    //The variable is already in a register
    if(registers.inRegister(variable)){
        return registers[variable];
    }

    Reg reg = get_free_reg(registers, manager);

    log::emit<Trace>("Registers") << "Found reg " << reg << log::endl;

    if(doMove){
        manager.move(variable, reg);
    }

    registers.setLocation(variable, reg);

    return reg;
}

template<typename Reg> 
Reg get_pseudo_reg(as::PseudoRegisters<Reg>& registers, std::shared_ptr<Variable> variable){
    //The variable is already in a register
    if(registers.inRegister(variable)){
        return registers[variable];
    }

    return registers.get_new_reg();
}
    
template<typename Reg>
void spills(as::Registers<Reg>& registers, Reg reg, ltac::Operator mov, ltac::RegisterManager& manager){
    //If the register is not used, there is nothing to spills
    if(registers.used(reg)){
        auto variable = registers[reg];

        //Do no spills variable stored in register
        if(variable->position().isParamRegister() || variable->position().is_register()){
            return;
        }

        //If the variable has not been written, there is no need to spill it
        if(manager.written.find(variable) != manager.written.end()){
            auto position = variable->position();
            if(position.isStack() || position.isParameter()){
                ltac::add_instruction(manager.access_compiler()->bb, mov, ltac::Address(ltac::BP, position.offset()), reg);
            } else if(position.isGlobal()){
                ltac::add_instruction(manager.access_compiler()->bb, mov, ltac::Address("V" + position.name()), reg);
            } else if(position.is_temporary()){
                //If the variable is live, move it to another register, else do nothing
                if(manager.is_live(variable)){
                    registers.remove(variable);
                    manager.reserve(reg);

                    auto newReg = get_reg(registers, variable, false, manager);
                    ltac::add_instruction(manager.access_compiler()->bb, mov, newReg, reg);

                    manager.release(reg);

                    return; //Return here to avoid erasing variable from variables
                }
            }
        }

        //The variable is no more contained in the register
        registers.remove(variable);

        //The variable has not been written now
        manager.written.erase(variable);
    }
}

} //end of anonymous namespace
    
ltac::RegisterManager::RegisterManager(const std::vector<ltac::Register>& registers, const std::vector<ltac::FloatRegister>& float_registers, 
        std::shared_ptr<mtac::Function> function, std::shared_ptr<FloatPool> float_pool) : 
    AbstractRegisterManager(registers, float_registers), function(function), float_pool(float_pool) {
        //Nothing else to init
}

void ltac::RegisterManager::reset(){
    registers.reset();
    float_registers.reset();

    written.clear();
}

bool ltac::RegisterManager::in_reg(std::shared_ptr<Variable> var){
    return registers.inRegister(var);
}

void ltac::RegisterManager::copy(mtac::Argument argument, ltac::PseudoFloatRegister reg){
    assert(ltac::is_variable(argument) || mtac::isFloat(argument));

    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
        auto variable = *ptr;
        
        //If the variable is hold in a register, just move the register value
        if(pseudo_float_registers.inRegister(variable)){
            auto old_reg = pseudo_float_registers[variable];
            ltac::add_instruction(access_compiler()->bb, ltac::Operator::FMOV, reg, old_reg);
        } else {
            auto position = variable->position();

            assert(position.isGlobal() || position.isParameter());

            if(position.isParameter()){
                ltac::add_instruction(access_compiler()->bb, ltac::Operator::FMOV, reg, ltac::Address(ltac::BP, position.offset()));
            } else if(position.isGlobal()){
                ltac::add_instruction(access_compiler()->bb, ltac::Operator::FMOV, reg, ltac::Address("V" + position.name()));
            } 
        }
    } else {
        auto label = float_pool->label(boost::get<double>(argument));
        ltac::add_instruction(access_compiler()->bb, ltac::Operator::FMOV, reg, ltac::Address(label));
    }
}

void ltac::RegisterManager::copy(mtac::Argument argument, ltac::PseudoRegister reg){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
        auto variable = *ptr;
        
        //If the variable is hold in a register, just move the register value
        if(pseudo_registers.inRegister(variable)){
            auto old_reg = pseudo_registers[variable];
            ltac::add_instruction(access_compiler()->bb, ltac::Operator::MOV, reg, old_reg);
        } else {
            auto position = variable->position();

            assert(position.isGlobal() || position.isParameter());

            if(position.isParameter()){
                ltac::add_instruction(access_compiler()->bb, ltac::Operator::MOV, reg, ltac::Address(ltac::BP, position.offset()));
            } else if(position.isGlobal()){
                ltac::add_instruction(access_compiler()->bb, ltac::Operator::MOV, reg, ltac::Address("V" + position.name()));
            } 
        }
    } else {
        //If it's a constant (int, double, string), just move it
        ltac::add_instruction(access_compiler()->bb, ltac::Operator::MOV, reg, to_arg(argument, *this));
    }
}

void ltac::RegisterManager::copy(mtac::Argument argument, ltac::FloatRegister reg){
    assert(ltac::is_variable(argument) || mtac::isFloat(argument));

    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
        auto variable = *ptr;

        log::emit<Trace>("Registers") << "Copy " << variable->name() << log::endl;

        //If the variable is hold in a register, just move the register value
        if(float_registers.inRegister(variable)){
            auto oldReg = float_registers[variable];

            ltac::add_instruction(access_compiler()->bb, ltac::Operator::FMOV, reg, oldReg);
        } else {
            auto position = variable->position();

            //The temporary should have been handled by the preceding condition (hold in a register)
            assert(!position.is_temporary());

            if(position.isStack() || position.isParameter()){
                ltac::add_instruction(access_compiler()->bb, ltac::Operator::FMOV, reg, ltac::Address(ltac::BP, position.offset()));
            } else if(position.isGlobal()){
                ltac::add_instruction(access_compiler()->bb, ltac::Operator::FMOV, reg, ltac::Address("V" + position.name()));
            } 
        }
    } else if(auto* ptr = boost::get<double>(&argument)){
        auto label = float_pool->label(*ptr);
        ltac::add_instruction(access_compiler()->bb, ltac::Operator::FMOV, reg, ltac::Address(label));
    }
}

void ltac::RegisterManager::copy(mtac::Argument argument, ltac::Register reg){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
        auto variable = *ptr;

        //If the variable is hold in a register, just move the register value
        if(registers.inRegister(variable)){
            auto oldReg = registers[variable];

            ltac::add_instruction(access_compiler()->bb, ltac::Operator::MOV, reg, oldReg);
        } else {
            auto position = variable->position();

            //The temporary should have been handled by the preceding condition (hold in a register)
            assert(!position.is_temporary());

            if(position.isStack() || position.isParameter()){
                ltac::add_instruction(access_compiler()->bb, ltac::Operator::MOV, reg, ltac::Address(ltac::BP, position.offset()));
            } else if(position.isGlobal()){
                ltac::add_instruction(access_compiler()->bb, ltac::Operator::MOV, reg, ltac::Address("V" + position.name()));
            } 
        } 
    } else {
        //If it's a constant (int, double, string), just move it
        ltac::add_instruction(access_compiler()->bb, ltac::Operator::MOV, reg, to_arg(argument, *this));
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

void ltac::RegisterManager::move(mtac::Argument argument, ltac::Register reg){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
        auto variable = *ptr;

        //If the variable is hold in a register, just move the register value
        if(registers.inRegister(variable)){
            auto oldReg = registers[variable];

            //Only if the variable is not already on the same register 
            if(oldReg != reg){
                ltac::add_instruction(access_compiler()->bb, ltac::Operator::MOV, reg, oldReg);

                //There is nothing more in the old register
                registers.remove(variable);
            }
        } else {
            auto position = variable->position();

            //The temporary should have been handled by the preceding condition (hold in a register)
            assert(!position.is_temporary());

            if(position.isStack() || position.isParameter()){
                ltac::add_instruction(access_compiler()->bb, ltac::Operator::MOV, reg, ltac::Address(ltac::BP, position.offset()));
            } else if(position.isGlobal()){
                ltac::add_instruction(access_compiler()->bb, ltac::Operator::MOV, reg, ltac::Address("V" + position.name()));
            } 
        } 

        //The variable is now held in the new register
        registers.setLocation(variable, reg);
    } else {
        //If it's a constant (int, double, string), just move it
        ltac::add_instruction(access_compiler()->bb, ltac::Operator::MOV, reg, to_arg(argument, *this));
    }
}

void ltac::RegisterManager::move(mtac::Argument argument, ltac::FloatRegister reg){
    assert(ltac::is_variable(argument) || mtac::isFloat(argument));

    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
        auto variable = *ptr;

        //If the variable is hold in a register, just move the register value
        if(float_registers.inRegister(variable)){
            auto oldReg = float_registers[variable];

            //Only if the variable is not already on the same register 
            if(oldReg != reg){
                ltac::add_instruction(access_compiler()->bb, ltac::Operator::FMOV, reg, oldReg);

                //There is nothing more in the old register
                float_registers.remove(variable);
            }
        } else {
            auto position = variable->position();

            //The temporary should have been handled by the preceding condition (hold in a register)
            assert(!position.is_temporary());

            if(position.isStack() || position.isParameter()){
                ltac::add_instruction(access_compiler()->bb, ltac::Operator::FMOV, reg, ltac::Address(ltac::BP, position.offset()));
            } else if(position.isGlobal()){
                ltac::add_instruction(access_compiler()->bb, ltac::Operator::FMOV, reg, ltac::Address("V" + position.name()));
            } 
        }

        //The variable is now held in the new register
        float_registers.setLocation(variable, reg);
    } else if(auto* ptr = boost::get<double>(&argument)){
        auto label = float_pool->label(*ptr);
        ltac::add_instruction(access_compiler()->bb, ltac::Operator::FMOV, reg, ltac::Address(label));
    }
}
        
ltac::PseudoRegister ltac::RegisterManager::get_pseudo_reg(std::shared_ptr<Variable> var){
    auto reg = ::get_pseudo_reg(pseudo_registers, var);
    move(var, reg);
    log::emit<Trace>("Registers") << "Get pseudo reg for " << var->name() << " => " << reg << log::endl;
    return reg;
}

ltac::PseudoRegister ltac::RegisterManager::get_pseudo_reg_no_move(std::shared_ptr<Variable> var){
    auto reg = ::get_pseudo_reg(pseudo_registers, var);
    pseudo_registers.setLocation(var, reg);
    log::emit<Trace>("Registers") << "Get pseudo reg for " << var->name() << " => " << reg << log::endl;
    return reg;
}

ltac::PseudoFloatRegister ltac::RegisterManager::get_pseudo_float_reg(std::shared_ptr<Variable> var){
    auto reg = ::get_pseudo_reg(pseudo_float_registers, var);
    move(var, reg);
    log::emit<Trace>("Registers") << "Get pseudo reg for " << var->name() << " => " << reg << log::endl;
    return reg;
}

ltac::PseudoFloatRegister ltac::RegisterManager::get_pseudo_float_reg_no_move(std::shared_ptr<Variable> var){
    auto reg = ::get_pseudo_reg(pseudo_float_registers, var);
    pseudo_float_registers.setLocation(var, reg);
    log::emit<Trace>("Registers") << "Get pseudo reg for " << var->name() << " => " << reg << log::endl;
    return reg;
}

ltac::PseudoRegister ltac::RegisterManager::get_bound_pseudo_reg(unsigned short hard){
    return pseudo_registers.get_bound_reg(hard);
}

ltac::PseudoFloatRegister ltac::RegisterManager::get_bound_pseudo_float_reg(unsigned short hard){
    return pseudo_float_registers.get_bound_reg(hard);
}

void ltac::RegisterManager::spills(ltac::Register reg){
    log::emit<Trace>("Registers") << "Spills Register " << reg << log::endl;
    ::spills(registers, reg, ltac::Operator::MOV, *this);
}

void ltac::RegisterManager::spills(ltac::FloatRegister reg){
    log::emit<Trace>("Registers") << "Spills Float Register " << reg << log::endl;
    ::spills(float_registers, reg, ltac::Operator::FMOV, *this);
}

ltac::PseudoRegister ltac::RegisterManager::get_free_pseudo_reg(){
    return pseudo_registers.get_new_reg();
}

ltac::PseudoFloatRegister ltac::RegisterManager::get_free_pseudo_float_reg(){
    return pseudo_float_registers.get_new_reg();
}

ltac::Register ltac::RegisterManager::get_free_reg(){
    log::emit<Trace>("Registers") << "Get a free reg" << log::endl;
    auto reg = ::get_free_reg(registers, *this);
    reserve(reg);
    return reg;
}

ltac::FloatRegister ltac::RegisterManager::get_free_float_reg(){
    log::emit<Trace>("Registers") << "Get a free float reg" << log::endl;
    auto reg = ::get_free_reg(float_registers, *this);
    reserve(reg);
    return reg;
}

bool ltac::RegisterManager::is_live(std::shared_ptr<Variable> variable, mtac::Statement statement){
    if(liveness->IN_S[statement].values().find(variable) != liveness->IN_S[statement].values().end()){
        return true;
    } else if(liveness->OUT_S[statement].values().find(variable) != liveness->OUT_S[statement].values().end()){
        return true;
    } else {
        return false;
    }
}

bool ltac::RegisterManager::is_escaped(std::shared_ptr<Variable> variable){
    if(pointer_escaped->count(variable)){
        log::emit<Trace>("Registers") << variable->name() << " is escaped " << log::endl;

        return true;
    }

    log::emit<Trace>("Registers") << variable->name() << " is not escaped " << log::endl;

    return false;
}

bool ltac::RegisterManager::is_live(std::shared_ptr<Variable> variable){
    auto live = is_live(variable, current);
    log::emit<Trace>("Registers") << variable->name() << " is live " << live << log::endl;
    return live;
}
    
void ltac::RegisterManager::collect_parameters(std::shared_ptr<eddic::Function> definition, const PlatformDescriptor* descriptor){
    for(auto parameter : definition->parameters){
        auto param = definition->context->getVariable(parameter.name);

        if(param->position().isParamRegister()){
            if(mtac::is_single_int_register(param->type())){
                registers.setLocation(param, ltac::Register(descriptor->int_param_register(param->position().offset())));
            } else if(mtac::is_single_float_register(param->type())){
                float_registers.setLocation(param, ltac::FloatRegister(descriptor->float_param_register(param->position().offset())));
            }
        }
    }
}

void ltac::RegisterManager::restore_pushed_registers(){
    //Restore the int parameters in registers (in the reverse order they were pushed)
    for(auto& reg : boost::adaptors::reverse(int_pushed)){
        access_compiler()->pop(reg);
    }

    //Restore the float parameters in registers (in the reverse order they were pushed)
    for(auto& reg : boost::adaptors::reverse(float_pushed)){
        ltac::add_instruction(access_compiler()->bb, ltac::Operator::FMOV, reg, ltac::Address(ltac::SP, 0));
        ltac::add_instruction(access_compiler()->bb, ltac::Operator::ADD, ltac::SP, static_cast<int>(FLOAT->size(function->context->global()->target_platform())));
    }

    //Each register has been restored
    int_pushed.clear();
    float_pushed.clear();

    //All the parameters have been handled by now, the next param will be the first for its call
    first_param = true;
}

void ltac::RegisterManager::save_registers(std::shared_ptr<mtac::Param> param, const PlatformDescriptor* descriptor){
    if(first_param){
        if(param->function){
            std::set<ltac::Register> overriden_registers;
            std::set<ltac::FloatRegister> overriden_float_registers;
    
            if(param->function->standard || configuration->option_defined("fparameter-allocation")){
                unsigned int maxInt = descriptor->numberOfIntParamRegisters();
                unsigned int maxFloat = descriptor->numberOfFloatParamRegisters();

                for(auto& parameter : param->function->parameters){
                    auto type = param->function->getParameterType(parameter.name);
                    unsigned int position = param->function->getParameterPositionByType(parameter.name);

                    if(mtac::is_single_int_register(type) && position <= maxInt){
                        overriden_registers.insert(ltac::Register(descriptor->int_param_register(position)));
                    }

                    if(mtac::is_single_float_register(type) && position <= maxFloat){
                        overriden_float_registers.insert(ltac::FloatRegister(descriptor->float_param_register(position)));
                    }
                }
            }

            for(auto& reg : overriden_registers){
                //If the parameter register is already used by a variable or a parent parameter
                if(!registers.reserved(reg) && registers.used(reg)){
                    if(registers[reg]->position().isParamRegister() || registers[reg]->position().is_register()){
                        int_pushed.push_back(reg);
                        access_compiler()->push(reg);
                    } else {
                        spills(reg);
                    }
                }
            }
            
            for(auto& reg : overriden_float_registers){
                //If the parameter register is already used by a variable or a parent parameter
                if(float_registers.used(reg)){
                    if(float_registers[reg]->position().isParamRegister() || float_registers[reg]->position().is_register()){
                        float_pushed.push_back(reg);

                        ltac::add_instruction(access_compiler()->bb, ltac::Operator::SUB, ltac::SP, static_cast<int>(FLOAT->size(function->context->global()->target_platform())));
                        ltac::add_instruction(access_compiler()->bb, ltac::Operator::FMOV, ltac::Address(ltac::SP, 0), reg);
                    } else {
                        spills(reg);
                    }
                }
            }
        }

        //The following parameters are for the same call
        first_param = false;
    }
}

void ltac::RegisterManager::set_current(mtac::Statement statement){
    current = statement;

    log::emit<Trace>("Registers") << "Current statement " << statement << log::endl;
}
        
bool ltac::RegisterManager::is_written(std::shared_ptr<Variable> variable){
    return written.find(variable) != written.end();
}

void ltac::RegisterManager::set_written(std::shared_ptr<Variable> variable){
    written.insert(variable);
}

std::shared_ptr<ltac::StatementCompiler> ltac::RegisterManager::access_compiler(){
    if(auto ptr = compiler.lock()){
        return ptr;
    }

    ASSERT_PATH_NOT_TAKEN("The shared_ptr on StatementCompiler has expired");
}

int ltac::RegisterManager::last_pseudo_reg(){
    return pseudo_registers.last_reg();
}
