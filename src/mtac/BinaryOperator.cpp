//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cassert>

#include "mtac/BinaryOperator.hpp"

using namespace eddic;

mtac::BinaryOperator eddic::mtac::toBinaryOperator(ast::Operator op){
    if(op == ast::Operator::NOT_EQUALS){
        return mtac::BinaryOperator::NOT_EQUALS;
    } else if(op == ast::Operator::EQUALS){
        return mtac::BinaryOperator::EQUALS;
    } else if(op == ast::Operator::GREATER_EQUALS){
        return mtac::BinaryOperator::GREATER_EQUALS;
    } else if(op == ast::Operator::GREATER){
        return mtac::BinaryOperator::GREATER;
    } else if(op == ast::Operator::LESS_EQUALS){
        return mtac::BinaryOperator::LESS_EQUALS;
    } else if(op == ast::Operator::LESS){
        return mtac::BinaryOperator::LESS;
    }

    assert(false && "Not an int binary operator"); 
}

mtac::BinaryOperator eddic::mtac::toFloatBinaryOperator(ast::Operator op){
    if(op == ast::Operator::NOT_EQUALS){
        return mtac::BinaryOperator::FNE;
    } else if(op == ast::Operator::EQUALS){
        return mtac::BinaryOperator::FE;
    } else if(op == ast::Operator::GREATER_EQUALS){
        return mtac::BinaryOperator::FGE;
    } else if(op == ast::Operator::GREATER){
        return mtac::BinaryOperator::FG;
    } else if(op == ast::Operator::LESS_EQUALS){
        return mtac::BinaryOperator::FLE;
    } else if(op == ast::Operator::LESS){
        return mtac::BinaryOperator::FL;
    }

    assert(false && "Not a float binary operator"); 
}
