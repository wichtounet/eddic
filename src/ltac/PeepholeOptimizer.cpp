//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <boost/optional.hpp>
#include <boost/range/adaptors.hpp>

#include "Utils.hpp"
#include "PerfsTimer.hpp"
#include "Options.hpp"
#include "likely.hpp"
#include "Platform.hpp"
#include "Type.hpp"
#include "FunctionContext.hpp"

#include "ltac/PeepholeOptimizer.hpp"
#include "ltac/Printer.hpp"

#include "ltac/Utils.hpp"

using namespace eddic;

namespace {

inline bool transform_to_nop(std::shared_ptr<ltac::Instruction> instruction){
    if(instruction->op == ltac::Operator::NOP){
        return false;
    }
    
    instruction->op = ltac::Operator::NOP;
    instruction->arg1.reset();
    instruction->arg2.reset();

    return true;
}

inline bool optimize_statement(ltac::Statement& statement){
    if(boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
        auto instruction = boost::get<std::shared_ptr<ltac::Instruction>>(statement);
        
        //SUB or ADD 0 has no effect
        if(instruction->op == ltac::Operator::ADD || instruction->op == ltac::Operator::SUB){
            if(ltac::is_reg(*instruction->arg1) && boost::get<int>(&*instruction->arg2)){
                auto value = boost::get<int>(*instruction->arg2);
                
                if(value == 0){
                    return transform_to_nop(instruction);
                }
            }
        }

        if(instruction->op == ltac::Operator::MOV){
            //MOV reg, 0 can be transformed into XOR reg, reg
            if(ltac::is_reg(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, 0)){
                instruction->op = ltac::Operator::XOR;
                instruction->arg2 = instruction->arg1;

                return true;
            }

            if(ltac::is_reg(*instruction->arg1) && ltac::is_reg(*instruction->arg2)){
                auto& reg1 = boost::get<ltac::Register>(*instruction->arg1); 
                auto& reg2 = boost::get<ltac::Register>(*instruction->arg2); 
            
                //MOV reg, reg is useless
                if(reg1 == reg2){
                    return transform_to_nop(instruction);
                }
            }
        }

        if(instruction->op == ltac::Operator::ADD){
            //ADD reg, 1 can be transformed into INC reg
            if(ltac::is_reg(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, 1)){
                instruction->op = ltac::Operator::INC;
                instruction->arg2.reset();

                return true;
            }
            
            //ADD reg, -1 can be transformed into DEC reg
            if(ltac::is_reg(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, -1)){
                instruction->op = ltac::Operator::DEC;
                instruction->arg2.reset();

                    return true;
            }
        }
        
        if(instruction->op == ltac::Operator::SUB){
            //SUB reg, 1 can be transformed into DEC reg
            if(ltac::is_reg(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, 1)){
                instruction->op = ltac::Operator::DEC;
                instruction->arg2.reset();

                    return true;
            }
            
            //SUB reg, -1 can be transformed into INC reg
            if(ltac::is_reg(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, -1)){
                instruction->op = ltac::Operator::INC;
                instruction->arg2.reset();

                    return true;
            }
        }

        if(instruction->op == ltac::Operator::MUL){
            //Optimize multiplications with SHIFTs or LEAs
            if(ltac::is_reg(*instruction->arg1) && mtac::is<int>(*instruction->arg2)){
                int constant = boost::get<int>(*instruction->arg2);

                auto reg = boost::get<ltac::Register>(*instruction->arg1);
        
                if(isPowerOfTwo(constant)){
                    instruction->op = ltac::Operator::SHIFT_LEFT;
                    instruction->arg2 = powerOfTwo(constant);

                    return true;
                } 
                
                if(constant == 3){
                    instruction->op = ltac::Operator::LEA;
                    instruction->arg2 = ltac::Address(reg, reg, 2, 0);

                    return true;
                } 
                
                if(constant == 5){
                    instruction->op = ltac::Operator::LEA;
                    instruction->arg2 = ltac::Address(reg, reg, 4, 0);

                    return true;
                } 
                
                if(constant == 9){
                    instruction->op = ltac::Operator::LEA;
                    instruction->arg2 = ltac::Address(reg, reg, 8, 0);

                    return true;
                }
            }
        }

        if(instruction->op == ltac::Operator::CMP_INT){
            //Optimize comparisons with 0 with or reg, reg
            if(ltac::is_reg(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, 0)){
                instruction->op = ltac::Operator::OR;
                instruction->arg2 = instruction->arg1;

                return true;
            }
        }

        if(instruction->op == ltac::Operator::LEA){
            auto address = boost::get<ltac::Address>(*instruction->arg2);

            if(address.base_register && !address.scaled_register){
                if(!address.displacement){
                    instruction->op = ltac::Operator::MOV;
                    instruction->arg2 = address.base_register;

                    return true;
                } else if(*address.displacement == 0){
                    instruction->op = ltac::Operator::MOV;
                    instruction->arg2 = address.base_register;

                    return true;
                }
            }
        }
    }

    return false;
}

inline bool multiple_statement_optimizations(ltac::Statement& s1, ltac::Statement& s2){
    if(mtac::is<std::shared_ptr<ltac::Instruction>>(s1) && mtac::is<std::shared_ptr<ltac::Instruction>>(s2)){
        auto& i1 = boost::get<std::shared_ptr<ltac::Instruction>>(s1);
        auto& i2 = boost::get<std::shared_ptr<ltac::Instruction>>(s2);

        //Statements after RET are dead
        if(i1->op == ltac::Operator::RET){
            return transform_to_nop(i2);
        }
        
        //Two following LEAVE are not useful
        if(i1->op == ltac::Operator::LEAVE && i2->op == ltac::Operator::LEAVE){
            return transform_to_nop(i2);
        }

        //Combine two ADD into one
        if(i1->op == ltac::Operator::ADD && i2->op == ltac::Operator::ADD){
            if(ltac::is_reg(*i1->arg1) && ltac::is_reg(*i2->arg1) && boost::get<int>(&*i1->arg2) && boost::get<int>(&*i2->arg2)){
                auto reg1 = boost::get<ltac::Register>(*i1->arg1);
                auto reg2 = boost::get<ltac::Register>(*i2->arg1);

                if(reg1 == reg2){
                    i1->arg2 = boost::get<int>(*i1->arg2) + boost::get<int>(*i2->arg2);

                    return transform_to_nop(i2);
                }
            }
        }
        
        //Combine two SUB into one
        if(i1->op == ltac::Operator::SUB && i2->op == ltac::Operator::SUB){
            if(ltac::is_reg(*i1->arg1) && ltac::is_reg(*i2->arg1) && boost::get<int>(&*i1->arg2) && boost::get<int>(&*i2->arg2)){
                auto reg1 = boost::get<ltac::Register>(*i1->arg1);
                auto reg2 = boost::get<ltac::Register>(*i2->arg1);

                if(reg1 == reg2){
                    i1->arg2 = boost::get<int>(*i1->arg2) + boost::get<int>(*i2->arg2);

                    return transform_to_nop(i2);
                }
            }
        }

        if(i1->op == ltac::Operator::MOV && i2->op == ltac::Operator::MOV){
            if(ltac::is_reg(*i1->arg1) && ltac::is_reg(*i1->arg2) && ltac::is_reg(*i2->arg1) && ltac::is_reg(*i2->arg2)){
                auto reg11 = boost::get<ltac::Register>(*i1->arg1);
                auto reg12 = boost::get<ltac::Register>(*i1->arg2);
                auto reg21 = boost::get<ltac::Register>(*i2->arg1);
                auto reg22 = boost::get<ltac::Register>(*i2->arg2);

                //cross MOV (ir4 = ir5, ir5 = ir4), keep only the first
                if (reg11 == reg22 && reg12 == reg21){
                    return transform_to_nop(i2);
                }
            } else if(ltac::is_reg(*i1->arg1) && ltac::is_reg(*i2->arg2)){
                auto reg11 = boost::get<ltac::Register>(*i1->arg1);
                auto reg22 = boost::get<ltac::Register>(*i2->arg2);
                
                if(reg11 == reg22 && boost::get<ltac::Address>(&*i1->arg2) && boost::get<ltac::Address>(&*i2->arg1)){
                    if(boost::get<ltac::Address>(*i1->arg2) == boost::get<ltac::Address>(*i2->arg1)){
                        return transform_to_nop(i2);
                    }
                }
            } else if(ltac::is_reg(*i1->arg2) && ltac::is_reg(*i2->arg1)){
                auto reg12 = boost::get<ltac::Register>(*i1->arg2);
                auto reg21 = boost::get<ltac::Register>(*i2->arg1);

                if(reg12 == reg21 && boost::get<ltac::Address>(&*i1->arg1) && boost::get<ltac::Address>(&*i2->arg2)){
                    if(boost::get<ltac::Address>(*i1->arg1) == boost::get<ltac::Address>(*i2->arg2)){
                        return transform_to_nop(i2);
                    }
                }
            }
        }
        
        if(i1->op == ltac::Operator::MOV && i2->op == ltac::Operator::ADD){
            if(ltac::is_reg(*i1->arg1) && ltac::is_reg(*i2->arg1)){
                if(boost::get<ltac::Register>(*i1->arg1) == boost::get<ltac::Register>(*i2->arg1)){
                    if(boost::get<ltac::Register>(&*i1->arg2) && boost::get<int>(&*i2->arg2)){
                        i2->op = ltac::Operator::LEA;
                        i2->arg2 = ltac::Address(boost::get<ltac::Register>(*i1->arg2), boost::get<int>(*i2->arg2));

                        return transform_to_nop(i1);
                    } else if(boost::get<std::string>(&*i1->arg2) && boost::get<int>(&*i2->arg2)){
                        i2->op = ltac::Operator::LEA;
                        i2->arg2 = ltac::Address(boost::get<std::string>(*i1->arg2), boost::get<int>(*i2->arg2));

                        return transform_to_nop(i1);
                    }
                }
            }
        }

        if(i1->op == ltac::Operator::POP && i2->op == ltac::Operator::PUSH){
            if(ltac::is_reg(*i1->arg1) && ltac::is_reg(*i2->arg1)){
                auto reg1 = boost::get<ltac::Register>(*i1->arg1);
                auto reg2 = boost::get<ltac::Register>(*i2->arg1);

                if(reg1 == reg2){
                    i1->op = ltac::Operator::MOV;
                    i1->arg2 = ltac::Address(ltac::SP, 0);

                    return transform_to_nop(i2);
                }
            }
        }
        
        if(i1->op == ltac::Operator::PUSH && i2->op == ltac::Operator::POP){
            if(ltac::is_reg(*i1->arg1) && ltac::is_reg(*i2->arg1)){
                auto reg1 = boost::get<ltac::Register>(*i1->arg1);
                auto reg2 = boost::get<ltac::Register>(*i2->arg1);

                if(reg1 == reg2){
                    transform_to_nop(i1);

                    return transform_to_nop(i2);
                }
            }
        }
    }

    return false;
}

inline bool multiple_statement_optimizations_second(ltac::Statement& s1, ltac::Statement& s2){
    if(mtac::is<std::shared_ptr<ltac::Instruction>>(s1) && mtac::is<std::shared_ptr<ltac::Instruction>>(s2)){
        auto& i1 = boost::get<std::shared_ptr<ltac::Instruction>>(s1);
        auto& i2 = boost::get<std::shared_ptr<ltac::Instruction>>(s2);

        if(i1->op == ltac::Operator::MOV && i2->op == ltac::Operator::MOV){
            if(ltac::is_reg(*i1->arg1) && ltac::is_reg(*i2->arg1) && ltac::is_reg(*i2->arg2)){
                auto reg11 = boost::get<ltac::Register>(*i1->arg1);
                auto reg21 = boost::get<ltac::Register>(*i2->arg1);
                auto reg22 = boost::get<ltac::Register>(*i2->arg2);

                if(reg22 == reg11){
                    auto descriptor = getPlatformDescriptor(platform);

                    for(unsigned int i = 0; i < descriptor->numberOfIntParamRegisters(); ++i){
                        auto reg = ltac::Register(descriptor->int_param_register(i + 1));

                        if(reg21 == reg){
                            i2->arg2 = i1->arg2;

                            return true;
                        }
                    }
    
                    if(reg21 == ltac::Register(descriptor->int_return_register1())){
                        i2->arg2 = i1->arg2;

                        return true;
                    }
    
                    if(reg21 == ltac::Register(descriptor->int_return_register2())){
                        i2->arg2 = i1->arg2;

                        return true;
                    }
                }
            }
        }
        
        if(i1->op == ltac::Operator::MOV && i2->op == ltac::Operator::PUSH){
            if(ltac::is_reg(*i1->arg1) && ltac::is_reg(*i2->arg1) && !ltac::is_float_reg(*i1->arg2)){
                auto reg11 = boost::get<ltac::Register>(*i1->arg1);
                auto reg21 = boost::get<ltac::Register>(*i2->arg1);
                
                if(reg11 == reg21){
                    i2->arg1 = i1->arg2;

                    return true;
                }
            }
        }
    }
    
    return false;
}

inline bool is_nop(ltac::Statement& statement){
    if(mtac::is<std::shared_ptr<ltac::Instruction>>(statement)){
        auto instruction = boost::get<std::shared_ptr<ltac::Instruction>>(statement);

        if(instruction->op == ltac::Operator::NOP){
            return true;
        }
    }

    return false;
}

bool basic_optimizations(std::shared_ptr<ltac::Function> function){
    auto& statements = function->getStatements();

    bool optimized = false;

    auto it = statements.begin();
    auto end = statements.end() - 1;

    while(it != end){
        auto& s1 = *it;
        auto& s2 = *(it + 1);

        //Optimizations that looks at only one statement
        optimized |= optimize_statement(s1);
        optimized |= optimize_statement(s2);

        //Optimizations that looks at several statements at once
        optimized |= multiple_statement_optimizations(s1, s2);

        if(unlikely(is_nop(s1))){
            it = statements.erase(it);
            end = statements.end() - 1;

            optimized = true;

            continue;
        }

        ++it;
    }
    
    it = statements.begin();
    end = statements.end() - 1;

    while(it != end){
        auto& s1 = *it;
        auto& s2 = *(it + 1);

        optimized |= multiple_statement_optimizations_second(s1, s2);

        ++it;
    }

    return optimized;
}

bool constant_propagation(std::shared_ptr<ltac::Function> function){
    bool optimized = false;

    auto& statements = function->getStatements();
    
    std::unordered_map<ltac::Register, int, ltac::RegisterHash> constants; 

    for(std::size_t i = 0; i < statements.size(); ++i){
        auto statement = statements[i];

        if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
            auto instruction = *ptr;

            //Erase constant
            if(instruction->arg1 && ltac::is_reg(*instruction->arg1)){
                auto reg1 = boost::get<ltac::Register>(*instruction->arg1);

                constants.erase(reg1);
            }

            //Collect constants
            if(instruction->op == ltac::Operator::XOR){
                if(ltac::is_reg(*instruction->arg1) && ltac::is_reg(*instruction->arg2)){
                    auto reg1 = boost::get<ltac::Register>(*instruction->arg1);
                    auto reg2 = boost::get<ltac::Register>(*instruction->arg2);

                    if(reg1 == reg2){
                        constants[reg1] = 0;
                    }
                }
            } else if(instruction->op == ltac::Operator::MOV){
                if(ltac::is_reg(*instruction->arg1)){
                    if (auto* valuePtr = boost::get<int>(&*instruction->arg2)){
                        auto reg1 = boost::get<ltac::Register>(*instruction->arg1);
                        constants[reg1] = *valuePtr;
                    }
                }
            }

            //Optimize MOV
            if(instruction->op == ltac::Operator::MOV){
                if(ltac::is_reg(*instruction->arg2)){
                    auto reg2 = boost::get<ltac::Register>(*instruction->arg2);

                    if(constants.find(reg2) != constants.end()){
                        instruction->arg2 = constants[reg2];
                        optimized = true;
                    }
                }
            }
        } else {
            //Takes care of safe functions
            if(auto* ptr = boost::get<std::shared_ptr<ltac::Jump>>(&statement)){
                if((*ptr)->type == ltac::JumpType::CALL && mtac::safe((*ptr)->label)){
                    continue;
                }
            }

            //At this point, the basic block is at its end
            constants.clear();
        }
    }

