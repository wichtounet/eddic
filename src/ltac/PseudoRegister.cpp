//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/PseudoRegister.hpp"

using namespace eddic;

ltac::PseudoRegister::PseudoRegister(){
    //Nothing to init
}

ltac::PseudoRegister::PseudoRegister(unsigned short reg) : reg(reg) {
    //Nothing to init
}

ltac::PseudoRegister::operator int(){
    return reg;
}
    
bool ltac::PseudoRegister::operator<(const PseudoRegister& rhs) const {
    return reg > rhs.reg;
}

bool ltac::PseudoRegister::operator>(const PseudoRegister& rhs) const {
    return reg < rhs.reg;
}

bool ltac::PseudoRegister::operator==(const PseudoRegister& rhs) const {
    return reg == rhs.reg;
}

bool ltac::PseudoRegister::operator!=(const PseudoRegister& rhs) const {
    return !(*this == rhs); 
}

std::ostream& ltac::operator<<(std::ostream& out, const ltac::PseudoRegister& reg){
    return out << "pr" << reg.reg;
}