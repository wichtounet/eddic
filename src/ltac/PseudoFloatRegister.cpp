//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/PseudoFloatRegister.hpp"

using namespace eddic;

ltac::PseudoFloatRegister::PseudoFloatRegister(){
    //Nothing to init
}

ltac::PseudoFloatRegister::PseudoFloatRegister(unsigned short reg) : reg(reg) {
    //Nothing to init
}

ltac::PseudoFloatRegister::PseudoFloatRegister(unsigned short reg, unsigned short hard) : reg(reg), bound(true), binding(hard) {
    //Nothing to init
}

ltac::PseudoFloatRegister::operator int(){
    return reg;
}

bool ltac::PseudoFloatRegister::operator<(const PseudoFloatRegister& rhs) const {
    return reg > rhs.reg;
}

bool ltac::PseudoFloatRegister::operator>(const PseudoFloatRegister& rhs) const {
    return reg < rhs.reg;
}

bool ltac::PseudoFloatRegister::operator==(const PseudoFloatRegister& rhs) const {
    return reg == rhs.reg;
}

bool ltac::PseudoFloatRegister::operator!=(const PseudoFloatRegister& rhs) const {
    return !(*this == rhs); 
}

std::ostream& ltac::operator<<(std::ostream& out, const ltac::PseudoFloatRegister& reg){
    return out << "pfr" << reg.reg;
}
