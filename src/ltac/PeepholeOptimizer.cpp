//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <boost/optional.hpp>
#include <boost/range/adaptors.hpp>

#include "assert.hpp"
#include "logging.hpp"
#include "Utils.hpp"
#include "PerfsTimer.hpp"
#include "Options.hpp"
#include "likely.hpp"
#include "Platform.hpp"
#include "Type.hpp"
#include "FunctionContext.hpp"
#include "Variable.hpp"

#include "mtac/GlobalOptimizations.hpp"
#include "mtac/Statement.hpp"

#include "ltac/PeepholeOptimizer.hpp"
#include "ltac/Statement.hpp"
#include "ltac/Register.hpp"
#include "ltac/FloatRegister.hpp"
#include "ltac/Printer.hpp"
#include "ltac/Utils.hpp"
#include "ltac/LiveRegistersProblem.hpp"
#include "ltac/Instruction.hpp"
#include "ltac/Jump.hpp"

using namespace eddic;

namespace {

inline bool optimize_statement(ltac::Statement& statement){
    if(boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
        auto instruction = boost::get<std::shared_ptr<ltac::Instruction>>(statement);
        
        //SUB or ADD 0 has no effect
        if(instruction->op == ltac::Operator::ADD || instruction->op == ltac::Operator::SUB){
            if(ltac::is_reg(*instruction->arg1) && boost::get<int>(&*instruction->arg2)){
                auto value = boost::get<int>(*instruction->arg2);
                
                if(value == 0){
                    return ltac::transform_to_nop(instruction);
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
                    return ltac::transform_to_nop(instruction);
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

        if(instruction->op == ltac::Operator::MUL2 || instruction->op == ltac::Operator::MUL3){
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
            return ltac::transform_to_nop(i2);
        }
        
        //Two following LEAVE are not useful
        if(i1->op == ltac::Operator::LEAVE && i2->op == ltac::Operator::LEAVE){
            return ltac::transform_to_nop(i2);
        }

        //Combine two ADD into one
        if(i1->op == ltac::Operator::ADD && i2->op == ltac::Operator::ADD){
            if(ltac::is_reg(*i1->arg1) && ltac::is_reg(*i2->arg1) && boost::get<int>(&*i1->arg2) && boost::get<int>(&*i2->arg2)){
                auto reg1 = boost::get<ltac::Register>(*i1->arg1);
                auto reg2 = boost::get<ltac::Register>(*i2->arg1);

                if(reg1 == reg2){
                    i1->arg2 = boost::get<int>(*i1->arg2) + boost::get<int>(*i2->arg2);

                    return ltac::transform_to_nop(i2);
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

                    return ltac::transform_to_nop(i2);
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
                    return ltac::transform_to_nop(i2);
                }
            } else if(ltac::is_reg(*i1->arg1) && ltac::is_reg(*i2->arg2)){
                auto reg11 = boost::get<ltac::Register>(*i1->arg1);
                auto reg22 = boost::get<ltac::Register>(*i2->arg2);
                
                if(reg11 == reg22 && boost::get<ltac::Address>(&*i1->arg2) && boost::get<ltac::Address>(&*i2->arg1)){
                    if(boost::get<ltac::Address>(*i1->arg2) == boost::get<ltac::Address>(*i2->arg1)){
                        return ltac::transform_to_nop(i2);
                    }
                }
            } else if(ltac::is_reg(*i1->arg2) && ltac::is_reg(*i2->arg1)){
                auto reg12 = boost::get<ltac::Register>(*i1->arg2);
                auto reg21 = boost::get<ltac::Register>(*i2->arg1);

                if(reg12 == reg21 && boost::get<ltac::Address>(&*i1->arg1) && boost::get<ltac::Address>(&*i2->arg2)){
                    if(boost::get<ltac::Address>(*i1->arg1) == boost::get<ltac::Address>(*i2->arg2)){
                        return ltac::transform_to_nop(i2);
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

                        return ltac::transform_to_nop(i1);
                    } else if(boost::get<std::string>(&*i1->arg2) && boost::get<int>(&*i2->arg2)){
                        i2->op = ltac::Operator::LEA;
                        i2->arg2 = ltac::Address(boost::get<std::string>(*i1->arg2), boost::get<int>(*i2->arg2));

                        return ltac::transform_to_nop(i1);
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

                    return ltac::transform_to_nop(i2);
                }
            }
        }
        
        if(i1->op == ltac::Operator::PUSH && i2->op == ltac::Operator::POP){
            if(ltac::is_reg(*i1->arg1) && ltac::is_reg(*i2->arg1)){
                auto reg1 = boost::get<ltac::Register>(*i1->arg1);
                auto reg2 = boost::get<ltac::Register>(*i2->arg1);

                if(reg1 == reg2){
                    ltac::transform_to_nop(i1);

                    return ltac::transform_to_nop(i2);
                }
            }
        }
    }

    return false;
}

inline bool multiple_statement_optimizations_second(ltac::Statement& s1, ltac::Statement& s2, Platform platform){
    if(mtac::is<std::shared_ptr<ltac::Instruction>>(s1) && mtac::is<std::shared_ptr<ltac::Instruction>>(s2)){
        auto& i1 = boost::get<std::shared_ptr<ltac::Instruction>>(s1);
        auto& i2 = boost::get<std::shared_ptr<ltac::Instruction>>(s2);

        if(i1->op == ltac::Operator::MOV && i2->op == ltac::Operator::MOV){
            if(ltac::is_reg(*i1->arg1) && ltac::is_reg(*i2->arg1) && ltac::is_reg(*i2->arg2)){
                auto reg11 = boost::get<ltac::Register>(*i1->arg1);
                auto reg21 = boost::get<ltac::Register>(*i2->arg1);
                auto reg22 = boost::get<ltac::Register>(*i2->arg2);

                bool possible = true;

                if(auto* ptr = boost::get<ltac::Address>(&*i1->arg2)){
                    if(ptr->base_register && boost::get<ltac::Register>(*ptr->base_register) == reg11){
                        possible = false;
                    }
                    
                    if(ptr->scaled_register && boost::get<ltac::Register>(*ptr->scaled_register) == reg11){
                        possible = false;
                    }
                }

                if(reg22 == reg11 && possible){
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
                    bool valid = true;
                    
                    if(auto* ptr = boost::get<ltac::Address>(&*i1->arg2)){
                        if(ptr->base_register && boost::get<ltac::Register>(*ptr->base_register) == reg11){
                            valid = false;
                        } else if(ptr->scaled_register && boost::get<ltac::Register>(*ptr->scaled_register) == reg11){
                            valid = false;
                        }
                    }

                    if(valid){
                        i2->arg1 = i1->arg2;

                        return true;
                    }
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

bool basic_optimizations(mtac::function_p function, Platform platform){
    bool optimized = false;
    
    for(auto& bb : function){
        auto& statements = bb->l_statements;

        if(statements.empty()){
            continue;
        }

        if(statements.size() == 1){
            optimized |= optimize_statement(statements.front());

            continue;
        }

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

            optimized |= multiple_statement_optimizations_second(s1, s2, platform);

            ++it;
        }
    }

    return optimized;
}

bool constant_propagation(mtac::function_p function){
    bool optimized = false;

    for(auto& bb : function){
        auto& statements = bb->l_statements;

        std::unordered_map<ltac::Register, int> constants; 

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
            } 
        }
    }
    

    return optimized;
}

void remove_reg(std::unordered_map<ltac::Register, ltac::Register>& copies, ltac::Register reg){
    auto it = copies.begin();
    auto end = copies.end();

    while(it != end){
        if(it->first == reg || it->second == reg){
            it = copies.erase(it);
            end = copies.end();
            continue;
        }

        ++it;
    }
}

bool copy_propagation(mtac::function_p function, Platform platform){
    auto descriptor = getPlatformDescriptor(platform);

    bool optimized = false;

    for(auto& bb : function){
        auto& statements = bb->l_statements;

        std::unordered_map<ltac::Register, ltac::Register> copies;

        for(std::size_t i = 0; i < statements.size(); ++i){
            auto statement = statements[i];

            if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                auto instruction = *ptr;

                //Erase constant
                if(instruction->arg1 && ltac::is_reg(*instruction->arg1)){
                    auto reg = boost::get<ltac::Register>(*instruction->arg1);

                    remove_reg(copies, reg);
                }

                if(instruction->op == ltac::Operator::DIV){
                    remove_reg(copies, ltac::Register(descriptor->a_register()));
                    remove_reg(copies, ltac::Register(descriptor->d_register()));
                }

                //Collect copies
                if(instruction->op == ltac::Operator::MOV){
                    if(ltac::is_reg(*instruction->arg1)){
                        if (auto* reg_ptr = boost::get<ltac::Register>(&*instruction->arg2)){
                            auto reg1 = boost::get<ltac::Register>(*instruction->arg1);
                            copies[reg1] = *reg_ptr;
                        }
                    }
                }

                //Optimize MOV
                if(instruction->op == ltac::Operator::MOV){
                    if(ltac::is_reg(*instruction->arg2)){
                        auto reg2 = boost::get<ltac::Register>(*instruction->arg2);

                        if(copies.find(reg2) != copies.end()){
                            instruction->arg2 = copies[reg2];
                            optimized = true;
                        }
                    }
                }
            } 
        }
    }

    return optimized;
}

typedef std::unordered_set<ltac::Register> RegisterUsage;

void add_param_registers(RegisterUsage& usage, Platform platform){
    auto descriptor = getPlatformDescriptor(platform);
    
    for(unsigned int i = 1; i <= descriptor->numberOfIntParamRegisters(); ++i){
        usage.insert(ltac::Register(descriptor->int_param_register(i)));
    }
   
    usage.insert(ltac::Register(descriptor->a_register()));
    usage.insert(ltac::Register(descriptor->d_register()));

    //Never optimize those registers
    usage.insert(ltac::SP);
    usage.insert(ltac::BP);
}

void add_escaped_registers(RegisterUsage& usage, mtac::function_p function, Platform platform){
    auto descriptor = getPlatformDescriptor(platform);
    
    if(function->definition->returnType == STRING){
        usage.insert(ltac::Register(descriptor->int_return_register1()));
        usage.insert(ltac::Register(descriptor->int_return_register2()));
    } else if(function->definition->returnType != VOID){
        usage.insert(ltac::Register(descriptor->int_return_register1()));
    }

    add_param_registers(usage, platform);

    for(auto& var : function->context->stored_variables()){
        if(var->position().is_register() && mtac::is_single_int_register(var->type())){
            usage.insert(ltac::Register(descriptor->int_variable_register(var->position().offset())));
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
                usage.insert(boost::get<ltac::Register>(*address.scaled_register));
            }
            
            if(address.base_register){
                usage.insert(boost::get<ltac::Register>(*address.base_register));
            }
        }
    }   
}

RegisterUsage collect_register_usage(mtac::function_p function, Platform platform){
    RegisterUsage usage;
    add_escaped_registers(usage, function, platform);

    for(auto& bb : function){
        for(auto& statement : bb->l_statements){
            if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                auto instruction = *ptr;

                collect_usage(usage, instruction->arg1);
                collect_usage(usage, instruction->arg2);
                collect_usage(usage, instruction->arg3);
            }
        }
    }

    return usage;
}

ltac::Register get_free_reg(RegisterUsage& usage, Platform platform){
    auto descriptor = getPlatformDescriptor(platform);
   
    for(auto& reg : descriptor->symbolic_registers()){
        if(usage.find(ltac::Register(reg)) == usage.end()){
            return ltac::Register(reg);
        }
    }

    return ltac::SP;
}

template<typename T>
inline bool one_of(const T& value, const std::vector<T>& container){
    return std::find(container.begin(), container.end(), value) != container.end();
}

bool dead_code_elimination(mtac::function_p function){
    bool optimized = false;

    ltac::LiveRegistersProblem problem;
    auto results = mtac::data_flow(function, problem);
    
    for(auto& block : function){
        auto it = iterate(block->l_statements);

        while(it.has_next()){
            auto& statement = *it;

            if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                if(ltac::erase_result((*ptr)->op)){
                    //OR is used for comparisons
                    if((*ptr)->op == ltac::Operator::OR){
                        ++it;
                        continue;
                    }

                    if(auto* reg_ptr = boost::get<ltac::Register>(&*(*ptr)->arg1)){
                        //SP is always live
                        if(*reg_ptr == ltac::SP){
                            ++it;
                            continue;
                        }

                        //Some statements (in ENTRY and EXIT) are not annotated, it is enough to ignore them
                        if(results->OUT_LS.count(statement)){
                            auto& liveness = results->OUT_LS[statement].values();

                            if(liveness.find(*reg_ptr) == liveness.end()){
                                it.erase();
                                optimized=true;
                                continue;
                            }
                        }
                    }
                }
            }

            ++it;
        }
    }

    return optimized;
}

ltac::Operator get_cmov_op(ltac::JumpType op){
    switch(op){
        case ltac::JumpType::NE:
            return ltac::Operator::CMOVNE;
        case ltac::JumpType::E:
            return ltac::Operator::CMOVE;
        case ltac::JumpType::GE:
            return ltac::Operator::CMOVGE;
        case ltac::JumpType::G:
            return ltac::Operator::CMOVG;
        case ltac::JumpType::LE:
            return ltac::Operator::CMOVLE;
        case ltac::JumpType::L:
            return ltac::Operator::CMOVL;
        case ltac::JumpType::B:
            return ltac::Operator::CMOVB;
        case ltac::JumpType::BE:
            return ltac::Operator::CMOVBE;
        case ltac::JumpType::A:
            return ltac::Operator::CMOVA;
        case ltac::JumpType::AE:
            return ltac::Operator::CMOVAE;
        default:
            eddic_unreachable("No cmov equivalent");
    }
}

template<typename BIt, typename It>
bool move_forward(BIt& bit, BIt& bend, It& it, It& end){
    if(it != end){
        ++it;

        if(it == end){
            return move_forward(bit, bend, it, end);
        }

        return true;
    }

    if(bit != bend){
        ++bit;

        if(bit == bend){
            return false;
        }
        
        auto& bb = *bit;

        it = bb->l_statements.begin();
        end = bb->l_statements.end();

        if(bb->l_statements.empty()){
            return move_forward(bit, bend, it, end);
        }

        return true;
    }

    return false;
}

bool conditional_move(mtac::function_p function, Platform platform){
    bool optimized = false;

    RegisterUsage usage = collect_register_usage(function, platform);

    auto free_reg = get_free_reg(usage, platform);
    if(free_reg == ltac::SP){
        return optimized;
    }

    auto bit = function->begin();
    auto bend = function->end();

    auto& bb = *bit;

    auto it = bb->l_statements.begin();
    auto end = bb->l_statements.end();

    while(true){
        if(it == end){
            move_forward(bit, bend, it, end);
        }

        auto statement = *it;

        if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
            if((*ptr)->op != ltac::Operator::CMP_INT){
                if(!move_forward(bit, bend, it, end)){
                    return optimized;
                }

                continue;
            }

            auto temp_bit = bit;
            auto temp_it = it;
            auto temp_end = end;

            if(!move_forward(temp_bit, bend, temp_it, temp_end)){
                return optimized;
            }

            if(auto* jump_1_ptr = boost::get<std::shared_ptr<ltac::Jump>>(&*temp_it)){
                if(!move_forward(temp_bit, bend, temp_it, temp_end)){
                    return optimized;
                }

                if(auto* mov_1_ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&*temp_it)){
                    if((*mov_1_ptr)->op != ltac::Operator::MOV){
                        if(!move_forward(bit, bend, it, end)){
                            return optimized;
                        }

                        continue;
                    }

                    if(!move_forward(temp_bit, bend, temp_it, temp_end)){
                        return optimized;
                    }

                    if(boost::get<std::shared_ptr<ltac::Jump>>(&*temp_it)){
                        if(!move_forward(temp_bit, bend, temp_it, temp_end)){
                            return optimized;
                        }

                        if(boost::get<std::string>(&*temp_it)){
                            if(!move_forward(temp_bit, bend, temp_it, temp_end)){
                                return optimized;
                            }

                            if(auto* mov_2_ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&*temp_it)){
                                if((*mov_2_ptr)->op != ltac::Operator::MOV){
                                    if(!move_forward(bit, bend, it, end)){
                                        return optimized;
                                    }

                                    continue;
                                }

                                if(!move_forward(temp_bit, bend, temp_it, temp_end)){
                                    return optimized;
                                }

                                if(boost::get<std::string>(&*temp_it)){
                                    if(ltac::is_reg(*(*mov_1_ptr)->arg1) && ltac::is_reg(*(*mov_2_ptr)->arg1)){
                                        auto& reg1 = boost::get<ltac::Register>(*(*mov_1_ptr)->arg1); 
                                        auto& reg2 = boost::get<ltac::Register>(*(*mov_2_ptr)->arg1); 

                                        if(reg1 != reg2){
                                            if(!move_forward(bit, bend, it, end)){
                                                return optimized;
                                            }

                                            continue;
                                        }

                                        auto cmov_op = get_cmov_op((*jump_1_ptr)->type);

                                        move_forward(bit, bend, it, end);
                                        *it = *mov_1_ptr;
                                        move_forward(bit, bend, it, end);
                                        *it = std::make_shared<ltac::Instruction>(ltac::Operator::MOV, free_reg, *(*mov_2_ptr)->arg2);
                                        move_forward(bit, bend, it, end);
                                        *it = std::make_shared<ltac::Instruction>(cmov_op, reg1, free_reg);
                                        move_forward(bit, bend, it, end);
                                        
                                        *it = std::make_shared<ltac::Instruction>(ltac::Operator::NOP);
                                        move_forward(bit, bend, it, end);
                                        *it = std::make_shared<ltac::Instruction>(ltac::Operator::NOP);
                                        move_forward(bit, bend, it, end);
                                        *it = std::make_shared<ltac::Instruction>(ltac::Operator::NOP);
                                        move_forward(bit, bend, it, end);

                                        optimized = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if(!move_forward(bit, bend, it, end)){
            return optimized;
        }
    }

    return optimized;
}

bool debug(const std::string& name, bool b, mtac::function_p function){
    if(log::enabled<Debug>()){
        if(b){
            LOG<Debug>("Peephole") << name << " returned true" << log::endl;

            //Print the function
            ltac::Printer printer;
            printer.print(function);
        } else {
            LOG<Debug>("Peephole") << name << " returned false" << log::endl;
        }
    }

    return b;
}

} //end of anonymous namespace

void eddic::ltac::optimize(mtac::program_p program, Platform platform){
    PerfsTimer timer("Peephole optimizations");

    for(auto& function : program->functions){
        if(log::enabled<Debug>()){
            LOG<Debug>("Peephole") << "Start optimizations on " << function->get_name() << log::endl;

            //Print the function
            ltac::Printer printer;
            printer.print(function);
        }

        bool optimized;
        do {
            optimized = false;
            
            optimized |= debug("Basic optimizations", basic_optimizations(function, platform), function);
            optimized |= debug("Constant propagation", constant_propagation(function), function);
            optimized |= debug("Copy propagation", copy_propagation(function, platform), function);
            optimized |= debug("Dead-Code Elimination", dead_code_elimination(function), function);
            optimized |= debug("Conditional move", conditional_move(function, platform), function);
        } while(optimized);
    }
}
