//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "FunctionContext.hpp"
#include "Type.hpp"

#include "ltac/Utils.hpp"
#include "ltac/RegisterManager.hpp"

#include "mtac/Utils.hpp"
#include "mtac/Printer.hpp"

#define DEBUG_GLOBAL_ENABLED false
#define DEBUG_GLOBAL if(DEBUG_GLOBAL_ENABLED)

using namespace eddic;

namespace {

template<typename Reg>
Reg get_free_reg(as::Registers<Reg>& registers, ltac::RegisterManager& manager){
    //Try to get a free register 
    for(Reg reg : registers){
        if(!registers.used(reg)){
            return reg;
        } 
        
        if(!registers.reserved(reg) && !manager.is_live(registers[reg]) && !registers[reg]->position().isParamRegister()){
            registers.remove(registers[reg]);

            return reg;
        }
    }

    //There are no free register, take one
    Reg reg = registers.first();
    bool found = false;

    //First, try to take a register that doesn't need to be spilled (variable has not modified)
    for(Reg remaining : registers){
        if(!registers.reserved(remaining) && !registers[reg]->position().isParamRegister()){
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
            if(!registers.reserved(remaining) && !registers[reg]->position().isTemporary() && !registers[reg]->position().isParamRegister()){
                reg = remaining;
                found = true;
                break;
            }
        }
    }
    
    if(!found){
        for(Reg r : registers){
            std::cout << "Register " << r << std::endl;
            if(!registers.reserved(r)){
                if(registers.used(r)){
                    std::cout << "  used by " << registers[reg]->name() << std::endl;
                } else {
                    std::cout << "  not used" << std::endl;
                }
            } else {
                std::cout << "  reserved" << std::endl;
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

    if(doMove){
        manager.move(variable, reg);
    }

    registers.setLocation(variable, reg);

    return reg;
}
    
template<typename Reg>
void safe_move(as::Registers<Reg>& registers, std::shared_ptr<Variable> variable, Reg reg, ltac::RegisterManager& manager){
    if(registers.used(reg)){
        if(registers[reg] != variable){
            manager.spills(reg);

            manager.move(variable, reg);
        }
    } else {
        manager.move(variable, reg);
    }
}
    
template<typename Reg>
void spills(as::Registers<Reg>& registers, Reg reg, ltac::Operator mov, ltac::RegisterManager& manager){
    //If the register is not used, there is nothing to spills
    if(registers.used(reg)){
        auto variable = registers[reg];

        //Do no spills param stored in register
        if(variable->position().isParamRegister()){
            return;
        }

        //If the variable has not been written, there is no need to spill it
        if(manager.written.find(variable) != manager.written.end()){
            auto position = variable->position();
            if(position.isStack()){
                ltac::add_instruction(manager.function, mov, ltac::Address(ltac::BP, -1 * position.offset()), reg);
            } else if(position.isParameter()){
                ltac::add_instruction(manager.function, mov, ltac::Address(ltac::BP, position.offset()), reg);
            } else if(position.isGlobal()){
                ltac::add_instruction(manager.function, mov, ltac::Address("V" + position.name()), reg);
            } else if(position.isTemporary()){
                //If the variable is live, move it to another register, else do nothing
                if(manager.is_live(variable)){
                    registers.remove(variable);
                    manager.reserve(reg);

                    auto newReg = get_reg(registers, variable, false, manager);
                    ltac::add_instruction(manager.function, mov, newReg, reg);

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

template<typename Reg>
void spills_all(as::Registers<Reg>& registers, ltac::RegisterManager& manager){
    for(auto reg : registers){
        //The register can be reserved if the ending occurs in a special break case
        if(!registers.reserved(reg) && registers.used(reg)){
            auto variable = registers[reg];

            if(!variable->position().isTemporary()){
                manager.spills(reg);    
            }
        }
    }
}

} //end of anonymous namespace
    
ltac::RegisterManager::RegisterManager(std::vector<ltac::Register> registers, std::vector<ltac::FloatRegister> float_registers, 
        std::shared_ptr<ltac::Function> function, std::shared_ptr<FloatPool> float_pool) : 
    registers(registers, std::make_shared<Variable>("__fake_int__", INT, Position(PositionType::TEMPORARY))),
    float_registers(float_registers, std::make_shared<Variable>("__fake_float__", FLOAT, Position(PositionType::TEMPORARY))), 
    function(function), float_pool(float_pool) {
        //Nothing else to init
}

void ltac::RegisterManager::reset(){
    registers.reset();
    float_registers.reset();

    written.clear();
}

void ltac::RegisterManager::copy(mtac::Argument argument, ltac::FloatRegister reg){
    assert(ltac::is_variable(argument) || mtac::isFloat(argument));

    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
        auto variable = *ptr;

        DEBUG_GLOBAL std::cout << "Copy " << variable->name() << std::endl;

        //If the variable is hold in a register, just move the register value
        if(float_registers.inRegister(variable)){
            auto oldReg = float_registers[variable];

            ltac::add_instruction(function, ltac::Operator::FMOV, reg, oldReg);
        } else {
            auto position = variable->position();

            //The temporary should have been handled by the preceding condition (hold in a register)
            assert(!position.isTemporary());

            if(position.isStack()){
                ltac::add_instruction(function, ltac::Operator::FMOV, reg, ltac::Address(ltac::BP, -1 * position.offset()));
            } else if(position.isParameter()){
                ltac::add_instruction(function, ltac::Operator::FMOV, reg, ltac::Address(ltac::BP, position.offset()));
            } else if(position.isGlobal()){
                ltac::add_instruction(function, ltac::Operator::FMOV, reg, ltac::Address("V" + position.name()));
            } 
        }
    } else if(auto* ptr = boost::get<double>(&argument)){
        auto label = float_pool->label(*ptr);
        ltac::add_instruction(function, ltac::Operator::FMOV, reg, ltac::Address(label));
    }
}

void ltac::RegisterManager::copy(mtac::Argument argument, ltac::Register reg){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
        auto variable = *ptr;

        //If the variable is hold in a register, just move the register value
        if(registers.inRegister(variable)){
            auto oldReg = registers[variable];

            ltac::add_instruction(function, ltac::Operator::MOV, reg, oldReg);
        } else {
            auto position = variable->position();

            //The temporary should have been handled by the preceding condition (hold in a register)
            assert(!position.isTemporary());

            if(position.isStack()){
                ltac::add_instruction(function, ltac::Operator::MOV, reg, ltac::Address(ltac::BP, -1 * position.offset()));
            } else if(position.isParameter()){
                ltac::add_instruction(function, ltac::Operator::MOV, reg, ltac::Address(ltac::BP, position.offset()));
            } else if(position.isGlobal()){
                ltac::add_instruction(function, ltac::Operator::MOV, reg, ltac::Address("V" + position.name()));
            } 
        } 
    } else {
        //If it's a constant (int, double, string), just move it
        ltac::add_instruction(function, ltac::Operator::MOV, reg, to_arg(argument, *this));
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
                ltac::add_instruction(function, ltac::Operator::MOV, reg, oldReg);

                //There is nothing more in the old register
                registers.remove(variable);
            }
        } else {
            auto position = variable->position();

            //The temporary should have been handled by the preceding condition (hold in a register)
            assert(!position.isTemporary());

            if(position.isStack()){
                ltac::add_instruction(function, ltac::Operator::MOV, reg, ltac::Address(ltac::BP, -1 * position.offset()));
            } else if(position.isParameter()){
                ltac::add_instruction(function, ltac::Operator::MOV, reg, ltac::Address(ltac::BP, position.offset()));
            } else if(position.isGlobal()){
                ltac::add_instruction(function, ltac::Operator::MOV, reg, ltac::Address("V" + position.name()));
            } 
        } 

        //The variable is now held in the new register
        registers.setLocation(variable, reg);
    } else {
        //If it's a constant (int, double, string), just move it
        ltac::add_instruction(function, ltac::Operator::MOV, reg, to_arg(argument, *this));
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
                ltac::add_instruction(function, ltac::Operator::FMOV, reg, oldReg);

                //There is nothing more in the old register
                float_registers.remove(variable);
            }
        } else {
            auto position = variable->position();

            //The temporary should have been handled by the preceding condition (hold in a register)
            assert(!position.isTemporary());

            if(position.isStack()){
                ltac::add_instruction(function, ltac::Operator::FMOV, reg, ltac::Address(ltac::BP, -1 * position.offset()));
            } else if(position.isParameter()){
                ltac::add_instruction(function, ltac::Operator::FMOV, reg, ltac::Address(ltac::BP, position.offset()));
            } else if(position.isGlobal()){
                ltac::add_instruction(function, ltac::Operator::FMOV, reg, ltac::Address("V" + position.name()));
            } 
        }

        //The variable is now held in the new register
        float_registers.setLocation(variable, reg);
    } else if(auto* ptr = boost::get<double>(&argument)){
        auto label = float_pool->label(*ptr);
        ltac::add_instruction(function, ltac::Operator::FMOV, reg, ltac::Address(label));
    }
}

ltac::Register ltac::RegisterManager::get_reg(std::shared_ptr<Variable> var){
    auto reg = ::get_reg(registers, var, true, *this);
    DEBUG_GLOBAL std::cout << "Get reg for " << var->name() << " : " << reg << std::endl;
    return reg;
}

ltac::Register ltac::RegisterManager::get_reg_no_move(std::shared_ptr<Variable> var){
    auto reg = ::get_reg(registers, var, false, *this);
    DEBUG_GLOBAL std::cout << "Get reg for " << var->name() << " : " << reg << std::endl;
    return reg;
}

ltac::FloatRegister ltac::RegisterManager::get_float_reg(std::shared_ptr<Variable> var){
    auto reg = ::get_reg(float_registers, var, true, *this);
    DEBUG_GLOBAL std::cout << "Get reg for " << var->name() << " : " << reg << std::endl;
    return reg;
}

ltac::FloatRegister ltac::RegisterManager::get_float_reg_no_move(std::shared_ptr<Variable> var){
    auto reg = ::get_reg(float_registers, var, false, *this);
    DEBUG_GLOBAL std::cout << "Get reg for " << var->name() << " : " << reg << std::endl;
    return reg;
}

void ltac::RegisterManager::safe_move(std::shared_ptr<Variable> variable, ltac::Register reg){
    DEBUG_GLOBAL std::cout << "Safe move " << variable->name() << " in " << reg << std::endl;
    return ::safe_move(registers, variable, reg, *this);
}

void ltac::RegisterManager::safe_move(std::shared_ptr<Variable> variable, ltac::FloatRegister reg){
    DEBUG_GLOBAL std::cout << "Safe move " << variable->name() << " in " << reg << std::endl;
    return ::safe_move(float_registers, variable, reg, *this);
}

void ltac::RegisterManager::spills(ltac::Register reg){
    DEBUG_GLOBAL std::cout << "Spills Register " << reg << std::endl;
    ::spills(registers, reg, ltac::Operator::MOV, *this);
}

void ltac::RegisterManager::spills(ltac::FloatRegister reg){
    DEBUG_GLOBAL std::cout << "Spills Float Register " << reg << std::endl;
    ::spills(float_registers, reg, ltac::Operator::FMOV, *this);
}

void ltac::RegisterManager::spills_if_necessary(ltac::Register reg, mtac::Argument arg){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&arg)){
        if(!registers.inRegister(*ptr, reg)){
            spills(reg);
        }
    } else {
        spills(reg);
    }
}

