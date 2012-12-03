//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>

#include "ltac/Jump.hpp"
#include "ltac/Register.hpp"
#include "ltac/FloatRegister.hpp"
#include "ltac/PseudoRegister.hpp"
#include "ltac/PseudoFloatRegister.hpp"

using namespace eddic;

ltac::Jump::Jump(){
    //Nothing to init
}

ltac::Jump::Jump(const std::string& label, JumpType type) : label(label), type(type) {
    //Nothing to init
}
