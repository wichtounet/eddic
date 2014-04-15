//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "ltac/PseudoFloatRegister.hpp"

using namespace eddic;

ltac::PseudoFloatRegister::PseudoFloatRegister() : reg(0), bound(false), binding(0) {
    //Nothing to init
}

ltac::PseudoFloatRegister::PseudoFloatRegister(unsigned short reg) : reg(reg), bound(false), binding(0) {
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
