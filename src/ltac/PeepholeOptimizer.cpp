//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <boost/optional.hpp>

#include "Utils.hpp"

#include "ltac/PeepholeOptimizer.hpp"

#include "mtac/Utils.hpp"

using namespace eddic;

void optimize_statement(ltac::Statement& statement){
    if(boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
        auto instruction = boost::get<std::shared_ptr<ltac::Instruction>>(statement);

        if(instruction->op == ltac::Operator::MOV){
            //MOV reg, 0 can be transformed into XOR reg, reg
            if(mtac::is<ltac::Register>(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, 0)){
                instruction->op = ltac::Operator::XOR;
                instruction->arg2 = instruction->arg1;

                return;
            }

            if(mtac::is<ltac::Register>(*instruction->arg1) && mtac::is<ltac::Register>(*instruction->arg2)){
                auto& reg1 = boost::get<ltac::Register>(*instruction->arg1); 
                auto& reg2 = boost::get<ltac::Register>(*instruction->arg2); 
            
                //MOV reg, reg is useless
                if(reg1 == reg2){
                    instruction->op = ltac::Operator::NOP;
                    instruction->arg1.reset();
                    instruction->arg2.reset();
                }
            }
        }

        if(instruction->op == ltac::Operator::ADD){
            //ADD reg, 1 can be transformed into INC reg
            if(mtac::is<ltac::Register>(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, 1)){
                instruction->op = ltac::Operator::INC;
                instruction->arg2.reset();

                return;
            }
            
            //ADD reg, -1 can be transformed into DEC reg
            if(mtac::is<ltac::Register>(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, -1)){
                instruction->op = ltac::Operator::DEC;
                instruction->arg2.reset();

                return;
            }
        }
        
        if(instruction->op == ltac::Operator::SUB){
            //SUB reg, 1 can be transformed into DEC reg
            if(mtac::is<ltac::Register>(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, 1)){
                instruction->op = ltac::Operator::DEC;
                instruction->arg2.reset();

                return;
            }
            
            //SUB reg, -1 can be transformed into INC reg
            if(mtac::is<ltac::Register>(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, -1)){
                instruction->op = ltac::Operator::INC;
                instruction->arg2.reset();

                return;
            }
        }

        if(instruction->op == ltac::Operator::MUL){
            //Optimize multiplications with SHIFTs or LEAs
            if(mtac::is<ltac::Register>(*instruction->arg1) && mtac::is<int>(*instruction->arg2)){
                int constant = boost::get<int>(*instruction->arg2);

                auto reg = boost::get<ltac::Register>(*instruction->arg1);
        
                if(isPowerOfTwo(constant)){
                    instruction->op = ltac::Operator::SHIFT_LEFT;
                    instruction->arg2 = powerOfTwo(constant);

                    return;
                } 
                
                if(constant == 3){
                    instruction->op = ltac::Operator::LEA;
                    instruction->arg2 = ltac::Address(reg, reg, 2, 0);

                    return;
                } 
                
                if(constant == 5){
                    instruction->op = ltac::Operator::LEA;
                    instruction->arg2 = ltac::Address(reg, reg, 4, 0);

                    return;
                } 
                
                if(constant == 9){
                    instruction->op = ltac::Operator::LEA;
                    instruction->arg2 = ltac::Address(reg, reg, 8, 0);

                    return;
                } 
            }
        }

        if(instruction->op == ltac::Operator::CMP_INT){
            //Optimize comparisons with 0 with or reg, reg
            if(mtac::is<ltac::Register>(*instruction->arg1) && mtac::equals<int>(*instruction->arg2, 0)){
                instruction->op = ltac::Operator::OR;
                instruction->arg2 = instruction->arg1;

                return;
            }
        }
    }
}

void single_statement_optimizations(std::shared_ptr<ltac::Program> program){
    for(auto& function : program->functions){
        for(auto& statement : function->getStatements()){
            optimize_statement(statement);
        }
    }
}

void multiple_statement_optimizations(std::shared_ptr<ltac::Program> program){
    for(auto& function : program->functions){
        auto& statements = function->getStatements();

        for(size_t i = 1; i < statements.size(); ++i){
            auto& s1 = statements[i -  1];
            auto& s2 = statements[i];

            if(mtac::is<std::shared_ptr<ltac::Instruction>>(s1) && mtac::is<std::shared_ptr<ltac::Instruction>>(s2)){
                auto& i1 = boost::get<std::shared_ptr<ltac::Instruction>>(s1);
                auto& i2 = boost::get<std::shared_ptr<ltac::Instruction>>(s2);

                //The seconde LEAVE is dead
                if(i1->op == ltac::Operator::LEAVE && i2->op == ltac::Operator::LEAVE){
                    i2->op = ltac::Operator::NOP;
                }

                //Combine two FREE STACK into one
                if(i1->op == ltac::Operator::FREE_STACK && i2->op == ltac::Operator::FREE_STACK){
                    i1->arg1 = boost::get<int>(*i1->arg1) + boost::get<int>(*i2->arg1);
                    i2->arg1.reset();
                    i2->op = ltac::Operator::NOP;
                }

                if(i1->op == ltac::Operator::MOV && i2->op == ltac::Operator::MOV){
                    if(boost::get<ltac::Register>(&*i1->arg1) && boost::get<ltac::Register>(&*i2->arg1)){
                        if(boost::get<ltac::Register>(*i1->arg1) == boost::get<ltac::Register>(*i2->arg1)){
                            i1->op = ltac::Operator::NOP;
                        }
                    }
                }
                
                if(i1->op == ltac::Operator::MOV && i2->op == ltac::Operator::ADD){
                    if(boost::get<ltac::Register>(&*i1->arg1) && boost::get<ltac::Register>(&*i2->arg1)){
                        if(boost::get<ltac::Register>(*i1->arg1) == boost::get<ltac::Register>(*i2->arg1)){
                            if(boost::get<ltac::Register>(&*i1->arg2) && boost::get<int>(&*i2->arg2)){
                                i2->op = ltac::Operator::LEA;
                                i2->arg2 = ltac::Address(boost::get<ltac::Register>(*i1->arg2), boost::get<int>(*i2->arg2));
                                
                                i1->op = ltac::Operator::NOP;
                                i1->arg1.reset();
                                i1->arg2.reset();
                            } else if(boost::get<std::string>(&*i1->arg2) && boost::get<int>(&*i2->arg2)){
                                i2->op = ltac::Operator::LEA;
                                i2->arg2 = ltac::Address(boost::get<std::string>(*i1->arg2), boost::get<int>(*i2->arg2));
                                
                                i1->op = ltac::Operator::NOP;
                                i1->arg1.reset();
                                i1->arg2.reset();
                            }
                        }
                    }
                }
            }
        }
    }
}

void eddic::ltac::optimize(std::shared_ptr<ltac::Program> program){
    //Optimizations that looks at only one statement
    single_statement_optimizations(program);

    //Optimizations that looks at several statements at once
    multiple_statement_optimizations(program);
}
