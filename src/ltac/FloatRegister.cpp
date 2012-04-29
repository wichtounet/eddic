//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/FloatRegister.hpp"

using namespace eddic;

ltac::FloatRegister::FloatRegister(unsigned short reg) : reg(reg) {
    //Nothing to init
}

ltac::FloatRegister::operator int(){
    return reg;
}
