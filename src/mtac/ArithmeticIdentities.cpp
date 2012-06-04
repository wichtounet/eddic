//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/ArithmeticIdentities.hpp"
#include "mtac/OptimizerUtils.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;
    
void mtac::ArithmeticIdentities::operator()(std::shared_ptr<mtac::Quadruple>& quadruple){
        switch(quadruple->op){
            case mtac::Operator::ADD:
                if(*quadruple->arg1 == 0){
                    replaceRight(*this, quadruple, *quadruple->arg2, mtac::Operator::ASSIGN);
                } else if(*quadruple->arg2 == 0){
                    replaceRight(*this, quadruple, *quadruple->arg1, mtac::Operator::ASSIGN);
                }

                break;
            case mtac::Operator::SUB:
                if(*quadruple->arg2 == 0){
                    replaceRight(*this, quadruple, *quadruple->arg1, mtac::Operator::ASSIGN);
                } 

                //a = b - b => a = 0
                else if(*quadruple->arg1 == *quadruple->arg2){
                    replaceRight(*this, quadruple, 0, mtac::Operator::ASSIGN);
                }
                
                //a = 0 - b => a = -b
                else if(*quadruple->arg1 == 0){
                    replaceRight(*this, quadruple, *quadruple->arg2, mtac::Operator::MINUS);
                }

                break;
            case mtac::Operator::MUL:
                if(*quadruple->arg1 == 1){
                    replaceRight(*this, quadruple, *quadruple->arg2, mtac::Operator::ASSIGN);
                } else if(*quadruple->arg2 == 1){
                    replaceRight(*this, quadruple, *quadruple->arg1, mtac::Operator::ASSIGN);
                }
                
                else if(*quadruple->arg1 == 0){
                    replaceRight(*this, quadruple, 0, mtac::Operator::ASSIGN);
                } else if(*quadruple->arg2 == 0){
                    replaceRight(*this, quadruple, 0, mtac::Operator::ASSIGN);
                }
                
                else if(*quadruple->arg1 == -1){
                    replaceRight(*this, quadruple, *quadruple->arg2, mtac::Operator::MINUS);
                } else if(*quadruple->arg2 == -1){
                    replaceRight(*this, quadruple, *quadruple->arg1, mtac::Operator::MINUS);
                }

                break;
            case mtac::Operator::DIV:
                if(*quadruple->arg2 == 1){
                    replaceRight(*this, quadruple, *quadruple->arg1, mtac::Operator::ASSIGN);
                }
                
                else if(*quadruple->arg1 == 0){
                    replaceRight(*this, quadruple, 0, mtac::Operator::ASSIGN);
                }

                //a = b / b => a = 1
                else if(*quadruple->arg1 == *quadruple->arg2){
                    replaceRight(*this, quadruple, 1, mtac::Operator::ASSIGN);
                }
                
                else if(*quadruple->arg2 == -1){
                    replaceRight(*this, quadruple, *quadruple->arg1, mtac::Operator::MINUS);
                }

                break;
            case mtac::Operator::FADD:
                if(*quadruple->arg1 == 0.0){
                    replaceRight(*this, quadruple, *quadruple->arg2, mtac::Operator::FASSIGN);
                } else if(*quadruple->arg2 == 0.0){
                    replaceRight(*this, quadruple, *quadruple->arg1, mtac::Operator::FASSIGN);
                }

                break;
            case mtac::Operator::FSUB:
                if(*quadruple->arg2 == 0.0){
                    replaceRight(*this, quadruple, *quadruple->arg1, mtac::Operator::FASSIGN);
                } 

                //a = b - b => a = 0
                else if(*quadruple->arg1 == *quadruple->arg2){
                    replaceRight(*this, quadruple, 0.0, mtac::Operator::FASSIGN);
                }
                
                //a = 0 - b => a = -b
                else if(*quadruple->arg1 == 0.0){
                    replaceRight(*this, quadruple, *quadruple->arg2, mtac::Operator::FMINUS);
                }

                break;
            case mtac::Operator::FMUL:
                if(*quadruple->arg1 == 1.0){
                    replaceRight(*this, quadruple, *quadruple->arg2, mtac::Operator::FASSIGN);
                } else if(*quadruple->arg2 == 1.0){
                    replaceRight(*this, quadruple, *quadruple->arg1, mtac::Operator::FASSIGN);
                }
                
                else if(*quadruple->arg1 == 0.0){
                    replaceRight(*this, quadruple, 0.0, mtac::Operator::FASSIGN);
                } else if(*quadruple->arg2 == 0){
                    replaceRight(*this, quadruple, 0.0, mtac::Operator::FASSIGN);
                }
                
                else if(*quadruple->arg1 == -1.0){
                    replaceRight(*this, quadruple, *quadruple->arg2, mtac::Operator::FMINUS);
                } else if(*quadruple->arg2 == -1.0){
                    replaceRight(*this, quadruple, *quadruple->arg1, mtac::Operator::FMINUS);
                }

                break;
            case mtac::Operator::FDIV:
                if(*quadruple->arg2 == 1.0){
                    replaceRight(*this, quadruple, *quadruple->arg1, mtac::Operator::FASSIGN);
                }

                else if(*quadruple->arg1 == 0.0){
                    replaceRight(*this, quadruple, 0.0, mtac::Operator::FASSIGN);
                }

                //a = b / b => a = 1
                else if(*quadruple->arg1 == *quadruple->arg2){
                    replaceRight(*this, quadruple, 1.0, mtac::Operator::FASSIGN);
                }
                
                else if(*quadruple->arg2 == -1.0){
                    replaceRight(*this, quadruple, *quadruple->arg1, mtac::Operator::MINUS);
                }

                break;
            default:
                break;
        }
}
