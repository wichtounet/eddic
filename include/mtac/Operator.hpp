//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_OPERATOR_H
#define MTAC_OPERATOR_H

#include "ast/Operator.hpp"

namespace eddic {

namespace mtac {

enum class Operator : unsigned int {
    /* Assignment operators  */
    ASSIGN,
    FASSIGN,
    PASSIGN,

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

    /* relational operators for expressions */
    EQUALS,
    NOT_EQUALS,
    GREATER,
    GREATER_EQUALS,
    LESS,
    LESS_EQUALS,
    
    /* Operators for If */

    IF_UNARY,

    /* relational operators */
    IF_EQUALS,
    IF_NOT_EQUALS,
    IF_GREATER,
    IF_GREATER_EQUALS,
    IF_LESS,
    IF_LESS_EQUALS,
    
    /* float relational operators */
    IF_FE,
    IF_FNE,
    IF_FG,
    IF_FGE,
    IF_FLE,
    IF_FL,

    /* Operators for IfFalse */
    
    IF_FALSE_UNARY,

    /* relational operators */
    IF_FALSE_EQUALS,
    IF_FALSE_NOT_EQUALS,
    IF_FALSE_GREATER,
    IF_FALSE_GREATER_EQUALS,
    IF_FALSE_LESS,
    IF_FALSE_LESS_EQUALS,
    
    /* float relational operators */
    IF_FALSE_FE,
    IF_FALSE_FNE,
    IF_FALSE_FG,
    IF_FALSE_FGE,
    IF_FALSE_FLE,
    IF_FALSE_FL,

    /* boolean operators */
    NOT,
    AND,
    
    /* float relational operators */
    FE,
    FNE,
    FG,
    FGE,
    FLE,
    FL,

    MINUS,          //result = -arg1
    FMINUS,         //result = -arg1

    I2F,            //result = (float) arg1
    F2I,            //result = (int) arg1
    
    DOT,            //result = (arg1)+arg2
    FDOT,           //result = (arg1)+arg2
    PDOT,           //result = address of arg1 + arg2
    
    DOT_ASSIGN,     //result+arg1=arg2
    DOT_FASSIGN,    //result+arg1=arg2
    DOT_PASSIGN,    //result+arg1=arg2
    
    GOTO,           //jump to a basic block (label in arg1)

    RETURN,         //return from a function

    NOP,            //for optimization purpose

    PARAM,          //for parameter passing
    
    CALL,           //call functions

    LABEL           //label in arg1
};

mtac::Operator toOperator(ast::Operator op);
mtac::Operator toFloatOperator(ast::Operator op);

mtac::Operator toRelationalOperator(ast::Operator op);
mtac::Operator toFloatRelationalOperator(ast::Operator op);

} //end of mtac

} //end of eddic

#endif
