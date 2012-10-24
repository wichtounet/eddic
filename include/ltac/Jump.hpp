//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_JUMP_H
#define LTAC_JUMP_H

#include <vector>

#include "ltac/PseudoRegister.hpp"
#include "ltac/PseudoFloatRegister.hpp"

namespace eddic {

namespace ltac {

enum class JumpType : unsigned int {
    ALWAYS,

    CALL,

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
    NZ      //Not zero
};

struct Jump {
    std::string label;
    JumpType type;
    
    std::vector<ltac::PseudoRegister> uses;
    std::vector<ltac::PseudoFloatRegister> float_uses;

    Jump();
    Jump(const std::string& label, JumpType type);
};

} //end of ltac

} //end of eddic

#endif
