//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <boost/optional.hpp>

#include "Utils.hpp"
#include "PerfsTimer.hpp"
#include "Options.hpp"
#include "likely.hpp"

#include "ltac/PeepholeOptimizer.hpp"
#include "ltac/Printer.hpp"

#include "mtac/Utils.hpp"

using namespace eddic;

namespace {

template<typename T>
inline bool is_reg(T value){
    return mtac::is<ltac::Register>(value);
}

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

        if(instruction->op == ltac::Operator::MOV){
            //MOV reg, 0 can be transformed into XOR reg, reg
            if(is_reg(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, 0)){
                instruction->op = ltac::Operator::XOR;
                instruction->arg2 = instruction->arg1;

                return true;
            }

            if(is_reg(*instruction->arg1) && is_reg(*instruction->arg2)){
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
            if(is_reg(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, 1)){
                instruction->op = ltac::Operator::INC;
                instruction->arg2.reset();

                return true;
            }
            
            //ADD reg, -1 can be transformed into DEC reg
            if(is_reg(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, -1)){
                instruction->op = ltac::Operator::DEC;
                instruction->arg2.reset();

                    return true;
            }
        }
        
        if(instruction->op == ltac::Operator::SUB){
            //SUB reg, 1 can be transformed into DEC reg
            if(is_reg(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, 1)){
                instruction->op = ltac::Operator::DEC;
                instruction->arg2.reset();

                    return true;
            }
            
            //SUB reg, -1 can be transformed into INC reg
            if(is_reg(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, -1)){
                instruction->op = ltac::Operator::INC;
                instruction->arg2.reset();

                    return true;
            }
        }

        if(instruction->op == ltac::Operator::MUL){
            //Optimize multiplications with SHIFTs or LEAs
            if(is_reg(*instruction->arg1) && mtac::is<int>(*instruction->arg2)){
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
            if(is_reg(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, 0)){
                instruction->op = ltac::Operator::OR;
                instruction->arg2 = instruction->arg1;

                    return true;
            }
        }
    }

    return false;
}

inline bool multiple_statement_optimizations(ltac::Statement& s1, ltac::Statement& s2){
    if(mtac::is<std::shared_ptr<ltac::Instruction>>(s1) && mtac::is<std::shared_ptr<ltac::Instruction>>(s2)){
        auto& i1 = boost::get<std::shared_ptr<ltac::Instruction>>(s1);
        auto& i2 = boost::get<std::shared_ptr<ltac::Instruction>>(s2);

        //Statements after LEAVE are dead
        if(i1->op == ltac::Operator::LEAVE){
            return transform_to_nop(i2);
        }

        //Combine two FREE STACK into one
        if(i1->op == ltac::Operator::FREE_STACK && i2->op == ltac::Operator::FREE_STACK){
            i1->arg1 = boost::get<int>(*i1->arg1) + boost::get<int>(*i2->arg1);
            
            return transform_to_nop(i2);
        }

        if(i1->op == ltac::Operator::MOV && i2->op == ltac::Operator::MOV){
            if(is_reg(*i1->arg1) && is_reg(*i1->arg2) && is_reg(*i2->arg1) && is_reg(*i2->arg2)){
                auto reg11 = boost::get<ltac::Register>(*i1->arg1);
                auto reg12 = boost::get<ltac::Register>(*i1->arg2);
                auto reg21 = boost::get<ltac::Register>(*i2->arg1);
                auto reg22 = boost::get<ltac::Register>(*i2->arg2);

                //cross MOV (ir4 = ir5, ir5 = ir4), keep only the first
                if (reg11 == reg22 && reg12 == reg21){
                    return transform_to_nop(i2);
                }
            } else if(is_reg(*i1->arg1) && is_reg(*i2->arg1)){
                auto reg11 = boost::get<ltac::Register>(*i1->arg1);
                auto reg21 = boost::get<ltac::Register>(*i2->arg1);

                //Two MOV to the same register => keep only last MOV
                if(reg11 == reg21){
                    return transform_to_nop(i1);
                }
            } else if(is_reg(*i1->arg1) && is_reg(*i2->arg2)){
                if(boost::get<ltac::Address>(&*i1->arg2) && boost::get<ltac::Address>(&*i2->arg1)){
                    if(boost::get<ltac::Address>(*i1->arg2) == boost::get<ltac::Address>(*i2->arg1)){
                        return transform_to_nop(i2);
                    }
                }
            } else if(is_reg(*i1->arg2) && is_reg(*i2->arg1)){
                if(boost::get<ltac::Address>(&*i1->arg1) && boost::get<ltac::Address>(&*i2->arg2)){
                    if(boost::get<ltac::Address>(*i1->arg1) == boost::get<ltac::Address>(*i2->arg2)){
                        return transform_to_nop(i2);
                    }
                }
            }
        }

        if(i1->op == ltac::Operator::MOV && i2->op == ltac::Operator::ADD){
            if(is_reg(*i1->arg1) && is_reg(*i2->arg1)){
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

    auto it = statements.begin();
    auto end = statements.end() - 1;

    bool optimized = false;

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

            continue;
        }

        ++it;
    }

    return optimized;
}

bool constant_propagation(std::shared_ptr<ltac::Function> function){
    bool optimized = false;

    auto& statements = function->getStatements();
    
    std::size_t bb = 0;

    while(bb < statements.size()){
        std::unordered_map<ltac::Register, int, ltac::RegisterHash> constants; 
        
        //Collect informations
        std::size_t i = bb;
        for(; i < statements.size(); ++i){
            auto statement = statements[i];

            if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                auto instruction = *ptr;

                if(instruction->op == ltac::Operator::XOR){
                    if(is_reg(*instruction->arg1) && is_reg(*instruction->arg2)){
                        auto reg1 = boost::get<ltac::Register>(*instruction->arg1);
                        auto reg2 = boost::get<ltac::Register>(*instruction->arg2);

                        if(reg1 == reg2){
                            constants[reg1] = 0;
                        }
                    }
                } else if(instruction->op == ltac::Operator::MOV){
                    if(is_reg(*instruction->arg1)){
                        if (auto* valuePtr = boost::get<int>(&*instruction->arg2)){
                            auto reg1 = boost::get<ltac::Register>(*instruction->arg1);

                            constants[reg1] = *valuePtr;
                        }
                    }
                }
            } else {
                //At this point, the basic block is at its end
                break;
            }
        }

        //Optimize the current basic block
        for(std::size_t j = bb; j < i + 1 && j < statements.size(); ++j){
            auto statement = statements[j];

            if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                auto instruction = *ptr;

                if(instruction->op == ltac::Operator::MOV){
                    if(is_reg(*instruction->arg2)){
                        auto reg2 = boost::get<ltac::Register>(*instruction->arg2);

                        if(constants.find(reg2) != constants.end()){
                            instruction->arg2 = constants[reg2];
                            optimized = true;
                        }
                    }
                }
            } 
        }

        //Start optimizations for the next basic block
        bb = i + 1;
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
    PerfsTimer timer("Peephole optimizations", true);

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
        } while(optimized);
    }
}