void ltac::RegisterManager::spills_all(){
    ::spills_all(registers, *this);
    ::spills_all(float_registers, *this);
}

ltac::Register ltac::RegisterManager::get_free_reg(){
    auto reg = ::get_free_reg(registers, *this);
    reserve(reg);
    return reg;
}

ltac::FloatRegister ltac::RegisterManager::get_free_float_reg(){
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
    return liveness->IN_S[current].values().pointer_escaped->find(variable) != liveness->IN_S[current].values().pointer_escaped->end();
}

bool ltac::RegisterManager::is_live(std::shared_ptr<Variable> variable){
    auto live = is_live(variable, current);
    DEBUG_GLOBAL std::cout << variable->name() << " is live " << live << std::endl;
    return live;
}
    
void ltac::RegisterManager::collect_parameters(std::shared_ptr<eddic::Function> definition, PlatformDescriptor* descriptor){
    for(auto parameter : definition->parameters){
        auto param = definition->context->getVariable(parameter.name);

        if(param->position().isParamRegister()){
            if(param->type() == INT){
                registers.setLocation(param, ltac::Register(descriptor->int_param_register(param->position().offset())));
            } else if(param->type() == FLOAT){
                float_registers.setLocation(param, ltac::FloatRegister(descriptor->float_param_register(param->position().offset())));
            }
        }
    }
}

