//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cassert>

#include "tac/Operator.hpp"

using namespace eddic;

tac::Operator tac::toOperator(ast::Operator op){
    switch(op){
        case ast::Operator::ADD:
            return tac::Operator::ADD;
        case ast::Operator::SUB:
            return tac::Operator::SUB;
        case ast::Operator::DIV:
            return tac::Operator::DIV;
        case ast::Operator::MUL:
            return tac::Operator::MUL;
        case ast::Operator::MOD:
            return tac::Operator::MOD;
        default:
            assert(false);
    }
}

tac::Operator tac::toFloatOperator(ast::Operator op){
    switch(op){
        case ast::Operator::ADD:
            return tac::Operator::FADD;
        case ast::Operator::SUB:
            return tac::Operator::FSUB;
        case ast::Operator::DIV:
            return tac::Operator::FDIV;
        case ast::Operator::MUL:
            return tac::Operator::FMUL;
        default:
            assert(false);
    }
}
