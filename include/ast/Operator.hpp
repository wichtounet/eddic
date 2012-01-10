//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_OPERATOR_H
#define AST_OPERATOR_H

namespace eddic {

namespace ast {

enum class Operator : unsigned int {
    ADD,
    SUB,
    DIV,
    MUL,
    MOD
};

} //end of ast

} //end of eddic

#endif
