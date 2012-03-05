//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cassert>

#include "tac/BinaryOperator.hpp"

using namespace eddic;

tac::BinaryOperator eddic::tac::toBinaryOperator(ast::Operator op){
    if(op == ast::Operator::NOT_EQUALS){
        return tac::BinaryOperator::NOT_EQUALS;
    } else if(op == ast::Operator::EQUALS){
        return tac::BinaryOperator::EQUALS;
    } else if(op == ast::Operator::GREATER_EQUALS){
        return tac::BinaryOperator::GREATER_EQUALS;
    } else if(op == ast::Operator::GREATER){
        return tac::BinaryOperator::GREATER;
    } else if(op == ast::Operator::LESS_EQUALS){
        return tac::BinaryOperator::LESS_EQUALS;
    } else if(op == ast::Operator::LESS){
        return tac::BinaryOperator::LESS;
    }

    assert(false && "Not an int binary operator"); 
}

tac::BinaryOperator eddic::tac::toFloatBinaryOperator(ast::Operator op){
    if(op == ast::Operator::NOT_EQUALS){
        return tac::BinaryOperator::FNE;
    } else if(op == ast::Operator::EQUALS){
        return tac::BinaryOperator::FE;
    } else if(op == ast::Operator::GREATER_EQUALS){
        return tac::BinaryOperator::FGE;
    } else if(op == ast::Operator::GREATER){
        return tac::BinaryOperator::FG;
    } else if(op == ast::Operator::LESS_EQUALS){
        return tac::BinaryOperator::FLE;
    } else if(op == ast::Operator::LESS){
        return tac::BinaryOperator::FL;
    }

    assert(false && "Not a float binary operator"); 
}
