//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
    if(reg == ltac::BP){
        return out << "bp";
    } else if(reg == ltac::SP){
        return out << "sp";
    }

    return out << "r" << reg.reg;
}
