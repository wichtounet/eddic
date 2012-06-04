//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/ConstantFolding.hpp"
#include "mtac/OptimizerUtils.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"
#include "mtac/If.hpp"
#include "mtac/IfFalse.hpp"

using namespace eddic;
    
void mtac::ConstantFolding::operator()(std::shared_ptr<mtac::Quadruple>& quadruple){
    if(quadruple->arg1){
        if(mtac::isInt(*quadruple->arg1)){
            if(quadruple->op == mtac::Operator::MINUS){
                replaceRight(*this, quadruple, -1 * boost::get<int>(*quadruple->arg1), mtac::Operator::ASSIGN);
            } else if(quadruple->op == mtac::Operator::I2F){
                replaceRight(*this, quadruple, (float) boost::get<int>(*quadruple->arg1), mtac::Operator::FASSIGN);
            } else if(quadruple->arg2 && mtac::isInt(*quadruple->arg2)){
                int lhs = boost::get<int>(*quadruple->arg1); 
                int rhs = boost::get<int>(*quadruple->arg2); 

                switch(quadruple->op){
                    case mtac::Operator::ADD:
                        replaceRight(*this, quadruple, lhs + rhs, mtac::Operator::ASSIGN);
                        break;
                    case mtac::Operator::SUB:
                        replaceRight(*this, quadruple, lhs - rhs, mtac::Operator::ASSIGN);
                        break;
                    case mtac::Operator::MUL:
                        replaceRight(*this, quadruple, lhs * rhs, mtac::Operator::ASSIGN);
                        break;
                    case mtac::Operator::DIV:
                        replaceRight(*this, quadruple, lhs / rhs, mtac::Operator::ASSIGN);
                        break;
                    case mtac::Operator::MOD:
                        replaceRight(*this, quadruple, lhs % rhs, mtac::Operator::ASSIGN);
                        break;
                    default:
                        break;
                }
            }
        } else if(mtac::isFloat(*quadruple->arg1)){
            if(quadruple->op == mtac::Operator::FMINUS){
                replaceRight(*this, quadruple, -1 * boost::get<double>(*quadruple->arg1), mtac::Operator::FASSIGN);
            } else if(quadruple->op == mtac::Operator::F2I){
                replaceRight(*this, quadruple, (int) boost::get<double>(*quadruple->arg1), mtac::Operator::ASSIGN);
            } else if(quadruple->arg2 && mtac::isInt(*quadruple->arg2)){
                double lhs = boost::get<double>(*quadruple->arg1); 
                double rhs = boost::get<double>(*quadruple->arg2); 

                switch(quadruple->op){
                    case mtac::Operator::FADD:
                        replaceRight(*this, quadruple, lhs + rhs, mtac::Operator::FASSIGN);
                        break;
                    case mtac::Operator::FSUB:
                        replaceRight(*this, quadruple, lhs - rhs, mtac::Operator::FASSIGN);
                        break;
                    case mtac::Operator::FMUL:
                        replaceRight(*this, quadruple, lhs * rhs, mtac::Operator::FASSIGN);
                        break;
                    case mtac::Operator::FDIV:
                        replaceRight(*this, quadruple, lhs / rhs, mtac::Operator::FASSIGN);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

namespace {

template<typename T>
bool computeValueInt(T& if_){
    int left = boost::get<int>(if_->arg1);
    int right = boost::get<int>(*if_->arg2);

    switch(*if_->op){
        case mtac::BinaryOperator::EQUALS:
            return left == right;
        case mtac::BinaryOperator::NOT_EQUALS:
            return left != right;
        case mtac::BinaryOperator::LESS:
            return left < right;
        case mtac::BinaryOperator::LESS_EQUALS:
            return left <= right;
        case mtac::BinaryOperator::GREATER:
            return left > right;
        case mtac::BinaryOperator::GREATER_EQUALS:
            return left >= right;
        default:
            assert(false);
    }
}

template<typename T>
bool computeValueFloat(T& if_){
    double left = boost::get<double>(if_->arg1);
    double right = boost::get<double>(*if_->arg2);

    switch(*if_->op){
        case mtac::BinaryOperator::FE:
            return left == right;
        case mtac::BinaryOperator::FNE:
            return left != right;
        case mtac::BinaryOperator::FL:
            return left < right;
        case mtac::BinaryOperator::FLE:
            return left <= right;
        case mtac::BinaryOperator::FG:
            return left > right;
        case mtac::BinaryOperator::FGE:
            return left >= right;
        default:
            assert(false);
    }
}

} //End of anonymous namespace

void mtac::ConstantFolding::operator()(std::shared_ptr<mtac::IfFalse>& ifFalse){
    if(ifFalse->op){
        if(mtac::isInt(ifFalse->arg1) && mtac::isInt(*ifFalse->arg2)){
            bool value = computeValueInt(ifFalse);

            ifFalse->op.reset();
            ifFalse->arg1 = value ? 1 : 0;
            ifFalse->arg2.reset();

            optimized = true;
        } else if(mtac::isFloat(ifFalse->arg1) && mtac::isFloat(*ifFalse->arg2)){
            bool value = computeValueFloat(ifFalse);

            ifFalse->op.reset();
            ifFalse->arg1 = value ? 1 : 0;
            ifFalse->arg2.reset();

            optimized = true;
        }
    }
}

void mtac::ConstantFolding::operator()(std::shared_ptr<mtac::If>& if_){
    if(if_->op){
        if(mtac::isInt(if_->arg1) && mtac::isInt(*if_->arg2)){
            bool value = computeValueInt(if_);

            if_->op.reset();
            if_->arg1 = value ? 1 : 0;
            if_->arg2.reset();

            optimized = true;
        } else if(mtac::isFloat(if_->arg1) && mtac::isFloat(*if_->arg2)){
            bool value = computeValueFloat(if_);

            if_->op.reset();
            if_->arg1 = value ? 1 : 0;
            if_->arg2.reset();

            optimized = true;
        }
    }
}
