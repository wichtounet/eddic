//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/ConstantPropagation.hpp"
#include "mtac/OptimizerUtils.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

void tac::ConstantPropagation::optimize(tac::Argument* arg){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(arg)){
        if(int_constants.find(*ptr) != int_constants.end()){
            optimized = true;
            *arg = int_constants[*ptr];
        } else if(string_constants.find(*ptr) != string_constants.end()){
            optimized = true;
            *arg = string_constants[*ptr];
        } else if(float_constants.find(*ptr) != float_constants.end()){
            optimized = true;
            *arg = float_constants[*ptr];
        }
    }
}

void tac::ConstantPropagation::optimize_optional(boost::optional<tac::Argument>& arg){
    if(arg){
        optimize(&*arg);
    }
}

void tac::ConstantPropagation::operator()(std::shared_ptr<tac::Quadruple>& quadruple){
    //Do not replace a variable by a constant when used in offset
    if(quadruple->op != tac::Operator::ARRAY && quadruple->op != tac::Operator::DOT){
        optimize_optional(quadruple->arg1);
    }
    
    optimize_optional(quadruple->arg2);

    if(quadruple->op == tac::Operator::ASSIGN || quadruple->op == tac::Operator::FASSIGN){
        if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
            int_constants[quadruple->result] = *ptr;
        } else if(auto* ptr = boost::get<double>(&*quadruple->arg1)){
            float_constants[quadruple->result] = *ptr;
        } else if(auto* ptr = boost::get<std::string>(&*quadruple->arg1)){
            string_constants[quadruple->result] = *ptr;
        } else {
            //The result is not constant at this point
            int_constants.erase(quadruple->result);
            string_constants.erase(quadruple->result);
            float_constants.erase(quadruple->result);
        }
    } else {
        auto op = quadruple->op;

        //Check if the operator erase the contents of the result variable
        if(op != tac::Operator::ARRAY_ASSIGN && op != tac::Operator::DOT_ASSIGN && op != tac::Operator::RETURN){
            //The result is not constant at this point
            int_constants.erase(quadruple->result);
            string_constants.erase(quadruple->result);
            float_constants.erase(quadruple->result);
        }
    }
}

void tac::ConstantPropagation::operator()(std::shared_ptr<tac::Param>& param){
    optimize(&param->arg);
}

void tac::ConstantPropagation::operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
    optimize(&ifFalse->arg1);
    optimize_optional(ifFalse->arg2);
}

void tac::ConstantPropagation::operator()(std::shared_ptr<tac::If>& if_){
    optimize(&if_->arg1);
    optimize_optional(if_->arg2);
}
        
void tac::OffsetConstantPropagation::operator()(std::shared_ptr<tac::Quadruple>& quadruple){
    //Store the value assigned to result+arg1
    if(quadruple->op == tac::Operator::DOT_ASSIGN){
        if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
            tac::Offset offset;
            offset.variable = quadruple->result;
            offset.offset = *ptr;

            if(auto* ptr = boost::get<int>(&*quadruple->arg2)){
                int_constants[offset] = *ptr;
            } else if(auto* ptr = boost::get<std::string>(&*quadruple->arg2)){
                string_constants[offset] = *ptr;
            } else {
                //The result is not constant at this point
                int_constants.erase(offset);
                string_constants.erase(offset);
            }
        }
    }

    //If constant replace the value assigned to result by the value stored for arg1+arg2
    if(quadruple->op == tac::Operator::DOT){
        if(auto* ptr = boost::get<int>(&*quadruple->arg2)){
            tac::Offset offset;
            offset.variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);
            offset.offset = *ptr;

            if(int_constants.find(offset) != int_constants.end()){
                replaceRight(*this, quadruple, int_constants[offset], tac::Operator::ASSIGN);
            } else if(string_constants.find(offset) != string_constants.end()){
                replaceRight(*this, quadruple, string_constants[offset], tac::Operator::ASSIGN);
            }
        }
    }
}
