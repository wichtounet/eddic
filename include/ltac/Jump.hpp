//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_JUMP_H
#define LTAC_JUMP_H

#include <vector>

#include "ltac/forward.hpp"

namespace eddic {

class Function;

namespace ltac {

enum struct JumpType : unsigned int {
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
    eddic::Function* target_function; //Only if a call
    
    std::vector<ltac::PseudoRegister> uses;
    std::vector<ltac::PseudoFloatRegister> float_uses;
    
    std::vector<ltac::PseudoRegister> kills;
    std::vector<ltac::PseudoFloatRegister> float_kills;
    
    std::vector<ltac::Register> hard_uses;
    std::vector<ltac::FloatRegister> hard_float_uses;
    
    std::vector<ltac::Register> hard_kills;
    std::vector<ltac::FloatRegister> hard_float_kills;

    Jump();
    Jump(const std::string& label, JumpType type);
};

} //end of ltac

} //end of eddic

#endif
