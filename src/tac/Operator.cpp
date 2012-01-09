//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cassert>

#include "tac/Operator.hpp"

using namespace eddic;

tac::Operator tac::toOperator(char op){
    switch(op){
        case '+':
            return tac::Operator::ADD;
        case '-':
            return tac::Operator::SUB;
        case '/':
            return tac::Operator::DIV;
        case '*':
            return tac::Operator::MUL;
        case '%':
            return tac::Operator::MOD;
        default:
            assert(false);
    }
}