void ltac::RegisterManager::restore_pushed_registers(){
    std::reverse(int_pushed.begin(), int_pushed.end());
    std::reverse(float_pushed.begin(), float_pushed.end());

    //Restore the int parameters in registers (in the reverse order they were pushed)
    for(auto& reg : int_pushed){
        if(registers.used(reg) && registers[reg]->position().isParamRegister()){
            ltac::add_instruction(function, ltac::Operator::POP, reg);
        }
    }

    //Restore the float parameters in registers (in the reverse order they were pushed)
    for(auto& reg : float_pushed){
        if(float_registers.used(reg) && float_registers[reg]->position().isParamRegister()){
            ltac::add_instruction(function, ltac::Operator::FMOV, reg, ltac::Address(ltac::SP, 0));
            ltac::add_instruction(function, ltac::Operator::ADD, ltac::SP, static_cast<int>(FLOAT->size()));
        }
    }

    //Each register has been restored
    int_pushed.clear();
    float_pushed.clear();

    //All the parameters have been handled by now, the next param will be the first for its call
    first_param = true;
}

void ltac::RegisterManager::save_registers(std::shared_ptr<mtac::Param>& param, PlatformDescriptor* descriptor){
    if(first_param){
        if(param->function){
            unsigned int maxInt = descriptor->numberOfIntParamRegisters();
            unsigned int maxFloat = descriptor->numberOfFloatParamRegisters();
            
            for(auto& parameter : param->function->parameters){
                auto type = param->function->getParameterType(parameter.name);
                unsigned int position = param->function->getParameterPositionByType(parameter.name);

                if(type == INT && position <= maxInt){
                    ltac::Register reg(descriptor->int_param_register(position));

                    //If the parameter register is already used by a variable or a parent parameter
                    if(registers.used(reg)){
                        if(registers[reg]->position().isParamRegister()){
                            int_pushed.push_back(reg);
                            ltac::add_instruction(function, ltac::Operator::PUSH, reg);
                        } else {
                            spills(reg);
                        }
                    }
                }

                if(type == FLOAT && position <= maxFloat){
                    ltac::FloatRegister reg(descriptor->float_param_register(position));

                    //If the parameter register is already used by a variable or a parent parameter
                    if(float_registers.used(reg)){
                        if(float_registers[reg]->position().isParamRegister()){
                            float_pushed.push_back(reg);

                            ltac::add_instruction(function, ltac::Operator::SUB, ltac::SP, static_cast<int>(FLOAT->size()));
                            ltac::add_instruction(function, ltac::Operator::FMOV, ltac::Address(ltac::SP, 0), reg);
                        } else {
                            spills(reg);
                        }
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

    if(DEBUG_GLOBAL_ENABLED){
        mtac::Printer printer;
        printer.printStatement(statement);
    }
}

void ltac::RegisterManager::reserve(ltac::Register reg){
    registers.reserve(reg);
    DEBUG_GLOBAL std::cout << "Int Register " << reg  << " reserved" << std::endl;
}

void ltac::RegisterManager::release(ltac::Register reg){
    registers.release(reg);
    DEBUG_GLOBAL std::cout << "Int Register " << reg  << " released" << std::endl;
}

void ltac::RegisterManager::reserve(ltac::FloatRegister reg){
    float_registers.reserve(reg);
    DEBUG_GLOBAL std::cout << "Float Register " << reg  << " reserved" << std::endl;
}

void ltac::RegisterManager::release(ltac::FloatRegister reg){
    float_registers.release(reg);
    DEBUG_GLOBAL std::cout << "Float Register " << reg  << " released" << std::endl;
}
    
bool ltac::RegisterManager::is_written(std::shared_ptr<Variable> variable){
    return written.find(variable) != written.end();
}

void ltac::RegisterManager::set_written(std::shared_ptr<Variable> variable){
    written.insert(variable);
}
