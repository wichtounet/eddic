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

ltac::Address::Address(const std::string& absolute, ltac::AddressRegister reg) : base_register(reg), absolute(absolute) {
    //Nothing to init    
}

ltac::Address::Address(const std::string& absolute, int displacement) : displacement(displacement), absolute(absolute) {
    //Nothing to init    
}

ltac::Address::Address(ltac::AddressRegister reg, int displacement) : base_register(reg), displacement(displacement) {
    //Nothing to init    
}

ltac::Address::Address(ltac::AddressRegister reg, ltac::AddressRegister scaled) : base_register(reg), scaled_register(scaled) {
    //Nothing to init    
}

ltac::Address::Address(ltac::AddressRegister reg, ltac::AddressRegister scaled, unsigned scale, int displacement) : base_register(reg), scaled_register(scaled), scale(scale), displacement(displacement){
    //Nothing to init    
}

template<typename Opt>
bool compare(Opt& o1, Opt& o2){
    if(o1.is_initialized()){
        return o2.is_initialized() && *o1 == *o2;
    } else {
        return !o2.is_initialized();
    }
}

bool ltac::operator==(ltac::Address& lhs, ltac::Address& rhs){
    return  compare(lhs.base_register, rhs.base_register) && 
            compare(lhs.scaled_register, rhs.scaled_register) && 
            compare(lhs.scale, rhs.scale) && 
            compare(lhs.displacement, rhs.displacement) && 
            compare(lhs.absolute, rhs.absolute);
}

bool ltac::operator!=(ltac::Address& lhs, ltac::Address& rhs){
    return !(lhs == rhs);
}