    return optimized;
}

typedef std::unordered_set<ltac::Register, ltac::RegisterHash> RegisterUsage;

void add_param_registers(RegisterUsage& usage){
    auto descriptor = getPlatformDescriptor(platform);
    
    for(unsigned int i = 1; i <= descriptor->numberOfIntParamRegisters(); ++i){
        usage.insert(ltac::Register(descriptor->int_param_register(i)));
    }
}

void add_escaped_registers(RegisterUsage& usage, std::shared_ptr<ltac::Function> function){
    auto descriptor = getPlatformDescriptor(platform);
    
    if(function->definition->returnType == STRING){
        usage.insert(ltac::Register(descriptor->int_return_register1()));
        usage.insert(ltac::Register(descriptor->int_return_register2()));
    } else if(function->definition->returnType != VOID){
        usage.insert(ltac::Register(descriptor->int_return_register1()));
    }

    add_param_registers(usage);

    for(auto var : function->context->stored_variables()){
        if(var.second->position().is_register() && mtac::is_single_int_register(var.second->type())){
            usage.insert(ltac::Register(descriptor->int_variable_register(var.second->position().offset())));
        }
    }
}

void collect_usage(RegisterUsage& usage, boost::optional<ltac::Argument>& arg){
    if(arg){
        if(ltac::is_reg(*arg)){
            auto reg1 = boost::get<ltac::Register>(*arg);
            usage.insert(reg1);
        }

        if(boost::get<ltac::Address>(&*arg)){
            auto address = boost::get<ltac::Address>(*arg);

            if(address.scaled_register){
                usage.insert(*address.scaled_register);
            }
            
            if(address.base_register){
                usage.insert(*address.base_register);
            }
        }
    }   
}

