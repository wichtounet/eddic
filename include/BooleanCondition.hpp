//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOLEAN_CONDITION_H
#define BOOLEAN_CONDITION_H

namespace eddic {

enum BooleanCondition {
    GREATER_OPERATOR,
    LESS_OPERATOR,
    EQUALS_OPERATOR,
    NOT_EQUALS_OPERATOR,
    GREATER_EQUALS_OPERATOR,
    LESS_EQUALS_OPERATOR,
    TRUE_VALUE,
    FALSE_VALUE
};

} //end of eddic

#endif
