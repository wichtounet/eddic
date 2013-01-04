//=======================================================================
// Copyrhs Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"

#include "mtac/ConstantFolding.hpp"
#include "mtac/OptimizerUtils.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"
#include "mtac/If.hpp"
#include "mtac/IfFalse.hpp"

using namespace eddic;

bool transform_to_if_false_unary(std::shared_ptr<mtac::Quadruple> ifFalse, bool value){
    ifFalse->op = mtac::Operator::IF_FALSE_UNARY;
    ifFalse->arg1 = value ? 1 : 0;
    ifFalse->arg2.reset();

    return true;
}

bool transform_to_if_unary(std::shared_ptr<mtac::Quadruple> if_, bool value){
    if_->op = mtac::Operator::IF_UNARY;
    if_->arg1 = value ? 1 : 0;
    if_->arg2.reset();

    return true;
}
    
void mtac::ConstantFolding::operator()(std::shared_ptr<mtac::Quadruple> quadruple){
    if(quadruple->arg1){
        if(mtac::isInt(*quadruple->arg1)){
            if(quadruple->op == mtac::Operator::MINUS){
                replaceRight(*this, quadruple, -1 * boost::get<int>(*quadruple->arg1), mtac::Operator::ASSIGN);
            } else if(quadruple->op == mtac::Operator::NOT){
                int lhs = boost::get<int>(*quadruple->arg1);
                
                if(lhs == 0){
                    replaceRight(*this, quadruple, 1, mtac::Operator::ASSIGN);
                } else {
                    replaceRight(*this, quadruple, 0, mtac::Operator::ASSIGN);
                }
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
                    case mtac::Operator::GREATER:
                        replaceRight(*this, quadruple, static_cast<int>(lhs > rhs), mtac::Operator::ASSIGN);
                        break;
                    case mtac::Operator::GREATER_EQUALS:
                        replaceRight(*this, quadruple, static_cast<int>(lhs >= rhs), mtac::Operator::ASSIGN);
                        break;
                    case mtac::Operator::LESS:
                        replaceRight(*this, quadruple, static_cast<int>(lhs < rhs), mtac::Operator::ASSIGN);
                        break;
                    case mtac::Operator::LESS_EQUALS:
                        replaceRight(*this, quadruple, static_cast<int>(lhs <= rhs), mtac::Operator::ASSIGN);
                        break;
                    case mtac::Operator::EQUALS:
                        replaceRight(*this, quadruple, static_cast<int>(lhs == rhs), mtac::Operator::ASSIGN);
                        break;
                    case mtac::Operator::NOT_EQUALS:
                        replaceRight(*this, quadruple, static_cast<int>(lhs != rhs), mtac::Operator::ASSIGN);
                        break;
                    case mtac::Operator::IF_EQUALS:
                        optimized = transform_to_if_unary(quadruple, lhs == rhs);
                        break;
                    case mtac::Operator::IF_FALSE_EQUALS:
                        optimized = transform_to_if_false_unary(quadruple, lhs == rhs);
                        break;
                    case mtac::Operator::IF_NOT_EQUALS:
                        optimized = transform_to_if_unary(quadruple, lhs != rhs);
                        break;
                    case mtac::Operator::IF_FALSE_NOT_EQUALS:
                        optimized = transform_to_if_false_unary(quadruple, lhs != rhs);
                        break;
                    case mtac::Operator::IF_LESS:
                        optimized = transform_to_if_unary(quadruple, lhs < rhs);
                        break;
                    case mtac::Operator::IF_FALSE_LESS:
                        optimized = transform_to_if_false_unary(quadruple, lhs < rhs);
                        break;
                    case mtac::Operator::IF_LESS_EQUALS:
                        optimized = transform_to_if_unary(quadruple, lhs <= rhs);
                        break;
                    case mtac::Operator::IF_FALSE_LESS_EQUALS:
                        optimized = transform_to_if_false_unary(quadruple, lhs <= rhs);
                        break;
                    case mtac::Operator::IF_GREATER:
                        optimized = transform_to_if_unary(quadruple, lhs > rhs);
                        break;
                    case mtac::Operator::IF_FALSE_GREATER:
                        optimized = transform_to_if_false_unary(quadruple, lhs > rhs);
                        break;
                    case mtac::Operator::IF_GREATER_EQUALS:
                        optimized = transform_to_if_unary(quadruple, lhs >= rhs);
                        break;
                    case mtac::Operator::IF_FALSE_GREATER_EQUALS:
                        optimized = transform_to_if_false_unary(quadruple, lhs >= rhs);
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
            } else if(quadruple->arg2 && mtac::isFloat(*quadruple->arg2)){
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
                    case mtac::Operator::FG:
                        replaceRight(*this, quadruple, static_cast<int>(lhs > rhs), mtac::Operator::FASSIGN);
                        break;
                    case mtac::Operator::FGE:
                        replaceRight(*this, quadruple, static_cast<int>(lhs >= rhs), mtac::Operator::FASSIGN);
                        break;
                    case mtac::Operator::FL:
                        replaceRight(*this, quadruple, static_cast<int>(lhs < rhs), mtac::Operator::FASSIGN);
                        break;
                    case mtac::Operator::FLE:
                        replaceRight(*this, quadruple, static_cast<int>(lhs <= rhs), mtac::Operator::FASSIGN);
                        break;
                    case mtac::Operator::FE:
                        replaceRight(*this, quadruple, static_cast<int>(lhs == rhs), mtac::Operator::FASSIGN);
                        break;
                    case mtac::Operator::FNE:
                        replaceRight(*this, quadruple, static_cast<int>(lhs != rhs), mtac::Operator::FASSIGN);
                        break;
                    case mtac::Operator::IF_FE:
                        optimized = transform_to_if_unary(quadruple, lhs == rhs);
                        break;
                    case mtac::Operator::IF_FALSE_FE:
                        optimized = transform_to_if_false_unary(quadruple, lhs == rhs);
                        break;
                    case mtac::Operator::IF_FNE:
                        optimized = transform_to_if_unary(quadruple, lhs != rhs);
                        break;
                    case mtac::Operator::IF_FALSE_FNE:
                        optimized = transform_to_if_false_unary(quadruple, lhs != rhs);
                        break;
                    case mtac::Operator::IF_FL:
                        optimized = transform_to_if_unary(quadruple, lhs < rhs);
                        break;
                    case mtac::Operator::IF_FALSE_FL:
                        optimized = transform_to_if_false_unary(quadruple, lhs < rhs);
                        break;
                    case mtac::Operator::IF_FLE:
                        optimized = transform_to_if_unary(quadruple, lhs <= rhs);
                        break;
                    case mtac::Operator::IF_FALSE_FLE:
                        optimized = transform_to_if_false_unary(quadruple, lhs <= rhs);
                        break;
                    case mtac::Operator::IF_FG:
                        optimized = transform_to_if_unary(quadruple, lhs > rhs);
                        break;
                    case mtac::Operator::IF_FALSE_FG:
                        optimized = transform_to_if_false_unary(quadruple, lhs > rhs);
                        break;
                    case mtac::Operator::IF_FGE:
                        optimized = transform_to_if_unary(quadruple, lhs >= rhs);
                        break;
                    case mtac::Operator::IF_FALSE_FGE:
                        optimized = transform_to_if_false_unary(quadruple, lhs >= rhs);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}
