//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_OPERATOR_H
#define AST_OPERATOR_H

#include <string>

namespace eddic {

namespace ast {

enum class Operator : unsigned int {
    ASSIGN,
    ADD,
    SUB,
    DIV,
    MUL,
    MOD,

    AND,
    OR,

    DEC,
    INC,

    EQUALS,
    NOT_EQUALS,
    LESS,
    LESS_EQUALS,
    GREATER,
    GREATER_EQUALS
};

std::string toString(Operator op);
std::ostream& operator<< (std::ostream& stream, Operator);

} //end of ast

} //end of eddic

#endif