bool dead_code_elimination(std::shared_ptr<ltac::Function> function){
    bool optimized = false;

    auto& statements = function->getStatements();
    
    RegisterUsage usage; 
    add_escaped_registers(usage, function);

    for(auto statement : boost::adaptors::reverse(statements)){
        if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
            auto instruction = *ptr;

            //Optimize MOV, LEA, XOR
            if(instruction->op == ltac::Operator::MOV || instruction->op == ltac::Operator::LEA || instruction->op == ltac::Operator::XOR){
                if(ltac::is_reg(*instruction->arg1)){
                    auto reg1 = boost::get<ltac::Register>(*instruction->arg1);

                    if(usage.find(reg1) == usage.end()){
                        std::cout << "removed" << std::endl;
                        ltac::Printer printer;
                        printer.print(statement);
                        optimized = transform_to_nop(instruction);
                    }
                    
                    usage.erase(reg1);
                    add_param_registers(usage);
                } else {
                    collect_usage(usage, instruction->arg1);
                }
            
                collect_usage(usage, instruction->arg2);
            } else {
                //Collect usage 
                collect_usage(usage, instruction->arg1);
                collect_usage(usage, instruction->arg2);
                collect_usage(usage, instruction->arg3);
            }

            //TODO Take in account more instructions that erase results, optimize them and remove them from the usage
        } else {
            //Takes care of safe functions
            if(auto* ptr = boost::get<std::shared_ptr<ltac::Jump>>(&statement)){
                if((*ptr)->type == ltac::JumpType::CALL && mtac::safe((*ptr)->label)){
                    add_param_registers(usage);
                    continue;
                }
            }
            
            //At this point, the basic block is at its end
            usage.clear();
            add_escaped_registers(usage, function);
        }
    }

    return optimized;
}

bool debug(const std::string& name, bool b, std::shared_ptr<ltac::Function> function){
    if(option_defined("dev")){
        if(b){
            std::cout << "optimization " << name << " returned true" << std::endl;

            //Print the function
            ltac::Printer printer;
            printer.print(function);
        } else {
            std::cout << "optimization " << name << " returned false" << std::endl;
        }
    }

    return b;
}

} //end of anonymous namespace

void eddic::ltac::optimize(std::shared_ptr<ltac::Program> program){
    PerfsTimer timer("Peephole optimizations");

    for(auto& function : program->functions){
        if(option_defined("dev")){
            std::cout << "Start optimizations on " << function->getName() << std::endl;

            //Print the function
            ltac::Printer printer;
            printer.print(function);
        }

        bool optimized;
        do {
            optimized = false;
            
            optimized |= debug("Basic optimizations", basic_optimizations(function), function);
            optimized |= debug("Constant propagation", constant_propagation(function), function);
            optimized |= debug("Dead-Code Elimination", dead_code_elimination(function), function);
        } while(optimized);
    }
}
