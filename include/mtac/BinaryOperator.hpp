//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_BINARY_OPERATOR_H
#define MTAC_BINARY_OPERATOR_H

#include <string>

#include "ast/Operator.hpp"

namespace eddic {

namespace mtac {

enum class BinaryOperator : unsigned int {
    /* relational operators */
    EQUALS,
    NOT_EQUALS,
    GREATER,
    GREATER_EQUALS,
    LESS,
    LESS_EQUALS,
    
    /* float relational operators */
    FE,
    FNE,
    FG,
    FGE,
    FLE,
    FL
};

BinaryOperator toBinaryOperator(ast::Operator op);
BinaryOperator toFloatBinaryOperator(ast::Operator op);

} //end of mtac

} //end of eddic

#endif
