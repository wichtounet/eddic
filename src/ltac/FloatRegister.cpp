//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "ltac/FloatRegister.hpp"

using namespace eddic;

ltac::FloatRegister::FloatRegister(){
    //Nothing to init
}

ltac::FloatRegister::FloatRegister(unsigned short reg) : reg(reg) {
    //Nothing to init
}

ltac::FloatRegister::operator int(){
    return reg;
}

bool ltac::FloatRegister::operator<(const FloatRegister& rhs) const {
    return reg > rhs.reg;
}

bool ltac::FloatRegister::operator>(const FloatRegister& rhs) const {
    return reg < rhs.reg;
}

bool ltac::FloatRegister::operator==(const FloatRegister& rhs) const {
    return reg == rhs.reg;
}

bool ltac::FloatRegister::operator!=(const FloatRegister& rhs) const {
    return !(*this == rhs); 
}

std::ostream& ltac::operator<<(std::ostream& out, const ltac::FloatRegister& reg){
    return out << "fr" << reg.reg;
}
