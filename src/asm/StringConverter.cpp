//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "assert.hpp"

#include "asm/StringConverter.hpp"

#include "ltac/Address.hpp"
#include "ltac/FloatRegister.hpp"
#include "ltac/PseudoFloatRegister.hpp"
#include "ltac/Register.hpp"
#include "ltac/PseudoRegister.hpp"

using namespace eddic;

std::string as::StringConverter::register_to_string(eddic::ltac::AddressRegister& reg) const {
    if(auto* ptr = boost::get<ltac::PseudoRegister>(&reg)){
        return (*this)(*ptr);
    } else if(auto* ptr = boost::get<ltac::Register>(&reg)){
        return (*this)(*ptr);
    } else {
       eddic_unreachable("Invalid variant");
    }
}

std::string as::StringConverter::address_to_string(eddic::ltac::Address& address) const {
    if(address.absolute){
        if(address.displacement){
            return "[" + *address.absolute + " + " + std::to_string(*address.displacement) + "]";
        }

        if(address.base_register){
            return "[" + *address.absolute + " + " + register_to_string(*address.base_register) + "]";
        }

        return "[" + *address.absolute + "]";
    }
        
    if(address.base_register){
        if(address.scaled_register){
            if(address.scale){
                if(address.displacement){
                    return "[" + register_to_string(*address.base_register) + " + " + register_to_string(*address.scaled_register) + " * " + std::to_string(*address.scale) + " + " + std::to_string(*address.displacement) + "]";
                }
                
                return "[" + register_to_string(*address.base_register) + " + " + register_to_string(*address.scaled_register) + " * " + std::to_string(*address.scale) + "]";
            }
                
            if(address.displacement){
                return "[" + register_to_string(*address.base_register) + " + " + register_to_string(*address.scaled_register) + " + " + std::to_string(*address.displacement) + "]";
            }
            
            return "[" + register_to_string(*address.base_register) + " + " + register_to_string(*address.scaled_register) + "]";
        }
                
        if(address.displacement){
            return "[" + register_to_string(*address.base_register) + " + " + std::to_string(*address.displacement) + "]";
        }

        return "[" + register_to_string(*address.base_register) + "]";
    }

    if(address.displacement){
        return "[" + std::to_string(*address.displacement) + "]";
    }

    eddic_unreachable("Invalid address type");
}
