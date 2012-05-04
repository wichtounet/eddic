//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/Address.hpp"

using namespace eddic;

ltac::Address::Address(){
    //Nothing to init    
}

ltac::Address::Address(int displacement) : displacement(displacement) {
    //Nothing to init    
}

ltac::Address::Address(const std::string& absolute) : absolute(absolute) {
    //Nothing to init    
}

ltac::Address::Address(const std::string& absolute, ltac::Register reg) : base_register(reg), absolute(absolute) {
    //Nothing to init    
}

ltac::Address::Address(const std::string& absolute, int displacement) : displacement(displacement), absolute(absolute) {
    //Nothing to init    
}

ltac::Address::Address(ltac::Register reg, int displacement) : base_register(reg), displacement(displacement) {
    //Nothing to init    
}

ltac::Address::Address(ltac::Register reg, ltac::Register scaled) : base_register(reg), scaled_register(scaled) {
    //Nothing to init    
}

ltac::Address::Address(ltac::Register reg, ltac::Register scaled, int displacement) : base_register(reg), scaled_register(scaled), displacement(displacement) {
    //Nothing to init    
}

ltac::Address::Address(ltac::Register reg, ltac::Register scaled, unsigned scale, int displacement) : base_register(reg), scaled_register(scaled), scale(scale), displacement(displacement){
    //Nothing to init    
}
