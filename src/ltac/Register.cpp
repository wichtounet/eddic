//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/Register.hpp"

using namespace eddic;

ltac::Register::Register(){
    //Nothing to init
}

ltac::Register::Register(unsigned short reg) : reg(reg) {
    //Nothing to init
}

ltac::Register::operator int(){
    return reg;
}
    
bool ltac::Register::operator<(const Register& rhs) const {
    return reg > rhs.reg;
}

bool ltac::Register::operator>(const Register& rhs) const {
    return reg < rhs.reg;
}

bool ltac::Register::operator==(const Register& rhs) const {
    return reg == rhs.reg;
}

bool ltac::Register::operator!=(const Register& rhs) const {
    return !(*this == rhs); 
}

std::ostream& ltac::operator<<(std::ostream& out, const ltac::Register& reg){
    return out << "r" << reg.reg;
}
