//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_OPERATOR_H
#define TAC_OPERATOR_H

#include "ast/Operator.hpp"

namespace eddic {

namespace tac {

enum class Operator : unsigned int {
    /* Assignment operators  */
    ASSIGN,
    FASSIGN,

    /* Integer operators */
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,

    /* Float operators  */
    FADD,
    FSUB,
    FMUL,
    FDIV,

    /* relational operators */
    EQUALS,
    NOT_EQUALS,
    GREATER,
    GREATER_EQUALS,
    LESS,
    LESS_EQUALS,

    MINUS,          //result = -arg1
    
    DOT,            //result = (arg1)+arg2
    DOT_ASSIGN,     //result+arg1=arg2
    
    ARRAY,          //result=arg1[arg2]
    ARRAY_ASSIGN,   //result[arg1]=arg2

    PARAM,          //push a single value

    RETURN          //return from a function
};

tac::Operator toOperator(ast::Operator op);
tac::Operator toFloatOperator(ast::Operator op);

} //end of tac

} //end of eddic

#endif
