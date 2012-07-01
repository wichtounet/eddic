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

#include "ltac/PeepholeOptimizer.hpp"

#include "mtac/Utils.hpp"

using namespace eddic;

template<typename T>
inline bool is_reg(T value){
    return mtac::is<ltac::Register>(value);
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
                    instruction->op = ltac::Operator::NOP;
                    instruction->arg1.reset();
                    instruction->arg2.reset();
                    
                    return true;
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

        //The seconde LEAVE is dead
        if(i1->op == ltac::Operator::LEAVE && i2->op == ltac::Operator::LEAVE){
            i2->op = ltac::Operator::NOP;
            
            return true;
        }

        //Combine two FREE STACK into one
        if(i1->op == ltac::Operator::FREE_STACK && i2->op == ltac::Operator::FREE_STACK){
            i1->arg1 = boost::get<int>(*i1->arg1) + boost::get<int>(*i2->arg1);
            
            i2->op = ltac::Operator::NOP;
            i2->arg1.reset();
            
            return true;
        }

        if(i1->op == ltac::Operator::MOV && i2->op == ltac::Operator::MOV){
            if(is_reg(*i1->arg1) && is_reg(*i1->arg2) && is_reg(*i2->arg1) && is_reg(*i2->arg2)){
                auto reg11 = boost::get<ltac::Register>(*i1->arg1);
                auto reg12 = boost::get<ltac::Register>(*i1->arg2);
                auto reg21 = boost::get<ltac::Register>(*i2->arg1);
                auto reg22 = boost::get<ltac::Register>(*i2->arg2);

                //cross MOV (ir4 = ir5, ir5 = ir4), keep only the first
                if (reg11 == reg22 && reg12 == reg21){
                    i2->op = ltac::Operator::NOP;
                    
                    return true;
                }
            } else if(is_reg(*i1->arg1) && is_reg(*i2->arg1)){
                auto reg11 = boost::get<ltac::Register>(*i1->arg1);
                auto reg21 = boost::get<ltac::Register>(*i2->arg1);

                //Two MOV to the same register => keep only last MOV
                if(reg11 == reg21){
                    i1->op = ltac::Operator::NOP;
                    
                    return true;
                }
            } else if(is_reg(*i1->arg1) && is_reg(*i2->arg2)){
                if(boost::get<ltac::Address>(&*i1->arg2) && boost::get<ltac::Address>(&*i2->arg1)){
                    if(boost::get<ltac::Address>(*i1->arg2) == boost::get<ltac::Address>(*i2->arg1)){
                        i2->op = ltac::Operator::NOP;
                        i2->arg1.reset();
                        i2->arg2.reset();
                        
                        return true;
                    }
                }
            } else if(is_reg(*i1->arg2) && is_reg(*i2->arg1)){
                if(boost::get<ltac::Address>(&*i1->arg1) && boost::get<ltac::Address>(&*i2->arg2)){
                    if(boost::get<ltac::Address>(*i1->arg1) == boost::get<ltac::Address>(*i2->arg2)){
                        i2->op = ltac::Operator::NOP;
                        i2->arg1.reset();
                        i2->arg2.reset();
                        
                        return true;
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

                        i1->op = ltac::Operator::NOP;
                        i1->arg1.reset();
                        i1->arg2.reset();
                        
                        return true;
                    } else if(boost::get<std::string>(&*i1->arg2) && boost::get<int>(&*i2->arg2)){
                        i2->op = ltac::Operator::LEA;
                        i2->arg2 = ltac::Address(boost::get<std::string>(*i1->arg2), boost::get<int>(*i2->arg2));

                        i1->op = ltac::Operator::NOP;
                        i1->arg1.reset();
                        i1->arg2.reset();
                        
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

        //Optimizations that looks at several statements at once
        optimized |= multiple_statement_optimizations(s1, s2);

        if(is_nop(s1)){
            it = statements.erase(it);
            end = statements.end() - 1;

            continue;
        }

        ++it;
    }

    return optimized;
}

void eddic::ltac::optimize(std::shared_ptr<ltac::Program> program){
    PerfsTimer("Peephole optimizations", true);

    for(auto& function : program->functions){
        bool optimized;
        do {
            optimized = false;
            
            optimized |= basic_optimizations(function);
        } while(optimized);
    }
}
