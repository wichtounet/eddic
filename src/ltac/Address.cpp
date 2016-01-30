//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "ltac/Address.hpp"

using namespace eddic;

ltac::Address::Address(){
    //Nothing to init    
}

ltac::Address::Address(int displacement) : displacement(displacement) {
    //Nothing to init    
}

ltac::Address::Address(std::string absolute) : absolute(std::move(absolute)) {
    //Nothing to init    
}

ltac::Address::Address(std::string absolute, const ltac::AddressRegister& reg) : base_register(reg), absolute(std::move(absolute)) {
    //Nothing to init    
}

ltac::Address::Address(std::string absolute, int displacement) : displacement(displacement), absolute(std::move(absolute)) {
    //Nothing to init    
}

ltac::Address::Address(const ltac::AddressRegister& reg, int displacement) : base_register(reg), displacement(displacement) {
    //Nothing to init    
}

ltac::Address::Address(const ltac::AddressRegister& reg, const ltac::AddressRegister& scaled) : base_register(reg), scaled_register(scaled) {
    //Nothing to init    
}

ltac::Address::Address(const ltac::AddressRegister& reg, const ltac::AddressRegister& scaled, unsigned scale, int displacement) : base_register(reg), scaled_register(scaled), scale(scale), displacement(displacement){
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

std::ostream& ltac::operator<<(std::ostream& out, const ltac::Address& address){
    if(address.absolute){
        if(address.displacement){
            return out << "[" << *address.absolute << " + " << *address.displacement << "]";
        }

        if(address.base_register){
            return out << "[" << *address.absolute << " + " << *address.base_register << "]";
        }

        return out << "[" << *address.absolute << "]";
    }

    if(address.base_register){
        if(address.scaled_register){
            if(address.scale){
                if(address.displacement){
                    return out << "[" << *address.base_register << " + " << *address.scaled_register << " * " << *address.scale << " + " << *address.displacement << "]";
                }

                return out << "[" << *address.base_register << " + " << *address.scaled_register << " * " << *address.scale << "]";
            }

            if(address.displacement){
                return out << "[" << *address.base_register << " + " << *address.scaled_register << " + " << *address.displacement << "]";
            }

            return out << "[" << *address.base_register << " + " << *address.scaled_register << "]";
        }

        if(address.displacement){
            return out << "[" << *address.base_register << " + " << *address.displacement << "]";
        }

        return out << "[" << *address.base_register << "]";
    }

    if(address.displacement){
        return out << "[" << *address.displacement << "]";
    }

    cpp_unreachable("Invalid address type");
}
