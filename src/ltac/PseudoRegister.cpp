//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "ltac/PseudoRegister.hpp"

using namespace eddic;

ltac::PseudoRegister::PseudoRegister() : reg(0), bound(false), binding(0) {
    //Nothing to init
}

ltac::PseudoRegister::PseudoRegister(unsigned short reg) : reg(reg), bound(false), binding(0) {
    //Nothing to init
}

ltac::PseudoRegister::PseudoRegister(unsigned short reg, unsigned short hard) : reg(reg), bound(true), binding(hard) {
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
    out << "pr" << reg.reg;

    if(reg.bound){
        out << "(ir" << reg.binding << ")";
    }

    return out;
}
