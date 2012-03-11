//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/ReduceInStrength.hpp"
#include "tac/OptimizerUtils.hpp"

using namespace eddic;

void tac::ReduceInStrength::operator()(std::shared_ptr<tac::Quadruple>& quadruple){
    switch(quadruple->op){
        case tac::Operator::MUL:
            if(*quadruple->arg1 == 2){
                replaceRight(*this, quadruple, *quadruple->arg2, tac::Operator::ADD, *quadruple->arg2);
            } else if(*quadruple->arg2 == 2){
                replaceRight(*this, quadruple, *quadruple->arg1, tac::Operator::ADD, *quadruple->arg1);
            }

            break;
        case tac::Operator::FMUL:
            if(*quadruple->arg1 == 2.0){
                replaceRight(*this, quadruple, *quadruple->arg2, tac::Operator::FADD, *quadruple->arg2);
            } else if(*quadruple->arg2 == 2.0){
                replaceRight(*this, quadruple, *quadruple->arg1, tac::Operator::FADD, *quadruple->arg1);
            }

            break;
        default:
            break;
    }
}
