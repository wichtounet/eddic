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
    }
}

void single_statement_optimizations(std::shared_ptr<ltac::Program> program){
    for(auto& function : program->functions){
        for(auto& statement : function->getStatements()){
            optimize_statement(statement);
        }
    }
}

void eddic::ltac::optimize(std::shared_ptr<ltac::Program> program){
    single_statement_optimizations(program);
}
