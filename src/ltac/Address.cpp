//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/Address.hpp"
#include "ltac/Register.hpp"

using namespace eddic;

ltac::Address::Address(){
    //Nothing to init    
}

ltac::Address::Address(unsigned displacement) : displacement(displacement) {
    //Nothing to init    
}

ltac::Address::Address(ltac::Register reg, unsigned displacement) : base_register(reg), displacement(displacement) {
    //Nothing to init    
}

ltac::Address::Address(ltac::Register reg, ltac::Register scaled) : base_register(reg), scaled_register(scaled) {
    //Nothing to init    
}

ltac::Address::Address(ltac::Register reg, ltac::Register scaled, unsigned displacement) : base_register(reg), scaled_register(scaled), displacement(displacement) {
    //Nothing to init    
}

ltac::Address::Address(ltac::Register reg, ltac::Register scaled, unsigned scale, unsigned displacement) : base_register(reg), scaled_register(scaled), scale(scale), displacement(displacement){
    //Nothing to init    
}
