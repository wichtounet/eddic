//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_BINARY_OPERATOR_H
#define TAC_BINARY_OPERATOR_H

#include <string>

namespace eddic {

namespace tac {

enum class BinaryOperator : unsigned int {
    EQUALS,
    NOT_EQUALS,
    LESS,
    LESS_EQUALS,
    GREATER,
    GREATER_EQUALS
};

BinaryOperator toBinaryOperator(const std::string& op);

} //end of tac

} //end of eddic

#endif
