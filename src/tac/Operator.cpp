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
            assert(false && "Not an int operator");
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
            assert(false && "Not a float operator");
    }
}

tac::Operator tac::toRelationalOperator(ast::Operator op){
    switch(op){
        case ast::Operator::EQUALS:
            return tac::Operator::EQUALS;
        case ast::Operator::NOT_EQUALS:
            return tac::Operator::NOT_EQUALS;
        case ast::Operator::LESS:
            return tac::Operator::LESS;
        case ast::Operator::LESS_EQUALS:
            return tac::Operator::LESS_EQUALS;
        case ast::Operator::GREATER:
            return tac::Operator::GREATER;
        case ast::Operator::GREATER_EQUALS:
            return tac::Operator::GREATER_EQUALS;
        default:
            assert(false && "Not a relational operator");
    }
}

tac::Operator tac::toFloatRelationalOperator(ast::Operator op){
    switch(op){
        case ast::Operator::EQUALS:
            return tac::Operator::FE;
        case ast::Operator::NOT_EQUALS:
            return tac::Operator::FNE;
        case ast::Operator::LESS:
            return tac::Operator::FL;
        case ast::Operator::LESS_EQUALS:
            return tac::Operator::FLE;
        case ast::Operator::GREATER:
            return tac::Operator::FG;
        case ast::Operator::GREATER_EQUALS:
            return tac::Operator::FGE;
        default:
            assert(false && "Not a float relational operator");
    }
}
