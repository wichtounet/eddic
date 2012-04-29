//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_OPERATOR_H
#define LTAC_OPERATOR_H

namespace eddic {

namespace ltac {

enum class Operator : unsigned int {
    MOV,

    //Set the memory to 0
    MEMSET,

    //For stack allocation
    ALLOC_STACK,
    FREE_STACK,

    //Comparisons
    CMP_INT,
    CMP_FLOAT,

    //Logical operations
    OR,

    //Stack manipulations
    PUSH,
    POP
};

} //end of ltac

} //end of eddic

#endif
