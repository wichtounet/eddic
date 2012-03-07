//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/ArithmeticIdentities.hpp"
#include "tac/OptimizerUtils.hpp"

using namespace eddic;
    
void tac::ArithmeticIdentities::operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        switch(quadruple->op){
            case tac::Operator::ADD:
                if(*quadruple->arg1 == 0){
                    replaceRight(*this, quadruple, *quadruple->arg2);
                } else if(*quadruple->arg2 == 0){
                    replaceRight(*this, quadruple, *quadruple->arg1);
                }

                break;
            case tac::Operator::SUB:
                if(*quadruple->arg2 == 0){
                    replaceRight(*this, quadruple, *quadruple->arg1);
                } 

                //a = b - b => a = 0
                else if(*quadruple->arg1 == *quadruple->arg2){
                    replaceRight(*this, quadruple, 0);
                }
                
                //a = 0 - b => a = -b
                else if(*quadruple->arg1 == 0){
                    replaceRight(*this, quadruple, *quadruple->arg2, tac::Operator::MINUS);
                }

                break;
            case tac::Operator::MUL:
                if(*quadruple->arg1 == 1){
                    replaceRight(*this, quadruple, *quadruple->arg2);
                } else if(*quadruple->arg2 == 1){
                    replaceRight(*this, quadruple, *quadruple->arg1);
                }
                
                else if(*quadruple->arg1 == 0){
                    replaceRight(*this, quadruple, 0);
                } else if(*quadruple->arg2 == 0){
                    replaceRight(*this, quadruple, 0);
                }
                
                else if(*quadruple->arg1 == -1){
                    replaceRight(*this, quadruple, *quadruple->arg2, tac::Operator::MINUS);
                } else if(*quadruple->arg2 == -1){
                    replaceRight(*this, quadruple, *quadruple->arg1, tac::Operator::MINUS);
                }

                break;
            case tac::Operator::DIV:
                if(*quadruple->arg2 == 1){
                    replaceRight(*this, quadruple, *quadruple->arg1);
                }

                else if(*quadruple->arg1 == 0){
                    replaceRight(*this, quadruple, 0);
                }

                //a = b / b => a = 1
                else if(*quadruple->arg1 == *quadruple->arg2){
                    replaceRight(*this, quadruple, 1);
                }
                
                else if(*quadruple->arg2 == 1){
                    replaceRight(*this, quadruple, *quadruple->arg1, tac::Operator::MINUS);
                }

                break;
            default:
                break;
        }
}
