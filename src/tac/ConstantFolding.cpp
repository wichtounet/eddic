//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/ConstantFolding.hpp"
#include "tac/OptimizerUtils.hpp"
#include "tac/Utils.hpp"

using namespace eddic;
    
void tac::ConstantFolding::operator()(std::shared_ptr<tac::Quadruple>& quadruple){
    if(quadruple->arg1 && tac::isInt(*quadruple->arg1)){
        if(quadruple->op == tac::Operator::MINUS){
            replaceRight(*this, quadruple, -1 * boost::get<int>(*quadruple->arg1));
        } else if(quadruple->arg2 && tac::isInt(*quadruple->arg2)){
            int lhs = boost::get<int>(*quadruple->arg1); 
            int rhs = boost::get<int>(*quadruple->arg2); 

            switch(quadruple->op){
                case tac::Operator::ADD:
                    replaceRight(*this, quadruple, lhs + rhs);
                    break;
                case tac::Operator::SUB:
                    replaceRight(*this, quadruple, lhs - rhs);
                    break;
                case tac::Operator::MUL:
                    replaceRight(*this, quadruple, lhs * rhs);
                    break;
                case tac::Operator::DIV:
                    replaceRight(*this, quadruple, lhs / rhs);
                    break;
                case tac::Operator::MOD:
                    replaceRight(*this, quadruple, lhs % rhs);
                    break;
                default:
                    break;
            }
        }
    }
}

namespace {

template<typename T>
bool computeValue(T& if_){
    int left = boost::get<int>(if_->arg1);
    int right = boost::get<int>(*if_->arg2);

    switch(*if_->op){
        case tac::BinaryOperator::EQUALS:
            return left == right;
        case tac::BinaryOperator::NOT_EQUALS:
            return left != right;
        case tac::BinaryOperator::LESS:
            return left < right;
        case tac::BinaryOperator::LESS_EQUALS:
            return left <= right;
        case tac::BinaryOperator::GREATER:
            return left > right;
        case tac::BinaryOperator::GREATER_EQUALS:
            return left >= right;
        default:
            assert(false);
    }
}

} //End of anonymous namespace

void tac::ConstantFolding::operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
    if(ifFalse->op){
        if(tac::isInt(ifFalse->arg1) && tac::isInt(*ifFalse->arg2)){
            bool value = computeValue(ifFalse);

            ifFalse->op.reset();
            ifFalse->arg1 = value ? 1 : 0;
            ifFalse->arg2.reset();

            optimized = true;
        }
    }
}

void tac::ConstantFolding::operator()(std::shared_ptr<tac::If>& if_){
    if(if_->op){
        if(tac::isInt(if_->arg1) && tac::isInt(*if_->arg2)){
            bool value = computeValue(if_);

            if_->op.reset();
            if_->arg1 = value ? 1 : 0;
            if_->arg2.reset();

            optimized = true;
        }
    }
}
