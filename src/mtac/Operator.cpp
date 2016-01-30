//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "mtac/Operator.hpp"

using namespace eddic;

mtac::Operator mtac::toOperator(ast::Operator op){
    switch(op){
        case ast::Operator::ADD:
            return mtac::Operator::ADD;
        case ast::Operator::SUB:
            return mtac::Operator::SUB;
        case ast::Operator::DIV:
            return mtac::Operator::DIV;
        case ast::Operator::MUL:
            return mtac::Operator::MUL;
        case ast::Operator::MOD:
            return mtac::Operator::MOD;
        default:
            cpp_unreachable("Not an int operator"); 
    }
}

mtac::Operator mtac::toFloatOperator(ast::Operator op){
    switch(op){
        case ast::Operator::ADD:
            return mtac::Operator::FADD;
        case ast::Operator::SUB:
            return mtac::Operator::FSUB;
        case ast::Operator::DIV:
            return mtac::Operator::FDIV;
        case ast::Operator::MUL:
            return mtac::Operator::FMUL;
        default:
            cpp_unreachable("Not a float operator"); 
    }
}

mtac::Operator mtac::toRelationalOperator(ast::Operator op){
    switch(op){
        case ast::Operator::EQUALS:
            return mtac::Operator::EQUALS;
        case ast::Operator::NOT_EQUALS:
            return mtac::Operator::NOT_EQUALS;
        case ast::Operator::LESS:
            return mtac::Operator::LESS;
        case ast::Operator::LESS_EQUALS:
            return mtac::Operator::LESS_EQUALS;
        case ast::Operator::GREATER:
            return mtac::Operator::GREATER;
        case ast::Operator::GREATER_EQUALS:
            return mtac::Operator::GREATER_EQUALS;
        default:
            cpp_unreachable("Not a relational operator"); 
    }
}

mtac::Operator mtac::toFloatRelationalOperator(ast::Operator op){
    switch(op){
        case ast::Operator::EQUALS:
            return mtac::Operator::FE;
        case ast::Operator::NOT_EQUALS:
            return mtac::Operator::FNE;
        case ast::Operator::LESS:
            return mtac::Operator::FL;
        case ast::Operator::LESS_EQUALS:
            return mtac::Operator::FLE;
        case ast::Operator::GREATER:
            return mtac::Operator::FG;
        case ast::Operator::GREATER_EQUALS:
            return mtac::Operator::FGE;
        default:
            cpp_unreachable("Not a float relational operator"); 
    }
}
