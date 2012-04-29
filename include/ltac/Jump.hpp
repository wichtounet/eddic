//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_JUMP_H
#define LTAC_JUMP_H

#include "tac/Jump.hpp"

namespace eddic {

namespace ltac {

class BasicBlock;

enum class JumpType : unsigned int {
    ALWAYS,

    //Egality
    NE,
    E,
    
    //signed comparisons
    GE,
    G,
    LE,
    L,

    //unsigned comparisons
    B,
    BE,
    A,
    AE,

    P,      //Parity
    Z,      //Zero
};

typedef tac::Jump<JumpType, BasicBlock> Jump;

} //end of ltac

} //end of eddic

#endif
